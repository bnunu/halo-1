/*
HOST_SDL.C

SDL3 on behalf of the guest (guest/runtime/guest_sdl.c). SDL objects are
64-bit pointers, which the guest cannot hold; it gets small handles into the
table here instead.

The guest calls these on its own threads, whose stacks (in guest memory) are
also the ones ART knows, so SDL may call into Java from them
(host_thread.c). SDL's audio thread is the exception: it has no guest stack,
so the audio callback is handed to a thread that has one.
*/

#include "host.h"

#include <SDL3/SDL.h>
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define HANDLE_COUNT 256

enum handle_type
{
	_handle_free,
	_handle_window,
	_handle_context,
	_handle_gamepad,
	_handle_audio,
};

struct handle
{
	int type;
	void *object;
	/* an audio stream with a callback: its binding (below) */
	void *binding;
};

static struct handle handles[HANDLE_COUNT];
static pthread_mutex_t handle_lock = PTHREAD_MUTEX_INITIALIZER;

static uint32_t handle_new(int type, void *object)
{
	uint32_t index;

	if (!object)
		return 0;
	pthread_mutex_lock(&handle_lock);
	/* an object that already has a handle keeps it */
	for (index = 1; index < HANDLE_COUNT; index++)
	{
		if (handles[index].type == type && handles[index].object == object)
		{
			pthread_mutex_unlock(&handle_lock);
			return index;
		}
	}
	for (index = 1; index < HANDLE_COUNT; index++)
	{
		if (handles[index].type == _handle_free)
		{
			handles[index].type = type;
			handles[index].object = object;
			handles[index].binding = NULL;
			pthread_mutex_unlock(&handle_lock);
			return index;
		}
	}
	pthread_mutex_unlock(&handle_lock);
	host_logf(HOST_LOG_ERROR, "out of SDL handles");
	return 0;
}

static void *handle_get(uint32_t handle, int type)
{
	void *object = NULL;

	if (handle == 0 || handle >= HANDLE_COUNT)
		return NULL;
	pthread_mutex_lock(&handle_lock);
	if (handles[handle].type == type)
		object = handles[handle].object;
	pthread_mutex_unlock(&handle_lock);
	return object;
}

/* the object of a handle, which the handle no longer refers to; NULL when
there is none. *binding gets what handle_new's caller stored (audio) */
static void *handle_release(uint32_t handle, int type, void **binding)
{
	void *object = NULL;

	if (binding)
		*binding = NULL;
	if (handle == 0 || handle >= HANDLE_COUNT)
		return NULL;
	pthread_mutex_lock(&handle_lock);
	if (handles[handle].type == type)
	{
		object = handles[handle].object;
		if (binding)
			*binding = handles[handle].binding;
		handles[handle].type = _handle_free;
		handles[handle].object = NULL;
		handles[handle].binding = NULL;
	}
	pthread_mutex_unlock(&handle_lock);
	return object;
}

/* ---------- general */

int host_sdl_init(uint32_t flags)
{
	return SDL_Init((SDL_InitFlags)flags);
}

int host_sdl_set_hint(const char *name, const char *value)
{
	return SDL_SetHint(name, value);
}

void host_sdl_get_error(char *buffer, uint32_t size)
{
	SDL_strlcpy(buffer, SDL_GetError(), size);
}

void host_sdl_scancode_name(int32_t scancode, char *buffer, uint32_t size)
{
	SDL_strlcpy(buffer, SDL_GetScancodeName((SDL_Scancode)scancode), size);
}

int32_t host_sdl_scancode_from_name(const char *name)
{
	return (int32_t)SDL_GetScancodeFromName(name);
}

int64_t host_sdl_ticks(void)
{
	return (int64_t)SDL_GetTicks();
}

int64_t host_sdl_thread_id(void)
{
	return (int64_t)SDL_GetCurrentThreadID();
}

/* ---------- video */

uint32_t host_sdl_create_window(const char *title, int width, int height, int64_t flags)
{
	return handle_new(_handle_window, SDL_CreateWindow(title, width, height, (SDL_WindowFlags)flags));
}

void host_sdl_window_size_in_pixels(uint32_t window, int *width, int *height)
{
	SDL_Window *object = handle_get(window, _handle_window);

	*width = 0;
	*height = 0;
	if (object)
		SDL_GetWindowSizeInPixels(object, width, height);
}

int host_sdl_set_relative_mouse(uint32_t window, int enabled)
{
	SDL_Window *object = handle_get(window, _handle_window);

	return object ? SDL_SetWindowRelativeMouseMode(object, enabled != 0) : 0;
}

int host_sdl_gl_set_attribute(int attribute, int value)
{
	return SDL_GL_SetAttribute((SDL_GLAttr)attribute, value);
}

uint32_t host_sdl_gl_create_context(uint32_t window)
{
	SDL_Window *object = handle_get(window, _handle_window);

	return object ? handle_new(_handle_context, SDL_GL_CreateContext(object)) : 0;
}

int host_sdl_gl_make_current(uint32_t window, uint32_t context)
{
	return SDL_GL_MakeCurrent(handle_get(window, _handle_window), handle_get(context, _handle_context));
}

int host_sdl_gl_set_swap_interval(int interval)
{
	return SDL_GL_SetSwapInterval(interval);
}

/* ---------- the performance hint (Android 13 and later)

The game draws each frame on one thread, which the scheduler moves between the
big cores; each core then looks half idle to the CPU governor, which kept them
at a third to a half of their clock in a fight (a Pixel 6a: 850-1430 of 2802
MHz, 40 fps). A performance hint session tells the system the thread's work
each frame and the time a frame has (one refresh), so the clocks rise to meet
it. The work is the time between two swaps, less the swap itself. The target
is HINT_TARGET_SHARE of a refresh, not all of it: the governor answers a
heavier frame a few frames late, and at a whole refresh a fight's spikes
dropped to the mid 40s. A display's refresh rate can change while the game
runs (60, 90 and 120 Hz phones switch), so it is read again every
HINT_REFRESH_CHECK_FRAMES and the target follows. The thread that runs the
audio callback (audio_thread) has a session of its own: left alone it mixed
on a mid core at 550-1000 of 2253 MHz, about 8 ms of each 21 ms callback, and
the game's thread waited for the mixer's lock meanwhile; its work is each
callback, its target HINT_AUDIO_TARGET. Looked up at run time: the app runs
from Android 9, where the functions are not there and nothing changes. */

#define HINT_TARGET_SHARE 0.75
#define HINT_REFRESH_CHECK_FRAMES 30
/* the audio callback's work: a fifth of a 1024 frame callback (21 ms) */
#define HINT_AUDIO_TARGET 4000000LL

typedef struct APerformanceHintManager APerformanceHintManager;
typedef struct APerformanceHintSession APerformanceHintSession;

static struct
{
	APerformanceHintManager *(*get_manager)(void);
	APerformanceHintSession *(*create_session)(APerformanceHintManager *, const int32_t *, size_t, int64_t);
	int (*update_target_work_duration)(APerformanceHintSession *, int64_t);
	int (*report_actual_work_duration)(APerformanceHintSession *, int64_t);
} performance_hint;

static APerformanceHintSession *hint_session;
static pid_t hint_thread;
static int hint_tried;
static int64_t hint_swap_end;
static int64_t hint_target;
static unsigned long hint_frames;

static int64_t monotonic_nanoseconds(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (int64_t)now.tv_sec * 1000000000LL + now.tv_nsec;
}

/* (the target for the window's display's refresh rate now) */
static int64_t performance_hint_target(SDL_Window *window)
{
	const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));
	double refresh = mode && mode->refresh_rate > 0.0f ? mode->refresh_rate : 60.0;

	return (int64_t)(1e9 * HINT_TARGET_SHARE / refresh);
}

/* the functions, looked up once from whichever thread asks first; the
manager, or NULL where there is none */
static pthread_once_t hint_loaded = PTHREAD_ONCE_INIT;
static APerformanceHintManager *hint_manager;

static void performance_hint_load(void)
{
	void *library = dlopen("libandroid.so", RTLD_NOW);

	if (!library)
		return;
	performance_hint.get_manager = (APerformanceHintManager *(*)(void))dlsym(library, "APerformanceHint_getManager");
	performance_hint.create_session = (APerformanceHintSession *(*)(APerformanceHintManager *, const int32_t *, size_t,
		int64_t))dlsym(library, "APerformanceHint_createSession");
	performance_hint.update_target_work_duration = (int (*)(APerformanceHintSession *, int64_t))dlsym(library,
		"APerformanceHint_updateTargetWorkDuration");
	performance_hint.report_actual_work_duration = (int (*)(APerformanceHintSession *, int64_t))dlsym(library,
		"APerformanceHint_reportActualWorkDuration");
	if (!performance_hint.get_manager || !performance_hint.create_session ||
		!performance_hint.update_target_work_duration || !performance_hint.report_actual_work_duration)
	{
		host_logf(HOST_LOG_INFO, "performance hint: not on this Android version");
		return;
	}
	hint_manager = performance_hint.get_manager();
}

/* a session for this thread, or NULL */
static APerformanceHintSession *performance_hint_session(const char *name, int64_t target)
{
	APerformanceHintSession *session;
	int32_t thread = (int32_t)gettid();

	pthread_once(&hint_loaded, performance_hint_load);
	session = hint_manager ? performance_hint.create_session(hint_manager, &thread, 1, target) : NULL;
	if (hint_manager)
		host_logf(HOST_LOG_INFO, "performance hint: %s for the %s thread %d, %.2f ms of work",
			session ? "session" : "no session", name, (int)thread, (double)target / 1e6);
	return session;
}

/* (the first swap's thread: the one that draws) */
static void performance_hint_start(SDL_Window *window)
{
	hint_tried = 1;
	hint_target = performance_hint_target(window);
	hint_session = performance_hint_session("drawing", hint_target);
	hint_thread = gettid();
}

/* (every HINT_REFRESH_CHECK_FRAMES: the refresh rate changed, the target with it) */
static void performance_hint_follow_refresh(SDL_Window *window)
{
	int64_t target;

	if (++hint_frames % HINT_REFRESH_CHECK_FRAMES)
		return;
	target = performance_hint_target(window);
	if (target == hint_target)
		return;
	hint_target = target;
	performance_hint.update_target_work_duration(hint_session, target);
	host_logf(HOST_LOG_INFO, "performance hint: %.2f ms of work a frame (the refresh rate changed)",
		(double)target / 1e6);
}

int host_sdl_gl_swap_window(uint32_t window)
{
	SDL_Window *object = handle_get(window, _handle_window);
	int64_t swap_start;
	int result;

	if (!object)
		return 0;
	if (!hint_tried)
		performance_hint_start(object);
	swap_start = monotonic_nanoseconds();
	/* (the drawing thread's frames only; the first has no start) */
	if (hint_session && gettid() == hint_thread)
	{
		if (hint_swap_end && swap_start > hint_swap_end)
			performance_hint.report_actual_work_duration(hint_session, swap_start - hint_swap_end);
		performance_hint_follow_refresh(object);
	}
	result = SDL_GL_SwapWindow(object);
	hint_swap_end = monotonic_nanoseconds();
	return result;
}

/* ---------- events */

int host_sdl_poll_event(void *event)
{
	SDL_Event host_event;

	if (!SDL_PollEvent(&host_event))
		return 0;
	/* the layouts agree except for the pointers of text, drop and user
	events, which the guest does not read */
	memcpy(event, &host_event, sizeof(host_event));
	return 1;
}

/* ---------- text input (Android's keyboard while a name or a chat line is
typed: sdl_platform.c). What it types arrives as key events, which the guest
reads; its text events are not (their text is a pointer) */

int host_sdl_text_input(uint32_t window, int type)
{
	SDL_Window *object = handle_get(window, _handle_window);
	SDL_PropertiesID properties;

	if (!object)
		return 0;
	if (type < 0)
	{
		SDL_StopTextInput(object);
		return 0;
	}
	properties = SDL_CreateProperties();
	SDL_SetNumberProperty(properties, SDL_PROP_TEXTINPUT_TYPE_NUMBER, type);
	SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
	SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
	SDL_StartTextInputWithProperties(object, properties);
	SDL_DestroyProperties(properties);
	return SDL_TextInputActive(object) ? 1 : 0;
}

/* (SDL's SDLDummyEdit stops it when back closes Android's keyboard; the
keyboard's own hide key goes unseen) */
int host_sdl_text_input_active(uint32_t window)
{
	SDL_Window *object = handle_get(window, _handle_window);

	return object && SDL_TextInputActive(object) ? 1 : 0;
}

/* ---------- gamepads */

int host_sdl_get_gamepads(uint32_t *ids, int capacity)
{
	int count = 0, index;
	SDL_JoystickID *list = SDL_GetGamepads(&count);

	if (!list)
		return 0;
	if (count > capacity)
		count = capacity;
	for (index = 0; index < count; index++)
		ids[index] = list[index];
	SDL_free(list);
	return count;
}

uint32_t host_sdl_open_gamepad(uint32_t id)
{
	SDL_Gamepad *gamepad = SDL_OpenGamepad((SDL_JoystickID)id);

	if (gamepad)
		host_logf(HOST_LOG_INFO, "gamepad %u: %s (type %d, %04x:%04x)", (unsigned)id, SDL_GetGamepadName(gamepad),
			(int)SDL_GetGamepadType(gamepad), SDL_GetGamepadVendor(gamepad), SDL_GetGamepadProduct(gamepad));
	return handle_new(_handle_gamepad, gamepad);
}

uint32_t host_sdl_gamepad_from_id(uint32_t id)
{
	return handle_new(_handle_gamepad, SDL_GetGamepadFromID((SDL_JoystickID)id));
}

/* (the event thread, when the controller goes: sdl_platform.c) */
void host_sdl_close_gamepad(uint32_t gamepad)
{
	SDL_Gamepad *object = handle_release(gamepad, _handle_gamepad, NULL);

	if (object)
		SDL_CloseGamepad(object);
}

int host_sdl_gamepad_axis(uint32_t gamepad, int axis)
{
	SDL_Gamepad *object = handle_get(gamepad, _handle_gamepad);

	return object ? SDL_GetGamepadAxis(object, (SDL_GamepadAxis)axis) : 0;
}

int host_sdl_gamepad_button(uint32_t gamepad, int button)
{
	SDL_Gamepad *object = handle_get(gamepad, _handle_gamepad);

	return object ? SDL_GetGamepadButton(object, (SDL_GamepadButton)button) : 0;
}

int host_sdl_gamepad_type(uint32_t gamepad)
{
	SDL_Gamepad *object = handle_get(gamepad, _handle_gamepad);

	return object ? SDL_GetGamepadType(object) : SDL_GAMEPAD_TYPE_UNKNOWN;
}

int host_sdl_rumble_gamepad(uint32_t gamepad, uint32_t low, uint32_t high, uint32_t milliseconds)
{
	SDL_Gamepad *object = handle_get(gamepad, _handle_gamepad);

	return object ? SDL_RumbleGamepad(object, (Uint16)low, (Uint16)high, milliseconds) : 0;
}

/* ---------- audio */

/* SDL calls audio_callback on its own audio thread, which cannot run guest
code; it passes each request to the stream's thread (audio_thread), which
can, and waits for it to be done. SDL holds the stream's lock throughout, so
the audio the guest puts into the stream meanwhile (from audio_thread) is
kept in the binding's buffer instead, and put in by audio_callback once the
guest is done: audio_thread putting it in itself would wait for the lock
forever */
struct audio_binding
{
	uint32_t handle;
	uint32_t callback;
	uint32_t userdata;
	pthread_mutex_t lock;
	pthread_cond_t requested;
	pthread_cond_t done;
	int pending;
	/* the stream is destroyed: the thread frees the binding and ends */
	int quit;
	int additional;
	int total;
	unsigned char *buffer;
	int buffer_length;
	int buffer_size;
};

/* the binding whose callback this thread is running, if any */
static __thread struct audio_binding *calling_back;

static void *audio_thread(void *context)
{
	struct audio_binding *binding = context;
	/* (its performance hint session: the hint above) */
	APerformanceHintSession *hint_audio = NULL;
	int hint_tried_audio = 0;
	int64_t start;

	pthread_mutex_lock(&binding->lock);
	for (;;)
	{
		int additional, total;

		while (!binding->pending && !binding->quit)
			pthread_cond_wait(&binding->requested, &binding->lock);
		if (binding->quit)
			break;
		additional = binding->additional;
		total = binding->total;
		pthread_mutex_unlock(&binding->lock);
		if (!hint_tried_audio)
		{
			hint_tried_audio = 1;
			hint_audio = performance_hint_session("audio", HINT_AUDIO_TARGET);
		}
		start = monotonic_nanoseconds();
		calling_back = binding;
		host_call_guest(binding->callback, binding->userdata, binding->handle, (uint32_t)additional, (uint32_t)total);
		calling_back = NULL;
		if (hint_audio)
			performance_hint.report_actual_work_duration(hint_audio, monotonic_nanoseconds() - start);
		pthread_mutex_lock(&binding->lock);
		binding->pending = 0;
		pthread_cond_signal(&binding->done);
	}
	pthread_mutex_unlock(&binding->lock);
	pthread_cond_destroy(&binding->done);
	pthread_cond_destroy(&binding->requested);
	pthread_mutex_destroy(&binding->lock);
	SDL_free(binding->buffer);
	SDL_free(binding);
	return NULL;
}

static void SDLCALL audio_callback(void *userdata, SDL_AudioStream *stream, int additional, int total)
{
	struct audio_binding *binding = userdata;

	pthread_mutex_lock(&binding->lock);
	binding->additional = additional;
	binding->total = total;
	binding->pending = 1;
	pthread_cond_signal(&binding->requested);
	while (binding->pending)
		pthread_cond_wait(&binding->done, &binding->lock);
	pthread_mutex_unlock(&binding->lock);
	if (binding->buffer_length)
	{
		SDL_PutAudioStreamData(stream, binding->buffer, binding->buffer_length);
		binding->buffer_length = 0;
	}
}

/* audio the guest puts into its stream during the stream's callback, for
audio_callback to put in; 1 on success */
static int audio_keep(struct audio_binding *binding, const void *data, int length)
{
	if (length < 0)
		return 0;
	if (binding->buffer_length + length > binding->buffer_size)
	{
		int size = (binding->buffer_length + length) * 2;
		unsigned char *buffer = SDL_realloc(binding->buffer, (size_t)size);

		if (!buffer)
			return 0;
		binding->buffer = buffer;
		binding->buffer_size = size;
	}
	memcpy(binding->buffer + binding->buffer_length, data, (size_t)length);
	binding->buffer_length += length;
	return 1;
}

uint32_t host_sdl_open_audio_stream(uint32_t device, const void *spec, uint32_t callback, uint32_t userdata)
{
	struct audio_binding *binding;
	SDL_AudioStream *stream;

	/* (one without a callback, the microphone's, which the guest reads:
	nothing to bind) */
	if (!callback)
	{
		stream = SDL_OpenAudioDeviceStream((SDL_AudioDeviceID)device, spec, NULL, NULL);
		return stream ? handle_new(_handle_audio, stream) : 0;
	}
	binding = SDL_calloc(1, sizeof(*binding));
	if (!binding)
		return 0;
	binding->callback = callback;
	binding->userdata = userdata;
	pthread_mutex_init(&binding->lock, NULL);
	pthread_cond_init(&binding->requested, NULL);
	pthread_cond_init(&binding->done, NULL);
	stream = SDL_OpenAudioDeviceStream((SDL_AudioDeviceID)device, spec,
		callback ? audio_callback : NULL, binding);
	if (!stream)
	{
		SDL_free(binding);
		return 0;
	}
	/* the device starts paused, so no callback can run before this */
	binding->handle = handle_new(_handle_audio, stream);
	if (!binding->handle)
	{
		SDL_DestroyAudioStream(stream);
		pthread_cond_destroy(&binding->done);
		pthread_cond_destroy(&binding->requested);
		pthread_mutex_destroy(&binding->lock);
		SDL_free(binding);
		return 0;
	}
	pthread_mutex_lock(&handle_lock);
	handles[binding->handle].binding = binding;
	pthread_mutex_unlock(&handle_lock);
	if (host_native_thread_create(audio_thread, binding, 256 * 1024) != 0)
		host_fatal("cannot start the audio thread");
	return binding->handle;
}

int host_sdl_put_audio_stream_data(uint32_t stream, const void *data, int length)
{
	SDL_AudioStream *object = handle_get(stream, _handle_audio);

	if (!object)
		return 0;
	if (calling_back && calling_back->handle == stream)
		return audio_keep(calling_back, data, length);
	return SDL_PutAudioStreamData(object, data, length);
}

int host_sdl_resume_audio_stream_device(uint32_t stream)
{
	SDL_AudioStream *object = handle_get(stream, _handle_audio);

	return object ? SDL_ResumeAudioStreamDevice(object) : 0;
}

/* (voice chat's microphone: port/linux/src/voice_audio.c) */
int host_sdl_get_audio_stream_data(uint32_t stream, void *data, int length)
{
	SDL_AudioStream *object = handle_get(stream, _handle_audio);

	return object ? SDL_GetAudioStreamData(object, data, length) : -1;
}

int host_sdl_get_audio_stream_available(uint32_t stream)
{
	SDL_AudioStream *object = handle_get(stream, _handle_audio);

	return object ? SDL_GetAudioStreamAvailable(object) : -1;
}

/* (a stream with a callback has a thread the guest's callbacks run on:
once no callback can run any more, it is told to end, and frees the
binding and its stack as it goes) */
void host_sdl_destroy_audio_stream(uint32_t stream)
{
	void *context;
	SDL_AudioStream *object = handle_release(stream, _handle_audio, &context);
	struct audio_binding *binding = context;

	if (!object)
		return;
	/* (returns once a callback that is running has finished) */
	SDL_DestroyAudioStream(object);
	if (binding)
	{
		pthread_mutex_lock(&binding->lock);
		binding->quit = 1;
		pthread_cond_signal(&binding->requested);
		pthread_mutex_unlock(&binding->lock);
	}
}

/* ---------- the clipboard (internet play's invite links) */

int host_sdl_set_clipboard_text(const char *text)
{
	return SDL_SetClipboardText(text) ? 1 : 0;
}

void host_sdl_get_clipboard_text(char *buffer, uint32_t size)
{
	char *text = SDL_GetClipboardText();

	SDL_strlcpy(buffer, text ? text : "", size);
	SDL_free(text);
}

int host_sdl_open_url(const char *url)
{
	return SDL_OpenURL(url) ? 1 : 0;
}

int host_sdl_show_toast(const char *message, int duration, int gravity, int x, int y)
{
	return SDL_ShowAndroidToast(message, duration, gravity, x, y) ? 1 : 0;
}

/* ---------- a message for the player (a host of another network version) */

int host_sdl_show_simple_message_box(uint32_t flags, const char *title, const char *message)
{
	return SDL_ShowSimpleMessageBox((SDL_MessageBoxFlags)flags, title, message, NULL) ? 1 : 0;
}

/* ---------- a question for the player (the mods at startup: port_mods.c) */

int host_sdl_show_message_box(uint32_t flags, const char *title, const char *message, int default_index,
	const char *button0, const char *button1, const char *button2)
{
	const char *labels[3] = { button0, button1, button2 };
	SDL_MessageBoxButtonData buttons[3];
	SDL_MessageBoxData question;
	int count = 0;
	int answer = -1;
	int index;

	for (index = 0; index < 3; index++)
	{
		if (!labels[index])
			continue;
		buttons[count].flags = index == default_index ?
			SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0;
		buttons[count].buttonID = index;
		buttons[count].text = labels[index];
		count++;
	}
	question.flags = (SDL_MessageBoxFlags)flags;
	question.window = NULL;
	question.title = title;
	question.message = message;
	question.numbuttons = count;
	question.buttons = buttons;
	question.colorScheme = NULL;
	return SDL_ShowMessageBox(&question, &answer) ? answer : -1;
}

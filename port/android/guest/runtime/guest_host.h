/*
GUEST_HOST.H

The host services the guest imports (the guest's view; the host defines
them in port/android/host). Every name here must also be listed in
port/android/host_imports.list, which generates the import stubs.

Parameter types follow the rules in halo_android_abi.h: 32-bit values are
int or unsigned int, 64-bit values long long, and pointers are passed as
they are.
*/

#ifndef __GUEST_HOST_H
#define __GUEST_HOST_H

/* ---------- process */

/* performs a Linux system call on the guest's behalf, converting the
structures whose layout differs; returns the raw result (-errno on failure) */
long long host_syscall(long long number, long long a, long long b, long long c,
	long long d, long long e, long long f);

/* Android log priorities (android/log.h) */
void host_log(int priority, const char *text);
void host_abort(const char *reason) __attribute__((noreturn));
void host_exit(int code) __attribute__((noreturn));
/* the host's errno on this thread, after a call to a host function */
int host_errno(void);

/* ---------- threads

The guest's thread pointer (its struct pthread) is kept by the host for
each thread. */

unsigned int host_get_tp(void);
void host_set_tp(unsigned int thread);
/* starts a host thread with a stack in guest memory that calls the image's
__guest_thread_start(thread); returns 0 or an errno value */
int host_thread_create(unsigned int thread, unsigned int stack_size);

/* MCC tag address space (host_mcc_memory.c), independent of the guest pools.
Reserve returns the address or negative errno; release returns zero or negative errno. */
long long host_mcc_memory_reserve(unsigned int address, unsigned int size);
int host_mcc_memory_release(unsigned int address, unsigned int size);

/* ---------- memory write tracking (port/linux/src/memory_watch.c) */

void host_memory_watch_initialize(void);
void host_memory_watch_protect(unsigned int address, unsigned int size);
unsigned int host_memory_watch_generation(unsigned int address, unsigned int size);
unsigned int host_memory_watch_serial(void);
void host_memory_watch_prepare_write(unsigned int address, unsigned int size);
void host_memory_watch_forget(unsigned int address, unsigned int size);
/* memory_watch_begin_frame (port/linux/src/platform.h) */
void host_memory_watch_begin_frame(void);

/* ---------- SDL (guest/runtime/guest_sdl.c)

Window, context, gamepad and audio stream objects are small integer
handles on this side. */

int host_sdl_init(unsigned int flags);
int host_sdl_set_hint(const char *name, const char *value);
void host_sdl_get_error(char *buffer, unsigned int size);
long long host_sdl_ticks(void);
long long host_sdl_thread_id(void);
unsigned int host_sdl_create_window(const char *title, int width, int height, long long flags);
void host_sdl_window_size_in_pixels(unsigned int window, int *width, int *height);
int host_sdl_set_relative_mouse(unsigned int window, int enabled);
int host_sdl_gl_set_attribute(int attribute, int value);
unsigned int host_sdl_gl_create_context(unsigned int window);
int host_sdl_gl_make_current(unsigned int window, unsigned int context);
int host_sdl_gl_set_swap_interval(int interval);
int host_sdl_gl_swap_window(unsigned int window);
int host_sdl_poll_event(void *event);
/* SDL's text input for the window (Android's keyboard while a name is typed:
sdl_platform.c), started as the SDL_TextInputType given, one line and with no
autocorrection, or stopped (-1); whether it runs */
int host_sdl_text_input(unsigned int window, int type);
int host_sdl_set_clipboard_text(const char *text);
void host_sdl_get_clipboard_text(char *buffer, unsigned int size);
int host_sdl_open_url(const char *url);
/* the keyboard's keys by name (the controls' bindings, xinput_sdl.c) */
void host_sdl_scancode_name(int scancode, char *buffer, unsigned int size);
int host_sdl_scancode_from_name(const char *name);
int host_sdl_show_toast(const char *message, int duration, int gravity, int x, int y);
int host_sdl_show_simple_message_box(unsigned int flags, const char *title, const char *message);
/* SDL_ShowMessageBox of up to three buttons (SDL_MessageBoxData has not the
guest's layout): the index of the one chosen, else -1 */
int host_sdl_show_message_box(unsigned int flags, const char *title, const char *message, int default_index,
	const char *button0, const char *button1, const char *button2);
int host_sdl_get_gamepads(unsigned int *ids, int capacity);
unsigned int host_sdl_open_gamepad(unsigned int id);
unsigned int host_sdl_gamepad_from_id(unsigned int id);
int host_sdl_gamepad_axis(unsigned int gamepad, int axis);
int host_sdl_gamepad_button(unsigned int gamepad, int button);
int host_sdl_gamepad_type(unsigned int gamepad);
int host_sdl_rumble_gamepad(unsigned int gamepad, unsigned int low, unsigned int high, unsigned int milliseconds);
void host_sdl_close_gamepad(unsigned int gamepad);
/* callback: void (*)(void *userdata, unsigned int stream, int additional, int total),
called on the audio thread */
unsigned int host_sdl_open_audio_stream(unsigned int device, const void *spec, unsigned int callback, unsigned int userdata);
int host_sdl_put_audio_stream_data(unsigned int stream, const void *data, int length);
int host_sdl_resume_audio_stream_device(unsigned int stream);
/* (voice chat's microphone, a stream without a callback) */
int host_sdl_get_audio_stream_data(unsigned int stream, void *data, int length);
int host_sdl_get_audio_stream_available(unsigned int stream);
void host_sdl_destroy_audio_stream(unsigned int stream);

/* ---------- OpenGL ES */

/* copies glGetString(name) (or glGetStringi when index >= 0) */
void host_gl_get_string(unsigned int name, int index, char *buffer, unsigned int size);
/* nonzero if the context supports the named extension */
int host_gl_has_extension(const char *name);
/* copies size bytes at offset of a GL buffer object into data, waiting for
the GPU's writes to it */
void host_gl_read_buffer(unsigned int buffer, unsigned int offset, unsigned int size, void *data);
/* unsynchronized write into the buffer bound to target */
void host_gl_buffer_write(unsigned int target, unsigned int offset, unsigned int size, const void *data);
/* fences the GPU work queued so far as that of ring slot `slot`; waits for
the GPU to finish the work last fenced for a slot */
void host_gl_fence_frame(unsigned int slot);
void host_gl_wait_frame(unsigned int slot);

/* ---------- Android */

/* SoC classification for the immutable rendering policy. */
int host_android_chip_family(void);

/* the storage directories the port uses, copied into buffer */
void host_android_path(int which, char *buffer, unsigned int size);

/* the edges where Android keeps its gestures, as left, top, right, bottom
in pixels of the current orientation, into insets[4]; all 0 when unknown */
void host_gesture_insets(int *insets);

/* ---------- the on-screen touch controls (host_touch.c) */

/* the overlay's controller: the SDL axes (left x, y, right x, y, left
trigger, right trigger) and the SDL button bits, into state[7] */
void host_touch_read(int *state);
/* the overlay's view swipe, then its gyroscope turn, since the last read,
into delta[4] */
void host_touch_look_read(float *delta);
/* port 0's motors, for the phone's vibration */
void host_touch_rumble(unsigned int low, unsigned int high);
/* tells the overlay when to show: touch_input.c's _touch_scene_* */
void host_touch_scene(int scene);
/* the game control on each of the 16 controller buttons (touch_game.c),
for the overlay's labels */
void host_touch_bindings(const int *controls);
/* the overlay's devices: 1, a touchscreen; 2, a game controller; 0 until it
has looked */
int host_touch_devices(void);

/* ---------- the server browser's master servers (host_https.c) */

/* a GET of an https:// url (port/linux/src/master_list.c): 1 with the
status, the headers ("Name: value" lines) and up to body_capacity bytes of
the body (its size in body_size), else 0 and why in error */
int host_https_get(const char *url, int *status, char *headers, int headers_size, char *body, int body_capacity,
	int *body_size, char *error, int error_size);

/* an MCC package's fingerprint (SHA-256 and CRC32) on the host's threads, with the CPU's
instructions, and kept on the card (port/android/host/host_mcc_hash.c, which says what each
does; the guest's side is port/linux/game/mcc_resource_check.inl's) */
int host_mcc_hash_start(unsigned int request);
int host_mcc_hash_wait(int id, int milliseconds, unsigned int result);
void host_mcc_hash_cancel(int id);
/* a file's plain SHA-256 and size, on the host's threads (an MCC package a host offers) */
int host_sha256_start(unsigned int path);
int host_sha256_poll(int id, unsigned int digest, unsigned int size);
void host_sha256_cancel(int id);
/* the Steam Workshop screen, whose Steam client is the host's (port/android/host/steam_downloads.c, which
says what each does; the guest's wrappers are port/linux/src/steam_screen.c's) */
void host_steam_open(unsigned int data_root);
void host_steam_close(void);
int host_steam_poll(void);
int host_steam_count(void);
int host_steam_view(void);
int host_steam_first(void);
void host_steam_set_first(int first);
int host_steam_chosen(void);
void host_steam_set_chosen(int chosen);
void host_steam_row(int place, unsigned int name, int name_size, unsigned int state, int state_size);
void host_steam_details(int place, unsigned int title, int title_size, unsigned int text, int text_size);
void host_steam_choose(int place);
int host_steam_back(void);
int host_steam_picture(unsigned int key, unsigned int pixels);
int host_steam_text_wanted(unsigned int initial, int initial_size, unsigned int secret);
void host_steam_text_caption(int kind, unsigned int text, int text_size);
void host_steam_text_typed(int kind, unsigned int text, int saved);
/* a joiner's fetch of an MCC package's own bitmaps.map and sounds.map from
the Workshop item it is, beside the host's download (steam_downloads.c):
records of fixed-size fields (struct host_steam_fetch_file, _status there) */
int host_steam_fetch_start(unsigned int data_root, unsigned int id_low, unsigned int id_high, unsigned int files,
	int count);
int host_steam_fetch_status(int index, unsigned int status);
void host_steam_fetch_skip(int index);
void host_steam_fetch_stop(void);
/* MCC's shared sets (the base resources no host sends): queued when missing */
int host_steam_shared_fetch(unsigned int data_root);
int host_steam_shared_state(void);
/* a whole Workshop item installed in mcc_maps for a joiner (a package with no resource maps of its own) */
int host_steam_install_start(unsigned int data_root, unsigned int id_low, unsigned int id_high);
int host_steam_install_status(void);
/* files put in mcc_maps by the Steam downloader since the last ask (once) */
int host_steam_take_rescan(void);

#endif

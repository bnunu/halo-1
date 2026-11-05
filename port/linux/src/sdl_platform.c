/*
SDL_PLATFORM.C

The SDL3 window, OpenGL context and event loop behind the Linux build.

The window is created with the Direct3D device (d3d8_gl.c) on the game's
main thread, which is also the only thread that pumps events. Keyboard and
mouse state gathered here feeds the controller emulation in xinput_sdl.c
and the debug keyboard that the game's console reads.
*/

#include "platform.h"
#include "sdl_platform.h"
#include "gl.h"
#include "port_config.h"
#include "p2p.h"
#include "port_mods.h"
#include "server_mode.h"
#include "server_master.h"
#include "player_link.h"
#include "xiso.h"
#include "touch_input.h"
#include "../game/text_filter.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HALO_WINDOWS
/* port/windows/src/win32_window.c, compiled with the Windows SDK */
void win32_window_borderless_apply(SDL_Window *window);
#endif
#if !defined(_WIN32) && !defined(HALO_ANDROID)
#include <signal.h>
#endif
#if defined(__linux__) && !defined(HALO_ARM64_GUEST) && !defined(HALO_ANDROID)
#include <dlfcn.h>
#include <unistd.h>
#endif
#ifdef HALO_ANDROID
#include <errno.h>
#include <time.h>
#endif

static SDL_Window *platform_window;
static SDL_GLContext platform_gl_context;
static SDL_ThreadID platform_event_thread;
static BOOL platform_sdl_started = FALSE;
/* the game's window was asked for (platform_video_initialize): without one
after that, the game runs without a window (platform_pump_events) */
static BOOL platform_video_tried = FALSE;

static struct platform_input_state input_state;
/* keys pressed since the last read, so a press and release between two
reads still counts as a press (input injected on Android, or a slow frame) */
static unsigned char keys_pressed[SDL_SCANCODE_COUNT];
/* The bound Screenshot action requests capture at the next presentation. */
static BOOL screenshot_requested;
/* likewise the mouse buttons pressed since the last read, so that a click
quicker than a frame still counts */
static unsigned char mouse_buttons_pressed[PLATFORM_MOUSE_BUTTON_COUNT];
#ifndef HALO_ANDROID
/* the menus' pointer (platform_ui_pointer_set_active), under input_lock */
static struct platform_ui_pointer ui_pointer;
static float ui_pointer_wheel;
#endif
static pthread_mutex_t input_lock = PTHREAD_MUTEX_INITIALIZER;
/* rebinding (platform_binding_capture_begin), under input_lock: waiting for
an input, and the one taken; then the keyboard and mouse settle (their keys
and buttons held then are let go) before they drive anything again */
enum
{
	_binding_capture_idle,
	_binding_capture_waiting,
	_binding_capture_taken,
};
static int binding_capture;
static int binding_capture_result;
static int binding_captured_input;
static BOOL binding_settling;
static Uint64 binding_taken_ms;
/* when the menus last asked (a capture they stop asking about, their screen
gone, ends: else the keyboard stays held from the game) */
static Uint64 binding_polled_ms;
#define BINDING_ABANDONED_MS 500
static unsigned mouse_buttons_down;
/* (an input taken that nothing asks for is let go after this) */
#define BINDING_UNCLAIMED_MS 2000
/* the multiplayer scoreboard is open (platform_scoreboard_scroll): the wheel
and Page Up/Down scroll it, and the wheel switches no weapon; how far they
have moved it since the game last asked (notches down, pages down). Open
until the game stops saying so for SCOREBOARD_OPEN_MS (a game that ends with
it open never says it closed). */
#define SCOREBOARD_OPEN_MS 250
static Uint64 scoreboard_open_until_ms;
static float scoreboard_wheel;
static long scoreboard_notches;
static long scoreboard_pages;
#ifndef HALO_ANDROID
/* the scoreboard's pointer (platform_scoreboard_pointer): while the game
offers it (a network game's scoreboard is open), a right click frees the
mouse, whose pointer then picks a player; its motion and clicks go to it,
not to the aim and the triggers. Another right click, or the scoreboard
closing, takes the mouse back for the aim. */
static BOOL scoreboard_pointer_offered;
static struct platform_ui_pointer scoreboard_pointer;
#endif
static BOOL scoreboard_pointer_active;

/* debug keyboard queue */
#define KEYSTROKE_QUEUE_SIZE 64
static struct platform_keystroke keystroke_queue[KEYSTROKE_QUEUE_SIZE];
static unsigned long keystroke_head, keystroke_count;

/* the system's on-screen keyboard shown or hidden as wanted (Steam's, or
Android's) */
static void screen_keyboard_update(void);
#ifndef HALO_ANDROID
/* dsound_sdl.c's: the output device followed */
void dsound_sdl_output_device_check(void);
/* updater.c's: the desktop self-updater */
void updater_start(void);
void updater_poll(SDL_Window *window);
#ifndef HALO_GLES
/* the windows' icon, a PNG (tools/embed_assets.py, from port/assets/icon) */
extern const unsigned int platform_window_icon[];
extern const unsigned long platform_window_icon_size;
/* the menus' pointer, a PNG (tools/embed_assets.py, from port/assets/cursor) */
extern const unsigned int platform_menu_cursor[];
extern const unsigned long platform_menu_cursor_size;
#endif
#endif

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* the maps folder checked to hold the Xbox maps (platform_maps_folder_check).
When it holds Halo PC's instead, or no ui.map, the player is told where each
kind goes and the game quits, rather than failing to load Halo PC's ui.map
as the Xbox's (game_load). Halo PC maps in maps/ itself, which play only from
custom_maps, are warned of */
static void data_check_maps(void)
{
	struct platform_maps_folder maps;
	char message[2048];
	char where[1200];
	BOOL quiet = config_boolean("debug.hidden_window") || config_real("debug.exit_after") > 0.0 ||
		config_string("debug.network_test")[0];

	platform_maps_folder_check(&maps);
	snprintf(where, sizeof(where),
		"Put the Xbox maps from your Halo: Combat Evolved disc image in %s/maps. Halo Custom Edition "
		"maps go in a custom_maps folder beside it.",
		maps.root);
	switch (maps.state)
	{
	case _maps_folder_xbox:
		if (!maps.stray_pc_maps)
			return;
		snprintf(message, sizeof(message),
			"These maps in the maps folder are Halo PC maps, which play only from custom_maps: %s%s.\n\n%s",
			maps.stray_names, maps.stray_pc_maps > PLATFORM_MAPS_FOLDER_NAMED ? " and more" : "", where);
		platform_log("maps folder: %ld Halo PC maps in %s/maps (%s)", maps.stray_pc_maps, maps.root,
			maps.stray_names);
		if (!quiet)
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Halo", message, NULL);
		return;
	case _maps_folder_halo_pc:
		snprintf(message, sizeof(message),
			"The maps folder holds Halo PC maps, not the Xbox maps: its ui.map is from %s.\n\n%s\n\n"
			"To copy the Xbox maps out of your disc image again, move this maps folder aside and "
			"start the game.",
			maps.ui_version == 609 ? "Halo PC (Custom Edition)" : "Halo PC", where);
		break;
	default:
		snprintf(message, sizeof(message),
			"Halo's Xbox maps were not found: %s/maps has no ui.map.\n\n%s\n\n"
			"To copy the Xbox maps out of your disc image, move the maps folder aside and start the game.",
			maps.root, where);
		break;
	}
	platform_log("maps folder: %s/maps is not the Xbox maps (ui.map version %ld): quitting", maps.root,
		maps.ui_version);
	platform_log("%s", message);
	if (!quiet)
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Halo", message, NULL);
	exit(EXIT_FAILURE);
}
#endif

#if defined(__linux__) && !defined(HALO_ARM64_GUEST) && !defined(HALO_ANDROID)
/* NVIDIA's driver draws into a Wayland window through its egl-wayland
library, which distributions ship 64-bit only (Arch has no lib32 package
of it). In this 32-bit game, OpenGL on Wayland (EGL) then falls back to
Mesa's llvmpipe, drawn on the processor, while on X11 (XWayland, GLX) the
32-bit driver draws it. So with NVIDIA's module loaded and no 32-bit
egl-wayland, SDL tries X11 before Wayland; SDL_VIDEO_DRIVER still decides */
static void platform_choose_video_driver(void)
{
	static const char *const egl_wayland[] = { "libnvidia-egl-wayland.so.1", "libnvidia-egl-wayland2.so.1" };
	size_t index;

	if (getenv("SDL_VIDEO_DRIVER") || !getenv("WAYLAND_DISPLAY") || access("/sys/module/nvidia", F_OK) != 0)
		return;
	for (index = 0; index < sizeof(egl_wayland) / sizeof(egl_wayland[0]); index++)
	{
		void *library = dlopen(egl_wayland[index], RTLD_LAZY | RTLD_LOCAL);

		if (library)
		{
			dlclose(library);
			return;
		}
	}
	platform_log("video: NVIDIA's driver has no 32-bit egl-wayland here: X11 (XWayland) before Wayland, "
		"where OpenGL would be drawn on the processor (SDL_VIDEO_DRIVER=wayland chooses Wayland)");
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland");
}
#endif

BOOL platform_sdl_initialize(void)
{
	SDL_InitFlags flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS;

	if (platform_sdl_started)
		return TRUE;
#if !defined(_WIN32) && !defined(HALO_ANDROID)
	/* a write to a connection the other end closed fails instead of ending
	the game (the game's sockets and Discord's pass MSG_NOSIGNAL, but UPnP's
	miniupnpc does not, nor does a write to a closed pipe's standard error) */
	signal(SIGPIPE, SIG_IGN);
#endif
	/* a copy of the game started to open an invite link hands it to the
	one already running, and goes */
	if (p2p_hand_off_invite())
		exit(EXIT_SUCCESS);
	SDL_SetHint(SDL_HINT_APP_NAME, "Halo");
#if defined(__linux__) && !defined(HALO_ARM64_GUEST) && !defined(HALO_ANDROID)
	platform_choose_video_driver();
#endif
#ifdef HALO_ANDROID
	/* landscape only; the back key arrives as a key event (xinput_sdl.c)
	instead of closing the activity */
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	/* touching the screen must not aim or fire (the mouse drives the
	controller emulation in xinput_sdl.c) */
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif
#if defined(__linux__) && !defined(HALO_ANDROID)
	/* input.playstation_kernel_driver: a PlayStation pad through the
	kernel's driver of it (hid-playstation's, hid-sony's: its buttons,
	sticks, triggers and rumble) where the kernel has one, not SDL's own:
	SDL's reads the pad's hidraw node, which only udev rules (Steam's) open
	to the player, and a pad it took and could not open was not found at
	all. SDL's own has the lightbar and the motion sensors */
	if (config_boolean("input.playstation_kernel_driver"))
	{
		if (SDL_GetPathInfo("/sys/bus/hid/drivers/playstation", NULL))
			SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "0");
		if (SDL_GetPathInfo("/sys/bus/hid/drivers/sony", NULL))
			SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "0");
	}
#endif
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	/* a dedicated server (server_mode.c) runs where there is no display,
	sound or controller (a service, a container): SDL's drivers that need
	none, and its events alone */
	if (halo_server_mode())
	{
		SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
		SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
		flags = SDL_INIT_EVENTS;
	}
#endif
	if (!SDL_Init(flags))
	{
		/* (tried again by each caller that needs SDL, some of them every
		frame: a failure is logged once, and again only when its reason
		changes, not tens of thousands of times without a display) */
		static char last_failure[256];
		const char *error = SDL_GetError();

		if (strncmp(error, last_failure, sizeof(last_failure) - 1))
		{
			snprintf(last_failure, sizeof(last_failure), "%s", error);
			platform_log("SDL_Init failed: %s", error);
		}
		return FALSE;
	}
	platform_sdl_started = TRUE;
#ifndef HALO_ANDROID
	/* found (or offered to the player, platform_offer_game_data) before the
	game's window opens, and checked to be the Xbox maps (a dedicated
	server's were, by server_mode.c) */
	platform_data_root();
#ifndef HALO_WEB
	if (!halo_server_mode())
		data_check_maps();
#endif
	/* (a new version looked for meanwhile, updater_poll asking about it; a
	dedicated server's is server.auto_update's) */
	if (!halo_server_mode())
	{
		updater_start();
		/* (a player's public game on the master servers too, with a
		publisher key: server_master.c) */
		server_master_start_host();
	}
#endif
	/* (a Discord account linked at a master server: its passes asked for
	again before they run out, player_link.c) */
	if (!halo_server_mode())
		player_link_start();
	/* (the mods, and whether to play those that say they cannot be played
	together, before the window opens: port_mods.c) */
	port_mods_startup();
	return TRUE;
}

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* ---------- first start without game data (xbox_files.c; the web page
imports it itself) */

struct data_extraction
{
	pthread_mutex_t lock;
	char image[1024];
	char destination[1024];
	char file[256];
	unsigned long long done;
	unsigned long long total;
	BOOL finished;
	BOOL succeeded;
	char error[512];
};

static void data_extraction_progress(void *context, const char *file, unsigned long long done,
	unsigned long long total)
{
	struct data_extraction *extraction = context;

	pthread_mutex_lock(&extraction->lock);
	snprintf(extraction->file, sizeof(extraction->file), "%s", file);
	extraction->done = done;
	extraction->total = total;
	pthread_mutex_unlock(&extraction->lock);
}

static void *data_extraction_thread(void *context)
{
	struct data_extraction *extraction = context;
	BOOL succeeded = xiso_extract_maps(extraction->image, extraction->destination, data_extraction_progress,
		extraction, extraction->error, sizeof(extraction->error)) != 0;

	pthread_mutex_lock(&extraction->lock);
	extraction->succeeded = succeeded;
	extraction->finished = TRUE;
	pthread_mutex_unlock(&extraction->lock);
	return NULL;
}

/* copies the maps, showing how far it has got; closing the window quits */
static BOOL data_extract(const char *image, const char *destination, char *error, int error_size)
{
	static struct data_extraction extraction;
	SDL_Window *window;
	SDL_Renderer *renderer = NULL;
	pthread_t thread;
	BOOL finished = FALSE;

	memset(&extraction, 0, sizeof(extraction));
	pthread_mutex_init(&extraction.lock, NULL);
	snprintf(extraction.image, sizeof(extraction.image), "%s", image);
	snprintf(extraction.destination, sizeof(extraction.destination), "%s", destination);
	if (pthread_create(&thread, NULL, data_extraction_thread, &extraction) != 0)
	{
		snprintf(error, (size_t)error_size, "Could not start the extraction.");
		return FALSE;
	}
	/* (waited for through extraction.finished; the Windows port's threads
	cannot be joined) */
	pthread_detach(thread);
	window = SDL_CreateWindow("Halo", 640, 150, 0);
	if (window)
	{
		renderer = SDL_CreateRenderer(window, NULL);
		if (renderer)
			SDL_SetRenderVSync(renderer, 1);
	}
	while (!finished)
	{
		SDL_Event event;
		char file[256];
		unsigned long long done, total;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				platform_log("extraction cancelled");
				exit(EXIT_SUCCESS);
			}
		}
		pthread_mutex_lock(&extraction.lock);
		finished = extraction.finished;
		snprintf(file, sizeof(file), "%s", extraction.file);
		done = extraction.done;
		total = extraction.total;
		pthread_mutex_unlock(&extraction.lock);
		if (renderer)
		{
			char line[320];
			SDL_FRect bar = { 20.0f, 100.0f, 600.0f, 24.0f };
			float fraction = total ? (float)((double)done / (double)total) : 0.0f;

			SDL_SetRenderDrawColor(renderer, 12, 16, 20, 255);
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
			SDL_SetRenderScale(renderer, 2.0f, 2.0f);
			SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Extracting the maps folder...");
			SDL_SetRenderScale(renderer, 1.0f, 1.0f);
			snprintf(line, sizeof(line), "%s  (%llu of %llu MB)", file, done >> 20, total >> 20);
			SDL_RenderDebugText(renderer, 20.0f, 70.0f, line);
			SDL_SetRenderDrawColor(renderer, 60, 66, 72, 255);
			SDL_RenderFillRect(renderer, &bar);
			bar.w *= fraction;
			SDL_SetRenderDrawColor(renderer, 90, 160, 90, 255);
			SDL_RenderFillRect(renderer, &bar);
			SDL_RenderPresent(renderer);
		}
		SDL_Delay(16);
	}
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	if (!extraction.succeeded)
		snprintf(error, (size_t)error_size, "%s", extraction.error);
	return extraction.succeeded;
}

struct data_image_choice
{
	SDL_AtomicInt done;
	BOOL failed;
	char path[1024];
	char error[256];
};

static void SDLCALL data_image_chosen(void *userdata, const char * const *files, int filter)
{
	struct data_image_choice *choice = userdata;

	(void)filter;
	if (!files)
	{
		/* no dialog could be shown (SDL needs the desktop portal through
		libdbus, or zenity) */
		choice->failed = TRUE;
		snprintf(choice->error, sizeof(choice->error), "%s",
			*SDL_GetError() ? SDL_GetError() : "no desktop portal or zenity");
	}
	else if (files[0])
	{
		snprintf(choice->path, sizeof(choice->path), "%s", files[0]);
	}
	SDL_SetAtomicInt(&choice->done, 1);
}

/* a disc image in the folder the maps are extracted to (next to the
executable), for systems where no file dialog can be shown */
static BOOL data_find_image(const char *directory, char *path, int size)
{
	static const char *const patterns[] = { "*.iso", "*.xiso" };
	int pattern;

	for (pattern = 0; pattern < (int)(sizeof(patterns) / sizeof(patterns[0])); pattern++)
	{
		int count = 0;
		char **files = SDL_GlobDirectory(directory, patterns[pattern], SDL_GLOB_CASEINSENSITIVE, &count);

		if (files && count > 0)
		{
			snprintf(path, (size_t)size, "%s/%s", directory, files[0]);
			SDL_free(files);
			return TRUE;
		}
		SDL_free(files);
	}
	return FALSE;
}

/* the disc image the player picks; FALSE if they pick none, with *failed
set (and error written) when no dialog could be shown */
static BOOL data_choose_image(char *path, int size, BOOL *failed, char *error, int error_size)
{
	static const SDL_DialogFileFilter filters[] =
	{
		{ "Xbox disc images", "iso;xiso" },
		{ "All files", "*" },
	};
	static struct data_image_choice choice;

	memset(&choice, 0, sizeof(choice));
	SDL_ShowOpenFileDialog(data_image_chosen, &choice, NULL, filters, 2, NULL, false);
	/* the dialog answers through events (and on some systems another
	thread) */
	while (!SDL_GetAtomicInt(&choice.done))
	{
		SDL_PumpEvents();
		SDL_Delay(50);
	}
	*failed = choice.failed;
	if (choice.failed)
		snprintf(error, (size_t)error_size, "%s", choice.error);
	if (!choice.path[0])
		return FALSE;
	snprintf(path, (size_t)size, "%s", choice.path);
	return TRUE;
}

BOOL platform_offer_game_data(const char *destination)
{
	static const SDL_MessageBoxButtonData buttons[] =
	{
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" },
	};
	char message[2600];
	char found[1024];
	BOOL image_found;

	/* not for runs nobody is watching */
	if (config_boolean("debug.hidden_window") || config_real("debug.exit_after") > 0.0 ||
		!SDL_Init(SDL_INIT_VIDEO))
	{
		return FALSE;
	}
	image_found = data_find_image(destination, found, sizeof(found));
	if (image_found)
	{
		snprintf(message, sizeof(message),
			"Halo's game data (its maps folder) was not found.\n\n"
			"Extract the maps folder from the Xbox disc image %s? "
			"It is copied to %s/maps (about 2 GB).\n\n"
			"(Or put the maps folder there yourself, or set paths.data in config.toml.)",
			found, destination);
	}
	else
	{
		snprintf(message, sizeof(message),
			"Halo's game data (its maps folder) was not found.\n\n"
			"Extract the maps folder from an Xbox disc image (.iso) of Halo: Combat Evolved? "
			"It is copied to %s/maps (about 2 GB).\n\n"
			"(Or put the maps folder there yourself, or set paths.data in config.toml.)",
			destination);
	}
	for (;;)
	{
		SDL_MessageBoxData question = { SDL_MESSAGEBOX_INFORMATION, NULL, "Halo", message, 2, buttons, NULL };
		char image[1024];
		char error[512];
		int answer = 0;
		BOOL failed = FALSE;

		if (!SDL_ShowMessageBox(&question, &answer) || answer != 1)
		{
			platform_log("no game data: quitting");
			exit(EXIT_SUCCESS);
		}
		if (image_found)
		{
			snprintf(image, sizeof(image), "%s", found);
		}
		else if (!data_choose_image(image, sizeof(image), &failed, error, sizeof(error)))
		{
			char explanation[1400];

			/* no image picked: ask again */
			if (!failed)
				continue;
			/* no dialog could be shown */
			platform_log("no file dialog: %s", error);
			snprintf(explanation, sizeof(explanation),
				"No file dialog could be shown (%s).\n\n"
				"Put the disc image (.iso) in %s and start Halo again, "
				"or install zenity.",
				error, destination);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Halo", explanation, NULL);
			exit(EXIT_SUCCESS);
		}
		platform_log("extracting the maps folder from %s to %s", image, destination);
		if (data_extract(image, destination, error, sizeof(error)))
		{
			platform_log("extracted the maps folder");
			return TRUE;
		}
		platform_log("extraction failed: %s", error);
		image_found = FALSE;
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Halo", error, NULL);
	}
}
#endif

int halo_interpolation_enabled(void)
{
	static int enabled;
	static unsigned long read_at = (unsigned long)-1;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		enabled = config_boolean("display.interpolation");
	}
	return enabled;
}

#ifndef HALO_ANDROID
/* the display mode (display.mode, else display.fullscreen's: borderless or
the window) */
enum
{
	_display_mode_windowed = 0,
	_display_mode_borderless,
	_display_mode_fullscreen
};

static int platform_display_mode(void)
{
	const char *mode = config_string("display.mode");

	if (!strcmp(mode, "fullscreen"))
		return _display_mode_fullscreen;
	if (!strcmp(mode, "borderless"))
		return _display_mode_borderless;
	if (!strcmp(mode, "windowed"))
		return _display_mode_windowed;
	return config_boolean("display.fullscreen") ? _display_mode_borderless : _display_mode_windowed;
}

/* whether the window opens fullscreen (either kind), never when it is
hidden */
static BOOL platform_fullscreen_setting(void)
{
	return !config_boolean("debug.hidden_window") && platform_display_mode() != _display_mode_windowed;
}

/* a size as a setting has it, "<width>x<height>": whether it is one, and
the Xbox's 640x480 or more */
static BOOL platform_size_parse(const char *text, long *width, long *height)
{
	char *end;

	*width = strtol(text, &end, 10);
	*height = *end == 'x' || *end == 'X' ? strtol(end + 1, &end, 10) : 0;
	return !*end && *width >= 640 && *height >= 480;
}

/* display.resolution in pixels, or 0x0 for the display's own ("native"),
as for one the game cannot draw at */
static void platform_resolution_setting(long *width, long *height)
{
	if (!platform_size_parse(config_string("display.resolution"), width, height))
		*width = *height = 0;
}

/* the window's size (display.window_size), else the Xbox's 640x480 times
display.window_scale, as older versions set it */
static void platform_window_size_setting(long *width, long *height)
{
	long scale = config_integer("display.window_scale");

	if (!platform_size_parse(config_string("display.window_size"), width, height))
	{
		*width = 640 * (scale < 1 ? 1 : scale);
		*height = 480 * (scale < 1 ? 1 : scale);
	}
}

/* a display's own size in pixels (its desktop mode) */
static BOOL platform_display_size(SDL_DisplayID display, long *width, long *height)
{
	const SDL_DisplayMode *mode = display ? SDL_GetDesktopDisplayMode(display) : NULL;

	if (!mode)
		return FALSE;
	*width = (long)(mode->w * mode->pixel_density + 0.5f);
	*height = (long)(mode->h * mode->pixel_density + 0.5f);
	return TRUE;
}

/* the window's fullscreen kind (display.mode): borderless, a window over
the whole desktop (SDL's fullscreen without a mode), or fullscreen, the
display taken at display.resolution's mode (the one nearest it), else at its
desktop one. F11 switches to the kind set. */
static void platform_fullscreen_kind_apply(void)
{
	static int applied = -1;
	static long applied_width, applied_height;
	int exclusive = platform_display_mode() == _display_mode_fullscreen ? 1 : 0;
	long width = 0, height = 0;
	SDL_DisplayID display;
	SDL_DisplayMode closest;
	const SDL_DisplayMode *mode = NULL;

	if (!platform_window)
		return;
	if (exclusive)
		platform_resolution_setting(&width, &height);
	if (exclusive == applied && width == applied_width && height == applied_height)
		return;
	applied = exclusive;
	applied_width = width;
	applied_height = height;
	display = SDL_GetDisplayForWindow(platform_window);
	if (exclusive && display)
	{
		if (width && SDL_GetClosestFullscreenDisplayMode(display, (int)width, (int)height, 0.0f, false, &closest))
			mode = &closest;
		else
			mode = SDL_GetDesktopDisplayMode(display);
	}
	SDL_SetWindowFullscreenMode(platform_window, mode);
}

/* whether the game last asked for the window to be fullscreen */
static BOOL platform_fullscreen_requested = FALSE;

static void platform_window_set_fullscreen(BOOL fullscreen)
{
	platform_fullscreen_requested = fullscreen;
	SDL_SetWindowFullscreen(platform_window, fullscreen ? true : false);
}

/* whether the window is fullscreen. SDL sets its flag when the window
manager confirms the request, and some never do: gamescope (the Steam
Deck's Game Mode) makes the window the size of the display but leaves the
flag unset, so the game drew 640x480 and gamescope stretched it. A window
that was asked to be fullscreen and covers its display counts too. */
static BOOL platform_window_fullscreen(void)
{
	SDL_DisplayID display;
	const SDL_DisplayMode *mode;
	int width, height;

	if (SDL_GetWindowFlags(platform_window) & SDL_WINDOW_FULLSCREEN)
		return TRUE;
	if (!platform_fullscreen_requested)
		return FALSE;
	display = SDL_GetDisplayForWindow(platform_window);
	mode = display ? SDL_GetDesktopDisplayMode(display) : NULL;
	if (!mode || !SDL_GetWindowSizeInPixels(platform_window, &width, &height))
		return FALSE;
	return width >= (int)(mode->w * mode->pixel_density + 0.5f) &&
		height >= (int)(mode->h * mode->pixel_density + 0.5f);
}

/* the size in pixels the game draws its picture at (d3d8_gl.c): the
window's, the display's while fullscreen, or display.resolution's while
fullscreen (either kind) where the display has room for it; before the
window opens, what it will be (d3d8_gl.c draws a part of it for
display.resolution_scaling's percentages). FALSE where it is "original": the
Xbox's 640x480, scaled to the window. */
BOOL platform_screen_mode(long *width, long *height)
{
	/* (the size last given, for a window that has none: minimized) */
	static long last_width, last_height;
	long resolution_width, resolution_height;
	BOOL fullscreen;

	if (!strcmp(config_string("display.resolution_scaling"), "original"))
		return FALSE;
	if (platform_window)
	{
		int pixel_width = 0, pixel_height = 0;

		SDL_GetWindowSizeInPixels(platform_window, &pixel_width, &pixel_height);
		if (pixel_width <= 0 || pixel_height <= 0)
		{
			*width = last_width;
			*height = last_height;
			return last_width > 0;
		}
		*width = pixel_width;
		*height = pixel_height;
		fullscreen = platform_window_fullscreen();
	}
	else
	{
		if (!platform_sdl_initialize())
			return FALSE;
		fullscreen = platform_fullscreen_setting();
		platform_window_size_setting(width, height);
		if (fullscreen && !platform_display_size(SDL_GetPrimaryDisplay(), width, height))
			return FALSE;
	}
	/* (fullscreen's display is at the resolution already, where it has that
	mode: platform_fullscreen_kind_apply; borderless's is scaled to) */
	platform_resolution_setting(&resolution_width, &resolution_height);
	if (fullscreen && resolution_width && resolution_width <= *width && resolution_height <= *height)
	{
		*width = resolution_width;
		*height = resolution_height;
	}
	last_width = *width;
	last_height = *height;
	return TRUE;
}

/* the size added to the list unless it has it already; the count */
static int platform_resolution_add(long *widths, long *heights, int count, int maximum, long width, long height)
{
	int index;

	for (index = 0; index < count; index++)
	{
		if (widths[index] == width && heights[index] == height)
			return count;
	}
	if (count < maximum)
	{
		widths[count] = width;
		heights[count] = height;
		count++;
	}
	return count;
}

int platform_display_resolutions(long *widths, long *heights, int maximum)
{
	SDL_DisplayID display;
	SDL_DisplayMode **modes;
	long display_width, display_height, width, height;
	int mode_count = 0, count = 0, index;

	if (maximum < 1 || !platform_sdl_initialize())
		return 0;
	display = platform_window ? SDL_GetDisplayForWindow(platform_window) : 0;
	if (!display)
		display = SDL_GetPrimaryDisplay();
	if (!platform_display_size(display, &display_width, &display_height))
		return 0;
	modes = SDL_GetFullscreenDisplayModes(display, &mode_count);
	for (index = 0; modes && index < mode_count; index++)
	{
		width = (long)(modes[index]->w * modes[index]->pixel_density + 0.5f);
		height = (long)(modes[index]->h * modes[index]->pixel_density + 0.5f);
		if (width >= 640 && height >= 480 && width <= display_width && height <= display_height &&
			(width != display_width || height != display_height))
		{
			count = platform_resolution_add(widths, heights, count, maximum, width, height);
		}
	}
	SDL_free(modes);
	/* (the one set, though this display has no such mode, so that Video
	Setup shows it) */
	platform_resolution_setting(&width, &height);
	if (width && (width != display_width || height != display_height))
		count = platform_resolution_add(widths, heights, count, maximum, width, height);
	/* largest first */
	for (index = 1; index < count; index++)
	{
		int place;

		width = widths[index];
		height = heights[index];
		for (place = index; place > 0 && (widths[place - 1] < width ||
			(widths[place - 1] == width && heights[place - 1] < height)); place--)
		{
			widths[place] = widths[place - 1];
			heights[place] = heights[place - 1];
		}
		widths[place] = width;
		heights[place] = height;
	}
	return count;
}

/* Video Setup's window sizes, by shape (4:3, 16:10, 16:9, 21:9), each from
the smallest */
static const short platform_window_sizes_offered[][2] =
{
	{ 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1280, 960 }, { 1600, 1200 }, { 1920, 1440 }, { 2560, 1920 },
	{ 1280, 800 }, { 1440, 900 }, { 1680, 1050 }, { 1920, 1200 }, { 2560, 1600 },
	{ 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }, { 3840, 2160 },
	{ 2560, 1080 }, { 3440, 1440 }, { 3840, 1600 }, { 5120, 2160 },
};

int platform_window_sizes(long *widths, long *heights, int maximum)
{
	SDL_DisplayID display;
	SDL_Rect usable;
	long width, height;
	int count = 0, index;

	if (maximum < 1 || !platform_sdl_initialize())
		return 0;
	display = platform_window ? SDL_GetDisplayForWindow(platform_window) : 0;
	if (!display)
		display = SDL_GetPrimaryDisplay();
	if (!display || !SDL_GetDisplayUsableBounds(display, &usable))
		usable.w = usable.h = 0;
	for (index = 0; index < (int)(sizeof(platform_window_sizes_offered) / sizeof(*platform_window_sizes_offered));
		index++)
	{
		width = platform_window_sizes_offered[index][0];
		height = platform_window_sizes_offered[index][1];
		/* (the Xbox's own whatever the desktop's size) */
		if (!index || (width <= usable.w && height <= usable.h))
			count = platform_resolution_add(widths, heights, count, maximum, width, height);
	}
	/* (the one set, though it is none of them or too big for this desktop,
	so that Video Setup shows it) */
	platform_window_size_setting(&width, &height);
	return platform_resolution_add(widths, heights, count, maximum, width, height);
}

#else
int platform_display_resolutions(long *widths, long *heights, int maximum)
{
	(void)widths;
	(void)heights;
	(void)maximum;
	return 0;
}

int platform_window_sizes(long *widths, long *heights, int maximum)
{
	(void)widths;
	(void)heights;
	(void)maximum;
	return 0;
}

#endif

/* ---------- audio devices (Settings > Audio: audio.output_device,
audio.input_device) */

#ifndef HALO_ANDROID
int platform_audio_devices(int recording, char (*names)[PLATFORM_AUDIO_DEVICE_NAME_SIZE], int maximum)
{
	SDL_AudioDeviceID *devices;
	int device_count = 0, count = 0, index;

	if (maximum < 1 || !platform_sdl_initialize())
		return 0;
	devices = recording ? SDL_GetAudioRecordingDevices(&device_count) : SDL_GetAudioPlaybackDevices(&device_count);
	for (index = 0; devices && index < device_count && count < maximum; index++)
	{
		const char *name = SDL_GetAudioDeviceName(devices[index]);

		/* (a name a setting can hold, and a menu show: no "|", which
		separates a spinner's values) */
		if (!name || !name[0] || strchr(name, '|') || strlen(name) >= PLATFORM_AUDIO_DEVICE_NAME_SIZE)
			continue;
		snprintf(names[count++], PLATFORM_AUDIO_DEVICE_NAME_SIZE, "%s", name);
	}
	SDL_free(devices);
	return count;
}

SDL_AudioDeviceID platform_audio_device(int recording, const char *name)
{
	SDL_AudioDeviceID *devices;
	SDL_AudioDeviceID found = recording ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
	int device_count = 0, index;

	if (!name || !name[0] || !strcmp(name, "default"))
		return found;
	devices = recording ? SDL_GetAudioRecordingDevices(&device_count) : SDL_GetAudioPlaybackDevices(&device_count);
	for (index = 0; devices && index < device_count; index++)
	{
		const char *device_name = SDL_GetAudioDeviceName(devices[index]);

		if (device_name && !strcmp(device_name, name))
		{
			found = devices[index];
			break;
		}
	}
	SDL_free(devices);
	if (found == (recording ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK))
		platform_log("audio: no %s device named \"%s\": the system's default", recording ? "input" : "output", name);
	return found;
}
#else
int platform_audio_devices(int recording, char (*names)[PLATFORM_AUDIO_DEVICE_NAME_SIZE], int maximum)
{
	(void)recording;
	(void)names;
	(void)maximum;
	return 0;
}

SDL_AudioDeviceID platform_audio_device(int recording, const char *name)
{
	(void)name;
	return recording ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
}
#endif

#ifndef HALO_ANDROID
/* the window's size (platform_window_size_setting), as the window was made
or last resized: platform_display_apply */
static long platform_window_width = -1, platform_window_height = -1;
/* the last configured mode; F11 can change the window independently */
static int platform_window_mode = _display_mode_windowed;

/* display.hdr_output: the window's pixels are half floats (scRGB), -1 until
the window is first made; FALSE once the driver has none */
static int platform_float_window = -1;

/* the attributes of a window with (or without) floating-point pixels */
static void platform_float_window_attributes(BOOL floating)
{
	int size = floating ? 16 : 8;

	SDL_GL_SetAttribute(SDL_GL_FLOATBUFFERS, floating ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, size);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, size);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, size);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, size);
}

/* a window with floating-point pixels that cannot be made, or have a
context made for it: one without them (on Windows a window's pixel format
is set once, so a new window) */
static BOOL platform_float_window_refused(void)
{
	if (platform_float_window != TRUE)
		return FALSE;
	platform_log("HDR output: no floating-point window (%s); the window is SDR", SDL_GetError());
	platform_float_window = FALSE;
	platform_float_window_attributes(FALSE);
	if (platform_window)
		SDL_DestroyWindow(platform_window);
	platform_window = NULL;
	return TRUE;
}

BOOL platform_video_hdr(float *sdr_white, float *headroom)
{
	SDL_PropertiesID properties = platform_window ? SDL_GetWindowProperties(platform_window) : 0;
	BOOL enabled = properties && SDL_GetBooleanProperty(properties, SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN, false);

	*sdr_white = properties ? SDL_GetFloatProperty(properties, SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT, 1.0f) : 1.0f;
	*headroom = properties ? SDL_GetFloatProperty(properties, SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT, 1.0f) : 1.0f;
	if (!(*sdr_white > 0.0f))
		*sdr_white = 1.0f;
	if (!(*headroom >= 1.0f))
		*headroom = 1.0f;
	return enabled;
}
#endif

#ifndef HALO_ANDROID
#ifndef HALO_GLES
/* ---------- the menus' cursor

The blue arrow the menus show for the mouse, in place of the system's. Halo
PC drew its cursor in the menus' 640 x 480, so it took the same part of the
screen at any resolution; this one is made again whenever the window's size
in pixels or display.menu_cursor changes (menu_cursor_fit, from
platform_pump_events). */

/* the arrow's tip in the picture, past its soft edge: the click point */
#define MENU_CURSOR_TIP 2

static SDL_Surface *menu_cursor_picture;
static SDL_Cursor *menu_cursor;
static int menu_cursor_height;

/* the picture at a height, halved first while it is twice that or more (one
linear scaling to less than half the size skips pixels) */
static SDL_Surface *menu_cursor_scale(int height)
{
	int width = (menu_cursor_picture->w * height + menu_cursor_picture->h / 2) / menu_cursor_picture->h;
	SDL_Surface *surface = SDL_DuplicateSurface(menu_cursor_picture);

	while (surface && surface->h / 2 >= height)
	{
		SDL_Surface *half = SDL_ScaleSurface(surface, surface->w / 2, surface->h / 2, SDL_SCALEMODE_LINEAR);

		SDL_DestroySurface(surface);
		surface = half;
	}
	if (surface && surface->h != height)
	{
		SDL_Surface *sized = SDL_ScaleSurface(surface, width > 0 ? width : 1, height, SDL_SCALEMODE_LINEAR);

		SDL_DestroySurface(surface);
		surface = sized;
	}
	return surface;
}

/* display.menu_cursor: the arrow's height in the menus' 480 lines (large,
the default, Halo PC's: 72 pixels at 1080p), or 0 for the system's pointer;
read again when a setting changes */
static int menu_cursor_size(void)
{
	static const struct { const char *name; int lines; } sizes[] =
	{
		{ "off", 0 }, { "small", 18 }, { "medium", 24 }, { "large", 32 }, { "extra_large", 40 }
	};
	static unsigned long read_at = (unsigned long)-1;
	static int lines = 32;

	if (read_at != config_changes())
	{
		const char *name = config_string("display.menu_cursor");
		size_t index;

		read_at = config_changes();
		lines = 32;
		for (index = 0; index < sizeof(sizes) / sizeof(sizes[0]); index++)
			if (!strcmp(name, sizes[index].name))
				lines = sizes[index].lines;
	}
	return lines;
}

/* the cursor made for the window's size and display.menu_cursor, when one
has changed: its height the part of the window the menus' 480 lines take
(their 4:3 fits a window of another shape), in pixels; where the desktop
counts in points (macOS, Wayland at a scale) the cursor is that size in
points, with the pixels' image as its alternate. Off, the system's pointer
again. */
static void menu_cursor_fit(void)
{
	int width = 0, height = 0, lines, cursor_height, size = menu_cursor_size();
	float density;
	SDL_Surface *pixels, *points;
	SDL_Cursor *cursor = NULL;

	if (!menu_cursor_picture || !SDL_GetWindowSizeInPixels(platform_window, &width, &height) ||
		width <= 0 || height <= 0)
		return;
	if (!size)
	{
		if (menu_cursor)
		{
			SDL_SetCursor(SDL_GetDefaultCursor());
			SDL_DestroyCursor(menu_cursor);
			menu_cursor = NULL;
		}
		menu_cursor_height = 0;
		return;
	}
	lines = height < width * 3 / 4 ? height : width * 3 / 4;
	cursor_height = lines * size / 480;
	if (cursor_height < 16)
		cursor_height = 16;
	if (cursor_height == menu_cursor_height)
		return;
	menu_cursor_height = cursor_height;
	density = SDL_GetWindowPixelDensity(platform_window);
	pixels = menu_cursor_scale(cursor_height);
	points = density > 1.0f ? menu_cursor_scale((int)(cursor_height / density + 0.5f)) : pixels;
	if (pixels && points && (points == pixels || SDL_AddSurfaceAlternateImage(points, pixels)))
		cursor = SDL_CreateColorCursor(points,
			(MENU_CURSOR_TIP * points->w + menu_cursor_picture->w / 2) / menu_cursor_picture->w,
			(MENU_CURSOR_TIP * points->h + menu_cursor_picture->h / 2) / menu_cursor_picture->h);
	if (cursor && SDL_SetCursor(cursor))
	{
		SDL_DestroyCursor(menu_cursor);
		menu_cursor = cursor;
	}
	else
	{
		platform_log("cannot set the menus' cursor: %s", SDL_GetError());
		if (cursor)
			SDL_DestroyCursor(cursor);
	}
	if (points != pixels)
		SDL_DestroySurface(points);
	SDL_DestroySurface(pixels);
}

/* A controller's buttons or sticks, used while a menu is up, hide the
cursor, and the mouse moved or clicked shows it again, so a player on a
controller has no arrow left over the menus; a menu opened while the
controller was used last opens without it
(platform_ui_pointer_set_active). */
static BOOL menu_cursor_controller;

static void menu_cursor_controller_used(BOOL in_menus)
{
	menu_cursor_controller = TRUE;
	if (in_menus && SDL_CursorVisible())
		SDL_HideCursor();
}

static void menu_cursor_mouse_used(BOOL in_menus)
{
	menu_cursor_controller = FALSE;
	if (in_menus && !SDL_CursorVisible())
		SDL_ShowCursor();
}
#endif
#endif

/* the swap interval set, each change only (on the desktop
platform_video_swap's: vsync while focused, none unfocused, its frames
paced at the refresh instead) */
static int platform_swap_interval_applied = -1;

static void platform_swap_interval(int interval)
{
	if (platform_swap_interval_applied == interval)
		return;
	SDL_GL_SetSwapInterval(interval);
	platform_swap_interval_applied = interval;
}

BOOL platform_video_initialize(unsigned long width, unsigned long height)
{
	if (platform_window)
		return TRUE;
	platform_video_tried = TRUE;
	if (!platform_sdl_initialize())
		return FALSE;
#ifndef HALO_ANDROID
#ifdef HALO_WEB
	/* (a browser's canvas has 8 bits a channel) */
	platform_float_window = FALSE;
#else
	if (platform_float_window < 0)
		platform_float_window = config_boolean("display.hdr") && config_boolean("display.hdr_output");
#endif
	if (platform_float_window)
		platform_float_window_attributes(TRUE);
#endif

#ifdef HALO_GLES
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
#ifdef HALO_WEB
	/* an opaque canvas: the game keeps scratch values in destination alpha,
	which a page would otherwise blend the picture by (black, or the page's
	colours through it) */
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
#endif
	if (config_boolean("debug.gl_debug"))
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#if !defined(HALO_ANDROID) && !defined(_WIN32)
	/* Mesa's GL thread: the renderer makes thousands of GL calls a frame
	and never waits for their results, so handing them to a thread of
	their own takes a fifth of the main thread's time off it. It leaves an
	explicit mesa_glthread setting alone and other drivers ignore it. */
	setenv("mesa_glthread", "true", 0);
#endif

#ifdef HALO_ANDROID
	{
		int scale = (int)config_integer("display.window_scale");

		if (scale < 1)
			scale = 1;
		platform_window = SDL_CreateWindow("Halo", (int)(width * scale), (int)(height * scale),
			SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
	}
#else
	/* fullscreen (either kind) unless display.mode is the window, which F11
	switches to and from: display.window_size, whatever shape the fullscreen
	picture has. The game draws at the size platform_screen_mode gives
	(d3d8_gl.c). */
	(void)width;
	(void)height;
	platform_window_size_setting(&platform_window_width, &platform_window_height);
	platform_window = SDL_CreateWindow("Halo", (int)platform_window_width, (int)platform_window_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY |
		(config_boolean("debug.hidden_window") ? SDL_WINDOW_HIDDEN : 0) |
		(platform_fullscreen_setting() ? SDL_WINDOW_FULLSCREEN : 0));
#endif
	if (!platform_window)
	{
#ifndef HALO_ANDROID
		if (platform_float_window_refused())
			return platform_video_initialize(width, height);
#endif
		platform_log("SDL_CreateWindow failed: %s", SDL_GetError());
		return FALSE;
	}
#ifndef HALO_ANDROID
#ifndef HALO_GLES
	/* the game's icon, which the desktop shows for the window (on Windows
	also halo.exe's own, port/windows/halo.rc; a web page has its own) */
	if (platform_window_icon_size)
	{
		SDL_Surface *icon = SDL_LoadPNG_IO(SDL_IOFromConstMem(platform_window_icon, platform_window_icon_size), true);

		if (!icon || !SDL_SetWindowIcon(platform_window, icon))
			platform_log("cannot set the window's icon: %s", SDL_GetError());
		SDL_DestroySurface(icon);
	}
	/* the pointer the menus show (platform_ui_pointer_set_active), in place
	of the system's, at the window's size */
	if (platform_menu_cursor_size && !menu_cursor_picture)
	{
		menu_cursor_picture = SDL_LoadPNG_IO(SDL_IOFromConstMem(platform_menu_cursor, platform_menu_cursor_size), true);
		if (!menu_cursor_picture)
			platform_log("cannot read the menus' cursor: %s", SDL_GetError());
	}
	menu_cursor_height = 0;
	menu_cursor_fit();
#endif
	platform_fullscreen_requested = platform_fullscreen_setting();
	platform_window_mode = platform_display_mode();
	platform_fullscreen_kind_apply();
#endif
#ifdef HALO_WINDOWS
	win32_window_borderless_apply(platform_window);
#endif
	platform_gl_context = SDL_GL_CreateContext(platform_window);
#ifdef HALO_GLES
	/* ES 3.2 where the driver has it, otherwise the renderer makes do with
	3.0 plus extensions */
	if (!platform_gl_context)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		platform_gl_context = SDL_GL_CreateContext(platform_window);
	}
#endif
#ifndef HALO_ANDROID
	if (!platform_gl_context && platform_float_window_refused())
		return platform_video_initialize(width, height);
#endif
	if (!platform_gl_context)
	{
		platform_log("cannot create an OpenGL context: %s", SDL_GetError());
		return FALSE;
	}
	SDL_GL_MakeCurrent(platform_window, platform_gl_context);
	if (!gl_functions_load())
		return FALSE;
	platform_swap_interval(config_boolean("display.vsync") ? 1 : 0);
	platform_event_thread = SDL_GetCurrentThreadID();
	platform_log("OpenGL %s on %s", (const char *)glGetString(GL_VERSION), (const char *)glGetString(GL_RENDERER));
#if defined(__linux__) && !defined(HALO_ANDROID)
	{
		const char *renderer = (const char *)glGetString(GL_RENDERER);

		/* (Mesa's software renderers: no GPU driver for this window) */
		if (renderer && (strstr(renderer, "llvmpipe") || strstr(renderer, "softpipe")))
			platform_log("OpenGL is drawn on the processor, not the GPU (video driver %s): "
				"refer to \"NVIDIA on Wayland\" in port/linux/README.md", SDL_GetCurrentVideoDriver());
	}
#endif
#ifndef HALO_ANDROID
	{
		float sdr_white, headroom;
		BOOL hdr = platform_video_hdr(&sdr_white, &headroom);

		platform_log("display: HDR %s (SDR white %.2f, headroom %.2f), %s window", hdr ? "on" : "off", sdr_white,
			headroom, platform_float_window ? "floating-point" : "8-bit");
	}
	platform_mouse_capture(TRUE);
#endif
	return TRUE;
}

/* display.mode, display.resolution (fullscreen's display mode),
display.window_size (when it changes: the window can be resized) and
display.vsync, as Settings has written them; display.resolution_scaling
and borderless's resolution are taken up between frames
(halo_screen_commit) */
void platform_display_apply(void)
{
#ifndef HALO_ANDROID
	int mode = platform_display_mode();
	long width, height;

	if (!platform_window)
		return;
	platform_fullscreen_kind_apply();
	/* Saving unrelated settings must preserve the window mode chosen by F11. */
	if (mode != platform_window_mode)
	{
		platform_window_mode = mode;
		platform_window_set_fullscreen(platform_fullscreen_setting());
	}
	platform_window_size_setting(&width, &height);
	if (width != platform_window_width || height != platform_window_height)
	{
		platform_window_width = width;
		platform_window_height = height;
		SDL_SetWindowSize(platform_window, (int)width, (int)height);
	}
#else
	if (!platform_window)
		return;
#endif
	platform_swap_interval(config_boolean("display.vsync") ? 1 : 0);
}

void platform_video_drawable_size(int *width, int *height)
{
	SDL_GetWindowSizeInPixels(platform_window, width, height);
}

/* with vsync off, the time between frames display.max_fps asks for (0:
twice the display's refresh rate), or 0 for no limit. A desktop GPU never
left idle can hang (Intel's Raptor Lake graphics, whose reset then takes
the desktop with it); the limit gives it a rest every frame. */
static Uint64 frame_interval_ns(void)
{
	static int vsync;
	static long maximum;
	static unsigned long read_at = (unsigned long)-1;
	float rate;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		vsync = config_boolean("display.vsync");
		maximum = config_integer("display.max_fps");
	}
	if (vsync || maximum < 0)
		return 0;
	rate = (float)maximum;
	if (!maximum)
	{
#ifdef HALO_ANDROID
		/* the app passes the display's refresh rate (port/android/host/host_main.c):
		the guest cannot ask SDL for the display mode */
		const char *refresh = getenv("HALO_DISPLAY_REFRESH");

		rate = 2.0f * (refresh && atof(refresh) > 0.0f ? (float)atof(refresh) : 60.0f);
#else
		SDL_DisplayID display = SDL_GetDisplayForWindow(platform_window);
		const SDL_DisplayMode *mode = display ? SDL_GetCurrentDisplayMode(display) : NULL;

		rate = 2.0f * (mode && mode->refresh_rate > 0.0f ? mode->refresh_rate : 60.0f);
#endif
	}
	return (Uint64)(1e9f / rate);
}

/* (the swap's sound thread and the unfocused window's pacing: the desktop's
alone) */
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
static Uint64 refresh_period_ns(void)
{
	SDL_DisplayID display;
	const SDL_DisplayMode *mode;
	float rate;

	display = platform_window ? SDL_GetDisplayForWindow(platform_window) : 0;
	mode = display ? SDL_GetCurrentDisplayMode(display) : NULL;
	rate = mode && mode->refresh_rate > 0.0f ? mode->refresh_rate : 60.0f;
	return (Uint64)(1e9f / rate);
}

static pthread_mutex_t swap_pump_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t swap_pump_cv = PTHREAD_COND_INITIALIZER;
static pthread_t swap_sound_thread_id;
static int swap_pump_started;
static int swap_sound_thread_live;
static int swap_blocking;
static int swap_sound_busy;
static Uint64 swap_blocking_since;

void __stdcall DirectSoundDoWork(void);

static void *swap_sound_thread(void *parameter)
{
	struct timespec when;

	(void)parameter;
	for (;;)
	{
		pthread_mutex_lock(&swap_pump_mutex);
		while (!swap_blocking)
			pthread_cond_wait(&swap_pump_cv, &swap_pump_mutex);
		pthread_mutex_unlock(&swap_pump_mutex);

		clock_gettime(CLOCK_REALTIME, &when);
		when.tv_nsec += 8000000L;
		if (when.tv_nsec >= 1000000000L)
		{
			when.tv_nsec -= 1000000000L;
			when.tv_sec++;
		}

		pthread_mutex_lock(&swap_pump_mutex);
		pthread_cond_timedwait(&swap_pump_cv, &swap_pump_mutex, &when);
		if (!swap_blocking || SDL_GetTicks() - swap_blocking_since < 20)
		{
			pthread_mutex_unlock(&swap_pump_mutex);
			continue;
		}
		swap_sound_busy = 1;
		pthread_mutex_unlock(&swap_pump_mutex);

		DirectSoundDoWork();

		pthread_mutex_lock(&swap_pump_mutex);
		swap_sound_busy = 0;
		pthread_cond_broadcast(&swap_pump_cv);
		pthread_mutex_unlock(&swap_pump_mutex);
	}
	return NULL;
}

static void swap_sound_begin(void)
{
	pthread_t thread;

	if (swap_sound_thread_live && pthread_equal(pthread_self(), swap_sound_thread_id))
		return;
	pthread_mutex_lock(&swap_pump_mutex);
	if (!swap_pump_started)
	{
		if (pthread_create(&thread, NULL, swap_sound_thread, NULL) != 0)
		{
			pthread_mutex_unlock(&swap_pump_mutex);
			return;
		}
		pthread_detach(thread);
		swap_sound_thread_id = thread;
		swap_sound_thread_live = 1;
		swap_pump_started = 1;
	}
	swap_blocking = 1;
	swap_blocking_since = SDL_GetTicks();
	pthread_cond_signal(&swap_pump_cv);
	pthread_mutex_unlock(&swap_pump_mutex);
}

static void swap_sound_end(void)
{
	if (!swap_pump_started)
		return;
	if (swap_sound_thread_live && pthread_equal(pthread_self(), swap_sound_thread_id))
		return;
	pthread_mutex_lock(&swap_pump_mutex);
	swap_blocking = 0;
	pthread_cond_broadcast(&swap_pump_cv);
	while (swap_sound_busy)
		pthread_cond_wait(&swap_pump_cv, &swap_pump_mutex);
	pthread_mutex_unlock(&swap_pump_mutex);
}

#endif
#ifdef HALO_WEB
#include "web_shared.h"

/* port/web/src/web_main.c's: the frames shown, which the page reads; and
web_library.js's: a line for the page (2: an invite link to offer) */
void web_frame_shown(void);
void web_js_post(int kind, const char *text);
#endif

#ifdef HALO_ANDROID
/* monotonic nanoseconds, and a sleep of that long: the guest's SDL bridge has
no SDL_GetTicksNS nor SDL_DelayPrecise, so use musl's clocks */
static Uint64 platform_ticks_ns(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (Uint64)now.tv_sec * 1000000000ull + (Uint64)now.tv_nsec;
}

static void platform_sleep_ns(Uint64 duration)
{
	struct timespec request, remaining;

	request.tv_sec = (time_t)(duration / 1000000000ull);
	request.tv_nsec = (long)(duration % 1000000000ull);
	while (nanosleep(&request, &remaining) == -1 && errno == EINTR)
		request = remaining;
}
#endif

void platform_video_swap(void)
{
	Uint64 swap_time;
	static Uint64 next_frame;
	Uint64 interval, now;
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	SDL_WindowFlags window_flags;
	int focused;
	int vsync;
	Uint64 swap_start;
#endif
#ifdef HALO_WINDOWS
	/* SDL can restore its fullscreen styles on a mode or focus change.
	Keep borderless on the desktop before the driver presents a frame. */
	win32_window_borderless_apply(platform_window);
#endif
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	SDL_PumpEvents();
	window_flags = platform_window ? SDL_GetWindowFlags(platform_window) : 0;
	focused = platform_window && (window_flags & SDL_WINDOW_INPUT_FOCUS) != 0 &&
		(window_flags & SDL_WINDOW_MINIMIZED) == 0;
	vsync = config_boolean("display.vsync");
	platform_swap_interval((vsync && focused) ? 1 : 0);
	swap_sound_begin();
	swap_start = SDL_GetTicks();
	SDL_GL_SwapWindow(platform_window);
	swap_time = SDL_GetTicks() - swap_start;
	swap_sound_end();
	if (swap_time >= 2000)
		platform_log("display: showing a frame took %.1f s in the graphics driver", (double)swap_time / 1000.0);
#else
	{
	Uint64 swap_start;

	swap_start = SDL_GetTicks();
	SDL_GL_SwapWindow(platform_window);
	/* (a frame the graphics driver held for seconds: the game froze, and
	online, a host stops hearing a machine after a few; the log says where) */
	swap_time = SDL_GetTicks() - swap_start;
	if (swap_time >= 2000)
		platform_log("display: showing a frame took %.1f s in the graphics driver", (double)swap_time / 1000.0);
	}
#endif
#ifdef HALO_WEB
	web_frame_shown();
#endif
	interval = frame_interval_ns();
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	if (!interval && vsync && !focused)
		interval = refresh_period_ns();
#endif
	if (!interval)
		return;
#ifdef HALO_ANDROID
	now = platform_ticks_ns();
#else
	now = SDL_GetTicksNS();
#endif
	if (next_frame > now)
	{
#ifdef HALO_ANDROID
		platform_sleep_ns(next_frame - now);
#else
		SDL_DelayPrecise(next_frame - now);
#endif
		now = next_frame;
	}
	/* (a frame more than an interval late starts the count again) */
	next_frame = now - next_frame > interval ? now + interval : next_frame + interval;
}

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* when SDL_SetWindowRelativeMouseMode has no XInput2 (32-bit libSDL3 on X11
without 32-bit libXi.so.6), the mouse is held with a hidden cursor, a window
grab and a warp back to the middle after each move */
static BOOL mouse_warp_capture;
static int mouse_warp_x;
static int mouse_warp_y;
#endif

void platform_mouse_capture(BOOL capture)
{
#ifdef HALO_WEB
	/* (for the page, which takes the pointer lock back on a click:
	web_shared.h) */
	__atomic_store_n(&web_shared[_web_mouse_wanted], capture ? 1 : 0, __ATOMIC_RELAXED);
#endif
	if (!platform_window)
		return;
#if defined(HALO_ANDROID) || defined(HALO_WEB)
	SDL_SetWindowRelativeMouseMode(platform_window, capture ? true : false);
#else
	/* (without the focus, the mouse is taken when the focus comes back:
	SDL_EVENT_WINDOW_FOCUS_GAINED. SDL_SetWindowRelativeMouseMode would only
	set SDL_WINDOW_MOUSE_RELATIVE_MODE then, without testing XInput2, and
	later calls would find it set and return at once) */
	if (!capture || SDL_GetKeyboardFocus() != platform_window)
	{
		mouse_warp_capture = FALSE;
		SDL_SetWindowRelativeMouseMode(platform_window, false);
		SDL_SetWindowMouseGrab(platform_window, false);
		SDL_ShowCursor();
		return;
	}
	if (SDL_SetWindowRelativeMouseMode(platform_window, true))
	{
		if (mouse_warp_capture)
		{
			mouse_warp_capture = FALSE;
			SDL_SetWindowMouseGrab(platform_window, false);
			SDL_ShowCursor();
		}
		return;
	}
	mouse_warp_capture = TRUE;
	SDL_HideCursor();
	SDL_SetWindowMouseGrab(platform_window, true);
	{
		int width = 0, height = 0;

		SDL_GetWindowSize(platform_window, &width, &height);
		if (width > 0 && height > 0)
		{
			mouse_warp_x = width / 2;
			mouse_warp_y = height / 2;
			SDL_WarpMouseInWindow(platform_window, (float)mouse_warp_x, (float)mouse_warp_y);
		}
	}
#endif
}

/* ---------- keyboard translation */

/* Windows virtual key code for an SDL scancode (the Xbox debug keyboard
reports virtual keys) */
static BYTE virtual_key_from_scancode(SDL_Scancode scancode)
{
	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
		return (BYTE)('A' + (scancode - SDL_SCANCODE_A));
	if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
		return (BYTE)('1' + (scancode - SDL_SCANCODE_1));
	if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F12)
		return (BYTE)(0x70 + (scancode - SDL_SCANCODE_F1));
	if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_9)
		return (BYTE)(0x61 + (scancode - SDL_SCANCODE_KP_1));
	switch (scancode)
	{
	case SDL_SCANCODE_0: return '0';
	case SDL_SCANCODE_KP_0: return 0x60;
	case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: return 0x0d;
	case SDL_SCANCODE_ESCAPE: return 0x1b;
	case SDL_SCANCODE_BACKSPACE: return 0x08;
	case SDL_SCANCODE_TAB: return 0x09;
	case SDL_SCANCODE_SPACE: return 0x20;
	case SDL_SCANCODE_MINUS: return 0xbd;
	case SDL_SCANCODE_EQUALS: return 0xbb;
	case SDL_SCANCODE_LEFTBRACKET: return 0xdb;
	case SDL_SCANCODE_RIGHTBRACKET: return 0xdd;
	case SDL_SCANCODE_BACKSLASH: return 0xdc;
	case SDL_SCANCODE_SEMICOLON: return 0xba;
	case SDL_SCANCODE_APOSTROPHE: return 0xde;
	case SDL_SCANCODE_GRAVE: return 0xc0;
	case SDL_SCANCODE_COMMA: return 0xbc;
	case SDL_SCANCODE_PERIOD: return 0xbe;
	case SDL_SCANCODE_SLASH: return 0xbf;
	case SDL_SCANCODE_CAPSLOCK: return 0x14;
	case SDL_SCANCODE_PRINTSCREEN: return 0x2c;
	case SDL_SCANCODE_SCROLLLOCK: return 0x91;
	case SDL_SCANCODE_PAUSE: return 0x13;
	case SDL_SCANCODE_INSERT: return 0x2d;
	case SDL_SCANCODE_HOME: return 0x24;
	case SDL_SCANCODE_PAGEUP: return 0x21;
	case SDL_SCANCODE_DELETE: return 0x2e;
	case SDL_SCANCODE_END: return 0x23;
	case SDL_SCANCODE_PAGEDOWN: return 0x22;
	case SDL_SCANCODE_RIGHT: return 0x27;
	case SDL_SCANCODE_LEFT: return 0x25;
	case SDL_SCANCODE_DOWN: return 0x28;
	case SDL_SCANCODE_UP: return 0x26;
	case SDL_SCANCODE_NUMLOCKCLEAR: return 0x90;
	case SDL_SCANCODE_KP_DIVIDE: return 0x6f;
	case SDL_SCANCODE_KP_MULTIPLY: return 0x6a;
	case SDL_SCANCODE_KP_MINUS: return 0x6d;
	case SDL_SCANCODE_KP_PLUS: return 0x6b;
	case SDL_SCANCODE_KP_PERIOD: return 0x6e;
	case SDL_SCANCODE_LCTRL: return 0xa2;
	case SDL_SCANCODE_RCTRL: return 0xa3;
	case SDL_SCANCODE_LSHIFT: return 0xa0;
	case SDL_SCANCODE_RSHIFT: return 0xa1;
	case SDL_SCANCODE_LALT: return 0xa4;
	case SDL_SCANCODE_RALT: return 0xa5;
	default: return 0;
	}
}

static CHAR ascii_from_key(SDL_Keycode key, SDL_Keymod modifiers)
{
	BOOL shift = (modifiers & SDL_KMOD_SHIFT) != 0;
	static const char shifted_digits[] = ")!@#$%^&*(";

	if (key >= 'a' && key <= 'z')
		return (CHAR)((shift ^ ((modifiers & SDL_KMOD_CAPS) != 0)) ? key - 32 : key);
	if (key >= '0' && key <= '9')
		return (CHAR)(shift ? shifted_digits[key - '0'] : key);
	if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
		return '\r';
	if (key == SDLK_BACKSPACE)
		return '\b';
	if (key == SDLK_TAB)
		return '\t';
	if (key == SDLK_ESCAPE)
		return 0x1b;
	if (key >= 32 && key < 127)
	{
		if (!shift)
			return (CHAR)key;
		switch (key)
		{
		case '-': return '_';
		case '=': return '+';
		case '[': return '{';
		case ']': return '}';
		case '\\': return '|';
		case ';': return ':';
		case '\'': return '"';
		case ',': return '<';
		case '.': return '>';
		case '/': return '?';
		case '`': return '~';
		default: return (CHAR)key;
		}
	}
	return 0;
}

static void queue_keystroke(const SDL_KeyboardEvent *event)
{
	struct platform_keystroke *keystroke;
	BYTE flags = 0;

	if (keystroke_count == KEYSTROKE_QUEUE_SIZE)
	{
		keystroke_head = (keystroke_head + 1) % KEYSTROKE_QUEUE_SIZE;
		keystroke_count--;
	}
	keystroke = &keystroke_queue[(keystroke_head + keystroke_count) % KEYSTROKE_QUEUE_SIZE];
	if (event->mod & SDL_KMOD_CTRL) flags |= 0x01;
	if (event->mod & SDL_KMOD_SHIFT) flags |= 0x02;
	if (event->mod & SDL_KMOD_ALT) flags |= 0x04;
	if (event->mod & SDL_KMOD_CAPS) flags |= 0x08;
	if (event->mod & SDL_KMOD_NUM) flags |= 0x10;
	if (!event->down) flags |= 0x40;
	if (event->repeat) flags |= 0x80;
	keystroke->virtual_key = virtual_key_from_scancode(event->scancode);
	keystroke->ascii = event->down ? ascii_from_key(event->key, event->mod) : 0;
	keystroke->flags = flags;
	keystroke_count++;
}

BOOL platform_next_keystroke(struct platform_keystroke *keystroke)
{
	BOOL result = FALSE;

	pthread_mutex_lock(&input_lock);
	if (keystroke_count)
	{
		*keystroke = keystroke_queue[keystroke_head];
		keystroke_head = (keystroke_head + 1) % KEYSTROKE_QUEUE_SIZE;
		keystroke_count--;
		result = TRUE;
	}
	pthread_mutex_unlock(&input_lock);
	return result;
}

/* ---------- internet play's invite links (p2p.c) */

#ifdef HALO_ANDROID
/* SDL declares it for Android builds only, which the guest is not
(guest/runtime/guest_sdl.c passes it to the host) */
bool SDL_ShowAndroidToast(const char *message, int duration, int gravity, int xoffset, int yoffset);
#endif

/* whether the text has an invite link in it (its prefix, in any case) */
static BOOL platform_text_has_invite_link(const char *text)
{
	static const char prefix[] = "halo://join/";
	size_t length = sizeof(prefix) - 1;

	for (; *text; text++)
	{
		size_t index;

		for (index = 0; index < length && text[index] &&
			(text[index] | 0x20) == prefix[index]; index++)
		{
		}
		if (index == length)
			return TRUE;
	}
	return FALSE;
}

/* the clipboard's text (the menus' text fields' Ctrl+V), and text put on it
(the server settings' invite link); 0 if there is none. The main thread's */
int platform_clipboard_get(char *text, int size)
{
	char *clipboard = SDL_GetClipboardText();
	int got = clipboard && *clipboard;

	snprintf(text, (size_t)size, "%s", got ? clipboard : "");
	SDL_free(clipboard);
	return got;
}

void platform_clipboard_set(const char *text)
{
	SDL_SetClipboardText(text);
}

void platform_open_url(const char *url)
{
#ifdef HALO_WEB
	web_js_post(5, url);
#else
	/* (the address logged too, for a machine whose browser does not open) */
	if (!SDL_OpenURL(url))
		platform_log("could not open the browser (%s): open %s", SDL_GetError(), url);
#endif
}

/* puts a new invite on the clipboard, and joins one found there when the
game comes to the front */
static void platform_invite_clipboard(BOOL look)
{
	/* the last clipboard text looked at, so each invite is joined once */
	static char seen[256];
	const char *invite = p2p_take_clipboard_text();

	if (invite)
	{
#ifdef HALO_WEB
		/* (a page may write the clipboard only when it is clicked: the page
		shows the link, with a button that copies it) */
		web_js_post(2, invite);
#else
		SDL_SetClipboardText(invite);
		snprintf(seen, sizeof(seen), "%s", invite);
		platform_log("Internet play: the invite link is on the clipboard");
#endif
#ifdef HALO_ANDROID
		SDL_ShowAndroidToast("Hosting: the invite link is on the clipboard", 1, -1, 0, 0);
#endif
	}
	if (look && config_boolean("network.join_from_clipboard"))
	{
		char *text = SDL_GetClipboardText();

		if (text && strcmp(text, seen) && strlen(text) < sizeof(seen))
		{
			snprintf(seen, sizeof(seen), "%s", text);
			/* (a link, not a bare code: 64 hex digits alone are as often a
			checksum copied for something else) */
			if (platform_text_has_invite_link(text) && p2p_join_invite(text))
			{
#ifdef HALO_ANDROID
				SDL_ShowAndroidToast("Joining the invite on the clipboard", 1, -1, 0, 0);
#endif
			}
		}
		SDL_free(text);
	}
}

/* ---------- messages for the player */

/* a message waiting for the event pump to show it (on the window's thread,
between frames) */
static pthread_mutex_t platform_message_lock = PTHREAD_MUTEX_INITIALIZER;
static char platform_message_title[80];
static char platform_message_text[600];
static BOOL platform_message_pending;

/* shows the player a message in a box of its own (the network code's: a
host of another version), and logs it */
void platform_show_message(const char *title, const char *message)
{
	platform_log("%s: %s", title, message);
	/* (a run nobody watches: the log only) */
	if (config_boolean("debug.hidden_window") || config_boolean("debug.null_renderer"))
		return;
	pthread_mutex_lock(&platform_message_lock);
	snprintf(platform_message_title, sizeof(platform_message_title), "%s", title);
	snprintf(platform_message_text, sizeof(platform_message_text), "%s", message);
	/* (streamer mode's words hidden here too: a map's or a mod's name) */
	text_filter_utf8(platform_message_title);
	text_filter_utf8(platform_message_text);
	platform_message_pending = TRUE;
	pthread_mutex_unlock(&platform_message_lock);
}

#ifdef HALO_WEB
/* web_library.js's: the page asks, and writes the answer in web_shared's
_web_answer */
void web_js_ask(const char *title, const char *text, const char *labels);
#endif

int platform_ask(const char *title, const char *text, const char *const *labels, int count, int default_index)
{
	/* (streamer mode's words hidden here too: mods' names) */
	static char shown_title[128];
	static char shown_text[4096];
	static char shown_labels[3][128];
	const char *shown[3];
	int index;
#ifdef HALO_WEB
	char joined[512] = "";
#else
	SDL_MessageBoxButtonData buttons[3];
	SDL_MessageBoxData question;
	int answer = -1;
#endif

	if (count > 3)
		count = 3;
	snprintf(shown_title, sizeof(shown_title), "%s", title);
	snprintf(shown_text, sizeof(shown_text), "%s", text);
	text_filter_utf8(shown_title);
	text_filter_utf8(shown_text);
	for (index = 0; index < count; index++)
	{
		snprintf(shown_labels[index], sizeof(shown_labels[index]), "%s", labels[index]);
		text_filter_utf8(shown_labels[index]);
		shown[index] = shown_labels[index];
	}
	title = shown_title;
	text = shown_text;
	labels = shown;
#ifdef HALO_WEB
	/* (the page's dialog: the game's thread waits for the player there, as
	SDL's box would, which a page's worker cannot show) */
	for (index = 0; index < count; index++)
	{
		size_t length = strlen(joined);

		snprintf(joined + length, sizeof(joined) - length, "%s%s", index ? "\n" : "", labels[index]);
	}
	__atomic_store_n(&web_shared[_web_answer], -1, __ATOMIC_SEQ_CST);
	web_js_ask(title, text, joined);
	while (__atomic_load_n(&web_shared[_web_answer], __ATOMIC_SEQ_CST) < 0)
		SDL_Delay(50);
	(void)default_index;
	return __atomic_load_n(&web_shared[_web_answer], __ATOMIC_SEQ_CST);
#else
	for (index = 0; index < count; index++)
	{
		buttons[index].flags = index == default_index ?
			SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT : 0;
		buttons[index].buttonID = index;
		buttons[index].text = labels[index];
	}
	question.flags = SDL_MESSAGEBOX_WARNING;
	question.window = NULL;
	question.title = title;
	question.message = text;
	question.numbuttons = count;
	question.buttons = buttons;
	question.colorScheme = NULL;
	if (!SDL_ShowMessageBox(&question, &answer))
		return -1;
	return answer;
#endif
}

static void platform_show_pending_message(void)
{
	char title[sizeof(platform_message_title)];
	char text[sizeof(platform_message_text)];
	BOOL pending;

	pthread_mutex_lock(&platform_message_lock);
	pending = platform_message_pending;
	platform_message_pending = FALSE;
	memcpy(title, platform_message_title, sizeof(title));
	memcpy(text, platform_message_text, sizeof(text));
	pthread_mutex_unlock(&platform_message_lock);
	if (!pending)
		return;
#ifdef HALO_ANDROID
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, text, NULL);
#else
	{
		/* (a box cannot show above a fullscreen game) */
		BOOL fullscreen = platform_window_fullscreen();

		if (fullscreen)
			platform_window_set_fullscreen(FALSE);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, text, platform_window);
		if (fullscreen)
			platform_window_set_fullscreen(TRUE);
	}
#endif
}

/* ---------- events */

/* quits as closing the window does, when the events are next read (the
menus' Quit: port/linux/game/menu_functions.c); on Android at once */
void platform_request_quit(void)
{
#ifdef HALO_ANDROID
	/* (the guest has no SDL_PushEvent: exit ends the process, host_exit) */
	exit(EXIT_SUCCESS);
#else
	SDL_Event event;

	memset(&event, 0, sizeof(event));
	event.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&event);
#endif
}

#ifndef HALO_ANDROID
/* (under input_lock, on the event thread) the scoreboard's pointer on: the
mouse freed, at the window's middle, and nothing held for the triggers */
static void scoreboard_pointer_start(void)
{
	int width, height;

	scoreboard_pointer_active = TRUE;
	memset(&scoreboard_pointer, 0, sizeof(scoreboard_pointer));
	memset(input_state.mouse_buttons, 0, sizeof(input_state.mouse_buttons));
	memset(mouse_buttons_pressed, 0, sizeof(mouse_buttons_pressed));
	input_state.mouse_dx = input_state.mouse_dy = 0.0f;
	platform_mouse_capture(FALSE);
	SDL_GetWindowSize(platform_window, &width, &height);
	SDL_WarpMouseInWindow(platform_window, width * 0.5f, height * 0.5f);
	scoreboard_pointer.x = width * 0.5f;
	scoreboard_pointer.y = height * 0.5f;
}

/* ... off: the mouse the aim's again (unless freed: F12, or the menus) */
static void scoreboard_pointer_stop(void)
{
	if (!scoreboard_pointer_active)
		return;
	scoreboard_pointer_active = FALSE;
	memset(&scoreboard_pointer, 0, sizeof(scoreboard_pointer));
	platform_mouse_capture(!input_state.mouse_released && !input_state.ui_pointer);
}

BOOL platform_scoreboard_pointer(BOOL offered, struct platform_ui_pointer *pointer)
{
	BOOL active;

	pthread_mutex_lock(&input_lock);
	scoreboard_pointer_offered = offered;
	active = scoreboard_pointer_active && offered;
	*pointer = scoreboard_pointer;
	scoreboard_pointer.moved = FALSE;
	scoreboard_pointer.left_clicks = 0;
	scoreboard_pointer.right_clicks = 0;
	scoreboard_pointer.wheel_steps = 0;
	pthread_mutex_unlock(&input_lock);
	return active;
}
#endif

void platform_scoreboard_scroll(int open, long *notches, long *pages)
{
	Uint64 now = SDL_GetTicks();

	pthread_mutex_lock(&input_lock);
	if (!open || now >= scoreboard_open_until_ms)
	{
		scoreboard_wheel = 0.0f;
		scoreboard_notches = 0;
		scoreboard_pages = 0;
	}
	scoreboard_open_until_ms = open ? now + SCOREBOARD_OPEN_MS : 0;
#ifndef HALO_ANDROID
	if (!open)
		scoreboard_pointer_offered = FALSE;
#endif
	if (notches)
		*notches = scoreboard_notches;
	if (pages)
		*pages = scoreboard_pages;
	scoreboard_notches = 0;
	scoreboard_pages = 0;
	pthread_mutex_unlock(&input_lock);
}

void platform_screenshot_request(void)
{
	pthread_mutex_lock(&input_lock);
	screenshot_requested = TRUE;
	pthread_mutex_unlock(&input_lock);
}

BOOL platform_screenshot_take_request(void)
{
	BOOL requested;

	pthread_mutex_lock(&input_lock);
	requested = screenshot_requested;
	screenshot_requested = FALSE;
	pthread_mutex_unlock(&input_lock);
	return requested;
}

/* debug.exit_after (seconds) ends the game that long after the window
opens (without one, after the first frame), as closing it does
(tools/pgo_train.py, net_compare.py) */
static void exit_after_check(void)
{
	static Uint64 exit_ticks = (Uint64)-1;

	if (exit_ticks == (Uint64)-1)
	{
		double seconds = config_real("debug.exit_after");

		exit_ticks = seconds > 0.0 ? SDL_GetTicks() + (Uint64)(seconds * 1000.0) : 0;
	}
	if (exit_ticks && SDL_GetTicks() >= exit_ticks)
	{
		platform_log("exiting after debug.exit_after");
		exit(EXIT_SUCCESS);
	}
}

/* the events without a window (debug.null_renderer, or no display): on the
thread that first asks (the game's, as its frames end), debug.exit_after, a
request to quit (SIGINT and SIGTERM, which SDL makes quit events: else the
game would not stop for them), and the updater's question if
debug.update_answer answers it (a dialog with no game window to stand
over is never shown); the other events are of no window, and dropped */
static void platform_pump_events_headless(void)
{
	static SDL_ThreadID thread;
	SDL_Event event;

	if (!thread)
		thread = SDL_GetCurrentThreadID();
	if (SDL_GetCurrentThreadID() != thread)
		return;
	/* (a dedicated server stops itself: server_director.c) */
	if (!halo_server_mode())
		exit_after_check();
#ifndef HALO_ANDROID
	if (config_string("debug.update_answer")[0])
		updater_poll(NULL);
#endif
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			platform_log("asked to quit (no window)");
			exit(EXIT_SUCCESS);
		}
	}
}

void platform_pump_events(void)
{
	SDL_Event event;
	static BOOL looked_at_clipboard;
	BOOL look_at_clipboard = !looked_at_clipboard;
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	BOOL mouse_needs_warp = FALSE;
#endif
	static int null_renderer = -1;

	if (!platform_window)
	{
		if (null_renderer < 0)
			null_renderer = config_boolean("debug.null_renderer") ? 1 : 0;
		if (platform_sdl_started && (null_renderer || platform_video_tried))
			platform_pump_events_headless();
		return;
	}
	if (SDL_GetCurrentThreadID() != platform_event_thread)
		return;
	exit_after_check();
	platform_show_pending_message();
#if !defined(HALO_ANDROID) && !defined(HALO_GLES)
	menu_cursor_fit();
#endif
#ifndef HALO_ANDROID
	updater_poll(platform_window);
	/* (Settings > Audio's output device, as it changes: dsound_sdl.c) */
	dsound_sdl_output_device_check();
#endif
	screen_keyboard_update();
	pthread_mutex_lock(&input_lock);
#ifndef HALO_ANDROID
	/* (the scoreboard closed, or no longer offering it: the pointer goes) */
	if (scoreboard_pointer_active && (SDL_GetTicks() >= scoreboard_open_until_ms || !scoreboard_pointer_offered ||
		input_state.ui_pointer))
	{
		scoreboard_pointer_stop();
	}
#endif
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			pthread_mutex_unlock(&input_lock);
			platform_log("window closed");
			exit(EXIT_SUCCESS);
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			if (event.key.scancode < SDL_SCANCODE_COUNT)
			{
				input_state.keys[event.key.scancode] = event.key.down;
				if (event.key.down)
					keys_pressed[event.key.scancode] = 1;
			}
			if (binding_capture == _binding_capture_waiting && event.key.down && !event.key.repeat &&
				event.key.scancode != SDL_SCANCODE_F11 && event.key.scancode != SDL_SCANCODE_F12)
			{
				binding_capture = _binding_capture_taken;
				binding_taken_ms = SDL_GetTicks();
				binding_capture_result = event.key.scancode == SDL_SCANCODE_ESCAPE ? 3 :
					event.key.scancode == SDL_SCANCODE_DELETE ? 2 : 1;
				binding_captured_input = event.key.scancode;
				break;
			}
			queue_keystroke(&event.key);
			if (SDL_GetTicks() < scoreboard_open_until_ms && event.key.down &&
				(event.key.scancode == SDL_SCANCODE_PAGEUP || event.key.scancode == SDL_SCANCODE_PAGEDOWN))
			{
				scoreboard_pages += event.key.scancode == SDL_SCANCODE_PAGEDOWN ? 1 : -1;
			}
			/* F12 releases or recaptures the mouse */
			if (event.key.down && !event.key.repeat && event.key.scancode == SDL_SCANCODE_F12)
			{
				input_state.mouse_released = !input_state.mouse_released;
				platform_mouse_capture(!input_state.mouse_released && !input_state.ui_pointer &&
					!scoreboard_pointer_active);
			}
#ifndef HALO_ANDROID
			/* F11 switches between fullscreen and the window (SDL keeps the
			window's size and place while fullscreen) */
			if (event.key.down && !event.key.repeat && event.key.scancode == SDL_SCANCODE_F11)
			{
				platform_window_set_fullscreen(!platform_window_fullscreen());
			}
#endif
			break;
		case SDL_EVENT_MOUSE_MOTION:
#ifndef HALO_ANDROID
#ifndef HALO_GLES
			/* (not the warps: to the middle as a menu opens, and back to
			where the mouse is held without relative mode) */
			if (input_state.ui_pointer ? event.motion.x != ui_pointer.x || event.motion.y != ui_pointer.y :
				!mouse_warp_capture || (int)event.motion.x != mouse_warp_x || (int)event.motion.y != mouse_warp_y)
				menu_cursor_mouse_used(input_state.ui_pointer);
#endif
			if (scoreboard_pointer_active)
			{
				scoreboard_pointer.x = event.motion.x;
				scoreboard_pointer.y = event.motion.y;
				scoreboard_pointer.moved = TRUE;
				break;
			}
			/* in the menus the mouse moves the pointer, not the view */
			if (input_state.ui_pointer)
			{
				ui_pointer.x = event.motion.x;
				ui_pointer.y = event.motion.y;
				ui_pointer.moved = TRUE;
				break;
			}
#ifndef HALO_WEB
			if (mouse_warp_capture)
			{
				if ((int)event.motion.x == mouse_warp_x && (int)event.motion.y == mouse_warp_y)
					break;
				mouse_needs_warp = TRUE;
			}
#endif
#endif
			input_state.mouse_dx += event.motion.xrel;
			input_state.mouse_dy += event.motion.yrel;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
#if !defined(HALO_ANDROID) && !defined(HALO_GLES)
			if (event.button.down)
				menu_cursor_mouse_used(input_state.ui_pointer);
#endif
			if (event.button.button < 32)
			{
				if (event.button.down)
					mouse_buttons_down |= 1u << event.button.button;
				else
					mouse_buttons_down &= ~(1u << event.button.button);
			}
			if (binding_capture == _binding_capture_waiting && event.button.down &&
				event.button.button < PLATFORM_MOUSE_BUTTON_COUNT)
			{
				binding_capture = _binding_capture_taken;
				binding_taken_ms = SDL_GetTicks();
				binding_capture_result = 1;
				binding_captured_input = INPUT_MOUSE + event.button.button;
				break;
			}
#ifndef HALO_ANDROID
			/* the open scoreboard's pointer: a right click frees it (and
			fires nothing), and another takes it back; its clicks pick */
			if (!input_state.ui_pointer && SDL_GetTicks() < scoreboard_open_until_ms && scoreboard_pointer_offered &&
				(scoreboard_pointer_active || (event.button.down && event.button.button == SDL_BUTTON_RIGHT)))
			{
				if (event.button.down && event.button.button == SDL_BUTTON_RIGHT)
				{
					if (scoreboard_pointer_active)
						scoreboard_pointer_stop();
					else
						scoreboard_pointer_start();
				}
				else if (event.button.down && event.button.button == SDL_BUTTON_LEFT)
				{
					scoreboard_pointer.left_clicks++;
					scoreboard_pointer.click_x = event.button.x;
					scoreboard_pointer.click_y = event.button.y;
				}
				break;
			}
			/* clicks in the menus go to the pointer; a button held down
			when the menu closes stays up until pressed again, so the click
			that resumes the game does not also fire */
			if (input_state.ui_pointer)
			{
				if (event.button.down && event.button.button == SDL_BUTTON_LEFT)
				{
					ui_pointer.left_clicks++;
					ui_pointer.click_x = event.button.x;
					ui_pointer.click_y = event.button.y;
				}
				else if (event.button.down && event.button.button == SDL_BUTTON_RIGHT)
				{
					ui_pointer.right_clicks++;
				}
				break;
			}
#endif
			if (event.button.button < PLATFORM_MOUSE_BUTTON_COUNT)
			{
				input_state.mouse_buttons[event.button.button] = event.button.down;
				if (event.button.down)
					mouse_buttons_pressed[event.button.button] = 1;
			}
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if (binding_capture == _binding_capture_waiting && event.wheel.y != 0.0f)
			{
				binding_capture = _binding_capture_taken;
				binding_taken_ms = SDL_GetTicks();
				binding_capture_result = 1;
				binding_captured_input = event.wheel.y > 0.0f ? INPUT_WHEEL_UP : INPUT_WHEEL_DOWN;
				break;
			}
			if (SDL_GetTicks() < scoreboard_open_until_ms)
			{
				/* whole notches, up (away) scrolling up */
				scoreboard_wheel -= event.wheel.y;
				while (scoreboard_wheel >= 1.0f)
				{
					scoreboard_notches++;
					scoreboard_wheel -= 1.0f;
				}
				while (scoreboard_wheel <= -1.0f)
				{
					scoreboard_notches--;
					scoreboard_wheel += 1.0f;
				}
				break;
			}
#ifndef HALO_ANDROID
			if (input_state.ui_pointer)
			{
				/* whole notches: smooth-scrolling wheels send fractions */
				ui_pointer_wheel += event.wheel.y;
				while (ui_pointer_wheel >= 1.0f)
				{
					ui_pointer.wheel_steps++;
					ui_pointer_wheel -= 1.0f;
				}
				while (ui_pointer_wheel <= -1.0f)
				{
					ui_pointer.wheel_steps--;
					ui_pointer_wheel += 1.0f;
				}
				break;
			}
#endif
			input_state.mouse_wheel += event.wheel.y;
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			memset(input_state.keys, 0, sizeof(input_state.keys));
			memset(input_state.mouse_buttons, 0, sizeof(input_state.mouse_buttons));
			memset(mouse_buttons_pressed, 0, sizeof(mouse_buttons_pressed));
			input_state.focused = FALSE;
			/* (the scoreboard's pointer goes; the mouse is taken back for
			the aim as the window has the focus again) */
			scoreboard_pointer_active = FALSE;
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
			if (mouse_warp_capture)
			{
				mouse_warp_capture = FALSE;
				SDL_SetWindowMouseGrab(platform_window, false);
				SDL_ShowCursor();
			}
#endif
#ifdef HALO_ANDROID
			touch_input_cancel();
#endif
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			input_state.focused = TRUE;
			look_at_clipboard = TRUE;
#ifndef HALO_ANDROID
			if (!input_state.mouse_released && !input_state.ui_pointer && !scoreboard_pointer_active)
				platform_mouse_capture(TRUE);
#endif
			break;
#ifdef HALO_ANDROID
		case SDL_EVENT_FINGER_DOWN:
		case SDL_EVENT_FINGER_MOTION:
		case SDL_EVENT_FINGER_UP:
		case SDL_EVENT_FINGER_CANCELED:
			touch_input_event(event.type, &event.tfinger);
			break;
#endif
		case SDL_EVENT_GAMEPAD_ADDED:
#ifdef HALO_ARM64_GUEST
			/* (the guest reaches SDL only through host_imports.list, which
			has no SDL_GetGamepadName) */
			SDL_OpenGamepad(event.gdevice.which);
#else
			{
				SDL_Gamepad *gamepad = SDL_OpenGamepad(event.gdevice.which);

				/* (which pads the game drives: under Steam Input, Steam's
				virtual ones, named for the controllers behind them; its
				kind, and the device SDL reads it through: on Linux, a
				hidraw node is SDL's own driver of the pad, an event node
				the kernel's) */
				if (gamepad)
				{
					const char *name = SDL_GetGamepadName(gamepad);
					const char *type = SDL_GetGamepadStringForType(SDL_GetGamepadType(gamepad));
					const char *path = SDL_GetGamepadPath(gamepad);

					platform_log("gamepad: %s (%s%s%s)", name ? name : "(unnamed)", type ? type : "unknown",
						path ? ", " : "", path ? path : "");
				}
			}
#endif
			break;
#if !defined(HALO_ANDROID) && !defined(HALO_GLES)
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			menu_cursor_controller_used(input_state.ui_pointer);
			break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			/* (past a stick's or trigger's rest, as gamepad_idle in
			xinput_sdl.c has it) */
			if (event.gaxis.value > 8000 || event.gaxis.value < -8000)
				menu_cursor_controller_used(input_state.ui_pointer);
			break;
#endif
		case SDL_EVENT_GAMEPAD_REMOVED:
			/* (SDL keeps a gamepad open until it is closed, even once the
			controller has gone) */
			{
				SDL_Gamepad *gamepad = SDL_GetGamepadFromID(event.gdevice.which);

				if (gamepad)
					SDL_CloseGamepad(gamepad);
			}
			break;
		default:
			break;
		}
	}
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	if (mouse_warp_capture && mouse_needs_warp && !input_state.ui_pointer &&
		!scoreboard_pointer_active && !input_state.mouse_released)
	{
		int width = 0, height = 0;

		SDL_GetWindowSize(platform_window, &width, &height);
		if (width > 0 && height > 0)
		{
			mouse_warp_x = width / 2;
			mouse_warp_y = height / 2;
			SDL_WarpMouseInWindow(platform_window, (float)mouse_warp_x, (float)mouse_warp_y);
		}
	}
#endif
	pthread_mutex_unlock(&input_lock);
	looked_at_clipboard = TRUE;
	platform_invite_clipboard(look_at_clipboard);
}

void platform_menus_set_active(BOOL active)
{
	pthread_mutex_lock(&input_lock);
	input_state.menus = active;
	pthread_mutex_unlock(&input_lock);
}

void platform_binding_capture_begin(void)
{
	pthread_mutex_lock(&input_lock);
	binding_capture = _binding_capture_waiting;
	binding_settling = TRUE;
	binding_polled_ms = SDL_GetTicks();
	pthread_mutex_unlock(&input_lock);
}

void platform_binding_capture_cancel(void)
{
	pthread_mutex_lock(&input_lock);
	binding_capture = _binding_capture_idle;
	pthread_mutex_unlock(&input_lock);
}

int platform_binding_capture_poll(int *input)
{
	int result = 0;

	pthread_mutex_lock(&input_lock);
	binding_polled_ms = SDL_GetTicks();
	if (binding_capture == _binding_capture_taken)
	{
		result = binding_capture_result;
		*input = binding_captured_input;
		binding_capture = _binding_capture_idle;
	}
	pthread_mutex_unlock(&input_lock);
	return result;
}

#ifndef HALO_ANDROID
/* ---------- the menus' pointer */

/* While a menu is up the mouse is released, its pointer shows (centered when
the menu opens) and its motion, clicks and wheel go to the menus
(halo_ui_pointer_update, d3d8_gl.c) instead of the controller and the aim. */
void platform_ui_pointer_set_active(BOOL active)
{
	if (!platform_window || (active != FALSE) == (input_state.ui_pointer != FALSE))
		return;
	pthread_mutex_lock(&input_lock);
	input_state.ui_pointer = active;
	memset(&ui_pointer, 0, sizeof(ui_pointer));
	ui_pointer_wheel = 0.0f;
	input_state.mouse_dx = 0.0f;
	input_state.mouse_dy = 0.0f;
	input_state.mouse_wheel = 0.0f;
	memset(input_state.mouse_buttons, 0, sizeof(input_state.mouse_buttons));
	memset(mouse_buttons_pressed, 0, sizeof(mouse_buttons_pressed));
	pthread_mutex_unlock(&input_lock);
	platform_mouse_capture(!active && !input_state.mouse_released);
#ifndef HALO_GLES
	if (active && menu_cursor_controller)
		SDL_HideCursor();
#endif
	if (active)
	{
		int width, height;

		SDL_GetWindowSize(platform_window, &width, &height);
		SDL_WarpMouseInWindow(platform_window, width * 0.5f, height * 0.5f);
		pthread_mutex_lock(&input_lock);
		ui_pointer.x = width * 0.5f;
		ui_pointer.y = height * 0.5f;
		pthread_mutex_unlock(&input_lock);
	}
}

/* what the pointer did since the last call; FALSE when it is not active */
BOOL platform_ui_pointer_read(struct platform_ui_pointer *pointer)
{
	BOOL active;

	pthread_mutex_lock(&input_lock);
	active = input_state.ui_pointer;
	*pointer = ui_pointer;
	ui_pointer.moved = FALSE;
	ui_pointer.left_clicks = 0;
	ui_pointer.right_clicks = 0;
	ui_pointer.wheel_steps = 0;
	pthread_mutex_unlock(&input_lock);
	return active;
}

void platform_video_window_size(int *width, int *height)
{
	SDL_GetWindowSize(platform_window, width, height);
}

/* ---------- the system's on-screen keyboard */

/* A menu's text field is typed into (platform_text_field, xinput_sdl.c).
Where Steam's on-screen keyboard is there to bring up (in Big Picture and in
the Steam Deck's Game Mode, which ask for it with
SDL_ENABLE_STEAM_SCREEN_KEYBOARD), SDL's text input runs while the field is
typed into: the keyboard comes up with the field and goes with it, and what
it types arrives as keys. Elsewhere text input stays off, as before, so that
no input method takes the keys the field reads: a Wayland touch screen's
keyboard (text-input-v3) would type text events, which the field does not
read. */
static SDL_AtomicInt screen_keyboard_wanted;
/* (each field begun, which brings the keyboard up again: Steam does not say
when its keyboard goes, by its own Enter or closed by hand, so SDL holds it
to be up still; after a field ended and another begun in the same frame, as
the password screen's is after a wrong password, it would not come back) */
static SDL_AtomicInt screen_keyboard_requests;

void platform_screen_keyboard(BOOL show, BOOL password)
{
	SDL_SetAtomicInt(&screen_keyboard_wanted, !show ? 0 : password ? 2 : 1);
	if (show)
		SDL_AddAtomicInt(&screen_keyboard_requests, 1);
}

/* (on the window's thread, as SDL asks: platform_pump_events) */
static void screen_keyboard_update(void)
{
	/* (a keyboard shown again is closed first, as SDL opens none that it
	holds to be up, and opened a moment later: Steam takes each as a URL,
	steam://close/keyboard then steam://open/keyboard, which must not
	arrive the other way round) */
	enum { REOPEN_DELAY_MS = 500 };
	static int requests_handled;
	static Uint64 open_time;
	int requests = SDL_GetAtomicInt(&screen_keyboard_requests);
	int wanted = SDL_GetAtomicInt(&screen_keyboard_wanted);

	if (!wanted)
	{
		open_time = 0;
		if (SDL_TextInputActive(platform_window))
			SDL_StopTextInput(platform_window);
		return;
	}
	if (requests != requests_handled)
	{
		requests_handled = requests;
		if (!SDL_HasScreenKeyboardSupport() ||
			!SDL_GetHintBoolean(SDL_HINT_ENABLE_STEAM_SCREEN_KEYBOARD, false))
		{
			return;
		}
		open_time = SDL_GetTicks();
		if (SDL_TextInputActive(platform_window))
		{
			SDL_StopTextInput(platform_window);
			open_time += REOPEN_DELAY_MS;
		}
	}
	if (open_time && SDL_GetTicks() >= open_time)
	{
		/* one line: the keyboard's Enter ends the field (and Steam's
		keyboard goes with it); a password's, for the keyboards that hide
		what is typed into one */
		SDL_PropertiesID properties = SDL_CreateProperties();

		open_time = 0;
		platform_log("text field: showing the on-screen keyboard");
		SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, false);
		SDL_SetNumberProperty(properties, SDL_PROP_TEXTINPUT_TYPE_NUMBER,
			wanted == 2 ? SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN : SDL_TEXTINPUT_TYPE_TEXT);
		SDL_StartTextInputWithProperties(platform_window, properties);
		SDL_DestroyProperties(properties);
	}
}

#else
/* ---------- the menus' pointer (the touchscreen)

While a menu is up, taps and drags go to the menus (touch_input.c,
halo_ui_pointer_update in d3d8_gl.c). */
void platform_ui_pointer_set_active(BOOL active)
{
	pthread_mutex_lock(&input_lock);
	if ((active != FALSE) != (input_state.ui_pointer != FALSE))
	{
		input_state.ui_pointer = active;
		touch_input_menu_set_active(active != FALSE);
	}
	pthread_mutex_unlock(&input_lock);
}

BOOL platform_ui_pointer_read(struct platform_ui_pointer *pointer)
{
	BOOL active;

	pthread_mutex_lock(&input_lock);
	active = input_state.ui_pointer;
	touch_input_menu_read(pointer);
	pthread_mutex_unlock(&input_lock);
	return active;
}

/* the window is its pixels on Android (no display scaling): touch_input.c
scales the fingers' 0..1 by the drawable's size */
void platform_video_window_size(int *width, int *height)
{
	platform_video_drawable_size(width, height);
}

/* ---------- the system's on-screen keyboard (Android's)

While a name is typed, into the game's own keyboard (Change Name's:
platform_text_typing) or a menu's text field (the server's name:
platform_text_field, xinput_sdl.c), SDL's text input runs where the touch
controls would show (touch_input_screen_buttons), which brings up Android's
keyboard. What it types arrives as keys, as a Bluetooth or USB keyboard's
does: SDL's input connection makes a key press of each character from space
to '~' and of each backspace, and its Enter is the Return key, which ends the
typing as a keyboard's does. The game's own keyboard still takes taps where
Android's does not cover it. Android's keyboard goes as the typing ends;
closed by hand (back), it comes again with the next name begun. (The
guest's SDL has no atomics: the compiler's.) */
static int screen_keyboard_wanted;
static int screen_keyboard_requests;

/* port/android/guest/runtime/guest_host.h: SDL's text input for the window,
started as the SDL_TextInputType given (one line, no autocorrection) or
stopped (-1); whether it runs */
int host_sdl_text_input(unsigned int window, int type);

void platform_screen_keyboard(BOOL show, BOOL password)
{
	__atomic_store_n(&screen_keyboard_wanted, !show ? 0 : password ? 2 : 1, __ATOMIC_SEQ_CST);
	if (show)
		__atomic_add_fetch(&screen_keyboard_requests, 1, __ATOMIC_SEQ_CST);
}

/* (on the window's thread: platform_pump_events) */
static void screen_keyboard_update(void)
{
	static int requests_handled;
	static BOOL started;
	unsigned int window = (unsigned int)(uintptr_t)platform_window;
	int requests = __atomic_load_n(&screen_keyboard_requests, __ATOMIC_SEQ_CST);
	int wanted = __atomic_load_n(&screen_keyboard_wanted, __ATOMIC_SEQ_CST);

	if (!wanted)
	{
		if (started)
			host_sdl_text_input(window, -1);
		started = FALSE;
		return;
	}
	if (requests == requests_handled)
		return;
	requests_handled = requests;
	/* (stopped first: one closed by hand may be held by SDL to be up still,
	and not shown again) */
	if (started)
		host_sdl_text_input(window, -1);
	started = FALSE;
	if (!touch_input_screen_buttons())
		return;
	platform_log("text field: showing Android's keyboard");
	/* (a name's as a visible password, for the keyboards' sake: no
	suggestions, and no words held back while they are put together, so that
	each key arrives as it is typed; a password's hidden) */
	started = host_sdl_text_input(window,
		wanted == 2 ? SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN : SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE) != 0;
}

#endif
void platform_input_read(struct platform_input_state *state, BOOL consume_motion)
{
	pthread_mutex_lock(&input_lock);
	*state = input_state;
	/* (rebinding: nothing reaches the controller until the input is taken
	and every key and button is up again) */
	if (binding_capture != _binding_capture_idle || binding_settling)
	{
		int scancode;
		BOOL held = mouse_buttons_down != 0;

		for (scancode = 0; scancode < SDL_SCANCODE_COUNT && !held; scancode++)
			held = input_state.keys[scancode] != 0;
		if (binding_capture == _binding_capture_taken && SDL_GetTicks() - binding_taken_ms > BINDING_UNCLAIMED_MS)
			binding_capture = _binding_capture_idle;
		if (binding_capture == _binding_capture_waiting && SDL_GetTicks() - binding_polled_ms > BINDING_ABANDONED_MS)
			binding_capture = _binding_capture_idle;
		if (binding_capture == _binding_capture_idle && !held)
			binding_settling = FALSE;
		memset(state->keys, 0, sizeof(state->keys));
		memset(state->mouse_buttons, 0, sizeof(state->mouse_buttons));
		memset(keys_pressed, 0, sizeof(keys_pressed));
		memset(mouse_buttons_pressed, 0, sizeof(mouse_buttons_pressed));
		state->mouse_wheel = 0.0f;
		/* (and the motion of the while, which would otherwise pile up for
		the aim) */
		state->mouse_dx = state->mouse_dy = 0.0f;
		if (consume_motion)
		{
			input_state.mouse_dx = input_state.mouse_dy = 0.0f;
			input_state.mouse_wheel = 0.0f;
		}
		pthread_mutex_unlock(&input_lock);
		return;
	}
	if (consume_motion)
	{
		int scancode;

		for (scancode = 0; scancode < SDL_SCANCODE_COUNT; scancode++)
		{
			state->keys[scancode] |= keys_pressed[scancode];
			keys_pressed[scancode] = 0;
		}
		for (scancode = 0; scancode < PLATFORM_MOUSE_BUTTON_COUNT; scancode++)
		{
			state->mouse_buttons[scancode] |= mouse_buttons_pressed[scancode];
			mouse_buttons_pressed[scancode] = 0;
		}
	}
	if (consume_motion)
	{
		input_state.mouse_dx = 0;
		input_state.mouse_dy = 0;
		input_state.mouse_wheel = 0;
	}
	pthread_mutex_unlock(&input_lock);
}

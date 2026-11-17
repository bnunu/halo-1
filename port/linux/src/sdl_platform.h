/*
SDL_PLATFORM.H

Window, OpenGL context and input state shared by the renderer and the
controller emulation (see sdl_platform.c).
*/

#ifndef __HALO_LINUX_SDL_PLATFORM_H
#define __HALO_LINUX_SDL_PLATFORM_H

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_audio.h>
#include <stddef.h>

#define PLATFORM_MOUSE_BUTTON_COUNT 8

struct platform_input_state
{
	unsigned char keys[SDL_SCANCODE_COUNT];
	unsigned char mouse_buttons[PLATFORM_MOUSE_BUTTON_COUNT]; /* SDL_BUTTON_* */
	float mouse_dx, mouse_dy;
	float mouse_wheel;
	BOOL focused;
	BOOL mouse_released;
	/* the mouse drives the menus' pointer (platform_ui_pointer_set_active)
	instead of the controller */
	BOOL ui_pointer;
	/* a menu is up (platform_menus_set_active): the keys drive the first
	controller, to move about it, instead of the player's actions */
	BOOL menus;
};

/* an input of the keyboard and mouse's controls (xinput_sdl.c): a scancode,
a mouse button (INPUT_MOUSE + SDL_BUTTON_*), or the wheel */
#define INPUT_MOUSE SDL_SCANCODE_COUNT
#define INPUT_WHEEL (INPUT_MOUSE + PLATFORM_MOUSE_BUTTON_COUNT)
#define INPUT_WHEEL_UP (INPUT_WHEEL + 1)
#define INPUT_WHEEL_DOWN (INPUT_WHEEL + 2)
/* an input by its name in config.toml ("W", "Mouse Left"), -1 if none */
int halo_input_from_name(const char *name);
void halo_input_name(int input, char *name, size_t size);
/* a binding's name ("Z") as the keyboard's layout labels its key ("Y"),
for the menus */
void halo_input_shown_name(const char *binding, char *shown, size_t size);

/* whether a menu is up (halo_ui_pointer_update, every frame) */
void platform_menus_set_active(BOOL active);
/* rebinding a control (Settings > Controls Setup): from now the next key,
mouse button or wheel turn is taken, and none reaches the game or the
menus; nor does the keyboard after, until every key and button is up */
void platform_binding_capture_begin(void);
/* the capture ends, nothing taken, as Escape ends it (the menus' B); its
guard for the keys already held stays */
void platform_binding_capture_cancel(void);
/* 0 while waiting; else the capture ends: 1 with the input, 2 for none
(Delete), 3 if cancelled (Escape) */
int platform_binding_capture_poll(int *input);

struct platform_keystroke
{
	BYTE virtual_key;
	CHAR ascii;
	BYTE flags;
};

BOOL platform_sdl_initialize(void);
/* creates the window and makes its OpenGL context current on this thread */
BOOL platform_video_initialize(unsigned long width, unsigned long height);
#ifndef HALO_ANDROID
BOOL platform_screen_mode(long *width, long *height);
#endif
/* Video Setup's resolutions (port/linux/game/menu_tags.c), in pixels: the
display's modes the game can draw at, largest first, each once, but the
display's own (Native), and display.resolution's where it is none of them;
how many. None on Android. */
int platform_display_resolutions(long *widths, long *heights, int maximum);
/* Video Setup's window sizes: those of each shape (4:3, 16:10, 16:9, 21:9)
that fit the desktop, and display.window_size's; how many. None on
Android. */
int platform_window_sizes(long *widths, long *heights, int maximum);
/* Settings > Audio's devices (port/linux/game/menu_tags.c): the names of
the output (or, recording, input) devices SDL finds; how many. None on
Android, which plays and records with the system's. */
#define PLATFORM_AUDIO_DEVICE_NAME_SIZE 128
int platform_audio_devices(int recording, char (*names)[PLATFORM_AUDIO_DEVICE_NAME_SIZE], int maximum);
/* ... the device of a name (audio.output_device, audio.input_device):
the system's default for "default", or one not found */
SDL_AudioDeviceID platform_audio_device(int recording, const char *name);
void platform_video_drawable_size(int *width, int *height);
#ifndef HALO_ANDROID
/* whether the window's display shows HDR, with the desktop's white (in
scRGB's units, 1.0 being 80 nits) and how many times brighter than it the
display goes (1.0 without HDR) */
BOOL platform_video_hdr(float *sdr_white, float *headroom);
#endif
/* the window's mode and size and V-Sync, from config.toml as Settings has
just written it (the main thread's) */
void platform_display_apply(void);
void platform_video_swap(void);
/* frames between the 30 Hz ticks at the display's refresh rate, unless
display.interpolation is false (port/linux/game/render_interpolation.c) */
int halo_interpolation_enabled(void);
void platform_mouse_capture(BOOL capture);

/* handles the window's and input devices' events; main thread only, a no-op
elsewhere; on Android this is where finger events reach touch_input.c, which
is why that module's state is only touched from the main thread */
void platform_pump_events(void);
/* request a capture from the bound Screenshot action */
void platform_screenshot_request(void);
/* consume the request; call on the render thread with a completed frame */
BOOL platform_screenshot_take_request(void);
/* a snapshot of the input state; consume_motion resets the mouse deltas */
void platform_input_read(struct platform_input_state *state, BOOL consume_motion);
/* the pointer in the menus (d3d8_gl.c, halo_ui_pointer_update) */
struct platform_ui_pointer
{
	/* in window coordinates, as SDL reports them (on Android the window's
	pixels: touch_input.c) */
	float x, y;
	float click_x, click_y;
	BOOL moved;
	int left_clicks, right_clicks;
	int wheel_steps;
	/* fingers down since the last read, and where the latest went down
	(the touchscreen only; the debug view of the menus' targets shows it) */
	int downs;
	float down_x, down_y;
	/* the pointer is the touchscreen (touch_input.c), not a mouse */
	BOOL touch;
};
/* says whether a menu is up; the mouse is released for the menus (desktop),
or the touchscreen goes to them (Android); a change of mode drops the gesture
in progress */
void platform_ui_pointer_set_active(BOOL active);

/* pointer receives what the pointer did since the last call; returns
nonzero while a menu is up (platform_ui_pointer_set_active) */
BOOL platform_ui_pointer_read(struct platform_ui_pointer *pointer);

/* returns the window's size in the units that pointer positions come in,
which differ from the drawable's pixels on displays that scale */
void platform_video_window_size(int *width, int *height);
#ifndef HALO_ANDROID
/* the open scoreboard's pointer, offered (a network game's) or not: TRUE
while a right click has freed it, with what it did since the last call */
BOOL platform_scoreboard_pointer(BOOL offered, struct platform_ui_pointer *pointer);
#endif
/* a menu's text field typed into or not (a password's or not), or on
Android also the game's own keyboard: the system's on-screen keyboard up
while it is, where there is one that text input shows (Steam's, Android's on
a touchscreen); each field begun brings it up again */
void platform_screen_keyboard(BOOL show, BOOL password);
#ifdef HALO_ANDROID
/* whether the keyboard last brought up is gone though the game still asks for
it (closed by hand, with back), or never came */
BOOL platform_screen_keyboard_lost(void);
#endif
BOOL platform_next_keystroke(struct platform_keystroke *keystroke);
/* the multiplayer scoreboard (game_engine.c) open or not: while it is, the
mouse wheel and Page Up/Down scroll it instead of switching weapons; how
far they moved it since the last call (notches and pages, down positive) */
void platform_scoreboard_scroll(int open, long *notches, long *pages);

#endif

/*
TOUCH_INPUT.C

The touchscreen of the Android build. SDL's finger events become the
menus' pointer (touch_menu.c) while a menu is up, as the desktop mouse
does (sdl_platform.c, platform_ui_pointer_read). Sizes are in dp, from the
display density the app passes (HALO_DISPLAY_DENSITY, host_main.c). The edges
of the screen where Android keeps its gestures are asked of the host at every
finger down (host_gesture_insets).
*/

#include "touch_input.h"
#include "touch_menu.h"
#include "port_config.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(HALO_ANDROID) || defined(HALO_WEB)
/* port/android/guest/runtime/guest_host.h (in a browser,
port/web/src/web_touch.c) */
void host_gesture_insets(int *insets);
void host_touch_read(int *state);
void host_touch_look_read(float *delta);
void host_touch_rumble(unsigned int low, unsigned int high);
void host_touch_scene(int scene);
void host_touch_bindings(const int *controls);
#endif
#ifdef HALO_ANDROID
int host_touch_devices(void);
#endif

/* the game's (port/linux/game/touch_game.c) */
int touch_game_cinematic_skippable(void);
int touch_game_cinematic_playing(void);
int touch_game_playing(void);
void touch_game_button_controls(int *controls);

/* the gamepad's buttons (input.h), which the touch controls are named by */
#define TOUCH_BUTTONS 16

/* a tap moves at most this far; a drag of this length is one wheel step */
#define TOUCH_TAP_SLOP_DP 12.0f
#define TOUCH_SCROLL_STEP_DP 40.0f
/* the menus post one press a frame and drop presses beyond their queue
(ui_widget.c): one step a read keeps up with them without losing any */
#define TOUCH_STEPS_PER_READ 1

static struct touch_menu menu;
static int menu_ready;
static int menu_active;
/* polls left to hold A: a tap is shorter than the game's poll, and the
game must see the press */
static int skip_polls;
#define TOUCH_PRESS_POLLS 2

/* returns pixels per dp; 0 only while neither the density
(HALO_DISPLAY_DENSITY) nor the window's size is known */
static float pixels_per_dp(void)
{
	const char *density = getenv("HALO_DISPLAY_DENSITY");
	int width = 0, height = 0;

	if (density && atof(density) > 0.0)
		return (float)atof(density);
	platform_video_drawable_size(&width, &height);
	/* a phone held in landscape is about 360 dp high, whatever its density */
	return height > 0 ? (float)height / 360.0f : 0.0f;
}

/* sets up gesture sizes once they can be known; tries again until they are */
static void menu_setup(void)
{
	if (!menu_ready)
	{
		struct touch_menu_settings settings;
		float dp = pixels_per_dp();
		int width = 0, height = 0;

		platform_video_drawable_size(&width, &height);

		settings.slop = TOUCH_TAP_SLOP_DP * (dp > 0.0f ? dp : 1.0f);
		settings.step = TOUCH_SCROLL_STEP_DP * (dp > 0.0f ? dp : 1.0f);
		settings.steps_per_read = TOUCH_STEPS_PER_READ;
		/* no zones until a finger goes down: touch_input_event reads them */
		settings.edge_left = settings.edge_top = settings.edge_right = settings.edge_bottom = 0.0f;
		settings.width = (float)width;
		settings.height = (float)height;
		touch_menu_init(&menu, &settings);
		menu_ready = dp > 0.0f && width > 0 && height > 0;
	}
}

/* updates gesture zones at every finger down: the phone rotates after startup
and zones matter only at a down, so changing them between two fingers is safe */
static void refresh_zones(int width, int height)
{
#ifdef HALO_ANDROID
	int insets[4];

	host_gesture_insets(insets);
	menu.settings.edge_left = (float)insets[0];
	menu.settings.edge_top = (float)insets[1];
	menu.settings.edge_right = (float)insets[2];
	menu.settings.edge_bottom = (float)insets[3];
#endif
	menu.settings.width = (float)width;
	menu.settings.height = (float)height;
}

void touch_input_event(unsigned int type, const SDL_TouchFingerEvent *finger)
{
	int width = 0, height = 0;
	float x, y;

	menu_setup();
	/* SDL gives 0..1 of the window; the window is its pixels on Android */
	platform_video_drawable_size(&width, &height);
	x = finger->x * (float)width;
	y = finger->y * (float)height;
	switch (type)
	{
	case SDL_EVENT_FINGER_DOWN:
		refresh_zones(width, height);
		touch_menu_down(&menu, finger->fingerID, x, y);
		break;
	case SDL_EVENT_FINGER_MOTION:
		touch_menu_move(&menu, finger->fingerID, x, y);
		break;
	case SDL_EVENT_FINGER_UP:
		touch_menu_up(&menu, finger->fingerID, x, y);
		break;
	case SDL_EVENT_FINGER_CANCELED:
		touch_menu_cancel(&menu);
		break;
	}
}

void touch_input_cancel(void)
{
	menu_setup();
	touch_menu_cancel(&menu);
}

void touch_input_menu_set_active(int active)
{
	/* what a finger began before the menu opened or closed must not
	click in it */
	menu_setup();
	touch_menu_reset(&menu);
	menu_active = active;
}

void touch_input_menu_read(struct platform_ui_pointer *pointer)
{
	struct touch_menu_output output;

	menu_setup();
	touch_menu_read(&menu, &output);
	pointer->x = output.x;
	pointer->y = output.y;
	pointer->click_x = output.click_x;
	pointer->click_y = output.click_y;
	pointer->moved = output.moved ? TRUE : FALSE;
	pointer->left_clicks = output.clicks;
	pointer->right_clicks = 0;
	pointer->wheel_steps = output.wheel_steps;
	pointer->downs = output.downs;
	pointer->down_x = output.down_x;
	pointer->down_y = output.down_y;
	pointer->touch = TRUE;
}

/* port/linux/game/network_chat.c: its on-screen keyboard is up, which
takes the taps outside the menus */
extern short network_chat_controller(void);

/* the taps for that keyboard since it last took them, and the latest's
place (0 to 1 of the window) */
static int chat_taps;
static float chat_tap_x, chat_tap_y;

int touch_input_chat_taps(float *x, float *y)
{
	int taps = chat_taps;

	*x = chat_tap_x;
	*y = chat_tap_y;
	chat_taps = 0;
	return taps;
}

void touch_input_gamepad(XINPUT_GAMEPAD *pad)
{
	/* outside the menus the touchscreen only skips a cinematic that can be
	skipped: a tap there presses A */
	if (!menu_active)
	{
		struct touch_menu_output output;

		menu_setup();
		touch_menu_read(&menu, &output);
		/* (or, while the text chat's on-screen keyboard is up, its keys) */
		if (output.clicks && (network_chat_controller() == 0))
		{
			int width = 0, height = 0;

			platform_video_drawable_size(&width, &height);
			chat_taps += output.clicks;
			chat_tap_x = width > 0 ? output.click_x / (float)width : 0.0f;
			chat_tap_y = height > 0 ? output.click_y / (float)height : 0.0f;
		}
		else if (output.clicks && touch_game_cinematic_skippable())
		{
			skip_polls = TOUCH_PRESS_POLLS;
			platform_log("touch: A pressed to skip a cinematic");
		}
	}
	if (skip_polls > 0)
	{
		pad->bAnalogButtons[XINPUT_GAMEPAD_A] = 0xff;
		skip_polls--;
	}
}

#if defined(HALO_ANDROID) || defined(HALO_WEB)
/* ---------- the on-screen touch controls

The overlay (port/android/app/.../TouchControls.java) draws and reads its
own fingers on Android's UI thread; port/android/host/host_touch.c hands
its state over. In a browser the page's overlay (port/web/site/touch.js)
does, through port/web/src/web_touch.c. These bits tell it when to show: */
enum
{
	/* the game has read its controller: the other bits are known */
	_touch_scene_known = 1 << 0,
	/* a menu or a cinematic is up: the overlay hides and lets the fingers
	through to the menus' pointer above */
	_touch_scene_menus = 1 << 1,
	/* input.touch_controls: "on" and "off" (none: "auto", shown when the
	device has a touchscreen and no controller) */
	_touch_scene_on = 1 << 2,
	_touch_scene_off = 1 << 3,
};

/* the touch controls' stick at port 0's last read, -1..1, y down */
static float move_x, move_y;

/* input.touch_controls as _touch_scene_on, _touch_scene_off or 0 */
static int touch_controls_setting(void)
{
	static int setting;
	static unsigned long read_at = (unsigned long)-1;

	if (read_at != config_changes())
	{
		const char *value = config_string("input.touch_controls");

		read_at = config_changes();
		setting = 0;
		if (value && !strcmp(value, "on"))
			setting = _touch_scene_on;
		else if (value && !strcmp(value, "off"))
			setting = _touch_scene_off;
	}
	return setting;
}

#ifdef HALO_ANDROID
/* the overlay's devices (port/android/host/host_touch.c's
HOST_TOUCH_DEVICE_*) */
#define TOUCH_DEVICE_TOUCHSCREEN 1
#define TOUCH_DEVICE_CONTROLLER 2

int touch_input_screen_buttons(void)
{
	int devices = host_touch_devices();
	int setting = touch_controls_setting();

	/* (TouchControls.java's shown(), but for the scene) */
	if (setting == _touch_scene_off || !(devices & TOUCH_DEVICE_TOUCHSCREEN))
		return FALSE;
	return setting == _touch_scene_on || !(devices & TOUCH_DEVICE_CONTROLLER);
}
#endif

void touch_input_controls(XINPUT_GAMEPAD *pad, int menus)
{
	/* SDL's gamepad buttons, in order (SDL_GamepadButton); the first four
	are the analog A, B, X and Y */
	static const WORD digital[] =
	{
		0, 0, 0, 0, XINPUT_GAMEPAD_BACK, 0, XINPUT_GAMEPAD_START,
		XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB,
		0, 0, XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
		XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT
	};
	static const int analog[] =
	{
		XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y
	};
	static int bindings_sent[TOUCH_BUTTONS];
	static int bindings_known;
	int bindings[TOUCH_BUTTONS];
	int state[7];
	int index;

	/* (the controls show only in a game: not while it starts, at the main
	menu, in a menu or during a cinematic; nor while the text chat's
	on-screen keyboard is up, whose keys the fingers then tap) */
	host_touch_scene(_touch_scene_known | touch_controls_setting() |
		(menus || !touch_game_playing() || touch_game_cinematic_playing() || (network_chat_controller() == 0) ?
			_touch_scene_menus : 0));
	/* (the profile's mapping, for the buttons' names; only when it changes) */
	touch_game_button_controls(bindings);
	if (!bindings_known || memcmp(bindings, bindings_sent, sizeof(bindings)))
	{
		memcpy(bindings_sent, bindings, sizeof(bindings));
		bindings_known = TRUE;
		host_touch_bindings(bindings);
	}
	host_touch_read(state);
	/* the stick moves the player through the game's input state, whatever
	the profile's sticks do (touch_input_move), not as the left stick */
	move_x = state[0] / 32767.0f;
	move_y = state[1] / 32767.0f;
	for (index = 0; index < 4; index++)
	{
		if (state[6] & (1 << index))
			pad->bAnalogButtons[analog[index]] = 0xff;
	}
	for (index = 0; index < (int)(sizeof(digital) / sizeof(digital[0])); index++)
	{
		if (state[6] & (1 << index))
			pad->wButtons |= digital[index];
	}
	/* the shoulders are white and black, as a DualSense's L1 and R1 */
	if (state[6] & (1 << 9))
		pad->bAnalogButtons[XINPUT_GAMEPAD_WHITE] = 0xff;
	if (state[6] & (1 << 10))
		pad->bAnalogButtons[XINPUT_GAMEPAD_BLACK] = 0xff;
	if (state[4])
		pad->bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] = 0xff;
	if (state[5])
		pad->bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] = 0xff;
}

int touch_input_move(float *forward, float *strafe)
{
	float x = move_x;
	float y = move_y;
	float largest = fabsf(x) > fabsf(y) ? fabsf(x) : fabsf(y);

	if (largest == 0.0f)
	{
		*forward = 0.0f;
		*strafe = 0.0f;
		return FALSE;
	}
	/* the overlay's circle onto the square a controller's stick gives
	(input_abstraction_update): a full diagonal is full on both axes */
	{
		float scale = sqrtf(x * x + y * y) / largest;

		x *= scale;
		y *= scale;
		x = x < -1.0f ? -1.0f : x > 1.0f ? 1.0f : x;
		y = y < -1.0f ? -1.0f : y > 1.0f ? 1.0f : y;
	}
	/* (SDL's y runs down; strafe is positive to the left) */
	*forward = -y;
	*strafe = -x;
	return TRUE;
}

void touch_input_look(float scale, float *yaw, float *pitch, float *gyro_yaw, float *gyro_pitch)
{
	float delta[4];

	host_touch_look_read(delta);
	*yaw -= delta[0] * scale;
	*pitch -= delta[1] * scale;
	*gyro_yaw -= delta[2] * scale;
	*gyro_pitch -= delta[3] * scale;
}

int touch_input_aim_assist(void)
{
	static int assisted;
	static unsigned long read_at = (unsigned long)-1;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		assisted = config_boolean("input.touch_aim_assist") != 0;
	}
	return assisted;
}

void touch_input_rumble(unsigned int left, unsigned int right)
{
	host_touch_rumble(left, right);
}
#endif

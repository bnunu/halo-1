/*
CHAT_SYSTEM_KEYBOARD.C (test)

A chat line typed on Android's own keyboard, real code over a fake SDL:
port/linux/game/network_chat.c begins a line from the touch controls' D-pad
Up on the system's keyboard (a controller's on the chat's own), lets it go
when that keyboard is closed with back or a tap lands elsewhere, and lets
the keyboard go with the line; port/linux/src/xinput_sdl.c and the Android
half of port/linux/src/sdl_platform.c bring the keyboard up with SDL's text
input and say when it went without the game's asking.
test_chat_system_keyboard.py takes the constants and the state (config.inc)
and the code (under_test.inc).
*/

#include "harness.h"

#include <stdint.h>

#define HALO_ANDROID

typedef int BOOL;

#include "config.inc"

/* SDL's text input types (the two the code asks for) */
enum
{
	SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN = 4,
	SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE = 5,
};

/* the fake SDL: its text input, which is Android's keyboard, and which SDL
stops itself when back closes the keyboard */
static struct
{
	int starts;
	int stops;
	int type;
	boolean active;
	boolean start_fails;
} sdl;
static void *platform_window = &sdl;
/* whether the touch controls would show (no controller connected) */
static boolean screen_buttons = TRUE;
/* the taps outside the menus since the chat last took them */
static int taps;
static unsigned long pad_buttons;

static int host_sdl_text_input(
	unsigned int window,
	int type)
{
	CHECK(window == (unsigned int)(uintptr_t)&sdl, "text input for another window");
	if (type < 0)
	{
		sdl.stops++;
		sdl.active = FALSE;
		return 0;
	}
	sdl.starts++;
	sdl.type = type;
	sdl.active = !sdl.start_fails;
	return sdl.active;
}

static int host_sdl_text_input_active(
	unsigned int window)
{
	(void)window;
	return sdl.active;
}

static int touch_input_screen_buttons(
	void)
{
	return screen_buttons;
}

static int halo_chat_taps(
	float *x,
	float *y)
{
	int count = taps;

	*x = 0.5f;
	*y = 0.3f;
	taps = 0;
	return count;
}

static unsigned long halo_chat_pad_buttons(
	void)
{
	return pad_buttons;
}

static unsigned long halo_chat_pad_buttons_for_port(
	short port)
{
	(void)port;
	return pad_buttons;
}

#define platform_log(...) ((void)0)
#define error(...) ((void)0)

static short chat_scroll;

#include "under_test.inc"

/* a line begun with D-pad Up by the first local player, as
network_chat_update begins it (chat_input_begin, then the keyboard) */
static void begin(
	boolean physical)
{
	memset(&chat_input, 0, sizeof(chat_input));
	chat_input.active = TRUE;
	chat_input.controller = 0;
	chat_input.physical = physical;
	chat_keyboard_begin(physical);
}

/* the window's thread's turn (platform_pump_events) */
static void pump(
	void)
{
	screen_keyboard_update();
}

/* the game's frame while the line is typed (network_chat_update's) */
static void frame(
	void)
{
	if (chat_input.active && chat_input.system_keyboard)
		chat_system_keyboard_update();
}

/* back on Android's keyboard: SDL's SDLDummyEdit stops its text input */
static void back(
	void)
{
	sdl.active = FALSE;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	/* (chat_input_end's alone sets the scroll; a negative control can take
	away the code's only read of the others) */
	(void)chat_scroll;
	(void)screen_buttons;
	(void)taps;

	/* the touch controls' D-pad Up: Android's keyboard, as a visible
	password (each key arrives as typed), not the chat's own; the touch
	controls hide and the taps go to the chat, as for the chat's keyboard */
	CASE("touch")
	{
		begin(FALSE);
		CHECK(chat_input.system_keyboard && !chat_keyboard.open, "the chat's own keyboard opened on the touch controls");
		CHECK(network_chat_controller() == 0 && network_chat_keyboard_open(),
			"the touch controls stay and the taps do not reach the chat while Android's keyboard is up");
		pump();
		CHECK(sdl.starts == 1 && sdl.active && sdl.type == SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE,
			"Android's keyboard did not come up as a visible password (%d starts, type %d)", sdl.starts, sdl.type);
		frame();
		pump();
		frame();
		CHECK(chat_input.active && sdl.starts == 1 && !sdl.stops, "the line or its keyboard went by itself");
		return 0;
	}
	/* a controller's D-pad Up (bound to the chat): the chat's own keyboard,
	which the controller drives; Android's does not come */
	CASE("controller")
	{
		begin(TRUE);
		CHECK(chat_keyboard.open && !chat_input.system_keyboard && chat_keyboard.row == 1 && !chat_keyboard.column,
			"a controller's line did not open the chat's own keyboard");
		pump();
		CHECK(!sdl.starts, "Android's keyboard came up for a controller's line");
		return 0;
	}
	/* no touch controls (a controller connected, or input.touch_controls
	"off"): the chat's own keyboard, as Android's would not come */
	CASE("no-touch-controls")
	{
		screen_buttons = FALSE;
		begin(FALSE);
		CHECK(chat_keyboard.open && !chat_input.system_keyboard, "no keyboard for a line without the touch controls");
		pump();
		CHECK(!sdl.starts, "Android's keyboard asked for without the touch controls");
		return 0;
	}
	/* back closes Android's keyboard: the line is let go, unsent, and the
	touch controls come back */
	CASE("back")
	{
		begin(FALSE);
		pump();
		frame();
		back();
		pump();
		frame();
		CHECK(!chat_input.active && !chat_input.system_keyboard && network_chat_controller() == NONE &&
			!network_chat_keyboard_open(), "the line stayed after back closed its keyboard");
		return 0;
	}
	/* a tap elsewhere lets the line go, and its keyboard with it */
	CASE("tap")
	{
		begin(FALSE);
		pump();
		taps = 1;
		frame();
		CHECK(!chat_input.active, "a tap elsewhere did not let the line go");
		pump();
		CHECK(sdl.stops == 1 && !sdl.active, "Android's keyboard stayed after the line went");
		return 0;
	}
	/* the line sent with Enter (or let go with Escape): the keyboard goes,
	and is not taken to have been closed by hand */
	CASE("ended")
	{
		begin(FALSE);
		pump();
		chat_input_end();
		CHECK(!platform_screen_keyboard_lost(), "the game's own close taken for back");
		pump();
		CHECK(sdl.stops == 1 && !sdl.active, "Android's keyboard stayed after the line went");
		pump();
		CHECK(sdl.stops == 1, "the keyboard stopped again");
		return 0;
	}
	/* a line closed with back says nothing of the next: it comes up again,
	and stays */
	CASE("next-line")
	{
		begin(FALSE);
		pump();
		back();
		pump();
		frame();
		CHECK(!chat_input.active, "the first line stayed after back");
		begin(FALSE);
		frame();
		CHECK(chat_input.active, "the first line's back let the next go before its keyboard came");
		pump();
		frame();
		CHECK(chat_input.active && sdl.starts == 2 && sdl.active, "the next line's keyboard did not come (%d starts)",
			sdl.starts);
		return 0;
	}
	/* a keyboard that never came (a controller connected meanwhile, or SDL's
	text input refused) lets the line go: it could not be typed */
	CASE("never-came")
	{
		sdl.start_fails = TRUE;
		begin(FALSE);
		pump();
		frame();
		CHECK(!chat_input.active, "a line stayed whose keyboard did not start");
		sdl.start_fails = FALSE;
		begin(FALSE);
		screen_buttons = FALSE;
		pump();
		frame();
		CHECK(!chat_input.active && sdl.starts == 1, "a line stayed whose keyboard could not come");
		return 0;
	}
	fprintf(stderr, "unknown case %s\n", case_name);
	return 2;
}

/*
INPUT_XBOX.C

symbols in this file:
000BE780 0050:
	_fix_dead_zone (0000)
000BE7D0 0030:
	_update_ticks (0000)
000BE800 0030:
	_code_000be800 (0000)
000BE830 0050:
	_input_dispose (0000)
000BE880 0010:
	_input_activate (0000)
000BE890 0010:
	_input_deactivate (0000)
000BE8A0 0060:
	_input_flush (0000)
000BE900 00c0:
	_input_key_is_down (0000)
000BE9C0 0070:
	_input_get_key (0000)
000BEA30 0010:
	_input_get_mouse_state (0000)
000BEA40 0010:
	_input_suppress (0000)
000BEA50 0010:
	_input_mouse_button_is_down (0000)
000BEA60 0050:
	_input_has_gamepad (0000)
000BEAB0 0070:
	_input_get_gamepad_state (0000)
000BEB20 0060:
	_input_set_gamepad_rumbler_state (0000)
000BEB80 0030:
	_input_vertical_blank_interrupt (0000)
000BEBB0 0010:
	_code_000bebb0 (0000)
000BEBC0 0010:
	_code_000bebc0 (0000)
000BEBD0 04d0:
	_code_000bebd0 (0000)
000BF0A0 00b0:
	_code_000bf0a0 (0000)
000BF150 0280:
	_code_000bf150 (0000)
000BF3D0 00f0:
	_input_get_raw_data_string (0000)
000BF4C0 0060:
	_input_update (0000)
000BF520 0010:
	_input_frame_begin (0000)
000BF530 0010:
	_input_frame_end (0000)
000BF540 0020:
	_code_000bf540 (0000)
000BF560 0100:
	_input_initialize (0000)
0026F570 0310:
	_rdata_0026f570 (0000)
0026F880 0027:
	??_C@_0CH@PJPACDN@key_code?$DO?$DN0?5?$CG?$CG?5key_code?$DMNUMBER_O@ (0000)
0026F8A8 0022:
	??_C@_0CC@GELNFCFL@c?3?2halo?2SOURCE?2input?2input_xbox?4@ (0000)
0026F8D0 006e:
	??_C@_0GO@PMFMJCLJ@input_globals?4buffered_key_read_@ (0000)
0026F940 0033:
	??_C@_0DD@KEPDEKGB@gamepad_index?$DO?$DN0?5?$CG?$CG?5gamepad_inde@ (0000)
0026F974 0037:
	??_C@_0DH@CKEAMKJJ@XGetState?5?$CIgamepad?$CJ?5failed?5?$CI?$CD?$CFd?$CJ@ (0000)
0026F9AC 0038:
	??_C@_0DI@FEEPFG@XInputOpen?5?$CIgamepad?$CJ?5failed?5?$CI?$CD?$CFd@ (0000)
0026F9E4 0033:
	??_C@_0DD@LHONKLFJ@input_globals?4gamepad_handles?$FLga@ (0000)
0026FA18 002d:
	??_C@_0CN@LLLCEEMO@input_globals?4gamepad_handles?$FLga@ (0000)
0026FA48 002f:
	??_C@_0CP@EHHPCBHL@key?4key_code?$DO?$DN0?5?$CG?$CG?5key?4key_code?$DM@ (0000)
0026FA78 0048:
	??_C@_0EI@INGGDNKP@keystroke?4VirtualKey?$DO?$DN0?5?$CG?$CG?5keyst@ (0000)
0026FAC0 003c:
	??_C@_0DM@NFGLODNP@keystroke?4Ascii?$DO?$DN0?5?$CG?$CG?5keystroke?4@ (0000)
0026FAFC 0039:
	??_C@_0DJ@CLMKEEPN@XInputOpen?5?$CIkeyboard?$CJ?5failed?5?$CI?$CD?$CF@ (0000)
0026FB38 0024:
	??_C@_0CE@HLFAFMJM@input_globals?4keyboard_handle?$DN?$DNN@ (0000)
0026FB5C 001e:
	??_C@_0BO@BOABODDP@input_globals?4keyboard_handle?$AA@ (0000)
0026FB7C 0021:
	??_C@_0CB@EFGOEOI@gamepad?5?$CFd?$HMt?$CI?$CFd?0?5?$CFd?$CJ?$HMt?$CI?$CFd?0?5?$CFd?$CJ?$HMn@ (0000)
0026FBA0 002d:
	??_C@_0CN@OAEKILIO@?$HMn?$HMn?$HMn?$HMngamepad?$HMtleft?5stick?$HMtrig@ (0000)
0026FBD0 0007:
	??_C@_06NOMBLFJD@size?$DO0?$AA@ (0000)
0026FBD8 0044:
	??_C@_0EE@KIBLAJLM@XInputDebugInitKeyboardQueue?5fai@ (0000)
004536A0 040c:
	_bss_004536a0 (0000)
*/

/* ---------- headers */

#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "game/game.h"
#ifndef _X86_
#define _X86_
#endif
#ifndef DEBUG_KEYBOARD
#define DEBUG_KEYBOARD
#endif
#include <excpt.h>
#include <windef.h>
#include <winbase.h>
#include <xbox.h>
#include <xkbd.h>

#include "input/HaloAutoTest.h"
#include "input.h"
#include "input/input_abstraction.h"
#include "interface/player_ui.h"
#include "main/console.h"

/* ---------- constants */

enum
{
	MAXIMUM_BUFFERED_KEYSTROKES = 64,
	NUMBER_OF_VIRTUAL_CODES = 256,
	NUMBER_OF_ASCII_CODES = 128,

	GAMEPAD_STICK_DEAD_RANGE = 9000,
	ANALOG_BUTTON_PRESS_THRESHOLD_OFFSET = 32,
	ANALOG_BUTTON_RELEASE_THRESHOLD_OFFSET = 64,
	ANALOG_BUTTON_RELEASE_SATURATION = 191
};

enum
{
	_input_device_change_gamepad0_removed = 0,
	_input_device_change_gamepad1_removed,
	_input_device_change_gamepad2_removed,
	_input_device_change_gamepad3_removed,
	_input_device_change_memory_unit0_removed,
	_input_device_change_memory_unit1_removed,
	_input_device_change_memory_unit2_removed,
	_input_device_change_memory_unit3_removed,
	_input_device_change_memory_unit4_removed,
	_input_device_change_memory_unit5_removed,
	_input_device_change_memory_unit6_removed,
	_input_device_change_memory_unit7_removed,
	_input_device_change_gamepad0_inserted,
	_input_device_change_gamepad1_inserted,
	_input_device_change_gamepad2_inserted,
	_input_device_change_gamepad3_inserted,
	_input_device_change_memory_unit0_inserted,
	_input_device_change_memory_unit1_inserted,
	_input_device_change_memory_unit2_inserted,
	_input_device_change_memory_unit3_inserted,
	_input_device_change_memory_unit4_inserted,
	_input_device_change_memory_unit5_inserted,
	_input_device_change_memory_unit6_inserted,
	_input_device_change_memory_unit7_inserted
};

enum
{
	_key_modifier_shift = 0,
	_key_modifier_control,
	_key_modifier_alt
};

/* ---------- macros */

/* ---------- structures */

struct raw_gamepad_state
{
	point2d sticks[NUMBER_OF_GAMEPAD_STICKS];
};

struct input_globals
{
	XINPUT_FEEDBACK gamepad_feedbacks[MAXIMUM_GAMEPADS];
	struct raw_gamepad_state raw_gamepad_states[MAXIMUM_GAMEPADS];
	boolean suppressed;
	boolean update_thread_started;
	unsigned char reserved1[2];
	HANDLE gamepad_handles[MAXIMUM_GAMEPADS];
	struct gamepad_state gamepad_states[MAXIMUM_GAMEPADS];
	struct gamepad_state suppressed_gamepad_state;
	struct vibrate_data gamepad_rumbler_states[MAXIMUM_GAMEPADS];
	HANDLE update_thread_handle;
	HANDLE update_event_handle;
	boolean update_event_pending;
	boolean frame_active;
	unsigned char reserved3[2];
	long keyboard_queue_state;
	HANDLE keyboard_handle;
	byte key_ticks[NUMBER_OF_KEYS];
	byte key_latches[NUMBER_OF_KEYS];
	short buffered_key_read_index;
	short buffered_key_write_index;
	struct key_stroke buffered_keys[MAXIMUM_BUFFERED_KEYSTROKES];
};

typedef char verify_raw_gamepad_state_size[
	sizeof(struct raw_gamepad_state) == 0x8 ? 1 : -1];
typedef char verify_input_gamepad_feedbacks_offset[
	offsetof(struct input_globals, gamepad_feedbacks) == 0 ? 1 : -1];
typedef char verify_input_raw_gamepad_states_offset[
	offsetof(struct input_globals, raw_gamepad_states) == 0x118 ? 1 : -1];
typedef char verify_input_suppressed_offset[
	offsetof(struct input_globals, suppressed) == 0x138 ? 1 : -1];
typedef char verify_input_update_thread_started_offset[
	offsetof(struct input_globals, update_thread_started) == 0x139 ? 1 : -1];
typedef char verify_input_gamepad_handles_offset[
	offsetof(struct input_globals, gamepad_handles) == 0x13C ? 1 : -1];
typedef char verify_input_gamepad_states_offset[
	offsetof(struct input_globals, gamepad_states) == 0x14C ? 1 : -1];
typedef char verify_input_suppressed_gamepad_state_offset[
	offsetof(struct input_globals, suppressed_gamepad_state) == 0x1EC ? 1 : -1];
typedef char verify_input_gamepad_rumbler_states_offset[
	offsetof(struct input_globals, gamepad_rumbler_states) == 0x214 ? 1 : -1];
typedef char verify_input_update_thread_handle_offset[
	offsetof(struct input_globals, update_thread_handle) == 0x224 ? 1 : -1];
typedef char verify_input_update_event_handle_offset[
	offsetof(struct input_globals, update_event_handle) == 0x228 ? 1 : -1];
typedef char verify_input_update_event_pending_offset[
	offsetof(struct input_globals, update_event_pending) == 0x22C ? 1 : -1];
typedef char verify_input_frame_active_offset[
	offsetof(struct input_globals, frame_active) == 0x22D ? 1 : -1];
typedef char verify_input_keyboard_queue_state_offset[
	offsetof(struct input_globals, keyboard_queue_state) == 0x230 ? 1 : -1];
typedef char verify_input_keyboard_handle_offset[
	offsetof(struct input_globals, keyboard_handle) == 0x234 ? 1 : -1];
typedef char verify_input_key_ticks_offset[
	offsetof(struct input_globals, key_ticks) == 0x238 ? 1 : -1];
typedef char verify_input_key_latches_offset[
	offsetof(struct input_globals, key_latches) == 0x2A0 ? 1 : -1];
typedef char verify_input_buffered_key_read_index_offset[
	offsetof(struct input_globals, buffered_key_read_index) == 0x308 ? 1 : -1];
typedef char verify_input_buffered_key_write_index_offset[
	offsetof(struct input_globals, buffered_key_write_index) == 0x30A ? 1 : -1];
typedef char verify_input_buffered_keys_offset[
	offsetof(struct input_globals, buffered_keys) == 0x30C ? 1 : -1];
typedef char verify_input_globals_size[
	sizeof(struct input_globals) == 0x40C ? 1 : -1];

/* ---------- prototypes */

static void input_update_analog_button_state(
	byte *threshold,
	byte value,
	boolean pressed);
static void input_get_device_states(
	void);
static void input_update_keyboard_devices(
	void);
static void input_flush_rumble(
	void);
static DWORD WINAPI input_keyboard_thread(
	void *context);

/* ---------- globals */

static struct input_globals input_globals = {0};
static long gamepad_button_down_times[MAXIMUM_GAMEPADS][NUMBER_OF_GAMEPAD_BUTTONS];
static long key_down_times[NUMBER_OF_KEYS];

static const byte gamepad_analog_button_indices[NUMBER_OF_GAMEPAD_ANALOG_BUTTONS] =
{
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_BLACK,
	XINPUT_GAMEPAD_WHITE,
	XINPUT_GAMEPAD_LEFT_TRIGGER,
	XINPUT_GAMEPAD_RIGHT_TRIGGER
};

static const byte gamepad_binary_button_masks[NUMBER_OF_GAMEPAD_BINARY_BUTTONS] =
{
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB
};

static const short virtual_key_to_key_code[NUMBER_OF_VIRTUAL_CODES] =
{
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	_key_backspace, _key_tab, NONE, NONE, NONE, _key_return, NONE, NONE,
	_key_shift, _key_control, _key_left_alt, _key_pause, _key_caps_lock, NONE, NONE, NONE,
	NONE, NONE, NONE, _key_escape, NONE, NONE, NONE, NONE,
	_key_space, _key_page_up, _key_page_down, _key_end, _key_home, _key_left_arrow, _key_up_arrow, _key_right_arrow,
	_key_down_arrow, NONE, NONE, NONE, NONE, _key_insert, _key_delete, NONE,
	_key_0, _key_1, _key_2, _key_3, _key_4, _key_5, _key_6, _key_7,
	_key_8, _key_9, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, _key_a, _key_b, _key_c, _key_d, _key_e, _key_f, _key_g,
	_key_h, _key_i, _key_j, _key_k, _key_l, _key_m, _key_n, _key_o,
	_key_p, _key_q, _key_r, _key_s, _key_t, _key_u, _key_v, _key_w,
	_key_x, _key_y, _key_z, _key_left_windows, _key_right_windows, _key_menu, NONE, NONE,
	_keypad_0, _keypad_1, _keypad_2, _keypad_3, _keypad_4, _keypad_5, _keypad_6, _keypad_7,
	_keypad_8, _keypad_9, _keypad_multiply, _keypad_add, NONE, _keypad_subtract, _keypad_decimal, _keypad_divide,
	_key_f1, _key_f2, _key_f3, _key_f4, _key_f5, _key_f6, _key_f7, _key_f8,
	_key_f9, _key_f10, _key_f11, _key_f12, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	_keypad_num_lock, _key_scroll_lock, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, _key_semicolon, _key_equal, _key_comma, _key_dash, _key_period, _key_forwardslash,
	_key_backquote, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, _key_left_bracket, _key_backslash, _key_right_bracket, _key_apostrophe, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE
};

static const short ascii_to_key_code[NUMBER_OF_ASCII_CODES] =
{
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE,
	_key_space, _key_1, _key_apostrophe, _key_3, _key_4, _key_5, _key_7, _key_apostrophe,
	_key_9, _key_0, _key_8, _key_equal, _key_comma, _key_dash, _key_period, _key_forwardslash,
	_key_0, _key_1, _key_2, _key_3, _key_4, _key_5, _key_6, _key_7,
	_key_8, _key_9, _key_semicolon, _key_semicolon, _key_comma, _key_equal, _key_period, _key_forwardslash,
	_key_2, _key_a, _key_b, _key_c, _key_d, _key_e, _key_f, _key_g,
	_key_h, _key_i, _key_j, _key_k, _key_l, _key_m, _key_n, _key_o,
	_key_p, _key_q, _key_r, _key_s, _key_t, _key_u, _key_v, _key_w,
	_key_x, _key_y, _key_z, _key_left_bracket, _key_backslash, _key_right_bracket, _key_6, _key_dash,
	_key_backquote, _key_a, _key_b, _key_c, _key_d, _key_e, _key_f, _key_g,
	_key_h, _key_i, _key_j, _key_k, _key_l, _key_m, _key_n, _key_o,
	_key_p, _key_q, _key_r, _key_s, _key_t, _key_u, _key_v, _key_w,
	_key_x, _key_y, _key_z, _key_left_bracket, _key_backslash, _key_right_bracket, _key_backquote, _key_delete
};

/* ---------- public code */

short fix_dead_zone(
	short value,
	short dead_range)
{
	if (value > dead_range)
	{
		return (short)((value - dead_range) * SHORT_MAX / (SHORT_MAX - dead_range));
	}

	if (value < -dead_range)
	{
		return (short)((value + dead_range) * SHORT_MIN / (SHORT_MIN + dead_range));
	}

	return 0;
}

void update_ticks(
	byte *ticks,
	boolean down)
{
	long value;

	if (down)
	{
		value = MIN(*ticks + 1, UNSIGNED_CHAR_MAX);
	}
	else
	{
		value = 0;
	}

	*ticks = (byte)value;

	return;
}

/* The native builds read input once a frame and draw several frames per
30 Hz tick (port/linux/game/render_interpolation.c), so counting frames would
make a held button count up as many times too fast: reload would turn into a
weapon swap after a few milliseconds. The count is 1 on the frame the button
goes down, as before, and from then on the ticks it has been held, at least
2 so a press is seen only once. */
static void update_hold_ticks(
	byte *ticks,
	long *down_time,
	boolean down)
{
	/* port: the clock is read only for a button held: this runs for every
	key each frame, and a read of the clock is slow in a browser */
	if (!down)
	{
		*ticks = 0;
	}
	else if (*ticks == 0)
	{
		*ticks = 1;
		*down_time = (long)system_milliseconds();
	}
	else
	{
		long held = (long)system_milliseconds() - *down_time;

		held = held < 0 ? 0 : MIN(held, (long)UNSIGNED_CHAR_MAX * 1000 / TICKS_PER_SECOND);
		*ticks = (byte)PIN(1 + held * TICKS_PER_SECOND / 1000, 2, UNSIGNED_CHAR_MAX);
	}

	return;
}

static void acquire_input_mutex(
	void)
{
	return;
}

static void release_input_mutex(
	void)
{
	return;
}

void input_suppress(
	void)
{
	input_globals.suppressed = TRUE;

	return;
}

void input_activate(
	void)
{
	return;
}

void input_deactivate(
	void)
{
	return;
}

void input_dispose(
	void)
{
	short gamepad_index;

	HATCleanup();
	if (input_globals.keyboard_handle != NULL)
	{
		XInputClose(input_globals.keyboard_handle);
		input_globals.keyboard_handle = NULL;
	}

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index] != NULL)
		{
			XInputClose(input_globals.gamepad_handles[gamepad_index]);
			input_globals.gamepad_handles[gamepad_index] = NULL;
		}
	}

	return;
}

void input_flush(
	void)
{
	csmemset(
		input_globals.gamepad_states,
		0,
		sizeof(input_globals.gamepad_states));
	csmemset(
		input_globals.key_ticks,
		0,
		sizeof(input_globals.key_ticks));
	csmemset(
		input_globals.key_latches,
		0,
		sizeof(input_globals.key_latches));
	input_globals.buffered_key_read_index = 0;
	input_globals.buffered_key_write_index = 0;
	csmemset(
		input_globals.buffered_keys,
		0,
		sizeof(input_globals.buffered_keys));

	return;
}

const struct mouse_state *input_get_mouse_state(
	void)
{
	return NULL;
}

boolean input_mouse_button_is_down(
	short button_index)
{
	return FALSE;
}

boolean input_has_gamepad(
	short gamepad_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\input\\input_xbox.c",
		0x171,
		gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	return input_globals.gamepad_handles[gamepad_index] != NULL;
}

const struct gamepad_state *input_get_gamepad_state(
	short gamepad_index)
{
	const struct gamepad_state *result = NULL;

	match_assert(
		"c:\\halo\\SOURCE\\input\\input_xbox.c",
		0x17B,
		gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	if (input_globals.gamepad_handles[gamepad_index])
	{
		result = input_globals.suppressed
			? &input_globals.suppressed_gamepad_state
			: &input_globals.gamepad_states[gamepad_index];
	}

	return result;
}

void input_set_gamepad_rumbler_state(
	short gamepad_index,
	word left_speed,
	word right_speed)
{
	match_assert(
		"c:\\halo\\SOURCE\\input\\input_xbox.c",
		0x198,
		gamepad_index>=0 && gamepad_index<MAXIMUM_GAMEPADS);

	if (!player_ui_rumble_disabled(gamepad_index))
	{
		input_globals.gamepad_rumbler_states[gamepad_index].left_frequency = left_speed;
		input_globals.gamepad_rumbler_states[gamepad_index].right_frequency = right_speed;
	}

	return;
}

boolean input_key_is_down(
	short key_code)
{
	boolean result = FALSE;

	if (!input_globals.suppressed)
	{
		switch (key_code)
		{
		case _key_shift:
			result = MAX(
				input_globals.key_ticks[_key_left_shift],
				input_globals.key_ticks[_key_right_shift]);
			break;
		case _key_control:
			result = MAX(
				input_globals.key_ticks[_key_left_control],
				input_globals.key_ticks[_key_right_control]);
			break;
		case _key_windows:
			result = MAX(
				input_globals.key_ticks[_key_left_windows],
				input_globals.key_ticks[_key_right_windows]);
			break;
		case _key_alt:
			result = MAX(
				input_globals.key_ticks[_key_left_alt],
				input_globals.key_ticks[_key_right_alt]);
			break;
		default:
			match_assert(
				"c:\\halo\\SOURCE\\input\\input_xbox.c",
				0x13A,
				key_code>=0 && key_code<NUMBER_OF_KEYS);
			result = input_globals.key_ticks[key_code];
			break;
		}
	}

	return result;
}

boolean input_get_key(
	struct key_stroke *key)
{
	boolean result = FALSE;

	if (input_globals.buffered_key_read_index < input_globals.buffered_key_write_index)
	{
		match_assert(
			"c:\\halo\\SOURCE\\input\\input_xbox.c",
			0x14E,
			input_globals.buffered_key_read_index>=0 &&
				input_globals.buffered_key_read_index<MAXIMUM_BUFFERED_KEYSTROKES);

		*key = input_globals.buffered_keys[input_globals.buffered_key_read_index++];
		result = TRUE;
	}

	return result;
}

void input_vertical_blank_interrupt(
	void)
{
	if (input_globals.update_event_pending)
	{
		SetEvent(input_globals.update_event_handle);
	}

	input_globals.update_event_pending = !input_globals.update_event_pending;

	return;
}

void input_get_raw_data_string(
	char *buffer,
	short size)
{
	match_assert(
		"c:\\halo\\SOURCE\\input\\input_xbox.c",
		0x32B,
		buffer);
	match_assert(
		"c:\\halo\\SOURCE\\input\\input_xbox.c",
		0x32C,
		size>0);

	if (buffer && size>0)
	{
		short buffer_length = (short)_snprintf(
			buffer,
			size,
			"|n|n|n|ngamepad|tleft stick|tright stick|t|n");
		short gamepad_index;

		for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
		{
			if (input_globals.gamepad_handles[gamepad_index] != NULL)
			{
				buffer_length += (short)_snprintf(
					buffer + buffer_length,
					size - buffer_length,
					"gamepad %d|t(%d, %d)|t(%d, %d)|n",
					gamepad_index,
					input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_left].x,
					input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_left].y,
					input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_right].x,
					input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_right].y);
			}
		}
	}

	return;
}

void input_update(
	void)
{
	input_globals.suppressed = FALSE;
	if (!input_globals.update_thread_started)
	{
		ResumeThread(input_globals.update_thread_handle);
		input_globals.update_thread_started = TRUE;
	}

	input_update_keyboard_devices();
	HATRun(&input_globals.gamepad_states[0]);
	HATRun(&input_globals.gamepad_states[1]);
	HATRun(&input_globals.gamepad_states[2]);
	HATRun(&input_globals.gamepad_states[3]);

	return;
}

void input_frame_begin(
	void)
{
	input_get_device_states();
	acquire_input_mutex();
	input_globals.frame_active = TRUE;

	return;
}

void input_frame_end(
	void)
{
	input_globals.frame_active = FALSE;
	release_input_mutex();

	return;
}

/* ---------- private code */

static void input_update_analog_button_state(
	byte *threshold,
	byte value,
	boolean pressed)
{
	byte new_threshold;

	if (pressed)
	{
		if (value < ANALOG_BUTTON_PRESS_THRESHOLD_OFFSET)
		{
			new_threshold = 0;
		}
		else
		{
			new_threshold = value - ANALOG_BUTTON_PRESS_THRESHOLD_OFFSET;
		}

		if (new_threshold > *threshold)
		{
			*threshold = new_threshold;
		}
	}
	else
	{
		if (value > ANALOG_BUTTON_RELEASE_SATURATION)
		{
			new_threshold = UNSIGNED_CHAR_MAX;
		}
		else
		{
			new_threshold = value + ANALOG_BUTTON_RELEASE_THRESHOLD_OFFSET;
		}

		if (new_threshold < *threshold)
		{
			*threshold = new_threshold;
		}
	}

	return;
}

static void input_get_device_states(
	void)
{
	unsigned long memory_unit_insertions;
	unsigned long memory_unit_removals;
	unsigned long device_change_flags = 0;
	short gamepad_index;

	{
		unsigned long insertions;
		unsigned long removals;

		if (XGetDeviceChanges(
			XDEVICE_TYPE_GAMEPAD,
			&insertions,
			&removals))
		{
			for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
			{
				if (TEST_FLAG(removals, gamepad_index))
				{
					match_assert(
						"c:\\halo\\SOURCE\\input\\input_xbox.c",
						0x217,
						input_globals.gamepad_handles[gamepad_index]);
					XInputClose(input_globals.gamepad_handles[gamepad_index]);
					input_globals.gamepad_handles[gamepad_index] = NULL;
				}

				if (TEST_FLAG(insertions, gamepad_index))
				{
					match_assert(
						"c:\\halo\\SOURCE\\input\\input_xbox.c",
						0x21E,
						input_globals.gamepad_handles[gamepad_index]==NULL);
					input_globals.gamepad_handles[gamepad_index] = XInputOpen(
						XDEVICE_TYPE_GAMEPAD,
						gamepad_index,
						XDEVICE_NO_SLOT,
						NULL);
					if (input_globals.gamepad_handles[gamepad_index] == NULL)
					{
						error(
							_error_silent,
							"XInputOpen (gamepad) failed (#%d) during input_update()",
							GetLastError());
					}
				}
			}

			if (TEST_FLAG(removals, XDEVICE_PORT0))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad0_removed, TRUE);
			}
			if (TEST_FLAG(removals, XDEVICE_PORT1))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad1_removed, TRUE);
			}
			if (TEST_FLAG(removals, XDEVICE_PORT2))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad2_removed, TRUE);
			}
			if (TEST_FLAG(removals, XDEVICE_PORT3))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad3_removed, TRUE);
			}
			if (TEST_FLAG(insertions, XDEVICE_PORT0))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad0_inserted, TRUE);
			}
			if (TEST_FLAG(insertions, XDEVICE_PORT1))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad1_inserted, TRUE);
			}
			if (TEST_FLAG(insertions, XDEVICE_PORT2))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad2_inserted, TRUE);
			}
			if (TEST_FLAG(insertions, XDEVICE_PORT3))
			{
				SET_FLAG(device_change_flags, _input_device_change_gamepad3_inserted, TRUE);
			}
		}
	}

	if (XGetDeviceChanges(
		XDEVICE_TYPE_MEMORY_UNIT,
		&memory_unit_insertions,
		&memory_unit_removals))
	{
		if (memory_unit_removals & XDEVICE_PORT0_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit0_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT0_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit1_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT1_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit2_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT1_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit3_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT2_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit4_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT2_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit5_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT3_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit6_removed, TRUE);
		}
		if (memory_unit_removals & XDEVICE_PORT3_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit7_removed, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT0_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit0_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT0_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit1_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT1_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit2_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT1_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit3_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT2_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit4_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT2_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit5_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT3_TOP_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit6_inserted, TRUE);
		}
		if (memory_unit_insertions & XDEVICE_PORT3_BOTTOM_MASK)
		{
			SET_FLAG(device_change_flags, _input_device_change_memory_unit7_inserted, TRUE);
		}
	}

	input_abstraction_update_device_changes(device_change_flags);

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index] != NULL)
		{
			XINPUT_STATE input_state;
			long result = (long)XInputGetState(
				input_globals.gamepad_handles[gamepad_index],
				&input_state);

			if (result >= 0)
			{
				struct gamepad_state *gamepad_state = &input_globals.gamepad_states[gamepad_index];
				short button_index;

				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_ANALOG_BUTTONS; button_index++)
				{
					gamepad_state->analog_buttons[button_index] =
						input_state.Gamepad.bAnalogButtons[gamepad_analog_button_indices[button_index]];
					update_hold_ticks(
						&gamepad_state->buttons[FIRST_GAMEPAD_ANALOG_BUTTON + button_index],
						&gamepad_button_down_times[gamepad_index][FIRST_GAMEPAD_ANALOG_BUTTON + button_index],
						gamepad_state->analog_buttons[button_index] > gamepad_state->analog_button_thresholds[button_index]);
					input_update_analog_button_state(
						&gamepad_state->analog_button_thresholds[button_index],
						gamepad_state->analog_buttons[button_index],
						gamepad_state->buttons[FIRST_GAMEPAD_ANALOG_BUTTON + button_index] != 0);
				}

				for (button_index = 0; button_index < NUMBER_OF_GAMEPAD_BINARY_BUTTONS; button_index++)
				{
					update_hold_ticks(
						&gamepad_state->buttons[FIRST_GAMEPAD_BINARY_BUTTON + button_index],
						&gamepad_button_down_times[gamepad_index][FIRST_GAMEPAD_BINARY_BUTTON + button_index],
						(input_state.Gamepad.wButtons & gamepad_binary_button_masks[button_index]) != 0);
				}

				input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_left].x = input_state.Gamepad.sThumbLX;
				input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_left].y = input_state.Gamepad.sThumbLY;
				input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_right].x = input_state.Gamepad.sThumbRX;
				input_globals.raw_gamepad_states[gamepad_index].sticks[_gamepad_stick_right].y = input_state.Gamepad.sThumbRY;

				/* port: each stick's dead range is a setting (halo_stick_dead_range), the
				game's GAMEPAD_STICK_DEAD_RANGE by default */
				gamepad_state->sticks[_gamepad_stick_left].x = fix_dead_zone(
					input_state.Gamepad.sThumbLX,
					halo_stick_dead_range(_gamepad_stick_left));
				gamepad_state->sticks[_gamepad_stick_left].y = fix_dead_zone(
					input_state.Gamepad.sThumbLY,
					halo_stick_dead_range(_gamepad_stick_left));
				gamepad_state->sticks[_gamepad_stick_right].x = fix_dead_zone(
					input_state.Gamepad.sThumbRX,
					halo_stick_dead_range(_gamepad_stick_right));
				gamepad_state->sticks[_gamepad_stick_right].y = fix_dead_zone(
					input_state.Gamepad.sThumbRY,
					halo_stick_dead_range(_gamepad_stick_right));
			}
			else
			{
				error(
					_error_silent,
					"XGetState (gamepad) failed (#%d) during input_update()",
					result);
			}
		}
	}

	return;
}

static void input_update_keyboard_devices(
	void)
{
	{
		unsigned long insertions;
		unsigned long removals;

		if (XGetDeviceChanges(
			XDEVICE_TYPE_DEBUG_KEYBOARD,
			&insertions,
			&removals))
		{
			long keyboard_index = 0;
			long keyboard_count = LONG_BITS;

			while (keyboard_count)
			{
				unsigned long keyboard_mask = FLAG(keyboard_index);

				if (removals & keyboard_mask)
				{
					match_assert(
						"c:\\halo\\SOURCE\\input\\input_xbox.c",
						0x2D5,
						input_globals.keyboard_handle);
					XInputClose(input_globals.keyboard_handle);
					input_globals.keyboard_handle = NULL;
				}

				if (insertions & keyboard_mask)
				{
					match_assert(
						"c:\\halo\\SOURCE\\input\\input_xbox.c",
						0x2DC,
						input_globals.keyboard_handle==NULL);
					input_globals.keyboard_handle = XInputOpen(
						XDEVICE_TYPE_DEBUG_KEYBOARD,
						keyboard_index,
						XDEVICE_NO_SLOT,
						NULL);
					if (input_globals.keyboard_handle == NULL)
					{
						error(
							_error_silent,
							"XInputOpen (keyboard) failed (#%d) during input_update()",
							GetLastError());
					}
				}

				keyboard_index++;
				keyboard_count--;
			}
		}
	}

	{
		short key_code;

		for (key_code = 0; key_code < NUMBER_OF_KEYS; key_code++)
		{
			update_hold_ticks(
				&input_globals.key_ticks[key_code],
				&key_down_times[key_code],
				input_globals.key_latches[key_code]);
		}
	}

	input_globals.buffered_key_read_index = 0;
	input_globals.buffered_key_write_index = 0;

	{
		XINPUT_DEBUG_KEYSTROKE keystroke;

		while (XInputDebugGetKeystroke(&keystroke) == ERROR_SUCCESS)
		{
			struct key_stroke key;

			key.modifier_flags =
				(keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_SHIFT) != 0;
			SET_FLAG(
				key.modifier_flags,
				_key_modifier_control,
				(keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_CTRL) != 0);
			SET_FLAG(
				key.modifier_flags,
				_key_modifier_alt,
				(keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_ALT) != 0);

			match_assert(
				"c:\\halo\\SOURCE\\input\\input_xbox.c",
				0x305,
				keystroke.Ascii>=0 && keystroke.Ascii<NUMBER_OF_ASCII_CODES);
			key.ascii_code = ascii_to_key_code[keystroke.Ascii] != NONE
				? keystroke.Ascii
				: NONE;

			match_assert(
				"c:\\halo\\SOURCE\\input\\input_xbox.c",
				0x308,
				keystroke.VirtualKey>=0 && keystroke.VirtualKey<NUMBER_OF_VIRTUAL_CODES);
			key.key_code = virtual_key_to_key_code[keystroke.VirtualKey];
			if (key.key_code != NONE)
			{
				match_assert(
					"c:\\halo\\SOURCE\\input\\input_xbox.c",
					0x30E,
					key.key_code>=0 && key.key_code<NUMBER_OF_KEYS);

				if (keystroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_KEYUP)
				{
					input_globals.key_latches[key.key_code] = FALSE;
					if (input_globals.key_ticks[key.key_code] > 1)
					{
						input_globals.key_ticks[key.key_code] = 0;
					}
				}
				else
				{
					if (input_globals.buffered_key_write_index < MAXIMUM_BUFFERED_KEYSTROKES)
					{
						input_globals.buffered_keys[
							input_globals.buffered_key_write_index++] = key;
					}

					input_globals.key_latches[key.key_code] = TRUE;
					if (input_globals.key_ticks[key.key_code] == 0)
					{
						input_globals.key_ticks[key.key_code] = 1;
						key_down_times[key.key_code] = (long)system_milliseconds();
					}
				}
			}
		}
	}

	return;
}

static void input_flush_rumble(
	void)
{
	boolean suppress_rumble =
		input_globals.suppressed ||
		console_is_active() ||
		game_time_get_paused() ||
		!game_in_progress();
	short gamepad_index;

	for (gamepad_index = 0; gamepad_index < MAXIMUM_GAMEPADS; gamepad_index++)
	{
		if (input_globals.gamepad_handles[gamepad_index] != NULL)
		{
			if (input_globals.gamepad_feedbacks[gamepad_index].Header.dwStatus == ERROR_SUCCESS)
			{
				input_globals.gamepad_feedbacks[gamepad_index].Rumble.wLeftMotorSpeed = suppress_rumble
					? 0
					: input_globals.gamepad_rumbler_states[gamepad_index].left_frequency;
				input_globals.gamepad_feedbacks[gamepad_index].Rumble.wRightMotorSpeed = suppress_rumble
					? 0
					: input_globals.gamepad_rumbler_states[gamepad_index].right_frequency;
				XInputSetState(
					input_globals.gamepad_handles[gamepad_index],
					&input_globals.gamepad_feedbacks[gamepad_index]);
			}
			else if (input_globals.gamepad_feedbacks[gamepad_index].Header.dwStatus != ERROR_IO_PENDING)
			{
				input_globals.gamepad_feedbacks[gamepad_index].Header.dwStatus = ERROR_SUCCESS;
			}
		}
	}

	return;
}

static DWORD WINAPI input_keyboard_thread(
	void *context)
{
	for (;;)
	{
		WaitForSingleObject(input_globals.update_event_handle, INFINITE);
		acquire_input_mutex();
		input_flush_rumble();
		release_input_mutex();
	}

	return ERROR_SUCCESS;
}

boolean input_initialize(
	void)
{
	XDEVICE_PREALLOC_TYPE device_types[] =
	{
		{ XDEVICE_TYPE_GAMEPAD, MAXIMUM_GAMEPADS },
		{ XDEVICE_TYPE_DEBUG_KEYBOARD, 1 },
		{ XDEVICE_TYPE_MEMORY_UNIT, 8 }
	};
	XINPUT_DEBUG_KEYQUEUE_PARAMETERS keyboard_queue_parameters;
	long result;

	csmemset(
		&input_globals.suppressed,
		0,
		sizeof(input_globals) - offsetof(struct input_globals, suppressed));
	XInitDevices(NUMBEROF(device_types), device_types);

	input_globals.update_event_pending = TRUE;
	input_globals.update_event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	input_globals.update_thread_handle = CreateThread(
		NULL,
		0x4000,
		input_keyboard_thread,
		NULL,
		CREATE_SUSPENDED,
		NULL);
	SetThreadPriority(input_globals.update_thread_handle, THREAD_PRIORITY_HIGHEST);
	input_globals.update_thread_started = TRUE;
	input_globals.keyboard_queue_state = 0;

	keyboard_queue_parameters.dwFlags =
		XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN |
		XINPUT_DEBUG_KEYQUEUE_FLAG_KEYREPEAT |
		XINPUT_DEBUG_KEYQUEUE_FLAG_KEYUP;
	keyboard_queue_parameters.dwQueueSize = MAXIMUM_BUFFERED_KEYSTROKES;
	keyboard_queue_parameters.dwRepeatDelay = 500;
	keyboard_queue_parameters.dwRepeatInterval = 100;
	result = (long)XInputDebugInitKeyboardQueue(&keyboard_queue_parameters);
	if (result < ERROR_SUCCESS)
	{
		error(
			_error_silent,
			"XInputDebugInitKeyboardQueue failed (#%d) during input_initialize()",
			result);
	}

	HATInit();
	input_get_device_states();
	input_update();
	input_globals.update_thread_started = FALSE;

	return TRUE;
}

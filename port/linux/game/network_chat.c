/*
NETWORK_CHAT.C

The text chat of network games (port/linux/NETCODE.md), as the PC version
had it: T (controls.chat) starts a line to everyone, Y (controls.team_chat)
one to the player's team; Tab, while typing, turns it to everyone, the team
or the vehicle in turn; Enter sends it, Escape lets it go. A controller's
D-pad Up (the touch controls' too) opens an on-screen keyboard instead,
whose keys the stick moves through and A types, a tap types (the
touchscreen), and the controller then drives nothing else (xinput_sdl.c's
halo_chat_pad_buttons, halo_chat_taps). The lines are
drawn above the motion sensor, at half the HUD font's size, each with its
player's name in their team's colour, the host's name white, wrapped to a
column a little over half the view wide, and fade after a while; where the
motion sensor is not drawn
(its player dead), where it was last drawn in a view of that size. While a
line is typed, Page Up and Page Down (and the mouse wheel) scroll back through
the last CHAT_MAXIMUM_LINES. Where the touch controls hold the bottom left
(Android, and a browser whose touch controls are used), they go at the top
left under the weapon's panel, the newest highest, at the font's own size. Each local player's view shows the lines that
player hears; the first local player types, and their view shows the line
being typed (its end, when it is wider than the column). No line is typed or
shown in a menu, a cinematic, or while the HUD is hidden, and the lines of a
game are let go when the next starts.

A client sends its line to the host (_distributed_message_chat_up,
reliably), which takes it only from the client's own stream, from one of
that machine's players, at most CHAT_BURST lines at once and one each
CHAT_REFILL_MILLISECONDS after (a machine new to its slot starting afresh),
its text cut to printable ASCII ("|", which the game's text drawing takes
for the start of a code, becomes "?"). The host
sends it on to every machine that hears it (_distributed_message_chat_down,
reliably): all of them, a team's line only those with a player of the
team (every machine in a game without teams), and a vehicle's line only
those with a player riding in the vehicle the sender rides in (the
outermost: a vehicle on another's back is its), whatever the map's vehicles
are, and the sender's own. A client shows only what the
host sends, its own lines too, so every machine shows the same lines in the
same order; it takes the host's lines and settings only over the host's
connection's stream, which no other machine can send as the host. A build
without the chat drops the three kinds as unknown: its players neither send
nor see lines.

The host's settings (network.text_chat: on, links allowed, or off) go to
every client (_distributed_message_chat_config), in every game: a private
or LAN one, a public one a player hosts and a dedicated server's alike. A
line they refuse is refused by its sender's machine and again by the host,
which tells the sender why; a client without them (from a host without the
chat) has the chat off. display.text_chat hides the chat on this machine.

The host may type "/mute <player> [minutes]" (20 when it gives none, a day
at most) or "/unmute <player>", the player as the console's kick finds one
(the whole name in either case, else the one it begins). It keeps the mute
by the player's name, in either case (the host keeps players' names apart
in either case), which then sends no line further than the host till
the minutes are up (or the host unmutes it) and is told for how long; every
machine is told of each mute and unmute, a line of the host's alone in the
chat (_chat_line_notice_bit). A client's /mute is not sent: only the host
mutes.

A dedicated server takes a player's "!switch" or "!team" (the whole line)
as their asking to change teams (server_team_switch.c): the line is sent on
to no one, and the host's notices say what came of it.
*/

#include "controller_remap.h"
#include "cseries.h"
#include "cseries/errors.h"
#include "memory/data.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "bots.h"
#include "input/input.h"
#include "interface/hud.h"
#include "interface/hud_draw.h"
#include "interface/hud_messaging.h"
#include "interface/ui_widget.h"
#include "interface/terminal.h"
#include "main/console.h"
#include "objects/objects.h"
#include "cutscene/cinematics.h"
#include "rasterizer/rasterizer.h"
#include "render/render.h"
#include "text/draw_string.h"
#include "text/font_group.h"
#include "text/text_group.h"
#include "network_distributed.h"
#include "network_votekick.h"
#include "network_voice.h"
#include "network_chat.h"
#include "text_filter.h"
#include "halo_keyboard.h"
#include "server_director.h"
#include "server_team_switch.h"

#include <stdio.h>

/* the platform layer's */
unsigned long system_milliseconds(void);
int platform_clipboard_get(char *text, int size);
/* port_config.c's */
int config_boolean(const char *name);
const char *config_string(const char *name);
unsigned long config_changes(void);
/* server_mode.c's: a dedicated server (halo --server) */
int halo_server_mode(void);
void platform_log(char const *format, ...);
/* the machine this game is played on (network_game_globals.c) */
short network_game_client_get_local_machine_index(void);

/* ---------- constants */

enum
{
	/* a line's text, with its end */
	CHAT_TEXT_SIZE = 64,
	/* a player's name, with its end */
	CHAT_NAME_SIZE = 12,
	/* the lines kept (Page Up scrolls back through them while a line is
	typed), and the rows of them shown at once (a line wraps) */
	CHAT_MAXIMUM_LINES = 64,
	CHAT_LINES_SHOWN = 6,
	/* ... in a view of split screen */
	CHAT_LINES_SHOWN_SPLIT = 3,
	/* the rows a line wraps to, at most, and the column's width, in hundredths
	of the view's (clear of the on-screen keyboard at the bottom right) */
	CHAT_LINE_ROWS = 3,
	CHAT_COLUMN_PERCENT = 55,
	/* the lines Page Up and Page Down scroll by (the mouse wheel, one a notch) */
	CHAT_PAGE_LINES = 3,
	/* a view whose HUD has drawn no motion sensor: about where it would be,
	in the HUD's units (480 high, before its scale), from the view's bottom
	left */
	CHAT_FALLBACK_LEFT_UNITS = 24,
	CHAT_FALLBACK_BOTTOM_UNITS = 110,
	/* a line shown, then faded out */
	CHAT_LINE_MILLISECONDS = 10000,
	CHAT_FADE_MILLISECONDS = 1000,
	/* the host: the lines a machine sends at once, and the time it gains
	another in */
	CHAT_BURST = 5,
	CHAT_REFILL_MILLISECONDS = 1500,
	MAXIMUM_CHAT_MACHINES = HALO_PORT_MAXIMUM_NETWORK_MACHINES,
	/* the caret's half period while typing */
	CHAT_CARET_MILLISECONDS = 500,
	/* with the touch controls: the top of the lines, in the HUD's units (480
	high, before its scale), below the tallest weapon's panel at the top left */
	CHAT_TOP_UNITS = 60,
	/* with the touch controls: the rows shown at once there, the one being
	typed among them (a fifth would reach the tops of their Up and Fire) */
	CHAT_LINES_SHOWN_ANDROID = 4,
	/* a key's Ctrl (struct key_stroke's modifier_flags: menu_functions.c's) */
	CHAT_KEY_MODIFIER_CONTROL_BIT = 1,
	/* the host: its chat settings sent again this often (to a machine that
	joined since) */
	CHAT_CONFIG_MILLISECONDS = 3000,
	/* the host's /mute: the minutes when it gives none, and the most (a day) */
	CHAT_MUTE_DEFAULT_MINUTES = 20,
	CHAT_MUTE_MAXIMUM_MINUTES = 1440,
	CHAT_MILLISECONDS_PER_MINUTE = 60000,
	/* the players muted at once */
	CHAT_MAXIMUM_MUTES = MAXIMUM_TRACKED_PLAYERS,
	/* (chat_name_match) a name more than one player's begins with */
	CHAT_NAME_AMBIGUOUS = -2,
	/* the on-screen keyboard: its columns, and its rows (the last of wide
	keys, each CHAT_KEYBOARD_WIDE_COLUMNS of the columns) */
	CHAT_KEYBOARD_COLUMNS = 10,
	CHAT_KEYBOARD_ROWS = 5,
	CHAT_KEYBOARD_WIDE_COLUMNS = 2,
	/* a direction held: the key moves once, then again after the first
	delay and at each repeat */
	CHAT_KEYBOARD_REPEAT_FIRST_MILLISECONDS = 350,
	CHAT_KEYBOARD_REPEAT_MILLISECONDS = 120,
	/* its width, in hundredths of the window's: at the bottom right, clear
	of the lines above the motion sensor; on Android at the bottom in the
	middle, between the touch controls (the lines are at the top left) */
	CHAT_KEYBOARD_WIDTH_PERCENT = 40,
	CHAT_KEYBOARD_WIDTH_PERCENT_ANDROID = 36,
};

/* the lines' size, of the HUD font's: laid out at its size, then drawn scaled
about the column's corner (not with the touch controls, on a phone's screen) */
#define CHAT_TEXT_SCALE 0.5f

/* the on-screen keyboard's last row's wide keys */
enum
{
	_chat_key_shift,
	_chat_key_symbols,
	_chat_key_space,
	_chat_key_delete,
	_chat_key_send,
	NUMBER_OF_CHAT_WIDE_KEYS
};

/* the line's flags */
enum
{
	/* to the sender's team alone */
	_chat_line_team_bit,
	/* to those in the sender's vehicle alone */
	_chat_line_vehicle_bit,
	/* the host's notice (a mute's): its text alone, from no player */
	_chat_line_notice_bit,
	/* the sender is a person on the machine running the game. the host
	sets it when it sends the line on. a name so marked is drawn white */
	_chat_line_host_bit,
};

/* the host's chat settings' flags (_distributed_message_chat_config) */
enum
{
	/* its players may chat */
	_chat_config_allowed_bit,
	/* their lines may have links */
	_chat_config_links_bit,
	/* pad[0] is the machine running the game. a build that does not set
	this leaves that byte 0, which is that machine when it joined first */
	_chat_config_host_machine_bit,
};

/* who a line is to (network_chat.h's _chat_channel_*) */

/* a line's colour: its sender's team's, or a notice's */
enum
{
	_chat_color_red,
	_chat_color_blue,
	_chat_color_notice,
	/* a person on the machine running the game */
	_chat_color_host,
};

/* what a typed line asks (chat_command_parse) */
enum
{
	/* nothing: it is said */
	_chat_command_none,
	_chat_command_mute,
	_chat_command_unmute,
	/* /mute or /unmute without a player, or a mute of no minutes */
	_chat_command_usage,
};

/* ---------- structures */

/* a line: up, the sender as it says (one of its machine's players); down,
the sender as the host found it */
struct distributed_chat_line
{
	/* the sender (absolute index) */
	byte player_index;
	byte flags;
	byte pad[2];
	char text[CHAT_TEXT_SIZE];
};

typedef char distributed_chat_line_size_assert[sizeof(struct distributed_chat_line) == 68 ? 1 : -1];

/* a message of one line */
struct chat_message
{
	struct distributed_message_header header;
	struct distributed_chat_line line;
};

/* the host's chat settings */
struct distributed_chat_config
{
	byte flags;
	byte pad[3];
};

typedef char distributed_chat_config_size_assert[sizeof(struct distributed_chat_config) == 4 ? 1 : -1];

/* the host: a machine's lines allowed (chat_rate_take) */
struct chat_rate
{
	boolean started;
	short tokens;
	/* (system_milliseconds) when the last token came, or the bucket was last full */
	unsigned long refilled;
};

/* the host: a player muted, by name (one who leaves and joins again under
it still is), till its minutes are up */
struct chat_mute
{
	boolean active;
	char name[CHAT_NAME_SIZE];
	/* (system_milliseconds) */
	unsigned long started;
	unsigned long milliseconds;
};

/* a line shown */
struct chat_line
{
	wchar_t name[CHAT_NAME_SIZE];
	char text[CHAT_TEXT_SIZE];
	short color;
	short channel;
	/* the local players who hear it, a bit for each (their views show it) */
	short hearers;
	/* (system_milliseconds) */
	unsigned long time;
};

/* ---------- globals */

/* the lines, the newest at chat_newest_line */
static struct chat_line chat_lines[CHAT_MAXIMUM_LINES];
static short chat_line_count;
static short chat_newest_line;
/* while a line is typed: the newest lines scrolled past (Page Up), 0 for
none */
static short chat_scroll;

/* the line typed */
static struct
{
	boolean active;
	short channel, controller;
	long player;
	boolean physical;
	char text[CHAT_TEXT_SIZE];
} chat_input;

/* the keyboard's chat actions held last frame (a press starts a line) */
static unsigned long chat_actions_held;

/* the on-screen keyboard, for a controller (D-pad Up opens it) and the
touchscreen (its keys tapped) */
static struct
{
	boolean open;
	unsigned long opening_buttons;
	/* the key picked */
	short row;
	short column;
	/* capitals; the symbols' page */
	boolean shift;
	boolean symbols;
	/* the controller's buttons last frame (a press acts), and (system_
	milliseconds) when a held direction moves the key again */
	unsigned long held;
	unsigned long repeat_at;
	/* where its keys were last drawn, in thousandths of the window across
	and down (a tap's place: halo_chat_taps) */
	boolean drawn;
	short left;
	short top;
	short right;
	short bottom;
} chat_keyboard;

/* its keys' characters but the last row's, the letters' and the symbols' */
static char const *const chat_keyboard_letters[CHAT_KEYBOARD_ROWS - 1] =
	{ "1234567890", "qwertyuiop", "asdfghjkl'", "zxcvbnm,.?" };
/* (no "|": chat_text_clean) */
static char const *const chat_keyboard_symbols[CHAT_KEYBOARD_ROWS - 1] =
	{ "1234567890", "!@#$%^&*()", "-_=+[]{};:", "\"'<>/\\?~,." };

/* the host's */
static struct chat_rate chat_rates[MAXIMUM_CHAT_MACHINES];
static struct chat_mute chat_mutes[CHAT_MAXIMUM_MUTES];

/* the host's chat settings: a client's as the host last sent them (none
till it does, as from a host without the chat); the host's own as it last
sent them */
static struct
{
	boolean known;
	byte flags;
	/* (the host, system_milliseconds) */
	unsigned long sent;
	/* the machine running the game, once its settings have named it */
	boolean host_named;
	short host_machine;
} chat_config;

/* each local player's motion sensor as last drawn (the window's units),
whether it has been, and the size of the view and the local players then: its
place in the HUD while they stay, drawn or not (a dead player's is not) */
static struct
{
	boolean valid;
	short left;
	short top;
	short view_width;
	short view_height;
	short players;
} chat_motion_sensors[MAXIMUM_LOCAL_PLAYERS];

/* whether this machine is in a network game (the lines of the last are let
go as the next starts) */
static boolean chat_in_game;

/* (system_milliseconds) when "Text chat is off in this game" was last said
(once a while, however often D-pad Up is pressed) */
static unsigned long chat_off_noticed_at;
static boolean chat_off_noticed;

/* ---------- the rules (pure: tools/harness/tests/chat_logic.c) */

/* what another machine sent, or the keyboard typed, as a line's text: at most
CHAT_TEXT_SIZE - 1 characters, to the source's end or source_size (what
another machine sent need not be ended), printable ASCII but "|" ("?" for
anything else: the game's text drawing takes a "|" for the start of a code,
and one at the end reads past it, draw_string.c), without the spaces it
begins and ends with; its length */
static short chat_text_clean(
	char *destination,
	char const *source,
	short source_size)
{
	short length = 0;
	short index;

	for (index = 0; index < source_size && source[index] && length < CHAT_TEXT_SIZE - 1; index++)
	{
		char character = source[index];

		if (character < ' ' || character > '~' || character == '|')
			character = '?';
		if (character == ' ' && length == 0)
			continue;
		destination[length++] = character;
	}
	while (length > 0 && destination[length - 1] == ' ')
		length--;
	destination[length] = 0;
	return length;
}

/* the typed text (or pasted) added to a line, as far as it holds: printable
ASCII but "|" only (chat_text_clean), the rest left out */
static void chat_text_append(
	char *text,
	char const *typed)
{
	short length = (short)csstrlen(text);

	for (; *typed && length < CHAT_TEXT_SIZE - 1; typed++)
	{
		if (*typed >= ' ' && *typed <= '~' && *typed != '|')
			text[length++] = *typed;
	}
	text[length] = 0;
}

/* whether a machine may send a line now (now: system_milliseconds), and it
counted: CHAT_BURST at once, and one more each CHAT_REFILL_MILLISECONDS
since the last came (or since the bucket was last full) */
static boolean chat_rate_take(
	struct chat_rate *rate,
	unsigned long now)
{
	unsigned long gained;

	if (!rate->started)
	{
		rate->started = TRUE;
		rate->tokens = CHAT_BURST;
		rate->refilled = now;
	}
	gained = (now - rate->refilled) / CHAT_REFILL_MILLISECONDS;
	if (gained > 0)
	{
		rate->tokens = (short)MIN((unsigned long)CHAT_BURST, rate->tokens + gained);
		rate->refilled += gained * CHAT_REFILL_MILLISECONDS;
	}
	/* (full: the next token comes a whole period after one is spent) */
	if (rate->tokens >= CHAT_BURST)
		rate->refilled = now;
	if (rate->tokens <= 0)
		return FALSE;
	rate->tokens--;
	return TRUE;
}

/* whether a listener hears a line of a channel: one to everyone; a team's
by the same team, or by everyone in a game without teams; a vehicle's by
those riding in the vehicle the sender rides in (NONE: none), none when the
sender rides in none (the sender sees their own lines whatever this says) */
static boolean chat_hears(
	short channel,
	boolean has_teams,
	long sender_team,
	long listener_team,
	long sender_vehicle,
	long listener_vehicle)
{
	switch (channel)
	{
	case _chat_channel_team:
		return !has_teams || sender_team == listener_team;
	case _chat_channel_vehicle:
		return sender_vehicle != NONE && sender_vehicle == listener_vehicle;
	default:
		return TRUE;
	}
}

/* a line's channel from its flags, and back */
static short chat_channel_from_flags(
	byte flags)
{
	if (TEST_FLAG(flags, _chat_line_vehicle_bit))
		return _chat_channel_vehicle;
	return TEST_FLAG(flags, _chat_line_team_bit) ? _chat_channel_team : _chat_channel_everyone;
}

static byte chat_channel_flags(
	short channel)
{
	if (channel == _chat_channel_vehicle)
		return (byte)FLAG(_chat_line_vehicle_bit);
	return channel == _chat_channel_team ? (byte)FLAG(_chat_line_team_bit) : 0;
}

/* how much of a line shows at its age (milliseconds): all of it for a while,
then fading out; all the lines kept while a line is typed */
static real chat_line_alpha(
	unsigned long age,
	boolean typing)
{
	if (typing || age < CHAT_LINE_MILLISECONDS)
		return 1.0f;
	if (age >= CHAT_LINE_MILLISECONDS + CHAT_FADE_MILLISECONDS)
		return 0.0f;
	return 1.0f - (real)(age - CHAT_LINE_MILLISECONDS) / (real)CHAT_FADE_MILLISECONDS;
}

static char chat_lower(
	char character)
{
	return character >= 'A' && character <= 'Z' ? (char)(character - 'A' + 'a') : character;
}

/* what follows a command's word (lower case) at the start of a line, in
either case, after its spaces; NULL when the line does not start with it
(a word of its own: "/muted" is not "/mute") */
static char const *chat_command_word(
	char const *text,
	char const *word)
{
	short index;

	for (index = 0; word[index]; index++)
	{
		if (chat_lower(text[index]) != word[index])
			return NULL;
	}
	if (text[index] && text[index] != ' ')
		return NULL;
	while (text[index] == ' ')
		index++;
	return text + index;
}

/* whether a line (chat_text_clean's) is a dedicated server's player's team
change, "!switch" or "!team" and nothing more, in either case (one with more
is said: "!team push" is talk) */
static boolean chat_team_switch_command(
	char const *text)
{
	char const *rest = chat_command_word(text, "!switch");

	if (!rest)
		rest = chat_command_word(text, "!team");
	return rest && !*rest;
}

/* what a typed line (chat_text_clean's) asks: "/mute <player> [minutes]" or
"/unmute <player>", the word in either case; the player's name as typed (at
most name_size with its end: it may have spaces) and the minutes (a mute's
last word when all digits after a name; CHAT_MUTE_DEFAULT_MINUTES when it
gives none, CHAT_MUTE_MAXIMUM_MINUTES at most); _chat_command_none for any
other line, said as it is */
static short chat_command_parse(
	char const *text,
	char *name,
	short name_size,
	short *minutes)
{
	char const *rest;
	short command;
	short length;
	short last_word;
	short index;

	name[0] = 0;
	*minutes = CHAT_MUTE_DEFAULT_MINUTES;
	if ((rest = chat_command_word(text, "/mute")) != NULL)
		command = _chat_command_mute;
	else if ((rest = chat_command_word(text, "/unmute")) != NULL)
		command = _chat_command_unmute;
	else
		return _chat_command_none;
	length = (short)csstrlen(rest);
	for (last_word = length; last_word > 0 && rest[last_word - 1] != ' '; last_word--)
		;
	if (command == _chat_command_mute && last_word > 0 && last_word < length)
	{
		long typed_minutes = 0;

		for (index = last_word; index < length && rest[index] >= '0' && rest[index] <= '9'; index++)
			typed_minutes = MIN(typed_minutes * 10 + (rest[index] - '0'), (long)CHAT_MUTE_MAXIMUM_MINUTES);
		if (index == length)
		{
			if (typed_minutes == 0)
				return _chat_command_usage;
			*minutes = (short)typed_minutes;
			for (length = last_word; length > 0 && rest[length - 1] == ' '; length--)
				;
		}
	}
	for (index = 0; index < length && index < name_size - 1; index++)
		name[index] = rest[index];
	name[index] = 0;
	return index ? command : _chat_command_usage;
}

/* the player a typed name means among the players' names (ASCII; "" where
there is no player), as the console's kick and ban find one
(network_server_manager.c): the one of that name, in either case, else the
one whose name begins with it; NONE for none, CHAT_NAME_AMBIGUOUS for more
than one */
static short chat_name_match(
	char const *typed,
	char const (*names)[CHAT_NAME_SIZE],
	short count)
{
	short exact_index = NONE;
	short exact_count = 0;
	short begins_index = NONE;
	short begins_count = 0;
	short index;

	for (index = 0; typed[0] && index < count; index++)
	{
		char const *name = names[index];
		short length;

		/* (a name's end differs from any character typed: never read past it) */
		for (length = 0; typed[length] && chat_lower(name[length]) == chat_lower(typed[length]); length++)
			;
		if (typed[length])
			continue;
		begins_index = index;
		begins_count++;
		if (!name[length])
		{
			exact_index = index;
			exact_count++;
		}
	}
	if (exact_count == 1)
		return exact_index;
	if (exact_count > 1 || begins_count > 1)
		return CHAT_NAME_AMBIGUOUS;
	return begins_index;
}

/* whether two names (ASCII) are the same in either case, as the host keeps
players' names apart (network_server_message_handler.c): "Bob" muted, "bob"
is */
static boolean chat_names_equal(
	char const *a,
	char const *b)
{
	short index;

	for (index = 0; chat_lower(a[index]) == chat_lower(b[index]); index++)
	{
		if (!a[index])
			return TRUE;
	}
	return FALSE;
}

/* the minutes a mute has left at now (system_milliseconds), the one begun
counted whole; 0 once it has run out, when it is let go */
static short chat_mute_minutes_left(
	struct chat_mute *mute,
	unsigned long now)
{
	unsigned long elapsed = now - mute->started;

	if (!mute->active)
		return 0;
	if (elapsed >= mute->milliseconds)
	{
		mute->active = FALSE;
		return 0;
	}
	return (short)((mute->milliseconds - elapsed + CHAT_MILLISECONDS_PER_MINUTE - 1) / CHAT_MILLISECONDS_PER_MINUTE);
}

/* the mute of a player's name (in either case), while it runs; NULL */
static struct chat_mute *chat_mute_find(
	struct chat_mute *mutes,
	short count,
	char const *name,
	unsigned long now)
{
	short index;

	for (index = 0; index < count; index++)
	{
		if (chat_mute_minutes_left(&mutes[index], now) && chat_names_equal(mutes[index].name, name))
			return &mutes[index];
	}
	return NULL;
}

/* a player's name muted for minutes from now: its mute again if it has one,
else one run out; FALSE when every one is held */
static boolean chat_mute_set(
	struct chat_mute *mutes,
	short count,
	char const *name,
	short minutes,
	unsigned long now)
{
	struct chat_mute *mute = chat_mute_find(mutes, count, name, now);
	short index;

	for (index = 0; !mute && index < count; index++)
	{
		if (!chat_mute_minutes_left(&mutes[index], now))
			mute = &mutes[index];
	}
	if (!mute)
		return FALSE;
	mute->active = TRUE;
	csstrncpy(mute->name, name, CHAT_NAME_SIZE - 1);
	mute->name[CHAT_NAME_SIZE - 1] = 0;
	mute->started = now;
	mute->milliseconds = (unsigned long)minutes * CHAT_MILLISECONDS_PER_MINUTE;
	return TRUE;
}

/* the minutes in a notice: "5 minutes", "1 minute" */
static char const *chat_minutes_plural(
	short minutes)
{
	return minutes == 1 ? "" : "s";
}

/* the character of an on-screen keyboard's key (rows: its page's characters,
but the last row's), a letter in capitals with shift; 0 for a key of the
last row, or none */
static char chat_keyboard_character(
	char const *const *rows,
	short row,
	short column,
	boolean shift)
{
	char character;

	if (row < 0 || row >= CHAT_KEYBOARD_ROWS - 1 || column < 0 || column >= CHAT_KEYBOARD_COLUMNS)
		return 0;
	character = rows[row][column];
	if (shift && character >= 'a' && character <= 'z')
		character = (char)(character - 'a' + 'A');
	return character;
}

/* the on-screen keyboard's key picked moved by rows down and columns right,
round its edges; on the last row a column is its wide key's first */
static void chat_keyboard_move(
	short *row,
	short *column,
	short rows_down,
	short columns_right)
{
	short width;

	*row = (short)((*row + rows_down % CHAT_KEYBOARD_ROWS + CHAT_KEYBOARD_ROWS) % CHAT_KEYBOARD_ROWS);
	width = *row == CHAT_KEYBOARD_ROWS - 1 ? CHAT_KEYBOARD_WIDE_COLUMNS : 1;
	*column = (short)(*column - *column % width);
	*column = (short)((*column + columns_right * width % CHAT_KEYBOARD_COLUMNS + CHAT_KEYBOARD_COLUMNS) %
		CHAT_KEYBOARD_COLUMNS);
}

/* the on-screen keyboard's key at a place in it (0 to 1 across and down);
FALSE outside it */
static boolean chat_keyboard_key_at(
	real x,
	real y,
	short *row,
	short *column)
{
	if (x < 0.0f || x >= 1.0f || y < 0.0f || y >= 1.0f)
		return FALSE;
	*row = (short)(y * CHAT_KEYBOARD_ROWS);
	*column = (short)(x * CHAT_KEYBOARD_COLUMNS);
	if (*row == CHAT_KEYBOARD_ROWS - 1)
		*column = (short)(*column - *column % CHAT_KEYBOARD_WIDE_COLUMNS);
	return TRUE;
}

static boolean chat_alphanumeric(
	char character)
{
	return (character >= 'a' && character <= 'z') || (character >= '0' && character <= '9');
}

/* how a top-level domain after a name and a dot counts (chat_text_has_link;
letters: count of them, lower case): 2 always ("discord.gg", "example.com");
1, a word as well ("me", "to", "is"), only with a path, a port or another
domain after it ("youtu.be/x", "t.co/x", "example.co.uk"), as "go.to" and
"he.is" are slips of typing; 0 for none */
static short chat_link_domain(
	char const *letters,
	short count)
{
	static char const *const domains[] =
	{
		"com", "net", "org", "gg", "io", "tv", "uk", "ru", "de", "fr", "nl", "eu", "info", "biz", "xyz", "ly", "app",
		"dev", "cc", "ca", "au", "stream", "shop", "site", "online", "club", "gl", "ws", "cn", "jp", "kr", "br", "es",
		"pl", "se", "tk", "icu", "vip", "edu", "gov", "mil", "ai", "sh", "fm", "gd", "im", "st", "su",
	};
	static char const *const words[] =
		{ "me", "us", "to", "co", "be", "in", "is", "it", "fun", "top", "win", "pro", "lol", "live", "link", "int" };
	short index;

	for (index = 0; index < (short)NUMBEROF(domains); index++)
	{
		if ((short)csstrlen(domains[index]) == count && !strncmp(letters, domains[index], count))
			return 2;
	}
	for (index = 0; index < (short)NUMBEROF(words); index++)
	{
		if ((short)csstrlen(words[index]) == count && !strncmp(letters, words[index], count))
			return 1;
	}
	return 0;
}

/* whether a line (chat_text_clean's) has a link, in either case: a scheme
("http://", any "://"), "www.", an address of four numbers, or a name and a
top-level domain (chat_link_domain), its dots also written "(dot)", "[dot]",
"{dot}", "(.)", "[.]" or " dot " (before a domain that is no word: "polka
dot me" is none), with spaces about them or not, or with spaces both sides
("example . com") */
static boolean chat_text_has_link(
	char const *text)
{
	static char const *const dots[] = { "(dot)", "[dot]", "{dot}", "(.)", "[.]", " dot " };
	char line[CHAT_TEXT_SIZE];
	short length = 0;
	short index;

	/* (in lower case, each dot written out a dot, without the spaces about it) */
	for (index = 0; text[index] && length < CHAT_TEXT_SIZE - 1;)
	{
		short dot;
		short at = 0;

		for (dot = 0; dot < (short)NUMBEROF(dots); dot++)
		{
			for (at = 0; dots[dot][at] && chat_lower(text[index + at]) == dots[dot][at]; at++)
				;
			if (!dots[dot][at])
				break;
		}
		if (dot < (short)NUMBEROF(dots) && dots[dot][0] == ' ')
		{
			char letters[8];
			short count;

			for (count = 0; count < (short)sizeof(letters) && chat_lower(text[index + at + count]) >= 'a' &&
				chat_lower(text[index + at + count]) <= 'z'; count++)
			{
				letters[count] = chat_lower(text[index + at + count]);
			}
			if (count == (short)sizeof(letters) || chat_link_domain(letters, count) != 2)
				dot = (short)NUMBEROF(dots);
		}
		if (dot == (short)NUMBEROF(dots))
		{
			/* (a dot with spaces both sides) */
			if (text[index] != '.' || !length || line[length - 1] != ' ' || text[index + 1] != ' ')
			{
				line[length++] = chat_lower(text[index++]);
				continue;
			}
			at = 1;
		}
		index = (short)(index + at);
		while (length > 0 && line[length - 1] == ' ')
			length--;
		while (text[index] == ' ')
			index++;
		line[length++] = '.';
	}
	line[length] = 0;
	for (index = 0; index < length; index++)
	{
		if (!strncmp(line + index, "://", 3) || (!strncmp(line + index, "www.", 4) &&
			(index == 0 || !chat_alphanumeric(line[index - 1]))))
		{
			return TRUE;
		}
		/* (a name, a dot and a domain, all of its letters, which ends there) */
		if (line[index] == '.' && index > 0 && chat_alphanumeric(line[index - 1]))
		{
			short end;
			short domain;

			for (end = (short)(index + 1); line[end] >= 'a' && line[end] <= 'z'; end++)
				;
			if (end - index - 1 < 2 || chat_alphanumeric(line[end]))
				continue;
			domain = chat_link_domain(line + index + 1, (short)(end - index - 1));
			if (domain == 2 || (domain == 1 && (line[end] == '/' || line[end] == ':' ||
				(line[end] == '.' && line[end + 1] >= 'a' && line[end + 1] <= 'z'))))
			{
				return TRUE;
			}
		}
		/* (four numbers and three dots, and no more: "1.2.3.4.5" is a
		version's) */
		if (line[index] >= '0' && line[index] <= '9' &&
			(index == 0 || (!chat_alphanumeric(line[index - 1]) && line[index - 1] != '.')))
		{
			short at = index;
			short part;

			for (part = 0; part < 4; part++)
			{
				short digits;

				for (digits = 0; line[at] >= '0' && line[at] <= '9'; at++)
					digits++;
				if (!digits || digits > 3 || (part < 3 && line[at++] != '.'))
					break;
			}
			if (part == 4 && !(line[at] == '.' && line[at + 1] >= '0' && line[at + 1] <= '9'))
				return TRUE;
		}
	}
	return FALSE;
}

/* ---------- the lines */

static boolean chat_network_game(
	void)
{
	short connection = game_connection();

	return connection == _game_connection_network_client || connection == _game_connection_network_server;
}

/* whether this player shows the text chat (display.text_chat): hidden, no
line shows and its keys open nothing */
static boolean chat_shown(
	void)
{
	static boolean shown = TRUE;
	static unsigned long read_at = (unsigned long)-1;

	/* (read again when Settings changes it) */
	if (read_at != config_changes())
	{
		read_at = config_changes();
		shown = config_boolean("display.text_chat");
	}
	return shown;
}

/* ---------- the host's chat settings */

static boolean chat_host(
	void)
{
	return game_connection() == _game_connection_network_server;
}

/* (the host) its chat settings: network.text_chat's */
static byte chat_host_config_flags(
	void)
{
	char const *setting = config_string("network.text_chat");
	byte flags = 0;

	/* (in either case: HALO_NET_TEXT_CHAT=OFF is off) */
	if (setting && !chat_names_equal(setting, "off"))
	{
		flags |= (byte)FLAG(_chat_config_allowed_bit);
		if (chat_names_equal(setting, "links"))
			flags |= (byte)FLAG(_chat_config_links_bit);
	}
	return flags;
}

/* the host's chat settings: the host's own, or as it sent them (none, and
no chat, till it does) */
static byte chat_config_flags(
	void)
{
	if (chat_host())
		return chat_host_config_flags();
	return chat_config.known ? chat_config.flags : 0;
}

static void chat_send_to_clients(void *message, byte type, word size);

/* (the host, in a game) its chat settings sent to every client as they
change, and each CHAT_CONFIG_MILLISECONDS */
static void chat_host_update_config(
	void)
{
	byte flags = chat_host_config_flags();
	unsigned long now = system_milliseconds();
	short host_machine = network_game_client_get_local_machine_index();
	struct
	{
		struct distributed_message_header header;
		struct distributed_chat_config config;
	} message;

	if (chat_config.known && flags == chat_config.flags &&
		chat_config.host_named == (host_machine != NONE) &&
		(!chat_config.host_named || chat_config.host_machine == host_machine) &&
		now - chat_config.sent < CHAT_CONFIG_MILLISECONDS)
		return;
	chat_config.known = TRUE;
	chat_config.flags = flags;
	chat_config.host_named = host_machine != NONE;
	chat_config.host_machine = host_machine;
	chat_config.sent = now;
	csmemset(&message, 0, sizeof(message));
	message.config.flags = flags;
	if (host_machine != NONE)
	{
		message.config.flags |= (byte)FLAG(_chat_config_host_machine_bit);
		message.config.pad[0] = (byte)host_machine;
	}
	chat_send_to_clients(&message, _distributed_message_chat_config, (word)sizeof(message));
}

void network_chat_handle_config(
	void const *entries,
	boolean from_stream)
{
	struct distributed_chat_config config;

	if (game_connection() != _game_connection_network_client || !from_stream)
		return;
	csmemcpy(&config, entries, sizeof(config));
	chat_config.known = TRUE;
	chat_config.host_named = TEST_FLAG(config.flags, _chat_config_host_machine_bit) != 0;
	chat_config.host_machine = chat_config.host_named ? (short)config.pad[0] : 0;
	chat_config.flags = (byte)(config.flags & (byte)~(byte)FLAG(_chat_config_host_machine_bit));
}

word network_chat_config_entry_size(
	void)
{
	return sizeof(struct distributed_chat_config);
}

/* what is wrong with a line the host's settings refuse: the chat off, or a
link where links are not allowed; NULL for none */
static char const *chat_refused(
	byte flags,
	char const *text)
{
	if (!TEST_FLAG(flags, _chat_config_allowed_bit))
		return "Text chat is off in this game";
	if (!TEST_FLAG(flags, _chat_config_links_bit) && chat_text_has_link(text))
		return "Links aren't allowed in this game";
	return NULL;
}

/* ---------- the server's word filter (server.chat_filter)

A dedicated server may keep words out of its chat and its players' names:
the words of streamer mode's list (text_filter_match.c), by group (slurs,
swearing, insults) and the least severity each keeps out, its own settings'
(server.chat_filter_slurs...). "censor" stars a line's words before the
line goes on; "block" refuses the line (its sender told why) and a machine
whose player's name has them at the door
(network_game_server_accept_client_machine_into_game). Off by default. */

enum
{
	_chat_filter_off,
	_chat_filter_censor,
	_chat_filter_block
};

/* server.chat_filter's mode (in either case; anything else: off) */
static short chat_filter_mode(
	char const *setting)
{
	if (setting && chat_names_equal(setting, "censor"))
		return _chat_filter_censor;
	if (setting && chat_names_equal(setting, "block"))
		return _chat_filter_block;
	return _chat_filter_off;
}

/* a group's level: the least severity kept out ("mild" keeps out every
term of the group; anything else: off) */
static unsigned char chat_filter_level(
	char const *setting)
{
	if (setting && chat_names_equal(setting, "mild"))
		return _text_filter_mild;
	if (setting && chat_names_equal(setting, "strong"))
		return _text_filter_strong;
	if (setting && chat_names_equal(setting, "severe"))
		return _text_filter_severe;
	return _text_filter_off;
}

/* a line's marked characters as stars, in place */
static void chat_filter_star(
	char *text,
	unsigned char const *marked,
	long length)
{
	long index;

	for (index = 0; index < length && text[index]; index++)
	{
		if (marked[index] && text[index] != ' ')
			text[index] = '*';
	}
}

/* (the host) the filter's mode, off but on a dedicated server */
static short chat_filter_mode_now(
	void)
{
	return halo_server_mode() ? chat_filter_mode(config_string("server.chat_filter")) : _chat_filter_off;
}

/* which characters of an ASCII text are the filter's words, marked in
marked (as long as the text); how many */
static long chat_filter_marks(
	char const *text,
	unsigned char *marked)
{
	unsigned short units[CHAT_TEXT_SIZE];
	unsigned char levels[NUMBER_OF_TEXT_FILTER_GROUPS];
	long length;

	for (length = 0; length < CHAT_TEXT_SIZE && text[length]; length++)
		units[length] = (unsigned char)text[length];
	if (!length)
		return 0;
	levels[_text_filter_slurs] = chat_filter_level(config_string("server.chat_filter_slurs"));
	levels[_text_filter_swearing] = chat_filter_level(config_string("server.chat_filter_swearing"));
	levels[_text_filter_insults] = chat_filter_level(config_string("server.chat_filter_insults"));
	levels[_text_filter_own] = _text_filter_off;
	return text_filter_censor(units, length, TRUE, levels, marked);
}

boolean network_chat_name_blocked(
	wchar_t const *name,
	short length)
{
	char text[CHAT_TEXT_SIZE];
	unsigned char marked[CHAT_TEXT_SIZE];
	short index;

	if (chat_filter_mode_now() != _chat_filter_block)
		return FALSE;
	for (index = 0; index < length && index < CHAT_TEXT_SIZE - 1 && name[index]; index++)
		text[index] = name[index] >= ' ' && name[index] <= '~' ? (char)name[index] : ' ';
	text[index] = 0;
	return chat_filter_marks(text, marked) > 0;
}

/* the characters of a player's name a line keeps (at most CHAT_NAME_SIZE - 1) */
static short chat_player_name_length(
	struct player_datum const *player)
{
	short length;

	for (length = 0; length < CHAT_NAME_SIZE - 1 && length < (short)NUMBEROF(player->name) && player->name[length];
		length++)
		;
	return length;
}

/* a player's name in ASCII ("?" for what is not), as debug.txt and /mute
have it */
static void chat_player_name_ascii(
	struct player_datum const *player,
	char *name)
{
	short length = chat_player_name_length(player);
	short index;

	for (index = 0; index < length; index++)
		name[index] = player_name_character_ascii(player->name[index]);
	name[length] = 0;
}

/* the newest line, cleared (the oldest gives way) */
static struct chat_line *chat_new_line(
	char const *text)
{
	struct chat_line *line;

	chat_newest_line = (short)((chat_newest_line + 1) % CHAT_MAXIMUM_LINES);
	if (chat_line_count < CHAT_MAXIMUM_LINES)
		chat_line_count++;
	line = &chat_lines[chat_newest_line];
	csmemset(line, 0, sizeof(*line));
	csstrncpy(line->text, text, CHAT_TEXT_SIZE - 1);
	line->time = system_milliseconds();
	return line;
}

/* the local players, a bit for each */
static short chat_local_players(
	void)
{
	short players = 0;
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE;
		local_player_index = local_player_get_next(local_player_index))
	{
		if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS)
			players |= (short)FLAG(local_player_index);
	}
	return players;
}

static short chat_local_hearers(struct player_datum const *sender, short player_index, short channel);

/* the machine running the game: this one, when it is the host; the machine
its settings named; or the first machine, which a host that does not say
still is */
static short chat_host_machine(
	void)
{
	if (chat_host())
		return network_game_client_get_local_machine_index();
	if (chat_config.host_named)
		return chat_config.host_machine;
	return 0;
}

/* a person on the machine running the game. a client says so from that
machine, including when the host did not mark the line. a bot's machine is
not this one */
static boolean chat_player_is_host(
	struct player_datum const *player)
{
	short host_machine;
	short machine;

	if (!player)
		return FALSE;
	host_machine = chat_host_machine();
	machine = (short)player->network_player_data.machine_index;
	if (host_machine == NONE || machine != host_machine)
		return FALSE;
	return !bots_machine_is_bot(machine);
}

/* a line shown, from a player (absolute index). host_name: their name is white */
static void chat_add_line(
	short player_index,
	short channel,
	char const *text,
	boolean host_name)
{
	struct player_datum *player = distributed_player(player_index);
	struct chat_line *line;
	char name[CHAT_NAME_SIZE];

	/* (nor one of a machine this player muted on the scoreboard: its voice
	and its lines, here alone) */
	if (!player || network_voice_machine_muted(player->network_player_data.machine_index))
		return;
	if (!host_name)
		host_name = chat_player_is_host(player);
	line = chat_new_line(text);
	csmemcpy(line->name, player->name, chat_player_name_length(player) * sizeof(line->name[0]));
	line->color = host_name ? _chat_color_host :
		game_engine_has_teams() && player->team_index == 0 ? _chat_color_red : _chat_color_blue;
	line->channel = channel;
	line->hearers = chat_local_hearers(player, player_index, channel);
	/* (debug.txt has the lines, in ASCII) */
	chat_player_name_ascii(player, name);
	error(_error_log, "chat%s: %s: %s", channel == _chat_channel_team ? " (team)" :
		channel == _chat_channel_vehicle ? " (vehicle)" : "", name, line->text);
}

/* a notice shown (the host's): its text alone */
static void chat_add_notice(
	char const *text)
{
	struct chat_line *line = chat_new_line(text);

	line->color = _chat_color_notice;
	line->channel = _chat_channel_everyone;
	line->hearers = chat_local_players();
	error(_error_log, "chat notice: %s", line->text);
}

/* ---------- the host */

/* the vehicle a player rides in, the outermost (a vehicle carried by another
is in that one); NONE for none, or no unit */
static long chat_player_vehicle(
	struct player_datum const *player)
{
	long object_index = player->unit_index;
	long vehicle_index = NONE;

	/* (each object riding in its parent: a unit in a seat is its vehicle's child) */
	while (object_index != NONE)
	{
		struct object_datum *object = object_try_and_get(object_index);

		if (!object || object->object.parent_object_index == NONE)
			break;
		vehicle_index = object->object.parent_object_index;
		object_index = vehicle_index;
	}
	return vehicle_index;
}

/* the local players who hear a line from a player (absolute index) to a
channel, a bit for each: its sender, and those chat_hears says; every local
player when none does (the host found this machine hears it, whatever the
players' vehicles were here a moment before) */
static short chat_local_hearers(
	struct player_datum const *sender,
	short player_index,
	short channel)
{
	boolean has_teams = game_engine_has_teams();
	long sender_vehicle = chat_player_vehicle(sender);
	short hearers = 0;
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE;
		local_player_index = local_player_get_next(local_player_index))
	{
		long local_index = local_player_get_player_index(local_player_index);

		if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS && local_index != NONE &&
			((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(local_index) == player_index ||
				chat_hears(channel, has_teams, sender->team_index, player_get(local_index)->team_index,
					sender_vehicle, chat_player_vehicle(player_get(local_index)))))
		{
			hearers |= (short)FLAG(local_player_index);
		}
	}
	return hearers ? hearers : chat_local_players();
}

/* a line's message: from a player (absolute index; NONE for the host's
notice), its flags and its text */
static void chat_message_new(
	struct chat_message *message,
	short player_index,
	byte flags,
	char const *text)
{
	csmemset(message, 0, sizeof(*message));
	if (player_index != NONE)
		message->line.player_index = distributed_player_to_byte(player_index);
	message->line.flags = flags;
	csstrncpy(message->line.text, text, CHAT_TEXT_SIZE - 1);
}

/* (the host) a message of one entry sent to every client machine, reliably */
static void chat_send_to_clients(
	void *message,
	byte type,
	word size)
{
	long machine_indices[MAXIMUM_CHAT_MACHINES];
	short count = distributed_client_machines(machine_indices, MAXIMUM_CHAT_MACHINES);
	short index;

	for (index = 0; index < count; index++)
		distributed_send_to_machine_reliably(machine_indices[index], message, type, 1, size);
}

/* (the host) a player's line, sent on to every machine that hears it (and
the sender's) and shown here if a player of this machine's hears it or said
it */
static void chat_host_send(
	short player_index,
	short channel,
	char const *text)
{
	struct player_datum *sender = distributed_player(player_index);
	boolean has_teams = game_engine_has_teams();
	long sender_vehicle;
	long machine_indices[MAXIMUM_CHAT_MACHINES];
	short count = distributed_client_machines(machine_indices, MAXIMUM_CHAT_MACHINES);
	struct chat_message message;
	short index;
	short local_player_index;
	byte flags;
	boolean host_name;

	if (!sender || sender->quit_out_of_game)
		return;
	sender_vehicle = chat_player_vehicle(sender);
	host_name = chat_player_is_host(sender);
	flags = chat_channel_flags(channel);
	if (host_name)
		flags |= (byte)FLAG(_chat_line_host_bit);
	server_event_chat(player_index, channel, text);
	chat_message_new(&message, player_index, flags, text);
	for (index = 0; index < count; index++)
	{
		struct data_iterator iterator;
		struct player_datum *player;
		boolean hears = distributed_machine_has_player(machine_indices[index], player_index);

		data_iterator_new(&iterator, player_data);
		while (!hears && (player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
		{
			hears = !player->quit_out_of_game &&
				distributed_machine_has_player(machine_indices[index],
					(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index)) &&
				chat_hears(channel, has_teams, sender->team_index, player->team_index, sender_vehicle,
					chat_player_vehicle(player));
		}
		if (hears)
		{
			distributed_send_to_machine_reliably(machine_indices[index], &message, _distributed_message_chat_down, 1,
				(word)sizeof(message));
		}
	}
	for (local_player_index = local_player_get_next(NONE); local_player_index != NONE;
		local_player_index = local_player_get_next(local_player_index))
	{
		long local_index = local_player_get_player_index(local_player_index);

		if (local_index != NONE &&
			((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(local_index) == player_index ||
				chat_hears(channel, has_teams, sender->team_index, player_get(local_index)->team_index,
					sender_vehicle, chat_player_vehicle(player_get(local_index)))))
		{
			chat_add_line(player_index, channel, text, host_name);
			break;
		}
	}
}

/* (the host) a notice to a client machine; or (NONE) to every one, and
shown here */
static void chat_host_notice(
	long machine_index,
	char const *text)
{
	struct chat_message message;

	chat_message_new(&message, NONE, (byte)FLAG(_chat_line_notice_bit), text);
	if (machine_index != NONE)
	{
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_chat_down, 1,
			(word)sizeof(message));
		return;
	}
	chat_send_to_clients(&message, _distributed_message_chat_down, (word)sizeof(message));
	chat_add_notice(text);
}

/* (the host) a player's name muted for everyone, their lines and their
voice (network_voice.c), for minutes, every machine told; FALSE (said to the
host alone) when every mute is held */
static boolean chat_host_mute_name(
	char const *name,
	short minutes)
{
	char notice[2 * CHAT_TEXT_SIZE];

	if (!chat_mute_set(chat_mutes, NUMBEROF(chat_mutes), name, minutes, system_milliseconds()))
	{
		chat_add_notice("Too many players are muted");
		return FALSE;
	}
	snprintf(notice, sizeof(notice), "%s muted for %d minute%s", name, minutes, chat_minutes_plural(minutes));
	chat_host_notice(NONE, notice);
	return TRUE;
}

/* (the host) a player's name unmuted, every machine told; FALSE (said to
the host alone) when it was not muted */
static boolean chat_host_unmute_name(
	char const *name)
{
	struct chat_mute *mute = chat_mute_find(chat_mutes, NUMBEROF(chat_mutes), name, system_milliseconds());
	char notice[2 * CHAT_TEXT_SIZE];

	if (!mute)
	{
		snprintf(notice, sizeof(notice), "%s isn't muted", name);
		chat_add_notice(notice);
		return FALSE;
	}
	mute->active = FALSE;
	snprintf(notice, sizeof(notice), "%s unmuted", name);
	chat_host_notice(NONE, notice);
	return TRUE;
}

/* (the host) its player's /mute or /unmute: the player named muted for the
minutes, or let speak again, every machine told; what is wrong with it shown
to the host alone; FALSE for a line that is no command (said as it is) */
static boolean chat_host_command(
	short speaker_index,
	char const *text)
{
	char typed[CHAT_TEXT_SIZE];
	char names[MAXIMUM_TRACKED_PLAYERS][CHAT_NAME_SIZE];
	char notice[2 * CHAT_TEXT_SIZE];
	struct data_iterator iterator;
	struct player_datum *player;
	short minutes;
	short command = chat_command_parse(text, typed, sizeof(typed), &minutes);
	short found;

	if (command == _chat_command_none)
		return FALSE;
	if (command == _chat_command_usage)
	{
		chat_add_notice("Say /mute <player> [minutes] or /unmute <player>");
		return TRUE;
	}
	csmemset(names, 0, sizeof(names));
	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		short index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index);

		if (!player->quit_out_of_game && index >= 0 && index < MAXIMUM_TRACKED_PLAYERS)
			chat_player_name_ascii(player, names[index]);
	}
	found = chat_name_match(typed, (char const (*)[CHAT_NAME_SIZE])names, MAXIMUM_TRACKED_PLAYERS);
	if (found == NONE)
		snprintf(notice, sizeof(notice), "No player's name begins with \"%s\"", typed);
	else if (found == CHAT_NAME_AMBIGUOUS)
		snprintf(notice, sizeof(notice), "More than one player's name begins with \"%s\"", typed);
	else if (found == speaker_index)
		snprintf(notice, sizeof(notice), "You can't %s yourself", command == _chat_command_mute ? "mute" : "unmute");
	else if (command == _chat_command_mute)
	{
		chat_host_mute_name(names[found], minutes);
		return TRUE;
	}
	else
	{
		chat_host_unmute_name(names[found]);
		return TRUE;
	}
	chat_add_notice(notice);
	return TRUE;
}

short network_chat_host_mute_minutes(
	short player_index)
{
	struct player_datum *player = chat_host() ? distributed_player(player_index) : NULL;
	unsigned long now = system_milliseconds();
	struct chat_mute *mute;
	char name[CHAT_NAME_SIZE];

	if (!player)
		return 0;
	chat_player_name_ascii(player, name);
	mute = chat_mute_find(chat_mutes, NUMBEROF(chat_mutes), name, now);
	return mute ? chat_mute_minutes_left(mute, now) : 0;
}

void network_chat_host_mute(
	short player_index,
	boolean mute)
{
	struct player_datum *player = chat_host() ? distributed_player(player_index) : NULL;
	char name[CHAT_NAME_SIZE];

	if (!player)
		return;
	chat_player_name_ascii(player, name);
	if (mute)
		chat_host_mute_name(name, CHAT_MUTE_DEFAULT_MINUTES);
	else
		chat_host_unmute_name(name);
}

boolean network_chat_host_mute_named(
	char const *name,
	short minutes,
	boolean mute)
{
	if (!chat_host() || !name || !name[0])
		return FALSE;
	if (!mute)
		return chat_host_unmute_name(name);
	return chat_host_mute_name(name, minutes > 0 ? minutes : CHAT_MUTE_DEFAULT_MINUTES);
}

boolean network_chat_host_name_muted(
	wchar_t const *name)
{
	char ascii[CHAT_NAME_SIZE];
	short index;

	/* (the network game's names, the lobby's too: none muted but the host's) */
	for (index = 0; index < CHAT_NAME_SIZE - 1 && name[index]; index++)
		ascii[index] = player_name_character_ascii(name[index]);
	ascii[index] = 0;
	return index && chat_mute_find(chat_mutes, NUMBEROF(chat_mutes), ascii, system_milliseconds()) != NULL;
}

void network_chat_handle_up(
	long machine_index,
	void const *entries,
	boolean from_stream)
{
	struct distributed_chat_line line;
	char text[CHAT_TEXT_SIZE];
	char name[CHAT_NAME_SIZE];
	struct player_datum *sender;
	struct chat_mute *mute;
	char const *refused;
	unsigned long now = system_milliseconds();

	if (!chat_host() || machine_index < 0 || machine_index >= MAXIMUM_CHAT_MACHINES || !from_stream)
		return;
	csmemcpy(&line, entries, sizeof(line));
	/* (one of the machine's own players; then counted, and its text) */
	if (line.player_index >= MAXIMUM_TRACKED_PLAYERS ||
		!distributed_machine_has_player(machine_index, line.player_index) ||
		!chat_rate_take(&chat_rates[machine_index], now) ||
		!chat_text_clean(text, line.text, CHAT_TEXT_SIZE) ||
		(sender = distributed_player(line.player_index)) == NULL)
	{
		return;
	}
	/* (a muted player's line goes no further: they are told for how long) */
	chat_player_name_ascii(sender, name);
	/* (a dedicated server's player's "!switch" or "!team", the whole line,
	muted or not: said to no one, they are moved to the other team and
	everyone told, or told why not: server_team_switch.c) */
	if (halo_server_mode() && chat_team_switch_command(text))
	{
		char notice[CHAT_TEXT_SIZE];

		if (server_team_switch(DATUM_INDEX_NEW(line.player_index, sender->identifier), name, notice,
				(long)sizeof(notice)))
		{
			chat_host_notice(NONE, notice);
		}
		else
		{
			chat_host_notice(machine_index, notice);
		}
		return;
	}
	if ((mute = chat_mute_find(chat_mutes, NUMBEROF(chat_mutes), name, now)) != NULL)
	{
		short minutes = chat_mute_minutes_left(mute, now);
		char notice[CHAT_TEXT_SIZE];

		snprintf(notice, sizeof(notice), "You are muted for %d more minute%s", minutes, chat_minutes_plural(minutes));
		chat_host_notice(machine_index, notice);
		return;
	}
	/* (nor one the host's settings refuse, whatever the client knew of them:
	the sender is told why) */
	if ((refused = chat_refused(chat_host_config_flags(), text)) != NULL)
	{
		chat_host_notice(machine_index, refused);
		return;
	}
	/* (nor, on a dedicated server, one with words its filter keeps out:
	refused, or its words starred) */
	{
		short mode = chat_filter_mode_now();
		unsigned char marked[CHAT_TEXT_SIZE];

		if (mode != _chat_filter_off && chat_filter_marks(text, marked) > 0)
		{
			if (mode == _chat_filter_block)
			{
				platform_log("chat filter: a line of %s not sent (blocked words)", name);
				chat_host_notice(machine_index, "Your line was not sent: it has words this server does not allow");
				return;
			}
			chat_filter_star(text, marked, CHAT_TEXT_SIZE);
			platform_log("chat filter: a line of %s sent with its blocked words starred", name);
		}
	}
	chat_host_send(line.player_index, chat_channel_from_flags(line.flags), text);
}

word network_chat_line_entry_size(
	void)
{
	return sizeof(struct distributed_chat_line);
}

/* ---------- a client */

void network_chat_handle_down(
	void const *entries,
	boolean from_stream)
{
	struct distributed_chat_line line;
	char text[CHAT_TEXT_SIZE];

	if (game_connection() != _game_connection_network_client || !from_stream)
		return;
	csmemcpy(&line, entries, sizeof(line));
	if (!chat_text_clean(text, line.text, CHAT_TEXT_SIZE))
		return;
	if (TEST_FLAG(line.flags, _chat_line_notice_bit))
	{
		chat_add_notice(text);
		return;
	}
	if (distributed_player_from_byte(line.player_index) == NONE)
		return;
	chat_add_line(line.player_index, chat_channel_from_flags(line.flags), text,
		TEST_FLAG(line.flags, _chat_line_host_bit));
}

/* ---------- typing */

/* The editor's owner, or the first local player for direct commands; NONE. */
static short chat_speaker(
	boolean typed)
{
	short local_player_index = typed && chat_input.active ? controller_remap_local_player(chat_input.controller) : local_player_get_next(NONE);
	long player_index = local_player_index != NONE ? local_player_get_player_index(local_player_index) : NONE;
	/* A departing player's unfinished line must never become somebody else's. */
	if (typed && chat_input.active && player_index != chat_input.player) return NONE;

	return player_index != NONE ? (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) : NONE;
}

/* a line said; checked: refused here as the host would refuse it (a test's
client sends one unchecked, as a changed build could, for the host to refuse) */
static void chat_send(
	short channel,
	char const *typed,
	boolean checked,
	boolean from_editor)
{
	char text[CHAT_TEXT_SIZE];
	char name[CHAT_TEXT_SIZE];
	short minutes;
	short player_index = chat_speaker(from_editor);
	char const *refused;

	if (player_index == NONE || !chat_text_clean(text, typed, CHAT_TEXT_SIZE))
		return;
	/* (the votes' words, not said: "!endgame" starts a vote to end the game,
	"!yes" and "!no" vote in the one running, as F1 and F2 do; the host's
	notices tell everyone: network_votekick.c) */
	if (chat_command_word(text, "!endgame"))
	{
		network_vote_end_game();
		return;
	}
	if (chat_command_word(text, "!yes") || chat_command_word(text, "!no"))
	{
		if (!network_vote_choose(chat_command_word(text, "!yes") != NULL))
			chat_add_notice("There is no vote you can vote in");
		return;
	}
	/* (the host's commands, whatever its settings; a client's is not sent) */
	if (chat_host() && chat_host_command(player_index, text))
		return;
	if (!chat_host() && chat_command_parse(text, name, sizeof(name), &minutes) != _chat_command_none)
	{
		chat_add_notice("Only the host can mute players");
		return;
	}
	/* (a line the host's settings refuse, refused here: the host would) */
	if ((checked || chat_host()) && (refused = chat_refused(chat_config_flags(), text)) != NULL)
	{
		chat_add_notice(refused);
		return;
	}
	if (chat_host())
	{
		chat_host_send(player_index, channel, text);
	}
	else
	{
		struct chat_message message;

		chat_message_new(&message, player_index, chat_channel_flags(channel), text);
		distributed_send(&message, _distributed_message_chat_up, 1, (word)sizeof(message),
			_distributed_to_host_reliably);
	}
}

void network_chat_host_notice(
	char const *text)
{
	long length = (long)csstrlen(text);
	long start = 0;

	/* (with the chat not shown, the console's red line, whatever
	game.console_log says: the host's notices are for the players) */
	if (!chat_network_game() || !chat_shown())
	{
		terminal_printf(global_real_argb_red, "%s", text);
		return;
	}
	/* (a line of the chat's at most: a longer notice in several, broken at
	a space where there is one) */
	while (start < length)
	{
		char part[CHAT_TEXT_SIZE];
		long end = start + CHAT_TEXT_SIZE - 1;

		if (end >= length)
			end = length;
		else
		{
			long space;

			for (space = end; space > start && text[space] != ' '; space--)
				;
			if (space > start)
				end = space;
		}
		csmemcpy(part, text + start, end - start);
		part[end - start] = 0;
		chat_add_notice(part);
		for (start = end; start < length && text[start] == ' '; start++)
			;
	}
}

void network_chat_say(
	short channel,
	char const *text)
{
	if (chat_network_game() && game_in_progress() && channel >= 0 && channel < NUMBER_OF_CHAT_CHANNELS)
		chat_send(channel, text, TRUE, FALSE);
}

void network_chat_say_unchecked(
	short channel,
	char const *text)
{
	if (chat_network_game() && game_in_progress() && channel >= 0 && channel < NUMBER_OF_CHAT_CHANNELS)
		chat_send(channel, text, FALSE, FALSE);
}

static void chat_input_begin(
	short channel, short controller, boolean physical)
{
	struct key_stroke key;
	short local = controller < 0 ? local_player_get_next(NONE) : controller_remap_local_player(controller);
	long player = local != NONE ? local_player_get_player_index(local) : NONE;
	if (player == NONE) return;

	csmemset(&chat_input, 0, sizeof(chat_input));
	chat_input.active = TRUE;
	chat_input.channel = channel;
	chat_scroll = 0;
	chat_input.controller = player_get(player)->local_player_index;
	chat_input.player = player;
	chat_input.physical = physical;
	/* (the keys pressed before) */
	while (input_get_key(&key))
		;
}

static void chat_input_end(
	void)
{
	chat_input.active = FALSE;
	chat_keyboard.open = FALSE;
	chat_keyboard.opening_buttons = 0;
	chat_keyboard.drawn = FALSE;
	chat_scroll = 0;
}

/* the line's last character gone */
static void chat_input_delete(
	void)
{
	short length = (short)csstrlen(chat_input.text);

	if (length)
		chat_input.text[length - 1] = 0;
}

/* the on-screen keyboard's key picked pressed: its character typed; or
capitals, the page, a space, a delete, or the line sent */
static void chat_keyboard_press(
	void)
{
	if (chat_keyboard.row < CHAT_KEYBOARD_ROWS - 1)
	{
		char typed[2];

		typed[0] = chat_keyboard_character(chat_keyboard.symbols ? chat_keyboard_symbols : chat_keyboard_letters,
			chat_keyboard.row, chat_keyboard.column, chat_keyboard.shift);
		typed[1] = 0;
		chat_text_append(chat_input.text, typed);
		return;
	}
	switch (chat_keyboard.column / CHAT_KEYBOARD_WIDE_COLUMNS)
	{
	case _chat_key_shift:
		chat_keyboard.shift = !chat_keyboard.shift;
		break;
	case _chat_key_symbols:
		chat_keyboard.symbols = !chat_keyboard.symbols;
		break;
	case _chat_key_space:
		chat_text_append(chat_input.text, " ");
		break;
	case _chat_key_delete:
		chat_input_delete();
		break;
	default:
		chat_send(chat_input.channel, chat_input.text, TRUE, TRUE);
		chat_input_end();
		break;
	}
}

/* the on-screen keyboard's controller (pressed: the buttons pressed this
frame, held: those held) and taps: B closes it, Start sends the line, D-pad
Up and Down turn it to everyone, the team or the vehicle, the stick (and
D-pad Left and Right) moves the key picked, A presses it, X deletes, Y
gives capitals; a tap presses the key under it */
static void chat_keyboard_update(
	unsigned long pressed,
	unsigned long held)
{
	unsigned long now = system_milliseconds();
	short rows_down = 0;
	short columns_right = 0;
	float x;
	float y;

	if (TEST_FLAG(pressed, HALO_CHAT_PAD_B))
	{
		chat_input_end();
		return;
	}
	if (TEST_FLAG(pressed, HALO_CHAT_PAD_START))
	{
		chat_send(chat_input.channel, chat_input.text, TRUE, TRUE);
		chat_input_end();
		return;
	}
	if (TEST_FLAG(pressed, HALO_CHAT_PAD_UP))
		chat_input.channel = (short)((chat_input.channel + NUMBER_OF_CHAT_CHANNELS - 1) % NUMBER_OF_CHAT_CHANNELS);
	if (TEST_FLAG(pressed, HALO_CHAT_PAD_DOWN))
		chat_input.channel = (short)((chat_input.channel + 1) % NUMBER_OF_CHAT_CHANNELS);
	/* (a direction pressed moves the key at once, held, again after a while
	and then quickly) */
	if (TEST_FLAG(held, HALO_CHAT_PAD_STICK_UP))
		rows_down = -1;
	else if (TEST_FLAG(held, HALO_CHAT_PAD_STICK_DOWN))
		rows_down = 1;
	if (TEST_FLAG(held, HALO_CHAT_PAD_STICK_LEFT) || TEST_FLAG(held, HALO_CHAT_PAD_LEFT))
		columns_right = -1;
	else if (TEST_FLAG(held, HALO_CHAT_PAD_STICK_RIGHT) || TEST_FLAG(held, HALO_CHAT_PAD_RIGHT))
		columns_right = 1;
	if (!rows_down && !columns_right)
	{
		chat_keyboard.repeat_at = 0;
	}
	else if (!chat_keyboard.repeat_at || (long)(now - chat_keyboard.repeat_at) >= 0)
	{
		chat_keyboard_move(&chat_keyboard.row, &chat_keyboard.column, rows_down, columns_right);
		chat_keyboard.repeat_at = now + (chat_keyboard.repeat_at ? CHAT_KEYBOARD_REPEAT_MILLISECONDS :
			CHAT_KEYBOARD_REPEAT_FIRST_MILLISECONDS);
	}
	if (TEST_FLAG(pressed, HALO_CHAT_PAD_A))
		chat_keyboard_press();
	if (chat_input.active && TEST_FLAG(pressed, HALO_CHAT_PAD_X))
		chat_input_delete();
	if (chat_input.active && TEST_FLAG(pressed, HALO_CHAT_PAD_Y))
		chat_keyboard.shift = !chat_keyboard.shift;
	/* (a tap on a key presses it; elsewhere it closes the keyboard, as the
	touch controls, B among them, hide while it is up: touch_input.c) */
	if (chat_input.active && chat_input.controller == 0 && halo_chat_taps(&x, &y) && chat_keyboard.drawn)
	{
		if (chat_keyboard_key_at((real)(x * 1000.0f - chat_keyboard.left) / (real)(chat_keyboard.right - chat_keyboard.left),
			(real)(y * 1000.0f - chat_keyboard.top) / (real)(chat_keyboard.bottom - chat_keyboard.top),
			&chat_keyboard.row, &chat_keyboard.column))
		{
			chat_keyboard_press();
		}
		else
			chat_input_end();
	}
}

unsigned char network_chat_keyboard_open(
	void)
{
	return chat_input.active && chat_keyboard.open;
}

/* whether the chat is out of sight: a menu up (the pause menu, however it
was opened), a cinematic, or the HUD hidden; no line is typed nor shown */
static boolean chat_hidden(
	void)
{
	return ui_widgets_active() || cinematic_in_progress() || (hud_scripted_globals && !hud_scripted_globals->show_hud);
}

/* (the host) the machines in the game: one gone has its lines' count let go,
so a machine new to its slot starts afresh */
static void chat_host_update_rates(
	void)
{
	long machine_indices[MAXIMUM_CHAT_MACHINES];
	short count = distributed_client_machines(machine_indices, MAXIMUM_CHAT_MACHINES);
	boolean present[MAXIMUM_CHAT_MACHINES];
	short index;

	csmemset(present, 0, sizeof(present));
	for (index = 0; index < count; index++)
	{
		if (machine_indices[index] >= 0 && machine_indices[index] < MAXIMUM_CHAT_MACHINES)
			present[machine_indices[index]] = TRUE;
	}
	for (index = 0; index < MAXIMUM_CHAT_MACHINES; index++)
	{
		if (!present[index])
			chat_rates[index].started = FALSE;
	}
}

/* The command's opening press is not navigation, even when bound to B,
 * a channel direction or a key-repeat direction. */
static unsigned long chat_keyboard_buttons(unsigned long buttons)
{
	chat_keyboard.opening_buttons &= buttons;
	return buttons & ~chat_keyboard.opening_buttons;
}

void network_chat_update(
	void)
{
	unsigned long held;
	unsigned long pressed;
	unsigned long pad = chat_keyboard_buttons(halo_chat_pad_buttons());
	unsigned long pad_pressed = pad & ~chat_keyboard.held;
	unsigned long now = system_milliseconds();
	short port, requested_port = NONE, requested_channel = _chat_channel_everyone;
	unsigned int chat_mask = (1u << PAD_CHAT) | (1u << PAD_TEAM_CHAT) | (1u << PAD_VEHICLE_CHAT);
	struct key_stroke key;

	/* Drain every port even while typing, so another player's press is not
	 * replayed after this line closes. Keyboard and touch keep their path. */
	for (port = 0; port < 4; port++)
	{
		unsigned int actions = controller_remap_pressed(port, chat_mask);
		if (actions && requested_port == NONE)
		{
			requested_port = port;
			requested_channel = actions & (1u << PAD_CHAT) ? _chat_channel_everyone :
				actions & (1u << PAD_TEAM_CHAT) ? _chat_channel_team : _chat_channel_vehicle;
		}
	}

	chat_keyboard.held = pad;
	/* (the host's settings: a client's as the host sends them in each game;
	and the last game's lines and counts let go as the next starts) */
	if (!chat_network_game() || !game_in_progress())
	{
		chat_config.known = FALSE;
		chat_config.host_named = FALSE;
		chat_config.host_machine = 0;
		chat_in_game = FALSE;
	}
	else
	{
		if (!chat_in_game)
		{
			chat_in_game = TRUE;
			chat_line_count = 0;
			chat_scroll = 0;
			csmemset(chat_motion_sensors, 0, sizeof(chat_motion_sensors));
			chat_off_noticed = FALSE;
			csmemset(chat_rates, 0, sizeof(chat_rates));
		}
		if (chat_host())
		{
			chat_host_update_config();
			chat_host_update_rates();
		}
	}
	/* (the votes' keys, F1 and F2, and the first controller's D-pad left and
	right while a vote it may vote in runs (network_vote_pad_open, which
	xinput_sdl.c keeps from the game meanwhile): yes or no, whether the chat
	is shown or not: network_votekick.c) */
	{
		static unsigned long vote_keys_held;
		static unsigned long vote_pad_held;
		unsigned long vote_keys = 0;
		unsigned long vote_pad = 0;

		if (chat_network_game() && game_in_progress() && !chat_hidden() && !chat_input.active && !console_is_active())
		{
			vote_keys = halo_keyboard_actions(0) & (FLAG(HALO_KEYBOARD_VOTE_YES) | FLAG(HALO_KEYBOARD_VOTE_NO));
			if (network_vote_pad_open())
				vote_pad = halo_chat_pad_buttons_for_port(0) & (FLAG(HALO_CHAT_PAD_LEFT) | FLAG(HALO_CHAT_PAD_RIGHT));
		}
		if (vote_keys & ~vote_keys_held)
			network_vote_choose(TEST_FLAG(vote_keys & ~vote_keys_held, HALO_KEYBOARD_VOTE_YES));
		else if (vote_pad & ~vote_pad_held)
			network_vote_choose(TEST_FLAG(vote_pad & ~vote_pad_held, HALO_CHAT_PAD_LEFT));
		vote_keys_held = vote_keys;
		vote_pad_held = vote_pad;
	}
	if (!chat_network_game() || !game_in_progress() || chat_speaker(TRUE) == NONE || !chat_shown() || chat_hidden() ||
		(chat_input.active && chat_input.physical && !controller_remap_connected(chat_input.controller)))
	{
		chat_input_end();
		chat_actions_held = 0;
		return;
	}
	if (!chat_input.active)
	{
		held = halo_keyboard_actions(0) &
			(FLAG(HALO_KEYBOARD_CHAT) | FLAG(HALO_KEYBOARD_TEAM_CHAT));
		pressed = held & ~chat_actions_held;
		chat_actions_held = held;
		/* (or the controller's D-pad Up, the touch controls' too, which opens
		the on-screen keyboard) */
		if ((pressed || requested_port != NONE || TEST_FLAG(pad_pressed, HALO_CHAT_PAD_UP)) && !console_is_active())
		{
			/* (no typing where the host has the chat off: it says so, once a
			while) */
			if (!TEST_FLAG(chat_config_flags(), _chat_config_allowed_bit))
			{
				if (!chat_off_noticed || now - chat_off_noticed_at >= CHAT_LINE_MILLISECONDS + CHAT_FADE_MILLISECONDS)
				{
					chat_off_noticed = TRUE;
					chat_off_noticed_at = now;
					chat_add_notice("Text chat is off in this game");
				}
			}
			else if (pressed)
				chat_input_begin(TEST_FLAG(pressed, HALO_KEYBOARD_CHAT) ? _chat_channel_everyone : _chat_channel_team, NONE, FALSE);
			else
			{
				chat_input_begin(requested_channel, requested_port, requested_port != NONE);
				if (!chat_input.active) return;
				chat_keyboard.opening_buttons = requested_port != NONE ?
					halo_chat_pad_buttons_for_port(chat_input.controller) : halo_chat_pad_buttons();
				chat_keyboard.held = 0;
				chat_keyboard.open = TRUE;
				error(_error_log, "chat: the on-screen keyboard opened");
				chat_keyboard.row = 1;
				chat_keyboard.column = 0;
				chat_keyboard.shift = FALSE;
				chat_keyboard.symbols = FALSE;
				chat_keyboard.repeat_at = 0;
			}
		}
		return;
	}
	chat_actions_held = 0;
	/* (the console, opened over it, takes the keys) */
	if (console_is_active())
		return;
	if (chat_keyboard.open)
		chat_keyboard_update(pad_pressed, pad);
	while (chat_input.active && input_get_key(&key))
	{
		switch (key.key_code)
		{
		case _key_escape:
			chat_input_end();
			break;
		case _key_return:
		case _keypad_enter:
			chat_send(chat_input.channel, chat_input.text, TRUE, TRUE);
			chat_input_end();
			break;
		/* (to everyone, the team, the vehicle, in turn) */
		case _key_tab:
			chat_input.channel = (short)((chat_input.channel + 1) % NUMBER_OF_CHAT_CHANNELS);
			break;
		case _key_backspace:
			chat_input_delete();
			break;
		/* (back through the lines kept, and forward again) */
		case _key_page_up:
			chat_scroll = (short)(chat_scroll + CHAT_PAGE_LINES);
			break;
		case _key_page_down:
			chat_scroll = (short)(chat_scroll - CHAT_PAGE_LINES);
			break;
		/* (it opens the console, over the line: console.c) */
		case _key_backquote:
			break;
		default:
			/* (Ctrl+V pastes; Ctrl with another key types nothing) */
			if (TEST_FLAG(key.modifier_flags, CHAT_KEY_MODIFIER_CONTROL_BIT))
			{
				char clipboard[CHAT_TEXT_SIZE];

				if (key.key_code == _key_v && platform_clipboard_get(clipboard, sizeof(clipboard)))
					chat_text_append(chat_input.text, clipboard);
			}
			else if (key.ascii_code >= ' ' && key.ascii_code <= '~')
			{
				char typed[2];

				typed[0] = key.ascii_code;
				typed[1] = 0;
				chat_text_append(chat_input.text, typed);
			}
			break;
		}
	}
	/* (the mouse wheel's notches, up older; then within the lines kept) */
	if (chat_input.active)
		chat_scroll = (short)(chat_scroll + halo_chat_wheel_steps());
	chat_scroll = (short)PIN(chat_scroll, 0, MAX(chat_line_count - 1, 0));
}

short network_chat_controller(void)
{
	return chat_input.active && chat_keyboard.open ? chat_input.controller : NONE;
}

boolean network_chat_typing(
	void)
{
	return chat_input.active;
}

/* ---------- drawing */

void network_chat_note_motion_sensor(
	short local_player_index,
	short left,
	short top)
{
	if (local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return;
	chat_motion_sensors[local_player_index].valid = TRUE;
	chat_motion_sensors[local_player_index].left = left;
	chat_motion_sensors[local_player_index].top = top;
	chat_motion_sensors[local_player_index].view_width =
		(short)(render.camera.window_bounds.x1 - render.camera.window_bounds.x0);
	chat_motion_sensors[local_player_index].view_height =
		(short)(render.camera.window_bounds.y1 - render.camera.window_bounds.y0);
	chat_motion_sensors[local_player_index].players = local_player_count();
}

/* where the lines go (and the on-screen keyboard): at the top left, the
newest highest, where the touch controls hold the bottom left (Android, and
a browser whose touch controls are used: xinput_sdl.c) */
static boolean chat_touch_layout(
	void)
{
	return halo_chat_touch_layout() != 0;
}

/* whether the lines are all white, as Custom Edition draws them
(display.chat_style "white"), not in their teams' colours ("colors", or
anything else) */
static boolean chat_style_white(
	void)
{
	static boolean white;
	static unsigned long read_at = (unsigned long)-1;

	/* (read again when Settings changes it) */
	if (read_at != config_changes())
	{
		const char *style = config_string("display.chat_style");

		read_at = config_changes();
		white = style && chat_names_equal(style, "white");
	}
	return white;
}

/* a name's colour: its team's (a notice's a pale gold), the host's white,
or every name white */
static void chat_color(
	short color,
	real alpha,
	real_argb_color *result)
{
	result->alpha = alpha;
	if (chat_style_white() || color == _chat_color_host)
	{
		result->red = result->green = result->blue = 1.0f;
	}
	else if (color == _chat_color_red)
	{
		result->red = 0.86f;
		result->green = 0.32f;
		result->blue = 0.28f;
	}
	else if (color == _chat_color_notice)
	{
		result->red = 0.93f;
		result->green = 0.82f;
		result->blue = 0.45f;
	}
	else
	{
		result->red = 0.34f;
		result->green = 0.51f;
		result->blue = 0.86f;
	}
}

/* the message's colour: a pale blue, or white */
static void chat_text_color(
	real alpha,
	real_argb_color *result)
{
	result->alpha = alpha;
	if (chat_style_white())
	{
		result->red = result->green = result->blue = 1.0f;
		return;
	}
	result->red = 0.64f;
	result->green = 0.75f;
	result->blue = 0.87f;
}

/* a string's soft shadow, down and right of it: copies in a translucent
black about its offset, which add up to a dark middle with a soft edge (the
text's own shadow, a hard copy a unit down and right, is not drawn) */
static void chat_draw_shadow(
	long font_index,
	rectangle2d const *bounds,
	wchar_t const *string,
	real alpha)
{
	static struct
	{
		short x;
		short y;
		real alpha;
	} const copies[] =
	{
		{ 1, 1, 0.45f },
		{ 2, 2, 0.35f },
		{ 2, 1, 0.25f },
		{ 1, 2, 0.25f },
		{ 3, 2, 0.12f },
		{ 2, 3, 0.12f },
		{ 3, 3, 0.10f },
	};
	short index;

	for (index = 0; index < (short)NUMBEROF(copies); index++)
	{
		real_argb_color color;
		rectangle2d shadow_bounds = *bounds;

		color.alpha = copies[index].alpha * alpha;
		color.red = color.green = color.blue = 0.0f;
		offset_rectangle2d(&shadow_bounds, copies[index].x, copies[index].y);
		draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, &color);
		rasterizer_draw_unicode_string(&shadow_bounds, NULL, NULL, 0, string);
	}
}

/* the width of a string's first count characters (NONE: all of it) as the
draw mode's font draws them, measured from bounds' left */
static short chat_string_width(
	rectangle2d const *bounds,
	wchar_t const *string,
	short count)
{
	wchar_t part[2 * CHAT_TEXT_SIZE];
	rectangle2d text_bounds;
	rectangle2d cursor_bounds;
	short index;

	for (index = 0; string[index] && (count == NONE || index < count) && index < (short)NUMBEROF(part) - 1; index++)
		part[index] = string[index];
	part[index] = 0;
	if (!index)
		return 0;
	draw_unicode_string_compute_bounds(bounds, part, &text_bounds, &cursor_bounds);
	return (short)(cursor_bounds.x0 - bounds->x0);
}

/* a line laid out in a column: its text's rows (each where it starts in the
text, and its characters), and the left of the text, after the name */
struct chat_layout
{
	short rows;
	short starts[CHAT_LINE_ROWS];
	short lengths[CHAT_LINE_ROWS];
	short text_left;
};

/* a line's text, from start, broken to fit width: the characters of its row
(at least one; at the last space that fits, where there is one) and the
spaces after them, which no row begins with */
static short chat_wrap_row(
	rectangle2d const *measure,
	wchar_t const *text,
	short start,
	short width,
	short *skip)
{
	short length;
	short low = 1;
	short high;
	short index;

	*skip = 0;
	for (length = 0; text[start + length]; length++)
		;
	if (chat_string_width(measure, text + start, NONE) <= width)
		return length;
	/* (the most characters that fit, at least one) */
	for (high = length; low < high;)
	{
		short middle = (short)((low + high + 1) / 2);

		if (chat_string_width(measure, text + start, middle) <= width)
			low = middle;
		else
			high = (short)(middle - 1);
	}
	for (index = low; index > 0 && text[start + index] != ' '; index--)
		;
	if (index > 0)
		low = index;
	while (text[start + low + *skip] == ' ')
		(*skip)++;
	return low;
}

/* a line (its name, "" for a notice, and its text) laid out from left to
right, in the draw mode's font, at most CHAT_LINE_ROWS rows */
static void chat_line_layout(
	short left,
	short right,
	short line_height,
	wchar_t const *name,
	wchar_t const *text,
	struct chat_layout *layout)
{
	rectangle2d measure;
	short start = 0;
	short width;

	measure.x0 = 0;
	measure.y0 = 0;
	measure.x1 = SHORT_MAX;
	measure.y1 = SHORT_MAX;
	layout->text_left = name[0] ? (short)(left + chat_string_width(&measure, name, NONE) + line_height / 4) : left;
	width = (short)(right - layout->text_left);
	layout->rows = 0;
	while (text[start] && layout->rows < CHAT_LINE_ROWS)
	{
		short skip = 0;
		short count = width > line_height ? chat_wrap_row(&measure, text, start, width, &skip) : (short)NONE;

		if (count == NONE)
			for (count = 0; text[start + count]; count++)
				;
		layout->starts[layout->rows] = start;
		layout->lengths[layout->rows] = count;
		layout->rows++;
		start = (short)(start + count + skip);
	}
	if (!layout->rows)
	{
		layout->starts[0] = 0;
		layout->lengths[0] = 0;
		layout->rows = 1;
	}
}

/* a row of a line: the name (row 0's, or none) in its colour, then that
row's text in its own, over their shadows; bounds' top and bottom the row's */
static void chat_draw_row(
	long font_index,
	rectangle2d const *bounds,
	wchar_t const *name,
	real_argb_color const *name_color,
	wchar_t const *text,
	short text_left,
	real_argb_color const *text_color,
	real alpha)
{
	rectangle2d text_bounds = *bounds;

	text_bounds.x0 = text_left;
	/* (the text's own shadow drawn clear: a colour of no alpha that is not
	0, which would be the text's black) */
	rasterizer_text_set_shadow_color(0x00000001);
	if (name && name[0])
		chat_draw_shadow(font_index, bounds, name, alpha);
	if (text[0])
		chat_draw_shadow(font_index, &text_bounds, text, alpha);
	if (name && name[0])
	{
		draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, name_color);
		rasterizer_draw_unicode_string(bounds, NULL, NULL, 0, name);
	}
	if (text[0])
	{
		draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, text_color);
		rasterizer_draw_unicode_string(&text_bounds, NULL, NULL, 0, text);
	}
	rasterizer_text_set_shadow_color(0);
}

/* a line laid out, drawn from top down, a row each line_height */
static void chat_draw_layout(
	long font_index,
	struct chat_layout const *layout,
	short left,
	short right,
	short top,
	short line_height,
	wchar_t const *name,
	real_argb_color const *name_color,
	wchar_t const *text,
	real_argb_color const *text_color,
	real alpha)
{
	short row;

	for (row = 0; row < layout->rows; row++)
	{
		wchar_t part[CHAT_TEXT_SIZE + 2];
		rectangle2d bounds;
		short index;

		for (index = 0; index < layout->lengths[row] && index < (short)NUMBEROF(part) - 1; index++)
			part[index] = text[layout->starts[row] + index];
		part[index] = 0;
		bounds.x0 = left;
		bounds.x1 = right;
		bounds.y0 = (short)(top + row * line_height);
		bounds.y1 = (short)(bounds.y0 + line_height);
		chat_draw_row(font_index, &bounds, row ? NULL : name, name_color, part, layout->text_left, text_color, alpha);
	}
}

/* a line's text (ASCII) as drawn, at most size with its end; its length */
static short chat_line_text(
	char const *text,
	wchar_t *drawn,
	short size)
{
	short index;

	for (index = 0; text[index] && index < size - 1; index++)
		drawn[index] = (wchar_t)(unsigned char)text[index];
	drawn[index] = 0;
	return index;
}

/* a string as streamer mode shows it (text_filter.c), in place, at most size
with its end: the whole line's, before it wraps, so that a word spelled out
over two rows is seen (the rows are then drawn as they are) */
static void chat_filter_shown(
	wchar_t *string,
	short size)
{
	wchar_t const *shown = text_filter_unicode(string);
	short index;

	if (shown == string)
		return;
	for (index = 0; shown[index] && index < size - 1; index++)
		string[index] = shown[index];
	string[index] = 0;
}

/* a line's name as drawn: "Name:", "Name (team):" or "Name (vehicle):" */
static void chat_line_name(
	struct chat_line const *line,
	wchar_t *name,
	short size)
{
	wchar_t const *suffix = line->channel == _chat_channel_team ? L" (team):" :
		line->channel == _chat_channel_vehicle ? L" (vehicle):" : L":";
	wchar_t const *player_name = line->name;
	short length = 0;
	short index;

	for (index = 0; index < CHAT_NAME_SIZE && player_name[index] && length < size - 1; index++)
		name[length++] = player_name[index];
	for (index = 0; suffix[index] && length < size - 1; index++)
		name[length++] = suffix[index];
	name[length] = 0;
}

/* the on-screen keyboard, at the window's bottom right (with the touch
controls in the middle, between them: chat_touch_layout), the key picked
lit; where its keys are kept, for taps */
static void chat_keyboard_draw(
	long font_index,
	rectangle2d const *window,
	short line_height)
{
	short percent = chat_touch_layout() ? CHAT_KEYBOARD_WIDTH_PERCENT_ANDROID : CHAT_KEYBOARD_WIDTH_PERCENT;
	short window_width = (short)(window->x1 - window->x0);
	short window_height = (short)(window->y1 - window->y0);
	short key_width = (short)(window_width * percent / 100 / CHAT_KEYBOARD_COLUMNS);
	short key_height = (short)MAX(key_width * 3 / 4, line_height + 4);
	rectangle2d keys;
	rectangle2d panel;
	short row;

	if (window_width <= 0 || window_height <= 0 || key_width <= 4)
		return;
	if (chat_touch_layout())
		keys.x0 = (short)((window->x0 + window->x1 - key_width * CHAT_KEYBOARD_COLUMNS) / 2);
	else
		keys.x0 = (short)(window->x1 - window_width / 25 - key_width * CHAT_KEYBOARD_COLUMNS);
	keys.x1 = (short)(keys.x0 + key_width * CHAT_KEYBOARD_COLUMNS);
	keys.y1 = (short)(window->y1 - line_height);
	keys.y0 = (short)(keys.y1 - key_height * CHAT_KEYBOARD_ROWS);
	panel = keys;
	panel.x0 -= 4;
	panel.y0 -= 4;
	panel.x1 += 4;
	panel.y1 += 4;
	draw_quad(&panel, 0xA0000000);
	for (row = 0; row < CHAT_KEYBOARD_ROWS; row++)
	{
		short span = row == CHAT_KEYBOARD_ROWS - 1 ? CHAT_KEYBOARD_WIDE_COLUMNS : 1;
		short column;

		for (column = 0; column < CHAT_KEYBOARD_COLUMNS; column += span)
		{
			boolean picked = row == chat_keyboard.row && column == chat_keyboard.column;
			boolean lit = row == CHAT_KEYBOARD_ROWS - 1 && column / span == _chat_key_shift && chat_keyboard.shift;
			real_argb_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
			rectangle2d key;
			rectangle2d bounds;
			wchar_t label[8] = { 0 };

			key.x0 = (short)(keys.x0 + column * key_width + 2);
			key.x1 = (short)(keys.x0 + (column + span) * key_width - 2);
			key.y0 = (short)(keys.y0 + row * key_height + 2);
			key.y1 = (short)(keys.y0 + (row + 1) * key_height - 2);
			draw_quad(&key, picked ? 0xF05682DC : lit ? 0xC0406090 : 0x90283848);
			if (row < CHAT_KEYBOARD_ROWS - 1)
			{
				label[0] = (wchar_t)(unsigned char)chat_keyboard_character(chat_keyboard.symbols ? chat_keyboard_symbols :
					chat_keyboard_letters, row, column, chat_keyboard.shift);
			}
			else
			{
				static wchar_t const *const wide[NUMBER_OF_CHAT_WIDE_KEYS] =
					{ L"Aa", L"#+=", L"space", L"DEL", L"SEND" };
				wchar_t const *text = column / span == _chat_key_symbols && chat_keyboard.symbols ? L"ABC" :
					wide[column / span];
				short index;

				for (index = 0; text[index] && index < (short)NUMBEROF(label) - 1; index++)
					label[index] = text[index];
			}
			bounds = key;
			bounds.y0 = (short)(key.y0 + (key.y1 - key.y0 - line_height) / 2);
			bounds.y1 = (short)(bounds.y0 + line_height);
			draw_string_set_draw_mode(font_index, NONE, 2 /* centred */, 0, &color);
			rasterizer_draw_unicode_string(&bounds, NULL, NULL, 0, label);
		}
	}
	/* (in thousandths of the viewport, which fills the screen as a tap's
	place is of it: the window inside it can be smaller, as on Android; the
	keys are drawn from the viewport's top left) */
	{
		short viewport_width = (short)(render.camera.viewport_bounds.x1 - render.camera.viewport_bounds.x0);
		short viewport_height = (short)(render.camera.viewport_bounds.y1 - render.camera.viewport_bounds.y0);

		if (viewport_width <= 0 || viewport_height <= 0)
			return;
		chat_keyboard.drawn = TRUE;
		chat_keyboard.left = (short)(keys.x0 * 1000 / viewport_width);
		chat_keyboard.right = (short)(keys.x1 * 1000 / viewport_width);
		chat_keyboard.top = (short)(keys.y0 * 1000 / viewport_height);
		chat_keyboard.bottom = (short)(keys.y1 * 1000 / viewport_height);
	}
}

void network_chat_draw(
	short local_player_index)
{
	long font_index = hud_get_font_index();
	unsigned long now = system_milliseconds();
	boolean speaker = chat_input.active ? local_player_index == controller_remap_local_player(chat_input.controller) :
		local_player_index == local_player_get_next(NONE);
	boolean typing = speaker && chat_input.active && !console_is_active();
	boolean touch = chat_touch_layout();
	rectangle2d window = render.camera.window_bounds;
	real_argb_color text_color;
	struct font_header *font;
	short line_height;
	short window_width;
	short left;
	short right;
	/* the edge the next line is drawn from (down from it with the touch
	controls, up from it without), and the rows left to draw */
	short edge;
	short rows;
	short index;
	short skipped = 0;
	real scale;
	/* the point the lines are scaled about */
	short anchor;

	if (!chat_network_game() || !chat_shown() || chat_hidden() || font_index == NONE || local_player_index < 0 ||
		local_player_index >= MAXIMUM_LOCAL_PLAYERS || (!chat_line_count && !typing))
	{
		return;
	}
	offset_rectangle2d(&window, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	window_width = (short)(window.x1 - window.x0);
	font = font_definition_get(font_index);
	line_height = font->ascending_height + font->descending_height + font->leading_height;
	if (line_height <= 0 || window_width <= 0)
		return;
	scale = touch ? 1.0f : CHAT_TEXT_SCALE;
	/* (above the motion sensor, a little in from its left, the newest
	lowest: where it was last drawn in a view of this size, drawn now or not
	(a dead player's is not); where it has not been, about where it would be) */
	if (chat_motion_sensors[local_player_index].valid &&
		chat_motion_sensors[local_player_index].view_width == window_width &&
		chat_motion_sensors[local_player_index].view_height == (short)(window.y1 - window.y0) &&
		chat_motion_sensors[local_player_index].players == local_player_count())
	{
		left = (short)(chat_motion_sensors[local_player_index].left + line_height * scale / 2);
		edge = (short)(chat_motion_sensors[local_player_index].top - line_height * scale / 2);
	}
	else
	{
		real hud_scale = hud_globals_get_scale(local_player_count() > 1);

		left = (short)(window.x0 + CHAT_FALLBACK_LEFT_UNITS * hud_scale);
		edge = (short)(window.y1 - CHAT_FALLBACK_BOTTOM_UNITS * hud_scale);
	}
	right = (short)MAX(window.x0 + window_width * CHAT_COLUMN_PERCENT / 100, left + 8 * line_height);
	rows = local_player_count() > 1 ? CHAT_LINES_SHOWN_SPLIT : CHAT_LINES_SHOWN;
	if (touch)
	{
		/* (the touch controls' stick and buttons are at the bottom left: the
		lines go at the top left, under the weapon's panel, in the same column,
		the newest highest, as wide as the view) */
		edge = (short)(window.y0 + CHAT_TOP_UNITS * hud_globals_get_scale(local_player_count() > 1));
		right = (short)(window.x1 - line_height);
		rows = CHAT_LINES_SHOWN_ANDROID;
	}
	right = (short)MIN(right, window.x1);
	/* (laid out at the font's size, the column as much wider; drawn scaled
	about its corner where the lines start, the bottom left, or the top left
	with the touch controls) */
	right = (short)(left + (right - left) / scale);
	anchor = edge;
	rasterizer_text_set_scale(scale, (real)left, (real)anchor);
	if (typing)
	{
		real_argb_color color;
		struct chat_layout layout;
		wchar_t text[CHAT_TEXT_SIZE + 2];
		wchar_t const *prompt = chat_input.channel == _chat_channel_team ? L"Say to team:" :
			chat_input.channel == _chat_channel_vehicle ? L"Say to vehicle:" : L"Say:";
		short length;
		short start = 0;

		length = chat_line_text(chat_input.text, text, CHAT_TEXT_SIZE);
		text[length] = (now / CHAT_CARET_MILLISECONDS) % 2 == 0 ? '_' : ' ';
		text[length + 1] = 0;
		chat_color(_chat_color_blue, 1.0f, &color);
		chat_text_color(1.0f, &text_color);
		draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, &color);
		/* (what the player types is measured and drawn as it is: text_filter.c) */
		text_filter_suspend();
		/* (one row: its end, with the caret, where the whole is wider) */
		chat_line_layout(left, SHORT_MAX, line_height, prompt, text, &layout);
		{
			rectangle2d measure;
			short width = (short)(right - layout.text_left);
			short high = length;

			measure.x0 = 0;
			measure.y0 = 0;
			measure.x1 = SHORT_MAX;
			measure.y1 = SHORT_MAX;
			while (start < high)
			{
				short middle = (short)((start + high) / 2);

				if (chat_string_width(&measure, text + middle, NONE) <= width)
					high = middle;
				else
					start = (short)(middle + 1);
			}
		}
		layout.rows = 1;
		layout.starts[0] = start;
		layout.lengths[0] = (short)(length + 1 - start);
		chat_draw_layout(font_index, &layout, left, right, touch ? edge : (short)(edge - line_height), line_height,
			prompt, &color, text, &text_color, 1.0f);
		text_filter_resume();
		edge = (short)(edge + (touch ? line_height : -line_height));
		rows--;
		if (chat_keyboard.open)
		{
			/* (the on-screen keyboard at its own size) */
			rasterizer_text_set_scale(1.0f, 0.0f, 0.0f);
			chat_keyboard_draw(font_index, &window, line_height);
			rasterizer_text_set_scale(scale, (real)left, (real)anchor);
		}
		/* (scrolled back: a row saying newer lines are below) */
		if (chat_scroll > 0 && rows > 0)
		{
			wchar_t const *hint = L"(Page Down: newer lines)";
			wchar_t empty[1] = { 0 };

			chat_text_color(0.6f, &text_color);
			draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, &text_color);
			chat_line_layout(left, right, line_height, empty, hint, &layout);
			layout.rows = 1;
			chat_draw_layout(font_index, &layout, left, right, touch ? edge : (short)(edge - line_height), line_height,
				empty, &text_color, hint, &text_color, 0.6f);
			edge = (short)(edge + (touch ? line_height : -line_height));
			rows--;
		}
	}
	for (index = 0; index < chat_line_count && rows > 0; index++)
	{
		struct chat_line const *line =
			&chat_lines[(chat_newest_line - index + CHAT_MAXIMUM_LINES) % CHAT_MAXIMUM_LINES];
		real alpha = chat_line_alpha(now - line->time, typing);
		/* (a name and its " (vehicle):" fit as a notice's text does) */
		wchar_t name[CHAT_TEXT_SIZE];
		wchar_t text[CHAT_TEXT_SIZE];
		real_argb_color color;
		struct chat_layout layout;

		/* (the older ones have faded too) */
		if (alpha <= 0.0f)
			break;
		/* (this view's player hears it) */
		if (!TEST_FLAG(line->hearers, local_player_index))
			continue;
		/* (scrolled past while typing: Page Up) */
		if (typing && skipped < chat_scroll)
		{
			skipped++;
			continue;
		}
		/* (a notice's text alone, in its colour) */
		chat_color(line->color, alpha, &color);
		chat_line_text(line->text, text, NUMBEROF(text));
		if (line->color == _chat_color_notice)
		{
			name[0] = 0;
			text_color = color;
		}
		else
		{
			chat_line_name(line, name, NUMBEROF(name));
			chat_text_color(alpha, &text_color);
		}
		/* (streamer mode's words hidden in the whole line, which is then
		measured and drawn as it is) */
		chat_filter_shown(name, NUMBEROF(name));
		chat_filter_shown(text, NUMBEROF(text));
		text_filter_suspend();
		draw_string_set_draw_mode(font_index, NONE, 0 /* left */, 0, &color);
		chat_line_layout(left, right, line_height, name, text, &layout);
		/* (the newest rows first: an older line that does not fit is not shown) */
		if (layout.rows > rows)
		{
			text_filter_resume();
			break;
		}
		chat_draw_layout(font_index, &layout, left, right, touch ? edge : (short)(edge - layout.rows * line_height),
			line_height, name, &color, text, &text_color, alpha);
		text_filter_resume();
		edge = (short)(edge + (touch ? 1 : -1) * layout.rows * line_height);
		rows = (short)(rows - layout.rows);
	}
	rasterizer_text_set_scale(1.0f, 0.0f, 0.0f);
}

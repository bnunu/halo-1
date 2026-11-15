/*
SERVER_COMMANDS.C

A dedicated server's commands (server_director.h; docs/dedicated_server.md,
"Commands"): the lines the server's owner, its web page and its remote
console give, whichever they came from, run on the game's thread as the
console's are (hs.c hands each line here first: server_command_try). Each
command has a role, checked where the line is run (the HTTP API checks it
too, before it asks). Halo PC's dedicated server's names are kept where they
mean the same (sv_name, sv_maxplayers, sv_map, sv_mapcycle_add...). A
password's command is never logged nor answered with the password.
*/

#include "cseries.h"
#include "main/console.h"
#include "main/main.h"
#include "hs/hs.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "networking/network_server_manager.h"
#include "networking/network_server_manager_internal.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "network_chat.h"
#include "network_distributed.h"
#include "network_link.h"
#include "server_director.h"
#include "server_gametypes.h"

#include "../src/p2p.h" /* port: port/linux/src/p2p.c */
#include "../src/server_console.h" /* port: port/linux/src/server_console.c */
#include "../src/server_events.h" /* port: port/linux/src/server_events.c */
#include "../src/server_json.h" /* port: port/linux/src/server_json.c */
#include "../src/server_log.h" /* port: port/linux/src/server_log.c */
#include "../src/server_master.h" /* port: port/linux/src/server_master.c */
#include "../src/server_netlist.h" /* port: port/linux/src/server_netlist.c */
#include "../src/posix_server.h" /* port: port/linux/src/posix_server.c */
#include "../src/port_config.h" /* port: port/linux/src/port_config.c */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void platform_log(char const *format, ...);
void platform_translate_path(char const *xbox_path, char *host_path, unsigned long host_path_size);

#if HALO_SERVER_BUILD

enum
{
	MAXIMUM_WORDS = 8,
	/* a moderator's ban's longest (an admin's has none) */
	MODERATOR_BAN_SECONDS = 24 * 60 * 60,
	/* bans.txt's lines the ban commands read */
	MAXIMUM_BAN_LINES = 2048,
	BAN_LINE_SIZE = 512,
	/* say's lines a minute from the web (the owner's are not counted) */
	SAY_PER_MINUTE = 20,
	MAXIMUM_SLOTS = HALO_PORT_MAXIMUM_NETWORK_PLAYERS,
	NOTICE_SIZE = 128,
};

#define BANS_FILE "d:\\bans.txt"

static struct
{
	int role;
	int origin;
	/* say's: the minute counted, and the lines said in it */
	unsigned long say_minute;
	long say_count;
	/* a line's output captured whole (server_commands_run), not through
	the console's lines */
	boolean capturing;
} server_commands = { _server_role_owner };

int server_commands_role(
	void)
{
	return server_commands.role;
}

/* ---------- output */

/* a line of the command's reply: whole into the capture its line is run
with (server_console_output), whatever its length (sv_status --json's);
else the console's, whose lines hold 254 characters, a piece at a time */
static void say(
	char const *format,
	...)
{
	static char text[SERVER_COMMAND_OUTPUT_SIZE];
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(text, sizeof(text), format, arguments);
	va_end(arguments);
	if (server_commands.capturing)
	{
		/* (an empty line is no line to the capture) */
		server_console_output(text[0] ? text : " ");
	}
	else
	{
		long length = (long)strlen(text);
		long at = 0;

		do
		{
			console_printf(FALSE, "%.*s", (int)MIN(length - at, 200), text + at);
			at += 200;
		}
		while (at < length);
	}
}

/* ---------- words */

/* the words of a line: separated by spaces, a quoted one whole; how many */
static long split_words(
	char *line,
	char **words,
	long maximum)
{
	long count = 0;
	char *cursor = line;

	while (*cursor && count < maximum)
	{
		while (*cursor == ' ' || *cursor == '\t')
			cursor++;
		if (!*cursor)
			break;
		if (*cursor == '"')
		{
			words[count++] = ++cursor;
			while (*cursor && *cursor != '"')
				cursor++;
		}
		else
		{
			words[count++] = cursor;
			while (*cursor && *cursor != ' ' && *cursor != '\t')
				cursor++;
		}
		if (*cursor)
			*cursor++ = 0;
	}
	return count;
}

/* the text after the line's first count words (a notice's, a name's) */
static char const *rest_after_words(
	char const *line,
	long count)
{
	long index;

	for (index = 0; index < count; index++)
	{
		while (*line == ' ' || *line == '\t')
			line++;
		if (*line == '"')
		{
			line++;
			while (*line && *line != '"')
				line++;
			if (*line)
				line++;
		}
		else
		{
			while (*line && *line != ' ' && *line != '\t')
				line++;
		}
	}
	while (*line == ' ' || *line == '\t')
		line++;
	return line;
}

/* a text kept to printable ASCII, its quotes taken off, size - 1 at most */
static void printable_text(
	char *destination,
	long size,
	char const *source)
{
	long length = 0, first, last, index;

	/* (the printable characters' first and last but spaces, inside the
	quotes around them if there are: they go before the text is cut) */
	for (first = 0; source && source[first] && ((unsigned char)source[first] <= 0x20 ||
		(unsigned char)source[first] >= 0x7F); first++)
	{
	}
	for (last = source ? (long)strlen(source) - 1 : -1; last >= first && ((unsigned char)source[last] <= 0x20 ||
		(unsigned char)source[last] >= 0x7F); last--)
	{
	}
	if (last > first && source[first] == '"' && source[last] == '"')
	{
		first++;
		last--;
	}
	for (index = first; index <= last && length < size - 1; index++)
	{
		if ((unsigned char)source[index] >= 0x20 && (unsigned char)source[index] < 0x7F)
			destination[length++] = source[index];
	}
	destination[length] = 0;
	while (length > 0 && destination[length - 1] == ' ')
		destination[--length] = 0;
}

/* a duration: "30m", "2h", "7d", "1w", "45s", a bare number of minutes, or
"forever" (0): its seconds, -1 if it is none (ten years at most) */
static long parse_duration(
	char const *text)
{
	char *end;
	long number;
	long unit = 60;

	if (!csstrcasecmp(text, "forever") || !csstrcasecmp(text, "permanent") || !strcmp(text, "0"))
		return 0;
	if (!isdigit((unsigned char)text[0]))
		return -1;
	number = strtol(text, &end, 10);
	if (number <= 0 || number > 100000)
		return -1;
	switch (*end)
	{
	case 0: case 'm': unit = 60; break;
	case 's': unit = 1; break;
	case 'h': unit = 60 * 60; break;
	case 'd': unit = 24 * 60 * 60; break;
	case 'w': unit = 7 * 24 * 60 * 60; break;
	case 'y': unit = 365 * 24 * 60 * 60; break;
	default: return -1;
	}
	if (*end && end[1])
		return -1;
	if ((double)number * unit > 10.0 * 365 * 24 * 60 * 60)
		return -1;
	return number * unit;
}

static boolean parse_integer(
	char const *text,
	long minimum,
	long maximum,
	long *value)
{
	char *end;
	long number;

	if (!text || !text[0])
		return FALSE;
	number = strtol(text, &end, 10);
	if (*end || number < minimum || number > maximum)
		return FALSE;
	*value = number;
	return TRUE;
}

/* ---------- players */

static struct network_game *server_game(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();

	return network_server ? network_game_server_get_game(network_server) : NULL;
}

/* whether name begins with the length characters of text, in either case */
static boolean name_begins_with(
	char const *name,
	char const *text,
	long length)
{
	long index;

	for (index = 0; index < length; index++)
	{
		if (!name[index] || tolower((unsigned char)name[index]) != tolower((unsigned char)text[index]))
			return FALSE;
	}
	return TRUE;
}

/* a player by slot number or the start of their name (in either case; the
whole name first): the slot, else NONE (said why) */
static long find_player(
	char const *command,
	char const *text)
{
	struct network_game *game = server_game();
	char *end;
	long number, slot, found = NONE, matches = 0, exact = NONE, exact_count = 0;
	long length = (long)strlen(text);

	if (!game)
	{
		say("%s: the server is not hosting yet", command);
		return NONE;
	}
	if (!text[0])
	{
		say("%s: give a player's slot (sv_players) or name", command);
		return NONE;
	}
	number = strtol(text, &end, 10);
	if (!*end && isdigit((unsigned char)text[0]))
	{
		if (number < 0 || number >= MAXIMUM_SLOTS || !network_player_is_valid(&game->players[number]))
		{
			say("%s: no player in slot %ld (sv_players lists them)", command, number);
			return NONE;
		}
		return number;
	}
	for (slot = 0; slot < MAXIMUM_SLOTS; slot++)
	{
		char name[16];

		if (!server_status_player_name(slot, name, sizeof(name)))
			continue;
		if (name_begins_with(name, text, length))
		{
			found = slot;
			matches++;
			if ((long)strlen(name) == length)
			{
				exact = slot;
				exact_count++;
			}
		}
	}
	if (exact_count == 1)
		return exact;
	if (exact_count > 1)
	{
		say("%s: %ld players are named \"%s\": give a slot (sv_players)", command, exact_count, text);
		return NONE;
	}
	if (matches > 1)
	{
		say("%s: %ld players' names begin with \"%s\": give more of it, or a slot", command, matches, text);
		return NONE;
	}
	if (!matches)
	{
		say("%s: no player's name begins with \"%s\"", command, text);
		return NONE;
	}
	return found;
}

/* ---------- bans.txt */

static char ban_lines[MAXIMUM_BAN_LINES][BAN_LINE_SIZE];

/* bans.txt's lines (empty ones left out); how many, and whether that is
all of it */
static long read_bans(
	boolean *whole)
{
	FILE *file = fopen(BANS_FILE, "r");
	char line[BAN_LINE_SIZE];
	long count = 0;

	*whole = TRUE;
	if (!file)
		return 0;
	while (fgets(line, sizeof(line), file))
	{
		if (!strchr(line, '\n') && !feof(file))
			*whole = FALSE;
		line[strcspn(line, "\r\n")] = 0;
		if (!line[0])
			continue;
		if (count == MAXIMUM_BAN_LINES)
		{
			*whole = FALSE;
			break;
		}
		csstrcpy(ban_lines[count++], line);
	}
	fclose(file);
	return count;
}

/* a field of a bans.txt line ("ip", "reason", "until"), into text */
static boolean ban_field(
	char const *line,
	char const *name,
	char *text,
	long size)
{
	char key[32];
	char const *at;
	long length;

	snprintf(key, sizeof(key), "\t%s=", name);
	at = strstr(line, key);
	if (!at)
		return FALSE;
	at += strlen(key);
	length = (long)strcspn(at, "\t");
	snprintf(text, (size_t)size, "%.*s", (int)MIN(length, size - 1), at);
	return TRUE;
}

/* bans.txt written again from the lines, but for the one left out (put in
place whole) */
static boolean write_bans(
	long count,
	long left_out)
{
	/* (the lines and their ends: as big as the lines read can be) */
	static char text[MAXIMUM_BAN_LINES * BAN_LINE_SIZE];
	char path[1024];
	unsigned long size = 0;
	long index;

	/* (written whole by the host's path, then put in place: the game's
	fopen takes Xbox paths) */
	platform_translate_path(BANS_FILE, path, sizeof(path));
	for (index = 0; index < count; index++)
	{
		if (index != left_out)
		{
			long length = (long)strlen(ban_lines[index]);

			csmemcpy(text + size, ban_lines[index], length);
			size += (unsigned long)length;
			text[size++] = '\n';
		}
	}
	return posix_server_write_file(path, text, size) != 0;
}

void server_commands_bans_json(
	struct server_json *json)
{
	boolean whole;
	long count = read_bans(&whole);
	long index;

	server_json_array_begin(json, "bans");
	for (index = 0; index < count; index++)
	{
		char field[128];
		char when[32];

		server_json_object_begin(json, NULL);
		server_json_integer(json, "n", index);
		snprintf(when, sizeof(when), "%.*s", (int)strcspn(ban_lines[index], "\t"), ban_lines[index]);
		server_json_string(json, "when", when);
		server_json_string(json, "ip", ban_field(ban_lines[index], "ip", field, sizeof(field)) ? field : "");
		server_json_string(json, "hwid", ban_field(ban_lines[index], "hwid", field, sizeof(field)) ? field : "");
		/* (a Discord id without "(self-reported)" after it is a verified
		account's, which keeps it out) */
		if (ban_field(ban_lines[index], "discord_id", field, sizeof(field)) && strcmp(field, "none"))
		{
			char *reported = strstr(field, " (self-reported)");

			if (reported)
				*reported = 0;
			server_json_string(json, "discord_id", field);
			server_json_boolean(json, "discord_verified", reported == NULL);
		}
		else
		{
			server_json_string(json, "discord_id", "");
			server_json_boolean(json, "discord_verified", 0);
		}
		server_json_string(json, "players", ban_field(ban_lines[index], "players", field, sizeof(field)) ? field : "");
		server_json_string(json, "reason", ban_field(ban_lines[index], "reason", field, sizeof(field)) ? field : "");
		if (ban_field(ban_lines[index], "until", field, sizeof(field)))
			server_json_integer(json, "until", strtoul(field, NULL, 10));
		else
			server_json_null(json, "until");
		server_json_object_end(json);
	}
	server_json_array_end(json);
}

/* ---------- the commands */

struct server_command
{
	char const *name;
	/* Halo PC's dedicated server's name, or a short one (NULL: none) */
	char const *alias;
	int role;
	char const *usage;
	char const *description;
	boolean (*run)(char const *line, long count, char **words);
};

static struct server_command const server_command_table[];
static long server_command_count(void);

static boolean command_help(char const *line, long count, char **words)
{
	long index;

	if (count >= 2)
	{
		for (index = 0; index < server_command_count(); index++)
		{
			struct server_command const *command = &server_command_table[index];

			if (!csstrcasecmp(words[1], command->name) || (command->alias && !csstrcasecmp(words[1], command->alias)))
			{
				say("%s %s", command->name, command->usage);
				say("  %s (%s)", command->description, server_role_name(command->role));
				if (command->alias)
					say("  also %s", command->alias);
				return TRUE;
			}
		}
		say("help: no command is named %s", words[1]);
		return FALSE;
	}
	say("the server's commands (help <command> says more; the game's console commands run too):");
	for (index = 0; index < server_command_count(); index++)
	{
		struct server_command const *command = &server_command_table[index];

		if (command->role <= server_commands.role)
			say("  %-18s %s", command->name, command->description);
	}
	return TRUE;
}

static boolean command_status(char const *line, long count, char **words)
{
	struct server_json json;
	struct server_entry const *current = server_director_current();
	struct server_entry const *next = server_director_next();
	struct network_game *game = server_game();
	long uptime = (long)server_mode_uptime();
	char invite[P2P_LISTING_INVITE_SIZE] = "";

	if (count >= 2 && !strcmp(words[1], "--json"))
	{
		server_json_begin(&json);
		server_status_write(&json, server_commands.role);
		say("%s", server_json_text(&json));
		server_json_free(&json);
		return TRUE;
	}
	say("%s: %s, up %ldh%02ldm, %ld game%s played; OpenCE %s, network version %d", config_string("server.name"),
		server_status_state_name(server_director_state()), uptime / 3600, (uptime / 60) % 60,
		server_director_games_hosted(), server_director_games_hosted() == 1 ? "" : "s", server_mode_build(),
		HALO_PORT_NETWORK_VERSION);
	if (current)
	{
		long players = 0, slot;

		for (slot = 0; game && slot < MAXIMUM_SLOTS; slot++)
			players += network_player_is_valid(&game->players[slot]) != 0;
		say("game: %s on %s, %ld of %d players", current->gametype, current->map_name, players,
			game ? (int)game->maximum_players : 0);
	}
	if (next)
		say("next: %s on %s", next->gametype, next->map_name);
	say("listing: %s%s", !config_boolean("network.online") ? "internet play off (LAN only)" :
		config_boolean("server.public") ? "public" : "by invite only", config_string("server.password")[0] ?
		", with a password" : "");
	if (server_commands.role >= _server_role_admin && p2p_invite_link(invite, sizeof(invite)) && invite[0])
		say("invite: %s", invite);
	return TRUE;
}

static boolean command_players(char const *line, long count, char **words)
{
	struct network_game *game = server_game();
	struct server_json json;
	long slot, listed = 0;
	boolean addresses = server_commands.role >= _server_role_owner || config_boolean("server.log_addresses");

	if (count >= 2 && !strcmp(words[1], "--json"))
	{
		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_status_players(&json, server_commands.role);
		server_json_object_end(&json);
		say("%s", server_json_text(&json));
		server_json_free(&json);
		return TRUE;
	}
	for (slot = 0; game && slot < MAXIMUM_SLOTS; slot++)
	{
		struct network_player const *player = &game->players[slot];
		char name[16];
		long ping = NONE;
		struct player_datum *datum;

		if (!server_status_player_name(slot, name, sizeof(name)))
			continue;
		datum = game_engine_running() ? distributed_player(player->player_list_index) : NULL;
		if (datum)
			ping = distributed_player_ping(player->player_list_index);
		if (addresses)
		{
			unsigned long address = distributed_machine_real_address(player->machine_index);
			char const *hardware_id = network_game_server_machine_hardware_id(player->machine_index);

			say("%3ld %-12s team %d  machine %d  ping %s%ld  %lu.%lu.%lu.%lu  hwid %s", slot, name,
				(int)player->team_index, (int)player->machine_index, ping == NONE ? "-" : "", ping == NONE ? 0 : ping,
				(address >> 24) & 255, (address >> 16) & 255, (address >> 8) & 255, address & 255,
				hardware_id && hardware_id[0] ? hardware_id : "none");
		}
		else
		{
			say("%3ld %-12s team %d  machine %d  ping %s%ld", slot, name, (int)player->team_index,
				(int)player->machine_index, ping == NONE ? "-" : "", ping == NONE ? 0 : ping);
		}
		if (datum)
		{
			say("    score %ld  kills %d  deaths %d  assists %d",
				game_engine && game_engine->get_player_score ?
					game_engine->get_player_score(DATUM_INDEX_TO_ABSOLUTE_INDEX(player->player_list_index),
						_get_score_individual) : 0L,
				datum->statistics.kills[0], datum->statistics.deaths, datum->statistics.assists[0]);
		}
		listed++;
	}
	if (!listed)
		say("no players");
	return TRUE;
}

static boolean command_rotation(char const *line, long count, char **words)
{
	struct server_rotation const *rotation = server_director_rotation();
	long index;
	long playing = server_director_index();

	for (index = 0; index < rotation->count; index++)
	{
		struct server_entry const *entry = &rotation->entries[index];

		say("%c%2ld %s%s%s", index == playing ? '>' : ' ', index, entry->text, entry->rejected[0] ? "  (left out: " : "",
			entry->rejected[0] ? entry->rejected : "");
		if (entry->rejected[0])
			say("      )");
	}
	if (!rotation->count)
		say("the rotation is empty");
	return TRUE;
}

static boolean command_maps(char const *line, long count, char **words)
{
	struct server_json json;
	struct server_json_document document;
	long index;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_status_maps(&json);
	server_json_object_end(&json);
	if (server_json_parse(&document, server_json_text(&json), (int)strlen(server_json_text(&json)), 16))
	{
		int maps = server_json_member(&document, 0, "maps");

		for (index = 0; server_json_element(&document, maps, index) >= 0; index++)
		{
			int map = server_json_element(&document, maps, index);
			char name[128], kind[16];

			if (server_json_get_string(&document, server_json_member(&document, map, "name"), name, sizeof(name)) &&
				server_json_get_string(&document, server_json_member(&document, map, "kind"), kind, sizeof(kind)))
			{
				say("%-24s %s", name, kind);
			}
		}
		server_json_document_free(&document);
	}
	server_json_free(&json);
	return TRUE;
}

static boolean command_gametypes(char const *line, long count, char **words)
{
	char text[1024] = "";
	long index, length = 0;
	char const *name;

	for (index = 0; (name = server_gametype_builtin_name(index)) != NULL; index++)
		length += snprintf(text + length, sizeof(text) - (size_t)length, "%s%s", index ? ", " : "", name);
	say("built in: %s", text);
	say("and gametypes/<name>.toml files, and file=<name> for gametypes/<name>.gametype");
	return TRUE;
}

static boolean command_bans(char const *line, long count, char **words)
{
	boolean whole;
	long lines = read_bans(&whole);
	long index;

	for (index = 0; index < lines; index++)
	{
		char until[32];

		if (ban_field(ban_lines[index], "until", until, sizeof(until)) &&
			strtoul(until, NULL, 10) <= posix_server_unix_seconds())
		{
			say("%3ld (ended) %s", index, ban_lines[index]);
		}
		else
		{
			say("%3ld %s", index, ban_lines[index]);
		}
	}
	if (!lines)
		say("no bans");
	if (!whole)
		say("(bans.txt has more lines, or longer ones, than the first %d read)", MAXIMUM_BAN_LINES);
	return TRUE;
}

/* the settings sv_set may change: a prefix ends with a "." */
static char const *const settable_settings[] =
{
	"server.name", "server.max_players", "server.min_players", "server.password", "server.public",
	"server.rotation_fallback", "server.lobby_seconds", "server.postgame_seconds", "server.empty_seconds",
	"server.idle_minutes", "server.team_start", "server.team_switch",
	"server.team_switch_cooldown_seconds", "server.motd", "server.frame_rate", "server.web_password",
	"server.web_moderator_password", "server.log_format", "server.log_addresses", "server.status_file",
	"server.auto_update", "server.coop_difficulty", "server.region", "server.master_publish",
	"server.master_publish_address", "server.metrics", "server.chat_filter", "server.chat_filter_slurs",
	"server.chat_filter_swearing", "server.chat_filter_insults", "server.vpn_policy", "server.vpn_fail",
	"server.vpn_check_lan", "server.link_min_age_days", "server.link_one_seat", "server.update_check_minutes",
	"server.update_game_minutes", "server.max_machines_per_address", "server.afk_kick_minutes",
	"server.afk_kick_when_full", "server.afk_kick_full_minutes", "server.coop_bsp_switch",
	"server.coop_bsp_switch_cooldown_seconds", "server.map_vote", "server.map_vote_choices", "server.map_vote_seconds",
	"network.text_chat", "network.voice_lobby", "network.voice_mode", "network.voice_kbps", "network.votekick",
	"network.vote_end_game", "network.votekick_threshold", "network.votekick_percent", "network.votekick_minutes", "network.votekick_ban_minutes", "network.map_share", "network.map_share_upload",
	"network.public_address", "network.coop_friendly_fire", "network.coop_player_collisions",
	"network.vehicle_riding", "server.vehicle_riding_override_gametype",
	"network.coop_ally_friendly_fire", "network.coop_team_wipe", "network.shield_effects", "network.fire_origins",
};

/* those of them the owner's console alone changes (the standard input, the
init file): a file the server writes to, the update it installs, and the
addresses it shows every role */
static char const *const owner_settings[] =
{
	"server.status_file", "server.auto_update", "server.log_addresses",
};

static boolean setting_owners(
	char const *name)
{
	long index;

	for (index = 0; index < (long)NUMBEROF(owner_settings); index++)
	{
		if (!strcmp(name, owner_settings[index]))
			return TRUE;
	}
	return FALSE;
}

static boolean setting_settable(
	char const *name)
{
	long index;

	for (index = 0; index < (long)NUMBEROF(settable_settings); index++)
	{
		if (!strcmp(name, settable_settings[index]))
			return TRUE;
	}
	return FALSE;
}

/* the values the settings sv_set changes take, as port_config.c's comments
say them (and the director and the game hold them to): a word of the
choices ("a|b|c", in either case), or a number from the least to the most
(NULL: none) */
static struct
{
	char const *name;
	char const *choices;
	char const *minimum;
	char const *maximum;
} const setting_values[] =
{
	{ "server.max_players", NULL, "2", "127" },
	{ "server.min_players", NULL, "1", "16" },
	{ "server.lobby_seconds", NULL, "0", "300" },
	{ "server.postgame_seconds", NULL, "0", "300" },
	{ "server.empty_seconds", NULL, "0", NULL },
	{ "server.idle_minutes", NULL, "0", NULL },
	{ "server.frame_rate", NULL, "30", "120" },
	{ "server.team_start", "one|both|skip", NULL, NULL },
	{ "server.team_switch_cooldown_seconds", NULL, "0", "3600" },
	{ "server.log_format", "text|json", NULL, NULL },
	{ "server.auto_update", "off|check|install", NULL, NULL },
	{ "server.coop_difficulty", "easy|normal|hard|impossible", NULL, NULL },
	{ "network.text_chat", "on|links|off", NULL, NULL },
	{ "server.vpn_policy", "off|link|block", NULL, NULL },
	{ "server.vpn_fail", "open|closed", NULL, NULL },
	{ "server.link_min_age_days", NULL, "0", "3650" },
	{ "server.update_check_minutes", NULL, "5", "1440" },
	{ "server.update_game_minutes", NULL, "0", "240" },
	{ "server.max_machines_per_address", NULL, "0", "128" },
	{ "server.afk_kick_minutes", NULL, "0", "60" },
	{ "server.afk_kick_full_minutes", NULL, "2", "60" },
	{ "server.coop_bsp_switch", "majority|anyone", NULL, NULL },
	{ "server.coop_bsp_switch_cooldown_seconds", NULL, "0", "300" },
	{ "server.map_vote_choices", NULL, "2", "8" },
	{ "server.map_vote_seconds", NULL, "5", "120" },
	{ "network.voice_mode", "off|team_proximity|team_enemy_proximity|team_global|team_global_enemy_proximity", NULL,
		NULL },
	{ "network.voice_kbps", NULL, "8", "64" },
	{ "network.votekick_threshold", "dynamic|static", NULL, NULL },
	{ "network.votekick_percent", NULL, "1", "100" },
	{ "network.votekick_minutes", NULL, "0", "60" },
	{ "network.votekick_ban_minutes", NULL, "1", "1440" },
	{ "network.map_share_upload", NULL, "16", "256000" },
	{ "network.coop_friendly_fire", "off|on|shields_only|explosives_only", NULL, NULL },
	{ "network.coop_team_wipe", "auto|revert|respawn", NULL, NULL },
	{ "network.shield_effects", "stock|glow|light_show|overshield|glow_overshield|light_show_overshield|players", NULL,
		NULL },
};

/* the setting's values (setting_values), NONE if any is taken */
static long setting_values_index(
	char const *name)
{
	long index;

	for (index = 0; index < (long)NUMBEROF(setting_values); index++)
	{
		if (!strcmp(name, setting_values[index].name))
			return index;
	}
	return NONE;
}

/* whether the value is one of the choices ("a|b|c"), in either case: TRUE
with the choice as the choices spell it */
static boolean setting_choice(
	char const *choices,
	char const *value,
	char *choice,
	long size)
{
	long length = (long)strlen(value);

	while (*choices)
	{
		long choice_length = (long)strcspn(choices, "|");

		if (choice_length == length && name_begins_with(choices, value, length))
		{
			snprintf(choice, (size_t)size, "%.*s", (int)choice_length, choices);
			return TRUE;
		}
		choices += choice_length + (choices[choice_length] == '|');
	}
	return FALSE;
}

/* a value of the setting's type as JSON: a boolean, a number, a text */
static void setting_json_value(
	struct server_json *json,
	char const *key,
	char const *type,
	char const *text)
{
	if (!strcmp(type, "boolean"))
		server_json_boolean(json, key, !csstrcasecmp(text, "true"));
	else if (!strcmp(type, "integer"))
		server_json_integer(json, key, strtol(text, NULL, 10));
	else if (!strcmp(type, "real"))
		server_json_real(json, key, strtod(text, NULL));
	else
		server_json_string(json, key, text);
}

boolean server_commands_setting_secret(
	char const *name)
{
	return !strcmp(name, "server.password") || !strcmp(name, "server.web_password") ||
		!strcmp(name, "server.web_moderator_password");
}

/* the settable settings, for the API (GET /api/settings) */
void server_commands_settings_json(
	struct server_json *json)
{
	struct config_setting_info info;
	long index;

	server_json_array_begin(json, "settings");
	for (index = 0; config_setting_info(index, &info); index++)
	{
		char value[400];
		long values = setting_values_index(info.name);

		if (!setting_settable(info.name))
			continue;
		config_text(info.name, value, sizeof(value));
		server_json_object_begin(json, NULL);
		server_json_string(json, "name", info.name);
		server_json_string(json, "type", info.type);
		/* (each value of its type: true, 16, 1.5, "text"; a secret's never) */
		if (server_commands_setting_secret(info.name))
		{
			server_json_string(json, "value", "");
			server_json_boolean(json, "set", value[0] != 0);
			server_json_boolean(json, "secret", TRUE);
			server_json_string(json, "default", "");
		}
		else
		{
			setting_json_value(json, "value", info.type, value);
			server_json_boolean(json, "secret", FALSE);
			setting_json_value(json, "default", info.type, info.default_text);
		}
		server_json_string(json, "comment", info.comment);
		if (info.environment)
			server_json_string(json, "env", info.environment);
		else
			server_json_null(json, "env");
		server_json_boolean(json, "pinned", info.pinned);
		if (values != NONE && setting_values[values].choices)
		{
			char const *choice = setting_values[values].choices;

			server_json_array_begin(json, "choices");
			while (*choice)
			{
				long length = (long)strcspn(choice, "|");

				server_json_string_length(json, NULL, choice, (int)length);
				choice += length + (choice[length] == '|');
			}
			server_json_array_end(json);
		}
		else
		{
			server_json_null(json, "choices");
		}
		if (values != NONE && setting_values[values].minimum)
			server_json_integer(json, "min", strtol(setting_values[values].minimum, NULL, 10));
		else
			server_json_null(json, "min");
		if (values != NONE && setting_values[values].maximum)
			server_json_integer(json, "max", strtol(setting_values[values].maximum, NULL, 10));
		else
			server_json_null(json, "max");
		server_json_object_end(json);
	}
	server_json_array_end(json);
}

/* a setting changed (sv_set): FALSE and why if it cannot be */
boolean server_commands_set_setting(
	char const *name,
	char const *value,
	char *reason,
	long reason_size)
{
	struct config_setting_info info;
	long index;
	long values = setting_values_index(name);
	char choice[64];

	for (index = 0; config_setting_info(index, &info) && strcmp(info.name, name); index++)
	{
	}
	if (!info.name || strcmp(info.name, name) || !setting_settable(name))
	{
		snprintf(reason, (size_t)reason_size, "%s is not a setting sv_set changes (sv_settings lists them)", name);
		return FALSE;
	}
	if (info.pinned)
	{
		snprintf(reason, (size_t)reason_size, "%s is set by %s for this run", name, info.environment);
		return FALSE;
	}
	if (setting_owners(name) && server_commands.role < _server_role_owner)
	{
		snprintf(reason, (size_t)reason_size, "%s needs the owner role (the server's console)", name);
		return FALSE;
	}
	/* (printable text only: a line's end or another control character in a
	value is no setting's) */
	{
		char const *character;

		for (character = value; *character; character++)
		{
			if ((unsigned char)*character < 0x20 || *character == 0x7F)
			{
				snprintf(reason, (size_t)reason_size, "%s's value has a control character", name);
				return FALSE;
			}
		}
	}
	/* (each type's text checked, and the setting's values: config_write
	takes any) */
	if (!strcmp(info.type, "integer"))
	{
		long minimum = values != NONE && setting_values[values].minimum ?
			strtol(setting_values[values].minimum, NULL, 10) : -2147483647L;
		long maximum = values != NONE && setting_values[values].maximum ?
			strtol(setting_values[values].maximum, NULL, 10) : 2147483647L;
		long number;

		if (!parse_integer(value, minimum, maximum, &number))
		{
			if (values != NONE && setting_values[values].maximum)
				snprintf(reason, (size_t)reason_size, "%s takes a whole number from %ld to %ld", name, minimum, maximum);
			else if (values != NONE)
				snprintf(reason, (size_t)reason_size, "%s takes a whole number from %ld", name, minimum);
			else
				snprintf(reason, (size_t)reason_size, "%s takes a whole number", name);
			return FALSE;
		}
	}
	else if (!strcmp(info.type, "string") && values != NONE && setting_values[values].choices)
	{
		if (!setting_choice(setting_values[values].choices, value, choice, sizeof(choice)))
		{
			snprintf(reason, (size_t)reason_size, "%s is one of %s", name, setting_values[values].choices);
			return FALSE;
		}
		value = choice;
	}
	else if (!strcmp(info.type, "boolean"))
	{
		if (csstrcasecmp(value, "true") && csstrcasecmp(value, "false") && strcmp(value, "1") && strcmp(value, "0") &&
			csstrcasecmp(value, "on") && csstrcasecmp(value, "off"))
		{
			snprintf(reason, (size_t)reason_size, "%s takes true or false", name);
			return FALSE;
		}
		value = !csstrcasecmp(value, "true") || !strcmp(value, "1") || !csstrcasecmp(value, "on") ? "true" : "false";
	}
	if ((!strcmp(name, "server.web_password") || !strcmp(name, "server.web_moderator_password")) && value[0] &&
		strlen(value) < 8)
	{
		snprintf(reason, (size_t)reason_size, "a web password has 8 characters or more");
		return FALSE;
	}
	if (strlen(value) > 256)
	{
		snprintf(reason, (size_t)reason_size, "the value is longer than 256 characters");
		return FALSE;
	}
	if (!config_write(name, value))
		server_log(_server_log_warning, "server", "config.toml cannot be written: %s holds for this run", name);
	if (!strcmp(name, "server.log_format"))
		server_log_set_json(!strcmp(value, "json"));
	if (!strncmp(name, "server.", 7))
		server_director_apply_settings();
	return TRUE;
}

/* (the web's: server_http.c) as server_commands_set_setting, with the
session's role (an admin's at most, but from this machine an owner's), and
an HTTP status for a refusal: 409 for a setting its environment variable
sets, 404 for none sv_set changes, 403 for the owner's, else 400 */
int server_commands_set_setting_text(
	char const *name,
	char const *value,
	int role,
	char *reason,
	long reason_size,
	int *code)
{
	struct config_setting_info info;
	long index;

	*code = 400;
	for (index = 0; config_setting_info(index, &info) && strcmp(info.name, name); index++)
	{
	}
	if (!info.name || strcmp(info.name, name) || !setting_settable(name))
		*code = 404;
	else if (info.pinned)
		*code = 409;
	else if (setting_owners(name) && role < _server_role_owner)
		*code = 403;
	{
		int previous_role = server_commands.role;
		boolean set;

		server_commands.role = MIN(role, _server_role_owner);
		set = server_commands_set_setting(name, !strcmp(value, "null") ? "" : value, reason, reason_size);
		server_commands.role = previous_role;
		return set;
	}
}

static boolean command_settings(char const *line, long count, char **words)
{
	struct config_setting_info info;
	long index;

	for (index = 0; config_setting_info(index, &info); index++)
	{
		char value[400];

		if (!setting_settable(info.name))
			continue;
		config_text(info.name, value, sizeof(value));
		say("%-32s %s%s", info.name, server_commands_setting_secret(info.name) ? (value[0] ? "(set)" : "(none)") : value,
			info.pinned ? "  (set by its environment variable)" : "");
	}
	return TRUE;
}

static boolean command_version(char const *line, long count, char **words)
{
	say("OpenCE %s, network version %d", server_mode_build(), HALO_PORT_NETWORK_VERSION);
	return TRUE;
}

static boolean command_log(char const *line, long count, char **words)
{
	static struct server_log_line lines[200];
	long wanted = 20;
	unsigned long latest = server_log_latest();
	unsigned long next, missed;
	long read, index;

	if (count >= 2 && !parse_integer(words[1], 1, (long)NUMBEROF(lines), &wanted))
	{
		say("sv_log: give how many lines (1 to %d)", (int)NUMBEROF(lines));
		return FALSE;
	}
	read = server_log_read(latest > (unsigned long)wanted ? latest - (unsigned long)wanted : 0, lines, (int)wanted, &next,
		&missed);
	/* (said into the log itself as it is read: the lines copied first) */
	for (index = 0; index < read; index++)
		say("%s: %s", lines[index].category, lines[index].text);
	return TRUE;
}

static boolean command_masters(char const *line, long count, char **words)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_master_status(&json);
	server_json_object_end(&json);
	say("%s", server_json_text(&json));
	server_json_free(&json);
	return TRUE;
}

static boolean command_publisher(char const *line, long count, char **words)
{
	unsigned char public_key[SERVER_MASTER_PUBLIC_KEY_SIZE];
	char key_id[2 * SERVER_MASTER_KEY_ID_SIZE + 1];
	char hex[2 * SERVER_MASTER_PUBLIC_KEY_SIZE + 1];
	long index;

	if (!server_master_publisher(public_key, key_id))
	{
		say("there is no publisher key (halo --server --keygen makes one; HALO_SERVER_PUBLISHER_KEY holds one)");
		return FALSE;
	}
	for (index = 0; index < SERVER_MASTER_PUBLIC_KEY_SIZE; index++)
		snprintf(hex + 2 * index, 3, "%02x", public_key[index]);
	say("publisher key id %s, public key ed25519:%s", key_id, hex);
	return TRUE;
}

/* a notice: every machine's (machine NONE), or one's */
static void send_notice(
	long machine_index,
	char const *text)
{
	char notice[NOTICE_SIZE];

	snprintf(notice, sizeof(notice), "%s", text);
	if (!game_in_progress() || !game_engine_running())
	{
		/* (in the lobby the notice reaches no client: said here) */
		say("(in the lobby: no machine sees a notice until a game runs) %s", notice);
		return;
	}
	if (machine_index == NONE)
		distributed_send_notice(notice);
	else
		distributed_send_notice_to_machine(machine_index, notice);
}

static boolean command_say(char const *line, long count, char **words)
{
	char text[NOTICE_SIZE];
	unsigned long minute = posix_server_unix_seconds() / 60;

	printable_text(text, sizeof(text), rest_after_words(line, 1));
	if (!text[0])
	{
		say("say: give the text");
		return FALSE;
	}
	if (server_commands.role < _server_role_owner)
	{
		if (minute != server_commands.say_minute)
		{
			server_commands.say_minute = minute;
			server_commands.say_count = 0;
		}
		if (++server_commands.say_count > SAY_PER_MINUTE)
		{
			say("say: %d lines a minute at most", SAY_PER_MINUTE);
			return FALSE;
		}
	}
	send_notice(NONE, text);
	server_log(_server_log_info, "chat", "server: %s", text);
	return TRUE;
}

static boolean command_tell(char const *line, long count, char **words)
{
	struct network_game *game = server_game();
	char text[NOTICE_SIZE];
	long slot;

	if (count < 3)
	{
		say("tell: give a player and the text");
		return FALSE;
	}
	slot = find_player("tell", words[1]);
	if (slot == NONE)
		return FALSE;
	printable_text(text, sizeof(text), rest_after_words(line, 2));
	send_notice(game->players[slot].machine_index, text);
	return TRUE;
}

static void moderation_event(
	char const *event,
	char const *name,
	char const *reason,
	unsigned long until)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", event);
	server_json_string(&json, "name", name);
	server_json_string(&json, "by", server_role_name(server_commands.role));
	if (reason)
		server_json_string(&json, "reason", reason);
	if (until)
		server_json_integer(&json, "until", until);
	server_json_object_end(&json);
	server_event_add("moderation", server_json_text(&json));
	server_json_free(&json);
}

static boolean command_kick(char const *line, long count, char **words)
{
	struct network_game *game = server_game();
	char name[16];
	long slot;

	if (count < 2)
	{
		say("kick: give a player's slot (sv_players) or name");
		return FALSE;
	}
	slot = find_player("kick", words[1]);
	if (slot == NONE)
		return FALSE;
	server_status_player_name(slot, name, sizeof(name));
	if (!network_game_server_kick_machine_of_player(game->players[slot].machine_index, FALSE))
		return FALSE;
	server_director_player_removed(game->players[slot].machine_index, "kicked");
	server_log(_server_log_info, "moderation", "%s kicked (%s)", name, server_role_name(server_commands.role));
	moderation_event("kick", name, NULL, 0);
	return TRUE;
}

/* whether the role may ban for so long (0: for ever): a moderator for a
day at most */
static boolean ban_duration_allowed(
	int role,
	long seconds)
{
	return role >= _server_role_admin || (seconds > 0 && seconds <= MODERATOR_BAN_SECONDS);
}

static boolean command_ban(char const *line, long count, char **words)
{
	struct network_game *game = server_game();
	char name[16];
	char reason[96] = "";
	long slot, seconds = 0;
	long reason_words = 2;
	unsigned long until = 0;

	if (count < 2)
	{
		say("ban: give a player's slot (sv_players) or name, then a duration (30m, 2h, 7d, forever) and a reason");
		return FALSE;
	}
	if (count >= 3)
	{
		seconds = parse_duration(words[2]);
		if (seconds >= 0)
			reason_words = 3;
		else
			seconds = 0;
	}
	if (!ban_duration_allowed(server_commands.role, seconds))
	{
		say("ban: a moderator's ban is for a day at most (give a duration: 30m, 2h, 1d)");
		return FALSE;
	}
	slot = find_player("ban", words[1]);
	if (slot == NONE)
		return FALSE;
	printable_text(reason, sizeof(reason), rest_after_words(line, reason_words));
	for (count = 0; reason[count]; count++)
	{
		if (reason[count] == '\t')
			reason[count] = ' ';
	}
	server_status_player_name(slot, name, sizeof(name));
	if (seconds > 0)
		until = posix_server_unix_seconds() + (unsigned long)seconds;
	{
		/* (the reason, and who gave it: the reason cut to leave its room) */
		char full_reason[sizeof(reason) + 32];

		if (server_commands.role < _server_role_owner)
		{
			snprintf(full_reason, sizeof(full_reason), "%s (by the %s)", reason[0] ? reason : "banned by the server",
				server_role_name(server_commands.role));
		}
		else
		{
			snprintf(full_reason, sizeof(full_reason), "%s", reason[0] ? reason : "banned by the server");
		}
		network_distributed_ban_details(full_reason, until);
	}
	if (!network_game_server_kick_machine_of_player(game->players[slot].machine_index, TRUE))
	{
		network_distributed_ban_details(NULL, 0);
		return FALSE;
	}
	server_director_player_removed(game->players[slot].machine_index, "banned");
	if (until)
	{
		say("%s banned for %ld minutes", name, seconds / 60);
		server_log(_server_log_info, "moderation", "%s banned until %lu (%s)", name, until,
			server_role_name(server_commands.role));
	}
	else
	{
		server_log(_server_log_info, "moderation", "%s banned (%s)", name, server_role_name(server_commands.role));
	}
	moderation_event("ban", name, reason[0] ? reason : NULL, until);
	return TRUE;
}

static boolean command_unban(char const *line, long count, char **words)
{
	boolean whole;
	long lines = read_bans(&whole);
	long number, index, found = NONE;

	if (count < 2)
	{
		say("unban: give a ban's number (sv_bans), an address, a hardware id or discord:<id>");
		return FALSE;
	}
	if (!whole)
	{
		say("unban: bans.txt has more lines (or longer ones) than the server reads: take the line out by hand");
		return FALSE;
	}
	if (parse_integer(words[1], 0, lines - 1, &number))
	{
		found = number;
	}
	else
	{
		for (index = 0; index < lines && found == NONE; index++)
		{
			char field[128];

			if ((ban_field(ban_lines[index], "ip", field, sizeof(field)) && !strcmp(field, words[1])) ||
				(ban_field(ban_lines[index], "hwid", field, sizeof(field)) && !strcmp(field, words[1])) ||
				(!strncmp(words[1], "discord:", 8) && ban_field(ban_lines[index], "discord_id", field, sizeof(field)) &&
					!strcmp(field, words[1] + 8)))
			{
				found = index;
			}
		}
	}
	if (found == NONE)
	{
		say("unban: no ban %s (sv_bans lists them)", words[1]);
		return FALSE;
	}
	if (!write_bans(lines, found))
	{
		say("unban: bans.txt cannot be written");
		return FALSE;
	}
	/* (and its machine let into the game it was dropped from: by the line's
	address, the real one an internet play peer's stand-in has, or its
	hardware id) */
	{
		char field[128];
		char hardware_id[128] = "";
		unsigned int bytes[4];
		unsigned long address = 0;

		if (ban_field(ban_lines[found], "ip", field, sizeof(field)) &&
			sscanf(field, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) == 4 &&
			bytes[0] < 256 && bytes[1] < 256 && bytes[2] < 256 && bytes[3] < 256)
		{
			address = (unsigned long)bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
		}
		if (!ban_field(ban_lines[found], "hwid", hardware_id, sizeof(hardware_id)) || !strcmp(hardware_id, "none"))
			hardware_id[0] = 0;
		network_game_server_port_forget_kicked_address(address, hardware_id);
	}
	say("unbanned: %s", ban_lines[found]);
	server_log(_server_log_info, "moderation", "ban %ld taken out (%s)", found, server_role_name(server_commands.role));
	/* (its players' names: the line's address and hardware id are the
	owner's to see, and the event goes to every role) */
	{
		char players[128];

		if (!ban_field(ban_lines[found], "players", players, sizeof(players)))
			snprintf(players, sizeof(players), "ban %ld", found);
		moderation_event("unban", players, NULL, 0);
	}
	return TRUE;
}

/* an address's text (a.b.c.d) checked: TRUE with the address (host byte
order) and its text again, written plainly */
static boolean parse_ipv4(
	char const *text,
	unsigned long *address,
	char *kept,
	long size)
{
	unsigned int bytes[4];
	char end;

	if (strspn(text, "0123456789.") != strlen(text) ||
		sscanf(text, "%3u.%3u.%3u.%3u%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3], &end) != 4 ||
		bytes[0] > 255 || bytes[1] > 255 || bytes[2] > 255 || bytes[3] > 255)
	{
		return FALSE;
	}
	*address = (unsigned long)bytes[0] << 24 | (unsigned long)bytes[1] << 16 | (unsigned long)bytes[2] << 8 | bytes[3];
	snprintf(kept, (size_t)size, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
	return *address != 0;
}

/* a hardware id's text checked: hex digits (in either case), 32 at most;
TRUE with it in lower case */
static boolean parse_hardware_id(
	char const *text,
	char *kept,
	long size)
{
	long length = (long)strlen(text);
	long index;

	if (!length || length > 2 * P2P_HARDWARE_ID_BYTES || length >= size ||
		(long)strspn(text, "0123456789abcdefABCDEF") != length)
	{
		return FALSE;
	}
	for (index = 0; index <= length; index++)
		kept[index] = (char)tolower((unsigned char)text[index]);
	return TRUE;
}

/* a verified Discord account's id checked: decimal digits, 20 at most (a
snowflake); TRUE with it */
static boolean parse_discord_id(
	char const *text,
	char *kept,
	long size)
{
	long length = (long)strlen(text);

	if (!length || length > 20 || length >= size || (long)strspn(text, "0123456789") != length)
		return FALSE;
	snprintf(kept, (size_t)size, "%s", text);
	return TRUE;
}

/* a ban of an address, a hardware id, a verified Discord account or more
than one, with no player needed (sv_ban_address, POST /api/bans): its line
in bans.txt as ban writes one (so that sv_bans, unban and the joins read
it), and each machine present of that address, hardware id or account
dropped as ban drops it. 200 with the new ban's number (sv_bans'), else 400
(a part that will not do) or 500 (bans.txt not written) and why */
static int ban_address(
	char const *ip,
	char const *hwid,
	char const *discord,
	char const *names,
	long seconds,
	char const *reason,
	char *why,
	long why_size,
	long *number)
{
	struct network_game *game = server_game();
	char address_text[32] = "none";
	char hardware_id[2 * P2P_HARDWARE_ID_BYTES + 1] = "";
	char discord_id[24] = "";
	char kept_names[64];
	char kept_reason[96];
	char full_reason[sizeof(kept_reason) + 32];
	char dropped[96] = "";
	boolean done[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	unsigned long address = 0, until = 0;
	boolean whole;
	long slot, index;

	if ((!ip || !ip[0]) && (!hwid || !hwid[0]) && (!discord || !discord[0]))
	{
		snprintf(why, (size_t)why_size, "give an address (a.b.c.d), a hardware id or a Discord account's id");
		return 400;
	}
	if (discord && discord[0] && !parse_discord_id(discord, discord_id, sizeof(discord_id)))
	{
		snprintf(why, (size_t)why_size, "a Discord account's id is up to 20 digits");
		return 400;
	}
	if (ip && ip[0] && !parse_ipv4(ip, &address, address_text, sizeof(address_text)))
	{
		snprintf(why, (size_t)why_size, "%s is not an IPv4 address (a.b.c.d)", ip);
		return 400;
	}
	if (hwid && hwid[0] && !parse_hardware_id(hwid, hardware_id, sizeof(hardware_id)))
	{
		snprintf(why, (size_t)why_size, "a hardware id is up to %d hex digits", 2 * P2P_HARDWARE_ID_BYTES);
		return 400;
	}
	printable_text(kept_names, sizeof(kept_names), names);
	printable_text(kept_reason, sizeof(kept_reason), reason);
	/* (the reason, and who gave it, as ban writes it) */
	if (server_commands.role < _server_role_owner)
	{
		snprintf(full_reason, sizeof(full_reason), "%s (by the %s)", kept_reason[0] ? kept_reason :
			"banned by the server", server_role_name(server_commands.role));
	}
	else
	{
		snprintf(full_reason, sizeof(full_reason), "%s", kept_reason[0] ? kept_reason : "banned by the server");
	}
	if (seconds > 0)
		until = posix_server_unix_seconds() + (unsigned long)seconds;
	if (!network_distributed_ban_record(address_text, hardware_id, discord_id, kept_names, full_reason, until))
	{
		snprintf(why, (size_t)why_size, "bans.txt cannot be written");
		return 500;
	}
	*number = read_bans(&whole) - 1;
	/* (each machine present of the address or the hardware id: dropped and
	kept out as ban does, its line written already) */
	csmemset(done, 0, sizeof(done));
	for (slot = 0; game && slot < MAXIMUM_SLOTS; slot++)
	{
		long machine_index;
		char const *machine_hardware_id;
		char name[16];

		if (!network_player_is_valid(&game->players[slot]))
			continue;
		machine_index = game->players[slot].machine_index;
		if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES || done[machine_index])
			continue;
		machine_hardware_id = network_game_server_machine_hardware_id(machine_index);
		{
			char machine_discord_id[32] = "";
			boolean verified = network_link_machine_verified(machine_index, machine_discord_id,
				sizeof(machine_discord_id), NULL, 0);

			if (!(address && distributed_machine_real_address(machine_index) == address) &&
				!(hardware_id[0] && machine_hardware_id && !strcmp(machine_hardware_id, hardware_id)) &&
				!(discord_id[0] && verified && !strcmp(machine_discord_id, discord_id)))
			{
				continue;
			}
		}
		done[machine_index] = TRUE;
		/* (its players' names, for the event, before they go) */
		for (index = 0; index < MAXIMUM_SLOTS; index++)
		{
			if (network_player_is_valid(&game->players[index]) && game->players[index].machine_index == machine_index &&
				server_status_player_name(index, name, sizeof(name)))
			{
				snprintf(dropped + strlen(dropped), sizeof(dropped) - strlen(dropped), "%s%s", dropped[0] ? ", " : "",
					name);
			}
		}
		network_distributed_ban_details(full_reason, until);
		network_distributed_ban_unrecorded(TRUE);
		if (network_game_server_kick_machine_of_player(machine_index, TRUE))
		{
			server_director_player_removed(machine_index, "banned");
		}
		else
		{
			network_distributed_ban_details(NULL, 0);
			network_distributed_ban_unrecorded(FALSE);
		}
	}
	server_log(_server_log_info, "moderation", "ban %ld added: %s%s%s%s%s (%s)", *number,
		config_boolean("server.log_addresses") ? (address ? address_text : "") : address ? "an address" : "",
		address && hardware_id[0] ? " and " : "",
		hardware_id[0] ? (config_boolean("server.log_addresses") ? hardware_id : "a hardware id") : "",
		(address || hardware_id[0]) && discord_id[0] ? " and " : "",
		discord_id[0] ? (config_boolean("server.log_addresses") ? discord_id : "a Discord account") : "",
		server_role_name(server_commands.role));
	moderation_event("ban", kept_names[0] ? kept_names : dropped, kept_reason[0] ? kept_reason : NULL, until);
	return 200;
}

static boolean command_ban_address(char const *line, long count, char **words)
{
	char why[128];
	long seconds = 0, reason_words = 2, number = 0;
	boolean address, discord;

	if (count < 2)
	{
		say("sv_ban_address: give an address (a.b.c.d), a hardware id or discord:<id>, then a duration (30m, 2h, 7d, "
			"forever) and a reason");
		return FALSE;
	}
	if (count >= 3)
	{
		seconds = parse_duration(words[2]);
		if (seconds >= 0)
			reason_words = 3;
		else
			seconds = 0;
	}
	address = strchr(words[1], '.') != NULL;
	discord = !strncmp(words[1], "discord:", 8);
	if (ban_address(address ? words[1] : "", address || discord ? "" : words[1], discord ? words[1] + 8 : "", "",
		seconds, rest_after_words(line, reason_words), why, sizeof(why), &number) != 200)
	{
		say("sv_ban_address: %s", why);
		return FALSE;
	}
	if (seconds > 0)
		say("ban %ld added, for %ld minutes", number, seconds / 60);
	else
		say("ban %ld added", number);
	return TRUE;
}

/* (the web's: server_http.c, POST /api/bans) ban_address with the session's
role, the duration as text (empty: for ever): 200 with the ban's number, or
400 or 500 and why */
int server_commands_ban_address_text(
	char const *ip,
	char const *hwid,
	char const *discord_id,
	char const *names,
	char const *duration,
	char const *reason,
	int role,
	char *message,
	long message_size,
	long *number)
{
	int previous_role = server_commands.role;
	long seconds = 0;
	int answer;

	if (duration && duration[0])
	{
		seconds = parse_duration(duration);
		if (seconds < 0)
		{
			snprintf(message, (size_t)message_size, "duration is 30m, 2h, 7d, 1w or forever (ten years at most)");
			return 400;
		}
	}
	server_commands.role = role;
	answer = ban_address(ip, hwid, discord_id, names, seconds, reason, message, message_size, number);
	server_commands.role = previous_role;
	return answer;
}

static boolean command_mute(char const *line, long count, char **words)
{
	char name[16];
	long slot, minutes = 0;
	boolean mute = csstrcasecmp(words[0], "unmute") != 0;

	if (count < 2)
	{
		say("%s: give a player's slot (sv_players) or name", words[0]);
		return FALSE;
	}
	if (mute && count >= 3 && !parse_integer(words[2], 1, 24 * 60, &minutes))
	{
		say("mute: give the minutes (1 to %d)", 24 * 60);
		return FALSE;
	}
	slot = find_player(words[0], words[1]);
	if (slot == NONE)
		return FALSE;
	server_status_player_name(slot, name, sizeof(name));
	if (!network_chat_host_mute_named(name, (short)minutes, mute))
	{
		say("%s: %s %s", words[0], name, mute ? "cannot be muted now" : "is not muted");
		return FALSE;
	}
	moderation_event(mute ? "mute" : "unmute", name, NULL, 0);
	return TRUE;
}

static boolean command_map(char const *line, long count, char **words)
{
	char entry[SERVER_ENTRY_TEXT_SIZE];
	char reason[SERVER_REASON_SIZE];

	if (count < 2)
	{
		say("sv_map: give a map, and a gametype: sv_map bloodgulch ctf");
		return FALSE;
	}
	/* ("map gametype", or an entry as the rotation's: map:gametype+rule=1) */
	if (count >= 3 && !strchr(words[1], ':'))
		snprintf(entry, sizeof(entry), "%s:%s", words[1], words[2]);
	else
		snprintf(entry, sizeof(entry), "%s", words[1]);
	if (!server_director_interpose(entry, reason, sizeof(reason)))
	{
		say("sv_map: %s", reason);
		return FALSE;
	}
	say("next: %s (%s)", entry, reason);
	return TRUE;
}

static boolean command_map_next(char const *line, long count, char **words)
{
	if (!server_director_end_game(_server_end_next, TRUE))
	{
		say("%s: no game is being played", words[0]);
		return FALSE;
	}
	return TRUE;
}

static boolean command_end_game(char const *line, long count, char **words)
{
	if (!server_director_end_game(_server_end_command, FALSE))
	{
		say("sv_end_game: no game is being played");
		return FALSE;
	}
	return TRUE;
}

static boolean command_rotation_set(char const *line, long count, char **words)
{
	struct server_json json;
	char const *text = rest_after_words(line, 1);
	boolean set;

	if (!text[0])
	{
		say("sv_rotation_set: give the entries, separated by commas");
		return FALSE;
	}
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	set = server_director_set_rotation(text, TRUE, &json);
	server_json_object_end(&json);
	if (!set)
		say("sv_rotation_set: not changed: %s", server_json_text(&json));
	server_json_free(&json);
	return set;
}

/* the rotation as text, an entry added or taken out */
static boolean command_rotation_add(char const *line, long count, char **words)
{
	static char text[SERVER_ROTATION_MAXIMUM_TEXT + SERVER_ENTRY_TEXT_SIZE + 4];
	char const *entry = rest_after_words(line, 1);
	struct server_entry parsed;
	boolean set;
	long length;

	if (!server_rotation_parse_entry(entry, (int)strlen(entry), &parsed))
	{
		say("%s: give an entry: map:gametype", words[0]);
		return FALSE;
	}
	/* (Halo PC's sv_mapcycle_add <map> <gametype>) */
	if (count >= 3 && !strchr(words[1], ':'))
	{
		char joined[SERVER_ENTRY_TEXT_SIZE];

		snprintf(joined, sizeof(joined), "%s:%s", words[1], words[2]);
		server_rotation_parse_entry(joined, (int)strlen(joined), &parsed);
	}
	server_rotation_text(server_director_rotation(), text, SERVER_ROTATION_MAXIMUM_TEXT);
	length = (long)strlen(text);
	snprintf(text + length, sizeof(text) - (size_t)length, "%s%s", length ? ", " : "", parsed.text);
	set = server_director_set_rotation(text, TRUE, NULL);
	if (!set)
		say("%s: %s cannot be played%s%s", words[0], parsed.text, parsed.rejected[0] ? ": " : "", parsed.rejected);
	return set;
}

static boolean command_rotation_del(char const *line, long count, char **words)
{
	static struct server_rotation rotation;
	static char text[SERVER_ROTATION_MAXIMUM_TEXT + 1];
	long index;

	rotation = *server_director_rotation();
	if (count < 2 || !parse_integer(words[1], 0, rotation.count - 1, &index))
	{
		say("%s: give an entry's number (sv_rotation lists them)", words[0]);
		return FALSE;
	}
	memmove(&rotation.entries[index], &rotation.entries[index + 1],
		sizeof(rotation.entries[0]) * (size_t)(rotation.count - index - 1));
	rotation.count--;
	server_rotation_text(&rotation, text, sizeof(text));
	if (!server_director_set_rotation(text, TRUE, NULL))
	{
		say("%s: the rotation would have nothing playable", words[0]);
		return FALSE;
	}
	return TRUE;
}

static boolean command_rotation_begin(char const *line, long count, char **words)
{
	long index = 0;

	if (count >= 2 && !parse_integer(words[1], 0, SERVER_ROTATION_MAXIMUM_ENTRIES - 1, &index))
	{
		say("%s: give an entry's number (sv_rotation lists them)", words[0]);
		return FALSE;
	}
	if (!server_director_rotation_begin(index))
	{
		say("%s: entry %ld cannot be played", words[0], index);
		return FALSE;
	}
	say("the next game is entry %ld", index);
	return TRUE;
}

static boolean command_set(char const *line, long count, char **words)
{
	char reason[SERVER_REASON_SIZE];
	char value[300];

	if (count < 2)
	{
		say("sv_set: give a setting and its value (sv_settings lists them)");
		return FALSE;
	}
	printable_text(value, sizeof(value), rest_after_words(line, 2));
	if (!server_commands_set_setting(words[1], value, reason, sizeof(reason)))
	{
		say("sv_set: %s", reason);
		return FALSE;
	}
	say("%s = %s", words[1], server_commands_setting_secret(words[1]) ? (value[0] ? "(set)" : "(none)") : value);
	return TRUE;
}

/* sv_name, sv_maxplayers... as sv_set of their settings */
static boolean command_set_alias(char const *line, long count, char **words)
{
	static struct
	{
		char const *command;
		char const *setting;
	} const aliases[] =
	{
		{ "sv_name", "server.name" },
		{ "sv_maxplayers", "server.max_players" },
		{ "sv_password", "server.password" },
		{ "sv_public", "server.public" },
		{ "sv_motd", "server.motd" },
	};
	char reason[SERVER_REASON_SIZE];
	char value[300];
	long index;

	for (index = 0; index < (long)NUMBEROF(aliases) && csstrcasecmp(words[0], aliases[index].command); index++)
	{
	}
	if (index == (long)NUMBEROF(aliases))
		return FALSE;
	printable_text(value, sizeof(value), rest_after_words(line, 1));
	if (!strcmp(aliases[index].setting, "server.name") && strlen(value) > 15)
	{
		say("%s: a name has 15 characters at most", words[0]);
		return FALSE;
	}
	if (!strcmp(aliases[index].setting, "server.max_players"))
	{
		long number;

		if (!parse_integer(value, 2, HALO_PORT_MAXIMUM_NETWORK_PLAYERS - 1, &number))
		{
			say("%s: give a number from 2 to %d", words[0], HALO_PORT_MAXIMUM_NETWORK_PLAYERS - 1);
			return FALSE;
		}
	}
	if (!server_commands_set_setting(aliases[index].setting, value, reason, sizeof(reason)))
	{
		say("%s: %s", words[0], reason);
		return FALSE;
	}
	say("%s = %s", aliases[index].setting, server_commands_setting_secret(aliases[index].setting) ?
		(value[0] ? "(set)" : "(none)") : value);
	return TRUE;
}

/* sv_vpn, sv_vpn_check, sv_vpn_fetch: the VPN policy (server_netlist.c,
network_link.c) */
static boolean command_vpn(char const *line, long count, char **words)
{
	static char text[4096];
	char *cursor = text;

	server_netlist_describe(text, sizeof(text));
	while (cursor && *cursor)
	{
		char *end = strchr(cursor, '\n');

		if (end)
			*end = 0;
		say("%s", cursor);
		cursor = end ? end + 1 : NULL;
	}
	return TRUE;
}

static boolean command_vpn_check(char const *line, long count, char **words)
{
	char kept[32], by[32];
	unsigned long address = 0;

	if (count < 2 || !parse_ipv4(words[1], &address, kept, sizeof(kept)))
	{
		say("sv_vpn_check: give an IPv4 address (a.b.c.d)");
		return FALSE;
	}
	if (server_netlist_listed(address, by, sizeof(by)))
		say("%s is listed (%s): refused as server.vpn_policy says (%s)", kept, by, config_string("server.vpn_policy"));
	else if (by[0])
		say("%s is allowed (%s)", kept, by);
	else
		say("%s is not listed", kept);
	return TRUE;
}

static boolean command_vpn_fetch(char const *line, long count, char **words)
{
	server_netlist_fetch_now();
	say("the master servers' netlists are being fetched (sv_vpn says when)");
	return TRUE;
}

static boolean command_reload(char const *line, long count, char **words)
{
	server_director_reload();
	return TRUE;
}

static boolean command_quit(char const *line, long count, char **words)
{
	server_director_quit(SERVER_EXIT_STOPPED, "quit");
	return TRUE;
}

/* sv_stop and sv_restart: now, or once the game is over or the server
empty (its listings withdrawn and new players refused meanwhile), or the
stop waiting cancelled; a notice said to every machine first */
static boolean command_stop(char const *line, long count, char **words)
{
	static char const *const modes[NUMBER_OF_SERVER_DRAINS] = { "now", "game", "empty", "cancel" };
	boolean restart = !csstrcasecmp(words[0], "sv_restart");
	char notice[NOTICE_SIZE];
	short after = _server_drain_now;
	long notice_words = 1;

	if (count >= 2)
	{
		for (after = 0; after < NUMBER_OF_SERVER_DRAINS && csstrcasecmp(words[1], modes[after]); after++)
		{
		}
		if (after == NUMBER_OF_SERVER_DRAINS)
		{
			say("%s: now, game, empty or cancel, then a notice for the players", words[0]);
			return FALSE;
		}
		notice_words = 2;
	}
	printable_text(notice, sizeof(notice), rest_after_words(line, notice_words));
	if (notice[0])
	{
		send_notice(NONE, notice);
		server_log(_server_log_info, "chat", "server: %s", notice);
	}
	if (after == _server_drain_cancel)
	{
		if (!server_director_drain_cancel(notice))
		{
			say("%s: no stop is waiting", words[0]);
			return FALSE;
		}
		say("the stop is cancelled: the server is listed and open again");
		return TRUE;
	}
	if (after == _server_drain_game)
		say("stopping once this game is over (exit code %d); the listings withdrawn, new players refused",
			restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED);
	else if (after == _server_drain_empty)
		say("stopping once nobody plays (exit code %d); the listings withdrawn, new players refused",
			restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED);
	else
		say("stopping (exit code %d)", restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED);
	server_director_drain(after, restart, notice);
	return TRUE;
}

static struct server_command const server_command_table[] =
{
	{ "help", NULL, _server_role_viewer, "[command]", "the commands, or one's use", command_help },
	{ "sv_status", NULL, _server_role_viewer, "[--json]", "the server and its game", command_status },
	{ "sv_players", NULL, _server_role_viewer, "[--json]", "the players", command_players },
	{ "sv_rotation", "sv_mapcycle", _server_role_viewer, "", "the rotation", command_rotation },
	{ "sv_maps", NULL, _server_role_viewer, "", "the multiplayer maps there are", command_maps },
	{ "sv_gametypes", NULL, _server_role_viewer, "", "the gametypes there are", command_gametypes },
	{ "sv_bans", "sv_banlist", _server_role_admin, "", "bans.txt's bans", command_bans },
	{ "sv_settings", NULL, _server_role_admin, "", "the settings sv_set changes", command_settings },
	{ "sv_version", NULL, _server_role_viewer, "", "the build and its network version", command_version },
	{ "sv_log", NULL, _server_role_viewer, "[lines]", "the log's last lines", command_log },
	{ "sv_masters", NULL, _server_role_viewer, "", "the master servers and their answers", command_masters },
	{ "sv_publisher", NULL, _server_role_viewer, "", "the publisher key's id and public key", command_publisher },
	{ "say", "sv_say", _server_role_moderator, "<text>", "a notice on every machine", command_say },
	{ "tell", NULL, _server_role_moderator, "<player> <text>", "a notice on a player's machine", command_tell },
	{ "kick", "sv_kick", _server_role_moderator, "<player>", "drops a player's machine (it may join again)",
		command_kick },
	{ "ban", "sv_ban", _server_role_moderator, "<player> [30m|2h|7d|forever] [reason]",
		"drops a player's machine and bans its address and hardware id (a moderator's: a day at most)", command_ban },
	{ "unban", "sv_unban", _server_role_admin, "<number|address|hardware id|discord:id>", "takes a ban out of bans.txt",
		command_unban },
	{ "sv_ban_address", NULL, _server_role_admin, "<address|hardware id|discord:id> [30m|2h|7d|forever] [reason]",
		"bans an address, hardware id or verified Discord account, its player here or not", command_ban_address },
	{ "sv_vpn", NULL, _server_role_viewer, "", "the VPN policy, the netlists held and the joins refused",
		command_vpn },
	{ "sv_vpn_check", NULL, _server_role_moderator, "<address>", "whether an IPv4 address is refused as a VPN's",
		command_vpn_check },
	{ "sv_vpn_fetch", NULL, _server_role_admin, "", "fetches the master servers' netlists now", command_vpn_fetch },
	{ "mute", NULL, _server_role_moderator, "<player> [minutes]", "mutes a player's chat and voice for everyone",
		command_mute },
	{ "unmute", NULL, _server_role_moderator, "<player>", "unmutes a player", command_mute },
	{ "sv_map", NULL, _server_role_moderator, "<map> [gametype]", "plays that game now (the rotation goes on after)",
		command_map },
	{ "sv_map_next", "sv_next", _server_role_moderator, "", "ends the game, no scores, and plays the next",
		command_map_next },
	{ "sv_end_game", NULL, _server_role_moderator, "", "ends the game: its scores, then the next", command_end_game },
	{ "sv_rotation_set", NULL, _server_role_admin, "<entries>", "replaces the rotation", command_rotation_set },
	{ "sv_rotation_add", "sv_mapcycle_add", _server_role_admin, "<entry>", "adds an entry to the rotation",
		command_rotation_add },
	{ "sv_rotation_del", "sv_mapcycle_del", _server_role_admin, "<number>", "takes an entry out of the rotation",
		command_rotation_del },
	{ "sv_rotation_begin", "sv_mapcycle_begin", _server_role_admin, "[number]",
		"the rotation's next game is that entry", command_rotation_begin },
	{ "sv_set", NULL, _server_role_admin, "<setting> <value>", "changes a setting (sv_settings lists them)",
		command_set },
	{ "sv_name", NULL, _server_role_admin, "<name>", "the server's name (15 characters)", command_set_alias },
	{ "sv_maxplayers", NULL, _server_role_admin, "<2-127>", "the most players", command_set_alias },
	{ "sv_password", NULL, _server_role_admin, "[password]", "the server browser's password (none without one)",
		command_set_alias },
	{ "sv_public", NULL, _server_role_admin, "<0|1>", "listed in the server browser, or by invite only",
		command_set_alias },
	{ "sv_motd", NULL, _server_role_admin, "<text>", "the notice each machine sees as it joins", command_set_alias },
	{ "sv_reload", NULL, _server_role_admin, "", "reads the server's settings and the rotation again", command_reload },
	{ "sv_stop", NULL, _server_role_owner, "[now|game|empty|cancel] [notice]",
		"stops the server now, once the game is over, or once it is empty", command_stop },
	{ "sv_restart", NULL, _server_role_owner, "[now|game|empty|cancel] [notice]",
		"as sv_stop, with exit code 5 (to be started again)", command_stop },
	{ "quit", "exit", _server_role_owner, "", "stops the server", command_quit },
};

static long server_command_count(
	void)
{
	return (long)NUMBEROF(server_command_table);
}

/* ---------- running a line */

boolean server_command_try(
	char const *expression,
	boolean *result)
{
	char copy[SERVER_COMMAND_LINE_SIZE];
	char *words[MAXIMUM_WORDS];
	long count, index;
	char const *line = expression;

	if (!halo_server_mode())
		return FALSE;
	while (*line == ' ' || *line == '\t')
		line++;
	snprintf(copy, sizeof(copy), "%s", line);
	count = split_words(copy, words, MAXIMUM_WORDS);
	if (!count)
		return FALSE;
	for (index = 0; index < server_command_count(); index++)
	{
		struct server_command const *command = &server_command_table[index];

		if (csstrcasecmp(words[0], command->name) && (!command->alias || csstrcasecmp(words[0], command->alias)))
			continue;
		if (command->role > server_commands.role)
		{
			say("%s: needs the %s role", command->name, server_role_name(command->role));
			*result = FALSE;
			return TRUE;
		}
		*result = command->run(line, count, words);
		return TRUE;
	}
	/* (the game's own commands, scripts and globals, which crash, save and
	change maps: the owner's alone) */
	if (server_commands.role < _server_role_owner)
	{
		say("%s: not a command of the server's, and the game's console needs the owner role (the server's "
			"console)", words[0]);
		*result = FALSE;
		return TRUE;
	}
	return FALSE;
}

char *server_commands_run(
	char const *line,
	int role,
	int origin,
	boolean *succeeded)
{
	int previous_role = server_commands.role;
	int previous_origin = server_commands.origin;
	boolean previous_capturing = server_commands.capturing;
	boolean captured = server_console_capture_begin();

	server_commands.role = role;
	server_commands.origin = origin;
	server_commands.capturing = captured;
	*succeeded = hs_compile_and_evaluate(line);
	server_commands.role = previous_role;
	server_commands.origin = previous_origin;
	server_commands.capturing = previous_capturing;
	return captured ? server_console_capture_end() : NULL;
}

#else

boolean server_command_try(char const *expression, boolean *result)
{
	return FALSE;
}

char *server_commands_run(char const *line, int role, int origin, boolean *succeeded)
{
	*succeeded = FALSE;
	return NULL;
}

int server_commands_role(void)
{
	return 0;
}

#endif

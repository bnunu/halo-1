/*
SERVER_DIRECTOR.C

A dedicated server's director (server_director.h; docs/dedicated_server.md):
what the game lacks to host one game after another with no player of its
own. Once the main menu is up, it hosts as the pregame screen's fast setup
does (as network_test.c's host does), with no player, and plays the
rotation (server.rotation, server_rotation.c) a game at a time: the entry's
map and gametype set in the lobby, the lobby's countdown held until
server.min_players are in and then started (server.lobby_seconds), the
game played (joins in progress taken, as any host takes them), ended when
nobody has been in it for server.empty_seconds or no score has changed for
server.idle_minutes, its scores shown server.postgame_seconds, and back to
the lobby with the next entry. It also takes the owner's commands (the
standard input, the init file: server_console.c), the web's
(server_http.c), the signals, and stops cleanly: the game ended, every
machine told, the listings withdrawn.

Called from the main loop every frame (main.c).
*/

#include "cseries.h"
#include "memory/data.h"
#include "main/main.h"
#include "bink/bink_playback.h"
#include "interface/player_ui.h"
#include "interface/ui_widget.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "networking/network_server_manager.h"
#include "networking/network_server_manager_internal.h"
#include "networking/network_client_manager.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "text/unicode.h"
#include "game_modes.h"
#include "network_distributed.h"
#include "network_coop.h"
#include "network_map_vote.h"
#include "server_director.h"
#include "mcc_server.h"
#include "mcc_ui_network.h"
#include "server_gametypes.h"

#include "../src/p2p.h" /* port: port/linux/src/p2p.c, p2p_lobby.c */
#include "../src/server_console.h" /* port: port/linux/src/server_console.c */
#include "../src/server_events.h" /* port: port/linux/src/server_events.c */
#include "../src/server_http.h" /* port: port/linux/src/server_http.c */
#include "../src/server_json.h" /* port: port/linux/src/server_json.c */
#include "../src/server_log.h" /* port: port/linux/src/server_log.c */
#include "../src/server_master.h" /* port: port/linux/src/server_master.c */
#include "../src/server_netlist.h" /* port: port/linux/src/server_netlist.c */
#include "../src/posix_server.h" /* port: port/linux/src/posix_server.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the platform layer's (port/linux/src/port_config.c) */
const char *config_string(char const *name);
int config_boolean(char const *name);
long config_integer(char const *name);
double config_real(char const *name);
int config_write(char const *name, char const *value);
int config_reload(char const *prefix);
void platform_log(char const *format, ...);
unsigned long system_milliseconds(void);
extern boolean debug_no_drawing;
/* (menu_functions.c's) */
extern boolean ui_widget_port_cooperative_level_choose(char const *map_name, short difficulty);
extern boolean ui_widget_port_mcc_cooperative_level_choose(char const *map_name, short difficulty);
extern void ui_widget_port_cooperative_settings_apply(void);
/* (port/linux/src/updater.c's) */
int updater_server_check(void);
long updater_server_available(void);
long updater_server_available_network_version(void);
int updater_server_install_start(void);
int updater_server_install_poll(char *error, int size);

#if HALO_SERVER_BUILD

enum
{
	/* the network game server's states (network_server_manager.c's) */
	SERVER_PREGAME = 0,
	SERVER_INGAME = 1,
	SERVER_POSTGAME = 2,
	/* seconds the main menu settles before hosting, and between tries */
	MENU_SETTLE_SECONDS = 2,
	HOSTING_RETRY_SECONDS = 5,
	/* seconds the lobby waits before its entry is set (the pregame screen
	sets its own first) */
	LOBBY_SETUP_SECONDS = 1,
	/* seconds the scores show when nobody played the game */
	EMPTY_POSTGAME_SECONDS = 1,
	/* seconds the machines are given to hear the server go */
	QUIT_SECONDS = 1,
	/* the deadline of a shutdown that hangs (a map loading) */
	QUIT_DEADLINE_SECONDS = 5,
	/* seconds between the status file's writes, and the health lines */
	STATUS_FILE_SECONDS = 2,
	HEALTH_SECONDS = 60,
	/* seconds a joined player waits for the message of the day (the game
	loading on its machine) */
	MOTD_DELAY_SECONDS = 5,
	/* server.auto_update: the seconds the server must have been empty before
	a build of the same network version is started; a new network version's
	notice in the lobby before the server stops; the first look's latest
	second after the start (spread, so that a machine's servers do not all
	ask GitHub at once) */
	UPDATE_EMPTY_SECONDS = 10 * 60,
	UPDATE_LOBBY_NOTICE_SECONDS = 30,
	UPDATE_FIRST_CHECK_SECONDS = 120,
	MAXIMUM_SLOTS = HALO_PORT_MAXIMUM_NETWORK_PLAYERS,
	MAXIMUM_MACHINES = HALO_PORT_MAXIMUM_NETWORK_MACHINES,
	/* the game's name's characters, with its end (network_game's) */
	NETWORK_GAME_NAME_LENGTH = 16,
	/* seconds a machine's players leaving are said to be kicked or banned
	after the command that dropped it */
	LEAVE_REASON_SECONDS = 10,
};

static char const *const server_end_names[NUMBER_OF_SERVER_ENDS] =
	{ "score", "time", "idle", "empty", "command", "next", "update", "vote" };

static struct
{
	boolean started;
	short state;
	real state_seconds;
	real menu_seconds;
	/* the rotation, and the entry of it played last (the next is the one
	after it), and whether one has been (or sv_rotation_begin chose the
	next): if not, the first entry is next whatever the rotation becomes */
	struct server_rotation rotation;
	long index;
	boolean index_anchored;
	/* the entry sv_map put before the rotation's next */
	struct server_entry interposed;
	boolean has_interposed;
	/* the entry this lobby or game plays: its index in the rotation (NONE:
	none, or the interposed one), and whether it is the interposed one
	(another sv_map since puts a new one in its place) */
	struct server_entry current;
	boolean current_set;
	long current_index;
	boolean current_is_interposed;
	boolean entry_applied;
	real lobby_seconds;
	real map_checked_seconds;
	/* the game: its seconds, the seconds it has been empty and without a
	score, why it ended, and whether its scores are skipped */
	real game_seconds;
	real empty_seconds;
	real idle_seconds;
	long score_sum;
	short end_reason;
	boolean end_asked;
	boolean skip_postgame;
	/* the game's end said (game_ended) */
	boolean game_over_said;
	boolean back_to_lobby;
	real postgame_wait;
	long games_hosted;
	/* stopping */
	boolean quitting;
	int exit_code;
	real quit_seconds;
	/* a stop waiting for the game's end or for nobody to play (sv_stop,
	sv_restart: _server_drain_game or _empty), since when (seconds since
	1970), and whether it restarts (exit code 5) */
	boolean drain_active;
	short drain_after;
	boolean drain_restart;
	unsigned long drain_since;
	/* the players seen, for their joins and leaves (each slot's name and
	machine), and when each was seen join (the message of the day) */
	char seen_names[MAXIMUM_SLOTS][16];
	long seen_machines[MAXIMUM_SLOTS];
	/* ... and each one's address and hardware id as it joined (the events'
	for the readers who see them) */
	char seen_addresses[MAXIMUM_SLOTS][16];
	char seen_hardware_ids[MAXIMUM_SLOTS][40];
	/* (and its Discord account as it joined: a verified one's, else what it
	said of itself) */
	char seen_discord_ids[MAXIMUM_SLOTS][32];
	char seen_discord_names[MAXIMUM_SLOTS][48];
	boolean seen_discord_verified[MAXIMUM_SLOTS];
	/* why each machine's players leave, a moment after a command dropped it
	("kicked", "banned"; NULL: they left), and the seconds since */
	char const *leave_reasons[MAXIMUM_MACHINES];
	real leave_reason_seconds[MAXIMUM_MACHINES];
	long long seen_joined[MAXIMUM_SLOTS];
	real motd_waits[MAXIMUM_SLOTS];
	boolean motd_sent[MAXIMUM_SLOTS];
	char invite[P2P_LISTING_INVITE_SIZE];
	/* the ready line, once the lobby is open (and listed), 10 s on at most */
	boolean ready_said;
	real ready_wait;
	/* the status file's and the health line's clocks */
	real status_file_seconds;
	real health_seconds;
	real score_event_seconds;
	long last_countdown_seconds;
	/* the master servers' state, as last published */
	char master_state[1024];
	/* server.auto_update: the seconds to the next look, the newer build
	last told of, and the seconds the server has been empty */
	real update_check_seconds;
	long update_told;
	real update_empty_seconds;
	/* the install of it under way (on a thread of its own: the server goes
	on), and the build it installs; done, the server stops once empty, or,
	for a new network version, once the game being played is over */
	boolean update_installing;
	boolean update_installed;
	long update_installing_build;
	/* the build installed plays another network version; whether a game was
	under way when it was installed, the seconds since, those of the
	lobby's notice, and whether its end was asked */
	boolean update_new_network;
	boolean update_after_game;
	real update_installed_seconds;
	real update_lobby_seconds;
	boolean update_end_asked;
	boolean update_checked_once;
	/* this lobby's map vote: over (or not held), and the rotation's entries
	it offers (server.map_vote) */
	boolean map_vote_done;
	short map_vote_count;
	long map_vote_entries[NETWORK_MAP_VOTE_MAXIMUM_CHOICES];
} server;

/* ---------- the server's game */

static struct network_game *server_game(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();

	return network_server ? network_game_server_get_game(network_server) : NULL;
}

static short server_network_state(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();

	return network_server ? (short)network_game_server_get_state(network_server, NULL) : NONE;
}

static long server_player_count(
	void)
{
	struct network_game *game = server_game();
	long count = 0;
	long index;

	for (index = 0; game && index < MAXIMUM_SLOTS; index++)
	{
		if (network_player_is_valid(&game->players[index]))
			count++;
	}
	return count;
}

/* the players in the game being played (not those who quit it) */
static long server_players_in_game(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long count = 0;

	if (!player_data || !game_engine_running())
		return server_player_count();
	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		if (!player->quit_out_of_game)
			count++;
	}
	return count;
}

static void server_set_state(
	short state)
{
	if (server.state == state)
		return;
	server.state = state;
	server.state_seconds = 0.0f;
}

short server_director_state(
	void)
{
	return server.state;
}

struct server_rotation const *server_director_rotation(
	void)
{
	return &server.rotation;
}

long server_director_index(
	void)
{
	return server.current_set && !server.current_is_interposed ? server.current_index : NONE;
}

struct server_entry const *server_director_current(
	void)
{
	return server.current_set ? &server.current : NULL;
}

long server_director_games_hosted(
	void)
{
	return server.games_hosted;
}

real server_director_game_seconds(
	void)
{
	return server.state == _server_state_playing || server.state == _server_state_postgame ? server.game_seconds : 0.0f;
}

/* ---------- events */

/* a variant's name, as the lobby shows it, in printable ASCII */
static void server_variant_name(
	struct game_variant const *variant,
	char *name,
	long size)
{
	long index;

	for (index = 0; index < (long)NUMBEROF(variant->human_readable_game_description) - 1 &&
		variant->human_readable_game_description[index] && index < size - 1; index++)
	{
		name[index] = player_name_character_ascii(variant->human_readable_game_description[index]);
	}
	name[index] = 0;
}

/* an event of the game: its name and the players (as viewers see them) */
static void server_game_event(
	char const *name,
	char const *reason)
{
	struct server_json json;
	struct network_game *game = server_game();
	boolean cooperative = server.current_set && !strcmp(server.current.gametype, "coop");
	long red, blue;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", name);
	if (server.current_set)
	{
		server_json_string(&json, "map", server.current.map_name);
		server_json_string(&json, "gametype", server.current.gametype);
	}
	if (game)
	{
		char variant[32];

		server_variant_name(&game->variant, variant, sizeof(variant));
		server_json_string(&json, "variant", cooperative ? "Co-op" : variant);
		server_json_boolean(&json, "teams", !cooperative && game->variant.universal_variant.teams);
	}
	if (reason)
	{
		server_json_string(&json, "reason", reason);
		server_json_integer(&json, "duration", (long long)server.game_seconds);
	}
	if (server_status_team_scores(&red, &blue))
	{
		server_json_array_begin(&json, "team_scores");
		server_json_integer(&json, NULL, red);
		server_json_integer(&json, NULL, blue);
		server_json_array_end(&json);
	}
	server_json_array_begin(&json, "players");
	{
		long slot;

		for (slot = 0; game && slot < MAXIMUM_SLOTS; slot++)
		{
			if (network_player_is_valid(&game->players[slot]))
				server_status_player(&json, slot, _server_role_viewer);
		}
	}
	server_json_array_end(&json);
	server_json_object_end(&json);
	server_event_add("game", server_json_text(&json));
	server_json_free(&json);
}

static void server_simple_event(
	char const *group,
	char const *name,
	char const *key,
	long long value)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", name);
	if (key)
		server_json_integer(&json, key, value);
	server_json_object_end(&json);
	server_event_add(group, server_json_text(&json));
	server_json_free(&json);
}

/* ---------- the rotation */

/* the rotation from the settings, each entry checked; a fallback's if
nothing can be played (server.rotation_fallback) */
static void server_read_rotation(
	struct server_rotation *rotation,
	char const *text)
{
	long index;

	server_rotation_parse(text, rotation);
	for (index = 0; index < rotation->count; index++)
	{
		server_rotation_resolve(&rotation->entries[index]);
		server_gametype_check_entry(&rotation->entries[index]);
	}
	if (!server_rotation_playable(rotation) && config_boolean("server.rotation_fallback") &&
		rotation->count < SERVER_ROTATION_MAXIMUM_ENTRIES)
	{
		struct server_entry *entry = &rotation->entries[rotation->count];

		if (server_rotation_parse_entry("bloodgulch:slayer", 17, entry))
		{
			server_rotation_resolve(entry);
			server_gametype_check_entry(entry);
			rotation->count++;
		}
	}
}

/* the entry after the one at index that can be played (index itself
last), NONE if none can */
static long server_rotation_after(
	long index)
{
	long step;

	for (step = 1; step <= server.rotation.count; step++)
	{
		long candidate = (index + step) % server.rotation.count;

		if (!server.rotation.entries[candidate].rejected[0])
			return candidate;
	}
	return NONE;
}

/* the level the network game is first set up on: the entry being played,
else the rotation's next. A co-op entry's own level too (its campaign map,
which the rotation found present), which server_apply_entry then makes the
co-op game: else the game's stock default, carousel, which a server's maps
need not have (or not readable as this build's), and a lobby that fails to
read its map is fatal: a rotation of co-op entries alone never started */
char const *server_director_first_level(
	void)
{
	struct server_entry const *entry;

	if (server.current_set)
		return server.current.level;
	entry = server_director_next();
	return entry ? entry->level : NULL;
}

struct server_entry const *server_director_next(
	void)
{
	long next;

	if (server.has_interposed && !server.current_is_interposed)
		return &server.interposed;
	if (!server.rotation.count)
		return NULL;
	next = server_rotation_after(server.index);
	return next != NONE ? &server.rotation.entries[next] : NULL;
}

/* ---------- the settings */

/* the game's name (15 characters of printable ASCII) and its most players,
for this game and the next */
static void server_apply_game_settings(
	void)
{
	char const *name = config_string("server.name");
	wchar_t wide[NETWORK_GAME_NAME_LENGTH];
	long maximum = config_integer("server.max_players");
	long index, length = 0;

	for (index = 0; name[index] && length < NETWORK_GAME_NAME_LENGTH - 1; index++)
	{
		if ((unsigned char)name[index] >= 0x20 && (unsigned char)name[index] < 0x7F)
			wide[length++] = (wchar_t)(unsigned char)name[index];
	}
	wide[length] = 0;
	/* (server.name's default: 13 characters, as the game's name holds) */
	if (!length)
		ustrncpy(wide, L"OpenCE Server", NETWORK_GAME_NAME_LENGTH - 1);
	network_game_server_port_set_settings(wide, PIN(maximum, 2, HALO_PORT_MAXIMUM_NETWORK_PLAYERS - 1));
	/* (a co-op game's clients told the host has no player: its loading zones
	ask for most of them, network_coop_host_dedicated) */
	network_game_server_port_set_cooperative_dedicated(TRUE);
}

/* the listing: internet play (network.online), public or by invite, and
its password */
static void server_apply_listing(
	void)
{
	char const *password = config_string("server.password");

	p2p_set_hosting_allowed(config_boolean("network.online"));
	p2p_set_hosting_password(password[0] ? password : NULL);
	/* (withdrawn while a stop waits, whatever server.public says) */
	p2p_set_hosting_public(config_boolean("server.public") && !server.drain_active);
}

void server_director_apply_settings(
	void)
{
	server_apply_game_settings();
	if (global_network_game_server_get())
		server_apply_listing();
}

/* ---------- setting a game up */

/* the riding rule an entry gives (NONE: it gives none) */
static short server_entry_riding_rule(
	struct server_entry const *entry)
{
	long index;

	for (index = 0; index < entry->rule_count; index++)
	{
		if (!strcmp(entry->rules[index].key, "vehicle_riding"))
			return entry->rules[index].value ? TRUE : FALSE;
	}
	return NONE;
}

/* the lobby's map and gametype set to the entry's (the rotation's at index;
NONE: the interposed one): TRUE, else FALSE (said) */
static boolean server_apply_entry(
	struct server_entry const *entry,
	long index)
{
	struct network_game_server *network_server = global_network_game_server_get();
	struct game_variant variant;
	struct game_variant_options options;
	char reason[SERVER_REASON_SIZE];
	char variant_name[32] = "Co-op";

	if (!network_server)
		return FALSE;
	csmemset(&variant, 0, sizeof(variant));
	if (!strcmp(entry->gametype, "coop"))
	{
		static char const *const difficulties[] = { "easy", "normal", "hard", "impossible" };
		char const *setting = config_string("server.coop_difficulty");
		short difficulty;

		for (difficulty = 0; difficulty < (short)NUMBEROF(difficulties) && strcmp(setting, difficulties[difficulty]);
			difficulty++)
		{
		}
		if (difficulty == (short)NUMBEROF(difficulties))
			difficulty = 1;
		if (!(entry->map_kind == _server_map_mcc ?
			ui_widget_port_mcc_cooperative_level_choose(entry->level, difficulty) :
			ui_widget_port_cooperative_level_choose(entry->level, difficulty)))
		{
			server_log(_server_log_warning, "lobby", "%s cannot be played as co-op", entry->level);
			return FALSE;
		}
		/* (and as the player's Server Setup gives a co-op game: its friendly
		fire, player collisions and ally friendly fire from config.toml,
		which a dedicated server had never applied; and server.max_players
		again, which choosing the level holds to the co-op default of 16) */
		ui_widget_port_cooperative_settings_apply();
		server_apply_game_settings();
	}
	else
	{
		if (!server_gametype_build(entry->gametype, entry->rules, entry->rule_count, &variant, &options, reason,
			sizeof(reason)))
		{
			server_log(_server_log_warning, "lobby", "%s cannot be played: %s", entry->text, reason);
			return FALSE;
		}
		main_set_multiplayer_map_name(entry->level);
		network_game_server_change_map_name(network_server, entry->level);
		player_ui_set_game_variant(&variant);
		player_ui_set_game_variant_options(&options);
		network_game_server_change_game_variant(network_server, &variant);
		server_variant_name(&variant, variant_name, sizeof(variant_name));
	}
	/* (every applied entry sets it, NONE without a riding rule: a co-op entry's own, which no later entry inherits) */
	network_game_port_vehicle_riding_entry(server_entry_riding_rule(entry));
	server.current = *entry;
	server.current_set = TRUE;
	server.current_index = index;
	server.current_is_interposed = index == NONE;
	server_log(_server_log_info, "lobby", "next game: %s on %s", entry->gametype, entry->map_name);
	{
		struct server_json json;

		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_string(&json, "event", "lobby_opened");
		server_json_string(&json, "map", entry->map_name);
		server_json_string(&json, "gametype", entry->gametype);
		server_json_string(&json, "variant", variant_name);
		server_json_boolean(&json, "teams", variant.universal_variant.teams);
		server_json_integer(&json, "index", index);
		server_json_object_end(&json);
		server_event_add("game", server_json_text(&json));
		server_json_free(&json);
	}
	return TRUE;
}

/* the entry for this lobby: an interposed one (index NONE), else the
rotation's after the one played last (its index) */
static struct server_entry const *server_lobby_entry(
	long *index)
{
	*index = NONE;
	if (server.has_interposed)
		return &server.interposed;
	if (!server.rotation.count)
		return NULL;
	*index = server_rotation_after(server.index);
	return *index != NONE ? &server.rotation.entries[*index] : NULL;
}

/* a game over: an interposed one was played (the rotation goes on after
the entry played before it) */
static void server_advance(
	void)
{
	if (server.current_is_interposed)
		server.has_interposed = FALSE;
	server.current_is_interposed = FALSE;
}

/* ---------- ending a game */

static boolean server_cooperative(
	void)
{
	return server.current_set && !strcmp(server.current.gametype, "coop");
}

/* the game over, said: how it ended, the scores' seconds, and the next game
on every machine */
static void server_game_over(
	void)
{
	struct server_json json;
	long postgame_seconds = server.skip_postgame ? 0 : PIN(config_integer("server.postgame_seconds"), 0, 300);

	server.game_over_said = TRUE;
	server_log(_server_log_info, "game", "ended (%s) after %ld s", server_end_names[server.end_reason],
		(long)server.game_seconds);
	server_game_event("game_ended", server_end_names[server.end_reason]);
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", "postgame");
	server_json_integer(&json, "seconds", postgame_seconds);
	server_json_object_end(&json);
	server_event_add("game", server_json_text(&json));
	server_json_free(&json);
	if (!server.skip_postgame && server.current_set && server_director_next())
	{
		char notice[128];
		struct server_entry const *next = server_director_next();

		snprintf(notice, sizeof(notice), "Next game in %ld s: %s, %s", postgame_seconds, next->map_name, next->gametype);
		distributed_send_notice(notice);
	}
}

/* a co-op level ended for everyone, as a won one is
(network_game_server_port_cooperative_won): the server's postgame, and at
once the lobby (co-op has no scores to show); the lobby tried again from
the postgame if its machines are still joining (server_postgame) */
static void server_end_cooperative(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();

	server.skip_postgame = TRUE;
	server_game_over();
	server_set_state(_server_state_postgame);
	if (network_server && server_network_state() == SERVER_INGAME)
	{
		network_game_server_switch_to_postgame(network_server);
		if (network_game_server_reset_to_pregame(network_server))
			server.back_to_lobby = TRUE;
	}
}

boolean server_director_end_game(
	short reason,
	boolean skip_postgame)
{
	boolean cooperative = server_cooperative();

	/* (a co-op level is played once loaded: it has no game engine) */
	if (server.state != _server_state_playing ||
		(cooperative ? !game_in_progress() : !game_engine_running() || !game_engine_can_score()))
	{
		return FALSE;
	}
	if (!server.end_asked)
	{
		server.end_asked = TRUE;
		server.end_reason = reason;
		server.skip_postgame = skip_postgame;
		server_log(_server_log_info, "game", "the game ends (%s)", server_end_names[reason]);
		if (cooperative)
			server_end_cooperative();
		else
			game_engine_end_game();
	}
	return TRUE;
}

boolean server_director_playing(
	void)
{
	boolean cooperative = server_cooperative();

	return server.state == _server_state_playing && !server.end_asked &&
		(cooperative ? game_in_progress() : game_engine_running() && game_engine_can_score());
}

boolean server_director_interpose(
	char const *text,
	char *reason,
	long reason_size)
{
	struct server_entry entry;

	if (!server_rotation_parse_entry(text, (int)strlen(text), &entry))
	{
		snprintf(reason, (size_t)reason_size, "give a map, and a gametype: sv_map bloodgulch ctf");
		return FALSE;
	}
	server_rotation_resolve(&entry);
	server_gametype_check_entry(&entry);
	if (entry.rejected[0])
	{
		snprintf(reason, (size_t)reason_size, "%s", entry.rejected);
		return FALSE;
	}
	server.interposed = entry;
	server.has_interposed = TRUE;
	/* (the owner chose: no map vote) */
	if (network_map_vote_running())
		network_map_vote_cancel();
	server.map_vote_done = TRUE;
	/* (a new one: not the one being played, if that was interposed too) */
	server.current_is_interposed = FALSE;
	/* (now: in the lobby the entry is set again, a game ends at once; once
	the lobby's start was sent, its machines load the game, and this one is
	the next) */
	if (server.state == _server_state_lobby &&
		!network_game_server_port_start_sent(global_network_game_server_get()))
	{
		/* (the rotation's entry it takes the place of played after it) */
		if (server.entry_applied && server.current_set && server.current_index != NONE && server.rotation.count)
		{
			server.index = (server.current_index + server.rotation.count - 1) % server.rotation.count;
			server.index_anchored = TRUE;
		}
		server.entry_applied = FALSE;
		snprintf(reason, (size_t)reason_size, "now");
	}
	else if (server.state == _server_state_playing && server_director_end_game(_server_end_next, TRUE))
	{
		snprintf(reason, (size_t)reason_size, "now: this game ends");
	}
	else
	{
		snprintf(reason, (size_t)reason_size, "once this game is over");
	}
	return TRUE;
}

/* the entry of the rotation with the text that is nearest index (an entry
may be in it more than once), NONE if none has it */
static long server_rotation_find(
	struct server_rotation const *rotation,
	char const *text,
	long index)
{
	long found = NONE, candidate;

	for (candidate = 0; candidate < rotation->count; candidate++)
	{
		if (!strcmp(rotation->entries[candidate].text, text) &&
			(found == NONE || labs(candidate - index) < labs(found - index)))
		{
			found = candidate;
		}
	}
	return found;
}

/* a rotation in place of the one played: the game being played goes on,
and the next is what comes after the entry played last in the new one (by
its text, else by the text of the one that came after it, else by its
place); the first entry if none has been played yet */
static void server_adopt_rotation(
	struct server_rotation const *rotation)
{
	long anchor = rotation->count - 1;

	/* (a map vote's choices were the old rotation's: a new vote) */
	if (network_map_vote_running())
	{
		network_map_vote_cancel();
		server.map_vote_done = FALSE;
	}

	if (server.index_anchored && server.index >= 0 && server.index < server.rotation.count && rotation->count)
	{
		long after = server_rotation_after(server.index);

		anchor = server_rotation_find(rotation, server.rotation.entries[server.index].text, server.index);
		/* (it was taken out: the one that came after it now comes next) */
		if (anchor == NONE && after != NONE && after != server.index)
		{
			anchor = server_rotation_find(rotation, server.rotation.entries[after].text, server.index);
			if (anchor != NONE)
				anchor = (anchor + rotation->count - 1) % rotation->count;
		}
		if (anchor == NONE)
			anchor = (MIN(server.index, rotation->count) + rotation->count - 1) % rotation->count;
	}
	if (server.current_set && !server.current_is_interposed && server.current_index != NONE)
		server.current_index = server_rotation_find(rotation, server.current.text, server.current_index);
	server.rotation = *rotation;
	server.index = anchor;
}

boolean server_director_set_rotation(
	char const *text,
	boolean write,
	struct server_json *rejected)
{
	static struct server_rotation rotation;
	long index;
	boolean playable;

	server_read_rotation(&rotation, text);
	playable = server_rotation_playable(&rotation) == rotation.count && rotation.count > 0 && !rotation.truncated;
	if (rejected)
	{
		server_json_array_begin(rejected, "rejected");
		for (index = 0; index < rotation.count; index++)
		{
			if (rotation.entries[index].rejected[0])
			{
				server_json_object_begin(rejected, NULL);
				server_json_integer(rejected, "index", index);
				server_json_string(rejected, "entry", rotation.entries[index].text);
				server_json_string(rejected, "reason", rotation.entries[index].rejected);
				server_json_object_end(rejected);
			}
		}
		server_json_array_end(rejected);
	}
	if (!playable)
		return FALSE;
	server_adopt_rotation(&rotation);
	if (write)
	{
		char joined[SERVER_ROTATION_MAXIMUM_TEXT + 1];

		server_rotation_text(&server.rotation, joined, sizeof(joined));
		if (!config_write("server.rotation", joined))
			server_log(_server_log_warning, "server", "config.toml cannot be written: the rotation holds for this run");
	}
	server_log(_server_log_info, "server", "the rotation now has %d entries", rotation.count);
	return TRUE;
}

/* (the web's: server_http.c) */
int server_director_set_rotation_text(
	char const *text,
	struct server_json *rejected)
{
	return server_director_set_rotation(text, TRUE, rejected);
}

boolean server_director_rotation_begin(
	long index)
{
	if (index < 0 || index >= server.rotation.count || server.rotation.entries[index].rejected[0])
		return FALSE;
	/* (the entry is the next: the one before it is "played") */
	server.index = (index + server.rotation.count - 1) % server.rotation.count;
	server.index_anchored = TRUE;
	server.has_interposed = FALSE;
	return TRUE;
}

void server_director_reload(
	void)
{
	static struct server_rotation rotation;
	char before[SERVER_ROTATION_MAXIMUM_TEXT + 1];
	char after[SERVER_ROTATION_MAXIMUM_TEXT + 1];
	struct server_json json;
	long index;
	boolean rejections_changed;

	server_rotation_text(&server.rotation, before, sizeof(before));
	if (!config_reload("server."))
	{
		server_log(_server_log_warning, "server", "config.toml cannot be read again: nothing changed");
		return;
	}
	/* MCC discovery waits for pregame; legacy rotation resolution is unchanged. */
	mcc_server_request_rescan();
	/* (server.vpn_list's file, vpn_allow.txt and vpn_block.txt too) */
	server_netlist_reload();
	server_read_rotation(&rotation, config_string("server.rotation"));
	server_rotation_text(&rotation, after, sizeof(after));
	/* (the same entries, checked again: a map or gametype file added or
	taken out since) */
	rejections_changed = rotation.count != server.rotation.count;
	for (index = 0; !rejections_changed && index < rotation.count; index++)
		rejections_changed = strcmp(rotation.entries[index].rejected, server.rotation.entries[index].rejected) != 0;
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", "reloaded");
	server_json_boolean(&json, "rotation_changed", strcmp(before, after) != 0);
	server_json_array_begin(&json, "rejected");
	for (index = 0; index < rotation.count; index++)
	{
		if (rotation.entries[index].rejected[0])
		{
			server_json_object_begin(&json, NULL);
			server_json_string(&json, "entry", rotation.entries[index].text);
			server_json_string(&json, "reason", rotation.entries[index].rejected);
			server_json_object_end(&json);
			server_log(_server_log_warning, "server", "rotation entry %ld (%s) left out: %s", index + 1,
				rotation.entries[index].text, rotation.entries[index].rejected);
		}
	}
	server_json_array_end(&json);
	server_json_object_end(&json);
	server_event_add("server", server_json_text(&json));
	server_json_free(&json);
	if (strcmp(before, after) || rejections_changed)
	{
		if (server_rotation_playable(&rotation))
		{
			server_adopt_rotation(&rotation);
			server_log(_server_log_info, "server", "reloaded: the rotation goes on after the entry played last");
		}
		else
		{
			server_log(_server_log_warning, "server", "reloaded: the new rotation has nothing playable, the old one stays");
		}
	}
	else
	{
		server_log(_server_log_info, "server", "reloaded");
	}
	server_director_apply_settings();
}

/* ---------- stopping */

static void server_withdraw(
	void)
{
	/* (the brokers' tombstone and the masters', once) */
	static boolean withdrawn;

	if (withdrawn)
		return;
	withdrawn = TRUE;
	p2p_set_hosting_public(FALSE);
	server_master_stop();
}

void server_director_quit(
	int exit_code,
	char const *why)
{
	struct network_game_server *network_server = global_network_game_server_get();

	if (server.quitting)
		return;
	server.quitting = TRUE;
	server.exit_code = exit_code;
	server_set_state(_server_state_stopping);
	server_log(_server_log_info, "server", "stopping (%s)", why);
	{
		struct server_json json;

		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_string(&json, "event", "shutdown");
		server_json_string(&json, "reason", why);
		server_json_integer(&json, "code", exit_code);
		server_json_object_end(&json);
		server_event_add("server", server_json_text(&json));
		server_json_free(&json);
	}
	/* (a shutdown that hangs ends the process anyway) */
	posix_server_quit_deadline(QUIT_DEADLINE_SECONDS, exit_code);
	if (network_server)
	{
		/* the game's end shown, then every machine told the server goes */
		if (server_network_state() == SERVER_INGAME)
			network_game_server_switch_to_postgame(network_server);
		network_game_server_graceful_shutdown(network_server);
	}
	server_withdraw();
}

void server_fatal(
	char const *text)
{
	if (!halo_server_mode())
		return;
	server_withdraw();
	server_mode_fatal(text);
}

/* ---------- a stop that waits (a drain) */

static char const *const server_drain_names[NUMBER_OF_SERVER_DRAINS] = { "now", "game", "empty", "cancel" };

static void server_drain_event(
	short after,
	boolean restart,
	char const *notice)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", "drain");
	server_json_string(&json, "after", server_drain_names[after]);
	server_json_boolean(&json, "restart", restart);
	server_json_string(&json, "notice", notice ? notice : "");
	server_json_object_end(&json);
	server_event_add("server", server_json_text(&json));
	server_json_free(&json);
}

void server_director_drain(
	short after,
	boolean restart,
	char const *notice)
{
	if (server.quitting || after < _server_drain_now || after > _server_drain_empty)
		return;
	server_drain_event(after, restart, notice);
	if (after == _server_drain_now)
	{
		server_director_quit(restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED, restart ? "sv_restart" : "sv_stop");
		return;
	}
	if (!server.drain_active)
		server.drain_since = posix_server_unix_seconds();
	server.drain_active = TRUE;
	server.drain_after = after;
	server.drain_restart = restart;
	/* (the listings withdrawn as sv_public 0 withdraws them, the setting
	kept: new players come by the invite or the LAN alone, and the game
	refuses them, server_director_refuses_joins) */
	p2p_set_hosting_public(FALSE);
	server_log(_server_log_info, "server", "stopping (exit code %d) once %s: the listings withdrawn, new players refused",
		restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED, after == _server_drain_game ? "the game is over" :
		"nobody plays");
}

boolean server_director_drain_cancel(
	char const *notice)
{
	if (!server.drain_active || server.quitting)
		return FALSE;
	server.drain_active = FALSE;
	server_drain_event(_server_drain_cancel, server.drain_restart, notice);
	p2p_set_hosting_public(config_boolean("server.public"));
	server_log(_server_log_info, "server", "the stop is cancelled: listed (as server.public says) and open again");
	return TRUE;
}

boolean server_director_refuses_joins(
	void)
{
	return server.drain_active || server.quitting;
}

void server_director_drain_json(
	struct server_json *json)
{
	if (!server.drain_active)
	{
		server_json_null(json, "drain");
		return;
	}
	server_json_object_begin(json, "drain");
	server_json_string(json, "after", server_drain_names[server.drain_after]);
	server_json_boolean(json, "restart", server.drain_restart);
	server_json_integer(json, "since", server.drain_since);
	server_json_object_end(json);
}

/* a drain's stop, when its moment comes: for "empty", nobody on the server;
for "game", no game being played or about to be (not hosting yet, or a
lobby whose start was not sent, with nobody in it or no countdown running:
a game over goes back to such a lobby) */
static void server_drain_update(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();
	boolean stop;

	if (!server.drain_active || server.quitting)
		return;
	if (server.drain_after == _server_drain_empty)
	{
		stop = server_player_count() == 0;
	}
	else if (!network_server || server.state == _server_state_waiting || server.state == _server_state_hosting)
	{
		stop = TRUE;
	}
	else
	{
		long countdown = network_game_server_port_countdown_milliseconds(network_server);

		stop = server.state == _server_state_lobby && !network_game_server_port_start_sent(network_server) &&
			(server_player_count() == 0 || countdown == NONE || network_game_server_port_countdown_paused(network_server));
	}
	if (stop)
	{
		server.drain_active = FALSE;
		server_director_quit(server.drain_restart ? SERVER_EXIT_RESTART : SERVER_EXIT_STOPPED, "drained");
	}
}

/* ---------- stopping by itself, and updates */

/* debug.exit_after (the tests'): the server stopped as quit stops it once
it has run that long */
static void server_exit_after(
	void)
{
	double seconds = config_real("debug.exit_after");

	if (seconds > 0.0 && server_mode_uptime() >= seconds)
		server_director_quit(SERVER_EXIT_STOPPED, "debug.exit_after");
}

/* whether nobody plays on the server, nor is about to */
static boolean server_empty(
	void)
{
	return server_player_count() == 0 && server.state != _server_state_playing && server.state != _server_state_loading;
}

/* the seconds between looks for a new release (server.update_check_minutes,
5 to 1440), a tenth of it either way at random so that a machine's servers
spread their asking */
static real server_update_interval(
	void)
{
	long minutes = PIN(config_integer("server.update_check_minutes"), 5, 24 * 60);
	long long spread = posix_server_unix_milliseconds() % 2001 - 1000;

	return (real)(minutes * 60) * (1.0f + (real)spread / 10000.0f);
}

/* the server stopped (exit code 5) to be started again as the build
installed */
static void server_update_restart(
	char const *why)
{
	server_log(_server_log_info, "update", "build %ld installed: stopping to be started again (%s)",
		server.update_installing_build, why);
	server_director_quit(SERVER_EXIT_RESTART, "update installed");
}

/* a build installed, for when the server stops to start it: one of the same
network version once nobody has played for UPDATE_EMPTY_SECONDS; one of a
new network version (which no player of this one can join once the others
update) at the end of the game being played, at once with nobody on, after
UPDATE_LOBBY_NOTICE_SECONDS in the lobby, and after server.update_game_minutes
of a game that does not end (ended as sv_end_game ends it, its scores and
postgame shown) */
static void server_update_installed(
	real seconds)
{
	boolean playing = server.state == _server_state_playing || server.state == _server_state_loading;

	server.update_installed_seconds += seconds;
	if (!server.update_new_network)
	{
		if (!server_empty())
		{
			server.update_empty_seconds = 0.0f;
			return;
		}
		server.update_empty_seconds += seconds;
		if (server.update_empty_seconds >= UPDATE_EMPTY_SECONDS)
			server_update_restart("empty for 10 minutes");
		return;
	}
	if (server_empty() && !playing)
	{
		server_update_restart("a new network version, and nobody plays");
		return;
	}
	if (playing)
	{
		long minutes = config_integer("server.update_game_minutes");

		server.update_after_game = TRUE;
		if (minutes > 0 && server.update_installed_seconds >= 60.0f * (real)minutes && !server.update_end_asked &&
			server.state == _server_state_playing)
		{
			server.update_end_asked = TRUE;
			distributed_send_notice("This server ends the game to update");
			server_log(_server_log_info, "update", "the game has gone on %ld minutes since the update was installed: "
				"it ends (server.update_game_minutes)", minutes);
			server_director_end_game(_server_end_update, FALSE);
		}
		return;
	}
	if (server.state == _server_state_postgame)
		return;
	/* (the game it waited for is over, and its postgame shown: at once) */
	if (server.update_after_game)
	{
		server_update_restart("a new network version, after the game");
		return;
	}
	/* (in the lobby: a short notice first) */
	if (server.update_lobby_seconds == 0.0f)
	{
		char notice[128];

		snprintf(notice, sizeof(notice), "This server restarts to update in %d seconds", UPDATE_LOBBY_NOTICE_SECONDS);
		distributed_send_notice(notice);
	}
	server.update_lobby_seconds += seconds;
	if (server.update_lobby_seconds >= UPDATE_LOBBY_NOTICE_SECONDS)
		server_update_restart("a new network version, from the lobby");
}

/* a warning event and log line, once a build is out */
static void server_update_warning(
	char const *text)
{
	struct server_json json;

	server_log(_server_log_warning, "update", "%s", text);
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", "warning");
	server_json_string(&json, "text", text);
	server_json_object_end(&json);
	server_event_add("server", server_json_text(&json));
	server_json_free(&json);
}

/* server.auto_update: "check" looks for a new release every
server.update_check_minutes and says when one is out (a warning event);
"install" (the default) also installs it at once (on a thread of its own:
the server goes on meanwhile), and stops with SERVER_EXIT_RESTART, for what
runs the server to start the new build, when server_update_installed says */
static void server_auto_update(
	real seconds)
{
	char const *mode = config_string("server.auto_update");
	boolean install = !strcmp(mode, "install");
	long available;

	/* (the install under way, or done: nothing else looked for) */
	if (server.update_installing)
	{
		char error[512];
		int installed = updater_server_install_poll(error, sizeof(error));

		if (installed == 0)
			return;
		server.update_installing = FALSE;
		if (installed > 0)
		{
			server.update_installed = TRUE;
			server.update_installed_seconds = 0.0f;
			server.update_empty_seconds = 0.0f;
			server.update_after_game = server.state == _server_state_playing || server.state == _server_state_loading;
			if (server.update_new_network)
			{
				server_log(_server_log_info, "update", "build %ld installed: a new network version, started %s",
					server.update_installing_build, server.update_after_game ? "once this game is over" :
						"after a notice");
				if (server.update_after_game)
					distributed_send_notice("This server updates after this game");
			}
			else
			{
				server_log(_server_log_info, "update", "build %ld installed: started once the server has been empty "
					"for 10 minutes", server.update_installing_build);
			}
		}
		else
		{
			/* (the next look finds it again) */
			server.update_check_seconds = server_update_interval();
			server_log(_server_log_error, "update", "build %ld could not be installed: %s (tried again in %ld "
				"minutes)", server.update_installing_build, error, (long)(server.update_check_seconds / 60.0f));
			return;
		}
	}
	if (server.update_installed)
	{
		server_update_installed(seconds);
		return;
	}
	if (!install && strcmp(mode, "check"))
		return;
	/* (the first look soon after the start, at a moment of its own) */
	if (!server.update_checked_once)
	{
		server.update_checked_once = TRUE;
		server.update_check_seconds = (real)(posix_server_unix_milliseconds() % (UPDATE_FIRST_CHECK_SECONDS * 1000)) /
			1000.0f;
	}
	server.update_check_seconds -= seconds;
	if (server.update_check_seconds <= 0.0f)
	{
		server.update_check_seconds = server_update_interval();
		if (!updater_server_check())
		{
			/* (never again: this build has no number to compare) */
			server.update_check_seconds = 1e30f;
			server_log(_server_log_warning, "update", "server.auto_update is %s, but this build has no number: "
				"only the builds CI numbers update themselves", mode);
			return;
		}
	}
	available = updater_server_available();
	if (!available)
		return;
	{
		long network_version = updater_server_available_network_version();
		boolean new_network = network_version > 0 && network_version != HALO_PORT_NETWORK_VERSION;

		if (available != server.update_told)
		{
			char text[200];

			server.update_told = available;
			snprintf(text, sizeof(text), "build %ld is out (this is build %s)%s%s", available, server_mode_build(),
				new_network ? ", a new network version" : "",
				!install ? "" : new_network ? ": installed now, and started once this game is over" :
					": installed now, and started once the server has been empty for 10 minutes");
			server_update_warning(text);
		}
		if (!install)
			return;
		server.update_installing_build = available;
		server.update_new_network = new_network;
		server.update_lobby_seconds = 0.0f;
		server.update_end_asked = FALSE;
		if (updater_server_install_start())
		{
			server.update_installing = TRUE;
			server_log(_server_log_info, "update", "installing build %ld", available);
		}
		else
		{
			char error[512];

			updater_server_install_poll(error, sizeof(error));
			server.update_check_seconds = server_update_interval();
			server_log(_server_log_error, "update", "build %ld could not be installed: %s (tried again in %ld "
				"minutes)", available, error, (long)(server.update_check_seconds / 60.0f));
		}
	}
}

/* ---------- the players */

void server_director_player_removed(
	long machine_index,
	char const *reason)
{
	if (machine_index >= 0 && machine_index < MAXIMUM_MACHINES)
	{
		server.leave_reasons[machine_index] = reason;
		server.leave_reason_seconds[machine_index] = 0.0f;
	}
}

/* a player's joining or leaving (player_joined: its team and machine;
player_left: its reason), as every role sees it, and with its address and
hardware id as it joined for the readers who see them (the owner, or every
role with server.log_addresses) */
static void server_player_event(
	char const *event,
	long slot,
	long team,
	long machine,
	char const *reason)
{
	struct server_json json[2];
	long pass;

	for (pass = 0; pass < 2; pass++)
	{
		server_json_begin(&json[pass]);
		server_json_object_begin(&json[pass], NULL);
		server_json_string(&json[pass], "event", event);
		server_json_integer(&json[pass], "slot", slot);
		server_json_string(&json[pass], "name", server.seen_names[slot]);
		if (reason)
		{
			server_json_string(&json[pass], "reason", reason);
		}
		else
		{
			server_json_integer(&json[pass], "team", team);
			server_json_integer(&json[pass], "machine", machine);
		}
		if (pass)
		{
			server_json_string(&json[pass], "address", server.seen_addresses[slot]);
			server_json_string(&json[pass], "hwid", server.seen_hardware_ids[slot]);
			server_json_string(&json[pass], "discord_id", server.seen_discord_ids[slot]);
			server_json_string(&json[pass], "discord_name", server.seen_discord_names[slot]);
			server_json_boolean(&json[pass], "discord_verified", server.seen_discord_verified[slot]);
		}
		server_json_object_end(&json[pass]);
	}
	server_event_add_addressed("player", server_json_text(&json[0]), server_json_text(&json[1]));
	server_json_free(&json[0]);
	server_json_free(&json[1]);
}

static void server_note_players(
	real seconds)
{
	struct network_game *game = server_game();
	long slot;
	long machine_index;

	/* (a command's reason for a machine's leaving, for a moment) */
	for (machine_index = 0; machine_index < MAXIMUM_MACHINES; machine_index++)
	{
		if (server.leave_reasons[machine_index])
		{
			server.leave_reason_seconds[machine_index] += seconds;
			if (server.leave_reason_seconds[machine_index] >= LEAVE_REASON_SECONDS)
				server.leave_reasons[machine_index] = NULL;
		}
	}

	for (slot = 0; slot < MAXIMUM_SLOTS; slot++)
	{
		char name[16] = "";
		long machine = NONE;

		if (game && network_player_is_valid(&game->players[slot]))
		{
			server_status_player_name(slot, name, sizeof(name));
			machine = game->players[slot].machine_index;
		}
		if (strcmp(name, server.seen_names[slot]) || machine != server.seen_machines[slot])
		{
			if (server.seen_names[slot][0])
			{
				long seen = server.seen_machines[slot];
				char const *reason = seen >= 0 && seen < MAXIMUM_MACHINES && server.leave_reasons[seen] ?
					server.leave_reasons[seen] : "left";

				server_log(_server_log_info, "player", "%s %s (slot %ld)", server.seen_names[slot], reason, slot);
				server_player_event("player_left", slot, NONE, NONE, reason);
			}
			snprintf(server.seen_names[slot], sizeof(server.seen_names[slot]), "%s", name);
			server.seen_machines[slot] = machine;
			server.seen_addresses[slot][0] = 0;
			server.seen_hardware_ids[slot][0] = 0;
			server.seen_discord_ids[slot][0] = 0;
			server.seen_discord_names[slot][0] = 0;
			server.seen_discord_verified[slot] = FALSE;
			if (name[0])
			{
				unsigned long address = distributed_machine_real_address(machine);
				char const *hardware_id = network_game_server_machine_hardware_id(machine);

				server_log(_server_log_info, "player", "%s joined (slot %ld, machine %ld)", name, slot, machine);
				server.seen_joined[slot] = posix_server_unix_milliseconds();
				if (address)
				{
					snprintf(server.seen_addresses[slot], sizeof(server.seen_addresses[slot]), "%lu.%lu.%lu.%lu",
						(address >> 24) & 255, (address >> 16) & 255, (address >> 8) & 255, address & 255);
				}
				snprintf(server.seen_hardware_ids[slot], sizeof(server.seen_hardware_ids[slot]), "%s",
					hardware_id ? hardware_id : "");
				server.seen_discord_verified[slot] = network_distributed_machine_discord(machine,
					server.seen_discord_ids[slot], sizeof(server.seen_discord_ids[slot]),
					server.seen_discord_names[slot], sizeof(server.seen_discord_names[slot]));
				server_player_event("player_joined", slot, game->variant.universal_variant.teams ?
					game->players[slot].team_index : -1, machine, NULL);
			}
			server.motd_sent[slot] = FALSE;
			server.motd_waits[slot] = 0.0f;
		}
		/* the message of the day, once the player's machine plays */
		if (name[0] && !server.motd_sent[slot] && server.state == _server_state_playing && config_string("server.motd")[0])
		{
			server.motd_waits[slot] += seconds;
			if (server.motd_waits[slot] >= MOTD_DELAY_SECONDS)
			{
				long other;

				/* (a machine's players see it once) */
				for (other = 0; other < MAXIMUM_SLOTS; other++)
				{
					if (server.seen_machines[other] == machine)
						server.motd_sent[other] = TRUE;
				}
				distributed_send_notice_to_machine(machine, config_string("server.motd"));
			}
		}
	}
}

long long server_director_joined_time(
	long slot)
{
	return slot >= 0 && slot < MAXIMUM_SLOTS ? server.seen_joined[slot] : 0;
}

/* the invite, said when it changes: whole on the standard output alone
(the owner's: a private server's invite lets anyone in), a word of it in
the log the web's viewers read */
static void server_note_invite(
	void)
{
	char invite[P2P_LISTING_INVITE_SIZE];

	if (!p2p_invite_link(invite, sizeof(invite)))
		invite[0] = 0;
	if (strcmp(invite, server.invite))
	{
		snprintf(server.invite, sizeof(server.invite), "%s", invite);
		if (invite[0])
		{
			server_log(_server_log_info, "net", "a new invite (on the standard output; /api/status gives it to admins)");
			server_log_private(_server_log_info, "net", "invite: %s", invite);
		}
	}
}

/* ---------- the lobby's rule (network_server_manager.c) */

boolean server_director_ok_to_countdown(
	long player_count,
	boolean needs_more_teams)
{
	long minimum = PIN(config_integer("server.min_players"), 1, 16);
	char const *team_start = config_string("server.team_start");

	if (server.quitting || server.state != _server_state_lobby || !server.entry_applied || player_count < minimum)
		return FALSE;
	/* (a team game: with one team's players, unless "both") */
	if (needs_more_teams && !strcmp(team_start, "both"))
		return FALSE;
	return TRUE;
}

/* ---------- each state */

static void server_hosting(
	real seconds)
{
	server.menu_seconds += seconds;
	if (server.menu_seconds < MENU_SETTLE_SECONDS)
		return;
	server_apply_game_settings();
	player_ui_fast_setup_network_server();
	if (!global_network_game_server_get() || !global_network_game_client_get())
	{
		server_log(_server_log_warning, "server", "could not host (the game's ports taken by another program? "
			"network.address gives each server on a machine its own): trying again in %d s", HOSTING_RETRY_SECONDS);
		server.menu_seconds = MENU_SETTLE_SECONDS - HOSTING_RETRY_SECONDS;
		return;
	}
	server_apply_game_settings();
	server_apply_listing();
	server.entry_applied = FALSE;
	server.lobby_seconds = 0.0f;
	server.back_to_lobby = FALSE;
	server_set_state(_server_state_lobby);
	server_log(_server_log_info, "lobby", "hosting");
}

/* whether the entry is a team game's */
static boolean server_entry_teams(
	struct server_entry const *entry)
{
	struct game_variant variant;
	struct game_variant_options options;
	char reason[SERVER_REASON_SIZE];

	return strcmp(entry->gametype, "coop") && server_gametype_build(entry->gametype, entry->rules, entry->rule_count,
		&variant, &options, reason, sizeof(reason)) && variant.universal_variant.teams;
}

/* whether the rotation has a game one player can play (no team game's) */
static boolean server_rotation_has_solo_entry(
	void)
{
	long index;

	for (index = 0; index < server.rotation.count; index++)
	{
		if (!server.rotation.entries[index].rejected[0] && !server_entry_teams(&server.rotation.entries[index]))
			return TRUE;
	}
	return FALSE;
}

/* the lobby's map vote (server.map_vote): after a game with players in
it, server.map_vote_choices entries of the rotation at random (not the one
just played, nor one twice), for server.map_vote_seconds; the one that won
the next. TRUE while it runs (the lobby's entry waits for it) */
static boolean server_map_vote(
	void)
{
	short winner;
	long index;

	if (server.map_vote_done)
		return FALSE;
	if (!network_map_vote_running())
	{
		char map_names[NETWORK_MAP_VOTE_MAXIMUM_CHOICES][NETWORK_MAP_VOTE_MAP_SIZE];
		char gametypes[NETWORK_MAP_VOTE_MAXIMUM_CHOICES][NETWORK_MAP_VOTE_GAMETYPE_SIZE];
		long candidates[SERVER_ROTATION_MAXIMUM_ENTRIES];
		long candidate_count = 0;
		long wanted = PIN(config_integer("server.map_vote_choices"), 2, NETWORK_MAP_VOTE_MAXIMUM_CHOICES);
		long seconds = PIN(config_integer("server.map_vote_seconds"), 5, 120);
		struct server_json json;

		server.map_vote_done = TRUE;
		if (!config_boolean("server.map_vote") || server.has_interposed || !server.games_hosted ||
			server_player_count() == 0)
		{
			return FALSE;
		}
		for (index = 0; index < server.rotation.count; index++)
		{
			long other;

			if (server.rotation.entries[index].rejected[0])
				continue;
			for (other = 0; other < candidate_count &&
				strcmp(server.rotation.entries[candidates[other]].text, server.rotation.entries[index].text); other++)
			{
			}
			if (other == candidate_count)
				candidates[candidate_count++] = index;
		}
		/* (not the one just played, when there are others enough) */
		if (candidate_count > 2 && server.current_set && server.current_index != NONE)
		{
			for (index = 0; index < candidate_count; index++)
			{
				if (!strcmp(server.rotation.entries[candidates[index]].text, server.current.text))
				{
					candidates[index] = candidates[--candidate_count];
					break;
				}
			}
		}
		if (candidate_count < 2)
			return FALSE;
		/* (shuffled: the first wanted) */
		for (index = candidate_count - 1; index > 0; index--)
		{
			long other = (long)(((unsigned long)rand() ^ system_milliseconds()) % (unsigned long)(index + 1));
			long swap = candidates[index];

			candidates[index] = candidates[other];
			candidates[other] = swap;
		}
		server.map_vote_count = (short)MIN(wanted, candidate_count);
		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_string(&json, "event", "map_vote_started");
		server_json_integer(&json, "seconds", seconds);
		server_json_array_begin(&json, "choices");
		for (index = 0; index < server.map_vote_count; index++)
		{
			struct server_entry const *entry = &server.rotation.entries[candidates[index]];

			server.map_vote_entries[index] = candidates[index];
			snprintf(map_names[index], sizeof(map_names[index]), "%s", entry->map_name);
			snprintf(gametypes[index], sizeof(gametypes[index]), "%s", entry->gametype);
			server_json_object_begin(&json, NULL);
			server_json_string(&json, "map", entry->map_name);
			server_json_string(&json, "gametype", entry->gametype);
			server_json_integer(&json, "index", candidates[index]);
			server_json_object_end(&json);
		}
		server_json_array_end(&json);
		server_json_object_end(&json);
		server_event_add("game", server_json_text(&json));
		server_json_free(&json);
		network_map_vote_begin((char const (*)[NETWORK_MAP_VOTE_MAP_SIZE])map_names,
			(char const (*)[NETWORK_MAP_VOTE_GAMETYPE_SIZE])gametypes, server.map_vote_count, (short)seconds);
		server_log(_server_log_info, "lobby", "map vote: %d choices for %ld s", server.map_vote_count, seconds);
		server.map_vote_done = FALSE;
		return TRUE;
	}
	/* (everyone gone: no vote) */
	if (server_player_count() == 0)
	{
		network_map_vote_cancel();
		server.map_vote_done = TRUE;
		return FALSE;
	}
	winner = network_map_vote_host_update();
	if (winner == NONE)
		return TRUE;
	server.map_vote_done = TRUE;
	index = server.map_vote_entries[winner];
	if (index < 0 || index >= server.rotation.count || server.rotation.entries[index].rejected[0])
		return FALSE;
	server.index = (index + server.rotation.count - 1) % server.rotation.count;
	server.index_anchored = TRUE;
	server_log(_server_log_info, "lobby", "map vote: %s on %s won (%d vote%s)", server.rotation.entries[index].gametype,
		server.rotation.entries[index].map_name, network_map_vote_votes(winner),
		network_map_vote_votes(winner) == 1 ? "" : "s");
	{
		struct server_json json;

		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_string(&json, "event", "map_vote_ended");
		server_json_string(&json, "map", server.rotation.entries[index].map_name);
		server_json_string(&json, "gametype", server.rotation.entries[index].gametype);
		server_json_integer(&json, "index", index);
		server_json_integer(&json, "votes", network_map_vote_votes(winner));
		server_json_object_end(&json);
		server_event_add("game", server_json_text(&json));
		server_json_free(&json);
	}
	return FALSE;
}

/* whether the lobby's map is to be set again, its widgets having put
another back: not once the game is starting, nor while an MCC map waits for
its package check (mcc_ui_network.c: the lobby shows its stand-in map
meanwhile, and setting the MCC map again would start the check over, so
that a check longer than the second between looks never ends) */
static boolean server_lobby_map_lost(
	struct network_game_server *network_server,
	struct network_game const *game)
{
	return game && csstrcasecmp(game->map.name, server.current.level) &&
		!network_game_server_port_start_sent(network_server) && !mcc_ui_network_map_pending(network_server);
}

static void server_lobby(
	real seconds)
{
	struct network_game_server *network_server = global_network_game_server_get();
	long players = server_player_count();
	long minimum = PIN(config_integer("server.min_players"), 1, 16);
	boolean skip_teams = !strcmp(config_string("server.team_start"), "skip");
	long countdown;

	server.lobby_seconds += seconds;
	if (!server.entry_applied)
	{
		struct server_entry const *entry;
		long index;

		if (server.lobby_seconds < LOBBY_SETUP_SECONDS)
			return;
		/* (the players' vote for it first: server.map_vote) */
		if (server_map_vote())
			return;
		/* (a team entry skipped while one player waits: server.team_start skip) */
		entry = server_lobby_entry(&index);
		if (entry && index != NONE && skip_teams && players == 1)
		{
			long tries;

			for (tries = 0; tries < server.rotation.count && entry && server_entry_teams(entry); tries++)
			{
				server.index = index;
				server.index_anchored = TRUE;
				entry = server_lobby_entry(&index);
			}
		}
		if (!entry)
		{
			server_log(_server_log_error, "server", "nothing in the rotation can be played");
			server_director_quit(SERVER_EXIT_CONFIGURATION, "nothing to play");
			return;
		}
		if (!server_apply_entry(entry, index))
		{
			/* (its map gone since: the next) */
			if (index == NONE)
				server.has_interposed = FALSE;
			else
				snprintf(server.rotation.entries[index].rejected, sizeof(server.rotation.entries[index].rejected),
					"could not be set up");
			server.lobby_seconds = 0.0f;
			return;
		}
		if (index != NONE)
		{
			server.index = index;
			server.index_anchored = TRUE;
		}
		server.entry_applied = TRUE;
		server.map_checked_seconds = server.lobby_seconds;
		network_game_server_pause_countdown(network_server, TRUE);
		return;
	}
	/* (the lobby's widgets may put their map back: the entry's again, said) */
	if (server.lobby_seconds - server.map_checked_seconds >= 1.0f && strcmp(server.current.gametype, "coop"))
	{
		struct network_game *game = server_game();

		server.map_checked_seconds = server.lobby_seconds;
		if (server_lobby_map_lost(network_server, game))
		{
			server_log(_server_log_warning, "lobby", "the lobby's map became %s: %s set again", game->map.name,
				server.current.level);
			network_game_server_change_map_name(network_server, server.current.level);
		}
	}
	/* the countdown: held below the players wanted, started with them in */
	countdown = network_game_server_port_countdown_milliseconds(network_server);
	if (players < minimum)
	{
		if (!network_game_server_port_countdown_paused(network_server))
		{
			network_game_server_pause_countdown(network_server, TRUE);
			if (countdown != NONE)
				server_simple_event("game", "countdown", "seconds", -1);
		}
	}
	else if (countdown == NONE || network_game_server_port_countdown_paused(network_server))
	{
		long lobby_seconds = PIN(config_integer("server.lobby_seconds"), 0, 300);

		/* (as the game's server checks it each tick, its teams as
		server.team_start says: a countdown it does not allow it stops at
		once) */
		if (network_game_server_port_ok_to_countdown(network_server))
		{
			/* (a team game, and one player waits: the next entry for him, if
			the rotation has one he can play: server.team_start skip) */
			if (skip_teams && players == 1 && !server.current_is_interposed && server_entry_teams(&server.current) &&
				server_rotation_has_solo_entry())
			{
				server_log(_server_log_info, "lobby", "one player waits: %s skipped (server.team_start)",
					server.current.text);
				server.entry_applied = FALSE;
				server.lobby_seconds = LOBBY_SETUP_SECONDS;
				return;
			}
			network_game_server_port_countdown_begin(network_server, lobby_seconds ? lobby_seconds * 1000 - 1 : 0);
			server_log(_server_log_info, "lobby", "%ld player%s in: the game starts in %ld s", players,
				players == 1 ? "" : "s", lobby_seconds);
			server_simple_event("game", "countdown", "seconds", lobby_seconds);
		}
	}
	if (network_game_server_port_start_sent(network_server))
	{
		server_set_state(_server_state_loading);
		server_log(_server_log_info, "game", "loading %s (%s)", server.current.map_name, server.current.gametype);
	}
}

static void server_playing(
	real seconds)
{
	long players;

	if (!game_engine_running() && !server_cooperative())
		return;
	if (game_engine_running() && !game_engine_can_score())
	{
		/* the game over: its scores (or, skipped, at once the lobby) */
		struct network_game *game = server_game();

		/* (a game the engine ended as its last player left (whose quit, a
		tick or two on, may not have come yet: its machine has gone), at its
		time limit, else at its score) */
		if (!server.end_asked)
		{
			server.end_reason = server_players_in_game() == 0 ||
				network_game_server_port_remote_machine_count(global_network_game_server_get(), TRUE) == 0 ?
				_server_end_empty :
				game && game->variant_options.time_limit > 0 &&
				server.game_seconds >= 60.0f * game->variant_options.time_limit - 2.0f ? _server_end_time :
				_server_end_score;
		}
		server_game_over();
		server_set_state(_server_state_postgame);
		return;
	}
	/* (a co-op level not loaded yet: nothing to count) */
	if (server_cooperative() && !game_in_progress())
		return;
	server.game_seconds += seconds;
	/* nobody in it (and nobody joining): it ends */
	players = server_players_in_game();
	if (players == 0 && network_game_server_port_remote_machine_count(global_network_game_server_get(), FALSE) == 0)
	{
		server.empty_seconds += seconds;
		if (config_integer("server.empty_seconds") > 0 && server.empty_seconds >= (real)config_integer("server.empty_seconds"))
			server_director_end_game(_server_end_empty, TRUE);
	}
	else
	{
		server.empty_seconds = 0.0f;
	}
	/* no score for so long: it ends */
	{
		long red, blue, sum = 0;
		struct data_iterator iterator;
		struct player_datum *player;

		if (server_status_team_scores(&red, &blue))
			sum = red + blue;
		if (player_data)
		{
			data_iterator_new(&iterator, player_data);
			while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
				sum += player->statistics.kills[0] + player->statistics.deaths;
		}
		if (sum != server.score_sum)
		{
			server.score_sum = sum;
			server.idle_seconds = 0.0f;
		}
		else
		{
			server.idle_seconds += seconds;
		}
		if (config_integer("server.idle_minutes") > 0 && players > 0 &&
			server.idle_seconds >= 60.0f * (real)config_integer("server.idle_minutes"))
		{
			server_director_end_game(_server_end_idle, FALSE);
		}
	}
	/* scores, once a second at most, as they change (but for a co-op level
	ended just now) */
	if (server.state != _server_state_playing)
		return;
	server.score_event_seconds += seconds;
	if (server.score_event_seconds >= 1.0f && server_status_scores_changed())
	{
		server.score_event_seconds = 0.0f;
		server_game_event("score", NULL);
	}
}

static void server_postgame(
	real seconds)
{
	struct network_game_server *network_server = global_network_game_server_get();
	real wanted = server.skip_postgame ? 0.0f : (real)PIN(config_integer("server.postgame_seconds"), 0, 300);

	/* (the engine's end of the game first: it switches the server to its
	postgame a few seconds on; a game whose scores are skipped switches at
	once. The lobby only takes the machines from the server's postgame) */
	if (server_network_state() != SERVER_POSTGAME)
	{
		if (server.skip_postgame && server_network_state() == SERVER_INGAME)
			network_game_server_switch_to_postgame(network_server);
		server.postgame_wait = 0.0f;
		return;
	}
	/* (a game nobody saw: its scores a moment only) */
	if (!server.skip_postgame && server_players_in_game() == 0)
		wanted = MIN(wanted, (real)EMPTY_POSTGAME_SECONDS);
	server.postgame_wait += seconds;
	if (server.postgame_wait >= wanted && !server.back_to_lobby)
	{
		/* (the lobby's machines may still be joining: tried each second until
		it takes) */
		if (network_game_server_reset_to_pregame(network_server))
			server.back_to_lobby = TRUE;
		else
			server.postgame_wait = wanted - 1.0f;
	}
}

/* back in the lobby after a game: the next entry */
static void server_back_in_lobby(
	void)
{
	struct network_game_server *network_server = global_network_game_server_get();

	server.back_to_lobby = FALSE;
	server_advance();
	server.entry_applied = FALSE;
	server.map_vote_done = FALSE;
	server.lobby_seconds = 0.0f;
	server.end_asked = FALSE;
	server.skip_postgame = FALSE;
	server.game_over_said = FALSE;
	server.game_seconds = server.empty_seconds = server.idle_seconds = 0.0f;
	server.score_sum = 0;
	/* (a join queued as the game ended would hold every later one) */
	network_game_server_port_clear_queued_players(network_server);
	network_game_server_pause_countdown(network_server, TRUE);
	server_set_state(_server_state_lobby);
}

/* ---------- each frame */

static void server_run_line(
	char const *line,
	int role,
	int origin)
{
	boolean succeeded;
	char *output;

	/* (the owner's console's lines on the standard output alone: what the
	owner sees, addresses and all, is not the web's viewers' to read) */
	if (origin == _server_origin_standard_input)
		server_log_private(_server_log_info, "console", "> %s", server_console_line_is_secret(line) ?
			"(a password's command)" : line);
	output = server_commands_run(line, role, origin, &succeeded);
	if (output)
	{
		char const *cursor = output;

		while (*cursor)
		{
			char const *end = strchr(cursor, '\n');
			long length = end ? (long)(end - cursor) : (long)strlen(cursor);

			if (length > 0)
			{
				server_log_private(succeeded ? _server_log_info : _server_log_warning, "console", "%.*s", (int)length,
					cursor);
			}
			cursor += length + (end != NULL);
		}
		server_console_free(output);
	}
}

static void server_signals(
	void)
{
	int quit_signal = 0;
	int signals = server_mode_signals(&quit_signal);

	if (signals & _posix_server_signal_quit)
	{
		char why[32];

		snprintf(why, sizeof(why), "signal %d", quit_signal);
		server_director_quit(SERVER_EXIT_STOPPED, why);
	}
	if (signals & _posix_server_signal_reload)
		server_director_reload();
	if (signals & _posix_server_signal_status)
	{
		struct server_json json;

		server_json_begin(&json);
		server_status_write(&json, _server_role_owner);
		fprintf(stdout, "%s\n", server_json_text(&json));
		fflush(stdout);
		server_json_free(&json);
	}
}

/* the status file (server.status_file), written whole and put in place */
static void server_write_status_file(
	void)
{
	char const *path = config_string("server.status_file");
	struct server_json json;

	if (!path[0])
		return;
	server_json_begin(&json);
	server_status_write(&json, _server_role_viewer);
	/* (a path of the host's: not the game's fopen, which takes the Xbox's) */
	posix_server_write_file(path, server_json_text(&json), (unsigned long)strlen(server_json_text(&json)));
	server_json_free(&json);
}

static void server_health_line(
	void)
{
	static unsigned long last_cpu;
	static double last_uptime;
	unsigned long cpu = posix_server_cpu_milliseconds();
	double uptime = server_mode_uptime();
	double longest, mean;
	double share = uptime > last_uptime ? (double)(cpu - last_cpu) / ((uptime - last_uptime) * 10.0) : 0.0;

	server_mode_frame_times(&longest, &mean);
	server_log(_server_log_info, "health", "players=%ld frame_ms_max=%.1f frame_ms_avg=%.2f cpu=%.1f%% rss=%luMB",
		server_player_count(), longest, mean, share, posix_server_resident_bytes() / (1024UL * 1024UL));
	last_cpu = cpu;
	last_uptime = uptime;
}

/* the state published to the master servers: what the listing lacks */
static void server_publish_state(
	void)
{
	struct server_json json;
	struct network_game *game = server_game();
	char variant[16] = "";

	if (game)
	{
		long index;

		for (index = 0; index < 11 && game->variant.human_readable_game_description[index]; index++)
		{
			wchar_t character = game->variant.human_readable_game_description[index];

			variant[index] = character >= 0x20 && character < 0x7F ? (char)character : '?';
		}
		variant[index] = 0;
	}
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "variant", variant);
	server_json_string(&json, "state", server_status_state_name(server.state));
	server_json_integer(&json, "players", server_player_count());
	server_json_string(&json, "map", server.current_set ? server.current.map_name : "");
	server_json_string(&json, "gametype", server.current_set ? server.current.gametype : "");
	server_json_object_end(&json);
	if (strcmp(server_json_text(&json), server.master_state))
	{
		snprintf(server.master_state, sizeof(server.master_state), "%s", server_json_text(&json));
		server_master_set_state(server.master_state, TRUE);
	}
	server_json_free(&json);
}

void server_update(
	boolean main_menu_loaded,
	real seconds)
{
	struct network_game_server *network_server;
	short network_state;
	char line[SERVER_COMMAND_LINE_SIZE];
	int origin;

	if (!halo_server_mode())
		return;
	if (!server.started)
	{
		long index;

		server.started = TRUE;
		/* (nothing is drawn: nobody sees it) */
		debug_no_drawing = TRUE;
		for (index = 0; index < MAXIMUM_SLOTS; index++)
			server.seen_machines[index] = NONE;
		server_read_rotation(&server.rotation, config_string("server.rotation"));
		server.index = server.rotation.count - 1;
		server.current_index = NONE;
		server.last_countdown_seconds = NONE;
		{
			struct server_json json;
			long entry;

			server_json_begin(&json);
			server_json_object_begin(&json, NULL);
			server_json_string(&json, "event", "server_started");
			server_json_string(&json, "name", config_string("server.name"));
			server_json_string(&json, "build", server_mode_build());
			server_json_integer(&json, "network_version", HALO_PORT_NETWORK_VERSION);
			server_status_identity(&json);
			server_json_array_begin(&json, "rotation");
			for (entry = 0; entry < server.rotation.count; entry++)
				server_json_string(&json, NULL, server.rotation.entries[entry].text);
			server_json_array_end(&json);
			server_json_object_end(&json);
			server_event_add("server", server_json_text(&json));
			server_json_free(&json);
		}
	}
	server_signals();
	server_console_frame();
	while (server_console_next(line, sizeof(line), &origin))
	{
		/* (a line left out, too long, comes empty) */
		if (line[0])
			server_run_line(line, _server_role_owner, origin);
	}
	server_http_main_thread();
	if (!server.quitting)
	{
		server_exit_after();
		server_auto_update(seconds);
	}

	/* stopping: the machines given a moment to hear it, then the end */
	if (server.quitting)
	{
		server.quit_seconds += seconds;
		if (server.quit_seconds >= QUIT_SECONDS)
		{
			dispose_global_network_game_server();
			server_log(_server_log_info, "server", "stopped (exit code %d)", server.exit_code);
			server_mode_exit(server.exit_code);
		}
		return;
	}
	/* a stop that waited, when its moment comes (stopping from the next
	frame) */
	server_drain_update();
	if (server.quitting)
		return;

	network_server = global_network_game_server_get();
	network_state = server_network_state();
	mcc_server_update((network_state == SERVER_PREGAME && server.state != _server_state_loading &&
        server.state != _server_state_playing) || (!network_server && main_menu_loaded));
	server.state_seconds += seconds;
	server_mode_set_idle(!network_server ||
		(network_state == SERVER_PREGAME && !network_game_server_port_remote_machine_count(network_server, FALSE)));

	if (!network_server)
	{
		/* the intro movie skipped, as a button press does (it holds the
		texture cache's spare memory, which the map takes) */
		if (main_menu_loaded && bink_playback_in_progress())
		{
			bink_playback_stop();
			return;
		}
		if (server.state != _server_state_waiting && server.state != _server_state_hosting)
		{
			server_log(_server_log_warning, "server", "the game's server stopped: hosting again");
			server.current_set = FALSE;
			server.current_is_interposed = FALSE;
			server.invite[0] = 0;
		}
		if (!main_menu_loaded)
		{
			server_set_state(_server_state_waiting);
			return;
		}
		if (server.state != _server_state_hosting)
		{
			server_set_state(_server_state_hosting);
			server.menu_seconds = 0.0f;
		}
		server_hosting(seconds);
		return;
	}

	server_note_invite();
	server_note_players(seconds);
	switch (network_state)
	{
	case SERVER_PREGAME:
		if (!main_menu_loaded)
			break;
		if (server.state == _server_state_postgame || server.state == _server_state_playing ||
			server.state == _server_state_loading)
		{
			/* (a co-op level won: network_game_server_port_cooperative_won
			took every machine to the lobby at once) */
			if (server.state == _server_state_playing && server_cooperative() && !server.game_over_said)
			{
				if (!server.end_asked)
					server.end_reason = _server_end_score;
				server.skip_postgame = TRUE;
				server_game_over();
			}
			if (server.state != _server_state_postgame)
				server_log(_server_log_info, "game", "back in the lobby");
			server_back_in_lobby();
		}
		if (server.state == _server_state_hosting || server.state == _server_state_waiting)
			server_set_state(_server_state_lobby);
		server_lobby(seconds);
		break;
	case SERVER_INGAME:
		if (server.state == _server_state_lobby || server.state == _server_state_loading)
		{
			if (game_engine_running() || server_cooperative())
			{
				server.games_hosted++;
				server.game_seconds = server.empty_seconds = server.idle_seconds = 0.0f;
				server.end_asked = FALSE;
				server.skip_postgame = FALSE;
				server.game_over_said = FALSE;
				server_set_state(_server_state_playing);
				server_log(_server_log_info, "game", "started: %s on %s, %ld player%s", server.current.gametype,
					server.current.map_name, server_player_count(), server_player_count() == 1 ? "" : "s");
				server_game_event("game_started", NULL);
			}
			else if (server.state == _server_state_lobby)
			{
				server_set_state(_server_state_loading);
			}
		}
		if (server.state == _server_state_playing)
			server_playing(seconds);
		else if (server.state == _server_state_postgame)
			server_postgame(seconds);
		break;
	case SERVER_POSTGAME:
		if (server.state != _server_state_postgame)
			server_set_state(_server_state_postgame);
		server_postgame(seconds);
		break;
	}

	/* the ready line: the lobby open (and listed, or 10 s on) */
	if (!server.ready_said && server.state == _server_state_lobby && server.entry_applied)
	{
		server.ready_wait += seconds;
		if (!config_boolean("server.public") || !config_boolean("network.online") || server.invite[0] ||
			server.ready_wait >= 10.0f)
		{
			char web[64] = "off";

			server.ready_said = TRUE;
			if (config_integer("server.web_port") > 0)
				snprintf(web, sizeof(web), "%s:%ld", config_string("server.web_bind"), config_integer("server.web_port"));
			server_log(_server_log_info, "server", "ready (tunnel %ld/udp, web %s, listing %s)",
				config_boolean("network.online") ? config_integer("network.tunnel_port") : 0L, web,
				!config_boolean("network.online") ? "LAN only" : config_boolean("server.public") ? "public" : "by invite");
		}
	}
	server.status_file_seconds += seconds;
	if (server.status_file_seconds >= STATUS_FILE_SECONDS)
	{
		server.status_file_seconds = 0.0f;
		server_write_status_file();
		server_publish_state();
	}
	server.health_seconds += seconds;
	if (server.health_seconds >= HEALTH_SECONDS)
	{
		server.health_seconds = 0.0f;
		server_health_line();
	}
}

#else

void server_update(boolean main_menu_loaded, real seconds)
{
}

char const *server_director_first_level(void)
{
	return NULL;
}

boolean server_director_ok_to_countdown(long player_count, boolean needs_more_teams)
{
	return FALSE;
}

void server_fatal(char const *text)
{
}

boolean server_director_refuses_joins(void)
{
	return FALSE;
}

/* (the players' votes and the AFK kick ask these only of a dedicated
server: network_votekick.c, network_afk.c) */
boolean server_director_playing(void)
{
	return FALSE;
}

boolean server_director_end_game(short reason, boolean skip_postgame)
{
	return FALSE;
}

void server_director_player_removed(long machine_index, char const *reason)
{
}

#endif

/*
SERVER_HELD_MACHINE_LOADING.C (test)

A machine left out of the start for a map download
(source/networking/network_server_manager.c). One that needs the map as the
game starts is not told to load, and is marked map-pending
(network_game_server_map_pending): it joins in progress once the game has
begun (network_game_server_release_map_fetchers). The need can end before
the others have loaded (an MCC package's need ends a few seconds after its
check), and the machine was then counted as loading: the game waited out the
wait for the others (a minute), and then dropped it for timing out a load it
was never told to start. A map-pending machine is now neither loading nor
timed out; the machines told to load still are.
test_server_held_machine_loading.py takes the constants (config.inc) and the
code (under_test.inc).
*/

#include "harness.h"

#define VALID_INDEX(index, count) ((index) >= 0 && (index) < (count))
#define SET_FLAG(flags, bit, value) ((value) ? ((flags) |= FLAG(bit)) : ((flags) &= ~FLAG(bit)))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define csmemset memset
#define NETWORK_SERVER_MANAGER_FILE "server"
#undef match_assert
#define match_assert(file, line, condition) ((void)(condition))

#include "config.inc"

struct network_player
{
	signed char machine_index;
	signed char controller_index;
	signed char player_list_index;
	signed char team_index;
};

struct network_machine
{
	unsigned short name[MAXIMUM_MACHINE_NAME_LENGTH];
	signed char machine_index;
};

struct network_game
{
	struct { struct { boolean teams; } universal_variant; } variant;
	struct { boolean auto_team_balance; } variant_options;
	struct network_machine machines[MAXIMUM_NETWORK_MACHINE_COUNT];
	struct network_player players[MAXIMUM_NETWORK_PLAYER_COUNT];
};

struct network_connection { int unused; };
struct network_message { int unused; };
struct countdown_timer { long remaining; };

struct network_game_server_client_machine
{
	struct network_connection *connection;
	short machine_index;
	unsigned long flags;
};

struct network_game_server
{
	short state;
	boolean sent_start_game_message;
	long time_of_first_client_loading_completion;
	long time_of_last_keep_alive;
	struct
	{
		boolean active;
		boolean paused;
		boolean adjusted_time_this_tick;
		long last_countdown_message_time;
		struct countdown_timer timer;
	} countdown_state;
	struct network_game game;
	struct network_game_server_client_machine client_machines[MAXIMUM_NETWORK_MACHINE_COUNT];
};

struct message_server_pregame_countdown { short seconds_to_start; };
struct message_server_pregame_keep_alive { long unused; };
enum { _message_server_pregame_countdown, _message_server_pregame_keep_alive };
enum { _rejection_code_game_is_full, _rejection_code_game_is_closed };

static struct network_game_server server;
static struct network_connection connection;
static unsigned long network_game_server_client_machine_join_times[MAXIMUM_NETWORK_MACHINE_COUNT];
static boolean network_game_server_map_pending[MAXIMUM_NETWORK_MACHINE_COUNT];
static struct network_player network_game_server_held_players[MAXIMUM_NETWORK_PLAYER_COUNT];
static long network_game_server_held_player_count;

/* the clock (system_milliseconds) */
static unsigned long now;
static unsigned long system_milliseconds(void) { return now; }

/* the machines that need the map (port/linux/game/map_share.c, mcc_share.c) */
static boolean needs_map[MAXIMUM_NETWORK_MACHINE_COUNT];
static boolean map_share_machine_needs_map(long machine_index)
{
	return VALID_INDEX(machine_index, MAXIMUM_NETWORK_MACHINE_COUNT) && needs_map[machine_index];
}
static boolean map_share_machine_downloading(long machine_index) { return map_share_machine_needs_map(machine_index); }

/* no MCC conversion holds the start (port/linux/game/mcc_load_network_game.c) */
static void mcc_load_network_peer_left(long machine_index) { (void)machine_index; }
static int mcc_load_network_host_waiting(long machine_index) { (void)machine_index; return 0; }

static void network_event(char const *text, ...) { (void)text; }
static boolean wide_to_ascii(unsigned short const *wide, char *ascii, long size)
{
	(void)wide;
	snprintf(ascii, (size_t)size, "a machine");
	return TRUE;
}

/* the game starts (network_game_server_all_machines_have_loaded) */
static int started;
static void network_game_server_all_machines_have_loaded(struct network_game_server *s)
{
	started++;
	s->state = _network_game_server_state_ingame;
	s->time_of_first_client_loading_completion = 0;
}

/* a machine dropped (network_game_server_remove_client_machine_from_game) */
static int dropped[MAXIMUM_NETWORK_MACHINE_COUNT];
static boolean network_game_server_remove_client_machine_from_game(struct network_game_server *s,
	struct network_game_server_client_machine *machine)
{
	(void)s;
	dropped[machine->machine_index]++;
	network_game_server_map_pending[machine->machine_index] = FALSE;
	machine->connection = NULL;
	machine->flags = 0;
	machine->machine_index = NONE;
	return TRUE;
}

/* a machine started in progress (network_game_server_start_late_joiner), or
its held players queued to join (network_game_server_queue_player_for_addition) */
static int started_late[MAXIMUM_NETWORK_MACHINE_COUNT];
static int queued[MAXIMUM_NETWORK_MACHINE_COUNT];
static void network_game_server_start_late_joiner(struct network_game_server *s,
	struct network_game_server_client_machine *machine)
{
	(void)s;
	started_late[machine->machine_index]++;
}
static void network_game_server_queue_player_for_addition(struct network_game_server *s,
	struct network_player const *player)
{
	(void)s;
	queued[player->machine_index]++;
}

/* the lobby's (not under test here) */
static boolean network_player_is_valid(struct network_player *player)
{
	return player->machine_index >= 0 && player->controller_index >= 0;
}
static boolean network_connection_active(struct network_connection *c) { (void)c; return TRUE; }
static boolean network_game_server_client_machine_is_local(struct network_game_server *s,
	struct network_game_server_client_machine *machine)
{
	return machine == &s->client_machines[0];
}
static boolean network_game_server_machine_has_players(struct network_game_server *s, long machine_index)
{
	(void)s; (void)machine_index;
	return TRUE;
}
static boolean network_game_has_free_player_slot(struct network_game *game) { (void)game; return TRUE; }
static void network_game_server_refuse_late_joiner(struct network_game_server *s,
	struct network_game_server_client_machine *machine, short code)
{
	(void)s; (void)machine; (void)code;
}
static boolean server_ok_to_countdown(struct network_game_server *s) { (void)s; return FALSE; }
static long countdown_timer_get_time_remaining(struct countdown_timer *timer) { return timer->remaining; }
static boolean network_game_server_have_all_machines_have_precached(struct network_game_server *s) { (void)s; return FALSE; }
static boolean network_game_server_start_network_game(struct network_game_server *s) { (void)s; return TRUE; }
static void network_game_server_flush_game_data_pregame(struct network_game_server *s) { (void)s; }
static boolean network_game_server_send_game_data_pregame(struct network_game_server *s) { (void)s; return TRUE; }
static struct network_message message;
static struct network_message *create_network_game_message(short type, void const *packet, long size)
{
	(void)type; (void)packet; (void)size;
	return &message;
}
static boolean network_game_server_send_message_to_all_machines(struct network_game_server *s,
	struct network_message *m)
{
	(void)s; (void)m;
	return TRUE;
}

static void network_game_server_release_map_fetchers(struct network_game_server *s);

#include "under_test.inc"

/* the dedicated server's own machine 0, and the players' machines 1 and 2 */
enum { HOST = 0, HELD = 1, OTHER = 2 };

static void join(long index)
{
	struct network_player player = { (signed char)index, 0, NONE, 0 };

	server.client_machines[index].connection = &connection;
	server.client_machines[index].machine_index = (short)index;
	server.client_machines[index].flags = FLAG(_network_client_machine_connected_bit) |
		FLAG(_network_client_machine_validated_bit);
	server.game.machines[index].machine_index = (signed char)index;
	if (index != HOST)
		server.game.players[index] = player;
}

/* the game has started with machine 1 left out of it for a map download
(network_game_server_start_network_game) and its player held, and the
others told to load */
static void started_without(long held)
{
	long index;

	memset(&server, 0, sizeof(server));
	memset(needs_map, 0, sizeof(needs_map));
	memset(network_game_server_map_pending, 0, sizeof(network_game_server_map_pending));
	memset(dropped, 0, sizeof(dropped));
	memset(started_late, 0, sizeof(started_late));
	memset(queued, 0, sizeof(queued));
	started = 0;
	now = 1000000;
	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
		server.client_machines[index].machine_index = NONE;
	for (index = 0; index < MAXIMUM_NETWORK_PLAYER_COUNT; index++)
		memset(&server.game.players[index], NONE, sizeof(server.game.players[index]));
	join(HOST);
	join(HELD);
	if (held != NONE)
	{
		needs_map[held] = TRUE;
		network_game_server_map_pending[held] = TRUE;
		network_game_server_held_players[0] = server.game.players[held];
		network_game_server_held_player_count = 1;
		memset(&server.game.players[held], NONE, sizeof(server.game.players[held]));
	}
	else
	{
		network_game_server_held_player_count = 0;
	}
	server.state = _network_game_server_state_pregame;
	server.sent_start_game_message = TRUE;
	server.time_of_last_keep_alive = (long)now;
}

/* the host's machine loads (network_game_server_client_machine_game_loading_complete) */
static void host_loaded(void)
{
	network_game_server_client_machine_game_loading_complete(&server, &server.client_machines[HOST]);
}

/* the wait for the others runs out (network_game_server_idle_pregame_tasks) */
static void wait_runs_out(void)
{
	now += NETWORK_GAME_SERVER_MAXIMUM_WAIT_TIME_FOR_LEVEL_LOADING;
	network_game_server_idle_pregame_tasks(&server);
}

int main(int argc, char **argv)
{
	char const *name = argc > 1 ? argv[1] : "";

	if (!strcmp(name, "need-ended-not-loading"))
	{
		/* its need ends while the others load: the game starts once the
		host has loaded, not a minute later */
		started_without(HELD);
		needs_map[HELD] = FALSE;
		host_loaded();
		CHECK(started == 1 && server.state == _network_game_server_state_ingame,
			"the game waited for a machine left out of the start (%d starts)", started);
		CHECK(network_game_server_map_pending[HELD], "the held machine was forgotten");
	}
	else if (!strcmp(name, "need-ended-check-complete"))
	{
		/* the host loaded while it still needed the map, and another
		machine leaves (network_game_server_check_loading_complete) */
		started_without(HELD);
		host_loaded();
		CHECK(started == 1, "fixture: the game did not start while machine 1 needed the map");
		server.state = _network_game_server_state_pregame;
		server.time_of_first_client_loading_completion = (long)now;
		started = 0;
		needs_map[HELD] = FALSE;
		network_game_server_check_loading_complete(&server);
		CHECK(started == 1, "the game waited for a machine left out of the start");
	}
	else if (!strcmp(name, "need-ended-not-timed-out"))
	{
		/* the wait runs out with it still left out: it is not dropped for
		a load it was never told to start */
		started_without(HELD);
		server.time_of_first_client_loading_completion = (long)now;
		SET_FLAG(server.client_machines[HOST].flags, _network_client_machine_level_loaded_bit, TRUE);
		needs_map[HELD] = FALSE;
		wait_runs_out();
		CHECK(dropped[HELD] == 0, "a machine left out of the start was dropped for timing out its load");
		CHECK(started == 1, "the game did not start");
	}
	else if (!strcmp(name, "need-ended-joins-late"))
	{
		/* and once the game has begun it joins in progress, its held player
		queued to join */
		started_without(HELD);
		needs_map[HELD] = FALSE;
		network_game_server_idle_pregame_tasks(&server);
		CHECK(queued[HELD] == 0 && started_late[HELD] == 0, "it joined before the game began");
		host_loaded();
		CHECK(started == 1, "the game did not start");
		network_game_server_release_map_fetchers(&server);
		CHECK(queued[HELD] == 1 && !network_game_server_map_pending[HELD] && network_game_server_held_player_count == 0,
			"it did not join in progress (%d players queued)", queued[HELD]);
	}
	else if (!strcmp(name, "told-machine-waited-for"))
	{
		/* a machine told to load is still waited for */
		started_without(NONE);
		host_loaded();
		CHECK(started == 0, "the game started without a machine that was told to load");
		server.time_of_first_client_loading_completion = (long)now;
		network_game_server_check_loading_complete(&server);
		CHECK(started == 0, "the game started without a machine that was told to load");
	}
	else if (!strcmp(name, "told-machine-timed-out"))
	{
		/* and dropped once the wait for the others runs out */
		started_without(NONE);
		host_loaded();
		wait_runs_out();
		CHECK(dropped[HELD] == 1, "a machine that did not load in time was not dropped");
		CHECK(started == 1, "the game did not start without it");
	}
	else if (!strcmp(name, "still-downloading"))
	{
		/* one still downloading is neither waited for nor dropped, as before */
		started_without(HELD);
		host_loaded();
		CHECK(started == 1, "the game waited for a download");
		server.state = _network_game_server_state_pregame;
		server.time_of_first_client_loading_completion = (long)now;
		wait_runs_out();
		CHECK(dropped[HELD] == 0, "a machine still downloading was dropped");
	}
	else
	{
		fprintf(stderr, "no case %s\n", name);
		return 2;
	}
	return 0;
}

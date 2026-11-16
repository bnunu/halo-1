#include "harness.h"

#define PLAYERS 8
#define TICKS_PER_SECOND 30
struct player_datum
{
	boolean quit_out_of_game;
	short team_index;
	long unit_index;
	boolean out_of_lives;
};
struct data_iterator
{
	long index;
	long datum_index;
};
struct data_array;
struct game_variant_options
{
	long time_limit;
};
static struct player_datum people[PLAYERS];
static int people_count;
static struct data_array *player_data;
static struct game_variant_options options;
static int engine_running = 1, *game_engine = &engine_running;
/* (a player's host by default; the settings' defaults: port_config.c) */
static boolean infection, server_mode, host_one_team_ends = TRUE, server_one_team_ends = FALSE;
static long now_ticks;
static boolean infection_enabled(void) { return infection; }
static struct game_variant_options *game_variant_options_get(void) { return &options; }
static long game_time_get(void) { return now_ticks; }
static int halo_server_mode(void) { return server_mode; }
static int config_boolean(const char *name)
{
	if (!strcmp(name, "network.one_team_ends_game"))
		return host_one_team_ends;
	if (!strcmp(name, "server.one_team_ends_game"))
		return server_one_team_ends;
	CHECK(0, "%s read", name);
	return 0;
}
static void data_iterator_new(struct data_iterator *iterator, struct data_array *data)
{
	iterator->index = NONE;
	iterator->datum_index = NONE;
}
static void *data_iterator_next(struct data_iterator *iterator)
{
	if (++iterator->index >= people_count)
		return NULL;
	iterator->datum_index = iterator->index;
	return &people[iterator->index];
}
static short players_in_game(void)
{
	short count = 0;
	int index;

	for (index = 0; index < people_count; index++)
		if (!people[index].quit_out_of_game)
			count++;
	return count;
}
static boolean game_engine_player_is_odd_man_out(long player_index) { return FALSE; }
static boolean game_engine_player_is_out_of_lives(long player_index) { return people[player_index].out_of_lives; }
#include "under_test.inc"

/* a player of a team: alive (a unit), dead and waiting to respawn, or out of
lives */
static void add(short team, boolean alive, boolean out_of_lives)
{
	people[people_count].team_index = team;
	people[people_count].unit_index = alive ? 100 + people_count : NONE;
	people[people_count].out_of_lives = out_of_lives;
	people_count++;
}

int main(int argc, char **argv)
{
	const char *case_name = argv[1];

	CASE("stock-one-team")
	{
		/* the Xbox game's rule, the default: three red players only */
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		add(0, FALSE, FALSE);
		CHECK(game_engine_should_end_game(), "a game of one team's players went on with the rule on");
	}
	CASE("one-team-goes-on")
	{
		/* the rule off: three red players, the blue team still loading */
		host_one_team_ends = FALSE;
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		add(0, FALSE, FALSE);
		CHECK(!game_engine_should_end_game(), "a game of one team's players ended with the rule off");
	}
	CASE("other-team-left")
	{
		/* the rule off: the blue players quit */
		host_one_team_ends = FALSE;
		add(0, TRUE, FALSE);
		add(1, TRUE, FALSE);
		add(0, TRUE, FALSE);
		add(1, FALSE, FALSE);
		people[1].quit_out_of_game = TRUE;
		people[3].quit_out_of_game = TRUE;
		CHECK(!game_engine_should_end_game(), "a game its other team left ended with the rule off");
	}
	CASE("out-of-lives-ends")
	{
		/* the rule off: blue's players are all out of lives */
		host_one_team_ends = FALSE;
		add(0, TRUE, FALSE);
		add(1, FALSE, TRUE);
		add(0, FALSE, FALSE);
		add(1, FALSE, TRUE);
		CHECK(game_engine_should_end_game(), "a game whose other team was out of lives went on");
	}
	CASE("both-teams-go-on")
	{
		/* either way: both teams playing, one of them all dead and respawning */
		add(0, TRUE, FALSE);
		add(1, FALSE, FALSE);
		CHECK(!game_engine_should_end_game(), "a game of both teams ended (the rule on)");
		host_one_team_ends = FALSE;
		CHECK(!game_engine_should_end_game(), "a game of both teams ended (the rule off)");
	}
	CASE("one-player")
	{
		add(0, TRUE, FALSE);
		CHECK(!game_engine_should_end_game(), "a game of one player ended");
	}
	CASE("time-limit")
	{
		/* the rule off: one team's players still end it at the time limit */
		host_one_team_ends = FALSE;
		options.time_limit = 10;
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		now_ticks = 10L * 60 * TICKS_PER_SECOND - 1;
		CHECK(!game_engine_should_end_game(), "ended before its time limit");
		now_ticks++;
		CHECK(game_engine_should_end_game(), "went on past its time limit");
	}
	CASE("server-goes-on")
	{
		/* a dedicated server, its default: three red players, blue loading */
		server_mode = TRUE;
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		add(0, FALSE, FALSE);
		CHECK(!game_engine_should_end_game(), "a dedicated server's game of one team's players ended");
	}
	CASE("server-stock")
	{
		/* a dedicated server with server.one_team_ends_game on (the player
		host's own setting off, which it doesn't go by) */
		server_mode = TRUE;
		server_one_team_ends = TRUE;
		host_one_team_ends = FALSE;
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		CHECK(game_engine_should_end_game(), "a dedicated server's game of one team's players went on with the rule on");
	}
	CASE("server-out-of-lives-ends")
	{
		server_mode = TRUE;
		add(0, TRUE, FALSE);
		add(1, FALSE, TRUE);
		CHECK(game_engine_should_end_game(), "a dedicated server's game whose other team was out of lives went on");
	}
	CASE("infection")
	{
		infection = TRUE;
		add(0, TRUE, FALSE);
		add(0, TRUE, FALSE);
		CHECK(!game_engine_should_end_game(), "Infection's round ended by the team rule");
	}
	return 0;
}

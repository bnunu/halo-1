#include "harness.h"
#include "enums.inc"
/* The server browser's choice and order of the games found
(port/linux/src/p2p_lobby.c): synthetic listings, filtered and sorted as the
menus have them (menu_functions.c's lobby_browser_update). */
#include "under_test.inc"

static struct p2p_listing games[16];
static int count;

/* a game: its name, map, engine, players of most, dedicated, locked */
static void add(const char *name, const char *map, int engine, int players, int most, int dedicated, int locked)
{
	struct p2p_listing *game = &games[count++];

	memset(game, 0, sizeof(*game));
	snprintf(game->name, sizeof(game->name), "%s", name);
	snprintf(game->map, sizeof(game->map), "%s", map);
	game->engine_type = (unsigned char)engine;
	game->player_count = (unsigned char)players;
	game->maximum_player_count = (unsigned char)most;
	game->dedicated = (unsigned char)dedicated;
	game->master = (unsigned char)dedicated;
	game->locked = (unsigned char)locked;
	game->open = 1;
	game->ping = -1;
	game->network_version = HALO_PORT_NETWORK_VERSION;
}

/* the standard set: two dedicated servers, three player games */
static void standard(void)
{
	count = 0;
	add("Bravo dedicated", "bloodgulch", 1, 12, 16, 1, 0);
	add("alpha player", "chillout", 3, 2, 6, 0, 0);
	add("Charlie empty", "prisoner", 2, 0, 16, 0, 1);
	add("delta full", "sidewinder", 2, 8, 8, 0, 0);
	add("Echo dedicated empty", "custom_maps\\hugeass", 1, 0, 32, 1, 0);
}

static struct p2p_lobby_choices all_choices(void)
{
	struct p2p_lobby_choices choices;

	memset(&choices, 0, sizeof(choices));
	choices.source = P2P_LOBBY_SOURCE_ALL;
	choices.show_empty = choices.show_full = 1;
	choices.maps = P2P_LOBBY_MAPS_YES;
	return choices;
}

static int has(int shown, const char *name)
{
	int index;

	for (index = 0; index < shown; index++)
	{
		if (!strcmp(games[index].name, name))
			return 1;
	}
	return 0;
}

/* a Custom Edition map is missing; Hugeass can be downloaded */
static int map_state(const char *map)
{
	if (!strcmp(map, "custom_maps\\hugeass"))
		return P2P_LOBBY_MAP_DOWNLOADABLE;
	if (!strncmp(map, "custom_maps\\", 12))
		return P2P_LOBBY_MAP_MISSING;
	return P2P_LOBBY_MAP_INSTALLED;
}

static int order_is(const char *const *names, int shown)
{
	int index;

	for (index = 0; index < shown; index++)
	{
		if (strcmp(games[index].name, names[index]))
		{
			fprintf(stderr, "place %d: %s, not %s\n", index, games[index].name, names[index]);
			return 0;
		}
	}
	return 1;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	struct p2p_lobby_choices choices = all_choices();
	int shown;

	standard();
	CASE("source") {
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 5, "ALL GAMES: every game");
		standard();
		choices.source = P2P_LOBBY_SOURCE_DEDICATED;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 2 && has(shown, "Bravo dedicated") && has(shown, "Echo dedicated empty"), "DEDICATED: the servers");
		CHECK(!strcmp(games[0].name, "Bravo dedicated"), "kept in their order");
		standard();
		choices.source = P2P_LOBBY_SOURCE_PLAYERS;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 3 && !has(shown, "Bravo dedicated"), "P2P GAMES: the player-hosted");
	} else CASE("empty_full") {
		choices.show_empty = 0;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 3 && !has(shown, "Charlie empty") && !has(shown, "Echo dedicated empty"), "no empty servers");
		standard();
		choices = all_choices();
		choices.show_full = 0;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 4 && !has(shown, "delta full"), "no full servers");
		CHECK(has(shown, "Bravo dedicated"), "12 of 16 is not full");
	} else CASE("game_type") {
		choices.engine_type = 2;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 2 && has(shown, "Charlie empty") && has(shown, "delta full"), "Slayer's alone");
	} else CASE("maps") {
		add("Foxtrot", "custom_maps\\lockout", 1, 3, 16, 0, 0);
		choices.maps = P2P_LOBBY_MAPS_NO;
		shown = p2p_lobby_filter(games, count, &choices, map_state);
		CHECK(shown == 4 && !has(shown, "Echo dedicated empty") && !has(shown, "Foxtrot"), "NO: installed maps only");
		standard();
		add("Foxtrot", "custom_maps\\lockout", 1, 3, 16, 0, 0);
		choices.maps = P2P_LOBBY_MAPS_DOWNLOADABLE;
		shown = p2p_lobby_filter(games, count, &choices, map_state);
		CHECK(shown == 5 && has(shown, "Echo dedicated empty") && !has(shown, "Foxtrot"), "IF DOWNLOADABLE");
		standard();
		add("Foxtrot", "custom_maps\\lockout", 1, 3, 16, 0, 0);
		choices.maps = P2P_LOBBY_MAPS_YES;
		shown = p2p_lobby_filter(games, count, &choices, map_state);
		CHECK(shown == 6, "YES: every map");
	} else CASE("dedicated_first") {
		static const char *const expected[] = { "Bravo dedicated", "Echo dedicated empty", "delta full",
			"alpha player", "Charlie empty" };

		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_DEDICATED, 0);
		CHECK(order_is(expected, count), "dedicated servers first, then the most players, then by name");
	} else CASE("sort_each_way") {
		static const char *const names[] = { "alpha player", "Bravo dedicated", "Charlie empty", "delta full",
			"Echo dedicated empty" };
		static const char *const names_reversed[] = { "Echo dedicated empty", "delta full", "Charlie empty",
			"Bravo dedicated", "alpha player" };
		static const char *const maps[] = { "Bravo dedicated", "alpha player", "Echo dedicated empty",
			"Charlie empty", "delta full" };
		static const char *const players[] = { "Bravo dedicated", "delta full", "alpha player", "Charlie empty",
			"Echo dedicated empty" };
		static const char *const players_reversed[] = { "Echo dedicated empty", "Charlie empty", "alpha player",
			"delta full", "Bravo dedicated" };

		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_NAME, 0);
		CHECK(order_is(names, count), "by name, whatever the case");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_NAME, 1);
		CHECK(order_is(names_reversed, count), "by name, the other way");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_MAP, 0);
		CHECK(order_is(maps, count), "by map");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_PLAYERS, 0);
		CHECK(order_is(players, count), "the most players first (equal ones by name)");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_PLAYERS, 1);
		CHECK(order_is(players_reversed, count), "the fewest first");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_LOCKED, 0);
		CHECK(!strcmp(games[0].name, "Charlie empty"), "a password's first");
		p2p_lobby_sort(games, count, P2P_LOBBY_SORT_TYPE, 0);
		CHECK(games[0].engine_type == 1 && games[count - 1].engine_type == 3, "by type");
	} else CASE("other_version") {
		add("Golf newer", "bloodgulch", 1, 15, 16, 1, 0);
		games[count - 1].network_version = HALO_PORT_NETWORK_VERSION + 1;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		CHECK(shown == 6 && has(shown, "Golf newer"), "kept: shown greyed");
		p2p_lobby_sort(games, shown, P2P_LOBBY_SORT_DEDICATED, 0);
		CHECK(!strcmp(games[shown - 1].name, "Golf newer"), "last, though it has the most players");
		p2p_lobby_sort(games, shown, P2P_LOBBY_SORT_PLAYERS, 1);
		CHECK(!strcmp(games[shown - 1].name, "Golf newer"), "last the other way too");
		CHECK(!strcmp(games[0].name, "Charlie empty") || !strcmp(games[0].name, "Echo dedicated empty"),
			"the fewest first");
	} else CASE("older_in_range") {
		/* an older version this build plays with: listed as any other; one
		older than the oldest, last */
		add("Golf older", "bloodgulch", 1, 15, 16, 0, 0);
		games[count - 1].network_version = HALO_PORT_NETWORK_VERSION_OLDEST;
		add("Hotel too old", "bloodgulch", 1, 14, 16, 0, 0);
		games[count - 1].network_version = HALO_PORT_NETWORK_VERSION_OLDEST - 1;
		shown = p2p_lobby_filter(games, count, &choices, NULL);
		p2p_lobby_sort(games, shown, P2P_LOBBY_SORT_PLAYERS, 0);
		CHECK(!strcmp(games[0].name, "Golf older"), "the oldest version played with: first, with the most players");
		CHECK(!strcmp(games[shown - 1].name, "Hotel too old"), "older than the oldest: last");
		CHECK(HALO_PORT_NETWORK_VERSION_PLAYS(HALO_PORT_NETWORK_VERSION, 1) &&
			!HALO_PORT_NETWORK_VERSION_PLAYS(HALO_PORT_NETWORK_VERSION + 1, 0) &&
			HALO_PORT_NETWORK_VERSION_PLAYS(HALO_PORT_NETWORK_VERSION_OLDEST, 0) &&
			!HALO_PORT_NETWORK_VERSION_PLAYS(HALO_PORT_NETWORK_VERSION_OLDEST_MCC - 1, 1),
			"the range: this version always, a newer one never, an MCC map's from its own oldest");
	} else CASE("sort_text") {
		int reverse;
		char text[32];

		CHECK(p2p_lobby_sort_parse("players", &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse, "players");
		CHECK(p2p_lobby_sort_parse("-map", &reverse) == P2P_LOBBY_SORT_MAP && reverse, "-map: reversed");
		CHECK(p2p_lobby_sort_parse("dedicated", &reverse) == P2P_LOBBY_SORT_DEDICATED && !reverse, "dedicated");
		CHECK(p2p_lobby_sort_parse("servers", &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse,
			"servers, the old default's name: players");
		CHECK(p2p_lobby_sort_parse("-servers", &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse,
			"-servers: players, not reversed");
		CHECK(p2p_lobby_sort_parse("nonsense", &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse, "unknown: players");
		CHECK(p2p_lobby_sort_parse("-", &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse, "a lone -: players");
		CHECK(p2p_lobby_sort_parse(NULL, &reverse) == P2P_LOBBY_SORT_PLAYERS && !reverse, "none: players");
		p2p_lobby_sort_text(P2P_LOBBY_SORT_TYPE, 1, text, sizeof(text));
		CHECK(!strcmp(text, "-type"), "written back");
		p2p_lobby_sort_text(99, 0, text, sizeof(text));
		CHECK(!strcmp(text, "players"), "an unknown order written as players");
		p2p_lobby_sort_text(P2P_LOBBY_SORT_DEDICATED, 0, text, sizeof(text));
		CHECK(!strcmp(text, "dedicated"), "dedicated's name");
	} else return 2;
	return 0;
}

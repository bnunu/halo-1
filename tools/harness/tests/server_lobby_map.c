/*
SERVER_LOBBY_MAP.C (test)

A dedicated server's lobby putting its entry's map back once a second
(port/linux/game/server_director.c's server_lobby_map_lost): when the
lobby's widgets put another map back, but not once the game is starting,
nor while an MCC map waits for its package check (mcc_ui_network.c), which
setting the map again starts over. "mcc-check-finishes" plays the seconds
out: a check taking three seconds must finish and the MCC map be played.
test_server_lobby_map.py takes the code (under_test.inc).
*/

#include "harness.h"

#include <ctype.h>

static int case_compare(char const *a, char const *b)
{
	while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b))
	{
		a++;
		b++;
	}
	return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
#define csstrcasecmp case_compare

struct network_game
{
	struct
	{
		char name[128];
	} map;
};

struct network_game_server
{
	boolean start_sent;
	/* the MCC selection waiting for its check, and the seconds it has run */
	boolean pending;
	int checked_seconds;
};

static struct
{
	struct
	{
		char level[128];
	} current;
} server;

static boolean network_game_server_port_start_sent(struct network_game_server *network_server)
{
	return network_server->start_sent;
}

static boolean mcc_ui_network_map_pending(struct network_game_server *network_server)
{
	return network_server->pending;
}

#include "under_test.inc"

#define STAND_IN "levels\\test\\bloodgulch\\bloodgulch"
#define MCC_LEVEL "mcc_maps\\2906110850\\blood_covenant"
/* the seconds the package check of the MCC map takes */
#define CHECK_SECONDS 3

static struct network_game game;
static struct network_game_server network_server;

/* network_game_server_change_map_name: an MCC map waits for its check (which
starts over), shown meanwhile as the stand-in; another is set at once */
static void change_map_name(char const *name)
{
	if (!strncmp(name, "mcc_maps\\", 9))
	{
		network_server.pending = TRUE;
		network_server.checked_seconds = 0;
		return;
	}
	snprintf(game.map.name, sizeof(game.map.name), "%s", name);
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	CASE("legacy-lost")
	{
		strcpy(game.map.name, STAND_IN);
		strcpy(server.current.level, "custom_maps\\bigass_openv1.6");
		CHECK(server_lobby_map_lost(&network_server, &game), "another map put back is set again");
		return 0;
	}
	CASE("same-map")
	{
		strcpy(game.map.name, "CUSTOM_MAPS\\Bigass_Openv1.6");
		strcpy(server.current.level, "custom_maps\\bigass_openv1.6");
		CHECK(!server_lobby_map_lost(&network_server, &game), "the entry's map, in another case, is kept");
		return 0;
	}
	CASE("no-game")
	{
		strcpy(server.current.level, "custom_maps\\bigass_openv1.6");
		CHECK(!server_lobby_map_lost(&network_server, NULL), "no game: nothing to set");
		return 0;
	}
	CASE("start-sent")
	{
		strcpy(game.map.name, STAND_IN);
		strcpy(server.current.level, "custom_maps\\bigass_openv1.6");
		network_server.start_sent = TRUE;
		CHECK(!server_lobby_map_lost(&network_server, &game), "the game is starting: left alone");
		return 0;
	}
	CASE("mcc-pending")
	{
		strcpy(game.map.name, STAND_IN);
		strcpy(server.current.level, MCC_LEVEL);
		network_server.pending = TRUE;
		CHECK(!server_lobby_map_lost(&network_server, &game), "an MCC map being checked is not set again");
		return 0;
	}
	CASE("mcc-check-finishes")
	{
		int second;

		strcpy(game.map.name, STAND_IN);
		strcpy(server.current.level, MCC_LEVEL);
		change_map_name(server.current.level);
		for (second = 0; second < 10 && strcmp(game.map.name, MCC_LEVEL); second++)
		{
			/* (the director's look, once a second) */
			if (server_lobby_map_lost(&network_server, &game))
				change_map_name(server.current.level);
			/* (the check's second) */
			if (network_server.pending && ++network_server.checked_seconds >= CHECK_SECONDS)
			{
				network_server.pending = FALSE;
				snprintf(game.map.name, sizeof(game.map.name), "%s", MCC_LEVEL);
			}
		}
		CHECK(!strcmp(game.map.name, MCC_LEVEL), "a %d-second check finishes and the MCC map is the lobby's (%s)",
			CHECK_SECONDS, game.map.name);
		return 0;
	}
	fprintf(stderr, "no case %s\n", case_name);
	return 2;
}

/*
MCC_START_HOLD.C (test)

Which joined machines hold a host's MCC game start
(port/linux/game/mcc_ui_network.inl's mcc_ui_network_download_pending): one
still checking its own copy of the package does; one fetching the package
from the host (the download prompt or the transfer) does not, the start
leaving it out as it leaves a Custom Edition map's downloader out. Nothing
holds once the start is sent, outside the pregame or on a map not MCC's.
test_mcc_start_hold.py takes the code (under_test.inc).
*/

#include "harness.h"

#define MAXIMUM_NETWORK_MACHINE_COUNT 16
#define MCC_LEVEL "mcc_maps\\2906110850\\aerowalk"

enum
{
	_network_game_server_state_pregame = 1,
	_network_game_server_state_ingame
};

struct network_game_server_client_machine
{
	long machine_index;
	boolean joined;
	/* (the MCC sharing's view of it) */
	boolean active;
	boolean fetching;
};

struct network_game_server
{
	short state;
	boolean sent_start_game_message;
	struct
	{
		struct
		{
			char name[128];
		} map;
	} game;
	struct network_game_server_client_machine client_machines[MAXIMUM_NETWORK_MACHINE_COUNT];
};

static struct network_game_server host;

static struct network_game_server *global_network_game_server_get(void)
{
	return &host;
}

static boolean mcc_level_name(char const *name)
{
	return !strncmp(name, "mcc_maps\\", 9);
}

static boolean network_game_server_client_machine_is_joined_to_game(struct network_game_server *server,
	struct network_game_server_client_machine *machine)
{
	(void)server;
	return machine->joined;
}

static struct network_game_server_client_machine *machine_at(long machine_index)
{
	return machine_index >= 0 && machine_index < MAXIMUM_NETWORK_MACHINE_COUNT ? &host.client_machines[machine_index] : NULL;
}

/* (mcc_share.c: an admission lease, a transfer, or the window after joining) */
static boolean mcc_share_game_machine_active(long machine_index)
{
	struct network_game_server_client_machine *machine = machine_at(machine_index);

	return machine && (machine->active || machine->fetching);
}

/* (map_share.c: fetching the game's MCC package: the prompt or the transfer) */
static boolean map_share_machine_needs_map(long machine_index)
{
	struct network_game_server_client_machine *machine = machine_at(machine_index);

	return machine && machine->fetching;
}

#include "under_test.inc"

static void lobby(void)
{
	long index;

	memset(&host, 0, sizeof(host));
	host.state = _network_game_server_state_pregame;
	strcpy(host.game.map.name, MCC_LEVEL);
	for (index = 0; index < MAXIMUM_NETWORK_MACHINE_COUNT; index++)
		host.client_machines[index].machine_index = index;
	/* (the host's own machine, and a player's that has the package) */
	host.client_machines[0].joined = TRUE;
	host.client_machines[1].joined = TRUE;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	lobby();
	CASE("ready")
	{
		CHECK(!mcc_ui_network_download_pending(&host), "machines with the package hold nothing");
		return 0;
	}
	CASE("checking-holds")
	{
		host.client_machines[2].joined = TRUE;
		host.client_machines[2].active = TRUE;
		CHECK(mcc_ui_network_download_pending(&host), "a machine checking its own copy holds the start");
		return 0;
	}
	CASE("fetching-joins-later")
	{
		host.client_machines[2].joined = TRUE;
		host.client_machines[2].fetching = TRUE;
		CHECK(!mcc_ui_network_download_pending(&host), "a machine fetching the package does not hold the start");
		return 0;
	}
	CASE("fetching-and-checking")
	{
		host.client_machines[2].joined = TRUE;
		host.client_machines[2].fetching = TRUE;
		host.client_machines[3].joined = TRUE;
		host.client_machines[3].active = TRUE;
		CHECK(mcc_ui_network_download_pending(&host), "the one checking still holds it");
		return 0;
	}
	CASE("not-joined")
	{
		host.client_machines[2].active = TRUE;
		CHECK(!mcc_ui_network_download_pending(&host), "a machine not joined holds nothing");
		return 0;
	}
	CASE("not-mcc")
	{
		strcpy(host.game.map.name, "levels\\test\\bloodgulch\\bloodgulch");
		host.client_machines[2].joined = TRUE;
		host.client_machines[2].active = TRUE;
		CHECK(!mcc_ui_network_download_pending(&host), "a map not MCC's is not held");
		return 0;
	}
	CASE("start-sent")
	{
		host.sent_start_game_message = TRUE;
		host.client_machines[2].joined = TRUE;
		host.client_machines[2].active = TRUE;
		CHECK(!mcc_ui_network_download_pending(&host), "nothing holds once the start is sent");
		return 0;
	}
	fprintf(stderr, "no case %s\n", case_name);
	return 2;
}

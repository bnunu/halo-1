/*
CONTROLLER_REMAP_STARTUP.C (test)

The controller remapping's game context (port/linux/game/controller_remap_game.c)
asked for as input_initialize reads the pads, before game_time_initialize has
made the game time globals: no game, and game_in_progress (which reads them
without checking) not called. test_controller_remap_startup.py takes the code
(under_test.inc).
*/

#include "harness.h"

struct player_datum { short local_player_index; };

static boolean time_initialized;
static boolean in_progress;
static int in_progress_calls_uninitialized;
static unsigned char camera_context[4];
static struct player_datum players[4];

static boolean game_time_initialized(void) { return time_initialized; }
static boolean game_in_progress(void)
{
	if (!time_initialized)
		in_progress_calls_uninitialized++;
	return in_progress;
}
static short local_player_get_next(short local) { return local == NONE ? 0 : NONE; }
static long local_player_get_player_index(short local) { return local; }
static struct player_datum *player_get(long player) { return &players[player]; }
static boolean ui_widgets_active_for_local_player(short local) { return FALSE; }
static boolean main_menu_is_active(void) { return FALSE; }
static boolean console_is_active(void) { return FALSE; }
static short network_chat_controller(void) { return NONE; }

#include "under_test.inc"

int main(int argc, char **argv)
{
	char const *case_name = argv[1];
	short port;

	CHECK(argc == 2, "case");
	CASE("startup")
	{
		in_progress = TRUE;
		for (port = 0; port < 4; port++)
		{
			camera_context[port] = 2;
			CHECK(controller_remap_context(port) == 0, "port %d in a game before there is one", port);
			CHECK(camera_context[port] == 0, "port %d's camera context kept", port);
		}
		CHECK(!in_progress_calls_uninitialized, "game_in_progress called %d times before the game time globals",
			in_progress_calls_uninitialized);
	}
	else CASE("playing")
	{
		time_initialized = TRUE;
		in_progress = TRUE;
		camera_context[0] = 1;
		CHECK(controller_remap_context(0) == 2, "a player in a game: %d", controller_remap_context(0));
		in_progress = FALSE;
		CHECK(controller_remap_context(0) == 0, "no game: %d", controller_remap_context(0));
	}
	else
	{
		CHECK(0, "no case %s", case_name);
	}
	return 0;
}

#include "harness.h"

#define csmemcpy memcpy
#define csmemset memset
typedef unsigned short word;
enum { MAXIMUM_NUMBER_OF_PLAYERS = 4, _game_connection_local = 0, _game_connection_network_server = 2 };
/* stand-ins: bits 0 and 1 are latched, the others are not */
enum { LATCHED_CONTROL_FLAGS = 0x3 };
#include "constants.inc"

struct player_action
{
	unsigned long control_flags;
	struct { real yaw, pitch; } desired_facing;
	struct { real i, j; } throttle;
	real primary_trigger;
	short desired_weapon_index, desired_grenade_index, desired_zoom_level, pad;
};
struct server_update { word action_count; short pad; struct player_action actions[MAXIMUM_NUMBER_OF_PLAYERS]; };
struct update { long update_number; struct server_update update; };
struct update_server_queue_datum { short identifier, pad; long next_update_number; struct player_action current_action; };

static struct { boolean initialized; long next_update_number_to_build; struct data_array *queues; } update_server_globals;
static unsigned long update_server_pending_control_flags[MAXIMUM_NUMBER_OF_PLAYERS];
static struct { boolean valid; long tick; long received_time; } update_server_distributed_inputs[MAXIMUM_NUMBER_OF_PLAYERS];
static struct update the_update;
static struct player_action handled;
static long now;

static short game_connection(void) { return _game_connection_network_server; }
static void update_server_take_local_actions(void) {}
static struct update *update_server_get_update(long update_number) { (void)update_number; return &the_update; }
static boolean game_time_initialized(void) { return TRUE; }
static long game_time_get(void) { return now; }
static void update_client_handle_server_update(struct server_update *update, long update_number)
{
	(void)update_number;
	handled = update->actions[0];
}

#include "under_test.inc"

/* a client machine's player whose last input arrived silence ticks ago, holding a latched and a plain button */
static void next_update(long silence)
{
	struct update_server_queue_datum *queue;

	memset(&update_server_globals, 0, sizeof(update_server_globals));
	update_server_globals.initialized = TRUE;
	update_server_globals.queues = game_state_data_new("server", 1, sizeof(struct update_server_queue_datum));
	update_server_globals.queues->count = 1;
	queue = (struct update_server_queue_datum *)update_server_globals.queues->data;
	queue->identifier = (short)0x8001;
	queue->current_action.control_flags = 0x1 | 0x100;
	queue->current_action.throttle.i = 1.f;
	queue->current_action.primary_trigger = 1.f;
	now = 1000;
	update_server_distributed_inputs[0].valid = TRUE;
	update_server_distributed_inputs[0].received_time = now - silence;
	update_server_next_update();
	free(update_server_globals.queues->data);
	free(update_server_globals.queues);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("stall")
	{
		next_update(DISTRIBUTED_INPUT_SILENCE_TICKS + 1);
		CHECK(handled.control_flags == 0x1, "control flags %lx", handled.control_flags);
		CHECK(handled.throttle.i == 0.f && handled.primary_trigger == 0.f, "stalled player still moves or fires");
	}
	else CASE("held")
	{
		next_update(DISTRIBUTED_INPUT_SILENCE_TICKS - 1);
		CHECK(handled.control_flags == (0x1 | 0x100), "control flags %lx", handled.control_flags);
		CHECK(handled.throttle.i == 1.f && handled.primary_trigger == 1.f, "held input let go");
	}
	else return 2;
	return 0;
}

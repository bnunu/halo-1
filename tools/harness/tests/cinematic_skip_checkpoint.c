/*
The checkpoint a cinematic's skip goes back to: main.c's saves, game_state_save
and cinematics.c's skip functions, taken from the sources, played through a
level's script timeline one tick a frame, as main_loop runs them: the save a
frame asks for (main_save_map_private, after the frame's ticks) is taken at the
next frame's start (main_saving_map_private).
*/

#include "harness.h"

#define csmemset memset
#define TAG_STRING_LENGTH 31

static long tick;
static boolean safe = TRUE;

/* the game state the checkpoint holds: the tick and whether the cinematic
could be skipped */
static int checkpoints;
static long checkpoint_tick = NONE;
static boolean checkpoint_skippable;

#include "types.inc"

static struct cinematic_global_data cinematic_data;
static struct cinematic_global_data *cinematic_globals = &cinematic_data;

static struct
{
	boolean skip_cinematic;
	boolean saving_map;
	boolean save_map_safely;
	boolean save_map_timeout;
	boolean save_map_completed;
	long ticks_until_next_save_check;
	long ticks_unable_to_save;
	short safe_intervals;
	short switch_to_structure_bsp_index;
} main_globals;
static long main_save_last_tick;
static boolean debug_game_save;

/* game_state_save's world */
#define GAME_STATE_SIZE 1024
#define GAME_STATE_CPU_SIZE 512
static struct {boolean saved_game_valid;void *base_address;unsigned long cpu_allocation_size;} game_state_globals;
static void game_state_call_before_save_procs(void){}
static boolean hs_console_state_capture(void){return TRUE;}
static int mcc_cache_tags_loaded(void){return FALSE;}
static int mcc_checkpoint_capture(void *image,unsigned long bytes,unsigned long used,unsigned long capacity){return TRUE;}
static void main_stop_time(void){}
static void main_start_time(void){}
static void network_coop_checkpoint_saved(void){}
static void game_state_note_event(char const *note){(void)note;}
static boolean game_state_write_to_file(void)
{
	checkpoints++;
	checkpoint_tick = tick;
	checkpoint_skippable = cinematic_globals->can_be_skipped;
	return TRUE;
}

/* main.c's world */
static long game_time_get(void){return tick;}
static boolean game_time_get_paused(void){return FALSE;}
static boolean coop_scripts_safe_to_save(void){return safe;}
static void players_note_checkpoint(void){}
static void hud_autosave(boolean on){(void)on;}
#define console_printf(...) ((void)0)

#include "under_test.inc"

/* one frame of one tick (main_loop): its start, the scripts' tick, its end */
static void frame(void (*scripts)(void))
{
	if (main_globals.save_map_completed)
		main_saving_map_private();
	if (scripts)
		scripts();
	tick++;
	if (main_globals.saving_map)
		main_save_map_private();
}

static void frames(int count)
{
	while (count-- > 0)
		frame(NULL);
}

/* game_state_revert: the checkpoint's game state, then its after-load procs
(game_state.c's after_load_procs ends with cinematic_port_checkpoint) */
static void revert(void)
{
	tick = checkpoint_tick;
	cinematic_globals->can_be_skipped = checkpoint_skippable;
	cinematic_port_checkpoint();
}

/* the scenario script's (if (cinematic_skip_start) ...): skippable, then
game_save_totally_unsafe */
static void cutscene_starts(void)
{
	cinematic_skip_start();
	main_save_map_nonsafe();
}

/* one cinematic stops and the next starts, and a timeout (game_save) */
static void next_cutscene_starts(void)
{
	cinematic_skip_stop();
	cinematic_skip_start();
	main_save_map_nonsafe();
	main_save_map_safe();
}

/* a script's (game_save), or a10's (game_save_no_timeout) */
static void script_saves(void)
{
	main_save_map_safe();
}

static void new_map(void)
{
	memset(&main_globals, 0, sizeof(main_globals));
	cinematic_initialize_for_new_map();
	tick = 0;
	checkpoints = 0;
	checkpoint_tick = NONE;
	safe = TRUE;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	new_map();

	/* mercury_falling: the intro's skip checkpoint, then a continuous script's
	(game_save) a second into it; a skip must go back to the skip's own */
	CASE("passive-save")
	{
		frame(cutscene_starts);
		frame(NULL);
		CHECK(checkpoints == 1 && checkpoint_tick == 1 && checkpoint_skippable, "the skip's checkpoint (%d at %ld)",
			checkpoints, checkpoint_tick);
		frames(30);
		frame(script_saves);
		frames(300);
		CHECK(checkpoints == 1 && checkpoint_tick == 1, "the cinematic's skip would go back to tick %ld",
			checkpoint_tick);
		CHECK(main_saving_map(), "the script's save waits for the cinematic's end");
		main_skip_cinematic();
		CHECK(!main_saving_map(), "a skip lets go of it");
		revert();
		CHECK(tick == 1 && cinematic_can_be_skipped(), "the skip goes back to the cinematic's start");
		/* (the script, seeing game_reverted, skips to cinematic_skip_stop) */
		frame(cinematic_skip_stop);
		frame(script_saves);
		frames(60);
		CHECK(checkpoints == 2 && !checkpoint_skippable, "saves after the cinematic are taken (%d)", checkpoints);
		return 0;
	}

	/* b30's cutscene_map: (game_save) at the cutscene's end, before
	cinematic_skip_stop, is taken once the cinematic is over, waiting for its
	own safe checks then and not given up for the time it waited */
	CASE("save-at-the-end")
	{
		long stopped;

		frame(cutscene_starts);
		frames(600);
		frame(script_saves);
		frames(400);
		CHECK(checkpoints == 1 && checkpoint_tick == 1, "the cinematic's skip would go back to tick %ld",
			checkpoint_tick);
		frame(cinematic_skip_stop);
		stopped = tick;
		frames(5);
		CHECK(checkpoints == 1, "the save waits for its safe checks after the cinematic (taken at %ld, stopped %ld)",
			checkpoint_tick, stopped);
		frames(60);
		CHECK(checkpoints == 2 && checkpoint_tick > stopped + 20 && !checkpoint_skippable,
			"the save is taken after the cinematic (%d at %ld, stopped %ld)", checkpoints, checkpoint_tick, stopped);
		return 0;
	}

	/* an unsafe save (game_save_totally_unsafe) during the cinematic waits too */
	CASE("unsafe-save")
	{
		frame(cutscene_starts);
		frames(10);
		frame(main_save_map_nonsafe);
		frames(100);
		CHECK(checkpoints == 1 && checkpoint_tick == 1, "the cinematic's skip would go back to tick %ld",
			checkpoint_tick);
		frame(cinematic_skip_stop);
		frames(2);
		CHECK(checkpoints == 2, "taken once the cinematic is over");
		return 0;
	}

	/* the next cinematic's own checkpoint is taken, when the first stops as
	the next starts (a10's) with a save held: here a timeout save, which turns
	the next one's unsafe save into a safe one that its script waits for */
	CASE("next-cinematic")
	{
		frame(cutscene_starts);
		frames(100);
		frame(script_saves);
		frames(10);
		CHECK(checkpoints == 1 && main_saving_map(), "a save held in the first cinematic");
		frame(next_cutscene_starts);
		frames(60);
		CHECK(checkpoints == 2 && checkpoint_tick > 112 && checkpoint_skippable,
			"the second cinematic's checkpoint (%d at %ld)", checkpoints, checkpoint_tick);
		frame(script_saves);
		frames(100);
		CHECK(checkpoints == 2, "a save in the second cinematic is held (taken at %ld)", checkpoint_tick);
		return 0;
	}

	/* a checkpoint gone back to (a saved game loaded, a revert) taken while a
	cinematic could be skipped is the one its skip goes back to */
	CASE("loaded-in-a-cinematic")
	{
		frame(cutscene_starts);
		frame(NULL);
		new_map();
		checkpoint_tick = 1;
		checkpoint_skippable = TRUE;
		revert();
		frame(script_saves);
		frames(100);
		CHECK(checkpoints == 0, "a checkpoint taken in the loaded cinematic (at %ld)", checkpoint_tick);
		return 0;
	}

	/* no cinematic: saves as the game takes them */
	CASE("no-cinematic")
	{
		frame(script_saves);
		frames(40);
		CHECK(checkpoints == 1 && checkpoint_tick > 30 && checkpoint_tick < 40, "a safe save (%d at %ld)",
			checkpoints, checkpoint_tick);
		frame(main_save_map_nonsafe);
		frame(NULL);
		CHECK(checkpoints == 2, "an unsafe save");
		return 0;
	}

	fprintf(stderr, "no case %s\n", case_name);
	return 2;
}

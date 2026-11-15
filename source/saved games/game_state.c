/*
GAME_STATE.C

symbols in this file:
001AF250 0010:
	_dummy (0000)
001AF260 0010:
	_code_001af260 (0000)
001AF270 0010:
	_code_001af270 (0000)
001AF280 0020:
	_code_001af280 (0000)
001AF2A0 0010:
	_game_state_dispose (0000)
001AF2B0 00b0:
	_game_state_initialize_for_new_map (0000)
001AF360 0010:
	_game_state_dispose_from_old_map (0000)
001AF370 0020:
	_game_state_save (0000)
001AF390 0040:
	_game_state_revert (0000)
001AF3D0 0040:
	_game_state_save_to_persistent_storage (0000)
001AF410 0070:
	_game_state_test_persistent_storage (0000)
001AF480 0050:
	_game_state_save_core (0000)
001AF4D0 0020:
	_game_state_reverted (0000)
001AF4F0 0160:
	_game_state_header_valid (0000)
001AF650 0070:
	_game_state_allocation_record (0000)
001AF6C0 0020:
	_code_001af6c0 (0000)
001AF6E0 0110:
	_game_state_malloc (0000)
001AF7F0 0110:
	_game_state_gpu_malloc (0000)
001AF900 0040:
	_game_state_data_new (0000)
001AF940 0040:
	_game_state_memory_pool_new (0000)
001AF980 0050:
	_game_state_lruv_cache_new (0000)
001AF9D0 00f0:
	_game_state_try_and_load_from_persistent_storage (0000)
001AFAC0 00a0:
	_game_state_load_core (0000)
001AFB60 0050:
	_game_state_initialize (0000)
002A7E7C 0013:
	??_C@_0BD@BHHOKLIG@error?5writing?5?8?$CFs?8?$AA@ (0000)
002A7E90 000b:
	??_C@_0L@MGAFKEJL@saved?5?8?$CFs?8?$AA@ (0000)
002A7E9C 0025:
	??_C@_0CF@NHHJOMMA@checksum?5from?5map?5file?5doesn?8t?5m@ (0000)
002A7EC4 0021:
	??_C@_0CB@ELFCALCI@expected?5?$CD?$CFd?5players?5but?5got?5?$CD?$CFd@ (0000)
002A7EE8 001d:
	??_C@_0BN@CGADBPLM@allocation?5checksum?5mismatch?$AA@ (0000)
002A7F08 001b:
	??_C@_0BL@PFDNBAEN@expected?5?$CC?$CFs?$CC?5but?5got?5?$CC?$CFs?$CC?$AA@ (0000)
002A7F24 001f:
	??_C@_0BP@FJPMMOPC@expected?5build?5?$CD?$CFd?5but?5got?5?$CD?$CFd?$AA@ (0000)
002A7F44 0028:
	??_C@_0CI@KECEBGOP@c?3?2halo?2SOURCE?2saved?5games?2game_@ (0000)
002A7F6C 0013:
	??_C@_0BD@JHLLDANP@?$CF?540s?$CF?520s?$CF?510d?$CFs?6?$AA@ (0000)
002A7F80 0011:
	??_C@_0BB@DCECPIDG@d?3?2gamestate?4txt?$AA@ (0000)
002A7F98 0041:
	??_C@_0EB@JIJEJKFB@game_state_globals?4cpu_allocatio@ (0000)
002A7FDC 001b:
	??_C@_0BL@IDCLCKDD@?$CBgame_state_globals?4locked?$AA@ (0000)
002A7FF8 000a:
	??_C@_09KFGLKCMD@?$CB?$CIsize?$CG3?$CJ?$AA@ (0000)
002A8008 0041:
	??_C@_0EB@OBCPKDFA@game_state_globals?4gpu_allocatio@ (0000)
002A804C 000b:
	??_C@_0L@CIHKLGI@data?5array?$AA@ (0000)
002A8058 000b:
	??_C@_0L@HOFDEGMG@lruv?5cache?$AA@ (0000)
002A8064 0013:
	??_C@_0BD@OOCHHAFI@couldn?8t?5open?5?8?$CFs?8?$AA@ (0000)
002A8078 000c:
	??_C@_0M@NNNFMOGK@loaded?5?8?$CFs?8?$AA@ (0000)
00316840 003c:
	_data_00316840 (0000)
004D27B0 0020:
	_bss_004d27b0 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "cseries/errors.h"
#include "real_math.h"
#include "console.h"
#include "game_state.h"
#include "coop_vehicles.h" /* port: preserve replacement-vehicle homes across reverts */
#include "network_coop.h" /* port: checkpoint roster restoration and host team-wipe policy */
#include "game.h"
#include "tag_files.h"
#include "cache_files.h"
#include "scenario.h"
#include "main.h"
#include "objects.h"
#include "objects/object_types.h" /* port: object_type_definitions (the saved game's layout checksum) */
#include "game_sound.h"
#include "sound_manager.h"
#include "observer.h"
#include "players.h"
#include "player_queues_new.h"
#include "rasterizer.h"
#include "recorded_animations.h"
#include "ai_debug.h"
#include "structures.h"
#include "director.h"
#include "hud_messaging.h"
#include "crc.h"
#include "data.h"
#include "lruv_cache.h"
#include "memory_pool.h"
#include "cluster_partitions.h"
#include "cutscene/cinematics.h" /* port: cinematic_port_checkpoint */
#include "mcc_cache.h" /* port: port/linux/game/mcc_cache.h */
#include "mcc_checkpoint.h" /* port: port/linux/game/mcc_checkpoint.h */
#include "hs_console_state.h" /* port: port/linux/game/hs_console_state.inl */
/* port: object_bounds_cache.c's */
void object_bounds_cache_invalidate(void);

/* port: riding taken from the settings after every load (bipeds.c) */
void bipeds_riding_latch(void);

/* port: (object_types.c) */
extern struct object_type_definition *object_type_definitions[NUMBER_OF_OBJECT_TYPES];

void platform_log(const char *format, ...);

/* ---------- constants */

enum
{
	/* the native builds' larger game state (halo_port_capacity.h) */
	GAME_STATE_CPU_SIZE = HALO_PORT_GAME_STATE_CPU_SIZE,
	GAME_STATE_GPU_SIZE = HALO_PORT_GAME_STATE_GPU_SIZE,
	GAME_STATE_SIZE = GAME_STATE_CPU_SIZE+GAME_STATE_GPU_SIZE
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

void dummy(
	void);
static boolean game_state_header_valid(
	struct game_state_header *header,
	boolean halt_on_error);
static void game_state_allocation_record(
	const char *name,
	const char *type,
	long size,
	boolean gpu);
static void game_state_set_revert_time(
	void);

/* ---------- globals */

boolean recover_saved_games_hack;

static FILE* bss_004d27b0;

static struct
{
	void *base_address; // 0x0
	long cpu_allocation_size; // 0x4
	long gpu_allocation_size; // 0x8
	long allocation_size_checksum; // 0xC
	boolean locked; // 0x10
	boolean saved_game_valid; // 0x11
	long revert_time; // 0x14
	struct game_state_header *header; // 0x18
} game_state_globals = { 0 };

typedef void (*game_state_before_load_proc)(void);
typedef void (*game_state_after_load_proc)(void);
typedef void (*game_state_before_save_proc)(void);

static game_state_before_save_proc before_save_procs[] =
{
	dummy,
	/* port: the host's vehicle home table follows the saved object handles. */
	coop_vehicles_before_save,
};

static game_state_before_load_proc before_load_procs[] =
{
	game_sound_clear,
	/* port: where the cluster lists' references are, which the game state
	being loaded does not hold */
	cluster_partitions_port_forget,
	/* port: (the objects are put back: object_bounds_cache.c) */
	object_bounds_cache_invalidate,
	/* port: (and the inputs their functions were computed from are not
	theirs: objects.c's object_function_inputs) */
	object_function_inputs_forget,
};

static game_state_after_load_proc after_load_procs[] =
{
	scenario_reload_structure_bsp_if_necessary,
	/* port: the session roster is not part of a gameplay checkpoint. */
	network_coop_restore_players,
	sound_stop_all,
	game_sound_restore,
	observer_initialize_for_new_map,
	update_queues_reset_and_fill_with_lies,
	rasterizer_decals_update_function_pointers,
	recorded_animations_clear_debug_storage,
	ai_debug_initialize_for_new_map,
	structure_detail_objects_flush,
	game_state_set_revert_time,
	player_control_fix_for_loaded_game_state,
	director_initialize_for_saved_game,
	scripted_hud_messages_clear,
	/* port: restore matching homes or forget an unrelated disk/core load. */
	coop_vehicles_after_load,
	/* port: (vehicle_riding.c: also for a revert in the same structure) */
	bipeds_riding_latch,
	/* port: a cinematic's skip goes back to the checkpoint gone back to */
	cinematic_port_checkpoint
};

/* ---------- public code */

void dummy(
	void)
{
	return;
}

static void game_state_call_before_save_procs(
	void)
{
	game_state_before_save_proc *proc = before_save_procs;
	long i;

	for (i =NUMBEROF(before_save_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

static void game_state_call_before_load_procs(
	void)
{
	game_state_before_load_proc *proc = before_load_procs;
	long i;

	for (i =NUMBEROF(before_load_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

static void game_state_call_after_load_procs(
	void)
{
	game_state_after_load_proc *proc = after_load_procs;
	long i;

	for (i =NUMBEROF(after_load_procs); i>0; i--, proc++)
	{
		(*proc)();
	}

	return;
}

void game_state_dispose(
	void)
{
	game_state_free_buffer();
	game_state_close_file();

	return;
}

/* port: what last rewrote the game state, and when (game time), for
game_state_check_data_arrays' report */
static char const *game_state_last_event = "startup";
static long game_state_last_event_time = NONE;

static void game_state_note_event(
	char const *event)
{
	game_state_last_event = event;
	game_state_last_event_time = game_time_initialized() ? game_time_get() : NONE;
}

static void game_state_data_arrays_new_map(void);

void game_state_initialize_for_new_map(
	void)
{
	const char *name;

	/* port: (the objects are put back: object_bounds_cache.c) */
	object_bounds_cache_invalidate();
	/* port: (and the inputs their functions were computed from are not
	theirs: objects.c's object_function_inputs) */
	object_function_inputs_forget();
	game_state_note_event("map start");
	game_state_data_arrays_new_map();

	game_state_globals.locked = TRUE;
	game_state_globals.saved_game_valid = FALSE;
	game_state_globals.revert_time = NONE;

	memset(game_state_globals.header, 0, sizeof(*game_state_globals.header));

	name = tag_get_name(global_scenario_index);
	/* port: no longer than the header has room for (a map's scenario's
	name is the map's) */
	strncpy(game_state_globals.header->map_name, name, sizeof(game_state_globals.header->map_name) - 1);
	strcpy(game_state_globals.header->build_number, "01.01.14.2342");

	game_state_globals.header->player_count = player_spawn_count;
	game_state_globals.header->difficulty = game_difficulty_level_get();
	game_state_globals.header->cache_file_checksum = cache_files_get_checksum();
	game_state_globals.header->allocation_size_checksum = game_state_globals.allocation_size_checksum;

	return;
}

void game_state_dispose_from_old_map(
	void)
{
	return;
}

void game_state_save(
	void)
{
	game_state_call_before_save_procs();
	/* port: syntax outside the CPU image must accompany saved threads, and
	MCC's extra slots occupy only proven-unused CPU bytes in this image. A
	state that cannot be taken is not saved: the previous checkpoint, if
	there is one, stays the one a revert goes back to */
	if (!hs_console_state_capture() ||
		(mcc_cache_tags_loaded() && !mcc_checkpoint_capture(game_state_globals.base_address,
		GAME_STATE_SIZE, game_state_globals.cpu_allocation_size, GAME_STATE_CPU_SIZE)))
	{
		return;
	}

	main_stop_time();
	game_state_globals.saved_game_valid = (game_state_write_to_file()!=FALSE);
	/* port: never associate new players with an older or failed save. */
	if (game_state_globals.saved_game_valid)
	{
		network_coop_checkpoint_saved();
		/* port: (the one a cinematic's skip goes back to) */
		cinematic_port_checkpoint();
	}
	main_start_time();
	game_state_note_event("checkpoint saved");

	return;
}

/* port: whether game_state_revert has a saved state to go back to */
boolean game_state_port_saved_game_valid(
	void)
{
	return game_state_globals.saved_game_valid;
}

/* port: stamps the revert at the current game time again, after a network
co-op host moved its clock on past the revert (game_state_reverted
compares the two) */
void game_state_port_restamp_revert_time(
	void)
{
	game_state_globals.revert_time = game_time_get();
}

void game_state_revert(
	void)
{
	if (!game_state_globals.saved_game_valid && !recover_saved_games_hack)
	{
		main_reset_map();

		return;
	}

	game_state_note_event("checkpoint revert");
	game_state_call_before_load_procs();
	/* port: a file that is not a saved game of this build is not taken, and
	the map starts over */
	if (!game_state_read_from_file())
	{
		game_state_globals.saved_game_valid = FALSE;
		main_reset_map();
	}
	game_state_call_after_load_procs();

	return;
}

void game_state_save_to_persistent_storage(
	void)
{
	if (player_spawn_count==1)
	{
		game_state_note_event("save and quit");
		game_state_revert();
		game_state_write_to_persistent_storage(
			game_state_globals.base_address,
			&game_state_globals.header->checksum,
			sizeof(*game_state_globals.header),
			/* the whole of the native builds' larger game state */
			GAME_STATE_SIZE);
	}

	return;
}

boolean game_state_test_persistent_storage(
	char *map_name,
	short *difficulty,
	boolean *corrupted)
{
	struct game_state_header header;
	boolean success;

	if (game_state_read_header_from_persistent_storage(
		&header,
		&header.checksum,
		sizeof(*game_state_globals.header),
		/* the whole of the native builds' larger game state */
		GAME_STATE_SIZE,
		corrupted) &&
		/* port: a saved game of another build's layout is not offered (the
		level would refuse it and start from its beginning) */
		header.allocation_size_checksum == (unsigned long)game_state_globals.allocation_size_checksum)
	{
		*difficulty = header.difficulty;
		csstrncpy(map_name, header.map_name, sizeof(header.map_name));
		map_name[sizeof(header.map_name) - 1] = 0;

		success = TRUE;
	}
	else
	{
		*difficulty = _game_difficulty_level_normal;
		strcpy(map_name, "");

		success = FALSE;
	}

	return success;
}

void game_state_save_core(
	const char *name)
{
	/* port: cores share the checkpoint's console syntax ownership. */
	if (!hs_console_state_capture())
		return;
	if (mcc_cache_tags_loaded() && !mcc_checkpoint_capture(game_state_globals.base_address,
		GAME_STATE_SIZE, game_state_globals.cpu_allocation_size, GAME_STATE_CPU_SIZE))
		return;
	/* the whole of the native builds' larger game state */
	if (game_state_write_core(name, game_state_globals.base_address, GAME_STATE_SIZE))
	{
		console_printf(FALSE, "saved '%s'", name);
	}
	else
	{
		console_printf(FALSE, "error writing '%s'", name);
	}

	return;
}

boolean game_state_reverted(
	void)
{
	return (game_state_globals.revert_time==game_time_get());
}

static boolean game_state_header_valid(
	struct game_state_header *header,
	boolean halt_on_error)
{
	boolean valid = FALSE;

	if (csstrcmp(header->map_name, tag_get_name(global_scenario_index)))
	{
		if (halt_on_error)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				409,
				FALSE,
				csprintf(temporary, "expected \"%s\" but got \"%s\"", tag_get_name(global_scenario_index), header->map_name));
		}
	}
	else if (header->allocation_size_checksum != game_state_globals.allocation_size_checksum)
	{
		/* port: a different build's saved allocation layout is an expected
		refusal (including images without saved console syntax), not a crash. */
	}
	else if (header->player_count != player_spawn_count)
	{
		if (halt_on_error)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				417,
				FALSE,
				csprintf(temporary, "expected #%d players but got #%d", player_spawn_count, header->player_count));
		}
	}
	else if (header->cache_file_checksum != cache_files_get_checksum())
	{
		if (halt_on_error)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\saved games\\game_state.c",
				422,
				FALSE,
				csprintf(temporary, "checksum from map file doesn't match"));
		}
	}
	else
	{
		valid = TRUE;
	}
	/* port: why a saved game is not loaded, for the log */
	if (!valid)
	{
		error(_error_silent, "the saved game is not taken: its header is of map '%s' (this is '%s'), allocations %08lx (%08lx), %d players (%d), map checksum %08lx (%08lx)",
			header->map_name, tag_get_name(global_scenario_index),
			(unsigned long)header->allocation_size_checksum, (unsigned long)game_state_globals.allocation_size_checksum,
			header->player_count, player_spawn_count,
			(unsigned long)header->cache_file_checksum, (unsigned long)cache_files_get_checksum());
	}

	return valid;
}

static void game_state_allocation_record(
	const char *name,
	const char *type,
	long size,
	boolean gpu)
{
	// The January compiler inlines this logger into both arena allocators while
	// retaining one out-of-line copy under its private address-derived name.
	FILE *file = bss_004d27b0;

	if (!file)
	{
		file = fopen("d:\\gamestate.txt", "w");
		bss_004d27b0 = file;
	}

	if (file)
	{
		fprintf(file, "% 40s% 20s% 10d%s\n", name, type, size, gpu ? "*" : "");
		fflush(bss_004d27b0);
	}

	return;
}

static void game_state_set_revert_time(
	void)
{
	game_state_globals.revert_time = game_time_get();
	game_time_set_paused(FALSE);

	return;
}

/* port: read-only arena bounds for MCC-owned state; no allocator changes. */
void *game_state_port_cpu_region(unsigned long *used, unsigned long *capacity)
{
	*used=game_state_globals.cpu_allocation_size;
	*capacity=GAME_STATE_CPU_SIZE;
	return game_state_globals.base_address;
}

void *game_state_malloc(
	const char *name,
	const char *type,
	long size)
{
	byte *pointer;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 153, !(size&3));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 156, !game_state_globals.locked);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 159, game_state_globals.cpu_allocation_size+size<=GAME_STATE_CPU_SIZE);

	game_state_allocation_record(name, type, size, FALSE);

	pointer = (byte *)game_state_globals.base_address+game_state_globals.cpu_allocation_size;
	game_state_globals.cpu_allocation_size+= size;

	crc_checksum_buffer((unsigned long *)&game_state_globals.allocation_size_checksum, &size, sizeof(size));

	return pointer;
}

void *game_state_gpu_malloc(
	const char *name,
	const char *type,
	long size)
{
	byte *pointer;

	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 182, !(size&3));
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 185, !game_state_globals.locked);
	match_assert("c:\\halo\\SOURCE\\saved games\\game_state.c", 188, game_state_globals.gpu_allocation_size+size<=GAME_STATE_GPU_SIZE);

	game_state_allocation_record(name, type, size, TRUE);

	game_state_globals.gpu_allocation_size+= size;
	pointer = (byte *)game_state_globals.base_address-game_state_globals.gpu_allocation_size+GAME_STATE_SIZE;

	crc_checksum_buffer((unsigned long *)&game_state_globals.allocation_size_checksum, &size, sizeof(size));

	return pointer;
}


/* ---------- port: the image of a saved game

A saved game (a checkpoint's savegame.bin, a core) is the game state's
memory as it was, read back over it: with the data arrays' pointers to
their elements, the objects' memory pool's blocks and their references, and
the caches' procedures in it. The file is anyone's, so before an image is
taken, what the game state has of those is checked against what was made
at startup (the same in every run of this build: the header's allocation
checksum says so), and the procedures are put back. An image that fails is
not taken. */

enum
{
	_game_state_allocation_data,
	_game_state_allocation_memory_pool,
	_game_state_allocation_lruv_cache,

	MAXIMUM_GAME_STATE_ALLOCATIONS = 256,
};

struct game_state_allocation
{
	short kind;
	short maximum_count;
	short element_size;
	void *address;
	long size;
	long page_count;
	long page_size_bits;
	lruv_delete_block_proc delete_block_proc;
	lruv_locked_block_proc locked_block_proc;
	/* a data array's last tick in order this map (NONE: not yet), and
	whether it was reported since (game_state_check_data_arrays) */
	long good_time;
	boolean reported;
};

static struct game_state_allocation game_state_allocations[MAXIMUM_GAME_STATE_ALLOCATIONS];
static long game_state_allocation_count;

static struct game_state_allocation *game_state_allocation_new(
	short kind,
	void *address)
{
	struct game_state_allocation *allocation;

	match_assert("game_state.c", 0, game_state_allocation_count < MAXIMUM_GAME_STATE_ALLOCATIONS);
	if (game_state_allocation_count >= MAXIMUM_GAME_STATE_ALLOCATIONS)
		return NULL;
	allocation = &game_state_allocations[game_state_allocation_count++];
	csmemset(allocation, 0, sizeof(*allocation));
	allocation->kind = kind;
	allocation->address = address;

	return allocation;
}

/* whether [address, address+size) lies in the game state */
static boolean game_state_image_contains(
	void const *address,
	long size)
{
	byte const *base = game_state_globals.base_address;
	byte const *pointer = address;

	return size >= 0 && pointer >= base && pointer <= base + GAME_STATE_SIZE &&
		size <= (base + GAME_STATE_SIZE) - pointer;
}

/* where the image holds what the game state has at address */
static void *game_state_image_pointer(
	byte *image,
	void const *address)
{
	return image + ((byte const *)address - (byte const *)game_state_globals.base_address);
}

static boolean game_state_image_refuse(
	char const *name,
	char const *reason)
{
	error(2, "the saved game is not taken: %s %s", name, reason);

	return FALSE;
}

/* a data array as it was made (its pointer to its elements, its capacity)
with counts that fit */
static boolean game_state_image_data_valid(
	byte *image,
	struct data_array *live,
	short maximum_count,
	short element_size)
{
	struct data_array *data = game_state_image_pointer(image, live);
	char const *name;

	data->name[NUMBEROF(data->name) - 1] = 0;
	name = data->name;
	if (data->signature != 'd@t@')
		return game_state_image_refuse(name, "is a data array of another signature");
	if (data->maximum_count != maximum_count || data->size != element_size)
		return game_state_image_refuse(name, "is a data array of another capacity");
	if (data->data != live + 1)
		return game_state_image_refuse(name, "is a data array whose elements are elsewhere");
	if (data->count < 0 || data->count > data->maximum_count ||
		data->first_free_absolute_index < 0 || data->first_free_absolute_index > data->maximum_count ||
		data->actual_count < 0 || data->actual_count > data->count)
	{
		return game_state_image_refuse(name, "is a data array with counts past its capacity");
	}
	data->valid = data->valid != FALSE;
	data->identifier_zero_invalid = data->identifier_zero_invalid != FALSE;
	/* (one made for the map has its identifiers seeded, and the game's is
	made for this map too: data_make_valid) */
	if (data->valid && !data->next_identifier)
		return game_state_image_refuse(name, "is a data array with no identifier to give out");
	if (!data->valid && live->valid)
		return game_state_image_refuse(name, "is a data array not made for the map");

	return TRUE;
}

/* whether the reference of a block (the pointer to it that the pool keeps
up to date) is in a data array's elements */
static boolean game_state_image_reference_valid(
	void **reference)
{
	long index;

	if (!game_state_image_contains(reference, sizeof(*reference)) || ((unsigned long)reference & 3))
		return FALSE;
	for (index = 0; index < game_state_allocation_count; index++)
	{
		struct game_state_allocation const *allocation = &game_state_allocations[index];

		if (allocation->kind == _game_state_allocation_data)
		{
			byte const *first = (byte const *)((struct data_array *)allocation->address + 1);
			byte const *last = first + (long)allocation->maximum_count * allocation->element_size;

			if ((byte const *)reference >= first && (byte const *)(reference + 1) <= last)
				return TRUE;
		}
	}

	return FALSE;
}

/* a memory pool as it was made, whose blocks lie in it in order, each
referenced from a data array's element that holds the block's address */
static boolean game_state_image_memory_pool_valid(
	byte *image,
	struct memory_pool *live,
	long size)
{
	struct memory_pool *pool = game_state_image_pointer(image, live);
	byte const *first = (byte const *)(live + 1);
	byte const *last = first + size;
	struct memory_pool_block *block_live = pool->first_block;
	struct memory_pool_block *previous_live = NULL;
	long used = 0;
	long count = 0;
	char const *name;

	pool->name[NUMBEROF(pool->name) - 1] = 0;
	name = pool->name;
	if (pool->signature != 'pool')
		return game_state_image_refuse(name, "is a memory pool of another signature");
	if (pool->base_address != live + 1 || pool->size != size)
		return game_state_image_refuse(name, "is a memory pool of another size or place");
	while (block_live)
	{
		struct memory_pool_block *block;

		if ((byte const *)block_live < first || (byte const *)block_live > last - sizeof(*block) ||
			((unsigned long)block_live & 3) || ++count > size / (long)sizeof(*block))
		{
			return game_state_image_refuse(name, "is a memory pool with a block outside it");
		}
		block = game_state_image_pointer(image, block_live);
		if (block->header_signature != 'head' || block->trailer_signature != 'tail')
			return game_state_image_refuse(name, "is a memory pool with a block of another signature");
		if (block->size < (long)sizeof(*block) || (block->size & 3) || block->size > last - (byte const *)block_live)
			return game_state_image_refuse(name, "is a memory pool with a block of a bad size");
		if (block->previous_block != previous_live)
			return game_state_image_refuse(name, "is a memory pool with blocks out of order");
		if (block->next_block && (byte const *)block->next_block < (byte const *)block_live + block->size)
			return game_state_image_refuse(name, "is a memory pool with blocks that overlap");
		if (!game_state_image_reference_valid(block->reference) ||
			*(void **)game_state_image_pointer(image, block->reference) != block_live + 1)
		{
			return game_state_image_refuse(name, "is a memory pool with a block referenced from elsewhere");
		}
		used += block->size;
		previous_live = block_live;
		block_live = block->next_block;
	}
	if (pool->last_block != previous_live || pool->free_size != size - used)
		return game_state_image_refuse(name, "is a memory pool whose last block or free size is wrong");

	return TRUE;
}

/* an lruv cache as it was made, with its procedures put back */
static boolean game_state_image_lruv_cache_valid(
	byte *image,
	struct lruv_cache *live,
	struct game_state_allocation const *allocation)
{
	struct lruv_cache *cache = game_state_image_pointer(image, live);
	struct data_array *blocks_live = (struct data_array *)(live + 1);
	char const *name;

	cache->name[NUMBEROF(cache->name) - 1] = 0;
	name = cache->name;
	if (cache->signature != 'weee')
		return game_state_image_refuse(name, "is a cache of another signature");
	if (cache->blocks != blocks_live || cache->page_count != allocation->page_count ||
		cache->page_size_bits != allocation->page_size_bits)
	{
		return game_state_image_refuse(name, "is a cache of another size or place");
	}
	cache->delete_block_proc = allocation->delete_block_proc;
	cache->locked_block_proc = allocation->locked_block_proc;

	return game_state_image_data_valid(image, blocks_live, allocation->maximum_count, sizeof(struct lruv_cache_block));
}

boolean game_state_image_accept(
	void *image,
	long size)
{
	long index;

	if (size != GAME_STATE_SIZE)
		return game_state_image_refuse("the image", "is of another size");
	for (index = 0; index < game_state_allocation_count; index++)
	{
		struct game_state_allocation const *allocation = &game_state_allocations[index];
		boolean valid;

		switch (allocation->kind)
		{
		case _game_state_allocation_data:
			valid = game_state_image_data_valid(image, allocation->address, allocation->maximum_count,
				allocation->element_size);
			break;
		case _game_state_allocation_memory_pool:
			valid = game_state_image_memory_pool_valid(image, allocation->address, allocation->size);
			break;
		case _game_state_allocation_lruv_cache:
			valid = game_state_image_lruv_cache_valid(image, allocation->address, allocation);
			break;
		default:
			valid = FALSE;
			break;
		}
		if (!valid)
			return FALSE;
	}
	if (mcc_cache_tags_loaded() && !mcc_checkpoint_validate(image, size,
		game_state_globals.cpu_allocation_size, GAME_STATE_CPU_SIZE, game_state_globals.base_address))
		return FALSE;
	/* port: validate console references before publishing any saved state. */
	if (!hs_console_state_validate(image, game_state_globals.base_address, size))
		return FALSE;
	csmemcpy(game_state_globals.base_address, image, size);
	hs_console_state_restore(); /* port: restore the syntax before threads run. */
	if (mcc_cache_tags_loaded() && !mcc_checkpoint_restore(game_state_globals.base_address, size,
		game_state_globals.cpu_allocation_size, GAME_STATE_CPU_SIZE))
		return FALSE;
	error(_error_silent, "the saved game is taken");

	return TRUE;
}

struct data_array *game_state_data_new(
	const char *name,
	short maximum_count,
	short size)
{
	struct data_array *data;
	struct game_state_allocation *allocation;

	data = game_state_malloc(name, "data array", data_allocation_size(maximum_count, size));
	data_initialize(data, name, maximum_count, size);
	allocation = game_state_allocation_new(_game_state_allocation_data, data);
	if (allocation)
	{
		allocation->maximum_count = maximum_count;
		allocation->element_size = size;
		allocation->good_time = NONE;
	}

	return data;
}

/* port: a data array in order: an array (its signature, its own elements)
made valid for the map, with an identifier to give out and counts that fit
(data.c's data_usable) */
static boolean game_state_data_array_good(
	struct data_array const *data)
{
	return data->signature == 'd@t@' && data->data == (void *)(data + 1) && data->valid && data->next_identifier &&
		data->count >= 0 && data->count <= data->maximum_count &&
		data->actual_count >= 0 && data->actual_count <= data->count &&
		data->first_free_absolute_index >= 0 && data->first_free_absolute_index <= data->maximum_count;
}

static void game_state_data_arrays_new_map(
	void)
{
	long index;

	for (index = 0; index < game_state_allocation_count; index++)
	{
		game_state_allocations[index].good_time = NONE;
		game_state_allocations[index].reported = FALSE;
	}
}

/* port: each tick, every data array that was in order this map and no
longer is, reported once a map to halo.log (which a crash report carries):
when it went wrong (between its last tick in order and this one), how, and
what last rewrote the game state. A lights array found made for no map
(Sentry NATIVE-7) was the first sign of it; data.c now gives out no datum
from such an array, so this says where it came from. */
void game_state_check_data_arrays(
	void)
{
	long now = game_time_get();
	long index;

	for (index = 0; index < game_state_allocation_count; index++)
	{
		struct game_state_allocation *allocation = &game_state_allocations[index];
		struct data_array const *data = allocation->address;

		if (allocation->kind != _game_state_allocation_data)
			continue;
		if (game_state_data_array_good(data))
		{
			allocation->good_time = now;
			continue;
		}
		if (allocation->good_time == NONE || allocation->reported)
			continue;
		allocation->reported = TRUE;
		platform_log("game state: %.31s went wrong between ticks %ld and %ld of %.255s: signature %08lx, %s, "
			"next identifier %04x, count %d (%d used, first free %d) of %d; last rewritten by %s at tick %ld",
			data->name, allocation->good_time, now, game_state_globals.header->map_name,
			(unsigned long)data->signature, data->valid ? "valid" : "not valid",
			(unsigned short)data->next_identifier, data->count, data->actual_count,
			data->first_free_absolute_index, data->maximum_count, game_state_last_event,
			game_state_last_event_time);
		error(_error_silent, "game state: %.31s went wrong between ticks %ld and %ld (last rewritten by %s at tick %ld)",
			data->name, allocation->good_time, now, game_state_last_event, game_state_last_event_time);
	}
}

struct memory_pool *game_state_memory_pool_new(
	const char *name,
	long size)
{
	struct memory_pool *pool;

	pool = game_state_malloc(name, "memory pool", memory_pool_allocation_size(size));
	memory_pool_initialize(pool, name, size);
	{
		struct game_state_allocation *allocation = game_state_allocation_new(_game_state_allocation_memory_pool, pool);

		if (allocation)
			allocation->size = size;
	}

	return pool;
}

struct lruv_cache *game_state_lruv_cache_new(
	const char *name,
	long page_count,
	long page_size_bits,
	long maximum_block_count,
	void (*delete_block_proc)(long),
	boolean (*locked_block_proc)(long))
{
	struct lruv_cache *cache;

	cache = game_state_malloc(name, "lruv cache", lruv_allocation_size(maximum_block_count));
	lruv_initialize(cache, name, page_count, page_size_bits, maximum_block_count, delete_block_proc, locked_block_proc);
	{
		struct game_state_allocation *allocation = game_state_allocation_new(_game_state_allocation_lruv_cache, cache);

		if (allocation)
		{
			allocation->maximum_count = (short)maximum_block_count;
			allocation->page_count = page_count;
			allocation->page_size_bits = page_size_bits;
			allocation->delete_block_proc = delete_block_proc;
			allocation->locked_block_proc = locked_block_proc;
		}
	}

	return cache;
}

void game_state_try_and_load_from_persistent_storage(
	void)
{
	struct game_state_header header;

	if (game_state_read_header_from_persistent_storage(
			&header,
			&header.checksum,
			sizeof(header),
			GAME_STATE_SIZE,
			NULL)
		&& game_state_header_valid(&header, FALSE)
		&& (main_get_difficulty() == header.difficulty ||
			(error(_error_silent, "the saved game is not taken: it is of difficulty %d, this game of %d",
				header.difficulty, main_get_difficulty()), FALSE)))
	{
		game_state_note_event("saved game loaded");
		game_state_call_before_load_procs();
		game_state_read_from_persistent_storage(
			game_state_globals.base_address,
			GAME_STATE_SIZE);
		game_difficulty_level_set(main_get_difficulty());
		game_state_call_after_load_procs();
		game_state_save();
	}

	return;
}

void game_state_load_core(
	const char *name)
{
	struct game_state_header header;

	if (game_state_read_core_header(name, &header, sizeof(header))
		&& game_state_header_valid(&header, TRUE))
	{
		game_state_note_event("core loaded");
		game_state_call_before_load_procs();
		game_state_read_core(
			name,
			game_state_globals.base_address,
			GAME_STATE_SIZE);
		console_printf(FALSE, "loaded '%s'", name);
		game_state_call_after_load_procs();
	}
	else
	{
		console_printf(FALSE, "couldn't open '%s'", name);
	}

	return;
}

void game_state_initialize(
	void)
{
	short object_type;

	crc_new(&game_state_globals.allocation_size_checksum);
	/* port: a saved game holds each object's datum in the layout of the
	build that wrote it: one of another layout is refused, never read into
	this one's (vehicle_riding.c grew the biped's) */
	for (object_type = 0; object_type < NUMBER_OF_OBJECT_TYPES; object_type++)
	{
		if (object_type_definitions[object_type])
		{
			long datum_size = object_type_definitions[object_type]->game_datum_size;

			crc_checksum_buffer((unsigned long *)&game_state_globals.allocation_size_checksum, &datum_size,
				sizeof(datum_size));
		}
	}
	/* the native builds place their larger game state above the tag cache
	(halo_port_capacity.h, cache/physical_memory_map.c) */
	game_state_globals.base_address = game_state_allocate_buffer(HALO_PORT_GAME_STATE_BASE_ADDRESS, GAME_STATE_CPU_SIZE, GAME_STATE_GPU_SIZE);
	game_state_create_or_open_file();
	game_state_globals.header = game_state_malloc("header", NULL, sizeof(*game_state_globals.header));

	return;
}

/* ---------- private code */

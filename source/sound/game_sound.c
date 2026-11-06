/*
GAME_SOUND.C

symbols in this file:
001B6B20 0030:
	_game_sound_initialize (0000)
001B6B50 0020:
	_game_sound_dispose (0000)
001B6B70 0030:
	_game_sound_initialize_for_new_map (0000)
001B6BA0 00b0:
	_game_sound_clear (0000)
001B6C50 00d0:
	_game_sound_restore (0000)
001B6D20 0100:
	_game_looping_sound_new (0000)
001B6E20 00a0:
	_game_looping_sound_delete (0000)
001B6EC0 00b0:
	_unattached_impulse_sound_new (0000)
001B6F70 0080:
	_unspatialized_impulse_sound_new (0000)
001B6FF0 0050:
	_scripted_sound_time (0000)
001B7040 0050:
	_scripted_sound_stop (0000)
001B7090 00b0:
	_scripted_foley_predict (0000)
001B7140 0070:
	_scripted_looping_sound_set_scale (0000)
001B71B0 0050:
	_scripted_looping_sound_set_alternate (0000)
001B7200 0060:
	_unattached_looping_sound_start (0000)
001B7260 0030:
	_unattached_looping_sound_stop (0000)
001B7290 0270:
	_code_001b7290 (0000)
001B7500 00f0:
	_track_object_impulse_sound (0000)
001B75F0 0040:
	_game_sound_set_mouth_aperture (0000)
001B7630 00f0:
	_code_001b7630 (0000)
001B7720 0070:
	_code_001b7720 (0000)
001B7790 0070:
	_code_001b7790 (0000)
001B7800 0060:
	_code_001b7800 (0000)
001B7860 00d0:
	_code_001b7860 (0000)
001B7930 0030:
	_game_sound_dispose_from_old_map (0000)
001B7960 0110:
	_object_impulse_sound_new (0000)
001B7A70 0160:
	_scripted_sound_new (0000)
001B7BD0 0060:
	_scripted_looping_sound_stop (0000)
001B7C30 01d0:
	_game_sound_update (0000)
001B7E00 0200:
	_compute_sound_obstruction (0000)
001B8000 0090:
	_scripted_looping_sound_start (0000)
002AA9B4 0013:
	??_C@_0BD@LHEMKJEF@game?5sound?5globals?$AA@ (0000)
002AA9C8 0016:
	??_C@_0BG@BOAENECE@object?5looping?5sounds?$AA@ (0000)
002AA9E0 003a:
	??_C@_0DK@LEFFPEOA@TEST_FLAG?$CIsound?9?$DOflags?0?5_game_lo@ (0000)
002AAA1C 0022:
	??_C@_0CC@BGEOADFC@c?3?2halo?2SOURCE?2sound?2game_sound?4@ (0000)
002AAA40 000c:
	??_C@_0M@GMDOBMHG@marker_name?$AA@ (0000)
002AAA50 0075:
	??_C@_0HF@HCOEOKBN@definition?9?$DOruntime_scripting_so@ (0000)
002AAAC8 003f:
	??_C@_0DP@OPHINILO@definition?9?$DOruntime_scripting_so@ (0000)
002AAB08 0019:
	??_C@_0BJ@CPMEKCKK@scale?$DO?$DN0?4f?5?$CG?$CG?5scale?$DM?$DN1?4f?$AA@ (0000)
002AAB28 0044:
	??_C@_0EE@BEMDGHLB@TEST_FLAG?$CIdefinition?9?$DOflags?0?5_lo@ (0000)
002AAB6C 0010:
	??_C@_0BA@EPHFINEK@attachment_data?$AA@ (0000)
002AAB80 0064:
	??_C@_0GE@GAKNCJMB@location?9?$DOcluster_index?$DO?$DNNONE?5?$CG?$CG@ (0000)
002AABE4 0014:
	??_C@_0BE@DCLHABOE@position?5?$CG?$CG?5forward?$AA@ (0000)
002AABF8 0041:
	??_C@_0EB@JEFKCLHK@source?9?$DOspatialization_mode?$DN?$DN_so@ (0000)
002AAC3C 0030:
	??_C@_0DA@LJCJMFEJ@definition?9?$DOruntime_scripting_so@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cache/cache_files.h"
#include "cache/sound_cache.h"
#include "camera/observer.h"
#include "game/game.h"
#include "game/players.h"
#include "memory/data.h"
#include "objects/objects.h"
#include "physics/collisions.h"
#include "physics/collision_usage.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "sound/game_sound.h"
#include "sound/sound_classes.h"
#include "sound/sound_definitions.h"
#include "sound/sound_manager.h"
#include "structures/structure_bsp_definitions.h"
#include "structures/structures.h"
#include "units/units.h"

/* ---------- constants */

#define MAXIMUM_CLUSTER_SOUND_DISTANCE 256.f

enum
{
	/* the native builds' larger pool of object looping sounds (halo_port_capacity.h) */
	MAXIMUM_GAME_LOOPING_SOUNDS = HALO_PORT_MAXIMUM_GAME_LOOPING_SOUNDS,
	_cluster_sound_distance_unreachable_bit = 7,
	CLUSTER_SOUND_DISTANCE_VALUE_MASK = 0x7F,
	_game_looping_sound_unattached_bit = 0,
	_game_looping_sound_unattached_stop_bit = 1,
	_game_looping_sound_unattached_stop_fixed_fadeout_bit = 2,
	_game_looping_sound_alternate_bit = 3,
	_game_looping_sound_scripted_bit = 4,
};

enum game_looping_sound_state
{
	_game_looping_sound_active,
	_game_looping_sound_deactivating,
	_game_looping_sound_inactive,
	NUMBER_OF_GAME_LOOPING_SOUND_STATES,
};

enum sound_spatialization_mode
{
	_sound_spatialization_mode_none,
	_sound_spatialization_mode_absolute,
	_sound_spatialization_mode_relative,
	NUMBER_OF_SOUND_SPATIALIZATION_MODES,
};

/* ---------- macros */

#define game_looping_sound_get(index) \
	((struct game_looping_sound_datum *)datum_get(game_looping_sound_data, (index)))

/* ---------- structures */

struct sound_source
{
	short spatialization_mode;
	short pad_2;
	real scale;
	real gain;
	struct sound_location location;
	real obstruction;
	real occlusion;
};

struct sound_attachment_data
{
	short function_index;
	short node_index;
	real_point3d position;
	real_vector3d forward;
};

struct game_looping_sound_datum
{
	struct datum_header header;
	short state;
	unsigned long flags;
	real scale;
	long definition_index;
	long object_index;
	long last_audible_frame_index;
	short function_index;
	short node_index;
	real_point3d position;
	real_vector3d forward;
};

struct game_sound_globals
{
	long update_index;
	long music_looping_sound_index;
};

typedef char game_looping_sound_datum_size_assert[
	sizeof(struct game_looping_sound_datum) == 0x34 ? 1 : -1];
typedef char game_sound_globals_size_assert[
	sizeof(struct game_sound_globals) == 0x8 ? 1 : -1];
typedef char sound_source_size_assert[
	sizeof(struct sound_source) == 0x40 ? 1 : -1];
typedef char sound_attachment_data_size_assert[
	sizeof(struct sound_attachment_data) == 0x1C ? 1 : -1];
typedef char sound_attachment_data_node_index_offset_assert[
	offsetof(struct sound_attachment_data, node_index) == 2 ? 1 : -1];

/* ---------- prototypes */

static void compute_combined_pas(
	void);
static boolean location_potentially_audible(
	struct location *location);
static void update_potentially_audible_looping_sound(
	long looping_sound_index,
	struct location const *location);
static boolean looping_sound_definition_is_music(
	long definition_index);
static void scripted_music_stop_all(
	void);
static void scripted_looping_sound_stop_internal(
	long sound_index,
	boolean fixed_fadeout);
/* ---------- globals */

struct game_sound_globals *game_sound_globals;
struct data_array *game_looping_sound_data;
unsigned long combined_pas[BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CLUSTERS_PER_STRUCTURE)];

/* ---------- public code */

void game_sound_initialize(
	void)
{
	game_looping_sound_data = game_state_data_new(
		"object looping sounds",
		MAXIMUM_GAME_LOOPING_SOUNDS,
		sizeof(struct game_looping_sound_datum));
	game_sound_globals = (struct game_sound_globals *)game_state_malloc(
		"game sound globals",
		NULL,
		sizeof(*game_sound_globals));

	return;
}

void game_sound_dispose(
	void)
{
	if (game_looping_sound_data)
	{
		game_looping_sound_data = NULL;
	}

	return;
}

void game_sound_initialize_for_new_map(
	void)
{
	if (game_looping_sound_data)
	{
		data_make_valid(game_looping_sound_data);
		game_sound_globals->music_looping_sound_index = NONE;
		game_sound_globals->update_index = 0;
	}

	return;
}

void game_sound_dispose_from_old_map(
	void)
{
	if (game_looping_sound_data && game_looping_sound_data->valid)
	{
		game_sound_clear();
		data_make_invalid(game_looping_sound_data);
	}

	return;
}

void game_sound_clear(
	void)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound =
			game_looping_sound_get(looping_sound_index);
		struct looping_sound_definition *definition =
			looping_sound_definition_get(sound->definition_index);

		if (definition->runtime_scripting_sound_index == looping_sound_index)
		{
			match_assert(
				"c:\\halo\\SOURCE\\sound\\game_sound.c",
				176,
				TEST_FLAG(sound->flags, _game_looping_sound_scripted_bit));
			definition->runtime_scripting_sound_index = NONE;
		}
		else if (definition->runtime_scripting_sound_index != NONE)
		{
			game_looping_sound_get(definition->runtime_scripting_sound_index);
		}
	}

	return;
}

void game_sound_restore(
	void)
{
	long looping_sound_index;
	struct tag_iterator iterator;
	long sound_definition_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound =
			game_looping_sound_get(looping_sound_index);

		if (TEST_FLAG(sound->flags, _game_looping_sound_scripted_bit))
		{
			struct looping_sound_definition *definition =
				looping_sound_definition_get(sound->definition_index);

			if (!TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit))
			{
				definition->runtime_scripting_sound_index = looping_sound_index;
			}
			else
			{
				datum_delete(game_looping_sound_data, looping_sound_index);
			}
		}
	}

	tag_iterator_new(&iterator, SOUND_DEFINITION_TAG);
	for (sound_definition_index = tag_iterator_next(&iterator);
		sound_definition_index != NONE;
		sound_definition_index = tag_iterator_next(&iterator))
	{
		sound_definition_get(sound_definition_index)->scripting_time = NONE;
	}

	return;
}

long game_looping_sound_new(
	long object_index,
	long definition_index,
	char const *marker_name,
	short function_index)
{
	struct object_marker marker;
	long looping_sound_index = NONE;
	struct game_looping_sound_datum *sound;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		240,
		marker_name);

	if (definition_index != NONE
		&& (object_index == NONE
			|| object_get_marker_by_name(object_index, marker_name, &marker, 1)))
	{
		looping_sound_index = datum_new(game_looping_sound_data);
		if (looping_sound_index != NONE)
		{
			sound = game_looping_sound_get(looping_sound_index);
			sound->object_index = object_index;
			sound->definition_index = definition_index;
			sound->state = _game_looping_sound_inactive;
			sound->flags = 0;
			sound->function_index = function_index;
			sound->last_audible_frame_index = NONE;
			if (object_index != NONE)
			{
				sound->node_index = marker.node_index;
				sound->position = marker.node_matrix.position;
				sound->forward = marker.node_matrix.forward;
			}
		}
	}

	return looping_sound_index;
}

void game_looping_sound_delete(
	long looping_sound_index)
{
	struct game_looping_sound_datum *sound =
		game_looping_sound_get(looping_sound_index);
	struct looping_sound_definition *definition =
		looping_sound_definition_get(sound->definition_index);

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		281,
		definition->runtime_scripting_sound_index!=looping_sound_index);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		282,
		definition->runtime_scripting_sound_index==NONE || game_looping_sound_get(definition->runtime_scripting_sound_index));
	datum_delete(game_looping_sound_data, looping_sound_index);

	return;
}

long object_impulse_sound_new(
	long object_index,
	long definition_index,
	short node_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real scale)
{
	struct sound_attachment_data attachment_data;
	struct sound_source source;
	long sound_index = NONE;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		301,
		position && forward);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		302,
		scale>=0.f && scale<=1.f);

	attachment_data.position = *position;
	attachment_data.forward = *forward;
	source.spatialization_mode = _sound_spatialization_mode_absolute;
	source.gain = 1.f;
	attachment_data.node_index = node_index;
	source.location.game_location.cluster_index = NONE;
	if (track_object_impulse_sound(object_index, &attachment_data, &source))
	{
		source.scale = scale;
		sound_index = sound_new_impulse(
			definition_index,
			&source,
			object_index,
			track_object_impulse_sound,
			&attachment_data,
			sizeof(attachment_data));
	}

	return sound_index;
}

long unattached_impulse_sound_new(
	long definition_index,
	struct sound_location const *location,
	real scale)
{
	struct sound_source source;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		329,
		location);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		330,
		scale>=0.f && scale<=1.f);

	source.location = *location;
	source.scale = scale;
	source.spatialization_mode = _sound_spatialization_mode_absolute;
	source.gain = 1.f;

	return sound_new_impulse(
		definition_index,
		&source,
		NONE,
		NULL,
		NULL,
		0);
}

long unspatialized_impulse_sound_new(
	long definition_index,
	real scale)
{
	struct sound_source source;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		346,
		scale>=0.f && scale<=1.f);

	source.spatialization_mode = _sound_spatialization_mode_none;
	source.scale = scale;
	source.gain = 1.f;

	return sound_new_impulse(
		definition_index,
		&source,
		NONE,
		NULL,
		NULL,
		0);
}

/* port: a co-op host sends its scripts' sounds to its clients
(port/linux/game/network_coop.c; the kinds are in network_distributed.h) */
enum
{
	_coop_sound_impulse,
	_coop_sound_looping_start,
	_coop_sound_looping_stop,
};
void network_coop_note_sound(short kind, long definition_index, long object_index, real scale);

void scripted_sound_new(
	long definition_index,
	long source_object_index,
	real scale)
{
	struct sound_definition *definition;
	struct object_marker marker;
	real_vector3d forward;
	real_point3d position;
	long impulse_sound_index;

	if (definition_index != NONE)
	{
		network_coop_note_sound(_coop_sound_impulse, definition_index, source_object_index, scale);
		definition = sound_definition_get(definition_index);
		sound_stop_impulse(definition->scripting_sound_index);
		definition->scripting_time = game_time_get()
			+ (long)definition->longest_permutation_length * TICKS_PER_SECOND / 1000;
		scale = PIN(scale, 0.f, 1.f);
		if (source_object_index != NONE)
		{
			short node_index;

			if (object_get_marker_by_name(source_object_index, "head", &marker, 1))
			{
				node_index = marker.node_index;
				position = marker.node_matrix.position;
				forward = marker.node_matrix.forward;
			}
			else
			{
				node_index = 0;
				position = *global_origin3d;
				forward = *global_forward3d;
			}
			impulse_sound_index = object_impulse_sound_new(
				source_object_index,
				definition_index,
				node_index,
				&position,
				&forward,
				scale);
			if (impulse_sound_index != NONE)
			{
				object_type_notify_impulse_sound(
					source_object_index,
					definition_index,
					impulse_sound_index);
			}
		}
		else
		{
			impulse_sound_index = unspatialized_impulse_sound_new(definition_index, scale);
		}
		definition->scripting_sound_index = impulse_sound_index;
	}

	return;
}

long scripted_sound_time(
	long sound_index)
{
	struct sound_definition *definition;
	long time = 0;

	if (sound_index != NONE)
	{
		definition = sound_definition_get(sound_index);
		if (definition->scripting_time != NONE)
		{
			long remaining = definition->scripting_time - game_time_get();

			time = MAX(remaining, 0);
		}
	}

	return time;
}

void scripted_sound_stop(
	long sound_index)
{
	struct sound_definition *definition;

	if (sound_index != NONE)
	{
		definition = sound_definition_get(sound_index);
		if (definition->scripting_sound_index != NONE)
		{
			sound_stop_impulse(definition->scripting_sound_index);
			definition->scripting_sound_index = NONE;
			definition->scripting_time = NONE;
		}
	}

	return;
}

void scripted_foley_predict(
	long definition_index)
{
	struct looping_sound_definition *definition;
	short track_index;

	if (definition_index != NONE)
	{
		definition = looping_sound_definition_get(definition_index);
		for (track_index = 0; track_index < definition->tracks.count; track_index++)
		{
			struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(
				&definition->tracks,
				track_index,
				struct looping_sound_track);

			if (track->loop_sound.index != NONE)
			{
				struct sound_definition *sound = sound_definition_get(track->loop_sound.index);

				if (sound->pitch_ranges.count == 1)
				{
					struct sound_pitch_range *pitch_range = TAG_BLOCK_GET_ELEMENT(
						&sound->pitch_ranges,
						0,
						struct sound_pitch_range);

					if (pitch_range->permutations.count)
					{
						struct sound_permutation *permutation = TAG_BLOCK_GET_ELEMENT(
							&pitch_range->permutations,
							0,
							struct sound_permutation);

						_sound_cache_sound_request(permutation, FALSE, TRUE, FALSE);
					}
				}
			}
		}
	}

	return;
}

void scripted_looping_sound_set_scale(
	long sound_index,
	real scale)
{
	struct looping_sound_definition *definition;
	struct game_looping_sound_datum *looping_sound;

	if (sound_index != NONE)
	{
		definition = looping_sound_definition_get(sound_index);
		if (definition->runtime_scripting_sound_index != NONE)
		{
			looping_sound = game_looping_sound_get(definition->runtime_scripting_sound_index);
			looping_sound->scale = PIN(scale, 0.0f, 1.0f);
		}
	}

	return;
}

void scripted_looping_sound_set_alternate(
	long sound_index,
	boolean alternate)
{
	struct looping_sound_definition *definition;
	struct game_looping_sound_datum *looping_sound;

	if (sound_index != NONE)
	{
		definition = looping_sound_definition_get(sound_index);
		if (definition->runtime_scripting_sound_index != NONE)
		{
			looping_sound = game_looping_sound_get(definition->runtime_scripting_sound_index);
			SET_FLAG(looping_sound->flags, _game_looping_sound_alternate_bit, alternate);
		}
	}

	return;
}

void unattached_looping_sound_stop(
	long looping_sound_index)
{
	struct game_looping_sound_datum *looping_sound =
		game_looping_sound_get(looping_sound_index);

	SET_FLAG(looping_sound->flags, _game_looping_sound_unattached_stop_bit, TRUE);

	return;
}

static void scripted_looping_sound_stop_internal(
	long sound_index,
	boolean fixed_fadeout)
{
	struct looping_sound_definition *definition;
	struct game_looping_sound_datum *looping_sound;

	if (sound_index != NONE)
	{
		definition = looping_sound_definition_get(sound_index);
		if (definition->runtime_scripting_sound_index != NONE)
		{
			looping_sound = game_looping_sound_get(definition->runtime_scripting_sound_index);
			SET_FLAG(looping_sound->flags, _game_looping_sound_scripted_bit, FALSE);
			unattached_looping_sound_stop(definition->runtime_scripting_sound_index);
			definition->runtime_scripting_sound_index = NONE;
			if (fixed_fadeout)
			{
				SET_FLAG(
					looping_sound->flags,
					_game_looping_sound_unattached_stop_fixed_fadeout_bit,
					TRUE);
			}
		}
	}

	return;
}

void scripted_looping_sound_stop(
	long sound_index)
{
	if (sound_index != NONE)
		network_coop_note_sound(_coop_sound_looping_stop, sound_index, NONE, 0.0f);
	scripted_looping_sound_stop_internal(sound_index, FALSE);

	return;
}

void scripted_looping_sound_start(
	long sound_index,
	long source_object_index,
	real gain)
{
	struct looping_sound_definition *definition;
	long looping_sound_index;
	struct game_looping_sound_datum *looping_sound;

	if (sound_index != NONE)
	{
		definition = looping_sound_definition_get(sound_index);
		scripted_looping_sound_stop(sound_index);
		/* after the stop above, so clients get them in the same order */
		network_coop_note_sound(_coop_sound_looping_start, sound_index, source_object_index, gain);
		match_assert(
			"c:\\halo\\SOURCE\\sound\\game_sound.c",
			495,
			definition->runtime_scripting_sound_index==NONE);
		if (TEST_FLAG(definition->flags, _looping_sound_stops_music_bit))
		{
			scripted_music_stop_all();
		}
		looping_sound_index = unattached_looping_sound_start(
			sound_index,
			source_object_index,
			gain);
		definition->runtime_scripting_sound_index = looping_sound_index;
		if (looping_sound_index != NONE)
		{
			looping_sound = game_looping_sound_get(looping_sound_index);
			SET_FLAG(looping_sound->flags, _game_looping_sound_scripted_bit, TRUE);
		}
	}

	return;
}

long unattached_looping_sound_start(
	long definition_index,
	long source_object_index,
	real scale)
{
	long looping_sound_index;
	struct game_looping_sound_datum *looping_sound;

	looping_sound_definition_get(definition_index);
	looping_sound_index = game_looping_sound_new(
		source_object_index,
		definition_index,
		"",
		NONE);
	if (looping_sound_index != NONE)
	{
		looping_sound = game_looping_sound_get(looping_sound_index);
		SET_FLAG(looping_sound->flags, _game_looping_sound_unattached_bit, TRUE);
		looping_sound->scale = scale;
	}

	return looping_sound_index;
}

void game_sound_update(
	real dt)
{
	long background_sound_definition_index;
	struct sound_environment_definition const *sound_environment;
	boolean crossed_water_boundary;
	long looping_sound_index;
	struct location location;

	scenario_get_sound_environment(
		&background_sound_definition_index,
		&sound_environment,
		&crossed_water_boundary);
	sound_manager_set_sound_environment(sound_environment);
	compute_combined_pas();

	if (background_sound_definition_index == NONE)
	{
		if (game_sound_globals->music_looping_sound_index != NONE)
		{
			unattached_looping_sound_stop(game_sound_globals->music_looping_sound_index);
			game_sound_globals->music_looping_sound_index = NONE;
		}
	}
	else if (game_sound_globals->music_looping_sound_index != NONE)
	{
		struct game_looping_sound_datum *sound =
			game_looping_sound_get(game_sound_globals->music_looping_sound_index);

		if (sound->definition_index != background_sound_definition_index)
		{
			unattached_looping_sound_stop(game_sound_globals->music_looping_sound_index);
			game_sound_globals->music_looping_sound_index = unattached_looping_sound_start(
				background_sound_definition_index,
				NONE,
				1.f);
		}
	}
	else
	{
		game_sound_globals->music_looping_sound_index = unattached_looping_sound_start(
			background_sound_definition_index,
			NONE,
			1.f);
	}

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);

		if (sound->object_index == NONE)
		{
			update_potentially_audible_looping_sound(looping_sound_index, NULL);
		}
		else if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit)
			&& !object_try_and_get(sound->object_index))
		{
			struct looping_sound_definition *definition =
				looping_sound_definition_get(sound->definition_index);

			if (definition->runtime_scripting_sound_index == looping_sound_index)
			{
				definition->runtime_scripting_sound_index = NONE;
			}
			game_looping_sound_delete(looping_sound_index);
		}
		else
		{
			struct object_datum *object = object_get(sound->object_index);

			if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit))
			{
				object_get_location(sound->object_index, &location);
				if (location_potentially_audible(&location))
				{
					update_potentially_audible_looping_sound(looping_sound_index, &location);
				}
			}
		}
	}

	game_sound_globals->update_index++;

	return;
}

void game_sound_set_mouth_aperture(
	long object_index,
	real mouth_aperture)
{
	if (game_looping_sound_data->valid)
	{
		if (object_try_and_get_and_verify_type(object_index, _object_mask_unit))
		{
			unit_set_mouth_aperture(object_index, mouth_aperture);
		}
	}

	return;
}

/* The native ports draw several frames per game tick (port/linux/game/
render_interpolation.c), and the sound manager refreshes every sound each
frame; on the Xbox, frames and ticks were one. A sound's obstruction, a
collision test from the camera, is computed once per tick and reused by the
tick's other frames. */
#define OBSTRUCTION_CACHE_SLOTS 128

static struct
{
	boolean valid;
	long game_time;
	short local_player_index;
	real_point3d position;
	real obstruction;
	real occlusion;
} obstruction_cache[OBSTRUCTION_CACHE_SLOTS];

static long obstruction_cache_slot(
	short local_player_index,
	real_point3d const *position)
{
	unsigned long const *words = (unsigned long const *)position;

	return (long)(((words[0] * 73856093UL) ^ (words[1] * 19349663UL) ^ (words[2] * 83492791UL) ^
		(unsigned long)local_player_index) % OBSTRUCTION_CACHE_SLOTS);
}

void compute_sound_obstruction(
	short local_player_index,
	struct sound_source *source,
	real distance)
{
	struct observer_result const *camera = observer_get_camera(local_player_index);
	long slot = obstruction_cache_slot(local_player_index, &source->location.position);
	long now = game_time_get();

	if (obstruction_cache[slot].valid &&
		obstruction_cache[slot].game_time == now &&
		obstruction_cache[slot].local_player_index == local_player_index &&
		obstruction_cache[slot].position.x == source->location.position.x &&
		obstruction_cache[slot].position.y == source->location.position.y &&
		obstruction_cache[slot].position.z == source->location.position.z)
	{
		source->obstruction = obstruction_cache[slot].obstruction;
		source->occlusion = obstruction_cache[slot].occlusion;
		return;
	}

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		882,
		global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] =
		_collision_user_sounds;

	source->obstruction = 0.6f;
	source->occlusion = 1.0f;
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		887,
		source->spatialization_mode==_sound_spatialization_mode_absolute);

	if (source->location.game_location.cluster_index != NONE &&
		camera->location.cluster_index != NONE)
	{
		struct structure_bsp *structure_bsp = global_structure_bsp_get();
		byte encoded_distance = structure_bsp_get_cluster_encoded_sound_distance(
			structure_bsp,
			camera->location.cluster_index,
			source->location.game_location.cluster_index);
		real cluster_distance = (encoded_distance & ~FLAG(_cluster_sound_distance_unreachable_bit))
			* (MAXIMUM_CLUSTER_SOUND_DISTANCE / CLUSTER_SOUND_DISTANCE_VALUE_MASK);

		if (cluster_distance < MAXIMUM_CLUSTER_SOUND_DISTANCE)
		{
			if (BIT_VECTOR_TEST_FLAG(
				structure_bsp_get_cluster_pvs(
					global_structure_bsp_get(),
					camera->location.cluster_index),
				source->location.game_location.cluster_index))
			{
				real_vector3d listener_to_source;
				struct collision_result collision;

				source->obstruction = 0.45f;
				vector_from_points3d(
					&camera->position,
					&source->location.position,
					&listener_to_source);
				if (!collision_test_vector(
					FLAG(_collision_test_front_facing_surfaces_bit) |
						FLAG(_collision_test_structure_bit) |
						FLAG(_collision_test_media_bit) |
						FLAG(_collision_test_objects_bit) |
						FLAG(_collision_test_objects_scenery_bit) |
						FLAG(_collision_test_objects_machines_bit),
					&camera->position,
					&listener_to_source,
					NONE,
					&collision))
				{
					source->obstruction = 0.0f;
					source->occlusion = 0.0f;
				}
			}

			if (source->obstruction != 0.0f)
			{
				source->occlusion = 1.0f - distance / (cluster_distance + distance);
				source->occlusion = PIN(source->occlusion * 1.4f, 0.0f, 1.0f);
			}
		}
	}

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		926,
		global_current_collision_user_depth > 1);
	--global_current_collision_user_depth;
	obstruction_cache[slot].valid = TRUE;
	obstruction_cache[slot].game_time = now;
	obstruction_cache[slot].local_player_index = local_player_index;
	obstruction_cache[slot].position = source->location.position;
	obstruction_cache[slot].obstruction = source->obstruction;
	obstruction_cache[slot].occlusion = source->occlusion;

	return;
}

/* ---------- private code */

static void compute_combined_pas(
	void)
{
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short local_player_index;

	csmemset(
		combined_pas,
		0,
		BIT_VECTOR_SIZE_IN_BYTES(structure_bsp->clusters.count));

	for (local_player_index = 0;
		local_player_index < MAXIMUM_LOCAL_PLAYERS;
		local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			struct observer_result const *camera = observer_get_camera(local_player_index);

			if (camera->location.cluster_index != NONE)
			{
				short cluster_index;

				for (cluster_index = 0;
					cluster_index < structure_bsp->clusters.count;
					cluster_index++)
				{
					real distance = (structure_bsp_get_cluster_encoded_sound_distance(
							structure_bsp,
							cluster_index,
							camera->location.cluster_index) & ~FLAG(_cluster_sound_distance_unreachable_bit))
						* (MAXIMUM_CLUSTER_SOUND_DISTANCE / CLUSTER_SOUND_DISTANCE_VALUE_MASK);

					if (distance < MAXIMUM_CLUSTER_SOUND_DISTANCE)
					{
						BIT_VECTOR_SET_FLAG(combined_pas, cluster_index, TRUE);
					}
				}
			}
		}
	}

	return;
}

static boolean location_potentially_audible(
	struct location *location)
{
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		870,
		location->cluster_index>=NONE && location->cluster_index<global_structure_bsp_get()->clusters.count);

	return location->cluster_index != NONE
		&& BIT_VECTOR_TEST_FLAG(combined_pas, location->cluster_index);
}

static boolean looping_sound_definition_is_music(
	long definition_index)
{
	struct looping_sound_definition *definition =
		looping_sound_definition_get(definition_index);
	short track_index;

	for (track_index = 0; track_index < definition->tracks.count; track_index++)
	{
		struct looping_sound_track *track = TAG_BLOCK_GET_ELEMENT(
			&definition->tracks,
			track_index,
			struct looping_sound_track);

		if (track->loop_sound.index != NONE
			&& sound_definition_get(track->loop_sound.index)->sound_class == _sound_class_music)
		{
			return TRUE;
		}
	}

	return FALSE;
}

boolean track_object_impulse_sound(
	long object_index,
	void const *attachment_data,
	struct sound_source *source)
{
	struct object_datum *object = object_try_and_get(object_index);
	struct sound_attachment_data const *attachment = attachment_data;
	struct location location;
	real_matrix4x3 const *node_matrix;

	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		794,
		attachment_data);
	match_assert(
		"c:\\halo\\SOURCE\\sound\\game_sound.c",
		795,
		source);

	if (object)
	{
		object_get_location(object_index, &location);
		if (location.cluster_index != NONE)
		{
			node_matrix = object_get_node_matrix(
				object_index,
				attachment->node_index == NONE ? 0 : attachment->node_index);
			source->location.game_location = location;
			matrix4x3_transform_point(
				node_matrix,
				&attachment->position,
				&source->location.position);
			matrix4x3_transform_normal(
				node_matrix,
				&attachment->forward,
				&source->location.forward);
			/* port: the attachment's velocity, not only the object's */
			object_get_point_velocity(
				object_index,
				&source->location.position,
				&source->location.translational_velocity);

			return TRUE;
		}
	}

	return FALSE;
}

static void update_potentially_audible_looping_sound(
	long looping_sound_index,
	struct location const *location)
{
	struct game_looping_sound_datum *sound = game_looping_sound_get(looping_sound_index);
	struct looping_sound_definition *definition =
		looping_sound_definition_get(sound->definition_index);
	boolean serviced_last_frame =
		sound->last_audible_frame_index == NONE
		|| sound->last_audible_frame_index == game_sound_globals->update_index - 1;
	boolean audible;
	struct sound_source source;

	if (!TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
	{
		audible = object_get_function_value(
			sound->object_index,
			sound->function_index,
			&source.scale);
	}
	else
	{
		audible = !TEST_FLAG(sound->flags, _game_looping_sound_unattached_stop_bit);
		source.scale = sound->scale;
	}

	if (audible ||
		(sound->state != _game_looping_sound_inactive && serviced_last_frame))
	{
		if (sound->object_index != NONE)
		{
			real_matrix4x3 const *node_matrix = object_get_node_matrix(
				sound->object_index,
				sound->node_index);

			match_assert(
				"c:\\halo\\SOURCE\\sound\\game_sound.c",
				619,
				location);
			matrix4x3_transform_point(node_matrix, &sound->position, &source.location.position);
			matrix4x3_transform_normal(node_matrix, &sound->forward, &source.location.forward);
			/* port: the attachment's velocity, not only the object's */
			object_get_point_velocity(
				sound->object_index,
				&source.location.position,
				&source.location.translational_velocity);
			source.location.game_location = *location;
			source.spatialization_mode = _sound_spatialization_mode_absolute;
		}
		else
		{
			source.spatialization_mode = _sound_spatialization_mode_none;
		}
		source.gain = 1.f;

		if (audible)
		{
			short refresh_state;

			if (sound->state == _game_looping_sound_active || !serviced_last_frame)
			{
				refresh_state = _looping_sound_refresh_loop;
				sound->state = _game_looping_sound_active;
			}
			else
			{
				refresh_state = _looping_sound_refresh_start;
				sound->state = _game_looping_sound_active;
			}

			if (sound_refresh_looping(
				sound->definition_index,
				looping_sound_index,
				&source,
				refresh_state,
				TEST_FLAG(sound->flags, _game_looping_sound_alternate_bit),
				0.f))
			{
				match_assert(
					"c:\\halo\\SOURCE\\sound\\game_sound.c",
					657,
					TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit));
				if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
				{
					if (definition->runtime_scripting_sound_index == looping_sound_index)
					{
						definition->runtime_scripting_sound_index = NONE;
					}
					game_looping_sound_delete(looping_sound_index);
				}
				else
				{
					sound->state = _game_looping_sound_inactive;
				}
			}
		}
		else if (!serviced_last_frame || sound_refresh_looping(
			sound->definition_index,
			looping_sound_index,
			&source,
			_looping_sound_refresh_stop,
			TEST_FLAG(sound->flags, _game_looping_sound_alternate_bit),
			TEST_FLAG(sound->flags, _game_looping_sound_unattached_stop_fixed_fadeout_bit) ? 4.f : 0.f))
		{
			if (TEST_FLAG(sound->flags, _game_looping_sound_unattached_bit))
			{
				game_looping_sound_delete(looping_sound_index);
			}
			else
			{
				sound->state = _game_looping_sound_inactive;
			}
		}
		else
		{
			sound->state = _game_looping_sound_deactivating;
		}
	}
	else if (sound->state != _game_looping_sound_inactive)
	{
		sound->state = _game_looping_sound_inactive;
	}

	sound->last_audible_frame_index = game_sound_globals->update_index;

	return;
}

static void scripted_music_stop_all(
	void)
{
	long looping_sound_index;

	for (looping_sound_index = data_next_index(game_looping_sound_data, NONE);
		looping_sound_index != NONE;
		looping_sound_index = data_next_index(game_looping_sound_data, looping_sound_index))
	{
		struct game_looping_sound_datum *looping_sound =
			game_looping_sound_get(looping_sound_index);

		if (looping_sound->object_index == NONE
			&& looping_sound_definition_is_music(looping_sound->definition_index))
		{
			scripted_looping_sound_stop_internal(looping_sound->definition_index, TRUE);
		}
	}

	return;
}

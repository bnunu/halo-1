/*
OBJECTS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "objects.h"

#include "damage.h"
#include "light_definitions.h"
#include "object_lights.h"
#include "object_types.h"

#include "ai/ai_debug.h"
#include "bitmaps/bitmaps.h"
#include "cache/predicted_resources.h"
#include "cseries/errors.h"
#include "cseries/profile.h"
#include "devices/device_definitions.h"
#include "devices/devices.h"
#include "editor/editor_stubs.h"
#include "effects/contrail_definitions.h"
#include "effects/contrails.h"
#include "effects/effect_definitions.h"
#include "effects/effects.h"
#include "effects/particle_system_definitions.h"
#include "effects/particle_systems.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "items/weapons.h"
#include "main/console.h"
#include "math/periodic_functions.h"
#include "memory/memory_pool.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "models/models.h"
#include "objects/widgets/widgets.h"
#include "physics/collision_bsp.h"
#include "physics/collision_model_definitions.h"
#include "physics/collision_models.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "physics/physics.h"
#include "physics/physics_definitions.h"
#include "render/render_debug.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "sound/sound_definitions.h"
#include "sound/game_sound.h"
#include "structures/structure_bsp_definitions.h"
#include "units/units.h"

/* ---------- constants */


/* ---------- macros */

#define MAXIMUM_DUMPS 1024


// This is dangerous, bungie returns the same value regardless of whether the index is valid
#define OBJECT_INCOMING_FUNCTION_GET_VALUE(object, index)	\
((index)>=NUMBER_OF_INCOMING_OBJECT_FUNCTIONS+1 ?			\
(object)->object.incoming_function_values[(index)-1] :		\
(object)->object.incoming_function_values[(index)-1])

#define OBJECT_FRAME_INDEX_GET(object_index) ((object_index) + game_time_get())

/* ---------- structures */

struct object_globals
{
	boolean initial_placement;
	boolean object_marker_initialized;
	boolean force_garbage_collection;
	short active_garbage_object_count;
	long first_garbage_object_index;
	unsigned long last_active_cluster_bits[16];
	unsigned long active_cluster_bits[16];
	long last_garbage_warn_time;
	short pvs_activation_type;
	union
	{
		long object_index;
		short cluster_index;
	} pvs_activation;
};

struct object_memory_release_function
{
	void(*init_function)(unsigned char *, short);
	boolean(*function)(char *, unsigned char *, unsigned char *, short);
};

struct dump_datum
{
	long definition_index;
	short object_type;
	short maximum_size;
	long total_size;
	short count;
	short active_count;
	short garbage_count;
	short dead_count;
	short outside_map_count;
	short at_rest_count;
};

/* ---------- prototypes */

static void object_connect_lights(long object_index, boolean disconnect, boolean reconnect);
static void object_name_list_allocate(void);
static void object_name_list_free(void);
static void object_name_list_clear(void);
static void object_name_list_new(long object_index, short name_index);
static void object_name_list_delete(long object_index);
static long object_name_list_lookup(short name_index);
static long recursive_object_adder(
	long object_index,
	boolean(*add_object_function)(long, void *),
	void *custom_data,
	long object_count,
	long maximum_count,
	long *object_indices);
static long sort_dumps(struct dump_datum const *dump1, struct dump_datum const *dump2);
static void object_add_to_dump(long object_index, struct dump_datum *dump);
static void object_dump_write(FILE *stream, struct dump_datum *dump);
static short object_find_region_permutations_available_with_variant(struct model_region *region, short variant_number, short *available_indices);
static boolean object_select_random_region_permutations_by_variant(long object_index, struct model *model, short variant_number);
static short object_determine_variant_number(long object_index, struct model *model);
static void object_choose_random_region_permutations(long object_index);
static void object_add_to_list(long *first_object_reference, long object_index);
static void object_remove_from_list(long *first_object_reference, long object_index);
static void attachments_delete(long object_index);
static void object_create_attachments(long object_index);
static void object_delete_attachments(long object_index);
static void object_delete_recursive(long object_index, boolean delete_siblings);
static void object_compute_function_values(long object_index);
static void object_compute_change_colors(long object_index);

/* ---------- globals */

const struct object_memory_release_function object_memory_release_procs[3] =
{
	{ NULL, ai_release_inactive_swarms },
	{ ai_find_inactive_encounters, ai_release_inactive_encounters },
	{ NULL, NULL }
};
static long *object_name_list = NULL;
static struct memory_pool *object_memory_pool = NULL;
static struct object_globals *object_globals = NULL;

boolean debug_objects_names;
boolean debug_objects_pathfinding_spheres;
boolean debug_objects_physics;
boolean debug_objects_root_node;

long global_object_marker;

struct cluster_partition noncollideable_object_cluster_partition;
struct cluster_partition collideable_object_cluster_partition;

boolean debug_object_garbage_collection;

struct data_array *object_header_data;

boolean debug_objects_position_velocity = FALSE;

/* ---------- public code */

long cluster_get_first_noncollideable_object(
	long *reference_index,
	short cluster_index)
{
	return cluster_partition_get_first_datum(
		&noncollideable_object_cluster_partition,
		reference_index,
		cluster_index);
}

long cluster_get_next_noncollideable_object(
	long *reference_index)
{
	return cluster_partition_get_next_datum(
		&noncollideable_object_cluster_partition,
		reference_index);
}

long cluster_get_first_collideable_object(
	long *reference_index,
	short cluster_index)
{
	return cluster_partition_get_first_datum(
		&collideable_object_cluster_partition,
		reference_index,
		cluster_index);
}

long cluster_get_next_collideable_object(
	long *reference_index)
{
	return cluster_partition_get_next_datum(
		&collideable_object_cluster_partition,
		reference_index);
}

short object_get_next_cluster(
	struct object_cluster_iterator *iterator,
	long object_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1049,
		iterator->cluster_partition==&collideable_object_cluster_partition || iterator->cluster_partition==&noncollideable_object_cluster_partition);

	return cluster_partition_get_next_cluster(iterator->cluster_partition, &iterator->reference_index);
}

void *object_try_and_get_and_verify_type(
	long object_index,
	unsigned long valid_type_flags)
{
	struct object_header_datum* header = object_header_try_and_get(object_index);
	void *result = NULL;

	if (header)
	{
		if (TEST_FLAG(valid_type_flags, header->type))
		{
			result = header->datum;
		}
	}

	return result;
}

void *object_get_and_verify_type(
	long object_index,
	unsigned long valid_type_flags)
{
	struct object_datum *result = object_header_get(object_index)->datum;

	match_vassert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1690,
		TEST_FLAG(valid_type_flags, result->object.type),
		csprintf(
			temporary,
			"got an object type we didn't expect (expected one of 0x%08x but got #%d).",
			valid_type_flags,
			result->object.type));

	return result;
}

void object_iterator_new(
	struct object_iterator *iterator,
	unsigned long type_flags,
	byte flags)
{
	data_verify(object_header_data);
	iterator->signature = OBJECT_ITERATOR_SIGNATURE;
	iterator->type_flags = type_flags;
	iterator->flags = flags;
	iterator->absolute_index = 0;
	iterator->index = NONE;

	return;
}

void *object_iterator_next(
	struct object_iterator *iterator)
{
	short abs_index;
	struct object_header_datum *header;
	void *result = NULL;

	match_vassert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1720,
		iterator->signature==OBJECT_ITERATOR_SIGNATURE,
		"uninitialized iterator passed to object_iterator_next()");

	data_verify(object_header_data);
	
	abs_index = iterator->absolute_index;

	// The operation to get this header is inlined in the original code?
	header = (struct object_header_datum *)((char*)object_header_data->data + sizeof(struct object_header_datum) * abs_index);

	while (abs_index<object_header_data->count)
	{
		long index = DATUM_INDEX_NEW(abs_index++, header->identifier);
		if (header->identifier)
		{
			if ((iterator->flags&header->flags)==iterator->flags &&
				TEST_FLAG(iterator->type_flags, header->type))
			{
				iterator->index = index;
				result = header->datum;
				break;
			}
		}
		++header;
	}
	iterator->absolute_index = abs_index;

	return result;
}

long object_get_ultimate_parent(
	long object_index)
{
	long result = NONE;
	struct object_datum *object;
	for (;object_index!=NONE; object_index = object->object.parent_object_index)
	{
		result = object_index;
		object = object_get(object_index);
	}

	return result;
}

void object_add_scenario_permutation(
	long object_index,
	struct scenario_object_permutation *permutation)
{
	return;
}

void object_set_object_index_for_name_index(
	short name_index,
	long object_index)
{
	if (VALID_INDEX(name_index, global_scenario_get()->object_names.count))
	{
		object_name_list[name_index] = object_index;
	}

	return;
}

void objects_fix_for_deleted_object(
	long deleted_object_index)
{
	struct object_datum *object;

	struct object_iterator iterator;
	object_iterator_new(&iterator, _object_mask_all, 0);

	for (object = (struct object_datum *)object_iterator_next(&iterator);
		object;
		object = (struct object_datum *)object_iterator_next(&iterator))
	{
		if (object->object.umbrella_shield_object_index==deleted_object_index)
		{
			object->object.umbrella_shield_object_index = NONE;
		}
		object_type_handle_deleted_object(iterator.index, deleted_object_index);
	}

	return;
}

void object_set_garbage(
	long object_index,
	boolean garbage)
{
	struct object_datum *garbage_object;
	struct object_datum* object = object_get(object_index);
	
	long garbage_object_index;
	for (garbage_object_index = object_globals->first_garbage_object_index;
		garbage_object_index!=NONE;
		garbage_object_index = garbage_object->object.next_garbage_object_index)
	{
		garbage_object = object_get(garbage_object_index);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			1952,
			TEST_FLAG(garbage_object->object.flags, _object_garbage_bit));
	}

	if (garbage)
	{
		if (!TEST_FLAG(object->object.flags, _object_garbage_bit))
		{
			if (!TEST_FLAG(object->object.flags, _object_cannot_be_garbage_bit))
			{
				object->object.next_garbage_object_index = object_globals->first_garbage_object_index;
				object_globals->first_garbage_object_index = object_index;
				SET_FLAG(object->object.flags, _object_garbage_bit, TRUE);
			}
		}
	}
	else
	{
		if (TEST_FLAG(object->object.flags, _object_garbage_bit))
		{
			long *garbage_object_index;
			for (garbage_object_index = &object_globals->first_garbage_object_index;
				*garbage_object_index!=object_index;
				garbage_object_index = &object_get(*garbage_object_index)->object.next_garbage_object_index)
			{
			}

			*garbage_object_index = object->object.next_garbage_object_index;
			object->object.next_garbage_object_index = NONE;
			SET_FLAG(object->object.flags, _object_garbage_bit, FALSE);
		}
	}

	for (garbage_object_index = object_globals->first_garbage_object_index;
		garbage_object_index!=NONE;
		garbage_object_index = garbage_object->object.next_garbage_object_index)
	{
		garbage_object = object_get(garbage_object_index);
		match_assert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			2006,
			TEST_FLAG(garbage_object->object.flags, _object_garbage_bit));
	}

	return;
}

void garbage_collect_now(
	void)
{
	object_globals->force_garbage_collection = TRUE;

	return;
}

void objects_information_get(
	struct objects_information *information)
{
	short i;
	struct object_header_datum *header;

	memset(information, 0, sizeof(*information));
	
	header = (struct object_header_datum *)object_header_data->data;
	for (i = 0; i<object_header_data->count; header++)
	{
		if (header->identifier)
		{
			information->object_count++;
			if (TEST_FLAG(header->flags, _object_header_active_bit))
			{
				information->active_object_count++;
			}
		}

		i++;
	}
	
	information->used_memory = 1.f - (real)memory_pool_get_contiguous_free_size(object_memory_pool) / (real)OBJECT_MEMORY_POOL_SIZE;
	
	return;
}

void object_pvs_set_object(
	long object_index)
{
	if (object_index == NONE)
	{
		object_globals->pvs_activation_type = _pvs_activation_normal;
	}
	else
	{
		object_globals->pvs_activation_type = _pvs_activation_object;
		object_globals->pvs_activation.object_index = object_index;
	}

	return;
}

void object_pvs_set_camera_point(
	short camera_point_index)
{
	if (camera_point_index == NONE)
	{
		object_globals->pvs_activation_type = _pvs_activation_normal;
	}
	else
	{
		struct location location;
		struct scenario_cutscene_camera_point *point = (struct scenario_cutscene_camera_point *)TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->cutscene_camera_points,
			camera_point_index,
			struct scenario_cutscene_camera_point);

		scenario_location_from_point(&location, &point->position);
		if (location.cluster_index==NONE)
		{
			error(
				_error_silent,
				"object_pvs_set_camera_point: camera point %s is outside the map",
				point->name);
			object_globals->pvs_activation_type = FALSE;
		}
		else
		{
			object_globals->pvs_activation_type = _pvs_activation_cluster;
			object_globals->pvs_activation.cluster_index = location.cluster_index;
		}
	}

	return;
}

void object_pvs_clear(void)
{
	object_globals->pvs_activation_type = _pvs_activation_normal;

	return;
}

short objects_get_activating_cluster_index(
	void)
{
	struct object_header_datum *header;
	short result = NONE;

	switch (object_globals->pvs_activation_type)
	{
	case _pvs_activation_cluster:
		result = object_globals->pvs_activation.cluster_index;
		break;
	case _pvs_activation_object:
	{
		long index = object_globals->pvs_activation.object_index;

		header = object_header_try_and_get(index);
		if (header && FLAG(header->type) && header->datum)
		{
			struct object_datum *parent_object = object_get(object_get_ultimate_parent(object_globals->pvs_activation.object_index));

			if (TEST_FLAG(parent_object->object.flags, _object_connected_to_map_bit))
			{
				if (parent_object->object.location.cluster_index!=NONE)
				{
					match_assert(
						"c:\\halo\\SOURCE\\objects\\objects.c",
						2279,
						parent_object->object.location.cluster_index>=0 &&
						parent_object->object.location.cluster_index<global_structure_bsp_get()->clusters.count);
					result = parent_object->object.location.cluster_index;
				}
			}
		}
		else
		{
			object_globals->pvs_activation_type = _pvs_activation_normal;
		}
		break;
	}
	default:
		break;
	}

	return result;
}

void object_definition_predict(
	long definition_index)
{
	if (definition_index!=NONE)
	{
		struct object_definition *object_definition = object_definition_get(definition_index);
		predicted_resources_precache(&object_definition->object.predicted_resources);
	}

	return;
}

void object_predict(
	long object_index)
{
	while (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);

		object_definition_predict(object->definition_index);
		object_predict(object->object.first_child_object_index);
		
		object_index = object->object.next_object_index;
	}

	return;
}

void object_beautify(
	long object_index,
	boolean beautiful)
{
	if (object_index!=NONE)
	{
		SET_FLAG(object_get(object_index)->object.flags, _object_movie_star_bit, beautiful);
	}

	return;
}

void *object_header_block_get(
	long object_index,
	struct object_header_block_reference *reference)
{
	struct object_header_datum *object_header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2443, reference->offset>0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2444, reference->offset+reference->size<=object_header->data_size);

	return (byte *)object + reference->offset;
}

static long object_header_new(
	struct data_array *data, 
	long index,
	short size)
{
	index = (index==NONE ? datum_new(data) : datum_new_at_index(data, index));
	
	if (index!=NONE)
	{
		struct object_header_datum *header = (struct object_header_datum *)datum_get(data, index);
		if (memory_pool_block_allocate(object_memory_pool, (void**)&header->datum, size))
		{
			header->data_size = size;
			memset(header->datum, 0, size);
		}
		else
		{
			datum_delete(data, index);
			index = NONE;
		}
	}

	return index;
}

static void object_header_delete(
	struct data_array *data,
	long object_index)
{
	struct object_header_datum *object_header = (struct object_header_datum *)datum_get(data, object_index);

	if (object_header->datum)
	{
		memory_pool_block_free(object_memory_pool, (void**)&object_header->datum);
	}

	datum_delete(data, object_index);
	object_header->datum = NULL;
	object_header->flags = 0;

	return;
}

boolean object_header_block_allocate(
	long object_index,
	short block_reference_offset,
	short size)
{
	boolean result;
	struct object_header_datum *object_header = object_header_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2459, size>=0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2460, object_header->data_size+size<=SHORT_MAX);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2462, block_reference_offset>=0);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2463, block_reference_offset+sizeof(struct object_header_block_reference)<=object_header->data_size);

	if (memory_pool_block_reallocate(
		object_memory_pool,
		(void **)&object_header->datum,
		size + object_header->data_size))
	{
		struct object_header_block_reference *block;
		short original_size = object_header->data_size;

		object_header->data_size+= size;
		block = (struct object_header_block_reference *)((byte *)object_get(object_index) + block_reference_offset);
		
		block->offset = original_size;
		block->size = size;

		memset(((byte *)object_header->datum) + block->offset, 0, size);
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

void object_scripting_set_collideable(
	long object_index,
	boolean collideable)
{
	if (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);
		SET_FLAG(object->object.flags, _object_no_collisions_bit, !collideable);
	}

	return;
}

void object_marker_begin(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3503, !object_globals->object_marker_initialized);

	++global_object_marker;
	object_globals->object_marker_initialized = TRUE;

	return;
}

void object_marker_end(
	void)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3514, object_globals->object_marker_initialized);

	object_globals->object_marker_initialized = FALSE;

	return;
}

boolean object_unmarked_function(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	boolean result;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3526, object_globals->object_marker_initialized);

	result = object->object.magic_number!=global_object_marker;
	
	return result;
}

boolean object_mark_function(
	long object_index)
{
	boolean result;
	struct object_datum *object = object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3543, object_globals->object_marker_initialized);

	if (object->object.magic_number!=global_object_marker)
	{
		object->object.magic_number = global_object_marker;
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	return result;
}

void objects_place(
	void)
{
	object_globals->initial_placement = TRUE;
	object_types_place_all(global_scenario_get());
	object_globals->initial_placement = FALSE;

	return;
}

long find_objects_from_point_vector(
	real_point3d const *position,
	real_vector3d const *direction, 
	boolean (*add_object_function)(long, void *),
	void *custom_data,
	long maximum_object_count,
	long *object_indices)
{
	long state = 0;

	if (scenario_leaf_index_from_point(position)!=NONE)
	{
		short cluster_index = TAG_BLOCK_GET_ELEMENT(
			&global_structure_bsp_get()->leaves,
			scenario_leaf_index_from_point(position) & LONG_MAX,
			struct structure_leaf)->cluster_index;

		if (cluster_index!=NONE)
		{
			unsigned long *cluster_pvs;
			short i;
			short bit_vector_size;

			object_marker_begin();

			cluster_pvs = structure_bsp_get_cluster_pvs(global_structure_bsp_get(), cluster_index);
			bit_vector_size = BIT_VECTOR_SIZE_IN_LONGS(global_structure_bsp_get()->clusters.count);

			for (i = 0; i<bit_vector_size; ++i)
			{
				if (cluster_pvs[i])
				{
					short offset = (short)(i << 5);
					short size = MIN(offset + LONG_BITS, global_structure_bsp_get()->clusters.count);
					short j;

					for (j = offset; j<size; ++j)
					{
						if (BIT_VECTOR_TEST_FLAG(cluster_pvs, j))
						{
							long reference;
							long k;
							for (k = cluster_get_first_collideable_object(&reference, j);
								k!=NONE;
								k = cluster_get_next_collideable_object(&reference))
							{
								if (object_mark_function(k))
								{
									state = recursive_object_adder(
										k,
										add_object_function,
										custom_data,
										state,
										maximum_object_count,
										object_indices);
								}
							}
						}
					}
				}
			}

			object_marker_end();
		}
	}

	return state;
}

void objects_dump_memory(
	void)
{
	struct dump_datum dumps[MAXIMUM_DUMPS];
	struct dump_datum dumps_by_type[NUMBER_OF_OBJECT_TYPES];
	struct object_iterator iterator;

	struct object_datum *object;
	struct object_header_datum *header;
	FILE *file;
	short object_type;
	short object_num;

	short dump_count = 0;
	short overflowed_count = 0;

	memset(dumps, 0, sizeof(dumps));
	memset(dumps_by_type, 0, sizeof(dumps_by_type));
	
	for (object_type = 0; object_type<NUMBER_OF_OBJECT_TYPES; ++object_type)
	{
		dumps_by_type[object_type].object_type = object_type;
		dumps_by_type[object_type].definition_index = NONE;
	}
	
	object_iterator_new(&iterator, _object_mask_all, 0);
	
	while (object = (struct object_datum *)object_iterator_next(&iterator))
	{
		short index = NONE;
		
		for (object_num = 0; object_num<dump_count; ++object_num)
		{
			if (dumps[object_num].definition_index==object->definition_index)
			{
				index = object_num;
				break;
			}
		}

		if (index==NONE)
		{
			if (dump_count<(short)NUMBEROF(dumps))
			{
				index = dump_count++;
				dumps[index].object_type = NONE;
				dumps[index].definition_index = object->definition_index;
			}
			else
			{
				++overflowed_count;
			}
		}

		header = object_header_get(iterator.index);

		if (index!=NONE)
		{
			object_add_to_dump(iterator.index, &dumps[index]);
		}

		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4905, (header->type >= 0) && (header->type < NUMBER_OF_OBJECT_TYPES));
		object_add_to_dump(iterator.index, &dumps_by_type[header->type]);
	}

	qsort(
		dumps,
		dump_count,
		sizeof(struct dump_datum),
		(int(__cdecl *)(const void *, const void *))sort_dumps);
	qsort(
		dumps_by_type,
		NUMBEROF(dumps_by_type),
		sizeof(struct dump_datum),
		(int(__cdecl *)(const void *, const void *))sort_dumps);

	file = fopen("d:\\object_memory.txt", "a+b");
	if (file)
	{
		{
			struct objects_information information;

			objects_information_get(&information);

			fprintf(
				file,
				"#%d objects (#%d active) using %3.2f%% of available memory\n\n",
				information.object_count,
				information.active_object_count,
				100.f * information.used_memory);
		}
		
		fprintf(file, "OBJECTS BY TYPE\n");
		fprintf(file, "number (active) [garbage/   dead/outside/at-rest] maxsize totsize\n");
		for (object_type = 0; object_type<NUMBER_OF_OBJECT_TYPES; object_type++)
		{
			object_dump_write(file, &dumps_by_type[object_type]);
		}

		fprintf(file, "\n");
		fprintf(file, "OBJECTS BY DEFINITION\n");
		fprintf(file, "number (active) [garbage/   dead/outside/at-rest] maxsize totsize\n");
		for (object_num = 0; object_num<dump_count; object_num++)
		{
			object_dump_write(file, &dumps[object_num]);
		}

		fprintf(file, "\n");

		if (overflowed_count>0)
		{
			fprintf(
				file,
				"WARNING: overflowed MAXIMUM_DUMPS (%d), this dump does not include %d objects that would not fit!\n",
				MAXIMUM_DUMPS,
				overflowed_count);
		}

		fprintf(file, "\n");
		fclose(file);
	}

	return;
}

void objects_initialize(
	void)
{
	damage_initialize();
	widgets_initialize();
	object_types_initialize();
	lights_initialize();

	if (!game_in_editor())
	{
		object_header_data = game_state_data_new("object", MAXIMUM_OBJECTS_PER_MAP, sizeof(struct object_header_datum));
		object_memory_pool = game_state_memory_pool_new("objects", OBJECT_MEMORY_POOL_SIZE);
	}
	else
	{
		object_header_data = data_new("object", MAXIMUM_OBJECTS_PER_MAP*5, sizeof(struct object_header_datum));
		object_memory_pool = memory_pool_new("objects", OBJECT_MEMORY_POOL_SIZE*5);
	}
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 216, object_header_data && object_memory_pool);

	object_globals = (struct object_globals *)game_state_malloc("object globals", NULL, sizeof(*object_globals));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 219, object_globals);

	object_name_list_allocate();
	cluster_partition_new(&collideable_object_cluster_partition, "collideable object");
	cluster_partition_new(&noncollideable_object_cluster_partition, "noncollideable object");

	return;
}

void objects_initialize_for_new_map(
	void)
{
	damage_initialize_for_new_map();
	widgets_initialize_for_new_map();
	object_types_initialize_for_new_map();
	lights_initialize_for_new_map();
	
	data_make_valid(object_header_data);
	object_name_list_clear();

	cluster_partition_make_valid(&collideable_object_cluster_partition);
	cluster_partition_make_valid(&noncollideable_object_cluster_partition);
	memset(object_globals->last_active_cluster_bits, 0, sizeof(object_globals->last_active_cluster_bits));
	memset(object_globals->active_cluster_bits, 0, sizeof(object_globals->active_cluster_bits));
	object_globals->pvs_activation_type = 0;
	object_globals->object_marker_initialized = FALSE;
	global_object_marker = 0;
	object_globals->first_garbage_object_index = NONE;
	object_globals->active_garbage_object_count = 0;
	object_globals->last_garbage_warn_time = 0;

	return;
}

void objects_dispose_from_old_map(
	void)
{
	damage_dispose_from_old_map();
	widgets_dispose_from_old_map();
	object_types_dispose_from_old_map();
	lights_dispose_from_old_map();

	if (object_header_data->valid)
	{
		long i;
		for (i = data_next_index(object_header_data, NONE);
			i!=NONE;
			i = data_next_index(object_header_data, i))
		{
			struct data_array *data = object_header_data;
			struct object_header_datum *header = object_header_get(i);
			
			if (header->datum)
			{
				memory_pool_block_free(object_memory_pool, (void**)&header->datum);
			}

			datum_delete(data, i);
			header->datum = NULL;
			header->flags = 0;
		}
		data_make_invalid(object_header_data);
	}

	cluster_partition_make_invalid(&collideable_object_cluster_partition);
	cluster_partition_make_invalid(&noncollideable_object_cluster_partition);

	return;
}

void objects_dispose(
	void)
{
	widgets_dispose();
	object_types_dispose();
	lights_dispose();

	if (!game_in_editor())
	{
		if (object_header_data)
		{
			object_header_data = 0;
		}
	}
	else
	{
		data_dispose(object_header_data);
	}

	if (object_memory_pool)
	{
		object_memory_pool = NULL;
	}

	object_name_list_free();
	cluster_partition_delete(&collideable_object_cluster_partition);
	cluster_partition_delete(&noncollideable_object_cluster_partition);

	return;
}

void object_activate(
	long object_index)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);

	if (!TEST_FLAG(header->flags, _object_header_active_bit) &&
		!TEST_FLAG(object->object.flags, _object_cannot_be_activated_bit) &&
		object->object.parent_object_index==NONE)
	{
		SET_FLAG(header->flags, _object_header_active_bit, TRUE);
	}

	return;
}

void object_deactivate(
	long object_index)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);

	if (TEST_FLAG(header->flags, _object_header_active_bit))
	{
		SET_FLAG(header->flags, _object_header_active_bit, FALSE);
	}

	return;
}

void object_reset(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	object->object.translational_velocity = *global_zero_vector3d;
	object->object.angular_velocity = *global_zero_vector3d;
	SET_FLAG(object->object.flags, _object_at_rest_bit, FALSE);
	object_type_reset(object_index);

	return;
}

void object_placement_data_new(
	struct object_placement_data *data,
	long definition_index,
	long owner_object_index)
{
	long i;
	struct object_header_datum *header;

	memset(data, 0, sizeof(*data));

	data->definition_index = definition_index;
	data->flags = 0;
	data->forward = *global_forward3d;
	data->up = *global_up3d;
	data->variant_number = 0;

	header = object_header_try_and_get(owner_object_index);
	if (header && FLAG(header->type) && header->datum)
	{
		struct object_datum *object = header->datum;

		data->owner_object_index = owner_object_index;
		data->owner_player_index = object->object.owner_player_index;
		data->owner_team_index = object->object.owner_team_index;
	}
	else
	{
		data->owner_object_index = NONE;
		data->owner_player_index = NONE;
		data->owner_team_index = NONE;
	}

	i = 0;
	do
	{
		data->change_colors[i] = *global_real_rgb_white;
		i++;
	}
	while (i<NUMBEROF(data->change_colors));

	return;
}

void object_disconnect_from_map(
	long object_index)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = header->datum;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 957, DATUM_INDEX_TO_IDENTIFIER(object_index));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 958, TEST_FLAG(object->object.flags, _object_connected_to_map_bit));

	if (object->object.parent_object_index!=NONE)
	{
		object_remove_from_list(&object_get(object->object.parent_object_index)->object.first_child_object_index, object_index);
	}
	else
	{
		cluster_partition_disconnect(
			(TEST_FLAG(object_get(object_index)->object.flags, _object_has_collision_model_bit) ?
				&collideable_object_cluster_partition :
				&noncollideable_object_cluster_partition
			),
			object_index,
			&object->object.first_cluster_reference_index);

		if (TEST_FLAG(header->flags, _object_header_automatically_deactivate_bit))
		{
			object_deactivate(object_index);
		}
	}

	SET_FLAG(object->object.flags, _object_connected_to_map_bit, FALSE);
	SET_FLAG(header->flags, _object_header_connected_to_map_bit, FALSE);

	return;
}

short object_get_first_cluster(
	struct object_cluster_iterator *iterator,
	long object_index)
{
	long ultimate_parent_index = object_get_ultimate_parent(object_index);
	
	iterator->cluster_partition = TEST_FLAG(object_get(ultimate_parent_index)->object.flags, _object_has_collision_model_bit) ?
		&collideable_object_cluster_partition :
		&noncollideable_object_cluster_partition;

	return cluster_partition_get_first_cluster(
		iterator->cluster_partition,
		&iterator->reference_index,
		object_get(ultimate_parent_index)->object.first_cluster_reference_index);
}

real_matrix4x3 *object_get_node_matrices(
	long object_index)
{
	return (real_matrix4x3 *)object_header_block_get(object_index, &object_get(object_index)->object.node_matrices);
}

char const *object_get_attachment_marker_name(
	long object_index,
	short attachment_index)
{
	const char *result;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (VALID_INDEX(attachment_index, object_definition->object.attachments.count))
	{
		result = TAG_BLOCK_GET_ELEMENT(
			&object_definition->object.attachments,
			attachment_index,
			struct object_attachment_definition)->marker_name;
	}
	else
	{
		result = NULL;
	}

	return result;
}

boolean object_has_node(
	long object_index,
	short node_index)
{
	struct object_datum *object = object_get(object_index);
	long model_index = object_definition_get(object->definition_index)->object.model.index;
	boolean result = FALSE;

	if (model_index==NONE)
	{
		if (!node_index)
		{
			result = TRUE;
		}
	}
	else
	{
		struct model *model = model_definition_get(model_index);
		if (VALID_INDEX(node_index, model->nodes.count))
		{
			result = TRUE;
		}
	}

	return result;
}

void object_set_automatic_deactivation(
	long object_index,
	boolean automatic_deactivation)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);

	SET_FLAG(header->flags, _object_header_automatically_deactivate_bit, automatic_deactivation);
	
	if (automatic_deactivation)
	{
		if (object->object.parent_object_index==NONE && object->object.location.cluster_index==NONE)
		{
			object_deactivate(object_index);
		}
	}
	else
	{
		if (!TEST_FLAG(header->flags, _object_header_active_bit))
		{
			object_activate(object_index);
		}
	}

	return;
}

void object_set_visibility(
	long object_index,
	boolean visible_flag)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE)
	{
		if (TEST_FLAG(object->object.flags, _object_invisible_bit) && visible_flag)
		{
			object_connect_lights(object_index, FALSE, TRUE);
		}
		else if (!TEST_FLAG(object->object.flags, _object_invisible_bit) && !visible_flag)
		{
			object_connect_lights(object_index, TRUE, FALSE);
		}
	}

	if (!visible_flag || object_definition->object.model.index!=NONE)
	{
		struct object_header_datum *header = object_header_get(object_index);

		SET_FLAG(object->object.flags, _object_invisible_bit, !visible_flag);
		SET_FLAG(header->flags, _object_header_visible_bit, visible_flag);
	}

	return;
}

void object_get_velocities(
	long object_index,
	real_vector3d *translational_velocity,
	real_vector3d *angular_velocity)
{
	struct object_datum *ultimate_parent = object_get(object_index);

	while (ultimate_parent->object.parent_object_index!=NONE)
	{
		ultimate_parent = object_get(ultimate_parent->object.parent_object_index);
	}

	if (translational_velocity)
	{
		*translational_velocity = ultimate_parent->object.translational_velocity;
	}

	if (angular_velocity)
	{
		*angular_velocity = ultimate_parent->object.angular_velocity;
	}

	return;
}

struct location *object_get_location(
	long object_index,
	struct location *location)
{
	*location = object_get(object_get_ultimate_parent(object_index))->object.location;

	return location;
}

void object_start_interpolation(
	long object_index,
	short frame_count)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	short node_count = model_definition_get(object_definition->object.model.index)->nodes.count;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1523, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));

	memcpy(
		object_header_block_get(object_index, &object->object.original_node_orientations),
		object_header_block_get(object_index, &object->object.node_orientations),
		sizeof(real_orientation)*node_count);
	if (frame_count>=
		object->object.animation.interpolation_frame_count-
		object->object.animation.interpolation_frame_index
	)
	{
		object->object.animation.interpolation_frame_index = 0;
		object->object.animation.interpolation_frame_count = frame_count;
	}

	return;
}

void object_offset_interpolation(
	long object_index,
	real_vector3d *offset)
{
	struct object_datum *object = object_get(object_index);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1546, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));

	if (object->object.animation.interpolation_frame_count)
	{
		real_orientation* orientation = (real_orientation *)object_header_block_get(
			object_index,
			&object->object.original_node_orientations);
		orientation->translation.x+= offset->i;
		orientation->translation.y+= offset->j;
		orientation->translation.z+= offset->k;
	}

	return;
}

void object_permute_region(
	long object_index,
	char *permutation_name,
	short desired_region_index,
	boolean active_flag)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE)
	{
		short region_index;
		struct model* model = model_definition_get(object_definition->object.model.index);
		
		for (region_index = 0; region_index<model->regions.count; region_index++)
		{
			if (desired_region_index==NONE || desired_region_index==region_index)
			{
				short permutation_index;
				struct model_region *region = TAG_BLOCK_GET_ELEMENT(
					&model->regions,
					region_index,
					struct model_region);

				for (permutation_index = 0; permutation_index<region->permutations.count; permutation_index++)
				{
					struct model_region_permutation *permutation = TAG_BLOCK_GET_ELEMENT(
						&region->permutations,
						permutation_index,
						struct model_region_permutation);
					
					if (!_stricmp(permutation->name, permutation_name))
					{
						object->object.region_permutations[region_index] = active_flag ? permutation_index : 0;
						break;
					}
				}
			}
		}
	}

	return;
}

boolean object_get_function_value(
	long object_index,
	short function_index,
	real *value_reference)
{
	boolean result;
	struct object_datum *object = object_get(object_index);

	
	if (function_index==NONE)
	{
		*value_reference = 1.f;
		result = TRUE;
	}
	else
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1654, function_index>=0 && function_index<NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS);

		*value_reference = object->object.outgoing_function_values[function_index];
		
		result = TEST_FLAG(object->object.functions_active_flags, function_index);
	}
	
	return result;
}

short objects_in_clusters_by_indices(
	unsigned long class_flags,
	short cluster_count,
	short const *cluster_indices,
	short maximum_object_count,
	long *object_indices)
{
	short cluster_index;

	short object_count = 0;

	if (!class_flags)
	{
		class_flags = NONE;
	}

	object_marker_begin();

	for (cluster_index = 0; cluster_index<cluster_count; ++cluster_index)
	{
		short cluster_num = cluster_indices[cluster_index];
		if (TEST_FLAG(class_flags, 0))
		{
			long reference;
			long object_index;

			for (object_index = cluster_get_first_collideable_object(&reference, cluster_num);
				object_index!=NONE;
				object_index = cluster_get_next_collideable_object(&reference))
			{
				if (object_mark_function(object_index))
				{
					if (object_count>=maximum_object_count)
					{
						object_marker_end();
						return object_count;
					}
					object_indices[object_count++] = object_index;
				}
			}
		}

		if (TEST_FLAG(class_flags, 1))
		{
			long reference;
			long object_index;

			for (object_index = cluster_get_first_noncollideable_object(&reference, cluster_num);
				object_index!=NONE;
				object_index = cluster_get_next_noncollideable_object(&reference))
			{
				if (object_mark_function(object_index))
				{
					if (object_count>=maximum_object_count)
					{
						object_marker_end();
						return object_count;
					}
					object_indices[object_count++] = object_index;
				}
			}
		}
	}

	object_marker_end();

	return object_count;
}

long object_index_from_name_index(
	short name_index)
{
	return name_index>=0 && name_index<MAXIMUM_OBJECT_NAMES_PER_SCENARIO ? object_name_list[name_index] : NONE;
}

void objects_disconnect_from_structure_bsp(
	void)
{
	struct object_datum *object;
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_all, 0);
	while (object = (struct object_datum *)object_iterator_next(&iterator))
	{
		if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
			object->object.parent_object_index==NONE)
		{
			object_disconnect_from_map(iterator.index);

			SET_FLAG(object->object.flags, _object_connected_to_map_bit, TRUE);
		}

		object_type_disconnect_from_structure_bsp(iterator.index);
	}

	return;
}

boolean object_visible_to_any_player(
	long object_index)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);
	boolean visible = FALSE;

	if (TEST_FLAG(header->flags, _object_header_active_bit) &&
		TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
		!TEST_FLAG(object->object.flags, _object_outside_of_map_bit))
	{
		short cluster_index;
		struct object_cluster_iterator iterator;

		const unsigned long* pvs = players_get_combined_pvs();

		for (cluster_index = object_get_first_cluster(&iterator, object_index);
			cluster_index!=NONE && !BIT_VECTOR_TEST_FLAG(pvs, cluster_index);
			cluster_index = object_get_next_cluster(&iterator, object_index))
		{
			;
		}

		if (cluster_index!=NONE)
		{
			long player_index;
			const real radius_squared = object->object.bounding_sphere_radius*object->object.bounding_sphere_radius;
			
			for (player_index = data_next_index(player_data, NONE);
				player_index!=NONE;
				player_index = data_next_index(player_data, player_index))
			{
				struct player_datum *player = player_get(player_index);
				
				if (player->unit_index!=NONE)
				{
					real player_distance_squared;
					real_point3d player_position;

					unit_get_head_position(player->unit_index, &player_position);
					player_distance_squared = distance_squared3d(&player_position, &object->object.bounding_sphere_center);
					
					if (player_distance_squared<radius_squared)
					{
						visible = TRUE;
						break;
					}
					else
					{
						real_vector3d eye_to_point;
						
						real const sloppy_maximum_field_of_view = DEGREES_TO_RADIANS(45);
						struct unit_datum const *unit = unit_get(player->unit_index);
						real distance = normalize3d(vector_from_points3d(&player_position, &object->object.bounding_sphere_center, &eye_to_point));

						if (dot_product3d(&unit->unit.desired_aiming_vector, &eye_to_point) > cosine(arctangent(object->object.bounding_sphere_radius, distance) + sloppy_maximum_field_of_view))
						{
							visible = TRUE;
							break;
						}
					}					
				}
			}
		}
	}

	return visible;
}

void object_pvs_activate(
	long object_index)
{
	object_pvs_set_object(object_index);

	return;
}

void objects_scripting_set_scale(
	long object_index,
	real scale,
	short interpolation_frame_count)
{
	if (object_index!=NONE)
	{
		struct object_datum *object = object_get(object_index);
		object->object.scale = scale;
		if (!TEST_FLAG(_object_mask_cannot_interpolate, object->object.type))
		{
			object_start_interpolation(object_index, interpolation_frame_count);
		}
	}

	return;
}

static void object_delete_initial_recursive(
	long object_index,
	boolean delete_siblings)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);

	if (game_engine_running() && object->object.type == _object_type_weapon)
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 829, !(weapon_is_flag(object_index)));
	}
	
	if (object->object.first_child_object_index!=NONE)
	{
		object_delete_initial_recursive(object->object.first_child_object_index, TRUE);
	}

	if (delete_siblings)
	{
		if (object->object.next_object_index!=NONE)
		{
			object_delete_initial_recursive(object->object.next_object_index, TRUE);
		}
	}

	SET_FLAG(header->flags, _object_header_being_deleted_bit, TRUE);

	object_set_visibility(object_index, FALSE);
	object_name_list_delete(object_index);

	return;
}

void object_delete(
	long object_index)
{
	object_delete_initial_recursive(object_index, FALSE);

	return;
}

void object_reconnect_to_map(
	long object_index,
	struct location const *location)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = header->datum;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 879, DATUM_INDEX_TO_IDENTIFIER(object_index));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 880, !TEST_FLAG(object->object.flags, _object_connected_to_map_bit));

	if (object->object.parent_object_index!=NONE)
	{
		struct object_datum *parent_object = object_get(object->object.parent_object_index);
		object_add_to_list(&parent_object->object.first_child_object_index, object_index);

		SET_FLAG(header->flags, _object_header_child_bit, TRUE);
		object->object.location.cluster_index =NONE;
	}
	else
	{
		if (!location)
		{
			struct location bounding_sphere_location;
			
			scenario_location_from_point(&bounding_sphere_location, &object->object.bounding_sphere_center);
			location = &bounding_sphere_location;

			if (bounding_sphere_location.cluster_index==NONE)
			{
				scenario_location_from_point(&bounding_sphere_location, &object->object.position);
			}
		}

		if (location->cluster_index!=NONE)
		{
			object->object.location = *location;
			header->cluster_index = object->object.location.cluster_index;

			SET_FLAG(object->object.flags, _object_outside_of_map_bit, FALSE);
		}
		else
		{
			SET_FLAG(object->object.flags, _object_outside_of_map_bit, TRUE);
		}

		SET_FLAG(header->flags, _object_header_child_bit, FALSE);

		cluster_partition_reconnect(
			(TEST_FLAG(object_get(object_index)->object.flags, _object_has_collision_model_bit) ?
				&collideable_object_cluster_partition :
				&noncollideable_object_cluster_partition
			),
			object_index,
			&object->object.first_cluster_reference_index,
			&object->object.bounding_sphere_center,
			object->object.bounding_sphere_radius,
			&object->object.location);

		if (TEST_FLAG(header->flags, _object_header_automatically_deactivate_bit))
		{
			if (header->cluster_index==NONE || !BIT_VECTOR_TEST_FLAG(players_get_combined_pvs(), header->cluster_index))
			{
				if (TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit))
				{
					object_delete(object_index);
				}
			}
			else
			{
				object_activate(object_index);
			}
		}
	}

	SET_FLAG(object->object.flags, _object_connected_to_map_bit, TRUE);
	SET_FLAG(header->flags, _object_header_connected_to_map_bit, TRUE);
	
	return;
}

real_matrix4x3 *object_get_node_matrix(
	long object_index,
	short node_index)
{
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1060, object_has_node(object_index, node_index));

	return &((real_matrix4x3 *)object_header_block_get(object_index, &object_get(object_index)->object.node_matrices))[node_index];
}

short object_get_marker_by_name(
	long object_index,
	char const *name,
	struct object_marker *markers,
	short maximum_marker_count)
{
	short marker;

	struct object_datum const *object = object_get(object_index);
	struct object_definition const *object_definition = object_definition_get(object->definition_index);
	struct object_datum *matrix_object = object_get(object_index);
	real_matrix4x3 const *matrices = (real_matrix4x3 *)object_header_block_get(object_index,
		&matrix_object->object.node_matrices);

	marker = model_get_marker_by_name(
		object_definition->object.model.index,
		name,
		object->object.region_permutations,
		FALSE,
		NONE,
		matrices,
		TEST_FLAG(object->object.flags, _object_mirrored_bit),
		markers,
		maximum_marker_count);

	if (marker==0)
	{
		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1113, maximum_marker_count>0);
		
		markers[marker].node_index = 0;
		matrix4x3_identity(&markers->node_matrix);
		markers[marker].matrix = *object_get_node_matrix(object_index, 0);

		if (TEST_FLAG(object->object.flags, _object_mirrored_bit))
		{
			negate_vector3d(&markers[0].matrix.left, &markers[0].matrix.left);
		}

		if (name && name[0]=='\0')
		{
			marker = 1;
		}
	}

	return marker;
}

void object_align_marker_to_matrix(
	struct object_datum *object,
	struct object_marker const *child_marker,
	real_matrix4x3 const *destination_matrix)
{
	real_matrix4x3 marker_transform_matrix;
	real_matrix4x3 final_matrix;

	real_vector3d vector;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1173, object);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1174, child_marker);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1175, destination_matrix);

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1177, valid_real_matrix4x3(destination_matrix));

	matrix4x3_from_point_and_vectors(&final_matrix, &object->object.position, &object->object.forward, &object->object.up);
	
	matrix4x3_inverse(&final_matrix, &marker_transform_matrix);
	matrix4x3_multiply(&marker_transform_matrix, &child_marker->matrix, &marker_transform_matrix);
	matrix4x3_inverse(&marker_transform_matrix, &marker_transform_matrix);
	matrix4x3_multiply(destination_matrix, &marker_transform_matrix, &final_matrix);

	object->object.position = final_matrix.position;
	object->object.forward = final_matrix.forward;

	cross_product3d(&final_matrix.forward, &final_matrix.up, &vector);
	cross_product3d(&vector, &final_matrix.forward, &object->object.up);

	normalize3d(&object->object.forward);
	normalize3d(&object->object.up);

	return;
}

void object_detach(
	long child_object_index)
{
	real_matrix4x3 object_translation_matrix;
	real_matrix4x3 matrix;
	real_matrix4x3 object_rotation_matrix;

	real_matrix4x3 *node_matrix;

	struct object_datum *child_object = object_get(child_object_index);
	struct object_datum *parent_object = object_get(child_object->object.parent_object_index);

	object_disconnect_from_map(child_object_index);
	node_matrix = object_get_node_matrix(child_object->object.parent_object_index, child_object->object.parent_node_index);

	matrix4x3_translation(&object_translation_matrix, &child_object->object.position);
	matrix4x3_rotation_from_vectors(&object_rotation_matrix, &child_object->object.forward, &child_object->object.up);
	matrix4x3_multiply(node_matrix, &object_translation_matrix, &matrix);
	matrix4x3_multiply(&matrix, &object_rotation_matrix, &matrix);
	matrix4x3_to_point_and_vectors(
		&matrix,
		&child_object->object.position,
		&child_object->object.forward,
		&child_object->object.up);

	child_object->object.translational_velocity = parent_object->object.translational_velocity;
	child_object->object.angular_velocity = parent_object->object.angular_velocity;
	child_object->object.parent_node_index = NONE;
	child_object->object.parent_object_index = NONE;

	object_reconnect_to_map(child_object_index, NULL);
	object_activate(child_object_index);

	return;
}

real_point3d *object_get_origin(
	long object_index,
	real_point3d *origin)
{
	struct object_datum *object = object_get(object_index);

	if (object->object.parent_object_index==NONE)
	{
		*origin = object->object.position;
	}
	else
	{
		real_matrix4x3 *matrix = object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index);
		matrix4x3_transform_point(matrix, &object->object.position, origin);
	}

	return origin;
}

void object_get_orientation(
	long object_index,
	real_vector3d *forward,
	real_vector3d *up)
{
	struct object_datum *object = object_get(object_index);

	if (object->object.parent_object_index==NONE)
	{
		if (forward)
		{
			*forward = object->object.forward;
		}
		if (up)
		{
			*up = object->object.up;
		}
	}
	else
	{
		real_matrix4x3 *node_matrix = object_get_node_matrix(
			object->object.parent_object_index,
			object->object.parent_node_index);

		if (forward)
		{
			matrix4x3_transform_normal(node_matrix, &object->object.forward, forward);
		}
		if (up)
		{
			matrix4x3_transform_normal(node_matrix, &object->object.up, up);
		}
	}

	if (forward && up)
	{
		match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\objects\\objects.c", 1462, forward, up);
	}

	return;
}

real_matrix4x3 *object_get_world_matrix(
	long object_index,
	real_matrix4x3 *matrix)
{
	struct object_datum *object = object_get(object_index);
	matrix4x3_from_point_and_vectors(matrix, &object->object.position, &object->object.forward, &object->object.up);

	if (object->object.parent_object_index!=NONE)
	{
		real_matrix4x3* node_matrix = object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index);
		matrix4x3_multiply(node_matrix, matrix, matrix);
	}

	return matrix;
}

void object_inverse_kinematics(
	long hand_object_index,
	char *hand_marker_name,
	long grip_object_index,
	char *grip_marker_name,
	real_matrix4x3 *node_matrices)
{
	struct object_marker hand_marker;
	struct object_marker grip_marker;
	real_matrix4x3 desired_hand_matrix;

	struct object_datum *hand_object = object_get(hand_object_index);
	struct object_definition *hand_object_definition = object_definition_get(hand_object->definition_index);
	struct model *hand_model = model_definition_get(hand_object_definition->object.model.index);
	
	if (object_get_marker_by_name(hand_object_index, hand_marker_name, &hand_marker, 1) && 
		object_get_marker_by_name(grip_object_index, grip_marker_name, &grip_marker, 1))
	{
		short hand_node_index = hand_marker.node_index;
		short hand_parent_node_index = TAG_BLOCK_GET_ELEMENT(&hand_model->nodes, hand_node_index, struct model_node)->parent_node_index;
		short desired_hand_index = NONE;

		if (hand_parent_node_index!=NONE)
		{
			desired_hand_index = TAG_BLOCK_GET_ELEMENT(&hand_model->nodes, hand_parent_node_index, struct model_node)->parent_node_index;
		}

		if (desired_hand_index!=NONE)
		{
			matrix4x3_inverse(&hand_marker.node_matrix, &desired_hand_matrix);
			matrix4x3_multiply(&grip_marker.matrix, &desired_hand_matrix, &desired_hand_matrix);
			inverse_kinematics_adjust_matrices(
				&desired_hand_matrix,
				&node_matrices[desired_hand_index],
				&node_matrices[hand_parent_node_index],
				&node_matrices[hand_node_index]);
		}
	}

	return;
}

short objects_in_sphere(
	unsigned long class_flags,
	unsigned long type_flags,
	struct location const *location,
	real_point3d const *center,
	real radius,
	long *object_indices,
	short maximum_count)
{
	long temporary_object_indices[MAXIMUM_OBJECTS_PER_MAP];
	short cluster_indices[MAXIMUM_CLUSTERS_PER_STRUCTURE];

	short cluster_count;
	short object_count_in_clusters;
	short object_index;
	
	short result = 0;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1779, location);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1780, center);
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 1781, object_indices);

	if (!type_flags)
	{
		type_flags = NONE;
	}

	cluster_count = structure_clusters_in_sphere(
		location->cluster_index,
		center,
		radius,
		NUMBEROF(cluster_indices),
		cluster_indices);
	object_count_in_clusters = objects_in_clusters_by_indices(
		class_flags,
		cluster_count,
		cluster_indices,
		NUMBEROF(temporary_object_indices),
		temporary_object_indices);
	for (object_index = 0; object_index<object_count_in_clusters && result<maximum_count; ++object_index)
	{
		struct object_datum *object = object_get(temporary_object_indices[object_index]);
		if (TEST_FLAG(type_flags, object->object.type) &&
			point_in_sphere(center, &object->object.bounding_sphere_center, (object->object.bounding_sphere_radius)+radius))
		{
			object_indices[result++] = temporary_object_indices[object_index];
		}
		
	}

	return result;
}

void objects_reconnect_to_structure_bsp(
	void)
{
	struct object_datum *object;

	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_all, 0);
	
	while(object = (struct object_datum *)object_iterator_next(&iterator))
	{
		if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
			object->object.parent_object_index==NONE)
		{
			struct location scenario_location;

			SET_FLAG(object->object.flags, _object_connected_to_map_bit, FALSE);
			
			object->object.location.cluster_index = NONE;
			object_header_get(iterator.index)->cluster_index = NONE;
		
			scenario_location_from_point(&scenario_location, &object->object.bounding_sphere_center);
			if (scenario_location.cluster_index==NONE)
			{
				struct collision_bsp_test_sphere_result result;
				collision_bsp_test_sphere(
					global_collision_bsp_get(),
					0,
					NULL,
					&object->object.bounding_sphere_center,
					object->object.bounding_sphere_radius,
					&result);

				if (result.leaf_count)
				{
					scenario_location.leaf_index = result.leaf_indices[0];
					if (result.leaf_indices[0] == -1)
					{
						scenario_location.cluster_index = -1;
					}
					else
					{
						scenario_location.cluster_index = TAG_BLOCK_GET_ELEMENT(
							&global_structure_bsp_get()->leaves,
							result.leaf_indices[0] & LONG_MAX,
							struct structure_leaf)->cluster_index;
					}
				}
				else
				{
					scenario_location_from_point(&scenario_location, &object->object.position);
				}
			}
			object_reconnect_to_map(iterator.index, &scenario_location);
		}
	}
	
	return;
}

void objects_paparazzi(
	void)
{
	struct object_datum *object;
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_all, 0);

	while (object = (struct object_datum *)object_iterator_next(&iterator))
	{
		if (TEST_FLAG(object->object.flags, _object_movie_star_bit))
		{
			object_delete_initial_recursive(iterator.index, 0);
		}
	}

	return;
}

void object_export_function_values(
	long object_index)
{
	short i;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	for (i = 0; i<NUMBEROF(object->object.incoming_function_values); ++i)
	{
		if (object_definition->object.function_modes[i])
		{
			short region_index;
			real_matrix4x3 *node_matrix;

			real value = 0.f;

			switch (object_definition->object.function_modes[i])
			{
			case _object_function_recent_body_damage:
				value = object->object.current_body_damage;
				break;
			case _object_function_recent_shield_damage:
				value = object->object.current_shield_damage;
				break;
			case _object_function_body_vitality:
				value = object->object.body_vitality;
				break;
			case _object_function_shield_vitality:
				value = object->object.shield_vitality;
				value = MIN(value, 1.f);
				break;
		
			case _object_function_random_constant:
				if (object->object.incoming_function_values[i]==1.f)
				{
					value = real_random();
				}
				break;
			case _object_function_alive:
				value = TEST_FLAG(object->object.damage_flags, _object_dead_bit) ? 0.f : 1.f;
				break;
			case _object_function_compass:
				node_matrix = object_get_node_matrix(object_index, 0);
				if (fabs(node_matrix->forward.k) < 0.995f)
				{
					value = signed_angular_difference(global_scenario_get()->local_north, arctangent(node_matrix->forward.i, node_matrix->forward.j));
					value = (0.15915494f * value) + 0.5f;
					value = PIN(value, 0.f, 1.f);
				}
				else
				{
					value = object->object.incoming_function_values[i];
				}
				break;
			default:
				region_index = object_definition->object.function_modes[i]-_object_function_first_region_damage;
				match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2630, region_index>=0 && region_index<MAXIMUM_REGIONS_PER_OBJECT);
				value = object->object.region_damage[region_index] / 255.f;
				break;
			}
			object->object.incoming_function_values[i] = value;
		}
		
	}

	return;
}

// These local variants preserve the original XDK 3911 operand provenance for
// this large function.  They are semantically identical to the shared matrix
// validation macros, but the typed aliases reproduce the January codegen.
#define ocnm_root_matrix_internal(file, line, matrix, string) \
match_vassert(file, line, valid_real((*matrix).scale), csprintf(temporary, "%s had a bad scale %f", string, (*matrix).scale)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).forward), csprintf(temporary, "%s had a bad forward (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).left), csprintf(temporary, "%s had a bad left (%f,%f,%f)", string, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).up), csprintf(temporary, "%s had a bad up (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k)); \
match_vassert(file, line, valid_real_point3d(&(*matrix).position), csprintf(temporary, "%s had a bad position (%f,%f,%f)", string, (*matrix).position.x, (*matrix).position.y, (*matrix).position.z)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).left), 0.f), \
	csprintf(temporary, "%s had a forward (%f,%f,%f) not perpendicular to left (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(ocnm_left, &(*matrix).up), 0.f), \
	csprintf(temporary, "%s had a up (%f,%f,%f) not perpendicular to left (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(ocnm_forward, ocnm_up), 0.f), \
	csprintf(temporary, "%s had a forward (%f,%f,%f) not perpendicular to up (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k)); \
match_vassert(file, line, valid_real_matrix4x3(matrix), csprintf(temporary, "%s: assert_valid_real_matrix4x3", string));

#define ocnm_root_matrix_custom(file, line, matrix, custom_string) \
if (!valid_real_matrix4x3(matrix)) \
{ \
	char const *string = (custom_string); \
	real_vector3d const *ocnm_forward = &(*matrix).forward; \
	real_vector3d const *ocnm_left = &(*matrix).left; \
	real_vector3d const *ocnm_up = &(*matrix).up; \
	ocnm_root_matrix_internal(file, line, matrix, string); \
}

#define ocnm_parent_matrix_internal(file, line, matrix, string) \
match_vassert(file, line, valid_real((*matrix).scale), csprintf(temporary, "%s had a bad scale %f", string, (*matrix).scale)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).forward), csprintf(temporary, "%s had a bad forward (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).left), csprintf(temporary, "%s had a bad left (%f,%f,%f)", string, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, valid_real_normal3d(&(*matrix).up), csprintf(temporary, "%s had a bad up (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k)); \
match_vassert(file, line, valid_real_point3d(&(*matrix).position), csprintf(temporary, "%s had a bad position (%f,%f,%f)", string, (*matrix).position.x, (*matrix).position.y, (*matrix).position.z)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).left), 0.f), \
	csprintf(temporary, "%s had a forward (%f,%f,%f) not perpendicular to left (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(ocnm_up, ocnm_left), 0.f), \
	csprintf(temporary, "%s had a up (%f,%f,%f) not perpendicular to left (%f,%f,%f)", string, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k, (*matrix).left.i, (*matrix).left.j, (*matrix).left.k)); \
match_vassert(file, line, \
	valid_realcmp(dot_product3d(&(*matrix).forward, &(*matrix).up), 0.f), \
	csprintf(temporary, "%s had a forward (%f,%f,%f) not perpendicular to up (%f,%f,%f)", string, (*matrix).forward.i, (*matrix).forward.j, (*matrix).forward.k, (*matrix).up.i, (*matrix).up.j, (*matrix).up.k)); \
match_vassert(file, line, valid_real_matrix4x3(matrix), csprintf(temporary, "%s: assert_valid_real_matrix4x3", string));

#define ocnm_parent_matrix_custom(file, line, matrix, custom_string) \
if (!valid_real_matrix4x3(matrix)) \
{ \
	char const *string = (custom_string); \
	real_vector3d const *ocnm_left = &(*matrix).left; \
	real_vector3d const *ocnm_up = &(*matrix).up; \
	ocnm_parent_matrix_internal(file, line, matrix, string); \
}

void object_compute_node_matrices(
	long object_index)
{
	real_orientation cannot_interpolate_node_orientations_storage[MAXIMUM_NODES_PER_MODEL];
	short node_stack[MAXIMUM_NODES_PER_MODEL];
	real_matrix4x3 center_matrix;
	real_matrix4x3 offset_matrix;
	real_matrix4x3 node_matrix;
	real_matrix4x3 object_translation_matrix;
	real_matrix4x3 object_rotation_matrix;
	real_point3d center;
	real_matrix4x3 parent_node_matrix_no_scale_no_mirror;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	
	real_matrix4x3 *object_nodes = (real_matrix4x3 *)object_header_block_get(object_index, &object->object.node_matrices);
	real_orientation *node_orientations =
		TEST_FLAG(_object_mask_cannot_interpolate, object->object.type) ?
		cannot_interpolate_node_orientations_storage :
		(real_orientation *)object_header_block_get(object_index, &object->object.node_orientations);

	if (object_definition->object.model.index!=NONE)
	{
		real_matrix4x3 *object_node_matrix;
		boolean world_relative;

		short node_count;
		short node_index;
		long unknown_var;

		struct object_type_definition *object_type_definition = object_type_definition_get(object->object.type);
		
		struct model *model = model_definition_get(object_definition->object.model.index);

		if (object->object.parent_object_index==NONE)
		{
			object_node_matrix = NULL;
		}
		else
		{
			object_node_matrix = object_get_node_matrix(object->object.parent_object_index, object->object.parent_node_index);
		}

		world_relative = FALSE;

		if (object->object.animation.animation_graph_index!=NONE)
		{
			if (object->object.animation.state.index!=NONE)
			{
				short frame_index;

				struct animation_graph *animation_graph = animation_graph_definition_get(
					object->object.animation.animation_graph_index);
				struct animation *animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					object->object.animation.state.index,
					struct animation);

				if (TEST_FLAG(object->object.flags, _object_animates_automatically_bit) && animation->frame_count>0)
				{

					frame_index = OBJECT_FRAME_INDEX_GET(object_index) % (unsigned long)animation->frame_count;

					match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2704, frame_index>=0)
				}
				else
				{
					frame_index = object->object.animation.state.frame_index;
				}

				animation_get_node_orientations(model, animation, frame_index, node_orientations);
				world_relative = TEST_FLAG(animation->flags, _animation_world_relative_bit);
			}
			else
			{
				model_get_node_orientations(model, node_orientations);
			}
		}
		else
		{
			model_get_node_orientations(model, node_orientations);
		}

		if (object_definition->object.animation_graph.index!=NONE)
		{
			short overlay_index;

			struct animation_graph *animation_graph= animation_graph_definition_get(object_definition->object.animation_graph.index);
			
			for (overlay_index = 0; overlay_index<animation_graph->object_overlays.count; ++overlay_index)
			{
				struct animation_graph_object_overlay* overlay = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->object_overlays,
					overlay_index,
					struct animation_graph_object_overlay);

				if (overlay->animation_index!=NONE &&
					overlay->function_index<object_definition->object.functions.count)
				{
					struct object_function_definition* function = TAG_BLOCK_GET_ELEMENT(
						&object_definition->object.functions,
						overlay->function_index,
						struct object_function_definition);
					struct animation *animation = TAG_BLOCK_GET_ELEMENT(
						&animation_graph->animations,
						overlay->animation_index,
						struct animation);
					real value= object->object.outgoing_function_values[overlay->function_index];

					if (overlay->mode==_object_overlay_mode_frame)
					{
						real frame_index;
						if (TEST_FLAG(function->flags, _object_function_additive_bit))
						{
							frame_index = animation->frame_count*value;
						}
						else
						{
							frame_index = (animation->frame_count-1) * value;
						}

						overlay_animation_apply_continuous(animation, frame_index, node_orientations);
					}
					else if (overlay->mode==_object_overlay_mode_scale)
					{
						short frame_index = OBJECT_FRAME_INDEX_GET(object_index) % (unsigned long)animation->frame_count;

						overlay_animation_apply_scaled(
							animation,
							frame_index,
							value,
							node_orientations);
					}
				}
			}
		}

		// Scale the object orientations
		if (object->object.scale>0.f)
		{
			node_orientations[0].scale*= object->object.scale;
			node_orientations[0].translation.x*= object->object.scale;
			node_orientations[0].translation.y*= object->object.scale;
			node_orientations[0].translation.z*= object->object.scale;
		}


		if (object_definition->object.animation_graph.index!=NONE)
		{
			object_type_preprocess_node_orientations(object_index, node_orientations);
		}

		if (object->object.animation.interpolation_frame_count>0)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2777, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));

			interpolate_node_orientations(
				model->nodes.count,
				(real_orientation *)object_header_block_get(
					object_index,
					&object->object.original_node_orientations
				),
				node_orientations,
				object->object.animation.interpolation_frame_index,
				object->object.animation.interpolation_frame_count);
		}

		if (!world_relative)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\objects\\objects.c",
				2786,
				valid_real_point3d(&object->object.position), 
				csprintf(
					temporary,
					"%s had a bad position before compute_node_matrices (%f,%f,%f)",
					tag_get_name(object->definition_index),
					object->object.position.x,
					object->object.position.y,
					object->object.position.z));
			{
				real_vector3d const *validated_forward = &object->object.forward;

				match_vassert(
					"c:\\halo\\SOURCE\\objects\\objects.c",
					2787,
					valid_real_vector3d_axes2(validated_forward, &object->object.up),
					csprintf(
						temporary,
						"%s had a bad forward and up before compute_node_matrices (%f,%f,%f)x(%f,%f,%f)",
						tag_get_name(object->definition_index),
						object->object.forward.i,
						object->object.forward.j,
						object->object.forward.k,
						object->object.up.i,
						object->object.up.j,
						object->object.up.k));
			}
		}

		node_index = 0;
		node_count = 1;
		unknown_var = 0;
		node_stack[0] = 0;

		while (TRUE)
		{
			short node_stack_index = node_stack[node_index++];
			short next_sibling_node_index;
			struct model_node *node = TAG_BLOCK_GET_ELEMENT(&model->nodes, node_stack_index, struct model_node);
			
			if (node_stack_index==0)
			{
				matrix4x3_from_orientation(&node_matrix, node_orientations);
				
				if (!world_relative)
				{
					matrix4x3_translation(&object_translation_matrix, &object->object.position);
					matrix4x3_rotation_from_vectors(&object_rotation_matrix, &object->object.forward, &object->object.up);

					if (TEST_FLAG(object->object.flags, _object_mirrored_bit))
					{
						object_rotation_matrix.left.i = -object_rotation_matrix.left.i;
						object_rotation_matrix.left.j = -object_rotation_matrix.left.j;
						object_rotation_matrix.left.k = -object_rotation_matrix.left.k;
					}

					if (object_definition->object.physics.index!=NONE)
					{
						struct physics_definition *physics_definition= physics_definition_get(object_definition->object.physics.index);
						set_real_point3d(&center, -physics_definition->center_of_mass.x, -physics_definition->center_of_mass.y, -physics_definition->center_of_mass.z);
						matrix4x3_translation(&center_matrix, &center);
						matrix4x3_multiply(&object_rotation_matrix, &center_matrix, &object_rotation_matrix);
					}

					matrix4x3_translation(&offset_matrix, &object_definition->object.origin_offset);
					matrix4x3_multiply(&object_rotation_matrix, &offset_matrix, &object_rotation_matrix);

					if (object_node_matrix)
					{
						real_matrix4x3 const *validated_parent_node_matrix;

						if (object_node_matrix->scale!=1.f)
						{
							real scaled_z;

							object_translation_matrix.position.x *= object_node_matrix->scale;
							object_translation_matrix.position.y *= object_node_matrix->scale;
							scaled_z = object_translation_matrix.position.z * object_node_matrix->scale;

							parent_node_matrix_no_scale_no_mirror= *object_node_matrix;
							object_translation_matrix.position.z = scaled_z;
							object_node_matrix= &parent_node_matrix_no_scale_no_mirror;
							parent_node_matrix_no_scale_no_mirror.scale= 1.f;
						}

						if (TEST_FLAG(object_get(object->object.parent_object_index)->object.flags, _object_mirrored_bit))
						{
							if (object_node_matrix!=&parent_node_matrix_no_scale_no_mirror)
							{
								parent_node_matrix_no_scale_no_mirror = *object_node_matrix;
								object_node_matrix = &parent_node_matrix_no_scale_no_mirror;
							}

							negate_vector3d(&object_node_matrix->left, &object_node_matrix->left);
						}

						validated_parent_node_matrix = object_node_matrix;
						ocnm_parent_matrix_custom(
							"c:\\halo\\SOURCE\\objects\\objects.c",
							2871,
							validated_parent_node_matrix,
							csprintf(
								temporary,
								"%s as parent node of %s",
								tag_get_name(object_get(object->object.parent_object_index)->definition_index),
								tag_get_name(object->definition_index)));


						matrix4x3_multiply(object_node_matrix, &object_translation_matrix, &object_nodes[node_stack_index]);
						matrix4x3_multiply(&object_nodes[node_stack_index], &object_rotation_matrix, &object_nodes[node_stack_index]);
						matrix4x3_multiply(&object_nodes[node_stack_index], &node_matrix, &object_nodes[node_stack_index]);
					}
					else
					{
						matrix4x3_multiply(&object_translation_matrix, &object_rotation_matrix, &object_nodes[node_stack_index]);
						matrix4x3_multiply(&object_nodes[node_stack_index], &node_matrix, &object_nodes[node_stack_index]);
					}
				}
				else
				{
					object_nodes[node_stack_index] = node_matrix;
				}

				if (!valid_real_matrix4x3(&object_nodes[node_stack_index]))
				{
					real_matrix4x3 const *validated_root_node_matrix;

					error(_error_silent, "object_compute_node_matrices FAILURE on root node of %s", tag_get_name(object->definition_index));
					error(
						_error_silent,
						"  object: pos %f %f %f, fwd %f %f %f, up %f %f %f",
						object->object.position.x,
						object->object.position.y,
						object->object.position.z,
						object->object.forward.i,
						object->object.forward.j,
						object->object.forward.k,
						object->object.up.i,
						object->object.up.j,
						object->object.up.k);

					if (object_definition->object.physics.index!=NONE)
					{
						struct physics_definition *physics_definition= physics_definition_get(object_definition->object.physics.index);
						error(
							_error_silent,
							"  center-of-mass translation %f %f %f",
							-physics_definition->center_of_mass.x,
							-physics_definition->center_of_mass.y,
							-physics_definition->center_of_mass.z);
					}
					error(
						_error_silent,
						"  origin-offset %f %f %f",
						object_definition->object.origin_offset.x,
						object_definition->object.origin_offset.y,
						object_definition->object.origin_offset.z);

					if (object_node_matrix)
					{
						error(
							_error_silent,
							"  parent-node matrix fwd  %f %f %f",
							object_node_matrix->forward.i,
							object_node_matrix->forward.j,
							object_node_matrix->forward.k);
						error(
							_error_silent,
							"                     left %f %f %f",
							object_node_matrix->left.i,
							object_node_matrix->left.j,
							object_node_matrix->left.k);
						error(
							_error_silent,
							"                     up   %f %f %f",
							object_node_matrix->up.i,
							object_node_matrix->up.j,
							object_node_matrix->up.k);
						error(
							_error_silent,
							"                     posn %f %f %f",
							object_node_matrix->position.x,
							object_node_matrix->position.y,
							object_node_matrix->position.z);
						error(_error_silent, "                     scale (jason's ugly secret) %f", object_node_matrix->scale);
					}
					else
					{
						error(_error_silent, "  no parent node");
					}

					error(_error_silent, "");
					error(
						_error_silent,
						"computed matrix fwd  %f %f %f",
						object_nodes[node_stack_index].forward.i,
						object_nodes[node_stack_index].forward.j,
						object_nodes[node_stack_index].forward.k);
					error(
						_error_silent,
						"                left %f %f %f",
						object_nodes[node_stack_index].left.i,
						object_nodes[node_stack_index].left.j,
						object_nodes[node_stack_index].left.k);
					error(
						_error_silent,
						"                up   %f %f %f",
						object_nodes[node_stack_index].up.i,
						object_nodes[node_stack_index].up.j,
						object_nodes[node_stack_index].up.k);
					error(_error_silent, "                posn %f %f %f",
						object_nodes[node_stack_index].position.x,
						object_nodes[node_stack_index].position.y,
						object_nodes[node_stack_index].position.z);
					error(_error_silent, "                scale %f", object_nodes[node_stack_index].scale);

					validated_root_node_matrix = &object_nodes[node_stack_index];
					ocnm_root_matrix_custom(
						"c:\\halo\\SOURCE\\objects\\objects.c",
						2921,
						validated_root_node_matrix,
						"object_compute_node_matrices root node matrix");
				}
			}
			else
			{
				real_orientation *orientation = &node_orientations[node_stack_index];

				matrix4x3_from_orientation(&object_nodes[node_stack_index], orientation);
				match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2929, node->parent_node_index!=NONE);
				matrix4x3_multiply(
					&object_nodes[node->parent_node_index],
					&object_nodes[node_stack_index],
					&object_nodes[node_stack_index]);
			}

			if (node_stack_index==0)
			{
				real_matrix4x3 *final_matrix = &object_nodes[node_stack_index];
				if (!valid_real_matrix4x3(final_matrix))
				{
					char *name2 = tag_get_name(object->definition_index);
					char *string = name2;
					match_assert_valid_real_matrix4x3_internal("c:\\halo\\SOURCE\\objects\\objects.c", 2935, final_matrix, string);
				}
			}

			next_sibling_node_index = node->next_sibling_node_index;

			if (next_sibling_node_index!=NONE)
			{
				node_stack[node_count++] = next_sibling_node_index;
			}
			if (node->first_child_node_index!=NONE)
			{
				node_stack[node_count++] = node->first_child_node_index;
			}

			if (node_index==node_count)
			{
				break;
			}
		}
	}
	else
	{
		object_nodes[0].scale = 1.f;
		object_nodes[0].forward = object->object.forward;
		object_nodes[0].up = object->object.up;
		cross_product3d(&object_nodes[0].up, &object_nodes[0].forward, &object_nodes[0].left);
		object_nodes[0].position = object->object.position;
	}

	matrix4x3_transform_point(object_nodes, &object_definition->object.bounding_offset, &object->object.bounding_sphere_center);
	object->object.bounding_sphere_radius = object_definition->object.bounding_radius;
	
	if (object->object.scale > 0.f)
	{
		object->object.bounding_sphere_radius *= object->object.scale;
	}

	return;
}

#undef ocnm_root_matrix_custom
#undef ocnm_root_matrix_internal
#undef ocnm_parent_matrix_custom
#undef ocnm_parent_matrix_internal

static void object_postprocess_node_matrices(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE &&
		object_definition->object.animation_graph.index != NONE)
	{
		real_matrix4x3 *matrices = (real_matrix4x3 *)object_header_block_get(
				object_index,
				&object->object.node_matrices);
		object_type_postprocess_node_matrices(object_index, matrices);
	}

	return;
}

static void object_choose_random_change_colors(
	long object_index,
	real_rgb_color *placement_change_colors)
{
	short cc_index;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	for (cc_index =0; cc_index<NUMBEROF(object->object.base_change_colors); cc_index++)
	{
		object->object.base_change_colors[cc_index] = placement_change_colors[cc_index];

		if (cc_index<object_definition->object.change_colors.count)
		{
			short permutation_index;

			struct object_change_color_definition *cc = TAG_BLOCK_GET_ELEMENT(
				&object_definition->object.change_colors,
				cc_index,
				struct object_change_color_definition);
			real weight = fmod(
				fabs(
					object->object.position.x*315.89313f +
					object->object.position.y*587.12946f +
					object->object.position.z*744.12415f +
					(real)cc_index*431.12894f
				), 
				1.f);

			for (permutation_index =0; permutation_index<cc->permutations.count; permutation_index++)
			{
				struct object_change_color_permutation *permutation = TAG_BLOCK_GET_ELEMENT(
					&cc->permutations,
					permutation_index,
					struct object_change_color_permutation);
				if (weight<=permutation->weight)
				{
					rgb_colors_interpolate(
						&object->object.base_change_colors[cc_index],
						1,
						&permutation->color_lower_bound,
						&permutation->color_upper_bound,
						fmod(fabs(object->object.position.y) + cc_index*0.71211f, 1.f));
					break;
				}
			}
		}

		object->object.outgoing_change_colors[cc_index].red = PIN(object->object.base_change_colors[cc_index].red, 0.f, 1.f);
		object->object.outgoing_change_colors[cc_index].green = PIN(object->object.base_change_colors[cc_index].green, 0.f, 1.f);
		object->object.outgoing_change_colors[cc_index].blue = PIN(object->object.base_change_colors[cc_index].blue, 0.f, 1.f);
	}

	return;
}

void objects_scripting_detach(
	long parent_object_index,
	long child_object_index)
{
	if (parent_object_index!=NONE &&
		child_object_index!=NONE &&
		object_get(child_object_index)->object.parent_object_index == parent_object_index)
	{
		object_detach(child_object_index);
	}

	return;
}

void object_render_debug(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (debug_objects_root_node)
	{
		render_debug_matrix(TRUE, object_get_node_matrix(object_index, 0), 0.3f);
	}

	if (debug_objects_names)
	{
		if (object->object.name_index!=NONE)
		{
			real_point3d origin;

			struct scenario_object_name* object_name = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->object_names,
				object->object.name_index,
				struct scenario_object_name);
			object_get_origin(object_index, &origin);
			render_debug_string_at_point(FALSE, &origin, object_name->name, global_real_argb_purple);
		}
	}

	if (debug_objects_position_velocity)
	{
		real_vector3d velocity;
		real_matrix4x3 world_matrix;
		
		char* model_name = strrchr(object_definition->object.model.name, '\\');
		object_get_world_matrix(object_index, &world_matrix);
		object_get_velocities(object_index, &velocity, NULL);

		model_name = model_name ? &model_name[1] : object_definition->object.model.name;
		
		render_debug_string_at_point(0, &world_matrix.position, model_name, global_real_argb_orange);
		render_debug_matrix(TRUE, &world_matrix, object->object.bounding_sphere_radius);
		render_debug_vector(TRUE, &world_matrix.position, &velocity, 1.f, global_real_argb_yellow);
	}

	if (debug_objects_bounding_spheres &&
		(object_definition->object.model.index!=NONE ||object_definition->object.collision_model.index!=NONE))
	{
		const real_argb_color *draw_color = global_real_argb_black;

		// Bounding sphere is colored differently based on a couple factors:
		// Red: Invalid bounding sphere
		// Yellow: Active object
		// Black: Inactive object
		if (object->object.bounding_sphere_radius<=0.f)
		{
			draw_color = global_real_argb_red;
		}
		else
		{
			if (TEST_FLAG(object_header_get(object_index)->flags, _object_header_active_bit))
			{
				draw_color = global_real_argb_yellow;
			}
		}

		render_debug_sphere(
			TRUE,
			&object->object.bounding_sphere_center,
			object->object.bounding_sphere_radius>0.f ? object->object.bounding_sphere_radius : 1.f,
			draw_color);
	}

	if (debug_objects_collision_models)
	{
		struct collision_model_instance collision_model_instance;
		if (collision_model_instance_new(&collision_model_instance, object_index))
		{
			render_debug_collision_model(&collision_model_instance);
		}
	}

	if (debug_objects_physics)
	{
		struct physics_instance physics_instance;
		if (physics_instance_new(&physics_instance, object_index))
		{
			render_debug_physics(&physics_instance);
		}
	}

	if (debug_objects_pathfinding_spheres)
	{
		if (!TEST_FLAG(object->object.flags, _object_invisible_bit) &&
			!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		{
			if (
				object->object.type != _object_type_machine ||
				TEST_FLAG(((struct machine_definition *)object_definition)->machine.flags, _machine_is_pathfinding_obstacle_bit) &&
				(
					!TEST_FLAG(
						((struct machine_definition *)object_definition)->machine.flags, 
						_machine_is_not_pathfinding_obstacle_when_open_bit
					) ||
					((struct machine_datum *)object)->device.position!=1.f)
				)
			{
				if (object_definition->object.collision_model.index!=NONE)
				{
					real_matrix4x3 world_matrix;
					short sphere_index;

					struct collision_model* collision_model = collision_model_definition_get(
						object_definition->object.collision_model.index);
					
					object_get_world_matrix(object_index, &world_matrix);

					// Render all pathfinding spheres

					for (sphere_index = 0; sphere_index<collision_model->pathfinding_spheres.count; ++sphere_index)
					{
						real_point3d origin;

						struct pathfinding_sphere *pathfinding_sphere = TAG_BLOCK_GET_ELEMENT(
							&collision_model->pathfinding_spheres,
							sphere_index,
							struct pathfinding_sphere);

						if (pathfinding_sphere->node_index!=NONE)
						{
							real_matrix4x3 *node_matrix = object_get_node_matrix(
								object_index,
								pathfinding_sphere->node_index);
							matrix4x3_transform_point(
								node_matrix,
								&pathfinding_sphere->center,
								&origin);
						}
						else
						{
							matrix4x3_transform_point(
								&world_matrix,
								&pathfinding_sphere->center,
								&origin);
						}
						render_debug_sphere(
							TRUE,
							&origin,
							pathfinding_sphere->radius,
							global_real_argb_blue);
					}


					if (collision_model->pathfinding_box.x0<collision_model->pathfinding_box.x1 &&
						collision_model->pathfinding_box.y0<collision_model->pathfinding_box.y1 &&
						collision_model->pathfinding_box.z0<collision_model->pathfinding_box.z1)
					{
						real_point3d line_points[NUMBER_OF_VERTICES_PER_LINE][NUMBER_OF_VERTICES_PER_LINE][NUMBER_OF_VERTICES_PER_LINE];
						short i;
						short j;
						short k;

						for (i = 0; i<NUMBER_OF_VERTICES_PER_LINE; ++i)
						{
							for (j = 0; j<NUMBER_OF_VERTICES_PER_LINE; ++j)
							{
								for (k = 0; k<NUMBER_OF_VERTICES_PER_LINE; ++k)
								{
									line_points[i][j][k].x = collision_model->pathfinding_box.n[i + 0];	// x0 and x1
									line_points[i][j][k].y = collision_model->pathfinding_box.n[j + 2];	// y0 and y1
									line_points[i][j][k].z = collision_model->pathfinding_box.n[k + 4];	// z0 and z1
									matrix4x3_transform_point(
										&world_matrix,
										&line_points[i][j][k],
										&line_points[i][j][k]);
								}
							}
						}


						for (i = 0; i < NUMBER_OF_VERTICES_PER_LINE; i++)
						{
							for (j = 0; j < NUMBER_OF_VERTICES_PER_LINE; j++)
							{
								for (k = 0; k < NUMBER_OF_VERTICES_PER_LINE; k++)
								{
									if (i==0)
									{
										render_debug_line(
											TRUE,
											&line_points[0][j][k],
											&line_points[1][j][k],
											global_real_argb_blue);
									}
										
									if (j==0)
									{
										render_debug_line(
											TRUE,
											&line_points[i][0][k],
											&line_points[i][1][k],
											global_real_argb_blue);
									}

									if (k==0)
									{
										render_debug_line(
											TRUE,
											&line_points[i][j][0],
											&line_points[i][j][1],
											global_real_argb_blue);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

static void attachments_new(
	long object_index)
{
	short attachment_num;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	for (attachment_num = 0; attachment_num<object_definition->object.attachments.count; attachment_num++)
	{
		struct object_attachment_definition* attachment = TAG_BLOCK_GET_ELEMENT(
			&object_definition->object.attachments,
			attachment_num,
			struct object_attachment_definition);
		short attachment_type = NONE;
		long attachment_index = NONE;

		if (attachment->type.index!=NONE)
		{
			switch (attachment->type.group_tag)
			{
			case LIGHT_DEFINITION_TAG:
				attachment_type = _object_attachment_type_light;
				break;
			case LOOPING_SOUND_DEFINITION_TAG:
				attachment_type = _object_attachment_type_looping_sound;
				break;
			case EFFECT_DEFINITION_TAG:
				attachment_type = _object_attachment_type_effect;
				break;
			case CONTRAIL_DEFINITION_TAG:
				attachment_type = _object_attachment_type_contrail;
				break;
			case PARTICLE_SYSTEM_DEFINITION_TAG:
				attachment_type = _object_attachment_type_particle_system;
				break;
			}
		}

		switch (attachment_type)
		{
		case _object_attachment_type_light:
			attachment_index = light_new(
				attachment->type.index,
				object_index,
				attachment_num,
				attachment->primary_scale_function_reference-1,
				attachment->change_color_reference-1);

			if (attachment_index!=NONE)
			{
				SET_FLAG(object->object.flags, _object_has_attached_lights_bit, TRUE);
			}
			break;
		case _object_attachment_type_looping_sound:
			attachment_index = game_looping_sound_new(
				object_index,
				attachment->type.index,
				attachment->marker_name,
				attachment->primary_scale_function_reference-1);

			if (attachment_index!=NONE)
			{
				SET_FLAG(object->object.flags, _object_has_attached_looping_sounds_bit, TRUE);
			}
			break;
		case _object_attachment_type_effect:
			attachment_index = effect_new_looping(
				attachment->type.index,
				object_index,
				attachment->primary_scale_function_reference-1,
				attachment->secondary_scale_function_reference-1,
				attachment->change_color_reference-1);
			break;
		case _object_attachment_type_contrail:
			attachment_index = contrail_new(attachment->type.index, object_index, attachment_num);
			break;
		case _object_attachment_type_particle_system:
			attachment_index = particle_system_new_attached(attachment->type.index, object_index, attachment_num);
			break;
		}

		object->object.attachment_types[attachment_num] = attachment_type;
		object->object.attachment_indices[attachment_num] = attachment_index;
	}

	return;
}

void object_set_position(
	long object_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up)
{
	struct object_datum *object = object_get(object_index);

	object_disconnect_from_map(object_index);

	if (position)
	{
		object->object.position = *position;
	}

	if (forward)
	{
		object->object.forward = *forward;
		if (up)
		{
			object->object.up = *up;
		}
		else
		{
			real_vector3d right = {forward->j, -forward->i, 0.f};
			if (normalize3d(&right)==0.f)
			{
				right.i = 1.f;
				right.k = 0.f;
				right.j = 0.f;
			}
			cross_product3d(&right, forward, &object->object.up);
		}
	}

	object_compute_node_matrices(object_index);
	object_reconnect_to_map(object_index, NULL);

	return;
}

void object_translate(
	long object_index,
	real_point3d const *new_position,
	struct location const *new_location)
{
	struct object_datum *object = object_get(object_index);

	match_assert_valid_real_point3d(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		562, 
		new_position);

	object_disconnect_from_map(object_index);
	object->object.position = *new_position;
	object_reconnect_to_map(object_index, new_location);

	return;
}

long object_new(
	struct object_placement_data *data)
{
	long object_index = NONE;
	long definition_index = data->definition_index;

	match_assert_valid_real_point3d("c:\\halo\\SOURCE\\objects\\objects.c", 618, &data->position)
	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\objects\\objects.c", 619, &data->forward, &data->up);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\objects\\objects.c", 620, &data->angular_velocity);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\objects\\objects.c", 621, &data->translational_velocity);

	if (game_engine_running() && definition_index!=NONE)
	{
		definition_index = game_engine_remap_object_definition(definition_index);
	}

	if (definition_index!=NONE)
	{
		struct object_definition *object_definition = object_definition_get(definition_index);
		struct object_type_definition *type_definition = object_type_definition_get(object_definition->object.type);

		object_index = object_header_new(object_header_data, NONE, type_definition->game_datum_size);

		if (object_index!=NONE)
		{
			short node_count;

			struct object_header_datum *object_header = object_header_get(object_index);
			struct object_datum *object = object_get(object_index);
			boolean success = TRUE;

			SET_FLAG(object_header->flags, _object_header_being_created_bit, TRUE);
			SET_FLAG(object_header->flags, _object_header_automatically_deactivate_bit, TRUE);

			object_header->type = object_definition->object.type;
			object->definition_index = definition_index;
			object->object.type = object_definition->object.type;
			
			object_type_adjust_placement(object_index, data);
			
			object->object.position = data->position;
			object->object.forward = data->forward;
			object->object.up = data->up;
			object->object.translational_velocity = data->translational_velocity;
			object->object.angular_velocity = data->angular_velocity;
			
			// Preserve the January inline schedule without emitting a point_from_line3d COMDAT.
			{
				real_point3d const *p = &object->object.position;
				real_vector3d const *v = &object->object.up;
				real t = data->height;
				real_point3d *result = &object->object.position;
				real height = t;

				result->x = (v->i*height) + p->x;
				result->y = (v->j*height) + p->y;
				result->z = (v->k*height) + p->z;
			}

			SET_FLAG(object->object.flags, _object_mirrored_bit, TEST_FLAG(data->flags, _new_object_mirrored_bit));

			object->object.location.cluster_index = NONE;
			object_header->cluster_index = NONE;

			object->object.magic_number = global_object_marker-1;
			object->object.umbrella_shield_object_index = NONE;
			object->object.first_cluster_reference_index = NONE;
			object->object.animation.state.index = NONE;
			object->object.animation.animation_graph_index = object_definition->object.animation_graph.index;
			object->object.cached_render_state_index = NONE;
			object->object.parent_object_index = NONE;
			object->object.next_object_index = NONE;
			object->object.first_child_object_index = NONE;
			object->object.name_index = NONE;
			object->object.shield_damage_decay_timer = NONE;
			object->object.body_damage_decay_timer = NONE;

			if (TEST_FLAG(object_definition->object.flags, _object_does_not_cast_shadow_bit))
			{
				SET_FLAG(object->object.flags, _object_shadowless_bit, TRUE);
			}

			SET_FLAG(object->object.flags, _object_has_collision_model_bit, object_definition->object.collision_model.index!=NONE);

			object_set_visibility(object_index, object_definition->object.model.index!=NONE);
			object->object.owner_team_index = data->owner_team_index;
			object->object.owner_player_index = data->owner_player_index;
			object->object.owner_object_index = data->owner_object_index;
			object->object.variant_number = data->variant_number;
			object->object.forced_shader_permutation_index = object_definition->object.forced_shader_permutation_index;

			if (object_definition->object.model.index==NONE)
			{
				node_count =1;
			}
			else
			{
				node_count = model_definition_get(object_definition->object.model.index)->nodes.count;
			}

			if (object_header_block_allocate(
				object_index,
				offsetof(struct object_datum, object.node_matrices),
				sizeof(struct real_matrix4x3) * node_count))
			{
				if (!TEST_FLAG(_object_mask_cannot_interpolate, object_definition->object.type) &&
					(
						!object_header_block_allocate(
							object_index,
							offsetof(struct object_datum, object.node_orientations),
							sizeof(struct real_orientation) * node_count
						) ||
						!object_header_block_allocate(
							object_index,
							offsetof(struct object_datum, object.original_node_orientations),
							sizeof(struct real_orientation) * node_count
						)
					)
				)
				{
					success = FALSE;
				}
			}
			else	
			{
				success = FALSE;
			}

			// Get the object, we do this so we can verify that we're able to retrieve it in the future
			object = object_get(object_index);

			if (success && object_type_new(object_index))
			{
				boolean original_deleted_when_deactivated = TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit);

				if (original_deleted_when_deactivated &&
					TEST_FLAG(data->flags, _new_object_never_automatically_delete_bit))
				{
					SET_FLAG(object->object.flags, _object_deleted_when_deactivated_bit, FALSE);
				}

				object_choose_random_change_colors(object_index, data->change_colors);
				object_choose_random_region_permutations(object_index);
				object_initialize_vitality(object_index, NULL, NULL);
				object_compute_node_matrices(object_index);
				object_reconnect_to_map(object_index, NULL);
				object_postprocess_node_matrices(object_index);
				object_type_export_function_values(object_index);
				object_compute_function_values(object_index);
				object_compute_change_colors(object_index);

				object_create_attachments(object_index);

				// Restore value
				SET_FLAG(object->object.flags, _object_deleted_when_deactivated_bit, original_deleted_when_deactivated);

				if (!TEST_FLAG(object_header->flags, _object_header_active_bit) &&
					TEST_FLAG(object->object.flags, _object_deleted_when_deactivated_bit))
				{
					if (TEST_FLAG(data->flags, _new_object_never_automatically_delete_bit))
					{
						if (object->object.location.cluster_index!=NONE)
						{
							object_delete(object_index);
						}
					}
					else
					{
						object_delete(object_index);
					}
				}
			}
			else
			{
				success = FALSE;
			}

			if (!success)
			{
				object_type_delete(object_index);
				object_header_delete(object_header_data, object_index);
				object_index = NONE;
			}
			else
			{
				if (object_definition->object.creation_effect.index!=NONE)
				{
					effect_new_from_object(
						object_definition->object.creation_effect.index,
						object_index,
						object_index,
						NONE,
						0.f,
						0.f,
						NULL,
						NULL);
				}
			}
		}
	}

	if (object_index==NONE && definition_index!=NONE)
	{
		char string[512];
		sprintf(string, "OUT OF OBJECTS: cannot create %s", tag_name_strip_path(tag_get_name(definition_index)));
		console_printf(FALSE, "%s", string);
		error(_error_log, "%s", string);
	}
	
	return object_index;
}


void object_attach_to_node(
	long parent_object_index,
	long child_object_index,
	short parent_node_index)
{
	long object_index;
	struct object_datum *object;
	void *(*get_object)(
		long,
		unsigned long) = object_get_and_verify_type;
	boolean (*has_node)(
		long,
		short) = object_has_node;
	real_matrix4x3 *(*get_node_matrix)(
		long,
		short) = object_get_node_matrix;

	boolean valid = TRUE;

	for (
		object_index = parent_object_index;
		object_index!=NONE;
		object_index = object->object.parent_object_index)
	{
		object = object_get(object_index);

		if (object_index==child_object_index)
		{
			valid = FALSE;
			break;
		}
	}

	match_vassert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		1225,
		valid,
		"cannot attach an object to one of its children");

	if (valid)
	{
		real_matrix4x3 inverse_node_matrix;

		struct object_datum *child_object = (struct object_datum *)get_object(child_object_index, _object_mask_all);
		struct object_datum *parent_object = (struct object_datum *)get_object(parent_object_index, _object_mask_all);
		boolean connected_to_map = TEST_FLAG(child_object->object.flags, _object_connected_to_map_bit);

		match_vassert(
			"c:\\halo\\SOURCE\\objects\\objects.c",
			1235,
			has_node(parent_object_index, parent_node_index),
			"object_has_node(parent_object_index, parent_node_index)");

		if (connected_to_map)
		{
			object_disconnect_from_map(child_object_index);
		}
		
		matrix4x3_inverse(get_node_matrix(parent_object_index, parent_node_index), &inverse_node_matrix);
		matrix4x3_transform_point(&inverse_node_matrix, &child_object->object.position, &child_object->object.position);
		matrix4x3_transform_normal(&inverse_node_matrix, &child_object->object.forward, &child_object->object.forward);
		matrix4x3_transform_normal(&inverse_node_matrix, &child_object->object.up, &child_object->object.up);

		child_object->object.parent_object_index = parent_object_index;
		child_object->object.parent_node_index = parent_node_index;

		if (connected_to_map)
		{
			object_reconnect_to_map(child_object_index, NULL);
		}

		{
			struct object_header_datum *header = object_header_get(child_object_index);
			struct object_datum *deactivated_object = (struct object_datum *)get_object(child_object_index, _object_mask_all);

			if (TEST_FLAG(header->flags, _object_header_active_bit))
			{
				SET_FLAG(header->flags, _object_header_active_bit, FALSE);
			}
		}

		{
			struct object_header_datum *header = object_header_get(child_object_index);
			SET_FLAG(header->flags, _object_header_do_not_update_bit, TRUE);
		}

		object_compute_node_matrices(child_object_index);
	}
	
	return;
}

boolean object_force_inside_bsp(
	long object_index,
	real_point3d const *known_good_point)
{
	struct collision_result collision;

	struct object_datum *object = object_get(object_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2365, global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
	global_current_collision_users[global_current_collision_user_depth++] = 19;
	
	if (collision_test_line(_collision_test_for_projectiles_flags, known_good_point, &object->object.position, NONE, &collision) ||
		object->object.location.cluster_index==NONE)
	{
		if (collision.location.cluster_index!=NONE)
		{
			object_translate(object_index, &collision.point, &collision.location);
			object_compute_node_matrices(object_index);
			result = TRUE;
		}
	}
	else
	{
		result = TRUE;
	}

	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2387, global_current_collision_user_depth > 1);
	--global_current_collision_user_depth;

	return result;
}

boolean object_update(
	long object_index)
{
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	boolean result = TRUE;
	if (!TEST_FLAG(header->flags, _object_header_do_not_update_bit))
	{
		if (TEST_FLAG(object->object.flags, _object_garbage_bit))
		{
			++object_globals->active_garbage_object_count;
		}

		if (object->object.animation.interpolation_frame_count)
		{
			match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 2508, !TEST_FLAG(_object_mask_cannot_interpolate, object->object.type));
			if (++object->object.animation.interpolation_frame_index>=
				object->object.animation.interpolation_frame_count)
			{
				object->object.animation.interpolation_frame_count = 0;
			}
		}

		object_type_update(object_index);
		if (object_definition->object.collision_model.index!=NONE)
		{
			object_damage_update(object_index);
		}

		object_type_export_function_values(object_index);

		if (!TEST_FLAG(object->object.flags, _object_do_not_recompute_node_matrices_bit))
		{
			object_compute_node_matrices(object_index);
		}

		object_compute_function_values(object_index);
		object_compute_change_colors(object_index);

		if (
			TEST_FLAG(object->object.flags, _object_dynamic_lighting_recompute_bit) &&
			(
				!TEST_FLAG(object->object.flags, _object_invisible_bit) ||
				object_definition_get(object->definition_index)->object.model.index==NONE
			)
		)
		{
			object_connect_lights(object_index, TRUE, TRUE);
		}

		// Update children (if we have any)
		if (object->object.first_child_object_index!=NONE)
		{
			object_update(object->object.first_child_object_index);
		}

		if (object->object.parent_object_index!=NONE)
		{
			if (object->object.next_object_index!=NONE)
			{
				object_update(object->object.next_object_index);
			}
		}

		object_postprocess_node_matrices(object_index);
	}

	return result;
}

void object_compute_node_matrices_recursive(
	long object_index)
{
	long child_index;
	struct object_datum *child_object;
	struct object_datum *object = object_get(object_index);
	
	object_compute_node_matrices(object_index);

	for (
		child_index = object->object.first_child_object_index;
		child_index!=NONE;
		child_index = child_object->object.next_object_index)
	{
		child_object = object_get(child_index);
		object_compute_node_matrices_recursive(child_index);
	}
	
	return;
}

long object_new_from_scenario(
	struct scenario_object_datum *scenario_object,
	struct tag_block *palette)
{
	struct object_placement_data placement_data;
	
	long result = NONE;

	if (scenario_object->palette_entry_index!=NONE &&
		(
			!object_globals->initial_placement ||
			!TEST_FLAG(scenario_object->placement_flags, _scenario_object_placement_not_automatic_bit)
		))
	{
		if (scenario_object->name_index==NONE ||
			object_name_list_lookup(scenario_object->name_index)==NONE)
		{
			long definition_index = TAG_BLOCK_GET_ELEMENT(
				palette,
				scenario_object->palette_entry_index,
				struct scenario_object_palette_entry
			)->reference.index;

			if (definition_index!=NONE)
			{
				object_placement_data_new(&placement_data, definition_index, NONE);
				placement_data.position = scenario_object->position;
				vectors3d_from_euler_angles3d(
					&placement_data.forward,
					&placement_data.up,
					&scenario_object->rotation);
				placement_data.variant_number = scenario_object->variant_number;
				
				result = object_new(&placement_data);
				if (result!=NONE)
				{
					object_type_place(result, scenario_object);
					if (scenario_object->name_index!=NONE)
					{
						object_name_list_new(result, scenario_object->name_index);
					}
				}
			}
		}
	}

	return result;
}

void object_attach_to_marker(
	long parent_object_index,
	char const *parent_marker_name,
	long child_object_index,
	char const *child_marker_name)
{
	struct object_marker parent_marker;
	struct object_marker child_marker;
	real_matrix4x3 inverted_matrix;

	struct object_datum *child_object = object_get(child_object_index);

	object_get_marker_by_name(parent_object_index, parent_marker_name, &parent_marker, 1);
	object_get_marker_by_name(child_object_index, child_marker_name, &child_marker, 1);
	object_disconnect_from_map(child_object_index);
	
	if (child_marker_name && *child_marker_name)
	{
		object_align_marker_to_matrix(child_object, &child_marker, &parent_marker.matrix);
	}
	else
	{
		matrix4x3_inverse(&child_marker.node_matrix, &inverted_matrix);
		matrix4x3_transform_point(&inverted_matrix, &parent_marker.matrix.position, &child_object->object.position);
		matrix4x3_transform_normal(&inverted_matrix, &parent_marker.matrix.forward, &child_object->object.forward);
		matrix4x3_transform_normal(&inverted_matrix, &parent_marker.matrix.up, &child_object->object.up);
	}

	object_reconnect_to_map(child_object_index, NULL);
	object_attach_to_node(parent_object_index, child_object_index, parent_marker.node_index);

	return;
}

long object_new_by_name(
	short name_index)
{
	long placement_tag_block_element_size;
	struct scenario *scenario = global_scenario_get();


	struct scenario_object_name* name = TAG_BLOCK_GET_ELEMENT(
		&scenario->object_names,
		name_index,
		struct scenario_object_name);

	struct tag_block* datum = scenario_get_object_type_scenario_datums(
		scenario,
		name->runtime_object_type,
		&placement_tag_block_element_size);

	struct tag_block *palette = scenario_get_object_type_scenario_palette(
		scenario,
		name->runtime_object_type);

	struct scenario_object_datum *object = (struct scenario_object_datum *)tag_block_get_element_with_size(
			datum,
			name->runtime_scenario_datum_index,
			placement_tag_block_element_size);

	return object_new_from_scenario(
		object,
		palette);
}

void objects_scripting_attach(
	long parent_object_index,
	char const *parent_marker_name,
	long child_object_index,
	char const *child_marker_name)
{
	if (parent_object_index!=NONE &&
		child_object_index !=NONE &&
		object_get(child_object_index)->object.parent_object_index==NONE)
	{
		object_attach_to_marker(parent_object_index, parent_marker_name, child_object_index, child_marker_name);
	}

	return;
}

void object_delete_immediately(
	long object_index)
{
	object_delete_initial_recursive(object_index, FALSE);
	object_delete_recursive(object_index, FALSE);

	return;
}

void objects_garbage_collection(
	void)
{
	short garbage_collect_mode = NONE;

	if (object_globals->force_garbage_collection)
	{
		garbage_collect_mode = _garbage_collect_everything;
	}
	else if (memory_pool_get_contiguous_free_size(object_memory_pool)<=GARBAGE_LIMIT_FREE_MEMORY_CRITICAL)
	{
		memory_pool_compact(object_memory_pool);
		
		if (memory_pool_get_contiguous_free_size(object_memory_pool)<=GARBAGE_LIMIT_FREE_MEMORY_TRIGGER)
		{
			garbage_collect_mode = _garbage_collect_for_space;
		}
	}
	else
	{
		if (MAXIMUM_OBJECTS_PER_MAP-object_header_data->actual_count<=GARBAGE_LIMIT_FREE_OBJECTS_TRIGGER)
		{
			garbage_collect_mode = _garbage_collect_for_space;
		}
		else
		{
			if (object_globals->active_garbage_object_count>=GARBAGE_LIMIT_ACTIVE_GARBAGE_TRIGGER)
			{
				garbage_collect_mode = _garbage_collect_active_objects;
			}
		}
	}
	
	if (garbage_collect_mode!=NONE)
	{
		long garbage_collect_mode_wide;
		short garbage_object_count = 0;
		boolean should_collect = FALSE;

		{
			long garbage_object_indices[MAXIMUM_OBJECTS_PER_MAP];

			if (debug_object_garbage_collection)
			{
				console_printf(
					FALSE,
					"#%d objects using 0x%x bytes (0x%x contiguous free)",
					object_header_data->actual_count,
					OBJECT_MEMORY_POOL_SIZE - memory_pool_get_free_size(object_memory_pool),
					memory_pool_get_contiguous_free_size(object_memory_pool));
			}

			{
				long garbage_object_index;
				struct object_datum *object;

				for (
					garbage_object_index = object_globals->first_garbage_object_index;
					garbage_object_index!=NONE;
					garbage_object_index = object->object.next_garbage_object_index)
				{
					object = object_get(garbage_object_index);

					match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4289, garbage_object_count<MAXIMUM_OBJECTS_PER_MAP);

					garbage_object_indices[garbage_object_count++] = garbage_object_index;
				}
			}

			garbage_collect_mode_wide = garbage_collect_mode;

			while (TRUE)
			{
				long object_index;
				struct object_header_datum *header;
				boolean garbage_collect;

				switch (garbage_collect_mode_wide)
				{
				case _garbage_collect_everything:
					should_collect = FALSE;
					break;
				case _garbage_collect_active_objects:
					should_collect = object_globals->active_garbage_object_count<=GARBAGE_LIMIT_ACTIVE_GARBAGE_TARGET;
					break;
				case _garbage_collect_for_space:
					should_collect =
						memory_pool_get_free_size(object_memory_pool)>=GARBAGE_LIMIT_FREE_MEMORY_TRIGGER &&
						MAXIMUM_OBJECTS_PER_MAP-object_header_data->count>=GARBAGE_LIMIT_FREE_OBJECTS_TARGET;
					break;
				default:
					match_vassert("c:\\halo\\SOURCE\\objects\\objects.c", 4314, FALSE, NULL);
					break;
				}

				if (should_collect || garbage_object_count==0)
				{
					break;
				}


				object_index = garbage_object_indices[--garbage_object_count];
				header = object_header_get(object_index);
				garbage_collect = TRUE;

				if (garbage_collect_mode==_garbage_collect_active_objects)
				{
					garbage_collect = TEST_FLAG(header->flags, _object_header_active_bit);
				}

				if (object_visible_to_any_player(object_index))
				{
					garbage_collect = FALSE;
				}

				if (garbage_collect)
				{
					struct object_datum *garbage_object = object_get(object_index);

					if (TEST_FLAG(_object_mask_unit, garbage_object->object.type) &&
						!TEST_FLAG(garbage_object->object.damage_flags, _object_dead_bit))
					{
						error(
							_error_silent,
							"WARNING: garbage collecting a living unit (%s)",
							ai_debug_describe_actor(NONE, object_index, TRUE, temporary, NUMBEROF(temporary)));
					}


					if (TEST_FLAG(header->flags, _object_header_active_bit))
					{
						--object_globals->active_garbage_object_count;
					}

					object_set_garbage(object_index, FALSE);
					object_delete_immediately(object_index);
				}
			}

			}

		memory_pool_compact(object_memory_pool);
		
		if (debug_object_garbage_collection)
		{
			console_printf(
				FALSE,
				"compacted to #%d with 0x%x contiguous bytes free",
				object_header_data->actual_count,
				memory_pool_get_contiguous_free_size(object_memory_pool));
		}

		if (!should_collect)
		{
			char warningbuf[512];
			unsigned char release_proc_working_memory[4096];
			char released_resultbuf[512];
			const struct object_memory_release_function *current_release_procs = object_memory_release_procs;
			boolean v0 = FALSE;
			boolean garbage_collection_after_first_attempt = FALSE;
			boolean garbage_should_warn = object_globals->last_garbage_warn_time==NONE || object_globals->last_garbage_warn_time+150<game_time_get();
			boolean update_time = FALSE;

			while (TRUE)
			{
				boolean debug_garbage_collection = FALSE;
				boolean status_still_critical = FALSE;

				switch (garbage_collect_mode_wide)
				{
				case _garbage_collect_for_space:
				{
					long free_size = memory_pool_get_contiguous_free_size(object_memory_pool);
					long free_objects = MAXIMUM_OBJECTS_PER_MAP-object_header_data->count;
					boolean debug_slots_free = FALSE;

					if (free_size<=GARBAGE_LIMIT_FREE_MEMORY_CRITICAL/2)
					{
						status_still_critical = TRUE;
						debug_garbage_collection = TRUE;
					}
					else
					{
						if (free_objects<=GARBAGE_LIMIT_FREE_OBJECTS_CRITICAL)
						{
							status_still_critical = TRUE;
							debug_garbage_collection = TRUE;
							debug_slots_free = TRUE;
						}
						else
						{
							if (free_size<=GARBAGE_LIMIT_FREE_MEMORY_CRITICAL)
							{
								debug_garbage_collection = TRUE;
							}
							else
							{
								if (free_objects<=GARBAGE_LIMIT_FREE_OBJECTS_TRIGGER)
								{
									debug_garbage_collection = TRUE;
									debug_slots_free = TRUE;
								}
							}
						}
					}

					if (debug_slots_free)
					{
						sprintf(warningbuf, "%d slots free", free_objects);
					}
					else
					{
						sprintf(warningbuf, "%4.2f%% memory free", ((free_size * 100.f) / (real)OBJECT_MEMORY_POOL_SIZE));
					}
				}
					break;
				}

				if (status_still_critical || garbage_collection_after_first_attempt)
				{
					char tempbuffer[512];
					const char *status;
					
					if (garbage_collection_after_first_attempt)
					{
						status = status_still_critical ? "still " : "not ";
					}
					else
					{
						status = "";
					}

					sprintf(tempbuffer, "garbage collection %scritical (%s)", status, warningbuf);
					console_printf(FALSE, "%s", tempbuffer);
					error(_error_log, "%s", tempbuffer);
					update_time = TRUE;
				}
				else if (debug_garbage_collection && garbage_should_warn)
				{
					error(_error_silent, "garbage collection warning (%s)", warningbuf);
					update_time = TRUE;
				}

				if (status_still_critical)
				{
					if (current_release_procs->function)
					{
						boolean result = FALSE;

						while (current_release_procs->function && !result)
						{
							boolean more_to_release = FALSE;

							if (!v0 && current_release_procs->init_function)
							{
								current_release_procs->init_function(
									release_proc_working_memory,
									sizeof(release_proc_working_memory));
								v0 = TRUE;
							}

							result = current_release_procs->function(
								released_resultbuf,
								&more_to_release,
								release_proc_working_memory,
								sizeof(release_proc_working_memory));

							if (result)
							{
								char tempbuffer[512];

								sprintf(tempbuffer, "removing objects: %s", released_resultbuf);
								console_printf(FALSE, "%s", tempbuffer);
								error(_error_log, "%s", tempbuffer);
							}

							if (!more_to_release)
							{
								++current_release_procs;
								v0 = FALSE;
							}
						}

						if (!result)
						{
							break;
						}

						garbage_collection_after_first_attempt = TRUE;
						memory_pool_compact(object_memory_pool);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}

			if (update_time)
			{
				object_globals->last_garbage_warn_time = game_time_get();
			}
		}
	}

	object_globals->force_garbage_collection = FALSE;

	return;
}


void objects_update(
	void)
{
	static struct profile_section section = {"objects_update", NONE, TRUE};

	unsigned long *last_active_cluster_bits;
	unsigned long *active_cluster_bits;
	short cluster_count;
	short i;
	struct object_header_datum *object_header;

	boolean dont_update_object = ((game_time_get()&1)!=0) && game_players_are_double_speed();

	profile_enter(section);

	object_globals->active_garbage_object_count = 0;
	
	last_active_cluster_bits = object_globals->last_active_cluster_bits;
	active_cluster_bits = object_globals->active_cluster_bits;
	cluster_count = global_structure_bsp_get()->clusters.count;

	memcpy(last_active_cluster_bits, active_cluster_bits, BIT_VECTOR_SIZE_IN_BYTES(cluster_count));
	memcpy(active_cluster_bits, players_get_combined_pvs(), BIT_VECTOR_SIZE_IN_BYTES(cluster_count));

	if (csmemcmp(last_active_cluster_bits, active_cluster_bits, BIT_VECTOR_SIZE_IN_BYTES(cluster_count)))
	{
		object_header = (struct object_header_datum *)object_header_data->data;
		for (i = 0; i<object_header_data->count; ++object_header)
		{
			if (object_header->identifier &&
				TEST_FLAG(object_header->flags, _object_header_automatically_deactivate_bit) &&
				TEST_FLAG(object_header->flags, _object_header_connected_to_map_bit))
			{
				if (TEST_FLAG(object_header->flags, _object_header_active_bit))
				{
					match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 369, object_header->cluster_index!=NONE);
					if (!BIT_VECTOR_TEST_FLAG(active_cluster_bits, object_header->cluster_index))
					{
						if (TEST_FLAG(object_header->datum->object.flags, _object_deleted_when_deactivated_bit))
						{
							object_delete_initial_recursive(i, FALSE);
						}
						else
						{
							object_deactivate(i);
						}
					}
				}
				else if (
					!TEST_FLAG(object_header->flags, _object_header_child_bit) &&
					object_header->cluster_index != NONE &&
					BIT_VECTOR_TEST_FLAG(active_cluster_bits, object_header->cluster_index)
				)
				{
					object_activate(i);
				}
			}

			++i;
		}

		structure_decals_update(
			last_active_cluster_bits,
			active_cluster_bits,
			cluster_count);
	}

	object_header = (struct object_header_datum *)object_header_data->data;
	for (i = 0; i<object_header_data->count; ++object_header)
	{
		if (object_header->identifier)
		{
			if (TEST_FLAG(object_header->flags, _object_header_active_bit) &&
				!TEST_FLAG(object_header->flags, _object_header_being_created_bit))
			{
				long object_index = DATUM_INDEX_NEW(i, object_header->identifier);
				match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 416, object_get(object_index)->object.parent_object_index==NONE);
				match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 417, object_get(object_index)->object.next_object_index==NONE);
			
				if (!dont_update_object ||
					TEST_FLAG(_object_mask_unit, object_header->type) &&
					unit_get(object_index)->unit.player_index!=NONE)
				{
					object_update(object_index);
				}
			}
		}
		++i;
	}

	object_header = (struct object_header_datum *)object_header_data->data;
	for (i = 0; i<object_header_data->count; ++object_header)
	{
		if (object_header->identifier)
		{
			SET_FLAG(object_header->flags, _object_header_do_not_update_bit, FALSE);

			if (TEST_FLAG(object_header->flags, _object_header_being_created_bit))
			{
				SET_FLAG(
					object_header->flags,
					_object_header_being_created_bit,
					FALSE);
				object_update(DATUM_INDEX_NEW(i, object_header->identifier));
			}

			if (TEST_FLAG(object_header->flags, _object_header_being_deleted_bit))
			{
				object_delete_recursive(DATUM_INDEX_NEW(i, object_header->identifier), FALSE);
			}
		}
		++i;
	}

	objects_garbage_collection();

	profile_exit(section);

	return;
}

boolean debug_objects_bounding_spheres = TRUE;
boolean debug_objects_collision_models = TRUE;

void objects_memory_compact(
	void)
{
	objects_garbage_collection();
	memory_pool_compact(object_memory_pool);
	return;
}

/* ---------- private code */

static void object_connect_lights(
	long object_index,
	boolean disconnect,
	boolean reconnect)
{
	struct object_datum *object = object_get(object_index);

	if (TEST_FLAG(object->object.flags, _object_has_attached_lights_bit))
	{
		short i;
		struct object_definition *object_definition = object_definition_get(object->definition_index);
		
		for (i =0; i<object_definition->object.attachments.count; ++i)
		{
			if (!object->object.attachment_types[i] && object->object.attachment_indices[i]!=NONE)
			{
				if (disconnect)
				{
					light_disconnect_from_map(object->object.attachment_indices[i]);
				}
				if (reconnect)
				{
					light_reconnect_to_map(object->object.attachment_indices[i]);
				}
			}
		}
	}

	return;
}

static void object_name_list_allocate(
	void)
{
	object_name_list = (long*)game_state_malloc("object name list", NULL, MAXIMUM_OBJECT_NAMES_PER_SCENARIO * sizeof(*object_name_list));
	match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 4072, object_name_list);

	return;
}

static void object_name_list_free(
	void)
{
	// Bungie never free'd this properly

	return;
}

static void object_name_list_clear(
	void)
{
	memset(object_name_list, NONE, MAXIMUM_OBJECT_NAMES_PER_SCENARIO * sizeof(*object_name_list));

	return;
}

static void object_name_list_new(
	long object_index,
	short name_index)
{
	struct object_datum *object = object_get(object_index);

	match_assert(
		"c:\\halo\\SOURCE\\objects\\objects.c",
		4099,
		name_index>=0 && name_index<MAXIMUM_OBJECT_NAMES_PER_SCENARIO);

	if (object_name_list[name_index]==NONE)
	{
		object_name_list[name_index] = object_index;
		object->object.name_index = name_index;
	}
	else
	{
		error(
			_error_silent,
			"an object with the name '%s' already exists!", 
			TAG_BLOCK_GET_ELEMENT(&global_scenario_get()->object_names, name_index, struct scenario_object_name)->name);
	}

	return;
}

static void object_name_list_delete(
	long object_index)
{
	struct object_datum *object = object_get(object_index);

	if (object->object.name_index!=NONE)
	{
		short i;
		struct scenario *scenario = global_scenario_get();
		object->object.name_index = NONE;

		for (i =0; i<scenario->object_names.count; ++i)
		{
			if (object_name_list[i]==object_index)
			{
				object_name_list[i] = NONE;
			}
		}
	}

	return;
}

static long object_name_list_lookup(
	short name_index)
{
	return VALID_INDEX(name_index, MAXIMUM_OBJECT_NAMES_PER_SCENARIO) ? object_name_list[name_index] : NONE;
}

static long recursive_object_adder(
	long object_index,
	boolean(*add_object_function)(long, void *),
	void *custom_data,
	long object_count,
	long maximum_count,
	long *object_indices)
{
	struct object_datum *object = object_get(object_index);
	
	if (object_count<maximum_count)
	{
		if (!add_object_function || add_object_function(object_index, custom_data))
		{
			object_indices[object_count++] = object_index;
		}
		
		if (object->object.first_child_object_index!=NONE)
		{
			object_count = recursive_object_adder(
				object->object.first_child_object_index,
				add_object_function,
				custom_data,
				object_count,
				maximum_count,
				object_indices);
		}

		if (object->object.next_object_index!=NONE)
		{
			object_count = recursive_object_adder(
				object->object.next_object_index,
				add_object_function,
				custom_data,
				object_count,
				maximum_count,
				object_indices);
		}
	}

	return object_count;
}

static long sort_dumps(
	struct dump_datum const *dump1,
	struct dump_datum const *dump2)
{
	long result;
	if (dump1->total_size < dump2->total_size)
	{
		result = 1;
	}
	else if (dump1->total_size <= dump2->total_size)
	{
		result = 0;
	}
	else
	{
		result = -1;
	}

	return result;
}

static void object_add_to_dump(
	long object_index,
	struct dump_datum *dump)
{
	long parent_index;
	struct object_datum *parent;
	struct object_header_datum *header = object_header_get(object_index);
	struct object_datum *object = object_get(object_index);


	if (header->data_size>dump->maximum_size)
	{
		dump->maximum_size = header->data_size;
	}

	dump->total_size+= header->data_size;
	dump->count++;

	if (TEST_FLAG(header->flags, _object_header_active_bit))
	{
		dump->active_count++;
	}

	if (TEST_FLAG(object->object.flags, _object_garbage_bit))
	{
		dump->garbage_count++;
	}

	if (TEST_FLAG(object->object.damage_flags, _object_dead_bit))
	{
		dump->dead_count++;
	}

	if (TEST_FLAG(object->object.flags, _object_at_rest_bit))
	{
		dump->at_rest_count++;
	}

	parent_index = object_get_ultimate_parent(object_index);
	parent = object_get(parent_index);
	if (TEST_FLAG(parent->object.flags, _object_outside_of_map_bit) || parent->object.location.cluster_index==NONE)
	{
		dump->outside_map_count++;
	}
	return;
}

static void object_dump_write(
	FILE *stream,
	struct dump_datum *dump)
{
	const char *name = "unknown";
	if (dump->definition_index!=NONE)
	{
		name = tag_get_name(dump->definition_index);
	}
	else
	{
		if (dump->object_type!=NONE)
		{
			name = object_type_get_name(dump->object_type);
		}
	}

	fprintf(
		stream,
		"% 6d (% 6d) [% 7d/% 7d/% 7d/% 7d] % 7d % 7d %s\r\n",
		dump->count,
		dump->active_count,
		dump->garbage_count,
		dump->dead_count,
		dump->outside_map_count,
		dump->at_rest_count,
		dump->maximum_size,
		dump->total_size,
		name);

	return;
}

static short object_find_region_permutations_available_with_variant(
	struct model_region *region,
	short variant_number,
	short *available_indices)
{
	short i;
	short result = 0;

	for (i = 0; i<region->permutations.count; i++)
	{
		struct model_region_permutation* permutation = TAG_BLOCK_GET_ELEMENT(
			&region->permutations,
			i,
			struct model_region_permutation);
		if (!TEST_FLAG(permutation->flags, _model_region_permutation_cannot_be_chosen_randomly_bit))
		{
			if (permutation->variant_number==variant_number ||
				variant_number==NONE && permutation->variant_number< 100)
			{
				available_indices[result++] = i;
			}
		}
	}

	return result;
}

static boolean object_select_random_region_permutations_by_variant(
	long object_index,
	struct model *model,
	short variant_number)
{
	short available_permutation_indices[32];
	short region_index;

	struct object_datum *object = object_get(object_index);
	boolean result = TRUE;

	for (region_index =0; region_index<model->regions.count; region_index++)
	{
		struct model_region *region = TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);
		short permutation_index = object_find_region_permutations_available_with_variant(region, variant_number, available_permutation_indices);
		
		if (permutation_index==0 && variant_number!=NONE)
		{
			permutation_index = object_find_region_permutations_available_with_variant(region, 0, available_permutation_indices);
		}

		if (!permutation_index)
		{
			object->object.region_permutations[region_index] = 0;
			result = FALSE;
		}
		else
		{
			short permutation_num = permutation_index==1 ? 0 : random_range(0, permutation_index);
			object->object.region_permutations[region_index] = available_permutation_indices[permutation_num];
		}
	}

	return result;
}

static short object_determine_variant_number(
	long object_index,
	struct model *model)
{
	short region_index;

	struct object_datum *object = object_get(object_index);
	short result = 0;

	for (region_index =0; region_index<model->regions.count&& !result; region_index++)
	{
		struct model_region* region = TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);
		if (object->object.region_permutations[region_index] < region->permutations.count)
		{
			result = TAG_BLOCK_GET_ELEMENT(
				&region->permutations,
				object->object.region_permutations[region_index],
				struct model_region_permutation
			)->variant_number;
		}
	}

	return result;
}

static void object_choose_random_region_permutations(long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (object_definition->object.model.index!=NONE)
	{
		struct model* model = model_definition_get(object_definition->object.model.index);
		if (object->object.variant_number <= 0 ||
			!object_select_random_region_permutations_by_variant(object_index, model, object->object.variant_number))
		{
			object_select_random_region_permutations_by_variant(object_index, model, NONE);
			object->object.variant_number = object_determine_variant_number(object_index, model);
			if (object->object.variant_number > 0)
			{
				object_select_random_region_permutations_by_variant(object_index, model, object->object.variant_number);
			}
		}
	}

	return;
}

static void object_add_to_list(
	long *first_object_reference,
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	object->object.next_object_index = *first_object_reference;
	*first_object_reference = object_index;

	return;
}

static void object_remove_from_list(
	long *first_object_reference,
	long object_index)
{
	while (*first_object_reference!=NONE)
	{
		struct object_datum *object = object_get(*first_object_reference);

		if (*first_object_reference==object_index)
		{
			*first_object_reference = object->object.next_object_index;
			object->object.next_object_index = NONE;
			break;
		}

		first_object_reference = &object->object.next_object_index;

		match_assert("c:\\halo\\SOURCE\\objects\\objects.c", 3179, *first_object_reference!=NONE);
	}
	
	return;
}

static void attachments_delete(
	long object_index)
{
	short attachment_index;
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	for (attachment_index =0; attachment_index<object_definition->object.attachments.count; ++attachment_index)
	{
		if (object->object.attachment_types[attachment_index]!=NONE &&
			object->object.attachment_indices[attachment_index]!=NONE)
		{
			switch (object->object.attachment_types[attachment_index])
			{
			case _object_attachment_type_light:
				light_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_looping_sound:
				game_looping_sound_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_effect:
				effect_delete(object->object.attachment_indices[attachment_index]);
				break;
			case _object_attachment_type_contrail:
				object_compute_node_matrices(object_index);
				contrail_owner_collision(object->object.attachment_indices[attachment_index], TRUE, 0.f);
				break;
			case _object_attachment_type_particle_system:
				particle_system_orphan(object->object.attachment_indices[attachment_index]);
				break;
			default:
				break;
			}
		}
	}

	return;
}

static void object_create_attachments(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	widgets_new(object_index);
	attachments_new(object_index);

	return;
}

static void object_delete_attachments(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	object_type_definition_get(object->object.type);
	widgets_delete(object_index);
	attachments_delete(object_index);

	return;
}

static void object_delete_recursive(
	long object_index,
	boolean delete_siblings)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (TEST_FLAG(object->object.flags, _object_garbage_bit))
	{
		object_set_garbage(object_index, FALSE);
	}

	object_deleted_procs_call(object_index);
	if (object->object.first_child_object_index!=NONE)
	{
		object_delete_recursive(object->object.first_child_object_index, TRUE);
	}

	if (delete_siblings && object->object.next_object_index!=NONE)
	{
		object_delete_recursive(object->object.next_object_index, TRUE);
	}

	object_deactivate(object_index);
	object_delete_attachments(object_index);

	if (TEST_FLAG(object->object.flags, _object_connected_to_map_bit))
	{
		object_disconnect_from_map(object_index);
	}
	
	object_type_delete(object_index);
	object_header_delete(object_header_data, object_index);

	return;
}

static void object_compute_function_values(
	long object_index)
{
	short function_index;

	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);
	real huh = (57 * DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index) + game_time_get()) * 0.033333335f;

	for (function_index = 0; function_index<object_definition->object.functions.count; ++function_index)
	{
		real value;
		real output;

		struct object_function_definition* function = TAG_BLOCK_GET_ELEMENT(
			&object_definition->object.functions,
			function_index,
			struct object_function_definition);
		boolean function_is_active = TRUE;
		real period = function->runtime_one_over_period;

		if (function->scale_period_by_function_index)
		{
			real function_value = OBJECT_INCOMING_FUNCTION_GET_VALUE(object, function->scale_period_by_function_index);
			if (function_value>0.f)
			{
				period = period/function_value;
			}
		}

		value = periodic_function_evaluate(function->function_type, huh*period);

		if (function->scale_function_by_function_index)
		{
			value*= OBJECT_INCOMING_FUNCTION_GET_VALUE(object, function->scale_function_by_function_index);
		}

		if (TEST_FLAG(function->flags, _object_function_invert_function_bit))
		{
			value = 1.f-value;
		}

		if (function->wobble_magnitude!=0.f)
		{
			real val = periodic_function_evaluate(function->wobble_function_type, huh*function->wobble_period);
			value+= (2.f*function->wobble_magnitude * (val-0.5f));
		}

		if (function->square_wave_threshold!=0.f)
		{
			value = value>function->square_wave_threshold ? 1.f : 0.f ;
		}

		if (function->step_count>1)
		{
			real floor_value = floor(function->step_count*value);
			value = floor_value*function->runtime_reciprocal_step_count;
		}

		if (function->runtime_reciprocal_sawtooth_count>0.f)
		{
			value = fmod(value, function->runtime_reciprocal_sawtooth_count);
		}

		if (function->add_function_index)
		{
			value+= OBJECT_INCOMING_FUNCTION_GET_VALUE(object, function->add_function_index);
			value = MIN(value, 1.f);
		}

		if (function->scale_result_by_function_index)
		{
			value*= OBJECT_INCOMING_FUNCTION_GET_VALUE(object, function->scale_result_by_function_index);
		}

		output = transition_function_evaluate(function->map_result_to_transition_function, value);

		if (function->scale_by>0.f)
		{
			output*= function->scale_by;
		}

		if (function->bounds_mode==_object_function_scale_to_fit_bounds)
		{
			output*= (function->upper_bound-function->lower_bound);
			output+= function->lower_bound;

			if (function->lower_bound+_real_epsilon>=output)
			{
				function_is_active = TEST_FLAG(function->flags, _object_function_does_not_deactivate_below_lower_bound_bit);
			}
		}
		else
		{
			if (function->lower_bound+_real_epsilon>=output)
			{
				output = function->lower_bound;
				function_is_active = TEST_FLAG(function->flags, _object_function_does_not_deactivate_below_lower_bound_bit);
			}

			if (output>function->upper_bound)
			{
				output = function->upper_bound;
			}

			if (function->bounds_mode==_object_function_clip_to_bounds_and_normalize)
			{
				output = (output-function->lower_bound)*function->runtime_reciprocal_bounds_range;
			}
		}

		if (function->turn_off_with_function_index!=NONE &&
			!TEST_FLAG(object->object.functions_active_flags, function->turn_off_with_function_index))
		{
			function_is_active = FALSE;
		}

		if (TEST_FLAG(function->flags, _object_function_clip_to_bounds_and_normalize))
		{
			output = fmod(output+object->object.outgoing_function_values[function_index], 1.f);
		}

		object->object.outgoing_function_values[function_index] = output;
		SET_FLAG(object->object.functions_active_flags, function_index, function_is_active);
	}

	return;
}

static void object_compute_change_colors(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	struct object_definition *object_definition = object_definition_get(object->definition_index);

	if (TEST_FLAG(object_definition->object.runtime_flags, _object_runtime_scaled_change_colors_bit))
	{
		short cc_index;
		for (cc_index = 0; cc_index<object_definition->object.change_colors.count; cc_index++)
		{
			struct object_change_color_definition *change_color = TAG_BLOCK_GET_ELEMENT(
				&object_definition->object.change_colors,
				cc_index,
				struct object_change_color_definition);

			if (change_color->scaled_by)
			{
				rgb_colors_interpolate(
					&object->object.outgoing_change_colors[cc_index],
					change_color->scale_flags,
					&change_color->color_lower_bound,
					&change_color->color_upper_bound,
					OBJECT_INCOMING_FUNCTION_GET_VALUE(object, change_color->scaled_by));
			}

			if (change_color->darken_by)
			{
				const real scale = OBJECT_INCOMING_FUNCTION_GET_VALUE(object, change_color->darken_by);
				object->object.outgoing_change_colors[cc_index].red*= scale;
				object->object.outgoing_change_colors[cc_index].green*= scale;
				object->object.outgoing_change_colors[cc_index].blue*= scale;
			}

			object->object.outgoing_change_colors[cc_index].red = PIN(
				object->object.outgoing_change_colors[cc_index].red, 
				0.f,
				1.f);
			object->object.outgoing_change_colors[cc_index].green = PIN(
				object->object.outgoing_change_colors[cc_index].green,
				0.f,
				1.f);
			object->object.outgoing_change_colors[cc_index].blue = PIN(
				object->object.outgoing_change_colors[cc_index].blue,
				0.f,
				1.f);
		}
	}

	return;
}

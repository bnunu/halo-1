/*
OBJECTS.H

file has inline function assertions.
*/

#ifndef __OBJECTS_H
#define __OBJECTS_H
#pragma once

/* ---------- headers */

#include "object_definitions.h"
#include "object_types.h"

#include "math/real_math.h"
#include "memory/data.h"
#include "structures/cluster_partitions.h"

/* ---------- constants */

enum
{
	OBJECT_ITERATOR_SIGNATURE = 0x86868686,
	MAXIMUM_CLUSTERS_PER_OBJECT = 32,
	/* the native builds' larger object pool (halo_port_capacity.h). The
	garbage limits keep January's proportions: 5/10/20 % of the pool (the pool
	only allocates at its end, so these are also the bytes one tick may
	allocate before a compaction), 2.5/5/10 % of the object headers, and
	75/50/30 active garbage objects per 16 players in the game
	(objects.c: active_garbage_limit), so campaign and games of up to 16
	players keep the Xbox's */
	OBJECT_MEMORY_POOL_SIZE = HALO_PORT_OBJECT_MEMORY_POOL_SIZE,
	GARBAGE_LIMIT_FREE_MEMORY_CRITICAL = HALO_PORT_OBJECT_MEMORY_POOL_SIZE/20,
	GARBAGE_LIMIT_FREE_MEMORY_TRIGGER = HALO_PORT_OBJECT_MEMORY_POOL_SIZE/10,
	GARBAGE_LIMIT_FREE_MEMORY_TARGET = HALO_PORT_OBJECT_MEMORY_POOL_SIZE/5,
	GARBAGE_LIMIT_FREE_OBJECTS_CRITICAL = HALO_PORT_MAXIMUM_OBJECTS_PER_MAP/40,
	GARBAGE_LIMIT_FREE_OBJECTS_TRIGGER = HALO_PORT_MAXIMUM_OBJECTS_PER_MAP/20,
	GARBAGE_LIMIT_FREE_OBJECTS_TARGET = HALO_PORT_MAXIMUM_OBJECTS_PER_MAP/10,
	GARBAGE_LIMIT_ACTIVE_GARBAGE_CRITICAL = 75,
	GARBAGE_LIMIT_ACTIVE_GARBAGE_TRIGGER = 50,
	GARBAGE_LIMIT_ACTIVE_GARBAGE_TARGET = 30,
};

enum
{
	MAXIMUM_MARKERS_PER_OBJECT = 64,
	/* the native builds' larger object limits (halo_port_capacity.h) */
	MAXIMUM_OBJECTS_PER_MAP = HALO_PORT_MAXIMUM_OBJECTS_PER_MAP,
	MAXIMUM_RENDERED_OBJECTS = HALO_PORT_MAXIMUM_RENDERED_OBJECTS,
};

enum
{
	_object_header_active_bit = 0,
	_object_header_visible_bit,
	_object_header_being_created_bit,
	_object_header_being_deleted_bit,
	_object_header_do_not_update_bit,
	_object_header_connected_to_map_bit,
	_object_header_automatically_deactivate_bit,
	_object_header_child_bit,
	NUMBER_OF_OBJECT_HEADER_FLAGS,
};

enum
{
	_object_invisible_bit = 0,
	_object_on_ground_bit,
	_object_on_media_bit,
	_object_partially_under_media_bit,
	_object_wholly_under_media_bit,
	_object_at_rest_bit,
	_object_child_locations_invalid_bit,
	_object_animates_automatically_bit,
	_object_has_attached_lights_bit,
	_object_has_attached_shader_bit,
	_object_has_attached_looping_sounds_bit,
	_object_connected_to_map_bit,
	_object_mirrored_bit,
	_object_dynamic_lighting_recompute_bit,
	_object_static_lighting_recompute_bit,
	_object_static_lighting_raycast_sideways_bit,
	_object_garbage_bit,
	_object_cannot_be_garbage_bit,
	_object_shadowless_bit,
	_object_deleted_when_deactivated_bit,
	_object_cannot_be_activated_bit,
	_object_outside_of_map_bit,
	_object_movie_star_bit,
	_object_do_not_recompute_node_matrices_bit,
	_object_no_collisions_bit,
	_object_has_collision_model_bit,
	_object_force_baseline_update_bit,
	_object_has_been_updated_from_network_bit,
	NUMBER_OF_OBJECT_DATA_FLAGS,
};

enum
{
	_new_object_mirrored_bit = 0,
	_new_object_never_automatically_delete_bit,
	NUMBER_OF_NEW_OBJECT_DATA_FLAGS,
};


enum
{
	_object_passed_body_damage_threshold_bit = 0,
	_object_passed_shield_damage_threshold_bit,
	_object_dead_bit,
	_object_shield_depleted_bit,
	_object_shield_over_charging_bit,
	_object_die_act_of_god_bit,
	_object_die_act_of_god_silent_bit,
	_object_melee_attack_inhibited_bit,
	_object_ranged_attack_inhibited_bit,
	_object_walking_inhibited_bit,
	_object_cannot_hold_weapon_bit,
	_object_cannot_take_damage_bit,
	_object_shield_charging_bit,
	_object_die_act_of_god_no_statistics_bit,
	NUMBER_OF_OBJECT_DAMAGE_FLAGS,
};

enum
{
	_object_attachment_type_light = 0,
	_object_attachment_type_looping_sound,
	_object_attachment_type_effect,
	_object_attachment_type_contrail,
	_object_attachment_type_particle_system,
	NUMBER_OF_OBJECT_ATTACHMENT_TYPES,
};

enum
{
	_object_function_reference_none = 0,
	_object_function_reference_a,
	_object_function_reference_b,
	_object_function_reference_c,
	_object_function_reference_d,
	NUMBER_OF_OBJECT_FUNCTION_REFERENCES,
};

enum
{
	_object_function_none = 0,
	_object_function_body_vitality,
	_object_function_shield_vitality,
	_object_function_recent_body_damage,
	_object_function_recent_shield_damage,
	_object_function_random_constant,
	_object_function_umbrella_shield_vitality,
	_object_function_shield_stun,
	_object_function_recent_umbrella_shield_vitality,
	_object_function_umbrella_shield_stun,
	_object_function_first_region_damage,
	_object_function_last_region_damage = 17,
	_object_function_alive,
	_object_function_compass,
	NUMBER_OF_OBJECT_FUNCTION_MODES,
};

enum
{
	_object_change_color_a = 0,
	_object_change_color_b,
	_object_change_color_c,
	_object_change_color_d,
	NUMBER_OF_OBJECT_CHANGE_COLORS,
};

enum
{
	_pvs_activation_normal = 0,
	_pvs_activation_object,
	_pvs_activation_cluster,
	NUMBER_OF_OBJECT_PVS_ACTIVATION_TYPES,
};

enum
{
	_garbage_collect_everything = 0,
	_garbage_collect_active_objects,
	_garbage_collect_for_space,
	NUMBER_OF_GARBAGE_COLLECTION_MODES,
};


/* ---------- macros */

#define object_header_get(index)			((struct object_header_datum*)datum_get(object_header_data, (index)))
#define object_header_try_and_get(index)	((struct object_header_datum*)datum_try_and_get(object_header_data, (index)))

#define object_get(index)			((struct object_datum*)object_get_and_verify_type(index, _object_mask_all))
#define object_try_and_get(index)	((struct object_datum*)object_try_and_get_and_verify_type(index, _object_mask_all))

/* the inline assertions below report January's __FILE__ for this header, and January spelled it
   two ways: its string census has "..\objects\objects.h" defined by action_vehicle (and
   referenced by render_objects) and "c:\halo\source\objects\objects.h" defined by
   object_lights; a unit that needs the absolute spelling defines OBJECTS_H_FILE first */
#ifndef OBJECTS_H_FILE
#define OBJECTS_H_FILE "..\\objects\\objects.h"
#endif

/* ---------- structures */

typedef void (*object_deleted_proc)(long);


struct object_cluster_iterator
{
	struct cluster_partition *cluster_partition;
	long reference_index;
};

struct object_header_datum
{
	short identifier;
	byte flags;
	byte type;
	short cluster_index;
	short data_size;
	struct object_datum *datum;
};

struct location
{
	long leaf_index;
	short cluster_index;
	word bonus;
};

struct animation_state
{
	short index;
	short frame_index;
};

struct object_animation_state
{
	long animation_graph_index;
	struct animation_state state;
	short interpolation_frame_index;
	short interpolation_frame_count;
};

struct object_header_block_reference
{
	short size;
	short offset;
};

struct _object_datum
{
	unsigned long flags;
	long magic_number;
	real_point3d position;
	real_vector3d translational_velocity;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d angular_velocity;
	struct location location;
	real_point3d bounding_sphere_center;
	real bounding_sphere_radius;
	real scale;
	short type;
	word render_flags;
	short owner_team_index;
	short name_index;
	short idle_ticks;
	short variant_number;
	long owner_player_index;
	long owner_object_index;
	long owner_object_definition_index;
	struct object_animation_state animation;
	real maximum_body_vitality;
	real maximum_shield_vitality;
	real body_vitality;
	real shield_vitality;
	real current_shield_damage;
	real current_body_damage;
	long umbrella_shield_object_index;
	real recent_shield_damage;
	real recent_body_damage;
	long shield_damage_decay_timer;
	long body_damage_decay_timer;
	short shield_stun_ticks;
	word damage_flags;
	long scenery_idx;
	long first_cluster_reference_index;
	long next_garbage_object_index;
	long next_object_index;
	long first_child_object_index;
	long parent_object_index;
	char parent_node_index;
	byte pad[1];
	boolean force_shield_update;
	byte functions_active_flags;
	real incoming_function_values[NUMBER_OF_INCOMING_OBJECT_FUNCTIONS];
	real outgoing_function_values[NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS];
	char attachment_types[MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT];
	long attachment_indices[MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT];
	long first_widget_index;
	long cached_render_state_index;
	word regions_destroyed_flags;
	short forced_shader_permutation_index;
	byte region_damage[MAXIMUM_REGIONS_PER_OBJECT];
	byte region_permutations[MAXIMUM_REGIONS_PER_OBJECT];
	real_rgb_color base_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
	real_rgb_color outgoing_change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
	struct object_header_block_reference original_node_orientations;
	struct object_header_block_reference node_orientations;
	struct object_header_block_reference node_matrices;
};

struct object_datum
{
	long definition_index;
	struct _object_datum object;
};

struct object_iterator
{
	unsigned long type_flags;
	byte flags;
	short absolute_index;
	long index;
	unsigned long signature;
};

struct objects_information
{
	short object_count;
	short active_object_count;
	real used_memory;
};

struct object_marker
{
	short node_index;
	real_matrix4x3 node_matrix;
	real_matrix4x3 matrix;
};

struct object_placement_data
{
	long definition_index;
	unsigned long flags;
	long owner_player_index;
	long owner_object_index;
	long owner_object_definition_index;
	short owner_team_index;
	short variant_number;
	real_point3d position;
	real height;
	real_vector3d translational_velocity;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d angular_velocity;
	real_rgb_color change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
};

/* ---------- prototypes/OBJECTS.C */

struct scenario_object_permutation;

long cluster_get_first_noncollideable_object(long *reference_index, short cluster_index);
long cluster_get_next_noncollideable_object(long *reference_index);
long cluster_get_first_collideable_object(long *reference_index, short cluster_index);
long cluster_get_next_collideable_object(long *reference_index);
short object_get_next_cluster(struct object_cluster_iterator *iterator, long object_index);
void *object_try_and_get_and_verify_type(long object_index, unsigned long valid_type_flags);
void *object_get_and_verify_type(long object_index, unsigned long valid_type_flags);
void object_iterator_new(struct object_iterator *iterator, unsigned long type_flags, byte flags);
void *object_iterator_next(struct object_iterator *iterator);
long object_get_ultimate_parent(long object_index);
void object_add_scenario_permutation(long object_index, struct scenario_object_permutation *permutation);
void object_set_object_index_for_name_index(short name_index, long object_index);
void objects_fix_for_deleted_object(long deleted_object_index);
void object_set_garbage(long object_index, boolean garbage);
void garbage_collect_now(void);
void objects_information_get(struct objects_information *information);
void object_pvs_set_object(long object_index);
void object_pvs_set_camera_point(short camera_point_index);
void object_pvs_clear(void);
short objects_get_activating_cluster_index(void);
/* port: makes a cluster active as object_pvs_activate would (NONE: back to
the players' own), for a network co-op client following the host's
cutscene (port/linux/game/network_coop.c) */
void objects_port_set_activating_cluster(short cluster_index);
void object_definition_predict(long definition_index);
void object_predict(long object_index);
void object_beautify(long object_index, boolean beautiful);
void *object_header_block_get(long object_index, struct object_header_block_reference *reference);
boolean object_header_block_allocate(long object_index, short block_reference_offset, short size);
void object_scripting_set_collideable(long object_index, boolean collideable);
void object_marker_begin(void);
void object_marker_end(void);
boolean object_unmarked_function(long object_index);
boolean object_mark_function(long object_index);
void objects_place(void);
long find_objects_from_point_vector(
	real_point3d const *position,
	real_vector3d const *direction,
	boolean(*add_object_function)(long, void *),
	void *custom_data,
	long maximum_object_count,
	long *object_indices);
void objects_dump_memory(void);
void objects_initialize(void);
void objects_initialize_for_new_map(void);
void objects_dispose_from_old_map(void);
void objects_dispose(void);
void object_activate(long object_index);
void object_deactivate(long object_index);
void object_reset(long object_index);
void object_placement_data_new(struct object_placement_data *data, long definition_index, long owner_object_index);
void object_disconnect_from_map(long object_index);
short object_get_first_cluster(struct object_cluster_iterator *iterator, long object_index);
real_matrix4x3 *object_get_node_matrices(long object_index);
char const *object_get_attachment_marker_name(long object_index, short attachment_index);
boolean object_has_node(long object_index, short node_index);
void object_set_automatic_deactivation(long object_index, boolean automatic_deactivation);
void object_set_visibility(long object_index, boolean visible_flag);
void object_get_velocities(long object_index, real_vector3d *translational_velocity, real_vector3d *angular_velocity);
/* port: for the sounds' Doppler effect (game_sound.c, sound_manager.c) */
void object_get_point_velocity(long object_index, real_point3d const *point, real_vector3d *velocity);
struct location *object_get_location(long object_index, struct location *location);
void object_start_interpolation(long object_index, short frame_count);
void object_offset_interpolation(long object_index, real_vector3d *offset);
void object_permute_region(long object_index, char *permutation_name, short desired_region_index, boolean active_flag);
boolean object_get_function_value(long object_index, short function_index, real *value_reference);
/* port: the picture is drawn between ticks. While this is set, a function
read for drawing is the tick's value moved forward by the fraction of the
tick that has passed. */
void object_functions_set_for_render(boolean for_render);
void object_function_values_for_render(long object_index, real values[NUMBER_OF_OUTGOING_OBJECT_FUNCTIONS]);
/* port: forgets the inputs kept for the objects' pictures (objects.c's
object_function_inputs), when a load puts the objects back or a map starts */
void object_function_inputs_forget(void);
short objects_in_clusters_by_indices(unsigned long class_flags, short cluster_count, short const *cluster_indices, short maximum_object_count, long *object_indices);
long object_index_from_name_index(short name_index);
/* port: this object takes a scenario name nobody holds, the way a placed
vehicle does. A respawned vehicle keeps the name of the one it replaces, so
a script that teleports that name still finds it. */
void object_claim_scenario_name(long object_index, short name_index);
void objects_disconnect_from_structure_bsp(void);
boolean object_visible_to_any_player(long object_index);
void object_pvs_activate(long object_index);
void objects_scripting_set_scale(long object_index, real scale, short interpolation_frame_count);
void object_delete(long object_index);
void object_reconnect_to_map(long object_index, struct location const *location);
real_matrix4x3 *object_get_node_matrix(long object_index, short node_index);
short object_get_marker_by_name(long object_index, char const *name, struct object_marker *markers, short maximum_marker_count);
void object_align_marker_to_matrix(struct object_datum *object, struct object_marker const *child_marker, real_matrix4x3 const *destination_matrix);
void object_detach(long child_object_index);
real_point3d *object_get_origin(long object_index, real_point3d *origin);
void object_get_orientation(long object_index, real_vector3d *forward, real_vector3d *up);
real_matrix4x3 *object_get_world_matrix(long object_index, real_matrix4x3 *matrix);
void object_inverse_kinematics(long hand_object_index, char *hand_marker_name, long grip_object_index, char *grip_marker_name, real_matrix4x3 *node_matrices);
short objects_in_sphere(unsigned long class_flags, unsigned long type_flags, struct location const *location, real_point3d const *center, real radius, long *object_indices, short maximum_count);
void objects_reconnect_to_structure_bsp(void);
void objects_paparazzi(void);
void object_export_function_values(long object_index);
void object_compute_node_matrices(long object_index);
void objects_scripting_detach(long parent_object_index, long child_object_index);
void object_render_debug(long object_index);
void object_set_position(long object_index, real_point3d const *position, real_vector3d const *forward, real_vector3d const *up);
void object_translate(long object_index, real_point3d const *new_position, struct location const *new_location);
long object_new(struct object_placement_data *data);
void object_attach_to_node(long parent_object_index, long child_object_index, short parent_node_index);
boolean object_force_inside_bsp(long object_index, real_point3d const *known_good_point);
void object_compute_node_matrices_recursive(long object_index);
long object_new_from_scenario(struct scenario_object_datum *scenario_object, struct tag_block *palette);
void object_attach_to_marker(long parent_object_index, char const *parent_marker_name, long child_object_index, char const *child_marker_name);
long object_new_by_name(short name_index);
void objects_scripting_attach(long parent_object_index, char const *parent_marker_name, long child_object_index, char const *child_marker_name);
void object_delete_immediately(long object_index);
void objects_garbage_collection(void);
void objects_update(void);
void objects_memory_compact(void);


/* ---------- prototypes/OBJECT_DELETED_PROCS.C */

void object_deleted_procs_call(long deleted_object_index);

/* ---------- globals */

extern boolean debug_objects_names;
extern boolean debug_objects_pathfinding_spheres;
extern boolean debug_objects_physics;
extern boolean debug_objects_root_node;

extern long global_object_marker;

extern struct cluster_partition noncollideable_object_cluster_partition;
extern struct cluster_partition collideable_object_cluster_partition;

extern boolean debug_object_garbage_collection;

extern struct data_array *object_header_data;

extern boolean debug_objects_position_velocity;
extern boolean debug_objects_bounding_spheres;
extern boolean debug_objects_collision_models;

/* ---------- public code */

__inline short object_get_type(
	long object_index)
{
	return object_header_get(object_index)->type;
}

__inline void object_get_bounding_sphere(
	long object_index,
	real_point3d *center,
	real *radius)
{
	struct object_datum *object = object_get(object_index);

	match_assert(OBJECTS_H_FILE, 535, center);
	match_assert(OBJECTS_H_FILE, 536, radius);

	*center = object->object.bounding_sphere_center;
	*radius = object->object.bounding_sphere_radius;

	return;
}

__inline void object_get_render_bounding_sphere(
	long object_index,
	real_point3d *center,
	real *radius)
{
	struct object_datum *object = object_get(object_index);

	match_assert(OBJECTS_H_FILE, 551, center);
	match_assert(OBJECTS_H_FILE, 552, radius);

	*center = object->object.bounding_sphere_center;
	*radius = object_definition_get(object->definition_index)->object.render_bounding_radius;
	/* port: (grown with an object scaled up, as its bounding sphere is in
	object_compute_node_matrices; one scaled down keeps the larger sphere) */
	if (object->object.scale > 1.f)
	{
		*radius *= object->object.scale;
	}

	return;
}


#endif // __OBJECTS_H

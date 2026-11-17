/*
ACTION_VEHICLE.C

symbols in this file:
000090F0 0090:
	_object_get_bounding_sphere (0000)
00009180 0050:
	_action_vehicle_begin (0000)
000091D0 0010:
	_action_vehicle_update (0000)
000091E0 0030:
	_action_vehicle_flush_structure_indices (0000)
00009210 00b0:
	_action_vehicle_control (0000)
000092C0 0030:
	_point_from_line2d (0000)
000092F0 0030:
	_distance_squared2d (0000)
00009320 0030:
	_distance2d (0000)
00009350 0020:
	_negate_vector2d (0000)
00009370 0040:
	_distance3d (0000)
000093B0 0110:
	_code_000093b0 (0000)
000094C0 03d0:
	_code_000094c0 (0000)
00009890 04d0:
	_code_00009890 (0000)
00009D60 0150:
	_action_vehicle_setup_specific (0000)
00009EB0 0300:
	_action_vehicle_perform (0000)
0000A1B0 0130:
	_code_0000a1b0 (0000)
0000A2E0 01d0:
	_action_vehicle_setup_impromptu (0000)
00243AA4 0007:
	??_C@_06KGHINEPG@radius?$AA@ (0000)
00243AAC 0007:
	??_C@_06BBLOAEEI@center?$AA@ (0000)
00243AB4 0015:
	??_C@_0BF@EIEPJPCC@?4?4?2objects?2objects?4h?$AA@ (0000)
00243ACC 0004:
	__real@3991a2b4 (0000)
00243AD0 0004:
	__real@40600000 (0000)
00243AD4 0004:
	__real@41200000 (0000)
00243AD8 0004:
	__real@3f8ccccd (0000)
00243ADC 0004:
	__real@3f19999a (0000)
00243AE0 0004:
	__real@40000000 (0000)
00243AE4 0004:
	__real@38d1b717 (0000)
00243AE8 0004:
	__real@3f99999a (0000)
00243AEC 0004:
	__real@3dfae147 (0000)
00243AF0 0023:
	??_C@_0CD@FHLDOEML@c?3?2halo?2SOURCE?2ai?2action_vehicle@ (0000)
00243B14 0004:
	__real@41c80000 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "actions.h"
#include "actors.h"
#include "actor_definitions.h"
#include "ai_debug.h"
#include "props.h"
#include "game/game.h"
#include "physics/collision_bsp.h"
#include "physics/collisions.h"
#include "scenario/scenario.h"
#include "units/unit_definitions.h"
#include "units/units.h"
#include "units/vehicle_definitions.h"
#include "units/vehicles.h"

#include "custom_edition_cache.h" /* port: port/linux/game/custom_edition_cache.c */

/* ---------- constants */

enum
{
	_action_vehicle_idle_look_type = 4,
};

enum
{
	_actor_definition_disallow_vehicle_combat_bit = 3,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static boolean action_vehicle_desirable(
	long actor_index,
	long vehicle_index,
	boolean scripted_request,
	real attempt_start_distance,
	real attempt_continue_distance,
	boolean already_inside,
	boolean already_attempting_entry);

static boolean action_vehicle_evaluate_seat(
	long actor_index,
	long vehicle_index,
	short seat_index,
	boolean allow_any_seat,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point,
	real *seat_weight_reference,
	boolean *within_range_reference,
	boolean *correct_facing_reference,
	boolean *could_potentially_fake_reference);

static boolean action_vehicle_find_destination(
	long actor_index,
	long vehicle_index,
	real_point3d const *entry_point,
	real_vector3d const *entry_facing,
	real_point3d *hint_point,
	boolean *ignore_hint_reference,
	real_point3d *destination_point,
	long *surface_index_reference);

static short action_vehicle_find_impromptu_seat(
	long actor_index,
	long vehicle_index,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point);

/* ---------- globals */

/* ---------- public code */

void action_vehicle_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;

	state_data->stuck_detection_counter = 0;
	state_data->stuck_detection_time = game_time_get();
	state_data->stuck_detection_point = actor->input.position.body_position;

	return;
}

void action_vehicle_update(
	long actor_index)
{
	return;
}

void action_vehicle_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;

	if (state_data->lock_facing)
	{
		actor->orders.look.primary_priority = _primary_priority_exact_facing;
		actor->orders.look.primary_direction.type = _direction_specification_vector;
		actor->orders.look.primary_direction.vector = state_data->destination_facing;
	}
	else if (actor_path_has_path(actor_index))
	{
		actor->orders.look.primary_priority = _primary_priority_facing;
		actor->orders.look.primary_direction.type = _direction_specification_movement;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_none;
	}

	actor->orders.look.idle_look_type = _action_vehicle_idle_look_type;
	actor->orders.combat.shoot_at_target = FALSE;
	actor->orders.move.stationary_crouch = FALSE;
	actor->orders.move.moving_crouch = FALSE;
	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	return;
}

void action_vehicle_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	actor->state.action_data.vehicle.destination_surface_index = NONE;

	return;
}

/* ---------- private code */

static boolean action_vehicle_desirable(
	long actor_index,
	long vehicle_index,
	boolean scripted_request,
	real attempt_start_distance,
	real attempt_continue_distance,
	boolean already_inside,
	boolean already_attempting_entry)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *vehicle = unit_get(vehicle_index);
	boolean desirable = FALSE;

	if (TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
	{
		desirable = FALSE;
	}
	else if (scripted_request)
	{
		desirable = TRUE;
	}
	else
	{
		real maximum_distance = already_attempting_entry ?
			attempt_start_distance : attempt_continue_distance;
		real_point3d origin;

		object_get_origin(vehicle_index, &origin);
		if (already_inside ||
			distance_squared3d(&actor->input.position.body_position, &origin) <
				maximum_distance*maximum_distance)
		{
			desirable = TRUE;
			if (!already_inside && !already_attempting_entry &&
				magnitude_squared3d(&vehicle->object.translational_velocity) >
					(1.0f / (60.0f*60.0f)))
			{
				desirable = FALSE;
			}
		}
	}

	if (!already_inside && vehicle->object.up.k < 0.5f)
	{
		desirable = FALSE;
	}

	return desirable;
}

static boolean action_vehicle_evaluate_seat(
	long actor_index,
	long vehicle_index,
	short seat_index,
	boolean allow_any_seat,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point,
	real *seat_weight_reference,
	boolean *within_range_reference,
	boolean *correct_facing_reference,
	boolean *could_potentially_fake_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *definition =
		actor_definition_get(actor->meta.definition_index);
	struct actor_variant_definition *variant_definition =
		actor_variant_definition_get(actor->meta.variant_definition_index);
	boolean result = FALSE;

	if (!unit_seat_filled(vehicle_index, seat_index) &&
		(!TEST_FLAG(
			definition->flags2,
			_actor_definition_disallow_vehicle_combat_bit) ||
			unit_seat_allow_noncombatants(vehicle_index, seat_index)))
	{
		real_point3d entrance_point;
		real_point3d seat_point;
		real_point3d seat_hint_point;

		if (unit_get_seat_entrance_point(
			actor->meta.unit_index,
			vehicle_index,
			seat_index,
			&entrance_point,
			&seat_point,
			&seat_hint_point))
		{
			real_point3d vehicle_origin;
			real_vector3d facing;
			real distance_to_seat;

			result = TRUE;

			object_get_origin(vehicle_index, &vehicle_origin);
			facing.i = seat_point.x - entrance_point.x;
			facing.j = seat_point.y - entrance_point.y;
			facing.k = 0.0f;
			if (normalize2d((real_vector2d *)&facing) == 0.0f)
			{
				facing = actor->input.facing_vector;
			}

			distance_to_seat = MIN(
				distance2d(
					(real_point2d const *)&actor->input.position.body_position,
					(real_point2d const *)&entrance_point),
				distance2d(
					(real_point2d const *)&actor->input.position.body_position,
					(real_point2d const *)&seat_point));

			{
				struct prop_iterator iterator;
				struct prop_datum *prop;

				prop_iterator_new(&iterator, actor_index);
				while (prop = prop_iterator_next(&iterator))
				{
					if (!prop->enemy && prop->actor_index != NONE)
					{
						struct actor_datum *other_actor =
							actor_try_and_get(prop->actor_index);

						if (other_actor->state.action == _actor_action_vehicle &&
							other_actor->state.action_data.vehicle.vehicle_index == vehicle_index)
						{
							struct vehicle_state_data *other_state =
								&other_actor->state.action_data.vehicle;

							if (other_state->seat_index == seat_index)
							{
								real other_dx = other_state->destination_point.x -
									other_actor->input.position.body_position.x;
								real other_dy = other_state->destination_point.y -
									other_actor->input.position.body_position.y;

								if (other_dy*other_dy + other_dx*other_dx <
									distance_to_seat*distance_to_seat)
								{
									result = FALSE;
									break;
								}
							}
							else
							{
								unit_seat_is_driver(vehicle_index, other_state->seat_index);
							}
						}
					}
				}
			}

			if (result)
			{
				/* port: on Custom Edition maps any unit's seat, as on Halo PC.
				Their scripts send AI to bipeds, and ai_scripting_go_to_vehicle_internal takes any unit. */
				struct unit_datum *vehicle = object_get_and_verify_type(vehicle_index,
					custom_edition_cache_tags_loaded() ? _object_mask_unit : _object_mask_vehicle);
				real_vector2d to_seat_direction;
				real facing_dot;
				boolean within_range;
				boolean correct_facing;
				boolean could_potentially_fake;
				real seat_weight;

				if (!allow_any_seat && vehicle->object.type == _object_type_vehicle)
				{
					vehicle_specific_definition_get(vehicle->definition_index);
					unit_seat_is_driver(vehicle_index, seat_index);
				}

				vector_from_points2d(
					(real_point2d const *)&actor->input.position.body_position,
					(real_point2d const *)&seat_point,
					&to_seat_direction);
				normalize2d(&to_seat_direction);
				facing_dot = dot_product2d(
					&to_seat_direction,
					(real_vector2d const *)&actor->input.facing_vector);
				within_range = distance_to_seat < 0.7f;
				correct_facing = facing_dot > 0.6f;
				could_potentially_fake =
					distance_to_seat < 1.1f && facing_dot > 0.0f;
				seat_weight = 10.0f / (distance_to_seat + 1.0f);

				if (TEST_FLAG(
					variant_definition->flags,
					_actor_variant_definition_prefer_passenger_seat_bit))
				{
					if (!unit_seat_is_gunner(vehicle_index, seat_index))
					{
						seat_weight += 3.5f;
					}
				}
				else if (unit_seat_is_gunner(vehicle_index, seat_index))
				{
					seat_weight += 3.5f;
				}

				if (entry_point)
				{
					*entry_point = entrance_point;
				}
				if (entry_facing)
				{
					*entry_facing = facing;
				}
				if (hint_point)
				{
					*hint_point = seat_hint_point;
				}
				if (seat_weight_reference)
				{
					*seat_weight_reference = seat_weight;
				}
				if (within_range_reference)
				{
					*within_range_reference = within_range;
				}
				if (correct_facing_reference)
				{
					*correct_facing_reference = correct_facing;
				}
				if (could_potentially_fake_reference)
				{
					*could_potentially_fake_reference = could_potentially_fake;
				}
			}
		}
	}

	return result;
}

static boolean action_vehicle_find_destination(
	long actor_index,
	long vehicle_index,
	real_point3d const *entry_point,
	real_vector3d const *entry_facing,
	real_point3d *hint_point,
	boolean *ignore_hint_reference,
	real_point3d *destination_point,
	long *surface_index_reference)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition =
		unit_definition_get(vehicle->definition_index);
	struct actor_debug_info *debug =
		&actor_debug_array[DATUM_INDEX_TO_ABSOLUTE_INDEX(actor_index)];
	real_point3d modified_destination = *entry_point;
	boolean success = FALSE;
	boolean ignore_hint =
		ignore_hint_reference ? *ignore_hint_reference : FALSE;

	if (!TEST_FLAG(
		vehicle_definition->unit.flags,
		_unit_has_entrance_points_inside_bounding_sphere_bit))
	{
		real_point3d vehicle_center;
		real_point3d target_point;
		real_vector2d actor_to_vehicle;
		real_vector2d actor_to_target_point;
		real_vector2d target_point_to_vehicle;
		real vehicle_radius;
		real actor_to_target_point_distance_squared;

		object_get_bounding_sphere(vehicle_index, &vehicle_center, &vehicle_radius);
		if (vehicle_definition->unit.ai_vehicle_avoidance_radius > 0.0f)
		{
			vehicle_radius = vehicle_definition->unit.ai_vehicle_avoidance_radius;
		}

		if (!ignore_hint)
		{
			real hint_distance = distance3d(&vehicle_center, hint_point);

			if (hint_distance < 0.5f)
			{
				ignore_hint = TRUE;
			}
			else
			{
				vehicle_radius = MAX(vehicle_radius, hint_distance + 0.3f);
			}
		}

		target_point = *(ignore_hint ? entry_point : hint_point);
		vector_from_points2d(
			(real_point2d const *)&actor->input.position.body_position,
			(real_point2d const *)&vehicle_center,
			&actor_to_vehicle);
		vector_from_points2d(
			(real_point2d const *)&actor->input.position.body_position,
			(real_point2d const *)&target_point,
			&actor_to_target_point);
		vector_from_points2d(
			(real_point2d const *)&target_point,
			(real_point2d const *)&vehicle_center,
			&target_point_to_vehicle);

		if (!ignore_hint)
		{
			real_vector2d perpendicular_vector_to_hint_line;
			real_vector2d hint_direction;

			vector_from_points2d(
				(real_point2d const *)entry_point,
				(real_point2d const *)hint_point,
				&hint_direction);
			point_from_line2d(
				(real_point2d const *)&actor_to_target_point,
				&hint_direction,
				-dot_product2d(&actor_to_target_point, &hint_direction),
				(real_point2d *)&perpendicular_vector_to_hint_line);
			if (magnitude_squared2d(&perpendicular_vector_to_hint_line) < 0.35f*0.35f)
			{
				ignore_hint = TRUE;
			}
		}

		debug->last_vehicle_avoidance_time = game_time_get();
		debug->field_C8 = actor->input.position.body_position;
		debug->field_D4 = vehicle_center;
		debug->field_E0 = vehicle_radius;
		debug->field_E4 = target_point;
		debug->field_F0 = REAL_MAX;
		debug->field_F4 = FALSE;

		actor_to_target_point_distance_squared =
			magnitude_squared2d(&actor_to_target_point);
		if (actor_to_target_point_distance_squared > 0.0f)
		{
			real t =
				dot_product2d(&actor_to_target_point, &actor_to_vehicle) /
				actor_to_target_point_distance_squared;
			real_vector2d avoid_vehicle_direction;
			boolean avoid_vehicle = FALSE;

			debug->field_F0 = t;
			if (t > 0.0f && t < 1.2f)
			{
				perpendicular2d(&actor_to_target_point, &avoid_vehicle_direction);
				if (dot_product2d(&actor_to_vehicle, &avoid_vehicle_direction) > 0.0f)
				{
					negate_vector2d(&avoid_vehicle_direction, &avoid_vehicle_direction);
				}
				avoid_vehicle = TRUE;
			}
			else if (!ignore_hint)
			{
				negate_vector2d(&target_point_to_vehicle, &avoid_vehicle_direction);
				avoid_vehicle = TRUE;
			}

			if (avoid_vehicle && normalize2d(&avoid_vehicle_direction) > 0.0f)
			{
				real_vector3d vector_to_destination;
				real distance_squared;

				point_from_line2d(
					(real_point2d const *)&vehicle_center,
					&avoid_vehicle_direction,
					vehicle_radius*1.1f,
					(real_point2d *)&modified_destination);
				vector_from_points3d(
					&actor->input.position.body_position,
					&modified_destination,
					&vector_to_destination);
				distance_squared = magnitude_squared3d(&vector_to_destination);
				if (distance_squared > 0.0001f && distance_squared < 4.0f)
				{
					real_vector3d tangential_offset_vector;
					real distance = square_root(distance_squared);

					perpendicular2d(
						&target_point_to_vehicle,
						(real_vector2d *)&tangential_offset_vector);
					if (dot_product2d(
						(real_vector2d const *)&tangential_offset_vector,
						(real_vector2d const *)&vector_to_destination) < 0.0f)
					{
						negate_vector2d(
							(real_vector2d const *)&tangential_offset_vector,
							(real_vector2d *)&tangential_offset_vector);
					}
					tangential_offset_vector.k = 0.0f;
					if (normalize2d((real_vector2d *)&tangential_offset_vector) > 0.0f)
					{
						point_from_line3d(
							&modified_destination,
							&tangential_offset_vector,
							2.0f - distance,
							&modified_destination);
					}
				}

				debug->field_F4 = TRUE;
				debug->field_F8 = modified_destination;
			}
		}
	}

	if (ignore_hint_reference)
	{
		*ignore_hint_reference = ignore_hint;
	}

	{
		struct collision_bsp *collision_bsp = global_collision_bsp_get();
		struct collision_bsp_test_vector_result result;
		real_point3d collision_origin;
		real_vector3d collision_vector;

		point_from_line3d(&modified_destination, global_up3d, 1.0f, &collision_origin);
		scale_vector3d(global_down3d, 4.0f, &collision_vector);
		if (collision_bsp_test_vector(
			FLAG(_collision_test_front_facing_surfaces_bit),
			collision_bsp,
			0,
			NULL,
			&collision_origin,
			&collision_vector,
			REAL_MAX,
			&result))
		{
			*surface_index_reference = result.surface_index;
			point_from_line3d(&collision_origin, &collision_vector, result.t, destination_point);
			success = TRUE;
		}
	}

	return success;
}

static short action_vehicle_find_impromptu_seat(
	long actor_index,
	long vehicle_index,
	real_point3d *entry_point,
	real_vector3d *entry_facing,
	real_point3d *hint_point)
{
	struct unit_datum *vehicle = unit_get(vehicle_index);
	struct unit_definition *vehicle_definition =
		unit_definition_get(vehicle->definition_index);
	real_point3d best_entry_point = { 0 };
	real_vector3d best_entry_facing = { 0 };
	real_point3d best_hint_point = { 0 };
	long best_seat_index = NONE;
	real best_seat_weight = 0.0f;
	long seat_index;

	for (seat_index = 0;
		(short)seat_index < vehicle_definition->unit.seats.count;
		seat_index++)
	{
		real seat_weight;
		real_point3d candidate_entry_point;
		real_vector3d candidate_entry_facing;
		real_point3d candidate_hint_point;

		if (action_vehicle_evaluate_seat(
			actor_index,
			vehicle_index,
			seat_index,
			FALSE,
			&candidate_entry_point,
			&candidate_entry_facing,
			&candidate_hint_point,
			&seat_weight,
			NULL,
			NULL,
			NULL) &&
			seat_weight > best_seat_weight)
		{
			best_seat_weight = seat_weight;
			best_entry_point = candidate_entry_point;
			best_entry_facing = candidate_entry_facing;
			best_seat_index = seat_index;
			best_hint_point = candidate_hint_point;
		}
	}

	if (entry_point)
	{
		*entry_point = best_entry_point;
	}
	if (entry_facing)
	{
		*entry_facing = best_entry_facing;
	}
	if (hint_point)
	{
		*hint_point = best_hint_point;
	}

	return (short)best_seat_index;
}

boolean action_vehicle_setup_specific(
	long actor_index,
	long vehicle_index,
	short seat_index,
	struct vehicle_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_vehicle.c", 104, state_data);

	csmemset(state_data, 0, sizeof(*state_data));
	if (actor->input.vehicle_index == NONE &&
		!actor->meta.swarm &&
		action_vehicle_desirable(
			actor_index,
			vehicle_index,
			TRUE,
			REAL_MAX,
			REAL_MAX,
			FALSE,
			FALSE))
	{
		real_point3d entry_point;
		real_vector3d entry_facing;
		real_point3d hint_point;

		state_data->vehicle_index = vehicle_index;
		state_data->seat_index = seat_index;
		state_data->impromptu = FALSE;
		if (unit_has_animation_to_enter_seat(
				actor->meta.unit_index,
				vehicle_index,
				seat_index) &&
			action_vehicle_evaluate_seat(
				actor_index,
				vehicle_index,
				seat_index,
				TRUE,
				&entry_point,
				&entry_facing,
				&hint_point,
				NULL,
				NULL,
				NULL,
				NULL) &&
			action_vehicle_find_destination(
				actor_index,
				vehicle_index,
				&entry_point,
				&entry_facing,
				&hint_point,
				NULL,
				&state_data->destination_point,
				&state_data->destination_surface_index) &&
			actor_move_to_point(
				actor_index,
				&state_data->destination_point,
				state_data->destination_surface_index,
				vehicle_index))
		{
			result = TRUE;
		}
	}

	return result;
}

boolean action_vehicle_perform(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct vehicle_state_data *state_data = &actor->state.action_data.vehicle;
	struct unit_datum *vehicle = vehicle_try_and_get(state_data->vehicle_index);

	match_assert(
		"c:\\halo\\SOURCE\\ai\\action_vehicle.c",
		161,
		!actor->meta.swarm);

	if (actor->input.vehicle_index != NONE)
	{
		state_data->vehicle_entry_done = TRUE;
	}
	else if (!state_data->started_entry)
	{
		if (!vehicle)
		{
			state_data->vehicle_index = NONE;
			state_data->vehicle_entry_failed = TRUE;
		}
		else if (!action_vehicle_desirable(
			actor_index,
			state_data->vehicle_index,
			!state_data->impromptu,
			state_data->attempt_distance,
			state_data->continue_distance,
			FALSE,
			TRUE))
		{
			state_data->vehicle_entry_failed = TRUE;
		}
		else
		{
			long current_time = game_time_get();

			if (current_time >= state_data->stuck_detection_time + 150)
			{
				state_data->stuck_detection_time = current_time;
				if (distance_squared3d(
					&state_data->stuck_detection_point,
					&actor->input.position.body_position) < 25.0f)
				{
					state_data->stuck_detection_counter++;
				}
				else
				{
					state_data->stuck_detection_counter = 0;
					state_data->stuck_detection_point =
						actor->input.position.body_position;
				}
			}

			if (state_data->stuck_detection_counter < 8)
			{
				real_point3d entry_point;
				real_vector3d entry_facing;
				real_point3d hint_point;
				boolean within_range;
				boolean correct_facing;
				boolean could_potentially_fake;

				if (action_vehicle_evaluate_seat(
					actor_index,
					state_data->vehicle_index,
					state_data->seat_index,
					!state_data->impromptu,
					&entry_point,
					&entry_facing,
					&hint_point,
					NULL,
					&within_range,
					&correct_facing,
					&could_potentially_fake))
				{
					if (could_potentially_fake)
					{
						state_data->fake_entry_potential_timer++;
						if (state_data->fake_entry_potential_timer >= 30)
						{
							correct_facing = TRUE;
							within_range = TRUE;
						}
					}
					else
					{
						state_data->fake_entry_potential_timer = 0;
					}

					if (within_range)
					{
						if (correct_facing)
						{
							unit_enter_seat(
								actor->meta.unit_index,
								state_data->vehicle_index,
								state_data->seat_index);
							state_data->started_entry = TRUE;
						}
						else
						{
							actor_move_halt(actor_index);
						}
					}
					else if (actor->meta.timeslice)
					{
						boolean moving = FALSE;

						if (action_vehicle_find_destination(
							actor_index,
							state_data->vehicle_index,
							&entry_point,
							&entry_facing,
							&hint_point,
							&state_data->ignore_hint,
							&state_data->destination_point,
							&state_data->destination_surface_index))
						{
							moving = actor_move_to_point(
								actor_index,
								&state_data->destination_point,
								state_data->destination_surface_index,
								state_data->vehicle_index);
						}

						if (moving)
						{
							state_data->pathfinding_failures = 0;
						}
						else
						{
							short maximum_pathfinding_failures =
								state_data->impromptu ? 5 : 50;

							state_data->pathfinding_failures++;
							if (state_data->pathfinding_failures >
								maximum_pathfinding_failures)
							{
								state_data->vehicle_entry_failed = TRUE;
							}
						}
					}

					state_data->lock_facing = distance_squared3d(
						&actor->input.position.body_position,
						&entry_point) < 1.0f;
					state_data->destination_facing = entry_facing;
					state_data->currently_correct_facing = correct_facing;
					state_data->currently_within_range = within_range;
				}
				else
				{
					state_data->vehicle_entry_failed = TRUE;
				}
			}
			else
			{
				state_data->vehicle_entry_failed = TRUE;
			}
		}
	}

	return state_data->vehicle_entry_done || state_data->vehicle_entry_failed;
}

boolean action_vehicle_setup_impromptu(
	long actor_index,
	long vehicle_index,
	real attempt_distance,
	real continue_distance,
	struct vehicle_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = FALSE;

	match_assert(
		"c:\\halo\\SOURCE\\ai\\action_vehicle.c",
		56,
		state_data);

	csmemset(state_data, 0, sizeof(*state_data));
	state_data->attempt_distance = attempt_distance;
	state_data->continue_distance = continue_distance;
	if (actor->input.vehicle_index == NONE &&
		!actor->meta.swarm &&
		actor->state.action != _actor_action_vehicle &&
		action_vehicle_desirable(
			actor_index,
			vehicle_index,
			FALSE,
			state_data->attempt_distance,
			state_data->continue_distance,
			FALSE,
			FALSE))
	{
		real_point3d entry_point;
		real_vector3d entry_facing;
		real_point3d hint_point;

		state_data->vehicle_index = vehicle_index;
		state_data->seat_index = action_vehicle_find_impromptu_seat(
			actor_index,
			vehicle_index,
			&entry_point,
			&entry_facing,
			&hint_point);
		if (state_data->seat_index != NONE)
		{
			state_data->impromptu = TRUE;
			if (unit_has_animation_to_enter_seat(
				actor->meta.unit_index,
				vehicle_index,
				state_data->seat_index) &&
				action_vehicle_find_destination(
					actor_index,
					vehicle_index,
					&entry_point,
					&entry_facing,
					&hint_point,
					NULL,
					&state_data->destination_point,
					&state_data->destination_surface_index) &&
				actor_move_to_point(
					actor_index,
					&state_data->destination_point,
					state_data->destination_surface_index,
					vehicle_index))
			{
				result = TRUE;
			}
		}
	}

	return result;
}

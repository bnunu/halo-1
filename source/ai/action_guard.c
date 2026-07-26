/*
ACTION_GUARD.C

symbols in this file:
00003E80 0080:
	_action_guard_setup_current_position (0000)
00003F00 00d0:
	_action_guard_setup_find_position (0000)
00003FD0 0160:
	_action_guard_setup_postcombat (0000)
00004130 0040:
	_action_guard_begin (0000)
00004170 0040:
	_action_guard_end (0000)
000041B0 0140:
	_code_000041b0 (0000)
000042F0 01d0:
	_action_guard_update (0000)
000044C0 0080:
	_action_guard_flush_position_indices (0000)
00004540 0030:
	_action_guard_flush_structure_indices (0000)
00004570 00a0:
	_action_guard_modify_color (0000)
00004610 0050:
	_action_guard_replace_prop (0000)
00004660 01c0:
	_action_guard_setup_from_combat_transition (0000)
00004820 01c0:
	_action_guard_setup_from_fleeing (0000)
000049E0 01c0:
	_action_guard_perform (0000)
00004BA0 03d0:
	_action_guard_control (0000)
002431D8 0021:
	??_C@_0CB@GJJCDDDH@c?3?2halo?2SOURCE?2ai?2action_guard?4c@ (0000)
002431FC 0018:
	??_C@_0BI@NGADMAPA@state_data?9?$DOpost_combat?$AA@ (0000)
00243214 0034:
	??_C@_0DE@PILBLNDH@actor?9?$DOstimuli?4combat_transition@ (0000)
00243248 0021:
	??_C@_0CB@CNNLKLII@actor?9?$DOstimuli?4combat_transition@ (0000)
0024326C 0004:
	__real@41100000 (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "actions.h"

#include "actors.h"
#include "actor_definitions.h"
#include "props.h"

/* ---------- constants */

enum
{
	_actor_guard_crouch_when_inactive_bit = 6,
	_actor_guard_crouch_when_uncovered_bit = 7,
	_actor_guard_crouch_when_cowering_bit = 23,
};

/* ---------- macros */

/*
 * January cachebeta layout evidence.  The inherited HCEX prop and actor
 * control layouts differ at these fields; keep the accessors local until the
 * shared structures are corrected independently.
 */
#define action_guard_prop_is_dead(prop) (*(boolean *)((byte *)(prop) + 0x60))
#define action_guard_actor_is_moving(actor) (*(boolean *)((byte *)(actor) + 0x484))
#define action_guard_cower_retreat_timer(actor) (*(short *)((byte *)(actor) + 0x3A8))

/* ---------- structures */

struct firing_position_search_definition
{
	long allowed_groups;
	short firing_position_group;
	byte unresolved[0x66A];
};

struct firing_position_search_workspace
{
	byte unresolved[0x1408C];
};

struct firing_position_candidate
{
	byte unresolved[0x3C];
};

/* ---------- prototypes */

boolean game_team_is_ally(
	short team_a,
	short team_b);

void ai_communication_event(
	short type,
	long unit_index,
	long prop_index,
	long object_index,
	long position_index,
	long structure_index,
	boolean allow_reply);

void actor_set_dormant(
	long actor_index,
	boolean dormant);

long actor_target_unit_index(
	long actor_index);

void actor_discard_firing_position(
	long actor_index,
	short firing_position_index,
	boolean temporary);

void actor_perception_forget_recent_damage(
	long actor_index);

void actor_perception_retreat_successful(
	long actor_index);

void actor_perception_find_prop_pathfinding_location(
	long actor_index,
	long prop_index);

boolean actor_nearby_firing_positions(
	long actor_index,
	real_point3d const *point,
	long surface_index,
	boolean allow_outside_range);

void actor_move_halt(
	long actor_index);

void actor_move_to_point(
	long actor_index,
	real_point3d const *point,
	long surface_index,
	long ignore_object_index);

boolean actor_move_to_firing_position(
	long actor_index,
	short firing_position_index,
	boolean allow_pathfinding_failure);

void actor_stimulus_suspicion(
	long actor_index,
	short suspicion_level,
	long ticks);

long actor_get_firing_position_group(
	long actor_index,
	short group,
	boolean include_discarded);

long actor_select_firing_position(
	long actor_index,
	struct firing_position_search_definition *search,
	struct firing_position_candidate *candidate,
	long *previous_owner_actor_index,
	struct firing_position_search_workspace *workspace,
	long *position_flags);

short actor_change_firing_position(
	long actor_index,
	long firing_position_index,
	struct firing_position_candidate *candidate,
	long previous_owner_actor_index,
	struct firing_position_search_workspace *workspace,
	long position_flags);

/* ---------- globals */

/* ---------- public code */

boolean
action_guard_setup_current_position(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 114, state_data);
	csmemset(state_data, 0, sizeof(*state_data));
	state_data->guard_location_type = 1;
	state_data->has_guard_direction = TRUE;
	state_data->guard_direction = actor->input.facing_vector;
	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

boolean
action_guard_setup_find_position(
	long actor_index,
	long wait_ticks,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 134, state_data);
	csmemset(state_data, 0, sizeof(*state_data));

	if (!actor->input.vehicle_passenger && !actor->meta.swarm)
	{
		state_data->wait_ticks = wait_ticks;
		if ((short)wait_ticks == 0)
		{
			state_data->find_new_guard_position = TRUE;
			state_data->guard_location_type = 0;
		}
		else
		{
			state_data->guard_location_type = 1;
			state_data->has_guard_direction = TRUE;
			state_data->guard_direction = actor->input.facing_vector;
		}
	}
	else
	{
		state_data->guard_location_type = 1;
	}

	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

boolean
action_guard_setup_postcombat(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	boolean result = TRUE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 172, state_data);
	csmemset(state_data, 0, sizeof(*state_data));
	state_data->wait_ticks = 120;
	state_data->guard_location_type = 1;
	state_data->post_combat = TRUE;
	state_data->guard_look_prop_index = NONE;

	if (actor->input.vehicle_driver_type == 4)
	{
		result = FALSE;
	}
	else if (!actor->input.vehicle_passenger &&
		!actor->meta.swarm &&
		actor->external_orders.postcombat_prop_index != NONE)
	{
		struct prop_datum *prop = prop_get(actor->external_orders.postcombat_prop_index);

		state_data->guard_look_prop_index = actor->external_orders.postcombat_prop_index;
		state_data->look_ticks = 120;
		state_data->guard_look_until_reached_point = TRUE;

		switch (actor->external_orders.postcombat_type)
		{
		case _actor_postcombat_run_to:
			state_data->guard_point.radius = 2.f;
			break;
		case _actor_postcombat_check_enemy:
		case _actor_postcombat_check_friend:
			state_data->guard_point.radius = 1.f;
			break;
		case _actor_postcombat_shoot_corpse:
			state_data->guard_point.radius = 1.5f;
			break;
		default:
			goto done;
		}

		actor_perception_find_prop_pathfinding_location(
			actor_index,
			actor->external_orders.postcombat_prop_index);
		state_data->guard_location_type = 2;
		state_data->guard_point.position = prop->pathfinding_point;
		state_data->guard_point.surface_index = prop->pathfinding_surface_index;
	}

done:
	return result;
}

boolean
action_guard_setup_from_combat_transition(
	long actor_index,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 32, state_data);
	csmemset(state_data, 0, sizeof(*state_data));
	state_data->wait_ticks = (short)actor->stimuli.combat_transition_guard_timer;

	if (!actor->input.vehicle_passenger && !actor->meta.swarm)
	{
		match_assert(
			"c:\\halo\\SOURCE\\ai\\action_guard.c",
			45,
			actor->stimuli.combat_transition);

		if (actor->stimuli.combat_transition_guard_at_point &&
			actor_nearby_firing_positions(
				actor_index,
				&actor->stimuli.combat_transition_guard_point,
				actor->stimuli.combat_transition_guard_point_surface_index,
				TRUE))
		{
			state_data->guard_location_type = 2;
			state_data->guard_point.position = actor->stimuli.combat_transition_guard_point;
			state_data->guard_point.surface_index = actor->stimuli.combat_transition_guard_point_surface_index;
			state_data->guard_point.radius = actor->stimuli.combat_transition_guard_point_distance;
		}
		else if (state_data->wait_ticks > 0)
		{
			state_data->guard_location_type = 1;
			state_data->has_guard_direction = actor->stimuli.combat_transition_has_vector;
			state_data->aim_in_guard_direction = FALSE;
			if (state_data->has_guard_direction)
			{
				state_data->guard_direction = actor->stimuli.combat_transition_vector;
				if (normalize3d(&state_data->guard_direction) == 0.f)
				{
					state_data->has_guard_direction = FALSE;
				}
			}
		}
		else
		{
			state_data->guard_location_type = 0;
			state_data->find_new_guard_position = TRUE;
		}
	}
	else
	{
		state_data->guard_location_type = 1;
	}

	if (actor->stimuli.combat_transition == 2)
	{
		match_assert(
			"c:\\halo\\SOURCE\\ai\\action_guard.c",
			90,
			actor->stimuli.combat_transition_prop_index != NONE);
		state_data->shout_about_dead_friend = TRUE;
		state_data->shout_dead_friend_prop_index = actor->stimuli.combat_transition_prop_index;
	}

	state_data->guard_look_prop_index = actor->stimuli.combat_transition_prop_index;
	if (state_data->guard_look_prop_index != NONE)
	{
		state_data->look_ticks = (short)actor->stimuli.combat_transition_prop_look_timer;
		state_data->guard_look_until_reached_point = actor->stimuli.combat_transition_prop_look_while_moving;
	}

	return TRUE;
}

boolean
action_guard_setup_from_fleeing(
	long actor_index,
	struct flee_state_data *flee_state,
	struct guard_state_data *state_data)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *definition = actor_definition_get(actor->meta.definition_index);

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 237, state_data);
	csmemset(state_data, 0, sizeof(*state_data));

	if (!actor->input.vehicle_passenger && !actor->meta.swarm)
	{
		state_data->wait_ticks = 0;
		state_data->cower = TRUE;
		state_data->cower_ticks = 0;
		state_data->cower_panicked = flee_state->panic_type > 0;
		state_data->cower_from_retreat =
			action_guard_cower_retreat_timer(actor) > 0 &&
			!state_data->cower_panicked;

		if (!state_data->cower_from_retreat)
		{
			if (state_data->cower_panicked)
			{
				real upper_bound = definition->panic.cower_time_upper_bound;
				real lower_bound = definition->panic.cower_time_lower_bound;

				state_data->cower_ticks = (short)(real_seed_random_range(
					get_global_random_seed_address(),
					lower_bound,
					upper_bound) * 30.f);
			}
			else
			{
				real upper_bound = definition->defensive.hide_time_upper_bound;
				real lower_bound = definition->defensive.hide_time_lower_bound;

				state_data->cower_ticks = (short)(real_seed_random_range(
					get_global_random_seed_address(),
					lower_bound,
					upper_bound) * 30.f);
			}
		}

		if (flee_state->flee_firing_position_index == NONE)
		{
			state_data->find_new_guard_position = TRUE;
			state_data->guard_location_type = 0;
		}
		else
		{
			state_data->find_new_guard_position = FALSE;
			state_data->guard_location_type = 3;
			state_data->guard_firing_position_index = flee_state->flee_firing_position_index;
			if (flee_state->has_approach_point)
			{
				state_data->has_guard_direction = TRUE;
				state_data->aim_in_guard_direction = TRUE;
				vector_from_points3d(
					&actor->input.position.body_position,
					&flee_state->approach_point,
					&state_data->guard_direction);
				if (normalize3d(&state_data->guard_direction) == 0.f)
				{
					state_data->has_guard_direction = FALSE;
				}
			}
		}
	}
	else
	{
		state_data->guard_location_type = 1;
	}

	state_data->guard_look_prop_index = NONE;

	return TRUE;
}

void
action_guard_begin(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	actor_perception_forget_recent_damage(actor_index);
	actor->state.searching = FALSE;
	if (state_data->cower_from_retreat)
	{
		actor_perception_retreat_successful(actor_index);
	}

	return;
}

void
action_guard_end(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);

	if (actor->state.action_data.guard.post_combat)
	{
		actor->external_orders.postcombat_type = _actor_postcombat_none;
		actor->external_orders.postcombat_prop_index = NONE;
	}

	return;
}

void
action_guard_flush_position_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (state_data->cower && state_data->guard_location_type == 3)
	{
		state_data->cower = FALSE;
		state_data->cower_ticks = 0;
		state_data->cower_from_retreat = FALSE;
	}

	if (state_data->guard_location_type == 3 ||
		(state_data->guard_location_type == 1 && !actor->input.vehicle_passenger))
	{
		state_data->guard_location_type = 0;
		state_data->guard_firing_position_index = NONE;
		state_data->find_new_guard_position = TRUE;
	}

	return;
}

void
action_guard_flush_structure_indices(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (state_data->guard_location_type == 2)
	{
		state_data->guard_point.surface_index = NONE;
	}

	return;
}

void
action_guard_modify_color(
	long actor_index,
	real_argb_color *color)
{
	struct guard_state_data *state_data = &actor_get(actor_index)->state.action_data.guard;

	if (!state_data->cower)
	{
		*color = *global_real_argb_salmon;
	}
	else if (state_data->cower_from_retreat)
	{
		*color = *global_real_argb_violet;
	}
	else if (state_data->cower_panicked)
	{
		*color = *global_real_argb_pink;
	}
	else
	{
		*color = *global_real_argb_purple;
	}

	return;
}

void
action_guard_replace_prop(
	long actor_index,
	long old_prop_index,
	long new_prop_index)
{
	struct guard_state_data *state_data = &actor_get(actor_index)->state.action_data.guard;

	if (state_data->guard_look_prop_index == old_prop_index)
	{
		state_data->guard_look_prop_index = new_prop_index;
	}

	if (state_data->shout_dead_friend_prop_index == old_prop_index)
	{
		state_data->shout_dead_friend_prop_index = new_prop_index;
		if (new_prop_index == NONE)
		{
			state_data->shout_about_dead_friend = FALSE;
		}
	}

	return;
}

/* ---------- private code */

/* action_guard_post_combat */
static boolean code_000041b0(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;
	boolean result = FALSE;

	match_assert("c:\\halo\\SOURCE\\ai\\action_guard.c", 344, state_data->post_combat);
	if (!state_data->post_combat_vocalized)
	{
		short communication_type = NONE;

		switch (actor->external_orders.postcombat_type)
		{
		case _actor_postcombat_speak_alone:
			communication_type = 48;
			break;
		case _actor_postcombat_speak_unscathed:
			communication_type = 49;
			break;
		case _actor_postcombat_speak_wounded:
			communication_type = 50;
			break;
		case _actor_postcombat_speak_massacre:
			communication_type = 51;
			break;
		case _actor_postcombat_speak_triumph:
			communication_type = 52;
			break;
		case _actor_postcombat_check_enemy:
			communication_type = 53;
			break;
		case _actor_postcombat_check_friend:
			communication_type = 54;
			break;
		case _actor_postcombat_shoot_corpse:
			communication_type = 55;
			break;
		case _actor_postcombat_celebrate:
			communication_type = 56;
			break;
		}

		if (communication_type != NONE)
		{
			long prop_unit_index = NONE;
			long prop_type = NONE;

			if (state_data->guard_look_prop_index != NONE)
			{
				struct prop_datum *prop = prop_get(state_data->guard_look_prop_index);

				prop_unit_index = prop->unit_index;
				if (action_guard_prop_is_dead(prop))
				{
					prop_type = game_team_is_ally(actor->meta.team_index, prop->team_index) ? 4 : 3;
				}
				else
				{
					prop_type = 2;
				}
			}

			ai_communication_event(
				communication_type,
				actor->meta.unit_index,
				prop_unit_index,
				prop_type,
				NONE,
				NONE,
				FALSE);
			result = TRUE;
		}

		state_data->post_combat_vocalized = TRUE;
	}

	return result;
}

void
action_guard_update(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (!actor->meta.dormant &&
		action_guard_actor_is_moving(actor) &&
		state_data->wait_ticks > 0)
	{
		state_data->wait_ticks--;
		if (state_data->wait_ticks == 0 &&
			!actor->input.vehicle_passenger &&
			!actor->meta.swarm)
		{
			if (state_data->post_combat)
			{
				code_000041b0(actor_index);
				actor->external_orders.postcombat_type = _actor_postcombat_none;
				actor->external_orders.postcombat_prop_index = NONE;
				state_data->post_combat = FALSE;
				state_data->post_combat_shooting = FALSE;
				state_data->guard_look_prop_index = NONE;
			}
			state_data->find_new_guard_position = TRUE;
		}
	}

	if (state_data->look_ticks > 0 &&
		(!state_data->guard_look_until_reached_point || action_guard_actor_is_moving(actor)))
	{
		state_data->look_ticks--;
		if (state_data->look_ticks == 0)
		{
			state_data->guard_look_prop_index = NONE;
		}
	}

	if (state_data->path_begun && action_guard_actor_is_moving(actor))
	{
		boolean cower_finished;

		if (state_data->cower_from_retreat)
		{
			state_data->cower_from_retreat = action_guard_cower_retreat_timer(actor) > 0;
			cower_finished = !state_data->cower_from_retreat;
		}
		else if (state_data->cower_ticks > 0)
		{
			state_data->cower_ticks--;
			cower_finished = state_data->cower_ticks == 0;
		}
		else
		{
			cower_finished = FALSE;
		}

		if (cower_finished)
		{
			actor_set_dormant(actor_index, FALSE);
			state_data->cower = FALSE;
			state_data->cower_panicked = FALSE;
			state_data->cower_from_retreat = FALSE;
			state_data->cower_ticks = 0;

			if (actor->state.combat_status >= 2 && actor->meta.unit_index != NONE)
			{
				ai_communication_event(
					35,
					actor->meta.unit_index,
					actor_target_unit_index(actor_index),
					NONE,
					NONE,
					NONE,
					FALSE);
			}

			if (state_data->guard_location_type == 3)
			{
				actor_discard_firing_position(
					actor_index,
					state_data->guard_firing_position_index,
					FALSE);
				state_data->guard_firing_position_index = NONE;
			}

			actor->firing_positions.current_position_index = NONE;
			if (actor->input.vehicle_passenger)
			{
				state_data->guard_location_type = 1;
				return;
			}

			state_data->guard_location_type = 0;
			state_data->find_new_guard_position = TRUE;
		}
	}

	return;
}

boolean
action_guard_perform(
	long actor_index)
{
	long position_flags;
	long previous_owner_actor_index;
	long selected_firing_position_index;
	real guard_position_time_lower_bound;
	real guard_position_time_upper_bound;
	short firing_position_index;
	struct firing_position_candidate candidate;
	struct firing_position_search_definition search;
	struct firing_position_search_workspace workspace;
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *definition = actor_definition_get(actor->meta.definition_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (actor->meta.swarm)
	{
		state_data->guard_location_type = 1;
		return FALSE;
	}

	if (actor->input.vehicle_passenger)
	{
		state_data->guard_location_type = 1;
		state_data->find_new_guard_position = TRUE;
		return FALSE;
	}

	if (state_data->guard_location_type == 3 &&
		actor->firing_positions.current_position_index == NONE)
	{
		state_data->guard_location_type = 0;
		state_data->find_new_guard_position = TRUE;
	}

	if (actor->meta.timeslice && state_data->find_new_guard_position)
	{
		if (state_data->guard_location_type == 3 &&
			actor->firing_positions.current_position_index != NONE)
		{
			actor_discard_firing_position(
				actor_index,
				actor->firing_positions.current_position_index,
				FALSE);
		}

		csmemset(&search, 0, sizeof(search));
		search.firing_position_group = 4;
		search.allowed_groups = actor_get_firing_position_group(actor_index, 4, FALSE);
		search.unresolved[0xF] = TRUE;
		selected_firing_position_index = actor_select_firing_position(
			actor_index,
			&search,
			&candidate,
			&previous_owner_actor_index,
			&workspace,
			&position_flags);
		firing_position_index = actor_change_firing_position(
			actor_index,
			selected_firing_position_index,
			&candidate,
			previous_owner_actor_index,
			&workspace,
			position_flags);
		state_data->find_new_guard_position = FALSE;
		state_data->has_guard_direction = FALSE;
		if (firing_position_index == NONE)
		{
			state_data->guard_location_type = 1;
		}
		else
		{
			state_data->guard_location_type = 3;
			state_data->guard_firing_position_index = firing_position_index;
		}

		guard_position_time_upper_bound = definition->firing_position.guard_position_time_upper_bound;
		guard_position_time_lower_bound = definition->firing_position.guard_position_time_lower_bound;
		state_data->wait_ticks = (short)(real_seed_random_range(
			get_global_random_seed_address(),
			guard_position_time_lower_bound,
			guard_position_time_upper_bound) * 30.f);
	}

	return FALSE;
}

void
action_guard_control(
	long actor_index)
{
	struct actor_datum *actor = actor_get(actor_index);
	struct actor_definition *definition = actor_definition_get(actor->meta.definition_index);
	struct guard_state_data *state_data = &actor->state.action_data.guard;

	if (TEST_FLAG(definition->flags, _actor_guard_crouch_when_inactive_bit) &&
		actor->state.combat_status == 0)
	{
		actor->orders.move.stationary_crouch = TRUE;
		actor->orders.move.moving_crouch = TRUE;
	}
	else
	{
		actor->orders.move.moving_crouch = FALSE;
		if (state_data->cower)
		{
			if (state_data->cower_from_retreat)
			{
				actor->orders.move.stationary_crouch =
					TEST_FLAG(definition->flags, _actor_guard_crouch_when_cowering_bit);
			}
			else
			{
				actor->orders.move.stationary_crouch = TRUE;
			}
		}
		else
		{
			actor->orders.move.stationary_crouch =
				TEST_FLAG(definition->flags, _actor_guard_crouch_when_uncovered_bit) &&
				actor->state.combat_status > 0;
		}
	}

	actor->orders.move.panicked = FALSE;
	actor->orders.move.dive_into_cover = FALSE;
	actor->orders.move.emerge_from_cover = FALSE;

	if (actor->meta.timeslice && !actor->meta.swarm)
	{
		boolean path_succeeded = FALSE;

		switch (state_data->guard_location_type)
		{
		case 0:
		case 1:
			actor_move_halt(actor_index);
			path_succeeded = TRUE;
			break;

		case 2:
		{
			real distance_squared = distance_squared3d(
				&actor->input.position.body_position,
				&state_data->guard_point.position);

			if (distance_squared < state_data->guard_point.radius * state_data->guard_point.radius)
			{
				actor_move_halt(actor_index);
			}
			else
			{
				actor_move_to_point(
					actor_index,
					&state_data->guard_point.position,
					state_data->guard_point.surface_index,
					NONE);
			}

			path_succeeded = distance_squared < 9.f;
			break;
		}

		case 3:
		{
			if (state_data->guard_firing_position_index != NONE)
			{
				actor->firing_positions.current_position_index = state_data->guard_firing_position_index;
				actor->firing_positions.current_position_found_outside_range = FALSE;
				if (!actor_move_to_firing_position(
					actor_index,
					actor->firing_positions.current_position_index,
					FALSE))
				{
					actor_discard_firing_position(
						actor_index,
						state_data->guard_firing_position_index,
						FALSE);
					actor->firing_positions.current_position_index = NONE;
				}
			}

			path_succeeded = !actor->control.path.path.valid ||
				distance_squared3d(
					&actor->control.path.path.endpoint.point,
					&actor->input.position.body_position) < 9.f;
			break;
		}

		default:
			display_assert(NULL, "c:\\halo\\SOURCE\\ai\\action_guard.c", 685, TRUE);
			system_exit(-1);
			break;
		}

		state_data->path_begun = TRUE;
		if (path_succeeded)
		{
			if (state_data->shout_about_dead_friend)
			{
				struct prop_datum *prop = prop_get(state_data->shout_dead_friend_prop_index);

				state_data->shout_about_dead_friend = FALSE;
				state_data->shout_dead_friend_prop_index = NONE;
				actor_stimulus_suspicion(actor_index, 2, 600);
				ai_communication_event(
					7,
					actor->meta.unit_index,
					prop->unit_index,
					NONE,
					NONE,
					2,
					FALSE);
			}

			if (state_data->post_combat)
			{
				code_000041b0(actor_index);
				if (actor->external_orders.postcombat_type == _actor_postcombat_shoot_corpse &&
					state_data->guard_location_type == 2)
				{
					state_data->post_combat_shooting = TRUE;
				}
			}
		}
	}

	if (state_data->post_combat_shooting)
	{
		real_vector3d const *up;

		actor->orders.look.primary_priority = 7;
		actor->orders.look.primary_direction.type = _direction_specification_target;
		actor->orders.combat.shoot_at_target = TRUE;
		actor->orders.combat.use_manual_target_point = TRUE;
		up = global_up3d;
		actor->orders.combat.target_point.x =
			state_data->guard_point.position.x + up->i * 0.05f;
		actor->orders.combat.target_point.y =
			state_data->guard_point.position.y + up->j * 0.05f;
		actor->orders.combat.target_point.z =
			state_data->guard_point.position.z + up->k * 0.05f;
	}
	else if (state_data->guard_look_prop_index != NONE)
	{
		actor->orders.look.primary_priority = 5;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = state_data->guard_look_prop_index;
	}
	else if (state_data->has_guard_direction)
	{
		actor->orders.look.primary_priority = state_data->aim_in_guard_direction ? 5 : 3;
		actor->orders.look.primary_direction.type = _direction_specification_vector;
		actor->orders.look.primary_direction.vector = state_data->guard_direction;
	}
	else if (actor->state.combat_status > 0 &&
		actor->target.target_prop_index != NONE)
	{
		actor->orders.look.primary_priority = 3;
		actor->orders.look.primary_direction.type = _direction_specification_prop;
		actor->orders.look.primary_direction.prop_index = actor->target.target_prop_index;
	}
	else
	{
		actor->orders.look.primary_priority = _primary_priority_none;
	}

	actor->orders.look.idle_look_type = actor->state.combat_status >= 4 ? 4 : 2;

	return;
}

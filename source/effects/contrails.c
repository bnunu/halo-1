/*
CONTRAILS.C

symbols in this file:
00085EE0 0020:
	_contrail_scale_value (0000)
00085F00 0020:
	_contrails_initialize_for_new_map (0000)
00085F20 0020:
	_contrails_dispose_from_old_map (0000)
00085F40 0020:
	_contrails_dispose (0000)
00085F60 0080:
	_contrail_delete (0000)
00085FE0 0010:
	_contrails_disconnect_from_structure_bsp (0000)
00085FF0 00d0:
	_contrails_reconnect_to_structure_bsp (0000)
000860C0 0090:
	_contrail_compute_new_point_count (0000)
00086150 01a0:
	_contrail_verify (0000)
000862F0 0020:
	_local_random_range (0000)
00086310 0030:
	_local_random_vector_in_cone3d (0000)
00086340 0070:
	_contrail_scale_random_value (0000)
000863B0 0070:
	_contrails_initialize (0000)
00086420 0090:
	_contrail_next_frame (0000)
000864B0 03c0:
	_contrail_add_points (0000)
00086870 0380:
	_contrail_update_points (0000)
00086BF0 0150:
	_contrail_new (0000)
00086D40 0080:
	_contrail_owner_collision (0000)
00086DC0 0220:
	_contrails_update (0000)
0025976C 0039:
	??_C@_0DJ@LAEOKLOH@contrail?5?$CFs?5thinks?5it?5has?5?$CFd?5poi@ (0000)
002597A8 0041:
	??_C@_0EB@OBNMHKML@contrail?5?$CFs?5?$CI?$CFld?$CJ?5has?5an?5object?5@ (0000)
002597EC 003c:
	??_C@_0DM@BEOPFMNL@contrail?5?$CFs?5attachment?5index?5?$CFd?5@ (0000)
00259828 0023:
	??_C@_0CD@HFOFGNFP@c?3?2halo?2SOURCE?2effects?2contrails@ (0000)
0025984C 0023:
	??_C@_0CD@MIFFODIN@couldn?8t?5allocate?5contrail?5globa@ (0000)
00259870 000f:
	??_C@_0P@NFJLENP@contrail?5point?$AA@ (0000)
00259880 0009:
	??_C@_08LACPGLIF@contrail?$AA@ (0000)
0025988C 0002:
	??_C@_01PFHFFBPC@t?$AA@ (0000)
00259890 0017:
	??_C@_0BH@KJOJEDNC@definition_index?$CB?$DNNONE?$AA@ (0000)
002598A8 0013:
	??_C@_0BD@GHNECCAG@object_index?$CB?$DNNONE?$AA@ (0000)
002598C0 0070:
	??_C@_0HA@NOKFPIOB@contrail?9?$DOcontrail_point_counts?$FL@ (0000)
*/

/* ---------- headers */

#include "effects/contrails.h"

#include "bitmaps/bitmap_group.h"
#include "cseries/errors.h"
#include "effects/contrail_definitions.h"
#include "game/game.h" /* port: game_players_are_double_speed */
#include "memory/data.h"
#include "objects/object_definitions.h"
#include "physics/point_physics.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

/* ---------- constants */

enum
{
	MAXIMUM_CONTRAIL_INSTANCES = 4,
	/* the native builds' larger contrail point pool (halo_port_capacity.h);
	contrail_update_points lists a contrail's points in an array of this size
	without a bound check, so it must stay the size of the pool */
	MAXIMUM_CONTRAIL_POINTS = HALO_PORT_MAXIMUM_CONTRAIL_POINTS
};

enum
{
	_contrail_active_bit = 0
};

enum
{
	_contrail_point_new_bit = 0,
	_contrail_point_transitioning_bit,
	_contrail_point_expired_bit
};

enum
{
	_contrail_state_duration_bit = 0,
	_contrail_state_duration_delta_bit,
	_contrail_state_transition_duration_bit,
	_contrail_state_transition_duration_delta_bit
};

enum
{
	_contrail_scales_point_generation_rate_bit = 0,
	_contrail_scales_point_velocity_bit,
	_contrail_scales_point_velocity_delta_bit,
	_contrail_scales_point_velocity_cone_angle_bit,
	_contrail_scales_inherited_velocity_fraction_bit,
	_contrail_scales_sequence_animation_rate_bit,
	_contrail_scales_texture_scale_u_bit,
	_contrail_scales_texture_scale_v_bit,
	_contrail_scales_texture_animation_u_bit,
	_contrail_scales_texture_animation_v_bit
};

/* ---------- macros */

#define contrail_get(index) \
	((struct contrail_datum *)datum_get(contrail_data, (index)))
#define contrail_point_get(index) \
	((struct contrail_point_datum *)datum_get(contrail_point_data, (index)))

/* ---------- structures */

/* ---------- prototypes */

static void contrail_next_frame(
	struct contrail_datum *contrail);
static short contrail_compute_new_point_count(
	long contrail_index,
	real dt);
static void contrail_add_points(
	long contrail_index,
	short contrail_point_count,
	boolean force);
static void contrail_update_points(
	long contrail_index,
	real dt);
static void contrail_verify(
	long contrail_index);

/* ---------- globals */

/* ---------- public code */

real contrail_scale_value(
	real value,
	real scale,
	unsigned long flags,
	short flag_bit)
{
	real result = scale;

	if (TEST_FLAG(flags, flag_bit))
		result *= value;

	return result;
}

real contrail_scale_random_value(
	real value,
	real lower_bound,
	real upper_bound,
	unsigned long flags,
	short flag_bit)
{
	real minimum = contrail_scale_value(
		value,
		lower_bound,
		flags,
		flag_bit);
	real range = contrail_scale_value(
		value,
		upper_bound - lower_bound,
		flags,
		flag_bit + 1);

	return real_local_random_range(0.0f, range) + minimum;
}

void contrails_initialize(
	void)
{
	/* the native builds' larger contrail pools (halo_port_capacity.h) */
	contrail_data = game_state_data_new("contrail", HALO_PORT_MAXIMUM_CONTRAILS, 0x44);
	contrail_point_data = game_state_data_new("contrail point", MAXIMUM_CONTRAIL_POINTS, 0x38);
	if (contrail_data && contrail_point_data)
		return;

	if (contrail_point_data)
	{
		contrail_point_data = NULL;
		error(_error_immediate, "couldn't allocate contrail globals");
		return;
	}

	if (contrail_data)
		contrail_data = NULL;

	error(_error_immediate, "couldn't allocate contrail globals");

	return;
}

void contrails_initialize_for_new_map(
	void)
{
	data_make_valid(contrail_data);
	data_make_valid(contrail_point_data);

	return;
}

void contrails_dispose_from_old_map(
	void)
{
	data_make_invalid(contrail_point_data);
	data_make_invalid(contrail_data);

	return;
}

void contrails_dispose(
	void)
{
	if (contrail_point_data)
		contrail_point_data = NULL;
	if (contrail_data)
		contrail_data = NULL;

	return;
}

void contrail_delete(
	long contrail_index)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	short instance_index;

	for (instance_index = 0; instance_index < MAXIMUM_CONTRAIL_INSTANCES; instance_index++)
	{
		long contrail_point_index = contrail->first_contrail_point_indices[instance_index];

		while (contrail_point_index != NONE)
		{
			long next_contrail_point_index = contrail_point_get(contrail_point_index)->next_contrail_point_index;

			datum_delete(contrail_point_data, contrail_point_index);
			contrail_point_index = next_contrail_point_index;
		}
	}

	datum_delete(contrail_data, contrail_index);

	return;
}

void contrails_disconnect_from_structure_bsp(
	void)
{
	return;
}

void contrails_reconnect_to_structure_bsp(
	void)
{
	long contrail_index;

	for (contrail_index = data_next_index(contrail_data, NONE);
		contrail_index != NONE;
		contrail_index = data_next_index(contrail_data, contrail_index))
	{
		struct contrail_datum *contrail = contrail_get(contrail_index);
		struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
		short instance_index;

		for (instance_index = 0; instance_index < MAXIMUM_CONTRAIL_INSTANCES; instance_index++)
		{
			long contrail_point_index = contrail->first_contrail_point_indices[instance_index];

			while (contrail_point_index != NONE)
			{
				struct contrail_point_datum *contrail_point = contrail_point_get(contrail_point_index);

				if (contrail_point->location.cluster_index != NONE)
					scenario_location_from_point(&contrail_point->location, &contrail_point->position);

				contrail_point_index = contrail_point->next_contrail_point_index;
			}
		}
	}

	return;
}

long contrail_new(
	long definition_index,
	long object_index,
	short attachment_index)
{
	long contrail_index = NONE;

	match_assert(
		"c:\\halo\\SOURCE\\effects\\contrails.c",
		97,
		object_index!=NONE);
	match_assert(
		"c:\\halo\\SOURCE\\effects\\contrails.c",
		98,
		definition_index!=NONE);

	if (definition_index != NONE)
	{
		struct contrail_definition *definition = contrail_definition_get(definition_index);

		contrail_index = datum_new(contrail_data);
		if (contrail_index != NONE)
		{
			struct contrail_datum *contrail = contrail_get(contrail_index);
			struct object_datum *object = object_get(object_index);
			short instance_index;

			contrail->flags = 0;
			contrail->definition_index = definition_index;
			contrail->object_index = object_index;
			contrail->attachment_index = attachment_index;
			contrail->density_function_index = (short)(TAG_BLOCK_GET_ELEMENT(
				&object_definition_get(object->definition_index)->object.attachments,
				attachment_index,
				struct object_attachment_definition)->primary_scale_function_reference - 1);
			contrail->sequence_index = NONE;
			contrail_next_frame(contrail);
			contrail->texture_offset_u = 0.0f;
			contrail->texture_offset_v = 0.0f;

			for (instance_index = 0;
				instance_index < MAXIMUM_CONTRAIL_INSTANCES;
				instance_index++)
			{
				contrail->contrail_point_counts[instance_index] = 0;
				contrail->first_contrail_point_indices[instance_index] = NONE;
			}

			if (object_get_function_value(
				contrail->object_index,
				contrail->density_function_index,
				&contrail->density))
			{
				SET_FLAG(contrail->flags, _contrail_active_bit, TRUE);
				contrail_add_points(contrail_index, 1, TRUE);
			}
		}
	}

	return contrail_index;
}

void contrails_update(
	real dt)
{
	long contrail_index;
	/* The native builds update contrails every frame, several frames per
	tick (port/linux/game/render_interpolation.c), but objects and their
	markers move only on ticks: points due on a frame that ran no tick would
	land on the last point, be skipped and be lost. Emit on the first frame
	after each tick, for all the time since the last emission. */
	static long last_point_game_time = NONE;
	real point_dt;

	/* port: the ticks' own time, a 30th of a second each, as the Xbox's
	frames were (a frame a tick, a 29.97th of a second at 30 frames a
	second): the frames' time since the last tick was four or five frames'
	at 144 frames a second, 28 or 35 ms, so the points were made and aged by
	uneven steps and a short trail's fades (a plasma turret's) flickered from
	tick to tick. A game time gone back or jumping (a new map, a checkpoint)
	counts as a tick. A tick is half as long while the players are double
	speed, as game_frame halves the frame's time. */
	if (last_point_game_time != game_time_get())
	{
		long ticks = game_time_get() - last_point_game_time;

		if (last_point_game_time == NONE || ticks < 1 || ticks > TICKS_PER_SECOND)
			ticks = 1;
		last_point_game_time = game_time_get();
		point_dt = (real)ticks / TICKS_PER_SECOND;
		if (game_players_are_double_speed())
			point_dt *= 0.5f;
	}
	else
	{
		point_dt = 0.0f;
	}

	for (contrail_index = data_next_index(contrail_data, NONE);
		contrail_index != NONE;
		contrail_index = data_next_index(contrail_data, contrail_index))
	{
		struct contrail_datum *contrail = contrail_get(contrail_index);
		struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
		real frame_dt = dt - contrail->expired_dt;
		real frames_per_second;
		real frame_period;
		real remaining_dt;
		real texture_animation_u;
		real texture_animation_v;
		short instance_index;

		contrail_verify(contrail_index);
		contrail->expired_dt = 0.0f;

		if (contrail->object_index != NONE)
		{
			boolean active = object_get_function_value(
				contrail->object_index,
				contrail->density_function_index,
				&contrail->density);

			if (active != TEST_FLAG(contrail->flags, _contrail_active_bit))
			{
				real density = contrail->density;

				contrail->density = 0.0f;
				contrail_add_points(contrail_index, 1, TRUE);
				contrail->density = density;
			}

			SET_FLAG(contrail->flags, _contrail_active_bit, active);

			if (active)
			{
				contrail_add_points(
					contrail_index,
					contrail_compute_new_point_count(contrail_index, point_dt),
					TRUE);
			}
		}

		remaining_dt = frame_dt;
		frames_per_second = definition->frames_per_second;
		if (TEST_FLAG(definition->scale_flags, _contrail_scales_sequence_animation_rate_bit))
			frames_per_second = frames_per_second * contrail->density;
		frame_period = 1.0f / frames_per_second;

		while (remaining_dt > 0.0f)
		{
			real time_until_next_frame = frame_period - contrail->frame_time;

			if (time_until_next_frame <= remaining_dt)
			{
				contrail_next_frame(contrail);
				remaining_dt = remaining_dt - time_until_next_frame;
			}
			else
			{
				contrail->frame_time = remaining_dt + contrail->frame_time;
				break;
			}
		}

		texture_animation_u = definition->texture_animation_u;
		if (TEST_FLAG(definition->scale_flags, _contrail_scales_texture_animation_u_bit))
			texture_animation_u = texture_animation_u * contrail->density;
		contrail->texture_offset_u = contrail->texture_offset_u - texture_animation_u * frame_dt;

		texture_animation_v = definition->texture_animation_v;
		if (TEST_FLAG(definition->scale_flags, _contrail_scales_texture_animation_v_bit))
			texture_animation_v = texture_animation_v * contrail->density;
		contrail->texture_offset_v = texture_animation_v * frame_dt + contrail->texture_offset_v;

		/* port: the points age a tick at a time too, as on the Xbox, which
		drew a frame a tick: a point's state that ends drops the rest of the
		frame's time (contrail_update_points), so each state lasted at least a
		tick. A Shade's or a Spirit's points (states of 0.01, 0.05 and 0.005
		seconds) lived four ticks, 133 ms, and aged a frame at a time at 240
		frames a second they lived about half that, the trail half as long.
		The points' physics and their states' fades step with them, so at a
		high frame rate a point moves and fades once a tick, as on the Xbox,
		not on every frame */
		if (point_dt > 0.0f)
			contrail_update_points(contrail_index, point_dt);

		for (instance_index = 0;
			instance_index < MAXIMUM_CONTRAIL_INSTANCES;
			instance_index++)
		{
			match_assert(
				"c:\\halo\\SOURCE\\effects\\contrails.c",
				266,
				contrail->contrail_point_counts[instance_index]>0==contrail->first_contrail_point_indices[instance_index]!=NONE);

			if (contrail->first_contrail_point_indices[instance_index] != NONE)
				break;
		}

		contrail_verify(contrail_index);

		if (instance_index == MAXIMUM_CONTRAIL_INSTANCES &&
			contrail->object_index == NONE)
		{
			contrail_delete(contrail_index);
		}
	}

	return;
}

void contrail_owner_collision(
	long contrail_index,
	unsigned char object_dying,
	real dt)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);

	if (TEST_FLAG(contrail->flags, _contrail_active_bit))
	{
		short contrail_point_count = contrail_compute_new_point_count(contrail_index, dt);

		contrail_add_points(contrail_index, MAX(1, contrail_point_count), FALSE);
	}

	if (object_dying)
		contrail->object_index = NONE;

	contrail->expired_dt += dt;

	return;
}

/* ---------- private code */

static void contrail_verify(
	long contrail_index)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
	short instance_index;

	if (contrail->object_index != NONE)
	{
		struct object_datum *object = object_get(contrail->object_index);

		match_vassert(
			"c:\\halo\\SOURCE\\effects\\contrails.c",
			654,
			contrail->attachment_index >= 0 &&
				contrail->attachment_index < object_definition_get(
					object->definition_index)->object.attachments.count,
			csprintf(
				temporary,
				"contrail %s attachment index %d is outside the valid range.",
				tag_get_name(contrail->definition_index),
				contrail->attachment_index));
		/* port: and only an index the object holds (8; the first assert
		only logs in a release build, and the count is the map's) */
		if (VALID_INDEX(contrail->attachment_index, MAXIMUM_NUMBER_OF_ATTACHMENTS_PER_OBJECT))
		{
			match_vassert(
				"c:\\halo\\SOURCE\\effects\\contrails.c",
				655,
				object->object.attachment_indices[contrail->attachment_index] == contrail_index,
				csprintf(
					temporary,
					"contrail %s (%ld) has an object that thinks it's attached to %ld",
					tag_get_name(contrail->definition_index),
					contrail_index,
					object->object.attachment_indices[contrail->attachment_index]));
		}
	}

	for (instance_index = 0;
		instance_index < MAXIMUM_CONTRAIL_INSTANCES;
		instance_index++)
	{
		short contrail_point_count = 0;
		long contrail_point_index = contrail->first_contrail_point_indices[instance_index];

		while (contrail_point_index != NONE)
		{
			contrail_point_index = contrail_point_get(contrail_point_index)->next_contrail_point_index;
			contrail_point_count++;
		}

		match_vassert(
			"c:\\halo\\SOURCE\\effects\\contrails.c",
			677,
			contrail_point_count == contrail->contrail_point_counts[instance_index],
			csprintf(
				temporary,
				"contrail %s thinks it has %d points but really it has %d",
				tag_get_name(contrail->definition_index),
				contrail->contrail_point_counts[instance_index],
				contrail_point_count));
	}

	return;
}

static void contrail_update_points(
	long contrail_index,
	real dt)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
	long contrail_point_indices[MAXIMUM_CONTRAIL_POINTS];
	short instance_index;

	for (instance_index = 0;
		instance_index < MAXIMUM_CONTRAIL_INSTANCES;
		instance_index++)
	{
		short contrail_point_count = 0;
		long contrail_point_index = contrail->first_contrail_point_indices[instance_index];

		while (contrail_point_index != NONE)
		{
			struct contrail_point_datum *contrail_point = contrail_point_get(contrail_point_index);

			if (!TEST_FLAG(contrail_point->flags, _contrail_point_expired_bit))
			{
				contrail_point->time = dt * contrail_point->delta + contrail_point->time;

				while (contrail_point->delta == 0.0f || contrail_point->time > 1.0f)
				{
					if (TEST_FLAG(contrail_point->flags, _contrail_point_transitioning_bit))
					{
						struct contrail_point_state *state = TAG_BLOCK_GET_ELEMENT(
							&definition->states,
							contrail_point->state_index + 1,
							struct contrail_point_state);

						contrail_point->state_index++;
						contrail_point->time = 0.0f;
						contrail_point->delta = contrail_scale_random_value(
							contrail_point->density,
							state->duration_lower_bound,
							state->duration_upper_bound,
							state->scale_flags,
							_contrail_state_duration_bit);
						if (contrail_point->delta != 0.0f)
							contrail_point->delta = 1.0f / contrail_point->delta;

						SET_FLAG(contrail_point->flags, _contrail_point_transitioning_bit, FALSE);
					}
					else if (contrail_point->state_index + 1 < definition->states.count)
					{
						struct contrail_point_state *state = TAG_BLOCK_GET_ELEMENT(
							&definition->states,
							contrail_point->state_index,
							struct contrail_point_state);

						contrail_point->time = 0.0f;
						contrail_point->delta = contrail_scale_random_value(
							contrail_point->density,
							state->transition_duration_lower_bound,
							state->transition_duration_upper_bound,
							state->scale_flags,
							_contrail_state_transition_duration_bit);
						if (contrail_point->delta != 0.0f)
							contrail_point->delta = 1.0f / contrail_point->delta;

						SET_FLAG(contrail_point->flags, _contrail_point_transitioning_bit, TRUE);
					}
					else
					{
						SET_FLAG(contrail_point->flags, _contrail_point_expired_bit, TRUE);
						break;
					}
				}
			}

			if (TEST_FLAG(contrail_point->flags, _contrail_point_new_bit))
			{
				SET_FLAG(contrail_point->flags, _contrail_point_new_bit, FALSE);
			}
			else if (!TEST_FLAG(contrail_point->flags, _contrail_point_expired_bit))
			{
				struct contrail_point_state *state = TAG_BLOCK_GET_ELEMENT(
					&definition->states,
					contrail_point->state_index,
					struct contrail_point_state);

				if (state->physics.index != NONE)
				{
					point_physics_update(
						0,
						point_physics_definition_get(state->physics.index),
						&contrail_point->location,
						NONE,
						&contrail_point->position,
						&contrail_point->velocity,
						NULL,
						NULL,
						NULL,
						state->width * 0.5f,
						dt);
				}
			}

			contrail_point_indices[contrail_point_count++] = contrail_point_index;
			contrail_point_index = contrail_point->next_contrail_point_index;
		}

		while (contrail_point_count > 1)
		{
			struct contrail_point_datum *contrail_point;
			struct contrail_point_datum *previous_contrail_point;

			contrail_point_count--;
			contrail_point = contrail_point_get(contrail_point_indices[contrail_point_count]);
			previous_contrail_point = contrail_point_get(
				contrail_point_indices[contrail_point_count - 1]);

			if (!TEST_FLAG(contrail_point->flags, _contrail_point_expired_bit) ||
				!TEST_FLAG(previous_contrail_point->flags, _contrail_point_expired_bit) ||
				contrail_point->next_contrail_point_index != NONE)
			{
				break;
			}

			previous_contrail_point->next_contrail_point_index = NONE;
			contrail->contrail_point_counts[instance_index]--;
			datum_delete(contrail_point_data, contrail_point_indices[contrail_point_count]);
		}

		if (contrail->contrail_point_counts[instance_index] == 1)
		{
			if (TEST_FLAG(
				contrail_point_get(
					contrail->first_contrail_point_indices[instance_index])->flags,
				_contrail_point_expired_bit))
			{
				datum_delete(
					contrail_point_data,
					contrail->first_contrail_point_indices[instance_index]);
				contrail->first_contrail_point_indices[instance_index] = NONE;
				contrail->contrail_point_counts[instance_index] = 0;
			}
		}
	}

	return;
}

static void contrail_next_frame(
	struct contrail_datum *contrail)
{
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
	struct bitmap_group *bitmap = bitmap_group_get(definition->bitmap.index);
	short sequence_index = contrail->sequence_index;

	contrail->frame_time = 0.0f;
	contrail->frame_index++;

	if (sequence_index < 0 ||
		sequence_index >= bitmap->sequences.count ||
		contrail->frame_index < 0 ||
		contrail->frame_index >= TAG_BLOCK_GET_ELEMENT(
			&bitmap->sequences,
			sequence_index,
			struct bitmap_group_sequence)->bitmap_count)
	{
		contrail->sequence_index = local_random_range(
			definition->first_sequence_index,
			definition->sequence_count + definition->first_sequence_index);
		contrail->frame_index = 0;
	}

	return;
}

static short contrail_compute_new_point_count(
	long contrail_index,
	real dt)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);
	real point_generation_rate = definition->point_generation_rate;
	real time_between_points;
	short contrail_point_count = 0;

	if (TEST_FLAG(definition->scale_flags, _contrail_scales_point_generation_rate_bit))
		point_generation_rate = point_generation_rate * contrail->density;

	time_between_points = 1.0f / point_generation_rate;

	while (dt != 0.0f)
	{
		if (dt >= contrail->time_until_point)
		{
			dt = dt - contrail->time_until_point;
			contrail->time_until_point = time_between_points;
			contrail_point_count++;
		}
		else
		{
			contrail->time_until_point = contrail->time_until_point - dt;
			break;
		}
	}

	return contrail_point_count;
}

static void contrail_add_points(
	long contrail_index,
	short contrail_point_count,
	boolean force)
{
	struct contrail_datum *contrail = contrail_get(contrail_index);
	struct contrail_definition *definition = contrail_definition_get(contrail->definition_index);

	if (contrail_point_count != 0)
	{
		struct object_datum *object = object_get(contrail->object_index);
		struct object_marker markers[MAXIMUM_CONTRAIL_INSTANCES];
		short marker_count = object_get_marker_by_name(
			contrail->object_index,
			TAG_BLOCK_GET_ELEMENT(
				&object_definition_get(object->definition_index)->object.attachments,
				contrail->attachment_index,
				struct object_attachment_definition)->marker_name,
			markers,
			NUMBEROF(markers));

		if (marker_count > 0)
		{
			real point_velocity = contrail_scale_random_value(
				contrail->density,
				definition->point_velocity_lower_bound,
				definition->point_velocity_upper_bound,
				definition->scale_flags,
				_contrail_scales_point_velocity_bit);
			real point_velocity_cone_angle = contrail_scale_value(
				contrail->density,
				definition->point_velocity_cone_angle,
				definition->scale_flags,
				_contrail_scales_point_velocity_cone_angle_bit);
			real point_inherited_velocity_fraction = contrail_scale_value(
				contrail->density,
				definition->point_inherited_velocity_fraction,
				definition->scale_flags,
				_contrail_scales_inherited_velocity_fraction_bit);
			short marker_index;

			for (marker_index = 0; marker_index < marker_count; marker_index++)
			{
				struct object_marker *marker = &markers[marker_index];
				struct contrail_point_datum *previous_contrail_point;
				short new_contrail_point_count;
				short contrail_point_index;

				if (contrail->first_contrail_point_indices[marker_index] != NONE)
				{
					previous_contrail_point = contrail_point_get(
						contrail->first_contrail_point_indices[marker_index]);
					new_contrail_point_count = contrail_point_count;
				}
				else
				{
					previous_contrail_point = NULL;
					new_contrail_point_count = 1;
				}

				if (previous_contrail_point &&
					!csmemcmp(
						&marker->matrix.position,
						&previous_contrail_point->position,
						sizeof(real_point3d)) &&
					!force)
				{
					continue;
				}

				for (contrail_point_index = 1;
					contrail_point_index <= new_contrail_point_count;
					contrail_point_index++)
				{
					long new_contrail_point_index = datum_new(contrail_point_data);

					if (new_contrail_point_index != NONE)
					{
						struct contrail_point_datum *contrail_point =
							contrail_point_get(new_contrail_point_index);
						real_vector3d direction;
						real_vector3d object_velocity;

						contrail_point->time = 0.0f;
						contrail_point->delta = 0.0f;
						contrail_point->flags = FLAG(_contrail_point_new_bit) |
							FLAG(_contrail_point_transitioning_bit);
						contrail_point->state_index = NONE;
						contrail_point->density = contrail->density;
						local_random_vector_in_cone3d(
							&marker->matrix.forward,
							0.0f,
							point_velocity_cone_angle,
							&direction);
						contrail_point->position = marker->matrix.position;
						scenario_location_from_point(
							&contrail_point->location,
							&contrail_point->position);
						object_get_velocities(contrail->object_index, &object_velocity, NULL);
						contrail_point->velocity.i = point_inherited_velocity_fraction *
							object_velocity.i + direction.i * point_velocity;
						contrail_point->velocity.j = point_inherited_velocity_fraction *
							object_velocity.j + direction.j * point_velocity;
						contrail_point->velocity.k = point_inherited_velocity_fraction *
							object_velocity.k + direction.k * point_velocity;

						if (contrail_point_index < new_contrail_point_count)
						{
							real t = (real)contrail_point_index / new_contrail_point_count;
							real one_minus_t = 1.0f - t;
							real_point3d interpolated_position;

							match_assert_valid_real(
								"c:\\halo\\SOURCE\\effects\\contrails.c",
								471,
								t);

							contrail_point->density = one_minus_t * previous_contrail_point->density +
								t * contrail_point->density;

							interpolated_position.x = one_minus_t * previous_contrail_point->position.x +
								t * contrail_point->position.x;
							interpolated_position.y = one_minus_t * previous_contrail_point->position.y +
								t * contrail_point->position.y;
							interpolated_position.z = one_minus_t * previous_contrail_point->position.z +
								t * contrail_point->position.z;
							scenario_location_from_line(
								&contrail_point->location,
								&contrail_point->location,
								&contrail_point->position,
								&interpolated_position);
							contrail_point->position = interpolated_position;

							contrail_point->velocity.i = one_minus_t * previous_contrail_point->velocity.i +
								t * contrail_point->velocity.i;
							contrail_point->velocity.j = one_minus_t * previous_contrail_point->velocity.j +
								t * contrail_point->velocity.j;
							contrail_point->velocity.k = one_minus_t * previous_contrail_point->velocity.k +
								t * contrail_point->velocity.k;
						}

						contrail_point->next_contrail_point_index =
							contrail->first_contrail_point_indices[marker_index];
						contrail->contrail_point_counts[marker_index]++;
						contrail->first_contrail_point_indices[marker_index] = new_contrail_point_index;
					}
				}
			}
		}
	}

	return;
}

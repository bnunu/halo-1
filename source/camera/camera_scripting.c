/*
CAMERA_SCRIPTING.C

symbols in this file:
00073650 0020:
	_scripted_camera_enable (0000)
00073670 00d0:
	_scripted_camera_set_animation (0000)
00073740 0040:
	_scripted_camera_set_first_person (0000)
00073780 0040:
	_scripted_camera_set_dead (0000)
000737C0 0030:
	_scripted_camera_object_is_first_person_camera (0000)
000737F0 00e0:
	_scripted_camera_set (0000)
000738D0 0020:
	_scripted_camera_set_absolute (0000)
000738F0 00d0:
	_scripted_camera_set_camera_point_relative (0000)
000739C0 0030:
	_scripted_camera_set_camera_point_absolute (0000)
000739F0 0010:
	_scripted_camera_next_camera_point (0000)
00073A00 0010:
	_scripted_camera_object_relative_to (0000)
00073A10 0020:
	_scripted_camera_time (0000)
00073A30 0680:
	_scripted_camera_update (0000)
00256A7C 003d:
	??_C@_0DN@JNLEKBID@cannot?5set?5first?5person?5camera?5o@ (0000)
00256ABC 0029:
	??_C@_0CJ@GLCFCHKA@c?3?2halo?2SOURCE?2camera?2camera_scr@ (0000)
002DCB60 0040:
	_camera_script_globals (0000)
*/

/* ---------- headers */

#include "camera_scripting.h"
#include "director.h"

#include "cseries/cseries.h"
#include "cseries/errors.h"
#include "game/game.h"
#include "math/real_math.h"
#include "models/model_animation_definitions.h"
#include "objects/objects.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	_camera_script_mode_point = 0,
	_camera_script_mode_animation,
	_camera_script_mode_first_person,
	_camera_script_mode_dead,
	NUMBER_OF_CAMERA_SCRIPT_MODES,
};

/* ---------- macros */

/* ---------- structures */

struct camera_control
{
	long unknown0;
	real seconds_elapsed;
};

struct dead_camera
{
	real_point3d position;
	real_euler_angles2d facing;
	real distance;
	real field_of_view;
	real timer;
	long player_index;
	long current_player_index;
	long unit_index;
	real switch_timer;
};

struct scripted_camera_command
{
	long flags;
	real_point3d position;
	real_vector3d offset;
	real depth;
	real field_of_view;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d velocity;
	real timer;
	byte position_flags;
	byte offset_flags;
	byte distance_flags;
	byte field_of_view_flags;
	byte orientation_flags;
	byte pad51[3];
	real position_timer;
	real offset_timer;
	real distance_timer;
	real field_of_view_timer;
	real orientation_timer;
};

struct scripted_camera_globals
{
	boolean enabled;
	boolean first_update;
	short mode;
	short camera_point_index;
	byte pad06[2];
	real timer;
	real_point3d point;
	real_vector3d forward;
	real_vector3d up;
	real field_of_view;
	long relative_object_index;
	long animation_graph_index;
	short animation_index;
};

typedef char scripted_camera_command_size_assert[
	sizeof(struct scripted_camera_command) == 0x68 ? 1 : -1];
typedef char scripted_camera_globals_size_assert[
	sizeof(struct scripted_camera_globals) == 0x40 ? 1 : -1];

/* ---------- prototypes */

void scripted_camera_set(
	short camera_point_index,
	word transition_time,
	long relative_object_index);
void first_person_camera_fake(
	long unit_index,
	struct scripted_camera_command *result);
void dead_camera_new(
	struct dead_camera *camera,
	short local_player_index,
	long unit_index);
void dead_camera_update(
	struct dead_camera *camera,
	struct camera_control const *controls,
	struct scripted_camera_command *result);

/* ---------- globals */

/* name from the 2003 PC demo PDB and the HCEX PDB (camera_scripting file static); January's 64 bytes are
 * identical to the demo's and it has no public for it (static) */
static struct scripted_camera_globals camera_script_globals =
{
	FALSE,
	FALSE,
	NONE,
	NONE,
	{ 0, 0 },
	0.f,
	{ { 0.f, 0.f, 0.f } },
	{ { 0.f, 0.f, 1.f } },
	{ { 0.f, 1.f, 0.f } },
	1.22173047f,
	NONE,
	NONE,
	0
};

/* ---------- public code */

void scripted_camera_enable(
	boolean enabled)
{
	camera_script_globals.enabled = enabled;
	sound_listener_motion_reset(NONE); /* port: entering or leaving a scripted camera is a cut */
	render_interpolation_cut(); /* port: ... drawn as one (render_interpolation.c) */
	camera_script_globals.first_update = TRUE;
	return;
}

void scripted_camera_set_animation(
	long animation_graph_index,
	char const *animation_name)
{
	struct animation_graph *animation_graph;
	struct animation *animation;
	short animation_index;

	if (animation_graph_index != NONE)
	{
		animation_graph = animation_graph_definition_get(animation_graph_index);
		if (animation_graph->nodes.count == 1)
		{
			animation_index = 0;
			while (animation_index < animation_graph->animations.count)
			{
				animation = TAG_BLOCK_GET_ELEMENT(
					&animation_graph->animations,
					animation_index,
					struct animation);
				if (!_stricmp(animation_name, animation->name))
				{
					camera_script_globals.camera_point_index = NONE;
					camera_script_globals.relative_object_index = NONE;
					camera_script_globals.mode = _camera_script_mode_animation;
					sound_listener_motion_reset(NONE); /* port: a new track has no preceding motion sample */
					render_interpolation_cut(); /* port: nor does its first frame blend from the last shot */
					camera_script_globals.first_update = TRUE;
					camera_script_globals.animation_index = animation_index;
					camera_script_globals.animation_graph_index = animation_graph_index;
					camera_script_globals.field_of_view = 1.22173047f;
					camera_script_globals.timer =
						(real)(animation->frame_count / TICKS_PER_SECOND);
					break;
				}

				animation_index++;
			}
		}
	}

	return;
}

void scripted_camera_set_first_person(
	long unit_index)
{
	if (unit_index != NONE)
	{
		camera_script_globals.mode = _camera_script_mode_first_person;
		sound_listener_motion_reset(NONE); /* port: camera attachment changes discontinuously */
		render_interpolation_cut(); /* port: a cut (render_interpolation.c) */
		camera_script_globals.first_update = TRUE;
		camera_script_globals.relative_object_index = unit_index;
	}
	else
	{
		error(
			_error_silent,
			"cannot set first person camera on a unit that doesn't exist.");
	}

	return;
}

void scripted_camera_set_dead(
	long unit_index)
{
	if (unit_index != NONE)
	{
		camera_script_globals.mode = _camera_script_mode_dead;
		sound_listener_motion_reset(NONE); /* port: camera attachment changes discontinuously */
		render_interpolation_cut(); /* port: a cut (render_interpolation.c) */
		camera_script_globals.first_update = TRUE;
		camera_script_globals.relative_object_index = unit_index;
	}
	else
	{
		error(
			_error_silent,
			"cannot set first person camera on a unit that doesn't exist.");
	}

	return;
}

boolean scripted_camera_object_is_first_person_camera(
	long object_index)
{
	return camera_script_globals.enabled &&
		camera_script_globals.mode == _camera_script_mode_first_person &&
		camera_script_globals.relative_object_index == object_index;
}

void scripted_camera_set(
	short camera_point_index,
	word transition_time,
	long relative_object_index)
{
	struct scenario_cutscene_camera_point *camera_point;
	long camera_time;

	camera_point = TAG_BLOCK_GET_ELEMENT(
		&global_scenario_get()->cutscene_camera_points,
		camera_point_index,
		struct scenario_cutscene_camera_point);
	camera_time = (short)transition_time / TICKS_PER_SECOND;
	/* port: short cuts also need an explicit reset, before the observer moves */
	if (camera_time <= 0)
		sound_listener_motion_reset(NONE);
	/* port: and are a new shot, drawn as one (render_interpolation.c), unless
	the camera is already there */
	if (camera_time <= 0 &&
		(camera_script_globals.timer > 0.f ||
		camera_script_globals.mode != _camera_script_mode_point ||
		camera_script_globals.camera_point_index != camera_point_index ||
		camera_script_globals.relative_object_index != relative_object_index))
	{
		render_interpolation_cut();
	}
	camera_script_globals.mode = _camera_script_mode_point;
	camera_script_globals.first_update = TRUE;
	camera_script_globals.camera_point_index = camera_point_index;
	camera_script_globals.point = camera_point->position;
	vectors3d_from_euler_angles3d(
		&camera_script_globals.forward,
		&camera_script_globals.up,
		&camera_point->orientation);
	if (camera_point->field_of_view != 0.f)
		camera_script_globals.field_of_view = camera_point->field_of_view;
	else
		camera_script_globals.field_of_view = 1.22173047f;
	camera_script_globals.relative_object_index = relative_object_index;
	camera_script_globals.timer = (real)camera_time;

	director_update(0.f);
	observer_update(0.0001f);
	return;
}

void scripted_camera_set_absolute(
	short camera_point_index,
	word transition_time)
{
	scripted_camera_set(camera_point_index, transition_time, NONE);
	return;
}

void scripted_camera_set_camera_point_relative(
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real field_of_view,
	word transition_time,
	long relative_object_index)
{
	/* port: co-op applies this point every tick with zero transition time;
	keep listener motion history until an actual camera or lifecycle change */
	camera_script_globals.mode = _camera_script_mode_point;
	camera_script_globals.camera_point_index = NONE;
	camera_script_globals.point = *position;
	camera_script_globals.forward = *forward;
	camera_script_globals.up = *up;
	if (field_of_view != 0.f)
		camera_script_globals.field_of_view = field_of_view;
	else
		camera_script_globals.field_of_view = 1.22173047f;
	camera_script_globals.timer =
		(real)((short)transition_time / TICKS_PER_SECOND);
	camera_script_globals.relative_object_index = relative_object_index;

	director_update(0.f);
	observer_update(0.0001f);
	return;
}

void scripted_camera_set_camera_point_absolute(
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real field_of_view,
	word transition_time)
{
	scripted_camera_set_camera_point_relative(
		position,
		forward,
		up,
		field_of_view,
		transition_time,
		NONE);
	return;
}

short scripted_camera_next_camera_point(
	void)
{
	return camera_script_globals.camera_point_index;
}

long scripted_camera_object_relative_to(
	void)
{
	return camera_script_globals.relative_object_index;
}

short scripted_camera_time(
	void)
{
	return (short)(camera_script_globals.timer * 30.f);
}

void scripted_camera_update(
	struct dead_camera *camera,
	struct camera_control const *controls,
	struct scripted_camera_command *result)
{
	real_point3d focus_position;
	real speed;

	focus_position = *global_origin3d;
	speed = game_time_get_speed();
	result->flags = FLAG(3);
	if (game_time_get_paused())
	{
		result->flags |= FLAG(5);
	}
	else
	{
		result->flags &= ~FLAG(5);
	}

	switch (camera_script_globals.mode)
	{
	case _camera_script_mode_point:
		if (camera_script_globals.relative_object_index != NONE)
		{
			struct object_datum *object;

			object = object_try_and_get_and_verify_type(
				camera_script_globals.relative_object_index,
				_object_mask_all);
			if (!object)
			{
				break;
			}
			focus_position = object->object.bounding_sphere_center;
		}

		result->timer = speed != 0.f
			? camera_script_globals.timer / speed
			: 0.f;
		result->field_of_view = camera_script_globals.field_of_view;
		result->forward = camera_script_globals.forward;
		result->up = camera_script_globals.up;
		if (camera_script_globals.relative_object_index != NONE)
		{
			real angle;
			real cosine_value;
			real dot;
			real rotated_offset_i;
			real sine_value;
			real_vector3d offset;

			angle = arctangent(result->forward.j, result->forward.i);
			dot = dot_product3d(
				(real_vector3d const *)&camera_script_globals.point,
				&result->forward);
			if (dot > 0.f)
			{
				dot = 0.f;
			}

			result->depth = -dot;
			result->position = focus_position;
			offset.i = camera_script_globals.point.x - dot * result->forward.i;
			offset.j = camera_script_globals.point.y - dot * result->forward.j;
			offset.k = camera_script_globals.point.z - dot * result->forward.k;
			result->position_timer = 0.f;
			result->position_flags = 1;
			sine_value = sine(angle);
			cosine_value = cosine(angle);
			rotated_offset_i = offset.i * cosine_value;
			rotated_offset_i += sine_value * offset.j;
			result->offset.i = rotated_offset_i;
			result->offset.j = sine_value * offset.i - cosine_value * offset.j;
			result->offset.k = offset.k;
			result->flags |= FLAG(0);
		}
		else
		{
			result->position = camera_script_globals.point;
			result->flags |= FLAG(0);
		}
		break;

	case _camera_script_mode_animation:
		{
			struct animation_graph *animation_graph;
			struct animation *animation;
			short frame;
			long frame_count;
			real_matrix4x3 root_matrix;

			animation_graph = animation_graph_definition_get(
				camera_script_globals.animation_graph_index);
			animation = TAG_BLOCK_GET_ELEMENT(
				&animation_graph->animations,
				camera_script_globals.animation_index,
				struct animation);
			frame_count = animation->frame_count;
			frame = (short)(frame_count - camera_script_globals.timer * 30.f);
			animation_get_root_matrix(
				NULL,
				animation,
				frame < 0 ? 0 : MIN(frame, frame_count - 1),
				&root_matrix);
			result->forward = root_matrix.forward;
			result->up = root_matrix.up;
			result->field_of_view = DEGREES_TO_RADIANS(70.f);
			result->position = root_matrix.position;
			result->depth = 0.f;
			result->timer = 0.f;
			result->flags |= FLAG(0);
		}
		break;

	case _camera_script_mode_first_person:
		if (object_try_and_get_and_verify_type(
			camera_script_globals.relative_object_index,
			_object_mask_unit))
		{
			first_person_camera_fake(
				camera_script_globals.relative_object_index,
				result);
		}
		break;

	case _camera_script_mode_dead:
		if (object_try_and_get_and_verify_type(
			camera_script_globals.relative_object_index,
			_object_mask_unit))
		{
			if (camera_script_globals.first_update)
			{
				dead_camera_new(
					camera,
					(short)controls->unknown0,
					camera_script_globals.relative_object_index);
			}
			dead_camera_update(camera, controls, result);
		}
		break;
	}

	camera_script_globals.timer = MAX(
		0.f,
		camera_script_globals.timer - speed * controls->seconds_elapsed);
	camera_script_globals.first_update = FALSE;

	match_vassert(
		"c:\\halo\\SOURCE\\camera\\camera_scripting.c",
		0x172,
		!(result->flags & FLAG(0)) ||
		(valid_real_vector3d_axes2(&result->forward, &result->up) &&
			valid_real(result->position.x) && result->position.x>=-5000.f && result->position.x<=5000.f &&
			valid_real(result->position.y) && result->position.y>=-5000.f && result->position.y<=5000.f &&
			valid_real(result->position.z) && result->position.z>=-5000.f && result->position.z<=5000.f &&
			valid_real(result->offset.i) && result->offset.i>=-5000.f && result->offset.i<=5000.f &&
			valid_real(result->offset.j) && result->offset.j>=-5000.f && result->offset.j<=5000.f &&
			valid_real(result->offset.k) && result->offset.k>=-5000.f && result->offset.k<=5000.f &&
			valid_real_vector3d(&result->velocity) &&
			valid_real(result->depth) && result->depth>=0.f && result->depth<=5000.f &&
			valid_real(result->field_of_view) && result->field_of_view>=0.001f && result->field_of_view<=_pi / 2.f &&
			valid_real(result->timer) && result->timer>=0.f && result->timer<=3600.f),
		csprintf(
			temporary,
			"Invalid camera command.\nF: (%f, %f, %f) U: (%f, %f, %f)\nP: (%f, %f, %f) O: (%f, %f, %f)\nD: %f V: (%f, %f, %f), FOV: %f, T: %f, FL: %ld",
			result->forward.i,
			result->forward.j,
			result->forward.k,
			result->up.i,
			result->up.j,
			result->up.k,
			result->position.x,
			result->position.y,
			result->position.z,
			result->offset.i,
			result->offset.j,
			result->offset.k,
			result->depth,
			result->velocity.i,
			result->velocity.j,
			result->velocity.k,
			result->field_of_view,
			result->timer,
			result->flags));

	return;
}

/* ---------- private code */

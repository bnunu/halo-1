/*
DIRECTOR.C

symbols in this file:
000747D0 0040:
	_director_get (0000)
00074810 0020:
	_director_initialize (0000)
00074830 0010:
	_director_dispose (0000)
00074840 0050:
	_director_inhibit_facing (0000)
00074890 0050:
	_director_inhibit_input (0000)
000748E0 0050:
	_director_inhibited_facing (0000)
00074930 0050:
	_director_inhibited_input (0000)
00074980 0050:
	_director_set_mode (0000)
000749D0 00b0:
	_director_save_camera (0000)
00074A80 00a0:
	_director_get_perspective (0000)
00074B20 00f0:
	_director_desired_perspective (0000)
00074C10 0060:
	_director_set_camera (0000)
00074C70 0070:
	_director_initialize_variables (0000)
00074CE0 0220:
	_director_process_variables (0000)
00074F00 0070:
	_director_dispose_from_old_map (0000)
00074F70 0150:
	_director_load_camera (0000)
000750C0 0130:
	_director_rotate_cameras (0000)
000751F0 0060:
	_director_camera_deterministic (0000)
00075250 00d0:
	_director_choose_game_perspective (0000)
00075320 0130:
	_director_script_camera (0000)
00075450 01c0:
	_director_choose_camera_game (0000)
00075610 00b0:
	_director_choose_camera_editor (0000)
000756C0 00c0:
	_director_choose_camera_script_camera_record (0000)
00075780 03c0:
	_director_update_controls (0000)
00075B40 0050:
	_director_choose_camera (0000)
00075B90 00d0:
	_director_initialize_for_new_map (0000)
00075C60 01f0:
	_director_update (0000)
00075E50 0020:
	_director_initialize_for_saved_game (0000)
00256B18 0008:
	_director_game_camera_modes (0000)
00256B20 0008:
	_director_script_camera_record_camera_modes (0000)
00256B28 0004:
	_ticks_per_millisecond (0000)
00256B2C 0004:
	_friction (0000)
00256B30 0004:
	_acceleration_scale (0000)
00256B34 0004:
	_genius_boy (0000)
00256B38 000d:
	??_C@_0N@PCFDPHGK@first?5person?$AA@ (0000)
00256B48 0007:
	??_C@_06GIFLLBB@editor?$AA@ (0000)
00256B50 0007:
	??_C@_06CPNKLHAC@flying?$AA@ (0000)
00256B58 0009:
	??_C@_08GFACNFFL@orbiting?$AA@ (0000)
00256B64 000a:
	??_C@_09BEIPBMDN@following?$AA@ (0000)
00256B70 004c:
	??_C@_0EM@JCKEKCNI@local_player_index?$DO?$DN0?5?$CG?$CG?5local_p@ (0000)
00256BBC 0021:
	??_C@_0CB@MLDELAOE@c?3?2halo?2SOURCE?2camera?2director?4c@ (0000)
00256BE0 0013:
	??_C@_0BD@CNMFAHOJ@director?5scripting?$AA@ (0000)
00256BF4 002e:
	??_C@_0CO@JGFKNOPP@mode?$DO?$DN0?5?$CG?$CG?5mode?$DMNUMBER_OF_DIRECT@ (0000)
00256C24 0004:
	??_C@_03PPOCCAPH@?$CFf?6?$AA@ (0000)
00256C28 000a:
	??_C@_09MALOBAAJ@?$CFf?5?$CFf?5?$CFf?6?$AA@ (0000)
00256C34 000e:
	??_C@_0O@EOBEHDJM@d?3?2camera?4txt?$AA@ (0000)
00256C44 000a:
	??_C@_09LKGGBKMJ@?$CFs?5camera?$AA@ (0000)
00256C50 0004:
	__real@bb4de32e (0000)
00256C54 0004:
	__real@b851b717 (0000)
00256C58 0004:
	__real@3851b717 (0000)
00256C5C 0004:
	__real@37a4b5be (0000)
00256C60 0004:
	__real@b824b5be (0000)
002DCBA0 0018:
	_director_camera_mode_names (0000)
002DCBB8 0070:
	_variables (0000)
0031D048 03ea:
	_director_globals (0000)
	_director_camera_switch_fast (03e8)
*/

/* ---------- headers */

#include "controller_remap.h" /* port: port/linux/game/controller_remap_game.c */
#include "director.h"

#include "camera_scripting.h"
#include "dead_camera.h"
#include "coop_spectate.h" /* port: port/linux/game/coop_spectate.c */
#include "editor_flying_camera.h"
#include "first_person_camera.h"
#include "flying_camera.h"
#include "following_camera.h"
#include "passenger_camera.h" /* port: port/linux/game/passenger_camera.c */
#include "observer.h"
#include "orbiting_camera.h"

#include "editor/editor_stubs.h"
#include "game/players.h"
#include "input/input.h"
#include "main/console.h"
#include "saved games/game_state.h"

#include "units/unit_definitions.h"
#include "units/units.h"

/* port: the player's movement and turn before the director's inhibit zeroes
them (player_control.c), which move the debug camera */
extern real_vector2d player_control_port_debug_throttle[];
extern real_euler_angles2d player_control_port_debug_facing[];

/* network_game_globals.c's */
boolean network_game_distributed_client(void);

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct camera_control
{
	short local_player_index;
	boolean active;
	byte pad3;
	real seconds_elapsed;
	real_euler_angles3d facing_delta;
	real_vector3d position_delta;
	real wheel_delta;
};

struct mouse_state
{
	long x;
	long y;
	long wheel_delta;
	byte buttons[2];
};

enum camera_control_flags
{
	_camera_control_forward_bit = 0,
	_camera_control_reverse_bit,
	_camera_control_left_bit,
	_camera_control_right_bit,
	_camera_control_up_bit,
	_camera_control_down_bit,
	_camera_control_roll_left_bit,
	_camera_control_roll_right_bit
};

static void director_process_variables(
	short local_player_index,
	unsigned long control_flags,
	real speed_delta);
static boolean director_update_controls(
	short local_player_index,
	struct camera_control *controls);

/* ---------- globals */

static char const *director_camera_mode_names[NUMBER_OF_DIRECTOR_CAMERA_MODES] =
{
	"following",
	"orbiting",
	"flying",
	"editor",
	"first person"
};

static short const director_game_camera_modes[3] =
{
	_camera_first_person,
	_camera_flying,
	_camera_following
};

static short const director_script_camera_record_camera_modes[4] =
{
	_camera_first_person,
	_camera_flying,
	_camera_following,
	_camera_orbiting
};

static real const ticks_per_millisecond = 0.03f;
static real const friction = 5.f;
static real const acceleration_scale = 25.f;
static real const genius_boy = 1.3f;

static struct director_variable_definition variables[NUMBER_OF_DIRECTOR_VARIABLES] =
{
	{ 5, 4, NONE, { 0, 0 }, 0.15f, 0.f, -REAL_MAX, REAL_MAX, TRUE, { 0, 0, 0 } },
	{ 6, 7, NONE, { 0, 0 }, 0.075f, 0.f, -REAL_MAX, REAL_MAX, FALSE, { 0, 0, 0 } },
	{ 1, 0, NONE, { 0, 0 }, 0.075f, 0.f, -REAL_MAX, REAL_MAX, TRUE, { 0, 0, 0 } },
	{ 3, 2, NONE, { 0, 0 }, 0.075f, 0.f, -REAL_MAX, REAL_MAX, TRUE, { 0, 0, 0 } }
};

static struct director_globals director_globals = {0};
boolean director_camera_switch_fast = FALSE;
static boolean hyper_key_down = FALSE;
boolean *director_camera_scripted;

/* ---------- public code */

static struct director *director_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\camera\\director.c", 179, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	return &director_globals.local_players[local_player_index];
}

void director_initialize(
	void)
{
	director_camera_scripted = game_state_malloc("director scripting", NULL, 4);
	*director_camera_scripted = FALSE;
	return;
}

void director_dispose(
	void)
{
	return;
}

void director_dispose_from_old_map(
	void)
{
	short local_player_index;

	controller_remap_reset(); /* port: release held actions and camera context between maps */

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		struct director *director = director_get(local_player_index);

		director->camera_proc = NULL;
		director->debug_input_scale = 1.f;
		director->debug_controls = FALSE;
	}

	*director_camera_scripted = FALSE;
	return;
}

void director_inhibit_facing(
	short local_player_index)
{
	director_get(local_player_index)->inhibited_facing = TRUE;
	return;
}

void director_inhibit_input(
	short local_player_index)
{
	director_get(local_player_index)->inhibited_input = TRUE;
	return;
}

boolean director_inhibited_facing(
	short local_player_index)
{
	return director_get(local_player_index)->inhibited_facing;
}

boolean director_inhibited_input(
	short local_player_index)
{
	return director_get(local_player_index)->inhibited_input;
}

director_perspective director_get_perspective(
	short local_player_index)
{
	struct director *camera;

	camera = director_get(local_player_index);

	if (camera->camera_proc == first_person_camera_update)
	{
		if (camera->camera_change_pause == 0.f)
			camera->perspective = 0;
	}
	else if (camera->camera_proc == following_camera_update)
	{
		camera->perspective = 1;
	}
	else if (camera->camera_proc == (director_camera_update_proc)scripted_camera_update)
	{
		camera->perspective = 2;
	}
	else
	{
		camera->perspective = 3;
	}

	return camera->perspective;
}

short director_desired_perspective(
	long unit_index,
	director_perspective *perspective)
{
	short following = FALSE;

	*perspective = 0;
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		if (unit->object.parent_object_index != NONE)
		{
			struct object_datum *parent_object = object_get(unit->object.parent_object_index);
			if (TEST_FLAG(_object_mask_unit, parent_object->object.type))
			{
				boolean third_person_on_enter;
				struct unit_seat *seat = TAG_BLOCK_GET_ELEMENT(
					&unit_definition_get(parent_object->definition_index)->unit.seats,
					unit->unit.parent_seat_index,
					struct unit_seat);

				third_person_on_enter = TEST_FLAG(seat->flags, _unit_seat_third_person_on_enter_bit);
				if (TEST_FLAG(seat->flags, _unit_seat_third_person_camera_bit))
					following = TRUE;
				if (third_person_on_enter)
				{
					if (unit->unit.animation.state == _unit_state_entering_seat)
						*perspective = 1;
					else if (unit->unit.animation.state == _unit_state_exiting_seat)
						*perspective = 3;
					else
						*perspective = 2;
				}
				else
				{
					*perspective = 2;
				}
			}
			else
			{
				*perspective = 2;
			}
		}

		if (*perspective == 1 || *perspective == 3)
			following = TRUE;
	}

	return following;
}

void director_set_mode(
	short mode)
{
	match_assert("c:\\halo\\SOURCE\\camera\\director.c", 384, mode>=0 && mode<NUMBER_OF_DIRECTOR_GAME_MODES);
	if (director_globals.game_mode != mode)
	{
		director_globals.game_mode = mode;
		director_globals.initialize_camera = TRUE;
	}

	return;
}

void director_save_camera(
	void)
{
	FILE *file;
	/* port: the first local player's camera, not local player 0's: a game
	started from a controller's menus may have no local player 0 */
	short local_player_index = local_player_get_next(NONE);

	if (local_player_index == NONE)
		return;
	/* port: in the working directory; the Xbox's was on its D: drive */
	file = fopen("camera.txt", "w");
	if (file)
	{
		struct observer_result const *camera;

		camera = observer_get_camera(local_player_index);
		fprintf(file, "%f %f %f\n", camera->position.x, camera->position.y, camera->position.z);
		fprintf(file, "%f %f %f\n", camera->forward.i, camera->forward.j, camera->forward.k);
		fprintf(file, "%f %f %f\n", camera->up.i, camera->up.j, camera->up.k);
		fprintf(file, "%f\n", camera->field_of_view);
		fclose(file);
	}

	return;
}

static void director_set_camera(
	short local_player_index,
	director_camera_update_proc camera_proc,
	boolean interpolate)
{
	struct director *director = director_get(local_player_index);

	director->camera_proc = camera_proc;
	director->debug_input_scale = 1.f;
	director->debug_controls = FALSE;
	if (interpolate)
		director->camera_change_pause = 1.f;

	return;
}

void director_load_camera(
	void)
{
	/* port: the first local player's (director_save_camera) */
	short local_player_index = local_player_get_next(NONE);
	FILE *file;

	if (local_player_index == NONE)
		return;
	/* port: in the working directory (director_save_camera) */
	file = fopen("camera.txt", "r");
	if (file)
	{
		real_point3d position;
		real_vector3d forward;
		real_vector3d derived_up;
		real_vector3d stored_up;
		real_vector3d cross_product;
		real field_of_view;
		struct director *director = director_get(local_player_index);
		struct flying_camera *camera =
			(struct flying_camera *)director->camera_data;

		fscanf(file, "%f %f %f\n", &position.x, &position.y, &position.z);
		fscanf(file, "%f %f %f\n", &forward.i, &forward.j, &forward.k);
		fscanf(file, "%f %f %f\n", &stored_up.i, &stored_up.j, &stored_up.k);
		fscanf(file, "%f\n", &field_of_view);
		fclose(file);

		flying_camera_new_from_point_and_vector(camera, &position, &forward);
		observer_up_from_forward(&forward, &derived_up);
		camera->roll = angle_between_vectors3d(&stored_up, &derived_up);
		cross_product3d(&stored_up, &derived_up, &cross_product);
		if (dot_product3d(&cross_product, &forward) > 0.f)
			camera->roll = -camera->roll;
		camera->field_of_view = field_of_view;
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)flying_camera_update,
			FALSE);
		director->camera_mode_index = _camera_flying;
	}

	return;
}

static void director_initialize_variables(
	short local_player_index)
{
	short variable_index;
	struct director *director = director_get(local_player_index);

	for (variable_index = 0;
		variable_index < NUMBER_OF_DIRECTOR_VARIABLES;
		variable_index++)
	{
		director->debug_variables[variable_index].value = variables[variable_index].initial_value;
		director->debug_variables[variable_index].delta = 0.f;
		director->debug_variables[variable_index].velocity = 0.f;
	}

	return;
}

static void director_process_variables(
	short local_player_index,
	unsigned long control_flags,
	real speed_delta)
{
	short variable_index;
	struct director *director = director_get(local_player_index);

	director->debug_input_scale *=
		(real)pow((double)genius_boy, (double)speed_delta);
	director->debug_input_scale =
		PIN(director->debug_input_scale, 0.01f, 50.f);

	for (variable_index = 0;
		variable_index < NUMBER_OF_DIRECTOR_VARIABLES;
		variable_index++)
	{
		struct director_variable_definition const *definition =
			&variables[variable_index];
		struct director_variable_instance *instance =
			&director->debug_variables[variable_index];
		real hyper_scale = definition->has_hyper_scale
			? director->debug_input_scale
			: 1.f;
		real velocity_scale =
			1.f - PIN(friction * director_globals.dtime, 0.f, 1.f);
		boolean negative = definition->negative_bit != NONE &&
			TEST_FLAG(control_flags, definition->negative_bit);
		boolean positive = definition->positive_bit != NONE &&
			TEST_FLAG(control_flags, definition->positive_bit);
		boolean reset = definition->reset_bit != NONE &&
			TEST_FLAG(control_flags, definition->reset_bit);

		instance->velocity *= velocity_scale;
		if (negative && !positive)
		{
			instance->velocity -=
				definition->scale * director_globals.dtime * hyper_scale * acceleration_scale;
		}
		else if (positive && !negative)
		{
			instance->velocity +=
				definition->scale * director_globals.dtime * hyper_scale * acceleration_scale;
		}
		else if (game_in_editor())
		{
			instance->velocity = 0.f;
		}

		instance->delta = director_globals.dtime * instance->velocity;
		if (reset)
			instance->value = definition->initial_value;
		else
			instance->value += instance->delta;
		instance->value =
			PIN(instance->value, definition->minimum, definition->maximum);
	}

	return;
}

static void director_rotate_cameras(
	short local_player_index,
	short const *camera_modes,
	short camera_mode_count)
{
	short camera_mode;
	struct director *director = director_get(local_player_index);

	director->camera_mode_index = (director->camera_mode_index + 1) % camera_mode_count;
	camera_mode = camera_modes[director->camera_mode_index];
	switch (camera_mode)
	{
	case _camera_following:
		following_camera_new((struct following_camera *)director->camera_data);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)following_camera_update,
			TRUE);
		break;

	case _camera_orbiting:
		orbiting_camera_new(
			(struct orbiting_camera *)director->camera_data,
			director->command.focus_distance,
			&director->command.forward);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)orbiting_camera_update,
			TRUE);
		break;

	case _camera_flying:
		flying_camera_new_from_point_and_vector(
			(struct flying_camera *)director->camera_data,
			&director->command.focus_position,
			&director->command.forward);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)flying_camera_update,
			TRUE);
		break;

	case _camera_editor:
		break;

	case _camera_first_person:
		first_person_camera_new((struct first_person_camera *)director->camera_data);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)first_person_camera_update,
			TRUE);
		break;

	default:
		display_assert(NULL, "c:\\halo\\SOURCE\\camera\\director.c", 512, TRUE);
		system_exit(NONE);
		break;
	}

	console_printf(
		FALSE,
		"%s camera",
		director_camera_mode_names[camera_modes[director->camera_mode_index]]);
	return;
}

static void director_choose_game_perspective(
	short local_player_index,
	boolean force)
{
	/* port: remember only local display choices, never a shared seat flag */
	static boolean passenger_views[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	boolean passenger = passenger_camera_enabled(local_player_index);
	director_perspective perspective;
	short following;
	long unit_index;
	struct director *director = director_get(local_player_index);

	unit_index = player_control_get_unit_index(local_player_index);
	following = director_desired_perspective(unit_index, &perspective);
	/* port: the local passenger view must not change the deterministic
	camera used by player_aim_projectile. Reconsider a menu toggle even
	when the seat's enter/exit state has not changed. */
	if (passenger)
		following = TRUE;
	if (force || director->seat_state != perspective ||
		passenger != passenger_views[local_player_index])
	{
		if (following == TRUE)
		{
			if (force || director->camera_proc == (director_camera_update_proc)first_person_camera_update)
			{
				following_camera_new((struct following_camera *)director->camera_data);
				director_set_camera(
					local_player_index,
					(director_camera_update_proc)following_camera_update,
					!force);
			}
		}
		else if (force || director->camera_proc == (director_camera_update_proc)following_camera_update)
		{
			first_person_camera_new((struct first_person_camera *)director->camera_data);
			director_set_camera(
				local_player_index,
				(director_camera_update_proc)first_person_camera_update,
				!force);
		}

		director->seat_state = perspective;
	}

	/* port: a menu toggle can change the view without changing seats */
	passenger_views[local_player_index] = passenger;
	return;
}

void director_script_camera(
	boolean scripted)
{
	short local_player_index;

	*director_camera_scripted = scripted;
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		director_get(local_player_index);

		if (scripted)
		{
			director_set_camera(
				local_player_index,
				(director_camera_update_proc)scripted_camera_update,
				FALSE);
		}
		else
		{
			director_choose_game_perspective(local_player_index, TRUE);
		}

		scripted_camera_enable(scripted);
	}

	return;
}

static void director_choose_camera_game(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	struct director *director = director_get(local_player_index);

	if (initialize)
	{
		first_person_camera_new((struct first_person_camera *)director->camera_data);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)first_person_camera_update,
			FALSE);
		return;
	}
	else
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_get(player_index);
		/* port: and a network co-op player who hasn't spawned yet, waiting for
		the level's first checkpoint, watches too */
		boolean use_dead_camera = player->unit_index == NONE &&
			(player->statistics.deaths > 0 || players_coop_waiting_to_start(player_index));

		/* (port: not a client in another's game, the host's rules: a flying
		camera would see all of it, and one behind the player round its
		corners) */
		if (key && !network_game_distributed_client())
			director_rotate_cameras(local_player_index, director_game_camera_modes, 3);
		if (!*director_camera_scripted)
		{
			director_choose_game_perspective(local_player_index, initialize);
			if (use_dead_camera)
			{
				/* port: a dead network co-op player watches a living teammate
				(port/linux/game/coop_spectate.c), or its own body if nobody is alive */
				long watched_unit_index = coop_spectating() ? coop_spectate_unit(local_player_index) : NONE;

				if (director->camera_proc != (director_camera_update_proc)dead_camera_update ||
					(watched_unit_index != NONE &&
						((struct dead_camera *)director->camera_data)->unit_index != watched_unit_index))
				{
					dead_camera_new(
						(struct dead_camera *)director->camera_data,
						local_player_index,
						watched_unit_index);
					director_set_camera(
						local_player_index,
						(director_camera_update_proc)dead_camera_update,
						TRUE);
				}
				if (watched_unit_index != NONE)
					coop_spectate_camera((struct dead_camera *)director->camera_data);
			}
			else if (director->camera_proc == (director_camera_update_proc)dead_camera_update)
			{
				director_choose_game_perspective(local_player_index, TRUE);
			}
		}
	}

	return;
}

static void director_choose_camera_editor(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	struct director *director = director_get(local_player_index);

	if (initialize || director->camera_proc != (director_camera_update_proc)editor_camera_update)
	{
		editor_camera_new(
			(struct flying_camera *)director->camera_data,
			local_player_index);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)editor_camera_update,
			FALSE);
	}

	return;
}

static void director_choose_camera_script_camera_record(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	if (initialize)
	{
		struct director *director = director_get(local_player_index);

		first_person_camera_new((struct first_person_camera *)director->camera_data);
		director_set_camera(
			local_player_index,
			(director_camera_update_proc)first_person_camera_update,
			FALSE);
	}
	else if (key)
	{
		director_rotate_cameras(
			local_player_index,
			director_script_camera_record_camera_modes,
			4);
	}

	return;
}

static boolean director_update_controls(
	short local_player_index,
	struct camera_control *controls)
{
	boolean switch_camera = FALSE;
	long player_index;
	/* port: the remapper receives camera availability, never enables it. */
	unsigned int pad_held, pad_pressed;
	int remapped;
	struct director *director = director_get(local_player_index);
	/* port: the debug camera's mouse button and keys (xinput_sdl.c) */
	int middle;
	int key_up;
	int key_down;
	float wheel;

	remapped = controller_remap_camera(local_player_index,
		director->camera_proc == (director_camera_update_proc)flying_camera_update ||
		director->camera_proc == (director_camera_update_proc)editor_camera_update ||
		director->camera_proc == (director_camera_update_proc)orbiting_camera_update,
		director->debug_controls, &pad_held, &pad_pressed);
	csmemset(controls, 0, sizeof(*controls));
	halo_linux_debug_camera_input(&middle, &key_up, &key_down, &wheel);
	controls->local_player_index = local_player_index;
	controls->seconds_elapsed = director_globals.dtime;
	player_index = player_get(
		local_player_get_player_index(local_player_index))->local_player_index;
	if (player_index != NONE && input_has_gamepad((short)player_index))
	{
		unsigned long control_flags;
		struct gamepad_state const *gamepad =
			input_get_gamepad_state((short)player_index);

		if (director_camera_switch_fast)
		{
			switch_camera =
				gamepad->buttons[_gamepad_analog_button_black] == 1;
		}
		else
		{
			byte ticks = gamepad->buttons[_gamepad_analog_button_black];
			/* The hold count is in 30 Hz ticks (input_xbox.c) and this runs
			once a frame, several frames a tick: switch once per second held,
			on the frame the count reaches it. */
			static byte last_ticks[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];

			switch_camera = ticks > 0 && ticks % TICKS_PER_SECOND == 0 &&
				ticks != last_ticks[local_player_index];
			last_ticks[local_player_index] = ticks;
		}

		/* port: cycle the camera from its physical binding in every context;
		 * the director's existing camera selection still enforces permissions. */
		if (remapped) switch_camera = controller_remap_camera_cycle((short)player_index, director_camera_switch_fast);
		if (director->camera_proc !=
				(director_camera_update_proc)first_person_camera_update &&
			director->camera_proc !=
				(director_camera_update_proc)following_camera_update)
		{
			/* port: separate camera context, with stock input as the fallback. */
			if (remapped ? (pad_pressed & (1u << PAD_CAMERA_SWITCH)) != 0 :
				gamepad->buttons[_gamepad_binary_button_right_thumb] == 1)
				director->debug_controls = !director->debug_controls;
			/* port: the mouse takes priority; menus and chat retain their input. */
			if (director->debug_controls && !middle &&
				(!remapped || controller_remap_playing((short)player_index)))
			{
				/* port: moved and turned by the player's input (the sticks,
				the keys and the mouse, with their settings), not the raw sticks;
				raised and lowered by R and F too */
				real_vector2d throttle = player_control_port_debug_throttle[local_player_index];
				real_euler_angles2d facing = player_control_port_debug_facing[local_player_index];
				control_flags = 0;
				SET_FLAG(
					control_flags,
					_camera_control_up_bit,
					(remapped ? (pad_held & (1u << PAD_CAMERA_UP)) != 0 :
						gamepad->buttons[_gamepad_analog_button_right_trigger] != 0) || key_up);
				SET_FLAG(
					control_flags,
					_camera_control_down_bit,
					(remapped ? (pad_held & (1u << PAD_CAMERA_DOWN)) != 0 :
						gamepad->buttons[_gamepad_analog_button_left_trigger] != 0) || key_down);
				controls->wheel_delta = remapped ?
					(real)(!!(pad_held & (1u << PAD_CAMERA_FASTER)) - !!(pad_held & (1u << PAD_CAMERA_SLOWER))) * 0.4f :
					(real)((gamepad->buttons[_gamepad_binary_button_dpad_up] > 1) -
						(gamepad->buttons[_gamepad_binary_button_dpad_down] > 1)) * 0.4f;
				director_process_variables(
					local_player_index,
					control_flags,
					controls->wheel_delta);
				controls->facing_delta.yaw = facing.yaw;
				controls->facing_delta.pitch = facing.pitch;
				controls->position_delta.i =
					throttle.i * director->debug_input_scale * director_globals.dtime * 1.638f;
				controls->position_delta.j =
					throttle.j * director->debug_input_scale * director_globals.dtime * 1.638f;
				controls->position_delta.k +=
					director->debug_variables[_variable_height].delta;
				controls->active = TRUE;
				director_inhibit_input(local_player_index);
				director_inhibit_facing(local_player_index);
			}
		}
	}
	else if (input_get_mouse_state())
	{
		unsigned long control_flags = 0;
		struct mouse_state const *mouse = input_get_mouse_state();

		switch_camera = input_key_is_down(_key_backspace) == TRUE;
		/* port: the passenger follows their aim, never the debug mouse orbit */
		if (!passenger_camera_enabled(local_player_index) && ((director->camera_proc !=
				(director_camera_update_proc)first_person_camera_update &&
			mouse->buttons[1]) || input_key_is_down(_key_tab)))
		{
			SET_FLAG(control_flags, _camera_control_forward_bit,
				input_key_is_down(_key_w));
			SET_FLAG(control_flags, _camera_control_reverse_bit,
				input_key_is_down(_key_s));
			SET_FLAG(control_flags, _camera_control_left_bit,
				input_key_is_down(_key_a));
			SET_FLAG(control_flags, _camera_control_right_bit,
				input_key_is_down(_key_d));
			SET_FLAG(control_flags, _camera_control_up_bit,
				input_key_is_down(_key_r));
			SET_FLAG(control_flags, _camera_control_down_bit,
				input_key_is_down(_key_f));
			SET_FLAG(control_flags, _camera_control_roll_left_bit,
				input_key_is_down(_key_t));
			SET_FLAG(control_flags, _camera_control_roll_right_bit,
				input_key_is_down(_key_g));
			director_process_variables(
				local_player_index,
				control_flags,
				(real)mouse->wheel_delta);
			controls->facing_delta.yaw =
				(real)mouse->x * -0.0031415927f;
			controls->facing_delta.pitch =
				(real)mouse->y * -0.0031415927f;
			controls->facing_delta.roll +=
				director->debug_variables[_variable_roll].delta;
			controls->wheel_delta = (real)mouse->wheel_delta;
			controls->position_delta.i +=
				director->debug_variables[_variable_forward].delta;
			controls->position_delta.j +=
				director->debug_variables[_variable_right].delta;
			controls->position_delta.k +=
				director->debug_variables[_variable_height].delta;
			controls->active = TRUE;
			director_inhibit_input(local_player_index);
			director_inhibit_facing(local_player_index);
		}
	}
	/* port: the debug camera flies while the middle mouse button is held, as
	Custom Edition's and Sapien's: WASD and the mouse move it, R and F raise
	and lower it, the wheel sets its speed. The controller's debug controls
	(the right stick's click) are left as they are, and give way to it while
	the button is held */
	if (middle && !controls->active &&
		(!remapped || controller_remap_playing((short)player_index)) &&
		director->camera_proc !=
			(director_camera_update_proc)first_person_camera_update &&
		director->camera_proc !=
			(director_camera_update_proc)following_camera_update)
	{
		unsigned long control_flags = 0;
		real_vector2d throttle = player_control_port_debug_throttle[local_player_index];
		real_euler_angles2d facing = player_control_port_debug_facing[local_player_index];

		SET_FLAG(control_flags, _camera_control_up_bit, key_up);
		SET_FLAG(control_flags, _camera_control_down_bit, key_down);
		controls->wheel_delta = -(real)wheel;
		director_process_variables(
			local_player_index,
			control_flags,
			controls->wheel_delta);
		controls->facing_delta.yaw = facing.yaw;
		controls->facing_delta.pitch = facing.pitch;
		controls->position_delta.i =
			throttle.i * director->debug_input_scale * director_globals.dtime * 1.638f;
		controls->position_delta.j =
			throttle.j * director->debug_input_scale * director_globals.dtime * 1.638f;
		controls->position_delta.k +=
			director->debug_variables[_variable_height].delta;
		controls->active = TRUE;
		director_inhibit_input(local_player_index);
		director_inhibit_facing(local_player_index);
	}
	return switch_camera;
}

static void director_choose_camera(
	short local_player_index,
	boolean initialize,
	boolean key)
{
	switch (director_globals.game_mode)
	{
	case _director_mode_game:
	case _director_mode_netgame:
		director_choose_camera_game(local_player_index, initialize, key);
		return;

	case _director_mode_editor:
		director_choose_camera_editor(local_player_index, initialize, key);
		return;

	case _director_mode_script_camera_record:
		director_choose_camera_script_camera_record(local_player_index, initialize, key);
		return;
	}

	return;
}

void director_initialize_for_new_map(
	void)
{
	short local_player_index;

	controller_remap_reset(); /* port: release held actions and camera context between maps */

	director_globals.game_mode = game_in_editor() ? _director_mode_editor : _director_mode_game;
	director_globals.initialize_camera = FALSE;
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		struct director *director = director_get(local_player_index);

		director->bored = FALSE;
		director->bored_time = 0;
		director->camera_change_pause = 0.f;
		director_choose_camera(local_player_index, TRUE, FALSE);
		director_initialize_variables(local_player_index);
	}

	return;
}

void director_update(
	real time_delta_sec)
{
	short local_player_index;

	director_globals.dtime = time_delta_sec;
	for (local_player_index = 0;
		local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS;
		local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			boolean switch_camera;
			struct camera_control controls;
			struct observer_command command;
			struct director *director = director_get(local_player_index);

			director->inhibited_facing = FALSE;
			director->inhibited_input = FALSE;
			switch_camera =
				director_update_controls(local_player_index, &controls);
			director_choose_camera(
				local_player_index,
				director_globals.initialize_camera,
				switch_camera);
			director_globals.initialize_camera = FALSE;
			csmemset(&command, 0, sizeof(command));

			if (director->camera_proc &&
				(director->camera_proc !=
					(director_camera_update_proc)scripted_camera_update ||
				local_player_index == local_player_get_next(NONE)))
			{
				director->camera_proc(
					director->camera_data,
					&controls,
					&command);
			}

			if (TEST_FLAG(command.flags, _observer_command_valid_bit))
			{
				if (director->camera_change_pause != 0.f)
				{
					if (director->camera_change_pause < 0.2f &&
						director->camera_proc ==
							(director_camera_update_proc)first_person_camera_update)
					{
						director->camera_change_pause = 0.f;
						command.parameter_timers[0] = 0.f;
						command.parameter_flags[0] =
							FLAG(_observer_time_valid_bit) |
							FLAG(_observer_time_force_bit);
						command.parameter_timers[2] = 0.f;
						command.parameter_flags[2] =
							FLAG(_observer_time_valid_bit) |
							FLAG(_observer_time_force_bit);
					}
					else
					{
						command.timer = MAX(
							command.timer,
							director->camera_change_pause);
					}

					director->camera_change_pause = MAX(
						0.f,
						director->camera_change_pause - time_delta_sec);
				}

				director->command = command;
			}
			else
			{
				SET_FLAG(
					director->command.flags,
					_observer_command_valid_bit,
					FALSE);
			}

			observer_set_camera(local_player_index, &director->command);
		}
	}

	return;
}

void director_initialize_for_saved_game(
	void)
{
	director_initialize_for_new_map();
	director_script_camera(*director_camera_scripted);

	return;
}

short director_camera_deterministic(
	long unit_index,
	real_point3d *position,
	real_vector3d *forward)
{
	director_perspective perspective;
	short following;

	following = director_desired_perspective(unit_index, &perspective);
	if (!following)
		first_person_camera_deterministic(unit_index, position, forward);
	else
		following_camera_deterministic(unit_index, position, forward);

	return following;
}

/* ---------- private code */

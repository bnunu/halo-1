/*
PLAYER_EFFECTS.C

symbols in this file:
00090D00 0070:
	_player_effect_get (0000)
00090D70 0040:
	_player_effect_initialize (0000)
00090DB0 0010:
	_player_effect_dispose (0000)
00090DC0 0040:
	_player_effect_initialize_for_new_map (0000)
00090E00 0010:
	_player_effect_dispose_from_old_map (0000)
00090E10 0140:
	_player_effect_add_continuous_effect (0000)
00090F50 0040:
	_scripted_player_effect_set_rotation (0000)
00090F90 0010:
	_scripted_player_effect_set_rumble (0000)
00090FA0 0040:
	_player_telefrag_effect_stop (0000)
00090FE0 0050:
	_player_effect_screen_fade_in (0000)
00091030 0050:
	_player_effect_screen_fade_out (0000)
00091080 0060:
	_player_effect_get_damage_indicators (0000)
000910E0 0020:
	_player_effect_clear_damage_indicators (0000)
00091100 0020:
	_effect_scale_factor (0000)
00091120 00f0:
	_player_effect_update_screen_flash (0000)
00091210 00d0:
	_player_effect_update_camera_shake (0000)
000912E0 0030:
	_effect_scale_value (0000)
00091310 0090:
	_player_effect_update (0000)
000913A0 0090:
	_player_effect_continuous_refresh (0000)
00091430 0030:
	_scripted_player_effect_set_translation (0000)
00091460 0050:
	_scripted_player_effect_start (0000)
000914B0 0040:
	_scripted_player_effect_stop (0000)
000914F0 0050:
	_player_effect_screen_flash (0000)
00091540 00f0:
	_player_telefrag_effect_start (0000)
00091630 0320:
	_player_effect_get_screen_flash (0000)
00091950 0090:
	_get_shake_matrix (0000)
000919E0 0520:
	_player_effect_get_camera_effect_matrix (0000)
00091F00 02f0:
	_player_effect_update_camera_impulse (0000)
000921F0 02e0:
	_player_effect_start (0000)
0025AA2C 0016:
	??_C@_0BG@DLCFJLMF@player_effect_globals?$AA@ (0000)
0025AA44 0028:
	??_C@_0CI@HGGDJELB@c?3?2halo?2SOURCE?2effects?2player_ef@ (0000)
0025AA6C 000f:
	??_C@_0P@DCHLBMFM@player?5effects?$AA@ (0000)
0025AA80 0008:
	__real@3f847ae147ae147b (0000)
0025AA88 0018:
	??_C@_0BI@MALCPACJ@screen_flash?9?$DOintensity?$AA@ (0000)
0025AAA0 003f:
	??_C@_0DP@NOLEHAI@screen_flash?9?$DOintensity?$DO?$DN0?40f?5?$CG?$CG@ (0000)
0025AAE0 000d:
	??_C@_0N@MFJHANDO@screen_flash?$AA@ (0000)
0025AAF0 0007:
	??_C@_06CGNOPMBC@matrix?$AA@ (0000)
0025AAF8 0004:
	__real@4016cbe4 (0000)
002DDDA0 000e:
	_render_screen_flash_type_map (0000)
0043D58C 0004:
	_player_effect_globals (0000)
*/

/* ---------- headers */

#include "effects/player_effects.h"

#include "camera/observer.h"
#include "cseries/errors.h"
#include "game/game.h"
#include "game/game_globals.h"
#include "game/player_rumble.h"
#include "game/players.h"
#include "math/periodic_functions.h"
#include "math/real_math.h"
#include "main/console.h"
#include "networking/network_connection.h"
#include "objects/damage.h"
#include "objects/damage_effect_definitions.h"
#include "objects/objects.h"
#include "render/render_cameras.h"
#include "saved games/game_state.h"
#include "sound/game_sound.h"
#include "tag_files/tag_groups.h"
#include "units/units.h"

#include <stddef.h>
#include "network_coop.h" /* port: port/linux/game/network_coop.c */

/* ---------- constants */

enum
{
	NUMBER_OF_DAMAGE_INDICATORS = 4
};

enum screen_flash_type
{
	_screen_flash_type_none = 0,
	_screen_flash_type_lighten,
	_screen_flash_type_darken,
	_screen_flash_type_max,
	_screen_flash_type_min,
	_screen_flash_type_invert,
	_screen_flash_type_tint,

	NUMBER_OF_SCREEN_FLASH_TYPES
};

enum screen_flash_priority
{
	_screen_flash_low_priority = 0,
	_screen_flash_medium_priority,
	_screen_flash_high_priority,

	NUMBER_OF_SCREEN_FLASH_PRIORITIES
};

enum render_screen_flash_type
{
	_render_screen_flash_type_none = 0,
	_render_screen_flash_type_lighten,
	_render_screen_flash_type_darken,
	_render_screen_flash_type_max,
	_render_screen_flash_type_min,
	_render_screen_flash_type_invert,
	_render_screen_flash_type_tint,

	NUMBER_OF_RENDER_SCREEN_FLASH_TYPES
};

enum
{
	_scripted_player_effect_active_bit,
	_scripted_player_effect_stopping_bit,

	NUMBER_OF_SCRIPTED_PLAYER_EFFECT_FLAGS
};

enum
{
	_player_effect_screen_flash_just_started_bit,
	_player_effect_camera_impulse_just_started_bit,
	_player_effect_camera_shake_just_started_bit,

	NUMBER_OF_PLAYER_EFFECT_FLAGS
};

enum
{
	_damage_draw_indicators_down_bit = 8
};

/* ---------- macros */

/* ---------- structures */

struct continuous_player_effect_datum
{
	real vibrate_frequencies[2];
	real translational_shake;
	real rotational_shake;
};

struct player_effect_datum
{
	real_vector3d direction;
	real_vector3d jitter;
	struct screen_flash_definition screen_flash;
	struct camera_impulse_definition camera_impulse;
	struct camera_shake_definition camera_shake;
	struct continuous_player_effect_datum continuous_effect;
	short continuous_effect_timer;
	short screen_flash_time_left;
	short camera_impulse_time_left;
	short camera_shake_time_left;
	byte damage_indicator_ticks[NUMBER_OF_DAMAGE_INDICATORS];
	byte flags;
	byte pad[3];
};

struct screen_fade_definition
{
	real_rgb_color color;
	long start_time;
	short ticks;
	boolean fading_out;
	byte pad;
};

struct scripted_player_effect_definition
{
	real_vector3d max_translation;
	real_euler_angles3d max_rotation;
	real max_intensity;
	short timer;
	short total_time;
};

struct player_effect_globals_definition
{
	struct player_effect_datum local_player_effect_data[MAXIMUM_NUMBER_OF_LOCAL_PLAYERS];
	struct screen_fade_definition screen_fade;
	struct scripted_player_effect_definition scripted_effect;
	unsigned long global_flags;
	long reference_time;
};

typedef char player_effect_datum_size_assert[
	sizeof(struct player_effect_datum) == 0xEC ? 1 : -1];
typedef char player_effect_datum_screen_flash_offset_assert[
	offsetof(struct player_effect_datum, screen_flash) == 0x18 ? 1 : -1];
typedef char player_effect_datum_camera_impulse_offset_assert[
	offsetof(struct player_effect_datum, camera_impulse) == 0x50 ? 1 : -1];
typedef char player_effect_datum_camera_shake_offset_assert[
	offsetof(struct player_effect_datum, camera_shake) == 0x84 ? 1 : -1];
typedef char player_effect_datum_continuous_offset_assert[
	offsetof(struct player_effect_datum, continuous_effect) == 0xCC ? 1 : -1];
typedef char player_effect_damage_indicator_ticks_offset_assert[
	offsetof(struct player_effect_datum, damage_indicator_ticks) == 0xE4 ? 1 : -1];
typedef char player_effect_globals_size_assert[
	sizeof(struct player_effect_globals_definition) == 0x3EC ? 1 : -1];
typedef char player_effect_globals_screen_fade_offset_assert[
	offsetof(struct player_effect_globals_definition, screen_fade) == 0x3B0 ? 1 : -1];
typedef char player_effect_globals_scripted_effect_offset_assert[
	offsetof(struct player_effect_globals_definition, scripted_effect) == 0x3C4 ? 1 : -1];
typedef char scripted_player_effect_definition_size_assert[
	sizeof(struct scripted_player_effect_definition) == 0x20 ? 1 : -1];
typedef char player_effect_globals_global_flags_offset_assert[
	offsetof(struct player_effect_globals_definition, global_flags) == 0x3E4 ? 1 : -1];
typedef char player_effect_globals_reference_time_offset_assert[
	offsetof(struct player_effect_globals_definition, reference_time) == 0x3E8 ? 1 : -1];

/* ---------- prototypes */

struct player_effect_datum *player_effect_get(
	short local_player_index);

static real effect_scale_factor(
	real zero_scale_factor,
	real scale);

static void player_effect_update_screen_flash(
	short local_player_index,
	struct player_effect_datum *effect,
	struct screen_flash_definition const *screen_flash,
	real scale,
	real time_scale);

static void player_effect_update_camera_shake(
	short local_player_index,
	struct player_effect_datum *effect,
	struct camera_shake_definition const *camera_shake,
	real scale,
	real time_scale);

static void player_effect_update_camera_impulse(
	short local_player_index,
	struct player_effect_datum *effect,
	struct camera_impulse_definition const *camera_impulse,
	real_vector3d const *direction,
	real scale,
	real time_scale);

static real effect_scale_value(
	short transition_function,
	real zero_scale_factor,
	real elapsed,
	real duration);

static void get_shake_matrix(
	real translation,
	real rotation,
	real_matrix4x3 *matrix);

/* ---------- globals */

static struct player_effect_globals_definition *player_effect_globals;

/* port: each local player's camera effect as the first frame of a tick made
it (the tick plus one, 0 for none), which the frames after it until the next
tick draw again (player_effect_get_camera_effect_matrix) */
static struct
{
	long tick;
	real_matrix4x3 matrix;
} camera_effect_of_tick[MAXIMUM_LOCAL_PLAYERS];

static short render_screen_flash_type_map[NUMBER_OF_SCREEN_FLASH_TYPES] =
{
	_render_screen_flash_type_none,
	_render_screen_flash_type_lighten,
	_render_screen_flash_type_darken,
	_render_screen_flash_type_max,
	_render_screen_flash_type_min,
	_render_screen_flash_type_invert,
	_render_screen_flash_type_tint
};

/* ---------- public code */

struct player_effect_datum *player_effect_get(
	short local_player_index)
{
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 115, local_player_index>=0 && local_player_index<MAXIMUM_NUMBER_OF_LOCAL_PLAYERS);
	match_vassert("c:\\halo\\SOURCE\\effects\\player_effects.c", 116, player_effect_globals, "player_effect_globals");

	return &player_effect_globals->local_player_effect_data[local_player_index];
}

void player_effect_initialize(
	void)
{
	player_effect_globals = (struct player_effect_globals_definition *)game_state_malloc(
		"player effects",
		NULL,
		sizeof(struct player_effect_globals_definition));
	match_vassert("c:\\halo\\SOURCE\\effects\\player_effects.c", 127, player_effect_globals, "player_effect_globals");

	return;
}

void player_effect_dispose(
	void)
{
	return;
}

void player_effect_initialize_for_new_map(
	void)
{
	csmemset(player_effect_globals, 0, sizeof(struct player_effect_globals_definition));
	player_effect_globals->screen_fade.ticks = NONE;
	player_effect_globals->reference_time = game_time_get();
	/* port: (player_effect_get_camera_effect_matrix) */
	csmemset(camera_effect_of_tick, 0, sizeof(camera_effect_of_tick));

	return;
}

void player_effect_dispose_from_old_map(
	void)
{
	return;
}

void player_effect_add_continuous_effect(
	short local_player_index,
	long effect_index,
	real distance)
{
	struct continuous_damage_effect_definition *definition =
		continuous_damage_effect_definition_get(effect_index);

	if (distance < definition->cutoff_radius)
	{
		struct player_effect_datum *effect = player_effect_get(local_player_index);
		struct continuous_player_effect_datum *continuous = &effect->continuous_effect;
		real scale = PIN(1.0f - (distance - definition->falloff_radius) / (definition->falloff_radius - definition->cutoff_radius), 0.0f, 1.0f);
		real magnitude;

		magnitude = ((1.0f - definition->camera_shake.periodic_weight) +
			periodic_function_evaluate(definition->camera_shake.periodic_function, game_time_get() / definition->camera_shake.periodic_period) *
			definition->camera_shake.periodic_weight) * scale;

		if (effect->continuous_effect_timer > 0)
		{
			effect->continuous_effect_timer = 0;
			csmemset(continuous, 0, sizeof(*continuous));
		}

		continuous->translational_shake += MAX(
			magnitude * definition->camera_shake.random_translation_magnitude,
			0.0f);
		continuous->rotational_shake += MAX(
			magnitude * definition->camera_shake.random_rotation_magnitude,
			0.0f);
		continuous->vibrate_frequencies[0] += scale * definition->vibrate.frequencies[0];
		continuous->vibrate_frequencies[1] += scale * definition->vibrate.frequencies[1];
	}

	return;
}

void scripted_player_effect_set_rotation(
	real yaw,
	real pitch,
	real roll)
{
	network_coop_note_player_effect(_coop_player_effect_rotation, yaw, pitch, roll);
	player_effect_globals->scripted_effect.max_rotation.yaw = DEGREES_TO_RADIANS(yaw);
	player_effect_globals->scripted_effect.max_rotation.pitch = DEGREES_TO_RADIANS(pitch);
	player_effect_globals->scripted_effect.max_rotation.roll = DEGREES_TO_RADIANS(roll);

	return;
}

void scripted_player_effect_set_rumble(
	real left_motor,
	real right_motor)
{
	rumble_player_set_scripted_values(left_motor, right_motor);
	return;
}

void player_telefrag_effect_stop(
	long player_index)
{
	long local_player_index = player_get(player_index)->local_player_index;

	if (local_player_index != NONE)
	{
		player_effect_get((short)local_player_index);
		rumble_player_continuous((short)local_player_index, 0.0f, 0.0f);
	}

	return;
}

void player_effect_screen_fade_in(
	real red,
	real green,
	real blue,
	short ticks)
{
	player_effect_globals->screen_fade.color.red = red;
	player_effect_globals->screen_fade.color.green = green;
	player_effect_globals->screen_fade.color.blue = blue;
	player_effect_globals->screen_fade.ticks = ticks;
	player_effect_globals->screen_fade.fading_out = FALSE;
	player_effect_globals->screen_fade.start_time = game_time_get();

	return;
}

void player_effect_screen_fade_out(
	real red,
	real green,
	real blue,
	short ticks)
{
	player_effect_globals->screen_fade.color.red = red;
	player_effect_globals->screen_fade.color.green = green;
	player_effect_globals->screen_fade.color.blue = blue;
	player_effect_globals->screen_fade.ticks = ticks;
	player_effect_globals->screen_fade.fading_out = TRUE;
	player_effect_globals->screen_fade.start_time = game_time_get();

	return;
}

/* port: read and set the screen fade, so a co-op host can send it to its
clients (port/linux/game/network_coop.c): color, length, direction, and
the game time it started */
void player_effect_port_screen_fade_get(
	real_rgb_color *color,
	short *ticks,
	boolean *fading_out,
	long *start_time)
{
	*color = player_effect_globals->screen_fade.color;
	*ticks = player_effect_globals->screen_fade.ticks;
	*fading_out = player_effect_globals->screen_fade.fading_out;
	*start_time = player_effect_globals->screen_fade.start_time;

	return;
}

void player_effect_port_screen_fade_set(
	real_rgb_color const *color,
	short ticks,
	boolean fading_out,
	long start_time)
{
	player_effect_globals->screen_fade.color = *color;
	player_effect_globals->screen_fade.ticks = ticks;
	player_effect_globals->screen_fade.fading_out = fading_out;
	player_effect_globals->screen_fade.start_time = start_time;

	return;
}

boolean player_effect_port_scripted_active(
	void)
{
	return TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_active_bit);
}

void player_effect_port_scripted_end(
	void)
{
	SET_FLAG(player_effect_globals->global_flags, _scripted_player_effect_active_bit, FALSE);
	SET_FLAG(player_effect_globals->global_flags, _scripted_player_effect_stopping_bit, FALSE);
}

void player_effect_get_damage_indicators(
	short local_player_index,
	byte *damage_indicators)
{
	struct player_effect_datum *effect = player_effect_get(local_player_index);
	short index;

	csmemcpy(damage_indicators, effect->damage_indicator_ticks, sizeof(effect->damage_indicator_ticks));

	for (index = 0; index < NUMBER_OF_DAMAGE_INDICATORS; index++)
	{
		if (effect->damage_indicator_ticks[index])
		{
			effect->damage_indicator_ticks[index] = (byte)((game_time_get_elapsed() + effect->damage_indicator_ticks[index] < 255)
				? game_time_get_elapsed() + effect->damage_indicator_ticks[index]
				: 255);
		}
	}

	return;
}

void player_effect_clear_damage_indicators(
	short local_player_index)
{
	struct player_effect_datum *effect = player_effect_get(local_player_index);

	csmemset(
		effect->damage_indicator_ticks,
		0,
		sizeof(effect->damage_indicator_ticks));
	return;
}

static real effect_scale_factor(
	real zero_scale_factor,
	real scale)
{
	return zero_scale_factor + (1.0f - zero_scale_factor) * scale;
}

static void player_effect_update_screen_flash(
	short local_player_index,
	struct player_effect_datum *effect,
	struct screen_flash_definition const *screen_flash,
	real scale,
	real time_scale)
{
	real time_factor = time_scale * TICKS_PER_SECOND;

	/* port: the type is the map's (a damage effect's; retail up to 6 of 7)
	and indexes the type map, here and as the flash is drawn: one that is
	no type is no flash, and that is said once */
	if (!VALID_INDEX(screen_flash->type, NUMBER_OF_SCREEN_FLASH_TYPES))
	{
		static boolean bad_type_reported = FALSE;

		if (!bad_type_reported)
		{
			bad_type_reported = TRUE;
			error(
				_error_silent,
				"screen flash of type %d (of %d) not shown",
				screen_flash->type,
				NUMBER_OF_SCREEN_FLASH_TYPES);
		}

		return;
	}

	if (!(effect->screen_flash.priority > screen_flash->priority &&
		effect->screen_flash_time_left > screen_flash->duration * time_factor) &&
		render_screen_flash_type_map[screen_flash->type])
	{
		effect->screen_flash = *screen_flash;
		effect->screen_flash.duration = time_factor * effect->screen_flash.duration;
		effect->screen_flash_time_left = (short)effect->screen_flash.duration;
		effect->screen_flash.zero_scale_factor = PIN(
			effect_scale_factor(screen_flash->zero_scale_factor, scale),
			0.0f,
			screen_flash->max_intensity);
		SET_FLAG(effect->flags, _player_effect_screen_flash_just_started_bit, TRUE);
	}

	return;
}

static void player_effect_update_camera_shake(
	short local_player_index,
	struct player_effect_datum *effect,
	struct camera_shake_definition const *camera_shake,
	real scale,
	real time_scale)
{
	real time_factor;
	real intensity;

	game_time_get();

	time_factor = time_scale * TICKS_PER_SECOND;
	intensity = effect_scale_factor(camera_shake->zero_scale_factor, scale);

	if (effect->camera_shake_time_left < time_factor * camera_shake->duration ||
		intensity > effect->camera_shake.zero_scale_factor ||
		(intensity >= effect->camera_shake.zero_scale_factor &&
			effect->camera_shake_time_left < time_factor * camera_shake->duration))
	{
		effect->camera_shake = *camera_shake;
		effect->camera_shake.zero_scale_factor = intensity;
		effect->camera_shake.duration = time_factor * effect->camera_shake.duration;
		effect->camera_shake_time_left = (short)effect->camera_shake.duration;
		effect->camera_shake.periodic_period = time_factor * effect->camera_shake.periodic_period;
		SET_FLAG(effect->flags, _player_effect_camera_shake_just_started_bit, TRUE);
	}

	return;
}

void player_effect_update(
	void)
{
	short local_player_index;

	for (local_player_index = local_player_get_next(NONE);
		local_player_index != NONE;
		local_player_index = local_player_get_next(local_player_index))
	{
		if (local_player_get_player_index(local_player_index) == NONE
			|| player_get(local_player_get_player_index(local_player_index))->unit_index == NONE)
		{
			player_effect_clear_damage_indicators(local_player_index);
			csmemset(player_effect_get(local_player_index), 0, sizeof(struct player_effect_datum));
			rumble_player_clear(local_player_index);
		}
	}

	return;
}

void player_effect_continuous_refresh(
	long definition_index,
	real_point3d const *position)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
	{
		if (local_player_get_player_index(local_player_index) != NONE)
		{
			long unit_index = player_get(local_player_get_player_index(local_player_index))->unit_index;

			if (unit_index != NONE)
			{
				real_point3d origin;

				object_get_origin(unit_index, &origin);
				player_effect_add_continuous_effect(local_player_index, definition_index, distance3d(&origin, position));
			}
		}
	}

	return;
}

void scripted_player_effect_set_translation(
	real horizontal,
	real vertical,
	real depth)
{
	real_vector3d *translation = &player_effect_globals->scripted_effect.max_translation;

	network_coop_note_player_effect(_coop_player_effect_translation, horizontal, vertical, depth);
	translation->i = horizontal;
	translation->j = vertical;
	translation->k = depth;

	return;
}

void scripted_player_effect_start(
	real maximum_intensity,
	real attack_time)
{
	short ticks;

	network_coop_note_player_effect(_coop_player_effect_start, maximum_intensity, attack_time, 0.0f);
	player_effect_globals->scripted_effect.max_intensity = maximum_intensity;

	ticks = (short)fast_ftol(attack_time * TICKS_PER_SECOND);
	player_effect_globals->scripted_effect.timer = ticks;
	player_effect_globals->scripted_effect.total_time = ticks;
	SET_FLAG(
		player_effect_globals->global_flags,
		_scripted_player_effect_stopping_bit,
		FALSE);
	SET_FLAG(
		player_effect_globals->global_flags,
		_scripted_player_effect_active_bit,
		TRUE);

	return;
}

void scripted_player_effect_stop(
	real duration)
{
	short ticks = (short)fast_ftol(duration * TICKS_PER_SECOND);

	network_coop_note_player_effect(_coop_player_effect_stop, duration, 0.0f, 0.0f);
	player_effect_globals->scripted_effect.timer = ticks;
	player_effect_globals->scripted_effect.total_time = ticks;
	SET_FLAG(
		player_effect_globals->global_flags,
		_scripted_player_effect_stopping_bit,
		TRUE);

	return;
}

void player_effect_screen_flash(
	long player_index,
	struct screen_flash_definition const *screen_flash,
	real scale)
{
	if (player_index != NONE)
	{
		long local_player_index = player_get(player_index)->local_player_index;

		if (local_player_index != NONE)
		{
			struct player_effect_datum *effect = player_effect_get((short)local_player_index);

			player_effect_update_screen_flash((short)local_player_index, effect, screen_flash, scale, 1.0f);
		}
	}

	return;
}

void player_effect_get_screen_flash(
	short local_player_index,
	struct render_screen_flash *screen_flash)
{
	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 484, screen_flash);

	if (!console_is_active())
	{
		if (player_effect_globals->screen_fade.ticks != NONE &&
			(player_effect_globals->screen_fade.fading_out ||
				game_time_get() - player_effect_globals->screen_fade.start_time <= player_effect_globals->screen_fade.ticks))
		{
			screen_flash->type = _render_screen_flash_type_lighten;
			screen_flash->color.rgb = player_effect_globals->screen_fade.color;
			screen_flash->color.alpha = 1.0f;
			screen_flash->intensity = player_effect_globals->screen_fade.ticks > 0
				? transition_function_evaluate(
					_transition_function_cosine,
					PIN((real)(game_time_get() - player_effect_globals->screen_fade.start_time) / player_effect_globals->screen_fade.ticks, 0.0f, 1.0f))
				: 1.0f;

			if (!player_effect_globals->screen_fade.fading_out)
				screen_flash->intensity = 1.0f - screen_flash->intensity;

			screen_flash->intensity = PIN(screen_flash->intensity, 0.0f, 1.0f);
		}
		else if (local_player_index != NONE)
		{
			struct player_effect_datum *effect = player_effect_get(local_player_index);

			player_effect_globals->screen_fade.ticks = NONE;

			if (effect->screen_flash_time_left > 0 ||
				TEST_FLAG(effect->flags, _player_effect_screen_flash_just_started_bit))
			{
				struct screen_flash_definition const *flash = &effect->screen_flash;

				SET_FLAG(
					effect->flags,
					_player_effect_screen_flash_just_started_bit,
					FALSE);

				screen_flash->type = render_screen_flash_type_map[flash->type];
				screen_flash->color = flash->screen_flash_color;
				if (flash->duration > 0.0f)
					screen_flash->intensity = transition_function_evaluate(
						flash->fade_function,
						flash->zero_scale_factor * ((real)effect->screen_flash_time_left / flash->duration));
				else
					screen_flash->intensity = flash->zero_scale_factor;

				effect->screen_flash_time_left -= game_time_get_elapsed();

				match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 530, screen_flash->intensity>=0.0f && screen_flash->intensity<=1.0f);
			}
		}
	}

	match_assert_valid_real("c:\\halo\\SOURCE\\effects\\player_effects.c", 535, screen_flash->intensity);

	return;
}

void player_telefrag_effect_start(
	long player_index,
	real intensity)
{
	struct screen_flash_definition screen_flash = {0};
	struct camera_shake_definition camera_shake = {0};
	long local_player_index = player_get(player_index)->local_player_index;

	if (local_player_index != NONE)
	{
		struct player_effect_datum *effect = player_effect_get((short)local_player_index);

		camera_shake.random_translation_magnitude = intensity * 0.01;
		screen_flash.max_intensity = intensity;
		camera_shake.duration = 1.0f;
		screen_flash.type = _screen_flash_type_lighten;
		screen_flash.priority = _screen_flash_high_priority;
		screen_flash.duration = 1.0f;
		screen_flash.zero_scale_factor = 0.0f;
		screen_flash.screen_flash_color = *global_real_argb_white;

		rumble_player_continuous((short)local_player_index, intensity, intensity);
		player_effect_update_screen_flash((short)local_player_index, effect, &screen_flash, intensity, 1.0f);
		player_effect_update_camera_shake((short)local_player_index, effect, &camera_shake, intensity, 1.0f);
	}

	return;
}

static real effect_scale_value(
	short transition_function,
	real zero_scale_factor,
	real elapsed,
	real duration)
{
	return transition_function_evaluate(
		transition_function,
		1.0f - elapsed / duration) * zero_scale_factor;
}

static void get_shake_matrix(
	real translation,
	real rotation,
	real_matrix4x3 *matrix)
{
	real_vector3d direction;

	if (rotation != 0.0f)
	{
		seed_random_direction3d(
			get_global_local_random_seed_address(),
			&direction);
		matrix4x3_rotation_from_axis_and_angle(
			matrix,
			&direction,
			sine(rotation),
			cosine(rotation));
	}

	if (translation != 0.0f)
	{
		seed_random_direction3d(
			get_global_local_random_seed_address(),
			&direction);
		matrix->position.x = direction.i * translation;
		matrix->position.y = direction.j * translation;
		matrix->position.z = direction.k * translation;
	}

	return;
}

void player_effect_get_camera_effect_matrix(
	short local_player_index,
	real_matrix4x3 *matrix)
{
	match_assert(
		"c:\\halo\\SOURCE\\effects\\player_effects.c",
		601,
		matrix);

	if (local_player_index != NONE)
	{
		/* The camera is drawn every frame, several frames per tick on the
		native builds (port/linux/game/render_interpolation.c), and each draw
		shook it in a new random direction: shake once a tick, as on the
		Xbox, by drawing from a seed made from the tick. */
		unsigned long *local_seed = get_global_local_random_seed_address();
		unsigned long saved_local_seed = *local_seed;
		unsigned long tick_seed =
			(unsigned long)game_time_get() * 0x9E3779B1UL ^
			(unsigned long)local_player_index * 0x85EBCA6BUL;

		tick_seed ^= tick_seed >> 16;
		tick_seed *= 0x7FEB352DUL;
		tick_seed ^= tick_seed >> 15;
		*local_seed = tick_seed;
		game_time_get();

		/* port: the Xbox drew a frame a tick, so a shake or impulse just
		started showed at its full scale for a tick, and each later tick's
		scale for a tick. A frame between ticks drew the scale after the
		tick's (its time left counted down by the tick's first frame), and
		the full scale only on that first frame: at 240 frames a second an
		eighth of the tick, so firing shook the view a fraction as much, the
		assault rifle's short kick hardly at all. The frames after a tick's
		first draw its effect again, as the Xbox drew it for the tick; the
		continuous shake looping sounds add each frame is still used up, as
		that frame would (game_sound.c). */
		if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS &&
			camera_effect_of_tick[local_player_index].tick == game_time_get() + 1)
		{
			struct player_effect_datum *effect = player_effect_get(local_player_index);

			*matrix = camera_effect_of_tick[local_player_index].matrix;
			if (!TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_active_bit) &&
				effect->camera_shake_time_left > 0)
			{
				effect->continuous_effect_timer = 0;
				csmemset(&effect->continuous_effect, 0, sizeof(effect->continuous_effect));
			}
			*local_seed = saved_local_seed;
			return;
		}

		if (TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_active_bit))
		{
			struct scripted_player_effect_definition *scripted_effect =
				&player_effect_globals->scripted_effect;
			real intensity = scripted_effect->max_intensity;

			*matrix = *global_identity4x3;

			if (scripted_effect->timer > 0)
			{
				if (TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_stopping_bit))
					intensity = ((real)scripted_effect->timer / scripted_effect->total_time) * intensity;
				else
					intensity = (1.0f - (real)scripted_effect->timer / scripted_effect->total_time) * intensity;

				scripted_effect->timer -= game_time_get_elapsed();
			}
			else if (TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_stopping_bit))
			{
				SET_FLAG(
					player_effect_globals->global_flags,
					_scripted_player_effect_active_bit,
					FALSE);
				rumble_player_set_scale(0.0f);
			}

			if (TEST_FLAG(player_effect_globals->global_flags, _scripted_player_effect_active_bit))
			{
				real random_roll;
				real random_pitch;
				real random_yaw;
				real random_depth;
				real random_horizontal;
				real random_vertical;

				intensity = PIN(intensity, 0.0f, 1.0f);
				rumble_player_set_scale(intensity);

				random_roll = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);
				random_pitch = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);
				random_yaw = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);

				matrix4x3_rotation_from_angles(
					matrix,
					scripted_effect->max_rotation.yaw * random_yaw * intensity,
					scripted_effect->max_rotation.pitch * random_pitch * intensity,
					scripted_effect->max_rotation.roll * random_roll * intensity);

				random_depth = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);
				random_horizontal = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);
				random_vertical = real_seed_random_range(
					get_global_local_random_seed_address(),
					-1.0f,
					1.0f);

				matrix->position.z = scripted_effect->max_translation.k * random_depth * intensity;
				matrix->position.y = scripted_effect->max_translation.i * random_horizontal * intensity;
				matrix->position.x = scripted_effect->max_translation.j * random_vertical * intensity;
			}
		}
		else
		{
			struct player_effect_datum *effect = player_effect_get(local_player_index);
			real_matrix4x3 effect_matrix;
			real_matrix4x3 const *source;
			short camera_shake_ticks;

			if (effect->camera_impulse_time_left > 0 ||
				TEST_FLAG(effect->flags, _player_effect_camera_impulse_just_started_bit))
			{
				real_vector3d axis;
				real scale;
				real rotation;
				real translation;

				if (TEST_FLAG(effect->flags, _player_effect_camera_impulse_just_started_bit))
				{
					scale = 1.0f;
				}
				else
				{
					real duration = effect->camera_impulse.temporary_duration;

					scale = effect_scale_value(
						effect->camera_impulse.temporary_transition,
						effect->camera_impulse.temporary_zero_scale_factor,
						duration - effect->camera_impulse_time_left,
						duration);
				}

				SET_FLAG(
					effect->flags,
					_player_effect_camera_impulse_just_started_bit,
					FALSE);

				cross_product3d(global_up3d, &effect->direction, &axis);

				rotation = scale * effect->camera_impulse.temporary_rotation;
				matrix4x3_rotation_from_axis_and_angle(
					&effect_matrix,
					&axis,
					sine(rotation),
					cosine(rotation));

				translation = scale * effect->camera_impulse.temporary_translation;
				effect_matrix.position.x = effect->direction.i * translation + effect->jitter.i * scale;
				effect_matrix.position.y = effect->direction.j * translation + effect->jitter.j * scale;
				effect_matrix.position.z = effect->direction.k * translation + effect->jitter.k * scale;

				effect->camera_impulse_time_left -= game_time_get_elapsed();
				source = &effect_matrix;
			}
			else
			{
				source = global_identity4x3;
			}

			*matrix = *source;

			camera_shake_ticks = effect->camera_shake_time_left;

			if (camera_shake_ticks > 0 ||
				TEST_FLAG(effect->flags, _player_effect_camera_shake_just_started_bit))
			{
				struct continuous_player_effect_datum *continuous =
					&effect->continuous_effect;
				real scale;
				real magnitude;
				real random_translation_magnitude;
				real random_rotation_magnitude;

				effect_matrix = *global_identity4x3;

				if (TEST_FLAG(effect->flags, _player_effect_camera_shake_just_started_bit))
				{
					scale = 1.0f;
				}
				else
				{
					real duration = effect->camera_shake.duration;

					scale = effect_scale_value(
						effect->camera_shake.falloff_transition_function,
						effect->camera_shake.zero_scale_factor,
						duration - camera_shake_ticks,
						duration);
				}

				magnitude = (periodic_function_evaluate(
					effect->camera_shake.periodic_function,
					(effect->camera_shake.duration - effect->camera_shake_time_left) /
						effect->camera_shake.periodic_period) *
						effect->camera_shake.periodic_weight +
						(1.0f - effect->camera_shake.periodic_weight)) * scale;

				random_translation_magnitude = MAX(
					magnitude * effect->camera_shake.random_translation_magnitude,
					0.0f);
				random_rotation_magnitude = MAX(
					magnitude * effect->camera_shake.random_rotation_magnitude,
					0.0f);

				SET_FLAG(
					effect->flags,
					_player_effect_camera_shake_just_started_bit,
					FALSE);

				get_shake_matrix(
					random_translation_magnitude + continuous->translational_shake,
					random_rotation_magnitude + continuous->rotational_shake,
					&effect_matrix);

				rumble_player_continuous(
					local_player_index,
					continuous->vibrate_frequencies[0],
					continuous->vibrate_frequencies[1]);

				/* Looping sounds add their continuous shake once a frame
				(game_sound.c), so it is used up once a frame too: a frame that
				runs no tick (port/linux/game/render_interpolation.c) must not
				keep it and have the next frame's shake added on top. */
				effect->continuous_effect_timer = 1;

				if (effect->continuous_effect_timer > 0)
				{
					effect->continuous_effect_timer = 0;
					csmemset(continuous, 0, sizeof(*continuous));
				}

				get_shake_matrix(
					random_translation_magnitude,
					random_rotation_magnitude,
					&effect_matrix);

				effect->camera_shake_time_left -= game_time_get_elapsed();

				matrix4x3_multiply(matrix, &effect_matrix, matrix);
			}
		}
		/* port: (above) */
		if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS)
		{
			camera_effect_of_tick[local_player_index].tick = game_time_get() + 1;
			camera_effect_of_tick[local_player_index].matrix = *matrix;
		}
		*local_seed = saved_local_seed;
	}

	return;
}

static void player_effect_update_camera_impulse(
	short local_player_index,
	struct player_effect_datum *effect,
	struct camera_impulse_definition const *camera_impulse,
	real_vector3d const *direction,
	real scale,
	real time_scale)
{
	real time_factor;
	real intensity;

	game_time_get();

	time_factor = time_scale * TICKS_PER_SECOND;
	intensity = effect_scale_factor(
		camera_impulse->temporary_zero_scale_factor,
		scale);

	if (effect->camera_impulse.temporary_duration > effect->camera_impulse_time_left ||
		intensity > effect->camera_impulse.temporary_zero_scale_factor ||
		(intensity >= effect->camera_impulse.temporary_zero_scale_factor &&
			effect->camera_impulse_time_left < time_factor * camera_impulse->temporary_duration))
	{
		real_vector3d flattened_direction = *direction;
		real_vector3d facing;
		real_euler_angles2d facing_angles;

		flattened_direction.k = 0.0f;
		normalize3d(&flattened_direction);

		facing_angles = *player_control_get_facing_angles(local_player_index);
		vector3d_from_euler_angles2d(&facing, &facing_angles);
		facing.k = 0.0f;
		normalize3d(&facing);

		if (realcmp(magnitude_squared3d(&flattened_direction), 1.0f) &&
			realcmp(magnitude_squared3d(&facing), 1.0f))
		{
			real angle = signed_angle_between_vectors2d(
				(real_vector2d const *)&facing,
				(real_vector2d const *)&flattened_direction);
			real jitter_lower_bound;
			real jitter_upper_bound;
			real jitter_magnitude;
			real jitter_angle;

			effect->camera_impulse = *camera_impulse;
			effect->camera_impulse.temporary_zero_scale_factor = intensity;
			effect->camera_impulse.temporary_duration =
				time_factor * effect->camera_impulse.temporary_duration;
			effect->camera_impulse_time_left =
				(short)effect->camera_impulse.temporary_duration;

			effect->direction.i = cosine(angle);
			effect->direction.j = sine(angle);
			effect->direction.k = 0.0f;

			jitter_upper_bound = effect->camera_impulse.temporary_jitter_upper_bound;
			jitter_lower_bound = effect->camera_impulse.temporary_jitter_lower_bound;
			jitter_magnitude = real_seed_random_range(
				get_global_local_random_seed_address(),
				jitter_lower_bound,
				jitter_upper_bound);
			jitter_angle = real_seed_random_range(
				get_global_local_random_seed_address(),
				0.0f,
				2 * _pi);

			cross_product3d(&effect->direction, global_up3d, &effect->jitter);
			normalize3d(&effect->jitter);
			rotate_vector_about_axis(
				&effect->jitter,
				&effect->direction,
				sine(jitter_angle),
				cosine(jitter_angle));
			scale_vector3d(&effect->jitter, jitter_magnitude, &effect->jitter);

			SET_FLAG(
				effect->flags,
				_player_effect_camera_impulse_just_started_bit,
				TRUE);
		}
	}

	{
		real permanent_intensity = effect_scale_factor(
			camera_impulse->permanent_zero_scale_factor,
			scale);
		real_vector3d facing;
		real_vector3d left;
		real_euler_angles2d delta;

		player_control_get_facing_direction(local_player_index, &facing);
		cross_product3d(global_up3d, &facing, &left);

		delta.yaw = dot_product3d(&left, direction) *
			camera_impulse->permanent_angle * permanent_intensity;
		delta.pitch = dot_product3d(&facing, direction) *
			camera_impulse->permanent_angle * permanent_intensity;

		player_control_permanent_impulse(local_player_index, &delta);
	}

	return;
}

void player_effect_start(
	long player_index,
	struct damage_data const *damage,
	real_vector3d const *direction,
	real scale,
	real total_damage)
{
	short local_player_index = player_get(player_index)->local_player_index;

	match_assert("c:\\halo\\SOURCE\\effects\\player_effects.c", 342, direction);

	lock_global_random_seed();

	if (local_player_index != NONE)
	{
		struct damage_effect_definition *definition =
			damage_effect_definition_get(damage->definition_index);
		struct player_effect_datum *effect = player_effect_get(local_player_index);

		player_effect_update_screen_flash(
			local_player_index,
			effect,
			&definition->screen_flash,
			scale,
			1.0f);
		player_effect_update_camera_impulse(
			local_player_index,
			effect,
			&definition->camera_impulse,
			direction,
			scale,
			1.0f);
		player_effect_update_camera_shake(
			local_player_index,
			effect,
			&definition->camera_shake,
			scale,
			1.0f);
		rumble_player_impulse(
			local_player_index,
			(struct rumble_definition *)&definition->vibrate,
			scale,
			1.0f);

		if (definition->sound.index != NONE)
			unspatialized_impulse_sound_new(definition->sound.index, 1.0f);

		if (total_damage > 0.0f && damage->owner_object_index != NONE)
		{
			if (TEST_FLAG(definition->damage.flags, _damage_draw_indicators_down_bit))
			{
				effect->damage_indicator_ticks[2] = 1;
			}
			else
			{
				long unit_index;
				struct unit_datum *unit;
				struct object_datum *attacker;

				if (local_player_get_player_index(local_player_index) == NONE)
					unit_index = NONE;
				else
					unit_index = player_get(
						local_player_get_player_index(local_player_index))->unit_index;

				unit = unit_try_and_get(unit_index);
				attacker = object_try_and_get(damage->owner_object_index);

				if (unit && attacker)
				{
					struct observer_result const *camera =
						observer_get_camera(local_player_index);

					if (camera)
					{
						real_point3d head_position;
						real_point3d attacker_position;
						real_vector3d delta;
						real_vector3d left;
						real_vector3d relative;

						unit_get_head_position(unit_index, &head_position);
						object_get_origin(
							damage->owner_object_index,
							&attacker_position);
						vector_from_points3d(
							&head_position,
							&attacker_position,
							&delta);
						cross_product3d(&camera->forward, &camera->up, &left);

						set_real_vector3d(&relative,
							dot_product3d(&left, &delta),
							dot_product3d(&delta, &camera->forward),
							dot_product3d(&delta, &camera->up));

						if (normalize3d(&relative) != 0.0f)
						{
							real angle;
							real absolute_angle;

							if (fabs(relative.k) > 0.5)
							{
								if (relative.k > 0.0f)
									effect->damage_indicator_ticks[0] = 1;
								else
									effect->damage_indicator_ticks[2] = 1;
							}

							angle = arctangent(relative.j, relative.i);
							absolute_angle = (real)fabs(angle);

							if (angle < _pi / 4 || angle > 3 * _pi / 4)
							{
								if (absolute_angle > _pi / 2)
									effect->damage_indicator_ticks[1] = 1;
								else
									effect->damage_indicator_ticks[3] = 1;
							}
						}
					}
				}
			}
		}
	}

	unlock_global_random_seed();

	return;
}

/* ---------- private code */

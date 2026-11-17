/*
RASTERIZER_CINEMATICS.C

symbols in this file:
0016D140 0020:
	_rasterizer_screen_effects_time (0000)
0016D160 0040:
	_rasterizer_screen_effects_initialize (0000)
0016D1A0 0030:
	_rasterizer_screen_effects_initialize_for_new_map (0000)
0016D1D0 0010:
	_rasterizer_screen_effects_dispose_from_old_map (0000)
0016D1E0 0010:
	_rasterizer_screen_effects_dispose (0000)
0016D1F0 0030:
	_rasterizer_script_screen_effect_set_value (0000)
0016D220 0030:
	_rasterizer_script_screen_effect_get_value (0000)
0016D250 0040:
	_rasterizer_screen_effect_start (0000)
0016D290 0070:
	_rasterizer_screen_effect_set_convolution (0000)
0016D300 0070:
	_rasterizer_screen_effect_set_filter (0000)
0016D370 0020:
	_rasterizer_screen_effect_set_filter_desaturation_tint (0000)
0016D390 0120:
	_rasterizer_screen_effect_set_video (0000)
0016D4B0 0010:
	_rasterizer_screen_effect_stop (0000)
0016D4C0 0250:
	_rasterizer_screen_effect_get_cinematic_parameters (0000)
0016D710 0020:
	_rasterizer_set_near_clip_distance (0000)
0016D730 0030:
	_rasterizer_get_near_clip_distance (0000)
0029D844 0020:
	??_C@_0CA@HAHKAAEK@cinematic_screen_effect_globals?$AA@ (0000)
0029D864 0032:
	??_C@_0DC@FCKAGPCD@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029D898 0014:
	??_C@_0BE@COKLOHBN@screen?5effect?5filth?$AA@ (0000)
0029D8B0 004a:
	??_C@_0EK@IJBENNIM@?$CD?$CD?$CD?5ERROR?5cinematics?5failed?5to?5s@ (0000)
0029D900 008d:
	??_C@_0IN@PCKCIAIJ@?$CD?$CD?$CD?5FATAL_ERROR?5screen?5effects?5c@ (0000)
004662F4 0004:
	_cinematic_screen_effect_globals (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps/bitmap_group.h"
#include "cseries/errors.h"
#include "game/game.h"
#include "game/game_globals.h"
#include "main/main.h"
#include "real_math.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/rasterizer_cinematics.h"
#include "rasterizer/rasterizer_globals_internal.h"
#include "saved games/game_state.h"

/* ---------- constants */

/* ---------- structures */

struct rasterizer_cinematic_screen_effect_state
{
	struct rasterizer_cinematic_screen_effect_parameters parameters;
	boolean has_control;
	boolean initialized;
	byte reserved3A[2];
	real convolution_radius[2];
	real convolution_time[2];
	real filter_light_enhancement_intensity[2];
	real filter_desaturation_intensity[2];
	real filter_time[2];
	real script_values[4];
	real near_clip_distance;
};

typedef char rasterizer_cinematic_screen_effect_parameters_tint_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, filter_desaturation_tint) == 0x14 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_parameters_video_on_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_on) == 0x23 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_parameters_video_scanline_map_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_scanline_map) == 0x28 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_parameters_video_noise_map_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_noise_map) == 0x34 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_size_assert[
	sizeof(struct rasterizer_cinematic_screen_effect_state) == 0x78 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_has_control_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, has_control) == 0x38 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_initialized_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, initialized) == 0x39 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_convolution_radius_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, convolution_radius) == 0x3C ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_filter_time_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, filter_time) == 0x5C ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_script_values_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, script_values) == 0x64 ? 1 : -1];
typedef char rasterizer_cinematic_screen_effect_state_near_clip_distance_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_state, near_clip_distance) == 0x74 ? 1 : -1];

/* ---------- globals */

static struct rasterizer_cinematic_screen_effect_state *cinematic_screen_effect_globals = NULL;

/* ---------- public code */

static real rasterizer_screen_effects_time(
	void)
{
	return (real)game_time_get() * (1.0f / TICKS_PER_SECOND);
}

void rasterizer_screen_effects_initialize(
	void)
{
	cinematic_screen_effect_globals = game_state_malloc(
		"screen effect filth",
		NULL,
		sizeof(*cinematic_screen_effect_globals));
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\rasterizer_cinematics.c",
		54,
		cinematic_screen_effect_globals);

	return;
}

void rasterizer_screen_effects_initialize_for_new_map(
	void)
{
	if (cinematic_screen_effect_globals)
	{
		csmemset(
			cinematic_screen_effect_globals,
			0,
			sizeof(*cinematic_screen_effect_globals));
		cinematic_screen_effect_globals->script_values[0] = 1.0f;
		cinematic_screen_effect_globals->script_values[1] = 1.0f;
		cinematic_screen_effect_globals->script_values[2] = 1.0f;
		cinematic_screen_effect_globals->script_values[3] = 1.0f;
	}

	return;
}

void rasterizer_screen_effects_dispose_from_old_map(
	void)
{
	return;
}

void rasterizer_screen_effects_dispose(
	void)
{
	return;
}

void rasterizer_script_screen_effect_set_value(
	word effect_index,
	real value)
{
	short signed_effect_index = (short)effect_index;

	if (cinematic_screen_effect_globals &&
		signed_effect_index >= 0 &&
		signed_effect_index < 4)
	{
		cinematic_screen_effect_globals->script_values[signed_effect_index] = value;
	}

	return;
}

real rasterizer_script_screen_effect_get_value(
	short effect_index)
{
	real value = 0.0f;

	if (cinematic_screen_effect_globals && effect_index >= 0 && effect_index < 4)
	{
		value = cinematic_screen_effect_globals->script_values[effect_index];
	}

	return value;
}

void rasterizer_screen_effect_start(
	boolean clear)
{
	struct rasterizer_cinematic_screen_effect_state *globals =
		cinematic_screen_effect_globals;

	if (globals)
	{
		if (clear || !globals->initialized)
		{
			csmemset(
				&globals->parameters,
				0,
				sizeof(globals->parameters));
			globals = cinematic_screen_effect_globals;
			globals->initialized = TRUE;
		}

		globals->has_control = TRUE;
	}

	return;
}

void rasterizer_screen_effect_set_convolution(
	short convolution_extra_passes,
	short convolution_type,
	real convolution_radius_lower_bound,
	real convolution_radius_upper_bound,
	real convolution_time)
{
	if (cinematic_screen_effect_globals)
	{
		real time;

		cinematic_screen_effect_globals->parameters.video_on = FALSE;
		cinematic_screen_effect_globals->parameters.video_overbright_mode = 0;
		cinematic_screen_effect_globals->parameters.video_scanline_map = NULL;
		cinematic_screen_effect_globals->parameters.video_noise_intensity = 0.0f;
		cinematic_screen_effect_globals->parameters.video_noise_map_scale = 0.0f;
		cinematic_screen_effect_globals->parameters.video_noise_map = NULL;
		cinematic_screen_effect_globals->parameters.convolution_extra_passes = convolution_extra_passes;
		cinematic_screen_effect_globals->parameters.convolution_type = convolution_type;
		cinematic_screen_effect_globals->convolution_radius[0] = convolution_radius_lower_bound;
		cinematic_screen_effect_globals->convolution_radius[1] = convolution_radius_upper_bound;
		time = rasterizer_screen_effects_time();
		cinematic_screen_effect_globals->convolution_time[0] = time;
		cinematic_screen_effect_globals->convolution_time[1] = time + convolution_time;
	}

	return;
}

void rasterizer_screen_effect_set_filter(
	real filter_light_enhancement_intensity_lower_bound,
	real filter_light_enhancement_intensity_upper_bound,
	real filter_desaturation_intensity_lower_bound,
	real filter_desaturation_intensity_upper_bound,
	boolean filter_desaturation_is_additive,
	real filter_time)
{
	if (cinematic_screen_effect_globals)
	{
		real time;

		cinematic_screen_effect_globals->parameters.video_on = FALSE;
		cinematic_screen_effect_globals->parameters.video_overbright_mode = 0;
		cinematic_screen_effect_globals->parameters.video_scanline_map = NULL;
		cinematic_screen_effect_globals->parameters.video_noise_intensity = 0.0f;
		cinematic_screen_effect_globals->parameters.video_noise_map_scale = 0.0f;
		cinematic_screen_effect_globals->parameters.video_noise_map = NULL;
		cinematic_screen_effect_globals->filter_light_enhancement_intensity[0] =
			filter_light_enhancement_intensity_lower_bound;
		cinematic_screen_effect_globals->filter_light_enhancement_intensity[1] =
			filter_light_enhancement_intensity_upper_bound;
		cinematic_screen_effect_globals->filter_desaturation_intensity[0] =
			filter_desaturation_intensity_lower_bound;
		cinematic_screen_effect_globals->filter_desaturation_intensity[1] =
			filter_desaturation_intensity_upper_bound;
		time = rasterizer_screen_effects_time();
		cinematic_screen_effect_globals->filter_time[0] = time;
		cinematic_screen_effect_globals->filter_time[1] = time + filter_time;
		cinematic_screen_effect_globals->parameters.filter_desaturation_is_additive =
			filter_desaturation_is_additive;
		cinematic_screen_effect_globals->parameters.filter_light_enhancement_uses_convolution_mask =
			FALSE;
		cinematic_screen_effect_globals->parameters.filter_desaturation_uses_convolution_mask =
			FALSE;
	}

	return;
}

struct rasterizer_cinematic_screen_effect_parameters *rasterizer_screen_effect_get_cinematic_parameters(
	struct rasterizer_cinematic_screen_effect_parameters *parameters)
{
	struct rasterizer_cinematic_screen_effect_parameters *result;

	result = parameters;
	if (cinematic_screen_effect_globals && cinematic_screen_effect_globals->has_control)
	{
		real convolution_fraction;
		real filter_fraction;

		if (cinematic_screen_effect_globals->convolution_time[1] !=
			cinematic_screen_effect_globals->convolution_time[0])
		{
			convolution_fraction = PIN(
				(rasterizer_screen_effects_time() - cinematic_screen_effect_globals->convolution_time[0]) /
					(cinematic_screen_effect_globals->convolution_time[1] -
						cinematic_screen_effect_globals->convolution_time[0]),
				0.0f,
				1.0f);
		}
		else
		{
			convolution_fraction = 1.0f;
		}

		if (cinematic_screen_effect_globals->filter_time[1] !=
			cinematic_screen_effect_globals->filter_time[0])
		{
			filter_fraction = PIN(
				(rasterizer_screen_effects_time() - cinematic_screen_effect_globals->filter_time[0]) /
					(cinematic_screen_effect_globals->filter_time[1] -
						cinematic_screen_effect_globals->filter_time[0]),
				0.0f,
				1.0f);
		}
		else
		{
			filter_fraction = 1.0f;
		}

		scalars_interpolate(
			cinematic_screen_effect_globals->convolution_radius[0],
			cinematic_screen_effect_globals->convolution_radius[1],
			convolution_fraction,
			&cinematic_screen_effect_globals->parameters.convolution_radius);
		scalars_interpolate_and_clamp_0_to_1(
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[0],
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[1],
			filter_fraction,
			&cinematic_screen_effect_globals->parameters.filter_light_enhancement_intensity);
		scalars_interpolate_and_clamp_0_to_1(
			cinematic_screen_effect_globals->filter_desaturation_intensity[0],
			cinematic_screen_effect_globals->filter_desaturation_intensity[1],
			filter_fraction,
			&cinematic_screen_effect_globals->parameters.filter_desaturation_intensity);

		if (csmemcmp(
			&cinematic_screen_effect_globals->parameters.filter_desaturation_tint,
			global_real_rgb_black,
			sizeof(cinematic_screen_effect_globals->parameters.filter_desaturation_tint)) == 0)
		{
			parameters = &cinematic_screen_effect_globals->parameters;
			parameters->filter_desaturation_tint = *global_real_rgb_green;
		}
		else
		{
			parameters = &cinematic_screen_effect_globals->parameters;
		}

		if (parameters->convolution_radius <= _real_epsilon)
		{
			parameters->convolution_radius = 0.0f;
			parameters->convolution_type = 0;
			parameters->convolution_extra_passes = 0;
		}
		else
		{
			parameters = &cinematic_screen_effect_globals->parameters;
			/* port: the blur and the video effect only work with one window.
			In split screen the views get the effect without them, instead of the Xbox's fatal error.
			This happens with a script's effect outside a cutscene,
			or with a co-op client's effect one tick before its cutscene switches to one window. */
			if (main_get_window_count() > 1)
			{
				static struct rasterizer_cinematic_screen_effect_parameters split_screen_parameters;

				split_screen_parameters = *parameters;
				split_screen_parameters.convolution_radius = 0.0f;
				split_screen_parameters.convolution_type = 0;
				split_screen_parameters.convolution_extra_passes = 0;
				split_screen_parameters.video_on = FALSE;
				parameters = &split_screen_parameters;
			}
		}

		if (parameters->filter_light_enhancement_intensity <= _real_epsilon &&
			parameters->filter_desaturation_intensity <= _real_epsilon &&
			filter_fraction >= 1.0f)
		{
			parameters->filter_light_enhancement_intensity = 0.0f;
			parameters->filter_desaturation_intensity = 0.0f;
		}

		return parameters;
	}

	return result;
}

void rasterizer_screen_effect_set_filter_desaturation_tint(
	real red,
	real green,
	real blue)
{
	struct rasterizer_cinematic_screen_effect_state *globals =
		cinematic_screen_effect_globals;

	if (globals)
	{
		globals->parameters.filter_desaturation_tint.red = red;
		globals->parameters.filter_desaturation_tint.green = green;
		globals->parameters.filter_desaturation_tint.blue = blue;
	}

	return;
}

void rasterizer_screen_effect_set_video(
	short video_overbright_mode,
	real video_noise_intensity)
{
	if (cinematic_screen_effect_globals)
	{
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\rasterizer_cinematics.c",
			225,
			global_rasterizer_data);

		if (global_rasterizer_data->screen_effect_video_scanline_map.index != NONE &&
			global_rasterizer_data->screen_effect_video_noise_map.index != NONE)
		{
			csmemset(
				&cinematic_screen_effect_globals->parameters,
				0,
				sizeof(cinematic_screen_effect_globals->parameters));
			cinematic_screen_effect_globals->convolution_radius[0] = 0.0f;
			cinematic_screen_effect_globals->convolution_radius[1] = 0.0f;
			cinematic_screen_effect_globals->convolution_time[0] = 0.0f;
			cinematic_screen_effect_globals->convolution_time[1] = 0.0f;
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[0] = 0.0f;
			cinematic_screen_effect_globals->filter_light_enhancement_intensity[1] = 0.0f;
			cinematic_screen_effect_globals->filter_desaturation_intensity[0] = 0.0f;
			cinematic_screen_effect_globals->filter_desaturation_intensity[1] = 0.0f;
			cinematic_screen_effect_globals->filter_time[0] = 0.0f;
			cinematic_screen_effect_globals->filter_time[1] = 0.0f;
			cinematic_screen_effect_globals->parameters.video_on = TRUE;
			cinematic_screen_effect_globals->parameters.video_overbright_mode = video_overbright_mode;
			cinematic_screen_effect_globals->parameters.video_scanline_map = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group_get(
					global_rasterizer_data->screen_effect_video_scanline_map.index)->bitmaps,
				0,
				struct bitmap_data);
			cinematic_screen_effect_globals->parameters.video_noise_intensity = video_noise_intensity;
			cinematic_screen_effect_globals->parameters.video_noise_map_scale = 1.0f;
			cinematic_screen_effect_globals->parameters.video_noise_map = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group_get(
					global_rasterizer_data->screen_effect_video_noise_map.index)->bitmaps,
				0,
				struct bitmap_data);
		}
		else
		{
			error(
				_error_silent,
				"### ERROR cinematics failed to set video mode; global bitmaps are not set");
		}
	}

	return;
}

void rasterizer_screen_effect_stop(
	void)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->has_control = FALSE;
	}

	return;
}

void rasterizer_set_near_clip_distance(
	real near_clip_distance)
{
	if (cinematic_screen_effect_globals)
	{
		cinematic_screen_effect_globals->near_clip_distance = near_clip_distance;
	}

	return;
}

real rasterizer_get_near_clip_distance(
	void)
{
	real near_clip_distance = rasterizer_global_defaults.near_clip_distance;
	struct rasterizer_cinematic_screen_effect_state *globals =
		cinematic_screen_effect_globals;

	if (globals && globals->near_clip_distance > 0.0f)
	{
		near_clip_distance = globals->near_clip_distance;
	}

	return near_clip_distance;
}

/* port: the state for network co-op (rasterizer_cinematics.h) */
void rasterizer_screen_effect_port_get(
	struct rasterizer_screen_effect_port_state *state)
{
	struct rasterizer_cinematic_screen_effect_state const *globals = cinematic_screen_effect_globals;

	csmemset(state, 0, sizeof(*state));
	if (!globals)
		return;
	state->has_control = globals->has_control;
	state->initialized = globals->initialized;
	state->video_on = globals->parameters.video_on;
	state->filter_desaturation_is_additive = globals->parameters.filter_desaturation_is_additive;
	state->filter_light_enhancement_uses_convolution_mask =
		globals->parameters.filter_light_enhancement_uses_convolution_mask;
	state->filter_desaturation_uses_convolution_mask = globals->parameters.filter_desaturation_uses_convolution_mask;
	state->convolution_extra_passes = globals->parameters.convolution_extra_passes;
	state->convolution_type = globals->parameters.convolution_type;
	state->video_overbright_mode = globals->parameters.video_overbright_mode;
	state->filter_desaturation_tint = globals->parameters.filter_desaturation_tint;
	state->video_noise_intensity = globals->parameters.video_noise_intensity;
	csmemcpy(state->convolution_radius, globals->convolution_radius, sizeof(state->convolution_radius));
	csmemcpy(state->convolution_time, globals->convolution_time, sizeof(state->convolution_time));
	csmemcpy(state->filter_light_enhancement_intensity, globals->filter_light_enhancement_intensity,
		sizeof(state->filter_light_enhancement_intensity));
	csmemcpy(state->filter_desaturation_intensity, globals->filter_desaturation_intensity,
		sizeof(state->filter_desaturation_intensity));
	csmemcpy(state->filter_time, globals->filter_time, sizeof(state->filter_time));
	csmemcpy(state->script_values, globals->script_values, sizeof(state->script_values));
	state->near_clip_distance = globals->near_clip_distance;
}

/* port: the first bitmap of one of the video effect's bitmap groups, as
rasterizer_screen_effect_set_video uses it */
static struct bitmap_data *screen_effect_video_bitmap(
	long bitmap_group_index)
{
	return TAG_BLOCK_GET_ELEMENT(&bitmap_group_get(bitmap_group_index)->bitmaps, 0, struct bitmap_data);
}

/* port: the host's state put in place on a client, with the video effect's
bitmaps found on this machine */
void rasterizer_screen_effect_port_set(
	struct rasterizer_screen_effect_port_state const *state)
{
	struct rasterizer_cinematic_screen_effect_state *globals = cinematic_screen_effect_globals;
	boolean video = state->video_on && global_rasterizer_data &&
		global_rasterizer_data->screen_effect_video_scanline_map.index != NONE &&
		global_rasterizer_data->screen_effect_video_noise_map.index != NONE;

	if (!globals)
		return;
	globals->has_control = state->has_control;
	globals->initialized = state->initialized;
	globals->parameters.convolution_extra_passes = state->convolution_extra_passes;
	globals->parameters.convolution_type = state->convolution_type;
	globals->parameters.convolution_mask = NULL;
	globals->parameters.filter_desaturation_tint = state->filter_desaturation_tint;
	globals->parameters.filter_desaturation_is_additive = state->filter_desaturation_is_additive;
	globals->parameters.filter_light_enhancement_uses_convolution_mask =
		state->filter_light_enhancement_uses_convolution_mask;
	globals->parameters.filter_desaturation_uses_convolution_mask = state->filter_desaturation_uses_convolution_mask;
	globals->parameters.video_on = video;
	globals->parameters.video_overbright_mode = state->video_overbright_mode;
	globals->parameters.video_noise_intensity = state->video_noise_intensity;
	globals->parameters.video_noise_map_scale = video ? 1.0f : 0.0f;
	globals->parameters.video_scanline_map =
		video ? screen_effect_video_bitmap(global_rasterizer_data->screen_effect_video_scanline_map.index) : NULL;
	globals->parameters.video_noise_map =
		video ? screen_effect_video_bitmap(global_rasterizer_data->screen_effect_video_noise_map.index) : NULL;
	csmemcpy(globals->convolution_radius, state->convolution_radius, sizeof(globals->convolution_radius));
	csmemcpy(globals->convolution_time, state->convolution_time, sizeof(globals->convolution_time));
	csmemcpy(globals->filter_light_enhancement_intensity, state->filter_light_enhancement_intensity,
		sizeof(globals->filter_light_enhancement_intensity));
	csmemcpy(globals->filter_desaturation_intensity, state->filter_desaturation_intensity,
		sizeof(globals->filter_desaturation_intensity));
	csmemcpy(globals->filter_time, state->filter_time, sizeof(globals->filter_time));
	csmemcpy(globals->script_values, state->script_values, sizeof(globals->script_values));
	globals->near_clip_distance = state->near_clip_distance;
}

/* ---------- private code */

/*
CINEMATICS.C

symbols in this file:
000813F0 0040:
	_cinematic_initialize (0000)
00081430 0010:
	_cinematic_dispose (0000)
00081440 0030:
	_cinematic_initialize_for_new_map (0000)
00081470 0020:
	_cinematic_dispose_from_old_map (0000)
00081490 0040:
	_cinematic_start (0000)
000814D0 0010:
	_cinematic_can_be_skipped (0000)
000814E0 0010:
	_cinematic_skip_start (0000)
000814F0 0010:
	_cinematic_skip_stop (0000)
00081500 0030:
	_cinematic_show_letterbox (0000)
00081530 0150:
	_draw_quad (0000)
00081680 0020:
	_cinematic_force_title (0000)
000816A0 0020:
	_cinematic_suppress_bsp_object_creation (0000)
000816C0 0050:
	_cinematic_stop (0000)
00081710 0010:
	_cinematic_in_progress (0000)
00081720 0090:
	_cinematic_set_title_delayed (0000)
000817B0 0500:
	_cinematic_render (0000)
00081CB0 0020:
	_cinematic_set_title (0000)
002589FC 0012:
	??_C@_0BC@NGABDHNK@cinematic_globals?$AA@ (0000)
00258A10 0025:
	??_C@_0CF@PIGNLDBH@c?3?2halo?2SOURCE?2cutscene?2cinemati@ (0000)
00258A38 0012:
	??_C@_0BC@PJBLDLNO@cinematic?5globals?$AA@ (0000)
00258A4C 0032:
	??_C@_0DC@GMMMKPOA@no?5free?5chapter?5title?5slots?5to?5d@ (0000)
00258A80 0004:
	__real@3e000000 (0000)
00435CA0 0004:
	_cinematic_globals (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries/errors.h"
#include "cinematics.h"
#include "ai/ai.h"
#include "bitmaps/bitmaps.h"
#include "bitmaps/bitmap_group.h"
#include "editor/editor_stubs.h"
#include "game/game.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "interface/hud.h"
#include "interface/hud_definitions.h"
#include "interface/ui_widget.h"
#include "items/projectiles.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_cinematics.h"
#include "render/render.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "text/draw_string.h"
#include "text/text_group.h"

/* ---------- constants */

enum
{
	MAXIMUM_QUEUED_CINEMATIC_TITLES = 4,
};

/* ---------- macros */

/* ---------- structures */

struct scenario_cutscene_title
{
	long flags;
	char name[TAG_STRING_LENGTH+1];
	long pad24;
	rectangle2d bounds;
	short text_index;
	word style;
	word justification;
	word pad36;
	unsigned long text_flags;
	pixel32 foreground_color;
	pixel32 shadow_color;
	real fade_in_time;
	real up_time;
	real fade_out_time;
	byte unused50[0x10];
};

typedef char verify_scenario_cutscene_title_size[
	sizeof(struct scenario_cutscene_title) == 0x60 ? 1 : -1];
typedef char verify_scenario_cutscene_title_name_offset[
	offsetof(struct scenario_cutscene_title, name) == 0x04 ? 1 : -1];
typedef char verify_scenario_cutscene_title_bounds_offset[
	offsetof(struct scenario_cutscene_title, bounds) == 0x28 ? 1 : -1];
typedef char verify_scenario_cutscene_title_fade_offset[
	offsetof(struct scenario_cutscene_title, fade_in_time) == 0x44 ? 1 : -1];
typedef char verify_hud_global_single_player_font_offset[
	offsetof(struct hud_globals_definition, messaging.single_player_font.index) == 0x54 ? 1 : -1];
typedef char verify_hud_global_default_title_bounds_offset[
	offsetof(struct hud_globals_definition, defaults.default_title_bounds) == 0x2DC ? 1 : -1];

/* ---------- prototypes */

/* ---------- globals */

struct cinematic_global_data *cinematic_globals = NULL;

/* ---------- public code */

void cinematic_initialize(
	void)
{
	cinematic_globals = game_state_malloc(
		"cinematic globals",
		NULL,
		sizeof(*cinematic_globals));
	match_assert(
		"c:\\halo\\SOURCE\\cutscene\\cinematics.c",
		24,
		cinematic_globals);

	return;
}

void cinematic_dispose(
	void)
{
	return;
}

void cinematic_initialize_for_new_map(
	void)
{
	csmemset(cinematic_globals, 0, sizeof(*cinematic_globals));
	csmemset(
		cinematic_globals->queued_titles,
		NONE,
		sizeof(cinematic_globals->queued_titles));

	return;
}

void cinematic_skip_start(
	void)
{
	cinematic_globals->can_be_skipped = TRUE;

	return;
}

void cinematic_skip_stop(
	void)
{
	cinematic_globals->can_be_skipped = FALSE;

	return;
}

void cinematic_dispose_from_old_map(
	void)
{
	cinematic_globals->show_letterbox = FALSE;
	cinematic_globals->in_progress = FALSE;

	return;
}

void cinematic_start(
	void)
{
	player_input_enable(FALSE);
	ai_globals_dialogue_triggers_enabled(FALSE);
	cinematic_globals->show_letterbox = TRUE;
	cinematic_globals->letterbox_last_game_time = game_time_get();
	cinematic_globals->in_progress = TRUE;
	projectiles_delete_all();

	return;
}

boolean cinematic_can_be_skipped(
	void)
{
	return cinematic_globals->can_be_skipped;
}

void cinematic_show_letterbox(
	boolean show)
{
	cinematic_globals->show_letterbox = show;
	if (show)
	{
		cinematic_globals->letterbox_last_game_time = game_time_get();
	}

	return;
}

void cinematic_force_title(
	unsigned short title_index)
{
	cinematic_globals->queued_titles[0].title_index = title_index;
	cinematic_globals->queued_titles[0].time = 0;

	return;
}

void cinematic_suppress_bsp_object_creation(
	boolean suppress)
{
	cinematic_globals->suppress_bsp_object_creation = suppress;

	return;
}

boolean cinematic_in_progress(
	void)
{
	return cinematic_globals->in_progress;
}

void cinematic_stop(
	void)
{
	cinematic_globals->show_letterbox = FALSE;
	player_input_enable(TRUE);
	ai_globals_dialogue_triggers_enabled(TRUE);
	cinematic_globals->in_progress = FALSE;
	rasterizer_screen_effects_initialize_for_new_map();
	if (global_rasterizer_model_ambient_reflection_tint)
	{
		csmemset(
			global_rasterizer_model_ambient_reflection_tint,
			0,
			sizeof(*global_rasterizer_model_ambient_reflection_tint));
	}
	rasterizer_set_near_clip_distance(0.0f);
	display_errors_deferred_until_cinematic_stop();

	return;
}

void cinematic_set_title_delayed(
	short title_index,
	real delay)
{
	short title_slot_index;

	for (title_slot_index = 0;
		title_slot_index < MAXIMUM_QUEUED_CINEMATIC_TITLES &&
		cinematic_globals->queued_titles[title_slot_index].title_index != NONE;
		title_slot_index++);

	if (title_slot_index < MAXIMUM_QUEUED_CINEMATIC_TITLES)
	{
		cinematic_globals->queued_titles[title_slot_index].title_index = title_index;
		cinematic_globals->queued_titles[title_slot_index].time =
			(short)-fast_ftol(delay * TICKS_PER_SECOND);
	}
	else
	{
		struct scenario_cutscene_title *title = TAG_BLOCK_GET_ELEMENT(
			&global_scenario_get()->cutscene_chapter_titles,
			title_index,
			struct scenario_cutscene_title);
		error(
			_error_silent,
			"no free chapter title slots to display title '%s'",
			title->name);
	}

	return;
}

void cinematic_render(
	void)
{
	if ((cinematic_globals->show_letterbox ||
		cinematic_globals->letterbox_amount > 0.0f) &&
		!ui_widgets_active())
	{
		long game_time;
		long elapsed_ticks;
		real letterbox_amount;

		game_time = game_time_get();
		elapsed_ticks =
			game_time - cinematic_globals->letterbox_last_game_time;
		cinematic_globals->letterbox_last_game_time = game_time;
		if (cinematic_globals->show_letterbox)
		{
			cinematic_globals->letterbox_amount +=
				(real)elapsed_ticks * (1.0f / (real)TICKS_PER_SECOND);
			letterbox_amount =
				MIN(cinematic_globals->letterbox_amount, 1.0f);
		}
		else
		{
			cinematic_globals->letterbox_amount =
				cinematic_globals->letterbox_amount -
				(real)elapsed_ticks * (1.0f / (real)TICKS_PER_SECOND);
			letterbox_amount =
				MAX(cinematic_globals->letterbox_amount, 0.0f);
		}

		cinematic_globals->letterbox_amount = letterbox_amount;

		if (cinematic_globals->letterbox_amount > 0.0f)
		{
			rectangle2d bar;
			real bar_height;
			real viewport_height;

			bar_height = cinematic_globals->letterbox_amount * 0.125f;
			viewport_height = (real)(
				render.camera.viewport_bounds.y1 -
				render.camera.viewport_bounds.y0);

			bar.x0 = (short)fast_ftol((real)render.camera.viewport_bounds.x0);
			bar.x1 = (short)fast_ftol((real)render.camera.viewport_bounds.x1);
			bar.y0 = (short)fast_ftol((real)render.camera.viewport_bounds.y0);
			bar_height *= viewport_height;
			bar.y1 = (short)fast_ftol(
				(real)render.camera.viewport_bounds.y0 + bar_height);
			draw_quad(&bar, 0xFF000000);

			bar.x0 = (short)fast_ftol((real)render.camera.viewport_bounds.x0);
			bar.x1 = (short)fast_ftol((real)render.camera.viewport_bounds.x1);
			bar.y0 = (short)fast_ftol(
				(real)render.camera.viewport_bounds.y1 - bar_height);
			bar.y1 = (short)fast_ftol((real)render.camera.viewport_bounds.y1);
			draw_quad(&bar, 0xFF000000);
		}
	}

	{
		short title_slot_index;

		for (title_slot_index = 0;
			title_slot_index < MAXIMUM_QUEUED_CINEMATIC_TITLES;
			title_slot_index++)
		{
			struct cinematic_title *active_title;
			struct scenario_cutscene_title *title;
			struct string_list *string_list;
			rectangle2d const *title_bounds;
			long font_index;
			long help_text_tag_index;
			real fade_amount;

			active_title =
				&cinematic_globals->queued_titles[title_slot_index];

			if (active_title->title_index == NONE)
				continue;

			font_index = hud_globals->messaging.single_player_font.index;
			if (font_index == NONE)
				continue;

			title = TAG_BLOCK_GET_ELEMENT(
				&global_scenario_get()->cutscene_chapter_titles,
				active_title->title_index,
				struct scenario_cutscene_title);

			help_text_tag_index =
				global_scenario_get()->ingame_help_text.index;
			if (help_text_tag_index == NONE)
				continue;

			string_list =
				unicode_string_list_definition_get(help_text_tag_index);
			if (title->text_index < 0 ||
				title->text_index >= string_list->strings.count)
			{
				continue;
			}

			title_bounds = &title->bounds;
			fade_amount = 1.0f;
			if (title_bounds->x1 == title_bounds->x0 ||
				title_bounds->y1 == title_bounds->y0)
			{
				title_bounds = &hud_globals->defaults.default_title_bounds;
			}

			if (!game_in_editor())
			{
				real title_time = (real)active_title->time;

				if (title_time < title->fade_in_time)
				{
					fade_amount =
						title_time / title->fade_in_time;
				}
				else if (title_time > title->up_time)
				{
					fade_amount =
						1.0f -
						(title_time - title->up_time) /
						title->fade_out_time;
				}

				fade_amount = PIN(fade_amount, 0.0f, 1.0f);
			}

			{
				real_argb_color text_color;

				pixel32_to_real_argb_color(
					title->foreground_color,
					&text_color);
				text_color.alpha *= fade_amount;

				if (fabs(text_color.red - 1.0f) < _real_epsilon &&
					fabs(text_color.green - 1.0f) < _real_epsilon &&
					fabs(text_color.blue - 1.0f) < _real_epsilon)
				{
					text_color.red = MIN(text_color.red, 0.8f);
					text_color.green = MIN(text_color.green, 0.8f);
					text_color.blue = MIN(text_color.blue, 0.8f);
				}

				draw_string_set_draw_mode(
					font_index,
					title->style - 1,
					title->justification,
					title->text_flags,
					&text_color);

				rasterizer_text_set_shadow_color(
					((pixel32)PIN(
						fast_ftol(
							(real)(long)(byte)(title->shadow_color >> 24) *
							fade_amount),
						0,
						255) << 24) |
					(title->shadow_color & 0x00FFFFFF));

				rasterizer_draw_unicode_string(
					title_bounds,
					NULL,
					NULL,
					0,
					unicode_string_list_get_string(
						help_text_tag_index,
						title->text_index));

				rasterizer_text_set_shadow_color(0);
			}

			active_title->time += game_time_get_paused()
				? 0
				: game_time_get_elapsed();

			if (!game_in_editor() &&
				(real)active_title->time >=
					title->up_time + title->fade_out_time)
			{
				active_title->title_index = NONE;
				active_title->time = NONE;
			}
		}
	}

	return;
}

void cinematic_set_title(
	unsigned short title_index)
{
	cinematic_set_title_delayed(title_index, 0.0f);

	return;
}

/* ---------- private code */

void draw_quad(
	rectangle2d *rectangle,
	pixel32 color)
{
	struct bitmap_data *map;
	real_point2d positions[4];
	struct rasterizer_dynamic_screen_geometry_parameters parameters;
	struct dynamic_screen_vertex vertices[4];
	real_point2d *position;
	struct dynamic_screen_vertex *vertex;
	short vertex_index;
	struct game_globals *game_globals;
	struct game_globals_rasterizer_data *rasterizer_data;

	global_scenario_get();
	game_globals = scenario_get_game_globals();
	rasterizer_data = game_globals->rasterizer_data.count
		? TAG_BLOCK_GET_ELEMENT(
			&game_globals->rasterizer_data,
			0,
			struct game_globals_rasterizer_data)
		: NULL;
	map = TAG_BLOCK_GET_ELEMENT(
		&bitmap_group_get(rasterizer_data->default_textures[0].index)->bitmaps,
		1,
		struct bitmap_data);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_cinematics;

	positions[0].x = (real)rectangle->x0;
	positions[0].y = (real)rectangle->y0;
	positions[1].x = (real)rectangle->x1;
	positions[1].y = (real)rectangle->y0;
	positions[2].x = (real)rectangle->x1;
	positions[2].y = (real)rectangle->y1;
	positions[3].x = (real)rectangle->x0;
	positions[3].y = (real)rectangle->y1;

	position = positions;
	vertex = vertices;
	for (vertex_index = 0; vertex_index < NUMBEROF(vertices); vertex_index++)
	{
		vertex->position.x = position->x;
		vertex->color = color;
		vertex->texture_coordinates.x = 0.0f;
		vertex->texture_coordinates.y = 0.0f;
		vertex->position.y = position->y;
		position++;
		vertex++;
	}

	csmemset(&parameters, 0, sizeof(parameters));
	parameters.framebuffer_blend_function = 0;
	parameters.map_texture_scale[0].j = 1.0f;
	parameters.map_texture_scale[0].i = 1.0f;
	parameters.map_scale[0].j = 1.0f;
	parameters.map_scale[0].i = 1.0f;
	parameters.meter_parameters = NULL;
	parameters.point_sampled = FALSE;
	parameters.map[0] = map;

	rasterizer_psuedo_dynamic_screen_quad_draw(&parameters, vertices);

	rasterizer_globals.current_lock_operation = _rasterizer_lock_none;

	return;
}

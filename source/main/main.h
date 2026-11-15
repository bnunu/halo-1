/*
MAIN.H
*/

#ifndef __MAIN_H
#define __MAIN_H
#pragma once

/* ---------- headers */

#include "math/integer_math.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct observer_result;
struct render_window;

/* ---------- prototypes/MAIN.C */

float main_get_seconds_elapsed(
	void);

boolean gamepad_button_is_down(
	short button_index);

void set_window_camera_values(
	struct render_window *window,
	struct observer_result const *observer);

void game_connection_set(
	short connection);

void main_disallow_persistent_storage(
	void);

void main_defer_map_map_change(
	void);

char *main_get_map_name(
	void);

char *main_get_multiplayer_map_name(
	void);

short main_get_difficulty(
	void);

boolean main_saving_map(
	void);

void main_save_cancel(
	void);

void main_won_map(
	void);

/* port: a campaign map's own script naming the level that comes next
(hs.c's map_name) */
void main_script_map_name(
	char const *map_name);

void main_lost_map(
	void);

void main_revert_map(
	void);

void main_skip_cinematic(
	void);

void main_save_map_nonsafe(
	void);

void main_respawn(
	boolean in_multiplayer);

void main_save_core(
	void);

void main_save_core_name(
	char const *core_name);

void main_load_core(
	void);

void main_load_core_at_startup(
	void);

void main_load_core_name(
	char const *core_name);

void main_load_core_name_at_startup(
	char const *core_name);

void main_switch_structure_bsp(
	short structure_bsp_index);

void main_goto_main_menu(
	void);

void main_menu_precache_resources(
	void);

void main_menu_unload(
	void);

void main_menu_ensure_player_queues_exist(
	void);

boolean main_menu_fade_active(
	void);

void main_menu_switch_to_single_player(
	void);

void main_set_game_connection_to_film_playback(
	void);

short main_get_current_solo_level(
	void);

char const *main_get_solo_level_name(
	short level);

void main_run_demos(
	void);

void compute_window_bounds(
	long player_index,
	long num_players,
	rectangle2d *pixel_bounds,
	rectangle2d *safe_frame_bounds);

short main_get_window_count(
	void);

void main_set_multiplayer_map_name(
	char const *map_name);

void main_set_map_name(
	char const *map_name);

void main_set_difficulty(
	short difficulty);

void main_save_map_safe(
	void);

void main_save_map_no_timeout(
	void);

void main_skip(
	short ticks);

void main_queue_map_name(
	char const *map_name);

boolean main_taking_screenshot(
	void);

void main_movie_start(
	float frames_per_second);

void main_movie_stop(
	void);

void main_save_current_solo_map(
	char const *map_name);

void main_load_ui_scenario(
	boolean precache_resources);

void main_menu_load(
	void);

void main_present_frame(
	void);

void main_pregame_render(
	void);

void main_framerate_render(
	void);

void main_loop_of_death(
	void);

void main_rasterizer_throttle(
	void);

void main_print_version(
	void);

void main_roll_credits(
	void);

void main_reset_map(void);

void main_stop_time(void);
void main_start_time(void);

void main_crash(
	char const *str);

void main_vertical_blank_interrupt_handler(
	unsigned long);

void main_loop(void);

/* ---------- globals */

extern short player_spawn_count;
extern boolean global_frame_rate_throttle;
extern short global_screenshot_size;
extern boolean debug_force_frame_rate_update;
extern boolean debug_no_drawing;
extern boolean debug_game_save;
extern boolean debug_frame_rate;
extern boolean display_framerate;
extern boolean display_vblank_deltas;
extern boolean display_precache_progress;

/* ---------- public code */

#endif // __MAIN_H

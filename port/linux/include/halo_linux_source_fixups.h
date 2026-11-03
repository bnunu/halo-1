/*
HALO_LINUX_SOURCE_FIXUPS.H

The port's functions that the game's sources call, declared for them.
*/

#ifndef __HALO_LINUX_SOURCE_FIXUPS_H
#define __HALO_LINUX_SOURCE_FIXUPS_H

/* frames between the 30 Hz ticks (port/linux/game/render_interpolation.c);
the platform layer reads the display.interpolation setting */
struct observer_result;
struct render_camera;
struct real_matrix4x3;
union real_point3d;
int halo_interpolation_enabled(void);
float game_time_get_tick_fraction(void);
void render_interpolation_tick(void);
void render_interpolation_reset(void);
void render_interpolation_frame_begin(void);
void render_interpolation_frame_end(void);
float render_interpolation_fraction(void);
struct real_matrix4x3 *render_interpolation_object_node_matrices(long object_index);
/* a point on an object (a nav marker's), moved to where the frame draws it */
void render_interpolation_object_point(long object_index, union real_point3d *point);
struct observer_result const *render_interpolation_camera(short local_player_index,
	struct observer_result const *observer);
void render_interpolation_first_person(short local_player_index, struct real_matrix4x3 *node_matrices,
	short node_count, struct render_camera const *camera);
float render_interpolation_game_time_sec(long ticks);

/* whether the map being loaded, by the name it was loaded by, is a
downloaded one, whose scripts may call and set less: one map sharing
downloaded, or one game.downloaded_maps names (port/linux/game/map_share.c) */
int map_share_map_downloaded(char const *map_name);

/* an IP address as the log may show it: a public one as a tag of the run's,
never whole (port/linux/src/log_address.c, log_address.h) */
const char *log_address(const unsigned char *bytes, int length, int port, char *text, int size);

/* the port's menus' screen for one of ui.map's, by name, else the name
(port/linux/game/menu_tags.c) */
char const *pc_menus_screen(char const *name);
/* debug.fixed_step: a game of this machine alone runs a tick a frame
(port/linux/src/port_config.c; game_time.c, main.c) */
int halo_fixed_step(void);
/* the Custom Edition tag cache window, or NULL unless the
game.custom_edition setting reserved it (port/linux/src/xbox_memory.c) */
void *halo_custom_edition_tag_cache(void);
/* where Halo PC keeps the channels of the pixels a Custom Edition bitmap
just arrived at (an enum custom_edition_channel_order,
port/linux/game/cache_file_formats.h), which the renderer then samples in
this build's order; forgotten together when the map goes
(port/linux/src/xbox_textures.c) */
void halo_custom_edition_texels_channels(const void *texels, unsigned char channel_order);
void halo_custom_edition_texels_forget(void);
void halo_custom_edition_hud_meter(int active);
/* whether a Halo Custom Edition map's multiplayer vehicles are chosen by
their placements' spawn flags, as in retail Halo, and whether a vehicle
placement is placed in the running game
(port/linux/game/custom_edition_objects.c) */
struct scenario_object_datum;
unsigned char custom_edition_vehicles_by_placement(void);
unsigned char custom_edition_vehicle_placement_allowed(struct scenario_object_datum const *placement);

/* the width of the screen the game draws, 480 lines tall: the device's or
the display's shape, or 640 (port/linux/src/d3d8_gl.c) */
long halo_screen_width(void);
/* the screen's pixels to the Xbox's one (port/linux/src/d3d8_gl.c) */
float halo_screen_scale(void);
/* takes up a new width between frames (F11); returns the width */
long halo_screen_commit(void);
/* the shadow maps' pixels for each of their 128 texels each way, a power of
two (display.shadow_resolution) */
long halo_shadow_map_scale(void);
/* how many times a model's size in the 480-line screen's pixels to measure
it at (display.model_detail, source/models/models.c): level nonzero, to
choose its level of detail, otherwise whether it is drawn */
float halo_model_detail_scale(int level);
/* whether display.model_detail is "stock" (port/linux/src/d3d8_gl.c), for
each permutation's level (port/linux/game/model_detail.c) */
int halo_model_detail_stock(void);
/* while TRUE, drawing shifts right to center 640-column layouts */
void halo_screen_ui_offset(unsigned char centered);
/* the same for a player's view of a split screen (its width): a half
screen's 640 columns are centred as the screen's, a quarter's 320 in the
quarter; the shift in effect */
void halo_screen_ui_offset_view(long view_width);
long halo_screen_ui_offset_current(void);
/* (port/linux/game/menu_functions.c, for source/interface/ui_widget.c) the
network lobby, which reads every controller's events however many players
there are: another controller's START joins it, a player's B leaves it (its
handlers know each controller), where a screen of player 1's would read
theirs alone; and ADD PLAYER's screen, whose quarters are each a
controller's: the screen takes every controller's events, its widgets their
own controller's alone */
struct widget_instance;
unsigned char pc_menu_screen_reads_every_controller(struct widget_instance const *widget);
unsigned char pc_menu_screen_routes_by_controller(struct widget_instance const *widget);
/* names a model lighting vertex program, whose draws can be lit for each
pixel (port/linux/src/d3d8_gl.c, display.per_pixel_lighting) */
void halo_vertex_shader_lighting(unsigned long handle);
/* display.anti_aliasing's pass over a window's 3D view, before the HUD and
menus (source/render/render.c): the window's bounds on the screen */
void halo_screen_anti_alias(short x0, short y0, short x1, short y1);
/* display.hdr's tone mapping of a window's 3D view, before its
anti-aliasing (source/render/render.c) */
void halo_screen_tone_map(short x0, short y0, short x1, short y1);
/* network.client_timeout_seconds, in milliseconds (port/linux/src/port_config.c) */
unsigned long network_client_timeout_milliseconds(void);
/* a dedicated server (halo --server: port/linux/src/server_mode.c,
port/linux/game/server_director.c; docs/dedicated_server.md): whether this
run is one; its start, before the game's (shell_xbox.c); its frames' pace
(main.c); the game's fatal error ended with (main.c); and the console's
output, which it logs and captures for its commands' replies (terminal.c) */
int halo_server_mode(void);
void server_mode_start(void);
void server_mode_pace_frame(void);
void server_fatal(char const *text);
void server_console_output(const char *text);
/* ... whether debug.txt's lines go to the standard error too
(server.log_debug_echo: errors.c) */
int halo_server_debug_echo(void);
/* ... the level its rotation plays next, NULL if none (game_engine.c) */
char const *server_director_first_level(void);
/* the deflection of a stick (0 the left, 1 the right) read as none, of
32767: input.left_stick_dead_zone and input.right_stick_dead_zone, else the
game's own (port/linux/src/xinput_sdl.c) */
short halo_stick_dead_range(short stick);
/* the mouse in the menus (source/interface/ui_widget.c) */
#include "halo_ui_pointer.h"
/* the debug camera's mouse and keys (source/camera/director.c): whether the
middle button, R and F are held, and the wheel's notches since the last call
(port/linux/src/xinput_sdl.c) */
int halo_linux_debug_camera_input(int *middle, int *up, int *down, float *wheel);

#endif

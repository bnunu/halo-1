/*
RASTERIZER.H

header included in hcex build.
*/

#ifndef __RASTERIZER_H
#define __RASTERIZER_H
#pragma once

/* ---------- headers */

#include "real_math.h"
#include "render_cameras.h"

/* ---------- constants */

enum
{
	MAXIMUM_WINDOWS = 4,
	MAXIMUM_LENS_FLARES_PER_FRAME = 1024,
	MAXIMUM_LIGHTS_PER_WINDOW = 128,
};

enum
{
	/* port: the Xbox pool (0x18000) held a frame of 1024 particles. The
	native particle pool is larger, and a full co-op Library spends the Xbox
	pool before the frame is drawn */
	RASTERIZER_MEMORY_POOL_SIZE = 0x18000 * (HALO_PORT_MAXIMUM_PARTICLES / 1024),
	RASTERIZER_MAXIMUM_TRIANGLES_PER_TRIANGLE_BUFFER = 24576,
	RASTERIZER_MAXIMUM_DEBUG_PRIMITIVES = 131072,
	RASTERIZER_MAXIMUM_DEBUG_VERTICES = 393216,
	/* port: the native builds' larger pool (halo_port_capacity.h) */
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS = HALO_PORT_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS,
	RASTERIZER_MAXIMUM_TRANSPARENT_GEOMETRY_GROUPS2 = 32,
	/* port: the structure's 32768 triangles (render.h) and, past them, as
	much again as the Xbox's 16384 left a frame for its lights, shadows and
	contrails (a request that would fill the buffer is refused) */
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLES = 49152,
	RASTERIZER_MAXIMUM_DYNAMIC_TRIANGLE_BUFFERS = 1024,
	/* four per particle (build_sprites_begin), for the native builds' larger
	particle pool (halo_port_capacity.h) */
	RASTERIZER_MAXIMUM_DYNAMIC_UNLIT_VERTICES = 4 * HALO_PORT_MAXIMUM_PARTICLES,
	RASTERIZER_MAXIMUM_DYNAMIC_LIT_VERTICES = 2,
	RASTERIZER_MAXIMUM_DYNAMIC_SCREEN_VERTICES = 16384,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_VERTICES = 2048,
	RASTERIZER_MAXIMUM_DYNAMIC_MODEL_PROCESSED_VERTICES = 8192,
	RASTERIZER_MAXIMUM_DYNAMIC_VERTEX_BUFFERS = 1024,
	RASTERIZER_MAXIMUM_DETAIL_OBJECTS_PER_FRAME = 4096,
	RASTERIZER_NODES_PER_MODEL_VERTEX = 2,
	RASTERIZER_MAXIMUM_NODES_PER_MODEL = 44,
	RASTERIZER_MAXIMUM_NEARBY_OPAQUE_MODEL_GEOMETRY_GROUPS_THAT_MIGHT_OBSCURE_THE_ENVIRONMENT_FOG_SCREEN_EFFECT = 1
};

enum
{
	_rasterizer_profile_clear = 0,
	_rasterizer_profile_model_sky,
	_rasterizer_profile_models,
	_rasterizer_profile_environment_lightmaps,
	_rasterizer_profile_environment_shadows,
	_rasterizer_profile_environment_diffuse_lights,
	_rasterizer_profile_environment_decals_light,
	_rasterizer_profile_environment_decals_alpha_tested,
	_rasterizer_profile_environment_textures,
	_rasterizer_profile_environment_decals_primary,
	_rasterizer_profile_environment_decals_secondary,
	_rasterizer_profile_environment_specular_lights,
	_rasterizer_profile_environment_specular_lightmaps,
	_rasterizer_profile_environment_reflection_lightmap_masks,
	_rasterizer_profile_environment_reflection_mirrors,
	_rasterizer_profile_environment_reflections,
	_rasterizer_profile_environment_transparents,
	_rasterizer_profile_environment_fog,
	_rasterizer_profile_environment_fog_screen,
	_rasterizer_profile_water,
	_rasterizer_profile_environment_decals_water,
	_rasterizer_profile_detail_objects,
	_rasterizer_profile_queued_transparents,
	_rasterizer_profile_lens_flare_occlusion_submit,
	_rasterizer_profile_lens_flare_occlusion_query,
	_rasterizer_profile_lens_flares,
	_rasterizer_profile_screen_effect,
	_rasterizer_profile_hud,
	_rasterizer_profile_screen_flash,
	NUMBER_OF_RASTERIZER_PROFILES,
};

enum rasterizer_lock_operation
{
	_rasterizer_lock_none = 0,
	_rasterizer_lock_texture_changed = 1,
	_rasterizer_lock_vertexbuffer_new = 2,
	_rasterizer_lock_detail_objects = 3,
	_rasterizer_lock_decal_update = 4,
	_rasterizer_lock_decal_vertices = 5,
	_rasterizer_lock_bink = 6,
	_rasterizer_lock_ui = 7,
	_rasterizer_lock_cinematics = 8,
	_rasterizer_lock_koth = 9,
	_rasterizer_lock_hud = 10,
	_rasterizer_lock_flag = 11,
	_rasterizer_lock_lightning = 12,
	_rasterizer_lock_debug = 13,
	_rasterizer_lock_text = 14,
	_rasterizer_lock_contrail = 15,
	_rasterizer_lock_sprite = 16,
	_rasterizer_lock_bsp_switch = 17,
	NUMBER_OF_RASTERIZER_LOCK_OPERATIONS,
};


/* ---------- macros */

#define RASTERIZER_GLOBALS_FLOATING_POINT_ZBUFFER(globals) ((globals).floating_point_zbuffer)

/* ---------- structures */

struct rasterizer_cinematic_screen_effect_parameters;

struct rasterizer_model_begin_parameters;
struct detail_object_view_data;
struct bitmap_data;
struct render_animation;
struct shader;
struct triangle_buffer;
struct vertex_buffer;
struct rasterizer_dynamic_screen_geometry_parameters;

#ifndef RASTERIZER_WIDGET_SIGNATURES_OWNED
typedef void (*rasterizer_widget_render_proc)(
	long object_index,
	long widget_index);
#endif

struct rasterizer_frame_begin_parameters
{
	real game_time_sec;
	real dt;
};

struct dynamic_screen_vertex
{
	real_point2d position;
	real_point2d texture_coordinates;
	pixel32 color;
};

typedef char verify_dynamic_screen_vertex_size[
	sizeof(struct dynamic_screen_vertex) == 0x14 ? 1 : -1];

struct rasterizer_dynamic_screen_geometry_parameters
{
	void *meter_parameters;
	real_vector2d const *offset;
	boolean map_anchor_screen[3];
	byte pad0B;
	struct bitmap_data *map[3];
	boolean map_wrapped[3];
	byte pad1B;
	real_point2d *map_offset[3];
	real_vector2d map_scale[3];
	real_vector2d map_texture_scale[3];
	real_rgb_color const *map_tint[3];
	real_argb_color plasma_fade;
	boolean doing_plasma_effect;
	byte pad75[3];
	real const *map_fade[3];
	short map0_to_1_blend_function;
	short map1_to_2_blend_function;
	short framebuffer_blend_function;
	boolean point_sampled;
	/* port: the maps' color is weighted by their alpha before the
	framebuffer blend (Halo PC's HUD overlays, hud_draw.c) */
	boolean alpha_weighted;
};

typedef char verify_rasterizer_dynamic_screen_geometry_parameters_size[
	sizeof(struct rasterizer_dynamic_screen_geometry_parameters) == 0x8C ? 1 : -1];

struct rasterizer_globals_reserved04
{
	rectangle2d screen_bounds;
	rectangle2d frame_bounds;
	byte __unknown14[4];
};

struct rasterizer_globals_definition
{
	boolean initialized;
	byte reserved01;
	short current_lock_operation;
	struct rasterizer_globals_reserved04 reserved04;
	unsigned __int64 fps_accumulation_frame_index;
	volatile unsigned long d3d_flip_count;
	byte reserved24[4];
	/* Updated asynchronously as one 64-bit counter by the vblank callback. */
	union
	{
		volatile __int64 vertical_blank_index;
		volatile unsigned __int64 frame_and_vertical_blank_index;
		struct
		{
			volatile unsigned long frame_index;
			volatile unsigned long vertical_blank_count;
		};
	};
	union
	{
		volatile unsigned __int64 previous_frame_and_vertical_blank_index;
		struct
		{
			volatile unsigned long previous_frame_index;
			volatile unsigned long previous_vertical_blank_index;
		};
	};
	short push_buffer_size;
	short kick_off_size;
	boolean floating_point_zbuffer;
	boolean framerate_throttle;
	boolean framerate_throttle_debug;
	byte reserved3F;
	short framerate_throttle_target;
	byte reserved42[2];
	real near_clip_distance;
	real far_clip_distance;
	real first_person_weapon_near_clip_distance;
	real first_person_weapon_far_clip_distance;
	void *default_2d_hardware_format;
	void *default_3d_hardware_format;
	void *default_cm_hardware_format;
	short lightmap_mode;
	byte reserved62[0x6];
};

typedef char verify_rasterizer_globals_size[
	sizeof(struct rasterizer_globals_definition) == 0x68 ? 1 : -1];
typedef char verify_rasterizer_globals_initialized_offset[
	offsetof(struct rasterizer_globals_definition, initialized) == 0x00 ? 1 : -1];
typedef char verify_rasterizer_globals_lock_operation_offset[
	offsetof(struct rasterizer_globals_definition, current_lock_operation) == 0x02 ? 1 : -1];
typedef char verify_rasterizer_globals_frame_bounds_offset[
	offsetof(struct rasterizer_globals_definition, reserved04.frame_bounds) == 0x0C ? 1 : -1];
typedef char verify_rasterizer_globals_fps_accumulation_frame_index_offset[
	offsetof(struct rasterizer_globals_definition, fps_accumulation_frame_index) == 0x18 ? 1 : -1];
typedef char verify_rasterizer_globals_framerate_throttle_offset[
	offsetof(struct rasterizer_globals_definition, framerate_throttle) == 0x3D ? 1 : -1];
typedef char verify_rasterizer_globals_d3d_flip_count_offset[
	offsetof(struct rasterizer_globals_definition, d3d_flip_count) == 0x20 ? 1 : -1];
typedef char verify_rasterizer_globals_frame_and_vertical_blank_index_offset[
	offsetof(struct rasterizer_globals_definition, frame_and_vertical_blank_index) == 0x28 ? 1 : -1];
typedef char verify_rasterizer_globals_previous_frame_and_vertical_blank_index_offset[
	offsetof(struct rasterizer_globals_definition, previous_frame_and_vertical_blank_index) == 0x30 ? 1 : -1];
typedef char verify_rasterizer_globals_framerate_throttle_debug_offset[
	offsetof(struct rasterizer_globals_definition, framerate_throttle_debug) == 0x3E ? 1 : -1];
typedef char verify_rasterizer_globals_framerate_throttle_target_offset[
	offsetof(struct rasterizer_globals_definition, framerate_throttle_target) == 0x40 ? 1 : -1];
typedef char verify_rasterizer_globals_floating_point_zbuffer_offset[
	offsetof(struct rasterizer_globals_definition, floating_point_zbuffer) == 0x3C ? 1 : -1];
typedef char verify_rasterizer_globals_near_clip_distance_offset[
	offsetof(struct rasterizer_globals_definition, near_clip_distance) == 0x44 ? 1 : -1];
struct rasterizer_window_begin_parameters
{
	short rasterizer_target;
	short window_index;
	boolean has_mirror;
	boolean suppress_clear;
	struct render_camera camera;
	struct render_frustum frustum;
	struct render_fog fog;
	struct render_screen_flash screen_flash;
	struct render_screen_effect screen_effect;
};

/* January's 0x170-byte linker-common record. Only counters already used by
 * reconstructed writers are named here; unreviewed interiors stay reserved. */
struct rasterizer_frame_statistics_globals
{
	real frames_per_second;
	short fps_sample_count;
	short pad006;
	real average_frames_per_second;
	real minimum_frames_per_second;
	real maximum_frames_per_second;
	long vertices_by_permutation[4]; /* per model vertex-shader permutation */
	unsigned long lightmap_dynamic_vertex_count;
	unsigned long lightmap_dynamic_triangle_count;
	unsigned long lightmap_dynamic_draw_count;
	unsigned long shadow_count;
	unsigned long shadow_vertex_count;
	unsigned long shadow_triangle_count;
	unsigned long shadow_draw_count;
	unsigned long environment_dynamic_vertex_count;
	unsigned long environment_dynamic_triangle_count;
	unsigned long environment_dynamic_draw_count;
	unsigned long decal_vertex_count;
	unsigned long decal_triangle_count;
	unsigned long decal_draw_count;
	unsigned long decal_shader_change_count;
	unsigned long decal_texture_change_count;
	unsigned long diffuse_texture_dynamic_vertex_count;
	unsigned long diffuse_texture_dynamic_triangle_count;
	unsigned long diffuse_texture_dynamic_draw_count;
	unsigned long specular_light_dynamic_vertex_count;
	unsigned long specular_light_dynamic_triangle_count;
	unsigned long specular_light_dynamic_draw_count;
	unsigned long specular_lightmap_dynamic_vertex_count;
	unsigned long specular_lightmap_dynamic_triangle_count;
	unsigned long specular_lightmap_dynamic_draw_count;
	unsigned long reflection_mask_dynamic_vertex_count;
	unsigned long reflection_mask_dynamic_triangle_count;
	unsigned long reflection_mask_dynamic_draw_count;
	unsigned long reflection_dynamic_vertex_count;
	unsigned long reflection_dynamic_triangle_count;
	unsigned long reflection_dynamic_draw_count;
	unsigned long transparent_geometry_dynamic_vertex_count;
	unsigned long transparent_geometry_dynamic_triangle_count;
	long transparent_geometry_largest_dynamic_triangle_count;
	unsigned long transparent_geometry_dynamic_draw_count;
	unsigned long environment_fog_dynamic_vertex_count;
	unsigned long environment_fog_dynamic_triangle_count;
	unsigned long environment_fog_dynamic_draw_count;
	unsigned long environment_fog_screen_dynamic_vertex_count;
	unsigned long environment_fog_screen_dynamic_triangle_count;
	unsigned long environment_fog_screen_dynamic_draw_count;
	unsigned long environment_fog_screen_model_count;
	unsigned long environment_fog_screen_static_vertex_count;
	unsigned long environment_fog_screen_static_triangle_count;
	unsigned long environment_fog_screen_static_draw_count;
	unsigned long model_count;
	unsigned long model_vertex_count;
	unsigned long model_triangle_count;
	unsigned long model_draw_count;
	long transparent_model_vertex_count;
	long transparent_model_triangle_count;
	long transparent_model_maximum_triangle_count;
	long transparent_model_submit_count;
	unsigned long model_shadow_count;
	unsigned long model_shadow_vertex_count;
	unsigned long model_shadow_triangle_count;
	unsigned long model_shadow_draw_count;
	unsigned long dynamic_unlit_draw_count;
	unsigned long dynamic_unlit_triangle_count;
	long largest_dynamic_unlit_triangle_count;
	unsigned long dynamic_unlit_vertex_count;
	byte reserved114[0x1C];
	long dynamic_vertex_count;
	long dynamic_vertex_buffer_count;
	long dynamic_triangle_count;
	long dynamic_triangle_buffer_count;
	long debug_primitive_count;
	byte reserved144[4];
	long dynamic_light_count;
	long lens_flare_count;
	long vertex_shader_skinning_constant_bytes;
	long vertex_shader_lighting_constant_bytes;
	long vertex_shader_instruction_count;
	long pixel_shader_pushbuffer_bytes;
	unsigned long model_skinning_constant_bytes;
	unsigned long model_lighting_constant_bytes;
	unsigned long model_vertex_shader_work_accumulated;
	byte reserved16C[4];
};

typedef char rasterizer_frame_statistics_globals_size_assert[
	sizeof(struct rasterizer_frame_statistics_globals) == 0x170 ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_dynamic_vertex_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_dynamic_vertex_count) == 0xB8 ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_dynamic_triangle_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_dynamic_triangle_count) == 0xBC ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_dynamic_draw_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_dynamic_draw_count) == 0xC0 ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_model_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_model_count) == 0xC4 ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_static_vertex_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_static_vertex_count) == 0xC8 ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_static_triangle_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_static_triangle_count) == 0xCC ? 1 : -1];
typedef char rasterizer_frame_statistics_environment_fog_screen_static_draw_count_offset_assert[
	offsetof(
		struct rasterizer_frame_statistics_globals,
		environment_fog_screen_static_draw_count) == 0xD0 ? 1 : -1];

struct transparent_geometry_group;

/* ---------- prototypes/RASTERIZER.C */

void rasterizer_reset_state(
	void);
void rasterizer_widget_end(
	void);

boolean rasterizer_initialize(void);

void rasterizer_frame_begin(const struct rasterizer_frame_begin_parameters *parameters);
void rasterizer_windows_begin(
	void);
void rasterizer_window_begin(
	struct rasterizer_window_begin_parameters const *parameters);
void rasterizer_window_end(
	void);
void rasterizer_windows_end(void);
void rasterizer_frame_end(void);

void rasterizer_present(struct bitmap_data *screenshot_bitmap, const point2d *screenshot_index);
void rasterizer_dispose(void);
void rasterizer_window_get_fog(
	struct render_fog *fog);
void rasterizer_window_set_fog(
	struct render_fog const *fog);
void rasterizer_set_vblank_callback(
	void (*callback)(unsigned long));
long rasterizer_dynamic_triangles_new(
	long triangle_count);
short *rasterizer_dynamic_triangles_lock(
	long triangle_buffer_index);
void rasterizer_dynamic_triangles_unlock(
	long triangle_buffer_index);
void rasterizer_dynamic_triangles_delete(
	long triangle_buffer_index);
long rasterizer_dynamic_vertices_new(
	short type,
	long vertex_count);
short rasterizer_dynamic_vertices_get_type(
	long dynamic_vertex_buffer_index);
void *rasterizer_dynamic_vertices_lock(
	long dynamic_vertex_buffer_index);
void rasterizer_dynamic_vertices_unlock(
	long dynamic_vertex_buffer_index);
void rasterizer_dynamic_vertices_delete(
	long dynamic_vertex_buffer_index);
void rasterizer_debug_immediate_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1);
void rasterizer_debug_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color);
void rasterizer_debug_immediate_point(
	real_point3d const *point,
	real radius,
	real_rgb_color const *color);
void rasterizer_debug_immediate_vector(
	real_point3d const *point,
	real_vector3d const *vector,
	real scale,
	real_rgb_color const *color);
void rasterizer_debug_immediate_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_rgb_color const *color0,
	real_rgb_color const *color1,
	real_rgb_color const *color2);
void rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1);
void rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color);
void *rasterizer_decal_vertices_lock(
	short cache_index,
	unsigned long cache_size);
void rasterizer_decal_vertices_unlock(
	void);
long rasterizer_decal_vertices_new(
	long size);
void rasterizer_decal_vertices_delete(
	long decal_vertex_buffer_index);

void rasterizer_decals_initialize(
	void);
void rasterizer_decals_initialize_for_new_map(
	void);
void rasterizer_decals_dispose_from_old_map(
	void);
void rasterizer_decals_dispose(
	void);
void rasterizer_decals_begin(
	short type);
void rasterizer_decals_end(
	void);
void rasterizer_decals_draw(
	short cluster_index);
void rasterizer_decals_flush(
	void);
void rasterizer_decals_update_function_pointers(void);
void rasterizer_detail_objects_begin(
	void);
void rasterizer_detail_objects_rebuild_vertices(
	struct detail_object_view_data *view_data);
void rasterizer_detail_objects_draw(
	struct detail_object_view_data *view_data);
void rasterizer_detail_objects_end(
	void);
void rasterizer_screen_effect(
	struct rasterizer_cinematic_screen_effect_parameters *parameters);

void rasterizer_debug_immediate_begin(
	void);
void rasterizer_debug_immediate_end(
	void);
void rasterizer_debug_immediate_begin_screenspace(
	void);
void rasterizer_debug_immediate_end_screenspace(
	void);

void rasterizer_hud_begin(
	void);
void rasterizer_hud_end(
	void);
void rasterizer_hud_motion_sensor_blip_begin(
	void);
void rasterizer_hud_motion_sensor_blip_draw(
	real_point2d const *position,
	real intensity,
	real size,
	real_rgb_color const *color,
	boolean large_blip);
void rasterizer_hud_motion_sensor_blip_end(
	real_point2d const *center,
	real scale);
void rasterizer_models_end(
	void);
void rasterizer_models_begin(
	boolean skip_obscurer_test);
void rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap);
void rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_lightmap_end(
	void);
void rasterizer_environment_lightmaps_end(
	void);
void rasterizer_environment_diffuse_light_end(
	void);
void rasterizer_environment_diffuse_lights_end(
	void);
void rasterizer_environment_diffuse_light_begin(
	long light_index);
void rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_shadows_begin(
	void);
boolean rasterizer_environment_shadow_begin(
	long object_index,
	real_matrix4x3 const *shadow_matrix,
	real_rgb_color const *shadow_color,
	real object_bounding_radius,
	real *shadow_volume_bounding_radius);
void rasterizer_environment_shadow_model_begin(
	struct rasterizer_model_begin_parameters const *parameters);
void rasterizer_environment_shadow_model_draw(
	struct shader const *shader,
	short bitmap_index,
	struct triangle_buffer const *triangle_buffer,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_shadow_model_end(
	void);
void rasterizer_environment_shadow_end(
	void);
void rasterizer_environment_shadow_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_shadows_end(
	void);
void rasterizer_environment_diffuse_textures_end(
	void);
void rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_specular_light_begin(
	long light_index);
void rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_specular_light_end(
	void);
void rasterizer_environment_specular_lights_end(
	void);
void rasterizer_environment_specular_lightmap_end(
	void);
void rasterizer_environment_specular_lightmaps_end(
	void);
void rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap);
void rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflection_lightmap_mask_end(
	void);
void rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap_bitmap);
void rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflection_lightmap_masks_end(
	void);
void rasterizer_environment_reflection_mirrors_begin(
	void);
void rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflection_mirrors_end(
	void);
void rasterizer_environment_reflections_begin(
	void);
void rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_reflections_end(
	void);
void rasterizer_environment_transparent_geometry_begin(
	void);
void rasterizer_environment_transparent_geometry_submit(
	struct shader const *shader,
	short shader_permutation_index,
	struct bitmap_data const *lightmap,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffers,
	real_point3d const *centroid,
	real_plane3d const *plane,
	real_vector3d const *offset,
	void const *lighting,
	unsigned long geometry_flags);
void rasterizer_environment_transparent_geometry_end(
	void);
void rasterizer_dynamic_screen_geometry_draw(
	long primitive_type,
	long vertex_type,
	long dynamic_vertex_buffer_index,
	long vertex_count);
void rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(
	struct rasterizer_dynamic_screen_geometry_parameters *base,
	struct rasterizer_dynamic_screen_geometry_parameters const *multitex_params);
void rasterizer_dynamic_unlit_geometry_draw(
	struct shader const *shader,
	struct bitmap_data const *bitmap,
	struct render_animation const *animation,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long vertex_count,
	real_point3d const *centroid,
	unsigned long geometry_flags);
void rasterizer_dynamic_lit_geometry_draw(
	void const *vertices,
	void const *parameters);
void rasterizer_psuedo_dynamic_screen_quad_draw(
	struct rasterizer_dynamic_screen_geometry_parameters *parameters,
	struct dynamic_screen_vertex *vertices);
#ifndef RASTERIZER_WIDGET_SIGNATURES_OWNED
#endif
void rasterizer_profile_enable(
	boolean enable);
void rasterizer_screen_flash(
	void);
void rasterizer_environment_fog_screen_begin(
	boolean render_fog);
void rasterizer_environment_fog_screen_wind_get_vector(
	short wind_index,
	real animation_time,
	real_vector3d *wind_vector);
void rasterizer_environment_fog_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void rasterizer_environment_fog_screen_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
#ifndef RASTERIZER_WIDGET_SIGNATURES_OWNED
#endif

/* ---------- prototypes/RASTERIZER_XBOX_ACTIVE_CAMOUFLAGE.C */

void rasterizer_active_camouflage_set_visibility(
	boolean visibility);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_draw_string(
	rectangle2d const *bounds,
	rectangle2d const *clip,
	point2d *cursor_reference,
	short height_adjust,
	char const *string);

/* ---------- prototypes/RASTERIZER_MEMORY_POOL.C */

boolean rasterizer_memory_pool_initialize(void);
void rasterizer_memory_pool_begin(void);
void *rasterizer_memory_alloc(const void *src, unsigned long size);
const void *rasterizer_memory_alloc_const(const void *src, unsigned long size);
void rasterizer_memory_pool_end(void);
void rasterizer_memory_pool_dispose(void);

/* ---------- prototypes/RASTERIZER_LIGHTS.C */

void rasterizer_lights_reset_for_new_map(
	void);
/* port: (scenario_switch_structure_bsp) */
void rasterizer_lights_reset_for_new_structure_bsp(
	void);
void rasterizer_lights_begin(
	void);
void rasterizer_lights_end(
	void);

/* ---------- prototypes/RASTERIZER_FRAME_STATISTICS.C */

boolean rasterizer_frame_statistics_initialize(
	void);
void rasterizer_frame_statistics_begin(
	void);
void rasterizer_frame_statistics_get_fps(
	struct rasterizer_frame_statistics_globals *frame_statistics);
void rasterizer_fps_accumulate(
	void);
long rasterizer_frame_statistics_count_static_vertices(
	struct triangle_buffer const *triangle_buffer,
	struct vertex_buffer const *vertex_buffer);
long rasterizer_frame_statistics_count_dynamic_vertices(
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count);
void rasterizer_frame_statistics_draw(
	void);
void rasterizer_frame_statistics_end(
	void);
void rasterizer_frame_statistics_dispose(
	void);

/* ---------- prototypes/RASTERIZER_TRANSPARENT_GEOMETRY.C */

boolean rasterizer_transparent_geometry_initialize(
	void);
boolean rasterizer_transparent_geometry_initialize_aux_buffer(
	void);
void rasterizer_transparent_geometry_dispose_aux_buffer(
	void);
void rasterizer_transparent_geometry_begin(
	void);
void rasterizer_transparent_geometry_end(
	void);
short rasterizer_transparent_geometry_get_primary_vertex_type(
	struct transparent_geometry_group const *group);
void rasterizer_transparent_geometry_set_group_pending_status(
	struct transparent_geometry_group const *group,
	boolean pending);
boolean rasterizer_transparent_geometry_get_group_pending_status(
	struct transparent_geometry_group const *group);
short rasterizer_transparent_geometry_get_group_presorted_index(
	struct transparent_geometry_group const *group);
void *rasterizer_transparent_geometry_get_group_from_presorted_index(
	short presorted_index);
void *rasterizer_transparent_geometry_get_groups2(
	short *group_count);
struct transparent_geometry_group *rasterizer_transparent_geometry_next_group(
	struct transparent_geometry_group const *group);
struct transparent_geometry_group *rasterizer_transparent_geometry_new_group(
	void);
struct transparent_geometry_group *rasterizer_transparent_geometry_new_group2(
	void);
void rasterizer_transparent_geometry_stop(
	void);
void rasterizer_transparent_geometry_dispose(
	void);
void rasterizer_transparent_geometry_draw(
	boolean water);

/* ---------- prototypes/RASTERIZER_DEBUG.C */

boolean rasterizer_debug_initialize(
	void);
void rasterizer_debug_begin(
	void);
void rasterizer_debug_end(
	void);
void rasterizer_debug_dispose(
	void);
void rasterizer_debug_draw(
	void);
void rasterizer_debug_line_shaded(
	real_point3d const *p0,
	real_point3d const *p1,
	real_argb_color const *color0,
	real_argb_color const *color1);
void rasterizer_debug_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_argb_color const *color);

/* ---------- prototypes/RASTERIZER_XBOX_DEBUG.C */

void rasterizer_debug_drawing_begin(
	boolean opaque);
void rasterizer_debug_drawing_end(
	void);
void _rasterizer_debug_immediate_begin(
	void);
void _rasterizer_debug_immediate_end(
	void);
void _rasterizer_debug_immediate_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1);
void _rasterizer_debug_immediate_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_rgb_color const *color0,
	real_rgb_color const *color1,
	real_rgb_color const *color2);
void _rasterizer_debug_immediate_begin_screenspace(
	void);
void _rasterizer_debug_immediate_end_screenspace(
	void);
void _rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1);
void _rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color);

/* ---------- prototypes/RASTERIZER_TEXT.C */

void rasterizer_text_set_shadow_color(
	pixel32 shadow_color);
void rasterizer_draw_unicode_string(
	rectangle2d const *bounds,
	rectangle2d const *clip,
	point2d *cursor_reference,
	short height_adjust,
	wchar_t const *string);
/* port: visible ink metrics and a scoped fractional offset for aligned adornments. */
void rasterizer_unicode_string_ink_bounds(rectangle2d const *bounds,
    wchar_t const *text, real_rectangle2d *ink);
void rasterizer_draw_unicode_string_offset(rectangle2d const *bounds,
    wchar_t const *text, real offset_x, real offset_y);
/* port: scoped local text scaling for compact keyboard keycaps. */
void rasterizer_draw_unicode_string_scaled_offset(rectangle2d const *bounds,
    wchar_t const *text, real scale, real offset_x, real offset_y);
/* port: text drawn scale times larger about a point, until set back to 1 */
void rasterizer_text_set_scale(
	real scale,
	real origin_x,
	real origin_y);
void rasterizer_text_cache_flush(
	void);
void rasterizer_text_cache_dispose(
	void);

/* ---------- globals */

extern real_argb_color *global_rasterizer_model_ambient_reflection_tint;
extern struct rasterizer_globals_definition rasterizer_globals;

/* comm. not sure where this should be */
extern struct rasterizer_frame_begin_parameters global_frame_parameters;
extern struct rasterizer_window_begin_parameters global_window_parameters;

extern struct rasterizer_frame_statistics_globals rasterizer_frame_statistics;

/* ---------- public code */

#endif // __RASTERIZER_H

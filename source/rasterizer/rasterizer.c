/*
RASTERIZER.C

symbols in this file:
0016BFE0 0040:
	_rasterizer_initialize (0000)
0016C020 0010:
	_rasterizer_reset_state (0000)
0016C030 00e0:
	_rasterizer_frame_begin (0000)
0016C110 0010:
	_rasterizer_windows_begin (0000)
0016C120 0010:
	_rasterizer_window_begin (0000)
0016C130 0010:
	_rasterizer_window_get_fog (0000)
0016C140 0010:
	_rasterizer_window_set_fog (0000)
0016C150 0010:
	_rasterizer_window_end (0000)
0016C160 0010:
	_rasterizer_windows_end (0000)
0016C170 0010:
	_rasterizer_frame_end (0000)
0016C180 0010:
	_rasterizer_present (0000)
0016C190 0010:
	_rasterizer_dispose (0000)
0016C1A0 0010:
	_rasterizer_set_vblank_callback (0000)
0016C1B0 0010:
	_rasterizer_profile_enable (0000)
0016C1C0 0010:
	_rasterizer_dynamic_triangles_new (0000)
0016C1D0 0010:
	_rasterizer_dynamic_triangles_lock (0000)
0016C1E0 0010:
	_rasterizer_dynamic_triangles_unlock (0000)
0016C1F0 0010:
	_rasterizer_dynamic_triangles_delete (0000)
0016C200 0010:
	_rasterizer_dynamic_vertices_new (0000)
0016C210 0010:
	_rasterizer_dynamic_vertices_get_type (0000)
0016C220 0010:
	_rasterizer_dynamic_vertices_lock (0000)
0016C230 0010:
	_rasterizer_dynamic_vertices_unlock (0000)
0016C240 0010:
	_rasterizer_dynamic_vertices_delete (0000)
0016C250 0010:
	_rasterizer_debug_immediate_begin (0000)
0016C260 0010:
	_rasterizer_debug_immediate_line (0000)
0016C270 0010:
	_rasterizer_debug_immediate_triangle (0000)
0016C280 0010:
	_rasterizer_debug_immediate_end (0000)
0016C290 0010:
	_rasterizer_debug_immediate_begin_screenspace (0000)
0016C2A0 0010:
	_rasterizer_debug_immediate_line_screenspace (0000)
0016C2B0 0010:
	_rasterizer_debug_immediate_linestrip_screenspace (0000)
0016C2C0 0010:
	_rasterizer_debug_immediate_end_screenspace (0000)
0016C2D0 0010:
	_rasterizer_decals_initialize (0000)
0016C2E0 0010:
	_rasterizer_decals_update_function_pointers (0000)
0016C2F0 0010:
	_rasterizer_decals_initialize_for_new_map (0000)
0016C300 0010:
	_rasterizer_decals_dispose_from_old_map (0000)
0016C310 0010:
	_rasterizer_decals_flush (0000)
0016C320 0010:
	_rasterizer_decals_dispose (0000)
0016C330 0010:
	_rasterizer_decal_vertices_new (0000)
0016C340 0010:
	_rasterizer_decal_vertices_lock (0000)
0016C350 0010:
	_rasterizer_decal_vertices_unlock (0000)
0016C360 0010:
	_rasterizer_decal_vertices_delete (0000)
0016C370 0010:
	_rasterizer_decals_begin (0000)
0016C380 0010:
	_rasterizer_decals_draw (0000)
0016C390 0010:
	_rasterizer_decals_end (0000)
0016C3A0 0010:
	_rasterizer_detail_objects_begin (0000)
0016C3B0 0010:
	_rasterizer_detail_objects_rebuild_vertices (0000)
0016C3C0 0010:
	_rasterizer_detail_objects_draw (0000)
0016C3D0 0010:
	_rasterizer_detail_objects_end (0000)
0016C3E0 0010:
	_rasterizer_screen_effect (0000)
0016C3F0 0010:
	_rasterizer_screen_flash (0000)
0016C400 0010:
	_rasterizer_model_begin (0000)
0016C410 0010:
	_rasterizer_model_draw (0000)
0016C420 0010:
	_rasterizer_model_transparent_geometry_submit (0000)
0016C430 0010:
	_rasterizer_model_end (0000)
0016C440 0010:
	_rasterizer_models_end (0000)
0016C450 0010:
	_rasterizer_environment_lightmaps_begin (0000)
0016C460 0010:
	_rasterizer_environment_lightmap_begin (0000)
0016C470 0010:
	_rasterizer_environment_lightmap_draw (0000)
0016C480 0010:
	_rasterizer_environment_lightmap_end (0000)
0016C490 0010:
	_rasterizer_environment_lightmaps_end (0000)
0016C4A0 0010:
	_rasterizer_environment_diffuse_lights_begin (0000)
0016C4B0 0010:
	_rasterizer_environment_diffuse_light_begin (0000)
0016C4C0 0010:
	_rasterizer_environment_diffuse_light_draw (0000)
0016C4D0 0010:
	_rasterizer_environment_diffuse_light_end (0000)
0016C4E0 0010:
	_rasterizer_environment_diffuse_lights_end (0000)
0016C4F0 0010:
	_rasterizer_environment_shadows_begin (0000)
0016C500 0010:
	_rasterizer_environment_shadow_begin (0000)
0016C510 0010:
	_rasterizer_environment_shadow_model_begin (0000)
0016C520 0010:
	_rasterizer_environment_shadow_model_draw (0000)
0016C530 0010:
	_rasterizer_environment_shadow_model_end (0000)
0016C540 0010:
	_rasterizer_environment_shadow_draw (0000)
0016C550 0010:
	_rasterizer_environment_shadow_end (0000)
0016C560 0010:
	_rasterizer_environment_shadows_end (0000)
0016C570 0010:
	_rasterizer_environment_diffuse_textures_begin (0000)
0016C580 0010:
	_rasterizer_environment_diffuse_texture_draw (0000)
0016C590 0010:
	_rasterizer_environment_diffuse_textures_end (0000)
0016C5A0 0010:
	_rasterizer_environment_specular_lights_begin (0000)
0016C5B0 0010:
	_rasterizer_environment_specular_light_begin (0000)
0016C5C0 0010:
	_rasterizer_environment_specular_light_draw (0000)
0016C5D0 0010:
	_rasterizer_environment_specular_light_end (0000)
0016C5E0 0010:
	_rasterizer_environment_specular_lights_end (0000)
0016C5F0 0010:
	_rasterizer_environment_specular_lightmaps_begin (0000)
0016C600 0010:
	_rasterizer_environment_specular_lightmap_begin (0000)
0016C610 0010:
	_rasterizer_environment_specular_lightmap_draw (0000)
0016C620 0010:
	_rasterizer_environment_specular_lightmap_end (0000)
0016C630 0010:
	_rasterizer_environment_specular_lightmaps_end (0000)
0016C640 0010:
	_rasterizer_environment_reflection_lightmap_masks_begin (0000)
0016C650 0010:
	_rasterizer_environment_reflection_lightmap_mask_begin (0000)
0016C660 0010:
	_rasterizer_environment_reflection_lightmap_mask_draw (0000)
0016C670 0010:
	_rasterizer_environment_reflection_lightmap_mask_end (0000)
0016C680 0010:
	_rasterizer_environment_reflection_lightmap_masks_end (0000)
0016C690 0010:
	_rasterizer_environment_reflection_mirrors_begin (0000)
0016C6A0 0010:
	_rasterizer_environment_reflection_mirror_draw (0000)
0016C6B0 0010:
	_rasterizer_environment_reflection_mirrors_end (0000)
0016C6C0 0010:
	_rasterizer_environment_reflections_begin (0000)
0016C6D0 0010:
	_rasterizer_environment_reflection_draw (0000)
0016C6E0 0010:
	_rasterizer_environment_reflections_end (0000)
0016C6F0 0010:
	_rasterizer_environment_transparent_geometry_begin (0000)
0016C700 0010:
	_rasterizer_environment_transparent_geometry_submit (0000)
0016C710 0010:
	_rasterizer_environment_transparent_geometry_end (0000)
0016C720 0010:
	_rasterizer_environment_fog_begin (0000)
0016C730 0010:
	_rasterizer_environment_fog_draw (0000)
0016C740 0010:
	_rasterizer_environment_fog_end (0000)
0016C750 0010:
	_rasterizer_environment_fog_screen_wind_get_vector (0000)
0016C760 0010:
	_rasterizer_environment_fog_screen_begin (0000)
0016C770 0010:
	_rasterizer_environment_fog_screen_draw (0000)
0016C780 0010:
	_rasterizer_environment_fog_screen_end (0000)
0016C790 0010:
	_rasterizer_hud_begin (0000)
0016C7A0 0010:
	_rasterizer_hud_end (0000)
0016C7B0 0010:
	_rasterizer_dynamic_unlit_geometry_draw (0000)
0016C7C0 0010:
	_rasterizer_dynamic_lit_geometry_draw (0000)
0016C7D0 0010:
	_rasterizer_dynamic_screen_geometry_draw (0000)
0016C7E0 0010:
	_rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base (0000)
0016C7F0 0010:
	_rasterizer_psuedo_dynamic_screen_quad_draw (0000)
0016C800 0010:
	_rasterizer_widget_submit (0000)
0016C810 0010:
	_rasterizer_widget_begin (0000)
0016C820 0010:
	_rasterizer_widget_set_texture (0000)
0016C830 0010:
	_rasterizer_widget_set_tint_factor (0000)
0016C840 0010:
	_rasterizer_widget_set_zbuffer_enable (0000)
0016C850 0010:
	_rasterizer_widget_draw_sprite2d (0000)
0016C860 0010:
	_rasterizer_widget_draw_sprite3d (0000)
0016C870 0010:
	_rasterizer_widget_end (0000)
0016C880 0010:
	_rasterizer_widget_submit_occlusion_test (0000)
0016C890 0010:
	_rasterizer_widget_get_occlusion_test_result (0000)
0016C8A0 0010:
	_rasterizer_hud_motion_sensor_blip_begin (0000)
0016C8B0 0010:
	_rasterizer_hud_motion_sensor_blip_draw (0000)
0016C8C0 0010:
	_rasterizer_hud_motion_sensor_blip_end (0000)
0016C8D0 00d0:
	_rasterizer_debug_immediate_point (0000)
0016C9A0 0050:
	_rasterizer_debug_immediate_vector (0000)
0016C9F0 0110:
	_rasterizer_models_begin (0000)
0016CB00 0640:
	_rasterizer_debug_model_vertices (0000)
0029D6DC 0010:
	_rasterizer_global_defaults (0000)
0029D6EC 0030:
	??_C@_0DA@KMFNEIAD@global_rasterizer_model_ambient_@ (0000)
0029D71C 0027:
	??_C@_0CH@FLEENNB@c?3?2halo?2SOURCE?2rasterizer?2raster@ (0000)
0029D744 0029:
	??_C@_0CJ@KNCEBLOJ@rasterizer?5model?5ambient?5reflect@ (0000)
0029D770 0004:
	__real@461c4000 (0000)
0029D774 0004:
	??_C@_03LDNPNKDL@?6V?$DN?$AA@ (0000)
0029D778 0005:
	??_C@_04PEOOHEKN@?$CFd?$CFc?$AA@ (0000)
0029D780 0003:
	??_C@_02BFBNNIBM@I?$DN?$AA@ (0000)
0029D784 0029:
	??_C@_0CJ@NGJFILME@node_weight0?$DO?$DN0?40f?5?$CG?$CG?5node_weigh@ (0000)
0029D7B0 0028:
	??_C@_0CI@MOEBMMIG@node_index1?$DMskinning?9?$DOnode_matri@ (0000)
0029D7D8 0028:
	??_C@_0CI@PCCBCPIO@node_index0?$DMskinning?9?$DOnode_matri@ (0000)
0029D800 0044:
	??_C@_0EE@KEGACHLN@part?9?$DOtriangle_buffer?4type?$DN?$DN_tri@ (0000)
0030D3D8 00f4:
	_rasterizer_globals (0000)
	_rasterizer_debug_options (0068)
004662EC 0008:
	_bss_004662ec (0000)
	_global_rasterizer_model_ambient_reflection_tint (0004)
*/

/* ---------- headers */

#include "cseries.h"
#include "physics/collisions.h"
#include "physics/collision_usage.h"
#include "real_math.h"
#include "rasterizer.h"
#include "rasterizer_geometry.h"
#include "render/render.h"
#include "render/render_debug.h"
#include "saved games/game_state.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	_triangle_buffer_type_precompiled_strip = 1,
	MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTICES = 2048,
	MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTEX_REFERENCES = 12,
};

/* ---------- macros */

#define rasterizer_model_obscurer_object_index bss_004662ec
#define rasterizer_debug_model_vertices_enabled rasterizer_debug_options.reserved0[7]

/* ---------- structures */

struct rasterizer_global_defaults
{
	real near_clip_distance;
	real far_clip_distance;
	real first_person_weapon_near_clip_distance;
	real first_person_weapon_far_clip_distance;
};

struct rasterizer_debug_options_definition
{
	byte reserved0[0x10];
	byte all;
	byte field_11;
	byte field_12;
	byte field_13;
	byte field_14;
	byte field_15;
	byte field_16;
	byte field_17;
	byte field_18;
	byte field_19;
	byte field_1A;
	byte field_1B;
	byte field_1C;
	byte field_1D;
	byte reserved1E[0x6E];
};

struct rasterizer_window_parameters
{
	short rasterizer_target;
	short window_index;
	byte reserved04[4];
	real_point3d camera_position;
	real_vector3d camera_forward;
};

struct rasterizer_triangle_buffer
{
	short type;
	word pad;
	long count;
	void *data;
	void *hardware_format;
};

struct rasterizer_model_vertex_compressed
{
	real_point3d position;
	unsigned long normal;
	unsigned long binormal;
	unsigned long tangent;
	point2d texture_coordinates;
	char node_indices[2];
	short node_weight;
};

struct rasterizer_model_geometry_part
{
	unsigned long flags;
	short shader_index;
	char previous_part_index;
	char next_part_index;
	short centroid_primary_node_index;
	short centroid_secondary_node_index;
	real centroid_primary_node_weight;
	real centroid_secondary_node_weight;
	real_point3d centroid;
	struct tag_block uncompressed_vertices;
	struct tag_block compressed_vertices;
	struct tag_block triangles;
	struct rasterizer_triangle_buffer triangle_buffer;
};

struct rasterizer_model_skinning
{
	real_matrix4x3 const *node_matrices;
	short node_matrix_count;
	word pad;
};

struct rasterizer_debug_model_vertex
{
	real_point3d position;
	short triangle_vertex_indices[MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTEX_REFERENCES];
	short model_vertex_indices[MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTEX_REFERENCES];
	byte triangle_vertex_index_count;
	byte model_vertex_index_count;
	word pad;
};

/* ---------- prototypes */

void _rasterizer_reset_state(
	void);
boolean _rasterizer_initialize(
	void);
void _rasterizer_frame_begin(
	struct rasterizer_frame_begin_parameters const *parameters);
void _rasterizer_present(
	struct bitmap_data *screenshot_bitmap,
	point2d const *screenshot_index);
void _rasterizer_window_get_fog(
	struct render_fog *fog);
void _rasterizer_window_set_fog(
	struct render_fog const *fog);
void _rasterizer_set_vblank_callback(
	void (*callback)(unsigned long));
long _rasterizer_dynamic_triangles_new(
	long triangle_count);
short *_rasterizer_dynamic_triangles_lock(
	long triangle_buffer_index);
void _rasterizer_dynamic_triangles_unlock(
	long triangle_buffer_index);
void _rasterizer_dynamic_triangles_delete(
	long triangle_buffer_index);
long _rasterizer_dynamic_vertices_new(
	short type,
	long vertex_count);
short _rasterizer_dynamic_vertices_get_type(
	long dynamic_vertex_buffer_index);
void *_rasterizer_dynamic_vertices_lock(
	long dynamic_vertex_buffer_index);
void _rasterizer_dynamic_vertices_unlock(
	long dynamic_vertex_buffer_index);
void _rasterizer_dynamic_vertices_delete(
	long dynamic_vertex_buffer_index);
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
void _rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1);
void _rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color);
void *_rasterizer_decal_vertices_lock(
	short cache_index,
	unsigned long cache_size);
long _rasterizer_decal_vertices_new(
	long size);
void _rasterizer_decal_vertices_delete(
	long decal_vertex_buffer_index);
void _rasterizer_decal_vertices_unlock(
	void);
void _rasterizer_widget_end(
	void);
real_vector3d *uncompress_int32_to_real_vector3d(
	real_vector3d *result,
	unsigned long compressed);

void _rasterizer_decals_update_function_pointers(void);
void _rasterizer_decals_initialize(
	void);
void _rasterizer_decals_initialize_for_new_map(
	void);
void _rasterizer_decals_dispose_from_old_map(
	void);
void _rasterizer_decals_dispose(
	void);
void _rasterizer_decals_begin(
	short type);
void _rasterizer_decals_end(
	void);
void _rasterizer_decals_draw(
	short cluster_index);
void _rasterizer_decals_flush(
	void);
void _rasterizer_detail_objects_begin(
	void);
void _rasterizer_detail_objects_rebuild_vertices(
	struct detail_object_global_runtime_data *runtime_data);
void _rasterizer_detail_objects_draw(
	struct detail_object_render_data *render_data);
void _rasterizer_detail_objects_end(
	void);
void _rasterizer_screen_effect(
	struct render_screen_effect const *effect);
void _rasterizer_debug_immediate_begin(
	void);
void _rasterizer_debug_immediate_end(
	void);
void _rasterizer_debug_immediate_begin_screenspace(
	void);
void _rasterizer_debug_immediate_end_screenspace(
	void);
void _rasterizer_hud_begin(
	void);
void _rasterizer_hud_end(
	void);
void _rasterizer_hud_motion_sensor_blip_begin(
	void);
void _rasterizer_hud_motion_sensor_blip_draw(
	real_point2d const *position,
	real intensity,
	real size,
	real_rgb_color const *color,
	boolean large_blip);
void _rasterizer_hud_motion_sensor_blip_end(
	real_point2d const *center,
	real scale);
void _rasterizer_model_begin(
	struct rasterizer_model_begin_parameters const *parameters,
	boolean is_dynamic);
void _rasterizer_model_draw(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	long geometry_index,
	long model_data,
	real_rgb_color const *change_colors,
	long model_effect);
void _rasterizer_model_transparent_geometry_submit(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	long geometry_index,
	long model_data,
	real_rgb_color const *change_colors,
	long model_effect,
	void const *lighting,
	void const *effect);
void _rasterizer_model_end(
	void);
void _rasterizer_models_end(
	void);
void _rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap);
void _rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_lightmaps_begin(
	void);
void _rasterizer_environment_lightmap_end(
	void);
void _rasterizer_environment_lightmaps_end(
	void);
void _rasterizer_environment_diffuse_light_end(
	void);
void _rasterizer_environment_diffuse_lights_end(
	void);
void _rasterizer_environment_diffuse_lights_begin(
	void);
void _rasterizer_environment_diffuse_light_begin(
	long light_index);
void _rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_shadows_begin(
	void);
void _rasterizer_environment_shadow_begin(
	long object_index,
	real_matrix4x3 const *shadow_matrix,
	real_point3d const *object_position,
	real object_bounding_radius,
	real_rgb_color const *shadow_color);
void _rasterizer_environment_shadow_model_begin(
	struct rasterizer_model_begin_parameters const *parameters);
void _rasterizer_environment_shadow_model_draw(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	real_rgb_color const *change_colors);
void _rasterizer_environment_shadow_model_end(
	void);
void _rasterizer_environment_shadow_end(
	void);
void _rasterizer_environment_shadow_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_shadows_end(
	void);
void _rasterizer_environment_diffuse_textures_end(
	void);
void _rasterizer_environment_diffuse_textures_begin(
	void);
void _rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_specular_light_end(
	void);
void _rasterizer_environment_specular_lights_end(
	void);
void _rasterizer_environment_specular_lights_begin(
	void);
void _rasterizer_environment_specular_light_begin(
	long light_index);
void _rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_specular_lightmap_end(
	void);
void _rasterizer_environment_specular_lightmaps_end(
	void);
void _rasterizer_environment_specular_lightmaps_begin(
	void);
void _rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap);
void _rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflection_lightmap_mask_end(
	void);
void _rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap_bitmap);
void _rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflection_lightmap_masks_end(
	void);
void _rasterizer_environment_reflection_lightmap_masks_begin(
	void);
void _rasterizer_environment_reflection_mirrors_begin(
	void);
void _rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflection_mirrors_end(
	void);
void _rasterizer_environment_reflections_begin(
	void);
void _rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_reflections_end(
	void);
void _rasterizer_environment_transparent_geometry_begin(
	void);
void _rasterizer_environment_transparent_geometry_submit(
	struct shader const *shader,
	short bitmap_index,
	struct bitmap_data const *bitmap,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_transparent_geometry_end(
	void);
void _rasterizer_environment_fog_begin(
	void);
void _rasterizer_environment_fog_end(
	void);
void _rasterizer_environment_fog_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_environment_fog_screen_end(
	void);
void _rasterizer_environment_fog_screen_wind_get_vector(
	short wind_index,
	real animation_time,
	real_vector3d *wind_vector);
void _rasterizer_environment_fog_screen_begin(
	boolean render_fog);
void _rasterizer_environment_fog_screen_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer);
void _rasterizer_dynamic_screen_geometry_draw(
	long primitive_type,
	long vertex_type,
	long dynamic_vertex_buffer_index,
	long vertex_count);
void _rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(
	struct rasterizer_dynamic_screen_geometry_parameters *base,
	struct rasterizer_dynamic_screen_geometry_parameters const *multitex_params);
void _rasterizer_dynamic_unlit_geometry_draw(
	struct shader const *shader,
	short bitmap_index,
	struct bitmap_data const *bitmap,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long vertex_count,
	real_point3d const *centroid,
	unsigned long geometry_flags);
void _rasterizer_dynamic_lit_geometry_draw(
	void const *vertices,
	void const *parameters);
void _rasterizer_psuedo_dynamic_screen_quad_draw(
	long dynamic_vertex_buffer_index);
long _rasterizer_widget_submit(
	long mode);
void *_rasterizer_widget_begin(
	long handle);
void _rasterizer_profile_enable(
	boolean enable);
void _rasterizer_screen_flash(
	void);
void _rasterizer_widget_set_texture(
	long handle);
void _rasterizer_widget_set_tint_factor(
	long handle);
long _rasterizer_widget_set_zbuffer_enable(
	long handle,
	long enable);
short _rasterizer_widget_draw_sprite2d(
	long dynamic_vertex_buffer_index);
long _rasterizer_widget_draw_sprite3d(
	long zbuffer_result);
void _rasterizer_widget_submit_occlusion_test(
	long handle);
long _rasterizer_widget_get_occlusion_test_result(
	long handle);
void _rasterizer_dispose(void);
void _rasterizer_frame_end(void);
void _rasterizer_window_begin(
	struct rasterizer_window_begin_parameters const *parameters);
void _rasterizer_window_end(
	void);
boolean _rasterizer_windows_begin(void);
void _rasterizer_windows_end(void);
void _rasterizer_models_begin(
	boolean skip_obscurer_test);

/* ---------- globals */

const struct rasterizer_global_defaults rasterizer_global_defaults =
{
	0.0625f,
	1024.f,
	0.01171875f,
	1024.f
};
struct rasterizer_globals_definition rasterizer_globals =
{
	FALSE,
	{ 0 },
	0,
	0,
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0 },
	TRUE,
	{ 0 },
	0.0625f,
	1024.f,
	0.01171875f,
	1024.f,
	{ 0 }
};
struct rasterizer_debug_options_definition rasterizer_debug_options =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 1, 1, 1, 1 },
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	{
		1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0,
		0x80, 0x3F, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
		1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0xCD, 0xCC, 0xCC, 0x3E, 8, 0, 0, 0, 0, 0, 0x80, 0x3B,
		0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0x80, 0x3F, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}
};
extern struct rasterizer_window_parameters global_window_parameters;
/* No PDB name survives for this target-owned BSS symbol. */
#pragma bss_seg(".bss")
long bss_004662ec;
real_argb_color *global_rasterizer_model_ambient_reflection_tint;
#pragma bss_seg()

/* ---------- public code */

boolean rasterizer_initialize(
	void)
{
	global_rasterizer_model_ambient_reflection_tint = game_state_malloc(
		"rasterizer model ambient reflection tint",
		NULL,
		sizeof(*global_rasterizer_model_ambient_reflection_tint));
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
		289,
		global_rasterizer_model_ambient_reflection_tint);
	return _rasterizer_initialize();
}

void rasterizer_reset_state(
	void)
{
	_rasterizer_reset_state();
	return;
}

void rasterizer_frame_begin(
	struct rasterizer_frame_begin_parameters const *parameters)
{
	if (rasterizer_debug_options.all <= 1)
	{
		rasterizer_debug_options.field_1C = rasterizer_debug_options.all;
		rasterizer_debug_options.field_1B = rasterizer_debug_options.all;
		rasterizer_debug_options.field_1A = rasterizer_debug_options.all;
		rasterizer_debug_options.field_19 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_18 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_17 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_16 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_15 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_14 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_12 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_13 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_11 = rasterizer_debug_options.all;
		rasterizer_debug_options.field_1D = rasterizer_debug_options.all;
		rasterizer_debug_options.all = 2;
	}

	if (rasterizer_globals.near_clip_distance == 0.f)
		rasterizer_globals.near_clip_distance = rasterizer_global_defaults.near_clip_distance;
	if (rasterizer_globals.far_clip_distance == 0.f)
		rasterizer_globals.far_clip_distance = rasterizer_global_defaults.far_clip_distance;
	if (rasterizer_globals.first_person_weapon_near_clip_distance == 0.f)
		rasterizer_globals.first_person_weapon_near_clip_distance = rasterizer_global_defaults.first_person_weapon_near_clip_distance;
	if (rasterizer_globals.first_person_weapon_far_clip_distance == 0.f)
		rasterizer_globals.first_person_weapon_far_clip_distance = rasterizer_global_defaults.first_person_weapon_far_clip_distance;

	_rasterizer_frame_begin(parameters);
	return;
}

void rasterizer_present(
	struct bitmap_data *screenshot_bitmap,
	point2d const *screenshot_index)
{
	_rasterizer_present(screenshot_bitmap, screenshot_index);
	return;
}

void rasterizer_window_get_fog(
	struct render_fog *fog)
{
	_rasterizer_window_get_fog(fog);
	return;
}

void rasterizer_window_set_fog(
	struct render_fog const *fog)
{
	_rasterizer_window_set_fog(fog);
	return;
}

void rasterizer_set_vblank_callback(
	void (*callback)(unsigned long))
{
	_rasterizer_set_vblank_callback(callback);
	return;
}

long rasterizer_dynamic_triangles_new(
	long triangle_count)
{
	return _rasterizer_dynamic_triangles_new(triangle_count);
}

short *rasterizer_dynamic_triangles_lock(
	long triangle_buffer_index)
{
	return _rasterizer_dynamic_triangles_lock(triangle_buffer_index);
}

void rasterizer_dynamic_triangles_unlock(
	long triangle_buffer_index)
{
	_rasterizer_dynamic_triangles_unlock(triangle_buffer_index);
	return;
}

void rasterizer_dynamic_triangles_delete(
	long triangle_buffer_index)
{
	_rasterizer_dynamic_triangles_delete(triangle_buffer_index);
	return;
}

long rasterizer_dynamic_vertices_new(
	short type,
	long vertex_count)
{
	return _rasterizer_dynamic_vertices_new(type, vertex_count);
}

short rasterizer_dynamic_vertices_get_type(
	long dynamic_vertex_buffer_index)
{
	return _rasterizer_dynamic_vertices_get_type(dynamic_vertex_buffer_index);
}

void *rasterizer_dynamic_vertices_lock(
	long dynamic_vertex_buffer_index)
{
	return _rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);
}

void rasterizer_dynamic_vertices_unlock(
	long dynamic_vertex_buffer_index)
{
	_rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
	return;
}

void rasterizer_dynamic_vertices_delete(
	long dynamic_vertex_buffer_index)
{
	_rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
	return;
}

long rasterizer_decal_vertices_new(
	long size)
{
	return _rasterizer_decal_vertices_new(size);
}

void rasterizer_decal_vertices_delete(
	long decal_vertex_buffer_index)
{
	_rasterizer_decal_vertices_delete(decal_vertex_buffer_index);
	return;
}

void rasterizer_debug_immediate_line(
	real_point3d const *p0,
	real_point3d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	_rasterizer_debug_immediate_line(p0, p1, color0, color1);
	return;
}

void rasterizer_debug_immediate_triangle(
	real_point3d const *p0,
	real_point3d const *p1,
	real_point3d const *p2,
	real_rgb_color const *color0,
	real_rgb_color const *color1,
	real_rgb_color const *color2)
{
	_rasterizer_debug_immediate_triangle(p0, p1, p2, color0, color1, color2);
	return;
}

void rasterizer_debug_immediate_line_screenspace(
	point2d const *p0,
	point2d const *p1,
	real_rgb_color const *color0,
	real_rgb_color const *color1)
{
	_rasterizer_debug_immediate_line_screenspace(p0, p1, color0, color1);
	return;
}

void rasterizer_debug_immediate_linestrip_screenspace(
	point2d const *points,
	short point_count,
	real_rgb_color const *color)
{
	_rasterizer_debug_immediate_linestrip_screenspace(points, point_count, color);
	return;
}

void *rasterizer_decal_vertices_lock(
	short cache_index,
	unsigned long cache_size)
{
	return _rasterizer_decal_vertices_lock(cache_index, cache_size);
}

void rasterizer_decal_vertices_unlock(
	void)
{
	_rasterizer_decal_vertices_unlock();
	return;
}

void rasterizer_widget_end(
	void)
{
	_rasterizer_widget_end();
	return;
}

void rasterizer_decals_initialize(
	void)
{
	_rasterizer_decals_initialize();
	return;
}

void rasterizer_decals_update_function_pointers(void)
{
	_rasterizer_decals_update_function_pointers();
}

void rasterizer_decals_initialize_for_new_map(
	void)
{
	_rasterizer_decals_initialize_for_new_map();
	return;
}

void rasterizer_decals_dispose_from_old_map(
	void)
{
	_rasterizer_decals_dispose_from_old_map();
	return;
}

void rasterizer_decals_dispose(
	void)
{
	_rasterizer_decals_dispose();
	return;
}

void rasterizer_decals_begin(
	short type)
{
	_rasterizer_decals_begin(type);
	return;
}

void rasterizer_decals_end(
	void)
{
	_rasterizer_decals_end();
	return;
}

void rasterizer_decals_draw(
	short cluster_index)
{
	_rasterizer_decals_draw(cluster_index);
	return;
}

void rasterizer_decals_flush(
	void)
{
	_rasterizer_decals_flush();
	return;
}

void rasterizer_detail_objects_begin(
	void)
{
	_rasterizer_detail_objects_begin();
	return;
}

void rasterizer_detail_objects_rebuild_vertices(
	struct detail_object_global_runtime_data *runtime_data)
{
	_rasterizer_detail_objects_rebuild_vertices(runtime_data);
	return;
}

void rasterizer_detail_objects_draw(
	struct detail_object_render_data *render_data)
{
	_rasterizer_detail_objects_draw(render_data);
	return;
}

void rasterizer_detail_objects_end(
	void)
{
	_rasterizer_detail_objects_end();
	return;
}

void rasterizer_screen_effect(
	struct render_screen_effect const *effect)
{
	_rasterizer_screen_effect(effect);
	return;
}

void rasterizer_debug_immediate_begin(
	void)
{
	_rasterizer_debug_immediate_begin();
	return;
}

void rasterizer_debug_immediate_end(
	void)
{
	_rasterizer_debug_immediate_end();
	return;
}

void rasterizer_debug_immediate_begin_screenspace(
	void)
{
	_rasterizer_debug_immediate_begin_screenspace();
	return;
}

void rasterizer_debug_immediate_end_screenspace(
	void)
{
	_rasterizer_debug_immediate_end_screenspace();
	return;
}

void rasterizer_hud_begin(
	void)
{
	_rasterizer_hud_begin();
	return;
}

void rasterizer_hud_end(
	void)
{
	_rasterizer_hud_end();
	return;
}

void rasterizer_hud_motion_sensor_blip_begin(
	void)
{
	_rasterizer_hud_motion_sensor_blip_begin();
	return;
}

void rasterizer_hud_motion_sensor_blip_draw(
	real_point2d const *position,
	real intensity,
	real size,
	real_rgb_color const *color,
	boolean large_blip)
{
	_rasterizer_hud_motion_sensor_blip_draw(position, intensity, size, color, large_blip);
	return;
}

void rasterizer_hud_motion_sensor_blip_end(
	real_point2d const *center,
	real scale)
{
	_rasterizer_hud_motion_sensor_blip_end(center, scale);
	return;
}

void rasterizer_model_begin(
	struct rasterizer_model_begin_parameters const *parameters,
	boolean is_dynamic)
{
	_rasterizer_model_begin(parameters, is_dynamic);
	return;
}

void rasterizer_model_draw(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	long geometry_index,
	long model_data,
	real_rgb_color const *change_colors,
	long model_effect)
{
	_rasterizer_model_draw(shader, bitmap_index, geometry, geometry_index, model_data, change_colors, model_effect);
	return;
}

void rasterizer_model_transparent_geometry_submit(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	long geometry_index,
	long model_data,
	real_rgb_color const *change_colors,
	long model_effect,
	void const *lighting,
	void const *effect)
{
	_rasterizer_model_transparent_geometry_submit(shader, bitmap_index, geometry, geometry_index, model_data, change_colors, model_effect, lighting, effect);
	return;
}

void rasterizer_model_end(
	void)
{
	_rasterizer_model_end();
	return;
}

void rasterizer_models_end(
	void)
{
	_rasterizer_models_end();
	return;
}

void rasterizer_environment_lightmaps_begin(
	void)
{
	_rasterizer_environment_lightmaps_begin();
	return;
}

void rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	_rasterizer_environment_lightmap_begin(lightmap_bitmap);
	return;
}

void rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_lightmap_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_lightmap_end(
	void)
{
	_rasterizer_environment_lightmap_end();
	return;
}

void rasterizer_environment_lightmaps_end(
	void)
{
	_rasterizer_environment_lightmaps_end();
	return;
}

void rasterizer_environment_diffuse_light_end(
	void)
{
	_rasterizer_environment_diffuse_light_end();
	return;
}

void rasterizer_environment_diffuse_lights_end(
	void)
{
	_rasterizer_environment_diffuse_lights_end();
	return;
}

void rasterizer_environment_diffuse_lights_begin(
	void)
{
	_rasterizer_environment_diffuse_lights_begin();
	return;
}

void rasterizer_environment_diffuse_light_begin(
	long light_index)
{
	_rasterizer_environment_diffuse_light_begin(light_index);
	return;
}

void rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_diffuse_light_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_shadows_begin(
	void)
{
	_rasterizer_environment_shadows_begin();
	return;
}

void rasterizer_environment_shadow_begin(
	long object_index,
	real_matrix4x3 const *shadow_matrix,
	real_point3d const *object_position,
	real object_bounding_radius,
	real_rgb_color const *shadow_color)
{
	_rasterizer_environment_shadow_begin(object_index, shadow_matrix, object_position, object_bounding_radius, shadow_color);
	return;
}

void rasterizer_environment_shadow_model_begin(
	struct rasterizer_model_begin_parameters const *parameters)
{
	_rasterizer_environment_shadow_model_begin(parameters);
	return;
}

void rasterizer_environment_shadow_model_draw(
	struct shader const *shader,
	short bitmap_index,
	void const *geometry,
	real_rgb_color const *change_colors)
{
	_rasterizer_environment_shadow_model_draw(shader, bitmap_index, geometry, change_colors);
	return;
}

void rasterizer_environment_shadow_model_end(
	void)
{
	_rasterizer_environment_shadow_model_end();
	return;
}

void rasterizer_environment_shadow_end(
	void)
{
	_rasterizer_environment_shadow_end();
	return;
}

void rasterizer_environment_shadow_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_shadow_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_shadows_end(
	void)
{
	_rasterizer_environment_shadows_end();
	return;
}

void rasterizer_environment_diffuse_textures_end(
	void)
{
	_rasterizer_environment_diffuse_textures_end();
	return;
}

void rasterizer_environment_diffuse_textures_begin(
	void)
{
	_rasterizer_environment_diffuse_textures_begin();
	return;
}

void rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_diffuse_texture_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_specular_light_end(
	void)
{
	_rasterizer_environment_specular_light_end();
	return;
}

void rasterizer_environment_specular_lights_end(
	void)
{
	_rasterizer_environment_specular_lights_end();
	return;
}

void rasterizer_environment_specular_lights_begin(
	void)
{
	_rasterizer_environment_specular_lights_begin();
	return;
}

void rasterizer_environment_specular_light_begin(
	long light_index)
{
	_rasterizer_environment_specular_light_begin(light_index);
	return;
}

void rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_specular_light_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_specular_lightmap_end(
	void)
{
	_rasterizer_environment_specular_lightmap_end();
	return;
}

void rasterizer_environment_specular_lightmaps_end(
	void)
{
	_rasterizer_environment_specular_lightmaps_end();
	return;
}

void rasterizer_environment_specular_lightmaps_begin(
	void)
{
	_rasterizer_environment_specular_lightmaps_begin();
	return;
}

void rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	_rasterizer_environment_specular_lightmap_begin(lightmap_bitmap);
	return;
}

void rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_specular_lightmap_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_reflection_lightmap_mask_end(
	void)
{
	_rasterizer_environment_reflection_lightmap_mask_end();
	return;
}

void rasterizer_environment_reflection_lightmap_masks_end(
	void)
{
	_rasterizer_environment_reflection_lightmap_masks_end();
	return;
}

void rasterizer_environment_reflection_lightmap_masks_begin(
	void)
{
	_rasterizer_environment_reflection_lightmap_masks_begin();
	return;
}

void rasterizer_environment_reflection_mirrors_begin(
	void)
{
	_rasterizer_environment_reflection_mirrors_begin();
	return;
}

void rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_mirror_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_reflection_mirrors_end(
	void)
{
	_rasterizer_environment_reflection_mirrors_end();
	return;
}

void rasterizer_environment_reflections_begin(
	void)
{
	_rasterizer_environment_reflections_begin();
	return;
}

void rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_reflections_end(
	void)
{
	_rasterizer_environment_reflections_end();
	return;
}

void rasterizer_environment_transparent_geometry_begin(
	void)
{
	_rasterizer_environment_transparent_geometry_begin();
	return;
}

void rasterizer_environment_transparent_geometry_submit(
	struct shader const *shader,
	short bitmap_index,
	struct bitmap_data const *bitmap,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_transparent_geometry_submit(shader, bitmap_index, bitmap, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_transparent_geometry_end(
	void)
{
	_rasterizer_environment_transparent_geometry_end();
	return;
}

void rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	_rasterizer_environment_reflection_lightmap_mask_begin(lightmap_bitmap);
	return;
}

void rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_reflection_lightmap_mask_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_dynamic_screen_geometry_draw(
	long primitive_type,
	long vertex_type,
	long dynamic_vertex_buffer_index,
	long vertex_count)
{
	_rasterizer_dynamic_screen_geometry_draw(primitive_type, vertex_type, dynamic_vertex_buffer_index, vertex_count);
	return;
}

void rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(
	struct rasterizer_dynamic_screen_geometry_parameters *base,
	struct rasterizer_dynamic_screen_geometry_parameters const *multitex_params)
{
	_rasterizer_dynamic_screen_geometry_add_multitexture_params_to_base(base, multitex_params);
	return;
}

void rasterizer_profile_enable(
	boolean enable)
{
	_rasterizer_profile_enable(enable);
	return;
}

void rasterizer_screen_flash(
	void)
{
	_rasterizer_screen_flash();
	return;
}

void rasterizer_environment_fog_screen_begin(
	boolean render_fog)
{
	_rasterizer_environment_fog_screen_begin(render_fog);
	return;
}

void rasterizer_environment_fog_screen_wind_get_vector(
	short wind_index,
	real animation_time,
	real_vector3d *wind_vector)
{
	_rasterizer_environment_fog_screen_wind_get_vector(wind_index, animation_time, wind_vector);
	return;
}

void rasterizer_environment_fog_screen_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_fog_screen_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_dynamic_lit_geometry_draw(
	void const *vertices,
	void const *parameters)
{
	_rasterizer_dynamic_lit_geometry_draw(vertices, parameters);
	return;
}

void rasterizer_dynamic_unlit_geometry_draw(
	struct shader const *shader,
	short bitmap_index,
	struct bitmap_data const *bitmap,
	long dynamic_triangle_buffer_index,
	long dynamic_vertex_buffer_index,
	long vertex_count,
	real_point3d const *centroid,
	unsigned long geometry_flags)
{
	_rasterizer_dynamic_unlit_geometry_draw(shader, bitmap_index, bitmap, dynamic_triangle_buffer_index, dynamic_vertex_buffer_index, vertex_count, centroid, geometry_flags);
	return;
}

void rasterizer_psuedo_dynamic_screen_quad_draw(
	long dynamic_vertex_buffer_index)
{
	_rasterizer_psuedo_dynamic_screen_quad_draw(dynamic_vertex_buffer_index);
	return;
}

long rasterizer_widget_submit(
	long mode)
{
	return _rasterizer_widget_submit(mode);
}

void *rasterizer_widget_begin(
	long handle)
{
	return _rasterizer_widget_begin(handle);
}

void rasterizer_widget_set_texture(
	long handle)
{
	_rasterizer_widget_set_texture(handle);
	return;
}

void rasterizer_widget_set_tint_factor(
	long handle)
{
	_rasterizer_widget_set_tint_factor(handle);
	return;
}

long rasterizer_widget_set_zbuffer_enable(
	long handle,
	long enable)
{
	return _rasterizer_widget_set_zbuffer_enable(handle, enable);
}

short rasterizer_widget_draw_sprite2d(
	long dynamic_vertex_buffer_index)
{
	return _rasterizer_widget_draw_sprite2d(dynamic_vertex_buffer_index);
}

long rasterizer_widget_draw_sprite3d(
	long zbuffer_result)
{
	return _rasterizer_widget_draw_sprite3d(zbuffer_result);
}

void rasterizer_widget_submit_occlusion_test(
	long handle)
{
	_rasterizer_widget_submit_occlusion_test(handle);
	return;
}

long rasterizer_widget_get_occlusion_test_result(
	long handle)
{
	return _rasterizer_widget_get_occlusion_test_result(handle);
}

void rasterizer_environment_fog_begin(
	void)
{
	_rasterizer_environment_fog_begin();
	return;
}

void rasterizer_environment_fog_end(
	void)
{
	_rasterizer_environment_fog_end();
	return;
}

void rasterizer_environment_fog_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	_rasterizer_environment_fog_draw(shader, bitmap_index, dynamic_triangle_buffer_index, first_triangle_index, triangle_count, vertex_buffer);
	return;
}

void rasterizer_environment_fog_screen_end(
	void)
{
	_rasterizer_environment_fog_screen_end();
	return;
}

void rasterizer_dispose(void)
{
	_rasterizer_dispose();
}

void rasterizer_frame_end(void)
{
	_rasterizer_frame_end();
}

void rasterizer_window_begin(
	struct rasterizer_window_begin_parameters const *parameters)
{
	_rasterizer_window_begin(parameters);
	return;
}

void rasterizer_window_end(
	void)
{
	_rasterizer_window_end();
	return;
}

boolean rasterizer_windows_begin(void)
{
	return _rasterizer_windows_begin();
}

void rasterizer_windows_end(void)
{
	_rasterizer_windows_end();
}

void rasterizer_models_begin(
	boolean skip_obscurer_test)
{
	struct collision_result collision;
	real_vector3d vector;

	if (!skip_obscurer_test)
	{
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
			740,
			global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH);
		global_current_collision_users[global_current_collision_user_depth++] = 21;
		vector.i = global_window_parameters.camera_forward.i * 10000.f;
		vector.j = global_window_parameters.camera_forward.j * 10000.f;
		vector.k = global_window_parameters.camera_forward.k * 10000.f;
		if (collision_test_vector(
			0xFFF80,
			&global_window_parameters.camera_position,
			&vector,
			render.local_player_index,
			&collision))
		{
			rasterizer_model_obscurer_object_index = collision.object_index;
		}
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
			756,
			global_current_collision_user_depth > 1);
		--global_current_collision_user_depth;
	}
	else
	{
		rasterizer_model_obscurer_object_index = NONE;
	}
	_rasterizer_models_begin(skip_obscurer_test);
	return;
}

void rasterizer_debug_immediate_point(
	real_point3d const *point,
	real radius,
	real_rgb_color const *color)
{
	real_point3d p0;
	real_point3d p1;

	radius *= 0.5f;
	set_real_point3d(&p0, point->x - radius, point->y, point->z);
	set_real_point3d(&p1, point->x + radius, point->y, point->z);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);
	set_real_point3d(&p0, point->x, point->y - radius, point->z);
	set_real_point3d(&p1, point->x, point->y + radius, point->z);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);
	set_real_point3d(&p0, point->x, point->y, point->z - radius);
	set_real_point3d(&p1, point->x, point->y, point->z + radius);
	_rasterizer_debug_immediate_line(&p0, &p1, color, color);
	return;
}

void rasterizer_debug_immediate_vector(
	real_point3d const *point,
	real_vector3d const *vector,
	real scale,
	real_rgb_color const *color)
{
	real_point3d endpoint;

	endpoint.x = point->x + scale * vector->i;
	endpoint.y = point->y + scale * vector->j;
	endpoint.z = point->z + scale * vector->k;
	_rasterizer_debug_immediate_line(point, &endpoint, color, color);
	return;
}

void rasterizer_debug_model_vertices(
	long object_index,
	struct rasterizer_model_skinning const *skinning,
	struct rasterizer_model_geometry_part const *part)
{
	long debug_vertex_count;
	long closest_debug_vertex_index;
	real closest_debug_vertex_dot;
	short triangle_vertex_index;
	char model_vertex_string[256];
	char triangle_vertex_string[256];
	struct rasterizer_debug_model_vertex debug_vertices[MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTICES];

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
		830,
		part);

	if (rasterizer_debug_model_vertices_enabled &&
		object_index == rasterizer_model_obscurer_object_index)
	{
		debug_vertex_count = 0;
		closest_debug_vertex_index = NONE;
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
			857,
			part->triangle_buffer.type==_triangle_buffer_type_precompiled_strip);

		for (triangle_vertex_index = 0;
			triangle_vertex_index < part->triangle_buffer.count + 2;
			triangle_vertex_index++)
		{
			word model_vertex_index = ((word const *)part->triangles.address)[triangle_vertex_index];
			struct rasterizer_model_vertex_compressed const *vertex =
				&((struct rasterizer_model_vertex_compressed const *)part->compressed_vertices.address)[model_vertex_index];
			short node_index0;
			short node_index1;
			real node_weight0;
			real node_weight1;
			real_vector3d decompressed_normal;
			real_vector3d *decompressed_normal_result;
			real_vector3d vertex_normal;
			real_point3d point;
			real_vector3d normal;

			node_index0 = vertex->node_indices[0] / 3;
			node_weight0 = (real)vertex->node_weight * (1.f / 32767.f);
			node_weight1 = 1.f - node_weight0;
			node_index1 = vertex->node_indices[1] / 3;
			{
				real_point3d node_point0 = { 0.f, 0.f, 0.f };
				real_vector3d node_normal0 = { 0.f, 0.f, 0.f };
				real_point3d node_point1 = { 0.f, 0.f, 0.f };
				real_vector3d node_normal1 = { 0.f, 0.f, 0.f };
				long debug_vertex_index;

			decompressed_normal_result = uncompress_int32_to_real_vector3d(&decompressed_normal, vertex->normal);
			vertex_normal = *decompressed_normal_result;
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
				878,
				node_index0<skinning->node_matrix_count);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
				879,
				node_index1<skinning->node_matrix_count);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\rasterizer.c",
				880,
				node_weight0>=0.0f && node_weight0<=1.0f);

			if (node_index0 >= 0)
			{
				matrix4x3_transform_point(&skinning->node_matrices[node_index0], &vertex->position, &node_point0);
				matrix4x3_transform_vector(&skinning->node_matrices[node_index0], &vertex_normal, &node_normal0);
			}
			if (node_index1 >= 0)
			{
				matrix4x3_transform_point(&skinning->node_matrices[node_index1], &vertex->position, &node_point1);
				matrix4x3_transform_vector(&skinning->node_matrices[node_index1], &vertex_normal, &node_normal1);
			}

			point.x = node_point0.x * node_weight0 + node_point1.x * node_weight1;
			point.y = node_point0.y * node_weight0 + node_point1.y * node_weight1;
			point.z = node_point0.z * node_weight0 + node_point1.z * node_weight1;
			normal.i = node_normal0.i * node_weight0 + node_normal1.i * node_weight1;
			normal.j = node_normal0.j * node_weight0 + node_normal1.j * node_weight1;
			normal.k = node_normal0.k * node_weight0 + node_normal1.k * node_weight1;
			/* Preserve January VC7 scheduling: this inline self-copy is eliminated,
			 * but keeps the x87 result pop ahead of the loop-bound reload. */
			normalize3d(set_real_vector3d(
				&normal,
				normal.i,
				normal.j,
				normal.k));

			for (debug_vertex_index = 0; debug_vertex_index < debug_vertex_count; debug_vertex_index++)
			{
				struct rasterizer_debug_model_vertex *debug_vertex = &debug_vertices[debug_vertex_index];
				if (point.x == debug_vertex->position.x &&
					point.y == debug_vertex->position.y &&
					point.z == debug_vertex->position.z)
				{
					short reference_index;

					if (debug_vertex->triangle_vertex_index_count < MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTEX_REFERENCES)
					{
						for (reference_index = 0;
							reference_index < debug_vertex->triangle_vertex_index_count &&
							debug_vertex->triangle_vertex_indices[reference_index] != triangle_vertex_index;
							reference_index++)
						{
						}
						if (reference_index == debug_vertex->triangle_vertex_index_count)
						{
							debug_vertex->triangle_vertex_indices[debug_vertex->triangle_vertex_index_count++] = triangle_vertex_index;
						}
					}

					if (debug_vertex->model_vertex_index_count < MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTEX_REFERENCES)
					{
						for (reference_index = 0;
							reference_index < debug_vertex->model_vertex_index_count &&
							debug_vertex->model_vertex_indices[reference_index] != model_vertex_index;
							reference_index++)
						{
						}
						if (reference_index == debug_vertex->model_vertex_index_count)
						{
							debug_vertex->model_vertex_indices[debug_vertex->model_vertex_index_count++] = model_vertex_index;
						}
					}
					break;
				}
			}

			if (debug_vertex_index == debug_vertex_count &&
				debug_vertex_count < MAXIMUM_RASTERIZER_DEBUG_MODEL_VERTICES)
			{
				struct rasterizer_debug_model_vertex *debug_vertex = &debug_vertices[debug_vertex_count];
				real_vector3d camera_to_vertex;
				real camera_dot;

				debug_vertex->position = point;
				debug_vertex->triangle_vertex_indices[0] = triangle_vertex_index;
				debug_vertex->model_vertex_indices[0] = model_vertex_index;
				debug_vertex->triangle_vertex_index_count = 1;
				debug_vertex->model_vertex_index_count = 1;

				vector_from_points3d(
					&global_window_parameters.camera_position,
					&point,
					&camera_to_vertex);
				normalize3d(&camera_to_vertex);
				camera_dot = dot_product3d(
					&camera_to_vertex,
					&global_window_parameters.camera_forward);
				if ((dot_product3d(&camera_to_vertex, &normal) < 0.f &&
					closest_debug_vertex_dot < camera_dot) ||
					closest_debug_vertex_dot == -1.f)
				{
					closest_debug_vertex_index = debug_vertex_index;
					closest_debug_vertex_dot = camera_dot;
				}
				debug_vertex_count++;
			}
			}
		}

		{
			long debug_vertex_index;
			for (debug_vertex_index = 0; debug_vertex_index < debug_vertex_count; debug_vertex_index++)
			{
				struct rasterizer_debug_model_vertex *debug_vertex = &debug_vertices[debug_vertex_index];
				if (debug_vertex_index == closest_debug_vertex_index)
				{
					short reference_index;

					csstrcpy(temporary, "I=");
					for (reference_index = 0; reference_index < debug_vertex->triangle_vertex_index_count; reference_index++)
					{
						sprintf(
							triangle_vertex_string,
							"%d%c",
							debug_vertex->triangle_vertex_indices[reference_index],
							reference_index == debug_vertex->triangle_vertex_index_count - 1 ? ' ' : ',');
						csstrcat(temporary, triangle_vertex_string);
					}
					csstrcat(temporary, "\nV=");
					for (reference_index = 0; reference_index < debug_vertex->model_vertex_index_count; reference_index++)
					{
						sprintf(
							model_vertex_string,
							"%d%c",
							debug_vertex->model_vertex_indices[reference_index],
							reference_index == debug_vertex->model_vertex_index_count - 1 ? ' ' : ',');
						csstrcat(temporary, model_vertex_string);
					}
					render_debug_point(FALSE, &debug_vertex->position, 0.03125f, global_real_argb_red);
					render_debug_string_at_point(FALSE, &debug_vertex->position, temporary, global_real_argb_yellow);
				}
				else
				{
					render_debug_point(FALSE, &debug_vertex->position, 0.03125f, global_real_argb_white);
				}
			}
		}
	}
	return;
}

/* ---------- private code */

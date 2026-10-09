/*
RASTERIZER_XBOX_ENVIRONMENT.C

symbols in this file:
0014FCC0 01b0:
	_D3DDevice_SetRenderState (0000)
0014FE70 0050:
	_D3DDevice_SetTextureStageState (0000)
0014FEC0 0220:
	_IDirect3DDevice8_SetRenderState (0000)
001500E0 0010:
	_IDirect3DDevice8_SetTexture (0000)
001500F0 0060:
	_IDirect3DDevice8_SetTextureStageState (0000)
00150150 0010:
	_IDirect3DDevice8_SetVertexShaderConstant (0000)
00150160 0010:
	__rasterizer_environment_lightmap_end (0000)
00150170 0010:
	__rasterizer_environment_lightmaps_end (0000)
00150180 0010:
	__rasterizer_environment_diffuse_light_end (0000)
00150190 0010:
	__rasterizer_environment_diffuse_lights_end (0000)
001501A0 0020:
	__rasterizer_environment_diffuse_textures_end (0000)
001501C0 0010:
	__rasterizer_environment_specular_light_end (0000)
001501D0 0010:
	__rasterizer_environment_specular_lights_end (0000)
001501E0 0010:
	__rasterizer_environment_specular_lightmap_end (0000)
001501F0 0010:
	__rasterizer_environment_specular_lightmaps_end (0000)
00150200 0200:
	__rasterizer_environment_reflection_lightmap_mask_draw (0000)
00150400 0010:
	__rasterizer_environment_reflection_lightmap_mask_end (0000)
00150410 0010:
	__rasterizer_environment_reflection_lightmap_masks_end (0000)
00150420 0010:
	__rasterizer_environment_reflection_mirrors_begin (0000)
00150430 0010:
	__rasterizer_environment_reflection_mirrors_end (0000)
00150440 0010:
	__rasterizer_environment_reflections_begin (0000)
00150450 0010:
	__rasterizer_environment_reflections_end (0000)
00150460 0010:
	__rasterizer_environment_transparent_geometry_begin (0000)
00150470 0010:
	__rasterizer_environment_transparent_geometry_end (0000)
00150480 0190:
	__rasterizer_environment_lightmaps_begin (0000)
00150610 0190:
	__rasterizer_environment_lightmap_begin (0000)
001507A0 0fb0:
	__rasterizer_environment_lightmap_draw (0000)
00151750 02c0:
	__rasterizer_environment_diffuse_lights_begin (0000)
00151A10 03a0:
	__rasterizer_environment_diffuse_light_begin (0000)
00151DB0 0230:
	__rasterizer_environment_diffuse_light_draw (0000)
00151FE0 0190:
	__rasterizer_environment_diffuse_textures_begin (0000)
00152170 0670:
	__rasterizer_environment_diffuse_texture_draw (0000)
001527E0 0240:
	__rasterizer_environment_specular_lights_begin (0000)
00152A20 03c0:
	_rasterizer_environment_specular_spot_light_begin (0000)
00152DE0 0380:
	__rasterizer_environment_specular_light_begin (0000)
00153160 0330:
	__rasterizer_environment_specular_light_draw (0000)
00153490 03a0:
	__rasterizer_environment_specular_lightmaps_begin (0000)
00153830 00f0:
	__rasterizer_environment_specular_lightmap_begin (0000)
00153920 0310:
	__rasterizer_environment_specular_lightmap_draw (0000)
00153C30 01b0:
	__rasterizer_environment_reflection_lightmap_masks_begin (0000)
00153DE0 0100:
	__rasterizer_environment_reflection_lightmap_mask_begin (0000)
00153EE0 0660:
	__rasterizer_environment_reflection_mirror_draw (0000)
00154540 0730:
	__rasterizer_environment_reflection_draw (0000)
00154C70 0360:
	__rasterizer_environment_transparent_geometry_submit (0000)
0028FDE8 0007:
	??_C@_06BILHELBG@shader?$AA@ (0000)
0028FDF0 003d:
	??_C@_0DN@KFFMBFDK@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
0028FE30 0040:
	??_C@_0EA@HODFKFKF@?$CD?$CD?$CD?5ERROR?5unsupported?5drawing?5mo@ (0000)
0028FE70 002c:
	??_C@_0CM@DBKIGDIP@illumination?9?$DOplasma_animation_p@ (0000)
0028FE9C 002f:
	??_C@_0CP@OOCPMCFJ@illumination?9?$DOsecondary_animatio@ (0000)
0028FECC 002d:
	??_C@_0CN@JNMPABNC@illumination?9?$DOprimary_animation_@ (0000)
0028FEFC 000e:
	??_C@_0O@EFOAJPDM@vertex_buffer?$AA@ (0000)
0028FF0C 0028:
	??_C@_0CI@FGDOPIAK@light?9?$DOdefinition?9?$DOgel?4roll_peri@ (0000)
0028FF34 0029:
	??_C@_0CJ@LCKNANBH@light?9?$DOdefinition?9?$DOgel?4pitch_per@ (0000)
0028FF60 0027:
	??_C@_0CH@MPENKOEE@light?9?$DOdefinition?9?$DOgel?4yaw_perio@ (0000)
0028FF88 003c:
	??_C@_0DM@PEENBPOM@light_index?$DO?$DN0?5?$CG?$CG?5light_index?$DMra@ (0000)
0028FFC4 003f:
	??_C@_0DP@CDHCMCNJ@?$CD?$CD?$CD?5ERROR?5unsupported?5drawing?5mo@ (0000)
00290004 0039:
	??_C@_0DJ@PDBPKOHB@?$CD?$CD?$CD?5ERROR?5unsupported?5environmen@ (0000)
00290040 002e:
	??_C@_0CO@HGCEMJKB@?$CD?$CD?$CD?5ERROR?5unsupported?5environmen@ (0000)
00290070 000e:
	??_C@_0O@GPGHEPFO@light?9?$DOradius?$AA@ (0000)
00290080 0026:
	??_C@_0CG@IFHNBCPC@?$CD?$CD?$CD?5ERROR?5unsupported?5reflection@ (0000)
002900A8 0054:
	??_C@_0FE@BFENDLHD@reflection_type?$DO?$DN0?5?$CG?$CG?5reflection@ (0000)
002900FC 003e:
	??_C@_0DO@MCLDAE@?$CBTEST_FLAG?$CIgeometry_flags?0?5_rast@ (0000)
0029013C 0038:
	??_C@_0DI@EJBHOIIP@shader_type_is_valid_for_environ@ (0000)
00290174 002c:
	??_C@_0CM@HAIODBIC@shader?9?$DObase?4type?$CB?$DN_shader_type_@ (0000)
0030CEF8 0002:
	_specular_light_vertex_shader_permutation_index (0000)
00465A18 00b5:
	_rasterizer_environment_globals (0000)
*/

/* ---------- headers */

#define REAL_MATH_EXTERNAL_POINT_FROM_LINE3D
#include "cseries.h"
#include "bitmaps/bitmaps.h"
#include "cseries/errors.h"
#include "effects/decals.h"
#include "game/game_globals.h"
#include "interface/hud_draw.h"
#include "bitmaps/bitmaps_inlines.h"
#include "math/periodic_functions.h"
#include "math/real_math.h"
#undef REAL_MATH_EXTERNAL_POINT_FROM_LINE3D
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_console_vars.h"
#include "rasterizer/rasterizer_environment.h"
#include "rasterizer/common/rasterizer_common.h"
#include "rasterizer/rasterizer_geometry.h"
#include "rasterizer/rasterizer_lights.h"
/* Empty reconstructed header removed; no declarations to include. */
#include "rasterizer/rasterizer_transparent_geometry.h"
#include "shaders/shader_definitions.h"
#include "shaders/shaders.h"

#include <xtl.h>

#include "interface/progress_bar_internal.h"
#include "rasterizer_xbox.h"
#include "rasterizer/xbox/rasterizer_xbox_pixel_shader.h"
#include "rasterizer_xbox_draw_primitives.h"
#include "rasterizer_xbox_internal.h"

/* ---------- constants */

enum
{
	_rasterizer_drawing_mode_normal = 0,
	_rasterizer_drawing_mode_overdraw,
	_rasterizer_drawing_mode_bump_color,
	_rasterizer_drawing_mode_specular_mask,
	_rasterizer_drawing_mode_specular_mask_times_bump_color,
	_rasterizer_drawing_mode_diffuse_texture_times_bump_color,
	_rasterizer_drawing_mode_bump_edge,
	_rasterizer_drawing_mode_specular_mask_times_bump_edge,
	_rasterizer_drawing_mode_diffuse_texture_times_bump_edge,
	_rasterizer_drawing_mode_vectors,
	NUMBER_OF_RASTERIZER_DRAWING_MODES,
};

enum
{
	_shader_environment_alpha_tested_bit = 0,
	_shader_environment_bump_map_is_specular_mask_bit,
	_shader_environment_true_atmospheric_fog_bit,
};

enum
{
	_shader_environment_specular_overbright_bit = 0,
	_shader_environment_specular_extra_shiny_bit,
	_shader_environment_specular_lightmap_bit,
};

enum
{
	_shader_environment_reflection_dynamic_mirror_bit = 0,
};

enum
{
	_rasterizer_environment_vector_mode_test_pattern = 50,
	NUMBER_OF_RASTERIZER_ENVIRONMENT_VECTOR_MODES,
};

enum
{
	_shader_environment_self_illumination_unfiltered_bit = 0,
};

enum
{
	_shader_environment_reflection_type_bumped_cube_map = 0,
	_shader_environment_reflection_type_flat_cube_map,
	_shader_environment_reflection_type_bumped_radiosity,
	NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES,
};

enum
{
	_rasterizer_target_render_primary = 0,
};

enum
{
	_rasterizer_vertex_shader_environment_reflection = 42,
	_rasterizer_vertex_shader_environment_reflection_mirror = 51,
};

enum
{
	_shader_environment_diffuse_rescale_detail_maps_bit = 0,
	_shader_environment_diffuse_rescale_bump_map_bit,
};

enum
{
	_shader_environment_type_normal = 0,
	_shader_environment_type_blended,
	_shader_environment_type_blended_base_specular,
	NUMBER_OF_SHADER_ENVIRONMENT_TYPES,
};

enum
{
	_detail_map_function_double_biased_multiply = 0,
	_detail_map_function_multiply,
	_detail_map_function_double_biased_add,
	NUMBER_OF_DETAIL_MAP_FUNCTIONS,
};

enum
{
	_shader_type_environment = 3,
	_shader_type_transparent_water = 7,
	_rasterizer_statistics_mode_enabled = 2,
};

enum
{
	_rasterizer_geometry_no_sort_bit = 0,
	_rasterizer_geometry_no_queue_bit,
	_rasterizer_geometry_no_fog_bit,
};

enum
{
	_shader_transparent_water_draw_before_fog_bit = 3,
};

enum
{
	_d3d_texture_address_clamp = 3,
	_d3d_texture_state_address_u = 10,
	_d3d_texture_state_address_v = 11,
	_d3d_texture_state_mag_filter = 13,
	_d3d_texture_state_min_filter = 14,
	_d3d_texture_state_mip_filter = 15,
};

/* ---------- macros */

/* ---------- structures */

struct bitmap_data;

struct transparent_geometry_group
{
	unsigned long geometry_flags;
	long object_index;
	long source_object_index;
	struct shader const *shader;
	short shader_permutation_index;
	short pad12;
	short effect_type;
	byte reserved16[0x26];
	real_vector2d model_base_map_scale;
	long dynamic_triangle_buffer_index;
	long triangle_buffer_index;
	long first_triangle_index;
	long triangle_count;
	long dynamic_vertex_buffer_index;
	struct vertex_buffer const *vertex_buffers;
	struct bitmap_data const *lightmap;
	void const *node_matrices;
	short node_matrix_count;
	word pad66;
	void const *lighting;
	void const *animation;
	real z_sort;
	real_point3d centroid;
	real_plane3d plane;
	long sorted_index;
	short previous_group_presorted_index;
	short next_group_presorted_index;
	long active_camouflage_transparent_source_object_index;
	byte reserved9C;
	boolean cortana_hack;
	byte reserved9E[2];
};

struct rasterizer_environment_globals
{
	struct transparent_geometry_group local_group;
	real_rgb_color local_lightmap_ambient_color;
	boolean lightmap_missing;
	byte reservedAD[0x3];
	real specular_light_brightness;
};

struct point_light_geometry_parameters
{
	real radius;
	real radius_modifier_lower_bound;
	real radius_modifier_upper_bound;
	real falloff_angle;
	real cutoff_angle;
	real lens_flare_radius;
	real runtime_cosine_falloff_angle;
	real runtime_cosine_cutoff_angle;
	real specular_radius_multiplier;
	real runtime_sine_cutoff_angle;
	long unused[2];
};

struct point_light_gel_parameters
{
	struct tag_reference map;
	word pad0;
	short texture_animation_function;
	real texture_animation_rate;
	struct tag_reference secondary_map;
	word pad1;
	short yaw_function;
	real yaw_period;
	word pad2;
	short roll_function;
	real roll_period;
	word pad3;
	short pitch_function;
	real pitch_period;
	long unused[2];
};

struct point_light_definition
{
	unsigned long flags;
	struct point_light_geometry_parameters geometry;
	byte reserved34[0x30];
	struct point_light_gel_parameters gel;
};

struct shader_environment_diffuse_properties
{
	word flags;
	short pad02;
	byte reserved04[0x18];
	struct tag_reference base_map;
	byte reserved2C[0x18];
	short detail_map_function;
	short pad46;
	real primary_detail_map_scale;
	struct tag_reference primary_detail_map;
	real secondary_detail_map_scale;
	struct tag_reference secondary_detail_map;
	byte reserved70[0x18];
	short micro_detail_map_function;
	short pad8A;
	real micro_detail_map_scale;
	struct tag_reference micro_detail_map;
	real_rgb_color material_color;
	byte reservedAC[0xC];
	real bump_map_scale;
	struct tag_reference bump_map;
	real_vector2d runtime_bump_map_scale;
	byte reservedD4[0x40];
};

struct shader_environment_self_illumination_properties
{
	word flags;
	short pad02;
	byte reserved04[0x18];
	real_rgb_color primary_on_color;
	real_rgb_color primary_off_color;
	short primary_animation_function;
	short pad36;
	real primary_animation_period;
	real primary_animation_phase;
	byte reserved40[0x18];
	real_rgb_color secondary_on_color;
	real_rgb_color secondary_off_color;
	short secondary_animation_function;
	short pad72;
	real secondary_animation_period;
	real secondary_animation_phase;
	byte reserved7C[0x18];
	real_rgb_color plasma_on_color;
	real_rgb_color plasma_off_color;
	short plasma_animation_function;
	short padAE;
	real plasma_animation_period;
	real plasma_animation_phase;
	byte reservedB8[0x18];
	real map_scale;
	struct tag_reference map;
};

struct rasterizer_environment_vector_mode
{
	short vertex_shader_permutation_index;
	long final_combiner_input;
};

struct shader_environment_reflection_properties
{
	word flags;
	short type;
	real lightmap_brightness_scale;
	long unused1[7];
	real view_perpendicular_brightness;
	real view_parallel_brightness;
	long unused2[4];
	real mirror_index_of_refraction;
	real mirror_depth;
	long unused3[4];
	struct tag_reference cube_map;
	long unused4[4];
};

struct shader_environment_specular_properties
{
	word flags;
	short type;
	long unused04[4];
	real brightness;
	long unused18[5];
	real_rgb_color view_perpendicular_color;
	real_rgb_color view_parallel_color;
	long unused44[4];
};

struct shader_environment_properties
{
	word flags;
	short type;
	real lens_flare_spacing;
	struct tag_reference lens_flare;
	long unused[11];
	struct shader_environment_diffuse_properties diffuse;
	struct shader_environment_self_illumination_properties self_illumination;
	byte reserved23C[0x18];
	struct shader_environment_specular_properties specular;
	struct shader_environment_reflection_properties reflection;
};

struct shader_environment_definition
{
	struct shader shader;
	struct shader_environment_properties environment;
};

struct shader_transparent_water_definition
{
	struct shader shader;
	word flags;
};

/* ---------- prototypes */

static void rasterizer_environment_specular_spot_light_begin(
	long light_index);

/* ---------- globals */

static struct rasterizer_environment_globals rasterizer_environment_globals;
static boolean warned = FALSE;
extern struct pixel_shader_definition pixel_shader;
static short specular_light_vertex_shader_permutation_index= NONE;

/* ---------- public code */

void _rasterizer_environment_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		135,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_vectors)
	{
		if (vertex_buffer->type == _rasterizer_vertex_type_environment_compressed)
		{
			struct rasterizer_environment_vector_mode vector_modes[NUMBER_OF_RASTERIZER_ENVIRONMENT_VECTOR_MODES] =
			{
				{ 0, 8 },
				{ 0, 9 },
				{ 0, 10 },
				{ 0, 11 },
				{ 1, 8 },
				{ 1, 9 },
				{ 1, 10 },
				{ 1, 20 },
				{ 1, 11 },
				{ NONE, NONE },
				{ 0, 4 },
				{ 0, 5 },
				{ 1, 4 },
				{ 1, 5 },
				{ 2, 4 },
				{ 2, 5 },
				{ 3, 4 },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ 5, 5 },
				{ 2, 20 },
				{ 2, 21 },
				{ 2, 19 },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ 2, 8 },
				{ 2, 9 },
				{ 2, 10 },
				{ 2, 11 },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ 3, 5 },
				{ 4, 4 },
				{ 4, 5 },
				{ 5, 4 },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ NONE, NONE },
				{ 3, 12 },
			};
			real_vector4d texture_transform_constants[3];
			short vector_index;
			short vector_bitmap_index;
			boolean vector_test_pattern;

			vector_index = (short)(rasterizer_debug_options.pad3 % 1000);
			vector_bitmap_index = (short)(rasterizer_debug_options.pad3 / 1000);
			if (vector_index >= 0 &&
				vector_index < NUMBER_OF_RASTERIZER_ENVIRONMENT_VECTOR_MODES &&
				vector_modes[vector_index].vertex_shader_permutation_index != NONE)
			{
				vector_test_pattern =
					(vector_index == _rasterizer_environment_vector_mode_test_pattern);
				if (vector_test_pattern &&
					global_rasterizer_data->test[3].index != NONE)
				{
					rasterizer_set_texture_direct(0, global_rasterizer_data->test[3].index, 0);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				}
				else
				{
					rasterizer_set_texture_direct(
						0,
						global_rasterizer_data->vector_normalization.index,
						vector_bitmap_index);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				}
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture_direct(
					1,
					global_rasterizer_data->vector_normalization.index,
					vector_bitmap_index);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture_direct(
					2,
					global_rasterizer_data->vector_normalization.index,
					vector_bitmap_index);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				rasterizer_set_texture_direct(
					3,
					global_rasterizer_data->vector_normalization.index,
					vector_bitmap_index);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_COLORWRITEENABLE,
					D3DCOLORWRITEENABLE_RED |
					D3DCOLORWRITEENABLE_GREEN |
					D3DCOLORWRITEENABLE_BLUE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, FALSE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, TRUE);
				IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

				rasterizer_set_vertex_shader_permutation(
					37,
					vertex_buffer->type,
					vector_modes[vector_index].vertex_shader_permutation_index);

				csmemset(&pixel_shader, 0, sizeof(pixel_shader));
				pixel_shader.texture_modes = 0x00018C60 | (vector_test_pattern ? 1 : 3);
				if (vector_test_pattern)
				{
					texture_transform_constants[0].i = rasterizer_debug_options.pad3_scale;
					texture_transform_constants[0].j = 1.0f;
					texture_transform_constants[0].k = 1.0f;
					texture_transform_constants[0].l = 1.0f;
					texture_transform_constants[1].i = 1.0f;
					texture_transform_constants[1].j = 0.0f;
					texture_transform_constants[1].k = 0.0f;
					texture_transform_constants[1].l = 0.0f;
					texture_transform_constants[2].i = 0.0f;
					texture_transform_constants[2].j = 1.0f;
					texture_transform_constants[2].k = 0.0f;
					texture_transform_constants[2].l = 0.0f;
					IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, texture_transform_constants, 3);

					pixel_shader.combiner_count = 3;
					pixel_shader.constant_0[0] = 0x00FF0000;
					pixel_shader.constant_1[0] = 0x000000FF;
					pixel_shader.rgb_inputs[0] = 0x4849484A;
					pixel_shader.rgb_outputs[0] = 0x000030CD;
					pixel_shader.rgb_inputs[1] = 0x0C0C0D0D;
					pixel_shader.rgb_outputs[1] = 0x000000CD;
					pixel_shader.rgb_inputs[2] = 0x0C010D02;
					pixel_shader.rgb_outputs[2] = 0x00000C00;
					pixel_shader.final_combiner_inputs_abcd =
						vector_modes[vector_index].final_combiner_input | 0x18200000;
				}
				else
				{
					pixel_shader.combiner_count = 1;
					pixel_shader.final_combiner_inputs_abcd =
						vector_modes[vector_index].final_combiner_input;
				}
				rasterizer_set_pixel_shader(&pixel_shader);

				rasterizer_draw_dynamic_triangles_static_vertices2(
					dynamic_triangle_buffer_index,
					first_triangle_index,
					triangle_count,
					vertex_buffer,
					vertex_buffer + !rasterizer_environment_globals.lightmap_missing);
			}
		}
	}
	else if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		struct shader_environment_definition *shader_environment;
		struct shader_environment_self_illumination_properties const *illumination;
		real_rgb_color primary_color;
		real_rgb_color secondary_color;
		real_rgb_color plasma_on_color;
		real_rgb_color plasma_off_color;
		real primary_value;
		real secondary_value;
		real plasma_value;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			347,
			shader);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			348,
			vertex_buffer);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);
		rasterizer_set_vertex_shader_permutation(
			16,
			vertex_buffer->type,
			shader_get_vertex_shader_permutation(shader));

		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_ALPHATESTENABLE,
			TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_alpha_tested_bit) &&
				rasterizer_debug_options.environment_alpha_testing_enabled);
		rasterizer_set_texture(
			0,
			0,
			3,
			TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_bump_map_is_specular_mask_bit)
				? NONE
				: shader_environment->environment.diffuse.bump_map.index,
			bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		illumination = &shader_environment->environment.self_illumination;
		rasterizer_set_texture(1, 0, 0, illumination->map.index, bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		if (TEST_FLAG(
			illumination->flags,
			_shader_environment_self_illumination_unfiltered_bit))
		{
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_POINT);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		}
		else
		{
			SetTextureStageStateSmart(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			SetTextureStageStateSmart(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		}

		{
			real_vector4d texture_transform_constants[3];

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = illumination->map_scale;
			texture_transform_constants[0].l = 1.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, texture_transform_constants, 3);
		}

		if (illumination->map.index == NONE)
		{
			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.combiner_count = 2;
			pixel_shader.texture_modes =
				rasterizer_environment_globals.lightmap_missing
					? 0x00018001
					: 0x00018401;
			pixel_shader.rgb_inputs[0] =
				rasterizer_debug_options.lightmap_incident_radiosity_enabled
					? 0x484B0A01
					: 0x20200000;
			pixel_shader.rgb_outputs[0] = 0x0000208C;
			pixel_shader.alpha_inputs[1] = 0x34201408;
			pixel_shader.alpha_outputs[1] = 0x00000C00;
			pixel_shader.final_combiner_inputs_abcd = 0x0A0F000C;
			pixel_shader.final_combiner_inputs_efg = 0x1C011800;
		}
		else
		{
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				448,
				illumination->primary_animation_period!=0.0f);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				449,
				illumination->secondary_animation_period!=0.0f);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				450,
				illumination->plasma_animation_period!=0.0f);

			primary_value =
				periodic_function_evaluate(
					illumination->primary_animation_function,
					(global_frame_parameters.game_time_sec + illumination->primary_animation_phase) /
						illumination->primary_animation_period);
			secondary_value =
				periodic_function_evaluate(
					illumination->secondary_animation_function,
					(global_frame_parameters.game_time_sec + illumination->secondary_animation_phase) /
						illumination->secondary_animation_period);
			plasma_value =
				periodic_function_evaluate(
					illumination->plasma_animation_function,
					(global_frame_parameters.game_time_sec + illumination->plasma_animation_phase) /
						illumination->plasma_animation_period);

			scale_vector3d(
				(real_vector3d const *)&illumination->primary_off_color,
				1.0f - primary_value,
				(real_vector3d *)&primary_color);
			scale_vector3d(
				(real_vector3d const *)&illumination->secondary_off_color,
				1.0f - secondary_value,
				(real_vector3d *)&secondary_color);
			point_from_line3d(
				(real_point3d const *)&primary_color,
				(real_vector3d const *)&illumination->primary_on_color,
				primary_value,
				(real_point3d *)&primary_color);
			point_from_line3d(
				(real_point3d const *)&secondary_color,
				(real_vector3d const *)&illumination->secondary_on_color,
				secondary_value,
				(real_point3d *)&secondary_color);
			scale_vector3d(
				(real_vector3d const *)&illumination->plasma_on_color,
				1.0f,
				(real_vector3d *)&plasma_on_color);
			scale_vector3d(
				(real_vector3d const *)&illumination->plasma_off_color,
				1.0f,
				(real_vector3d *)&plasma_off_color);

			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.texture_modes =
				rasterizer_environment_globals.lightmap_missing
					? 0x00018021
					: 0x00018421;
			pixel_shader.combiner_count = 0x00011106;
			pixel_shader.constant_0[0] = real_alpha_to_pixel32(plasma_value);
			pixel_shader.alpha_inputs[0] = 0x1120B920;
			pixel_shader.alpha_outputs[0] = 0x00000C00;
			pixel_shader.rgb_inputs[0] = 0x1920B120;
			pixel_shader.rgb_outputs[0] = 0x00000C00;
			pixel_shader.alpha_inputs[1] = 0xDCDCCCCC;
			pixel_shader.alpha_outputs[1] = 0x00024C00;
			pixel_shader.rgb_inputs[1] =
				rasterizer_debug_options.lightmap_incident_radiosity_enabled
					? 0x484B0A01
					: 0x20200000;
			pixel_shader.rgb_outputs[1] = 0x00002080;
			pixel_shader.constant_0[2] = 0x00FF0000;
			pixel_shader.constant_1[2] = 0x0000FF00;
			pixel_shader.alpha_inputs[2] = 0x1C1C0920;
			pixel_shader.alpha_outputs[2] = 0x000000C9;
			pixel_shader.rgb_inputs[2] = 0x09010902;
			pixel_shader.rgb_outputs[2] = 0x000030CD;
			pixel_shader.constant_0[3] = real_rgb_color_to_pixel32(&primary_color);
			pixel_shader.constant_1[3] = real_rgb_color_to_pixel32(&secondary_color);
			pixel_shader.alpha_inputs[3] = 0x00005C5C;
			pixel_shader.alpha_outputs[3] = 0x00004C00;
			pixel_shader.rgb_inputs[3] = 0x0C010D02;
			pixel_shader.rgb_outputs[3] = 0x00000D00;
			pixel_shader.constant_0[4] = real_rgb_color_to_pixel32(&plasma_on_color);
			pixel_shader.constant_1[4] = real_rgb_color_to_pixel32(&plasma_off_color);
			pixel_shader.alpha_inputs[4] = 0x34201408;
			pixel_shader.alpha_outputs[4] = 0x00000C00;
			pixel_shader.rgb_inputs[4] = 0x011C0220;
			pixel_shader.rgb_outputs[4] = 0x00000C00;
			pixel_shader.rgb_inputs[5] = 0x0C190D20;
			pixel_shader.rgb_outputs[5] = 0x00000C00;
			pixel_shader.final_combiner_inputs_abcd = 0x0A0F000C;
			pixel_shader.final_combiner_inputs_efg = 0x1C011800;
		}
		pixel_shader.final_combiner_constant_0 =
			real_rgb_color_to_pixel32(&shader_environment->environment.diffuse.material_color);

		if (rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_no_lightmap)
		{
			real_vector4d lightmap_mode_constants[4];

			lightmap_mode_constants[0].i = 0.5f;
			lightmap_mode_constants[0].j = 0.6f;
			lightmap_mode_constants[0].k = 0.6f;
			lightmap_mode_constants[0].l = 1.0f;
			lightmap_mode_constants[1].i = 0.0f;
			lightmap_mode_constants[1].j = 0.0f;
			lightmap_mode_constants[1].k = 1.0f;
			lightmap_mode_constants[1].l = 0.1f;
			lightmap_mode_constants[2].i = 0.9f;
			lightmap_mode_constants[2].j = 0.9f;
			lightmap_mode_constants[2].k = 0.8f;
			lightmap_mode_constants[2].l = 0.0f;
			lightmap_mode_constants[3].i = 0.1f;
			lightmap_mode_constants[3].j = 0.1f;
			lightmap_mode_constants[3].k = 0.3f;
			lightmap_mode_constants[3].l = 0.0f;
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -81, lightmap_mode_constants, 4);
			pixel_shader.final_combiner_inputs_abcd = 0x2004000C;
		}
		else if (rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_fullbright ||
			rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_lightmap_bitmap)
		{
			pixel_shader.final_combiner_constant_1 =
				real_rgb_color_to_pixel32(&rasterizer_environment_globals.local_lightmap_ambient_color);
			pixel_shader.final_combiner_inputs_abcd = 0x2002000C;
		}

		switch (rasterizer_debug_options.drawing_mode)
		{
			case _rasterizer_drawing_mode_normal:
				break;

			case _rasterizer_drawing_mode_bump_color:
			case _rasterizer_drawing_mode_specular_mask_times_bump_color:
			case _rasterizer_drawing_mode_diffuse_texture_times_bump_color:
				pixel_shader.combiner_count = 1;
				pixel_shader.alpha_inputs[0] = 0;
				pixel_shader.alpha_outputs[0] = 0;
				pixel_shader.rgb_inputs[0] = 0;
				pixel_shader.rgb_outputs[0] = 0;
				pixel_shader.final_combiner_inputs_abcd = 0x00000008;
				break;

			case _rasterizer_drawing_mode_specular_mask:
				pixel_shader.combiner_count = 1;
				pixel_shader.alpha_inputs[0] = 0;
				pixel_shader.alpha_outputs[0] = 0;
				pixel_shader.rgb_inputs[0] = 0;
				pixel_shader.rgb_outputs[0] = 0;
				pixel_shader.final_combiner_inputs_abcd = 0x00000020;
				break;

			case _rasterizer_drawing_mode_bump_edge:
			case _rasterizer_drawing_mode_specular_mask_times_bump_edge:
			case _rasterizer_drawing_mode_diffuse_texture_times_bump_edge:
				pixel_shader.combiner_count = 1;
				pixel_shader.alpha_inputs[0] = 0x48402020;
				pixel_shader.alpha_outputs[0] = 0x00020D00;
				pixel_shader.rgb_inputs[0] = 0;
				pixel_shader.rgb_outputs[0] = 0;
				pixel_shader.final_combiner_inputs_abcd = 0x0000001D;
				break;

			default:
				match_vassert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
					575,
					FALSE,
					"### ERROR unsupported drawing mode in environment lightmap pass");
				break;
		}
		rasterizer_set_pixel_shader(&pixel_shader);

		rasterizer_draw_dynamic_triangles_static_vertices2(
			dynamic_triangle_buffer_index,
			first_triangle_index,
			triangle_count,
			vertex_buffer,
			vertex_buffer + !rasterizer_environment_globals.lightmap_missing);
		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
		{
			rasterizer_frame_statistics.lightmap_dynamic_draw_count++;
			rasterizer_frame_statistics.lightmap_dynamic_triangle_count += triangle_count;
			rasterizer_frame_statistics.lightmap_dynamic_vertex_count +=
				rasterizer_frame_statistics_count_dynamic_vertices(
					dynamic_triangle_buffer_index,
					first_triangle_index,
					triangle_count);
		}
	}
	return;
}

void _rasterizer_environment_lightmap_end(
	void)
{
	return;
}

void _rasterizer_environment_lightmaps_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_lightmaps);
	return;
}

void _rasterizer_environment_lightmaps_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x11,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_lightmaps);
	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ALPHAREF, 0x7F);
		D3DDevice_SetRenderState(D3DRS_ZENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);
	}
	return;
}

void _rasterizer_environment_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x44,
		global_d3d_device);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_lightmaps)
	{
		if (lightmap_bitmap)
		{
			rasterizer_set_texture_bitmap_data(2, lightmap_bitmap);
			D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			D3DDevice_SetTextureState_Deferred(2, D3DTSS_MAGFILTER, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(2, D3DTSS_MINFILTER, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(2, D3DTSS_MIPFILTER, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			rasterizer_environment_globals.lightmap_missing = FALSE;
		}
		else
		{
			IDirect3DDevice8_SetTexture(global_d3d_device, 2, NULL);
			rasterizer_environment_globals.lightmap_missing = TRUE;
		}

		if (rasterizer_globals.lightmap_mode > _rasterizer_lightmap_mode_normal)
		{
			if (rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_fullbright)
			{
				rasterizer_environment_globals.local_lightmap_ambient_color.red =
					rasterizer_environment_globals.local_lightmap_ambient_color.green =
					rasterizer_environment_globals.local_lightmap_ambient_color.blue =
						rasterizer_debug_options.lightmap_ambient;
			}
			else
			{
				unsigned long seed = (unsigned long)lightmap_bitmap;
				rasterizer_environment_globals.local_lightmap_ambient_color.red = real_seed_random(&seed);
				rasterizer_environment_globals.local_lightmap_ambient_color.green = real_seed_random(&seed);
				rasterizer_environment_globals.local_lightmap_ambient_color.blue = real_seed_random(&seed);
			}
		}
	}
	return;
}

void _rasterizer_environment_diffuse_light_end(
	void)
{
	return;
}

void _rasterizer_environment_diffuse_light_begin(
	long light_index)
{
	struct rasterizer_light_submit_parameters *light;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x2D3,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x2D9,
			light_index>=0 && light_index<rasterizer_lights.light_count);
		light = &rasterizer_lights.lights[light_index];
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x2DC,
			light->radius>0.0f);

		rasterizer_set_texture(1, 2, 1, light->definition->gel.map.index, 0);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x2EC,
			light->definition->gel.yaw_period>0.0f);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x2ED,
			light->definition->gel.pitch_period>0.0f);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x2EE,
			light->definition->gel.roll_period>0.0f);

		{
			real_matrix3x3 gel_matrix;

			{
				real yaw = periodic_function_evaluate(
					light->definition->gel.yaw_function,
					global_frame_parameters.game_time_sec / light->definition->gel.yaw_period) * (2.0f * _pi);
				real pitch = periodic_function_evaluate(
					light->definition->gel.pitch_function,
					global_frame_parameters.game_time_sec / light->definition->gel.pitch_period) * (2.0f * _pi);
				real roll = periodic_function_evaluate(
					light->definition->gel.roll_function,
					global_frame_parameters.game_time_sec / light->definition->gel.roll_period) * (2.0f * _pi);
				real_matrix4x3 gel_rotation;

				matrix4x3_rotation_from_angles(&gel_rotation, yaw, pitch, roll);
				matrix4x3_transform_normal(&gel_rotation, &light->forward, &gel_matrix.forward);
				matrix4x3_transform_normal(&gel_rotation, &light->up, &gel_matrix.up);
			}

			cross_product3d(&gel_matrix.forward, &gel_matrix.up, &gel_matrix.left);
			normalize3d(&gel_matrix.left);

			{
				real vertex_constants[20];
				vertex_constants[0] = light->position.x;
				vertex_constants[1] = light->position.y;
				vertex_constants[2] = light->position.z;
				vertex_constants[3] = 0.5f / light->radius;
				vertex_constants[4] = -gel_matrix.forward.i;
				vertex_constants[5] = -gel_matrix.forward.j;
				vertex_constants[6] = -gel_matrix.forward.k;
				vertex_constants[7] = 1.0f;
				vertex_constants[8] = -gel_matrix.left.i;
				vertex_constants[9] = -gel_matrix.left.j;
				vertex_constants[10] = -gel_matrix.left.k;
				vertex_constants[11] = 1.0f;
				vertex_constants[12] = -gel_matrix.up.i;
				vertex_constants[13] = -gel_matrix.up.j;
				vertex_constants[14] = -gel_matrix.up.k;
				vertex_constants[15] = 1.0f;
				vertex_constants[16] = 0.0f;
				vertex_constants[17] = 0.0f;
				vertex_constants[18] = 0.0f;
				vertex_constants[19] = 1.0f;
				D3DDevice_SetVertexShaderConstant(-81, vertex_constants, 5);
			}
		}

		D3DDevice_SetRenderState(
			D3DRS_PSCONSTANT0_0,
			real_rgb_color_to_pixel32(&light->color));
	}
	return;
}

void _rasterizer_environment_diffuse_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x333,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		struct shader_environment_definition *shader_environment;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x33A,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x33F,
			vertex_buffer);

		rasterizer_set_vertex_shader_permutation(
			49,
			vertex_buffer->type,
			shader_get_vertex_shader_permutation(shader));
		rasterizer_set_texture(
			0,
			0,
			3,
			TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_bump_map_is_specular_mask_bit)
					? NONE
					: shader_environment->environment.diffuse.bump_map.index,
			bitmap_index);
		D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		D3DDevice_SetTextureState_Deferred(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		{
			real_vector4d texture_transform_constants[3];

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 1.0f;
			texture_transform_constants[0].l = 1.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			D3DDevice_SetVertexShaderConstant(-84, texture_transform_constants, 3);
			D3DDevice_SetRenderState(
				D3DRS_PSFINALCOMBINERCONSTANT0,
				real_rgb_color_to_pixel32(&shader_environment->environment.diffuse.material_color));

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.environment_dynamic_draw_count++;
				rasterizer_frame_statistics.environment_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.environment_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_diffuse_lights_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x273,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_diffuse_lights);
	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_diffuse_lights)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->distance_attenuation.index, 0);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSW, D3DTADDRESS_BORDER);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MINFILTER, D3DTEXF_POINT);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MINFILTER, D3DTEXF_POINT);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MIPFILTER, D3DTEXF_POINT);

		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(
			D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE);
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		D3DDevice_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		D3DDevice_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		D3DDevice_SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ALPHAREF, 0);
		D3DDevice_SetRenderState(D3DRS_ZENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.texture_modes = 0x00018861;
		pixel_shader.combiner_count = 4;
		pixel_shader.alpha_inputs[0] = 0x4B204B20;
		pixel_shader.alpha_outputs[0] = 0x00020C00;
		pixel_shader.rgb_inputs[0] = 0x090A484B;
		pixel_shader.rgb_outputs[0] = 0x000010CD;
		pixel_shader.rgb_inputs[1] = 0x0C0D0000;
		pixel_shader.rgb_outputs[1] = 0x000000C0;
		pixel_shader.rgb_inputs[2] = 0x0C1C0000;
		pixel_shader.rgb_outputs[2] = 0x000000C0;
		pixel_shader.rgb_inputs[3] = 0x0C010C01;
		pixel_shader.rgb_outputs[3] = 0x000010CD;
		pixel_shader.final_combiner_inputs_abcd = 0x0C010000;
		pixel_shader.final_combiner_inputs_efg = 0x00000D00;
		rasterizer_set_pixel_shader(&pixel_shader);
	}
	return;
}

void _rasterizer_environment_diffuse_lights_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_diffuse_lights);
	return;
}

void _rasterizer_environment_diffuse_textures_end(
	void)
{
	rasterizer_set_stencil_mode(2);
	rasterizer_profile_end(_rasterizer_profile_environment_textures);
	return;
}

void _rasterizer_environment_diffuse_textures_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x399,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_textures);
	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_textures)
	{
		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(
			D3DRS_COLORWRITEENABLE,
			rasterizer_debug_options.environment_specular_mask_enabled
				? D3DCOLORWRITEENABLE_ALL
				: (D3DCOLORWRITEENABLE_RED |
					D3DCOLORWRITEENABLE_GREEN |
					D3DCOLORWRITEENABLE_BLUE));
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		D3DDevice_SetRenderState(
			D3DRS_SRCBLEND,
			rasterizer_debug_options.drawing_mode != _rasterizer_drawing_mode_overdraw
				? D3DBLEND_DESTCOLOR
				: D3DBLEND_ONE);
		D3DDevice_SetRenderState(
			D3DRS_DESTBLEND,
			rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw);
		D3DDevice_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		D3DDevice_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		D3DDevice_SetRenderState(
			D3DRS_ZENABLE,
			rasterizer_debug_options.drawing_mode != _rasterizer_drawing_mode_overdraw);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);
		rasterizer_set_stencil_mode(5);
	}
	return;
}

void _rasterizer_environment_diffuse_texture_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		968,
		global_d3d_device);

	if ((rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_overdraw ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_specular_mask_times_bump_edge ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_color ||
		rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_diffuse_texture_times_bump_edge) &&
		rasterizer_debug_options.draw_environment_textures)
	{
		struct shader_environment_definition *shader_environment;
		union point2d base_map_size;
		union point2d primary_detail_map_size;
		union point2d secondary_detail_map_size;
		union point2d micro_detail_map_size;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			981,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			995,
			vertex_buffer);

		rasterizer_set_vertex_shader_permutation(
			40,
			vertex_buffer->type,
			shader_get_vertex_shader_permutation(shader));

		base_map_size = *rasterizer_set_texture(
			0,
			0,
			1,
			shader_environment->environment.diffuse.base_map.index,
			bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		primary_detail_map_size = *rasterizer_set_texture(
			1,
			0,
			2,
			shader_environment->environment.diffuse.primary_detail_map.index,
			bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		secondary_detail_map_size = *rasterizer_set_texture(
			2,
			0,
			2,
			shader_environment->environment.diffuse.secondary_detail_map.index,
			bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		micro_detail_map_size = *rasterizer_set_texture(
			3,
			0,
			2,
			shader_environment->environment.diffuse.micro_detail_map.index,
			bitmap_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		{
			real_vector4d texture_transform_constants[3];
			real_vector2d primary_detail_map_scale;
			real_vector2d secondary_detail_map_scale;
			real_vector2d micro_detail_map_scale;

			if (TEST_FLAG(
				shader_environment->environment.diffuse.flags,
				_shader_environment_diffuse_rescale_detail_maps_bit))
			{
				primary_detail_map_scale.i = (real)base_map_size.x / (real)primary_detail_map_size.x;
				primary_detail_map_scale.j = (real)base_map_size.y / (real)primary_detail_map_size.y;
				secondary_detail_map_scale.i = (real)base_map_size.x / (real)secondary_detail_map_size.x;
				secondary_detail_map_scale.j = (real)base_map_size.y / (real)secondary_detail_map_size.y;
				micro_detail_map_scale.i = (real)base_map_size.x / (real)micro_detail_map_size.x;
				micro_detail_map_scale.j = (real)base_map_size.y / (real)micro_detail_map_size.y;
			}
			else
			{
				primary_detail_map_scale.i =
					primary_detail_map_scale.j =
					secondary_detail_map_scale.i =
					secondary_detail_map_scale.j =
					micro_detail_map_scale.i =
					micro_detail_map_scale.j = 1.0f;
			}

			texture_transform_constants[0].i =
				primary_detail_map_scale.i * shader_environment->environment.diffuse.primary_detail_map_scale;
			texture_transform_constants[0].j =
				primary_detail_map_scale.j * shader_environment->environment.diffuse.primary_detail_map_scale;
			texture_transform_constants[0].k =
				secondary_detail_map_scale.i * shader_environment->environment.diffuse.secondary_detail_map_scale;
			texture_transform_constants[0].l =
				secondary_detail_map_scale.j * shader_environment->environment.diffuse.secondary_detail_map_scale;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k =
				micro_detail_map_scale.i * shader_environment->environment.diffuse.micro_detail_map_scale;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k =
				micro_detail_map_scale.j * shader_environment->environment.diffuse.micro_detail_map_scale;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, texture_transform_constants, 3);

			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.texture_modes = 0x00008421;
			pixel_shader.combiner_count = 3;
			switch (shader_environment->environment.type)
			{
				case _shader_environment_type_normal:
					pixel_shader.alpha_inputs[0] = 0x3A1A1A19;
					pixel_shader.rgb_inputs[0] = 0x3A0A1A09;
					pixel_shader.alpha_inputs[1] = 0x181C0000;
					pixel_shader.alpha_outputs[1] = 0x000000C0;
					pixel_shader.alpha_outputs[0] = 0x00000C00;
					pixel_shader.rgb_outputs[0] = 0x00000C00;
					break;

				case _shader_environment_type_blended:
					pixel_shader.alpha_inputs[0] = 0x381A1819;
					pixel_shader.rgb_inputs[0] = 0x380A1809;
					pixel_shader.alpha_outputs[1] = 0;
					pixel_shader.alpha_outputs[0] = 0x00000C00;
					pixel_shader.rgb_outputs[0] = 0x00000C00;
					break;

				case _shader_environment_type_blended_base_specular:
					pixel_shader.alpha_inputs[0] = 0x18200000;
					pixel_shader.alpha_outputs[0] = 0x000000C0;
					pixel_shader.rgb_inputs[0] = 0x380A1809;
					pixel_shader.alpha_outputs[1] = 0;
					pixel_shader.rgb_outputs[0] = 0x00000C00;
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
						1145,
						FALSE,
						"### ERROR unsupported environment shader type");
					break;
			}

			switch (shader_environment->environment.diffuse.detail_map_function)
			{
				case _detail_map_function_double_biased_multiply:
					pixel_shader.rgb_inputs[1] = 0x080C080C;
					pixel_shader.rgb_outputs[1] = 0x00000C00;
					break;

				case _detail_map_function_multiply:
					pixel_shader.rgb_inputs[1] = 0x080C0000;
					pixel_shader.rgb_outputs[1] = 0x00000C00;
					break;

				case _detail_map_function_double_biased_add:
					pixel_shader.rgb_inputs[1] = 0x08204C20;
					pixel_shader.rgb_outputs[1] = 0x00000C00;
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
						1163,
						FALSE,
						"### ERROR unsupported environment shader detail function");
					break;
			}

			pixel_shader.alpha_inputs[2] = 0x1C1B0000;
			pixel_shader.alpha_outputs[2] = 0x000000C0;
			switch (shader_environment->environment.diffuse.micro_detail_map_function)
			{
				case _detail_map_function_double_biased_multiply:
					pixel_shader.rgb_inputs[2] = 0x0C0B0C0B;
					pixel_shader.rgb_outputs[2] = 0x00000C00;
					break;

				case _detail_map_function_multiply:
					pixel_shader.rgb_inputs[2] = 0x0C0B0000;
					pixel_shader.rgb_outputs[2] = 0x00000C00;
					break;

				case _detail_map_function_double_biased_add:
					pixel_shader.rgb_inputs[2] = 0x0C204B20;
					pixel_shader.rgb_outputs[2] = 0x00000C00;
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
						1184,
						FALSE,
						"### ERROR unsupported environment shader detail function");
					break;
			}

			pixel_shader.final_combiner_inputs_efg = 0x00001C00;
			switch (rasterizer_debug_options.drawing_mode)
			{
				case _rasterizer_drawing_mode_normal:
				case _rasterizer_drawing_mode_diffuse_texture_times_bump_color:
				case _rasterizer_drawing_mode_diffuse_texture_times_bump_edge:
					pixel_shader.final_combiner_inputs_abcd = 0x0000000C;
					break;

				case _rasterizer_drawing_mode_overdraw:
					pixel_shader.final_combiner_constant_0 = real_alpha_to_pixel32(0.33f);
					pixel_shader.final_combiner_inputs_abcd = 0x0C110000;
					break;

				case _rasterizer_drawing_mode_specular_mask:
				case _rasterizer_drawing_mode_specular_mask_times_bump_color:
				case _rasterizer_drawing_mode_specular_mask_times_bump_edge:
					pixel_shader.final_combiner_inputs_abcd = 0x0000001C;
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
						1206,
						FALSE,
						"### ERROR unsupported drawing mode in environment texture pass");
					break;
			}
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.diffuse_texture_dynamic_draw_count++;
				rasterizer_frame_statistics.diffuse_texture_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.diffuse_texture_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_specular_light_end(
	void)
{
	return;
}

void _rasterizer_environment_specular_lights_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x4E5,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_specular_lights);
	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lights)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(
			D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE);
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		D3DDevice_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		D3DDevice_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		D3DDevice_SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ALPHAREF, 0);
		D3DDevice_SetRenderState(D3DRS_ZENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);
	}
	return;
}

void _rasterizer_environment_specular_light_begin(
	long light_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x5C0,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct rasterizer_light_submit_parameters *light;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x5C6,
			light_index>=0 && light_index<rasterizer_lights.light_count);
		light = &rasterizer_lights.lights[light_index];
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x5C9,
			light->radius);

		if (light->definition->geometry.runtime_cosine_falloff_angle != -1.0f &&
			(light->definition->gel.map.index != NONE ||
			light->definition->gel.secondary_map.index != NONE))
		{
			specular_light_vertex_shader_permutation_index = 1;
			rasterizer_environment_globals.specular_light_brightness =
				real_rgb_color_brightness(&light->color);
			rasterizer_environment_specular_spot_light_begin(light_index);
		}
		else
		{
			real_vector4d vertex_constants[5];
			real inverse_radius;

			specular_light_vertex_shader_permutation_index = 0;
			rasterizer_environment_globals.specular_light_brightness =
				real_rgb_color_brightness(&light->color);
			inverse_radius = 1.0f /
				(light->definition->geometry.specular_radius_multiplier * light->radius);
			vertex_constants[0].i = light->position.x;
			vertex_constants[0].j = light->position.y;
			vertex_constants[0].k = light->position.z;
			vertex_constants[0].l = inverse_radius * 0.5f;
			vertex_constants[1].i = 0.0f;
			vertex_constants[1].j = 0.0f;
			vertex_constants[1].k = 0.0f;
			vertex_constants[1].l = 1.0f;
			vertex_constants[2].i = 0.0f;
			vertex_constants[2].j = 0.0f;
			vertex_constants[2].k = 0.0f;
			vertex_constants[2].l = 1.0f;
			vertex_constants[3].i = 0.0f;
			vertex_constants[3].j = 0.0f;
			vertex_constants[3].k = 0.0f;
			vertex_constants[3].l = 1.0f;
			vertex_constants[4].i = 0.0f;
			vertex_constants[4].j = 0.0f;
			vertex_constants[4].k = 0.0f;
			vertex_constants[4].l = 1.0f;
			D3DDevice_SetVertexShaderConstant(-81, vertex_constants, 5);

			rasterizer_set_texture_direct(
				1,
				global_rasterizer_data->distance_attenuation.index,
				0);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSU, D3DTADDRESS_BORDER);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSV, D3DTADDRESS_BORDER);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSW, D3DTADDRESS_BORDER);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.texture_modes = 0x00018C41;
			pixel_shader.combiner_count = 0x00011006;
			pixel_shader.constant_0[0] = 0x000000FF;
			pixel_shader.alpha_inputs[0] = 0x4B204B20;
			pixel_shader.alpha_outputs[0] = 0x00020400;
			pixel_shader.rgb_inputs[0] = 0x484A0000;
			pixel_shader.rgb_outputs[0] = 0x000020C0;
			pixel_shader.constant_0[1] = 0x000000FF;
			pixel_shader.alpha_inputs[1] = 0x4A204A20;
			pixel_shader.alpha_outputs[1] = 0x00020500;
			pixel_shader.rgb_inputs[1] = 0x48CC8A40;
			pixel_shader.rgb_outputs[1] = 0x00010D00;
			pixel_shader.alpha_inputs[2] = 0x2C120C11;
			pixel_shader.alpha_outputs[2] = 0x00000C00;
			pixel_shader.rgb_inputs[2] = 0xCD4B0809;
			pixel_shader.rgb_outputs[2] = 0x000020D0;
			pixel_shader.alpha_inputs[3] = 0x0D0D1415;
			pixel_shader.alpha_outputs[3] = 0x000000D5;
			pixel_shader.rgb_inputs[3] = 0x2C020C01;
			pixel_shader.rgb_outputs[3] = 0x00000C00;
			pixel_shader.alpha_inputs[4] = 0x1D1D151C;
			pixel_shader.alpha_outputs[4] = 0x000000D5;
			pixel_shader.rgb_inputs[4] = 0x0C091C09;
			pixel_shader.rgb_outputs[4] = 0x000110CD;
			pixel_shader.alpha_inputs[5] = 0x1D1D0000;
			pixel_shader.alpha_outputs[5] = 0x000000D0;
			pixel_shader.rgb_inputs[5] = 0x0C150D1D;
			pixel_shader.rgb_outputs[5] = 0x000010CD;
			pixel_shader.final_combiner_inputs_abcd = 0x0C0F0000;
			pixel_shader.final_combiner_inputs_efg = 0x1D200D00;
			rasterizer_set_pixel_shader(&pixel_shader);
		}
	}
	return;
}

void _rasterizer_environment_specular_light_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x660,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct shader_environment_definition *shader_environment;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x667,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);

		if (shader_environment->environment.specular.brightness > 0.0f &&
			rasterizer_environment_globals.specular_light_brightness > 0.0f)
		{
			real_vector4d texture_transform_constants[3];

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				0x66D,
				vertex_buffer);
			rasterizer_set_vertex_shader_permutation(
				21,
				vertex_buffer->type,
				specular_light_vertex_shader_permutation_index);
			rasterizer_set_texture(
				0,
				0,
				3,
				shader_environment->environment.diffuse.bump_map.index,
				bitmap_index);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 1.0f;
			texture_transform_constants[0].l = 1.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			D3DDevice_SetVertexShaderConstant(-84, texture_transform_constants, 3);

			pixel_shader.constant_0[2] = real_alpha_to_pixel32(
				rasterizer_environment_globals.specular_light_brightness *
				shader_environment->environment.specular.brightness);
			pixel_shader.constant_1[2] = real_alpha_to_pixel32(
				rasterizer_environment_globals.specular_light_brightness *
				shader_environment->environment.specular.brightness);
			pixel_shader.constant_0[3] =
				real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.constant_1[3] =
				real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_parallel_color);

			if (TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_bump_map_is_specular_mask_bit))
			{
				pixel_shader.rgb_inputs[0] = 0x014A0000;
				pixel_shader.rgb_inputs[1] = 0x01CC8A40;
				pixel_shader.rgb_outputs[2] = 0x000020D9;
			}
			else
			{
				pixel_shader.rgb_inputs[0] = 0x484A0000;
				pixel_shader.rgb_inputs[1] = 0x48CC8A40;
				pixel_shader.rgb_outputs[2] = 0x000020D0;
			}

			pixel_shader.rgb_outputs[4] =
				TEST_FLAG(
					shader_environment->environment.specular.flags,
					_shader_environment_specular_overbright_bit)
					? 0x000210CD
					: 0x000110CD;
			if (TEST_FLAG(
				shader_environment->environment.specular.flags,
				_shader_environment_specular_extra_shiny_bit))
			{
				pixel_shader.combiner_count = 0x00011008;
				pixel_shader.alpha_inputs[6] = 0x1D1D0000;
				pixel_shader.alpha_outputs[6] = 0x000000D0;
				pixel_shader.alpha_inputs[7] = 0x1D1D0000;
				pixel_shader.alpha_outputs[7] = 0x000000D0;
			}
			else
			{
				pixel_shader.combiner_count = 0x00011006;
				pixel_shader.alpha_inputs[6] = 0;
				pixel_shader.alpha_outputs[6] = 0;
				pixel_shader.alpha_inputs[7] = 0;
				pixel_shader.alpha_outputs[7] = 0;
			}
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.specular_light_dynamic_draw_count++;
				rasterizer_frame_statistics.specular_light_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.specular_light_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_specular_lights_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_specular_lights);
	return;
}

void _rasterizer_environment_specular_lightmap_end(
	void)
{
	return;
}

void _rasterizer_environment_specular_lightmaps_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_specular_lightmaps);
	return;
}

void _rasterizer_environment_specular_lightmaps_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x6F9,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_specular_lightmaps);
	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lightmaps &&
		rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal)
	{
		rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		rasterizer_set_texture_direct(3, global_rasterizer_data->vector_normalization.index, 0);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(
			D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE);
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		D3DDevice_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		D3DDevice_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		D3DDevice_SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ALPHAREF, 0);
		D3DDevice_SetRenderState(D3DRS_ZENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.texture_modes = 0x00018C21;
		pixel_shader.combiner_count = 0x00011006;
		pixel_shader.constant_0[0] = 0x800000FF;
		pixel_shader.alpha_inputs[0] = 0x4B204B20;
		pixel_shader.alpha_outputs[0] = 0x00020400;
		pixel_shader.rgb_inputs[0] = 0x484A0911;
		pixel_shader.rgb_outputs[0] = 0x000030C9;
		pixel_shader.constant_0[1] = 0x000000FF;
		pixel_shader.alpha_inputs[1] = 0x4A204A20;
		pixel_shader.alpha_outputs[1] = 0x00020500;
		pixel_shader.rgb_inputs[1] = 0x48CC8A40;
		pixel_shader.rgb_outputs[1] = 0x00010D00;
		pixel_shader.alpha_inputs[2] = 0x2C120C11;
		pixel_shader.alpha_outputs[2] = 0x00000C00;
		pixel_shader.rgb_inputs[2] = 0xCD4B0809;
		pixel_shader.rgb_outputs[2] = 0x000020D0;
		pixel_shader.alpha_inputs[3] = 0x0D0D1415;
		pixel_shader.alpha_outputs[3] = 0x000000D5;
		pixel_shader.rgb_inputs[3] = 0x2C020C01;
		pixel_shader.rgb_outputs[3] = 0x00000C00;
		pixel_shader.alpha_inputs[4] = 0x1D1D151C;
		pixel_shader.alpha_outputs[4] = 0x000000D5;
		pixel_shader.rgb_inputs[4] = 0x0C091C09;
		pixel_shader.rgb_outputs[4] = 0x000110CD;
		pixel_shader.alpha_inputs[5] = 0x1D1D0000;
		pixel_shader.alpha_outputs[5] = 0x000000D0;
		pixel_shader.rgb_inputs[5] = 0x0C150D1D;
		pixel_shader.rgb_outputs[5] = 0x000010CD;
		pixel_shader.final_combiner_constant_0 = 0x80000000;
		pixel_shader.final_combiner_inputs_abcd = 0x0C0F0000;
		pixel_shader.final_combiner_inputs_efg = 0x1D110D00;
		rasterizer_set_pixel_shader(&pixel_shader);
	}
	return;
}

void _rasterizer_environment_specular_lightmap_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		1874,
		global_d3d_device);

	if (!rasterizer_debug_options.drawing_mode && rasterizer_debug_options.draw_environment_specular_lightmaps)
	{
		if (lightmap_bitmap)
		{
			rasterizer_set_texture_bitmap_data(1, lightmap_bitmap);
			D3DDevice_SetTextureState_Deferred(1, _d3d_texture_state_address_u, _d3d_texture_address_clamp);
			D3DDevice_SetTextureState_Deferred(1, _d3d_texture_state_address_v, _d3d_texture_address_clamp);
			D3DDevice_SetTextureState_Deferred(1, _d3d_texture_state_mag_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(1, _d3d_texture_state_min_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(1, _d3d_texture_state_mip_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			rasterizer_environment_globals.lightmap_missing = FALSE;
		}
		else
		{
			rasterizer_environment_globals.lightmap_missing = TRUE;
		}
	}
	return;
}

void _rasterizer_environment_specular_lightmap_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x77B,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lightmaps &&
		rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal &&
		!rasterizer_environment_globals.lightmap_missing)
	{
		struct shader_environment_definition *shader_environment;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x784,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);

		if (shader_environment->environment.specular.brightness > 0.0f &&
			TEST_FLAG(
				shader_environment->environment.specular.flags,
				_shader_environment_specular_lightmap_bit))
		{
			real_vector4d texture_transform_constants[3];

			rasterizer_set_vertex_shader_permutation(21, vertex_buffer->type, 2);
			rasterizer_set_texture(
				0,
				0,
				3,
				shader_environment->environment.diffuse.bump_map.index,
				bitmap_index);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			D3DDevice_SetTextureState_Deferred(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 1.0f;
			texture_transform_constants[0].l = 1.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			D3DDevice_SetVertexShaderConstant(-84, texture_transform_constants, 3);

			pixel_shader.constant_0[2] =
				real_alpha_to_pixel32(shader_environment->environment.specular.brightness);
			pixel_shader.constant_1[2] =
				real_alpha_to_pixel32(shader_environment->environment.specular.brightness);
			pixel_shader.constant_0[3] =
				real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.constant_1[3] =
				real_rgb_color_to_pixel32(&shader_environment->environment.specular.view_parallel_color);

			if (TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_bump_map_is_specular_mask_bit))
			{
				pixel_shader.rgb_inputs[0] = 0x014A0911;
				pixel_shader.rgb_inputs[1] = 0x01CC8A40;
				pixel_shader.rgb_outputs[2] = 0x000020D9;
			}
			else
			{
				pixel_shader.rgb_inputs[0] = 0x484A0911;
				pixel_shader.rgb_inputs[1] = 0x48CC8A40;
				pixel_shader.rgb_outputs[2] = 0x000020D0;
			}

			pixel_shader.rgb_outputs[4] =
				TEST_FLAG(
					shader_environment->environment.specular.flags,
					_shader_environment_specular_overbright_bit)
					? 0x000210CD
					: 0x000110CD;
			if (TEST_FLAG(
				shader_environment->environment.specular.flags,
				_shader_environment_specular_extra_shiny_bit))
			{
				pixel_shader.combiner_count = 0x00011008;
				pixel_shader.alpha_inputs[6] = 0x1D1D0000;
				pixel_shader.alpha_outputs[6] = 0x000000D0;
				pixel_shader.alpha_inputs[7] = 0x1D1D0000;
				pixel_shader.alpha_outputs[7] = 0x000000D0;
			}
			else
			{
				pixel_shader.combiner_count = 0x00011006;
				pixel_shader.alpha_inputs[6] = 0;
				pixel_shader.alpha_outputs[6] = 0;
				pixel_shader.alpha_inputs[7] = 0;
				pixel_shader.alpha_outputs[7] = 0;
			}
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer,
				vertex_buffer + 1);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.specular_lightmap_dynamic_draw_count++;
				rasterizer_frame_statistics.specular_lightmap_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.specular_lightmap_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_reflection_lightmap_mask_end(
	void)
{
	return;
}

void _rasterizer_environment_reflection_lightmap_masks_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflection_lightmap_masks);
	return;
}

void _rasterizer_environment_reflection_lightmap_masks_begin(
	void)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x815,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_environment_reflection_lightmap_masks);
	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal)
	{
		D3DDevice_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		D3DDevice_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
		D3DDevice_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		D3DDevice_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		D3DDevice_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		D3DDevice_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZENABLE, TRUE);
		D3DDevice_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
		D3DDevice_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		D3DDevice_SetRenderState(D3DRS_ZBIAS, 0);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.texture_modes = 1;
		pixel_shader.combiner_count = 2;
		pixel_shader.constant_0[0] = 0x0080B050;
		pixel_shader.rgb_inputs[0] = 0x08010000;
		pixel_shader.rgb_outputs[0] = 0x000020C0;
		pixel_shader.alpha_inputs[1] = 0x2C120C20;
		pixel_shader.alpha_outputs[1] = 0x00000C00;
		pixel_shader.final_combiner_inputs_abcd = 0;
		pixel_shader.final_combiner_inputs_efg = 0x00001C00;
		rasterizer_set_pixel_shader(&pixel_shader);
	}
	return;
}

void _rasterizer_environment_reflection_lightmap_mask_begin(
	struct bitmap_data const *lightmap_bitmap)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		2120,
		global_d3d_device);

	if (!rasterizer_debug_options.drawing_mode &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		!rasterizer_globals.lightmap_mode)
	{
		if (lightmap_bitmap)
		{
			rasterizer_set_texture_bitmap_data(0, lightmap_bitmap);
			D3DDevice_SetTextureState_Deferred(0, _d3d_texture_state_address_u, _d3d_texture_address_clamp);
			D3DDevice_SetTextureState_Deferred(0, _d3d_texture_state_address_v, _d3d_texture_address_clamp);
			D3DDevice_SetTextureState_Deferred(0, _d3d_texture_state_mag_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(0, _d3d_texture_state_min_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			D3DDevice_SetTextureState_Deferred(0, _d3d_texture_state_mip_filter, (rasterizer_debug_options.lightmap_filtering_enabled != FALSE) + 1);
			rasterizer_environment_globals.lightmap_missing = FALSE;
		}
		else
		{
			rasterizer_environment_globals.lightmap_missing = TRUE;
		}
	}
	return;
}

void _rasterizer_environment_reflection_lightmap_mask_draw(
	struct shader const *shader,
	short shader_permutation_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x873,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_lightmap_masks &&
		rasterizer_debug_options.draw_environment_reflections &&
		rasterizer_globals.lightmap_mode == _rasterizer_lightmap_mode_normal &&
		!rasterizer_environment_globals.lightmap_missing)
	{
		struct shader_environment_definition *shader_environment;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x87D,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);

		if ((shader_environment->environment.reflection.view_perpendicular_brightness > 0.0f ||
			shader_environment->environment.reflection.view_parallel_brightness > 0.0f) &&
			shader_environment->environment.reflection.lightmap_brightness_scale < 1.0f)
		{
			real_vector4d texture_transform_constants[3];

			rasterizer_set_vertex_shader_permutation(58, vertex_buffer->type, 0);

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 1.0f;
			texture_transform_constants[0].l = 1.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			D3DDevice_SetVertexShaderConstant(-84, texture_transform_constants, 3);
			D3DDevice_SetRenderState(
				D3DRS_PSCONSTANT1_0,
				real_alpha_to_pixel32(
					shader_environment->environment.reflection.lightmap_brightness_scale));

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer,
				vertex_buffer + 1);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.reflection_mask_dynamic_draw_count++;
				rasterizer_frame_statistics.reflection_mask_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.reflection_mask_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_reflection_mirrors_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_reflection_mirrors);
	return;
}

void _rasterizer_environment_reflection_mirror_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		2271,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflection_mirrors &&
		global_window_parameters.has_mirror &&
		global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		struct shader_environment_definition *shader_environment;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			2280,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);

		if (TEST_FLAG(
			shader_environment->environment.reflection.flags,
			_shader_environment_reflection_dynamic_mirror_bit) &&
			(shader_environment->environment.reflection.view_perpendicular_brightness > 0.0f ||
			shader_environment->environment.reflection.view_parallel_brightness > 0.0f))
		{
			real_vector4d texture_transform_constants[3];
			real_rgb_color mirror_color;

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				2287,
				vertex_buffer);

			rasterizer_set_texture(
				0,
				0,
				3,
				shader_environment->environment.diffuse.bump_map.index,
				bitmap_index);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, 0);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_POINT);

			rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_POINT);

			rasterizer_set_target_as_texture(3, 1, 0);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
			IDirect3DDevice8_SetRenderState(
				global_d3d_device,
				D3DRS_COLORWRITEENABLE,
				D3DCOLORWRITEENABLE_RED |
				D3DCOLORWRITEENABLE_GREEN |
				D3DCOLORWRITEENABLE_BLUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_ONE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_EQUAL);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

			rasterizer_set_vertex_shader_permutation(
				_rasterizer_vertex_shader_environment_reflection_mirror,
				vertex_buffer->type,
				0);

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 320.0f;
			texture_transform_constants[0].l = 240.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, texture_transform_constants, 3);

			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.texture_modes = PS_TEXTUREMODES(
				PS_TEXTUREMODES_PROJECT2D,
				PS_TEXTUREMODES_CUBEMAP,
				PS_TEXTUREMODES_CUBEMAP,
				PS_TEXTUREMODES_PROJECT2D);
			pixel_shader.combiner_count = PS_COMBINERCOUNT(
				5,
				PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);
			if (shader_environment->environment.diffuse.bump_map.index == NONE)
			{
				mirror_color.red = PIN(0.5f - global_window_parameters.camera.forward.i * 0.5f, 0.0f, 1.0f);
				mirror_color.green = PIN(0.5f - global_window_parameters.camera.forward.j * 0.5f, 0.0f, 1.0f);
				mirror_color.blue = PIN(0.5f - global_window_parameters.camera.forward.k * 0.5f, 0.0f, 1.0f);
				pixel_shader.constant_0[0] = real_rgb_color_to_pixel32(&mirror_color);
				pixel_shader.rgb_inputs[0] = PS_COMBINERINPUTS(
					PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T3,
					PS_REGISTER_T3);
			}
			else
			{
				pixel_shader.rgb_inputs[0] = PS_COMBINERINPUTS(
					PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T3,
					PS_REGISTER_T3);
			}
			pixel_shader.rgb_outputs[0] = PS_COMBINEROUTPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.rgb_inputs[1] = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_R1);
			pixel_shader.rgb_outputs[1] = PS_COMBINEROUTPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_inputs[2] = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_R1);
			pixel_shader.rgb_outputs[2] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.constant_0[3] = real_a_rgb_color_to_pixel32(
				shader_environment->environment.reflection.view_perpendicular_brightness,
				&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.constant_1[3] = real_a_rgb_color_to_pixel32(
				shader_environment->environment.reflection.view_parallel_brightness,
				&shader_environment->environment.specular.view_parallel_color);
			pixel_shader.alpha_inputs[3] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_C1 | PS_CHANNEL_ALPHA,
				PS_REGISTER_R0,
				PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
			pixel_shader.alpha_outputs[3] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_inputs[3] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_C1,
				PS_REGISTER_R0,
				PS_REGISTER_C0);
			pixel_shader.rgb_outputs[3] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_inputs[4] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_R1,
				PS_REGISTER_R0,
				PS_REGISTER_T3);
			pixel_shader.rgb_outputs[4] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.final_combiner_inputs_abcd = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_EF_PROD,
				PS_REGISTER_ZERO,
				PS_REGISTER_ZERO);
			pixel_shader.final_combiner_inputs_efg = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_CHANNEL_ALPHA,
				TEST_FLAG(
					shader_environment->environment.flags,
					_shader_environment_bump_map_is_specular_mask_bit)
					? PS_REGISTER_T0
					: PS_REGISTER_ONE,
				PS_REGISTER_ZERO,
				0);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer);
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.reflection_dynamic_draw_count++;
				rasterizer_frame_statistics.reflection_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.reflection_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_reflection_mirrors_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflection_mirrors);
	return;
}

void _rasterizer_environment_reflections_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_reflections);
	return;
}

void _rasterizer_environment_reflection_draw(
	struct shader const *shader,
	short bitmap_index,
	long dynamic_triangle_buffer_index,
	long first_triangle_index,
	long triangle_count,
	struct vertex_buffer const *vertex_buffer)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		2462,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_reflections)
	{
		struct shader_environment_definition *shader_environment;
		short reflection_type;

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			2470,
			shader);
		shader_environment = (struct shader_environment_definition *)
			shader_get_and_verify_type((struct shader *)shader, _shader_type_environment);

		reflection_type = shader_environment->environment.reflection.type;
		if (reflection_type == _shader_environment_reflection_type_bumped_cube_map ||
			reflection_type == _shader_environment_reflection_type_bumped_radiosity)
		{
			if (TEST_FLAG(
				shader_environment->environment.flags,
				_shader_environment_bump_map_is_specular_mask_bit))
			{
				reflection_type = _shader_environment_reflection_type_flat_cube_map;
			}
			if (shader_environment->environment.diffuse.bump_map.index == NONE)
			{
				reflection_type = _shader_environment_reflection_type_flat_cube_map;
			}
		}

		if ((shader_environment->environment.reflection.view_perpendicular_brightness > 0.0f ||
			shader_environment->environment.reflection.view_parallel_brightness > 0.0f) &&
			shader_environment->environment.reflection.cube_map.index != NONE)
		{
			real_vector4d texture_transform_constants[3];
			real_rgb_color mirror_color;

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				2495,
				vertex_buffer);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
				2496,
				reflection_type>=0 && reflection_type<NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES);

			rasterizer_set_texture(
				0,
				0,
				3,
				shader_environment->environment.diffuse.bump_map.index,
				bitmap_index);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			rasterizer_set_texture_direct(1, global_rasterizer_data->vector_normalization.index, 0);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_POINT);

			rasterizer_set_texture_direct(2, global_rasterizer_data->vector_normalization.index, 0);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_POINT);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_POINT);

			rasterizer_set_texture(
				3,
				2,
				0,
				shader_environment->environment.reflection.cube_map.index,
				bitmap_index);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
			IDirect3DDevice8_SetTextureStageState(global_d3d_device, 3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_CCW);
			IDirect3DDevice8_SetRenderState(
				global_d3d_device,
				D3DRS_COLORWRITEENABLE,
				D3DCOLORWRITEENABLE_RED |
				D3DCOLORWRITEENABLE_GREEN |
				D3DCOLORWRITEENABLE_BLUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_ONE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_EQUAL);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
			IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

			rasterizer_set_vertex_shader_permutation(
				_rasterizer_vertex_shader_environment_reflection,
				vertex_buffer->type,
				reflection_type);

			texture_transform_constants[0].i = shader_environment->environment.diffuse.runtime_bump_map_scale.i;
			texture_transform_constants[0].j = shader_environment->environment.diffuse.runtime_bump_map_scale.j;
			texture_transform_constants[0].k = 320.0f;
			texture_transform_constants[0].l = 240.0f;
			texture_transform_constants[1].i = 1.0f;
			texture_transform_constants[1].j = 0.0f;
			texture_transform_constants[1].k = 0.0f;
			texture_transform_constants[1].l = 0.0f;
			texture_transform_constants[2].i = 0.0f;
			texture_transform_constants[2].j = 1.0f;
			texture_transform_constants[2].k = 0.0f;
			texture_transform_constants[2].l = 0.0f;
			shader_environment_texture_animation_evaluate(
				shader,
				global_frame_parameters.game_time_sec,
				&texture_transform_constants[1].l,
				&texture_transform_constants[2].l);
			IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, texture_transform_constants, 3);

			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			switch (reflection_type)
			{
				case _shader_environment_reflection_type_bumped_cube_map:
				case _shader_environment_reflection_type_bumped_radiosity:
					pixel_shader.texture_modes = PS_TEXTUREMODES(
						PS_TEXTUREMODES_PROJECT2D,
						PS_TEXTUREMODES_DOTPRODUCT,
						PS_TEXTUREMODES_DOT_RFLCT_DIFF,
						PS_TEXTUREMODES_DOT_RFLCT_SPEC);
					pixel_shader.input_texture = PS_INPUTTEXTURE(0, 0, 0, 0);
					pixel_shader.dot_mapping = PS_DOTMAPPING(
						PS_DOTMAPPING_ZERO_TO_ONE,
						PS_DOTMAPPING_MINUS1_TO_1_D3D,
						PS_DOTMAPPING_MINUS1_TO_1_D3D,
						PS_DOTMAPPING_MINUS1_TO_1_D3D);
					break;

				case _shader_environment_reflection_type_flat_cube_map:
					pixel_shader.texture_modes = PS_TEXTUREMODES(
						PS_TEXTUREMODES_PROJECT2D,
						PS_TEXTUREMODES_CUBEMAP,
						PS_TEXTUREMODES_CUBEMAP,
						PS_TEXTUREMODES_CUBEMAP);
					break;

				default:
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
						2592,
						FALSE,
						"### ERROR unsupported reflection type");
					break;
			}
			pixel_shader.combiner_count = PS_COMBINERCOUNT(
				5,
				PS_COMBINERCOUNT_UNIQUE_C0 | PS_COMBINERCOUNT_UNIQUE_C1);

			if (reflection_type == _shader_environment_reflection_type_bumped_cube_map ||
				reflection_type == _shader_environment_reflection_type_bumped_radiosity ||
				shader_environment->environment.diffuse.bump_map.index == NONE)
			{
				mirror_color.red = PIN(0.5f - global_window_parameters.camera.forward.i * 0.5f, 0.0f, 1.0f);
				mirror_color.green = PIN(0.5f - global_window_parameters.camera.forward.j * 0.5f, 0.0f, 1.0f);
				mirror_color.blue = PIN(0.5f - global_window_parameters.camera.forward.k * 0.5f, 0.0f, 1.0f);
				pixel_shader.constant_0[0] = real_rgb_color_to_pixel32(&mirror_color);
				pixel_shader.rgb_inputs[0] = PS_COMBINERINPUTS(
					PS_REGISTER_T2 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_C0 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T3,
					PS_REGISTER_T3);
			}
			else
			{
				pixel_shader.rgb_inputs[0] = PS_COMBINERINPUTS(
					PS_REGISTER_T1 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T0 | PS_INPUTMAPPING_EXPAND_NORMAL,
					PS_REGISTER_T3,
					PS_REGISTER_T3);
			}
			pixel_shader.rgb_inputs[1] = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_R1);
			pixel_shader.rgb_inputs[2] = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_R1);
			pixel_shader.rgb_outputs[0] = PS_COMBINEROUTPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_AB_DOT_PRODUCT);
			pixel_shader.rgb_outputs[1] = PS_COMBINEROUTPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_outputs[2] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_R1,
				PS_REGISTER_DISCARD,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.constant_0[3] = real_a_rgb_color_to_pixel32(
				shader_environment->environment.reflection.view_perpendicular_brightness,
				&shader_environment->environment.specular.view_perpendicular_color);
			pixel_shader.constant_1[3] = real_a_rgb_color_to_pixel32(
				shader_environment->environment.reflection.view_parallel_brightness,
				&shader_environment->environment.specular.view_parallel_color);
			pixel_shader.alpha_inputs[3] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_C1 | PS_CHANNEL_ALPHA,
				PS_REGISTER_R0,
				PS_REGISTER_C0 | PS_CHANNEL_ALPHA);
			pixel_shader.alpha_outputs[3] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_inputs[3] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_C1,
				PS_REGISTER_R0,
				PS_REGISTER_C0);
			pixel_shader.rgb_outputs[3] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.rgb_inputs[4] = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_INPUTMAPPING_UNSIGNED_INVERT,
				PS_REGISTER_R1,
				PS_REGISTER_R0,
				PS_REGISTER_T3);
			pixel_shader.rgb_outputs[4] = PS_COMBINEROUTPUTS(
				PS_REGISTER_DISCARD,
				PS_REGISTER_DISCARD,
				PS_REGISTER_R0,
				PS_COMBINEROUTPUT_IDENTITY);
			pixel_shader.final_combiner_inputs_abcd = PS_COMBINERINPUTS(
				PS_REGISTER_R0,
				PS_REGISTER_EF_PROD,
				PS_REGISTER_ZERO,
				PS_REGISTER_ZERO);
			pixel_shader.final_combiner_inputs_efg = PS_COMBINERINPUTS(
				PS_REGISTER_R0 | PS_CHANNEL_ALPHA,
				TEST_FLAG(
					shader_environment->environment.flags,
					_shader_environment_bump_map_is_specular_mask_bit)
					? PS_REGISTER_T0
					: PS_REGISTER_ONE,
				PS_REGISTER_ZERO,
				0);
			rasterizer_set_pixel_shader(&pixel_shader);

			rasterizer_draw_dynamic_triangles_static_vertices2(
				dynamic_triangle_buffer_index,
				first_triangle_index,
				triangle_count,
				vertex_buffer,
				vertex_buffer +
					(rasterizer_globals.lightmap_mode != _rasterizer_lightmap_mode_normal &&
					reflection_type == _shader_environment_reflection_type_bumped_radiosity));
			if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
			{
				rasterizer_frame_statistics.reflection_dynamic_draw_count++;
				rasterizer_frame_statistics.reflection_dynamic_triangle_count += triangle_count;
				rasterizer_frame_statistics.reflection_dynamic_vertex_count +=
					rasterizer_frame_statistics_count_dynamic_vertices(
						dynamic_triangle_buffer_index,
						first_triangle_index,
						triangle_count);
			}
		}
	}
	return;
}

void _rasterizer_environment_reflections_end(
	void)
{
	rasterizer_profile_end(_rasterizer_profile_environment_reflections);
	return;
}

void _rasterizer_environment_transparent_geometry_begin(
	void)
{
	rasterizer_profile_begin(_rasterizer_profile_environment_transparents);
	rasterizer_transparent_geometry_groups_begin();
	return;
}

void _rasterizer_environment_transparent_geometry_submit(
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
	unsigned long geometry_flags)
{
	real_vector3d camera_to_centroid;
	real_plane3d null_plane;
	real_plane3d const *group_plane;
	unsigned long no_queue_flag;
	struct transparent_geometry_group *group;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0xA8B,
		global_d3d_device);

	if (rasterizer_debug_options.draw_environment_transparent_geometry)
	{
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0xA92,
			shader);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0xA93,
			shader->base.type!=_shader_type_environment);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0xA94,
			shader_type_is_valid_for_environment(shader->base.type));
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0xA95,
			centroid);

		camera_to_centroid.i = centroid->x - global_window_parameters.camera.position.x;
		camera_to_centroid.j = centroid->y - global_window_parameters.camera.position.y;
		camera_to_centroid.k = centroid->z - global_window_parameters.camera.position.z;
		if (plane)
			geometry_flags |= FLAG(_rasterizer_geometry_no_sort_bit);
		if (shader_is_decal((struct shader *)shader))
		{
			geometry_flags |= FLAG(_rasterizer_geometry_no_sort_bit) |
				FLAG(_rasterizer_geometry_no_queue_bit) |
				FLAG(_rasterizer_geometry_no_fog_bit);
		}

		no_queue_flag = geometry_flags & FLAG(_rasterizer_geometry_no_queue_bit);
		if (no_queue_flag)
		{
			group = &rasterizer_environment_globals.local_group;
			group->sorted_index = NONE;
		}
		else
		{
			group = rasterizer_transparent_geometry_new_group();
		}

		if (group)
		{
		group->geometry_flags = geometry_flags;
		group->shader = shader;
		group->object_index = 0;
		group->source_object_index = 0;
		group->effect_type = 0;
		group->shader_permutation_index = shader_permutation_index;
		group->dynamic_triangle_buffer_index = dynamic_triangle_buffer_index;
		group->triangle_buffer_index = 0;
		group->first_triangle_index = first_triangle_index;
		group->triangle_count = triangle_count;
		group->dynamic_vertex_buffer_index = NONE;
		group->vertex_buffers = vertex_buffers;
		group->lightmap = lightmap;
		null_plane.n.i = 0.0f;
		null_plane.n.j = 0.0f;
		null_plane.n.k = 0.0f;
		null_plane.d = 0.0f;
		group->z_sort = -(
			global_window_parameters.camera.forward.i * camera_to_centroid.i +
			global_window_parameters.camera.forward.j * camera_to_centroid.j +
			global_window_parameters.camera.forward.k * camera_to_centroid.k);
		group->centroid = *centroid;

		if (plane)
		{
			group_plane = plane;
		}
		else
		{
			group_plane = &null_plane;
		}
		group->plane = *group_plane;

		group->model_base_map_scale.i = group->model_base_map_scale.j = 1.0f;
		group->next_group_presorted_index = group->previous_group_presorted_index = NONE;
		group->active_camouflage_transparent_source_object_index = 0;
		group->cortana_hack = FALSE;
		group->node_matrices = NULL;
		group->node_matrix_count = 0;
		group->lighting = rasterizer_memory_alloc_const(lighting, 0x74);
		group->animation = NULL;

		if (shader->base.type == _shader_type_transparent_water)
			rasterizer_water_set_visibility_for_window(TRUE);
		if (shader->base.type == _shader_type_transparent_water &&
			TEST_FLAG(
				((struct shader_transparent_water_definition *)shader_get_and_verify_type(
					(struct shader *)shader,
					_shader_type_transparent_water))->flags,
				_shader_transparent_water_draw_before_fog_bit))
		{
				match_assert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
					0xADE,
					!TEST_FLAG(geometry_flags, _rasterizer_geometry_no_queue_bit));
				SET_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit, TRUE);
				rasterizer_transparent_geometry_group_draw(group, FALSE);
				rasterizer_transparent_geometry_set_group_pending_status(group, TRUE);
				SET_FLAG(group->geometry_flags, _rasterizer_geometry_no_queue_bit, FALSE);
		}
		else if (no_queue_flag)
		{
			rasterizer_transparent_geometry_group_draw(group, FALSE);
		}

		if (rasterizer_debug_options.statistics_mode == _rasterizer_statistics_mode_enabled)
		{
			rasterizer_frame_statistics.transparent_geometry_dynamic_draw_count++;
			rasterizer_frame_statistics.transparent_geometry_dynamic_triangle_count += triangle_count;
			if (triangle_count > rasterizer_frame_statistics.transparent_geometry_largest_dynamic_triangle_count)
			{
				rasterizer_frame_statistics.transparent_geometry_largest_dynamic_triangle_count = triangle_count;
			}
			rasterizer_frame_statistics.transparent_geometry_dynamic_vertex_count +=
				rasterizer_frame_statistics_count_dynamic_vertices(
					dynamic_triangle_buffer_index,
					first_triangle_index,
					triangle_count);
		}
		}
		else if (!warned)
		{
			error(_error_silent, "### ERROR too many transparent geometry groups");
			warned = TRUE;
		}
	}
	(void)offset;
	return;
}

void _rasterizer_environment_transparent_geometry_end(
	void)
{
	rasterizer_transparent_geometry_groups_end();
	rasterizer_profile_end(_rasterizer_profile_environment_transparents);
	return;
}

/* ---------- private code */

static void rasterizer_environment_specular_spot_light_begin(
	long light_index)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
		0x51F,
		global_d3d_device);

	if (rasterizer_debug_options.drawing_mode == _rasterizer_drawing_mode_normal &&
		rasterizer_debug_options.draw_environment_specular_lights)
	{
		struct rasterizer_light_submit_parameters *light;
		real_matrix4x3 light_matrix;
		long gel_bitmap_index;
		real radius;
		real inner_radius;
		real cone_scale;
		real_vector4d vertex_constants[5];

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x525,
			light_index>=0 && light_index<rasterizer_lights.light_count);
		light = &rasterizer_lights.lights[light_index];
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_environment.c",
			0x528,
			light->radius>0.0f);

		gel_bitmap_index = light->definition->gel.map.index;
		if (gel_bitmap_index == NONE)
			gel_bitmap_index = light->definition->gel.secondary_map.index;

		light_matrix.forward = light->forward;
		light_matrix.up = light->up;
		cross_product3d(&light->forward, &light->up, &light_matrix.left);
		normalize3d(&light_matrix.left);
		radius = light->definition->geometry.specular_radius_multiplier * light->radius;
		inner_radius = radius * 0.5f;
		cone_scale = 1.0f / (radius - inner_radius);

		vertex_constants[0].i = light->position.x;
		vertex_constants[0].j = light->position.y;
		vertex_constants[0].k = light->position.z;
		vertex_constants[0].l = 0.5f / radius;
		vertex_constants[1].i = -light_matrix.forward.i;
		vertex_constants[1].j = -light_matrix.forward.j;
		vertex_constants[1].k = -light_matrix.forward.k;
		vertex_constants[1].l = 1.0f;
		vertex_constants[2].i = -light_matrix.left.i;
		vertex_constants[2].j = -light_matrix.left.j;
		vertex_constants[2].k = -light_matrix.left.k;
		vertex_constants[2].l = 1.0f;
		vertex_constants[3].i = -light_matrix.up.i;
		vertex_constants[3].j = -light_matrix.up.j;
		vertex_constants[3].k = -light_matrix.up.k;
		vertex_constants[3].l = 1.0f;
		vertex_constants[4].i = light_matrix.forward.i * cone_scale;
		vertex_constants[4].j = light_matrix.forward.j * cone_scale;
		vertex_constants[4].k = light_matrix.forward.k * cone_scale;
		vertex_constants[4].l = -cone_scale * inner_radius;
		D3DDevice_SetVertexShaderConstant(-81, vertex_constants, 5);

		rasterizer_set_texture(1, 2, 1, gel_bitmap_index, 0);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice_SetTextureState_Deferred(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.texture_modes = 0x00018C61;
		pixel_shader.combiner_count = 0x00011006;
		pixel_shader.constant_0[0] = 0x000000FF;
		pixel_shader.alpha_inputs[0] = 0x4B204B20;
		pixel_shader.alpha_outputs[0] = 0x00020400;
		pixel_shader.rgb_inputs[0] = 0x484A0000;
		pixel_shader.rgb_outputs[0] = 0x000020C0;
		pixel_shader.constant_0[1] = 0x000000FF;
		pixel_shader.alpha_inputs[1] = 0x4A204A20;
		pixel_shader.alpha_outputs[1] = 0x00020500;
		pixel_shader.rgb_inputs[1] = 0x48CC8A40;
		pixel_shader.rgb_outputs[1] = 0x00010D00;
		pixel_shader.alpha_inputs[2] = 0x2C120C11;
		pixel_shader.alpha_outputs[2] = 0x00000C00;
		pixel_shader.rgb_inputs[2] = 0xCD4B0809;
		pixel_shader.rgb_outputs[2] = 0x000020D0;
		pixel_shader.alpha_inputs[3] = 0x0D0D1415;
		pixel_shader.alpha_outputs[3] = 0x000000D5;
		pixel_shader.rgb_inputs[3] = 0x2C020C01;
		pixel_shader.rgb_outputs[3] = 0x00000C00;
		pixel_shader.alpha_inputs[4] = 0x1D1D151C;
		pixel_shader.alpha_outputs[4] = 0x000000D5;
		pixel_shader.rgb_inputs[4] = 0x0C091C09;
		pixel_shader.rgb_outputs[4] = 0x000110CD;
		pixel_shader.alpha_inputs[5] = 0x1D1D0000;
		pixel_shader.alpha_outputs[5] = 0x000000D0;
		pixel_shader.rgb_inputs[5] = 0x0C150D1D;
		pixel_shader.rgb_outputs[5] = 0x000010CD;
		pixel_shader.final_combiner_inputs_abcd = 0x0C0F0000;
		pixel_shader.final_combiner_inputs_efg = 0x1D330D00;
		rasterizer_set_pixel_shader(&pixel_shader);
	}
	return;
}

/*
RASTERIZER_XBOX_PLASMA_ENERGY.C

symbols in this file:
0015E2B0 01b0:
	_code_0015e2b0 (0000)
0015E460 0050:
	_code_0015e460 (0000)
0015E4B0 0220:
	_code_0015e4b0 (0000)
0015E6D0 0060:
	_code_0015e6d0 (0000)
0015E730 0010:
	_code_0015e730 (0000)
0015E740 0590:
	_rasterizer_plasma_energy_draw (0000)
00291F10 0004:
	__real@3a03126f (0000)
00291F14 0033:
	??_C@_0DD@OEJHPIIG@plasma?9?$DOsecondary_noise_map_anim@ (0000)
00291F48 0031:
	??_C@_0DB@CGFOFGKP@plasma?9?$DOprimary_noise_map_animat@ (0000)
00291F7C 003f:
	??_C@_0DP@HOMIPMOO@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "real_math.h"
#include "shaders/shader_definitions.h"
#include <stddef.h>

/* The January translation unit retains the XDK's out-of-line D3D wrappers.
 * Keep the stock D3DINLINE definition: the real calls below make VC7 emit
 * the target's complete wrapper bodies. */
#include <xtl.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct rasterizer_debug_options_plasma
{
	byte reserved00[0x43];
	boolean plasma_energy;
};

struct rasterizer_frame_begin_parameters
{
	real game_time_sec;
	real dt;
};

struct plasma_runtime_parameters
{
	real_rgb_color const *colors;
	real const *exponents;
};

struct rasterizer_transparent_geometry_group_plasma
{
	byte reserved00[0xC];
	struct shader *shader;
	short bitmap_sequence_index;
	byte reserved12[0x5A];
	struct plasma_runtime_parameters const *runtime_parameters;
};

struct shader_transparent_plasma_definition
{
	byte reserved00[4];
	short intensity_exponent_source;
	short pad06;
	real intensity_exponent;
	short offset_exponent_source;
	short pad0E;
	real offset_amount;
	real offset_exponent;
	byte reserved40[0x20];
	real perpendicular_alpha;
	real_rgb_color perpendicular_color;
	real parallel_alpha;
	real_rgb_color parallel_color;
	short color_source;
	byte reserved82[0x3E];
	real primary_noise_map_animation_period;
	real_vector3d primary_noise_map_animation_direction;
	real primary_noise_map_scale;
	byte reservedAC[0xC];
	long primary_noise_map;
	byte reservedBC[0x24];
	real secondary_noise_map_animation_period;
	real_vector3d secondary_noise_map_animation_direction;
	real secondary_noise_map_scale;
	byte reservedF4[0xC];
	long secondary_noise_map;
};

struct pixel_shader_definition
{
	unsigned long alpha_inputs[8];
	unsigned long final_combiner_inputs_abcd;
	unsigned long final_combiner_inputs_efg;
	unsigned long constant_0[8];
	unsigned long constant_1[8];
	unsigned long alpha_outputs[8];
	unsigned long rgb_inputs[8];
	unsigned long compare_mode;
	unsigned long final_combiner_constant_0;
	unsigned long final_combiner_constant_1;
	unsigned long rgb_outputs[8];
	unsigned long combiner_count;
	unsigned long texture_modes;
	unsigned long dot_mapping;
	unsigned long input_texture;
	unsigned long c0_mapping;
	unsigned long c1_mapping;
	unsigned long final_combiner_constants;
};

typedef char plasma_group_size_assert[
	sizeof(struct rasterizer_transparent_geometry_group_plasma) == 0x70 ? 1 : -1];
typedef char plasma_group_shader_offset_assert[
	offsetof(struct rasterizer_transparent_geometry_group_plasma, shader) == 0xC ? 1 : -1];
typedef char plasma_group_runtime_offset_assert[
	offsetof(struct rasterizer_transparent_geometry_group_plasma, runtime_parameters) == 0x6C ? 1 : -1];
typedef char plasma_primary_period_offset_assert[
	offsetof(struct shader_transparent_plasma_definition, primary_noise_map_animation_period) == 0x98 ? 1 : -1];
typedef char plasma_primary_bitmap_offset_assert[
	offsetof(struct shader_transparent_plasma_definition, primary_noise_map) == 0xB8 ? 1 : -1];
typedef char plasma_secondary_period_offset_assert[
	offsetof(struct shader_transparent_plasma_definition, secondary_noise_map_animation_period) == 0xE0 ? 1 : -1];
typedef char plasma_secondary_bitmap_offset_assert[
	offsetof(struct shader_transparent_plasma_definition, secondary_noise_map) == 0x100 ? 1 : -1];
typedef char pixel_shader_definition_size_assert[
	sizeof(struct pixel_shader_definition) == 0xF0 ? 1 : -1];

/* ---------- prototypes */

double pow(
	double x,
	double y);

void rasterizer_set_texture(
	short stage,
	short bitmap_type,
	short bitmap_index,
	long bitmap_definition_index,
	short bitmap_sequence_index);

short rasterizer_transparent_geometry_get_primary_vertex_type(
	struct rasterizer_transparent_geometry_group_plasma const *group);

void rasterizer_set_vertex_shader_permutation(
	short vertex_type,
	short permutation,
	boolean one_node);

void rasterizer_set_pixel_shader(
	struct pixel_shader_definition const *definition);

void rasterizer_transparent_geometry_group_draw__internal(
	struct rasterizer_transparent_geometry_group_plasma const *group,
	boolean dirty);

/* ---------- globals */

extern void *global_d3d_device;
extern struct rasterizer_debug_options_plasma rasterizer_debug_options;
extern struct rasterizer_frame_begin_parameters global_frame_parameters;
extern struct pixel_shader_definition pixel_shader;

/* ---------- public code */

/* January uses three-axis texture wrapping and additive plasma blending.
 * The initialization order below also preserves VC7's original x87 schedule. */
void rasterizer_plasma_energy_draw(
	struct rasterizer_transparent_geometry_group_plasma const *group)
{
	struct shader_transparent_plasma_definition const *plasma;
	struct plasma_runtime_parameters const *runtime;
	real_rgb_color const *tint;
	real intensity;
	real offset;
	real primary_time;
	real secondary_time;
	real primary_scale;
	real secondary_scale;
	real vertex_constants[6][4];
	real color_constants[3][4];
	short source;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c",
		21,
		global_d3d_device);
	if (rasterizer_debug_options.plasma_energy)
	{
		plasma = (struct shader_transparent_plasma_definition const *)(
			(byte *)shader_get_and_verify_type(group->shader, 10) + sizeof(struct shader));
		tint = global_real_rgb_white;
		intensity = 1.0f;
		offset = 0.0f;
		runtime = group->runtime_parameters;
		if (runtime)
		{
			if (runtime->colors)
			{
				source = plasma->color_source;
				if (source >= 1 && source <= 4)
					tint = runtime->colors + source - 1;
			}
			if (runtime->exponents)
			{
				source = plasma->intensity_exponent_source;
				if (source >= 1 && source <= 4)
					intensity = (real)pow(
						(double)runtime->exponents[source - 1],
						(double)plasma->intensity_exponent);
				source = plasma->offset_exponent_source;
				if (source >= 1 && source <= 4)
					offset = (real)pow(
						(double)runtime->exponents[source - 1],
						(double)plasma->offset_exponent) * plasma->offset_amount;
			}
		}

		rasterizer_set_texture(0, 1, 0, plasma->primary_noise_map, group->bitmap_sequence_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		rasterizer_set_texture(1, 1, 0, plasma->secondary_noise_map, group->bitmap_sequence_index);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);

		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_CULLMODE, D3DCULL_NONE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHABLENDENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_DESTBLEND, D3DBLEND_ONE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_BLENDOP, D3DBLENDOP_ADD);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ALPHATESTENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZENABLE, TRUE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZWRITEENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_ZBIAS, 0);

		rasterizer_set_vertex_shader_permutation(
			15,
			rasterizer_transparent_geometry_get_primary_vertex_type(group),
			FALSE);

		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c",
			105,
			plasma->primary_noise_map_animation_period!=0.0f);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_plasma_energy.c",
			106,
			plasma->secondary_noise_map_animation_period!=0.0f);

		if (offset < 0.0005f)
			/* Preserve January's immediate integer zero store without adding a
			 * third __real@00000000 relocation. */
			*(long *)&offset = 0;
		primary_time = global_frame_parameters.game_time_sec / plasma->primary_noise_map_animation_period;
		vertex_constants[0][1] = 0.0f;
		vertex_constants[0][2] = offset;
		vertex_constants[1][0] = 0.0f;
		vertex_constants[1][2] = 0.0f;
		vertex_constants[2][0] = 0.0f;
		vertex_constants[2][1] = 0.0f;
		vertex_constants[3][1] = 0.0f;
		vertex_constants[3][2] = 0.0f;
		vertex_constants[4][0] = 0.0f;
		vertex_constants[4][2] = 0.0f;
		vertex_constants[5][0] = 0.0f;
		vertex_constants[5][1] = 0.0f;
		color_constants[0][0] = 1.0f;
		color_constants[0][1] = 1.0f;
		color_constants[0][2] = 1.0f;
		color_constants[0][3] = 1.0f;

		secondary_time = global_frame_parameters.game_time_sec / plasma->secondary_noise_map_animation_period;
		primary_scale = plasma->primary_noise_map_scale;
		secondary_scale = plasma->secondary_noise_map_scale;
		vertex_constants[0][0] = primary_scale;
		vertex_constants[0][3] = primary_time * plasma->primary_noise_map_animation_direction.i;
		vertex_constants[1][1] = primary_scale;
		vertex_constants[1][3] = primary_time * plasma->primary_noise_map_animation_direction.j;
		vertex_constants[2][2] = primary_scale;
		vertex_constants[2][3] = primary_time * plasma->primary_noise_map_animation_direction.k;
		vertex_constants[3][0] = secondary_scale;
		vertex_constants[3][3] = secondary_time * plasma->secondary_noise_map_animation_direction.i;
		vertex_constants[4][1] = secondary_scale;
		vertex_constants[4][3] = secondary_time * plasma->secondary_noise_map_animation_direction.j;
		vertex_constants[5][2] = secondary_scale;
		vertex_constants[5][3] = secondary_time * plasma->secondary_noise_map_animation_direction.k;
		color_constants[1][0] = (plasma->perpendicular_color.red - plasma->parallel_color.red) * tint->red;
		color_constants[1][1] = (plasma->perpendicular_color.green - plasma->parallel_color.green) * tint->green;
		color_constants[1][2] = (plasma->perpendicular_color.blue - plasma->parallel_color.blue) * tint->blue;
		color_constants[1][3] = (plasma->perpendicular_alpha - plasma->parallel_alpha) * intensity;
		color_constants[2][0] = plasma->parallel_color.red * tint->red;
		color_constants[2][1] = plasma->parallel_color.green * tint->green;
		color_constants[2][2] = plasma->parallel_color.blue * tint->blue;
		color_constants[2][3] = intensity * plasma->parallel_alpha;

		IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -81, vertex_constants, 6);
		IDirect3DDevice8_SetVertexShaderConstant(global_d3d_device, -84, color_constants, 3);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.texture_modes = 0x42;
		pixel_shader.combiner_count = 0x104;
		pixel_shader.alpha_inputs[0] = 0x0820A920;
		pixel_shader.alpha_outputs[0] = 0xC00;
		pixel_shader.rgb_inputs[0] = 0x1920B820;
		pixel_shader.rgb_outputs[0] = 0xC00;
		pixel_shader.alpha_inputs[1] = 0x1C1C0C0C;
		pixel_shader.alpha_outputs[1] = 0x24C00;
		pixel_shader.rgb_inputs[1] = 0;
		pixel_shader.rgb_outputs[1] = 0;
		pixel_shader.alpha_inputs[2] = 0x5C5C;
		pixel_shader.alpha_outputs[2] = 0x4D00;
		pixel_shader.rgb_inputs[2] = 0;
		pixel_shader.rgb_outputs[2] = 0;
		pixel_shader.alpha_inputs[3] = 0x14150000;
		pixel_shader.alpha_outputs[3] = 0x40;
		pixel_shader.rgb_inputs[3] = 0x1C051DA0;
		pixel_shader.rgb_outputs[3] = 0xC00;
		pixel_shader.final_combiner_inputs_abcd = 0x0C0F0000;
		pixel_shader.final_combiner_inputs_efg = 0x1C1C1400;
		rasterizer_set_pixel_shader(&pixel_shader);
		rasterizer_transparent_geometry_group_draw__internal(group, FALSE);
	}
	return;
}

/* ---------- private code */

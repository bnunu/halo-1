/*
RASTERIZER_XBOX_TEXT.C

symbols in this file:
00162EA0 01b0:
	_code_00162ea0 (0000)
00163050 0050:
	_code_00163050 (0000)
001630A0 0220:
	_code_001630a0 (0000)
001632C0 0060:
	_code_001632c0 (0000)
00163320 0010:
	_code_00163320 (0000)
00163330 0010:
	_rasterizer_text_end (0000)
00163340 0020:
	_code_00163340 (0000)
00163360 0010:
	_code_00163360 (0000)
00163370 0010:
	_code_00163370 (0000)
00163380 0010:
	_code_00163380 (0000)
00163390 0690:
	_rasterizer_text_begin (0000)
00163A20 0120:
	_rasterizer_text_draw_character (0000)
00292B28 0036:
	??_C@_0DG@NLGEIAMP@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
00292B60 0030:
	??_C@_0DA@FEBHLDDN@?$CD?$CD?$CD?5ERROR?5rasterizer_text_draw_c@ (0000)
00292B90 0087:
	??_C@_0IH@NAGGOICD@IDirect3DDevice8_SetVertexData2f@ (0000)
00292C18 007d:
	??_C@_0HN@NMADEKEL@IDirect3DDevice8_SetVertexData2f@ (0000)
00292C98 0058:
	??_C@_0FI@KKLBHBJN@IDirect3DDevice8_SetVertexDataCo@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries/errors.h"
#include "real_math.h"
/* The January object retains out-of-line copies of the D3D inline wrappers.
 * Defining D3DINLINE as static before the normal XDK headers reproduces eight
 * of the nine wrappers exactly.  IDirect3DDevice8_SetRenderState is the lone
 * translation-unit-context exception: January retained its 0x220-byte body,
 * while this partial translation unit emits a 0x10-byte call to the exact
 * D3DDevice_SetRenderState helper.  Do not replace either with a handwritten
 * Microsoft dispatcher.
 *
 * code_00162ea0 = D3DDevice_SetRenderState
 * code_00163050 = D3DDevice_SetTextureStageState
 * code_001630a0 = IDirect3DDevice8_SetRenderState
 * code_001632c0 = IDirect3DDevice8_SetTextureStageState
 * code_00163320 = IDirect3DDevice8_SetVertexShaderConstant
 * code_00163340 = IDirect3DDevice8_SetVertexData2f
 * code_00163360 = IDirect3DDevice8_SetVertexDataColor
 * code_00163370 = IDirect3DDevice8_Begin
 * code_00163380 = IDirect3DDevice8_End
 */
#define D3DINLINE static
#include <xtl.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct rasterizer_text_vertex
{
	real x;
	real y;
	real u;
	real v;
	unsigned long color;
};

struct rasterizer_text_debug_options
{
	byte reserved00[0x22];
	boolean render_text;
};

struct rasterizer_text_window_parameters
{
	short rasterizer_target;
	byte reserved02[0x32];
	short left;
	short top;
	short right;
	short bottom;
};

struct bitmap_data;

struct rasterizer_text_begin_parameters
{
	void *meter_parameters;
	real_vector2d const *scale;
	boolean map_enabled[3];
	byte pad0B;
	struct bitmap_data const *map[3];
	boolean clamp[3];
	byte pad1B;
	real_vector2d const *texture_offset[3];
	real first_constants[6];
	real second_constants[6];
	real_rgb_color const *constant_color[3];
	real_argb_color color;
	unsigned long reserved74;
	real const *constant_alpha[3];
	unsigned long reserved84;
	short framebuffer_blend_function;
	boolean point_filtering;
	byte pad8B;
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

/* ---------- prototypes */

void rasterizer_error(
	long error_result,
	char const *format,
	...);

void rasterizer_set_framebuffer_blend_function(
	short function);

void rasterizer_set_texture_bitmap_data(
	short stage,
	struct bitmap_data const *bitmap);

void rasterizer_set_vertex_shader_permutation(
	short vertex_type,
	short permutation,
	boolean one_node);

void rasterizer_set_pixel_shader(
	struct pixel_shader_definition const *pixel_shader_definition);

unsigned long real_argb_color_to_pixel32(
	real_argb_color const *color);

/* ---------- globals */

extern void *global_d3d_device;
extern struct rasterizer_text_debug_options rasterizer_debug_options;
extern struct rasterizer_text_window_parameters global_window_parameters;
extern struct pixel_shader_definition pixel_shader;

/* ---------- public code */

void rasterizer_text_end(
	void)
{
	return;
}

void rasterizer_text_begin(
	struct rasterizer_text_begin_parameters const *parameters)
{
	real vertex_constants[5][4];
	real texture_constants[6][4];
	short window_width;
	short window_height;
	real_vector2d scale;
	short map_index;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
		13,
		global_d3d_device);
	if (rasterizer_debug_options.render_text &&
		global_window_parameters.rasterizer_target == 0)
	{
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
			18,
			parameters);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
			20,
			parameters->map[0]);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
			22,
			!parameters->map[2] || parameters->map[1]);
		match_assert(
			"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
			24,
			!parameters->map[1] || !parameters->meter_parameters);

		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_CULLMODE,
			D3DCULL_NONE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_RED |
			D3DCOLORWRITEENABLE_GREEN |
			D3DCOLORWRITEENABLE_BLUE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_ALPHABLENDENABLE,
			TRUE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_ZENABLE,
			FALSE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_ZBIAS,
			0);
		rasterizer_set_framebuffer_blend_function(
			parameters->framebuffer_blend_function);

		window_height = global_window_parameters.bottom -
			global_window_parameters.top;
		window_width = global_window_parameters.right -
			global_window_parameters.left;
		if (parameters->scale)
			scale.i = 2.0f * parameters->scale->i / window_height;
		else
			scale.i = 0.0f;
		if (parameters->scale)
			scale.j = -2.0f * parameters->scale->j / window_width;
		else
			scale.j = 0.0f;

		vertex_constants[0][0] = 2.0f / window_height;
		vertex_constants[0][1] = 0.0f;
		vertex_constants[0][2] = 0.0f;
		vertex_constants[0][3] = scale.i - (1.0f + 1.0f / window_height);
		vertex_constants[1][0] = 0.0f;
		vertex_constants[1][1] = -2.0f / window_width;
		vertex_constants[1][2] = 0.0f;
		vertex_constants[1][3] = scale.j + 1.0f / window_width + 1.0f;
		vertex_constants[2][0] = 0.0f;
		vertex_constants[2][1] = 0.0f;
		vertex_constants[2][2] = 0.0f;
		vertex_constants[2][3] = 0.5f;
		vertex_constants[3][0] = 0.0f;
		vertex_constants[3][1] = 0.0f;
		vertex_constants[3][2] = 0.0f;
		vertex_constants[3][3] = 1.0f;
		vertex_constants[4][0] = parameters->second_constants[0];
		vertex_constants[4][1] = parameters->second_constants[1];
		vertex_constants[4][2] = 0.0f;
		vertex_constants[4][3] = 1.0f;

		texture_constants[0][0] = parameters->second_constants[2];
		texture_constants[0][1] = parameters->second_constants[3];
		texture_constants[0][2] = parameters->second_constants[4];
		texture_constants[0][3] = parameters->second_constants[5];
		texture_constants[1][0] = parameters->map_enabled[0] ? 1.0f : 0.0f;
		texture_constants[1][1] = parameters->map_enabled[0] ? 0.0f : 1.0f;
		texture_constants[1][2] = parameters->map_enabled[1] ? 1.0f : 0.0f;
		texture_constants[1][3] = parameters->map_enabled[1] ? 0.0f : 1.0f;
		texture_constants[2][0] = parameters->map_enabled[2] ? 1.0f : 0.0f;
		texture_constants[2][1] = parameters->map_enabled[2] ? 0.0f : 1.0f;
		texture_constants[2][2] = parameters->texture_offset[0] ? parameters->texture_offset[0]->i : 0.0f;
		texture_constants[2][3] = parameters->texture_offset[0] ? parameters->texture_offset[0]->j : 0.0f;
		texture_constants[3][0] = parameters->texture_offset[1] ? parameters->texture_offset[1]->i : 0.0f;
		texture_constants[3][1] = parameters->texture_offset[1] ? parameters->texture_offset[1]->j : 0.0f;
		texture_constants[3][2] = parameters->texture_offset[2] ? parameters->texture_offset[2]->i : 0.0f;
		texture_constants[3][3] = parameters->texture_offset[2] ? parameters->texture_offset[2]->j : 0.0f;
		texture_constants[4][0] = parameters->first_constants[0];
		texture_constants[4][1] = parameters->first_constants[1];
		texture_constants[4][2] = parameters->first_constants[2];
		texture_constants[4][3] = parameters->first_constants[3];
		texture_constants[5][0] = parameters->first_constants[4];
		texture_constants[5][1] = parameters->first_constants[5];
		texture_constants[5][2] = 0.0f;
		texture_constants[5][3] = 0.0f;

		IDirect3DDevice8_SetVertexShaderConstant(
			global_d3d_device,
			-68,
			vertex_constants,
			5);
		IDirect3DDevice8_SetVertexShaderConstant(
			global_d3d_device,
			-63,
			texture_constants,
			6);

		map_index = 0;
map_loop:
		{
			long map_array_index;
			short map_stage;
			struct bitmap_data const *bitmap;

			map_stage = map_index;
			map_array_index = map_stage;
			bitmap = parameters->map[map_array_index];
			/* Texture maps are contiguous; the original exits at the first gap.
			 * Keep the increment on the populated path to preserve that control
			 * flow and VC7's direct signed-index loop shape. */
			if (bitmap)
			{
				rasterizer_set_texture_bitmap_data(map_index, bitmap);
				IDirect3DDevice8_SetTextureStageState(
					global_d3d_device,
					map_array_index,
					D3DTSS_ADDRESSU,
					parameters->clamp[map_array_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(
					global_d3d_device,
					map_array_index,
					D3DTSS_ADDRESSV,
					parameters->clamp[map_array_index] ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(
					global_d3d_device,
					map_array_index,
					D3DTSS_MAGFILTER,
					parameters->point_filtering ? D3DTEXF_POINT : D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(
					global_d3d_device,
					map_array_index,
					D3DTSS_MINFILTER,
					parameters->point_filtering ? D3DTEXF_POINT : D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(
					global_d3d_device,
					map_array_index,
					D3DTSS_MIPFILTER,
					parameters->point_filtering ? D3DTEXF_POINT : D3DTEXF_LINEAR);
				map_index++;
			}
			else
			{
				goto maps_done;
			}
			if (map_index < 3)
				goto map_loop;
		}

maps_done:
		rasterizer_set_vertex_shader_permutation(4, 8, TRUE);
		if (parameters->map[0])
		{
			real_argb_color *constant_colors;
			real_rgb_color const *constant_color;

			constant_colors = (real_argb_color *)&vertex_constants[2];
			csmemset(&pixel_shader, 0, sizeof(pixel_shader));
			pixel_shader.texture_modes =
				((((parameters->map[2] != NULL) << 5) |
				(parameters->map[1] != NULL)) << 5) |
				(parameters->map[0] != NULL);
			constant_color = parameters->constant_color[0] ?
				parameters->constant_color[0] : global_real_rgb_white;
			constant_colors[0].rgb = *constant_color;
			constant_color = parameters->constant_color[1] ?
				parameters->constant_color[1] : global_real_rgb_white;
			constant_colors[1].rgb = *constant_color;
			constant_color = parameters->constant_color[2] ?
				parameters->constant_color[2] : global_real_rgb_white;
			constant_colors[2].rgb = *constant_color;
			if (parameters->constant_alpha[0])
				constant_colors[0].alpha = *parameters->constant_alpha[0];
			else
				constant_colors[0].alpha = 1.0f;
			if (parameters->constant_alpha[1])
				constant_colors[1].alpha = *parameters->constant_alpha[1];
			else
				constant_colors[1].alpha = 1.0f;
			if (parameters->constant_alpha[2])
				constant_colors[2].alpha = *parameters->constant_alpha[2];
			else
				constant_colors[2].alpha = 1.0f;
			pixel_shader.constant_0[0] = real_argb_color_to_pixel32(&constant_colors[0]);
			pixel_shader.constant_1[0] = real_argb_color_to_pixel32(&constant_colors[1]);
			pixel_shader.constant_0[1] = real_argb_color_to_pixel32(&constant_colors[2]);
			pixel_shader.constant_0[4] = real_argb_color_to_pixel32(&parameters->color);
			pixel_shader.constant_0[5] = real_argb_color_to_pixel32(&parameters->color);
			pixel_shader.constant_0[6] = real_argb_color_to_pixel32(&parameters->color);
			pixel_shader.constant_0[7] = real_argb_color_to_pixel32(&parameters->color);
			pixel_shader.rgb_outputs[0] = 0x89;
			pixel_shader.alpha_outputs[0] = 0x89;
			pixel_shader.rgb_inputs[0] = 0x08010902;
			pixel_shader.alpha_inputs[0] = 0x18111912;
			pixel_shader.rgb_inputs[1] = 0x0A010804;
			pixel_shader.rgb_outputs[1] = 0xAC;
			pixel_shader.alpha_inputs[1] = 0x1A111814;
			pixel_shader.alpha_outputs[1] = 0xAC;
			pixel_shader.combiner_count = 0x11102;
			pixel_shader.final_combiner_inputs_abcd = 12;
			pixel_shader.final_combiner_inputs_efg = 0x1C00;
		}

		rasterizer_set_pixel_shader(&pixel_shader);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_CULLMODE,
			0x901);
		rasterizer_set_vertex_shader_permutation(4, 8, FALSE);
		IDirect3DDevice8_SetTextureStageState(
			global_d3d_device,
			0,
			D3DTSS_ALPHAKILL,
			0);
	}

	return;
}

void rasterizer_text_draw_character(
	struct rasterizer_text_vertex const *vertices)
{
	boolean success;
	short vertex_index;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_text.c",
		216,
		global_d3d_device);

	if (rasterizer_debug_options.render_text &&
		global_window_parameters.rasterizer_target == 0)
	{
		success = IDirect3DDevice8_Begin(
			global_d3d_device,
			D3DPT_TRIANGLEFAN) >= 0;
		for (vertex_index = 0; vertex_index < 4; vertex_index++)
		{
			if (IDirect3DDevice8_SetVertexDataColor(
				global_d3d_device,
				9,
				vertices[vertex_index].color) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexDataColor(global_d3d_device, 9, vertices[vertex_index].color)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				4,
				vertices[vertex_index].u,
				vertices[vertex_index].v) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, vertices[vertex_index].texcoord.u, vertices[vertex_index].texcoord.v)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				0,
				vertices[vertex_index].x,
				vertices[vertex_index].y) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertices[vertex_index].position.x, vertices[vertex_index].position.y)");
			}
		}

		if (IDirect3DDevice8_End(global_d3d_device) >= 0 && success)
		{
			success = TRUE;
		}
		else
		{
			success = FALSE;
			rasterizer_error(
				0,
				"IDirect3DDevice8_End(global_d3d_device)");
		}

		if (!success)
			error(2, "### ERROR rasterizer_text_draw_character failed");
	}

	return;
}

/* ---------- private code */

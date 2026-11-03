/*
RASTERIZER_XBOX_SCREEN_EFFECT.C

symbols in this file:
0015F720 01b0:
	_code_0015f720 (0000)
0015F8D0 0050:
	_code_0015f8d0 (0000)
0015F920 0090:
	_code_0015f920 (0000)
0015F9B0 0220:
	_code_0015f9b0 (0000)
0015FBD0 0060:
	_code_0015fbd0 (0000)
0015FC30 0010:
	_code_0015fc30 (0000)
0015FC40 0020:
	_code_0015fc40 (0000)
0015FC60 0010:
	_code_0015fc60 (0000)
0015FC70 0010:
	_code_0015fc70 (0000)
0015FC80 0010:
	_code_0015fc80 (0000)
0015FC90 0850:
	_rasterizer_screen_effect_set_texture_transforms (0000)
001604E0 0f30:
	__rasterizer_screen_effect (0000)
00161410 0540:
	__rasterizer_screen_flash (0000)
00292390 0019:
	??_C@_0BJ@BEKEHHEJ@v?9?$DOi?$CB?$DN0?40f?5?$CG?$CG?5v?9?$DOj?$CB?$DN0?40f?$AA@ (0000)
002923AC 0002:
	??_C@_01MHEDDDHA@v?$AA@ (0000)
002923B0 003f:
	??_C@_0DP@GDHJCEBP@c?3?2halo?2SOURCE?2rasterizer?2xbox?2r@ (0000)
002923F0 001b:
	??_C@_0BL@MEELOPHA@main_get_window_count?$CI?$CJ?$DM?$DN1?$AA@ (0000)
0029240C 0016:
	??_C@_0BG@OBFLACOO@?$CBparameters?9?$DOvideo_on?$AA@ (0000)
00292424 002a:
	??_C@_0CK@GCKMEOHN@?$CD?$CD?$CD?5ERROR?5rasterizer_screen_effe@ (0000)
00292450 0065:
	??_C@_0GF@LMEHBMJP@IDirect3DDevice8_SetVertexData2f@ (0000)
002924B8 0065:
	??_C@_0GF@CDJNJPAB@IDirect3DDevice8_SetVertexData2f@ (0000)
00292520 004a:
	??_C@_0EK@DDGAKMME@IDirect3DDevice8_SetVertexData2s@ (0000)
00292570 0065:
	??_C@_0GF@CCFPPFDG@IDirect3DDevice8_SetVertexData2f@ (0000)
002925D8 0058:
	??_C@_0FI@MCHFKFGN@IDirect3DDevice8_SetVertexData2s@ (0000)
00292630 0065:
	??_C@_0GF@LNIFHGKI@IDirect3DDevice8_SetVertexData2f@ (0000)
00292698 004b:
	??_C@_0EL@FFKPDIEL@IDirect3DDevice8_SetVertexData2s@ (0000)
002926E4 0004:
	__real@3b088889 (0000)
002926E8 0004:
	__real@3acccccd (0000)
002926EC 0033:
	??_C@_0DD@IJEPJHCB@combiner_count?$DM?$DNRASTERIZER_MAXIM@ (0000)
00292720 003a:
	??_C@_0DK@EJBEKMFA@?$CD?$CD?$CD?5ERROR?5video?5effect?5tried?5to?5@ (0000)
00292760 0084:
	??_C@_0IE@LDDMKLNC@parameters?9?$DOvideo_overbright_mod@ (0000)
002927E8 0044:
	??_C@_0EE@MEDEOJLI@?$CD?$CD?$CD?5ERROR?5non?9convolution?5effect@ (0000)
0029282C 0032:
	??_C@_0DC@CMADNDPH@video?5effect?5noise?5map?5cannot?5be@ (0000)
00292860 0008:
	??_C@_07MACHECCK@pass?$DN?$DN1?$AA@ (0000)
00292868 0008:
	??_C@_07NJDMHDGL@pass?$DN?$DN0?$AA@ (0000)
00292870 002c:
	??_C@_0CM@OFCEIJKA@blur?5effect?5cannot?5specify?5convo@ (0000)
0029289C 002d:
	??_C@_0CN@EJCBMOOJ@video?5effect?5cannot?5specify?5conv@ (0000)
002928CC 0037:
	??_C@_0DH@DCBDBJNG@video?5effect?5cannot?5specify?5nonz@ (0000)
00292904 002d:
	??_C@_0CN@CNLJNHLN@video?5effect?5cannot?5specify?5conv@ (0000)
00292934 0035:
	??_C@_0DF@FNIELJAM@video?5effect?5cannot?5specify?5extr@ (0000)
0029296C 0024:
	??_C@_0CE@MIPIKB@video?5effect?5must?5specify?5noise?5@ (0000)
00292990 0027:
	??_C@_0CH@KIKPHJLK@video?5effect?5must?5specify?5scanli@ (0000)
002929B8 0028:
	??_C@_0CI@PFHEHAHJ@?$CD?$CD?$CD?5ERROR?5unsupported?5screen?5fla@ (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps/bitmap_group.h"
#include "cseries/errors.h"
#include "interface/hud_draw.h"
#include "bitmaps/bitmaps_inlines.h"
#include "main/main.h"
#include "math/integer_math.h"
#include "math/real_math.h"
#include "render/render_cameras.h"
#include "rasterizer/rasterizer.h"
#include "rasterizer/rasterizer_cinematics.h"
#include "rasterizer/rasterizer_console_vars.h"
#include <stddef.h>
#include <xtl.h>
#include "rasterizer_xbox.h"
#include "rasterizer_xbox_pixel_shader.h"

/* ---------- constants */

enum
{
	_render_screen_flash_type_none = 0,
	_render_screen_flash_type_lighten,
	_render_screen_flash_type_darken,
	_render_screen_flash_type_max,
	_render_screen_flash_type_min,
	_render_screen_flash_type_invert,
	_render_screen_flash_type_tint,
	NUMBER_OF_SCREEN_FLASH_TYPES
};

enum
{
	_rasterizer_screen_effect_convolution_type_none = 0,
	_rasterizer_screen_effect_convolution_type_blur,
	_rasterizer_screen_effect_convolution_type_warp,
	NUMBER_OF_RASTERIZER_SCREEN_EFFECT_CONVOLUTION_TYPES
};

enum
{
	NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES = 3
};

enum
{
	RASTERIZER_MAXIMUM_COMBINER_STAGES = 8
};

enum
{
	_rasterizer_target_render_primary = 0,
	_rasterizer_target_screen_effect = 7,
	NUMBER_OF_RASTERIZER_TARGETS = 8
};

enum
{
	_bitmap_type_2d = 0,
	_bitmap_linear_bit = 4
};

enum
{
	_rasterizer_vertex_shader_screen = 4,
	_rasterizer_vertex_shader_screen_effect = 38,
	_rasterizer_vertex_type_screen = 8,
	_rasterizer_vertex_shader_permutation_default = 0,
	_rasterizer_vertex_register_position = 0
};

/* ---------- macros */

/* ---------- structures */

typedef char rasterizer_screen_effect_debug_options_flashes_offset_assert[
	offsetof(struct rasterizer_debug_options, screen_flash_enabled) == 0x47 ? 1 : -1];
typedef char rasterizer_screen_effect_debug_options_effects_offset_assert[
	offsetof(struct rasterizer_debug_options, screen_effects_enabled) == 0x48 ? 1 : -1];
typedef char rasterizer_screen_effect_parameters_mask_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, convolution_mask) == 0x08 ? 1 : -1];
typedef char rasterizer_screen_effect_parameters_tint_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, filter_desaturation_tint) == 0x14 ? 1 : -1];
typedef char rasterizer_screen_effect_parameters_video_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_on) == 0x23 ? 1 : -1];
typedef char rasterizer_screen_effect_parameters_scanline_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_scanline_map) == 0x28 ? 1 : -1];
typedef char rasterizer_screen_effect_parameters_noise_offset_assert[
	offsetof(struct rasterizer_cinematic_screen_effect_parameters, video_noise_map) == 0x34 ? 1 : -1];
typedef char rasterizer_screen_effect_window_viewport_offset_assert[
	offsetof(struct rasterizer_window_begin_parameters, camera.viewport_bounds) == 0x34 ? 1 : -1];
typedef char rasterizer_screen_effect_window_bounds_offset_assert[
	offsetof(struct rasterizer_window_begin_parameters, camera.window_bounds) == 0x3C ? 1 : -1];
typedef char rasterizer_screen_effect_window_flash_offset_assert[
	offsetof(struct rasterizer_window_begin_parameters, screen_flash) == 0x238 ? 1 : -1];
typedef char rasterizer_screen_effect_pixel_shader_size_assert[
	sizeof(struct pixel_shader_definition) == 0xF0 ? 1 : -1];

/* ---------- globals */


/* ---------- private code */

static real_vector2d vector2d_reciprocal(
	real_vector2d const *v)
{
	real_vector2d reciprocal;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		30,
		v);
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		31,
		v->i!=0.0f && v->j!=0.0f);

	reciprocal.i = 1.0f / v->i;
	reciprocal.j = 1.0f / v->j;

	return reciprocal;
}

static void rasterizer_screen_effect_set_texture_transforms(
	struct rasterizer_cinematic_screen_effect_parameters const *parameters,
	short pass,
	short pass_count)
{
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		47,
		parameters);
	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		48,
		global_d3d_device);

	{
		struct bitmap_data viewport_map =
		{
			BITMAP_GROUP_TAG,
			global_window_parameters.camera.viewport_bounds.x1 - global_window_parameters.camera.viewport_bounds.x0,
			global_window_parameters.camera.viewport_bounds.y1 - global_window_parameters.camera.viewport_bounds.y0,
			1,
			_bitmap_type_2d,
			NONE,
			FLAG(_bitmap_linear_bit)
		};
		real constants[8][4];
		real_vector2d convolution_size;
		real_vector2d screen_size;
		real_vector2d scanline_size;
		real_vector2d noise_size;
		real_vector2d screen_scale;
		real_vector2d default_scale;
		real_vector2d noise_scale;
		real_vector2d scanline_scale;
		real_vector2d convolution_scale;
		struct bitmap_data const *scanline_bitmap;
		struct bitmap_data const *noise_bitmap;
		struct bitmap_data const *convolution_bitmap;
		short x_offset;
		short y_offset;

		if (parameters->convolution_mask &&
			(pass > 0 ||
			pass_count == 1 ||
			parameters->convolution_type != _rasterizer_screen_effect_convolution_type_none))
			convolution_bitmap = parameters->convolution_mask;
		else
			convolution_bitmap = &viewport_map;

		if (parameters->video_on)
			scanline_bitmap = parameters->video_scanline_map;
		else
			scanline_bitmap = &viewport_map;

		if (parameters->video_on)
			noise_bitmap = parameters->video_noise_map;
		else
			noise_bitmap = &viewport_map;

		convolution_size.i = convolution_bitmap->width;
		convolution_size.j = convolution_bitmap->height;
		scanline_size.i = scanline_bitmap->width;
		scanline_size.j = scanline_bitmap->height;
		noise_size.i = noise_bitmap->width;
		noise_size.j = noise_bitmap->height;
		screen_size.i = viewport_map.width;
		screen_size.j = viewport_map.height;

		default_scale.i = 1.0f;
		default_scale.j = 1.0f;

		convolution_scale = TEST_FLAG(convolution_bitmap->flags, _bitmap_linear_bit) ?
			default_scale : vector2d_reciprocal(&convolution_size);

		scanline_scale = TEST_FLAG(scanline_bitmap->flags, _bitmap_linear_bit) ?
			default_scale : vector2d_reciprocal(&scanline_size);

		noise_scale = TEST_FLAG(noise_bitmap->flags, _bitmap_linear_bit) ?
			default_scale : vector2d_reciprocal(&noise_size);

		screen_scale = TEST_FLAG(viewport_map.flags, _bitmap_linear_bit) ?
			default_scale : vector2d_reciprocal(&screen_size);

		constants[0][0] = convolution_scale.i;
		constants[0][1] = 0.0f;
		constants[0][2] = 0.0f;
		constants[0][3] = (convolution_size.i + 1.0f - viewport_map.width) * convolution_scale.i * 0.5f;
		constants[1][0] = 0.0f;
		constants[1][1] = convolution_scale.j;
		constants[1][2] = 0.0f;
		constants[1][3] = (convolution_size.j + 1.0f - viewport_map.height) * convolution_scale.j * 0.5f;
		constants[2][0] = scanline_scale.i;
		constants[2][1] = 0.0f;
		constants[2][2] = 0.0f;
		constants[2][3] = (scanline_size.i + 1.0f - viewport_map.width) * scanline_scale.i * 0.5f;
		constants[3][0] = 0.0f;
		constants[3][1] = scanline_scale.j;
		constants[3][2] = 0.0f;
		constants[3][3] = (scanline_size.j + 1.0f - viewport_map.height) * scanline_scale.j * 0.5f;
		constants[4][0] = noise_scale.i;
		constants[4][1] = 0.0f;
		constants[4][2] = 0.0f;
		constants[4][3] = (noise_size.i + 1.0f - viewport_map.width) * noise_scale.i * 0.5f;
		constants[5][0] = 0.0f;
		constants[5][1] = noise_scale.j;
		constants[5][2] = 0.0f;
		constants[5][3] = (noise_size.j + 1.0f - viewport_map.height) * noise_scale.j * 0.5f;
		constants[6][0] = screen_scale.i;
		constants[6][1] = 0.0f;
		constants[6][2] = 0.0f;
		constants[6][3] = (screen_size.i + 1.0f - viewport_map.width) * screen_scale.i * 0.5f;
		constants[7][0] = 0.0f;
		constants[7][1] = screen_scale.j;
		constants[7][2] = 0.0f;
		constants[7][3] = (screen_size.j + 1.0f - viewport_map.height) * screen_scale.j * 0.5f;

		x_offset = (global_window_parameters.camera.viewport_bounds.x0 +
			global_window_parameters.camera.viewport_bounds.x1) / 2 -
			(global_window_parameters.camera.window_bounds.x1 +
			global_window_parameters.camera.window_bounds.x0) / 2;
		y_offset = (global_window_parameters.camera.viewport_bounds.y0 +
			global_window_parameters.camera.viewport_bounds.y1) / 2 -
			(global_window_parameters.camera.window_bounds.y1 +
			global_window_parameters.camera.window_bounds.y0) / 2;

		if (convolution_bitmap == &viewport_map)
		{
			constants[0][3] += global_window_parameters.camera.viewport_bounds.x0;
			constants[1][3] += global_window_parameters.camera.viewport_bounds.y0;
		}
		else if (convolution_bitmap == parameters->convolution_mask)
		{
			constants[0][3] += x_offset * convolution_scale.i;
			constants[1][3] += y_offset * convolution_scale.j;
		}

		if (scanline_bitmap == &viewport_map)
		{
			constants[2][3] += global_window_parameters.camera.viewport_bounds.x0;
			constants[3][3] += global_window_parameters.camera.viewport_bounds.y0;
		}
		else if (scanline_bitmap == parameters->convolution_mask)
		{
			constants[2][3] += x_offset * scanline_scale.i;
			constants[3][3] += y_offset * scanline_scale.j;
		}

		if (parameters->convolution_type == _rasterizer_screen_effect_convolution_type_blur)
		{
			real convolution_radius = parameters->convolution_radius;

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				176,
				!parameters->video_on);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				177,
				main_get_window_count()<=1);

			constants[0][3] += parameters->convolution_mask ?
				0.0f : convolution_scale.i * convolution_radius;
			constants[1][3] += parameters->convolution_mask ?
				0.0f : convolution_scale.j * convolution_radius;
			constants[2][3] -= scanline_scale.i * convolution_radius;
			constants[3][3] -= scanline_scale.j * convolution_radius;
			constants[4][3] += noise_scale.i * convolution_radius;
			constants[5][3] -= noise_scale.j * convolution_radius;
			constants[6][3] -= screen_scale.i * convolution_radius;
			constants[7][3] += screen_scale.j * convolution_radius;
		}
		else if (parameters->convolution_type == _rasterizer_screen_effect_convolution_type_warp)
		{
			real convolution_radius = parameters->convolution_radius;
			real convolution_offset = parameters->convolution_mask ?
				0.0f : -convolution_radius;
			real scanline_offset = 0.0f;
			real noise_offset = convolution_radius;
			real screen_offset = parameters->convolution_mask ?
				-convolution_radius : convolution_radius * 2.0f;

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				198,
				!parameters->video_on);
			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				199,
				main_get_window_count()<=1);

			constants[0][0] = (1.0f - convolution_offset / convolution_size.i) * constants[0][0];
			constants[1][1] = (1.0f - convolution_offset / convolution_size.j) * constants[1][1];
			constants[2][0] = (1.0f - scanline_offset / scanline_size.i) * constants[2][0];
			constants[3][1] = (1.0f - scanline_offset / scanline_size.j) * constants[3][1];
			constants[4][0] = (1.0f - noise_offset / noise_size.i) * constants[4][0];
			constants[5][1] = (1.0f - noise_offset / noise_size.j) * constants[5][1];
			constants[6][0] = (1.0f - screen_offset / screen_size.i) * constants[6][0];
			constants[7][1] = (1.0f - screen_offset / screen_size.j) * constants[7][1];
			constants[0][3] += convolution_offset * 0.5f;
			constants[1][3] += convolution_offset * 0.5f;
			constants[2][3] += scanline_offset * 0.5f;
			constants[3][3] += scanline_offset * 0.5f;
			constants[4][3] += noise_offset * 0.5f;
			constants[5][3] += noise_offset * 0.5f;
			constants[6][3] += screen_offset * 0.5f;
			constants[7][3] += screen_offset * 0.5f;
		}
		else if (pass == 1 && parameters->video_on)
		{
			real random_value;

			match_assert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				223,
				main_get_window_count()<=1);

			/* The native builds draw several frames per tick
			(port/linux/game/render_interpolation.c): move the noise 30 times
			a second, as the Xbox did once a frame, not every frame. */
			{
				unsigned long noise_seed =
					(unsigned long)system_milliseconds() / (MILLISECONDS_PER_SECOND / TICKS_PER_SECOND) * 0x9E3779B1UL;

				noise_seed ^= noise_seed >> 16;
				noise_seed *= 0x7FEB352DUL;
				noise_seed ^= noise_seed >> 15;
				random_value = real_seed_random(&noise_seed);
				constants[4][3] += noise_scale.i * random_value * noise_size.i;
				random_value = real_seed_random(&noise_seed);
				constants[5][3] += noise_scale.j * random_value * noise_size.j;
			}
		}

		IDirect3DDevice8_SetVertexShaderConstant(
			global_d3d_device,
			-81,
			constants,
			8);
	}

	return;
}
/* port: the passes a convolution draws after the Xbox's own on a screen of
screen_scale pixels to the Xbox's one, 0 at the Xbox's size. Each of the
Xbox's passes blends three copies of the screen a radius apart, and at
several pixels to the Xbox's one the copies stand apart as sharp ghosts.
Two more passes, at a third and a ninth of the radius, put eight copies
between each two, a ninth of the radius apart: on a screen of up to nine
pixels to the Xbox's one (2160 lines are 4.5) they stand no farther apart
than the Xbox's did in its own pixels, where they blended, and two more
again for up to 81. The blur stays as wide as the Xbox's (its variance 6%
more). Splitting the radius into as many times the passes as the screen's
scale, to six, drew ten full-screen passes at 2160 lines, where a scope's
zoom halved the frame rate, and narrowed the blur to less than half the
Xbox's. The count is even: the last pass draws into the screen, as the
Xbox's does. */
static short rasterizer_screen_effect_refinement_passes(
	real screen_scale)
{
	short passes = 0;
	real spacing = 1.0f;

	while (screen_scale * spacing > 1.0f && passes < 4)
	{
		passes += 2;
		spacing /= 9.0f;
	}
	return passes;
}

/* port: the radius of a pass of a convolution drawn with refinement
passes: the Xbox's passes keep the effect's, each pass after them a third
of the one before it */
static real rasterizer_screen_effect_pass_radius(
	real radius,
	short pass,
	short stock_pass_count)
{
	short refinement;

	for (refinement = stock_pass_count; refinement <= pass; refinement++)
		radius /= 3.0f;
	return radius;
}


/* port: the video mask was authored for the Xbox's 640-wide view. A wider
viewport exposes its black side margins, whose original noise-dependent
alpha still admits bright parts of the sampled scene. Keep the video and
its noise unchanged inside that view, and cover only the added margins.
The existing screen-effect shader and RGB-only write mask stay in force;
zero blend factors blacken the margins without changing scratch alpha. */
static boolean rasterizer_screen_effect_video_borders(
	boolean video_on,
	short viewport_width,
	short viewport_height)
{
	boolean success = TRUE;
	short side;
	real inner;
	real half_pixel_x;
	real half_pixel_y;

	if (!video_on || viewport_width <= 640 || viewport_height <= 0)
		return TRUE;

	inner = 640.0f / (real)viewport_width;
	/* The shader translator adds half a logical pixel when converting the
	Xbox's integer pixel centers. Counter it here so every margin pixel is
	covered and neither inner edge moves into the authored picture. */
	half_pixel_x = 1.0f / (real)viewport_width;
	half_pixel_y = 1.0f / (real)viewport_height;
	success = IDirect3DDevice8_SetRenderState(global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_ZERO) >= 0;
	for (side = 0; side < 2; side++)
	{
		real left = (side ? inner : -1.0f) - half_pixel_x;
		real right = (side ? 1.0f : -inner) - half_pixel_x;

		success = IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN) >= 0 && success;
		success = IDirect3DDevice8_SetVertexData2s(global_d3d_device, 4, 0, 0) >= 0 && success;
		success = IDirect3DDevice8_SetVertexData2f(
			global_d3d_device, _rasterizer_vertex_register_position, left, -1.0f + half_pixel_y) >= 0 && success;
		success = IDirect3DDevice8_SetVertexData2f(
			global_d3d_device, _rasterizer_vertex_register_position, right, -1.0f + half_pixel_y) >= 0 && success;
		success = IDirect3DDevice8_SetVertexData2f(
			global_d3d_device, _rasterizer_vertex_register_position, right, 1.0f + half_pixel_y) >= 0 && success;
		success = IDirect3DDevice8_SetVertexData2f(
			global_d3d_device, _rasterizer_vertex_register_position, left, 1.0f + half_pixel_y) >= 0 && success;
		success = IDirect3DDevice8_End(global_d3d_device) >= 0 && success;
	}
	success = IDirect3DDevice8_SetRenderState(
		global_d3d_device, D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA) >= 0 && success;
	return success;
}

/* port: the shader translator turns Direct3D's pixel centers (on integers)
into OpenGL's (on halves) by moving every vertex half a pixel right and down,
a pixel of the game's 480 lines (nv2a_vsh.c). The screen is drawn at several
of the display's pixels to one of those (halo_screen_scale), so a pass's quad
started half a game pixel inside its target's left and top edges, and the
display's pixels there were never drawn: each pass of a scope's zoom warp read
their stale contents and drew them farther in, a dark band down the left of
the blur and along its top (the right and bottom edges are clamped to drawn
pixels). Move the quad back by that half pixel, a pixel of the viewport it
draws into (the whole surface for the second render-primary surface, the
window for the primary: rasterizer_set_target), and return how far its
texture coordinates, in the viewport's pixels, move with it, so that every
pixel samples where it did and the quad covers the whole viewport. */
static real rasterizer_screen_effect_cover_target(
	short destination_target,
	short viewport_width,
	short viewport_height,
	real_rectangle2d *vertex_bounds)
{
	real half_pixel_x;
	real half_pixel_y;

	if (destination_target == _rasterizer_target_screen_effect)
	{
		half_pixel_x = 1.0f / (real)halo_screen_width();
		half_pixel_y = 1.0f / 480.0f;
	}
	else
	{
		half_pixel_x = 1.0f / (real)viewport_width;
		half_pixel_y = 1.0f / (real)viewport_height;
	}
	vertex_bounds->x0 -= half_pixel_x;
	vertex_bounds->x1 -= half_pixel_x;
	vertex_bounds->y0 += half_pixel_y;
	vertex_bounds->y1 += half_pixel_y;

	return 0.5f;
}
/* ---------- public code */

void _rasterizer_screen_effect(
	struct rasterizer_cinematic_screen_effect_parameters *parameters)
{
	boolean success = TRUE;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		251,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_screen_effect);

	parameters = rasterizer_screen_effect_get_cinematic_parameters(parameters);

	if (parameters &&
		(parameters->convolution_type != _rasterizer_screen_effect_convolution_type_none ||
		parameters->convolution_mask ||
		parameters->filter_light_enhancement_intensity > 0.0f ||
		parameters->filter_desaturation_intensity > 0.0f ||
		parameters->video_on) &&
		rasterizer_debug_options.screen_effects_enabled &&
		global_window_parameters.rasterizer_target == _rasterizer_target_render_primary)
	{
		short pass_count = (parameters->convolution_extra_passes + 1) * 2;
		short pass;
		short source_target;
		short destination_target;
		short stage;
		short viewport_width;
		short viewport_height;
		real_rectangle2d vertex_bounds;
		/* port: rasterizer_screen_effect_cover_target */
		real texel_shift;
		unsigned long light_enhancement_input;
		unsigned long desaturation_input;
		struct rasterizer_cinematic_screen_effect_parameters scaled_parameters;

		/* port: the native builds draw the screen at several pixels to the
		Xbox's one (halo_screen_scale): a convolution's few copies of the
		screen, apart by its radius in the Xbox's pixels, blended into a blur
		at 640x480, and at four times the pixels they stand apart as sharp
		ghosts (the zoom's warp). The Xbox's passes are drawn as they were,
		and a few more, each a third of the last one's radius, fill the gaps
		between the copies (rasterizer_screen_effect_refinement_passes). */
		short stock_pass_count = pass_count;
		struct rasterizer_cinematic_screen_effect_parameters const *original_parameters = parameters;

		if (parameters->convolution_type != _rasterizer_screen_effect_convolution_type_none && !parameters->video_on)
		{
			short refinement_passes = rasterizer_screen_effect_refinement_passes(halo_screen_scale());

			if (refinement_passes > 0)
			{
				scaled_parameters = *parameters;
				parameters = &scaled_parameters;
				pass_count += refinement_passes;
			}
		}

		if (parameters->video_on)
		{
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				276,
				parameters->video_scanline_map,
				"video effect must specify scanline map");
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				277,
				parameters->video_noise_map,
				"video effect must specify noise map");
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				278,
				!parameters->convolution_extra_passes,
				"video effect cannot specify extra convolution passes");
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				279,
				!parameters->convolution_type,
				"video effect cannot specify convolution type");
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				280,
				parameters->convolution_radius == 0.0f,
				"video effect cannot specify nonzero convolution radius");
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				281,
				!parameters->convolution_mask,
				"video effect cannot specify convolution mask");
		}

		if (parameters->convolution_type == _rasterizer_screen_effect_convolution_type_blur)
			match_vassert(
				"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
				286,
				!parameters->convolution_mask,
				"blur effect cannot specify convolution mask");

		rasterizer_set_vertex_shader_permutation(
			_rasterizer_vertex_shader_screen_effect,
			_rasterizer_vertex_type_screen,
			_rasterizer_vertex_shader_permutation_default);

		for (pass = 0; pass < pass_count; pass++)
		{
			if (pass_count == 1)
			{
				match_assert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
					313,
					pass==0);

				source_target = NONE;
				destination_target = NONE;
			}
			else if (!(pass & 1))
			{
				source_target = _rasterizer_target_render_primary;
				destination_target = _rasterizer_target_screen_effect;
			}
			else
			{
				source_target = _rasterizer_target_screen_effect;
				destination_target = _rasterizer_target_render_primary;
			}

			if ((pass & 1) && parameters->video_on)
			{
				match_assert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
					334,
					pass==1);

				rasterizer_set_target_as_texture(0, source_target, FALSE);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MINFILTER, D3DTEXF_POINT);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 0, D3DTSS_MIPFILTER, D3DTEXF_POINT);

				rasterizer_set_texture_bitmap_data(1, parameters->video_scanline_map);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MAGFILTER, D3DTEXF_POINT);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MINFILTER, D3DTEXF_POINT);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 1, D3DTSS_MIPFILTER, D3DTEXF_POINT);

				match_vassert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
					351,
					!TEST_FLAG(parameters->video_noise_map->flags, _bitmap_linear_bit),
					"video effect noise map cannot be in linear format");

				rasterizer_set_texture_bitmap_data(2, parameters->video_noise_map);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
				IDirect3DDevice8_SetTextureStageState(global_d3d_device, 2, D3DTSS_MIPFILTER, D3DTEXF_POINT);
			}
			else
			{
				for (stage = 0; stage < 4; stage++)
				{
					if (parameters->convolution_type == _rasterizer_screen_effect_convolution_type_none)
					{
						if (pass_count == 1)
						{
							match_assert(
								"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
								371,
								pass==0);

							if (stage == 0)
								rasterizer_set_texture_bitmap_data(0, parameters->convolution_mask);
							else
								continue;
						}
						else if (pass == 0)
						{
							if (stage == 0)
								rasterizer_set_target_as_texture(0, source_target, FALSE);
							else
								continue;
						}
						else if (pass == 1)
						{
							if (parameters->convolution_mask)
							{
								if (stage == 0)
									rasterizer_set_texture_bitmap_data(0, parameters->convolution_mask);
								else if (stage == 1)
									rasterizer_set_target_as_texture(1, source_target, FALSE);
								else
									continue;
							}
							else if (stage == 0)
							{
								rasterizer_set_target_as_texture(0, source_target, FALSE);
							}
							else
							{
								continue;
							}
						}
						else
						{
							match_vassert(
								"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
								398,
								FALSE,
								"### ERROR non-convolution effect tried to render more than 2 passes");
						}
					}
					else if (parameters->convolution_mask)
					{
						if (stage == 0)
							rasterizer_set_texture_bitmap_data(0, parameters->convolution_mask);
						else if (stage == 1)
							rasterizer_set_target_as_texture(1, source_target, FALSE);
						else if (stage == 2)
							rasterizer_set_target_as_texture(2, source_target, FALSE);
						else if (stage == 3)
							rasterizer_set_target_as_texture(3, source_target, FALSE);
						else
							continue;
					}
					else
					{
						if (stage == 0)
							rasterizer_set_target_as_texture(0, source_target, FALSE);
						else if (stage == 1)
							rasterizer_set_target_as_texture(1, source_target, FALSE);
						else if (stage == 2)
							rasterizer_set_target_as_texture(2, source_target, FALSE);
						else if (stage == 3)
							rasterizer_set_target_as_texture(3, source_target, FALSE);
						else
							continue;
					}

					IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
					IDirect3DDevice8_SetTextureStageState(global_d3d_device, stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
				}
			}

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
				FALSE);
			IDirect3DDevice8_SetRenderState(
				global_d3d_device,
				D3DRS_ALPHATESTENABLE,
				FALSE);
			IDirect3DDevice8_SetRenderState(
				global_d3d_device,
				D3DRS_ZENABLE,
				FALSE);
			IDirect3DDevice8_SetRenderState(
				global_d3d_device,
				D3DRS_ZBIAS,
				0);

			/* port: the refinement passes' radius (above) */
			if (parameters == &scaled_parameters)
			{
				scaled_parameters.convolution_radius = rasterizer_screen_effect_pass_radius(
					original_parameters->convolution_radius,
					pass,
					stock_pass_count);
			}
			rasterizer_screen_effect_set_texture_transforms(parameters, pass, pass_count);

			if (destination_target != NONE)
				rasterizer_set_target(destination_target, FALSE, FALSE, FALSE, FALSE);

			if (parameters->video_on)
			{
				csmemset(&pixel_shader, 0, sizeof(pixel_shader));

				if (pass == 0)
				{
					pixel_shader.texture_modes = 0x00000001;
					pixel_shader.combiner_count = 1;
					pixel_shader.final_combiner_inputs_abcd = 0x00000008;
				}
				else if (pass == 1)
				{
					long video_overbright_scales[NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES] =
						{0x00, 0x10, 0x20};

					pixel_shader.texture_modes = 0x00000421;
					pixel_shader.combiner_count = 4;

					pixel_shader.constant_0[0] =
						real_alpha_to_pixel32(PIN(parameters->video_noise_intensity, 0.0f, 1.0f));
					pixel_shader.alpha_inputs[0] = 0x3120111a;
					pixel_shader.alpha_outputs[0] = 0x00000c00;
					pixel_shader.rgb_inputs[0] = 0x3120110a;
					pixel_shader.rgb_outputs[0] = 0x00000c00;
					pixel_shader.rgb_inputs[1] = 0x0c091c19;
					pixel_shader.rgb_outputs[1] = 0x000000c4;
					pixel_shader.rgb_inputs[2] = 0x08080000;
					pixel_shader.rgb_outputs[2] = 0x000000d0;

					match_assert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
						497,
						parameters->video_overbright_mode>=0 && parameters->video_overbright_mode<NUMBER_OF_RASTERIZER_SCREEN_EFFECT_VIDEO_OVERBRIGHT_MODES);

					pixel_shader.rgb_inputs[3] = 0x0d0d0820;
					pixel_shader.rgb_outputs[3] =
						(video_overbright_scales[parameters->video_overbright_mode] << 12) | 0x000000d8;
					pixel_shader.final_combiner_inputs_abcd = 0x2c0d0800;
					pixel_shader.final_combiner_inputs_efg = 0x00000400;

					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_ALPHABLENDENABLE,
						TRUE);
					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_SRCBLEND,
						D3DBLEND_INVSRCALPHA);
					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_DESTBLEND,
						D3DBLEND_ZERO);
					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_BLENDOP,
						D3DBLENDOP_ADD);
				}
				else
				{
					match_vassert(
						"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
						513,
						FALSE,
						"### ERROR video effect tried to render more than 2 passes");
				}

				rasterizer_set_pixel_shader(&pixel_shader);
			}
			else
			{
				short combiner_count = 0;

				csmemset(&pixel_shader, 0, sizeof(pixel_shader));

				if (parameters->convolution_type == _rasterizer_screen_effect_convolution_type_none)
				{
					if (pass_count == 1)
					{
						match_assert(
							"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
							529,
							pass==0);

						pixel_shader.texture_modes = 0x00000001;
						pixel_shader.rgb_inputs[0] = 0x08200000;
						pixel_shader.rgb_outputs[0] = 0x000000c0;
						combiner_count = 1;
						pixel_shader.final_combiner_inputs_efg = 0x00000800;
						pixel_shader.final_combiner_inputs_abcd = 0x0000000c;
					}
					else if (pass == 0)
					{
						pixel_shader.texture_modes = 0x00000001;
						pixel_shader.rgb_inputs[0] = 0x08200000;
						pixel_shader.rgb_outputs[0] = 0x000000c0;
						combiner_count = 1;
						pixel_shader.final_combiner_inputs_efg = 0;
						pixel_shader.final_combiner_inputs_abcd = 0x0000000c;
					}
					else if (pass == 1)
					{
						if (parameters->convolution_mask)
						{
							pixel_shader.texture_modes = 0x00000021;
							pixel_shader.rgb_inputs[0] = 0x09200000;
							pixel_shader.rgb_outputs[0] = 0x000000c0;
							combiner_count = 1;
							pixel_shader.final_combiner_inputs_efg = 0x00000800;
							pixel_shader.final_combiner_inputs_abcd = 0x0000000c;
						}
						else
						{
							pixel_shader.texture_modes = 0x00000021;
							pixel_shader.rgb_inputs[0] = 0x08200000;
							pixel_shader.rgb_outputs[0] = 0x000000c0;
							combiner_count = 1;
							pixel_shader.final_combiner_inputs_efg = 0;
							pixel_shader.final_combiner_inputs_abcd = 0x0000000c;
						}
					}
					else
					{
						match_vassert(
							"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
							584,
							FALSE,
							"### ERROR non-convolution effect tried to render more than 2 passes");
					}
				}
				else
				{
					if (parameters->convolution_mask)
					{
						pixel_shader.texture_modes = 0x00008421;
						pixel_shader.rgb_inputs[0] = 0x89208a20;
						pixel_shader.rgb_outputs[0] = 0x00000c00;
						pixel_shader.constant_0[1] = real_alpha_to_pixel32(1.0f / 3.0f);
						pixel_shader.rgb_inputs[1] = 0xcc118b11;
						pixel_shader.rgb_outputs[1] = 0x00000c00;
						pixel_shader.rgb_inputs[2] = 0xcc20a020;
						pixel_shader.rgb_outputs[2] = 0x00000c00;
						pixel_shader.rgb_inputs[3] = 0x3809180c;
						pixel_shader.rgb_outputs[3] = 0x00000c00;
						combiner_count = 4;
						pixel_shader.final_combiner_inputs_efg = 0x00000800;
					}
					else
					{
						pixel_shader.texture_modes = 0x00008421;
						pixel_shader.rgb_inputs[0] = 0x88208920;
						pixel_shader.rgb_outputs[0] = 0x00030c00;
						pixel_shader.rgb_inputs[1] = 0x8a208b20;
						pixel_shader.rgb_outputs[1] = 0x00030d00;
						pixel_shader.rgb_inputs[2] = 0xcc20cd20;
						pixel_shader.rgb_outputs[2] = 0x00030c00;
						pixel_shader.rgb_inputs[3] = 0xcc20a020;
						pixel_shader.rgb_outputs[3] = 0x00000c00;
						combiner_count = 4;
						pixel_shader.final_combiner_inputs_efg = 0;
					}

					pixel_shader.final_combiner_inputs_abcd = 0x0000000c;
				}

				light_enhancement_input = parameters->filter_light_enhancement_uses_convolution_mask ? 0x68 : 0x20;
				desaturation_input = parameters->filter_desaturation_uses_convolution_mask ? 0x68 : 0x20;
				pixel_shader.constant_0[combiner_count] =
					real_alpha_to_pixel32(parameters->filter_light_enhancement_intensity);
				pixel_shader.constant_1[combiner_count] =
					real_alpha_to_pixel32(parameters->filter_desaturation_intensity);
				pixel_shader.alpha_inputs[combiner_count] =
					(light_enhancement_input << 16) |
					desaturation_input |
					0x11001200;
				pixel_shader.alpha_outputs[combiner_count] = 0x000000cd;

				if (pass == pass_count - 1 &&
					parameters->filter_light_enhancement_intensity > 0.0f)
				{
					pixel_shader.rgb_inputs[combiner_count] = 0x2c2c0000;
					pixel_shader.rgb_outputs[combiner_count] = 0x00000d00;
					combiner_count++;
					pixel_shader.rgb_inputs[combiner_count] = 0x0d0d0000;
					pixel_shader.rgb_outputs[combiner_count] = 0x00000d00;
					combiner_count++;
					pixel_shader.final_combiner_inputs_abcd = 0x3c0c2d00;
					pixel_shader.final_combiner_inputs_efg =
						(parameters->convolution_mask ? 8 : 0) << 8;
				}

				if (pass == pass_count - 1 &&
					parameters->filter_desaturation_intensity > 0.0f)
				{
					real_rgb_color desaturation_weights;

					desaturation_weights.red = 1.0f / 3.0f;
					desaturation_weights.green = 1.0f / 3.0f;
					desaturation_weights.blue = 1.0f / 3.0f;

					if (parameters->filter_light_enhancement_intensity > 0.0f)
					{
						pixel_shader.constant_0[combiner_count] =
							real_alpha_to_pixel32(parameters->filter_light_enhancement_intensity);
						pixel_shader.rgb_inputs[combiner_count] = 0x3c0c1c2d;
						pixel_shader.rgb_outputs[combiner_count] = 0x00000c00;
						combiner_count++;
					}

					pixel_shader.constant_0[combiner_count] =
						real_rgb_color_to_pixel32(&desaturation_weights);
					pixel_shader.rgb_inputs[combiner_count] = 0x0c010000;
					pixel_shader.rgb_outputs[combiner_count] = 0x000020d0;
					combiner_count++;

					if (parameters->filter_desaturation_is_additive)
					{
						pixel_shader.final_combiner_constant_0 =
							real_rgb_color_to_pixel32(&parameters->filter_desaturation_tint);
						pixel_shader.final_combiner_inputs_abcd = 0x1d0f000c;
						pixel_shader.final_combiner_inputs_efg =
							((parameters->convolution_mask ? 8 : 0) | 0x00010d00) << 8;
					}
					else
					{
						pixel_shader.final_combiner_constant_0 =
							real_rgb_color_to_pixel32(&parameters->filter_desaturation_tint);
						pixel_shader.final_combiner_inputs_abcd = 0x1d0f0c00;
						pixel_shader.final_combiner_inputs_efg =
							((parameters->convolution_mask ? 8 : 0) | 0x00010d00) << 8;
					}
				}

				match_assert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
					705,
					combiner_count<=RASTERIZER_MAXIMUM_COMBINER_STAGES);

				pixel_shader.combiner_count = combiner_count | 0x00011000;
				rasterizer_set_pixel_shader(&pixel_shader);

				if (pass_count == 1)
				{
					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_ALPHABLENDENABLE,
						TRUE);
					IDirect3DDevice8_SetRenderState(
						global_d3d_device,
						D3DRS_SRCBLEND,
						D3DBLEND_ZERO);
					SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				}
				else if (pass == pass_count - 1)
				{
					SetRenderStateSmart(D3DRS_ALPHABLENDENABLE, TRUE);
					SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
					SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				}
			}

			viewport_width = global_window_parameters.camera.viewport_bounds.x1 -
				global_window_parameters.camera.viewport_bounds.x0;
			viewport_height = global_window_parameters.camera.viewport_bounds.y1 -
				global_window_parameters.camera.viewport_bounds.y0;

			if (pass == 0 && main_get_window_count() > 1 && pass_count != 1)
			{
				vertex_bounds.x0 = 2 * global_window_parameters.camera.viewport_bounds.x0 *
					(1.0f / (real)halo_screen_width()) - 1.0f;
				vertex_bounds.x1 = 2 * global_window_parameters.camera.viewport_bounds.x1 *
					(1.0f / (real)halo_screen_width()) - 1.0f;
				vertex_bounds.y0 = -2 * global_window_parameters.camera.viewport_bounds.y0 *
					(1.0f / 480.0f) + 1.0f;
				vertex_bounds.y1 = -2 * global_window_parameters.camera.viewport_bounds.y1 *
					(1.0f / 480.0f) + 1.0f;
			}
			else
			{
				vertex_bounds.x0 = vertex_bounds.y1 = -1.0f;
				vertex_bounds.x1 = vertex_bounds.y0 = 1.0f;
			}

			/* port: the quad covers its whole viewport on a screen drawn at
			more pixels than the Xbox's; its texture coordinates are floats,
			moved with it (rasterizer_screen_effect_cover_target) */
			texel_shift = rasterizer_screen_effect_cover_target(
				destination_target,
				viewport_width,
				viewport_height,
				&vertex_bounds);

			if (IDirect3DDevice8_Begin(
				global_d3d_device,
				D3DPT_TRIANGLEFAN) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				4,
				-texel_shift,
				(real)viewport_height - texel_shift) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, -texel_shift, viewport_height - texel_shift)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				_rasterizer_vertex_register_position,
				vertex_bounds.x0,
				vertex_bounds.y1) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x0, vertex_bounds.y1)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				4,
				(real)viewport_width - texel_shift,
				(real)viewport_height - texel_shift) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, viewport_width - texel_shift, viewport_height - texel_shift)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				_rasterizer_vertex_register_position,
				vertex_bounds.x1,
				vertex_bounds.y1) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x1, vertex_bounds.y1)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				4,
				(real)viewport_width - texel_shift,
				-texel_shift) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, viewport_width - texel_shift, -texel_shift)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				_rasterizer_vertex_register_position,
				vertex_bounds.x1,
				vertex_bounds.y0) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x1, vertex_bounds.y0)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				4,
				-texel_shift,
				-texel_shift) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, 4, -texel_shift, -texel_shift)");
			}

			if (IDirect3DDevice8_SetVertexData2f(
				global_d3d_device,
				_rasterizer_vertex_register_position,
				vertex_bounds.x0,
				vertex_bounds.y0) >= 0 && success)
			{
				success = TRUE;
			}
			else
			{
				success = FALSE;
				rasterizer_error(
					0,
					"IDirect3DDevice8_SetVertexData2f(global_d3d_device, VSDE_VERTEX, vertex_bounds.x0, vertex_bounds.y0)");
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
		}

		success = rasterizer_screen_effect_video_borders(parameters->video_on, viewport_width, viewport_height) && success;
		rasterizer_set_target(
			global_window_parameters.rasterizer_target,
			FALSE,
			FALSE,
			FALSE,
			TRUE);
	}

	rasterizer_profile_end(_rasterizer_profile_screen_effect);

	if (!success)
		error(2, "### ERROR rasterizer_screen_effect failed");

	return;
}

void _rasterizer_screen_flash(
	void)
{
	real vertex_constants[5][4];
	real_argb_color inverse_flash_color;
	real_argb_color flash_color;
	pixel32 inverse_flash_pixel;
	pixel32 flash_pixel;
	unsigned long rgb_input;
	unsigned long alpha_input;
	short viewport_width;
	short viewport_height;

	match_assert(
		"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
		786,
		global_d3d_device);

	rasterizer_profile_begin(_rasterizer_profile_screen_flash);

	if (rasterizer_debug_options.screen_flash_enabled &&
		global_window_parameters.screen_flash.type != _render_screen_flash_type_none)
	{
		flash_color.alpha = global_window_parameters.screen_flash.intensity *
			global_window_parameters.screen_flash.color.alpha;
		flash_color.red = global_window_parameters.screen_flash.color.red *
			global_window_parameters.screen_flash.intensity;
		flash_color.green = global_window_parameters.screen_flash.color.green *
			global_window_parameters.screen_flash.intensity;
		flash_color.blue = global_window_parameters.screen_flash.color.blue *
			global_window_parameters.screen_flash.intensity;
		inverse_flash_color.alpha = flash_color.alpha;
		inverse_flash_color.red = (1.0f - global_window_parameters.screen_flash.color.red) *
			global_window_parameters.screen_flash.intensity;
		inverse_flash_color.green = (1.0f - global_window_parameters.screen_flash.color.green) *
			global_window_parameters.screen_flash.intensity;
		inverse_flash_color.blue = (1.0f - global_window_parameters.screen_flash.color.blue) *
			global_window_parameters.screen_flash.intensity;

		flash_pixel = real_argb_color_to_pixel32(&flash_color);
		inverse_flash_pixel = real_argb_color_to_pixel32(&inverse_flash_color);

		rgb_input = 0;
		alpha_input = 0;

		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_CULLMODE,
			D3DCULL_CCW);
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

		switch (global_window_parameters.screen_flash.type)
		{
			case _render_screen_flash_type_lighten:
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_SRCBLEND,
					D3DBLEND_ONE);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_DESTBLEND,
					D3DBLEND_INVSRCALPHA);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDOP,
					D3DBLENDOP_ADD);
				rgb_input = 0x01200000;
				alpha_input = 0x11200000;
				break;

			case _render_screen_flash_type_darken:
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_SRCBLEND,
					D3DBLEND_ONE);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_DESTBLEND,
					D3DBLEND_ONE);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDOP,
					D3DBLENDOP_REVSUBTRACT);
				rgb_input = 0x01200000;
				alpha_input = 0x11200000;
				break;

			case _render_screen_flash_type_max:
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_SRCBLEND,
					D3DBLEND_INVDESTCOLOR);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_DESTBLEND,
					D3DBLEND_INVCONSTANTCOLOR);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDOP,
					D3DBLENDOP_MAX);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDCOLOR,
					flash_pixel);
				rgb_input = 0x01201140;
				alpha_input = 0;
				break;

			case _render_screen_flash_type_min:
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_SRCBLEND,
					D3DBLEND_INVDESTCOLOR);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_DESTBLEND,
					D3DBLEND_INVCONSTANTCOLOR);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDOP,
					D3DBLENDOP_MIN);
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_BLENDCOLOR,
					flash_pixel);
				rgb_input = 0x01201120;
				alpha_input = 0;
				break;

			case _render_screen_flash_type_invert:
				IDirect3DDevice8_SetRenderState(
					global_d3d_device,
					D3DRS_SRCBLEND,
					D3DBLEND_INVDESTCOLOR);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				SetRenderStateSmart(D3DRS_BLENDCOLOR, flash_pixel);
				rgb_input = 0x01411120;
				alpha_input = 0;
				break;

			case _render_screen_flash_type_tint:
				SetRenderStateSmart(D3DRS_SRCBLEND, D3DBLEND_ONE);
				SetRenderStateSmart(D3DRS_DESTBLEND, D3DBLEND_INVCONSTANTCOLOR);
				SetRenderStateSmart(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				flash_pixel = inverse_flash_pixel;
				SetRenderStateSmart(D3DRS_BLENDCOLOR, flash_pixel);
				rgb_input = 0x11200000;
				alpha_input = 0;
				break;

			default:
				match_vassert(
					"c:\\halo\\SOURCE\\rasterizer\\xbox\\rasterizer_xbox_screen_effect.c",
					875,
					FALSE,
					"### ERROR unsupported screen flash type");
				flash_pixel = inverse_flash_pixel;
				break;
		}

		SetRenderStateSmart(D3DRS_ALPHATESTENABLE, FALSE);
		SetRenderStateSmart(D3DRS_ZENABLE, FALSE);
		IDirect3DDevice8_SetRenderState(
			global_d3d_device,
			D3DRS_ZBIAS,
			0);
		rasterizer_set_vertex_shader_permutation(
			_rasterizer_vertex_shader_screen,
			_rasterizer_vertex_type_screen,
			_rasterizer_vertex_shader_permutation_default);

		viewport_width = global_window_parameters.camera.viewport_bounds.x1 -
			global_window_parameters.camera.viewport_bounds.x0;
		viewport_height = global_window_parameters.camera.viewport_bounds.y1 -
			global_window_parameters.camera.viewport_bounds.y0;
		vertex_constants[0][0] = 1.0f / viewport_width * 2.0f;
		vertex_constants[0][1] = 0.0f;
		vertex_constants[0][2] = 0.0f;
		vertex_constants[0][3] = -1.0f - 1.0f / viewport_width;
		vertex_constants[1][0] = 0.0f;
		vertex_constants[1][1] = 1.0f / viewport_height * -2.0f;
		vertex_constants[1][2] = 0.0f;
		vertex_constants[1][3] = 1.0f / viewport_height + 1.0f;
		vertex_constants[2][0] = 0.0f;
		vertex_constants[2][1] = 0.0f;
		vertex_constants[2][2] = 0.0f;
		vertex_constants[2][3] = 0.5f;
		vertex_constants[3][0] = 0.0f;
		vertex_constants[3][1] = 0.0f;
		vertex_constants[3][2] = 0.0f;
		vertex_constants[3][3] = 1.0f;
		vertex_constants[4][0] = 0.0f;
		vertex_constants[4][1] = 0.0f;
		vertex_constants[4][2] = 0.0f;
		vertex_constants[4][3] = 1.0f;
		IDirect3DDevice8_SetVertexShaderConstant(
			global_d3d_device,
			-68,
			vertex_constants,
			5);

		csmemset(&pixel_shader, 0, sizeof(pixel_shader));
		pixel_shader.combiner_count = 1;
		pixel_shader.constant_0[0] = flash_pixel;
		pixel_shader.alpha_inputs[0] = alpha_input;
		pixel_shader.alpha_outputs[0] = 0x00000C00;
		pixel_shader.rgb_inputs[0] = rgb_input;
		pixel_shader.rgb_outputs[0] = 0x00000C00;
		pixel_shader.final_combiner_inputs_abcd = 0x0000000C;
		pixel_shader.final_combiner_inputs_efg = 0x00001C00;
		rasterizer_set_pixel_shader(&pixel_shader);

		viewport_width = global_window_parameters.camera.viewport_bounds.x1 -
			global_window_parameters.camera.viewport_bounds.x0;
		viewport_height = global_window_parameters.camera.viewport_bounds.y1 -
			global_window_parameters.camera.viewport_bounds.y0;

		IDirect3DDevice8_Begin(global_d3d_device, D3DPT_TRIANGLEFAN);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 0, 0, 0);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 0, viewport_width, 0);
		IDirect3DDevice8_SetVertexData2s(
			global_d3d_device,
			0,
			viewport_width,
			viewport_height);
		IDirect3DDevice8_SetVertexData2s(global_d3d_device, 0, 0, viewport_height);
		IDirect3DDevice8_End(global_d3d_device);
	}

	rasterizer_profile_end(_rasterizer_profile_screen_flash);

	return;
}

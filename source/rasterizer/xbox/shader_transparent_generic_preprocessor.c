/*
SHADER_TRANSPARENT_GENERIC_PREPROCESSOR.C
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries/errors.h"
#include "interface/hud_draw.h"
#include "bitmaps/bitmap_color_conversion.h"
#include "shaders/shader_definitions.h"
#include "tag_files/tag_groups.h"
#include "rasterizer/xbox/rasterizer_xbox_pixel_shader.h"

/* ---------- constants */

enum
{
	_shader_type_transparent_generic = 5
};

enum
{
	_shader_transparent_generic_stage_flag_color_mux_bit = 0,
	_shader_transparent_generic_stage_flag_alpha_mux_bit
};

enum
{
	_shader_transparent_generic_stage_output_discard = 0,
	_shader_transparent_generic_stage_output_scratch_color_0,
	_shader_transparent_generic_stage_output_scratch_color_1,
	_shader_transparent_generic_stage_output_vertex_color_0,
	_shader_transparent_generic_stage_output_vertex_color_1,
	_shader_transparent_generic_stage_output_map_color_0,
	_shader_transparent_generic_stage_output_map_color_1,
	_shader_transparent_generic_stage_output_map_color_2,
	_shader_transparent_generic_stage_output_map_color_3,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS
};

enum
{
	_shader_transparent_generic_stage_output_function_multiply = 0,
	_shader_transparent_generic_stage_output_function_dot_product,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS
};

enum
{
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS = 25,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_CONSTANT_INPUTS = 5,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS = 8,
	NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS = 6
};

enum
{
	_shader_stage_output_flag_CD_dot_product_bit = 0,
	_shader_stage_output_flag_AB_dot_product_bit,
	_shader_stage_output_flag_mux_bit
};

enum
{
	_xbox_texture_mode_none = 0,
	_xbox_texture_mode_2d = 1,
	_xbox_texture_mode_cube_map = 3
};

/* ---------- macros */

#define SHADER_STAGE_REGISTER_MASK 0x0f

/* ---------- structures */

struct shader_transparent_generic_map
{
	word flags;
	byte reserved_before_mipmap_bias[22];
	real mipmap_bias;
	struct tag_reference map;
	byte reserved_after_map[56];
};

struct shader_transparent_generic_stage
{
	word flags;
	byte reserved_before_color1[42];
	real_argb_color color1;
	short color_input_A;
	short color_input_A_mapping;
	short color_input_B;
	short color_input_B_mapping;
	short color_input_C;
	short color_input_C_mapping;
	short color_input_D;
	short color_input_D_mapping;
	short color_output_AB;
	short color_output_AB_function;
	short color_output_CD;
	short color_output_CD_function;
	short color_output_AB_CD_mux_sum;
	short color_output_mapping;
	short alpha_input_A;
	short alpha_input_A_mapping;
	short alpha_input_B;
	short alpha_input_B_mapping;
	short alpha_input_C;
	short alpha_input_C_mapping;
	short alpha_input_D;
	short alpha_input_D_mapping;
	short alpha_output_A;
	short alpha_output_B;
	short alpha_output_C;
	short alpha_output_mapping;
};

struct shader_transparent_generic_definition
{
	struct shader shader;
	byte numeric_counter_limit;
	byte flags;
	short first_map_type;
	short framebuffer_blend_function;
	short framebuffer_fade_mode;
	short framebuffer_fade_source;
	short pad;
	real lens_flare_spacing;
	struct tag_reference lens_flare;
	struct tag_block extra_layers;
	struct tag_block maps;
	struct tag_block stages;
};

/* ---------- prototypes */

static long shader_stage_color_input(
	short register_index,
	short mapping_index);
static long shader_stage_color_output(
	short register_index);
static long shader_stage_color_output_flags(
	struct shader_transparent_generic_stage *stage);
static long shader_stage_alpha_input(
	short register_index,
	short mapping_index);
static long shader_stage_alpha_output(
	short register_index);
static long shader_stage_alpha_output_flags(
	struct shader_transparent_generic_stage *stage);
static boolean shader_map_verify(
	struct shader_transparent_generic_map *map,
	short map_index);
static boolean shader_stage_verify(
	struct shader_transparent_generic_stage *stage,
	short stage_index);

/* ---------- globals */

static const long shader_stage_color_input_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS] =
{
	NONE, NONE, NONE, NONE, NONE,
	8, 9, 10, 11,
	4, 5,
	12, 13,
	1, 2,
	24, 25, 26, 27,
	20, 21,
	28, 29,
	17, 18
};

static const long shader_stage_alpha_input_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS] =
{
	NONE, NONE, NONE, NONE, NONE,
	24, 25, 26, 27,
	20, 21,
	28, 29,
	17, 18,
	8, 9, 10, 11,
	4, 5,
	12, 13,
	1, 2
};

static const long shader_stage_color_input_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] =
{
	0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0
};

static const long shader_stage_alpha_input_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] =
{
	0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0
};

static const long shader_stage_constant_input_registers
	[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_CONSTANT_INPUTS]
	[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS] =
{
	{ 0x00, 0x20, 0x40, 0x20, 0x80, 0xa0, 0x00, 0x00 },
	{ 0x20, 0x00, 0x20, 0x40, 0xa0, 0x80, 0x20, 0x40 },
	{ 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x80 },
	{ 0x00, 0x20, 0x40, 0x20, 0x80, 0xa0, 0x40, 0x20 },
	{ 0x00, 0x20, 0x40, 0x20, 0x80, 0xa0, 0x80, 0xa0 }
};

static const long shader_stage_color_output_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS] =
{
	0, 12, 13, 4, 5, 8, 9, 10, 11
};

static const long shader_stage_alpha_output_registers[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS] =
{
	0, 28, 29, 20, 21, 24, 25, 26, 27
};

static const long shader_stage_color_output_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS] =
{
	0x00, 0x30, 0x10, 0x20, 0x08, 0x18
};

static const long shader_stage_alpha_output_mappings[NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS] =
{
	0x00, 0x30, 0x10, 0x20, 0x08, 0x18
};

/* ---------- private code */

static long shader_stage_color_input(
	short register_index,
	short mapping_index)
{
	long register_value;

#line 212 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS);
	match_assert(__FILE__, __LINE__, mapping_index>=0 && mapping_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS);

	register_value = shader_stage_color_input_registers[register_index];

	if (register_value == NONE)
	{
		return shader_stage_constant_input_registers[register_index][mapping_index];
	}

	return register_value | shader_stage_color_input_mappings[mapping_index];
}

static long shader_stage_color_output(
	short register_index)
{
#line 233 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS);

	return shader_stage_color_output_registers[register_index];
}

static long shader_stage_color_output_flags(
	struct shader_transparent_generic_stage *stage)
{
	long flags;

#line 246 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, stage);
	match_assert(__FILE__, __LINE__, stage->color_output_mapping>=0 && stage->color_output_mapping<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS);
	match_assert(__FILE__, __LINE__, stage->color_output_AB_function>=0 && stage->color_output_AB_function<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS);
	match_assert(__FILE__, __LINE__, stage->color_output_CD_function>=0 && stage->color_output_CD_function<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_FUNCTIONS);

	flags = shader_stage_color_output_mappings[stage->color_output_mapping];

	if (stage->color_output_AB_function == _shader_transparent_generic_stage_output_function_dot_product)
	{
		flags |= FLAG(_shader_stage_output_flag_AB_dot_product_bit);
	}

	if (stage->color_output_CD_function == _shader_transparent_generic_stage_output_function_dot_product)
	{
		flags |= FLAG(_shader_stage_output_flag_CD_dot_product_bit);
	}

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_flag_color_mux_bit))
	{
		flags |= FLAG(_shader_stage_output_flag_mux_bit);
	}

	return flags;
}

static long shader_stage_alpha_input(
	short register_index,
	short mapping_index)
{
	long register_value;

#line 266 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUTS);
	match_assert(__FILE__, __LINE__, mapping_index>=0 && mapping_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_INPUT_MAPPINGS);

	register_value = shader_stage_alpha_input_registers[register_index];

	if (register_value == NONE)
	{
		return shader_stage_constant_input_registers[register_index][mapping_index];
	}

	return register_value | shader_stage_alpha_input_mappings[mapping_index];
}

static long shader_stage_alpha_output(
	short register_index)
{
#line 287 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, register_index>=0 && register_index<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUTS);

	return shader_stage_alpha_output_registers[register_index];
}

static long shader_stage_alpha_output_flags(
	struct shader_transparent_generic_stage *stage)
{
	long flags;

#line 300 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, stage);
	match_assert(__FILE__, __LINE__, stage->alpha_output_mapping>=0 && stage->alpha_output_mapping<NUMBER_OF_SHADER_TRANSPARENT_GENERIC_STAGE_OUTPUT_MAPPINGS);

	flags = shader_stage_alpha_output_mappings[stage->alpha_output_mapping];

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_flag_alpha_mux_bit))
	{
		flags |= FLAG(_shader_stage_output_flag_mux_bit);
	}

	return flags;
}

static boolean shader_map_verify(
	struct shader_transparent_generic_map *map,
	short map_index)
{
	boolean result = TRUE;

#line 316 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, map);

	if (map->map.index == NONE)
	{
		error(_error_silent, "### ERROR transparent shader map #%d has no associated bitmap", map_index);
		result = FALSE;
	}

	if (map->mipmap_bias != 0.0f)
	{
		error(_error_silent, "### ERROR unsupported: transparent shader map #%d has non-zero mipmap bias", map_index);
		result = FALSE;
	}

	return result;
}

static boolean shader_stage_verify(
	struct shader_transparent_generic_stage *stage,
	short stage_index)
{
	boolean result = TRUE;

#line 340 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, stage);

	if ((stage->color_output_AB != _shader_transparent_generic_stage_output_discard && stage->color_output_CD != _shader_transparent_generic_stage_output_discard && stage->color_output_AB == stage->color_output_CD) ||
		(stage->color_output_AB != _shader_transparent_generic_stage_output_discard && stage->color_output_AB_CD_mux_sum != _shader_transparent_generic_stage_output_discard && stage->color_output_AB == stage->color_output_AB_CD_mux_sum) ||
		(stage->color_output_CD != _shader_transparent_generic_stage_output_discard && stage->color_output_AB_CD_mux_sum != _shader_transparent_generic_stage_output_discard && stage->color_output_CD == stage->color_output_AB_CD_mux_sum) ||
		(stage->alpha_output_A != _shader_transparent_generic_stage_output_discard && stage->alpha_output_B != _shader_transparent_generic_stage_output_discard && stage->alpha_output_A == stage->alpha_output_B) ||
		(stage->alpha_output_A != _shader_transparent_generic_stage_output_discard && stage->alpha_output_C != _shader_transparent_generic_stage_output_discard && stage->alpha_output_A == stage->alpha_output_C) ||
		(stage->alpha_output_B != _shader_transparent_generic_stage_output_discard && stage->alpha_output_C != _shader_transparent_generic_stage_output_discard && stage->alpha_output_B == stage->alpha_output_C))
	{
		error(_error_silent, "### ERROR transparent shader output conflict in stage #%d", stage_index);
		result = FALSE;
	}

	if ((stage->color_output_AB_function != _shader_transparent_generic_stage_output_function_multiply ||
		stage->color_output_CD_function != _shader_transparent_generic_stage_output_function_multiply) &&
		stage->color_output_AB_CD_mux_sum != _shader_transparent_generic_stage_output_discard)
	{
		error(_error_silent, "### ERROR transparent shader evaluates dot product and AB+CD sum in stage #%d", stage_index);
		result = FALSE;
	}

	if (stage->alpha_output_A == _shader_transparent_generic_stage_output_vertex_color_0 ||
		stage->alpha_output_C == _shader_transparent_generic_stage_output_vertex_color_0)
	{
		error(_error_silent, "### ERROR transparent shader writes to fog density register in stage #%d", stage_index);
		result = FALSE;
	}

	if (TEST_FLAG(stage->flags, _shader_transparent_generic_stage_flag_color_mux_bit) &&
		(stage->color_output_AB_function != _shader_transparent_generic_stage_output_function_multiply ||
		stage->color_output_CD_function != _shader_transparent_generic_stage_output_function_multiply))
	{
		error(_error_silent, "### ERROR transparent shader evaluates dot product and mux[AB,CD] in stage #%d", stage_index);
		result = FALSE;
	}

	return result;
}

/* ---------- public code */

boolean shader_transparent_generic_create(
	struct shader *shader,
	struct pixel_shader_definition *pixel_shader)
{
	struct shader_transparent_generic_definition *generic;
	boolean result = TRUE;
	short map_index;
	short stage_index;

#line 388 "c:\\halo\\SOURCE\\rasterizer\\xbox\\shader_transparent_generic_preprocessor.c"
	match_assert(__FILE__, __LINE__, shader);
	match_assert(__FILE__, __LINE__, pixel_shader);

	generic = (struct shader_transparent_generic_definition *)shader_get_and_verify_type(shader, _shader_type_transparent_generic);

	csmemset(pixel_shader, 0, sizeof(*pixel_shader));

	if (generic->maps.count > 0 || generic->stages.count > 0)
	{
		pixel_shader->texture_modes =
			((((generic->maps.count > 3) << 5 | (generic->maps.count > 2)) << 5 |
			(generic->maps.count > 1)) << 5) |
			(generic->maps.count > 0 ?
			(generic->first_map_type != 0 ? _xbox_texture_mode_cube_map : _xbox_texture_mode_2d) :
			_xbox_texture_mode_none);

		for (map_index = 0; map_index < generic->maps.count; map_index++)
		{
			struct shader_transparent_generic_map *map = TAG_BLOCK_GET_ELEMENT(
				&generic->maps,
				map_index,
				struct shader_transparent_generic_map);

			if (result)
			{
				result = shader_map_verify(map, map_index);
			}
		}
	}
	else
	{
		error(_error_silent, "### ERROR generic shader has no maps or stages");
		result = FALSE;
	}

	pixel_shader->combiner_count = (FLOOR(generic->stages.count, 1) + 1) | 0x11100;

	if (generic->stages.count > 0)
	{
		for (stage_index = 0; stage_index < generic->stages.count; stage_index++)
		{
			struct shader_transparent_generic_stage *stage = TAG_BLOCK_GET_ELEMENT(
				&generic->stages,
				stage_index,
				struct shader_transparent_generic_stage);
			long color_input_A, color_input_B, color_input_C, color_input_D;
			long color_output_AB, color_output_CD, color_output_AB_CD_mux_sum, color_output_flags;
			long alpha_input_A, alpha_input_B, alpha_input_C, alpha_input_D;
			long alpha_output_A, alpha_output_B, alpha_output_C, alpha_output_flags;

			if (result)
			{
				result = shader_stage_verify(stage, stage_index);
			}

			pixel_shader->constant_1[stage_index] = real_argb_color_to_pixel32(&stage->color1);

			color_input_A = shader_stage_color_input(stage->color_input_A, stage->color_input_A_mapping);
			color_input_B = shader_stage_color_input(stage->color_input_B, stage->color_input_B_mapping);
			color_input_C = shader_stage_color_input(stage->color_input_C, stage->color_input_C_mapping);
			color_input_D = shader_stage_color_input(stage->color_input_D, stage->color_input_D_mapping);
			color_output_AB = shader_stage_color_output(stage->color_output_AB);
			color_output_CD = shader_stage_color_output(stage->color_output_CD);
			color_output_AB_CD_mux_sum = shader_stage_color_output(stage->color_output_AB_CD_mux_sum);
			color_output_flags = shader_stage_color_output_flags(stage);

			pixel_shader->rgb_inputs[stage_index] =
				((color_input_A << 8 | color_input_B) << 8 | color_input_C) << 8 | color_input_D;
			pixel_shader->rgb_outputs[stage_index] =
				((color_output_flags << 4 | (color_output_AB_CD_mux_sum & SHADER_STAGE_REGISTER_MASK)) << 4 |
				(color_output_AB & SHADER_STAGE_REGISTER_MASK)) << 4 |
				(color_output_CD & SHADER_STAGE_REGISTER_MASK);

			alpha_input_A = shader_stage_alpha_input(stage->alpha_input_A, stage->alpha_input_A_mapping);
			alpha_input_B = shader_stage_alpha_input(stage->alpha_input_B, stage->alpha_input_B_mapping);
			alpha_input_C = shader_stage_alpha_input(stage->alpha_input_C, stage->alpha_input_C_mapping);
			alpha_input_D = shader_stage_alpha_input(stage->alpha_input_D, stage->alpha_input_D_mapping);
			alpha_output_A = shader_stage_alpha_output(stage->alpha_output_A);
			alpha_output_B = shader_stage_alpha_output(stage->alpha_output_B);
			alpha_output_C = shader_stage_alpha_output(stage->alpha_output_C);
			alpha_output_flags = shader_stage_alpha_output_flags(stage);

			pixel_shader->alpha_inputs[stage_index] =
				((alpha_input_A << 8 | alpha_input_B) << 8 | alpha_input_C) << 8 | alpha_input_D;
			pixel_shader->alpha_outputs[stage_index] =
				((alpha_output_flags << 4 | (alpha_output_C & SHADER_STAGE_REGISTER_MASK)) << 4 |
				(alpha_output_A & SHADER_STAGE_REGISTER_MASK)) << 4 |
				(alpha_output_B & SHADER_STAGE_REGISTER_MASK);
		}
	}
	else
	{
		pixel_shader->rgb_inputs[0] = 0x08200000;
		pixel_shader->rgb_outputs[0] = 0x000000c0;
		pixel_shader->alpha_inputs[0] = 0x18200000;
		pixel_shader->alpha_outputs[0] = 0x000000c0;
	}

	pixel_shader->final_combiner_inputs_abcd = 12;
	pixel_shader->final_combiner_inputs_efg = 0x1c00;

	return result;
}

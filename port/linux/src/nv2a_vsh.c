/*
NV2A_VSH.C

Translation of NV2A vertex programs (Xbox vertex shader microcode) into
GLSL.

Each instruction is four little-endian words. Word 1 to 3 hold a MAC
(vector) operation and an ILU (scalar) operation that execute in parallel
on the same three operands A, B and C, and the destinations of both; the
bit positions of every field are spelled out in operand_fields() and in
nv2a_vertex_shader_to_glsl(). When both units run, the ILU result goes to
temporary r1, whatever the instruction's temporary register field says.

Xbox vertex programs finish by converting their clip-space position to
screen space with the viewport constants c[-38] and c[-37], which Direct3D
maintains. The generated shader inverts that transform to hand OpenGL a
clip-space position again.
*/

#include "xgpu.h"

#include <stdlib.h>
#include <string.h>

/* ---------- instruction fields */

static unsigned long field(const DWORD *instruction, int word, int low_bit, int bit_count)
{
	return (instruction[word] >> low_bit) & ((1UL << bit_count) - 1);
}

enum
{
	_mac_nop, _mac_mov, _mac_mul, _mac_add, _mac_mad, _mac_dp3, _mac_dph, _mac_dp4,
	_mac_dst, _mac_min, _mac_max, _mac_slt, _mac_sge, _mac_arl,
};

enum
{
	_ilu_nop, _ilu_mov, _ilu_rcp, _ilu_rcc, _ilu_rsq, _ilu_exp, _ilu_log, _ilu_lit,
};

enum
{
	_mux_unknown, _mux_temporary, _mux_input, _mux_constant,
};

/* output register addresses (o[]) */
static const char *output_name(unsigned long address)
{
	switch (address)
	{
	case 0: return "oPos";
	case 3: return "oD0";
	case 4: return "oD1";
	case 5: return "oFog";
	case 6: return "oPts";
	case 7: return "oB0";
	case 8: return "oB1";
	case 9: return "oT0";
	case 10: return "oT1";
	case 11: return "oT2";
	case 12: return "oT3";
	default: return "oUnused";
	}
}

static void write_mask(unsigned long mask, char *out)
{
	/* bit 3 is x */
	int count = 0;

	if (mask & 8) out[count++] = 'x';
	if (mask & 4) out[count++] = 'y';
	if (mask & 2) out[count++] = 'z';
	if (mask & 1) out[count++] = 'w';
	out[count] = 0;
}

struct operand_fields
{
	unsigned long negate, swizzle[4], index, mux;
};

static void operand_fields(const DWORD *instruction, char which, struct operand_fields *fields)
{
	switch (which)
	{
	case 'A':
		fields->negate = field(instruction, 1, 8, 1);
		fields->swizzle[0] = field(instruction, 1, 6, 2);
		fields->swizzle[1] = field(instruction, 1, 4, 2);
		fields->swizzle[2] = field(instruction, 1, 2, 2);
		fields->swizzle[3] = field(instruction, 1, 0, 2);
		fields->index = field(instruction, 2, 28, 4);
		fields->mux = field(instruction, 2, 26, 2);
		break;
	case 'B':
		fields->negate = field(instruction, 2, 25, 1);
		fields->swizzle[0] = field(instruction, 2, 23, 2);
		fields->swizzle[1] = field(instruction, 2, 21, 2);
		fields->swizzle[2] = field(instruction, 2, 19, 2);
		fields->swizzle[3] = field(instruction, 2, 17, 2);
		fields->index = field(instruction, 2, 13, 4);
		fields->mux = field(instruction, 2, 11, 2);
		break;
	default:
		fields->negate = field(instruction, 2, 10, 1);
		fields->swizzle[0] = field(instruction, 2, 8, 2);
		fields->swizzle[1] = field(instruction, 2, 6, 2);
		fields->swizzle[2] = field(instruction, 2, 4, 2);
		fields->swizzle[3] = field(instruction, 2, 2, 2);
		fields->index = (field(instruction, 2, 0, 2) << 2) | field(instruction, 3, 30, 2);
		fields->mux = field(instruction, 3, 28, 2);
		break;
	}
}

static void operand(struct xgpu_text *text, const DWORD *instruction, char which, int relative)
{
	static const char swizzle_names[] = "xyzw";
	struct operand_fields fields;
	unsigned long *swizzle = fields.swizzle, index, mux;

	operand_fields(instruction, which, &fields);
	index = fields.index;
	mux = fields.mux;
	xgpu_text_append(text, "%s", fields.negate ? "-" : "");
	switch (mux)
	{
	case _mux_temporary:
		/* r12 reads back the position output */
		if (index == 12)
			xgpu_text_append(text, "oPos");
		else
			xgpu_text_append(text, "r%lu", index);
		break;
	case _mux_input:
		xgpu_text_append(text, "v%lu", field(instruction, 1, 9, 4));
		break;
	case _mux_constant:
		if (relative)
			xgpu_text_append(text, "c[clamp(a0 + %lu, 0, %d)]", field(instruction, 1, 13, 8), XGPU_VERTEX_CONSTANT_COUNT - 1);
		else
			xgpu_text_append(text, "c[%lu]", field(instruction, 1, 13, 8));
		break;
	default:
		xgpu_text_append(text, "vec4(0.0)");
		break;
	}
	xgpu_text_append(text, ".%c%c%c%c",
		swizzle_names[swizzle[0]], swizzle_names[swizzle[1]], swizzle_names[swizzle[2]], swizzle_names[swizzle[3]]);
}

/* ---------- model lighting

The game's model lighting programs (rasterizer_xbox_vertex_shaders_data.inc
9, 10, 17 and 27) sum into the diffuse color oD0.xyz the ambient light
c[-69], two distant lights (direction c[-73] and c[-71], color c[-72] and
c[-70]; the first also lights the back by the translucency c[-82].z) and, in
10 and 17, two point lights (c[-79] to c[-77] and c[-76] to c[-74]: position
and 1 / radius squared, the cone's axis and falloff scale, the color and
falloff offset). The normal and world position they light by are
temporaries that the programs reuse (10 and 17 overwrite both after the
sum). In 9, 10 and 17 the normal is the skinned normal turned by c[-84].w
for the back faces' pass, r0 (written by instruction 17) and the position
the skinned position, r10 (by 12); in 27, of a single node, the normal is
r3 (by 5). They are taken as the first instruction that lights by them
reads them: in 9 the normal before instruction 19, in 10 and 17 the normal
before 24 and the position before 19, in 27 the normal before 7. A program
is taken only where its lighting is the sum nv2a_psh.c computes for each
pixel (the diffuse color written once, by the sum; both distant lights by
one normal; the translucency; each point light by one position, its cone
by its axis reversed) and the registers keep the same normal and position
through it. Nothing else of the programs goes into oD0.xyz; oD0.w is 0, or
in 17 the planar fog's. */

static BOOL xyz_unswizzled(const struct operand_fields *operand)
{
	return operand->swizzle[0] == 0 && operand->swizzle[1] == 1 && operand->swizzle[2] == 2;
}

/* the operand is c[reg] (as the game numbers them: -73), its xyz unswizzled */
static BOOL light_constant(const DWORD *instruction, const struct operand_fields *operand, int reg, BOOL negated)
{
	return operand->mux == _mux_constant && !field(instruction, 3, 1, 1) &&
		field(instruction, 1, 13, 8) == (unsigned long)(XGPU_VERTEX_CONSTANT_BIAS + reg) &&
		operand->negate == (negated ? 1UL : 0UL) && xyz_unswizzled(operand);
}

/* the operand is a temporary (not the position output), its xyz unswizzled */
static BOOL light_temporary(const struct operand_fields *operand, BOOL negated)
{
	return operand->mux == _mux_temporary && operand->index < 12 &&
		operand->negate == (negated ? 1UL : 0UL) && xyz_unswizzled(operand);
}

/* any of x, y and z of temporary reg written by the instructions in [first, last) */
static BOOL temporary_written(const DWORD *instructions, unsigned long first, unsigned long last, unsigned long reg)
{
	unsigned long index;

	for (index = first; index < last; index++)
	{
		const DWORD *instruction = instructions + index * 4;
		unsigned long mac = field(instruction, 1, 21, 4);
		unsigned long ilu = field(instruction, 1, 25, 3);
		unsigned long temporary = field(instruction, 3, 20, 4);

		if (mac != _mac_nop && mac != _mac_arl && temporary == reg && (field(instruction, 3, 24, 4) & 0xe))
			return TRUE;
		/* (the ILU writes r1 when both units run) */
		if (ilu != _ilu_nop && (mac != _mac_nop ? 1 : temporary) == reg && (field(instruction, 3, 16, 4) & 0xe))
			return TRUE;
	}
	return FALSE;
}

BOOL nv2a_vertex_shader_lighting(const DWORD *instructions, unsigned long instruction_count,
	struct nv2a_vertex_lighting *lighting)
{
	unsigned long count, index, diffuse;
	unsigned long distant[2], distant_register[2] = { 0, 0 }, point[2], point_register[2] = { 0, 0 };
	BOOL cone[2] = { FALSE, FALSE }, translucency = FALSE, point_lights = FALSE;
	int light;

	/* to the instruction that ends the program */
	for (count = 0; count < instruction_count;)
	{
		if (field(instructions + count++ * 4, 3, 0, 1))
			break;
	}
	diffuse = distant[0] = distant[1] = point[0] = point[1] = count;
	for (index = 0; index < count; index++)
	{
		const DWORD *instruction = instructions + index * 4;
		unsigned long mac = field(instruction, 1, 21, 4);
		unsigned long output_mask = field(instruction, 3, 12, 4);
		unsigned long constant = field(instruction, 1, 13, 8);
		struct operand_fields a, b, c;

		operand_fields(instruction, 'A', &a);
		operand_fields(instruction, 'B', &b);
		operand_fields(instruction, 'C', &c);
		/* the diffuse color's x, y and z, written once and last by the sum:
		the second distant light's term times its color, plus the rest */
		if ((output_mask & 0xe) && field(instruction, 3, 11, 1) && field(instruction, 3, 3, 8) == 3)
		{
			if (diffuse != count || (output_mask & 0xe) != 0xe || field(instruction, 3, 2, 1) ||
				mac != _mac_mad || !light_constant(instruction, &b, -70, FALSE) || c.mux != _mux_temporary)
			{
				return FALSE;
			}
			diffuse = index;
		}
		for (light = 0; light < 2; light++)
		{
			/* each distant light's facing: the normal's dot product with the
			light's direction reversed */
			if (mac == _mac_dp3 && light_constant(instruction, &b, light ? -71 : -73, TRUE))
			{
				if (distant[light] != count || !light_temporary(&a, FALSE))
					return FALSE;
				distant[light] = index;
				distant_register[light] = a.index;
			}
			/* each point light's direction, from the world position */
			if (mac == _mac_add && light_constant(instruction, &a, light ? -76 : -79, FALSE) &&
				light_temporary(&c, TRUE))
			{
				if (point[light] != count)
					return FALSE;
				point[light] = index;
				point_register[light] = c.index;
			}
			/* and its cone: the direction's dot product with the light's axis
			reversed */
			if (mac == _mac_dp3 && b.mux == _mux_constant && !field(instruction, 3, 1, 1) &&
				constant == (unsigned long)(XGPU_VERTEX_CONSTANT_BIAS + (light ? -75 : -78)))
			{
				if (!light_constant(instruction, &b, light ? -75 : -78, TRUE))
					return FALSE;
				cone[light] = TRUE;
			}
		}
		/* the back lit by the first distant light: its facing reversed, times
		the translucency */
		if (mac == _mac_mul && a.mux == _mux_temporary && a.negate && b.mux == _mux_constant &&
			!field(instruction, 3, 1, 1) && constant == XGPU_VERTEX_CONSTANT_BIAS - 82 && b.swizzle[0] == 2)
		{
			translucency = TRUE;
		}
		if ((a.mux == _mux_constant || b.mux == _mux_constant || c.mux == _mux_constant) &&
			!field(instruction, 3, 1, 1) && constant >= XGPU_VERTEX_CONSTANT_BIAS - 79 &&
			constant <= XGPU_VERTEX_CONSTANT_BIAS - 74)
		{
			point_lights = TRUE;
		}
	}

	if (diffuse == count || distant[0] >= diffuse || distant[1] >= diffuse ||
		distant_register[0] != distant_register[1] || !translucency)
	{
		return FALSE;
	}
	lighting->lights = 1;
	lighting->normal_register = distant_register[0];
	lighting->normal_instruction = distant[0] < distant[1] ? distant[0] : distant[1];
	if (temporary_written(instructions, lighting->normal_instruction, diffuse, lighting->normal_register))
		return FALSE;
	lighting->position_instruction = lighting->position_register = 0;
	if (point_lights)
	{
		unsigned long last = point[0] > point[1] ? point[0] : point[1];

		if (point[0] >= diffuse || point[1] >= diffuse || point_register[0] != point_register[1] ||
			!cone[0] || !cone[1])
		{
			return FALSE;
		}
		lighting->lights = 2;
		lighting->position_register = point_register[0];
		lighting->position_instruction = point[0] < point[1] ? point[0] : point[1];
		if (temporary_written(instructions, lighting->position_instruction, last, lighting->position_register))
			return FALSE;
	}
	return TRUE;
}

/* ---------- translation */

static const char shader_prologue[] =
#ifdef HALO_GLES
	/* the #version line comes first, from the context's capabilities */
	"precision highp float;\n"
	"precision highp int;\n"
#else
	"#version 450 core\n"
#endif
	"uniform vec4 c[192];\n"
	"uniform vec4 viewport_scale;\n"
	"uniform vec4 viewport_offset;\n"
	"uniform float point_size;\n"
	/* columns the menus shift by to center on a wide screen (d3d8_gl.c) */
	"uniform float screen_offset;\n"
	/* a fade above a height on the model (d3d8_gl_set_height_fade): off
	while w is zero */
	"uniform vec4 height_fade;\n"
	/* D3DRS_ZBIAS's depth units with display.reversed_depth, in the
	depth the vertex program writes (d3d8_gl.c, draw_uniforms) */
	"uniform float depth_bias;\n"
	"out vec4 xD0;\n"
	"out vec4 xD1;\n"
	"out vec4 xB0;\n"
	"out vec4 xB1;\n"
	"out vec4 xT0;\n"
	"out vec4 xT1;\n"
	"out vec4 xT2;\n"
	"out vec4 xT3;\n"
	"out float xFog;\n"
	"invariant gl_Position;\n"
	"vec4 unpack_normpacked3(uint p)\n"
	"{\n"
	"	int x = int(p << 21) >> 21;\n"
	"	int y = int(p << 10) >> 21;\n"
	"	int z = int(p) >> 22;\n"
	"	return vec4(float(x) / 1023.0, float(y) / 1023.0, float(z) / 511.0, 1.0);\n"
	"}\n"
	"vec4 nv2a_rcc(float x)\n"
	"{\n"
	"	float r = 1.0 / x;\n"
	"	if (r > 0.0) r = clamp(r, 5.42101e-20, 1.884467e+19);\n"
	"	else r = clamp(r, -1.884467e+19, -5.42101e-20);\n"
	"	return vec4(r);\n"
	"}\n"
	"vec4 nv2a_exp(float x)\n"
	"{\n"
	"	return vec4(exp2(floor(x)), fract(x), exp2(x), 1.0);\n"
	"}\n"
	"vec4 nv2a_log(float x)\n"
	"{\n"
	"	x = abs(x);\n"
	"	if (x == 0.0) return vec4(-1.0e30, 1.0, -1.0e30, 1.0);\n"
	"	float e = floor(log2(x));\n"
	"	return vec4(e, x / exp2(e), log2(x), 1.0);\n"
	"}\n"
	"vec4 nv2a_lit(vec4 s)\n"
	"{\n"
	"	float specular = s.x > 0.0 ? pow(max(s.y, 0.0), clamp(s.w, -127.9961, 127.9961)) : 0.0;\n"
	"	return vec4(1.0, max(s.x, 0.0), specular, 1.0);\n"
	"}\n";

/* display.reversed_depth: the depth of the clip-space position, turned
around (w - z), taken from the rows of the matrix the program transforms the
position by. Halo's programs write oPos.z and oPos.w as dph or dp4 of one
position with consecutive constants c[n] and c[n + 1]; w - z is then that
position with c[n + 1] - c[n], a difference that is exact in floats (the
rows are within a factor of two of each other). Taking z from w per vertex
instead would lose the precision a reversed float depth buffer gives: at a
distance z and w are nearly equal. */
struct reversed_depth_rows
{
	/* the constants oPos.z and oPos.w were read from; -1 until written */
	long z_row, w_row;
	/* the position's operand, which both must read */
	struct operand_fields z_position;
	unsigned long z_position_input;
	/* oPos.z or oPos.w written some other way: w - z instead */
	BOOL other;
};

static BOOL same_operand(const struct operand_fields *a, unsigned long a_input, const struct operand_fields *b,
	unsigned long b_input)
{
	return a->negate == b->negate && a->mux == b->mux && a->index == b->index && a_input == b_input &&
		!memcmp(a->swizzle, b->swizzle, sizeof(a->swizzle));
}

/* an instruction's writes to oPos.z and oPos.w before the position is
converted to screen space, and for oPos.z the reversed depth beside it */
static void reversed_depth_instruction(struct xgpu_text *text, const DWORD *instruction, struct reversed_depth_rows *rows)
{
	unsigned long mac = field(instruction, 1, 21, 4);
	unsigned long ilu = field(instruction, 1, 25, 3);
	unsigned long temporary = field(instruction, 3, 20, 4);
	unsigned long mac_mask = mac != _mac_nop && mac != _mac_arl && temporary == 12 ? field(instruction, 3, 24, 4) : 0;
	unsigned long ilu_mask = ilu != _ilu_nop && mac == _mac_nop && temporary == 12 ? field(instruction, 3, 16, 4) : 0;
	unsigned long output_mask = field(instruction, 3, 11, 1) && field(instruction, 3, 3, 8) == 0 ?
		field(instruction, 3, 12, 4) : 0;
	BOOL output_from_ilu = field(instruction, 3, 2, 1) != 0;
	unsigned long writes = mac_mask | (output_from_ilu ? 0 : output_mask);
	struct operand_fields a, b;
	unsigned long row = field(instruction, 1, 13, 8);
	unsigned long input = field(instruction, 1, 9, 4);

	/* (the ILU's z or w: no row) */
	if ((ilu_mask | (output_from_ilu ? output_mask : 0)) & 3)
		rows->other = TRUE;
	if (!(writes & 3))
		return;
	operand_fields(instruction, 'A', &a);
	operand_fields(instruction, 'B', &b);
	if ((mac != _mac_dph && mac != _mac_dp4) || b.mux != _mux_constant || field(instruction, 3, 1, 1) ||
		b.negate || b.swizzle[0] != 0 || b.swizzle[1] != 1 || b.swizzle[2] != 2 || b.swizzle[3] != 3 ||
		row + 1 >= XGPU_VERTEX_CONSTANT_COUNT || (writes & 3) == 3)
	{
		rows->other = TRUE;
		return;
	}
	if (writes & 2)
	{
		rows->z_row = (long)row;
		rows->z_position = a;
		rows->z_position_input = input;
		if (mac == _mac_dph)
			xgpu_text_append(text, "\treversed_z = dot(A.xyz, c[%lu].xyz - c[%lu].xyz) + (c[%lu].w - c[%lu].w);\n",
				row + 1, row, row + 1, row);
		else
			xgpu_text_append(text, "\treversed_z = dot(A, c[%lu] - c[%lu]);\n", row + 1, row);
	}
	else
	{
		rows->w_row = (long)row;
		if (rows->z_row >= 0 && !same_operand(&a, input, &rows->z_position, rows->z_position_input))
			rows->other = TRUE;
	}
}

char *nv2a_vertex_shader_to_glsl(const DWORD *instructions, unsigned long instruction_count,
	unsigned long packed_attribute_mask, const struct nv2a_vertex_lighting *lighting)
{
	struct xgpu_text text = { 0 };
	unsigned long index;
	struct reversed_depth_rows rows;
	BOOL position_converted = FALSE, reversed_rows;

	rows.z_row = rows.w_row = -1;
	rows.other = FALSE;

#ifdef HALO_GLES
	xgpu_text_append(&text, "#version %s\n", xgpu_capabilities.shading_language);
#endif
	xgpu_text_append(&text, "%s", shader_prologue);
	/* (the pixel shader's model_lighting; the normal's length in w) */
	if (lighting)
		xgpu_text_append(&text, "out vec4 xWorldNormal;\n");
	if (lighting && lighting->lights == 2)
		xgpu_text_append(&text, "out vec3 xWorldPosition;\n");
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
	{
		if (packed_attribute_mask & (1UL << index))
			xgpu_text_append(&text, "layout(location = %lu) in uint v%lu_packed;\n", index, index);
		else
			xgpu_text_append(&text, "layout(location = %lu) in vec4 v%lu_in;\n", index, index);
	}

	xgpu_text_append(&text, "void main()\n{\n");
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
	{
		if (packed_attribute_mask & (1UL << index))
			xgpu_text_append(&text, "\tvec4 v%lu = unpack_normpacked3(v%lu_packed);\n", index, index);
		else
			xgpu_text_append(&text, "\tvec4 v%lu = v%lu_in;\n", index, index);
	}
	xgpu_text_append(&text,
		"\tvec4 r0 = vec4(0.0), r1 = vec4(0.0), r2 = vec4(0.0), r3 = vec4(0.0);\n"
		"\tvec4 r4 = vec4(0.0), r5 = vec4(0.0), r6 = vec4(0.0), r7 = vec4(0.0);\n"
		"\tvec4 r8 = vec4(0.0), r9 = vec4(0.0), r10 = vec4(0.0), r11 = vec4(0.0);\n"
		"\tvec4 oPos = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\tvec4 oD0 = vec4(0.0, 0.0, 0.0, 1.0), oD1 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\tvec4 oB0 = vec4(0.0, 0.0, 0.0, 1.0), oB1 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\tvec4 oT0 = vec4(0.0, 0.0, 0.0, 1.0), oT1 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\tvec4 oT2 = vec4(0.0, 0.0, 0.0, 1.0), oT3 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\tvec4 oFog = vec4(1.0), oPts = vec4(point_size), oUnused = vec4(0.0);\n"
		"\tint a0 = 0;\n"
		"\tvec4 A, B, C, mac, ilu;\n");
	xgpu_text_append(&text, "\tvec4 clip_position = vec4(0.0);\n\tbool clip_captured = false;\n");
	if (xgpu_reversed_depth)
		xgpu_text_append(&text, "\tfloat reversed_z = 0.0;\n");

	for (index = 0; index < instruction_count; index++)
	{
		const DWORD *instruction = instructions + index * 4;
		unsigned long mac = field(instruction, 1, 21, 4);
		unsigned long ilu = field(instruction, 1, 25, 3);
		unsigned long mac_mask = field(instruction, 3, 24, 4);
		unsigned long temporary = field(instruction, 3, 20, 4);
		unsigned long ilu_mask = field(instruction, 3, 16, 4);
		unsigned long output_mask = field(instruction, 3, 12, 4);
		unsigned long output_is_register = field(instruction, 3, 11, 1);
		unsigned long output_address = field(instruction, 3, 3, 8);
		unsigned long output_from_ilu = field(instruction, 3, 2, 1);
		int relative = (int)field(instruction, 3, 1, 1);
		char mask[5];

		xgpu_text_append(&text, "\t/* %lu */\n", index);
		/* the lighting's normal and position, before the program reuses them */
		if (lighting && index == lighting->normal_instruction)
		{
			xgpu_text_append(&text, "\txWorldNormal = vec4(r%lu.xyz, length(r%lu.xyz));\n",
				lighting->normal_register, lighting->normal_register);
		}
		if (lighting && lighting->lights == 2 && index == lighting->position_instruction)
			xgpu_text_append(&text, "\txWorldPosition = r%lu.xyz;\n", lighting->position_register);
		xgpu_text_append(&text, "\tA = "); operand(&text, instruction, 'A', relative); xgpu_text_append(&text, ";\n");
		xgpu_text_append(&text, "\tB = "); operand(&text, instruction, 'B', relative); xgpu_text_append(&text, ";\n");
		xgpu_text_append(&text, "\tC = "); operand(&text, instruction, 'C', relative); xgpu_text_append(&text, ";\n");

		switch (mac)
		{
		case _mac_nop: break;
		case _mac_mov: xgpu_text_append(&text, "\tmac = A;\n"); break;
		case _mac_mul: xgpu_text_append(&text, "\tmac = A * B;\n"); break;
		case _mac_add: xgpu_text_append(&text, "\tmac = A + C;\n"); break;
		case _mac_mad: xgpu_text_append(&text, "\tmac = A * B + C;\n"); break;
		case _mac_dp3: xgpu_text_append(&text, "\tmac = vec4(dot(A.xyz, B.xyz));\n"); break;
		case _mac_dph: xgpu_text_append(&text, "\tmac = vec4(dot(A.xyz, B.xyz) + B.w);\n"); break;
		case _mac_dp4: xgpu_text_append(&text, "\tmac = vec4(dot(A, B));\n"); break;
		case _mac_dst: xgpu_text_append(&text, "\tmac = vec4(1.0, A.y * B.y, A.z, B.w);\n"); break;
		case _mac_min: xgpu_text_append(&text, "\tmac = min(A, B);\n"); break;
		case _mac_max: xgpu_text_append(&text, "\tmac = max(A, B);\n"); break;
		case _mac_slt: xgpu_text_append(&text, "\tmac = vec4(lessThan(A, B));\n"); break;
		case _mac_sge: xgpu_text_append(&text, "\tmac = vec4(greaterThanEqual(A, B));\n"); break;
		case _mac_arl: xgpu_text_append(&text, "\tmac = A;\n"); break;
		default: xgpu_text_append(&text, "\tmac = vec4(0.0);\n"); break;
		}
		switch (ilu)
		{
		case _ilu_nop: break;
		case _ilu_mov: xgpu_text_append(&text, "\tilu = C;\n"); break;
		case _ilu_rcp: xgpu_text_append(&text, "\tilu = vec4(1.0 / C.x);\n"); break;
		case _ilu_rcc: xgpu_text_append(&text, "\tilu = nv2a_rcc(C.x);\n"); break;
		case _ilu_rsq: xgpu_text_append(&text, "\tilu = vec4(inversesqrt(abs(C.x)));\n"); break;
		case _ilu_exp: xgpu_text_append(&text, "\tilu = nv2a_exp(C.x);\n"); break;
		case _ilu_log: xgpu_text_append(&text, "\tilu = nv2a_log(C.x);\n"); break;
		case _ilu_lit: xgpu_text_append(&text, "\tilu = nv2a_lit(C);\n"); break;
		default: xgpu_text_append(&text, "\tilu = vec4(0.0);\n"); break;
		}
		/* the screen-space conversion takes the reciprocal of the clip-space
		position's w (rcc of r12.w); keep the position it converts */
		if (ilu == _ilu_rcc && field(instruction, 3, 28, 2) == _mux_temporary &&
			((field(instruction, 2, 0, 2) << 2) | field(instruction, 3, 30, 2)) == 12)
		{
			xgpu_text_append(&text, "\tclip_position = oPos;\n\tclip_captured = true;\n");
			position_converted = TRUE;
		}
		else if (xgpu_reversed_depth && !position_converted)
		{
			reversed_depth_instruction(&text, instruction, &rows);
		}

		/* results are written only after both units have read their inputs */
		if (mac == _mac_arl)
		{
			xgpu_text_append(&text, "\ta0 = int(floor(mac.x + 0.001));\n");
		}
		else if (mac != _mac_nop && mac_mask)
		{
			write_mask(mac_mask, mask);
			if (temporary == 12)
				xgpu_text_append(&text, "\toPos.%s = mac.%s;\n", mask, mask);
			else
				xgpu_text_append(&text, "\tr%lu.%s = mac.%s;\n", temporary, mask, mask);
		}
		if (ilu != _ilu_nop && ilu_mask)
		{
			unsigned long ilu_temporary = mac != _mac_nop ? 1 : temporary;

			write_mask(ilu_mask, mask);
			if (ilu_temporary == 12)
				xgpu_text_append(&text, "\toPos.%s = ilu.%s;\n", mask, mask);
			else
				xgpu_text_append(&text, "\tr%lu.%s = ilu.%s;\n", ilu_temporary, mask, mask);
		}
		if (output_mask && (output_from_ilu ? ilu : mac) != 0)
		{
			const char *source = output_from_ilu ? "ilu" : "mac";

			write_mask(output_mask, mask);
			if (output_is_register)
				xgpu_text_append(&text, "\t%s.%s = %s.%s;\n", output_name(output_address), mask, source, mask);
			/* writes to constant memory are not used by Halo's shaders */
		}
		if (field(instruction, 3, 0, 1))
			break;
	}

	xgpu_text_append(&text,
		"\tif (height_fade.w != 0.0)\n"
		"\t{\n"
		"\t\tfloat faded = clamp((height_fade.y - (oT0.z - height_fade.z) / height_fade.w) * height_fade.x, 0.0, 1.0);\n"
		"\t\toD0 *= faded;\n"
		"\t\toD1 *= faded;\n"
		"\t}\n");
	xgpu_text_append(&text,
		"\t/* undo the screen-space conversion done with c[-38] and c[-37] */\n"
		"\tvec3 scale = vec3(viewport_scale.x != 0.0 ? viewport_scale.x : 1.0,\n"
		"\t\tviewport_scale.y != 0.0 ? viewport_scale.y : 1.0,\n"
		"\t\tviewport_scale.z != 0.0 ? viewport_scale.z : 1.0);\n"
		/* Direct3D 8 puts pixel centres on integer screen coordinates (the
		game offsets its screen-space quads by -0.5 to match), OpenGL on
		half-integers */
		/* The conversion is screen = clip * c[-38] * rcc(w) + c[-37]; undoing
		it by multiplying by w again is lossy near the camera plane, where
		rcc clamps and 1/w rounds differently on each GPU (Mali put vertices
		of the first-person weapon at the vanishing point). Where the clip
		position was kept, the same result is computed without dividing. */
		"\tvec4 position;\n"
		"\tif (clip_captured)\n"
		"\t\tposition = vec4((clip_position.xyz * c[%d].xyz + (c[%d].xyz + vec3(0.5 + screen_offset, 0.5, 0.0)\n"
		"\t\t\t- viewport_offset.xyz) * clip_position.w) / scale, clip_position.w);\n"
		"\telse\n"
		"\t{\n"
		"\t\tvec3 ndc = (vec3(oPos.xy + vec2(0.5 + screen_offset, 0.5), oPos.z) - viewport_offset.xyz) / scale;\n"
		"\t\tposition = vec4(ndc * oPos.w, oPos.w);\n"
		"\t}\n",
		XGPU_VERTEX_CONSTANT_BIAS - 38, XGPU_VERTEX_CONSTANT_BIAS - 37);
	/* display.reversed_depth: Direct3D's depth turned around, 1 - z / w.
	With the rows found, the clip-space z above is a z + b w (a the depth
	scale's c[-38].z / scale.z, b its offset's), so w - (a z + b w) is a times
	the reversed depth plus (1 - a - b) w, which is 0 where the game's
	conversion is the device's own. Else, from z itself. */
	reversed_rows = rows.z_row >= 0 && rows.w_row == rows.z_row + 1 && !rows.other && position_converted;
	if (xgpu_reversed_depth && reversed_rows)
	{
		xgpu_text_append(&text,
			"\tif (clip_captured)\n"
			"\t\tposition.z = (reversed_z * c[%d].z + (scale.z - c[%d].z - c[%d].z + viewport_offset.z) * clip_position.w)\n"
			"\t\t\t/ scale.z;\n"
			"\telse\n"
			"\t\tposition.z = position.w - position.z;\n",
			XGPU_VERTEX_CONSTANT_BIAS - 38, XGPU_VERTEX_CONSTANT_BIAS - 38, XGPU_VERTEX_CONSTANT_BIAS - 37);
	}
	else if (xgpu_reversed_depth)
	{
		xgpu_text_append(&text, "\tposition.z = position.w - position.z;\n");
	}
	/* A z bias moves a decal's depth toward the camera by a number of the
	Xbox's 24-bit depth units. A polygon offset's units in a floating-point
	depth buffer are a part of the depth itself, which reversed is small far
	away: 8 of them moved a decal a hundredth of what the Xbox did at ten
	world units, and an additive light on a curved hull (the Pelican's)
	sank into it in slivers. So the units are added here, in the Xbox's
	measure; the polygon offset keeps only the slope (d3d8_gl.c). */
	if (xgpu_reversed_depth)
		xgpu_text_append(&text, "\tposition.z += depth_bias * position.w;\n");
	xgpu_text_append(&text,
		/* A position whose w is zero, or is not a number, is the clip-space
		origin: the screen conversion's reciprocal is clamped rather than
		infinite, so a large position times a w of zero is exactly zero, and
		the origin is inside the frustum. Nothing then clips the triangle
		away and OpenGL divides zero by zero there: the vertex lands on the
		middle of the screen and the triangle is drawn out to it from the
		first-person weapon, whose pose follows the camera and so reaches the
		camera plane. The divide on the Xbox sends such a vertex to infinity
		and the clipper takes the triangle; put it behind the camera instead,
		which the clipper also takes. */
		"\tif (!(abs(position.w) > 0.0))\n"
		"\t\tposition = vec4(0.0, 0.0, 0.0, -1.0);\n"
		"\tgl_Position = position;\n"
#ifdef HALO_GLES
		/* what glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE) does on desktop
		GL: rows from the top, depth 0..1 */
		"\tgl_Position.y = -gl_Position.y;\n"
		"\tgl_Position.z = 2.0 * gl_Position.z - gl_Position.w;\n"
#endif
		"\tgl_PointSize = oPts.x;\n"
		"\txD0 = clamp(oD0, 0.0, 1.0);\n"
		"\txD1 = clamp(oD1, 0.0, 1.0);\n"
		"\txB0 = clamp(oB0, 0.0, 1.0);\n"
		"\txB1 = clamp(oB1, 0.0, 1.0);\n"
		"\txT0 = oT0;\n"
		"\txT1 = oT1;\n"
		"\txT2 = oT2;\n"
		"\txT3 = oT3;\n"
		"\txFog = oFog.x;\n"
		"}\n");
	return text.buffer;
}

/* The fixed-function pipeline's vertex shader, for the draws that select it
(SetVertexShader with 0): only the loading screen's
(progress_bar.c), which gives each vertex its position, diffuse color and
up to four texture coordinates in immediate mode. d3d8_gl.c transforms the
positions by the world, view and projection matrices before the draw, so the
position here is already in Direct3D's clip space. */
char *nv2a_fixed_function_glsl(void)
{
	struct xgpu_text text = { 0 };
	unsigned long index;

#ifdef HALO_GLES
	xgpu_text_append(&text, "#version %s\n", xgpu_capabilities.shading_language);
#endif
	xgpu_text_append(&text, "%s", shader_prologue);
	for (index = 0; index < XGPU_VERTEX_ATTRIBUTE_COUNT; index++)
		xgpu_text_append(&text, "layout(location = %lu) in vec4 v%lu_in;\n", index, index);
	xgpu_text_append(&text,
		"void main()\n{\n"
		"\tgl_Position = v0_in;\n"
#ifdef HALO_GLES
		"\tgl_Position.y = -gl_Position.y;\n"
		"\tgl_Position.z = 2.0 * gl_Position.z - gl_Position.w;\n"
#endif
		"%s"
		"\tgl_PointSize = 1.0;\n"
		"\txD0 = clamp(v3_in, 0.0, 1.0);\n"
		"\txD1 = clamp(v4_in, 0.0, 1.0);\n"
		"\txB0 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\txB1 = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"\txT0 = v9_in;\n"
		"\txT1 = v10_in;\n"
		"\txT2 = v11_in;\n"
		"\txT3 = v12_in;\n"
		"\txFog = 1.0;\n"
		"}\n",
		/* (display.reversed_depth) */
		xgpu_reversed_depth ? "\tgl_Position.z = gl_Position.w - gl_Position.z;\n" : "");
	return text.buffer;
}

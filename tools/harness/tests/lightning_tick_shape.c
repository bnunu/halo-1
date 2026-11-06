/*
LIGHTNING_TICK_SHAPE.C (test)

The real lightning_submit (lightning.c), with lightning_new and
lightnings_initialize_for_new_map, over a fake renderer that keeps each bolt
it is given: a bolt keeps its shape through the frames of a tick and takes a
new one with the next tick, as the Xbox drew it once a tick, and the frames
after a tick's first leave the local random seed as they found it. A bolt's
random offsets (its markers', its points' jitter, its texture's) came from the
local random seed each frame, so at high frame rates it took a new shape each
frame, flickering several times as fast. Each window of a split screen draws
its bolt from its own seed, and a new lightning does not take the one before
it's. test_lightning_tick_shape.py takes the constants, structures and code
from the sources (config.inc, under_test.inc).
*/

#include <math.h>
#include "harness.h"

typedef unsigned short word;
typedef unsigned long pixel32;
typedef struct { real x, y; } real_point2d;
typedef struct { real red, green, blue; } real_rgb_color;
typedef struct { real alpha, red, green, blue; } real_argb_color;
typedef struct { real x0, x1, y0, y1, z0, z1; } real_rectangle3d;
typedef struct
{
	real scale;
	real_vector3d forward;
	real_vector3d left;
	real_vector3d up;
	real_point3d position;
} real_matrix4x3;
struct tag_reference { unsigned long group_tag; char const *name; long name_length; long index; };
struct tag_block { long count; void *address; void const *definition; };
#define TAG_BLOCK_GET_ELEMENT(block, element, type) (&((type *)(block)->address)[(element)])
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SHORT_MAX 32767
#define SHORT_BITS 16
#define csmemset memset
enum { _object_function_reference_none, _object_function_reference_a, _object_function_reference_b,
	_object_function_reference_c, _object_function_reference_d };
enum { _error_silent };

#include "config.inc"

/* the game's time, in ticks */
static long game_time;
static long game_time_get(void) { return game_time; }

/* the local random seed (random_math.c's: a linear congruential generator) */
static unsigned long local_random_seed = 0x2468ace0UL;
static unsigned long *get_global_local_random_seed_address(void) { return &local_random_seed; }
static real real_seed_random(unsigned long *seed)
{
	*seed = 1664525UL * *seed + 1013904223UL;
	return (real)(*seed >> 16) / 65535.0f;
}
static real real_seed_random_range(unsigned long *seed, real lower, real upper)
{
	return lower + (upper - lower) * real_seed_random(seed);
}

/* the math the bolt is drawn with */
static void set_real_vector3d(real_vector3d *v, real i, real j, real k) { v->i = i; v->j = j; v->k = k; }
static void set_real_point3d(real_point3d *p, real x, real y, real z) { p->x = x; p->y = y; p->z = z; }
static void matrix4x3_transform_vector(real_matrix4x3 const *m, real_vector3d const *in, real_vector3d *out)
{
	real_vector3d v = *in;

	out->i = m->scale * (m->forward.i * v.i + m->left.i * v.j + m->up.i * v.k);
	out->j = m->scale * (m->forward.j * v.i + m->left.j * v.j + m->up.j * v.k);
	out->k = m->scale * (m->forward.k * v.i + m->left.k * v.j + m->up.k * v.k);
}
static void vector_from_points3d(real_point3d const *a, real_point3d const *b, real_vector3d *v)
{
	v->i = b->x - a->x;
	v->j = b->y - a->y;
	v->k = b->z - a->z;
}
static void cross_product3d(real_vector3d const *a, real_vector3d const *b, real_vector3d *out)
{
	real_vector3d v;

	v.i = a->j * b->k - a->k * b->j;
	v.j = a->k * b->i - a->i * b->k;
	v.k = a->i * b->j - a->j * b->i;
	*out = v;
}
static real normalize3d(real_vector3d *v)
{
	real length = sqrtf(v->i * v->i + v->j * v->j + v->k * v->k);

	if (length > 0.0f)
	{
		v->i /= length;
		v->j /= length;
		v->k /= length;
	}
	return length;
}
static void fast_normalize3d(real_vector3d *v) { normalize3d(v); }
static void scale_vector3d(real_vector3d const *v, real s, real_vector3d *out)
{
	out->i = v->i * s;
	out->j = v->j * s;
	out->k = v->k * s;
}
static pixel32 real_argb_color_to_pixel32(real_argb_color const *c)
{
	return ((pixel32)(c->alpha * 255.0f) << 24) | ((pixel32)(c->red * 255.0f) << 16) |
		((pixel32)(c->green * 255.0f) << 8) | (pixel32)(c->blue * 255.0f);
}
static real_rgb_color const white = { 1.0f, 1.0f, 1.0f };
static real_rgb_color const *global_real_rgb_white = &white;
static real_point3d const z_axis = { 0.0f, 0.0f, 1.0f };
static real_point3d const *global_z_axis3d = &z_axis;

/* the objects: a lightning's two markers, "start" and "end", where the frame
draws them */
struct object_marker { short node_index; real_matrix4x3 node_matrix; real_matrix4x3 matrix; };
static real_point3d marker_start = { 0.0f, 0.0f, 0.0f };
static real_point3d marker_end = { 2.0f, 0.0f, 0.0f };
static short object_get_marker_by_name(long object_index, char const *name, struct object_marker *marker, short count)
{
	memset(marker, 0, sizeof(*marker));
	marker->matrix.scale = 1.0f;
	marker->matrix.forward.i = marker->matrix.left.j = marker->matrix.up.k = 1.0f;
	marker->matrix.position = strcmp(name, "start") ? marker_end : marker_start;
	return 1;
}

/* the textures and shaders */
struct bitmap_data { short width; };
struct bitmap_group { struct tag_block bitmaps; };
static struct bitmap_data bolt_bitmap;
static struct bitmap_group bolt_bitmaps = { { 1, &bolt_bitmap, NULL } };
static struct bitmap_group *bitmap_group_get(long index) { return &bolt_bitmaps; }
static boolean _texture_cache_bitmap_get_hardware_format(struct bitmap_data *bitmap, boolean a, boolean b)
{
	return TRUE;
}
struct shader { long type; };
struct shader_effect_definition { struct shader shader; };
static struct shader_effect_definition global_shader_effect_additive;
struct render_lighting;
struct render_animation { real_rgb_color const *colors; real const *values; };
static void error(int priority, char const *format, ...) { }
static char const *tag_get_name(long index) { return "lightning"; }

/* the frame being drawn: its window */
static struct { struct { real_vector3d forward; } camera; short window_index; } render = { { { 0.0f, 1.0f, 0.0f } }, 0 };

/* the renderer: the bolt it was last given */
enum { _rasterizer_lock_none, _rasterizer_lock_lightning };
enum { _rasterizer_vertex_type_dynamic_unlit = 1 };
static struct { short current_lock_operation; } rasterizer_globals;
enum { MAXIMUM_VERTICES = 64 };
static struct lightning_vertex vertex_buffer[MAXIMUM_VERTICES];
static struct lightning_vertex drawn[MAXIMUM_VERTICES];
static long drawn_count;
static long vertex_buffer_count;
static long rasterizer_dynamic_vertices_new(short type, long count)
{
	CHECK(count <= MAXIMUM_VERTICES, "a bolt of %ld vertices", count);
	vertex_buffer_count = count;
	return 0;
}
static void *rasterizer_dynamic_vertices_lock(long index) { return vertex_buffer; }
static void rasterizer_dynamic_vertices_unlock(long index) { }
static void rasterizer_dynamic_vertices_delete(long index) { }
static void rasterizer_dynamic_unlit_geometry_draw(struct shader const *shader, struct bitmap_data const *bitmap,
	struct render_animation const *animation, long triangles, long vertex_buffer_index, long vertex_count,
	real_point3d const *centroid, unsigned long flags)
{
	memcpy(drawn, vertex_buffer, sizeof(drawn));
	drawn_count = vertex_buffer_count;
}

/* the lightnings */
static struct lightning_globals lightning_globals;
#define lightning_get(lightning_index) \
	((struct lightning_datum *)datum_get(lightning_globals.lightning_data, (lightning_index)))
static void data_make_valid(struct data_array *array) { }

/* the lightning's definition: a bolt from "start" to "end", 8 points to the
segment, its ends moved and its points jittered at random */
static struct lightning_marker_definition markers[2];
static struct lightning_definition bolt_definition;
#define lightning_definition_get(definition_index) (&bolt_definition)

#include "under_test.inc"

static void define_bolt(void)
{
	short index;

	for (index = 0; index < 2; index++)
	{
		memset(&markers[index], 0, sizeof(markers[index]));
		strcpy(markers[index].attachment_marker, index ? "end" : "start");
		markers[index].octaves_to_next_marker = 3;
		set_real_vector3d(&markers[index].random_position_bounds, 0.1f, 0.1f, 0.1f);
		markers[index].random_jitter_offset = 0.2f;
		markers[index].thickness = 0.05f;
		markers[index].tint.alpha = markers[index].tint.red = markers[index].tint.green = markers[index].tint.blue = 1.0f;
	}
	memset(&bolt_definition, 0, sizeof(bolt_definition));
	bolt_definition.count = 1;
	bolt_definition.markers.count = 2;
	bolt_definition.markers.address = markers;
	lightning_globals.lightning_data = game_state_data_new("lightnings", 256, sizeof(struct lightning_datum));
	lightnings_initialize_for_new_map();
}

/* a bolt as drawn: its vertices */
struct bolt { struct lightning_vertex vertices[MAXIMUM_VERTICES]; long count; };

/* a frame: the bolt of a lightning, drawn in a window, after the other
effects of the frame have used some of the local seed's numbers */
static struct bolt frame(long lightning, short window, short others)
{
	struct bolt bolt;

	while (others-- > 0)
		real_seed_random(&local_random_seed);
	render.window_index = window;
	drawn_count = 0;
	lightning_submit(7, lightning, NULL, NULL);
	CHECK(drawn_count > 4, "no bolt drawn (%ld vertices)", drawn_count);
	memcpy(bolt.vertices, drawn, sizeof(bolt.vertices));
	bolt.count = drawn_count;
	return bolt;
}

/* the second bolt the first moved by (dx, 0, 0), vertex by vertex */
static boolean same_shape(struct bolt const *a, struct bolt const *b, real dx)
{
	long index;

	if (a->count != b->count)
		return FALSE;
	for (index = 0; index < a->count; index++)
	{
		if (fabsf(a->vertices[index].point.x + dx - b->vertices[index].point.x) > 1e-5f ||
			fabsf(a->vertices[index].point.y - b->vertices[index].point.y) > 1e-5f ||
			fabsf(a->vertices[index].point.z - b->vertices[index].point.z) > 1e-5f ||
			fabsf(a->vertices[index].texture.x - b->vertices[index].texture.x) > 1e-5f)
		{
			return FALSE;
		}
	}
	return TRUE;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	long lightning;

	define_bolt();
	lightning = lightning_new(0);

	/* the frames of a tick draw the tick's bolt, whatever the frame's other
	effects took from the seed; the next tick's is a new one */
	CASE("same-shape-in-a-tick")
	{
		struct bolt first, again, next;
		short index;

		game_time = 10;
		first = frame(lightning, 0, 0);
		for (index = 1; index < 8; index++)
		{
			again = frame(lightning, 0, (short)(index * 3));
			CHECK(same_shape(&first, &again, 0.0f), "frame %d of a tick drew another bolt than its first", index);
		}
		game_time = 11;
		next = frame(lightning, 0, 2);
		CHECK(!same_shape(&first, &next, 0.0f), "the next tick drew the same bolt");
		again = frame(lightning, 0, 5);
		CHECK(same_shape(&next, &again, 0.0f), "the next tick's later frame drew another bolt than its first");
		return 0;
	}
	/* the bolt's ends follow its markers where each frame draws them: the
	same shape, moved with them */
	CASE("follows-its-markers")
	{
		struct bolt first, moved;

		game_time = 10;
		first = frame(lightning, 0, 0);
		marker_start.x += 0.5f;
		marker_end.x += 0.5f;
		moved = frame(lightning, 0, 4);
		CHECK(same_shape(&first, &moved, 0.5f), "a frame of the tick with the markers moved drew another shape");
		return 0;
	}
	/* a tick's first frame takes the seed's numbers as the Xbox's frame did;
	the frames after it leave the seed as they found it */
	CASE("seed-left-as-found")
	{
		unsigned long before;
		short index;

		game_time = 10;
		before = local_random_seed;
		frame(lightning, 0, 0);
		CHECK(local_random_seed != before, "the tick's first bolt took no numbers from the seed");
		/* (the frame's other effects take some first) */
		for (index = 0; index < 3; index++)
			real_seed_random(&local_random_seed);
		before = local_random_seed;
		frame(lightning, 0, 0);
		CHECK(local_random_seed == before, "a later frame of the tick moved the seed on (%08lx, not %08lx)",
			local_random_seed, before);
		return 0;
	}
	/* split screen: each window's bolt is its own, kept for the tick */
	CASE("windows-apart")
	{
		struct bolt first0, first1, again0, again1;

		game_time = 10;
		first0 = frame(lightning, 0, 0);
		first1 = frame(lightning, 1, 0);
		CHECK(!same_shape(&first0, &first1, 0.0f), "the second window drew the first window's bolt");
		again0 = frame(lightning, 0, 3);
		again1 = frame(lightning, 1, 3);
		CHECK(same_shape(&first0, &again0, 0.0f) && same_shape(&first1, &again1, 0.0f),
			"a window's later frame drew another bolt than its first");
		return 0;
	}
	/* a lightning made where another was, in the same tick, draws its own
	bolt, not the other's */
	CASE("new-lightning")
	{
		struct bolt old, renewed;

		game_time = 10;
		old = frame(lightning, 0, 0);
		datum_delete(lightning_globals.lightning_data, lightning);
		lightning = lightning_new(0);
		CHECK(DATUM_INDEX_TO_ABSOLUTE_INDEX(lightning) == 0, "the new lightning is not where the old one was");
		renewed = frame(lightning, 0, 0);
		CHECK(!same_shape(&old, &renewed, 0.0f), "the new lightning drew the old one's bolt");
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

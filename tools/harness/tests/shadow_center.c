/*
SHADOW_CENTER.C (test)

The real render_object_shadow_begin (render_objects.c): an object's shadow map
is centered where the frame draws the object (render_interpolation.c, Smooth
Motion's blend between the ticks), as its model is drawn into the map, not
where the tick left it. Centered at the tick, a fast vehicle (a Ghost) stood
up to a tick's travel off its map's center, and the map's edge and corner fade
cut its shadow's trailing side, flickering frame by frame.
test_shadow_center.py takes the structure and the code (under_test.inc).
*/

#include "harness.h"

typedef unsigned short word;
typedef struct { real scale; real_vector3d forward, left, up; real_point3d position; } real_matrix4x3;
union real_rgb_color { struct { real red, green, blue; }; real n[3]; };
struct render_lighting { real_vector3d shadow_vector; union real_rgb_color shadow_color; };

#include "config.inc"

/* the object, a vehicle: not a unit with camouflage */
enum { _object_mask_unit = 3 };
struct object_datum { struct { long type; } object; };
struct unit_datum { struct { real active_camouflage; } unit; };
static struct object_datum object_data;
static struct unit_datum unit_data;
#define object_get(object_index) (&object_data)
#define unit_get(object_index) (&unit_data)

/* where the tick left the object's center, and how far this frame draws it
from there (render_interpolation_object_point) */
static real_point3d tick_center = { 10.0f, 20.0f, 3.0f };
static real_vector3d drawn_offset;
static void object_get_bounding_sphere(long object_index, real_point3d *center, real *radius)
{
	*center = tick_center;
	*radius = 1.5f;
}
static void render_interpolation_object_point(long object_index, real_point3d *point)
{
	point->x += drawn_offset.i;
	point->y += drawn_offset.j;
	point->z += drawn_offset.k;
}

static void perpendicular3d(real_vector3d const *vector, real_vector3d *perpendicular)
{
	perpendicular->i = 1.0f;
	perpendicular->j = 0.0f;
	perpendicular->k = 0.0f;
}
#define normalize3d(vector) ((void)0)
static void matrix4x3_from_point_and_vectors(real_matrix4x3 *matrix, real_point3d const *point,
	real_vector3d const *forward, real_vector3d const *up)
{
	matrix->position = *point;
}

/* the shadow map's center, as the rasterizer is given it */
static real_point3d map_center;
static boolean rasterizer_environment_shadow_begin(long object_index, real_matrix4x3 const *matrix,
	union real_rgb_color const *color, real radius, real *bounding_radius)
{
	map_center = matrix->position;
	*bounding_radius = radius;
	return TRUE;
}

#include "under_test.inc"

static boolean near(real a, real b) { return a - b < 0.0001f && b - a < 0.0001f; }

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	struct render_lighting lighting = { { 0.0f, 0.0f, -1.0f }, { { 0.5f, 0.5f, 0.5f } } };
	struct object_render_data data;

	memset(&data, 0, sizeof(data));
	data.object_index = 7;
	data.lighting = &lighting;

	/* a frame between ticks draws the vehicle half a world unit back from
	the tick: the map is centered there */
	CASE("centered-where-drawn")
	{
		drawn_offset.i = -0.5f;
		drawn_offset.j = 0.25f;
		CHECK(render_object_shadow_begin(&data, 1.0f), "the shadow was not begun");
		CHECK(near(map_center.x, 9.5f) && near(map_center.y, 20.25f) && near(map_center.z, 3.0f),
			"the shadow map is centered at %g %g %g, not where the vehicle is drawn (9.5 20.25 3)",
			map_center.x, map_center.y, map_center.z);
		CHECK(near(data.shadow_matrix.position.x, 9.5f) && near(data.shadow_matrix.position.y, 20.25f),
			"the shadow's projection is centered at %g %g, not where the map is",
			data.shadow_matrix.position.x, data.shadow_matrix.position.y);
		return 0;
	}
	/* drawn at the tick (Smooth Motion off, or a frame on a tick): the tick's
	center, as on the Xbox */
	CASE("at-the-tick")
	{
		CHECK(render_object_shadow_begin(&data, 1.0f), "the shadow was not begun");
		CHECK(near(map_center.x, 10.0f) && near(map_center.y, 20.0f) && near(map_center.z, 3.0f),
			"the shadow map is centered at %g %g %g, not the tick's 10 20 3", map_center.x, map_center.y,
			map_center.z);
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

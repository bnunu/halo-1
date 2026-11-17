#include "harness.h"
#include "declarations.inc"

/* A teammate on screen.
The map's globals have a multiplayer HUD bitmap,
or none (NONE), as on some Custom Edition campaign maps. */
#define PIN(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
typedef struct { real x, y; } real_point2d;
typedef struct { short x, y; } point2d;
typedef struct { real x0, x1, y0, y1; } real_rectangle2d;
typedef struct { real m[4][3]; } real_matrix4x3;
struct bitmap_data { short format; };
struct player_datum { long unit_index; };
static struct
{
	struct { real_matrix4x3 world_to_view; real z_near, z_far; } frustum;
	struct { struct { short x0, y0; } viewport_bounds; } camera;
} render = {{{{{0}}}, 0.1f, 100.0f}, {{0, 0}}};
static struct player_datum teammate = {0x10001};
static struct bitmap_data multiplayer_hud;
static long multiplayer_hud_index;
static int markers_drawn;

static struct player_datum *player_get(long index) { return &teammate; }
static void unit_get_head_position(long unit_index, real_point3d *position)
{
	position->x = 0.0f, position->y = 0.0f, position->z = -5.0f;
}
static void matrix4x3_transform_point(real_matrix4x3 const *matrix, real_point3d const *point, real_point3d *result)
{
	*result = *point;
}
static boolean render_camera_view_to_screen(void const *camera, void const *frustum, real_point3d const *view,
	real_point2d *screen)
{
	screen->x = 320.0f, screen->y = 240.0f;
	return TRUE;
}
static long interface_get_tag_index(short index) { return index == _interface_bitmap_multiplayer_hud ? multiplayer_hud_index : 0; }
static struct bitmap_data *bitmap_group_get_bitmap_from_sequence(long index, short sequence, short frame)
{
	return index == NONE ? NULL : &multiplayer_hud;
}
/* reads the bitmap's format, so NULL crashes here at address 0xe */
static long _texture_cache_bitmap_get_hardware_format(struct bitmap_data const *bitmap, boolean a, boolean b)
{
	CHECK(bitmap, "the marker's bitmap read through NULL");
	return 1;
}
static long fast_ftol(real value) { return (long)value; }
static void interface_draw_bitmap(struct bitmap_data const *bitmap, point2d const *position,
	real_rectangle2d const *clip, real scale, real rotation, real alpha)
{
	markers_drawn++;
}

#include "under_test.inc"

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("bitmap")
	{
		multiplayer_hud_index = 0x10001;
		hud_draw_friendly_indicator(0);
		CHECK(markers_drawn == 1, "%d markers", markers_drawn);
	}
	else CASE("no-bitmap")
	{
		multiplayer_hud_index = NONE;
		hud_draw_friendly_indicator(0);
		CHECK(markers_drawn == 0, "%d markers", markers_drawn);
	}
	else return 2;
	return 0;
}

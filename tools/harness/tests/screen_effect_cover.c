/*
SCREEN_EFFECT_COVER.C (test)

The real rasterizer_screen_effect_cover_target (rasterizer_xbox_screen_effect.c):
a screen effect pass's quad, once the shader translator has moved it half a game
pixel right and down (nv2a_vsh.c), covers the whole viewport it draws into, from
its left and top edges, and each place on it has the texture coordinate it had
before. Before, the quad started half a game pixel in, and on a screen drawn at
several pixels to the game's one the first pixels on the left and top were never
drawn: a scope's zoom warp spread their stale contents into a dark band. The
draw gives the quad's corners the texture coordinates 0 and the viewport's size,
less the shift the function returns; checked_quad does the same.
test_screen_effect_cover.py takes the target's number (config.inc) and the code
(under_test.inc).
*/

#include <math.h>
#include "harness.h"

typedef struct { real x0, x1, y0, y1; } real_rectangle2d;

#include "config.inc"

/* the screen's columns: a 16:9 screen's, or the Xbox's 640 */
static long screen_width = 854;
static long halo_screen_width(void) { return screen_width; }

#include "under_test.inc"

enum { SCREEN_HEIGHT = 480 };
enum { _rasterizer_target_render_primary = 0 };

/* the viewport a pass draws into, in the game's pixels */
struct viewport { real x, y, width, height; };

/* a vertex where the translator puts it, in the game's pixels: Direct3D's
viewport transform, then OpenGL's half pixel */
static real screen_x(real ndc, struct viewport const *viewport)
{
	return viewport->x + (ndc + 1.0f) * 0.5f * viewport->width + 0.5f;
}
static real screen_y(real ndc, struct viewport const *viewport)
{
	return viewport->y + (1.0f - ndc) * 0.5f * viewport->height + 0.5f;
}

/* a pass's quad from the stock bounds the game sets: it covers left to right
and top to bottom (the game's pixels), and the texture coordinate at each place
across it is the stock quad's there */
static void checked_quad(
	short destination,
	short viewport_width,
	short viewport_height,
	real_rectangle2d stock,
	struct viewport const *viewport,
	real left,
	real top,
	real right,
	real bottom)
{
	real_rectangle2d bounds = stock;
	real shift = rasterizer_screen_effect_cover_target(destination, viewport_width, viewport_height, &bounds);
	real x0 = screen_x(bounds.x0, viewport), x1 = screen_x(bounds.x1, viewport);
	real y0 = screen_y(bounds.y0, viewport), y1 = screen_y(bounds.y1, viewport);
	real stock_x0 = screen_x(stock.x0, viewport), stock_x1 = screen_x(stock.x1, viewport);
	real stock_y0 = screen_y(stock.y0, viewport), stock_y1 = screen_y(stock.y1, viewport);
	int step;

	CHECK(fabsf(x0 - left) < 0.001f && fabsf(x1 - right) < 0.001f,
		"the quad covers columns %g to %g, not %g to %g", x0, x1, left, right);
	CHECK(fabsf(y0 - top) < 0.001f && fabsf(y1 - bottom) < 0.001f,
		"the quad covers lines %g to %g, not %g to %g", y0, y1, top, bottom);
	for (step = 0; step <= 8; step++)
	{
		real x = left + (right - left) * (real)step / 8.0f;
		real y = top + (bottom - top) * (real)step / 8.0f;
		real u = -shift + (x - x0) / (x1 - x0) * (real)viewport_width;
		real v = -shift + (y - y0) / (y1 - y0) * (real)viewport_height;
		real stock_u = (x - stock_x0) / (stock_x1 - stock_x0) * (real)viewport_width;
		real stock_v = (y - stock_y0) / (stock_y1 - stock_y0) * (real)viewport_height;

		CHECK(fabsf(u - stock_u) < 0.001f, "column %g samples %g, not the stock quad's %g", x, u, stock_u);
		CHECK(fabsf(v - stock_v) < 0.001f, "line %g samples %g, not the stock quad's %g", y, v, stock_v);
	}
}

/* the stock bounds of a pass that covers its viewport (-1 to 1), and of a
split screen's first pass, its window in the whole surface (the game's) */
static real_rectangle2d whole_viewport(void)
{
	real_rectangle2d bounds;

	bounds.x0 = bounds.y1 = -1.0f;
	bounds.x1 = bounds.y0 = 1.0f;
	return bounds;
}
static real_rectangle2d window_in_surface(short x0, short y0, short x1, short y1)
{
	real_rectangle2d bounds;

	bounds.x0 = 2 * x0 * (1.0f / (real)halo_screen_width()) - 1.0f;
	bounds.x1 = 2 * x1 * (1.0f / (real)halo_screen_width()) - 1.0f;
	bounds.y0 = -2 * y0 * (1.0f / 480.0f) + 1.0f;
	bounds.y1 = -2 * y1 * (1.0f / 480.0f) + 1.0f;
	return bounds;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";

	/* one window on a 16:9 screen: the passes into the second surface and
	into the primary, and a single pass, cover the whole screen */
	CASE("covers-the-screen")
	{
		struct viewport screen = { 0.0f, 0.0f, 854.0f, 480.0f };

		checked_quad(_rasterizer_target_screen_effect, 854, 480, whole_viewport(), &screen, 0, 0, 854, 480);
		checked_quad(_rasterizer_target_render_primary, 854, 480, whole_viewport(), &screen, 0, 0, 854, 480);
		checked_quad(NONE, 854, 480, whole_viewport(), &screen, 0, 0, 854, 480);
		return 0;
	}
	/* the Xbox's 640x480: the same */
	CASE("covers-at-the-xbox-size")
	{
		struct viewport screen = { 0.0f, 0.0f, 640.0f, 480.0f };

		screen_width = 640;
		checked_quad(_rasterizer_target_screen_effect, 640, 480, whole_viewport(), &screen, 0, 0, 640, 480);
		checked_quad(_rasterizer_target_render_primary, 640, 480, whole_viewport(), &screen, 0, 0, 640, 480);
		return 0;
	}
	/* four players: the bottom right window's first pass draws its part of
	the whole second surface, the second pass the window's viewport of the
	primary */
	CASE("split-screen")
	{
		struct viewport surface = { 0.0f, 0.0f, 854.0f, 480.0f };
		struct viewport window = { 427.0f, 240.0f, 427.0f, 240.0f };

		checked_quad(_rasterizer_target_screen_effect, 427, 240, window_in_surface(427, 240, 854, 480), &surface,
			427, 240, 854, 480);
		checked_quad(_rasterizer_target_render_primary, 427, 240, whole_viewport(), &window, 427, 240, 854, 480);
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

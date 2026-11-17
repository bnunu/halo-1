#include "harness.h"

/* A script's screen effect with a blur (convolution), the video effect and light enhancement,
drawn in one window or in split screen. */
#define PIN(x, lo, hi) ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))
#define csmemcmp memcmp
#define _real_epsilon 0.0001f
#define TICKS_PER_SECOND 30
typedef struct { real red, green, blue; } real_rgb_color;
struct bitmap_data;
#include "declarations.inc"

static real_rgb_color const black = {0}, green = {0.0f, 1.0f, 0.0f};
static real_rgb_color const *global_real_rgb_black = &black, *global_real_rgb_green = &green;
static struct rasterizer_cinematic_screen_effect_state state;
static struct rasterizer_cinematic_screen_effect_state *cinematic_screen_effect_globals = &state;
static short window_count;

static long game_time_get(void) { return 300; }
static short main_get_window_count(void) { return window_count; }
static void scalars_interpolate(real a, real b, real fraction, real *result) { *result = a + (b - a) * fraction; }
static void scalars_interpolate_and_clamp_0_to_1(real a, real b, real fraction, real *result)
{
	scalars_interpolate(a, b, fraction, result);
	*result = PIN(*result, 0.0f, 1.0f);
}
/* the Xbox's fatal error, which ends the game */
static void display_assert(char const *message, char const *file, long line, boolean halt)
{
	CHECK(FALSE, "%s", message);
}
#define system_exit(code) ((void)0)

#include "under_test.inc"

static struct rasterizer_cinematic_screen_effect_parameters *blur(short windows)
{
	struct rasterizer_cinematic_screen_effect_parameters stock = {0};

	memset(&state, 0, sizeof(state));
	state.has_control = TRUE;
	state.convolution_radius[0] = state.convolution_radius[1] = 10.0f;
	state.filter_light_enhancement_intensity[0] = state.filter_light_enhancement_intensity[1] = 0.5f;
	state.parameters.convolution_type = 2;
	state.parameters.convolution_extra_passes = 3;
	state.parameters.video_on = TRUE;
	window_count = windows;
	return rasterizer_screen_effect_get_cinematic_parameters(&stock);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("one")
	{
		struct rasterizer_cinematic_screen_effect_parameters *parameters = blur(1);

		CHECK(parameters->convolution_radius == 10.0f && parameters->convolution_type == 2 &&
			parameters->convolution_extra_passes == 3 && parameters->video_on, "the blur or video effect left out");
		CHECK(parameters->filter_light_enhancement_intensity == 0.5f, "light enhancement %f",
			parameters->filter_light_enhancement_intensity);
	}
	else CASE("split")
	{
		struct rasterizer_cinematic_screen_effect_parameters *parameters = blur(2);

		CHECK(parameters->convolution_radius == 0.0f && parameters->convolution_type == 0 &&
			parameters->convolution_extra_passes == 0 && !parameters->video_on, "the blur or video effect in split screen");
		CHECK(parameters->filter_light_enhancement_intensity == 0.5f, "light enhancement %f",
			parameters->filter_light_enhancement_intensity);
		/* the next frame in one window has the effect's own parameters again */
		CHECK(state.parameters.convolution_radius == 10.0f && state.parameters.video_on, "the effect itself changed");
	}
	else return 2;
	return 0;
}

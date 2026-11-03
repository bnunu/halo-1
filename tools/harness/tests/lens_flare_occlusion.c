/*
LENS_FLARE_OCCLUSION.C (test)

The real lens flare occlusion bookkeeping of rasterizer_lights.c
(lens_flare_occlusion_test_index, lens_flare_occlusion_visible_pixels,
rasterizer_lights_begin_for_new_frame) over a fake GPU that reads each test's
count two frames after it, as the native builds do (d3d8_gl.c,
visibility_read_batch). Each flare's count must be its own: with the flares'
order in the frame's list changing (they come into view and go out of it), a
count read by the place in the list was another flare's, and a hidden one's
put a visible flare out (the Pelicans' thrusters blinking). A slot just taken
is not read until the flare's own counts are. test_lens_flare_occlusion.py
takes the enums, structures and code from the sources (config.inc,
under_test.inc).
*/

#include <math.h>
#include "harness.h"

typedef unsigned short word;
#define TICKS_PER_SECOND 30
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define PIN(n, low, high) ((n) < (low) ? (low) : ((n) > (high) ? (high) : (n)))

/* the limits of the arrays the flares' visibility is kept in */
enum
{
	MAXIMUM_WINDOWS = 4,
	MAXIMUM_LENS_FLARES_PER_FRAME = 1024,
	MAXIMUM_LIGHTS_PER_MAP = 64,
	MAXIMUM_LENS_FLARES_PER_LIGHT = 8,
	MAXIMUM_QUEUED_LENS_FLARES = 8,
	MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE = 65536,
};
struct lens_flare_definition;

#include "config.inc"

static struct lens_flare_occlusion_test_results local_lens_flare_occlusion_test_results[MAXIMUM_LIGHTS_PER_MAP];
static byte local_lens_flare_occlusion_test_results2[MAXIMUM_LENS_FLARE_MARKERS_PER_STRUCTURE+MAXIMUM_QUEUED_LENS_FLARES][MAXIMUM_WINDOWS];
static struct rasterizer_lens_flare_submit_parameters local_lens_flare_parameters[MAXIMUM_LENS_FLARES_PER_FRAME];
static long local_lens_flare_count;
static struct lens_flare_occlusion_slot lens_flare_occlusion_slots[LENS_FLARE_OCCLUSION_SLOT_COUNT];
static short local_lens_flare_occlusion_tests[MAXIMUM_LENS_FLARES_PER_FRAME];
static unsigned long lens_flare_occlusion_frame;

static struct { boolean draw_lens_flares; } rasterizer_debug_options = { TRUE };
#define rasterizer_profile_begin(profile) ((void)0)
#define rasterizer_profile_end(profile) ((void)0)
#define screenshot_in_progress() FALSE
/* a frame a tick (the fades move as they do at 30 frames a second) */
#define main_get_seconds_elapsed() (1.0f / 30.0f)

/* the fake GPU: a test's count is read two frames after the frame it was
made in, into its index's place, as the native builds read them */
enum { TEST_INDICES = 4096, LATENCY = 2, MAXIMUM_TESTS = 64 };
static long known[TEST_INDICES];
static struct { long index, count; } made[LATENCY + 1][MAXIMUM_TESTS];
static long made_count[LATENCY + 1];
static unsigned long frame;
static long rasterizer_widget_get_occlusion_test_result(long index) { return known[index % TEST_INDICES]; }

#include "under_test.inc"

/* the flares: a BSP marker each (their visibility in
local_lens_flare_occlusion_test_results2), and how many of their 100 pixels
show */
enum { FLARES = 6, PIXELS = 100 };
static long const shown[FLARES] = { 100, 0, 100, 0, 100, 0 };

static byte visibility_of(short flare)
{
	return local_lens_flare_occlusion_test_results2[flare + MAXIMUM_QUEUED_LENS_FLARES][0];
}

/* one frame: the start of it (last frame's counts used), then the flares in
this order queued and tested, then the GPU reads what it has */
static void run_frame(short const *order, short count)
{
	short index;
	short slot = (short)(frame % (LATENCY + 1));

	rasterizer_lights_begin_for_new_frame();
	made_count[slot] = 0;
	for (index = 0; index < count; index++)
	{
		struct rasterizer_lens_flare_submit_parameters *parameters = &local_lens_flare_parameters[index];
		short flare = order[index];

		memset(parameters, 0, sizeof(*parameters));
		/* (a BSP marker's, as rasterizer_lens_flare_submit has them) */
		parameters->light_identifier = NONE;
		parameters->light_index = (short)_lens_flare_dynamic_light_flag;
		parameters->lens_flare_index = (short)(flare + MAXIMUM_QUEUED_LENS_FLARES);
		parameters->compressed_window_index = 0;
		local_lens_flare_count = index + 1;
		local_lens_flare_occlusion_tests[index] = NONE;
		/* (as rasterizer_lens_flares_submit_occlusion_tests tests it) */
		local_lens_flare_occlusion_tests[index] = lens_flare_occlusion_test_index(index);
		parameters->internal__occlusion_pixels = PIXELS;
		CHECK(made_count[slot] < MAXIMUM_TESTS, "too many tests");
		made[slot][made_count[slot]].index = local_lens_flare_occlusion_tests[index];
		made[slot][made_count[slot]++].count = shown[flare];
	}
	/* (at the present: the frame's made LATENCY frames ago is read) */
	if (frame >= LATENCY)
	{
		short read = (short)((frame - LATENCY) % (LATENCY + 1));

		for (index = 0; index < made_count[read]; index++)
			known[made[read][index].index % TEST_INDICES] = made[read][index].count;
	}
	frame++;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	short index;

	/* the flares come into view and go out of it, in a different order each
	frame: the visible ones are never put out by a hidden one's count, and
	the hidden ones never shown */
	CASE("own-counts")
	{
		static short const orders[4][FLARES] = {
			{ 0, 1, 2, 3, 4, 5 },
			{ 1, 0, 3, 2, 5, 4 },
			{ 3, 4, 5, 0, 1, 2 },
			{ 5, 2, 1, 4, 0, 3 },
		};
		static short const counts[4] = { 6, 5, 6, 4 };
		long round;

		for (round = 0; round < 200; round++)
		{
			run_frame(orders[round % 4], counts[round % 4]);
			if (round < 20)
				continue;
			for (index = 0; index < FLARES; index++)
			{
				if (shown[index])
					CHECK(visibility_of(index) > 200, "visible flare %d at %d in frame %ld", index,
						visibility_of(index), round);
				else
					CHECK(visibility_of(index) == 0, "hidden flare %d shown at %d in frame %ld", index,
						visibility_of(index), round);
			}
		}
		return 0;
	}
	/* a flare keeps its slot from frame to frame, wherever it is in the list,
	and two flares of one frame never share one */
	CASE("stable-slots")
	{
		static short const first[3] = { 0, 1, 2 };
		static short const second[3] = { 2, 0, 1 };
		short slots[3];

		run_frame(first, 3);
		for (index = 0; index < 3; index++)
			slots[first[index]] = local_lens_flare_occlusion_tests[index];
		CHECK(slots[0] != slots[1] && slots[1] != slots[2] && slots[0] != slots[2], "two flares share a slot");
		run_frame(second, 3);
		for (index = 0; index < 3; index++)
			CHECK(local_lens_flare_occlusion_tests[index] == slots[second[index]],
				"flare %d moved from slot %d to %d", second[index], slots[second[index]],
				local_lens_flare_occlusion_tests[index]);
		return 0;
	}
	/* a slot just taken holds another flare's counts: it is not read for the
	frames until the new flare's own are */
	CASE("settles")
	{
		static short const one[1] = { 0 };
		short slot;
		unsigned long taken;

		run_frame(one, 1);
		slot = local_lens_flare_occlusion_tests[0];
		taken = lens_flare_occlusion_slots[slot].taken_frame;
		known[slot] = 12345;
		for (index = 0; index < LENS_FLARE_OCCLUSION_SLOT_SETTLE_FRAMES; index++)
		{
			lens_flare_occlusion_frame = taken + (unsigned long)index;
			CHECK(lens_flare_occlusion_visible_pixels(0) == NONE, "slot read %d frames after it was taken", index);
		}
		lens_flare_occlusion_frame = taken + LENS_FLARE_OCCLUSION_SLOT_SETTLE_FRAMES;
		CHECK(lens_flare_occlusion_visible_pixels(0) == 12345, "slot not read once settled");
		CHECK(LENS_FLARE_OCCLUSION_SLOT_SETTLE_FRAMES > LATENCY, "settles before the GPU's counts arrive");
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

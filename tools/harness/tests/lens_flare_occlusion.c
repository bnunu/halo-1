/*
LENS_FLARE_OCCLUSION.C (test)

The real lens flare occlusion bookkeeping of rasterizer_lights.c
(lens_flare_occlusion_test_index, lens_flare_occlusion_visible_pixels,
rasterizer_lights_begin_for_new_frame, rasterizer_lights_reset_for_new_map)
over a fake GPU that numbers each index's tests as they end and reads a
test's count into its index, with its number, frames after it, as the native
builds do (d3d8_gl.c, visibility_read_batch): later while it is behind, never
for a test it drops. Each flare's count must be its own: with the flares'
order in the frame's list changing (they come into view and go out of it), a
count read by the place in the list was another flare's, and a hidden one's
put a visible flare out (the Pelicans' thrusters blinking). A slot just taken
gives the last flare's counts until the new flare's own come, however late,
and is not read before; a flare that finds no slot keeps its visibility.
test_lens_flare_occlusion.py takes the enums, structures and code from the
sources (config.inc, under_test.inc).
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

static struct { boolean draw_lens_flares, lens_flare_occlusion_enabled; } rasterizer_debug_options = { TRUE, TRUE };
#define rasterizer_profile_begin(profile) ((void)0)
#define rasterizer_profile_end(profile) ((void)0)
#define screenshot_in_progress() FALSE
/* a frame a tick (the fades move as they do at 30 frames a second) */
#define main_get_seconds_elapsed() (1.0f / 30.0f)

/* the fake GPU: each index's tests numbered as they end; a test's count read
into its index, with its number, once it is latency frames old, the tests in
the order they were made (the oldest batch first), and none of the tests made
while dropping (every batch waiting to be read) */
enum { TEST_INDICES = 4096, MAXIMUM_PENDING = 8192 };
static long known[TEST_INDICES];
static unsigned long long ended[TEST_INDICES], counted[TEST_INDICES];
static struct { long index, count; unsigned long long number; unsigned long due; } pending[MAXIMUM_PENDING];
static long pending_count;
static unsigned long frame, latency = 2;
static boolean dropping;

/* (rasterizer_xbox_widgets.c: 1 with the occlusion tests off) */
static long rasterizer_widget_get_occlusion_test_result(long index)
{
	return rasterizer_debug_options.lens_flare_occlusion_enabled ? known[index % TEST_INDICES] : 1;
}
static unsigned long long xgpu_visibility_tests_ended(unsigned long index) { return ended[index % TEST_INDICES]; }
static unsigned long long xgpu_visibility_test_counted(unsigned long index) { return counted[index % TEST_INDICES]; }

static void gpu_test(long index, long count)
{
	unsigned long long number = ++ended[index % TEST_INDICES];

	if (dropping)
		return;
	CHECK(pending_count < MAXIMUM_PENDING, "too many tests pending");
	pending[pending_count].index = index % TEST_INDICES;
	pending[pending_count].count = count;
	pending[pending_count].number = number;
	pending[pending_count++].due = frame + latency;
}

/* the present: the counts of the tests old enough read */
static void gpu_present(void)
{
	long read = 0;

	while (read < pending_count && pending[read].due <= frame)
	{
		known[pending[read].index] = pending[read].count;
		counted[pending[read].index] = pending[read].number;
		read++;
	}
	memmove(pending, pending + read, (size_t)(pending_count - read) * sizeof(pending[0]));
	pending_count -= read;
}

#include "under_test.inc"

/* the flares: BSP markers (their visibility in
local_lens_flare_occlusion_test_results2), and how many of their PIXELS
pixels show in each window */
enum { FLARES = 4096, PIXELS = 100 };
static long shown[FLARES][MAXIMUM_WINDOWS];

static byte visibility_of(short flare, short window)
{
	return local_lens_flare_occlusion_test_results2[flare + MAXIMUM_QUEUED_LENS_FLARES][window];
}

/* the flare at place index in the frame's list, in a window, as
rasterizer_lens_flare_submit lists a BSP marker's */
static void listed(short index, short flare, short window)
{
	struct rasterizer_lens_flare_submit_parameters *parameters = &local_lens_flare_parameters[index];

	CHECK(flare >= 0 && flare < FLARES, "flare %d past the test's", flare);
	memset(parameters, 0, sizeof(*parameters));
	parameters->light_identifier = NONE;
	parameters->light_index = (short)_lens_flare_dynamic_light_flag;
	parameters->lens_flare_index = (short)(flare + MAXIMUM_QUEUED_LENS_FLARES);
	parameters->compressed_window_index = (byte)window;
	local_lens_flare_count = index + 1;
	local_lens_flare_occlusion_tests[index] = NONE;
}

/* every slot taken by another flare, tested this frame (crowd) */
static byte others[LENS_FLARE_OCCLUSION_SLOT_COUNT];
static boolean crowded;

static void crowd(void)
{
	short slot;

	for (slot = 0; slot < LENS_FLARE_OCCLUSION_SLOT_COUNT; slot++)
	{
		lens_flare_occlusion_slots[slot].visibility = &others[slot];
		lens_flare_occlusion_slots[slot].light_identifier = NONE;
		lens_flare_occlusion_slots[slot].tested_frame = lens_flare_occlusion_frame;
	}
}

/* one frame: the start of it (the counts read so far used), then the flares
in this order (each in its window, or the first) listed and tested as
rasterizer_lens_flares_submit_occlusion_tests and
rasterizer_widget_submit_occlusion_test test them (the test's area, PIXELS;
with the occlusion tests off, 1 and no test), then the present */
static void run_frame_in(short const *order, short const *windows, short count)
{
	short index;

	rasterizer_lights_begin_for_new_frame();
	if (crowded)
		crowd();
	for (index = 0; index < count; index++)
	{
		short flare = order[index];
		short window = windows ? windows[index] : 0;

		listed(index, flare, window);
		local_lens_flare_occlusion_tests[index] = lens_flare_occlusion_test_index(index);
		if (rasterizer_debug_options.lens_flare_occlusion_enabled)
		{
			local_lens_flare_parameters[index].internal__occlusion_pixels = PIXELS;
			gpu_test(local_lens_flare_occlusion_tests[index], shown[flare][window]);
		}
		else
		{
			local_lens_flare_parameters[index].internal__occlusion_pixels = 1;
		}
	}
	gpu_present();
	frame++;
}

static void run_frame(short const *order, short count)
{
	run_frame_in(order, NULL, count);
}

/* two flares that take the same slot of an empty table (the first they look
at), found by trying them: the slots are a hash table of 2048 */
static void sharing_flares(short pair[2])
{
	static short first_slot[FLARES];
	short flare, other;

	for (flare = 0; flare < FLARES; flare++)
	{
		listed(0, flare, 0);
		first_slot[flare] = lens_flare_occlusion_test_index(0);
		memset(&lens_flare_occlusion_slots[first_slot[flare]], 0, sizeof(lens_flare_occlusion_slots[0]));
		for (other = 0; other < flare; other++)
		{
			if (first_slot[other] == first_slot[flare])
			{
				pair[0] = other;
				pair[1] = flare;
				return;
			}
		}
	}
	CHECK(FALSE, "no two flares share a slot");
}

/* own-counts' flares, and the order they are listed in from frame to frame */
static long const own_shown[6] = { PIXELS, 0, PIXELS, 0, PIXELS, 0 };
static short const orders[4][6] = {
	{ 0, 1, 2, 3, 4, 5 },
	{ 1, 0, 3, 2, 5, 4 },
	{ 3, 4, 5, 0, 1, 2 },
	{ 5, 2, 1, 4, 0, 3 },
};
static short const counts[4] = { 6, 5, 6, 4 };

static void own_flares(void)
{
	short flare;

	for (flare = 0; flare < 6; flare++)
		shown[flare][0] = own_shown[flare];
}

/* the visible flares lit, the hidden ones dark */
static void own_flares_checked(const char *when, long round)
{
	short flare;

	for (flare = 0; flare < 6; flare++)
	{
		if (own_shown[flare])
			CHECK(visibility_of(flare, 0) > 200, "visible flare %d at %d in frame %ld %s", flare,
				visibility_of(flare, 0), round, when);
		else
			CHECK(visibility_of(flare, 0) == 0, "hidden flare %d shown at %d in frame %ld %s", flare,
				visibility_of(flare, 0), round, when);
	}
}

/* a slot reused: a visible flare's, then, once that flare has been out of
view long enough to give it up, a hidden flare's, whose counts come late
frames after its tests, none for the first dropped frames' (the GPU behind,
every batch waiting). The slot gives the visible flare's count until the
hidden one's own come: the hidden flare is never shown, and its own count is
read once it has come. */
static void reused(unsigned long late, long dropped)
{
	short pair[2];
	short slot;
	long round;

	sharing_flares(pair);
	shown[pair[0]][0] = PIXELS;
	shown[pair[1]][0] = 0;
	for (round = 0; round < 20; round++)
		run_frame(&pair[0], 1);
	slot = local_lens_flare_occlusion_tests[0];
	CHECK(visibility_of(pair[0], 0) > 200, "the visible flare at %d", visibility_of(pair[0], 0));
	for (round = 0; round <= LENS_FLARE_OCCLUSION_SLOT_KEPT_FRAMES + 1; round++)
		run_frame(NULL, 0);
	CHECK(known[slot] == PIXELS, "the slot gives %ld, not the visible flare's count", known[slot]);
	latency = late;
	for (round = 0; round < 60; round++)
	{
		dropping = round < dropped;
		run_frame(&pair[1], 1);
		if (!round)
			CHECK(local_lens_flare_occlusion_tests[0] == slot,
				"the hidden flare took slot %d, not the visible one's (%d): no slot reused",
				local_lens_flare_occlusion_tests[0], slot);
		CHECK(visibility_of(pair[1], 0) == 0,
			"the hidden flare shown at %d %ld frames after it took the visible one's slot",
			visibility_of(pair[1], 0), round);
	}
	CHECK(lens_flare_occlusion_visible_pixels(0) == 0, "the hidden flare's own count not read (%ld)",
		lens_flare_occlusion_visible_pixels(0));
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	short index;
	long round;

	/* the flares come into view and go out of it, in a different order each
	frame: the visible ones are never put out by a hidden one's count, and
	the hidden ones never shown */
	CASE("own-counts")
	{
		own_flares();
		for (round = 0; round < 200; round++)
		{
			run_frame(orders[round % 4], counts[round % 4]);
			if (round >= 20)
				own_flares_checked("", round);
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
	/* a slot reused while the GPU is 12 frames behind */
	CASE("reused-late")
	{
		reused(12, 0);
		return 0;
	}
	/* a slot reused while the GPU drops the new flare's first 20 tests */
	CASE("reused-dropped")
	{
		reused(2, 20);
		return 0;
	}
	/* own-counts' flares out of view while every slot is taken by flares
	tested since, then back in view, in the changing orders: none finds a
	slot, and each keeps the visibility it had, read no other flare's count */
	CASE("no-slot")
	{
		own_flares();
		for (round = 0; round < 20; round++)
			run_frame(orders[round % 4], counts[round % 4]);
		own_flares_checked("before the slots were all taken", round);
		crowded = TRUE;
		for (round = 0; round <= LENS_FLARE_OCCLUSION_SLOT_KEPT_FRAMES + 1; round++)
			run_frame(NULL, 0);
		for (round = 0; round < 100; round++)
		{
			run_frame(orders[round % 4], counts[round % 4]);
			own_flares_checked("with no slot", round);
		}
		for (index = 0; index < counts[(round - 1) % 4]; index++)
			CHECK(local_lens_flare_occlusion_tests[index] == LENS_FLARE_OCCLUSION_UNREAD_INDEX,
				"flare %d tested at %d with every slot taken", orders[(round - 1) % 4][index],
				local_lens_flare_occlusion_tests[index]);
		return 0;
	}
	/* split screen: the same two flares in two windows, the first shown in
	the first window and hidden in the second, the second the other way, the
	windows listed in turn: each window's flare has its own visibility */
	CASE("split-screen")
	{
		static short const flares[2][4] = { { 0, 1, 0, 1 }, { 1, 0, 1, 0 } };
		static short const windows[2][4] = { { 0, 0, 1, 1 }, { 1, 1, 0, 0 } };

		shown[0][0] = PIXELS;
		shown[1][1] = PIXELS;
		for (round = 0; round < 100; round++)
		{
			run_frame_in(flares[round % 2], windows[round % 2], 4);
			if (round < 20)
				continue;
			CHECK(visibility_of(0, 0) > 200 && visibility_of(1, 1) > 200,
				"a flare shown in its window at %d and %d in frame %ld", visibility_of(0, 0), visibility_of(1, 1),
				round);
			CHECK(visibility_of(0, 1) == 0 && visibility_of(1, 0) == 0,
				"a flare hidden in its window shown at %d and %d in frame %ld", visibility_of(0, 1),
				visibility_of(1, 0), round);
		}
		return 0;
	}
	/* the map changes while the GPU is behind, the old map's last counts to
	come: the new map's flare at the same marker, hidden, takes the same slot
	and is never shown by the old flare's count */
	CASE("new-map")
	{
		static short const one[1] = { 0 };

		shown[0][0] = PIXELS;
		for (round = 0; round < 20; round++)
			run_frame(one, 1);
		CHECK(visibility_of(0, 0) > 200, "the old map's flare at %d", visibility_of(0, 0));
		latency = 6;
		run_frame(one, 1);
		rasterizer_lights_reset_for_new_map();
		shown[0][0] = 0;
		for (round = 0; round < 30; round++)
		{
			run_frame(one, 1);
			CHECK(visibility_of(0, 0) == 0, "the new map's hidden flare shown at %d %ld frames after the change",
				visibility_of(0, 0), round);
		}
		CHECK(lens_flare_occlusion_visible_pixels(0) == 0, "the new map's flare's own count not read (%ld)",
			lens_flare_occlusion_visible_pixels(0));
		return 0;
	}
	/* with the occlusion tests off (rasterizer_lens_flares_occlusion), no
	test is made and every flare's result is 1 of 1: all shown */
	CASE("occlusion-off")
	{
		rasterizer_debug_options.lens_flare_occlusion_enabled = FALSE;
		for (round = 0; round < 40; round++)
			run_frame(orders[round % 4], counts[round % 4]);
		for (index = 0; index < 6; index++)
			CHECK(visibility_of(index, 0) > 200, "flare %d at %d with the occlusion tests off", index,
				visibility_of(index, 0));
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

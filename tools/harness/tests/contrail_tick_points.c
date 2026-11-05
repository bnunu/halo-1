/*
CONTRAIL_TICK_POINTS.C (test)

The real contrails_update (contrails.c), with contrail_add_points,
contrail_update_points and contrail_compute_new_point_count, over a projectile
whose marker moves a tick at a time, with a Covenant plasma turret's contrail
(vehicles\c gun turret\c gun turret: a point a tick, states of 0.01, 0.05 and
0.005 seconds, both ends drawn faded): the native builds update contrails
every frame, and on the first frame after a tick its points are made and aged
by a tick's time, so that the trail is the same each tick at any frame rate,
as the Xbox drew it a frame a tick. Made and aged by the frames' time since
the last tick (four or five frames' at 144 frames a second), the trail's
fades changed from tick to tick, and a far turret's trail flickered.
test_contrail_tick_points.py takes the constants, structures and code from
the sources (config.inc, under_test.inc).
*/

#include <math.h>
#include "harness.h"

typedef unsigned short word;
typedef struct { real alpha, red, green, blue; } real_argb_color;
typedef struct { real_vector3d forward, left, up; real_point3d position; } real_matrix4x3;
struct tag_reference { unsigned long group_tag; char const *name; long name_length; long index; };
struct tag_block { long count; void *address; void const *definition; };
struct shader { long data[10]; };
struct location { long cluster_reference; short leaf_index; short cluster_index; };
#define TAG_BLOCK_GET_ELEMENT(block, element, type) (&((type *)(block)->address)[(long)(element)])
#define SET_FLAG(flags, bit, value) ((value) ? ((flags) |= FLAG(bit)) : ((flags) &= ~FLAG(bit)))
#define NUMBEROF(a) (sizeof(a) / sizeof((a)[0]))
#define TICKS_PER_SECOND 30
#define csmemcmp memcmp
#define match_assert_valid_real(file, line, value) ((void)0)

#include "config.inc"

/* the game's time, in ticks */
static long game_time;
static long game_time_get(void) { return game_time; }

/* the players at their normal speed */
static boolean game_players_are_double_speed(void) { return FALSE; }

/* the local random numbers (all the turret's ranges are a single value) */
static real real_local_random_range(real lower, real upper) { return lower; }
static short local_random_range(short lower, short upper) { return lower; }
static void local_random_vector_in_cone3d(real_vector3d const *forward, real minimum, real maximum, real_vector3d *out)
{
	*out = *forward;
}

/* the projectile: its contrail's marker, where the tick left it */
struct object_attachment_definition { char marker_name[32]; };
struct object_definition { struct { struct tag_block attachments; } object; };
struct object_datum { long definition_index; };
struct object_marker { short node_index; real_matrix4x3 node_matrix; real_matrix4x3 matrix; };
static struct object_attachment_definition attachment = { "contrail" };
static struct object_definition projectile_definition = { { { 1, &attachment, NULL } } };
static struct object_datum projectile;
static real_point3d projectile_position;
static struct object_datum *object_get(long index) { return &projectile; }
static struct object_definition *object_definition_get(long index) { return &projectile_definition; }
static boolean object_get_function_value(long object_index, short function_index, real *value)
{
	*value = 1.0f;
	return TRUE;
}
static short object_get_marker_by_name(long object_index, char const *name, struct object_marker *markers, short count)
{
	memset(markers, 0, sizeof(*markers));
	markers->matrix.forward.i = 1.0f;
	markers->matrix.position = projectile_position;
	return 1;
}
static void object_get_velocities(long object_index, real_vector3d *linear, real_vector3d *angular)
{
	memset(linear, 0, sizeof(*linear));
}
static void scenario_location_from_point(struct location *location, real_point3d const *point) { }
static void scenario_location_from_line(struct location *location, struct location const *from,
	real_point3d const *a, real_point3d const *b) { }
static void *point_physics_definition_get(long index) { return NULL; }
static void point_physics_update(long a, void *definition, struct location *location, long b, real_point3d *position,
	real_vector3d *velocity, void *c, void *d, void *e, real radius, real dt) { }

/* the contrail's bitmap (its sequences only: the turret's does not animate) */
struct bitmap_group_sequence { char name[32]; short first_bitmap_index; short bitmap_count; };
struct bitmap_group { struct tag_block sequences; };
static struct bitmap_group_sequence sequence = { "", 0, 1 };
static struct bitmap_group contrail_bitmaps = { { 1, &sequence, NULL } };
static struct bitmap_group *bitmap_group_get(long index) { return &contrail_bitmaps; }

/* the turret's contrail */
static struct contrail_point_state states[3];
static struct contrail_definition turret;
#define contrail_definition_get(index) (&turret)

/* the data */
struct data_array *contrail_data;
struct data_array *contrail_point_data;
static long data_next_index(struct data_array *array, long index)
{
	long absolute = index == NONE ? 0 : DATUM_INDEX_TO_ABSOLUTE_INDEX(index) + 1;

	for (; absolute < array->maximum_count; absolute++)
	{
		struct datum_header *header = (struct datum_header *)(array->data + absolute * array->size);

		if (header->identifier)
			return ((long)(unsigned short)header->identifier << 16) | absolute;
	}
	return NONE;
}
#define contrail_get(index) ((struct contrail_datum *)datum_get(contrail_data, (index)))
#define contrail_point_get(index) ((struct contrail_point_datum *)datum_get(contrail_point_data, (index)))
static void contrail_verify(long contrail_index) { }
static void contrail_delete(long contrail_index) { datum_delete(contrail_data, contrail_index); }

#include "under_test.inc"

static long contrail_index;

static void set_up(void)
{
	static real const durations[3] = { 0.010f, 0.000f, 0.000f };
	static real const transitions[3] = { 0.050f, 0.005f, 0.000f };
	short index;
	struct contrail_datum *contrail;

	for (index = 0; index < 3; index++)
	{
		memset(&states[index], 0, sizeof(states[index]));
		states[index].duration_lower_bound = states[index].duration_upper_bound = durations[index];
		states[index].transition_duration_lower_bound = states[index].transition_duration_upper_bound =
			transitions[index];
		states[index].physics.index = NONE;
		states[index].width = index ? 0.0f : 0.15f;
		states[index].color_lower_bound.alpha = states[index].color_upper_bound.alpha = index ? 0.0f : 1.0f;
	}
	memset(&turret, 0, sizeof(turret));
	turret.point_generation_rate = 30.0f;
	turret.texture_animation_u = 3.0f;
	turret.states.count = 3;
	turret.states.address = states;
	contrail_data = game_state_data_new("contrail", 16, sizeof(struct contrail_datum));
	contrail_point_data = game_state_data_new("contrail point", MAXIMUM_CONTRAIL_POINTS,
		sizeof(struct contrail_point_datum));
	contrail_index = datum_new(contrail_data);
	contrail = datum_get(contrail_data, contrail_index);
	contrail->definition_index = 0;
	contrail->object_index = 1;
	for (index = 0; index < 4; index++)
		contrail->first_contrail_point_indices[index] = NONE;
}

/* what of the trail is drawn (render_contrails.c): its points' width by
their alpha, each of a state or of the part of its transition gone, the
first and the last point drawn faded */
static real shown(void)
{
	struct contrail_datum *contrail = datum_get(contrail_data, contrail_index);
	long point_index = contrail->first_contrail_point_indices[0];
	long count = contrail->contrail_point_counts[0];
	long position = 0;
	real total = 0.0f;

	for (; point_index != NONE; position++)
	{
		struct contrail_point_datum *point = datum_get(contrail_point_data, point_index);
		struct contrail_point_state *state = &states[(long)point->state_index];
		real width = state->width;
		real alpha = state->color_lower_bound.alpha;

		if ((point->flags & FLAG(_contrail_point_transitioning_bit)) && point->state_index + 1 < 3)
		{
			width += (states[point->state_index + 1].width - width) * point->time;
			alpha += (states[point->state_index + 1].color_lower_bound.alpha - alpha) * point->time;
		}
		if (position > 0 && position < count - 1)
			total += width * alpha;
		point_index = point->next_contrail_point_index;
	}
	return total;
}

/* frames of the given times; a tick runs whenever the time passes the next
30th of a second, moving the projectile a world unit. What is shown after
each tick's first frame is kept in shown_each_tick. */
enum { MAXIMUM_TICKS = 256 };
static real shown_each_tick[MAXIMUM_TICKS];
static long tick_count;
static double clock_time;

static void frame(real dt)
{
	long ticks_due;

	clock_time += dt;
	ticks_due = (long)floor(clock_time * TICKS_PER_SECOND + 1e-6);
	if (ticks_due > game_time)
	{
		game_time = ticks_due;
		projectile_position.x += 1.0f;
	}
	contrails_update(dt);
	if (tick_count < game_time && tick_count < MAXIMUM_TICKS)
		shown_each_tick[tick_count++] = shown();
}

/* the same from tick 10 on (the trail made), and as a frame a tick shows it */
static void same_each_tick(real expected, const char *what)
{
	long tick;

	for (tick = 10; tick < tick_count; tick++)
		CHECK(fabsf(shown_each_tick[tick] - expected) < 1e-4f,
			"%s: tick %ld shows %g of the trail, not %g as a frame a tick", what, tick, shown_each_tick[tick], expected);
}

/* a frame a tick, as the Xbox drew it: what each tick shows */
static real a_frame_a_tick(void)
{
	long tick;
	real value;

	set_up();
	for (tick = 0; tick < 40; tick++)
		frame(1.0f / TICKS_PER_SECOND);
	value = shown_each_tick[tick_count - 1];
	CHECK(value > 0.0f, "a frame a tick shows none of the trail");
	same_each_tick(value, "a frame a tick");
	tick_count = 0;
	clock_time = 0.0;
	game_time = 0;
	projectile_position.x = 0.0f;
	return value;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	real expected = a_frame_a_tick();
	long index;

	set_up();
	/* 144 frames a second: four or five frames a tick */
	CASE("steady-at-144")
	{
		for (index = 0; index < 144 * 3; index++)
			frame(1.0f / 144.0f);
		same_each_tick(expected, "at 144 frames a second");
		return 0;
	}
	/* uneven frames (5, 9 and 3 milliseconds), as a frame's time is */
	CASE("uneven-frames")
	{
		static real const times[3] = { 0.005f, 0.009f, 0.003f };

		for (index = 0; index < 600; index++)
			frame(times[index % 3]);
		same_each_tick(expected, "with uneven frames");
		return 0;
	}
	/* the game's time gone back (a checkpoint, a new map) or jumping ahead:
	a tick's time, the trail as it was */
	CASE("time-jumps")
	{
		for (index = 0; index < 144 * 4; index++)
			frame(1.0f / 144.0f);
		game_time -= 50;
		clock_time -= 50.0 / TICKS_PER_SECOND;
		for (index = 0; index < 144 * 3; index++)
			frame(1.0f / 144.0f);
		game_time += 500;
		clock_time += 500.0 / TICKS_PER_SECOND;
		for (index = 0; index < 144; index++)
			frame(1.0f / 144.0f);
		CHECK(tick_count > 170, "the trail was not looked at after the jumps (%ld ticks)", tick_count);
		same_each_tick(expected, "with the game's time gone back and jumping ahead");
		return 0;
	}
	fprintf(stderr, "unknown case '%s'\n", case_name);
	return 2;
}

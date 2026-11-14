#include "harness.h"
#include <math.h>

#define MAXIMUM_NUMBER_OF_LOCAL_PLAYERS 4
#define TICKS_PER_SECOND 30
typedef unsigned short word;
static long now, owner = 42;
static real_vector3d zero;
static real_vector3d *global_zero_vector3d = &zero;
static long game_time_get(void) { return now; }
static long local_player_get_player_index(short player) { return owner + player; }
static void vector_from_points3d(const real_point3d *a, const real_point3d *b, real_vector3d *v)
{ v->i = b->x - a->x; v->j = b->y - a->y; v->k = b->z - a->z; }
static real magnitude3d(const real_vector3d *v) { return sqrtf(v->i*v->i + v->j*v->j + v->k*v->k); }
static void scale_vector3d(const real_vector3d *a, real s, real_vector3d *b)
{ b->i = a->i*s; b->j = a->j*s; b->k = a->k*s; }
static void cross_product3d(const real_vector3d *a, const real_vector3d *b, real_vector3d *v)
{ v->i = a->j*b->k - a->k*b->j; v->j = a->k*b->i - a->i*b->k; v->k = a->i*b->j - a->j*b->i; }
static void add_vectors3d(const real_vector3d *a, const real_vector3d *b, real_vector3d *v)
{ v->i = a->i + b->i; v->j = a->j + b->j; v->k = a->k + b->k; }
struct object_datum
{
	struct { real_point3d position; real_vector3d angular_velocity, translational_velocity; } object;
};
static struct object_datum parent;
static long object_get_ultimate_parent(long child) { return 7; }
static struct object_datum *object_get(long index)
{ CHECK(index == 7, "attachment did not use ultimate parent"); return &parent; }

typedef void (*director_camera_update_proc)(void);
static void first_person_camera_update(void) {}
static void following_camera_update(void) {}
static void dead_camera_update(void) {}
static void flying_camera_update(void) {}
struct dead_camera { long unit_index; };
struct director
{
	director_camera_update_proc camera_proc;
	void *camera_data;
	real debug_input_scale, camera_change_pause;
	boolean debug_controls;
};
static struct director directors[4];
static boolean scripted;
static boolean *director_camera_scripted = &scripted;
static struct director *director_get(short player) { return &directors[player]; }
static long player_control_get_unit_index(short player) { return 100 + player; }
static void director_update(real dt) {}
static void observer_update(real dt) {}
#define _camera_script_mode_point 1
static struct
{
	boolean enabled, first_update;
	long mode, camera_point_index, relative_object_index;
	real_point3d point;
	real_vector3d forward, up;
	real field_of_view, timer;
} camera_script_globals;

struct scenario_cutscene_camera_point
{
	real_point3d position;
	real_vector3d orientation;
	real field_of_view;
};
static struct scenario_cutscene_camera_point test_camera_point;
#define TAG_BLOCK_GET_ELEMENT(block, index, type) (&test_camera_point)
static void vectors3d_from_euler_angles3d(real_vector3d *forward, real_vector3d *up,
	const real_vector3d *orientation) { *forward = zero; *up = zero; }
/* the cuts the scripts make, as the frames' interpolation hears them */
static int interpolation_cuts;
static void render_interpolation_cut(void) { interpolation_cuts++; }

#include "under_test.inc"

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";
	real_point3d p = { 0, 0, 0 };
	real_vector3d v;
	short player;

	now = 100;
	listener_measure_velocity(0, &p, &v);
	CHECK(v.i == 0, "first sample has velocity");
	now++;
	p.x = 1;
	listener_measure_velocity(0, &p, &v);
	CHECK(v.i == 30, "one unit per tick is 30 units per second");
	CASE("motion")
	{
		p.x = 1.5f;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 30, "render frames changed tick velocity");
		now++;
		p.x = 2;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 30, "render frames changed next tick velocity");
		now++;
		p.x = 100;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "teleport produced velocity");
		return 0;
	}
	CASE("coop-camera-motion")
	{
		unsigned tick;

		/* Co-op applies the host camera with no interpolation every tick.
		Zero transition time here describes an update, not a camera cut. */
		for (tick = 0; tick < 6; tick++)
		{
			now++;
			p.x += 1;
			scripted_camera_set_camera_point_relative(&p, &zero, &zero, 1, 0, NONE);
			listener_measure_velocity(0, &camera_script_globals.point, &v);
			CHECK(v.i == 30, "co-op camera update discarded listener motion");
			/* Reapplying the same snapshot within a tick also preserves it. */
			scripted_camera_set_camera_point_relative(&p, &zero, &zero, 1, 0, NONE);
			listener_measure_velocity(0, &camera_script_globals.point, &v);
			CHECK(v.i == 30, "same-tick co-op camera update discarded motion");
		}
		return 0;
	}
	CASE("cuts")
	{
		/* A five-unit instant cut, including multiple changes in one tick. */
		p.x += 5;
		test_camera_point.position = p;
		scripted_camera_set(0, 0, NONE);
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "short same-tick cut retained velocity");
		now++;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "cut became velocity on the next tick");
		p.x += 5;
		scripted_camera_enable(TRUE);
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "script camera selection retained motion");
		now++;
		p.x += 1;
		test_camera_point.position = p;
		scripted_camera_set(0, 30, NONE);
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 30, "smooth transition was incorrectly treated as a cut");
		director_set_camera(0, flying_camera_update, FALSE);
		p.x += 5;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "director selection retained motion");
		return 0;
	}
	/* a cut to another point, or out of another mode, is a new shot to the
	frames' interpolation; the same point set again, or a point moved to over
	time, is not */
	CASE("interpolation-cuts")
	{
		camera_script_globals.mode = 0;
		camera_script_globals.camera_point_index = -1;
		camera_script_globals.relative_object_index = -1;
		scripted_camera_set(3, 0, -1);
		CHECK(interpolation_cuts == 1, "a cut out of another mode: %d", interpolation_cuts);
		scripted_camera_set(3, 0, -1);
		CHECK(interpolation_cuts == 1, "the same point set again: %d", interpolation_cuts);
		scripted_camera_set(4, 0, -1);
		CHECK(interpolation_cuts == 2, "a cut to another point: %d", interpolation_cuts);
		scripted_camera_set(4, 0, 9);
		CHECK(interpolation_cuts == 3, "a cut to the point relative to an object: %d", interpolation_cuts);
		scripted_camera_set(5, 120, 9);
		CHECK(interpolation_cuts == 3, "a move over four seconds: %d", interpolation_cuts);
		scripted_camera_enable(FALSE);
		CHECK(interpolation_cuts == 4, "the scripted camera turned off: %d", interpolation_cuts);
		p.x += 1;
		scripted_camera_set_camera_point_relative(&p, &zero, &zero, 1, 0, -1);
		CHECK(interpolation_cuts == 4, "co-op's camera each tick: %d", interpolation_cuts);
		return 0;
	}
	CASE("lifecycle")
	{
		for (player = 0; player < 4; player++)
			listener_measure_velocity(player, &p, &v);
		sound_initialize_for_new_map();
		p.x = 6;
		for (player = 0; player < 4; player++)
		{
			listener_measure_velocity(player, &p, &v);
			CHECK(v.i == 0, "map reset leaked same-tick motion into player %d", player);
		}
		owner++;
		now++;
		p.x++;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "reused player slot retained motion");
		now = 50;
		p.x++;
		listener_measure_velocity(0, &p, &v);
		CHECK(v.i == 0, "backwards ticks retained motion");
		return 0;
	}
	CASE("followed-unit")
	{
		struct dead_camera dead = { 77 };
		director_camera_update_proc cameras[] = { first_person_camera_update, following_camera_update,
			dead_camera_update, flying_camera_update };
		long expected[] = { 100, 100, 77, NONE };
		for (player = 0; player < 4; player++)
		{
			directors[0].camera_proc = cameras[player];
			directors[0].camera_data = &dead;
			CHECK(director_get_followed_unit(0) == expected[player], "wrong followed unit for camera %d", player);
		}
		scripted = TRUE;
		directors[0].camera_proc = first_person_camera_update;
		CHECK(director_get_followed_unit(0) == NONE, "scripted path fell through to player unit");
		return 0;
	}
	CASE("rigid-body")
	{
		real_point3d source = { 2, 1, 0 }, ear = { -3, 2, 1 };
		real_vector3d source_velocity, ear_velocity, line;
		real radial;

		parent.object.translational_velocity.i = 2;
		parent.object.angular_velocity.k = 0.5f;
		object_get_point_velocity(5, &source, &source_velocity);
		object_get_point_velocity(6, &ear, &ear_velocity);
		CHECK(source_velocity.i == 1.5f && source_velocity.j == 1, "source ignored angular velocity");
		CHECK(ear_velocity.i == 1 && ear_velocity.j == -1.5f, "listener ignored angular velocity");
		vector_from_points3d(&source, &ear, &line);
		radial = (source_velocity.i - ear_velocity.i)*line.i +
			(source_velocity.j - ear_velocity.j)*line.j + (source_velocity.k - ear_velocity.k)*line.k;
		CHECK(fabsf(radial) < 1.0e-6f, "same vehicle's source and listener have relative radial motion");
		return 0;
	}
	return 2;
}

/*
RANDOM_MATH.C

symbols in this file:
000FA8B0 0010:
	_lock_global_random_seed (0000)
000FA8C0 0030:
	_unlock_global_random_seed (0000)
000FA8F0 0040:
	_get_global_random_seed_address (0000)
000FA930 0010:
	_get_random_seed (0000)
000FA940 0010:
	_get_global_local_random_seed_address (0000)
000FA950 0010:
	_random_seed_debug_log (0000)
000FA960 0020:
	_get_number_suitable_for_initializing_random_seed (0000)
000FA980 00c0:
	_random_math_initialize (0000)
000FAA40 0020:
	_random_math_dispose (0000)
000FAA60 0030:
	_real_seed_random (0000)
000FAA90 0040:
	_real_seed_random_range (0000)
000FAAD0 0020:
	_seed_random (0000)
000FAAF0 0030:
	_seed_random_range (0000)
000FAB20 0080:
	_direction3d_from_table (0000)
000FABA0 0040:
	_seed_random_direction3d (0000)
000FABE0 0100:
	_seed_random_orientation (0000)
000FACE0 0100:
	_seed_random_vector_in_cone3d (0000)
0027AE88 0031:
	??_C@_0DB@GOKAHAFJ@unmatched?5call?5to?5unlock_random_@ (0000)
0027AEBC 0022:
	??_C@_0CC@CGOEMOHM@c?3?2halo?2SOURCE?2math?2random_math?4@ (0000)
0027AEE0 0044:
	??_C@_0EE@KKMKHIJL@you?5should?5not?5be?5using?5global?5r@ (0000)
0027AF24 001b:
	??_C@_0BL@CJPMPDBB@random_direction_geosphere?$AA@ (0000)
0027AF40 0042:
	??_C@_0EC@LPLKDAPN@index?$DO?$DN0?5?$CG?$CG?5index?$DMrandom_math_gl@ (0000)
0027AF84 002b:
	??_C@_0CL@KJMLMACI@random_math_globals?4random_direc@ (0000)
00456208 0014:
	_random_math_globals (0000)
*/

/* ---------- headers */

#include "cseries.h"
#include "cseries_windows.h"
#include "real_math.h"
#include "geometry.h"
#include "game_engine.h"

/* ---------- constants */

enum
{
	RANDOM_DIRECTION_TABLE_GEOSPHERE_SEGMENT_COUNT= 16,
	RANDOM_A= 1664525L,
	RANDOM_C= 1013904223L
};

/* ---------- macros */

/* ---------- structures */

struct random_math_globals
{
	real_vector3d *random_direction_table;
	short random_direction_table_size;
	short pad;
	long global_random_seed_lock;
	unsigned long global_random_seed;
	unsigned long global_local_random_seed;
};

/* ---------- prototypes */

static real_vector3d *
direction3d_from_table(
	real_vector3d *result,
	short index);

/* ---------- globals */

static struct random_math_globals random_math_globals;

/* ---------- public code */

void
lock_global_random_seed(
	void)
{
	random_math_globals.global_random_seed_lock++;
	return;
}

void
unlock_global_random_seed(
	void)
{
	match_dassert(
		"c:\\halo\\SOURCE\\math\\random_math.c",
		41,
		random_math_globals.global_random_seed_lock>0,
		"unmatched call to unlock_random_seed() somewhere");
	random_math_globals.global_random_seed_lock--;
	return;
}

unsigned long get_random_seed(
	void)
{
	return random_math_globals.global_random_seed;
}

unsigned long *get_global_random_seed_address(
	void)
{
	match_dassert(
		"c:\\halo\\SOURCE\\math\\random_math.c",
		56,
		!game_engine_running() || !random_math_globals.global_random_seed_lock,
		"you should not be using global random(); use local random() instead");
	return &random_math_globals.global_random_seed;
}

unsigned long *get_global_local_random_seed_address(
	void)
{
	return &random_math_globals.global_local_random_seed;
}

void
random_seed_debug_log(
	boolean log)
{
	return;
}

unsigned long get_number_suitable_for_initializing_random_seed(
	void)
{
	return system_seconds()^system_milliseconds()^rand();
}

void
random_math_initialize(
	void)
{
	struct geosphere *random_direction_geosphere;
	short index;

	random_math_globals.global_local_random_seed= get_number_suitable_for_initializing_random_seed();
	random_direction_geosphere= geosphere_new(RANDOM_DIRECTION_TABLE_GEOSPHERE_SEGMENT_COUNT);
	match_assert("c:\\halo\\SOURCE\\math\\random_math.c", 174, random_direction_geosphere);
	random_math_globals.random_direction_table= match_malloc(
		"c:\\halo\\SOURCE\\math\\random_math.c",
		176,
		random_direction_geosphere->vertex_count*sizeof(real_vector3d));
	random_math_globals.random_direction_table_size= random_direction_geosphere->vertex_count;
	for (index= 0; index<random_direction_geosphere->vertex_count; index++)
		random_math_globals.random_direction_table[index]=
			*((real_vector3d *)random_direction_geosphere->vertices + index);
	geosphere_dispose(random_direction_geosphere);
	return;
}

void
random_math_dispose(
	void)
{
	debug_free(
		random_math_globals.random_direction_table,
		"c:\\halo\\SOURCE\\math\\random_math.c",
		200);
	return;
}

unsigned short seed_random(
	unsigned long *seed)
{
	*seed = *seed*RANDOM_A+RANDOM_C;
	return *seed>>16;
}

short seed_random_range(
	unsigned long *seed,
	short lower_bound,
	short upper_bound)
{
	return lower_bound+((unsigned long)(upper_bound-lower_bound)*seed_random(seed)>>16);
}

real real_seed_random(
	unsigned long *seed)
{
	*seed = *seed*RANDOM_A+RANDOM_C;
	return (real)(*seed>>16)/65535.0f;
}

real real_seed_random_range(
	unsigned long *seed,
	real lower_bound,
	real upper_bound)
{
	real random= real_seed_random(seed);
	return lower_bound+(upper_bound-lower_bound)*random;
}

real_vector3d *seed_random_direction3d(
	unsigned long *seed,
	real_vector3d *direction)
{
	return direction3d_from_table(
		direction,
		seed_random_range(seed, 0, random_math_globals.random_direction_table_size));
}

void
seed_random_orientation(
	unsigned long *seed,
	real_vector3d *facing,
	real_vector3d *up)
{
	real azimuth= real_seed_random_range(seed, 0.f, 2.f*_pi);
	real elevation= real_seed_random_range(seed, -_pi/2.f, _pi/2.f);
	real roll= real_seed_random_range(seed, 0.f, 2.f*_pi);
	real azimuth_cosine= cosine(azimuth);
	real azimuth_sine= sine(azimuth);
	real elevation_cosine= cosine(elevation);
	real elevation_sine= sine(elevation);

	facing->i= elevation_cosine*azimuth_cosine;
	facing->j= elevation_cosine*azimuth_sine;
	facing->k= elevation_sine;

	up->i= -azimuth_cosine*elevation_sine;
	up->j= -azimuth_sine*elevation_sine;
	up->k= elevation_cosine;

	yaw_vectors(up, facing, sine(roll), cosine(roll));
	return;
}

real_vector3d *
seed_random_vector_in_cone3d(
	unsigned long *seed,
	real_vector3d const *axis,
	real inner_cone_angle,
	real outer_cone_angle,
	real_vector3d *result)
{
	real_vector3d random_direction;
	real_vector3d rotation_axis;
	real angle;

	*result= *axis;
	direction3d_from_table(
		&random_direction,
		seed_random_range(seed, 0, random_math_globals.random_direction_table_size));
	cross_product3d(axis, &random_direction, &rotation_axis);
	if (normalize3d(&rotation_axis)>0.f)
	{
		angle= real_seed_random_range(seed, inner_cone_angle, outer_cone_angle);
		rotate_vector_about_axis(result, &rotation_axis, sine(angle), cosine(angle));
	}

	return result;
}

/* ---------- private code */

static real_vector3d *
direction3d_from_table(
	real_vector3d *result,
	short index)
{
	real_vector3d *random_direction=
		&random_math_globals.random_direction_table[index];
	match_assert(
		"c:\\halo\\SOURCE\\math\\random_math.c",
		250,
		random_math_globals.random_direction_table);
	match_assert(
		"c:\\halo\\SOURCE\\math\\random_math.c",
		251,
		index>=0 && index<random_math_globals.random_direction_table_size);
	*result= *random_direction;
	return result;
}

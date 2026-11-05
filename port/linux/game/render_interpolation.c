/*
RENDER_INTERPOLATION.C

Frames between the game's 30 Hz ticks, for the native ports (port/linux,
port/android, port/windows; see port/linux/README.md, "Frame rate").

The game simulates in 30 Hz ticks and originally drew one frame per tick.
The ports draw at the display's refresh rate instead, and every frame shows
the world between the last two ticks: after each tick the camera, every
object's node matrices and the first-person weapon's pose are kept, and a
frame blends the previous and the latest by how far the game clock has run
into the next tick. That puts what is drawn one tick (33 ms) behind the
simulation, the usual price of interpolation. (A Catmull-Rom spline would
also need the tick after the pair it spans: two ticks behind.)

Rotations are blended as quaternions (normalised lerp, taking the shorter
way round), positions and scales linearly. Anything that moves further than
a tick of motion plausibly allows (teleports, respawns, camera cuts) snaps
instead of sweeping across the world, and so does a pose whose nodes moved
too far for one tick: two snapshots of different poses (a model swapped,
another weapon's skeleton, a pose left from seconds before) blended node by
node stretch vertices across the screen. The first-person weapon's two poses
are told apart by the weapon and its animation graph instead, since its own
animations (a rocket launcher's melee) move further in a tick than any
distance that would tell them from another weapon's.

Particles, contrails and other effects already move every frame
(game_frame), so they need nothing here.

An object the distributed netcode moves to where the host has it
(port/linux/game/network_objects.c) is drawn gliding there over a few ticks
rather than jumping: its snapshots move with it, and the difference is drawn
on top of it, whole until the next tick and fading from then on (no more than
a few world units of it: further is a jump). A local player's view glides so
with their unit, and with what it rides. A point placed on an object from the
tick, such as a nav marker, is moved as far as the object is drawn from there
(render_interpolation_object_point).
*/

#include "cseries.h"
#include "math/real_math.h"
#include "objects/objects.h"
#include "camera/camera_scripting.h"
#include "camera/director.h"
#include "camera/observer.h"
#include "cutscene/cinematics.h"
#include "game/players.h"
#include "render/render_cameras.h"
#include "units/units.h"

/* port/linux/src/port_config.c */
int config_boolean(const char *name);
unsigned long config_changes(void);

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ---------- constants */

/* (by absolute index: the object array's all) */
#define MAXIMUM_INTERPOLATED_OBJECTS MAXIMUM_OBJECTS_PER_MAP
#define MAXIMUM_INTERPOLATED_NODES 64

/* world units (10 feet each) a node may move in one tick before it snaps:
well beyond any vehicle, short of any teleport */
#define OBJECT_SNAP_DISTANCE 10.0f
/* ... a node may move in the object's root node's frame in one tick before
the pose is taken for a new one, not blended to: further than any limb or
part of a model moves in 33 ms (37 m/s), short of the game changing a pose
at once (an actor waking from dormancy, a model swapped), which blended
sweeps the vertices through poses it never had. (In the root's frame, so
that the whole object turning moves nothing: measured in the world, a fast
turn swung the head and weapon far enough to snap, and characters moved
like robots.) */
#define NODE_SNAP_DISTANCE 0.4f
/* a first-person node's two rotations within about two degrees of half a
turn apart (the quaternions' dot product): which way it turned cannot be
told, and blending either way spins it through poses it never had, as the
rocket launcher's barrels set back by half a turn after a shot */
#define FIRST_PERSON_HALF_TURN_COSINE 0.02f
/* a correction's difference left drawn after each tick (of 1) */
#define CORRECTION_DECAY 0.6f
/* ... and small enough to be none */
#define CORRECTION_NEGLIGIBLE 0.001f
/* ... or so large that it is no glide but a jump (corrections come one on
another: their sum is drawn): past the largest a client's own unit or
vehicle is corrected by (3 and 4 world units), short of a snap */
#define CORRECTION_MAXIMUM 8.0f
/* a camera cut: a jump or turn no player or scripted camera makes in 33 ms */
#define CAMERA_CUT_DISTANCE 3.0f
#define CAMERA_CUT_COSINE 0.5f

/* ---------- structures */

struct interpolation_quaternion
{
	real i, j, k, w;
};

/* a node's rotation: whether its basis is one a quaternion can hold, and
that quaternion */
struct interpolation_rotation
{
	struct interpolation_quaternion quaternion;
	boolean is_rotation;
};

struct interpolated_object
{
	long object_index; /* NONE when unused */
	long tick; /* the tick of the latest snapshot */
	short node_count;
	short node_capacity;
	boolean has_previous;
	byte latest; /* which snapshot is the latest */
	long blended_frame;
	/* where it is drawn from where it is: a correction fading, and those
	since the last tick, drawn whole until the next begins to fade them */
	real_vector3d correction;
	real_vector3d correction_pending;
	/* [0] and [1]: the two snapshots, [2]: the blend drawn this frame */
	real_matrix4x3 *nodes;
	/* ... the two snapshots' rotations, found when first blended */
	struct interpolation_rotation *rotations;
	boolean rotations_valid[2];
};

struct interpolated_camera
{
	long tick;
	boolean valid;
	boolean has_previous;
	struct observer_result previous;
	struct observer_result latest;
	struct observer_result blended;
	/* its player's unit (or what it rides) corrected: as an object's */
	real_vector3d correction;
	real_vector3d correction_pending;
};

struct interpolated_first_person
{
	long tick;
	/* the weapon and animation graph the latest pose was of */
	long weapon_index;
	long animation_graph_index;
	short node_count;
	boolean has_previous;
	real_matrix4x3 previous[MAXIMUM_INTERPOLATED_NODES];
	real_matrix4x3 latest[MAXIMUM_INTERPOLATED_NODES];
};

/* ---------- globals */

static struct interpolated_object *interpolated_objects;
static struct interpolated_camera interpolated_cameras[MAXIMUM_LOCAL_PLAYERS];
static struct interpolated_first_person interpolated_first_person[MAXIMUM_LOCAL_PLAYERS];
static long interpolation_tick;
static long interpolation_frame;
static boolean interpolation_rendering;
static real interpolation_fraction = 1.0f;

/* ---------- blending */

static real lerp(real a, real b, real t)
{
	return a + (b - a) * t;
}

static void point_lerp(real_point3d const *a, real_point3d const *b, real t, real_point3d *result)
{
	result->x = lerp(a->x, b->x, t);
	result->y = lerp(a->y, b->y, t);
	result->z = lerp(a->z, b->z, t);
}

static real vector_length(real_vector3d const *v)
{
	return (real)sqrt(v->i * v->i + v->j * v->j + v->k * v->k);
}

static void vector_nlerp(real_vector3d const *a, real_vector3d const *b, real t, real_vector3d *result)
{
	real length;

	result->i = lerp(a->i, b->i, t);
	result->j = lerp(a->j, b->j, t);
	result->k = lerp(a->k, b->k, t);
	length = vector_length(result);
	if (length > 1e-6f)
	{
		result->i /= length;
		result->j /= length;
		result->k /= length;
	}
	else
	{
		*result = *b;
	}
}

/* an orthonormal right-handed basis, which a quaternion can represent */
static boolean basis_is_rotation(real_matrix4x3 const *matrix)
{
	real_vector3d cross;
	real determinant;

	if (fabs(vector_length(&matrix->forward) - 1.0f) > 1e-2f ||
		fabs(vector_length(&matrix->left) - 1.0f) > 1e-2f ||
		fabs(vector_length(&matrix->up) - 1.0f) > 1e-2f)
	{
		return FALSE;
	}
	cross.i = matrix->forward.j * matrix->left.k - matrix->forward.k * matrix->left.j;
	cross.j = matrix->forward.k * matrix->left.i - matrix->forward.i * matrix->left.k;
	cross.k = matrix->forward.i * matrix->left.j - matrix->forward.j * matrix->left.i;
	determinant = cross.i * matrix->up.i + cross.j * matrix->up.j + cross.k * matrix->up.k;
	return determinant > 0.5f;
}

/* the basis vectors are the matrix's columns (x forward, y left, z up) */
static void quaternion_from_basis(real_matrix4x3 const *matrix, struct interpolation_quaternion *q)
{
	real m00 = matrix->forward.i, m10 = matrix->forward.j, m20 = matrix->forward.k;
	real m01 = matrix->left.i, m11 = matrix->left.j, m21 = matrix->left.k;
	real m02 = matrix->up.i, m12 = matrix->up.j, m22 = matrix->up.k;
	real trace = m00 + m11 + m22;
	real s;

	if (trace > 0.0f)
	{
		s = 0.5f / (real)sqrt(trace + 1.0f);
		q->w = 0.25f / s;
		q->i = (m21 - m12) * s;
		q->j = (m02 - m20) * s;
		q->k = (m10 - m01) * s;
	}
	else if (m00 > m11 && m00 > m22)
	{
		s = 2.0f * (real)sqrt(1.0f + m00 - m11 - m22);
		q->w = (m21 - m12) / s;
		q->i = 0.25f * s;
		q->j = (m01 + m10) / s;
		q->k = (m02 + m20) / s;
	}
	else if (m11 > m22)
	{
		s = 2.0f * (real)sqrt(1.0f + m11 - m00 - m22);
		q->w = (m02 - m20) / s;
		q->i = (m01 + m10) / s;
		q->j = 0.25f * s;
		q->k = (m12 + m21) / s;
	}
	else
	{
		s = 2.0f * (real)sqrt(1.0f + m22 - m00 - m11);
		q->w = (m10 - m01) / s;
		q->i = (m02 + m20) / s;
		q->j = (m12 + m21) / s;
		q->k = 0.25f * s;
	}
}

static void basis_from_quaternion(struct interpolation_quaternion const *q, real_matrix4x3 *matrix)
{
	real ii = q->i * q->i, jj = q->j * q->j, kk = q->k * q->k;
	real ij = q->i * q->j, ik = q->i * q->k, jk = q->j * q->k;
	real wi = q->w * q->i, wj = q->w * q->j, wk = q->w * q->k;

	matrix->forward.i = 1.0f - 2.0f * (jj + kk);
	matrix->forward.j = 2.0f * (ij + wk);
	matrix->forward.k = 2.0f * (ik - wj);
	matrix->left.i = 2.0f * (ij - wk);
	matrix->left.j = 1.0f - 2.0f * (ii + kk);
	matrix->left.k = 2.0f * (jk + wi);
	matrix->up.i = 2.0f * (ik + wj);
	matrix->up.j = 2.0f * (jk - wi);
	matrix->up.k = 1.0f - 2.0f * (ii + jj);
}

static void rotation_from_matrix(real_matrix4x3 const *matrix, struct interpolation_rotation *rotation)
{
	rotation->is_rotation = basis_is_rotation(matrix);
	if (rotation->is_rotation)
		quaternion_from_basis(matrix, &rotation->quaternion);
}

/* a matrix a fraction t of the way from a to b, their rotations found */
static void matrix_blend_rotations(
	real_matrix4x3 const *a,
	real_matrix4x3 const *b,
	struct interpolation_rotation const *rotation_a,
	struct interpolation_rotation const *rotation_b,
	real t,
	real_matrix4x3 *result)
{
	result->scale = lerp(a->scale, b->scale, t);
	point_lerp(&a->position, &b->position, t, &result->position);
	if (rotation_a->is_rotation && rotation_b->is_rotation)
	{
		struct interpolation_quaternion qa = rotation_a->quaternion, qb = rotation_b->quaternion, q;
		real length;

		/* q and -q are the same rotation: take the shorter way round */
		if (qa.i * qb.i + qa.j * qb.j + qa.k * qb.k + qa.w * qb.w < 0.0f)
		{
			qb.i = -qb.i;
			qb.j = -qb.j;
			qb.k = -qb.k;
			qb.w = -qb.w;
		}
		q.i = lerp(qa.i, qb.i, t);
		q.j = lerp(qa.j, qb.j, t);
		q.k = lerp(qa.k, qb.k, t);
		q.w = lerp(qa.w, qb.w, t);
		length = (real)sqrt(q.i * q.i + q.j * q.j + q.k * q.k + q.w * q.w);
		if (length > 1e-6f)
		{
			q.i /= length;
			q.j /= length;
			q.k /= length;
			q.w /= length;
			basis_from_quaternion(&q, result);
			return;
		}
	}
	/* a basis a quaternion cannot hold (scaled or mirrored): blend it as is */
	result->forward.i = lerp(a->forward.i, b->forward.i, t);
	result->forward.j = lerp(a->forward.j, b->forward.j, t);
	result->forward.k = lerp(a->forward.k, b->forward.k, t);
	result->left.i = lerp(a->left.i, b->left.i, t);
	result->left.j = lerp(a->left.j, b->left.j, t);
	result->left.k = lerp(a->left.k, b->left.k, t);
	result->up.i = lerp(a->up.i, b->up.i, t);
	result->up.j = lerp(a->up.j, b->up.j, t);
	result->up.k = lerp(a->up.k, b->up.k, t);
}

/* the vector's parts' sum, large enough to be a correction (so written that
one not a number is none) */
static boolean correction_significant(real_vector3d const *correction)
{
	return fabs(correction->i) + fabs(correction->j) + fabs(correction->k) >= CORRECTION_NEGLIGIBLE;
}

/* a correction a tick on: what was drawn fades, what came since is drawn
whole from now on, fading from the next */
static void correction_advance(real_vector3d *correction, real_vector3d *pending)
{
	correction->i = correction->i * CORRECTION_DECAY + pending->i;
	correction->j = correction->j * CORRECTION_DECAY + pending->j;
	correction->k = correction->k * CORRECTION_DECAY + pending->k;
	*pending = *global_zero_vector3d;
	if (!correction_significant(correction))
		*correction = *global_zero_vector3d;
}

/* a correction as drawn this frame: fading through the tick as it does tick
to tick, and those since the tick whole */
static void correction_drawn(real_vector3d const *correction, real_vector3d const *pending, real_vector3d *drawn)
{
	real fade = lerp(1.0f, CORRECTION_DECAY, interpolation_fraction);

	drawn->i = correction->i * fade + pending->i;
	drawn->j = correction->j * fade + pending->j;
	drawn->k = correction->k * fade + pending->k;
}

/* a correction added (offset): all of it dropped when the sum is too large
to glide (or not a number) */
static void correction_add(real_vector3d *correction, real_vector3d *pending, real_vector3d const *offset)
{
	real i, j, k;

	pending->i += offset->i;
	pending->j += offset->j;
	pending->k += offset->k;
	i = correction->i + pending->i;
	j = correction->j + pending->j;
	k = correction->k + pending->k;
	if (!(i * i + j * j + k * k <= CORRECTION_MAXIMUM * CORRECTION_MAXIMUM))
	{
		*correction = *global_zero_vector3d;
		*pending = *global_zero_vector3d;
	}
}

static real distance_squared(real_point3d const *a, real_point3d const *b)
{
	real x = a->x - b->x, y = a->y - b->y, z = a->z - b->z;

	return x * x + y * y + z * z;
}

/* ---------- ticks */

void render_interpolation_tick(void)
{
	struct object_iterator iterator;
	struct object_datum *object;
	long previous_tick = interpolation_tick++;

	if (!halo_interpolation_enabled())
		return;
	/* the cameras' corrections a tick on, as the objects' (below) */
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
		{
			struct interpolated_camera *camera = &interpolated_cameras[local_player_index];

			if (camera->valid)
				correction_advance(&camera->correction, &camera->correction_pending);
		}
	}
	if (!interpolated_objects)
	{
		long index;

		interpolated_objects = calloc(MAXIMUM_INTERPOLATED_OBJECTS, sizeof(*interpolated_objects));
		if (!interpolated_objects)
			return;
		for (index = 0; index < MAXIMUM_INTERPOLATED_OBJECTS; index++)
			interpolated_objects[index].object_index = NONE;
	}

	object_iterator_new(&iterator, _object_mask_all, 0);
	while ((object = (struct object_datum *)object_iterator_next(&iterator)) != NULL)
	{
		long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		struct interpolated_object *record;
		short node_count = (short)(object->object.node_matrices.size / (short)sizeof(real_matrix4x3));
		boolean continuing;

		if (absolute_index >= MAXIMUM_INTERPOLATED_OBJECTS)
			continue;
		record = &interpolated_objects[absolute_index];
		if (node_count <= 0 || node_count > MAXIMUM_INTERPOLATED_NODES)
		{
			record->object_index = NONE;
			continue;
		}
		if (record->node_capacity < node_count)
		{
			real_matrix4x3 *nodes = realloc(record->nodes, 3 * node_count * sizeof(real_matrix4x3));
			struct interpolation_rotation *rotations;

			if (!nodes)
			{
				record->object_index = NONE;
				continue;
			}
			record->nodes = nodes;
			rotations = realloc(record->rotations, 2 * node_count * sizeof(struct interpolation_rotation));
			if (!rotations)
			{
				record->object_index = NONE;
				continue;
			}
			record->rotations = rotations;
			record->node_capacity = node_count;
			record->object_index = NONE; /* the old snapshots moved */
		}
		continuing = record->object_index == iterator.index &&
			record->node_count == node_count &&
			record->tick == previous_tick;
		if (continuing)
		{
			record->latest ^= 1;
			correction_advance(&record->correction, &record->correction_pending);
		}
		else
		{
			record->correction = *global_zero_vector3d;
			record->correction_pending = *global_zero_vector3d;
		}
		memcpy(
			record->nodes + record->latest * record->node_capacity,
			object_get_node_matrices(iterator.index),
			node_count * sizeof(real_matrix4x3));
		record->rotations_valid[record->latest] = FALSE;
		record->object_index = iterator.index;
		record->node_count = node_count;
		record->tick = interpolation_tick;
		record->has_previous = continuing;
		record->blended_frame = NONE;
	}
}

/* a new map (game.c): its objects take the indices of the last one's, and
nothing of theirs is drawn from */
void render_interpolation_reset(void)
{
	long index;

	if (interpolated_objects)
	{
		for (index = 0; index < MAXIMUM_INTERPOLATED_OBJECTS; index++)
			interpolated_objects[index].object_index = NONE;
	}
	memset(interpolated_cameras, 0, sizeof(interpolated_cameras));
	for (index = 0; index < MAXIMUM_LOCAL_PLAYERS; index++)
	{
		interpolated_first_person[index].node_count = 0;
		interpolated_first_person[index].has_previous = FALSE;
	}
}

/* ---------- frames */

void render_interpolation_frame_begin(void)
{
	interpolation_rendering = halo_interpolation_enabled();
	interpolation_frame++;
	interpolation_fraction = game_time_get_tick_fraction();
}

void render_interpolation_frame_end(void)
{
	interpolation_rendering = FALSE;
}

real render_interpolation_fraction(void)
{
	return interpolation_rendering ? interpolation_fraction : 1.0f;
}

real_matrix4x3 *render_interpolation_object_node_matrices(long object_index)
{
	struct interpolated_object *record;
	long absolute_index;

	if (!interpolation_rendering || !interpolated_objects || object_index == NONE)
		return NULL;
	absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);
	if (absolute_index >= MAXIMUM_INTERPOLATED_OBJECTS)
		return NULL;
	record = &interpolated_objects[absolute_index];
	if (record->object_index != object_index || record->tick != interpolation_tick || !record->has_previous)
		return NULL;
	if (record->blended_frame != interpolation_frame)
	{
		real_matrix4x3 const *previous = record->nodes + (record->latest ^ 1) * record->node_capacity;
		real_matrix4x3 const *latest = record->nodes + record->latest * record->node_capacity;
		real_matrix4x3 *blended = record->nodes + 2 * record->node_capacity;
		short node_index;
		/* (so written that a position not a number snaps) */
		boolean snap = !(distance_squared(&previous[0].position, &latest[0].position) <=
			OBJECT_SNAP_DISTANCE * OBJECT_SNAP_DISTANCE);

		/* a node moved further in the root's frame than a tick allows: the
		two snapshots are different poses, not one moving (so written that
		a position not a number snaps) */
		for (node_index = 1; !snap && node_index < record->node_count; node_index++)
		{
			real_point3d previous_local, latest_local;

			matrix4x3_inverse_transform_point(&previous[0], &previous[node_index].position, &previous_local);
			matrix4x3_inverse_transform_point(&latest[0], &latest[node_index].position, &latest_local);
			snap = !(distance_squared(&previous_local, &latest_local) <= NODE_SNAP_DISTANCE * NODE_SNAP_DISTANCE);
		}
		if (!snap)
		{
			struct interpolation_rotation *previous_rotations =
				record->rotations + (record->latest ^ 1) * record->node_capacity;
			struct interpolation_rotation *latest_rotations =
				record->rotations + record->latest * record->node_capacity;
			short snapshot;

			/* (each snapshot's rotations found once, not every frame: its
			nodes' positions may move with a correction, their rotations
			never) */
			for (snapshot = 0; snapshot < 2; snapshot++)
			{
				if (!record->rotations_valid[snapshot])
				{
					real_matrix4x3 const *nodes = record->nodes + snapshot * record->node_capacity;
					struct interpolation_rotation *rotations = record->rotations + snapshot * record->node_capacity;

					for (node_index = 0; node_index < record->node_count; node_index++)
						rotation_from_matrix(&nodes[node_index], &rotations[node_index]);
					record->rotations_valid[snapshot] = TRUE;
				}
			}
			for (node_index = 0; node_index < record->node_count; node_index++)
			{
				matrix_blend_rotations(&previous[node_index], &latest[node_index], &previous_rotations[node_index],
					&latest_rotations[node_index], interpolation_fraction, &blended[node_index]);
			}
		}
		if (snap)
			memcpy(blended, latest, record->node_count * sizeof(real_matrix4x3));
		if (correction_significant(&record->correction) || correction_significant(&record->correction_pending))
		{
			real_vector3d drawn;

			correction_drawn(&record->correction, &record->correction_pending, &drawn);
			for (node_index = 0; node_index < record->node_count; node_index++)
			{
				blended[node_index].position.x += drawn.i;
				blended[node_index].position.y += drawn.j;
				blended[node_index].position.z += drawn.k;
			}
		}
		record->blended_frame = interpolation_frame;
	}
	return record->nodes + 2 * record->node_capacity;
}

/* a point placed on the object from the tick, moved with the object's root
to where this frame draws it: unchanged outside a frame between ticks, or
for an object not blended */
void render_interpolation_object_point(long object_index, real_point3d *point)
{
	struct object_datum *object;
	real_matrix4x3 const *drawn;
	real_matrix4x3 const *simulated;

	if (!interpolation_rendering || object_index == NONE)
		return;
	drawn = render_interpolation_object_node_matrices(object_index);
	object = object_try_and_get(object_index);
	if (!drawn || !object || object->object.node_matrices.size < (short)sizeof(real_matrix4x3))
		return;
	simulated = object_header_block_get(object_index, &object->object.node_matrices);
	point->x += drawn[0].position.x - simulated[0].position.x;
	point->y += drawn[0].position.y - simulated[0].position.y;
	point->z += drawn[0].position.z - simulated[0].position.z;
}

/* ---------- corrections */

/* the object (and what it carries) moved by the netcode from where it was,
offset from where it is now: drawn from there, gliding */
void render_interpolation_correct_object(long object_index, real_vector3d const *offset)
{
	struct object_datum *object;
	long child_index;
	long absolute_index;

	/* (so written that an offset not a number is none) */
	if (!interpolated_objects || object_index == NONE ||
		!(offset->i * offset->i + offset->j * offset->j + offset->k * offset->k <= OBJECT_SNAP_DISTANCE * OBJECT_SNAP_DISTANCE))
	{
		return;
	}
	absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);
	if (absolute_index < MAXIMUM_INTERPOLATED_OBJECTS &&
		interpolated_objects[absolute_index].object_index == object_index)
	{
		struct interpolated_object *record = &interpolated_objects[absolute_index];
		short snapshot;
		short node_index;

		/* the snapshots where it would have been, the difference drawn */
		for (snapshot = 0; snapshot < 2; snapshot++)
		{
			real_matrix4x3 *nodes = record->nodes + snapshot * record->node_capacity;

			for (node_index = 0; node_index < record->node_count; node_index++)
			{
				nodes[node_index].position.x -= offset->i;
				nodes[node_index].position.y -= offset->j;
				nodes[node_index].position.z -= offset->k;
			}
		}
		correction_add(&record->correction, &record->correction_pending, offset);
		record->blended_frame = NONE;
	}
	/* a local player's unit (by itself, or with what it rides): its
	first-person view, which the observer poses from it, glides with it */
	{
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
		{
			struct interpolated_camera *camera = &interpolated_cameras[local_player_index];

			if (!camera->valid || player_control_get_unit_index(local_player_index) != object_index)
				continue;
			camera->previous.position.x -= offset->i;
			camera->previous.position.y -= offset->j;
			camera->previous.position.z -= offset->k;
			camera->latest.position.x -= offset->i;
			camera->latest.position.y -= offset->j;
			camera->latest.position.z -= offset->k;
			correction_add(&camera->correction, &camera->correction_pending, offset);
		}
	}
	object = object_get(object_index);
	for (child_index = object->object.first_child_object_index; child_index != NONE;
		child_index = object_get(child_index)->object.next_object_index)
	{
		render_interpolation_correct_object(child_index, offset);
	}
}

/* ---------- camera */

static struct observer_result direct_cameras[MAXIMUM_LOCAL_PLAYERS];

static struct observer_result const *render_interpolation_blended_camera(
	short local_player_index,
	struct observer_result const *observer);

/* A first-person view is posed from the player's facing, which the input
turns every frame (player_control.c), but the observer keeps it as of the
last tick and the blend above draws it a tick later still. On foot, the view
points where the player aims now (display.direct_camera). In a vehicle's
seat or a cinematic the view is the seat's or the script's: left as it is.
Desktop only: display.direct_camera is not an Android setting, and its
default would otherwise apply there. */
static struct observer_result const *render_interpolation_direct_camera(
	short local_player_index,
	struct observer_result const *observer)
{
#ifdef HALO_ANDROID
	(void)local_player_index;
	return observer;
#else
	static int enabled;
	static unsigned long read_at = (unsigned long)-1;
	struct observer_result *direct;
	long unit_index;

	if (read_at != config_changes())
	{
		read_at = config_changes();
		enabled = config_boolean("display.direct_camera");
	}
	if (!enabled || !observer ||
		director_get_perspective(local_player_index) != _director_perspective_first_person ||
		director_inhibited_facing(local_player_index) ||
		cinematic_in_progress())
	{
		return observer;
	}
	unit_index = player_control_get_unit_index(local_player_index);
	if (unit_index == NONE || object_get(unit_index)->object.parent_object_index != NONE)
		return observer;

	direct = &direct_cameras[local_player_index];
	*direct = *observer;
	player_control_get_facing_direction(local_player_index, &direct->forward);
	observer_up_from_forward(&direct->forward, &direct->up);
	return direct;
#endif
}

struct observer_result const *render_interpolation_camera(
	short local_player_index,
	struct observer_result const *observer)
{
	if (local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return observer;
	return render_interpolation_direct_camera(local_player_index,
		render_interpolation_blended_camera(local_player_index, observer));
}

/* between two of the observer's results: the position, the axes (up kept
square to forward) and the field of view */
static void observer_blend(
	struct observer_result const *a,
	struct observer_result const *b,
	real t,
	struct observer_result *result)
{
	real along, length;

	*result = *b;
	point_lerp(&a->position, &b->position, t, &result->position);
	vector_nlerp(&a->forward, &b->forward, t, &result->forward);
	vector_nlerp(&a->up, &b->up, t, &result->up);
	along = result->up.i * result->forward.i + result->up.j * result->forward.j + result->up.k * result->forward.k;
	result->up.i -= result->forward.i * along;
	result->up.j -= result->forward.j * along;
	result->up.k -= result->forward.k * along;
	length = vector_length(&result->up);
	if (length > 1e-6f)
	{
		result->up.i /= length;
		result->up.j /= length;
		result->up.k /= length;
	}
	else
	{
		result->up = b->up;
	}
	result->field_of_view = lerp(a->field_of_view, b->field_of_view, t);
}

/* a cut between two of the observer's results (so written that a position
or direction not a number cuts) */
static boolean observer_cut(
	struct observer_result const *a,
	struct observer_result const *b)
{
	return !(distance_squared(&a->position, &b->position) <= CAMERA_CUT_DISTANCE * CAMERA_CUT_DISTANCE) ||
		!(a->forward.i * b->forward.i + a->forward.j * b->forward.j + a->forward.k * b->forward.k >= CAMERA_CUT_COSINE);
}

static struct observer_result const *render_interpolation_blended_camera(
	short local_player_index,
	struct observer_result const *observer)
{
	struct interpolated_camera *camera;
	real t = interpolation_fraction;

	if (!interpolation_rendering || !observer)
	{
		return observer;
	}
	camera = &interpolated_cameras[local_player_index];
	/* A scripted camera that moves with an object (a cinematic's camera
	point relative to a lifepod or dropship, or a unit's eyes in one) is
	placed by the object as it stands after the last tick. Blended from its
	own snapshots (taken at the first frame after each tick, a different
	part of a tick each tick), it did not follow the object as drawn, which
	is blended from the object's snapshots, or steps from tick to tick when
	its pose snaps: it ran up to 2 m ahead of or behind the object, out
	through the lifepod's hull in a30. It is drawn where the observer has
	it, moved with the object from where the object is to where it is
	drawn. (An animated camera has no such object; a scripted first-person
	camera looks along its unit's aim, which turns once a tick, and is
	blended from its snapshots as the player's camera is.) */
	if (director_get_perspective(local_player_index) == _director_perspective_scripted)
	{
		long relative_object_index = scripted_camera_object_relative_to();
		real_matrix4x3 const *drawn;

		if (relative_object_index != NONE &&
			!scripted_camera_object_is_first_person_camera(relative_object_index) &&
			object_try_and_get_and_verify_type(relative_object_index, _object_mask_all) &&
			(drawn = render_interpolation_object_node_matrices(relative_object_index)) != NULL)
		{
			/* (as of the last tick: object_get_node_matrices gives the drawn
			pose while a frame is drawn) */
			real_matrix4x3 const *nodes = (real_matrix4x3 const *)object_header_block_get(
				relative_object_index, &object_get(relative_object_index)->object.node_matrices);

			camera->blended = *observer;
			camera->blended.position.x += drawn[0].position.x - nodes[0].position.x;
			camera->blended.position.y += drawn[0].position.y - nodes[0].position.y;
			camera->blended.position.z += drawn[0].position.z - nodes[0].position.z;
			/* (the snapshots start again after) */
			camera->valid = FALSE;
			return &camera->blended;
		}
	}
	/* the observer as it stood after each tick (the first frame drawn
	after the tick) */
	if (!camera->valid || camera->tick != interpolation_tick)
	{
		/* (its correction taken on each tick, render_interpolation_tick) */
		if (!camera->valid)
		{
			camera->correction = *global_zero_vector3d;
			camera->correction_pending = *global_zero_vector3d;
		}
		camera->has_previous = camera->valid;
		camera->previous = camera->latest;
		camera->latest = *observer;
		/* Several ticks since the last snapshot (a long frame): the objects
		are drawn between the last two ticks, so the camera's previous is
		where it was a tick ago, as nearly as a steady move from the last
		snapshot tells. (Blended across all the ticks since, the camera
		moved further each frame than the world it is in, out through a
		Pelican's hull for a frame.) A cut stays a cut. */
		if (camera->has_previous && interpolation_tick - camera->tick > 1 &&
			!observer_cut(&camera->previous, &camera->latest))
		{
			real ticks = (real)(interpolation_tick - camera->tick);
			struct observer_result previous = camera->previous;

			observer_blend(&previous, &camera->latest, (ticks - 1.0f) / ticks, &camera->previous);
		}
		camera->tick = interpolation_tick;
		camera->valid = TRUE;
	}
	if (!camera->has_previous || observer_cut(&camera->previous, &camera->latest))
	{
		return observer;
	}

	observer_blend(&camera->previous, &camera->latest, t, &camera->blended);
	if (correction_significant(&camera->correction) || correction_significant(&camera->correction_pending))
	{
		real_vector3d drawn;

		correction_drawn(&camera->correction, &camera->correction_pending, &drawn);
		camera->blended.position.x += drawn.i;
		camera->blended.position.y += drawn.j;
		camera->blended.position.z += drawn.k;
	}
	return &camera->blended;
}

/* ---------- first-person weapon */

/* The first-person weapon and hands are posed in world space from the drawn
camera each frame, from animation state that changes once a tick: blend the
pose relative to the camera. */
void render_interpolation_first_person(
	short local_player_index,
	long weapon_index,
	long animation_graph_index,
	real_matrix4x3 *node_matrices,
	short node_count,
	struct render_camera const *camera)
{
	struct interpolated_first_person *first_person;
	real_matrix4x3 camera_matrix;
	real_matrix4x3 inverse_camera;
	struct interpolation_rotation previous_rotations[MAXIMUM_INTERPOLATED_NODES];
	struct interpolation_rotation latest_rotations[MAXIMUM_INTERPOLATED_NODES];
	short node_index;

	if (!interpolation_rendering ||
		local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS ||
		node_count <= 0 || node_count > MAXIMUM_INTERPOLATED_NODES)
	{
		return;
	}
	first_person = &interpolated_first_person[local_player_index];
	matrix4x3_from_point_and_vectors(&camera_matrix, &camera->position, &camera->forward, &camera->up);
	matrix4x3_inverse(&camera_matrix, &inverse_camera);
	if (first_person->tick != interpolation_tick ||
		first_person->weapon_index != weapon_index ||
		first_person->animation_graph_index != animation_graph_index)
	{
		/* the pose drawn last, if that was at the end of the tick just
		before: not one left from before the view was last away from first
		person (zoomed, in a vehicle, dead, in a cinematic), seconds old, and
		not another weapon's or skeleton's (of as many nodes) */
		first_person->has_previous = first_person->tick == interpolation_tick - 1 &&
			first_person->node_count == node_count &&
			first_person->weapon_index == weapon_index &&
			first_person->animation_graph_index == animation_graph_index;
		memcpy(first_person->previous, first_person->latest, sizeof(first_person->previous));
		first_person->tick = interpolation_tick;
		first_person->weapon_index = weapon_index;
		first_person->animation_graph_index = animation_graph_index;
	}
	for (node_index = 0; node_index < node_count; node_index++)
		matrix4x3_multiply(&inverse_camera, &node_matrices[node_index], &first_person->latest[node_index]);
	first_person->node_count = node_count;
	if (!first_person->has_previous)
		return;
	/* a node's move is not limited: one weapon's own animations, a melee
	or a reload, move as far in a tick as they were made to. Only a position
	not a number (so written) snaps the pose. */
	for (node_index = 0; node_index < node_count; node_index++)
	{
		real_matrix4x3 const *previous = &first_person->previous[node_index];
		real_matrix4x3 const *latest = &first_person->latest[node_index];

		if (!(distance_squared(&previous->position, &latest->position) <= REAL_MAX))
			return;
		rotation_from_matrix(previous, &previous_rotations[node_index]);
		rotation_from_matrix(latest, &latest_rotations[node_index]);
	}
	for (node_index = 0; node_index < node_count; node_index++)
	{
		struct interpolation_rotation const *previous_rotation = &previous_rotations[node_index];
		struct interpolation_rotation const *latest_rotation = &latest_rotations[node_index];
		real_matrix4x3 blended;

		if (previous_rotation->is_rotation && latest_rotation->is_rotation &&
			fabs(previous_rotation->quaternion.i * latest_rotation->quaternion.i +
				previous_rotation->quaternion.j * latest_rotation->quaternion.j +
				previous_rotation->quaternion.k * latest_rotation->quaternion.k +
				previous_rotation->quaternion.w * latest_rotation->quaternion.w) < FIRST_PERSON_HALF_TURN_COSINE)
		{
			/* half a turn: this node snaps, the others still blend */
			blended = first_person->latest[node_index];
		}
		else
		{
			matrix_blend_rotations(
				&first_person->previous[node_index],
				&first_person->latest[node_index],
				previous_rotation,
				latest_rotation,
				interpolation_fraction,
				&blended);
		}
		matrix4x3_multiply(&camera_matrix, &blended, &node_matrices[node_index]);
	}
}

/* ---------- time */

/* game time for animated shaders, continuous between ticks: the time of
the frame drawn (a tick behind the simulation, like the objects) */
real render_interpolation_game_time_sec(long ticks)
{
	real time;

	if (!interpolation_rendering)
		return (real)ticks * (1.0f / TICKS_PER_SECOND);
	time = ((real)ticks - 1.0f + interpolation_fraction) * (1.0f / TICKS_PER_SECOND);
	return time > 0.0f ? time : 0.0f;
}

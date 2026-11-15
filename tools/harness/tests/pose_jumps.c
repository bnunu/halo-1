#include "harness.h"
#include <math.h>

#include "config.inc"

/* The real pose_jumps (render_interpolation.c): whether an object's latest
pose is a new one, not blended to from the one before. A model of three
nodes, the root turned and moved between the poses (which moves no node in
the root's frame), node 1 the one that moves. */
#define NODES 3

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include "under_test.inc"

static real_matrix4x3 previous[NODES], latest[NODES];
static real_vector3d moves[NODES];

/* a root at x along the world's x, turned by the angle, its nodes at the
points given in its frame */
static void pose(real_matrix4x3 *nodes, real x, real angle, real node1_x, real node2_x)
{
	short node_index;

	for (node_index = 0; node_index < NODES; node_index++)
	{
		real local_x = node_index == 1 ? node1_x : node_index == 2 ? node2_x : 0.0f;
		real c = cosf(angle), s = sinf(angle);

		nodes[node_index].scale = 1.0f;
		nodes[node_index].forward.i = c; nodes[node_index].forward.j = s; nodes[node_index].forward.k = 0.0f;
		nodes[node_index].left.i = -s; nodes[node_index].left.j = c; nodes[node_index].left.k = 0.0f;
		nodes[node_index].up.i = 0.0f; nodes[node_index].up.j = 0.0f; nodes[node_index].up.k = 1.0f;
		nodes[node_index].position.x = x + c * local_x;
		nodes[node_index].position.y = s * local_x;
		nodes[node_index].position.z = 0.0f;
	}
}

/* each tick a pose, node 1 at the places given in turn: whether each tick's
is a jump, as a string of 0 and 1 (the first tick has no move before it) */
static char const *ticks(real const *places, short count)
{
	static char result[64];
	short tick;

	memset(moves, 0, sizeof(moves));
	pose(latest, 0.0f, 0.0f, places[0], 0.5f);
	for (tick = 1; tick < count; tick++)
	{
		memcpy(previous, latest, sizeof(latest));
		/* (the root goes far and turns: none of that is a node's move) */
		pose(latest, 3.0f * tick, 0.7f * tick, places[tick], 0.5f);
		result[tick - 1] = pose_jumps(previous, latest, NODES, moves, tick > 1) ? '1' : '0';
	}
	result[count - 1] = 0;
	return result;
}

int main(int argc, char **argv)
{
	const char *case_name = argc > 1 ? argv[1] : "";

	/* a node moving a little each tick, the root going far and turning: no jump */
	CASE("still")
	{
		real places[] = { 0.1f, 0.2f, 0.3f, 0.35f, 0.4f };

		CHECK(!strcmp(ticks(places, 5), "0000"), "%s", ticks(places, 5));
		return 0;
	}
	/* a node jumping from rest to a far place: a jump, the move before known or not */
	CASE("jump")
	{
		real first[] = { 0.0f, 1.0f, 1.0f };
		real later[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f };

		CHECK(!strcmp(ticks(first, 3), "10"), "from the first tick %s", ticks(first, 3));
		CHECK(!strcmp(ticks(later, 5), "0010"), "later %s", ticks(later, 5));
		return 0;
	}
	/* a node flying off faster each tick, as a30's lifepod's airbrakes:
	after its first fast tick (from rest, a jump) no jump */
	CASE("steady")
	{
		real places[] = { 0.0f, 0.3f, 0.9f, 1.6f, 2.5f, 3.6f, 5.0f, 7.0f, 9.5f };

		CHECK(!strcmp(ticks(places, 9), "00000000"), "%s", ticks(places, 9));
		return 0;
	}
	/* fast and steady, then a sudden turn back: a jump */
	CASE("reversal")
	{
		real places[] = { 0.0f, 0.3f, 0.6f, 1.0f, 1.5f, 2.0f, 1.0f };

		CHECK(!strcmp(ticks(places, 7), "000001"), "%s", ticks(places, 7));
		return 0;
	}
	/* a node not a number: a jump */
	CASE("not a number")
	{
		real places[] = { 0.0f, 0.1f, (real)NAN };

		CHECK(!strcmp(ticks(places, 3), "01"), "%s", ticks(places, 3));
		return 0;
	}

	return 2;
}

/*
OBSERVER_SMALL_TURNS.C (test)

The real observer_apply_rotational_displacement and
observer_rotation_vector_from_matrices (observer.c): the observer, which the
port moves every frame drawn, turns the camera a step a frame toward where
it is to look, and measures how far it has left to turn. The game's
normalize3d calls a vector of less than 0.0001 none, so a step that small
was not made and a turn left of less than 0.0002 was taken for none: at
thousands of frames a second the main menu's camera moved but never
turned. Turns however small are made and measured; large ones as before.
test_observer_small_turns.py takes the code from the sources
(under_test.inc).
*/

#include "harness.h"
#include <math.h>

#include "config.inc"

#include "under_test.inc"

static real_matrix4x3 about_z(real angle)
{
	real_matrix4x3 matrix;

	memset(&matrix, 0, sizeof(matrix));
	matrix.scale = 1.f;
	matrix.forward.i = (real)cos(angle);
	matrix.forward.j = (real)sin(angle);
	matrix.left.i = -(real)sin(angle);
	matrix.left.j = (real)cos(angle);
	matrix.up.k = 1.f;
	return matrix;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	/* a step of 0.00005 about up is made */
	CASE("small step")
	{
		real_vector3d forward = { 1.f, 0.f, 0.f }, up = { 0.f, 0.f, 1.f };
		real_vector3d step = { 0.f, 0.f, 0.00005f };

		observer_apply_rotational_displacement(&step, &forward, &up);
		CHECK(fabs(fabs(forward.j) - 0.00005f) < 0.000002f, "forward (%g, %g) not turned 0.00005", forward.i, forward.j);
		return 0;
	}
	/* a step of a radian, as before */
	CASE("large step")
	{
		real_vector3d forward = { 1.f, 0.f, 0.f }, up = { 0.f, 0.f, 1.f };
		real_vector3d step = { 0.f, 0.f, 1.f };

		observer_apply_rotational_displacement(&step, &forward, &up);
		CHECK(fabs(fabs(forward.j) - (real)sin(1.0)) < 1e-5f && fabs(forward.i - (real)cos(1.0)) < 1e-5f,
			"forward (%g, %g)", forward.i, forward.j);
		CHECK(fabs(up.k - 1.f) < 1e-6f, "up moved: %g", up.k);
		return 0;
	}
	/* a turn left of 0.00005 is measured as that */
	CASE("small turn left")
	{
		real_matrix4x3 a = about_z(0.f), b = about_z(0.00005f);
		real_vector3d rotation;

		observer_rotation_vector_from_matrices(&a, &b, &rotation);
		CHECK(fabs(fabs(rotation.k) - 0.00005f) < 0.000002f && fabs(rotation.i) < 1e-7f && fabs(rotation.j) < 1e-7f,
			"rotation (%g, %g, %g)", rotation.i, rotation.j, rotation.k);
		return 0;
	}
	/* a turn of half a radian, and one past half a turn the shorter way */
	CASE("large turn left")
	{
		real_matrix4x3 a = about_z(0.f), b = about_z(0.5f), c = about_z(4.f);
		real_vector3d rotation;

		observer_rotation_vector_from_matrices(&a, &b, &rotation);
		CHECK(fabs(fabs(rotation.k) - 0.5f) < 1e-5f, "half a radian: %g", rotation.k);
		observer_rotation_vector_from_matrices(&a, &c, &rotation);
		CHECK(fabs(fabs(rotation.k) - (real)(2.0 * 3.14159265358979 - 4.0)) < 1e-4f, "4 radians: %g", rotation.k);
		return 0;
	}
	/* none: none */
	CASE("no turn")
	{
		real_matrix4x3 a = about_z(0.3f);
		real_vector3d rotation;

		observer_rotation_vector_from_matrices(&a, &a, &rotation);
		CHECK(fabs(rotation.i) + fabs(rotation.j) + fabs(rotation.k) < 1e-6f, "rotation (%g, %g, %g)",
			rotation.i, rotation.j, rotation.k);
		return 0;
	}
	/* the measured turn, made, is the turn: the step and the measure agree in sign */
	CASE("agree")
	{
		real_matrix4x3 a = about_z(0.f), b = about_z(0.00005f);
		real_vector3d rotation;
		real_vector3d forward = { 1.f, 0.f, 0.f }, up = { 0.f, 0.f, 1.f };

		observer_rotation_vector_from_matrices(&a, &b, &rotation);
		observer_apply_rotational_displacement(&rotation, &forward, &up);
		CHECK(fabs(forward.j - b.forward.j) < 0.000002f, "turned to %g, not %g", forward.j, b.forward.j);
		return 0;
	}

	return 2;
}

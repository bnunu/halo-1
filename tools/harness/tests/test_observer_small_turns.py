"""The real observer_apply_rotational_displacement and observer_rotation_vector_from_matrices (observer.c): the
observer turns the camera a step a frame and measures the turn left; normalize3d's cutoff made a step of less than
0.0001 none and measured a turn of less than 0.0002 as none, so at thousands of frames a second the main menu's camera
moved but never turned. Turns however small are made and measured."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, inline, mutated, read, run, structure  # noqa: E402

CASES = ["small step", "large step", "small turn left", "large turn left", "no turn", "agree"]

NEGATIVE_CONTROLS = {
    # the game's: normalize3d's cutoff on the step
    "step-cutoff": (("\treal angle = magnitude3d(&axis);\n", "\treal angle = normalize3d(&axis);\n"), "small step"),
    # the game's: vector_from_matrices4x3's measure (quaternion_to_angle_and_vector)
    "measure-cutoff": (("\tlength = magnitude3d(&quaternion.v);\n",
                        "\tlength = magnitude3d(&quaternion.v);\n\tif (length < 0.0001f) length = 0.f;\n"),
                       "small turn left"),
    # the longer way round
    "long-way": (("\t\tif (angle > _pi)\n\t\t\tangle -= 2.f*_pi;\n", ""), "large turn left"),
}


def generated(fault=None):
    maths = read("source/math/real_math.h").replace("\r\n", "\n")
    matrices = read("source/math/matrix_math.c").replace("\r\n", "\n")
    observer = read("source/camera/observer.c").replace("\r\n", "\n")
    real_math = read("source/math/real_math.c").replace("\r\n", "\n")
    config = "\n".join([
        "#define _real_epsilon 0.0001f",
        "#define _pi ((real)3.14159265358979323846f)",
        "typedef struct { real n[3]; } unused_vector;",
        structure(maths, "real_matrix4x3"),
        "typedef struct real_matrix4x3 real_matrix4x3;",
        structure(maths, "real_quaternion"),
        "typedef struct real_quaternion real_quaternion;",
        "static real_vector3d const zero_vector = { 0.f, 0.f, 0.f };",
        "static real_vector3d const *global_zero_vector3d = &zero_vector;",
        "static short matrix4x3_next[3] = { 1, 2, 0 };",
    ]) + "\n"
    code = "\n".join(
        [inline(maths, name) for name in ["square_root", "arctangent", "dot_product3d", "magnitude_squared3d",
                                          "magnitude3d", "scale_vector3d", "normalize3d"]] +
        [function(real_math, "rotate_vector_about_axis")] +
        [function(matrices, name) for name in ["matrix4x3_inverse", "matrix4x3_multiply",
                                               "matrix4x3_rotation_to_quaternion"]] +
        [function(observer, name) for name in ["observer_apply_rotational_displacement",
                                               "observer_rotation_vector_from_matrices"]]) + "\n"
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code))


def test_the_observer_uses_them():
    """The observer measures the turn left with observer_rotation_vector_from_matrices."""
    observer = read("source/camera/observer.c").replace("\r\n", "\n")
    displacement = function(observer, "observer_rotational_displacement")
    assert "observer_rotation_vector_from_matrices(&matrix0, &matrix1, rotation);" in displacement
    assert "vector_from_matrices4x3(" not in displacement


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("observer_small_turns", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("observer_small_turns", generated(fault)), case)
    assert status == CHECK_FAILED, output

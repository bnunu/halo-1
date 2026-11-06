"""The real rasterizer_screen_effect_cover_target (rasterizer_xbox_screen_effect.c): a screen effect pass's quad covers
the whole viewport it draws into, from its left and top edges, on a screen of any width and in split screen, and each
place on it samples the texture where it did before."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run  # noqa: E402

CASES = ["covers-the-screen", "covers-at-the-xbox-size", "split-screen"]

# a fault in rasterizer_screen_effect_cover_target and the case that must catch it
NEGATIVE_CONTROLS = {
    # the quad left where the translator puts it: half a game pixel inside the left and top edges
    "not-moved": (("half_pixel_x = 1.0f / (real)viewport_width;", "half_pixel_x = 0.0f;"), "covers-the-screen"),
    "moved-down": (("vertex_bounds->y0 += half_pixel_y;", "vertex_bounds->y0 -= half_pixel_y;"),
                   "covers-the-screen"),
    # the quad moved but not its texture coordinates: every pass samples half a pixel over
    "texture-not-moved": (("return 0.5f;", "return 0.0f;"), "covers-the-screen"),
    # the window's pixel for the whole surface's
    "window-for-the-surface": (("destination_target == _rasterizer_target_screen_effect", "FALSE"), "split-screen"),
}


def generated(fault=None):
    source = read("source/rasterizer/xbox/rasterizer_xbox_screen_effect.c")
    config = "enum { _rasterizer_target_screen_effect = %d };\n" % constant(source, "_rasterizer_target_screen_effect")
    code = function(source, "rasterizer_screen_effect_cover_target")
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code + "\n"))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("screen_effect_cover", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("screen_effect_cover", generated(fault)), case)
    assert status == CHECK_FAILED, f"the quad with a fault ({control}) passed '{case}': the test cannot see it"

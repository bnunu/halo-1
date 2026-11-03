"""The real render_object_shadow_begin (render_objects.c): an object's shadow map is centered where the frame draws the
object between the ticks, as its model is drawn into the map, not where the tick left it."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure  # noqa: E402

CASES = ["centered-where-drawn", "at-the-tick"]

# a fault in render_object_shadow_begin and the case that must catch it
NEGATIVE_CONTROLS = {
    # centered where the tick left the object, as before
    "centered-at-the-tick": (("render_interpolation_object_point(data->object_index, &center);", ""),
                             "centered-where-drawn"),
}


def generated(fault=None):
    source = read("source/render/render_objects.c")
    config = structure(source, "object_render_data") + "\n"
    code = function(source, "render_object_shadow_begin")
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code + "\n"))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("shadow_center", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("shadow_center", generated(fault)), case)
    assert status == CHECK_FAILED, f"the shadow with a fault ({control}) passed '{case}': the test cannot see it"

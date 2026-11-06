"""The real contrails_update (contrails.c) over a projectile moving a tick at a time, with a Covenant plasma turret's
contrail: on the first frame after a tick its points are made and aged by the ticks' time, a 30th of a second each,
so that the trail is the same each tick at any frame rate, as the Xbox drew it a frame a tick; a game time gone back
or jumping counts as a tick."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, mutated, read, run, structure  # noqa: E402

CASES = ["steady-at-144", "uneven-frames", "time-jumps"]
FUNCTIONS = ("contrail_scale_value", "contrail_scale_random_value", "contrail_next_frame",
             "contrail_compute_new_point_count", "contrail_add_points", "contrail_update_points", "contrails_update")

# a fault in the code and the case that must catch it
NEGATIVE_CONTROLS = {
    # the points made and aged by the frames' time since the last tick, about a tick's (as before)
    "frames-time": (("point_dt = (real)ticks / TICKS_PER_SECOND;", "point_dt = dt * 4.8f * (real)ticks;"),
                    "uneven-frames"),
    # the points aged every frame by its own time (before fix-contrail-point-ticks)
    "aged-each-frame": (("if (point_dt > 0.0f)\n\t\t\tcontrail_update_points(contrail_index, point_dt);",
                         "if (dt > 0.0f)\n\t\t\tcontrail_update_points(contrail_index, dt);"), "steady-at-144"),
    # a game time gone back ages the points backwards
    "time-back": (("ticks < 1 || ", ""), "time-jumps"),
    # a game time jumping ahead ages them all at once
    "time-ahead": ((" || ticks > TICKS_PER_SECOND", ""), "time-jumps"),
}


def generated(fault=None):
    source = read("source/effects/contrails.c")
    header = read("source/effects/contrails.h")
    definitions = read("source/effects/contrail_definitions.h")
    config = "\n".join([
        "enum { MAXIMUM_CONTRAIL_INSTANCES = %d, MAXIMUM_CONTRAIL_POINTS = %d };" % (
            constant(source, "MAXIMUM_CONTRAIL_INSTANCES"), constant(source, "MAXIMUM_CONTRAIL_POINTS")),
        enum_with(source, "_contrail_active_bit"),
        enum_with(source, "_contrail_point_new_bit"),
        enum_with(source, "_contrail_state_duration_bit"),
        enum_with(source, "_contrail_scales_point_generation_rate_bit"),
        structure(header, "contrail_datum"),
        structure(header, "contrail_point_datum"),
        structure(definitions, "contrail_point_state"),
        structure(definitions, "contrail_shader"),
        structure(definitions, "contrail_definition"),
    ]) + "\n"
    code = "\n".join(function(source, name) for name in FUNCTIONS) + "\n"
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("contrail_tick_points", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("contrail_tick_points", generated(fault)), case)
    assert status == CHECK_FAILED, f"the contrails with a fault ({control}) passed '{case}': the test cannot see it"

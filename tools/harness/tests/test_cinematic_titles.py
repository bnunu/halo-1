"""The real cinematic_render (cinematics.c) over a fake scenario: a scenario's title is drawn in the game, not over a
menu, and its time goes on under one; on a wide screen, the edge it is justified to keeps its place."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run, structure  # noqa: E402

CASES = ["drawn-in-the-game", "not-over-a-menu", "time-goes-on-under-a-menu", "right-justified-on-a-wide-screen",
         "left-justified-on-a-wide-screen", "centered-on-a-wide-screen", "unmoved-on-640-columns"]

# a fault in cinematic_render and the case that must catch it
NEGATIVE_CONTROLS = {
    "drawn-over-a-menu": (("if (!ui_widgets_active())", "if (TRUE)"), "not-over-a-menu"),
    "time-stops-under-a-menu": (("active_title->time += game_time_get_paused()",
                                 "active_title->time += ui_widgets_active() || game_time_get_paused()"),
                                "time-goes-on-under-a-menu"),
    # the box's middle moved, whatever the justification, as before the fix
    "right-justified-moved-by-its-middle": (("anchor = title_bounds->x1;",
                                             "anchor = (short)((title_bounds->x0 + title_bounds->x1) / 2);"),
                                            "right-justified-on-a-wide-screen"),
    "left-justified-moved-by-its-middle": (("anchor = title_bounds->x0;",
                                            "anchor = (short)((title_bounds->x0 + title_bounds->x1) / 2);"),
                                           "left-justified-on-a-wide-screen"),
}


def generated(fault=None):
    cinematics = read("source/cutscene/cinematics.c")
    header = read("source/cutscene/cinematics.h")
    config = "enum { MAXIMUM_QUEUED_CINEMATIC_TITLES = %d };\n" % constant(cinematics,
                                                                          "MAXIMUM_QUEUED_CINEMATIC_TITLES")
    for source, name in ((cinematics, "scenario_cutscene_title"), (header, "cinematic_title"),
                         (header, "cinematic_global_data")):
        config += structure(source, name) + "\n"
    code = function(cinematics, "cinematic_render")
    if fault:
        code = mutated(code, *fault)
    return (("config.inc", config), ("under_test.inc", code + "\n"))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("cinematic_titles", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("cinematic_titles", generated(fault)), case)
    assert status == CHECK_FAILED, f"cinematics.c with a fault ({control}) passed '{case}': the test cannot see it"

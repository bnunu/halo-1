"""No teammate marker is drawn when the map's globals have no multiplayer HUD bitmap."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run  # noqa: E402

CASES = ["bitmap", "no-bitmap"]


def generated(fault=None):
    code = function(read("source/interface/hud.c"), "hud_draw_friendly_indicator")
    if fault:
        code = mutated(code, *fault)
    declarations = enum_with(read("source/interface/interface.h"), "_interface_bitmap_multiplayer_hud,")
    return (("declarations.inc", declarations + "\n"), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_marker(case):
    status, output = run(build("hud_ally_marker", generated()), case)
    assert status == 0, output


def test_negative_control():
    # without the NULL check
    fault = ("if (bitmap && _texture", "if (_texture")
    status, output = run(build("hud_ally_marker", generated(fault)), "no-bitmap")
    assert status == CHECK_FAILED, output

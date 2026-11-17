"""A screen effect's blur in split screen is left out instead of ending the game."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure  # noqa: E402

CASES = ["one", "split"]


def generated(fault=None):
    source = read("source/rasterizer/rasterizer_cinematics.c")
    code = "\n".join([
        function(source, "rasterizer_screen_effects_time"),
        function(source, "rasterizer_screen_effect_get_cinematic_parameters"),
    ])
    if fault:
        code = mutated(code, *fault)
    declarations = "\n".join([
        structure(read("source/rasterizer/rasterizer_cinematics.h"), "rasterizer_cinematic_screen_effect_parameters"),
        structure(source, "rasterizer_cinematic_screen_effect_state"),
    ])
    return (("declarations.inc", declarations + "\n"), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_blur(case):
    status, output = run(build("split_screen_blur", generated()), case)
    assert status == 0, output


def test_negative_control():
    # without the window check
    fault = ("if (main_get_window_count() > 1)", "if (FALSE)")
    status, output = run(build("split_screen_blur", generated(fault)), "split")
    assert status == CHECK_FAILED, output

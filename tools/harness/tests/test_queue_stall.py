"""A client machine's player whose input stalls stands still but keeps its latched buttons pressed."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run  # noqa: E402

CASES = ["stall", "held"]


def generated(fault=None):
    source = read("source/game/player_queues_new.c")
    code = function(source, "update_server_next_update")
    if fault:
        code = mutated(code, *fault)
    silence = constant(source, "DISTRIBUTED_INPUT_SILENCE_TICKS")
    return (
        ("constants.inc", f"enum {{ DISTRIBUTED_INPUT_SILENCE_TICKS = {silence} }};\n"),
        ("under_test.inc", code),
    )


@pytest.mark.parametrize("case", CASES)
def test_stall(case):
    status, output = run(build("queue_stall", generated()), case)
    assert status == 0, output


def test_negative_control():
    # every button released, as on main
    fault = ("action->control_flags &= LATCHED_CONTROL_FLAGS;", "action->control_flags = 0;")
    status, output = run(build("queue_stall", generated(fault)), "stall")
    assert status == CHECK_FAILED, output

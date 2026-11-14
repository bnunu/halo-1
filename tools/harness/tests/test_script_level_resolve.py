"""Scripted campaign destinations stay within their map family and MCC package."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import build, function, read, run


@pytest.mark.parametrize("case", ["mcc", "mcc-path", "mcc-foreign-path", "mcc-missing", "mcc-multiplayer",
                                  "ce", "ce-missing", "xbox", "empty", "long", "small-buffer", "transitions"])
def test_destination(case):
    code = function(read("port/linux/game/mcc_main.c"), "mcc_main_script_level_resolve")
    code += "\n" + function(read("source/main/main.c"), "main_script_level_resolve")
    status, output = run(build("script_level_resolve", (("under_test.inc", code),)), case)
    assert status == 0, output

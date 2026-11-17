"""A unit readying its next weapon puts away the current one only if it holds it."""

import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run  # noqa: E402

CASES = ["held", "taken", "dropped"]


def generated(fault=None):
    units = read("source/units/units.c")
    code = "\n".join([
        re.search(r"^#define unit_get_current_weapon_index\(.*$", units, re.M).group(0),
        re.search(r"^#define unit_get_desired_weapon_index\(.*$", units, re.M).group(0),
        function(units, "unit_get_weapon"),
        function(units, "unit_inventory_get_weapon"),
        function(units, "unit_ready_desired_weapon"),
    ])
    if fault:
        code = mutated(code, *fault)
    return (
        ("constants.inc", enum_with(read("source/units/units.h"), "MAXIMUM_WEAPONS_PER_UNIT =")),
        ("under_test.inc", code),
    )


@pytest.mark.parametrize("case", CASES)
def test_current_weapon(case):
    status, output = run(build("unit_current_weapon", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("case", ["taken", "dropped"])
def test_negative_control(case):
    # without the parent check
    fault = ("object_get(current_weapon_index)->object.parent_object_index!=unit_index", "FALSE")
    status, output = run(build("unit_current_weapon", generated(fault)), case)
    assert status == CHECK_FAILED, output

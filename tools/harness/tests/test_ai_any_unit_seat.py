"""AI takes a biped's seat on Custom Edition maps; on Xbox maps a seat stays a vehicle's."""

import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run  # noqa: E402

CASES = ["ce-biped", "ce-vehicle", "xbox-vehicle"]
CONTROLS = [
    ("custom_edition_cache_tags_loaded() ? _object_mask_unit : _object_mask_vehicle", "_object_mask_vehicle"),
    ("if (!allow_any_seat && vehicle->object.type == _object_type_vehicle)", "if (!allow_any_seat)"),
]


def generated(fault=None):
    code = function(read("source/ai/action_vehicle.c"), "action_vehicle_evaluate_seat")
    if fault:
        code = mutated(code, *fault)
    declarations = "\n".join([
        enum_with(read("source/objects/object_types.h"), "_object_mask_unit ="),
        re.search(r"^#define vehicle_get\(.*$", read("source/units/vehicles.h"), re.M).group(0),
    ])
    return (("declarations.inc", declarations + "\n"), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_seat(case):
    status, output = run(build("ai_any_unit_seat", generated()), case)
    assert status == 0, output


def test_xbox_biped():
    # Xbox maps keep the vehicle type check
    status, output = run(build("ai_any_unit_seat", generated()), "xbox-biped")
    assert status == CHECK_FAILED and "objects.c#1690" in output, output


@pytest.mark.parametrize("before,after", CONTROLS)
def test_negative_control(before, after):
    status, output = run(build("ai_any_unit_seat", generated((before, after))), "ce-biped")
    assert status == CHECK_FAILED, output

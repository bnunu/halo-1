"""Outside the map, a co-op client deactivates the weapons and riders it drops from a deleted unit."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run  # noqa: E402

CASES = ["outside", "inside"]
CONTROLS = [
    ("unit_network_drop_weapon(object_index, weapon_slot);\n\t\t\t\tdistributed_client_let_go(weapon_index);",
     "unit_network_drop_weapon(object_index, weapon_slot);\n\t\t\t\t(void)weapon_index;"),
    ("unit_exit_seat_end(child_index);\n\t\t\t\tdistributed_client_let_go(child_index);",
     "unit_exit_seat_end(child_index);"),
]


def generated(fault=None):
    network = read("port/linux/game/network_objects.c")
    code = "\n".join([
        function(network, "distributed_client_let_go"),
        function(network, "distributed_client_delete"),
    ])
    if fault:
        code = mutated(code, *fault)
    enums = "\n".join([
        enum_with(read("source/objects/object_types.h"), "_object_mask_unit ="),
        enum_with(read("source/objects/objects.h"), "_object_header_active_bit = 0"),
        enum_with(read("source/units/units.h"), "MAXIMUM_WEAPONS_PER_UNIT ="),
    ])
    return (("enums.inc", enums + "\n"), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_let_go(case):
    status, output = run(build("coop_drop_outside_map", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("before,after", CONTROLS)
def test_negative_control(before, after):
    status, output = run(build("coop_drop_outside_map", generated((before, after))), "outside")
    assert status == CHECK_FAILED, output

"""A camera cluster expires on a successful BSP change and survives rollback."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import build, function, read, run


@pytest.mark.parametrize("case", ["switch", "recover", "object"])
def test_activation_at_reconnect(case):
    objects = read("source/objects/objects.c")
    code = "\n".join((function(objects, "object_pvs_clear"),
                      function(objects, "objects_clear_activating_cluster"),
                      function(read("source/scenario/scenario.c"), "scenario_switch_structure_bsp")))
    status, output = run(build("bsp_activation", (("under_test.inc", code),)), case)
    assert status == 0, output

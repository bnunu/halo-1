"""The host's co-op nav points resent to its clients forget one on an object that is gone or dead, as a player's HUD
does: a marker a script left on a dead enemy flashed up on every client at each resend."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run

CASES = ['alive', 'dead', 'gone', 'flag', 'mixed', 'no-refresh']


def generated(fault=None):
    coop = read('port/linux/game/network_coop.c')
    header = read('port/linux/game/network_coop.h')
    config = enum_with(read('source/objects/objects.h'), '_object_dead_bit') + '\n'
    config += enum_with(header, '_coop_nav_point_team_flag') + '\n'
    # (by their declarations: the file names some of them earlier)
    config += enum_with(coop, '\t_coop_event_hud,') + '\n'
    config += enum_with(header, '\t_coop_hud_objective,') + '\n'
    config += enum_with(coop, '\tMAXIMUM_NAV_POINTS =') + '\n'
    state = re.search(r'^static struct\n\{\n\tshort objective;.*?^\} host_hud_state;\n', coop, re.M | re.S)
    if not state:
        raise LookupError('host_hud_state not found in the sources')
    code = function(coop, 'host_send_hud_state')
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('state.inc', state.group(0)), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('coop_nav_point_resend', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('case, fault', [
    ('dead', (' || TEST_FLAG(object->object.damage_flags, _object_dead_bit)', '')),
    ('mixed', ('nav_points[index--]', 'nav_points[index]')),
])
def test_negative_control(case, fault):
    status, output = run(build('coop_nav_point_resend', generated(fault)), case)
    assert status == CHECK_FAILED, output

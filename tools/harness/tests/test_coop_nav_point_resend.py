"""The host's co-op nav points resent to its clients skip one on an object that is gone or dead, as a player's HUD
drops it: a marker a script left on a dead enemy flashed up on every client at each resend. The nav point is kept, so
a checkpoint's revert, which brings the enemy back, brings its marker back too."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run

CASES = ['alive', 'dead', 'gone', 'flag', 'mixed', 'revert', 'full', 'no-refresh']


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
    code = '\n'.join([function(coop, 'host_nav_point_object_dead'), function(coop, 'network_coop_note_nav_point'),
        function(coop, 'host_send_hud_state')])
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('state.inc', state.group(0)), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('coop_nav_point_resend', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('case, fault', [
    ('dead', (' || TEST_FLAG(object->object.damage_flags, _object_dead_bit)', '')),
    # (the dead forgotten: a revert's enemy back without its marker)
    ('revert', ('\t\tif (host_nav_point_object_dead(index))\n\t\t\tcontinue;',
        '\t\tif (host_nav_point_object_dead(index))\n\t\t{\n'
        '\t\t\thost_hud_state.nav_points[index--] = host_hud_state.nav_points[--host_hud_state.nav_point_count];\n'
        '\t\t\tcontinue;\n\t\t}')),
    ('full', ('index < host_hud_state.nav_point_count && !host_nav_point_object_dead(index)',
        'index < host_hud_state.nav_point_count')),
])
def test_negative_control(case, fault):
    status, output = run(build('coop_nav_point_resend', generated(fault)), case)
    assert status == CHECK_FAILED, output

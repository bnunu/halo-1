"""The lobby's author line (menu_functions.c's lobby_author_line and lobby_line_fits): one line under the map's name, a name too wide cut short with "..."."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run

# a fault in menu_functions.c and the case that must catch it
NEGATIVE_CONTROLS = {
    'never-cut': (('if (!kept || lobby_line_fits(line))', 'if (1)'), 'cut'),
    'no-dots': (('\t\tif (kept < length)\n\t\t\ttext_append(line, size, L"...");\n', ''), 'cut'),
    'margin-ignored': (('<= lobby_data_box.width - MOD_TEXT_MARGIN', '<= lobby_data_box.width'), 'cut'),
    'uncounted': (('return ustrlen(line) <= LOBBY_LINE_CHARACTERS;', 'return TRUE;'), 'counted'),
    'host-item-ignored': (('if (!mcc_workshop_wire_unpack(map_name, &item))', 'if (!mcc_workshop_wire_unpack(map_name, &item) || 1)'),
                          'host-item'),
    'any-map': (('\tif (!mcc_level_name(map_name))\n\t\treturn FALSE;\n', ''), 'none'),
}

CASES = ['line', 'host-item', 'none', 'cut', 'counted']


def generated(fault=None):
    menus = read('port/linux/game/menu_functions.c')
    constants = ''.join(f'#define {name} {constant(menus, name)}\n' for name in ('MOD_TEXT_MARGIN', 'LOBBY_LINE_CHARACTERS'))
    code = ''.join(function(menus, name) + '\n' for name in ('mods_wide_text', 'text_append', 'lobby_line_fits',
                                                             'lobby_author_line'))
    if fault:
        code = mutated(code, *fault)
    return (('constants.inc', constants), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_lobby_author_line(case):
    status, output = run(build('lobby_author_line', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('control', NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build('lobby_author_line', generated(fault)), case)
    assert status == CHECK_FAILED, f'menu_functions.c with a fault ({control}) passed {case!r}: the test cannot see it'

"""Discord Account's LINK: OK on the master a refusing server needs, shown already, opens its page."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure

NAMES = ('text_is_number', 'spinner_item', 'setting_value_index', 'setting_text', 'setting_write', 'setting_load',
         'setting_changed_save')


def generated(fault=None):
    source = read('port/linux/game/menu_functions.c')
    code = '\n'.join(function(source, name) for name in NAMES)
    if fault:
        code = mutated(code, *fault)
    return (('under_test_types.inc', structure(source, 'pc_menu_setting')), ('under_test.inc', code))


@pytest.mark.parametrize('case', ['chosen_already', 'chosen'])
def test_case(case):
    status, output = run(build('link_action_menu', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [
    (('setting_value_index(setting, "none")', 'setting->loaded_index'), 'chosen_already'),
    (('player_link_begin(atoi(value + 5) - 1);', 'player_link_begin(atoi(value + 5));'), 'chosen'),
])
def test_negative_control(fault, case):
    status, output = run(build('link_action_menu', generated(fault)), case)
    assert status == CHECK_FAILED, output

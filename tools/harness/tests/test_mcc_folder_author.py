"""The help under a folder of the MCC map lists (menu_functions.c's mcc_folder_help_text): a Workshop item's folder ends with its author."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run, structure

# a fault in menu_functions.c and the case that must catch it
NEGATIVE_CONTROLS = {
    'collections-too': (('!browser->folders[row] || browser->groups[row] != NONE ||', '!browser->folders[row] ||'),
                        'collection'),
    'map-rows-too': (('row >= browser->count || !browser->folders[row] ||', 'row >= browser->count ||'), 'map-row'),
    'any-row': (('if (row < 0 || row >= browser->count || ', 'if ('), 'empty'),
    'never-shown': (('\ttext_append(help, size, wide);\n', ''), 'item'),
}

CASES = ['item', 'not-yet', 'collection', 'package', 'map-row', 'empty', 'fits']


def generated(fault=None):
    menus = read('port/linux/game/menu_functions.c')
    maps = read('port/linux/game/mcc_maps.h')
    constants = ''.join(f'#define {name} {constant(source, name)}\n' for source, name in (
        (maps, 'MCC_MAPS_MAXIMUM'), (read('port/linux/src/workshop_collections.h'), 'WORKSHOP_COLLECTION_MAXIMUM_PER_ITEM'),
        (read('port/linux/src/workshop_authors.h'), 'WORKSHOP_AUTHORS_NAME_SIZE'), (menus, 'ROW_TEXT_LENGTH')))
    constants += '#define MCC_FOLDER_HELP_LENGTH (ROW_TEXT_LENGTH * 3)\n'
    assert '#define MCC_FOLDER_HELP_LENGTH (ROW_TEXT_LENGTH * 3)' in menus
    code = ''.join(function(menus, name) + '\n' for name in ('mods_wide_text', 'text_append', 'mcc_folder_help_text'))
    if fault:
        code = mutated(code, *fault)
    return (('constants.inc', constants), ('types.inc', structure(maps, 'mcc_map_browser')), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_mcc_folder_author(case):
    status, output = run(build('mcc_folder_author', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('control', NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build('mcc_folder_author', generated(fault)), case)
    assert status == CHECK_FAILED, f'menu_functions.c with a fault ({control}) passed {case!r}: the test cannot see it'

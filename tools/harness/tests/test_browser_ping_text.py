"""The server browser's ping cell (menu_functions.c's browser_ping_text)."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run, structure


def generated(fault=None):
    header = read('port/linux/src/p2p.h')
    menus = read('port/linux/game/menu_functions.c')
    code = function(menus, 'browser_ping_dots') + function(menus, 'browser_ping_text')
    if fault:
        code = mutated(code, *fault)
    return (('listing.inc', enum_with(header, 'P2P_LISTING_NAME_SIZE') + structure(header, 'p2p_listing')),
            ('words.inc', enum_with(menus, '_browser_word_of')),
            ('under_test.inc', code))


@pytest.mark.parametrize('case', ['number', 'refresh', 'dots', 'status', 'dash'])
def test_case(case):
    status, output = run(build('browser_ping_text', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [
    (('now / 300', 'now / 600'), 'dots'),
    (('return game->probing &&', 'return game->open && !game->in_progress && game->probing &&'), 'dots'),
    (('if (game->failed)', 'if (0)'), 'status'),
    (('game->ping < 0 || game->refreshing', 'game->ping < 0 || 1'), 'number'),
    (('game->ping < 0 || game->refreshing', 'game->ping < 0'), 'refresh'),
])
def test_negative_control(fault, case):
    status, output = run(build('browser_ping_text', generated(fault)), case)
    assert status == CHECK_FAILED, output

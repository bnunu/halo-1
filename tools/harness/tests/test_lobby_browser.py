"""The server browser's choice and order of the games found: its source (ALL GAMES, DEDICATED, P2P GAMES), its
Filters (empty, full, game type, missing maps) and its column titles' orders, both ways; a game of another network
version kept, last (port/linux/src/p2p_lobby.c's p2p_lobby_filter and p2p_lobby_sort)."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, mutated, read, run, structure


def generated(fault=None):
    header = read('port/linux/src/p2p.h')
    version = constant(read('port/linux/include/halo_port_limits.h'), 'HALO_PORT_NETWORK_VERSION')
    enums = '\n'.join([f'#define HALO_PORT_NETWORK_VERSION {version}',
                       enum_with(header, 'P2P_LISTING_NAME_SIZE = 32'),
                       enum_with(header, 'P2P_LOBBY_SOURCE_ALL'),
                       enum_with(header, 'P2P_LOBBY_MAPS_YES'),
                       enum_with(header, 'P2P_LOBBY_MAP_INSTALLED'),
                       enum_with(header, 'P2P_LOBBY_SORT_DEDICATED'),
                       structure(header, 'p2p_listing'),
                       structure(header, 'p2p_lobby_choices')]) + '\n'
    source = read('port/linux/src/p2p_lobby.c')
    names = re.search(r'static const char \*const sort_names\[NUMBER_OF_P2P_LOBBY_SORTS\] =.*?\};', source,
                      re.S).group(0)
    code = '\n'.join([names, function(source, 'p2p_lobby_filter'), function(source, 'text_order'),
                      function(source, 'sort_order'), function(source, 'p2p_lobby_sort'),
                      function(source, 'p2p_lobby_sort_parse'), function(source, 'p2p_lobby_sort_text')])
    if fault:
        code = mutated(code, *fault)
    return (('enums.inc', enums), ('under_test.inc', code))


CASES = ['source', 'empty_full', 'game_type', 'maps', 'dedicated_first', 'sort_each_way', 'other_version',
         'sort_text']


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('lobby_browser', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [
    (('(choices->source == P2P_LOBBY_SOURCE_PLAYERS && game->dedicated)',
      '(choices->source == P2P_LOBBY_SOURCE_PLAYERS && !game->dedicated)'), 'source'),
    (('game->player_count >= game->maximum_player_count', 'game->player_count > game->maximum_player_count'),
     'empty_full'),
    (('state == P2P_LOBBY_MAP_MISSING))', 'state != P2P_LOBBY_MAP_INSTALLED))'), 'maps'),
    (('difference = second->dedicated - first->dedicated;', 'difference = 0;'), 'dedicated_first'),
    (('(reverse ? -difference : difference) <= 0', 'difference <= 0'), 'sort_each_way'),
    (('before_other != other_version ? !before_other :', 'before_other != other_version ? before_other :'),
     'other_version'),
    (('*reverse = text && *text == \'-\';', '*reverse = 0;'), 'sort_text'),
    (('\treturn P2P_LOBBY_SORT_PLAYERS;', '\treturn P2P_LOBBY_SORT_DEDICATED;'), 'sort_text'),
    (('"dedicated", "players",', '"servers", "players",'), 'sort_text'),
])
def test_negative_control(fault, case):
    status, output = run(build('lobby_browser', generated(fault)), case)
    assert status == CHECK_FAILED, output

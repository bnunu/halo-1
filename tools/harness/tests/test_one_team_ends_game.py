"""A team game with only one team's players in it ends (the Xbox game's rule: game_engine_should_end_game) unless the
host's setting is off: a player's host's network.one_team_ends_game (on by default), a dedicated server's
server.one_team_ends_game (off). Off, it goes on, as when it starts before the other team's players have loaded or they
leave, while a team out of lives still ends it (the other's, or the only one's: no one left who can play), and so
does the time limit."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run

CASES = ['stock-one-team', 'one-team-goes-on', 'other-team-left', 'out-of-lives-ends', 'both-teams-go-on',
         'one-player', 'time-limit', 'server-goes-on', 'server-stock', 'server-out-of-lives-ends',
         'sole-team-out-of-lives', 'out-of-lives-after-quit', 'infection']


def generated(fault=None):
    engine = read('source/game/game_engine.c')
    code = '\n'.join([function(engine, name) for name in ('multiple_teams_alive', 'game_engine_one_team_ends_game',
        'game_engine_multiple_teams_playing', 'game_engine_any_player_alive',
        'game_engine_should_end_game')])
    if fault:
        code = mutated(code, *fault)
    return (('under_test.inc', code),)


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('one_team_ends_game', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('case, fault', [
    # (the settings not read: the Xbox game's rule always)
    ('one-team-goes-on', ('(game_engine_one_team_ends_game() || game_engine_multiple_teams_playing() ||', '(1 ||')),
    ('server-goes-on', ('(game_engine_one_team_ends_game() || game_engine_multiple_teams_playing() ||', '(1 ||')),
    # (the rule dropped altogether: a team out of lives goes on)
    ('out-of-lives-ends', ('game_engine_one_team_ends_game() || game_engine_multiple_teams_playing() ||',
        'game_engine_one_team_ends_game() ||')),
    # (a dedicated server going by the player host's setting)
    ('server-stock', ('config_boolean("server.one_team_ends_game")', 'config_boolean("network.one_team_ends_game")')),
    # (no one left who can play not ending it: the only team out of lives, alone or after the other quit)
    ('sole-team-out-of-lives', (' ||\n\t\t\t!game_engine_any_player_alive()', '')),
    ('out-of-lives-after-quit', (' ||\n\t\t\t!game_engine_any_player_alive()', '')),
    # (players who quit counted as playing)
    ('other-team-left', ('\t\tif (player->quit_out_of_game)\n\t\t\tcontinue;\n', '')),
])
def test_negative_control(case, fault):
    status, output = run(build('one_team_ends_game', generated(fault)), case)
    assert status == CHECK_FAILED, output

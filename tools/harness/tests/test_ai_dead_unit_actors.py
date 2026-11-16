"""An actor whose unit is dead without unit_died (a region that forces death) dies with it, so its encounter's count
falls: a feigned death and a player's unit are left alone."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run

CASES = ['dead', 'braindead', 'alive', 'feigning', 'player', 'other-actor', 'swarm', 'erase']


def generated(fault=None):
    actors = read('source/ai/actors.c')
    config = enum_with(read('source/objects/objects.h'), '_object_dead_bit') + '\n'
    config += enum_with(actors, '_actor_mode_braindead') + '\n'
    code = '\n'.join([function(actors, 'actor_port_unit_dead'), function(actors, 'actors_update')])
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('ai_dead_unit_actors', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('case, fault', [
    ('dead', ('else if (actor_port_unit_dead(iterator.index))', 'else if (FALSE)')),
    ('feigning', (' || unit->unit.feign_death_timer > 0', '')),
    ('player', (' ||\n\t\tunit->unit.player_index != NONE', ' ||\n\t\tFALSE')),
])
def test_negative_control(case, fault):
    status, output = run(build('ai_dead_unit_actors', generated(fault)), case)
    assert status == CHECK_FAILED, output

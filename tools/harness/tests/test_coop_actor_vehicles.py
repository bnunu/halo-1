"""A network game's client drives the host's recording-driven vehicles with the host's control, as its actors' bipeds."""
import re
import sys
from pathlib import Path
import pytest
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import build, read, run, function, structure, mutated, CHECK_FAILED


def generated(fault=None):
    source = read('port/linux/game/network_actors.c')
    held = re.search(r'CONTROL_HELD_TICKS = [^,\n]+', source).group(0)
    drive = function(source, 'actor_unit_valid') + '\n' + function(source, 'client_actor_release') + '\n' + \
        function(source, 'network_actors_drive')
    if fault == 'vehicles-skipped':
        # (as before: a vehicle neither marked nor driven)
        drive = mutated(drive, '\t\tif (!actor->driven)\n',
                        '\t\tif (object_get(actor->unit_index)->object.type == _object_type_vehicle)\n'
                        '\t\t\tcontinue;\n\t\tif (!actor->driven)\n')
    if fault == 'not-possessed':
        # (driven, but its idle driver's throttle taken over the recording's)
        drive = mutated(drive, 'unit_set_possessed(actor->unit_index, TRUE);', '(void)0;')
    if fault == 'kept-possessed':
        drive = mutated(drive, 'unit_set_possessed(actor->unit_index, FALSE);', '(void)0;')
    if fault == 'not-kept-active':
        # (a cutscene's vehicle far from the players left inactive: its rotors still)
        drive = mutated(drive, 'object_set_automatic_deactivation(actor->unit_index, FALSE);', '(void)0;')
    if fault == 'kept-active':
        drive = mutated(drive, 'object_set_automatic_deactivation(actor->unit_index, TRUE);', '(void)0;')
    if fault == 'driven-outside':
        # (out of the map, flown by itself from wherever it last was)
        drive = mutated(drive, 'if (vehicle && TEST_FLAG(object_get(actor->unit_index)->object.flags, '
                        '_object_outside_of_map_bit))', 'if (FALSE)')
    return (('client_actor.inc', 'enum { ' + held + ' };\n' + structure(source, 'client_actor')),
            ('drive.inc', drive))


@pytest.mark.parametrize('case', ['vehicle', 'biped', 'released', 'outside', 'leaves'])
def test_case(case):
    status, output = run(build('coop_actor_vehicles', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', [('vehicles-skipped', 'vehicle'), ('not-possessed', 'vehicle'),
                                        ('kept-possessed', 'released'), ('not-kept-active', 'vehicle'),
                                        ('kept-active', 'released'), ('driven-outside', 'outside'),
                                        ('driven-outside', 'leaves')])
def test_negative_control(fault, case):
    status, output = run(build('coop_actor_vehicles', generated(fault)), case)
    assert status == CHECK_FAILED, output

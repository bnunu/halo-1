"""The first-person weapon drawn between ticks: its own animations blended however far they move in a tick (a
rocket launcher's melee), another weapon's or skeleton's pose never blended to, and a node's half turn snapped."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure

CASES = ['melee', 'weapon', 'graph', 'within a tick', 'gap', 'not a number', 'half turn', 'large turn']
CONTROLS = [
    # the distance limit the weapon's identity replaced
    ('<= REAL_MAX))', '<= 0.25f * 0.25f))', 'melee'),
    ('first_person->node_count == node_count &&\n\t\t\tfirst_person->weapon_index == weapon_index &&',
     'first_person->node_count == node_count &&', 'weapon'),
    ('first_person->weapon_index == weapon_index &&\n\t\t\tfirst_person->animation_graph_index == animation_graph_index;',
     'first_person->weapon_index == weapon_index;', 'graph'),
    ('if (first_person->tick != interpolation_tick ||\n\t\tfirst_person->weapon_index != weapon_index ||\n'
     '\t\tfirst_person->animation_graph_index != animation_graph_index)',
     'if (first_person->tick != interpolation_tick)', 'within a tick'),
    ('first_person->has_previous = first_person->tick == interpolation_tick - 1 &&',
     'first_person->has_previous = ', 'gap'),
    ('< FIRST_PERSON_HALF_TURN_COSINE)', '< 0.0f)', 'half turn'),
]


def generated(fault=None):
    source = read('port/linux/game/render_interpolation.c')
    maths = read('source/math/real_math.h')
    matrices = read('source/math/matrix_math.c')
    config = '\n'.join([
        '#define MAXIMUM_LOCAL_PLAYERS 4',
        re.search(r'^#define MAXIMUM_INTERPOLATED_NODES .*', source, re.M).group(),
        re.search(r'^#define FIRST_PERSON_HALF_TURN_COSINE .*', source, re.M).group(),
        re.search(r'^#define REAL_MAX .*', maths, re.M).group(),
        structure(maths, 'real_matrix4x3'),
        'typedef struct real_matrix4x3 real_matrix4x3;',
    ]) + '\n'
    code = '\n'.join(
        [function(matrices, name) for name in
         ['matrix4x3_inverse', 'matrix4x3_rotation_from_vectors', 'matrix4x3_from_point_and_vectors',
          'matrix4x3_multiply']] +
        [structure(source, name) for name in
         ['interpolation_quaternion', 'interpolation_rotation', 'interpolated_first_person']] +
        ['static struct interpolated_first_person interpolated_first_person[MAXIMUM_LOCAL_PLAYERS];'] +
        [function(source, name) for name in
         ['lerp', 'point_lerp', 'vector_length', 'basis_is_rotation', 'quaternion_from_basis',
          'basis_from_quaternion', 'rotation_from_matrix', 'matrix_blend_rotations', 'distance_squared',
          'render_interpolation_first_person']])
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('first_person_interpolation', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('before,after,case', CONTROLS)
def test_negative_control(before, after, case):
    status, output = run(build('first_person_interpolation', generated((before, after))), case)
    assert status == CHECK_FAILED, output

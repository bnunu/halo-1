"""The real pose_jumps (render_interpolation.c): an object's pose taken for a new one, not blended to, when a node
jumps in its root's frame, from rest or turning back; a node moving steadily, however fast (a30's lifepod's airbrakes
flying off as it falls), blended, so that the object and a camera that follows it are drawn between the ticks."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure

CASES = ['still', 'jump', 'steady', 'reversal', 'not a number']
CONTROLS = [
    # every fast node a jump (the rule before)
    ('if (!moves_valid ||', 'if (TRUE ||', 'steady'),
    # the change allowed no more than the snap distance, however fast the node
    ('MAX(NODE_SNAP_DISTANCE * NODE_SNAP_DISTANCE, 0.25f * before_squared)',
     'NODE_SNAP_DISTANCE * NODE_SNAP_DISTANCE', 'steady'),
    # the change not looked at
    ('!(change.i * change.i + change.j * change.j + change.k * change.k <= allowed_squared)', 'FALSE', 'reversal'),
    # the moves not kept for the next tick
    ('\t\tmoves[node_index] = move;\n', '', 'steady'),
    # a node not a number no jump
    ('!(move.i * move.i + move.j * move.j + move.k * move.k <= NODE_SNAP_DISTANCE * NODE_SNAP_DISTANCE)',
     '(move.i * move.i + move.j * move.j + move.k * move.k > NODE_SNAP_DISTANCE * NODE_SNAP_DISTANCE)',
     'not a number'),
]


def generated(fault=None):
    source = read('port/linux/game/render_interpolation.c')
    maths = read('source/math/real_math.h')
    matrices = read('source/math/matrix_math.c')
    config = '\n'.join([
        re.search(r'^#define NODE_SNAP_DISTANCE .*', source, re.M).group(),
        structure(maths, 'real_matrix4x3'),
        'typedef struct real_matrix4x3 real_matrix4x3;',
    ]) + '\n'
    code = '\n'.join([function(matrices, 'matrix4x3_inverse_transform_point'), function(source, 'pose_jumps')])
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('under_test.inc', code))


def test_tick_finds_it():
    """each object's pose is judged once a tick, as its latest snapshot is taken, and a frame snaps on it."""
    source = read('port/linux/game/render_interpolation.c').replace('\r\n', '\n')
    tick = function(source, 'render_interpolation_tick')
    taken = tick.index('object_get_node_matrices(iterator.index)')
    assert tick.index('record->pose_jumps = continuing && pose_jumps(', taken) > taken
    assert 'record->moves_valid = continuing;' in tick
    frame = function(source, 'render_interpolation_object_node_matrices')
    assert 'boolean snap = record->pose_jumps ||' in frame


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('pose_jumps', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('before, after, case', CONTROLS)
def test_negative_control(before, after, case):
    status, output = run(build('pose_jumps', generated((before, after))), case)
    assert status == CHECK_FAILED, f'pose_jumps with ({before[:40]}) passed {case}'

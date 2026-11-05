"""The desktop visibility readback asks the driver for nothing before the swap (a GL error, a fence's state), and at
most one question after it, so that a threaded driver need not catch up with the frame just drawn before it is
shown. Each batch's counts are taken once the fence after their copies has passed; a GPU however late is waited for,
never given up on; a copy that failed is seen by the place it left uncopied, without asking for GL errors. Each test
is numbered at its slot as it ends, dropped ones too, and a slot's count is given as the test's it was read from."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run, structure

CASES = ['quiet', 'late', 'lost_copy', 'fence_failed', 'logging', 'numbered']
SOURCE = 'port/linux/src/d3d8_gl.c'

# a fault and the case that must catch it
NEGATIVE_CONTROLS = [
    # GL errors asked for before every swap
    (('if (device.visibility_logging)\n\t\t\tvisibility_readback_gl_ok("before query copies");',
      'visibility_readback_gl_ok("before query copies");'), 'quiet'),
    # the batch just sent asked about too: two questions a present
    (('if (batch->frame == device.frame)\n\t\t\tbreak;', 'if (0)\n\t\t\tbreak;'), 'quiet'),
    # a late GPU's batches given up after a while
    (('if (status == GL_TIMEOUT_EXPIRED)\n',
      'if (status == GL_TIMEOUT_EXPIRED && device.frame - batch->frame < 240)\n'), 'late'),
    # a copy that wrote nothing taken for a count
    (('if (samples == VISIBILITY_UNCOPIED)', 'if (0)'), 'lost_copy'),
    # a fence waited on, counting on the swap to have sent it
    (('glClientWaitSync(batch->fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0)', 'glClientWaitSync(batch->fence, 0, 0)'),
     'quiet'),
    # the buffer read on after a lost copy, whose batch is not used again (with all three lost, no more counts)
    (('if (fall_back)\n\t\tvisibility_fall_back_to_queries();', 'if (fall_back)\n\t\tfall_back = FALSE;'),
     'lost_copy'),
    # a failed fence taken for a passed one
    (('if (status != GL_ALREADY_SIGNALED && status != GL_CONDITION_SATISFIED)', 'if (0)'), 'fence_failed'),
    # a count read without the number of its test
    (('device.visibility_counted[batch->slots[test]] = batch->tests[test];', ''), 'numbered'),
    # a slot's count given as its latest test's as that test ends, its count still to come
    (('batch->tests[batch->count] = test;',
      'batch->tests[batch->count] = test;\n\t\tdevice.visibility_counted[index] = test;'), 'numbered'),
]


def generated(fault=None):
    source = read(SOURCE)
    config = '#define VISIBILITY_TEST_SLOTS %d\n#define VISIBILITY_BATCH_COUNT %d\n#define VISIBILITY_ALL_SAMPLES %d\n' \
        '%s;\n' % (constant(source, 'VISIBILITY_TEST_SLOTS'), constant(source, 'VISIBILITY_BATCH_COUNT'),
                   constant(source, 'VISIBILITY_ALL_SAMPLES'), structure(source, 'visibility_batch'))
    code = '\n'.join(function(source, name) for name in (
        'visibility_readback_gl_ok', 'visibility_fall_back_to_queries', 'visibility_send_batch',
        'visibility_read_batch', 'D3DDevice_BeginVisibilityTest', 'D3DDevice_EndVisibilityTest',
        'D3DDevice_GetVisibilityTestResult', 'xgpu_visibility_tests_ended', 'xgpu_visibility_test_counted'))
    if fault:
        code = mutated(code, *fault)
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('visibility_readback', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('fault,case', NEGATIVE_CONTROLS)
def test_negative_control(fault, case):
    status, output = run(build('visibility_readback', generated(fault)), case)
    assert status == CHECK_FAILED, output

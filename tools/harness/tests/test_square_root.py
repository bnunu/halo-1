"""square_root (source/math/real_math.h) in single precision gives the bits the game's double-precision root rounded
back to a float gave, on a compiler with the elementwise builtin and on one without it."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, inline, mutated, read, run

CASES = ['every exponent', 'squares']
# a compiler without the builtin: the double root, as before
FALLBACK = ('#ifdef __has_builtin', '#ifdef __harness_no_builtin')
# a root a rounding off: the reciprocal root, multiplied back
APPROXIMATE = ('return __builtin_elementwise_sqrt(x);', 'return x * (1.0f / __builtin_elementwise_sqrt(x));')
CONTROLS = [
    ((APPROXIMATE,), 'every exponent'),
    ((APPROXIMATE,), 'squares'),
    # the fallback's root of a float a rounding off
    ((FALLBACK, ('return (real)sqrt(x);', 'return (real)sqrt(x * 1.0000001f);')), 'every exponent'),
]


def generated(*faults):
    code = inline(read('source/math/real_math.h'), 'square_root')
    for fault in faults:
        code = mutated(code, *fault)
    return (('under_test.inc', code),)


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('square_root', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('case', CASES)
def test_fallback(case):
    status, output = run(build('square_root', generated(FALLBACK)), case)
    assert status == 0, output


@pytest.mark.parametrize('faults,case', CONTROLS)
def test_negative_control(faults, case):
    status, output = run(build('square_root', generated(*faults)), case)
    assert status == CHECK_FAILED, output

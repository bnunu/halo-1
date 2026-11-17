"""`DATUM_INDEX_TO_ABSOLUTE_INDEX` is a long in printf-style arguments under the game's 32-bit ABI."""

import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, mutated, read, run  # noqa: E402

CASES = ["size", "varargs"]


def generated(fault=None):
    source = read("source/cseries/cseries.h")
    macro = re.search(r"^#define DATUM_INDEX_TO_ABSOLUTE_INDEX\(.*$", source, re.M).group(0)
    code = enum_with(source, "UNSIGNED_LONG_MAX =") + "\n" + macro + "\n"
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", code),)


@pytest.mark.parametrize("case", CASES)
def test_width(case):
    status, output = run(build("datum_index_width", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("case", CASES)
def test_negative_control(case):
    # the macro without the cast
    fault = ("((long)((datum_index)&UNSIGNED_SHORT_MAX))", "((datum_index)&UNSIGNED_SHORT_MAX)")
    status, output = run(build("datum_index_width", generated(fault)), case)
    assert status == CHECK_FAILED, output

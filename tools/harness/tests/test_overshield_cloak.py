"""The real overshield_effect_intensity (overshield_effect.c): the overshield as display.overshield_effect draws it,
lit whole, showed a cloaked player's sniper rifle or rocket launcher (a unit's weapon is drawn with its values) where
the weapon reached past the body. It is as faint as the cloak is strong; under it the body still shows the shield's
own flares, as the stock game does."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run  # noqa: E402

CASES = ["uncloaked", "cloaked", "partly cloaked"]

# a fault in the code and the case that must catch it
NEGATIVE_CONTROLS = {
    # lit whole through the cloak (as before)
    "body-through": (("MAX(own, lit * uncloaked)", "MAX(own, lit)"), "cloaked"),
    "arms-through": (("arms ? lit * uncloaked", "arms ? lit"), "cloaked"),
    # the shield's own flare hidden under the cloak
    "own-hidden": (("MAX(own, lit * uncloaked)", "lit * uncloaked"), "cloaked"),
}


def generated(fault=None):
    source = read("port/linux/game/overshield_effect.c").replace("\r\n", "\n")
    code = function(source, "overshield_effect_intensity") + "\n"
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", code),)


def test_drawn_with_it():
    """The values the body and the arms are drawn with take their intensity from it, with the unit's cloak."""
    source = read("port/linux/game/overshield_effect.c").replace("\r\n", "\n")
    values = function(source, "overshield_effect_values")
    assert "copy[source] = overshield_effect_intensity(values[source], 1.f, overshield_effect_uncloaked(unit_index), FALSE);" in values
    arms = function(source, "overshield_effect_arms_values")
    assert "arms[source] = overshield_effect_intensity(values[source]," in arms
    assert "overshield_effect_uncloaked(unit_index), TRUE);" in arms
    uncloaked = function(source, "overshield_effect_uncloaked")
    assert "PIN(1.f - unit->unit.active_camouflage, 0.f, 1.f)" in uncloaked


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("overshield_cloak", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("overshield_cloak", generated(fault)), case)
    assert status == CHECK_FAILED, f"the cloaked overshield with a fault ({control}) passed '{case}'"

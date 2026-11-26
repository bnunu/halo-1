"""The shield's flare on the first-person arms (port/linux/game/viewmodel_shield.c, display.viewmodel_shield): the
unit's modifier on the hands only, its plasma offset kept to a share of the view at the arms' distance and the
viewmodel's angle, a copy per local player, and the overshield's arms copy (overshield_effect.c)."""
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run, structure

CASES = ["off", "on", "others", "limit", "distance", "split", "overshield",
         "rasterizer-limit", "rasterizer-stock", "toggle", "replace-player", "color"]
CONTROLS = {
    # the setting ignored: the hands always get the shield
    "always-on": (("if (!viewmodel_shield_enabled() || local_player_index", "if (local_player_index"), "off"),
    # every plasma clamped, the weapon's and the body's too (the old code's fault)
    "clamps-everything": (("if (values == viewmodel_shields[index].values)", "if (TRUE)"), "others"),
    # a fixed limit, whatever the viewmodel's angle
    "fov-ignored": (("(real)tan(0.5 * (double)vertical_field_of_view)", "1.0f"), "limit"),
    # the root, at the eye, counted in the arms' distance
    "root-counted": (("if (node->parent_node_index == NONE)\n\t\t\tcontinue;", ""), "distance"),
    # only the first local player's arms known again
    "one-view": (("values && index < MAXIMUM_LOCAL_PLAYERS", "values && index < 1"), "split"),
    # the body's height fade on the arms
    "arms-faded": (("if (values == effect->body_values && effect->top", "if (effect->top"), "overshield"),
    # an overshield that is gone still drawn on the arms
    "stale-arms": (("overshield_effects[index].arms_values = NULL;", ";"), "overshield"),
    # the renderer no longer calls the arm-only cap after calculating displacement
    "missing-rasterizer-hook": (("offset = viewmodel_shield_offset_amount(runtime->exponents, offset);", ""),
                                "rasterizer-limit"),
}


def plasma_calculation():
    """The rasterizer's actual runtime-value calculation, before GPU submission."""
    source = read("source/rasterizer/xbox/rasterizer_xbox_plasma_energy.c")
    draw = function(source, "rasterizer_plasma_energy_draw")
    start = draw.index("tint = global_real_rgb_white;")
    end = draw.index("perpendicular_color = plasma->perpendicular_color;", start)
    return "\n".join([
        structure(source, "plasma_runtime_parameters"),
        structure(source, "shader_transparent_plasma_definition"),
        """static real plasma_offset(real const *values, real amount, real exponent, short offset_source)
{
    struct shader_transparent_plasma_definition definition = {0};
    struct shader_transparent_plasma_definition const *plasma = &definition;
    struct plasma_runtime_parameters parameters = {NULL, values};
    struct { struct plasma_runtime_parameters const *runtime_parameters; } packet = {&parameters};
    struct plasma_runtime_parameters const *runtime;
    real_rgb_color const *global_real_rgb_white = NULL;
    real_rgb_color const *tint;
    real intensity, offset;
    short source;
    """,
        "    definition.offset_amount = amount; definition.offset_exponent = exponent;",
        "    definition.offset_exponent_source = offset_source;",
        draw[start:end].replace("group->runtime_parameters", "packet.runtime_parameters"),
        "    (void)tint; (void)intensity; return offset;\n}",
    ])


def define(source, name):
    match = re.search(r"^#define " + re.escape(name) + r" .*$", source, re.M)
    if not match:
        raise LookupError(f"#define not found in the sources: {name}")
    return match.group(0)


def per_player_state(source):
    """The local players' copies, as the module declares them."""
    end = source.index("} viewmodel_shields[MAXIMUM_LOCAL_PLAYERS];")
    start = source.rindex("static struct\n{", 0, end)
    return source[start:end] + "} viewmodel_shields[MAXIMUM_LOCAL_PLAYERS];"


def generated(control=None):
    shield = read("port/linux/game/viewmodel_shield.c")
    overshield = read("port/linux/game/overshield_effect.c")
    types = "\n".join([enum_with(overshield, "OVERSHIELD_EFFECT_MAXIMUM")] +
                      [define(overshield, name) for name in ["OVERSHIELD_EFFECT_ENERGY_SPEED", "OVERSHIELD_EFFECT_EDGE",
                                                             "OVERSHIELD_EFFECT_ARMS_START",
                                                             "OVERSHIELD_EFFECT_ARMS_LENGTH"]] +
                      [structure(overshield, "overshield_effect")])
    code = "\n".join([define(shield, "VIEWMODEL_SHIELD_STAND_OFF"), per_player_state(shield)] +
                     [function(overshield, name) for name in ["overshield_effect_intensity", "overshield_effect_arms_values",
                                                           "overshield_effect_find_values", "overshield_effect_draw"]] +
                     [function(shield, name) for name in ["viewmodel_shield_enabled", "viewmodel_shield_arms_distance",
                                                          "viewmodel_shield_maximum_offset", "viewmodel_shield_apply",
                                                          "viewmodel_shield_offset_amount"]] +
                    [plasma_calculation()])
    if control:
        code = mutated(code, *CONTROLS[control][0])
    return (("overshield_types.inc", types), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("viewmodel_shield", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", CONTROLS)
def test_negative_control(control):
    status, output = run(build("viewmodel_shield", generated(control)), CONTROLS[control][1])
    assert status == CHECK_FAILED, f"{control} escaped its check: {output}"


def hands_hook(source):
    """Keep the actual hook between the weapon draw and the hands draw."""
    draw = function(source, "first_person_weapon_draw")
    weapon, hands = list(re.finditer(r"\brender_model\(", draw))
    call = re.search(r"viewmodel_shield_apply\(\s*render\.local_player_index,\s*unit_index,\s*"
                     r"first_person_interface->hands\.index,\s*model_node_matrices,\s*unit_functions,\s*&model_effect\);", draw)
    assert call and weapon.start() < call.start() < hands.start()
    assert "viewmodel_shield_apply" not in draw[:weapon.start()]
    assert "&model_effect" in draw[hands.start():]


def test_hands_hook():
    hands_hook(read("source/interface/first_person_weapons.c"))


def test_missing_hands_hook_is_detected():
    source = read("source/interface/first_person_weapons.c")
    source = source.replace("viewmodel_shield_apply(", "removed_shield_apply(")
    with pytest.raises(AssertionError):
        hands_hook(source)

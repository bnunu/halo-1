"""The real lens flare occlusion bookkeeping (rasterizer_lights.c) over a fake GPU that reads counts two frames late:
each flare is read its own test's count, wherever it is in the frame's list, and a slot just taken is not read until
the flare's own counts are."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run, structure  # noqa: E402

CASES = ["own-counts", "stable-slots", "settles"]
FUNCTIONS = ("lens_flare_parameters_get", "lens_flare_occlusion_test_results_get", "lens_flare_occlusion_test_index",
             "lens_flare_occlusion_visible_pixels", "rasterizer_lights_begin_for_new_frame")

# a fault (in config.inc or a function) and the case that must catch it
NEGATIVE_CONTROLS = {
    # tested and read at the flare's place in the frame's list: the code before the slots
    "tested-by-place": (("\tshort probe;\n\n", "\tshort probe;\n\n\treturn lens_flare_index;\n"),
                        "lens_flare_occlusion_test_index", "own-counts"),
    # read by the flare's place in the frame's list, its test in its slot
    "read-by-place": (("return rasterizer_widget_get_occlusion_test_result(test_index);",
                       "return rasterizer_widget_get_occlusion_test_result(lens_flare_index);"),
                      "lens_flare_occlusion_visible_pixels", "own-counts"),
    # a flare never finds the slot it had
    "slot-not-kept": (("if (slot->visibility==visibility && slot->light_identifier==light_identifier)",
                       "if (FALSE)"), "lens_flare_occlusion_test_index", "stable-slots"),
    # a slot just taken read at once
    "no-settling": (("LENS_FLARE_OCCLUSION_SLOT_SETTLE_FRAMES = 4", "LENS_FLARE_OCCLUSION_SLOT_SETTLE_FRAMES = 0"),
                    "config", "settles"),
}


def generated(fault=None, where=None):
    source = read("source/rasterizer/rasterizer_lights.c")
    config = "\n".join([
        enum_with(source, "_lens_flare_first_person_weapon_flag"),
        enum_with(source, "_lens_flare_dynamic_light_flag"),
        enum_with(source, "LENS_FLARE_OCCLUSION_SLOT_COUNT"),
        structure(source, "lens_flare_occlusion_test_results"),
        structure(source, "rasterizer_lens_flare_submit_parameters"),
        structure(source, "lens_flare_occlusion_slot"),
    ]) + "\n"
    if fault and where == "config":
        config = mutated(config, *fault)
    code = ""
    for name in FUNCTIONS:
        text = function(source, name)
        if fault and where == name:
            text = mutated(text, *fault)
        code += text + "\n"
    return (("config.inc", config), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("lens_flare_occlusion", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, where, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("lens_flare_occlusion", generated(fault, where)), case)
    assert status == CHECK_FAILED, f"the occlusion code with a fault ({control}) passed '{case}': the test cannot see it"

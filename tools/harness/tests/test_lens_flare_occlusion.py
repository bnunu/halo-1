"""The real lens flare occlusion bookkeeping (rasterizer_lights.c) over a fake GPU that numbers each index's tests and
reads their counts frames late, later while it is behind, never for tests it drops: each flare is read its own test's
count, wherever it is in the frame's list and in whichever window, a slot just taken is not read until a count of the
new flare's own tests has come, and a flare that finds no slot keeps its visibility."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, enum_with, function, mutated, read, run, structure  # noqa: E402

CASES = ["own-counts", "stable-slots", "reused-late", "reused-dropped", "no-slot", "split-screen", "new-map",
         "occlusion-off"]
FUNCTIONS = ("lens_flare_parameters_get", "lens_flare_occlusion_test_results_get", "lens_flare_occlusion_test_index",
             "lens_flare_occlusion_visible_pixels", "rasterizer_lights_begin_for_new_frame",
             "rasterizer_lights_reset_for_new_map")

# a fault (one or more changes, each to config.inc or a function) and the case that must catch it
NEGATIVE_CONTROLS = {
    # tested and read at the flare's place in the frame's list: the code before the slots
    "tested-by-place": ((("lens_flare_occlusion_test_index", "\tshort probe;\n\n",
                          "\tshort probe;\n\n\treturn lens_flare_index;\n"),), "own-counts"),
    # read by the flare's place in the frame's list, its test in its slot
    "read-by-place": ((("lens_flare_occlusion_visible_pixels",
                        "visible_pixels= rasterizer_widget_get_occlusion_test_result(test_index);",
                        "visible_pixels= rasterizer_widget_get_occlusion_test_result(lens_flare_index);"),),
                      "own-counts"),
    # a flare never finds the slot it had
    "slot-not-kept": ((("lens_flare_occlusion_test_index",
                        "if (slot->visibility==visibility && slot->light_identifier==light_identifier)",
                        "if (FALSE)"),), "stable-slots"),
    # a slot's count read whoever's test it was, the GPU behind or dropping tests
    "owner-not-checked-late": ((("lens_flare_occlusion_visible_pixels",
                                 "xgpu_visibility_test_counted(test_index)<lens_flare_occlusion_slots[test_index].first_test",
                                 "FALSE"),), "reused-late"),
    "owner-not-checked-dropped": ((("lens_flare_occlusion_visible_pixels",
                                    "xgpu_visibility_test_counted(test_index)<lens_flare_occlusion_slots[test_index].first_test",
                                    "FALSE"),), "reused-dropped"),
    # the slot's last test before it was taken counted as the new flare's
    "owner-from-last-test": ((("lens_flare_occlusion_test_index", "xgpu_visibility_tests_ended(free_slot) + 1",
                               "xgpu_visibility_tests_ended(free_slot)"),), "reused-late"),
    # with no slot found, tested and read at the flare's place in the list past the slots (this change's first version)
    "no-slot-read-by-place": ((("lens_flare_occlusion_test_index", "return LENS_FLARE_OCCLUSION_UNREAD_INDEX;",
                                "return (short)(LENS_FLARE_OCCLUSION_UNREAD_INDEX+lens_flare_index);"),
                               ("lens_flare_occlusion_visible_pixels",
                                "if (test_index==NONE || test_index>=LENS_FLARE_OCCLUSION_SLOT_COUNT)\n\t\treturn NONE;",
                                "if (test_index==NONE)\n\t\treturn NONE;\n"
                                "\tif (test_index>=LENS_FLARE_OCCLUSION_SLOT_COUNT)\n"
                                "\t\treturn rasterizer_widget_get_occlusion_test_result(test_index);")), "no-slot"),
    # with no slot found, the count at the index no flare's is read at read: the latest flare's there
    "no-slot-read": ((("lens_flare_occlusion_visible_pixels",
                       "if (test_index==NONE || test_index>=LENS_FLARE_OCCLUSION_SLOT_COUNT)\n\t\treturn NONE;",
                       "if (test_index==NONE)\n\t\treturn NONE;\n"
                       "\tif (test_index>=LENS_FLARE_OCCLUSION_SLOT_COUNT)\n"
                       "\t\treturn rasterizer_widget_get_occlusion_test_result(test_index);"),), "no-slot"),
    # a flare tested in one slot for every window it is in
    "windows-share-a-slot": ((("lens_flare_occlusion_test_index",
                               "byte const *visibility= lens_flare_occlusion_test_results_get(lens_flare_parameters);",
                               "byte const *visibility= lens_flare_occlusion_test_results_get(lens_flare_parameters) - "
                               "(lens_flare_parameters->compressed_window_index & _lens_flare_window_index_mask);"),),
                             "split-screen"),
    # the slots kept from one map to the next: a new map's flare at an old one's marker reads the old one's count
    "slots-kept-over-map": ((("rasterizer_lights_reset_for_new_map",
                              "memset(lens_flare_occlusion_slots, 0, sizeof(lens_flare_occlusion_slots));", ""),),
                            "new-map"),
    # with the occlusion tests off, a count waited for that never comes
    "off-waits": ((("lens_flare_occlusion_visible_pixels", "rasterizer_debug_options.lens_flare_occlusion_enabled &&",
                    ""),), "occlusion-off"),
}


def generated(fault=()):
    source = read("source/rasterizer/rasterizer_lights.c")
    config = "\n".join([
        enum_with(source, "_lens_flare_first_person_weapon_flag"),
        enum_with(source, "_lens_flare_dynamic_light_flag"),
        enum_with(source, "LENS_FLARE_OCCLUSION_SLOT_COUNT"),
        structure(source, "lens_flare_occlusion_test_results"),
        structure(source, "rasterizer_lens_flare_submit_parameters"),
        structure(source, "lens_flare_occlusion_slot"),
    ]) + "\n"
    texts = {name: function(source, name) for name in FUNCTIONS}
    for where, before, after in fault:
        if where == "config":
            config = mutated(config, before, after)
        else:
            texts[where] = mutated(texts[where], before, after)
    return (("config.inc", config), ("under_test.inc", "\n".join(texts[name] for name in FUNCTIONS) + "\n"))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("lens_flare_occlusion", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("lens_flare_occlusion", generated(fault)), case)
    assert status == CHECK_FAILED, f"the occlusion code with a fault ({control}) passed '{case}': the test cannot see it"

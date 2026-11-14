"""Listener motion through real camera cuts, ownership changes and map resets."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run

CASES = ["motion", "coop-camera-motion", "cuts", "interpolation-cuts", "lifecycle", "followed-unit", "rigid-body"]


def generated(fault=None):
    manager = read("source/sound/sound_manager.c")
    director = read("source/camera/director.c")
    scripting = read("source/camera/camera_scripting.c")
    start = manager.index("#define LISTENER_SNAP_DISTANCE")
    end = manager.index("static void refresh_listener(", start)
    code = manager[start:end] + function(manager, "sound_initialize_for_new_map")
    code += function(scripting, "scripted_camera_set_camera_point_relative")
    code += function(scripting, "scripted_camera_enable")
    code += function(scripting, "scripted_camera_set")
    code += function(director, "director_set_camera")
    code += function(director, "director_get_followed_unit")
    code += function(read("source/objects/objects.c"), "object_get_point_velocity")
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", code),)


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("listener_motion", generated()), case)
    assert status == 0, output


def test_cut_reset_is_required():
    fault = ("memset(listener_motion, 0, sizeof(listener_motion));", "(void)0;")
    status, output = run(build("listener_motion", generated(fault)), "cuts")
    assert status == CHECK_FAILED, f"missing camera-cut reset escaped detection: {output}"


def test_lifecycle_wiring():
    director = read("source/camera/director.c")
    for name in ("director_initialize_for_new_map", "director_dispose_from_old_map"):
        assert "sound_listener_motion_reset(NONE)" in function(director, name)
    assert "director_initialize_for_new_map()" in function(director, "director_initialize_for_saved_game")
    scripting = read("source/camera/camera_scripting.c")
    for name in ("scripted_camera_set", "scripted_camera_set_animation",
                 "scripted_camera_set_first_person", "scripted_camera_set_dead"):
        assert "sound_listener_motion_reset(NONE)" in function(scripting, name)


def test_interpolation_cut_needs_a_change():
    fault = ("camera_script_globals.mode != _camera_script_mode_point ||", "1 ||")
    status, output = run(build("listener_motion", generated(fault)), "interpolation-cuts")
    assert status == CHECK_FAILED, f"a cut to the point the camera is at escaped detection: {output}"


def test_interpolation_cut_wiring():
    scripting = read("source/camera/camera_scripting.c")
    for name in ("scripted_camera_enable", "scripted_camera_set_animation",
                 "scripted_camera_set_first_person", "scripted_camera_set_dead", "scripted_camera_set"):
        assert "render_interpolation_cut();" in function(scripting, name), name
    assert "render_interpolation_cut" not in function(scripting, "scripted_camera_set_camera_point_relative")


def test_coop_camera_updates_must_preserve_motion():
    fault = ("camera_script_globals.camera_point_index = NONE;",
             "camera_script_globals.camera_point_index = NONE; sound_listener_motion_reset(NONE);")
    status, output = run(build("listener_motion", generated(fault)), "coop-camera-motion")
    assert status == CHECK_FAILED, f"per-update listener reset escaped detection: {output}"

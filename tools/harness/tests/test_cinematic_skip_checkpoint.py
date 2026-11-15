"""The checkpoint a cinematic's skip goes back to. A skip reverts to the checkpoint cinematic_skip_start's script takes
as the cinematic begins (game_save_totally_unsafe), and its game_reverted then skips the cinematic. A checkpoint taken
later in the cinematic took its place: Mercury Rising's mercury_falling saves every so often from a continuous script
(game_save), a second into its intro, and each skip went back there and played the rest of the intro again. main.c
holds a save while a cinematic can be skipped and its skip's checkpoint is taken (cinematics.c says which), until
cinematic_skip_stop, as one that is not safe waits: a (game_save) gives up after as many ticks, an unsafe one at once;
a skip lets go of it. The checkpoint the next cinematic takes, and one gone back to, are its skip's.
"""

import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run, structure  # noqa: E402

CASES = ["passive-save", "save-at-the-end", "unsafe-save", "timed-save-gives-up", "untimed-save-held",
         "next-cinematic", "loaded-in-a-cinematic", "no-cinematic"]
MAIN = ("main_saving_map", "main_skip_cinematic", "main_save_map_nonsafe", "main_save_map_safe",
        "main_save_map_no_timeout", "main_saving_map_private", "main_save_map_private")
CINEMATICS = ("cinematic_initialize_for_new_map", "cinematic_skip_start", "cinematic_skip_stop",
              "cinematic_can_be_skipped", "cinematic_port_checkpoint", "cinematic_port_holds_checkpoints")
STATE = ("game_state_save",)

# a fault in the code, the function it is in, and the case that must catch it
NEGATIVE_CONTROLS = {
    # as before: a save in the cinematic was taken, and the skip went back to it
    "save-not-held": (("if (cinematic_port_holds_checkpoints())", "if (FALSE)"), "main_save_map_private",
                      "passive-save"),
    "unsafe-save-not-held": (("if (cinematic_port_holds_checkpoints())", "if (main_globals.save_map_safely && "
                              "cinematic_port_holds_checkpoints())"), "main_save_map_private", "unsafe-save"),
    "unsafe-save-waits": (("if (!main_globals.save_map_safely ||", "if (FALSE ||"), "main_save_map_private",
                          "unsafe-save"),
    # as the change was before review: a (game_save) a script waited for before cinematic_skip_stop never ended
    "timed-save-never-gives-up": (("(main_globals.save_map_timeout && main_globals.ticks_unable_to_save > 240)",
                                   "FALSE"), "main_save_map_private", "timed-save-gives-up"),
    "held-ticks-counted-twice": (("\t\tmain_save_last_tick = current_tick;\n\t\tif (!main_globals.save_map_safely",
                                  "\t\tif (!main_globals.save_map_safely"), "main_save_map_private",
                                 "timed-save-gives-up"),
    "untimed-save-dropped": (("if (!main_globals.save_map_safely ||", "if (!main_globals.save_map_timeout ||"),
                             "main_save_map_private", "untimed-save-held"),
    "held-for-good": (("return cinematic_globals->can_be_skipped && cinematic_skip_checkpoint_taken;",
                       "return cinematic_skip_checkpoint_taken;"), "cinematic_port_holds_checkpoints",
                      "save-at-the-end"),
    "next-cinematic-held": (("\tcinematic_skip_checkpoint_taken = FALSE;\n", ""), "cinematic_skip_start",
                            "next-cinematic"),
    "skip-checkpoint-not-noted": (("cinematic_skip_checkpoint_taken = cinematic_globals->can_be_skipped;",
                                   "cinematic_skip_checkpoint_taken = FALSE;"), "cinematic_port_checkpoint",
                                  "passive-save"),
    "loaded-checkpoint-not-noted": (("cinematic_skip_checkpoint_taken = cinematic_globals->can_be_skipped;",
                                     "if (!cinematic_globals->can_be_skipped) cinematic_skip_checkpoint_taken = FALSE;"),
                                    "cinematic_port_checkpoint", "loaded-in-a-cinematic"),
    "noted-before-saving": (("\t\tcinematic_port_checkpoint();\n", ""), "game_state_save", "passive-save"),
}


def generated(fault=None, faulty_function=None):
    cinematics = read("source/cutscene/cinematics.c")
    header = read("source/cutscene/cinematics.h")
    sources = ((cinematics, CINEMATICS), (read("source/saved games/game_state.c"), STATE),
               (read("source/main/main.c"), MAIN))
    flag = re.search(r"^static boolean cinematic_skip_checkpoint_taken = FALSE;$", cinematics, re.M).group(0)
    types = structure(header, "cinematic_title") + "\n" + structure(header, "cinematic_global_data") + "\n" + flag + "\n"
    text = ""
    for source, names in sources:
        for name in names:
            code = function(source, name)
            if fault and name == faulty_function:
                code = mutated(code, *fault)
            text += code + "\n"
    return (("types.inc", types), ("under_test.inc", text))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("cinematic_skip_checkpoint", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, faulty_function, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("cinematic_skip_checkpoint", generated(fault, faulty_function)), case)
    assert status == CHECK_FAILED, f"the code with a fault ({control}) passed '{case}': the test cannot see it"


def test_a_checkpoint_gone_back_to_is_noted():
    # every load of a checkpoint (a revert, a skip, a saved game) runs the after-load procs, the last of which notes
    # whether it is a cinematic's skip checkpoint (the case loaded-in-a-cinematic plays that last step)
    state = read("source/saved games/game_state.c")
    procs = state[state.index("static game_state_after_load_proc after_load_procs[] ="):]
    procs = procs[:procs.index("};")]
    assert re.search(r"\bcinematic_port_checkpoint\s*$", procs.strip())
    revert = function(state, "game_state_revert")
    assert revert.index("game_state_read_from_file()") < revert.index("game_state_call_after_load_procs();")


def test_main_loop_saves_after_the_ticks():
    # the order the cases play: a save asked for in a frame is taken at the next one's start
    main = read("source/main/main.c")
    loop = main[main.index("if (main_globals.save_map_completed)"):]
    assert loop.index("main_saving_map_private();") < loop.index("game_time_update(")
    assert loop.index("game_time_update(") < loop.index("main_save_map_private();")

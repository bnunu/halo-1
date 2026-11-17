"""A chat line on Android's own keyboard: the touch controls' D-pad Up brings it up instead of the chat's keyboard (a
controller's still opens the chat's), back or a tap elsewhere lets the line go, the keyboard goes with the line, and a
line closed with back says nothing of the next. network_chat.c, xinput_sdl.c and sdl_platform.c's Android half, over
a fake SDL."""

import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, function, mutated, read, run  # noqa: E402

CASES = ["touch", "controller", "no-touch-controls", "back", "tap", "ended", "next-line", "never-came"]
PLATFORM = ("platform_screen_keyboard", "platform_screen_keyboard_lost", "screen_keyboard_update")
INPUT = ("halo_chat_system_keyboard", "halo_chat_system_keyboard_lost")
CHAT = ("chat_input_end", "chat_keyboard_begin", "chat_system_keyboard_update", "network_chat_keyboard_open",
        "network_chat_controller")

# a fault in the code, the function it is in, and the case that must catch it
NEGATIVE_CONTROLS = {
    "back-unseen": (("if (started && !host_sdl_text_input_active(window))", "if (0)"), "screen_keyboard_update",
                    "back"),
    "never-came-unseen": (("if (!started)\n\t\t__atomic_store_n", "if (0)\n\t\t__atomic_store_n"),
                          "screen_keyboard_update", "never-came"),
    "an-old-back-counts": (("__atomic_load_n(&screen_keyboard_lost, __ATOMIC_SEQ_CST) == requests",
                            "__atomic_load_n(&screen_keyboard_lost, __ATOMIC_SEQ_CST) != 0"),
                           "platform_screen_keyboard_lost", "next-line"),
    "without-touch-controls": (("if (show && !touch_input_screen_buttons())\n\t\treturn FALSE;\n", ""),
                               "halo_chat_system_keyboard", "no-touch-controls"),
    "keyboard-stays": (("if (chat_input.system_keyboard)\n\t\thalo_chat_system_keyboard(FALSE);\n", ""),
                       "chat_input_end", "ended"),
    "controller-on-android-keyboard": (("if (!physical && halo_chat_system_keyboard(TRUE))",
                                        "if (halo_chat_system_keyboard(TRUE))"), "chat_keyboard_begin", "controller"),
    "taps-ignored": ((" || halo_chat_taps(&x, &y)", ""), "chat_system_keyboard_update", "tap"),
    "touch-controls-stay": (("(chat_keyboard.open || chat_input.system_keyboard)", "chat_keyboard.open"),
                            "network_chat_controller", "touch"),
}


def android_keyboard(platform):
    """sdl_platform.c's Android half of the system's on-screen keyboard (the desktop's has the same names)."""
    start = platform.index("/* ---------- the system's on-screen keyboard (Android's)")
    return platform[start:platform.index("\n#endif", start)]


def generated(fault=None, faulty_function=None):
    chat = read("port/linux/game/network_chat.c")
    platform = android_keyboard(read("port/linux/src/sdl_platform.c"))
    inputs = read("port/linux/src/xinput_sdl.c")
    config = f"enum {{ CHAT_TEXT_SIZE = {constant(chat, 'CHAT_TEXT_SIZE')} }};\n"
    for name in ("chat_input", "chat_keyboard"):
        config += re.search(r"^static struct\n\{[^{}]*\} " + name + ";", chat, re.M).group(0) + "\n"
    config += "".join(re.findall(r"^static int screen_keyboard_\w+;\n", platform, re.M))
    text = ""
    for source, names in ((platform, PLATFORM), (inputs, INPUT), (chat, CHAT)):
        for name in names:
            code = function(source, name)
            if fault and name == faulty_function:
                code = mutated(code, *fault)
            text += code + "\n"
    return (("config.inc", config), ("under_test.inc", text))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("chat_system_keyboard", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, faulty_function, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("chat_system_keyboard", generated(fault, faulty_function)), case)
    assert status == CHECK_FAILED, f"the code with a fault ({control}) passed '{case}': the test cannot see it"


def test_the_line_is_typed_with_the_keys():
    # the line's update reads Android's keyboard's keys from the keystroke queue as a keyboard's (Enter sends it),
    # after the system keyboard's check; the touch controls' D-pad Up begins it with no controller (not physical)
    update = function(read("port/linux/game/network_chat.c"), "network_chat_update")
    assert update.index("chat_system_keyboard_update();") < update.index("while (chat_input.active && input_get_key(&key))")
    assert "chat_keyboard_begin(requested_port != NONE);" in update
    assert "case _key_return:" in update


def test_the_host_import_is_declared():
    # a guest import is listed, declared to the guest and defined in the host, or the image does not link
    assert "host_sdl_text_input_active" in read("port/android/host_imports.list").split()
    assert "int host_sdl_text_input_active(unsigned int window);" in read("port/android/guest/runtime/guest_host.h")
    assert "int host_sdl_text_input_active(uint32_t window)" in read("port/android/host/host_sdl.c")

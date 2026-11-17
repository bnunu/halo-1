"""Co-op hosts reach the lobby while split-screen and MCC restart routes retain priority."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run  # noqa: E402


def generated(fault=False):
    source = read("source/interface/ui_widget.c")
    code = "\n".join(function(source, name) for name in (
        "ui_network_server_hosting_cooperative", "network_game_reset_to_pregame_ui"))
    if fault:
        code = mutated(code, " && !ui_network_server_hosting_cooperative()", "")
    return (("under_test.inc", code),)


@pytest.mark.parametrize("case", ["coop", "multiplayer", "not-campaign", "client", "missing-game",
                                 "splitscreen", "quickstart", "mcc-restart", "mcc-split-restart"])
def test_lobby(case):
    status, output = run(build("coop_postgame_lobby", generated()), case)
    assert status == 0, output


def test_negative_control():
    status, output = run(build("coop_postgame_lobby", generated(True)), "coop")
    assert status == CHECK_FAILED, output

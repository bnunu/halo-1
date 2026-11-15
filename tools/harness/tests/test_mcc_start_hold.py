"""Which joined machines hold a host's MCC game start (mcc_ui_network.inl's mcc_ui_network_download_pending): one
still checking its own copy of the package does; one fetching it from the host (the download prompt or the
transfer) does not, and joins the game in progress, as a Custom Edition map's downloader does. Nothing holds once the
start is sent, or on a map not MCC's."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run  # noqa: E402

CASES = ["ready", "checking-holds", "fetching-joins-later", "fetching-and-checking", "not-joined", "not-mcc",
         "start-sent"]
FUNCTION = "mcc_ui_network_download_pending"

# a fault in mcc_ui_network_download_pending and the case that must catch it
NEGATIVE_CONTROLS = {
    "fetchers-hold": (("&&\n            !map_share_machine_needs_map(machine->machine_index))", ")"),
                      "fetching-joins-later"),
    "checkers-ignored": (("mcc_share_game_machine_active(machine->machine_index) &&", "0 &&"), "checking-holds"),
    "joined-ignored": (("network_game_server_client_machine_is_joined_to_game(server, machine) &&", ""),
                       "not-joined"),
    "start-ignored": (("|| server->sent_start_game_message ||", "||"), "start-sent"),
}


def generated(fault=None):
    code = function(read("port/linux/game/mcc_ui_network.inl"), FUNCTION)
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", code + "\n"),)


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("mcc_start_hold", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("mcc_start_hold", generated(fault)), case)
    assert status == CHECK_FAILED, f"the code with a fault ({control}) passed '{case}': the test cannot see it"

"""A dedicated server's lobby setting its entry's map again (server_director.c's server_lobby_map_lost): when the
lobby's widgets put another map back, not once the game is starting, nor while an MCC map waits for its package
check, which setting it again starts over: a check longer than the second between looks finished, and the MCC map
played."""

import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, function, mutated, read, run  # noqa: E402

CASES = ["legacy-lost", "same-map", "no-game", "start-sent", "mcc-pending", "mcc-check-finishes"]
FUNCTION = "server_lobby_map_lost"

# a fault in server_director.c's server_lobby_map_lost and the case that must catch it
NEGATIVE_CONTROLS = {
    "mcc-check-restarted": (("&& !mcc_ui_network_map_pending(network_server)", ""), "mcc-check-finishes"),
    "mcc-pending-ignored": (("&& !mcc_ui_network_map_pending(network_server)", ""), "mcc-pending"),
    "start-ignored": (("!network_game_server_port_start_sent(network_server) &&", ""), "start-sent"),
    "case-counted": (("csstrcasecmp(game->map.name, server.current.level)",
                      "strcmp(game->map.name, server.current.level)"), "same-map"),
    "lost-never": (("return game &&", "return 0 && game &&"), "legacy-lost"),
}


def generated(fault=None):
    code = function(read("port/linux/game/server_director.c"), FUNCTION)
    if fault:
        code = mutated(code, *fault)
    return (("under_test.inc", code + "\n"),)


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("server_lobby_map", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("server_lobby_map", generated(fault)), case)
    assert status == CHECK_FAILED, f"the code with a fault ({control}) passed '{case}': the test cannot see it"

"""A machine left out of the start for a map download (network_server_manager.c): once its need ends it is neither
waited for nor timed out while the others load, and joins in progress once the game has begun; a machine told to load
is still waited for, and dropped when the wait runs out."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, mutated, read, run  # noqa: E402

CASES = ['need-ended-not-loading', 'need-ended-check-complete', 'need-ended-not-timed-out', 'need-ended-joins-late',
         'told-machine-waited-for', 'told-machine-timed-out', 'still-downloading']
FUNCTIONS = ('network_game_server_client_machine_is_joined_to_game', 'network_game_server_discard_held_players',
             'network_game_server_queue_held_players', 'network_game_server_release_map_fetchers',
             'network_game_server_client_machine_game_loading_complete',
             'network_game_server_check_loading_complete', 'network_game_server_idle_pregame_tasks')

# a fault in network_server_manager.c, the function it is in, and the case that must catch it
NEGATIVE_CONTROLS = {
    'pending-loading': (('\t\t\t!network_game_server_map_pending[client_machine_index] &&\n', ''),
                        'network_game_server_client_machine_game_loading_complete', 'need-ended-not-loading'),
    'pending-holds-check': (('\t\t\t!network_game_server_map_pending[client_machine_index] &&\n', ''),
                            'network_game_server_check_loading_complete', 'need-ended-check-complete'),
    'pending-timed-out': ((' &&\n\t\t\t\t\t!network_game_server_map_pending[i])', ')'),
                          'network_game_server_idle_pregame_tasks', 'need-ended-not-timed-out'),
    'nothing-waited-for': (('\t\t\t!TEST_FLAG(client_machine->flags, _network_client_machine_level_loaded_bit))',
                            '\t\t\t0)'),
                           'network_game_server_client_machine_game_loading_complete', 'told-machine-waited-for'),
    'nothing-timed-out': (('\t\t\t\t\t!network_game_server_map_pending[i])', '\t\t\t\t\t0)'),
                          'network_game_server_idle_pregame_tasks', 'told-machine-timed-out'),
    'released-in-pregame': (('\t\tif (server->state != _network_game_server_state_ingame)\n\t\t\tcontinue;\n', ''),
                            'network_game_server_release_map_fetchers', 'need-ended-joins-late'),
}


def generated(fault=None, faulty_function=None):
    server = read('source/networking/network_server_manager.c')
    limits = read('port/linux/include/halo_port_limits.h')
    config = ''
    for name in ('HALO_PORT_MAXIMUM_NETWORK_MACHINES', 'HALO_PORT_MAXIMUM_NETWORK_PLAYERS',
                 'HALO_PORT_MAXIMUM_NETWORK_MESSAGE_SIZE'):
        config += f'#define {name} {constant(limits, name)}\n'
    config += f'#define MILLISECONDS_PER_SECOND {constant(read("source/cseries/cseries.h"), "MILLISECONDS_PER_SECOND")}\n'
    config += f'#define MAXIMUM_LOCAL_PLAYERS {constant(read("source/game/players.h"), "MAXIMUM_LOCAL_PLAYERS")}\n'
    for member in ('_network_client_machine_validated_bit', '_network_game_server_state_pregame',
                   'NETWORK_GAME_SERVER_PLAYERLESS_MACHINE_TIMEOUT'):
        config += enum_with(server, member) + '\n'
    code = ''
    for name in FUNCTIONS:
        text = function(server, name)
        if fault and name == faulty_function:
            text = mutated(text, *fault)
        if not text.startswith('static '):
            text = 'static ' + text
        code += text + '\n'
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_held_machine_loading(case):
    status, output = run(build('server_held_machine_loading', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('control', NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, faulty_function, case = NEGATIVE_CONTROLS[control]
    status, output = run(build('server_held_machine_loading', generated(fault, faulty_function)), case)
    assert status == CHECK_FAILED, f'network_server_manager.c with a fault ({control}) passed {case!r}: the test cannot see it'

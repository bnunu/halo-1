"""StageKit's tests' tools (README.md): build the generator, write a scenario's map, validate it, and play it.
"""

import errno
import os
import re
import shlex
import shutil
import struct
import subprocess
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
STAGEKIT = ROOT / "tools/stagekit"
BUILD = ROOT / "build/linux"
GAME = Path(os.environ.get("HALO_GAME", ROOT / "dist/halo-linux-release/halo"))
# the scenarios (scenarios/<name>.c), each played as its test module (test_<name>.py) says
SCENARIOS = sorted(path.stem for path in (STAGEKIT / "scenarios").glob("*.c"))
# the game's ticks a second
TICKS_PER_SECOND = 30
# the game's start and the map's loading, before it plays
LOADING_SECONDS = 10
# a map file's first words: 'head' and the version, the Xbox's 5 (xbox_files.c's map_file_version)
MAP_HEADER_SIGNATURE = 0x68656164
XBOX_MAP_VERSION = 5
# launch's copies of the game and its starts, one at a time (conftest.py's sessions play side by side): a copy open
# for writing when another thread forks stays open in that child until it runs its program, and the game's start
# fails while it is (ETXTBSY, "Text file busy")
LAUNCHING = threading.Lock()
# the tries at starting the game, a tenth of a second apart, for the other threads' forks (generate's tools)
LAUNCH_TRIES = 5


def build(folder):
    """build/linux's flags for the game's sources (the scenario module's): the generator, linked with the tag schemas
    (tag_schema_*.c, which set what it leaves unset to none); the game's own functions they name are never called.
    The port names its maths functions halo_ (port/include/halo_math.h) and the generator links none of them:
    it uses the C library's exact ones (sqrt) only"""
    command = subprocess.run(["ninja", "-t", "commands", "build/linux/obj/source/scenario/scenario.o"], cwd=ROOT,
                             capture_output=True, text=True, check=True).stdout.strip().splitlines()[-1]
    words = shlex.split(command)
    while words and not words[0].startswith("-"):
        words = words[1:]
    flags, skip = [], False
    for word in words:
        if skip:
            skip = False
        elif word in ("-MF", "-o", "-c"):
            skip = True
        elif word == "-MMD" or word.startswith("-flto") or word.startswith("-fprofile-use") or word.endswith(".c"):
            continue
        else:
            flags.append(word)
    sources = sorted(STAGEKIT.glob("*.c")) + sorted((STAGEKIT / "scenarios").glob("*.c")) + \
        sorted((ROOT / "port/linux/game").glob("tag_schema_*.c"))
    program = folder / "stagekit"
    # (the game's flags silence its warnings, -w; the tag schemas' calls into the game are left unresolved, and
    # anything of StageKit's own unresolved fails the build, rather than calling nothing when it runs)
    linked = subprocess.run([os.environ.get("CC", "clang"), *flags, "-O1", "-no-pie", *map(str, sources),
                             "-Wl,--warn-unresolved-symbols", "-lm", "-o", str(program)], cwd=ROOT,
                            capture_output=True, text=True)
    own = [line for line in linked.stderr.splitlines() if "undefined reference" in line and str(STAGEKIT) in line]
    if linked.returncode or own:
        raise RuntimeError(f"stagekit did not build:\n{linked.stderr}")
    return program


def run_tool(*command):
    """a tool's output; one that fails raises with what it said"""
    result = subprocess.run([str(word) for word in command], capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(f"{' '.join(map(str, command))} failed ({result.returncode}):\n"
                           f"{result.stdout}{result.stderr}")
    return result.stdout


def generate(program, scenario, folder):
    """the scenario's map (custom_maps/void.map) under folder"""
    maps = folder / "custom_maps"
    maps.mkdir(parents=True, exist_ok=True)
    run_tool(program, scenario, maps / "void.map")
    return maps / "void.map"


def validate(map_path):
    """map_validate's corrections"""
    match = re.search(r": (\d+) corrections", run_tool(BUILD / "map_validate", map_path))
    return int(match.group(1))


class Session:
    """a scenario played, from its slice's start (every player in the game): each second's {player: (x, y, z)}
    (a player not in the game that second absent), the machine each player plays on, and the game's log and
    debug.txt"""

    def __init__(self, folder, start, end, seconds, machines, log, debug):
        self.folder = folder
        self.start = start
        self.end = end
        self.seconds = seconds
        self.machines = machines
        self.log = log
        self.debug = debug

    def logged(self):
        """the test's log of each second of the slice, as the game wrote it"""
        return [line for line in self.log.splitlines() if (tick := TICK.search(line)) and
                self.start <= int(tick.group(1)) <= self.end]

    def tails(self, lines=25):
        """the end of each machine's logs: what a failed test shows"""
        return "\n".join(f"{path}:\n" + "\n".join(path.read_text(errors="replace").splitlines()[-lines:])
                         for path in sorted(self.folder.glob("**/game.log")) + sorted(self.folder.glob("**/debug.txt")))

    def places(self, player=0):
        """where the player was each second it was in the game"""
        return [second[player] for second in self.seconds if player in second]

    def deaths(self, player=0):
        """how many times the player has died, by the last second logged"""
        counts = re.findall(rf"player {player}: (?:\([^)]*\)[^|]*?|dead) s-?\d+ k\d+ d(\d+)", self.log)
        return int(counts[-1]) if counts else 0


# a second of the test's log (debug.network_test): its tick, then each player's place (or dead) and machine
TICK = re.compile(r"network test: tick (\d+) (.*)")
PLAYER = re.compile(r"player (\d+): (?:\(([-\w.]+) ([-\w.]+) ([-\w.]+)\)|dead).*? m(\d+)")


def ticks(log_path):
    """each second logged: (tick, {player: (x, y, z) or None when dead}, {player: machine})"""
    lines = log_path.read_text(errors="replace").splitlines() if log_path.exists() else []
    result = []
    for line in lines:
        tick = TICK.search(line)
        if tick:
            players = PLAYER.findall(tick.group(2))
            result.append((int(tick.group(1)),
                           {int(p): (float(x), float(y), float(z)) if x else None for p, x, y, z, _ in players},
                           {int(p): int(m) for p, _, _, _, m in players}))
    return result


def launch(machine, input, address, broadcast, test, init="", **environment):
    """the game in machine (a folder with its maps), with no window or sound, drawing nothing:
    debug.network_test test, the scripted input on the controller"""
    (machine / "init.txt").write_text("debug_no_drawing 1\n" + init)
    for name in ("debug.txt", "game.log"):
        (machine / name).unlink(missing_ok=True)
    (machine / "runtime").mkdir(mode=0o700, exist_ok=True)
    # (none of the caller's own settings; system link on the loopback only, no internet play)
    environment = dict({name: value for name, value in os.environ.items() if not name.startswith("HALO_")},
                       HALO_NET_ONLINE="false", SDL_AUDIODRIVER="dummy", SDL_VIDEODRIVER="offscreen", HALO_NO_AUDIO="1",
                       HALO_NULL_RENDERER="1", HALO_UPDATE_AUTO="false", HALO_TEST_INPUT=input,
                       HALO_SAVE_ROOT=str(machine / "save"), XDG_RUNTIME_DIR=str(machine / "runtime"),
                       HOME=str(machine), HALO_NETWORK_TEST=test, HALO_NET_ADDRESS=address,
                       HALO_NET_BROADCAST=broadcast, **environment)
    with LAUNCHING, open(machine / "game.log", "w") as log:
        # (the game, and the libraries it finds beside it, in machine's own folder: a desktop build keeps its
        # config.toml beside the executable, so each machine has its own settings; copied when they are on
        # another file system, as on CI)
        for source in [GAME, *GAME.parent.glob("lib*.so*")]:
            target = machine / ("halo" if source == GAME else source.name)
            target.unlink(missing_ok=True)
            try:
                os.link(source, target)
            except OSError:
                shutil.copy(source, target)
        for attempt in range(LAUNCH_TRIES):
            try:
                return subprocess.Popen([str(machine / "halo")], cwd=machine, env=environment, stdout=log,
                                        stderr=subprocess.STDOUT)
            except OSError as error:
                if error.errno != errno.ETXTBSY or attempt == LAUNCH_TRIES - 1:
                    raise
                time.sleep(0.1)


# the first tick every player has a unit, from which a scenario's seconds (and cues) count (network_test.c)
SLICE = re.compile(r"network test: cues from tick (\d+)")


def cue_settings(cues, players):
    """debug.network_test_cues: the game runs each (second, console line) at its tick of the slice"""
    return dict(HALO_NETWORK_TEST_CUES="|".join(f"{second} {line}" for second, line in sorted(cues)),
                HALO_NETWORK_TEST_CUE_PLAYERS=str(players))


def play_slice(log_path, seconds, games):
    """the slice's start, once the game has played seconds of it; a game that stops first, or never gets there
    in time, fails the run with the end of its logs"""
    deadline = time.monotonic() + LOADING_SECONDS + seconds + 60
    while time.monotonic() < deadline and all(game.poll() is None for game in games):
        text = log_path.read_text(errors="replace") if log_path.exists() else ""
        start = SLICE.search(text)
        logged = ticks(log_path) if start else []
        if logged and logged[-1][0] >= int(start.group(1)) + seconds * TICKS_PER_SECOND:
            return int(start.group(1))
        time.sleep(0.05)
    stopped = [game.returncode for game in games if game.poll() is not None]
    raise RuntimeError(f"the game {'stopped ' + str(stopped) if stopped else 'ran out of time'} before {seconds} "
                       "seconds of its slice:\n" + Session(log_path.parent, 0, -1, [], {}, "", "").tails())


def session(folder, start, seconds):
    """the Session of the game logged in folder, from its slice's start, for seconds"""
    end = start + seconds * TICKS_PER_SECOND
    logged = [(places, machines) for tick, places, machines in ticks(folder / "game.log") if start <= tick <= end]
    debug = folder / "debug.txt"
    return Session(folder, start, end, [{player: place for player, place in places.items() if place is not None}
                                        for places, _ in logged], logged[-1][1] if logged else {},
                   (folder / "game.log").read_text(errors="replace"),
                   debug.read_text(errors="replace") if debug.exists() else "")


def play(folder, seconds, slot=0, cues=(), input="bot:1"):
    """the game on folder's custom_maps/void.map on one machine for seconds of game time (a Session); slot sets
    the run's addresses apart, so runs go side by side; the game runs the cues' (second, console line) at their
    ticks; input is debug.test_input's.

    It plays the map in single player with no game data at all (a maps folder with only the header of an Xbox
    ui.map, which is all the game asks for to start: its check of the maps folder reads no more of it, and
    map_name skips the menus), logged by debug.network_test "log", a tick a frame (debug.fixed_step): as fast as
    the machine goes, and the same every time."""
    (folder / "maps").mkdir(exist_ok=True)
    (folder / "maps" / "ui.map").write_bytes(struct.pack("<II", MAP_HEADER_SIGNATURE, XBOX_MAP_VERSION))
    game = launch(folder, input, f"127.10.{slot}.10", f"127.10.{slot}.11", "log",
                  init="map_name custom_maps\\void\n", HALO_FIXED_STEP="1", **cue_settings(cues, 1))
    # (stopped here: debug.exit_after is read with the window's events, and the null renderer opens no window)
    try:
        start = play_slice(folder / "game.log", seconds, [game])
    finally:
        game.kill()
        game.wait()
    return session(folder, start, seconds)


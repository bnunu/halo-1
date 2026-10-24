"""Internet play between two copies of tools/p2p_tunnel_check.c (p2p.c and
p2p_signal.c as the game has them), on this computer: one hosts, the other
joins with its invite through the signalling brokers, offering addresses of
one family. ipv6 is what two machines behind carrier NATs that let no IPv4
connection through have: it is skipped where this computer has no global
IPv6 address.

    python -m pytest -q tools/test_p2p_tunnel.py

It needs a configured build (python configure.py) of this computer's
platform, whose objects it builds and links with the check, and the
internet's brokers (the game's own list, or HARNESS_BROKERS: host:port,
comma-separated, a local mosquitto's for one).
"""

import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
PLATFORM = "windows" if os.name == "nt" else "linux"
OBJECTS = [
    "port/linux/src/p2p", "port/linux/src/p2p_signal", "port/linux/src/p2p_crypto", "port/linux/src/log_address",
    "port/third_party/kcp/ikcp", "port/third_party/monocypher/monocypher",
    "port/third_party/monocypher/monocypher-ed25519",
] + (["port/windows/src/win32_net", "port/windows/src/win32_posix"] if PLATFORM == "windows"
     else ["port/linux/src/posix_net"])
LIBRARIES = (["-lws2_32", "-liphlpapi", "-lbcrypt", "-lwinmm", "-ladvapi32", "-luser32"] if PLATFORM == "windows"
             else ["-lpthread"])
TIMEOUT = 120


def _ninja(*arguments):
    return subprocess.run(["ninja", *arguments], cwd=ROOT, capture_output=True, text=True)


@pytest.fixture(scope="module")
def program(tmp_path_factory):
    """the check, built with the flags ninja gives p2p.c, and linked with the
    build's own objects of the rest"""
    if not shutil.which("clang") or not shutil.which("ninja") or not (ROOT / "build.ninja").is_file():
        pytest.skip("needs clang, ninja and a configured build")
    objects = [f"build/{PLATFORM}/obj/{name}.o" for name in OBJECTS]
    built = _ninja(*objects)
    assert built.returncode == 0, built.stdout[-4000:] + built.stderr[-4000:]
    command = _ninja("-t", "commands", objects[0]).stdout.strip().splitlines()[-1]
    words = shlex.split(command, posix=PLATFORM != "windows")
    # (the compiler, and whatever runs it, ccache in CI: up to the first flag)
    while words and not words[0].startswith("-"):
        words = words[1:]
    flags, skip = [], False
    for word in words:
        if skip:
            skip = False
        elif word in ("-MF", "-o", "-c"):
            skip = True
        elif word == "-MMD" or word.startswith("-fprofile-use"):
            continue
        else:
            flags.append(word)
    flags = [flag for flag in flags if not flag.endswith(".c")]
    work = tmp_path_factory.mktemp("p2p_tunnel")
    executable = work / ("p2p_tunnel_check.exe" if PLATFORM == "windows" else "p2p_tunnel_check")
    target = [flag for flag in flags if flag.startswith("--target") or flag in ("-m32", "-flto=full", "-flto")]
    link = ["-fuse-ld=lld"] if PLATFORM == "windows" else ["-no-pie"]
    built = subprocess.run(["clang", *flags, "-o", str(work / "check.o"), "-c", "tools/p2p_tunnel_check.c"],
                           cwd=ROOT, capture_output=True, text=True)
    assert built.returncode == 0, built.stderr[-4000:]
    built = subprocess.run(["clang", *target, *link, "-O2", "-o", str(executable), str(work / "check.o"),
                            *objects, *LIBRARIES], cwd=ROOT, capture_output=True, text=True)
    assert built.returncode == 0, built.stderr[-4000:]
    return executable


def _wait(process, name, work):
    try:
        output, _ = process.communicate(timeout=TIMEOUT)
    except subprocess.TimeoutExpired:
        process.kill()
        output, _ = process.communicate()
    log = (work / f"{name}.log").read_text(errors="replace")
    return output, log


@pytest.mark.parametrize("family", ["ipv4", "ipv6"])
def test_invite_reaches_host(program, family, tmp_path):
    """the joiner reaches the host by its invite, over addresses of the family
    alone; the host's real address, as bans and votes see it, is of the
    family (an IPv6 one's /64, and its stand-in in 240.0.0.0/4)"""
    environment = dict(os.environ)
    with open(tmp_path / "host.log", "w") as host_log, open(tmp_path / "join.log", "w") as join_log:
        host = subprocess.Popen([str(program), "host", family], cwd=tmp_path, stdout=subprocess.PIPE,
                                stderr=host_log, text=True, env=environment)
        first = host.stdout.readline().strip()
        if first.startswith("SKIP"):
            host.wait()
            pytest.skip(first)
        assert first.startswith("invite halo://join/"), first + (tmp_path / "host.log").read_text()
        join = subprocess.Popen([str(program), "join", first.split()[1], family], cwd=tmp_path,
                                stdout=subprocess.PIPE, stderr=join_log, text=True, env=environment)
        joined, joined_log = _wait(join, "join", tmp_path)
        hosted, hosted_log = _wait(host, "host", tmp_path)
    assert "PASS" in joined, joined + joined_log
    assert "PASS" in hosted, hosted + hosted_log
    connected = [line for line in (joined + hosted).splitlines() if line.startswith("connected ")]
    assert len(connected) == 2
    for line in connected:
        # (an IPv6 address is written in brackets, before its port)
        assert ("[" in line) == (family == "ipv6"), line
    real = re.search(r"^real (\S+) (\d+)\.\d+\.\d+\.\d+$", joined, re.M)
    assert real, joined
    if family == "ipv6":
        assert real.group(1).endswith("::/64") and 240 <= int(real.group(2)) <= 255, real.group(0)
    else:
        assert re.fullmatch(r"\d+\.\d+\.\d+\.\d+", real.group(1)), real.group(0)


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))

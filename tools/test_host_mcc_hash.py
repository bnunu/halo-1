"""port/android/host/host_mcc_hash.c's MCC fingerprints against the guest's
definition (port/linux/game/mcc_resource_check.inl), worked out again here
with hashlib and zlib: one map's, a package's kept in each way, the SHA-256
and CRC32 by the portable code and by ARMv8's instructions (under
qemu-aarch64, when there is an NDK and qemu)."""
import hashlib
import os
import shutil
import subprocess
import zlib
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
DRIVER = ROOT / "tools/tests/host_mcc_hash.c"
ZLIB = ROOT / "port/third_party/zlib"
NDK = Path(os.environ.get("ANDROID_NDK_HOME") or os.environ.get("ANDROID_NDK_ROOT") or "/nonexistent",
           "toolchains/llvm/prebuilt/linux-x86_64/bin/clang")
PREFIX = b"GulchCE MCC content v1"


def fingerprint(map_bytes, bitmaps, sounds, init):
    sha = hashlib.sha256(PREFIX)
    crc = init
    for kind, data in ((0, map_bytes), (1, bitmaps), (2, sounds)):
        if data is None:
            continue
        sha.update(bytes([kind]) + len(data).to_bytes(8, "little"))
        sha.update(data)
        if kind:
            crc = zlib.crc32(bytes([kind]), crc)
            crc = zlib.crc32(data, crc)
            crc = zlib.crc32(b"\xff", crc)
    return sha.hexdigest(), crc


def package(tmp_path):
    root = tmp_path / "root"
    folder = root / "mcc_maps" / "123"
    folder.mkdir(parents=True)
    rng = __import__("random").Random(7)
    # (sizes off every chunk and block boundary: 512 KB chunks, 64-byte blocks)
    maps = {f"h1_m{i}.map": rng.randbytes(700_001 + i * 37) for i in range(3)}
    for name, data in maps.items():
        (folder / name).write_bytes(data)
    bitmaps = rng.randbytes(2 * 512 * 1024 + 13)
    sounds = rng.randbytes(300_003)
    (folder / "bitmaps.map").write_bytes(bitmaps)
    (folder / "sounds.map").write_bytes(sounds)
    return root, folder, maps, bitmaps, sounds


def check(command, tmp_path, expect_kind=None):
    root, folder, maps, bitmaps, sounds = package(tmp_path)
    name, data = sorted(maps.items())[1]
    init = 0x1234abcd
    output = subprocess.run(command + [str(root), str(folder), str(folder / name), str(folder / "bitmaps.map"),
                                       str(folder / "sounds.map"), f"{init:x}"],
                            check=True, capture_output=True, text=True).stdout.split("\n")
    lines = {line.split()[0]: line.split()[1:] for line in output if line}
    if expect_kind:
        assert expect_kind in " ".join(lines["kind"]), lines["kind"]
    digest, crc = fingerprint(data, bitmaps, sounds, init)
    assert lines["job"] == [digest, f"{crc:08x}"]
    assert lines["package"] == ["3"]
    for way in range(4):
        digest, crc = fingerprint(data, bitmaps if way & 1 else None, sounds if way & 2 else None, init)
        assert lines[f"way{way}"] == [digest, f"{crc:08x}"], way
    assert (root / "mcc_fingerprints.txt").read_text().startswith("mcc fingerprints 2\n")


def build(tmp_path, compiler, *flags, threads=("-lpthread",)):
    exe = tmp_path / "driver"
    # (with the port's zlib, whose CRC-32 is the portable one's)
    zlib_objects = []
    for source in ["crc32.c", "zutil.c"]:
        zlib_objects.append(tmp_path / (source + ".o"))
        subprocess.run([*compiler, "-O2", "-w", "-DZ_PREFIX", "-c", str(ZLIB / source), "-o", str(zlib_objects[-1])],
                       check=True)
    subprocess.run([*compiler, "-O2", "-std=gnu11", "-Wall", "-Werror", "-I", str(ZLIB), *flags, str(DRIVER),
                    *map(str, zlib_objects), "-o", str(exe), *threads], check=True)
    return exe


def test_portable(tmp_path):
    check([str(build(tmp_path, ["cc"], "-DHOST_MCC_HASH_PORTABLE"))], tmp_path, "portable portable")


@pytest.mark.skipif(not NDK.exists() or not shutil.which("qemu-aarch64-static"), reason="no NDK or qemu-aarch64")
def test_arm(tmp_path):
    # (Android's threads are its libc's)
    exe = build(tmp_path, [str(NDK), "--target=aarch64-linux-android29", "-static"], threads=())
    check(["qemu-aarch64-static", "-cpu", "max", str(exe)], tmp_path, "ARMv8 SHA-256 ARMv8 CRC32")


def test_cache_not_fooled_by_kept_times(tmp_path):
    # (a file changed in place with its size and times put back: the cache's
    # key must not match it, so the fingerprint is worked out again)
    exe = build(tmp_path, ["cc"])
    root, folder, maps, bitmaps, sounds = package(tmp_path)
    name, data = sorted(maps.items())[1]
    init = 0x1234abcd
    command = [str(exe), str(root), str(folder), str(folder / name), str(folder / "bitmaps.map"),
               str(folder / "sounds.map"), f"{init:x}"]
    subprocess.run(command, check=True, capture_output=True)
    before = os.stat(folder / "bitmaps.map")
    changed = bytes([bitmaps[0] ^ 0xFF]) + bitmaps[1:]
    (folder / "bitmaps.map").write_bytes(changed)
    os.utime(folder / "bitmaps.map", ns=(before.st_atime_ns, before.st_mtime_ns))
    output = subprocess.run(command, check=True, capture_output=True, text=True).stdout.split("\n")
    lines = {line.split()[0]: line.split()[1:] for line in output if line}
    digest, crc = fingerprint(data, changed, sounds, init)
    assert lines["job"] == [digest, f"{crc:08x}"]

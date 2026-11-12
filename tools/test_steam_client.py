"""The Steam Workshop screen's Steam client (port/android/host/steam_*.c, the host's), built natively with -Wall -Wextra
-Werror, and its offline parts checked: its own SHA-1, HMAC, AES-256, CRC-32 and base64 against known
answers, the connection's envelope (which the seal outside Android also uses) refusing a changed byte, the session file
(the refresh token sealed, never written as it is where there is a key; another device's key opening nothing; a token
kept as it is where there was no key sealed at the next load; the file the owner's alone; forgetting it), the depots'
LZMA ("VZa"), Zstandard ("VSZa") and zip chunks, and the paths a Workshop item's files may take in its package.
Nothing here reaches Steam."""
import hashlib
import hmac
import lzma
import os
from pathlib import Path
import shutil
import stat
import struct
import subprocess
import zipfile
import zlib

import pytest

ROOT = Path(__file__).resolve().parents[1]
ZLIB = ROOT / "port/third_party/zlib"
HOST = ROOT / "port/android/host"
STEAM_SOURCES = ["steam_cm.c", "steam_crypto.c", "steam_depot.c", "steam_net.c", "steam_seal.c", "steam_unpack.c"]
TOKEN = "eyJhbGciOiJFZERTQSJ9.eyJzdWIiOiI3NjU2MTE5OTAwMDAwMDAwMSJ9.c2lnbmF0dXJl"


@pytest.fixture(scope="module")
def client(tmp_path_factory):
    clang = shutil.which("clang")
    if not clang:
        pytest.skip("clang required")
    work = tmp_path_factory.mktemp("steam-client")
    objects = []
    for source in ["posix_files.c", "posix_mcc_share.c"]:
        objects.append(work / (source + ".o"))
        subprocess.run([clang, "-std=gnu99", "-w", "-c", str(ROOT / "port/linux/src" / source), "-o", str(objects[-1])],
                       check=True)
    for source in ["adler32.c", "crc32.c", "inffast.c", "inflate.c", "inftrees.c", "zutil.c"]:
        objects.append(work / (source + ".o"))
        subprocess.run([clang, "-std=gnu99", "-w", "-DZ_PREFIX", "-c", str(ZLIB / source), "-o", str(objects[-1])],
                       check=True)
    for source in [*(HOST / name for name in STEAM_SOURCES), ROOT / "tools/tests/steam_client_driver.c"]:
        objects.append(work / (source.name + ".o"))
        # (-Wall -Wextra -Werror, but for the unpacker's third-party decoders; the driver
        # includes steam_downloads.c, so that is held to it too)
        warnings = ["-w"] if source.name == "steam_unpack.c" else ["-Wall", "-Wextra", "-Werror"]
        result = subprocess.run([clang, "-std=c11", "-D_GNU_SOURCE", "-DSTEAM_SEAL_TEST", *warnings,
                                 "-I", str(HOST), "-I", str(ROOT / "port/android/include"),
                                 "-I", str(ROOT / "port/linux/src"), "-I", str(ZLIB),
                                 "-c", str(source), "-o", str(objects[-1])], capture_output=True, text=True)
        assert result.returncode == 0, result.stderr
    exe = work / "steam_client"
    subprocess.run([clang, *map(str, objects), "-lpthread", "-o", str(exe)], check=True)
    return exe


def run(client, *arguments, secret=None):
    environment = dict(os.environ)
    environment.pop("STEAM_SEAL_TEST_SECRET", None)
    if secret is not None:
        environment["STEAM_SEAL_TEST_SECRET"] = secret
    result = subprocess.run([str(client), *map(str, arguments)], capture_output=True, env=environment)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    return result.stdout


def test_crypto_known_answers(client):
    lines = run(client, "crypto").decode().split("\n")
    assert lines[0] == hashlib.sha1(b"abc").hexdigest()
    assert lines[1] == hmac.new(b"key", b"The quick brown fox jumps over the lazy dog", hashlib.sha1).hexdigest()
    # (FIPS 197's appendix C.3: AES-256)
    assert lines[2] == "8ea2b7ca516745bfeafc49904b496089"
    assert lines[3] == "00112233445566778899aabbccddeeff"
    assert lines[4] == "hello world"
    assert lines[5] == f"{zlib.crc32(b'123456789'):08x}"


def test_envelope_refuses_a_changed_byte(client):
    assert run(client, "envelope").decode().split("\n")[:2] == ["round trip 1", "tampered 0"]


def session_file(root):
    return root / "steam" / "session.txt"


def test_token_sealed_with_the_devices_key(client, tmp_path):
    run(client, "save", tmp_path, "player", TOKEN, secret="device one")
    text = session_file(tmp_path).read_text()
    assert "account=player\n" in text
    assert "sealed=device:" in text
    assert TOKEN not in text and "token=" not in text
    assert stat.S_IMODE(session_file(tmp_path).stat().st_mode) == 0o600
    assert not (tmp_path / "steam" / "session.txt.part").exists()
    assert run(client, "load", tmp_path, secret="device one").decode() == f"account player\ntoken {TOKEN}\n"


def test_another_devices_copy_opens_nothing(client, tmp_path):
    run(client, "save", tmp_path, "player", TOKEN, secret="device one")
    assert run(client, "load", tmp_path, secret="device two").decode() == "account player\ntoken \n"
    assert run(client, "load", tmp_path).decode() == "account player\ntoken \n"


def test_a_damaged_seal_opens_nothing(client, tmp_path):
    run(client, "save", tmp_path, "player", TOKEN, secret="device one")
    text = session_file(tmp_path).read_text()
    sealed = text.split("sealed=device:")[1].split("\n")[0]
    changed = sealed[:-1] + ("0" if sealed[-1] != "0" else "1")
    session_file(tmp_path).write_text(text.replace(sealed, changed))
    assert run(client, "load", tmp_path, secret="device one").decode() == "account player\ntoken \n"


def test_without_a_key_kept_as_it_is_then_sealed(client, tmp_path):
    run(client, "save", tmp_path, "player", TOKEN)
    text = session_file(tmp_path).read_text()
    assert f"token={TOKEN}\n" in text and "sealed=" not in text
    assert stat.S_IMODE(session_file(tmp_path).stat().st_mode) == 0o600
    # (a key now: the load seals what it read)
    assert run(client, "load", tmp_path, secret="device one").decode() == f"account player\ntoken {TOKEN}\n"
    text = session_file(tmp_path).read_text()
    assert "sealed=device:" in text and TOKEN not in text


def test_forgotten(client, tmp_path):
    run(client, "save", tmp_path, "player", TOKEN, secret="device one")
    run(client, "forget", tmp_path, secret="device one")
    assert not session_file(tmp_path).exists()


def unpack(client, tmp_path, chunk, size):
    (tmp_path / "chunk").write_bytes(chunk)
    out = run(client, "unpack", tmp_path / "chunk", size)
    head, _, body = out.partition(b"\n")
    return head == b"unpacked 1", body


PLAIN = bytes(range(256)) * 64 + b"the end of a chunk"


def test_lzma_chunk(client, tmp_path):
    properties = {"id": lzma.FILTER_LZMA1, "dict_size": 1 << 16, "lc": 3, "lp": 0, "pb": 2}
    raw = lzma.compress(PLAIN, format=lzma.FORMAT_RAW, filters=[properties])
    props = bytes([(2 * 5 + 0) * 9 + 3]) + struct.pack("<I", 1 << 16)
    chunk = b"VZa" + b"\0" * 4 + props + raw + struct.pack("<II", zlib.crc32(PLAIN), len(PLAIN)) + b"zv"
    assert unpack(client, tmp_path, chunk, len(PLAIN)) == (True, PLAIN)
    assert unpack(client, tmp_path, chunk, len(PLAIN) + 1)[0] is False


def test_zstd_chunk(client, tmp_path):
    zstd = shutil.which("zstd")
    if not zstd:
        pytest.skip("zstd required")
    frame = subprocess.run([zstd, "-q", "-c"], input=PLAIN, capture_output=True, check=True).stdout
    chunk = b"VSZa" + b"\0" * 4 + frame + b"\0" * 15
    assert unpack(client, tmp_path, chunk, len(PLAIN)) == (True, PLAIN)


def test_zip_chunk(client, tmp_path):
    zipped = tmp_path / "chunk.zip"
    with zipfile.ZipFile(zipped, "w", zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("z", PLAIN)
    assert unpack(client, tmp_path, zipped.read_bytes(), len(PLAIN)) == (True, PLAIN)


def test_unknown_chunk_refused(client, tmp_path):
    assert unpack(client, tmp_path, b"XXXX" + PLAIN, len(PLAIN))[0] is False


@pytest.mark.parametrize("name,expected", [
    ("maps/Level.map", "level.map"),
    ("halo1/maps/bitmaps.map", "bitmaps.map"),
    ("ModInfo.json", "modinfo.json"),
    ("Images/Preview.PNG", "images/preview.png"),
    ("maps/sub/level.map", None),
    ("other/level.map", None),
    ("../escape.json", None),
    ("/rooted.json", None),
    ("a//b.json", None),
    ("c:/drive.json", None),
    ("readme.txt", None),
])
def test_item_paths(client, name, expected):
    assert run(client, "path", name).decode().strip() == (expected or "(refused)")

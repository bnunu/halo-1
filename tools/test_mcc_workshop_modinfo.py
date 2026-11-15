"""A package's Workshop item read from its own ModInfo.json (mcc_workshop.c),
when it has neither workshop_id.txt nor Steam's folder name."""
import json
import shutil
import subprocess
from pathlib import Path

import pytest
from harness import function

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / "port/linux/game"
SRC = ROOT / "port/linux/src"

DRIVER = r'''
#include "mcc_workshop.h"
#include "server_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
'''

MAIN = r'''
int main(int argc, char **argv)
{
    static char text[1 << 20];
    struct mcc_workshop_item item;
    FILE *file = fopen(argv[1], "rb");
    size_t length;
    if (!file) return 2;
    length = fread(text, 1, sizeof(text), file);
    fclose(file);
    (void)argc;
    if (!mcc_workshop_parse_modinfo(text, (int)length, &item)) { printf("none\n"); return 0; }
    printf("id=%s\ntitle=%s\n", item.id, item.title);
    return 0;
}
'''


@pytest.fixture(scope="module")
def tool(tmp_path_factory):
    compiler = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not compiler:
        pytest.skip("no C compiler")
    work = tmp_path_factory.mktemp("mcc-workshop-modinfo")
    workshop = (GAME / "mcc_workshop.c").read_text()
    source = work / "driver.c"
    source.write_text(DRIVER + "\n".join(function(workshop, name) for name in (
        "mcc_workshop_digits", "mcc_workshop_title_copy", "mcc_workshop_parse_modinfo")) + MAIN)
    exe = work / "driver"
    result = subprocess.run([compiler, "-std=gnu11", "-Wall", "-Wextra", "-Werror", "-I", str(GAME), "-I", str(SRC),
                             str(source), str(SRC / "server_json.c"), "-o", str(exe)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return exe


def parse(tool, tmp_path, text):
    path = tmp_path / "ModInfo.json"
    path.write_bytes(text if isinstance(text, bytes) else text.encode())
    result = subprocess.run([str(tool), str(path)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
    if result.stdout.strip() == "none":
        return None
    return dict(line.split("=", 1) for line in result.stdout.splitlines())


def modinfo(workshop_id, engine="Halo1", title="Frycook"):
    info = {"Engine": engine, "Title": {"Neutral": title}, "ModIdentifier": {"ModGuid": "c75c92d4"}}
    if workshop_id is not ...:
        info["ModIdentifier"]["HostedModIds"] = {"SteamWorkshopId": workshop_id}
    return json.dumps(info, indent="\t")


def test_number_as_mcc_writes_it(tool, tmp_path):
    assert parse(tool, tmp_path, modinfo(2900261691)) == {"id": "2900261691", "title": "Frycook"}


def test_string_id(tool, tmp_path):
    assert parse(tool, tmp_path, modinfo("2900261691")) == {"id": "2900261691", "title": "Frycook"}


def test_id_past_a_doubles_precision_kept_exact(tool, tmp_path):
    text = modinfo(0).replace(": 0", ": 18446744073709551615")
    assert parse(tool, tmp_path, text)["id"] == "18446744073709551615"


def test_byte_order_mark(tool, tmp_path):
    assert parse(tool, tmp_path, b"\xef\xbb\xbf" + modinfo(123).encode())["id"] == "123"


def test_title_made_printable(tool, tmp_path):
    assert parse(tool, tmp_path, modinfo(5, title="Café – Map  "))["title"] == "Caf  Map"


@pytest.mark.parametrize("workshop_id", [..., 0, "0", "", None, -5, 1.5, "12a", "123456789012345678901", True])
def test_no_usable_id(tool, tmp_path, workshop_id):
    assert parse(tool, tmp_path, modinfo(workshop_id)) is None


def test_other_engines_refused(tool, tmp_path):
    assert parse(tool, tmp_path, modinfo(5, engine="Halo2")) is None


@pytest.mark.parametrize("text", ["", "[]", "{", "not json"])
def test_not_a_document(tool, tmp_path, text):
    assert parse(tool, tmp_path, text) is None

"""An MCC map's sound or bitmap stored under another tag's name is found at its position in the map's own
resource file, as MCC finds it; never to choose a shared file."""
import pytest

from tools.harness import build, constant, function, mutated, read, run, structure

SOURCE = read("port/linux/game/mcc_resources.c")


def code(source=SOURCE):
    limits = "\n".join(f"#define {name} {value}" for name, value in (
        ("MCC_RESOURCE_METADATA_LIMIT", constant(source.replace("0x1000000u", "0x1000000"),
                                                 "MCC_RESOURCE_METADATA_LIMIT")),
        ("MCC_PACKAGE_BASE_SETS", constant(read("port/linux/game/mcc_package.h"), "MCC_PACKAGE_BASE_SETS"))))
    under_test = "\n".join([structure(source, "mcc_resource_item"), structure(source, "mcc_resource_file"),
                            structure(source, "mcc_resources"), structure(source, "mcc_resource_probe")] +
                           [function(source, name) for name in (
                               "mcc_resource_word", "mcc_resource_close", "mcc_resource_open", "mcc_resource_read",
                               "mcc_resource_check", "mcc_resource_equal", "mcc_resources_select_paths",
                               "mcc_resources_dispose")])
    return build("mcc_resource_position", (("limits.inc", limits), ("under_test.inc", under_test)))


@pytest.mark.parametrize("case", ["lookups", "decoding", "shared"])
def test_resource_found_by_position(case):
    status, output = run(code(), case)
    assert status == 0, output


def test_negative_control_shared_file_by_position():
    """A shared file that finds a member by its position is caught."""
    status, _ = run(code(mutated(SOURCE, "if (!file->own) return 0;", "")), "shared")
    assert status != 0


def test_negative_control_decoder_read_whole():
    """A decoder's read held to the member's whole size is caught."""
    status, _ = run(code(mutated(SOURCE, "file->items[i].size>=bytes", "file->items[i].size==bytes")), "decoding")
    assert status != 0

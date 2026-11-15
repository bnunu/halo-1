"""An MCC map's sound or bitmap stored under another tag's name is found at its position, as MCC finds it."""
from tools.harness import build, function, read, run, structure


def test_resource_found_by_position():
    source = read("port/linux/game/mcc_resources.c")
    code = "\n".join((structure(source, "mcc_resource_item"), structure(source, "mcc_resource_file"),
                      function(source, "mcc_resource_read")))
    status, output = run(build("mcc_resource_position", (("under_test.inc", code),)), "lookups")
    assert status == 0, output

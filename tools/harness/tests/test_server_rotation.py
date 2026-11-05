"""A dedicated server's rotation (port/linux/src/server_rotation.c), taken whole: the entries' grammar (map, gametype,
+rule=value), the rules' table and values, the separators and limits, the text written back and read again, and the
maps found on disk from small cache file headers the test writes."""

import sys
import tempfile
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, mutated, read, run  # noqa: E402

CASES = [
    "names", "custom-maps", "paths", "gametypes", "rules", "whitespace-comments", "quotes", "long", "ui",
    "rotation-separators", "rotation-limits", "round-trip", "rule-find", "rule-values", "check-kind", "map-header",
    "resolve", "maps-each",
    "long-map-level",
    "rotation-cut-entry",
    "resolve-version",
    "resolve-case",
]

# a fault in server_rotation.c and the case that must catch it
NEGATIVE_CONTROLS = {
    "comments-read": (("if (text[start] == '#')\n\t\treturn 0;", ""), "whitespace-comments"),
    "dots-climb": (("if (!length || dots == length)", "if (!length || dots > length)"), "paths"),
    "slashes-kept": (("if (map[index] == '/')", "if (0)"), "paths"),
    "custom-maps-path": (("if (!plain_name(map + 12))", "if (!level_path(map + 12))"), "custom-maps"),
    "long-map-cut": (("if (map_length >= sizeof(entry->map))", "if (map_length > sizeof(entry->map))"), "long"),
    "long-gametype-cut": (("gametype_length >= sizeof(entry->gametype)", "gametype_length > sizeof(entry->gametype)"),
                          "gametypes"),
    "file-names-unchecked": (('!plain_name(!strncmp(gametype, "file=", 5) ? gametype + 5 : gametype)',
                              '!plain_name(gametype)'), "gametypes"),
    "ui-played": (('if (!strcmp(entry->map_name, "ui"))', "if (0)"), "ui"),
    "rule-twice-counted": (("&& strcmp(entry->rules[existing].key, definition->key);", ";"), "rules"),
    "rule-untrimmed": (("while (key_end > rule && is_space(key_end[-1]))", "while (0)"), "rules"),
    "aliases-unknown": (("|| (rule->alias && text_equal_case(key, rule->alias))", ""), "rule-find"),
    "prefixes-equal": (("return !*a && !*b;", "return 1;"), "rule-find"),
    "range-unchecked": (("if (number < rule->minimum || number > rule->maximum)", "if (0)"), "rule-values"),
    "index-past-words": (("number >= 0 && number < index", "number >= 0 && number <= index"), "rule-values"),
    "plus-signs-taken": (("(isdigit((unsigned char)text[0]) || (text[0] == '-'", "(isdigit((unsigned char)text[0]) || "
                          "(text[0] == '+'"), "rule-values"),
    "semicolons-joined": ((" && text[end] != ';'", ""), "rotation-separators"),
    "empty-segments-truncate": (("if (server_rotation_parse_entry(text + start, end - start, &ignored))\n\t\t\t\t"
                                 "rotation->truncated = 1;", "rotation->truncated = 1;"), "rotation-limits"),
    "entries-run-together": (('index ? ", " : ""', 'index ? " " : ""'), "round-trip"),
    "campaign-as-multiplayer": (("else if (type == 0 && !coop)", "else if (0)"), "check-kind"),
    "foot-unchecked": (('|| memcmp(header + 0x7FC, "toof", 4)', ""), "map-header"),
    "type-unsigned": (("(long)(short)(header[0x60]", "(long)(unsigned short)(header[0x60]"), "map-header"),
    "custom-level-kept": (('entry->map_kind = _server_map_custom;\n\t\t\tsnprintf(entry->level, sizeof(entry->level), '
                           '"custom_maps\\\\%s", entry->map_name);', "entry->map_kind = _server_map_custom;"),
                          "resolve"),
    "missing-unflagged": (("entry->map_name, entry->map_name, entry->map_name);\n\t\tentry->map_missing = 1;",
                           "entry->map_name, entry->map_name, entry->map_name);"), "resolve"),
    "campaign-listed": (("type != 1 ||", ""), "maps-each"),
    "custom-maps-case-sensitive": (("posix_find_entry_case_insensitive(folder, file_name, on_disk, sizeof(on_disk))",
                                    "0"), "resolve-case"),
}
# (controls a case-insensitive file system cannot see: it finds Mixed_Case.map by any spelling)
CASE_CONTROLS = {"custom-maps-case-sensitive"}


def generated(fault=None):
    code = read("port/linux/src/server_rotation.c")
    if fault:
        code = mutated(code, *fault)
    # (the platform's headers: the test's stubs stand for what the file calls)
    stub = "/* (the test's stubs) */\n"
    return (("server_rotation.h", read("port/linux/src/server_rotation.h")), ("platform.h", stub),
            ("port_config.h", stub), ("posix.h", stub), ("under_test.inc", code))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("server_rotation", generated()), case)
    assert status == 0, output


def case_insensitive_files():
    with tempfile.TemporaryDirectory() as folder:
        Path(folder, "Case").touch()
        return Path(folder, "case").exists()


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    if control in CASE_CONTROLS and case_insensitive_files():
        pytest.skip("this file system finds a file in any case")
    fault, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("server_rotation", generated(fault)), case)
    assert status == CHECK_FAILED, f"server_rotation.c with a fault ({control}) passed '{case}': the test cannot see it"

"""Exercise MCC traversal and the real shared callback dispatch without assets."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

import pytest

from harness import constant, structure

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / "port/linux/game"


def build_validation_tool(tmp_path_factory, hud=False):
    compiler = shutil.which("clang")
    if compiler is None:
        pytest.skip("clang is required for native MCC validation tests")
    directory = tmp_path_factory.mktemp("mcc-hud-validation" if hud else "mcc-validation")
    (directory / "cache").mkdir()
    (directory / "tag_files").mkdir()
    (directory / "cseries.h").write_text("""
#ifndef TEST_CSERIES_H
#define TEST_CSERIES_H
#include <stddef.h>
#include <stdlib.h>
void *validation_malloc(size_t size);
void validation_free(void *pointer);
/* Match cseries.h: malloc/free are paired wrappers; calloc is not. */
#define malloc validation_malloc
#define free validation_free
typedef unsigned char byte;
typedef unsigned char boolean;
typedef float real;
#define TRUE 1
#define FALSE 0
#define NONE (-1)
#define UNSIGNED_SHORT_MAX 65535
#define FLAG(n) (1UL << (n))
#define TEST_FLAG(v,n) (((v) & FLAG(n)) != 0)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
""")
    (directory / "cache/physical_memory_map.h").write_text("#define TAG_CACHE_SIZE 0x01600000\n")
    (directory / "tag_files/tag_groups.h").write_text("""
#ifndef TEST_TAG_GROUPS_H
#define TEST_TAG_GROUPS_H
#include "cseries.h"
struct tag_block { long count; void *address; void *definition; };
struct tag_data { long size; unsigned long pad; long file_offset; void *address; void *definition; };
struct tag_reference { unsigned long group_tag; char *name; long name_length; long index; };
#endif
""")
    (directory / "errors.h").write_text("#define _error_silent 0\nvoid error(int,const char *,...);\n")
    binary = directory / ("validation.exe" if os.name == "nt" else "validation")
    allocator = directory / "allocator.c"
    allocator.write_text('''
#include <stdlib.h>
static void *allocations[100000];
void *validation_malloc(size_t size) {
    unsigned i;
    void *p = malloc(size);
    if (!p) return NULL;
    for (i = 0; i < 100000; i++) if (!allocations[i]) {
        allocations[i] = p; return p;
    }
    abort();
}
void validation_free(void *p) {
    unsigned i;
    if (!p) abort(); /* cseries.h routes free to debug_free, which requires a pointer. */
    for (i = 0; i < 100000; i++) if (allocations[i] == p) {
        allocations[i] = NULL; free(p); return;
    }
    abort(); /* Detect mixing the engine allocator with CRT calloc. */
}
''')
    target = (["--target=i686-pc-windows-msvc", "-fuse-ld=lld", "-D_CRT_SECURE_NO_WARNINGS"]
              if sys.platform == "win32" else ["-m32"])
    command = [compiler, *target, "-std=gnu99", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
               "-Wno-multichar", "-fsanitize=undefined", "-fsanitize-trap=undefined",
               f"-I{directory}", f"-I{GAME}", str(GAME / "mcc_tag_validate.c"),
               str(GAME / "tag_validate.c"), str(GAME / "mcc_diagnostics.c"), str(ROOT / "tools/harness/tests/mcc_tag_validation.c"), str(allocator),
               "-o", str(binary)]
    if hud:
        declarations = []
        for path in ["source/interface/hud_definitions.h",
                     "source/interface/weapon_hud_interface_definition.h",
                     "source/bitmaps/bitmap_group.h"]:
            header = (ROOT / path).read_text()
            declarations.extend(structure(header, name) for name in re.findall(r"^struct (\w+)\s*\{", header, re.M))
        effects = (GAME / "tag_schema_effects.c").read_text()
        for name in ["weapon_hud_overlay_item", "weapon_hud_overlay_definition",
                     "weapon_hud_overlays_element", "weapon_hud_crosshair_definition",
                     "weapon_hud_crosshairs_element", "weapon_hud_crosshair_item"]:
            declarations.append(structure(effects, name))
        types = '''
#include "cseries.h"
#include "tag_files/tag_groups.h"
#include <stdint.h>
typedef unsigned short word;
typedef union point2d {struct {short x,y;};uint32_t value;} point2d;
typedef struct {float i,j;} real_vector2d;
typedef struct {short x0,y0,x1,y1;} rectangle2d;
typedef struct {float alpha,red,green,blue;} real_argb_color;
typedef struct {float red,green,blue;} real_rgb_color;
typedef struct {float x0,x1,y0,y1;} real_rectangle2d;
typedef struct {float x,y;} real_point2d;
#define NUMBEROF(a) (sizeof(a)/sizeof((a)[0]))
#define _hud_anchor_center 4
#define _error_silent 0
#define MCC_HUD_NATIVE_FRAME 0x4D53
'''
        (directory / "hud_test_types.h").write_text(types + "\n".join(declarations))
        schemas = []
        for name in ["NUMBER_OF_WEAPON_HUD_FLASH_REFERENCES", "NUMBER_OF_WEAPON_HUD_MAP_TYPES",
                     "NUMBER_OF_WEAPON_HUD_CROSSHAIR_STATES", "MAXIMUM_ITEMS_PER_HUD_OVERLAY",
                     "MAXIMUM_ITEMS_PER_HUD_CROSSHAIR", "MAXIMUM_ELEMENTS_PER_WEAPON_HUD",
                     "MAXIMUM_CROSSHAIRS_PER_WEAPON_HUD"]:
            schemas.append(f"#define {name} {constant(effects, name)}")
        for name in ["weapon_hud_overlay_item", "weapon_hud_overlay", "weapon_hud_overlays_element",
                     "weapon_hud_crosshair_item", "weapon_hud_crosshairs_element"]:
            fields = re.search(r"static struct tag_schema_field const " + name + r"_fields\[\]\s*=\s*\{.*?\n\};", effects, re.S)
            schema = re.search(r"static struct tag_schema_definition const " + name + r"_schema\s*=.*?;", effects, re.S)
            assert fields and schema, name
            schemas.extend([fields.group(), schema.group()])
        (directory / "hud_test_schemas.inc").write_text("\n".join(schemas))
        hud_source = (GAME / "mcc_hud.c").read_text()
        # Keep production helper includes; replace only platform/header dependencies.
        hud_source = "\n".join(line for line in hud_source.splitlines()
                               if not line.startswith("#include ") or line.endswith('.inl"'))
        hud_file = directory / "hud.c"
        hud_file.write_text('''
#include "hud_test_types.h"
#include "mcc_runtime.h"
#include <string.h>
void error(short level,char const *text,...);
''' + hud_source)
        command[1:1] = ["-DMCC_HUD_VALIDATION", str(hud_file)]
    built = subprocess.run(command, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    if hud:
        return binary
    # The existing map_validate executable links tag_validate.c without any
    # game runtime adapters. MCC callback routing must retain that property.
    legacy = directory / "legacy.c"
    legacy.write_text('''
#include "cseries.h"
#include "tag_schema.h"
struct tag_schema_group const *const tag_schema_group_lists[] = {NULL};
struct tag_schema_group const tag_schema_custom_edition_groups[] = {{0, {0, 0}, NULL}};
void tag_validate_report(char const *message) { (void)message; }
int main(void) { return tag_validate_corrections() != 0; }
''')
    legacy_binary = directory / ("legacy.exe" if os.name == "nt" else "legacy")
    standalone = command[:command.index(str(GAME / "mcc_tag_validate.c"))] + [
        str(GAME / "tag_validate.c"), str(legacy), "-o", str(legacy_binary)]
    linked = subprocess.run(standalone, capture_output=True, text=True)
    assert linked.returncode == 0, linked.stdout + linked.stderr
    assert subprocess.run([str(legacy_binary)], timeout=30).returncode == 0
    return binary


@pytest.fixture(scope="module")
def validation_tool(tmp_path_factory):
    return build_validation_tool(tmp_path_factory)


@pytest.fixture(scope="module")
def hud_validation_tool(tmp_path_factory):
    return build_validation_tool(tmp_path_factory, hud=True)


@pytest.mark.parametrize("case", [
    "hud_scalar_first", "hud_scalar_last", "hud_raw_scalar_first", "hud_raw_scalar_last",
    "hud_root_alias", "hud_root_partial", "hud_root_schema", "hud_owner_partial", "hud_owner_schema",
    "hud_prefix_short_first", "hud_prefix_long_first", "hud_tail_short_first", "hud_tail_long_first",
])
def test_hud_conversion_preserves_validation(hud_validation_tool, case):
    result = subprocess.run([str(hud_validation_tool), case], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("case", ["model_part_owned", "model_part_rejected", "jet_pair", "jet_state", "jet_complete", "jet_unused", "jet_short", "jet_outside", "jet_capacity", "jet_explosion", "valid", "collision_valid", "collision_swapped", "collision_no_owner", "collision_short_ring", "collision_open_ring", "collision_bad_next", "collision_bad_owner", "collision_outside", "overlap", "outside", "negative", "cycle", "alias", "alias_partial", "alias_group", "alias_bad_child", "alias_block", "alias_block_partial",
                                     "graph_model_complete", "graph_model_pruned", "graph_model_pruned_parent", "graph_model_pruned_sibling", "graph_model_pruned_cycle", "graph_model_pruned_child", "graph_model_pruned_name", "graph_model_pruned_model_cycle", "graph_model", "graph_model_partial", "graph_model_partial_omitted_parents", "graph_model_partial_conflict", "graph_model_partial_cycle", "graph_model_holes", "graph_model_holes_parent", "graph_model_holes_cycle", "graph_model_holes_name", "graph_model_holes_link", "graph_model_holes_model_cycle", "graph_model_mismatch", "graph_model_outside", "graph_model_cycle", "graph_model_tail", "graph_model_tail_conflict", "graph_model_tail_cycle", "graph_model_empty", "graph_model_empty_partial", "damage_names",
                                     "prefix_smaller", "prefix_expand", "prefix_tail_overlap", "empty_weapon", "empty_weapon_nonzero",
                                     "animation_data", "animation_data_outside",
                                     "file_range", "handle", "unknown_group", "bsp",
                                     "legacy_isolation", "callbacks", "misaligned", "stream_hole",
                                     "collection", "collection_outside", "device", "device_outside",
                                     "trimmed_structure", "graph_large", "graph_cycle", "graph_node_cycle",
                                     "graph_orphan", "graph_missing_parent", "graph_conflicting_parent", "graph_missing_parent_cycle", "graph_root_sibling", "graph_root_outside", "graph_root_orphan", "bitmap_owned", "bitmap_unowned", "first_person_slots",
                                     "grenades_zero", "grenades_one", "grenades_two", "grenades_four", "grenades_excess",
                                     "anchor_0", "anchor_4", "anchor_5", "anchor_6", "anchor_7", "anchor_8", "anchor_9", "anchor_-1",
                                     "flag_name", "names_512", "names_513", "names_640", "names_32767", "names_32768",
                                     "palette_100", "palette_101", "palette_32767", "palette_32768",
                                     "palette_bad_index", "palette_outside", "palette_overlap",
                                     "syntax_full", "syntax_overflow", "syntax_outside", "syntax_overlap", "syntax_legacy"])
def test_mcc_validation(validation_tool, case):
    result = subprocess.run([str(validation_tool), case], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr


def test_rejected_nested_field_has_authoring_path(validation_tool):
    result = subprocess.run([str(validation_tool), "nested_data"], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "scenario.scenario" in result.stdout
    assert "children[0]" in result.stdout
    assert "children[0].data" in result.stdout
    assert "data overlaps another allocation" in result.stdout

"""Exercise MCC model/BSP conversion, the real vertex compressor, and cleanup.

GPU allocation is mocked. These checks establish data/ownership behavior;
they do not substitute for an in-game rendering test.
"""
import os
import re
from pathlib import Path
import shutil
import struct
import subprocess
import sys

import pytest

from harness import function, structure

ROOT = Path(__file__).resolve().parents[1]


def geometry_map():
    base, tag_offset = 0x50000000, 0xC00
    data = bytearray(0x1700)
    def u(o, v):
        struct.pack_into("<I", data, o, v)
    def cc(s):
        return int.from_bytes(s.encode(), "big")
    for o, v in {0: cc("head"), 4: 13, 8: len(data), 16: tag_offset, 20: 0xB00,
                 0x68: 7, 0x7FC: cc("foot"), tag_offset: base+40,
                 tag_offset+4: 0xE1740000, tag_offset+12: 3,
                 tag_offset+20: 0xB00, tag_offset+28: 204, tag_offset+32: 210,
                 tag_offset+36: cc("tags")}.items():
        u(o, v)
    data[32:41] = b"geometry\0"
    data[64:69] = b"test\0"
    for i, (group, name, address) in enumerate([
            ("scnr", 0x90, 0x200), ("sbsp", 0x95, 0), ("mod2", 0x99, 0x800)]):
        struct.pack_into("<8I", data, tag_offset+40+i*32, cc(group), 0xFFFFFFFF,
                         0xFFFFFFFF, 0xE1740000+i*0x10001, base+name,
                         base+address if address else 0, 0, 0)
    data[tag_offset+0x90:tag_offset+0x9D] = b"scnr\0bsp\0mod\0"
    bsp = base+0x4000000-0x300
    struct.pack_into("<6I", data, 0x800, bsp+24, 0, 0, 0, 0, cc("sbsp"))
    struct.pack_into("<3I", data, tag_offset+0x200+0x5A4, 1, base+0x7B0, 0)
    struct.pack_into("<8I", data, tag_offset+0x7B0, 0x800, 0x300, bsp, 0,
                     cc("sbsp"), base+0x95, 0, 0xE1750001)
    for o, v in {0x8B8: 1, 0x8BC: base+0xA00, 0x8D0: 1, 0x8D4: base+0x900,
                 0x8DC: 1, 0x8E0: base+0xAA0, 0x924: 1, 0x928: base+0x940,
                 0x984: 1, 0x988: 1, 0x994: 4, 0x998: 3}.items():
        u(tag_offset+o, v)
    data[tag_offset+0x946:tag_offset+0x948] = b"\xff\xff"
    for i in range(3):
        struct.pack_into("<14f2h2f", data, 0xB00+i*68,
                         float(i == 1), float(i == 2), 0., 0., 0., 1., 0., 1., 0.,
                         1., 0., 0., 0., 0., 0, 0, 1., 0.)
    struct.pack_into("<3H", data, 0xB00+204, 0, 1, 2)
    return data


@pytest.fixture(scope="session")
def geometry_tool(tmp_path_factory):
    compiler = shutil.which("clang")
    if not compiler or sys.platform != "win32":
        pytest.skip("this harness uses the native x86 Windows game headers")
    output = tmp_path_factory.mktemp("mcc-geometry")/"geometry.exe"
    includes = ["port/windows/include/crt", "port/windows/include", "source", "source/cseries",
                "source/math", "source/tag_files", "source/rasterizer", "source/structures",
                "source/cache", "source/render", "port/include/xdk", "port/linux/game"]
    command = [compiler, "-include", str(ROOT / "port/linux/game/mcc_loading.h"), str(ROOT / "tools/mcc_loading_stub.c"), "--target=i686-pc-windows-msvc", "-fuse-ld=lld", "-fms-extensions",
               "-std=gnu89", "-O1", "-DDEBUG", "-Dxbox", "-D_CRT_SECURE_NO_WARNINGS",
               "-Wno-nonportable-include-path", "-Wno-c99-compat", "-Wno-visibility",
               "-include", str(ROOT/"port/windows/include/halo_windows_prefix.h"),
               "-iquote", str(ROOT/"port/linux/include")]
    command += ["-I"+str(ROOT/p) for p in includes]
    command += [str(ROOT/p) for p in ["port/tools/mcc_geometry_check.c",
                "port/linux/game/mcc_geometry.c", "port/linux/game/mcc_diagnostics.c", "port/linux/game/mcc_cache_format.c",
                "source/rasterizer/rasterizer_geometry.c"]]
    # Exercise the real legacy callback with the same long strips. Its
    # original 16-bit count limit must remain effective for Xbox/CE.
    schema = (ROOT / "port/linux/game/tag_schema_models.c").read_text()
    sizes = re.search(r"static short const model_vertex_type_sizes\[.*?\n};", schema, re.S).group()
    legacy = output.parent / "legacy.c"
    legacy.write_text("""
#include "cseries.h"
#include "tag_files/tag_groups.h"
#include "rasterizer/rasterizer_geometry.h"
#include "models/model_definitions.h"
#include "tag_schema.h"
#include <string.h>
static byte vertex_data[96];
static word index_data[70945];
static int corrections;
void tag_validate_correct(struct tag_validation *v,char const *f,...) {(void)v;(void)f;corrections++;}
boolean tag_validate_contains(struct tag_validation *v,void const *p,unsigned long n)
{(void)v;return (p==vertex_data && n<=sizeof(vertex_data)) || (p==index_data && n<=sizeof(index_data));}
boolean tag_validate_any_claimed(void const *p,unsigned long n) {(void)p;(void)n;return FALSE;}
void *tag_validate_vertex_buffer_data(struct tag_validation *v,void const *p) {(void)v;(void)p;return vertex_data;}
void *tag_validate_index_buffer_data(struct tag_validation *v,void const *p) {(void)v;(void)p;return index_data;}
""" + structure(schema, "model_geometry_part") + "\n" + sizes + "\n" +
        function(schema, "model_geometry_part_check") + """
int legacy_model_limits_check(void) {
    struct model_geometry_part part;
    long counts[]={65535,65536,70943};
    int i;
    for(i=0;i<3;i++) {
        memset(&part,0,sizeof(part));corrections=0;
        part.vertex_buffer.type=_rasterizer_vertex_type_model_compressed;
        part.vertex_buffer.count=3;part.vertex_buffer.hardware_format=vertex_data;
        part.triangle_buffer.type=_triangle_buffer_type_precompiled_strip;
        part.triangle_buffer.count=counts[i];part.triangle_buffer.hardware_format=index_data;
        if(!model_geometry_part_check(NULL,&part)) return 0;
        if(i==0 ? (!part.triangle_buffer.hardware_format || corrections) :
            (part.triangle_buffer.hardware_format || corrections!=1)) return 0;
    }
    return 1;
}
""")
    command += [str(legacy)]
    command += ["-o", str(output), "-lkernel32"]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return output


def run(tool, path, *args):
    result = subprocess.run([str(tool), str(path), *map(str, args)], capture_output=True, text=True)
    assert result.returncode in (0, 1), result.stderr
    fields = dict(line.split("=", 1) for line in result.stdout.splitlines() if "=" in line)
    assert fields["active_buffers"] == "0", result.stderr
    return result.returncode, fields


def test_model_conversion_and_dispose(geometry_tool, tmp_path):
    path = tmp_path/"geometry.map"
    path.write_bytes(geometry_map())
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert fields["created_buffers"] == "2"


@pytest.mark.parametrize("failure_at", [0, 1])
def test_partial_gpu_failure_cleanup(geometry_tool, tmp_path, failure_at):
    path = tmp_path/"geometry.map"
    path.write_bytes(geometry_map())
    code, _ = run(geometry_tool, path, failure_at)
    assert code == 1


@pytest.mark.parametrize("shader", [0, 0xffff, 1, 0x8000])
def test_model_material_correction_preserves_geometry(geometry_tool, tmp_path, shader):
    data = geometry_map()
    struct.pack_into("<H", data, 0xC00+0x940+4, shader)
    path = tmp_path / "material.map"
    path.write_bytes(data)
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert fields["created_buffers"] == "2"
    assert fields["shader_sum"] == ("-1" if shader == 0xffff else "0")
    result = subprocess.run([str(geometry_tool), str(path)], capture_output=True, text=True)
    assert ("missing material" in result.stderr) == (shader not in (0, 0xffff))


def test_missing_entire_material_list_is_refused(geometry_tool, tmp_path):
    data = geometry_map()
    struct.pack_into("<I", data, 0xC00+0x8DC, 0)
    path = tmp_path / "no_materials.map"
    path.write_bytes(data)
    code, _ = run(geometry_tool, path)
    assert code == 1


@pytest.mark.parametrize("local", [False, True])
def test_many_node_palette_is_owned_by_part(geometry_tool, tmp_path, local):
    data = geometry_map()
    # Give the model 64 real node records outside the original tag payload.
    node_offset = len(data) - 0xC00
    data.extend(bytes(64 * 156))
    struct.pack_into("<I", data, 8, len(data))
    struct.pack_into("<I", data, 20, len(data) - 0xC00)
    struct.pack_into("<2I", data, 0xC00 + 0x8B8, 64, 0x50000000 + node_offset)
    if local:
        struct.pack_into("<I", data, 0xC00 + 0x800, 2)
        data[0xC00 + 0x940 + 0x6B] = 1
        data[0xC00 + 0x940 + 0x6C] = 63
    else:
        for vertex in range(3):
            struct.pack_into("<2h", data, 0xB00 + vertex * 68 + 56, 63, 63)
    path = tmp_path / "palette.map"
    path.write_bytes(data)
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert fields["palette_parts"] == "1"
    assert fields["palette_sum"] == "63"


@pytest.mark.parametrize("offset,encoding,value", [
    (0xC00+0x940+0x64, "I", 0xFFFFFFFC),
    (0xB00+204, "H", 7),
    (0xB00+56, "h", 9),
    (0xB00+12, "f", float("nan")),
    (0xB00, "f", float("nan")),
    (0xB00+48, "f", float("inf")),
])
def test_invalid_model_data_is_refused(geometry_tool, tmp_path, offset, encoding, value):
    data = geometry_map()
    struct.pack_into("<"+encoding, data, offset, value)
    path = tmp_path/"bad.map"
    path.write_bytes(data)
    code, _ = run(geometry_tool, path)
    assert code == 1


def test_optional_real_geometry(geometry_tool):
    fixture = os.environ.get("MCC_TEST_MAP")
    if not fixture:
        pytest.skip("set MCC_TEST_MAP to opt in to local fixture conversion")
    code, fields = run(geometry_tool, Path(fixture))
    assert code == 0
    assert int(fields["created_buffers"]) > 0


@pytest.mark.parametrize("uvs", [False, True])
def test_missing_model_tangent_frames_are_reconstructed(geometry_tool, tmp_path, uvs):
    data = geometry_map()
    for vertex in range(3):
        struct.pack_into("<6f", data, 0xB00+vertex*68+24, *([float("nan")]*6))
    if uvs:
        struct.pack_into("<2f", data, 0xB00+68+48, 1., 0.)
        struct.pack_into("<2f", data, 0xB00+136+48, 0., 1.)
    path = tmp_path/"missing_tangents.map"
    path.write_bytes(data)
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert int(fields["created_buffers"]) > 0


def large_strip_map(triangles, *, declared=None, repair=False, vertices=3):
    data = geometry_map()
    tag_offset = 0xC00
    tags = data[tag_offset:]
    vertex_data = bytearray(data[0xB00:0xB00+204] * (vertices // 3))
    vertex_data.extend(data[0xB00:0xB00+(vertices % 3)*68])
    if repair:
        for vertex in range(vertices):
            struct.pack_into("<6f", vertex_data, vertex*68+24, *([float("nan")]*6))
    stream = struct.pack("<" + "H"*(triangles+2), *[i % vertices for i in range(triangles+2)])
    tag_offset = (0xB00+len(vertex_data)+len(stream)+3) & ~3
    result = data[:0xB00] + vertex_data + stream
    result.extend(bytes(tag_offset-len(result)))
    result.extend(tags)
    struct.pack_into("<I", result, 8, len(result))
    struct.pack_into("<I", result, 16, tag_offset)
    struct.pack_into("<I", result, tag_offset+28, len(vertex_data))
    struct.pack_into("<I", result, tag_offset+32, len(vertex_data)+len(stream))
    struct.pack_into("<I", result, tag_offset+0x998, vertices)
    struct.pack_into("<I", result, tag_offset+0x988, triangles if declared is None else declared)
    return result


@pytest.mark.parametrize("triangles", [65535, 65536, 70943, 1048576])
def test_large_strip_preserves_count_and_validates_owned_buffers(geometry_tool, tmp_path, triangles):
    path = tmp_path / "large.map"
    path.write_bytes(large_strip_map(triangles))
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert int(fields["maximum_triangles"]) == triangles


@pytest.mark.parametrize("count", [0, 1048577, 0x7fffffff, 0xfffffffe, 0xffffffff])
def test_untrusted_strip_counts_are_rejected(geometry_tool, tmp_path, count):
    path = tmp_path / "overflow.map"
    path.write_bytes(large_strip_map(1, declared=count))
    code, _ = run(geometry_tool, path)
    assert code == 1


@pytest.mark.parametrize("fault", ["short_stream", "last_index", "offset", "vertex_count"])
def test_large_strip_bounds(geometry_tool, tmp_path, fault):
    data = large_strip_map(70943)
    tags = struct.unpack_from("<I", data, 16)[0]
    if fault == "short_stream":
        struct.pack_into("<I", data, tags+32, 204+70945*2-2)
    elif fault == "last_index":
        struct.pack_into("<H", data, 0xB00+204+70944*2, 3)
    elif fault == "offset":
        struct.pack_into("<I", data, tags+0x98C, 0xfffffffe)
    else:
        struct.pack_into("<I", data, tags+0x998, 65536)
    path = tmp_path / "bad-large.map"
    path.write_bytes(data)
    code, _ = run(geometry_tool, path)
    assert code == 1


def test_large_strip_tangent_repair(geometry_tool, tmp_path):
    path = tmp_path / "repair-large.map"
    path.write_bytes(large_strip_map(70943, repair=True))
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert fields["maximum_triangles"] == "70943"


@pytest.mark.parametrize("failure_at", [0, 1])
def test_large_strip_failed_gpu_allocation_cleanup(geometry_tool, tmp_path, failure_at):
    path = tmp_path / "large-failure.map"
    path.write_bytes(large_strip_map(70943))
    code, _ = run(geometry_tool, path, failure_at)
    assert code == 1


def test_maximum_strip_and_vertices_with_tangent_scratch(geometry_tool, tmp_path):
    path = tmp_path / "maximum.map"
    path.write_bytes(large_strip_map(1048576, vertices=65535, repair=True))
    code, fields = run(geometry_tool, path)
    assert code == 0
    assert fields["maximum_triangles"] == "1048576"


@pytest.mark.parametrize("allocation", range(9))
def test_large_strip_cpu_allocation_failure_cleanup(geometry_tool, tmp_path, allocation):
    path = tmp_path / "allocation.map"
    path.write_bytes(large_strip_map(70943, repair=True))
    code, _ = run(geometry_tool, path, -1, allocation)
    assert code == 1


@pytest.mark.parametrize("offset,encoding,value,reason", [
    (0xB00 + 204, "H", 7, "triangle_indices[0]=7 exceeds vertex count 3"),
    (0xB00 + 56, "h", 9, "vertices[0].nodes[0] references 9"),
    (0xB00, "f", float("nan"), "vertices[0].position[0]"),
])
def test_geometry_failure_names_source_model_and_element(geometry_tool, tmp_path, offset, encoding, value, reason):
    data = geometry_map()
    struct.pack_into("<" + encoding, data, offset, value)
    path = tmp_path / "diagnostic.map"
    path.write_bytes(data)
    result = subprocess.run([str(geometry_tool), str(path)], capture_output=True, text=True)
    assert result.returncode == 1
    assert "'mod.gbxmodel'" in result.stderr
    assert "geometries[0].parts[0]" in result.stderr
    assert reason in result.stderr
    assert "active_buffers=0" in result.stdout

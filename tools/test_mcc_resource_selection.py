"""Local resource choice retains full hashes, package precedence and stamp checks."""
from pathlib import Path
import struct
import subprocess
import pytest
from harness import function
from mcc_test_support import compile_tool

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / 'port/linux/game'


def build_resource_tool(work, android=False):
    game = GAME
    for name in ('mcc_resources.h', 'mcc_package.h', 'mcc_validation.h', 'mcc_resource_check.inl', 'mcc_validation.inl'):
        (work / name).write_text((game / name).read_text())
    (work / 'cseries.h').write_text('typedef int boolean;\ntypedef unsigned char byte;\n')
    (work / 'errors.h').write_text('#define _error_silent 0\n#define error(...) ((void)0)\n')
    (work / 'xtl.h').write_text('''
#ifndef _WIN32
#define __stdcall
#endif
#define TRUE 1
#define FALSE 0
#define WAIT_OBJECT_0 0
#define INFINITE (~0u)
#define CreateEventA(a,b,c,d) NULL
#define CreateThread(a,b,c,d,e,f) ((void)(c),NULL)
#define SetEvent(h) ((void)(h))
#define WaitForSingleObject(h,t) ((void)(h),(void)(t),WAIT_OBJECT_0)
#define CloseHandle(h) ((void)(h))
''')
    resources = (game / 'mcc_resources.c').read_text().replace('../../third_party/zlib/zlib_prefixed.h',
        (ROOT / 'port/third_party/zlib/zlib_prefixed.h').as_posix())
    planner = (GAME / 'mcc_share_package.c').read_text()
    code = (ROOT / 'tools/tests/mcc_resource_selection.c').read_text()
    code = code.replace('/* RESOURCES */', resources).replace('/* PLANNER */',
        '\n'.join(function(planner, n) for n in ('base_resource_copy', 'base_resources')))
    (work / 'driver.c').write_text(code)
    (work / 'crc.c').write_text('#define Z_PREFIX\n#include "' + (ROOT / 'port/third_party/zlib/crc32.c').as_posix() + '"\n')
    # Deterministic access denial on every host, including an elevated Windows run.
    package = work / 'package.c'
    package.write_text('#include <stdio.h>\n#include <string.h>\n#include <errno.h>\n'
        'extern int denied;\nstatic FILE *test_open(char const *path,char const *mode) {\n'
        'if(denied && strstr(path,"mcc_maps/p/sounds.map")) {errno=EACCES;return NULL;}\n'
        'return fopen(path,mode); }\n#define fopen test_open\n#include "' +
        (game / 'mcc_package.c').as_posix() + '"\n')
    if android:
        # (the same driver as Android builds it: its host hash jobs in play)
        (work / 'android.c').write_text('#define HALO_ANDROID 1\n#include "driver.c"\n')
        return compile_tool(work, [work / 'android.c', package, GAME / 'mcc_sha256.c', work / 'crc.c'])
    return compile_tool(work, [work / 'driver.c', package, GAME / 'mcc_sha256.c',
                              work / 'crc.c'])


@pytest.fixture(scope='module')
def resource_tool(tmp_path_factory):
    return build_resource_tool(tmp_path_factory.mktemp('resource-choice'))


def test_early_failure_cancels_no_host_job(tmp_path):
    # (a check failing before its host job is set must not cancel the host's job 0)
    tool = build_resource_tool(tmp_path, android=True)
    result = subprocess.run([str(tool), 'early_failure'], capture_output=True, text=True, cwd=tmp_path)
    assert result.returncode == 0, result.stderr
    assert result.stdout == 'none 0\n'


@pytest.mark.parametrize('case', ['match', 'mismatch', 'override', 'unused', 'changed', 'cache', 'stage', 'both',
    'owned_mismatch', 'unused_mismatch', 'all_mismatch', 'denied'])
def test_local_resources(resource_tool, tmp_path, case):
    def resource(value, kind=2):
        names = b'sound\\test__0__0\0' if kind == 2 else b'sound\\test_0\0'
        return struct.pack('<4I', kind, 20, 20+len(names), 1) + value + names + struct.pack('<3I', 0, 4, 16)
    for folder in ('p', '_shared/singleplayer', '_shared/multiplayer'):
        (tmp_path / 'mcc_maps' / folder).mkdir(parents=True, exist_ok=True)
    (tmp_path / 'mcc_maps/p/a.map').write_bytes(bytes(2048))
    (tmp_path / 'mcc_maps/sounds.map').write_bytes(resource(b'old!'))
    (tmp_path / 'mcc_maps/_shared/singleplayer/sounds.map').write_bytes(b'bad')
    (tmp_path / 'mcc_maps/_shared/multiplayer/sounds.map').write_bytes(resource(b'new!'))
    (tmp_path / 'mcc_staging/p').mkdir(parents=True)
    (tmp_path / 'mcc_staging/p/a.map').write_bytes(bytes(2048))
    (tmp_path / 'mcc_maps/bitmaps.map').write_bytes(resource(b'old!', 1))
    (tmp_path / 'mcc_maps/_shared/singleplayer/bitmaps.map').write_bytes(resource(b'new!', 1))
    if case in ('owned_mismatch', 'denied'):
        (tmp_path / 'mcc_maps/p/sounds.map').write_bytes(resource(b'new!'))
    if case == 'owned_mismatch':
        (tmp_path / 'mcc_maps/p/bitmaps.map').write_bytes(resource(b'new!', 1))
    if case == 'all_mismatch':
        (tmp_path / 'mcc_maps/_shared/singleplayer/sounds.map').write_bytes(resource(b'mid!'))
        (tmp_path / 'mcc_maps/_shared/multiplayer/bitmaps.map').write_bytes(resource(b'end!', 1))
    result = subprocess.run([str(resource_tool), case], cwd=tmp_path, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr

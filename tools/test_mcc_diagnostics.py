"""MCC diagnostic names remain bounded and preserve the original authoring tag."""
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture(scope="module")
def diagnostic_tool(tmp_path_factory):
    compiler = shutil.which("clang")
    if not compiler:
        pytest.skip("clang is required")
    work = tmp_path_factory.mktemp("mcc-diagnostics")
    (work / "cseries.h").write_text("")
    (work / "errors.h").write_text("#define _error_silent 0\nvoid error(int,const char *,...);\n")
    source = work / "test.c"
    source.write_text(r'''
#include "mcc_runtime.h"
#include "mcc_errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static unsigned char data[1024];
static uint32_t original[8];
void error(int level,char const *format,...) {
    va_list args;(void)level;va_start(args,format);vprintf(format,args);va_end(args);putchar(10);
}
void *mcc_runtime_pointer(struct mcc_runtime *r,uint32_t address,uint32_t size) {
    uint32_t offset=address-r->report.tag_base;
    return address>=r->report.tag_base && offset<=r->used && size<=r->used-offset ? r->tags+offset : NULL;
}
static int read_original(void *context,uint32_t offset,uint32_t size,void *out) {
    (void)context;if(offset!=40 || size!=32)return 0;memcpy(out,original,32);return 1;
}
int main(int argc,char **argv) {
    struct mcc_runtime r={0};uint32_t *entry=(uint32_t *)data;char first[320];
    if(argc!=2)return 2;
    r.tags=r.tag_index=data;r.used=sizeof(data);r.report.tag_base=0x50000000;r.report.tag_count=1;
    entry[0]='bitm';entry[3]=0xe91e0000;entry[4]=r.report.tag_base+128;
    strcpy((char *)data+128,"weapons\\assault rifle\\bitmaps\\ar_multipurpose");
    if(!strcmp(argv[1],"boundary"))r.used=128+(uint32_t)strlen((char *)data+128)+1;
    if(!strcmp(argv[1],"bad-pointer"))entry[4]=0xfffffff0;
    if(!strcmp(argv[1],"unterminated")){memset(data+128,'x',16);r.used=144;}
    if(!strcmp(argv[1],"long"))memset(data+128,'x',256);
    if(!strcmp(argv[1],"control"))strcpy((char *)data+128,"name\n%n");
    if(!strcmp(argv[1],"original-class")) {
        entry[0]='mode';memcpy(original,entry,sizeof(original));original[0]='mod2';
        r.source.read=read_original;r.source.size=sizeof(data);r.report.identity.tag_data_size=sizeof(data);
    }
    if(!strcmp(argv[1],"scenario")) {
        r.report.scenario_handle=entry[3];strcpy(r.original_scenario_tag,"levels\\original\\level.scenario");
        strcpy((char *)data+128,"mcc_maps\\package\\renamed");
    }
    mcc_diagnostic_tag(&r,entry[3]);mcc_diagnostic_at(&r,"bitmaps[%u]",3u);
    if(mcc_diagnostic_fail(&r,"pixel payload is too short: %u bytes available, %u required",4u,8u))return 3;
    if(r.failure[0])return 6; /* Log details must not override the loader prompt. */
    strcpy(r.failure,"Existing player explanation. See debug.txt.");
    memcpy(first,r.failure,sizeof(first));
    if(!strcmp(argv[1],"first-failure")) {
        mcc_diagnostic_fail(&r,"later failure");if(memcmp(first,r.failure,sizeof(first)))return 4;
        memset(&r,0,sizeof(r));mcc_diagnostic_tag(&r,UINT32_MAX);mcc_diagnostic_fail(&r,"next map failure");
        if(r.failure[0])return 5;
        {
            char prompt[512];
            mcc_error_message("mcc_maps/example",r.failure[0] ? r.failure :
                "Its textures could not be converted. See debug.txt for the failing texture check.",prompt,sizeof(prompt));
            printf("PROMPT: %s\n",prompt);
        }
    }
    return 0;
}
''')
    output = work / ("diagnostics.exe" if sys.platform == "win32" else "diagnostics")
    command = [compiler, "-std=gnu89", "-Wall", "-Wextra", "-Werror", "-Wno-multichar",
               "-I" + str(work), "-I" + str(ROOT / "port/linux/game"), str(source),
               str(ROOT / "port/linux/game/mcc_diagnostics.c"), str(ROOT / "port/linux/game/mcc_errors.c"), "-o", str(output)]
    if sys.platform == "win32":
        command[1:1] = ["--target=i686-pc-windows-msvc", "-fuse-ld=lld", "-D_CRT_SECURE_NO_WARNINGS"]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return output


@pytest.mark.parametrize("case,expected", [
    ("ordinary", r"weapons\assault rifle\bitmaps\ar_multipurpose.bitmap"),
    ("boundary", r"weapons\assault rifle\bitmaps\ar_multipurpose.bitmap"),
    ("bad-pointer", "(tag path unavailable)"),
    ("unterminated", "(tag path unavailable)"),
    ("long", "....bitmap"),
    ("control", "name?%n.bitmap"),
    ("original-class", "ar_multipurpose.gbxmodel"),
    ("scenario", r"levels\original\level.scenario"),
    ("first-failure", "next map failure"),
])
def test_bounded_original_identity_and_failure_lifetime(diagnostic_tool, case, expected):
    result = subprocess.run([str(diagnostic_tool), case], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    assert expected in result.stdout
    assert "bitmaps[3]" in result.stdout
    assert "4 bytes available, 8 required" in result.stdout
    assert max(map(len, result.stdout.splitlines())) < 1022


def test_conversion_details_preserve_established_player_prompt(diagnostic_tool):
    result = subprocess.run([str(diagnostic_tool), "first-failure"], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    prompt = next(line.removeprefix("PROMPT: ") for line in result.stdout.splitlines() if line.startswith("PROMPT: "))
    assert prompt.startswith("MCC example: Its textures could not be converted.")
    assert "debug.txt" in prompt
    assert "pixel payload" not in prompt

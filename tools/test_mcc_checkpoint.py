"""Actual MCC snapshot module and native save dispatch behavior, without assets."""
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import zlib

import pytest
from harness import function

ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture(scope="session")
def core_header_tool(tmp_path_factory, request):
    """Exercise the real core header gate before any state or callback is published."""
    compiler = shutil.which("clang") or shutil.which("cc")
    if not compiler:
        pytest.skip("a C compiler is needed for core header tests")
    work = tmp_path_factory.mktemp("core-header")
    native = (ROOT / "source/saved games/game_state.c").read_text()
    gate = function(native, "game_state_header_valid")
    if getattr(request, "param", None) == "assert-old-layout":
        gate = gate.replace("/* port: a different build's saved allocation layout",
            'match_vassert("fault", 413, FALSE, "allocation mismatch");\n'
            "\t\t/* port: a different build's saved allocation layout")
    source = r'''
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char boolean;
#define TRUE 1
#define FALSE 0
#define GAME_STATE_SIZE 64
#define csstrcmp strcmp
#define csprintf sprintf
#define _error_silent 0
static int assertions,diagnostics,reads,before,after;
#define match_vassert(file,line,test,text) do {if(!(test))assertions++;} while(0)
struct game_state_header {char map_name[256];uint32_t allocation_size_checksum,cache_file_checksum;short player_count;};
static struct game_state_header incoming;
static unsigned char live[GAME_STATE_SIZE],saved[GAME_STATE_SIZE];
static struct {void *base_address;uint32_t allocation_size_checksum;} game_state_globals;
static short player_spawn_count=1;
static long global_scenario_index;
static char map_name[256],temporary[512];
static char const *tag_get_name(long index) {(void)index;return map_name;}
static uint32_t cache_files_get_checksum(void) {return 0x12345678;}
static void error(int level,char const *format,...) {(void)level;(void)format;diagnostics++;}
static int game_state_read_core_header(char const *name,void *out,long size)
{(void)name;memcpy(out,&incoming,size);return 1;}
static void game_state_read_core(char const *name,void *out,long size)
{(void)name;reads++;memcpy(out,saved,size);}
static void game_state_note_event(char const *name) {(void)name;}
static void game_state_call_before_load_procs(void) {before++;}
static void game_state_call_after_load_procs(void) {after++;}
static void console_printf(int flag,char const *format,...) {(void)flag;(void)format;}
''' + gate + "\n" + function(native, "game_state_load_core") + r'''
int main(int argc,char **argv) {
    int valid,expected_assertions;
    if(argc!=3)return 2;
    strcpy(map_name,argv[1]);strcpy(incoming.map_name,map_name);
    incoming.allocation_size_checksum=game_state_globals.allocation_size_checksum=0x272335b8;
    incoming.cache_file_checksum=cache_files_get_checksum();incoming.player_count=player_spawn_count;
    game_state_globals.base_address=live;memset(saved,0xA5,sizeof(saved));
    valid=!strcmp(argv[2],"valid");expected_assertions=!valid&&strcmp(argv[2],"allocation");
    if(!strcmp(argv[2],"allocation"))incoming.allocation_size_checksum=0x67ec7664;
    else if(!strcmp(argv[2],"map"))strcpy(incoming.map_name,"another map");
    else if(!strcmp(argv[2],"players"))incoming.player_count++;
    else if(!strcmp(argv[2],"checksum"))incoming.cache_file_checksum++;
    else if(!valid)return 2;
    game_state_load_core("test");
    if(assertions!=expected_assertions || diagnostics!=!valid || reads!=valid || before!=valid || after!=valid)return 1;
    if(valid ? memcmp(live,saved,sizeof(live))!=0 : memcmp(live,(unsigned char[GAME_STATE_SIZE]){0},sizeof(live))!=0)return 1;
    return 0;
}
'''
    input_path = work / "core_header.c"
    input_path.write_text(source)
    binary = work / ("core_header.exe" if sys.platform == "win32" else "core_header")
    result = subprocess.run([compiler, "-m32", "-std=gnu89", "-O2", "-Wall", "-Werror", "-D_CRT_SECURE_NO_WARNINGS",
        "-Wno-unused-variable", str(input_path), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    return binary


@pytest.mark.parametrize("family", [r"levels\test\test", r"custom_maps\test", r"mcc_maps\package\test"])
@pytest.mark.parametrize("header", ["valid", "allocation", "map", "players", "checksum"])
def test_core_header_admission(core_header_tool, family, header):
    result = subprocess.run([str(core_header_tool), family, header], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("core_header_tool", ["assert-old-layout"], indirect=True)
def test_core_layout_refusal_detects_assertion(core_header_tool):
    result = subprocess.run([str(core_header_tool), r"levels\test\test", "allocation"], capture_output=True, text=True)
    assert result.returncode == 1, result.stdout + result.stderr


@pytest.fixture(scope="session")
def checkpoint_tool(tmp_path_factory):
    compiler = shutil.which("clang") or shutil.which("cc")
    if not compiler:
        pytest.skip("a C compiler is needed for MCC checkpoint tests")
    work = tmp_path_factory.mktemp("mcc-checkpoint")
    (work / "saved games").mkdir()
    (work / "memory").mkdir()
    (work / "objects").mkdir()
    (work / "cseries.h").write_text("#pragma once\ntypedef unsigned char boolean,byte;\ntypedef float real;\n")
    (work / "errors.h").write_text("#define _error_silent 0\nvoid error(int level,char const *format,...);\n")
    (work / "saved games/game_state.h").write_text(
        "#pragma once\n#include <stdint.h>\nstruct game_state_header {uint32_t allocation_checksum;"
        "char map_name[256],build_number[32];short player_count,difficulty;"
        "uint32_t cache_file_checksum,unused[7],checksum;};\n")
    data_header = (ROOT / "source/memory/data.h").read_text()
    (work / "memory/data.h").write_text("#pragma once\n#define TAG_STRING_LENGTH 31\n" + "\n".join(
        re.search(r"struct " + name + r"\s*\{.*?\n\};", data_header, re.S).group()
        for name in ("data_array", "datum_header")) + "\n")
    (work / "objects/objects.h").write_text(
        "#define _object_mask_unit 3\nstruct object_datum {unsigned char state[64];};\n"
        "struct object_header_datum {short identifier;unsigned char flags,type;"
        "short cluster_index,data_size;struct object_datum *datum;};\n"
        "extern struct data_array *object_header_data;\n")
    native = (ROOT / "source/saved games/game_state.c").read_text()
    funcs = "\n".join(function(native, name) for name in
                      ("game_state_save", "game_state_save_core", "game_state_image_accept"))
    data_source = (ROOT / "source/memory/data.c").read_text()
    objects = (ROOT / "source/objects/objects.c").read_text()
    grenades = (ROOT / "port/linux/game/mcc_grenades.c").read_text()
    real_units = "\n".join((function(native, "game_state_image_pointer"),
        function(native, "game_state_image_data_valid").replace("game_state_image_data_valid", "checkpoint_native_data_valid"),
        function(data_source, "datum_try_and_get"),
        "static void *datum_get(struct data_array *data,long index) {void *p=datum_try_and_get(data,index);if(!p)abort();return p;}",
        function(data_source, "datum_delete"),
        function(objects, "object_try_and_get_and_verify_type")))
    grenade_units = "\n".join(function(grenades, name) for name in
        ("mcc_grenades_reset", "mcc_grenade_u32", "mcc_grenades_snapshot", "mcc_grenades_snapshot_validate", "mcc_grenades_restore"))
    for name in ("mcc_grenades_snapshot_validate", "mcc_grenades_snapshot", "mcc_grenades_restore", "mcc_grenades_reset"):
        grenade_units = grenade_units.replace(name, name.replace("mcc_", "real_", 1))
    source = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0
#define GAME_STATE_SIZE 524288
#define GAME_STATE_CPU_SIZE 393216
#define csmemcpy memcpy
#define NONE (-1)
#define NUMBEROF(a) (sizeof(a)/sizeof((a)[0]))
#define TEST_FLAG(flags,bit) ((flags)&(1u<<(bit)))
#define match_assert(file,line,test) do {if(!(test))abort();} while(0)
#define MAXIMUM_OBJECTS_PER_MAP 8192
static int real_units_mode,real_restore_calls;
struct mcc_grenade_inventory {long handle;unsigned char counts[2];};
static struct mcc_grenade_inventory mcc_grenade_inventories[MAXIMUM_OBJECTS_PER_MAP];
static long mcc_grenade_flash_time[4];
static uint32_t real_grenades_snapshot(void *out,uint32_t capacity);
static int real_grenades_snapshot_validate(void const *in,uint32_t bytes);
static int real_grenades_restore(void const *in,uint32_t bytes);
static int loaded=1, malformed=0, reject_handles=0, captures=0, restores=0, writes=0;
static uint32_t payload_size=16;
static unsigned char live[GAME_STATE_SIZE], saved[GAME_STATE_SIZE];
static struct {void *base_address;long cpu_allocation_size;int saved_game_valid;} game_state_globals;
enum {_game_state_allocation_data,_game_state_allocation_memory_pool,_game_state_allocation_lruv_cache};
struct game_state_allocation {int kind;void *address;long maximum_count,element_size,size;};
static struct game_state_allocation game_state_allocations[1];
static long game_state_allocation_count;
static int native_valid=1,checkpoint_notifications;
static int console_valid=1,console_captured=1,console_restores;
static int hs_console_state_capture(void) {return console_captured;}
static int hs_console_state_validate(void const *image,void const *base,unsigned long bytes)
{(void)image;(void)base;(void)bytes;return console_valid;}
static void hs_console_state_restore(void) {console_restores++;if(live[4095]!=saved[4095])abort();}
''' + '#include "' + (ROOT / "port/linux/game/mcc_checkpoint.c").as_posix() + '"\n' + r'''
boolean mcc_cache_tags_loaded(void) {return loaded;}
int mcc_script_threads_capture(void) {return 1;}
int mcc_script_threads_validate(void const *a,void const *b,unsigned long c,unsigned long d)
{(void)a;(void)b;(void)c;(void)d;return 1;}
int mcc_script_threads_restore(void *a,unsigned long b,unsigned long c)
{(void)a;(void)b;(void)c;return 1;}
static int campaign_restores;
static unsigned long name_bytes;
static unsigned char name_state[MCC_OBJECT_NAMES_SNAPSHOT_MAXIMUM];
int mcc_object_names_active(void) {return name_bytes!=0;}
unsigned long mcc_object_names_snapshot(void *out) {if(out)memcpy(out,name_state,name_bytes);return name_bytes;}
int mcc_object_names_validate(void const *in,unsigned long bytes) {return bytes==name_bytes && (!bytes||in);}
int mcc_object_names_restore(void const *in,unsigned long bytes) {if(!mcc_object_names_validate(in,bytes))return 0;memcpy(name_state,in,bytes);return 1;}

void mcc_campaign_snapshot(void *out) {
    memset(out,0,MCC_CAMPAIGN_SNAPSHOT_BYTES);((unsigned char *)out)[0]=1;
    ((unsigned char *)out)[MCC_CAMPAIGN_SNAPSHOT_V5_BYTES]=42;
}
int mcc_campaign_validate(void const *in,unsigned long bytes) {
    return (bytes==MCC_CAMPAIGN_SNAPSHOT_BYTES || bytes==MCC_CAMPAIGN_SNAPSHOT_V5_BYTES || bytes==MCC_CAMPAIGN_SNAPSHOT_V4_BYTES || bytes==MCC_CAMPAIGN_SNAPSHOT_V1_BYTES) && ((unsigned char const *)in)[0]==1 &&
        (bytes!=MCC_CAMPAIGN_SNAPSHOT_BYTES || ((unsigned char const *)in)[MCC_CAMPAIGN_SNAPSHOT_V5_BYTES]==42);
}
int mcc_campaign_restore(void const *in,unsigned long bytes) {
    if(!mcc_campaign_validate(in,bytes))return 0;campaign_restores++;return 1;
}
void mcc_campaign_begin(void) {campaign_restores++;}
struct data_array *object_header_data;
short mcc_grenades_type_count(void) {return 4;}
boolean mcc_level_name(char const *name) {return !strncmp(name,"mcc_maps\\",9);}
void error(int level,char const *format,...) {(void)level;(void)format;}
uint32_t mcc_grenades_snapshot(void *out,uint32_t capacity) {
    unsigned char *p=out;
    if(real_units_mode)return real_grenades_snapshot(out,capacity);
    if(!p)return payload_size;
    if(capacity<payload_size)return 0;
    memset(p,0,payload_size);memcpy(p,"GMC4",4);p[4]=1;
    captures++;return malformed?0:payload_size;
}
int mcc_grenades_snapshot_validate(void const *in,uint32_t bytes) {
    if(real_units_mode)return real_grenades_snapshot_validate(in,bytes);
    return bytes>=8 && !memcmp(in,"GMC4\1\0",6);
}
int mcc_grenades_restore(void const *in,uint32_t bytes) {
    if(real_units_mode){real_restore_calls++;return real_grenades_restore(in,bytes);}
    if(!mcc_grenades_snapshot_validate(in,bytes)||reject_handles)return 0;
    restores++;return 1;
}
void game_state_call_before_save_procs(void) {}
void main_stop_time(void) {}
void main_start_time(void) {}
void network_coop_checkpoint_saved(void) {checkpoint_notifications++;}
void cinematic_port_checkpoint(void) {}
void game_state_note_event(char const *event) {(void)event;}
int game_state_write_to_file(void) {memcpy(saved,live,sizeof(saved));writes++;return 1;}
int game_state_write_core(char const *name,void const *data,long size) {
    (void)name;memcpy(saved,data,size);writes++;return 1;
}
void console_printf(int flag,char const *format,...) {(void)flag;(void)format;}
int game_state_image_refuse(char const *name,char const *reason) {(void)name;(void)reason;return 0;}
static boolean checkpoint_native_data_valid(byte *image,struct data_array *data,short count,short size);
int game_state_image_data_valid(void *image,void *address,long count,long size) {
    return real_units_mode ? checkpoint_native_data_valid(image,address,(short)count,(short)size) : native_valid;
}
int game_state_image_memory_pool_valid(void *image,void *address,long size) {
    (void)image;(void)address;(void)size;return native_valid;
}
int game_state_image_lruv_cache_valid(void *image,void *address,struct game_state_allocation const *a) {
    (void)image;(void)address;(void)a;return native_valid;
}
/* These are the native public macro routes, with the real implementations below. */
#define object_header_try_and_get(index) ((struct object_header_datum *)datum_try_and_get(object_header_data,index))
#define unit_try_and_get(index) object_try_and_get_and_verify_type(index,_object_mask_unit)
''' + real_units + "\n" + grenade_units + "\n" + funcs + r'''
static int check_unit_preflight(char const *test,unsigned long used)
{
    struct object_header_datum *units=(void *)(object_header_data+1);
    unsigned char original_objects[sizeof(struct data_array)+2*sizeof(*units)];
    unsigned char original_names[640*4],*before;
    unsigned char *payload,*footer;
    int valid=!strcmp(test,"real_unit_valid"),preflight,accepted,native_ok,image_changed,names_changed;
    real_units_mode=1;
    memset(object_header_data,0,sizeof(original_objects));
    strcpy(object_header_data->name,"object");object_header_data->signature='d@t@';
    object_header_data->maximum_count=object_header_data->count=object_header_data->actual_count=2;
    object_header_data->size=sizeof(*units);object_header_data->valid=1;
    object_header_data->first_free_absolute_index=2;object_header_data->next_identifier=3;
    object_header_data->data=units;
    units[0].identifier=1;units[0].datum=(void *)(live+768);
    units[1].identifier=2;units[1].datum=(void *)(live+896);
    mcc_grenade_inventories[0].handle=0x10000;mcc_grenade_inventories[0].counts[0]=2;
    name_bytes=sizeof(original_names);memset(name_state,255,name_bytes);
    mcc_checkpoint_put(name_state+513*4,0x20001);
    game_state_allocation_count=1;game_state_allocations[0].kind=_game_state_allocation_data;
    game_state_allocations[0].address=object_header_data;game_state_allocations[0].maximum_count=2;
    game_state_allocations[0].element_size=sizeof(*units);
    game_state_save();
    if(writes!=1)return 31;
    payload=saved+GAME_STATE_CPU_SIZE-MCC_CHECKPOINT_SLOT_BYTES;
    footer=saved+GAME_STATE_CPU_SIZE-MCC_CHECKPOINT_FOOTER_BYTES;
    if(!strcmp(test,"real_unit_unused")) {
        memcpy(original_objects,object_header_data,sizeof(original_objects));
        datum_delete(object_header_data,0x10000);
        if(units[0].identifier || object_header_data->count!=2 || object_header_data->actual_count!=1 ||
            units[0].datum!=(void *)(live+768) || unit_try_and_get(0))return 32;
        memcpy(saved+512,object_header_data,sizeof(original_objects));
        memcpy(object_header_data,original_objects,sizeof(original_objects));
        mcc_checkpoint_put(payload+8,0);
    } else if(!strcmp(test,"real_unit_stale"))mcc_checkpoint_put(payload+8,0x30000);
    else if(!strcmp(test,"real_unit_pointer")) {
        struct object_header_datum *incoming=(void *)(saved+512+sizeof(struct data_array));
        incoming[0].datum=(void *)(live+used);
    } else if(!valid)return 33;
    mcc_checkpoint_put(footer+12,mcc_checkpoint_checksum(payload,mcc_checkpoint_word(footer+8)));
    mcc_checkpoint_put(footer+92,mcc_checkpoint_checksum(footer,92));
    /* A CRC-correct image with legal generic-array counts reaches MCC preflight. */
    native_ok=checkpoint_native_data_valid(saved,object_header_data,2,sizeof(*units));
    if(!native_ok || !real_grenades_snapshot_validate(payload,16))return 34;
    live[4095]=33;memset(name_state,255,name_bytes);
    memcpy(original_names,name_state,name_bytes);
    before=malloc(sizeof(live));if(!before)return 35;memcpy(before,live,sizeof(live));
    preflight=mcc_checkpoint_validate(saved,sizeof(saved),used,GAME_STATE_CPU_SIZE,live);
    accepted=game_state_image_accept(saved,sizeof(saved));
    image_changed=memcmp(before,live,sizeof(live))!=0;
    names_changed=memcmp(original_names,name_state,name_bytes)!=0;
    printf("native=%d preflight=%d accepted=%d image_changed=%d names_changed=%d grenade_restore=%d campaign_restore=%d\n",
        native_ok,preflight,accepted,image_changed,names_changed,real_restore_calls,campaign_restores);
    free(before);
    if(valid) {
        if(!preflight || !accepted || !image_changed || !names_changed || real_restore_calls!=1 ||
            campaign_restores!=1 || live[4095]!=0xA5 || mcc_grenade_inventories[0].handle!=0x10000 ||
            mcc_grenade_inventories[0].counts[0]!=2 || mcc_checkpoint_word(name_state+513*4)!=0x20001)return 36;
    } else if(preflight || accepted || image_changed || names_changed || real_restore_calls || campaign_restores)return 37;
    mcc_checkpoint_dispose();return 0;
}

int main(int argc,char **argv) {
    unsigned char *tail,*footer;
    unsigned long used=4096;
    int result=0;
    struct game_state_header *header=(void *)live,*incoming=(void *)saved;
    if(argc<2||argc>3)return 2;
    if(!strcmp(argv[1],"crc")) {
        unsigned long i,bytes;
        unsigned char *data;
        if(argc!=3)return 2;
        if(!strcmp(argv[2],"null")) {printf("%08x\n",mcc_checkpoint_checksum(NULL,0));return 0;}
        if(!strcmp(argv[2],"known")) {printf("%08x\n",mcc_checkpoint_checksum((unsigned char const *)"123456789",9));return 0;}
        bytes=strtoul(argv[2],NULL,10);if(bytes>MCC_OBJECT_NAMES_SNAPSHOT_MAXIMUM)return 2;
        data=malloc(bytes?bytes:1);if(!data)return 2;
        for(i=0;i<bytes;i++)data[i]=(unsigned char)(i*37+(i>>8)+19);
        printf("%08x\n",mcc_checkpoint_checksum(data,(uint32_t)bytes));free(data);return 0;
    }
    memset(live,0xA5,sizeof(live));
    memset(header,0,sizeof(*header));strcpy(header->map_name,"mcc_maps\\one\\a10");
    header->cache_file_checksum=0x12345678;
    object_header_data=(void *)(live+512);
    memset(object_header_data,0,sizeof(*object_header_data));
    object_header_data->maximum_count=object_header_data->count=1;
    object_header_data->valid=1;object_header_data->size=sizeof(struct object_header_datum);
    object_header_data->data=live+640;
    {struct object_header_datum *unit=object_header_data->data;
     memset(unit,0,sizeof(*unit));unit->identifier=1;unit->datum=(void *)(live+768);}
    game_state_globals.base_address=live;game_state_globals.cpu_allocation_size=used;
    if(!strncmp(argv[1],"console_",8)) {
        if(!strcmp(argv[1],"console_capture") || !strcmp(argv[1],"console_core_capture")) {
            console_captured=0;
            if(!strcmp(argv[1],"console_core_capture"))game_state_save_core("test");else game_state_save();
            return writes || captures || checkpoint_notifications || game_state_globals.saved_game_valid ? 40 : 0;
        }
        game_state_save();live[4095]=33;
        if(!strcmp(argv[1],"console_validate")) {
            console_valid=0;
            if(game_state_image_accept(saved,sizeof(saved)) || live[4095]!=33 || console_restores || restores)return 41;
        } else if(!strcmp(argv[1],"console_restore_order")) {
            if(!game_state_image_accept(saved,sizeof(saved)) || console_restores!=1 || restores!=1)return 42;
        } else return 43;
        mcc_checkpoint_dispose();return 0;
    }
    if(!strncmp(argv[1],"real_unit_",10))return check_unit_preflight(argv[1],used);
    if(!strncmp(argv[1],"snapshot",8)) {
        FILE *output;
        if(argc!=3)return 2;
        if(!strcmp(argv[1],"snapshot_maximum"))payload_size=MCC_CHECKPOINT_PAYLOAD_LIMIT;
        if(!strcmp(argv[1],"snapshot_names")||!strcmp(argv[1],"snapshot_names_maximum")) {
            name_bytes=!strcmp(argv[1],"snapshot_names_maximum")?MCC_OBJECT_NAMES_SNAPSHOT_MAXIMUM:640*4;
            memset(name_state,255,name_bytes);mcc_checkpoint_put(name_state+513*4,0x00010000);
        }
        if(!mcc_checkpoint_capture(live,sizeof(live),used,GAME_STATE_CPU_SIZE))return 25;
        output=fopen(argv[2],"wb");if(!output)return 26;
        if(fwrite(live,1,sizeof(live),output)!=sizeof(live)||fclose(output))return 27;
        mcc_checkpoint_dispose();return 0;
    }
    if(!strncmp(argv[1],"names_",6)) {
        unsigned char *name_tail;
        name_bytes=!strcmp(argv[1],"names_maximum")?MCC_OBJECT_NAMES_SNAPSHOT_MAXIMUM:640*4;
        memset(name_state,255,name_bytes);
        mcc_checkpoint_put(name_state+513*4,0x00010000);
        if(!mcc_checkpoint_capture(live,sizeof(live),used,GAME_STATE_CPU_SIZE))return 21;
        memcpy(saved,live,sizeof(live));
        name_tail=mcc_checkpoint_names_tail(saved,used,GAME_STATE_CPU_SIZE,name_bytes);
        if(!strcmp(argv[1],"names_roundtrip")||!strcmp(argv[1],"names_maximum")) {
            memset(name_state,0,name_bytes);
            if(!game_state_image_accept(saved,sizeof(saved)))return 22;
            if(mcc_checkpoint_word(name_state+513*4)!=0x00010000 || mcc_checkpoint_word(name_state)!=0xFFFFFFFFu)return 23;
            mcc_checkpoint_dispose();
            return 0;
        }
        if(!strcmp(argv[1],"names_crc"))name_tail[16]^=1;
        if(!strcmp(argv[1],"names_missing"))name_tail[0]=0;
        if(!strcmp(argv[1],"names_salt")) {
            mcc_checkpoint_put(name_tail+16+513*4,0x00020000);
            mcc_checkpoint_put(name_tail+12,mcc_checkpoint_checksum(name_tail+16,name_bytes));
        }
        if(game_state_image_accept(saved,sizeof(saved)))return 24;
        mcc_checkpoint_dispose();return 0;
    }
    tail=live+GAME_STATE_CPU_SIZE-MCC_CHECKPOINT_SLOT_BYTES;
    footer=tail+MCC_CHECKPOINT_TOTAL_PAYLOAD;
    if(!strcmp(argv[1],"stock")) {
        loaded=0;strcpy(header->map_name,"levels\\a10\\a10");
        game_state_save();result=game_state_image_accept(saved,sizeof(saved));
        if(checkpoint_notifications!=1||captures||restores||memcmp(tail,saved+(tail-live),MCC_CHECKPOINT_SLOT_BYTES))return 3;
    } else if(!strcmp(argv[1],"capacity")) {
        game_state_globals.cpu_allocation_size=GAME_STATE_CPU_SIZE-MCC_CHECKPOINT_SLOT_BYTES+1;
        game_state_save();result=!writes&&!checkpoint_notifications&&!game_state_globals.saved_game_valid;
    } else if(!strcmp(argv[1],"capture_fail")) {
        malformed=1;game_state_save();result=!writes&&!checkpoint_notifications&&!game_state_globals.saved_game_valid;
    } else if(!strcmp(argv[1],"maximum")) {
        payload_size=MCC_CHECKPOINT_PAYLOAD_LIMIT;
        result=mcc_checkpoint_capture(live,sizeof(live),used,GAME_STATE_CPU_SIZE);
        result &= mcc_checkpoint_restore(live,sizeof(live),used,GAME_STATE_CPU_SIZE);
    } else if(!strcmp(argv[1],"oversize")) {
        payload_size=MCC_CHECKPOINT_PAYLOAD_LIMIT+1;
        result=!mcc_checkpoint_capture(live,sizeof(live),used,GAME_STATE_CPU_SIZE);
    } else {
        if(!strcmp(argv[1],"core"))game_state_save_core("test");else game_state_save();
        if(writes!=1||captures!=1||checkpoint_notifications!=(strcmp(argv[1],"core")!=0))return 4;
        if(!strcmp(argv[1],"v1")) {
            unsigned char *f=saved+(footer-live),*p=saved+(tail-live);
            memmove(p+MCC_CHECKPOINT_CAMPAIGN_BYTES,p,payload_size);
            mcc_checkpoint_put(f+4,1);mcc_checkpoint_put(f+8,payload_size);
            mcc_checkpoint_put(f+12,mcc_checkpoint_checksum(p+MCC_CHECKPOINT_CAMPAIGN_BYTES,payload_size));
            mcc_checkpoint_put(f+20,0);memset(f+88,0,8);
            result=game_state_image_accept(saved,sizeof(saved));
            if(restores!=1 || campaign_restores!=1)return 12;
        } else if(!strcmp(argv[1],"campaign_v1") || !strcmp(argv[1],"campaign_v4") || !strcmp(argv[1],"campaign_v5")) {
            unsigned char *f=saved+(footer-live),*p=saved+(tail-live);
            unsigned long old_size=!strcmp(argv[1],"campaign_v5") ? MCC_CAMPAIGN_SNAPSHOT_V5_BYTES :
                !strcmp(argv[1],"campaign_v4") ? MCC_CAMPAIGN_SNAPSHOT_V4_BYTES : MCC_CAMPAIGN_SNAPSHOT_V1_BYTES;
            unsigned long shift=MCC_CHECKPOINT_CAMPAIGN_BYTES-old_size;
            unsigned long bytes=payload_size+old_size;
            memmove(p+shift,p,bytes);
            mcc_checkpoint_put(f+8,bytes);
            mcc_checkpoint_put(f+12,mcc_checkpoint_checksum(p+shift,bytes));
            mcc_checkpoint_put(f+20,old_size);
            mcc_checkpoint_put(f+4,2);memset(f+88,0,8);
            result=game_state_image_accept(saved,sizeof(saved));
            if(restores!=1 || campaign_restores!=1)return 13;
        } else if(!strcmp(argv[1],"roundtrip")||!strcmp(argv[1],"core")) {
            live[4095]=33;result=game_state_image_accept(saved,sizeof(saved));
            if(restores!=1||campaign_restores!=1||live[4095]!=0xA5)return 5;
        } else if(!strcmp(argv[1],"cleanup")) {
            mcc_checkpoint_dispose();
            for(unsigned long i=0;i<MCC_CHECKPOINT_SLOT_BYTES;i++)if(tail[i]!=0xA5)return 6;
            result=1;
        } else {
            unsigned char *f=saved+(footer-live),*p=saved+(tail-live);
            if(!strcmp(argv[1],"payload"))p[7]^=1;
            else if(!strcmp(argv[1],"campaign")) {
                p[payload_size]=2;
                mcc_checkpoint_put(f+12,mcc_checkpoint_checksum(p,payload_size+MCC_CHECKPOINT_CAMPAIGN_BYTES));
                mcc_checkpoint_put(f+92,mcc_checkpoint_checksum(f,92));
            }
            else if(!strcmp(argv[1],"footer"))f[0]^=1;
            else if(!strcmp(argv[1],"version"))f[4]=4;
            else if(!strcmp(argv[1],"reserved"))f[88]^=1;
            else if(!strcmp(argv[1],"ambient_invalid")) {
                f[88]^=1;mcc_checkpoint_put(f+92,mcc_checkpoint_checksum(f,92));
            }
            else if(!strcmp(argv[1],"footer_crc"))f[92]^=1;
            else if(!strcmp(argv[1],"name"))strcpy(incoming->map_name,"mcc_maps\\two\\a10");
            else if(!strcmp(argv[1],"checksum"))incoming->cache_file_checksum++;
            else if(!strcmp(argv[1],"highwater"))game_state_globals.cpu_allocation_size=GAME_STATE_CPU_SIZE-1;
            else if(!strcmp(argv[1],"native")){game_state_allocation_count=1;native_valid=0;}
            else if(!strcmp(argv[1],"handles"))reject_handles=1;
            else if(!strncmp(argv[1],"unit_",5)) {
                struct data_array *a=(void *)(saved+512);
                struct object_header_datum *unit=(void *)(saved+640);
                p[6]=1;p[8]=0;p[9]=0;p[10]=1;p[11]=0;p[12]=2;
                mcc_checkpoint_put(f+12,mcc_checkpoint_checksum(p,16+MCC_CHECKPOINT_CAMPAIGN_BYTES));
                mcc_checkpoint_put(f+92,mcc_checkpoint_checksum(f,92));
                if(!strcmp(argv[1],"unit_salt"))unit->identifier=2;
                else if(!strcmp(argv[1],"unit_type"))unit->type=2;
                else if(!strcmp(argv[1],"unit_pointer"))unit->datum=(void *)(live+sizeof(live));
                else if(!strcmp(argv[1],"unit_array"))a->data=(void *)(live+sizeof(live));
                else return 11;
            }
            else return 7;
            result=!game_state_image_accept(saved,sizeof(saved));
            if(restores)return 8;
        }
    }
    mcc_checkpoint_dispose();
    if(!result)return 9;
    puts("ok");return 0;
}
'''
    path = work / "checkpoint.c"
    path.write_text(source)
    output = work / ("checkpoint.exe" if sys.platform == "win32" else "checkpoint")
    zlib_dir = ROOT / "port/third_party/zlib"
    command = [compiler, "-std=c99", "-Wall", "-Wextra", "-Werror", "-DZ_PREFIX",
               "-Dz_errmsg=z_port_errmsg", "-I", str(work), "-I", str(zlib_dir),
               str(path), str(zlib_dir / "crc32.c"), "-o", str(output)]
    if sys.platform == "win32":
        command[1:1] = ["--target=i686-pc-windows-msvc", "-fuse-ld=lld", "-D_CRT_SECURE_NO_WARNINGS"]
    result = subprocess.run(command, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return output


@pytest.mark.parametrize("case", ["stock", "capacity", "capture_fail", "maximum", "oversize",
                                      "console_capture", "console_core_capture", "console_validate", "console_restore_order",
                                      "roundtrip", "core", "cleanup", "payload", "footer", "version",
                                      "reserved", "ambient_invalid", "footer_crc", "name", "checksum", "highwater", "native", "handles",
                                      "unit_salt", "unit_type", "unit_pointer", "unit_array", "v1", "campaign", "campaign_v1", "campaign_v4", "campaign_v5", "names_roundtrip", "names_maximum", "names_crc", "names_missing", "names_salt"])
def test_mcc_checkpoint_and_native_hooks(checkpoint_tool, case):
    result = subprocess.run([str(checkpoint_tool), case], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr + f" ({case}: {result.returncode})"


@pytest.mark.parametrize("case", ["real_unit_unused", "real_unit_valid", "real_unit_stale", "real_unit_pointer"])
def test_checkpoint_unit_preflight_prevents_partial_restore(checkpoint_tool, case):
    result = subprocess.run([str(checkpoint_tool), case], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr + f" ({case}: {result.returncode})"


@pytest.mark.parametrize("size", ["null", "known", 0, 1, 7, 8, 9, 15, 16, 17, 255, 256, 257,
                                  32767, 32768, 32769, 65544, 66312, 131068])
def test_checkpoint_checksum_matches_independent_oracle(checkpoint_tool, size):
    data = b"" if size == "null" else b"123456789" if size == "known" else bytes(
        (i * 37 + (i >> 8) + 19) & 255 for i in range(size))
    result = subprocess.run([str(checkpoint_tool), "crc", str(size)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    assert int(result.stdout, 16) == zlib.crc32(data)


@pytest.mark.parametrize("case,payload_bytes,name_bytes", [
    ("snapshot", 16, 0), ("snapshot_maximum", 65544, 0),
    ("snapshot_names", 16, 2560), ("snapshot_names_maximum", 16, 131068),
])
def test_checkpoint_persisted_bytes(checkpoint_tool, tmp_path, case, payload_bytes, name_bytes):
    output = tmp_path / "image.bin"
    result = subprocess.run([str(checkpoint_tool), case, str(output)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    image = output.read_bytes()
    assert len(image) == 524288
    # The persisted format keeps its CPU tail and 96-byte footer at fixed offsets.
    tail, footer = 393216 - (65544 + 768 + 96), 393216 - 96
    payload = bytearray(payload_bytes + 768)
    payload[:6] = b"GMC4\1\0"
    payload[payload_bytes] = 1
    expected = bytearray(96)
    struct.pack_into("<4s5I", expected, 0, b"MCCS", 3, len(payload), zlib.crc32(payload), 0x12345678, 768)
    name = b"mcc_maps\\one\\a10\0"
    expected[24:24 + len(name)] = name
    expected[88] = 42
    struct.pack_into("<I", expected, 92, zlib.crc32(expected[:92]))
    assert image[tail:tail + len(payload)] == payload
    assert image[tail + len(payload):footer] == b"\xa5" * (footer - tail - len(payload))
    assert image[footer:footer + 96] == expected
    if name_bytes:
        names = bytearray(b"\xff" * name_bytes)
        struct.pack_into("<I", names, 513 * 4, 0x00010000)
        expected_names = struct.pack("<4s3I", b"MCCN", 1, name_bytes, zlib.crc32(names)) + names
        assert image[tail - len(expected_names):tail] == expected_names
    assert image[393216:] == b"\xa5" * (524288 - 393216)

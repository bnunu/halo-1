#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc_package.h"
#include "mcc_validation.h"
#include "mcc_resources.h"
#include "mcc_runtime.h"
#include "mcc_sha256.h"
int denied;
static int fingerprints;
static void counted_sha256_end(struct mcc_sha256 *hash,unsigned char digest[32]) {
    fingerprints++;mcc_sha256_end(hash,digest);
}
#define mcc_sha256_end counted_sha256_end
static struct mcc_precache_snapshot stamps;
static int unused,both,reads,stamp_reads,change_at;
static size_t counted_read(void *p,size_t n,size_t count,FILE *f) {reads++;return fread(p,n,count,f);}
#define fread counted_read
unsigned long system_milliseconds(void) {return 0;}
void mcc_loading_pulse(void) {}
static int read_map(void *p,uint32_t at,uint32_t bytes,void *out) {
    return !fseek(p,(long)at,SEEK_SET) && fread(out,1,bytes,p)==bytes;
}
int mcc_precache_snapshot(char const *name,struct mcc_precache_snapshot *out) {
    (void)name;if(++stamp_reads==change_at)stamps.files[0].written_low++;
    *out=stamps;return 1;
}
enum mcc_cache_status mcc_cache_identify(void const *p,size_t n,uint64_t size,struct mcc_cache_identity *out) {
    (void)p;(void)n;memset(out,0,sizeof(*out));out->file_length=(uint32_t)size;out->tag_data_size=16;return MCC_CACHE_OK;
}
enum mcc_cache_status mcc_cache_load(struct mcc_cache_source const *s,void *p,size_t n,struct mcc_cache_report *out) {
    (void)s;(void)p;(void)n;memset(out,0,sizeof(*out));out->external_sound_count=!unused;out->external_bitmap_count=both;return MCC_CACHE_OK;
}
int mcc_cache_visit_resources(void *raw,struct mcc_cache_report const *r,
    int (*visit)(void *,struct mcc_resource_reference const *),void *p) {
    struct mcc_resource_reference ref;
    (void)raw;(void)r;memset(&ref,0,sizeof(ref));ref.type=2;ref.tag_name="sound\\test";ref.offset=16;ref.bytes=4;
    if(!visit(p,&ref))return 0;
    ref.type=1;return !both || visit(p,&ref);
}
char const *mcc_status_explanation(enum mcc_cache_status s) {(void)s;return "fixture read failure";}
char const *mcc_cache_status_string(enum mcc_cache_status s) {(void)s;return "fixture read failure";}
/* RESOURCES */
static FILE *mcc_open_explain(char const *name,struct mcc_cache_source *source,
    struct mcc_cache_identity *identity,char *reason,size_t size) {
    char path[128];FILE *f;unsigned i;
    (void)identity;(void)reason;(void)size;
    assert(mcc_package_map_path(name,path));
    for(i=3;path[i];i++)if(path[i]=='\\')path[i]='/';
    f=fopen(path+3,"rb");
    source->context=f;source->read=read_map;source->size=2048;return f;
}
#include "mcc_validation.inl"
/* PLANNER */
#if defined(HALO_ANDROID)
/* (the host's calls, in the Android build of this driver: no hash job is
the host's, and a cancel of job 0 is recorded) */
static int host_cancelled_zero;
const char *platform_data_root(void) {return ".";}
void platform_translate_path(const char *xbox_path,char *host_path,unsigned long size) {snprintf(host_path,size,"%s",xbox_path);}
int host_mcc_hash_start(unsigned int request) {(void)request;return -1;}
int host_mcc_hash_wait(int id,int milliseconds,unsigned int result) {(void)id;(void)milliseconds;(void)result;return -1;}
void host_mcc_hash_cancel(int id) {if(!id)host_cancelled_zero=1;}
static int failing_read(void *p,uint32_t at,uint32_t bytes,void *out) {(void)p;(void)at;(void)bytes;(void)out;return 0;}
#endif
static void write_bytes(char const *path,char const *bytes) {
    FILE *f=fopen(path,"wb");assert(f);assert(fwrite(bytes,1,strlen(bytes),f)==strlen(bytes));assert(!fclose(f));
}
int main(int argc,char **argv) {
    char const *name="mcc_maps/p/a";char paths[2][128],reason[320];
    unsigned char digest[32],actual[32];unsigned mask;uint32_t checksum;
    struct mcc_cache_source source;struct mcc_cache_identity identity;
    struct mcc_resource_check *check;struct mcc_validation_check *job;struct mcc_validation_proof proof;
    struct mcc_runtime runtime;FILE *f;int status,i,before,mismatch,owned,all;
    struct mcc_resource_candidates candidates;
    assert(argc==2);
#if defined(HALO_ANDROID)
    /* a check that fails before it has a host job of its own cancels none */
    if(!strcmp(argv[1],"early_failure")) {
        memset(&source,0,sizeof(source));source.read=failing_read;source.size=2048;
        check=mcc_resources_check_begin_candidate(name,&source,NULL,reason,sizeof(reason));
        printf("%s %d\n",check?"check":"none",host_cancelled_zero);
        return 0;
    }
#endif
    if(!strcmp(argv[1],"cache")) {
        memset(paths,0,sizeof(paths));strcpy(paths[1],"mcc_maps/p/sounds.map");
        write_bytes(paths[1],"base");write_bytes("mcc_maps/sounds.map","base");
        assert(base_resources(name,paths,&mask) && mask==2);before=reads;
        assert(base_resources(name,paths,&mask) && mask==2 && before==reads);
        for(i=0;i<MCC_PRECACHE_FILES;i++) {
            stamps.files[i].written_low++;before=reads;
            assert(base_resources(name,paths,&mask) && mask==2 && reads>before);
        }
        write_bytes(paths[1],"mod!");stamps.files[1].written_low++;
        assert(base_resources(name,paths,&mask) && !mask);
        before=reads;assert(base_resources(name,paths,&mask) && !mask && reads==before);
        stamps.files[0].written_low++;change_at=stamp_reads+2;
        assert(!base_resources(name,paths,&mask));change_at=0;
        assert(base_resources(name,paths,&mask));
        remove(paths[1]);stamps.files[1].written_low++;
        assert(!base_resources(name,paths,&mask));
        assert(!mcc_package_base_resource_path(0,0,paths[0]));
        assert(!mcc_package_base_resource_path(2,MCC_PACKAGE_BASE_SETS,paths[0]));
        return 0;
    }
    owned=!strcmp(argv[1],"owned_mismatch");all=!strcmp(argv[1],"all_mismatch");
    mismatch=strstr(argv[1],"mismatch")!=NULL;
    both=!strcmp(argv[1],"both") || owned || all;
    unused=!strncmp(argv[1],"unused",6);
    memset(&candidates,0,sizeof(candidates));
    candidates.index=unused || owned || !strcmp(argv[1],"denied") ? 0 : all ? 8 : both ? 3 : 1;
    f=mcc_open_explain(name,&source,&identity,reason,sizeof(reason));assert(f);
    check=mcc_resources_check_begin_candidate(name,&source,&candidates,reason,sizeof(reason));assert(check);
    do {status=mcc_resources_check_step(check,&checksum,digest);} while(!status);
    assert(status==1);mcc_resources_check_end(check);fclose(f);
    if(!strcmp(argv[1],"override") || unused) write_bytes("mcc_maps/p/sounds.map","invalid override");
    denied=!strcmp(argv[1],"denied");
    /* A one-byte difference anywhere in the 32-byte SHA must reject admission. */
    for(i=0;i<(!strcmp(argv[1],"mismatch")?32:1);i++) {
        if(mismatch)digest[i]^=1;
        job=mcc_validation_check_begin(name,!strcmp(argv[1],"stage") ? "mcc_staging/p" : NULL);assert(job);mcc_validation_check_expect(job,digest);
        if(!strcmp(argv[1],"changed"))change_at=stamp_reads+2;
        before=0;fingerprints=0;
        do {status=mcc_validation_check_step(job,&proof,reason,sizeof(reason));assert(++before<40);} while(!status);
        mcc_validation_check_end(job);
        if(mismatch || denied || !strcmp(argv[1],"override") || !strcmp(argv[1],"changed")) {
            assert(status==-1);
            if(mismatch) {
                assert(fingerprints==(unused || owned ? 1 : all ? 9 : 2));
                assert(strstr(reason,"SHA-256"));
            }
            if(denied || !strcmp(argv[1],"override")) {
                assert(fingerprints==0 && strstr(reason,"sounds.map") && !strstr(reason,"SHA-256"));
                if(denied) assert(strstr(reason,"Cannot read"));
            }
        }
        else {
            assert(status==1 && !memcmp(proof.digest,digest,32));
            assert(unused || !strcmp(proof.resources[1],"mcc_maps/_shared/multiplayer/sounds.map"));
            assert(mcc_validation_publish(name,&proof));
            assert(mcc_validation_resources(name,paths));
            memset(&runtime,0,sizeof(runtime));strcpy(runtime.level_name,name);
            f=mcc_open_explain(name,&runtime.source,&identity,reason,sizeof(reason));assert(f);
            runtime.report.external_sound_count=!unused;runtime.report.external_bitmap_count=both;
            assert(mcc_resources_select_paths(&runtime,NULL,paths));
            assert(mcc_resources_fingerprint(&runtime,actual) && !memcmp(actual,digest,32));
            mcc_resources_dispose(&runtime);fclose(f);
            for(before=0;before<MCC_PRECACHE_FILES;before++) {
                stamps.files[before].written_low++;
                assert(!mcc_validation_resources(name,paths));stamps.files[before].written_low--;
                assert(mcc_validation_publish(name,&proof));
            }
        }
        if(mismatch)digest[i]^=1;
    }
    return 0;
}

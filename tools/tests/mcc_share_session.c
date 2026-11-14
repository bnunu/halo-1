/* Runs the real staging, list and session code: a host and a joiner exchange the
 * MCC share messages in one process over real files in a temporary folder. No
 * game, no sockets, no game data. argv[1] is a case, argv[2] an empty folder. */
#define _POSIX_C_SOURCE 200809L
#include "mcc_share_session.h"
#include "mcc_package.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir(path,mode) _mkdir(path)
#define rmdir _rmdir
#define unlink _unlink
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT)==_S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT)==_S_IFREG)
#endif
#else
#include <unistd.h>
#endif

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); return 1; } } while (0)
#define CASE(x) if (!strcmp(argv[1],x))

#define LEVEL "mcc_maps\\solar_flare\\m0"
#define MACHINE 3

static int pending_check_open, pending_delete_violation;

static int io_make(char const *path) { return mkdir(path,0755)==0; }
static int io_remove(char const *path) { return rmdir(path)==0; }
static int io_exists(char const *path) { struct stat s; return stat(path,&s)==0 && S_ISDIR(s.st_mode); }
static int file_exists_any(char const *path) { struct stat s; return stat(path,&s)==0; }
/* (MoveFileA does not replace, so nothing here may count on rename replacing either) */
static int io_move(char const *from, char const *to) { return !file_exists_any(to) && rename(from,to)==0; }
static int io_delete(char const *path) {
    if (pending_check_open) pending_delete_violation++;
    return remove(path)==0;
}
static unsigned native_reads,native_cached_reads;
static FILE *io_open_native(char const *path) {
    native_reads++;
    if (strstr(path,"/cached/")) native_cached_reads++;
    return fopen(path,"rb");
}
int posix_mcc_share_path_safe(char const *path);
int posix_mcc_share_publish(char const *from,char const *to);
static struct mcc_share_io const real_io={io_make,io_remove,io_exists,io_move,io_delete,
    posix_mcc_share_path_safe,posix_mcc_share_publish,io_open_native,NULL};
static int stage_test_verified(void *user,char const *path) { (void)user;(void)path;return 1; }
/* a folder check that misses a package folder made meanwhile (another process) */
static int io_exists_late(char const *path)
{
    size_t n=strlen(path);
    if (n>=13 && !strcmp(path+n-13,"mcc_maps/pack")) return 0;
    return io_exists(path);
}
static struct mcc_share_io const late_io={io_make,io_remove,io_exists_late,io_move,io_delete,
    posix_mcc_share_path_safe,posix_mcc_share_publish,io_open_native,NULL};

static int file_exists(char const *path) { struct stat s; return stat(path,&s)==0 && S_ISREG(s.st_mode); }

static unsigned char *make_bytes(unsigned size, unsigned seed)
{
    unsigned char *data=malloc(size?size:1);
    unsigned i,x=seed*2654435761u+1;
    for (i=0;i<size;i++) {
        x=x*1664525u+1013904223u;
        data[i]=(unsigned char)(x>>24);
    }
    return data;
}

static int write_file(char const *path, unsigned char const *data, unsigned size)
{
    FILE *f=fopen(path,"wb");
    if (!f) return 0;
    if (fwrite(data,1,size,f)!=size) {fclose(f);return 0;}
    return fclose(f)==0;
}

static unsigned char *read_file(char const *path, unsigned *size)
{
    FILE *f=fopen(path,"rb");
    unsigned char *data;
    long n;
    if (!f) return NULL;
    fseek(f,0,SEEK_END);
    n=ftell(f);
    fseek(f,0,SEEK_SET);
    data=malloc((size_t)n+1);
    if (fread(data,1,(size_t)n,f)!=(size_t)n) {fclose(f);free(data);return NULL;}
    fclose(f);
    *size=(unsigned)n;
    return data;
}

static void sha_of(unsigned char const *data, unsigned size, unsigned char out[32])
{
    struct mcc_sha256 h;
    mcc_sha256_begin(&h);
    mcc_sha256_add(&h,data,(int)size);
    mcc_sha256_end(&h,out);
}

static int same_file(char const *a, char const *b)
{
    unsigned sa,sb;
    unsigned char *da=read_file(a,&sa),*db=read_file(b,&sb);
    int same=da && db && sa==sb && !memcmp(da,db,sa);
    free(da);free(db);
    return same;
}

/* ---------- a host and a joiner */

static char H[200],J[200];                       /* the two machines' data roots, ending in / */
static char const *const names[]={"m0.map","bitmaps.map","sounds.map","credits.txt"};
static unsigned const sizes[]={15483,7000,6144,200};

struct message { unsigned bytes; unsigned char data[MCC_SHARE_RESPONSE_MAXIMUM+8]; };
static struct {
    struct mcc_share host,join;
    uint32_t clock;
    struct message to_host[600],to_join[600];
    int host_count,join_count;
    enum mcc_share_result refusal;
    int taken,identity_ok,finished,finished_ok;
    char finished_text[256];
    uint32_t limit;
    unsigned char digest[32];
    long last_machine;
    int responses;
    int identity_calls;
    int prepare_calls, prepare_ok, prepare_order;
    int reject_requests, reject_responses, reject_done, reject_have;
    unsigned delivered_bytes, response_bytes;
    unsigned reuse_mask, reuse_candidates, have_requests, file_requests[MCC_SHARE_STAGE_FILES];
    int reuse_missing;
    int offers, host_completions;
    uint64_t offer_bytes;
    unsigned extra_metadata, manifest_pages;
    int stop_after_page;
    int check_result,check_polls,check_cancels,install_calls,install_ok,prepare_mode;
    int verify_heartbeats;
    /* reuse_wait: its calls and answer; whether the held files have come in */
    int wait_calls, wait_answer, reuse_ready;
    /* hash_begin's jobs: their files, polls each, cancels; a failing one */
    char hash_paths[4][1024];
    int hash_jobs, hash_polls[4], hash_cancels, hash_fail;
} W;

static uint32_t env_now(void *u) { (void)u; return W.clock; }
static void env_host_finished(void *u,long machine) { (void)u; if(machine!=MACHINE)exit(91); W.host_completions++; }
static int env_request(void *u, struct mcc_share_request const *r)
{
    if (r->op==MCC_SHARE_OP_MANIFEST && W.join.state==MCC_SHARE_CLIENT_VERIFYING)
        W.verify_heartbeats++;
    (void)u;
    if (W.reject_requests || (W.reject_done && r->op==MCC_SHARE_OP_DONE) ||
        (W.reject_have && r->op==MCC_SHARE_OP_HAVE)) return 0;
    if (W.host_count<600) {
        if (r->op==MCC_SHARE_OP_CHUNK) W.file_requests[r->file]++;
        if (r->op==MCC_SHARE_OP_HAVE) W.have_requests++;
        W.to_host[W.host_count].bytes=sizeof(*r);
        memcpy(W.to_host[W.host_count].data,r,sizeof(*r));
        W.host_count++;
        return 1;
    }
    return 0;
}
static int env_response(void *u, long machine, void const *m, unsigned bytes)
{
    struct mcc_share_response head;
    (void)u;
    if (W.reject_responses) return 0;
    memcpy(&head,m,sizeof(head));
    if (W.stop_after_page && head.op==MCC_SHARE_OP_MANIFEST && W.manifest_pages) return 0;
    W.last_machine=machine;
    W.responses++;
    W.response_bytes+=bytes;
    if (head.op==MCC_SHARE_OP_MANIFEST && head.result==MCC_SHARE_RESULT_OK) W.manifest_pages++;
    if (head.op==MCC_SHARE_OP_CHUNK && head.result==MCC_SHARE_RESULT_OK) W.delivered_bytes+=head.length;
    if (W.join_count<600 && machine==MACHINE) {
        W.to_join[W.join_count].bytes=bytes;
        memcpy(W.to_join[W.join_count].data,m,bytes);
        W.join_count++;
    }
    return 1;
}
static void env_offered(void *u, uint64_t bytes, char const *credit)
{
    (void)u;(void)credit;
    W.offers++;
    W.offer_bytes=bytes;
}
static int env_taken(void *u, char const *level) { (void)u;(void)level; return W.taken; }
static void identity_digest(char const *dir, unsigned char out[32], int *ok)
{
    struct mcc_sha256 h;
    unsigned i;
    mcc_sha256_begin(&h);
    *ok=1;
    for (i=0;i<3;i++) {
        char path[300];
        unsigned size;
        unsigned char *data;
        snprintf(path,sizeof(path),"%s/%s",dir,names[i]);
        data=read_file(path,&size);
        if (!data) {*ok=0;break;}
        mcc_sha256_add(&h,data,(int)size);
        free(data);
    }
    mcc_sha256_end(&h,out);
}
static int env_identity(void *u, char const *dir)
{
    unsigned char got[32];
    int ok;
    (void)u;
    W.identity_calls++;
    identity_digest(dir,got,&ok);
    return W.identity_ok && ok && !memcmp(got,W.digest,32);
}
static int env_prepare(void *u, struct mcc_share_stage *stage)
{
    (void)u;
    W.prepare_calls++;
    W.prepare_order=W.identity_calls==1 && stage->verified &&
        stage->state==MCC_SHARE_STAGE_STAGED && !io_exists(stage->package_dir);
    return W.prepare_ok;
}
static int env_pending_check(void *u, char const *dir)
{
    (void)u;
    if (!io_exists(dir)) exit(92);
    pending_check_open=1;
    W.check_polls++;
    return W.check_result;
}
static void env_cancel_check(void *u)
{
    (void)u;
    W.check_cancels++;
    pending_check_open=0;
}
static int env_installed_ready(void *u)
{
    (void)u;
    if (!pending_check_open || W.check_cancels || W.join.stage.state!=MCC_SHARE_STAGE_INSTALLED ||
        !io_exists(W.join.stage.package_dir) || W.join.terminal_pending) exit(93);
    W.install_calls++;
    return W.install_ok;
}
static int fail_flush(FILE *file) { (void)file;return 0; }
static int fail_publish(char const *from,char const *to) { (void)from;(void)to;return 0; }
static int env_pending_prepare(void *u,struct mcc_share_stage *stage)
{
    static struct mcc_share_direct_io const failed_receipt={NULL,NULL,fail_flush};
    (void)u;
    if (!pending_check_open || !stage->verified) exit(94);
    if (W.prepare_mode==1) return 0;
    if (W.prepare_mode==2) {
        stage->io.direct=&failed_receipt;
        return mcc_share_stage_companion(stage,MCC_SHARE_STAGE_RECEIPT,"receipt",7);
    }
    if (W.prepare_mode==3) stage->io.publish_directory=fail_publish;
    return 1;
}
static int env_reuse(void *u, struct mcc_share_file const *file, char path[1024])
{
    (void)u;
    if (!(W.reuse_mask&(1u<<file->kind))) return 0;
    W.reuse_candidates++;
    snprintf(path,1024,"%scached/%s",J,W.reuse_missing ? "missing.map" : file->leaf);
    return 1;
}
/* a resource on its way from elsewhere (the Workshop): not here until reuse_ready */
static int env_reuse_late(void *u, struct mcc_share_file const *file, char path[1024])
{
    return W.reuse_ready ? env_reuse(u,file,path) : 0;
}
static int env_wait(void *u, struct mcc_share_file const *file, uint32_t host_rate)
{
    (void)u;(void)file;(void)host_rate;
    W.wait_calls++;
    return W.wait_answer;
}
/* a file hashed elsewhere: done on its third poll */
static int env_hash_begin(void *u, char const *path)
{
    (void)u;
    if (W.hash_jobs>=4) return -1;
    snprintf(W.hash_paths[W.hash_jobs],sizeof(W.hash_paths[0]),"%s",path);
    return W.hash_jobs++;
}
static int env_hash_poll(void *u, int job, uint8_t digest[32], uint64_t *size)
{
    struct mcc_sha256 h;
    unsigned char *data;
    unsigned bytes;
    (void)u;
    if (job<0 || job>=W.hash_jobs) exit(93);
    if (++W.hash_polls[job]<3) return 0;
    if (W.hash_fail) return -1;
    data=read_file(W.hash_paths[job],&bytes);
    if (!data) return -1;
    mcc_sha256_begin(&h);mcc_sha256_add(&h,data,(int)bytes);mcc_sha256_end(&h,digest);
    free(data);
    *size=bytes;
    return 1;
}
static void env_hash_cancel(void *u, int job) { (void)u;(void)job; W.hash_cancels++; }
static void env_finished(void *u, int ok, char const *text)
{
    (void)u;
    W.finished++;
    W.finished_ok=ok;
    snprintf(W.finished_text,sizeof(W.finished_text),"%s",text);
}
static void env_package(void *u, char const *level, struct mcc_share_host_package *p)
{
    unsigned i;
    (void)u;
    memset(p,0,sizeof(*p));
    p->refusal=strcmp(level,LEVEL) ? MCC_SHARE_RESULT_NOT_SHAREABLE : W.refusal;
    snprintf(p->credit,sizeof(p->credit),"Example Maker");
    p->count=4;
    for (i=0;i<4;i++) {
        p->files[i].kind=(uint8_t)(i==3 ? MCC_SHARE_FILE_CREDIT : i);
        snprintf(p->files[i].leaf,sizeof(p->files[i].leaf),"%s",names[i]);
        snprintf(p->files[i].path,sizeof(p->files[i].path),"%smcc_maps/solar_flare/%s",H,names[i]);
    }
    for (i=0;i<W.extra_metadata;i++) {
        unsigned at=(unsigned)p->count++;
        p->files[at].kind=MCC_SHARE_FILE_JSON;
        snprintf(p->files[at].leaf,sizeof(p->files[at].leaf),"metadata/mission%02u.json",i);
        snprintf(p->files[at].path,sizeof(p->files[at].path),"%smetadata%02u.json",H,i);
    }
}
static uint32_t env_limit(void *u) { (void)u; return W.limit; }
static unsigned long long free_space;
static uint64_t env_free(void *u) { (void)u; return free_space; }

static int setup(char const *base)
{
    unsigned i;
    char path[300];
    struct mcc_share_env env;
    memset(&W,0,sizeof(W));
    snprintf(H,sizeof(H),"%s/host/",base);
    snprintf(J,sizeof(J),"%s/join/",base);
    snprintf(path,sizeof(path),"%s/host",base); if (mkdir(path,0755)) return 0;
    snprintf(path,sizeof(path),"%s/join",base); if (mkdir(path,0755)) return 0;
    snprintf(path,sizeof(path),"%smcc_maps",H); if (mkdir(path,0755)) return 0;
    snprintf(path,sizeof(path),"%smcc_maps/solar_flare",H); if (mkdir(path,0755)) return 0;
    for (i=0;i<4;i++) {
        unsigned char *data=make_bytes(sizes[i],i+1);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/%s",H,names[i]);
        if (!write_file(path,data,sizes[i])) return 0;
        free(data);
    }
    {
        char dir[300];
        int ok;
        snprintf(dir,sizeof(dir),"%smcc_maps/solar_flare",H);
        identity_digest(dir,W.digest,&ok);
        if (!ok) return 0;
    }
    memset(&env,0,sizeof(env));
    env.now=env_now;env.send_request=env_request;env.send_response=env_response;
    env.package_taken=env_taken;env.identity_check=env_identity;env.finished=env_finished;
    env.host_package=env_package;env.upload_limit=env_limit;env.free_bytes=env_free;env.io=real_io;
    env.host_finished=env_host_finished;
    snprintf(env.root,sizeof(env.root),"%s",J);
    mcc_share_init(&W.host,&env);
    mcc_share_init(&W.join,&env);
    W.refusal=MCC_SHARE_RESULT_OK;
    W.identity_ok=1;
    W.limit=4u<<20;
    W.clock=1000;
    return 1;
}

/* Deliver everything in flight, run both frames, and let 20 ms pass. */
static void step(void)
{
    int i,count;
    count=W.host_count;W.host_count=0;
    {
        static struct message batch[600];
        memcpy(batch,W.to_host,(size_t)count*sizeof(batch[0]));
        for (i=0;i<count;i++) mcc_share_host_request(&W.host,MACHINE,batch[i].data,batch[i].bytes);
    }
    count=W.join_count;W.join_count=0;
    {
        static struct message batch[600];
        memcpy(batch,W.to_join,(size_t)count*sizeof(batch[0]));
        for (i=0;i<count;i++) mcc_share_client_response(&W.join,batch[i].data,batch[i].bytes);
    }
    mcc_share_update(&W.host);
    mcc_share_update(&W.join);
    W.clock+=20;
}

static int run_until_finished(int limit)
{
    int i;
    for (i=0;i<limit && !W.finished;i++) step();
    return W.finished;
}

static int staging_gone(void)
{
    char path[300];
    snprintf(path,sizeof(path),"%smcc_staging/solar_flare",J);
    return !io_exists(path);
}

static int package_absent(void)
{
    char path[300];
    snprintf(path,sizeof(path),"%smcc_maps/solar_flare/m0.map",J);
    return !file_exists(path);
}

static void install_hostile(struct mcc_share_entry *entries, unsigned count, int truncate)
{
    struct mcc_share_response head;
    unsigned char buffer[MCC_SHARE_RESPONSE_MAXIMUM];
    unsigned bytes=(unsigned)(sizeof(head)+count*sizeof(entries[0]));
    memset(&head,0,sizeof(head));
    head.op=MCC_SHARE_OP_MANIFEST;
    head.session=77;
    head.count=(uint8_t)count;
    head.length=count;
    snprintf(head.level,sizeof(head.level),"%s",LEVEL);
    memcpy(buffer,&head,sizeof(head));
    memcpy(buffer+sizeof(head),entries,count*sizeof(entries[0]));
    if (truncate) bytes-=10;
    mcc_share_client_response(&W.join,buffer,bytes);
}

static void entry(struct mcc_share_entry *e, int kind, char const *leaf, uint32_t size)
{
    memset(e,0,sizeof(*e));
    e->kind=(uint8_t)kind;
    snprintf(e->leaf,sizeof(e->leaf),"%s",leaf);
    e->size=size;
    memset(e->sha256,7,32);
}

static char const *env_identity_error(void *user)
{
    (void)user;
    return "Cannot read mcc_maps/_shared/multiplayer/sounds.map. Check access to this MCC resource.";
}

int main(int argc,char **argv)
{
    unsigned i;
    if (argc!=3) return 2;

    CASE("list") {
        unsigned char d[32],e[32];
        char hex[65],credit[MCC_SHARE_CREDIT_SIZE],text[512];
        for (i=0;i<32;i++) {d[i]=(unsigned char)i;e[i]=(unsigned char)(i+1);}
        mcc_share_list_hex(d,hex);
        CHECK(!strcmp(hex,"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));
        snprintf(text,sizeof(text),"# comment\n\n%s  Example Maker \r\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==1 && !strcmp(credit,"Example Maker"));
        CHECK(mcc_share_list_find(text,strlen(text),e,credit)==0 && !credit[0]);
        snprintf(text,sizeof(text),"%s\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==-1);
        snprintf(text,sizeof(text),"%s   \n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==-1);
        snprintf(text,sizeof(text),"%sXMaker\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==0);
        snprintf(text,sizeof(text),"#%s Maker\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==0);
        snprintf(text,sizeof(text),"%.63s Maker\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==0);
        hex[10]='g';
        snprintf(text,sizeof(text),"%s Maker\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==0);
        mcc_share_list_hex(d,hex);
        for (i=0;i<80;i++) text[i]=(char)('a'+i%26);
        snprintf(text+80,sizeof(text)-80,"\n%s Maker with \001 control and a very long tail that goes past sixty-three characters of room\n",hex);
        CHECK(mcc_share_list_find(text,strlen(text),d,credit)==1);
        CHECK(strlen(credit)==MCC_SHARE_CREDIT_SIZE-1 || strlen(credit)<MCC_SHARE_CREDIT_SIZE);
        CHECK(!strchr(credit,1));
        CHECK(mcc_share_list_find(NULL,0,d,credit)==0);
        CHECK(mcc_share_list_find("",0,d,credit)==0);
        return 0;
    }

    CASE("admit") {
        unsigned char d[32];
        char hex[65],list[200],credit[MCC_SHARE_CREDIT_SIZE];
        for (i=0;i<32;i++) d[i]=(unsigned char)(i*3);
        mcc_share_list_hex(d,hex);
        snprintf(list,sizeof(list),"%s Example Maker\n",hex);
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK);
        CHECK(!strcmp(credit,"Example Maker"));
        CHECK(mcc_share_admit("mcc_maps/solar_flare/m0",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK);
        /* disabled sharing decides first and nothing is read from the optional list */
        CHECK(mcc_share_admit(LEVEL,0,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OFF && !credit[0]);
        /* unlisted, unknown fingerprint, other fingerprint */
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_ABSENT,d,1,"",0,credit)==MCC_SHARE_RESULT_OK);
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_ABSENT,d,0,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        d[0]^=1;
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK);
        d[0]^=1;
        /* a missing maker is optional attribution, not an admission requirement */
        snprintf(list,sizeof(list),"%s\n",hex);
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK && !credit[0]);
        CHECK(mcc_share_admit("mcc_maps/2915693904/a10",1,MCC_SHARE_SIDECAR_VALID,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK && !credit[0]);
        snprintf(list,sizeof(list),"%s Example Maker\n",hex);
        /* Workshop and local packages use the same automatic policy. */
        CHECK(mcc_share_admit("mcc_maps\\2915693904\\a10",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK);
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_VALID,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OK);
        CHECK(!strcmp(credit,"Example Maker"));
        CHECK(mcc_share_admit(LEVEL,0,MCC_SHARE_SIDECAR_VALID,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_OFF && !credit[0]);
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_VALID,d,1,"",0,credit)==MCC_SHARE_RESULT_OK && !credit[0]);
        /* The remaining rules still refuse invalid origins/names and shared resources. */
        CHECK(mcc_share_admit(LEVEL,1,MCC_SHARE_SIDECAR_UNREADABLE,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        CHECK(mcc_share_admit("mcc_maps\\loose",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        CHECK(mcc_share_admit("mcc_maps\\_shared\\multiplayer",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        CHECK(mcc_share_admit("mcc_maps\\a\\CON",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        CHECK(mcc_share_admit("mcc_maps\\a\\..\\b",1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        CHECK(mcc_share_admit(NULL,1,MCC_SHARE_SIDECAR_ABSENT,d,1,list,strlen(list),credit)==MCC_SHARE_RESULT_NOT_SHAREABLE);
        return 0;
    }

    CASE("manifest") {
        struct mcc_share_file f[MCC_SHARE_STAGE_FILES];
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=100;
        f[1].kind=MCC_SHARE_FILE_BITMAPS;strcpy(f[1].leaf,"BITMAPS.MAP");f[1].size=5;
        f[2].kind=MCC_SHARE_FILE_SOUNDS;strcpy(f[2].leaf,"sounds.map");f[2].size=5;
        f[3].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[3].leaf,"ModInfo.json");f[3].size=50;
        CHECK(!mcc_share_manifest_valid(f,4,"m0")); /* JSON cannot bypass schema validation as a credit. */
        f[3].kind=MCC_SHARE_FILE_JSON;
        CHECK(mcc_share_manifest_valid(f,4,"m0"));
        f[3].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[3].leaf,"credits.txt");
        CHECK(mcc_share_manifest_valid(f,4,"m0"));
        CHECK(mcc_share_manifest_valid(f,1,"M0"));              /* the map's name matches without case */
        CHECK(!mcc_share_manifest_valid(f,0,"m0"));
        CHECK(!mcc_share_manifest_valid(f,4,"other"));          /* the map is the level's */
        CHECK(!mcc_share_manifest_valid(f,4,"m0/x"));
        CHECK(!mcc_share_manifest_valid(NULL,4,"m0"));
        CHECK(!mcc_share_manifest_valid(f,4,NULL));
        f[1].size=0;
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        f[1].size=5;f[0].size=MCC_SHARE_MAXIMUM_FILE_BYTES+1;
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        f[0].size=MCC_SHARE_MAXIMUM_FILE_BYTES;f[1].size=MCC_SHARE_MAXIMUM_FILE_BYTES;f[2].size=MCC_SHARE_MAXIMUM_FILE_BYTES;
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));             /* over the total */
        f[0].size=100;f[1].size=5;f[2].size=5;
        f[3].size=MCC_SHARE_MAXIMUM_CREDIT_BYTES+1;
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        f[3].size=50;
        strcpy(f[3].leaf,"workshop_id.txt");
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        strcpy(f[3].leaf,"loc.map");
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        strcpy(f[3].leaf,"readme.txt");
        CHECK(mcc_share_manifest_valid(f,4,"m0"));
        f[3].kind=MCC_SHARE_FILE_MAP;                           /* two maps */
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        f[3].kind=MCC_SHARE_FILE_KINDS;
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        f[3].kind=MCC_SHARE_FILE_CREDIT;
        strcpy(f[3].leaf,"../readme.txt");
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        strcpy(f[3].leaf,"a\\readme.txt");
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        memset(f[3].leaf,'a',sizeof(f[3].leaf));                /* no terminator */
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        strcpy(f[3].leaf,"credits.txt");
        f[2].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[2].leaf,"CREDITS.TXT"); /* a repeated name */
        CHECK(!mcc_share_manifest_valid(f,4,"m0"));
        return 0;
    }

    if (!argv[2][0]) return 2;

    CASE("redirect") {
        char path[128];
        /* the package's own files read from the staging folder while the redirect is set, and only that package's */
        CHECK(mcc_package_map_path("mcc_maps\\solar_flare\\m0",path) && !strcmp(path,"d:\\mcc_maps\\solar_flare\\m0.map"));
        mcc_package_stage_redirect("MCC_MAPS/Solar_Flare/M0","mcc_staging/solar_flare");
        CHECK(mcc_package_map_path("mcc_maps\\solar_flare\\m0",path) && !strcmp(path,"d:\\mcc_staging\\solar_flare\\m0.map"));
        CHECK(mcc_package_resource_path("mcc_maps\\solar_flare\\m0",1,path) && !strcmp(path,"mcc_staging/solar_flare/bitmaps.map"));
        CHECK(mcc_package_resource_path("mcc_maps\\solar_flare\\m0",2,path) && !strcmp(path,"mcc_staging/solar_flare/sounds.map"));
        CHECK(mcc_package_resource_path("mcc_maps/root",1,path) && !strcmp(path,"mcc_maps/bitmaps.map"));
        CHECK(mcc_package_map_path("mcc_maps\\solar_flare\\m1",path) && !strcmp(path,"d:\\mcc_maps\\solar_flare\\m1.map"));
        CHECK(mcc_package_map_path("mcc_maps\\other\\m0",path) && !strcmp(path,"d:\\mcc_maps\\other\\m0.map"));
        mcc_package_stage_redirect(NULL,NULL);
        CHECK(mcc_package_map_path("mcc_maps\\solar_flare\\m0",path) && !strcmp(path,"d:\\mcc_maps\\solar_flare\\m0.map"));
        CHECK(mcc_package_resource_path("mcc_maps\\solar_flare\\m0",1,path) && !strcmp(path,"mcc_maps/solar_flare/bitmaps.map"));
        mcc_package_stage_redirect("not a name","x");
        CHECK(mcc_package_map_path("mcc_maps\\solar_flare\\m0",path) && !strcmp(path,"d:\\mcc_maps\\solar_flare\\m0.map"));
        return 0;
    }

    CASE("stage_cancel") {
        struct mcc_share_stage s;
        struct mcc_share_file f[2];
        unsigned char *a=make_bytes(1000,1),*b=make_bytes(500,2);
        char path[300];
        snprintf(path,sizeof(path),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        f[1].kind=MCC_SHARE_FILE_BITMAPS;strcpy(f[1].leaf,"bitmaps.map");f[1].size=500;sha_of(b,500,f[1].sha256);
        CHECK(mcc_share_stage_begin(&s,&real_io,path,"pack","m0",f,2)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_wanted(&s)==1000);
        CHECK(mcc_share_stage_write(&s,a,400)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_wanted(&s)==600);
        snprintf(path,sizeof(path),"%s/mcc_staging/pack/m0.map.part",argv[2]);
        CHECK(file_exists(path));
        mcc_share_stage_cancel(&s);
        CHECK(!file_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_maps/pack",argv[2]);
        CHECK(!io_exists(path));
        CHECK(mcc_share_stage_write(&s,a,1)==MCC_SHARE_STAGE_BAD_STATE);
        free(a);free(b);
        return 0;
    }

    CASE("stage_hash_and_size") {
        struct mcc_share_stage s;
        struct mcc_share_file f[1];
        unsigned char *a=make_bytes(1000,1);
        char path[300];
        snprintf(path,sizeof(path),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        /* a wrong byte */
        CHECK(mcc_share_stage_begin(&s,&real_io,path,"pack","m0",f,1)==MCC_SHARE_STAGE_OK);
        a[999]^=1;
        CHECK(mcc_share_stage_write(&s,a,999)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a+999,1)==MCC_SHARE_STAGE_BAD_HASH);
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        a[999]^=1;
        /* more than the manifest's size */
        snprintf(path,sizeof(path),"%s/",argv[2]);
        CHECK(mcc_share_stage_begin(&s,&real_io,path,"pack","m0",f,1)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,900)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,101)==MCC_SHARE_STAGE_TOO_MUCH);
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        /* the right bytes, in uneven pieces */
        snprintf(path,sizeof(path),"%s/",argv[2]);
        CHECK(mcc_share_stage_begin(&s,&real_io,path,"pack","m0",f,1)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,0)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,1)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a+1,998)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a+999,1)==MCC_SHARE_STAGE_ALL_DONE);
        CHECK(mcc_share_stage_write(&s,a,1)==MCC_SHARE_STAGE_BAD_STATE);
        mcc_share_stage_cancel(&s);
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        free(a);
        return 0;
    }

    CASE("stage_exists") {
        /* an existing package with the same name is never touched */
        struct mcc_share_stage s;
        struct mcc_share_file f[1];
        unsigned char *a=make_bytes(1000,1),*c=make_bytes(40,3);
        char root[300],path[300];
        unsigned size;
        unsigned char *data;
        snprintf(root,sizeof(root),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        snprintf(path,sizeof(path),"%s/mcc_maps",argv[2]);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%s/mcc_maps/Pack",argv[2]);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%s/mcc_maps/Pack/m0.map",argv[2]);
        CHECK(write_file(path,c,40));
        CHECK(mcc_share_stage_begin(&s,&real_io,root,"Pack","m0",f,1)==MCC_SHARE_STAGE_EXISTS);
        data=read_file(path,&size);
        CHECK(data && size==40 && !memcmp(data,c,40));
        free(data);free(a);free(c);
        snprintf(path,sizeof(path),"%s/mcc_staging",argv[2]);
        CHECK(!io_exists(path));
        return 0;
    }

    CASE("stage_install") {
        struct mcc_share_stage s;
        struct mcc_share_file f[3];
        unsigned char *a=make_bytes(1000,1),*b=make_bytes(500,2),*c=make_bytes(40,3);
        char root[300],path[300];
        unsigned size;
        unsigned char *data;
        snprintf(root,sizeof(root),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        f[1].kind=MCC_SHARE_FILE_BITMAPS;strcpy(f[1].leaf,"bitmaps.map");f[1].size=500;sha_of(b,500,f[1].sha256);
        f[2].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[2].leaf,"credits.txt");f[2].size=40;sha_of(c,40,f[2].sha256);
        CHECK(mcc_share_stage_begin(&s,&real_io,root,"pack","m0",f,3)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,1000)==MCC_SHARE_STAGE_FILE_DONE);
        CHECK(mcc_share_stage_write(&s,b,500)==MCC_SHARE_STAGE_FILE_DONE);
        CHECK(mcc_share_stage_write(&s,c,40)==MCC_SHARE_STAGE_ALL_DONE);
        CHECK(mcc_share_stage_install(&s)==MCC_SHARE_STAGE_BAD_STATE);
        CHECK(mcc_share_stage_verify(&s,stage_test_verified,NULL)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_install(&s)==MCC_SHARE_STAGE_OK);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/m0.map",argv[2]);
        data=read_file(path,&size);
        CHECK(data && size==1000 && !memcmp(data,a,1000));free(data);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/bitmaps.map",argv[2]);
        data=read_file(path,&size);
        CHECK(data && size==500 && !memcmp(data,b,500));free(data);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/credits.txt",argv[2]);
        CHECK(file_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        mcc_share_stage_cancel(&s);                 /* after an install: nothing of the package is touched */
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/m0.map",argv[2]);
        CHECK(file_exists(path));
        free(a);free(b);free(c);
        return 0;
    }

    CASE("stage_race") {
        /* the package folder appears after the first check, as another process could make it:
         * the install refuses and deletes nothing of the folder */
        struct mcc_share_stage s;
        struct mcc_share_file f[2];
        unsigned char *a=make_bytes(1000,1),*c=make_bytes(40,3);
        char root[300],path[300];
        snprintf(root,sizeof(root),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        f[1].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[1].leaf,"readme.txt");f[1].size=40;sha_of(c,40,f[1].sha256);
        CHECK(mcc_share_stage_begin(&s,&late_io,root,"pack","m0",f,2)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,1000)==MCC_SHARE_STAGE_FILE_DONE);
        CHECK(mcc_share_stage_write(&s,c,40)==MCC_SHARE_STAGE_ALL_DONE);
        snprintf(path,sizeof(path),"%s/mcc_maps",argv[2]);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack",argv[2]);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/keep.txt",argv[2]);
        CHECK(write_file(path,c,40));
        CHECK(mcc_share_stage_verify(&s,stage_test_verified,NULL)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_install(&s)==MCC_SHARE_STAGE_INSTALL_FAILED);
        CHECK(file_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_maps/pack/m0.map",argv[2]);
        CHECK(!file_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        free(a);free(c);
        return 0;
    }

    CASE("stage_move_failure") {
        /* A staged file removed before publication prevents any install. */
        struct mcc_share_stage s;
        struct mcc_share_file f[3];
        unsigned char *a=make_bytes(1000,1),*b=make_bytes(500,2),*c=make_bytes(40,3);
        char root[300],path[300];
        snprintf(root,sizeof(root),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=1000;sha_of(a,1000,f[0].sha256);
        f[1].kind=MCC_SHARE_FILE_BITMAPS;strcpy(f[1].leaf,"bitmaps.map");f[1].size=500;sha_of(b,500,f[1].sha256);
        f[2].kind=MCC_SHARE_FILE_CREDIT;strcpy(f[2].leaf,"readme.txt");f[2].size=40;sha_of(c,40,f[2].sha256);
        CHECK(mcc_share_stage_begin(&s,&real_io,root,"pack","m0",f,3)==MCC_SHARE_STAGE_OK);
        CHECK(mcc_share_stage_write(&s,a,1000)==MCC_SHARE_STAGE_FILE_DONE);
        CHECK(mcc_share_stage_write(&s,b,500)==MCC_SHARE_STAGE_FILE_DONE);
        CHECK(mcc_share_stage_write(&s,c,40)==MCC_SHARE_STAGE_ALL_DONE);
        CHECK(mcc_share_stage_verify(&s,stage_test_verified,NULL)==MCC_SHARE_STAGE_OK);
        /* The staged credit file vanishes before publication. */
        snprintf(path,sizeof(path),"%s/mcc_staging/pack/readme.txt",argv[2]);
        CHECK(unlink(path)==0);
        CHECK(mcc_share_stage_install(&s)==MCC_SHARE_STAGE_INSTALL_FAILED);
        snprintf(path,sizeof(path),"%s/mcc_maps/pack",argv[2]);
        CHECK(!io_exists(path));
        snprintf(path,sizeof(path),"%s/mcc_staging/pack",argv[2]);
        CHECK(!io_exists(path));
        free(a);free(b);free(c);
        return 0;
    }

    CASE("stage_names") {
        struct mcc_share_stage s;
        struct mcc_share_file f[1];
        char root[300];
        unsigned char *a=make_bytes(10,1);
        static char const *const bad[]={"","..",".x","a/b","a\\b","x ","a:b","CON1:","a\"b"};
        snprintf(root,sizeof(root),"%s/",argv[2]);
        memset(f,0,sizeof(f));
        f[0].kind=MCC_SHARE_FILE_MAP;strcpy(f[0].leaf,"m0.map");f[0].size=10;sha_of(a,10,f[0].sha256);
        for (i=0;i<sizeof(bad)/sizeof(bad[0]);i++)
            CHECK(mcc_share_stage_begin(&s,&real_io,root,bad[i],"m0",f,1)==MCC_SHARE_STAGE_BAD_MANIFEST);
        CHECK(mcc_share_stage_begin(&s,&real_io,root,"pack","m0/..",f,1)==MCC_SHARE_STAGE_BAD_MANIFEST);
        CHECK(mcc_share_stage_begin(&s,NULL,root,"pack","m0",f,1)==MCC_SHARE_STAGE_BAD_MANIFEST);
        {
            char path[300];
            snprintf(path,sizeof(path),"%s/mcc_staging",argv[2]);
            /* a refused name made nothing under the data root */
            CHECK(!io_exists(path));
        }
        free(a);
        return 0;
    }

    if (!setup(argv[2])) {fprintf(stderr,"setup failed\n");return 2;}

    if (!strncmp(argv[1],"verify_",7)) {
        int frames=0,success=0,published=0;
        uint32_t progress;
        W.join.env.identity_check=env_pending_check;
        W.join.env.cancel_check=env_cancel_check;
        W.join.env.installed_ready=env_installed_ready;
        W.join.env.prepare_install=env_pending_prepare;
        W.check_result=MCC_SHARE_CHECK_PENDING;W.install_ok=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        while (!W.check_polls && frames++<2000) step();
        CHECK(W.check_polls && W.join.state==MCC_SHARE_CLIENT_VERIFYING && pending_check_open);
        CHECK(mcc_share_client_percent(&W.join)==100);
        CHECK(!W.join.stage.verified && !W.finished && package_absent());
        CHECK(mcc_share_stage_install(&W.join.stage)==MCC_SHARE_STAGE_BAD_STATE);
        progress=W.join.progress_at;
        if (!strcmp(argv[1],"verify_cancel") || !strcmp(argv[1],"verify_disconnect")) {
            if (!strcmp(argv[1],"verify_cancel")) mcc_share_client_cancel(&W.join);
            else mcc_share_client_disconnect(&W.join);
            CHECK(!W.finished && !mcc_share_client_busy(&W.join));
        } else if (!strcmp(argv[1],"verify_timeout")) {
            W.clock=progress+80000u;
            mcc_share_update(&W.join);
            CHECK(W.finished && !W.finished_ok && strstr(W.finished_text,"verified in time"));
        } else {
            if (!strcmp(argv[1],"verify_recheck")) {
                W.check_result=MCC_SHARE_CHECK_READY;
                CHECK(mcc_share_stage_verify(&W.join.stage,env_pending_check,NULL)==MCC_SHARE_STAGE_OK);
                CHECK(W.join.stage.verified);
                W.check_result=MCC_SHARE_CHECK_PENDING;
                CHECK(mcc_share_stage_verify(&W.join.stage,env_pending_check,NULL)==MCC_SHARE_STAGE_PENDING);
                CHECK(!W.join.stage.verified && mcc_share_stage_install(&W.join.stage)==MCC_SHARE_STAGE_BAD_STATE);
            }
            if (!strcmp(argv[1],"verify_heartbeat")) {
                uint32_t until=W.clock+45000u;
                while (W.clock<until) {
                    W.clock+=980;step();
                    CHECK(W.join.state==MCC_SHARE_CLIENT_VERIFYING && !W.finished);
                    CHECK(mcc_share_host_active(&W.host,MACHINE));
                }
                CHECK(W.verify_heartbeats>=4 && W.verify_heartbeats<=5);
                CHECK(W.join.progress_at==progress && !W.check_cancels);
            }
            W.check_result=!strcmp(argv[1],"verify_refused") ? MCC_SHARE_CHECK_FAILED :
                !strcmp(argv[1],"verify_invalid") ? 3 : MCC_SHARE_CHECK_READY;
            W.prepare_mode=!strcmp(argv[1],"verify_metadata_failure") ? 1 :
                !strcmp(argv[1],"verify_receipt_failure") ? 2 :
                !strcmp(argv[1],"verify_install_failure") ? 3 : 0;
            W.install_ok=strcmp(argv[1],"verify_bind_failure")!=0;
            published=W.check_result==MCC_SHARE_CHECK_READY && !W.prepare_mode;
            success=published && W.install_ok;
            step();
            CHECK(W.finished && W.finished_ok==success && W.install_calls==published);
            if (published && !success) CHECK(strstr(W.finished_text,"installed, but"));
            step();
            CHECK(W.host_completions==success);
        }
        CHECK(W.check_cancels==1 && !pending_check_open && !pending_delete_violation);
        CHECK(staging_gone() && package_absent()==!published);
        mcc_share_client_cancel(&W.join);mcc_share_client_disconnect(&W.join);
        CHECK(W.check_cancels==1);
        return 0;
    }

    if (!strcmp(argv[1],"clock_zero") || !strcmp(argv[1],"clock_wrap")) {
        W.clock=!strcmp(argv[1],"clock_zero") ? 0 : UINT32_MAX-99;
        W.host.budget_at=W.join.budget_at=W.clock;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        step();
        CHECK(mcc_share_host_active(&W.host,MACHINE));
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    if (!strcmp(argv[1],"done_expired") || !strcmp(argv[1],"done_cancelled")) {
        struct mcc_share_request request;
        W.reject_done=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000) && W.finished_ok && !W.host_completions);
        request=W.join.terminal;
        CHECK(request.op==MCC_SHARE_OP_DONE && request.flags==1);
        if (!strcmp(argv[1],"done_expired")) W.clock+=80001;
        else request.flags=0;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(!W.host_completions);
        mcc_share_update(&W.host);
        CHECK(!mcc_share_host_active(&W.host,MACHINE));
        request.flags=1;mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(!W.host_completions);
        return 0;
    }

    CASE("transfer") {
        char a[400],b[400];
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(mcc_share_client_busy(&W.join));
        CHECK(mcc_share_client_level(&W.join,"MCC_MAPS\\Solar_Flare\\M0"));
        CHECK(!mcc_share_client_begin(&W.join,LEVEL,W.digest));        /* one download at a time */
        CHECK(run_until_finished(2000));
        CHECK(W.finished==1 && W.finished_ok);
        CHECK(!strcmp(W.finished_text,"Example Maker"));
        for (i=0;i<4;i++) {
            snprintf(a,sizeof(a),"%smcc_maps/solar_flare/%s",H,names[i]);
            snprintf(b,sizeof(b),"%smcc_maps/solar_flare/%s",J,names[i]);
            CHECK(same_file(a,b));
        }
        CHECK(staging_gone());
        CHECK(!mcc_share_client_busy(&W.join));
        CHECK(mcc_share_client_percent(&W.join)==-1);
        for (i=0;i<5;i++) step();
        CHECK(!mcc_share_host_active(&W.host,MACHINE));                 /* the joiner's done ended it */
        CHECK(W.identity_calls==1);
        CHECK(native_reads==8 && !native_cached_reads); /* four hashes, and each file opened once for its chunks */
        CHECK(W.host_completions==1);
        mcc_share_host_request(&W.host,MACHINE,&W.join.terminal,sizeof(W.join.terminal));
        CHECK(W.host_completions==1); /* A replay cannot renew admission grace. */
        return 0;
    }

    CASE("rate") {
        uint32_t start=W.clock;
        W.limit=8192;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<30;i++) step();                                      /* 0.6 s */
        CHECK(!W.finished);
        CHECK(mcc_share_client_percent(&W.join)>=0);
        CHECK(mcc_share_host_active(&W.host,MACHINE));
        CHECK(run_until_finished(3000));
        CHECK(W.finished_ok);
        /* 28827 bytes at 8192 a second cannot take under two seconds (the first second's burst) */
        CHECK(W.clock-start>=2000);
        return 0;
    }

    CASE("aggregate_rate") {
        struct mcc_share_request request;
        unsigned machine, chunk;
        W.limit=8192;
        memset(&request,0,sizeof(request));
        request.op=MCC_SHARE_OP_MANIFEST;
        strcpy(request.level,LEVEL);
        for (machine=0;machine<MCC_SHARE_HOST_TRANSFERS;machine++)
            mcc_share_host_request(&W.host,machine,&request,sizeof(request));
        mcc_share_update(&W.host);
        for (machine=0;machine<MCC_SHARE_HOST_TRANSFERS;machine++) {
            request.op=MCC_SHARE_OP_CHUNK;
            request.session=W.host.transfers[machine].session;
            request.length=MCC_SHARE_CHUNK_BYTES;
            for (chunk=0;chunk<4;chunk++) {
                request.offset=chunk*MCC_SHARE_CHUNK_BYTES;
                mcc_share_host_request(&W.host,machine,&request,sizeof(request));
            }
        }
        for (i=0;i<50;i++) { W.clock+=20; mcc_share_update(&W.host); }
        /* One second of initial credit plus one elapsed second and at most one
         * chunk of debt, across all four transfers rather than per transfer. */
        CHECK(W.delivered_bytes<=2*W.limit+MCC_SHARE_CHUNK_BYTES);
        CHECK(W.delivered_bytes>W.limit);
        return 0;
    }

    CASE("offer_accept") {
        unsigned j;
        W.join.env.offered=env_offered;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<5;i++) step();
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.offers==1);
        CHECK(W.offer_bytes==sizes[0]+sizes[1]+sizes[2]+sizes[3]);
        CHECK(W.delivered_bytes==0 && staging_gone() && package_absent());
        for (j=0;j<4500;j++) step(); /* heartbeats hold the offer for a human decision */
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.offers==1);
        CHECK(mcc_share_host_active(&W.host,MACHINE));
        CHECK(W.delivered_bytes==0 && staging_gone());
        CHECK(mcc_share_client_accept(&W.join));
        CHECK(!mcc_share_client_accept(&W.join));
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    CASE("manifest_pages") {
        char path[300];
        unsigned char const json[]="{\"hello\":\"world\"}";
        W.extra_metadata=20;
        for (i=0;i<W.extra_metadata;i++) {
            snprintf(path,sizeof(path),"%smetadata%02u.json",H,i);
            CHECK(write_file(path,json,sizeof(json)-1));
        }
        W.join.env.offered=env_offered;W.stop_after_page=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<10;i++) step();
        CHECK(W.manifest_pages==1 && W.join.offer_received==MCC_SHARE_MANIFEST_PAGE);
        CHECK(W.join.state==MCC_SHARE_CLIENT_ASKING && !W.offers && staging_gone());
        W.stop_after_page=0;
        for (i=0;i<5;i++) step();
        CHECK(W.manifest_pages==2 && W.offers==1 && W.join.offer_count==24);
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.delivered_bytes==0);
        CHECK(W.offer_bytes==sizes[0]+sizes[1]+sizes[2]+sizes[3]+20*(sizeof(json)-1));
        CHECK(mcc_share_client_accept(&W.join));
        CHECK(run_until_finished(2000) && W.finished_ok);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/metadata/mission19.json",J);
        CHECK(file_exists(path));
        return 0;
    }

    CASE("offer_expiry") {
        W.join.env.offered=env_offered;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<6100 && !W.finished;i++) step();
        CHECK(W.finished==1 && !W.finished_ok && strstr(W.finished_text,"expired"));
        CHECK(!mcc_share_client_accept(&W.join));
        CHECK(W.delivered_bytes==0 && staging_gone() && package_absent());
        return 0;
    }

    CASE("manifest_rate") {
        struct mcc_share_request request;
        char path[300];
        unsigned machine,frame;
        uint32_t start=W.clock;
        W.limit=16384;W.extra_metadata=MCC_SHARE_STAGE_FILES-4;
        for (i=0;i<W.extra_metadata;i++) {
            snprintf(path,sizeof(path),"%smetadata%02u.json",H,i);
            CHECK(write_file(path,(unsigned char const *)"{}",2));
        }
        memset(&request,0,sizeof(request));request.op=MCC_SHARE_OP_MANIFEST;strcpy(request.level,LEVEL);
        for (frame=0;frame<500;frame++) {
            if (!(frame%50)) for (machine=0;machine<MCC_SHARE_HOST_TRANSFERS;machine++)
                mcc_share_host_request(&W.host,machine,&request,sizeof(request));
            mcc_share_update(&W.host);
            CHECK(W.response_bytes<=(uint64_t)W.limit*(1000+W.clock-start)/1000+MCC_SHARE_RESPONSE_MAXIMUM);
            W.clock+=20;
        }
        CHECK(W.manifest_pages>40 && W.delivered_bytes==0);
        return 0;
    }

    CASE("offer_delivery_deadline") {
        char path[300];
        struct mcc_share_host_transfer *transfer=&W.host.transfers[0];
        uint32_t completed;
        W.extra_metadata=20;
        for (i=0;i<W.extra_metadata;i++) {
            snprintf(path,sizeof(path),"%smetadata%02u.json",H,i);
            CHECK(write_file(path,(unsigned char const *)"{}",2));
        }
        W.join.env.offered=env_offered;W.stop_after_page=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<1000;i++) step();
        CHECK(!transfer->offer_complete && !W.offers && W.manifest_pages==1);
        W.stop_after_page=0;
        for (i=0;i<5;i++) step();
        completed=transfer->offered_at;
        CHECK(transfer->offer_complete && W.offers==1);
        for (i=0;i<5250;i++) step();
        CHECK(transfer->used && transfer->offered_at==completed && W.offers==1);
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.delivered_bytes==0);
        CHECK(mcc_share_client_accept(&W.join));
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    CASE("progress_expiry") {
        struct mcc_share_request request;
        struct mcc_share_host_transfer *transfer=&W.host.transfers[0];
        memset(&request,0,sizeof(request));request.op=MCC_SHARE_OP_MANIFEST;strcpy(request.level,LEVEL);
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));mcc_share_update(&W.host);
        request.op=MCC_SHARE_OP_CHUNK;request.session=transfer->session;request.length=1;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        W.clock+=20;mcc_share_update(&W.host);CHECK(transfer->sent_bytes==1);
        request.op=MCC_SHARE_OP_MANIFEST;request.session=0;request.length=0;
        for (i=0;i<7;i++) {
            W.clock+=10000;
            mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));mcc_share_update(&W.host);
            CHECK(transfer->used);
        }
        W.clock+=10000;mcc_share_update(&W.host);
        CHECK(!transfer->used);
        return 0;
    }

    if (!strncmp(argv[1],"terminal_",9)) {
        W.reject_done=1;
        if (!strcmp(argv[1],"terminal_success") || !strcmp(argv[1],"terminal_failure")) {
            W.identity_ok=!strcmp(argv[1],"terminal_success");
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            CHECK(run_until_finished(2000));
            CHECK(W.finished_ok==W.identity_ok);
            CHECK(W.join.terminal.flags==W.identity_ok);
        } else {
            W.join.env.offered=env_offered;
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            for (i=0;i<5;i++) step();
            CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING);
            mcc_share_client_cancel(&W.join);
        }
        CHECK(W.join.state==MCC_SHARE_CLIENT_IDLE && W.join.terminal_pending);
        if (!strcmp(argv[1],"terminal_new_request")) {
            uint32_t old_session=W.join.session;
            struct mcc_share_request done,manifest;
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            /* A delayed old offer cannot satisfy an intent not yet sent. */
            mcc_share_client_response(&W.join,W.to_join[0].data,W.to_join[0].bytes);
            CHECK(W.join.state==MCC_SHARE_CLIENT_ASKING && W.offers==1);
            for (i=0;i<100;i++) step();
            CHECK(W.join.state==MCC_SHARE_CLIENT_ASKING && W.join.terminal_pending && !W.host_count);
            W.reject_done=0;
            mcc_share_update(&W.join);
            CHECK(W.host_count==2);
            memcpy(&done,W.to_host[0].data,sizeof(done));
            memcpy(&manifest,W.to_host[1].data,sizeof(manifest));
            CHECK(done.op==MCC_SHARE_OP_DONE && done.session==old_session && manifest.op==MCC_SHARE_OP_MANIFEST);
            for (i=0;i<5;i++) step();
            CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.join.session!=old_session && W.offers==2);
        } else if (!strcmp(argv[1],"terminal_disconnect")) {
            mcc_share_client_disconnect(&W.join);
            W.reject_done=0;
            for (i=0;i<5;i++) step();
            CHECK(W.host.transfers[0].used && !W.join.terminal_pending && !W.host_count);
        } else if (!strcmp(argv[1],"terminal_expiry")) {
            W.clock+=30000;mcc_share_update(&W.join);
            CHECK(!W.join.terminal_pending);
            /* An expired slot cannot be revived by a fresh manifest request. */
            W.reject_done=0;
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            for (i=0;i<5;i++) step();
            CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING && W.offers==2);
        } else {
            W.reject_done=0;
            for (i=0;i<5;i++) step();
            CHECK(!W.join.terminal_pending && !W.host.transfers[0].used);
        }
        return 0;
    }

    if (!strncmp(argv[1],"prepare_",8)) {
        W.join.env.prepare_install=env_prepare;
        W.prepare_ok=!strcmp(argv[1],"prepare_success");
        W.identity_ok=strcmp(argv[1],"prepare_unverified")!=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000));
        if (W.identity_ok) {
            CHECK(W.prepare_calls==1 && W.prepare_order);
            CHECK(W.finished_ok==W.prepare_ok);
            if (!W.prepare_ok) CHECK(strstr(W.finished_text,"download metadata could not be saved"));
        } else CHECK(W.prepare_calls==0 && !W.finished_ok);
        CHECK(staging_gone() && (W.finished_ok || package_absent()));
        step();
        CHECK(!W.host.transfers[0].used);
        return 0;
    }

    CASE("offer_cancel") {
        W.join.env.offered=env_offered;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<5;i++) step();
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING);
        mcc_share_client_cancel(&W.join);
        step();
        CHECK(!mcc_share_host_active(&W.host,MACHINE));
        CHECK(W.delivered_bytes==0 && staging_gone() && package_absent());
        CHECK(!W.finished);
        return 0;
    }

    if (!strncmp(argv[1],"reuse_",6)) {
        char path[400],source[400];
        unsigned char *data;
        unsigned bytes;
        int frames=0,identity_ok;
        W.join.env.reuse_file=env_reuse;
        W.reuse_mask=(1u<<MCC_SHARE_FILE_BITMAPS)|(1u<<MCC_SHARE_FILE_SOUNDS);
        if (!strcmp(argv[1],"reuse_bounded") || !strcmp(argv[1],"reuse_cancel") ||
            !strcmp(argv[1],"reuse_disconnect") || !strcmp(argv[1],"reuse_short_read")) {
            bytes=9u*1024u*1024u+7;
            data=make_bytes(bytes,99);
            snprintf(path,sizeof(path),"%smcc_maps/solar_flare/bitmaps.map",H);
            CHECK(write_file(path,data,bytes));free(data);
            snprintf(path,sizeof(path),"%smcc_maps/solar_flare",H);
            identity_digest(path,W.digest,&identity_ok);CHECK(identity_ok);
        }
        snprintf(path,sizeof(path),"%scached",J);CHECK(io_make(path));
        for (i=1;i<=2;i++) {
            snprintf(source,sizeof(source),"%smcc_maps/solar_flare/%s",H,names[i]);
            data=read_file(source,&bytes);CHECK(data);
            snprintf(path,sizeof(path),"%scached/%s",J,names[i]);
            if (i==1 && !strcmp(argv[1],"reuse_bad_hash")) data[bytes-1]^=0x55;
            CHECK(write_file(path,data,bytes));free(data);
        }
        if (!strcmp(argv[1],"reuse_missing")) W.reuse_missing=1;
        if (!strcmp(argv[1],"reuse_absent")) W.reuse_mask=0;
        if (!strcmp(argv[1],"reuse_backpressure")) W.reject_have=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        if (!strcmp(argv[1],"reuse_bounded") || !strcmp(argv[1],"reuse_cancel") ||
            !strcmp(argv[1],"reuse_disconnect") || !strcmp(argv[1],"reuse_short_read")) {
            for (i=0;i<100 && !W.join.reuse_stream;i++) step();
            CHECK(W.join.reuse_stream && W.join.stage.written==4u*1024u*1024u);
            CHECK(!W.file_requests[1] && !W.have_requests);
            snprintf(path,sizeof(path),"%scached/bitmaps.map",J);
            if (!strcmp(argv[1],"reuse_cancel") || !strcmp(argv[1],"reuse_disconnect")) {
                if (!strcmp(argv[1],"reuse_cancel")) mcc_share_client_cancel(&W.join);
                else mcc_share_client_disconnect(&W.join);
                CHECK(!W.join.reuse_stream && !W.join.have_pending && staging_gone() && package_absent());
                /* On Windows this also proves the source handle was closed. */
                CHECK(io_delete(path));
                return 0;
            }
            if (!strcmp(argv[1],"reuse_short_read")) CHECK(write_file(path,(unsigned char const *)"x",1));
            else while (W.join.reuse_stream) {
                uint32_t before=W.join.stage.done;
                W.clock+=10000;step();frames++;
                CHECK(!W.finished && W.join.stage.done-before<=4u*1024u*1024u);
                CHECK(frames<10);
            }
        }
        if (!strcmp(argv[1],"reuse_backpressure")) {
            for (i=0;i<100 && !W.join.have_pending;i++) step();
            CHECK(W.join.have_pending && !W.join.reuse_stream && W.join.stage.current==2);
            for (i=0;i<10;i++) step();
            CHECK(!W.have_requests && !W.file_requests[1] && !W.file_requests[2]);
            CHECK(W.host.transfers[0].request_file==1 && !W.host.transfers[0].queued);
            W.reject_have=0;
        }
        CHECK(run_until_finished(2000));
        CHECK(!W.join.reuse_stream && !W.join.have_pending && staging_gone());
        if (!strcmp(argv[1],"reuse_bad_hash") || !strcmp(argv[1],"reuse_missing") ||
            !strcmp(argv[1],"reuse_short_read")) {
            CHECK(!W.finished_ok && strstr(W.finished_text,"previously downloaded resource"));
            CHECK(package_absent() && !W.have_requests && !W.file_requests[1]);
        } else {
            CHECK(W.finished_ok);
            if (!strcmp(argv[1],"reuse_absent")) CHECK(W.have_requests==0 && W.file_requests[1] && W.file_requests[2]);
            else {
                CHECK(W.have_requests==2 && W.reuse_candidates==2);
                CHECK(native_cached_reads==2); /* local reads must use the native-path bridge too */
                CHECK(!W.file_requests[1] && !W.file_requests[2]);
                CHECK(W.delivered_bytes==sizes[0]+sizes[3]);
                for (i=1;i<=2;i++) {
                    snprintf(source,sizeof(source),"%smcc_maps/solar_flare/%s",H,names[i]);
                    snprintf(path,sizeof(path),"%smcc_maps/solar_flare/%s",J,names[i]);
                    CHECK(same_file(source,path));
                }
            }
        }
        return 0;
    }

    if (!strcmp(argv[1],"hold_reuse") || !strcmp(argv[1],"hold_limit") || !strcmp(argv[1],"hold_declined")) {
        char path[400],source[400];
        unsigned char *data;
        unsigned bytes;
        snprintf(path,sizeof(path),"%scached",J);CHECK(io_make(path));
        for (i=1;i<=2;i++) {
            snprintf(source,sizeof(source),"%smcc_maps/solar_flare/%s",H,names[i]);
            data=read_file(source,&bytes);CHECK(data);
            snprintf(path,sizeof(path),"%scached/%s",J,names[i]);
            CHECK(write_file(path,data,bytes));free(data);
        }
        W.join.env.reuse_file=env_reuse_late;
        W.join.env.reuse_wait=env_wait;
        W.reuse_mask=(1u<<MCC_SHARE_FILE_BITMAPS)|(1u<<MCC_SHARE_FILE_SOUNDS);
        W.wait_answer=strcmp(argv[1],"hold_declined")!=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<400 && !W.wait_calls;i++) step();
        CHECK(W.wait_calls && W.join.stage.current==1);
        if (!strcmp(argv[1],"hold_declined")) {
            /* the host asked at once */
            for (i=0;i<5;i++) step();
            CHECK(W.file_requests[1]);
            CHECK(run_until_finished(2000) && W.finished_ok && !W.have_requests);
            return 0;
        }
        /* held: nothing asked of the host for it, the host hearing from us */
        /* (past the host's 30 s idle limit: the hold's heartbeats keep it) */
        for (i=0;i<40;i++) { W.clock+=980; step(); }
        CHECK(!W.file_requests[1] && !W.finished && W.host.transfers[0].used);
        if (!strcmp(argv[1],"hold_reuse")) {
            W.reuse_ready=1;
            CHECK(run_until_finished(2000) && W.finished_ok);
            CHECK(W.have_requests==2 && !W.file_requests[1] && !W.file_requests[2]);
            for (i=0;i<5;i++) step();
            CHECK(W.host_completions==1);
            return 0;
        }
        /* never in: each file asked of the host once its hold reaches the limit,
           the host's transfer kept the while */
        for (i=0;i<400 && !W.finished;i++) { W.clock+=980; step(); }
        CHECK(W.finished && W.finished_ok && W.file_requests[1] && W.file_requests[2] && !W.have_requests);
        for (i=0;i<5;i++) step();
        CHECK(W.host_completions==1);
        return 0;
    }

    if (!strcmp(argv[1],"hashed_elsewhere") || !strcmp(argv[1],"hashed_elsewhere_fails")) {
        W.host.env.hash_begin=env_hash_begin;
        W.host.env.hash_poll=env_hash_poll;
        W.host.env.hash_cancel=env_hash_cancel;
        W.hash_fail=!strcmp(argv[1],"hashed_elsewhere_fails");
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000));
        if (W.hash_fail) {
            CHECK(!W.finished_ok && W.hash_jobs==1);
            return 0;
        }
        /* each file hashed elsewhere, in turn, and the package the host's */
        CHECK(W.finished_ok && W.hash_jobs==4 && !W.hash_cancels);
        for (i=0;i<4;i++) CHECK(W.hash_polls[i]==3);
        for (i=0;i<4;i++) {
            char a[400],b[400];
            snprintf(a,sizeof(a),"%smcc_maps/solar_flare/%s",H,names[i]);
            snprintf(b,sizeof(b),"%smcc_maps/solar_flare/%s",J,names[i]);
            CHECK(same_file(a,b));
        }
        return 0;
    }

    CASE("window") {
        char path[400];
        unsigned char *data;
        unsigned bytes=1u<<20;
        int max_outstanding=0,max_queued=0,identity_ok;
        data=make_bytes(bytes,7);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/bitmaps.map",H);
        CHECK(write_file(path,data,bytes));free(data);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare",H);
        identity_digest(path,W.digest,&identity_ok);CHECK(identity_ok);
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<5000 && !W.finished;i++) {
            step();
            if (W.join.outstanding>max_outstanding) max_outstanding=W.join.outstanding;
            if (W.host.transfers[0].queued>max_queued) max_queued=W.host.transfers[0].queued;
        }
        /* every host queues 32 a transfer: as many in flight, never more */
        CHECK(W.finished && W.finished_ok);
        CHECK(max_outstanding==MCC_SHARE_CLIENT_WINDOW && max_queued<=MCC_SHARE_WINDOW*2);
        return 0;
    }

    CASE("have_fields") {
        struct mcc_share_request request,bad;
        struct mcc_share_host_transfer *transfer=&W.host.transfers[0];
        uint32_t offset,advanced;
        int32_t budget;
        memset(&request,0,sizeof(request));request.op=MCC_SHARE_OP_MANIFEST;strcpy(request.level,LEVEL);
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));mcc_share_update(&W.host);
        request.session=transfer->session;request.op=MCC_SHARE_OP_HAVE;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));CHECK(transfer->request_file==0);
        request.op=MCC_SHARE_OP_CHUNK;
        for (offset=0;offset<sizes[0];offset+=request.length) {
            request.offset=offset;request.length=sizes[0]-offset;
            if (request.length>MCC_SHARE_CHUNK_BYTES) request.length=MCC_SHARE_CHUNK_BYTES;
            mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        }
        mcc_share_update(&W.host);CHECK(transfer->request_file==1 && !transfer->queued);
        request.op=MCC_SHARE_OP_HAVE;request.file=1;request.length=request.offset=0;
        for (i=0;i<8;i++) {
            bad=request;
            switch (i) {
            case 0: bad.file=2;break;
            case 1: bad.file=255;break;
            case 2: bad.session++;break;
            case 3: bad.flags=1;break;
            case 4: bad.offset=1;break;
            case 5: bad.length=1;break;
            case 6: bad.reserved=1;break;
            case 7: strcpy(bad.level,"mcc_maps\\other\\m0");break;
            }
            mcc_share_host_request(&W.host,MACHINE,&bad,sizeof(bad));
            CHECK(transfer->request_file==1 && !transfer->reused_files);
        }
        bad=request;bad.op=MCC_SHARE_OP_CHUNK;bad.length=1;
        mcc_share_host_request(&W.host,MACHINE,&bad,sizeof(bad));
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));CHECK(transfer->request_file==1);
        for (offset=1;offset<sizes[1];offset+=bad.length) {
            bad.offset=offset;bad.length=sizes[1]-offset;
            if (bad.length>MCC_SHARE_CHUNK_BYTES) bad.length=MCC_SHARE_CHUNK_BYTES;
            mcc_share_host_request(&W.host,MACHINE,&bad,sizeof(bad));
        }
        mcc_share_update(&W.host);CHECK(transfer->request_file==2 && !transfer->queued);
        request.file=2;budget=W.host.budget;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        advanced=transfer->advanced_at;
        CHECK(transfer->request_file==3 && transfer->reused_files==1 && W.host.budget==budget);
        W.clock+=500;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(transfer->request_file==3 && transfer->reused_files==1 && transfer->advanced_at==advanced);
        request.file=3;mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(transfer->request_file==3); /* credits are never locally asserted */
        return 0;
    }

    CASE("backpressure") {
        W.join.env.offered=env_offered;
        W.reject_requests=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<5;i++) step();
        CHECK(W.host_count==0 && W.join.state==MCC_SHARE_CLIENT_ASKING);
        W.reject_requests=0;W.reject_responses=1;
        for (i=0;i<5;i++) step();
        CHECK(W.responses==0 && W.join.state==MCC_SHARE_CLIENT_ASKING);
        W.reject_responses=0;
        for (i=0;i<5;i++) step();
        CHECK(W.join.state==MCC_SHARE_CLIENT_WAITING);
        W.reject_requests=1;
        CHECK(mcc_share_client_accept(&W.join));
        for (i=0;i<5;i++) step();
        CHECK(W.join.outstanding==0 && W.delivered_bytes==0);
        W.reject_requests=0;W.reject_responses=1;
        for (i=0;i<5;i++) step();
        CHECK(W.delivered_bytes==0 && W.join.outstanding>0);
        W.reject_responses=0;
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    CASE("response_fields") {
        struct { struct mcc_share_response head; struct mcc_share_entry file; } packet;
        struct mcc_share_response good;
        unsigned j;
        W.join.env.offered=env_offered;
        memset(&packet,0,sizeof(packet));
        packet.head.op=MCC_SHARE_OP_MANIFEST;packet.head.session=77;
        packet.head.count=1;packet.head.length=1;strcpy(packet.head.level,LEVEL);
        entry(&packet.file,MCC_SHARE_FILE_MAP,"m0.map",10);
        good=packet.head;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (j=0;j<5;j++) {
            packet.head=good;
            switch (j) {
            case 0: packet.head.session=0; break;
            case 1: packet.head.file=1; break;
            case 2: packet.head.result=255; break;
            case 3: memset(packet.head.credit,'x',sizeof(packet.head.credit)); break;
            case 4: packet.head.credit[0]=1; break;
            }
            mcc_share_client_response(&W.join,&packet,sizeof(packet));
            CHECK(W.join.state==MCC_SHARE_CLIENT_ASKING && !W.finished && !W.offers);
        }
        packet.head=good;packet.file.reserved[1]=1;
        mcc_share_client_response(&W.join,&packet,sizeof(packet));
        CHECK(W.finished==1 && !W.finished_ok && !W.offers && staging_gone());
        W.finished=0;packet.file.reserved[1]=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        packet.head.count=2;
        mcc_share_client_response(&W.join,&packet,sizeof(packet));
        CHECK(W.join.offer_received==1 && !W.offers);
        /* A duplicate page must not overwrite accepted entries or change the snapshot. */
        mcc_share_client_response(&W.join,&packet,sizeof(packet));
        CHECK(W.finished==1 && !W.finished_ok && !W.offers && staging_gone());
        return 0;
    }

    CASE("request_fields") {
        struct mcc_share_request r;
        struct mcc_share_host_transfer *transfer;
        memset(&r,0,sizeof(r));r.op=MCC_SHARE_OP_MANIFEST;strcpy(r.level,LEVEL);
        r.flags=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.flags=0;
        r.file=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.file=0;
        r.offset=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.offset=0;
        r.length=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.length=0;
        r.session=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.session=0;
        CHECK(!mcc_share_host_active(&W.host,MACHINE) && !W.responses);
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));mcc_share_update(&W.host);
        transfer=&W.host.transfers[0];CHECK(transfer->session);
        r.op=MCC_SHARE_OP_DONE;r.session=transfer->session+1;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(transfer->used);
        r.session=transfer->session;r.flags=2;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(transfer->used);
        r.flags=1; /* a client cannot claim installation before receiving files */
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(transfer->used);
        r.flags=0;r.length=1;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(transfer->used);
        r.length=0;r.op=MCC_SHARE_OP_CHUNK;r.length=100;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(transfer->queued==1);
        for (i=0;i<100;i++) mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(transfer->queued==1 && transfer->request_offset==100);
        r.op=MCC_SHARE_OP_DONE;r.length=0;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));CHECK(!transfer->used);
        CHECK(!W.host_completions);
        return 0;
    }

    CASE("cancel") {
        W.limit=3000;                                                  /* a slow host: the download takes seconds */
        char path[400];
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<4;i++) step();
        CHECK(mcc_share_client_busy(&W.join));
        snprintf(path,sizeof(path),"%smcc_staging/solar_flare",J);
        CHECK(io_exists(path));
        mcc_share_client_cancel(&W.join);
        CHECK(!mcc_share_client_busy(&W.join));
        CHECK(staging_gone() && package_absent());
        CHECK(W.finished==0);
        /* late messages from the host are ignored */
        for (i=0;i<10;i++) step();
        CHECK(staging_gone() && package_absent() && W.finished==0);
        /* Cancellation notifies the host immediately; no timeout is needed. */
        CHECK(!mcc_share_host_active(&W.host,MACHINE));
        /* and the joiner can ask again */
        W.clock+=1500;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    CASE("bad_hash") {
        W.limit=3000;                                                  /* a slow host: the download takes seconds */
        char path[400];
        unsigned char *data;
        unsigned size;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<3;i++) step();                                       /* the manifest is out */
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/bitmaps.map",H);
        data=read_file(path,&size);
        data[size-1]^=0x55;
        CHECK(write_file(path,data,size));
        free(data);
        CHECK(run_until_finished(2000));
        CHECK(!W.finished_ok);
        CHECK(strstr(W.finished_text,"did not match its SHA-256") && strstr(W.finished_text,"Nothing was installed"));
        CHECK(W.identity_calls==0);                                     /* stopped at the file, before the identity check */
        CHECK(staging_gone() && package_absent());
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare",J);
        CHECK(!io_exists(path));
        return 0;
    }

    CASE("changed_size") {
        W.limit=3000;                                                  /* a slow host: the download takes seconds */
        char path[400];
        unsigned char *data;
        unsigned size;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<3;i++) step();
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/sounds.map",H);
        data=read_file(path,&size);
        CHECK(write_file(path,data,size-1));
        free(data);
        CHECK(run_until_finished(2000));
        CHECK(!W.finished_ok && strstr(W.finished_text,"changed"));
        CHECK(staging_gone() && package_absent());
        return 0;
    }

    CASE("identity") {
        char path[400];
        W.identity_ok=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000));
        CHECK(!W.finished_ok && strstr(W.finished_text,"identity") && strstr(W.finished_text,"Nothing was installed"));
        CHECK(W.identity_calls==1);
        CHECK(staging_gone() && package_absent());
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare",J);
        CHECK(!io_exists(path));
        /* A local read failure survives stage cancellation and reaches the UI. */
        W.join.env.identity_error=env_identity_error;W.finished=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000));
        CHECK(!W.finished_ok && strstr(W.finished_text,"Cannot read") && strstr(W.finished_text,"sounds.map"));
        CHECK(strstr(W.finished_text,"Nothing was installed") && staging_gone() && package_absent());
        W.join.env.identity_error=NULL;
        /* a different fingerprint in the game record */
        W.identity_ok=1;W.finished=0;
        W.digest[5]^=1;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000));
        CHECK(!W.finished_ok);
        CHECK(staging_gone() && package_absent());
        return 0;
    }

    CASE("disk_space") {
        /* room for the files and some to spare, or the download does not begin */
        free_space=10000;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(200));
        CHECK(!W.finished_ok && strstr(W.finished_text,"not enough free disk space"));
        CHECK(staging_gone() && package_absent());
        CHECK(!mcc_share_client_busy(&W.join));
        free_space=1ull<<40;W.finished=0;W.clock+=2000;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(2000) && W.finished_ok);
        return 0;
    }

    CASE("taken") {
        W.taken=1;
        CHECK(!mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(W.host_count==0 && !mcc_share_client_busy(&W.join));
        return 0;
    }

    CASE("exists") {
        char path[400];
        unsigned char *old=make_bytes(77,9);
        unsigned char *data;
        unsigned size;
        snprintf(path,sizeof(path),"%smcc_maps",J);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare",J);CHECK(mkdir(path,0755)==0);
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/m0.map",J);CHECK(write_file(path,old,77));
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        CHECK(run_until_finished(200));
        CHECK(!W.finished_ok && strstr(W.finished_text,"already exists"));
        data=read_file(path,&size);
        CHECK(data && size==77 && !memcmp(data,old,77));
        free(data);free(old);
        CHECK(staging_gone());
        CHECK(W.identity_calls==0);
        return 0;
    }

    CASE("refusals") {
        static enum mcc_share_result const results[]={MCC_SHARE_RESULT_OFF,
            MCC_SHARE_RESULT_NOT_SHAREABLE,MCC_SHARE_RESULT_NO_CREDIT,MCC_SHARE_RESULT_UNAVAILABLE};
        static char const *const words[]={"does not send","cannot send","maker credit","could not read"};
        for (i=0;i<sizeof(results)/sizeof(results[0]);i++) {
            W.finished=0;
            W.refusal=results[i];
            W.clock+=2000;
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            CHECK(run_until_finished(200));
            CHECK(!W.finished_ok);
            CHECK(strstr(W.finished_text,words[i]));
            CHECK(strstr(W.finished_text,"m0"));
            CHECK(staging_gone() && package_absent());
            CHECK(!mcc_share_client_busy(&W.join));
        }
        return 0;
    }

    CASE("busy") {
        struct mcc_share_request r;
        long machine;
        memset(&r,0,sizeof(r));
        r.op=MCC_SHARE_OP_MANIFEST;r.length=0;
        snprintf(r.level,sizeof(r.level),"%s",LEVEL);
        for (machine=0;machine<MCC_SHARE_HOST_TRANSFERS;machine++) {
            mcc_share_host_request(&W.host,machine,&r,sizeof(r));
            CHECK(mcc_share_host_active(&W.host,machine));
        }
        W.responses=0;
        mcc_share_host_request(&W.host,MCC_SHARE_HOST_TRANSFERS,&r,sizeof(r));
        CHECK(W.responses==1 && W.last_machine==MCC_SHARE_HOST_TRANSFERS);
        CHECK(!mcc_share_host_active(&W.host,MCC_SHARE_HOST_TRANSFERS));
        mcc_share_host_forget(&W.host,0);
        W.clock+=1500;
        mcc_share_host_request(&W.host,MCC_SHARE_HOST_TRANSFERS,&r,sizeof(r));
        CHECK(mcc_share_host_active(&W.host,MCC_SHARE_HOST_TRANSFERS));
        return 0;
    }

    CASE("host_requests") {
        struct mcc_share_request r;
        unsigned char big[sizeof(r)+4];
        uint32_t session;
        int sent;
        memset(&r,0,sizeof(r));
        r.op=MCC_SHARE_OP_MANIFEST;
        snprintf(r.level,sizeof(r.level),"%s",LEVEL);
        /* wrong sizes, no terminator, reserved bytes, machines out of range, unknown ops: nothing answers */
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r)-1);
        memset(big,0,sizeof(big));memcpy(big,&r,sizeof(r));
        mcc_share_host_request(&W.host,MACHINE,big,sizeof(r)+1);
        mcc_share_host_request(&W.host,-1,&r,sizeof(r));
        mcc_share_host_request(&W.host,MCC_SHARE_HOST_MACHINES,&r,sizeof(r));
        mcc_share_host_request(&W.host,MACHINE,NULL,0);
        r.reserved=1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.reserved=0;
        memset(r.level,'a',sizeof(r.level));mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        snprintf(r.level,sizeof(r.level),"%s",LEVEL);
        r.op=9;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.op=0;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.op=MCC_SHARE_OP_CHUNK;r.length=100;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r)); /* no transfer */
        CHECK(W.responses==0 && !mcc_share_host_active(&W.host,MACHINE));
        W.join_count=0;
        /* another level than the game's is refused */
        r.op=MCC_SHARE_OP_MANIFEST;r.length=0;
        snprintf(r.level,sizeof(r.level),"mcc_maps\\solar_flare\\m1");
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(W.responses==1 && !mcc_share_host_active(&W.host,MACHINE));
        W.responses=0;W.join_count=0;
        /* a good manifest request, repeated at once, is taken once */
        W.clock+=5000;
        snprintf(r.level,sizeof(r.level),"%s",LEVEL);
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(mcc_share_host_active(&W.host,MACHINE));
        for (i=0;i<5;i++) {mcc_share_update(&W.host);W.clock+=20;}
        CHECK(W.responses==1);
        {
            struct mcc_share_response head;
            memcpy(&head,W.to_join[0].data,sizeof(head));
            CHECK(head.op==MCC_SHARE_OP_MANIFEST && head.result==MCC_SHARE_RESULT_OK && head.count==4);
            session=head.session;
            CHECK(session!=0);
        }
        /* the manifest repeated after a second is answered again, the same */
        W.clock+=1500;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(W.responses==2);
        /* bad chunk requests: wrong session answers "changed"; the rest "bad request" */
        W.responses=0;
        r.op=MCC_SHARE_OP_CHUNK;r.session=session;r.file=0;r.offset=0;r.length=100;
        r.session=session+1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.session=session;
        r.file=4;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));r.file=0;
        r.length=0;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.length=MCC_SHARE_CHUNK_BYTES+1;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.length=100;r.offset=sizes[0];mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.offset=sizes[0]-99;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.offset=0xffffffffu;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        r.offset=0;snprintf(r.level,sizeof(r.level),"mcc_maps\\solar_flare\\m1");
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(W.responses==8);
        /* queueing is bounded: only so many chunks wait; none is answered before the budget */
        W.responses=0;
        snprintf(r.level,sizeof(r.level),"%s",LEVEL);
        W.limit=1;
        for (i=0;i<100;i++) {r.offset=0;r.length=100;mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));}
        W.clock+=1000;
        sent=0;
        mcc_share_update(&W.host);
        sent=W.responses;
        CHECK(sent<=2);                                                 /* a budget of one byte */
        W.limit=1u<<20;
        for (i=0;i<10;i++) {W.clock+=100;mcc_share_update(&W.host);}
        CHECK(W.responses<=MCC_SHARE_WINDOW*2+2);                       /* the queue was capped */
        /* done frees only its matching session and level, with reserved fields clear */
        r.op=MCC_SHARE_OP_DONE;
        r.offset=0;r.length=0;r.file=0;
        mcc_share_host_request(&W.host,MACHINE,&r,sizeof(r));
        CHECK(!mcc_share_host_active(&W.host,MACHINE));
        return 0;
    }

    CASE("hostile_manifest") {
        struct mcc_share_entry e[9];
        char base[400];
        unsigned j;
        struct {char const *name;int kinds[4];char const *leaves[4];uint32_t sizes[4];unsigned count;} const bad[]={
            {"traversal",{0,1,2,3},{"../m0.map","bitmaps.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"separator",{0,1,2,3},{"m0.map","a\\bitmaps.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"no map",{1,2,3,3},{"bitmaps.map","sounds.map","credits.txt","readme.txt"},{10,10,10,10},4},
            {"two maps",{0,0,2,3},{"m0.map","m0.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"other map",{0,1,2,3},{"other.map","bitmaps.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"loc",{0,1,2,3},{"m0.map","loc.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"program",{0,1,2,3},{"m0.map","bitmaps.map","sounds.map","evil.dll"},{10,10,10,10},4},
            {"sidecar",{0,1,2,3},{"m0.map","bitmaps.map","sounds.map","workshop_id.txt"},{10,10,10,10},4},
            {"zero size",{0,1,2,3},{"m0.map","bitmaps.map","sounds.map","credits.txt"},{10,0,10,10},4},
            {"huge",{0,1,2,3},{"m0.map","bitmaps.map","sounds.map","credits.txt"},{0x7fffffffu,10,10,10},4},
            {"huge credit",{0,1,2,3},{"m0.map","bitmaps.map","sounds.map","credits.txt"},{10,10,10,0x20000},4},
            {"kind",{0,9,2,3},{"m0.map","bitmaps.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"kind mismatch",{0,2,1,3},{"m0.map","bitmaps.map","sounds.map","credits.txt"},{10,10,10,10},4},
            {"repeat",{0,3,3,3},{"m0.map","credits.txt","CREDITS.TXT","readme.txt"},{10,10,10,10},4}
        };
        (void)base;
        for (i=0;i<sizeof(bad)/sizeof(bad[0]);i++) {
            W.finished=0;
            W.clock+=2000;
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            for (j=0;j<bad[i].count;j++) entry(&e[j],bad[i].kinds[j],bad[i].leaves[j],bad[i].sizes[j]);
            install_hostile(e,bad[i].count,0);
            if (!(W.finished==1 && !W.finished_ok)) {fprintf(stderr,"case %s\n",bad[i].name);return 1;}
            CHECK(staging_gone() && package_absent());
            {
                char path[400];
                snprintf(path,sizeof(path),"%smcc_staging",J);
                CHECK(!io_exists(path));                                /* nothing made for a refused list */
            }
        }
        /* An excessive total count and a short page body are ignored. */
        W.finished=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        {
            struct mcc_share_response head;
            memset(&head,0,sizeof(head));head.op=MCC_SHARE_OP_MANIFEST;head.session=77;
            head.count=MCC_SHARE_STAGE_FILES+1;head.length=1;strcpy(head.level,LEVEL);
            mcc_share_client_response(&W.join,&head,sizeof(head));
        }
        CHECK(W.finished==0 && mcc_share_client_busy(&W.join));        /* ignored as malformed, still asking */
        entry(&e[0],0,"m0.map",10);
        install_hostile(e,1,1);
        CHECK(W.finished==0 && mcc_share_client_busy(&W.join));
        mcc_share_client_cancel(&W.join);
        return 0;
    }

    CASE("hashed_once") {
        struct mcc_share_request request;
        char path[400];
        unsigned char *data;
        unsigned frame,size;
        memset(&request,0,sizeof(request));request.op=MCC_SHARE_OP_MANIFEST;strcpy(request.level,LEVEL);
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        for (frame=0;frame<100 && !W.manifest_pages;frame++) { mcc_share_update(&W.host);W.clock+=20; }
        CHECK(W.manifest_pages==1 && native_reads==4);                 /* four files hashed */
        /* asked again (a new transfer for the machine, then another machine):
         * offered at once, the files only opened for their sizes */
        mcc_share_host_forget(&W.host,MACHINE);W.clock+=2000;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(W.manifest_pages==2 && native_reads==8);
        mcc_share_host_request(&W.host,MACHINE+1,&request,sizeof(request));
        CHECK(W.manifest_pages==3 && native_reads==12);
        /* a file of another size is hashed again */
        snprintf(path,sizeof(path),"%smcc_maps/solar_flare/sounds.map",H);
        data=read_file(path,&size);
        CHECK(write_file(path,data,size-1));
        free(data);
        mcc_share_host_forget(&W.host,MACHINE);W.clock+=2000;
        mcc_share_host_request(&W.host,MACHINE,&request,sizeof(request));
        CHECK(W.manifest_pages==3);
        for (frame=0;frame<100 && W.manifest_pages==3;frame++) { mcc_share_update(&W.host);W.clock+=20; }
        CHECK(W.manifest_pages==4);
        /* and none is kept once the host stops */
        mcc_share_host_stop(&W.host);
        CHECK(!W.host.hashed.valid);
        return 0;
    }

    CASE("hostile_chunk") {
        unsigned char buffer[MCC_SHARE_RESPONSE_MAXIMUM];
        struct mcc_share_response head;
        unsigned j;
        for (j=0;j<7;j++) {
            struct mcc_share_response good;
            W.finished=0;W.join_count=0;W.host_count=0;
            W.clock+=2000;
            mcc_share_host_forget(&W.host,MACHINE);
            W.host.hashed.valid=0;                                      /* hashed again: each round at the same step */
            CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
            for (i=0;i<3;i++) step();                                   /* manifest in, first chunks out */
            CHECK(mcc_share_client_busy(&W.join));
            memset(&head,0,sizeof(head));
            head.op=MCC_SHARE_OP_CHUNK;
            head.session=W.join.session;
            head.file=0;
            head.offset=0;
            head.length=100;
            snprintf(head.level,sizeof(head.level),"%s",LEVEL);
            good=head;
            switch (j) {
            case 0: head.session^=1; break;
            case 1: head.offset=4; break;
            case 2: head.file=1; break;
            case 3: head.length=0; break;
            case 4: head.length=MCC_SHARE_CHUNK_BYTES+1; break;
            case 5: head.length=100; break;
            case 6: head.length=100; break;
            }
            (void)good;
            memcpy(buffer,&head,sizeof(head));
            memset(buffer+sizeof(head),1,sizeof(buffer)-sizeof(head));
            /* body length: j==5 sends fewer bytes than the head says; j==6 more */
            mcc_share_client_response(&W.join,buffer,(unsigned)sizeof(head)+(j==5 ? 50 : j==6 ? 150 : head.length>sizeof(buffer)-sizeof(head) ? 100 : head.length));
            CHECK(W.finished==1 && !W.finished_ok);
            CHECK(staging_gone() && package_absent());
        }
        /* a response when nothing was asked, and one for another level, change nothing */
        W.finished=0;
        memset(&head,0,sizeof(head));
        head.op=MCC_SHARE_OP_MANIFEST;
        snprintf(head.level,sizeof(head.level),"%s",LEVEL);
        mcc_share_client_response(&W.join,&head,sizeof(head));
        CHECK(W.finished==0);
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        snprintf(head.level,sizeof(head.level),"mcc_maps\\solar_flare\\m1");
        head.result=MCC_SHARE_RESULT_OFF;
        mcc_share_client_response(&W.join,&head,sizeof(head));
        CHECK(W.finished==0 && mcc_share_client_busy(&W.join));
        mcc_share_client_cancel(&W.join);
        return 0;
    }

    CASE("names") {
        static char const *const bad[]={"mcc_maps\\loose","mcc_maps\\_shared\\multiplayer",
            "mcc_maps\\solar_flare\\CON","mcc_maps\\a\\..\\b","maps\\solar_flare\\m0",
            "custom_maps\\x","","mcc_maps\\solar_flare\\bitmaps","mcc_maps\\a:b\\c","mcc_maps\\solar_flare\\"};
        for (i=0;i<sizeof(bad)/sizeof(bad[0]);i++) {
            CHECK(!mcc_share_client_begin(&W.join,bad[i],W.digest));
            CHECK(W.host_count==0 && !mcc_share_client_busy(&W.join));
        }
        CHECK(!mcc_share_client_begin(&W.join,NULL,W.digest));
        return 0;
    }

    CASE("timeouts") {
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        W.host_count=0;                                                 /* the host never hears */
        W.clock+=31000;
        mcc_share_update(&W.join);
        CHECK(W.finished==1 && !W.finished_ok && strstr(W.finished_text,"did not answer"));
        W.finished=0;
        CHECK(mcc_share_client_begin(&W.join,LEVEL,W.digest));
        for (i=0;i<3;i++) step();                                       /* in transfer; then the host goes quiet */
        W.host_count=0;W.join_count=0;
        W.clock+=61000;
        mcc_share_update(&W.join);
        CHECK(W.finished==1 && !W.finished_ok && strstr(W.finished_text,"nothing for a minute"));
        CHECK(staging_gone() && package_absent());
        return 0;
    }

    return 2;
}

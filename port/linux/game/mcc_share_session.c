#include "mcc_share_session.h"
#include "mcc_share_rules.h"
#include "mcc_package.h"
#include <stdio.h>
#include <string.h>

#define MCC_SHARE_ASK_TIMEOUT 30000u
#define MCC_SHARE_STALL_TIMEOUT 60000u
#define MCC_SHARE_HOST_IDLE_TIMEOUT 30000u
#define MCC_SHARE_ACTIVE_HEARD 20000u
#define MCC_SHARE_ACTIVE_ADVANCED 80000u
#define MCC_SHARE_MANIFEST_INTERVAL 1000u
#define MCC_SHARE_HASH_SLICE 0x400000u
#define MCC_SHARE_OFFER_TIMEOUT 120000u
#define MCC_SHARE_OFFER_HEARTBEAT 10000u
#define MCC_SHARE_REUSE_SLICE 0x400000u
/* A local copy's time an update: it hashes as it copies, on the game's thread. */
#define MCC_SHARE_REUSE_MILLISECONDS 12u
/* how long the last package's digests stand in for hashing it again */
#define MCC_SHARE_HASHED_LIFETIME 600000u

static int share_elapsed(uint32_t now, uint32_t since, uint32_t span)
{
    return (uint32_t)(now-since)>=span;
}

static uint32_t share_now(struct mcc_share const *share)
{
    return share->env.now(share->env.user);
}

/* Host sources and managed reuse candidates are native paths, unlike the
 * staging paths interpreted by the game's virtual filesystem. */
static FILE *share_open_native(struct mcc_share *share, char const *path)
{
    return share->env.io.open_native_read ? share->env.io.open_native_read(path) : fopen(path,"rb");
}

static int share_same(char const *a, char const *b)
{
    for (;*a && *b;a++,b++) {
        unsigned char x=(unsigned char)*a,y=(unsigned char)*b;
        if (x>='A' && x<='Z') x=(unsigned char)(x+'a'-'A');
        if (y>='A' && y<='Z') y=(unsigned char)(y+'a'-'A');
        if (x!=y) return 0;
    }
    return !*a && !*b;
}

/* The names a package may be shared under, by the rules module's own name checks.
 * The facts passed besides the name are fixed here: only the name is judged, and
 * nothing a remote host says establishes content or origin. */
static int share_level_split(char const *level, char folder[64], char leaf[MCC_SHARE_LEAF_SIZE])
{
    struct mcc_share_package package;
    char canonical[64],*separator;
    size_t length=strlen(level);
    memset(&package,0,sizeof(package));
    package.name=level;
    package.name_length=length;
    package.sidecar=MCC_SHARE_SIDECAR_ABSENT;
    package.content=MCC_SHARE_CONTENT_CUSTOM;
    package.content_known=1;
    package.host_opt_in=1;
    if (mcc_share_decide(&package)!=MCC_SHARE_ELIGIBLE || !mcc_package_name(level,canonical)) return 0;
    separator=strchr(canonical+9,'\\');
    if (!separator || strlen(separator+1)>=MCC_SHARE_LEAF_SIZE-4) return 0;
    *separator=0;
    strcpy(folder,canonical+9);
    strcpy(leaf,separator+1);
    return 1;
}

static int share_level_text(char const *text)
{
    return memchr(text,0,MCC_SHARE_LEVEL_SIZE)!=NULL;
}

static int share_credit_valid(char const *credit)
{
    unsigned i;
    for (i=0;i<MCC_SHARE_CREDIT_SIZE;i++) {
        if (!credit[i]) return 1;
        if ((unsigned char)credit[i]<32 || (unsigned char)credit[i]>126) return 0;
    }
    return 0;
}

static int share_zero(void const *data, unsigned bytes)
{
    uint8_t const *p=data;
    while (bytes--) if (*p++) return 0;
    return 1;
}

enum mcc_share_result mcc_share_admit(char const *level, int opt_in, enum mcc_share_sidecar sidecar,
    unsigned char const digest[32], int digest_known, char const *list, unsigned long list_length,
    char credit[MCC_SHARE_CREDIT_SIZE])
{
    struct mcc_share_package package;
    credit[0]=0;
    if (!opt_in) return MCC_SHARE_RESULT_OFF;
    /* Older optional lists supply attribution only. A validated package no
     * longer needs manual fingerprint approval to be offered. */
    if (digest_known) mcc_share_list_find(list,list_length,digest,credit);
    memset(&package,0,sizeof(package));
    package.name=level;
    package.name_length=level ? strlen(level) : 0;
    package.sidecar=sidecar;
    package.content=MCC_SHARE_CONTENT_CUSTOM;
    package.content_known=digest_known;
    package.host_opt_in=1;
    switch (mcc_share_decide(&package)) {
    case MCC_SHARE_ELIGIBLE:
        return MCC_SHARE_RESULT_OK;
    case MCC_SHARE_NOT_OPTED_IN: return MCC_SHARE_RESULT_OFF;
    default: break;
    }
    credit[0]=0;
    return MCC_SHARE_RESULT_NOT_SHAREABLE;
}

void mcc_share_init(struct mcc_share *share, struct mcc_share_env const *env)
{
    memset(share,0,sizeof(*share));
    share->env=*env;
    share->next_session=0x4d435331u;
    share->budget_at=share_now(share);
}

void mcc_share_refusal_text(enum mcc_share_result result, char const *level, char *text, unsigned size)
{
    char const *leaf=strrchr(level,'\\');
    leaf=leaf ? leaf+1 : level;
    switch (result) {
    case MCC_SHARE_RESULT_OFF:
        snprintf(text,size,"The host does not send MCC maps. Get the package for %s from its maker, "
            "or ask the host to turn sharing on.",leaf);
        break;
    case MCC_SHARE_RESULT_NO_CREDIT:
        snprintf(text,size,"The host has no maker credit for %s, so it does not send it. "
            "Get the package from its maker.",leaf);
        break;
    case MCC_SHARE_RESULT_BUSY:
        snprintf(text,size,"The host is sending maps to other players. Try joining again in a moment.");
        break;
    case MCC_SHARE_RESULT_UNAVAILABLE:
        snprintf(text,size,"The host could not read the files of %s. Get the package from its maker.",leaf);
        break;
    case MCC_SHARE_RESULT_CHANGED:
        snprintf(text,size,"The host's files for %s changed during the download. Join again.",leaf);
        break;
    default:
        snprintf(text,size,"The host cannot send %s: it is not a package the host can share. "
            "Get the package from its maker.",leaf);
        break;
    }
}

/* ---------- the host */

static int host_reply(struct mcc_share *share, long machine, uint8_t op, enum mcc_share_result result,
    char const *level, uint32_t session, uint8_t file, uint8_t count, uint32_t offset, uint32_t length,
    char const *credit, void const *body, unsigned body_bytes)
{
    static uint8_t buffer[MCC_SHARE_RESPONSE_MAXIMUM];
    struct mcc_share_response head;
    memset(&head,0,sizeof(head));
    head.op=op;
    head.result=(uint8_t)result;
    head.file=file;
    head.count=count;
    head.session=session;
    head.offset=offset;
    head.length=length;
    snprintf(head.level,sizeof(head.level),"%s",level);
    if (credit) snprintf(head.credit,sizeof(head.credit),"%s",credit);
    if (body_bytes>sizeof(buffer)-sizeof(head)) return 0;
    memcpy(buffer,&head,sizeof(head));
    if (body_bytes) memcpy(buffer+sizeof(head),body,body_bytes);
    return share->env.send_response(share->env.user,machine,buffer,(unsigned)sizeof(head)+body_bytes);
}

static struct mcc_share_host_transfer *host_find(struct mcc_share *share, long machine)
{
    int i;
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS;i++)
        if (share->transfers[i].used && share->transfers[i].machine==machine) return &share->transfers[i];
    return NULL;
}

static void host_free(struct mcc_share_host_transfer *transfer)
{
    if (transfer->hash_job_active && transfer->hash_job_cancel)
        transfer->hash_job_cancel(transfer->hash_job_user,transfer->hash_job);
    if (transfer->hash_stream) fclose(transfer->hash_stream);
    if (transfer->serve_stream) fclose(transfer->serve_stream);
    memset(transfer,0,sizeof(*transfer));
}

static int host_expired(struct mcc_share_host_transfer const *transfer, uint32_t now)
{
    if (transfer->hashing) return share_elapsed(now,transfer->heard_at,MCC_SHARE_OFFER_TIMEOUT);
    return share_elapsed(now,transfer->heard_at,MCC_SHARE_HOST_IDLE_TIMEOUT) ||
        (transfer->sent_bytes || transfer->reused_files ? share_elapsed(now,transfer->advanced_at,MCC_SHARE_ACTIVE_ADVANCED) :
         transfer->offer_complete && share_elapsed(now,transfer->offered_at,MCC_SHARE_OFFER_TIMEOUT));
}

void mcc_share_host_forget(struct mcc_share *share, long machine)
{
    struct mcc_share_host_transfer *transfer=host_find(share,machine);
    if (transfer) host_free(transfer);
    if (machine>=0 && machine<MCC_SHARE_HOST_MACHINES) share->manifest_seen[machine]=0;
}

void mcc_share_host_stop(struct mcc_share *share)
{
    int i;
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS;i++) if (share->transfers[i].used) host_free(&share->transfers[i]);
    memset(share->manifest_seen,0,sizeof(share->manifest_seen));
    share->hashed.valid=0;
    share->budget=0;
    share->budget_fraction=0;
    share->budget_at=share_now(share);
}

static void host_send_manifest(struct mcc_share *share, struct mcc_share_host_transfer *transfer)
{
    struct mcc_share_entry entries[MCC_SHARE_MANIFEST_PAGE];
    unsigned i, count, first=transfer->manifest_cursor;
    if (!transfer->manifest_pending || first>=(unsigned)transfer->package.count || share->budget<=0) return;
    count=(unsigned)transfer->package.count-first;
    if (count>MCC_SHARE_MANIFEST_PAGE) count=MCC_SHARE_MANIFEST_PAGE;
    memset(entries,0,sizeof(entries));
    for (i=0;i<count;i++) {
        entries[i].kind=transfer->files[first+i].kind;
        entries[i].size=transfer->files[first+i].size;
        memcpy(entries[i].leaf,transfer->files[first+i].leaf,sizeof(entries[i].leaf));
        memcpy(entries[i].sha256,transfer->files[first+i].sha256,32);
    }
    if (!host_reply(share,transfer->machine,MCC_SHARE_OP_MANIFEST,MCC_SHARE_RESULT_OK,transfer->level,
        transfer->session,0,(uint8_t)transfer->package.count,first,count,transfer->credit,
        entries,(unsigned)(count*sizeof(entries[0])))) return;
    share->budget-=(int32_t)(sizeof(struct mcc_share_response)+count*sizeof(entries[0]));
    transfer->manifest_cursor+=count;
    transfer->manifest_pending=transfer->manifest_cursor<(unsigned)transfer->package.count;
    if (!transfer->manifest_pending && !transfer->offer_complete) {
        transfer->offer_complete=1;
        transfer->offered_at=transfer->heard_at=share_now(share);
    }
}

static void host_offer(struct mcc_share *share, struct mcc_share_host_transfer *transfer, int hashed);

static void hash_text(struct mcc_sha256 *hash, char const *text, unsigned size)
{
    char const *end=memchr(text,0,size);
    mcc_sha256_add(hash,text,end ? (int)(end-text)+1 : (int)size);
}

/* What the package is made of: its level and its files' kinds, leaves and paths. */
static void host_package_key(struct mcc_share_host_transfer const *transfer, uint8_t key[32])
{
    struct mcc_sha256 hash;
    int i;
    mcc_sha256_begin(&hash);
    hash_text(&hash,transfer->level,sizeof(transfer->level));
    for (i=0;i<transfer->package.count;i++) {
        mcc_sha256_add(&hash,&transfer->package.files[i].kind,1);
        hash_text(&hash,transfer->package.files[i].leaf,sizeof(transfer->package.files[i].leaf));
        hash_text(&hash,transfer->package.files[i].path,sizeof(transfer->package.files[i].path));
    }
    mcc_sha256_end(&hash,key);
}

/* Take the digests of the package hashed last if it is the same package and its
 * files are still their sizes. (A file changed in place to the same size is
 * caught by the joiner, which checks every file's digest as it arrives.) */
static int host_hashed(struct mcc_share *share, struct mcc_share_host_transfer *transfer)
{
    uint8_t key[32];
    int i;
    if (!share->hashed.valid || share_elapsed(share_now(share),share->hashed.at,MCC_SHARE_HASHED_LIFETIME) ||
        share->hashed.count!=transfer->package.count) return 0;
    host_package_key(transfer,key);
    if (memcmp(key,share->hashed.key,32)) return 0;
    for (i=0;i<transfer->package.count;i++) {
        FILE *stream=share_open_native(share,transfer->package.files[i].path);
        long size=-1;
        if (stream) {
            if (!fseek(stream,0,SEEK_END)) size=ftell(stream);
            fclose(stream);
        }
        if (size<=0 || (uint32_t)size!=share->hashed.files[i].size) return 0;
    }
    for (i=0;i<transfer->package.count;i++) {
        transfer->files[i].size=share->hashed.files[i].size;
        memcpy(transfer->files[i].sha256,share->hashed.files[i].sha256,32);
    }
    return 1;
}

static void host_request_manifest(struct mcc_share *share, long machine, struct mcc_share_request const *request)
{
    struct mcc_share_host_transfer *transfer=host_find(share,machine);
    uint32_t now=share_now(share);
    int i;
    if (share->manifest_seen[machine] && !share_elapsed(now,share->manifest_at[machine],MCC_SHARE_MANIFEST_INTERVAL))
        return;
    share->manifest_at[machine]=now;
    share->manifest_seen[machine]=1;
    if (transfer && host_expired(transfer,now)) {
        host_free(transfer);
        transfer=NULL;
    }
    if (transfer) {
        if (transfer->hashing) return;
        if (share_same(transfer->level,request->level)) {
            transfer->heard_at=now;
            if (!transfer->manifest_pending) transfer->manifest_cursor=0;
            transfer->manifest_pending=1;
            host_send_manifest(share,transfer);
            return;
        }
        host_free(transfer);
    }
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS && share->transfers[i].used;i++) {}
    if (i==MCC_SHARE_HOST_TRANSFERS) {
        host_reply(share,machine,MCC_SHARE_OP_MANIFEST,MCC_SHARE_RESULT_BUSY,request->level,0,0,0,0,0,NULL,NULL,0);
        return;
    }
    transfer=&share->transfers[i];
    memset(transfer,0,sizeof(*transfer));
    /* The admission (opt-in, the rules module, the host's attestation) is the game's;
     * a refusal carries only its reason. */
    share->env.host_package(share->env.user,request->level,&transfer->package);
    if (transfer->package.refusal!=MCC_SHARE_RESULT_OK || transfer->package.count<1 ||
        transfer->package.count>MCC_SHARE_STAGE_FILES) {
        enum mcc_share_result refusal=transfer->package.refusal!=MCC_SHARE_RESULT_OK ?
            transfer->package.refusal : MCC_SHARE_RESULT_NOT_SHAREABLE;
        if (share->env.host_refused)
            share->env.host_refused(share->env.user,machine,request->level,(int)refusal,
                transfer->package.refusal!=MCC_SHARE_RESULT_OK ? "its admission refused it" : "it has no files");
        memset(transfer,0,sizeof(*transfer));
        host_reply(share,machine,MCC_SHARE_OP_MANIFEST,refusal,request->level,0,0,0,0,0,NULL,NULL,0);
        return;
    }
    transfer->used=1;
    transfer->machine=machine;
    transfer->hashing=1;
    transfer->heard_at=transfer->advanced_at=now;
    snprintf(transfer->level,sizeof(transfer->level),"%s",request->level);
    snprintf(transfer->credit,sizeof(transfer->credit),"%s",transfer->package.credit);
    for (i=0;i<transfer->package.count;i++) {
        transfer->files[i].kind=transfer->package.files[i].kind;
        memcpy(transfer->files[i].leaf,transfer->package.files[i].leaf,MCC_SHARE_LEAF_SIZE);
        transfer->files[i].leaf[MCC_SHARE_LEAF_SIZE-1]=0;
    }
    if (host_hashed(share,transfer)) host_offer(share,transfer,0);
}

static void host_fail(struct mcc_share *share, struct mcc_share_host_transfer *transfer,
    enum mcc_share_result result, char const *why)
{
    if (share->env.host_refused)
        share->env.host_refused(share->env.user,transfer->machine,transfer->level,(int)result,why);
    host_reply(share,transfer->machine,MCC_SHARE_OP_MANIFEST,result,transfer->level,0,0,0,0,0,NULL,NULL,0);
    host_free(transfer);
}

/* Hash a slice of the files each frame, so a large map does not stall the host's frame. */
static void host_hash(struct mcc_share *share, struct mcc_share_host_transfer *transfer)
{
    uint32_t budget=MCC_SHARE_HASH_SLICE;
    static uint8_t block[16384];                       /* (static: this runs in the game's frame, on its stack) */
    while (budget && transfer->hash_file<transfer->package.count) {
        struct mcc_share_file *file=&transfer->files[transfer->hash_file];
        /* (elsewhere when it can be: the host's threads, not a slice a frame) */
        if (!transfer->hash_stream && share->env.hash_begin) {
            int status;
            uint64_t size=0;
            if (!transfer->hash_job_active) {
                transfer->hash_job=share->env.hash_begin(share->env.user,transfer->package.files[transfer->hash_file].path);
                if (transfer->hash_job>=0) {
                    transfer->hash_job_active=1;
                    transfer->hash_job_cancel=share->env.hash_cancel;
                    transfer->hash_job_user=share->env.user;
                }
            }
            if (transfer->hash_job_active) {
                status=share->env.hash_poll(share->env.user,transfer->hash_job,file->sha256,&size);
                if (!status) return;
                transfer->hash_job_active=0;
                if (status<0) {
                    host_fail(share,transfer,MCC_SHARE_RESULT_UNAVAILABLE,"a file could not be read to be hashed");
                    return;
                }
                if (!size || size>MCC_SHARE_MAXIMUM_FILE_BYTES) {
                    host_fail(share,transfer,MCC_SHARE_RESULT_UNAVAILABLE,"a file is empty or too large");
                    return;
                }
                file->size=(uint32_t)size;
                transfer->hash_file++;
                continue;
            }
        }
        if (!transfer->hash_stream) {
            long size;
            transfer->hash_stream=share_open_native(share,transfer->package.files[transfer->hash_file].path);
            if (!transfer->hash_stream || fseek(transfer->hash_stream,0,SEEK_END) || (size=ftell(transfer->hash_stream))<=0 ||
                (uint32_t)size>MCC_SHARE_MAXIMUM_FILE_BYTES || fseek(transfer->hash_stream,0,SEEK_SET)) {
                host_fail(share,transfer,MCC_SHARE_RESULT_UNAVAILABLE,transfer->hash_stream ?
                    "a file is empty or too large" : "a file would not open");
                return;
            }
            file->size=(uint32_t)size;
            transfer->hash_offset=0;
            mcc_sha256_begin(&transfer->hash);
        }
        {
            uint32_t want=file->size-transfer->hash_offset;
            size_t got;
            if (want>sizeof(block)) want=sizeof(block);
            if (want>budget) want=budget;
            got=fread(block,1,want,transfer->hash_stream);
            if (got!=want) {
                host_fail(share,transfer,MCC_SHARE_RESULT_UNAVAILABLE,"a file read short");
                return;
            }
            mcc_sha256_add(&transfer->hash,block,(int)got);
            transfer->hash_offset+=(uint32_t)got;
            budget-=(uint32_t)got;
        }
        if (transfer->hash_offset==file->size) {
            mcc_sha256_end(&transfer->hash,file->sha256);
            fclose(transfer->hash_stream);
            transfer->hash_stream=NULL;
            transfer->hash_file++;
        }
    }
    if (transfer->hash_file<transfer->package.count) return;
    host_offer(share,transfer,1);
}

/* Every file hashed (or its digest known): offer the package. */
static void host_offer(struct mcc_share *share, struct mcc_share_host_transfer *transfer, int hashed)
{
    char level_leaf[MCC_SHARE_LEAF_SIZE],folder[64];
    if (!share_level_split(transfer->level,folder,level_leaf) ||
        !mcc_share_manifest_valid(transfer->files,transfer->package.count,level_leaf)) {
        host_fail(share,transfer,MCC_SHARE_RESULT_UNAVAILABLE,"its list of files is not a valid package");
        return;
    }
    if (hashed) {
        share->hashed.valid=1;
        share->hashed.at=share_now(share);
        host_package_key(transfer,share->hashed.key);
        share->hashed.count=transfer->package.count;
        memcpy(share->hashed.files,transfer->files,sizeof(transfer->files[0])*transfer->package.count);
    }
    transfer->hashing=0;
    share->next_session+=0x9E3779B1u;
    transfer->session=(share->next_session^share_now(share))|1;
    transfer->heard_at=transfer->advanced_at=share_now(share);
    transfer->manifest_pending=1;
    host_send_manifest(share,transfer);
}

static void host_request_chunk(struct mcc_share *share, long machine, struct mcc_share_request const *request)
{
    struct mcc_share_host_transfer *transfer=host_find(share,machine);
    struct mcc_share_file const *file;
    if (!transfer) return;
    if (host_expired(transfer,share_now(share))) { host_free(transfer); return; }
    if (transfer->hashing || !transfer->offer_complete) return;
    if (request->session!=transfer->session || !share_same(request->level,transfer->level)) {
        host_reply(share,machine,MCC_SHARE_OP_CHUNK,MCC_SHARE_RESULT_CHANGED,request->level,request->session,
            request->file,0,request->offset,0,NULL,NULL,0);
        return;
    }
    if (request->file>=transfer->package.count || !request->length || request->length>MCC_SHARE_CHUNK_BYTES) {
        host_reply(share,machine,MCC_SHARE_OP_CHUNK,MCC_SHARE_RESULT_BAD_REQUEST,request->level,request->session,
            request->file,0,request->offset,0,NULL,NULL,0);
        return;
    }
    file=&transfer->files[request->file];
    if (request->offset>=file->size || request->length>file->size-request->offset) {
        host_reply(share,machine,MCC_SHARE_OP_CHUNK,MCC_SHARE_RESULT_BAD_REQUEST,request->level,request->session,
            request->file,0,request->offset,0,NULL,NULL,0);
        return;
    }
    /* The receiver requests each range once, in file order, over one reliable
     * stream. Repeats must not replenish its download hold or consume upload. */
    if (request->file!=transfer->request_file || request->offset!=transfer->request_offset) return;
    if (transfer->queued>=(int)(sizeof(transfer->queue)/sizeof(transfer->queue[0]))) return;
    transfer->heard_at=share_now(share);
    transfer->queue[transfer->queued].file=request->file;
    transfer->queue[transfer->queued].offset=request->offset;
    transfer->queue[transfer->queued].length=request->length;
    transfer->queued++;
    transfer->request_offset+=request->length;
    if (transfer->request_offset==file->size) {
        transfer->request_file++;
        transfer->request_offset=0;
    }
}

static void host_serve(struct mcc_share *share, struct mcc_share_host_transfer *transfer)
{
    uint32_t now=share_now(share);
    while (transfer->queued) {
        static uint8_t data[MCC_SHARE_CHUNK_BYTES];
        struct mcc_share_file const *file;
        FILE *stream;
        long end=0;
        uint32_t length=transfer->queue[0].length,offset=transfer->queue[0].offset;
        uint8_t index=transfer->queue[0].file;
        size_t got=0;
        /* A chunk goes whenever the budget is positive and puts it in debt, so a
         * limit lower than a chunk still moves at the limit on average. */
        if (share->budget<=0) return;
        file=&transfer->files[index];
        /* The file stays open while its chunks are served, in order: opened,
         * and its size checked, once a file, not once a 3 KB chunk. */
        stream=transfer->serve_stream && transfer->serve_file==index ? transfer->serve_stream : NULL;
        if (!stream) {
            if (transfer->serve_stream) fclose(transfer->serve_stream);
            transfer->serve_stream=NULL;
            transfer->serve_file=-1;
            stream=share_open_native(share,transfer->package.files[index].path);
            if (stream) setvbuf(stream,NULL,_IOFBF,0x40000);
            if (stream && !fseek(stream,0,SEEK_END)) end=ftell(stream);
            if (stream && end==(long)file->size && !fseek(stream,(long)offset,SEEK_SET)) {
                transfer->serve_stream=stream;
                transfer->serve_file=index;
                transfer->serve_offset=offset;
            } else if (stream) {
                fclose(stream);
                stream=NULL;
            }
        } else if (transfer->serve_offset!=offset && fseek(stream,(long)offset,SEEK_SET)) stream=NULL;
        if (!stream || (got=fread(data,1,length,stream))!=length) {
            host_reply(share,transfer->machine,MCC_SHARE_OP_CHUNK,MCC_SHARE_RESULT_CHANGED,transfer->level,
                transfer->session,index,0,offset,0,NULL,NULL,0);
            host_free(transfer);
            return;
        }
        transfer->serve_offset=offset+length;
        if (!host_reply(share,transfer->machine,MCC_SHARE_OP_CHUNK,MCC_SHARE_RESULT_OK,transfer->level,
            transfer->session,index,0,offset,length,NULL,data,length)) return;
        share->budget-=(int32_t)(sizeof(struct mcc_share_response)+length);
        transfer->sent_bytes+=length;
        transfer->advanced_at=now;
        memmove(&transfer->queue[0],&transfer->queue[1],(size_t)(transfer->queued-1)*sizeof(transfer->queue[0]));
        transfer->queued--;
    }
}

static void host_request_have(struct mcc_share *share, long machine, struct mcc_share_request const *request)
{
    struct mcc_share_host_transfer *transfer=host_find(share,machine);
    uint8_t kind;
    uint32_t now=share_now(share);
    if (!transfer) return;
    if (host_expired(transfer,now)) { host_free(transfer); return; }
    if (transfer->hashing || !transfer->offer_complete || request->session!=transfer->session ||
        !share_same(request->level,transfer->level) || request->file>=transfer->package.count ||
        request->file!=transfer->request_file || transfer->request_offset || transfer->queued) return;
    kind=transfer->files[request->file].kind;
    if (kind!=MCC_SHARE_FILE_BITMAPS && kind!=MCC_SHARE_FILE_SOUNDS) return;
    /* Only a whole, next-in-order resource can advance progress. Replayed HAVE
     * messages cannot reserve the host slot or skip required map/metadata files. */
    transfer->request_file++;
    transfer->reused_files++;
    transfer->heard_at=transfer->advanced_at=now;
}

void mcc_share_host_request(struct mcc_share *share, long machine, void const *message, unsigned bytes)
{
    struct mcc_share_request request;
    struct mcc_share_host_transfer *transfer;
    if (!message || bytes!=sizeof(request) || machine<0 || machine>=MCC_SHARE_HOST_MACHINES) return;
    memcpy(&request,message,sizeof(request));
    if (!share_level_text(request.level) || request.reserved) return;
    switch (request.op) {
    case MCC_SHARE_OP_MANIFEST:
        if (request.file || request.flags || request.session || request.offset || request.length) return;
        host_request_manifest(share,machine,&request);
        break;
    case MCC_SHARE_OP_CHUNK:
        if (request.flags || !request.session) return;
        host_request_chunk(share,machine,&request);
        break;
    case MCC_SHARE_OP_DONE:
        if (request.file || request.flags>1 || !request.session || request.offset || request.length) return;
        transfer=host_find(share,machine);
        if (transfer && !transfer->hashing && request.session==transfer->session &&
            share_same(request.level,transfer->level) &&
            (!request.flags || (transfer->request_file==transfer->package.count && !transfer->queued &&
                               !host_expired(transfer,share_now(share))))) {
            mcc_share_host_forget(share,machine);
            if (request.flags && share->env.host_finished)
                share->env.host_finished(share->env.user,machine);
        }
        break;
    case MCC_SHARE_OP_HAVE:
        if (request.flags || !request.session || request.offset || request.length) return;
        host_request_have(share,machine,&request);
        break;
    }
}

int mcc_share_host_active(struct mcc_share const *share, long machine)
{
    int i;
    uint32_t now=share->env.now(share->env.user);
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS;i++) {
        struct mcc_share_host_transfer const *transfer=&share->transfers[i];
        if (!transfer->used || transfer->machine!=machine) continue;
        if (transfer->hashing) return !share_elapsed(now,transfer->heard_at,MCC_SHARE_OFFER_TIMEOUT);
        return !share_elapsed(now,transfer->heard_at,MCC_SHARE_ACTIVE_HEARD) &&
            (!transfer->sent_bytes && !transfer->reused_files ? (!transfer->offer_complete || !share_elapsed(now,transfer->offered_at,MCC_SHARE_OFFER_TIMEOUT)) :
             !share_elapsed(now,transfer->advanced_at,MCC_SHARE_ACTIVE_ADVANCED));
    }
    return 0;
}

/* ---------- the joiner */

static void client_idle(struct mcc_share *share)
{
    if (share->reuse_stream) fclose(share->reuse_stream);
    share->reuse_stream=NULL;
    share->have_pending=0;
    share->state=MCC_SHARE_CLIENT_IDLE;
    share->outstanding=0;
}

static int client_send(struct mcc_share *share, uint8_t op, uint8_t file, uint8_t flags,
    uint32_t offset, uint32_t length)
{
    struct mcc_share_request request;
    memset(&request,0,sizeof(request));
    request.op=op;
    request.file=file;
    request.flags=flags;
    request.session=op==MCC_SHARE_OP_MANIFEST ? 0 : share->session;
    request.offset=offset;
    request.length=length;
    snprintf(request.level,sizeof(request.level),"%s",share->level);
    return share->env.send_request(share->env.user,&request);
}

static void client_retry_terminal(struct mcc_share *share)
{
    if (share->terminal_pending &&
        (share_elapsed(share_now(share),share->terminal_at,MCC_SHARE_HOST_IDLE_TIMEOUT) ||
         share->env.send_request(share->env.user,&share->terminal))) share->terminal_pending=0;
}

static void client_terminal(struct mcc_share *share, uint8_t flags)
{
    if (!share->session) return;
    memset(&share->terminal,0,sizeof(share->terminal));
    share->terminal.op=MCC_SHARE_OP_DONE;
    share->terminal.flags=flags;
    share->terminal.session=share->session;
    strcpy(share->terminal.level,share->level);
    share->terminal_at=share_now(share);
    share->terminal_pending=1;
    client_retry_terminal(share);
}

static void client_fail(struct mcc_share *share, char const *message)
{
    if (share->state!=MCC_SHARE_CLIENT_IDLE) client_terminal(share,0);
    mcc_share_stage_cancel(&share->stage);
    memset(&share->stage,0,sizeof(share->stage));
    client_idle(share);
    share->env.finished(share->env.user,0,message);
}

static void client_pump(struct mcc_share *share)
{
    int current=share->stage.current;
    if (share->reuse_stream || share->have_pending || current>=share->stage.count) return;
    if (share->reuse_checked_file!=current && share->request_file==current && !share->request_offset &&
        !share->outstanding) {
        struct mcc_share_file const *file=&share->stage.files[current];
        char path[1024];
        share->reuse_checked_file=current;
        memset(path,0,sizeof(path));
        if (share->env.reuse_file && (file->kind==MCC_SHARE_FILE_BITMAPS || file->kind==MCC_SHARE_FILE_SOUNDS) &&
            share->env.reuse_file(share->env.user,file,path)) {
            long bytes=-1;
            if (memchr(path,0,sizeof(path))) share->reuse_stream=share_open_native(share,path);
            if (share->reuse_stream && !fseek(share->reuse_stream,0,SEEK_END)) bytes=ftell(share->reuse_stream);
            if (bytes<0 || (uint32_t)bytes!=file->size || fseek(share->reuse_stream,0,SEEK_SET)) {
                client_fail(share,"A previously downloaded resource could not be read or changed size. Nothing was installed.");
                return;
            }
            share->reuse_file=current;
            share->reported_at=share_now(share);
            return;
        }
    }
    /* Decide reuse before requesting a file. The chunk window stays within that
     * file so no speculative resource requests precede its local verification. */
    while (share->outstanding<MCC_SHARE_CLIENT_WINDOW && share->request_file==current &&
        share->request_file<share->stage.count) {
        uint32_t size=share->stage.files[share->request_file].size;
        uint32_t length=size-share->request_offset;
        if (length>MCC_SHARE_CHUNK_BYTES) length=MCC_SHARE_CHUNK_BYTES;
        if (!client_send(share,MCC_SHARE_OP_CHUNK,(uint8_t)share->request_file,0,share->request_offset,length)) return;
        share->outstanding++;
        share->request_offset+=length;
        if (share->request_offset==size) {
            share->request_file++;
            share->request_offset=0;
        }
    }
}

static void client_reuse(struct mcc_share *share)
{
    uint32_t budget=MCC_SHARE_REUSE_SLICE,began=share_now(share);
    static unsigned char block[65536];
    while (share->reuse_stream && budget && !share_elapsed(share_now(share),began,MCC_SHARE_REUSE_MILLISECONDS)) {
        uint32_t want=mcc_share_stage_wanted(&share->stage);
        enum mcc_share_stage_result result;
        if (want>sizeof(block)) want=sizeof(block);
        if (want>budget) want=budget;
        if (!want || fread(block,1,want,share->reuse_stream)!=want) {
            client_fail(share,"A previously downloaded resource could not be read. Nothing was installed.");
            return;
        }
        result=mcc_share_stage_write(&share->stage,block,want);
        budget-=want;
        share->progress_at=share_now(share);
        if (result==MCC_SHARE_STAGE_FILE_DONE || result==MCC_SHARE_STAGE_ALL_DONE) {
            fclose(share->reuse_stream);
            share->reuse_stream=NULL;
            share->request_file=share->stage.current;
            share->request_offset=0;
            share->have_pending=1;
        } else if (result!=MCC_SHARE_STAGE_OK) {
            client_fail(share,result==MCC_SHARE_STAGE_BAD_HASH ?
                "A previously downloaded resource changed: its SHA-256 does not match. Nothing was installed." :
                "A previously downloaded resource could not be copied. Nothing was installed.");
            return;
        }
    }
    if (share->have_pending && client_send(share,MCC_SHARE_OP_HAVE,(uint8_t)share->reuse_file,0,0,0)) {
        share->have_pending=0;
        if (share->stage.state==MCC_SHARE_STAGE_STAGED) share->state=MCC_SHARE_CLIENT_VERIFYING;
        else client_pump(share);
    }
    if ((share->reuse_stream || share->have_pending) &&
        share_elapsed(share_now(share),share->reported_at,MCC_SHARE_OFFER_HEARTBEAT) &&
        client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0)) share->reported_at=share_now(share);
}

int mcc_share_client_begin(struct mcc_share *share, char const *level, unsigned char const digest[32])
{
    char folder[64],leaf[MCC_SHARE_LEAF_SIZE];
    client_retry_terminal(share);
    if (share->state!=MCC_SHARE_CLIENT_IDLE || !level || strlen(level)>=MCC_SHARE_LEVEL_SIZE ||
        !share_level_split(level,folder,leaf) || share->env.package_taken(share->env.user,level)) return 0;
    memset(&share->stage,0,sizeof(share->stage));
    snprintf(share->level,sizeof(share->level),"%s",level);
    memcpy(share->digest,digest,32);
    share->session=0;
    share->credit[0]=0;
    share->request_file=0;
    share->request_offset=0;
    share->outstanding=0;
    share->reuse_checked_file=-1;
    share->offer_count=share->offer_received=0;
    share->offer_total=0;
    share->state=MCC_SHARE_CLIENT_ASKING;
    share->asked_at=share->progress_at=share_now(share);
    share->ask_pending=share->terminal_pending || !client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0);
    return 1;
}

int mcc_share_client_accept(struct mcc_share *share)
{
    char folder[64],leaf[MCC_SHARE_LEAF_SIZE],text[256];
    enum mcc_share_stage_result result;
    uint64_t free_bytes;
    if (share->state!=MCC_SHARE_CLIENT_WAITING) return 0;
    if (share_elapsed(share_now(share),share->progress_at,MCC_SHARE_OFFER_TIMEOUT)) {
        client_fail(share,"The map download offer expired. Join again to request it.");
        return 0;
    }
    if (!share_level_split(share->level,folder,leaf)) return 0;
    if (share->env.destination && !share->env.destination(share->env.user,share->level,
        share->offer_files,share->offer_count,folder)) {
        client_fail(share,"The host's package cannot be installed into its download folder.");
        return 0;
    }
    free_bytes=share->env.free_bytes ? share->env.free_bytes(share->env.user) : 0;
    if (free_bytes && share->offer_total+(64u<<20)>free_bytes) {
        client_fail(share,"There is not enough free disk space for the host's map. Free some space and join again.");
        return 0;
    }
    result=mcc_share_stage_begin(&share->stage,&share->env.io,share->env.root,folder,leaf,
        share->offer_files,share->offer_count);
    if (result==MCC_SHARE_STAGE_EXISTS) {
        snprintf(text,sizeof(text),"A package folder named %s already exists here but does not match the host's. "
            "Rename or remove it, or install the host's version, then join again.",folder);
        client_fail(share,text);
        return 0;
    }
    if (result!=MCC_SHARE_STAGE_OK) {
        client_fail(share,result==MCC_SHARE_STAGE_STORAGE_UNCERTAIN ?
            "Browser storage stopped responding. Close this page and reopen it to recover the download. Existing packages were kept." :
            "The host's list of files was not acceptable.");
        return 0;
    }
    share->state=MCC_SHARE_CLIENT_TRANSFERRING;
    share->stage.cancel_check=share->env.cancel_check;
    share->stage.check_user=share->env.user;
    share->progress_at=share_now(share);
    client_pump(share);
    return 1;
}

static void client_manifest(struct mcc_share *share, struct mcc_share_response const *head,
    uint8_t const *body)
{
    char folder[64],leaf[MCC_SHARE_LEAF_SIZE];
    unsigned i;
    if (!head->count || head->count>MCC_SHARE_STAGE_FILES || !head->length ||
        head->length>MCC_SHARE_MANIFEST_PAGE || head->offset>=head->count ||
        head->length>head->count-head->offset || head->offset!=(unsigned)share->offer_received ||
        (share->offer_received && (share->offer_count!=head->count || share->session!=head->session ||
         strcmp(share->credit,head->credit)))) {
        client_fail(share,"The host's list of files was not acceptable.");
        return;
    }
    if (!share->offer_received) {
        share->offer_count=head->count;
        share->session=head->session;
        strcpy(share->credit,head->credit);
    }
    for (i=0;i<head->length;i++) {
        struct mcc_share_entry entry;
        struct mcc_share_file *file=&share->offer_files[share->offer_received];
        memcpy(&entry,body+i*sizeof(entry),sizeof(entry));
        if (entry.reserved[0] || entry.reserved[1] || entry.reserved[2]) {
            client_fail(share,"The host's list of files was not acceptable.");
            return;
        }
        memset(file,0,sizeof(*file));
        file->kind=entry.kind;
        memcpy(file->leaf,entry.leaf,sizeof(file->leaf));
        file->size=entry.size;
        memcpy(file->sha256,entry.sha256,32);
        share->offer_total+=file->size;
        share->offer_received++;
    }
    if (share->offer_received!=share->offer_count) return;
    if (!share_level_split(share->level,folder,leaf) ||
        !mcc_share_manifest_valid(share->offer_files,share->offer_count,leaf)) {
        client_fail(share,"The host's list of files was not acceptable.");
        return;
    }
    share->state=MCC_SHARE_CLIENT_WAITING;
    share->reported_at=share->progress_at=share_now(share);
    if (share->env.offered) share->env.offered(share->env.user,share->offer_total,share->credit);
    else mcc_share_client_accept(share);
}

static void client_chunk(struct mcc_share *share, struct mcc_share_response const *head, uint8_t const *data,
    unsigned bytes)
{
    enum mcc_share_stage_result result;
    if (head->session!=share->session || head->file!=share->stage.current || head->offset!=share->stage.written ||
        !head->length || head->length!=bytes || head->length>MCC_SHARE_CHUNK_BYTES ||
        head->length>mcc_share_stage_wanted(&share->stage) || share->outstanding<=0) {
        client_fail(share,"The host sent data this machine did not ask for. The download was stopped.");
        return;
    }
    share->outstanding--;
    result=mcc_share_stage_write(&share->stage,data,head->length);
    share->progress_at=share_now(share);
    switch (result) {
    case MCC_SHARE_STAGE_OK:
    case MCC_SHARE_STAGE_FILE_DONE:
        client_pump(share);
        break;
    case MCC_SHARE_STAGE_ALL_DONE:
        share->state=MCC_SHARE_CLIENT_VERIFYING;
        break;
    case MCC_SHARE_STAGE_BAD_HASH:
        client_fail(share,"A file from the host did not match its SHA-256. Nothing was installed.");
        break;
    default:
        client_fail(share,"The files from the host could not be written. Nothing was installed.");
        break;
    }
}

void mcc_share_client_response(struct mcc_share *share, void const *message, unsigned bytes)
{
    struct mcc_share_response head;
    uint8_t const *body;
    char text[256];
    if (!message || share->state==MCC_SHARE_CLIENT_IDLE || bytes<sizeof(head) ||
        (share->state==MCC_SHARE_CLIENT_ASKING && share->ask_pending)) return;
    body=(uint8_t const *)message+sizeof(head);
    memcpy(&head,message,sizeof(head));
    if (!share_level_text(head.level) || !share_same(head.level,share->level)) return;
    bytes-=(unsigned)sizeof(head);
    if (head.result!=MCC_SHARE_RESULT_OK) {
        if ((head.op!=MCC_SHARE_OP_MANIFEST && head.op!=MCC_SHARE_OP_CHUNK) || bytes ||
            head.result>MCC_SHARE_RESULT_CHANGED || head.count || head.length ||
            !share_zero(head.credit,sizeof(head.credit))) return;
        if (head.op==MCC_SHARE_OP_MANIFEST && (head.file || head.offset || head.session ||
            (share->state!=MCC_SHARE_CLIENT_ASKING && share->state!=MCC_SHARE_CLIENT_WAITING))) return;
        if (head.op==MCC_SHARE_OP_CHUNK && (share->state!=MCC_SHARE_CLIENT_TRANSFERRING ||
            head.session!=share->session || head.file!=share->stage.current || head.offset!=share->stage.written)) return;
        mcc_share_refusal_text((enum mcc_share_result)head.result,share->level,text,sizeof(text));
        client_fail(share,text);
        return;
    }
    if (head.op==MCC_SHARE_OP_MANIFEST) {
        if (share->state!=MCC_SHARE_CLIENT_ASKING || head.file || !head.session ||
            !share_credit_valid(head.credit) || head.count>MCC_SHARE_STAGE_FILES ||
            head.length>MCC_SHARE_MANIFEST_PAGE || bytes!=head.length*sizeof(struct mcc_share_entry)) return;
        client_manifest(share,&head,body);
    } else if (head.op==MCC_SHARE_OP_CHUNK) {
        if (share->state!=MCC_SHARE_CLIENT_TRANSFERRING) return;
        if (head.count || !share_zero(head.credit,sizeof(head.credit))) {
            client_fail(share,"The host sent a malformed map chunk. The download was stopped.");
            return;
        }
        client_chunk(share,&head,body,bytes);
    }
}

static int client_check(void *user, char const *stage_dir)
{
    struct mcc_share *share=user;
    return share->env.identity_check(share->env.user,stage_dir);
}

static void client_cancel_check(struct mcc_share *share)
{
    if (share->stage.cancel_check) {
        void (*cancel)(void *)=share->stage.cancel_check;
        share->stage.cancel_check=NULL;
        cancel(share->stage.check_user);
    }
}

void mcc_share_client_cancel(struct mcc_share *share)
{
    if (share->state==MCC_SHARE_CLIENT_IDLE) return;
    client_terminal(share,0);
    mcc_share_stage_cancel(&share->stage);
    memset(&share->stage,0,sizeof(share->stage));
    client_idle(share);
}

void mcc_share_client_disconnect(struct mcc_share *share)
{
    mcc_share_stage_cancel(&share->stage);
    memset(&share->stage,0,sizeof(share->stage));
    client_idle(share);
    share->terminal_pending=0;
}

int mcc_share_client_busy(struct mcc_share const *share)
{
    return share->state!=MCC_SHARE_CLIENT_IDLE;
}

int mcc_share_client_level(struct mcc_share const *share, char const *level)
{
    return share->state!=MCC_SHARE_CLIENT_IDLE && level && share_same(share->level,level);
}

int mcc_share_client_percent(struct mcc_share const *share)
{
    if (share->state==MCC_SHARE_CLIENT_IDLE) return -1;
    if (share->state==MCC_SHARE_CLIENT_VERIFYING) return 100;
    if (share->state!=MCC_SHARE_CLIENT_TRANSFERRING || !share->stage.total) return 0;
    return (int)(((uint64_t)share->stage.done*100u)/share->stage.total);
}

static void client_update(struct mcc_share *share)
{
    uint32_t now=share_now(share);
    /* Keep the new intent while the preceding DONE waits for this same reliable
     * stream. Its bounded retry expires before the new offer's timeout starts. */
    if (share->state==MCC_SHARE_CLIENT_ASKING && share->terminal_pending) {
        share->asked_at=now;
        return;
    }
    if (share->state==MCC_SHARE_CLIENT_ASKING && share_elapsed(now,share->asked_at,MCC_SHARE_ASK_TIMEOUT)) {
        client_fail(share,"The host did not answer the request for the map.");
    } else if (share->state==MCC_SHARE_CLIENT_ASKING && share->ask_pending) {
        share->ask_pending=!client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0);
    } else if (share->state==MCC_SHARE_CLIENT_WAITING) {
        if (share_elapsed(now,share->progress_at,MCC_SHARE_OFFER_TIMEOUT))
            client_fail(share,"The map download offer expired. Join again to request it.");
        else if (share_elapsed(now,share->reported_at,MCC_SHARE_OFFER_HEARTBEAT) &&
            client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0)) share->reported_at=now;
    } else if (share->state==MCC_SHARE_CLIENT_TRANSFERRING &&
        share_elapsed(now,share->progress_at,MCC_SHARE_STALL_TIMEOUT)) {
        client_fail(share,"The download stopped: the host sent nothing for a minute.");
    } else if (share->state==MCC_SHARE_CLIENT_TRANSFERRING) {
        client_pump(share);
        if (share->state==MCC_SHARE_CLIENT_TRANSFERRING) client_reuse(share);
    } else if (share->state==MCC_SHARE_CLIENT_VERIFYING) {
        enum mcc_share_stage_result result;
        if (share_elapsed(now,share->progress_at,MCC_SHARE_ACTIVE_ADVANCED)) {
            client_fail(share,"The downloaded map could not be verified in time. Nothing was installed.");
            return;
        }
        /* Polling is bounded by the last received/copied byte, not heartbeats.
         * Keep the host's existing transfer alive while its identity is checked. */
        if (share_elapsed(now,share->reported_at,MCC_SHARE_OFFER_HEARTBEAT) &&
            client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0)) share->reported_at=now;
        result=mcc_share_stage_verify(&share->stage,client_check,share);
        if (result==MCC_SHARE_STAGE_PENDING) return;
        if (result==MCC_SHARE_STAGE_REFUSED) {
            char message[384];
            char const *reason=share->env.identity_error ? share->env.identity_error(share->env.user) : NULL;
            snprintf(message,sizeof(message),"%s Nothing was installed.",reason && reason[0] ? reason :
                "The files from the host are not the host's map: their SHA-256 identity differs.");
            memset(&share->stage,0,sizeof(share->stage));
            client_terminal(share,0);
            client_idle(share);
            share->env.finished(share->env.user,0,message);
            return;
        }
        if (result==MCC_SHARE_STAGE_OK && share->env.prepare_install &&
            !share->env.prepare_install(share->env.user,&share->stage)) {
            client_fail(share,"The download metadata could not be saved. Nothing was installed.");
            return;
        }
        if (result==MCC_SHARE_STAGE_OK) result=mcc_share_stage_install(&share->stage);
        if (result!=MCC_SHARE_STAGE_OK) {
            client_cancel_check(share);
            memset(&share->stage,0,sizeof(share->stage));
            client_terminal(share,0);
            client_idle(share);
            share->env.finished(share->env.user,0,result==MCC_SHARE_STAGE_EXISTS ?
                "A package of that name appeared during the download. Nothing was installed." :
                result==MCC_SHARE_STAGE_STORAGE_UNCERTAIN ?
                "Browser storage stopped responding. Close this page and reopen it to recover the download. Existing packages were kept." :
                "The package could not be installed. Nothing was installed.");
            return;
        }
        if (share->env.installed_ready && !share->env.installed_ready(share->env.user)) {
            client_cancel_check(share);
            client_fail(share,"The map was installed, but its verified identity could not be bound to this game. Join again to retry.");
            return;
        }
        client_cancel_check(share);
        client_terminal(share,1);
        memset(&share->stage,0,sizeof(share->stage));
        client_idle(share);
        share->env.finished(share->env.user,1,share->credit);
    }
}

void mcc_share_update(struct mcc_share *share)
{
    int i;
    uint32_t now=share_now(share);
    uint32_t elapsed=now-share->budget_at,limit=share->env.upload_limit(share->env.user);
    uint64_t allowance;
    int64_t budget;
    if (elapsed>1000) elapsed=1000;
    if (limit>0x7fffffffu) limit=0x7fffffffu;
    share->budget_at=now;
    allowance=(uint64_t)limit*elapsed+share->budget_fraction;
    share->budget_fraction=(uint32_t)(allowance%1000u);
    budget=(int64_t)share->budget+(int64_t)(allowance/1000u);
    share->budget=(int32_t)(budget>limit ? limit : budget);
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS;i++) {
        struct mcc_share_host_transfer *transfer=&share->transfers[(share->serve_first+i)%MCC_SHARE_HOST_TRANSFERS];
        if (!transfer->used) continue;
        if (host_expired(transfer,now)) {
            host_free(transfer);
            continue;
        }
        if (transfer->hashing) host_hash(share,transfer);
        else if (transfer->manifest_pending) host_send_manifest(share,transfer);
        else host_serve(share,transfer);
    }
    share->serve_first=(share->serve_first+1)%MCC_SHARE_HOST_TRANSFERS;
    client_retry_terminal(share);
    client_update(share);
}

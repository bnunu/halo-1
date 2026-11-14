/* The same CRC and SHA framing as the synchronous load checks, with bounded
 * slices for lobby navigation. Owns its readers and never draws or ticks game
 * state. The source stream belongs to mcc_validation and outlives this check. */
#if defined(HALO_ANDROID)
/* The hashing is the host's (port/android/host/host_mcc_hash.c): the CPU's
 * SHA-256 and CRC32 instructions on two threads, and the results kept on the
 * card, a Steam Workshop package's worked out as it was installed. The
 * guest's addresses are the host's, passed as 32-bit numbers. */
const char *platform_data_root(void);
void platform_translate_path(const char *xbox_path, char *host_path, unsigned long host_path_size);
int host_mcc_hash_start(unsigned int request);
int host_mcc_hash_wait(int id, int milliseconds, unsigned int result);
void host_mcc_hash_cancel(int id);
struct mcc_host_hash_request {uint32_t data_root;uint32_t paths[3];uint32_t crc;};
struct mcc_host_hash_result {uint32_t crc;unsigned char digest[32];};
#endif

static int mcc_resources_host_fingerprint(struct mcc_runtime *runtime,unsigned char digest[32])
{
#if defined(HALO_ANDROID)
    static char paths[3][1024];
    static struct mcc_host_hash_request request;
    static struct mcc_host_hash_result result;
    char xbox_path[128];
    unsigned part;
    int job,status;
    memset(paths,0,sizeof(paths));
    if (!mcc_package_map_path(runtime->level_name,xbox_path)) return 0;
    platform_translate_path(xbox_path,paths[0],sizeof(paths[0]));
    for (part=1;part<=2;part++) {
        struct mcc_resource_file *resource;
        if (!(part==1 ? runtime->report.external_bitmap_count : runtime->report.external_sound_count)) continue;
        resource=mcc_resource_get(runtime,part);
        if (!resource || !resource->path[0]) return 0;
        platform_translate_path(resource->path,paths[part],sizeof(paths[part]));
    }
    memset(&request,0,sizeof(request));
    request.data_root=(uint32_t)(uintptr_t)platform_data_root();
    for (part=0;part<3;part++) request.paths[part]=paths[part][0] ? (uint32_t)(uintptr_t)paths[part] : 0;
    job=host_mcc_hash_start((unsigned int)(uintptr_t)&request);
    if (job<0) return 0;
    /* (the loading screen kept alive while it waits) */
    while (!(status=host_mcc_hash_wait(job,20,(unsigned int)(uintptr_t)&result)))
        mcc_loading_pulse();
    if (status<0) return 0;
    memcpy(digest,result.digest,32);
    return 1;
#else
    (void)runtime;
    (void)digest;
    return 0;
#endif
}

struct mcc_resource_check {
    struct mcc_runtime runtime;
    struct mcc_sha256 hash;
    uint32_t crc, offset, size;
    unsigned type, mask;
    int framed;
#if defined(HALO_ANDROID)
    /* the files' paths on the host (map, bitmaps, sounds; "" not used), and
     * the host's job (-1 none yet, -2 none: the guest hashes) */
    char host_paths[3][1024];
    int host_job;
    struct mcc_host_hash_request host_request;
    struct mcc_host_hash_result host_result;
#endif
};

int mcc_resources_check_paths(struct mcc_resource_check const *check, char paths[2][128])
{
    unsigned type;
    memset(paths,0,2*128);
    if (!check) return 0;
    for (type=1;type<=2;type++) {
        struct mcc_resources const *resources=check->runtime.resources;
        if (!(check->mask&(1u<<(type-1)))) continue;
        if (!resources || !resources->files[type-1].stream) return 0;
        strcpy(paths[type-1],resources->files[type-1].path);
    }
    return 1;
}

void mcc_resources_check_end(struct mcc_resource_check *check)
{
    if (!check) return;
#if defined(HALO_ANDROID)
    if (check->host_job>=0) host_mcc_hash_cancel(check->host_job);
#endif
    mcc_resources_dispose(&check->runtime);
    free(check);
}

/* Reject a broken authoritative override before trying any base combinations.
 * Base files that do not satisfy the map are not fingerprint candidates. */
static int mcc_resources_candidates(struct mcc_runtime *runtime,void *raw,
    struct mcc_resource_candidates *choices)
{
    unsigned type,set,i;
    for (type=1;type<=2;type++) {
        unsigned *count=&choices->count[type-1];
        if (!(type==1 ? runtime->report.external_bitmap_count : runtime->report.external_sound_count)) {
            *count=1;
            continue;
        }
        for (set=0;set<MCC_PACKAGE_BASE_SETS;set++) {
            struct mcc_resource_file file;
            struct mcc_resource_probe probe;
            char path[128];
            int authoritative,valid;
            if (!mcc_package_resource_candidate(runtime->level_name,type,set,path)) {
                snprintf(runtime->failure,sizeof(runtime->failure),
                    "Cannot read %s. Check access to this MCC resource.",path);
                return 0;
            }
            authoritative=!mcc_package_shared_resource(path,type);
            for (i=0;i<*count;i++) if (!strcmp(path,choices->paths[type-1][i])) break;
            if (i<*count) continue;
            memset(&file,0,sizeof(file));
            runtime->failure[0]=0;
            valid=mcc_resource_open(&file,type,runtime->level_name,path,runtime->failure,sizeof(runtime->failure));
            if (valid==1) {
                probe.type=type;probe.file=&file;
                valid=mcc_cache_visit_resources(raw,&runtime->report,mcc_resource_check,&probe);
                if (!valid) snprintf(runtime->failure,sizeof(runtime->failure),
                    "%s does not satisfy this MCC map's external references. Install the matching resource.",path);
            } else valid=0;
            mcc_resource_close(&file);
            if (valid) strcpy(choices->paths[type-1][(*count)++],path);
            if (authoritative) {
                if (!valid) return 0;
                break;
            }
        }
        if (!*count) {
            if (!runtime->failure[0]) snprintf(runtime->failure,sizeof(runtime->failure),
                "No readable matching MCC %s. Install the required base resources.",type==1 ? "bitmaps.map" : "sounds.map");
            return 0;
        }
    }
    runtime->failure[0]=0;
    return 1;
}

struct mcc_resource_check *mcc_resources_check_begin_candidate(char const *name,
    struct mcc_cache_source const *source, struct mcc_resource_candidates *candidates, char *reason, size_t size)
{
    struct mcc_resource_check *check;
    struct mcc_cache_identity identity;
    unsigned char header[MCC_CACHE_HEADER_BYTES];
    void *raw=NULL;
    char paths[2][128];
    enum mcc_cache_status status=MCC_CACHE_READ_FAILED;
    check=calloc(1,sizeof(*check));
    if (!check) return NULL;
    if (!source || !source->read || source->size>0x7fffffffu ||
        !mcc_package_name(name,check->runtime.level_name) ||
        !source->read(source->context,0,sizeof(header),header)) goto failed;
    status=mcc_cache_identify(header,sizeof(header),source->size,&identity);
    if (status!=MCC_CACHE_OK) goto failed;
    raw=malloc(identity.tag_data_size);
    status=MCC_CACHE_NO_MEMORY;
    if (!raw) goto failed;
    check->runtime.source=*source;
    status=mcc_cache_load(source,raw,identity.tag_data_size,&check->runtime.report);
    if (status!=MCC_CACHE_OK) goto failed;
    memset(paths,0,sizeof(paths));
    if (candidates) {
        if (!candidates->count[0] && !mcc_resources_candidates(&check->runtime,raw,candidates)) goto failed;
        if (!candidates->count[1] || candidates->index>=candidates->count[0]*candidates->count[1]) goto failed;
        strcpy(paths[0],candidates->paths[0][candidates->index%candidates->count[0]]);
        strcpy(paths[1],candidates->paths[1][candidates->index/candidates->count[0]]);
    }
    if (!mcc_resources_select_paths(&check->runtime,raw,candidates ? paths : NULL)) goto failed;
    free(raw);
    check->mask=(check->runtime.report.external_bitmap_count ? 1u : 0u) |
        (check->runtime.report.external_sound_count ? 2u : 0u);
    check->crc=check->runtime.report.identity.checksum;
#if defined(HALO_ANDROID)
    /* (the paths now, while a staged package's redirect holds: the worker
     * hashes after it is cleared) */
    check->host_job=-2;
    {
        char xbox_path[128];
        unsigned part;
        int usable=mcc_package_map_path(check->runtime.level_name,xbox_path);
        if (usable) platform_translate_path(xbox_path,check->host_paths[0],sizeof(check->host_paths[0]));
        for (part=1;usable && part<=2;part++) {
            struct mcc_resources const *resources=check->runtime.resources;
            if (!(check->mask&(1u<<(part-1)))) continue;
            if (!resources || !resources->files[part-1].stream || !resources->files[part-1].path[0]) usable=0;
            else platform_translate_path(resources->files[part-1].path,check->host_paths[part],
                sizeof(check->host_paths[part]));
        }
        if (usable && check->host_paths[0][0]) check->host_job=-1;
    }
#endif
    mcc_sha256_begin(&check->hash);
    mcc_sha256_add(&check->hash,"GulchCE MCC content v1",sizeof("GulchCE MCC content v1")-1);
    return check;
failed:
    if (reason && size) snprintf(reason,size,"%s",check->runtime.failure[0] ?
        check->runtime.failure : mcc_status_explanation(status));
    free(raw);
    mcc_resources_check_end(check);
    return NULL;
}

int mcc_resources_check_step(struct mcc_resource_check *check, uint32_t *checksum,
    unsigned char digest[32])
{
#ifdef HALO_WEB
    /* OPFS proxies each read to a worker. Larger bounded reads reduce that
     * overhead without changing the identity bytes or the slice limits. */
    unsigned char buffer[65536],marker;
#else
    unsigned char buffer[16384],marker;
#endif
    struct mcc_resource_file *resource;
    uint32_t bytes,spent=0;
    unsigned long began=system_milliseconds();
    long size;
#if defined(HALO_ANDROID)
    /* (the host's, when it takes it: the guest's own below otherwise) */
    if (check->host_job==-1 && !check->offset && !check->type && !check->framed) {
        unsigned part;
        check->host_request.data_root=(uint32_t)(uintptr_t)platform_data_root();
        for (part=0;part<3;part++)
            check->host_request.paths[part]=check->host_paths[part][0] ? (uint32_t)(uintptr_t)check->host_paths[part] : 0;
        check->host_request.crc=check->crc;
        check->host_job=host_mcc_hash_start((unsigned int)(uintptr_t)&check->host_request);
        if (check->host_job<0) check->host_job=-2;
    }
    if (check->host_job>=0) {
        int status=host_mcc_hash_wait(check->host_job,10,(unsigned int)(uintptr_t)&check->host_result);
        if (!status) return 0;
        check->host_job=-2;
        if (status<0) return -1;
        *checksum=check->host_result.crc;
        if (!*checksum) *checksum=1;
        memcpy(digest,check->host_result.digest,32);
        return 1;
    }
#endif
    /* Bound both CPU work and bytes. No validation slices run in gameplay. */
    do {
        resource=check->type ? mcc_resource_get(&check->runtime,check->type) : NULL;
        if (!check->framed) {
            if (check->type) {
                if (!resource || fseek(resource->stream,0,SEEK_END) ||
                    (size=ftell(resource->stream))<0 || fseek(resource->stream,0,SEEK_SET)) return -1;
                check->size=(uint32_t)size;
                marker=(unsigned char)check->type;
                check->crc=(uint32_t)crc32(check->crc,&marker,1);
            } else check->size=(uint32_t)check->runtime.source.size;
            mcc_fingerprint_file_header(&check->hash,check->type,check->size);
            check->framed=1;
        }
        bytes=check->size-check->offset;
        if (bytes>sizeof(buffer)) bytes=sizeof(buffer);
        if (bytes) {
            if (check->type) {
                if (fread(buffer,1,bytes,resource->stream)!=bytes) return -1;
                check->crc=(uint32_t)crc32(check->crc,buffer,(uInt)bytes);
            } else if (!check->runtime.source.read(check->runtime.source.context,
                check->offset,bytes,buffer)) return -1;
            mcc_sha256_add(&check->hash,buffer,(int)bytes);
            check->offset+=bytes;
            spent+=bytes;
        }
        if (check->offset==check->size) {
            if (check->type) {
                marker=0xff;
                check->crc=(uint32_t)crc32(check->crc,&marker,1);
            }
            do {check->type++;} while (check->type<=2 && !(check->mask&(1u<<(check->type-1))));
            if (check->type>2) {
                *checksum=check->crc;
                if (!*checksum) *checksum=1;
                mcc_sha256_end(&check->hash,digest);
                return 1;
            }
            check->offset=0;
            check->framed=0;
        }
    } while (spent<4194304 && (unsigned long)(system_milliseconds()-began)<2);
    return 0;
}

struct mcc_resource_check *mcc_resources_check_begin(char const *name,
    struct mcc_cache_source const *source,char *reason,size_t size)
{
    return mcc_resources_check_begin_candidate(name,source,NULL,reason,size);
}

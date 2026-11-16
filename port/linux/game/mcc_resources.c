/* MCC resource cache reader. The on-disk header and twelve-byte entries are
 * decoded explicitly; no CE resource state or readers are shared. */
#include "mcc_runtime.h"
#include "mcc_loading.h"
#include "mcc_resources.h"
#include "mcc_identity.h"
#include "mcc_sha256.h"
#include "mcc_package.h"
#include "mcc_errors.h"
#include "cseries.h"
#include "errors.h"
#include "../../third_party/zlib/zlib_prefixed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
unsigned long system_milliseconds(void);

/* Logging includes the game's allocation macros. This reader owns libc
 * allocations (including calloc), so keep their original matching frees. */
#undef malloc
#undef free

#define MCC_RESOURCE_METADATA_LIMIT 0x1000000u
struct mcc_resource_item { uint32_t name, size, offset; };
struct mcc_resource_file {
    FILE *stream;
    char *names;
    struct mcc_resource_item *items;
    uint32_t count,data_bytes;
    int own; /* the map's own file, not a shared one (mcc_package_shared_resource) */
    char path[128];
};
struct mcc_resources { struct mcc_resource_file files[2]; };

static uint32_t mcc_resource_word(unsigned char const *p)
{
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}

static void mcc_resource_close(struct mcc_resource_file *file)
{
    if (file->stream) fclose(file->stream);
    free(file->names);free(file->items);memset(file,0,sizeof(*file));
}

static int mcc_resource_open(struct mcc_resource_file *file,unsigned type,char const *level,char const *explicit_path,
    char *reason,size_t reason_size)
{
    unsigned char header[16],entry[12];
    uint32_t names,index,count,names_bytes,i;
    long length;
    char path[128];
    if (explicit_path) {
        snprintf(path,sizeof(path),"%s",explicit_path);errno=0;file->stream=fopen(path,"rb");
    } else file->stream=mcc_package_resource_open(level,type,path);
    if (!file->stream) {
        if (errno==ENOENT) return -1;
        error(_error_silent,"mcc resources: cannot open %s (required by %s)",path,level);
        if (reason && reason_size) snprintf(reason,reason_size,
            "Cannot read %s. Check access to this MCC resource.",type==1 ? "bitmaps.map" : "sounds.map");
        return 0;
    }
    if (fseek(file->stream,0,SEEK_END) || (length=ftell(file->stream))<16 ||
        fseek(file->stream,0,SEEK_SET) || fread(header,1,16,file->stream)!=16) goto failed;
    names=mcc_resource_word(header+4);index=mcc_resource_word(header+8);count=mcc_resource_word(header+12);
    if (mcc_resource_word(header)!=type || names<16 || index<names || index>(uint32_t)length ||
        !count || count>MCC_RESOURCE_METADATA_LIMIT/sizeof(*file->items) ||
        count>((uint32_t)length-index)/12) goto failed;
    names_bytes=index-names;
    if (!names_bytes || names_bytes>MCC_RESOURCE_METADATA_LIMIT) goto failed;
    file->names=malloc(names_bytes);file->items=malloc(count*sizeof(*file->items));
    if (!file->names || !file->items) {
        if (reason && reason_size) snprintf(reason,reason_size,"Not enough memory to inspect MCC resources. Close other applications.");
        goto failed;
    }
    if (fseek(file->stream,(long)names,SEEK_SET) ||
        fread(file->names,1,names_bytes,file->stream)!=names_bytes) goto failed;
    for (i=0;i<count;i++) {
        struct mcc_resource_item *item=&file->items[i];
        if (fread(entry,1,12,file->stream)!=12) goto failed;
        item->name=mcc_resource_word(entry);item->size=mcc_resource_word(entry+4);item->offset=mcc_resource_word(entry+8);
        if (item->name>=names_bytes || !memchr(file->names+item->name,0,
            names_bytes-item->name<320 ? names_bytes-item->name : 320) ||
            item->offset<16 || item->offset>names || item->size>names-item->offset) goto failed;
    }
    file->count=count;
    file->data_bytes=names;
    file->own=!mcc_package_shared_resource(path,type);
    strcpy(file->path,path);
    return 1;
failed:
    error(_error_silent,"mcc resources: invalid resource file %s (required by %s)",path,level);
    if (reason && reason_size && !reason[0]) snprintf(reason,reason_size,
        "MCC %s is invalid or unreadable. Reinstall the matching resource.",type==1 ? "bitmaps.map" : "sounds.map");
    mcc_resource_close(file);return 0;
}

static struct mcc_resource_file *mcc_resource_get(struct mcc_runtime *runtime,unsigned type)
{
    struct mcc_resources *resources;
    struct mcc_resource_file *file;
    if (!runtime || type<1 || type>2) return NULL;
    resources=runtime->resources;
    if (!resources) {
        resources=calloc(1,sizeof(*resources));
        if (!resources) return NULL;
        runtime->resources=resources;
    }
    file=&resources->files[type-1];
    if (!file->stream && mcc_resource_open(file,type,runtime->level_name,NULL,NULL,0)!=1) return NULL;
    return file;
}

/* whole: the map's check of a reference's full extent; a decoder's read may
   take less of it */
static int mcc_resource_read(struct mcc_resource_file *file,char const *name,
    uint32_t offset,uint32_t bytes,void *out,char const *group_name,int whole)
{
    uint32_t i;
    struct mcc_resource_item const *found=NULL;
    if (!name || !bytes || offset<16 || offset>file->data_bytes || bytes>file->data_bytes-offset) return 0;
    for (i=0;i<file->count;i++) if (!strcmp(file->names+file->items[i].name,name)) {
        if (found) return 0;
        found=&file->items[i];
    }
    if (found) {
        /* Only whole, uniquely named members may be relocated. */
        if (bytes>found->size || (offset!=found->offset && bytes!=found->size)) return 0;
        offset=found->offset;
    } else {
        /* Grouped MCC resources require the original, bounded slice. */
        if (group_name) for (i=0;i<file->count;i++) if (!strcmp(file->names+file->items[i].name,group_name)) {
            if (found) return 0;
            found=&file->items[i];
        }
        if (found) {
            if (offset<found->offset || offset-found->offset>found->size ||
                bytes>found->size-(offset-found->offset)) return 0;
        } else {
            /* (MCC reads a resource at its offset, whatever the member's name:
               in the map's own file, a map built with a tag renamed since keeps
               its sounds and bitmaps. Its check takes exactly one whole member,
               and a decoder then reads from that member's start within it. A
               shared file is never chosen this way: the same offset and size in
               another file says nothing of which file the map was built with) */
            if (!file->own) return 0;
            for (i=0;i<file->count && !found;i++)
                if (file->items[i].offset==offset &&
                    (whole ? file->items[i].size==bytes : file->items[i].size>=bytes)) found=&file->items[i];
            if (!found) return 0;
        }
    }
    return !out || (!fseek(file->stream,(long)offset,SEEK_SET) && fread(out,1,bytes,file->stream)==bytes);
}

int mcc_resources_read(struct mcc_runtime *runtime,unsigned type,char const *name,
    uint32_t offset,uint32_t bytes,void *out,char const *group_name)
{
    struct mcc_resource_file *file=mcc_resource_get(runtime,type);
    return file && out && mcc_resource_read(file,name,offset,bytes,out,group_name,0);
}

struct mcc_resource_probe {struct mcc_resource_file *file;unsigned type;char problem[640];};
static int mcc_resource_check(void *context,struct mcc_resource_reference const *ref)
{
    struct mcc_resource_probe *probe=context;
    char name[288],group[288];
    if (ref->type!=probe->type) return 1;
    if (strlen(ref->tag_name)>255) {
        snprintf(probe->problem,sizeof(probe->problem),"resource tag path exceeds 255 characters: %.255s...",ref->tag_name);return 0;
    }
    if (ref->type==1) snprintf(name,sizeof(name),"%s_%lu",ref->tag_name,(unsigned long)ref->first);
    else snprintf(name,sizeof(name),"%s__%lu__%lu",ref->tag_name,(unsigned long)ref->first,(unsigned long)ref->second);
    snprintf(group,sizeof(group),"%s%s",ref->tag_name,ref->type==1 ? "__pixels" : "__permutations");
    if (!mcc_resource_read(probe->file,name,ref->offset,ref->bytes,NULL,group,1)) {
        if (ref->type==1) snprintf(probe->problem,sizeof(probe->problem),
            "'%.255s.bitmap', bitmaps[%lu]: resource '%s' (offset 0x%08lx, %lu bytes) is missing, ambiguous or outside its matching resource entry",
            ref->tag_name,(unsigned long)ref->first,name,(unsigned long)ref->offset,(unsigned long)ref->bytes);
        else snprintf(probe->problem,sizeof(probe->problem),
            "'%.255s.sound', pitch_ranges[%lu].permutations[%lu]: resource '%s' (offset 0x%08lx, %lu bytes) is missing, ambiguous or outside its matching resource entry",
            ref->tag_name,(unsigned long)ref->first,(unsigned long)ref->second,name,(unsigned long)ref->offset,(unsigned long)ref->bytes);
        return 0;
    }
    return 1;
}

static int mcc_resource_equal(FILE *a,FILE *b)
{
    unsigned char left[16384],right[16384];
    size_t n,m;
    if (fseek(a,0,SEEK_SET) || fseek(b,0,SEEK_SET)) return 0;
    do {
        n=fread(left,1,sizeof(left),a);m=fread(right,1,sizeof(right),b);
        mcc_loading_pulse();
        if (n!=m || memcmp(left,right,n)) return 0;
    } while (n);
    return !ferror(a) && !ferror(b);
}

int mcc_resources_select_paths(struct mcc_runtime *runtime,void *raw,char const paths[2][128])
{
    struct mcc_resources *resources;
    unsigned type;
    if (!runtime || runtime->resources) return 0;
    if (!runtime->report.external_bitmap_count && !runtime->report.external_sound_count) return 1;
    resources=calloc(1,sizeof(*resources));
    if (!resources) {
        snprintf(runtime->failure,sizeof(runtime->failure),"Not enough memory to check the required MCC resources.");
        return 0;
    }
    runtime->resources=resources;
    for (type=1;type<=2;type++) {
        struct mcc_resource_file *selected=&resources->files[type-1];
        struct mcc_resource_probe probe;
        int status,set;
        if (!(type==1 ? runtime->report.external_bitmap_count : runtime->report.external_sound_count)) continue;
        memset(&probe,0,sizeof(probe));probe.type=type;probe.file=selected;
        /* Explicit package and legacy root files are authoritative. */
        status=mcc_resource_open(selected,type,runtime->level_name,paths ? paths[type-1] : NULL,
            runtime->failure,sizeof(runtime->failure));
        if (!status || (paths && status<0)) return 0;
        if (status==1) {
            if (!mcc_cache_visit_resources(raw,&runtime->report,mcc_resource_check,&probe)) {
                error(_error_silent,"mcc resources: %s required by %s: %s; map refused",selected->path,runtime->level_name,probe.problem);
                snprintf(runtime->failure,sizeof(runtime->failure),
                    "%s does not match. Install the matching MCC package resource.",type==1 ? "bitmaps.map" : "sounds.map");
                return 0;
            }
        } else for (set=1;set<MCC_PACKAGE_BASE_SETS;set++) {
            struct mcc_resource_file candidate;
            char path[128];
            memset(&candidate,0,sizeof(candidate));
            if (!mcc_package_base_resource_path(type,(unsigned)set,path)) return 0;
            status=mcc_resource_open(&candidate,type,runtime->level_name,path,runtime->failure,sizeof(runtime->failure));
            if (!status) return 0;
            if (status<0) continue;
            probe.file=&candidate;
            if (mcc_cache_visit_resources(raw,&runtime->report,mcc_resource_check,&probe)) {
                if (selected->stream && !mcc_resource_equal(selected->stream,candidate.stream)) {
                    error(_error_silent,"mcc resources: ambiguous shared %s for %s; put the intended file beside the level",
                        type==1 ? "bitmaps.map" : "sounds.map",runtime->level_name);
                    snprintf(runtime->failure,sizeof(runtime->failure),
                        "Shared %s is ambiguous. Put the intended file beside the level.",
                        type==1 ? "bitmaps.map" : "sounds.map");
                    mcc_resource_close(&candidate);return 0;
                }
                if (!selected->stream) {*selected=candidate;memset(&candidate,0,sizeof(candidate));}
            }
            mcc_resource_close(&candidate);
        }
        if (!selected->stream) {
            if (!probe.problem[0]) {probe.file=selected;mcc_cache_visit_resources(raw,&runtime->report,mcc_resource_check,&probe);}
            if (probe.problem[0]) error(_error_silent,"mcc resources: %s; map refused",probe.problem);
            error(_error_silent,"mcc resources: no complete matching %s for %s; check the package and MCC shared folders",
                type==1 ? "bitmaps.map" : "sounds.map",runtime->level_name);
            snprintf(runtime->failure,sizeof(runtime->failure),
                "No matching %s. Install its MCC package resources beside the level.",
                type==1 ? "bitmaps.map" : "sounds.map");
            return 0;
        }
        error(_error_silent,"mcc resources: using %s (required by %s)",selected->path,runtime->level_name);
    }
    return 1;
}

int mcc_resources_select(struct mcc_runtime *runtime,void *raw)
{
    return mcc_resources_select_paths(runtime,raw,NULL);
}

void mcc_resources_dispose(struct mcc_runtime *runtime)
{
    struct mcc_resources *resources=runtime->resources;
    if (!resources) return;
    mcc_resource_close(&resources->files[0]);mcc_resource_close(&resources->files[1]);
    free(resources);runtime->resources=NULL;
}
int mcc_resources_checksum(struct mcc_runtime *runtime,unsigned resources,uint32_t *checksum)
{
    unsigned char buffer[32768],marker;
    uint32_t crc;
    unsigned type;
    size_t bytes;
    FILE *file;
    struct mcc_resource_file *selected;
    if (!runtime || !checksum || (resources&~3u)) return 0;
    /* zlib chains finalized CRC values; framing and the published value stay
     * identical to the former complemented accumulator. */
    crc=runtime->report.identity.checksum;
    /* Self-contained maps neither open nor fingerprint unrelated resources. */
    if (!resources) { *checksum=runtime->report.identity.checksum ? runtime->report.identity.checksum : 1;return 1; }
    for (type=1;type<=2;type++) {
        if (!(resources&(1u<<(type-1)))) continue;
        selected=mcc_resource_get(runtime,type);
        if (!selected) return 0;
        file=selected->stream;
        if (fseek(file,0,SEEK_SET)) return 0;
        /* Fingerprint the selected contents, not their installation location. */
        marker=(unsigned char)type;
        crc=(uint32_t)crc32(crc,&marker,1);
        while ((bytes=fread(buffer,1,sizeof(buffer),file))!=0) {
            crc=(uint32_t)crc32(crc,buffer,(uInt)bytes);
            mcc_loading_pulse();
        }
        if (ferror(file)) return 0;
        marker=0xff;
        crc=(uint32_t)crc32(crc,&marker,1);
    }
    *checksum=crc;
    if (!*checksum) *checksum=1; /* Zero means no comparison in native admission. */
    return 1;
}

/* Length/type framing keeps file boundaries distinct without including paths. */
static void mcc_fingerprint_file_header(struct mcc_sha256 *hash, unsigned type, uint64_t size)
{
    unsigned char header[9];
    unsigned i;
    header[0]=(unsigned char)type;
    for (i=0;i<8;i++) header[i+1]=(unsigned char)(size>>(i*8));
    mcc_sha256_add(hash,header,sizeof(header));
}

/* the same fingerprint worked out by the host (mcc_resource_check.inl): 1, or 0
when it cannot (the guest works it out then) */
static int mcc_resources_host_fingerprint(struct mcc_runtime *runtime,unsigned char digest[32]);

int mcc_resources_fingerprint(struct mcc_runtime *runtime,unsigned char digest[32])
{
    struct mcc_sha256 hash;
    unsigned char buffer[32768];
    uint32_t offset,bytes;
    unsigned type;
    struct mcc_resource_file *resource;
    long size;
    if (!runtime || !runtime->source.read || runtime->source.size>0x7fffffffu) return 0;
    /* (the host's: the CPU's SHA-256 on its threads, and known at once when its
       files are unchanged since it last worked them out. In the guest, a slice
       at a time in software, a 360 MB package's took 24 to 32 seconds of each
       load of a network game) */
    if (mcc_resources_host_fingerprint(runtime,digest)) return 1;
    mcc_sha256_begin(&hash);
    mcc_sha256_add(&hash,"GulchCE MCC content v1",sizeof("GulchCE MCC content v1")-1);
    mcc_fingerprint_file_header(&hash,0,runtime->source.size);
    for (offset=0;offset<runtime->source.size;offset+=bytes) {
        bytes=(uint32_t)(runtime->source.size-offset);
        if (bytes>sizeof(buffer)) bytes=sizeof(buffer);
        if (!runtime->source.read(runtime->source.context,offset,bytes,buffer)) return 0;
        mcc_sha256_add(&hash,buffer,(int)bytes);
        mcc_loading_pulse();
    }
    for (type=1;type<=2;type++) {
        if (!(type==1 ? runtime->report.external_bitmap_count : runtime->report.external_sound_count)) continue;
        resource=mcc_resource_get(runtime,type);
        if (!resource || fseek(resource->stream,0,SEEK_END) ||
            (size=ftell(resource->stream))<0 || fseek(resource->stream,0,SEEK_SET)) return 0;
        mcc_fingerprint_file_header(&hash,type,(uint32_t)size);
        for (offset=0;offset<(uint32_t)size;offset+=bytes) {
            bytes=(uint32_t)size-offset;
            if (bytes>sizeof(buffer)) bytes=sizeof(buffer);
            if (fread(buffer,1,bytes,resource->stream)!=bytes) return 0;
            mcc_sha256_add(&hash,buffer,(int)bytes);
            mcc_loading_pulse();
        }
    }
    mcc_sha256_end(&hash,digest);
    return 1;
}

static int mcc_resources_inspect(char const *name,struct mcc_cache_source const *source,
    uint32_t *checksum,unsigned char *digest,char *reason,size_t reason_size)
{
    struct mcc_runtime runtime;
    unsigned char header[MCC_CACHE_HEADER_BYTES];
    struct mcc_cache_identity identity;
    void *raw=NULL;
    int ok=0;
    enum mcc_cache_status status=MCC_CACHE_READ_FAILED;
    char const *problem="The map could not be inspected. Check that its complete file is accessible.";
    if (reason && reason_size) reason[0]=0;
    memset(&runtime,0,sizeof(runtime));
    if (!source || !source->read || !mcc_package_name(name,runtime.level_name) ||
        !source->read(source->context,0,sizeof(header),header)) goto done;
    status=mcc_cache_identify(header,sizeof(header),source->size,&identity);
    problem=mcc_status_explanation(status);
    if (status!=MCC_CACHE_OK) goto done;
    raw=malloc(identity.tag_data_size);
    problem=mcc_status_explanation(MCC_CACHE_NO_MEMORY);
    if (!raw) goto done;
    runtime.source=*source;
    status=mcc_cache_load(source,raw,identity.tag_data_size,&runtime.report);
    problem=mcc_status_explanation(status);
    if (status!=MCC_CACHE_OK) {
        error(_error_silent,"mcc: '%s', %s: %s; map refused (tag index %lu, reader location 0x%08lx)",
            name,runtime.report.problem_path[0] ? runtime.report.problem_path : "map header",
            runtime.report.problem_detail,(unsigned long)runtime.report.problem_tag,(unsigned long)runtime.report.problem_location);
        goto done;
    }
    ok=mcc_resources_select(&runtime,raw);
    problem="The required MCC resources could not be checked. See debug.txt for the failing check.";
    if (ok && digest) ok=mcc_resources_fingerprint(&runtime,digest);
    if (ok && checksum) ok=mcc_resources_checksum(&runtime,
        (runtime.report.external_bitmap_count ? 1u : 0u)|(runtime.report.external_sound_count ? 2u : 0u),checksum);
done:
    if (!ok && reason && reason_size) snprintf(reason,reason_size,"%s",runtime.failure[0] ? runtime.failure : problem);
    mcc_resources_dispose(&runtime);free(raw);
    return ok;
}

int mcc_resources_probe(char const *name,struct mcc_cache_source const *source,uint32_t *checksum)
{
    return mcc_resources_inspect(name,source,checksum,NULL,NULL,0);
}

int mcc_resources_probe_explain(char const *name,struct mcc_cache_source const *source,
    uint32_t *checksum,char *reason,size_t size)
{
    return mcc_resources_inspect(name,source,checksum,NULL,reason,size);
}

int mcc_resources_fingerprint_probe(char const *name,struct mcc_cache_source const *source,unsigned char digest[32])
{
    return mcc_resources_inspect(name,source,NULL,digest,NULL,0);
}

#include "mcc_resource_check.inl"

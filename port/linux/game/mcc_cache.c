/* Independent MCC cache lifetime and I/O. Source files are opened read-only.
 * Conversion uses exclusively MCC-owned memory and a separate tag window. */
#include "cseries.h"
#include "errors.h"
#include "cache/cache_files.h"
#include "scenario/scenario_definitions.h"
#include "interface/ui_widget.h"
#include "mcc_cache.h"
#include "mcc_identity.h"
#include "mcc_maps.h"
#include "mcc_runtime.h"
#include "mcc_resources.h"
#include "mcc_package.h"
#include "mcc_tag_validate.h"
#include "mcc_checkpoint.h"
#include "mcc_script_threads.h"
#include "mcc_object_names.h"
#include "mcc_grenades.h"
#include "mcc_script_parameters.h"
#include "mcc_campaign.h"
#include "mcc_texture_cache.h"
#include "mcc_ui_teams.h"
#include "mcc_hud_draw.h"
#include "mcc_loading.h"
#include "mcc_precache.h"
#include "mcc_validation.h"
#include "mcc_errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

void *mcc_memory_reserve(uint32_t address, uint32_t size);
void mcc_memory_release(void *allocation, uint32_t size);
int mcc_tags_prepare(struct mcc_runtime *runtime);
int mcc_scripts_prepare(struct mcc_runtime *runtime);
short mcc_geometry_part_palette(struct mcc_runtime *runtime, void const *buffer, byte const **nodes);

static struct mcc_runtime mcc_loaded_cache;
static FILE *mcc_stream;
static boolean mcc_loaded;

/* Menu selection, local admission and readiness ask about the same package.
 * Retain only its small validation result, never an open file or converted tags. */
static struct {
    char name[64];
    struct mcc_precache_snapshot files;
    uint32_t checksum;
} mcc_validation;

void mcc_cache_validation_clear(void)
{
    memset(&mcc_validation, 0, sizeof(mcc_validation));
}

static int mcc_cache_probe(char const *name, struct mcc_cache_source const *source, uint32_t *checksum,
    char *reason, size_t reason_size)
{
    struct mcc_precache_snapshot before, after;
    int stamped = mcc_precache_snapshot(name, &before);
    if (reason && reason_size) reason[0]=0;
    if (mcc_validation_lookup(name, checksum, NULL)) return 1;
    if (stamped && mcc_validation.checksum && !strcmp(name, mcc_validation.name) &&
        !memcmp(&before, &mcc_validation.files, sizeof(before))) {
        *checksum = mcc_validation.checksum;
        return 1;
    }
    mcc_cache_validation_clear();
    if (!mcc_resources_probe_explain(name, source, checksum, reason, reason_size)) return 0;
    if (stamped) {
        /* All candidates matter, including absent package/root overrides and
         * both shared sets. Never publish a result from an in-flight install. */
        if (!mcc_precache_snapshot(name, &after) || memcmp(&before, &after, sizeof(before))) {
            if (reason && reason_size) snprintf(reason,reason_size,
                "Its map or resources changed during validation. Finish installing and try again.");
            return 0;
        }
        if (strlen(name) < sizeof(mcc_validation.name)) {
            strcpy(mcc_validation.name, name);
            mcc_validation.files = after;
            mcc_validation.checksum = *checksum;
        }
    }
    return 1;
}

static int mcc_prefix(char const *name)
{
    static char const prefix[] = "mcc_maps";
    size_t i;
    if (!name)
        return FALSE;
    for (i = 0; i < sizeof(prefix) - 1; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
        if (c != (unsigned char)prefix[i]) return FALSE;
    }
    return name[i] == '\\' || name[i] == '/';
}

boolean mcc_level_name(char const *name)
{
    return mcc_prefix(name);
}

static int mcc_path(char const *name, char path[128])
{
    return mcc_package_map_path(name, path);
}

/* Checkpoints store the scenario tag's name, not the menu's level name.
 * Give this MCC instance its canonical namespace so Continue/reload resolves
 * the same file even when its internal scenario path names a stock level. */
static int mcc_scenario_identity(struct mcc_runtime *runtime, char const *name)
{
    char checked_path[128], canonical[64];
    char *identity;
    uint32_t address, index;
    size_t length;
    if (!runtime || !runtime->tag_index || !mcc_path(name, checked_path) ||
        !mcc_package_name(name, canonical)) return FALSE;
    index = runtime->report.scenario_handle & 0xFFFFu;
    if (index >= runtime->report.tag_count) return FALSE;
    name = canonical;
    length = strlen(name + sizeof(MCC_MAPS_LEVEL_PREFIX) - 1);
    identity = mcc_runtime_allocate(runtime, (uint32_t)(sizeof(MCC_MAPS_LEVEL_PREFIX) + length));
    if (!identity) return FALSE;
    memcpy(identity, MCC_MAPS_LEVEL_PREFIX, sizeof(MCC_MAPS_LEVEL_PREFIX) - 1);
    memcpy(identity + sizeof(MCC_MAPS_LEVEL_PREFIX) - 1,
        name + sizeof(MCC_MAPS_LEVEL_PREFIX) - 1, length + 1);
    address = (uint32_t)(uintptr_t)identity;
    mcc_diagnostic_tag(runtime,runtime->report.scenario_handle);
    snprintf(runtime->original_scenario_tag,sizeof(runtime->original_scenario_tag),"%s",runtime->diagnostic.tag);
    memcpy(runtime->tag_index + index * 32 + 16, &address, sizeof(address));
    return TRUE;
}

static int mcc_file_read(void *context, uint32_t offset, uint32_t bytes, void *out)
{
    FILE *stream = context;
    return stream && offset <= 0x7FFFFFFFu &&
        !fseek(stream, (long)offset, SEEK_SET) && fread(out, 1, bytes, stream) == bytes;
}

static FILE *mcc_open_explain(char const *name, struct mcc_cache_source *source,
    struct mcc_cache_identity *identity, char *reason, size_t reason_size)
{
    char path[128];
    unsigned char header[MCC_CACHE_HEADER_BYTES];
    FILE *stream;
    long size;
    enum mcc_cache_status status=MCC_CACHE_READ_FAILED;
    if (reason && reason_size) snprintf(reason,reason_size,"The MCC package or level name is invalid.");
    if (!mcc_path(name, path)) return NULL;
    stream = fopen(path, "rb");
    if (!stream) {
        if (reason && reason_size) snprintf(reason,reason_size, "%s", errno==ENOENT ?
            "Its .map file is missing. Install the complete level in its mcc_maps package folder." :
            "Its .map file could not be opened. Check that the file is accessible.");
        return NULL;
    }
    if (fseek(stream, 0, SEEK_END) || (size = ftell(stream)) < 0 ||
        !mcc_file_read(stream, 0, sizeof(header), header) ||
        (status=mcc_cache_identify(header, sizeof(header), (uint64_t)size, identity)) != MCC_CACHE_OK) {
        if (reason && reason_size) snprintf(reason,reason_size,"%s",mcc_status_explanation(status));
        fclose(stream);
        return NULL;
    }
    if (reason && reason_size) reason[0]=0;
    source->context = stream;
    source->read = mcc_file_read;
    source->size = (uint64_t)size;
    return stream;
}

static FILE *mcc_open(char const *name, struct mcc_cache_source *source,
    struct mcc_cache_identity *identity)
{
    return mcc_open_explain(name,source,identity,NULL,0);
}

#include "mcc_validation.inl"

boolean mcc_cache_available(char const *name)
{
    struct mcc_cache_identity identity;
    struct mcc_cache_source source;
    FILE *stream = mcc_open(mcc_identity_local_name(name), &source, &identity);
    if (!stream) return FALSE;
    fclose(stream);
    return TRUE;
}

unsigned long mcc_cache_checksum(char const *name)
{
    uint32_t checksum=0;
    name=mcc_identity_local_name(name);
    if (mcc_validation_request(name,&checksum,NULL)!=1) return 0;
    /* Initial host setup asks for the version before opening the lobby.
     * Prepare the identity with it, before the first settings send. Early
     * lobby map changes are deferred by mcc_ui_network until local admission.
     * Keep the existing checksum value. */
    if (checksum && !mcc_identity_prepare_host(name)) checksum=0;
    return checksum;
}

boolean mcc_cache_present(char const *name, unsigned long checksum, char *message, long size)
{
    struct mcc_cache_identity identity;
    struct mcc_cache_source source;
    FILE *stream;
    uint32_t actual;
    char reason[320];
    if (!mcc_identity_check(name)) {
        snprintf(message,(size_t)size,"The resolved MCC map or resources changed since admission. Rejoin with the host's matching package.");
        return FALSE;
    }
    name=mcc_identity_local_name(name);
    stream=mcc_open_explain(name,&source,&identity,reason,sizeof(reason));
    if (!stream) {
        mcc_error_message(name,reason,message,size>0 ? (size_t)size : 0);
        return FALSE;
    }
    if (!mcc_cache_probe(name,&source,&actual,reason,sizeof(reason))) {
        fclose(stream);
        mcc_error_message(name,reason,message,size>0 ? (size_t)size : 0);
        return FALSE;
    }
    fclose(stream);
    if (checksum && actual != checksum) {
        snprintf(message, (size_t)size, "The MCC map %.63s or its package resources differ from the host. Install the same complete package.", name);
        return FALSE;
    }
    return TRUE;
}

boolean mcc_cache_require(char const *name, unsigned long checksum)
{
    char message[512];
    wchar_t text[512];
    size_t i;
    if (mcc_cache_present(name, checksum, message, sizeof(message))) return TRUE;
    error(_error_silent, "mcc: %s", message);
    for (i = 0; message[i] && i < NUMBEROF(text) - 1; ++i) text[i] = (unsigned char)message[i];
    text[i] = 0;
    display_error_text_when_main_menu_loaded(text);
    return FALSE;
}

void *mcc_runtime_pointer(struct mcc_runtime *runtime, uint32_t address, uint32_t bytes)
{
    uint32_t offset = address - runtime->report.tag_base;
    if (address < runtime->report.tag_base || offset > runtime->used || bytes > runtime->used - offset)
        return NULL;
    return runtime->tags + offset;
}

int mcc_runtime_read(struct mcc_runtime *runtime, uint32_t offset, uint32_t bytes, void *out)
{
    return (uint64_t)offset <= runtime->source.size &&
        bytes <= runtime->source.size - offset &&
        runtime->source.read(runtime->source.context, offset, bytes, out);
}

void *mcc_runtime_allocate(struct mcc_runtime *runtime, uint32_t bytes)
{
    uint32_t limit = runtime->capacity, index;
    uint32_t start = (runtime->used + 15u) & ~15u;
    void *result;
    for (index = 0; index < runtime->report.bsp_count; ++index) {
        uint32_t bsp_offset = runtime->report.bsps[index].address - runtime->report.tag_base;
        if (bsp_offset < limit) limit = bsp_offset;
    }
    if (start < runtime->used || start > limit || bytes > limit - start) return NULL;
    result = runtime->tags + start;
    memset(result, 0, bytes);
    runtime->used = start + bytes;
    return result;
}

boolean mcc_cache_tags_loaded(void) { return mcc_loaded; }

short mcc_cache_scenario_type(void)
{
    return mcc_loaded ? (short)mcc_loaded_cache.report.identity.scenario_type : NONE;
}

boolean mcc_cache_contains(void const *address, long bytes)
{
    uintptr_t offset = (uintptr_t)address - (uintptr_t)mcc_loaded_cache.tags;
    return mcc_loaded && bytes > 0 && offset <= mcc_loaded_cache.capacity &&
        (uint32_t)bytes <= mcc_loaded_cache.capacity - offset;
}

void mcc_cache_tags_unload(void)
{
    mcc_validation_cancel();
    mcc_cache_validation_clear();
    mcc_cache_precache_clear();
    mcc_hud_glyphs_dispose();
    mcc_campaign_dispose();
    mcc_ui_teams_reset();
    mcc_script_threads_dispose();
    mcc_checkpoint_dispose();
    mcc_grenades_reset();
    mcc_object_names_clear();
    mcc_parameters_dispose();
    mcc_loaded = FALSE;
    mcc_texture_cache_dispose();
    mcc_geometry_dispose(&mcc_loaded_cache);
    mcc_audio_dispose(&mcc_loaded_cache);
    mcc_bitmaps_dispose(&mcc_loaded_cache);
    mcc_resources_dispose(&mcc_loaded_cache);
    mcc_validation_dispose(&mcc_loaded_cache);
    mcc_memory_release(mcc_loaded_cache.tags, mcc_loaded_cache.capacity);
    if (mcc_stream) fclose(mcc_stream);
    mcc_stream = NULL;
    memset(&mcc_loaded_cache, 0, sizeof(mcc_loaded_cache));
}

struct cache_file_tag_header *mcc_cache_tags_load(char const *name, void *header)
{
    struct mcc_runtime *runtime = &mcc_loaded_cache;
    struct mcc_cache_identity identity;
    unsigned char *raw = NULL;
    enum mcc_cache_status status;
    uint32_t package_checksum;
    char memory_problem[192],resource_paths[2][128];
    char const *problem = "the map could not be opened";
    mcc_cache_tags_unload();
    mcc_stream = mcc_open_explain(mcc_identity_local_name(name), &runtime->source, &identity,
        runtime->failure,sizeof(runtime->failure));
    if (!mcc_stream) goto failed;
    if (!mcc_package_name(mcc_identity_local_name(name), runtime->level_name)) goto failed;
    /* The virtual streams are disjoint from the on-disk file. */
    problem = "Its file is larger than the MCC loader supports.";
    if (identity.file_length >= 0x50000000u) goto failed;
    raw = malloc(identity.tag_data_size);
    problem = "Not enough memory to inspect its map data. Close other applications and try again.";
    if (!raw) goto failed;
    status = mcc_cache_load(&runtime->source, raw, identity.tag_data_size, &runtime->report);
    problem = mcc_status_explanation(status);
    if (status != MCC_CACHE_OK) {
        error(_error_silent,"mcc: '%s', %s: %s; map refused (tag index %lu, reader location 0x%08lx)",
            name ? name : "(map)",runtime->report.problem_path[0] ? runtime->report.problem_path : "map header",
            runtime->report.problem_detail,(unsigned long)runtime->report.problem_tag,
            (unsigned long)runtime->report.problem_location);
        goto failed;
    }
    problem = "Its MCC resources could not be selected. See debug.txt for the failing check.";
    if (!mcc_resources_select_paths(runtime,raw,
        mcc_validation_resources(runtime->level_name,resource_paths) ? resource_paths : NULL)) goto failed;
    mcc_loading_update(MCC_LOADING_RESOURCES, 1, 3);
    problem = "Its map resources could not be read completely. Check that the files are accessible.";
    if (!mcc_resources_checksum(runtime,
        (runtime->report.external_bitmap_count ? 1u : 0u)|(runtime->report.external_sound_count ? 2u : 0u),
        &package_checksum)) goto failed;
    mcc_loading_update(MCC_LOADING_RESOURCES, 2, 3);
    problem = "Its map or resources changed after joining. Rejoin with the host's matching complete package.";
    if (!mcc_identity_loaded(name,runtime)) goto failed;
    runtime->capacity = MCC_CACHE_TAG_CAPACITY;
    runtime->tags = mcc_memory_reserve(runtime->report.tag_base, runtime->capacity);
    if (!runtime->tags) {
        snprintf(memory_problem, sizeof(memory_problem),
            "the MCC tag address window at %08lx (%lu MiB) is unavailable: %s",
            (unsigned long)runtime->report.tag_base,
            (unsigned long)(runtime->capacity >> 20), strerror(errno));
        problem = memory_problem;
        snprintf(runtime->failure,sizeof(runtime->failure),
            "The memory address range needed by this MCC map is unavailable. Restart OpenCE and try again.");
        goto failed;
    }
    memcpy(runtime->tags, raw, identity.tag_data_size);
    runtime->used = identity.tag_data_size;
    runtime->tag_index = runtime->tags + 40;
    free(raw);
    raw = NULL;
    problem = "Its saved-game identity could not be prepared. See debug.txt for details.";
    if (!mcc_scenario_identity(runtime, name)) goto failed;
    problem = "Its map data could not be converted. Check for an updated map; debug.txt has the failing check.";
    mcc_loading_stage(MCC_LOADING_TAGS);
    mcc_diagnostic_tag(runtime,UINT32_MAX);
    if (!mcc_hud_bitmaps_prepare(runtime) || !mcc_tags_prepare(runtime)) goto failed;
    mcc_loading_stage(MCC_LOADING_SCRIPTS);
    problem = "Its scripts could not be prepared. Check for an updated map; debug.txt names the failing script check.";
    if (!mcc_scripts_prepare(runtime)) goto failed;
    mcc_loading_stage(MCC_LOADING_GEOMETRY);
    problem = "Its level geometry could not be prepared. See debug.txt for the failing geometry check.";
    if (!mcc_geometry_prepare(runtime)) goto failed;
    mcc_loading_stage(MCC_LOADING_AUDIO);
    problem = "Its sounds could not be converted. See debug.txt for the failing sound check.";
    if (!mcc_audio_prepare(runtime)) goto failed;
    mcc_loading_stage(MCC_LOADING_BITMAPS);
    problem = "Its textures could not be converted. See debug.txt for the failing texture check.";
    if (!mcc_bitmaps_prepare(runtime)) goto failed;
    /* The runtime tag table is stock-shaped. The independent wire reader's
     * report retains model offsets; no other loader sees an MCC header. */
    memset(runtime->tags + 16, 0, 20);
    memcpy(runtime->tags, &runtime->tag_index, 4);
    memcpy(runtime->tags + 12, &runtime->report.tag_count, 4);
    memcpy(runtime->tags + 32, "sgat", 4);
    problem = "Its converted map data failed validation. Check for an updated map; debug.txt has the failing check.";
    mcc_loading_stage(MCC_LOADING_VALIDATION);
    if (!mcc_tags_validate(runtime)) goto failed;
    if (!mcc_runtime_read(runtime, 0, MCC_CACHE_HEADER_BYTES, header)) {
        problem = "Its map header could not be read. Check that the complete file is accessible.";
        goto failed;
    }
    /* The cache header checksum feeds native save validation as well as the lobby. */
    memcpy((unsigned char *)header + 0x64, &package_checksum, 4);
    problem = "Not enough memory to prepare its scripts. Close other applications and try again.";
    if (!mcc_script_threads_prepare()) {
        mcc_diagnostic_tag(runtime,runtime->report.scenario_handle);
        mcc_diagnostic_at(runtime,"script thread storage");
        mcc_diagnostic_fail(runtime,"not enough memory to allocate script thread state");
        goto failed;
    }
    mcc_loaded = TRUE;
    mcc_loading_stage(MCC_LOADING_SCENARIO);
    mcc_bitmaps_prepare_gpu(runtime);
    mcc_campaign_begin();
    error(_error_silent, "mcc: loaded %s (%lu tags, %lu bytes of runtime tags)", name,
        (unsigned long)runtime->report.tag_count, (unsigned long)runtime->used);
    return (struct cache_file_tag_header *)runtime->tags;
failed:
    error(_error_silent, "mcc: refused %s: %s", name ? name : "(unnamed)", problem);
    if (runtime->failure[0]) error(_error_silent,"mcc: %s",runtime->failure);
    {
        wchar_t text[512];
        mcc_error_text(mcc_identity_local_name(name),runtime->failure[0] ? runtime->failure : problem,text,NUMBEROF(text));
        display_error_text_when_main_menu_loaded(text);
    }
    if (raw) free(raw);
    mcc_cache_tags_unload();
    return NULL;
}

boolean mcc_cache_read(long tag, long offset, long bytes, void *out)
{
    int result;
    if (!mcc_loaded || offset < 0 || bytes < 0 || !out) return FALSE;
    result = mcc_bitmaps_read(&mcc_loaded_cache, tag, (uint32_t)offset, (uint32_t)bytes, out);
    if (result < 0) result = mcc_audio_read(&mcc_loaded_cache, tag, (uint32_t)offset, (uint32_t)bytes, out);
    if (result < 0) result = mcc_runtime_read(&mcc_loaded_cache, (uint32_t)offset, (uint32_t)bytes, out);
    if (!result) {
        memset(out, 0, (size_t)bytes);
        error(_error_silent, "mcc: I/O failed at %08lx (%ld bytes)", (unsigned long)offset, bytes);
    }
    return result ? TRUE : FALSE;
}

boolean mcc_cache_bsp_load(struct scenario_structure_bsp_reference const *reference,
    void **header, void **structure)
{
    uint32_t i;
    for (i = 0; mcc_loaded && i < mcc_loaded_cache.report.bsp_count; ++i) {
        struct mcc_cache_bsp const *bsp = &mcc_loaded_cache.report.bsps[i];
        if (bsp->tag_handle != (uint32_t)reference->structure_bsp.index) continue;
        if (bsp->address != (uint32_t)(uintptr_t)reference->base_address ||
            bsp->file_offset != (uint32_t)reference->file_offset || bsp->size != (uint32_t)reference->file_size)
            return FALSE;
        if (!mcc_geometry_bsp_load(&mcc_loaded_cache, bsp->tag_handle, bsp->file_offset,
            bsp->size, reference->base_address, structure)) return FALSE;
        *header = reference->base_address;
        if (!mcc_bsp_validate(&mcc_loaded_cache, (long)bsp->tag_handle, *header, (long)bsp->size)) {
            mcc_geometry_bsp_unload(&mcc_loaded_cache);
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

void mcc_cache_bsp_unload(void) { mcc_geometry_bsp_unload(&mcc_loaded_cache); }

short mcc_cache_part_palette(void const *buffer, byte const **nodes)
{
    return mcc_loaded ? mcc_geometry_part_palette(&mcc_loaded_cache, buffer, nodes) : 0;
}

boolean mcc_cache_bitmap_valid(struct bitmap_data *bitmap)
{
    extern int mcc_bitmaps_valid(struct mcc_runtime *runtime, struct bitmap_data *bitmap);
    return mcc_loaded && mcc_bitmaps_valid(&mcc_loaded_cache, bitmap);
}

void const *mcc_cache_bitmap_pixels(struct bitmap_data const *bitmap,unsigned long *bytes,int *coverage)
{
    uint32_t size=0;
    void const *pixels;
    if (coverage) *coverage=0;
    pixels=mcc_loaded ? mcc_bitmaps_pixels(&mcc_loaded_cache,bitmap,&size,coverage) : NULL;
    if (bytes) *bytes=size;
    return pixels;
}

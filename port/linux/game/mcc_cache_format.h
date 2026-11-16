/* Independent reader for Halo 1 MCC version-13 cache files.
 * Wire layouts are described in docs/mcc_maps.md. No Xbox or Custom Edition
 * loader is called, modified, or used as an alternate interpretation. */
#ifndef MCC_CACHE_FORMAT_H
#define MCC_CACHE_FORMAT_H

#include <stddef.h>
#include <stdint.h>

#define MCC_CACHE_HEADER_BYTES 0x800u
#define MCC_CACHE_VERSION 13u
#define MCC_CACHE_TAG_CAPACITY 0x04000000u
#define MCC_CACHE_MAX_BSPS 32u

enum mcc_cache_status {
    MCC_CACHE_OK = 0,
    MCC_CACHE_BAD_ARGUMENT,
    MCC_CACHE_READ_FAILED,
    MCC_CACHE_BAD_HEADER,
    MCC_CACHE_WRONG_VERSION,
    MCC_CACHE_BAD_FILE_SIZE,
    MCC_CACHE_BAD_STRING,
    MCC_CACHE_BAD_SCENARIO_TYPE,
    MCC_CACHE_BAD_TAG_RANGE,
    MCC_CACHE_NO_MEMORY,
    MCC_CACHE_BAD_TAG_INDEX,
    MCC_CACHE_BAD_TAG,
    MCC_CACHE_BAD_SCENARIO,
    MCC_CACHE_BAD_MODEL_RANGE,
    MCC_CACHE_BAD_BSP,
    MCC_CACHE_BAD_BITMAP,
    MCC_CACHE_BAD_SOUND,
    MCC_CACHE_BAD_SCRIPT,
    MCC_CACHE_EXTERNAL_TAG
};

struct mcc_cache_identity {
    uint64_t file_size;
    uint32_t file_length, tag_data_offset, tag_data_size, checksum;
    uint16_t scenario_type, flags;
    char name[32], build[32];
};

/* All reads are absolute file offsets; a successful read fills every byte. */
struct mcc_cache_source {
    void *context;
    int (*read)(void *context, uint32_t offset, uint32_t size, void *out);
    uint64_t size;
};

struct mcc_resource_reference {
    char const *tag_name;
    unsigned type;
    uint32_t first, second, offset, bytes;
};

struct mcc_cache_bsp {
    uint32_t file_offset, size, address, tag_handle;
    uint32_t structure_address;
    uint32_t vertex_file_offset, vertex_bytes;
};

struct mcc_cache_report {
    struct mcc_cache_identity identity;
    enum mcc_cache_status status;
    uint32_t problem_tag, problem_location;
    char problem_path[320], problem_detail[384];
    uint32_t tag_base, tag_count, scenario_handle;
    uint32_t model_file_offset, model_vertex_bytes, model_bytes;
    uint32_t bsp_count, largest_bsp_bytes, bsp_material_count;
    uint32_t bitmap_count, bc7_bitmap_count, external_bitmap_count;
    uint32_t sound_permutation_count, external_sound_count;
    uint32_t model_count, generic_shader_count;
    uint32_t script_count, global_count, script_node_count;
    uint32_t parameterized_script_count;
    struct mcc_cache_bsp bsps[MCC_CACHE_MAX_BSPS];
};

/* Fast menu probe: needs only the first 2048 bytes and the actual file length.
 * A successful probe identifies a map; it does NOT certify it can run. */
enum mcc_cache_status mcc_cache_identify(void const *header, size_t header_bytes,
    uint64_t file_size, struct mcc_cache_identity *identity);

/* Load raw tags without changing their wire contents. Caller owns the buffer;
 * it need not be at the map's linked address for inspection. The index,
 * scenario, BSP geometry and raw asset ranges are checked independently.
 * Runtime conversion and engine tag validation remain separate obligations. */
enum mcc_cache_status mcc_cache_load(struct mcc_cache_source const *source,
    void *tag_data, size_t capacity, struct mcc_cache_report *report);

/* Same audit using a temporary tag buffer, freed before this call returns. */
enum mcc_cache_status mcc_cache_inspect(struct mcc_cache_source const *source,
    struct mcc_cache_report *report);

/* Return a validated slice within loaded raw tags, or NULL. */
void *mcc_cache_pointer(void *tag_data, struct mcc_cache_report const *report,
    uint32_t address, uint32_t bytes);
/* Visit external payloads in validated, unconverted tags. */
int mcc_cache_visit_resources(void *tag_data, struct mcc_cache_report const *report,
    int (*visit)(void *, struct mcc_resource_reference const *), void *context);
char const *mcc_cache_status_string(enum mcc_cache_status status);

#endif

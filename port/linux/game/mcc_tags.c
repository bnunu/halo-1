/* MCC metadata normalization. All writes are to an MCC-owned, checked slice;
 * the original map and both established format implementations are untouched. */
#include "cseries.h"
#include "errors.h"
#include "mcc_runtime.h"
#include "mcc_recordings.h"
#include <float.h>
#include <string.h>

static uint32_t mcc_word(void const *p)
{
    uint32_t value;
    memcpy(&value, p, 4);
    return value;
}

static void *mcc_block(struct mcc_runtime *r, unsigned char *p, uint32_t stride, uint32_t *count)
{
    *count = mcc_word(p);
    if (*count > r->used / stride) return NULL;
    return *count ? mcc_runtime_pointer(r, mcc_word(p + 4), *count * stride) : NULL;
}

static int mcc_scenario_recordings(struct mcc_runtime *r, unsigned char *scenario)
{
    uint32_t count, i;
    unsigned char *recordings = mcc_block(r, scenario + 0x36C, 0x40, &count);
    if (count && !recordings) return mcc_diagnostic_fail(r,"recorded_animations block is outside loaded tag data (count %lu)",(unsigned long)count);
    for (i = 0; i < count; ++i) {
        unsigned char *recording = recordings + i * 0x40;
        uint32_t size = mcc_word(recording + 0x2C), corrected,bad_offset;
        char const *reason;
        unsigned char *stream = mcc_runtime_pointer(r, mcc_word(recording + 0x38), size);
        mcc_diagnostic_at(r,"recorded_animations[%lu] '%.32s'",(unsigned long)i,(char const *)recording);
        /* Preserve an explicitly disabled, empty recording. */
        if (!recording[0x20] && !recording[0x24] && !recording[0x25] &&
            !size && !mcc_word(recording + 0x38)) continue;
        if (!mcc_recording_prepare_ex(stream, size, recording[0x20], recording[0x22], &corrected,&reason,&bad_offset)) {
            return mcc_diagnostic_fail(r,"%s at stream byte %lu (version %u, unit version %u, size %lu)",reason,(unsigned long)bad_offset,recording[0x20],recording[0x22],(unsigned long)size);
        }
        if (mcc_recording_weapon_only(stream, size, recording[0x20], recording[0x22]))
            error(_error_silent, "mcc: recorded animation #%lu: weapon-only controls will inherit the unit's current directions",
                (unsigned long)i);
        if (corrected) error(_error_silent,
            "mcc: recorded animation #%lu: %lu invalid weapon selections changed to NONE",
            (unsigned long)i, (unsigned long)corrected);
    }
    return TRUE;
}

static int mcc_scenario_cameras(struct mcc_runtime *r, unsigned char *scenario)
{
    uint32_t count, i;
    unsigned char *points = mcc_block(r, scenario + 0x4F0, 0x68, &count);
    float const native_maximum = 1.5707963267948966f;
    if (count && !points) return mcc_diagnostic_fail(r,"cutscene_camera_points block is outside loaded tag data (count %lu)",(unsigned long)count);
    for (i = 0; i < count; ++i) {
        unsigned char *field = points + i * 0x68 + 0x40;
        float angle;
        memcpy(&angle, field, 4);
        mcc_diagnostic_at(r,"cutscene_camera_points[%lu] '%.32s'.field_of_view",(unsigned long)i,(char const *)(points+i*0x68+4));
        /* Zero selects the native default. Wide MCC cinematic lenses are
         * valid authored data, but the native observer both validates and
         * clamps to 90 degrees. Normalize the MCC copy before commands reach
         * it; the Xbox/CE camera and its validation remain unchanged. */
        if (angle == 0.0f) continue;
        if (!(angle >= 0.001f && angle < 3.1415926535897932f)) {
            return mcc_diagnostic_fail(r,"field of view %g radians must be zero or between 0.001 and pi",(double)angle);
        }
        if (angle > native_maximum) {
            memcpy(field, &native_maximum, 4);
            error(_error_silent, "mcc: cutscene camera #%lu: field of view limited to 90 degrees",
                (unsigned long)i);
        }
    }
    return TRUE;
}

static int mcc_hud(struct mcc_runtime *r, uint32_t group, unsigned char *p)
{
    unsigned char *block;
    uint32_t count, i;
    /* mcc_hud_bitmaps_prepare has already normalized every placement and
     * consumed its high-resolution flag. Only meter metadata remains here. */
    if (group == 'unhi') {
        memset(p + 0xF4 + 0x58, 0, 12);
        memset(p + 0x1E4 + 0x58, 0, 12);
        mcc_diagnostic_at(r,"auxiliary_meters");
        block = mcc_block(r, p + 0x3CC, 0x144, &count);
        if (count && !block) return mcc_diagnostic_fail(r,"HUD placement block is outside loaded tag data (count %lu)",(unsigned long)count);
        for (i = 0; i < count; ++i) {
            memset(block + i * 0x144 + 0x7C + 0x58, 0, 12);
        }
    } else if (group == 'wphi') {
        mcc_diagnostic_at(r,"meter_elements");
        block = mcc_block(r, p + 0x6C, 0xB4, &count);
        if (count && !block) return mcc_diagnostic_fail(r,"HUD placement block is outside loaded tag data (count %lu)",(unsigned long)count);
        for (i = 0; i < count; ++i) memset(block + i * 0xB4 + 0x24 + 0x58, 0, 12);
    }
    return TRUE;
}

int mcc_tags_prepare(struct mcc_runtime *r)
{
    uint32_t i;
    for (i = 0; i < r->report.tag_count; ++i) {
        unsigned char *entry = r->tag_index + i * 32;
        uint32_t group = mcc_word(entry), address = mcc_word(entry + 20), size = 0;
        unsigned char *p;
        short shader = -1;
        switch (group) {
        case 'senv': shader = 3; break;
        case 'soso': shader = 4; break;
        case 'sotr': shader = 5; break;
        case 'schi': case 'scex': shader = 6; break;
        case 'swat': shader = 7; break;
        case 'sgla': shader = 8; break;
        case 'smet': shader = 9; break;
        case 'spla': shader = 10; break;
        case 'unhi': size = 0x56C; break;
        case 'wphi': size = 0x17C; break;
        case 'grhi': size = 0x1F8; break;
        case 'hudg': size = 0x450; break;
        case 'weap': size = 0x508; break;
        case 'DeLa': size = 0x60; break;
        case 'scnr': size = 0x5B0; break;
        case 'jpt!': size = 0x2A0; break;
        default: break;
        }
        if (shader >= 0) size = group == 'scex' ? 0x78 : 0x28;
        if (!size) continue;
        mcc_diagnostic_tag(r,mcc_word(entry+12));
        mcc_diagnostic_at(r,"metadata");
        p = mcc_runtime_pointer(r, address, size);
        if (!p) {
            return mcc_diagnostic_fail(r,"tag header at 0x%08lx requires %lu bytes outside loaded data",(unsigned long)address,(unsigned long)size);
        }
        if (shader >= 0) memcpy(p + 0x24, &shader, 2);
        if (group == 'jpt!' && (mcc_word(p + 0x1C8) & 0x30000u)) {
            unsigned j, count = mcc_word(p + 0x1C8) & 0x10000u ? 3 : 1;
            for (j = 0; j < count; ++j) {
                float acceleration;
                memcpy(&acceleration, p + 0x1F4 + j * 4, 4);
                if (!(acceleration >= -FLT_MAX && acceleration <= FLT_MAX)) {
                    return mcc_diagnostic_fail(r,"acceleration[%u] is not finite (%g)",j,(double)acceleration);
                }
            }
        }
        if (group == 'scnr' && (!mcc_scenario_recordings(r, p) || !mcc_scenario_cameras(r, p))) return FALSE;
        if (group == 'scex') {
            uint32_t flags = mcc_word(p + 0x6C), new_group = 'schi';
            if (!mcc_word(p + 0x54)) memcpy(p + 0x54, p + 0x60, 12);
            memset(p + 0x60, 0, 12);
            memcpy(p + 0x60, &flags, 4);
            memcpy(entry, &new_group, 4);
        }
        if (group == 'weap') {
            unsigned j;
            uint16_t mode;
            float forward, sideways;
            memcpy(&mode, p + 0x3FC, 2);
            memcpy(&forward, p + 0x400, 4);
            memcpy(&sideways, p + 0x404, 4);
            /* Do not silently drop authored movement rules, or admit values
             * which expand the native host's validated speed envelope. */
            if (mode > 2 || !(forward >= 0.0f && forward <= 2.0f) ||
                !(sideways >= 0.0f && sideways <= 2.0f)) {
                return mcc_diagnostic_fail(r,"movement penalties: mode %u must be 0..2; forward %g and sideways %g must be finite and within 0..2",(unsigned)mode,(double)forward,(double)sideways);
            }
            for (j = 0; j < 4; ++j) {
                uint16_t value;
                memcpy(&value, p + 0x330 + j * 2, 2);
                if (value == 17 || value == 18) {
                    value -= 2;
                    memcpy(p + 0x330 + j * 2, &value, 2);
                }
            }
        }
        if (group == 'DeLa') {
            uint32_t count;
            unsigned char *events = mcc_block(r, p + 0x54, 0x48, &count);
            if (count && !events) return mcc_diagnostic_fail(r,"event_handlers block is outside loaded tag data (count %lu)",(unsigned long)count);
            /* The first 102 callback IDs retain their Xbox meanings. MCC's
             * additional callbacks are interpreted by mcc_ui.c only for an
             * MCC-owned widget. Preserve the handlers, including mouse input,
             * confirmation actions, scripts and their navigation flags. */
        }
        if (!mcc_hud(r, group, p)) return FALSE;
    }
    return TRUE;
}

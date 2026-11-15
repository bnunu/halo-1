/* Independently written MCC cache normalization. Solar Flare M1's pilot
 * recordings contain 0xFF00 in the initial weapon field. Passing that signed
 * short (-256) to the Xbox interpreter indexes outside the unit's inventory.
 * There is no valid slot to translate it to: retain the current weapon instead.
 * Apply the same rule to later selection events, without changing their timing,
 * movement or flags. This runs only on the MCC loader's private tag buffer. */
#include "mcc_recordings.h"

static unsigned mcc_recording_word(unsigned char const *p)
{
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}

static uint32_t mcc_recording_weapon(unsigned char *p, int write)
{
    unsigned value = mcc_recording_word(p);
    /* Xbox units have four slots; 0xFFFF requests no selection change. */
    if (value < 4 || value == 0xFFFF) return 0;
    if (write) p[0] = p[1] = 0xFF;
    return 1;
}

static int mcc_recording_walk(unsigned char *stream, uint32_t size,
    unsigned version, unsigned control_version, int write, uint32_t *corrected,
    int *weapon_only,char const **reason,uint32_t *bad_offset)
{
    uint32_t offset = 52, changes;
    unsigned i;
    int stationary, weapon_event = 0;
    *bad_offset=0;
    *reason="recording data is outside loaded tags";
    if (!stream) return 0;
    *reason="recording version must be 1..4 and unit control version 0..4";
    if (version < 1 || version > 4 || control_version > 4) return 0;
    for (i = 1; i < control_version; ++i) offset += i == 1 ? 4 : 2;
    if (version == 4) offset += 12; /* facing/aiming/looking angle state */
    *reason="recording initial control state is truncated";
    if (offset > size) return 0;
    stationary = weapon_only && version == 4 && control_version == 4 && !mcc_recording_word(stream + 2);
    /* Tiny Halo's weapon-only recordings omit both throttle and directions.
     * Accept signed zero, but not nonzero, nonfinite or partial directions. */
    for (i = 8; stationary && i < 52; i += 4)
        if (stream[i] || stream[i+1] || stream[i+2] || (stream[i+3] & 127)) stationary = 0;
    changes = mcc_recording_weapon(stream + 4, write);
    while (offset < size) {
        uint32_t header_size, payload_size;
        unsigned type;
        *bad_offset=offset;
        *reason="recording event header is truncated";
        if (version == 4) {
            unsigned time = stream[offset] & 3;
            type = stream[offset] >> 2;
            header_size = time < 2 ? 1 : time;
            payload_size = type < 2 ? 0 : type < 4 ? 1 : type < 6 ? 2 :
                type == 6 ? 8 : type < 15 ? 2 : 4;
        } else {
            if (size - offset < 4) return 0;
            type = mcc_recording_word(stream + offset);
            header_size = 4;
            payload_size = type < 2 || type == 7 || type == 8 ? 0 :
                type < 6 ? 2 : type == 6 || type >= 16 ? 8 : 12;
        }
        if (header_size > size - offset) return 0;
        *reason="recording event type is outside 0..22";
        if (type >= 23) return 0;
        offset += header_size;
        *reason="recording event payload is truncated";
        if (payload_size > size - offset) return 0;
        if (type == 1) {
            *corrected = changes;
            if (weapon_only) *weapon_only = stationary && weapon_event;
            return 1;
        }
        if (type == 5) {
            changes += mcc_recording_weapon(stream + offset, write);
            weapon_event = 1;
        } else if (type != 0) stationary = 0;
        offset += payload_size;
    }
    *bad_offset=offset;
    *reason="recording has no end event";
    return 0;
}

int mcc_recording_prepare_ex(unsigned char *stream, uint32_t size,
    unsigned version, unsigned control_version, uint32_t *corrected,
    char const **reason,uint32_t *bad_offset)
{
    uint32_t changes;
    *corrected = 0;
    if (!mcc_recording_walk(stream, size, version, control_version, 0, &changes, 0,reason,bad_offset)) return 0;
    if (changes) return mcc_recording_walk(stream, size, version, control_version, 1, corrected, 0,reason,bad_offset);
    return 1;
}

int mcc_recording_prepare(unsigned char *stream, uint32_t size,
    unsigned version, unsigned control_version, uint32_t *corrected)
{
    char const *reason;
    uint32_t bad_offset;
    return mcc_recording_prepare_ex(stream,size,version,control_version,corrected,&reason,&bad_offset);
}

int mcc_recording_weapon_only(unsigned char *stream, uint32_t size,
    unsigned version, unsigned control_version)
{
    uint32_t changes,bad_offset;
    char const *reason;
    int weapon_only = 0;
    return mcc_recording_walk(stream, size, version, control_version, 0, &changes,
        &weapon_only,&reason,&bad_offset) && weapon_only;
}

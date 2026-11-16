/* MCC audio normalization. An MCC-owned Vorbis adapter handles codec differences;
 * no Custom Edition state or sound conversion routine is used. A damaged or
 * unsupported permutation rejects the map, rather than quietly silencing it. */
#include "cseries.h"
#include "errors.h"
#include "sound/sound_definitions.h"
#include "mcc_runtime.h"
#include "mcc_resources.h"
#include "mcc_vorbis.h"
#include "mcc_loading.h"
#include <stdlib.h>
#include <string.h>

#define MCC_AUDIO_BASE 0x50000000u
#define MCC_AUDIO_LIMIT 0x10000000u
#define MCC_AUDIO_STORAGE_LIMIT 0x40000000u
#define MCC_AUDIO_FRAME_LIMIT 0x1000000u

struct mcc_audio_storage { unsigned char *bytes; uint32_t used, capacity, limit, handle; char const *failure; };
/* Cache reads carry the owning sound tag. Its virtual offsets can therefore
 * be reused by other sounds, just as MCC bitmap offsets are scoped by tag. */
struct mcc_audio_tags {
    struct mcc_audio_storage *items;
    uint32_t count,used,capacity,stream_end;
};
struct mcc_pcm { short *samples; uint32_t frames, rate; int channels, vorbis_owned; char const *failure; int decoder_error; };
struct mcc_ima { int value, step; };

/* The IMA ADPCM quantizer tables are part of the public codec format. */
static int const mcc_ima_steps[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,
    80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,
    494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
    2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,
    8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,
    27086,29794,32767
};
static int const mcc_ima_changes[8] = {-1,-1,-1,-1,2,4,6,8};

static int mcc_ima_advance(struct mcc_ima *state, unsigned code)
{
    int quantizer=mcc_ima_steps[state->step];
    int delta=quantizer/8;
    if (code&1) delta+=quantizer/4;
    if (code&2) delta+=quantizer/2;
    if (code&4) delta+=quantizer;
    state->value+=(code&8) ? -delta : delta;
    state->value=PIN(state->value,-32768,32767);
    state->step=PIN(state->step+mcc_ima_changes[code&7],0,88);
    return state->value;
}

static unsigned mcc_ima_quantize(struct mcc_ima *state, int sample)
{
    int error=sample-state->value, divisor=mcc_ima_steps[state->step];
    unsigned code=error<0 ? 8 : 0, bit;
    if (error<0) error=-error;
    for (bit=4; bit; bit>>=1,divisor>>=1) {
        if (error>=divisor) {code|=bit; error-=divisor;}
    }
    mcc_ima_advance(state,code);
    return code;
}

static int mcc_pcm_open(unsigned char const *bytes, uint32_t size, int compression,
    int channels, uint32_t rate, struct mcc_pcm *pcm)
{
    uint32_t i;
    memset(pcm,0,sizeof(*pcm));
    pcm->channels=channels; pcm->rate=rate;
    if (compression==3) {
        pcm->vorbis_owned=1;
        pcm->failure="Ogg Vorbis decoding failed: invalid or unsupported stream, frame limit, or decoder allocation failure";
        return mcc_vorbis_decode_ex(bytes,size,MCC_AUDIO_FRAME_LIMIT,
            &pcm->samples,&pcm->frames,&pcm->rate,&pcm->channels,&pcm->failure,&pcm->decoder_error);
    }
    if (compression==0) {
        if (size%(2*channels)) {pcm->failure="PCM byte count is not a whole 16-bit sample frame";return 0;}
        pcm->frames=size/(2*channels);
    } else if (compression==1) {
        if (size%(36*channels)) {pcm->failure="Xbox ADPCM byte count is not a whole 36-byte block per channel";return 0;}
        pcm->frames=size/(36*channels)*64;
    } else {pcm->failure="unsupported sound codec (supported: 0=16-bit PCM, 1=Xbox ADPCM, 3=Ogg Vorbis)";return 0;}
    if (!pcm->frames || pcm->frames>MCC_AUDIO_FRAME_LIMIT) {pcm->failure="decoded frame count is zero or exceeds 16777216 frames";return 0;}
    pcm->samples=malloc(pcm->frames*channels*sizeof(short));
    if (!pcm->samples) {pcm->failure="not enough memory for decoded sound samples";return 0;}
    if (!compression) {
        for (i=0;i<pcm->frames*(uint32_t)channels;i++)
            pcm->samples[i]=(short)(bytes[i*2]|bytes[i*2+1]<<8);
    } else {
        for (i=0;i<pcm->frames/64;i++) {
            int channel;
            unsigned char const *block=bytes+i*36*channels;
            for (channel=0;channel<channels;channel++) {
                struct mcc_ima state;
                uint32_t frame;
                state.value=(short)(block[channel*4]|block[channel*4+1]<<8);
                state.step=block[channel*4+2];
                if (state.step>88 || block[channel*4+3]) {pcm->failure="invalid Xbox ADPCM block header (step index exceeds 88 or reserved byte is nonzero)";return 0;}
                for (frame=0;frame<64;frame++) {
                    unsigned char packed=block[channels*4+(frame/8*channels+channel)*4+frame%8/2];
                    unsigned nibble=(packed>>(frame%2*4))&15;
                    pcm->samples[(i*64+frame)*channels+channel]=(short)mcc_ima_advance(&state,nibble);
                }
            }
        }
    }
    return 1;
}

static void mcc_pcm_close(struct mcc_pcm *pcm)
{
    /* mcc_vorbis uses the C runtime allocator; cseries remaps this module's free
     * to debug_free. Preserve ownership even after the permutation is converted
     * to ADPCM, so each allocation returns to the allocator that created it. */
    if (pcm->samples) {
        if (pcm->vorbis_owned) mcc_vorbis_free(pcm->samples);
        else free(pcm->samples);
    }
    pcm->samples=NULL;
}

static int mcc_pcm_sample(struct mcc_pcm const *pcm, uint32_t frame, int channel, int channels)
{
    if (pcm->channels==1) return pcm->samples[frame];
    if (channels==1) return ((int)pcm->samples[frame*2]+pcm->samples[frame*2+1])/2;
    return pcm->samples[frame*2+channel];
}

static int mcc_resampled(struct mcc_pcm const *pcm, uint32_t frame, int channel, int channels, uint32_t rate)
{
    uint64_t position=(uint64_t)frame*pcm->rate;
    uint32_t first=(uint32_t)(position/rate), fraction=(uint32_t)(position%rate), next;
    int a,b;
    if (first>=pcm->frames) first=pcm->frames-1;
    next=first+1<pcm->frames ? first+1 : first;
    a=mcc_pcm_sample(pcm,first,channel,channels);
    b=mcc_pcm_sample(pcm,next,channel,channels);
    return a+(int)((int64_t)(b-a)*fraction/rate);
}

/* The engine's tracked allocator requires each allocation to be strictly
 * below 256 MiB. Rounding the final growth step to that boundary used to
 * assert before the converter could report an exhausted virtual stream. */
static int mcc_audio_reserve(struct mcc_audio_storage *storage,uint32_t size)
{
    uint32_t capacity;
    unsigned char *larger;
    if (storage->limit>=MCC_AUDIO_LIMIT || storage->used>storage->limit || size>storage->limit-storage->used) {
        storage->failure="audio storage limit exceeded (less than 256 MiB per sound; 1 GiB total)";return 0;
    }
    if (storage->used+size<=storage->capacity) return 1;
    capacity=MIN((storage->used+size+0xFFF)&~0xFFFu,storage->limit);
    larger=malloc(capacity);
    if (!larger) {storage->failure="not enough memory to grow the converted sound buffer";return 0;}
    if (storage->used) memcpy(larger,storage->bytes,storage->used);
    if (storage->bytes) free(storage->bytes);
    storage->bytes=larger;storage->capacity=capacity;
    return 1;
}

static int mcc_audio_store(struct mcc_audio_storage *storage, struct mcc_pcm const *pcm,
    int channels, uint32_t rate, struct sound_permutation *permutation)
{
    uint64_t frames64=((uint64_t)pcm->frames*rate+pcm->rate-1)/pcm->rate;
    uint32_t frames,blocks,size,block;
    struct mcc_ima states[2]={{0,0},{0,0}};
    if (!frames64 || frames64>MCC_AUDIO_FRAME_LIMIT) {storage->failure="resampled sound exceeds the 16777216-frame limit or is empty";return 0;}
    frames=(uint32_t)frames64; blocks=(frames+63)/64; size=blocks*36*channels;
    if (!mcc_audio_reserve(storage,size)) return 0;
    for (block=0;block<blocks;block++) {
        unsigned char *out=storage->bytes+storage->used+block*36*channels;
        int channel;
        if (!(block & 255u)) mcc_loading_pulse();
        for (channel=0;channel<channels;channel++) {
            uint32_t pair;
            out[channel*4]=(unsigned char)states[channel].value;
            out[channel*4+1]=(unsigned char)(states[channel].value>>8);
            out[channel*4+2]=(unsigned char)states[channel].step;
            out[channel*4+3]=0;
            for (pair=0;pair<32;pair++) {
                uint32_t f=block*64+pair*2;
                unsigned lo=mcc_ima_quantize(&states[channel],mcc_resampled(pcm,MIN(f,frames-1),channel,channels,rate));
                unsigned hi=mcc_ima_quantize(&states[channel],mcc_resampled(pcm,MIN(f+1,frames-1),channel,channels,rate));
                out[channels*4+(pair/4*channels+channel)*4+pair%4]=(unsigned char)(lo|(hi<<4));
            }
        }
    }
    permutation->samples.file_offset=MCC_AUDIO_BASE+storage->used;
    permutation->samples.size=size;
    permutation->sample_buffer_size=0;
    permutation->compression=1;
    storage->used+=size;
    return 1;
}

/* Keep validated external ADPCM byte-for-byte, avoiding a lossy round trip. */
static int mcc_audio_store_raw(struct mcc_audio_storage *storage,
    unsigned char const *bytes,struct sound_permutation *permutation)
{
    uint32_t size=(uint32_t)permutation->samples.size;
    if (!mcc_audio_reserve(storage,size)) return 0;
    memcpy(storage->bytes+storage->used,bytes,size);
    permutation->samples.file_offset=MCC_AUDIO_BASE+storage->used;
    permutation->samples.pad&=~1;
    storage->used+=size;
    return 1;
}

static char const *mcc_audio_codec_name(long codec)
{
    switch (codec) {
    case 0: return "16-bit PCM";
    case 1: return "Xbox ADPCM";
    case 2: return "IMA ADPCM (unsupported)";
    case 3: return "Ogg Vorbis";
    default: return "unknown";
    }
}

int mcc_audio_prepare(struct mcc_runtime *runtime)
{
    struct mcc_audio_tags *audio;
    struct mcc_audio_storage *storage;
    uint32_t index,converted=0;
    long failed_codec=NONE,failed_size=0;
    uint32_t failed_offset=0;
    char const *stage="sound header", *tag_name="(unavailable)";
    mcc_diagnostic_tag(runtime,UINT32_MAX);
    mcc_diagnostic_at(runtime,"sound conversion setup");
    if (runtime->source.size>=MCC_AUDIO_BASE || !runtime->report.tag_count || runtime->report.tag_count>65535)
        return mcc_diagnostic_fail(runtime,"map size or tag count exceeds sound converter limits");
    audio=malloc(sizeof(*audio));
    if (!audio) return mcc_diagnostic_fail(runtime,"not enough memory for sound conversion state");
    memset(audio,0,sizeof(*audio));runtime->audio=audio;
    audio->items=malloc(runtime->report.tag_count*sizeof(*audio->items));
    if (!audio->items) return mcc_diagnostic_fail(runtime,"not enough memory for sound storage table");
    audio->count=runtime->report.tag_count;
    memset(audio->items,0,audio->count*sizeof(*audio->items));
    for (index=0;index<audio->count;index++) audio->items[index].handle=0xFFFFFFFFu;
    for (index=0;index<runtime->report.tag_count;index++) {
        uint32_t *entry=(uint32_t *)(runtime->tag_index+index*0x20);
        struct sound_definition *sound;
        uint32_t range_index,rate,output_rate;
        int channels;
        mcc_loading_update(MCC_LOADING_AUDIO, index, runtime->report.tag_count);
        if (entry[0]!=(uint32_t)SOUND_DEFINITION_TAG) continue;
        mcc_diagnostic_tag(runtime,entry[3]);
        mcc_diagnostic_at(runtime,"sound header");
        storage=&audio->items[index];storage->handle=entry[3];
        storage->limit=MIN(MCC_AUDIO_LIMIT-1u,MCC_AUDIO_STORAGE_LIMIT-audio->capacity);
        failed_codec=NONE;failed_size=0;failed_offset=0;
        stage="sound header";
        tag_name=mcc_runtime_pointer(runtime,entry[4],256);
        if (!tag_name || !memchr(tag_name,0,256)) tag_name="(unavailable)";
        sound=mcc_runtime_pointer(runtime,entry[5],sizeof(*sound));
        if (!sound) {stage="sound header is outside loaded tag data";goto failed;}
        if (sound->encoding<0 || sound->encoding>1) return mcc_diagnostic_fail(runtime,"unsupported encoding %d (expected 0=mono or 1=stereo)",sound->encoding);
        if (sound->sample_rate<NONE || sound->sample_rate>1) return mcc_diagnostic_fail(runtime,"unsupported sample_rate %d (expected -1=unspecified, 0=22050 Hz or 1=44100 Hz)",sound->sample_rate);
        /* Extended's empty dialogue tags use NONE for the sample rate.
         * Normalize their unused header, but never guess a payload's rate. */
        channels=sound->encoding+1; rate=sound->sample_rate==1 ? 44100 : 22050;
        output_rate=channels==1 ? 22050 : rate;
        for (range_index=0;range_index<(uint32_t)sound->pitch_ranges.count;range_index++) {
            struct sound_pitch_range *range=mcc_runtime_pointer(runtime,
                (uint32_t)sound->pitch_ranges.address+range_index*sizeof(*range),sizeof(*range));
            uint32_t permutation_index;
            failed_codec=NONE;failed_size=0;failed_offset=0;stage="pitch range is outside loaded tag data";
            mcc_diagnostic_at(runtime,"pitch_ranges[%lu]",(unsigned long)range_index);
            if (!range) goto failed;
            for (permutation_index=0;permutation_index<(uint32_t)range->permutations.count;permutation_index++) {
                struct sound_permutation *permutation=mcc_runtime_pointer(runtime,
                    (uint32_t)range->permutations.address+permutation_index*sizeof(*permutation),sizeof(*permutation));
                unsigned char *data;
                struct mcc_pcm pcm;
                int ok,external;
                char resource_name[288],group_name[288];
                mcc_loading_pulse();
                stage="permutation header";
                failed_codec=permutation ? permutation->compression : NONE;
                failed_size=permutation ? permutation->samples.size : 0;
                failed_offset=permutation ? (uint32_t)permutation->samples.file_offset : 0;
                mcc_diagnostic_at(runtime,"pitch_ranges[%lu] '%.32s'.permutations[%lu] '%.32s'",
                    (unsigned long)range_index,range->name,(unsigned long)permutation_index,permutation ? permutation->name : "(unavailable)");
                if (!permutation) {stage="permutation header is outside loaded tag data";goto failed;}
                if (permutation->samples.size<0 || permutation->samples.size>0x4000000)
                    return mcc_diagnostic_fail(runtime,"sample payload size %ld is outside 0..67108864 bytes",permutation->samples.size);
                external=(permutation->samples.pad&1)!=0;
                permutation->unknown0=NONE;
                permutation->unknown1=0;
                permutation->unknown2=entry[3];
                permutation->unknown3=entry[3];
                permutation->samples.address=NULL;
                permutation->sample_buffer_size=0;
                if (!permutation->samples.size) continue;
                stage="nonempty sound has unspecified sample rate";
                if (sound->sample_rate==NONE) goto failed;
                data=malloc(permutation->samples.size);
                stage="not enough memory to read the source sound payload";
                if (!data) goto failed;
                stage="embedded sound payload could not be read from the map";
                if (external) {
                    stage="external samples (selected MCC sounds.map data does not match)";
                    snprintf(resource_name,sizeof(resource_name),"%s__%lu__%lu",tag_name,range_index,permutation_index);
                    snprintf(group_name,sizeof(group_name),"%s__permutations",tag_name);
                    ok=mcc_resources_read(runtime,2,resource_name,permutation->samples.file_offset,permutation->samples.size,data,group_name);
                } else ok=mcc_runtime_read(runtime,permutation->samples.file_offset,permutation->samples.size,data);
                if (!ok) {
                    free(data); goto failed;
                }
                stage="codec decode failed";
                ok=mcc_pcm_open(data,permutation->samples.size,permutation->compression,channels,rate,&pcm);
                if (!ok && pcm.failure) stage=pcm.failure;
                if (ok && (permutation->compression!=1 || rate!=output_rate)) {
                    stage="ADPCM encoding, allocation, or 256 MiB per-sound or 1 GiB total audio limit";
                    ok=mcc_audio_store(storage,&pcm,channels,output_rate,permutation);
                    if (!ok && storage->failure) stage=storage->failure;
                    if (ok) converted++;
                } else if (ok && external) {
                    stage="external ADPCM allocation or 256 MiB per-sound or 1 GiB total audio limit";
                    ok=mcc_audio_store_raw(storage,data,permutation);
                    if (!ok && storage->failure) stage=storage->failure;
                    if (ok) converted++;
                }
                free(data);
                if (ok) permutation->samples.pad&=~1;
                mcc_pcm_close(&pcm);
                if (!ok) {
                    error(_error_silent,"mcc sound payload: codec %d, %d channels, %lu Hz, offset 0x%08lx, %ld bytes, %s, decoder error %d",
                        permutation->compression,channels,(unsigned long)rate,(unsigned long)permutation->samples.file_offset,
                        permutation->samples.size,external ? "external sounds.map" : "embedded in map",pcm.decoder_error);
                    goto failed;
                }
            }
        }
        sound->compression=1;
        sound->sample_rate=output_rate==44100 ? 1 : 0;
        audio->used+=storage->used;audio->capacity+=storage->capacity;
        audio->stream_end=MAX(audio->stream_end,storage->used);
    }
    error(_error_silent,"mcc audio: %lu permutations converted, %lu ADPCM bytes",converted,audio->used);
    return 1;
failed:
    return mcc_diagnostic_fail(runtime,"%s; codec %ld (%s), payload offset 0x%08lx, %ld bytes",
        stage,failed_codec,mcc_audio_codec_name(failed_codec),(unsigned long)failed_offset,failed_size);
}

static struct mcc_audio_storage *mcc_audio_tag_storage(struct mcc_runtime *runtime,long tag)
{
    struct mcc_audio_tags *audio=runtime->audio;
    uint32_t index=(uint32_t)tag&0xFFFFu;
    return tag!=NONE && audio && index<audio->count && audio->items[index].handle==(uint32_t)tag ?
        &audio->items[index] : NULL;
}

int mcc_audio_read(struct mcc_runtime *runtime,long tag,uint32_t offset,uint32_t bytes,void *out)
{
    struct mcc_audio_storage *storage=mcc_audio_tag_storage(runtime,tag);
    if (offset<MCC_AUDIO_BASE || offset>=MCC_AUDIO_BASE+MCC_AUDIO_LIMIT) return -1;
    offset-=MCC_AUDIO_BASE;
    if (!storage || offset>storage->used || bytes>storage->used-offset) return 0;
    if (bytes) memcpy(out,storage->bytes+offset,bytes);
    return 1;
}

int mcc_audio_contains(struct mcc_runtime *runtime,long tag,uint32_t offset,uint32_t bytes)
{
    struct mcc_audio_storage *storage=mcc_audio_tag_storage(runtime,tag);
    return storage && offset>=MCC_AUDIO_BASE && offset-MCC_AUDIO_BASE<=storage->used &&
        bytes<=storage->used-(offset-MCC_AUDIO_BASE);
}

uint32_t mcc_audio_stream_end(struct mcc_runtime *runtime)
{
    struct mcc_audio_tags *audio=runtime->audio;
    return audio && audio->stream_end ? MCC_AUDIO_BASE+audio->stream_end : (uint32_t)runtime->source.size;
}

void mcc_audio_dispose(struct mcc_runtime *runtime)
{
    struct mcc_audio_tags *audio=runtime->audio;
    uint32_t i;
    if (audio) {
        for (i=0;i<audio->count;i++) if (audio->items[i].bytes) free(audio->items[i].bytes);
        if (audio->items) free(audio->items);
        free(audio);runtime->audio=NULL;
    }
}

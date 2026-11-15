/* MCC's private instance of the unmodified generic Vorbis dependency. Every
 * public dependency symbol is namespaced so Xbox/CE keep their existing decoder.
 * Empty residue vector books add nothing in Xiph's reference decoder; stb treats
 * an attempted read from one as an error and abandons the remaining residue.
 * Normalize only those vector-book references before decoding any audio packet.
 * Reference: https://github.com/xiph/vorbis/blob/v1.3.7/lib/codebook.c
 * (vorbis_book_decodevs_add, vorbis_book_decodev_add, vorbis_book_decodevv_add).
 */
#include "mcc_vorbis.h"
#include "mcc_loading.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#define stb_vorbis_get_info mcc_stb_get_info
#define stb_vorbis_get_comment mcc_stb_get_comment
#define stb_vorbis_get_error mcc_stb_get_error
#define stb_vorbis_close mcc_stb_close
#define stb_vorbis_get_sample_offset mcc_stb_get_sample_offset
#define stb_vorbis_get_file_offset mcc_stb_get_file_offset
#define stb_vorbis_decode_memory mcc_stb_decode_memory
#define stb_vorbis_open_memory mcc_stb_open_memory
#define stb_vorbis_seek_frame mcc_stb_seek_frame
#define stb_vorbis_seek mcc_stb_seek
#define stb_vorbis_seek_start mcc_stb_seek_start
#define stb_vorbis_stream_length_in_samples mcc_stb_stream_length_in_samples
#define stb_vorbis_stream_length_in_seconds mcc_stb_stream_length_in_seconds
#define stb_vorbis_get_frame_float mcc_stb_get_frame_float
#define stb_vorbis_get_frame_short_interleaved mcc_stb_get_frame_short_interleaved
#define stb_vorbis_get_frame_short mcc_stb_get_frame_short
#define stb_vorbis_get_samples_float_interleaved mcc_stb_get_samples_float_interleaved
#define stb_vorbis_get_samples_float mcc_stb_get_samples_float
#define stb_vorbis_get_samples_short_interleaved mcc_stb_get_samples_short_interleaved
#define stb_vorbis_get_samples_short mcc_stb_get_samples_short
#include "../../third_party/stb_vorbis/stb_vorbis.c"

static uint32_t mcc_ogg_u32(unsigned char const *p)
{
    return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
}

/* The pull decoder does not verify page checksums. Validate the complete bounded
 * permutation first, including EOS, so a truncated/damaged packet is not made
 * acceptable by the narrow empty-book compatibility rule. */
static int mcc_ogg_validate(unsigned char const *bytes, uint32_t size, uint32_t *frames,
    uint32_t *payload_end, uint32_t *final_page, char const **reason)
{
    uint32_t table[256], offset=0, serial=0, sequence=0, i, j;
    uint32_t last_page=0, last_end=0, previous_granule=0;
    int continued=0, ended=0;
    *payload_end=size;*final_page=0;
    for (i=0;i<256;i++) {
        uint32_t crc=i<<24;
        for (j=0;j<8;j++) crc=(crc<<1)^((crc&0x80000000u)?0x04C11DB7u:0);
        table[i]=crc;
    }
    while (offset<size) {
        unsigned char const *page=bytes+offset;
        uint32_t length=27, count, crc=0;
        mcc_loading_pulse();
        if (ended) {*reason="Ogg contains data after the end-of-stream page";return 0;}
        if (size-offset<27) {*reason="Ogg page header is truncated";return 0;}
        if (memcmp(page,"OggS",4) || page[4] || (page[5]&~7)) {*reason="Ogg page has an invalid signature, version or flags";return 0;}
        count=page[26];
        if (count>size-offset-length) {*reason="Ogg segment table is truncated";return 0;}
        length+=count;
        for (i=0;i<count;i++) length+=page[27+i];
        if (length>size-offset) {*reason="Ogg page payload is truncated";return 0;}
        if (count && !!(page[5]&1)!=continued) {*reason="Ogg packet continuation flags disagree between pages";return 0;}
        if (!offset) {
            if (!(page[5]&2)) {*reason="Ogg first page lacks the beginning-of-stream flag";return 0;}
            serial=mcc_ogg_u32(page+14);sequence=mcc_ogg_u32(page+18);
        } else if ((page[5]&2) || mcc_ogg_u32(page+14)!=serial || mcc_ogg_u32(page+18)!=sequence) {*reason="Ogg page stream serial, sequence or beginning flag is inconsistent";return 0;}
        for (i=0;i<length;i++) {
            unsigned char value=i>=22 && i<26 ? 0 : page[i];
            crc=(crc<<8)^table[(crc>>24)^value];
        }
        if (crc!=mcc_ogg_u32(page+22)) {*reason="Ogg page CRC checksum does not match its contents";return 0;}
        /* A page without a completed packet has no granule. All other known
         * positions must progress; EOS cannot trim an earlier page's audio. */
        if (mcc_ogg_u32(page+6)!=0xFFFFFFFFu || mcc_ogg_u32(page+10)!=0xFFFFFFFFu) {
            if (mcc_ogg_u32(page+10) || mcc_ogg_u32(page+6)<previous_granule) {*reason="Ogg granule position is too large or moves backwards";return 0;}
            previous_granule=mcc_ogg_u32(page+6);
        }
        /* Ogg permits zero-segment pages (including a separate EOS page). With
         * no lacing entries no packet begins or ends, so retain pending state.
         * https://xiph.org/ogg/doc/framing.html#page_segments */
        if (count) {
            continued=page[26+count]==255;
            last_page=offset;last_end=offset+length;
        }
        ended=(page[5]&4)!=0;
        if (ended) {
            if (continued || mcc_ogg_u32(page+10)) {*reason="Ogg end-of-stream page ends mid-packet or has an unsupported granule";return 0;}
            *frames=mcc_ogg_u32(page+6);
            if (!count) {
                if (!last_end) {*reason="empty Ogg terminal page has no preceding packet";return 0;}
                *payload_end=last_end;*final_page=last_page;
            }
        }
        offset+=length;sequence++;
    }
    if (!ended) *reason="Ogg end-of-stream page is missing";
    return ended;
}

/* stb's page reader uses a stale lacing entry for an empty terminal page. Give
 * only this case a private equivalent framing: finish the last complete packet
 * page with the already-validated EOS granule. This also preserves final-block
 * trimming. Source bytes and the shared Xbox/CE decoder remain untouched. */
static unsigned char *mcc_ogg_terminal_page(unsigned char const *bytes,
    uint32_t size, uint32_t offset, uint32_t frames)
{
    unsigned char *copy=malloc(size), *page;
    uint32_t i,j,crc=0;
    if (!copy) return NULL;
    memcpy(copy,bytes,size);page=copy+offset;
    page[5]|=4;
    for (i=0;i<4;i++) page[6+i]=(unsigned char)(frames>>(8*i));
    memset(page+10,0,4);memset(page+22,0,4);
    for (i=0;i<size-offset;i++) {
        crc^=(uint32_t)page[i]<<24;
        for (j=0;j<8;j++) crc=(crc<<1)^((crc&0x80000000u)?0x04C11DB7u:0);
    }
    for (i=0;i<4;i++) page[22+i]=(unsigned char)(crc>>(8*i));
    return copy;
}

static int mcc_vorbis_empty_book(Codebook const *book)
{
    int i;
    if (book->sparse) return !book->sorted_entries;
    /* stb expands very small sparse books, including an empty 1-3 entry book. */
    for (i=0;i<book->entries;i++) if (book->codeword_lengths[i]!=NO_CODE) return 0;
    return 1;
}

static int mcc_vorbis_residues(vorb *stream)
{
    int index, classification, pass;
    /* A floor book supplies scalar values, never an additive vector. Keep that
     * distinct from the zero-contribution residue case. */
    for (index=0;index<stream->floor_count;index++) {
        Floor1 const *floor=&stream->floor_config[index].floor1;
        int partition;
        if (stream->floor_types[index]!=1) return 0;
        for (partition=0;partition<floor->partitions;partition++) {
            int kind=floor->partition_class_list[partition];
            int subclasses=floor->class_subclasses[kind];
            if (subclasses && mcc_vorbis_empty_book(stream->codebooks+floor->class_masterbooks[kind])) return 0;
            for (classification=0;classification<(1<<subclasses);classification++) {
                int number=floor->subclass_books[kind][classification];
                if (number>=0 && mcc_vorbis_empty_book(stream->codebooks+number)) return 0;
            }
        }
    }
    for (index=0;index<stream->residue_count;index++) {
        Residue *residue=stream->residue_config+index;
        /* Empty scalar classbooks cannot supply classifications. This is not the
         * empty additive-vector case, and must not be silently accepted. */
        if (mcc_vorbis_empty_book(stream->codebooks+residue->classbook)) return 0;
        for (classification=0;classification<residue->classifications;classification++) {
            for (pass=0;pass<8;pass++) {
                int number=residue->residue_books[classification][pass];
                if (number>=0) {
                    Codebook const *book=stream->codebooks+number;
                    if (!book->lookup_type) return 0;
                    if (mcc_vorbis_empty_book(book)) residue->residue_books[classification][pass]=-1;
                }
            }
        }
    }
    return 1;
}

/* Keep dependency error numbers supplementary to an author-readable cause. */
static char const *mcc_vorbis_decoder_reason(int code)
{
    switch (code) {
    case VORBIS_outofmem: return "not enough memory inside the Vorbis decoder";
    case VORBIS_feature_not_supported: return "Vorbis stream uses an unsupported floor/codec feature";
    case VORBIS_too_many_channels: return "Vorbis channel count exceeds the decoder limit";
    case VORBIS_unexpected_eof: return "Vorbis packet ended before the decoder could read its required data";
    case VORBIS_invalid_setup: return "Vorbis setup packet or block-size declaration is invalid";
    case VORBIS_invalid_stream: return "Vorbis audio packet is invalid";
    case VORBIS_missing_capture_pattern: return "Vorbis decoder could not find the Ogg page signature";
    case VORBIS_invalid_stream_structure_version: return "Vorbis decoder encountered an unsupported Ogg version";
    case VORBIS_continued_packet_flag_invalid: return "Vorbis decoder encountered an invalid packet continuation flag";
    case VORBIS_incorrect_stream_serial_number: return "Vorbis decoder encountered inconsistent Ogg stream serial numbers";
    case VORBIS_invalid_first_page: return "Vorbis identification/header page is invalid";
    case VORBIS_bad_packet_type: return "Vorbis packet type is invalid for this decoding stage";
    case VORBIS_cant_find_last_page: return "Vorbis decoder could not find the final Ogg page";
    case VORBIS_ogg_skeleton_not_supported: return "Ogg Skeleton streams are unsupported by the Vorbis decoder";
    default: return NULL;
    }
}

int mcc_vorbis_decode_ex(unsigned char const *bytes, uint32_t size, uint32_t frame_limit,
    short **samples, uint32_t *frames, uint32_t *rate, int *channels,
    char const **reason, int *decoder_error)
{
    vorb stream;
    uint32_t expected=0, count=0, payload_end=0, final_page=0;
    unsigned char *normalized=NULL;
    short *pcm=NULL;
    short packet[8192 * 2];
    uint32_t decoded=0;
    int ok=0, boundary_known=0, boundary_bias=0;
    char const *decoder_reason;
    *samples=NULL;*frames=0;*rate=0;*channels=0;
    *decoder_error=0;
    *reason="Ogg payload is empty or exceeds the decoder's input limit";
    if (!bytes || !size || size>INT_MAX) return 0;
    if (!mcc_ogg_validate(bytes,size,&expected,&payload_end,&final_page,reason)) return 0;
    *reason="Ogg declared frame count is zero or exceeds the decoded frame/allocation limit";
    if (!expected || expected>frame_limit || expected>INT_MAX/(2*sizeof(short))) return 0;
    if (payload_end<size) {
        normalized=mcc_ogg_terminal_page(bytes,payload_end,final_page,expected);
        if (!normalized) {*reason="not enough memory to normalize an empty Ogg terminal page";return 0;}
        bytes=normalized;size=payload_end;
    }
    vorbis_init(&stream,NULL);
    stream.stream=(uint8 *)bytes;
    stream.stream_start=stream.stream;
    stream.stream_end=stream.stream+size;
    stream.stream_len=size;
    *reason="Vorbis setup headers were rejected by the decoder";
    if (!start_decoder(&stream)) goto done;
    *reason="unsupported Vorbis floor/residue codebook layout";
    if (!mcc_vorbis_residues(&stream)) goto done;
    *reason="Vorbis requires 1 or 2 channels and a sample rate of 8000..192000 Hz";
    if (stream.channels<1 || stream.channels>2 || stream.sample_rate<8000 || stream.sample_rate>192000) goto done;
    pcm=malloc((size_t)expected*stream.channels*sizeof(short));
    *reason="not enough memory for decoded Vorbis samples";
    if (!pcm) goto done;
    /* Normalization precedes the discarded overlap packet, which can itself use
     * an empty residue vector book. No Xbox/CE decoder state is accessible here. */
    *reason="Vorbis first audio packet could not be decoded";
    if (!vorbis_pump_first_frame(&stream) || stream.error) goto done;
    for (;;) {
        int got=stb_vorbis_get_frame_short_interleaved(&stream,stream.channels,packet,8192*2);
        uint32_t retain;
        mcc_loading_pulse();
        if (stream.error) {*reason="Vorbis audio packet decode failed";goto done;}
        if (!got) break;
        decoded+=(uint32_t)got;
        if (!(stream.page_flag&PAGEFLAG_last_page) && stream.last_seg_which==stream.end_seg_with_known_loc) {
            int difference=(int)decoded-(int)stream.known_loc_for_packet;
            /* A long block followed by a short block emits up to its right
             * window's start, beyond the packet's centre/granule. This offset
             * disappears again after the short packet. Accept only that exact
             * window transition, not arbitrary drifting page positions. */
            if (difference!=0 && difference!=(stream.blocksize_1-stream.blocksize_0)/4) {
                *reason="Vorbis decoded frame positions disagree with Ogg page granules";goto done;
            }
            boundary_bias=difference;boundary_known=1;
        }
        retain=(uint32_t)got;
        if (retain>expected-count) {
            /* Vorbis I, appendix A.2. A final-packet trim plus the validated
             * window-boundary offset is bounded by one maximum block. Never
             * drain an unbounded remainder or suppress packet errors. */
            if (decoded-expected>(uint32_t)stream.blocksize_1 ||
                (!(stream.page_flag&PAGEFLAG_last_page) &&
                 (!boundary_known || decoded-expected>(uint32_t)boundary_bias))) {
                *reason="Vorbis final-packet trimming exceeds the permitted block/window boundary";goto done;
            }
            retain=expected-count;
        }
        memcpy(pcm+count*stream.channels,packet,(size_t)retain*stream.channels*sizeof(short));
        count+=retain;
    }
    if (count!=expected) {*reason="Vorbis decoded fewer frames than the Ogg end-of-stream declaration";goto done;}
    *samples=pcm;*frames=count;*rate=stream.sample_rate;*channels=stream.channels;
    pcm=NULL;ok=1;
done:
    *decoder_error=stream.error;
    decoder_reason=mcc_vorbis_decoder_reason(stream.error);
    if (ok) *reason=NULL;
    else if (decoder_reason) *reason=decoder_reason;
    free(pcm);
    vorbis_deinit(&stream);
    free(normalized);
    return ok;
}

int mcc_vorbis_decode(unsigned char const *bytes, uint32_t size, uint32_t frame_limit,
    short **samples, uint32_t *frames, uint32_t *rate, int *channels)
{
    char const *reason;
    int decoder_error;
    return mcc_vorbis_decode_ex(bytes,size,frame_limit,samples,frames,rate,channels,&reason,&decoder_error);
}

void mcc_vorbis_free(short *samples)
{
    free(samples);
}

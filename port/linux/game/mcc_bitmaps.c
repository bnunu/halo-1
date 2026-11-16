/* MCC bitmap resources. Convert and pack each resource in MCC-owned memory,
 * then stream its native hardware layout. The common rasterizer supplies
 * descriptor size limits; no CE bitmap state or conversion API is used. */
#include "cseries.h"
#include "errors.h"
#include "bitmaps/bitmaps.h"
#include "bitmaps/bitmap_group.h"
#include "interface/unit_hud_interface_definition.h"
#include "rasterizer/rasterizer_swizzle.h"
#include "mcc_runtime.h"
#include "mcc_loading.h"
#include "mcc_resources.h"
#include "mcc_texture_cache.h"
#include "../src/mcc_texture_bridge.h"
#include "../src/port_config.h"
#include <stdlib.h>
#include <string.h>
/* Unmodified iOrange/bcdec, MIT option (see third_party/bcdec/LICENSE).
 * Revision 80859ed3b7afb1c527a2a99d70c61457bea72d0c.
 * https://github.com/iOrange/bcdec
 * Header SHA-256: 134520764d96f70a27db814173616c89ad85db95fbd3328417c7a8c77e7ca189. */
#define BCDEC_STATIC
#define BCDEC_IMPLEMENTATION
#include "../../third_party/bcdec/bcdec.h"

#define MCC_BITMAP_BASE 0x60000000u
#define MCC_BITMAP_LIMIT 0x1FFFFFFFu
#define MCC_BITMAP_ALLOCATION_LIMIT 0x08000000u
/* XAM's original textures alone exceed 1 GiB. Keep a bounded aggregate
 * budget; individual allocations and per-tag stream offsets stay smaller. */
#define MCC_BITMAP_STORAGE_LIMIT 0x80000000u

struct mcc_texture {
    struct bitmap_data *bitmap;
    uint32_t handle, offset, size;
    unsigned char *pixels;
    unsigned channels;
    uint32_t resource_handle, resource_index;
};
struct mcc_bitmap_clone { uint32_t source, target; unsigned channels; };
struct mcc_textures {
    struct mcc_texture *items;
    struct mcc_bitmap_clone *clones;
    uint32_t count, end, capacity, clone_count, tag_capacity, stream_end;
};

static void *mcc_bitmap_tag(struct mcc_runtime *runtime,uint32_t handle,uint32_t group,uint32_t size)
{
    uint32_t *entry;
    if (handle==0xFFFFFFFFu || (handle&0xFFFF)>=runtime->report.tag_count) return NULL;
    entry=(uint32_t *)(runtime->tag_index+(handle&0xFFFF)*0x20);
    if (entry[0]!=group || entry[3]!=handle) return NULL;
    return mcc_runtime_pointer(runtime,entry[5],size);
}

static void *mcc_bitmap_block(struct mcc_runtime *runtime,struct tag_block const *block,uint32_t size)
{
    if (block->count<0 || (uint32_t)block->count>UINT32_MAX/size) return NULL;
    if (!block->count) return runtime->tags;
    return mcc_runtime_pointer(runtime,(uint32_t)block->address,(uint32_t)block->count*size);
}

static void mcc_bitmap_mark_one(struct mcc_textures *textures,struct bitmap_data *bitmap,unsigned channels)
{
    uint32_t index;
    for (index=0;index<textures->count;index++) {
        if (textures->items[index].bitmap==bitmap) textures->items[index].channels|=channels;
    }
}

static int mcc_bitmap_clone_block(struct mcc_runtime *runtime,struct tag_block *block,uint32_t stride)
{
    void *source,*copy;
    uint32_t bytes;
    if (!block->count) {block->address=NULL;return 1;}
    source=mcc_bitmap_block(runtime,block,stride);
    if (!source) return mcc_diagnostic_fail(runtime,"bitmap data block or source tag is outside loaded data");
    bytes=(uint32_t)block->count*stride;
    copy=mcc_runtime_allocate(runtime,bytes);
    if (!copy) return mcc_diagnostic_fail(runtime,"not enough tag memory for independent bitmap metadata");
    memcpy(copy,source,bytes);block->address=copy;
    return 1;
}

/* A bitmap can be used as both color and a packed material mask. Each
 * specialized consumer gets its own tag and descriptors, preserving the
 * ordinary source tag and every other consumer's channels. */
static int mcc_bitmap_specialize(struct mcc_runtime *runtime,struct mcc_textures *textures,
    uint32_t *handle,unsigned channels)
{
    struct bitmap_group *source,*copy;
    struct bitmap_group_sequence *sequences;
    struct bitmap_data *bitmaps;
    uint32_t i,*entry,new_handle;
    char *name;
    if (*handle==0xFFFFFFFFu || channels==1) return 1;
    for (i=0;i<textures->clone_count;i++) {
        struct mcc_bitmap_clone const *clone=&textures->clones[i];
        if (clone->source==*handle && clone->channels==channels) {*handle=clone->target;return 1;}
    }
    if (runtime->report.tag_count>=textures->tag_capacity) return mcc_diagnostic_fail(runtime,"bitmap specialization exceeds the tag table capacity (%lu)",(unsigned long)textures->tag_capacity);
    source=mcc_bitmap_tag(runtime,*handle,BITMAP_GROUP_TAG,sizeof(*source));
    if (!source) return mcc_diagnostic_fail(runtime,"bitmap data block or source tag is outside loaded data");
    copy=mcc_runtime_allocate(runtime,sizeof(*copy));
    if (!copy) return mcc_diagnostic_fail(runtime,"not enough tag memory for independent bitmap metadata");
    *copy=*source;
    if (!mcc_bitmap_clone_block(runtime,&copy->sequences,sizeof(*sequences)) ||
        !mcc_bitmap_clone_block(runtime,&copy->bitmaps,sizeof(*bitmaps))) return 0;
    sequences=copy->sequences.address;
    for (i=0;i<(uint32_t)copy->sequences.count;i++) {
        if (!mcc_bitmap_clone_block(runtime,&sequences[i].sprites,sizeof(struct bitmap_group_sprite))) return 0;
    }
    /* Import/pixel tag_data are editor storage. Runtime pixels use each
     * descriptor's file range, so these never alias a cloned tag allocation. */
    memset(&copy->import_bitmap,0,sizeof(copy->import_bitmap));
    memset(&copy->pixel_data,0,sizeof(copy->pixel_data));
    name=mcc_runtime_allocate(runtime,48);
    if (!name) return mcc_diagnostic_fail(runtime,"not enough tag memory for bitmap copy name");
    snprintf(name,48,"mcc_runtime\\bitmap_%08lx_%u",(unsigned long)*handle,channels);
    entry=(uint32_t *)(runtime->tag_index+runtime->report.tag_count*32);
    memcpy(entry,runtime->tag_index+(*handle&0xFFFF)*32,32);
    new_handle=(*handle&0xFFFF0000u)|runtime->report.tag_count;
    entry[3]=new_handle;entry[4]=(uint32_t)name;entry[5]=(uint32_t)copy;
    bitmaps=copy->bitmaps.address;
    for (i=0;i<(uint32_t)copy->bitmaps.count;i++) {
        struct mcc_texture *texture;
        if (textures->count>=textures->capacity) return mcc_diagnostic_fail(runtime,"bitmap specialization exceeds the texture table capacity");
        texture=&textures->items[textures->count++];
        texture->bitmap=&bitmaps[i];texture->handle=new_handle;texture->channels=channels;
        texture->resource_handle=*handle;texture->resource_index=i;
    }
    textures->clones[textures->clone_count].source=*handle;
    textures->clones[textures->clone_count].target=new_handle;
    textures->clones[textures->clone_count++].channels=channels;
    runtime->report.tag_count++;
    *handle=new_handle;
    return 1;
}

/* Channels is a usage mask: ordinary=1, multipurpose=2, HUD meter=4. */
static int mcc_bitmap_mark(struct mcc_runtime *runtime,struct mcc_textures *textures,
    uint32_t *reference,short sequence,unsigned channels)
{
    struct bitmap_group *group;
    struct bitmap_data *bitmaps;
    uint32_t index;
    uint32_t handle;
    if (!mcc_bitmap_specialize(runtime,textures,reference,channels)) return 0;
    handle=*reference;
    if (handle==0xFFFFFFFFu) return 1;
    group=mcc_bitmap_tag(runtime,handle,BITMAP_GROUP_TAG,sizeof(*group));
    if (!group) return mcc_diagnostic_fail(runtime,"bitmap reference %08lx has the wrong group, a stale handle, or an out-of-range header",(unsigned long)handle);
    bitmaps=mcc_bitmap_block(runtime,&group->bitmaps,sizeof(*bitmaps));
    if (!bitmaps) return mcc_diagnostic_fail(runtime,"referenced bitmap has an out-of-range bitmaps block");
    if (sequence>=0 && sequence<group->sequences.count) {
        struct bitmap_group_sequence *sequences=mcc_bitmap_block(runtime,&group->sequences,sizeof(*sequences));
        struct bitmap_group_sequence *selected;
        if (!sequences) return mcc_diagnostic_fail(runtime,"referenced bitmap has an out-of-range sequences block");
        selected=&sequences[sequence];
        if (selected->sprites.count) {
            struct bitmap_group_sprite *sprites=mcc_bitmap_block(runtime,&selected->sprites,sizeof(*sprites));
            if (!sprites) return mcc_diagnostic_fail(runtime,"bitmap sequence %d has an out-of-range sprites block",sequence);
            for (index=0;index<(uint32_t)selected->sprites.count;index++) {
                short n=sprites[index].bitmap_index;
                if (n<0 || n>=group->bitmaps.count) return mcc_diagnostic_fail(runtime,"bitmap sequence %d sprites[%lu].bitmap_index=%d is outside 0..%ld",sequence,(unsigned long)index,n,group->bitmaps.count-1);
                mcc_bitmap_mark_one(textures,&bitmaps[n],channels);
            }
        } else {
            if (selected->first_bitmap_index<0 || selected->bitmap_count<0 ||
                selected->first_bitmap_index>group->bitmaps.count-selected->bitmap_count) return mcc_diagnostic_fail(runtime,"bitmap sequence %d range (first %d, count %d) exceeds %ld bitmaps",sequence,selected->first_bitmap_index,selected->bitmap_count,group->bitmaps.count);
            for (index=0;index<(uint32_t)selected->bitmap_count;index++)
                mcc_bitmap_mark_one(textures,&bitmaps[selected->first_bitmap_index+index],channels);
        }
    } else {
        for (index=0;index<(uint32_t)group->bitmaps.count;index++) mcc_bitmap_mark_one(textures,&bitmaps[index],channels);
    }
    return 1;
}

static int mcc_meter_usage(struct mcc_runtime *runtime,struct mcc_textures *textures,
    struct meter_hud_element_definition *meter)
{
    /* MCC meter flag 5 keeps Xbox fill/shape channels. The native meter
     * already consumes that order; only Gearbox-style meters need a clone
     * with luminance and alpha exchanged. */
    unsigned channels=meter->meter_flags&0x20 ? 1 : 4;
    /* Gearbox meters do not apply the Xbox RGB opacity tint. Descent's
     * authored opacity of one otherwise turns its shield and ammo black.
     * Fade and the explicitly Xbox-style meter convention remain intact. */
    if (channels==4) meter->opacity=0.0f;
    return mcc_bitmap_mark(runtime,textures,(uint32_t *)&meter->meter_bitmap.index,meter->sequence_index,channels);
}

static int mcc_static_usage(struct mcc_runtime *runtime,struct mcc_textures *textures,
    struct static_hud_element_definition *element)
{
    return mcc_bitmap_mark(runtime,textures,(uint32_t *)&element->interface_bitmap.index,element->sequence_index,1);
}

static int mcc_bitmap_model_usage(struct mcc_runtime *runtime,struct mcc_textures *textures,
    unsigned char *shader)
{
    static unsigned const offsets[]={0xB0,0xC8,0xE8,0x170};
    uint16_t flags;
    unsigned n;
    memcpy(&flags,shader+0x28,sizeof(flags));
    /* MCC's model flag 6 selects original Xbox mask channels. These masks
     * already have reflection in red and change color in blue; applying the
     * Gearbox swizzle would turn opaque DXT1 alpha into full-surface tint.
     * Select per reference: another shader may use this same bitmap with
     * the flag clear and must still receive its own reordered clone. */
    for (n=0;n<4;n++) {
        unsigned channels=n==1 && !(flags&0x40) ? 2 : 1;
        static char const *const names[]={"base_map","multipurpose_map","detail_map","reflection_cube_map"};
        mcc_diagnostic_at(runtime,"%s",names[n]);
        if (!mcc_bitmap_mark(runtime,textures,(uint32_t *)(shader+offsets[n]),NONE,channels)) return 0;
    }
    return 1;
}

static int mcc_bitmap_usages(struct mcc_runtime *runtime,struct mcc_textures *textures)
{
    uint32_t index;
    for (index=0;index<runtime->report.tag_count;index++) {
        uint32_t *entry=(uint32_t *)(runtime->tag_index+index*0x20);
        mcc_diagnostic_tag(runtime,entry[3]);
        mcc_diagnostic_at(runtime,"shader/HUD bitmap references");
        if (entry[0]=='soso') {
            unsigned char *shader=mcc_runtime_pointer(runtime,entry[5],0x174);
            if (!shader) return mcc_diagnostic_fail(runtime,"shader_model header is outside loaded data");
            if (!mcc_bitmap_model_usage(runtime,textures,shader)) return 0;
        } else if (entry[0]==UNIT_HUD_INTERFACE_DEFINITION_TAG) {
            struct unit_hud_interface_definition *hud=mcc_runtime_pointer(runtime,entry[5],sizeof(*hud));
            struct auxilary_meter_definition *meters;
            struct auxilary_overlay_definition *overlays;
            uint32_t n;
            if (!hud) return mcc_diagnostic_fail(runtime,"unit HUD header is outside loaded data");
            mcc_diagnostic_at(runtime,"shield_meter.meter.bitmap");
            if (!mcc_meter_usage(runtime,textures,&hud->shield_meter.meter)) return 0;
            mcc_diagnostic_at(runtime,"health_meter.meter.bitmap");
            if (!mcc_meter_usage(runtime,textures,&hud->health_meter.meter)) return 0;
            mcc_diagnostic_at(runtime,"background.bitmap");
            if (!mcc_static_usage(runtime,textures,&hud->background)) return 0;
            mcc_diagnostic_at(runtime,"shield_meter.background.bitmap");
            if (!mcc_static_usage(runtime,textures,&hud->shield_meter.background)) return 0;
            mcc_diagnostic_at(runtime,"health_meter.background.bitmap");
            if (!mcc_static_usage(runtime,textures,&hud->health_meter.background)) return 0;
            mcc_diagnostic_at(runtime,"motion_sensor.background.bitmap");
            if (!mcc_static_usage(runtime,textures,&hud->motion_sensor.background)) return 0;
            mcc_diagnostic_at(runtime,"motion_sensor.foreground.bitmap");
            if (!mcc_static_usage(runtime,textures,&hud->motion_sensor.foreground)) return 0;
            meters=mcc_bitmap_block(runtime,&hud->auxilary_meters,sizeof(*meters));
            overlays=mcc_bitmap_block(runtime,&hud->auxilary_panel.auxilary_overlays,sizeof(*overlays));
            if (!meters || !overlays) return mcc_diagnostic_fail(runtime,"auxiliary HUD meters or overlays block is outside loaded data");
            for (n=0;n<(uint32_t)hud->auxilary_meters.count;n++) {
                mcc_diagnostic_at(runtime,"auxiliary_meters[%lu].meter.bitmap",(unsigned long)n);
                if (!mcc_meter_usage(runtime,textures,&meters[n].panel.meter)) return 0;
                mcc_diagnostic_at(runtime,"auxiliary_meters[%lu].background.bitmap",(unsigned long)n);
                if (!mcc_static_usage(runtime,textures,&meters[n].panel.background)) return 0;
            }
            for (n=0;n<(uint32_t)hud->auxilary_panel.auxilary_overlays.count;n++) {
                mcc_diagnostic_at(runtime,"auxiliary_overlays[%lu].bitmap",(unsigned long)n);
                if (!mcc_static_usage(runtime,textures,&overlays[n].static_element)) return 0;
            }
        } else if (entry[0]=='wphi') {
            unsigned char *hud=mcc_runtime_pointer(runtime,entry[5],0x78);
            unsigned which;
            if (!hud) return mcc_diagnostic_fail(runtime,"weapon HUD header is outside loaded data");
            for (which=0;which<2;which++) {
                struct tag_block *block=(struct tag_block *)(hud+0x60+which*12);
                unsigned char *elements=mcc_bitmap_block(runtime,block,0xB4);
                uint32_t n;
                if (!elements) return mcc_diagnostic_fail(runtime,"weapon HUD %s elements block is outside loaded data",which ? "meter" : "static");
                for (n=0;n<(uint32_t)block->count;n++) {
                    void *element=elements+n*0xB4+0x24;
                    mcc_diagnostic_at(runtime,"%s_elements[%lu].bitmap",which ? "meter" : "static",(unsigned long)n);
                    if (!(which ? mcc_meter_usage(runtime,textures,element) : mcc_static_usage(runtime,textures,element))) return 0;
                }
            }
        }
    }
    return 1;
}

static int mcc_bitmap_bytes(short format)
{
    switch (format) {
        case 0: case 1: case 2: case 17: return 1;
        case 3: case 6: case 8: case 9: return 2;
        case 10: case 11: return 4;
        default: return 0;
    }
}

static uint32_t mcc_pixel(unsigned char const *source,int format)
{
    unsigned v=source[0], a=255,r=v,g=v,b=v;
    if (format==0) {a=v; r=g=b=255;}
    else if (format==2) a=v;
    else if (format==3) a=source[1];
    else if (format==6 || format==8 || format==9) {
        v|=(unsigned)source[1]<<8;
        if (format==6) {r=(v>>11)*255/31; g=((v>>5)&63)*255/63; b=(v&31)*255/31;}
        else if (format==8) {a=(v&0x8000)?255:0; r=((v>>10)&31)*255/31; g=((v>>5)&31)*255/31; b=(v&31)*255/31;}
        else {a=(v>>12)*17; r=((v>>8)&15)*17; g=((v>>4)&15)*17; b=(v&15)*17;}
    } else if (format==10 || format==11) {
        b=source[0];g=source[1];r=source[2];a=format==10 ? 255 : source[3];
    } else if (format==17) return global_vector_palette[v];
    return b|(g<<8)|(r<<16)|(a<<24);
}

static uint32_t mcc_channels(uint32_t color,unsigned usage)
{
    unsigned b=color&255,g=(color>>8)&255,r=(color>>16)&255,a=color>>24;
    if (usage==2) return a|(g<<8)|(b<<16)|(r<<24);
    /* Native meters read fill order from blue and shape from alpha. Keep
     * shape in green too for the renderer's existing coverage blend, so
     * antialiased edges retain the scene instead of becoming black. */
    if (usage==4) return a|(r<<8)|(a<<16)|(r<<24);
    return color;
}

/* Decode one slice to BGRA. Compressed blocks remain 4x4 even for tiny mips. */
static void mcc_bitmap_decode(unsigned char const *input,uint32_t *output,
    unsigned width,unsigned height,short format,unsigned usage)
{
    unsigned x,y;
    if (format>=14 && format!=17) {
        unsigned step=format==14 ? 8 : 16;
        for (y=0;y<height;y+=4) for (x=0;x<width;x+=4) {
            unsigned char rgba[64];
            unsigned dx,dy;
            if (!x && !(y & 63u)) mcc_loading_pulse();
            if (format==14) bcdec_bc1(input,rgba,16);
            else if (format==15) bcdec_bc2(input,rgba,16);
            else if (format==16) bcdec_bc3(input,rgba,16);
            else bcdec_bc7(input,rgba,16);
            input+=step;
            for (dy=0;dy<4 && y+dy<height;dy++) for (dx=0;dx<4 && x+dx<width;dx++) {
                unsigned char const *p=rgba+(dy*4+dx)*4;
                uint32_t color=p[2]|((uint32_t)p[1]<<8)|((uint32_t)p[0]<<16)|((uint32_t)p[3]<<24);
                output[(y+dy)*width+x+dx]=mcc_channels(color,usage);
            }
        }
    } else {
        unsigned step=mcc_bitmap_bytes(format);
        for (x=0;x<width*height;x++) output[x]=mcc_channels(mcc_pixel(input+x*step,format),usage);
    }
}

static uint32_t mcc_morton_axis(unsigned value,unsigned width,unsigned height,unsigned depth,unsigned axis)
{
    unsigned dimensions[3]={width,height,depth},bit,position=0,n;
    uint32_t address=0;
    for (bit=1;bit<width || bit<height || bit<depth;bit<<=1) {
        for (n=0;n<3;n++) if (bit<dimensions[n]) {
            if (n==axis && (value&bit)) address|=1u<<position;
            position++;
        }
    }
    return address;
}

/* MCC input is mip-major. Native textures have Morton-ordered uncompressed
 * texels, six face-major cube chains, and explicit row/face alignment. Pack
 * directly into an owned, zeroed buffer so allocation failure is observable. */
static int mcc_bitmap_pack(struct bitmap_data *bitmap,unsigned char const *source,
    uint32_t source_size,unsigned char *target,uint32_t target_size)
{
    static unsigned const cube_face[6]={0,2,1,3,4,5};
    uint32_t x_address[8192],y_address[8192],z_address[512],out=0;
    unsigned face,faces=bitmap->type==2 ? 6 : 1;
    unsigned maximum=rasterizer_xbox_bitmap_get_max_mipmap_count(bitmap);
    unsigned bytes=mcc_bitmap_bytes(bitmap->format);
    /* CPU object lighting can select the final 2x2/1x1 DXT mips even though
     * the GPU descriptor stops at 4 texels. A 2D chain has identical CPU/GPU
     * prefix layout, so retain its complete source tail in owned storage. */
    if (bitmap->type==0 && (bitmap->flags&2)) maximum=bitmap->mipmap_count;
    memset(target,0,target_size);
    for (face=0;face<faces;face++) {
        unsigned level;
        uint32_t input=0;
        for (level=0;level<=maximum;level++) {
            unsigned width=MAX(bitmap->width>>level,1),height=MAX(bitmap->height>>level,1),depth=MAX(bitmap->depth>>level,1);
            uint32_t size=bytes ? width*height*depth*bytes : ((width+3)/4)*((height+3)/4)*depth*(bitmap->format==14 ? 8 : 16);
            uint32_t at=input+(faces==6 ? cube_face[face] : 0)*size;
            unsigned x,y,z;
            if (at>source_size || size>source_size-at || out>target_size) return 0;
            if (bitmap->flags&16) {
                unsigned pitch=(width*bytes+63)&~63u;
                if (pitch*height>target_size-out) return 0;
                for (y=0;y<height;y++) memcpy(target+out+y*pitch,source+at+y*width*bytes,width*bytes);
                out+=pitch*height;
            } else {
                if (size>target_size-out) return 0;
                if (!bytes) memcpy(target+out,source+at,size);
                else {
                    if (width>8192 || height>8192 || depth>512) return 0;
                    for (x=0;x<width;x++) x_address[x]=mcc_morton_axis(x,width,height,depth,0);
                    for (y=0;y<height;y++) y_address[y]=mcc_morton_axis(y,width,height,depth,1);
                    for (z=0;z<depth;z++) z_address[z]=mcc_morton_axis(z,width,height,depth,2);
                    for (z=0;z<depth;z++) for (y=0;y<height;y++) for (x=0;x<width;x++) {
                        uint32_t address=x_address[x]|y_address[y]|z_address[z];
                        memcpy(target+out+address*bytes,source+at+((z*height+y)*width+x)*bytes,bytes);
                    }
                }
                out+=size;
            }
            input+=size*faces;
        }
        out=(out+127)&~127u;
    }
    if (!(bitmap->flags&(2|16))) bitmap->flags|=8;
    return out==target_size;
}

/* Recent MCC campaign lightmaps carry bit 12 in addition to the environment
 * flag. Its wider MCC meaning is not assumed here: admit only the observed
 * lightmap-group RGB565 layout with a complete, tightly packed pixel range.
 * The normal descriptor verifier still checks every other bit and field. */
static int mcc_bitmap_lightmap_flags(struct bitmap_data *bitmap,short usage)
{
    if (!(bitmap->flags&0x1000)) return 1;
    if (usage!=4 || bitmap->flags!=0x1281 || bitmap->type!=0 || bitmap->format!=6 ||
        bitmap->width<=0 || bitmap->height<=0 || bitmap->depth!=1 || bitmap->mipmap_count ||
        (uint64_t)(unsigned)bitmap->width*(unsigned)bitmap->height*2!=(uint32_t)bitmap->pixels_size) return 0;
    bitmap->flags&=~0x1000;
    return 1;
}

/* Some MCC HUD textures declare a complete DXT1 chain but size its sub-4x4
 * tail as width*height/2 instead of whole compression blocks. Recognize only
 * that exact size formula and retain the complete prefix. Arbitrary short
 * ranges, missing base levels and other formats remain load failures. */
static void mcc_bitmap_complete_mips(struct bitmap_data *bitmap)
{
    uint64_t packed=0;
    unsigned level,maximum,complete=0;
    if (bitmap->type!=0 || bitmap->format!=14 || bitmap->flags!=0x81 || bitmap->depth!=1 ||
        bitmap->width<4 || bitmap->height<4 ||
        (bitmap->width&(bitmap->width-1)) || (bitmap->height&(bitmap->height-1))) return;
    maximum=(unsigned)floor_log2(MAX(bitmap->width,bitmap->height));
    if (bitmap->mipmap_count!=(short)maximum) return;
    for (level=0;level<=maximum;level++) {
        unsigned width=MAX(bitmap->width>>level,1),height=MAX(bitmap->height>>level,1);
        packed+=(uint64_t)width*height/2;
        if (width>=4 && height>=4) complete=level;
    }
    if (packed==(uint32_t)bitmap->pixels_size) bitmap->mipmap_count=(short)complete;
}

static int mcc_texture_prepare(struct mcc_runtime *runtime,struct mcc_textures *textures,struct mcc_texture *texture,
    uint32_t stream_offset)
{
    struct bitmap_data *bitmap=texture->bitmap, normalized=*bitmap;
    unsigned level,faces=bitmap->type==2 ? 6 : 1;
    uint64_t raw_size=0,decoded_size=0;
    unsigned char *raw=NULL,*pixels=NULL,*hardware=NULL;
    int convert=bitmap->format==18 || texture->channels==2 || texture->channels==4 ||
        ((bitmap->flags&16) && bitmap->format>=14 && bitmap->format<=17);
    int bytes_per_pixel=mcc_bitmap_bytes(bitmap->format);
    int non_power_two=bitmap->type==0 && ((bitmap->width&(bitmap->width-1)) || (bitmap->height&(bitmap->height-1)));
    uint32_t input_offset=0,output_offset=0,hardware_size,allocation;
    char const *reason="pixel packing failed for the normalized hardware layout";
    mcc_diagnostic_tag(runtime,texture->resource_handle);
    mcc_diagnostic_at(runtime,"bitmaps[%lu] (zero-based)",(unsigned long)texture->resource_index);
    if (bitmap->width<=0 || bitmap->height<=0 || bitmap->depth<=0 ||
        bitmap->width>16384 || bitmap->height>16384 || bitmap->depth>512)
        return mcc_diagnostic_fail(runtime,"dimensions %d x %d x %d exceed supported positive limits (16384 x 16384 x 512)",
            bitmap->width,bitmap->height,bitmap->depth);
    if (bitmap->type<0 || bitmap->type>2)
        return mcc_diagnostic_fail(runtime,"unsupported bitmap type %d (expected 0=2D, 1=3D, 2=cubemap)",bitmap->type);
    if (bitmap->mipmap_count<0 || bitmap->mipmap_count>13)
        return mcc_diagnostic_fail(runtime,"mipmap count %d is outside 0..13",bitmap->mipmap_count);
    if (bitmap->pixels_size<=0)
        return mcc_diagnostic_fail(runtime,"pixel payload size %ld must be positive",(long)bitmap->pixels_size);
    if (!bytes_per_pixel && (bitmap->format<14 || bitmap->format>18))
        return mcc_diagnostic_fail(runtime,"unsupported pixel format %d",bitmap->format);
    if ((bitmap->width>8192 || bitmap->height>8192) && !non_power_two && !(bitmap->flags&16))
        return mcc_diagnostic_fail(runtime,"nonlinear texture %d x %d exceeds the 8192-pixel dimension limit",bitmap->width,bitmap->height);
    if ((bitmap->type==1 && (bitmap->width>512 || bitmap->height>512 || bitmap->depth>256)) ||
        (bitmap->type!=1 && bitmap->depth!=1))
        return mcc_diagnostic_fail(runtime,"dimensions %d x %d x %d are invalid for type %d (3D maximum 512 x 512 x 256; other types require depth 1)",
            bitmap->width,bitmap->height,bitmap->depth,bitmap->type);
    if (texture->channels && (texture->channels&(texture->channels-1))) {
        return mcc_diagnostic_fail(runtime,"conflicting pixel-channel uses (mask %u)",texture->channels);
    }
    mcc_bitmap_complete_mips(&normalized);
    for (level=0;level<=(unsigned)normalized.mipmap_count;level++) {
        unsigned w=MAX(bitmap->width>>level,1),h=MAX(bitmap->height>>level,1),d=MAX(bitmap->depth>>level,1);
        uint64_t slice=bytes_per_pixel ? (uint64_t)w*h*bytes_per_pixel :
            (uint64_t)((w+3)/4)*((h+3)/4)*(bitmap->format==14 ? 8 : 16);
        raw_size+=slice*d*faces;
        decoded_size+=(uint64_t)w*h*d*faces*4;
    }
    if (raw_size>(uint32_t)bitmap->pixels_size)
        return mcc_diagnostic_fail(runtime,"pixel payload is too short: %lu bytes available, %llu required for format %d, %d x %d x %d, %d mipmaps",
            (unsigned long)bitmap->pixels_size,(unsigned long long)raw_size,bitmap->format,
            bitmap->width,bitmap->height,bitmap->depth,bitmap->mipmap_count);
    if (raw_size>MCC_BITMAP_ALLOCATION_LIMIT || (convert && decoded_size>MCC_BITMAP_ALLOCATION_LIMIT))
        return mcc_diagnostic_fail(runtime,"texture exceeds the 128 MiB allocation limit (source %llu bytes, decoded %llu bytes)",
            (unsigned long long)raw_size,(unsigned long long)decoded_size);
    if (non_power_two) {
        /* Preserve arbitrary MCC HUD dimensions using the engine's linear
         * ARGB layout. Validate the entire declared source chain above,
         * then retain its base level: Xbox linear textures have no mips. */
        if (decoded_size>MCC_BITMAP_ALLOCATION_LIMIT)
            return mcc_diagnostic_fail(runtime,"non-power-of-two texture needs %llu decoded bytes; limit is 128 MiB",(unsigned long long)decoded_size);
        convert=1;normalized.flags=(normalized.flags&~1u)|16u;
        normalized.mipmap_count=0;
        decoded_size=(uint64_t)(unsigned)bitmap->width*(unsigned)bitmap->height*4;
    }
    if (normalized.flags&0x1000) {
        struct bitmap_group *group=mcc_bitmap_tag(runtime,texture->handle,BITMAP_GROUP_TAG,sizeof(*group));
        if (!group) return mcc_diagnostic_fail(runtime,"bitmap tag header is outside loaded data");
        if (!mcc_bitmap_lightmap_flags(&normalized,group->usage))
            return mcc_diagnostic_fail(runtime,"unsupported lightmap layout: usage %d, flags 0x%04x, format %d, type %d, dimensions %d x %d x %d, mipmaps %d, bytes %ld; bit 12 requires a tightly packed RGB565 lightmap",
                group->usage,bitmap->flags,bitmap->format,bitmap->type,bitmap->width,bitmap->height,bitmap->depth,bitmap->mipmap_count,(long)bitmap->pixels_size);
    }
    if (convert) {
        normalized.format=11;
        normalized.flags&=~(2|4|8|32|0x100);
    } else normalized.flags&=~(8|0x100);
    /* CEA bit 9 marks environment/lightmap use (Invader's public bitmap
     * schema). It is not a pixel-layout flag and has no Xbox counterpart. */
    normalized.flags&=~0x200;
    normalized.flags=(normalized.flags&~64)|128;
    /* MCC raw flags are not an Xbox hardware descriptor. Derive the
     * compressed/palette bits from the normalized pixel format. */
    normalized.flags&=~(2|4);
    if (normalized.format>=14 && normalized.format<=16) normalized.flags|=2;
    if (normalized.format==17) normalized.flags|=4;
    if (!bitmap_verify(&normalized,FALSE)) {
        char const *check="format/type dimension or mipmap constraint";
        if (normalized.signature!=BITMAP_GROUP_TAG) check="bitmap signature is not bitm";
        else if (normalized.flags&~0x1FFu) check="unsupported normalized bitmap flags";
        else if (normalized.mipmap_count>floor_log2(MAX(normalized.width,MAX(normalized.height,normalized.depth)))) check="mipmap count exceeds the texture dimensions";
        return mcc_diagnostic_fail(runtime,"normalized descriptor failed bitmap validation (%s): signature %08lx, flags 0x%04x, format %d, type %d, dimensions %d x %d x %d, mipmaps %d",
            check,(unsigned long)normalized.signature,normalized.flags,normalized.format,normalized.type,
            normalized.width,normalized.height,normalized.depth,normalized.mipmap_count);
    }
    if (!(normalized.flags&16) &&
        ((normalized.width&(normalized.width-1)) || (normalized.height&(normalized.height-1)) ||
         (normalized.depth&(normalized.depth-1))))
        return mcc_diagnostic_fail(runtime,"nonlinear texture dimensions must be powers of two");
    if ((normalized.flags&16) && (normalized.type!=0 || normalized.mipmap_count || (normalized.flags&2)))
        return mcc_diagnostic_fail(runtime,"linear layout requires a 2D uncompressed texture with no mipmaps");
    hardware_size=(uint32_t)rasterizer_xbox_bitmap_get_pixel_data_size(&normalized);
    /* The generic bitmap verifier includes the tiny DXT mip tail, whereas
     * native hardware stops at a 4-texel dimension. Own enough storage for
     * both contracts, including the 2D tail sampled by CPU object lighting. */
    allocation=(MAX(hardware_size,(uint32_t)(convert ? decoded_size : raw_size))+127u)&~127u;
    /* The cache read contract includes the bitmap tag handle. Give each tag
     * its own bounded virtual stream; the aggregate decoded storage can then
     * exceed the old 512 MiB offset window without signed offsets or aliases
     * between descriptors within one tag. Reads still require exact handles. */
    if (!hardware_size || allocation>MCC_BITMAP_ALLOCATION_LIMIT ||
        stream_offset>MCC_BITMAP_LIMIT || textures->end>MCC_BITMAP_STORAGE_LIMIT ||
        allocation>MCC_BITMAP_LIMIT-stream_offset || allocation>MCC_BITMAP_STORAGE_LIMIT-textures->end) {
        error(_error_silent,"mcc bitmap: storage limit (used %lu, requested %lu)",textures->end,allocation);
        return mcc_diagnostic_fail(runtime,"hardware texture storage limit exceeded (hardware %lu, allocation %lu, tag stream %lu, total %lu bytes)",
            (unsigned long)hardware_size,(unsigned long)allocation,(unsigned long)stream_offset,(unsigned long)textures->end);
    }
    raw=malloc((size_t)raw_size);
    if (!raw) goto out_of_memory;
    if (bitmap->flags&0x100) {
        uint32_t *entry=(uint32_t *)(runtime->tag_index+(texture->resource_handle&0xFFFF)*32);
        char const *name=mcc_runtime_pointer(runtime,entry[4],256);
        char resource_name[272],group_name[272];
        if (!name || !memchr(name,0,256)) {reason="external bitmap resource name is outside loaded data or exceeds 255 characters";goto failed;}
        snprintf(resource_name,sizeof(resource_name),"%s_%lu",name,(unsigned long)texture->resource_index);
        snprintf(group_name,sizeof(group_name),"%s__pixels",name);
        if (!mcc_resources_read(runtime,1,resource_name,bitmap->pixels_offset,(uint32_t)raw_size,raw,group_name)) {
            error(_error_silent,"mcc bitmap: external '%s' does not match the selected MCC bitmaps.map",resource_name);
            reason="external pixel payload could not be read or matched in the selected MCC bitmaps.map; check the package's matching resources";
            goto failed;
        }
    } else if (!mcc_runtime_read(runtime,bitmap->pixels_offset,(uint32_t)raw_size,raw)) {
        reason="embedded pixel payload could not be read from the map";goto failed;
    }
    if (convert) {
        pixels=malloc(allocation);
        if (!pixels) goto out_of_memory;
        memset(pixels,0,allocation);
        for (level=0;level<=(unsigned)normalized.mipmap_count;level++) {
            unsigned w=MAX(bitmap->width>>level,1),h=MAX(bitmap->height>>level,1),d=MAX(bitmap->depth>>level,1),slice;
            unsigned source_bytes=bytes_per_pixel ? w*h*bytes_per_pixel : ((w+3)/4)*((h+3)/4)*(bitmap->format==14 ? 8 : 16);
            for (slice=0;slice<faces*d;slice++) {
                mcc_bitmap_decode(raw+input_offset,(uint32_t *)(pixels+output_offset),w,h,bitmap->format,texture->channels);
                input_offset+=source_bytes;output_offset+=w*h*4;
            }
        }
    } else {pixels=raw;raw=NULL;}
    if (raw) free(raw);
    raw=NULL;
    hardware=malloc(allocation);
    if (!hardware) goto out_of_memory;
    memset(hardware,0,allocation);
    if (!mcc_bitmap_pack(&normalized,pixels,(uint32_t)(convert ? decoded_size : raw_size),hardware,
        normalized.type==0 && (normalized.flags&2) ? allocation : hardware_size)) goto failed;
    free(pixels);pixels=hardware;hardware=NULL;
    if (bitmap->mipmap_count!=normalized.mipmap_count)
        error(_error_silent,non_power_two ? "mcc bitmap: tag %08lx retained linear base level (omitted levels %d through %d)" :
            "mcc bitmap: tag %08lx omitted incomplete DXT1 mip tail (levels %d through %d)",
            texture->handle,normalized.mipmap_count+1,bitmap->mipmap_count);
    bitmap->format=normalized.format;
    bitmap->flags=normalized.flags;
    bitmap->mipmap_count=normalized.mipmap_count;
    bitmap->pixels_offset=MCC_BITMAP_BASE+stream_offset;
    bitmap->pixels_size=allocation;
    bitmap->tag_index=texture->handle;
    bitmap->cache_block_index=NONE;
    bitmap->hardware_format=NULL;
    bitmap->base_address=NULL;
    texture->offset=bitmap->pixels_offset;texture->size=allocation;texture->pixels=pixels;
    textures->end+=allocation;
    textures->stream_end=MAX(textures->stream_end,stream_offset+allocation);
    return 1;
out_of_memory:
    error(_error_silent,"mcc bitmap: out of memory for tag %08lx (stored %lu, texture %lu bytes)",
        texture->handle,textures->end,allocation);
    reason="not enough memory for texture conversion";
failed:
    if (raw) free(raw);
    if (pixels) free(pixels);
    if (hardware) free(hardware);
    return mcc_diagnostic_fail(runtime,"%s (pixel offset 0x%08lx, source bytes %llu, allocation %lu)",reason,
        (unsigned long)bitmap->pixels_offset,(unsigned long long)raw_size,(unsigned long)allocation);
}

int mcc_bitmaps_prepare(struct mcc_runtime *runtime)
{
    struct mcc_textures *textures;
    uint32_t index,original_count=runtime->report.tag_count,stream_offset=0;
    unsigned char *new_index;
    mcc_diagnostic_tag(runtime,UINT32_MAX);
    mcc_diagnostic_at(runtime,"bitmap conversion setup");
    textures=malloc(sizeof(*textures));
    if (!textures) return mcc_diagnostic_fail(runtime,"not enough memory for bitmap conversion state");
    memset(textures,0,sizeof(*textures));
    runtime->bitmaps=textures;
    /* three entries a bitmap: its own and its channel clones' */
    if (runtime->report.bitmap_count>SIZE_MAX/3/sizeof(*textures->items)) return 0;
    textures->capacity=runtime->report.bitmap_count*3;
    textures->tag_capacity=MIN(original_count*3,0xFFFFu);
    new_index=mcc_runtime_allocate(runtime,textures->tag_capacity*32);
    if (!new_index) return mcc_diagnostic_fail(runtime,"not enough tag memory for expanded bitmap tag table");
    memcpy(new_index,runtime->tag_index,original_count*32);
    runtime->tag_index=new_index;
    textures->clones=malloc(textures->tag_capacity*sizeof(*textures->clones));
    if (!textures->clones) return mcc_diagnostic_fail(runtime,"not enough memory for bitmap copy table");
    if (runtime->report.bitmap_count) {
        textures->items=malloc(textures->capacity*sizeof(*textures->items));
        if (!textures->items) return mcc_diagnostic_fail(runtime,"not enough memory for bitmap descriptor table");
        memset(textures->items,0,textures->capacity*sizeof(*textures->items));
    }
    for (index=0;index<original_count;index++) {
        uint32_t *entry=(uint32_t *)(runtime->tag_index+index*0x20), n;
        struct bitmap_group *group;
        struct bitmap_data *bitmaps;
        if (entry[0]!=BITMAP_GROUP_TAG) continue;
        mcc_diagnostic_tag(runtime,entry[3]);
        mcc_diagnostic_at(runtime,"bitmaps");
        group=mcc_runtime_pointer(runtime,entry[5],sizeof(*group));
        if (!group || !(bitmaps=mcc_bitmap_block(runtime,&group->bitmaps,sizeof(*bitmaps)))) return mcc_diagnostic_fail(runtime,"bitmap header or bitmaps block is outside loaded data");
        for (n=0;n<(uint32_t)group->bitmaps.count;n++) {
            struct mcc_texture *texture;
            if (textures->count>=runtime->report.bitmap_count) return mcc_diagnostic_fail(runtime,"bitmap count exceeds the count validated in the original map");
            texture=&textures->items[textures->count++];
            texture->bitmap=&bitmaps[n];texture->handle=entry[3];
            texture->resource_handle=entry[3];texture->resource_index=n;
        }
    }
    if (!mcc_bitmap_usages(runtime,textures)) {
        return 0;
    }
    /* Original groups and fresh-handle clones are appended as complete runs.
     * Each runtime handle has its own stream, including clones that share a
     * resource handle. Only successful conversions advance its cursor. */
    for (index=0;index<textures->count;index++) {
        if (!index || textures->items[index].handle!=textures->items[index-1].handle) stream_offset=0;
        if (!mcc_texture_prepare(runtime,textures,&textures->items[index],stream_offset)) {
            return 0;
        }
        stream_offset+=textures->items[index].size;
        mcc_loading_update(MCC_LOADING_BITMAPS, index + 1, textures->count);
    }
    error(_error_silent,"mcc bitmaps: %lu textures converted to %lu bytes",textures->count,textures->end);
    return 1;
}

uint32_t mcc_bitmaps_source_handle(struct mcc_runtime *runtime,uint32_t handle)
{
    struct mcc_textures *textures=runtime->bitmaps;
    uint32_t i;
    for (i=0;textures && i<textures->clone_count;i++)
        if (textures->clones[i].target==handle) return textures->clones[i].source;
    return handle;
}

int mcc_bitmaps_read(struct mcc_runtime *runtime,long tag,uint32_t offset,uint32_t bytes,void *out)
{
    struct mcc_textures *textures=runtime->bitmaps;
    uint32_t index;
    if (offset<MCC_BITMAP_BASE) return -1;
    for (index=0;textures && index<textures->count;index++) {
        struct mcc_texture *texture=&textures->items[index];
        uint32_t position=offset-texture->offset;
        if ((uint32_t)tag==texture->handle && offset>=texture->offset && position<=texture->size && bytes<=texture->size-position) {
            memcpy(out,texture->pixels+position,bytes); return 1;
        }
    }
    return 0;
}

uint32_t mcc_bitmaps_stream_end(struct mcc_runtime *runtime)
{
    struct mcc_textures *textures=runtime->bitmaps;
    return textures && textures->stream_end ? MCC_BITMAP_BASE+textures->stream_end : (uint32_t)runtime->source.size;
}

int mcc_bitmaps_contains(struct mcc_runtime *runtime,uint32_t offset,uint32_t bytes)
{
    struct mcc_textures *textures=runtime->bitmaps;
    uint32_t index;
    for (index=0;textures && index<textures->count;index++) {
        struct mcc_texture const *texture=&textures->items[index];
        if (texture->pixels && offset>=texture->offset && offset-texture->offset<=texture->size &&
            bytes<=texture->size-(offset-texture->offset)) return 1;
    }
    return 0;
}

static struct mcc_texture const *mcc_bitmap_valid_texture(struct mcc_textures const *textures,
    struct bitmap_data const *bitmap)
{
    uint32_t index;
    for (index=0;textures && index<textures->count;index++) {
        struct mcc_texture const *texture=&textures->items[index];
        if (texture->bitmap!=bitmap) continue;
        return texture->pixels && (uint32_t)bitmap->tag_index==texture->handle &&
            (uint32_t)bitmap->pixels_offset==texture->offset && (uint32_t)bitmap->pixels_size==texture->size &&
            bitmap_verify(texture->bitmap,FALSE) &&
            rasterizer_xbox_bitmap_get_pixel_data_size(texture->bitmap)<=(long)texture->size ? texture : NULL;
    }
    return NULL;
}

int mcc_bitmaps_valid(struct mcc_runtime *runtime,struct bitmap_data *bitmap)
{
    return mcc_bitmap_valid_texture(runtime->bitmaps,bitmap)!=NULL;
}

/* MCC pixels are owned allocations, not the legacy file stream (whose
 * per-bitmap budget is smaller). Structural validation precedes this call. */
int mcc_bitmaps_group_valid(struct mcc_runtime *runtime,void *address)
{
    struct bitmap_group *group=address;
    long i;
    if (group->pixel_data.file_offset!=0) return 0;
    for (i=0;i<group->bitmaps.count;i++)
        if (!mcc_bitmaps_valid(runtime,(struct bitmap_data *)group->bitmaps.address+i)) return 0;
    for (i=0;i<group->sequences.count;i++) {
        struct bitmap_group_sequence *sequence=(struct bitmap_group_sequence *)group->sequences.address+i;
        if (sequence->first_bitmap_index<0 || sequence->bitmap_count<0 ||
            sequence->first_bitmap_index>group->bitmaps.count ||
            sequence->bitmap_count>group->bitmaps.count-sequence->first_bitmap_index) {
            error(_error_silent,"mcc bitmap: sequence %ld outside converted bitmap group; disabled",i);
            sequence->first_bitmap_index=sequence->bitmap_count=0;
        }
    }
    return 1;
}

/* The renderer may bind the immutable MCC allocation directly. A copied
 * descriptor, another map's bitmap, or a changed stream range cannot borrow
 * this storage. The caller receives bytes only after ownership validation. */
void const *mcc_bitmaps_pixels(struct mcc_runtime *runtime,struct bitmap_data const *bitmap,uint32_t *bytes,int *coverage)
{
    struct mcc_textures const *textures=runtime ? runtime->bitmaps : NULL;
    struct mcc_texture const *texture;
    if (bytes) *bytes=0;
    if (coverage) *coverage=0;
    if (!textures || !bitmap) return NULL;
    texture=mcc_bitmap_valid_texture(textures,bitmap);
    if (!texture) return NULL;
    if (bytes) *bytes=texture->size;
    if (coverage) *coverage=texture->channels==4;
    return texture->pixels;
}

#ifndef HALO_GLES
static int mcc_bitmap_prepare_order(void const *left,void const *right)
{
    struct mcc_texture const *a=*(struct mcc_texture const *const *)left;
    struct mcc_texture const *b=*(struct mcc_texture const *const *)right;
    return a->size>b->size ? -1 : a->size<b->size ? 1 : 0;
}
#endif

/* Only this module can enumerate the validated, converted MCC allocations.
 * Register through the existing adapter; no Xbox/CE bitmap cache is touched. */
void mcc_bitmaps_prepare_gpu(struct mcc_runtime *runtime)
{
#ifndef HALO_GLES
    struct mcc_textures *textures=runtime ? runtime->bitmaps : NULL;
    struct mcc_texture **ordered;
    long megabytes=config_integer("display.mcc_texture_prepare_mb");
    unsigned long budget,spent=0,prepared=0;
    uint32_t index,count=0;
    if (!textures || megabytes<=0 || config_boolean("debug.null_renderer") || config_server_mode_enabled()) return;
    if (megabytes>512) megabytes=512;
    budget=(unsigned long)megabytes*1024*1024;
    if (textures->count>SIZE_MAX/sizeof(*ordered)) return;
    ordered=malloc(textures->count*sizeof(*ordered));
    if (!ordered) return;
    for (index=0;index<textures->count;index++) {
        struct mcc_texture *texture=&textures->items[index];
        if (texture->size>=256*1024 && texture->bitmap->format!=17) ordered[count++]=texture;
    }
    qsort(ordered,count,sizeof(*ordered),mcc_bitmap_prepare_order);
    for (index=0;index<count && budget-spent>=256*1024;index++) {
        unsigned long charge;
        if (!mcc_texture_cache_get(ordered[index]->bitmap,TRUE)) continue;
        charge=mcc_texture_bridge_prepare(ordered[index]->bitmap,budget-spent);
        spent+=charge;
        if (charge) prepared++;
        mcc_loading_pulse();
    }
    free(ordered);
    error(_error_silent,"mcc textures: prepared %lu large textures (%lu KiB estimated GPU storage)",prepared,spent/1024);
#else
    (void)runtime;
#endif
}

void mcc_bitmaps_dispose(struct mcc_runtime *runtime)
{
    struct mcc_textures *textures=runtime->bitmaps;
    uint32_t index;
    if (!textures) return;
    for (index=0;index<textures->count;index++) {
        struct mcc_texture *texture=&textures->items[index];
        if (texture->bitmap && texture->bitmap->base_address==texture->pixels)
            texture->bitmap->base_address=NULL;
        if (texture->pixels) free(texture->pixels);
    }
    if (textures->items) free(textures->items);
    if (textures->clones) free(textures->clones);
    free(textures);runtime->bitmaps=NULL;
}

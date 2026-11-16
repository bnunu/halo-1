/* MCC HUD normalization. Unflagged placements use a 960p canvas; legacy
 * high-resolution placements retain their 480p offsets. Bitmap half-HUD flags
 * apply independently of anchor conversion. Keep the original bitmap pixels and sprite texture coordinates. */
#include "cseries.h"
#include "bitmaps/bitmap_group.h"
#include "interface/unit_hud_interface_definition.h"
#include "interface/weapon_hud_interface_definition.h"
#include "errors.h"
#include "mcc_runtime.h"
#include "mcc_hud_draw.h"
#include <stdlib.h>
#include <string.h>

char const *config_string(char const *name);

/* MCC's PC score hint contains one formatting argument. The native HUD
 * copies this string verbatim, so resolve that argument in owned UTF-16
 * storage. Keep the configured keyboard binding and the controller label. */
static int mcc_hud_score_text(uint16_t const *source,uint32_t characters,
    char const *binding,uint16_t *out,uint32_t capacity)
{
    uint32_t length=0,placeholder=UINT32_MAX,i,used=0,key_length=0,needed;
    char key[49];
    static char const controller[]="BACK";
    if (!source || !characters) return -1;
    while (length<characters && source[length]) length++;
    if (length==characters) return -1;
    for (i=0;i+1<length;i++) {
        if (source[i]=='%' && source[i+1]=='s') {placeholder=i;break;}
    }
    if (placeholder==UINT32_MAX) return 0;
    if (binding) {
        while (*binding==' ') binding++;
        while (*binding && *binding!=',' && key_length<sizeof(key)-1) {
            unsigned char c=(unsigned char)*binding++;
            key[key_length++]=(char)(c>=32 && c<127 ? c : '?');
        }
        while (key_length && key[key_length-1]==' ') key_length--;
    }
    /* The additional text, excluding the two source characters and NUL. */
    needed=length-2+4+(key_length ? key_length+3 : 0)+1;
    if (!out) return (int)needed;
    if (capacity<needed) return -1;
    for (i=0;i<placeholder;i++) out[used++]=source[i];
    for (i=0;i<key_length;i++) out[used++]=(unsigned char)key[i];
    if (key_length) {out[used++]=' ';out[used++]='/';out[used++]=' ';}
    for (i=0;i<sizeof(controller)-1;i++) out[used++]=controller[i];
    for (i=placeholder+2;i<=length;i++) out[used++]=source[i];
    return (int)used;
}

static uint32_t mcc_hud_word(void const *p)
{
    uint32_t n;memcpy(&n,p,4);return n;
}

static unsigned char *mcc_hud_block(struct mcc_runtime *r,unsigned char *p,uint32_t stride,uint32_t *count)
{
    *count=mcc_hud_word(p);
    if (*count>r->used/stride) return NULL;
    return *count ? mcc_runtime_pointer(r,mcc_hud_word(p+4),*count*stride) : r->tags;
}

#include "mcc_hud_layout.inl"
#include "mcc_hud_ownership.inl"

static int mcc_hud_multiplayer_text(struct mcc_runtime *r,uint32_t const *entry)
{
    static char const name[]="ui\\multiplayer_game_text";
    char const *actual=mcc_runtime_pointer(r,entry[4],sizeof(name));
    unsigned char *root,*strings,*hint;
    uint16_t const *source;
    uint16_t *replacement;
    uint32_t count,bytes,address;
    int characters;
    if (!actual || memcmp(actual,name,sizeof(name))) return 1;
    root=mcc_runtime_pointer(r,entry[5],12);
    if (!root) return mcc_diagnostic_fail(r,"unicode string list header is outside loaded tag data");
    strings=mcc_hud_block(r,root,20,&count);
    if (!strings) return mcc_diagnostic_fail(r,"unicode strings block is outside loaded tag data");
    if (count<=100) return 1;
    mcc_diagnostic_at(r,"strings[100] (scoreboard hint)");
    hint=strings+100*20;bytes=mcc_hud_word(hint);
    if (!bytes) return 1; /* Maps may deliberately omit the score hint. */
    if (bytes<2 || (bytes&1)) return mcc_diagnostic_fail(r,"UTF-16 string size %lu must be even and at least 2 bytes",(unsigned long)bytes);
    source=mcc_runtime_pointer(r,mcc_hud_word(hint+12),bytes);
    if (!source || ((uintptr_t)source&1)) return mcc_diagnostic_fail(r,"UTF-16 string is misaligned or outside loaded data");
    characters=mcc_hud_score_text(source,bytes/2,config_string("controls.scoreboard"),NULL,0);
    if (characters<0) return mcc_diagnostic_fail(r,"scoreboard hint has no terminating UTF-16 NUL");
    if (!characters) return 1;
    replacement=mcc_runtime_allocate(r,(uint32_t)characters*2);
    if (!replacement || mcc_hud_score_text(source,bytes/2,config_string("controls.scoreboard"),
        replacement,(uint32_t)characters)!=characters) return mcc_diagnostic_fail(r,"could not allocate or format the scoreboard hint");
    bytes=(uint32_t)characters*2;address=(uint32_t)(uintptr_t)replacement;
    memcpy(hint,&bytes,4);memcpy(hint+12,&address,4);
    return 1;
}

static int mcc_hud_half_bitmap(struct mcc_runtime *r,uint32_t handle,int *half)
{
    uint32_t *entry;
    struct bitmap_group *bitmap;
    *half=0;
    if (handle==0xFFFFFFFFu) return 1;
    if ((handle&0xFFFF)>=r->report.tag_count) return mcc_diagnostic_fail(r,"bitmap reference %08lx is outside the tag table",(unsigned long)handle);
    entry=(uint32_t *)(r->tag_index+(handle&0xFFFF)*32);
    if (entry[0]!='bitm' || entry[3]!=handle) return mcc_diagnostic_fail(r,"reference %08lx does not identify a bitmap tag",(unsigned long)handle);
    bitmap=mcc_runtime_pointer(r,entry[5],sizeof(*bitmap));
    if (!bitmap) return mcc_diagnostic_fail(r,"referenced bitmap header is outside loaded data");
    *half=(bitmap->flags&(0x10|0x80))!=0;
    return 1;
}

static void mcc_hud_offset(struct hud_placement_definition *placement)
{
    placement->pad=0;
    /* Legacy high-resolution placements keep their 480p anchor offsets.
     * Other MCC placements use the doubled canvas. Bitmap resolution is
     * independent of this flag. Consume it before native drawing. */
    if (!(placement->multiplayer_scaling_flags&4)) {
        placement->offset.x/=2;
        placement->offset.y/=2;
    }
    placement->multiplayer_scaling_flags&=~4;
}

/* Stock messages sit 60 native pixels below the safe-frame corner. Match
 * that placement, not the font, icon sheet or tag name: those may be replaced
 * independently. Other authored offsets and edge/center anchors stay MCC. */
static void mcc_hud_message_frame(struct hud_messaging_parameters_definition *message)
{
    if (message->absolute_placement.corner==0 && !message->placement.offset.x &&
        message->placement.offset.y==60)
        message->placement.pad=MCC_HUD_NATIVE_FRAME;
}

static void mcc_hud_scale(struct hud_placement_definition *placement,int half)
{
    float factor=half ? 0.25f : 0.5f;
    mcc_hud_offset(placement);
    placement->scale.i*=factor;
    placement->scale.j*=factor;
}

static int mcc_hud_bitmap_element(struct mcc_runtime *r,unsigned char *element,short marker)
{
    int half;
    if (!mcc_hud_half_bitmap(r,mcc_hud_word(element+0x30),&half)) return 0;
    mcc_hud_scale((void *)element,half);
    ((struct hud_placement_definition *)element)->pad=marker;
    return 1;
}

static int mcc_hud_items(struct mcc_runtime *r,unsigned char *definition,uint32_t stride,short marker)
{
    uint32_t i,count;
    unsigned char *items=mcc_hud_block(r,definition+16,stride,&count);
    int half;
    if (!items) return mcc_diagnostic_fail(r,"HUD overlay items block is outside loaded tag data");
    if (!mcc_hud_half_bitmap(r,mcc_hud_word(definition+12),&half)) return 0;
    for (i=0;i<count;i++) {
        struct hud_placement_definition *placement=(void *)(items+i*stride);
        mcc_hud_scale(placement,half);placement->pad=marker;
    }
    return 1;
}

/* MCC can omit the unarmed HUD. The native update path requires a tag even
 * between client spawn and receipt of its weapons. An empty owned definition
 * preserves the omitted visuals without changing the shared HUD path. */
static int mcc_hud_default(struct mcc_runtime *r,struct hud_globals_definition *globals,uint32_t *empty)
{
    static char const name[]="mcc_runtime\\unarmed_hud";
    struct weapon_hud_interface_definition *hud;
    unsigned char *index;
    uint32_t *entry,count=r->report.tag_count;
    char *owned_name;
    if (globals->defaults.default_weapon_hud.index!=NONE) return 1;
    if (*empty==UINT32_MAX) {
        /* Native tag lookup uses a signed 16-bit absolute index. */
        if (count>32767) return mcc_diagnostic_fail(r,"cannot add omitted default_weapon_hud: tag count exceeds the signed 16-bit index limit");
        index=mcc_runtime_allocate(r,(count+1)*32);
        hud=mcc_runtime_allocate(r,sizeof(*hud));
        owned_name=mcc_runtime_allocate(r,sizeof(name));
        if (!index || !hud || !owned_name) return mcc_diagnostic_fail(r,"not enough tag memory for the omitted default_weapon_hud");
        memcpy(index,r->tag_index,count*32);
        memcpy(owned_name,name,sizeof(name));
        hud->parent_hud.group_tag='wphi';hud->parent_hud.index=NONE;
        hud->absolute_placement.corner=_hud_anchor_center;
        *empty=0xE1740000u|count;
        entry=(uint32_t *)(index+count*32);
        entry[0]='wphi';entry[1]=entry[2]=UINT32_MAX;entry[3]=*empty;
        entry[4]=(uint32_t)(uintptr_t)owned_name;entry[5]=(uint32_t)(uintptr_t)hud;
        r->tag_index=index;r->report.tag_count=count+1;
        error(_error_silent,"mcc hud: omitted unarmed HUD represented by an empty definition");
    }
    globals->defaults.default_weapon_hud.group_tag='wphi';
    globals->defaults.default_weapon_hud.index=(long)*empty;
    return 1;
}

static int mcc_hud_convert(struct mcc_runtime *r,struct mcc_hud_plan const *plan)
{
    uint32_t i,empty=UINT32_MAX;
    for (i=0;i<r->report.tag_count;i++) {
        uint32_t *entry=(uint32_t *)(r->tag_index+i*32),n,count;
        short marker=i<plan->tag_count && plan->frames[i]==1 ? MCC_HUD_NATIVE_FRAME : 0;
        unsigned char *p,*block;
        if (i<plan->tag_count && plan->frames[i]==2) continue;
        mcc_diagnostic_tag(r,entry[3]);
        mcc_diagnostic_at(r,"HUD layout");
        if (entry[0]=='unhi') {
            static unsigned const elements[]={0x24,0x8C,0xF4,0x17C,0x1E4,0x26C,0x2D4};
            p=mcc_runtime_pointer(r,entry[5],0x56C);
            if (!p) return mcc_diagnostic_fail(r,"HUD tag header is outside loaded data");
            memcpy(p+2,&marker,2);
            for (n=0;n<NUMBEROF(elements);n++) {
                static char const *const names[]={"background","shield_panel.background","shield_panel.meter","health_panel.background","health_panel.meter","motion_sensor.background","motion_sensor.foreground"};
                mcc_diagnostic_at(r,"%s.bitmap",names[n]);
                if (!mcc_hud_bitmap_element(r,p+elements[n],marker)) return 0;
            }
            mcc_hud_offset((void *)(p+0x35C)); /* radar blip anchor */
            ((struct hud_placement_definition *)(p+0x35C))->pad=marker;
            mcc_diagnostic_at(r,"auxiliary_overlays");
            block=mcc_hud_block(r,p+0x3A4,0x84,&count);
            if (!block) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
            for (n=0;n<count;n++) {
                mcc_diagnostic_at(r,"auxiliary_overlays[%lu].bitmap",(unsigned long)n);
                if (!mcc_hud_bitmap_element(r,block+n*0x84,marker)) return 0;
            }
            mcc_diagnostic_at(r,"auxiliary_meters");
            block=mcc_hud_block(r,p+0x3CC,0x144,&count);
            if (!block) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
            for (n=0;n<count;n++) {
                mcc_diagnostic_at(r,"auxiliary_meters[%lu].background.bitmap",(unsigned long)n);
                if (!mcc_hud_bitmap_element(r,block+n*0x144+0x14,marker)) return 0;
                mcc_diagnostic_at(r,"auxiliary_meters[%lu].meter.bitmap",(unsigned long)n);
                if (!mcc_hud_bitmap_element(r,block+n*0x144+0x7C,marker)) return 0;
            }
        } else if (entry[0]=='wphi') {
            unsigned k;
            p=mcc_runtime_pointer(r,entry[5],0x17C);
            if (!p) return mcc_diagnostic_fail(r,"HUD tag header is outside loaded data");
            memcpy(p+0x3E,&marker,2);
            for (k=0;k<2;k++) {
                mcc_diagnostic_at(r,"%s_elements",k ? "meter" : "static");
                block=mcc_hud_block(r,p+0x60+k*12,0xB4,&count);
                if (!block) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
                for (n=0;n<count;n++) {
                    mcc_diagnostic_at(r,"%s_elements[%lu].bitmap",k ? "meter" : "static",(unsigned long)n);
                    if (!mcc_hud_bitmap_element(r,block+n*0xB4+0x24,marker)) return 0;
                }
                mcc_diagnostic_at(r,"%s_overlays",k ? "screen" : "crosshair");
                block=mcc_hud_block(r,p+0x84+k*12,0x68,&count);
                if (!block) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
                for (n=0;n<count;n++) {
                    mcc_diagnostic_at(r,"%s_overlays[%lu]",k ? "screen" : "crosshair",(unsigned long)n);
                    if (!mcc_hud_items(r,block+n*0x68+0x24,k ? 0x88 : 0x6C,k ? marker : 0)) return 0;
                }
            }
            mcc_diagnostic_at(r,"number_elements");
            block=mcc_hud_block(r,p+0x78,0xA0,&count);
            if (!block) return mcc_diagnostic_fail(r,"HUD block count or address is outside loaded tag data");
            for (n=0;n<count;n++) {
                struct hud_placement_definition *placement=(void *)(block+n*0xA0+0x24);
                mcc_hud_offset(placement);placement->pad=marker;
            }
        } else if (entry[0]=='grhi') {
            p=mcc_runtime_pointer(r,entry[5],0x1F8);
            if (!p) return mcc_diagnostic_fail(r,"grenade HUD header is outside loaded data");
            mcc_diagnostic_at(r,"background.bitmap");
            if (!mcc_hud_bitmap_element(r,p+0x24,marker)) return 0;
            mcc_diagnostic_at(r,"total_grenades.background.bitmap");
            if (!mcc_hud_bitmap_element(r,p+0x8C,marker)) return 0;
            mcc_diagnostic_at(r,"overlays");
            if (!mcc_hud_items(r,p+0x14C,0x88,marker)) return 0;
            memcpy(p+2,&marker,2);
            mcc_hud_offset((void *)(p+0xF4));
            ((struct hud_placement_definition *)(p+0xF4))->pad=marker;
        } else if (entry[0]=='hudg') {
            struct hud_globals_definition *globals=mcc_runtime_pointer(r,entry[5],sizeof(*globals));
            if (!globals) return mcc_diagnostic_fail(r,"HUD globals header is outside loaded data");
            if (!mcc_hud_default(r,globals,&empty)) return 0;
            mcc_hud_offset(&globals->messaging.placement);
            mcc_hud_message_frame(&globals->messaging);
            globals->waypoint.top_offset*=0.5f;
            globals->waypoint.bottom_offset*=0.5f;
            globals->waypoint.left_offset*=0.5f;
            globals->waypoint.right_offset*=0.5f;
            globals->damage_indicators.top_offset/=2;
            globals->damage_indicators.bottom_offset/=2;
            globals->damage_indicators.left_offset/=2;
            globals->damage_indicators.right_offset/=2;
        } else if (entry[0]=='hud#') {
            /* The number renderer owns bitmap scaling; these are screen
             * advances, not sprite texture coordinates. */
            p=mcc_runtime_pointer(r,entry[5],0x64);
            if (!p) return mcc_diagnostic_fail(r,"HUD tag header is outside loaded data");
            ((signed char *)p)[0x11]/=2; /* screen_width */
            ((signed char *)p)[0x14]/=2; /* decimal_point_width */
        } else if (entry[0]=='bitm') {
            struct bitmap_group *bitmap=mcc_runtime_pointer(r,entry[5],sizeof(*bitmap));
            if (!bitmap) return mcc_diagnostic_fail(r,"referenced bitmap header is outside loaded data");
            /* Native number drawing already honors half-HUD-scale; MCC's
             * force-highres spelling has the same effect for these digits. */
            if (bitmap->flags&0x80) bitmap->flags|=0x10;
        } else if (entry[0]=='ustr') {
            if (!mcc_hud_multiplayer_text(r,entry)) return 0;
        }
    }
    return 1;
}

int mcc_hud_bitmaps_prepare(struct mcc_runtime *r)
{
    struct mcc_hud_plan plan={0};
    int ok=mcc_hud_plan_prepare(&plan,r) && mcc_hud_convert(r,&plan);
    if (ok) mcc_hud_plan_report(&plan);
    mcc_hud_plan_dispose(&plan);
    return ok;
}

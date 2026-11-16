/* Only diagnostics: never change acceptance, resource selection or tag data. */
#include "cseries.h"
#include "errors.h"
#include "mcc_runtime.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char const *mcc_tag_extension(uint32_t group)
{
    switch (group) {
    case 'bitm': return "bitmap";
    case 'snd!': return "sound";
    case 'scnr': return "scenario";
    case 'sbsp': return "scenario_structure_bsp";
    case 'mod2': return "gbxmodel";
    case 'mode': return "model";
    case 'unhi': return "unit_hud_interface";
    case 'wphi': return "weapon_hud_interface";
    case 'grhi': return "grenade_hud_interface";
    case 'hudg': return "hud_globals";
    case 'hud#': return "hud_number";
    case 'ustr': return "unicode_string_list";
    case 'weap': return "weapon";
    case 'jpt!': return "damage_effect";
    case 'DeLa': return "ui_widget_definition";
    case 'soso': return "shader_model";
    case 'senv': return "shader_environment";
    case 'scex': return "shader_transparent_chicago_extended";
    case 'schi': return "shader_transparent_chicago";
    case 'actr': return "actor";
    case 'actv': return "actor_variant";
    case 'ant!': return "antenna";
    case 'antr': return "model_animations";
    case 'bipd': return "biped";
    case 'coll': return "model_collision_geometry";
    case 'cont': return "contrail";
    case 'deca': return "decal";
    case 'dobc': return "detail_object_collection";
    case 'effe': return "effect";
    case 'eqip': return "equipment";
    case 'flag': return "flag";
    case 'fog ': return "fog";
    case 'font': return "font";
    case 'glw!': return "glow";
    case 'itmc': return "item_collection";
    case 'lens': return "lens_flare";
    case 'ligh': return "light";
    case 'lsnd': return "sound_looping";
    case 'mach': return "device_machine";
    case 'matg': return "globals";
    case 'metr': return "meter";
    case 'mgs2': return "light_volume";
    case 'obje': return "object";
    case 'part': return "particle";
    case 'pctl': return "particle_system";
    case 'phys': return "physics";
    case 'plac': return "placeholder";
    case 'pphy': return "point_physics";
    case 'proj': return "projectile";
    case 'scen': return "scenery";
    case 'snde': return "sound_environment";
    case 'sky ': return "sky";
    case 'sotr': return "shader_transparent_generic";
    case 'swat': return "shader_transparent_water";
    case 'sgla': return "shader_transparent_glass";
    case 'smet': return "shader_transparent_meter";
    case 'spla': return "shader_transparent_plasma";
    case 'ssce': return "sound_scenery";
    case 'str#': return "string_list";
    case 'tagc': return "tag_collection";
    case 'trak': return "camera_track";
    case 'vcky': return "virtual_keyboard";
    case 'vehi': return "vehicle";
    case 'wind': return "wind";
    case 'udlg': return "dialogue";
    case 'hmt ': return "hud_message_text";
    case 'ctrl': return "device_control";
    case 'lifi': return "device_light_fixture";
    default: return NULL;
    }
}

static void mcc_diagnostic_name(struct mcc_runtime *runtime,uint32_t const *entry)
{
    struct mcc_diagnostic *d=&runtime->diagnostic;
    uint32_t i;
    char name[256],group[5];
    char const *extension;
    /* Validate each byte, including names ending at the last loaded byte.
     * A bad name must never make reporting the original failure unsafe. */
    for (i=0;i<sizeof(name);i++) {
        char const *p;
        if (entry[4]>UINT32_MAX-i) return;
        p=mcc_runtime_pointer(runtime,entry[4]+i,1);
        if (!p) return;
        name[i]=*p;
        if (!name[i]) break;
        if ((unsigned char)name[i]<32 || (unsigned char)name[i]==127) name[i]='?';
    }
    if (i==sizeof(name)) {memcpy(name+sizeof(name)-4,"...",4);}
    extension=mcc_tag_extension(entry[0]);
    if (extension) snprintf(d->tag,sizeof(d->tag),"%s.%s",name,extension);
    else {
        for (i=0;i<4;i++) {
            unsigned c=(entry[0]>>(24-i*8))&255;
            group[i]=(char)(c>=32 && c<127 ? c : '?');
        }
        group[4]=0;
        snprintf(d->tag,sizeof(d->tag),"%s [group %s]",name,group);
    }
}

void mcc_diagnostic_tag(struct mcc_runtime *runtime, uint32_t handle)
{
    struct mcc_diagnostic *d;
    uint32_t *entry;
    if (!runtime) return;
    d=&runtime->diagnostic;d->handle=handle;
    memcpy(d->location,"root",sizeof("root"));
    memcpy(d->tag,"(tag path unavailable)",sizeof("(tag path unavailable)"));
    if (handle==UINT32_MAX) {memcpy(d->tag,"(map)",sizeof("(map)"));return;}
    if (!runtime->tag_index || (handle&0xFFFFu)>=runtime->report.tag_count) return;
    entry=(uint32_t *)(runtime->tag_index+(handle&0xFFFFu)*32);
    if (entry[3]!=handle) return;
    if (handle==runtime->report.scenario_handle && runtime->original_scenario_tag[0])
        snprintf(d->tag,sizeof(d->tag),"%s",runtime->original_scenario_tag);
    else mcc_diagnostic_name(runtime,entry);
}

void mcc_diagnostic_at(struct mcc_runtime *runtime, char const *format, ...)
{
    va_list args;
    size_t i;
    if (!runtime) return;
    va_start(args,format);
    vsnprintf(runtime->diagnostic.location,sizeof(runtime->diagnostic.location),format,args);
    va_end(args);
    for (i=0;runtime->diagnostic.location[i];i++)
        if ((unsigned char)runtime->diagnostic.location[i]<32 || runtime->diagnostic.location[i]==127)
            runtime->diagnostic.location[i]='?';
}

int mcc_diagnostic_fail(struct mcc_runtime *runtime, char const *format, ...)
{
    char reason[384];
    va_list args;
    size_t i;
    va_start(args,format);
    vsnprintf(reason,sizeof(reason),format,args);
    va_end(args);
    for (i=0;reason[i];i++) if ((unsigned char)reason[i]<32 || reason[i]==127) reason[i]='?';
    if (runtime) {
        struct mcc_diagnostic const *d=&runtime->diagnostic;
        uint32_t original[8];
        uint64_t offset=40+(uint64_t)(d->handle&0xFFFFu)*32;
        /* Failure-only I/O restores source classes such as mod2 -> mode.
         * Never depend on this read succeeding to report the original error. */
        if (d->handle!=UINT32_MAX && runtime->source.read &&
            offset+32<=runtime->report.identity.tag_data_size &&
            offset+runtime->report.identity.tag_data_offset+32<=runtime->source.size &&
            runtime->source.read(runtime->source.context,(uint32_t)offset+runtime->report.identity.tag_data_offset,
                sizeof(original),original) && original[3]==d->handle)
            mcc_diagnostic_name(runtime,original);
        error(_error_silent,"mcc: '%s', %s: %s; map refused (tag %08lx)",
            d->tag[0] ? d->tag : "(map)",d->location[0] ? d->location : "load",
            reason,(unsigned long)d->handle);
        /* The loader owns concise player prompts; these details stay in debug.txt. */
    } else error(_error_silent,"mcc: %s; map refused",reason);
    return 0;
}

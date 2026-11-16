/* MCC catalog text is authored by package makers, not to the fixed line
 * lengths of the original menus. Wrap a draw-local copy in every style. */
#include "cseries.h"
#include "math/integer_math.h"
#include "math/real_math.h"
#include "tag_files/tag_groups.h"
#include "text/draw_string.h"
#include "rasterizer/rasterizer.h"
#include "mcc_maps.h"
#include "mcc_menu_text.h"
#include "../src/workshop_authors.h"
#include <string.h>

static boolean field(long tag, short index)
{
    char const *name, *leaf;
    if (tag == NONE) return FALSE;
    name=tag_get_name(tag); leaf=strrchr(name,'\\'); leaf=leaf ? leaf+1 : name;
    if (!strcmp(name,"pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\network_setup\\mcc_download\\description") ||
        !strcmp(name,"pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\network_setup\\mcc_downloads\\description")) return TRUE;
    if (!strncmp(name,"pc\\mcc_xbox\\",12) && !strcmp(leaf,"status")) return TRUE;
    if (!strcmp(leaf,"mcc_folder_name") || !strcmp(leaf,"mcc_folder_help")) return TRUE;
    if (index < MCC_MAPS_FIRST_DISPLAY_INDEX || index >= MCC_MAPS_FIRST_DISPLAY_INDEX+MCC_MAPS_MAXIMUM) return FALSE;
    return !strcmp(leaf,"mp_map_right_name") || !strcmp(leaf,"mp_map_right_data") || !strcmp(leaf,"lobby_map_name");
}

/* Measure from the left with the horizontal clamp removed. The native
 * renderer clips a long line; it does not wrap it for Classic PC or Stock. */
static int width_of(wchar_t const *text)
{
    rectangle2d measure, ink, cursor;
    set_rectangle2d(&measure,0,0,SHORT_MAX,SHORT_MAX);
    draw_unicode_string_compute_bounds(&measure,text,&ink,&cursor);
    return MAX(cursor.x0,ink.x1);
}

/* MCC's PC-style list callbacks provide a draw-only label, not the catalog
 * title. Fit the narrowest row (Modern: 312, Stock/Classic PC: 330), keeping
 * its native single-line large font and leaving selection arrows unobscured. */
void mcc_menu_text_row(wchar_t text[64])
{
    real_argb_color color={1,1,1,1};
    long font=tag_loaded('font',"ui\\large_ui");
    int used;
    if (font==NONE) return;
    draw_string_set_draw_mode(font,NONE,0,0,&color);
    if (width_of(text)<=310) return;
    for (used=0; used<60 && text[used]; used++) {}
    do {
        text[used]='.'; text[used+1]='.'; text[used+2]='.'; text[used+3]=0;
    } while (width_of(text)>310 && used-- > 0);
}

static int wrap(wchar_t const *text, wchar_t output[512], int width)
{
    int used=0, start=0, space=NONE, i;
    for (i=0; text[i] && used<509; i++) {
        wchar_t c=text[i];
        if (c=='\n' && i && text[i-1]=='\r') continue;
        if (c=='\n') c='\r';
        output[used++]=c; output[used]=0;
        if (c=='\r') { start=used; space=NONE; continue; }
        if (c==' ') space=used-1;
        if (width_of(output+start)>width) {
            if (space>=start) { output[space]='\r'; start=space+1; space=NONE; }
            else if (used-start>1) { output[used-1]='\r'; output[used++]=c; output[used]=0; start=used-1; }
        }
    }
    output[used]=0; return used;
}

static int text_height(wchar_t const *text)
{
    rectangle2d measure, ink, cursor;
    set_rectangle2d(&measure,0,0,SHORT_MAX,SHORT_MAX);
    draw_unicode_string_compute_bounds(&measure,text,&ink,&cursor);
    return MAX(ink.y1,cursor.y1);
}

enum { AUTHOR_TEXT_SIZE=WORKSHOP_AUTHORS_NAME_SIZE+16 };

/* A Workshop map's author under its description in the map lists' panel
 * (workshop_authors.c), once Steam has said: "Publisher" (the account that
 * published the item), then the name on one
 * line, cut short with "..." to fit the width in the current font. 0 for
 * none. UTF-8 to the menus' UTF-16, a character it cannot hold a ?. */
static int author_text(long tag, short index, int width, wchar_t text[AUTHOR_TEXT_SIZE])
{
    char author[WORKSHOP_AUTHORS_NAME_SIZE];
    char const *name, *leaf, *at;
    int used, start;
    if (tag==NONE || index<MCC_MAPS_FIRST_DISPLAY_INDEX || index>=MCC_MAPS_FIRST_DISPLAY_INDEX+MCC_MAPS_MAXIMUM) return 0;
    name=tag_get_name(tag); leaf=strrchr(name,'\\'); leaf=leaf ? leaf+1 : name;
    if (strcmp(leaf,"mp_map_right_data")) return 0;
    if (!workshop_authors_get(mcc_maps_item_id((short)(index-MCC_MAPS_FIRST_DISPLAY_INDEX)),author,sizeof(author))) return 0;
    for (used=0; "Publisher\r"[used]; used++) text[used]="Publisher\r"[used];
    start=used;
    for (at=author; *at && used<AUTHOR_TEXT_SIZE-4; ) {
        unsigned char first=(unsigned char)*at++;
        unsigned long c=first;
        int following=first>=0xF0 ? 3 : first>=0xE0 ? 2 : first>=0xC0 ? 1 : 0;
        if (first>=0x80) {
            c=first&(0x3F>>following);
            for (; following>0 && ((unsigned char)*at&0xC0)==0x80; following--) c=(c<<6)|((unsigned char)*at++&0x3F);
            if (following || first<0xC0 || c>0xFFFF) c='?';
        }
        text[used++]=(wchar_t)c;
    }
    text[used]=0;
    while (width_of(text+start)>width && used>start) {
        used--; text[used]='.'; text[used+1]='.'; text[used+2]='.'; text[used+3]=0;
    }
    return 1;
}

boolean mcc_menu_text_draw(long tag, short index, wchar_t const *text, long font, short justification,
    real_argb_color const *color, rectangle2d const *bounds, rectangle2d const *parent_clip)
{
    wchar_t wrapped[512], author[2+AUTHOR_TEXT_SIZE];
    rectangle2d area=*bounds, clip;
    int width=bounds->x1-bounds->x0-2, height=bounds->y1-bounds->y0, used, authored;
    long small;
    char const *name;
    if (!field(tag,index)) return FALSE;
    name=tag_get_name(tag);
    /* Xbox cards reserve their lower row for earned badges. Keep the native
     * text origin, but never let an authored description draw over that row. */
    if (!strncmp(name,"pc\\mcc_xbox\\",12) && strstr(name,"\\mcc_folder_help")) {
        area.y1=(short)MIN(area.y1,area.y0+83);
        height=area.y1-area.y0;
    }
    clip=area;
    if (width<=0 || height<=0) return TRUE;
    draw_string_set_draw_mode(font,NONE,0,0,color);
    used=wrap(text,wrapped,width);
    small=tag_loaded('font',"ui\\small_ui");
    if (text_height(wrapped)>height && small!=NONE && small!=font) {
        font=small; draw_string_set_draw_mode(font,NONE,0,0,color);
        used=wrap(text,wrapped,width);
    }
    /* A Workshop map's author keeps its two lines under the description,
     * which is cut short above them: by the height they add below a line. */
    authored=author_text(tag,index,width,author+2);
    if (authored) {
        author[0]='x'; author[1]='\r';
        height-=text_height(author)-text_height(L"x");
    }
    /* Arbitrarily long authored text cannot fit a fixed panel. Keep complete
     * visible lines and signal truncation, instead of cutting glyphs in half. */
    if (text_height(wrapped)>height) {
        while (used && text_height(wrapped)>height) wrapped[--used]=0;
        while (used && (wrapped[used-1]==' ' || wrapped[used-1]=='\r')) wrapped[--used]=0;
        while (used) {
            int start=used;
            while (start && wrapped[start-1]!='\r') start--;
            wrapped[used]='.'; wrapped[used+1]='.'; wrapped[used+2]='.'; wrapped[used+3]=0;
            if (width_of(wrapped+start)<=width && text_height(wrapped)<=height) break;
            wrapped[--used]=0;
        }
    }
    if (authored) {
        int at;
        /* (from the description's end: past its "..." when it was cut) */
        for (used=0; wrapped[used]; used++) {}
        while (used && (wrapped[used-1]==' ' || wrapped[used-1]=='\r')) wrapped[--used]=0;
        if (used) wrapped[used++]='\r';
        for (at=2; author[at] && used<(int)NUMBEROF(wrapped)-1; at++) wrapped[used++]=author[at];
        wrapped[used]=0;
    }
    if (parent_clip) {
        clip.x0=MAX(clip.x0,parent_clip->x0); clip.y0=MAX(clip.y0,parent_clip->y0);
        clip.x1=MIN(clip.x1,parent_clip->x1); clip.y1=MIN(clip.y1,parent_clip->y1);
    }
    draw_string_set_draw_mode(font,NONE,justification,0,color);
    if (clip.x1>clip.x0 && clip.y1>clip.y0) rasterizer_draw_unicode_string(&area,&clip,NULL,0,wrapped);
    return TRUE;
}

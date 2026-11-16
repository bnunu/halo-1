/* A Workshop map's author in the map lists' panel (mcc_menu_text.c's
mcc_menu_text_draw and author_text): "Publisher" and the name, two lines under
the map's description in mp_map_right_data (146 by 136 pixels: 6 lines of
small_ui, 20 pixels apart, 17 high), the description cut short with "..."
above them and a long name cut to one line. Drawn in a font of fixed widths:
each character 7 pixels, "W" 12. */
#include <wchar.h>
#include "harness.h"

#define NUMBEROF(array) (sizeof(array) / sizeof((array)[0]))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define SHORT_MAX 0x7FFF
#include "constants.inc"

typedef union { short v[4]; struct { short y0, x0, y1, x1; }; } rectangle2d;
typedef union { short n[2]; struct { short x, y; }; } point2d;
typedef struct { real alpha, red, green, blue; } real_argb_color;

static rectangle2d *set_rectangle2d(rectangle2d *rectangle, short x0, short y0, short x1, short y1)
{
 rectangle->x0=x0;rectangle->y0=y0;rectangle->x1=x1;rectangle->y1=y1;
 return rectangle;
}

/* the font: lines 20 pixels apart, each 17 high */
enum {LINE=20,LINE_HEIGHT=17,SMALL_FONT=5};
static int line_width(wchar_t const *text,int count)
{
 int width=0,index;
 for(index=0;index<count;index++)width+=text[index]=='W'?12:7;
 return width;
}
static void draw_string_set_draw_mode(long font,short style,short justification,unsigned long flags,real_argb_color const *color)
{
 (void)font;(void)style;(void)justification;(void)flags;(void)color;
}
static void draw_unicode_string_compute_bounds(rectangle2d const *bounds,wchar_t const *text,rectangle2d *ink,rectangle2d *cursor)
{
 int lines=1,widest=0,start=0,index;
 for(index=0;;index++)
 {
  if(text[index]=='\r'||!text[index])
  {
   int width=line_width(text+start,index-start);
   widest=MAX(widest,width);
   if(!text[index])
   {
    *ink=*bounds;
    ink->x1=(short)(bounds->x0+widest);
    ink->y1=(short)(bounds->y0+(lines-1)*LINE+LINE_HEIGHT);
    *cursor=*ink;
    cursor->x0=(short)(bounds->x0+width);
    return;
   }
   lines++;
   start=index+1;
  }
 }
}
static long tag_loaded(unsigned long group,char const *name){(void)group;(void)name;return SMALL_FONT;}

/* the widgets: the description, and the map's name above it */
enum {DATA_TAG=1,NAME_TAG=2};
static char const *tag_get_name(long tag)
{
 return tag==DATA_TAG?"pc\\main_menu\\multiplayer_type_select\\mp_map_select\\mp_map_right_data":
  "pc\\main_menu\\multiplayer_type_select\\mp_map_select\\mp_map_right_name";
}

/* the map's Workshop item and Steam's answer */
static char const *steam_author;
static char asked_item[32];
static char const *mcc_maps_item_id(short index){return index==3?"3629556661":"";}
static int workshop_authors_get(char const *item_id,char *name,int size)
{
 snprintf(asked_item,sizeof(asked_item),"%s",item_id);
 name[0]=0;
 if(!steam_author)return 0;
 snprintf(name,(size_t)size,"%s",steam_author);
 return 1;
}

/* what is drawn */
static wchar_t drawn[1024];
static void rasterizer_draw_unicode_string(rectangle2d const *bounds,rectangle2d const *clip,point2d *cursor,short height_adjust,wchar_t const *text)
{
 (void)bounds;(void)clip;(void)cursor;(void)height_adjust;
 wcsncpy(drawn,text,NUMBEROF(drawn)-1);
}

#include "under_test.inc"

static int lines_of(wchar_t const *text)
{
 int lines=1;
 for(;*text;text++)lines+=*text=='\r';
 return lines;
}

/* the text drawn for a map's description in the panel (or "" for none) */
static wchar_t const *draw(long tag,short index,wchar_t const *description,char const *author)
{
 static real_argb_color const white={1,1,1,1};
 rectangle2d bounds;
 set_rectangle2d(&bounds,417,258,417+146,258+136);
 steam_author=author;
 drawn[0]=0;
 mcc_menu_text_draw(tag,index,description,SMALL_FONT,0,&white,&bounds,NULL);
 return drawn;
}

/* the drawn text's last lines: "Publisher" and the name, the line before the description's */
static wchar_t const *author_lines(wchar_t const *text)
{
 wchar_t const *at=wcsstr(text,L"Publisher\r");
 CHECK(at,"no author drawn: \"%ls\"",text);
 CHECK(at==text||at[-1]=='\r',"the author is not on a line of its own: \"%ls\"",text);
 return at;
}

#define LONG_DESCRIPTION L"A remake of a classic map with new routes, a second base on the cliffs and many more weapons than before, " \
 L"for teams of eight on a side and anything from slayer to capture the flag."
#define MAP (MCC_MAPS_FIRST_DISPLAY_INDEX+3)

int main(int argc,char **argv)
{
 char const *name=argc>1?argv[1]:"";
 wchar_t const *text;

 if(!strcmp(name,"short"))
 {
  text=draw(DATA_TAG,MAP,L"A small map.","altis94");
  CHECK(!wcscmp(text,L"A small map.\rPublisher\raltis94"),"drew \"%ls\"",text);
  CHECK(!strcmp(asked_item,"3629556661"),"asked Steam about %s",asked_item);
 }
 else if(!strcmp(name,"long"))
 {
  /* the description cut to 4 lines with "...", then the author's 2: 6 lines in 136 pixels */
  wchar_t const *author;
  text=draw(DATA_TAG,MAP,LONG_DESCRIPTION,"altis94");
  author=author_lines(text);
  CHECK(!wcscmp(author,L"Publisher\raltis94"),"the author's lines are \"%ls\"",author);
  CHECK(lines_of(text)==6,"%d lines drawn: \"%ls\"",lines_of(text),text);
  CHECK(author-text>=4&&!wcsncmp(author-4,L"...\r",4),"the description is not cut with \"...\": \"%ls\"",text);
  {
   rectangle2d bounds,ink,cursor;
   set_rectangle2d(&bounds,0,0,SHORT_MAX,SHORT_MAX);
   draw_unicode_string_compute_bounds(&bounds,text,&ink,&cursor);
   CHECK(ink.y1<=136&&ink.x1<=144,"the text is %d by %d pixels, more than the box",ink.x1,ink.y1);
  }
 }
 else if(!strcmp(name,"no-author"))
 {
  /* before Steam has said (or for a map that is no Workshop item): the description as before, 6 lines */
  text=draw(DATA_TAG,MAP,LONG_DESCRIPTION,NULL);
  CHECK(!wcsstr(text,L"Publisher"),"an author drawn: \"%ls\"",text);
  CHECK(lines_of(text)==6,"%d lines drawn: \"%ls\"",lines_of(text),text);
 }
 else if(!strcmp(name,"other-field"))
 {
  text=draw(NAME_TAG,MAP,L"Birdcage","altis94");
  CHECK(!wcscmp(text,L"Birdcage"),"the map's name drawn as \"%ls\"",text);
 }
 else if(!strcmp(name,"long-name"))
 {
  /* 144 pixels: 20 characters, the name cut with "..." */
  wchar_t const *author;
  text=draw(DATA_TAG,MAP,L"A small map.","A Very Long Steam Name Indeed");
  author=author_lines(text);
  CHECK(!wcscmp(author,L"Publisher\rA Very Long Steam..."),"the author's lines are \"%ls\"",author);
  /* wide letters sooner */
  text=draw(DATA_TAG,MAP,L"A small map.","WWWWWWWWWWWWWWW");
  author=author_lines(text);
  CHECK(!wcscmp(author,L"Publisher\rWWWWWWWWWW..."),"the author's lines are \"%ls\"",author);
 }
 else if(!strcmp(name,"no-description"))
 {
  text=draw(DATA_TAG,MAP,L"","altis94");
  CHECK(!wcscmp(text,L"Publisher\raltis94"),"drew \"%ls\"",text);
 }
 else if(!strcmp(name,"characters"))
 {
  /* UTF-8 to the menus' UTF-16: a character beyond it a ? */
  text=draw(DATA_TAG,MAP,L"A small map.","J\xC3\xBCrgen \xF0\x9F\x98\x80");
  CHECK(!wcscmp(author_lines(text),L"Publisher\rJ\x00FCrgen ?"),"drew \"%ls\"",text);
 }
 else
 {
  fprintf(stderr,"unknown case %s\n",name);
  return 2;
 }
 return 0;
}

/* The lobby's author line (menu_functions.c's lobby_author_line and
lobby_line_fits): "Author: " and a Workshop map's author, one line of the
lobby's game data box, which holds 7 and a dedicated server's game already
fills. A name too wide for the box is cut short with "...", measured in the
box's font once it has been drawn, counted before. No line for a map that is
not an MCC one, nor before Steam has said. */
#include <stdarg.h>
#include <wchar.h>
#include "harness.h"
#include "constants.inc"

#define NUMBEROF(array) (sizeof(array) / sizeof((array)[0]))
#define MAX(a,b) ((a)>(b)?(a):(b))
enum {WORKSHOP_AUTHORS_NAME_SIZE=100,MCC_WORKSHOP_ID_SIZE=24};
typedef struct {short y0,x0,y1,x1;} rectangle2d;
typedef struct {real alpha,red,green,blue;} real_argb_color;
struct mcc_workshop_item {char id[MCC_WORKSHOP_ID_SIZE];char title[64];};

static size_t ustrlen(wchar_t const *text){return wcslen(text);}
static int usnprintf(wchar_t *text,size_t size,wchar_t const *format,...)
{
 /* (only "%s", as text_append asks) */
 wchar_t const *word;
 va_list arguments;
 CHECK(!wcscmp(format,L"%s"),"usnprintf asked for %ls",format);
 va_start(arguments,format);
 word=va_arg(arguments,wchar_t const *);
 va_end(arguments);
 wcsncpy(text,word,size);
 if(size)text[size-1]=0;
 return 0;
}

/* the box as drawn: a font of fixed widths (each character 7 pixels, "W" 12) */
static struct {long font;short width;} lobby_data_box={NONE,0};
static long drawing_font=NONE;
static void draw_string_set_draw_mode(long font,short style,short justification,unsigned long flags,real_argb_color const *color)
{
 (void)style;(void)justification;(void)flags;(void)color;
 drawing_font=font;
}
static void draw_unicode_string_compute_bounds(rectangle2d const *bounds,wchar_t const *text,rectangle2d *text_bounds,rectangle2d *cursor)
{
 short width=0;
 CHECK(drawing_font==lobby_data_box.font,"measured in font %ld, not the box's %ld",drawing_font,lobby_data_box.font);
 for(;*text;text++)width+=*text=='W'?12:7;
 *text_bounds=*bounds;
 text_bounds->x1=(short)(bounds->x0+width);
 *cursor=*bounds;
 cursor->x0=text_bounds->x1;
}

/* the maps: "mcc_maps\..." are MCC maps; the host's game names its item in the name's tail, else this machine's
copy's item is the catalog's */
static int wire_has_item;
static int mcc_level_name(char const *name){return !strncmp(name,"mcc_maps\\",9);}
static int mcc_workshop_wire_unpack(char const name[128],struct mcc_workshop_item *item)
{
 (void)name;
 memset(item,0,sizeof(*item));
 if(wire_has_item)strcpy(item->id,"2899894797");
 return wire_has_item;
}
static short mcc_maps_find(char const *name){return strstr(name,"unknown")?NONE:3;}
static char const *mcc_maps_item_id(short index){return index==3?"3629556661":"";}

/* Steam's answers */
static char const *steam_author;
static char asked_item[32];
static int workshop_authors_get(char const *item_id,char *name,int size)
{
 snprintf(asked_item,sizeof(asked_item),"%s",item_id);
 name[0]=0;
 if(!steam_author)return 0;
 snprintf(name,(size_t)size,"%s",steam_author);
 return 1;
}

#include "under_test.inc"

static void expect(char const *map,char const *author,wchar_t const *line)
{
 wchar_t text[128];
 boolean made;
 steam_author=author;
 memset(text,0x55,sizeof(text));
 made=lobby_author_line(map,text,NUMBEROF(text));
 if(!line)
 {
  CHECK(!made,"a line for %s: %ls",map,text);
  return;
 }
 CHECK(made,"no line for %s",map);
 CHECK(!wcscmp(text,line),"%s: \"%ls\", not \"%ls\"",map,text,line);
}

int main(int argc,char **argv)
{
 char const *name=argc>1?argv[1]:"";

 if(!strcmp(name,"line"))
 {
  lobby_data_box.font=7;lobby_data_box.width=146;
  expect("mcc_maps\\3629556661\\c10_flood","altis94",L"Author: altis94");
  CHECK(!strcmp(asked_item,"3629556661"),"asked Steam about %s",asked_item);
 }
 else if(!strcmp(name,"host-item"))
 {
  /* the host's game names its item: that one, not this machine's copy's */
  lobby_data_box.font=7;lobby_data_box.width=146;
  wire_has_item=1;
  expect("mcc_maps\\3629556661\\c10_flood","altis94",L"Author: altis94");
  CHECK(!strcmp(asked_item,"2899894797"),"asked Steam about %s",asked_item);
  /* a joiner without the map has its item from the host */
  expect("mcc_maps\\unknown\\map","altis94",L"Author: altis94");
 }
 else if(!strcmp(name,"none"))
 {
  lobby_data_box.font=7;lobby_data_box.width=146;
  expect("levels\\test\\bloodgulch\\bloodgulch","altis94",NULL);
  expect("mcc_maps\\unknown\\map","altis94",NULL);
  /* (before Steam has said) */
  expect("mcc_maps\\3629556661\\c10_flood",NULL,NULL);
 }
 else if(!strcmp(name,"cut"))
 {
  /* 146 - 8 = 138 pixels, 19 characters: "Author: " (8), 8 of the name and "..." */
  lobby_data_box.font=7;lobby_data_box.width=146;
  expect("mcc_maps\\3629556661\\c10_flood","A Very Long Steam Name",L"Author: A Very L...");
  /* wide letters are cut sooner */
  expect("mcc_maps\\3629556661\\c10_flood","WWWWWWWWWW",L"Author: WWWWW...");
  /* the most that fits (133 pixels): whole */
  expect("mcc_maps\\3629556661\\c10_flood","abcdefghijk",L"Author: abcdefghijk");
 }
 else if(!strcmp(name,"counted"))
 {
  /* before the box is drawn: counted, 18 characters at most */
  lobby_data_box.font=NONE;
  expect("mcc_maps\\3629556661\\c10_flood","altis94",L"Author: altis94");
  expect("mcc_maps\\3629556661\\c10_flood","A Very Long Steam Name",L"Author: A Very ...");
 }
 else
 {
  fprintf(stderr,"unknown case %s\n",name);
  return 2;
 }
 return 0;
}

/* The help under a folder of the MCC map lists (menu_functions.c's
mcc_folder_help_text): a Workshop item's folder's ends with "Author" and its
author's name, once Steam has said; a collection's folder holds the items of
many makers and has none, nor has a map's row. */
#include <stdarg.h>
#include <wchar.h>
#include "harness.h"

#define NUMBEROF(array) (sizeof(array) / sizeof((array)[0]))
#include "constants.inc"
#define MCC_MAP_BROWSER_MAXIMUM (MCC_MAPS_MAXIMUM * WORKSHOP_COLLECTION_MAXIMUM_PER_ITEM)
#include "types.inc"

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

/* the catalog: map 3 is in Workshop item 3629556661, map 5 in a package that is no Workshop item */
static char const *mcc_maps_item_id(short index){return index==3?"3629556661":"";}
static char const *steam_author;
static char asked_item[32];
static int workshop_authors_get(char const *item_id,char *name,int size)
{
 snprintf(asked_item,sizeof(asked_item),"%s",item_id);
 name[0]=0;
 if(!steam_author||strcmp(item_id,"3629556661"))return 0;
 snprintf(name,(size_t)size,"%s",steam_author);
 return 1;
}

#include "under_test.inc"

static struct mcc_map_browser browser;

/* the browser at the top: row 0 a collection's folder, row 1 an item's (map 3), row 2 a package's (map 5), row 3 a
loose map */
static void top(void)
{
 memset(&browser,0,sizeof(browser));
 browser.count=4;
 browser.folders[0]=TRUE;browser.groups[0]=1;browser.entries[0]=3;
 browser.folders[1]=TRUE;browser.groups[1]=NONE;browser.entries[1]=3;
 browser.folders[2]=TRUE;browser.groups[2]=NONE;browser.entries[2]=5;
 browser.folders[3]=FALSE;browser.groups[3]=NONE;browser.entries[3]=3;
}

static void expect(short row,char const *author,wchar_t const *help)
{
 wchar_t text[MCC_FOLDER_HELP_LENGTH];
 browser.chosen=row;
 steam_author=author;
 memset(text,0x55,sizeof(text));
 mcc_folder_help_text(&browser,text,NUMBEROF(text));
 CHECK(!wcscmp(text,help),"row %d: \"%ls\", not \"%ls\"",row,text,help);
}

int main(int argc,char **argv)
{
 char const *name=argc>1?argv[1]:"";

 top();
 if(!strcmp(name,"item"))
 {
  expect(1,"altis94",L"Open folder to\r\nchoose a map.\r\nAuthor\r\naltis94");
  CHECK(!strcmp(asked_item,"3629556661"),"asked Steam about %s",asked_item);
 }
 else if(!strcmp(name,"not-yet"))
  expect(1,NULL,L"Open folder to\r\nchoose a map.");
 else if(!strcmp(name,"collection"))
  expect(0,"altis94",L"Open folder to\r\nchoose a map.");
 else if(!strcmp(name,"package"))
  expect(2,"altis94",L"Open folder to\r\nchoose a map.");
 else if(!strcmp(name,"map-row"))
  expect(3,"altis94",L"Open folder to\r\nchoose a map.");
 else if(!strcmp(name,"empty"))
 {
  browser.count=0;
  expect(0,"altis94",L"No MCC maps found\r\nin this category.");
  /* (a row out of the list's, an item's folder left beyond it from before: nothing read there) */
  browser.count=4;
  browser.folders[4]=TRUE;browser.groups[4]=NONE;browser.entries[4]=3;
  expect(-1,"altis94",L"Open folder to\r\nchoose a map.");
  expect(4,"altis94",L"Open folder to\r\nchoose a map.");
 }
 else if(!strcmp(name,"fits"))
 {
  /* the longest name Steam gives fits the text box's length */
  static char longest[WORKSHOP_AUTHORS_NAME_SIZE];
  wchar_t text[MCC_FOLDER_HELP_LENGTH];
  memset(longest,'x',sizeof(longest)-1);
  browser.chosen=1;
  steam_author=longest;
  mcc_folder_help_text(&browser,text,NUMBEROF(text));
  CHECK(wcslen(text)==wcslen(L"Open folder to\r\nchoose a map.\r\nAuthor\r\n")+sizeof(longest)-1,
   "a name of %d characters made %d: cut short",(int)sizeof(longest)-1,(int)wcslen(text));
 }
 else
 {
  fprintf(stderr,"unknown case %s\n",name);
  return 2;
 }
 return 0;
}

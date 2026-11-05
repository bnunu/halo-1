#include "harness.h"
#include <strings.h>
#define MAXIMUM_STRINGS 32
#define NUMBEROF(a) ((short)(sizeof(a)/sizeof((a)[0])))
#define _stricmp strcasecmp
struct widget_instance {
 long definition_tag_index; short type;
 struct widget_instance *parent,*child,*next;
 union {struct {short selected_index,number_of_items;} list;} parameters;
};
struct player_profile {byte fields[8];};
#include "under_test_types.inc"
/* the profile's settings: none here */
static struct {char const *name;short field;long default_value;} profile_settings[1]={{"profile.none",0,0}};
static struct player_profile *player_ui_get_edit_player_profile(void){return NULL;}
static byte *profile_setting_field(struct player_profile *profile,short field){return &profile->fields[field];}
static boolean profile_setting_boolean(short field){(void)field;return FALSE;}
static boolean config_default(char const *name,char *text,unsigned int size){(void)name;snprintf(text,size,"x");return TRUE;}
static boolean config_text(char const *name,char *text,unsigned int size){(void)name;snprintf(text,size,"x");return TRUE;}
static boolean config_boolean(char const *name){(void)name;return FALSE;}
static int config_writes;
static boolean config_write(char const *name,char const *value){(void)name;(void)value;config_writes++;return TRUE;}
static void platform_log(char const *format,...){(void)format;}
/* player_link.c: the master a refusing server needs, and what OK does */
static int needed_master=-1,begun=-1,begin_count,unlink_count;
static int player_link_needed_master(void){return needed_master;}
static int player_link_begin(int master){begun=master;begin_count++;return 1;}
static void player_link_unlink(void){unlink_count++;}
static struct pc_menu_setting link_setting={7,"link.action",6,{"none","link_1","link_2","link_3","link_4","unlink"},NONE};
static struct pc_menu_setting *pc_menu_setting_get(long definition_index){return definition_index==7?&link_setting:NULL;}
#include "under_test.inc"
static struct widget_instance spinner;
/* the screen opened (the spinner's "port setting load"), the spinner moved
by `moves`, then OK ("port settings save") */
static void open_choose_ok(int needed,int moves)
{
 needed_master=needed;begun=-1;begin_count=unlink_count=config_writes=0;
 memset(&spinner,0,sizeof(spinner));spinner.definition_tag_index=7;spinner.type=2;
 spinner.parameters.list.number_of_items=6;
 CHECK(setting_load(&spinner),"load failed");
 spinner.parameters.list.selected_index=(short)((spinner.parameters.list.selected_index+moves+6)%6);
 CHECK(setting_changed_save(&spinner,&link_setting),"save failed");
 CHECK(config_writes==0,"link.action written to config.toml");
}
int main(int argc,char **argv)
{
 const char *case_name=argc>1?argv[1]:"";
 CASE("chosen_already") {
  /* (a server refused this machine for its VPN: master 1 is shown, and OK
  on it opens its page) */
  open_choose_ok(0,0);
  CHECK(spinner.parameters.list.selected_index==1,"the needed master not shown");
  CHECK(begin_count==1&&begun==0,"OK on the master chosen already did nothing");
 } else CASE("chosen") {
  open_choose_ok(-1,0);
  CHECK(spinner.parameters.list.selected_index==0,"NO CHANGE not shown");
  CHECK(begin_count==0&&unlink_count==0,"NO CHANGE did something");
  open_choose_ok(-1,2);
  CHECK(begin_count==1&&begun==1,"WITH MASTER 2 not begun");
  open_choose_ok(0,-1);
  CHECK(begin_count==0&&unlink_count==0,"NO CHANGE did something");
  open_choose_ok(2,2);
  CHECK(unlink_count==1&&begin_count==0,"UNLINK ALL not done");
 } else return 2;
 return 0;
}

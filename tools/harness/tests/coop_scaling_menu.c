#include "harness.h"
#define MIN(a,b) ((a)<(b)?(a):(b))
enum { _cooperative_enemies_per_player=0, _cooperative_enemies_multiplier=1, _loadout_custom=1 };
struct widget_instance {
 char const *name; short type,vertical_offset; boolean visible;
 struct widget_instance *child,*next,*focused_child;
 union {
  struct {short selected_index,number_of_items;struct widget_instance *extended_description;} list;
  struct {short string_list_index;} text_box;
 } parameters;
};
static struct widget_instance *named(struct widget_instance *w,char const *name,int unused)
{
 (void)unused;if(!strcmp(w->name,name))return w;
 for(struct widget_instance *c=w->child;c;c=c->next){struct widget_instance *r=named(c,name,0);if(r)return r;}
 return NULL;
}
static void visible_set(struct widget_instance *w,boolean visible){if(w)w->visible=visible;}
#include "under_test.inc"
#define ROWS 10
static char const *rows[]={"op_coop_friendly_fire","op_coop_extra_enemies","op_coop_extra_pickups","op_coop_extra_vehicles",
 "op_coop_enemies_multiplier","op_coop_enemies_per_player","op_coop_player_collisions","op_coop_ally_friendly_fire",
 "op_coop_team_wipe","op_coop_vehicle_riding"};
static char const *spinners[]={"coop_friendly_fire_spinner","coop_extra_enemies_spinner","coop_extra_pickups_spinner","coop_extra_vehicles_spinner",
 "coop_enemies_multiplier_spinner","coop_enemies_per_player_spinner","coop_player_collisions_spinner",
 "coop_ally_friendly_fire_spinner","coop_team_wipe_spinner","coop_vehicle_riding_spinner"};
static short counts[]={4,3,3,3,5,5,2,2,3,2};
static struct widget_instance list,help,row[ROWS],label[ROWS],spinner[ROWS],clone[ROWS];
static void setup(void)
{
 memset(&list,0,sizeof(list));list.name="coop_options_menu";list.vertical_offset=9;list.child=row;
 list.parameters.list.extended_description=&help;
 for(int i=0;i<ROWS;i++){
  short y=73+30*i;
  row[i]=(struct widget_instance){.name=rows[i],.visible=TRUE,.vertical_offset=y,.child=&label[i],.next=i<ROWS-1?&row[i+1]:NULL};
  label[i]=(struct widget_instance){.name="label",.vertical_offset=4,.next=&spinner[i]};
  spinner[i]=(struct widget_instance){.name=spinners[i],.type=2,.vertical_offset=1,.child=&clone[i]};
  spinner[i].parameters.list.number_of_items=counts[i];
  clone[i]=(struct widget_instance){.name="clone",.vertical_offset=2};
 }
}
int main(int argc,char **argv)
{
 const char *case_name=argc>1?argv[1]:"";setup();
 CASE("layout") {
  for(int repeat=0;repeat<100;repeat++)for(int mode=0;mode<3;mode++)for(int pickups=0;pickups<3;pickups++)for(int vehicles=0;vehicles<3;vehicles++){
   spinner[1].parameters.list.selected_index=mode;spinner[2].parameters.list.selected_index=pickups;
   spinner[3].parameters.list.selected_index=vehicles;
   list.focused_child=&row[1];gametype_option_help(&list);short y=73;
   CHECK(row[5].visible==(mode==0||pickups==0||vehicles==0),"per-player visibility");
   CHECK(row[4].visible==(mode==1||pickups==1||vehicles==1),"multiplier visibility");
    CHECK(list.focused_child==&row[1],"focus changed");
    int visible=0;for(int i=0;i<ROWS;i++)visible+=row[i].visible;
    short spacing=visible==10?26:visible==9?29:30;
    CHECK(row[9].visible,"riding row hidden");
    for(int i=0;i<ROWS;i++){
     if(row[i].visible){CHECK(row[i].vertical_offset==y,"gap or overlap at %s",rows[i]);
      CHECK(row[i].vertical_offset+28<=340,"rows overlap help area");y+=spacing;}
    CHECK(label[i].vertical_offset==4&&spinner[i].vertical_offset==1&&clone[i].vertical_offset==2,
     "relative descendant positions %s",rows[i]);
    CHECK(row[i].next==(i<ROWS-1?&row[i+1]:NULL),"row order changed");
   }
    CHECK(row[9].vertical_offset==y-spacing,"riding row not last");
  }
 } else CASE("help") {
  for(int mode=0;mode<3;mode++)for(int pickups=0;pickups<3;pickups++)for(int vehicles=0;vehicles<3;vehicles++){
   spinner[1].parameters.list.selected_index=mode;spinner[2].parameters.list.selected_index=pickups;
   spinner[3].parameters.list.selected_index=vehicles;
   gametype_option_help(&list);short offset=0;
   for(int i=0;i<ROWS;i++){
    if(row[i].visible)for(int choice=0;choice<counts[i];choice++){
     list.focused_child=&row[i];spinner[i].parameters.list.selected_index=choice;gametype_option_help(&list);
     CHECK(help.parameters.text_box.string_list_index==offset+choice,"wrong help row%d choice%d",i,choice);
     spinner[1].parameters.list.selected_index=mode;spinner[2].parameters.list.selected_index=pickups;
     spinner[3].parameters.list.selected_index=vehicles;
    }
    offset+=counts[i];
   }
  }
 } else CASE("unrelated") {
  spinner[1].name="unrelated";list.name="other_options";gametype_option_help(&list);
  for(int i=0;i<ROWS;i++)CHECK(row[i].visible&&row[i].vertical_offset==73+30*i,"unrelated menu altered");
 } else return 2;
 return 0;
}

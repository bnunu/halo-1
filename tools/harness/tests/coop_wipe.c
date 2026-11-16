#include "harness.h"
#define MAXIMUM_TRACKED_PLAYERS 4
#define HALO_PORT_MAXIMUM_NETWORK_MACHINES 3
#define MAXIMUM_LOCAL_PLAYERS 2
#define MIN(a,b) ((a)<(b)?(a):(b))
#define VALID_INDEX(index,count) ((index)>=0&&(index)<(count))
#define csmemset memset
#define _error_silent 0
static int diagnostics;
#define error(...) (++diagnostics)
enum {_game_connection_local,_game_connection_network_client,_game_connection_network_server};
struct player_datum {short identifier;boolean quit_out_of_game;long unit_index;};
struct data_iterator {long datum_index;int next;};
static struct data_array *player_data;
static long live_machine_players[3][2];
static long *machine_get_player_list(long machine){return live_machine_players[machine];}
static void data_iterator_new(struct data_iterator *i,struct data_array *array){(void)array;i->next=0;}
static void *data_iterator_next(struct data_iterator *i)
{
 while(i->next<player_data->maximum_count){
  int slot=i->next++;struct player_datum *p=(void *)(player_data->data+slot*player_data->size);
  if(p->identifier){i->datum_index=((long)(unsigned short)p->identifier<<16)|slot;return p;}
 }
 return NULL;
}
#include "types.inc"
static struct coop_checkpoint_players checkpoint_players;
static int connection,active;
static struct {int public,has_password,listed;} lobby;
static boolean browser_enabled=TRUE;
#define pthread_mutex_lock(lock) ((void)0)
#define pthread_mutex_unlock(lock) ((void)0)
/* (a dedicated server: its server.public and network.online, not the lobby's) */
static int dedicated,dedicated_public=TRUE,dedicated_online=TRUE;
static int halo_server_mode(void){return dedicated;}
static int config_boolean(char const *name)
{
 if(dedicated&&!strcmp(name,"server.public"))return dedicated_public;
 if(dedicated&&!strcmp(name,"network.online"))return dedicated_online;
 CHECK(!strcmp(name,"network.public_lobby"),"wrong visibility key");return browser_enabled;
}
static char const *policy="auto";
static short game_connection(void){return (short)connection;}
static boolean coop_game(void){return active;}
static char const *config_string(char const *name){CHECK(!strcmp(name,"network.coop_team_wipe"),"wrong policy key");return policy;}
static long tick,main_loss_last_tick;
static boolean paused,all_dead,save_succeeds;
static int reverts,respawns;
static struct {boolean lost_map,saving_map;short loss_timer;} main_globals;
static struct {boolean saved_game_valid;void *base_address;unsigned long cpu_allocation_size;} game_state_globals;
/* These legacy checkpoints contain no temporary console or MCC state. */
#define GAME_STATE_SIZE 1024
#define GAME_STATE_CPU_SIZE 512
static boolean hs_console_state_capture(void){return TRUE;}
static int mcc_cache_tags_loaded(void){return FALSE;}
static int mcc_checkpoint_capture(void *image,unsigned long bytes,unsigned long used,unsigned long capacity){return TRUE;}
static long game_time_get(void){return tick;}
static boolean game_time_get_paused(void){return paused;}
static boolean players_are_all_dead(void){return all_dead;}
static boolean main_coop_host_revert(void){if(!game_state_globals.saved_game_valid)return FALSE;reverts++;return TRUE;}
static void players_respawn_at_checkpoint(void){respawns++;}
static void game_state_revert(void){reverts++;}
static void game_state_call_before_save_procs(void){}
static void main_stop_time(void){}
static void main_start_time(void){}
static void cinematic_port_checkpoint(void){}
static boolean game_state_write_to_file(void){return save_succeeds;}
static void game_state_note_event(char const *note){(void)note;}
/* The unrelated state reset by the actual new-game lifecycle entry point. */
static int coop_presentation,coop_events,host_devices,client_devices,client_names_differing[1];
static int host_sent_transforms[1],host_sent_looks[1],host_sent_devices[1],host_player_structure_bsps[1],host_friendly_fire[1];
static int host_sent_screen_effect,host_resend,players_vitality,host_attachment_count,host_looping_sound_count;
static struct {int definition_indices[4];} client_script_sounds;
static struct {int count;} client_looping_sounds;
static struct {int objective,nav_point_count;} host_hud_state;
static struct {boolean offered,skip_save_written;int voters,cooldown_until;} skip_vote;
static void skip_vote_clear(void){}
static void coop_enemies_new_game(void){}
static void coop_pickups_new_game(void){}
static void coop_vehicles_new_game(void){}
static int sound_syncs;
static void coop_sound_sync_reset(void){sound_syncs=0;}
#include "under_test.inc"
static long add_player(void){long id=datum_new(player_data);int slot=DATUM_INDEX_TO_ABSOLUTE_INDEX(id);CHECK(id!=NONE,"player fixture full");((struct player_datum *)datum_get(player_data,id))->unit_index=100+slot;live_machine_players[slot<2?slot:2][slot<3?0:1]=id;return id;}
static struct player_datum *player(int slot){return (void *)(player_data->data+slot*player_data->size);}
static void setup(void)
{
 memset(live_machine_players,0xff,sizeof(live_machine_players));
 player_data=game_state_data_new("players",4,sizeof(struct player_datum));add_player();add_player();
 connection=_game_connection_network_server;active=TRUE;memset(&lobby,0,sizeof(lobby));browser_enabled=TRUE;policy="auto";
 tick=main_loss_last_tick=0;paused=FALSE;all_dead=TRUE;save_succeeds=TRUE;reverts=respawns=diagnostics=0;
 memset(&main_globals,0,sizeof(main_globals));game_state_globals.saved_game_valid=FALSE;
 network_coop_new_game();game_state_save();
}
static void wipe(void){main_lost_map();tick+=92;main_lost_map_private();}
int main(int argc,char **argv)
{
 char const *case_name=argc>1?argv[1]:"";setup();
 CASE("policy") {
  struct {char const *mode;int public_game;int want;} cases[]={
   {"auto",0,1},{"auto",1,0},{"revert",1,1},{"respawn",0,0},{"invalid",0,1},{"invalid",1,0}};
  for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++)for(int locked=0;locked<2;locked++)for(int transport=0;transport<2;transport++){
   setup();policy=cases[i].mode;lobby.public=cases[i].public_game;lobby.has_password=locked;lobby.listed=transport;wipe();
   CHECK(reverts==cases[i].want&&respawns==1-cases[i].want,"policy %s/public%d/password%d/transport%d",policy,lobby.public,locked,transport);
  }
  setup();lobby.public=TRUE;browser_enabled=FALSE;wipe();CHECK(reverts==1&&!respawns,"disabled public browser counted as listed");
  /* a dedicated server is public by server.public with internet play, whatever its lobby says */
  setup();dedicated=1;lobby.public=FALSE;wipe();CHECK(!reverts&&respawns==1,"public dedicated server reverted on auto");
  setup();dedicated=1;dedicated_online=FALSE;wipe();CHECK(reverts==1&&!respawns,"LAN-only dedicated server counted as public");
  dedicated=0;dedicated_online=TRUE;
 } else CASE("authority") {
  policy="revert";connection=_game_connection_network_client;CHECK(!network_coop_wipe_reverts(),"client decides wipe");
  connection=_game_connection_network_server;active=FALSE;CHECK(!network_coop_wipe_reverts(),"multiplayer decides co-op wipe");
  connection=_game_connection_local;wipe();CHECK(reverts==1&&!respawns,"local campaign behavior changed");
 } else CASE("roster") {
  for(int change=0;change<4;change++){
   setup();policy="revert";
   if(change==0)add_player();
   else if(change==1)player(1)->quit_out_of_game=TRUE;
   else if(change==2)player(1)->identifier++;
   else live_machine_players[1][0]=NONE; /* A leave before its quit timer expires. */
   wipe();CHECK(!reverts&&respawns==1&&diagnostics>0,"changed membership reverted");
  }
  setup();add_player();game_state_save();wipe();CHECK(reverts==1&&!respawns,"new matching checkpoint rejected");
 } else CASE("save") {
  add_player();save_succeeds=FALSE;game_state_save();
  CHECK(!network_coop_wipe_reverts(),"failed save replaced membership snapshot");
  setup();game_state_globals.saved_game_valid=FALSE;wipe();CHECK(!reverts&&respawns==1,"missing saved state rewound");
 } else CASE("new-map") {
  CHECK(network_coop_wipe_reverts(),"valid checkpoint rejected");
  host_attachment_count=3;host_looping_sound_count=2;client_looping_sounds.count=2;sound_syncs=2;
  network_coop_new_game();
  CHECK(!network_coop_wipe_reverts(),"checkpoint metadata survived a new game/map");
  CHECK(host_attachment_count==0&&host_looping_sound_count==0&&client_looping_sounds.count==0,"attachment or sound history survived a new game/map");
  CHECK(sound_syncs==0,"a host's time for a sound survived a new game/map");
 } else CASE("delay") {
  main_lost_map();tick=91;main_lost_map_private();CHECK(!reverts&&!respawns,"rewound early");
  tick=92;main_lost_map_private();CHECK(reverts==1&&!main_globals.lost_map&&!main_globals.loss_timer,"wipe did not clear loss state");
  wipe();CHECK(reverts==2&&!respawns,"second wipe failed");
  setup();main_lost_map();paused=TRUE;tick=92;main_lost_map_private();CHECK(!reverts,"pause advanced death timer");
  paused=FALSE;main_lost_map_private();tick=184;main_lost_map_private();CHECK(reverts==1,"resumed death timer stalled");
 } else CASE("failure") {
  policy="respawn";all_dead=FALSE;add_player();wipe();CHECK(reverts==1&&!respawns,"scripted failure used wipe policy/roster guard");
 } else return 2;
 return 0;
}

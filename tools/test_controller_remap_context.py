"""Real UI ownership and director input combined with physical remapping."""
from test_controller_remap import COMMON, POLICY, INPUT, ROOT, source
from test_hud_button_style import run_c
from harness import function


def integrated_source():
    return (COMMON + POLICY + INPUT.split('void controller_remap_profile(')[0] +
            (ROOT / 'tools/fixtures/controller_remap_context.c').read_text() +
            function((ROOT / 'source/interface/ui_widget.c').read_text(), 'ui_widgets_active_for_local_player') +
            source('port/linux/game/controller_remap_game.c') +
            source('port/linux/src/controller_remap_input.c') +
            function((ROOT / 'source/camera/director.c').read_text(), 'director_update_controls') + r'''
static void layout_save(void) {
 unsigned char layout[CONTROLLER_REMAP_ACTIONS];char text[CONTROLLER_REMAP_TEXT];
 controller_remap_profile(0,layout);controller_remap_assign(layout,PAD_MELEE,2);
 controller_remap_format(layout,text,sizeof(text));config_write("input.controller_p1",text);
}
static int frame(int button, unsigned int milliseconds) {
 XINPUT_GAMEPAD pad={0};unsigned char values[16];int i;struct camera_control controls;
 now+=milliseconds;
 if(button>=0) {if(button<8)pad.bAnalogButtons[button]=255;else pad.wButtons=digital_masks[button-8];}
 controller_remap_input(0,&pad,1);physical_values(&pad,values);
 for(i=0;i<16;i++)gamepad.buttons[i]=values[i]?(gamepad.buttons[i]?2:1):0;
 return director_update_controls(0,&controls);
}
''')


def test_main_menu_and_pause_keep_physical_buttons(tmp_path):
    run_c(tmp_path, integrated_source() + r'''
int main(void) {
 XINPUT_GAMEPAD pad={0},expected;
 layout_save();main_menu=1;
 assert(!ui_widgets_active_for_local_player(0)); /* The main menu belongs to any player. */
 assert(!controller_remap_playing(0));
 controller_remap_input(0,&pad,1);
 pad.wButtons=15;pad.bAnalogButtons[1]=255;expected=pad;
 controller_remap_input(0,&pad,1);assert(!memcmp(&pad,&expected,sizeof(pad)));
 /* Pause uses an explicit player. Other split-screen players can still play. */
 main_menu=0;root_widget.local_player_index=0;
 pad=expected;controller_remap_input(0,&pad,1);assert(!memcmp(&pad,&expected,sizeof(pad)));
 root_widget.local_player_index=1;assert(controller_remap_playing(0));
 /* Holding a menu button on return to gameplay must not trigger its binding. */
 widget_globals.active_widgets[0]=NULL;
 pad=expected;controller_remap_input(0,&pad,1);assert(!controller_remap_held(0));
 memset(&pad,0,sizeof(pad));controller_remap_input(0,&pad,1);
 pad.wButtons=4;controller_remap_input(0,&pad,1);assert(controller_remap_held(0)&(1u<<PAD_LEFT));
 main_menu=1;pad=expected;controller_remap_input(0,&pad,1);assert(!memcmp(&pad,&expected,sizeof(pad)));
 config_write("input.controller_p1","");pad=expected;
 controller_remap_input(0,&pad,1);assert(!memcmp(&pad,&expected,sizeof(pad)));
 return 0;
}
''')


def test_custom_camera_can_cycle_in_and_out(tmp_path):
    run_c(tmp_path, integrated_source() + r'''
int main(void) {
 layout_save();widget_globals.active_widgets[0]=NULL;director_camera_switch_fast=1;
 assert(!frame(-1,0));assert(frame(4,0));
 director_state.camera_proc=flying_camera_update;
 assert(!frame(4,1)); /* The entering hold cannot immediately cycle back out. */
 assert(!frame(-1,1));assert(frame(4,1));
 director_state.camera_proc=first_person_camera_update;
 assert(!frame(4,1));assert(!frame(-1,1));assert(frame(4,1));
 return 0;
}
''')


def test_camera_cycle_binding_timing_and_lifecycle(tmp_path):
    run_c(tmp_path, integrated_source() + r'''
int main(void) {
 unsigned char layout[CONTROLLER_REMAP_ACTIONS];char text[512];int i;
 director_camera_update_proc cameras[3]={flying_camera_update,orbiting_camera_update,editor_camera_update};
 layout_save();widget_globals.active_widgets[0]=NULL;
 assert(!frame(-1,0));assert(!frame(4,0));assert(!frame(4,999));assert(frame(4,1));
 assert(!frame(4,0));assert(frame(4,1000));assert(!frame(-1,0));
 /* A physical binding is independent of the profile's translated RB action. */
 controller_remap_read(0,layout);controller_remap_assign(layout,PAD_CAMERA_CYCLE,13);
 controller_remap_format(layout,text,sizeof(text));config_write("input.controller_p1",text);
 assert(!frame(-1,0));assert(!frame(4,0));assert(!frame(4,1000));
 assert(!frame(-1,0));assert(!frame(13,0));assert(frame(13,1000));
 for(i=0;i<3;i++) {
  director_state.camera_proc=cameras[i];director_state.debug_controls=0;
  assert(!frame(13,1000));assert(!frame(-1,0));assert(!frame(13,0));assert(frame(13,1000));
  assert(!frame(-1,0));assert(!frame(15,0));assert(director_state.debug_controls);
  /* The director publishes its new control context on the next frame. */
  assert(!frame(-1,0));assert(!frame(-1,0));assert(!frame(13,0));assert(frame(13,1000));
 }
 /* Menus, chat, console and lost focus cancel rather than defer a cycle. */
 main_menu=1;assert(!frame(13,1000));main_menu=0;assert(!frame(13,1000));
 assert(!frame(-1,0));assert(!frame(13,0));assert(frame(13,1000));
 chat_owner=0;assert(!frame(13,1000));chat_owner=NONE;assert(!frame(13,1000));
 assert(!frame(-1,0));console_open=1;assert(!frame(13,1000));console_open=0;assert(!frame(13,1000));
 assert(!frame(-1,0));focused=0;assert(!frame(13,1000));focused=1;assert(!frame(13,1000));
 assert(!frame(-1,0));assert(!frame(13,0));instance[0]++;assert(!frame(13,1000));
 assert(!frame(-1,0));assert(!frame(13,0));controller_remap_reset();assert(!frame(13,1000));
 assert(!frame(-1,0));assert(!frame(13,0));assert(frame(13,1000));
 /* Unbinding disables both the custom cycle and the old incidental RB shortcut. */
 controller_remap_assign(layout,PAD_CAMERA_CYCLE,255);
 controller_remap_format(layout,text,sizeof(text));config_write("input.controller_p1",text);
 assert(!frame(-1,0));assert(!frame(13,0));assert(!frame(13,1000));
 assert(!frame(-1,0));assert(!frame(4,0));assert(!frame(4,1000));
 /* Empty config keeps the original fast and held-button camera paths. */
 config_write("input.controller_p1","");director_camera_switch_fast=1;
 assert(!frame(-1,0));assert(frame(4,0));assert(!frame(4,0));
 director_camera_switch_fast=0;gamepad.buttons[4]=30;
 {struct camera_control controls;assert(director_update_controls(0,&controls));assert(!director_update_controls(0,&controls));}
 return 0;
}
''')


def test_cycle_still_obeys_director_client_permission(tmp_path):
    director = (ROOT / 'source/camera/director.c').read_text()
    run_c(tmp_path, integrated_source() + r'''
static int client, rotations, scripted;
static int *director_camera_scripted=&scripted;
static short const director_game_camera_modes[3]={0,1,2};
struct first_person_camera {int unused;};
struct dead_camera {long unit_index;};
static void dead_camera_update(void){}
static void first_person_camera_new(struct first_person_camera *p){}
static void dead_camera_new(struct dead_camera *p,short local,long watched){}
static void director_set_camera(short local,director_camera_update_proc proc,int interpolate){director_state.camera_proc=proc;}
static void director_choose_game_perspective(short local,int force){}
static void director_rotate_cameras(short local,short const *modes,short count){rotations++;}
static int network_game_distributed_client(void){return client;}
static int players_coop_waiting_to_start(long player){return 0;}
static int coop_spectating(void){return 0;}
static long coop_spectate_unit(short local){return NONE;}
static void coop_spectate_camera(struct dead_camera *p){}
''' + function(director, 'director_choose_camera_game') + r'''
int main(void) {
 int cycle;
 layout_save();widget_globals.active_widgets[0]=NULL;director_camera_switch_fast=1;
 frame(-1,0);cycle=frame(4,0);assert(cycle);
 client=1;director_choose_camera_game(0,0,cycle);assert(rotations==0);
 client=0;director_choose_camera_game(0,0,cycle);assert(rotations==1);
 return 0;
}
''')

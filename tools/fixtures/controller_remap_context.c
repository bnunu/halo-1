/* Fake UI/player/director state for production controller integration tests. */

#define NONE -1
#define TRUE 1
#define MAXIMUM_NUMBER_OF_LOCAL_PLAYERS 4
#define csmemcpy memcpy
#define csmemset memset
#define match_vassert(file,line,condition,message) assert(condition)
typedef int boolean;
typedef unsigned char byte;
typedef float real;
typedef struct {real i,j;} real_vector2d;
typedef struct {real yaw,pitch;} real_euler_angles2d;
static real_vector2d player_control_port_debug_throttle[4];
static real_euler_angles2d player_control_port_debug_facing[4];
static void halo_linux_debug_camera_input(int *middle,int *up,int *down,float *wheel){
 *middle=*up=*down=0;*wheel=0;
}
struct widget_instance {short local_player_index;};
static struct widget_instance root_widget = {-1};
static struct {int initialized; struct widget_instance *active_widgets[4];} widget_globals = {1,{&root_widget,0,0,0}};
struct game_input_preferences {unsigned char game_control_to_xbox_buttons[12];};
static struct player_datum {short local_player_index;long unit_index;struct {int deaths;} statistics;} fake_player = {0};
static int main_menu, console_open, chat_owner = NONE;
static int main_menu_is_active(void){return main_menu;}
static int console_is_active(void){return console_open;}
static int game_time_initialized(void){return 1;}
static int game_in_progress(void){return 1;}
static short local_player_get_next(short p){return p<0?0:NONE;}
static long local_player_get_player_index(short p){return p==0?0:NONE;}
#define player_get(p) (&fake_player)
short network_chat_controller(void){return (short)chat_owner;}
static void input_abstraction_get_local_player_preferences(short p,struct game_input_preferences *out){
 unsigned char b[CONTROLLER_REMAP_ACTIONS]; int i; memset(out,0,sizeof(*out)); controller_remap_defaults(b);
 for(i=0;i<10;i++)out->game_control_to_xbox_buttons[controller_remap_controls[i]]=b[i];
}
typedef void (*director_camera_update_proc)(void);
static void first_person_camera_update(void){}
static void following_camera_update(void){}
static void flying_camera_update(void){}
static void editor_camera_update(void){}
static void orbiting_camera_update(void){}
static struct director {director_camera_update_proc camera_proc; int debug_controls; float debug_input_scale;
 struct {float delta;} debug_variables[4];char camera_data[32];} director_state = {first_person_camera_update,0,1,{{0}}};
static struct {float dtime;} director_globals = {1.0f/30};
static struct director *director_get(short p){return &director_state;}
static int director_camera_switch_fast;
#define TICKS_PER_SECOND 30
enum {_gamepad_analog_button_black=4,_gamepad_analog_button_right_trigger=7,_gamepad_analog_button_left_trigger=6,
 _gamepad_binary_button_right_thumb=15,_gamepad_binary_button_dpad_up=8,_gamepad_binary_button_dpad_down=9};
enum {_gamepad_stick_left,_gamepad_stick_right};
enum {_variable_height,_variable_roll,_variable_forward,_variable_right};
enum {_camera_control_forward_bit,_camera_control_reverse_bit,_camera_control_left_bit,_camera_control_right_bit,
 _camera_control_up_bit,_camera_control_down_bit,_camera_control_roll_left_bit,_camera_control_roll_right_bit};
enum {_key_backspace,_key_tab,_key_w,_key_s,_key_a,_key_d,_key_r,_key_f,_key_t,_key_g};
struct camera_control {short local_player_index;float seconds_elapsed,wheel_delta;int active;
 struct {float yaw,pitch,roll;} facing_delta; struct {float i,j,k;} position_delta;};
static struct gamepad_state {byte buttons[16];struct {short x,y;} sticks[2];} gamepad;
struct mouse_state {int x,y,wheel_delta,buttons[3];};
static int input_has_gamepad(short p){return 1;}
static struct gamepad_state const *input_get_gamepad_state(short p){return &gamepad;}
static struct mouse_state const *input_get_mouse_state(void){return NULL;}
static int input_key_is_down(int key){return 0;}
static int passenger_camera_enabled(short p){return 0;}
#define SET_FLAG(v,b,on) ((v) = ((v)&~(1u<<(b))) | ((on)?(1u<<(b)):0))
#define director_process_variables(p,f,w) ((void)0)
#define director_inhibit_input(p) ((void)0)
#define director_inhibit_facing(p) ((void)0)

#ifndef CONTROLLER_REMAP_H
#define CONTROLLER_REMAP_H

#define CONTROLLER_REMAP_GAME_ACTIONS 10
#define CONTROLLER_REMAP_BUTTONS 15
#define CONTROLLER_REMAP_TEXT 512
#define CONTROLLER_REMAP_UNBOUND 255
enum controller_remap_action
{
    PAD_JUMP, PAD_GRENADE_TYPE, PAD_USE, PAD_WEAPON, PAD_MELEE,
    PAD_FLASHLIGHT, PAD_GRENADE, PAD_FIRE, PAD_CROUCH, PAD_ZOOM,
    PAD_SCOREBOARD, PAD_SCREENSHOT, PAD_FORWARD, PAD_BACKWARD, PAD_LEFT, PAD_RIGHT,
    PAD_CHAT, PAD_TEAM_CHAT, PAD_VEHICLE_CHAT, PAD_TALK,
    PAD_CAMERA_FASTER, PAD_CAMERA_SLOWER, PAD_CAMERA_SWITCH, PAD_CAMERA_UP, PAD_CAMERA_DOWN,
    PAD_CAMERA_CYCLE,
    CONTROLLER_REMAP_ACTIONS
};

/* Game control indices: jump, grenade type, use/reload, weapon, melee,
 * flashlight, grenade, fire, crouch, zoom. Start and stick axes stay stock. */
extern unsigned char const controller_remap_controls[CONTROLLER_REMAP_GAME_ACTIONS];
char const *controller_remap_setting(short port);
int controller_remap_parse(char const *text, unsigned char *buttons);
void controller_remap_format(unsigned char const *buttons, char *text, unsigned int size);
int controller_remap_read(short port, unsigned char *buttons);
void controller_remap_defaults(unsigned char *buttons);
void controller_remap_complete(unsigned char *buttons);
int controller_remap_conflict(unsigned char const *buttons, int action, int button);
void controller_remap_assign(unsigned char *buttons, int action, int button);
char const *controller_remap_button_name(int button, int family);
int controller_remap_cycle(int button, int direction);

/* Game-side access to the existing profile and menu context. No profile writes. */
void controller_remap_profile(short port, unsigned char *buttons);
int controller_remap_playing(short port);
void controller_remap_reset(void);
void controller_remap_input_reset(void);
short controller_remap_local_player(short port);
int controller_remap_context(short port);
int controller_remap_camera(short local_player, int available, int controlling,
    unsigned int *held, unsigned int *pressed);
unsigned int controller_remap_held(short port);
unsigned int controller_remap_pressed(short port, unsigned int mask);
int controller_remap_connected(short port);
int controller_remap_camera_cycle(short port, int fast);
int controller_remap_talk(void);
void controller_remap_movement(short port, float *forward, float *strafe);
struct game_input_preferences;
void controller_remap_prompt(short port, struct game_input_preferences *preferences);

/* Raw physical input only, before keyboard/touch merging. Returns true while
 * capture consumes this port. The caller also suppresses its merged menu input. */
struct _XINPUT_GAMEPAD;
int controller_remap_input(short port, struct _XINPUT_GAMEPAD *pad, int connected);
void controller_remap_merge(struct _XINPUT_GAMEPAD const *source, struct _XINPUT_GAMEPAD *target);
void controller_remap_capture_begin(short port);
int controller_remap_capture_poll(int *button);
void controller_remap_capture_end(void);

#endif

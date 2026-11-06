/* Existing profile mappings remain authoritative for stock and menu input.
 * Custom layouts translate physical buttons before keyboard/touch merging. */
#include "cseries.h"
#include "memory/data.h"
#include "controller_remap.h"
#include "input/input_abstraction.h"
#include "game/game.h"
#include "game/players.h"
#include "interface/ui_widget.h"
#include "input/input.h"
#include "main/console.h"
short network_chat_controller(void);
static unsigned char camera_context[4];

/* input_initialize reads the pads before game_time_initialize has made the
 * game time globals, which game_in_progress reads without checking. */
static int remap_game_in_progress(void)
{
    return game_time_initialized() && game_in_progress();
}

void controller_remap_reset(void)
{
    csmemset(camera_context, 0, sizeof(camera_context));
    controller_remap_input_reset();
}

void controller_remap_profile(short port, unsigned char *buttons)
{
    struct game_input_preferences preferences;
    static unsigned char const standard[CONTROLLER_REMAP_GAME_ACTIONS] = {0,4,2,3,1,5,6,7,14,15};
    unsigned int seen = 0;
    int i;
    controller_remap_defaults(buttons);
    if (port != 0) { buttons[PAD_CHAT] = CONTROLLER_REMAP_UNBOUND; buttons[PAD_FORWARD] = 8; }
    input_abstraction_get_local_player_preferences(port, &preferences);
    for (i = 0; i < CONTROLLER_REMAP_GAME_ACTIONS; i++)
    {
        int button = preferences.game_control_to_xbox_buttons[controller_remap_controls[i]];
        if ((button > 7 && button != 10 && button != 14 && button != 15) || (seen & (1u << button))) break;
        seen |= 1u << button;
        buttons[i] = (unsigned char)button;
    }
    /* Profiles can be queried before input initialization from the main menu. */
    if (i < CONTROLLER_REMAP_GAME_ACTIONS) csmemcpy(buttons, standard, sizeof(standard));
    controller_remap_complete(buttons);
}

short controller_remap_local_player(short port)
{
    short local;
    if (port < 0 || port >= 4 || !remap_game_in_progress()) return NONE;
    for (local = local_player_get_next(NONE); local != NONE; local = local_player_get_next(local))
    {
        long player = local_player_get_player_index(local);
        if (player != NONE && player_get(player)->local_player_index == port) return local;
    }
    return NONE;
}

int controller_remap_playing(short port)
{
    short local = controller_remap_local_player(port);
    /* The main menu has a live UI-map player, but its widgets belong to NONE. */
    return remap_game_in_progress() && !main_menu_is_active() && local != NONE && !ui_widgets_active_for_local_player(local) &&
        !console_is_active() && network_chat_controller() != port;
}

int controller_remap_context(short port)
{
    if (!controller_remap_playing(port))
    {
        if (!remap_game_in_progress() && port >= 0 && port < 4) camera_context[port] = 0;
        return 0;
    }
    return 1 + camera_context[port];
}

/* The director supplies the free-camera context and retains camera selection
 * and permissions. Cycle input is available in the normal player context too. */
int controller_remap_camera(short local, int available, int controlling,
    unsigned int *held, unsigned int *pressed)
{
    long player = local_player_get_player_index(local);
    short port;
    unsigned int mask = ((1u << PAD_CAMERA_CYCLE) - 1) & ~((1u << PAD_CAMERA_FASTER) - 1);
    *held = *pressed = 0;
    if (player == NONE) return 0;
    port = player_get(player)->local_player_index;
    if (port < 0 || port >= 4) return 0;
    camera_context[port] = available ? (controlling ? 2 : 1) : 0;
    {
        unsigned char layout[CONTROLLER_REMAP_ACTIONS];
        if (!controller_remap_read(port, layout)) { camera_context[port] = 0; return 0; }
    }
    /* Custom cycling also owns the shortcut before a debug camera is active. */
    if (!controller_remap_connected(port)) return 0;
    *held = controller_remap_held(port) & mask;
    *pressed = controller_remap_pressed(port, mask);
    return 1;
}

void controller_remap_movement(short port, float *forward, float *strafe)
{
    unsigned int held = controller_remap_held(port);
    unsigned char layout[CONTROLLER_REMAP_ACTIONS];
    int x, y;
    float length;
    if (!controller_remap_read(port, layout)) return;
    x = !!(held & (1u << PAD_RIGHT)) - !!(held & (1u << PAD_LEFT));
    y = !!(held & (1u << PAD_FORWARD)) - !!(held & (1u << PAD_BACKWARD));
    if (!x && !y) return;
    length = x && y ? 0.70710678f : 1.f;
    if (held & (1u << PAD_CROUCH)) length *= 0.97f;
    *forward = y * length;
    *strafe = -x * length;
}

void controller_remap_prompt(short port, struct game_input_preferences *preferences)
{
    unsigned char buttons[CONTROLLER_REMAP_ACTIONS];
    long player;
    int i;
    if (port < 0 || port >= 4) return;
    player = local_player_get_player_index(port);
    if (player != NONE) port = player_get(player)->local_player_index;
    if (!controller_remap_read(port, buttons)) return;
    for (i = 0; i < CONTROLLER_REMAP_GAME_ACTIONS; i++)
        preferences->game_control_to_xbox_buttons[controller_remap_controls[i]] = buttons[i];
}

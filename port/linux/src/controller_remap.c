/* Controller-owned configuration. Empty/invalid layouts retain the profile.
 * Stored names describe physical positions; artwork preferences never affect input. */
#include "controller_remap.h"
#include "controller_buttons.h"
#include "port_config.h"
#include <stdio.h>
#include <string.h>

unsigned char const controller_remap_controls[CONTROLLER_REMAP_GAME_ACTIONS] = {0,1,2,3,4,5,6,7,10,11};
static unsigned char const remap_buttons[CONTROLLER_REMAP_BUTTONS] = {0,1,2,3,4,5,6,7,14,15,8,9,10,11,13};
static char const *const remap_names[] = {"south","east","west","north","rb","lb","lt","rt","ls","rs",
    "dpad_up","dpad_down","dpad_left","dpad_right","back"};

void controller_remap_defaults(unsigned char *buttons)
{
    static unsigned char const defaults[CONTROLLER_REMAP_ACTIONS] = {
        0,4,2,3,1,5,6,7,14,15, 13,255,255,9,10,11, 8,255,255,255, 8,9,15,7,6,4
    };
    memcpy(buttons, defaults, sizeof(defaults));
}

/* Presets and older saves own the first ten actions. Their buttons take
 * priority over newly exposed defaults, notably D-pad Left in Recon. */
void controller_remap_complete(unsigned char *buttons)
{
    int i, j;
    for (i = CONTROLLER_REMAP_GAME_ACTIONS; i < PAD_CAMERA_FASTER; i++)
        if (buttons[i] != CONTROLLER_REMAP_UNBOUND)
            for (j = 0; j < i; j++)
                if (buttons[i] == buttons[j]) { buttons[i] = CONTROLLER_REMAP_UNBOUND; break; }
}

int controller_remap_conflict(unsigned char const *buttons, int action, int button)
{
    int i, first, end;
    if (action < 0 || action >= CONTROLLER_REMAP_ACTIONS || button == CONTROLLER_REMAP_UNBOUND) return -1;
    first = action >= PAD_CAMERA_FASTER ? PAD_CAMERA_FASTER : 0;
    end = action >= PAD_CAMERA_FASTER ? CONTROLLER_REMAP_ACTIONS : PAD_CAMERA_FASTER;
    for (i = first; i < end; i++) if (i != action && buttons[i] == button) return i;
    return -1;
}

static int valid_button(int button)
{
    int i;
    if (button == CONTROLLER_REMAP_UNBOUND) return 1;
    for (i = 0; i < CONTROLLER_REMAP_BUTTONS; i++) if (remap_buttons[i] == button) return 1;
    return 0;
}

char const *controller_remap_setting(short port)
{
    static char const *const settings[] = {"input.controller_p1", "input.controller_p2",
        "input.controller_p3", "input.controller_p4"};
    return port >= 0 && port < 4 ? settings[port] : NULL;
}

int controller_remap_parse(char const *text, unsigned char *buttons)
{
    unsigned char parsed[CONTROLLER_REMAP_ACTIONS];
    int action, i, count;
    if (!text || !*text) return 0;
    count = !strncmp(text, "v3:", 3) ? CONTROLLER_REMAP_ACTIONS :
        !strncmp(text, "v2:", 3) ? 25 : CONTROLLER_REMAP_GAME_ACTIONS;
    if (count != CONTROLLER_REMAP_GAME_ACTIONS) text += 3;
    controller_remap_defaults(parsed);
    for (action = 0; action < count; action++)
    {
        size_t length = strcspn(text, ",");
        int button = CONTROLLER_REMAP_UNBOUND;
        if (length != 7 || strncmp(text, "unbound", 7))
        {
            for (i = 0; i < CONTROLLER_REMAP_BUTTONS; i++)
                if (strlen(remap_names[i]) == length && !strncmp(text, remap_names[i], length)) break;
            if (i == CONTROLLER_REMAP_BUTTONS) return 0;
            button = remap_buttons[i];
        }
        for (i = action >= PAD_CAMERA_FASTER ? PAD_CAMERA_FASTER : 0; i < action; i++)
            if (button != CONTROLLER_REMAP_UNBOUND && parsed[i] == button) return 0;
        parsed[action] = (unsigned char)button;
        text += length;
        if (action < count - 1) { if (*text++ != ',') return 0; }
        else if (*text) return 0;
    }
    if (count == CONTROLLER_REMAP_GAME_ACTIONS) controller_remap_complete(parsed);
    /* Older debug layouts keep their buttons if RB is already assigned. */
    if (count < CONTROLLER_REMAP_ACTIONS &&
        controller_remap_conflict(parsed, PAD_CAMERA_CYCLE, parsed[PAD_CAMERA_CYCLE]) >= 0)
        parsed[PAD_CAMERA_CYCLE] = CONTROLLER_REMAP_UNBOUND;
    memcpy(buttons, parsed, sizeof(parsed));
    return 1;
}

void controller_remap_format(unsigned char const *buttons, char *text, unsigned int size)
{
    unsigned int used = 3;
    int action, i;
    if (!size) return;
    *text = 0;
    if (size < 4) return;
    strcpy(text, "v3:");
    for (action = 0; action < CONTROLLER_REMAP_ACTIONS; action++)
    {
        char const *name = "unbound";
        if (!valid_button(buttons[action]) || controller_remap_conflict(buttons, action, buttons[action]) >= 0)
        { *text = 0; return; }
        for (i = 0; i < CONTROLLER_REMAP_BUTTONS; i++)
            if (remap_buttons[i] == buttons[action]) { name = remap_names[i]; break; }
        if (used + strlen(name) + 2 > size) { *text = 0; return; }
        used += (unsigned int)snprintf(text + used, size - used, "%s%s", action ? "," : "", name);
    }
}

int controller_remap_read(short port, unsigned char *buttons)
{
    static unsigned long read_at = (unsigned long)-1;
    static unsigned char layouts[4][CONTROLLER_REMAP_ACTIONS];
    static int valid[4];
    int i;
    if (port < 0 || port >= 4) return 0;
    if (read_at != config_changes())
    {
        for (i = 0; i < 4; i++)
        {
            char const *text = config_string(controller_remap_setting((short)i));
            valid[i] = controller_remap_parse(text, layouts[i]);
            if (valid[i] && i && strncmp(text, "v2:", 3) && strncmp(text, "v3:", 3))
            {
                layouts[i][PAD_CHAT] = CONTROLLER_REMAP_UNBOUND;
                layouts[i][PAD_FORWARD] = 8;
                controller_remap_complete(layouts[i]);
            }
        }
        read_at = config_changes();
    }
    if (valid[port]) memcpy(buttons, layouts[port], CONTROLLER_REMAP_ACTIONS);
    return valid[port];
}

void controller_remap_assign(unsigned char *buttons, int action, int button)
{
    int conflict;
    if (action < 0 || action >= CONTROLLER_REMAP_ACTIONS || !valid_button(button)) return;
    conflict = controller_remap_conflict(buttons, action, button);
    if (conflict >= 0) buttons[conflict] = CONTROLLER_REMAP_UNBOUND;
    buttons[action] = (unsigned char)button;
}

int controller_remap_cycle(int button, int direction)
{
    int i;
    for (i = 0; i < CONTROLLER_REMAP_BUTTONS && remap_buttons[i] != button; i++);
    i = (i + (direction < 0 ? CONTROLLER_REMAP_BUTTONS : 1)) % (CONTROLLER_REMAP_BUTTONS + 1);
    return i == CONTROLLER_REMAP_BUTTONS ? CONTROLLER_REMAP_UNBOUND : remap_buttons[i];
}

char const *controller_remap_button_name(int button, int family)
{
    static char const *const labels[4][CONTROLLER_REMAP_BUTTONS] = {
        {"A","B","X","Y","RB / BLACK","LB / WHITE","LT","RT","LEFT STICK","RIGHT STICK","D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT","BACK / SELECT"},
        {"A","B","X","Y","RB / BLACK","LB / WHITE","LT","RT","LEFT STICK","RIGHT STICK","D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT","BACK / SELECT"},
        {"CROSS","CIRCLE","SQUARE","TRIANGLE","R1","L1","L2","R2","L3","R3","D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT","TOUCHPAD / SELECT"},
        {"B","A","Y","X","R","L","ZL","ZR","LEFT STICK","RIGHT STICK","D-PAD UP","D-PAD DOWN","D-PAD LEFT","D-PAD RIGHT","BACK / SELECT"}
    };
    int i;
    if (family < 0 || family > 3) family = 0;
    for (i = 0; i < CONTROLLER_REMAP_BUTTONS; i++)
        if (remap_buttons[i] == button) return labels[family][i];
    return "UNBOUND";
}

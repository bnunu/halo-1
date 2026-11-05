/* Physical controller remapping and capture, on the game's input thread. */
#include "platform.h"
#include "sdl_platform.h"
#include "controller_remap.h"
#include "controller_buttons.h"
#include <SDL3/SDL.h>
#include <string.h>
#include <stdlib.h>

static WORD const digital_masks[8] = {XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
    XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_START,
    XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB};

static struct
{
    short port;
    int phase, result, button;
    unsigned int instance;
    Uint64 polled;
} capture = {-1, 0, 0, 0, 0, 0};

static struct
{
    int custom, context, connected;
    unsigned int instance, blocked, held, pressed;
    Uint64 cycle_since, cycle_count;
    unsigned char layout[CONTROLLER_REMAP_ACTIONS], targets[CONTROLLER_REMAP_ACTIONS];
} remap_ports[4];

void controller_remap_input_reset(void)
{
    controller_remap_capture_end();
    memset(remap_ports, 0, sizeof(remap_ports));
}

static void physical_values(XINPUT_GAMEPAD const *pad, unsigned char *values)
{
    int i;
    memcpy(values, pad->bAnalogButtons, 8);
    for (i = 0; i < 8; i++) values[8+i] = pad->wButtons & digital_masks[i] ? 255 : 0;
}

static int capture_device_present(void)
{
    struct controller_device device;
    controller_device_get(capture.port, &device);
    return device.name[0] && device.instance == capture.instance;
}

void controller_remap_capture_begin(short port)
{
    struct controller_device device;
    controller_remap_capture_end();
    if (port < 0 || port >= 4) return;
    controller_device_get(port, &device);
    capture.port = port;
    capture.instance = device.instance;
    capture.phase = 1; /* Opening press must be released first. */
    capture.polled = SDL_GetTicks();
    platform_binding_capture_begin();
}

void controller_remap_capture_end(void)
{
    if (!capture.phase) return;
    /* A disconnected port may no longer receive XInputGetState calls. */
    capture.phase = capture_device_present() ? 4 : 0;
    if (!capture.phase) capture.port = -1;
    capture.result = 0;
    platform_binding_capture_cancel();
}

int controller_remap_capture_poll(int *button)
{
    int key, result;
    if (!capture.phase || capture.phase == 4) return 3;
    if (!capture_device_present())
    {
        controller_remap_capture_end();
        return 3;
    }
    capture.polled = SDL_GetTicks();
    result = platform_binding_capture_poll(&key);
    if (result)
    {
        if (result == 2 || result == 3) { capture.result = result; capture.phase = 3; }
        else platform_binding_capture_begin(); /* Delete clears; Escape cancels. */
    }
    if (capture.phase != 3) return 0;
    result = capture.result;
    *button = capture.button;
    controller_remap_capture_end();
    return result;
}

int controller_remap_input(short port, XINPUT_GAMEPAD *pad, int connected)
{
    unsigned char values[16], layout[CONTROLLER_REMAP_ACTIONS], targets[CONTROLLER_REMAP_ACTIONS];
    unsigned int held = 0;
    struct controller_device device;
    struct platform_input_state input;
    unsigned int actions = 0, allowed;
    int i, context, custom;
    if (port < 0 || port >= 4) return 0;
    /* Check from any polled port, including while the selected one is absent. */
    if (capture.phase && (!capture_device_present() ||
        (capture.phase < 4 && SDL_GetTicks() - capture.polled > 1000)))
        controller_remap_capture_end();
    /* Another controller cannot navigate the editor while it waits for the
     * selected port. Players still in gameplay retain their own input. */
    if (capture.phase && capture.port != port && !controller_remap_playing(port))
    {
        memset(pad, 0, sizeof(*pad));
        return 1;
    }
    physical_values(pad, values);
    for (i = 0; i < 16; i++) if (values[i] > 30) held |= 1u << i;
    platform_input_read(&input, FALSE);
    controller_device_get(port, &device);
    if (!input.focused) connected = 0;
    if (capture.phase && capture.port == port)
    {
        controller_device_get(port, &device);
        if (capture.phase < 3 && (!connected || device.instance != capture.instance))
        { capture.result = 3; capture.phase = 3; }
        if (capture.phase == 1 && !held) capture.phase = 2;
        else if (capture.phase == 2 && held)
        {
            if (held & (1u << 12))
            { capture.result = 3; capture.phase = 3; }
            else
                for (i = 0; i < 16; i++)
                    if (i != 12 && (held & (1u << i)))
                    { capture.button = i; capture.result = 1; capture.phase = 3; break; }
        }
        if (capture.phase == 4 && !held) { capture.phase = 0; capture.port = -1; }
        memset(pad, 0, sizeof(*pad));
        memset(&remap_ports[port], 0, sizeof(remap_ports[port]));
        return 1;
    }
    controller_remap_profile(port, targets);
    custom = controller_remap_read(port, layout);
    if (!custom) memcpy(layout, targets, sizeof(layout));
    context = connected ? controller_remap_context(port) : 0;
    if (context != remap_ports[port].context || custom != remap_ports[port].custom ||
        connected != remap_ports[port].connected || device.instance != remap_ports[port].instance ||
        memcmp(layout, remap_ports[port].layout, sizeof(layout)) ||
        memcmp(targets, remap_ports[port].targets, sizeof(targets)))
    {
        remap_ports[port].blocked = held;
        remap_ports[port].held = remap_ports[port].pressed = 0;
        memcpy(remap_ports[port].layout, layout, sizeof(layout));
        memcpy(remap_ports[port].targets, targets, sizeof(targets));
    }
    remap_ports[port].context = context;
    remap_ports[port].custom = custom;
    remap_ports[port].connected = connected;
    remap_ports[port].instance = device.instance;
    remap_ports[port].blocked &= held;
    if (!context)
    {
        remap_ports[port].held = remap_ports[port].pressed = 0;
        if (!connected) memset(pad, 0, sizeof(*pad));
        return 0; /* Menus and the on-screen keyboard use physical buttons. */
    }
    allowed = context == 3 ? ((1u << CONTROLLER_REMAP_ACTIONS) - (1u << PAD_CAMERA_FASTER)) :
        (1u << PAD_CAMERA_FASTER) - 1;
    if (context == 2) allowed |= 1u << PAD_CAMERA_SWITCH;
    allowed |= 1u << PAD_CAMERA_CYCLE;
    if (!custom) allowed = 0; /* Stock input retains its original shortcuts. */
    for (i = 0; i < CONTROLLER_REMAP_ACTIONS; i++)
    {
        int button = layout[i];
        if (!(allowed & (1u << i)) || button == CONTROLLER_REMAP_UNBOUND) continue;
        /* While the player controls an available free camera, its switch
         * owns its button. The rest of the debug layout is inactive. */
        if (context == 2 && i < PAD_CAMERA_FASTER && button == layout[PAD_CAMERA_SWITCH]) continue;
        if ((held & ~remap_ports[port].blocked) & (1u << button)) actions |= 1u << i;
    }
    if ((actions & (1u << PAD_CAMERA_CYCLE)) && !(remap_ports[port].held & (1u << PAD_CAMERA_CYCLE)))
    {
        remap_ports[port].cycle_since = SDL_GetTicks();
        remap_ports[port].cycle_count = 0;
    }
    remap_ports[port].pressed |= actions & ~remap_ports[port].held;
    remap_ports[port].held = actions;
    if (controller_remap_pressed(port, 1u << PAD_SCREENSHOT)) platform_screenshot_request();
    if (custom)
    {
        memset(pad->bAnalogButtons, 0, sizeof(pad->bAnalogButtons));
        pad->wButtons &= XINPUT_GAMEPAD_START;
        for (i = 0; i < CONTROLLER_REMAP_GAME_ACTIONS; i++)
            if (actions & (1u << i))
            {
                if (targets[i] < 8) pad->bAnalogButtons[targets[i]] = values[layout[i]];
                else pad->wButtons |= digital_masks[targets[i]-8];
            }
        if (actions & (1u << PAD_SCOREBOARD)) pad->wButtons |= XINPUT_GAMEPAD_BACK;
    }

    return 0;
}

int controller_remap_connected(short port)
{
    struct controller_device device;
    struct platform_input_state input;
    if (port < 0 || port >= 4) return 0;
    controller_device_get(port, &device);
    platform_input_read(&input, FALSE);
    return remap_ports[port].connected && input.focused && device.name[0] &&
        device.instance == remap_ports[port].instance;
}

unsigned int controller_remap_held(short port)
{
    return controller_remap_connected(port) && controller_remap_context(port) == remap_ports[port].context ?
        remap_ports[port].held : 0;
}

unsigned int controller_remap_pressed(short port, unsigned int mask)
{
    unsigned int result;
    if (port < 0 || port >= 4) return 0;
    result = controller_remap_connected(port) && controller_remap_context(port) == remap_ports[port].context ?
        remap_ports[port].pressed & mask : 0;
    remap_ports[port].pressed &= ~mask;
    return result;
}

/* Keep the stock hold-to-cycle shortcut independent of gameplay translation.
 * Context changes use the same release guard as other remapped actions. */
int controller_remap_camera_cycle(short port, int fast)
{
    Uint64 count;
    unsigned int pressed = controller_remap_pressed(port, 1u << PAD_CAMERA_CYCLE);
    if (!(controller_remap_held(port) & (1u << PAD_CAMERA_CYCLE))) return 0;
    if (fast) return pressed != 0;
    count = (SDL_GetTicks() - remap_ports[port].cycle_since) / 1000;
    if (count <= remap_ports[port].cycle_count) return 0;
    remap_ports[port].cycle_count = count;
    return 1;
}

int controller_remap_talk(void)
{
    short port;
    for (port = 0; port < 4; port++)
        if (controller_remap_held(port) & (1u << PAD_TALK)) return 1;
    return 0;
}

/* The existing SDL merge's maximum pressure / greatest stick deflection rules.
 * Run only after remapping the physical pad, so keys and touch keep their own bindings. */
void controller_remap_merge(XINPUT_GAMEPAD const *source, XINPUT_GAMEPAD *target)
{
    int i;
    target->wButtons |= source->wButtons;
    for (i = 0; i < 8; i++)
        if (source->bAnalogButtons[i] > target->bAnalogButtons[i]) target->bAnalogButtons[i] = source->bAnalogButtons[i];
    if (abs(source->sThumbLX) > abs(target->sThumbLX)) target->sThumbLX = source->sThumbLX;
    if (abs(source->sThumbLY) > abs(target->sThumbLY)) target->sThumbLY = source->sThumbLY;
    if (abs(source->sThumbRX) > abs(target->sThumbRX)) target->sThumbRX = source->sThumbRX;
    if (abs(source->sThumbRY) > abs(target->sThumbRY)) target->sThumbRY = source->sThumbRY;
}

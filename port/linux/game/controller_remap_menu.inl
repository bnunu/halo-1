/* The controller editor follows Controls Setup's pending Save/Cancel/Defaults
 * convention, with one complete layout per input port and no profile mutation. */
static wchar_t const *const pad_action_labels[CONTROLLER_REMAP_ACTIONS] = {
    L"JUMP", L"SWITCH GRENADE", L"USE / RELOAD", L"SWITCH WEAPON", L"MELEE",
    L"FLASHLIGHT", L"THROW GRENADE", L"FIRE", L"CROUCH", L"ZOOM",
    L"SCOREBOARD", L"SCREENSHOT", L"MOVE FORWARD", L"MOVE BACKWARD", L"STRAFE LEFT", L"STRAFE RIGHT",
    L"ALL CHAT", L"TEAM CHAT", L"VEHICLE CHAT", L"PUSH TO TALK",
    L"CAMERA SPEED UP", L"CAMERA SPEED DOWN", L"CAMERA / PLAYER SWITCH", L"CAMERA RAISE", L"CAMERA LOWER", L"CYCLE CAMERA"
};
static short const pad_action_groups[CONTROLLER_REMAP_ACTIONS] = {0,1,1,1,1,2,1,1,0,2,2,2,0,0,0,0,3,3,3,3,4,4,4,4,4,4};
static struct
{
    unsigned char buttons[4][CONTROLLER_REMAP_ACTIONS];
    boolean custom[4];
    short capturing, port, proposed, conflict, button, group;
} pad_editor;

static short pad_editor_port(struct widget_instance *screen)
{
    struct widget_instance *spinner = named(screen, "pad_player", 0);
    short port = spinner ? spinner->parameters.list.selected_index : 0;
    return port >= 0 && port < 4 ? port : 0;
}

static short pad_editor_action(struct widget_instance *widget)
{
    struct widget_instance *group = named(screen_of(widget), "pad_group", 0);
    short wanted, action;
    for (; widget && strncmp(widget->name, "op_pad_", 7); widget = widget->parent);
    if (!widget) return NONE;
    wanted = (short)(atoi(widget->name + 7) - 1);
    for (action = 0; action < CONTROLLER_REMAP_ACTIONS && wanted >= 0; action++)
        if (pad_action_groups[action] == (group ? group->parameters.list.selected_index : 0) && !wanted--)
            return action;
    return NONE;
}

/* A replacement is pending until the player accepts it. There is no swap. */
static void pad_editor_propose(short port, short action, int button)
{
    pad_editor.port = port;
    pad_editor.group = pad_action_groups[action];
    pad_editor.button = (short)button;
    pad_editor.conflict = (short)controller_remap_conflict(pad_editor.buttons[port], action, button);
    pad_editor.proposed = pad_editor.conflict == NONE ? NONE : action;
    if (pad_editor.proposed == NONE)
    {
        controller_remap_assign(pad_editor.buttons[port], action, button);
        pad_editor.custom[port] = TRUE;
    }
}

static boolean pad_editor_event(struct widget_instance *widget, char const *name)
{
    struct widget_instance *screen = screen_of(widget);
    short port = pad_editor_port(screen), action, i;
    if (!strcmp(name, "port controller init"))
    {
        struct widget_instance *spinner = named(screen, "pad_player", 0);
        controller_remap_capture_end();
        pad_editor.capturing = pad_editor.proposed = NONE;
        for (i = 0; i < 4; i++)
        {
            controller_remap_profile(i, pad_editor.buttons[i]);
            pad_editor.custom[i] = (boolean)controller_remap_read(i, pad_editor.buttons[i]);
        }
        if (spinner) spinner->parameters.list.selected_index = controller_of(widget);
    }
    else if (!strcmp(name, "port controller close"))
    {
        pad_editor.capturing = pad_editor.proposed = NONE;
        controller_remap_capture_end();
    }
    else if (!strcmp(name, "port controller cancel"))
    {
        if (pad_editor.proposed != NONE)
        { pad_editor.proposed = NONE; return FALSE; }
        return TRUE;
    }
    else if (pad_editor.proposed != NONE)
    {
        if (!strcmp(name, "port controller bind"))
        {
            controller_remap_assign(pad_editor.buttons[pad_editor.port], pad_editor.proposed, pad_editor.button);
            pad_editor.custom[pad_editor.port] = TRUE;
            pad_editor.proposed = NONE;
        }
        return FALSE;
    }
    else if (!strcmp(name, "port controller defaults"))
    {
        controller_remap_profile(port, pad_editor.buttons[port]);
        pad_editor.custom[port] = FALSE;
    }
    else if (!strcmp(name, "port controller save"))
    {
        boolean result = TRUE;
        if (pad_editor.capturing != NONE) return FALSE;
        for (i = 0; i < 4; i++)
        {
            char text[CONTROLLER_REMAP_TEXT] = {0};
            if (pad_editor.custom[i]) controller_remap_format(pad_editor.buttons[i], text, sizeof(text));
            if (strcmp(text, config_string(controller_remap_setting(i))) &&
                !config_write(controller_remap_setting(i), text)) result = FALSE;
        }
        return result;
    }
    else if ((action = pad_editor_action(widget)) != NONE)
    {
        if (!strcmp(name, "port controller clear"))
            pad_editor_propose(port, action, CONTROLLER_REMAP_UNBOUND);
        else if (!strcmp(name, "port controller bind"))
        {
            pad_editor.capturing = action;
            pad_editor.port = port;
            controller_remap_capture_begin(port);
        }
        else
        {
            int direction = !strcmp(name, "port controller left") ? -1 : 1;
            pad_editor_propose(port, action, controller_remap_cycle(pad_editor.buttons[port][action], direction));
        }
    }
    return TRUE;
}

static void pad_editor_update(struct widget_instance *list)
{
    struct widget_instance *screen = screen_of(list), *row;
    struct controller_device device;
    short port = pad_editor_port(screen);
    if (pad_editor.proposed != NONE && (port != pad_editor.port ||
        (named(screen, "pad_group", 0) && named(screen, "pad_group", 0)->parameters.list.selected_index != pad_editor.group)))
        pad_editor.proposed = NONE;
    if (pad_editor.capturing != NONE)
    {
        int button, result = controller_remap_capture_poll(&button);
        if (result)
        {
            if (result == 1 || result == 2)
                pad_editor_propose(pad_editor.port, pad_editor.capturing, result == 2 ? CONTROLLER_REMAP_UNBOUND : button);
            pad_editor.capturing = NONE;
        }
    }
    controller_device_get(port, &device);
    for (row = list->child; row; row = row->next)
    {
        short action;
        wchar_t text[64];
        char const *label;
        short i;
        if (strncmp(row->name, "op_pad_", 7)) continue;
        action = pad_editor_action(row);
        row->visible = action != NONE;
        if (action == NONE)
        {
            if (list->focused_child == row) focus_row(list, 1);
            continue;
        }
        text_set(named(row, "pad_action", 0), pad_action_labels[action]);
        label = pad_editor.capturing == action && pad_editor.port == port ? "PRESS A BUTTON" :
            controller_remap_button_name(pad_editor.buttons[port][action], device.family);
        for (i = 0; label[i] && i < 63; i++) text[i] = (wchar_t)(unsigned char)label[i];
        text[i] = 0;
        text_set(named(row, "pad_binding", 0), text);
    }
    if (list->parameters.list.extended_description)
    {
        wchar_t text[256];
        char const *accept = controller_remap_button_name(0, device.family);
        char const *cancel = controller_remap_button_name(1, device.family);
        char const *clear = controller_remap_button_name(2, device.family);
        char const *pause = device.family == CONTROLLER_PLAYSTATION ? "Start / Options" :
            device.family == CONTROLLER_SWITCH ? "+" : "Start / Menu";
        char caption[256];
        short i;
        if (pad_editor.proposed != NONE)
        {
            wchar_t const *label = pad_action_labels[pad_editor.conflict];
            char conflict[64];
            for (i = 0; label[i] && i < 63; i++) conflict[i] = (char)label[i];
            conflict[i] = 0;
            snprintf(caption, sizeof(caption), "%s already uses %s.\r\nReplacing will leave that action UNBOUND.\r\nPress %s or Enter to replace it.\r\nPress %s or Escape to cancel.",
                conflict, controller_remap_button_name(pad_editor.button, device.family), accept, cancel);
        }
        else if (pad_editor.capturing != NONE)
            snprintf(caption, sizeof(caption), "Release the button, then press a new one\r\non player %d's controller.\r\nPress %s or Escape to cancel.\r\nDelete on the keyboard leaves it UNBOUND.", pad_editor.port + 1, pause);
        else
            snprintf(caption, sizeof(caption), "Press %s or Enter to choose a new button.\r\nLeft or Right also changes the button.\r\nPress %s on the controller or Delete\r\non the keyboard to leave this action UNBOUND.", accept, clear);
        if (pad_editor.proposed == NONE && pad_editor.capturing == NONE && list->focused_child)
        {
            if (!strcmp(list->focused_child->name, "op_player"))
                snprintf(caption, sizeof(caption), "Choose which player's buttons to edit.\r\nEach player has a separate saved layout.\r\nButton names follow their connected controller.\r\nOK saves changes; Cancel discards them.");
            else if (!strcmp(list->focused_child->name, "op_group"))
            {
                short group = named(screen, "pad_group", 0)->parameters.list.selected_index;
                snprintf(caption, sizeof(caption), "Choose a group of actions with Left or Right.\r\nThen select an action to change its button.\r\nUNBOUND means no button is assigned.");
                if (group == 3)
                    snprintf(caption, sizeof(caption), "Choose buttons for chat and push-to-talk.\r\nChat belongs to the player who opens it.\r\nVoice uses the shared microphone and the\r\nfirst player's team and vehicle channels.");
                else if (group == 4)
                    snprintf(caption, sizeof(caption), "Hold Cycle Camera for one second to change views.\r\nCamera / Player Switch changes who you control.\r\nOther actions control an available debug camera.\r\nThese buttons can reuse your gameplay buttons.");
            }
        }
        for (i = 0; caption[i] && i < 255; i++) text[i] = (wchar_t)(unsigned char)caption[i];
        text[i] = 0;
        text_set_length(list->parameters.list.extended_description, text, 256);
    }
}

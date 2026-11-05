# Controller button remapping

Gamepad Setup > **Button Remapping** edits separate P1â€“P4 layouts. These belong
to input slots on this installation, not device serial numbers or player profiles.
Empty settings keep the profile preset and stock shortcuts.

Choose Movement, Combat, General, Communication or Debug. Select an action, release
the opening press, then press the desired button on that player's controller.
All four D-pad directions and Back/Select are available. Start/Options and Escape
cancel capture. Button names follow the detected controller family independently
of Button Style. Stick axes, sensitivity and menu navigation keep their settings.

Left/right also cycles assignments, including **UNBOUND**. The menu's X action
(Square on PlayStation, Y on Switch) or keyboard Delete clears the selected action;
Delete also clears during capture. Assigning an occupied button proposes **Replace**,
names the affected action and waits for Select to confirm or Back to cancel.
Replacing clears the previous action; it never swaps the two. Multiple actions may
be unbound, including every action. Start remains available to open Pause.

OK saves pending edits for all four slots. Cancel discards them. Defaults
restores the selected slot's profile when saved. Use / Reload remains combined.
General includes Flashlight, Zoom, Scoreboard (hold) and Screenshot (one press).
Movement buttons use forward/backward/strafe directions independently of stick
presets, with the keyboard controls' diagonal and crouch-walk speeds.

Communication includes All Chat, Team Chat, Vehicle Chat and Push to Talk.
Chat opens once per press and belongs to the local player who opened it: their
viewport, name, team and vehicle. The same controller drives the on-screen keyboard.
Only one line can be edited at a time. A departing or disconnected owner cancels it.
Chat remains subject to the game's existing multiplayer and host chat settings.
Voice uses the existing shared microphone and first player's voice routing; any
slot can hold its assigned Push to Talk button. Enable push-to-talk in voice settings.
Keyboard and touch retain their own bindings.

Debug includes Camera Speed Up/Down, Camera / Player Switch, Camera Raise/Lower
and Cycle Camera. These reuse buttons independently of gameplay. Hold Cycle Camera
for one second to advance through the director's views: first person, flying,
following, then first person again during normal play. Release between view changes.
Its default is RB/Black. The existing `director_camera_switch_fast` debug setting
changes it to one cycle per press. The director still refuses cycling for clients
in another machine's game; remapping does not bypass that restriction.

The other debug actions apply only to an already available flying, orbiting or
editor camera. Camera / Player Switch changes whether input controls the camera or
player. While controlling the player with a debug camera available, only the debug
switch takes priority over gameplay. While controlling the camera, the whole debug
layout takes priority. Defaults match stock camera controls: D-pad Up/Down,
Right Stick, RT and LT. All can be changed or cleared. Cycle Camera also works
while controlling the player; its button can still perform its gameplay action,
just as the stock RB shortcut does. Clearing Cycle Camera disables custom camera
cycling, including the old shortcut through the translated gameplay buttons.
Keyboard R/F still raise and lower the camera with a custom layout. Holding the
middle mouse button gives the mouse camera controls priority, with the wheel
adjusting speed. Camera movement uses the player's movement and look settings.
Menus and chat continue to block custom camera input in either mode.

Gameplay prompts use the physical binding, or **UNBOUND** for a cleared action.
Xbox and CE retain their existing glyph renderers, sizes and anchoring.

## Reading route and integration

1. [controller_remap.c](../../port/linux/src/controller_remap.c) owns tokens,
   parsing, labels, defaults and conflict resolution. Configuration is cached by
   `config_changes`; invalid lists fall back to the profile without partial writes.
2. [controller_remap_menu.inl](../../port/linux/game/controller_remap_menu.inl)
   owns pending edits, replacement proposals and capture. `_controller_screen` in
   [port_settings.py](../../tools/port_settings.py) generates the submenu; regenerate
   with `python tools/ce_menus.py --settings-only`.
3. [controller_remap_input.c](../../port/linux/src/controller_remap_input.c)
   reads physical buttons before keyboard/touch merging. It translates the original
   ten gameplay actions into the profile's buttons, preserving pressure and hold
   timers. Additional commands use semantic action bits, never synthetic key presses.
   Capture, gameplay, menus and camera contexts have independent release guards.
4. [controller_remap_game.c](../../port/linux/game/controller_remap_game.c) maps
   physical ports to local players, reads profile and camera context, supplies
   digital movement and adjusts a temporary copy of prompt preferences. Its main-menu
   guard is separate from per-player widgets: the UI map has a live player, while
   the main-menu root belongs to any player (`NONE`). Pause menus retain their
   per-player ownership so other split-screen players can continue playing.
5. The minimal hooks in [xinput_sdl.c](../../port/linux/src/xinput_sdl.c) provide
   physical input, per-port chat navigation and push-to-talk. Chat's existing
   [network_chat.c](../../port/linux/game/network_chat.c) editor retains its normal
   host validation and packet format while recording its local owner. No network
   version change is needed: existing chat packets already identify their sender,
   and the host already validates that sender against the sending machine.
6. Small `port:` hooks in [director.c](../../source/camera/director.c) supply camera
   availability and map cleanup, and pass the custom cycle request to the existing
   camera selector. `controller_remap_camera_cycle` owns its physical-button timing
   in the input module; the director owns camera selection and permissions, which
   cannot be implemented solely in that module. No map-family selector is changed.
   [input_abstraction.c](../../source/input/input_abstraction.c)
   applies digital movement; [hud_messaging.c](../../source/interface/hud_messaging.c)
   maps prompt identity and renders UNBOUND. Those private engine states cannot be
   accessed solely from a feature module. No map loading path is changed.

## Persistence and lifecycle

`input.controller_p1` through `input.controller_p4` default to empty strings.
New custom layouts use `v3:` followed by 26 comma-separated entries in this order:

```
jump, grenade type, use/reload, weapon, melee, flashlight, grenade, fire, crouch,
zoom, scoreboard, screenshot, forward, backward, left, right, all chat, team chat,
vehicle chat, push to talk, camera faster, camera slower, camera/player switch,
camera raise, camera lower, cycle camera
```

Entries are physical tokens: `south,east,west,north,rb,lb,lt,rt,ls,rs,dpad_up,
dpad_down,dpad_left,dpad_right,back`, or `unbound`. South means A on Xbox, Cross on
PlayStation and B on Switch. Tokens must be unique within gameplay/communication
and separately within Debug; `unbound` may repeat. Unbound saves remain distinct
from an empty setting (profile defaults).

Old ten-action and 25-action `v2:` strings remain readable. The appended Cycle
Camera defaults to RB unless an old debug action already owns RB, in which case
Cycle Camera starts unbound and can be assigned in the Debug group. Existing
assignments are preserved. Ten-action strings' original actions take priority over
new defaults, including Recon's use of D-pad Left. P1 retains the Up chat shortcut;
other slots default to Up movement. Additional chat channels, voice and screenshot
start unbound. Saving writes the new format. Custom digital movement now has the
explicit directions above; stick presets still govern the analog sticks.

Disconnects, focus loss, menu/chat transitions, layout changes and map cleanup
release transient action states. Buttons held across a transition must be released
before they trigger gameplay again. Capture has a watchdog for an abandoned menu.
Clearing a layout never changes another player's bindings or a saved player profile.

## Verification

[Controller tests](../../tools/test_controller_remap.py) execute production C for
parsing, migration, replacement, cleared saves, all stock presets, per-player input,
pressure, capture, disconnects, focus and context changes, semantic commands,
prompt identity and pending menu state. Chat ownership and hold-over checks use
the real production functions. [Camera routing tests](../../tools/harness/tests/test_controller_remap_camera.py)
check stock and remapped camera input alongside keyboard and mouse controls,
including mouse priority and menu/chat blocking.
[Context integration tests](../../tools/test_controller_remap_context.py)
combine the real widget-ownership check, remapper and director input for main-menu
navigation, pause, camera cycling, alternate bindings, timing and lifecycle guards.
Tests distinguish those contracts from physical driver behaviour. Runtime
screenshots and Xbox/CE gameplay checks complement the tests;
real-controller playtesting is still needed for device-specific behaviour.

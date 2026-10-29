# Linux

`ninja linux` compiles the game with clang for 32-bit x86 Linux. The result
is a native executable, `build/linux/halo`. The game shows its graphics with
OpenGL 4.5. It plays sound through SDL3. It accepts keyboard, mouse and
gamepad input.

The game is 32-bit code because its data (tags, cache files, saved games)
contains 32-bit pointers, as on the Xbox.

## Requirements

You do not need the Xbox SDK. The declarations that the game uses are in
`port/include/xdk`.

To build:

- Python and ninja.
- clang. The option `--linux-cc` of `configure.py` selects a different
  compiler.
- The 32-bit glibc development files: `lib32-glibc` on Arch Linux,
  `gcc-multilib` and `libc6-dev-i386` on Debian and Ubuntu.
- The 32-bit SDL3: `lib32-sdl3` on Arch Linux, `libsdl3-dev:i386` on Debian
  and Ubuntu. The portable build does not need it: refer to "Portable
  build".

To start the game:

- glibc 2.29 or later (32-bit), for the builds from GitHub Actions, which
  are built against glibc 2.31. They start on SteamOS 3 (Steam Deck). They
  should also start on Debian 11, Ubuntu 20.04, Fedora 32 and later
  distributions, but this is not tested. A build that you make without
  `--portable` needs the glibc of the computer that built it, or a later
  one.
- The 32-bit OpenGL libraries (`lib32-mesa`).
- The 32-bit PipeWire or PulseAudio client libraries (`lib32-pipewire` or
  `lib32-libpulse`).
- The 32-bit X11 libraries (`lib32-libx11`, `lib32-libxext`, and
  `lib32-libxi`, with which SDL holds the mouse; without it the game hides
  the pointer and moves it back to the middle of the window instead). In a
  Wayland session, the game uses them through XWayland. SDL uses Wayland
  itself only if the 32-bit Wayland libraries are version 1.20 or later
  (`lib32-wayland`, `lib32-libxkbcommon`; Debian 11, Ubuntu 20.04 and
  Fedora 32 have older ones) and the compositor has the fifo-v1 protocol.
  On GNOME, the window then has borders only with the 32-bit libdecor
  (`lib32-libdecor`).

SteamOS has all of these. Its system is read-only, and the builds from
GitHub Actions need no package: they bring their own SDL3
(`libSDL3.so.0`, next to the executable).

### Portable build

The builds from GitHub Actions are portable builds (`--portable`; refer to
"Build options" in the main [README](../../README.md#build-options)). The
portable build starts on more systems than the build machine's:

- It is compiled and linked against the 32-bit glibc 2.31 of Debian 11, the
  glibc of the Steam Runtime 3 ("sniper"), not against the glibc of the
  build machine. An executable needs the glibc version that it was built
  against, or a later one.
- It brings SDL 3 (`libSDL3.so.0`), built from source against the same
  glibc. The executable looks for it in its own folder first (a `DT_RPATH`
  of `$ORIGIN`, which comes before `LD_LIBRARY_PATH`), so that it uses this
  SDL even when Steam sets `LD_LIBRARY_PATH`. Few distributions have a
  32-bit SDL 3. This SDL loads X11, Wayland, libdecor, PipeWire, PulseAudio
  and ALSA when it starts, so it starts with whichever of them the system
  has. It is linked only to glibc. Its license is `SDL3-LICENSE.txt`.

At the first build, `tools/linux_sysroot.py` downloads the Debian packages
(about 25 MB) and the source of SDL. Each package comes from
`archive.debian.org` or `deb.debian.org`, or else from
`snapshot.debian.org`; the source of SDL comes from GitHub or from
`libsdl.org`. It checks each download against its SHA-256 sum. The system root is in
`build/linux/third_party/sysroot`. `tools/linux_sysroot.cmake` builds SDL
against it.

To make the portable build you need, in addition to the tools above,
CMake, `pkg-config` (`pkgconf`) and `wayland-scanner` (`wayland` on Arch
Linux, `libwayland-bin` on Debian and Ubuntu). You do not need the 32-bit
SDL3. You need the 32-bit glibc only for the tests. Keep the checkout in a
folder whose path has no spaces: SDL's build takes the system root's
include folders from `pkg-config`, which splits a path at its spaces, so the
portable build stops with a message there.

To see the glibc version that a build needs, enter
`readelf -V build/linux/halo | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1`.

## Build the game

1. Go to the root folder of the repository.
2. Enter `python configure.py`.
3. Enter `ninja linux`.

## Start the game

Enter `build/linux/halo`.

To play on a Steam Deck, unpack `halo-linux-release.zip` into a folder (in
Desktop Mode), and add `halo` to Steam as a non-Steam game ("Add a Game" in
the Games menu of Steam). The game then starts in Game Mode as well. Refer
to "Steam Deck".

The game data is the folder that contains `maps/`, from an Xbox disc image
of any version of the game. The game looks for this folder in this
sequence:

1. `paths.data` in `config.toml`.
2. The current folder.
3. The folder of the executable.
4. `assets/` in the current folder, and `assets/` in the repository that
   contains the executable.

If the game finds no data, it asks for an Xbox disc image (`.xiso` or
`.iso`). This occurs at the first start. First the game looks for a disc
image in the folder of the executable (a name that ends in `.iso` or
`.xiso`, in capitals or not). If it finds one, it names that image in its
question:

- Select "No" to stop the game.
- Select "Yes" to copy `maps/` from the disc image. If the game found an
  image next to the executable, it uses that image and shows no file picker.
  Otherwise it opens a file picker: select the disc image, or cancel to see
  the question again. The game copies `maps/` next to the executable and
  shows the progress. If the copy from the image found next to the
  executable fails, the next "Yes" opens the file picker.

On Linux, the file picker needs the desktop portal or `zenity`. If neither
is present, the game says to put the disc image next to the executable (or
to install `zenity`) and stops. Put the image there and start the game
again.

The game writes the copy to `maps.partial`. When the copy is complete, the
game changes the name to `maps`. If the copy stops before it is complete,
the game asks for the disc image again at the next start.

### Steam Deck

Do the set-up in Desktop Mode: unpack the release, put `maps/` next to
`halo` (or start the game one time to copy it from a disc image), and add
`halo` to Steam. Then start the game from the library in Game Mode.

- Screen: the game fills the screen without borders (`display.mode` empty,
  thus borderless). The 3D view and the HUD have the shape of the screen
  (16:10) and are drawn at its resolution; the menus are at the center.
- Controls: Steam Input gives the game a virtual controller, which the game
  operates as the controller of the Xbox. With the template that Steam selects
  ("Gamepad With Joystick Trackpad"), the buttons, sticks and triggers have
  the functions of the same controls on an Xbox controller, the right
  trackpad operates as the right stick, and the back buttons (L4, L5, R4,
  R5) do nothing until you assign them in the controller settings of the
  game in Steam. With a template that makes a trackpad a mouse, the mouse
  aims in the game and moves the pointer in the menus.
- Text: names of profiles and gametypes use the keyboard of the game, which
  the controller operates. The text fields of the menus (the name and the
  password in Server Setup, and the password of a game in the server
  browser) open the keyboard of Steam. Type the text, then select Enter on
  that keyboard.
- Frame rate: the game shows one frame for each refresh of the display. It
  follows the refresh rate and the frame limit of Quick Access >
  Performance (40 to 60 Hz on the LCD model, up to 90 Hz on the OLED model).
  The world is calculated at 30 Hz at all rates. Keep `display.vsync =
  true`. Refer to "Frame rate".
- Sleep: the clocks of the game do not count the time that the Deck sleeps,
  so the game continues from where it stopped. A network game does not
  wait: the host drops a machine that it has not heard from for 15 seconds,
  and while a Deck that hosts sleeps, the other players have no host.

### NVIDIA on Wayland

The game is a 32-bit program. NVIDIA's driver draws OpenGL into a Wayland
window through its `egl-wayland` library, which most distributions only
ship 64-bit (Arch and its derivatives have no 32-bit package of it). A
32-bit OpenGL window on Wayland then falls back to Mesa's `llvmpipe`, which
draws on the processor: the log shows `OpenGL ... on llvmpipe`, and the
game is slow or never finishes loading.

So on a Wayland desktop with NVIDIA's driver loaded, the game opens its
window through XWayland (X11), where the 32-bit driver draws, unless a
32-bit `egl-wayland` is installed. The log says so ("video: NVIDIA's
driver ..."). `SDL_VIDEO_DRIVER=wayland` or `SDL_VIDEO_DRIVER=x11` chooses
the video driver yourself.

## Files and folders

| Xbox drive | Folder |
| --- | --- |
| `d:\` | The data root: the folder that contains `maps/`. |
| `z:\` | `z/` in the save root. This folder contains the cache (approximately 800 MB of map data) and the saved games. |
| `u:\` | `u/` in the save root. This folder contains the user data. |

The save root is `paths.saves` in `config.toml`. If that setting is empty,
the save root is `$XDG_DATA_HOME/halo-linux` (usually
`~/.local/share/halo-linux`).

The game makes the folders when it needs them. Names of files and folders
are not case-sensitive, as on the Xbox.

On Windows and Linux the save root also has `shader_cache/`: one file for
each graphics driver (named by a hash of its vendor, renderer and version),
with the GPU programs the game has linked on it (`display.shader_cache`).
The first time a material or an effect comes into view the driver links its
program, which can stop the game for a frame of 30 to 90 ms (more on
integrated graphics); after that the program is loaded from this file in
under 2 ms, in that game and the next. The files grow to 64 MB at most (a
large map's programs take some 2 MB). Delete the folder to clear them.

These files are in the data root:

| File | Contents |
| --- | --- |
| `debug.txt` | The log of the game. At start-up, the game shows the data root in the terminal. A crash writes its report (the faulting address and the calls that led to it) here as well; the `reference address` line at the top of each session places those addresses in the build. |
| `init.txt` | Console commands that the game does at start-up. For example, `map_name levels\a10\a10` starts the first campaign level. |
| `mods/` | Mods: folders of Halo PC tag files whose sounds, bitmaps, text and models play over the maps' on this machine, in the order of the main menu's Mods (`load_order.toml`). Refer to [docs/mods.md](../../docs/mods.md). |

The settings are in `config.toml` next to the executable. Refer to
"Settings". Internet play's MQTT brokers are in `brokers.txt` next to it
(`network.brokers_file`).

If the game stops because of a fatal signal, it writes the address and a
backtrace to the standard error. To find the function at the address, enter
`addr2line -e build/linux/halo <address>`.

## Controls

The keyboard and the mouse are a control scheme of their own for the player
of controller 1: each action has up to two keys or mouse buttons, which
Settings > Controls Setup (or `[controls]` in `config.toml`) changes. The
game adds the input of the first gamepad to controller 1. The other
gamepads operate controllers 2 to 4. With two or more players on this
computer (co-op, or split screen in a network game) and only one gamepad,
that gamepad is controller 2 (player 2) and the keyboard and mouse stay
controller 1; with more gamepads, SPACE on the ADD PLAYER screen does the
same for the first (refer to the network games' split screen below). The gamepad changes controller only when none of its buttons
is held. The profile's button layout (Settings > Gamepads) is the
gamepads' only. Besides the Xbox's five, it has the Master Chief
Collection's Bumper Jumper: jump on the left bumper, melee on the right,
switching grenades on A, action and reload on B and the flashlight on X.
Fishsticks has zoom on the left trigger and melee on the right stick's
button, the grenade thrown with the right bumper, crouch on B and grenades
switched with the pad's left direction (which then does not strafe or turn).

Recon is the standard layout with action and reload on the right bumper and
grenades switched with the left bumper (the flashlight moves to X).

| Action | Keys and buttons (default) |
| --- | --- |
| move | W, A, S, D |
| aim | mouse (direct aim) |
| fire | left mouse button |
| throw a grenade | right mouse button, G |
| jump (and skip a cutscene) | space |
| crouch | left ctrl, C (C alone in the web build) |
| melee | F, mouse button 4 |
| reload | R |
| action (pick up, hold to swap weapons, enter or leave a vehicle; never reloads) | E |
| change the weapon | mouse wheel, 1 |
| change the grenade | X |
| flashlight | Q |
| zoom | Z, middle mouse button |
| show the scores (hold) | tab |
| pause menu | escape |
| screenshot | F10 |
| talk in voice chat (hold) | V |
| type a chat line to everyone | T |
| type a chat line to your team | Y |
| vote yes in the vote running (to kick a player, to end the game) | F1 (a controller: the D-pad's left, while the vote runs) |
| vote no in the vote running | F2 (a controller: the D-pad's right, while the vote runs) |
| while typing a chat line: to everyone, the team or the vehicle, in turn | Tab |

With a controller, or the touch controls' D-pad, D-pad Up opens the text
chat's on-screen keyboard (refer to "Text chat").

Always: \` opens the developer console, F12 releases or captures the mouse,
F11 changes between fullscreen and window. Screenshot (default F10,
rebindable under Controls Setup > Actions, below Pause Menu; not on Android) saves a PNG of the completed
frame to `screenshots/` beside `maps/`, named `YYYY-MM-DD_HH.MM.SS.png` in
local time, and prints the filename in the console. Captures in the same
second get a numeric suffix so previous screenshots are preserved.

One movement of the mouse wheel changes the weapon one time. A second
movement after a short pause changes it again.

In the menus, the mouse moves a pointer, a blue arrow
(`port/assets/cursor/menu_cursor.png`) in place of the system's, sized to
the window as Halo PC's was (Settings > Mouse > MENU CURSOR:
`display.menu_cursor`, large by default, 32 of the menus' 480 lines tall;
off shows the system's pointer):

- The item below the pointer gets the focus.
- A left click selects the item. On a setting with values, a click on the
  left or right half changes the value. On a button in the key of a screen
  (for example "B = Back"), a click pushes that button.
- On the on-screen keyboard (a profile's name), a left click presses the
  key below the pointer, or pushes the "B =BACK" or "A =ENTER" legend.
- A right click goes back.
- The mouse wheel moves through the items.

The keyboard also operates the menus, with keys of its own: the arrow keys
(and W, A, S, D) move, space or enter selects, escape or backspace goes
back (escape resumes the game from the pause menu), delete deletes. When the game continues, the mouse aims again. A mouse button that you hold from the menu does not fire until
you push it again.

## Menus

The menus are the PC version's (Halo Custom Edition's): its main menu and
every screen it leads to (campaign, profiles, multiplayer with its server
browser, direct IP and server setup, the gametype editor, and the profile's
settings: controls, gamepads, mouse, audio, video, network and colour),
laid out as it has them. Their pictures are the high-res redraws; the few
that are not yet (3D renders, screenshots) are drawn from the Xbox's own
menus where it has the same, else a placeholder, listed in
`port/assets/menus/NON_HANDDRAWN.md`.

The campaign is wired: Campaign, on player 1's profile (the one last used),
continues its saved game, starts a level it has reached at a difficulty (New
Game), or loads or deletes any profile's saved game (Load Game). Many of the
other functions behind the screens are not wired to this game yet
(`port/assets/menus/UNWIRED.md`): the lists they fill (profiles, maps,
servers, key bindings) are empty, and the settings they change do not
change. The screens open, close and move between each other as the PC
version's. `display.menus = "xbox"` gives the Xbox's menus, which start every
kind of game.

The menus' text is in the language of the Custom Edition whose `loc.map` is
in `custom_maps` (French, German, Italian, Spanish and so on), as that
edition shows it: each of the PC version's texts is that `loc.map`'s
(`display.menu_language`). Without one, and for the lines the port adds,
it is English. The pictures with words in them (the screens' titles, the
main menu's items) are drawn in English.

Multiplayer > CO-OP CAMPAIGN is the Xbox's cooperative play, which the PC
version does not have: two players on this computer play the campaign in
split screen. Player 1 is the player who chose it, on the current profile.
Player 2 then chooses a profile with their own controller (a gamepad), and
New Game's levels are those either profile has reached. Either player's
controller chooses the level and the difficulty. A co-op game does
not continue a saved game of one player.

Network games have split screen too: up to 4 players on each computer. The
game lobby's ADD PLAYER button, or another controller's START, opens the
Xbox's SELECT PROFILE screen (its own widgets): a quarter for each
controller. Each controller not yet playing presses A to join, chooses a
profile with left and right (it starts on one no other player has, and one
another player has cannot be selected), and presses A to select it; its B
goes back a step. A when all players have selected their profiles (Enter on
the keyboard) brings them into the lobby; player 1's B (Escape) goes back
without them. The keyboard and the first gamepad share controller 1 until
SPACE is pressed on that screen: the keyboard is then player 1 alone, and
that gamepad joins in a quarter of its own (with four gamepads, every
controller is taken, and the keyboard stays with the first). The screen's
bottom left lists the keyboard's keys. Two players on one profile get
different names from the host. A
player's B in the lobby leaves the game alone, and the last player of the
computer leaves it for all of them. When all the players are on the host's
computer, START NOW starts the game without another computer, and a team
game does not need a player on each team. The host's START NOW starts the
game at once; another computer's starts the countdown or takes five
seconds off it. In the game, each player's pause menu
opens on their part of the screen, and its LEAVE GAME is theirs: their part
of the screen stays until the game ends. A game under way shows its own
screen before JOIN GAME: players join there the same way, START or ADD
PLAYER, and JOIN GAME brings them all into the game.

In a multiplayer game, the pause menu (escape) has SETTINGS, which opens
the profile's settings while the game goes on, and for the host END GAME.
There Network Setup has only its pages that work in a game (Map Sharing,
Multiplayer HUD with the text chat, and Streamer Mode), not INTERNET PLAY,
which would drop the game, nor the rows for the main menu.
END GAME ends the game as its time limit does: the players stay, and after
the carnage report the host picks the next map and gametype (PICK GAME).
LEAVE GAME of the host still ends the game for everyone. The buttons go into
the map's own pause menu, before LEAVE GAME, so the buttons of a custom map
stay. The Xbox's pause box is drawn taller to hold them (a redraw,
`port/assets/menus/port_svg/pause`); a custom map's box keeps its size, and
what is below its list moves down. The few pictures of the settings that
come from the main menu's map are not drawn there.

In a single-player campaign, the pause menu has SETTINGS too, before REVERT
TO SAVED, in the same box: a list with room for it centres its rows, and
one without keeps its size, its rows closer. It opens the same settings,
while the game stays paused, with only Controls, Gamepads, Mouse, Audio and
Video Setup; Gamepads' OK saves the profile at once. In co-op the settings
have Network Setup too, with its in-game pages as in a multiplayer game.

In an online co-op game, the pause menu also has FRIENDLY FIRE ON or
FRIENDLY FIRE OFF, after SETTINGS, in a box drawn taller for it: whether this
computer's players hurt, and are hurt by, the other players
(`network.coop_own_friendly_fire`). A or enter changes it, from that moment.

The menus are XML files in `port/assets/menus` (`tools/ce_menus.py` writes
them from the PC version's tags), which the game contains. To change them,
put files in a `menus` folder next to `config.toml`: a file with the same
path replaces one of the game's, and another `.xml` file is added.
`port/assets/menus/README.md` describes the files. If a file has a problem,
the game uses the Xbox's menus and the log names the file, the line and the
problem.

## Bots

Computer players (bots) can play a local multiplayer game with you:
`bots.count` (1 to 15) of them join the lobby of a Slayer game played on
this computer, which is Multiplayer > Split Screen in the Xbox's menus
(`display.menus = "xbox"`; the PC version's menus have no game of this
computer alone). They are players as you are: in the scores, the respawns
and the game's rules, on the teams that `bots.teams` asks for, and they
follow the teams' changes in the lobby. They look for enemies, aim, fire,
throw grenades and pick up better weapons, as well as `bots.skill` lets
them, and walk the paths that the campaign's AI finds on the maps' walkable
surfaces (`game/bots.c`). They are never in system link or internet games,
nor in other gametypes (but SWAT, Infection and Gun Game, which are
Slayer's: below), and nothing that the machines send each other changes.

`bots.skill` sets six levels, from `"recruit"` (for new players: it reacts
late, aims loosely and never throws) to `"legendary"`. Each level fights as
a player of that level does (`game/bots_combat.c`): its reaction is never
the same twice; the lower levels' aim trails a target that turns back, and
every level but `"legendary"` keeps an aim error at long range; from
`"normal"` up a bot aims at the head once the shields are down, paces the
pistol at range and zooms the sniper rifle (again, a moment after a hit
knocks the zoom out); its blows and grenades follow its level (how near,
how soon, how often; from `"heroic"` a throw leads the target's path, from
`"veteran"` one goes to where a target was last seen); it strafes in the
level's rhythm, from long sidesteps to short jiggles at the top, halts,
crouches and jumps; it steps in for a blow when near; from `"normal"` it
backs off along a path when it is losing, and from `"veteran"` as soon as
its shields pop; from `"heroic"` it runs in to finish a won fight; it does
not see a player far above or below where it looks (unless near, or shot by
them); and from `"veteran"` it prefers high ground as it roams. halo.log's
bots line each minute is followed by the fight's counts.

## OpenCE gametypes

Create Game's Select Gametype screen has a bank between STANDARD and
CUSTOM: OPENCE, the port's own gametypes (`game/game_modes.c`).

| Gametype | Rules |
| --- | --- |
| SWAT | Team Slayer with no shields, in which a shot to the head of any weapon kills (a bullet's, a plasma bolt's, a needle's or a shotgun pellet's, not a melee's or an explosion's). Everyone starts with a pistol and an assault rifle; no weapons are on the map, no one has grenades, and there is no motion sensor. |
| Infection | Team Slayer. Humans are red and keep the map's weapons; zombies are blue, carry the Oddball skull and nothing else, move faster, hit harder with the skull, and cannot drive, pick up grenades or powerups. Once two players are alive the round counts down, then the host chooses the first zombies (a human is always left). A human killed by a zombie, by themselves or by a fall becomes a zombie, one killed by a teammate does not; a zombie respawns a zombie, and a player who joins late is one. The zombies win when no human is left, the humans when the round's time runs out (5 minutes). Each player scores a kill of the other side. The rules follow pull request #420's (`game/infection.c`). |
| Gun Game | Free for all Slayer. Everyone starts with the first weapon of a list (the rocket launcher, the shotgun, the sniper rifle, the plasma rifle, the assault rifle, the needler, the plasma pistol, the pistol, of those the map has); each kill gives the killer the next, and a kill with the last wins. No one picks up a weapon, a weapon emptied is refilled, and a melee kill takes the victim back a weapon (`game/gun_game.c`). |
| Hardcore Slayer, Hardcore CTF, Hardcore Oddball, Hardcore King | The team gametypes with Hardcore's rules: no motion sensor (the flag, the ball and the hill keep their nav points), no aim assist (no autoaim nor magnetism, for controllers and the mouse alike), friendly fire on, and a kill restores the killer's health, and their shields unless an overshield has them past full. |

Edit Gametypes edits them as any gametype, and its OPENCE OPTIONS screen
(after TEAMPLAY OPTIONS) gives any gametype these rules: its MODE (NONE,
SWAT, INFECTION or GUN GAME: Slayer only; Infection is played on teams and
Gun Game free for all, which the screen's OK sets), HARDCORE (any game
type), and the mode's own options: Infection's first zombies, zombie speed,
skull damage, zombie respawn time, countdown and zombie shields, and Gun
Game's weapon order (strongest first, weakest first, or random, drawn by
the host for each game) and whether a melee kill demotes. A saved gametype
keeps them with its PC options (`saved games/playlist_profile.c`, version 2
of their block; a gametype saved before them has none), and the network
game carries them to every machine (network version 35, `NETCODE.md`).
Only the host decides each mode's events (Infection's zombies, Gun Game's
weapons, Hardcore's restores); the clients take them from it as they take
the rest of the game. The lobby and Server Setup name the gametype's mode.
`debug.network_test` hosts them by name: `swat`, `infection`, `gun_game`,
`hardcore`, `hardcore_ctf`, `hardcore_oddball` and `hardcore_king`
(`host:bloodgulch:infection`).

## Settings

The settings are in `config.toml` next to the executable
(`build/linux/config.toml`). At the first start, the game writes the file
with the default values and a comment for each setting. To get the default
values again, delete the file.

The game reads the file at start-up. If a key is not correct, or a value
has the wrong type, the game writes the line to the log and uses the default
value. A key in the wrong section is named with the section it goes under. A
file that cannot be parsed leaves every setting at its default; a word value
needs its quotes (`menus = "pc"`, not `menus = pc`). The Settings menu (Video, Mouse, Audio, Network and Controls Setup)
changes the useful settings, writes them into the file (only their lines
change) and applies them at once, but `audio.enabled`, and `display.menus`
from the next main menu.

Each setting has an environment variable. The environment variable changes
the setting for one start of the game. It has priority over the file.

| Setting | Default | Environment variable | Function |
| --- | --- | --- | --- |
| `display.mode` | `""` | `HALO_DISPLAY_MODE` | `"fullscreen"`: the display, taken at the mode of `display.resolution` (the nearest the display has), or at its desktop mode. `"borderless"`: a window over the whole desktop, whose mode does not change. `"windowed"`: a window of `display.window_size`. Empty: `display.fullscreen` decides (`true`: borderless). F11 changes between the window and the fullscreen mode. Video Setup sets it. |
| `display.fullscreen` | `true` | `HALO_FULLSCREEN` | `true`: borderless, as `display.mode = "borderless"`. `false`: a window, as `display.mode = "windowed"`. Used when `display.mode` is empty. |
| `display.resolution` | `"native"` | `HALO_RESOLUTION` | What fullscreen and borderless draw at: `"native"`, the display's own resolution, or `"<width>x<height>"`, such as `"1920x1080"`, 640x480 or more. Fullscreen sets the display to it. Borderless draws at it and scales the picture to the display, where the display has room for it. The picture has 480 lines of the game and the width of the resolution's shape. A window draws at its own size instead. Video Setup's Resolution sets it, from the display's modes; it shows with Fullscreen and Borderless. |
| `display.resolution_scaling` | `"native"` (Android: `"original"`) | `HALO_RESOLUTION_SCALING` | `"native"`: the game draws at the resolution of the window, or of the display (or `display.resolution`) fullscreen. `"75%"`, `"50%"` or `"25%"`: the game draws at that part of it each way (`"50%"` of 1920x1080 is 960x540) and scales the picture up, for a GPU that cannot keep up with the full resolution. `"original"`: the game draws the 640x480 picture of the Xbox (on Android, 480 lines at the display's shape) and scales it up to the window or the display, whatever `display.mode` is. Video Setup sets it. |
| `display.window_size` | `""` | `HALO_WINDOW_SIZE` | The size of the window, as `"<width>x<height>"`, such as `"1920x1080"`, 640x480 or more. You can change the size of the window; the game's picture takes its shape. Empty: `display.window_scale` decides. Video Setup's Window Size sets it, from sizes of each shape (4:3, 16:10, 16:9 and 21:9) that fit the desktop; it shows with Windowed. |
| `display.window_scale` | `2` | `HALO_WINDOW_SCALE` | Used when `display.window_size` is empty: the size of the window, as a multiple of 640x480. |
| `display.vsync` | `true` | `HALO_NO_VSYNC=1` sets `false` | `true`: each frame waits for the display. |
| `display.max_fps` | `0` | `HALO_MAX_FPS` | With vsync off, the most frames each second. `0`: twice the display's refresh rate. `-1`: no limit, which can hang some Intel graphics (Raptor Lake), resetting the desktop's graphics too. |
| `display.frames_in_flight` | `3` | `HALO_FRAMES_IN_FLIGHT` | Windows and Linux: the most frames the GPU has queued, 1 to 8. Each frame waits until the GPU has finished the frame this many before it, so that the frames come evenly when the GPU is slower than the game. `0`: the driver decides, which on a slow GPU can give a long frame every few frames. Refer to "Frame rate". |
| `display.anti_aliasing` | `"off"` | `HALO_ANTI_ALIASING` | The smoothing of jagged edges, which the Xbox did not have. `"off"`: none, as on the Xbox. `"fxaa"` or `"smaa"`: a pass over the 3D view after the game draws it. The HUD and the menus stay sharp. SMAA is the sharper and costs more. `"ssaa2x"`: the game draws at two times the resolution in each direction (at most the GPU's largest texture), and the picture is scaled down. The GPU does four times the work. Not with `display.resolution_scaling = "original"`. `"msaa2x"`, `"msaa4x"` or `"msaa8x"`: each pixel of the 3D view has that many samples (at most the GPU's). On Android, `"smaa"` gives FXAA and `"ssaa2x"` none. A change applies from the next frame. Refer to "Anti-aliasing" in "What operates". |
| `debug.gpu_flush_draws` | `-1` | `HALO_GPU_FLUSH_DRAWS` | Flush the GPU's pipeline every this many draws. `-1`: every 3 on Intel graphics with Mesa's driver, which can otherwise hang in the game's long runs of small draws and reset the desktop's graphics too. `0`: never. |
| `display.interpolation` | `true` | `HALO_INTERPOLATION` | `true`: one frame for each refresh of the display. `false`: 30 frames each second, as on the Xbox. Refer to "Frame rate". |
| `display.menu_cursor` | `"large"` | `HALO_MENU_CURSOR` | Windows, Linux and the web: the menus' pointer for the mouse, the game's blue arrow, sized to the window as Halo PC's was: `"small"`, `"medium"`, `"large"` or `"extra_large"` (18, 24, 32 or 40 of the menus' 480 lines tall), or `"off"` for the system's pointer. MENU CURSOR in Settings > Mouse writes its choice here. |
| `display.show_fps` | `false` | `HALO_SHOW_FPS=1` sets `true` | `true`: the frame rate at the bottom right of a game, counted over half a second and green from 30 frames each second, red below. The game's own counter, which the Xbox version's `display_framerate` console variable showed. Refer to "Frame rate". |
| `display.direct_camera` | `true` | `HALO_DIRECT_CAMERA` | `true`: in first person, on foot, the view points where the player aims in each frame, not where the last tick left it. Refer to "Frame rate". |
| `display.fov` | `0.0` | `HALO_FOV` | The first-person view's field of view on foot, in degrees across at 16:9, from 20 to 150. `0`: the stock view. Refer to "Field of view". |
| `display.viewmodel_fov` | `0.0` | `HALO_VIEWMODEL_FOV` | The first-person weapon's and hands' field of view, in degrees across at 16:9, from 20 to 150. `0`: the weapon's stock view, also when `display.fov` widens the world. Refer to "Field of view". |
| `display.viewmodel_visible` | `true` | `HALO_VIEWMODEL_VISIBLE` | `true`: the first-person weapon, hands and what is attached to them are drawn. `false`: they are not; firing, animation, sound and lights go on, and other players' models are drawn. |
| `display.passenger_third_person` | `false` | `HALO_PASSENGER_THIRD_PERSON` | Third-person view for the Warthog's passenger and the Scorpion's four passengers. Uses the gunner's camera on the Warthog and the driver's on the Scorpion, within the passenger's firing arc. Smooth entry and exit; weapon zoom is disabled while riding. The reticle follows the shot's centre ray. Local view only. In the campaign and co-op only: not in multiplayer, where seeing around the vehicle would be an advantage. |
| `display.viewmodel_shield` | `false` | `HALO_VIEWMODEL_SHIELD` | `true`: the energy shield's flare (its hits, its charging, its fall) is drawn on the first-person arms too, as on the body, with the unit's own shield shader, kept closer to the arms than to the body. The stock game draws it on the body only. |
| `display.first_person_legs` | `false` | `HALO_FIRST_PERSON_LEGS` | `true`: your own body is drawn from the waist down in first person, so you see your legs when you look down, moving as the body's animation moves them. Refer to "Field of view". |
| `display.shield_color` | `default` | `HALO_SHIELD_COLOR` | Your energy shield's flare as this machine draws it: `default` (the shield's own) or a multiplayer armor color (`white`, `black`, `red`, `blue`, `gray`, `yellow`, `green`, `pink`, `purple`, `cyan`, `cobalt`, `orange`, `teal`, `sage`, `brown`, `tan`, `maroon`, `salmon`), whatever the armor's, on the body and, with `display.viewmodel_shield`, on the first-person arms. The shield's own shader is drawn with its two colors replaced at their brightness; the flare is added to what is behind it, so `black` is drawn a faint grey. Only this machine sees it. |
| `display.high_res_hud` | `true` | `HALO_HIGH_RES_HUD` | `true`: the HUD (meters, counters, panels and their outlines, the motion sensor, reticles, waypoints, scopes) is drawn from the high-res assets in `port/assets/hud`, 8x the size of the maps' bitmaps. The bitmaps with English text keep the maps' own. `false`: the maps' own bitmaps. |
| `display.hud_button_style` | `classic` | `HALO_HUD_BUTTON_STYLE` | Manual high-res gameplay button style: `classic`, `gems`, `menu_gems` (Xbox Dark Gems), `xbox_modern`, `ps_classic`, `ps_gems`, `ps_dark_gems`, `ps_modern`, `switch_classic`, `switch_gems`, `switch_modern`. Existing Xbox artwork and saved tokens are unchanged. Applies to Xbox and Custom Edition prompts. |
| `input.auto_button_detection` | `true` | `HALO_AUTO_BUTTON_DETECTION` | Gamepads > Button Appearance: AUTO BUTTON DETECTION restricts Button Style to the menu player's assigned controller family. Off retains all manual choices; device detection still runs. Unknown/disconnected devices use the manual style. Artwork changes require High-Res HUD in Graphics. Existing saved choices are preserved. |
| `display.hud_bumper_style` | `original` | `HALO_HUD_BUMPER_STYLE` | Gamepads > Button Appearance: BUMPER STYLE offers `original` (Original Xbox black/white buttons) and `modern` (LB/RB), only for Xbox Classic, Xbox Gems and Xbox Dark Gems. Shared by local players and retained with automatic detection on. Requires High-Res HUD; other styles keep their own bumpers. |
| `input.controller_p1` | `""` | `HALO_CONTROLLER_P1` | P1 custom controller layout, including D-pad, communication, debug camera and unbound actions; empty uses the profile preset. Edit in Gamepad Setup > Button Remapping. See [format and behavior](../../docs/reading/controller_remapping.md). |
| `input.controller_p2` | `""` | `HALO_CONTROLLER_P2` | P2 custom controller layout; empty uses the profile preset. Edit in Gamepad Setup > Button Remapping. See [format and behavior](../../docs/reading/controller_remapping.md). |
| `input.controller_p3` | `""` | `HALO_CONTROLLER_P3` | P3 custom controller layout; empty uses the profile preset. Edit in Gamepad Setup > Button Remapping. See [format and behavior](../../docs/reading/controller_remapping.md). |
| `input.controller_p4` | `""` | `HALO_CONTROLLER_P4` | P4 custom controller layout; empty uses the profile preset. Edit in Gamepad Setup > Button Remapping. See [format and behavior](../../docs/reading/controller_remapping.md). |
| `display.hud_button_style_xbox` | `classic` | `HALO_HUD_BUTTON_STYLE_XBOX` | Xbox preference while automatic detection is on: `classic`, `gems`, `menu_gems`, `xbox_modern`. |
| `display.hud_button_style_ps` | `ps_classic` | `HALO_HUD_BUTTON_STYLE_PS` | PlayStation preference while automatic detection is on: `ps_classic`, `ps_gems`, `ps_dark_gems`, `ps_modern`. |
| `display.hud_button_style_switch` | `switch_classic` | `HALO_HUD_BUTTON_STYLE_SWITCH` | Switch preference while automatic detection is on: `switch_classic`, `switch_gems`, `switch_modern`. Preferences are per family; each split-screen viewport resolves its own assigned controller. See the [controller walkthrough](../../docs/reading/controller_buttons.md). |
| `display.high_res_text` | `true` | `HALO_HIGH_RES_TEXT` | `true`: the menus' and HUD's text is drawn with the fonts in `port/assets/fonts` (Overpass, in place of the maps' Interstate) at the resolution the game draws at, laid out as before; the menus' titles are drawn from the high-res pictures in `port/assets/titles`, and the controller button icons from those in `port/assets/buttons`. `false`: the maps' bitmap fonts, titles and button icons. |
| `display.shadow_resolution` | `128` | `HALO_SHADOW_RESOLUTION` | The size of the maps that the shadows of the objects are drawn in, in pixels each way: `128`, `256`, `512` or `1024` (other values go down to one of these). The game draws the shadow of each object into a map of 128x128 pixels, blurs it and projects it onto the ground. On a large screen, the edges of these shadows show steps that move when the object moves. A larger map makes the edges smooth; the blur is made wider to match, so the shadows are as soft as on the Xbox. Each doubling adds two passes of the blur. `128`: as on the Xbox. |
| `display.menus` | `"pc"` | `HALO_MENUS` | `"pc"`: the PC version's menus, from the files in `port/assets/menus` and a `menus` folder next to `config.toml`. Refer to "Menus". `"xbox"`: the Xbox's menus. |
| `display.menu_language` | `"maps"` | `HALO_MENU_LANGUAGE` | The PC menus' language. `"maps"`: the language of the Custom Edition whose `loc.map` is in `custom_maps` (English without one). `"english"`: English. The port's own lines are English. |
| `display.player_names` | `"all"` | `HALO_PLAYER_NAMES` | In multiplayer, whose names are drawn above their heads: `"all"`, `"allies"`, `"enemies"` or `"none"`. An ally's name is drawn above the triangle the game shows over teammates. An enemy's name shows only within the motion sensor's reach, while the enemy is in sight and not camouflaged, so it never shows where an enemy hides. The gametype's motion tracker setting also applies: no names if it shows no players, only allies' if it shows only friends. |
| `display.player_name_scale` | `1.0` | `HALO_PLAYER_NAME_SCALE` | How large the players' names are drawn: `1.0` is three quarters of the size of the HUD's text, from `0.25` to `4`. With high-res text, larger names are rasterized at their size, so they stay sharp. |
| `display.match_timer` | `false` | `HALO_MATCH_TIMER` | `true`: a multiplayer game draws its clock at the bottom right, counting up from the game's first tick, which the items' respawn periods count from too: a weapon is back on the map at every multiple of its period on the clock. Campaign and co-op have none. MATCH TIMER in Settings > Network Setup > Multiplayer HUD. |
| `display.text_chat` | `true` | `HALO_TEXT_CHAT` | Show the text chat of network games (Settings > Network Setup > Multiplayer HUD > TEXT CHAT). `false` hides its lines, and its keys and a controller's D-pad Up open nothing. Refer to "Text chat". |
| `display.chat_style` | `"colors"` | `HALO_CHAT_STYLE` | The text chat's colours (Settings > Network Setup > Multiplayer HUD > CHAT TEXT): `"colors"`, the player's name in their team's colour and the text a pale blue; `"white"`, all of it white, as Custom Edition. |
| `display.scoreboard_team_layout` | `"teams"` | `HALO_SCOREBOARD_TEAM_LAYOUT` | How the multiplayer scoreboard (hold BACK, or tab) lists a team game's players. `"teams"`: a column for each team, red on the left and blue on the right. `"score"`: all the players in order of score. With more players than fit, the mouse wheel and Page Up / Page Down scroll the scoreboard. |
| `display.scoreboard_background` | `true` | `HALO_SCOREBOARD_BACKGROUND` | `true`: the multiplayer scoreboard (hold BACK, or tab) has a panel behind its text, for clearer text. |
| `display.scoreboard_background_color` | `"16, 16, 16, 150"` | `HALO_SCOREBOARD_BACKGROUND_COLOR` | The colour of the scoreboard's panel: `"red, green, blue, alpha"`, each from `0` to `255`. Alpha `0` is see-through, `255` is solid. |
| `display.anisotropic_filtering` | `0` | `HALO_ANISOTROPIC_FILTERING` | Textures seen at a grazing angle (floors, walls, terrain) are sampled along the angle from finer mip levels, so they stay sharp; the Xbox game asks for none. `0` or `1`: as on the Xbox. `2`, `4`, `8` or `16`: at most that many samples (at most the GPU's); `4` keeps most of `16`'s sharpness at less cost on slower GPUs. TEXTURE FILTERING in Video Setup > Graphics. It applies to the textures of the map that have mip levels and are filtered linearly (base, detail, bump and environment maps; lightmaps have no mip levels), not to what the game renders, such as the water's ripples. On Android and the web, only where the GPU has `GL_EXT_texture_filter_anisotropic`. |
| `display.latency_meter` | `false` | `HALO_LATENCY_METER` | `true`: in a network game, your round trip to the host in milliseconds at the top right of the screen, "Ping 45 ms" with four bars: green below 80 ms, yellow below 150, red from 150. The host shows "Max ping", its slowest player's. A player's machine that has heard nothing from the host for two seconds shows "Connection problem". The round trip includes each machine's wait for its next tick, so a LAN reads 15 to 35 ms. Not shown in the menus, after the game, in a co-op cutscene or when the host plays alone. The scoreboard's pings take the same colours, and show the round trips this machine times itself (the host: each player's; a player: their own), so they agree with the meter. Nothing more is sent. Only in config.toml, not in the menus. |
| `display.shader_cache` | `true` | `HALO_SHADER_CACHE` | Windows and Linux: keep the GPU programs that the game links in `shader_cache/` in the save root, and load each from there the next time instead of linking it again, which stops the game for a frame on some drivers. `false`: link each program every time. Refer to "Files and folders". |
| `display.stats_log` | `false` | `HALO_STATS_LOG` | `true`: a line a second in the log with the frame rate, the mode, the map, the renderer and the size of the 3D view (`STATS fps=120.2 mode=Slayer map=bloodgulch gpu="Adreno (TM) 735" res=1068x480`), so a graphics bug's report or two builds compared say which of them was running and at what frame rate. Nothing is drawn and nothing is sent. |
| `display.per_pixel_lighting` | `false` | `HALO_PER_PIXEL_LIGHTING` | `false`: the models (characters, weapons, vehicles, scenery) are lit at each vertex and the light is blended between them, as on the Xbox. The light across a curved surface then shows facets, and a point light that passes close lights only the vertices it reaches. `true`: the models are lit at each pixel by the same lights (the ambient light, two distant lights and two point lights), which changes their look. |
| `display.structure_culling` | `31` | `HALO_STRUCTURE_CULLING` | Which tests skip the triangles of the level (its walls, floors and terrain) that the camera cannot see before they are drawn, as bits added together: `1`, triangles outside the view; `2`, groups of triangles that all face away from the camera; `4`, single triangles that face away; `8`, smaller groups, so that more of them are skipped whole; `16`, the rooms nearest the camera first. Each test is conservative: the picture is the same, with fewer triangles drawn. `0`: the original walk, which draws every triangle of each visible room. The console's `structure_culling <bits>` changes it until the game quits, and `debug_structure_culling true` shows what was skipped (red: outside the view, blue: facing away). |
| `display.shield_glow` | `false` | `HALO_SHIELD_GLOW` | `true`: as a unit's energy shield flares (hit, charging, falling), it lights what is around it and itself, as a plasma bolt does, in the shield shader's own color (a Spartan's gold, an Elite's blue), rising and falling with the flare seen on the body. An ordinary dynamic light of the game's, drawn with a light definition the map has (`shield_glow.c`). Up to 8 units near the camera. Video Setup's Extra Effects screen sets it. In a network game, the host's `network.shield_effects` decides, unless it is `players` (each player's own) or the game is co-op (always each player's own). |
| `display.shield_glow_intensity` | `default` | `HALO_SHIELD_GLOW_INTENSITY` | The shield glow's reach and brightness: `default`, or `light_show` (three times as far, and at full strength from a third of the flare: a light is no brighter than full). The units glowing are those near any local player's camera, so each of a split screen's views has the same. Video Setup's Extra Effects screen sets it. In a network game, the host's `network.shield_effects` decides, unless it is `players` (each player's own) or the game is co-op (always each player's own). |
| `display.overshield_effect` | `false` | `HALO_OVERSHIELD_EFFECT` | `true`: the overshield's flare rises up the body from the feet as it charges, with a soft edge (2 seconds, the charge's time from a full shield; 3 from a shield that was down; 2 from any other), then stays lit, whole, for as long as the overshield lasts, its energy moving at 30% of its speed, in its own color (the powerup's, 253, 78, 116), whatever shield color its player chose; the shield's own flares are not drawn over it, and its glow (`display.shield_glow`) rises and stays with it. With `display.viewmodel_shield` it comes up on the first-person arms as the rise reaches them. A hit on it plays what the projectile plays on an Elite's shield (its sound, sparks and light), when the map has that effect and it only shows and sounds. `false`: the overshield as the stock game draws it. Only this machine's drawing and sounds change (`overshield_effect.c`; the rise is a fade in the vertex shader, `d3d8_gl_set_height_fade`). Video Setup's Extra Effects screen sets it. In a network game, the host's `network.shield_effects` decides, unless it is `players` (each player's own) or the game is co-op (always each player's own). |
| `display.model_detail` | `"stock"` | `HALO_MODEL_DETAIL` | Which level of detail (of up to five) each model is drawn at. The game measures a model by its size in the lines of its 480-line screen, at any resolution, so at 1440 lines a model three times larger than the levels were made for is drawn as the Xbox drew it. `"stock"`: as on the Xbox. `"native"`: measured in the lines of the display (not supersampling's), so more detail is kept at a distance at high resolutions, and small far models that the Xbox left out are drawn. `"highest"`: measured as `"native"` for which models are drawn, and those drawn at their most detailed level. With either, a part that has no geometry at the level chosen draws its most detailed level below that has some, and a level made for a model's inside (one for a size over 1000 pixels, as the top level of a10's cryotubes) is drawn only while the camera is inside the model. Models whose map gives one level (the trees and rocks of b30) do not change. On Android, which draws 480 lines, `"native"` is `"stock"`. |
| `display.reversed_depth` | `true` | `HALO_REVERSED_DEPTH` | Not on Android or in a browser. `true`: the depth buffers hold 32-bit floats, reversed (1 at the near plane, 0 at the far), so that depth is about as precise far away as near. With the Xbox's 24 bits and its near plane of 0.0625 world units, surfaces a few millimetres apart (the light strips on Blood Gulch's base, 0.0016 to 0.013 world units in front of their walls, with no depth bias) are run together past some tens of world units and flicker through each other, which the Xbox's 480 lines hid. The renderer turns the game's depth around (its tests, clears, depth range and polygon offsets), and the vertex programs take the reversed depth from their projection's rows (w's row less z's), not from w - z, which would lose the precision at a distance. `false`: 24 bits, as the Xbox's. Read at start. |
| `display.hdr` | `false` | `HALO_HDR` | `true`: the back buffer that the 3D view is drawn into holds half floats (`GL_RGBA16F`) in place of 8 bits a channel, so that the light the game adds up (blended lights, glows, fog, particles) is kept past white in place of being clipped. Before the HUD and the menus, a tone mapping pass brings it back into the display's range (`display.hdr_tone_mapping`). The HUD and the menus do not change. On Android, only with a GPU that draws into half floats (OpenGL ES 3.2, `GL_EXT_color_buffer_half_float` or `GL_EXT_color_buffer_float`); otherwise off, with a line in the log. A change applies from the next frame. Video Setup's GRAPHICS screen sets it (HDR) and `display.hdr_output` (HDR DISPLAY). Refer to "HDR" in "What operates". |
| `display.hdr_exposure` | `1.0` | `HALO_HDR_EXPOSURE` | With `display.hdr`, the light of the 3D view is multiplied by this before the tone mapping, from `0.25` to `4.0`. |
| `display.hdr_tone_mapping` | `"gt7"` | `HALO_HDR_TONE_MAPPING` | With `display.hdr`, the curve that brings the light into the display's range. `"gt7"`: Polyphony Digital's Gran Turismo 7 tone mapping (`port/third_party/gt7_tone_mapping`, MIT), in Rec.2020 at the light's brightness in nits: each channel along its curve to the display's peak, blended with that curve's brightness at the colour's own hue (ICtCp), whose chroma fades out as the light nears the peak, so that the brightest light goes to white, as on film and as the Xbox's 8 bits clipped it, in place of staying saturated. Without an HDR display, as GT7's SDR (white at 250 nits). Video Setup's Extra Effects screen sets it (TONE CURVE), taking effect from the next frame. `"neutral"`: the picture is as without HDR up to 80% of white, and the light past it rolls off to white in place of being clipped. `"aces"`: the ACES filmic curve (Narkowicz's fit), with more contrast and darker shadows. `"reinhard"`: the extended Reinhard curve on luminance, softer and darker. |
| `display.hdr_output` | `false` | `HALO_HDR_OUTPUT` | Not on Android or in a browser. With `display.hdr`, `true`: the window asks for floating-point pixels (scRGB). On a display that has HDR on (on Windows, "Use HDR"), the tone mapping then goes up to the display's headroom over SDR white, and the light past white is shown brighter than white. Without an HDR display the picture is as with `false`; if the driver has no such window, the game makes an 8-bit one. `display.hdr_paper_white` and `display.hdr_peak_brightness` set how bright white and the brightest light are. Read at start. With NVIDIA's driver on Windows, the NVIDIA Control Panel's "Vulkan/OpenGL present method" must be "Prefer layered on DXGI Swapchain": otherwise a fullscreen or borderless window turns black, then oversaturated. Experimental. |
| `display.hdr_paper_white` | `0` | `HALO_HDR_PAPER_WHITE` | Not on Android or in a browser. With `display.hdr_output` on an HDR display, how bright white is in nits, 80 to 1000: the HUD, the menus and the 3D view's white (scRGB's 1.0 is 80 nits). `203` is HDR television's reference white for SDR content (ITU-R BT.2408). `0`: Windows' "SDR content brightness", as SDL reports it. Video Setup's GRAPHICS screen sets it (PAPER WHITE), and its HDR CALIBRATION screen by eye, beside a grey scale ("HDR" in "What operates"). |
| `display.hdr_peak_brightness` | `0` | `HALO_HDR_PEAK_BRIGHTNESS` | Not on Android or in a browser. With `display.hdr_output` on an HDR display, the brightest the display shows in nits, 250 to 10000, which `display.hdr_tone_mapping` fits the brightest light to. `0`: what Windows reports, which its HDR Calibration app sets. Video Setup's GRAPHICS screen sets it (PEAK BRIGHTNESS), and its HDR CALIBRATION screen by eye: raised until a ring at 10000 nits disappears into a square at the value ("HDR" in "What operates"). |
| `audio.enabled` | `true` | `HALO_NO_AUDIO=1` sets `false` | `false`: no audio device. The sound continues without output. |
| `audio.volume` | `1.0` | `HALO_VOLUME` | The master volume. |
| `audio.music_volume` | `1.0` | `HALO_MUSIC_VOLUME` | The music's volume, of the master volume. |
| `audio.effects_volume` | `1.0` | `HALO_EFFECTS_VOLUME` | The volume of the other sounds (effects and speech), of the master volume. |
| `audio.buffer_frames` | `512` | `HALO_AUDIO_BUFFER_FRAMES` | The audio device's buffer, in sample frames at 48 kHz, from `64` to `8192`. Larger rides out stalls that would cut the sound out; smaller has less delay (512 is 11 ms, 2048 is 43 ms). Not on Android or the web. |
| `audio.channels` | `"stereo"` | `HALO_AUDIO_CHANNELS` | `"stereo"`, or `"5.1"` for surround sound as with Dolby Digital on the Xbox. `"auto"`: 5.1 if the sound device has 6 or more channels. Linux and Windows only. |
| `audio.reverb` | `true` | `HALO_REVERB` | `true`: the sounds of the world reverberate as the place the player is in does: the sound environments of the maps (a corridor, a cave, a large hall, outdoors) set the reverberation, as the I3DL2 reverb of the Xbox did. A sound behind a wall or a door is muffled in it too. `false`: no reverberation (sounds behind a wall are still muffled). |
| `audio.resampler` | `"sinc"` | `HALO_AUDIO_RESAMPLER` | How the sounds are converted to the output's rate (48 kHz): `"sinc"` with a band-limited low pass, which keeps out the images above the sound's own rate; `"linear"` between neighbouring samples, which lets some of them through and sounds brighter for it. Audio Setup's SOUND PROCESSING screen sets it, with `audio.reverb`. |
| `audio.voice_chat` | `"push_to_talk"` | `HALO_VOICE_CHAT` | How you talk in voice chat: `"push_to_talk"` (while `controls.push_to_talk` is held; the microphone opens when you first press it), `"open_mic"` (when the microphone hears speech), or `"off"`. You hear the other players in every case. Refer to "Voice chat". |
| `audio.voice_volume` | `1.0` | `HALO_VOICE_VOLUME` | The volume of the voices of the other players, `0` to `2`. |
| `audio.output_device`, `audio.input_device` | `"default"` | `HALO_AUDIO_OUTPUT_DEVICE`, `HALO_AUDIO_INPUT_DEVICE` | The speakers and the microphone, by the name that Settings > Audio shows, or `"default"` for the device of the system. If the device is not found, the game uses the device of the system. Not on Android. |
| `input.mouse_sensitivity` | `1.0` | `HALO_MOUSE_SENSITIVITY` | The multiplier for the mouse aim. |
| `input.mouse_vertical_sensitivity` | `0.0` | `HALO_MOUSE_VERTICAL_SENSITIVITY` | The multiplier for the vertical mouse aim. `0`: the same as `input.mouse_sensitivity`. |
| `input.playstation_kernel_driver` | `false` | `HALO_PLAYSTATION_KERNEL_DRIVER` | Linux: `true` drives a PlayStation controller (DualSense, DualShock 4) through the kernel's driver of it (`hid-playstation` since Linux 5.12, `hid-sony`): its buttons, sticks, triggers and rumble, with no setup, but not its lightbar. `false`: SDL's own driver, which needs access to the controller's `/dev/hidraw` device, as Steam's udev rules give it (`steam-devices`, or Steam installed). Read at start-up. |
| `input.invert_mouse` | `false` | `HALO_MOUSE_INVERT=1` sets `true` | `true`: the vertical mouse aim is inverted. |
| `input.mouse_aim_assist` | `false` | `HALO_MOUSE_AIM_ASSIST` | `true`: the magnetism of the controller also operates for the mouse. `false`: when the mouse moved after the right stick, the view is not slowed or dragged by a target. The autoaim of the bullets operates in both cases. |
| `controls.<action>` | (the table in "Controls") | `HALO_KEY_<ACTION>` | The keys and mouse buttons of an action, up to two, separated by a comma: `move_forward`, `move_backward`, `strafe_left`, `strafe_right`, `jump`, `crouch`, `fire`, `throw_grenade`, `melee`, `reload`, `zoom`, `switch_weapon`, `switch_grenade`, `action`, `flashlight`, `scoreboard`, `pause`, `screenshot`, `push_to_talk`, `chat`, `team_chat`, `vote_yes`, `vote_no`. Keys by their names on a US keyboard (`"W"`, `"Space"`, `"Left Ctrl"`, `"F1"`): a key is the one in that place on any keyboard, which Controls Setup shows by its own label (the US name on Android). Buttons: `"Mouse Left"`, `"Mouse Right"`, `"Mouse Middle"`, `"Mouse 4"`, `"Mouse 5"`, `"Wheel"` (either way), `"Wheel Up"`, `"Wheel Down"`. |
| `game.downloaded_maps` | `""` | `HALO_DOWNLOADED_MAPS` | Maps to play as downloaded maps, besides the maps that map sharing downloaded (it names them in `downloaded_maps.txt` in `custom_maps`): their names (`bloodgulch`, `hugeass`), separated by commas, or `"*"` for all the maps. The scripts of a downloaded map cannot change the settings of the player or the games of other players. `debug.txt` names what the game refused. |
| `game.console_log` | `"none"` | `HALO_CONSOLE_LOG` | What the console shows on the screen. `"none"`: only the asserts that stop the game. `"important"`: bans, players that the host drops for cheating, the reasons that the game refuses a command, and the asserts that stop the game. `"all"`: all the lines. The output of a command always shows. `debug.txt` gets all the lines. |
| `game.language` | `""` | `HALO_LANGUAGE` | The language of the menus: `ja`, `de`, `fr`, `es` or `it`. Empty: English. |
| `game.enhanced_animations` | `true` | `HALO_ENHANCED_ANIMATIONS` | `true`: the player bipeds' grenade throws keep their legs moving, blended by speed and direction (crouched throws stay crouched, throws in the air use the jump's legs), Warthog and Scorpion riders stay seated to throw and let go of the grips to throw and reload, and a player turns with the aim while throwing, as while meleeing, and in the air, as while running (the original froze the body in a jump and kept the aim within its aiming screen about it, so a shot after turning in the air went between the jump's facing and the crosshair). `false`: the original animations, which freeze the legs during a throw and stand a rider up and the body in the air. The turning with the aim (throwing, in the air) moves where shots go, so in a network game every player turns so whatever this says; `false` turns it off only in a game of one machine. Only in config.toml, not in the menus. |
| `game.unlock_all_levels` | `false` | `HALO_UNLOCK_ALL_LEVELS` | `true`: a profile created afterward starts with all the campaign levels finished on all the difficulties. `false`: a new profile starts at The Pillar of Autumn, and each level opens when you finish the level before it, as on the Xbox. Existing profiles keep their progress. To start a profile over, type `profile_reset_campaign` in the console. |
| `bots.count` | `0` | `HALO_BOTS` | Computer players (bots) in a local (split screen) Slayer game: `0` to `15` join its lobby with the players, as far as the game has room. Not in system link or internet games. `0`: none, as in the stock game. Refer to "Bots". |
| `bots.skill` | `"normal"` | `HALO_BOT_SKILL` | How well the bots play: `"recruit"`, `"easy"`, `"normal"`, `"heroic"`, `"veteran"` or `"legendary"`: how soon they react, how far and how wide they see, how large their aim error is and how fast it settles, how fast they turn, and how they fight. `"easy"`, `"normal"`, `"heroic"` and `"legendary"` keep their numbers; `"recruit"` is below `"easy"`, and `"veteran"` halfway from `"heroic"` to `"legendary"`. Refer to "Bots". |
| `bots.teams` | `"even"` | `HALO_BOT_TEAMS` | In a team game: `"even"` puts the bots on both teams, evening them with the players; `"against"` puts them all on the team the players are not on. |
| `paths.data` | `""` | `HALO_DATA_ROOT` | The data root. Refer to "Start the game". |
| `paths.saves` | `""` | `HALO_SAVE_ROOT` | The save root. Refer to "Files and folders". |
| `paths.logs` | `""` | `HALO_LOG_ROOT` | The folder of `debug.txt`, `bans.txt` and `cheaters.txt`. Empty: the data root. With it, the data root can be read-only (a dedicated server's maps mounted so). |
| `paths.custom_maps` | `""` | `HALO_CUSTOM_MAPS_ROOT` | The `custom_maps` folder (Custom Edition maps, with their `bitmaps.map`, `sounds.map` and `loc.map`). Empty: the one in the data root. |
| `network.address` | `""` | `HALO_NET_ADDRESS` | The IPv4 address of this machine for system link. Refer to "Play on one computer". |
| `network.broadcast` | `""` | `HALO_NET_BROADCAST` | IPv4 addresses, with commas between them, that get the broadcasts of the game. Empty: 255.255.255.255. |
| `network.online` | `true` | `HALO_NET_ONLINE` | `true`: internet play. `false`: system link on the local network only. |
| `network.join_from_clipboard` | `true` | `HALO_NET_JOIN_FROM_CLIPBOARD` | `true`: when the game comes to the front, it joins the game of an invite link on the clipboard. |
| `network.tunnel_port` | `0` | `HALO_NET_TUNNEL_PORT` | The UDP port for internet play. `0`: the game selects a port. Refer to "Internet play". |
| `network.allow_upnp` | `true` | `HALO_NET_ALLOW_UPNP` | `true`: internet play can ask the router to forward its port (UPnP). `false`: the game does not ask. Refer to "Internet play". |
| `network.public_lobby` | `true` | `HALO_NET_PUBLIC_LOBBY` | `true`: the server browser. Public games are listed, and Join Game > Server Browser shows them. `false`: no games are listed or shown. Refer to "Server browser". |
| `network.host_public` | `true` | `HALO_NET_HOST_PUBLIC` | `true`: a new game of Create Game > Internet starts as PUBLIC. `false`: it starts as PRIVATE. LISTING in Server Setup changes it for each game. Refer to "Server browser". |
| `network.coop_friendly_fire` | `"on"` | `HALO_NET_COOP_FRIENDLY_FIRE` | Whether the players of an online co-op game hurt each other: `"off"`, `"on"`, `"shields_only"` or `"explosives_only"`. FRIENDLY FIRE in co-op's Server Setup > Co-op Options writes its choice here. Their AI allies they hurt as `network.coop_ally_friendly_fire` says. |
| `network.coop_ally_friendly_fire` | `true` | `HALO_NET_COOP_ALLY_FRIENDLY_FIRE` | Whether the players of an online co-op game hurt their AI allies (the Marines, Captain Keyes, the Sentinels while they are allies), as in the campaign. `false`: the players' weapons, grenades, melee and vehicles do them no harm, so that no player can kill the allies a level needs. The allies' and enemies' own damage is unchanged. ALLY FRIENDLY FIRE in co-op's Server Setup > Co-op Options writes its choice here; the host sends it with the game's settings. |
| `network.coop_own_friendly_fire` | `true` | `HALO_NET_COOP_OWN_FRIENDLY_FIRE` | Whether this machine's players hurt, and are hurt by, the other players of an online co-op game. Two players hurt each other only if both have it `true`, and then as the host's `network.coop_friendly_fire` says. FRIENDLY FIRE in that game's pause menu writes its choice here. A player away from the controls for over 70 seconds is hurt by every player, whatever either chose. |
| `network.coop_player_collisions` | `true` | `HALO_NET_COOP_PLAYER_COLLISIONS` | Whether the players of an online co-op game bump into each other. `false`: they walk through each other, so that one cannot block a doorway or stand on another; they still bump into the AI's characters. PLAYER COLLISIONS in co-op's Server Setup > Co-op Options writes its choice here. |
| `network.coop_enemies_mode` | `"per_player"` | `HALO_NET_COOP_ENEMIES_MODE` | Online co-op's extra enemies: `"none"`; `"per_player"`, each squad of enemies that a level places grows by `network.coop_enemies` for each player past the first; or `"multiplier"`, each squad is `network.coop_enemies_multiplier` times as large, for any number of players. The extra enemies stand around the squad's places, and those that a dropship has no seats for drop out of it after its passengers. EXTRA ENEMIES in co-op's Server Setup > Co-op Options writes its choice here. |
| `network.coop_enemies` | `50` | `HALO_NET_COOP_ENEMIES` | The extra enemies per player, a percentage from `25` to `200`: for each player past the first, each squad of enemies gets this much of itself more (`100`: as many again, so four players meet four times the squad), up to 8 times the squad however many players there are. PER PLAYER in co-op's Server Setup > Co-op Options writes its choice here. |
| `network.coop_pickups` | `"none"` | `HALO_NET_COOP_PICKUPS` | EXTRA PICKUPS in co-op Server Setup > Co-op Options. Modes: `"per_player"`, `"multiplier"`, `"none"`. Static mode uses the shared MULTIPLIER, capped at 8x. In per-player mode, the host scales scenario-placed campaign weapons, grenades, health kits, overshields and active camouflage by the shared PER PLAYER percentage, capped at 8 times their original count. Fractions are distributed over placements of the same item. Copies retain weapon ammunition and use clear nearby floor in the active BSP. No safe space or capacity means no extra pickup. Enemy drops, inventory and vehicles are excluded. Script-created supplies count surviving copies against their placement budget. Checkpoints retain pending counts and copies together, without multiplying again on restore. |
| `network.coop_vehicles` | `"none"` | `HALO_NET_COOP_VEHICLES` | EXTRA VEHICLES after EXTRA PICKUPS in Co-op Options. Modes: `"per_player"`, `"multiplier"`, `"none"`. Each authored driveable vehicle supplies floor((players - 1) * Per Player * 2 / 300) extra copies in per-player mode, or floor((Multiplier - 1) * 2 / 3) extras in static mode, one at a time after a player drives the previous one clear of the home by the sum of both bounding-sphere radii plus 15%. Vehicle growth has an exact 2/3 modifier before rounding down; the default 50% requires four players for the first copy. Pelican cargo uses its unloaded resting position. Only the host spawns; obstructed pads and exhausted object capacity wait. Copies have no script name or AI passengers. In-memory checkpoint reverts restore the home table and budgets together; loading a disk/core checkpoint without that table forgets its old homes. None by default. |
| `network.coop_enemies_multiplier` | `2` | `HALO_NET_COOP_ENEMIES_MULTIPLIER` | The shared static multiplier, `2` to `32`, for options set to STATIC MULTIPLIER. Enemy squads are this many times as large; pickups are capped at 8x; vehicles supply floor((Multiplier - 1) * 2 / 3) extra copies. MULTIPLIER in co-op's Server Setup > Co-op Options writes its choice here. |
| `network.voice_lobby` | `true` | `HALO_NET_VOICE_LOBBY` | When you host: `true`, all players hear all players in the lobby, before and after a game. |
| `network.voice_mode` | `"team_global_enemy_proximity"` | `HALO_NET_VOICE_MODE` | When you host: who hears whom during a game. `"off"`; `"team_proximity"` (teammates who are near); `"team_enemy_proximity"` (all players who are near); `"team_global"` (all teammates); `"team_global_enemy_proximity"` (all teammates, and enemies who are near). Refer to "Voice chat". |
| `network.voice_kbps` | `24` | `HALO_NET_VOICE_KBPS` | When you host: the voice quality in the lobby and in a game, in kilobits per second, `8` to `64`. |
| `network.voice_proximity` | `15.0` | `HALO_NET_VOICE_PROXIMITY` | When you host: the distance in world units (1 unit is approximately 3 metres) at which players are near, for voice chat. `5` to `100`. |
| `network.text_chat` | `"on"` | `HALO_NET_TEXT_CHAT` | When you host: the text chat. `"on"`, lines with links refused; `"links"`, links allowed; `"off"`, no chat (in either case). Always off in a game a player hosts that is listed in the server browser; a dedicated server's follows it, listed or not. TEXT CHAT in Server Setup > Teamplay Options (in co-op, Voice and Voting) writes its choice here. Refer to "Text chat". |
| `network.votekick` | `true` | `HALO_NET_VOTEKICK` | When you host: `true`, the players can vote to kick a player. Refer to "Security". `false`: no votes. VOTE KICK in Server Setup > Teamplay Options (in co-op, Voice and Voting) writes its choice here. |
| `network.votekick_threshold` | `"dynamic"` | `HALO_NET_VOTEKICK_THRESHOLD` | When you host: the yes votes that a vote (to kick, to end the game) needs, of the players who can vote. `"dynamic"`: scaled with the players, more than half up to 8 players, 40% (at least 5) up to 32, 30% (at least 13) above. `"static"`: `network.votekick_percent` of them, rounded up. At least 2 in both cases, and more yes than no. Only in config.toml (and a dedicated server's `sv_set`), not in the menus. |
| `network.votekick_percent` | `50` | `HALO_NET_VOTEKICK_PERCENT` | When you host, with `network.votekick_threshold = "static"`: the percentage of the players who can vote that must vote yes (`1` to `100`). For a vote to kick, never more than the players who can vote yes (the player of the vote cannot). |
| `network.vote_end_game` | `true` | `HALO_NET_VOTE_END_GAME` | When you host: `true`, the players can vote to end the game (`!endgame` in the chat; F1 votes yes, F2 no), counted as a vote to kick. The game ends with its scores, as `sv_end_game` ends it. `false`: no such votes. |
| `network.votekick_minutes` | `5` | `HALO_NET_VOTEKICK_MINUTES` | When you host: the minutes that a player must play on the server before the player can start a vote to kick (`0` to `60`). To vote, a player must play for 2 minutes, or for this time if it is less. |
| `network.votekick_ban_minutes` | `30` | `HALO_NET_VOTEKICK_BAN_MINUTES` | When you host: the minutes that a player who is kicked by a vote cannot join again (`1` to `1440`). |
| `network.shield_effects` | `stock` | `HALO_NET_SHIELD_EFFECTS` | Host: the shield effects every player of a network game sees, whatever their own settings, so that none sees what another does not: `stock`, `glow` (`display.shield_glow`), `light_show` (its Light Show intensity), `overshield` (`display.overshield_effect`), `glow_overshield` or `light_show_overshield`; or `players`, not synced: each player sees their own settings' (PLAYERS' OWN). Sent to each client when it is ready and when it changes (`network_shield_effects.c`); a client draws stock until it has it. An online co-op game is always each player's own, whatever the host chose: the players play together. SHIELD EFFECTS in Server Setup > Player Options writes its choice here. |
| `network.fire_origins` | `true` | `HALO_NET_FIRE_ORIGINS` | Host: whether a client's reported hit that the shooter's camera had no line of sight to may still count from where the client says its player's shots leave, when that is not the camera (distributed message 93: a VR build's weapon hand, which may be past a corner the camera is behind). The hand must be within 0.4 world units of the camera, no more than 0.05 behind the eye along the line to the target, 0.2 to its side and 0.1 above it, with the line from the eye to it clear; the host logs how many hits counted only that way. `false`: from the camera and vehicles' guns only (`network_damage.c`). |
| `network.block_mouse_aim_assist` | `false` | `HALO_NET_BLOCK_MOUSE_AIM_ASSIST` | Host: Player Options > Aim Assist, Allowed or Block M+KB. Block overrides optional mouse magnetism while mouse/keyboard aims, including on the host, without changing the saved preference. Controllers and touch retain their existing behavior; bullet autoaim is unchanged. |
| `network.map_share` | `true` | `HALO_NET_MAP_SHARE` | When you host: send a player who joins without the game's Custom Edition map the map, and the resource maps that it needs and that the player does not have. SHARE MAPS in Server Setup > Map Sharing writes its choice here. Refer to "Map sharing". |
| `network.map_share_upload` | `1024` | `HALO_NET_MAP_SHARE_UPLOAD` | When you host: the most kilobytes per second that the game sends of maps (`16` to `20480`, 20 MB/s). UPLOAD LIMIT in Server Setup > Map Sharing writes its choice here. |
| `network.map_seed` | `true` | `HALO_NET_MAP_SEED` | When you play in the game of another machine: help send its map to players who join without it. HELP SEND MAPS in Settings > Network Setup > Map Sharing. |
| `network.map_seed_upload` | `256` | `HALO_NET_MAP_SEED_UPLOAD` | When you play in the game of another machine: the most kilobytes per second that the game sends of maps to other players (`16` to `10240`, 10 MB/s). UPLOAD LIMIT in Settings > Network Setup > Map Sharing. |
| `network.map_download` | `true` | `HALO_NET_MAP_DOWNLOAD` | When you join a game on a Custom Edition map that you do not have: download it from the players of the game. `false`: leave the game and show which map is missing. DOWNLOAD MAPS in Settings > Network Setup > Map Sharing. |
| `network.coop_public` | `false` | `HALO_NET_COOP_PUBLIC` | `true`: an online co-op game (Create Game > Internet, a SINGLEPLAYER map) starts as PUBLIC. `false`: it starts as PRIVATE. LISTING in co-op's Server Setup writes its choice here. Refer to "Server browser". |
| `network.coop_team_wipe` | `"auto"` | `HALO_NET_COOP_TEAM_WIPE` | TEAM WIPE in Server Setup > Co-op Options: `"auto"` reverts the checkpoint in a PRIVATE game and only respawns the team in a PUBLIC game (a dedicated server's: `server.public` with internet play). `"revert"` or `"respawn"` overrides visibility; a password does not make a PUBLIC game private. Changed players/machine assignments fall back to position-only respawn with a host diagnostic. A missing checkpoint also uses position-only respawn. Scripted mission failures keep their existing revert behavior. |
| `network.brokers_file` | `"brokers.txt"` | `HALO_NET_BROKERS_FILE` | The file of the public MQTT brokers that let the machines of an invite find each other, and that carry the listings of the server browser: next to `config.toml`, unless a full path. One `host:port` on each line, up to 4; `#` starts a comment. |
| `network.stun_servers` | Google and Cloudflare | `HALO_NET_STUN` | The public STUN servers (`host:port`, with commas between them) that give the internet address of a machine. |
| `network.client_timeout_seconds` | `15` | `HALO_NET_CLIENT_TIMEOUT_SECONDS` | Hosting: the seconds the host waits for a machine it hears nothing from before it drops it (5 to 300; the stock game waits 15). The same seconds are how long each machine's reliable stream waits for its peer to read before it gives up, and how long a host in a game waits for its own network to come back (an address renewed, a wifi drop) before it ends the game; each machine reads its own setting. A longer wait keeps players whose connection stalls (a browser tab in the background), but a machine that is really gone leaves its players frozen for longer. |
| `network.public_address` | `""` | `HALO_NET_PUBLIC_ADDRESS` | Hosting: the internet address the joiners reach this machine at, `host[:port]` (the port forwarded to `network.tunnel_port`), for a server behind a NAT with one forwarded port. It is offered first, and its name is looked up again every 5 minutes. Empty: what STUN and UPnP find. |
| `network.listings` | `"both"` | `HALO_NET_LISTINGS` | What the server browser lists. `"both"`: the dedicated servers of the master servers first, then the games players host (from the brokers). `"masters"`: the master servers' only. `"brokers"`: the brokers' only. Refer to "Server browser". |
| `network.link_share` | `"when_needed"` | `HALO_NET_LINK_SHARE` | When the game shows a server the Discord account linked to it (Settings > Network Setup > Discord Account): `"when_needed"` (a server that asks, to let you in from a VPN), `"always"` or `"never"`. Refer to "Discord account". |
| `network.masters_file` | `"masters.txt"` | `HALO_NET_MASTERS_FILE` | The file of the master servers the server browser asks for dedicated servers, next to `config.toml` unless a full path: on each line an `https://` URL and the master's key (`ed25519:<hex>`), 4 at most. A file that names none (missing, or comments alone, as the release's) leaves the official master, built in (`https://master.opencommunityedition.com`). Refer to "Dedicated server". |
| `network.browser_source` | `"all"` | `HALO_NET_BROWSER_SOURCE` | The server browser's games. `"all"` (ALL GAMES): the dedicated servers first, then the games players host; `"dedicated"`: the dedicated servers of the master servers; `"players"` (P2P GAMES): the games players host. The tab at the top left of the server browser changes it. |
| `network.browser_show_empty`, `network.browser_show_full` | `true` | `HALO_NET_BROWSER_SHOW_EMPTY`, `HALO_NET_BROWSER_SHOW_FULL` | `false`: the server browser does not list the games with no player, the full games. The server browser's Filters screen sets them. |
| `network.browser_maps` | `"yes"` | `HALO_NET_BROWSER_MAPS` | `"yes"`: the server browser lists the games on maps this computer does not have. `"no"`: it does not. `"downloadable"`: only those of a Custom Edition map that the host shares. |
| `network.browser_gametype` | `"any"` | `HALO_NET_BROWSER_GAMETYPE` | The server browser lists the games of one kind only: `"any"`, `"ctf"`, `"slayer"`, `"oddball"`, `"king"` or `"race"` (the Filters screen). |
| `network.browser_sort` | `"servers"` | `HALO_NET_BROWSER_SORT` | The server browser's order. `"servers"`: dedicated servers first, then by players. `"players"`, `"name"`, `"map"`, `"type"` or `"locked"` (games with a password first). A `-` before one reverses it (`"-players"`). A column's title sorts by it, and again the other way. |
| `server.enabled` | `false` | `HALO_SERVER` | `true`: run as a dedicated server, as `halo --server` does. Refer to "Dedicated server". The `server.*` settings are written to `config.toml` only by a dedicated server. |
| `server.name` | `"OpenCE Server"` | `HALO_SERVER_NAME` | The game's name in the lobby and the server browser: 15 characters of printable ASCII. |
| `server.max_players` | `16` | `HALO_SERVER_MAX_PLAYERS` | The most players (2 to 127). |
| `server.min_players` | `1` | `HALO_SERVER_MIN_PLAYERS` | The players in the lobby before it counts down to a game (1 to 16). |
| `server.password` | `""` | `HALO_SERVER_PASSWORD` | The password the server browser asks for (an invite link joins without it). Empty: none. It is never logged. `HALO_SERVER_PASSWORD_FILE` names a file whose first line is it instead (a container's secret; pinned, as the variable would be). |
| `server.public` | `true` | `HALO_SERVER_PUBLIC` | `true`: the server is in everyone's server browser (on the master servers, with `network.online`). `false`: players join with its invite link, or on the LAN. |
| `server.rotation` | `"bloodgulch:ctf, sidewinder:team_slayer, prisoner:slayer"` | `HALO_SERVER_ROTATION` | The games, played one after another: `map[:gametype[+rule=value...]]`, separated by commas, semicolons or new lines, 64 at most (an entry 255 characters, the whole 4096); a line that begins with `#` is a comment. A map is a multiplayer map's name (`bloodgulch`), `custom_maps\<name>` or a level's path; a gametype is a built-in gametype's name (`slayer`, `team_slayer`, `ctf`...), an OpenCE gametype's (`swat`...), a gametype file's (`gametypes/<name>.toml`) or `file=<saved gametype>`. An entry that cannot be played is left out with a warning. A change of the rotation (a command, the web page, `sv_reload`) keeps its place: the next game is the entry after the one played last. Refer to `docs/dedicated_server.md`, "The rotation". |
| `server.rotation_fallback` | `false` | `HALO_SERVER_ROTATION_FALLBACK` | `true`: with no entry of `server.rotation` that can be played, play `bloodgulch:slayer`. `false`: stop (exit code 3). |
| `server.lobby_seconds` | `15` | `HALO_SERVER_LOBBY_SECONDS` | The seconds the lobby counts down once `server.min_players` are in (0 to 300; `0`: the game starts at once). A player's START NOW takes five seconds off it. |
| `server.postgame_seconds` | `12` | `HALO_SERVER_POSTGAME_SECONDS` | The seconds the scores show after a game (0 to 300). |
| `server.empty_seconds` | `30` | `HALO_SERVER_EMPTY_SECONDS` | The seconds a game with nobody in it goes on before it ends. `0`: it does not end. A game whose last player leaves ends at once, as the game ends it. |
| `server.idle_minutes` | `0` | `HALO_SERVER_IDLE_MINUTES` | The minutes a game goes on with no score before it ends. `0`: it does not end. |
| `server.team_start` | `"one"` | `HALO_SERVER_TEAM_START` | A team game. `"one"`: it starts with the players of one team (the next players to join balance the teams). `"both"`: it waits, saying nothing, for a player on each team. `"skip"`: while one player waits, a team game (not one `sv_map` asked for) gives way to the rotation's next entry without teams, if it has one; checked when the lobby opens and again when its countdown would begin. |
| `server.motd` | `""` | `HALO_SERVER_MOTD` | A notice each machine sees when it joins (127 characters). Empty: none. |
| `server.frame_rate` | `60` | `HALO_SERVER_FRAME_RATE` | The frames a second (30 to 120); the game's ticks stay at 30 a second. A server with no other machine runs 10 frames a second. |
| `server.console`, `server.console_exit_on_eof` | `true`, `false` | `HALO_SERVER_CONSOLE`, `HALO_SERVER_CONSOLE_EXIT_ON_EOF` | `true`: the server takes commands from the standard input; it stops when the standard input ends if `server.console_exit_on_eof` is `true` (a panel can close it), after running the last line, whether or not a line's end ended it. |
| `server.init_file` | `"init.txt"` | `HALO_SERVER_INIT_FILE` | The commands run once at the start, one on each line: in the data root unless a full path. Empty: none. |
| `server.web_port`, `server.web_bind` | `8080`, `"127.0.0.1"` | `HALO_SERVER_WEB_PORT`, `HALO_SERVER_WEB_BIND` | The TCP port and IPv4 address of the web page, its API and the remote console (RCON). `0`: none. With an address beyond loopback, each web password that is set must have 8 characters or more, else the server stops (exit code 3, as for a port it may not bind or an address the machine does not have; a port in use: exit code 4); put a reverse proxy with TLS in front of it. Each server on a machine needs its own port. Read at the start only. |
| `server.web_password` | `""` | `HALO_SERVER_WEB_PASSWORD` | The admin password of the web page (8 characters or more). Empty: the server makes one at the first start, shows it once and writes it here. |
| `server.web_owner_password` | `""` | `HALO_SERVER_WEB_OWNER_PASSWORD` | A password with the owner role, for a supervisor on the server's own machine (a fleet's agent, a panel): over the API and the RCON it does what the standard input does (`quit`, `sv_stop`, `sv_restart`, the owner's settings, the game's console commands, the players' addresses). It signs in from `127.0.0.0/8` alone, never through `server.web_trusted_proxy`; from another address it is refused (`403 forbidden`) and counted as a wrong password. 8 characters or more (else the server exits with code 3). Empty: none. `sv_set` does not change it (`config.toml` and `sv_reload`). |
| `server.web_moderator_password`, `server.web_viewer_password` | `""` | `HALO_SERVER_WEB_MODERATOR_PASSWORD`, `HALO_SERVER_WEB_VIEWER_PASSWORD` | Other passwords: the moderator role (kick, mute, say, the next map, bans of one day at most) and the viewer role (the status, the players and the log; no changes). 8 characters or more when the web port is bound beyond loopback (else the server exits with code 3; on loopback, a warning). Empty: none. `sv_set` changes the moderator password, not the viewer password (`config.toml` and `sv_reload`). Each web password's variable has a `_FILE` form (`HALO_SERVER_WEB_PASSWORD_FILE`, `HALO_SERVER_WEB_OWNER_PASSWORD_FILE`...): when the variable is not set, the file's first line is the password, pinned as the variable would be; read at the start. |
| `server.web_ui` | `true` | `HALO_SERVER_WEB_UI` | `false`: the web port serves the API and RCON, but not the admin page. |
| `server.web_trusted_proxy` | `""` | `HALO_SERVER_WEB_TRUSTED_PROXY` | The address of a reverse proxy in front of the web port: its `X-Forwarded-For` (the last entry, the one the proxy added, an IPv4 or IPv6 address with or without a port), `-Host` and `-Proto` headers are believed, and a sign-in through it with `X-Forwarded-Proto: https` gets a `Secure` cookie. Empty: none. |
| `server.metrics` | `true` | `HALO_SERVER_METRICS` | `true`: the web port serves `/metrics` (Prometheus) without a password. |
| `server.log_format` | `"text"` | `HALO_SERVER_LOG_FORMAT` | The standard output's lines: `"text"` or `"json"`. |
| `server.log_addresses` | `false` | `HALO_SERVER_LOG_ADDRESSES` | `true`: the players' internet addresses and hardware IDs are in the log, the events and the API for every role (the owner's console always has them). |
| `server.log_debug_echo` | `false` | `HALO_SERVER_LOG_DEBUG_ECHO` | `true`: the lines of `debug.txt` are also written to the standard error. |
| `server.status_file` | `""` | `HALO_SERVER_STATUS_FILE` | A file the status (as `/api/status` gives it) is written to every 2 seconds, for a panel without HTTP. Empty: none. |
| `server.auto_update` | `"install"` | `HALO_SERVER_AUTO_UPDATE` | `"check"`: every `server.update_check_minutes`, the server looks for a newer release and logs it (the Docker image's default: its image is updated instead). `"install"`: it also installs it at once (on a thread of its own, the server playing on meanwhile) and stops with exit code 5, to be started again: a release of the same network version once the server has been empty for 10 minutes; one of a new network version when the game ends (players are told "This server updates after this game"; in the lobby, after 30 seconds' notice), a game going on past `server.update_game_minutes` being ended with its scores. An install that fails is tried again at the next check. `"off"`: neither. Refer to `docs/dedicated_server.md`, "Updates". |
| `server.update_check_minutes` | `30` | `HALO_SERVER_UPDATE_CHECK_MINUTES` | The minutes between checks for a newer release (`5` to `1440`; the first two minutes after the start). |
| `server.update_game_minutes` | `30` | `HALO_SERVER_UPDATE_GAME_MINUTES` | After a new network version's install, the minutes a game may go on before it is ended for the update (`0`: until it ends; up to `240`). |
| `server.upnp` | `false` | `HALO_SERVER_UPNP` | `true`: ask the router (UPnP) to forward the tunnel's port (as `network.allow_upnp`, for a server at home). |
| `server.coop_difficulty` | `"normal"` | `HALO_SERVER_COOP_DIFFICULTY` | The difficulty of co-op entries of the rotation (`<level>:coop`): `"easy"`, `"normal"`, `"hard"` or `"impossible"`. |
| `server.list_on_brokers` | `false` | `HALO_SERVER_LIST_ON_BROKERS` | `true`: the server is also listed with the games players host (through the brokers), for server browsers without master servers. |
| `server.master_publish` | `true` | `HALO_SERVER_MASTER_PUBLISH` | `true`: a public server is published to the master servers, signed with the publisher key. |
| `server.masters_file` | `"masters.txt"` | `HALO_SERVER_MASTERS_FILE` | The master servers the server is published to, next to `config.toml` unless a full path: on each line an `https://` URL and the master's key (`ed25519:<hex>`). None named: the official master, built in. |
| `server.publisher_key_file` | `"publisher.key"` | `HALO_SERVER_PUBLISHER_KEY_FILE` | The operator's Ed25519 key for the master servers (`halo --server --keygen` makes it), next to `config.toml` unless a full path. In a container, `HALO_SERVER_PUBLISHER_KEY` can hold the key itself. |
| `server.instance` | `""` | `HALO_SERVER_INSTANCE` | A short name that tells this server from the operator's other servers on the master servers (letters, digits, `-` and `_`). Empty: made from `server.name`. |
| `server.region` | `""` | `HALO_SERVER_REGION` | A region the server browser shows (`eu`, `na-east`...): 16 characters. Empty: none. |
| `server.chat_filter` | `"off"` | `HALO_SERVER_CHAT_FILTER` | The words the server keeps out of its chat and its players' names (streamer mode's list): `"off"`; `"censor"`, a line's words shown as stars; `"block"`, a line with them refused (its sender told why) and a player whose name has them refused at the door. Links are `network.text_chat`'s. |
| `server.chat_filter_slurs`, `server.chat_filter_swearing`, `server.chat_filter_insults` | `"mild"`, `"off"`, `"off"` | `HALO_SERVER_CHAT_FILTER_SLURS`, `_SWEARING`, `_INSULTS` | With `server.chat_filter`, each group's words kept out: `"off"`, `"severe"` (the worst), `"strong"` or `"mild"` (all). |
| `server.vpn_policy` | `"link"` | `HALO_SERVER_VPN_POLICY` | Machines joining from the address of a VPN, a Tor exit or a datacenter (the master servers' netlist): `"link"`, refused unless the player shows a Discord account linked at a master server this server publishes to; `"block"`, refused; `"off"`, let in. Refer to `docs/dedicated_server.md`, "VPNs and linked Discord accounts". |
| `server.vpn_fail` | `"open"` | `HALO_SERVER_VPN_FAIL` | Until a netlist is held: `"open"` lets every machine in, `"closed"` refuses those from the internet as `server.vpn_policy` `"block"` would. |
| `server.vpn_list` | `""` | `HALO_SERVER_VPN_LIST` | A netlist file checked besides the master servers' (next to `config.toml` unless a full path): for a server that publishes nowhere, or a test. Read at the start and by `sv_reload`. |
| `server.vpn_check_lan` | `false` | `HALO_SERVER_VPN_CHECK_LAN` | Check the LAN's machines' addresses against the netlist too (tests). |
| `server.link_min_age_days` | `365` | `HALO_SERVER_LINK_MIN_AGE_DAYS` | A machine joining over a VPN with a linked Discord account is refused when the account is younger than this many days (0: any age). |
| `server.link_one_seat` | `true` | `HALO_SERVER_LINK_ONE_SEAT` | Refuse a second machine of a linked Discord account already in the game. |
| `server.max_machines_per_address` | `0` | `HALO_SERVER_MAX_MACHINES_PER_ADDRESS` | The machines of one address (an internet play peer's real one) that may play at once; one more is refused ("too many players from your address"). `0`: no limit. |
| `server.afk_kick_minutes` | `5` | `HALO_SERVER_AFK_KICK_MINUTES` | The minutes a player may be away from the controls in a game (no button, stick, trigger or turn of the view) before being kicked, told a minute before (`0`: never; up to `60`). The lobby, the postgame, cinematics and time dead do not count. |
| `server.afk_kick_when_full` | `false` | `HALO_SERVER_AFK_KICK_WHEN_FULL` | `true`: players away are kicked only while the game is full, after `server.afk_kick_full_minutes`. |
| `server.afk_kick_full_minutes` | `3` | `HALO_SERVER_AFK_KICK_FULL_MINUTES` | With `server.afk_kick_when_full`, the minutes away before a kick (`2` to `60`). |
| `server.coop_bsp_switch` | `"majority"` | `HALO_SERVER_COOP_BSP_SWITCH` | A co-op loading zone: `"majority"`, it switches the level's part once more than half of the living players who are not away stand in zones to it (the others are told how many more must cross); `"anyone"`, the first player in one. A mission script's switch is not held. |
| `server.coop_bsp_switch_cooldown_seconds` | `15` | `HALO_SERVER_COOP_BSP_SWITCH_COOLDOWN_SECONDS` | The seconds after a loading zone switched the level's part before another may (`0` to `300`). Each switch is told, logged and a `coop_bsp_switch` event. |
| `server.map_vote` | `false` | `HALO_SERVER_MAP_VOTE` | `true`: as the lobby opens after a game, the players vote for the next game among `server.map_vote_choices` entries of the rotation at random, on a screen of their lobby; the rotation goes on after the winner. |
| `server.map_vote_choices` | `4` | `HALO_SERVER_MAP_VOTE_CHOICES` | The entries a map vote offers (`2` to `8`). |
| `server.map_vote_seconds` | `20` | `HALO_SERVER_MAP_VOTE_SECONDS` | How long a map vote runs (`5` to `120`); it ends sooner once every player has voted. |
| `server.master_publish_address` | `false` | `HALO_SERVER_MASTER_PUBLISH_ADDRESS` | `true`: the master servers can give this server's address to players. `false`: only their probe uses it. |
| `discord.application_id` | the application of the project | `HALO_DISCORD_APPLICATION` | The Discord application for invites. Empty: no Discord. |
| `update.auto` | `true` | `HALO_UPDATE_AUTO` | `true`: at start-up, the game looks for a new version. Refer to "Updates". `false`: the game does not look. |
| `streamer.enabled` | `false` | `HALO_STREAMER_MODE` | `true`: streamer mode hides offensive words, as stars, in the names and text of other players, maps and mods, at the levels below. Refer to "Streamer mode". STREAMER MODE in Settings > Network Setup > Streamer Mode. |
| `streamer.slurs` | `"mild"` | `HALO_STREAMER_SLURS` | With `streamer.enabled`, the slurs (race, ethnicity, sexuality, gender, disability) hidden: `"off"`, `"severe"`, `"strong"` (strong and severe) or `"mild"` (all). CENSOR SLURS in Settings > Network Setup > Streamer Mode. |
| `streamer.swearing` | `"strong"` | `HALO_STREAMER_SWEARING` | With `streamer.enabled`, the swearing and sexual words hidden: `"off"`, `"severe"`, `"strong"` or `"mild"`. CENSOR SWEARING in Settings > Network Setup > Streamer Mode. |
| `streamer.insults` | `"off"` | `HALO_STREAMER_INSULTS` | With `streamer.enabled`, the insults that are not slurs hidden: `"off"`, `"severe"`, `"strong"` or `"mild"`. CENSOR INSULTS in Settings > Network Setup > Streamer Mode. |
| `crash_reports.upload` | `"ask"` | `HALO_CRASH_REPORTS` | Windows only. `"yes"`: the game sends a report of each crash to the developers. `"no"`: the game sends no reports. `"ask"`: the game asks at the next crash and writes the answer here. Refer to "Crash reports" in [port/windows/README.md](../windows/README.md#crash-reports). |
| `debug.update_answer` | `""` | `HALO_UPDATE_ANSWER` | The answer to the update question, for automatic tests: `yes`, `no` or `never`. Empty: the game asks. |
| `debug.fixed_step` | `false` | `HALO_FIXED_STEP` | `true`: a game of this computer alone runs one tick a frame, whatever time has passed, so that it plays the same every time (tests). |
| `debug.exit_after` | `0.0` | `HALO_EXIT_AFTER` | The game stops after this number of seconds (from when its window opens; with `debug.null_renderer`, or no display, from its first frame; a dedicated server stops as `quit` stops it). `0`: never. |
| `debug.screenshot_directory`, `debug.screenshot_every` | `""`, `0` | `HALO_SCREENSHOT_DIR`, `HALO_SCREENSHOT_EVERY` | The game writes each Nth frame to this folder as a BMP file. |
| `debug.hidden_window`, `debug.null_renderer` | `false` | `HALO_HIDDEN_WINDOW`, `HALO_NULL_RENDERER` | `true`: no visible window, or no graphics. |
| `debug.gpu_stats`, `debug.gpu_trace_frame`, `debug.gpu_trace_constants`, `debug.gpu_dump_shaders`, `debug.texture_dump_directory`, `debug.texture_log`, `debug.gl_debug`, `debug.texture_no_cache` | off | `HALO_GPU_STATS`, `HALO_GPU_TRACE`, `HALO_GPU_TRACE_CONSTANTS`, `HALO_GPU_DUMP_SHADERS`, `HALO_TEXTURE_DUMP`, `HALO_TEXTURE_LOG`, `HALO_GL_DEBUG`, `HALO_TEXTURE_NO_CACHE` | Tools to find problems in the graphics: counts for each frame, all the GL state of one frame, the GLSL code, the textures. |
| `debug.menu_open` | `""` | `HALO_MENU_OPEN` | Start on this screen of the menus (`main_menu/settings_select/...`, as `port/assets/menus` names it), a player profile being edited, to look at it. |
| `debug.gpu_skip_vertex_shaders`, `debug.gpu_debug_expression`, `debug.gpu_debug_flat`, `debug.gpu_debug_texture0` | off | `HALO_GPU_SKIP_VS`, `HALO_GPU_DEBUG_EXPR`, `HALO_GPU_DEBUG_FLAT`, `HALO_GPU_DEBUG_T0` | Tools to find problems in the graphics: skip the draws of a vertex shader, or replace the output of all pixel shaders with a GLSL expression (for example `t0.rgb`). |
| `debug.network_test`, `debug.network_test_start`, `debug.network_test_kill`, `debug.network_test_score`, `debug.network_test_shoot`, `debug.network_test_vehicle`, `debug.network_test_pickup`, `debug.network_test_pickup_weapon`, `debug.network_test_chat`, `debug.network_test_name`, `debug.network_test_public`, `debug.test_input` | off | `HALO_NETWORK_TEST`, `HALO_NETWORK_TEST_START`, `HALO_NETWORK_TEST_KILL`, `HALO_NETWORK_TEST_SCORE`, `HALO_NETWORK_TEST_SHOOT`, `HALO_NETWORK_TEST_VEHICLE`, `HALO_NETWORK_TEST_PICKUP`, `HALO_NETWORK_TEST_PICKUP_WEAPON`, `HALO_NETWORK_TEST_CHAT`, `HALO_NETWORK_TEST_NAME`, `HALO_NETWORK_TEST_PUBLIC`, `HALO_TEST_INPUT` | Automatic tests of system link, or (`log`) a log of the game one machine plays (`game/network_test.c`). Refer to `NETCODE.md`. `debug.test_input = "menu:a down down a"` presses menu buttons one a second (`a`, `b`, `x`, `y`, `up`, `down`, `left`, `right`, `start`, `back` or `wait`), for testing the menus. |
| `debug.virtual_gamepads`, `debug.virtual_gamepad_script` | `0`, empty | `HALO_VIRTUAL_GAMEPADS`, `HALO_VIRTUAL_GAMEPAD_SCRIPT` | Desktop: attach this many SDL virtual gamepads (`0` to `4`), which the game takes for gamepads plugged in, and press their buttons from a script of `<seconds> <pad> <what>` entries separated by semicolons: the seconds from when the network lobby first opens, the pad `1` to `4` (`0`: the keyboard, whose `a` is SPACE and `start` Enter), and a button (`a`, `b`, `x`, `y`, `start`, `back`, `up`, `down`, `left`, `right`, `lb`, `rb`, `lt`, `rt`), `walk` (the sticks move from then on) or `still`. For testing split screen without four controllers, for example `"3 2 start; 6 2 a; 8 0 a; 10 1 a; 11 1 a; 14 0 start"` with `debug.network_test = "host:bloodgulch"`. |
| `debug.network_test_cues`, `debug.network_test_cue_players` | none, `1` | `HALO_NETWORK_TEST_CUES`, `HALO_NETWORK_TEST_CUE_PLAYERS` | Console lines a test runs at their seconds, from the first tick that many players are in (`"<second> <line>\|..."`; not on the web). |
| `debug.bot_trace` | `0` | `HALO_BOT_TRACE` | Every this many seconds, each bot's state is written to the log: where it is and where it goes, its path, its target and its weapon. `0`: never. |
| `debug.touch_targets` | `false` | `HALO_TOUCH_TARGETS` | Outlines the tap targets of the menus (item green, value blue, list slot yellow, legend button red, the band beside the slots of a list orange, keys of the on-screen keyboard white), marks where the last finger went down and the last tap landed for 3 seconds, and logs each tap with the target that it hit (for a value, also where it splits into previous and next): to judge the accuracy of touch. |
| `debug.log_addresses` | `false` | `HALO_LOG_ADDRESSES=1` sets `true` | `true`: other machines' internet addresses are written whole in `debug.txt` and the console, for debugging your own network. `false`: a public address is written as a tag (`addr#3f2a9c`) that matches up only within one run; local network addresses are always whole. Do not post a `debug.txt` written with this on. |
| `debug.network_latency`, `debug.network_loss`, `debug.network_corrupt`, `debug.network_corrupt_stream`, `debug.network_corrupt_after` | `0` | `HALO_NETWORK_LATENCY`, `HALO_NETWORK_LOSS`, `HALO_NETWORK_CORRUPT`, `HALO_NETWORK_CORRUPT_STREAM`, `HALO_NETWORK_CORRUPT_AFTER` | The game holds all the data that it receives for this number of milliseconds, ignores this percentage of the datagrams, and damages this percentage of the datagrams it receives, and this percentage of its reads of streams, at random (bytes changed, cut short, stretched or replaced), from this many seconds after the start. Use the first two to test the netcode as on the internet, and the others to test that nothing another machine sends can crash the game (a damaged stream is closed, so a little goes a long way; a host's messages to its own client are damaged too, so start damaging once the game has started). |
| `debug.voice_test` | `false` | `HALO_VOICE_TEST` | Automatic tests of voice chat: a tone replaces the microphone, and each voice that the game hears is written to the log once each second. |
| `debug.telnet_console`, `debug.telnet_console_port` | `false`, `2323` | `HALO_TELNET_CONSOLE`, `HALO_TELNET_CONSOLE_PORT` | The game listens on 127.0.0.1, on this port, for a script console (connect with telnet). The console has no password, so only this computer can reach it. |
| `debug.profile_record`, `debug.profile_record_when`, `debug.profile_memory` | `false`, `"start"`, `256` | `HALO_PROFILE_RECORD`, `HALO_PROFILE_RECORD_WHEN`, `HALO_PROFILE_MEMORY` | Profiling builds only (`configure.py --profile`). Record without a command; `"start"` records from the first frame until the first map change, `"game"` records each game outside the main menu. Memory is 4–1024 MB. Numbered `profile_<stamp>_<role>.part<n>.json` files remain in `profiles/` after recording. Refer to "Profiling builds" in the main README. |


With Mesa drivers, the game sends its GL calls through the GL thread of
Mesa. To stop this, set the environment variable `mesa_glthread=false`.

## Updates

The builds from GitHub Actions (refer to the main [README](../../README.md#download))
can update themselves. At start-up, the game asks GitHub for the latest
release. The game does not wait for the answer. If the latest release is not
newer, the game does nothing.

If the latest release is newer, the game asks: "Do you want to update?"

- Select "Yes" to update. The game downloads the release for this platform,
  replaces its files and starts the new version. The old files get the
  extension `.old`. The new version deletes them.
- Select "No" to continue. The game asks again at the next start.
- Select "Do not ask again", then "Yes", to stop the questions. The game
  writes `auto = false` in the `[update]` section of `config.toml`. To get
  the questions again, set `auto = true`.

Under gamescope (the Game Mode of the Steam Deck), the game does not ask,
because a system dialog stops the game there. The game writes the new
version in the log. To update, start the game from the desktop, or set
`update_answer = "yes"` in the `[debug]` section of `config.toml`.

The game downloads through HTTPS. It examines the certificate of the server
against the certificate authorities of the system: on Linux, the bundle of
the distribution (`src/posix_update.c`, with Mbed TLS); on Windows, the
certificate store of Windows (WinHTTP). The folder of the executable must
let the game write to it.

Builds that you make yourself have no build number. They do not look for
updates.

## Frame rate

The game calculates its world at 30 Hz, as on the Xbox. On the Xbox, the
game showed one frame for each calculation (tick). This port shows one frame
for each refresh of the display, for example at 60, 120 or 240 Hz.

Each frame shows the world between the last two ticks
(`game/render_interpolation.c`):

- After each tick, the game keeps the camera, the position of each part of
  each object, and the first-person weapon.
- Each frame mixes the last two ticks. The mix agrees with the time since
  the last tick.
- Rotations use quaternions. Positions and scales are linear.
- A teleport, a respawn or a cut of the camera does not mix. It jumps.
- After a long frame (several ticks in one frame), the camera mixes the
  last tick only, as the objects do.
- In cinematics, a camera that moves with an object (the lifepod in a30,
  a Pelican) moves with the object as it is drawn.

Thus the frames are one tick (33 ms) after the calculation. The calculation
does not change.

The direction of the view is an exception. The game reads the mouse and the
sticks in each frame. In first person, on foot, each frame points the view
where the player aims at that time (`display.direct_camera`). Thus the view
turns in the frame that the mouse moves. In a vehicle and in cinematics, the
view mixes as the other things do. On Android, the view mixes as before.

To get 30 frames each second, set `display.interpolation = false`.

When the GPU is slower than the game, the frames come at the GPU's pace,
evenly: each frame waits until the GPU has finished the frame three before
it (`display.frames_in_flight`). Without the wait, a driver queues frames up
to its own limit and then stops for several at once: on Intel graphics on
Windows, measured on a large Custom Edition map, a frame of 50 to 60 ms
every sixth frame, which looks like 15 frames each second where the mean
is 80. With the wait the same machine drew 108 to 142 frames each second,
with one frame of more than 33 ms in a minute instead of about 300.

To see the frame rate:

1. Push \` to open the developer console.
2. Enter `display_framerate true`.

Or set `display.show_fps` (Video Setup > Graphics' SHOW FRAMES, which on Android is
where the console is not at hand), which turns on the same counter.

In the game of another host, the console runs only the commands that change
nothing of the game (such as `display_framerate`), and the game puts back
cheats, the game speed and the settings of the drawing that show more of
the world (such as `rasterizer_wireframe`). Refer to `NETCODE.md`.

The frame rate shows at the bottom right of the screen. It is the mean over
half a second.

## Field of view

Video Setup's FOV AND VIEWMODELS screen sets the field of view of the
first-person view (`display.fov`) and of the weapon and hands
(`display.viewmodel_fov`), from 80 to 150 degrees in steps of 5, and can
hide the weapon (`display.viewmodel_visible`). DEFAULT, the setting's `0`,
keeps the stock view. `config.toml` takes any angle from 20 to 150.

- An angle is across the screen at 16:9. Another shape keeps the same
  angle up and down, as the stock view does.
- The field of view applies on foot in first person only. Vehicles, death,
  cinematics and scripted cameras keep their own view.
- A scope's zoom levels keep their stock view: the extra width fades out as
  the zoom comes in. A view narrower than the stock one narrows the zoom
  too.
- The reticles scale with the view, so that they stay on the aim. The
  scopes' pictures keep their place.
- By default the weapon keeps its stock view when the world is wider:
  arms and a gun right against the camera stretch at a wide angle.

LEGS (`display.first_person_legs`) draws your own body in first person
from the waist down, so you see your legs when you look down, walking,
running and crouching as everyone else sees them. It is the body others
see, posed as they see it: the upper body is folded away, and the body is
set back behind the eye as far as keeps the open waist below the view
(the way Halo 2 keeps its first-person body's open top out of sight), more
the further down you look. It is drawn with the world's camera, so it keeps
its place and size at any field of view. Hit boxes, markers and everything
the game does with the body are unchanged.

Only this machine's view changes. Nothing the machines send each other
changes, so players with different settings play together.

## System link

The Xbox game lets 16 players on 4 machines play a system link game. This
port lets up to 128 players on up to 128 machines play. Each machine can
have up to 4 players (split screen).

- `include/halo_port_limits.h` sets the limits.
- `include/halo_port_capacity.h` sets the memory for the limits. The game
  state is 16 MB at `0x81A00000` (3.3 MB on the Xbox). The pools of objects,
  effects, particles, contrails, lights and sounds are also larger.

Obey these rules:

- All the machines in a game must use a build with the same limits.
- The port uses protocol version 2. It does not see the Xbox game or older
  builds of the port. They do not see the port.

These are the differences from the Xbox:

- The host waits up to 60 seconds (15 seconds on the Xbox) for the other
  machines to load the map.
- If a machine does not read the messages of the host for two seconds, the
  host removes it from the game.
- The saved games contain all the game state. Thus a saved game is 16 MB.
  Saved games from older builds of the port do not operate.
- In campaign and in games of up to 16 players, the game removes garbage
  (bodies, dropped weapons) as on the Xbox. In larger games, it keeps more
  garbage, in proportion to the players.
- The lobby shows the local machine and the first three remote machines.
  The other machines are also in the game.
- In free-for-all games, each player is a team.

Linux, Windows and Android machines can play in the same game. Each machine
simulates the players from the same inputs, and the host does not correct
all of the game. Thus each machine must calculate the same floating-point
results, and all the ports:

- Compile without fused multiply-add (`-ffp-contract=off`).
- Use the math functions of musl (`port/include/halo_math.h`,
  `port/third_party/musl-math`), not the math functions of the system.

### Play on one computer

More than one copy of the game can play on one computer. Each copy must
have a different loopback address. A copy with an address gets no
broadcasts. Thus each copy must send its broadcasts to the other copies.

For a host and two clients, enter these commands in three terminals:

```sh
HALO_NET_ADDRESS=127.0.0.200 HALO_NET_BROADCAST=127.0.0.201,127.0.0.202 build/linux/halo
HALO_NET_ADDRESS=127.0.0.201 HALO_NET_BROADCAST=127.0.0.200 build/linux/halo
HALO_NET_ADDRESS=127.0.0.202 HALO_NET_BROADCAST=127.0.0.200 build/linux/halo
```

Do not give 127.0.0.1 to a copy. Each copy gets to its own address through
127.0.0.1. Linux and Windows send all of 127.0.0.0/8 to the loopback
interface.

### Test with many machines

`tools/system_link_bots.py` adds simple machines to a game. Each machine has
one player. The machines obey the system link protocol, but they do not
calculate the game or move their players.

1. Start a game on the host.
2. Enter `python tools/system_link_bots.py --host 127.0.0.200 --machines 127 --start`.

Each machine uses its own loopback address, from 127.0.0.2. The option
`--start` starts the game when all the machines are in the lobby. If the
host has no `network.address`, do not give `--host`.

## Internet play

Machines with an invite link can play system link on the internet. This
project has no server.

When a copy of the game starts to host a system link game, it makes an
invite link: `halo://join/<64 hexadecimal digits>`. The game writes the link
to the standard error and puts it on the clipboard. The links of older
versions of the game (44 digits) do not operate. The game writes a message
when it gets one.

To join a game, do one of these steps:

- Open the link. The game is the handler of `halo://` links. If the game
  already operates, the new copy gives the link to it and stops. A key in a
  file that only the user can read (`halo-ce-universal.key` in
  `$XDG_RUNTIME_DIR`, else `~/.halo-ce-universal.key`; on Windows in
  `%LOCALAPPDATA%`) encrypts the link, so the programs of other users cannot
  read it.
- Copy the link (or the 64 digits) and go to the game.
- Enter `halo <link>`.
- Accept a Discord invite. Refer to "Discord".

The game also takes the address of the web build's page with an invite
(`https://halocombatevolved.com/?join=<64 digits>`), as a link.

When the machines connect, the game of the host shows in Multiplayer,
System Link. Join the game as on a local network. System link on a local
network does not need an invite.

### Browsers

The web build ([port/web/README.md](../web/README.md)) plays internet games
with the Linux, Windows and Android builds: it hosts and joins by invite, and
the server browser lists its public games and theirs. A browser has no UDP
socket, only WebRTC, so a native build takes a browser's WebRTC on the port
of its tunnel (`network.tunnel_port`), as little of it as one data channel
needs (`src/p2p_webrtc.c`):

- ICE: the native build is an ICE-lite agent. It answers the checks of the
  browser, and the browser chooses the path. The credentials come from the
  secret of the session, which both machines have. Until the browser
  connects, the native build sends STUN indications to the addresses of the
  browser, which open its NAT to the checks of the browser.
- DTLS 1.2 (`src/posix_dtls.c`, Mbed TLS: `port/third_party/mbedtls`): the
  native build is the server, with a certificate that it makes when it
  starts (ECDSA P-256). Each machine sends the SHA-256 of its certificate in
  its signalling messages, which the token seals and the proof of the
  session authenticates; the other machine accepts only that certificate.
- SCTP, over DTLS: the browser opens the association, and one data channel,
  agreed before (unordered, without retransmissions), carries the packets of
  the tunnel. They are sealed as on UDP, and KCP makes the streams reliable
  as on UDP.

Two browsers connect to each other with their own WebRTC. There is no relay:
as between native builds, two networks that do not let a direct connection
through (some company and school networks block WebRTC) cannot play
together.

### Text chat

Network games have a text chat, as the PC version had. Press T
(`controls.chat`) to type a line to all the players, or Y
(`controls.team_chat`) to type one to your team (in a game without teams,
to all the players). While you type, Tab changes who the line is to, in
turn: all the players, your team, or your vehicle (the players riding in
the vehicle you ride in, whatever the map's vehicles are, Custom Edition
maps' too; on foot, only you see it). Enter sends the line, Escape cancels
it. While you type, your player does not move.

With a controller, D-pad Up opens an on-screen keyboard at the bottom right
(on Android, at the bottom in the middle) for a line to all the players:
the left stick (or D-pad Left and Right) moves between the keys, A types
the key, X deletes, Y switches capitals, D-pad Up and Down change who the
line is to (all the players, your team, your vehicle), Start sends the
line, and B closes the keyboard (the button that closed it does nothing in
the game until you let it go). Its last row has Aa (capitals), #+= (the
symbols), space, DEL and SEND. On a touchscreen (Android, and a browser on
a phone or tablet), the touch controls' D-pad Up opens it, a tap on a key
types it, and a tap anywhere else closes it: the touch controls hide while
it is up. While the keyboard is up the controller and the touch controls
move nothing in the game. The face buttons are the same places on
other controllers (on a PlayStation controller: Cross types, Square
deletes, Triangle switches capitals, Circle closes).

The lines show above the motion sensor, in a column a little over half the
screen wide: a long line wraps onto the next rows, and the line you type
shows its end when it is wider. The player's name is in the colour of
their team (blue in a game without teams), the text in a pale blue, and the
host's notices in a pale gold; WHITE in CHAT TEXT in Settings > Network
Setup > Multiplayer HUD (`display.chat_style`) draws them all white, as
Custom Edition does. A line to the
team says "(team)" after the name, one to the vehicle "(vehicle)". The
lines go after ten seconds; while you type, the last six rows show. Where
the touch controls are (Android, and a browser whose touch controls you
use), the lines go at the top left, the newest highest. In split screen,
each player's view shows the lines that player hears, and the first player
types. No line shows, and none can be typed, in a menu, in a cinematic or
while the HUD is hidden; opening a menu ends the line being typed. A new
game starts with no lines. A line has 63 characters at most, letters,
numbers and punctuation but `|` only (ASCII; `|` becomes `?`). `debug.txt`
gets every line.

Streamer mode (refer to "Streamer mode") hides offensive words in the
chat's lines and players' names as it does in all the text the game draws:
only on your screen, never in what is sent, and never in the line you are
typing.

The host sends each line on: a player of an older build does not see the
lines and cannot send them. The host takes 5 lines at once from a machine,
then one each 1.5 seconds; a machine that joins starts afresh.

TEXT CHAT in Settings > Network Setup > Multiplayer HUD (`display.text_chat`)
can HIDE the chat entirely, for purists and streamers: no lines show, and
its keys open nothing.

The host chooses the text chat for its games with TEXT CHAT in Server
Setup > Teamplay Options (in co-op, Voice and Voting; `network.text_chat`):
ON (the default: a line with a link, such as `discord.gg/...`,
`example.com`, `www.`, an address, or `example [dot] com`, is refused),
WITH LINKS, or OFF. A domain that is a word as well (`.me`, `.to`, `.is`)
counts only with a path after it (`youtu.be/...`), so that `go.to` and
`he.is` are not refused. A game a player hosts that is listed in
the server browser (a public internet game) never has the text chat,
whatever it says (TEXT CHAT shows OFF there, and cannot be changed, while
LISTING is PUBLIC); a private game, joined by its invite, and a LAN game have
it as chosen. A player who joins such a game from the server browser has no
text chat in it even if its host turns it on: their game takes no line from
that host and sends none. A dedicated server is the exception: its own
`network.text_chat` decides (on by default, and its operator can turn it
off, or allow links), listed or not, and a player who joins it from the
server browser's dedicated servers (a master server's list, which names
approved operators alone) has the chat as it says. Pressing T where the chat
is off says so, and a refused line tells its sender why.

On the scoreboard (right-click, then click the player), Mute voice and chat
mutes a player for you alone: you no longer see their lines or hear their
voice, and nobody else is told. The host's scoreboard also has Mute for
everyone: no machine sees the player's lines or hears their voice for 20
minutes, or until the host picks Unmute for everyone, and all the players
see "Milenko muted for 20 minutes". The mute holds in the lobby too. A
machine's voice is one: muting one of two players in split screen mutes
the other's voice too.

The host can also mute a player from the chat: type `/mute <player>
<minutes>`, as `/mute Milenko 5` (20 minutes when you give none, 1440 at
most), which mutes their voice too. The player is found as the console's `kick` finds one: the whole
name in either case, else the one player whose name begins with what you
typed. All the players see "Milenko muted for 5 minutes". The muted
player's lines go to no one, and they see how many minutes are left. The
mute ends when its minutes are up, or with `/unmute <player>`. A player
who leaves and joins again under the same name, in either case, is still
muted. Only the
host mutes: a player's `/mute` is not sent.

### Voice chat

Network games have voice chat, on the local network and on the internet.
Hold V (`controls.push_to_talk`, Settings > Controls Setup) to talk, or set
VOICE CHAT in Settings > Audio to OPEN MIC. VOICE VOLUME sets the volume of
the other players. On Android, set VOICE CHAT to OPEN MIC to talk: the game
then asks for the microphone.

OUTPUT DEVICE and INPUT DEVICE in Settings > Audio select the speakers and
the microphone (SYSTEM DEFAULT, the default, follows the system). The list
has the devices that the computer had when the game started.

The host sets voice chat for all players in Server Setup > Teamplay Options
(`network.voice_*` in `config.toml`):

- In the lobby, before and after a game, all players hear all players
  (LOBBY VOICE CHAT).
- During a game (VOICE CHAT): OFF; TEAM NEAR; ANYONE NEAR; TEAM; or TEAM,
  ENEMIES NEAR (the default). Near is VOICE NEAR DISTANCE (45 metres is the
  default), and both players must be alive. A player who is near sounds
  quieter when further, and from their direction. In a game without teams
  all players are enemies; in co-op, all are teammates.
- The quality (VOICE QUALITY), 8 to 64 kilobits per second (24 is the
  default), in the lobby and in the game.

In co-op, Server Setup has no Teamplay Options: the same settings are in
Server Setup > Voice and Voting.

A speaker shows next to the name of a player who talks: in the lobby, on
the scoreboard, in a list at the left of the screen during a game, and
beside the name above their head (when names are shown there:
`display.player_names`). To mute a player, open the scoreboard,
right-click, click the player, and click Mute voice and chat. You no longer
hear them or see their text chat lines, and nobody else is told. The host's
Mute for everyone silences a player's voice for every machine (refer to
"Text chat").

The host sends each voice on to the players who can hear it. The host
accepts a voice only from the machine of the player (with a key that it
gives each machine on its connection), only at the quality it set, at most
50 frames each second from a machine, and from at most 8 players at a time.
The voices are compressed with Opus (`port/third_party/opus`).

### Server browser

Server Setup in Create Game > Internet has a LISTING row:

- PUBLIC (the default): the game also shows in Join Game > Server Browser
  on every machine. Anyone can see and join the game.
- PRIVATE: only players with the invite link can join.

Each new game starts as PUBLIC (`network.host_public = false` makes new
games start as PRIVATE). An online co-op game starts as PRIVATE, and keeps
the last choice of its LISTING (`network.coop_public`). A LAN game is never listed. `network.public_lobby = false` turns the server browser off.

A PUBLIC game can also have a PASSWORD (a row of Server Setup, below
LISTING). The Server Browser shows a lock at the left of a game with a
password. A player who selects that game must type the password, and JOIN
GAME joins only with the correct password. The invite link of the game joins
it without the password. The host keeps the password only while the game
runs.

In the Server Browser, as in Custom Edition's, a click selects a game
(outlined in yellow), and a click on the selected game, a double click, A,
Enter or JOIN joins it. The game joins the invite of the game, as for a
link. When it reaches the host, it opens the lobby. If it cannot reach the
host in 30 seconds, it marks the game FAILED; when this machine's NAT gives
each destination its own port and no router forwards
`network.tunnel_port`, the line below the games says so (turn on UPnP on
the router, or forward the port: refer to "Connection").

The PING column shows the round trip to each host in milliseconds. The
browser times it as a join would reach the host: it makes the tunnel with
the listing's invite, keeps the shortest of three pings, and closes the
tunnel again. It times a few hosts at once, top first in the list as it
is shown (its sort, tab and Filters: a new sort orders the hosts timed
next, and the games hidden come last), each again every two minutes and
on REFRESH; a game being timed with no ping yet, or timed again on
REFRESH, shows moving dots, and the line below the games says
"Refreshing..." until REFRESH's pings are in (10 seconds at most).
Joining a host whose tunnel is open uses that tunnel. A game with a
password, whose invite the listing hides, shows no ping, nor does a host
not reached; a game that is full or starting shows CLOSED. REFRESH also
asks the hosts for their listings again.

The gametype column names an OpenCE gametype's kind (SWAT, INFECTION, GUN
GAME, or HC and the game engine for Hardcore: HC CTF, HC SLAYER), which the
host sends with the game (network version 37), and otherwise the game
engine's (SLAYER, CTF, KING OF THE HILL). The browser of LAN games, and the
preview of a game before joining it, name it the same way.

The browser lists two kinds of games: the dedicated servers that the
master servers list (`masters.txt`, `network.masters_file`; the official
master, `https://master.opencommunityedition.com`, built in, when the file
names none), and the games
that players host, which come through the brokers. `network.listings`
chooses the kinds that the browser asks for: both (the default), only the
master servers, or only the brokers.

- The top-left tab chooses the games that the list shows: ALL GAMES (at
  first), DEDICATED or P2P GAMES (left and right, `network.browser_source`).
  When `network.listings` is `"masters"`, the tab shows DEDICATED only;
  when it is `"brokers"`, P2P GAMES only.
  PLAYERS, the sum of the players in the games shown, is in the middle tab.
- The column titles sort the list (A or Enter on a title, or a click).
  The titles are: server name, map, game, players, the lock (the games
  with a password first) and the dedicated icon (dedicated servers first).
  A second choice of the same title reverses the order, and an arrow on
  the title shows the direction. The default order is dedicated servers
  first, then the games with the most players, then by name. The browser
  keeps the order in `network.browser_sort`.
- FILTERS opens the Filters screen of Custom Edition. It has SHOW EMPTY
  SERVERS, SHOW FULL SERVERS, GAME TYPE (ANY or one game engine) and SHOW
  MISSING MAPS (NO, YES, or IF DOWNLOADABLE, for a Custom Edition map that
  Map Sharing can download when you join). OK writes the
  `network.browser_*` settings. Settings > Network Setup > SERVER BROWSER
  writes the same settings, and also shows LISTS, SORT BY (DEDICATED FIRST,
  PLAYERS, NAME, MAP, GAME TYPE or LOCKED FIRST; an order that a title reversed
  shows as the same choice) and MASTER SERVERS
  (the number of master servers that answered the last request of the
  browser, for example `2 OF 2`). The PING and TEAMPLAY OPTIONS rows of
  Custom Edition are not on the Filters screen.
- A dedicated server's row shows the dedicated icon of Custom Edition.
  Instead of a ping, a dedicated server shows its region (EU, NA-E),
  because the browser does not ping the servers that the master servers
  list. The row shows JOINING while a join runs. A game of another network
  version stays in the list in grey, after the other games, with NEWER or
  OLDER. Its row does not join the game: it shows the message about the
  version. The server count above the list adds the number of dedicated
  servers (`SERVERS: 8 (5 dedicated)`) when the list shows all the games on
  one page. PAGE shows only when the list has more than one page.
- The two lines below the list describe the selected game. The first line
  shows its players, map, game and region. The second line shows where the
  game comes from: "Dedicated server by <publisher>, listed on N master
  servers" (the operator's name comes from the master server, and only this
  line shows it), "Hosted by a player (anonymous listing)", "Password
  required", or, for a game of another version, the update that it needs.
  During a join, the second line shows the steps of the join, and how many
  seconds it has taken. The steps are: the request to the brokers for the
  host, the answer of the host and the connection attempt (UDP hole
  punching), and the connection to the host. If the join fails, the line
  shows the reason:
  - the host did not answer;
  - no route to the host (the line says strict NAT when that is the cause);
  - the host refused the join: the game is full or closed, this machine is
    banned, the password is wrong, the version is different, or the invite
    changed.
  The browser shows a refusal when it opens again after the main menu
  error.
- If no master server answers but the brokers do, the second line shows "No
  master server answered: showing player-hosted games only", and the list
  shows the games from the brokers. An error panel covers the list in two
  cases: internet play is off, or nothing answers (no master server and no
  broker) after 15 seconds. OK, Enter or B closes the panel.
- In the lobby of a dedicated server, the panel at the right shows the
  name of the server and "dedicated" on its first line. It shows "Waiting
  for players" while the server waits for enough players
  (`server.min_players`) to start its countdown.

How it operates (`src/p2p_lobby.c`):

- The host of a public game publishes a listing: the invite, and the name,
  map, gametype and player counts of the game. The listing goes to the
  same MQTT brokers as the invites (`network.brokers_file`), retained,
  to a topic of the host (`hceu/3/lobby/s/<hash of its key>`).
- The key of the host signs the listing (Ed25519). The key is the key of the
  invite, so no other machine can list the invite of the host, change its
  listing, or list a game with false details.
- The host publishes the listing again every 30 seconds, when the game
  changes, and when a browser asks. If the topic of the host is empty or
  holds another listing, the host publishes again in 5 seconds or less. Thus
  a broker that deletes the listing does not remove the game.
- When the host stops (it stops hosting, the game becomes private, or the
  game quits), it publishes a closed listing, and then empties its topic.
  If the host loses its connection, the broker empties its topic (the will
  of the connection). A browser removes a game that it does not hear for 90
  seconds.
- When a public game becomes private, the host makes a new invite. Thus a
  player who saw the listing cannot join with the old invite.
- The listing of a game with a password does not hold the invite in clear
  text. The secret part of the invite (its token) is encrypted with a key
  from the password (Argon2id, salted with the key of the host, then
  XChaCha20-Poly1305). The browser makes the key from the password that the
  player types, and opens the invite only if the password is correct. When
  the host sets or changes the password, it makes a new invite. A player who
  has the listing can try passwords on their own machine without the host,
  so use a long password.

A public game does not publish the address of the host. But any machine
with the invite can ask the host to connect, and the host then sends its
addresses. Thus anyone can learn the address of the host of a public game,
as for any public server.

The brokers are in `brokers.txt` next to the executable (from
`port/assets/network/brokers.txt`; on Android, the app writes it next to
`config.toml` at each start), one `host:port` on each line. The game uses
all of them at once (up to 4), so one that works is enough. Without the
file (a game run from somewhere the builds did not put it), the game uses
its own copy of the list. An update replaces `brokers.txt`: to use brokers of your own, put them in another
file and name it in `network.brokers_file`. All the players must use the
same broker to see each other's games. The game uses
MQTT 5 if the broker has it, else MQTT 3.1.1. A broker that does not keep
retained messages, or does not let clients subscribe with wildcards, carries
only invites, not listings.

### Map sharing

When you join a game on a Custom Edition map that you do not have, the game
downloads the map from the players of the game, if the host shares its maps
(SHARE MAPS in Server Setup > Map Sharing, `network.map_share`, on by
default). This operates on a local network and
on the internet. It also downloads the resource maps that the map needs
(`bitmaps.map`, `sounds.map`, `loc.map`) if you do not have them. A line at
the top of the screen shows the progress. In the lobby, the game starts when
every player has the map. If the game already operates, you join it when
the download is complete. If no player sends any of the map for a minute,
you leave the game, and the main menu tells you why. If the downloaded map
cannot be played, the game deletes the files that it downloaded, and you
leave the game.

All the players help (HELP SEND MAPS in Settings > Network Setup > Map
Sharing, `network.map_seed`), as the peers of a torrent do:
the downloaded map comes in pieces from the host and from each player who
has it or has a part of it. The game checks each piece against its hash, so
a player cannot send a different map. A player who sends a bad piece is not
asked again. To keep the upload free for the game, the host sends at most
`network.map_share_upload` (1024 KB/s: UPLOAD LIMIT in Server Setup > Map
Sharing) and each other player at most `network.map_seed_upload` (256 KB/s:
UPLOAD LIMIT in Settings > Network Setup > Map Sharing). Set these lower on
a slow connection. DOWNLOAD MAPS there (`network.map_download`) turns
downloading off: you then leave a game whose map you do not have.

Maps keep their names. A game is on a map by its name (`hugeass`), and says
which version of it by the start of the map's hash, so a game always loads
the version of the host, whatever your copy is called. A downloaded map
takes its own name (`hugeass.map`); if you have another version of that
name, the download is named with the start of its hash
(`hugeass_0e2b3ee4.map`), and the menus show it as "Hugeass (2)". A map that
an earlier build renamed this way gets its name back, unless you have a map
of that name. A download that stops leaves a `.part` file in `custom_maps`
(only of the last map whose download stopped, and only if it holds part of
the map); the next download of that map continues from it. The host shares
maps in the folder of a Halo Custom Edition install (`paths.custom_edition`)
too; downloads go to `custom_maps`.

Over the internet, the players of a game connect only to the host. To send
each other a map, a player answers the other players of the game (who have
its invite) as the host does, and a player who downloads connects to them in
the same way that it connected to the host. Map sharing uses port 5153 for
its connections. Refer to `NETCODE.md`, "Map sharing".

### Security

Only machines with the invite can find the game:

- Each copy of the game makes an Ed25519 key pair when it starts, and from
  it an X25519 key pair (Monocypher, `port/third_party/monocypher`). Its
  identifier is from the hash of its X25519 public key. The Ed25519 key
  signs the listing of a public game.
- The link contains a 16-byte hash of the public key of the host and a
  random 16-byte token. The identifier of the host is from the first 6
  bytes of the hash.
- The machines exchange their public keys and addresses through public MQTT
  brokers (`network.brokers_file`). The topics are HMACs of the token.
  A key from the token encrypts and authenticates the messages
  (`src/p2p_signal.c`, `src/p2p_crypto.c`). The host authenticates its answer
  with a key that only it and the player can calculate. Its public key must
  agree with the hash in the link. The hash is long, so no other machine can
  find a key with the same hash.
- Then the player shows in the same way that it has the private key of its
  public key. Only then does the host make a session for the player. Thus
  other machines with the invite cannot make sessions in the name of a
  player (such a session would keep the player out).
- Each two machines get the keys of their packets from their key pairs and
  a random number from each. The keys do not go through the brokers. Thus
  other machines with the invite cannot read or change the packets.
- Each packet is encrypted and authenticated, with a different key in each
  direction. A machine ignores a packet that it already received. To and
  from a browser, the packets also go through DTLS, with the certificates
  that the signalling messages named.
- A machine can send only to the ports of the game on the other machine.
- The host makes one session from each request of a player. If a person
  sends a copy of an old request again, the host ignores it. A player that
  must ask again sends a new request.
- The host tries to reach at most 8 new players at the same time. The
  other players ask again.
- The host answers a request that is not proven at most one time each
  second through each broker. It answers at most 20 of these requests each
  second, after a first 32. Each answer goes only through the broker that
  brought the request. Thus a flood of requests does not use much of the
  bandwidth of the host.
- The host does the key work of at most 20 requests each second from keys
  that it does not know, after a first 32. It keeps the key work of the
  last 256 keys. Thus the proof of a player does not need more key work. A
  flood of requests can make players join more slowly. A player asks again
  for 90 seconds.
- The host refuses the predicted movement of a player whose game runs
  faster than time (a speed hack). If the messages on the player's
  connection were also ahead for ten seconds, the host drops and bans the
  player: each player sees who in red on the console, and the host adds a
  line to `cheaters.txt` and `bans.txt` (beside `debug.txt`) with the
  address and hardware id of the player, and the Discord name and id that
  the game of the player told it, marked `(self-reported)` (a player can
  change these). The host refuses a machine whose address or hardware id is
  in `bans.txt`. If only the player's datagrams were ahead, the host does
  not drop the player, because another machine can send datagrams with the
  player's address: it adds an `unverified` line to `cheaters.txt`.
- The host can ban a player with `ban <player name>` in the developer
  console (Tab completes the name). Remove a line from `bans.txt` to unban.
  Refer to `NETCODE.md`. `kick <player name>` drops the player the same
  way, but keeps nothing: no line in `bans.txt`, and the player can join
  again at once. In co-op, `bringto` brings every player to the host.
- Players can vote to kick a player. The host turns this on or off with
  VOTE KICK in Server Setup > Teamplay Options (`network.votekick`; in
  co-op, Server Setup > Voice and Voting). Hold the scoreboard key, right-click to
  show the pointer, and click the name of the player. Then click **Start
  a vote to kick**. While a vote runs, every player sees its box at the
  left of the screen: F1 votes yes, F2 no, or with a controller the D-pad's
  left and right (only while the vote runs: then they do nothing else; the
  box shows the keys of what you last used). `!yes` and `!no` in the chat do
  the same. Players can also vote to end the game: say `!endgame` in the
  chat (`network.vote_end_game`). `votekick <player name>` in the developer console does
  the same. The host also gets **Kick** and **Ban** in this menu: these do
  the same as the `kick` and `ban` commands (click **Ban** two times). The
  host counts the votes, and these rules prevent abuse:
  - The yes votes needed scale with the players: more than half of them up
    to 8 players, then 40% (at least 5) up to 32, then 30% (at least 13),
    and at least two; and there must be more yes than no. The host can set
    a fixed percentage instead (`network.votekick_threshold = "static"`,
    `network.votekick_percent`, 50 by default). The player of
    the vote is counted, but cannot vote. Thus, in a small game of two
    equal teams, one team cannot kick a player of the other team without
    help.
  - The host counts one vote for each internet address (for internet
    play, the real address of the player, not the address of the tunnel).
    Two machines at one address, or with one hardware id, have one vote.
  - To start a vote, a player must have played for
    `network.votekick_minutes` (5) on this server. To vote, a player must
    have played for 2 minutes (or less, if that setting is less). The host
    counts the time. When a player joins again, the time starts again.
    Players who cannot vote yet are not counted.
  - The host only accepts a vote that comes on the connection of the
    player, not a datagram, which another machine can send with the
    address of the player.
  - One vote runs at a time, for 45 seconds, with 30 seconds before the
    next vote. If a vote fails, the player who started it cannot start a
    vote for 5 minutes, and nobody can start a vote against the same player
    for 10 minutes.
  - Nobody can vote to kick a player of the host.
  - A player kicked by a vote cannot join again for
    `network.votekick_ban_minutes` (30), by address and hardware id.
  - A player who leaves during a vote against them is kept out the same
    way, as if the vote passed. While the vote runs, the host refuses the
    player if they try to join again. Thus nobody can avoid a vote by
    leaving.
  So that every player can be named, the host trims the spaces around a
  name and removes characters that draw as nothing. A
  letter with a mark is typed as the plain letter (`ban jose` for "José").
  A name with nothing left to type becomes "Player", and a name that another
  player already has gets a number ("Player 2"). The game refuses a profile
  name that is blank, and a multiplayer game refuses a profile whose name was
  made blank before this check.
- An invite operates while the copy of the game that made it operates.

### Connection

Each machine gets its public address from public STUN servers. Then the two
machines send packets to each other until the packets get through (UDP hole
punching). There is no relay.

Some networks give a different port for each destination (for example some
mobile and company networks). Two machines behind such networks cannot
connect. To connect, forward `network.tunnel_port` on the router of one of
the machines.

The game can ask the router to forward the port (UPnP,
`src/posix_upnp.c`, with `port/third_party/miniupnpc`):

- The host asks its router when a player uses its invite.
- A player that joins asks its router when it does not reach the host in
  5 seconds.
- The forwarded port is one more address that the machine gives to the
  other machine.
- The forward has a duration of one hour. The game makes it longer while
  it operates. When the game stops normally, it removes the forward. It
  does not remove the forward after a crash, or if a request to the router
  is still under way 3 seconds after the game starts to stop. Some routers only make forwards without a duration.
- When the game finds the router, it removes the forwards to this machine
  that have the description "Halo internet play" and that no copy of the
  game uses now (forwards that a copy of the game did not remove).
- UPnP does not help behind a second NAT, for example the NAT of a mobile
  network provider. Then the router has a private address, and the game
  does not ask.

To stop all UPnP requests, set `network.allow_upnp` to `false`.

In the game, each machine has an address in 100.64.0.0/10:

- `src/xnet.c` gives the datagrams that the bound UDP sockets of the game
  send to such an address to `src/p2p.c`. Other datagrams (of sockets that
  are not bound yet, or that are connected to the address) and the TCP
  connections of the game go through local sockets on 127.0.0.1 (or
  `network.address`). The traffic from the other machines comes to the
  game from local sockets too.
- `src/p2p.c` sends that traffic through one UDP socket. UDP datagrams go
  as they are. TCP connections go as KCP streams (`port/third_party/kcp`).
- The broadcasts of the game go to all the machines. Thus the game of the
  host shows on the other machines.

### Discord

If the Discord desktop client operates, the game of the host shows in
Discord (through the application of `discord.application_id`). The activity
has a private party with the invite as its join secret. The host can send
the invite with the invite button of Discord. When a person accepts it, that
person joins the game. If the game does not operate, Discord starts it.
The game sends the activity only to a Discord client of the same user.

### Discord account

Dedicated servers refuse by default players who join from the address of a
VPN, a Tor exit or a datacenter (`server.vpn_policy`), unless the player has
linked a Discord account at a master server the server publishes to.
Settings > Network Setup > Discord Account links one: a row for each master
server of `masters.txt` (up to 4) says whether the account is linked with
it; LINK: WITH MASTER n, then OK, opens that master's page in the browser
(`player_link.c`), which shows a code that the master's row shows too, and
logs in with Discord (its username and id alone). The game then holds a
pass that master signs, for 72 hours, and asks for a new one before it runs
out. A server that refuses the game for its VPN asks for it, and the game
joins again at once with the pass of the master it is linked with among
those that list the server (any it holds, in turn, for a game joined by an
invite): the server sees the account's username and id (SHOW TO SERVERS,
`network.link_share`, also offers ALWAYS and NEVER). A server listed by none
of the masters the game is linked with is refused for it with "link Discord
with its master": the screen marks that master (A SERVER NEEDS IT) and LINK
has it chosen. Masters share no links: linking with one master does not link
with another. UNLINK ALL deletes the key and its passes; each master's page
`/link/manage` deletes its record of them.

The game keeps its key in `player.key` and the passes in
`pass-<master key id>.bin`, next to `config.toml`. The key is this
machine's: whoever has it joins as the account. Refer to
`docs/master_protocol.md`, "Player keys, links and passes".

## Dedicated server

`halo --server` (or `server.enabled = true`) runs the game as a host with no
window, sound or player of its own, for a VPS, a container or a game panel.
It hosts the games of `server.rotation` one after another: the lobby counts
down when `server.min_players` are in, the game is played, its scores show,
and the lobby opens with the next entry. Players join it as they join any
host: on the LAN, with its invite link, or from the server browser.
`docs/dedicated_server.md` is the reference: the command line, the exit
codes, the commands, the HTTP API, the remote console, the events and the
files.

```sh
./halo --server                                  # the settings of config.toml
HALO_SERVER_ROTATION="bloodgulch:ctf, prisoner:slayer+score=25" ./halo --server
./halo --server --config /srv/opence/config.toml
```

The server needs the game data (`maps/`, as for the game); without
`maps/ui.map` it stops at once with exit code 2. At the first start it
writes the `server.*` settings to `config.toml` (they are written only by a
server) and a web password if there is none. It writes one line for each
event to the standard output (`server.log_format = "json"` for JSON
lines), and the game's own log to `debug.txt` as the game does
(`paths.logs` moves it, so that the data root can be read-only).

- **Commands.** The standard input, `init.txt` (`server.init_file`) and the
  web page take the same commands: `sv_status`, `sv_players`, `say`, `tell`,
  `kick`, `ban <player> 2h <reason>`, `unban`, `mute`, `sv_map`,
  `sv_map_next`, `sv_end_game`, `sv_rotation_set`, `sv_set <setting>
  <value>`, `sv_reload`, `sv_ban_address <address|hardware id>`,
  `sv_stop [now|game|empty] [notice]`, `sv_restart`, `quit`. `help` lists
  them. `sv_stop game` (or `empty`) withdraws the listings, refuses new
  players and stops once the game is over (or nobody plays), without
  cutting a game short; `sv_stop cancel` undoes it. The game's own console
  commands need the owner role: the standard input and `init.txt` take
  them, and the web page and the remote console with
  `server.web_owner_password` from the machine itself; with another
  password they do not (and `kick` and `ban` are always the server's).
- **Web page.** `server.web_port` (8080, on `127.0.0.1`) serves an admin
  page, a JSON API, a remote console over WebSocket, `/healthz` and
  `/metrics`. Sign in with `server.web_password` (the admin); the moderator
  and viewer passwords give fewer rights, and `server.web_owner_password`,
  from the machine itself alone, the owner's. To reach it from another
  computer, use an SSH tunnel, or a reverse proxy with TLS,
  `server.web_bind` and `server.web_trusted_proxy`; bound beyond loopback,
  every web password must have 8 characters or more.
- **Signals.** SIGTERM and SIGINT stop the server cleanly (a game in
  progress shows its scores, every machine is told, the listings are
  withdrawn); SIGHUP reads the settings and the rotation again.
- **Listing.** A public server (`server.public`) with `network.online` is
  published to the master servers of `masters.txt`, signed with the
  operator's key: `halo --server --keygen` makes `publisher.key` and prints
  the public key, which the operator gives to a master server's Discord
  bot: `/apply` in the master's Discord server (the master's `/apply` page
  links to it), with `halo --server --sign <nonce>`'s proof of key. Until a
  master approves it, the server is not listed
  there (its log says so), but players can join it with its invite link
  and on the LAN. A change the lists show (public or private, the
  players, the name, the map) is published within 5 seconds; made
  private, the server makes a new invite and takes itself off the
  masters' lists, as `network.public_lobby = false` does
  (`docs/dedicated_server.md`, "The listing"). `server.list_on_brokers`
  also lists it with the games players host. A player's public game is published to the masters too
  when `publisher.key` is next to `config.toml` (not as a dedicated
  server), and withdrawn when it ends. `docs/master_protocol.md` is the
  protocol.
- **Ports.** The tunnel's UDP port (`network.tunnel_port`, 2302 for a
  server) must be reachable from the internet; `network.public_address`
  names the address and forwarded port behind a NAT. The web port is TCP
  and best kept private.
- **Updates.** `server.auto_update = "install"` (the default) installs a
  new release as soon as a check finds it (every 30 minutes,
  `server.update_check_minutes`; on a thread of its own: the server goes on
  meanwhile), and stops with exit code 5, so that what runs it (systemd,
  Docker, a panel) starts the new build: once it has been empty for 10
  minutes, or, for a release of a new network version, when the game ends
  (a game is ended after `server.update_game_minutes`, 30). An install that
  fails is tried again at the next check.
- **Probe.** `halo --probe <invite>` prints the state of the invite's game
  (a server's name, map, gametype and players) as one line of JSON, for a
  list site, and exits: the game of the invite's host alone. It never
  starts a server, whatever the settings say, and writes nothing to
  `config.toml`.

`deploy/` has a systemd unit (`deploy/systemd`), a Docker image and compose
file (`deploy/docker`), a Pterodactyl and Pelican egg (`deploy/pterodactyl`),
a Helm chart (`deploy/helm/opence-server`), PufferPanel and MCSManager
templates (`deploy/pufferpanel`, `deploy/mcsmanager`), example gametype
files (`deploy/gametypes`, described in `docs/dedicated_server.md`,
"Gametype files": their rules have their full names, `team_vehicles`, not
the rotation's short names) and an example `init.txt`
(`deploy/init.txt.example`). The README of each of the first six folders
says how to use it. `deploy/release-key.pub` is the public key of the
releases' signed `SHA256SUMS` (`docs/dedicated_server.md`, "Releases").

**Fleets.** For many servers on many machines, beside the panels,
`opence-fleet`
([github.com/OpenCommunityEdition/opence-fleet](https://github.com/OpenCommunityEdition/opence-fleet))
is one control plane over them all: an agent on each machine runs its
servers and drives each through the API as the owner, from the machine
itself (`server.web_owner_password`), drains a server before an update
(`sv_stop game`, `POST /api/stop`) and bans an address on every server at
once (`POST /api/bans`); `docs/dedicated_server.md`, "Panels and fleets".

## Mods

A mod is a folder in `mods/` in the data root with a `mod.toml` and a
`tags` folder of Halo PC tag files, as the Halo Editing Kit and Invader write
them: sounds, bitmaps, string lists and models that play over the maps' tags
of the same names, on this machine only (a high-res audio or texture pack, a
translation, higher-detail models that keep the map's nodes, markers and
regions). Every mod in the folder is on, in alphabetical order moved as the
mods ask (`load_after`, `load_before`); the main menu's Mods lists them as a
resource pack list does, the mod at the top playing over those below it,
turns them off and moves them, and keeps that in `mods/load_order.toml`.
Mods that say they cannot be played together (`incompatible`) are asked
about at startup. The console's `mods_list` lists them, `mods_reload` reads
their files again and `mods_enable false` plays the maps' own tags. The game
checks every file before it plays it, and leaves out (with a line in
`debug.txt`) one it cannot trust or that does not fit the map's tag it
replaces. A mod changes nothing that the machines of a
network game agree on. Refer to [docs/mods.md](../../docs/mods.md).

## Streamer mode

Streamer mode (Settings > Network Setup > Streamer Mode, `streamer.enabled`)
hides offensive words as stars wherever the game draws text that other
people or files wrote: the text chat's lines, players' names over their
heads, in the chat, in the kill feed, on
the scoreboards and in the lobbies, the server browser's games, the host's
notices in the console, map and mod names, and the text of a map's or mod's
own string lists, as well as the message boxes about mods and maps. It
changes only what this machine shows: the names others see, and what the
machines send each other, stay as they are.

The words come from Surge AI's rated list of English profanities, reviewed
for the game (`port/assets/text_filter`), in three groups, each with a level
that hides that severity and worse:

| Group | Setting | Default | What it has |
| --- | --- | --- | --- |
| SLURS | `streamer.slurs` | `"mild"` (all) | Slurs about race, ethnicity, nationality, religion, sexuality, gender and disability |
| SWEARING | `streamer.swearing` | `"strong"` | Swearing and sexual words; the mildest (damn, hell) show at `"strong"` |
| INSULTS | `streamer.insults` | `"off"` | Name-calling that is not a slur |

A word is found however it is dressed: in capitals or with accents, in
look-alike letters (Cyrillic, Greek, full-width, small capitals), in leet
(`0` for o, `@` for a, `ph` for f), with its letters stretched ("fuuuck"),
with punctuation between them (`f.u.c.k`), spaced out (`f u c k`, `fu ck`),
and with characters the game does not draw (zero-width spaces, the menus'
`|` codes) between them. Each word may be found inside a name, at a word's
start, or only as a whole word, as fits it, and some words are never
hidden though a hidden one is inside them (Scunthorpe). `python3
tools/text_filter_terms.py --check "some text"` shows what would be hidden.

What it cannot catch: letters added into a word or its letters scrambled, new
spellings, a word split between two names or two lines, voice chat (mute a
player from the scoreboard, or set `audio.voice_volume` to 0), and pictures.
What you type is shown as you type it.

Your own words: `text_filter.txt` in the data root, with lines of `add
<word>` (hidden whenever streamer mode is on) and `allow <word>` (never
hidden), read when the game starts and when the settings change. The web
build's page draws its own list of mods, which is not filtered.

## Map checks

The game reads a map's tags straight into memory and uses them as its own
structures: every pointer, count, index and enum in them is the map's, and
the game writes values into tags as it runs. So before anything reads a
map's tags, the port checks every tag against a schema of its group
(`game/tag_schema_*.c`, read by `game/tag_validate.c`), and each structure
BSP as it loads:

- Every block and every piece of data must lie in the tags (or the BSP) and
  overlap no other. Otherwise the game refuses the map.
- A block with more elements than the game has room for is cut to the
  maximum. A tag reference that is not a tag of the right group becomes
  none. So do an index past its block and an enum past its values (or they
  become 0, where the game cannot take none). A string gets its terminator.
  Values that the game sets as it runs are reset.
- Checks that the schema cannot express run last: the BSPs' and the models'
  graphs, vertex and index buffers, and indices into other tags.

Each correction goes to `debug.txt`. The game's own maps need none.
`build/linux/map_validate [--strict] map.map...` runs the same checks on map
files without the game, and `tools/test_linux_port.py` runs it on the maps
in `assets/maps`. `map_validate --fuzz <runs> map.map` changes a few words
of the tags at random in each run. The checks must not crash or hang, and a
map that they let through must need no more corrections.

A map's scripts can call only the script functions that a map needs (the
allowlist in `hs/hs.c`). They cannot call the functions for files, the
saved state of the game, the console, debugging or cheats. A script that
calls one does not run. The developer console can call every function.

Halo Custom Edition maps get the same checks (those that need OpenSauce are
refused). Their own loader (`game/cache_file_formats.c`) reads them into
their tag cache at 0x40440000 and converts what Custom Edition lays out
differently, then the validator checks their tags and each of their BSPs as
it checks this build's maps, before the game converts their models, BSP
geometry and scripts. Put them with `bitmaps.map`, `sounds.map` and
`loc.map` in `custom_maps`, beside `maps`, or set `paths.custom_edition` to
a Custom Edition install; the map lists show them as CUSTOM SINGLEPLAYER and
CUSTOM MULTIPLAYER, played as campaign levels (alone, or as network co-op)
or as multiplayer maps by their scenario type, and `game.custom_edition =
false` refuses them. `map_validate` checks them too, with the resource maps
beside each map or in `--maps <folder>`. See
`docs/custom_edition_caches.md`.

Defensive checks stay in the game code too. An index into a tag block, the
tags or a tag's data that is out of range gets zeros (`tag_empty_data` in
`tag_files/tag_groups.c`), not other memory.

A map's name must be its file's: the cache file slots are found by the name
in the map's header, so a map file whose header names another map (a
renamed one) is refused, not copied again for ever. The `loading.tga` a map
pack may put in the maps folder is read only if it is an uncompressed 24-bit
picture of 320 by 240, the loading screen's texture.

A checkpoint (`savegame.bin`, in the profile's folder) and a core are
images of the game state's memory: with the data arrays' pointers to their
elements, the objects' memory pool's blocks and the references to them, and
the caches' procedures. Before one is taken, each of those is checked
against what the game made at startup (`game_state_image_accept` in
`saved games/game_state.c`): an image that does not match (a damaged or
crafted file) is refused, and the level starts over.

## What operates

| Area | Status |
| --- | --- |
| Game code | All 466 C files of the game. The changes are in "Game source changes". |
| Graphics | Direct3D 8 on OpenGL 4.5 core through SDL3 (`src/d3d8_gl.c`). The port translates the NV2A vertex shaders and register combiners to GLSL. It decodes all the Xbox texture formats. The vertex and index buffers come from a GL copy of the Xbox memory. |
| High-res HUD | The HUD is drawn from high-res assets: redraws at 8x the size of the maps' bitmaps (4x for the largest), in `port/assets/hud`. They cover the meters, counters, panels and their outlines, the motion sensor, reticles, waypoints and scopes, but no bitmap with English text. `tools/hud_assets.py` makes them from the SVG redraws (the weapons' reticles from those of the PC HUD sheet that match the Xbox's exactly, the sniper rifle's 2x and 10x set in Overpass), and the build embeds them in the executable. When the game uploads one of those bitmaps, `src/hud_hires.c` gives the high-res texture in its place, if the bitmap's pixels are those of the English maps: another language's maps keep their own. The game sizes and places the HUD from its tags as before. `display.high_res_hud = false` turns this off. |
| High-res text | The menus' and HUD's text is drawn with Overpass (`port/assets/fonts`, SIL Open Font License) in place of the maps' bitmap fonts, which are Interstate. `src/text_hires.c` rasterizes each glyph with stb_truetype (`port/third_party/stb`) at the resolution the game draws at, into an atlas that a placeholder bitmap of the game stands for. The game lays the text out from its font tags as before: each font is as tall as its tag's capitals, and narrowed where the tag's letters are narrower than the Xbox maps' (the fonts of a Custom Edition `loc.map`), so that its letters do not run into each other. The menus' titles (the screens' headers and the main menu's items) are pictures of text in the maps, so they are drawn as the high-res HUD is: `tools/title_assets.py` sets each one again in OpenCE, Roger White's public-domain Newtown respaced to match the maps' commercial title typeface (`tools/title_font.py`), at 4x the bitmap's size over its own plate or glow, each letter placed where the map's letter is, in `port/assets/titles`. The postgame carnage report's title is set over a hand-made SVG redraw of its panel (`port/assets/titles/svg`) instead. The A, B, X and Y button icons beside the menus' key labels and in their text ("Press A to Join") are drawn from hand-made SVG redraws at 8x the size of the maps' bitmaps (`port/assets/buttons`, made by `tools/button_assets.py`), in the same sizes and places. `display.high_res_text = false` turns it off. |
| Anti-aliasing | Off unless `display.anti_aliasing` is set (`src/d3d8_gl.c`, `src/xgpu_post.c`). FXAA (written in the port) and SMAA (`port/third_party/smaa`, MIT licensed, at its HIGH preset, compiled as GLSL) are passes over the 3D view of each window, after the lens flares and before the HUD and the menus (`render/render.c`). Their programs are built when the setting is chosen. Supersampling draws the render targets the size of the screen at two times the resolution in each direction, and the display blit scales them down. Multisampling draws the back buffer and its depth buffer into multisampled renderbuffers, and with them any target that is drawn together with one of them (a mirror's view, in the secondary target with the back buffer's depth buffer), so that the attachments of a framebuffer are all multisampled or none is. A target's pixels are resolved into its texture before something reads the texture (as a texture, or at the display blit). Visibility tests count samples, divided by the samples of a pixel. An alpha-tested surface (foliage, grates) covers the samples of a pixel in proportion to its alpha past the reference (`gl_SampleMask`, not on Android). |
| HDR | Off unless `display.hdr` is set (`src/d3d8_gl.c`, `src/xgpu_post.c`). The back buffer's texture, and its multisampled renderbuffer, is `GL_RGBA16F` (`GL_HALF_FLOAT` on Android), with the same `GL_DEPTH24_STENCIL8` depth buffer. The game's pixel shaders give colours from 0 to 1, as the NV2A's register combiners did, so the light past white comes from blending, which a floating-point target does not clamp. The tone mapping pass is a pass over the 3D view of each window, before the anti-aliasing (`render/render.c`): it takes each pixel from gamma 2.2 to linear light, multiplies it by `display.hdr_exposure`, applies the curve, and returns to gamma 2.2. It writes the colour only, since the game keeps its own data in the alpha. The display blit becomes a draw that clamps to 1, or, in a floating-point window, gives scRGB (linear light, 1 at the display's SDR white, which SDL reports with the headroom). The settings are read between frames; a change of `display.hdr` makes the back buffer again in the other format. Extra Effects' HDR CALIBRATION, there with a floating-point window on an HDR display, sets the peak and paper white by eye (`game/menu_functions.c`'s "port hdr calibration"): while its screen is drawn, the display takes the values its rows show, and after the display blit a program fills rectangles and rings of the window in scRGB, over its menus. For the peak, a square at the peak shown with a ring in it at 10000 nits, which disappears into the square once the square is as bright as the display goes; for paper white, a grey scale up to white, beside a white square with a ring at the peak. OK writes the values, Cancel goes back to the settings' own. |
| Sound | Xbox DirectSound on SDL3 audio (`src/dsound_sdl.c`): PCM and Xbox ADPCM, mixed at 48 kHz in stereo or 5.1, with volume, pitch, mix bins, distance, pan, occlusion and obstruction. There is no Doppler effect and no cones. On 5.1 the game gets the AC-3 speaker setting of the Xbox and sends its 2D sounds to the center and back speakers, 3D sounds pan across the five speakers, and the reverb plays from the front and back pairs. The game sends nothing to the LFE channel. |
| Input | XInput on SDL3 (`src/xinput_sdl.c`): keyboard, mouse, gamepads with rumble, and the debug keyboard for the console. |
| Files | The Win32 file functions and the MSVC file functions on POSIX, with the translation of Xbox paths. |
| Threads | Threads, events, mutexes, critical sections, interlocked operations and alertable waits. |
| Memory | The port reserves the Xbox memory at `0x80000000`. Thus the game gets the fixed addresses that it expects. |
| Saved games | The Xbox `UDATA` layout, with SHA-1 signatures. |
| Networking | Winsock on BSD sockets. System link on a local network and on the internet. |
| Bink video | Not available. The game skips the movies. |

## How the port operates

### The compiler

`tools/linux_build.py` compiles the game with clang and these options, which
give the ABI of the MSVC compiler:

- `--target=i686-linux-gnu`: 32-bit x86.
- `-fms-extensions`: the MSVC extensions.
- `-fshort-wchar`: 16-bit `wchar_t`.
- `-malign-double`: 8-byte alignment of 64-bit members.
- `-fcommon`: tentative definitions, as in C89.

glibc gives only ISO C (`__STRICT_ANSI__`). Thus POSIX names, for example
`random`, do not conflict with the names of the game.

These files supply the MSVC functions that clang does not have:

| File | Contents |
| --- | --- |
| `include/halo_linux_prefix.h` | The first header of each file: the architecture macros of the SDK, MSVC `__inline`, SEH keywords, `__declspec(selectany)`. |
| `include/` | Headers that add MSVC names to the C runtime headers. |
| `port/include/xdk` | The Xbox SDK declarations. The compiler reads this folder after all the other folders. |
| `tools/linux_msvc_semantics.py` | Makes a header that declares each struct tag at file scope, as MSVC does. It also makes the header inline functions weak, as the COMDAT functions of MSVC. `game/msvc_comdat.c` gives one external copy of each. |
| `include/halo_linux_winsock_names.h` | Gives new names to the Winsock functions of the SDK. Thus they do not link to the glibc functions with the same names. |
| `include/halo_linux_source_fixups.h` | Declares the port's functions that the game's sources call. |

`tools/linux_link_check.py` stops the link if a weak reference has no
definition. Without this check, the linker gives the reference the address
0.

### The platform layer (`src/`)

- The files `posix_*.c` use glibc. The compiler uses the ABI of the host
  for these files, because some glibc structures have a different layout
  with `-malign-double`.
- The other files include the SDK declarations through `platform.h`. Thus
  the compiler examines each definition against the SDK prototype.
- `src/halo_linker_common.c` gives weak storage for some globals of the
  January link, and for `fast_ftol_C` and `main_crash`.
- `main/d3d_intimacy.cpp` reads a private structure of the Xbox Direct3D.
  The Linux build does not use this file. `src/d3d8_gl.c` gives
  `d3d_find_flipcount`.
- The build returns small structures and unions in registers
  (`-freg-struct-return`), as on Win32.
- The GPU driver cannot open the kernel's `trace_marker`
  (`src/posix_trace_marker.c`). SteamOS keeps kernel tracing on for its GPU
  performance captures (`gpu-trace.service`), and its Mesa then writes a
  marker for each traced driver function: on the Steam Frame, some 480,000
  writes a second, which took the game from the headset's 72 Hz to about
  50 frames a second. The Steam Deck runs the same service, but its Mesa
  (25.3, 32-bit and 64-bit) has no markers to write: there the refusal
  changes nothing (measured: the same frame times and power either way).
  `HALO_GPU_TRACE_MARKERS=1` lets the driver write them, to capture with
  gpuvis.

### Game source changes

Five files of the game have changes for clang. These changes do not change
the MSVC objects: a comparison of all 612 C objects showed no difference in
code or data.

| File | Change |
| --- | --- |
| `cseries/cseries.c` | The naked function `stristr` uses `[ebp+8]` and `[ebp+12]` for its parameters. |
| `bitmaps/bitmap_drawing.c` | `*((word *)p)++` is now `*(*(word **)&p)++`. |
| `rasterizer/xbox/rasterizer_xbox_hardware_bitmaps.c` | `&(T *)x` is now `(T **)&x`. |
| `hs/hs.c` | Local prototypes that did not agree with `ai_script.h` are removed. |
| `units/vehicles.c` | The local prototype of `unit_update_animation` uses the type of `units.h`. |

`math/real_math.h` had a copy of `plane2d_from_points` that did not agree
with the function in `effects/decals.c`. clang used the copy, and parts of
levels were not visible. The copy now agrees with the function.

Other changes:

| File | Change |
| --- | --- |
| `scenario/scenario.c` | The BSP connection tables have names, not MSVC offsets. |
| `rasterizer/xbox/rasterizer_xbox_environment_fog.c` | A local pointer gets its value from the file-scope array with the same name. |
| `game/player_control.c` | The mouse aims the player on controller 1 directly. |
| `sound/game_sound.c` | The game calculates the obstruction of each sound one time for each tick, not for each frame. |
| `sound/sound_manager.c` | A track of a looping sound that is gone stops, even in the millisecond it started (`refresh_sound`). The Xbox never drew three frames in one millisecond; the port can (the null renderer, no frame limit), and then read the deleted looping sound and crashed. |
| `cseries/errors.c` | `debug.txt` stays open between lines. A line that is the same as the line before it is counted, not written again (`last line repeated N times`). At 64 MB, `debug.txt` becomes `debug.old.txt` and a new `debug.txt` starts. |
| `networking/`, `game/`, `interface/`, `bungie_net/network/` and the pools of objects, effects and sounds | The system link limits and the memory for them. |
| `game/`, `objects/`, `units/`, `networking/` | The distributed netcode. Refer to `NETCODE.md`. |
| `cache/cache_files.c` | When a map's tags load and unload, the port finds the bitmaps that the high-res HUD replaces (`game/hud_hires_tags.c`), adds the tags of the menus to the menus' map (`game/menu_tags.c`), and plays the mods' tags (`game/mods.c`). |
| `cache/xbox_sound_cache.c`, `cache/xbox_texture_cache.c` | The samples of a mod's sound are copied from memory, and the pixels of a mod's bitmap read from its file and laid out for the hardware, at once (`game/mod_sounds.c`, `game/mod_bitmaps.c`); a mod's linear bitmap need not have rows of whole steps of the pitch. |
| `hs/hs.c` | The console's `mods_reload`, `mods_enable` and `mods_list`, which a client may run too. |
| `interface/ui_widget.c`, `interface/ui_widget_event_handler_functions.c`, `interface/ui_widget_game_data_input_functions.c` | The main menu is the PC version's from `port/assets/menus` (`display.menus`); the menus' widgets can call the port's functions (`game/menu_functions.c`) and send the PC version's custom activation event; the widgets' memory is 256 KB, not 16 KB; the main menu and Multiplayer clear co-op's controllers, so that a gamepad going back to controller 1 is not a controller unplugged; the lobby's split screen players leave alone, and one who quits in game is not joined to the next game (`interface/player_ui.c`). |
| `interface/player_ui.c` | Fast setup opens the pregame screen through the port's menus (the lobby), as `interface/ui_widget.c` does. |
| `game/game_engine.c`, `game/game_engine.h`, `game/game_engine_slayer.c`, `game/game.c`, `game/aim_assist.c`, `game/players.c`, `objects/damage.c`, `objects/objects.c`, `units/units.c`, `items/weapons.c`, `saved games/playlist_profile.c`, `saved games/saved_game_files.c` | OpenCE's gametypes (`game/game_modes.c`, "OpenCE gametypes"): the gametype's PC options carry a mode and Hardcore with their options; the built-in ones are written after the Xbox's default gametypes; Infection's and Gun Game's game engines are Slayer's with their rules; SWAT's headshots, Hardcore's motion sensor, aim assist and restore of the killer; Infection's zombies' skulls, grenades, powerups, seats and melee. |
| `game/game_time.c`, `main/main.c` | With `debug.fixed_step`, a game of one machine runs a tick a frame, each frame taking a tick's time, so that it plays the same every time (`tools/stagekit`). |
| `input/input_abstraction.c`, `game/player_control.c`, `game/players.c`, `game/player_queues_new.c`, `units/units.h` | The keyboard and mouse's actions (`src/xinput_sdl.c`, `include/halo_keyboard.h`) join controller 1's game controls; their reload key reloads on its own, and their action key only acts (a control flag of the port's, sent with the player's action, stops the reload the controller's X falls back to). |
| `saved games/player_profile.h`, `interface/player_ui.c` | A sixth button layout, the Master Chief Collection's Bumper Jumper (`profile.button_preset` 5), and a seventh, Fishsticks (6: zoom on the left trigger, melee on the right stick's button, grenade throw on the right bumper, crouch on B, grenade switch on the pad's left, which then does not move you), and an eighth, Recon (7: the standard buttons with action and reload on the right bumper, grenade switch on the left bumper, flashlight on X). A layout the build does not know (a later build's) gets the standard buttons, not none, which moved start and back and stopped the game. |
| `sound/sound_manager.c`, `interface/hud.c`, `game/game_engine.c` | The music's and the other sounds' volumes; the HUD's and the scoreboard's settings are read again when Settings changes them. |
| `rasterizer/xbox/rasterizer_xbox_shadows.c` | With larger shadow maps (`display.shadow_resolution`), the blur's taps stay half a texel apart, and more passes widen it to cover the same part of the map as on the Xbox. |
| `rasterizer/rasterizer.c` | The 128 KB of vertices of the debug display of a model's vertices are static, not on the stack: the compiler inlines the function in `render_model`, and the Windows build probed the stack a page at a time for every model drawn. |
| `render/render.c` | The 3D view of each window is tone mapped (`display.hdr`) and antialiased (`display.anti_aliasing`) before the HUD is drawn. |
| `render/render.c` | `render_initialize` reads the structure culling tests from `display.structure_culling`. |
| `models/models.c` | A model's size, which chooses its level of detail and whether it is drawn, is scaled as `display.model_detail` says (`halo_model_detail_scale`, `src/d3d8_gl.c`), and each permutation draws the level `game/model_detail.c` chooses from the model's (stock: the model's). |
| `interface/hud.c` | In multiplayer, players' names are drawn above their heads (`display.player_names`, `display.player_name_scale`). |
| `text/draw_string.c`, `interface/terminal.c`, `interface/virtual_keyboard.c` | Streamer mode's words are hidden in every string drawn or measured (`game/text_filter.c`), but in the console's line and the virtual keyboard's text being typed. |
| `rasterizer/rasterizer_text.c`, `text/draw_string.c` | Text is drawn from an atlas of the fonts' glyphs, rasterized at the resolution the game draws at (`src/text_hires.c`), when the font has every character of the string. Text can be drawn scaled about a point (`rasterizer_text_set_scale`), as the players' names are. Each glyph's advance is centred on the font tag character's, so the layout is the same, and a glyph is cut at a text box only where the font tag's character visibly was. |
| `interface/first_person_weapons.c` | The first-person hands are drawn with the unit's shield modifier shader, as its body is (`display.viewmodel_shield`). |
| `render/render_objects.c`, `objects/objects.c` | In first person, your own body is drawn from the waist down with the pose `first_person_legs.c` makes (`display.first_person_legs`); the waist it found and the mesh `object_mesh.c` read are forgotten on a new map. |
| `objects/object_lights.c` | Glows the port lights itself (`shield_glow.c`): dynamic lights attached to an object's node, colored and sized each frame by their maker, drawn with a light definition the map has. |
| `render/render_objects.c`, `interface/first_person_weapons.c`, `rasterizer/xbox/rasterizer_xbox_plasma_energy.c`, `items/projectiles.c` | An overshield as `overshield_effect.c` draws it (`display.overshield_effect`): in its own color, its intensity full, its energy slower, faded above the height it has risen to; a hit on it plays the projectile's effect for an Elite's energy shield in place of its own (only the effect: the projectile's response is its own). |
| `render/render_objects.c`, `interface/first_person_weapons.c`, `rasterizer/xbox/rasterizer_xbox_plasma_energy.c` | A local player's shield plasma is drawn in the chosen armor color (`display.shield_color`, `shield_color.c`), on the body and on the first-person arms. |
| `rasterizer/xbox/rasterizer_xbox_text.c`, `rasterizer/rasterizer_text.c` | The characters of a string are one draw, not a draw each (and a second for the drop shadow). The batch is drawn early before a character is cached and before the high-res glyph atlas starts over (`src/text_hires.c`). |
| `rasterizer/xbox/rasterizer_xbox_decals.c` | Decals drawn one after another with the same blend function and bitmap are one draw, with each decal's color in its vertices, not a draw each: a long fight leaves hundreds in view. Within a run of decals whose blend functions commute (multiply, add, min, max, reverse subtract), they are drawn by bitmap, so more of them share a draw. |
| `rasterizer/xbox/rasterizer_xbox_widgets.c`, `objects/widgets/light_volumes.c` | A light volume's sprites (a plasma bolt's glow, a couple of hundred) are one draw. |
| `rasterizer/rasterizer.h` | A frame holds 2048 transparent geometry groups, not 384 (`include/halo_port_capacity.h`): a co-op game with many added enemies asked for more, and the Xbox's pool drew the frame without the rest. |
| `structures/structure_render.c` | The structure's visible surfaces are put in order by a radix sort, not `qsort_4byte`: the same order. |
| `cache/cache_files.c`, `networking/network_client_manager.c`, `networking/network_server_manager.c`, `networking/network_server_message_handler.c`, `main/main.c`, `interface/interface.c` | Map sharing (`game/map_share.c`): a client that does not have the game's Custom Edition map asks the host for it instead of leaving, is not given time to precache it until it has it, and adds its players to a game in progress only then; the host gives the game's map its own name and its version (its hash's start), takes the requests from any machine in the game, lobby too, and does not drop a machine that downloads; the transfers run each frame, and their progress is drawn over the screen. |
| `cutscene/cinematics.c` | A scenario's titles (some Custom Edition maps show their name for the whole game) are not drawn over a menu, as the letterbox is not: the screen's overlays are drawn after the players' menus. |
| `tag_files/tag_groups.h` | `TAG_BLOCK_GET_ELEMENT` returns an element in the block without a call, and calls `tag_block_get_element_with_size` for anything else. |
| `game/players.c`, `networking/network_server_manager.c` | Bots (`game/bots.c`, `bots.count`): the lobby of a local Slayer game takes the bot players, each the one player of a machine of its own that no real machine is, and every tick their actions come from `game/bots.c` instead of their empty queues. Nothing changes in a system link or internet game. |
| `input/input_xbox.c`, `physics/collision_usage.c` | The clock is read only for a button held, and for collision tests only when their times are shown: a read of the clock is slow in a browser. |
| `main/main.c`, `interface/hud.c`, `interface/hud_unit.c`, `interface/hud_draw.c`, `game/game_engine.c` | The text chat of network games (`game/network_chat.c`): its keys are read each frame, and the debug keys are not while a line is typed; each view's HUD draws its lines, above the motion sensor, whose place the HUD notes (the left of the last picture drawn); the scoreboard's menu has Mute voice and chat, and the host's Mute for everyone. |

The x86 inline assembly of the game is replaced by C. Thus the compiler
can optimize that code for each processor:

| File | Assembly | Replacement |
| --- | --- | --- |
| `cseries/cseries.h` | x87 `fistp` (`fast_ftol`) | `__builtin_rint` |
| `bitmaps/bitmaps_inlines.h` | x87 conversions | C conversions |
| `math/matrix_math.c` | SSE `matrix4x3_multiply` | a C loop |
| `effects/decals.c` | an x87 conversion | a C conversion |
| `cseries/profile.c` | `rdtsc` | `QueryPerformanceCounter` |
| `cseries/cseries.c` | naked `stristr` | a C `stristr` |
| `cseries/stack_walk_windows.c` | a read of EBP | `__builtin_frame_address` |
| `interface/hud_draw.c` | a read of `[ebp+4]` | `__builtin_return_address(1)` |
| `bink/bink_playback.c` | `int 3` | `__builtin_trap` |

The x87 control and status words (`_control87`, `_statusfp`, `_clearfp` in
`src/msvc_crt.c`) use `fenv.h`. On Android, they use the FPCR and FPSR. In
the web build, the control word is only remembered.

The web build's changes (`HALO_WEB`: the main loop runs one iteration for
each of the browser's frames, the cache thread starts at a function of
`CreateThread`'s type, `debug.txt`'s lines go to the page's log) and the
declarations given their definitions' types, which WebAssembly needs, are
listed in "Game source changes" in [../web/README.md](../web/README.md).

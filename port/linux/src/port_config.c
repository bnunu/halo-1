/*
PORT_CONFIG.C

The native ports' settings (port_config.h), parsed with tomlc17
(port/third_party/tomlc17). Every setting is in the table below with its
type, default, the HALO_* environment variable that overrides it and the
comment written into a new file. The file is read once, on the first
question; unknown keys and values of the wrong type are reported in the log
and the defaults used instead, and the file itself is never rewritten once
it exists, so that the player's edits and comments stay.
*/

#include "platform.h"
#include "port_config.h"
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* (the command line: --config, --server, --probe...) */
#include "posix.h"
#include "posix_server.h"
#endif
#include "tomlc17.h"

#include <SDL3/SDL.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

/* ---------- the settings */

enum config_type
{
	_config_boolean,
	_config_integer,
	_config_real,
	_config_string,
};

/* how the setting's environment variable sets it */
enum config_environment
{
	/* the variable's text is the value ("0", "false", "no" and "off" are
	false for a boolean) */
	_environment_value,
	/* the variable being set (not empty, "0", "false", "no" or "off") makes it true */
	_environment_set_is_true,
	/* the variable being set (not empty, "0", "false", "no" or "off") makes it false */
	_environment_set_is_false,
};

/* the builds a setting means something in, and is written for */
enum
{
	_platform_desktop = 1,
	_platform_android = 2,
	_platform_all = _platform_desktop | _platform_android,
	/* (of the desktop builds, only Windows) */
	_platform_windows = 4,
	/* (the web build, which has the desktop's settings, and these) */
	_platform_web = 8,
	/* (a dedicated server's, of the Linux and Windows builds: written into
	config.toml only in server mode) */
	_platform_server = 16,
};

struct config_setting
{
	const char *name;
	enum config_type type;
	/* as it is written in the file */
	const char *default_value;
	/* NULL: none, for a setting the Android app reads from the file itself */
	const char *environment;
	enum config_environment environment_style;
	unsigned platforms;
	const char *comment;
};

/* crouching's keys (in a browser C alone: a page cannot keep Ctrl+W, Ctrl+S or
Ctrl+D, which moving and crouching would press, from the browser) */
#ifdef HALO_WEB
#define CROUCH_KEYS "\"C\""
#else
#define CROUCH_KEYS "\"Left Ctrl, C\""
#endif

/* display.resolution_scaling's default: on Android the Xbox's 480 lines, which
a phone's GPU draws with time to spare, elsewhere the window's resolution */
#ifdef HALO_ANDROID
#define RESOLUTION_SCALING "\"original\""
#else
#define RESOLUTION_SCALING "\"native\""
#endif

static const struct config_setting config_settings[] =
{
	{ "display.fullscreen", _config_boolean, "true", "HALO_FULLSCREEN", _environment_value, _platform_desktop,
		"Where display.mode is empty: start borderless over the whole display;\n"
		"false starts in a window. F11 switches." },
	{ "display.mode", _config_string, "\"\"", "HALO_DISPLAY_MODE", _environment_value, _platform_desktop,
		"\"fullscreen\" takes the display (at display.resolution's mode),\n"
		"\"borderless\" is a window over the whole desktop, \"windowed\" a window\n"
		"(display.window_size). Empty: display.fullscreen's (true: borderless).\n"
		"F11 switches to the window and back." },
	{ "display.resolution", _config_string, "\"native\"", "HALO_RESOLUTION", _environment_value, _platform_desktop,
		"What fullscreen and borderless draw at: \"native\", the display's own, or\n"
		"\"<width>x<height>\" (\"1920x1080\"), 640x480 or more. Fullscreen sets the\n"
		"display to it; borderless draws it scaled to the display." },
	{ "display.resolution_scaling", _config_string, RESOLUTION_SCALING, "HALO_RESOLUTION_SCALING", _environment_value,
		_platform_all,
		"\"native\" draws at the window's resolution (fullscreen, the display's or\n"
		"display.resolution; on Android, the display's); \"75%\", \"50%\" or \"25%\"\n"
		"draw that part of it each way and scale it up; \"original\" draws the\n"
		"Xbox's 480 lines (640x480; on Android, at the display's shape) and\n"
		"scales them up, as Android does unless this is set." },
	{ "display.window_size", _config_string, "\"\"", "HALO_WINDOW_SIZE", _environment_value, _platform_desktop,
		"The window's size, \"<width>x<height>\" (\"1920x1080\"), 640x480 or more (it\n"
		"can be resized). Empty: display.window_scale's." },
	{ "display.window_scale", _config_integer, "2", "HALO_WINDOW_SCALE", _environment_value, _platform_desktop,
		"Where display.window_size is empty: the window's size as a multiple of\n"
		"640x480." },
	{ "display.screen_width", _config_integer, "0", "HALO_SCREEN_WIDTH", _environment_value, _platform_android,
		"Columns of the 480-line picture: 0 for the display's shape, 640 for the\n"
		"Xbox's 4:3." },
	{ "display.vsync", _config_boolean, "true", "HALO_NO_VSYNC", _environment_set_is_false, _platform_all,
		"Wait for the display between frames; false draws as fast as possible." },
	{ "display.max_fps", _config_integer, "0", "HALO_MAX_FPS", _environment_value, _platform_all,
		"With vsync off, the most frames a second: 0 for twice the display's\n"
		"refresh rate, -1 for no limit (which can hang some Intel graphics)." },
	{ "display.frames_in_flight", _config_integer, "3", "HALO_FRAMES_IN_FLIGHT", _environment_value,
		_platform_desktop,
		"The most frames the GPU has queued (1 to 8): a frame waits for the GPU\n"
		"to finish the one this many before it, so that frames come evenly when\n"
		"the GPU is the slower; 0 leaves it to the driver." },
	/* port: the frame counter the game draws at the bottom right of a game
	(main.c's main_framerate_render, display_framerate) for a setting, so the
	settings screens show it (tools/port_settings.py) and it is kept in
	config.toml: true to draw it, green from 30 frames a second, red below */
	{ "display.show_fps", _config_boolean, "false", "HALO_SHOW_FPS", _environment_set_is_true, _platform_all,
		"Draw the frame rate at the bottom right of a game, counted over half a\n"
		"second: green from 30 frames a second, red below." },
	{ "display.anti_aliasing", _config_string, "\"off\"", "HALO_ANTI_ALIASING", _environment_value, _platform_all,
		"Smoothing of jagged edges, which the Xbox did not have: \"off\"; \"fxaa\"\n"
		"or \"smaa\" smooth the 3D view once it is drawn (the HUD and menus stay\n"
		"sharp); \"ssaa2x\" draws at twice the resolution each way (four times\n"
		"the work); \"msaa2x\", \"msaa4x\" or \"msaa8x\" draw with that many samples\n"
		"a pixel. Android has \"fxaa\" for \"smaa\", and no \"ssaa2x\"." },
	{ "display.interpolation", _config_boolean, "true", "HALO_INTERPOLATION", _environment_value, _platform_all,
		"Draw a frame for every display refresh, blending between the game's 30\n"
		"ticks a second; false keeps the original 30 frames a second." },
	{ "display.menu_cursor", _config_string, "\"large\"", "HALO_MENU_CURSOR", _environment_value, _platform_desktop,
		"The menus' pointer for the mouse: the game's blue arrow, \"small\",\n"
		"\"medium\", \"large\" or \"extra_large\" (18, 24, 32 or 40 of the menus'\n"
		"480 lines tall, at any resolution), or \"off\" for the system's pointer." },
	{ "display.direct_camera", _config_boolean, "true", "HALO_DIRECT_CAMERA", _environment_value, _platform_desktop,
		"In first person, point the view where the player aims now instead of\n"
		"where the last tick left it: the view turns the frame the mouse moves,\n"
		"not up to two ticks (66 ms) later." },
	{ "display.fov", _config_real, "0.0", "HALO_FOV", _environment_value, _platform_all,
		"The first-person view's field of view on foot, in degrees across at\n"
		"16:9 (20 to 150); 0 keeps the stock view. Vehicles, cinematics and\n"
		"scripted cameras keep their own." },
	{ "display.viewmodel_fov", _config_real, "0.0", "HALO_VIEWMODEL_FOV", _environment_value, _platform_all,
		"The first-person weapon's and hands' field of view, in degrees across\n"
		"at 16:9 (20 to 150); 0 keeps the weapon's stock view, also when\n"
		"display.fov widens the world." },
	{ "display.viewmodel_visible", _config_boolean, "true", "HALO_VIEWMODEL_VISIBLE", _environment_value, _platform_all,
		"Draw the first-person weapon, hands and what is attached to them.\n"
		"Off, they are not drawn; firing, animation, sound and lights go on." },
	{ "display.passenger_third_person", _config_boolean, "false", "HALO_PASSENGER_THIRD_PERSON", _environment_value, _platform_all,
		"Third-person Warthog and Scorpion passengers. Weapon zoom is disabled." },
	{ "display.high_res_hud", _config_boolean, "true", "HALO_HIGH_RES_HUD", _environment_value, _platform_all,
		"Draw the HUD (meters, counters, panels, motion sensor, reticles,\n"
		"waypoints, scopes) from the high-res assets (8x the maps' bitmaps);\n"
		"false draws the maps' own bitmaps." },
	{ "display.hud_button_style", _config_string, "\"classic\"", "HALO_HUD_BUTTON_STYLE", _environment_value, _platform_all,
		"Manual high-res gameplay button style, preserved while automatic detection\n"
		"is on. Xbox: classic, gems, menu_gems (dark gems), xbox_modern.\n"
		"PlayStation: ps_classic, ps_gems, ps_dark_gems, ps_modern.\n"
		"Switch: switch_classic, switch_gems, switch_modern. Requires high_res_hud." },
	{ "display.hud_bumper_style", _config_string, "\"original\"", "HALO_HUD_BUMPER_STYLE", _environment_value, _platform_all,
		"Xbox Classic, Gems and Dark Gems bumpers: original (black/white) or modern\n"
		"(LB/RB). Shared by local players, including automatic button detection.\n"
		"Requires high_res_hud; other button styles keep their own artwork." },
	{ "input.auto_button_detection", _config_boolean, "true", "HALO_AUTO_BUTTON_DETECTION", _environment_value, _platform_all,
		"Limit Button Style to the assigned controller's family. Unknown devices\n"
		"keep the manual choice. Device detection itself is always available." },
	{ "input.controller_p1", _config_string, "\"\"", "HALO_CONTROLLER_P1", _environment_value, _platform_all,
		"P1 custom controller layout; empty uses the profile. Edit in Gamepads >\n"
		"Button Remapping. v2: stores 25 physical-button names, including unbound.\n"
		"Gameplay and debug camera have separate bindings. Old ten-action layouts\n"
		"are accepted. See docs/reading/controller_remapping.md for the full order." },
	{ "input.controller_p2", _config_string, "\"\"", "HALO_CONTROLLER_P2", _environment_value, _platform_all,
		"P2 custom controller buttons, same format as controller_p1. Empty uses the profile." },
	{ "input.controller_p3", _config_string, "\"\"", "HALO_CONTROLLER_P3", _environment_value, _platform_all,
		"P3 custom controller buttons, same format as controller_p1. Empty uses the profile." },
	{ "input.controller_p4", _config_string, "\"\"", "HALO_CONTROLLER_P4", _environment_value, _platform_all,
		"P4 custom controller buttons, same format as controller_p1. Empty uses the profile." },
	{ "display.hud_button_style_xbox", _config_string, "\"classic\"", "HALO_HUD_BUTTON_STYLE_XBOX", _environment_value, _platform_all,
		"Remembered automatic Xbox style: classic, gems, menu_gems, xbox_modern." },
	{ "display.hud_button_style_ps", _config_string, "\"ps_classic\"", "HALO_HUD_BUTTON_STYLE_PS", _environment_value, _platform_all,
		"Remembered automatic PlayStation style: ps_classic, ps_gems, ps_dark_gems, ps_modern." },
	{ "display.hud_button_style_switch", _config_string, "\"switch_classic\"", "HALO_HUD_BUTTON_STYLE_SWITCH", _environment_value, _platform_all,
		"Remembered automatic Switch style: switch_classic, switch_gems, switch_modern." },
	{ "display.high_res_text", _config_boolean, "true", "HALO_HIGH_RES_TEXT", _environment_value, _platform_all,
		"Draw the menus' and HUD's text with the fonts in port/assets/fonts\n"
		"(Overpass) at the resolution the game draws at, and the menus' titles\n"
		"from port/assets/titles; false draws the maps' bitmap fonts and titles." },
	{ "display.shadow_resolution", _config_integer, "128", "HALO_SHADOW_RESOLUTION", _environment_value,
		_platform_all,
		"The size the objects' shadows are drawn at, in pixels each way: 128 as\n"
		"on the Xbox, or 256, 512 or 1024 for smoother edges, as soft." },
	{ "display.menus", _config_string, "\"pc\"", "HALO_MENUS", _environment_value, _platform_all,
		"The menus: \"pc\" for the PC version's main menu (port/assets/menus,\n"
		"and a menus folder here for your own), \"xbox\" for the Xbox's." },
	{ "display.menu_language", _config_string, "\"maps\"", "HALO_MENU_LANGUAGE", _environment_value, _platform_all,
		"The PC menus' language: \"maps\" for the language of the Custom Edition\n"
		"whose loc.map is in custom_maps (as that edition shows them; English\n"
		"without one), \"english\" for English. The port's own lines are English." },
	{ "display.player_names", _config_string, "\"all\"", "HALO_PLAYER_NAMES", _environment_value, _platform_all,
		"In multiplayer, whose names are drawn above their heads: \"all\",\n"
		"\"allies\", \"enemies\" or \"none\". An enemy's shows only within the\n"
		"motion sensor's reach, in sight and not camouflaged; none show if the\n"
		"gametype's motion tracker shows no players, only allies' if it shows\n"
		"only friends." },
	{ "display.player_name_scale", _config_real, "1.0", "HALO_PLAYER_NAME_SCALE", _environment_value, _platform_all,
		"How large the players' names are drawn: 1.0 three quarters of the size of\n"
		"the HUD's text, 0.25 to 4." },
	{ "display.match_timer", _config_boolean, "false", "HALO_MATCH_TIMER", _environment_value, _platform_all,
		"Draw the match clock at the bottom right in multiplayer. It counts up\n"
		"from the game's first tick, which the items' respawn periods count from\n"
		"too: a weapon is back on the map at every multiple of its period on the\n"
		"clock." },
	{ "display.scoreboard_team_layout", _config_string, "\"teams\"", "HALO_SCOREBOARD_TEAM_LAYOUT", _environment_value,
		_platform_all,
		"How the scoreboard lists a team game's players: \"teams\" in a column for\n"
		"each team (red on the left, blue on the right), \"score\" all in order of\n"
		"score." },
	{ "display.scoreboard_background", _config_boolean, "true", "HALO_SCOREBOARD_BACKGROUND", _environment_value,
		_platform_all,
		"Draw a panel behind the multiplayer scoreboard, for clearer text." },
	{ "display.scoreboard_background_color", _config_string, "\"16, 16, 16, 150\"", "HALO_SCOREBOARD_BACKGROUND_COLOR",
		_environment_value, _platform_all,
		"The scoreboard panel's colour: \"red, green, blue, alpha\", each 0 to 255\n"
		"(alpha 0 is see-through, 255 solid)." },
	{ "display.anisotropic_filtering", _config_integer, "0", "HALO_ANISOTROPIC_FILTERING", _environment_value,
		_platform_all,
		"Sharpen textures seen at a grazing angle (floors, walls, terrain), which\n"
		"the Xbox did not: 2, 4, 8 or 16 times; 0 filters as the Xbox did. 4 keeps\n"
		"most of 16's sharpness at less cost on slower GPUs." },
	{ "display.dxt1_astc", _config_boolean, "true", "HALO_DXT1_ASTC", _environment_value, _platform_android,
		"On a GPU without DXT textures (Mali), give it the DXT1 textures as ASTC,\n"
		"which holds them nearly as they are in a quarter of the memory of the\n"
		"32-bit texels they are otherwise decoded to; false decodes them." },
	{ "display.latency_meter", _config_boolean, "false", "HALO_LATENCY_METER", _environment_value, _platform_all,
		"In a network game, your round trip to the host in milliseconds at the\n"
		"screen's top right, with bars (the host: its slowest player's), and\n"
		"\"Connection problem\" when the host has sent nothing for two seconds;\n"
		"the scoreboard's pings in the same colours, as this machine times them." },
	{ "display.shader_cache", _config_boolean, "true", "HALO_SHADER_CACHE", _environment_value, _platform_desktop,
		"Keep the GPU programs the game links under the save root (shader_cache),\n"
		"and load them from there next time instead of linking them again, which\n"
		"stalls a frame on some drivers; false links each every run." },
	{ "display.stats_log", _config_boolean, "false", "HALO_STATS_LOG", _environment_value, _platform_all,
		"A line a second in the log with the frame rate, the mode, the map, the\n"
		"renderer and the size of the 3D view, so a graphics bug's report or two\n"
		"builds compared say which of them was running and at what frame rate.\n"
		"Nothing is sent anywhere; off by default." },
	{ "display.per_pixel_lighting", _config_boolean, "false", "HALO_PER_PIXEL_LIGHTING", _environment_value,
		_platform_all,
		"Light the models (characters, weapons, vehicles, scenery) for each\n"
		"pixel by the lights the game gives them, without the facets the light\n"
		"of each vertex shows across curved surfaces; false lights each vertex,\n"
		"as the Xbox does." },
	{ "display.structure_culling", _config_integer, "31", "HALO_STRUCTURE_CULLING", _environment_value,
		_platform_all,
		"Which tests skip the level's triangles the camera cannot see, as bits\n"
		"added together: 1 outside the view, 2 groups facing away, 4 triangles\n"
		"facing away, 8 smaller groups, 16 nearer rooms first; 0 walks them as\n"
		"the Xbox does. The console's structure_culling changes it until the\n"
		"game quits." },
	{ "display.model_detail", _config_string, "\"stock\"", "HALO_MODEL_DETAIL", _environment_value, _platform_all,
		"Which of their levels of detail models are drawn at: \"stock\" chooses by\n"
		"their size on the Xbox's 480 lines, at any resolution; \"native\" by their\n"
		"size on the display's, so that more detail is kept at a distance at high\n"
		"resolutions; \"highest\" draws the models \"native\" does, each at its most\n"
		"detailed level." },
	{ "display.reversed_depth", _config_boolean, "true", "HALO_REVERSED_DEPTH", _environment_value, _platform_desktop,
		"Keep depth in 32-bit floats, reversed (1 near, 0 far), as precise far\n"
		"away as near: surfaces a few millimetres apart (the lights on Blood\n"
		"Gulch's base) stay apart at any distance in place of flickering\n"
		"through each other. false: 24 bits, as the Xbox's. Taken up when the\n"
		"game starts." },
	{ "display.hdr", _config_boolean, "false", "HALO_HDR", _environment_value, _platform_all,
		"Draw the 3D view into a floating-point back buffer, which keeps what\n"
		"the game's additive effects (explosions, plasma, lights, the sun's\n"
		"glare) add past white, and tone map it before the HUD is drawn:\n"
		"display.hdr_tone_mapping brings it back to the display's range\n"
		"instead of clipping it. false draws into 8 bits a channel, as the\n"
		"Xbox does." },
	{ "display.hdr_exposure", _config_real, "1.0", "HALO_HDR_EXPOSURE", _environment_value, _platform_all,
		"With display.hdr, the 3D view's brightness in linear light before it\n"
		"is tone mapped, 0.25 to 4.0." },
	{ "display.hdr_tone_mapping", _config_string, "\"gt7\"", "HALO_HDR_TONE_MAPPING", _environment_value,
		_platform_all,
		"With display.hdr, the curve that fits the brightest light into what the\n"
		"display shows: \"gt7\" (Gran Turismo 7's: the brightest light turns to\n"
		"white, as film shows it), \"neutral\" (the picture as drawn, its\n"
		"brightest parts rolled off at their own colour), \"aces\" (filmic,\n"
		"more contrast) or \"reinhard\" (softer, and darker throughout)." },
	{ "display.hdr_output", _config_boolean, "false", "HALO_HDR_OUTPUT", _environment_value, _platform_desktop,
		"With display.hdr, ask for a floating-point window (scRGB), so that an\n"
		"HDR display shows what is past white brighter than the desktop's\n"
		"white rather than tone mapped down to it; the operating system's HDR\n"
		"must be on. Taken up when the game starts." },
	{ "display.hdr_paper_white", _config_integer, "0", "HALO_HDR_PAPER_WHITE", _environment_value, _platform_desktop,
		"With display.hdr_output on an HDR display, how bright white is, in\n"
		"nits (80 to 1000): the HUD, the menus, and the 3D view's white. 0:\n"
		"Windows' SDR content brightness." },
	{ "display.hdr_peak_brightness", _config_integer, "0", "HALO_HDR_PEAK_BRIGHTNESS", _environment_value,
		_platform_desktop,
		"With display.hdr_output on an HDR display, the brightest the display\n"
		"shows, in nits (250 to 10000), which the brightest light is fitted to.\n"
		"0: what Windows reports (set by its HDR Calibration app)." },

	{ "audio.enabled", _config_boolean, "true", "HALO_NO_AUDIO", _environment_set_is_false, _platform_all,
		"Play sound." },
	{ "audio.volume", _config_real, "1.0", "HALO_VOLUME", _environment_value, _platform_all,
		"The volume of everything, 0.0 to 1.0." },
	{ "audio.music_volume", _config_real, "1.0", "HALO_MUSIC_VOLUME", _environment_value, _platform_all,
		"The music's volume, 0.0 to 1.0 (of audio.volume)." },
	{ "audio.effects_volume", _config_real, "1.0", "HALO_EFFECTS_VOLUME", _environment_value, _platform_all,
		"The volume of every other sound (effects and speech), 0.0 to 1.0 (of\n"
		"audio.volume)." },
	{ "audio.buffer_frames", _config_integer, "512", "HALO_AUDIO_BUFFER_FRAMES", _environment_value,
		_platform_desktop,
		"The audio device's buffer, in sample frames at 48 kHz (64 to 8192): larger\n"
		"rides out stalls that cut the sound out, smaller has less delay (512\n"
		"is 11 ms, 2048 is 43 ms)." },
	{ "audio.channels", _config_string, "\"stereo\"", "HALO_AUDIO_CHANNELS", _environment_value, _platform_desktop,
		"\"stereo\", \"5.1\" (surround, as the Xbox's Dolby Digital), or \"auto\"\n"
		"for 5.1 when the sound device has six or more channels." },
	{ "audio.reverb", _config_boolean, "true", "HALO_REVERB", _environment_value, _platform_all,
		"Reverberate the world's sounds as the place the player is in does (the\n"
		"maps' sound environments, as the Xbox's I3DL2 reverb did); false keeps\n"
		"them dry." },
	{ "audio.resampler", _config_string, "\"sinc\"", "HALO_AUDIO_RESAMPLER", _environment_value, _platform_all,
		"How sounds are converted to the output's rate: \"sinc\" with a\n"
		"band-limited low pass; \"linear\" between neighbouring samples,\n"
		"brighter for the aliasing it lets through." },
	{ "audio.voice_chat", _config_string, "\"push_to_talk\"", "HALO_VOICE_CHAT", _environment_value, _platform_all,
		"Talking in network games' voice chat: \"push_to_talk\" (while\n"
		"controls.push_to_talk is held; the microphone opens the first time),\n"
		"\"open_mic\" (whenever the microphone hears speech), or \"off\". Others'\n"
		"voices play whatever this is (audio.voice_volume 0 silences them)." },
	{ "audio.voice_volume", _config_real, "1.0", "HALO_VOICE_VOLUME", _environment_value, _platform_all,
		"The volume of the other players' voices (0 to 2)." },
	{ "audio.output_device", _config_string, "\"default\"", "HALO_AUDIO_OUTPUT_DEVICE", _environment_value,
		_platform_desktop,
		"Where the game's sound and the voices play: a device's name as Settings >\n"
		"Audio lists it, or \"default\" for the system's (also if it is not found)." },
	{ "audio.input_device", _config_string, "\"default\"", "HALO_AUDIO_INPUT_DEVICE", _environment_value,
		_platform_desktop,
		"The microphone voice chat listens to: a device's name as Settings >\n"
		"Audio lists it, or \"default\" for the system's (also if it is not found)." },

	{ "input.touch_controls", _config_string, "\"auto\"", "HALO_TOUCH_CONTROLS", _environment_value, _platform_android | _platform_web,
		"The on-screen touch controls in a game: \"auto\" shows them on a\n"
		"touchscreen while no controller is connected, \"on\" also with a\n"
		"controller, \"off\" never. A device without a touchscreen never shows\n"
		"them. The menus take taps in any case. On Android, the carnage\n"
		"report's CONTINUE and LEAVE buttons follow the same rule (otherwise\n"
		"it shows its A and B prompt)." },
	{ "input.touch_aim_assist", _config_boolean, "true", "HALO_TOUCH_AIM_ASSIST", _environment_value, _platform_android | _platform_web,
		"The touch controls' swipe aiming gets a controller's aim assist: the\n"
		"aim slows over a target and follows a moving one. false: none, as a\n"
		"mouse (the bullets' own autoaim stays)." },
	{ "input.mouse_sensitivity", _config_real, "1.0", "HALO_MOUSE_SENSITIVITY", _environment_value, _platform_desktop,
		"How far the view turns for the mouse's movement." },
	{ "input.playstation_kernel_driver", _config_boolean, "false", "HALO_PLAYSTATION_KERNEL_DRIVER",
		_environment_value, _platform_desktop,
		"Linux: a PlayStation controller (DualSense, DualShock 4) through the\n"
		"kernel's driver of it (hid-playstation, hid-sony), which needs no setup;\n"
		"false: through SDL's own, which needs Steam's udev rules to open it. Read\n"
		"at start-up." },
	{ "input.invert_mouse", _config_boolean, "false", "HALO_MOUSE_INVERT", _environment_set_is_true, _platform_desktop,
		"Moving the mouse forward looks down." },
	{ "network.block_mouse_aim_assist", _config_boolean, "false", "HALO_NET_BLOCK_MOUSE_AIM_ASSIST", _environment_value,
		_platform_all, "Host: disable optional mouse magnetism for mouse/keyboard players. Controllers and touch keep their own assist." },
	{ "network.shield_effects", _config_string, "\"stock\"", "HALO_NET_SHIELD_EFFECTS", _environment_value, _platform_all,
		"Host: the shield effects every player of a network game sees, whatever\n"
		"their own: \"stock\", \"glow\", \"light_show\", \"overshield\",\n"
		"\"glow_overshield\" or \"light_show_overshield\" (display.shield_glow,\n"
		"its light_show intensity, display.overshield_effect); \"players\": not\n"
		"synced, each player sees their own settings'. An online co-op game is\n"
		"always each player's own. SHIELD EFFECTS in Server Setup > Player\n"
		"Options writes its choice here." },
	{ "network.fire_origins", _config_boolean, "true", "HALO_NET_FIRE_ORIGINS", _environment_value, _platform_all,
		"Host: whether a reported hit the shooter's camera had no line of sight\n"
		"to may still count from where their machine says the shot left, when\n"
		"that is not the camera (a VR build's hand: past a corner the camera is\n"
		"behind), no further than a player leaning out and an arm, and in front\n"
		"of the eye. false: from the camera and vehicles' guns only." },
	{ "input.mouse_aim_assist", _config_boolean, "false", "HALO_MOUSE_AIM_ASSIST", _environment_value, _platform_desktop,
		"Magnetism while aiming with the mouse, as with a controller: the view\n"
		"slowed and dragged along by a target. The last of the mouse and the\n"
		"right stick to move decides. The bullets' autoaim (bent toward the\n"
		"target) stays either way." },
	{ "input.mouse_vertical_sensitivity", _config_real, "0.0", "HALO_MOUSE_VERTICAL_SENSITIVITY", _environment_value,
		_platform_desktop,
		"How far the view turns up and down for the mouse's movement; 0 for the\n"
		"same as input.mouse_sensitivity." },
	{ "input.left_stick_dead_zone", _config_integer, "-1", "HALO_LEFT_STICK_DEAD_ZONE", _environment_value, _platform_all,
		"How far the controller's left stick moves before it moves the player,\n"
		"in percent of its travel (0 to 50): lower for finer control, higher for\n"
		"a worn stick that drifts; -1 for the game's own (27%). On Android, the\n"
		"touch controls' stick too." },
	{ "input.right_stick_dead_zone", _config_integer, "-1", "HALO_RIGHT_STICK_DEAD_ZONE", _environment_value, _platform_all,
		"How far the controller's right stick moves before it turns the view, in\n"
		"percent of its travel (0 to 50); -1 for the game's own (27%)." },

	/* the keyboard and mouse's own controls (port/linux/src/xinput_sdl.c) */
	{ "controls.move_forward", _config_string, "\"W\"", "HALO_KEY_MOVE_FORWARD", _environment_value, _platform_all,
		"The keyboard and mouse's controls, which Settings > Controls Setup\n"
		"changes: up to two keys or buttons each, separated by a comma. Keys by\n"
		"their names on a US keyboard (\"W\", \"Space\", \"Left Ctrl\", \"F1\"): a key\n"
		"is the one in that place on any keyboard, which the menus show by its\n"
		"own label. Buttons: \"Mouse Left\", \"Mouse Right\", \"Mouse Middle\",\n"
		"\"Mouse 4\", \"Mouse 5\", \"Wheel\" (either way), \"Wheel Up\" and \"Wheel\n"
		"Down\"; empty for none. Moving forward:" },
	{ "controls.move_backward", _config_string, "\"S\"", "HALO_KEY_MOVE_BACKWARD", _environment_value, _platform_all,
		"Moving backward." },
	{ "controls.strafe_left", _config_string, "\"A\"", "HALO_KEY_STRAFE_LEFT", _environment_value, _platform_all,
		"Moving left." },
	{ "controls.strafe_right", _config_string, "\"D\"", "HALO_KEY_STRAFE_RIGHT", _environment_value, _platform_all,
		"Moving right." },
	{ "controls.jump", _config_string, "\"Space\"", "HALO_KEY_JUMP", _environment_value, _platform_all,
		"Jumping (and skipping cutscenes)." },
	{ "controls.crouch", _config_string, CROUCH_KEYS, "HALO_KEY_CROUCH", _environment_value, _platform_all,
		"Crouching." },
	{ "controls.fire", _config_string, "\"Mouse Left\"", "HALO_KEY_FIRE", _environment_value, _platform_all,
		"Firing." },
	{ "controls.throw_grenade", _config_string, "\"Mouse Right, G\"", "HALO_KEY_THROW_GRENADE", _environment_value,
		_platform_all,
		"Throwing a grenade." },
	{ "controls.melee", _config_string, "\"F, Mouse 4\"", "HALO_KEY_MELEE", _environment_value, _platform_all,
		"Melee attack." },
	{ "controls.reload", _config_string, "\"R\"", "HALO_KEY_RELOAD", _environment_value, _platform_all,
		"Reloading." },
	{ "controls.zoom", _config_string, "\"Z, Mouse Middle\"", "HALO_KEY_ZOOM", _environment_value, _platform_all,
		"Zooming the scope." },
	{ "controls.switch_weapon", _config_string, "\"Wheel, 1\"", "HALO_KEY_SWITCH_WEAPON", _environment_value,
		_platform_all,
		"Switching weapons." },
	{ "controls.switch_grenade", _config_string, "\"X\"", "HALO_KEY_SWITCH_GRENADE", _environment_value, _platform_all,
		"Switching grenades." },
	{ "controls.action", _config_string, "\"E\"", "HALO_KEY_ACTION", _environment_value, _platform_all,
		"The action: picking up (held: swapping weapons), entering and leaving\n"
		"vehicles, pressing switches; never reloading (the controller's X does\n"
		"when there is nothing to act on)." },
	{ "controls.flashlight", _config_string, "\"Q\"", "HALO_KEY_FLASHLIGHT", _environment_value, _platform_all,
		"The flashlight." },
	{ "controls.scoreboard", _config_string, "\"Tab\"", "HALO_KEY_SCOREBOARD", _environment_value, _platform_all,
		"Showing the scores (the controller's Back)." },
	{ "controls.pause", _config_string, "\"Escape\"", "HALO_KEY_PAUSE", _environment_value, _platform_all,
		"The pause menu (the controller's Start)." },
	{ "controls.screenshot", _config_string, "\"F10\"", "HALO_KEY_SCREENSHOT", _environment_value, _platform_all,
		"Save a PNG screenshot beside maps/ (press once per capture)." },
	{ "controls.push_to_talk", _config_string, "\"V\"", "HALO_KEY_PUSH_TO_TALK", _environment_value, _platform_all,
		"Voice chat: talk while it is held (audio.voice_chat \"push_to_talk\")." },
	{ "controls.chat", _config_string, "\"T\"", "HALO_KEY_CHAT", _environment_value, _platform_all,
		"Network games' text chat: type a line to everyone (Enter sends it,\n"
		"Escape lets it go)." },
	{ "controls.team_chat", _config_string, "\"Y\"", "HALO_KEY_TEAM_CHAT", _environment_value, _platform_all,
		"Network games' text chat: type a line to your team." },
	{ "controls.vote_yes", _config_string, "\"F1\"", "HALO_KEY_VOTE_YES", _environment_value, _platform_all,
		"Network games' votes (to kick a player, to end the game): vote yes in\n"
		"the vote running, whose box shows at the left." },
	{ "controls.vote_no", _config_string, "\"F2\"", "HALO_KEY_VOTE_NO", _environment_value, _platform_all,
		"Network games' votes: vote no in the vote running." },

	{ "game.console_log", _config_string, "\"none\"", "HALO_CONSOLE_LOG", _environment_value, _platform_all,
		"What the game's console shows on screen of what it logs: \"important\"\n"
		"(bans, players dropped for cheating, what refuses a command, and the\n"
		"asserts that stop the game), \"all\" (every line, the game's own\n"
		"chatter too), or \"none\" (the asserts that stop the game only). What\n"
		"a command prints shows whatever this is, and debug.txt has every line." },

	{ "game.language", _config_string, "\"\"", "HALO_LANGUAGE", _environment_value, _platform_all,
		"The language the game asks the Xbox for: \"ja\", \"de\", \"fr\", \"es\" or \"it\";\n"
		"empty for English. The game data decides what is translated." },
	{ "display.text_chat", _config_boolean, "true", "HALO_TEXT_CHAT", _environment_value, _platform_all,
		"Show the text chat of network games: false hides its lines and keeps\n"
		"its keys (and a controller's D-pad Up) from opening it, as for purists\n"
		"and streamers." },
	{ "display.chat_style", _config_string, "\"colors\"", "HALO_CHAT_STYLE", _environment_value, _platform_all,
		"The text chat's colours: \"colors\" (names in their team's colour, the\n"
		"text a pale blue) or \"white\" (all white, as Custom Edition)." },
	{ "game.enhanced_animations", _config_boolean, "true", "HALO_ENHANCED_ANIMATIONS", _environment_value, _platform_all,
		"The player bipeds' grenade throws keep their legs moving (crouched,\n"
		"in the air and in a vehicle's seat too), riders' hands leave the grips\n"
		"to throw and reload, and a player turns with the aim while throwing\n"
		"and in the air (so shots after a turn in a jump go to the crosshair);\n"
		"false: the original animations, which freeze the legs and stand a\n"
		"rider up (the turning, which moves where shots go, stays on in a\n"
		"network game for every player)." },
	{ "game.custom_edition", _config_boolean, "true", "HALO_CUSTOM_EDITION", _environment_value, _platform_all,
		"Load and run Halo Custom Edition maps (not those that need OpenSauce):\n"
		"put them and Custom Edition's bitmaps.map, sounds.map and loc.map in\n"
		"the custom_maps folder beside the maps folder; the map lists show them\n"
		"as CUSTOM SINGLEPLAYER and CUSTOM MULTIPLAYER. Their tags are checked\n"
		"as the game's own maps' are before they run; false refuses them\n"
		"(docs/custom_edition_caches.md)." },
	{ "bots.count", _config_integer, "0", "HALO_BOTS", _environment_value, _platform_all,
		"Computer players (bots) in a local (split screen) Slayer game: 0 to 15\n"
		"join its lobby with the players, as far as the game has room. Not in\n"
		"system link or internet games. 0: none, as in the stock game." },
	{ "bots.skill", _config_string, "\"normal\"", "HALO_BOT_SKILL", _environment_value, _platform_all,
		"How well the bots play: \"recruit\", \"easy\", \"normal\", \"heroic\",\n"
		"\"veteran\" or \"legendary\" (how soon they react, how well they aim,\n"
		"how far they see, and how they fight: blows, grenades, strafes, retreats)." },
	{ "bots.teams", _config_string, "\"even\"", "HALO_BOT_TEAMS", _environment_value, _platform_all,
		"In team games: \"even\" puts the bots on both teams, evening them with\n"
		"the players; \"against\" puts them all on the team the players are not on." },
	{ "display.viewmodel_shield", _config_boolean, "false", "HALO_VIEWMODEL_SHIELD", _environment_value, _platform_all,
		"The energy shield's flare on the first-person arms too, as on the body\n"
		"(the stock game draws it on the body only)." },
	{ "display.first_person_legs", _config_boolean, "false", "HALO_FIRST_PERSON_LEGS", _environment_value, _platform_all,
		"Your own legs in first person, seen looking down: your body as others\n"
		"see it, drawn from the waist down, moving as it moves." },
	{ "display.overshield_effect", _config_boolean, "false", "HALO_OVERSHIELD_EFFECT", _environment_value, _platform_all,
		"The overshield as this machine draws it: its flare rising up the body as\n"
		"it charges (2 seconds, 3 from a shield that was down) and staying lit,\n"
		"its energy slower, in its own color (the powerup's), on the first-person\n"
		"arms too with viewmodel_shield; a hit on it plays what the projectile\n"
		"plays on an Elite's shield. Only what is drawn and heard changes.\n"
		"In a network game, the host's network.shield_effects decides, unless\n"
		"it is \"players\" (each player's own) or the game is co-op." },
	{ "display.shield_glow", _config_boolean, "false", "HALO_SHIELD_GLOW", _environment_value, _platform_all,
		"Energy shields light what is around them as they flare, as plasma\n"
		"does, in the shield's own color. In a network game, the host's\n"
		"network.shield_effects decides, unless it is \"players\" (each\n"
		"player's own) or the game is co-op." },
	{ "display.shield_glow_intensity", _config_string, "\"default\"", "HALO_SHIELD_GLOW_INTENSITY", _environment_value, _platform_all,
		"How far the shield glow reaches: \"default\" or \"light_show\" (three\n"
		"times as far, and at full strength from a third of the flare).\n"
		"In a network game, the host's network.shield_effects decides, unless\n"
		"it is \"players\" (each player's own) or the game is co-op." },
	{ "display.shield_color", _config_string, "\"default\"", "HALO_SHIELD_COLOR", _environment_value, _platform_all,
		"Your energy shield's color, as this machine draws it: \"default\" (the\n"
		"shield's own) or a multiplayer armor color: white, black, red, blue,\n"
		"gray, yellow, green, pink, purple, cyan, cobalt, orange, teal, sage,\n"
		"brown, tan, maroon or salmon." },

	{ "game.unlock_all_levels", _config_boolean, "false", "HALO_UNLOCK_ALL_LEVELS", _environment_value, _platform_all,
		"true: a profile made from now on starts with every campaign level\n"
		"finished on every difficulty, so any level can be played. false: a new\n"
		"profile starts at The Pillar of Autumn and opens each level as the one\n"
		"before it is finished, as on the Xbox. Profiles already made keep their\n"
		"progress; the console's profile_reset_campaign starts one over." },

	{ "game.downloaded_maps", _config_string, "\"\"", "HALO_DOWNLOADED_MAPS", _environment_value, _platform_all,
		"Maps played as downloaded ones, besides those map sharing downloaded:\n"
		"their names (bloodgulch, hugeass), separated by commas, or \"*\" for\n"
		"every map. A downloaded map's scripts may not change the player's\n"
		"settings or other players' games (debug.txt names what they were\n"
		"refused)." },

	{ "paths.data", _config_string, "\"\"", "HALO_DATA_ROOT", _environment_value, _platform_desktop,
		"The folder holding the game data's maps folder; empty looks in the\n"
		"working directory and its assets folder. Windows paths are easiest in\n"
		"single quotes: 'C:\\Games\\Halo'." },
	{ "paths.saves", _config_string, "\"\"", "HALO_SAVE_ROOT", _environment_value, _platform_desktop,
		"Where saved games and profiles go; empty for the usual place\n"
		"(~/.local/share/halo-linux, or %APPDATA%\\halo on Windows)." },
	{ "paths.custom_edition", _config_string, "\"\"", "HALO_CUSTOM_EDITION_ROOT", _environment_value, _platform_desktop,
		"A Halo Custom Edition install whose maps folder is looked in after the\n"
		"custom_maps folder for Custom Edition maps and their bitmaps.map,\n"
		"sounds.map and loc.map (game.custom_edition); empty for none." },
	{ "paths.logs", _config_string, "\"\"", "HALO_LOG_ROOT", _environment_value, _platform_desktop,
		"Where debug.txt, bans.txt and cheaters.txt go; empty for the data\n"
		"folder. Lets the data folder be read-only (a dedicated server's maps\n"
		"mounted so)." },
	{ "paths.custom_maps", _config_string, "\"\"", "HALO_CUSTOM_MAPS_ROOT", _environment_value, _platform_desktop,
		"The custom_maps folder (Custom Edition maps, and their bitmaps.map,\n"
		"sounds.map and loc.map); empty for the one in the data folder." },

	{ "network.address", _config_string, "\"\"", "HALO_NET_ADDRESS", _environment_value, _platform_all,
		"This machine's IPv4 address for system link, for a machine on several\n"
		"networks; empty chooses one." },
	{ "network.broadcast", _config_string, "\"\"", "HALO_NET_BROADCAST", _environment_value, _platform_all,
		"Comma-separated IPv4 addresses system link sends its announcements to\n"
		"instead of the local network's broadcast address (for VPNs); empty for\n"
		"the local network." },
	{ "network.online", _config_boolean, "true", "HALO_NET_ONLINE", _environment_value, _platform_all,
		"Internet play: hosting makes an invite link (logged, and put on the\n"
		"clipboard) that lets whoever has it join over the internet; opening a\n"
		"link (or copying one before switching to the game) joins. Only people\n"
		"with the invite can join. Off keeps system link to the local network." },
	{ "network.join_from_clipboard", _config_boolean, "true", "HALO_NET_JOIN_FROM_CLIPBOARD", _environment_value,
		_platform_all,
		"Join the game of an invite link found on the clipboard when the game\n"
		"comes to the front." },
	{ "network.tunnel_port", _config_integer, "0", "HALO_NET_TUNNEL_PORT", _environment_value, _platform_all,
		"The UDP port internet play uses; 0 picks one. A fixed one can be\n"
		"forwarded on the router, for networks whose NAT stops connections." },
	{ "network.client_timeout_seconds", _config_integer, "15", "HALO_NET_CLIENT_TIMEOUT_SECONDS", _environment_value,
		_platform_all,
		"Seconds a host waits for a machine it hears nothing from before it\n"
		"drops it (5 to 300; the stock game's 15). Longer keeps players whose\n"
		"connection stalls (a browser tab in the background) at the cost of\n"
		"frozen players for longer when one really goes." },
	{ "network.allow_upnp", _config_boolean, "true", "HALO_NET_ALLOW_UPNP", _environment_value, _platform_all,
		"Let internet play ask the router (UPnP) to forward its port, for\n"
		"networks whose NAT stops connections: when a player joins this\n"
		"machine's game, and when joining a game takes too long. False never\n"
		"asks." },
	{ "network.public_lobby", _config_boolean, "true", "HALO_NET_PUBLIC_LOBBY", _environment_value, _platform_all,
		"The server browser: public games are listed through the signalling\n"
		"brokers, and Join Game > Server Browser shows them. False lists no\n"
		"game of this machine's and shows none." },
	{ "network.host_public", _config_boolean, "true", "HALO_NET_HOST_PUBLIC", _environment_value, _platform_all,
		"Whether a new game of Create Game > Internet starts as PUBLIC (listed\n"
		"in everyone's server browser: anyone can see and join it) or, false,\n"
		"PRIVATE (only players with its invite link can join). Server Setup's\n"
		"LISTING changes it for each game." },
	{ "network.voice_lobby", _config_boolean, "true", "HALO_NET_VOICE_LOBBY", _environment_value, _platform_all,
		"Hosting: voice chat in the lobby, before and after a game, where every\n"
		"player hears every other." },
	{ "network.voice_mode", _config_string, "\"team_global_enemy_proximity\"", "HALO_NET_VOICE_MODE",
		_environment_value, _platform_all,
		"Hosting: voice chat in a game: \"off\"; \"team_proximity\" (teammates\n"
		"near); \"team_enemy_proximity\" (anyone near); \"team_global\" (all\n"
		"teammates); or \"team_global_enemy_proximity\" (all teammates, and\n"
		"enemies near). A game without teams has only enemies; co-op only\n"
		"teammates." },
	{ "network.voice_kbps", _config_integer, "24", "HALO_NET_VOICE_KBPS", _environment_value, _platform_all,
		"Hosting: the voices' quality, in the lobby and in a game, in kilobits a\n"
		"second (8 to 64)." },
	{ "network.voice_proximity", _config_real, "15.0", "HALO_NET_VOICE_PROXIMITY", _environment_value,
		_platform_all,
		"Hosting: how near (world units: 1 is about 3 metres) a player must be\n"
		"to be heard by proximity voice chat (5 to 100)." },
	{ "network.text_chat", _config_string, "\"on\"", "HALO_NET_TEXT_CHAT", _environment_value, _platform_all,
		"Hosting: the text chat: \"on\" (links in lines are refused), \"links\"\n"
		"(links allowed) or \"off\", in either case. Always off in a game a\n"
		"player hosts that is listed in the server browser (a public internet\n"
		"game); a dedicated server's follows this setting, listed or not." },
	{ "network.votekick", _config_boolean, "true", "HALO_NET_VOTEKICK", _environment_value, _platform_all,
		"Hosting: let the players vote to kick a player (the scoreboard's\n"
		"right-click, or the console's votekick; F1 or the D-pad's left votes\n"
		"yes, F2 or its right no). The yes votes needed are\n"
		"network.votekick_threshold's, counted once per address, and more yes\n"
		"than no." },
	{ "network.votekick_threshold", _config_string, "\"dynamic\"", "HALO_NET_VOTEKICK_THRESHOLD", _environment_value,
		_platform_all,
		"Hosting: the yes votes a vote (to kick, to end the game) needs, of the\n"
		"players who may vote: \"dynamic\", scaled with the players (more than\n"
		"half up to 8, 40% up to 32, 30% beyond); \"static\",\n"
		"network.votekick_percent of them. At least 2 either way." },
	{ "network.votekick_percent", _config_integer, "50", "HALO_NET_VOTEKICK_PERCENT", _environment_value,
		_platform_all,
		"Hosting: with network.votekick_threshold \"static\", the percentage of\n"
		"the players who may vote whose yes passes a vote (1 to 100, rounded\n"
		"up; a kick's never more than those who can vote)." },
	{ "network.vote_end_game", _config_boolean, "true", "HALO_NET_VOTE_END_GAME", _environment_value, _platform_all,
		"Hosting: let the players vote to end the game (\"!endgame\" in the chat;\n"
		"F1 votes yes, F2 no), counted as a vote to kick. The game ends with its\n"
		"scores, as sv_end_game ends it." },
	{ "network.votekick_minutes", _config_integer, "5", "HALO_NET_VOTEKICK_MINUTES", _environment_value,
		_platform_all,
		"Hosting: the minutes a player must have played on this server to start\n"
		"a vote to kick (0 to 60); to vote, 2 minutes or this, the less." },
	{ "network.votekick_ban_minutes", _config_integer, "30", "HALO_NET_VOTEKICK_BAN_MINUTES", _environment_value,
		_platform_all,
		"Hosting: the minutes a player kicked by a vote cannot join again (1 to\n"
		"1440)." },
	{ "network.map_share", _config_boolean, "true", "HALO_NET_MAP_SHARE", _environment_value, _platform_all,
		"Hosting: send a player who joins without the game's Custom Edition map\n"
		"the map (from custom_maps), and the resource maps it needs that the\n"
		"player lacks (bitmaps.map, sounds.map, loc.map). The game's other\n"
		"players help (network.map_seed)." },
	{ "network.map_share_upload", _config_integer, "1024", "HALO_NET_MAP_SHARE_UPLOAD", _environment_value,
		_platform_all,
		"Hosting: the most kilobytes a second the host sends of maps (16 to\n"
		"20480: 20 MB/s), so that the game's own traffic keeps the rest of the\n"
		"upload." },
	{ "network.map_seed", _config_boolean, "true", "HALO_NET_MAP_SEED", _environment_value, _platform_all,
		"Playing in another's game: help send its map to players who join\n"
		"without it, as a torrent's peers do (the pieces this machine has, at\n"
		"most network.map_seed_upload)." },
	{ "network.map_seed_upload", _config_integer, "256", "HALO_NET_MAP_SEED_UPLOAD", _environment_value,
		_platform_all,
		"Playing in another's game: the most kilobytes a second this machine\n"
		"sends of maps to other players (16 to 10240: 10 MB/s)." },
	{ "network.map_download", _config_boolean, "true", "HALO_NET_MAP_DOWNLOAD", _environment_value, _platform_all,
		"Joining a game on a Custom Edition map this machine does not have:\n"
		"download it (and the resource maps it needs) from the game's players\n"
		"into custom_maps, if its host shares it (network.map_share); false\n"
		"leaves the game, saying which map is missing." },
	{ "network.coop_public", _config_boolean, "false", "HALO_NET_COOP_PUBLIC", _environment_value, _platform_all,
		"Whether an online co-op game (Create Game > Internet, a SINGLEPLAYER\n"
		"map) starts as PUBLIC or, false, PRIVATE: Server Setup's LISTING in\n"
		"co-op, which writes its choice here." },
	{ "network.coop_team_wipe", _config_string, "\"auto\"", "HALO_NET_COOP_TEAM_WIPE", _environment_value,
		_platform_all,
		"When every co-op player dies: \"auto\" reverts the checkpoint in a\n"
		"PRIVATE game and only respawns in a PUBLIC game; \"revert\" or\n"
		"\"respawn\" overrides that choice. A password does not make a PUBLIC\n"
		"game private. Reverting requires a saved checkpoint with the current\n"
		"players and machines; otherwise they respawn without a world reset.\n"
		"TEAM WIPE in Server Setup > Co-op Options writes its choice here." },
	{ "network.coop_friendly_fire", _config_string, "\"on\"", "HALO_NET_COOP_FRIENDLY_FIRE", _environment_value,
		_platform_all,
		"Whether the players of an online co-op game hurt each other: \"off\",\n"
		"\"on\", \"shields_only\" or \"explosives_only\" (FRIENDLY FIRE in\n"
		"co-op's Server Setup > Co-op Options writes its choice here). Their AI\n"
		"allies they hurt as network.coop_ally_friendly_fire says." },
	{ "network.coop_ally_friendly_fire", _config_boolean, "true", "HALO_NET_COOP_ALLY_FRIENDLY_FIRE",
		_environment_value, _platform_all,
		"Whether the players of an online co-op game hurt their AI allies (the\n"
		"Marines, Captain Keyes, the Sentinels while they are allies), as in the\n"
		"campaign; false, the players' weapons, grenades and vehicles do them\n"
		"no harm. ALLY FRIENDLY FIRE in co-op's Server Setup > Co-op Options\n"
		"writes its choice here." },
	{ "network.coop_own_friendly_fire", _config_boolean, "true", "HALO_NET_COOP_OWN_FRIENDLY_FIRE",
		_environment_value, _platform_all,
		"Whether this machine's players hurt, and are hurt by, the other players\n"
		"of an online co-op game: two players hurt each other only if both have\n"
		"it true. FRIENDLY FIRE in that game's pause menu writes its choice\n"
		"here. A player away from the controls for over 70 seconds is hurt by\n"
		"every player, whatever either chose." },
	{ "network.coop_player_collisions", _config_boolean, "true", "HALO_NET_COOP_PLAYER_COLLISIONS", _environment_value,
		_platform_all,
		"Whether the players of an online co-op game bump into each other;\n"
		"false, they walk through each other (the AI's characters they still\n"
		"bump into). PLAYER COLLISIONS in co-op's Server Setup > Co-op Options\n"
		"writes its choice here." },
	{ "network.coop_enemies_mode", _config_string, "\"per_player\"", "HALO_NET_COOP_ENEMIES_MODE", _environment_value,
		_platform_all,
		"Online co-op's extra enemies: \"none\", \"per_player\" (each squad of\n"
		"enemies grows by coop_enemies for each player past the first) or\n"
		"\"multiplier\" (each is coop_enemies_multiplier times as large, for any\n"
		"number of players). EXTRA ENEMIES in co-op's Server Setup > Co-op\n"
		"Options writes its choice here." },
	{ "network.coop_pickups", _config_string, "\"none\"", "HALO_NET_COOP_PICKUPS", _environment_value, _platform_all,
		"Extra campaign weapons, grenades, health kits, overshields and camouflage.\n"
		"Modes: \"none\", \"per_player\" (coop_enemies percent per extra player),\n"
		"or \"multiplier\" (coop_enemies_multiplier times as many), up to 8 times\n"
		"the original pickups. Only safe nearby places receive copies." },
	{ "network.coop_vehicles", _config_string, "\"none\"", "HALO_NET_COOP_VEHICLES", _environment_value, _platform_all,
		"Extra campaign vehicles: \"none\", \"per_player\" or \"multiplier\".\n"
		"After a player clears its unloaded starting place by both bounding\n"
		"radii plus 15 percent, make one more there, up to\n"
		"floor((players - 1) * coop_enemies * 2 / 300) extras, or floor((coop_enemies_multiplier - 1) * 2 / 3) in static mode.\n"
		"The host alone creates them, and waits while the starting place is blocked." },
	{ "network.coop_enemies", _config_integer, "50", "HALO_NET_COOP_ENEMIES", _environment_value, _platform_all,
		"Online co-op's extra enemies per player, a percentage: for each player\n"
		"past the first, each squad of enemies a level places gets this much of\n"
		"itself more (100: as many again; 25 to 200). PER PLAYER in co-op's\n"
		"Server Setup > Co-op Options writes its choice here." },
	{ "network.coop_enemies_multiplier", _config_integer, "2", "HALO_NET_COOP_ENEMIES_MULTIPLIER", _environment_value,
		_platform_all,
		"Online co-op's shared static multiplier (2 to 32). Pickups are capped\n"
		"at 8 times their original count; extra vehicle growth uses two thirds\n"
		"of (multiplier - 1), rounded down. MULTIPLIER in\n"
		"co-op's Server Setup > Co-op Options writes its choice here." },
	{ "network.brokers_file", _config_string, "\"brokers.txt\"",
		"HALO_NET_BROKERS_FILE", _environment_value, _platform_all,
		"The file of the public MQTT brokers through which the machines of an\n"
		"invite find each other (its messages are encrypted), beside this file\n"
		"unless a full path: one host:port on each line, up to 4. Updates\n"
		"replace brokers.txt: keep a list of your own under another name." },
	{ "network.public_address", _config_string, "\"\"", "HALO_NET_PUBLIC_ADDRESS", _environment_value,
		_platform_desktop,
		"Hosting: the internet address joiners reach this machine at, as\n"
		"host[:port] (the port: the one forwarded to network.tunnel_port), for a\n"
		"server behind a NAT with one forwarded port; offered first, its name\n"
		"looked up again every 5 minutes. Empty: what STUN and UPnP find." },
	{ "network.listings", _config_string, "\"both\"", "HALO_NET_LISTINGS", _environment_value, _platform_all,
		"What the server browser lists: \"both\" (dedicated servers from the\n"
		"master servers first, then the games players host, from the brokers),\n"
		"\"masters\" (the master servers' alone) or \"brokers\" (the brokers'\n"
		"alone)." },
	{ "network.masters_file", _config_string, "\"masters.txt\"", "HALO_NET_MASTERS_FILE", _environment_value,
		_platform_all,
		"The file of the master servers the server browser asks for dedicated\n"
		"servers, beside this file unless a full path: an https:// URL and its\n"
		"key (ed25519:<hex>) on each line, up to 4. None named (no file, or\n"
		"comments alone): the official master, built in." },
	{ "network.link_share", _config_string, "\"when_needed\"", "HALO_NET_LINK_SHARE", _environment_value,
		_platform_all,
		"When the game shows a server the Discord account linked to it\n"
		"(Settings > Network Setup > Discord Account): \"when_needed\" (a server\n"
		"that asks, to let you in from a VPN), \"always\" or \"never\"." },
	{ "network.browser_source", _config_string, "\"all\"", "HALO_NET_BROWSER_SOURCE", _environment_value,
		_platform_all,
		"The server browser's games: \"all\" (ALL GAMES: the dedicated\n"
		"servers first, then the games players host), \"dedicated\" (the\n"
		"master servers' dedicated servers) or \"players\" (P2P GAMES: games\n"
		"players host). Its top-left tab changes it." },
	{ "network.browser_show_empty", _config_boolean, "true", "HALO_NET_BROWSER_SHOW_EMPTY", _environment_value,
		_platform_all,
		"The server browser lists games with no player (its Filters screen)." },
	{ "network.browser_show_full", _config_boolean, "true", "HALO_NET_BROWSER_SHOW_FULL", _environment_value,
		_platform_all,
		"The server browser lists full games (its Filters screen)." },
	{ "network.browser_maps", _config_string, "\"yes\"", "HALO_NET_BROWSER_MAPS", _environment_value, _platform_all,
		"The server browser lists games on maps this machine lacks: \"yes\", \"no\",\n"
		"or \"downloadable\" (those of a Custom Edition map its host shares)." },
	{ "network.browser_gametype", _config_string, "\"any\"", "HALO_NET_BROWSER_GAMETYPE", _environment_value,
		_platform_all,
		"The server browser lists games of one kind only: \"any\", \"ctf\",\n"
		"\"slayer\", \"oddball\", \"king\" or \"race\" (its Filters screen)." },
	{ "network.browser_sort", _config_string, "\"servers\"", "HALO_NET_BROWSER_SORT", _environment_value,
		_platform_all,
		"The server browser's order: \"servers\" (dedicated servers first, then by\n"
		"players), \"players\", \"name\", \"map\", \"type\" or \"locked\" (a password's\n"
		"first); \"-\" before one reverses it (\"-players\"). A column title sorts by\n"
		"it, and again the other way." },
	{ "network.stun_servers", _config_string, "\"stun.l.google.com:19302,stun.cloudflare.com:3478\"",
		"HALO_NET_STUN", _environment_value, _platform_all,
		"Public STUN servers that tell this machine its internet address;\n"
		"comma-separated host:port." },
	{ "discord.application_id", _config_string, "\"1553978809840050229\"", "HALO_DISCORD_APPLICATION",
		_environment_value, _platform_desktop,
		"The Discord application internet play invites go through while the\n"
		"Discord desktop client runs; empty for none." },

	{ "update.auto", _config_boolean, "true", "HALO_UPDATE_AUTO", _environment_value, _platform_all,
		"Look for a new version when the game starts, and offer to update to it;\n"
		"false never looks (the game's \"Do not ask again\" writes false here)." },
	/* (a dedicated server's: halo --server, port/linux/src/server_mode.c;
	written into the file only by one) */
	{ "server.enabled", _config_boolean, "false", "HALO_SERVER", _environment_set_is_true, _platform_server,
		"Run as a dedicated server (as halo --server does): host games with no\n"
		"window, sound or player of its own, one after another from\n"
		"server.rotation (docs/dedicated_server.md)." },
	{ "server.name", _config_string, "\"OpenCE Server\"", "HALO_SERVER_NAME", _environment_value, _platform_server,
		"The game's name in the lobby and the server browser: 15 characters of\n"
		"printable ASCII." },
	{ "server.max_players", _config_integer, "16", "HALO_SERVER_MAX_PLAYERS", _environment_value, _platform_server,
		"The most players (2 to 127)." },
	{ "server.min_players", _config_integer, "1", "HALO_SERVER_MIN_PLAYERS", _environment_value, _platform_server,
		"Players in the lobby before it counts down to a game (1 to 16)." },
	{ "server.password", _config_string, "\"\"", "HALO_SERVER_PASSWORD", _environment_value, _platform_server,
		"The password the server browser asks for (the invite link still joins);\n"
		"empty for none. Never logged." },
	{ "server.public", _config_boolean, "true", "HALO_SERVER_PUBLIC", _environment_value, _platform_server,
		"List the server in everyone's server browser (on the master servers,\n"
		"with network.online); false: joined by its invite link and on the LAN." },
	{ "server.rotation", _config_string, "\"bloodgulch:ctf, sidewinder:team_slayer, prisoner:slayer\"",
		"HALO_SERVER_ROTATION", _environment_value, _platform_server,
		"The games played one after another: map[:gametype[+rule=value...]],\n"
		"separated by commas, 64 at most. A map is a multiplayer map's name\n"
		"(bloodgulch), custom_maps\\<name> or a level's path; a gametype a\n"
		"built-in's name (slayer, team_slayer, ctf...), an OpenCE gametype's\n"
		"(swat...), a gametypes/<name>.toml or file=<saved gametype>." },
	{ "server.rotation_fallback", _config_boolean, "false", "HALO_SERVER_ROTATION_FALLBACK", _environment_value,
		_platform_server,
		"With no playable rotation entry, play bloodgulch:slayer rather than\n"
		"stop (exit code 3)." },
	{ "server.lobby_seconds", _config_integer, "15", "HALO_SERVER_LOBBY_SECONDS", _environment_value,
		_platform_server,
		"Seconds the lobby counts down once min_players are in (0 to 300; 0\n"
		"starts at once)." },
	{ "server.postgame_seconds", _config_integer, "12", "HALO_SERVER_POSTGAME_SECONDS", _environment_value,
		_platform_server,
		"Seconds the scores show after a game (0 to 300)." },
	{ "server.empty_seconds", _config_integer, "30", "HALO_SERVER_EMPTY_SECONDS", _environment_value,
		_platform_server,
		"Seconds a game nobody is in goes on before it ends (0: never). A game\n"
		"whose last player leaves ends at once, as the game ends it." },
	{ "server.idle_minutes", _config_integer, "0", "HALO_SERVER_IDLE_MINUTES", _environment_value, _platform_server,
		"Minutes a game goes on with no score before it ends (0: never)." },
	{ "server.team_start", _config_string, "\"one\"", "HALO_SERVER_TEAM_START", _environment_value, _platform_server,
		"A team game: \"one\" starts with one team's players (the next joiners\n"
		"balance), \"both\" waits for a player on each team, \"skip\" skips team\n"
		"games while one player waits." },
	{ "server.motd", _config_string, "\"\"", "HALO_SERVER_MOTD", _environment_value, _platform_server,
		"A notice each machine is shown as it joins (127 characters); empty for\n"
		"none." },
	{ "server.frame_rate", _config_integer, "60", "HALO_SERVER_FRAME_RATE", _environment_value, _platform_server,
		"Frames a second (30 to 120); the game's ticks stay 30 a second. A\n"
		"server with no other machine runs 10." },
	{ "server.console", _config_boolean, "true", "HALO_SERVER_CONSOLE", _environment_value, _platform_server,
		"Take commands from the standard input." },
	{ "server.console_exit_on_eof", _config_boolean, "false", "HALO_SERVER_CONSOLE_EXIT_ON_EOF", _environment_value,
		_platform_server,
		"Stop when the standard input ends; false goes on (a panel may close\n"
		"it)." },
	{ "server.init_file", _config_string, "\"init.txt\"", "HALO_SERVER_INIT_FILE", _environment_value,
		_platform_server,
		"Commands run once at the start, one a line: in the data folder unless a\n"
		"full path; empty for none." },
	{ "server.web_port", _config_integer, "8080", "HALO_SERVER_WEB_PORT", _environment_value, _platform_server,
		"The TCP port of the web page, its API and the remote console (RCON);\n"
		"0 for none." },
	{ "server.web_bind", _config_string, "\"127.0.0.1\"", "HALO_SERVER_WEB_BIND", _environment_value,
		_platform_server,
		"The address the web port listens on. Any but 127.0.0.1 needs\n"
		"web_password (behind a reverse proxy with TLS)." },
	{ "server.web_password", _config_string, "\"\"", "HALO_SERVER_WEB_PASSWORD", _environment_value,
		_platform_server,
		"The web page's admin password (8 characters or more). Empty: one is\n"
		"made at the first start, printed once and written here." },
	{ "server.web_owner_password", _config_string, "\"\"", "HALO_SERVER_WEB_OWNER_PASSWORD", _environment_value,
		_platform_server,
		"A password with the owner role (quit, sv_restart, sv_stop, the owner's\n"
		"settings, the game's console), for a supervisor on this machine: it\n"
		"signs in from 127.0.0.0/8 alone. 8 characters or more; empty for none." },
	{ "server.web_moderator_password", _config_string, "\"\"", "HALO_SERVER_WEB_MODERATOR_PASSWORD",
		_environment_value, _platform_server,
		"A second password, with the moderator role (kick, mute, say, the next\n"
		"map, bans of a day at most); empty for none." },
	{ "server.web_viewer_password", _config_string, "\"\"", "HALO_SERVER_WEB_VIEWER_PASSWORD", _environment_value,
		_platform_server,
		"A password with the viewer role (the status, the players, the log: no\n"
		"changes); empty for none." },
	{ "server.web_ui", _config_boolean, "true", "HALO_SERVER_WEB_UI", _environment_value, _platform_server,
		"Serve the admin page on the web port; false: its API and RCON only." },
	{ "server.web_trusted_proxy", _config_string, "\"\"", "HALO_SERVER_WEB_TRUSTED_PROXY", _environment_value,
		_platform_server,
		"The address of a reverse proxy in front of the web port, whose\n"
		"X-Forwarded-For (its last entry, the proxy's), -Host and -Proto are\n"
		"believed; empty for none." },
	{ "server.metrics", _config_boolean, "true", "HALO_SERVER_METRICS", _environment_value, _platform_server,
		"Serve /metrics (Prometheus) on the web port without a password." },
	{ "server.log_format", _config_string, "\"text\"", "HALO_SERVER_LOG_FORMAT", _environment_value,
		_platform_server,
		"The standard output's lines: \"text\" or \"json\"." },
	{ "server.log_addresses", _config_boolean, "false", "HALO_SERVER_LOG_ADDRESSES", _environment_value,
		_platform_server,
		"Players' internet addresses and hardware ids in the log, the events and\n"
		"the API for every role (the owner's console has them always)." },
	{ "server.log_debug_echo", _config_boolean, "false", "HALO_SERVER_LOG_DEBUG_ECHO", _environment_value,
		_platform_server,
		"Also write debug.txt's lines to the standard error." },
	{ "server.status_file", _config_string, "\"\"", "HALO_SERVER_STATUS_FILE", _environment_value,
		_platform_server,
		"A file the status (as /api/status) is written to every 2 seconds, for a\n"
		"panel without HTTP; empty for none." },
	{ "server.auto_update", _config_string, "\"install\"", "HALO_SERVER_AUTO_UPDATE", _environment_value,
		_platform_server,
		"\"install\" installs a newer release at once, on a thread of its own,\n"
		"and stops (exit code 5) to be started again: once the server has been\n"
		"empty for 10 minutes, or, for a new network version, once the game\n"
		"being played is over (server.update_game_minutes at most). \"check\"\n"
		"logs a newer release; \"off\" looks for none. A server whose files it\n"
		"may not write (a container's image) is \"check\"." },
	{ "server.update_check_minutes", _config_integer, "30", "HALO_SERVER_UPDATE_CHECK_MINUTES", _environment_value,
		_platform_server,
		"Minutes between looks for a newer release, with server.auto_update\n"
		"(5 to 1440; a tenth either way at random)." },
	{ "server.update_game_minutes", _config_integer, "30", "HALO_SERVER_UPDATE_GAME_MINUTES", _environment_value,
		_platform_server,
		"A new network version installed: the minutes a game may go on before\n"
		"it is ended (its scores and postgame shown) and the server restarts\n"
		"(0: it waits for the game's end)." },
	{ "server.upnp", _config_boolean, "false", "HALO_SERVER_UPNP", _environment_value, _platform_server,
		"Ask the router (UPnP) to forward the tunnel's port (network.allow_upnp,\n"
		"for a server at home)." },
	{ "server.coop_difficulty", _config_string, "\"normal\"", "HALO_SERVER_COOP_DIFFICULTY", _environment_value,
		_platform_server,
		"Co-op rotation entries' (<level>:coop) difficulty: \"easy\", \"normal\",\n"
		"\"hard\" or \"impossible\"." },
	{ "server.list_on_brokers", _config_boolean, "false", "HALO_SERVER_LIST_ON_BROKERS", _environment_value,
		_platform_server,
		"Also list the server among the games players host (through the\n"
		"signalling brokers), for server browsers with no master server." },
	{ "server.master_publish", _config_boolean, "true", "HALO_SERVER_MASTER_PUBLISH", _environment_value,
		_platform_server,
		"Publish a public server to the official master servers, signed with\n"
		"the publisher key (publisher_key_file)." },
	{ "server.masters_file", _config_string, "\"masters.txt\"", "HALO_SERVER_MASTERS_FILE", _environment_value,
		_platform_server,
		"The master servers published to, beside this file unless a full path:\n"
		"an https:// URL and its key (ed25519:<hex>) on each line. None named:\n"
		"the official master, built in." },
	{ "server.publisher_key_file", _config_string, "\"publisher.key\"", "HALO_SERVER_PUBLISHER_KEY_FILE",
		_environment_value, _platform_server,
		"The operator's Ed25519 key for the master servers (halo --server\n"
		"--keygen makes it), beside this file unless a full path.\n"
		"HALO_SERVER_PUBLISHER_KEY holds the key itself for containers." },
	{ "server.instance", _config_string, "\"\"", "HALO_SERVER_INSTANCE", _environment_value, _platform_server,
		"A short name telling this server from the operator's others on the\n"
		"master servers (letters, digits, - and _); empty: from server.name." },
	{ "server.region", _config_string, "\"\"", "HALO_SERVER_REGION", _environment_value, _platform_server,
		"A region shown in the server browser (eu, na-east...): 16 characters;\n"
		"empty for none." },
	{ "server.chat_filter", _config_string, "\"off\"", "HALO_SERVER_CHAT_FILTER", _environment_value, _platform_server,
		"The words the server keeps out of its chat and its players' names\n"
		"(streamer mode's list, by chat_filter_slurs, _swearing and _insults):\n"
		"\"off\"; \"censor\", a line's words shown as stars; \"block\", a line\n"
		"with them refused (its sender is told why) and a player whose name has\n"
		"them refused at the door. Links are network.text_chat's." },
	{ "server.chat_filter_slurs", _config_string, "\"mild\"", "HALO_SERVER_CHAT_FILTER_SLURS", _environment_value,
		_platform_server,
		"With server.chat_filter, the slurs (race, sexuality, gender, disability)\n"
		"kept out: \"off\", \"severe\" (the worst), \"strong\" or \"mild\" (all)." },
	{ "server.chat_filter_swearing", _config_string, "\"off\"", "HALO_SERVER_CHAT_FILTER_SWEARING",
		_environment_value, _platform_server,
		"With server.chat_filter, the swearing and sexual words kept out: \"off\",\n"
		"\"severe\", \"strong\" or \"mild\" (all)." },
	{ "server.chat_filter_insults", _config_string, "\"off\"", "HALO_SERVER_CHAT_FILTER_INSULTS",
		_environment_value, _platform_server,
		"With server.chat_filter, the insults that are not slurs kept out: \"off\",\n"
		"\"severe\", \"strong\" or \"mild\" (all)." },
	{ "server.master_publish_address", _config_boolean, "false", "HALO_SERVER_MASTER_PUBLISH_ADDRESS",
		_environment_value, _platform_server,
		"Let the master servers give this server's address to players; false:\n"
		"only their reachability probe uses it." },
	{ "server.vpn_policy", _config_string, "\"link\"", "HALO_SERVER_VPN_POLICY", _environment_value, _platform_server,
		"Machines joining from the address of a VPN, a Tor exit or a datacenter\n"
		"(the master servers' netlist): \"link\", refused unless the player shows\n"
		"a Discord account linked at a master server this server publishes to;\n"
		"\"block\", refused; \"off\", let in." },
	{ "server.vpn_fail", _config_string, "\"open\"", "HALO_SERVER_VPN_FAIL", _environment_value, _platform_server,
		"Until a netlist is held: \"open\" lets every machine in, \"closed\"\n"
		"refuses those from the internet as server.vpn_policy \"block\" would." },
	{ "server.vpn_list", _config_string, "\"\"", "HALO_SERVER_VPN_LIST", _environment_value, _platform_server,
		"A netlist file checked besides the master servers' (beside this file\n"
		"unless a full path): for a server that publishes nowhere, or a test.\n"
		"Read at the start and by sv_reload." },
	{ "server.vpn_check_lan", _config_boolean, "false", "HALO_SERVER_VPN_CHECK_LAN", _environment_value,
		_platform_server,
		"Check the LAN's machines' addresses against the netlist too (tests)." },
	{ "server.link_min_age_days", _config_integer, "365", "HALO_SERVER_LINK_MIN_AGE_DAYS", _environment_value,
		_platform_server,
		"A machine joining over a VPN with a linked Discord account is refused\n"
		"when the account is younger than this many days (0: any age)." },
	{ "server.link_one_seat", _config_boolean, "true", "HALO_SERVER_LINK_ONE_SEAT", _environment_value,
		_platform_server,
		"Refuse a second machine of a linked Discord account already in the game." },
	{ "server.max_machines_per_address", _config_integer, "0", "HALO_SERVER_MAX_MACHINES_PER_ADDRESS",
		_environment_value, _platform_server,
		"The most machines that play from one address (an internet play peer's\n"
		"real one); one more is refused (0: no limit). Households, LAN cafes\n"
		"and mobile carriers share one address among many players." },
	{ "server.afk_kick_minutes", _config_integer, "5", "HALO_SERVER_AFK_KICK_MINUTES", _environment_value,
		_platform_server,
		"Minutes a player may stand idle in a game (no move, look or button)\n"
		"before the server kicks them, told a minute before (0: never). The\n"
		"lobby, the postgame, cutscenes and waiting to respawn do not count." },
	{ "server.afk_kick_when_full", _config_boolean, "false", "HALO_SERVER_AFK_KICK_WHEN_FULL", _environment_value,
		_platform_server,
		"Kick idle players only when the server is full, after\n"
		"server.afk_kick_full_minutes (server.afk_kick_minutes is not used)." },
	{ "server.afk_kick_full_minutes", _config_integer, "3", "HALO_SERVER_AFK_KICK_FULL_MINUTES", _environment_value,
		_platform_server,
		"With server.afk_kick_when_full, the minutes a player may stand idle while\n"
		"the server is full (told a minute before)." },
	{ "server.coop_bsp_switch", _config_string, "\"majority\"", "HALO_SERVER_COOP_BSP_SWITCH", _environment_value,
		_platform_server,
		"A co-op game's loading zones: \"majority\", a zone switches the level's\n"
		"part only once more than half of the living players who are not idle\n"
		"stand in zones to the same part (the others are told how many more it\n"
		"needs); \"anyone\", any player's crossing brings everyone. The\n"
		"mission's own switches are never held." },
	{ "server.coop_bsp_switch_cooldown_seconds", _config_integer, "15", "HALO_SERVER_COOP_BSP_SWITCH_COOLDOWN_SECONDS",
		_environment_value, _platform_server,
		"Seconds after a co-op loading zone switched the level's part before\n"
		"another may (0 to 300)." },
	{ "server.map_vote", _config_boolean, "false", "HALO_SERVER_MAP_VOTE", _environment_value, _platform_server,
		"As the lobby opens after a game, the players vote for the next among\n"
		"server.map_vote_choices entries of the rotation picked at random (not\n"
		"the one just played); the rotation goes on after the winner." },
	{ "server.map_vote_choices", _config_integer, "4", "HALO_SERVER_MAP_VOTE_CHOICES", _environment_value,
		_platform_server,
		"The entries a map vote offers (2 to 8)." },
	{ "server.map_vote_seconds", _config_integer, "20", "HALO_SERVER_MAP_VOTE_SECONDS", _environment_value,
		_platform_server,
		"How long a map vote runs (5 to 120 seconds); it ends sooner once every\n"
		"player has voted." },
	{ "streamer.enabled", _config_boolean, "false", "HALO_STREAMER_MODE", _environment_value, _platform_all,
		"Hide offensive words in names and text from other players, maps and mods\n"
		"as stars, at the levels below (Settings > Network Setup > Streamer Mode)." },
	{ "streamer.slurs", _config_string, "\"mild\"", "HALO_STREAMER_SLURS", _environment_value, _platform_all,
		"With streamer.enabled, the slurs (race, sexuality, gender, disability)\n"
		"hidden: \"off\", \"severe\", \"strong\" (and severe) or \"mild\" (all)." },
	{ "streamer.swearing", _config_string, "\"strong\"", "HALO_STREAMER_SWEARING", _environment_value, _platform_all,
		"With streamer.enabled, the swearing and sexual words hidden: \"off\",\n"
		"\"severe\", \"strong\" (and severe) or \"mild\" (all)." },
	{ "streamer.insults", _config_string, "\"off\"", "HALO_STREAMER_INSULTS", _environment_value, _platform_all,
		"With streamer.enabled, the insults that are not slurs hidden: \"off\",\n"
		"\"severe\", \"strong\" (and severe) or \"mild\" (all)." },
	{ "crash_reports.upload", _config_string, "\"ask\"", "HALO_CRASH_REPORTS", _environment_value, _platform_windows,
		"Send a report of each crash (a minidump and halo.log) to the developers'\n"
		"Sentry project (port/windows/src/win32_crash.c): \"yes\" sends them, \"no\"\n"
		"never does, \"ask\" asks at the next crash and writes the answer here." },

	{ "debug.network_test", _config_string, "\"\"", "HALO_NETWORK_TEST", _environment_value, _platform_all,
		"Automated system link sessions for testing (port/linux/game/network_test.c):\n"
		"\"host:<map>\" hosts a game on that map, \"local:<map>\" plays a local (split\n"
		"screen) game on it, \"join\" joins the first game found,\n"
		"\"browse\" the one with the most players the server browser lists (not\n"
		"co-op, if it can), \"log\" logs the game that runs (and runs its cues);\n"
		"empty for none." },
	{ "debug.network_test_start", _config_real, "15.0", "HALO_NETWORK_TEST_START", _environment_value, _platform_all,
		"Seconds after hosting that an automated test game starts." },
	{ "debug.network_test_kill", _config_real, "0.0", "HALO_NETWORK_TEST_KILL", _environment_value, _platform_all,
		"Every this many seconds an automated test host kills its last player; 0 never." },
	{ "debug.network_test_score", _config_integer, "0", "HALO_NETWORK_TEST_SCORE", _environment_value, _platform_all,
		"The score an automated test host's game type plays to (a short game, to\n"
		"test the next); 0 the game type's own." },
	{ "debug.network_test_shoot", _config_real, "0.0", "HALO_NETWORK_TEST_SHOOT", _environment_value, _platform_all,
		"Every this many seconds each automated test player hits the next with\n"
		"their weapon, within its reach (the host brings far players near the\n"
		"first a second before); 0 never." },
	{ "debug.network_test_vehicle", _config_real, "0.0", "HALO_NETWORK_TEST_VEHICLE", _environment_value, _platform_all,
		"This many seconds into an automated test game the host seats its last\n"
		"player as a vehicle's driver (and out 15 seconds on); 0 never." },
	{ "debug.network_test_pickup", _config_real, "0.0", "HALO_NETWORK_TEST_PICKUP", _environment_value, _platform_all,
		"This many seconds into an automated test game the host stands its last\n"
		"player on a weapon, which a joining player then picks up; 0 never." },
	{ "debug.network_test_chat", _config_real, "0.0", "HALO_NETWORK_TEST_CHAT", _environment_value, _platform_all,
		"This many seconds into an automated test game each machine's player\n"
		"says a chat line to everyone, then to the team and to the vehicle (the\n"
		"host seats the last two players in one first), then the host mutes and\n"
		"unmutes a player, refuses a link and turns the chat off, and the\n"
		"clients type on the on-screen keyboard (port/linux/NETCODE.md); 0 never." },
	{ "debug.network_test_name", _config_string, "\"\"", "HALO_NETWORK_TEST_NAME", _environment_value, _platform_all,
		"The name this machine's player takes in an automated test game (ASCII);\n"
		"empty keeps its profile's." },
	{ "debug.network_test_pickup_weapon", _config_string, "\"\"", "HALO_NETWORK_TEST_PICKUP_WEAPON", _environment_value,
		_platform_all,
		"The weapon network_test_pickup stands the player on: the first whose tag\n"
		"name has this in it (\"sniper\", say); empty any." },
	{ "debug.network_test_cues", _config_string, "\"\"", "HALO_NETWORK_TEST_CUES", _environment_value, _platform_all,
		"Console lines an automated test runs at a game second, from the first tick\n"
		"every player of debug.network_test_cue_players has a unit:\n"
		"\"<second> <line>|<second> <line>...\" (the host's, or a game of one machine;\n"
		"not on the web)." },
	{ "debug.network_test_cue_players", _config_integer, "1", "HALO_NETWORK_TEST_CUE_PLAYERS", _environment_value,
		_platform_all,
		"The players debug.network_test_cues waits for." },
	{ "debug.network_test_public", _config_boolean, "false", "HALO_NETWORK_TEST_PUBLIC", _environment_value,
		_platform_all,
		"The network test's host lists its game in the server browser, as Create\n"
		"Game > Internet's PUBLIC does (debug.network_test \"browse\" joins it)." },
	{ "debug.telnet_console", _config_boolean, "false", "HALO_TELNET_CONSOLE", _environment_set_is_true, _platform_all,
		"Listen on 127.0.0.1 (port telnet_console_port) for a script console that\n"
		"runs what it is sent as the game's console does, with no password; false\n"
		"none." },
	{ "debug.telnet_console_port", _config_integer, "2323", "HALO_TELNET_CONSOLE_PORT", _environment_value,
		_platform_all,
		"The port of the script console (telnet_console); the Xbox's was 23, which\n"
		"only the administrator can listen on." },
	{ "debug.touch_targets", _config_boolean, "false", "HALO_TOUCH_TARGETS", _environment_set_is_true, _platform_all,
		"Outline the menus' tap targets (item green, value blue, list slot yellow,\n"
		"legend button red, the band beside a list's slots orange; the virtual\n"
		"keyboard's keys white), mark where the last finger went down and the\n"
		"last tap landed for 3 seconds, and log each tap with the target it hit\n"
		"(and a value's split); to judge touch accuracy." },
	{ "debug.log_addresses", _config_boolean, "false", "HALO_LOG_ADDRESSES", _environment_set_is_true, _platform_all,
		"Write other machines' internet addresses whole in debug.txt and the\n"
		"console, for debugging your own network. False writes each public\n"
		"address as a tag (addr#3f2a9c) that only matches up within one run;\n"
		"local network addresses are always whole. Do not post a debug.txt\n"
		"written with this on: it has the address of everyone you played with." },
	{ "debug.network_latency", _config_real, "0.0", "HALO_NETWORK_LATENCY", _environment_value, _platform_all,
		"Milliseconds everything received is held back (a round trip between two\n"
		"machines of twice it), to test the netcode as over the internet; 0 none." },
	{ "debug.network_loss", _config_real, "0.0", "HALO_NETWORK_LOSS", _environment_value, _platform_all,
		"Percent of datagrams received that are dropped, for the same; 0 none." },
	{ "debug.network_corrupt", _config_real, "0.0", "HALO_NETWORK_CORRUPT", _environment_value, _platform_all,
		"Percent of the datagrams received that are damaged at random, to test\n"
		"that nothing a machine sends can crash the game; 0 none." },
	{ "debug.network_corrupt_stream", _config_real, "0.0", "HALO_NETWORK_CORRUPT_STREAM", _environment_value,
		_platform_all,
		"Percent of the reads of streams that are damaged at random, for the\n"
		"same (a damaged stream is closed, so a little goes a long way); 0 none." },
	{ "debug.network_corrupt_after", _config_real, "0.0", "HALO_NETWORK_CORRUPT_AFTER", _environment_value,
		_platform_all,
		"Seconds after the start before anything is damaged, so that a game can\n"
		"be set up and started first (a host's messages to its own client are\n"
		"damaged too)." },
	{ "debug.test_input", _config_string, "\"\"", "HALO_TEST_INPUT", _environment_value, _platform_all,
		"\"bot:<seed>\" plays controller 1 with a scripted pattern (automated\n"
		"network tests); \"look:<seed>\" stands still, only turning and looking\n"
		"up and down; \"menu:<buttons>\" presses menu buttons one a second (a,\n"
		"b, x, y, up, down, left, right, start, back or wait, separated by\n"
		"spaces or commas), for testing the menus; empty for none." },
	{ "debug.virtual_gamepads", _config_integer, "0", "HALO_VIRTUAL_GAMEPADS", _environment_value, _platform_desktop,
		"Attach this many SDL virtual gamepads (0 to 4), which the game takes for\n"
		"gamepads plugged in, for testing split screen; debug.virtual_gamepad_script\n"
		"drives them." },
	{ "debug.virtual_gamepad_script", _config_string, "\"\"", "HALO_VIRTUAL_GAMEPAD_SCRIPT", _environment_value,
		_platform_desktop,
		"What debug.virtual_gamepads press: \"<seconds> <pad> <what>; ...\", the\n"
		"seconds from when the network lobby first opens, the pad 1 to 4 (0: the\n"
		"keyboard, a SPACE and start Enter), and a button (a, b, x, y, start, back,\n"
		"up, down, left, right, lb, rb, lt, rt: pressed for 150 ms),\n"
		"\"walk\" (the sticks move from then on) or \"still\"." },
	{ "debug.bot_trace", _config_integer, "0", "HALO_BOT_TRACE", _environment_value, _platform_all,
		"Log each bot's state (bots.count) every this many seconds: where it is\n"
		"and goes, its path, target and weapon; 0 never." },
	{ "debug.update_answer", _config_string, "\"\"", "HALO_UPDATE_ANSWER", _environment_value, _platform_desktop,
		"The answer to the new version question, for automated tests: \"yes\",\n"
		"\"no\" or \"never\" (do not ask again, confirmed); empty asks." },
	{ "debug.fixed_step", _config_boolean, "false", "HALO_FIXED_STEP", _environment_set_is_true, _platform_all,
		"A game of this machine alone runs one tick a frame, whatever time has\n"
		"passed, so that it plays the same every time (tests); game_speed has no\n"
		"effect." },
	{ "debug.exit_after", _config_real, "0.0", "HALO_EXIT_AFTER", _environment_value, _platform_all,
		"Quit this many seconds after the window opens (without a window, after\n"
		"the first frame); 0 never." },
	{ "debug.hidden_window", _config_boolean, "false", "HALO_HIDDEN_WINDOW", _environment_set_is_true, _platform_desktop,
		"Keep the window hidden (and never fullscreen)." },
	{ "debug.voice_test", _config_boolean, "false", "HALO_VOICE_TEST", _environment_set_is_true, _platform_all,
		"Voice chat's automated tests: a tone instead of the microphone, and\n"
		"each voice heard logged once a second." },
	{ "debug.null_renderer", _config_boolean, "false", "HALO_NULL_RENDERER", _environment_set_is_true, _platform_all,
		"Run without a window, drawing nothing." },
	{ "debug.gl_debug", _config_boolean, "false", "HALO_GL_DEBUG", _environment_set_is_true, _platform_all,
		"Report OpenGL errors in the log." },
	{ "debug.menu_open", _config_string, "\"\"", "HALO_MENU_OPEN", _environment_value, _platform_all,
		"Start on this screen of the menus (port/assets/menus) instead of the main\n"
		"menu, a player profile being edited; empty for the main menu." },
	{ "debug.gpu_flush_draws", _config_integer, "-1", "HALO_GPU_FLUSH_DRAWS", _environment_value, _platform_desktop,
		"Flush the GPU's pipeline every this many draws: -1 for every 3 on Intel\n"
		"graphics with Mesa's driver (which can hang without), 0 never." },
	{ "debug.gpu_stats", _config_boolean, "false", "HALO_GPU_STATS", _environment_set_is_true, _platform_all,
		"Log the renderer's draw counts once a second." },
	{ "debug.gpu_trace_frame", _config_integer, "-1", "HALO_GPU_TRACE", _environment_value, _platform_all,
		"Log every draw of this frame; -1 none." },
	{ "debug.gpu_trace_constants", _config_boolean, "false", "HALO_GPU_TRACE_CONSTANTS", _environment_set_is_true, _platform_all,
		"With gpu_trace_frame, also the vertex shader constants." },
	{ "debug.gpu_skip_vertex_shaders", _config_string, "\"\"", "HALO_GPU_SKIP_VS", _environment_value, _platform_all,
		"Comma-separated ids of vertex shaders not to draw with." },
	{ "debug.gpu_dump_shaders", _config_string, "\"\"", "HALO_GPU_DUMP_SHADERS", _environment_value, _platform_all,
		"A folder to write the generated GLSL to; empty none." },
	{ "debug.gpu_debug_expression", _config_string, "\"\"", "HALO_GPU_DEBUG_EXPR", _environment_value, _platform_all,
		"A GLSL expression every pixel shader shows instead of its result." },
	{ "debug.gpu_debug_texture0", _config_boolean, "false", "HALO_GPU_DEBUG_T0", _environment_set_is_true, _platform_all,
		"Pixel shaders show their first texture." },
	{ "debug.gpu_debug_flat", _config_boolean, "false", "HALO_GPU_DEBUG_FLAT", _environment_set_is_true, _platform_all,
		"Pixel shaders show their vertex colour." },
	{ "debug.screenshot_directory", _config_string, "\"\"", "HALO_SCREENSHOT_DIR", _environment_value, _platform_all,
		"A folder to save frames to (with screenshot_every); empty none." },
	{ "debug.screenshot_every", _config_integer, "0", "HALO_SCREENSHOT_EVERY", _environment_value, _platform_all,
		"Save every this many frames to screenshot_directory; 0 none." },
	{ "debug.texture_dump_directory", _config_string, "\"\"", "HALO_TEXTURE_DUMP", _environment_value, _platform_all,
		"A folder to write every texture to as it is uploaded; empty none." },
	{ "debug.texture_log", _config_boolean, "false", "HALO_TEXTURE_LOG", _environment_set_is_true, _platform_all,
		"Log texture uploads." },
	{ "debug.texture_no_cache", _config_boolean, "false", "HALO_TEXTURE_NO_CACHE", _environment_set_is_true, _platform_all,
		"Upload textures again every time they are used." },
	{ "debug.sample_seconds", _config_real, "0.0", "HALO_SAMPLE", _environment_value, _platform_android,
		"Log where every game thread is this often, in seconds (read by the\n"
		"app, port/android/host/host_debug.c); 0 never." },
	{ "debug.memory_watch", _config_boolean, "true", NULL, _environment_value, _platform_android,
		"Notice the game's writes to cached textures and vertices by page\n"
		"protection; false compares page contents once a frame instead, which is\n"
		"slower. Under ARM translation (the x86 emulator) the app always compares\n"
		"contents. Read by the app from the file (port/android/host/host_main.c)." },
#ifdef HALO_PROFILE
	{ "debug.profile_record", _config_boolean, "false", "HALO_PROFILE_RECORD", _environment_value, _platform_all,
		"Record a profile with no command (configure.py --profile builds): from\n"
		"when profile_record_when says until profile_stop, a map change or the\n"
		"end, into numbered part files in the data folder's profiles folder\n"
		"(tools/net_report.py reads it)." },
	{ "debug.profile_record_when", _config_string, "\"start\"", "HALO_PROFILE_RECORD_WHEN", _environment_value,
		_platform_all,
		"\"start\": from the first frame until the first map change; \"game\":\n"
		"each game that is not the main menu, a recording each, until its map\n"
		"goes." },
	{ "debug.profile_memory", _config_integer, "256", "HALO_PROFILE_MEMORY", _environment_value, _platform_all,
		"Megabytes a recording keeps in memory, 4 to 1024: two halves, each\n"
		"written out as a part when it fills." },
#endif
};

#define NUMBER_OF_CONFIG_SETTINGS (sizeof(config_settings) / sizeof(config_settings[0]))

#ifdef HALO_ANDROID
#define CONFIG_BUILD_PLATFORM _platform_android
#elif defined(HALO_WEB)
#define CONFIG_BUILD_PLATFORM (_platform_desktop | _platform_web)
#elif defined(_WIN32)
#define CONFIG_BUILD_PLATFORM (_platform_desktop | _platform_windows)
#else
#define CONFIG_BUILD_PLATFORM _platform_desktop
#endif

/* a dedicated server (server.enabled, --server, HALO_SERVER: decided as the
file is read): its settings are written into the file too */
static int config_server_mode;
/* a run that only answers its command line (--version...): nothing written */
static int config_answer_only;
/* halo --probe [<invite>]: a list site's look at a game (network_test.c's
"probe"), headless */
static int config_probe_mode;
#define CONFIG_PLATFORM (CONFIG_BUILD_PLATFORM | (config_server_mode ? _platform_server : 0))

struct config_value
{
	int boolean;
	long integer;
	double real;
	char *string;
};

static struct config_value config_values[NUMBER_OF_CONFIG_SETTINGS];
static int config_loaded = 0;
static pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

/* ---------- the command line */

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* whether the command line has the argument name ("--server"), and with
value, the one after it (or after "=": --config=<path>) */
static int config_argument(const char *name, char *value, size_t size)
{
	char text[1024];
	size_t length = strlen(name);
	int index;

	for (index = 1; posix_command_line_argument(index, text, (posix_ulong)sizeof(text)); index++)
	{
		if (!strcmp(text, name))
			return !value || posix_command_line_argument(index + 1, value, (posix_ulong)size);
		if (value && !strncmp(text, name, length) && text[length] == '=')
		{
			snprintf(value, size, "%s", text + length + 1);
			return 1;
		}
	}
	return 0;
}
#endif

/* ---------- the file */

static void config_path(char *path, size_t size)
{
#if defined(HALO_ANDROID) || defined(HALO_WEB)
	/* the data folder, which the app names (port/android/host/host_main.c;
	on the web, port/web/src/web_main.c) */
	const char *root = getenv("HALO_DATA_ROOT");

	snprintf(path, size, "%s/config.toml", root && *root ? root : ".");
#else
	/* HALO_CONFIG_FILE or --config <path> (a dedicated server's instance's
	own), else the executable's folder's, with its separator */
	const char *file = getenv("HALO_CONFIG_FILE");
	const char *base;

	if (file && *file)
	{
		snprintf(path, size, "%s", file);
		return;
	}
	if (config_argument("--config", path, size) && *path)
		return;
	base = SDL_GetBasePath();
	snprintf(path, size, "%sconfig.toml", base ? base : "");
#endif
}

/* the whole file, NUL terminated, or NULL; free() it */
static char *config_read_file(const char *path, size_t *size)
{
#ifdef HALO_ARM64_GUEST
	FILE *file = fopen(path, "rb");
	char *text = NULL;
	long length;

	if (!file)
		return NULL;
	if (fseek(file, 0, SEEK_END) == 0 && (length = ftell(file)) >= 0 && fseek(file, 0, SEEK_SET) == 0)
	{
		text = malloc((size_t)length + 1);
		if (text && fread(text, 1, (size_t)length, file) == (size_t)length)
		{
			text[length] = 0;
			*size = (size_t)length;
		}
		else
		{
			free(text);
			text = NULL;
		}
	}
	fclose(file);
	return text;
#else
	/* SDL's, for UTF-8 paths on Windows */
	void *data = SDL_LoadFile(path, size);
	char *text;

	if (!data)
		return NULL;
	text = malloc(*size + 1);
	if (text)
	{
		memcpy(text, data, *size);
		text[*size] = 0;
	}
	SDL_free(data);
	return text;
#endif
}

static int config_write_file(const char *path, const char *text)
{
#ifdef HALO_ARM64_GUEST
	FILE *file = fopen(path, "wb");
	int written;

	if (!file)
		return 0;
	written = fwrite(text, 1, strlen(text), file) == strlen(text);
	return fclose(file) == 0 && written;
#elif !defined(HALO_ANDROID) && !defined(HALO_WEB)
	/* (a dedicated server's holds its passwords: its user's to read alone) */
	if (!SDL_SaveFile(path, text, strlen(text)))
		return 0;
	if (config_server_mode)
		posix_server_private_file(path);
	return 1;
#else
	return SDL_SaveFile(path, text, strlen(text));
#endif
}

struct config_text
{
	char *buffer;
	size_t length, capacity;
};

static void config_append(struct config_text *text, const char *string)
{
	size_t length = strlen(string);

	if (text->length + length + 1 > text->capacity)
	{
		size_t capacity = (text->capacity ? text->capacity : 4096) * 2 + length;
		char *buffer = realloc(text->buffer, capacity);

		if (!buffer)
			return;
		text->buffer = buffer;
		text->capacity = capacity;
	}
	memcpy(text->buffer + text->length, string, length + 1);
	text->length += length;
}

/* the first length characters of text, as a string of their own */
static char *config_copy(const char *text, size_t length)
{
	char *copy = malloc(length + 1);

	if (copy)
	{
		memcpy(copy, text, length);
		copy[length] = 0;
	}
	return copy;
}

/* one setting as the file holds it: its comment, and its key at the
default */
static void config_append_setting(struct config_text *text, const struct config_setting *setting)
{
	const char *dot = strchr(setting->name, '.');
	const char *line;
	char buffer[256];

	config_append(text, "\n");
	for (line = setting->comment; *line;)
	{
		size_t length = strcspn(line, "\n");

		snprintf(buffer, sizeof(buffer), "# %.*s\n", (int)length, line);
		config_append(text, buffer);
		line += length;
		if (*line)
			line++;
	}
#ifndef HALO_ANDROID
	/* (Android apps have no environment to set) */
	switch (setting->environment_style)
	{
	case _environment_value:
		snprintf(buffer, sizeof(buffer), "# (for one run: %s=<value>)\n", setting->environment);
		break;
	case _environment_set_is_true:
		snprintf(buffer, sizeof(buffer), "# (for one run: %s=1 makes it true)\n", setting->environment);
		break;
	case _environment_set_is_false:
		snprintf(buffer, sizeof(buffer), "# (for one run: %s=1 makes it false)\n", setting->environment);
		break;
	}
	config_append(text, buffer);
#endif
	snprintf(buffer, sizeof(buffer), "%s = %s\n", dot + 1, setting->default_value);
	config_append(text, buffer);
}

/* the file with every setting of this build at its default */
static char *config_default_text(void)
{
	struct config_text text = { NULL, 0, 0 };
	char section[32] = "";
	size_t index;

#ifdef HALO_ANDROID
	config_append(&text,
		"# Halo settings\n"
		"#\n"
		"# The game writes this file with the defaults when it is missing: delete\n"
		"# it to go back to them.\n");
#else
	config_append(&text,
		"# Halo settings\n"
		"#\n"
		"# The game writes this file with the defaults when it is missing: delete\n"
		"# it to go back to them. Each setting can also be set for one run with\n"
		"# the environment variable named with it, which wins over this file.\n");
#endif
	/* each section once, where its first setting is, with all of its
	settings: TOML refuses a section twice, and the whole file would go
	unread (config_join_sections) */
	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const struct config_setting *setting = &config_settings[index];
		const char *dot = strchr(setting->name, '.');
		size_t length, other;
		char buffer[64];

		if (!(setting->platforms & CONFIG_PLATFORM) || !dot)
			continue;
		length = (size_t)(dot - setting->name);
		for (other = 0; other < index; other++)
		{
			if ((config_settings[other].platforms & CONFIG_PLATFORM) &&
				!strncmp(config_settings[other].name, setting->name, length + 1))
				break;
		}
		if (other < index)
			continue;
		snprintf(section, sizeof(section), "%.*s", (int)length, setting->name);
		snprintf(buffer, sizeof(buffer), "\n[%s]\n", section);
		config_append(&text, buffer);
		for (other = index; other < NUMBER_OF_CONFIG_SETTINGS; other++)
		{
			if ((config_settings[other].platforms & CONFIG_PLATFORM) &&
				!strncmp(config_settings[other].name, setting->name, length + 1))
				config_append_setting(&text, &config_settings[other]);
		}
	}
	return text.buffer;
}

/* the settings of this build that text (the file, parsed as table) lacks,
added to it in their sections, keeping the rest as it is: a newer version's
settings appear in an older file. Returns the new text, or NULL if nothing
was missing */
static char *config_add_missing(const char *text, toml_datum_t table)
{
	char *result = NULL;
	size_t index;

	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const struct config_setting *setting = &config_settings[index];
		const char *dot = strchr(setting->name, '.');
		const char *current = result ? result : text;
		struct config_text block = { NULL, 0, 0 };
		struct config_text updated = { NULL, 0, 0 };
		char header[40];
		const char *line;
		const char *insert = NULL;

		if (!(setting->platforms & CONFIG_PLATFORM) || !dot || toml_seek(table, setting->name).type != TOML_UNKNOWN)
			continue;
		snprintf(header, sizeof(header), "[%.*s]", (int)(dot - setting->name), setting->name);
		/* the end of the section's last line that is not blank */
		for (line = current; *line; )
		{
			const char *start = line;
			size_t length = strcspn(line, "\n");

			while (*start == ' ' || *start == '\t')
				start++;
			if (insert && *start == '[')
				break;
			if (!insert && !strncmp(start, header, strlen(header)))
				insert = line + length;
			else if (insert && start < line + length && *start != '\r')
				insert = line + length;
			line += length;
			if (*line)
				line++;
		}
		if (insert)
		{
			if (*insert)
				insert++;
			config_append_setting(&block, setting);
		}
		else
		{
			/* no such section: a new one at the end */
			insert = current + strlen(current);
			config_append(&block, current[0] && insert[-1] != '\n' ? "\n\n" : "\n");
			config_append(&block, header);
			config_append(&block, "\n");
			config_append_setting(&block, setting);
		}
		if (!block.buffer)
			continue;
		{
			char *before = config_copy(current, (size_t)(insert - current));

			if (before)
				config_append(&updated, before);
			free(before);
		}
		if (insert > current && insert[-1] != '\n')
			config_append(&updated, "\n");
		config_append(&updated, block.buffer);
		config_append(&updated, insert);
		free(block.buffer);
		if (updated.buffer)
		{
			free(result);
			result = updated.buffer;
			platform_log("settings: added %s (new in this version) at its default", setting->name);
		}
	}
	return result;
}

static int config_line_section(const char *line, const char *end, char *section, size_t size);

/* text from start to end, appended */
static void config_append_range(struct config_text *text, const char *start, const char *end)
{
	char *copy = config_copy(start, (size_t)(end - start));

	if (copy)
		config_append(text, copy);
	free(copy);
}

/* a file with a section twice, which TOML refuses (the defaults of the
versions from 2026-10-10 had a second [display] after [game]): each
section's later parts moved to the end of its first, the rest kept as it
is. Returns the new text, or NULL if no section is there twice */
static char *config_join_sections(const char *text)
{
	enum { PARTS_MAXIMUM = 64 };
	struct
	{
		char name[64];
		const char *start, *end;
	} parts[PARTS_MAXIMUM];
	struct config_text joined = { NULL, 0, 0 };
	const char *line;
	const char *first_part = NULL;
	int count = 0, repeated = 0, index, other;

	/* the parts, each from its section's line to the next one's */
	for (line = text; *line; )
	{
		size_t length = strcspn(line, "\n");
		char name[64];

		if (config_line_section(line, line + length, name, sizeof(name)))
		{
			if (count == PARTS_MAXIMUM)
				return NULL;
			if (count)
				parts[count - 1].end = line;
			else
				first_part = line;
			for (other = 0; other < count; other++)
				repeated |= !strcmp(parts[other].name, name);
			strcpy(parts[count].name, name);
			parts[count].start = line;
			count++;
		}
		line += length;
		if (*line)
			line++;
	}
	if (!repeated)
		return NULL;
	parts[count - 1].end = line;

	config_append_range(&joined, text, first_part);
	for (index = 0; index < count; index++)
	{
		for (other = 0; other < index && strcmp(parts[other].name, parts[index].name); other++)
			;
		if (other < index)
			continue;
		config_append_range(&joined, parts[index].start, parts[index].end);
		for (other = index + 1; other < count; other++)
		{
			const char *body;

			if (strcmp(parts[other].name, parts[index].name))
				continue;
			/* (its settings, without its section's line) */
			body = memchr(parts[other].start, '\n', (size_t)(parts[other].end - parts[other].start));
			if (!body)
				continue;
			if (joined.length && joined.buffer[joined.length - 1] != '\n')
				config_append(&joined, "\n");
			config_append_range(&joined, body + 1, parts[other].end);
		}
	}
	return joined.buffer;
}

/* ---------- values */

static int config_text_is_false(const char *text)
{
	char lower[8];
	size_t index;

	for (index = 0; index + 1 < sizeof(lower) && text[index]; index++)
		lower[index] = (char)tolower((unsigned char)text[index]);
	lower[index] = 0;
	return !strcmp(lower, "0") || !strcmp(lower, "false") || !strcmp(lower, "no") || !strcmp(lower, "off");
}

/* whether a variable that only has to be set (_environment_set_is_true or
_environment_set_is_false) is: empty, "0", "false", "no" or "off" is not */
static int config_environment_set(const char *text)
{
	return text[0] && !config_text_is_false(text);
}

static void config_set_from_text(struct config_value *value, enum config_type type, const char *text)
{
	switch (type)
	{
	case _config_boolean:
		value->boolean = !config_text_is_false(text);
		break;
	case _config_integer:
		value->integer = strtol(text, NULL, 10);
		break;
	case _config_real:
		value->real = strtod(text, NULL);
		break;
	case _config_string:
		/* (the old string is kept, not freed: config_string's callers hold
		its pointer, and Settings writes few, seldom) */
		value->string = strdup(text);
		break;
	}
}

/* the value in the file, if it is there and of the setting's type */
static void config_set_from_file(struct config_value *value, const struct config_setting *setting,
	toml_datum_t table)
{
	toml_datum_t datum = toml_seek(table, setting->name);
	int wrong_type = 0;

	if (datum.type == TOML_UNKNOWN)
		return;
	switch (setting->type)
	{
	case _config_boolean:
		if (datum.type == TOML_BOOLEAN)
			value->boolean = datum.u.boolean;
		else
			wrong_type = 1;
		break;
	case _config_integer:
		if (datum.type == TOML_INT64)
			value->integer = (long)datum.u.int64;
		else
			wrong_type = 1;
		break;
	case _config_real:
		if (datum.type == TOML_FP64)
			value->real = datum.u.fp64;
		else if (datum.type == TOML_INT64)
			value->real = (double)datum.u.int64;
		else
			wrong_type = 1;
		break;
	case _config_string:
		if (datum.type == TOML_STRING)
		{
			free(value->string);
			value->string = strdup(datum.u.s);
		}
		else
		{
			wrong_type = 1;
		}
		break;
	}
	if (wrong_type)
	{
		static const char *const expected[] = { "true or false", "a whole number", "a number", "a quoted string" };

		platform_log("config.toml line %d: %s should be %s; using %s", datum.lineno, setting->name,
			expected[setting->type], setting->default_value);
	}
}

/* how many times a setting has been written (config_write): readers that
keep a setting watch this, to read it again */
static volatile unsigned long config_change_count;

static long config_setting_index(const char *name)
{
	size_t index;

	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		if (!strcmp(config_settings[index].name, name))
			return (long)index;
	}
	return -1;
}

/* a key in the file that is no setting, likely misspelt or in the wrong
section: said in the log, with the section of a setting of that name */
static void config_report_unknown_key(int line, const char *name)
{
	const char *key = strrchr(name, '.');
	const char *found = NULL;
	size_t index, count = 0;

	key = key ? key + 1 : name;
	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const char *dot = strchr(config_settings[index].name, '.');

		if (dot && !strcmp(dot + 1, key))
		{
			found = config_settings[index].name;
			count++;
		}
	}
	/* (only a name one setting has: the section is then sure) */
	if (count == 1)
	{
		platform_log("config.toml line %d: unknown setting %s, ignored (%s goes under [%.*s])", line, name, key,
			(int)(strchr(found, '.') - found), found);
	}
	else
	{
		platform_log("config.toml line %d: unknown setting %s, ignored", line, name);
	}
}

/* keys in the file that are no setting, likely misspelt */
static void config_report_unknown_keys(toml_datum_t table)
{
	int section_index;

	for (section_index = 0; section_index < table.u.tab.size; section_index++)
	{
		toml_datum_t section = table.u.tab.value[section_index];
		int key_index;

		if (section.type != TOML_TABLE)
		{
			config_report_unknown_key(section.lineno, table.u.tab.key[section_index]);
			continue;
		}
		for (key_index = 0; key_index < section.u.tab.size; key_index++)
		{
			char name[128];

			snprintf(name, sizeof(name), "%s.%s", table.u.tab.key[section_index], section.u.tab.key[key_index]);
			if (config_setting_index(name) < 0)
				config_report_unknown_key(section.u.tab.value[key_index].lineno, name);
		}
	}
}

/* the most lines of a file that cannot be read that are skipped to read
the rest */
#define CONFIG_MAXIMUM_SKIPPED_LINES 16

/* the line a parse error is on ("(line 12) ..."), or 0 */
static int config_error_line(const char *message)
{
	const char *at = strstr(message, "(line ");
	int line = 0;

	if (!at || sscanf(at, "(line %d)", &line) != 1)
		return 0;
	return line;
}

/* whether length bytes of text are word, in any case (ASCII) */
static int config_word_is(const char *text, size_t length, const char *word)
{
	size_t index;

	if (strlen(word) != length)
		return 0;
	for (index = 0; index < length; index++)
	{
		if (tolower((unsigned char)text[index]) != word[index])
			return 0;
	}
	return 1;
}

/* the line of text that cannot be read, said in the log with what likely
makes it so, and emptied (its number kept for the lines after it): returns
whether it was there */
static int config_skip_line(char *text, int line_number, const char *message)
{
	char *line = text, *end, *value, *value_end;
	const char *reason = strchr(message, ')');
	char key[64] = "";
	char hint[160] = "";
	int number;

	for (number = 1; number < line_number && *line; number++)
		line += strcspn(line, "\n") + (line[strcspn(line, "\n")] ? 1 : 0);
	if (line_number < 1 || number != line_number || !*line)
		return 0;
	end = line + strcspn(line, "\n");
	reason = reason ? reason + 1 : message;
	while (*reason == ' ')
		reason++;
	/* "key = value": the value as written, to the end of the line or a
	comment */
	value = memchr(line, '=', (size_t)(end - line));
	if (value)
	{
		char *key_start = line, *key_end = value;

		while (key_start < key_end && (*key_start == ' ' || *key_start == '\t'))
			key_start++;
		while (key_end > key_start && (key_end[-1] == ' ' || key_end[-1] == '\t'))
			key_end--;
		snprintf(key, sizeof(key), "%.*s", (int)(key_end - key_start), key_start);
		value++;
		while (value < end && (*value == ' ' || *value == '\t'))
			value++;
		value_end = value + strcspn(value, "#\n");
		if (value_end > end)
			value_end = end;
		while (value_end > value && (value_end[-1] == ' ' || value_end[-1] == '\t' || value_end[-1] == '\r'))
			value_end--;
		if (config_word_is(value, (size_t)(value_end - value), "true") ||
			config_word_is(value, (size_t)(value_end - value), "false"))
		{
			snprintf(hint, sizeof(hint), ": true and false are lowercase, %s = %.*s", key, (int)(value_end - value),
				value_end - value == 4 ? "true" : "false");
		}
		else if (value < value_end && isalpha((unsigned char)*value))
		{
			snprintf(hint, sizeof(hint), ": text needs quotes, %s = \"%.*s\"", key, (int)(value_end - value), value);
		}
	}
	platform_log("config.toml line %d cannot be read (%s), so it is skipped%s", line_number, reason, hint);
	memset(line, ' ', (size_t)(end - line));
	return 1;
}

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* a config.toml the game never reads, likely edited in its place: said in
the log, with the one it reads (path) */
static void config_report_unread(const char *path)
{
	char other[1100];
#ifndef _WIN32
	/* in the folder the game was started from, when that is not the
	executable's (the game's is beside the executable) */
	char folder[1024];

	if (getcwd(folder, sizeof(folder)))
	{
		snprintf(other, sizeof(other), "%s/config.toml", folder);
		if (strcmp(other, path) && SDL_GetPathInfo(other, NULL))
		{
			SDL_PathInfo mine, theirs;

			/* (the same file by another name is the one read) */
			if (!SDL_GetPathInfo(path, &mine) || !SDL_GetPathInfo(other, &theirs) ||
				mine.size != theirs.size || mine.modify_time != theirs.modify_time)
			{
				platform_log("settings: %s is not read; the game's settings are %s", other, path);
			}
		}
	}
#endif
	/* the file saved with .txt after it, by an editor that adds it (and
	Windows Explorer hides it) */
	snprintf(other, sizeof(other), "%s.txt", path);
	if (SDL_GetPathInfo(other, NULL))
	{
#ifdef _WIN32
		platform_log("settings: %s is not read: Windows saved it as config.toml.txt; rename it to config.toml", other);
#else
		platform_log("settings: %s is not read; rename it to config.toml", other);
#endif
	}
}
#endif

/* ---------- words */

/* the settings that take one of a few words: one in any case is taken (as
written here: "Windowed" is "windowed"), and another is said once in the
log */
static const struct
{
	const char *name;
	/* NULL ended; "" where empty is one */
	const char *words[24];
	/* what the game does with another, for the log (NULL: not said) */
	const char *otherwise;
} config_words[] =
{
	{ "display.mode", { "", "fullscreen", "borderless", "windowed", NULL }, "display.fullscreen's" },
	{ "display.resolution_scaling", { "native", "75%", "50%", "25%", "original", NULL }, NULL },
	{ "display.anti_aliasing", { "off", "fxaa", "smaa", "ssaa2x", "msaa2x", "msaa4x", "msaa8x", NULL }, NULL },
	{ "display.hud_button_style", { "classic", "gems", "menu_gems", "dark_gems", "xbox_modern",
		"ps_classic", "ps_gems", "ps_dark_gems", "ps_modern", "switch_classic", "switch_gems", "switch_modern", NULL }, NULL },
	{ "display.hud_button_style_xbox", { "classic", "gems", "menu_gems", "dark_gems", "xbox_modern", NULL }, NULL },
	{ "display.hud_bumper_style", { "original", "modern", NULL }, NULL },
	{ "display.hud_button_style_ps", { "ps_classic", "ps_gems", "ps_dark_gems", "ps_modern", NULL }, NULL },
	{ "display.hud_button_style_switch", { "switch_classic", "switch_gems", "switch_modern", NULL }, NULL },
	{ "display.menus", { "pc", "xbox", NULL }, NULL },
	{ "display.player_names", { "all", "allies", "enemies", "none", NULL }, NULL },
	{ "display.scoreboard_team_layout", { "teams", "score", NULL }, NULL },
	{ "display.chat_style", { "colors", "white", NULL }, NULL },
	{ "display.shield_glow_intensity", { "default", "light_show", NULL }, NULL },
	{ "display.shield_color", { "default", "white", "black", "red", "blue", "gray", "yellow", "green", "pink",
		"purple", "cyan", "cobalt", "orange", "teal", "sage", "brown", "tan", "maroon", "salmon", NULL }, NULL },
	{ "audio.resampler", { "sinc", "linear", NULL }, NULL },
	{ "audio.voice_chat", { "push_to_talk", "open_mic", "off", NULL }, NULL },
	{ "input.touch_controls", { "auto", "on", "off", NULL }, NULL },
	{ "game.console_log", { "important", "all", "none", NULL }, NULL },
	{ "game.language", { "", "ja", "de", "fr", "es", "it", NULL }, NULL },
	{ "network.text_chat", { "on", "links", "off", NULL }, NULL },
	{ "server.chat_filter", { "off", "censor", "block", NULL }, NULL },
	{ "server.chat_filter_slurs", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "server.chat_filter_swearing", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "server.chat_filter_insults", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "server.vpn_policy", { "link", "block", "off", NULL }, NULL },
	{ "server.vpn_fail", { "open", "closed", NULL }, NULL },
	{ "network.votekick_threshold", { "dynamic", "static", NULL }, NULL },
	{ "server.coop_bsp_switch", { "majority", "anyone", NULL }, NULL },
	{ "network.link_share", { "when_needed", "always", "never", NULL }, NULL },
	{ "network.voice_mode", { "off", "team_proximity", "team_enemy_proximity", "team_global",
		"team_global_enemy_proximity", NULL }, NULL },
	{ "network.coop_team_wipe", { "auto", "revert", "respawn", NULL }, NULL },
	{ "network.coop_friendly_fire", { "off", "on", "shields_only", "explosives_only", NULL }, NULL },
	{ "network.shield_effects", { "stock", "glow", "light_show", "overshield", "glow_overshield",
		"light_show_overshield", "players", NULL }, NULL },
	{ "network.coop_enemies_mode", { "none", "per_player", "multiplier", NULL }, NULL },
	{ "streamer.slurs", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "streamer.swearing", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "streamer.insults", { "off", "severe", "strong", "mild", NULL }, NULL },
	{ "crash_reports.upload", { "ask", "yes", "no", NULL }, NULL },
};

#define NUMBER_OF_CONFIG_WORDS (sizeof(config_words) / sizeof(config_words[0]))

/* the word settings' values as their readers compare them (lowercase), and
the log told of one that is none of its words */
static void config_check_words(void)
{
	size_t word_index;

	for (word_index = 0; word_index < NUMBER_OF_CONFIG_WORDS; word_index++)
	{
		long index = config_setting_index(config_words[word_index].name);
		const char *const *words = config_words[word_index].words;
		struct config_value *value;
		char list[400] = "";
		size_t word;

		if (index < 0 || !(config_settings[index].platforms & CONFIG_PLATFORM))
			continue;
		value = &config_values[index];
		for (word = 0; words[word] && !config_word_is(value->string, strlen(value->string), words[word]); word++)
		{
		}
		if (words[word])
		{
			if (strcmp(value->string, words[word]))
			{
				free(value->string);
				value->string = strdup(words[word]);
			}
			continue;
		}
		/* ("a", "b" or "c", and "or empty" for "") */
		for (word = 0; words[word]; word++)
		{
			size_t length = strlen(list);

			if (!words[word][0])
				continue;
			snprintf(list + length, sizeof(list) - length, "%s\"%s\"",
				!length ? "" : !words[word + 1] && words[0][0] ? " or " : ", ", words[word]);
		}
		if (!words[0][0])
			snprintf(list + strlen(list), sizeof(list) - strlen(list), " or empty");
		platform_log("settings: %s = \"%s\" is not %s%s%s", config_words[word_index].name, value->string, list,
			config_words[word_index].otherwise ? "; using " : "",
			config_words[word_index].otherwise ? config_words[word_index].otherwise : "");
	}
}

/* the passwords a file may hold instead of their variable (a container's
secrets): <variable>_FILE names it, and its first line is the value */
static const char *const config_secret_files[] =
{
	"server.password", "server.web_password", "server.web_owner_password", "server.web_moderator_password",
	"server.web_viewer_password",
};
#define NUMBER_OF_CONFIG_SECRET_FILES (sizeof(config_secret_files) / sizeof(config_secret_files[0]))

/* the value of the setting's <variable>_FILE (read once, at the first
question: a file changed after the start is not read again), NULL if
there is none, it cannot be read or its first line is empty */
static const char *config_environment_file(const struct config_setting *setting)
{
	static char values[NUMBER_OF_CONFIG_SECRET_FILES][512];
	static signed char read[NUMBER_OF_CONFIG_SECRET_FILES];
	size_t index;

	for (index = 0; index < NUMBER_OF_CONFIG_SECRET_FILES && strcmp(config_secret_files[index], setting->name); index++)
	{
	}
	if (index == NUMBER_OF_CONFIG_SECRET_FILES || !setting->environment)
		return NULL;
	if (!read[index])
	{
		char variable[96];
		const char *path;
		char *text;
		size_t size = 0, length;

		read[index] = -1;
		snprintf(variable, sizeof(variable), "%s_FILE", setting->environment);
		path = getenv(variable);
		if (!path || !path[0])
			return NULL;
		text = config_read_file(path, &size);
		if (!text)
		{
			platform_log("settings: %s names %s, which cannot be read: %s is not set by it", variable, path,
				setting->name);
			return NULL;
		}
		/* (the first line, its CR and LF off) */
		length = strcspn(text, "\n");
		if (length && text[length - 1] == '\r')
			length--;
		if (length >= sizeof(values[index]))
		{
			platform_log("settings: the first line of %s (%s) is longer than %d characters: %s is not set by it",
				path, variable, (int)sizeof(values[index]) - 1, setting->name);
			length = 0;
		}
		memcpy(values[index], text, length);
		values[index][length] = 0;
		memset(text, 0, size);
		free(text);
		read[index] = length ? 1 : -1;
	}
	return read[index] > 0 ? values[index] : NULL;
}

/* a setting's environment variable, NULL if it is not set; a dedicated
server's settings' empty ones are not set either (a panel passes every
variable it has, empty ones too: an empty password must not stand for one).
A password's variable that is not set may be read from its file
(<variable>_FILE: config_environment_file) */
static const char *config_environment(const struct config_setting *setting)
{
	const char *environment = setting->environment ? getenv(setting->environment) : NULL;

	if (environment && !environment[0] &&
		(setting->platforms == _platform_server || !strcmp(setting->name, "network.public_address")))
	{
		environment = NULL;
	}
	if (!environment)
		environment = config_environment_file(setting);
	return environment;
}

#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
/* a dedicated server (port/linux/src/server_mode.c) has no window, sound,
microphone or player, never asks anything, and is reached only as its
settings say, whatever the file and the environment say of these (for this
run: nothing of it is written) */
/* halo --probe: no window, sound, update, Discord or menus' test; the
network test's probe, and a deadline (internet play as set: on by default,
which an invite needs) */
static void config_apply_probe_mode(void)
{
	static const char *const forced[][2] =
	{
		{ "debug.null_renderer", "true" },
		{ "debug.hidden_window", "true" },
		{ "display.fullscreen", "false" },
		{ "audio.enabled", "false" },
		{ "audio.voice_chat", "off" },
		{ "network.join_from_clipboard", "false" },
		{ "update.auto", "false" },
		{ "crash_reports.upload", "no" },
		{ "discord.application_id", "" },
		{ "debug.network_test", "probe" },
		{ "server.enabled", "false" },
		{ "debug.test_input", "" },
		{ "debug.menu_open", "" },
		{ "debug.exit_after", "45" },
	};
	size_t index;

	for (index = 0; index < sizeof(forced) / sizeof(forced[0]); index++)
	{
		long found = config_setting_index(forced[index][0]);

		if (found >= 0)
			config_set_from_text(&config_values[found], config_settings[found].type, forced[index][1]);
	}
}

static void config_apply_server_mode(void)
{
	static const char *const forced[][2] =
	{
		{ "server.enabled", "true" },
		{ "debug.null_renderer", "true" },
		{ "debug.hidden_window", "true" },
		{ "display.fullscreen", "false" },
		{ "display.vsync", "false" },
		{ "display.interpolation", "false" },
		{ "display.high_res_hud", "false" },
		{ "display.high_res_text", "false" },
		{ "display.anti_aliasing", "off" },
		{ "audio.enabled", "false" },
		{ "audio.voice_chat", "off" },
		{ "network.join_from_clipboard", "false" },
		{ "update.auto", "false" },
		{ "crash_reports.upload", "no" },
		{ "discord.application_id", "" },
		{ "debug.network_test", "" },
		{ "debug.test_input", "" },
		{ "debug.menu_open", "" },
	};
	const char *tunnel_port = getenv("HALO_NET_TUNNEL_PORT");
	size_t index;

	for (index = 0; index < sizeof(forced) / sizeof(forced[0]); index++)
	{
		long found = config_setting_index(forced[index][0]);

		if (found >= 0)
			config_set_from_text(&config_values[found], config_settings[found].type, forced[index][1]);
	}
	/* (the tunnel's port fixed, Halo PC's 2302, unless one is given: a
	server's players and firewall need to know it; a router asked to
	forward it only as server.upnp says) */
	if ((!tunnel_port || !*tunnel_port) && !config_values[config_setting_index("network.tunnel_port")].integer)
		config_values[config_setting_index("network.tunnel_port")].integer = 2302;
	config_values[config_setting_index("network.allow_upnp")].boolean =
		config_values[config_setting_index("server.upnp")].boolean;
}
#endif

static void config_load(void)
{
	char path[1024];
	size_t size = 0;
	char *text;
	size_t index;

	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const char *default_value = config_settings[index].default_value;

		if (config_settings[index].type == _config_string)
		{
			/* written as a TOML basic string without escapes */
			size_t length = strlen(default_value);

			config_values[index].string = length >= 2 ? config_copy(default_value + 1, length - 2) : strdup("");
		}
		else
		{
			config_set_from_text(&config_values[index], config_settings[index].type, default_value);
		}
	}

	config_path(path, sizeof(path));
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	config_report_unread(path);
	/* (a dedicated server's file has its settings too: asked for by the
	command line or the environment, or by the file's server.enabled; a
	probe is never one, whatever they say) */
	config_probe_mode = config_argument("--probe", NULL, 0);
	{
		const char *environment = getenv("HALO_SERVER");

		config_server_mode = !config_probe_mode && (config_argument("--server", NULL, 0) ||
			(environment && config_environment_set(environment)));
	}
	/* (a run that only answers its command line, halo --version, writes no
	settings: the file read is left as it is) */
	config_answer_only = config_argument("--help", NULL, 0) || config_argument("--version", NULL, 0) ||
		config_argument("--keygen", NULL, 0) || config_argument("--sign", NULL, 0) || config_probe_mode;
#endif
	text = config_read_file(path, &size);
	if (text)
	{
		char *joined = config_join_sections(text);
		toml_result_t result;
		int skipped = 0;

		if (joined)
		{
			free(text);
			text = joined;
			size = strlen(text);
		}
		result = toml_parse(text, (int)size);
		/* (a line that cannot be read is skipped, in memory, so that the
		rest is) */
		while (!result.ok && skipped < CONFIG_MAXIMUM_SKIPPED_LINES &&
			config_skip_line(text, config_error_line(result.errmsg), result.errmsg))
		{
			toml_free(result);
			result = toml_parse(text, (int)strlen(text));
			skipped++;
		}
		if (result.ok)
		{
			char *completed;

			if (joined && !skipped && !config_answer_only)
			{
				if (config_write_file(path, text))
					platform_log("settings: joined the sections %s had twice", path);
				else
					platform_log("settings: cannot write %s", path);
			}
			for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
				config_set_from_file(&config_values[index], &config_settings[index], result.toptab);
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
			if (config_values[config_setting_index("server.enabled")].boolean && !config_probe_mode)
				config_server_mode = 1;
#endif
			config_report_unknown_keys(result.toptab);
			platform_log("settings: %s", path);
			/* (a file with lines skipped is left as the player wrote it, to
			be fixed) */
			if (skipped)
			{
				platform_log("settings: %d line%s of config.toml skipped; every other setting in it is used", skipped,
					skipped == 1 ? "" : "s");
			}
			else if (!config_answer_only)
			{
				completed = config_add_missing(text, result.toptab);
				if (completed && !config_write_file(path, completed))
					platform_log("settings: cannot write %s", path);
				free(completed);
			}
		}
		else
		{
			platform_log("config.toml cannot be read (%s): every setting is at its default until that is fixed "
				"(a word value needs quotes: menus = \"pc\")", result.errmsg);
		}
		toml_free(result);
		free(text);
	}
	else if (!config_answer_only)
	{
		char *defaults = config_default_text();

		if (defaults && config_write_file(path, defaults))
			platform_log("settings: wrote the defaults to %s", path);
		else
			platform_log("settings: cannot write %s; using the defaults", path);
		free(defaults);
	}

	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const struct config_setting *setting = &config_settings[index];
		const char *environment = config_environment(setting);

		if (!environment || (setting->environment_style != _environment_value && !config_environment_set(environment)))
			continue;
		switch (setting->environment_style)
		{
		case _environment_value:
			config_set_from_text(&config_values[index], setting->type, environment);
			break;
		case _environment_set_is_true:
			config_values[index].boolean = 1;
			break;
		case _environment_set_is_false:
			config_values[index].boolean = 0;
			break;
		}
	}
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	if (config_server_mode)
		config_apply_server_mode();
	else if (config_probe_mode)
		config_apply_probe_mode();
#endif
	config_check_words();
}

/* the file read, the first time a setting is asked for */
static void config_ensure_loaded(void)
{
	pthread_mutex_lock(&config_lock);
	if (!config_loaded)
	{
		config_load();
		config_loaded = 1;
	}
	pthread_mutex_unlock(&config_lock);
}

static const struct config_value *config_value(const char *name, enum config_type type)
{
	static const struct config_value none = { 0, 0, 0.0, "" };
	long index;

	config_ensure_loaded();
	index = config_setting_index(name);
	if (index < 0 || config_settings[index].type != type)
	{
		platform_log("settings: no %s setting %s", type == _config_string ? "string" : "such", name);
		return &none;
	}
	return &config_values[index];
}

/* ---------- writing a setting */

/* the line's key, if it is "key = ..." (after spaces), in key */
static int config_line_key(const char *line, const char *end, const char *key)
{
	size_t length = strlen(key);

	while (line < end && (*line == ' ' || *line == '\t'))
		line++;
	if ((size_t)(end - line) <= length || strncmp(line, key, length) != 0)
		return 0;
	line += length;
	while (line < end && (*line == ' ' || *line == '\t'))
		line++;
	return line < end && *line == '=';
}

/* the section the line opens, if it is "[section]" (after spaces) */
static int config_line_section(const char *line, const char *end, char *section, size_t size)
{
	const char *close;

	while (line < end && (*line == ' ' || *line == '\t'))
		line++;
	if (line >= end || *line != '[')
		return 0;
	close = memchr(line, ']', (size_t)(end - line));
	if (!close || (size_t)(close - line - 1) >= size)
		return 0;
	memcpy(section, line + 1, (size_t)(close - line - 1));
	section[close - line - 1] = 0;
	return 1;
}

/* the longest string config_write takes */
#define CONFIG_MAXIMUM_WRITTEN_STRING 4096

/* sets a setting, for now and in config.toml, from its value as text
("true", "60", "1.5", "all"): its line there is changed (or added), the rest
of the file kept as it is */
int config_write(const char *name, const char *value)
{
	const char *dot = strchr(name, '.');
	long index = config_setting_index(name);
	char section[64], key[64], wanted[80], current[64] = "", path[1024];
	struct config_text out = { 0 };
	size_t size = 0, line_size;
	char *text, *line_text;
	const char *line;
	int written = 0, in_section = 0, succeeded;

	/* (a string as long as a server's rotation, server_rotation.h's
	SERVER_ROTATION_MAXIMUM_TEXT; any other value is a word or a number) */
	if (index < 0 || !dot || (size_t)(dot - name) >= sizeof(section) ||
		strlen(value) > (config_settings[index].type == _config_string ? CONFIG_MAXIMUM_WRITTEN_STRING : 256))
	{
		return 0;
	}
	/* (the line: its key, and its value with each character escaped as
	\uXXXX at most) */
	line_size = strlen(dot + 1) + 6 * strlen(value) + 64;
	line_text = malloc(line_size);
	if (!line_text)
		return 0;
	/* (the file read first, as the other settings are) */
	config_value(name, config_settings[index].type);
	pthread_mutex_lock(&config_lock);
	config_set_from_text(&config_values[index], config_settings[index].type, value);
	/* (a run that only answers its command line, a probe's too, writes
	nothing: the value is this run's alone) */
	if (config_answer_only)
	{
		config_change_count++;
		pthread_mutex_unlock(&config_lock);
		free(line_text);
		return 1;
	}
	snprintf(section, sizeof(section), "%.*s", (int)(dot - name), name);
	snprintf(key, sizeof(key), "%s", dot + 1);
	switch (config_settings[index].type)
	{
	case _config_boolean:
		snprintf(line_text, line_size, "%s = %s\n", key, config_values[index].boolean ? "true" : "false");
		break;
	case _config_integer:
		snprintf(line_text, line_size, "%s = %ld\n", key, config_values[index].integer);
		break;
	case _config_real:
		/* (with its point: TOML reads 1 as an integer) */
		snprintf(line_text, line_size, "%s = %.15g", key, config_values[index].real);
		if (!strpbrk(line_text + strlen(key) + 3, ".en"))
			strcat(line_text, ".0");
		strcat(line_text, "\n");
		break;
	case _config_string:
	{
		char *end = line_text + snprintf(line_text, line_size, "%s = \"", key);
		const char *character;

		/* (a TOML basic string: quotes and backslashes escaped, and control
		characters, so that a value cannot end its line) */
		for (character = config_values[index].string; *character; character++)
		{
			if ((unsigned char)*character < 0x20 || *character == 0x7F)
			{
				end += sprintf(end, "\\u%04x", (unsigned)(unsigned char)*character);
				continue;
			}
			if (*character == '"' || *character == '\\')
				*end++ = '\\';
			*end++ = *character;
		}
		strcpy(end, "\"\n");
		break;
	}
	}
	snprintf(wanted, sizeof(wanted), "%s", section);
	config_path(path, sizeof(path));
	text = config_read_file(path, &size);
	for (line = text ? text : ""; *line;)
	{
		const char *end = line + strcspn(line, "\n");
		const char *next = *end ? end + 1 : end;

		if (config_line_section(line, end, current, sizeof(current)))
		{
			/* (leaving the section without the key: it goes at its end) */
			if (in_section && !written)
			{
				config_append(&out, line_text);
				written = 1;
			}
			in_section = !strcmp(current, wanted);
		}
		else if (in_section && !written && config_line_key(line, end, key))
		{
			config_append(&out, line_text);
			written = 1;
			line = next;
			continue;
		}
		{
			char *copy = config_copy(line, (size_t)(next - line));

			if (copy)
			{
				config_append(&out, copy);
				free(copy);
			}
		}
		line = next;
	}
	if (!written)
	{
		if (out.length && out.buffer[out.length - 1] != '\n')
			config_append(&out, "\n");
		if (!in_section)
		{
			char header[80];

			snprintf(header, sizeof(header), "\n[%s]\n", section);
			config_append(&out, header);
		}
		config_append(&out, line_text);
	}
	succeeded = out.buffer && config_write_file(path, out.buffer);
	config_change_count++;
	pthread_mutex_unlock(&config_lock);
	free(out.buffer);
	free(text);
	free(line_text);
	return succeeded;
}

int config_write_boolean(const char *name, int value)
{
	long index = config_setting_index(name);

	return index >= 0 && config_settings[index].type == _config_boolean && config_write(name, value ? "true" : "false");
}

/* a setting's value as text ("true", "60", "1.5", "all"): 0 if there is no
such setting */
int config_text(const char *name, char *text, size_t size)
{
	long index = config_setting_index(name);
	const struct config_value *value;

	if (index < 0)
		return 0;
	value = config_value(name, config_settings[index].type);
	switch (config_settings[index].type)
	{
	case _config_boolean:
		snprintf(text, size, "%s", value->boolean ? "true" : "false");
		break;
	case _config_integer:
		snprintf(text, size, "%ld", value->integer);
		break;
	case _config_real:
		snprintf(text, size, "%.15g", value->real);
		break;
	case _config_string:
		snprintf(text, size, "%s", value->string ? value->string : "");
		break;
	}
	return 1;
}

void config_folder(char *path, size_t size)
{
	char file[1024];
	char *separator;

	config_path(file, sizeof(file));
	separator = strrchr(file, '/');
#ifndef HALO_ANDROID
	if (!separator || (strrchr(file, '\\') && strrchr(file, '\\') > separator))
		separator = strrchr(file, '\\');
#endif
	if (separator)
		separator[1] = 0;
	else
		file[0] = 0;
	snprintf(path, size, "%s", file);
}

/* ---------- public code */

char *config_file_read(const char *path, size_t *size)
{
	return config_read_file(path, size);
}

unsigned long config_changes(void)
{
	return config_change_count;
}

int config_server_mode_enabled(void)
{
	config_value("server.enabled", _config_boolean);
	return config_server_mode;
}

int config_setting_info(int index, struct config_setting_info *info)
{
	static const char *const types[] = { "boolean", "integer", "real", "string" };
	const struct config_setting *setting;
	const char *environment;

	if (index < 0 || (size_t)index >= NUMBER_OF_CONFIG_SETTINGS)
		return 0;
	setting = &config_settings[index];
	memset(info, 0, sizeof(*info));
	info->name = setting->name;
	info->type = types[setting->type];
	info->environment = setting->environment;
	info->comment = setting->comment;
	config_default(setting->name, info->default_text, sizeof(info->default_text));
	environment = config_environment(setting);
	info->pinned = environment && (setting->environment_style == _environment_value || config_environment_set(environment));
	info->written = (setting->platforms & CONFIG_PLATFORM) != 0;
	return 1;
}

int config_reload(const char *prefix)
{
	char path[1024];
	size_t size = 0;
	size_t length = strlen(prefix);
	char *text;
	toml_result_t result;
	size_t index;

	config_ensure_loaded();
	config_path(path, sizeof(path));
	text = config_read_file(path, &size);
	if (!text)
		return 0;
	result = toml_parse(text, (int)size);
	if (!result.ok)
	{
		platform_log("settings: %s cannot be read again (%s); the settings stay as they were", path, result.errmsg);
		toml_free(result);
		free(text);
		return 0;
	}
	pthread_mutex_lock(&config_lock);
	for (index = 0; index < NUMBER_OF_CONFIG_SETTINGS; index++)
	{
		const struct config_setting *setting = &config_settings[index];
		const char *environment = config_environment(setting);
		struct config_value value;

		/* (the setting's own: those of the environment stay its) */
		if (strncmp(setting->name, prefix, length) ||
			(environment && (setting->environment_style == _environment_value || config_environment_set(environment))))
		{
			continue;
		}
		memset(&value, 0, sizeof(value));
		if (setting->type == _config_string)
		{
			size_t default_length = strlen(setting->default_value);

			value.string = default_length >= 2 ? config_copy(setting->default_value + 1, default_length - 2) : strdup("");
		}
		else
		{
			config_set_from_text(&value, setting->type, setting->default_value);
		}
		config_set_from_file(&value, setting, result.toptab);
		/* (the old string kept, as config_set_from_text keeps it) */
		config_values[index] = value;
	}
#if !defined(HALO_ANDROID) && !defined(HALO_WEB)
	/* (what the run forces, forced again: a server started by --server
	stays one whatever the file's server.enabled says, and its router asked
	as its server.upnp now says) */
	if (config_server_mode)
		config_apply_server_mode();
	else if (config_probe_mode)
		config_apply_probe_mode();
#endif
	config_change_count++;
	pthread_mutex_unlock(&config_lock);
	toml_free(result);
	free(text);
	config_check_words();
	return 1;
}

int config_default(const char *name, char *text, size_t size)
{
	long index = config_setting_index(name);
	const char *value;
	size_t length;

	if (index < 0)
		return 0;
	value = config_settings[index].default_value;
	length = strlen(value);
	/* (a string's without its quotes: the defaults have no escapes) */
	if (config_settings[index].type == _config_string && length >= 2 && value[0] == '"')
	{
		value++;
		length -= 2;
	}
	snprintf(text, size, "%.*s", (int)length, value);
	return 1;
}

int config_boolean(const char *name)
{
	return config_value(name, _config_boolean)->boolean;
}

int halo_fixed_step(void)
{
	static int fixed_step = -1;

	if (fixed_step < 0)
		fixed_step = config_boolean("debug.fixed_step") != 0;
	return fixed_step;
}

long config_integer(const char *name)
{
	return config_value(name, _config_integer)->integer;
}

unsigned long network_client_timeout_milliseconds(void)
{
	long seconds = config_integer("network.client_timeout_seconds");

	if (seconds < 5)
		seconds = 5;
	if (seconds > 300)
		seconds = 300;
	return (unsigned long)seconds * 1000UL;
}

double config_real(const char *name)
{
	return config_value(name, _config_real)->real;
}

const char *config_string(const char *name)
{
	const char *string = config_value(name, _config_string)->string;

	return string ? string : "";
}

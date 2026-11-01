/*
MENU_FUNCTIONS.C

The event handler functions the menus written in XML can run besides the
game's (menu_tags.c names them; ui_widget_event_handler_function_invoke
calls them from PC_MENU_FUNCTION_BASE on):

- "port quit game" (and the PC version's "main menu quit game") quits, as
  closing the window does;
- the PC version's "profile set edit begin" begins editing the first player
  profile, as its settings screens need, and fails with none: the main
  menu's then asks for a new one's name, as the menu's other items do, and
  its handlers open the screens that make the profile ("new campaign
  decision", after which Settings' is the one being edited);
- "gamespy screen init" hides the server browser's error and filter panels,
  and the title of the mode "mp type set mode" did not choose (Internet or
  LAN);
- its buttons' events: "mouse emit back event" and "mouse emit x event" push
  B and X (as the mouse does), "emit custom activation event" (an OK
  button) runs the screen's custom activation handler, "single prev cl item
  activated" (an item chosen) its list's, and its back handlers go back;
- "port setting load", on a spinner's created event, shows the value its
  setting has (or the nearest of its values): config.toml's, or for
  "profile.<field>" the controller's in the profile being edited;
- "port setting save", on its deleted event, writes the value shown back,
  if the player changed it;
- the settings screens' (tools/port_settings.py): "port settings save"
  (OK) writes those changed and applies the window's (the rest follow
  config.toml themselves), "port settings defaults" shows the defaults,
  "port settings help" the help of the row chosen (and Video Setup's
  Resolution or Window Size, for the display mode shown; Extra Effects' HDR
  Calibration only with an HDR display);
- HDR Calibration's: "port hdr calibration" has the renderer draw the
  pattern of the row chosen, at the peak and paper white its rows show,
  which the display takes while the screen is drawn;
- Controls Setup's: the keyboard and mouse's controls, two bindings each,
  shown a group at a time ("controls update menu"); "controls begin
  binding" takes the next key or button pressed for the binding left and
  right choose ("controls binding slot"); OK ("controls screen change set")
  writes them, "controls screen defaults" shows the defaults.

A spinner's values are its strings' (setting= and values= in its <widget>,
menu_tags.c's pc_menu_setting).

The PC version's campaign menus, on player 1's profile (the active one, else
the one last used, else the first) and the game's saved game in it:
- "campaign menu continue" goes on with the saved game;
- New Game's list of levels ("initialize sp level list solo", "solo map
  list update", "solo level set map") has those the profile has reached, and
  marks those it has finished on Normal, Heroic and Legendary, as the
  Xbox's list does; a level with the saved game in it goes on with it, at
  its difficulty. Its first row's chooser shows MULTIPLAYER's maps
  instead, one chosen played alone at once;
- the difficulty menu: "difficulty item select" (a difficulty chosen) and
  "set difficulty" (its OK button: the difficulty shown) start the game;
- Load Game's list ("load game menu init", "load game list update", "load
  game menu activated") has each profile's saved game, and "load game menu
  delete request" and "delete finish" delete one.
The Xbox's functions of those names take the Xbox's widgets (a spinner of
levels, the difficulty list itself), so ours run instead (menu_tags.c).

Co-op (the Xbox's Cooperative Play, Multiplayer's CO-OP CAMPAIGN): "port coop
begin" makes two players, player 1 on its profile; "port coop player 2"
gives player 2 the profile chosen and the controller that chose it; then the
campaign's New Game and difficulty start the game for both.

The rest are the PC version's, which its menus (port/assets/menus/ce)
name and the Xbox's has not: they do nothing yet, and succeed, so that what
their handlers open opens.
*/

#include "cseries.h"
#include "input/input.h"
#include "interface/event_manager.h"
#include "interface/player_ui.h"
#include "interface/ui_widget.h"
#include "interface/ui_widget_event_handler_functions.h"
#include "main/main.h"
#include "networking/network_game_manager.h"
#include "saved games/player_profile.h"
#include "tag_files/tag_groups.h"
#include "text/text_group.h"
#include "text/unicode.h"

#include "halo_menus.h"
#include "controller_remap.h"
#include "custom_edition_cache.h"
#include "custom_edition_maps.h"
#include "game_modes.h"
#include "network_voice.h"
#include "network_map_vote.h"
#include "text/draw_string.h"
/* (internet play's server browser: the platform layer's) */
#include "../src/p2p.h"
/* (the server browser's master servers, and Server Setup's publisher key) */
#include "../src/master_list.h"
#include "../src/player_link.h" /* port: port/linux/src/player_link.c */
#include "../src/server_master.h"
#include "text_filter.h"

#include <stdlib.h>
#include <string.h>
#include <xtl.h>

/* the platform layer's (port/linux/src) */
void platform_log(char const *format, ...);
void platform_request_quit(void);
char const *pc_menu_function_name(long function_index);
char const *pc_menu_game_data_input_name(long function_index);
void event_manager_post_button(short controller_index, short button_index);
int config_text(char const *name, char *text, unsigned int size);
int config_write(char const *name, char const *value);
int config_boolean(char const *name);
int config_default(char const *name, char *text, unsigned int size);
char const *config_string(char const *name);
void platform_display_apply(void);
int halo_hdr_calibration_available(void);
void halo_hdr_calibration_show(int pattern, int paper_white, int peak_brightness);
void platform_binding_capture_begin(void);
int platform_binding_capture_poll(int *input);
void platform_binding_capture_cancel(void);
void halo_input_name(int input, char *name, unsigned int size);
void halo_input_shown_name(char const *binding, char *shown, unsigned int size);
short pc_menu_string_index(long definition_index);
/* cseries_windows.c's */
unsigned long system_milliseconds(void);

/* the game's (port) */
boolean ui_widget_port_dispatch_event(struct widget_instance *widget, short event_type, short controller_index,
	boolean *deleted);
void ui_widget_port_go_back(struct widget_instance *widget);
short ui_widget_port_list_index(struct widget_instance *list_widget);
boolean ui_widget_port_saved_game(char const **map_name, short *level, short *difficulty);
char const *ui_widget_event_handler_function_name(long function_index);
short main_get_solo_level_from_name(char const *name);
boolean player_name_clean(wchar_t *name, long count);
short players_port_local_player_count(void);

boolean pc_menu_event_function_invoke(struct widget_instance *widget, struct event_record *event,
	long function_index, boolean *widget_deleted);
void pc_menu_game_data_function_invoke(struct widget_instance *widget, long function);

/* ---------- constants */

#define MAXIMUM_STRINGS 64
/* the PC version's custom activation event (this engine never sends it) */
#define EVENT_CUSTOM_ACTIVATION 32
#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_X 2
#define BUTTON_Y 3
#define BUTTON_START 12

enum
{
	_pc_menu_function_quit_game,
	_pc_menu_function_setting_load,
	_pc_menu_function_setting_save,
	NUMBER_OF_PC_MENU_FUNCTIONS
};

/* ---------- structures */

/* a widget, as ui_widget.c has it */
struct widget_instance
{
	long definition_tag_index;
	char const *name;
	short local_player_index;
	short horizontal_offset;
	short vertical_offset;
	short type;
	boolean visible;
	boolean render_regardless_of_controller_index;
	boolean disabled;
	boolean pause_game_time;
	boolean delete_recursion_lock;
	boolean widget_is_error_dialog;
	boolean close_if_local_player_controller_present;
	byte pad17;
	long creation_time;
	unsigned long milliseconds_to_auto_close;
	unsigned long auto_close_fade_time;
	real alpha_modifier;
	struct widget_instance *previous;
	struct widget_instance *next;
	struct widget_instance *parent;
	struct widget_instance *child;
	struct widget_instance *focused_child;
	union
	{
		struct
		{
			wchar_t *text;
			short string_list_index;
		} text_box;
		struct
		{
			short selected_index;
			short last_list_tab_direction;
			void *list_items;
			word number_of_items;
			struct widget_instance *extended_description;
			wchar_t *item_text;
		} list;
	} parameters;
	struct
	{
		short current_frame_index;
		short first_frame_index;
		short last_frame_index;
		short number_of_sprite_frames;
	} animation;
};

typedef char verify_widget_instance_size[
	sizeof(struct widget_instance) == 0x58 ? 1 : -1];
typedef char verify_widget_instance_child_offset[
	offsetof(struct widget_instance, child) == 0x34 ? 1 : -1];
typedef char verify_widget_instance_parent_offset[
	offsetof(struct widget_instance, parent) == 0x30 ? 1 : -1];
typedef char verify_widget_instance_selected_index_offset[
	offsetof(struct widget_instance, parameters.list.selected_index) == 0x3C ? 1 : -1];
typedef char verify_widget_instance_number_of_items_offset[
	offsetof(struct widget_instance, parameters.list.number_of_items) == 0x44 ? 1 : -1];
typedef char verify_widget_instance_animation_offset[
	offsetof(struct widget_instance, animation) == 0x50 ? 1 : -1];

/* menu_tags.c's */
struct pc_menu_setting
{
	long definition_index;
	char const *setting;
	long value_count;
	char const *values[MAXIMUM_STRINGS];
	short loaded_index;
};

struct pc_menu_setting *pc_menu_setting_get(long definition_index);

/* ---------- private code */

static boolean campaign_profile(short controller, struct player_profile *profile);

static boolean text_is_number(char const *text, double *number)
{
	char *end;

	*number = strtod(text, &end);
	return *text && !*end;
}

/* whether the widget is one of a spinner's items, which the game makes of
its strings from its own definition, handlers and all (ui_widget.c,
ui_widget_load_children_recursive) */
static boolean spinner_item(struct widget_instance const *widget)
{
	return widget->parent && widget->parent->definition_tag_index == widget->definition_tag_index;
}

/* the value shown for the setting's: the same one, else the nearest number;
a word with "-" before it (network.browser_sort's order reversed: "-players")
the word's */
static short setting_value_index(struct pc_menu_setting const *setting, char const *current)
{
	double current_number, value_number, distance = 0.0;
	short index, nearest = 0;

	for (index = 0; index < setting->value_count; index++)
	{
		if (!_stricmp(setting->values[index], current))
			return index;
	}
	if (current[0] == '-' && !text_is_number(current, &current_number))
	{
		for (index = 0; index < setting->value_count; index++)
		{
			if (!_stricmp(setting->values[index], current + 1))
				return index;
		}
	}
	if (!text_is_number(current, &current_number))
		return 0;
	for (index = 0; index < setting->value_count; index++)
	{
		if (text_is_number(setting->values[index], &value_number))
		{
			double gap = value_number > current_number ? value_number - current_number : current_number - value_number;

			if (index == 0 || gap < distance)
			{
				distance = gap;
				nearest = index;
			}
		}
	}
	return nearest;
}

/* the profile's settings ("profile.<field>"): those of the controller in
the profile being edited */
static struct
{
	char const *name;
	short field;
	/* its default (player_profile.c's new profile's) */
	byte default_value;
} const profile_settings[] =
{
	{ "profile.look_sensitivity", 0, 3 },
	{ "profile.invert_look", 1, FALSE },
	{ "profile.flight_inversion", 2, FALSE },
	{ "profile.autocenter", 3, FALSE },
	{ "profile.button_preset", 4, 0 },
	{ "profile.joystick_preset", 5, 0 },
	{ "profile.vibration", 6, TRUE },
	{ "profile.ingame_help", 7, TRUE },
};

static byte *profile_setting_field(struct player_profile *profile, short field)
{
	struct player_profile_controller_settings *controls = &profile->controller_settings;

	switch (field)
	{
	case 0: return &controls->look_sensitivity;
	case 1: return (byte *)&controls->invert_look;
	case 2: return (byte *)&controls->flight_stick_aircraft_controls;
	case 3: return (byte *)&controls->autocenter;
	case 4: return &controls->button_preset;
	case 5: return &controls->joystick_preset;
	case 6: return (byte *)&controls->vibration_disabled;
	case 7: return (byte *)&controls->ingame_help_disabled;
	}
	return NULL;
}

static boolean profile_setting_boolean(short field)
{
	return field != 0 && field != 4 && field != 5;
}

/* a setting's value as text, or (default_value) its default: config.toml's,
or the profile's */
static boolean setting_text(char const *name, char *text, unsigned int size, boolean default_value)
{
	short index;

	/* (DISCORD ACCOUNT's LINK: no change, until one is chosen; the master a
	server that refused this machine for its VPN is listed by, chosen
	already, player_link.c) */
	if (!strcmp(name, "link.action"))
	{
		int needed = default_value ? -1 : player_link_needed_master();

		if (needed >= 0)
			snprintf(text, size, "link_%d", needed + 1);
		else
			snprintf(text, size, "none");
		return TRUE;
	}
	if (strncmp(name, "profile.", 8))
	{
		if (!(default_value ? config_default(name, text, size) : config_text(name, text, size)))
			return FALSE;
		/* (display.mode empty: display.fullscreen's, as the window has it:
		sdl_platform.c) */
		if (!strcmp(name, "display.mode") && !text[0])
			snprintf(text, size, "%s", default_value || config_boolean("display.fullscreen") ? "borderless" : "windowed");
		/* (display.window_size empty: 640x480 times display.window_scale, as
		older versions set it) */
		if (!strcmp(name, "display.window_size") && !text[0])
		{
			char scale_text[32] = "";
			long scale;

			if (default_value)
				config_default("display.window_scale", scale_text, sizeof(scale_text));
			else
				config_text("display.window_scale", scale_text, sizeof(scale_text));
			scale = atol(scale_text);
			if (scale < 1)
				scale = 1;
			snprintf(text, size, "%ldx%ld", 640 * scale, 480 * scale);
		}
		return TRUE;
	}
	for (index = 0; index < NUMBEROF(profile_settings); index++)
	{
		struct player_profile *profile = player_ui_get_edit_player_profile();
		short field = profile_settings[index].field;
		long value;

		if (strcmp(name, profile_settings[index].name))
			continue;
		if (default_value)
			value = profile_settings[index].default_value;
		else if (profile && profile_setting_field(profile, field))
			value = *profile_setting_field(profile, field);
		else
			return FALSE;
		/* (vibration and help are kept as their opposites) */
		if (!default_value && (field == 6 || field == 7))
			value = !value;
		if (profile_setting_boolean(field))
			snprintf(text, size, "%s", value ? "true" : "false");
		else
			snprintf(text, size, "%ld", value);
		return TRUE;
	}
	return FALSE;
}

static boolean setting_write(char const *name, char const *value)
{
	short index;

	/* (DISCORD ACCOUNT's LINK: done, not written: the master's page opened
	in the browser, or the account forgotten, player_link.c) */
	if (!strcmp(name, "link.action"))
	{
		if (!strncmp(value, "link_", 5))
			player_link_begin(atoi(value + 5) - 1);
		else if (!strcmp(value, "unlink"))
			player_link_unlink();
		return TRUE;
	}
	if (strncmp(name, "profile.", 8))
		return config_write(name, value);
	for (index = 0; index < NUMBEROF(profile_settings); index++)
	{
		struct player_profile *profile = player_ui_get_edit_player_profile();
		short field = profile_settings[index].field;
		byte *place;
		long number;

		if (strcmp(name, profile_settings[index].name))
			continue;
		if (!profile || !(place = profile_setting_field(profile, field)))
			return FALSE;
		number = profile_setting_boolean(field) ? !strcmp(value, "true") : atol(value);
		if (field == 6 || field == 7)
			number = !number;
		*place = (byte)number;
		return TRUE;
	}
	return FALSE;
}

static boolean setting_load(struct widget_instance *widget)
{
	struct pc_menu_setting *setting = pc_menu_setting_get(widget->definition_tag_index);
	char current[300];

	if (spinner_item(widget))
		return TRUE;
	if (!setting || !setting_text(setting->setting, current, sizeof(current), FALSE))
		return FALSE;
	setting->loaded_index = setting_value_index(setting, current);
	if (setting->loaded_index < (short)widget->parameters.list.number_of_items)
		widget->parameters.list.selected_index = setting->loaded_index;
	/* (DISCORD ACCOUNT's LINK: an action, loaded as no change, so OK does
	the master chosen already, setting_text's) */
	if (!strcmp(setting->setting, "link.action"))
		setting->loaded_index = setting_value_index(setting, "none");
	return TRUE;
}

static boolean setting_save(struct widget_instance *widget)
{
	struct pc_menu_setting *setting = pc_menu_setting_get(widget->definition_tag_index);

	if (spinner_item(widget))
		return TRUE;
	if (!setting || setting->loaded_index == NONE || widget->parameters.list.selected_index < 0 ||
		widget->parameters.list.selected_index >= setting->value_count)
		return FALSE;
	if (widget->parameters.list.selected_index == setting->loaded_index)
		return TRUE;
	if (!config_write(setting->setting, setting->values[widget->parameters.list.selected_index]))
	{
		platform_log("menus: could not write %s to config.toml", setting->setting);
		return FALSE;
	}
	platform_log("menus: %s = %s (from the next start)", setting->setting, setting->values[widget->parameters.list.selected_index]);
	setting->loaded_index = widget->parameters.list.selected_index;
	return TRUE;
}

static short controller_of(struct widget_instance const *widget)
{
	return widget->local_player_index >= 0 && widget->local_player_index < 4 ? widget->local_player_index : 0;
}

/* the controller the event came from, else the widget's */
static short event_controller(struct widget_instance const *widget, struct event_record const *event)
{
	return event && event->controller_index >= 0 && event->controller_index < 4 ?
		event->controller_index : controller_of(widget);
}

/* runs the custom activation handler of the widget's list, or the nearest
of its parents' that has one (widget_deleted: the widget went with its
screen) */
static boolean item_activated(struct widget_instance *widget, short controller, boolean *widget_deleted)
{
	struct widget_instance *parent;

	for (parent = widget->parent; parent; parent = parent->parent)
	{
		boolean deleted;

		if (ui_widget_port_dispatch_event(parent, EVENT_CUSTOM_ACTIVATION, controller, &deleted))
		{
			if (deleted)
				*widget_deleted = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

/* whether the multiplayer menu's choice was LAN, not Internet (mp type set
mode) */
static boolean lan_mode;

/* hides the descendants of the widget with the name */
static void hide_named(struct widget_instance *widget, char const *name)
{
	struct widget_instance *child;

	for (child = widget->child; child; child = child->next)
	{
		if (!strcmp(child->name, name))
			child->visible = FALSE;
		hide_named(child, name);
	}
}

/* the server browser's panels for errors and filters are hidden until they
are wanted, and its title is Internet's or LAN's (gamespy screen init) */
static void server_browser_initialize(struct widget_instance *screen)
{
	struct widget_instance *child;

	hide_named(screen, lan_mode ? "header_internet" : "header_lan");
	/* (a widget's name is its definition's: the last part of ours) */
	for (child = screen->child; child; child = child->next)
	{
		if (!strcmp(child->name, "gamespy_error_fullscreen") || !strcmp(child->name, "filters_screen"))
		{
			child->visible = FALSE;
			if (screen->focused_child == child)
				screen->focused_child = NULL;
		}
	}
	for (child = screen->child; child && !screen->focused_child; child = child->next)
	{
		if (child->visible && child->type == 3)
			screen->focused_child = child;
	}
}

/* begins editing player 1's profile (as the campaign has it); FALSE if
there is none */
boolean pc_menu_profile_edit_begin(void)
{
	struct player_profile profile;

	if (!campaign_profile(0, &profile))
		return FALSE;
	player_ui_begin_editing_profile(player_ui_get_active_player_profile_index(0));
	return TRUE;
}

/* the game's own function of the name (as its tags name it), run for the
widget: for ours of a name of the game's, which do more than it */
static boolean game_function(char const *name, struct widget_instance *widget, struct event_record *event,
	boolean *widget_deleted)
{
	char const *function;
	long index;

	for (index = 0; (function = ui_widget_event_handler_function_name(index)) != NULL; index++)
	{
		if (!strcmp(function, name))
			return ui_widget_event_handler_function_invoke(widget, event, (word)index, widget_deleted);
	}
	return FALSE;
}

/* "profile set edit begin": Settings, the main menu's and a game's pause
menu's. With no profile, the main menu's asks for a new one's name, as the
menu's other items do ("new game if no plyr profiles"), and fails: the
screens its handler then opens make the profile and edit it
(new_profile_decision). Without the name asked, they made none, and showed
"new campaign decision" as failed. The pause menu's has no such screens,
and only fails. */
static boolean profile_edit_begin(struct widget_instance *widget, struct event_record *event,
	boolean *widget_deleted)
{
	if (pc_menu_profile_edit_begin())
		return TRUE;
	/* (a press: the profile is made on its controller) */
	if (main_menu_is_active() && event && event->controller_index >= 0 && event->controller_index < 4)
		game_function("new campaign chosen", widget, event, widget_deleted);
	return FALSE;
}

/* "new campaign decision" (the screen after a new profile's name, each of
the main menu's items having its own): the game's makes the profile, if the
name was entered. Settings' then begins editing it: the PC version's screens
open Settings with nothing more run, and its screen closes at once with no
profile being edited ("close if not editing profile"). */
static boolean new_profile_decision(struct widget_instance *widget, struct event_record *event,
	boolean *widget_deleted)
{
	if (!game_function("new campaign decision", widget, event, widget_deleted))
		return FALSE;
	return strcmp(widget->name, "settings_creating_profile") || pc_menu_profile_edit_begin();
}

/* ---------- the campaign */

/* the rows of its lists (main_menu/new_select/list_item_0 to 10) */
#define MAXIMUM_ROWS 11
/* its strings' and sp_levels' entries past the levels': a level not
reached, and (map_data) a saved game on the level */
#define LEVEL_UNAVAILABLE 10
#define LEVEL_IN_PROGRESS 11
#define ROW_TEXT_LENGTH 64
#define MAXIMUM_PROFILES 32
#define SOUND_ERROR 4
#define SOUND_CURSOR 1
#define SOUND_FORWARD 2
#define SOUND_BACK 3

/* (an index whose profile reads: saved_game_files.c's) */
#define PROFILE_VALID_BIT 0x80000000UL

struct campaign_level
{
	boolean available;
	/* finished on Normal, Heroic, Legendary (difficulty_options_small's
	frames 1 to 3) */
	boolean finished[3];
};

struct campaign_saved_game
{
	long profile_index;
	wchar_t profile_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH + 1];
	short level;
	short difficulty;
};

static struct
{
	struct campaign_level levels[NUMBER_OF_SINGLE_PLAYER_LEVELS];
	/* player 1's saved game's level, else NONE */
	short saved_level;
	/* the level and saved game the lists' descriptions last showed */
	short shown_level;
	short shown_saved_game;
	/* Load Game's */
	struct campaign_saved_game saved_games[MAXIMUM_ROWS];
	short saved_game_count;
	short saved_game_to_delete;
} campaign;

/* the descendant of the widget with the name (a widget's name is its
definition's: the last part of ours), the nth of them */
static struct widget_instance *descendant(struct widget_instance *widget, char const *name, long *nth)
{
	struct widget_instance *child;

	for (child = widget->child; child; child = child->next)
	{
		struct widget_instance *found;

		if (!strcmp(child->name, name) && (*nth)-- == 0)
			return child;
		found = descendant(child, name, nth);
		if (found)
			return found;
	}
	return NULL;
}

static struct widget_instance *named(struct widget_instance *widget, char const *name, long nth)
{
	return widget ? descendant(widget, name, &nth) : NULL;
}

/* the text box's own text (as the server list sets its items'), in a
buffer of length characters (made the first time: a text box is always
given the same length) */
static void text_set_length(struct widget_instance *text_box, wchar_t const *text, short length)
{
	if (!text_box)
		return;
	if (!text_box->parameters.text_box.text)
	{
		text_box->parameters.text_box.text = ui_widget_realloc(NULL, length * sizeof(wchar_t),
			__FILE__, __LINE__);
	}
	if (text_box->parameters.text_box.text)
	{
		ustrncpy(text_box->parameters.text_box.text, text, length - 1);
		text_box->parameters.text_box.text[length - 1] = 0;
	}
}

static void text_set(struct widget_instance *text_box, wchar_t const *text)
{
	text_set_length(text_box, text, ROW_TEXT_LENGTH);
}

/* one of our string lists' strings, on one line (its line breaks spaces),
of up to size characters */
static void words_get(char const *list, short index, wchar_t *text, short size)
{
	long tag_index = tag_loaded('ustr', list);
	wchar_t const *string = tag_index != NONE ? unicode_string_list_get_string(tag_index, index) : NULL;
	long length = 0;

	for (; string && *string && length < size - 1; string++)
	{
		wchar_t character = *string == '\r' || *string == '\n' ? ' ' : *string;

		if (character != ' ' || (length && text[length - 1] != ' '))
			text[length++] = character;
	}
	while (length && text[length - 1] == ' ')
		length--;
	text[length] = 0;
}

static void string_get(char const *list, short index, wchar_t *text)
{
	words_get(list, index, text, ROW_TEXT_LENGTH);
}

/* the list's row that has the focus, else NONE (its buttons) */
static short focused_row(struct widget_instance *list)
{
	struct widget_instance *child;
	short index = 0;

	for (child = list->child; child && index < MAXIMUM_ROWS; child = child->next, index++)
	{
		if (child == list->focused_child)
			return strncmp(child->name, "list_item_", 10) ? NONE : index;
	}
	return NONE;
}

static void focus_row(struct widget_instance *list, short row)
{
	struct widget_instance *child = list->child;
	short index;

	for (index = 0; child && child->next && index < row; index++)
		child = child->next;
	if (row == NONE)
	{
		/* (its buttons: the last child) */
		while (child && child->next)
			child = child->next;
	}
	if (child)
	{
		list->focused_child = child;
		list->parameters.list.selected_index = row == NONE ? 0 : row;
	}
}

/* shows the first count rows, with their texts, the arrows on the one that
has the focus, and no scroll buttons (the rows are enough) */
static void rows_update(struct widget_instance *list, short count, void (*row_text)(short row, wchar_t *text))
{
	struct widget_instance *row;
	short index = 0;

	for (row = list->child; row && index < MAXIMUM_ROWS; row = row->next, index++)
	{
		wchar_t text[ROW_TEXT_LENGTH];
		struct widget_instance *arrows = named(row, "list_item_arrows", 0);

		if (strncmp(row->name, "list_item_", 10))
			break;
		row->visible = index < count;
		if (index < count)
		{
			row_text(index, text);
			text_set(named(row, "list_item_text", 0), text);
		}
		if (arrows)
			arrows->visible = row == list->focused_child;
		if (named(row, "scroll_up_button", 0))
			named(row, "scroll_up_button", 0)->visible = FALSE;
		if (named(row, "scroll_down_button", 0))
			named(row, "scroll_down_button", 0)->visible = FALSE;
	}
}

/* player 1's profile, read again (the active one, else the one last used,
else the first), on the controller (in co-op, on the one that chose co-op:
coop_begin); FALSE if there is none */
static boolean campaign_profile(short controller, struct player_profile *profile)
{
	long profile_index = player_ui_get_active_player_profile_index(0);

	if (profile_index == NONE || !player_profile_get(profile_index, profile))
	{
		profile_index = player_ui_get_player1_last_used_profile_index();
		if (profile_index == NONE || !player_profile_get(profile_index, profile))
		{
			word count = 1;

			profile_index = NONE;
			player_profiles_enumerate_available_to_local_player_index(NONE, &count, &profile_index, FALSE);
			if ((short)count <= 0 || profile_index == NONE || !player_profile_get(profile_index, profile))
				return FALSE;
		}
	}
	player_ui_set_active_player_profile(0, profile_index, profile);
	if (player_spawn_count < 2)
		player_ui_set_single_player_local_player_controller(0, controller);
	return TRUE;
}

/* the levels the profile has reached (as the Xbox's list has them: those
it has played, the one after the last it finished, the first) and finished,
and its saved game's; in co-op, those either player's has reached and
finished, and no saved game (a game of one player's does not go on with
two: game_state.c's game_state_header_valid) */
static void campaign_levels_read(struct player_profile const *profile)
{
	char const *map_name;
	struct player_profile player2;
	short highest_level, highest_difficulty, difficulty, level, player;

	for (level = 0; level < NUMBER_OF_SINGLE_PLAYER_LEVELS; level++)
		memset(&campaign.levels[level], 0, sizeof(campaign.levels[level]));
	if (player_spawn_count >= 2)
		player_ui_get_active_player_profile(1, &player2);
	for (player = 0; player < (player_spawn_count >= 2 ? 2 : 1); player++)
	{
		struct player_profile const *reader = player == 0 ? profile : &player2;

		player_profile_get_highest_completed_solo_level((struct player_profile *)reader, &highest_level,
			&highest_difficulty);
		for (level = 0; level < NUMBER_OF_SINGLE_PLAYER_LEVELS; level++)
		{
			byte flags = reader->single_player_map_flags[level];
			short marker;

			campaign.levels[level].available |= flags || level == highest_level + 1 || level == 0;
			for (marker = 0; marker < 3; marker++)
				campaign.levels[level].finished[marker] |= (flags >> (marker + 1)) & 1;
		}
	}
	if (player_spawn_count >= 2 || !ui_widget_port_saved_game(&map_name, &campaign.saved_level, &difficulty))
		campaign.saved_level = NONE;
}

/* plays the map, at the difficulty (a saved game in it goes on: main.c's
main_new_map, if its difficulty is this one) */
static void campaign_start(char const *map_name, short difficulty, short controller)
{
	if (player_spawn_count < 2)
		player_ui_set_single_player_local_player_controller(0, controller);
	main_set_map_name(map_name);
	main_set_difficulty(difficulty);
	game_connection_set(0);
	main_menu_switch_to_single_player();
	player_ui_remember_player1_profile(TRUE);
	ui_play_audio_feedback_sound(SOUND_FORWARD);
}

static boolean campaign_fail(void)
{
	ui_play_audio_feedback_sound(SOUND_ERROR);
	return FALSE;
}

/* the description's level: its name, picture and words, and the
difficulties it has been finished on (finished: New Game's, NULL for a level
not reached), or (saved_difficulty, not NONE: Load Game's) the saved game's
difficulty */
static void level_description(struct widget_instance *description, char const *prefix, short level, boolean in_progress,
	boolean const *finished, short saved_difficulty)
{
	char name[64];
	struct widget_instance *widget;
	short marker;

	snprintf(name, sizeof(name), "%s_right_name", prefix);
	if ((widget = named(description, name, 0)) != NULL)
		widget->parameters.text_box.string_list_index = level;
	snprintf(name, sizeof(name), "%s_right_pic", prefix);
	if ((widget = named(description, name, 0)) != NULL)
		widget->animation.current_frame_index = level;
	snprintf(name, sizeof(name), "%s_right_data", prefix);
	if ((widget = named(description, name, 0)) != NULL && saved_difficulty != NONE)
	{
		wchar_t text[ROW_TEXT_LENGTH];

		string_get("pc\\main_menu\\player_profiles_select\\difficulty_names", saved_difficulty, text);
		text_set(widget, text);
	}
	else if (widget)
		widget->parameters.text_box.string_list_index = in_progress ? LEVEL_IN_PROGRESS : level;
	for (marker = 0; marker < 3; marker++)
	{
		if ((widget = named(description, "difficulty_indicator", marker)) != NULL)
		{
			widget->visible = saved_difficulty != NONE ? marker == 0 : finished && finished[marker];
			widget->animation.current_frame_index = saved_difficulty != NONE ? saved_difficulty : marker + 1;
		}
	}
}

/* player 1's profile's name (the active one, else the one used last: the
main menu clears the active ones), on the descriptions'
current_profile_name */
static void profile_name_show(struct widget_instance *description)
{
	/* (the profile read from its file at most once a second) */
	static struct player_profile profile;
	static long read_index = NONE;
	static unsigned long read_time;
	static boolean read_good;
	long index = player_ui_get_active_player_profile_index(0);

	if (!description)
		return;
	if (index != NONE)
	{
		player_ui_get_active_player_profile(0, &profile);
		read_index = NONE;
	}
	else if ((index = player_ui_get_player1_last_used_profile_index()) == NONE)
		return;
	else
	{
		/* (one that cannot be read too: tried again a second later) */
		if (index != read_index || system_milliseconds() - read_time > 1000)
		{
			read_index = index;
			read_good = player_profile_get(index, &profile);
			read_time = system_milliseconds();
		}
		if (!read_good)
			return;
	}
	text_set(named(description, "current_profile_name", 0), profile.player_name);
}

/* (Campaign's menu) "campaign menu init": player 1's profile and saved
game, for its items */
static boolean campaign_menu_initialize(short controller)
{
	struct player_profile profile;

	if (campaign_profile(controller, &profile))
		campaign_levels_read(&profile);
	return TRUE;
}

/* "campaign menu continue": the saved game goes on */
static boolean campaign_continue(short controller)
{
	struct player_profile profile;
	char const *map_name;
	short level, difficulty;

	if (!campaign_profile(controller, &profile) || !ui_widget_port_saved_game(&map_name, &level, &difficulty))
		return campaign_fail();
	campaign_start(map_name, difficulty, controller);
	return TRUE;
}

/* ---- the map lists (New Game's and the Map screen's): their first row's
chooser of SINGLEPLAYER (the campaign's levels), MULTIPLAYER maps (the
Xbox's), CUSTOM SINGLEPLAYER or CUSTOM MULTIPLAYER maps (the Custom Edition
maps of the custom_maps folder, by the scenario type their files give:
custom_edition_maps.c) (port_settings.MAP_KIND_CHOOSER), then the kind's
rows, scrolling as list_scroll scrolls. A singleplayer kind's map is played
as the campaign is (alone, or hosted as network co-op), a multiplayer kind's
as multiplayer maps are. */

enum
{
	MAP_KIND_SINGLEPLAYER,
	MAP_KIND_MULTIPLAYER,
	MAP_KIND_CUSTOM_SINGLEPLAYER,
	MAP_KIND_CUSTOM_MULTIPLAYER,
	NUMBER_OF_MAP_KINDS
};

/* the rows after the chooser */
#define MAP_KIND_ROWS 10

static void visible_set(struct widget_instance *widget, boolean visible);
short ui_widget_port_multiplayer_maps(char const *const **names, short *last_used);

static boolean map_kind_singleplayer(short kind)
{
	return kind == MAP_KIND_SINGLEPLAYER || kind == MAP_KIND_CUSTOM_SINGLEPLAYER;
}

/* The kind the chooser shows, kept to the multiplayer kinds unless
`singleplayer`: a singleplayer kind it was turned to becomes the next
multiplayer one the way it turned from `previous`. */
static short map_kind_shown(struct widget_instance *list, boolean singleplayer, short previous)
{
	struct widget_instance *spinner = named(list, "list_item_0_map_kind_spinner", 0);
	short kind;

	if (!spinner)
		return singleplayer ? MAP_KIND_SINGLEPLAYER : MAP_KIND_MULTIPLAYER;
	kind = (short)PIN(spinner->parameters.list.selected_index, 0, NUMBER_OF_MAP_KINDS - 1);
	if (!singleplayer && map_kind_singleplayer(kind))
	{
		short step = kind == (previous + NUMBER_OF_MAP_KINDS - 1) % NUMBER_OF_MAP_KINDS ? -1 : 1;

		do
			kind = (short)((kind + step + NUMBER_OF_MAP_KINDS) % NUMBER_OF_MAP_KINDS);
		while (map_kind_singleplayer(kind));
		spinner->parameters.list.selected_index = kind;
	}
	return kind;
}

static void map_kind_set(struct widget_instance *list, short kind)
{
	struct widget_instance *spinner = named(list, "list_item_0_map_kind_spinner", 0);

	if (spinner)
		spinner->parameters.list.selected_index = kind;
}

/* the first of count entries shown, for the entry chosen to be in the middle */
static short map_kind_first(short chosen, short count)
{
	return (short)PIN(chosen - MAP_KIND_ROWS / 2, 0, MAX(count - MAP_KIND_ROWS, 0));
}

static boolean list_scrolls_at_end(void);

static void map_kind_focus(struct widget_instance *list, short first, short chosen)
{
	focus_row(list, (short)(1 + chosen - first));
}

/* the rows after the chooser: count entries from *first's, scrolled on at
their ends; the entry focused, NONE when the chooser or the buttons have the
focus */
static short map_kind_rows_update(struct widget_instance *list, short *first, short count,
	void (*entry_text)(short entry, wchar_t *text))
{
	short shown = (short)MIN(count, MAP_KIND_ROWS);
	short row = focused_row(list), index = 0;
	struct widget_instance *child;

	if (row != NONE && row >= 1 && row == shown && *first + shown < count && list_scrolls_at_end())
	{
		(*first)++;
		focus_row(list, --row);
	}
	else if (row == 1 && *first > 0 && list_scrolls_at_end())
	{
		(*first)--;
		focus_row(list, ++row);
	}
	for (child = list->child; child; child = child->next, index++)
	{
		wchar_t text[ROW_TEXT_LENGTH];

		if (index < 1)
			continue;
		if (strncmp(child->name, "list_item_", 10))
			break;
		child->visible = index - 1 < shown;
		if (index - 1 < shown)
		{
			entry_text((short)(*first + index - 1), text);
			text_set(named(child, "list_item_text", 0), text);
		}
		visible_set(named(child, "list_item_arrows", 0), child == list->focused_child);
		visible_set(named(child, "scroll_up_button", 0), FALSE);
		visible_set(named(child, "scroll_down_button", 0), FALSE);
	}
	return row == NONE || row < 1 || row > shown ? NONE : (short)(*first + row - 1);
}

/* the Xbox's multiplayer maps: the first of the multiplayer maps
(ui_widget_port_multiplayer_maps), which go on with the Custom Edition ones */
static short xbox_multiplayer_map_count(short multiplayer_map_count)
{
	return (short)MAX(multiplayer_map_count - custom_edition_maps_count(FALSE), 0);
}

static void multiplayer_map_text(short map, wchar_t *text)
{
	string_get("pc\\main_menu\\mp_map_list", map, text);
}

/* a Custom Edition map's name (the menus find its name, picture and
description by its display index: custom_edition_maps.c) */
static void custom_map_text(boolean campaign, short map, wchar_t *text)
{
	wchar_t const *name = custom_edition_maps_name(custom_edition_maps_display_index_of(campaign, map));

	ustrncpy(text, name ? name : L"", ROW_TEXT_LENGTH - 1);
	text[ROW_TEXT_LENGTH - 1] = 0;
}

static void custom_campaign_map_text(short map, wchar_t *text)
{
	custom_map_text(TRUE, map, text);
}

static void custom_multiplayer_map_text(short map, wchar_t *text)
{
	custom_map_text(FALSE, map, text);
}

/* the description's map (each list's has both kinds' widgets): a campaign
level's picture, name and words (level_description sets them), or a map's
by its display index (a multiplayer map's, or a Custom Edition map's), or
(both NONE) neither */
static void map_description_show(struct widget_instance *description, short level, short map)
{
	struct widget_instance *widget;
	short marker;

	visible_set(named(description, "replay_level_right_name", 0), level != NONE);
	visible_set(named(description, "replay_level_right_pic", 0), level != NONE);
	visible_set(named(description, "replay_level_right_data", 0), level != NONE);
	visible_set(named(description, "mp_map_right_name", 0), map != NONE);
	visible_set(named(description, "mp_map_right_pic", 0), map != NONE);
	visible_set(named(description, "mp_map_right_data", 0), map != NONE);
	if (level == NONE)
	{
		for (marker = 0; marker < 3; marker++)
			visible_set(named(description, "difficulty_indicator", marker), FALSE);
	}
	if (map == NONE)
		return;
	if ((widget = named(description, "mp_map_right_name", 0)) != NULL)
		widget->parameters.text_box.string_list_index = map;
	if ((widget = named(description, "mp_map_right_pic", 0)) != NULL)
		widget->animation.current_frame_index = map;
	if ((widget = named(description, "mp_map_right_data", 0)) != NULL)
		widget->parameters.text_box.string_list_index = map;
}

/* New Game's list: SINGLEPLAYER's levels, the profile's reached, or
MULTIPLAYER's maps, played alone to walk around (no game engine: a campaign
game on the map); CUSTOM SINGLEPLAYER's maps played as the campaign's levels
are, at the difficulty chosen next, and CUSTOM MULTIPLAYER's walked around
as MULTIPLAYER's are */
static struct
{
	short kind;
	short first, chosen;
	/* the multiplayer maps (ui_widget_port_multiplayer_maps) */
	char const *const *map_names;
	short map_count;
} level_list;

/* a map list's entries of a kind (the Map screen's difficulties aside) */
static short map_kind_count(short kind, short multiplayer_map_count)
{
	switch (kind)
	{
	case MAP_KIND_SINGLEPLAYER:
		return NUMBER_OF_SINGLE_PLAYER_LEVELS;
	case MAP_KIND_MULTIPLAYER:
		return xbox_multiplayer_map_count(multiplayer_map_count);
	case MAP_KIND_CUSTOM_SINGLEPLAYER:
		return custom_edition_maps_count(TRUE);
	default:
		return custom_edition_maps_count(FALSE);
	}
}

/* the display index of a multiplayer or Custom Edition kind's entry, for its
description (map_description_show) */
static short map_kind_display_index(short kind, short entry)
{
	switch (kind)
	{
	case MAP_KIND_CUSTOM_SINGLEPLAYER:
		return custom_edition_maps_display_index_of(TRUE, entry);
	case MAP_KIND_CUSTOM_MULTIPLAYER:
		return custom_edition_maps_display_index_of(FALSE, entry);
	default:
		return entry;
	}
}

/* the level name of a Custom Edition kind's entry, or NULL */
static char const *custom_map_level_name(short kind, short entry)
{
	return custom_edition_maps_level_name(map_kind_display_index(kind, entry));
}

/* "initialize sp level list solo" starts on the level last played */
static boolean level_list_initialize(struct widget_instance *list, short controller)
{
	struct player_profile profile;
	short level, last_used;

	if (!campaign_profile(controller, &profile))
		return FALSE;
	campaign_levels_read(&profile);
	level = PIN(profile.last_single_player_map_played, 0, NUMBER_OF_SINGLE_PLAYER_LEVELS - 1);
	if (campaign.saved_level != NONE)
		level = campaign.saved_level;
	if (!campaign.levels[level].available)
		level = 0;
	campaign.shown_level = level;
	level_list.map_count = ui_widget_port_multiplayer_maps(&level_list.map_names, &last_used);
	level_list.kind = MAP_KIND_SINGLEPLAYER;
	level_list.chosen = level;
	level_list.first = map_kind_first(level, NUMBER_OF_SINGLE_PLAYER_LEVELS);
	map_kind_set(list, level_list.kind);
	map_kind_focus(list, level_list.first, level_list.chosen);
	return TRUE;
}

static void level_row_text(short level, wchar_t *text)
{
	string_get("pc\\main_menu\\map_list", campaign.levels[level].available ? level : LEVEL_UNAVAILABLE, text);
}

/* "solo map list update" */
static void level_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	short kind = map_kind_shown(list, TRUE, level_list.kind), count, entry, level;

	if (kind != level_list.kind)
	{
		level_list.kind = kind;
		level_list.first = level_list.chosen = 0;
	}
	count = map_kind_count(kind, level_list.map_count);
	entry = map_kind_rows_update(list, &level_list.first, count,
		kind == MAP_KIND_SINGLEPLAYER ? level_row_text :
		kind == MAP_KIND_MULTIPLAYER ? multiplayer_map_text :
		kind == MAP_KIND_CUSTOM_SINGLEPLAYER ? custom_campaign_map_text : custom_multiplayer_map_text);
	if (entry != NONE)
		level_list.chosen = entry;
	if (kind != MAP_KIND_SINGLEPLAYER)
	{
		map_description_show(description, NONE,
			level_list.chosen < count ? map_kind_display_index(kind, level_list.chosen) : NONE);
		profile_name_show(description);
		return;
	}
	map_description_show(description, 0, NONE);
	campaign.shown_level = level_list.chosen;
	level = campaign.shown_level;
	if (!campaign.levels[level].available)
		level = LEVEL_UNAVAILABLE;
	level_description(description, "replay_level", level, level == campaign.saved_level,
		level == LEVEL_UNAVAILABLE ? NULL : campaign.levels[level].finished, NONE);
	profile_name_show(description);
}

/* "solo level set map": the level shown, if reached, or the Custom Edition
campaign map, for the difficulty menu; or the multiplayer map, played at
once (FALSE: no difficulty menu). The list is read again first: a tap on
the touchscreen focuses a row and activates it at once, before the list's
update has read which row it is (it started the level shown before, Pillar
of Autumn on a new profile). The map and gametype lists do the same. */
static boolean level_choose(struct widget_instance *list, short controller)
{
	short level, count;

	level_list_update(list);
	level = campaign.shown_level;
	count = map_kind_count(level_list.kind, level_list.map_count);

	if (level_list.kind != MAP_KIND_SINGLEPLAYER && (level_list.chosen < 0 || level_list.chosen >= count))
		return campaign_fail();
	if (level_list.kind == MAP_KIND_MULTIPLAYER || level_list.kind == MAP_KIND_CUSTOM_MULTIPLAYER)
	{
		struct player_profile profile;
		char const *map_name = level_list.kind == MAP_KIND_MULTIPLAYER ? level_list.map_names[level_list.chosen] :
			custom_map_level_name(level_list.kind, level_list.chosen);

		if (!map_name || !campaign_profile(controller, &profile))
			return campaign_fail();
		campaign_start(map_name, main_get_difficulty(), controller);
		return FALSE;
	}
	if (level_list.kind == MAP_KIND_CUSTOM_SINGLEPLAYER)
	{
		char const *map_name = custom_map_level_name(level_list.kind, level_list.chosen);

		if (!map_name)
			return campaign_fail();
		main_set_map_name(map_name);
		main_defer_map_map_change();
		return TRUE;
	}
	if (level < 0 || level >= NUMBER_OF_SINGLE_PLAYER_LEVELS || !campaign.levels[level].available)
		return campaign_fail();
	/* (not yet: setting it at the main menu changes map, as the Xbox's has it
	not) */
	main_set_map_name(main_get_solo_level_name(level));
	main_defer_map_map_change();
	return TRUE;
}

/* the difficulty menu: "difficulty item select" starts the game at the
item's; "set difficulty" (its OK button) at the one its description shows */
static boolean difficulty_start(short difficulty, short controller)
{
	struct player_profile profile;
	char const *map_name = main_get_map_name();

	if (!campaign_profile(controller, &profile))
		return campaign_fail();
	/* (a campaign level, or a Custom Edition campaign map) */
	if (!custom_edition_maps_level_campaign(map_name))
		map_name = main_get_solo_level_name(0);
	campaign_start(map_name, PIN(difficulty, 0, 3), controller);
	return TRUE;
}

static short difficulty_shown(struct widget_instance *widget)
{
	for (; widget; widget = widget->parent)
	{
		if (!strcmp(widget->name, "difficulty_select_list") && widget->parameters.list.extended_description)
		{
			struct widget_instance *picture = named(widget->parameters.list.extended_description,
				"difficulty_options", 0);

			return picture ? picture->animation.current_frame_index : main_get_difficulty();
		}
	}
	return main_get_difficulty();
}

static short sibling_index(struct widget_instance *widget)
{
	struct widget_instance *child;
	short index = 0;

	for (child = widget->parent ? widget->parent->child : widget; child && child != widget; child = child->next)
		index++;
	return index;
}

/* Load Game's list: each profile's saved game (read with the profile as
player 1's, which then is again) */
static void saved_games_read(short controller)
{
	struct player_profile profile, active;
	long profiles[MAXIMUM_PROFILES];
	long active_index;
	word count = MAXIMUM_PROFILES;
	short index;

	campaign.saved_game_count = 0;
	if (!campaign_profile(controller, &active))
		return;
	active_index = player_ui_get_active_player_profile_index(0);
	player_profiles_enumerate_available_to_local_player_index(NONE, &count, profiles, FALSE);
	for (index = 0; index < (short)count && campaign.saved_game_count < MAXIMUM_ROWS; index++)
	{
		struct campaign_saved_game *saved_game = &campaign.saved_games[campaign.saved_game_count];
		char const *map_name;

		if (!((unsigned long)profiles[index] & PROFILE_VALID_BIT) || !player_profile_get(profiles[index], &profile))
			continue;
		player_ui_set_active_player_profile(0, profiles[index], &profile);
		if (ui_widget_port_saved_game(&map_name, &saved_game->level, &saved_game->difficulty))
		{
			saved_game->profile_index = profiles[index];
			ustrncpy(saved_game->profile_name, profile.player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
			saved_game->profile_name[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = 0;
			campaign.saved_game_count++;
		}
	}
	player_ui_set_active_player_profile(0, active_index, &active);
	campaign_levels_read(&active);
}

/* "load game menu init" */
static boolean saved_game_list_initialize(struct widget_instance *list, short controller)
{
	short index;

	saved_games_read(controller);
	campaign.shown_saved_game = 0;
	for (index = 0; index < campaign.saved_game_count; index++)
	{
		if (campaign.saved_games[index].profile_index == player_ui_get_active_player_profile_index(0))
			campaign.shown_saved_game = index;
	}
	focus_row(list, campaign.saved_game_count ? campaign.shown_saved_game : NONE);
	return TRUE;
}

static void saved_game_row_text(short row, wchar_t *text)
{
	ustrncpy(text, campaign.saved_games[row].profile_name, ROW_TEXT_LENGTH - 1);
	text[ROW_TEXT_LENGTH - 1] = 0;
}

/* "load game list update" */
static void saved_game_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *item = named(description, "load_level_right_item", 0);
	short row = focused_row(list);

	rows_update(list, campaign.saved_game_count, saved_game_row_text);
	if (row != NONE && row < campaign.saved_game_count)
		campaign.shown_saved_game = row;
	if (campaign.shown_saved_game >= campaign.saved_game_count)
		campaign.shown_saved_game = campaign.saved_game_count - 1;
	if (item)
		item->visible = campaign.saved_game_count > 0;
	if (campaign.saved_game_count > 0)
	{
		struct campaign_saved_game const *saved_game = &campaign.saved_games[campaign.shown_saved_game];

		level_description(description, "load_level", saved_game->level, FALSE, NULL, saved_game->difficulty);
	}
	else if (row != NONE)
	{
		focus_row(list, NONE);
	}
	profile_name_show(description);
}

/* "load game menu activated": the saved game goes on, with its profile as
player 1's */
static boolean saved_game_continue(short controller)
{
	struct campaign_saved_game const *saved_game;
	struct player_profile profile;

	if (campaign.shown_saved_game < 0 || campaign.shown_saved_game >= campaign.saved_game_count)
		return campaign_fail();
	saved_game = &campaign.saved_games[campaign.shown_saved_game];
	if (!player_profile_get(saved_game->profile_index, &profile))
		return campaign_fail();
	player_ui_set_active_player_profile(0, saved_game->profile_index, &profile);
	campaign_start(main_get_solo_level_name(saved_game->level), saved_game->difficulty, controller);
	return TRUE;
}

/* "load game menu delete request" (before its question) and "delete
finish" (its OK): the saved game shown goes */
static boolean saved_game_delete_request(void)
{
	if (campaign.shown_saved_game < 0 || campaign.shown_saved_game >= campaign.saved_game_count)
		return campaign_fail();
	campaign.saved_game_to_delete = campaign.shown_saved_game;
	return TRUE;
}

static boolean saved_game_delete(short controller)
{
	char path[256];

	if (campaign.saved_game_to_delete < 0 || campaign.saved_game_to_delete >= campaign.saved_game_count ||
		!player_profile_get_enclosing_directory_path(campaign.saved_games[campaign.saved_game_to_delete].profile_index,
			path) || strlen(path) + strlen("savegame.bin") >= sizeof(path))
	{
		return campaign_fail();
	}
	strcat(path, "savegame.bin");
	if (!DeleteFileA(path))
		platform_log("menus: could not delete %s", path);
	campaign.saved_game_to_delete = NONE;
	saved_games_read(controller);
	return TRUE;
}

/* the settings' screens (tools/port_settings.py): every spinner of a
setting in the screen */
static void settings_each(struct widget_instance *widget, boolean (*visit)(struct widget_instance *spinner,
	struct pc_menu_setting *setting))
{
	struct widget_instance *child;

	for (child = widget->child; child; child = child->next)
	{
		struct pc_menu_setting *setting = child->type == 2 && !spinner_item(child) ?
			pc_menu_setting_get(child->definition_tag_index) : NULL;

		if (setting)
			visit(child, setting);
		else
			settings_each(child, visit);
	}
}

static struct widget_instance *screen_of(struct widget_instance *widget)
{
	while (widget->parent)
		widget = widget->parent;
	return widget;
}

/* "port settings save" (OK): those changed written, and applied */
static boolean setting_changed_save(struct widget_instance *spinner, struct pc_menu_setting *setting)
{
	short index = spinner->parameters.list.selected_index;

	if (index < 0 || index >= setting->value_count || index == setting->loaded_index)
		return TRUE;
	if (!setting_write(setting->setting, setting->values[index]))
	{
		platform_log("menus: could not set %s", setting->setting);
		return FALSE;
	}
	platform_log("menus: %s = %s", setting->setting, setting->values[index]);
	setting->loaded_index = index;
	return TRUE;
}

/* "port settings defaults" */
static boolean setting_default_show(struct widget_instance *spinner, struct pc_menu_setting *setting)
{
	char text[300];

	if (setting_text(setting->setting, text, sizeof(text), TRUE))
		spinner->parameters.list.selected_index = setting_value_index(setting, text);
	return TRUE;
}

/* "port settings help": the line of the row chosen (by its label's string:
the buttons' is the first) */
static void settings_help(struct widget_instance *list)
{
	struct widget_instance *help = list->parameters.list.extended_description;
	struct widget_instance *row = list->focused_child;
	short index = 0;

	if (!help)
		return;
	if (row && row->child && row->child->type == 1 && strncmp(row->name, "button", 6))
	{
		short label = pc_menu_string_index(row->child->definition_tag_index);

		if (label != NONE)
			index = label + 1;
	}
	help->parameters.text_box.string_list_index = index;
}

/* Video Setup's Resolution and Window Size, in the one row's place
(tools/port_settings.py): the one the display mode shown uses (Window Size
the window's, Resolution fullscreen's and borderless's) is shown, and the
list passes over the other, which takes no focus while it is hidden */
static void video_rows_show(struct widget_instance *list)
{
	struct widget_instance *mode = named(list, "mode_spinner", 0);
	struct widget_instance *resolution = named(list, "op_resolution", 0);
	struct widget_instance *window_size = named(list, "op_window_size", 0);
	struct pc_menu_setting *setting = mode ? pc_menu_setting_get(mode->definition_tag_index) : NULL;
	struct widget_instance *shown, *child;
	short index;

	if (!setting || !resolution || !window_size)
		return;
	index = mode->parameters.list.selected_index;
	shown = index >= 0 && index < setting->value_count && !strcmp(setting->values[index], "windowed") ?
		window_size : resolution;
	resolution->visible = shown == resolution;
	window_size->visible = shown == window_size;
	/* (the focus on the one hidden goes to the one shown: Defaults can
	change the mode while it has it) */
	if (list->focused_child == (shown == resolution ? window_size : resolution))
	{
		for (index = 0, child = list->child; child && child != shown; child = child->next)
			index++;
		list->focused_child = shown;
		list->parameters.list.selected_index = index;
	}
}

/* Network Setup's SERVER BROWSER's MASTER SERVERS (tools/port_settings.py:
its words): how many of masters.txt's answered when the browser last asked,
"<answered> OF <count>"; none listed, not asked yet, or off (network.listings
is "brokers"). masters.txt is read again every few seconds */
#define SERVER_BROWSER_WORDS \
	"pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\network_setup\\server_browser\\words"
enum
{
	_master_servers_word_of,
	_master_servers_word_none,
	_master_servers_word_not_asked,
	_master_servers_word_off,
};

static void master_servers_show(struct widget_instance *list)
{
	static int listed;
	static unsigned long read_time;
	static boolean read;
	struct widget_instance *value = named(list, "master_servers_value", 0);
	wchar_t text[ROW_TEXT_LENGTH], word[ROW_TEXT_LENGTH];
	int count, answered, asked;

	if (!value)
		return;
	if (!read || system_milliseconds() - read_time > 3000)
	{
		static char urls[MASTER_LIST_MAXIMUM_MASTERS][MASTER_LIST_URL_SIZE];
		static unsigned char keys[MASTER_LIST_MAXIMUM_MASTERS][MASTER_LIST_KEY_SIZE];

		listed = master_list_read(config_string("network.masters_file"), urls, keys, MASTER_LIST_MAXIMUM_MASTERS);
		read_time = system_milliseconds();
		read = TRUE;
	}
	master_list_counts(&count, &answered, &asked);
	if (master_list_policy() == _master_listings_brokers)
		words_get(SERVER_BROWSER_WORDS, _master_servers_word_off, text, NUMBEROF(text));
	else if (!listed)
		words_get(SERVER_BROWSER_WORDS, _master_servers_word_none, text, NUMBEROF(text));
	else if (!asked)
		words_get(SERVER_BROWSER_WORDS, _master_servers_word_not_asked, text, NUMBEROF(text));
	else
	{
		words_get(SERVER_BROWSER_WORDS, _master_servers_word_of, word, NUMBEROF(word));
		usnprintf(text, NUMBEROF(text) - 1, L"%d %s %d", answered, word, count);
		text[NUMBEROF(text) - 1] = 0;
	}
	text_set(value, text);
}

/* Network Setup's DISCORD ACCOUNT (tools/port_settings.py: its words): the
account, "LINKED AS <name> (<n> OF <masters>)", the link page's code while it
waits, revoked, expired (the passes ran out and no master answered), or not
linked; and a row a master, "<host>: <its state>", a server that refused
this machine for its VPN marking the one to link with (player_link.c) */
#define DISCORD_ACCOUNT_WORDS \
	"pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\network_setup\\discord_account\\words"
enum
{
	_discord_account_word_none,
	_discord_account_word_linked,
	_discord_account_word_waiting,
	_discord_account_word_revoked,
	_discord_account_word_expired,
	_discord_account_word_of,
	_discord_account_word_needed,
	_discord_account_word_no_master,
};

/* text widened into a wide one (ASCII) */
static void discord_account_widen(char const *text, wchar_t *wide, long size)
{
	long index;

	for (index = 0; text[index] && index < size - 1; index++)
		wide[index] = (wchar_t)(unsigned char)text[index];
	wide[index] = 0;
}

/* a state's word and its detail (a name, a code), into text */
static void discord_account_state_text(int state, char const *name, char const *code, wchar_t *text, long size)
{
	wchar_t word[ROW_TEXT_LENGTH], detail[48];
	short word_index = state == _player_link_state_linked ? _discord_account_word_linked :
		state == _player_link_state_waiting ? _discord_account_word_waiting :
		state == _player_link_state_revoked ? _discord_account_word_revoked :
		state == _player_link_state_expired ? _discord_account_word_expired :
		state == _player_link_state_needed ? _discord_account_word_needed : _discord_account_word_none;

	words_get(DISCORD_ACCOUNT_WORDS, word_index, word, NUMBEROF(word));
	discord_account_widen(state == _player_link_state_waiting ? code : state == _player_link_state_linked ||
		state == _player_link_state_expired ? name : "", detail, NUMBEROF(detail));
	if (detail[0])
		usnprintf(text, size - 1, L"%s %s", word, detail);
	else
		usnprintf(text, size - 1, L"%s", word);
	text[size - 1] = 0;
}

static void discord_account_show(struct widget_instance *list)
{
	struct widget_instance *value = named(list, "discord_account_value", 0);
	wchar_t text[ROW_TEXT_LENGTH], state_text[ROW_TEXT_LENGTH], word[ROW_TEXT_LENGTH], wide[96];
	char name[PLAYER_LINK_NAME_SIZE + 1], code[PLAYER_LINK_CODE_SIZE], host[80], row[32];
	long master;
	int state, linked, masters;

	if (!value)
		return;
	state = player_link_state(name, sizeof(name), code, sizeof(code), &linked, &masters);
	discord_account_state_text(state, name, code, text, NUMBEROF(text));
	/* ("(1 OF 2)": the masters it is linked with) */
	if (state == _player_link_state_linked && masters > 1)
	{
		words_get(DISCORD_ACCOUNT_WORDS, _discord_account_word_of, word, NUMBEROF(word));
		usnprintf(state_text, NUMBEROF(state_text) - 1, L"%s (%d %s %d)", text, linked, word, masters);
		state_text[NUMBEROF(state_text) - 1] = 0;
		text_set(value, state_text);
	}
	else
	{
		text_set(value, text);
	}
	for (master = 0; master < PLAYER_LINK_MAXIMUM_MASTERS; master++)
	{
		struct widget_instance *row_value;

		snprintf(row, sizeof(row), "link_master_%ld_value", master + 1);
		row_value = named(list, row, 0);
		if (!row_value)
			continue;
		state = player_link_master_state((int)master, host, sizeof(host), name, sizeof(name), code, sizeof(code));
		if (!host[0])
		{
			words_get(DISCORD_ACCOUNT_WORDS, _discord_account_word_no_master, text, NUMBEROF(text));
			text_set(row_value, text);
			continue;
		}
		discord_account_state_text(state, name, code, state_text, NUMBEROF(state_text));
		discord_account_widen(host, wide, NUMBEROF(wide));
		usnprintf(text, NUMBEROF(text) - 1, L"%s: %s", wide, state_text);
		text[NUMBEROF(text) - 1] = 0;
		text_set(row_value, text);
	}
}

/* Graphics' button style follows the pending HUD choice, including Defaults.
The remaining options close the gap; the help and bottom buttons stay put. */
/* Button Appearance filters its rows in the controller feature module. */
#include "controller_buttons.h"
#include "controller_button_menu.inl"

static void hud_button_style_row_show(struct widget_instance *list)
{
	struct widget_instance *hud = named(list, "high_res_hud_spinner", 0);
	struct widget_instance *row = named(list, "op_hud_button_style", 0);
	struct pc_menu_setting *setting = hud ? pc_menu_setting_get(hud->definition_tag_index) : NULL;
	struct widget_instance *child;
	short index;
	boolean visible;

	if (named(list, "op_auto_button_detection", 0))
	{
		controller_button_rows_show(list);
		return;
	}
	if (!setting || !row) return;
	index = hud->parameters.list.selected_index;
	visible = index >= 0 && index < setting->value_count && !strcmp(setting->values[index], "true");
	if (row->visible != visible)
	{
		for (child = row->next; child; child = child->next)
			if (!strncmp(child->name, "op_", 3))
				child->vertical_offset += visible ? 30 : -30;
		row->visible = visible;
	}
	if (!visible && list->focused_child == row)
	{
		for (index = 0, child = list->child; child && child != hud->parent; child = child->next)
			index++;
		list->focused_child = hud->parent;
		list->parameters.list.selected_index = index;
	}
}

/* ---------- HDR Calibration (tools/port_settings.py): Extra Effects' row
for it is there only with an HDR display (display.hdr_output's window, the
system's HDR on); on its screen the renderer draws the pattern of the row
chosen, at the values the rows show, and the display takes them until the
screen closes (d3d8_gl.c). OK writes them, as any settings screen's does. */

/* d3d8_gl.c's patterns */
#define HDR_CALIBRATION_PEAK 0
#define HDR_CALIBRATION_PAPER_WHITE 1

/* the pattern shown: the last row chosen's (the buttons keep it) */
static int hdr_calibration_pattern = HDR_CALIBRATION_PEAK;

static void hdr_calibration_row_show(struct widget_instance *list)
{
	struct widget_instance *row = named(list, "op_hdr_calibration", 0);
	struct widget_instance *child;
	short index;

	if (!row)
		return;
	row->visible = halo_hdr_calibration_available() != 0;
	/* (the focus on it, if the display's HDR went off, to the row before) */
	if (!row->visible && list->focused_child == row && row->previous)
	{
		for (index = 0, child = list->child; child && child != row->previous; child = child->next)
			index++;
		list->focused_child = row->previous;
		list->parameters.list.selected_index = index;
	}
}

/* the number a spinner of a setting shows; 0 if none */
static int setting_number_shown(struct widget_instance *list, char const *spinner_name)
{
	struct widget_instance *spinner = named(list, spinner_name, 0);
	struct pc_menu_setting *setting = spinner ? pc_menu_setting_get(spinner->definition_tag_index) : NULL;
	short index;

	if (!setting)
		return 0;
	index = spinner->parameters.list.selected_index;
	return index >= 0 && index < setting->value_count ? atoi(setting->values[index]) : 0;
}

/* "port hdr calibration" (its list's, as it is drawn) */
static void hdr_calibration_update(struct widget_instance *list)
{
	struct widget_instance *row = list->focused_child;

	if (row && !strcmp(row->name, "op_hdr_peak_brightness"))
		hdr_calibration_pattern = HDR_CALIBRATION_PEAK;
	else if (row && !strcmp(row->name, "op_hdr_paper_white"))
		hdr_calibration_pattern = HDR_CALIBRATION_PAPER_WHITE;
	halo_hdr_calibration_show(hdr_calibration_pattern, setting_number_shown(list, "hdr_paper_white_spinner"),
		setting_number_shown(list, "hdr_peak_brightness_spinner"));
}

/* ---------- Change Color: the profile's colour, from a list of the
game's colours (more than its rows: it scrolls), by the Xbox's names for
its spinner's functions */

#define COLOR_COUNT 18
#define COLOR_ROWS 11

static struct
{
	/* the colour on the first row, and the colour chosen */
	short first;
	short color;
} color_list;

static void color_row_text(short row, wchar_t *text)
{
	string_get("pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\color_edit\\colors_list",
		(short)(color_list.first + row), text);
}

/* "color picker menu initialize": on the profile's colour */
static boolean color_list_initialize(struct widget_instance *list)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();

	if (!profile)
		return FALSE;
	color_list.color = (short)PIN(profile->primary_color_index, 0, COLOR_COUNT - 1);
	color_list.first = (short)PIN(color_list.color - COLOR_ROWS / 2, 0, COLOR_COUNT - COLOR_ROWS);
	focus_row(list, (short)(color_list.color - color_list.first));
	return TRUE;
}

/* (ui_widget.c) whether the pointer moved the menus' focus last */
boolean ui_widget_port_pointer_focused(void);
boolean ui_widget_port_hover_focused(void);

/* Whether a list whose focus is on its first or last row moves on: when the
d-pad put it there. The pointer resting there would make it move every
frame; the wheel scrolls it instead. */
static boolean list_scrolls_at_end(void)
{
	return !ui_widget_port_pointer_focused();
}

/* a list of more items than its rows: the row chosen kept off its ends
while there are more past them, the list moving instead; the item chosen,
or NONE (its buttons) */
static short list_scroll(struct widget_instance *list, short *first, short count, short rows)
{
	short row = focused_row(list);

	if (row == rows - 1 && *first + rows < count && list_scrolls_at_end())
	{
		(*first)++;
		focus_row(list, --row);
	}
	else if (row == 0 && *first > 0 && list_scrolls_at_end())
	{
		(*first)--;
		focus_row(list, ++row);
	}
	return row == NONE || row >= rows ? NONE : (short)(*first + row);
}

/* "color picker update": the list scrolled on at its ends, the colour's
name and picture */
static void color_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *widget;
	short color = list_scroll(list, &color_list.first, COLOR_COUNT, COLOR_ROWS);

	if (color != NONE)
		color_list.color = color;
	rows_update(list, COLOR_ROWS, color_row_text);
	if ((widget = named(description, "color_right_name", 0)) != NULL)
		widget->parameters.text_box.string_list_index = color_list.color;
	if ((widget = named(description, "color_right_pic", 0)) != NULL)
		widget->animation.current_frame_index = color_list.color;
	profile_name_show(description);
}

/* "color picker select color" */
static boolean color_choose(void)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();

	if (!profile)
		return FALSE;
	profile->primary_color_index = color_list.color;
	return TRUE;
}

/* ---------- Edit Profile Settings: its picture of Gamepad Setup, the
profile's button settings as the Xbox's Controller Setup shows them */

/* the picture's frames (the bitmap's: tools/port_settings.py's
BITMAP_FRAMES): Gamepad Setup's row's, as the list numbers its rows
(player_profile_edit_select_menu_update_extended_description), and after
the PC version's nine the Xbox's five of the button settings */
#define PROFILE_GAMEPAD_FRAME 2
#define PROFILE_FIRST_LAYOUT_FRAME 9
#define PROFILE_LAYOUT_FRAMES (_button_preset_swap_b_and_right_thumb + 1)

/* "port gamepad layout preview" (the picture's own): on Gamepad Setup's
row, the edited profile's button settings; the port's own (Bumper Jumper),
of which the Xbox has no picture, keeps the row's */
static void profile_gamepad_layout(struct widget_instance *picture)
{
	struct player_profile *profile = player_ui_get_edit_player_profile();
	short frame = picture->animation.current_frame_index;

	if (frame == PROFILE_GAMEPAD_FRAME ||
		(frame >= PROFILE_FIRST_LAYOUT_FRAME && frame < PROFILE_FIRST_LAYOUT_FRAME + PROFILE_LAYOUT_FRAMES))
	{
		byte preset = profile ? profile->controller_settings.button_preset : (byte)_button_preset_standard;

		picture->animation.current_frame_index = preset < PROFILE_LAYOUT_FRAMES ?
			(short)(PROFILE_FIRST_LAYOUT_FRAME + preset) : PROFILE_GAMEPAD_FRAME;
	}
}

/* ---------- Profiles: the player profiles, and a row to make one (as the
PC version's list has them, but by the Xbox's names for its spinner's
functions); choosing one makes it player 1's, the profile the campaign and
Settings use; deleting one asks first */

#define PROFILE_ROWS 11
#define PROFILE_DEFAULT_BIT 0x40000000UL

static struct
{
	long indices[MAXIMUM_PROFILES];
	struct player_profile profiles[MAXIMUM_PROFILES];
	/* the profiles; the row after them makes a new one */
	short count;
	short first;
	short chosen;
	long to_delete;
} profile_list;

/* the profiles, read again when they are not those read last */
static void profile_list_read(boolean always)
{
	long indices[MAXIMUM_PROFILES];
	word count = MAXIMUM_PROFILES;
	short index, valid = 0;

	player_profiles_enumerate_available_to_local_player_index(NONE, &count, indices, FALSE);
	for (index = 0; index < (short)count; index++)
	{
		if ((unsigned long)indices[index] & PROFILE_VALID_BIT)
			indices[valid++] = indices[index];
	}
	if (!always && valid == profile_list.count &&
		!memcmp(indices, profile_list.indices, valid * sizeof(indices[0])))
	{
		return;
	}
	profile_list.count = 0;
	for (index = 0; index < valid; index++)
	{
		if (player_profile_get(indices[index], &profile_list.profiles[profile_list.count]))
			profile_list.indices[profile_list.count++] = indices[index];
	}
	profile_list.chosen = (short)PIN(profile_list.chosen, 0, profile_list.count);
}

static short profile_list_rows(void)
{
	return (short)MIN(profile_list.count + 1, PROFILE_ROWS);
}

/* "player profile list initialize": on player 1's profile */
static boolean profile_list_initialize(struct widget_instance *list)
{
	long active = player_ui_get_active_player_profile_index(0);
	short index;

	profile_list_read(TRUE);
	if (active == NONE)
		active = player_ui_get_player1_last_used_profile_index();
	profile_list.chosen = 0;
	for (index = 0; index < profile_list.count; index++)
	{
		if (profile_list.indices[index] == active)
			profile_list.chosen = index;
	}
	profile_list.first = (short)PIN(profile_list.chosen - PROFILE_ROWS / 2, 0,
		MAX(0, profile_list.count + 1 - PROFILE_ROWS));
	focus_row(list, (short)(profile_list.chosen - profile_list.first));
	return TRUE;
}

static void profile_row_text(short row, wchar_t *text)
{
	short item = (short)(profile_list.first + row);

	if (item < profile_list.count)
	{
		ustrncpy(text, profile_list.profiles[item].player_name, MAXIMUM_PLAYER_PROFILE_NAME_LENGTH);
		text[MAXIMUM_PLAYER_PROFILE_NAME_LENGTH] = 0;
	}
	else
	{
		string_get("pc\\main_menu\\player_profiles_select\\profile_description_labels", 4, text);
	}
}

static void visible_set(struct widget_instance *widget, boolean visible)
{
	if (widget)
		widget->visible = visible;
}

/* "3wide player profile list update": the rows, and the profile chosen:
its name, colour, current level, best difficulty and controls */
static void profile_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *widget;
	short item;
	boolean profile;

	profile_list_read(FALSE);
	item = list_scroll(list, &profile_list.first, (short)(profile_list.count + 1), profile_list_rows());
	if (item != NONE)
		profile_list.chosen = item;
	rows_update(list, profile_list_rows(), profile_row_text);
	profile = profile_list.chosen < profile_list.count;
	text_set(named(description, "player_profile_right_name", 0),
		profile ? profile_list.profiles[profile_list.chosen].player_name : L"");
	visible_set(named(description, "empty_profile_slot_text", 0), !profile);
	visible_set(named(description, "qtr_screen_profile_color_pic", 0), profile);
	visible_set(named(description, "current_level_label", 0), profile);
	visible_set(named(description, "current_level", 0), profile);
	visible_set(named(description, "joystick_controls_label", 0), profile);
	visible_set(named(description, "joystick_controls", 0), profile);
	visible_set(named(description, "skill_level_label", 0), FALSE);
	visible_set(named(description, "skill_level", 0), FALSE);
	if (profile)
	{
		struct player_profile *chosen = &profile_list.profiles[profile_list.chosen];
		short level, difficulty;

		if ((widget = named(description, "qtr_screen_profile_color_pic", 0)) != NULL)
			widget->animation.current_frame_index = (short)PIN(chosen->primary_color_index, 0, COLOR_COUNT - 1);
		if ((widget = named(description, "current_level", 0)) != NULL)
		{
			widget->parameters.text_box.string_list_index =
				(short)PIN(chosen->last_single_player_map_played, 0, NUMBER_OF_SINGLE_PLAYER_LEVELS - 1);
		}
		if ((widget = named(description, "joystick_controls", 0)) != NULL)
			widget->parameters.text_box.string_list_index = chosen->controller_settings.invert_look ? 1 : 0;
		player_profile_get_highest_completed_solo_level(chosen, &level, &difficulty);
		if (level != NONE && (widget = named(description, "skill_level", 0)) != NULL)
		{
			widget->parameters.text_box.string_list_index = difficulty;
			widget->visible = TRUE;
			visible_set(named(description, "skill_level_label", 0), TRUE);
		}
	}
	profile_name_show(description);
}

/* "profile manager select": the profile chosen made player 1's (and the
one used last); FALSE for the row that makes one (its handler then asks) */
static boolean profile_choose(short controller)
{
	struct player_profile profile;

	if (profile_list.chosen >= profile_list.count ||
		!player_profile_get(profile_list.indices[profile_list.chosen], &profile))
	{
		return FALSE;
	}
	player_ui_set_active_player_profile(0, profile_list.indices[profile_list.chosen], &profile);
	player_ui_set_single_player_local_player_controller(0, controller);
	player_ui_remember_player1_profile(TRUE);
	return TRUE;
}

/* "request del player profile" (before its question) */
static boolean profile_delete_request(void)
{
	if (profile_list.chosen >= profile_list.count)
		return campaign_fail();
	profile_list.to_delete = profile_list.indices[profile_list.chosen];
	return TRUE;
}

/* "final del player profile" (its OK): deleted, and if it was player 1's,
the first left is */
static boolean profile_delete(void)
{
	long index = profile_list.to_delete;
	boolean active = index == player_ui_get_active_player_profile_index(0);

	profile_list.to_delete = NONE;
	if (index == NONE || ((unsigned long)index & PROFILE_DEFAULT_BIT))
		return campaign_fail();
	player_profile_delete(index);
	profile_list_read(TRUE);
	if (active)
	{
		struct player_profile profile;

		if (profile_list.count && player_profile_get(profile_list.indices[0], &profile))
		{
			player_ui_set_active_player_profile(0, profile_list.indices[0], &profile);
			player_ui_remember_player1_profile(TRUE);
		}
		else
		{
			memset(&profile, 0, sizeof(profile));
			player_ui_set_active_player_profile(0, NONE, &profile);
		}
	}
	return TRUE;
}

/* ---------- Co-op: the campaign for two players on this machine, in split
screen (the Xbox's Cooperative Play, which the PC version has not):
Multiplayer's CO-OP CAMPAIGN ("port coop begin"), player 2's profile, chosen
with player 2's controller ("port coop player 2"), then New Game's levels
(those either has reached) and difficulty, with either player's controller
(ui_widget.c, widget_takes_events_of_controller). The main menu and
Multiplayer go back to one player (main_menu_initialize,
multiplayer_type_menu_initialize).
With one gamepad, it is player 2's (pc_menu_split_players) */

/* "port coop begin": two players, player 1 on its profile (campaign_profile)
and the controller that chose co-op */
static boolean coop_begin(short controller)
{
	struct player_profile profile;

	player_spawn_count = 1;
	player_ui_reset_single_player_local_player_controllers();
	if (!campaign_profile(controller, &profile))
		return campaign_fail();
	player_spawn_count = 2;
	return TRUE;
}

/* "port coop player 2 list initialize": on player 2's profile, else the first
that is not player 1's */
static boolean coop_player2_list_initialize(struct widget_instance *list)
{
	long player1 = player_ui_get_active_player_profile_index(0);
	long player2 = player_ui_get_active_player_profile_index(1);
	short index;

	profile_list_read(TRUE);
	profile_list.chosen = 0;
	for (index = profile_list.count - 1; index >= 0; index--)
	{
		if (profile_list.indices[index] != player1)
			profile_list.chosen = index;
	}
	for (index = 0; index < profile_list.count && player2 != NONE; index++)
	{
		if (profile_list.indices[index] == player2)
			profile_list.chosen = index;
	}
	profile_list.first = (short)PIN(profile_list.chosen - PROFILE_ROWS / 2, 0,
		MAX(0, profile_list.count + 1 - PROFILE_ROWS));
	focus_row(list, (short)(profile_list.chosen - profile_list.first));
	return TRUE;
}

/* "port coop player 2": the profile chosen made player 2's, on the controller
that chose it, which must not be player 1's */
static boolean coop_player2_choose(short controller)
{
	struct player_profile profile;

	if (player_spawn_count < 2 || profile_list.chosen >= profile_list.count ||
		!player_profile_get(profile_list.indices[profile_list.chosen], &profile))
	{
		return campaign_fail();
	}
	if (controller == player_ui_get_single_player_local_player_controller(0))
	{
		display_error_text_deferred(L"Player 2 chooses their\r\nprofile with their own\r\ncontroller.",
			NONE);
		return campaign_fail();
	}
	player_ui_set_single_player_local_player_controller(1, controller);
	player_ui_set_active_player_profile(1, profile_list.indices[profile_list.chosen], &profile);
	return TRUE;
}

/* ---------- Controls Setup: the keyboard and mouse's controls
(port/linux/include/halo_keyboard.h), shown a group at a time, two
bindings each */

#define CONTROL_BINDINGS 2
#define CONTROL_ROWS 7
#define CONTROL_NAME_LENGTH 32

static struct
{
	char const *setting;
	wchar_t const *label;
	short group;
} const controls[] =
{
	{ "controls.move_forward", L"MOVE FORWARD", 0 },
	{ "controls.move_backward", L"MOVE BACKWARD", 0 },
	{ "controls.strafe_left", L"STRAFE LEFT", 0 },
	{ "controls.strafe_right", L"STRAFE RIGHT", 0 },
	{ "controls.jump", L"JUMP", 0 },
	{ "controls.crouch", L"CROUCH", 0 },
	{ "controls.fire", L"FIRE", 1 },
	{ "controls.throw_grenade", L"THROW GRENADE", 1 },
	{ "controls.melee", L"MELEE", 1 },
	{ "controls.reload", L"RELOAD", 1 },
	{ "controls.zoom", L"ZOOM", 1 },
	{ "controls.switch_weapon", L"SWITCH WEAPON", 1 },
	{ "controls.switch_grenade", L"SWITCH GRENADE", 1 },
	{ "controls.action", L"ACTION", 2 },
	{ "controls.flashlight", L"FLASHLIGHT", 2 },
	{ "controls.scoreboard", L"SHOW SCORES", 2 },
	{ "controls.pause", L"PAUSE MENU", 2 },
	{ "controls.screenshot", L"SCREENSHOT", 2 },
	{ "controls.push_to_talk", L"PUSH TO TALK", 3 },
	{ "controls.chat", L"CHAT", 3 },
	{ "controls.team_chat", L"TEAM CHAT", 3 },
	{ "controls.vote_yes", L"VOTE YES", 3 },
	{ "controls.vote_no", L"VOTE NO", 3 },
};

static struct
{
	/* the bindings shown, until OK writes them */
	char bindings[NUMBEROF(controls)][CONTROL_BINDINGS][CONTROL_NAME_LENGTH];
	/* the binding left and right choose, and the one being set */
	short slot;
	short capturing_control;
	short capturing_slot;
} controls_screen = { { { { 0 } } }, 0, NONE, 0 };

/* a setting's bindings ("Left Ctrl, C") into the control's */
static void control_bindings_read(short control, char const *text)
{
	short slot;

	for (slot = 0; slot < CONTROL_BINDINGS; slot++)
	{
		char *binding = controls_screen.bindings[control][slot];
		unsigned int length;

		while (*text == ' ' || *text == ',')
			text++;
		length = (unsigned int)strcspn(text, ",");
		if (length >= CONTROL_NAME_LENGTH)
			length = CONTROL_NAME_LENGTH - 1;
		memcpy(binding, text, length);
		binding[length] = 0;
		while (length && binding[length - 1] == ' ')
			binding[--length] = 0;
		text += strcspn(text, ",");
	}
}

static void control_bindings_text(short control, char *text, unsigned int size)
{
	char const *first = controls_screen.bindings[control][0];
	char const *second = controls_screen.bindings[control][1];

	snprintf(text, size, "%s%s%s", *first ? first : second, *first && *second ? ", " : "", *first ? second : "");
}

/* "controls screen init" (and "controls screen defaults": the defaults) */
static boolean controls_load(boolean defaults)
{
	short control;

	for (control = 0; control < NUMBEROF(controls); control++)
	{
		char text[128];

		if (defaults)
			config_default(controls[control].setting, text, sizeof(text));
		else
			snprintf(text, sizeof(text), "%s", config_string(controls[control].setting));
		control_bindings_read(control, text);
	}
	controls_screen.capturing_control = NONE;
	return TRUE;
}

/* "controls screen change set" (OK) */
static boolean controls_save(void)
{
	short control;
	boolean result = TRUE;

	for (control = 0; control < NUMBEROF(controls); control++)
	{
		char text[128];

		control_bindings_text(control, text, sizeof(text));
		if (strcmp(text, config_string(controls[control].setting)))
		{
			if (config_write(controls[control].setting, text))
				platform_log("menus: %s = %s", controls[control].setting, text);
			else
				result = FALSE;
		}
	}
	return result;
}

static struct widget_instance *control_row(struct widget_instance *widget)
{
	for (; widget; widget = widget->parent)
	{
		if (!strncmp(widget->name, "op_command_", 11))
			return widget;
	}
	return NULL;
}

/* the control on the row (op_command_<n>) in the group shown, else NONE */
static short control_of_row(struct widget_instance *row, short group)
{
	short wanted = row ? (short)(atoi(row->name + 11) - 1) : NONE;
	short control;

	for (control = 0; control < NUMBEROF(controls) && wanted >= 0; control++)
	{
		if (controls[control].group == group && wanted-- == 0)
			return control;
	}
	return NONE;
}

static short controls_group(struct widget_instance *screen)
{
	struct widget_instance *spinner = named(screen, "group_spinner", 0);

	return spinner ? spinner->parameters.list.selected_index : 0;
}

/* "controls begin binding" (A on a row): the next key or button pressed */
static boolean control_capture_begin(struct widget_instance *widget)
{
	short control = control_of_row(control_row(widget), controls_group(screen_of(widget)));

	if (control == NONE)
		return FALSE;
	controls_screen.capturing_control = control;
	controls_screen.capturing_slot = controls_screen.slot;
	platform_binding_capture_begin();
	return TRUE;
}

/* the key or button taken (then bound to no other control), or cleared */
static void control_capture_poll(void)
{
	short control = controls_screen.capturing_control;
	short slot = controls_screen.capturing_slot;
	int input = -1;
	int result;

	if (control == NONE || !(result = platform_binding_capture_poll(&input)))
		return;
	controls_screen.capturing_control = NONE;
	if (result == 1)
	{
		char name[CONTROL_NAME_LENGTH];
		short other, other_slot;

		halo_input_name(input, name, sizeof(name));
		if (!*name)
			return;
		for (other = 0; other < NUMBEROF(controls); other++)
		{
			for (other_slot = 0; other_slot < CONTROL_BINDINGS; other_slot++)
			{
				if (!_stricmp(controls_screen.bindings[other][other_slot], name))
					controls_screen.bindings[other][other_slot][0] = 0;
			}
		}
		snprintf(controls_screen.bindings[control][slot], CONTROL_NAME_LENGTH, "%s", name);
	}
	else if (result == 2)
	{
		controls_screen.bindings[control][slot][0] = 0;
	}
}

/* "controls back handler" (B, Back): the key being set is left as it was,
else the screen is left */
static boolean controls_back(struct widget_instance *widget, boolean *widget_deleted)
{
	if (controls_screen.capturing_control != NONE)
	{
		controls_screen.capturing_control = NONE;
		platform_binding_capture_cancel();
		return TRUE;
	}
	ui_widget_port_go_back(widget);
	*widget_deleted = TRUE;
	return TRUE;
}

/* "controls update menu": the group's rows, their bindings (the one left
and right choose marked on the row chosen), and the help */
static void controls_update(struct widget_instance *list)
{
	short group = controls_group(list);
	struct widget_instance *row;
	struct widget_instance *focused_row = control_row(list->focused_child);

	control_capture_poll();
	for (row = list->child; row; row = row->next)
	{
		short control = control_of_row(row, group);
		short slot;

		if (strncmp(row->name, "op_command_", 11))
			continue;
		row->visible = control != NONE;
		if (control == NONE)
		{
			if (row == focused_row)
				focus_row(list, 0);
			continue;
		}
		text_set(named(row, "command_label", 0), controls[control].label);
		for (slot = 0; slot < CONTROL_BINDINGS; slot++)
		{
			char const *binding = controls_screen.bindings[control][slot];
			wchar_t text[ROW_TEXT_LENGTH];
			char name[CONTROL_NAME_LENGTH];
			char shown[64];
			short index;

			/* (the key as the keyboard's layout labels it: xinput_sdl.c) */
			if (*binding)
				halo_input_shown_name(binding, name, sizeof(name));
			else
				snprintf(name, sizeof(name), "%s", "-");
			if (controls_screen.capturing_control == control && controls_screen.capturing_slot == slot)
				snprintf(shown, sizeof(shown), "%s", "PRESS A KEY");
			else if (row == focused_row && controls_screen.slot == slot)
				snprintf(shown, sizeof(shown), "> %s <", name);
			else
				snprintf(shown, sizeof(shown), "%s", name);
			for (index = 0; shown[index] && index < ROW_TEXT_LENGTH - 1; index++)
				text[index] = (wchar_t)(unsigned char)shown[index];
			text[index] = 0;
			text_set(named(row, "command_binding", slot), text);
		}
	}
	if (list->parameters.list.extended_description)
	{
		list->parameters.list.extended_description->parameters.text_box.string_list_index =
			controls_screen.capturing_control != NONE ? 2 : focused_row ? 1 : 0;
	}
}

#include "controller_remap_menu.inl"

/* ---------- Mods (the main menu's: tools/port_settings.py's _mods), laid
out as the lobby is: the mods folder's mods, a row each (op_mod_<n>), as a
resource pack list is, the one that plays over the others at the top (the
load order upside down: the mod loaded last first), turned on or off and
moved up or down there, and in the frame beside them the mod chosen's
details; OK keeps it (port/linux/src/port_mods.c's load_order.toml) and
plays them on the map now (mods.c), Cancel forgets it */

#define MOD_ROWS 11
#define MOD_DETAILS_LENGTH 1024
/* the details' lines: as many as fit down the frame */
#define MOD_DETAILS_LINES 13
/* (from the shadow of each box's last character) */
#define MOD_TEXT_MARGIN 8

/* the text boxes whose lines are measured: the details' mod name and the
rest, and the rows' names */
enum
{
	_mod_details_name,
	_mod_details_text,
	_mod_row_name,
	NUMBER_OF_MOD_TEXT_BOXES
};

/* the text boxes' names, and as many characters as fit across each until
it has been drawn, and its font is known */
static char const *const mod_text_box_names[NUMBER_OF_MOD_TEXT_BOXES] = { "mod_details_name", "mod_details_text",
	"mod_name" };
static short const mod_text_box_characters[NUMBER_OF_MOD_TEXT_BOXES] = { 17, 30, 24 };

/* (port/linux/src/port_mods.c, port_mods.h's) */
enum
{
	_port_mod_fine,
	_port_mod_out_of_order,
	_port_mod_incompatible,
	_port_mod_unplayable,
};

long port_mods_count(void);
long port_mods_at(long position);
char const *port_mods_name(long index);
int port_mods_on(long index);
int port_mods_status(long index, char *text, unsigned long size);
int port_mods_details(long index, char *text, unsigned long size);
long port_mods_move(long position, long places);
void port_mods_turn(long index, int on);
int port_mods_save(void);
void port_mods_revert(void);
/* (mods.c) */
void mods_apply(void);

static struct
{
	/* the place on the list (0 the top: the mod loaded last) of the first
	row, and of the mod chosen last (MOVE UP and MOVE DOWN's, which take the
	focus from the rows) */
	short first;
	short chosen;
	/* the text boxes' fonts and widths as they were last drawn
	(menu_functions_text_box_drawn), which their lines are measured in;
	NONE before then */
	long fonts[NUMBER_OF_MOD_TEXT_BOXES];
	short widths[NUMBER_OF_MOD_TEXT_BOXES];
	/* what the details show (they are wrapped again when it changes), and
	the mod each row's name is of, in which font (NONE: none yet) */
	char shown[128];
	long row_mods[MOD_ROWS];
	long row_fonts[MOD_ROWS];
} mods_screen;

/* the row (op_mod_<n>) a widget is or is in, else NULL */
static struct widget_instance *mod_row(struct widget_instance *widget)
{
	for (; widget; widget = widget->parent)
	{
		if (!strncmp(widget->name, "op_mod_", 7))
			return widget;
	}
	return NULL;
}

/* the place on the list of the mod on a row, else NONE */
static short mod_row_place(struct widget_instance *row)
{
	short place = row ? (short)(mods_screen.first + atoi(row->name + 7) - 1) : NONE;

	return place >= 0 && place < port_mods_count() ? place : NONE;
}

/* the mod at a place on the list (its place in the load order counted from
the end), else NONE */
static long mod_at_place(short place)
{
	long count = port_mods_count();

	return place >= 0 && place < count ? port_mods_at(count - 1 - place) : NONE;
}

/* UTF-8 text (mod.toml's) as the menus' (a line break the game's \r\n; a
character the menus' UTF-16 cannot hold, or a broken one, a ?) */
static void mods_wide_text(char const *text, wchar_t *wide, short length)
{
	short index = 0;

	while (*text && index < length - 2)
	{
		unsigned char first = (unsigned char)*text++;
		unsigned long character = first;
		short following = first >= 0xF0 ? 3 : first >= 0xE0 ? 2 : first >= 0xC0 ? 1 : 0;

		if (first >= 0x80)
		{
			character = first & (0x3F >> following);
			for (; following > 0 && ((unsigned char)*text & 0xC0) == 0x80; following--)
				character = (character << 6) | ((unsigned char)*text++ & 0x3F);
			if (following || first < 0xC0 || character > 0xFFFF)
				character = '?';
		}
		if (character == '\n')
			wide[index++] = '\r';
		wide[index++] = (wchar_t)character;
	}
	wide[index] = 0;
}

/* the start of the UTF-8 character before `at` in text */
static size_t mods_character_before(char const *text, size_t at)
{
	if (at > 0)
		at--;
	while (at > 0 && ((unsigned char)text[at] & 0xC0) == 0x80)
		at--;
	return at;
}

/* whether `count` bytes of text fit across a text box (box): measured in
its font, from its left as it is drawn (leading spaces and all), else
counted */
static boolean mods_text_fits(short box, char const *text, size_t count)
{
	static real_argb_color const white = { 1.0f, 1.0f, 1.0f, 1.0f };
	char part[MOD_DETAILS_LENGTH];
	wchar_t wide[MOD_DETAILS_LENGTH];
	rectangle2d bounds;
	rectangle2d text_bounds;
	rectangle2d cursor;

	if (!count)
		return TRUE;
	if (mods_screen.fonts[box] == NONE)
		return count <= (size_t)mod_text_box_characters[box];
	if (count >= sizeof(part))
		return FALSE;
	memcpy(part, text, count);
	part[count] = 0;
	mods_wide_text(part, wide, NUMBEROF(wide));
	bounds.x0 = 0;
	bounds.y0 = 0;
	bounds.x1 = 0x3FFF;
	bounds.y1 = 480;
	draw_string_set_draw_mode(mods_screen.fonts[box], NONE, 0, 0, &white);
	draw_unicode_string_compute_bounds(&bounds, wide, &text_bounds, &cursor);
	return MAX(text_bounds.x1, cursor.x0) - bounds.x0 <= mods_screen.widths[box] - MOD_TEXT_MARGIN;
}

/* `count` bytes of text, as much of them as fits across a text box (box)
with "..." after it, into `shortened` */
static void mods_text_shorten(short box, char const *text, size_t count, char *shortened, size_t size)
{
	size_t kept = count;

	for (;;)
	{
		snprintf(shortened, size, "%.*s...", (int)kept, text);
		if (!kept || mods_text_fits(box, shortened, strlen(shortened)))
			return;
		kept = mods_character_before(text, kept);
	}
}

/* text, after what `wrapped` holds, in lines that fit across a text box
(box), broken at spaces and at its own line breaks (word by word: a word
too long for a line broken where it fits); `lines` lines in all at most,
the last cut short with "..." when there is more: how many it holds now */
static short mods_wrap(char const *text, char *wrapped, size_t size, short box, short lines, short line)
{
	size_t length = strlen(wrapped);

	while (line < lines && *text)
	{
		char const *end = strchr(text, '\n');
		size_t count = end ? (size_t)(end - text) : strlen(text);
		size_t cut = 0;
		boolean more;

		for (;;)
		{
			size_t next = cut;

			while (next < count && text[next] == ' ')
				next++;
			while (next < count && text[next] != ' ')
				next++;
			if (next == cut || !mods_text_fits(box, text, next))
				break;
			cut = next;
		}
		if (!cut && count)
		{
			for (cut = 0; cut < count && text[cut] == ' '; cut++)
			{
			}
			while (cut < count && text[cut] != ' ')
				cut++;
			while (cut > 1 && !mods_text_fits(box, text, cut))
				cut = mods_character_before(text, cut);
			if (!cut)
				cut = 1;
		}
		more = cut < count || (text[count] == '\n' && text[count + 1]);
		if (line == lines - 1 && more)
		{
			char shortened[MOD_DETAILS_LENGTH];

			mods_text_shorten(box, text, cut, shortened, sizeof(shortened));
			snprintf(wrapped + length, size - length, "%s%s", length ? "\n" : "", shortened);
			return lines;
		}
		snprintf(wrapped + length, size - length, "%s%.*s", length ? "\n" : "", (int)cut, text);
		length = strlen(wrapped);
		line++;
		text += cut;
		while (*text == ' ')
			text++;
		if (*text == '\n')
			text++;
	}
	return line;
}

/* "port mods init" (the list's created), and "port mods close" (its
deleted: what OK has not kept is forgotten) */
static boolean mods_screen_reset(void)
{
	short index;

	port_mods_revert();
	mods_screen.first = 0;
	mods_screen.chosen = 0;
	mods_screen.shown[0] = 0;
	for (index = 0; index < NUMBER_OF_MOD_TEXT_BOXES; index++)
	{
		mods_screen.fonts[index] = NONE;
		mods_screen.widths[index] = 0;
	}
	for (index = 0; index < MOD_ROWS; index++)
		mods_screen.row_mods[index] = NONE;
	return TRUE;
}

/* "port mods toggle" (A on a row; a mod that cannot be played stays as it
is, so that it is still off when it can be) */
static boolean mods_toggle(struct widget_instance *widget)
{
	short place = mod_row_place(mod_row(widget));
	long mod = mod_at_place(place);

	if (place == NONE)
		return FALSE;
	if (port_mods_status(mod, NULL, 0) != _port_mod_unplayable)
		port_mods_turn(mod, !port_mods_on(mod));
	return TRUE;
}

/* "port mods move up" and "port mods move down" (left and right on a row,
and the buttons): the mod of the row, else the one chosen last, moved one
place on the list (up: later in the load order, over one more mod); the
focus follows it on its row */
static boolean mods_move(struct widget_instance *widget, short places)
{
	struct widget_instance *row = mod_row(widget);
	short count = (short)port_mods_count();
	short place = row ? mod_row_place(row) : mods_screen.chosen;
	short moved;

	/* (no mods: nothing to move) */
	if (!count)
		return TRUE;
	if (place == NONE || place >= count)
		return FALSE;
	moved = (short)(count - 1 - port_mods_move(count - 1 - place, -places));
	mods_screen.chosen = moved;
	if (moved < mods_screen.first)
		mods_screen.first = moved;
	else if (moved >= mods_screen.first + MOD_ROWS)
		mods_screen.first = (short)(moved - MOD_ROWS + 1);
	if (row && row->parent)
		focus_row(row->parent, (short)(moved - mods_screen.first));
	return TRUE;
}

/* the details of the mod at `place` on the list, in the frame (the list's
description): its name; what is wrong with it, beside its icon; whether it
is on and its priority (1 the top, the mod that wins); and what its
mod.toml says */
static void mods_details_show(struct widget_instance *details, short place, short count)
{
	long mod = mod_at_place(place);
	struct widget_instance *icon = named(details, "mod_details_status", 0);
	char problem[MOD_DETAILS_LENGTH];
	char about[MOD_DETAILS_LENGTH];
	char text[MOD_DETAILS_LENGTH];
	wchar_t wide[MOD_DETAILS_LENGTH];
	int status;
	short line;
	char shown[sizeof(mods_screen.shown)];

	/* (wrapped again only when what they show, or its fonts, change) */
	snprintf(shown, sizeof(shown), "%ld %d %d %d %d %ld %d %ld %d", mod, port_mods_status(mod, NULL, 0),
		port_mods_on(mod), place, count, mods_screen.fonts[_mod_details_name],
		mods_screen.widths[_mod_details_name], mods_screen.fonts[_mod_details_text],
		mods_screen.widths[_mod_details_text]);
	if (!strcmp(shown, mods_screen.shown))
		return;
	strcpy(mods_screen.shown, shown);
	if (mod < 0)
	{
		/* (no mods: where they go) */
		mods_wide_text(count ? "" : "No mods", wide, NUMBEROF(wide));
		text_set_length(named(details, "mod_details_name", 0), wide, NUMBEROF(wide));
		text[0] = 0;
		if (!count)
		{
			mods_wrap("Each mod is a folder of its own in the mods folder, beside the maps folder.", text,
				sizeof(text), _mod_details_text, MOD_DETAILS_LINES, 0);
		}
		mods_wide_text(text, wide, NUMBEROF(wide));
		text_set_length(named(details, "mod_details_text", 0), wide, NUMBEROF(wide));
		visible_set(icon, FALSE);
		return;
	}
	status = port_mods_status(mod, problem, sizeof(problem));
	text[0] = 0;
	mods_wrap(port_mods_name(mod), text, sizeof(text), _mod_details_name, 2, 0);
	mods_wide_text(text, wide, NUMBEROF(wide));
	text_set_length(named(details, "mod_details_name", 0), wide, NUMBEROF(wide));
	visible_set(icon, status != _port_mod_fine);
	if (icon)
		icon->animation.current_frame_index = status == _port_mod_out_of_order ? 1 : 2;
	text[0] = 0;
	line = 0;
	if (status != _port_mod_fine)
	{
		/* (its first line indented, for the icon) */
		snprintf(text, sizeof(text), "     %s", problem);
		strcpy(problem, text);
		text[0] = 0;
		line = mods_wrap(problem, text, sizeof(text), _mod_details_text, MOD_DETAILS_LINES, line);
		line = mods_wrap(" ", text, sizeof(text), _mod_details_text, MOD_DETAILS_LINES, line);
	}
	snprintf(about, sizeof(about), "%s\nPriority: %d of %d", status == _port_mod_unplayable ? "Cannot be played" :
		port_mods_on(mod) ? "On" : "Off", place + 1, count);
	line = mods_wrap(about, text, sizeof(text), _mod_details_text, MOD_DETAILS_LINES, line);
	line = mods_wrap(" ", text, sizeof(text), _mod_details_text, MOD_DETAILS_LINES, line);
	port_mods_details(mod, about, sizeof(about));
	mods_wrap(about, text, sizeof(text), _mod_details_text, MOD_DETAILS_LINES, line);
	mods_wide_text(text, wide, NUMBEROF(wide));
	text_set_length(named(details, "mod_details_text", 0), wide, NUMBEROF(wide));
}

/* "port mods update": the rows (the list moving on at its ends while there
are more mods past them), each mod's name (cut short where it is too long),
ON or OFF and its icon (out of the order it asks for; another says it cannot
be played with it, or it cannot be played), and the details of the mod
chosen */
static void mods_update(struct widget_instance *list)
{
	struct widget_instance *row;
	struct widget_instance *focused = mod_row(list->focused_child);
	short count = (short)port_mods_count();
	short focused_index = focused ? (short)(atoi(focused->name + 7) - 1) : NONE;

	if (focused_index == MOD_ROWS - 1 && mods_screen.first + MOD_ROWS < count && list_scrolls_at_end())
	{
		mods_screen.first++;
		focus_row(list, --focused_index);
	}
	else if (focused_index == 0 && mods_screen.first > 0 && list_scrolls_at_end())
	{
		mods_screen.first--;
		focus_row(list, ++focused_index);
	}
	if (mods_screen.first > 0 && mods_screen.first + MOD_ROWS > count)
		mods_screen.first = (short)MAX(0, count - MOD_ROWS);
	if (focused_index != NONE && mods_screen.first + focused_index < count)
		mods_screen.chosen = (short)(mods_screen.first + focused_index);
	for (row = list->child; row; row = row->next)
	{
		short place;
		short row_index;
		long mod;
		struct widget_instance *icon;
		int status;

		if (strncmp(row->name, "op_mod_", 7))
			continue;
		/* (its background: the focused one's outlined frame, which the
		engine shows only on a bitmap of two frames, as the browser's) */
		row->animation.current_frame_index = row == focused ? 1 : 0;
		place = mod_row_place(row);
		row->visible = place != NONE;
		if (place == NONE)
		{
			if (row == focused)
				focus_row(list, count ? (short)(count - 1 - mods_screen.first) : NONE);
			continue;
		}
		mod = mod_at_place(place);
		status = port_mods_status(mod, NULL, 0);
		row_index = (short)PIN(atoi(row->name + 7) - 1, 0, MOD_ROWS - 1);
		if (mods_screen.row_mods[row_index] != mod ||
			mods_screen.row_fonts[row_index] != mods_screen.fonts[_mod_row_name])
		{
			char const *name = port_mods_name(mod);
			char shortened[MOD_DETAILS_LENGTH];
			wchar_t wide[ROW_TEXT_LENGTH];

			if (mods_text_fits(_mod_row_name, name, strlen(name)))
				snprintf(shortened, sizeof(shortened), "%s", name);
			else
				mods_text_shorten(_mod_row_name, name, strlen(name), shortened, sizeof(shortened));
			mods_wide_text(shortened, wide, ROW_TEXT_LENGTH);
			text_set(named(row, "mod_name", 0), wide);
			mods_screen.row_mods[row_index] = mod;
			mods_screen.row_fonts[row_index] = mods_screen.fonts[_mod_row_name];
		}
		text_set(named(row, "mod_state", 0), port_mods_on(mod) ? L"ON" : L"OFF");
		icon = named(row, "mod_status", 0);
		visible_set(icon, status != _port_mod_fine);
		if (icon)
			icon->animation.current_frame_index = status == _port_mod_out_of_order ? 1 : 2;
	}
	mods_details_show(list->parameters.list.extended_description, count ? mods_screen.chosen : NONE, count);
}

/* "port mods save" (OK): kept, and played at once (port_mods_save keeps
what it cannot write until the game quits) */
static boolean mods_save(void)
{
	port_mods_save();
	mods_apply();
	return TRUE;
}

/* ---------- Multiplayer (port/assets/menus' PLAN: Create Game > Internet
or LAN; Join Game > Server Browser (blank, for now), LAN, Direct Link): the
Xbox's networking, run by the engine's port entry points
(ui_widget_event_handler_functions.c) on our lists */

#define MAXIMUM_ADVERTISED_GAMES 9
#define GAMETYPE_ROWS 10
#define MAXIMUM_GAMETYPES 100
#define BROWSER_ROWS 15
/* the server browser's source tab's words and arrows while another item
has the focus (lobby_browser_source) */
#define TAB_UNFOCUSED_ALPHA 0.45f
#define LOBBY_ROWS 11
#define TEXT_FIELD_LENGTH 128
#define PLAYLIST_READ_ONLY_BIT 0x40000000UL
/* (a key stroke's modifier, as input_xbox.c has them: shift, control) */
#define KEY_MODIFIER_CONTROL_BIT 1
#define LOBBY_NAME "pc\\main_menu\\multiplayer_type_select\\lobby\\lobby_screen"
#define PREVIEW_NAME "pc\\main_menu\\multiplayer_type_select\\lobby\\preview_screen"
/* the server browser's password screen (tools/port_settings.py), and the
longest password */
#define PASSWORD_NAME "pc\\main_menu\\multiplayer_type_select\\join_game\\password\\password_screen"
#define PASSWORD_LENGTH 32
/* the lobby's panel's lines (the gametype, the players, the countdown, the
invite) */
#define LOBBY_TEXT_LENGTH (ROW_TEXT_LENGTH * 4)

enum
{
	_multiplayer_mode_server_browser,
	_multiplayer_mode_lan,
	_multiplayer_mode_direct_link,
	_multiplayer_mode_host_internet,
	_multiplayer_mode_host_lan,
};

enum
{
	_client_state_searching,
	_client_state_joining,
	_client_state_pregame,
	_client_state_ingame,
	_client_state_postgame,
};

/* a game the client found, as network_client_manager.c has it */
struct advertised_game
{
	byte key_id[8];
	byte key[16];
	/* XNADDR: size, flags, abEnet (the port's machine identifier), ina */
	byte xnaddr[12];
	byte nonce[8];
	unsigned long update_time;
	wchar_t game_name[16];
	long map_version;
	char map_name[0x80];
	short engine_type;
	word machine_count;
	word player_count;
	short maximum_player_count;
	short unknown100;
	short platform;
	boolean open;
	boolean valid;
	boolean has_teams;
	boolean oddball_variant;
};

typedef char verify_advertised_game_platform_offset[offsetof(struct advertised_game, platform) == 0xDE ? 1 : -1];
typedef char verify_advertised_game_open_offset[offsetof(struct advertised_game, open) == 0xE0 ? 1 : -1];

/* the engine's (port) */
short ui_widget_port_multiplayer_maps(char const *const **names, short *last_used);
boolean ui_widget_port_multiplayer_map_choose(short level_index);
boolean ui_widget_port_cooperative_level_choose(char const *map_name, short difficulty);
short ui_widget_port_gametypes(long *indices, short maximum, short *last_used);
boolean ui_widget_port_gametype_choose(long profile_index);
boolean ui_widget_port_host(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
boolean ui_widget_port_browse(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted);
boolean ui_widget_port_open(struct widget_instance *widget, char const *name, boolean *widget_deleted);
boolean network_game_client_advertised_game_in_progress(void *client, struct advertised_game *game);
byte network_game_client_advertised_game_kind(void *client, struct advertised_game *game);
boolean network_game_client_advertised_game_dedicated(void *client, struct advertised_game *game);
void network_game_client_tell_version(unsigned int theirs, boolean distributed);
boolean ui_widget_port_join(struct widget_instance *widget, void *advertised_game, char const *lobby_name,
	boolean *widget_deleted);
boolean ui_widget_port_multiplayer_player(short controller_index, long profile_index);
boolean ui_widget_port_unjoin_player(struct widget_instance *widget, struct event_record *event,
	boolean *widget_deleted);
void network_game_server_port_set_settings(wchar_t const *name, long maximum_players);
void network_game_server_port_set_cooperative_friendly_fire(short friendly_fire);
void network_game_server_port_set_cooperative_player_collisions(boolean player_collisions);
void network_game_server_port_set_cooperative_ally_friendly_fire(boolean ally_friendly_fire);
void *global_network_game_client_get(void);
void *global_network_game_server_get(void);
boolean network_game_is_splitscreen_local(void);
struct advertised_game *network_game_client_get_available_games(void *client);
boolean network_game_client_advertised_game_is_valid(struct advertised_game *game);
short network_game_client_get_state(void *client, short *state_data);
struct network_game *network_game_client_get_game(void *client);
struct network_game *network_game_server_get_game(void *server);
short network_game_client_get_local_machine_index(void);
short network_game_client_get_seconds_to_game_start(void *client);
boolean network_player_is_valid(struct network_player *player);
boolean playlist_profile_get(long index, struct game_variant *variant);
boolean playlist_profile_get_display_name(long index, wchar_t *name);
boolean playlist_profile_get_options(long index, struct game_variant_options *options);
boolean input_get_key(struct key_stroke *key);
boolean game_engine_running(void);
void game_engine_end_game(void);
/* the platform layer's */
int p2p_join_invite(char const *text);
int p2p_join_listing(struct p2p_listing const *listing);
int p2p_invite_link(char *link, int size);
void p2p_set_hosting_allowed(int allowed);
int p2p_peer_address(unsigned char const *identifier, unsigned long *address);
int platform_clipboard_get(char *text, int size);
void platform_clipboard_set(char const *text);
void platform_text_field(int typing, int password);
int config_boolean(char const *name);
void ui_widget_port_post_button(short controller_index, short button_index);

static wchar_t const *const engine_names[] = { L"", L"CTF", L"SLAYER", L"ODDBALL", L"KING OF THE HILL", L"RACE" };
static short const maximum_players[] = { 2, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128 };

static struct
{
	short mode;
	/* Create Game's gametype list */
	long gametypes[MAXIMUM_GAMETYPES];
	short gametype_count;
	/* (those of the bank the list's spinner shows: STANDARD, OPENCE, CUSTOM) */
	short bank[MAXIMUM_GAMETYPES], bank_count, bank_shown;
	short gametype_first, gametype_chosen;
	/* the server settings */
	wchar_t game_name[16];
	short maximum_players_index;
	/* (co-op's own, so it leaves multiplayer's as it was: each game hosted
	starts with COOPERATIVE_DEFAULT_PLAYERS players at most) */
	short cooperative_maximum_players_index;
	boolean cooperative_maximum_players_set;
	/* the co-op game's FRIENDLY FIRE, and its PLAYER COLLISIONS OFF, as it
	starts (CO-OP OPTIONS': network.coop_friendly_fire's and
	network.coop_player_collisions': server_start) */
	short cooperative_friendly_fire;
	boolean cooperative_no_player_collisions;
	/* the browser's games */
	struct advertised_game *games[MAXIMUM_ADVERTISED_GAMES];
	short game_count, game_chosen;
	/* the lobby's */
	short lobby_first;
	/* the game under way whose lobby is shown before joining it: its key
	(the client's slot it is in may come to hold another game) */
	byte preview_key_id[8];
	byte preview_xnaddr[12];
	/* Server Setup's LISTING: PRIVATE (each new game starts with
	network.host_public's choice: PUBLIC unless set otherwise) */
	boolean game_private;
	/* Server Setup's PASSWORD (a public internet game's; empty: none), kept
	while the game runs, never written down */
	char game_password[PASSWORD_LENGTH + 1];
} multiplayer = { .maximum_players_index = NUMBEROF(maximum_players) - 1 };

/* ---- a text field (Direct Link's link, the game's name): the keyboard
types into it (Ctrl+V pastes), its row's A (enter) is done, B (escape)
cancels */
static struct
{
	struct widget_instance *row;
	char text[TEXT_FIELD_LENGTH];
	char before[TEXT_FIELD_LENGTH];
	short maximum;
	void (*done)(char const *text);
	/* a password's: shown as stars (text_field_begin_masked) */
	boolean masked;
} text_field;

/* when the field was last shown (a field not shown for a while is let go
of: its screen has gone) */
static unsigned long text_field_shown_time;

static boolean text_field_editing(struct widget_instance *row)
{
	return text_field.row && (!row || text_field.row == row);
}

static void text_field_open(struct widget_instance *row, char const *text, short maximum,
	void (*done)(char const *text), boolean masked)
{
	struct key_stroke key;

	text_field.row = row;
	snprintf(text_field.text, sizeof(text_field.text), "%s", text);
	snprintf(text_field.before, sizeof(text_field.before), "%s", text);
	text_field.maximum = (short)MIN(maximum, TEXT_FIELD_LENGTH - 1);
	text_field.done = done;
	text_field.masked = masked;
	text_field_shown_time = system_milliseconds();
	while (input_get_key(&key))
		;
	platform_text_field(TRUE, masked);
}

static void text_field_begin(struct widget_instance *row, char const *text, short maximum,
	void (*done)(char const *text))
{
	text_field_open(row, text, maximum, done, FALSE);
}

/* a password's field: as text_field_begin, its text shown as stars */
static void text_field_begin_masked(struct widget_instance *row, char const *text, short maximum,
	void (*done)(char const *text))
{
	text_field_open(row, text, maximum, done, TRUE);
}

static void text_field_end(boolean keep)
{
	void (*done)(char const *text) = text_field.done;

	platform_text_field(FALSE, FALSE);
	text_field.row = NULL;
	text_field.done = NULL;
	if (!keep)
		snprintf(text_field.text, sizeof(text_field.text), "%s", text_field.before);
	else if (done)
		done(text_field.text);
}

static void text_field_insert(char const *text)
{
	size_t length = strlen(text_field.text);

	for (; *text && length < (size_t)text_field.maximum; text++)
	{
		if (*text >= ' ' && *text <= '~')
			text_field.text[length++] = *text;
	}
	text_field.text[length] = 0;
}

/* the keys typed since the last frame, and the field shown (a caret
blinking while it is edited) */

static void text_field_show(struct widget_instance *value, char const *text, boolean editing)
{
	wchar_t shown[TEXT_FIELD_LENGTH + 2];
	short index;

	if (editing)
	{
		struct key_stroke key;

		text_field_shown_time = system_milliseconds();
		while (input_get_key(&key))
		{
			size_t length = strlen(text_field.text);

			if (key.key_code == _key_backspace)
			{
				if (length)
					text_field.text[length - 1] = 0;
			}
			else if (TEST_FLAG(key.modifier_flags, KEY_MODIFIER_CONTROL_BIT) && key.key_code == _key_v)
			{
				char clipboard[TEXT_FIELD_LENGTH];

				if (platform_clipboard_get(clipboard, sizeof(clipboard)))
					text_field_insert(clipboard);
			}
			else if ((unsigned char)key.ascii_code >= ' ' && (unsigned char)key.ascii_code <= '~')
			{
				char character[2] = { key.ascii_code, 0 };

				text_field_insert(character);
			}
		}
		text = text_field.text;
	}
	for (index = 0; text[index] && index < TEXT_FIELD_LENGTH; index++)
		shown[index] = editing && text_field.masked ? L'*' : (wchar_t)(unsigned char)text[index];
	if (editing && (system_milliseconds() / 500) % 2)
		shown[index++] = L'_';
	shown[index] = 0;
	text_set(value, shown);
	/* (what is being typed shown as it is: streamer mode hides nothing of it,
	text_filter.c) */
	if (editing && value)
		text_filter_exempt(value->parameters.text_box.text);
	else if (!text_field_editing(NULL))
		text_filter_exempt(NULL);
}

/* ---- the menu */

/* "mp type set mode": the item's (by its name) */
static void multiplayer_mode_set(struct widget_instance *widget)
{
	char const *name = widget->name;

	/* (an instance's name is cut to 31 characters:
	"multiplayer_type_create_interne") */
	multiplayer.mode =
		strstr(name, "create_inter") ? _multiplayer_mode_host_internet :
		strstr(name, "create_lan") ? _multiplayer_mode_host_lan :
		strstr(name, "join_lan") ? _multiplayer_mode_lan :
		strstr(name, "join_direct") ? _multiplayer_mode_direct_link : _multiplayer_mode_server_browser;
}

/* player 1's profile for the controller's player in a network game */
static boolean multiplayer_player(short controller)
{
	struct player_profile profile;

	if (!campaign_profile(controller, &profile))
		return campaign_fail();
	return ui_widget_port_multiplayer_player(controller, player_ui_get_active_player_profile_index(0));
}

/* Create Game's ("join controller to mp game", first): player 1 in, and
the server made: an internet one (an invite, Discord) or a LAN one */
static boolean multiplayer_host(struct widget_instance *widget, struct event_record *event, short controller,
	boolean *widget_deleted)
{
	multiplayer_mode_set(widget);
	if (!multiplayer_player(controller))
		return FALSE;
	p2p_set_hosting_allowed(multiplayer.mode == _multiplayer_mode_host_internet);
	/* (in the server browser, if PUBLIC: Server Setup's LISTING, which a
	new game starts with as network.host_public says) */
	multiplayer.game_private = !config_boolean("network.host_public");
	p2p_set_hosting_public(multiplayer.mode == _multiplayer_mode_host_internet && !multiplayer.game_private);
	multiplayer.cooperative_maximum_players_set = FALSE;
	return ui_widget_port_host(widget, event, widget_deleted);
}

/* ---- the map list (the Map screen's): its chooser's SINGLEPLAYER levels,
MULTIPLAYER maps, CUSTOM SINGLEPLAYER or CUSTOM MULTIPLAYER maps (the map
lists', above). A level or Custom Edition campaign map chosen lists the
difficulties (B goes back to the map) and is hosted as a network co-op game,
which goes to Server Setup; a multiplayer map goes on to the gametypes.
Split screen hosts only multiplayer maps, its chooser kept to the
multiplayer kinds. */

enum
{
	MAP_STEP_MAPS,
	MAP_STEP_DIFFICULTIES,
};

static struct
{
	short kind, step;
	/* hosting over the network, which co-op needs */
	boolean hosting;
	/* the multiplayer maps (ui_widget_port_multiplayer_maps): the Xbox's,
	then the Custom Edition ones */
	short map_count;
	short first, chosen;
	/* the level or Custom Edition campaign map (the kind's entry) whose
	difficulties are listed */
	short level;
} map_list;

#define SERVER_SETUP_NAME "pc\\main_menu\\multiplayer_type_select\\server_settings\\server_settings_screen"

static short map_step_count(void)
{
	if (map_list.step == MAP_STEP_DIFFICULTIES)
		return NUMBER_OF_GAME_DIFFICULTY_LEVELS;
	return map_kind_count(map_list.kind, map_list.map_count);
}

/* opens a step at the entry given */
static void map_step_open(struct widget_instance *list, short step, short entry)
{
	short count;

	map_list.step = step;
	count = map_step_count();
	map_list.chosen = (short)PIN(entry, 0, MAX(count - 1, 0));
	map_list.first = map_kind_first(map_list.chosen, count);
	map_kind_focus(list, map_list.first, map_list.chosen);
}

static void map_level_text(short level, wchar_t *text)
{
	string_get("pc\\main_menu\\map_list", level, text);
}

static void map_difficulty_text(short difficulty, wchar_t *text)
{
	string_get("pc\\main_menu\\player_profiles_select\\difficulty_names", difficulty, text);
}

/* "mp level list initialize": on the multiplayer map used last */
static boolean map_list_initialize(struct widget_instance *list)
{
	char const *const *names;
	short last_used = 0;

	map_list.hosting = global_network_game_server_get() != NULL && !network_game_is_splitscreen_local();
	/* (the maps folders looked for again as the list opens) */
	custom_edition_maps_look_again();
	map_list.map_count = ui_widget_port_multiplayer_maps(&names, &last_used);
	/* (a Custom Edition map's kind, when it was one) */
	map_list.kind = MAP_KIND_MULTIPLAYER;
	if (last_used >= xbox_multiplayer_map_count(map_list.map_count))
	{
		map_list.kind = MAP_KIND_CUSTOM_MULTIPLAYER;
		last_used -= xbox_multiplayer_map_count(map_list.map_count);
	}
	map_kind_set(list, map_list.kind);
	map_step_open(list, MAP_STEP_MAPS, last_used);
	return TRUE;
}

/* "mp map list update": the chooser's kind, the step's rows, and beside
them the level or map chosen: its picture, name and words */
static void map_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	short kind = map_kind_shown(list, map_list.hosting, map_list.kind), count, entry;
	short shown;

	if (kind != map_list.kind)
	{
		map_list.kind = kind;
		map_list.step = MAP_STEP_MAPS;
		map_list.first = map_list.chosen = 0;
	}
	count = map_step_count();
	entry = map_kind_rows_update(list, &map_list.first, count, map_list.step == MAP_STEP_DIFFICULTIES ?
		map_difficulty_text : kind == MAP_KIND_SINGLEPLAYER ? map_level_text :
		kind == MAP_KIND_MULTIPLAYER ? multiplayer_map_text :
		kind == MAP_KIND_CUSTOM_SINGLEPLAYER ? custom_campaign_map_text : custom_multiplayer_map_text);
	if (entry != NONE)
		map_list.chosen = entry;
	visible_set(named(description, "mp_map_right_item", 0), map_list.chosen < count);
	/* (the map whose difficulties are listed, else the one chosen) */
	shown = map_list.step == MAP_STEP_DIFFICULTIES ? map_list.level : map_list.chosen;
	if (kind == MAP_KIND_SINGLEPLAYER)
	{
		map_description_show(description, shown, NONE);
		level_description(description, "replay_level", shown, FALSE, NULL, NONE);
	}
	else
	{
		map_description_show(description, NONE,
			map_list.step == MAP_STEP_DIFFICULTIES || map_list.chosen < count ? map_kind_display_index(kind, shown) :
			NONE);
	}
	profile_name_show(description);
}

/* "mp level select" (the list's OK): the map chosen, or a level's
difficulties, or the level at the difficulty chosen. FALSE stays on the Map
screen (the gametypes open on TRUE). */
static boolean map_list_choose(struct widget_instance *list, boolean *widget_deleted)
{
	short chosen;

	map_list_update(list);
	chosen = map_list.chosen;

	if (chosen >= map_step_count())
		return campaign_fail();
	if (map_list.step == MAP_STEP_DIFFICULTIES)
	{
		char const *map_name = map_list.kind == MAP_KIND_SINGLEPLAYER ? main_get_solo_level_name(map_list.level) :
			custom_map_level_name(map_list.kind, map_list.level);

		/* the co-op game set up, then Server Setup in the gametypes' place */
		if (!ui_widget_port_cooperative_level_choose(map_name, chosen))
			return campaign_fail();
		return ui_widget_port_open(list, SERVER_SETUP_NAME, widget_deleted);
	}
	/* (a multiplayer map: the Xbox's, then the Custom Edition ones, in the
	multiplayer maps) */
	if (map_list.kind == MAP_KIND_MULTIPLAYER)
		return ui_widget_port_multiplayer_map_choose(chosen);
	if (map_list.kind == MAP_KIND_CUSTOM_MULTIPLAYER)
		return ui_widget_port_multiplayer_map_choose((short)(xbox_multiplayer_map_count(map_list.map_count) + chosen));
	ui_play_audio_feedback_sound(SOUND_FORWARD);
	map_list.level = chosen;
	map_step_open(list, MAP_STEP_DIFFICULTIES, (short)PIN(main_get_difficulty(), 0, NUMBER_OF_GAME_DIFFICULTY_LEVELS - 1));
	return FALSE;
}

/* "port map list back" (B on the list): from the difficulties back to the
level, else out of the Map screen */
static boolean map_list_back(struct widget_instance *list, boolean *widget_deleted)
{
	ui_play_audio_feedback_sound(SOUND_BACK);
	if (map_list.step == MAP_STEP_DIFFICULTIES)
	{
		map_step_open(list, MAP_STEP_MAPS, map_list.level);
		return TRUE;
	}
	ui_widget_port_go_back(list);
	*widget_deleted = TRUE;
	return TRUE;
}

/* (the gametype editor's: Server Setup's copy of the game's gametype) */
static void gametype_setup_begin(void);
static void gametype_setup_end(void);
static boolean gametype_setup_apply(void);
static void gametype_setup_type(wchar_t *text);

/* ---- the gametype list: its spinner's bank (STANDARD: the Xbox's built-in
ones, OPENCE: OpenCE's built-in ones, game_modes.c, CUSTOM: those saved) in
its first row, then the bank's gametypes */

enum
{
	GAMETYPE_BANK_STANDARD = 0,
	GAMETYPE_BANK_OPENCE,
	GAMETYPE_BANK_CUSTOM,
	NUMBER_OF_GAMETYPE_BANKS
};

/* a gametype's bank */
static short gametype_bank(long profile_index)
{
	struct game_variant variant;
	struct game_variant_options options;

	if (!((unsigned long)profile_index & PLAYLIST_READ_ONLY_BIT))
		return GAMETYPE_BANK_CUSTOM;
	if (playlist_profile_get(profile_index, &variant) && playlist_profile_get_options(profile_index, &options) &&
		game_modes_variant_is_opence(&variant, &options))
	{
		return GAMETYPE_BANK_OPENCE;
	}
	return GAMETYPE_BANK_STANDARD;
}

static void gametype_bank_read(short bank)
{
	short index;

	multiplayer.bank_shown = bank;
	multiplayer.bank_count = 0;
	for (index = 0; index < multiplayer.gametype_count; index++)
	{
		if (gametype_bank(multiplayer.gametypes[index]) == bank)
			multiplayer.bank[multiplayer.bank_count++] = index;
	}
	multiplayer.gametype_first = 0;
	multiplayer.gametype_chosen = 0;
}

/* a gametype's kind and rules, for the description beside a list: its game
engine's name (or its OpenCE gametype's), teams, and what wins */
static void gametype_rules_text(long profile_index, wchar_t *text, short size)
{
	struct game_variant variant;
	struct game_variant_options options;
	wchar_t const *kind;
	long engine;

	text[0] = 0;
	if (!playlist_profile_get(profile_index, &variant))
		return;
	if (!playlist_profile_get_options(profile_index, &options))
		game_variant_options_default(&variant, &options);
	engine = PIN(variant.game_engine_index, 0, 5);
	kind = game_modes_variant_kind_name(&variant, &options);
	switch (game_modes_variant_mode(&variant, &options))
	{
	case _game_mode_infection:
		if (options.time_limit > 0)
			usnprintf(text, size - 1, L"%s\r\nTeams\r\nRound time: %d min", kind, options.time_limit);
		else
			usnprintf(text, size - 1, L"%s\r\nTeams\r\nUntil no human is left", kind);
		break;
	case _game_mode_gun_game:
		usnprintf(text, size - 1, L"%s\r\nFree for all\r\nA kill with each weapon", kind);
		break;
	default:
		usnprintf(text, size - 1, L"%s\r\n%s\r\nScore to win: %ld", kind ? kind : engine_names[engine],
			variant.universal_variant.teams ? L"Teams" : L"Free for all", variant.universal_variant.score_to_win);
		break;
	}
	text[size - 1] = 0;
}

/* "mp profiles list initialize": on the gametype used last */
static boolean gametype_list_initialize(struct widget_instance *list)
{
	struct widget_instance *spinner = named(list, "list_item_0_chooser_spinner", 0);
	short last, index;

	multiplayer.gametype_count = ui_widget_port_gametypes(multiplayer.gametypes, MAXIMUM_GAMETYPES, &last);
	gametype_bank_read(multiplayer.gametype_count ? gametype_bank(multiplayer.gametypes[last]) : GAMETYPE_BANK_STANDARD);
	for (index = 0; index < multiplayer.bank_count; index++)
	{
		if (multiplayer.bank[index] == last)
			multiplayer.gametype_chosen = index;
	}
	multiplayer.gametype_first = (short)PIN(multiplayer.gametype_chosen - GAMETYPE_ROWS / 2, 0,
		MAX(0, multiplayer.bank_count - GAMETYPE_ROWS));
	if (spinner)
		spinner->parameters.list.selected_index = multiplayer.bank_shown;
	focus_row(list, (short)(1 + multiplayer.gametype_chosen - multiplayer.gametype_first));
	return TRUE;
}

/* a gametype's name, in a row's text (the name's own buffer is longer:
playlist_profile_get_display_name fills MAX_GAMENAME characters) */
static void gametype_display_name(long profile_index, wchar_t *text)
{
	wchar_t name[MAX_GAMENAME];

	text[0] = 0;
	if (playlist_profile_get_display_name(profile_index, name))
	{
		ustrncpy(text, name, ROW_TEXT_LENGTH - 1);
		text[ROW_TEXT_LENGTH - 1] = 0;
	}
}

static void gametype_name(short item, wchar_t *text)
{
	text[0] = 0;
	if (item >= 0 && item < multiplayer.bank_count)
		gametype_display_name(multiplayer.gametypes[multiplayer.bank[item]], text);
}

/* "gt select list update": the bank shown, its rows (from the second), and
the gametype chosen: its name, picture and rules */
static void gametype_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *spinner = named(list, "list_item_0_chooser_spinner", 0);
	struct widget_instance *row;
	struct widget_instance *widget;
	short row_index = 0, focused = NONE, count;

	if (spinner && spinner->parameters.list.selected_index != multiplayer.bank_shown)
		gametype_bank_read((short)PIN(spinner->parameters.list.selected_index, 0, NUMBER_OF_GAMETYPE_BANKS - 1));
	count = (short)MIN(multiplayer.bank_count, GAMETYPE_ROWS);
	/* (the rows after the spinner's: scrolled as list_scroll does) */
	for (row = list->child; row; row = row->next, row_index++)
	{
		if (row == list->focused_child && row_index >= 1 && row_index <= GAMETYPE_ROWS)
			focused = (short)(row_index - 1);
	}
	if (focused == count - 1 && multiplayer.gametype_first + count < multiplayer.bank_count && list_scrolls_at_end())
	{
		multiplayer.gametype_first++;
		focus_row(list, (short)focused);
		focused--;
	}
	else if (focused == 0 && multiplayer.gametype_first > 0 && list_scrolls_at_end())
	{
		multiplayer.gametype_first--;
		focus_row(list, 2);
		focused++;
	}
	if (focused != NONE)
		multiplayer.gametype_chosen = (short)(multiplayer.gametype_first + focused);
	for (row = list->child, row_index = 0; row; row = row->next, row_index++)
	{
		wchar_t text[ROW_TEXT_LENGTH];

		if (row_index < 1 || row_index > GAMETYPE_ROWS)
			continue;
		row->visible = row_index - 1 < count;
		gametype_name((short)(multiplayer.gametype_first + row_index - 1), text);
		text_set(named(row, "list_item_text", 0), text);
		if (named(row, "list_item_arrows", 0))
			named(row, "list_item_arrows", 0)->visible = row == list->focused_child;
		if (named(row, "scroll_up_button", 0))
			named(row, "scroll_up_button", 0)->visible = FALSE;
		if (named(row, "scroll_down_button", 0))
			named(row, "scroll_down_button", 0)->visible = FALSE;
	}
	visible_set(named(description, "locked_gametype_icon", 0), FALSE);
	visible_set(named(description, "gametype_right_item", 0), multiplayer.bank_count > 0);
	if (multiplayer.gametype_chosen < multiplayer.bank_count)
	{
		struct game_variant variant;
		wchar_t text[ROW_TEXT_LENGTH * 2];

		gametype_name(multiplayer.gametype_chosen, text);
		text_set(named(description, "gametype_right_name", 0), text);
		if (playlist_profile_get(multiplayer.gametypes[multiplayer.bank[multiplayer.gametype_chosen]], &variant))
		{
			long engine = PIN(variant.game_engine_index, 0, 5);

			if ((widget = named(description, "gametype_right_pic", 0)) != NULL)
				widget->animation.current_frame_index = (short)engine;
			gametype_rules_text(multiplayer.gametypes[multiplayer.bank[multiplayer.gametype_chosen]], text,
				NUMBEROF(text));
			text_set(named(description, "gametype_right_data", 0), text);
		}
	}
	profile_name_show(description);
}

/* "mp profile set for game" */
static boolean gametype_choose(struct widget_instance *list)
{
	gametype_list_update(list);
	gametype_setup_end();
	if (multiplayer.gametype_chosen >= multiplayer.bank_count)
		return campaign_fail();
	return ui_widget_port_gametype_choose(multiplayer.gametypes[multiplayer.bank[multiplayer.gametype_chosen]]);
}

/* ---- the server settings */

static void game_name_done(char const *text)
{
	short index;

	for (index = 0; text[index] && index < NUMBEROF(multiplayer.game_name) - 1; index++)
		multiplayer.game_name[index] = (wchar_t)(unsigned char)text[index];
	multiplayer.game_name[index] = 0;
}

/* Whether a network game is co-op: a campaign level or a Custom Edition
campaign map, with no game engine (set up by
ui_widget_port_cooperative_level_choose). A Custom Edition map this machine
has not is taken to be one: co-op is the only game with no game engine. */
static boolean game_cooperative(struct network_game const *game)
{
	return game && !game->variant.game_engine_index &&
		(custom_edition_maps_level_campaign(game->map.name) ||
		(custom_edition_level_name(game->map.name) && custom_edition_maps_display_index(game->map.name) == NONE));
}

/* the same, for the game this machine is hosting */
static boolean hosting_cooperative(void)
{
	void *server = global_network_game_server_get();

	return server && game_cooperative(network_game_server_get_game(server));
}

/* Server Setup's gametype rows, which co-op hides */
static char const *const server_settings_gametype_rows[] =
{
	"op_game_type", "op_player_options", "op_item_options", "op_vehicle_options", "op_indicator_options",
	"op_team_options",
};

/* the most players a co-op game hosted starts with (maximum_players') */
#define COOPERATIVE_DEFAULT_PLAYERS 16
/* Server Setup's help for co-op's options' rows (CO-OP OPTIONS, VOICE AND
VOTING), PASSWORD's and MAP SHARING's (its help_strings, tools/port_settings.py) */
#define COOPERATIVE_OPTIONS_HELP 12
#define COOPERATIVE_VOICE_OPTIONS_HELP 13
#define SERVER_PASSWORD_HELP 14
#define MAP_SHARE_OPTIONS_HELP 15
/* (LISTING's PUBLIC on a machine with a publisher key: server_master.c) */
#define SERVER_LISTING_PUBLISHER_HELP 16

/* whether this machine has a publisher key (read as Server Setup opens) */
static boolean server_settings_publisher;

/* co-op's EXTRA ENEMIES' choices (network.coop_enemies_mode's values, as
gametype_options' coop_extra_enemies_spinner has them, in this order): its
amount's row is the choice's */
enum
{
	_cooperative_enemies_per_player,
	_cooperative_enemies_multiplier,
	_cooperative_enemies_none,
	NUMBER_OF_COOPERATIVE_ENEMIES_MODES
};

/* co-op's FRIENDLY FIRE's choices (network.coop_friendly_fire's values, as
gametype_options' coop_friendly_fire_spinner and server_start have them, in
this order) */
static short const cooperative_friendly_fire_modes[] =
{
	_friendly_fire_off, _friendly_fire_on, _friendly_fire_shields_only, _friendly_fire_explosives_only
};

/* the most players Server Setup shows and sets: the multiplayer game's, or
the co-op game's (COOPERATIVE_DEFAULT_PLAYERS the first time in each game
hosted: multiplayer_host) */
static short *server_settings_maximum_players_index(void)
{
	short index;

	if (!hosting_cooperative())
		return &multiplayer.maximum_players_index;
	if (!multiplayer.cooperative_maximum_players_set)
	{
		multiplayer.cooperative_maximum_players_index = NUMBEROF(maximum_players) - 1;
		for (index = 0; index < NUMBEROF(maximum_players); index++)
		{
			if (maximum_players[index] == COOPERATIVE_DEFAULT_PLAYERS)
				multiplayer.cooperative_maximum_players_index = index;
		}
		multiplayer.cooperative_maximum_players_set = TRUE;
	}
	return &multiplayer.cooperative_maximum_players_index;
}

/* Server Setup's LISTING, PRIVATE: the multiplayer game's (as
network.host_public started it), or co-op's, saved in network.coop_public */
static boolean server_settings_private(void)
{
	return hosting_cooperative() ? !config_boolean("network.coop_public") : multiplayer.game_private;
}

static void server_settings_private_set(boolean private_game)
{
	if (!hosting_cooperative())
		multiplayer.game_private = private_game;
	else if (private_game != server_settings_private() &&
		!config_write("network.coop_public", private_game ? "false" : "true"))
	{
		platform_log("menus: could not write network.coop_public to config.toml");
	}
}

/* "server settings init": the game's name (player 1's, else the one given
last), the most players, the gametype's copy (once: the screen is made
again on coming back from an option's screen) */
static boolean server_settings_initialize(struct widget_instance *list)
{
	struct widget_instance *spinner = named(list, "max_players_spinner", 0);

	/* (co-op has no gametype to edit) */
	if (!hosting_cooperative())
		gametype_setup_begin();
	if (!multiplayer.game_name[0] && player_ui_get_active_player_profile_index(0) != NONE)
	{
		struct player_profile profile;

		player_ui_get_active_player_profile(0, &profile);
		ustrncpy(multiplayer.game_name, profile.player_name, NUMBEROF(multiplayer.game_name) - 1);
	}
	if (spinner)
		spinner->parameters.list.selected_index = *server_settings_maximum_players_index();
	/* (PUBLIC or PRIVATE: this game's; the screen is made again on coming
	back from an option's screen) */
	if ((spinner = named(list, "listing_spinner", 0)) != NULL)
		spinner->parameters.list.selected_index = server_settings_private() ? 1 : 0;
	p2p_set_hosting_public(multiplayer.mode == _multiplayer_mode_host_internet && !server_settings_private());
	{
		unsigned char public_key[SERVER_MASTER_PUBLIC_KEY_SIZE];
		char key_id[2 * SERVER_MASTER_KEY_ID_SIZE + 1];

		server_settings_publisher = server_master_publisher(public_key, key_id) != 0;
	}
	return TRUE;
}

static void wide_to_text(wchar_t const *wide, char *text, short size)
{
	short index;

	for (index = 0; wide[index] && index < size - 1; index++)
		text[index] = wide[index] < 0x80 ? (char)wide[index] : '?';
	text[index] = 0;
}

/* "server settings update" */
static void server_settings_update(struct widget_instance *list)
{
	struct widget_instance *spinner = named(list, "max_players_spinner", 0);
	struct widget_instance *row = named(list, "op_server_name", 0);
	char text[TEXT_FIELD_LENGTH];

	if (spinner)
		*server_settings_maximum_players_index() = (short)PIN(spinner->parameters.list.selected_index, 0,
			NUMBEROF(maximum_players) - 1);
	wide_to_text(multiplayer.game_name, text, sizeof(text));
	text_field_show(named(list, "server_name_value", 0), text, text_field_editing(row));
	if (multiplayer.mode != _multiplayer_mode_host_internet)
		snprintf(text, sizeof(text), "NONE: A LAN GAME");
	else if (!config_boolean("network.online"))
		snprintf(text, sizeof(text), "INTERNET PLAY IS OFF (SETTINGS)");
	else if (!p2p_invite_link(text, sizeof(text)))
		snprintf(text, sizeof(text), "MADE WHEN THE GAME STARTS");
	text_field_show(named(list, "invite_value", 0), text, FALSE);
	{
		wchar_t type[ROW_TEXT_LENGTH];

		gametype_setup_type(type);
		text_set(named(list, "game_type_value", 0), type);
	}
	settings_help(list);
	{
		boolean cooperative = hosting_cooperative();
		short row;

		for (row = 0; row < NUMBEROF(server_settings_gametype_rows); row++)
			visible_set(named(list, server_settings_gametype_rows[row], 0), !cooperative);
		/* co-op's options in their place, rows opening their screens
		(tools/port_settings.py's COOP_SETUP_SCREENS), each its help */
		{
			struct widget_instance *help = list->parameters.list.extended_description;

			visible_set(named(list, "op_coop_options", 0), cooperative);
			visible_set(named(list, "op_voice_options", 0), cooperative);
			if (help && list->focused_child == named(list, "op_coop_options", 0))
				help->parameters.text_box.string_list_index = COOPERATIVE_OPTIONS_HELP;
			else if (help && list->focused_child == named(list, "op_voice_options", 0))
				help->parameters.text_box.string_list_index = COOPERATIVE_VOICE_OPTIONS_HELP;
			/* (MAP SHARING's, any game's: map_share.c) */
			else if (help && list->focused_child == named(list, "op_map_share_options", 0))
				help->parameters.text_box.string_list_index = MAP_SHARE_OPTIONS_HELP;
		}
	}
	/* LISTING (an internet game's): PUBLIC, listed in everyone's server
	browser, or PRIVATE, for this game. Its help is its choice's */
	visible_set(named(list, "op_listing", 0), multiplayer.mode == _multiplayer_mode_host_internet &&
		config_boolean("network.online") && config_boolean("network.public_lobby"));
	if ((spinner = named(list, "listing_spinner", 0)) != NULL && named(list, "op_listing", 0) &&
		named(list, "op_listing", 0)->visible)
	{
		boolean public = spinner->parameters.list.selected_index == 0;
		struct widget_instance *help = list->parameters.list.extended_description;

		server_settings_private_set(!public);
		p2p_set_hosting_public(public);
		/* (PUBLIC: through the brokers, and the master servers too with a
		publisher key) */
		if (help && list->focused_child == named(list, "op_listing", 0))
		{
			help->parameters.text_box.string_list_index = public && server_settings_publisher ?
				SERVER_LISTING_PUBLISHER_HELP : (short)(10 + spinner->parameters.list.selected_index);
		}
	}
	/* PASSWORD (a PUBLIC internet game's): its stars, NONE if it has none */
	row = named(list, "op_password", 0);
	visible_set(row, named(list, "op_listing", 0) && named(list, "op_listing", 0)->visible &&
		!server_settings_private());
	if (row && !row->visible && text_field_editing(row))
		text_field_end(FALSE);
	if (row && row->visible)
	{
		struct widget_instance *help = list->parameters.list.extended_description;
		short index;

		for (index = 0; multiplayer.game_password[index] && index < (short)sizeof(text) - 1; index++)
			text[index] = '*';
		text[index] = 0;
		text_field_show(named(list, "password_value", 0), index ? text : "NONE", text_field_editing(row));
		if (help && list->focused_child == row)
			help->parameters.text_box.string_list_index = SERVER_PASSWORD_HELP;
	}
}

static void server_password_done(char const *text)
{
	snprintf(multiplayer.game_password, sizeof(multiplayer.game_password), "%s", text);
}

/* "ss edit server password" (its row's A: begins, or ends) */
static boolean server_password_edit(struct widget_instance *row)
{
	if (text_field_editing(row))
	{
		text_field_end(TRUE);
		return TRUE;
	}
	text_field_begin_masked(row, multiplayer.game_password, PASSWORD_LENGTH, server_password_done);
	return TRUE;
}

/* "ss edit server name" (its row's A: begins, or ends) */
static boolean server_name_edit(struct widget_instance *row)
{
	char text[TEXT_FIELD_LENGTH];

	if (text_field_editing(row))
	{
		text_field_end(TRUE);
		return TRUE;
	}
	wide_to_text(multiplayer.game_name, text, sizeof(text));
	text_field_begin(row, text, NUMBEROF(multiplayer.game_name) - 1, game_name_done);
	return TRUE;
}

/* "ss copy invite" */
static boolean invite_copy(void)
{
	char link[TEXT_FIELD_LENGTH];

	if (!p2p_invite_link(link, sizeof(link)))
		return campaign_fail();
	platform_clipboard_set(link);
	ui_play_audio_feedback_sound(SOUND_FORWARD);
	return TRUE;
}

/* "ss start game": the settings given the server, then the lobby */
/* the co-op game's settings, as CO-OP OPTIONS left them in config.toml
(network.coop_friendly_fire, in cooperative_friendly_fire_modes' order,
network.coop_player_collisions and network.coop_ally_friendly_fire), given
the game's server: Server Setup's START, and a dedicated server's co-op
entry (server_director.c's server_apply_entry) */
void ui_widget_port_cooperative_settings_apply(void)
{
	static char const *const friendly_fire_names[] = { "off", "on", "shields_only", "explosives_only" };
	char const *friendly_fire = config_string("network.coop_friendly_fire");
	short index;

	multiplayer.cooperative_friendly_fire = _friendly_fire_on;
	for (index = 0; friendly_fire && index < (short)NUMBEROF(friendly_fire_names); index++)
	{
		if (!strcmp(friendly_fire, friendly_fire_names[index]))
			multiplayer.cooperative_friendly_fire = cooperative_friendly_fire_modes[index];
	}
	multiplayer.cooperative_no_player_collisions = !config_boolean("network.coop_player_collisions");
	network_game_server_port_set_cooperative_friendly_fire(multiplayer.cooperative_friendly_fire);
	network_game_server_port_set_cooperative_player_collisions(!multiplayer.cooperative_no_player_collisions);
	network_game_server_port_set_cooperative_ally_friendly_fire(config_boolean("network.coop_ally_friendly_fire"));
}

static boolean server_start(void)
{
	if (text_field_editing(NULL))
		text_field_end(TRUE);
	network_game_server_port_set_settings(multiplayer.game_name,
		maximum_players[PIN(*server_settings_maximum_players_index(), 0, NUMBEROF(maximum_players) - 1)]);
	/* (its listing's password: a PUBLIC internet game's, else none) */
	p2p_set_hosting_password(multiplayer.mode == _multiplayer_mode_host_internet && !server_settings_private() ?
		multiplayer.game_password : NULL);
	/* the gametype as Server Setup's options left it (co-op keeps its own,
	and its FRIENDLY FIRE) */
	if (hosting_cooperative())
	{
		gametype_setup_end();
		ui_widget_port_cooperative_settings_apply();
	}
	else if (!gametype_setup_apply())
		return campaign_fail();
	return global_network_game_server_get() != NULL;
}

/* ---- the browser (Join Game > Server Browser, LAN, Direct Link): the
games found on the LAN, or (Direct Link) from internet play's peers
(reached by an invite link, the clipboard, or Discord) */

#define JOIN_GAME_LABELS "pc\\main_menu\\multiplayer_type_select\\join_game\\join_game_ticker_labels"
enum { _join_game_label_servers = 6, _join_game_label_players, _join_game_label_page };

static boolean game_from_peer(struct advertised_game const *game)
{
	unsigned long address;

	return p2p_peer_address(game->xnaddr + 2, &address) != 0;
}

/* (network_test.c's probe) whether the client's advertised game at the
index is a peer's (game_from_peer) */
boolean menu_advertised_game_from_peer(long index)
{
	void *client = global_network_game_client_get();
	struct advertised_game *games = client ? network_game_client_get_available_games(client) : NULL;

	return games && index >= 0 && index < MAXIMUM_ADVERTISED_GAMES && game_from_peer(&games[index]);
}

static boolean advertised_in_progress(struct advertised_game *game)
{
	return network_game_client_advertised_game_in_progress(global_network_game_client_get(), game);
}

static void browser_games_read(void)
{
	void *client = global_network_game_client_get();
	struct advertised_game *games = client ? network_game_client_get_available_games(client) : NULL;
	short pass, index;

	multiplayer.game_count = 0;
	if (!games || multiplayer.mode == _multiplayer_mode_server_browser)
		return;
	/* (the open games, then those under way) */
	for (pass = 0; pass < 2; pass++)
	{
		for (index = 0; index < MAXIMUM_ADVERTISED_GAMES; index++)
		{
			struct advertised_game *game = &games[index];

			if (network_game_client_advertised_game_is_valid(game) && !advertised_in_progress(game) == (pass == 0) &&
				game_from_peer(game) == (multiplayer.mode == _multiplayer_mode_direct_link))
			{
				multiplayer.games[multiplayer.game_count++] = game;
			}
		}
	}
	if (multiplayer.game_chosen >= multiplayer.game_count)
		multiplayer.game_chosen = (short)MAX(0, multiplayer.game_count - 1);
}

static short browser_row_index(struct widget_instance *row)
{
	return row && !strncmp(row->name, "server_item_", 12) ? (short)(atoi(row->name + 12) - 1) : NONE;
}

/* the ping each row shows (-1: none), coloured by pc_menu_text_color: green
under PING_GOOD milliseconds, yellow under PING_FAIR, red above; a row's
status word (JOINING, NEWER, OLDER) in yellow (marked), and a game of
another network version's row grey */
static short browser_row_pings[BROWSER_ROWS];
static boolean browser_row_marked[BROWSER_ROWS];
static boolean browser_row_greyed[BROWSER_ROWS];
enum
{
	PING_GOOD = 80,
	PING_FAIR = 160,
};
#define PING_COLOR_GOOD 0x5ED38CFFUL
#define PING_COLOR_FAIR 0xE8C547FFUL
#define PING_COLOR_POOR 0xE86A5AFFUL
#define ROW_COLOR_GREYED 0x8A8F99FFUL

/* ui_widget.c: a text's colour in place of its own (FALSE: its own): a
server browser row's ping by how good it is */
boolean pc_menu_text_color(struct widget_instance *widget, real_argb_color *color)
{
	short row = widget->parent ? browser_row_index(widget->parent) : NONE;
	short ping;
	unsigned long rgba;

	if (row < 0 || row >= BROWSER_ROWS || strncmp(widget->name, "server_item_", 12))
		return FALSE;
	ping = browser_row_pings[row];
	if (strcmp(widget->name, "server_item_ping"))
		rgba = ROW_COLOR_GREYED;
	else if (browser_row_marked[row])
		rgba = PING_COLOR_FAIR;
	else if (ping >= 0)
		rgba = ping < PING_GOOD ? PING_COLOR_GOOD : ping < PING_FAIR ? PING_COLOR_FAIR : PING_COLOR_POOR;
	else
		rgba = ROW_COLOR_GREYED;
	if (rgba == ROW_COLOR_GREYED && !browser_row_greyed[row])
		return FALSE;
	color->red = (real)((rgba >> 24) & 0xFF) / 255.0f;
	color->green = (real)((rgba >> 16) & 0xFF) / 255.0f;
	color->blue = (real)((rgba >> 8) & 0xFF) / 255.0f;
	return TRUE;
}

/* a bar of buttons' focus, off one hidden: on its first one shown */
static void focus_off_hidden(struct widget_instance *bar)
{
	struct widget_instance *button;
	short button_index = 0;

	for (button = bar && bar->focused_child && !bar->focused_child->visible ? bar->child : NULL; button;
		button = button->next, button_index++)
	{
		if (button->visible)
		{
			bar->focused_child = button;
			bar->parameters.list.selected_index = button_index;
			break;
		}
	}
}

/* whether the browser's list's child can take focus (once visible): the
server browser's tab (its source) and column titles (its order) too */
static boolean browser_item_usable(struct widget_instance *child)
{
	return browser_row_index(child) != NONE || !strcmp(child->name, "join_game_button_bar") ||
		(multiplayer.mode == _multiplayer_mode_server_browser &&
			(!strcmp(child->name, "op_browser_mode") || !strcmp(child->name, "join_game_header")));
}

/* the browser's focus, off what cannot have it (at the start, or a row whose
game went): the first game, else the buttons (off those hidden); the row of
the game chosen is `chosen_row` (NONE: none shown) */
static void browser_focus(struct widget_instance *list, short chosen_row)
{
	char const *const choices[] = { "server_item_1", "join_game_button_bar" };
	struct widget_instance *focused = list->focused_child;
	struct widget_instance *row;
	short index;

	/* (the rows' backgrounds, as Custom Edition's: the chosen game's
	outlined in yellow, the focused one's in blue. The engine shows a list
	item's focus only on a bitmap of two frames, and theirs has three:
	normal, focused, selected) */
	for (row = list->child; row; row = row->next)
	{
		short row_index = browser_row_index(row);

		if (row_index != NONE)
			row->animation.current_frame_index = row_index == chosen_row ? 2 : row == focused ? 1 : 0;
	}
	focus_off_hidden(named(list, "join_game_button_bar", 0));
	if (focused && focused->visible && !focused->disabled)
		return;
	for (index = 0; index < NUMBEROF(choices); index++)
	{
		struct widget_instance *child;
		short child_index = 0;

		for (child = list->child; child; child = child->next, child_index++)
		{
			if (!strcmp(child->name, choices[index]) && child->visible)
			{
				list->focused_child = child;
				list->parameters.list.selected_index = child_index;
				return;
			}
		}
	}
}

/* (the server browser's: below) */
static void lobby_browser_begin(void);
static void lobby_browser_update(struct widget_instance *list);
static boolean lobby_browser_select(struct widget_instance *widget, short controller, short row,
	boolean *widget_deleted);

/* "gamespy screen init": the mode's title and parts; the games' client */
static boolean browser_initialize(struct widget_instance *screen, struct event_record *event,
	boolean *widget_deleted)
{
	char const *title = multiplayer.mode == _multiplayer_mode_lan ? "header_lan" :
		multiplayer.mode == _multiplayer_mode_direct_link ? "header_direct_link" : "header_server_browser";
	char const *const titles[] = { "header_internet", "header_lan", "header_direct_link", "header_server_browser" };
	char const *const unused[] = { "op_browser_mode", "join_game_button_update", "join_game_button_filters" };
	boolean server_browser = multiplayer.mode == _multiplayer_mode_server_browser;
	short index;

	server_browser_initialize(screen);
	for (index = 0; index < NUMBEROF(titles); index++)
		visible_set(named(screen, titles[index], 0), !strcmp(titles[index], title));
	/* (the server browser's tab, its source of games, and FILTERS are its
	own; Update stays hidden: REFRESH does what it did) */
	for (index = 0; index < NUMBEROF(unused); index++)
		visible_set(named(screen, unused[index], 0), server_browser && index != 1);
	if (server_browser && named(screen, "browser_mode_spinner", 0))
		setting_load(named(screen, "browser_mode_spinner", 0));
	visible_set(named(screen, "button_clipboard", 0), multiplayer.mode == _multiplayer_mode_direct_link);
	visible_set(named(screen, "join_game_button_refresh", 0), multiplayer.mode == _multiplayer_mode_server_browser);
	{
		struct widget_instance *list = named(screen, "join_game_items_list", 0);
		struct widget_instance *child;

		/* (only the games' rows, the buttons and the server browser's tab
		and column titles take focus: not the ticker) */
		for (child = list ? list->child : NULL; child; child = child->next)
			child->disabled = !browser_item_usable(child);
		if (list)
			browser_focus(list, NONE);
	}
	visible_set(named(screen, "scroll_up_button", 0), FALSE);
	visible_set(named(screen, "scroll_down_button", 0), FALSE);
	/* (the columns' sort arrows, both of each over its title: LAN's and
	Direct Link's lists are not sorted; the server browser shows its order's,
	lobby_browser_sort_arrows) */
	for (index = 0; named(screen, "header_sort_arrows", index); index++)
		visible_set(named(screen, "header_sort_arrows", index), FALSE);
	multiplayer.game_chosen = 0;
	/* (the server browser's games are internet play's listings; joining
	one reaches its host, whose game the client then finds as Direct Link's) */
	if (multiplayer.mode == _multiplayer_mode_server_browser)
		lobby_browser_begin();
	return ui_widget_port_browse(screen, event, widget_deleted);
}

/* a scenario's name without its path */
static char const *scenario_name(char const *path)
{
	char const *name = strrchr(path, '\\');

	return name ? name + 1 : path;
}

/* the campaign level whose scenario a path or name is (a network co-op
game's map), else NONE */
static short campaign_level_of(char const *map_name)
{
	short level;

	/* (a Custom Edition map, custom_maps\a30, is never one) */
	if (custom_edition_level_name(map_name))
		return NONE;
	for (level = 0; level < NUMBER_OF_SINGLE_PLAYER_LEVELS; level++)
	{
		if (!_stricmp(scenario_name(main_get_solo_level_name(level)), scenario_name(map_name)))
			return level;
	}
	return NONE;
}

/* a game's type as the browsers and lobby show it: its engine's name, or
CO-OP for one with none (network co-op: ui_widget_port_cooperative_level_choose) */
static wchar_t const *game_type_name(long engine_type)
{
	return engine_type == 0 ? L"CO-OP" : engine_names[PIN(engine_type, 0, 5)];
}

/* ... or its OpenCE gametype's kind (its advertisement's or listing's:
game_modes.h), "HC" for Hardcore in the browsers' narrow column */
static wchar_t const *game_kind_name(long engine_type, byte kind, boolean abbreviated)
{
	wchar_t const *name = game_modes_kind_name(engine_type, kind, abbreviated);

	return name ? name : game_type_name(engine_type);
}

/* an advertised (LAN, Direct Link) game's kind */
static byte advertised_game_kind(struct advertised_game *game)
{
	return network_game_client_advertised_game_kind(global_network_game_client_get(), game);
}

/* a map's name as the menus show it (a campaign level's too, hosted as
network co-op; its scenario's name, if not one of theirs), from its
scenario's path or name */
static void map_display_name(char const *map_name, wchar_t *text)
{
	char const *const *names;
	short index;
	short count = xbox_multiplayer_map_count(ui_widget_port_multiplayer_maps(&names, NULL));

	/* (a Custom Edition map's, if this machine has it: custom_edition_maps.c) */
	if (custom_edition_level_name(map_name))
	{
		short display_index = custom_edition_maps_display_index(map_name);
		wchar_t const *name = display_index != NONE ? custom_edition_maps_name(display_index) : NULL;

		if (name)
		{
			ustrncpy(text, name, ROW_TEXT_LENGTH - 1);
			text[ROW_TEXT_LENGTH - 1] = 0;
			return;
		}
		map_name = scenario_name(map_name);
		count = 0;
	}
	for (index = 0; index < count; index++)
	{
		if (!_stricmp(scenario_name(names[index]), scenario_name(map_name)))
		{
			string_get("pc\\main_menu\\mp_map_list", index, text);
			return;
		}
	}
	if ((index = campaign_level_of(map_name)) != NONE)
	{
		string_get("pc\\main_menu\\map_list", index, text);
		return;
	}
	for (index = 0; map_name[index] && index < ROW_TEXT_LENGTH - 1; index++)
		text[index] = (wchar_t)(unsigned char)map_name[index];
	text[index] = 0;
}

static void game_map_name(struct advertised_game const *game, wchar_t *text)
{
	map_display_name(game->map_name, text);
}

/* ---- Join Game > Server Browser: internet play's public games (their
hosts' listings, p2p_lobby.c: the brokers' player-hosted games and the
master servers' dedicated servers, master_list.c). Joining one joins its
invite (as Direct Link's PASTE LINK); once its host is reached its game is
among the client's, and an A press (posted) joins it, or shows its lobby if
under way.

What it shows of them is the network.browser_* settings': the top-left tab
(ALL GAMES, DEDICATED, P2P GAMES), the Filters screen (CE's, which
Network Setup's SERVER BROWSER sets too) and the column titles, which sort
the rows (again: the other way). A game of another network version is
shown greyed, NEWER or OLDER, and is not joined. The line below the rows
says where the chosen game was listed and, while it is joined, how the join
goes (p2p_join_progress); the error panel covers the list when nothing can
be listed */

#define LOBBY_BROWSER_GAMES 256
/* milliseconds: a host not reached in this long is given up (p2p's own
wait is longer) */
#define LOBBY_BROWSER_JOIN_TIMEOUT 30000
/* how long the browser looks before it says it found none, and shows a
join that failed; and before it says nothing answers */
#define LOBBY_BROWSER_LOOK_TIME 6000
#define LOBBY_BROWSER_FAILED_TIME 8000
#define LOBBY_BROWSER_UNREACHABLE_TIME 15000
/* the line below the rows: two rows' worth */
#define TICKER_TEXT_LENGTH (ROW_TEXT_LENGTH * 2)
/* between the parts of the line below the rows */
#define TICKER_SEPARATOR L" \x00B7 "

/* the browser's words (tools/port_settings.py's BROWSER_WORDS, in order) */
#define BROWSER_WORDS "pc\\main_menu\\multiplayer_type_select\\join_game\\browser_words"
enum
{
	_browser_word_of,
	_browser_word_players,
	_browser_word_player,
	_browser_word_under_way,
	_browser_word_full,
	_browser_word_dedicated_by,
	_browser_word_listed_on,
	_browser_word_master,
	_browser_word_masters,
	_browser_word_dedicated_server,
	_browser_word_player_hosted,
	_browser_word_password,
	_browser_word_newer_build,
	_browser_word_older_build,
	_browser_word_asking,
	_browser_word_answered,
	_browser_word_connected,
	_browser_word_no_answer,
	_browser_word_no_route,
	_browser_word_refused_full,
	_browser_word_refused_closed,
	_browser_word_refused_banned,
	_browser_word_refused_password,
	_browser_word_refused_version,
	_browser_word_refused_token,
	_browser_word_strict_nat,
	_browser_word_online_off,
	_browser_word_lobby_off,
	_browser_word_looking,
	_browser_word_newer_games,
	_browser_word_none_found,
	_browser_word_none_match,
	_browser_word_masters_down,
	_browser_word_dedicated,
	_browser_word_joining,
	_browser_word_newer,
	_browser_word_older,
	_browser_word_failed,
	_browser_word_live,
	_browser_word_closed,
	_browser_word_error_online_off,
	_browser_word_error_unreachable,
	_browser_word_refused_name,
	_browser_word_refused_vpn,
	_browser_word_refused_vpn_blocked,
	_browser_word_refused_link,
	_browser_word_refused_link_age,
	_browser_word_refused_link_in_use,
	_browser_word_refused_vpn_master,
	_browser_word_refused_address_full,
};

/* the words p2p_join_progress gives a join that failed, and the browser's
for each */
static struct
{
	char const *reason;
	short word;
} const join_failures[] =
{
	{ "no_answer", _browser_word_no_answer },
	{ "no_route", _browser_word_no_route },
	{ "full", _browser_word_refused_full },
	{ "closed", _browser_word_refused_closed },
	{ "banned", _browser_word_refused_banned },
	{ "name", _browser_word_refused_name },
	{ "vpn", _browser_word_refused_vpn },
	{ "vpn_blocked", _browser_word_refused_vpn_blocked },
	{ "link", _browser_word_refused_link },
	{ "link_age", _browser_word_refused_link_age },
	{ "link_in_use", _browser_word_refused_link_in_use },
	{ "vpn_master", _browser_word_refused_vpn_master },
	{ "address_full", _browser_word_refused_address_full },
	{ "password", _browser_word_refused_password },
	{ "version", _browser_word_refused_version },
	{ "token", _browser_word_refused_token },
};

/* the column titles (their widgets' names) and the order each sorts by
(NONE: none; the dedicated servers' icon's is the default order) */
static struct
{
	char const *name;
	short order;
} const sort_headers[] =
{
	{ "op_sort_header_server_name", P2P_LOBBY_SORT_NAME },
	{ "op_sort_header_map", P2P_LOBBY_SORT_MAP },
	{ "op_sort_header_type", P2P_LOBBY_SORT_TYPE },
	{ "op_sort_header_players", P2P_LOBBY_SORT_PLAYERS },
	{ "op_sort_header_locked", P2P_LOBBY_SORT_LOCKED },
	{ "op_sort_header_dedicated", P2P_LOBBY_SORT_SERVERS },
	{ "op_sort_header_classic", NONE },
	{ "op_sort_header_ping", NONE },
};

static struct
{
	struct p2p_listing games[LOBBY_BROWSER_GAMES];
	/* the games shown (of those found), the first on the rows, the one last
	focused (Join's) */
	short count, found, first, chosen;
	/* whether it has shown a game (the first has the focus) */
	boolean shown;
	unsigned long begin_time;
	/* the order (network.browser_sort's); the source the tab showed */
	short order;
	boolean reverse;
	short source;
	/* the error panel was shown since the browser opened */
	boolean error_shown;
	/* a game being joined: its host, its name, since when, the controller
	that asked; ready: its game reached (the A press posted) */
	boolean joining, ready;
	unsigned char identifier[6];
	char name[P2P_LISTING_NAME_SIZE + 1];
	unsigned long join_time;
	short controller;
	/* whether the game joined is a dedicated server's (the lobby's line) */
	boolean joining_dedicated;
	/* when the press was posted (again if it was lost) */
	unsigned long ready_time;
	/* the press was posted: the host's game may yet refuse the join, which
	the browser says when it opens again */
	boolean refusal_watched;
	/* the last that failed, when, and why (a _browser_word_*) */
	char failed_name[P2P_LISTING_NAME_SIZE + 1];
	unsigned long failed_time;
	short failed_word;
	/* a game with a password chosen: the password screen's (its own copy:
	the rows' are read again each frame), and a join it began, which the
	browser takes up again as it comes back */
	struct p2p_listing password_game;
	boolean password_joined;
} lobby_browser;

/* whether the game this machine joined last is a dedicated server's: its
advertisement's flag, or its listing's (the lobby's line, lobby_update) */
static boolean lobby_joined_dedicated;

static void lobby_dedicated_set(struct advertised_game *game, boolean listed)
{
	void *client = global_network_game_client_get();

	lobby_joined_dedicated = listed ||
		(game && client && network_game_client_advertised_game_dedicated(client, game));
}

/* one of the browser's words, of up to length characters */
static void browser_word(short word, wchar_t *text, short length)
{
	words_get(BROWSER_WORDS, word, text, length);
}

/* the word appended to the text (of size characters) */
static void text_append(wchar_t *text, short size, wchar_t const *word)
{
	size_t length = ustrlen(text);

	if ((short)length < size - 1)
		usnprintf(text + length, size - 1 - length, L"%s", word);
	text[size - 1] = 0;
}

static void text_append_word(wchar_t *text, short size, short word)
{
	wchar_t piece[TICKER_TEXT_LENGTH];

	browser_word(word, piece, NUMBEROF(piece));
	text_append(text, size, piece);
}

/* the row of the browser's chosen game, NONE if it is not on the rows */
static short browser_chosen_row(void)
{
	boolean server_browser = multiplayer.mode == _multiplayer_mode_server_browser;
	short chosen = server_browser ? lobby_browser.chosen : multiplayer.game_chosen;
	short row = server_browser ? (short)(chosen - lobby_browser.first) : chosen;

	return chosen < (server_browser ? lobby_browser.count : multiplayer.game_count) && row >= 0 &&
		row < BROWSER_ROWS ? row : NONE;
}

/* the word of a join's failure (p2p_join_progress's reason), NONE if none */
static short join_failure_word(char const *reason)
{
	short index;

	for (index = 0; index < NUMBEROF(join_failures); index++)
	{
		if (!strcmp(reason, join_failures[index].reason))
			return join_failures[index].word;
	}
	return NONE;
}

static void lobby_browser_begin(void)
{
	int reverse;

	lobby_browser.count = 0;
	lobby_browser.found = 0;
	lobby_browser.first = 0;
	lobby_browser.chosen = 0;
	lobby_browser.shown = FALSE;
	lobby_browser.error_shown = FALSE;
	lobby_browser.source = NONE;
	lobby_browser.order = (short)p2p_lobby_sort_parse(config_string("network.browser_sort"), &reverse);
	lobby_browser.reverse = reverse != 0;
	/* (going on with the join the password screen began) */
	lobby_browser.joining = lobby_browser.password_joined;
	lobby_browser.ready = FALSE;
	lobby_browser.password_joined = FALSE;
	lobby_browser.begin_time = system_milliseconds();
	/* (the host's game refused the join posted last time: why, now) */
	if (lobby_browser.refusal_watched)
	{
		char reason[16];
		int stage;
		short word;

		lobby_browser.refusal_watched = FALSE;
		p2p_join_progress(&stage, reason, sizeof(reason));
		word = stage == P2P_JOIN_FAILED ? join_failure_word(reason) : NONE;
		if (word != NONE)
		{
			csmemcpy(lobby_browser.failed_name, lobby_browser.name, sizeof(lobby_browser.failed_name));
			lobby_browser.failed_word = word;
			lobby_browser.failed_time = lobby_browser.begin_time ? lobby_browser.begin_time : 1;
		}
	}
	p2p_lobby_browse(TRUE);
}

/* "gamespy screen dispose" */
static void lobby_browser_end(void)
{
	lobby_browser.joining = lobby_browser.ready = FALSE;
	p2p_lobby_browse(FALSE);
}

static void text_to_wide(char const *text, wchar_t *wide, short size)
{
	short index;

	for (index = 0; text[index] && index < size - 1; index++)
		wide[index] = (wchar_t)(unsigned char)text[index];
	wide[index] = 0;
}

/* a master server's region label as shown, in capitals (EU, NA-E) */
static void region_to_wide(char const *region, wchar_t *wide, short size)
{
	short index;

	text_to_wide(region, wide, size);
	for (index = 0; wide[index]; index++)
	{
		if (wide[index] >= 'a' && wide[index] <= 'z')
			wide[index] = (wchar_t)(wide[index] - 'a' + 'A');
	}
}

/* the games whose names are valid, as a host keeps the names of the
machines and players that join it (network_game_server_clean_name): each
name cleaned (player_name_clean), and a game whose name has nothing left
that names it left out; returns how many are left */
static short lobby_browser_valid_games(struct p2p_listing *games, short count)
{
	short read;
	short written = 0;

	for (read = 0; read < count; read++)
	{
		wchar_t name[P2P_LISTING_NAME_SIZE + 1];
		short index;

		text_to_wide(games[read].name, name, NUMBEROF(name));
		if (!player_name_clean(name, NUMBEROF(name)))
			continue;
		/* (ASCII still: the listing's names are) */
		for (index = 0; name[index]; index++)
			games[read].name[index] = (char)name[index];
		games[read].name[index] = 0;
		if (written != read)
			games[written] = games[read];
		written++;
	}
	return written;
}

/* what a game's map is to this machine (p2p_lobby_filter): a Custom
Edition map it has not can be downloaded as it joins, if map downloads are
on (its host shares it, as hosts do unless they turned it off) */
static int lobby_browser_map_state(char const *map)
{
	char const *const *names;
	short index;
	short count;

	if (custom_edition_level_name(map))
	{
		if (custom_edition_maps_display_index(map) != NONE)
			return P2P_LOBBY_MAP_INSTALLED;
		return config_boolean("network.map_download") ? P2P_LOBBY_MAP_DOWNLOADABLE : P2P_LOBBY_MAP_MISSING;
	}
	count = xbox_multiplayer_map_count(ui_widget_port_multiplayer_maps(&names, NULL));
	for (index = 0; index < count; index++)
	{
		if (!_stricmp(scenario_name(names[index]), scenario_name(map)))
			return P2P_LOBBY_MAP_INSTALLED;
	}
	return campaign_level_of(map) != NONE ? P2P_LOBBY_MAP_INSTALLED : P2P_LOBBY_MAP_MISSING;
}

/* the settings' choices of games shown (the tab's source: source) */
static void lobby_browser_choices(struct p2p_lobby_choices *choices, short source)
{
	static char const *const engines[] = { "any", "ctf", "slayer", "oddball", "king", "race" };
	char const *maps = config_string("network.browser_maps");
	char const *engine = config_string("network.browser_gametype");
	short index;

	choices->source = source;
	choices->show_empty = config_boolean("network.browser_show_empty");
	choices->show_full = config_boolean("network.browser_show_full");
	choices->engine_type = 0;
	for (index = 0; index < NUMBEROF(engines); index++)
	{
		if (!strcmp(engine, engines[index]))
			choices->engine_type = index;
	}
	choices->maps = !strcmp(maps, "no") ? P2P_LOBBY_MAPS_NO :
		!strcmp(maps, "downloadable") ? P2P_LOBBY_MAPS_DOWNLOADABLE : P2P_LOBBY_MAPS_YES;
}

/* the top-left tab: the source of the games shown (network.browser_source,
written as it changes); under network.listings = "masters" the dedicated
servers alone, under "brokers" the games players host alone (the tab and
its setting kept as they were) */
static short lobby_browser_source(struct widget_instance *list)
{
	struct widget_instance *spinner = named(list, "browser_mode_spinner", 0);
	struct pc_menu_setting *setting = spinner ? pc_menu_setting_get(spinner->definition_tag_index) : NULL;
	short source;

	if (!spinner || !setting)
		return P2P_LOBBY_SOURCE_ALL;
	/* (CE's box behind the tab, whose highlight showed it had the focus, is
	gone: its outline clashed with the screen's own. The tab is dimmed
	instead, its words and arrows, but while it has the focus) */
	spinner->alpha_modifier = list->focused_child == spinner->parent ? 1.0f : TAB_UNFOCUSED_ALPHA;
	if (master_list_policy() == _master_listings_masters)
		spinner->parameters.list.selected_index = P2P_LOBBY_SOURCE_DEDICATED;
	else if (master_list_policy() == _master_listings_brokers)
		spinner->parameters.list.selected_index = P2P_LOBBY_SOURCE_PLAYERS;
	source = (short)PIN(spinner->parameters.list.selected_index, 0, setting->value_count - 1);
	if (lobby_browser.source != NONE && source != lobby_browser.source &&
		master_list_policy() == _master_listings_both)
	{
		config_write("network.browser_source", setting->values[source]);
		/* (a list of other games: from its top) */
		lobby_browser.first = 0;
		lobby_browser.chosen = 0;
	}
	lobby_browser.source = source;
	return source;
}

/* the game being joined, once its host is reached (the client's game from
it), else NULL */
static struct advertised_game *lobby_browser_joined_game(void)
{
	void *client = global_network_game_client_get();
	struct advertised_game *games = client ? network_game_client_get_available_games(client) : NULL;
	short index;

	for (index = 0; games && index < MAXIMUM_ADVERTISED_GAMES; index++)
	{
		if (network_game_client_advertised_game_is_valid(&games[index]) &&
			!memcmp(games[index].xnaddr + 2, lobby_browser.identifier, sizeof(lobby_browser.identifier)))
		{
			return &games[index];
		}
	}
	return NULL;
}

/* the focus on a game's row (0 to BROWSER_ROWS - 1) */
static void lobby_browser_focus_row(struct widget_instance *list, short row)
{
	struct widget_instance *child;
	short child_index = 0;
	char name[16];

	sprintf(name, "server_item_%d", row + 1);
	for (child = list->child; child; child = child->next, child_index++)
	{
		if (!strcmp(child->name, name))
		{
			list->focused_child = child;
			list->parameters.list.selected_index = child_index;
			return;
		}
	}
}

/* the column titles' arrows: the one the list is sorted by shows the way
(up: A to Z, or the fewest first), in white (its bitmaps' second frame: the
first, blue, is lost on the title's blue) */
static void lobby_browser_sort_arrows(struct widget_instance *list)
{
	short index;

	for (index = 0; index < NUMBEROF(sort_headers); index++)
	{
		struct widget_instance *header = named(list, sort_headers[index].name, 0);
		struct widget_instance *arrows = named(header, "header_sort_arrows", 0);
		short order = sort_headers[index].order;
		boolean sorted = order != NONE && order == lobby_browser.order;
		/* (names, maps and types first go A to Z; players and the others
		the most first) */
		boolean up = (order == P2P_LOBBY_SORT_NAME || order == P2P_LOBBY_SORT_MAP || order == P2P_LOBBY_SORT_TYPE) !=
			lobby_browser.reverse;
		struct widget_instance *up_arrow = named(arrows, "header_sort_up_arrow", 0);
		struct widget_instance *down_arrow = named(arrows, "header_sort_down_arrow", 0);

		visible_set(arrows, sorted);
		visible_set(up_arrow, up);
		visible_set(down_arrow, !up);
		if (up_arrow)
			up_arrow->animation.current_frame_index = 1;
		if (down_arrow)
			down_arrow->animation.current_frame_index = 1;
	}
}

/* the list and the title, hidden under a panel (their backgrounds let
them through) */
static void browser_list_show(struct widget_instance *screen, boolean shown)
{
	visible_set(named(screen, "join_game_items_list", 0), shown);
	visible_set(named(screen, "header_join_game", 0), shown);
}

/* the error panel over the list, with its words (NONE: hidden, the focus
back on the list) */
static void lobby_browser_error(struct widget_instance *screen, short word)
{
	struct widget_instance *panel = named(screen, "gamespy_error_fullscreen", 0);
	struct widget_instance *list = named(screen, "join_game_items_list", 0);

	if (!panel)
		return;
	if (word != NONE)
	{
		wchar_t text[TICKER_TEXT_LENGTH];
		long tag_index = tag_loaded('ustr', BROWSER_WORDS);
		wchar_t const *string = tag_index != NONE ? unicode_string_list_get_string(tag_index, word) : NULL;

		/* (its line breaks kept: the panel's text is a box) */
		ustrncpy(text, string ? string : L"", NUMBEROF(text) - 1);
		text[NUMBEROF(text) - 1] = 0;
		text_set_length(named(panel, "error_words", 0), text, TICKER_TEXT_LENGTH);
		visible_set(named(panel, "error_text_box", 0), FALSE);
		lobby_browser.error_shown = TRUE;
		ui_play_audio_feedback_sound(SOUND_ERROR);
	}
	panel->visible = word != NONE;
	browser_list_show(screen, word == NONE);
	if (word != NONE)
		screen->focused_child = panel;
	else if (list && screen->focused_child == panel)
		screen->focused_child = list;
}

/* the Filters screen over the list (filters_show) or not: shown, it shows
the settings; its OK writes those changed ("gamespy update filter
settings"), and CANCEL or B leaves them */
static boolean setting_shown_load(struct widget_instance *spinner, struct pc_menu_setting *setting)
{
	(void)setting;
	return setting_load(spinner);
}

static void filters_show(struct widget_instance *screen, boolean show, boolean save)
{
	struct widget_instance *filters = named(screen, "filters_screen", 0);
	struct widget_instance *list = named(screen, "join_game_items_list", 0);

	if (!filters)
		return;
	if (show)
		settings_each(filters, setting_shown_load);
	else if (save && filters->visible)
		settings_each(filters, setting_changed_save);
	filters->visible = show;
	browser_list_show(screen, !show);
	if (show)
		screen->focused_child = filters;
	else if (list && screen->focused_child == filters)
		screen->focused_child = list;
}

/* "gamespy back handler" (B) and "gamespy dismiss error/filters": a panel
over the server browser closed, if one is shown; FALSE if none is */
static boolean browser_panel_close(struct widget_instance *widget)
{
	struct widget_instance *screen = screen_of(widget);
	struct widget_instance *filters = named(screen, "filters_screen", 0);
	struct widget_instance *panel = named(screen, "gamespy_error_fullscreen", 0);

	if (filters && filters->visible)
	{
		filters_show(screen, FALSE, FALSE);
		return TRUE;
	}
	if (panel && panel->visible)
	{
		lobby_browser_error(screen, NONE);
		return TRUE;
	}
	return FALSE;
}

/* Filters' help: its row's, for the value shown (the PC version's
cap_filter_options: empty and full servers' NO and YES, the game type's,
and missing maps' NO, YES and the port's IF DOWNLOADABLE) */
static void filters_help(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	static struct
	{
		char const *row;
		short first;
		boolean by_value;
	} const helps[] =
	{
		{ "op_empty_servers", 0, TRUE },
		{ "op_full_servers", 2, TRUE },
		{ "op_game_type", 5, FALSE },
		{ "op_installed_maps", 9, TRUE },
	};
	struct widget_instance *row = list->focused_child;
	short index;

	if (strcmp(list->name, "filters_options_menu"))
		return;
	for (index = 0; row && description && index < NUMBEROF(helps); index++)
	{
		struct widget_instance *spinner;

		if (strcmp(row->name, helps[index].row))
			continue;
		for (spinner = row->child; spinner && spinner->type != 2 /* spinner */; spinner = spinner->next)
			;
		description->parameters.text_box.string_list_index = (short)(helps[index].first +
			(helps[index].by_value && spinner ? spinner->parameters.list.selected_index : 0));
	}
}

/* "gamespy select header": the list sorted by the column chosen; the one
it is sorted by, the other way (network.browser_sort) */
static boolean lobby_browser_sort_choose(struct widget_instance *header)
{
	char text[32];
	short index;

	for (index = 0; index < NUMBEROF(sort_headers); index++)
	{
		if (strcmp(header->name, sort_headers[index].name))
			continue;
		if (sort_headers[index].order == NONE)
			return TRUE;
		lobby_browser.reverse = sort_headers[index].order == lobby_browser.order ? !lobby_browser.reverse : FALSE;
		lobby_browser.order = sort_headers[index].order;
		p2p_lobby_sort_text(lobby_browser.order, lobby_browser.reverse, text, sizeof(text));
		config_write("network.browser_sort", text);
		ui_play_audio_feedback_sound(SOUND_CURSOR);
		return TRUE;
	}
	return TRUE;
}

/* the line below the rows of the chosen game: its players, map, type and
region, and where it was listed (or what it needs) */
static void lobby_browser_game_lines(struct p2p_listing const *game, wchar_t *players, wchar_t *rules, short size)
{
	wchar_t word[TICKER_TEXT_LENGTH], map[ROW_TEXT_LENGTH];

	usnprintf(players, size - 1, L"%d ", game->player_count);
	text_append_word(players, size, _browser_word_of);
	usnprintf(word, NUMBEROF(word) - 1, L" %d ", game->maximum_player_count);
	text_append(players, size, word);
	text_append_word(players, size, game->maximum_player_count == 1 ? _browser_word_player : _browser_word_players);
	map_display_name(game->map, map);
	text_append(players, size, TICKER_SEPARATOR);
	text_append(players, size, map);
	text_append(players, size, TICKER_SEPARATOR);
	text_append(players, size, game_kind_name(game->engine_type, game->kind, FALSE));
	if (game->region[0])
	{
		region_to_wide(game->region, word, NUMBEROF(word));
		text_append(players, size, TICKER_SEPARATOR);
		text_append(players, size, word);
	}
	if (!game->open || game->in_progress)
	{
		text_append(players, size, TICKER_SEPARATOR);
		text_append_word(players, size, !game->open ? _browser_word_full : _browser_word_under_way);
	}
	rules[0] = 0;
	if (game->network_version != HALO_PORT_NETWORK_VERSION)
	{
		text_append_word(rules, size, game->network_version > HALO_PORT_NETWORK_VERSION ?
			_browser_word_newer_build : _browser_word_older_build);
		return;
	}
	if (game->dedicated && game->master)
	{
		/* (the publisher's name, which only this line shows: an operator
		cannot pass for a server by its name) */
		text_append_word(rules, size, game->publisher[0] ? _browser_word_dedicated_by : _browser_word_dedicated_server);
		if (game->publisher[0])
		{
			text_to_wide(game->publisher, word, NUMBEROF(word));
			text_append(rules, size, L" ");
			text_append(rules, size, word);
		}
		text_append(rules, size, L", ");
		text_append_word(rules, size, _browser_word_listed_on);
		usnprintf(word, NUMBEROF(word) - 1, L" %d ", MAX(1, game->master_count));
		text_append(rules, size, word);
		text_append_word(rules, size, game->master_count > 1 ? _browser_word_masters : _browser_word_master);
	}
	else
		text_append_word(rules, size, game->dedicated ? _browser_word_dedicated_server : _browser_word_player_hosted);
	if (game->locked)
	{
		text_append(rules, size, TICKER_SEPARATOR);
		text_append_word(rules, size, _browser_word_password);
	}
}

/* "gamespy screen update" for the server browser: the games shown, the
rows (scrolled on at their ends), the line below them, the counts over the
titles, the titles' arrows; a join going on; the error panel */
static void lobby_browser_update(struct widget_instance *list)
{
	struct widget_instance *stats = list->parameters.list.extended_description;
	struct widget_instance *screen = screen_of(list);
	struct widget_instance *row;
	short focused = browser_row_index(list->focused_child);
	wchar_t text[TICKER_TEXT_LENGTH], rules[TICKER_TEXT_LENGTH];
	unsigned long now = system_milliseconds();
	struct p2p_lobby_choices choices;
	int join_stage = P2P_JOIN_NONE;
	int masters, masters_answered, masters_asked;
	boolean masters_down;
	short chosen;

	lobby_browser.found = lobby_browser_valid_games(lobby_browser.games,
		(short)p2p_lobby_games(lobby_browser.games, LOBBY_BROWSER_GAMES));
	lobby_browser_choices(&choices, lobby_browser_source(list));
	lobby_browser.count = (short)p2p_lobby_filter(lobby_browser.games, lobby_browser.found, &choices,
		lobby_browser_map_state);
	p2p_lobby_sort(lobby_browser.games, lobby_browser.count, lobby_browser.order, lobby_browser.reverse);
	if (focused == BROWSER_ROWS - 1 && lobby_browser.first + BROWSER_ROWS < lobby_browser.count && list_scrolls_at_end())
	{
		lobby_browser.first++;
		lobby_browser_focus_row(list, --focused);
	}
	else if (focused == 0 && lobby_browser.first > 0 && list_scrolls_at_end())
	{
		lobby_browser.first--;
		lobby_browser_focus_row(list, ++focused);
	}
	lobby_browser.first = (short)PIN(lobby_browser.first, 0, MAX(0, lobby_browser.count - BROWSER_ROWS));
	/* (the first game found takes the focus from the buttons, which had it
	while there were none) */
	if (lobby_browser.count && !lobby_browser.shown)
	{
		struct widget_instance *first_row = named(list, "server_item_1", 0);

		lobby_browser.shown = TRUE;
		if (first_row)
		{
			first_row->visible = TRUE;
			lobby_browser_focus_row(list, 0);
			focused = 0;
			lobby_browser.chosen = lobby_browser.first;
		}
	}
	if (focused != NONE && !ui_widget_port_hover_focused())
		lobby_browser.chosen = (short)(lobby_browser.first + focused);
	chosen = lobby_browser.chosen;
	for (row = list->child; row; row = row->next)
	{
		short index = browser_row_index(row);
		short place;
		struct p2p_listing const *game;
		boolean other_version;

		if (index == NONE)
			continue;
		place = index;
		index = (short)(index + lobby_browser.first);
		row->visible = index < lobby_browser.count;
		if (index >= lobby_browser.count)
			continue;
		game = &lobby_browser.games[index];
		other_version = game->network_version != HALO_PORT_NETWORK_VERSION;
		text_to_wide(game->name, text, ROW_TEXT_LENGTH);
		text_set(named(row, "server_item_server_name", 0), text);
		map_display_name(game->map, text);
		text_set(named(row, "server_item_map", 0), text);
		text_set(named(row, "server_item_type", 0), game_kind_name(game->engine_type, game->kind, TRUE));
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%d/%d", game->player_count, game->maximum_player_count);
		text_set(named(row, "server_item_players", 0), text);
		/* (its status: JOINING, NEWER or OLDER, in yellow; its host's ping,
		once probed (p2p_lobby.c), else FAILED, CLOSED, LIVE; a master
		server's, which is not probed, its region) */
		browser_row_pings[place] = game->failed || other_version ? -1 : game->ping;
		browser_row_greyed[place] = other_version;
		browser_row_marked[place] = TRUE;
		if (lobby_browser.joining && !memcmp(game->identifier, lobby_browser.identifier, 6))
			browser_word(_browser_word_joining, text, ROW_TEXT_LENGTH);
		else if (other_version)
		{
			browser_word(game->network_version > HALO_PORT_NETWORK_VERSION ? _browser_word_newer : _browser_word_older,
				text, ROW_TEXT_LENGTH);
		}
		else
		{
			browser_row_marked[place] = FALSE;
			if (game->failed)
				browser_word(_browser_word_failed, text, ROW_TEXT_LENGTH);
			else if (game->ping >= 0)
				usnprintf(text, ROW_TEXT_LENGTH - 1, L"%d", game->ping);
			else if (game->master && game->region[0])
				region_to_wide(game->region, text, ROW_TEXT_LENGTH);
			else if (!game->open)
				browser_word(_browser_word_closed, text, ROW_TEXT_LENGTH);
			else if (game->in_progress)
				browser_word(_browser_word_live, text, ROW_TEXT_LENGTH);
			else
				usnprintf(text, ROW_TEXT_LENGTH - 1, L"-");
		}
		text_set(named(row, "server_item_ping", 0), text);
		/* (the lock: a game with a password; the server: a dedicated
		server's (CE's own icon). Their bitmaps' first frame is empty, the
		PC version's for none; the next is the icon, in the menus' blue) */
		{
			struct widget_instance *lock = named(row, "server_item_locked", 0);
			struct widget_instance *dedicated = named(row, "server_item_dedicated", 0);

			visible_set(lock, game->locked);
			if (lock)
				lock->animation.current_frame_index = 1;
			visible_set(dedicated, game->dedicated);
			if (dedicated)
				dedicated->animation.current_frame_index = 1;
		}
		visible_set(named(row, "server_item_classic", 0), FALSE);
	}
	/* joining: once the host is reached, its game joined (by an A press, on
	its row); given up after a while, or once p2p says it failed */
	if (lobby_browser.ready && now - lobby_browser.ready_time > 3000 &&
		now - lobby_browser.join_time <= LOBBY_BROWSER_JOIN_TIMEOUT)
	{
		lobby_browser.ready_time = now;
		ui_widget_port_post_button(lobby_browser.controller, BUTTON_A);
	}
	if (lobby_browser.joining)
	{
		char reason[16];

		p2p_join_progress(&join_stage, reason, sizeof(reason));
		if (lobby_browser.ready)
		{
			/* (its press posted: waiting for it, or for the timeout) */
			if (now - lobby_browser.join_time > LOBBY_BROWSER_JOIN_TIMEOUT)
				lobby_browser.joining = lobby_browser.ready = FALSE;
		}
		else if (lobby_browser_joined_game())
		{
			short index;

			lobby_browser.ready = TRUE;
			lobby_browser.refusal_watched = TRUE;
			for (index = 0; index < lobby_browser.count; index++)
			{
				if (!memcmp(lobby_browser.games[index].identifier, lobby_browser.identifier, 6) &&
					index >= lobby_browser.first && index < lobby_browser.first + BROWSER_ROWS)
				{
					lobby_browser_focus_row(list, (short)(index - lobby_browser.first));
				}
			}
			lobby_browser.ready_time = now;
			ui_widget_port_post_button(lobby_browser.controller, BUTTON_A);
		}
		else if (now - lobby_browser.join_time > LOBBY_BROWSER_JOIN_TIMEOUT || join_stage == P2P_JOIN_FAILED)
		{
			/* (why: p2p's word, else the stage it got to. A strict NAT is
			what stops a host that answered: the router's UPnP, or a
			forwarded network.tunnel_port, lets it through) */
			short word = join_stage == P2P_JOIN_FAILED ? join_failure_word(reason) : NONE;

			if (word == NONE)
				word = join_stage == P2P_JOIN_ANSWERED ? _browser_word_no_route : _browser_word_no_answer;
			if (word == _browser_word_no_route && p2p_nat_strict())
				word = _browser_word_strict_nat;
			p2p_lobby_mark_failed(lobby_browser.identifier);
			csmemcpy(lobby_browser.failed_name, lobby_browser.name, sizeof(lobby_browser.failed_name));
			lobby_browser.failed_time = now ? now : 1;
			lobby_browser.failed_word = word;
			lobby_browser.joining = FALSE;
			platform_log("menus: could not reach the server browser's game %s", lobby_browser.name);
			ui_play_audio_feedback_sound(SOUND_ERROR);
		}
	}
	/* the master servers: none answered while the brokers do is told on the
	line; nothing answering at all, and internet play off, on the panel */
	if (!lobby_browser.error_shown && !config_boolean("network.online"))
		lobby_browser_error(screen, _browser_word_error_online_off);
	master_list_counts(&masters, &masters_answered, &masters_asked);
	masters_down = master_list_policy() != _master_listings_brokers && masters > 0 && masters_asked >= masters &&
		!masters_answered;
	if (!lobby_browser.error_shown && !lobby_browser.found && config_boolean("network.online") &&
		config_boolean("network.public_lobby") && now - lobby_browser.begin_time > LOBBY_BROWSER_UNREACHABLE_TIME)
	{
		boolean brokers = master_list_policy() != _master_listings_masters && p2p_lobby_brokers_reached();
		boolean masters_reached = master_list_policy() != _master_listings_brokers && masters > 0 && !masters_down;

		if (!brokers && !masters_reached)
			lobby_browser_error(screen, _browser_word_error_unreachable);
	}
	/* the line below the rows: the chosen game's (or the list's state), and
	where it was listed, or how its join goes */
	{
		wchar_t name[P2P_LISTING_NAME_SIZE + 1];

		text[0] = rules[0] = 0;
		if (!config_boolean("network.online"))
			browser_word(_browser_word_online_off, text, NUMBEROF(text));
		else if (!config_boolean("network.public_lobby"))
			browser_word(_browser_word_lobby_off, text, NUMBEROF(text));
		else if (!lobby_browser.count)
		{
			/* (an old build sees none of the games: it says why) */
			int newer = p2p_lobby_newer_games();

			if (now - lobby_browser.begin_time < LOBBY_BROWSER_LOOK_TIME)
				browser_word(_browser_word_looking, text, NUMBEROF(text));
			else if (newer)
			{
				browser_word(_browser_word_newer_games, text, NUMBEROF(text));
				usnprintf(name, NUMBEROF(name) - 1, L" %d", newer);
				text_append(text, NUMBEROF(text), name);
			}
			else
				browser_word(lobby_browser.found ? _browser_word_none_match : _browser_word_none_found, text, NUMBEROF(text));
		}
		else if (chosen < lobby_browser.count)
			lobby_browser_game_lines(&lobby_browser.games[chosen], text, rules, NUMBEROF(rules));
		if (lobby_browser.joining)
		{
			/* (its stage, and for how long: p2p_join_progress) */
			short word = join_stage == P2P_JOIN_CONNECTED || lobby_browser.ready ? _browser_word_connected :
				join_stage == P2P_JOIN_ANSWERED ? _browser_word_answered : _browser_word_asking;

			text_to_wide(lobby_browser.name, name, NUMBEROF(name));
			browser_word(word, rules, NUMBEROF(rules));
			usnprintf(text, NUMBEROF(text) - 1, L" %lu s", (now - lobby_browser.join_time) / 1000);
			text_append(rules, NUMBEROF(rules), text);
			text_set_length(named(list, "ticker_rules_info", 0), rules, TICKER_TEXT_LENGTH);
			/* (the game joined's own line, wherever the focus is) */
			text[0] = 0;
			for (chosen = 0; chosen < lobby_browser.count; chosen++)
			{
				if (!memcmp(lobby_browser.games[chosen].identifier, lobby_browser.identifier, 6))
				{
					lobby_browser_game_lines(&lobby_browser.games[chosen], text, rules, NUMBEROF(rules));
					break;
				}
			}
			if (!text[0])
				ustrncpy(text, name, NUMBEROF(text) - 1);
			chosen = lobby_browser.chosen;
			text[NUMBEROF(text) - 1] = 0;
			text_set_length(named(list, "ticker_player_info", 0), text, TICKER_TEXT_LENGTH);
		}
		else
		{
			if (lobby_browser.failed_time && now - lobby_browser.failed_time < LOBBY_BROWSER_FAILED_TIME)
			{
				text_to_wide(lobby_browser.failed_name, text, NUMBEROF(text));
				browser_word(lobby_browser.failed_word, rules, NUMBEROF(rules));
			}
			else if (masters_down && (!lobby_browser.count ||
				(chosen < lobby_browser.count && !lobby_browser.games[chosen].dedicated)))
			{
				browser_word(_browser_word_masters_down, rules, NUMBEROF(rules));
			}
			text_set_length(named(list, "ticker_player_info", 0), text, TICKER_TEXT_LENGTH);
			text_set_length(named(list, "ticker_rules_info", 0), rules, TICKER_TEXT_LENGTH);
		}
	}
	/* the counts over the titles: the players beside the page (the tab holds
	the source), the servers and how many are dedicated */
	if (stats)
	{
		wchar_t label[ROW_TEXT_LENGTH], word[ROW_TEXT_LENGTH];
		long players = 0;
		short index, dedicated = 0;
		boolean one_page = lobby_browser.count <= BROWSER_ROWS;

		for (index = 0; index < lobby_browser.count; index++)
		{
			players += lobby_browser.games[index].player_count;
			dedicated += lobby_browser.games[index].dedicated ? 1 : 0;
		}
		/* (the page only when there are more than one, and the dedicated
		servers' count when every game is listed on one: the tab is not wide
		enough for all of them. The players as far in from the panel's left
		edge, 301, as the servers are from its right, 630 (their place is
		CE's); the page centred, at 466, between them) */
		if (named(stats, "player_count_label", 0))
			named(stats, "player_count_label", 0)->horizontal_offset = 312;
		if (named(stats, "page_count_label", 0))
			named(stats, "page_count_label", 0)->horizontal_offset = 466 - 170 / 2;
		visible_set(named(stats, "page_count_label", 0), !one_page);
		string_get(JOIN_GAME_LABELS, _join_game_label_players, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s %ld", label, players);
		text_set(named(stats, "player_count_label", 0), text);
		string_get(JOIN_GAME_LABELS, _join_game_label_page, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s %d/%d", label, chosen / BROWSER_ROWS + 1,
			MAX(1, (lobby_browser.count + BROWSER_ROWS - 1) / BROWSER_ROWS));
		text_set(named(stats, "page_count_label", 0), text);
		string_get(JOIN_GAME_LABELS, _join_game_label_servers, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s %d", label, lobby_browser.count);
		if (dedicated && one_page && lobby_browser.source == P2P_LOBBY_SOURCE_ALL)
		{
			browser_word(_browser_word_dedicated, word, NUMBEROF(word));
			usnprintf(label, ROW_TEXT_LENGTH - 1, L" (%d %s)", dedicated, word);
			text_append(text, ROW_TEXT_LENGTH, label);
		}
		text_set(named(stats, "server_count_label", 0), text);
	}
	lobby_browser_sort_arrows(list);
	browser_focus(list, browser_chosen_row());
}

/* the game's host reached by its invite (joined, once it is reached:
lobby_browser_update) */
static boolean lobby_browser_join(struct p2p_listing const *game, short controller)
{
	/* (a public game: the text chat is off in it, whatever its host says:
	network_chat.c) */
	if (!p2p_join_listing(game))
		return FALSE;
	csmemcpy(lobby_browser.identifier, game->identifier, sizeof(lobby_browser.identifier));
	csmemcpy(lobby_browser.name, game->name, sizeof(lobby_browser.name));
	lobby_browser.joining = TRUE;
	lobby_browser.ready = FALSE;
	lobby_browser.refusal_watched = FALSE;
	lobby_browser.failed_time = 0;
	lobby_browser.joining_dedicated = game->dedicated != 0;
	lobby_browser.join_time = system_milliseconds();
	lobby_browser.controller = controller;
	ui_play_audio_feedback_sound(SOUND_FORWARD);
	return TRUE;
}

/* the server browser's rows and buttons: Refresh asks the hosts and the
master servers again; Filters shows its screen; a row (or Join) joins its
game's invite, a game with a password once its screen has it, one of
another network version not at all (the player is told why); the A press
posted once its host is reached joins its game (or shows its lobby, if
under way) */
static boolean lobby_browser_select(struct widget_instance *widget, short controller, short row,
	boolean *widget_deleted)
{
	struct p2p_listing const *game;
	short index;

	/* (the press posted: whatever has the focus) */
	if (lobby_browser.ready)
	{
		struct advertised_game *found = lobby_browser_joined_game();

		lobby_browser.joining = lobby_browser.ready = FALSE;
		if (!found)
			return campaign_fail();
		lobby_dedicated_set(found, lobby_browser.joining_dedicated);
		if (advertised_in_progress(found))
		{
			/* (player 1 joining it: others join them there) */
			if (!multiplayer_player(controller))
				return FALSE;
			csmemcpy(multiplayer.preview_key_id, found->key_id, sizeof(multiplayer.preview_key_id));
			csmemcpy(multiplayer.preview_xnaddr, found->xnaddr, sizeof(multiplayer.preview_xnaddr));
			return ui_widget_port_open(widget, PREVIEW_NAME, widget_deleted);
		}
		if (!found->open)
			return campaign_fail();
		if (!multiplayer_player(controller))
			return FALSE;
		return ui_widget_port_join(widget, found, LOBBY_NAME, widget_deleted);
	}
	if (strstr(widget->name, "button_refresh"))
	{
		p2p_lobby_refresh();
		lobby_browser.begin_time = system_milliseconds();
		ui_play_audio_feedback_sound(SOUND_FORWARD);
		return TRUE;
	}
	if (strstr(widget->name, "button_filters"))
	{
		filters_show(screen_of(widget), TRUE, FALSE);
		ui_play_audio_feedback_sound(SOUND_FORWARD);
		return TRUE;
	}
	if (row == NONE && !strstr(widget->name, "button_join"))
		return TRUE;
	if (lobby_browser.joining)
		return TRUE;
	index = row != NONE ? (short)(lobby_browser.first + row) : lobby_browser.chosen;
	if (index >= lobby_browser.count)
		return campaign_fail();
	game = &lobby_browser.games[index];
	if (game->network_version != HALO_PORT_NETWORK_VERSION)
	{
		network_game_client_tell_version(game->network_version, TRUE);
		return campaign_fail();
	}
	if (!game->open || !config_boolean("network.online"))
		return campaign_fail();
	/* (a game with a password: its screen asks for it, then joins) */
	if (game->locked)
	{
		lobby_browser.password_game = *game;
		lobby_browser.controller = controller;
		return ui_widget_port_open(widget, PASSWORD_NAME, widget_deleted);
	}
	return lobby_browser_join(game, controller) || campaign_fail();
}

/* ---- the server browser's password screen (a game with a password
chosen: lobby_browser_select, tools/port_settings.py): its password typed at
once, shown as stars; JOIN GAME, or Enter, joins the game with it if it is
the game's (the browser, back again, goes on with the join), else its help
says it is not and the typing begins again */

/* its help's lines (tools/port_settings.py's) */
enum
{
	_password_help_ask = 1,
	_password_help_wrong,
	_password_help_failed,
};

static struct
{
	char text[PASSWORD_LENGTH + 1];
	short help;
} password_screen;

static void password_screen_done(char const *text)
{
	snprintf(password_screen.text, sizeof(password_screen.text), "%s", text);
}

/* "port password init" */
static boolean password_screen_initialize(struct widget_instance *list)
{
	struct widget_instance *row = named(list, "op_password", 0);

	password_screen.text[0] = 0;
	password_screen.help = _password_help_ask;
	if (row)
		text_field_begin_masked(row, "", PASSWORD_LENGTH, password_screen_done);
	return TRUE;
}

/* "port password update" */
static void password_screen_update(struct widget_instance *list)
{
	struct widget_instance *help = list->parameters.list.extended_description;
	char stars[PASSWORD_LENGTH + 1];
	short index;

	for (index = 0; password_screen.text[index] && index < PASSWORD_LENGTH; index++)
		stars[index] = '*';
	stars[index] = 0;
	text_field_show(named(list, "password_value", 0), stars, text_field_editing(named(list, "op_password", 0)));
	if (help)
		help->parameters.text_box.string_list_index = password_screen.help;
}

/* "port password join" (JOIN GAME, and Enter: password_screen_edit) */
static boolean password_screen_join(struct widget_instance *widget, boolean *widget_deleted)
{
	struct p2p_listing game = lobby_browser.password_game;
	struct widget_instance *row = named(screen_of(widget), "op_password", 0);

	if (text_field_editing(NULL))
		text_field_end(TRUE);
	if (!p2p_listing_unlock(&game, password_screen.text))
	{
		password_screen.help = _password_help_wrong;
		password_screen.text[0] = 0;
		if (row)
			text_field_begin_masked(row, "", PASSWORD_LENGTH, password_screen_done);
		return campaign_fail();
	}
	if (!lobby_browser_join(&game, lobby_browser.controller))
	{
		password_screen.help = _password_help_failed;
		return campaign_fail();
	}
	password_screen.text[0] = 0;
	lobby_browser.password_joined = TRUE;
	ui_widget_port_go_back(widget);
	*widget_deleted = TRUE;
	return TRUE;
}

/* "port password back" (B, Escape): back to the browser at once, though the
password is being typed */
static boolean password_screen_back(struct widget_instance *widget, boolean *widget_deleted)
{
	if (text_field_editing(NULL))
		text_field_end(FALSE);
	password_screen.text[0] = 0;
	ui_widget_port_go_back(widget);
	*widget_deleted = TRUE;
	return TRUE;
}

/* "port password edit" (the field's row's A: the typing begins; Enter, its
end, joins) */
static boolean password_screen_edit(struct widget_instance *row, boolean *widget_deleted)
{
	if (text_field_editing(row))
		return password_screen_join(row, widget_deleted);
	text_field_begin_masked(row, password_screen.text, PASSWORD_LENGTH, password_screen_done);
	return TRUE;
}

/* "gamespy screen update": the games' rows (name, map, gametype, players),
the one chosen's line, the counts over the titles (the PC version's: its
players, page, servers) */
static void browser_update(struct widget_instance *list)
{
	struct widget_instance *row;
	struct widget_instance *stats = list->parameters.list.extended_description;
	short focused = browser_row_index(list->focused_child);

	if (multiplayer.mode == _multiplayer_mode_server_browser)
	{
		lobby_browser_update(list);
		return;
	}
	browser_games_read();
	if (focused != NONE && focused < multiplayer.game_count && !ui_widget_port_hover_focused())
		multiplayer.game_chosen = focused;
	for (row = list->child; row; row = row->next)
	{
		short index = browser_row_index(row);
		struct advertised_game *game;
		wchar_t text[ROW_TEXT_LENGTH];
		int ping;

		if (index == NONE)
			continue;
		row->visible = index < multiplayer.game_count;
		if (index >= multiplayer.game_count)
			continue;
		game = multiplayer.games[index];
		ustrncpy(text, game->game_name, NUMBEROF(game->game_name) - 1);
		text[NUMBEROF(game->game_name) - 1] = 0;
		text_set(named(row, "server_item_server_name", 0), text);
		game_map_name(game, text);
		text_set(named(row, "server_item_map", 0), text);
		text_set(named(row, "server_item_type", 0),
			game_kind_name(game->engine_type, advertised_game_kind(game), TRUE));
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%d/%d", game->player_count, game->maximum_player_count);
		text_set(named(row, "server_item_players", 0), text);
		/* (a Direct Link game's host: the tunnel's ping) */
		ping = multiplayer.mode == _multiplayer_mode_direct_link ? p2p_peer_ping(game->xnaddr + 2) : -1;
		browser_row_pings[index] = (short)ping;
		browser_row_marked[index] = browser_row_greyed[index] = FALSE;
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%d", ping);
		text_set(named(row, "server_item_ping", 0), ping >= 0 ? text : advertised_in_progress(game) ? L"LIVE" : L"");
		visible_set(named(row, "server_item_locked", 0), FALSE);
		visible_set(named(row, "server_item_dedicated", 0), FALSE);
		visible_set(named(row, "server_item_classic", 0), FALSE);
	}
	{
		wchar_t text[ROW_TEXT_LENGTH * 2];

		if (multiplayer.mode == _multiplayer_mode_server_browser)
			text[0] = 0;
		else if (!multiplayer.game_count)
		{
			usnprintf(text, NUMBEROF(text) - 1, L"%s", multiplayer.mode == _multiplayer_mode_direct_link ?
				L"Copy an invite link and PASTE LINK, or accept a Discord invite" :
				L"Looking for games on your LAN...");
		}
		else
		{
			struct advertised_game *game = multiplayer.games[multiplayer.game_chosen];

			usnprintf(text, NUMBEROF(text) - 1, L"%d players of %d, on %d machines%s", game->player_count,
				game->maximum_player_count, game->machine_count, advertised_in_progress(game) ? L": under way" : L"");
		}
		text[NUMBEROF(text) - 1] = 0;
		text_set_length(named(list, "ticker_player_info", 0), text, TICKER_TEXT_LENGTH);
		text_set_length(named(list, "ticker_rules_info", 0), L"", TICKER_TEXT_LENGTH);
	}
	if (stats)
	{
		wchar_t label[ROW_TEXT_LENGTH], text[ROW_TEXT_LENGTH];
		long players = 0;
		short index;

		for (index = 0; index < multiplayer.game_count; index++)
			players += multiplayer.games[index]->player_count;
		string_get(JOIN_GAME_LABELS, _join_game_label_players, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s %ld", label, players);
		text_set(named(stats, "player_count_label", 0), text);
		/* (one page: the list holds every game found) */
		string_get(JOIN_GAME_LABELS, _join_game_label_page, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s 1/1", label);
		text_set(named(stats, "page_count_label", 0), text);
		string_get(JOIN_GAME_LABELS, _join_game_label_servers, label);
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s %d", label, multiplayer.game_count);
		text_set(named(stats, "server_count_label", 0), text);
	}
	browser_focus(list, browser_chosen_row());
}

/* "direct ip connect go" (Direct Link's PASTE LINK): the invite
link on the clipboard reached, its game then in the list */
static boolean direct_link_from_clipboard(void)
{
	char text[TEXT_FIELD_LENGTH];
	char *link = text;
	size_t length;

	if (!platform_clipboard_get(text, sizeof(text)))
		text[0] = 0;
	while (*link == ' ' || *link == '\t' || *link == '\r' || *link == '\n')
		link++;
	for (length = strlen(link); length && (link[length - 1] == ' ' || link[length - 1] == '\t' ||
		link[length - 1] == '\r' || link[length - 1] == '\n'); length--)
		link[length - 1] = 0;
	if (!*link || !p2p_join_invite(link))
	{
		platform_log("menus: the clipboard has no invite link");
		return campaign_fail();
	}
	return TRUE;
}

/* joining the game chosen: player 1 in, then the lobby */
static boolean browser_join(struct widget_instance *widget, short controller, boolean *widget_deleted)
{
	if (multiplayer.game_chosen >= multiplayer.game_count)
		return campaign_fail();
	if (!multiplayer_player(controller))
		return FALSE;
	lobby_dedicated_set(multiplayer.games[multiplayer.game_chosen], FALSE);
	return ui_widget_port_join(widget, multiplayer.games[multiplayer.game_chosen], LOBBY_NAME, widget_deleted);
}

/* "gamespy select item" (a row: chooses its game, or joins it once chosen)
and "gamespy select button" (Join) */
static boolean browser_select(struct widget_instance *widget, struct event_record *event, short controller,
	boolean *widget_deleted)
{
	short row = browser_row_index(widget);

	/* (a row not chosen, clicked after the pointer passed over it: chosen,
	as Custom Edition's; a click on it chosen, a double click, A or JOIN
	joins its game. Not the A press a join posts: lobby_browser_select) */
	if (row != NONE && row != browser_chosen_row() && !lobby_browser.ready)
	{
		if (multiplayer.mode == _multiplayer_mode_server_browser)
			lobby_browser.chosen = (short)(lobby_browser.first + row);
		else
			multiplayer.game_chosen = row;
		ui_play_audio_feedback_sound(SOUND_CURSOR);
		return TRUE;
	}
	if (multiplayer.mode == _multiplayer_mode_server_browser)
		return lobby_browser_select(widget, controller, row, widget_deleted);
	if (row != NONE)
		multiplayer.game_chosen = row;
	if (row != NONE || strstr(widget->name, "button_join"))
	{
		/* (a game under way: its lobby first, then JOIN GAME) */
		if (multiplayer.game_chosen < multiplayer.game_count &&
			advertised_in_progress(multiplayer.games[multiplayer.game_chosen]))
		{
			/* (player 1 joining it: others join them there) */
			if (!multiplayer_player(controller))
				return FALSE;
			csmemcpy(multiplayer.preview_key_id, multiplayer.games[multiplayer.game_chosen]->key_id,
				sizeof(multiplayer.preview_key_id));
			csmemcpy(multiplayer.preview_xnaddr, multiplayer.games[multiplayer.game_chosen]->xnaddr,
				sizeof(multiplayer.preview_xnaddr));
			return ui_widget_port_open(widget, PREVIEW_NAME, widget_deleted);
		}
		return browser_join(widget, controller, widget_deleted);
	}
	return TRUE;
}

/* "player profile save changes" (Settings' OK, the profile being edited):
saved if it has changes, as the Xbox's (the saving screen follows); if it
has none (the settings' own screens write theirs to config.toml as their OK
is chosen), editing ends and the previous screen comes back, as CANCEL
(the Xbox's called that a failure, and closed every screen) */
static boolean profile_save_changes(struct widget_instance *widget, boolean *widget_deleted)
{
	if (player_ui_edit_profile_is_dirty())
	{
		if (player_ui_save_profile())
			return TRUE;
		platform_log("menus: could not save the profile's changes");
		return campaign_fail();
	}
	player_ui_end_editing_profile();
	ui_play_audio_feedback_sound(SOUND_FORWARD);
	ui_widget_port_go_back(widget);
	*widget_deleted = TRUE;
	return TRUE;
}

/* "port profile settings save" (Gamepads' OK in a single-player campaign:
menu_tags.c's pause_settings_patch): the profile saved at once, not on
Settings' OK, so that the campaign's next save of the player's profile
keeps it; saving makes it the player's own (player_ui_save_profile), and it
is edited again from what was saved, for Settings to go on with */
static boolean profile_settings_save(struct widget_instance *widget)
{
	long index = player_ui_get_edit_profile_index();

	settings_each(screen_of(widget), setting_changed_save);
	if (!player_ui_get_edit_player_profile() || !player_ui_edit_profile_is_dirty())
		return TRUE;
	if (!player_ui_save_profile())
	{
		platform_log("menus: could not save the profile's changes");
		return campaign_fail();
	}
	player_ui_begin_editing_profile(index);
	return TRUE;
}

/* "port pause end game" (the in-game pause menu's END GAME, the host's:
menu_tags.c's pause_patch): the game ends as its time limit would, its
players staying for the next (the carnage report, then the host's PICK GAME) */
static boolean pause_end_game(void)
{
	if (!global_network_game_server_get() || !game_engine_running())
		return campaign_fail();
	game_engine_end_game();
	return TRUE;
}

/* "port coop friendly fire" (network co-op's pause menu's FRIENDLY FIRE,
menu_tags.c): switches this machine's own friendly fire, from now on */
static boolean coop_friendly_fire_change(void)
{
	char const *choice = config_boolean("network.coop_own_friendly_fire") ? "false" : "true";

	if (!config_write("network.coop_own_friendly_fire", choice))
		platform_log("menus: could not write network.coop_own_friendly_fire to config.toml");
	ui_play_audio_feedback_sound(SOUND_FORWARD);
	return TRUE;
}

/* "port coop friendly fire text": the button's text, ON or OFF */
static void coop_friendly_fire_text(struct widget_instance *button)
{
	button->parameters.text_box.string_list_index = config_boolean("network.coop_own_friendly_fire") ? 0 : 1;
}

/* ---- the lobby: the game's players (up to the port's 128), its map and
gametype, the countdown */

static struct network_player *lobby_players[HALO_PORT_MAXIMUM_NETWORK_PLAYERS];
static short lobby_player_count;

/* split screen: up to 4 players on this machine. In the lobby, a controller
not playing presses START to join ("port lobby join"), then chooses a
profile (its screen, "port lobby player choose"); ADD PLAYER ("port lobby
add player") gives one gamepad its own controller first (as it shares
player 1's: pc_menu_split_players). A player's B leaves the game alone ("port
lobby leave"; the machine's last leaves it). In game, a player's pause menu's
QUIT is theirs (the Xbox's "mp game player quit") */
static struct
{
	/* ADD PLAYER chosen: one gamepad is the new player's */
	boolean adding;
	/* the controller choosing its profile, NONE if none */
	short controller;
} lobby_join = { FALSE, NONE };

/* the client's player of the controller on this machine, NULL if none */
static struct network_player *lobby_local_player(short controller)
{
	void *client = global_network_game_client_get();
	struct network_game *game = client ? network_game_client_get_game(client) : NULL;
	short machine_index = network_game_client_get_local_machine_index();
	short index;

	for (index = 0; game && machine_index != NONE && index < HALO_PORT_MAXIMUM_NETWORK_PLAYERS; index++)
	{
		struct network_player *player = &game->players[index];

		if (network_player_is_valid(player) && (short)player->machine_index == machine_index &&
			(short)player->controller_index == controller)
		{
			return player;
		}
	}
	return NULL;
}

/* whether the controller plays (or is to: joined, not yet added) */
static boolean lobby_controller_playing(short controller)
{
	return player_ui_local_player_wants_to_play_multiplayer(controller) || lobby_local_player(controller);
}

/* this machine's players in the network game (or to be) */
static short lobby_local_player_count(void)
{
	short controller, count = 0;

	if (!global_network_game_client_get())
		return 0;
	for (controller = 0; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
		count += lobby_controller_playing(controller) ? 1 : 0;
	return count;
}

/* (xinput_sdl.c) whether this machine has, or is adding, a second player: co-op,
or split screen in a network game (a player who quit keeps their part of the
screen until the game ends). Then one gamepad is its own controller, not
sharing player 1's with the keyboard */
unsigned char pc_menu_split_players(void)
{
	return player_spawn_count >= 2 || lobby_join.adding || lobby_join.controller != NONE ||
		lobby_local_player_count() >= 2 || players_port_local_player_count() >= 2;
}

/* the lobby's widget that has the focus (the one a press goes to) */
static struct widget_instance *focused_leaf(struct widget_instance *widget)
{
	while (widget->parent)
		widget = widget->parent;
	while (widget->focused_child)
		widget = widget->focused_child;
	return widget;
}

/* (xinput_sdl.c) debug.virtual_gamepad_script's clock starts (the first
time: the lobby opening) */
extern void halo_virtual_gamepads_start(void);

/* "port lobby open" (the lobby made, or come back to): no player being added */
static boolean lobby_join_reset(void)
{
	halo_virtual_gamepads_start();
	lobby_join.adding = FALSE;
	lobby_join.controller = NONE;
	return TRUE;
}

/* "port lobby add player" (ADD PLAYER, and player 1's Y): its 4-way screen
opens ("port join4 open"). That screen is player 1's, whose START ends it,
and a screen a press opens is the presser's: another controller's press asks
player 1's Y instead (FALSE: nothing opens for it) */
static boolean lobby_add_player(short controller)
{
	if (lobby_local_player_count() >= MAXIMUM_LOCAL_PLAYERS)
		return campaign_fail();
	if (controller > 0)
	{
		event_manager_post_button(0, BUTTON_Y);
		return FALSE;
	}
	lobby_join.adding = TRUE;
	return TRUE;
}

/* "port lobby join" (START): a controller not playing joins, choosing its
profile next (FALSE: no profile screen); a player's START is the focused
button's */
static boolean lobby_join_start(struct widget_instance *widget, short controller, boolean *widget_deleted)
{
	void *client = global_network_game_client_get();
	struct network_game *game = client ? network_game_client_get_game(client) : NULL;
	short state_data;

	if (lobby_controller_playing(controller))
	{
		ui_widget_port_dispatch_event(focused_leaf(widget), BUTTON_START, controller, widget_deleted);
		return FALSE;
	}
	if (!game || network_game_client_get_state(client, &state_data) != _client_state_pregame)
		return campaign_fail();
	if (lobby_player_count >= game->maximum_players)
	{
		display_error_text_deferred(L"The game is full.", NONE);
		return campaign_fail();
	}
	/* (the 4-way screen opens as player 1's, with this controller joining:
lobby_add_player) */
	lobby_join.controller = controller;
	event_manager_post_button(0, BUTTON_Y);
	return FALSE;
}

/* "port lobby leave" (B): a player leaves the game, the machine's last
leaving it (TRUE: back from the lobby); a controller not playing cancels
ADD PLAYER */
static boolean lobby_leave(struct widget_instance *widget, struct event_record *event, short controller,
	boolean *widget_deleted)
{
	if (!lobby_controller_playing(controller))
	{
		lobby_join_reset();
		return FALSE;
	}
	return ui_widget_port_unjoin_player(widget, event, widget_deleted);
}

/* "port lobby player list initialize": on the joining controller's profile,
else the first no other player of this machine has */
static boolean lobby_player_list_initialize(struct widget_instance *list)
{
	long mine = lobby_join.controller != NONE ? player_ui_get_active_player_profile_index(lobby_join.controller) : NONE;
	short index, controller;

	profile_list_read(TRUE);
	profile_list.chosen = 0;
	for (index = profile_list.count - 1; index >= 0; index--)
	{
		boolean taken = FALSE;

		for (controller = 0; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
		{
			taken |= controller != lobby_join.controller && lobby_controller_playing(controller) &&
				player_ui_get_active_player_profile_index(controller) == profile_list.indices[index];
		}
		if (!taken)
			profile_list.chosen = index;
	}
	for (index = 0; index < profile_list.count && mine != NONE; index++)
	{
		if (profile_list.indices[index] == mine)
			profile_list.chosen = index;
	}
	profile_list.first = (short)PIN(profile_list.chosen - PROFILE_ROWS / 2, 0,
		MAX(0, profile_list.count + 1 - PROFILE_ROWS));
	focus_row(list, (short)(profile_list.chosen - profile_list.first));
	return TRUE;
}

/* "port lobby player choose": the joining controller's player on the profile
chosen (the lobby's "net splitscreen prejoin players" adds them) */
static boolean lobby_player_choose(void)
{
	short controller = lobby_join.controller;

	if (controller == NONE || profile_list.chosen >= profile_list.count ||
		!ui_widget_port_multiplayer_player(controller, profile_list.indices[profile_list.chosen]))
	{
		return campaign_fail();
	}
	return TRUE;
}

/* the lobby's line under its players: how another player joins */
static void lobby_join_help(struct widget_instance *list)
{
	struct widget_instance *help = named(screen_of(list), "lobby_join_help", 0);
	wchar_t const *text = L"";
	short controller;

	if (lobby_join.adding)
	{
		text = L"New player: press START.";
	}
	else if (lobby_local_player_count() < MAXIMUM_LOCAL_PLAYERS)
	{
		for (controller = 0; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
		{
			if (input_has_gamepad(controller) && !lobby_controller_playing(controller))
				text = L"Another controller: START joins.";
		}
	}
	text_set(help, text);
}

static void lobby_row_text(short row, wchar_t *text)
{
	struct network_player *player = lobby_players[multiplayer.lobby_first + row];
	struct network_game *game = network_game_client_get_game(global_network_game_client_get());
	wchar_t name[NUMBEROF(player->name) + 1];

	ustrncpy(name, player->name, NUMBEROF(player->name));
	name[NUMBEROF(player->name)] = 0;
	if (game && game->variant.universal_variant.teams)
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s  (%s)", name, player->team_index ? L"BLUE" : L"RED");
	else
		usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s", name);
	/* (this machine's players, when it has more than one: their controllers) */
	if (lobby_local_player_count() >= 2 && (short)player->machine_index == network_game_client_get_local_machine_index())
	{
		size_t length = ustrlen(text);

		usnprintf(text + length, ROW_TEXT_LENGTH - 1 - length, L"  [P%d]", player->controller_index + 1);
	}
	text[ROW_TEXT_LENGTH - 1] = 0;
}

/* the lobby's panel's map: its picture and name */
static void lobby_map_show(struct widget_instance *description, char const *map_name)
{
	char const *const *names;
	short count = xbox_multiplayer_map_count(ui_widget_port_multiplayer_maps(&names, NULL)), map = 19, index;
	short level = campaign_level_of(map_name);
	struct widget_instance *widget;

	/* (by the scenario's own name: a map vote's choices name only that) */
	for (index = 0; index < count; index++)
	{
		if (!_stricmp(scenario_name(names[index]), scenario_name(map_name)))
			map = index;
	}
	/* (a Custom Edition map's picture and name, multiplayer or campaign, by
	its display index, if this machine has it: custom_edition_maps.c) */
	if (custom_edition_level_name(map_name) && custom_edition_maps_display_index(map_name) != NONE)
		map = custom_edition_maps_display_index(map_name);
	/* (a network co-op game's level: its picture and name, sp_levels' and
	map_list's, in the place of the map's) */
	visible_set(named(description, "lobby_map_pic", 0), level == NONE);
	visible_set(named(description, "lobby_map_name", 0), level == NONE);
	visible_set(named(description, "replay_level_right_pic", 0), level != NONE);
	visible_set(named(description, "replay_level_right_name", 0), level != NONE);
	if ((widget = named(description, "lobby_map_pic", 0)) != NULL)
		widget->animation.current_frame_index = map;
	if ((widget = named(description, "lobby_map_name", 0)) != NULL)
		widget->parameters.text_box.string_list_index = map;
	if (level == NONE)
		return;
	if ((widget = named(description, "replay_level_right_pic", 0)) != NULL)
		widget->animation.current_frame_index = level;
	if ((widget = named(description, "replay_level_right_name", 0)) != NULL)
		widget->parameters.text_box.string_list_index = level;
}

/* the lobby's rows' texts as last updated, and their players' machines,
for their speaker icons (menu_functions_text_box_drawn); and when */
static struct
{
	struct widget_instance *texts[LOBBY_ROWS];
	long machines[LOBBY_ROWS];
	short count;
	unsigned long updated_at;
} lobby_icons;

/* ui_widget.c's: a text box drawn, at the bounds its text was drawn in
(its font and justification still set): a lobby row's player talking in
voice chat (or muted) has a speaker just left of their name
(port/linux/game/network_voice.c) */
void menu_functions_text_box_drawn(struct widget_instance *widget, rectangle2d const *bounds)
{
	short row;

	/* (the Mods screen's texts: the fonts and widths their lines are
	measured in, mods_text_fits) */
	for (row = 0; widget->name && row < NUMBER_OF_MOD_TEXT_BOXES; row++)
	{
		if (!strcmp(widget->name, mod_text_box_names[row]))
		{
			mods_screen.fonts[row] = draw_string_get_font();
			mods_screen.widths[row] = (short)(bounds->x1 - bounds->x0);
			return;
		}
	}

	/* (as the lobby last updated them: the rows' widgets go with it) */
	if (!lobby_icons.count || system_milliseconds() - lobby_icons.updated_at > 250)
		return;
	for (row = 0; row < lobby_icons.count; row++)
	{
		long machine_index = lobby_icons.machines[row];
		rectangle2d icon;
		short size;

		if (lobby_icons.texts[row] != widget)
			continue;
		if (!network_voice_machine_speaking(machine_index) && !network_voice_machine_muted(machine_index))
			return;
		size = (short)MIN(14, bounds->y1 - bounds->y0);
		icon.x1 = (short)(bounds->x1 - 4);
		icon.y0 = (short)((bounds->y0 + bounds->y1 - size) / 2);
		/* (the name's ink: the icon just left of it, centred on its
		capitals) */
		if (widget->parameters.text_box.text && widget->parameters.text_box.text[0])
		{
			rectangle2d text;
			rectangle2d cursor;

			draw_unicode_string_compute_bounds(bounds, widget->parameters.text_box.text, &text, &cursor);
			if (text.x0 - size - 6 >= bounds->x0)
				icon.x1 = (short)(text.x0 - 6);
			icon.y0 = (short)(draw_unicode_string_capital_middle(bounds, widget->parameters.text_box.text) - size / 2);
		}
		icon.x0 = (short)(icon.x1 - size);
		icon.y1 = (short)(icon.y0 + size);
		network_voice_draw_icon(&icon, network_voice_machine_muted(machine_index), 1.0f);
		return;
	}
}

/* the lobby's words (tools/port_settings.py's lobby_words) */
#define LOBBY_WORDS "pc\\main_menu\\multiplayer_type_select\\lobby\\lobby_words"
enum
{
	_lobby_word_dedicated,
};

static void map_vote_lobby_check(void);

/* "port lobby update" */
static void lobby_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	void *client = global_network_game_client_get();
	struct network_game *game = client ? network_game_client_get_game(client) : NULL;
	short state_data, state = client ? network_game_client_get_state(client, &state_data) : NONE;
	wchar_t text[LOBBY_TEXT_LENGTH];
	short index;

	lobby_player_count = 0;
	for (index = 0; game && state >= _client_state_pregame && index < HALO_PORT_MAXIMUM_NETWORK_PLAYERS; index++)
	{
		if (network_player_is_valid(&game->players[index]))
			lobby_players[lobby_player_count++] = &game->players[index];
	}
	list_scroll(list, &multiplayer.lobby_first, lobby_player_count, LOBBY_ROWS);
	if (multiplayer.lobby_first > MAX(0, lobby_player_count - LOBBY_ROWS))
		multiplayer.lobby_first = (short)MAX(0, lobby_player_count - LOBBY_ROWS);
	rows_update(list, (short)MIN(lobby_player_count, LOBBY_ROWS), lobby_row_text);
	/* (the rows' players, for their speaker icons) */
	{
		struct widget_instance *row = list->child;

		lobby_icons.count = 0;
		for (index = 0; row && index < MIN(lobby_player_count, LOBBY_ROWS); row = row->next, index++)
		{
			if (strncmp(row->name, "list_item_", 10))
				break;
			lobby_icons.texts[index] = named(row, "list_item_text", 0);
			lobby_icons.machines[index] = lobby_players[multiplayer.lobby_first + index]->machine_index;
			lobby_icons.count = (short)(index + 1);
		}
		lobby_icons.updated_at = system_milliseconds();
	}
	lobby_join_help(list);
	map_vote_lobby_check();
	visible_set(named(list, "lobby_button_team", 0), game && game->variant.universal_variant.teams);
	/* (the buttons' focus, off Switch Team when it is hidden) */
	focus_off_hidden(named(list, "lobby_button_bar", 0));
	visible_set(named(description, "lobby_right_item", 0), game != NULL && state >= _client_state_pregame);
	if (!game || state < _client_state_pregame)
	{
		profile_name_show(description);
		return;
	}
	lobby_map_show(description, game->map.name);
	{
		short seconds = network_game_client_get_seconds_to_game_start(client);
		char link[TEXT_FIELD_LENGTH];
		wchar_t gametype[NUMBEROF(game->variant.human_readable_game_description) + 1];
		/* (an OpenCE gametype's kind: game_modes.c) */
		wchar_t const *kind = game_modes_variant_kind_name(&game->variant, &game->variant_options);
		/* (a dedicated server's game: its name first, and it waits for
		players while its director holds the countdown) */
		boolean dedicated = lobby_joined_dedicated && !global_network_game_server_get();
		wchar_t name[NUMBEROF(game->name) + ROW_TEXT_LENGTH];

		name[0] = 0;
		if (dedicated)
		{
			wchar_t word[ROW_TEXT_LENGTH];

			ustrncpy(name, game->name, NUMBEROF(game->name));
			name[NUMBEROF(game->name)] = 0;
			words_get(LOBBY_WORDS, _lobby_word_dedicated, word, NUMBEROF(word));
			text_append(name, NUMBEROF(name), TICKER_SEPARATOR);
			text_append(name, NUMBEROF(name), word);
			text_append(name, NUMBEROF(name), L"\r\n");
		}
		ustrncpy(gametype, game->variant.human_readable_game_description, NUMBEROF(gametype) - 1);
		gametype[NUMBEROF(gametype) - 1] = 0;
		usnprintf(text, NUMBEROF(text) - 1, L"%s%s\r\n%s\r\n%d of %d players\r\n\r\n%s", name, gametype,
			kind ? kind : engine_names[PIN(game->variant.game_engine_index, 0, 5)], lobby_player_count,
			game->maximum_players,
			seconds > 0 ? L"Starting in:" : game->machine_count < 2 || dedicated ? L"Waiting for players" : L"");
		text[NUMBEROF(text) - 1] = 0;
		if (seconds > 0)
		{
			size_t length = ustrlen(text);

			usnprintf(text + length, NUMBEROF(text) - 1 - length, L" %d", seconds);
		}
		if (global_network_game_server_get() && p2p_invite_link(link, sizeof(link)))
		{
			size_t length = ustrlen(text);

			usnprintf(text + length, NUMBEROF(text) - 1 - length, L"\r\n\r\nInvite link copied:\r\npaste it to friends");
		}
		text_set_length(named(description, "lobby_game_data", 0), text, LOBBY_TEXT_LENGTH);
	}
	profile_name_show(description);
}

/* ---- a dedicated server's map vote (network_map_vote.c): the lobby opens
its screen as a vote begins (a posted X: the lobby's X opens it again, while
the vote runs; Y is the 4-way join screen's), the choices a row each with their votes, the focused one's
map and the time left at the right; A votes (again, for another). Once the
vote is over it shows the winner a moment, and goes back to the lobby */

#define MAP_VOTE_ROWS NETWORK_MAP_VOTE_MAXIMUM_CHOICES
/* the winner shown this long before the screen goes back */
#define MAP_VOTE_WINNER_MILLISECONDS 3000

static struct
{
	/* the vote whose screen the lobby opened, and the one it closed for */
	short opened_round;
	short closed_round;
	/* when the winner was first shown (0: not yet) */
	unsigned long winner_at;
	short winner_round;
	struct network_map_vote_status status;
} map_vote_screen = { -1, -1 };

/* (the lobby's update) a vote begun since: its screen opened */
static void map_vote_lobby_check(void)
{
	struct network_map_vote_status status;

	if (network_map_vote_status_get(&status) && status.winner == NONE && status.round != map_vote_screen.opened_round)
	{
		map_vote_screen.opened_round = status.round;
		ui_widget_port_post_button(0, BUTTON_X);
	}
}

/* "port map vote open" (the lobby's X): only while a vote runs */
static boolean map_vote_open(void)
{
	struct network_map_vote_status status;

	return network_map_vote_status_get(&status) && status.winner == NONE;
}

/* "port map vote choose" (a row's A) */
static boolean map_vote_choose(struct widget_instance *widget)
{
	if (!network_map_vote_choose(sibling_index(widget)))
		return campaign_fail();
	return TRUE;
}

static void map_vote_row_text(short row, wchar_t *text)
{
	struct network_map_vote_status const *status = &map_vote_screen.status;
	wchar_t map[ROW_TEXT_LENGTH];
	wchar_t gametype[NETWORK_MAP_VOTE_GAMETYPE_SIZE];
	short index;

	map_display_name(status->map_names[row], map);
	for (index = 0; status->gametypes[row][index] && index < NETWORK_MAP_VOTE_GAMETYPE_SIZE - 1; index++)
	{
		char character = status->gametypes[row][index];

		gametype[index] = (wchar_t)(unsigned char)(character >= 'a' && character <= 'z' ? character - 'a' + 'A' : character);
	}
	gametype[index] = 0;
	usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s%s  %s  (%d)", row == status->chosen ? L"* " : L"", map, gametype,
		status->votes[row]);
	text[ROW_TEXT_LENGTH - 1] = 0;
}

/* "port map vote update" */
static void map_vote_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct network_map_vote_status *status = &map_vote_screen.status;
	wchar_t text[LOBBY_TEXT_LENGTH];
	wchar_t name[ROW_TEXT_LENGTH];
	short row;
	boolean heard = network_map_vote_status_get(status);
	unsigned long now = system_milliseconds();

	/* (no vote, or its winner shown long enough: back to the lobby, once) */
	if (heard && status->winner != NONE && map_vote_screen.winner_round != status->round)
	{
		map_vote_screen.winner_round = status->round;
		map_vote_screen.winner_at = now;
	}
	if (!heard || (status->winner != NONE && now - map_vote_screen.winner_at >= MAP_VOTE_WINNER_MILLISECONDS))
	{
		if (map_vote_screen.closed_round != (heard ? status->round : map_vote_screen.opened_round))
		{
			map_vote_screen.closed_round = heard ? status->round : map_vote_screen.opened_round;
			ui_widget_port_post_button(0, BUTTON_B);
		}
		if (!heard)
		{
			rows_update(list, 0, map_vote_row_text);
			visible_set(named(description, "lobby_right_item", 0), FALSE);
			return;
		}
	}
	rows_update(list, status->count, map_vote_row_text);
	row = focused_row(list);
	if (status->winner != NONE)
		row = status->winner;
	else if (row == NONE || row >= status->count)
		row = status->chosen != NONE ? status->chosen : 0;
	visible_set(named(description, "lobby_right_item", 0), TRUE);
	lobby_map_show(description, status->map_names[row]);
	if (status->winner != NONE)
	{
		map_display_name(status->map_names[status->winner], name);
		usnprintf(text, NUMBEROF(text) - 1, L"Next game:\r\n%s\r\n%d vote%s", name, status->votes[status->winner],
			status->votes[status->winner] == 1 ? L"" : L"s");
	}
	else
	{
		wchar_t gametype[NETWORK_MAP_VOTE_GAMETYPE_SIZE];
		short index;

		for (index = 0; status->gametypes[row][index] && index < NETWORK_MAP_VOTE_GAMETYPE_SIZE - 1; index++)
		{
			char character = status->gametypes[row][index];

			gametype[index] = (wchar_t)(unsigned char)(character >= 'a' && character <= 'z' ? character - 'a' + 'A' :
				character);
		}
		gametype[index] = 0;
		usnprintf(text, NUMBEROF(text) - 1, L"%s\r\n%d vote%s\r\n\r\n%d s left\r\n\r\n%s", gametype,
			status->votes[row], status->votes[row] == 1 ? L"" : L"s", status->seconds_left,
			status->chosen == NONE ? L"A: vote" : row == status->chosen ? L"Your vote" : L"A: change vote");
	}
	text[NUMBEROF(text) - 1] = 0;
	text_set_length(named(description, "lobby_game_data", 0), text, LOBBY_TEXT_LENGTH);
	profile_name_show(description);
}

/* ---- an in-progress game's lobby, before joining it (Direct Link and
LAN's rows of games under way): what its advertisement tells (no players'
names: they come with joining), JOIN GAME. Split screen players join here
as in the lobby (lobby_join), for the game starts at once for a machine
that joins it: player 1 as it opens, the others with START */

/* the previewed game, found again among the client's (NULL: gone) */
static struct advertised_game *preview_game(void)
{
	void *client = global_network_game_client_get();
	struct advertised_game *games = client ? network_game_client_get_available_games(client) : NULL;
	short index;

	for (index = 0; games && index < MAXIMUM_ADVERTISED_GAMES; index++)
	{
		if (network_game_client_advertised_game_is_valid(&games[index]) &&
			!csmemcmp(games[index].key_id, multiplayer.preview_key_id, sizeof(multiplayer.preview_key_id)) &&
			!csmemcmp(games[index].xnaddr, multiplayer.preview_xnaddr, sizeof(multiplayer.preview_xnaddr)))
		{
			return &games[index];
		}
	}
	return NULL;
}

/* the preview's status's end: the players joining from here, when there
are more than one (split screen) */
static void preview_players_text(wchar_t *text, short size)
{
	short controller;
	size_t length;

	if (lobby_local_player_count() < 2)
		return;
	length = ustrlen(text);
	usnprintf(text + length, size - 1 - length, L"\r\n\r\nJoining from here:");
	for (controller = 0; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
	{
		struct player_profile profile;
		wchar_t name[NUMBEROF(profile.player_name) + 1];

		if (!player_ui_local_player_wants_to_play_multiplayer(controller))
			continue;
		player_ui_get_active_player_profile(controller, &profile);
		ustrncpy(name, profile.player_name, NUMBEROF(profile.player_name));
		name[NUMBEROF(profile.player_name)] = 0;
		text[size - 1] = 0;
		length = ustrlen(text);
		usnprintf(text + length, size - 1 - length, L"\r\n%s  [P%d]", name, controller + 1);
	}
	text[size - 1] = 0;
}

/* "port lobby preview update" */
static void preview_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct advertised_game *game = preview_game();
	boolean valid = game != NULL;
	wchar_t text[LOBBY_TEXT_LENGTH], name[NUMBEROF(game->game_name) + 1];

	visible_set(named(description, "lobby_right_item", 0), valid);
	if (valid)
	{
		lobby_map_show(description, game->map_name);
		usnprintf(text, NUMBEROF(text) - 1, L"%s\r\n%d of %d players\r\non %d machines",
			game_kind_name(game->engine_type, advertised_game_kind(game), FALSE), game->player_count,
			game->maximum_player_count, game->machine_count);
		text[NUMBEROF(text) - 1] = 0;
		text_set_length(named(description, "lobby_game_data", 0), text, LOBBY_TEXT_LENGTH);
		ustrncpy(name, game->game_name, NUMBEROF(game->game_name));
		name[NUMBEROF(game->game_name)] = 0;
		/* (the text box does not wrap: lines of up to 28 characters) */
		usnprintf(text, NUMBEROF(text) - 1, L"%s\r\n\r\nThis game is under way.\r\n\r\n%s", name,
			game->open ? L"JOIN GAME joins it now;\r\nits players show then." :
			L"It cannot be joined now:\r\nit is loading, over or full.");
		text[NUMBEROF(text) - 1] = 0;
		preview_players_text(text, NUMBEROF(text));
	}
	else
		usnprintf(text, NUMBEROF(text) - 1, L"The game is gone.");
	text[NUMBEROF(text) - 1] = 0;
	text_set_length(named(list, "preview_status", 0), text, LOBBY_TEXT_LENGTH);
	lobby_join_help(list);
	profile_name_show(description);
}

/* "port lobby preview add" (START): as the lobby's ("port lobby join"), a
controller not joining chooses its profile, to join with the others; a
joining player's START is the focused button's */
static boolean preview_add(struct widget_instance *widget, short controller, boolean *widget_deleted)
{
	struct advertised_game *game = preview_game();
	short count = lobby_local_player_count();

	if (lobby_controller_playing(controller))
	{
		ui_widget_port_dispatch_event(focused_leaf(widget), BUTTON_START, controller, widget_deleted);
		return FALSE;
	}
	if (!game || !game->open || count >= MAXIMUM_LOCAL_PLAYERS)
		return campaign_fail();
	if (game->player_count + count >= game->maximum_player_count)
	{
		display_error_text_deferred(L"The game is full.", NONE);
		return campaign_fail();
	}
	lobby_join.controller = controller;
	event_manager_post_button(0, BUTTON_Y);
	return FALSE;
}

/* "port lobby preview leave" (B): a joining player stays out, the last of
them backing out (TRUE); a controller not joining cancels ADD PLAYER, else
backs out for them all (the game is not joined yet: none is kept here) */
static boolean preview_leave(short controller)
{
	short index;

	if (!lobby_controller_playing(controller))
	{
		if (lobby_join.adding)
		{
			lobby_join_reset();
			return FALSE;
		}
		for (index = 0; index < MAXIMUM_LOCAL_PLAYERS; index++)
			player_ui_local_player_left_multiplayer_game(index);
		return TRUE;
	}
	player_ui_local_player_left_multiplayer_game(controller);
	return lobby_local_player_count() == 0;
}

/* "port lobby preview join": with the players joining from here (player 1,
as the preview opened, and those added), whom the lobby asks the host for
all at once ("net splitscreen prejoin players"): it starts the machine in
the game once it has them all */
static boolean preview_join(struct widget_instance *widget, short controller, boolean *widget_deleted)
{
	struct advertised_game *game = preview_game();

	if (!game || !game->open)
		return campaign_fail();
	if (!lobby_local_player_count() && !multiplayer_player(controller))
		return FALSE;
	lobby_dedicated_set(game, lobby_joined_dedicated && p2p_joined_listed_host());
	return ui_widget_port_join(widget, game, LOBBY_NAME, widget_deleted);
}

/* ---- ADD PLAYER's screen: the Xbox's split screen profile select
(tools/port_settings.py's _join4: its widgets and bitmaps), a quarter for
each controller. A controller not playing presses A in its quarter to join,
chooses a profile on the Xbox's spinner, and presses A to select it ("1wide
plyr prof set for game": the controller's active profile); B goes back a
step. A selected or playing quarter's A (or Enter) brings the players who
selected theirs into the lobby; player 1's B goes back without them. The
keyboard's SPACE makes the keyboard player 1 alone, when it shares player
1 with a gamepad and a port is free (xinput_sdl.c's halo_keyboard_alone_set):
that gamepad then joins in a quarter of its own. Each quarter takes its own
controller's presses (pc_menu_screen_routes_by_controller) */

#define JOIN4_NAME "pc\\main_menu\\multiplayer_type_select\\lobby\\join4_screen"

/* (xinput_sdl.c) the keyboard's SPACE held; the keyboard plays player 1
alone; whether it shares port 0 with a gamepad; how many gamepads there
are */
extern unsigned char halo_keyboard_join_held(void);
extern void halo_keyboard_alone_set(unsigned char alone);
extern unsigned char halo_keyboard_shares_gamepad(void);
extern int halo_gamepad_count(void);

enum
{
	_join4_away,
	_join4_choosing,
	_join4_selected,
};

static struct
{
	unsigned char state[MAXIMUM_LOCAL_PLAYERS];
	/* the profile each choosing player's spinner started on (two joining
	at once start on different ones) */
	long starting[MAXIMUM_LOCAL_PLAYERS];
	/* the controller whose START opened the screen (it joins at once) */
	short opener;
	boolean keyboard_held;
	/* "All four controllers are in use." and the like, for a while */
	wchar_t notice[64];
	unsigned long notice_until;
} join4;

static void join4_notice(wchar_t const *text)
{
	ustrncpy(join4.notice, text, NUMBEROF(join4.notice) - 1);
	join4.notice[NUMBEROF(join4.notice) - 1] = 0;
	join4.notice_until = system_milliseconds() + 3000;
}

/* the quarters' events that set them at first (no button the PC's
controllers have): a player's shows their profile, the opener's its
spinner */
#define JOIN4_EVENT_PLAYING 4
#define JOIN4_EVENT_OPENER 5

/* "port join4 open" (the screen made, its quarters with it): nobody joining
yet, but the players in the game and the controller whose START opened it;
the host's countdown waits (as for the profile screen) */
static boolean join4_open(struct widget_instance *screen)
{
	struct widget_instance *quarters[8];
	struct widget_instance *child;
	long quarter = tag_loaded('DeLa', "pc\\main_menu\\multiplayer_type_select\\lobby\\join4_qtr_join");
	short controller, count = 0, index;

	for (controller = 0; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
		join4.state[controller] = _join4_away;
	join4.opener = lobby_join.controller;
	join4.keyboard_held = TRUE;
	/* (player 1's quarter shows their profile: the one used last, when the
	main menu cleared the active one) */
	if (player_ui_get_active_player_profile_index(0) == NONE)
	{
		struct player_profile profile;
		long last = player_ui_get_player1_last_used_profile_index();

		if (last != NONE && player_profile_get(last, &profile))
			player_ui_set_active_player_profile(0, last, &profile);
	}
	join4.notice[0] = 0;
	lobby_join.controller = NONE;
	lobby_join.adding = TRUE;
	/* (a quarter's event replaces it: the list is taken first) */
	for (child = screen->child; child && count < (short)NUMBEROF(quarters); child = child->next)
	{
		if (child->definition_tag_index == quarter)
			quarters[count++] = child;
	}
	for (index = 0; index < count; index++)
	{
		boolean deleted = FALSE;

		controller = quarters[index]->local_player_index;
		if (controller < 0 || controller >= MAXIMUM_LOCAL_PLAYERS)
			continue;
		if (lobby_controller_playing(controller))
			ui_widget_port_dispatch_event(quarters[index], JOIN4_EVENT_PLAYING, controller, &deleted);
		else if (controller > 0 && controller == join4.opener && input_has_gamepad(controller))
			ui_widget_port_dispatch_event(quarters[index], JOIN4_EVENT_OPENER, controller, &deleted);
	}
	join4.opener = NONE;
	return TRUE;
}

/* "port join4 join" (A, START): a controller not playing joins (FALSE:
its quarter stays) */
static boolean join4_join(short controller)
{
	return controller > 0 && controller < MAXIMUM_LOCAL_PLAYERS && input_has_gamepad(controller) &&
		!lobby_controller_playing(controller);
}

/* whether another player of this machine plays, or has selected, the
profile; with choosing, or has a spinner started on it */
static boolean join4_profile_has(short controller, long profile, boolean choosing)
{
	short other;

	for (other = 0; other < MAXIMUM_LOCAL_PLAYERS; other++)
	{
		if (other == controller)
			continue;
		if ((lobby_controller_playing(other) || join4.state[other] == _join4_selected) &&
			player_ui_get_active_player_profile_index(other) == profile)
		{
			return TRUE;
		}
		if (choosing && join4.state[other] == _join4_choosing && join4.starting[other] == profile)
			return TRUE;
	}
	return FALSE;
}

static boolean join4_profile_taken(short controller, long profile)
{
	return join4_profile_has(controller, profile, FALSE);
}

static boolean join4_profile_free(short controller, long profile)
{
	return !join4_profile_has(controller, profile, TRUE);
}

/* "port join4 select" (A, START on the spinner): the profile is the
controller's ("1wide plyr prof set for game"), unless another player has it
(FALSE: the spinner stays) */
static boolean join4_select(struct widget_instance *widget, struct event_record *event, boolean *widget_deleted)
{
	short controller = controller_of(widget);

	if (!game_function("1wide plyr prof set for game", widget, event, widget_deleted))
		return FALSE;
	if (join4_profile_taken(controller, player_ui_get_active_player_profile_index(controller)))
	{
		join4_notice(L"Another player has that profile.");
		ui_play_audio_feedback_sound(SOUND_ERROR);
		return FALSE;
	}
	return TRUE;
}

/* "port join4 choosing", "port join4 leave", "port join4 ready", "port
join4 unready": the controller's quarter */
static boolean join4_set(short controller, unsigned char state)
{
	if (controller >= 0 && controller < MAXIMUM_LOCAL_PLAYERS)
		join4.state[controller] = state;
	return TRUE;
}

/* "port join4 back" (player 1's B): back to the lobby, nobody added
(FALSE for another player's) */
static boolean join4_back(short controller)
{
	if (controller > 0)
		return FALSE;
	lobby_join_reset();
	return TRUE;
}

/* "port join4 done" (a selected or playing quarter's A): those who
selected their profile join, back to the lobby (whose "net splitscreen
prejoin players" adds them); FALSE while a player is still choosing, and
for the keyboard's SPACE (join4_keyboard's) */
static boolean join4_done(short pressed_by)
{
	short controller;

	if (pressed_by == 0 && halo_keyboard_join_held())
		return FALSE;
	for (controller = 1; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
	{
		if (join4.state[controller] == _join4_choosing)
		{
			join4_notice(L"Waiting for every player to select a profile.");
			return FALSE;
		}
	}
	for (controller = 1; controller < MAXIMUM_LOCAL_PLAYERS; controller++)
	{
		long profile = player_ui_get_active_player_profile_index(controller);

		if (join4.state[controller] == _join4_selected && profile != NONE &&
			!lobby_controller_playing(controller) && !ui_widget_port_multiplayer_player(controller, profile))
		{
			platform_log("menus: player %d could not join", controller + 1);
		}
	}
	lobby_join_reset();
	return TRUE;
}

/* the keyboard's SPACE: player 1 alone, freeing the gamepad it shared */
static void join4_keyboard(void)
{
	boolean down = halo_keyboard_join_held() != 0;
	boolean pressed = down && !join4.keyboard_held;

	join4.keyboard_held = down;
	if (!pressed || !halo_keyboard_shares_gamepad())
		return;
	if (halo_gamepad_count() >= MAXIMUM_LOCAL_PLAYERS)
	{
		join4_notice(L"All four controllers are in use.");
		return;
	}
	halo_keyboard_alone_set(TRUE);
	join4_notice(L"The keyboard plays alone: controller 1 can join.");
}

/* "port join4 update" (each frame): SPACE, and the keyboard's keys (the
controller's are the Xbox's, at the bottom right) */
static void join4_update(struct widget_instance *widget)
{
	struct widget_instance *screen = screen_of(widget);

	join4_keyboard();
	text_set(named(screen, "join4_hint", 0),
		system_milliseconds() < join4.notice_until ? join4.notice :
		halo_keyboard_shares_gamepad() && halo_gamepad_count() < MAXIMUM_LOCAL_PLAYERS ?
			L"SPACE: join    ENTER: select    ESC: back" : L"ENTER: select    ESC: back");
}

static long join4_tag(void)
{
	return tag_loaded('DeLa', JOIN4_NAME);
}

/* (ui_widget.c) whether the screen reads every controller's events, not
only its player's: the lobby, where another controller's START joins and
a player's B leaves (lobby_join_start, lobby_leave: each knows its
controller), however many players there are, and ADD PLAYER's quarters */
unsigned char pc_menu_screen_reads_every_controller(struct widget_instance const *widget)
{
	long tag;

	while (widget && widget->parent)
		widget = widget->parent;
	if (!widget || (tag = widget->definition_tag_index) == NONE)
		return FALSE;
	/* (looked up each time: the menus' tags are made again with each map) */
	return tag == tag_loaded('DeLa', LOBBY_NAME) ||
		tag == tag_loaded('DeLa', "pc\\main_menu\\multiplayer_type_select\\lobby\\preview_screen") ||
		tag == join4_tag();
}

/* (ui_widget.c) whether the widgets of the screen take a controller's
events by their own controller alone: ADD PLAYER's quarters, each a
controller's (the screen itself takes them all, to pass to its quarters) */
unsigned char pc_menu_screen_routes_by_controller(struct widget_instance const *widget)
{
	struct widget_instance const *screen = widget;

	while (screen && screen->parent)
		screen = screen->parent;
	return screen && screen->definition_tag_index != NONE && screen->definition_tag_index == join4_tag();
}

/* ---- the gametype editor: Edit Gametypes' list (the built-in gametypes
and those saved), the gametype being edited (player_ui's) or Server Setup's
(a copy of the game's gametype), its options' screens: each option a
spinner, found by its name (the PC version's screens put them in another
order than the Xbox's, whose functions of the same names walk them by
place) */

boolean ui_widget_port_gametype_edit_begin(long profile_index);
boolean ui_widget_port_gametype_save(struct widget_instance *widget, boolean *widget_deleted);
boolean ui_widget_port_game_variant_set(struct game_variant *variant, struct game_variant_options const *options);
boolean ui_widget_port_gametype_delete(long profile_index);

#define GAMETYPE_EDIT_ROWS 11

static struct
{
	/* the list's */
	long gametypes[MAXIMUM_GAMETYPES];
	short count, first, chosen;
	boolean stale;
	long deleting;
	/* Server Setup's copy of the game's gametype, edited in place of
	player_ui's (setup) */
	boolean setup;
	struct game_variant setup_variant;
	struct game_variant_options setup_options;
	/* the vehicle screen's side shown */
	short vehicle_side;
} gametype_edit = { { 0 }, 0, 0, 0, TRUE, NONE, FALSE };

static struct game_variant *edit_variant(void)
{
	return gametype_edit.setup ? &gametype_edit.setup_variant : player_ui_get_edit_playlist_profile();
}

static struct game_variant_options *edit_options(void)
{
	return gametype_edit.setup ? &gametype_edit.setup_options : player_ui_get_edit_playlist_options();
}

enum
{
	_option_long,		/* a long of the variant */
	_option_byte,		/* a byte of the variant */
	_option_flag,		/* a bit of the variant's flags (argument), set by value 1 */
	_option_health,		/* the variant's health, tenths */
	_option_short,		/* a short of the options */
	_option_option_byte,	/* a byte of the options */
	_option_radar,		/* the options' radar players, and the variant's flag */
	_option_setting,	/* the host's own (config.toml): Server Setup's copy only */
	_option_byte_value	/* a byte of the variant, its number (not on or off) */
};

struct gametype_option
{
	char const *spinner;
	short kind;
	short offset;
	unsigned long argument;
	short count;
	long values[16];
	/* (_option_setting: the setting, and its values' text) */
	char const *setting;
	char const *setting_values[16];
};

#define VARIANT_FIELD(field) (short)offsetof(struct game_variant, field)
#define OPTIONS_FIELD(field) (short)offsetof(struct game_variant_options, field)
#define TIME_LIMITS { 0, 10, 15, 20, 25, 30, 45 }

static struct gametype_option const gametype_options[] =
{
	/* OPENCE OPTIONS (OpenCE's gametypes: game_modes.c) */
	{ "oce_mode_spinner", _option_option_byte, OPTIONS_FIELD(mode), 0, 4,
		{ _game_mode_none, _game_mode_swat, _game_mode_infection, _game_mode_gun_game } },
	{ "oce_hardcore_spinner", _option_option_byte, OPTIONS_FIELD(hardcore), 0, 2, { FALSE, TRUE } },
	{ "inf_zombies_spinner", _option_option_byte, OPTIONS_FIELD(infection_initial_zombies), 0, 6, { 1, 2, 3, 4, 8, 16 } },
	{ "inf_speed_spinner", _option_option_byte, OPTIONS_FIELD(infection_zombie_speed), 0, 6,
		{ 100, 110, 125, 150, 175, 200 } },
	{ "inf_damage_spinner", _option_option_byte, OPTIONS_FIELD(infection_skull_damage), 0, 4, { 100, 150, 200, 250 } },
	{ "inf_respawn_spinner", _option_option_byte, OPTIONS_FIELD(infection_respawn_time), 0, 6, { 1, 3, 5, 10, 15, 30 } },
	{ "inf_countdown_spinner", _option_option_byte, OPTIONS_FIELD(infection_countdown), 0, 5, { 0, 5, 10, 15, 30 } },
	{ "inf_shields_spinner", _option_option_byte, OPTIONS_FIELD(infection_zombie_shields), 0, 2, { FALSE, TRUE } },
	{ "gg_order_spinner", _option_option_byte, OPTIONS_FIELD(gun_game_order), 0, 3,
		{ _gun_game_order_strongest_first, _gun_game_order_weakest_first, _gun_game_order_random } },
	{ "gg_melee_spinner", _option_option_byte, OPTIONS_FIELD(gun_game_melee_demotes), 0, 2, { FALSE, TRUE } },
	/* player options */
	{ "number_of_lives_spinner", _option_long, VARIANT_FIELD(universal_variant.lives), 0, 4, { 0, 1, 3, 5 } },
	{ "maximum_health_spinner", _option_health, 0, 0, 6, { 5, 10, 15, 20, 30, 40 } },
	{ "shields_spinner", _option_flag, 0, FLAG(_game_variant_no_shields_bit), 2, { 0, 1 } },
	{ "respawn_time_spinner", _option_long, VARIANT_FIELD(universal_variant.respawn_time), 0, 4, { 0, 150, 300, 450 } },
	{ "respawn_time_growth_spinner", _option_long, VARIANT_FIELD(universal_variant.respawn_time_growth), 0, 4,
		{ 0, 150, 300, 450 } },
	{ "odd_man_out_spinner", _option_byte, VARIANT_FIELD(universal_variant.odd_man_out), 0, 2, { 1, 0 } },
	{ "invisible_players_spinner", _option_flag, 0, FLAG(_game_variant_always_invisible_bit), 2, { 1, 0 } },
	{ "suicide_penalty_spinner", _option_long, VARIANT_FIELD(universal_variant.suicide_penalty), 0, 4,
		{ 0, 150, 300, 450 } },
	/* item options (weapon sets: the PC's list, then the Xbox's NO GRENADES) */
	{ "item_options_infinite_grenades_spinner", _option_flag, 0, FLAG(_game_variant_infinite_grenades_bit), 2,
		{ 1, 0 } },
	{ "item_options_weapon_set_spinner", _option_long, VARIANT_FIELD(universal_variant.weapon_set), 0, 14,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 10 } },
	{ "item_options_starting_equipment_spinner", _option_flag, 0, FLAG(_game_variant_generic_starting_equipment_bit),
		2, { 0, 1 } },
	{ "map_weapons_spinner", _option_option_byte, OPTIONS_FIELD(no_map_weapons), 0, 2, { 0, 1 } },
	/* (the loadout: the weapon set's, or each player's two weapons) */
	{ "loadout_spinner", _option_option_byte, OPTIONS_FIELD(loadout), 0, 2, { _loadout_category, _loadout_custom } },
	{ "primary_weapon_spinner", _option_option_byte, OPTIONS_FIELD(primary_weapon), 0, NUMBER_OF_LOADOUT_WEAPONS,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
	{ "secondary_weapon_spinner", _option_option_byte, OPTIONS_FIELD(secondary_weapon), 0, NUMBER_OF_LOADOUT_WEAPONS,
		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
	/* indicator options */
	{ "indicator_options_radar display_spinner", _option_long, VARIANT_FIELD(universal_variant.goal_radar), 0, 3,
		{ 0, 1, 2 } },
	{ "indicator_options_players_on_radar_spinner", _option_radar, OPTIONS_FIELD(radar_players), 0, 3,
		{ _radar_players_all, _radar_players_friends, _radar_players_none } },
	{ "indicator_options_friends_on_screen_spinner", _option_flag, 0, FLAG(_game_variant_allow_friendly_navpoints_bit),
		2, { 1, 0 } },
	/* capture the flag */
	{ "assault_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.ctf.assault), 0, 2, { 1, 0 } },
	{ "single_flag_spinner", _option_long, VARIANT_FIELD(game_engine_variant.ctf.single_flag_time), 0, 6,
		{ 0, 1800, 3600, 5400, 9000, 18000 } },
	{ "flag_must_reset_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.ctf.flag_must_reset), 0, 2,
		{ 1, 0 } },
	{ "flag_at_home_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.ctf.flag_at_home_to_score), 0, 2,
		{ 1, 0 } },
	{ "captures_to_win_spinner", _option_long, VARIANT_FIELD(universal_variant.score_to_win), 0, 5,
		{ 1, 3, 5, 10, 15 } },
	/* (port: kills score too, and a capture is worth its points: game_engine_ctf.c) */
	{ "kills_score_spinner", _option_long, VARIANT_FIELD(game_engine_variant.ctf.kills_score), 0, 2, { 1, 0 } },
	{ "kills_per_capture_spinner", _option_long, VARIANT_FIELD(game_engine_variant.ctf.points_per_capture), 0, 10,
		{ 5, 10, 15, 20, 25, 30, 40, 50, 75, 100 } },
	{ "time_limit_spinner", _option_short, OPTIONS_FIELD(time_limit), 0, 7, TIME_LIMITS },
	/* king of the hill */
	{ "koth_moving_hill_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.king.moving_hill), 0, 2, { 1, 0 } },
	/* (port: Multi-Hill's hills at once, game_engine_king.c) */
	{ "koth_multi_hill_spinner", _option_byte_value, VARIANT_FIELD(game_engine_variant.king.multi_hill), 0, 5,
		{ 0, 2, 3, 4, 5 } },
	{ "koth_score_to_win_spinner", _option_long, VARIANT_FIELD(universal_variant.score_to_win), 0, 5,
		{ 1, 2, 5, 10, 15 } },
	{ "koth_team_play_spinner", _option_byte, VARIANT_FIELD(universal_variant.teams), 0, 2, { 1, 0 } },
	/* oddball */
	{ "trait_with_ball_spinner", _option_long, VARIANT_FIELD(game_engine_variant.oddball.trait_with_ball), 0, 4,
		{ 0, 1, 2, 3 } },
	{ "trait_without_ball_spinner", _option_long, VARIANT_FIELD(game_engine_variant.oddball.trait_without_ball), 0, 4,
		{ 0, 1, 2, 3 } },
	{ "speed_with_ball_spinner", _option_long, VARIANT_FIELD(game_engine_variant.oddball.speed_with_ball), 0, 3,
		{ 1, 0, 2 } },
	{ "ball_type_spinner", _option_long, VARIANT_FIELD(game_engine_variant.oddball.oddball_ball_type), 0, 3,
		{ 0, 1, 2 } },
	{ "random_start_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.oddball.random_start), 0, 2, { 1, 0 } },
	{ "ball_spawn_count_spinner", _option_long, VARIANT_FIELD(game_engine_variant.oddball.ball_spawn_count), 0, 16,
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } },
	{ "score_to_win_spinner", _option_long, VARIANT_FIELD(universal_variant.score_to_win), 0, 5, { 1, 2, 5, 10, 15 } },
	/* (Slayer's, Oddball's and Race's) */
	{ "team_play_spinner", _option_byte, VARIANT_FIELD(universal_variant.teams), 0, 2, { 1, 0 } },
	/* race */
	{ "team_scoring_spinner", _option_long, VARIANT_FIELD(game_engine_variant.race.team_scoring), 0, 3, { 0, 1, 2 } },
	{ "race_type_spinner", _option_long, VARIANT_FIELD(game_engine_variant.race.race_type), 0, 3, { 0, 1, 2 } },
	{ "laps_to_win_spinner", _option_long, VARIANT_FIELD(universal_variant.score_to_win), 0, 6,
		{ 1, 3, 5, 10, 15, 25 } },
	/* slayer (the PC's rows' names and labels are crossed: kill_penalty's
	row is labelled KILL IN ORDER, kill_in_order's KILL PENALTY; each sets
	what its label says) */
	{ "death_bonus_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.slayer.no_death_bonus), 0, 2, { 0, 1 } },
	{ "kill_penalty_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.slayer.kill_in_order), 0, 2, { 1, 0 } },
	{ "kill_in_order_spinner", _option_byte, VARIANT_FIELD(game_engine_variant.slayer.no_kill_penalty), 0, 2,
		{ 0, 1 } },
	/* (up to 500, for big games: tools/port_settings.py's STRING_INSERTS) */
	{ "kills_to_win_spinner", _option_long, VARIANT_FIELD(universal_variant.score_to_win), 0, 11,
		{ 5, 10, 15, 25, 50, 75, 100, 150, 200, 250, 500 } },
	/* team options */
	{ "friendly_fire_spinner", _option_short, OPTIONS_FIELD(friendly_fire), 0, 4,
		{ _friendly_fire_off, _friendly_fire_on, _friendly_fire_shields_only, _friendly_fire_explosives_only } },
	{ "friendly_fire_penalty_spinner", _option_short, OPTIONS_FIELD(friendly_fire_penalty), 0, 4, { 0, 5, 10, 15 } },
	{ "autobalance_spinner", _option_option_byte, OPTIONS_FIELD(auto_team_balance), 0, 2, { 0, 1 } },
	/* (port: spawns as CTF's, game_engine.c; the first of
	tools/port_settings.py's TEAMPLAY_ROWS, the gametype's own) */
	{ "force_team_spawns_spinner", _option_option_byte, OPTIONS_FIELD(force_team_spawns), 0, 2, { 0, 1 } },
	/* (the host's voice chat, below them: port/linux/game/network_voice.c;
	tools/port_settings.py's TEAMPLAY_ROWS) */
	{ "voice_mode_spinner", _option_setting, 0, 0, 5, { 0 }, "network.voice_mode",
		{ "off", "team_proximity", "team_enemy_proximity", "team_global", "team_global_enemy_proximity" } },
	{ "voice_lobby_spinner", _option_setting, 0, 0, 2, { 0 }, "network.voice_lobby", { "true", "false" } },
	{ "voice_kbps_spinner", _option_setting, 0, 0, 7, { 0 }, "network.voice_kbps",
		{ "8", "12", "16", "24", "32", "48", "64" } },
	{ "voice_proximity_spinner", _option_setting, 0, 0, 6, { 0 }, "network.voice_proximity",
		{ "5", "10", "15", "20", "30", "50" } },
	/* (and the players' votes to kick: port/linux/game/network_votekick.c) */
	{ "votekick_spinner", _option_setting, 0, 0, 2, { 0 }, "network.votekick", { "true", "false" } },
	/* (and the text chat: port/linux/game/network_chat.c) */
	{ "text_chat_spinner", _option_setting, 0, 0, 3, { 0 }, "network.text_chat", { "on", "links", "off" } },
	{ "aim_assist_spinner", _option_setting, 0, 0, 2, { 0 }, "network.block_mouse_aim_assist", { "false", "true" } },
	/* (the shield effects every machine draws: network_shield_effects.c) */
	{ "shield_effects_spinner", _option_setting, 0, 0, 7, { 0 }, "network.shield_effects",
		{ "stock", "glow", "light_show", "overshield", "glow_overshield", "light_show_overshield", "players" } },
	/* co-op's options (Server Setup's CO-OP OPTIONS: tools/port_settings.py's
	COOP_SETUP_SCREENS; coop_enemies.c) */
	{ "coop_friendly_fire_spinner", _option_setting, 0, 0, 4, { 0 }, "network.coop_friendly_fire",
		{ "off", "on", "shields_only", "explosives_only" } },
	{ "coop_team_wipe_spinner", _option_setting, 0, 0, 3, { 0 }, "network.coop_team_wipe",
		{ "auto", "revert", "respawn" } },
	{ "coop_extra_enemies_spinner", _option_setting, 0, 0, 3, { 0 }, "network.coop_enemies_mode",
		{ "per_player", "multiplier", "none" } },
	{ "coop_extra_pickups_spinner", _option_setting, 0, 0, 3, { 0 }, "network.coop_pickups",
		{ "per_player", "multiplier", "none" } },
	{ "coop_extra_vehicles_spinner", _option_setting, 0, 0, 3, { 0 }, "network.coop_vehicles",
		{ "per_player", "multiplier", "none" } },
	{ "coop_enemies_per_player_spinner", _option_setting, 0, 0, 5, { 0 }, "network.coop_enemies",
		{ "25", "50", "100", "150", "200" } },
	{ "coop_enemies_multiplier_spinner", _option_setting, 0, 0, 5, { 0 }, "network.coop_enemies_multiplier",
		{ "2", "4", "8", "16", "32" } },
	{ "coop_player_collisions_spinner", _option_setting, 0, 0, 2, { 0 }, "network.coop_player_collisions",
		{ "true", "false" } },
	{ "coop_ally_friendly_fire_spinner", _option_setting, 0, 0, 2, { 0 }, "network.coop_ally_friendly_fire",
		{ "true", "false" } },
	/* map sharing's (Server Setup's MAP SHARING: tools/port_settings.py's
	MAP_SHARE_SETUP; map_share.c) */
	{ "map_share_spinner", _option_setting, 0, 0, 2, { 0 }, "network.map_share", { "true", "false" } },
	{ "map_share_upload_spinner", _option_setting, 0, 0, 8, { 0 }, "network.map_share_upload",
		{ "256", "512", "1024", "2048", "4096", "8192", "16384", "20480" } },
	/* vehicle options (the side's set and counts: vehicles_update) */
	{ "vehicles_respawn_spinner", _option_short, OPTIONS_FIELD(vehicle_respawn_time), 0, 7,
		{ 0, 30, 60, 90, 120, 180, 300 } },
};

/* TEXT CHAT's (network.text_chat) OFF, which it shows while the game is
listed in the server browser, and its host cannot change: a listed game has
no text chat whatever the setting (network_chat.c), which is kept for a
private game or a LAN one */
#define TEXT_CHAT_SPINNER "text_chat_spinner"
#define TEXT_CHAT_OFF 2

static boolean text_chat_locked(struct gametype_option const *option)
{
	return !strcmp(option->spinner, TEXT_CHAT_SPINNER) && p2p_hosting_listed();
}

/* an option of the host's own setting: the value shown for it (the one
set, else the nearest) */
static short gametype_option_setting_index(struct gametype_option const *option)
{
	struct pc_menu_setting setting;
	char text[64];
	short index;

	csmemset(&setting, 0, sizeof(setting));
	setting.setting = option->setting;
	setting.value_count = option->count;
	for (index = 0; index < option->count; index++)
		setting.values[index] = option->setting_values[index];
	if (!setting_text(option->setting, text, sizeof(text), FALSE))
		return 0;
	return setting_value_index(&setting, text);
}

/* (an instance's name is its definition's cut to 31 characters:
"item_options_infinite_grenades_") */
#define WIDGET_NAME_LENGTH 31

static struct gametype_option const *gametype_option_named(char const *spinner)
{
	short index;

	for (index = 0; index < NUMBEROF(gametype_options); index++)
	{
		if (!strncmp(gametype_options[index].spinner, spinner, WIDGET_NAME_LENGTH))
			return &gametype_options[index];
	}
	return NULL;
}

static long gametype_option_value(struct gametype_option const *option, struct game_variant *variant,
	struct game_variant_options *options)
{
	byte *v = (byte *)variant;
	byte *o = (byte *)options;

	switch (option->kind)
	{
	case _option_long: return *(long *)(v + option->offset);
	case _option_byte: return v[option->offset] != 0;
	case _option_flag: return (variant->universal_variant.flags & option->argument) != 0;
	case _option_health: return (long)(variant->universal_variant.health * 10.0f + 0.5f);
	case _option_short: return *(short *)(o + option->offset);
	case _option_option_byte: return o[option->offset];
	case _option_byte_value: return v[option->offset];
	case _option_radar: return options->radar_players;
	}
	return 0;
}

static void gametype_option_value_set(struct gametype_option const *option, long value, struct game_variant *variant,
	struct game_variant_options *options)
{
	byte *v = (byte *)variant;
	byte *o = (byte *)options;

	switch (option->kind)
	{
	case _option_long: *(long *)(v + option->offset) = value; break;
	case _option_byte: v[option->offset] = (byte)value; break;
	case _option_flag:
		if (value)
			variant->universal_variant.flags |= option->argument;
		else
			variant->universal_variant.flags &= ~option->argument;
		break;
	case _option_health: variant->universal_variant.health = (real)value / 10.0f; break;
	case _option_short: *(short *)(o + option->offset) = (short)value; break;
	case _option_option_byte: o[option->offset] = (byte)value; break;
	case _option_byte_value: v[option->offset] = (byte)value; break;
	case _option_radar:
		/* (and the Xbox's flag: other players on the tracker or not) */
		options->radar_players = (byte)value;
		if (value == _radar_players_none)
			variant->universal_variant.flags &= ~FLAG(_game_variant_draw_object_in_motion_sensor_bit);
		else
			variant->universal_variant.flags |= FLAG(_game_variant_draw_object_in_motion_sensor_bit);
		break;
	}
}

/* the value's place in the option's list: its own, else the nearest */
static short gametype_option_index(struct gametype_option const *option, long value)
{
	short index, best = 0;
	long best_distance = 0x7FFFFFFF;

	for (index = 0; index < option->count; index++)
	{
		long distance = option->values[index] > value ? option->values[index] - value : value - option->values[index];

		if (distance < best_distance)
		{
			best_distance = distance;
			best = index;
		}
	}
	return best;
}

/* each of the screen's option rows' spinner, with the option */
static void gametype_options_each(struct widget_instance *list, boolean save)
{
	struct game_variant *variant = edit_variant();
	struct game_variant_options *options = edit_options();
	struct widget_instance *row;

	if (!variant || !options)
		return;
	for (row = list->child; row; row = row->next)
	{
		struct widget_instance *spinner;

		for (spinner = row->child; spinner; spinner = spinner->next)
		{
			/* (a spinner: its label's name, cut to 31 characters, can be
			the same as its own) */
			struct gametype_option const *option = spinner->type == 2 ? gametype_option_named(spinner->name) : NULL;

			if (!option)
				continue;
			/* (the host's own settings: Server Setup's alone, their rows
			hidden in Edit Gametypes', which edits a gametype for any game;
			written on OK, as the gametype's are kept) */
			if (option->kind == _option_setting)
			{
				short index = (short)PIN(spinner->parameters.list.selected_index, 0, option->count - 1);

				visible_set(row, gametype_edit.setup);
				if (!gametype_edit.setup)
					continue;
				if (!save)
				{
					spinner->parameters.list.selected_index = text_chat_locked(option) ? TEXT_CHAT_OFF :
						gametype_option_setting_index(option);
				}
				else if (!text_chat_locked(option) && index != gametype_option_setting_index(option) &&
					!setting_write(option->setting, option->setting_values[index]))
				{
					platform_log("menus: could not set %s", option->setting);
				}
				continue;
			}
			if (save)
			{
				short index = (short)PIN(spinner->parameters.list.selected_index, 0, option->count - 1);

				gametype_option_value_set(option, option->values[index], variant, options);
			}
			else
				spinner->parameters.list.selected_index =
					gametype_option_index(option, gametype_option_value(option, variant, options));
		}
	}
}

/* each of a screen's rows of the host's own settings (_option_setting:
co-op's options' screens, tools/port_settings.py's _setup_option_screen),
shown, or (save) set */
static void setting_options_each(struct widget_instance *list, boolean save)
{
	struct widget_instance *row;

	for (row = list->child; row; row = row->next)
	{
		struct widget_instance *spinner;

		for (spinner = row->child; spinner; spinner = spinner->next)
		{
			struct gametype_option const *option = spinner->type == 2 ? gametype_option_named(spinner->name) : NULL;
			short index;

			if (!option || option->kind != _option_setting)
				continue;
			index = (short)PIN(spinner->parameters.list.selected_index, 0, option->count - 1);
			if (!save)
			{
				spinner->parameters.list.selected_index = text_chat_locked(option) ? TEXT_CHAT_OFF :
					gametype_option_setting_index(option);
			}
			else if (!text_chat_locked(option) && index != gametype_option_setting_index(option) &&
				!setting_write(option->setting, option->setting_values[index]))
			{
				platform_log("menus: could not set %s", option->setting);
			}
		}
	}
}

/* the options' list of a button of its bar (OK's): the one above it whose
rows are its options */
static struct widget_instance *gametype_options_list(struct widget_instance *widget)
{
	struct widget_instance *list;

	for (list = widget; list; list = list->parent)
	{
		struct widget_instance *child;

		for (child = list->child; child; child = child->next)
		{
			if (!strncmp(child->name, "op_", 3))
				return list;
		}
	}
	return NULL;
}

/* ---- the vehicle options: a side's (red's, blue's) set and counts, on
the spinners while its side is shown */

static char const *const vehicle_spinners[NUMBER_OF_VARIANT_VEHICLES] =
{
	"warthog_spinner", "ghost_spinner", "scorpion_spinner", "rwarthog_spinner", "banshee_spinner", "cgturret_spinner"
};
/* (the presets' spinner: the vehicle sets 0 to 7, then PC and CUSTOM) */
#define VEHICLE_PRESET_PC 8
#define VEHICLE_PRESET_CUSTOM 9

static struct widget_instance *vehicle_spinner(struct widget_instance *list, char const *name)
{
	struct widget_instance *row;

	for (row = list->child; row; row = row->next)
	{
		struct widget_instance *spinner = named(row, name, 0);

		if (spinner)
			return spinner;
	}
	return NULL;
}

static void vehicles_show(struct widget_instance *list)
{
	struct game_variant_options *options = edit_options();
	struct widget_instance *spinner;
	short side = gametype_edit.vehicle_side, index;

	if (!options)
		return;
	if ((spinner = vehicle_spinner(list, "team_spinner")) != NULL)
		spinner->parameters.list.selected_index = side;
	if ((spinner = vehicle_spinner(list, "vehicle_presets_spinner")) != NULL)
		spinner->parameters.list.selected_index = options->vehicle_set[side] == VARIANT_VEHICLE_SET_CUSTOM ?
			VEHICLE_PRESET_CUSTOM : options->vehicle_set[side] == VARIANT_VEHICLE_SET_PC ?
			VEHICLE_PRESET_PC : (short)MIN(options->vehicle_set[side], VEHICLE_PRESET_PC - 1);
	for (index = 0; index < NUMBER_OF_VARIANT_VEHICLES; index++)
	{
		if ((spinner = vehicle_spinner(list, vehicle_spinners[index])) != NULL)
			spinner->parameters.list.selected_index =
				(short)MIN(options->vehicle_counts[side][index], MAXIMUM_VARIANT_VEHICLE_COUNT);
	}
}

/* the side shown's spinners kept (and the Xbox's vehicle set: red's, if it
is one of the Xbox's) */
static void vehicles_keep(struct widget_instance *list)
{
	struct game_variant *variant = edit_variant();
	struct game_variant_options *options = edit_options();
	struct widget_instance *spinner;
	short side = gametype_edit.vehicle_side, index;

	if (!options || !variant)
		return;
	if ((spinner = vehicle_spinner(list, "vehicle_presets_spinner")) != NULL)
		options->vehicle_set[side] = spinner->parameters.list.selected_index >= VEHICLE_PRESET_CUSTOM ?
			VARIANT_VEHICLE_SET_CUSTOM : spinner->parameters.list.selected_index == VEHICLE_PRESET_PC ?
			VARIANT_VEHICLE_SET_PC : (byte)spinner->parameters.list.selected_index;
	for (index = 0; index < NUMBER_OF_VARIANT_VEHICLES; index++)
	{
		if ((spinner = vehicle_spinner(list, vehicle_spinners[index])) != NULL)
			options->vehicle_counts[side][index] =
				(byte)PIN(spinner->parameters.list.selected_index, 0, MAXIMUM_VARIANT_VEHICLE_COUNT);
	}
	variant->universal_variant.vehicle_set = options->vehicle_set[0] <= 4 ? options->vehicle_set[0] : 0;
}

/* "mp prof vehicles update": the side changed (the one left kept, the other
shown); a count changed makes the side's set CUSTOM */
static void vehicles_update(struct widget_instance *list)
{
	struct game_variant_options *options = edit_options();
	struct widget_instance *team = vehicle_spinner(list, "team_spinner");
	struct widget_instance *preset = vehicle_spinner(list, "vehicle_presets_spinner");
	short index;

	if (!options)
		return;
	if (team && team->parameters.list.selected_index != gametype_edit.vehicle_side)
	{
		vehicles_keep(list);
		gametype_edit.vehicle_side = (short)PIN(team->parameters.list.selected_index, 0, 1);
		vehicles_show(list);
		return;
	}
	for (index = 0; preset && index < NUMBER_OF_VARIANT_VEHICLES; index++)
	{
		struct widget_instance *spinner = vehicle_spinner(list, vehicle_spinners[index]);

		if (spinner && spinner->parameters.list.selected_index !=
			options->vehicle_counts[gametype_edit.vehicle_side][index])
		{
			options->vehicle_counts[gametype_edit.vehicle_side][index] = (byte)spinner->parameters.list.selected_index;
			preset->parameters.list.selected_index = VEHICLE_PRESET_CUSTOM;
		}
	}
}

/* "mp profile init X" (the list's creation): its spinners from the gametype */
static boolean gametype_options_init(struct widget_instance *list)
{
	if (!edit_variant())
		return campaign_fail();
	gametype_options_each(list, FALSE);
	if (named(list, "op_team", 0))
		vehicles_show(list);
	return TRUE;
}

/* "mp profile set X" (OK): the gametype from its spinners */
static boolean gametype_options_save(struct widget_instance *widget)
{
	struct widget_instance *list = gametype_options_list(widget);

	if (!list || !edit_variant())
		return campaign_fail();
	if (named(list, "op_team", 0))
		vehicles_keep(list);
	gametype_options_each(list, TRUE);
	/* (OPENCE OPTIONS: Infection is played on teams, Gun Game free for all:
	game_modes_variant_mode) */
	if (named(list, "oce_mode_spinner", 0) && edit_variant()->game_engine_index == game_engine_slayer)
	{
		if (edit_options()->mode == _game_mode_infection)
			edit_variant()->universal_variant.teams = TRUE;
		else if (edit_options()->mode == _game_mode_gun_game)
			edit_variant()->universal_variant.teams = FALSE;
	}
	return TRUE;
}

/* ---- Server Setup's options: a copy of the game's gametype (the one chosen
before), edited by the editor's screens, the game's on START GAME */

static void gametype_setup_begin(void)
{
	if (gametype_edit.setup)
		return;
	if (!player_ui_game_variant_specified(&gametype_edit.setup_variant))
		return;
	gametype_edit.setup_options = *player_ui_get_game_variant_options();
	gametype_edit.setup = TRUE;
	gametype_edit.vehicle_side = 0;
}

static void gametype_setup_end(void)
{
	gametype_edit.setup = FALSE;
}

static boolean gametype_setup_apply(void)
{
	boolean applied = !gametype_edit.setup ||
		ui_widget_port_game_variant_set(&gametype_edit.setup_variant, &gametype_edit.setup_options);

	gametype_edit.setup = FALSE;
	return applied;
}

/* the game type row's: the gametype's name and type */
static void gametype_setup_type(wchar_t *text)
{
	wchar_t name[NUMBEROF(gametype_edit.setup_variant.human_readable_game_description) + 1];
	wchar_t const *kind;

	text[0] = 0;
	if (!gametype_edit.setup)
		return;
	ustrncpy(name, gametype_edit.setup_variant.human_readable_game_description, NUMBEROF(name) - 1);
	name[NUMBEROF(name) - 1] = 0;
	kind = game_modes_variant_kind_name(&gametype_edit.setup_variant, &gametype_edit.setup_options);
	usnprintf(text, ROW_TEXT_LENGTH - 1, L"%s (%s%s)", name,
		kind ? kind : engine_names[PIN(gametype_edit.setup_variant.game_engine_index, 0, 5)],
		gametype_edit.setup_variant.universal_variant.teams ? L", TEAMS" : L"");
	text[ROW_TEXT_LENGTH - 1] = 0;
}

/* "port setup edit" (a Server Setup option's row): its screen edits the copy */
static boolean gametype_setup_edit(void)
{
	gametype_setup_begin();
	return gametype_edit.setup || campaign_fail();
}

/* ---- Edit Gametypes' list */

static void gametype_edit_read(void)
{
	short last;

	gametype_edit.count = ui_widget_port_gametypes(gametype_edit.gametypes, MAXIMUM_GAMETYPES, &last);
	gametype_edit.chosen = (short)MIN(gametype_edit.chosen, MAX(0, gametype_edit.count - 1));
	gametype_edit.stale = FALSE;
}

/* "mp profiles list initialize" (Edit Gametypes'): on the gametype used last */
static boolean gametype_edit_list_initialize(struct widget_instance *list)
{
	short last;

	gametype_edit.setup = FALSE;
	gametype_edit.count = ui_widget_port_gametypes(gametype_edit.gametypes, MAXIMUM_GAMETYPES, &last);
	gametype_edit.chosen = last;
	gametype_edit.first = (short)MAX(0, MIN(last - GAMETYPE_EDIT_ROWS / 2, gametype_edit.count - GAMETYPE_EDIT_ROWS));
	gametype_edit.stale = FALSE;
	focus_row(list, (short)(gametype_edit.chosen - gametype_edit.first));
	return TRUE;
}

static void gametype_edit_row_text(short row, wchar_t *text)
{
	gametype_display_name(gametype_edit.gametypes[gametype_edit.first + row], text);
}

/* "gt edit list update": the rows (scrolled), the gametype chosen's
picture, name and rules */
static void gametype_edit_list_update(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *widget;
	short focused;

	if (gametype_edit.stale)
		gametype_edit_read();
	focused = list_scroll(list, &gametype_edit.first, gametype_edit.count, GAMETYPE_EDIT_ROWS);
	if (focused != NONE)
		gametype_edit.chosen = (short)MAX(0, MIN(focused, gametype_edit.count - 1));
	rows_update(list, (short)MIN(gametype_edit.count, GAMETYPE_EDIT_ROWS), gametype_edit_row_text);
	visible_set(named(description, "gametype_right_item", 0), gametype_edit.count > 0);
	if (gametype_edit.chosen < gametype_edit.count)
	{
		long profile_index = gametype_edit.gametypes[gametype_edit.chosen];
		struct game_variant variant;
		wchar_t text[ROW_TEXT_LENGTH * 2];

		visible_set(named(description, "locked_gametype_icon", 0),
			((unsigned long)profile_index & PLAYLIST_READ_ONLY_BIT) != 0);
		gametype_display_name(profile_index, text);
		text_set(named(description, "gametype_right_name", 0), text);
		if (playlist_profile_get(profile_index, &variant))
		{
			long engine = PIN(variant.game_engine_index, 0, 5);

			if ((widget = named(description, "gametype_right_pic", 0)) != NULL)
				widget->animation.current_frame_index = (short)engine;
			gametype_rules_text(profile_index, text, NUMBEROF(text));
			text_set(named(description, "gametype_right_data", 0), text);
		}
	}
	profile_name_show(description);
}

/* "port playlist edit desc" (after the PC version's "playlist settings menu
update desc"): the gametype editor's OPENCE OPTIONS, its eighth row, has its
own words (the list's ninth) over GAME OPTIONS' picture, and the buttons
after it SAVE CHANGES' */
#define PLAYLIST_EDIT_OPENCE_ROW 7
#define PLAYLIST_EDIT_OPENCE_WORDS 8
#define PLAYLIST_EDIT_GAME_OPTIONS_PICTURE 1

static void playlist_edit_description(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	short index = list->focused_child ? ui_widget_port_list_index(list) : NONE;

	if (!description || !description->child || !description->child->next || index < PLAYLIST_EDIT_OPENCE_ROW)
		return;
	description->child->parameters.text_box.string_list_index =
		index == PLAYLIST_EDIT_OPENCE_ROW ? PLAYLIST_EDIT_OPENCE_WORDS : (short)(index - 1);
	description->child->next->animation.current_frame_index =
		index == PLAYLIST_EDIT_OPENCE_ROW ? PLAYLIST_EDIT_GAME_OPTIONS_PICTURE : (short)(index - 1);
}

/* "mp profile begin editing" (the list's custom activation: OK, a row's A) */
static boolean gametype_edit_begin(void)
{
	if (gametype_edit.chosen >= gametype_edit.count)
		return campaign_fail();
	gametype_edit.setup = FALSE;
	gametype_edit.vehicle_side = 0;
	return ui_widget_port_gametype_edit_begin(gametype_edit.gametypes[gametype_edit.chosen]);
}

/* "request del playlist profile" (X, DELETE): not a built-in one */
static boolean gametype_delete_request(void)
{
	long profile_index = gametype_edit.chosen < gametype_edit.count ? gametype_edit.gametypes[gametype_edit.chosen] :
		NONE;

	if (profile_index == NONE || ((unsigned long)profile_index & PLAYLIST_READ_ONLY_BIT))
		return campaign_fail();
	gametype_edit.deleting = profile_index;
	return TRUE;
}

/* "final del playlist profile" */
static boolean gametype_delete_final(void)
{
	boolean deleted = gametype_edit.deleting != NONE && ui_widget_port_gametype_delete(gametype_edit.deleting);

	gametype_edit.deleting = NONE;
	gametype_edit.stale = TRUE;
	return deleted;
}

/* ---- the gametype's screen and the gametype type's */

/* "get edit game settings name": the gametype's name */
static void gametype_edit_name(struct widget_instance *text_box)
{
	struct game_variant *variant = edit_variant();
	wchar_t name[NUMBEROF(variant->human_readable_game_description) + 1];

	if (!variant)
		return;
	ustrncpy(name, variant->human_readable_game_description, NUMBEROF(name) - 1);
	name[NUMBEROF(name) - 1] = 0;
	text_set(text_box, name);
}

/* "game settings lists text update": the help of the option with the
focus, for its value (the description's strings are each option's values'
helps in turn), as the Xbox's, which stops the game on the buttons' row */
static void gametype_option_help(struct widget_instance *list)
{
	struct widget_instance *description = list->parameters.list.extended_description;
	struct widget_instance *row;
	short index = 0;

	/* (Item Options: the weapon set's row for a category loadout, the two
	weapons' for a custom one) */
	if (named(list, "loadout_spinner", 0))
	{
		boolean custom = named(list, "loadout_spinner", 0)->parameters.list.selected_index == _loadout_custom;

		visible_set(named(list, "op_weapon_set", 0), !custom);
		visible_set(named(list, "op_primary_weapon", 0), custom);
		visible_set(named(list, "op_secondary_weapon", 0), custom);
	}
	/* (OPENCE OPTIONS: the mode's own rows, Infection's or Gun Game's, below
	MODE and HARDCORE) */
	if (named(list, "oce_mode_spinner", 0))
	{
		short mode = named(list, "oce_mode_spinner", 0)->parameters.list.selected_index;
		short place = 73 + 2 * 30;

		for (row = list->child; row; row = row->next)
		{
			boolean infection = !strncmp(row->name, "op_inf_", 7);

			if (!infection && strncmp(row->name, "op_gg_", 6))
				continue;
			row->visible = infection ? mode == _game_mode_infection : mode == _game_mode_gun_game;
			if (row->visible)
			{
				row->vertical_offset = place;
				place += 30;
			}
		}
	}
	/* (TEXT CHAT held at OFF while the game is listed: text_chat_locked) */
	if (named(list, TEXT_CHAT_SPINNER, 0) && p2p_hosting_listed())
		named(list, TEXT_CHAT_SPINNER, 0)->parameters.list.selected_index = TEXT_CHAT_OFF;
	/* CO-OP OPTIONS: enemies, pickups and vehicles share the amount rows.
	Mixed modes may need both rows at once. */
	if (named(list, "coop_extra_enemies_spinner", 0))
	{
		short mode = named(list, "coop_extra_enemies_spinner", 0)->parameters.list.selected_index;

		struct widget_instance *pickups = named(list, "coop_extra_pickups_spinner", 0);
		struct widget_instance *vehicles = named(list, "coop_extra_vehicles_spinner", 0);
		boolean per_player = mode == _cooperative_enemies_per_player ||
			(pickups && pickups->parameters.list.selected_index == _cooperative_enemies_per_player) ||
			(vehicles && vehicles->parameters.list.selected_index == _cooperative_enemies_per_player);
		boolean multiplier = mode == _cooperative_enemies_multiplier ||
			(pickups && pickups->parameters.list.selected_index == _cooperative_enemies_multiplier) ||
			(vehicles && vehicles->parameters.list.selected_index == _cooperative_enemies_multiplier);

		visible_set(named(list, "op_coop_enemies_per_player", 0), per_player);
		visible_set(named(list, "op_coop_enemies_multiplier", 0), multiplier);

		/* Hidden amounts must not reserve a blank row. Offsets are relative
		to the parent: moving only the row carries its labels and spinners
		with it when widget_instance_render_recursive draws and notes targets. */
		{
			short y = 73;

			for (row = list->child; row; row = row->next)
				if (row->visible && !strncmp(row->name, "op_coop_", 8))
				{
					row->vertical_offset = y;
					y += 30;
				}
		}
	}
	/* (the server browser's filters, hidden: their helps are fewer than
	their values, filters_help) */
	if (!description || !list->focused_child || !strncmp(list->name, "filters", 7))
		return;
	for (row = list->child; row; row = row->next)
	{
		struct widget_instance *spinner;

		for (spinner = row->child; spinner && spinner->type != 2 /* spinner */; spinner = spinner->next)
			;
		if (row == list->focused_child)
		{
			if (spinner)
				description->parameters.text_box.string_list_index = (short)(index + spinner->parameters.list.selected_index);
			return;
		}
		if (spinner)
			index += spinner->parameters.list.number_of_items;
	}
}

static char const *const engine_items[] =
{
	/* (the type list's rows, in order: game_engine_ctf and so on) */
	"gametype_select_ctf_item", "gametype_select_koth_item", "gametype_select_slayer_item",
	"gametype_select_oddball_item", "gametype_select_race_item"
};
static long const engine_of_item[] = { 1, 4, 2, 3, 5 };

/* "mp profile init game engine": the gametype's type focused */
static boolean gametype_engine_init(struct widget_instance *list)
{
	struct game_variant *variant = edit_variant();
	short index;

	if (!variant)
		return campaign_fail();
	for (index = 0; index < NUMBEROF(engine_items); index++)
	{
		if (engine_of_item[index] == variant->game_engine_index)
			focus_row(list, index);
	}
	return TRUE;
}

/* "mp profile set game engine" (a type's A): another type clears the type's
own rules (as the Xbox's) */
static boolean gametype_engine_set(struct widget_instance *item)
{
	struct game_variant *variant = edit_variant();
	short index;

	if (!variant)
		return campaign_fail();
	for (index = 0; index < NUMBEROF(engine_items); index++)
	{
		if (!strcmp(item->name, engine_items[index]))
		{
			if (variant->game_engine_index != engine_of_item[index])
				csmemset(&variant->game_engine_variant, 0, sizeof(variant->game_engine_variant));
			variant->game_engine_index = engine_of_item[index];
			return TRUE;
		}
	}
	return campaign_fail();
}

/* "mp edit profile set rule text": the gametype's type (its string in
ui\multiplayer_game_text, as the Xbox's: capture the flag 3, slayer 4,
oddball 5, king of the hill 6, race 7, unknown 8) */
static void gametype_engine_name(struct widget_instance *text_box)
{
	static short const strings[] = { 8, 3, 4, 5, 6, 7 };
	struct game_variant *variant = edit_variant();

	if (variant)
		text_box->parameters.text_box.string_list_index = strings[PIN(variant->game_engine_index, 0, 5)];
}

/* ---------- public code */

boolean pc_menu_event_function_invoke(
	struct widget_instance *widget,
	struct event_record *event,
	long function_index,
	boolean *widget_deleted)
{
	short controller = event_controller(widget, event);

	switch (function_index)
	{
	case _pc_menu_function_quit_game:
		platform_request_quit();
		return TRUE;
	case _pc_menu_function_setting_load:
		return setting_load(widget);
	case _pc_menu_function_setting_save:
		return setting_save(widget);
	}
	{
		char const *name = pc_menu_function_name(function_index);

		if (!name)
			return FALSE;
		if (!strncmp(name, "port controller ", 16))
			return pad_editor_event(widget, name);
		if (!strcmp(name, "main menu quit game"))
		{
			platform_request_quit();
		}
		else if (!strcmp(name, "profile set edit begin"))
		{
			return profile_edit_begin(widget, event, widget_deleted);
		}
		else if (!strcmp(name, "new campaign decision"))
		{
			return new_profile_decision(widget, event, widget_deleted);
		}
		/* (the press posted is the controller's that chose the button: a
		split screen player's LEAVE is theirs) */
		else if (!strcmp(name, "mouse emit accept event"))
		{
			event_manager_post_button(controller, BUTTON_A);
		}
		else if (!strcmp(name, "mouse emit back event"))
		{
			event_manager_post_button(controller, BUTTON_B);
		}
		else if (!strcmp(name, "mouse emit x event"))
		{
			event_manager_post_button(controller, BUTTON_X);
		}
		else if (!strcmp(name, "emit custom activation event"))
		{
			struct widget_instance *top = widget;
			boolean deleted;

			while (top->parent)
				top = top->parent;
			ui_widget_port_dispatch_event(top, EVENT_CUSTOM_ACTIVATION, event_controller(widget, event), &deleted);
			if (deleted)
				*widget_deleted = TRUE;
		}
		else if (!strcmp(name, "mp type set mode"))
		{
			lan_mode = strstr(widget->name, "_lan_") != NULL;
			multiplayer_mode_set(widget);
			/* (Create's: the map list only for a game made, "join controller
			to mp game" before; it fails with the game's port in use, by
			another copy of the game) */
			if ((multiplayer.mode == _multiplayer_mode_host_internet ||
				multiplayer.mode == _multiplayer_mode_host_lan) && !global_network_game_server_get())
			{
				platform_log("menus: the game could not be made (is its port in use?)");
				return campaign_fail();
			}
		}
		else if (!strcmp(name, "gamespy screen init"))
		{
			return browser_initialize(widget, event, widget_deleted);
		}
		else if (!strcmp(name, "gamespy select item") || !strcmp(name, "gamespy select button"))
		{
			return browser_select(widget, event, controller, widget_deleted);
		}
		else if (!strcmp(name, "gamespy back handler") && text_field_editing(NULL))
		{
			text_field_end(FALSE);
		}
		/* (the server browser's panels: B, or their own buttons, close them
		over the list) */
		else if ((!strcmp(name, "gamespy back handler") || !strcmp(name, "gamespy dismiss error") ||
			!strcmp(name, "gamespy dismiss filters")) && browser_panel_close(widget))
		{
			ui_play_audio_feedback_sound(SOUND_BACK);
		}
		else if (!strcmp(name, "gamespy select header"))
		{
			return lobby_browser_sort_choose(widget);
		}
		else if (!strcmp(name, "gamespy update filter settings"))
		{
			filters_show(screen_of(widget), FALSE, TRUE);
			ui_play_audio_feedback_sound(SOUND_FORWARD);
		}
		else if (!strcmp(name, "port setup edit"))
		{
			return gametype_setup_edit();
		}
		/* (co-op's options' screens: their settings shown, and OK's) */
		else if (!strcmp(name, "port setup options init"))
		{
			setting_options_each(widget, FALSE);
		}
		else if (!strcmp(name, "port setup options save"))
		{
			struct widget_instance *list = gametype_options_list(widget);

			if (list)
				setting_options_each(list, TRUE);
		}
		else if (!strcmp(name, "mp profile save changes"))
		{
			return ui_widget_port_gametype_save(widget, widget_deleted);
		}
		else if (!strcmp(name, "mp profile begin editing"))
		{
			return gametype_edit_begin();
		}
		else if (!strcmp(name, "request del playlist profile"))
		{
			return gametype_delete_request();
		}
		else if (!strcmp(name, "final del playlist profile"))
		{
			return gametype_delete_final();
		}
		else if (!strcmp(name, "mp profile init game engine"))
		{
			return gametype_engine_init(widget);
		}
		else if (!strcmp(name, "mp profile set game engine"))
		{
			return gametype_engine_set(widget);
		}
		else if (!strncmp(name, "mp profile init ", 16) || !strcmp(name, "mp prof init teamplay options") ||
			!strcmp(name, "mp prof init vehicle options"))
		{
			return gametype_options_init(widget);
		}
		else if ((!strncmp(name, "mp profile set ", 15) && strcmp(name, "mp profile set for game")) ||
			!strcmp(name, "mp prof save teamplay options") || !strcmp(name, "mp prof save vehicle options"))
		{
			return gametype_options_save(widget);
		}
		else if (!strcmp(name, "port lobby preview join"))
		{
			return preview_join(widget, controller, widget_deleted);
		}
		else if (!strcmp(name, "gamespy screen dispose"))
		{
			lobby_browser_end();
		}
		else if (!strcmp(name, "port pause end game"))
		{
			return pause_end_game();
		}
		else if (!strcmp(name, "port coop friendly fire"))
		{
			return coop_friendly_fire_change();
		}
		else if (!strcmp(name, "player profile save changes"))
		{
			return profile_save_changes(widget, widget_deleted);
		}
		else if (!strcmp(name, "port profile settings save"))
		{
			return profile_settings_save(widget);
		}
		else if (!strcmp(name, "direct ip connect go"))
		{
			return direct_link_from_clipboard();
		}
		else if (!strcmp(name, "join controller to mp game"))
		{
			return multiplayer_host(widget, event, controller, widget_deleted);
		}
		else if (!strcmp(name, "mp level list initialize"))
		{
			return map_list_initialize(widget);
		}
		else if (!strcmp(name, "mp level select"))
		{
			return map_list_choose(widget, widget_deleted);
		}
		else if (!strcmp(name, "port map list back"))
		{
			return map_list_back(widget, widget_deleted);
		}
		else if (!strcmp(name, "mp profiles list initialize"))
		{
			if (!strcmp(widget->name, "playlist_select_list"))
				return gametype_edit_list_initialize(widget);
			return gametype_list_initialize(widget);
		}
		else if (!strcmp(name, "mp profile set for game"))
		{
			return gametype_choose(widget);
		}
		else if (!strcmp(name, "server settings init"))
		{
			return server_settings_initialize(widget);
		}
		else if (!strcmp(name, "ss edit server name"))
		{
			return server_name_edit(widget);
		}
		else if (!strcmp(name, "ss edit server password"))
		{
			return server_password_edit(widget);
		}
		else if (!strcmp(name, "port password init"))
		{
			return password_screen_initialize(widget);
		}
		else if (!strcmp(name, "port password edit"))
		{
			return password_screen_edit(widget, widget_deleted);
		}
		else if (!strcmp(name, "port password join"))
		{
			return password_screen_join(widget, widget_deleted);
		}
		else if (!strcmp(name, "port password back"))
		{
			return password_screen_back(widget, widget_deleted);
		}
		else if (!strcmp(name, "ss copy invite"))
		{
			return invite_copy();
		}
		else if (!strcmp(name, "ss start game"))
		{
			return server_start();
		}
		else if (!strcmp(name, "single prev cl item activated"))
		{
			item_activated(widget, event_controller(widget, event), widget_deleted);
		}
		else if (!strcmp(name, "port settings save"))
		{
			settings_each(screen_of(widget), setting_changed_save);
			platform_display_apply();
		}
		else if (!strcmp(name, "port settings defaults"))
		{
			settings_each(screen_of(widget), setting_default_show);
		}
		else if (!strcmp(name, "port mods init") || !strcmp(name, "port mods close"))
		{
			return mods_screen_reset();
		}
		else if (!strcmp(name, "port mods toggle"))
		{
			return mods_toggle(widget);
		}
		else if (!strcmp(name, "port mods move up") || !strcmp(name, "port mods move down"))
		{
			return mods_move(widget, (short)(!strcmp(name, "port mods move up") ? -1 : 1));
		}
		else if (!strcmp(name, "port mods save"))
		{
			return mods_save();
		}
		else if (!strcmp(name, "controls screen init") || !strcmp(name, "controls screen defaults"))
		{
			return controls_load(!strcmp(name, "controls screen defaults"));
		}
		else if (!strcmp(name, "controls screen change set"))
		{
			return controls_save();
		}
		else if (!strcmp(name, "controls begin binding"))
		{
			return control_capture_begin(widget);
		}
		else if (!strcmp(name, "controls back handler"))
		{
			return controls_back(widget, widget_deleted);
		}
		else if (!strcmp(name, "player profile list initialize"))
		{
			return profile_list_initialize(widget);
		}
		else if (!strcmp(name, "profile manager select"))
		{
			return profile_choose(controller);
		}
		else if (!strcmp(name, "port lobby open"))
		{
			return lobby_join_reset();
		}
		else if (!strcmp(name, "port map vote open"))
		{
			return map_vote_open();
		}
		else if (!strcmp(name, "port map vote choose"))
		{
			return map_vote_choose(widget);
		}
		else if (!strcmp(name, "port lobby add player"))
		{
			return lobby_add_player(controller);
		}
		else if (!strcmp(name, "port join4 open"))
		{
			return join4_open(widget);
		}
		else if (!strcmp(name, "port join4 playing") || !strcmp(name, "port join4 opener"))
		{
			/* (join4_open sends their events only to the quarters to set) */
			return TRUE;
		}
		else if (!strcmp(name, "port join4 select"))
		{
			return join4_select(widget, event, widget_deleted);
		}
		else if (!strcmp(name, "port join4 join"))
		{
			return join4_join(controller_of(widget));
		}
		else if (!strcmp(name, "port join4 choosing"))
		{
			/* (the spinner on a profile no other player has) */
			extern long ui_widget_port_spinner_first_available(struct widget_instance *widget,
				boolean (*usable)(short controller, long profile));
			long profile = ui_widget_port_spinner_first_available(widget, join4_profile_free);

			if (controller_of(widget) < MAXIMUM_LOCAL_PLAYERS)
				join4.starting[controller_of(widget)] = profile;
			return join4_set(controller_of(widget), _join4_choosing);
		}
		else if (!strcmp(name, "port join4 unready"))
		{
			return join4_set(controller_of(widget), _join4_choosing);
		}
		else if (!strcmp(name, "port join4 leave"))
		{
			return join4_set(controller_of(widget), _join4_away);
		}
		else if (!strcmp(name, "port join4 ready"))
		{
			return join4_set(controller_of(widget), _join4_selected);
		}
		else if (!strcmp(name, "port join4 back"))
		{
			return join4_back(event_controller(widget, event));
		}
		else if (!strcmp(name, "port join4 done"))
		{
			return join4_done(controller_of(widget));
		}
		else if (!strcmp(name, "port lobby join"))
		{
			return lobby_join_start(widget, controller, widget_deleted);
		}
		else if (!strcmp(name, "port lobby leave"))
		{
			return lobby_leave(widget, event, controller, widget_deleted);
		}
		else if (!strcmp(name, "port lobby player list initialize"))
		{
			return lobby_player_list_initialize(widget);
		}
		else if (!strcmp(name, "port lobby player choose"))
		{
			return lobby_player_choose();
		}
		else if (!strcmp(name, "port lobby preview add"))
		{
			return preview_add(widget, controller, widget_deleted);
		}
		else if (!strcmp(name, "port lobby preview leave"))
		{
			return preview_leave(controller);
		}
		else if (!strcmp(name, "port coop begin"))
		{
			return coop_begin(controller);
		}
		else if (!strcmp(name, "port coop player 2 list initialize"))
		{
			return coop_player2_list_initialize(widget);
		}
		else if (!strcmp(name, "port coop player 2"))
		{
			return coop_player2_choose(controller);
		}
		else if (!strcmp(name, "request del player profile"))
		{
			return profile_delete_request();
		}
		else if (!strcmp(name, "final del player profile"))
		{
			return profile_delete();
		}
		else if (!strcmp(name, "color picker menu initialize"))
		{
			return color_list_initialize(widget);
		}
		else if (!strcmp(name, "color picker select color"))
		{
			return color_choose();
		}
		else if (!strcmp(name, "controls binding slot"))
		{
			controls_screen.slot = (short)!controls_screen.slot;
		}
		else if (!strcmp(name, "campaign menu init"))
		{
			return campaign_menu_initialize(controller);
		}
		else if (!strcmp(name, "campaign menu continue"))
		{
			return campaign_continue(controller);
		}
		else if (!strcmp(name, "initialize sp level list solo"))
		{
			return level_list_initialize(widget, controller);
		}
		else if (!strcmp(name, "solo level set map"))
		{
			return level_choose(widget, controller);
		}
		else if (!strcmp(name, "difficulty item select"))
		{
			return difficulty_start(sibling_index(widget), controller);
		}
		else if (!strcmp(name, "set difficulty"))
		{
			main_set_difficulty(PIN(difficulty_shown(widget), 0, 3));
		}
		else if (!strcmp(name, "load game menu init"))
		{
			return saved_game_list_initialize(widget, controller);
		}
		else if (!strcmp(name, "load game menu activated"))
		{
			return saved_game_continue(controller);
		}
		else if (!strcmp(name, "load game menu delete request"))
		{
			return saved_game_delete_request();
		}
		else if (!strcmp(name, "load game menu delete finish"))
		{
			return saved_game_delete(controller);
		}
		else if (!strcmp(name, "gamespy back handler") || !strcmp(name, "gamespy dismiss error") ||
			!strcmp(name, "gamespy dismiss filters"))
		{
			ui_widget_port_go_back(widget);
			*widget_deleted = TRUE;
		}
		return TRUE;
	}
}

void pc_menu_game_data_function_invoke(
	struct widget_instance *widget,
	long function)
{
	char const *name = pc_menu_game_data_input_name(function);

	if (!name)
		return;
	/* (a text field whose screen has gone: let go of) */
	if (text_field.row && system_milliseconds() - text_field_shown_time > 500)
		text_field_end(FALSE);
	if (!strcmp(name, "solo map list update"))
		level_list_update(widget);
	else if (!strcmp(name, "mp map list update"))
		map_list_update(widget);
	else if (!strcmp(name, "gt select list update"))
		gametype_list_update(widget);
	else if (!strcmp(name, "server settings update"))
		server_settings_update(widget);
	else if (!strcmp(name, "port password update"))
		password_screen_update(widget);
	else if (!strcmp(name, "gamespy screen update"))
		browser_update(widget);
	else if (!strcmp(name, "gt edit list update"))
		gametype_edit_list_update(widget);
	else if (!strcmp(name, "get edit game settings name"))
		gametype_edit_name(widget);
	else if (!strcmp(name, "game settings lists text update"))
	{
		gametype_option_help(widget);
		filters_help(widget);
	}
	else if (!strcmp(name, "mp edit profile set rule text"))
		gametype_engine_name(widget);
	else if (!strcmp(name, "mp prof vehicles update"))
		vehicles_update(widget);
	else if (!strcmp(name, "port lobby preview update"))
		preview_update(widget);
	else if (!strcmp(name, "port lobby update"))
		lobby_update(widget);
	else if (!strcmp(name, "port map vote update"))
		map_vote_update(widget);
	else if (!strcmp(name, "port join4 update"))
		join4_update(widget);
	else if (!strcmp(name, "port settings help"))
	{
		video_rows_show(widget);
		hud_button_style_row_show(widget);
		master_servers_show(widget);
		discord_account_show(widget);
		hdr_calibration_row_show(widget);
		settings_help(widget);
	}
	else if (!strcmp(name, "port hdr calibration"))
		hdr_calibration_update(widget);
	else if (!strcmp(name, "controls update menu"))
		controls_update(widget);
	else if (!strcmp(name, "port controller update"))
		pad_editor_update(widget);
	else if (!strcmp(name, "port mods update"))
		mods_update(widget);
	else if (!strcmp(name, "color picker update"))
		color_list_update(widget);
	else if (!strcmp(name, "3wide player profile list update"))
		profile_list_update(widget);
	else if (!strcmp(name, "load game list update"))
		saved_game_list_update(widget);
	else if (!strcmp(name, "port gamepad layout preview"))
		profile_gamepad_layout(widget);
	else if (!strcmp(name, "port coop friendly fire text"))
		coop_friendly_fire_text(widget);
	else if (!strcmp(name, "port playlist edit desc"))
		playlist_edit_description(widget);
}

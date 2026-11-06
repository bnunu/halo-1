/*
SERVER_ROTATION.C

A dedicated server's rotation and gametype rules (server_rotation.h).
*/

#include "server_rotation.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SERVER_ROTATION_NO_FILES
#include "platform.h"
#include "port_config.h"
#include "posix.h"
#endif

/* ---------- the rules */

static const char *const boolean_words[] = { "false", "true", NULL };
static const char *const weapon_set_words[] =
{
	/* (game_engine.c's enum game_engine_weapons) */
	"normal", "pistols", "rifles", "plasma", "sniper", "no_sniping", "rocket_launchers", "shotguns", "short_range",
	"human", "no_grenades", "covenant", "classic", "heavy", NULL,
};
static const char *const vehicle_set_words[] =
{
	/* (its enum game_engine_vehicles) */
	"default", "none", "warthog", "ghost", "scorpion", NULL,
};
static const char *const goal_radar_words[] = { "motion_tracker", "nav_point", "none", NULL };
static const char *const friendly_fire_words[] = { "on", "off", "shields_only", "explosives_only", NULL };
static const char *const radar_players_words[] = { "all", "friends", "none", NULL };
static const char *const loadout_words[] = { "category", "custom", NULL };
static const char *const loadout_weapon_words[] =
{
	"none", "random", "assault_rifle", "pistol", "shotgun", "sniper_rifle", "rocket_launcher", "plasma_pistol",
	"plasma_rifle", "needler", NULL,
};
static const char *const team_vehicles_words[] = { "gametype", "pc", "custom", NULL };

#define ALL _server_engine_all
#define RULE_INTEGER(key, alias, table, engines, minimum, maximum, description) \
	{ key, alias, table, engines, _server_rule_integer, minimum, maximum, NULL, description }
#define RULE_BOOLEAN(key, alias, table, engines, description) \
	{ key, alias, table, engines, _server_rule_boolean, 0, 1, boolean_words, description }
#define RULE_CHOICE(key, alias, table, engines, words, description) \
	{ key, alias, table, engines, _server_rule_choice, 0, 0, words, description }

const struct server_rule_definition server_rule_definitions[] =
{
	RULE_INTEGER("score_to_win", "score", "universal", ALL, 0, 999,
		"The score that wins (kills, captures, laps, seconds of the ball or the hill...); 0 for no limit"),
	RULE_INTEGER("lives", NULL, "universal", ALL, 0, 99, "Lives each player has; 0 for no limit"),
	RULE_INTEGER("respawn_time", "respawn", "universal", ALL, 0, 300, "Seconds before a dead player respawns"),
	RULE_INTEGER("respawn_time_growth", NULL, "universal", ALL, 0, 300, "Seconds added to the respawn at each death"),
	RULE_INTEGER("suicide_penalty", NULL, "universal", ALL, 0, 300, "Seconds added to a suicide's respawn"),
	RULE_INTEGER("health", NULL, "universal", ALL, 25, 400, "Players' health, in percent"),
	RULE_BOOLEAN("teams", NULL, "universal", ALL, "Teams (CTF always has them)"),
	RULE_BOOLEAN("odd_man_out", NULL, "universal", ALL, "The player in the lead is shown to everyone"),
	RULE_BOOLEAN("radar", NULL, "universal", ALL, "Players on the motion tracker"),
	RULE_BOOLEAN("friend_indicators", NULL, "universal", ALL, "Teammates' markers over their heads"),
	RULE_BOOLEAN("infinite_grenades", NULL, "universal", ALL, "Grenades never run out"),
	RULE_BOOLEAN("no_shields", NULL, "universal", ALL, "Players have no shields"),
	RULE_BOOLEAN("invisible", NULL, "universal", ALL, "Players are always camouflaged"),
	RULE_BOOLEAN("generic_equipment", NULL, "universal", ALL, "Every map starts players with the same weapons"),
	RULE_CHOICE("weapon_set", "weapons", "universal", ALL, weapon_set_words, "The weapons on the map"),
	RULE_CHOICE("vehicle_set", "vehicles", "universal", ALL, vehicle_set_words, "The vehicles on the map"),
	RULE_CHOICE("goal_radar", NULL, "universal", ALL, goal_radar_words,
		"How the flag, the ball or the hill is shown"),

	RULE_INTEGER("time_limit", "time", "options", ALL, 0, 1440, "The game's minutes; 0 for no limit"),
	RULE_CHOICE("friendly_fire", "ff", "options", ALL, friendly_fire_words, "Damage to teammates"),
	RULE_INTEGER("friendly_fire_penalty", NULL, "options", ALL, 0, 300,
		"Seconds added to a team killer's respawn"),
	RULE_INTEGER("vehicle_respawn_time", NULL, "options", ALL, 0, 300,
		"Seconds before a destroyed vehicle respawns; 0 for never"),
	RULE_BOOLEAN("auto_team_balance", "balance", "options", ALL, "Joining players go to the smaller team"),
	RULE_CHOICE("radar_players", NULL, "options", ALL, radar_players_words,
		"Which players the motion tracker shows"),
	RULE_CHOICE("loadout", NULL, "options", ALL, loadout_words,
		"The starting weapons: the weapon set's (category), or primary and secondary (custom)"),
	RULE_CHOICE("primary_weapon", NULL, "options", ALL, loadout_weapon_words, "A custom loadout's first weapon"),
	RULE_CHOICE("secondary_weapon", NULL, "options", ALL, loadout_weapon_words,
		"A custom loadout's second weapon"),
	RULE_BOOLEAN("no_map_weapons", NULL, "options", ALL, "No weapons lie on the map"),
	RULE_CHOICE("team_vehicles", NULL, "options", ALL, team_vehicles_words,
		"Each team's vehicles: the vehicle set's (gametype), every one the map places (pc), or the counts below (custom)"),
	RULE_INTEGER("warthogs", NULL, "options", ALL, 0, 4, "Warthogs of each team, with team_vehicles custom"),
	RULE_INTEGER("ghosts", NULL, "options", ALL, 0, 4, "Ghosts of each team, with team_vehicles custom"),
	RULE_INTEGER("scorpions", NULL, "options", ALL, 0, 4, "Scorpions of each team, with team_vehicles custom"),
	RULE_INTEGER("rocket_warthogs", NULL, "options", ALL, 0, 4,
		"Rocket Warthogs of each team, with team_vehicles custom"),
	RULE_INTEGER("banshees", NULL, "options", ALL, 0, 4, "Banshees of each team, with team_vehicles custom"),
	RULE_INTEGER("gun_turrets", NULL, "options", ALL, 0, 4, "Gun turrets of each team, with team_vehicles custom"),
	RULE_BOOLEAN("hardcore", NULL, "options", ALL,
		"Hardcore: no motion sensor, no aim assist, a kill restores the killer's health and shields"),

	RULE_BOOLEAN("assault", NULL, "ctf", _server_engine_ctf, "One team defends the flag, the other attacks"),
	RULE_BOOLEAN("reset_on_capture", NULL, "ctf", _server_engine_ctf, "Both flags go home at a capture"),
	RULE_BOOLEAN("flag_must_reset", NULL, "ctf", _server_engine_ctf, "A dropped flag goes home only by its timer"),
	RULE_BOOLEAN("flag_at_home_to_score", NULL, "ctf", _server_engine_ctf,
		"A team's own flag must be home to score"),
	RULE_INTEGER("single_flag_time", NULL, "ctf", _server_engine_ctf, 0, 600,
		"Seconds of each side's turn in a single flag game"),
	RULE_BOOLEAN("no_death_bonus", NULL, "slayer", _server_engine_slayer, "No point for a kill after a death"),
	RULE_BOOLEAN("no_kill_penalty", NULL, "slayer", _server_engine_slayer, "No point taken for a suicide"),
	RULE_BOOLEAN("kill_in_order", NULL, "slayer", _server_engine_slayer, "Players must be killed in order"),
	RULE_BOOLEAN("moving_hill", NULL, "king", _server_engine_king, "The hill moves"),
	RULE_BOOLEAN("random_start", NULL, "oddball", _server_engine_oddball, "The ball starts at a random place"),
	RULE_BOOLEAN("ball_spawn_delay", NULL, "oddball", _server_engine_oddball, "The ball respawns after a delay"),
	RULE_INTEGER("speed_with_ball", NULL, "oddball", _server_engine_oddball, 0, 3, "The ball carrier's speed"),
	RULE_INTEGER("trait_with_ball", NULL, "oddball", _server_engine_oddball, 0, 3, "The ball carrier's trait"),
	RULE_INTEGER("trait_without_ball", NULL, "oddball", _server_engine_oddball, 0, 3, "The others' trait"),
	RULE_INTEGER("ball_type", NULL, "oddball", _server_engine_oddball, 0, 2, "The kind of ball game"),
	RULE_INTEGER("ball_spawn_count", NULL, "oddball", _server_engine_oddball, 1, 16, "Balls on the map"),
	RULE_INTEGER("race_type", NULL, "race", _server_engine_race, 0, 2, "The kind of race"),
	RULE_INTEGER("team_scoring", NULL, "race", _server_engine_race, 0, 2, "How a team's laps are counted"),
};

int server_rule_definition_count(void)
{
	return (int)(sizeof(server_rule_definitions) / sizeof(server_rule_definitions[0]));
}

static int text_equal_case(const char *a, const char *b)
{
	for (; *a && *b; a++, b++)
	{
		if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
			return 0;
	}
	return !*a && !*b;
}

const struct server_rule_definition *server_rule_find(const char *key)
{
	int index;

	for (index = 0; index < server_rule_definition_count(); index++)
	{
		const struct server_rule_definition *rule = &server_rule_definitions[index];

		if (text_equal_case(key, rule->key) || (rule->alias && text_equal_case(key, rule->alias)))
			return rule;
	}
	return NULL;
}

/* a whole number of text (no sign but a leading -), into value: 1 if it is */
static int whole_number(const char *text, long *value)
{
	char *end;
	long number;

	if (!text[0] || strlen(text) > 9)
		return 0;
	if (!(isdigit((unsigned char)text[0]) || (text[0] == '-' && isdigit((unsigned char)text[1]))))
		return 0;
	number = strtol(text, &end, 10);
	if (*end)
		return 0;
	*value = number;
	return 1;
}

int server_rule_value(const struct server_rule_definition *rule, const char *text, long *value, char *reason,
	int reason_size)
{
	long number;
	int index;

	switch (rule->type)
	{
	case _server_rule_integer:
		if (!whole_number(text, &number))
		{
			snprintf(reason, (size_t)reason_size, "%s takes a whole number, not \"%s\"", rule->key, text);
			return 0;
		}
		if (number < rule->minimum || number > rule->maximum)
		{
			snprintf(reason, (size_t)reason_size, "%s takes %ld to %ld, not %ld", rule->key, rule->minimum,
				rule->maximum, number);
			return 0;
		}
		*value = number;
		return 1;
	case _server_rule_boolean:
		if (text_equal_case(text, "true") || text_equal_case(text, "yes") || text_equal_case(text, "on") ||
			!strcmp(text, "1"))
		{
			*value = 1;
			return 1;
		}
		if (text_equal_case(text, "false") || text_equal_case(text, "no") || text_equal_case(text, "off") ||
			!strcmp(text, "0"))
		{
			*value = 0;
			return 1;
		}
		snprintf(reason, (size_t)reason_size, "%s takes true or false, not \"%s\"", rule->key, text);
		return 0;
	case _server_rule_choice:
		for (index = 0; rule->choices[index]; index++)
		{
			if (text_equal_case(text, rule->choices[index]))
			{
				*value = index;
				return 1;
			}
		}
		if (whole_number(text, &number) && number >= 0 && number < index)
		{
			*value = number;
			return 1;
		}
		{
			int length = snprintf(reason, (size_t)reason_size, "%s takes ", rule->key);

			for (index = 0; rule->choices[index] && length < reason_size; index++)
			{
				length += snprintf(reason + length, (size_t)(reason_size - length), "%s%s",
					index ? (rule->choices[index + 1] ? ", " : " or ") : "", rule->choices[index]);
			}
			if (length < reason_size)
				snprintf(reason + length, (size_t)(reason_size - length), ", not \"%s\"", text);
		}
		return 0;
	}
	return 0;
}

/* ---------- the rotation's text */

static int is_space(char character)
{
	return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

/* a name of letters, digits, "_", "-" and "." (no path) */
static int plain_name(const char *text)
{
	int index;

	for (index = 0; text[index]; index++)
	{
		unsigned char character = (unsigned char)text[index];

		if (!(isalnum(character) || character == '_' || character == '-' || character == '.'))
			return 0;
	}
	return index > 0 && text[0] != '.';
}

/* a level's path: letters, digits, "_", "-", "." and backslashes, no part
of it only dots (nothing leaves the maps folder) */
static int level_path(const char *text)
{
	const char *part = text;
	int index;

	if (!text[0] || text[0] == '\\')
		return 0;
	for (index = 0;; index++)
	{
		unsigned char character = (unsigned char)text[index];

		if (character == '\\' || !character)
		{
			int length = (int)(text + index - part);
			int dots = 0, at;

			for (at = 0; at < length; at++)
				dots += part[at] == '.';
			if (!length || dots == length)
				return 0;
			if (!character)
				return 1;
			part = text + index + 1;
			continue;
		}
		if (!(isalnum(character) || character == '_' || character == '-' || character == '.'))
			return 0;
	}
}

int server_rotation_parse_entry(const char *text, int length, struct server_entry *entry)
{
	char copy[SERVER_ENTRY_TEXT_SIZE];
	char *colon, *rule;
	int start = 0, end = length, index;

	memset(entry, 0, sizeof(*entry));
	while (start < end && is_space(text[start]))
		start++;
	while (end > start && is_space(text[end - 1]))
		end--;
	if (start >= end)
		return 0;
	/* (a comment) */
	if (text[start] == '#')
		return 0;
	if (end - start >= (int)sizeof(copy))
	{
		snprintf(entry->text, sizeof(entry->text), "%.*s...", (int)sizeof(entry->text) - 4, text + start);
		snprintf(entry->rejected, sizeof(entry->rejected), "longer than %d characters", (int)sizeof(copy) - 1);
		return 1;
	}
	memcpy(copy, text + start, (size_t)(end - start));
	copy[end - start] = 0;
	snprintf(entry->text, sizeof(entry->text), "%s", copy);
	for (index = 0; copy[index]; index++)
	{
		if ((unsigned char)copy[index] < 0x20 || (unsigned char)copy[index] >= 0x7F)
		{
			snprintf(entry->rejected, sizeof(entry->rejected), "has a character that is not printable ASCII");
			return 1;
		}
	}
	/* map[:gametype[+rule...]] (a map's path has no colon) */
	colon = strchr(copy, ':');
	rule = NULL;
	if (colon)
	{
		*colon = 0;
		rule = strchr(colon + 1, '+');
		if (rule)
			*rule++ = 0;
	}
	/* the map */
	{
		char *map = copy;
		size_t map_length;
		char prefix[12] = "";

		while (is_space(*map))
			map++;
		map_length = strlen(map);
		while (map_length && is_space(map[map_length - 1]))
			map[--map_length] = 0;
		/* (forward slashes as the game's backslashes) */
		for (index = 0; map[index]; index++)
		{
			if (map[index] == '/')
				map[index] = '\\';
		}
		if (map_length >= sizeof(entry->map))
		{
			snprintf(entry->rejected, sizeof(entry->rejected), "the map's name is too long");
			return 1;
		}
		snprintf(entry->map, sizeof(entry->map), "%s", map);
		/* (custom_maps\ in any case) */
		if (map_length >= 12 && map[11] == '\\')
			snprintf(prefix, sizeof(prefix), "%.11s", map);
		if (text_equal_case(prefix, "custom_maps"))
		{
			entry->map_kind = _server_map_custom;
			memcpy(entry->map, "custom_maps", 11);
			if (!plain_name(map + 12))
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "\"%s\" is not a map's name", map + 12);
				return 1;
			}
			snprintf(entry->map_name, sizeof(entry->map_name), "%s", map + 12);
			snprintf(entry->level, sizeof(entry->level), "custom_maps\\%s", map + 12);
		}
		else if (strchr(map, '\\'))
		{
			const char *last = strrchr(map, '\\') + 1;

			entry->map_kind = _server_map_path;
			if (!level_path(map))
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "\"%s\" is not a level's path", map);
				return 1;
			}
			snprintf(entry->map_name, sizeof(entry->map_name), "%s", last);
			snprintf(entry->level, sizeof(entry->level), "%s", map);
		}
		else
		{
			entry->map_kind = _server_map_name;
			if (!plain_name(map))
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "\"%s\" is not a map's name", map);
				return 1;
			}
			snprintf(entry->map_name, sizeof(entry->map_name), "%s", map);
			if (snprintf(entry->level, sizeof(entry->level), "levels\\test\\%s\\%s", map, map) >=
				(int)sizeof(entry->level))
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "the map's name is too long");
				return 1;
			}
		}
		for (index = 0; entry->map_name[index]; index++)
			entry->map_name[index] = (char)tolower((unsigned char)entry->map_name[index]);
		if (!strcmp(entry->map_name, "ui"))
		{
			snprintf(entry->rejected, sizeof(entry->rejected), "ui is the main menu's map");
			return 1;
		}
	}
	/* the gametype */
	snprintf(entry->gametype, sizeof(entry->gametype), "slayer");
	if (colon)
	{
		char *gametype = colon + 1;
		size_t gametype_length;

		while (is_space(*gametype))
			gametype++;
		gametype_length = strlen(gametype);
		while (gametype_length && is_space(gametype[gametype_length - 1]))
			gametype[--gametype_length] = 0;
		if (!gametype_length || gametype_length >= sizeof(entry->gametype) ||
			!plain_name(!strncmp(gametype, "file=", 5) ? gametype + 5 : gametype))
		{
			snprintf(entry->rejected, sizeof(entry->rejected), "\"%s\" is not a gametype's name", gametype);
			return 1;
		}
		for (index = 0; gametype[index]; index++)
			gametype[index] = (char)tolower((unsigned char)gametype[index]);
		snprintf(entry->gametype, sizeof(entry->gametype), "%s", gametype);
	}
	/* the rules */
	while (rule)
	{
		char *next = strchr(rule, '+');
		char *equals;
		const struct server_rule_definition *definition;
		long value;
		int existing;

		if (next)
			*next++ = 0;
		equals = strchr(rule, '=');
		while (is_space(*rule))
			rule++;
		if (!equals)
		{
			snprintf(entry->rejected, sizeof(entry->rejected), "the rule \"%s\" has no value (key=value)", rule);
			return 1;
		}
		*equals = 0;
		{
			char *key_end = equals;
			char *value_text = equals + 1;
			size_t value_length;

			while (key_end > rule && is_space(key_end[-1]))
				*--key_end = 0;
			while (is_space(*value_text))
				value_text++;
			value_length = strlen(value_text);
			while (value_length && is_space(value_text[value_length - 1]))
				value_text[--value_length] = 0;
			definition = server_rule_find(rule);
			if (!definition)
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "no rule is named \"%s\"", rule);
				return 1;
			}
			if (!server_rule_value(definition, value_text, &value, entry->rejected, sizeof(entry->rejected)))
				return 1;
		}
		/* (a rule given twice: the last) */
		for (existing = 0; existing < entry->rule_count && strcmp(entry->rules[existing].key, definition->key);
			existing++)
		{
		}
		if (existing == entry->rule_count)
		{
			if (entry->rule_count >= SERVER_RULES_MAXIMUM)
			{
				snprintf(entry->rejected, sizeof(entry->rejected), "more than %d rules", SERVER_RULES_MAXIMUM);
				return 1;
			}
			entry->rule_count++;
		}
		snprintf(entry->rules[existing].key, sizeof(entry->rules[existing].key), "%s", definition->key);
		entry->rules[existing].value = value;
		rule = next;
	}
	return 1;
}

void server_rotation_parse(const char *text, struct server_rotation *rotation)
{
	int length = (int)strlen(text);
	int start = 0;

	memset(rotation, 0, sizeof(*rotation));
	if (length > SERVER_ROTATION_MAXIMUM_TEXT)
	{
		/* (cut at the separator before the cut: the entry it crosses is not
		played cut short) */
		length = SERVER_ROTATION_MAXIMUM_TEXT;
		while (length > 0 && text[length] != ',' && text[length] != '\n' && text[length] != ';')
			length--;
		rotation->truncated = 1;
	}
	while (start < length)
	{
		int end = start;

		while (end < length && is_space(text[end]) && text[end] != '\n')
			end++;
		/* (a comment runs to its line's end, commas and all) */
		if (end < length && text[end] == '#')
		{
			while (end < length && text[end] != '\n')
				end++;
			start = end + 1;
			continue;
		}
		while (end < length && text[end] != ',' && text[end] != '\n' && text[end] != ';')
			end++;
		if (rotation->count >= SERVER_ROTATION_MAXIMUM_ENTRIES)
		{
			struct server_entry ignored;

			if (server_rotation_parse_entry(text + start, end - start, &ignored))
				rotation->truncated = 1;
		}
		else if (server_rotation_parse_entry(text + start, end - start, &rotation->entries[rotation->count]))
		{
			rotation->count++;
		}
		start = end + 1;
	}
}

int server_rotation_playable(struct server_rotation const *rotation)
{
	int count = 0, index;

	for (index = 0; index < rotation->count; index++)
		count += !rotation->entries[index].rejected[0];
	return count;
}

void server_rotation_text(struct server_rotation const *rotation, char *text, int size)
{
	int length = 0, index;

	if (size <= 0)
		return;
	text[0] = 0;
	for (index = 0; index < rotation->count && length < size; index++)
		length += snprintf(text + length, (size_t)(size - length), "%s%s", index ? ", " : "", rotation->entries[index].text);
}

/* ---------- the maps on disk */

#ifndef SERVER_ROTATION_NO_FILES

int server_map_header(const char *path, long *version, long *scenario_type)
{
	unsigned char header[0x800];
	FILE *file = fopen(path, "rb");
	size_t read;

	*version = 0;
	*scenario_type = -1;
	if (!file)
		return 0;
	read = fread(header, 1, sizeof(header), file);
	fclose(file);
	/* ('head' and 'foot', little-endian; Halo PC's and Custom Edition's
	headers end where the Xbox's does) */
	if (read != sizeof(header) || memcmp(header, "daeh", 4) || memcmp(header + 0x7FC, "toof", 4))
		return 0;
	*version = (long)(header[4] | header[5] << 8 | header[6] << 16 | (unsigned long)header[7] << 24);
	*scenario_type = (long)(short)(header[0x60] | header[0x61] << 8);
	return 1;
}

/* the map file of a name in a folder (d:\maps, an absolute folder): the
first of its spellings found, into path */
static int map_file(const char *folder, const char *name, char *path, int size)
{
	char xbox_path[512];
	char file_name[300];
	char on_disk[300];
	long version, type;

	if (folder[0] == 'd' && folder[1] == ':')
	{
		snprintf(xbox_path, sizeof(xbox_path), "%s\\%s.map", folder, name);
		platform_translate_path(xbox_path, path, (unsigned long)size);
	}
	else
	{
		snprintf(path, (size_t)size, "%s/%s.map", folder, name);
		/* (a file named in other letters' case, cmt_Snow_Grove.map: the maps'
		list names each in lower case, and the game finds it in any case, as
		platform_translate_path does) */
		snprintf(file_name, sizeof(file_name), "%s.map", name);
		if (!server_map_header(path, &version, &type) &&
			posix_find_entry_case_insensitive(folder, file_name, on_disk, sizeof(on_disk)))
		{
			snprintf(path, (size_t)size, "%s/%s", folder, on_disk);
		}
	}
	return server_map_header(path, &version, &type);
}

/* a campaign level plays only as co-op, a multiplayer map never so, and
the main menu's not at all (the scenario's type in its header); the maps
folder's an Xbox cache file, custom_maps' a Custom Edition one (version) */
static void server_rotation_check_kind(struct server_entry *entry, long type, long version)
{
	int coop = !strcmp(entry->gametype, "coop");
	long wanted = entry->map_kind == _server_map_custom ? 609 : 5;

	if (version != wanted)
		snprintf(entry->rejected, sizeof(entry->rejected), "%s is not %s cache file (version %ld, not %ld)",
			entry->map_name, wanted == 5 ? "an Xbox" : "a Custom Edition", version, wanted);
	else if (type == 2)
		snprintf(entry->rejected, sizeof(entry->rejected), "%s is the main menu's map", entry->map_name);
	else if (type == 0 && !coop)
		snprintf(entry->rejected, sizeof(entry->rejected), "%s is a campaign level (as co-op: %s:coop)",
			entry->map_name, entry->map);
	else if (type != 0 && coop)
		snprintf(entry->rejected, sizeof(entry->rejected), "%s is not a campaign level, for co-op", entry->map_name);
}

/* the custom_maps folder, as the game finds it */
static void custom_maps_folder(char *folder, int size)
{
	const char *setting = config_string("paths.custom_maps");

	if (setting[0])
		snprintf(folder, (size_t)size, "%s", setting);
	else
		platform_translate_path("d:\\custom_maps", folder, (unsigned long)size);
}

void server_rotation_resolve(struct server_entry *entry)
{
	char path[1024];
	char folder[1024];
	long version, type;

	if (entry->rejected[0])
		return;
	custom_maps_folder(folder, sizeof(folder));
	/* a name: the Xbox's map of it, else Custom Edition's */
	if (entry->map_kind == _server_map_name)
	{
		if (map_file("d:\\maps", entry->map_name, path, sizeof(path)))
		{
			server_map_header(path, &version, &type);
			server_rotation_check_kind(entry, type, version);
			return;
		}
		if (map_file(folder, entry->map_name, path, sizeof(path)) ||
			(platform_custom_edition_root()[0] &&
				(snprintf(folder, sizeof(folder), "%s/maps", platform_custom_edition_root()),
				map_file(folder, entry->map_name, path, sizeof(path)))))
		{
			entry->map_kind = _server_map_custom;
			snprintf(entry->level, sizeof(entry->level), "custom_maps\\%s", entry->map_name);
			server_map_header(path, &version, &type);
			server_rotation_check_kind(entry, type, version);
			return;
		}
		snprintf(entry->rejected, sizeof(entry->rejected), "there is no map %s (maps/%s.map, custom_maps/%s.map)",
			entry->map_name, entry->map_name, entry->map_name);
		entry->map_missing = 1;
		return;
	}
	if (entry->map_kind == _server_map_custom)
	{
		if (map_file(folder, entry->map_name, path, sizeof(path)) ||
			(platform_custom_edition_root()[0] &&
				(snprintf(folder, sizeof(folder), "%s/maps", platform_custom_edition_root()),
				map_file(folder, entry->map_name, path, sizeof(path)))))
		{
			server_map_header(path, &version, &type);
			server_rotation_check_kind(entry, type, version);
			return;
		}
		snprintf(entry->rejected, sizeof(entry->rejected), "there is no Custom Edition map %s (custom_maps/%s.map)",
			entry->map_name, entry->map_name);
		entry->map_missing = 1;
		return;
	}
	/* a path: the maps folder's file of its last part */
	if (!map_file("d:\\maps", entry->map_name, path, sizeof(path)))
	{
		snprintf(entry->rejected, sizeof(entry->rejected), "there is no map file maps/%s.map for %s", entry->map_name,
			entry->level);
		entry->map_missing = 1;
		return;
	}
	server_map_header(path, &version, &type);
	server_rotation_check_kind(entry, type, version);
}

/* the multiplayer maps of a folder (an absolute one), each told; how many */
static int maps_in_folder(const char *folder, int custom,
	void (*each)(void *context, const char *name, const char *level, const char *kind), void *context)
{
	void *directory = posix_directory_open(folder);
	char name[256];
	int count = 0;

	if (!directory)
		return 0;
	while (posix_directory_next(directory, name, sizeof(name)))
	{
		size_t length = strlen(name);
		char path[1400];
		char map[SERVER_MAP_SIZE];
		char level[SERVER_MAP_SIZE + 32];
		long version, type;
		size_t index;

		if (length < 5 || length - 4 >= sizeof(map) || strcmp(name + length - 4, ".map"))
			continue;
		snprintf(map, sizeof(map), "%.*s", (int)(length - 4), name);
		for (index = 0; map[index]; index++)
			map[index] = (char)tolower((unsigned char)map[index]);
		/* (the resource maps, and the main menu's) */
		if (!strcmp(map, "bitmaps") || !strcmp(map, "sounds") || !strcmp(map, "loc") || !strcmp(map, "ui"))
			continue;
		snprintf(path, sizeof(path), "%s/%s", folder, name);
		if (!server_map_header(path, &version, &type) || type != 1 || (custom ? version != 609 : version != 5))
			continue;
		if (custom)
			snprintf(level, sizeof(level), "custom_maps\\%s", map);
		else
			snprintf(level, sizeof(level), "levels\\test\\%s\\%s", map, map);
		each(context, map, level, custom ? "custom" : "xbox");
		count++;
	}
	posix_directory_close(directory);
	return count;
}

int server_maps_each(void (*each)(void *context, const char *name, const char *level, const char *kind),
	void *context)
{
	char folder[1024];
	int count;

	platform_translate_path("d:\\maps", folder, sizeof(folder));
	count = maps_in_folder(folder, 0, each, context);
	custom_maps_folder(folder, sizeof(folder));
	count += maps_in_folder(folder, 1, each, context);
	return count;
}

#endif

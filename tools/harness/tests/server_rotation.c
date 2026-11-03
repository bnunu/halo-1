/*
SERVER_ROTATION.C (test)

A dedicated server's rotation (port/linux/src/server_rotation.c), taken
whole: the entries' grammar (map, gametype, +rule=value), the rules' table
and values, the rotation's separators and limits, its text written back,
and the maps found on disk, with small cache file headers this test writes
in a folder beside it. The platform's paths and folders are stubs over that
folder. test_server_rotation.py takes the code (under_test.inc).
*/

#include "harness.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* the folder that stands for the data root (d:\), and the settings read */
static char data_root[512];
static char custom_maps_setting[512];
static char custom_edition_root[512];

/* d:\<path> as data_root/<path>, with forward slashes */
void platform_translate_path(const char *xbox_path, char *host_path, unsigned long host_path_size)
{
	int length = snprintf(host_path, (size_t)host_path_size, "%s/%s", data_root,
		!strncmp(xbox_path, "d:\\", 3) ? xbox_path + 3 : xbox_path);
	int index;

	for (index = 0; index < length && host_path[index]; index++)
	{
		if (host_path[index] == '\\')
			host_path[index] = '/';
	}
}

const char *config_string(const char *name)
{
	return !strcmp(name, "paths.custom_maps") ? custom_maps_setting : "";
}

const char *platform_custom_edition_root(void)
{
	return custom_edition_root;
}

void *posix_directory_open(const char *path)
{
	return opendir(path);
}

int posix_directory_next(void *directory, char *name, unsigned long name_size)
{
	struct dirent *entry;

	while ((entry = readdir(directory)))
	{
		if (entry->d_name[0] == '.')
			continue;
		snprintf(name, (size_t)name_size, "%s", entry->d_name);
		return 1;
	}
	return 0;
}

void posix_directory_close(void *directory)
{
	closedir(directory);
}

/* the entry of a folder named as name is, in any letters' case */
int posix_find_entry_case_insensitive(const char *directory, const char *name, char *result,
	unsigned long result_size)
{
	DIR *handle = opendir(directory);
	struct dirent *found;
	int matched = 0;

	if (!handle)
		return 0;
	while (!matched && (found = readdir(handle)))
	{
		if (!strcasecmp(found->d_name, name) && strlen(found->d_name) < result_size)
		{
			snprintf(result, (size_t)result_size, "%s", found->d_name);
			matched = 1;
		}
	}
	closedir(handle);
	return matched;
}

#include "under_test.inc"

static struct server_entry entry;
static struct server_rotation rotation;
static struct server_rotation again;

static int parse(char const *text)
{
	return server_rotation_parse_entry(text, (int)strlen(text), &entry);
}

static int rejected_with(char const *text, char const *reason)
{
	return parse(text) && entry.rejected[0] && strstr(entry.rejected, reason);
}

static int playable(char const *text)
{
	return parse(text) && !entry.rejected[0];
}

static long rule_of(char const *key)
{
	int index;

	for (index = 0; index < entry.rule_count; index++)
	{
		if (!strcmp(entry.rules[index].key, key))
			return entry.rules[index].value;
	}
	return NONE;
}

static int value_of(char const *key, char const *text, long *value, char *reason)
{
	const struct server_rule_definition *rule = server_rule_find(key);

	*value = NONE;
	reason[0] = 0;
	return rule && server_rule_value(rule, text, value, reason, SERVER_REASON_SIZE);
}

/* a text of count times the character */
static char const *repeated(char character, int count)
{
	static char text[8192];

	memset(text, character, (size_t)count);
	text[count] = 0;
	return text;
}

/* ---------- map files */

/* a folder of the case beside the executable: the data root */
static void make_root(char const *executable, char const *case_name)
{
	char const *slash = strrchr(executable, '/');

	snprintf(data_root, sizeof(data_root), "%.*s/root-%s-%d", slash ? (int)(slash - executable) : 1,
		slash ? executable : ".", case_name, (int)getpid());
	mkdir(data_root, 0755);
}

static void make_folder(char const *relative)
{
	char path[1024];

	snprintf(path, sizeof(path), "%s/%s", data_root, relative);
	mkdir(path, 0755);
}

/* a cache file's header of size bytes: its signatures, version and
scenario type where server_map_header reads them */
static void write_map(char const *relative, long version, long type, size_t size, char const *head, char const *foot)
{
	char path[1024];
	unsigned char *bytes = calloc(1, size + 4);
	FILE *file;

	snprintf(path, sizeof(path), "%s/%s", data_root, relative);
	memcpy(bytes, head, 4);
	bytes[4] = (unsigned char)version;
	bytes[5] = (unsigned char)(version >> 8);
	bytes[6] = (unsigned char)(version >> 16);
	bytes[7] = (unsigned char)(version >> 24);
	bytes[0x60] = (unsigned char)type;
	bytes[0x61] = (unsigned char)(type >> 8);
	if (size >= 0x800)
		memcpy(bytes + 0x7FC, foot, 4);
	file = fopen(path, "wb");
	fwrite(bytes, 1, size, file);
	fclose(file);
	free(bytes);
}

static void write_maps(void)
{
	make_folder("maps");
	make_folder("custom_maps");
	write_map("maps/bloodgulch.map", 5, 1, 0x800, "daeh", "toof");
	write_map("maps/sidewinder.map", 5, 1, 0x1000, "daeh", "toof");
	write_map("maps/a10.map", 5, 0, 0x800, "daeh", "toof");
	write_map("maps/ui.map", 5, 2, 0x800, "daeh", "toof");
	write_map("maps/bitmaps.map", 5, 1, 0x800, "daeh", "toof");
	write_map("maps/broken.map", 5, 1, 0x400, "daeh", "toof");
	write_map("maps/pcmap.map", 7, 1, 0x800, "daeh", "toof");
	write_map("custom_maps/hugeass.map", 609, 1, 0x800, "daeh", "toof");
	write_map("custom_maps/c_campaign.map", 609, 0, 0x800, "daeh", "toof");
	write_map("custom_maps/xboxmap.map", 5, 1, 0x800, "daeh", "toof");
}

/* server_maps_each's maps, as "name level kind;" in the order told */
static char maps_told[2048];

static void tell_map(void *context, const char *name, const char *level, const char *kind)
{
	(*(int *)context)++;
	snprintf(maps_told + strlen(maps_told), sizeof(maps_told) - strlen(maps_told), "%s %s %s;", name, level, kind);
}

static int resolved(char const *text)
{
	parse(text);
	server_rotation_resolve(&entry);
	return !entry.rejected[0];
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	/* a map's name: the Xbox's level of it; slayer when no gametype is given */
	CASE("names")
	{
		CHECK(playable("bloodgulch"), "a map's name was rejected: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_name, "kind %d", entry.map_kind);
		CHECK(!strcmp(entry.map, "bloodgulch") && !strcmp(entry.map_name, "bloodgulch"), "map %s", entry.map);
		CHECK(!strcmp(entry.level, "levels\\test\\bloodgulch\\bloodgulch"), "level %s", entry.level);
		CHECK(!strcmp(entry.gametype, "slayer") && !entry.rule_count, "gametype %s", entry.gametype);
		CHECK(playable("BloodGulch:CTF"), "a name in capitals was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.map_name, "bloodgulch") && !strcmp(entry.gametype, "ctf"),
			"read %s:%s, not in lower case", entry.map_name, entry.gametype);
		CHECK(!strcmp(entry.text, "BloodGulch:CTF"), "the text kept was %s", entry.text);
		CHECK(playable("dangercanyon-2.v1"), "a name of '-' and '.' was rejected: %s", entry.rejected);
		CHECK(rejected_with("blood gulch", "is not a map's name"), "a name with a space was taken");
		CHECK(rejected_with(".hidden", "is not a map's name"), "a name beginning with a dot was taken");
		CHECK(rejected_with("blood*gulch", "is not a map's name"), "a name with a '*' was taken");
		CHECK(rejected_with(":ctf", "is not a map's name"), "an entry with no map was taken");
		return 0;
	}
	/* custom_maps\<name>: a Custom Edition map by its name */
	CASE("custom-maps")
	{
		CHECK(playable("custom_maps\\hugeass:team_slayer"), "a Custom Edition map was rejected: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_custom, "kind %d", entry.map_kind);
		CHECK(!strcmp(entry.map_name, "hugeass") && !strcmp(entry.level, "custom_maps\\hugeass"),
			"map %s, level %s", entry.map_name, entry.level);
		CHECK(playable("CUSTOM_MAPS/HugeAss"), "custom_maps in capitals with a slash was rejected: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_custom && !strcmp(entry.map, "custom_maps\\HugeAss"), "map %s", entry.map);
		CHECK(!strcmp(entry.map_name, "hugeass"), "map name %s", entry.map_name);
		CHECK(rejected_with("custom_maps\\", "is not a map's name"), "custom_maps\\ alone was taken");
		CHECK(rejected_with("custom_maps\\a\\b", "is not a map's name"), "a folder in custom_maps was taken");
		CHECK(rejected_with("custom_maps\\..", "is not a map's name"), "custom_maps\\.. was taken");
		CHECK(rejected_with("custom_maps/../maps/x", "is not a map's name"), "a path out of custom_maps was taken");
		return 0;
	}
	/* a level's path: taken as is, its last part the map's file; nothing
	that leaves the maps folder */
	CASE("paths")
	{
		CHECK(playable("levels\\a10\\a10:coop"), "a level's path was rejected: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_path, "kind %d", entry.map_kind);
		CHECK(!strcmp(entry.level, "levels\\a10\\a10") && !strcmp(entry.map_name, "a10"),
			"level %s, map %s", entry.level, entry.map_name);
		CHECK(!strcmp(entry.gametype, "coop"), "gametype %s", entry.gametype);
		CHECK(playable("levels/test/wizard/wizard:ctf"), "a path with slashes was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.level, "levels\\test\\wizard\\wizard"), "slashes kept: %s", entry.level);
		CHECK(playable("levels\\test\\v1.2\\map.x"), "dots inside a part were rejected: %s", entry.rejected);
		CHECK(rejected_with("levels\\..\\..\\etc\\passwd", "is not a level's path"), "a path with .. was taken");
		CHECK(rejected_with("levels/.../x", "is not a level's path"), "a part of dots only was taken");
		CHECK(rejected_with("levels\\test\\..", "is not a level's path"), "a path ending in .. was taken");
		CHECK(rejected_with("\\levels\\x", "is not a level's path"), "a path from the root was taken");
		CHECK(rejected_with("/etc/passwd", "is not a level's path"), "an absolute path was taken");
		CHECK(rejected_with("levels\\\\x", "is not a level's path"), "an empty part was taken");
		CHECK(rejected_with("levels\\x\\", "is not a level's path"), "a path ending in a backslash was taken");
		CHECK(rejected_with("levels\\a b\\c", "is not a level's path"), "a part with a space was taken");
		return 0;
	}
	/* the gametype: a name, file=<name>, lowercased and trimmed */
	CASE("gametypes")
	{
		char text[256];

		CHECK(playable("bloodgulch: Team_Slayer "), "a gametype with spaces was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.gametype, "team_slayer"), "gametype %s", entry.gametype);
		CHECK(playable("bloodgulch:file=My_CTF"), "file=<name> was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.gametype, "file=my_ctf"), "gametype %s", entry.gametype);
		CHECK(playable("bloodgulch:btb-slayer.v2"), "a gametype file's name was rejected: %s", entry.rejected);
		CHECK(rejected_with("bloodgulch:", "is not a gametype's name"), "an empty gametype was taken");
		CHECK(rejected_with("bloodgulch:  ", "is not a gametype's name"), "a gametype of spaces was taken");
		CHECK(rejected_with("bloodgulch:file=", "is not a gametype's name"), "file= with no name was taken");
		CHECK(rejected_with("bloodgulch:file=../x", "is not a gametype's name"), "file=../x was taken");
		CHECK(rejected_with("bloodgulch:file=.x", "is not a gametype's name"), "file=.x was taken");
		CHECK(rejected_with("bloodgulch:ctf:slayer", "is not a gametype's name"), "a second colon was taken");
		CHECK(rejected_with("bloodgulch:team slayer", "is not a gametype's name"), "a gametype with a space was taken");
		CHECK(rejected_with("bloodgulch:x=y", "is not a gametype's name"), "a gametype with '=' was taken");
		snprintf(text, sizeof(text), "bloodgulch:%s", repeated('g', SERVER_GAMETYPE_SIZE - 1));
		CHECK(playable(text), "the longest gametype was rejected: %s", entry.rejected);
		CHECK(strlen(entry.gametype) == SERVER_GAMETYPE_SIZE - 1, "kept %d characters", (int)strlen(entry.gametype));
		snprintf(text, sizeof(text), "bloodgulch:%s", repeated('g', SERVER_GAMETYPE_SIZE));
		CHECK(rejected_with(text, "is not a gametype's name"), "a gametype longer than its room was taken");
		return 0;
	}
	/* +rule=value: by key or alias, in any case, trimmed; the last of a rule
	given twice; an unknown rule, one with no value or out of range rejected */
	CASE("rules")
	{
		CHECK(playable("sidewinder:team_slayer+score=50+time=15"), "two rules were rejected: %s", entry.rejected);
		CHECK(entry.rule_count == 2 && rule_of("score_to_win") == 50 && rule_of("time_limit") == 15,
			"%d rules, score %ld, time %ld", entry.rule_count, rule_of("score_to_win"), rule_of("time_limit"));
		CHECK(playable("bloodgulch:ctf + FF = Off +Weapons=rocket_launchers+teams=yes"),
			"rules with spaces and capitals were rejected: %s", entry.rejected);
		CHECK(rule_of("friendly_fire") == 1 && rule_of("weapon_set") == 6 && rule_of("teams") == 1,
			"ff %ld, weapons %ld, teams %ld", rule_of("friendly_fire"), rule_of("weapon_set"), rule_of("teams"));
		CHECK(!strcmp(entry.rules[0].key, "friendly_fire"), "an alias was kept as the key: %s", entry.rules[0].key);
		CHECK(playable("bloodgulch:slayer+score=5+lives=3+score_to_win=7"), "a rule given twice was rejected: %s",
			entry.rejected);
		CHECK(entry.rule_count == 2 && rule_of("score_to_win") == 7 && rule_of("lives") == 3,
			"%d rules, score %ld: not the last given", entry.rule_count, rule_of("score_to_win"));
		CHECK(playable("bloodgulch:oddball+ball_spawn_count=16+speed_with_ball=0"), "an engine's rules were rejected: %s",
			entry.rejected);
		CHECK(rejected_with("bloodgulch:ctf+score", "the rule \"score\" has no value"), "a rule with no value was taken");
		CHECK(rejected_with("bloodgulch:ctf+", "has no value"), "an empty rule was taken");
		CHECK(rejected_with("bloodgulch:ctf+=5", "no rule is named \"\""), "a rule with no key was taken");
		CHECK(rejected_with("bloodgulch:ctf+bogus=1", "no rule is named \"bogus\""), "an unknown rule was taken");
		CHECK(rejected_with("bloodgulch:ctf+score_to=1", "no rule is named"), "the start of a key was taken");
		CHECK(rejected_with("bloodgulch:ctf+score=1000", "score_to_win takes 0 to 999, not 1000"),
			"a score out of range was taken: %s", entry.rejected);
		CHECK(rejected_with("bloodgulch:ctf+score=", "score_to_win takes a whole number, not \"\""),
			"an empty value was taken: %s", entry.rejected);
		CHECK(rejected_with("bloodgulch:ctf+score=5=6", "takes a whole number"), "a value with '=' was taken");
		CHECK(rejected_with("bloodgulch:ctf+score=5+health=10", "health takes 25 to 400, not 10"),
			"a second rule out of range was taken: %s", entry.rejected);
		CHECK(rejected_with("bloodgulch+score=5", "is not a map's name"), "rules with no gametype were taken");
		return 0;
	}
	/* spaces around an entry, an empty entry and a comment say nothing; a
	character that is not printable ASCII inside one rejects it */
	CASE("whitespace-comments")
	{
		CHECK(!parse(""), "nothing said something");
		CHECK(!parse(" \t\r\n "), "spaces said something");
		CHECK(!parse("# prisoner:slayer"), "a comment said something");
		CHECK(!parse("   #prisoner"), "a comment after spaces said something");
		CHECK(playable("\t bloodgulch:ctf \r\n"), "an entry in spaces was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.text, "bloodgulch:ctf"), "the text kept its spaces: \"%s\"", entry.text);
		CHECK(rejected_with("bloodgulch:\tctf", "not printable ASCII"), "a tab inside an entry was taken");
		CHECK(rejected_with("bl\xc3\xb6odgulch", "not printable ASCII"), "a character past ASCII was taken");
		CHECK(rejected_with("blood\x7fgulch", "not printable ASCII"), "a delete character was taken");
		CHECK(parse("bloodgulch#ctf") && entry.rejected[0], "a '#' inside a name was taken");
		/* (only the length given is read) */
		CHECK(server_rotation_parse_entry("bloodgulch:ctf", 10, &entry) && !strcmp(entry.text, "bloodgulch") &&
			!strcmp(entry.gametype, "slayer"), "read past the length: %s", entry.text);
		return 0;
	}
	/* the rotation has no quoting: a quote is no map's or gametype's */
	CASE("quotes")
	{
		CHECK(rejected_with("\"bloodgulch\"", "is not a map's name"), "a quoted map was taken");
		CHECK(rejected_with("'bloodgulch'", "is not a map's name"), "a map in single quotes was taken");
		CHECK(rejected_with("bloodgulch:\"ctf\"", "is not a gametype's name"), "a quoted gametype was taken");
		CHECK(rejected_with("bloodgulch:ctf+weapons=\"plasma\"", "weapon_set takes"), "a quoted value was taken");
		server_rotation_parse("\"bloodgulch:ctf, sidewinder:slayer\"", &rotation);
		CHECK(rotation.count == 2 && !server_rotation_playable(&rotation),
			"a quoted rotation's commas were not separators: %d entries", rotation.count);
		return 0;
	}
	/* an entry or a map longer than its room is rejected, its text cut */
	CASE("long")
	{
		char text[600];

		CHECK(rejected_with(repeated('a', SERVER_ENTRY_TEXT_SIZE), "longer than 255 characters"),
			"a 256 character entry was taken: %s", entry.rejected);
		CHECK(strlen(entry.text) == SERVER_ENTRY_TEXT_SIZE - 1, "its text is %d long", (int)strlen(entry.text));
		CHECK(!strcmp(entry.text + SERVER_ENTRY_TEXT_SIZE - 4, "..."), "its text does not end in ...: %s",
			entry.text + SERVER_ENTRY_TEXT_SIZE - 8);
		CHECK(rejected_with(repeated('a', 500), "longer than"), "a 500 character entry was taken");
		CHECK(rejected_with(repeated('a', SERVER_ENTRY_TEXT_SIZE - 1), "the map's name is too long"),
			"a 255 character name was taken: %s", entry.rejected);
		CHECK(rejected_with(repeated('a', SERVER_MAP_SIZE), "the map's name is too long"),
			"a %d character name was taken: %s", SERVER_MAP_SIZE, entry.rejected);
		snprintf(text, sizeof(text), "custom_maps\\%s", repeated('a', SERVER_MAP_SIZE - 12));
		CHECK(rejected_with(text, "the map's name is too long"), "a %d character custom map was taken: %s",
			(int)strlen(text), entry.rejected);
		snprintf(text, sizeof(text), "custom_maps\\%s", repeated('a', SERVER_MAP_SIZE - 13));
		CHECK(playable(text), "the longest custom map was rejected: %s", entry.rejected);
		CHECK(!strcmp(entry.level, text), "level %s", entry.level);
		/* (spaces around an entry do not count) */
		snprintf(text, sizeof(text), "%sbloodgulch%s", repeated(' ', 300), "");
		snprintf(text + strlen(text), sizeof(text) - strlen(text), "%s", repeated(' ', 200));
		CHECK(playable(text), "an entry in many spaces was rejected: %s", entry.rejected);
		return 0;
	}
	/* a map's name whose Xbox level is longer than the level's room is
	rejected, not played as a cut level */
	CASE("long-map-level")
	{
		char const *name = repeated('m', 60);

		CHECK(parse(name), "a 60 character name said nothing");
		CHECK(entry.rejected[0] || strlen(entry.level) == 13 + 2 * strlen(name),
			"its level was cut to \"%s\" (%d characters) and the entry taken", entry.level, (int)strlen(entry.level));
		return 0;
	}
	/* ui is the main menu's, however it is given */
	CASE("ui")
	{
		CHECK(rejected_with("ui", "main menu"), "ui was taken");
		CHECK(rejected_with("UI:ctf", "main menu"), "UI was taken");
		CHECK(rejected_with("levels\\ui\\ui", "main menu"), "ui's level was taken");
		CHECK(rejected_with("custom_maps\\ui", "main menu"), "custom_maps\\ui was taken");
		CHECK(playable("uil"), "a name beginning with ui was rejected: %s", entry.rejected);
		return 0;
	}
	/* entries are separated by commas, newlines and semicolons; empty ones
	and comments are left out, rejected ones kept */
	CASE("rotation-separators")
	{
		server_rotation_parse("bloodgulch:ctf, sidewinder:team_slayer+score=50\nprisoner;wizard:king\r\n", &rotation);
		CHECK(rotation.count == 4 && !rotation.truncated, "%d entries", rotation.count);
		CHECK(!strcmp(rotation.entries[0].map_name, "bloodgulch") && !strcmp(rotation.entries[1].map_name, "sidewinder") &&
			!strcmp(rotation.entries[2].map_name, "prisoner") && !strcmp(rotation.entries[3].map_name, "wizard"),
			"the entries are not in order");
		CHECK(!strcmp(rotation.entries[1].text, "sidewinder:team_slayer+score=50"), "text %s", rotation.entries[1].text);
		CHECK(server_rotation_playable(&rotation) == 4, "%d playable", server_rotation_playable(&rotation));
		server_rotation_parse(",,, ,\n\n;", &rotation);
		CHECK(rotation.count == 0 && !rotation.truncated, "separators alone made %d entries", rotation.count);
		server_rotation_parse("", &rotation);
		CHECK(rotation.count == 0 && !server_rotation_playable(&rotation), "nothing made %d entries", rotation.count);
		server_rotation_parse("# the rotation\nbloodgulch\n  # prisoner\nwizard", &rotation);
		CHECK(rotation.count == 2, "comments made %d entries", rotation.count);
		server_rotation_parse("bloodgulch, nowhere:bogus+x, ui, prisoner", &rotation);
		CHECK(rotation.count == 4, "rejected entries were left out: %d entries", rotation.count);
		CHECK(server_rotation_playable(&rotation) == 2, "%d playable", server_rotation_playable(&rotation));
		CHECK(rotation.entries[1].rejected[0] && rotation.entries[2].rejected[0] && !rotation.entries[3].rejected[0],
			"the wrong entries were rejected");
		return 0;
	}
	/* 64 entries and 4096 characters at most: the rest left out, and said */
	CASE("rotation-limits")
	{
		char text[8192];
		int index;

		text[0] = 0;
		for (index = 0; index < SERVER_ROTATION_MAXIMUM_ENTRIES; index++)
			snprintf(text + strlen(text), sizeof(text) - strlen(text), "%smap%d", index ? "," : "", index);
		server_rotation_parse(text, &rotation);
		CHECK(rotation.count == SERVER_ROTATION_MAXIMUM_ENTRIES && !rotation.truncated, "%d entries, truncated %d",
			rotation.count, rotation.truncated);
		snprintf(text + strlen(text), sizeof(text) - strlen(text), ", ,\n# the end\n");
		server_rotation_parse(text, &rotation);
		CHECK(rotation.count == SERVER_ROTATION_MAXIMUM_ENTRIES && !rotation.truncated,
			"empty entries and comments after the last counted as more (truncated %d)", rotation.truncated);
		snprintf(text + strlen(text), sizeof(text) - strlen(text), ",extra");
		server_rotation_parse(text, &rotation);
		CHECK(rotation.count == SERVER_ROTATION_MAXIMUM_ENTRIES && rotation.truncated,
			"a 65th entry: %d entries, truncated %d", rotation.count, rotation.truncated);
		CHECK(!strcmp(rotation.entries[SERVER_ROTATION_MAXIMUM_ENTRIES - 1].map_name, "map63"), "the last entry is %s",
			rotation.entries[SERVER_ROTATION_MAXIMUM_ENTRIES - 1].map_name);
		/* (4096 characters of entries, then more) */
		snprintf(text, sizeof(text), "bloodgulch,%s", repeated(' ', SERVER_ROTATION_MAXIMUM_TEXT - 11));
		server_rotation_parse(text, &rotation);
		CHECK(rotation.count == 1 && !rotation.truncated, "4096 characters were too many");
		snprintf(text, sizeof(text), "bloodgulch,%s,prisoner", repeated(' ', SERVER_ROTATION_MAXIMUM_TEXT));
		server_rotation_parse(text, &rotation);
		CHECK(rotation.count == 1 && rotation.truncated, "text past 4096 characters: %d entries, truncated %d",
			rotation.count, rotation.truncated);
		return 0;
	}
	/* an entry that runs past the 4096th character is left out, not read cut */
	CASE("rotation-cut-entry")
	{
		char text[8192];

		snprintf(text, sizeof(text), "%s,bloodgulch:ctf+score=50", repeated(' ', SERVER_ROTATION_MAXIMUM_TEXT - 23));
		CHECK(strlen(text) == SERVER_ROTATION_MAXIMUM_TEXT + 1, "the text is %d long", (int)strlen(text));
		server_rotation_parse(text, &rotation);
		CHECK(rotation.truncated, "the text was not cut");
		CHECK(rotation.count == 0 || rotation.entries[0].rejected[0],
			"the entry cut at the 4096th character was taken as \"%s\" (score %ld)", rotation.entries[0].text,
			rotation.entries[0].rules[0].value);
		return 0;
	}
	/* the rotation written as text reads back the same */
	CASE("round-trip")
	{
		char text[SERVER_ROTATION_MAXIMUM_TEXT * 2];
		char second[SERVER_ROTATION_MAXIMUM_TEXT * 2];
		char source[2048];
		int index;

		snprintf(source, sizeof(source),
			"bloodgulch:ctf\n  sidewinder : team_slayer + score = 50 + time=15 ;custom_maps/hugeass:file=mine,"
			"levels\\a10\\a10:coop, # a comment\n nowhere:bogus+x, ui,%s, prisoner:oddball+ball_spawn_count=3",
			repeated('z', 300));
		server_rotation_parse(source, &rotation);
		server_rotation_text(&rotation, text, sizeof(text));
		server_rotation_parse(text, &again);
		CHECK(again.count == rotation.count && rotation.count == 8, "%d entries, then %d", rotation.count, again.count);
		for (index = 0; index < rotation.count; index++)
		{
			struct server_entry const *first = &rotation.entries[index];
			struct server_entry const *read_again = &again.entries[index];

			CHECK(!strcmp(first->text, read_again->text), "entry %d: \"%s\", then \"%s\"", index, first->text,
				read_again->text);
			CHECK(!first->rejected[0] == !read_again->rejected[0], "entry %d: rejected \"%s\", then \"%s\"", index,
				first->rejected, read_again->rejected);
			if (first->rejected[0])
				continue;
			CHECK(!strcmp(first->map, read_again->map) && !strcmp(first->level, read_again->level) &&
				!strcmp(first->gametype, read_again->gametype) && first->map_kind == read_again->map_kind,
				"entry %d read back otherwise", index);
			CHECK(first->rule_count == read_again->rule_count &&
				!memcmp(first->rules, read_again->rules, sizeof(first->rules[0]) * (size_t)first->rule_count),
				"entry %d's rules read back otherwise", index);
		}
		server_rotation_text(&again, second, sizeof(second));
		CHECK(!strcmp(text, second), "the text changed when written again:\n%s\n%s", text, second);
		CHECK(!strncmp(text, "bloodgulch:ctf, sidewinder : team_slayer + score = 50 + time=15, custom_maps/hugeass:",
			strlen("bloodgulch:ctf, sidewinder : team_slayer + score = 50 + time=15, custom_maps/hugeass:")),
			"written as %s", text);
		/* (a small room: cut, ended, nothing past it) */
		memset(second, '#', 64);
		server_rotation_text(&rotation, second, 20);
		CHECK(strlen(second) == 19 && second[20] == '#', "a 20 character room held %d, the next '%c'",
			(int)strlen(second), second[20]);
		server_rotation_text(&rotation, second, 1);
		CHECK(!second[0], "a room of one held something");
		return 0;
	}
	/* every rule by its key and alias, in any case, and nothing else */
	CASE("rule-find")
	{
		static char const *const aliases[][2] =
		{
			{ "score", "score_to_win" }, { "respawn", "respawn_time" }, { "weapons", "weapon_set" },
			{ "vehicles", "vehicle_set" }, { "time", "time_limit" }, { "ff", "friendly_fire" },
			{ "balance", "auto_team_balance" },
		};
		static char const *const tables[] = { "universal", "options", "ctf", "slayer", "king", "oddball", "race" };
		int index, other;

		CHECK(server_rule_definition_count() > 40, "%d rules", server_rule_definition_count());
		for (index = 0; index < server_rule_definition_count(); index++)
		{
			const struct server_rule_definition *rule = &server_rule_definitions[index];
			char upper[64];
			int table, letter;

			CHECK(server_rule_find(rule->key) == rule, "%s found another rule", rule->key);
			for (letter = 0; rule->key[letter]; letter++)
				upper[letter] = (char)toupper((unsigned char)rule->key[letter]);
			upper[letter] = 0;
			CHECK(server_rule_find(upper) == rule, "%s not found in capitals", upper);
			CHECK(!rule->alias || server_rule_find(rule->alias) == rule, "%s's alias found another rule", rule->key);
			CHECK(strlen(rule->key) < SERVER_RULE_KEY_SIZE, "%s is longer than a rule's key", rule->key);
			for (table = 0; table < (int)(sizeof(tables) / sizeof(tables[0])) && strcmp(rule->table, tables[table]);
				table++)
			{
			}
			CHECK(table < (int)(sizeof(tables) / sizeof(tables[0])), "%s is in the table %s", rule->key, rule->table);
			CHECK(rule->engines && !(rule->engines & ~_server_engine_all), "%s's engines are %x", rule->key, rule->engines);
			CHECK(rule->minimum <= rule->maximum, "%s's range is %ld to %ld", rule->key, rule->minimum, rule->maximum);
			CHECK(rule->type != _server_rule_integer || !rule->choices, "%s has words", rule->key);
			CHECK(rule->type == _server_rule_integer || (rule->choices && rule->choices[0] && rule->choices[1]),
				"%s has no words", rule->key);
			CHECK(rule->description && rule->description[0], "%s has no description", rule->key);
			for (other = 0; other < index; other++)
			{
				const struct server_rule_definition *before = &server_rule_definitions[other];

				CHECK(strcmp(before->key, rule->key) && (!rule->alias || strcmp(before->key, rule->alias)) &&
					(!before->alias || strcmp(before->alias, rule->key)),
					"%s and %s share a name", before->key, rule->key);
			}
		}
		for (index = 0; index < (int)(sizeof(aliases) / sizeof(aliases[0])); index++)
		{
			const struct server_rule_definition *rule = server_rule_find(aliases[index][0]);

			CHECK(rule && !strcmp(rule->key, aliases[index][1]), "%s found %s", aliases[index][0], rule ? rule->key : "none");
		}
		CHECK(server_rule_find("FF") == server_rule_find("friendly_fire"), "FF did not find friendly_fire");
		CHECK(!server_rule_find(""), "an empty key found a rule");
		CHECK(!server_rule_find("scor"), "the start of an alias found a rule");
		CHECK(!server_rule_find("score_to_win_"), "a key with more after it found a rule");
		CHECK(!server_rule_find("bogus"), "an unknown key found a rule");
		CHECK(!strcmp(server_rule_find("assault")->table, "ctf") && server_rule_find("assault")->engines == _server_engine_ctf,
			"assault is not ctf's");
		CHECK(!strcmp(server_rule_find("moving_hill")->table, "king"), "moving_hill is not king's");
		return 0;
	}
	/* numbers in range, booleans' words, choices' words and indices; why not */
	CASE("rule-values")
	{
		char reason[SERVER_REASON_SIZE];
		long value;

		CHECK(value_of("score", "0", &value, reason) && value == 0, "0: %s", reason);
		CHECK(value_of("score", "999", &value, reason) && value == 999, "999: %s", reason);
		CHECK(value_of("score", "050", &value, reason) && value == 50, "050: %ld", value);
		CHECK(!value_of("score", "1000", &value, reason) && !strcmp(reason, "score_to_win takes 0 to 999, not 1000"),
			"1000: %s", reason);
		CHECK(value == NONE, "a rejected value was written: %ld", value);
		CHECK(!value_of("score", "-1", &value, reason) && strstr(reason, "not -1"), "-1: %s", reason);
		CHECK(!value_of("health", "24", &value, reason) && value_of("health", "25", &value, reason) &&
			value_of("health", "400", &value, reason) && !value_of("health", "401", &value, reason),
			"health's range is not 25 to 400");
		CHECK(!value_of("score", "+5", &value, reason) && strstr(reason, "takes a whole number, not \"+5\""), "+5: %s",
			reason);
		CHECK(!value_of("score", "5x", &value, reason), "5x was taken");
		CHECK(!value_of("score", "1e3", &value, reason), "1e3 was taken");
		CHECK(!value_of("score", "", &value, reason), "nothing was taken");
		CHECK(!value_of("score", "-", &value, reason), "- was taken");
		CHECK(!value_of("score", "1234567890", &value, reason), "ten digits were taken");
		CHECK(!value_of("score", "99999999999999999999", &value, reason), "a number past a long was taken");
		CHECK(!value_of("score", "five", &value, reason), "a word was taken for a number");
		CHECK(value_of("ball_spawn_count", "1", &value, reason) && !value_of("ball_spawn_count", "0", &value, reason),
			"ball_spawn_count's range is not from 1");
		/* (booleans) */
		CHECK(value_of("teams", "true", &value, reason) && value == 1, "true: %ld", value);
		CHECK(value_of("teams", "TRUE", &value, reason) && value == 1, "TRUE: %ld", value);
		CHECK(value_of("teams", "Yes", &value, reason) && value == 1, "Yes: %ld", value);
		CHECK(value_of("teams", "on", &value, reason) && value == 1, "on: %ld", value);
		CHECK(value_of("teams", "1", &value, reason) && value == 1, "1: %ld", value);
		CHECK(value_of("teams", "false", &value, reason) && value == 0, "false: %ld", value);
		CHECK(value_of("teams", "NO", &value, reason) && value == 0, "NO: %ld", value);
		CHECK(value_of("teams", "Off", &value, reason) && value == 0, "Off: %ld", value);
		CHECK(value_of("teams", "0", &value, reason) && value == 0, "0: %ld", value);
		CHECK(!value_of("teams", "2", &value, reason) && !strcmp(reason, "teams takes true or false, not \"2\""),
			"2: %s", reason);
		CHECK(!value_of("teams", "y", &value, reason), "y was taken");
		CHECK(!value_of("teams", "", &value, reason), "nothing was taken");
		CHECK(!value_of("teams", "truer", &value, reason), "truer was taken");
		CHECK(!value_of("teams", "01", &value, reason), "01 was taken");
		/* (choices) */
		CHECK(value_of("ff", "on", &value, reason) && value == 0, "on: %ld", value);
		CHECK(value_of("ff", "OFF", &value, reason) && value == 1, "OFF: %ld", value);
		CHECK(value_of("ff", "explosives_only", &value, reason) && value == 3, "explosives_only: %ld", value);
		CHECK(value_of("ff", "3", &value, reason) && value == 3, "3: %ld", value);
		CHECK(!value_of("ff", "4", &value, reason), "an index past the words was taken");
		CHECK(!value_of("ff", "-1", &value, reason), "-1 was taken");
		CHECK(!value_of("ff", "", &value, reason), "nothing was taken");
		CHECK(!value_of("ff", "of", &value, reason), "the start of a word was taken");
		CHECK(!value_of("ff", "bogus", &value, reason) &&
			!strcmp(reason, "friendly_fire takes on, off, shields_only or explosives_only, not \"bogus\""), "%s", reason);
		CHECK(value_of("weapons", "heavy", &value, reason) && value == 13, "heavy: %ld", value);
		CHECK(value_of("vehicles", "scorpion", &value, reason) && value == 4, "scorpion: %ld", value);
		CHECK(value_of("goal_radar", "nav_point", &value, reason) && value == 1, "nav_point: %ld", value);
		CHECK(value_of("primary_weapon", "needler", &value, reason) && value == 9, "needler: %ld", value);
		/* (a long reason, cut to its room) */
		CHECK(!value_of("weapons", "bogus", &value, reason), "bogus was taken");
		CHECK(strlen(reason) < SERVER_REASON_SIZE && !strncmp(reason, "weapon_set takes normal, pistols, ", 34),
			"%s", reason);
		{
			char small[40];
			const struct server_rule_definition *rule = server_rule_find("weapons");

			memset(small, '#', sizeof(small));
			CHECK(!server_rule_value(rule, "bogus", &value, small, 24), "bogus was taken");
			CHECK(strlen(small) == 23 && small[24] == '#', "a 24 character room held %d, the next '%c'",
				(int)strlen(small), small[24]);
		}
		return 0;
	}
	/* a campaign level only as co-op, a multiplayer map never so, the main
	menu's not at all */
	CASE("check-kind")
	{
		CHECK(parse("bloodgulch:ctf"), "nothing read");
		server_rotation_check_kind(&entry, 1, 5);
		CHECK(!entry.rejected[0], "a multiplayer map was rejected: %s", entry.rejected);
		server_rotation_check_kind(&entry, 0, 5);
		CHECK(strstr(entry.rejected, "bloodgulch is a campaign level (as co-op: bloodgulch:coop)"), "%s", entry.rejected);
		parse("levels\\a10\\a10:coop");
		server_rotation_check_kind(&entry, 0, 5);
		CHECK(!entry.rejected[0], "a campaign level as co-op was rejected: %s", entry.rejected);
		server_rotation_check_kind(&entry, 1, 5);
		CHECK(strstr(entry.rejected, "is not a campaign level"), "a multiplayer map as co-op: %s", entry.rejected);
		parse("menu");
		server_rotation_check_kind(&entry, 2, 5);
		CHECK(strstr(entry.rejected, "main menu"), "the main menu's map: %s", entry.rejected);
		return 0;
	}
	/* a cache file's header: both signatures, its version and scenario type */
	CASE("map-header")
	{
		char path[1024];
		long version, type;

		make_root(argv[0], case_name);
		write_map("xbox.map", 5, 1, 0x800, "daeh", "toof");
		write_map("ce.map", 609, 0, 0x2000, "daeh", "toof");
		write_map("negative.map", 0x12345678, 0xFFFF, 0x800, "daeh", "toof");
		write_map("short.map", 5, 1, 0x7FF, "daeh", "toof");
		write_map("head.map", 5, 1, 0x800, "head", "toof");
		write_map("foot.map", 5, 1, 0x800, "daeh", "foot");
		snprintf(path, sizeof(path), "%s/xbox.map", data_root);
		CHECK(server_map_header(path, &version, &type) && version == 5 && type == 1, "version %ld, type %ld", version,
			type);
		snprintf(path, sizeof(path), "%s/ce.map", data_root);
		CHECK(server_map_header(path, &version, &type) && version == 609 && type == 0, "version %ld, type %ld",
			version, type);
		snprintf(path, sizeof(path), "%s/negative.map", data_root);
		CHECK(server_map_header(path, &version, &type) && version == 0x12345678 && type == -1,
			"version %lx, type %ld", version, type);
		snprintf(path, sizeof(path), "%s/short.map", data_root);
		CHECK(!server_map_header(path, &version, &type) && version == 0 && type == -1, "a short file was read");
		snprintf(path, sizeof(path), "%s/head.map", data_root);
		CHECK(!server_map_header(path, &version, &type), "a file with 'head' backwards was read");
		snprintf(path, sizeof(path), "%s/foot.map", data_root);
		CHECK(!server_map_header(path, &version, &type), "a file with 'foot' backwards was read");
		snprintf(path, sizeof(path), "%s/none.map", data_root);
		CHECK(!server_map_header(path, &version, &type) && version == 0 && type == -1, "a missing file was read");
		CHECK(!server_map_header(data_root, &version, &type), "a folder was read");
		return 0;
	}
	/* an entry's map found in maps, custom_maps or Custom Edition's folder */
	CASE("resolve")
	{
		char path[1024];

		make_root(argv[0], case_name);
		write_maps();
		CHECK(resolved("bloodgulch:ctf"), "bloodgulch: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_name && !strcmp(entry.level, "levels\\test\\bloodgulch\\bloodgulch"),
			"level %s", entry.level);
		CHECK(resolved("BloodGulch"), "BloodGulch: %s", entry.rejected);
		CHECK(resolved("hugeass:team_slayer"), "a Custom Edition map by its name: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_custom && !strcmp(entry.level, "custom_maps\\hugeass"),
			"kind %d, level %s", entry.map_kind, entry.level);
		CHECK(resolved("custom_maps\\hugeass"), "custom_maps\\hugeass: %s", entry.rejected);
		CHECK(!resolved("nowhere") && entry.map_missing && strstr(entry.rejected, "there is no map nowhere"),
			"a missing map: %s", entry.rejected);
		CHECK(!resolved("custom_maps\\bloodgulch") && entry.map_missing &&
			strstr(entry.rejected, "no Custom Edition map bloodgulch"), "an Xbox map as Custom Edition's: %s",
			entry.rejected);
		CHECK(!resolved("levels\\test\\nowhere\\nowhere") && entry.map_missing, "a missing level: %s", entry.rejected);
		CHECK(!resolved("broken") && entry.map_missing, "a file too short for a header: %s", entry.rejected);
		CHECK(!resolved("a10") && !entry.map_missing && strstr(entry.rejected, "campaign level"), "a10: %s",
			entry.rejected);
		CHECK(resolved("a10:coop"), "a10 as co-op: %s", entry.rejected);
		CHECK(resolved("levels\\a10\\a10:coop"), "a10's level as co-op: %s", entry.rejected);
		CHECK(!resolved("custom_maps\\c_campaign:slayer") && strstr(entry.rejected, "campaign level"),
			"a Custom Edition campaign level: %s", entry.rejected);
		CHECK(!resolved("bloodgulch:coop") && strstr(entry.rejected, "not a campaign level"), "bloodgulch:coop: %s",
			entry.rejected);
		/* (an entry already rejected stays as it is) */
		CHECK(!resolved("nowhere+x") && !entry.map_missing && strstr(entry.rejected, "is not a map's name"),
			"a rejected entry was resolved: %s", entry.rejected);
		/* (paths.custom_maps, then Custom Edition's own maps folder) */
		make_folder("elsewhere");
		write_map("elsewhere/other.map", 609, 1, 0x800, "daeh", "toof");
		snprintf(custom_maps_setting, sizeof(custom_maps_setting), "%s/elsewhere", data_root);
		CHECK(resolved("custom_maps\\other"), "a map in paths.custom_maps: %s", entry.rejected);
		CHECK(!resolved("custom_maps\\hugeass") && entry.map_missing, "custom_maps was searched past the setting");
		custom_maps_setting[0] = 0;
		make_folder("ce");
		make_folder("ce/maps");
		write_map("ce/maps/ceonly.map", 609, 1, 0x800, "daeh", "toof");
		CHECK(!resolved("ceonly"), "a map in no folder was found");
		snprintf(custom_edition_root, sizeof(custom_edition_root), "%s/ce", data_root);
		CHECK(resolved("ceonly"), "a map in Custom Edition's folder: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_custom && !strcmp(entry.level, "custom_maps\\ceonly"), "level %s",
			entry.level);
		CHECK(resolved("custom_maps\\ceonly"), "custom_maps\\ceonly: %s", entry.rejected);
		snprintf(path, sizeof(path), "%s/maps/bloodgulch.map", data_root);
		CHECK(!access(path, R_OK), "the maps were not written");
		return 0;
	}
	/* a Custom Edition map whose file has capitals (cmt_Snow_Grove.map) is
	found by the name the maps' list gives it, in lower case, in each of
	custom_maps, paths.custom_maps and Custom Edition's own folder */
	CASE("resolve-case")
	{
		int told = 0;

		make_root(argv[0], case_name);
		write_maps();
		write_map("custom_maps/Mixed_Case.map", 609, 1, 0x800, "daeh", "toof");
		server_maps_each(tell_map, &told);
		CHECK(strstr(maps_told, "mixed_case custom_maps\\mixed_case custom;"), "%s", maps_told);
		CHECK(resolved("mixed_case:ctf"), "mixed_case: %s", entry.rejected);
		CHECK(entry.map_kind == _server_map_custom && !strcmp(entry.level, "custom_maps\\mixed_case"),
			"kind %d, level %s", entry.map_kind, entry.level);
		CHECK(resolved("custom_maps\\mixed_case:ctf+ff=off"), "custom_maps\\mixed_case: %s", entry.rejected);
		CHECK(resolved("custom_maps\\MIXED_CASE"), "custom_maps\\MIXED_CASE: %s", entry.rejected);
		CHECK(!resolved("custom_maps\\mixed_cases") && entry.map_missing, "a name of no file was found");
		make_folder("elsewhere");
		write_map("elsewhere/Other_Case.map", 609, 1, 0x800, "daeh", "toof");
		snprintf(custom_maps_setting, sizeof(custom_maps_setting), "%s/elsewhere", data_root);
		CHECK(resolved("custom_maps\\other_case"), "a map in paths.custom_maps: %s", entry.rejected);
		custom_maps_setting[0] = 0;
		make_folder("ce");
		make_folder("ce/maps");
		write_map("ce/maps/CE_Case.map", 609, 1, 0x800, "daeh", "toof");
		snprintf(custom_edition_root, sizeof(custom_edition_root), "%s/ce", data_root);
		CHECK(resolved("ce_case"), "a map in Custom Edition's folder: %s", entry.rejected);
		custom_edition_root[0] = 0;
		return 0;
	}
	/* a map file of another game's version is not played as the Xbox's or
	Custom Edition's (server_maps_each does not list it) */
	CASE("resolve-version")
	{
		make_root(argv[0], case_name);
		write_maps();
		CHECK(!resolved("pcmap"), "a Halo PC map (version 7) in maps was taken");
		CHECK(!resolved("custom_maps\\xboxmap"), "an Xbox map (version 5) in custom_maps was taken");
		return 0;
	}
	/* the multiplayer maps there are: the Xbox's of version 5, Custom
	Edition's of 609, not the resource maps', the main menu's or a campaign's */
	CASE("maps-each")
	{
		int count = 0, told = 0;

		make_root(argv[0], case_name);
		write_maps();
		count = server_maps_each(tell_map, &told);
		CHECK(count == 3 && told == 3, "%d maps (%d told): %s", count, told, maps_told);
		CHECK(strstr(maps_told, "bloodgulch levels\\test\\bloodgulch\\bloodgulch xbox;"), "%s", maps_told);
		CHECK(strstr(maps_told, "sidewinder levels\\test\\sidewinder\\sidewinder xbox;"), "%s", maps_told);
		CHECK(strstr(maps_told, "hugeass custom_maps\\hugeass custom;"), "%s", maps_told);
		snprintf(data_root, sizeof(data_root), "/nonexistent-halo-harness");
		maps_told[0] = 0;
		CHECK(!server_maps_each(tell_map, &told) && !maps_told[0], "maps were found in no folder");
		return 0;
	}
	fprintf(stderr, "unknown case: %s\n", case_name);
	return 2;
}

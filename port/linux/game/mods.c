/*
MODS.C

Mods: tag files played over a map's tags on this machine, as a resource
pack is: a map's sounds (mod_sounds.c), bitmaps (mod_bitmaps.c), text
(mod_strings.c), models (mod_models.c) and model and environment shaders
(mod_shaders.c) from Halo PC tag files, as the
Halo Editing Kit and Invader write them, in the mods of the mods folder that
are on, in their load order (port/linux/src/port_mods.c; the main menu's
Mods chooses: menu_functions.c; the mod.toml files: docs/mods.md). Nothing a
mod plays is anything the machines of a network game must agree on: no tag
that the game simulates is played from a file (a model's nodes, markers and
regions must be the map's: only what is drawn of it is the file's), and no
mod is sent to another machine, so a mod is this machine's alone, in any
game.

When a map's tags load (scenario_tags_load), when the console asks
(mods_reload) and when the Mods menu's OK keeps a new order (mods_apply),
the mods are found again, and each of the map's tags of those groups that a
mod has a file of (tags\<its name>.<its group's extension>)
is played from the file, made what this build plays: the tag's definition
takes the file's values in place, so everything that holds the tag keeps
holding it, and the map's own values come back when the file goes. A file
that is not what its group's reader can trust (each checks every count,
offset and value against the file and against what the game reads), or
that does not fit what the map's tags ask of the tag it replaces, is said
and left out, and the map's tag played. mods_enable false gives every tag
back until mods_enable true, or until the map changes.
*/

/* ---------- headers */

#include "cseries.h"
#include "main/console.h"
#include "mods.h"
#include "mcc_cache.h"

#include <stdlib.h>
#include <string.h>

/* the game's (port) */
char *tag_get_name(long tag_index);

/* ---------- constants */

/* the most the mods' tags keep in memory (their files, their sounds'
samples, their bitmaps' tags and their models' geometry; bitmaps' pixels
are read as they are drawn) */
#if defined(HALO_ANDROID) || defined(HALO_WEB)
#define MAXIMUM_KEPT_BYTES (192UL * 1024 * 1024)
#else
#define MAXIMUM_KEPT_BYTES (512UL * 1024 * 1024)
#endif

/* ---------- globals */

static struct
{
	boolean map_loaded;
	boolean enabled;
	char map_name[64];
	unsigned long kept_bytes;
	/* how many times the mods had been found when they last were here
	(port_mods_scans) */
	unsigned long scans;
	/* the groups' updates say what they did at the console (not as the
	Mods menu applies them) */
	boolean say;
	/* how many of the map's strings, bitmaps, sounds, models and shaders the
	mods play */
	long string_count;
	long bitmap_count;
	long sound_count;
	long model_count;
	long shader_count;
} mods_globals;

/* ---------- private code */

static void mods_let_go(boolean give_back);

/* every group's tags played from the mods' files, or (remove_all) none */
static void mods_update(
	boolean remove_all,
	boolean at_load)
{
	mods_globals.string_count = mod_strings_update(remove_all, at_load);
	mods_globals.bitmap_count = mod_bitmaps_update(remove_all, at_load);
	mods_globals.sound_count = mod_sounds_update(remove_all, at_load);
	mods_globals.model_count = mod_models_update(remove_all, at_load);
	mods_globals.shader_count = mod_shaders_update(remove_all, at_load);

	return;
}

/* ---------- public code */

unsigned long mods_read_be16(
	byte const *bytes)
{
	return ((unsigned long)bytes[0] << 8) | bytes[1];
}

unsigned long mods_read_be32(
	byte const *bytes)
{
	return ((unsigned long)bytes[0] << 24) | ((unsigned long)bytes[1] << 16) |
		((unsigned long)bytes[2] << 8) | bytes[3];
}

short mods_read_short(
	byte const *bytes)
{
	return (short)mods_read_be16(bytes);
}

real mods_read_real(
	byte const *bytes)
{
	unsigned long bits = mods_read_be32(bytes);
	real value;

	memcpy(&value, &bits, sizeof(value));

	return value;
}

/* whether `size` bytes at `offset` lie within `file_size` */
boolean mods_in_file(
	unsigned long offset,
	unsigned long size,
	unsigned long file_size)
{
	return offset <= file_size && size <= file_size - offset;
}

boolean mods_finite_real(
	real value)
{
	return value == value && value - value == 0.f;
}

boolean mods_tag_file_header_valid(
	byte const *file,
	unsigned long group_tag,
	short version)
{
	return mods_read_be32(file + MOD_TAG_FILE_GROUP_OFFSET) == group_tag &&
		!memcmp(file + MOD_TAG_FILE_SIGNATURE_OFFSET, "blam", 4) &&
		mods_read_be32(file + MOD_TAG_FILE_HEADER_SIZE_OFFSET) == MOD_TAG_FILE_HEADER_SIZE &&
		mods_read_short(file + MOD_TAG_FILE_VERSION_OFFSET) == version;
}

long mods_tag_file_find(
	long tag_index,
	char const *extension)
{
	char const *name = tag_get_name(tag_index);
	char tag_file[512];

	if (!mods_globals.map_loaded || !name || !*name || strlen(name) + strlen(extension) >= sizeof(tag_file))
	{
		return NONE;
	}
	strcpy(tag_file, name);
	strcat(tag_file, extension);

	return port_mods_file_find(tag_file, mods_globals.map_name);
}

byte *mods_tag_file_read(
	long file,
	unsigned long *size,
	char const **reason)
{
	unsigned long stamp[2];
	byte *data;

	*reason = "it cannot be read";
	if (!port_mods_file_stamp(file, size, stamp))
	{
		return NULL;
	}
	if (*size < MOD_TAG_FILE_HEADER_SIZE || *size > MAXIMUM_MOD_TAG_FILE_SIZE)
	{
		*reason = "it is not the size of a tag file";
		return NULL;
	}
	data = malloc(*size);
	if (!data)
	{
		*reason = "out of memory";
		return NULL;
	}
	if (!port_mods_file_read(file, 0, data, *size))
	{
		free(data);
		return NULL;
	}

	return data;
}

boolean mods_memory_take(
	unsigned long size)
{
	if (size > MAXIMUM_KEPT_BYTES - mods_globals.kept_bytes)
	{
		return FALSE;
	}
	mods_globals.kept_bytes += size;

	return TRUE;
}

unsigned long mods_memory_left(
	void)
{
	return MAXIMUM_KEPT_BYTES - mods_globals.kept_bytes;
}

void mods_memory_give(
	unsigned long size)
{
	mods_globals.kept_bytes -= MIN(size, mods_globals.kept_bytes);

	return;
}

char const *mods_map_name(
	void)
{
	return mods_globals.map_name;
}

boolean mods_map_loaded(
	void)
{
	return mods_globals.map_loaded;
}

/* (scenario_tags_load, once a map's tags are loaded) */
void mods_tags_loaded(
	char const *map_name)
{
	/* (a map whose tags loaded, but not the rest of it, is not unloaded
	before the next map's tags load over its own: what the mods made for it
	goes, and nothing of it is written over the new map's tags) */
	if (mods_globals.map_loaded)
		mods_let_go(FALSE);
	/* MCC owns its converted tags. Leave mods inactive, including console
	and menu reapplication, until an Xbox or Custom Edition map loads. */
	if (mcc_level_name(map_name))
		return;
	mods_globals.map_loaded = TRUE;
	mods_globals.enabled = TRUE;
	csstrncpy(mods_globals.map_name, map_name ? map_name : "", sizeof(mods_globals.map_name) - 1);
	mods_globals.map_name[sizeof(mods_globals.map_name) - 1] = 0;
	/* (found again, unless the menus of this map just found them: the
	first map's, menu_tags.c) */
	if (port_mods_scans() == mods_globals.scans)
		port_mods_scan();
	mods_globals.scans = port_mods_scans();
	if (port_mods_played())
	{
		mods_update(FALSE, TRUE);
	}

	return;
}

/* Every group lets go of what it made for the map; the map's models get
their own definitions back if `give_back` (their tags are still there). */
static void mods_let_go(
	boolean give_back)
{
	mod_sounds_tags_unloaded();
	mod_bitmaps_tags_unloaded();
	mod_models_tags_unloaded(give_back);
	mod_shaders_tags_unloaded(give_back);
	mod_strings_tags_unloaded();
	mods_globals.map_loaded = FALSE;
	mods_globals.enabled = FALSE;
	mods_globals.map_name[0] = 0;
	mods_globals.kept_bytes = 0;
	mods_globals.string_count = 0;
	mods_globals.bitmap_count = 0;
	mods_globals.sound_count = 0;
	mods_globals.model_count = 0;
	mods_globals.shader_count = 0;

	return;
}

/* (scenario_tags_unload, once the sound and texture caches are closed: no
cache block holds a mod's permutation or bitmap; nothing draws a model) */
void mods_tags_unloaded(
	void)
{
	mods_let_go(TRUE);

	return;
}

boolean mods_say(
	void)
{
	return mods_globals.say;
}

/* (the script function mods_reload) */
void mods_reload(
	void)
{
	if (!mods_globals.map_loaded)
	{
		return;
	}
	if (!mods_globals.enabled)
	{
		console_warning("the mods are off until mods_enable true, or the map changes");
		return;
	}
	console_printf(FALSE, "mods: %ld played", port_mods_scan());
	mods_globals.scans = port_mods_scans();
	mods_globals.say = TRUE;
	mods_update(FALSE, FALSE);
	mods_globals.say = FALSE;

	return;
}

/* (the Mods menu's OK, menu_functions.c) The mods found again and played
on the map in their new order, quietly. */
void mods_apply(
	void)
{
	if (!mods_globals.map_loaded)
	{
		port_mods_scan();
		return;
	}
	port_mods_scan();
	mods_globals.scans = port_mods_scans();
	if (mods_globals.enabled)
	{
		mods_update(FALSE, FALSE);
	}

	return;
}

/* (the script function mods_enable) Plays the mods' tags, or gives the
map's back, until the map changes. */
void mods_enable(
	boolean enabled)
{
	if (!mods_globals.map_loaded || enabled == mods_globals.enabled)
	{
		return;
	}
	mods_globals.enabled = enabled;
	if (enabled)
	{
		port_mods_scan();
		mods_globals.scans = port_mods_scans();
	}
	mods_globals.say = TRUE;
	mods_update(!enabled, FALSE);
	mods_globals.say = FALSE;

	return;
}

/* (the script function mods_list) */
void mods_list(
	void)
{
	long count = port_mods_count();
	long index;

	if (!count)
	{
		console_printf(FALSE, "mods: none in the mods folder");
	}
	/* (in their load order) */
	for (index = 0; index < count; index++)
	{
		char text[512];

		if (port_mods_describe(port_mods_at(index), text, sizeof(text), mods_globals.map_name))
		{
			console_printf(FALSE, "%s", text);
		}
	}
	if (mods_globals.map_loaded)
	{
		if (mods_globals.enabled)
		{
			console_printf(FALSE,
				"mods: on %s, %ld string lists, %ld bitmaps, %ld sounds, %ld models and %ld shaders are theirs (%lu KB kept)",
				mods_globals.map_name, mods_globals.string_count, mods_globals.bitmap_count, mods_globals.sound_count,
				mods_globals.model_count, mods_globals.shader_count, mods_globals.kept_bytes / 1024);
		}
		else
		{
			console_printf(FALSE, "mods: off on %s until mods_enable true", mods_globals.map_name);
		}
	}

	return;
}

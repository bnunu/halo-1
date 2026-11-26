/*
MOD_SHADERS.C

The shaders of mods (mods.c): each of a map's model and environment shader
tags that a mod has a tag file of (tags\...\name.shader_model or
tags\...\name.shader_environment, Halo PC's, as the Halo Editing Kit and
Invader write them) is drawn from that file instead of the map, and
mods_reload reads them again. The other shader groups are the map's.

A shader is drawn, not simulated, but for its material type, which says how
projectiles, footsteps and impacts meet what it covers: that is the map's,
as are its physics flags and its type (the group's; the rasterizer reads a
shader as the structure of its type). Everything else Halo PC's tag has is
the file's; what Halo PC's has not got, where its tag is padding, is the
map's. A mod cannot add tags: every bitmap (and an environment's lens flare)
the file names must be one the map has, of the kind the field draws (a cube
map where a reflection is drawn, a 2D texture elsewhere); a mod's bitmaps can
redraw them.

A tag file is big-endian: the shader, then the names of its references in
order (each its length's characters and a terminator; an empty reference has
none), and nothing after. Every field is read by its width (shader_fields),
and checked as the schema checks a map's (tag_schema_render.c): an enum
within its values, a real finite, a reference of its group in the map. What
the tool works out as it builds a map is worked out as it does (Invader's
pre_compile): an environment's bump map scale for x and y, from its bump map
scale; a material color of black, white; a model's map scales and reflection
distances, as the tool sets them.

The map's shader tag takes the shader made in place, so everything that holds
the tag keeps holding it, and its own comes back when the file goes.
*/

/* ---------- headers */

#include "cseries.h"
#include "errors.h"
#include "bitmaps/bitmap_group.h"
#include "cache/cache_files.h"
#include "main/console.h"
#include "models/models.h"
#include "objects/objects.h"
#include "tag_files/tag_groups.h"
#include "mods.h"

#include <stdlib.h>
#include <string.h>

/* the game's (port) */
char *tag_get_name(long tag_index);
unsigned long tag_get_group_tag(long tag_index);

/* ---------- constants */

#define SHADER_MODEL_GROUP_TAG 'soso'
#define SHADER_ENVIRONMENT_GROUP_TAG 'senv'
#define BITMAP_GROUP_TAG 'bitm'
#define LENS_FLARE_GROUP_TAG 'lens'

enum
{
	/* both groups' tag files (Invader's header.cpp) */
	SHADER_FILE_VERSION = 2,

	SHADER_MODEL_SIZE = 0x1B8,
	SHADER_ENVIRONMENT_SIZE = 0x344,
	MAXIMUM_SHADER_SIZE = SHADER_ENVIRONMENT_SIZE,

	/* a tag reference: its group, its name's pointer, its name's length and
	its tag's index */
	REFERENCE_SIZE = 0x10,
	REFERENCE_NAME_LENGTH_OFFSET = 0x08,
	/* (the tool's names are at most this long) */
	MAXIMUM_REFERENCE_NAME_LENGTH = 255,

	/* the shader's own (struct shader): the map's */
	SHADER_PHYSICS_FLAGS_OFFSET = 0x20,
	SHADER_MATERIAL_TYPE_OFFSET = 0x22,
	SHADER_TYPE_OFFSET = 0x24,

	/* the enums' values (tag_schema_render.c's, the game's) */
	NUMBER_OF_SHADER_DETAIL_LEVELS = 4,
	NUMBER_OF_PERIODIC_FUNCTIONS = 12,
	NUMBER_OF_SHADER_DETAIL_FUNCTIONS = 3,
	NUMBER_OF_SHADER_MODEL_DETAIL_MASKS = 9,
	NUMBER_OF_SHADER_ENVIRONMENT_TYPES = 3,
	NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES = 3,
	NUMBER_OF_SHADER_ANIMATION_SOURCES = NUMBER_OF_OBJECT_FUNCTION_REFERENCES,

	/* a bitmap group's type (bitmap_group.type) */
	_bitmap_group_type_2d_textures = 0,
	_bitmap_group_type_cube_maps = 2,

	/* what the tool works out */
	SHADER_MODEL_MAP_U_SCALE_OFFSET = 0x9C,
	SHADER_MODEL_MAP_V_SCALE_OFFSET = 0xA0,
	SHADER_MODEL_REFLECTION_FALLOFF_OFFSET = 0x13C,
	SHADER_MODEL_REFLECTION_CUTOFF_OFFSET = 0x140,
	SHADER_MODEL_UNKNOWN_OFFSET = 0x184,
	SHADER_ENVIRONMENT_BASE_MAP_OFFSET = 0x88,
	SHADER_ENVIRONMENT_MATERIAL_COLOR_OFFSET = 0x10C,
	SHADER_ENVIRONMENT_BUMP_MAP_SCALE_OFFSET = 0x124,
	SHADER_ENVIRONMENT_BUMP_MAP_SCALE_XY_OFFSET = 0x138,
};

/* what a field of Halo PC's tag is, as it is read */
enum shader_field_kind
{
	/* flags, or a value the renderer checks as it reads it: 2 bytes */
	_shader_field_word,
	/* an enum of `maximum` values: 2 bytes */
	_shader_field_enum,
	/* `count` reals, each finite */
	_shader_field_real,
	/* a reference to a tag of `group` (a bitmap of `bitmap_type`) */
	_shader_field_reference,
	NUMBER_OF_SHADER_FIELD_KINDS
};

/* ---------- definitions */

struct shader_field
{
	unsigned short offset;
	byte kind;
	byte count;
	short maximum;
	short bitmap_type;
	unsigned long group;
};

struct shader_group
{
	unsigned long group_tag;
	char const *extension;
	unsigned long size;
	struct shader_field const *fields;
	long field_count;
};

/* a shader made from a mod's file, played or not */
struct mod_shader
{
	struct mod_shader *next;
	long tag_index;
	long file;
	char path[512];
	unsigned long file_size;
	unsigned long stamp[2];
	unsigned long size;
	byte definition[MAXIMUM_SHADER_SIZE];
};

/* a map's shader tag: its own definition while a mod's is played */
struct mod_shader_override
{
	struct mod_shader *shader;
	byte map_definition[MAXIMUM_SHADER_SIZE];
};

/* ---------- macros */

#define WORD(offset) { (offset), _shader_field_word, 1, 0, 0, 0 }
#define ENUM(offset, maximum) { (offset), _shader_field_enum, 1, (maximum), 0, 0 }
#define REALS(offset, count) { (offset), _shader_field_real, (count), 0, 0, 0 }
#define BITMAP(offset, type) { (offset), _shader_field_reference, 1, 0, (type), BITMAP_GROUP_TAG }
#define REFERENCE(offset, group) { (offset), _shader_field_reference, 1, 0, 0, (group) }

/* ---------- globals */

/* the fields of Halo PC's tags (Invader's definitions), in order: the
shader's own first (its physics flags, material type and type, at 0x20 to
0x26, are the map's) */
static struct shader_field const shader_model_fields[] =
{
	WORD(0x00), ENUM(0x02, NUMBER_OF_SHADER_DETAIL_LEVELS), REALS(0x04, 1), REALS(0x08, 3), REALS(0x14, 3),
	WORD(0x28), REALS(0x38, 1),
	/* (the change color and self-illumination color sources: the renderer
	checks them as it reads them) */
	WORD(0x4C), WORD(0x6C), WORD(0x70),
	ENUM(0x72, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x74, 1), REALS(0x78, 3), REALS(0x84, 3),
	REALS(0x9C, 2),
	BITMAP(0xA4, _bitmap_group_type_2d_textures), BITMAP(0xBC, _bitmap_group_type_2d_textures),
	ENUM(0xD4, NUMBER_OF_SHADER_DETAIL_FUNCTIONS), ENUM(0xD6, NUMBER_OF_SHADER_MODEL_DETAIL_MASKS), REALS(0xD8, 1),
	BITMAP(0xDC, _bitmap_group_type_2d_textures), REALS(0xEC, 1),
	ENUM(0xFC, NUMBER_OF_SHADER_ANIMATION_SOURCES), ENUM(0xFE, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x100, 3),
	ENUM(0x10C, NUMBER_OF_SHADER_ANIMATION_SOURCES), ENUM(0x10E, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x110, 3),
	ENUM(0x11C, NUMBER_OF_SHADER_ANIMATION_SOURCES), ENUM(0x11E, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x120, 3),
	REALS(0x12C, 2),
	REALS(0x13C, 3), REALS(0x148, 3), REALS(0x154, 1), REALS(0x158, 3),
	BITMAP(0x164, _bitmap_group_type_cube_maps),
};

static struct shader_field const shader_environment_fields[] =
{
	WORD(0x00), ENUM(0x02, NUMBER_OF_SHADER_DETAIL_LEVELS), REALS(0x04, 1), REALS(0x08, 3), REALS(0x14, 3),
	WORD(0x28), ENUM(0x2A, NUMBER_OF_SHADER_ENVIRONMENT_TYPES), REALS(0x2C, 1), REFERENCE(0x30, LENS_FLARE_GROUP_TAG),
	WORD(0x6C),
	BITMAP(0x88, _bitmap_group_type_2d_textures),
	ENUM(0xB0, NUMBER_OF_SHADER_DETAIL_FUNCTIONS), REALS(0xB4, 1), BITMAP(0xB8, _bitmap_group_type_2d_textures),
	REALS(0xC8, 1), BITMAP(0xCC, _bitmap_group_type_2d_textures),
	ENUM(0xF4, NUMBER_OF_SHADER_DETAIL_FUNCTIONS), REALS(0xF8, 1), BITMAP(0xFC, _bitmap_group_type_2d_textures),
	REALS(0x10C, 3), REALS(0x124, 1), BITMAP(0x128, _bitmap_group_type_2d_textures),
	ENUM(0x150, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x154, 2),
	ENUM(0x15C, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x160, 2),
	WORD(0x180),
	REALS(0x19C, 3), REALS(0x1A8, 3), ENUM(0x1B4, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x1B8, 2),
	REALS(0x1D8, 3), REALS(0x1E4, 3), ENUM(0x1F0, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x1F4, 2),
	REALS(0x214, 3), REALS(0x220, 3), ENUM(0x22C, NUMBER_OF_PERIODIC_FUNCTIONS), REALS(0x230, 2),
	REALS(0x250, 1), BITMAP(0x254, _bitmap_group_type_2d_textures),
	WORD(0x27C), REALS(0x290, 1), REALS(0x2A8, 3), REALS(0x2B4, 3),
	WORD(0x2D0), ENUM(0x2D2, NUMBER_OF_SHADER_ENVIRONMENT_REFLECTION_TYPES), REALS(0x2D4, 1),
	REALS(0x2F4, 2),
	BITMAP(0x324, _bitmap_group_type_cube_maps),
};

static struct shader_group const shader_groups[] =
{
	{ SHADER_MODEL_GROUP_TAG, ".shader_model", SHADER_MODEL_SIZE, shader_model_fields, NUMBEROF(shader_model_fields) },
	{ SHADER_ENVIRONMENT_GROUP_TAG, ".shader_environment", SHADER_ENVIRONMENT_SIZE, shader_environment_fields,
		NUMBEROF(shader_environment_fields) },
};

static struct
{
	struct mod_shader *shaders;
	struct mod_shader_override *overrides;
	long override_count;
} mod_shaders_globals;

/* ---------- private code */

static void write_word(
	byte *definition,
	unsigned long offset,
	unsigned short value)
{
	memcpy(definition + offset, &value, sizeof(value));
}

static void write_real(
	byte *definition,
	unsigned long offset,
	real value)
{
	memcpy(definition + offset, &value, sizeof(value));
}

static real read_definition_real(
	byte const *definition,
	unsigned long offset)
{
	real value;

	memcpy(&value, definition + offset, sizeof(value));

	return value;
}

/* The map's tag a reference of the file names: *index the tag's (NONE for
none), FALSE (with *reason) when it is not one the map has, of the field's
group and kind. *cursor is where the reference's name is in the file, and
passes it. */
static boolean reference_read(
	struct shader_field const *field,
	byte const *file,
	unsigned long size,
	byte const *reference,
	unsigned long *cursor,
	long *index,
	char const **reason)
{
	unsigned long length = mods_read_be32(reference + REFERENCE_NAME_LENGTH_OFFSET);
	char const *name = (char const *)file + *cursor;

	*index = NONE;
	if (!length)
		return TRUE;
	if (length > MAXIMUM_REFERENCE_NAME_LENGTH || !mods_in_file(*cursor, length + 1, size) || file[*cursor + length] ||
		memchr(name, 0, length))
	{
		*reason = "a reference's name is not in the file";
		return FALSE;
	}
	*cursor += length + 1;
	*index = tag_loaded((long)field->group, name);
	if (*index == NONE)
	{
		*reason = field->group == BITMAP_GROUP_TAG ?
			"it names a bitmap the map has not got (a mod cannot add tags)" :
			"it names a lens flare the map has not got (a mod cannot add tags)";
		return FALSE;
	}
	if (field->group == BITMAP_GROUP_TAG)
	{
		struct bitmap_group const *bitmap = tag_get(BITMAP_GROUP_TAG, *index);

		if (!bitmap || bitmap->type != field->bitmap_type)
		{
			*reason = field->bitmap_type == _bitmap_group_type_cube_maps ?
				"a reflection's bitmap is not a cube map" : "a map's bitmap is not a 2D texture";
			return FALSE;
		}
	}

	return TRUE;
}

/* What the tool works out as it builds a map (Invader's pre_compile). */
static void shader_tool_values(
	unsigned long group_tag,
	byte *definition)
{
	if (group_tag == SHADER_ENVIRONMENT_GROUP_TAG)
	{
		real scale = read_definition_real(definition, SHADER_ENVIRONMENT_BUMP_MAP_SCALE_OFFSET);
		long channel;
		boolean black = TRUE;

		write_real(definition, SHADER_ENVIRONMENT_BUMP_MAP_SCALE_XY_OFFSET, scale);
		write_real(definition, SHADER_ENVIRONMENT_BUMP_MAP_SCALE_XY_OFFSET + 4, scale);
		for (channel = 0; channel < 3; channel++)
			black &= read_definition_real(definition, SHADER_ENVIRONMENT_MATERIAL_COLOR_OFFSET + 4 * channel) == 0.0f;
		for (channel = 0; black && channel < 3; channel++)
			write_real(definition, SHADER_ENVIRONMENT_MATERIAL_COLOR_OFFSET + 4 * channel, 1.0f);
	}
	else
	{
		real u_scale = read_definition_real(definition, SHADER_MODEL_MAP_U_SCALE_OFFSET);
		real v_scale = read_definition_real(definition, SHADER_MODEL_MAP_V_SCALE_OFFSET);
		real falloff = read_definition_real(definition, SHADER_MODEL_REFLECTION_FALLOFF_OFFSET);
		real cutoff = read_definition_real(definition, SHADER_MODEL_REFLECTION_CUTOFF_OFFSET);

		write_real(definition, SHADER_MODEL_UNKNOWN_OFFSET, 1.0f);
		if (u_scale == 0.0f && v_scale == 0.0f)
			u_scale = v_scale = 1.0f;
		else if (u_scale == 0.0f)
			u_scale = v_scale;
		else if (v_scale == 0.0f)
			v_scale = u_scale;
		write_real(definition, SHADER_MODEL_MAP_U_SCALE_OFFSET, u_scale);
		write_real(definition, SHADER_MODEL_MAP_V_SCALE_OFFSET, v_scale);
		if (falloff >= cutoff && cutoff != 0.0f && falloff != 0.0f)
		{
			write_real(definition, SHADER_MODEL_REFLECTION_FALLOFF_OFFSET, 0.0f);
			write_real(definition, SHADER_MODEL_REFLECTION_CUTOFF_OFFSET, 0.0f);
		}
	}
}

/* Makes the shader of a group from a tag file of `size` bytes, over a copy
of the map's (`map_definition`): FALSE (with *reason) when the file is not
one the reader can trust, or names what the map has not got. */
static boolean shader_make(
	struct shader_group const *group,
	byte const *file,
	unsigned long size,
	byte const *map_definition,
	byte *definition,
	char const **reason)
{
	byte const *bytes = file + MOD_TAG_FILE_HEADER_SIZE;
	unsigned long cursor = MOD_TAG_FILE_HEADER_SIZE + group->size;
	long index;

	*reason = "it is not a tag file of its group (or of another version)";
	if (size < MOD_TAG_FILE_HEADER_SIZE || !mods_tag_file_header_valid(file, group->group_tag, SHADER_FILE_VERSION))
		return FALSE;
	*reason = "it ends before its shader does";
	if (!mods_in_file(MOD_TAG_FILE_HEADER_SIZE, group->size, size))
		return FALSE;
	memcpy(definition, map_definition, group->size);
	for (index = 0; index < group->field_count; index++)
	{
		struct shader_field const *field = &group->fields[index];
		long element;

		switch (field->kind)
		{
		case _shader_field_word:
			write_word(definition, field->offset, (unsigned short)mods_read_be16(bytes + field->offset));
			break;
		case _shader_field_enum:
		{
			short value = mods_read_short(bytes + field->offset);

			if (value < 0 || value >= field->maximum)
			{
				*reason = "an enum is past its values";
				return FALSE;
			}
			write_word(definition, field->offset, (unsigned short)value);
			break;
		}
		case _shader_field_real:
			for (element = 0; element < field->count; element++)
			{
				real value = mods_read_real(bytes + field->offset + 4 * element);

				if (!mods_finite_real(value))
				{
					*reason = "a number is not finite";
					return FALSE;
				}
				write_real(definition, field->offset + 4 * (unsigned long)element, value);
			}
			break;
		case _shader_field_reference:
		{
			struct tag_reference reference;
			long tag_index;

			if (!reference_read(field, file, size, bytes + field->offset, &cursor, &tag_index, reason))
				return FALSE;
			csmemset(&reference, 0, sizeof(reference));
			reference.group_tag = field->group;
			reference.index = tag_index;
			reference.name = tag_index != NONE ? tag_get_name(tag_index) : NULL;
			reference.name_length = reference.name ? (long)strlen(reference.name) : 0;
			memcpy(definition + field->offset, &reference, sizeof(reference));
			break;
		}
		}
	}
	*reason = "the file goes on past its last reference's name";
	if (cursor != size)
		return FALSE;
	/* (lights take a lightmap's permutation modulo its base map's bitmaps:
	object_lights.c, tag_schema_render.c's shader_environment_check) */
	if (group->group_tag == SHADER_ENVIRONMENT_GROUP_TAG)
	{
		struct tag_reference base_map;
		struct bitmap_group const *bitmap;

		memcpy(&base_map, definition + SHADER_ENVIRONMENT_BASE_MAP_OFFSET, sizeof(base_map));
		bitmap = base_map.index != NONE ? tag_get(BITMAP_GROUP_TAG, base_map.index) : NULL;
		if (bitmap && bitmap->bitmaps.count <= 0)
		{
			*reason = "its base map has no bitmaps";
			return FALSE;
		}
	}
	shader_tool_values(group->group_tag, definition);
	/* (the map's: its material is simulated, and its type is its group's) */
	memcpy(definition + SHADER_PHYSICS_FLAGS_OFFSET, map_definition + SHADER_PHYSICS_FLAGS_OFFSET,
		SHADER_TYPE_OFFSET + 2 - SHADER_PHYSICS_FLAGS_OFFSET);

	return TRUE;
}

static struct mod_shader_override *override_get(
	long tag_index)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(tag_index);

	return absolute_index >= 0 && absolute_index < mod_shaders_globals.override_count ?
		&mod_shaders_globals.overrides[absolute_index] : NULL;
}

/* Makes the table of overrides, one for each of the map's tags up to its
last shader of the groups read; FALSE when it cannot be. */
static boolean overrides_new(
	void)
{
	long count = 0;
	long group;

	if (mod_shaders_globals.overrides)
		return TRUE;
	for (group = 0; group < (long)NUMBEROF(shader_groups); group++)
	{
		struct tag_iterator iterator;
		long tag_index;

		tag_iterator_new(&iterator, shader_groups[group].group_tag);
		while ((tag_index = tag_iterator_next(&iterator)) != NONE)
			count = MAX(count, DATUM_INDEX_TO_ABSOLUTE_INDEX(tag_index) + 1);
	}
	if (count <= 0 || !(mod_shaders_globals.overrides = malloc((size_t)count * sizeof(struct mod_shader_override))))
		return FALSE;
	memset(mod_shaders_globals.overrides, 0, (size_t)count * sizeof(struct mod_shader_override));
	mod_shaders_globals.override_count = count;

	return TRUE;
}

/* Gives the map's shader tag its own definition back. */
static void shader_unplug(
	long tag_index,
	struct mod_shader_override *override)
{
	memcpy(tag_get(tag_get_group_tag(tag_index), tag_index), override->map_definition, override->shader->size);
	override->shader = NULL;

	return;
}

/* Draws the shader made `made` for the map's shader tag. */
static void shader_plug(
	long tag_index,
	struct mod_shader_override *override,
	struct mod_shader *made)
{
	byte *definition = tag_get(tag_get_group_tag(tag_index), tag_index);

	memcpy(override->map_definition, definition, made->size);
	memcpy(definition, made->definition, made->size);
	override->shader = made;

	return;
}

/* a shader made before for the tag from the file at `path` as it is now,
played or not, or NULL */
static struct mod_shader *shader_kept(
	long tag_index,
	char const *path,
	unsigned long size,
	unsigned long const stamp[2])
{
	struct mod_shader *made;

	for (made = mod_shaders_globals.shaders; made; made = made->next)
	{
		if (made->tag_index == tag_index && made->file_size == size && made->stamp[0] == stamp[0] &&
			made->stamp[1] == stamp[1] && !strcmp(made->path, path))
		{
			return made;
		}
	}

	return NULL;
}

/* Makes the shader of the map's tag from a mod's file, over the map's own
definition; NULL (with *reason) when it cannot be. */
static struct mod_shader *mod_shader_new(
	struct shader_group const *group,
	long tag_index,
	long file,
	byte const *map_definition,
	char const **reason)
{
	struct mod_shader *made = NULL;
	unsigned long size;
	unsigned long stamp[2];
	byte *bytes;

	if (!port_mods_file_stamp(file, &size, stamp) || !(bytes = mods_tag_file_read(file, &size, reason)))
	{
		*reason = *reason ? *reason : "it cannot be read";
		return NULL;
	}
	made = malloc(sizeof(*made));
	if (!made)
	{
		*reason = "out of memory";
	}
	else
	{
		csmemset(made, 0, sizeof(*made));
		if (!shader_make(group, bytes, size, map_definition, made->definition, reason))
		{
			free(made);
			made = NULL;
		}
		else
		{
			made->tag_index = tag_index;
			made->file = file;
			made->size = group->size;
			made->file_size = size;
			made->stamp[0] = stamp[0];
			made->stamp[1] = stamp[1];
			csstrncpy(made->path, port_mods_file_path(file), sizeof(made->path) - 1);
		}
	}
	free(bytes);

	return made;
}

/* ---------- public code */

long mod_shaders_update(
	boolean remove_all,
	boolean at_load)
{
	long loaded_count = 0;
	long removed_count = 0;
	long refused_count = 0;
	long played_count = 0;
	boolean changed = FALSE;
	long group;
	long index;

	if (!mods_map_loaded() || !overrides_new())
		return 0;
	for (group = 0; group < (long)NUMBEROF(shader_groups); group++)
	{
		struct shader_group const *shader_group = &shader_groups[group];
		struct tag_iterator iterator;
		long tag_index;

		tag_iterator_new(&iterator, shader_group->group_tag);
		while ((tag_index = tag_iterator_next(&iterator)) != NONE)
		{
			struct mod_shader_override *override = override_get(tag_index);
			byte *definition = tag_get(shader_group->group_tag, tag_index);
			struct mod_shader *made = NULL;
			long file = remove_all ? NONE : mods_tag_file_find(tag_index, shader_group->extension);
			char const *reason = NULL;

			if (!override || !definition)
				continue;
			if (file != NONE)
			{
				unsigned long size;
				unsigned long stamp[2];

				/* (one made before from the file as it is: only where it is in
				the mods' files changes) */
				if (port_mods_file_stamp(file, &size, stamp))
					made = shader_kept(tag_index, port_mods_file_path(file), size, stamp);
				if (made)
					made->file = file;
				if (made && made == override->shader)
					continue;
			}
			if (file != NONE && !made)
			{
				made = mod_shader_new(shader_group, tag_index, file,
					override->shader ? override->map_definition : definition, &reason);
				if (made && !mods_memory_take(sizeof(*made)))
				{
					reason = "the mods' tags already hold too much memory";
					free(made);
					made = NULL;
				}
				/* (one that changed and now fails a check leaves what was played
				before, as the other groups' files do) */
				if (!made)
				{
					error(_error_silent, "mods: %s left out: %s", port_mods_file_path(file), reason);
					console_warning("mods: the shader %s of %s left out: %s", tag_get_name(tag_index),
						port_mods_file_mod(file), reason);
					refused_count++;
					continue;
				}
				made->next = mod_shaders_globals.shaders;
				mod_shaders_globals.shaders = made;
				loaded_count++;
			}
			if (!made && !override->shader)
				continue;
			if (override->shader)
			{
				shader_unplug(tag_index, override);
				if (!made)
					removed_count++;
			}
			if (made)
				shader_plug(tag_index, override, made);
			changed = TRUE;
		}
	}
	for (index = 0; index < mod_shaders_globals.override_count; index++)
	{
		if (mod_shaders_globals.overrides[index].shader)
			played_count++;
	}
	/* (a model's parts are linked by their shaders' transparency as its render
	starts: render_initialize_for_new_map) */
	if (changed && !at_load)
		models_fix_transparent_part_links();
	if (!at_load || loaded_count || refused_count)
	{
		error(_error_silent, "mods: %ld shaders from their files (%ld new), %ld given back, %ld left out",
			played_count, loaded_count, removed_count, refused_count);
		if (!at_load && mods_say())
		{
			console_printf(FALSE, "mods: %ld shaders from their files (%ld new), %ld given back, %ld left out",
				played_count, loaded_count, removed_count, refused_count);
		}
	}

	return played_count;
}

/* (before the tags go: the map's shaders get their own definitions back,
unless they are gone already) */
void mod_shaders_tags_unloaded(
	boolean give_back)
{
	struct mod_shader *made = mod_shaders_globals.shaders;
	long index;

	for (index = 0; give_back && index < mod_shaders_globals.override_count; index++)
	{
		struct mod_shader_override *override = &mod_shaders_globals.overrides[index];

		if (override->shader)
			shader_unplug(override->shader->tag_index, override);
	}
	while (made)
	{
		struct mod_shader *next = made->next;

		free(made);
		made = next;
	}
	if (mod_shaders_globals.overrides)
		free(mod_shaders_globals.overrides);
	memset(&mod_shaders_globals, 0, sizeof(mod_shaders_globals));

	return;
}

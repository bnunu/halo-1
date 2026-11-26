#include "harness.h"

/* The shaders of mods (port/linux/game/mod_shaders.c, shader_make): Halo PC
tag files of model and environment shaders made into the game's shaders over
the map's, every field read by its width, checked as the schema checks a
map's, and what the tool works out worked out as it does. */

#define NUMBEROF(array) ((long)(sizeof(array) / sizeof((array)[0])))
#define csmemset memset

struct tag_block
{
	long count;
	void *address;
	void *definition;
};

struct tag_reference
{
	unsigned long group_tag;
	char *name;
	long name_length;
	long index;
};

struct bitmap_group
{
	short type;
	struct tag_block bitmaps;
};

/* the map's tags: their names, groups and (for bitmaps) kinds */
static struct
{
	char const *name;
	unsigned long group;
	short type;
	long bitmap_count;
} map_tags[] =
{
	{ "weapons\\rifle\\base", 'bitm', 0, 1 },
	{ "weapons\\rifle\\multipurpose", 'bitm', 0, 1 },
	{ "levels\\cube", 'bitm', 2, 1 },
	{ "levels\\ground", 'bitm', 0, 1 },
	{ "levels\\empty", 'bitm', 0, 0 },
	{ "levels\\flare", 'lens', 0, 0 },
};
static struct bitmap_group bitmaps[NUMBEROF(map_tags)];

static long tag_loaded(long group, char const *name)
{
	long index;

	for (index = 0; index < NUMBEROF(map_tags); index++)
	{
		if ((unsigned long)group == map_tags[index].group && !strcmp(name, map_tags[index].name))
			return 0x10000 + index;
	}
	return NONE;
}

static void *tag_get(unsigned long group, long tag_index)
{
	long index = tag_index - 0x10000;

	if (group != 'bitm' || index < 0 || index >= NUMBEROF(map_tags))
		return NULL;
	bitmaps[index].type = map_tags[index].type;
	bitmaps[index].bitmaps.count = map_tags[index].bitmap_count;
	return &bitmaps[index];
}

static char *tag_get_name(long tag_index)
{
	return (char *)map_tags[tag_index - 0x10000].name;
}

#include "config.inc"
#include "under_test.inc"

/* ---------- a Halo PC tag file, written as the tool writes one */

static byte file[0x1000];
static unsigned long file_size;

static void put16(unsigned long offset, unsigned long value)
{
	file[offset] = (byte)(value >> 8);
	file[offset + 1] = (byte)value;
}

static void put32(unsigned long offset, unsigned long value)
{
	file[offset] = (byte)(value >> 24);
	file[offset + 1] = (byte)(value >> 16);
	file[offset + 2] = (byte)(value >> 8);
	file[offset + 3] = (byte)value;
}

static void put_real(unsigned long offset, real value)
{
	unsigned long bits;

	memcpy(&bits, &value, sizeof(bits));
	put32(offset, bits);
}

/* a shader of the group, its structure all zeroes */
static void file_begin(unsigned long group, unsigned long size)
{
	memset(file, 0, sizeof(file));
	put32(MOD_TAG_FILE_GROUP_OFFSET, group);
	put32(MOD_TAG_FILE_HEADER_SIZE_OFFSET, MOD_TAG_FILE_HEADER_SIZE);
	put16(MOD_TAG_FILE_VERSION_OFFSET, 2);
	memcpy(file + MOD_TAG_FILE_SIGNATURE_OFFSET, "blam", 4);
	file_size = MOD_TAG_FILE_HEADER_SIZE + size;
}

/* a field of the shader (an offset in its structure) */
static void field16(unsigned long offset, unsigned long value) { put16(MOD_TAG_FILE_HEADER_SIZE + offset, value); }
static void field_real(unsigned long offset, real value) { put_real(MOD_TAG_FILE_HEADER_SIZE + offset, value); }

/* a reference, its name after the names before it (in their fields' order) */
static void reference(unsigned long offset, unsigned long group, char const *name)
{
	unsigned long length = (unsigned long)strlen(name);

	put32(MOD_TAG_FILE_HEADER_SIZE + offset, group);
	put32(MOD_TAG_FILE_HEADER_SIZE + offset + 8, length);
	put32(MOD_TAG_FILE_HEADER_SIZE + offset + 12, 0xFFFFFFFF);
	memcpy(file + file_size, name, length + 1);
	file_size += length + 1;
}

/* ---------- the shader made */

static byte map_definition[MAXIMUM_SHADER_SIZE];
static byte made[MAXIMUM_SHADER_SIZE];

static unsigned short made16(unsigned long offset)
{
	unsigned short value;

	memcpy(&value, made + offset, sizeof(value));
	return value;
}

static real made_real(unsigned long offset)
{
	real value;

	memcpy(&value, made + offset, sizeof(value));
	return value;
}

static struct tag_reference made_reference(unsigned long offset)
{
	struct tag_reference value;

	memcpy(&value, made + offset, sizeof(value));
	return value;
}

/* the map's own shader: a pattern, its material and type set */
static void map_shader(unsigned short type)
{
	unsigned short material = 7;

	memset(map_definition, 0xAB, sizeof(map_definition));
	memcpy(map_definition + 0x22, &material, sizeof(material));
	memcpy(map_definition + 0x24, &type, sizeof(type));
}

static boolean make(struct shader_group const *group, char const **reason)
{
	memset(made, 0, sizeof(made));
	return shader_make(group, file, file_size, map_definition, made, reason);
}

/* a model shader's file, as a mod's would be */
static void model_file(void)
{
	file_begin('soso', SHADER_MODEL_SIZE);
	field16(0x22, 3);                 /* (its material: the map's is played) */
	field16(0x24, 9);                 /* (its type: the map's is played) */
	field16(0x28, 0x0003);
	field_real(0x38, 0.5f);
	field16(0xD4, 2);
	field16(0xD6, 5);
	field16(0xFC, 1);
	field16(0xFE, 11);
	field_real(0x13C, 2.0f);          /* (a falloff past its cutoff: both none) */
	field_real(0x140, 1.0f);
	reference(0xA4, 'bitm', "weapons\\rifle\\base");
	reference(0xBC, 'bitm', "weapons\\rifle\\multipurpose");
	reference(0x164, 'bitm', "levels\\cube");
}

static void environment_file(void)
{
	file_begin('senv', SHADER_ENVIRONMENT_SIZE);
	field16(0x2A, 1);
	field_real(0x124, 2.0f);
	field16(0x2D2, 2);
	reference(0x30, 'lens', "levels\\flare");
	reference(0x88, 'bitm', "levels\\ground");
}

static void check_refused(struct shader_group const *group, char const *what)
{
	char const *reason = NULL;

	CHECK(!make(group, &reason), "%s was taken", what);
	CHECK(reason != NULL, "%s was refused with no reason", what);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";
	struct shader_group const *model = &shader_groups[0];
	struct shader_group const *environment = &shader_groups[1];
	char const *reason = NULL;

	CASE("model")
	{
		struct tag_reference base, detail, cube;

		map_shader(4);
		model_file();
		CHECK(make(model, &reason), "a model shader was refused: %s", reason);
		CHECK(made16(0x22) == 7 && made16(0x24) == 4, "the material or type was the file's, not the map's");
		CHECK(made16(0x28) == 0x0003, "flags read as %04x", made16(0x28));
		CHECK(made_real(0x38) == 0.5f, "translucency read as %f", made_real(0x38));
		CHECK(made16(0xD4) == 2 && made16(0xD6) == 5 && made16(0xFC) == 1 && made16(0xFE) == 11, "an enum was misread");
		CHECK(made[0x2A] == 0xAB && made[0x37] == 0xAB, "Halo PC's padding was not the map's");
		CHECK(made_real(0x9C) == 1.0f && made_real(0xA0) == 1.0f, "map scales of none were not made 1");
		CHECK(made_real(0x13C) == 0.0f && made_real(0x140) == 0.0f, "a falloff past its cutoff was kept");
		CHECK(made_real(0x184) == 1.0f, "the tool's value was not 1");
		base = made_reference(0xA4);
		detail = made_reference(0xDC);
		cube = made_reference(0x164);
		CHECK(base.group_tag == 'bitm' && base.index == 0x10000 && base.name && !strcmp(base.name, "weapons\\rifle\\base") &&
			base.name_length == (long)strlen(base.name), "the base map was not the map's bitmap");
		CHECK(detail.index == NONE, "an empty reference was not none");
		CHECK(cube.index == 0x10002, "the reflection was not the map's cube map");
	}
	else CASE("environment")
	{
		struct tag_reference flare;

		map_shader(3);
		environment_file();
		CHECK(make(environment, &reason), "an environment shader was refused: %s", reason);
		CHECK(made16(0x2A) == 1 && made16(0x2D2) == 2, "an enum was misread");
		CHECK(made_real(0x138) == 2.0f && made_real(0x13C) == 2.0f, "the bump map scale's x and y were not its scale");
		CHECK(made_real(0x10C) == 1.0f && made_real(0x110) == 1.0f && made_real(0x114) == 1.0f,
			"a black material color was not made white");
		CHECK(made[0x30C] == 0xAB && made[0x30F] == 0xAB, "the Xbox's field where Halo PC has padding was not the map's");
		flare = made_reference(0x30);
		CHECK(flare.group_tag == 'lens' && flare.index == 0x10005, "the lens flare was not the map's");
		CHECK(made16(0x22) == 7 && made16(0x24) == 3, "the material or type was the file's, not the map's");
	}
	else CASE("refused")
	{
		map_shader(4);
		model_file();
		put16(MOD_TAG_FILE_VERSION_OFFSET, 3);
		check_refused(model, "another version's file");
		model_file();
		file_size = MOD_TAG_FILE_HEADER_SIZE + SHADER_MODEL_SIZE - 1;
		check_refused(model, "a file that ends before its shader");
		model_file();
		file_size++;
		check_refused(model, "a file with a byte past its last name");
		model_file();
		field16(0xD6, 9);
		check_refused(model, "a detail mask past its values");
		model_file();
		field16(0xFC, NUMBER_OF_OBJECT_FUNCTION_REFERENCES);
		check_refused(model, "an animation source past its values");
		model_file();
		put32(MOD_TAG_FILE_HEADER_SIZE + 0x38, 0x7FC00000);
		check_refused(model, "a translucency that is not a number");
		file_begin('soso', SHADER_MODEL_SIZE);
		reference(0xA4, 'bitm', "weapons\\rifle\\missing");
		check_refused(model, "a bitmap the map has not got");
		file_begin('soso', SHADER_MODEL_SIZE);
		reference(0xA4, 'bitm', "levels\\cube");
		check_refused(model, "a cube map as a base map");
		file_begin('soso', SHADER_MODEL_SIZE);
		reference(0x164, 'bitm', "levels\\ground");
		check_refused(model, "a 2D texture as a reflection");
		file_begin('soso', SHADER_MODEL_SIZE);
		reference(0xA4, 'bitm', "weapons\\rifle\\base");
		file_size--;
		check_refused(model, "a name without its terminator");
		map_shader(3);
		file_begin('senv', SHADER_ENVIRONMENT_SIZE);
		reference(0x88, 'bitm', "levels\\empty");
		check_refused(environment, "an environment's base map of no bitmaps");
		file_begin('senv', SHADER_ENVIRONMENT_SIZE);
		field16(0x2A, 3);
		check_refused(environment, "an environment type past its values");
	}
	else
		return 2;
	return 0;
}

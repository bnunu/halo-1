#include "harness.h"
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include "limits.inc"
#ifdef _WIN32
#include <direct.h>
#define change_directory _chdir
#else
#include <unistd.h>
#define change_directory chdir
#endif

/* what the reader and the selection use of the engine */
#define _error_silent 0
static void error(int kind, char const *format, ...)
{
	(void)kind;
	(void)format;
}
static void mcc_loading_pulse(void) {}

struct mcc_resource_reference { char const *tag_name; unsigned type; uint32_t first, second, offset, bytes; };
struct mcc_cache_report { uint32_t external_bitmap_count, external_sound_count; };
struct mcc_runtime { void *resources; struct mcc_cache_report report; char level_name[64]; char failure[256]; };

/* the map's one sound: the permutation a\sound's, 8 bytes at offset 16 */
static struct mcc_resource_reference reference = { "a\\sound", 2, 0, 0, 16, 8 };

static int mcc_cache_visit_resources(void *tag_data, struct mcc_cache_report const *report,
	int (*visit)(void *, struct mcc_resource_reference const *), void *context)
{
	(void)tag_data;
	(void)report;
	return visit(context, &reference);
}

/* the package's own sounds.map, and the shared sets': the root's, singleplayer's and multiplayer's */
static char const *own_path = "own.map";
static int mcc_package_base_resource_path(unsigned type, unsigned set, char path[128])
{
	static char const *paths[] = { "root.map", "singleplayer.map", "multiplayer.map" };
	(void)type;
	if (set >= MCC_PACKAGE_BASE_SETS) return 0;
	snprintf(path, 128, "%s", paths[set]);
	return 1;
}
static int mcc_package_shared_resource(char const *path, unsigned type)
{
	char expected[128];
	unsigned set;
	for (set = 0; set < MCC_PACKAGE_BASE_SETS; set++)
		if (mcc_package_base_resource_path(type, set, expected) && !strcmp(path, expected)) return 1;
	return 0;
}
static FILE *mcc_package_resource_open(char const *name, unsigned type, char path[128])
{
	(void)name;
	(void)type;
	snprintf(path, 128, "%s", own_path);
	errno = 0;
	return fopen(path, "rb");
}

#include "under_test.inc"

static uint32_t word(unsigned char *p, uint32_t value)
{
	p[0] = (unsigned char)value;
	p[1] = (unsigned char)(value >> 8);
	p[2] = (unsigned char)(value >> 16);
	p[3] = (unsigned char)(value >> 24);
	return value;
}

/* a sounds.map holding one member: its name, 8 bytes at offset 16 that all read fill */
static void write_sounds(char const *path, char const *member, unsigned char fill)
{
	unsigned char file[256];
	uint32_t names = 16 + 40, index;
	size_t length = strlen(member) + 1;
	FILE *stream = fopen(path, "wb");

	CHECK(stream, "cannot write %s", path);
	memset(file, 0, sizeof(file));
	memset(file + 16, fill, 8);
	memcpy(file + names, member, length);
	index = names + (uint32_t)length;
	word(file, 2);
	word(file + 4, names);
	word(file + 8, index);
	word(file + 12, 1);
	word(file + index, 0);
	word(file + index + 4, 8);
	word(file + index + 8, 16);
	CHECK(fwrite(file, 1, index + 12, stream) == index + 12, "cannot write %s", path);
	fclose(stream);
}

static void clear_files(void)
{
	remove("own.map");
	remove("root.map");
	remove("singleplayer.map");
	remove("multiplayer.map");
}

/* the sounds.map selection makes for the map, or NULL when it refuses */
static char const *select_sounds(struct mcc_runtime *runtime)
{
	memset(runtime, 0, sizeof(*runtime));
	runtime->report.external_sound_count = 1;
	strcpy(runtime->level_name, "mcc_maps\\1\\level");
	if (!mcc_resources_select_paths(runtime, NULL, NULL)) return NULL;
	return ((struct mcc_resources *)runtime->resources)->files[1].path;
}

static unsigned char selected_byte(struct mcc_runtime *runtime)
{
	unsigned char out[8];
	struct mcc_resource_file *file = &((struct mcc_resources *)runtime->resources)->files[1];

	CHECK(mcc_resource_read(file, "a\\sound__0__0", 16, 8, out, "a\\sound__permutations", 0), "the selected sound");
	return out[0];
}

/* a sounds.map's members: one named sound and one group of permutations */
static char names[] = "a\\sound__0__0\0b\\group__permutations";
static struct mcc_resource_item items[] = {{0, 8, 16}, {14, 16, 24}};

static void open_fixture(struct mcc_resource_file *file, int own)
{
	unsigned char data[40];
	int i;

	for (i = 0; i < 40; i++) data[i] = (unsigned char)i;
	memset(file, 0, sizeof(*file));
	file->stream = tmpfile();
	CHECK(file->stream && fwrite(data, 1, sizeof(data), file->stream) == sizeof(data), "no fixture file");
	file->names = names;
	file->items = items;
	file->count = 2;
	file->data_bytes = 40;
	file->own = own;
}

static int check(struct mcc_resource_file *file, char const *name, char const *group, uint32_t offset, uint32_t bytes)
{
	return mcc_resource_read(file, name, offset, bytes, NULL, group, 1);
}

static int decode(struct mcc_resource_file *file, char const *name, char const *group,
	uint32_t offset, uint32_t bytes, unsigned char *out)
{
	return mcc_resource_read(file, name, offset, bytes, out, group, 0);
}

int main(int argc, char **argv)
{
	struct mcc_resource_file file;
	unsigned char out[16];
	char directory[1024], *end;

	CHECK(argc == 2, "usage: mcc_resource_position <case>");
	/* (the fixtures' files are written beside the test) */
	snprintf(directory, sizeof(directory), "%s", argv[0]);
	end = strrchr(directory, '/');
	if (!end || (strrchr(directory, '\\') && strrchr(directory, '\\') > end)) end = strrchr(directory, '\\');
	if (end)
	{
		*end = 0;
		CHECK(!change_directory(directory), "cannot work in %s", directory);
	}

	if (!strcmp(argv[1], "lookups"))
	{
		open_fixture(&file, 1);
		/* by name, and by group, as before */
		CHECK(check(&file, "a\\sound__0__0", "a\\sound__permutations", 16, 8), "a named member");
		CHECK(decode(&file, "b\\group__0__1", "b\\group__permutations", 28, 4, out) && out[0] == 28, "a group's slice");

		/* another tag's name for a whole member at its own position: that member, as MCC reads it */
		CHECK(check(&file, "c\\other__0__0", "c\\other__permutations", 16, 8), "a whole member at its position");
		memset(out, 0, sizeof(out));
		CHECK(decode(&file, "c\\other__0__0", "c\\other__permutations", 16, 8, out), "reading it");
		CHECK(out[0] == 16 && out[7] == 23, "the member's bytes");
		CHECK(check(&file, "c\\other__0__0", "c\\other__permutations", 24, 16), "a whole group at its position");

		/* but never a part of one, nor a slice that starts elsewhere */
		CHECK(!check(&file, "c\\other__0__0", "c\\other__permutations", 16, 4), "a part of a member");
		CHECK(!check(&file, "c\\other__0__0", "c\\other__permutations", 17, 8), "a slice across members");
		CHECK(!check(&file, "c\\other__0__0", "c\\other__permutations", 28, 4), "a part of a group");
		CHECK(!check(&file, "c\\other__0__0", NULL, 20, 8), "a slice of no member");
		return 0;
	}
	if (!strcmp(argv[1], "decoding"))
	{
		/* a bitmap checked whole whose decoder reads less of it (its padding left) */
		open_fixture(&file, 1);
		CHECK(check(&file, "c\\other_0", "c\\other__pixels", 24, 16), "the whole member, checked");
		memset(out, 0, sizeof(out));
		CHECK(decode(&file, "c\\other_0", "c\\other__pixels", 24, 12, out), "a shorter read from its start");
		CHECK(out[0] == 24 && out[11] == 35, "the member's first bytes");
		/* (never past it, nor from inside it) */
		CHECK(!decode(&file, "c\\other_0", "c\\other__pixels", 16, 12, out), "a read past a member");
		CHECK(!decode(&file, "c\\other_0", "c\\other__pixels", 26, 4, out), "a read from inside a member");
		return 0;
	}
	if (!strcmp(argv[1], "shared"))
	{
		struct mcc_runtime runtime;

		/* a shared file is never found by position, for a check or a read */
		open_fixture(&file, 0);
		CHECK(!check(&file, "c\\other__0__0", "c\\other__permutations", 16, 8), "a shared file's member by position");
		CHECK(!decode(&file, "c\\other__0__0", "c\\other__permutations", 16, 8, out), "reading it");
		CHECK(check(&file, "a\\sound__0__0", "a\\sound__permutations", 16, 8), "but by name, as before");

		/* the shared file named for the map is chosen, beside another with other bytes at that position */
		clear_files();
		write_sounds("singleplayer.map", "a\\sound__0__0", 0x11);
		write_sounds("multiplayer.map", "z\\unrelated__0__0", 0x22);
		CHECK(select_sounds(&runtime), "a shared file for the map: %s", runtime.failure);
		CHECK(!strcmp(((struct mcc_resources *)runtime.resources)->files[1].path, "singleplayer.map"), "the one named for it");
		CHECK(selected_byte(&runtime) == 0x11, "its bytes");
		mcc_resources_dispose(&runtime);

		/* and the other alone is no match */
		remove("singleplayer.map");
		CHECK(!select_sounds(&runtime), "an unrelated shared file chosen");
		mcc_resources_dispose(&runtime);

		/* the package's own file, its sound under another tag's name: found at its position */
		write_sounds("own.map", "z\\renamed__0__0", 0x33);
		CHECK(select_sounds(&runtime), "the package's own file: %s", runtime.failure);
		CHECK(!strcmp(((struct mcc_resources *)runtime.resources)->files[1].path, "own.map"), "the package's own");
		mcc_resources_dispose(&runtime);
		clear_files();
		return 0;
	}
	fprintf(stderr, "unknown case %s\n", argv[1]);
	return 2;
}

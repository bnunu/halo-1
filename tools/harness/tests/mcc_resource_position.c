#include "harness.h"
#include <stdint.h>

#include "under_test.inc"

/* a sounds.map's members: one named sound and one group of permutations */
static char names[] = "a\\sound__0__0\0b\\group__permutations";
static struct mcc_resource_item items[] = {{0, 8, 16}, {14, 16, 24}};

static int lookup(struct mcc_resource_file *file, char const *name, char const *group,
	uint32_t offset, uint32_t bytes, unsigned char *out)
{
	return mcc_resource_read(file, name, offset, bytes, out, group);
}

int main(int argc, char **argv)
{
	struct mcc_resource_file file;
	unsigned char data[40], out[16];
	int i;
	(void)argc;
	(void)argv;
	for (i = 0; i < 40; i++) data[i] = (unsigned char)i;
	memset(&file, 0, sizeof(file));
	file.stream = tmpfile();
	CHECK(file.stream && fwrite(data, 1, sizeof(data), file.stream) == sizeof(data), "no fixture file");
	file.names = names;
	file.items = items;
	file.count = 2;
	file.data_bytes = 40;

	/* by name, and by group, as before */
	CHECK(lookup(&file, "a\\sound__0__0", "a\\sound__permutations", 16, 8, out) && out[0] == 16, "a named member");
	CHECK(lookup(&file, "b\\group__0__1", "b\\group__permutations", 28, 4, out) && out[0] == 28, "a group's slice");

	/* another tag's name for a whole member at its own position: that member, as MCC reads it */
	memset(out, 0, sizeof(out));
	CHECK(lookup(&file, "c\\other__0__0", "c\\other__permutations", 16, 8, out), "a whole member at its position");
	CHECK(out[0] == 16 && out[7] == 23, "the member's bytes");
	CHECK(lookup(&file, "c\\other__0__0", "c\\other__permutations", 24, 16, NULL), "a whole group at its position");

	/* but never a part of one, nor a slice that starts elsewhere */
	CHECK(!lookup(&file, "c\\other__0__0", "c\\other__permutations", 16, 4, NULL), "a part of a member");
	CHECK(!lookup(&file, "c\\other__0__0", "c\\other__permutations", 17, 8, NULL), "a slice across members");
	CHECK(!lookup(&file, "c\\other__0__0", "c\\other__permutations", 28, 4, NULL), "a part of a group");
	CHECK(!lookup(&file, "c\\other__0__0", NULL, 20, 8, NULL), "a slice of no member");
	return 0;
}

#include "harness.h"
#include "constants.inc"

/* units and weapons; a weapon's parent is the unit that holds it, or none on the ground */
#define OBJECT_COUNT 8
#define OBJECT(n) (0x10000L | (n))
#define TAG_BLOCK_GET_ELEMENT(block, index, type) (&((type *)(block)->address)[index])
struct tag_block { long count; void *address; };
struct animation_graph_weapon_class { char hand_marker_name[32], grip_marker_name[32]; };
struct animation_graph_unit_seat { struct tag_block weapon_classes; };
struct animation_graph { struct tag_block unit_seats; };
struct unit_definition { struct { struct { long index; } animation_graph; } object; };
struct unit_datum
{
	long definition_index;
	struct { long parent_object_index; } object;
	struct
	{
		short current_weapon_index, desired_weapon_index;
		struct { short seat_index, weapon_index; } animation;
		long weapon_object_indices[MAXIMUM_WEAPONS_PER_UNIT];
		long weapon_last_used_at_game_time[MAXIMUM_WEAPONS_PER_UNIT];
	} unit;
};
static struct unit_datum objects[OBJECT_COUNT];
static struct animation_graph_weapon_class weapon_class = {"right hand", "primary trigger"};
static struct animation_graph_unit_seat unit_seat = {{1, &weapon_class}};
static struct animation_graph graph = {{1, &unit_seat}};
static struct unit_definition definition;
/* what was done to each object */
static boolean detached[OBJECT_COUNT], stowed[OBJECT_COUNT], readied[OBJECT_COUNT];

static struct unit_datum *object_get(long index) { return &objects[index & 0xFFFF]; }
#define unit_get object_get
static struct unit_definition *unit_definition_get(long index) { return &definition; }
static struct animation_graph *animation_graph_definition_get(long index) { return &graph; }
static boolean weapon_put_away(long index, boolean immediate) { return immediate; }
static void object_detach(long index)
{
	detached[index & 0xFFFF] = TRUE;
	object_get(index)->object.parent_object_index = NONE;
}
static void item_in_unit_inventory(long index, long unit_index) { stowed[index & 0xFFFF] = TRUE; }
static void object_attach_to_marker(long parent_index, char const *marker, long index, char const *child_marker)
{
	object_get(index)->object.parent_object_index = parent_index;
}
static void weapon_ready(long index) { readied[index & 0xFFFF] = TRUE; }
static long game_time_get(void) { return 30; }
#define object_disconnect_from_map(index) ((void)0)
#define object_activate(index) ((void)0)
#define object_set_visibility(index, visible) ((void)0)
#define object_reconnect_to_map(index, location) ((void)0)
#define unit_set_or_test_seat_and_weapon_label(unit_index, seat, weapon, set) ((void)0)
#define unit_get_seat_label(unit_index) 0
#define weapon_get_label(index) 0
#define base_weapon_label_get(type) 0
#define unit_unzoom(unit_index) ((void)0)

#include "under_test.inc"

#define UNIT OBJECT(1)
#define OTHER_UNIT OBJECT(2)
#define RIFLE OBJECT(3)
#define PISTOL OBJECT(4)

/* a unit whose current weapon is the rifle in slot 0 (held by holder), switching to the pistol it carries */
static void switch_weapons(long holder)
{
	struct unit_datum *unit = object_get(UNIT);
	short slot;

	for (slot = 0; slot < MAXIMUM_WEAPONS_PER_UNIT; slot++)
		unit->unit.weapon_object_indices[slot] = NONE;
	unit->unit.weapon_object_indices[0] = RIFLE;
	unit->unit.weapon_object_indices[1] = PISTOL;
	unit->unit.current_weapon_index = 0;
	unit->unit.desired_weapon_index = 1;
	object_get(RIFLE)->object.parent_object_index = holder;
	object_get(PISTOL)->object.parent_object_index = UNIT;
	unit_ready_desired_weapon(UNIT, TRUE);
	CHECK(unit->unit.current_weapon_index == 1 && readied[PISTOL & 0xFFFF], "the pistol is not readied");
	CHECK(object_get(PISTOL)->object.parent_object_index == UNIT, "the pistol is not in the unit's hand");
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("held")
	{
		switch_weapons(UNIT);
		CHECK(detached[RIFLE & 0xFFFF] && stowed[RIFLE & 0xFFFF], "the held rifle is not put away");
	}
	else CASE("taken")
	{
		/* the host's updates gave the rifle to another unit */
		switch_weapons(OTHER_UNIT);
		CHECK(!detached[RIFLE & 0xFFFF] && !stowed[RIFLE & 0xFFFF], "the rifle is put away from another unit");
		CHECK(object_get(RIFLE)->object.parent_object_index == OTHER_UNIT, "the other unit lost the rifle");
	}
	else CASE("dropped")
	{
		switch_weapons(NONE);
		CHECK(!detached[RIFLE & 0xFFFF] && !stowed[RIFLE & 0xFFFF], "the rifle on the ground is put away");
	}
	else return 2;
	return 0;
}

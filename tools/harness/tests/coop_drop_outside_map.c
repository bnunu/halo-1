#include "harness.h"
#include "enums.inc"

/* On a co-op client, the host deletes a unit that carries a rifle and has a rider in a seat.
Dropping the rifle or ejecting the rider detaches it,
which connects it to the map where it is and activates it (objects.c's object_detach).
Outside the structure bsp it gets no cluster. */
#define OBJECT_COUNT 8
#define OBJECT(n) (0x10000L | (n))
struct object_header_datum { unsigned short flags; short cluster_index; };
struct object_datum
{
	struct { short type; long next_object_index, first_child_object_index; } object;
	struct { long player_index; short parent_seat_index; long weapon_object_indices[MAXIMUM_WEAPONS_PER_UNIT]; } unit;
};
#define unit_datum object_datum
struct player_datum { long unit_index, action_object_index; short action_result; };
struct data_iterator { long unused; };
enum { _player_action_result_reload = 1 };
static struct object_header_datum headers[OBJECT_COUNT];
static struct object_datum objects[OBJECT_COUNT];
static boolean deleted[OBJECT_COUNT];
static boolean objects_client_deleting;
static void *player_data;
/* the cluster where the deleted unit stands */
static short cluster_here;

static struct object_header_datum *object_header_get(long index) { return &headers[index & 0xFFFF]; }
static struct object_datum *object_get(long index) { return &objects[index & 0xFFFF]; }
static struct object_datum *object_try_and_get(long index) { return deleted[index & 0xFFFF] ? NULL : object_get(index); }
#define unit_get object_get
static void let_go(long index)
{
	object_header_get(index)->flags |= FLAG(_object_header_active_bit) | FLAG(_object_header_connected_to_map_bit);
	object_header_get(index)->cluster_index = cluster_here;
}
static void unit_network_drop_weapon(long unit_index, short slot)
{
	let_go(unit_get(unit_index)->unit.weapon_object_indices[slot]);
	unit_get(unit_index)->unit.weapon_object_indices[slot] = NONE;
}
static void unit_exit_seat_end(long index)
{
	let_go(index);
	unit_get(index)->unit.parent_seat_index = NONE;
}
static void object_deactivate(long index) { object_header_get(index)->flags &= ~FLAG(_object_header_active_bit); }
static void object_delete_immediately(long index) { deleted[index & 0xFFFF] = objects_client_deleting; }
static boolean network_objects_client_has(long index) { return TRUE; }
static void data_iterator_new(struct data_iterator *iterator, void *data) {}
static void *data_iterator_next(struct data_iterator *iterator) { return NULL; }
#define distributed_client_release_item(index, dropped) ((void)0)
#define game_engine_running() FALSE
#define weapon_is_flag(index) FALSE
#define game_engine_objective_weapon_deleted(index) ((void)0)
#define player_try_and_get(index) NULL
#define player_get(index) ((struct player_datum *)NULL)
#define network_player_detach_unit(index) ((void)0)

#include "under_test.inc"

#define UNIT OBJECT(1)
#define RIFLE OBJECT(2)
#define RIDER OBJECT(3)

static void delete_unit(void)
{
	short slot;
	long index;

	objects[1].object.type = _object_type_vehicle;
	objects[1].object.first_child_object_index = RIDER;
	objects[1].unit.player_index = NONE;
	for (slot = 0; slot < MAXIMUM_WEAPONS_PER_UNIT; slot++)
		objects[1].unit.weapon_object_indices[slot] = NONE;
	objects[1].unit.weapon_object_indices[0] = RIFLE;
	objects[2].object.type = _object_type_weapon;
	objects[2].object.next_object_index = NONE;
	objects[3].object.type = _object_type_biped;
	objects[3].object.next_object_index = NONE;
	objects[3].unit.parent_seat_index = 0;
	distributed_client_delete(UNIT);
	CHECK(deleted[1] && !deleted[2] && !deleted[3], "the unit's weapon or rider went with it");
	/* what objects_update asserts of every active object connected to the map (objects.c#369) */
	for (index = 2; index <= 3; index++)
		CHECK(!TEST_FLAG(headers[index].flags, _object_header_active_bit) || headers[index].cluster_index != NONE,
			"object %ld active without a cluster", index);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("outside")
	{
		/* the unit deleted after a structure bsp switch, where nothing has a cluster */
		cluster_here = NONE;
		delete_unit();
	}
	else CASE("inside")
	{
		cluster_here = 3;
		delete_unit();
		CHECK(TEST_FLAG(headers[2].flags, _object_header_active_bit) &&
			TEST_FLAG(headers[3].flags, _object_header_active_bit), "a dropped weapon or rider on the map deactivated");
	}
	else return 2;
	return 0;
}

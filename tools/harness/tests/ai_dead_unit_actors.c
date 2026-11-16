#include "harness.h"

#include "config.inc"
enum { ACTORS = 2 };
struct unit_datum
{
	struct { unsigned short damage_flags; } object;
	struct { short feign_death_timer; long player_index, actor_index; } unit;
};
struct actor_datum
{
	struct { boolean swarm, erase_next_tick; long unit_index; } meta;
	struct { short mode; } state;
};
struct actor_iterator { long index; };
static struct
{
	long last_highest_service_timer, current_highest_service_timer;
	boolean time_given_this_frame;
} globals, *ai_globals = &globals;
static struct actor_datum actors[ACTORS];
static struct unit_datum units[ACTORS];
static boolean deleted[ACTORS];
static int died[ACTORS], updated[ACTORS], erased[ACTORS];
static struct actor_datum *actor_get(long index) { CHECK(index >= 0 && index < ACTORS && !deleted[index], "bad actor"); return &actors[index]; }
static struct unit_datum *unit_get(long index) { CHECK(index >= 0 && index < ACTORS, "bad unit"); return &units[index]; }
static void actor_iterator_new(struct actor_iterator *iterator, boolean active_only) { iterator->index = NONE; }
static struct actor_datum *actor_iterator_next(struct actor_iterator *iterator)
{
	while (++iterator->index < ACTORS)
		if (!deleted[iterator->index])
			return &actors[iterator->index];
	return NULL;
}
static void actor_erase(long index, boolean silent) { erased[index]++; deleted[index] = TRUE; }
static void actor_update(long index) { updated[index]++; }
/* (unit_died: its actor_died deletes the actor) */
static void unit_died(long unit_index, boolean feigned)
{
	CHECK(!feigned, "a feigned death");
	died[unit_index]++;
	deleted[units[unit_index].unit.actor_index] = TRUE;
	units[unit_index].unit.actor_index = NONE;
}
#include "under_test.inc"

int main(int argc, char **argv)
{
	const char *case_name = argv[1];
	long index;

	for (index = 0; index < ACTORS; index++)
	{
		actors[index].meta.unit_index = index;
		actors[index].state.mode = _actor_mode_braindead + 1;
		units[index].unit.player_index = NONE;
		units[index].unit.actor_index = index;
	}
	/* (the other actor is alive throughout, and updated as before) */
	units[0].object.damage_flags = FLAG(_object_dead_bit);
	CASE("dead")
	{
		/* a unit marked dead without unit_died: its actor goes */
		actors_update();
		CHECK(died[0] == 1 && !updated[0], "dead unit's actor kept (died %d, updated %d)", died[0], updated[0]);
		CHECK(deleted[0], "actor not deleted");
		actors_update();
		CHECK(died[0] == 1, "died twice");
	}
	CASE("braindead")
	{
		actors[0].state.mode = _actor_mode_braindead;
		actors_update();
		CHECK(died[0] == 1, "braindead actor's dead unit kept");
	}
	CASE("alive")
	{
		units[0].object.damage_flags = 0;
		actors_update();
		CHECK(!died[0] && updated[0] == 1, "living unit died or its actor was not updated");
	}
	CASE("feigning")
	{
		units[0].unit.feign_death_timer = 30;
		actors_update();
		CHECK(!died[0] && updated[0] == 1, "feigned death made real");
	}
	CASE("player")
	{
		units[0].unit.player_index = 0;
		actors_update();
		CHECK(!died[0], "player's unit died through its actor");
	}
	CASE("other-actor")
	{
		units[0].unit.actor_index = 1;
		actors_update();
		CHECK(!died[0], "unit died for an actor that is not its own");
	}
	CASE("swarm")
	{
		actors[0].meta.swarm = TRUE;
		actors_update();
		CHECK(!died[0], "swarm actor's unit died");
	}
	CASE("erase")
	{
		actors[0].meta.erase_next_tick = TRUE;
		actors_update();
		CHECK(erased[0] == 1 && !died[0], "an actor to erase was not erased first");
	}
	CHECK(!died[1] && updated[1] >= 1, "the living actor was disturbed");
	return 0;
}

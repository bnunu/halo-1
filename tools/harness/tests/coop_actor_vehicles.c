/* A network game's client drives the host's units that no player has with
the control the host sent (network_actors.c, network_actors_drive): a
vehicle a recording drives as well as an actor's biped, so what the vehicle
makes of the control (its speed, its seat's power: its functions) is what
the host's makes of it. The real drive loop, in a world of a vehicle and a
biped. */
#include "harness.h"

typedef struct { real i, j; } real_vector2d;
struct unit_control_data { real_vector3d throttle; unsigned short control_flags; };
enum { _object_type_biped, _object_type_vehicle };
enum { _object_dead_bit };
enum { _object_outside_of_map_bit = 3 };
#define _object_mask_unit 3
enum { _actor_action_melee = 100, _actor_action_leap };
#define TICKS_PER_SECOND 30
#define MAXIMUM_NETWORK_ACTORS 8

struct fake_unit
{
	struct { short type; unsigned long flags; unsigned long damage_flags; boolean kept_active; } object;
	struct
	{
		long player_index;
		long driver_object_index;
		boolean actively_controlled;
		boolean possessed;
		int controls;
		real throttle;
	} unit;
};
#define unit_datum fake_unit
static struct fake_unit units[4];
static long now;

static void *object_try_and_get_and_verify_type(long index, unsigned long mask)
{
	return index >= 0 && index < 4 ? &units[index] : NULL;
}
static struct fake_unit *unit_get(long index) { return &units[index]; }
static struct fake_unit *object_get(long index) { return &units[index]; }
static long game_time_get(void) { return now; }
static void unit_set_actively_controlled(long index, boolean on) { units[index].unit.actively_controlled = on; }
/* (an object not automatically deactivated stays active away from the players) */
static void object_set_automatic_deactivation(long index, boolean automatic)
{
	units[index].object.kept_active = !automatic;
}
static void unit_set_possessed(long index, boolean on) { units[index].unit.possessed = on; }
static void unit_control(long index, struct unit_control_data const *control)
{
	units[index].unit.controls++;
	units[index].unit.throttle = control->throttle.i;
}
static void unit_melee_attack_begin(long unit, boolean player, real_vector2d *alignment) {}
static void unit_leap_begin(long unit, real_vector2d *alignment) {}
static void unit_start_animation_impulse(long unit, short impulse, real_vector2d *alignment) {}
static void speech_play(long unit, long sound) {}
static void unit_port_play_user_animation(long unit, long graph, short animation, boolean interpolate, long flags) {}

#include "client_actor.inc"
static struct client_actor client_actors[MAXIMUM_NETWORK_ACTORS];
static short client_actor_count;
#include "drive.inc"

enum { VEHICLE = 1, BIPED = 2, DRIVER = 3 };

static void reset(void)
{
	memset(units, 0, sizeof(units));
	memset(client_actors, 0, sizeof(client_actors));
	units[VEHICLE].object.type = _object_type_vehicle;
	units[BIPED].object.type = units[DRIVER].object.type = _object_type_biped;
	units[VEHICLE].unit.player_index = units[BIPED].unit.player_index = units[DRIVER].unit.player_index = NONE;
	/* (the vehicle's AI driver, idle while the recording drives) */
	units[VEHICLE].unit.driver_object_index = DRIVER;
	units[BIPED].unit.driver_object_index = units[DRIVER].unit.driver_object_index = NONE;
	client_actor_count = 0;
	now = 1000;
}

/* units.c's unit_update: a vehicle not possessed by a recording takes its
driver's throttle */
static void units_update(void)
{
	if (!units[VEHICLE].unit.possessed && units[VEHICLE].unit.driver_object_index != NONE)
		units[VEHICLE].unit.throttle = units[units[VEHICLE].unit.driver_object_index].unit.throttle;
}

/* the host's control for a unit, received this tick */
static struct client_actor *received(long unit_index, real throttle)
{
	struct client_actor *actor = &client_actors[client_actor_count++];

	actor->unit_index = unit_index;
	actor->received_time = now;
	actor->control.throttle.i = throttle;
	actor->impulse = NONE;
	actor->speech_sound = NONE;
	actor->user_animation_graph = NONE;
	return actor;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";

	CASE("vehicle")
	{
		/* a recording's vehicle: actively controlled, possessed, kept
		active, and driven with the host's control each tick, which it keeps
		over its idle driver's */
		reset();
		received(VEHICLE, 0.75f);
		network_actors_drive();
		units_update();
		now++;
		network_actors_drive();
		units_update();
		CHECK(units[VEHICLE].unit.actively_controlled && units[VEHICLE].unit.possessed,
			"the vehicle is not actively controlled (%d) or possessed (%d)", units[VEHICLE].unit.actively_controlled,
			units[VEHICLE].unit.possessed);
		CHECK(units[VEHICLE].object.kept_active, "the vehicle is not kept active away from the players");
		CHECK(units[VEHICLE].unit.controls == 2 && units[VEHICLE].unit.throttle == 0.75f,
			"the vehicle was driven %d times, throttle %.2f", units[VEHICLE].unit.controls, units[VEHICLE].unit.throttle);
		return 0;
	}
	CASE("biped")
	{
		/* an actor's biped, as before */
		reset();
		received(BIPED, 0.5f);
		network_actors_drive();
		CHECK(units[BIPED].unit.actively_controlled && units[BIPED].unit.controls == 1, "the biped was not driven");
		CHECK(!units[BIPED].unit.possessed && !units[BIPED].object.kept_active,
			"an actor's biped was taken for a recording's");
		return 0;
	}
	CASE("released")
	{
		/* the host stopped sending (the recording ended): once the control
		has been held its while, the vehicle is let go of */
		reset();
		received(VEHICLE, 0.75f);
		network_actors_drive();
		now += 2 * TICKS_PER_SECOND + 1;
		network_actors_drive();
		CHECK(!units[VEHICLE].unit.actively_controlled && !units[VEHICLE].unit.possessed &&
			!units[VEHICLE].object.kept_active && client_actor_count == 0,
			"the vehicle is still held: actively %d, possessed %d, kept active %d, %d entries",
			units[VEHICLE].unit.actively_controlled, units[VEHICLE].unit.possessed, units[VEHICLE].object.kept_active,
			client_actor_count);
		CHECK(units[VEHICLE].unit.controls == 1, "the vehicle was driven after it was let go of");
		return 0;
	}
	CASE("outside")
	{
		/* a recording's vehicle out of the map, where the host sends no place
		of it: left as it was, not flown by itself */
		reset();
		units[VEHICLE].object.flags |= FLAG(_object_outside_of_map_bit);
		received(VEHICLE, 0.75f);
		network_actors_drive();
		CHECK(!units[VEHICLE].unit.actively_controlled && !units[VEHICLE].unit.possessed &&
			!units[VEHICLE].object.kept_active && units[VEHICLE].unit.controls == 0,
			"a vehicle out of the map was driven: actively %d, possessed %d, kept active %d, %d controls",
			units[VEHICLE].unit.actively_controlled, units[VEHICLE].unit.possessed, units[VEHICLE].object.kept_active,
			units[VEHICLE].unit.controls);
		CHECK(client_actor_count == 1, "the host's control was dropped");
		return 0;
	}
	CASE("leaves")
	{
		/* driven in the map, let go of as it leaves it, and taken again once
		the host's states bring it back */
		reset();
		received(VEHICLE, 0.75f);
		network_actors_drive();
		units[VEHICLE].object.flags |= FLAG(_object_outside_of_map_bit);
		now++;
		network_actors_drive();
		CHECK(!units[VEHICLE].unit.actively_controlled && !units[VEHICLE].unit.possessed &&
			!units[VEHICLE].object.kept_active && units[VEHICLE].unit.controls == 1,
			"a vehicle that left the map is still driven: actively %d, possessed %d, kept active %d, %d controls",
			units[VEHICLE].unit.actively_controlled, units[VEHICLE].unit.possessed, units[VEHICLE].object.kept_active,
			units[VEHICLE].unit.controls);
		units[VEHICLE].object.flags &= ~FLAG(_object_outside_of_map_bit);
		now++;
		network_actors_drive();
		CHECK(units[VEHICLE].unit.actively_controlled && units[VEHICLE].unit.possessed &&
			units[VEHICLE].object.kept_active && units[VEHICLE].unit.controls == 2,
			"a vehicle back in the map was not taken again");
		return 0;
	}
	fprintf(stderr, "no case %s\n", case_name);
	return 2;
}

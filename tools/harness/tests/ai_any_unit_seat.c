#include "harness.h"
#include <math.h>
#include "declarations.inc"

/* An actor sent to a seat 2 world units ahead of it, in a vehicle or in a biped
(Custom Edition maps give bipeds seats). */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
typedef struct { real x, y; } real_point2d;
typedef struct { real i, j; } real_vector2d;
enum { _actor_definition_disallow_vehicle_combat_bit = 0, _actor_variant_definition_prefer_passenger_seat_bit = 0 };
enum { _actor_action_vehicle = 1 };
struct vehicle_state_data { long vehicle_index; short seat_index; real_point3d destination_point; };
struct actor_datum
{
	struct { long definition_index, variant_definition_index, unit_index; } meta;
	struct { struct { real_point3d body_position; } position; real_vector3d facing_vector; } input;
	struct { short action; struct { struct vehicle_state_data vehicle; } action_data; } state;
};
struct actor_definition { unsigned long flags2; };
struct actor_variant_definition { unsigned long flags; };
struct prop_iterator { long unused; };
struct prop_datum { boolean enemy; long actor_index; };
struct unit_datum { long definition_index; struct { short type; } object; };
#define OBJECT(n) (0x10000L | (n))
#define VEHICLE OBJECT(1)
#define BIPED OBJECT(2)
#define VEHICLE_DEFINITION 0x100
static struct unit_datum objects[3] = {{0}, {VEHICLE_DEFINITION, {_object_type_vehicle}}, {0x200, {_object_type_biped}}};
static struct actor_datum actor = {{0, 0, OBJECT(0)}, {{{0.0f, 0.0f, 0.0f}}, {1.0f, 0.0f, 0.0f}}};
static struct actor_definition definition;
static struct actor_variant_definition variant_definition;
static boolean custom_edition_tags;

/* like objects.c's, asserts the type (objects.c#1690) */
static void *object_get_and_verify_type(long index, unsigned long mask)
{
	CHECK(TEST_FLAG(mask, objects[index & 0xFFFF].object.type), "objects.c#1690: object type %d not in %lx",
		objects[index & 0xFFFF].object.type, mask);
	return &objects[index & 0xFFFF];
}
/* the vehicle-specific part of a vehicle tag, which a biped does not have */
static void *vehicle_specific_definition_get(long definition_index)
{
	CHECK(definition_index == VEHICLE_DEFINITION, "a biped's definition read as a vehicle's");
	return NULL;
}
static boolean custom_edition_cache_tags_loaded(void) { return custom_edition_tags; }
static struct actor_datum *actor_get(long index) { return &actor; }
static struct actor_datum *actor_try_and_get(long index) { return &actor; }
static struct actor_definition *actor_definition_get(long index) { return &definition; }
static struct actor_variant_definition *actor_variant_definition_get(long index) { return &variant_definition; }
static boolean unit_get_seat_entrance_point(long unit_index, long vehicle_index, short seat_index, real_point3d *entrance,
	real_point3d *seat, real_point3d *hint)
{
	entrance->x = 1.5f, entrance->y = 0.0f, entrance->z = 0.0f;
	seat->x = 2.0f, seat->y = 0.0f, seat->z = 0.0f;
	*hint = *seat;
	return TRUE;
}
static void object_get_origin(long index, real_point3d *origin) { origin->x = 2.0f, origin->y = origin->z = 0.0f; }
static real normalize2d(real_vector2d *vector)
{
	real length = sqrtf(vector->i * vector->i + vector->j * vector->j);

	if (length > 0.0f)
		vector->i /= length, vector->j /= length;
	return length;
}
static real distance2d(real_point2d const *a, real_point2d const *b) { return hypotf(b->x - a->x, b->y - a->y); }
static void vector_from_points2d(real_point2d const *a, real_point2d const *b, real_vector2d *vector)
{
	vector->i = b->x - a->x, vector->j = b->y - a->y;
}
static real dot_product2d(real_vector2d const *a, real_vector2d const *b) { return a->i * b->i + a->j * b->j; }
static boolean unit_seat_is_driver(long unit_index, short seat_index) { return seat_index == 0; }
#define unit_seat_filled(unit_index, seat_index) FALSE
#define unit_seat_allow_noncombatants(unit_index, seat_index) TRUE
#define unit_seat_is_gunner(unit_index, seat_index) FALSE
#define prop_iterator_new(iterator, actor_index) ((void)0)
#define prop_iterator_next(iterator) ((struct prop_datum *)NULL)

#pragma clang diagnostic ignored "-Wparentheses"
#include "under_test.inc"

/* whether the actor takes the unit's seat 0, and how much it wants it */
static void evaluate(long unit_index)
{
	real weight = 0.0f;
	boolean within_range = TRUE, correct_facing = FALSE, could_fake = FALSE;

	CHECK(action_vehicle_evaluate_seat(OBJECT(3), unit_index, 0, FALSE, NULL, NULL, NULL, &weight, &within_range,
		&correct_facing, &could_fake), "the seat is not taken");
	CHECK(weight > 3.0f && !within_range && correct_facing && !could_fake, "seat weight %f", weight);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";

	CASE("ce-biped")
	{
		custom_edition_tags = TRUE;
		evaluate(BIPED);
	}
	else CASE("ce-vehicle")
	{
		custom_edition_tags = TRUE;
		evaluate(VEHICLE);
	}
	else CASE("xbox-vehicle")
	{
		evaluate(VEHICLE);
	}
	else CASE("xbox-biped")
	{
		evaluate(BIPED);
	}
	else return 2;
	return 0;
}

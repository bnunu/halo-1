/*
NETWORK_OBJECTS.C

The distributed netcode's objects (port/linux/NETCODE.md, stages 3 and 4).

The game's objects that matter to more than the eye (units, vehicles,
weapons and equipment) are the host's, and every machine has them at the
same datum index (identifier and all), so that any message can name one:

- The host tells its clients (reliably) of each object it makes, with what
  it is, where, and how it looks, and of each it deletes; a client makes
  and deletes its copies to match, at the host's index. A client that has
  loaded asks for the host's objects all over and is told when it has them
  all; from then on it deletes any such object it has that the host has not
  told it of (what its own simulation made: a dying unit's grenades, a
  vehicle's weapons), and nothing but the host's word deletes the host's.
- A client places the game's objects when its map loads as the host did,
  at the same indices: the host's word on those finds them already there.
  Past loading, its own objects (projectiles, effects' objects: what only
  it sees) take indices from the upper half of the array, which the host's
  do not reach in practice; a host's object that does comes first.
- Every tick the host sends where its moving objects are (vehicles, items,
  bodies), each client those near its players every tick and those further
  less often, with each as it comes to rest, and to every client a few of
  those at rest, round the lot; a client moves its copies there, a little
  off half of the way each tick, further (or come to rest) drawn gliding
  from where they were rather than jumping (render_interpolation.c).
  A client's own player's vehicle is its own, as its own player's unit is:
  it sends the host where it drives it at which of its ticks, which the host
  takes within what the vehicle can have moved at its next tick, and tells
  the client which of its ticks it has it at; the client moves it by how far
  that is from where it had it at that tick, only when far off.
- Ten times a second, the host sends what a unit carries when that has
  changed (and once a second whatever it is): which weapons, slot for slot,
  their ammunition, the weapon in hand and the grenades; a client moves its
  copies of those weapons in and out of its copies of the units to match (it
  decides no pickups, swaps or drops itself). A change of weapons or
  grenades goes to every client at once, one of ammunition alone to the
  unit's own client at once and to the others as often as the unit's place
  is sent them. A client's own player spends its own ammunition and grenades:
  it takes the host's count only once the host has had a round trip to see
  what it did.
*/

#include "cseries.h"
#include "game/game.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "networking/network_game_globals.h"
#include "objects/objects.h"
#include "vehicle_riding.h"
#include "objects/damage.h"
#include "objects/object_definitions.h"
#include "effects/effects.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "units/units.h"
#include "units/bipeds.h"
#include "mcc_seats.h"
#include "units/biped_definitions.h"
#include "units/vehicle_definitions.h"
#include "units/vehicle_datum.h"
#include "items/items.h"
#include "items/weapons.h"
#include "items/weapon_definitions.h"
#include "items/equipment_definitions.h"
#include "camera/observer.h"
#include "cutscene/cinematics.h"
#include "network_coop.h"
#include "network_distributed.h"
#include "mcc_network.h"
#include "profile_sections.h"

#include <math.h>

/* physics.c's */
extern real global_gravity;

/* units.c's */
void unit_network_add_weapon(long unit_index, long weapon_index, short slot);
void unit_network_drop_weapon(long unit_index, short slot);
void unit_network_forget_weapon(long unit_index, short slot);
/* players.c's */
void network_player_detach_unit(long player_index);
/* render_interpolation.c's */
void render_interpolation_correct_object(long object_index, real_vector3d const *offset);
/* cache_files.c's */
boolean tag_index_is_group(long tag_index, long group_tag);

/* the vehicle tag's speeds (world units a tick), as actor_moving.c reads
them: its layout is not the public header's */
struct vehicle_definition
{
	byte __unknown0[0x2F4];
	short vehicle_type;
	short __unknown2F6;
	real maximum_forward_speed;
	real maximum_reverse_speed;
};

typedef char network_objects_vehicle_type_offset_assert[
	offsetof(struct vehicle_definition, vehicle_type) == 0x2F4 ? 1 : -1];
/* (vehicles.c's vehicle types: those that float, fly or stay) */
enum
{
	_vehicle_type_human_boat = 2,
	_vehicle_type_human_plane = 3,
	_vehicle_type_alien_fighter = 5,
	_vehicle_type_turret = 6,
};

typedef char network_objects_vehicle_maximum_forward_speed_offset_assert[
	offsetof(struct vehicle_definition, maximum_forward_speed) == 0x2F8 ? 1 : -1];

enum
{
	INVENTORY_INTERVAL_TICKS = 3,
	/* a unit's inventory is sent this often even unchanged (one lost is
	made good) */
	INVENTORY_REFRESH_TICKS = TICKS_PER_SECOND,
	/* ... and a unit's that has come to carry nothing, for this long after */
	EMPTY_INVENTORY_TICKS = 3 * INVENTORY_REFRESH_TICKS,
	/* the host's objects at rest sent each tick, round them all */
	RESTING_STATES_PER_TICK = 4,
	/* an object come to rest is sent to every client this many times, this
	many ticks apart: one lost would leave a falling body hanging until its
	turn round all of them, which in a large game is many seconds */
	REST_STATE_REPEATS = 3,
	REST_STATE_REPEAT_TICKS = 8,
	/* a client asks for the host's objects again until it has them (a host
	still loading misses the asking), after this, twice as long each time
	up to the most */
	CLIENT_READY_INTERVAL_TICKS = TICKS_PER_SECOND,
	CLIENT_READY_MAXIMUM_INTERVAL_TICKS = 4 * TICKS_PER_SECOND,
	/* the host tells a machine all its objects again at most once in this
	long, asked again (a client that failed to make one: asked again sooner,
	they are on their way; a flood of asking is not answered in full) */
	HOST_OBJECTS_RESEND_TICKS = 10 * TICKS_PER_SECOND,
	/* a client that failed to make one of the host's objects asks for them
	all again this long after, twice as long each time it fails again up to
	the most */
	CLIENT_RETRY_TICKS = HOST_OBJECTS_RESEND_TICKS + TICKS_PER_SECOND,
	CLIENT_RETRY_MAXIMUM_TICKS = 60 * TICKS_PER_SECOND,
	/* a client's own objects, from here up */
	LOCAL_OBJECTS_FIRST_INDEX = MAXIMUM_OBJECTS_PER_MAP / 2,
	/* ... those of the kinds the host has, made since the last tick, checked
	(more, and all of them are) */
	MAXIMUM_CLIENT_NEW_OBJECTS = 1024,
	/* ... the host's inventories it could not apply in full yet, one for
	each unit that carries anything (a vehicle is a unit, its weapons are
	objects) whose objects the stream has not brought: most of them while
	it joins, few after. Room for as many as there are players; past that
	the oldest goes, and its unit waits for the host's next inventory of it */
	MAXIMUM_CLIENT_RETRIED_INVENTORIES = MAXIMUM_TRACKED_PLAYERS,
	MAXIMUM_ENTRIES_PER_MESSAGE = 64,
	/* the objects the host has at the same indices everywhere */
	NETWORKED_OBJECT_TYPES =
		_object_mask_biped | _object_mask_vehicle | _object_mask_weapon | _object_mask_equipment,
	/* how often the host sends a client a moving object further from its
	players than each distance below (every tick nearer), and the vehicle the
	client drives itself */
	MAXIMUM_OBJECT_PERIOD_TICKS = 4,
	OWN_VEHICLE_PERIOD_TICKS = 3,
	/* a client: where the vehicles its own players drive were, the last
	ticks (a power of two, more than the longest round trip) */
	OWN_VEHICLE_POSITION_TICKS = 64,
	/* the host: the client's ticks it takes as run since a client's vehicle's
	anchor beyond its own (its messages delayed, then bunched) */
	VEHICLE_PREDICTION_JITTER_TICKS = 6,
	/* ... how much higher than a throw takes it a vehicle in the air may be
	(its bounces, the ground's bumps), world units */
	VEHICLE_PREDICTION_RISE_TOLERANCE = 2,
	/* ... and the most of a client's round trip it takes a client's vehicle
	in the air to be ahead of its copy by */
	VEHICLE_PREDICTION_CEILING_LEAD_TICKS = 15,
	/* ... how long it measures predictions from one it took, the anchor,
	before it takes a newer as the anchor (the jitter and the blend granted
	once a second, not once a tick) */
	VEHICLE_PREDICTION_ANCHOR_TICKS = TICKS_PER_SECOND,
	/* ... and how long, with none taken, it keeps the anchor, and tells the
	client which of its ticks it has the vehicle at (longer: from where it
	has the vehicle only, a ride begun again) */
	VEHICLE_PREDICTION_REFERENCE_TICKS = 2 * TICKS_PER_SECOND,
	/* ... and how long it remembers how fast its own ticks sent a client's
	vehicle (an anchor's second, and the longest round trip: a client learns
	of an explosion that throws its vehicle a round trip late) */
	VEHICLE_PREDICTION_SPEED_TICKS = VEHICLE_PREDICTION_ANCHOR_TICKS + 2 * TICKS_PER_SECOND,
	/* a client: how long a biped the host says is dead, no player's, may
	live on here before it is killed with nothing to show (the host's
	killing blow, unreliable, shows it; lost, nothing else would) */
	DEAD_BIPED_FALLBACK_TICKS = TICKS_PER_SECOND / 2,
	/* a client: its own player's ammunition and grenades are the host's once
	the host has had this long past a round trip to see what it did with
	them (the host's inventories go every INVENTORY_INTERVAL_TICKS) */
	OWN_INVENTORY_MARGIN_TICKS = 2,
	/* ... the round trip without one measured, and the most */
	DEFAULT_OWN_ROUND_TRIP_TICKS = 6,
	MAXIMUM_OWN_ROUND_TRIP_TICKS = 60,
	/* ... how long the weapon the host says it picked up may take to reach
	its unit, to be readied (the host's messages are not in step) */
	PICKED_UP_WEAPON_TICKS = 3 * TICKS_PER_SECOND,
	/* the most grenades of a kind a unit carries, as the host says (a dead
	unit drops each) */
	MAXIMUM_INVENTORY_GRENADES = 16,
};

/* the distances from a client's nearest player (world units) past which
the host sends it a moving object every second tick, every third and every
fourth (as it sends players, network_distributed.c) */
#define NEAR_OBJECT_DISTANCE 25.0f
#define MIDDLE_OBJECT_DISTANCE 60.0f
#define FAR_OBJECT_DISTANCE 120.0f

/* a step of an unsteered vehicle's path (world units a tick). Further than
this in one tick, the host moved it some other way (a teleport), which is
not carried on as the path */
#define UNSTEERED_PATH_STEP 4.0f
/* the most ticks an unsteered vehicle's glide between samples covers: twice
the longest the host leaves between a moving object's states. A record left
from before (driven since, or at rest for long) glides no slower, and a jump
is judged against no more than this. */
#define UNSTEERED_PATH_MAXIMUM_TICKS (2 * MAXIMUM_OBJECT_PERIOD_TICKS)
/* the most ticks an unsteered vehicle goes on along its path past the newest
sample, when the next is late (the longest the host leaves between a moving
object's states); as long again it takes to come back to the newest, if
none came (the host's has stopped) */
#define UNSTEERED_PATH_LATE_TICKS MAXIMUM_OBJECT_PERIOD_TICKS
/* world units */
#define REMOTE_OBJECT_TOLERANCE 0.05f
/* ... and the cosine of the angle (an object at rest turned) */
#define REMOTE_OBJECT_ANGLE_TOLERANCE 0.98f
/* how far a client's vehicle prediction moves the host's copy for the copy
at rest to wake, world units */
#define VEHICLE_PREDICTION_STILL_DISTANCE 0.01f
#define LOCAL_VEHICLE_TOLERANCE 4.0f
/* (the host takes further than a client puts right: between the two they
would disagree for good) */
#define HOST_VEHICLE_ACCEPT_TOLERANCE 5.0f
/* how far from the origin an object is (world units) */
#define OBJECT_WORLD_BOUND 32768.0f
/* how fast a client's own player's vehicle moves (world units, radians a
tick) */
#define MAXIMUM_PREDICTED_VEHICLE_SPEED 3.0f
#define MAXIMUM_PREDICTED_VEHICLE_ANGULAR_SPEED 1.0f
/* ... how much further a tick than it goes (twice its tag's top speed, or
as fast as the host's ticks sent its copy lately) it may be (world units a
tick) */
#define PREDICTED_VEHICLE_SPEED_MARGIN 0.1f
/* the teams a flag's or ball's owner team names (game_engine_ctf.c's
NUMBER_OF_CTF_TEAMS, game_engine_oddball.c's MAXIMUM_ODDBALLS) */
#define CTF_FLAG_TEAMS 2
#define ODDBALL_TEAMS 16

enum
{
	_object_change_create,
	_object_change_delete,
};

/* struct distributed_object_change and struct distributed_object_state
flags */
enum
{
	/* an item in a unit's inventory, not in the world */
	_distributed_object_carried_bit = 0,
	_distributed_object_at_rest_bit,
	/* a unit dead */
	_distributed_object_dead_bit,
	/* (struct distributed_object_state, the host's) the host has the vehicle
	where the client it goes to had it at the tick in time */
	_distributed_object_predicted_bit,
	/* (struct distributed_object_state) a unit's integrated light is on, and
	the host sent it at all: older builds leave both 0, which is not "off" */
	_distributed_object_light_sent_bit,
	_distributed_object_light_on_bit,
};

struct distributed_object_change
{
	byte change;
	byte flags;
	byte owner_player_index;
	/* which bitmap of its shaders it draws with, when not its tag's (0). An
	AI unit's actor variant sets it after the unit is made
	(actor_customize_unit): the Elite major's and commander's armor are 1
	and 2. Builds before it sent 0 here. */
	byte forced_shader_permutation_index;
	long object_index;
	long definition_index;
	short owner_team_index;
	short variant_number;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d translational_velocity;
	real_vector3d angular_velocity;
	real_rgb_color change_colors[NUMBER_OF_OBJECT_CHANGE_COLORS];
	byte region_permutations[MAXIMUM_REGIONS_PER_OBJECT];
};

struct distributed_object_state
{
	long object_index;
	byte flags;
	byte pad;
	/* the client's tick, its low 16 bits: a vehicle prediction's, and
	(_distributed_object_predicted_bit) the one the host has it at */
	short time;
	real_point3d position;
	struct distributed_vector forward;
	struct distributed_vector up;
	struct distributed_vector translational_velocity;
	struct distributed_vector angular_velocity;
};

struct distributed_inventory
{
	long unit_index;
	char grenade_counts[NUMBER_OF_UNIT_GRENADE_TYPES];
	char current_weapon_index;
	byte pad;
	long weapon_indices[MAXIMUM_WEAPONS_PER_UNIT];
	/* each weapon's magazines */
	short rounds_total[MAXIMUM_WEAPONS_PER_UNIT][2];
	short rounds_loaded[MAXIMUM_WEAPONS_PER_UNIT][2];
	/* (of 0 to 1, AGE_SCALE) */
	word age[MAXIMUM_WEAPONS_PER_UNIT];
};

#define AGE_SCALE 65535.0f

struct distributed_object_change_message
{
	struct distributed_message_header header;
	struct distributed_object_change changes[MAXIMUM_ENTRIES_PER_MESSAGE];
};

struct distributed_object_state_message
{
	struct distributed_message_header header;
	struct distributed_object_state states[MAXIMUM_ENTRIES_PER_MESSAGE];
};

struct distributed_inventory_message
{
	struct distributed_message_header header;
	struct distributed_inventory inventories[MAXIMUM_ENTRIES_PER_MESSAGE];
};

/* ---------- globals */

/* the host: the objects it has told its clients of, by absolute index
(the object's datum index), NONE for none */
static long objects_host_told[MAXIMUM_TRACKED_OBJECTS];
/* ... the next object at rest to send, round them all */
static long objects_host_resting_cursor;
/* ... past the highest of them */
static long objects_host_told_count;
/* ... whether each was moving at the last tick, and when each last came to
rest (NONE: not since it was told of), to send it to every client
(REST_STATE_REPEATS) */
static boolean objects_host_state_moving[MAXIMUM_TRACKED_OBJECTS];
static long objects_host_rest_times[MAXIMUM_TRACKED_OBJECTS];
/* ... what each unit's inventory was last sent as (in all, and its weapons
and grenades), when to every client, and when its ammunition alone last
changed (NONE: not since); when it last carried anything (NONE: never) */
static struct
{
	unsigned long checksum;
	unsigned long weapons_checksum;
	long time;
	long ammunition_time;
	long carried_time;
} objects_host_inventories[MAXIMUM_TRACKED_OBJECTS];
/* ... each client machine, found once a tick: where its players' living
units are, those units, and the vehicles they drive (with the player's
absolute index); and where every player's living unit is, for a machine
with none (dead or joining, it watches one of them) */
static struct
{
	short count;
	long indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short player_unit_count;
	real_point3d player_unit_origins[MAXIMUM_TRACKED_PLAYERS];
	struct
	{
		short unit_count;
		long unit_indices[MAXIMUM_LOCAL_PLAYERS];
		real_point3d origins[MAXIMUM_LOCAL_PLAYERS];
		short vehicle_count;
		long vehicle_indices[MAXIMUM_LOCAL_PLAYERS];
		short vehicle_player_indices[MAXIMUM_LOCAL_PLAYERS];
	} machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
} objects_host_viewers;
/* ... each machine it has told all its objects: when, and which players it
had then (another machine in its place has others) */
static struct
{
	long time;
	long player_indices[MAXIMUM_LOCAL_PLAYERS];
} objects_host_machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
/* ... each client's player's latest vehicle prediction, taken at the next
tick; the vehicle it last took (NONE: none), at which of the client's ticks
and its own, and where that left the host's copy; and the anchor
predictions are measured from: one it took, at which of the client's ticks
and its own, and where the client had it; the velocity it took (what its
next tick started from); and how fast its own ticks sent the vehicle beyond
the client's word, the most of this span of VEHICLE_PREDICTION_SPEED_TICKS
and of the one before */
static struct
{
	boolean valid;
	long machine_index;
	struct distributed_object_state state;
	long accepted_vehicle_index;
	word accepted_time;
	long accepted_host_time;
	real_point3d accepted_host_position;
	word anchor_time;
	long anchor_host_time;
	real_point3d anchor_position;
	real_vector3d accepted_velocity;
	long speed_time;
	real speeds[2];
	/* the vehicle it last had on the ground (NONE: none), how high, and how
	fast up it went as it left it, and when (network_objects_note_vehicle_ground) */
	long ground_vehicle_index;
	long ground_time;
	/* the vehicle whose copy was at rest when it last took its word (NONE:
	none), and where it came to rest */
	long rest_vehicle_index;
	real_point3d rest_position;
	real ground_height;
	real ground_rise_speed;
} objects_host_vehicle_predictions[MAXIMUM_TRACKED_PLAYERS];
/* a client: the host's objects it has, by absolute index */
static long objects_client_has[MAXIMUM_TRACKED_OBJECTS];
/* Only the first validated description in a checkpoint world may replace a
same-handle corpse. A later snapshot can precede a death already received. */
static boolean objects_client_checkpoint_pending[MAXIMUM_TRACKED_OBJECTS];
/* ... the bipeds, no player's, the host said were dead while they lived
here, and when it first did (DEAD_BIPED_FALLBACK_TICKS) */
static struct
{
	long object_index;
	long time;
} objects_client_dead[MAXIMUM_TRACKED_OBJECTS];
/* ... the host samples an unsteered vehicle is drawn between: where it was,
where the host has it now, the ticks the glide between them takes, the host's
tick of the newest, and the path's velocity between the host's last two
(distributed_client_note_unsteered_path) */
static struct
{
	long object_index;
	long time;
	long host_time;
	real interval;
	real_vector3d velocity;
	real_point3d previous_position;
	real_point3d latest_position;
	real_vector3d previous_forward;
	real_vector3d previous_up;
	real_vector3d latest_forward;
	real_vector3d latest_up;
} objects_client_unsteered_path[MAXIMUM_TRACKED_OBJECTS];
/* ... its own objects made since the last tick (of any kind: those of the
kinds the host has go), and whether to look through all the objects
instead (the host's all told, too many made, or made where it cannot say) */
static long objects_client_new_objects[MAXIMUM_CLIENT_NEW_OBJECTS];
static short objects_client_new_object_count;
static boolean objects_client_check_all;
/* ... the lowest of its own indices that may be free (those below it
taken), and the object array's count when it last made an object (NONE:
not known; another count, an object deleted since: from the first) */
static long objects_client_local_free_index;
static long objects_client_object_count;
/* ... where the vehicles its own players drive were at its last ticks */
static struct distributed_own_vehicle
{
	long time;
	long vehicle_index;
	real_point3d position;
} objects_client_own_vehicles[MAXIMUM_LOCAL_PLAYERS][OWN_VEHICLE_POSITION_TICKS];
/* ... what its own players' units carried at the last tick, and when they
last threw a grenade of each kind, and fired or reloaded each weapon
(NONE: not while it has had it) */
static struct distributed_own_inventory
{
	long unit_index;
	char grenade_counts[NUMBER_OF_UNIT_GRENADE_TYPES];
	long grenade_times[NUMBER_OF_UNIT_GRENADE_TYPES];
	long weapon_indices[MAXIMUM_WEAPONS_PER_UNIT];
	short rounds[MAXIMUM_WEAPONS_PER_UNIT][2][2];
	long fired_times[MAXIMUM_WEAPONS_PER_UNIT];
	long weapon_times[MAXIMUM_WEAPONS_PER_UNIT];
} objects_client_own_inventories[MAXIMUM_LOCAL_PLAYERS];
/* ... the weapon each of them last picked up (the host said so), its unit
and when, until it is readied (definition NONE: none) */
static struct distributed_picked_up_weapon
{
	long unit_index;
	long definition_index;
	long time;
} objects_client_picked_up_weapons[MAXIMUM_LOCAL_PLAYERS];
/* ... the host's inventories that named a unit or a weapon not here yet. An
inventory goes as a datagram, and units and weapons are made on the stream,
which can trail it, so a player just spawned can be told what it carries
before its unit or its weapons are here, and would carry nothing until the
host's next inventory of it, a second later. Each is kept, the newest of its
unit's, and applied once, the tick all it names is here; let go of once
three of the host's refreshes pass without a newer (the host has stopped
sending it) */
static struct distributed_retried_inventory
{
	struct distributed_inventory inventory;
	long time;
} objects_client_retried_inventories[MAXIMUM_CLIENT_RETRIED_INVENTORIES];
static short objects_client_retried_inventory_count;
/* ... whether it failed to make one of the host's objects since it last
asked for them (it says so when it asks again) */
static boolean objects_client_ask_again;
/* ... all of them (the host said so), when it last asked for them and how
long until it asks again */
static boolean objects_client_synchronized;
static long objects_client_ready_time;
static long objects_client_ready_interval;
/* ... when it last failed to make one (NONE: not since the host last told
it all), and how long it waits after the next failure */
static long objects_client_failed_time;
static long objects_client_retry_ticks;
/* ... past loading: its own objects from the upper half */
static boolean objects_client_local_allocation;
static short objects_client_local_identifier;
/* ... making the host's object at the host's index, NONE for none */
static long objects_client_creating_index = NONE;
static boolean objects_client_creating;
/* ... deleting one on the host's word */
static boolean objects_client_deleting;

/* for the automated tests' reports (network_test.c) */
static struct
{
	long creates;
	long deletes;
	long create_failures;
	long own_objects_removed;
} objects_statistics;

void network_distributed_item_statistics(
	long *creates,
	long *deletes,
	long *failures,
	long *removed)
{
	*creates = objects_statistics.creates;
	*deletes = objects_statistics.deletes;
	*failures = objects_statistics.create_failures;
	*removed = objects_statistics.own_objects_removed;
}

/* ---------- objects (objects.c) */

/* the datum index a new object takes: the host's, for its object a client
makes, or a client's own from the upper half; NONE for the first free */
long network_objects_new_object_index(
	void)
{
	long object_index = objects_client_creating_index;
	long object_count = object_header_data->actual_count;
	long absolute_index;

	/* (every object is made just after this: one more since the last, and
	none has been deleted, the indices below the lowest free one are still
	taken; an object made or deleted otherwise, and they are looked through
	from the first again) */
	if (objects_client_object_count == NONE || object_count != objects_client_object_count + 1)
		objects_client_local_free_index = LOCAL_OBJECTS_FIRST_INDEX;
	objects_client_object_count = object_count;
	if (object_index != NONE)
	{
		objects_client_creating_index = NONE;
		return object_index;
	}
	if (!objects_client_local_allocation || !network_game_distributed_client())
	{
		objects_client_object_count = NONE;
		return NONE;
	}
	for (absolute_index = MAX(objects_client_local_free_index, LOCAL_OBJECTS_FIRST_INDEX);
		absolute_index < object_header_data->maximum_count; absolute_index++)
	{
		struct datum_header const *header = (struct datum_header const *)
			((byte const *)object_header_data->data + absolute_index * object_header_data->size);

		if (!header->identifier)
		{
			objects_client_local_free_index = absolute_index;
			/* (any identifier but 0, which marks a free datum) */
			if (++objects_client_local_identifier == 0)
				objects_client_local_identifier = 1;
			object_index = ((long)objects_client_local_identifier << 16) | absolute_index;
			/* (of the kinds the host has, it goes: looked at next tick) */
			if (objects_client_new_object_count < MAXIMUM_CLIENT_NEW_OBJECTS)
				objects_client_new_objects[objects_client_new_object_count++] = object_index;
			else
				objects_client_check_all = TRUE;
			return object_index;
		}
	}
	/* (the upper half full: the first free, anywhere) */
	objects_client_local_free_index = object_header_data->maximum_count;
	objects_client_check_all = TRUE;
	return NONE;
}

/* whether a client is making the host's object: which the host has made as
the game type has it (object_new does not remap it again), with its own
parts (a unit's initial weapons are the host's objects too) */
boolean network_objects_creating_host_object(
	void)
{
	return objects_client_creating;
}

/* whether the object may be deleted: not the host's on a client, but on
the host's word */
boolean network_objects_may_delete(
	long object_index)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);

	return objects_client_deleting || !network_game_distributed_client() ||
		absolute_index < 0 || absolute_index >= MAXIMUM_TRACKED_OBJECTS ||
		objects_client_has[absolute_index] != object_index;
}

/* whether a client is in distributed_client_delete: deleting an object on
the host's word, or one of its own the host never told it of; a flag or
ball among them, which the game itself never deletes (objects.c, weapons.c) */
boolean network_objects_client_deleting(
	void)
{
	return objects_client_deleting;
}

/* the map's objects placed (game.c): a client's own come from the upper
half from now on */
void network_objects_placed(
	void)
{
	objects_client_local_allocation = network_game_distributed_client();
}

/* ---------- common */

static boolean distributed_object_networked(
	long object_index)
{
	struct object_header_datum *header = object_header_try_and_get(object_index);

	return header && header->datum && TEST_FLAG(NETWORKED_OBJECT_TYPES, header->type) &&
		!TEST_FLAG(header->flags, _object_header_being_deleted_bit) &&
		DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index) < MAXIMUM_TRACKED_OBJECTS;
}

boolean network_objects_client_has(
	long object_index)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);

	return object_index != NONE && absolute_index >= 0 && absolute_index < MAXIMUM_TRACKED_OBJECTS &&
		objects_client_has[absolute_index] == object_index && object_try_and_get(object_index);
}

word network_objects_entry_size(
	byte type)
{
	switch (type)
	{
	case _distributed_message_object_changes: return sizeof(struct distributed_object_change);
	case _distributed_message_object_states:
	case _distributed_message_vehicle_prediction: return sizeof(struct distributed_object_state);
	case _distributed_message_inventories: return sizeof(struct distributed_inventory);
	}
	return 0;
}

static boolean distributed_vector_valid(
	real_vector3d const *vector)
{
	return distributed_real_valid(vector->i) && distributed_real_valid(vector->j) && distributed_real_valid(vector->k);
}

/* whether a message's transform can be: the position finite and within the
world, the axes a rotation (made exactly one in valid_forward, valid_up),
the velocities finite */
static boolean distributed_transform_valid(
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real_vector3d const *velocity,
	real_vector3d const *angular_velocity,
	real_vector3d *valid_forward,
	real_vector3d *valid_up)
{
	if (!position || !forward || !up || !distributed_point_valid(position, OBJECT_WORLD_BOUND) ||
		(velocity && !distributed_vector_valid(velocity)) ||
		(angular_velocity && !distributed_vector_valid(angular_velocity)))
	{
		return FALSE;
	}
	*valid_forward = *forward;
	*valid_up = *up;
	return distributed_axes_make_valid(valid_forward, valid_up);
}

/* the vector no longer than maximum */
static void distributed_vector_clamp(
	real_vector3d *vector,
	real maximum)
{
	real length = (real)sqrt(vector->i * vector->i + vector->j * vector->j + vector->k * vector->k);

	if (length > maximum)
	{
		vector->i *= maximum / length;
		vector->j *= maximum / length;
		vector->k *= maximum / length;
	}
}

/* Whether the object is a vehicle no player drives in co-op (a Pelican on
its flight path): nothing on a client steers it, so its copy is kept at rest
there (no physics of its own) and moved only to where the host has it. In
multiplayer an empty vehicle (one bailed out of, or tumbling) keeps its own
physics between updates. */
static boolean distributed_vehicle_unsteered(
	long object_index)
{
	struct unit_datum *vehicle;

	if (!network_coop_active() || object_get(object_index)->object.type != _object_type_vehicle)
		return FALSE;
	vehicle = unit_get(object_index);
	return vehicle->unit.driver_object_index == NONE ||
		unit_get(vehicle->unit.driver_object_index)->unit.player_index == NONE;
}

/* whether a client keeps the vehicle only where the host has it (the riding
code measures its rider by elapsed time, not by its updates). Not the host's
copy of a co-op vehicle, which is simulated there as in any game */
boolean network_objects_vehicle_followed(
	long vehicle_index)
{
	return game_connection() == _game_connection_network_client && distributed_vehicle_unsteered(vehicle_index);
}

/* the object if it is a body (a dead biped), else NULL */
static struct biped_datum *distributed_body_get(
	long object_index)
{
	struct biped_datum *biped = (struct biped_datum *)object_try_and_get_and_verify_type(object_index,
		_object_mask_biped);

	return biped && TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ? biped : NULL;
}

/* A body lying limp here, which the host moves: its pose, fixed in the world
where it came to rest (bipeds.c), would stay there as the body goes, drawn
hanging in the air or in a wall. Unfixed, as the push of the explosion that
moved the host's body would have unfixed it here (that push reaches only the
machines that see the body's player, network_damage.c); it lies limp again
where it comes to rest (units.c). */
static void distributed_body_unfix_pose(
	long object_index)
{
	if (distributed_body_get(object_index))
		biped_stop_limp_body_physics(object_index);
}

/* A body at rest or moving as the host has it. Moving, its pose goes with
it, however close it is. At rest, it is on the ground, as the host's is:
not airborne, which only its physics clears, and rest stops its physics
(bipeds.c); left airborne, it would keep the pose of its fall, in the air,
and never lie limp. */
static void distributed_body_set_at_rest(
	long object_index,
	boolean at_rest)
{
	struct biped_datum *biped = distributed_body_get(object_index);

	if (!biped)
		return;
	if (!at_rest)
		distributed_body_unfix_pose(object_index);
	else
		SET_FLAG(biped->biped.flags, _biped_airborne_bit, FALSE);
}

/* whether this machine's player drives the unit: it is their biped, or the
vehicle they drive */
static boolean distributed_unit_driven_here(
	long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);

	if (unit->unit.driver_object_index != NONE)
		unit = unit_get(unit->unit.driver_object_index);
	return unit->unit.player_index != NONE && distributed_player_is_local(unit->unit.player_index);
}

/* Whether a client puts its copy exactly where the host has it, rather than
closing on it: an unsteered vehicle, and anything in a co-op cutscene
(where it keeps up with the host's camera). */
static boolean distributed_object_follows_host(
	long object_index)
{
	return (network_coop_active() && cinematic_in_progress()) || distributed_vehicle_unsteered(object_index);
}

/* whether a write of a vehicle copy is a host state correction a client makes of a copy it does not follow
(the host's own copy, its driving prediction, an own, co-op or changed vehicle and a carry are other writes). A
correction is no motion of the vehicle: what the written hull newly touches moves with it, as a rider is carried,
rather than being pushed or crushed as if the vehicle had driven into it */
static boolean network_objects_vehicle_carries(
	long object_index,
	short kind)
{
	return kind == _network_vehicle_write_state && game_connection() == _game_connection_network_client &&
		!network_objects_vehicle_followed(object_index) && vehicle_riding_enabled() &&
		object_get(object_index)->object.type == _object_type_vehicle;
}

/* before the write, while the hull is still where it was (what it touches is measured against that) */
static void network_objects_vehicle_writing(
	long object_index,
	short kind)
{
	if (network_objects_vehicle_carries(object_index, kind))
		biped_riding_vehicle_writing(object_index);
}

/* (the transform checked) */
static void distributed_object_move(
	long object_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real_vector3d const *velocity,
	real_vector3d const *angular_velocity,
	short kind)
{
	struct object_datum *object = object_get(object_index);
	real_point3d before = object->object.position;
	real_vector3d before_forward = object->object.forward;
	real_vector3d before_up = object->object.up;
	real_vector3d offset;

	/* (drawn from where it was, the difference fading over a few ticks) */
	offset.i = object->object.position.x - position->x;
	offset.j = object->object.position.y - position->y;
	offset.k = object->object.position.z - position->z;
	distributed_body_unfix_pose(object_index);
	network_objects_vehicle_writing(object_index, kind);
	object_set_position(object_index, position, forward, up);
	network_objects_vehicle_written(object_index, &before, &before_forward, &before_up, kind);
	if (velocity)
		object->object.translational_velocity = *velocity;
	if (angular_velocity)
		object->object.angular_velocity = *angular_velocity;
	if (!distributed_object_follows_host(object_index))
		render_interpolation_correct_object(object_index, &offset);
	if (distributed_vehicle_unsteered(object_index))
		SET_FLAG(object->object.flags, _object_at_rest_bit, TRUE);
}

void network_objects_correct(
	long object_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real_vector3d const *velocity,
	real_vector3d const *angular_velocity,
	short kind)
{
	real_vector3d valid_forward, valid_up;

	/* (what cannot be, from a message: not taken) */
	if (distributed_transform_valid(position, forward, up, velocity, angular_velocity, &valid_forward, &valid_up))
		distributed_object_move(object_index, position, &valid_forward, &valid_up, velocity, angular_velocity, kind);
}

boolean network_objects_reconcile(
	long object_index,
	real_point3d const *position,
	real_vector3d const *forward,
	real_vector3d const *up,
	real_vector3d const *velocity,
	real_vector3d const *angular_velocity,
	real blend_distance,
	short kind)
{
	struct object_datum *object = object_get(object_index);
	real_vector3d valid_forward, valid_up;
	real_point3d blended;
	real dx, dy, dz;

	/* (what cannot be, from a message: not taken) */
	if (!distributed_transform_valid(position, forward, up, velocity, angular_velocity, &valid_forward, &valid_up))
		return FALSE;
	dx = position->x - object->object.position.x;
	dy = position->y - object->object.position.y;
	dz = position->z - object->object.position.z;
	/* (so written that its own position gone wrong, not a number, is put
	right) */
	if (!(dx * dx + dy * dy + dz * dz <= blend_distance * blend_distance))
	{
		distributed_object_move(object_index, position, &valid_forward, &valid_up, velocity, angular_velocity, kind);
		return TRUE;
	}
	/* Move halfway, and draw the move as a glide over the next few frames,
	as larger corrections are. Drawn as one step, a unit corrected every tick
	or two (a distant enemy) visibly hitched each time. */
	if (distributed_object_follows_host(object_index))
		blended = *position;
	else
	{
		blended.x = object->object.position.x + dx * 0.5f;
		blended.y = object->object.position.y + dy * 0.5f;
		blended.z = object->object.position.z + dz * 0.5f;
	}
	{
		real_point3d before = object->object.position;
		real_vector3d before_forward = object->object.forward;
		real_vector3d before_up = object->object.up;
		real_vector3d offset;

		offset.i = object->object.position.x - blended.x;
		offset.j = object->object.position.y - blended.y;
		offset.k = object->object.position.z - blended.z;
		distributed_body_unfix_pose(object_index);
		network_objects_vehicle_writing(object_index, kind);
		object_set_position(object_index, &blended, &valid_forward, &valid_up);
		network_objects_vehicle_written(object_index, &before, &before_forward, &before_up, kind);
		if (!distributed_object_follows_host(object_index))
			render_interpolation_correct_object(object_index, &offset);
	}
	if (velocity)
		object->object.translational_velocity = *velocity;
	if (angular_velocity)
		object->object.angular_velocity = *angular_velocity;
	if (distributed_vehicle_unsteered(object_index))
		SET_FLAG(object->object.flags, _object_at_rest_bit, TRUE);
	return FALSE;
}

void network_objects_vehicle_written(
	long object_index,
	real_point3d const *before_position,
	real_vector3d const *before_forward,
	real_vector3d const *before_up,
	short kind)
{
	struct object_datum *object = object_get(object_index);

	if (object->object.type == _object_type_vehicle)
	{
		if (!network_objects_vehicle_followed(object_index))
			vehicle_riding_vehicle_corrected(object_index, before_position, before_forward, before_up,
				&object->object.position, &object->object.forward, &object->object.up);
		if (network_objects_vehicle_carries(object_index, kind))
			biped_riding_vehicle_written(object_index, before_position, before_forward, before_up);
	}
}

/* an object's state as the host sent it, in full */
static void distributed_object_state_unpack(
	struct distributed_object_state const *state,
	real_vector3d *forward,
	real_vector3d *up,
	real_vector3d *velocity,
	real_vector3d *angular_velocity)
{
	/* (as they came: distributed_transform_valid makes them a rotation or
	refuses them) */
	distributed_vector_unpack(&state->forward, DISTRIBUTED_UNIT_SCALE, forward);
	distributed_vector_unpack(&state->up, DISTRIBUTED_UNIT_SCALE, up);
	distributed_vector_unpack(&state->translational_velocity, DISTRIBUTED_VELOCITY_SCALE, velocity);
	distributed_vector_unpack(&state->angular_velocity, DISTRIBUTED_ANGULAR_VELOCITY_SCALE, angular_velocity);
}

static void distributed_state_from_object(
	long object_index,
	struct distributed_object_state *state)
{
	struct object_datum *object = object_get(object_index);

	csmemset(state, 0, sizeof(*state));
	state->object_index = object_index;
	SET_FLAG(state->flags, _distributed_object_at_rest_bit, TEST_FLAG(object->object.flags, _object_at_rest_bit));
	SET_FLAG(state->flags, _distributed_object_dead_bit, TEST_FLAG(_object_mask_unit, object->object.type) &&
		TEST_FLAG(object->object.damage_flags, _object_dead_bit));
	if (TEST_FLAG(_object_mask_unit, object->object.type))
	{
		SET_FLAG(state->flags, _distributed_object_light_sent_bit, TRUE);
		SET_FLAG(state->flags, _distributed_object_light_on_bit,
			TEST_FLAG(unit_get(object_index)->unit.flags, _unit_integrated_light_on_bit));
	}
	state->position = object->object.position;
	distributed_vector_pack(&object->object.forward, DISTRIBUTED_UNIT_SCALE, &state->forward);
	distributed_vector_pack(&object->object.up, DISTRIBUTED_UNIT_SCALE, &state->up);
	distributed_vector_pack(&object->object.translational_velocity, DISTRIBUTED_VELOCITY_SCALE,
		&state->translational_velocity);
	distributed_vector_pack(&object->object.angular_velocity, DISTRIBUTED_ANGULAR_VELOCITY_SCALE,
		&state->angular_velocity);
}

/* the vehicle the player's unit drives, or NONE */
static long distributed_driven_vehicle(
	struct player_datum const *player)
{
	long unit_index = distributed_living_unit(player);
	struct unit_datum *vehicle;

	if (unit_index == NONE || object_get(unit_index)->object.parent_object_index == NONE)
		return NONE;
	vehicle = (struct unit_datum *)object_try_and_get_and_verify_type(
		object_get(unit_index)->object.parent_object_index, _object_mask_vehicle);
	return vehicle && vehicle->unit.driver_object_index == unit_index ?
		object_get(unit_index)->object.parent_object_index : NONE;
}

/* ---------- damage animations

A unit's flinches and deaths are animations picked by where the damage came
from and at random among their permutations, so a client replaying the
host's damage could pick others. The host sends its picks (each in three
ticks' messages, in case one is lost). A client plays the host's pick when
it gets there first, or switches to it if its own has only just begun
(unit_port_correct_damage_animation). */

enum
{
	DAMAGE_ANIMATION_SENDS = 3,
	MAXIMUM_QUEUED_DAMAGE_ANIMATIONS = 32,
	/* how long a client keeps the host's pick for a hit it hasn't replayed yet */
	DAMAGE_ANIMATION_KEPT_TICKS = 2 * TICKS_PER_SECOND,
};

/* type: units.h's _unit_damage_animation_soft_ping and the rest; number:
the pick's, so a client tells a pick sent again from a new one */
struct distributed_damage_animation
{
	long unit_index;
	short animation_index;
	byte type;
	byte number;
};

struct distributed_damage_animations_message
{
	struct distributed_message_header header;
	struct distributed_damage_animation animations[MAXIMUM_QUEUED_DAMAGE_ANIMATIONS];
};

/* host: the picks still to be sent */
static struct
{
	struct distributed_damage_animation animation;
	short sends;
} host_damage_animations[MAXIMUM_QUEUED_DAMAGE_ANIMATIONS];
static short host_damage_animation_count;
static byte host_damage_animation_number;

/* client: the host's latest pick of each type for each unit, by absolute
index: its number, and when it came (NONE once played) */
static struct
{
	long unit_index;
	short animation_index[NUMBER_OF_UNIT_DAMAGE_ANIMATIONS];
	byte number[NUMBER_OF_UNIT_DAMAGE_ANIMATIONS];
	long time[NUMBER_OF_UNIT_DAMAGE_ANIMATIONS];
} client_damage_animations[MAXIMUM_TRACKED_OBJECTS];

/* whether the animation is one of the unit's graph */
static boolean distributed_unit_has_animation(
	long unit_index,
	short animation_index)
{
	long graph_index = unit_definition_get(unit_get(unit_index)->definition_index)->object.animation_graph.index;

	return graph_index != NONE && animation_index >= 0 &&
		animation_index < animation_graph_definition_get(graph_index)->animations.count;
}

/* units.c, as a unit flinches or dies: the host notes the animation it
picked; a client takes the host's pick when it has one */
short network_objects_damage_animation(
	long unit_index,
	short type,
	short animation_index)
{
	short connection = game_connection();
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_index);

	if (type < 0 || type >= NUMBER_OF_UNIT_DAMAGE_ANIMATIONS)
		return animation_index;
	if (connection == _game_connection_network_server && animation_index != NONE &&
		host_damage_animation_count < MAXIMUM_QUEUED_DAMAGE_ANIMATIONS)
	{
		struct distributed_damage_animation *animation =
			&host_damage_animations[host_damage_animation_count].animation;

		animation->unit_index = unit_index;
		animation->animation_index = animation_index;
		animation->type = (byte)type;
		animation->number = ++host_damage_animation_number;
		host_damage_animations[host_damage_animation_count++].sends = DAMAGE_ANIMATION_SENDS;
	}
	else if (connection == _game_connection_network_client && absolute_index >= 0 &&
		absolute_index < MAXIMUM_TRACKED_OBJECTS && client_damage_animations[absolute_index].unit_index == unit_index)
	{
		long *time = &client_damage_animations[absolute_index].time[type];

		if (*time != NONE && game_time_get() - *time < DAMAGE_ANIMATION_KEPT_TICKS)
		{
			*time = NONE;
			return client_damage_animations[absolute_index].animation_index[type];
		}
	}
	return animation_index;
}

/* host, each tick: the picks still to be sent, to every client */
static void distributed_host_send_damage_animations(
	void)
{
	struct distributed_damage_animations_message message;
	short count = 0;
	short index;

	for (index = 0; index < host_damage_animation_count; index++)
	{
		message.animations[count++] = host_damage_animations[index].animation;
		if (--host_damage_animations[index].sends <= 0)
			host_damage_animations[index--] = host_damage_animations[--host_damage_animation_count];
	}
	if (count > 0)
	{
		distributed_send(&message, _distributed_message_damage_animations, count,
			(word)(sizeof(message.header) + count * sizeof(message.animations[0])), _distributed_to_clients);
	}
}

word network_objects_damage_animation_entry_size(
	void)
{
	return sizeof(struct distributed_damage_animation);
}

void network_objects_handle_damage_animations(
	void const *entries,
	short count)
{
	struct distributed_damage_animation const *animations = entries;
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_damage_animation const *animation = &animations[index];
		long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(animation->unit_index);
		short type;

		if (!distributed_object_index_valid(animation->unit_index) || absolute_index >= MAXIMUM_TRACKED_OBJECTS ||
			animation->type >= NUMBER_OF_UNIT_DAMAGE_ANIMATIONS || !network_objects_client_has(animation->unit_index) ||
			!object_try_and_get_and_verify_type(animation->unit_index, _object_mask_unit) ||
			!distributed_unit_has_animation(animation->unit_index, animation->animation_index))
		{
			continue;
		}
		if (client_damage_animations[absolute_index].unit_index != animation->unit_index)
		{
			csmemset(&client_damage_animations[absolute_index], 0, sizeof(client_damage_animations[absolute_index]));
			client_damage_animations[absolute_index].unit_index = animation->unit_index;
			for (type = 0; type < NUMBER_OF_UNIT_DAMAGE_ANIMATIONS; type++)
				client_damage_animations[absolute_index].time[type] = NONE;
		}
		/* (a pick sent again) */
		else if (client_damage_animations[absolute_index].number[animation->type] == animation->number)
		{
			continue;
		}
		client_damage_animations[absolute_index].number[animation->type] = animation->number;
		client_damage_animations[absolute_index].animation_index[animation->type] = animation->animation_index;
		/* (replayed here already: switched to the host's if it has only just begun) */
		client_damage_animations[absolute_index].time[animation->type] =
			unit_port_correct_damage_animation(animation->unit_index, animation->type, animation->animation_index) ?
			NONE : game_time_get();
	}
}

/* ---------- the host */

static void distributed_change_from_object(
	long object_index,
	struct distributed_object_change *change)
{
	struct object_datum *object = object_get(object_index);

	csmemset(change, 0, sizeof(*change));
	change->change = _object_change_create;
	change->object_index = object_index;
	change->definition_index = object->definition_index;
	change->owner_player_index = distributed_player_to_byte(object->object.owner_player_index);
	change->owner_team_index = object->object.owner_team_index;
	change->variant_number = object->object.variant_number;
	change->forced_shader_permutation_index = (byte)PIN(object->object.forced_shader_permutation_index, 0, 0xFF);
	change->position = object->object.position;
	change->forward = object->object.forward;
	change->up = object->object.up;
	change->translational_velocity = object->object.translational_velocity;
	change->angular_velocity = object->object.angular_velocity;
	csmemcpy(change->change_colors, object->object.base_change_colors, sizeof(change->change_colors));
	csmemcpy(change->region_permutations, object->object.region_permutations, sizeof(change->region_permutations));
	SET_FLAG(change->flags, _distributed_object_at_rest_bit, TEST_FLAG(object->object.flags, _object_at_rest_bit));
	SET_FLAG(change->flags, _distributed_object_dead_bit, TEST_FLAG(_object_mask_unit, object->object.type) &&
		TEST_FLAG(object->object.damage_flags, _object_dead_bit));
	if (TEST_FLAG(_object_mask_item, object->object.type))
	{
		struct item_datum *item = item_get(object_index);

		SET_FLAG(change->flags, _distributed_object_carried_bit,
			TEST_FLAG(item->item.flags, _item_attached_to_unit_bit) ||
			object->object.parent_object_index != NONE ||
			!TEST_FLAG(object->object.flags, _object_connected_to_map_bit));
	}
}

/* the objects made and deleted since the last time, to every client */
static void distributed_host_update_objects(
	void)
{
	static boolean seen[MAXIMUM_TRACKED_OBJECTS];
	struct distributed_object_change_message message;
	short count = 0;
	short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_object_change));
	struct object_iterator iterator;
	long absolute_index;

	/* (none set past those told of) */
	csmemset(seen, 0, objects_host_told_count * sizeof(seen[0]));
	object_iterator_new(&iterator, NETWORKED_OBJECT_TYPES, 0);
	while (object_iterator_next(&iterator))
	{
		if (!distributed_object_networked(iterator.index))
			continue;
		absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		seen[absolute_index] = TRUE;
		if (objects_host_told[absolute_index] == iterator.index)
			continue;
		/* (another object in the same place: that one is gone) */
		if (objects_host_told[absolute_index] != NONE)
		{
			csmemset(&message.changes[count], 0, sizeof(message.changes[count]));
			message.changes[count].change = _object_change_delete;
			message.changes[count].object_index = objects_host_told[absolute_index];
			objects_statistics.deletes++;
			if (++count == limit)
			{
				distributed_send(&message, _distributed_message_object_changes, count,
					(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)),
					_distributed_to_clients_reliably);
				count = 0;
			}
		}
		objects_host_told[absolute_index] = iterator.index;
		if (absolute_index >= objects_host_told_count)
			objects_host_told_count = absolute_index + 1;
		objects_statistics.creates++;
		distributed_change_from_object(iterator.index, &message.changes[count]);
		if (++count == limit)
		{
			distributed_send(&message, _distributed_message_object_changes, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)),
				_distributed_to_clients_reliably);
			count = 0;
		}
	}
	for (absolute_index = 0; absolute_index < objects_host_told_count; absolute_index++)
	{
		if (objects_host_told[absolute_index] == NONE || seen[absolute_index])
			continue;
		csmemset(&message.changes[count], 0, sizeof(message.changes[count]));
		message.changes[count].change = _object_change_delete;
		message.changes[count].object_index = objects_host_told[absolute_index];
		objects_host_told[absolute_index] = NONE;
		objects_statistics.deletes++;
		if (++count == limit)
		{
			distributed_send(&message, _distributed_message_object_changes, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)),
				_distributed_to_clients_reliably);
			count = 0;
		}
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_object_changes, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)),
			_distributed_to_clients_reliably);
	}
}

/* The order a newly loaded client receives the host's objects in, most
needed first: players' units (it may spectate them), the vehicles players
and AI ride, other units, then everything else. */
enum
{
	_send_rank_player_unit,
	_send_rank_vehicle,
	_send_rank_unit,
	_send_rank_other,
	NUMBER_OF_SEND_RANKS
};

static void distributed_host_send_inventories_to_machine(long machine_index);

static short distributed_object_send_rank(
	long object_index)
{
	struct object_datum *object = object_get(object_index);

	if (object->object.type == _object_type_vehicle)
		return _send_rank_vehicle;
	if (object_try_and_get_and_verify_type(object_index, _object_mask_unit))
		return unit_get(object_index)->unit.player_index != NONE ? _send_rank_player_unit : _send_rank_unit;
	return _send_rank_other;
}

/* a client has loaded the game: every object the host has, to it alone,
and word that that is all of them; asked again, only the word (they are on
their way ahead of it), unless it failed to make one since it last asked
(again) */
void network_objects_client_asked(
	long machine_index,
	boolean again)
{
	struct distributed_object_change_message message;
	short count = 0;
	short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_object_change));
	long absolute_index;
	short rank;
	long *player_list;

	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return;
	distributed_checkpoint_to_machine(machine_index);
	player_list = machine_get_player_list(machine_index);
	/* (another machine in its place has other players) */
	if ((again && game_time_get() - objects_host_machines[machine_index].time >= HOST_OBJECTS_RESEND_TICKS) ||
		objects_host_machines[machine_index].time == NONE ||
		csmemcmp(objects_host_machines[machine_index].player_indices, player_list,
			sizeof(objects_host_machines[machine_index].player_indices)) != 0)
	{
		objects_host_machines[machine_index].time = game_time_get();
		csmemcpy(objects_host_machines[machine_index].player_indices, player_list,
			sizeof(objects_host_machines[machine_index].player_indices));
	}
	else
	{
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_objects_synchronized, 0,
			(word)sizeof(message.header));
		return;
	}
	distributed_host_update_objects();
	for (absolute_index = 0; absolute_index < objects_host_told_count; absolute_index++)
	{
		/* (what every unit carries, with the next of them) */
		objects_host_inventories[absolute_index].checksum = 0;
		objects_host_inventories[absolute_index].weapons_checksum = 0;
	}
	for (rank = 0; rank < NUMBER_OF_SEND_RANKS; rank++)
	{
		for (absolute_index = 0; absolute_index < objects_host_told_count; absolute_index++)
		{
			long object_index = objects_host_told[absolute_index];

			if (object_index == NONE || distributed_object_send_rank(object_index) != rank)
				continue;
			distributed_change_from_object(object_index, &message.changes[count]);
			if (++count == limit)
			{
				distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_object_changes, count,
					(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)));
				count = 0;
			}
		}
	}
	if (count)
	{
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_object_changes, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_object_change)));
	}
	distributed_host_send_inventories_to_machine(machine_index);
	distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_objects_synchronized, 0,
		(word)sizeof(message.header));
}

/* A checkpoint can restore a deleted object, or revive one under the same
handle. Neither is a new object to the host's change tracker. Send fresh
descriptions immediately: waiting for a client's rate-limited recovery can
leave its player dead for eleven seconds. Only the host's own revert bypasses
that limit; ordinary client requests retain it. */
void network_objects_host_reverted(void)
{
	long machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short count, index;

	if (game_connection() != _game_connection_network_server || !network_coop_active())
		return;
	count = distributed_client_machines(machines, NUMBEROF(machines));
	for (index = 0; index < count; index++)
	{
		long machine = machines[index];

		if (machine < 0 || machine >= HALO_PORT_MAXIMUM_NETWORK_MACHINES ||
			objects_host_machines[machine].time == NONE)
			continue;
		objects_host_machines[machine].time = NONE;
		network_objects_client_asked(machine, TRUE);
	}
}

/* the host's object at an absolute index whose state goes to its clients
(not players' living units, which have their own messages, nor what is
attached or carried), NONE for none */
static long distributed_host_placed_object(
	long absolute_index)
{
	long object_index = objects_host_told[absolute_index];
	struct object_datum *object;

	if (object_index == NONE || !object_try_and_get(object_index))
		return NONE;
	object = object_get(object_index);
	if (object->object.parent_object_index != NONE || !TEST_FLAG(object->object.flags, _object_connected_to_map_bit))
		return NONE;
	/* (out of the map, where nobody sees it: a body fallen through the
	ground falls on for good, ever faster, never at rest) */
	if (TEST_FLAG(object->object.flags, _object_outside_of_map_bit))
		return NONE;
	if (TEST_FLAG(_object_mask_unit, object->object.type) && unit_get(object_index)->unit.player_index != NONE &&
		!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
	{
		return NONE;
	}
	return object_index;
}

/* each client machine: where its players' living units are, and the
vehicles they drive, once a tick */
static void distributed_host_find_viewers(
	void)
{
	short machine_number;

	struct data_iterator iterator;
	struct player_datum *player;

	objects_host_viewers.count = distributed_client_machines(objects_host_viewers.indices,
		HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	objects_host_viewers.player_unit_count = 0;
	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL &&
		objects_host_viewers.player_unit_count < MAXIMUM_TRACKED_PLAYERS)
	{
		long unit_index = distributed_living_unit(player);

		if (unit_index != NONE)
		{
			object_get_origin(unit_index,
				&objects_host_viewers.player_unit_origins[objects_host_viewers.player_unit_count++]);
		}
	}
	for (machine_number = 0; machine_number < objects_host_viewers.count; machine_number++)
	{
		long *player_list = machine_get_player_list(objects_host_viewers.indices[machine_number]);
		short local_player_index;

		objects_host_viewers.machines[machine_number].unit_count = 0;
		objects_host_viewers.machines[machine_number].vehicle_count = 0;
		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
		{
			struct player_datum *player = player_list[local_player_index] != NONE ?
				player_try_and_get(player_list[local_player_index]) : NULL;
			long unit_index = distributed_living_unit(player);
			long vehicle_index = unit_index != NONE ? distributed_driven_vehicle(player) : NONE;
			short count;

			if (unit_index == NONE)
				continue;
			count = objects_host_viewers.machines[machine_number].unit_count++;
			objects_host_viewers.machines[machine_number].unit_indices[count] = unit_index;
			object_get_origin(unit_index, &objects_host_viewers.machines[machine_number].origins[count]);
			if (vehicle_index != NONE)
			{
				count = objects_host_viewers.machines[machine_number].vehicle_count++;
				objects_host_viewers.machines[machine_number].vehicle_indices[count] = vehicle_index;
				objects_host_viewers.machines[machine_number].vehicle_player_indices[count] =
					(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(player_list[local_player_index]);
			}
		}
	}
}

/* How often, in ticks, the host sends a moving object's position to a
client machine, based on its distance from that machine's nearest player.
A machine with no player in the world (dead or joining) spectates someone,
so the distance is measured from every player instead. Sending such a
machine every moving object every tick flooded it on campaign levels and
threw everything it saw out of sync. */
static short distributed_host_object_period(
	short machine_number,
	real_point3d const *position)
{
	short count = objects_host_viewers.machines[machine_number].unit_count;
	real_point3d const *origins = objects_host_viewers.machines[machine_number].origins;
	real nearest = -1.0f;
	short index;

	if (!count)
	{
		count = objects_host_viewers.player_unit_count;
		origins = objects_host_viewers.player_unit_origins;
	}
	/* in a co-op cutscene every client looks through the host's camera, so
	what it films is sent as often as what's next to the client's players
	(b30's sword Elite walking out of the door) */
	if (network_coop_active() && cinematic_in_progress())
	{
		real_point3d const *camera = &observer_get_camera(0)->position;
		real dx = position->x - camera->x;
		real dy = position->y - camera->y;
		real dz = position->z - camera->z;

		nearest = dx * dx + dy * dy + dz * dz;
	}
	/* nobody is alive anywhere, so nothing needs to be sent often */
	if (!count && nearest < 0.0f)
		return MAXIMUM_OBJECT_PERIOD_TICKS;
	for (index = 0; index < count; index++)
	{
		real_point3d const *origin = &origins[index];
		real dx = position->x - origin->x;
		real dy = position->y - origin->y;
		real dz = position->z - origin->z;
		real distance_squared = dx * dx + dy * dy + dz * dz;

		if (nearest < 0.0f || distance_squared < nearest)
			nearest = distance_squared;
	}
	return nearest < NEAR_OBJECT_DISTANCE * NEAR_OBJECT_DISTANCE ? 1 :
		nearest < MIDDLE_OBJECT_DISTANCE * MIDDLE_OBJECT_DISTANCE ? 2 :
		nearest < FAR_OBJECT_DISTANCE * FAR_OBJECT_DISTANCE ? 3 : MAXIMUM_OBJECT_PERIOD_TICKS;
}

/* ... for other units sent by a machine's index (network_actors.c) */
short network_objects_send_period(
	long machine_index,
	real_point3d const *position)
{
	short machine_number;

	for (machine_number = 0; machine_number < objects_host_viewers.count; machine_number++)
	{
		if (objects_host_viewers.indices[machine_number] == machine_index)
			return distributed_host_object_period(machine_number, position);
	}
	return 1;
}

/* the kinds of states sent this tick */
enum
{
	/* moving: to each client as often as it is near its players */
	_host_state_moving,
	/* come to rest, or at rest in its turn: to every client */
	_host_state_to_all,
};

/* whether an object at rest is due to every client again: it came to rest
this tick, or REST_STATE_REPEAT_TICKS after one of its sends */
static boolean distributed_host_rest_state_due(
	long absolute_index)
{
	long since = game_time_get() - objects_host_rest_times[absolute_index];

	return objects_host_rest_times[absolute_index] != NONE &&
		since < REST_STATE_REPEATS * REST_STATE_REPEAT_TICKS && since % REST_STATE_REPEAT_TICKS == 0;
}

/* where the moving objects are, to each client those near its players
every tick, those further less often, and the vehicle it drives itself
(which it has already) every few ticks, with which of its ticks the host
has it at; to every client each as it comes to rest (REST_STATE_REPEATS
times), and a few at rest, round them all */
static void distributed_host_send_states(
	void)
{
	/* (one come to rest may be in its turn too) */
	static struct distributed_object_state states[MAXIMUM_TRACKED_OBJECTS + RESTING_STATES_PER_TICK];
	static byte kinds[MAXIMUM_TRACKED_OBJECTS + RESTING_STATES_PER_TICK];
	struct distributed_object_state_message message;
	short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, DATAGRAM_ENTRIES(struct distributed_object_state));
	short resting = 0;
	long told_count = objects_host_told_count;
	long resting_start = objects_host_resting_cursor;
	long state_count = 0;
	long absolute_index;
	long step;
	short machine_number;

	/* the moving ones, and those at rest now that were not */
	for (absolute_index = 0; absolute_index < told_count; absolute_index++)
	{
		long object_index = distributed_host_placed_object(absolute_index);
		boolean was_moving = objects_host_state_moving[absolute_index];
		boolean at_rest;

		objects_host_state_moving[absolute_index] = FALSE;
		if (object_index == NONE)
			continue;
		at_rest = TEST_FLAG(object_get(object_index)->object.flags, _object_at_rest_bit);
		objects_host_state_moving[absolute_index] = !at_rest;
		if (at_rest && was_moving)
			objects_host_rest_times[absolute_index] = game_time_get();
		if (at_rest && !distributed_host_rest_state_due(absolute_index))
			continue;
		distributed_state_from_object(object_index, &states[state_count]);
		kinds[state_count++] = at_rest ? _host_state_to_all : _host_state_moving;
	}
	/* those at rest from the cursor on, round to it */
	for (step = 0; step < told_count && resting < RESTING_STATES_PER_TICK; step++)
	{
		long object_index;

		/* (the scan counts steps from where it began: from the cursor it moves, it skips rested objects, and a
		rested hull a rider stands on goes unrefreshed; riding off keeps the original scan) */
		absolute_index = ((vehicle_riding_enabled() ? resting_start : objects_host_resting_cursor) + step) % told_count;
		object_index = distributed_host_placed_object(absolute_index);
		if (object_index == NONE || !TEST_FLAG(object_get(object_index)->object.flags, _object_at_rest_bit))
			continue;
		resting++;
		objects_host_resting_cursor = (absolute_index + 1) % told_count;
		distributed_state_from_object(object_index, &states[state_count]);
		kinds[state_count++] = _host_state_to_all;
	}
	for (machine_number = 0; machine_number < objects_host_viewers.count; machine_number++)
	{
		long machine_index = objects_host_viewers.indices[machine_number];
		short count = 0;
		long index;

		for (index = 0; index < state_count; index++)
		{
			struct distributed_object_state *state = &message.states[count];
			short own_vehicle = NONE;
			short vehicle_number;

			for (vehicle_number = 0; vehicle_number < objects_host_viewers.machines[machine_number].vehicle_count;
				vehicle_number++)
			{
				if (objects_host_viewers.machines[machine_number].vehicle_indices[vehicle_number] ==
					states[index].object_index)
				{
					own_vehicle = vehicle_number;
				}
			}
			if (kinds[index] == _host_state_moving)
			{
				short period = own_vehicle != NONE ? OWN_VEHICLE_PERIOD_TICKS :
					distributed_host_object_period(machine_number, &states[index].position);

				if ((game_time_get() + (own_vehicle != NONE ? 0 : DATUM_INDEX_TO_ABSOLUTE_INDEX(states[index].object_index))) %
					period != 0)
				{
					continue;
				}
			}
			*state = states[index];
			/* (its own vehicle where it had it: at which of its ticks) */
			if (own_vehicle != NONE)
			{
				short player_index = objects_host_viewers.machines[machine_number].vehicle_player_indices[own_vehicle];

				/* (the client's clock against the host's, which a gap or a
				prediction refused since leaves as it was) */
				if (player_index >= 0 && player_index < MAXIMUM_TRACKED_PLAYERS &&
					objects_host_vehicle_predictions[player_index].accepted_vehicle_index == state->object_index &&
					game_time_get() - objects_host_vehicle_predictions[player_index].accepted_host_time <
						VEHICLE_PREDICTION_REFERENCE_TICKS)
				{
					SET_FLAG(state->flags, _distributed_object_predicted_bit, TRUE);
					state->time = (short)(word)(objects_host_vehicle_predictions[player_index].accepted_time +
						game_time_get() - objects_host_vehicle_predictions[player_index].accepted_host_time);
				}
			}
			if (++count == limit)
			{
				distributed_send_to_machine(machine_index, &message, _distributed_message_object_states, count,
					(word)(sizeof(message.header) + count * sizeof(struct distributed_object_state)));
				count = 0;
			}
		}
		if (count)
		{
			distributed_send_to_machine(machine_index, &message, _distributed_message_object_states, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_object_state)));
		}
	}
}

static void distributed_inventory_from_unit(
	long unit_index,
	struct distributed_inventory *inventory)
{
	struct unit_datum *unit = unit_get(unit_index);
	short weapon_slot;

	csmemset(inventory, 0, sizeof(*inventory));
	inventory->unit_index = unit_index;
	csmemcpy(inventory->grenade_counts, unit->unit.grenade_counts, sizeof(inventory->grenade_counts));
	inventory->current_weapon_index = (char)unit->unit.current_weapon_index;
	for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
	{
		long weapon_index = unit->unit.weapon_object_indices[weapon_slot];
		struct weapon_datum *weapon = weapon_index != NONE ? weapon_try_and_get(weapon_index) : NULL;

		inventory->weapon_indices[weapon_slot] = NONE;
		if (weapon)
		{
			short magazine;

			inventory->weapon_indices[weapon_slot] = weapon_index;
			for (magazine = 0; magazine < 2; magazine++)
			{
				inventory->rounds_total[weapon_slot][magazine] = weapon->weapon.magazines[magazine].rounds_total;
				inventory->rounds_loaded[weapon_slot][magazine] = weapon->weapon.magazines[magazine].rounds_loaded;
			}
			{
				real age = weapon->weapon.age;

				/* (so written that one not a number is none) */
				age = age > 1.0f ? 1.0f : age >= 0.0f ? age : 0.0f;
				inventory->age[weapon_slot] = (word)(long)floor(age * AGE_SCALE + 0.5f);
			}
		}
	}
}

/* Complete the ordered snapshot, including empty inventories. The periodic
sender stops repeating empty units; a retained checkpoint replica can still
carry equipment from the discarded world after that window has elapsed. */
static void distributed_host_send_inventories_to_machine(long machine_index)
{
	struct distributed_inventory_message message;
	short count = 0;
	short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_inventory));
	long absolute_index;

	for (absolute_index = 0; absolute_index < objects_host_told_count; absolute_index++)
	{
		long object_index = objects_host_told[absolute_index];
		struct unit_datum *unit = object_index != NONE ?
			(struct unit_datum *)object_try_and_get_and_verify_type(object_index, _object_mask_unit) : NULL;

		if (!unit || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			continue;
		distributed_inventory_from_unit(object_index, &message.inventories[count]);
		if (++count == limit)
		{
			distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_inventories, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_inventory)));
			count = 0;
		}
	}
	if (count)
	{
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_inventories, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_inventory)));
	}
}

/* the bytes' checksum (FNV-1a) */
static unsigned long distributed_checksum(
	void const *data,
	long size)
{
	byte const *bytes = (byte const *)data;
	unsigned long checksum = 2166136261UL;
	long index;

	for (index = 0; index < size; index++)
		checksum = (checksum ^ bytes[index]) * 16777619UL;
	return checksum;
}

/* the kinds of inventories sent this time */
enum
{
	/* its weapons or grenades changed, or not sent for a while: to every
	client */
	_host_inventory_to_all,
	/* its ammunition alone changed: to its own client, and to the others as
	often as they are sent where it is */
	_host_inventory_ammunition,
	/* ... not now, but lately: to the others, in their turn */
	_host_inventory_ammunition_lately,
};

/* what every unit carries, when that has changed or not been sent for a
while: a change of weapons or grenades to every client at once, one of
ammunition alone to the unit's own client at once and to the others by how
near their players are, as the host sends them objects (those further off
see its shots less) */
static void distributed_host_send_inventories(
	void)
{
	static struct distributed_inventory inventories[MAXIMUM_TRACKED_OBJECTS];
	static byte kinds[MAXIMUM_TRACKED_OBJECTS];
	static real_point3d origins[MAXIMUM_TRACKED_OBJECTS];
	struct distributed_inventory_message message;
	short limit = MIN(MAXIMUM_ENTRIES_PER_MESSAGE, DATAGRAM_ENTRIES(struct distributed_inventory));
	long inventory_count = 0;
	long round = game_time_get() / INVENTORY_INTERVAL_TICKS;
	struct object_iterator iterator;
	short machine_number;

	object_iterator_new(&iterator, _object_mask_unit, 0);
	while (object_iterator_next(&iterator) && inventory_count < MAXIMUM_TRACKED_OBJECTS)
	{
		struct unit_datum *unit = unit_get(iterator.index);
		long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		struct distributed_inventory *inventory = &inventories[inventory_count];
		short weapon_slot;
		boolean carries = unit->unit.grenade_counts[0] || unit->unit.grenade_counts[1];
		unsigned long checksum;
		unsigned long weapons_checksum;
		byte kind;

		for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
			carries |= unit->unit.weapon_object_indices[weapon_slot] != NONE;
		if (!distributed_object_networked(iterator.index) || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			continue;
		/* (one carrying nothing, as it has come to for a while: its last
		weapon out of its hands everywhere, and then no more of it) */
		if (carries)
			objects_host_inventories[absolute_index].carried_time = game_time_get();
		else if (objects_host_inventories[absolute_index].carried_time == NONE ||
			game_time_get() - objects_host_inventories[absolute_index].carried_time >= EMPTY_INVENTORY_TICKS)
		{
			continue;
		}
		distributed_inventory_from_unit(iterator.index, inventory);
		checksum = distributed_checksum(inventory, sizeof(*inventory));
		/* (its weapons, the one in hand and its grenades: the bytes up to the
		ammunition) */
		weapons_checksum = distributed_checksum(inventory, (long)offsetof(struct distributed_inventory, rounds_total));
		if (objects_host_inventories[absolute_index].weapons_checksum != weapons_checksum ||
			game_time_get() - objects_host_inventories[absolute_index].time >= INVENTORY_REFRESH_TICKS)
		{
			kind = _host_inventory_to_all;
			objects_host_inventories[absolute_index].time = game_time_get();
			objects_host_inventories[absolute_index].ammunition_time = NONE;
		}
		else if (objects_host_inventories[absolute_index].checksum != checksum)
		{
			kind = _host_inventory_ammunition;
			objects_host_inventories[absolute_index].ammunition_time = game_time_get();
		}
		else if (objects_host_inventories[absolute_index].ammunition_time != NONE &&
			game_time_get() - objects_host_inventories[absolute_index].ammunition_time <
				MAXIMUM_OBJECT_PERIOD_TICKS * INVENTORY_INTERVAL_TICKS)
		{
			kind = _host_inventory_ammunition_lately;
		}
		else
		{
			continue;
		}
		objects_host_inventories[absolute_index].checksum = checksum;
		objects_host_inventories[absolute_index].weapons_checksum = weapons_checksum;
		kinds[inventory_count] = kind;
		object_get_origin(iterator.index, &origins[inventory_count]);
		inventory_count++;
	}
	for (machine_number = 0; machine_number < objects_host_viewers.count; machine_number++)
	{
		long machine_index = objects_host_viewers.indices[machine_number];
		short count = 0;
		long index;

		for (index = 0; index < inventory_count; index++)
		{
			if (kinds[index] != _host_inventory_to_all)
			{
				long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(inventories[index].unit_index);
				boolean own = FALSE;
				short unit_number;

				for (unit_number = 0; unit_number < objects_host_viewers.machines[machine_number].unit_count; unit_number++)
					own |= objects_host_viewers.machines[machine_number].unit_indices[unit_number] == inventories[index].unit_index;
				if (own)
				{
					/* (its own unit's: each change at once) */
					if (kinds[index] != _host_inventory_ammunition)
						continue;
				}
				else
				{
					/* (another's: every round near its players, further every
					second to fourth, in the unit's turn, which comes once in
					the rounds after each change that its period spans) */
					short period = distributed_host_object_period(machine_number, &origins[index]);

					if ((round + absolute_index) % period != 0 ||
						game_time_get() - objects_host_inventories[absolute_index].ammunition_time >=
							period * INVENTORY_INTERVAL_TICKS)
					{
						continue;
					}
				}
			}
			message.inventories[count] = inventories[index];
			if (++count == limit)
			{
				distributed_send_to_machine(machine_index, &message, _distributed_message_inventories, count,
					(word)(sizeof(message.header) + count * sizeof(struct distributed_inventory)));
				count = 0;
			}
		}
		if (count)
		{
			distributed_send_to_machine(machine_index, &message, _distributed_message_inventories, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_inventory)));
		}
	}
}

/* (the host) the vehicles a client's own players drive: the latest of
each, taken at the next tick */
void network_objects_handle_vehicle_prediction(
	long machine_index,
	void const *entries,
	short count)
{
	struct distributed_object_state const *states = (struct distributed_object_state const *)entries;
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_object_state const *state = &states[index];
		struct unit_datum *vehicle = distributed_object_index_valid(state->object_index) ?
			(struct unit_datum *)object_try_and_get_and_verify_type(state->object_index, _object_mask_vehicle) : NULL;
		short player_index;

		if (!vehicle || vehicle->unit.driver_object_index == NONE)
			continue;
		if (unit_get(vehicle->unit.driver_object_index)->unit.player_index == NONE)
			continue;
		player_index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(unit_get(vehicle->unit.driver_object_index)->unit.player_index);
		if (player_index < 0 || player_index >= MAXIMUM_TRACKED_PLAYERS ||
			!distributed_machine_has_player(machine_index, player_index))
		{
			continue;
		}
		objects_host_vehicle_predictions[player_index].valid = TRUE;
		objects_host_vehicle_predictions[player_index].machine_index = machine_index;
		objects_host_vehicle_predictions[player_index].state = *state;
	}
}

/* (the host) where a client's player's vehicle, which that player drives,
left the ground (network_objects_vehicle_below_ceiling): noted each tick,
from the host's copy */
static void network_objects_note_vehicle_ground(
	short player_index)
{
	long vehicle_index = objects_host_vehicle_predictions[player_index].accepted_vehicle_index;
	struct vehicle_datum *vehicle = vehicle_index != NONE ?
		(struct vehicle_datum *)object_try_and_get_and_verify_type(vehicle_index, _object_mask_vehicle) : NULL;
	struct unit_datum *driver;

	if (!vehicle || vehicle->unit.driver_object_index == NONE ||
		!(driver = unit_get(vehicle->unit.driver_object_index)) || driver->unit.player_index == NONE ||
		DATUM_INDEX_TO_ABSOLUTE_INDEX(driver->unit.player_index) != player_index)
	{
		objects_host_vehicle_predictions[player_index].ground_vehicle_index = NONE;
		return;
	}
	/* (on the ground, or one new to it: from where it is) */
	if (vehicle->vehicle.airborne_ticks == 0 ||
		objects_host_vehicle_predictions[player_index].ground_vehicle_index != vehicle_index)
	{
		objects_host_vehicle_predictions[player_index].ground_vehicle_index = vehicle_index;
		objects_host_vehicle_predictions[player_index].ground_time = game_time_get();
		objects_host_vehicle_predictions[player_index].ground_height = vehicle->object.position.z;
		objects_host_vehicle_predictions[player_index].ground_rise_speed =
			MAX(vehicle->object.translational_velocity.k, 0.0f);
		/* (so written that a speed not a number is none) */
		if (!(objects_host_vehicle_predictions[player_index].ground_rise_speed <= MAXIMUM_PREDICTED_VEHICLE_SPEED))
			objects_host_vehicle_predictions[player_index].ground_rise_speed = 0.0f;
	}
}

/* (the host) whether a client's player's vehicle that does not fly (nor
floats) may be where they say it is: on the ground anywhere its speed
takes it; in the air no higher above where it left the ground than a thing
thrown up as fast as it went up then (and a tolerance) falls to since (the
ticks since, less the client's round trip, no more than
VEHICLE_PREDICTION_CEILING_LEAD_TICKS, and jitter, which are the client's
ahead): no flying, nor hovering */
static boolean network_objects_vehicle_below_ceiling(
	short player_index,
	long vehicle_index,
	real_point3d const *position)
{
	struct vehicle_datum *vehicle = vehicle_datum_get(vehicle_index);
	short type = vehicle_specific_definition_get(vehicle->definition_index)->vehicle_type;
	real ticks;
	real ceiling;

	if (type == _vehicle_type_human_boat || type == _vehicle_type_human_plane ||
		type == _vehicle_type_alien_fighter || type == _vehicle_type_turret ||
		objects_host_vehicle_predictions[player_index].ground_vehicle_index != vehicle_index ||
		vehicle->vehicle.airborne_ticks == 0)
	{
		return TRUE;
	}
	ticks = (real)(game_time_get() - objects_host_vehicle_predictions[player_index].ground_time) -
		MIN(distributed_machine_round_trip_ticks(objects_host_vehicle_predictions[player_index].machine_index),
			(real)VEHICLE_PREDICTION_CEILING_LEAD_TICKS) -
		(real)VEHICLE_PREDICTION_JITTER_TICKS;
	if (!(ticks > 0.0f) || !(global_gravity > 0.0f))
		return TRUE;
	ceiling = VEHICLE_PREDICTION_RISE_TOLERANCE +
		(objects_host_vehicle_predictions[player_index].ground_rise_speed + PREDICTED_VEHICLE_SPEED_MARGIN) * ticks -
		0.5f * global_gravity * ticks * ticks;
	/* (so written that a position not a number is not below) */
	return position->z - objects_host_vehicle_predictions[player_index].ground_height <= ceiling;
}

/* ... taken as they are, within a tolerance, if that machine's player
still drives it: a little off closed by half, more put there. Each taken
moves the host's copy, and the next is measured from there, so a client
could take its vehicle HOST_VEHICLE_ACCEPT_TOLERANCE further each tick: a
prediction is also no further from the anchor (one taken, a newer once a
second) than the vehicle goes in the client's ticks since (after it, and no
more than the host's since and a little jitter), with a margin, and no
faster: as fast as twice its tag's top speed, or as the host's own ticks
sent its copy lately (thrown), whichever is faster. What they sent it is
its speed less as much as the velocity it took of the client was faster
than twice the top speed: a client that says it goes faster gains nothing
by it (its gravity on a copy said to hover and fall). A teleporter moves
the host's copy too, further than it goes since the last taken: it is
measured from where the host has it only, a new anchor. In the air it is
no higher than network_objects_vehicle_below_ceiling says. Which of the
client's ticks the host has it at is noted, to tell the client. */
void network_objects_apply_vehicle_predictions(
	void)
{
	long now = game_time_get();
	short player_index;

	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		struct distributed_object_state const *state = &objects_host_vehicle_predictions[player_index].state;
		struct unit_datum *vehicle;
		struct unit_datum *driver;
		real_vector3d forward, up, velocity, angular_velocity;
		real dx, dy, dz;
		real speed;
		boolean anchored;

		network_objects_note_vehicle_ground(player_index);
		if (!objects_host_vehicle_predictions[player_index].valid)
			continue;
		objects_host_vehicle_predictions[player_index].valid = FALSE;
		vehicle = (struct unit_datum *)object_try_and_get_and_verify_type(state->object_index, _object_mask_vehicle);
		if (!vehicle || vehicle->unit.driver_object_index == NONE || vehicle->object.parent_object_index != NONE)
			continue;
		driver = unit_get(vehicle->unit.driver_object_index);
		if (driver->unit.player_index == NONE ||
			DATUM_INDEX_TO_ABSOLUTE_INDEX(driver->unit.player_index) != player_index ||
			!distributed_machine_has_player(objects_host_vehicle_predictions[player_index].machine_index, player_index))
		{
			continue;
		}
		/* (not from a co-op client still loading the host's BSP: the
		vehicle falls there with no ground under it) */
		if (!network_coop_player_has_structure_bsp(driver->unit.player_index))
			continue;
		dx = state->position.x - vehicle->object.position.x;
		dy = state->position.y - vehicle->object.position.y;
		dz = state->position.z - vehicle->object.position.z;
		/* (so written that a position not a number is not taken) */
		if (!(dx * dx + dy * dy + dz * dz <= HOST_VEHICLE_ACCEPT_TOLERANCE * HOST_VEHICLE_ACCEPT_TOLERANCE))
			continue;
		/* (no higher in the air than it was thrown up leaving the ground,
		falling since: past that the host's copy falls as its ticks have it) */
		if (!network_objects_vehicle_below_ceiling(player_index, state->object_index, &state->position))
			continue;
		/* (how fast it may go, a tick) */
		{
			struct vehicle_definition const *definition = vehicle_specific_definition_get(vehicle->definition_index);
			real top_speed = 2.0f * MAX(definition->maximum_forward_speed, definition->maximum_reverse_speed);
			real_vector3d const *host_velocity = &vehicle->object.translational_velocity;
			real host_speed = (real)sqrt(host_velocity->i * host_velocity->i + host_velocity->j * host_velocity->j +
				host_velocity->k * host_velocity->k);
			long *speed_time = &objects_host_vehicle_predictions[player_index].speed_time;
			real *speeds = objects_host_vehicle_predictions[player_index].speeds;

			/* (so written that a tag's speed not a number is none) */
			if (!(top_speed >= 0.0f))
				top_speed = 0.0f;
			/* (what the host's ticks sent it since the last taken) */
			if (objects_host_vehicle_predictions[player_index].accepted_vehicle_index == state->object_index)
			{
				real_vector3d const *taken = &objects_host_vehicle_predictions[player_index].accepted_velocity;
				real taken_speed = (real)sqrt(taken->i * taken->i + taken->j * taken->j + taken->k * taken->k);

				if (taken_speed > top_speed)
					host_speed -= taken_speed - top_speed;
			}
			/* (the most of each span; so written that a speed not a number
			is none) */
			if (now < *speed_time || now - *speed_time >= 2 * VEHICLE_PREDICTION_SPEED_TICKS)
			{
				*speed_time = now;
				speeds[0] = 0.0f;
				speeds[1] = 0.0f;
			}
			else if (now - *speed_time >= VEHICLE_PREDICTION_SPEED_TICKS)
			{
				*speed_time = now;
				speeds[1] = speeds[0];
				speeds[0] = 0.0f;
			}
			if (host_speed > speeds[0])
				speeds[0] = host_speed;
			speed = MAX(top_speed, MAX(speeds[0], speeds[1]));
			speed = MIN(speed, MAXIMUM_PREDICTED_VEHICLE_SPEED);
		}
		anchored = objects_host_vehicle_predictions[player_index].accepted_vehicle_index == state->object_index &&
			now - objects_host_vehicle_predictions[player_index].anchor_host_time < VEHICLE_PREDICTION_REFERENCE_TICKS;
		if (anchored)
		{
			long host_ticks = now - objects_host_vehicle_predictions[player_index].accepted_host_time;
			real reach = (speed + PREDICTED_VEHICLE_SPEED_MARGIN) * (real)host_ticks + HOST_VEHICLE_BLEND_DISTANCE;
			real_point3d const *from = &objects_host_vehicle_predictions[player_index].accepted_host_position;

			dx = vehicle->object.position.x - from->x;
			dy = vehicle->object.position.y - from->y;
			dz = vehicle->object.position.z - from->z;
			/* (the host's copy moved further than it goes since the last
			taken, which only the host moves it by: taken from where it is,
			a new anchor) */
			if (!(dx * dx + dy * dy + dz * dz <= reach * reach))
			{
				anchored = FALSE;
			}
			else
			{
				/* (a client's clock that jumped counts no more than the
				host's ticks since and the jitter) */
				long ticks = MIN((long)(short)(word)((word)state->time - objects_host_vehicle_predictions[player_index].anchor_time),
					now - objects_host_vehicle_predictions[player_index].anchor_host_time + VEHICLE_PREDICTION_JITTER_TICKS);

				reach = (speed + PREDICTED_VEHICLE_SPEED_MARGIN) * (real)ticks + HOST_VEHICLE_BLEND_DISTANCE;
				from = &objects_host_vehicle_predictions[player_index].anchor_position;
				dx = state->position.x - from->x;
				dy = state->position.y - from->y;
				dz = state->position.z - from->z;
				if (ticks <= 0 || !(dx * dx + dy * dy + dz * dz <= reach * reach))
					continue;
			}
		}
		distributed_object_state_unpack(state, &forward, &up, &velocity, &angular_velocity);
		/* (no faster than it goes: the margin is for where it is only, else
		its copy would go faster by it each tick) */
		distributed_vector_clamp(&velocity, speed);
		distributed_vector_clamp(&angular_velocity, MAXIMUM_PREDICTED_VEHICLE_ANGULAR_SPEED);
		if (!distributed_transform_valid(&state->position, &forward, &up, &velocity, &angular_velocity, &forward, &up))
			continue;
		{
			real_point3d previous_position = vehicle->object.position;

			network_objects_reconcile(state->object_index, &state->position, &forward, &up, &velocity,
				&angular_velocity, HOST_VEHICLE_BLEND_DISTANCE, _network_vehicle_write_prediction);
			/* (a copy at rest runs no physics, and so neither falls nor
			counts its ticks in the air: one the client moved from where it
			came to rest (a little at a time too), or says moves, wakes, and
			settles again if it is still) */
			if (!TEST_FLAG(vehicle->object.flags, _object_at_rest_bit))
			{
				objects_host_vehicle_predictions[player_index].rest_vehicle_index = NONE;
			}
			else
			{
				if (objects_host_vehicle_predictions[player_index].rest_vehicle_index != state->object_index)
				{
					objects_host_vehicle_predictions[player_index].rest_vehicle_index = state->object_index;
					objects_host_vehicle_predictions[player_index].rest_position = previous_position;
				}
				dx = vehicle->object.position.x - objects_host_vehicle_predictions[player_index].rest_position.x;
				dy = vehicle->object.position.y - objects_host_vehicle_predictions[player_index].rest_position.y;
				dz = vehicle->object.position.z - objects_host_vehicle_predictions[player_index].rest_position.z;
				if (!(dx * dx + dy * dy + dz * dz <=
						VEHICLE_PREDICTION_STILL_DISTANCE * VEHICLE_PREDICTION_STILL_DISTANCE) ||
					velocity.i != 0.0f || velocity.j != 0.0f || velocity.k != 0.0f)
				{
					SET_FLAG(vehicle->object.flags, _object_at_rest_bit, FALSE);
					objects_host_vehicle_predictions[player_index].rest_vehicle_index = NONE;
				}
			}
		}
		objects_host_vehicle_predictions[player_index].accepted_vehicle_index = state->object_index;
		objects_host_vehicle_predictions[player_index].accepted_time = (word)state->time;
		objects_host_vehicle_predictions[player_index].accepted_host_time = now;
		objects_host_vehicle_predictions[player_index].accepted_host_position = vehicle->object.position;
		objects_host_vehicle_predictions[player_index].accepted_velocity = vehicle->object.translational_velocity;
		/* (a new anchor: the first, one a second on, or after a jump) */
		if (!anchored ||
			now - objects_host_vehicle_predictions[player_index].anchor_host_time >= VEHICLE_PREDICTION_ANCHOR_TICKS)
		{
			objects_host_vehicle_predictions[player_index].anchor_time = (word)state->time;
			objects_host_vehicle_predictions[player_index].anchor_host_time = now;
			objects_host_vehicle_predictions[player_index].anchor_position = state->position;
		}
	}
}

/* the host tick's steps, timed in the profiling build */
PROFILE_SECTION(objects_host_tick_update_objects_section, "network_objects_host_tick.update_objects")
PROFILE_SECTION(objects_host_tick_find_viewers_section, "network_objects_host_tick.find_viewers")
PROFILE_SECTION(objects_host_tick_send_states_section, "network_objects_host_tick.send_states")
PROFILE_SECTION(objects_host_tick_send_inventories_section, "network_objects_host_tick.send_inventories")
PROFILE_SECTION(objects_host_tick_send_damage_animations_section, "network_objects_host_tick.send_damage_animations")

void network_objects_host_tick(
	void)
{
	profile_scope(objects_host_tick_update_objects_section, distributed_host_update_objects();)
	profile_scope(objects_host_tick_find_viewers_section, distributed_host_find_viewers();)
	profile_scope(objects_host_tick_send_states_section, distributed_host_send_states();)
	if (game_time_get() % INVENTORY_INTERVAL_TICKS == 0)
		profile_scope(objects_host_tick_send_inventories_section, distributed_host_send_inventories();)
	profile_scope(objects_host_tick_send_damage_animations_section, distributed_host_send_damage_animations();)
	mcc_network_host_tick();
}

/* ---------- a client */

/* the item out of whatever unit has it */
static void distributed_client_release_item(
	long item_index,
	boolean deleting)
{
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_unit, 0);
	while (object_iterator_next(&iterator))
	{
		struct unit_datum *unit = unit_get(iterator.index);
		short weapon_slot;

		for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
		{
			if (unit->unit.weapon_object_indices[weapon_slot] != item_index)
				continue;
			if (deleting)
				unit_network_forget_weapon(iterator.index, weapon_slot);
			else
				unit_network_drop_weapon(iterator.index, weapon_slot);
		}
	}
}

/* deletes an object (the host's, on its word, or one the host has not
told of), undoing what refers to it first */
static void distributed_client_delete(
	long object_index)
{
	struct object_datum *object = object_try_and_get(object_index);

	if (!object)
		return;
	if (TEST_FLAG(_object_mask_item, object->object.type))
		distributed_client_release_item(object_index, TRUE);
	/* (a flag or ball, which the game never deletes but a host has, for a
	reason not known: the client follows its word all the same. Out of its
	carrier's hands above, as any weapon; and the goal its game type shows
	at it goes too) */
	if (object->object.type == _object_type_weapon && game_engine_running() && weapon_is_flag(object_index))
		game_engine_objective_weapon_deleted(object_index);
	if (TEST_FLAG(_object_mask_unit, object->object.type))
	{
		struct unit_datum *unit = unit_get(object_index);
		long child_index;
		short weapon_slot;

		/* (the host's weapons it carries out: deleted with it otherwise, one
		the host's unit had let go of before (its inventory not here yet)
		would be gone here for good; the host's word deletes those it deleted
		with its unit) */
		for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
		{
			if (unit->unit.weapon_object_indices[weapon_slot] != NONE &&
				network_objects_client_has(unit->unit.weapon_object_indices[weapon_slot]))
			{
				unit_network_drop_weapon(object_index, weapon_slot);
			}
		}
		child_index = unit->object.first_child_object_index;
		/* (its riders out first: they are deleted with it otherwise) */
		while (child_index != NONE)
		{
			long next_index = object_get(child_index)->object.next_object_index;

			if (TEST_FLAG(_object_mask_unit, object_get(child_index)->object.type) &&
				unit_get(child_index)->unit.parent_seat_index != NONE)
			{
				unit_exit_seat_end(child_index);
			}
			child_index = next_index;
		}
		if (unit->unit.player_index != NONE && player_try_and_get(unit->unit.player_index) &&
			player_get(unit->unit.player_index)->unit_index == object_index)
		{
			network_player_detach_unit(unit->unit.player_index);
		}
	}
	/* (a player's action on it, the prompt the hud draws: deleted between
	ticks, it would draw on with the object gone until the next tick looked
	again) */
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);
		while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
		{
			if (player->action_object_index == object_index)
			{
				player->action_result = _player_action_result_reload;
				player->action_object_index = NONE;
			}
		}
	}
	objects_client_deleting = TRUE;
	object_delete_immediately(object_index);
	objects_client_deleting = FALSE;
}

/* a create failed (the object pool full, its place not to be had): all of
them again from the host, a while after (asked again, flagged: the host
tells them all only then, not on each asking while they are on their way),
longer each time it fails again */
static void distributed_client_request_objects(
	void)
{
	/* (not asked since the last: that asking covers this one) */
	if (objects_client_failed_time != NONE && objects_client_ready_time <= objects_client_failed_time)
		return;
	objects_client_ask_again = TRUE;
	objects_client_failed_time = game_time_get();
	objects_client_synchronized = FALSE;
	objects_client_ready_time = game_time_get();
	objects_client_ready_interval = objects_client_retry_ticks;
	objects_client_retry_ticks = MIN(2 * objects_client_retry_ticks, CLIENT_RETRY_MAXIMUM_TICKS);
}

static void distributed_client_create_failed(void)
{
	objects_statistics.create_failures++;
	distributed_client_request_objects();
}

/* A co-op checkpoint may restore a unit under the same object handle after
this client's copy died. Request a full, validated host description; merely
clearing its dead bit would leave death animations and inventory state behind. */
void network_objects_expect_live_unit(long object_index)
{
	struct object_datum *object;

	if (!network_game_distributed_client() || !network_coop_active() ||
		!network_objects_client_has(object_index))
		return;
	object = object_try_and_get_and_verify_type(object_index, _object_mask_unit);
	if (object && TEST_FLAG(object->object.damage_flags, _object_dead_bit))
		distributed_client_request_objects();
}

/* whether the object is this machine's own player's unit or the vehicle it
drives (where they are is its own) */
static boolean distributed_client_own_object(
	long object_index)
{
	struct object_datum *object = object_get(object_index);
	long unit_index = object_index;

	if (!TEST_FLAG(_object_mask_unit, object->object.type))
		return FALSE;
	if (object->object.type == _object_type_vehicle)
		unit_index = unit_get(object_index)->unit.driver_object_index;
	return unit_index != NONE && distributed_player_is_local(unit_get(unit_index)->unit.player_index);
}

/* whether the host's object can be made: of the kinds the host has, its
definition a tag of the group its type says, at a whole index, of a team
the game has, where the world is (its axes made a rotation in forward, up) */
static boolean distributed_client_change_valid(
	struct distributed_object_change const *change,
	real_vector3d *forward,
	real_vector3d *up)
{
	long group_tag;
	short type;
	short team_count = MAXIMUM_TRACKED_PLAYERS;

	if (!distributed_object_index_valid(change->object_index) ||
		!tag_index_is_group(change->definition_index, OBJECT_DEFINITION_TAG))
	{
		return FALSE;
	}
	type = object_definition_get(change->definition_index)->object.type;
	switch (type)
	{
	case _object_type_biped: group_tag = BIPED_DEFINITION_TAG; break;
	case _object_type_vehicle: group_tag = VEHICLE_DEFINITION_TAG; break;
	case _object_type_weapon: group_tag = WEAPON_DEFINITION_TAG; break;
	case _object_type_equipment: group_tag = EQUIPMENT_DEFINITION_TAG; break;
	default: return FALSE;
	}
	if (!TEST_FLAG(NETWORKED_OBJECT_TYPES, type) || !tag_index_is_group(change->definition_index, group_tag))
		return FALSE;
	/* (a flag's or ball's team names the game type's flag or ball: an index
	of its arrays; weapon_is_flag's bit) */
	if (type == _object_type_weapon && ((weapon_definition_get(change->definition_index)->weapon.flags >> 3) & 1))
		team_count = game_engine_get_variant()->game_engine_index == game_engine_ctf ? CTF_FLAG_TEAMS : ODDBALL_TEAMS;
	if (change->owner_team_index != NONE && (change->owner_team_index < 0 || change->owner_team_index >= team_count))
		return FALSE;
	/* (a flag or ball of no team would index the game type's arrays at -1) */
	if (type == _object_type_weapon && ((weapon_definition_get(change->definition_index)->weapon.flags >> 3) & 1) &&
		change->owner_team_index == NONE)
	{
		return FALSE;
	}
	/* (its colors numbers) */
	{
		short color_index;

		for (color_index = 0; color_index < NUMBER_OF_OBJECT_CHANGE_COLORS; color_index++)
		{
			if (!distributed_real_valid(change->change_colors[color_index].red) ||
				!distributed_real_valid(change->change_colors[color_index].green) ||
				!distributed_real_valid(change->change_colors[color_index].blue))
			{
				return FALSE;
			}
		}
	}
	return distributed_transform_valid(&change->position, &change->forward, &change->up,
		&change->translational_velocity, &change->angular_velocity, forward, up);
}

/* the host's word on an object made or found here: how it looks, whether
carried, whether dead */
static void distributed_client_apply_change(
	long object_index,
	struct distributed_object_change const *change)
{
	struct object_datum *object = object_get(object_index);

	csmemcpy(object->object.base_change_colors, change->change_colors, sizeof(object->object.base_change_colors));
	if (change->forced_shader_permutation_index)
		object->object.forced_shader_permutation_index = change->forced_shader_permutation_index;
	/* (and the colors drawn, which object_new chose from the tag: an AI unit's
	are its variant's, set after it was made, actors.c) */
	{
		short color_index;

		for (color_index = 0; color_index < NUMBER_OF_OBJECT_CHANGE_COLORS; color_index++)
		{
			object->object.outgoing_change_colors[color_index].red = PIN(change->change_colors[color_index].red, 0.0f, 1.0f);
			object->object.outgoing_change_colors[color_index].green =
				PIN(change->change_colors[color_index].green, 0.0f, 1.0f);
			object->object.outgoing_change_colors[color_index].blue = PIN(change->change_colors[color_index].blue, 0.0f, 1.0f);
		}
	}
	/* (a permutation the model has, or none: the model's renderer takes it
as it is) */
	{
		long model_index = object_definition_get(object->definition_index)->object.model.index;
		struct model *model = model_index != NONE ? model_definition_get(model_index) : NULL;
		short region_index;

		for (region_index = 0; model && region_index < model->regions.count &&
			region_index < MAXIMUM_REGIONS_PER_OBJECT; region_index++)
		{
			struct model_region *region = TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region);
			byte permutation_index = change->region_permutations[region_index];

			if (permutation_index == (byte)NONE || permutation_index < region->permutations.count)
				object->object.region_permutations[region_index] = permutation_index;
		}
	}
	object_set_garbage(object_index, FALSE);
	/* (in a unit's inventory: the next inventories put it there) */
	if (TEST_FLAG(change->flags, _distributed_object_carried_bit) &&
		TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
		object->object.parent_object_index == NONE)
	{
		object_disconnect_from_map(object_index);
		object_set_visibility(object_index, FALSE);
	}
	/* (a body: dead on the host before this machine had it, killed here
	with nothing to show or count of it) */
	if (TEST_FLAG(change->flags, _distributed_object_dead_bit) && TEST_FLAG(_object_mask_unit, object->object.type) &&
		!TEST_FLAG(object->object.damage_flags, _object_dead_bit))
	{
		unit_kill_silent(object_index);
		unit_kill_no_statistics(object_index);
		/* (at once: a body where no player is may not be updated for long) */
		object_damage_update(object_index);
	}
}

static boolean distributed_client_reuses_object(long existing_index,
	struct object_datum const *existing, struct distributed_object_change const *change,
	boolean checkpoint_pending)
{
	return existing_index == change->object_index && existing &&
		existing->definition_index == change->definition_index &&
		!(checkpoint_pending && network_coop_active() && TEST_FLAG(_object_mask_unit, existing->object.type) &&
			TEST_FLAG(existing->object.damage_flags, _object_dead_bit) &&
			!TEST_FLAG(change->flags, _distributed_object_dead_bit));
}

static void distributed_client_create(
	struct distributed_object_change const *change)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(change->object_index);
	struct datum_header const *header;
	struct object_placement_data placement;
	real_vector3d forward, up;
	long object_index;
	boolean checkpoint_pending;

	if (absolute_index < 0 || absolute_index >= MAXIMUM_TRACKED_OBJECTS ||
		absolute_index >= object_header_data->maximum_count ||
		!distributed_client_change_valid(change, &forward, &up))
	{
		objects_statistics.create_failures++;
		return;
	}
	checkpoint_pending = objects_client_checkpoint_pending[absolute_index];
	objects_client_checkpoint_pending[absolute_index] = FALSE;
	header = (struct datum_header const *)((byte const *)object_header_data->data +
		absolute_index * object_header_data->size);
	if (header->identifier)
	{
		long existing_index = ((long)header->identifier << 16) | absolute_index;
		struct object_datum *existing = object_try_and_get(existing_index);

		/* placed here when the map loaded, as on the host, or told of
		before: the same object, where the host has it (unless carried, or
		this machine's own) */
		if (distributed_client_reuses_object(existing_index, existing, change, checkpoint_pending))
		{
			objects_client_has[absolute_index] = existing_index;
			if (!TEST_FLAG(change->flags, _distributed_object_carried_bit) &&
				existing->object.parent_object_index == NONE &&
				TEST_FLAG(existing->object.flags, _object_connected_to_map_bit) &&
				!distributed_client_own_object(existing_index))
			{
				distributed_object_move(existing_index, &change->position, &forward, &up,
					&change->translational_velocity, &change->angular_velocity, _network_vehicle_write_change);
			}
			distributed_client_apply_change(existing_index, change);
			return;
		}
		/* something else in its place: gone */
		distributed_client_delete(existing_index);
		objects_client_has[absolute_index] = NONE;
		if (header->identifier)
		{
			distributed_client_create_failed();
			return;
		}
	}
	object_placement_data_new(&placement, change->definition_index, NONE);
	placement.owner_player_index = distributed_player_from_byte(change->owner_player_index);
	placement.owner_team_index = change->owner_team_index;
	placement.variant_number = change->variant_number;
	placement.position = change->position;
	placement.forward = forward;
	placement.up = up;
	placement.translational_velocity = change->translational_velocity;
	placement.angular_velocity = change->angular_velocity;
	csmemcpy(placement.change_colors, change->change_colors, sizeof(placement.change_colors));
	objects_client_creating_index = change->object_index;
	objects_client_creating = TRUE;
	object_index = object_new(&placement);
	objects_client_creating = FALSE;
	objects_client_creating_index = NONE;
	objects_statistics.creates++;
	if (object_index != change->object_index)
	{
		if (object_index != NONE)
		{
			objects_client_deleting = TRUE;
			object_delete_immediately(object_index);
			objects_client_deleting = FALSE;
		}
		distributed_client_create_failed();
		return;
	}
	objects_client_has[absolute_index] = object_index;
	/* (a unit's grenades are the host's, which its inventories bring: none
	until they come, not the tag's that unit_new gave it, which the host's
	game engine replaces with the game's at a player's spawn: a player
	spawned with the button held threw one here the host's copy had not) */
	if (TEST_FLAG(_object_mask_unit, object_get(object_index)->object.type))
		csmemset(unit_get(object_index)->unit.grenade_counts, 0, sizeof(unit_get(object_index)->unit.grenade_counts));
	distributed_client_apply_change(object_index, change);
}

void network_objects_handle_changes(
	void const *entries,
	short count)
{
	struct distributed_object_change const *changes = (struct distributed_object_change const *)entries;
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_object_change const *change = &changes[index];
		long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(change->object_index);

		if (!distributed_object_index_valid(change->object_index) ||
			absolute_index < 0 || absolute_index >= MAXIMUM_TRACKED_OBJECTS)
		{
			continue;
		}
		if (change->change == _object_change_create)
		{
			distributed_client_create(change);
		}
		else if (change->change == _object_change_delete &&
			objects_client_has[absolute_index] == change->object_index)
		{
			objects_statistics.deletes++;
			distributed_client_delete(change->object_index);
			objects_client_has[absolute_index] = NONE;
		}
	}
}

void network_objects_handle_synchronized(
	void)
{
	/* (a create failed since this machine last asked: not all of them, it
	asks again) */
	if (objects_client_failed_time != NONE)
	{
		if (objects_client_ready_time <= objects_client_failed_time)
			return;
		objects_client_failed_time = NONE;
		objects_client_retry_ticks = CLIENT_RETRY_TICKS;
	}
	objects_client_synchronized = TRUE;
}

/* a vehicle this machine's own player drives where the host has it, the
host saying at which of this machine's ticks (its prediction come back):
moved by how far that is from where this machine had it at that tick, if
further than a tolerance, keeping what it has done since (no rubber band a
round trip long); FALSE without that tick */
static boolean distributed_client_correct_own_vehicle(
	long vehicle_index,
	long player_index,
	struct distributed_object_state const *state)
{
	struct player_datum *player = player_try_and_get(player_index);
	short local_player_index = player ? player->local_player_index : NONE;
	long now = game_time_get();
	long time = now - (long)(word)((word)now - (word)state->time);
	struct object_datum *object = object_get(vehicle_index);
	real_vector3d error;
	real_vector3d forward, up;
	real_point3d before, position;
	short index;

	if (!TEST_FLAG(state->flags, _distributed_object_predicted_bit) ||
		local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS ||
		objects_client_own_vehicles[local_player_index][time & (OWN_VEHICLE_POSITION_TICKS - 1)].time != time ||
		objects_client_own_vehicles[local_player_index][time & (OWN_VEHICLE_POSITION_TICKS - 1)].vehicle_index !=
			vehicle_index)
	{
		return FALSE;
	}
	/* (driving: the round trip as the host has its vehicle, the latency
	meter's) */
	distributed_note_own_round_trip(time);
	{
		struct distributed_own_vehicle const *own =
			&objects_client_own_vehicles[local_player_index][time & (OWN_VEHICLE_POSITION_TICKS - 1)];

		error.i = state->position.x - own->position.x;
		error.j = state->position.y - own->position.y;
		error.k = state->position.z - own->position.z;
	}
	if (error.i * error.i + error.j * error.j + error.k * error.k <= LOCAL_VEHICLE_TOLERANCE * LOCAL_VEHICLE_TOLERANCE)
		return TRUE;
	/* (turned and going as it drives it) */
	before = object->object.position;
	position.x = before.x + error.i;
	position.y = before.y + error.j;
	position.z = before.z + error.k;
	forward = object->object.forward;
	up = object->object.up;
	network_objects_correct(vehicle_index, &position, &forward, &up, NULL, NULL, _network_vehicle_write_own);
	distributed_count_correction();
	/* (the ticks noted since moved as it was, not corrected again) */
	error.i = object->object.position.x - before.x;
	error.j = object->object.position.y - before.y;
	error.k = object->object.position.z - before.z;
	for (index = 0; index < OWN_VEHICLE_POSITION_TICKS; index++)
	{
		if (objects_client_own_vehicles[local_player_index][index].vehicle_index == vehicle_index)
		{
			objects_client_own_vehicles[local_player_index][index].position.x += error.i;
			objects_client_own_vehicles[local_player_index][index].position.y += error.j;
			objects_client_own_vehicles[local_player_index][index].position.z += error.k;
		}
	}
	return TRUE;
}

/* (a client) a biped, no player's (a player's the units' states kill), the
host says is dead, alive here: its killing blow, unreliable, kills it; lost,
nothing else would. Killed with nothing to show or count of it once the host
has said so for a while. */
static void distributed_client_dead_biped(
	long object_index)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);
	struct unit_datum *unit = (struct unit_datum *)object_try_and_get_and_verify_type(object_index,
		_object_mask_biped);

	if (!unit || TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || unit->unit.player_index != NONE)
		return;
	if (objects_client_dead[absolute_index].object_index != object_index)
	{
		objects_client_dead[absolute_index].object_index = object_index;
		objects_client_dead[absolute_index].time = game_time_get();
	}
	else if (game_time_get() - objects_client_dead[absolute_index].time >= DEAD_BIPED_FALLBACK_TICKS)
	{
		unit_kill_silent(object_index);
		unit_kill_no_statistics(object_index);
		object_damage_update(object_index);
	}
}

/* client: a vehicle the host says is dead is destroyed here too, with its
explosion. No killing blow comes for vehicles, only for bipeds. */
static void distributed_client_dead_vehicle(
	long object_index)
{
	struct unit_datum *vehicle = (struct unit_datum *)object_try_and_get_and_verify_type(object_index,
		_object_mask_vehicle);

	if (!vehicle || TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit))
		return;
	unit_kill_no_statistics(object_index);
	object_damage_update(object_index);
}

/* the facing a fraction t of the way from one sample to the next */
static void unsteered_blend_facing(
	real_vector3d const *forward0,
	real_vector3d const *up0,
	real_vector3d const *forward1,
	real_vector3d const *up1,
	real t,
	real_vector3d *forward,
	real_vector3d *up)
{
	real_matrix4x3 from;
	real_matrix4x3 to;
	real_matrix4x3 blended;
	real_quaternion rotation0;
	real_quaternion rotation1;
	real_quaternion rotation;

	matrix4x3_rotation_from_vectors(&from, forward0, up0);
	matrix4x3_rotation_from_vectors(&to, forward1, up1);
	matrix4x3_rotation_to_quaternion(&from, &rotation0);
	matrix4x3_rotation_to_quaternion(&to, &rotation1);
	quaternions_interpolate_and_normalize(&rotation0, &rotation1, t, &rotation);
	matrix4x3_rotation_from_quaternion(&blended, &rotation);
	*forward = blended.forward;
	*up = blended.up;
}

/* Client: the next host sample of an unsteered vehicle, of the host's tick
host_time. The path's velocity is the host's last two samples' difference
over the host's ticks between them (not when they arrived, which a late or
bunched message would make uneven). The picture moves from where the vehicle
is to the sample at that speed, so it keeps its pace whether it was behind or
gone on past the last sample, and goes on along the path if the next sample
is late (distributed_client_carry_unsteered_vehicles). One the host has at
rest stops at the sample. The host's physics velocity is not used: a lifepod
swings about the path, and a Pelican's spin flaps its fins. A jump further
than a step of the path is taken whole. */
static void distributed_client_note_unsteered_path(
	long object_index,
	real_point3d const *host_position,
	real_vector3d const *host_forward,
	real_vector3d const *host_up,
	long host_time,
	boolean host_at_rest)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index);
	struct object_datum *object;
	long now;
	long since;
	boolean same;
	real_vector3d step;
	real limit;
	real_vector3d velocity;
	real speed_squared;
	real interval;

	/* (rest uses this glide too: a late sample holds at the newest) */
	if (absolute_index < 0 || absolute_index >= MAXIMUM_TRACKED_OBJECTS ||
		!host_position || !host_forward || !host_up)
	{
		return;
	}
	object = object_get(object_index);
	now = game_time_get();
	same = objects_client_unsteered_path[absolute_index].object_index == object_index;
	/* (an older sample than the newest, or the same tick's again) */
	if (same && host_time - objects_client_unsteered_path[absolute_index].host_time < 1)
		return;
	since = same ? host_time - objects_client_unsteered_path[absolute_index].host_time : 0;
	if (since < 1)
		since = 1;
	if (since > UNSTEERED_PATH_MAXIMUM_TICKS)
		since = UNSTEERED_PATH_MAXIMUM_TICKS;
	step.i = host_position->x - object->object.position.x;
	step.j = host_position->y - object->object.position.y;
	step.k = host_position->z - object->object.position.z;
	limit = (real)since * UNSTEERED_PATH_STEP;
	/* (a teleport is not a step of the path: leave it where the host put it) */
	if (!same || !(step.i * step.i + step.j * step.j + step.k * step.k <= limit * limit))
	{
		objects_client_unsteered_path[absolute_index].object_index = object_index;
		objects_client_unsteered_path[absolute_index].time = now;
		objects_client_unsteered_path[absolute_index].host_time = host_time;
		objects_client_unsteered_path[absolute_index].interval = 1.0f;
		objects_client_unsteered_path[absolute_index].velocity = *global_zero_vector3d;
		objects_client_unsteered_path[absolute_index].previous_position = *host_position;
		objects_client_unsteered_path[absolute_index].latest_position = *host_position;
		objects_client_unsteered_path[absolute_index].previous_forward = *host_forward;
		objects_client_unsteered_path[absolute_index].previous_up = *host_up;
		objects_client_unsteered_path[absolute_index].latest_forward = *host_forward;
		objects_client_unsteered_path[absolute_index].latest_up = *host_up;
		object->object.translational_velocity = *global_zero_vector3d;
		object->object.angular_velocity = *global_zero_vector3d;
		{
			real_point3d before = object->object.position;
			real_vector3d before_forward = object->object.forward;
			real_vector3d before_up = object->object.up;

			object_set_position(object_index, host_position, host_forward, host_up);
			network_objects_vehicle_written(object_index, &before, &before_forward, &before_up, _network_vehicle_write_state);
		}
		return;
	}
	/* the path's velocity, sample to sample on the host's clock */
	velocity.i = (host_position->x - objects_client_unsteered_path[absolute_index].latest_position.x) / (real)since;
	velocity.j = (host_position->y - objects_client_unsteered_path[absolute_index].latest_position.y) / (real)since;
	velocity.k = (host_position->z - objects_client_unsteered_path[absolute_index].latest_position.z) / (real)since;
	if (host_at_rest)
		velocity = *global_zero_vector3d;
	speed_squared = velocity.i * velocity.i + velocity.j * velocity.j + velocity.k * velocity.k;
	/* the glide to the sample at that speed: no faster than twice it, nor
	slower than half (behind or ahead of the path by more, it closes the
	distance); one at rest over the samples' gap */
	interval = (real)since;
	if (speed_squared > 1.0e-6f)
	{
		interval = (real)sqrt((double)((step.i * step.i + step.j * step.j + step.k * step.k) / speed_squared));
		if (!(interval >= 0.5f * (real)since))
			interval = 0.5f * (real)since;
		if (interval > 2.0f * (real)since)
			interval = 2.0f * (real)since;
	}
	objects_client_unsteered_path[absolute_index].previous_position = object->object.position;
	objects_client_unsteered_path[absolute_index].previous_forward = object->object.forward;
	objects_client_unsteered_path[absolute_index].previous_up = object->object.up;
	objects_client_unsteered_path[absolute_index].latest_position = *host_position;
	objects_client_unsteered_path[absolute_index].latest_forward = *host_forward;
	objects_client_unsteered_path[absolute_index].latest_up = *host_up;
	objects_client_unsteered_path[absolute_index].interval = interval;
	objects_client_unsteered_path[absolute_index].velocity = velocity;
	objects_client_unsteered_path[absolute_index].time = now;
	objects_client_unsteered_path[absolute_index].host_time = host_time;
	objects_client_unsteered_path[absolute_index].object_index = object_index;
}

void network_objects_handle_states(
	void const *entries,
	short count,
	long host_time)
{
	struct distributed_object_state const *states = (struct distributed_object_state const *)entries;
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_object_state const *state = &states[index];
		struct object_datum *object;
		real tolerance = REMOTE_OBJECT_TOLERANCE;
		real angle_tolerance = REMOTE_OBJECT_ANGLE_TOLERANCE;
		real blend_distance = REMOTE_BLEND_DISTANCE;
		real_vector3d forward, up, velocity, angular_velocity;
		real dx, dy, dz;
		boolean own_vehicle = FALSE;
		boolean at_rest;

		if (!distributed_object_index_valid(state->object_index) || !network_objects_client_has(state->object_index))
			continue;
		object = object_get(state->object_index);
		/* (the unit's light as the host has it: only presses toggle it here,
		so one missed, or made before this machine joined, left it wrong for
		good. Not for what this machine's player drives: their press lands
		here first) */
		if (TEST_FLAG(state->flags, _distributed_object_light_sent_bit) &&
			TEST_FLAG(_object_mask_unit, object->object.type) &&
			!distributed_unit_driven_here(state->object_index))
		{
			SET_FLAG(unit_get(state->object_index)->unit.flags, _unit_integrated_light_on_bit,
				TEST_FLAG(state->flags, _distributed_object_light_on_bit));
		}
		if (TEST_FLAG(state->flags, _distributed_object_dead_bit))
		{
			distributed_client_dead_biped(state->object_index);
			distributed_client_dead_vehicle(state->object_index);
			/* (a body its tag destroys at once is gone) */
			if (!object_try_and_get_and_verify_type(state->object_index, _object_mask_all))
				continue;
		}
		else
			network_objects_expect_live_unit(state->object_index);
		/* (an item made carried whose unit never had it here, placed on the
		host: in the world, seen) */
		if (TEST_FLAG(_object_mask_item, object->object.type) && object->object.parent_object_index == NONE &&
			!TEST_FLAG(object->object.flags, _object_connected_to_map_bit) &&
			!TEST_FLAG(item_get(state->object_index)->item.flags, _item_attached_to_unit_bit))
		{
			object_reconnect_to_map(state->object_index, NULL);
			object_set_visibility(state->object_index, TRUE);
		}
		if (object->object.parent_object_index != NONE ||
			!TEST_FLAG(object->object.flags, _object_connected_to_map_bit))
		{
			continue;
		}
		distributed_object_state_unpack(state, &forward, &up, &velocity, &angular_velocity);
		if (!distributed_transform_valid(&state->position, &forward, &up, NULL, NULL, &forward, &up))
			continue;
		/* (come to rest: all of the way, drawn gliding, not half of it and
		left there) */
		if (TEST_FLAG(state->flags, _distributed_object_at_rest_bit))
			blend_distance = 0.0f;
		/* (a vehicle this machine's own player drives is its own) */
		if (object->object.type == _object_type_vehicle)
		{
			struct unit_datum *vehicle = unit_get(state->object_index);

			if (blend_distance > 0.0f)
				blend_distance = REMOTE_VEHICLE_BLEND_DISTANCE;
			/* (one no player drives follows every move the host sends, however small) */
			if (distributed_vehicle_unsteered(state->object_index))
			{
				tolerance = 0.0f;
				angle_tolerance = 2.0f;
			}
			if (vehicle->unit.driver_object_index != NONE &&
				distributed_player_is_local(unit_get(vehicle->unit.driver_object_index)->unit.player_index))
			{
				if (distributed_client_correct_own_vehicle(state->object_index,
					unit_get(vehicle->unit.driver_object_index)->unit.player_index, state))
				{
					continue;
				}
				tolerance = LOCAL_VEHICLE_TOLERANCE;
				/* (turned as it drives it) */
				angle_tolerance = -1.0f;
				blend_distance = 0.0f;
				own_vehicle = TRUE;
			}
		}
		/* (at rest as the host has it, however close: a copy left at rest
		while the host's falls hangs there; but not one this machine drives,
		which the host's word, a round trip old, would stop as it sets off) */
		at_rest = TEST_FLAG(state->flags, _distributed_object_at_rest_bit) || distributed_vehicle_unsteered(state->object_index);
		if (!own_vehicle)
			SET_FLAG(object->object.flags, _object_at_rest_bit, at_rest);
		distributed_body_set_at_rest(state->object_index, at_rest);
		/* (no snap onto the host: the carry moves it across the gap) */
		if (distributed_vehicle_unsteered(state->object_index))
		{
			distributed_client_note_unsteered_path(state->object_index, &state->position, &forward, &up,
				host_time, TEST_FLAG(state->flags, _distributed_object_at_rest_bit));
			continue;
		}
		dx = state->position.x - object->object.position.x;
		dy = state->position.y - object->object.position.y;
		dz = state->position.z - object->object.position.z;
		/* (close enough where it is, and turned as it is: one at rest too) */
		if (dx * dx + dy * dy + dz * dz <= tolerance * tolerance &&
			forward.i * object->object.forward.i + forward.j * object->object.forward.j +
				forward.k * object->object.forward.k >= angle_tolerance &&
			up.i * object->object.up.i + up.j * object->object.up.j + up.k * object->object.up.k >= angle_tolerance)
		{
			/* (a copy kept where it is still takes the host's velocities: left to its own physics they decay
			between corrections, and a rider's frame, the vehicle's motion, goes wrong with them) */
			if (vehicle_riding_enabled() && object->object.type == _object_type_vehicle && !own_vehicle &&
				!distributed_vehicle_unsteered(state->object_index))
			{
				object->object.translational_velocity = velocity;
				object->object.angular_velocity = angular_velocity;
			}
			continue;
		}
		if (network_objects_reconcile(state->object_index, &state->position, &forward, &up, &velocity,
			&angular_velocity, blend_distance, _network_vehicle_write_state))
		{
			distributed_count_correction();
		}
		/* (one it drives, put where the host has it, is as the host has it) */
		if (own_vehicle)
			SET_FLAG(object->object.flags, _object_at_rest_bit, at_rest);
	}
}

/* what this machine's own player's unit carries now noted (it is that
player's: local_player_index); compared with what it carried at the last,
when it threw a grenade and fired or reloaded a weapon, noted too */
static void distributed_client_note_own_inventory(
	struct distributed_own_inventory *own,
	long unit_index,
	boolean compare)
{
	struct unit_datum *unit = unit_get(unit_index);
	long now = game_time_get();
	short grenade_type;
	short weapon_slot;

	if (own->unit_index != unit_index)
	{
		own->unit_index = unit_index;
		compare = FALSE;
		for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
			own->grenade_times[grenade_type] = NONE;
		for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
		{
			own->weapon_indices[weapon_slot] = NONE;
			own->weapon_times[weapon_slot] = NONE;
		}
	}
	for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
	{
		if (compare && unit->unit.grenade_counts[grenade_type] < own->grenade_counts[grenade_type])
			own->grenade_times[grenade_type] = now;
		own->grenade_counts[grenade_type] = unit->unit.grenade_counts[grenade_type];
	}
	for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
	{
		long weapon_index = unit->unit.weapon_object_indices[weapon_slot];
		struct weapon_datum *weapon = weapon_index != NONE ? weapon_try_and_get(weapon_index) : NULL;
		short magazine;

		if (!weapon)
		{
			own->weapon_indices[weapon_slot] = NONE;
			continue;
		}
		/* (another weapon: none of it done here yet) */
		if (own->weapon_indices[weapon_slot] != weapon_index)
		{
			own->weapon_indices[weapon_slot] = weapon_index;
			own->weapon_times[weapon_slot] = NONE;
		}
		else if (compare)
		{
			boolean used = weapon->weapon.game_time_last_fired != own->fired_times[weapon_slot];

			for (magazine = 0; magazine < 2; magazine++)
			{
				used |= weapon->weapon.magazines[magazine].state == _magazine_reloading ||
					weapon->weapon.magazines[magazine].rounds_total != own->rounds[weapon_slot][magazine][0] ||
					weapon->weapon.magazines[magazine].rounds_loaded != own->rounds[weapon_slot][magazine][1];
			}
			if (used)
				own->weapon_times[weapon_slot] = now;
		}
		own->fired_times[weapon_slot] = weapon->weapon.game_time_last_fired;
		for (magazine = 0; magazine < 2; magazine++)
		{
			own->rounds[weapon_slot][magazine][0] = weapon->weapon.magazines[magazine].rounds_total;
			own->rounds[weapon_slot][magazine][1] = weapon->weapon.magazines[magazine].rounds_loaded;
		}
	}
}

/* whether the host has had a round trip (and the time its inventories take
to go) since this machine last did that with its own player's unit (at
time, NONE for never): what the host says of it then has what it did */
static boolean distributed_client_own_settled(
	long time)
{
	real round_trip = distributed_own_round_trip_ticks();

	if (time == NONE)
		return TRUE;
	/* (so written that one not a number is none) */
	if (!(round_trip > 0.0f))
		round_trip = (real)DEFAULT_OWN_ROUND_TRIP_TICKS;
	if (round_trip > (real)MAXIMUM_OWN_ROUND_TRIP_TICKS)
		round_trip = (real)MAXIMUM_OWN_ROUND_TRIP_TICKS;
	return game_time_get() - time > (long)ceil(round_trip) + INVENTORY_INTERVAL_TICKS + OWN_INVENTORY_MARGIN_TICKS;
}

/* whether the weapon the host has in a slot is here (none always is) */
static boolean distributed_client_weapon_here(
	long weapon_index)
{
	return weapon_index == NONE ||
		(network_objects_client_has(weapon_index) && object_try_and_get_and_verify_type(weapon_index, _object_mask_weapon));
}

/* whether the unit an inventory names and every weapon it names are here */
static boolean distributed_client_inventory_whole(
	struct distributed_inventory const *inventory)
{
	short weapon_slot;

	if (!network_objects_client_has(inventory->unit_index) ||
		!object_try_and_get_and_verify_type(inventory->unit_index, _object_mask_unit))
	{
		return FALSE;
	}
	for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
	{
		if (!distributed_client_weapon_here(inventory->weapon_indices[weapon_slot]))
			return FALSE;
	}
	return TRUE;
}

/* the inventory kept to apply once all it names is here, the newest of its
unit's (in its entry, else a new one, else the oldest's) */
static void distributed_client_retry_inventory(
	struct distributed_inventory const *inventory)
{
	struct distributed_retried_inventory *retried = NULL;
	short index;

	for (index = 0; !retried && index < objects_client_retried_inventory_count; index++)
	{
		if (objects_client_retried_inventories[index].inventory.unit_index == inventory->unit_index)
			retried = &objects_client_retried_inventories[index];
	}
	if (!retried && objects_client_retried_inventory_count < MAXIMUM_CLIENT_RETRIED_INVENTORIES)
		retried = &objects_client_retried_inventories[objects_client_retried_inventory_count++];
	if (!retried)
	{
		short oldest = 0;

		for (index = 1; index < objects_client_retried_inventory_count; index++)
		{
			if (objects_client_retried_inventories[index].time < objects_client_retried_inventories[oldest].time)
				oldest = index;
		}
		retried = &objects_client_retried_inventories[oldest];
	}
	retried->inventory = *inventory;
	retried->time = game_time_get();
}

/* the unit's kept inventory, if it has one, kept no more */
static void distributed_client_forget_retried_inventory(
	long unit_index)
{
	short index;

	for (index = 0; index < objects_client_retried_inventory_count; index++)
	{
		if (objects_client_retried_inventories[index].inventory.unit_index == unit_index)
		{
			objects_client_retried_inventories[index] =
				objects_client_retried_inventories[--objects_client_retried_inventory_count];
			return;
		}
	}
}

/* the unit carries what the host's does */
static void distributed_client_apply_inventory(
	struct distributed_inventory const *inventory)
{
	struct unit_datum *unit;
	struct distributed_own_inventory *own = NULL;
	boolean local;
	short weapon_slot;

	/* (what is here of it now, and the rest once it is: the newest of the
	unit's is the one kept) */
	if (distributed_client_inventory_whole(inventory))
		distributed_client_forget_retried_inventory(inventory->unit_index);
	else
		distributed_client_retry_inventory(inventory);
	if (!network_objects_client_has(inventory->unit_index) ||
		!object_try_and_get_and_verify_type(inventory->unit_index, _object_mask_unit))
	{
		return;
	}
	unit = unit_get(inventory->unit_index);
	if (TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		return;
	local = distributed_player_is_local(unit->unit.player_index);
	if (local)
	{
		short local_player_index = player_get(unit->unit.player_index)->local_player_index;

		if (local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS &&
			objects_client_own_inventories[local_player_index].unit_index == inventory->unit_index)
		{
			own = &objects_client_own_inventories[local_player_index];
		}
	}
	/* picked up, swapped or dropped on the host: the same weapons here,
	slot for slot (the host's own objects, moved in and out as
	unit_add_weapon_to_inventory and unit_drop_current_weapon do, without
	their rules: the host has applied them) */
	for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
	{
		long wanted_index = inventory->weapon_indices[weapon_slot];
		long weapon_index = unit->unit.weapon_object_indices[weapon_slot];

		if (weapon_index == wanted_index)
			continue;
		/* (not here yet: as it is, until it is) */
		if (!distributed_client_weapon_here(wanted_index))
			continue;
		if (weapon_index != NONE)
			unit_network_drop_weapon(inventory->unit_index, weapon_slot);
		if (wanted_index != NONE)
		{
			distributed_client_release_item(wanted_index, FALSE);
			unit_network_add_weapon(inventory->unit_index, wanted_index, weapon_slot);
		}
	}
	/* the weapon in hand: a client's own player chooses its own, unless its
	choice is gone */
	if (inventory->current_weapon_index >= 0 && inventory->current_weapon_index < MAXIMUM_WEAPONS_PER_UNIT &&
		unit->unit.current_weapon_index != inventory->current_weapon_index &&
		(!local || unit->unit.current_weapon_index == NONE ||
			unit->unit.weapon_object_indices[unit->unit.current_weapon_index] == NONE))
	{
		unit->unit.desired_weapon_index = inventory->current_weapon_index;
	}
	/* the ammunition (as much as the weapon's magazines hold): a client's
	own player spends its own as it fires and reloads (the host's count
	trails it), so it is the host's only once the host has had a round trip
	to see what it did with the weapon; then whatever the host's is */
	for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
	{
		long weapon_index = unit->unit.weapon_object_indices[weapon_slot];
		struct weapon_datum *weapon = weapon_index != NONE ? weapon_try_and_get(weapon_index) : NULL;
		struct weapon_definition *definition;
		short magazine;

		if (!weapon || weapon_index != inventory->weapon_indices[weapon_slot])
			continue;
		if (own && own->weapon_indices[weapon_slot] == weapon_index &&
			!distributed_client_own_settled(own->weapon_times[weapon_slot]))
		{
			continue;
		}
		definition = weapon_definition_get(weapon->definition_index);
		for (magazine = 0; magazine < 2 && magazine < definition->weapon.magazines.count; magazine++)
		{
			struct weapon_magazine *state = &weapon->weapon.magazines[magazine];
			struct weapon_magazine_definition *magazine_definition = TAG_BLOCK_GET_ELEMENT(
				&definition->weapon.magazines, magazine, struct weapon_magazine_definition);
			short total_maximum = MAX(magazine_definition->rounds_total_maximum, magazine_definition->rounds_total_initial);
			short total = inventory->rounds_total[weapon_slot][magazine];
			short loaded = inventory->rounds_loaded[weapon_slot][magazine];

			state->rounds_total = total < 0 ? 0 : MIN(total, total_maximum);
			state->rounds_loaded = loaded < 0 ? 0 : MIN(loaded, magazine_definition->rounds_loaded_maximum);
		}
		weapon->weapon.age = (real)inventory->age[weapon_slot] / AGE_SCALE;
	}
	/* the grenades (no more than a unit carries): a client's own player
	throws its own, so the host's count is taken when it is lower, and when
	higher (a pickup) only once the host has had a round trip to see the last
	it threw */
	{
		short grenade_type;

		for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
		{
			char count = inventory->grenade_counts[grenade_type] < 0 ? 0 :
				(char)MIN(inventory->grenade_counts[grenade_type], MAXIMUM_INVENTORY_GRENADES);

			if (!own || count < unit->unit.grenade_counts[grenade_type] ||
				distributed_client_own_settled(own->grenade_times[grenade_type]))
			{
				unit->unit.grenade_counts[grenade_type] = count;
			}
		}
		/* an AI unit's grenade type: the host's actor chose it as it gave the
		unit its grenades (unit_add_grenade_type_to_inventory), and a client
		has no actors. Without one, the throws the host's control starts
		never begin here (unit_throw_grenade_begin), and the grenades, whose
		blasts the host's damage brings, are never seen. The type it carries
		is it; not changed in the middle of a throw */
		if (!own && unit->unit.player_index == NONE &&
			unit->unit.grenade_throw_state == _unit_grenade_throw_idle &&
			(unit->unit.current_grenade_index == NONE ||
				unit->unit.grenade_counts[unit->unit.current_grenade_index] == 0))
		{
			for (grenade_type = 0; grenade_type < NUMBER_OF_UNIT_GRENADE_TYPES; grenade_type++)
			{
				if (unit->unit.grenade_counts[grenade_type] > 0)
				{
					unit->unit.current_grenade_index = grenade_type;
					unit->unit.desired_grenade_index = grenade_type;
					break;
				}
			}
		}
	}
	/* (what the host changed here is not this machine's doing) */
	if (own)
		distributed_client_note_own_inventory(own, inventory->unit_index, FALSE);
}

/* a client's own player picked up a weapon (network_player_show_pickup) */
void network_objects_client_picked_up_weapon(
	short local_player_index,
	long unit_index,
	long definition_index)
{
	struct distributed_picked_up_weapon *picked_up;

	if (local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return;
	picked_up = &objects_client_picked_up_weapons[local_player_index];
	picked_up->unit_index = unit_index;
	picked_up->definition_index = definition_index;
	picked_up->time = game_time_get();
}

/* the weapons its own players picked up readied once they have them, as
unit_add_weapon_to_inventory readies a pickup where it decides it (a second
weapon too, while firing): the client chooses its players' weapons in hand
(distributed_client_apply_inventory), and the host follows */
static void distributed_client_ready_picked_up_weapons(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		struct distributed_picked_up_weapon *picked_up = &objects_client_picked_up_weapons[local_player_index];
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_index != NONE ? player_try_and_get(player_index) : NULL;
		long unit_index = distributed_living_unit(player);
		struct unit_datum *unit;
		short weapon_slot;

		if (picked_up->definition_index == NONE)
			continue;
		if (unit_index == NONE || unit_index != picked_up->unit_index ||
			game_time_get() - picked_up->time > PICKED_UP_WEAPON_TICKS)
		{
			picked_up->definition_index = NONE;
			continue;
		}
		unit = unit_get(unit_index);
		{
			short current_slot = unit->unit.current_weapon_index;
			short matched_slot = NONE;
			long current_weapon_index = unit_inventory_get_weapon(unit_index, current_slot);
			struct weapon_datum *current_weapon = current_weapon_index != NONE ?
				weapon_try_and_get(current_weapon_index) : NULL;

			/* already the one in hand (a swap into that slot): nothing to ready */
			if (current_weapon && current_weapon->definition_index == picked_up->definition_index)
			{
				picked_up->definition_index = NONE;
				continue;
			}
			for (weapon_slot = 0; weapon_slot < MAXIMUM_WEAPONS_PER_UNIT; weapon_slot++)
			{
				long weapon_index = unit->unit.weapon_object_indices[weapon_slot];
				struct weapon_datum *weapon = weapon_index != NONE ? weapon_try_and_get(weapon_index) : NULL;

				if (weapon && weapon_slot != current_slot &&
					weapon->definition_index == picked_up->definition_index)
				{
					matched_slot = weapon_slot;
				}
			}
			if (matched_slot == NONE)
				continue;
			weapon_slot = matched_slot;
		}
		/* port: a one-weapon loadout's second weapon is readied when it
		arrives, including while firing. Dropping the announcement on a
		held trigger left it holstered until a manual switch. */
		player_control_set_desired_weapon(unit_index, weapon_slot);
		unit->unit.desired_weapon_index = weapon_slot;
		picked_up->definition_index = NONE;
	}
}

void network_objects_handle_inventories(
	void const *entries,
	short count)
{
	struct distributed_inventory const *inventories = (struct distributed_inventory const *)entries;
	short index;

	for (index = 0; index < count; index++)
		distributed_client_apply_inventory(&inventories[index]);
}

void network_objects_set_seat(
	long unit_index,
	long vehicle_index,
	short seat_index)
{
	struct unit_datum *unit = (struct unit_datum *)object_try_and_get_and_verify_type(unit_index, _object_mask_unit);
	long occupant_index = NONE;

	if (!unit)
		return;
	/* (a seat the vehicle has) */
	if (vehicle_index != NONE)
	{
		struct unit_datum *vehicle;

		if (vehicle_index == unit_index || !distributed_object_index_valid(vehicle_index) ||
			!network_objects_client_has(vehicle_index))
		{
			return;
		}
		vehicle = (struct unit_datum *)object_try_and_get_and_verify_type(vehicle_index, _object_mask_vehicle);
		/* MCC biped seats use the host's existing seat state and all the
		same identity, capacity and attachment-cycle checks below. */
		if (!vehicle)
		{
			vehicle = mcc_seat_parent(vehicle_index);
			if (vehicle && (TEST_FLAG(vehicle->object.damage_flags, _object_dead_bit) ||
				!unit_has_animation_to_enter_seat(unit_index, vehicle_index, seat_index)))
				return;
		}
		if (!vehicle || seat_index < 0 ||
			seat_index >= unit_definition_get(vehicle->definition_index)->unit.seats.count)
		{
			return;
		}
		/* (and not one the unit carries, however far down: a ring of
		parents would turn every walk up them forever) */
		{
			long ancestor_index = vehicle->object.parent_object_index;
			long steps;

			for (steps = 0; ancestor_index != NONE; steps++)
			{
				struct object_header_datum const *ancestor;

				if (ancestor_index == unit_index || steps >= 16)
					return;
				ancestor = (struct object_header_datum const *)datum_try_and_get(object_header_data, ancestor_index);
				if (!ancestor || !ancestor->datum)
					return;
				ancestor_index = ancestor->datum->object.parent_object_index;
			}
		}
	}
	if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		unit_exit_seat_end(unit_index);
	if (vehicle_index == NONE || unit->object.parent_object_index != NONE)
		return;
	/* (whoever this machine still has in the seat: out) */
	unit_can_enter_seat(unit_index, vehicle_index, seat_index, &occupant_index);
	if (occupant_index != NONE && occupant_index != unit_index)
		unit_exit_seat_end(occupant_index);
	unit_enter_seat(unit_index, vehicle_index, seat_index);
}

/* the vehicles its own players drive, to the host, with this tick (which
the host tells back, with where it has them), and where they are noted */
static void distributed_client_send_vehicles(
	void)
{
	struct distributed_object_state_message message;
	short count = 0;
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_index != NONE ? player_try_and_get(player_index) : NULL;
		long vehicle_index = player ? distributed_driven_vehicle(player) : NONE;
		struct distributed_own_vehicle *own =
			&objects_client_own_vehicles[local_player_index][game_time_get() & (OWN_VEHICLE_POSITION_TICKS - 1)];

		if (vehicle_index != NONE && (!network_objects_client_has(vehicle_index) ||
			object_get(vehicle_index)->object.parent_object_index != NONE))
		{
			vehicle_index = NONE;
		}
		own->time = game_time_get();
		own->vehicle_index = vehicle_index;
		if (vehicle_index == NONE)
			continue;
		own->position = object_get(vehicle_index)->object.position;
		distributed_state_from_object(vehicle_index, &message.states[count]);
		message.states[count++].time = (short)(word)game_time_get();
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_vehicle_prediction, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_object_state)), _distributed_to_host);
	}
}

/* what its own players' units carry, noted: when they threw grenades,
fired and reloaded */
static void distributed_client_note_own_inventories(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_index != NONE ? player_try_and_get(player_index) : NULL;
		long unit_index = distributed_living_unit(player);

		if (unit_index == NONE)
			objects_client_own_inventories[local_player_index].unit_index = NONE;
		else
			distributed_client_note_own_inventory(&objects_client_own_inventories[local_player_index], unit_index, TRUE);
	}
}

/* the objects of the kinds the host has that it has not told of go (what
this machine's own simulation made): those it made since the last tick, or
all of them once the host has told it all (or when it cannot say which it
made) */
static void distributed_client_remove_own_objects(
	void)
{
	short index;

	if (objects_client_check_all)
	{
		struct object_iterator iterator;

		objects_client_check_all = FALSE;
		objects_client_new_object_count = 0;
		object_iterator_new(&iterator, NETWORKED_OBJECT_TYPES, 0);
		while (object_iterator_next(&iterator))
		{
			long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);

			if (!distributed_object_networked(iterator.index) || objects_client_has[absolute_index] == iterator.index)
				continue;
			objects_statistics.own_objects_removed++;
			distributed_client_delete(iterator.index);
		}
		return;
	}
	for (index = 0; index < objects_client_new_object_count; index++)
	{
		long object_index = objects_client_new_objects[index];

		if (!distributed_object_networked(object_index) ||
			objects_client_has[DATUM_INDEX_TO_ABSOLUTE_INDEX(object_index)] == object_index)
		{
			continue;
		}
		objects_statistics.own_objects_removed++;
		distributed_client_delete(object_index);
	}
	objects_client_new_object_count = 0;
}

/* Client: moves each unsteered vehicle from where it was to the host's newest
sample (distributed_client_note_unsteered_path). Past that, with no newer
sample, it goes on at the path's velocity for UNSTEERED_PATH_LATE_TICKS (a
late sample then finds it moving), then comes back to the newest sample over
as long and holds there (the host's has stopped). A Pelican its pilot flies,
and a recording's lifepod. Its velocity is its path's while it moves on, for
what reads it (the engine's pitch, the wheels' turn, a passenger who gets
out), and none once it comes back or holds; its spin is none (the host's
flaps a Pelican's fins). One already holding at the newest sample is not
placed again. */
static void distributed_client_carry_unsteered_vehicles(
	void)
{
	struct object_iterator iterator;
	struct object_datum *object;

	object_iterator_new(&iterator, _object_mask_vehicle, 0);
	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		long absolute_index;
		long since;
		real interval;
		real alpha;
		real late;
		real_point3d position;
		real_vector3d forward;
		real_vector3d up;
		real_vector3d velocity;

		if (object->object.parent_object_index != NONE || !distributed_vehicle_unsteered(iterator.index))
			continue;
		absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		if (absolute_index < 0 || absolute_index >= MAXIMUM_TRACKED_OBJECTS ||
			objects_client_unsteered_path[absolute_index].object_index != iterator.index)
		{
			continue;
		}
		since = game_time_get() - objects_client_unsteered_path[absolute_index].time;
		interval = objects_client_unsteered_path[absolute_index].interval;
		if (!(interval >= 0.5f))
			interval = 0.5f;
		if (since < 0)
			since = 0;
		alpha = (real)since / interval;
		if (alpha > 1.0f)
			alpha = 1.0f;
		/* past the newest sample: on along the path, then back to it */
		late = (real)since - interval;
		if (late > 2.0f * (real)UNSTEERED_PATH_LATE_TICKS)
			late = 0.0f;
		else if (late > (real)UNSTEERED_PATH_LATE_TICKS)
			late = 2.0f * (real)UNSTEERED_PATH_LATE_TICKS - late;
		if (!(late > 0.0f))
			late = 0.0f;
		velocity = objects_client_unsteered_path[absolute_index].velocity;
		if (alpha >= 1.0f && late == 0.0f &&
			object->object.position.x == objects_client_unsteered_path[absolute_index].latest_position.x &&
			object->object.position.y == objects_client_unsteered_path[absolute_index].latest_position.y &&
			object->object.position.z == objects_client_unsteered_path[absolute_index].latest_position.z)
		{
			object->object.translational_velocity = *global_zero_vector3d;
			object->object.angular_velocity = *global_zero_vector3d;
			continue;
		}
		position.x = objects_client_unsteered_path[absolute_index].previous_position.x +
			(objects_client_unsteered_path[absolute_index].latest_position.x -
				objects_client_unsteered_path[absolute_index].previous_position.x) * alpha;
		position.y = objects_client_unsteered_path[absolute_index].previous_position.y +
			(objects_client_unsteered_path[absolute_index].latest_position.y -
				objects_client_unsteered_path[absolute_index].previous_position.y) * alpha;
		position.z = objects_client_unsteered_path[absolute_index].previous_position.z +
			(objects_client_unsteered_path[absolute_index].latest_position.z -
				objects_client_unsteered_path[absolute_index].previous_position.z) * alpha;
		unsteered_blend_facing(
			&objects_client_unsteered_path[absolute_index].previous_forward,
			&objects_client_unsteered_path[absolute_index].previous_up,
			&objects_client_unsteered_path[absolute_index].latest_forward,
			&objects_client_unsteered_path[absolute_index].latest_up,
			alpha,
			&forward,
			&up);
		if (alpha >= 1.0f)
		{
			position.x += velocity.i * late;
			position.y += velocity.j * late;
			position.z += velocity.k * late;
		}
		if (!distributed_transform_valid(&position, &forward, &up, NULL, NULL, &forward, &up))
			continue;
		if (alpha < 1.0f)
		{
			object->object.translational_velocity.i =
				(objects_client_unsteered_path[absolute_index].latest_position.x -
					objects_client_unsteered_path[absolute_index].previous_position.x) / interval;
			object->object.translational_velocity.j =
				(objects_client_unsteered_path[absolute_index].latest_position.y -
					objects_client_unsteered_path[absolute_index].previous_position.y) / interval;
			object->object.translational_velocity.k =
				(objects_client_unsteered_path[absolute_index].latest_position.z -
					objects_client_unsteered_path[absolute_index].previous_position.z) / interval;
		}
		else if ((real)since - interval <= (real)UNSTEERED_PATH_LATE_TICKS)
			object->object.translational_velocity = velocity;
		else
			object->object.translational_velocity = *global_zero_vector3d;
		object->object.angular_velocity = *global_zero_vector3d;
		{
			real_point3d before = object->object.position;
			real_vector3d before_forward = object->object.forward;
			real_vector3d before_up = object->object.up;

			object_set_position(iterator.index, &position, &forward, &up);
			network_objects_vehicle_written(iterator.index, &before, &before_forward, &before_up, _network_vehicle_write_carry);
		}
	}
}

/* the kept inventories all of whose unit and weapons have come: applied once,
as if they had come now */
static void distributed_client_apply_retried_inventories(
	void)
{
	short index = 0;

	while (index < objects_client_retried_inventory_count)
	{
		struct distributed_retried_inventory *retried = &objects_client_retried_inventories[index];

		if (game_time_get() - retried->time >= EMPTY_INVENTORY_TICKS)
		{
			*retried = objects_client_retried_inventories[--objects_client_retried_inventory_count];
		}
		else if (distributed_client_inventory_whole(&retried->inventory))
		{
			struct distributed_inventory inventory = retried->inventory;

			*retried = objects_client_retried_inventories[--objects_client_retried_inventory_count];
			distributed_client_apply_inventory(&inventory);
		}
		else
		{
			index++;
		}
	}
}

void network_objects_client_tick(
	void)
{
	/* (loaded: the host's objects, please; flagged when one of them failed
	since it last asked) */
	if (!objects_client_synchronized &&
		(objects_client_ready_time == NONE || game_time_get() - objects_client_ready_time >= objects_client_ready_interval))
	{
		struct distributed_message_header message;

		/* (asked, less often the longer the host takes: a slow link does
		not queue up the asking) */
		if (objects_client_ready_time != NONE)
			objects_client_ready_interval = MIN(2 * objects_client_ready_interval, CLIENT_READY_MAXIMUM_INTERVAL_TICKS);
		objects_client_ready_time = game_time_get();
		distributed_send(&message, _distributed_message_client_ready, objects_client_ask_again ? 1 : 0,
			(word)sizeof(message), _distributed_to_host_reliably);
		objects_client_ask_again = FALSE;
	}
	distributed_client_apply_retried_inventories();
	if (objects_client_synchronized)
		distributed_client_remove_own_objects();
	/* (too many to look at one by one later: all of them) */
	else if (objects_client_new_object_count >= MAXIMUM_CLIENT_NEW_OBJECTS)
		objects_client_check_all = TRUE;
	/* Do not record predictions from retained old replicas while the
	ordered checkpoint inventory is still arriving. Its empty state must
	not be mistaken for an acknowledgement of a new local throw or shot. */
	if (!distributed_checkpoint_ready())
		return;
	distributed_client_ready_picked_up_weapons();
	distributed_client_note_own_inventories();
	mcc_network_client_tick();
	distributed_client_send_vehicles();
	distributed_client_carry_unsteered_vehicles();
	/* (who it is, as its Discord told it, and its build: the checkpoint gate above means the
	host has answered a ready, which makes this a machine it takes messages of. Not
	"once the ready's time is set": a checkpoint boundary adopted in the same read as
	the snapshot's end clears that time (network_objects_reverted) with no tick in
	between to ask again, and both were then never sent) */
	distributed_client_send_identity();
	distributed_client_send_build();
}

/* ---------- the game */

/* A checkpoint starts new prediction, inventory and animation histories even
when it restores the same object handles. Keep ownership and the host's last
object list: its next comparison still needs to send deletions. */
void network_objects_reverted(
	void)
{
	long absolute_index;
	short local_player_index;
	short index;

	mcc_network_new_game();
	csmemset(objects_host_inventories, 0, sizeof(objects_host_inventories));
	host_damage_animation_count = 0;
	csmemset(client_damage_animations, 0, sizeof(client_damage_animations));
	for (absolute_index = 0; absolute_index < MAXIMUM_TRACKED_OBJECTS; absolute_index++)
	{
		objects_host_state_moving[absolute_index] = FALSE;
		objects_host_rest_times[absolute_index] = NONE;
		objects_host_inventories[absolute_index].carried_time = NONE;
		objects_host_inventories[absolute_index].ammunition_time = NONE;
		objects_client_dead[absolute_index].object_index = NONE;
		objects_client_checkpoint_pending[absolute_index] = TRUE;
		objects_client_unsteered_path[absolute_index].object_index = NONE;
		objects_client_unsteered_path[absolute_index].time = NONE;
		objects_client_unsteered_path[absolute_index].interval = 0;
	}
	objects_host_resting_cursor = 0;
	objects_host_viewers.count = 0;
	csmemset(objects_host_vehicle_predictions, 0, sizeof(objects_host_vehicle_predictions));
	for (index = 0; index < MAXIMUM_TRACKED_PLAYERS; index++)
	{
		objects_host_vehicle_predictions[index].accepted_vehicle_index = NONE;
		objects_host_vehicle_predictions[index].ground_vehicle_index = NONE;
		objects_host_vehicle_predictions[index].rest_vehicle_index = NONE;
	}
	objects_client_synchronized = FALSE;
	objects_client_ready_time = NONE;
	objects_client_ready_interval = CLIENT_READY_INTERVAL_TICKS;
	objects_client_failed_time = NONE;
	objects_client_retry_ticks = CLIENT_RETRY_TICKS;
	objects_client_ask_again = FALSE;
	objects_client_new_object_count = 0;
	objects_client_retried_inventory_count = 0;
	objects_client_check_all = TRUE;
	objects_client_local_free_index = LOCAL_OBJECTS_FIRST_INDEX;
	objects_client_object_count = NONE;
	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		objects_client_own_inventories[local_player_index].unit_index = NONE;
		objects_client_picked_up_weapons[local_player_index].definition_index = NONE;
		for (index = 0; index < OWN_VEHICLE_POSITION_TICKS; index++)
		{
			objects_client_own_vehicles[local_player_index][index].time = NONE;
			objects_client_own_vehicles[local_player_index][index].vehicle_index = NONE;
		}
	}
}

/* Client projectiles and their lingering effects belong to the discarded
prediction. They must not produce new-epoch hits or objects from an old shot. */
void network_objects_discard_client_effects(void)
{
	struct object_iterator iterator;

	if (!network_game_distributed_client())
		return;
	object_iterator_new(&iterator, _object_mask_projectile, 0);
	while (object_iterator_next(&iterator))
		object_delete_immediately(iterator.index);
	effects_discard_gameplay();
}

void network_objects_new_game(
	void)
{
	long absolute_index;
	short machine_index;

	network_objects_reverted();
	for (absolute_index = 0; absolute_index < MAXIMUM_TRACKED_OBJECTS; absolute_index++)
	{
		objects_host_told[absolute_index] = NONE;
		objects_client_has[absolute_index] = NONE;
	}
	objects_host_told_count = 0;
	for (machine_index = 0; machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES; machine_index++)
		objects_host_machines[machine_index].time = NONE;
	objects_client_local_allocation = FALSE;
	objects_client_creating_index = NONE;
	objects_client_creating = FALSE;
	objects_client_deleting = FALSE;
}

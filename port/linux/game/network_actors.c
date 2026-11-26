/*
NETWORK_ACTORS.C

AI sync (port/linux/NETCODE.md).

Only the host runs the AI (game.c skips ai_update on clients). An actor
drives its unit the way a player does, through unit_control. The clients
have the units (network_objects.c); this file makes them move like the
host's.

Each tick the host records the control every actor gave its unit (a hook
in unit_control, and the bursts it holds the trigger for) and sends it to
each client with the unit's state: position, seat, shields and health,
camouflage, and whether it flees in a panic. Units near a client's players
are sent every tick, others less often, but a unit starting a one-tick
action (a jump, a grenade) goes to everyone that tick. Each tick a client
feeds the latest control into unit_control, the same path that drives a
remote player's unit, so the unit walks, aims, fires and throws as it does
on the host. The state corrects any drift.

What the AI starts once is numbered, so a client plays each once however
many times it receives it: animation impulses (dives, vaults), melee
attacks and leaps (which the AI starts directly, not through the control),
speech, and custom animations (outside co-op, whose events carry them).
Pain and death sounds aren't sent: a client plays those itself as it
replays the host's damage.

A client creates no actors of its own (actors.c): the host's are the only
ones. Deaths need nothing here: network_damage.c kills a client's copy of
an AI unit the same way it kills a player's.
*/

/* ---------- headers */

#include "cseries.h"
#include "game/game.h"
#include "networking/network_game_globals.h"
#include "objects/objects.h"
#include "objects/damage.h"
#include "sound/game_sound.h"
#include "sound/sound_definitions.h"
#include "units/units.h"
#include "units/unit_control_data.h"
#include "network_coop.h"
#include "network_distributed.h"
#include "vehicle_riding.h"

#include <math.h>

/* ---------- constants */

enum
{
	/* AI units tracked at once: every actor (halo_port_capacity.h's
	HALO_PORT_MAXIMUM_ACTORS) and the units the cutscenes' recorded
	animations drive */
	MAXIMUM_NETWORK_ACTORS = HALO_PORT_MAXIMUM_ACTORS + 32,
	MAXIMUM_ENTRIES_PER_MESSAGE = 64,
	/* A client keeps applying a control for this long after the host last
	sent one. After that the unit is left alone: the actor let go of it, or
	the host went quiet. */
	CONTROL_HELD_TICKS = 2 * TICKS_PER_SECOND,
	/* an impulse is repeated in this many entries, in case one is lost */
	IMPULSE_REPEAT_TICKS = 3,
	/* the number of unit_start_animation_impulse impulses (private to units.c) */
	NUMBER_OF_UNIT_ANIMATION_IMPULSES = 14,
	NO_IMPULSE = 0xFF,
	NO_TEAM = 0xFF,

	/* An entry's impulse is one of the animation impulses or, past them, an
	action the AI starts directly: a melee attack (unit_melee_attack_begin)
	or a leap (unit_leap_begin). */
	_actor_action_melee = NUMBER_OF_UNIT_ANIMATION_IMPULSES,
	_actor_action_leap,
	NUMBER_OF_ACTOR_ACTIONS,

	/* control flags a unit acts on in one tick: a unit that sets one goes to
	every client that tick, however far away, so the action isn't missed */
	ONE_SHOT_CONTROL_FLAGS = FLAG(_unit_control_jump_bit) | FLAG(_unit_control_action_bit) |
		FLAG(_unit_control_weapon_reload_bit) | FLAG(_unit_control_throw_grenade_bit) |
		FLAG(_unit_control_swap_weapons_bit),
};

/* struct distributed_actor_state flags */
enum
{
	_distributed_actor_rides_bit = 0,
	_distributed_actor_shield_depleted_bit,
	_distributed_actor_shield_charging_bit,
	_distributed_actor_shield_over_charging_bit,
	_distributed_actor_camouflaged_bit,
	_distributed_actor_super_camouflaged_bit,
	/* the impulse turns the unit to impulse_alignment */
	_distributed_actor_impulse_aligned_bit,
	/* fleeing in a panic, toward run_blindly_angle (unit_start_running_blindly) */
	_distributed_actor_running_blindly_bit,
};

/* facing, aiming and looking vectors are sent as yaw and pitch */
enum
{
	_angle_yaw,
	_angle_pitch,
	NUMBER_OF_ANGLES
};

/* ---------- structures */

/* an AI unit's control and state at the end of the host's tick (like
struct distributed_unit_state for players) */
struct distributed_actor_state
{
	long unit_index;
	long vehicle_index;
	short seat_index;
	byte flags;
	/* the impulse's sequence number; impulse is NO_IMPULSE when there is none */
	byte impulse_number;
	byte impulse;
	byte animation_state;
	byte aiming_speed;
	byte primary_trigger;
	word control_flags;
	signed char throttle[3];
	byte active_camouflage;
	/* the speech's sequence number; 0 when there is none */
	byte speech_number;
	/* the unit's team (NO_TEAM for none): the AI sets it after the unit is
	made, so a client's copy would keep the team it was made with */
	byte team;
	short facing[NUMBER_OF_ANGLES];
	short aiming[NUMBER_OF_ANGLES];
	short looking[NUMBER_OF_ANGLES];
	short impulse_alignment;
	short run_blindly_angle;
	word body_vitality;
	word shield_vitality;
	/* the sound the unit started saying (a sound tag) */
	long speech_sound;
	/* the custom animation it started (unit_start_user_animation), and its
	sequence number; the graph is NONE when there is none */
	long user_animation_graph;
	short user_animation;
	byte user_animation_number;
	byte user_animation_interpolate;
	real_point3d position;
	struct distributed_vector velocity;
	struct distributed_vector forward;
	struct distributed_vector up;
};

/* the damage an AI unit is taking: its shield's flare and its body's wounds
are drawn by it */
struct distributed_actor_damage
{
	long unit_index;
	word current_shield_damage;
	word recent_shield_damage;
	word current_body_damage;
	word recent_body_damage;
};

struct distributed_actor_damage_message
{
	struct distributed_message_header header;
	struct distributed_actor_damage units[MAXIMUM_ENTRIES_PER_MESSAGE];
};

struct distributed_actor_state_message
{
	struct distributed_message_header header;
	struct distributed_actor_state states[MAXIMUM_ENTRIES_PER_MESSAGE];
};

/* host: what an actor gave its unit this tick */
struct host_actor
{
	long unit_index;
	/* got a control since the last send (noted during game_tick, sent after it) */
	boolean noted;
	/* its damage was sent last tick (and once more when it ends) */
	boolean damaged;
	struct unit_control_data control;
	/* the control flags of the last tick's entry */
	word sent_control_flags;
	/* the latest impulse, how many more entries carry it, and its number */
	short impulse;
	short impulse_sends;
	byte impulse_number;
	boolean impulse_aligned;
	real_vector2d impulse_alignment;
	/* the latest speech, how many more entries carry it, and its number */
	long speech_sound;
	short speech_sends;
	byte speech_number;
	/* the latest custom animation, the same way */
	long user_animation_graph;
	short user_animation;
	boolean user_animation_interpolate;
	short user_animation_sends;
	byte user_animation_number;
};

/* client: the latest the host sent about an AI unit */
struct client_actor
{
	long unit_index;
	long received_time;
	struct unit_control_data control;
	/* the impulse to play, and the number of the last one played */
	short impulse;
	byte impulse_number;
	byte played_impulse_number;
	boolean impulse_aligned;
	real_vector2d impulse_alignment;
	/* the speech to play (NONE when there is none), and the number of the last played */
	long speech_sound;
	byte speech_number;
	byte played_speech_number;
	/* the custom animation to play (graph NONE when there is none), and the
	number of the last played */
	long user_animation_graph;
	short user_animation;
	boolean user_animation_interpolate;
	byte user_animation_number;
	byte played_user_animation_number;
	boolean driven;
};

/* ---------- globals */

static struct host_actor host_actors[MAXIMUM_NETWORK_ACTORS];
static short host_actor_count;
/* host: the numbers of every unit's impulses, speech and animations, from
one counter, so a unit that drops out of the table and comes back can't
repeat the number a client last played */
static byte host_event_number;

static struct client_actor client_actors[MAXIMUM_NETWORK_ACTORS];
static short client_actor_count;

/* ---------- private code */

static void angles_pack(
	real_vector3d const *vector,
	short *angles)
{
	angles[_angle_yaw] = distributed_angle_pack((real)atan2(vector->j, vector->i));
	angles[_angle_pitch] = distributed_angle_pack((real)asin(PIN(vector->k, -1.0f, 1.0f)));
}

static void angles_unpack(
	short const *angles,
	real_vector3d *vector)
{
	real yaw = distributed_angle_unpack(angles[_angle_yaw], FALSE);
	real pitch = distributed_angle_unpack(angles[_angle_pitch], TRUE);

	vector->i = (real)(cos(pitch) * cos(yaw));
	vector->j = (real)(cos(pitch) * sin(yaw));
	vector->k = (real)sin(pitch);
}

/* host: the next event's number (never 0, which a client's new entry has
as played) */
static byte next_event_number(
	void)
{
	if (++host_event_number == 0)
		host_event_number = 1;
	return host_event_number;
}

/* the unit's entry, adding one if needed; NULL if the table is full */
static struct host_actor *host_actor_for(
	long unit_index)
{
	short index;

	for (index = 0; index < host_actor_count; index++)
	{
		if (host_actors[index].unit_index == unit_index)
			return &host_actors[index];
	}
	if (host_actor_count >= MAXIMUM_NETWORK_ACTORS)
		return NULL;
	csmemset(&host_actors[host_actor_count], 0, sizeof(host_actors[0]));
	host_actors[host_actor_count].unit_index = unit_index;
	host_actors[host_actor_count].impulse = NONE;
	host_actors[host_actor_count].speech_sound = NONE;
	host_actors[host_actor_count].user_animation_graph = NONE;
	return &host_actors[host_actor_count++];
}

static struct client_actor *client_actor_for(
	long unit_index)
{
	short index;

	for (index = 0; index < client_actor_count; index++)
	{
		if (client_actors[index].unit_index == unit_index)
			return &client_actors[index];
	}
	if (client_actor_count >= MAXIMUM_NETWORK_ACTORS)
		return NULL;
	csmemset(&client_actors[client_actor_count], 0, sizeof(client_actors[0]));
	client_actors[client_actor_count].unit_index = unit_index;
	client_actors[client_actor_count].impulse = NONE;
	client_actors[client_actor_count].speech_sound = NONE;
	client_actors[client_actor_count].user_animation_graph = NONE;
	return &client_actors[client_actor_count++];
}

/* whether the unit exists, is alive, and isn't a player's */
static boolean actor_unit_valid(
	long unit_index)
{
	struct unit_datum *unit;

	if (!object_try_and_get_and_verify_type(unit_index, _object_mask_unit))
		return FALSE;
	unit = unit_get(unit_index);
	return !TEST_FLAG(unit->object.damage_flags, _object_dead_bit) && unit->unit.player_index == NONE;
}

/* host: builds the entry to send for an AI unit */
static void actor_state_from_unit(
	struct host_actor const *actor,
	struct distributed_actor_state *state)
{
	struct unit_datum *unit = unit_get(actor->unit_index);
	struct damage_network_state damage;
	real camouflage = PIN(unit->unit.active_camouflage, 0.0f, 1.0f);

	csmemset(state, 0, sizeof(*state));
	state->unit_index = actor->unit_index;
	state->vehicle_index = NONE;
	state->seat_index = NONE;
	if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
	{
		SET_FLAG(state->flags, _distributed_actor_rides_bit, TRUE);
		state->vehicle_index = unit->object.parent_object_index;
		state->seat_index = unit->unit.parent_seat_index;
	}
	state->animation_state = (byte)actor->control.animation_state;
	state->aiming_speed = (byte)actor->control.aiming_speed;
	state->primary_trigger = (byte)(long)floor(PIN(actor->control.primary_trigger, 0.0f, 1.0f) * 255.0f + 0.5f);
	state->control_flags = actor->control.control_flags;
	/* for a burst the AI holds the trigger for (unit_persistent_control),
	send the flags and trigger the unit's update produced this tick */
	if (unit->unit.persistent_control_timer > 0)
	{
		state->control_flags = (word)(unit->unit.control_flags & (FLAG(NUMBER_OF_UNIT_CONTROL_FLAGS) - 1));
		state->primary_trigger = (byte)(long)floor(PIN(unit->unit.primary_trigger, 0.0f, 1.0f) * 255.0f + 0.5f);
	}
	state->throttle[0] = (signed char)(long)floor(PIN(actor->control.throttle.i, -1.0f, 1.0f) * 127.0f + 0.5f);
	state->throttle[1] = (signed char)(long)floor(PIN(actor->control.throttle.j, -1.0f, 1.0f) * 127.0f + 0.5f);
	state->throttle[2] = (signed char)(long)floor(PIN(actor->control.throttle.k, -1.0f, 1.0f) * 127.0f + 0.5f);
	angles_pack(&actor->control.facing_vector, state->facing);
	angles_pack(&actor->control.aiming_vector, state->aiming);
	angles_pack(&actor->control.looking_vector, state->looking);
	state->impulse = NO_IMPULSE;
	if (actor->impulse != NONE && actor->impulse_sends > 0)
	{
		state->impulse = (byte)actor->impulse;
		state->impulse_number = actor->impulse_number;
		SET_FLAG(state->flags, _distributed_actor_impulse_aligned_bit, actor->impulse_aligned);
		if (actor->impulse_aligned)
		{
			state->impulse_alignment = distributed_angle_pack(
				(real)atan2(actor->impulse_alignment.j, actor->impulse_alignment.i));
		}
	}
	state->speech_sound = NONE;
	if (actor->speech_sound != NONE && actor->speech_sends > 0)
	{
		state->speech_sound = actor->speech_sound;
		state->speech_number = actor->speech_number;
	}
	state->user_animation_graph = NONE;
	if (actor->user_animation_graph != NONE && actor->user_animation_sends > 0)
	{
		state->user_animation_graph = actor->user_animation_graph;
		state->user_animation = actor->user_animation;
		state->user_animation_interpolate = (byte)actor->user_animation_interpolate;
		state->user_animation_number = actor->user_animation_number;
	}
	if (TEST_FLAG(unit->unit.flags, _unit_running_blindly_bit))
	{
		SET_FLAG(state->flags, _distributed_actor_running_blindly_bit, TRUE);
		state->run_blindly_angle = distributed_angle_pack(unit->unit.run_blindly_angle);
	}
	damage_get_network_state(actor->unit_index, &damage);
	SET_FLAG(state->flags, _distributed_actor_shield_depleted_bit, damage.shield_depleted);
	SET_FLAG(state->flags, _distributed_actor_shield_charging_bit, damage.shield_charging);
	SET_FLAG(state->flags, _distributed_actor_shield_over_charging_bit, damage.shield_over_charging);
	state->body_vitality = distributed_vitality_pack(damage.body_vitality);
	state->shield_vitality = distributed_vitality_pack(damage.shield_vitality);
	SET_FLAG(state->flags, _distributed_actor_camouflaged_bit,
		TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit));
	SET_FLAG(state->flags, _distributed_actor_super_camouflaged_bit,
		TEST_FLAG(unit->unit.flags, _unit_super_camouflaged_bit));
	state->active_camouflage = (byte)(long)floor(camouflage * 255.0f + 0.5f);
	state->team = unit->object.owner_team_index >= 0 && unit->object.owner_team_index < NO_TEAM ?
		(byte)unit->object.owner_team_index : NO_TEAM;
	state->position = unit->object.position;
	distributed_vector_pack(&unit->object.translational_velocity, DISTRIBUTED_VELOCITY_SCALE, &state->velocity);
	distributed_vector_pack(&unit->object.forward, DISTRIBUTED_UNIT_SCALE, &state->forward);
	distributed_vector_pack(&unit->object.up, DISTRIBUTED_UNIT_SCALE, &state->up);
}

/* Client: stores the entry's control and applies its state to the unit.
Returns FALSE for an entry with invalid values. */
static boolean actor_state_apply(
	struct distributed_actor_state const *state,
	long now)
{
	struct client_actor *actor;
	struct unit_datum *unit;
	struct damage_network_state damage;
	real_vector3d velocity;
	real_vector3d forward;
	real_vector3d up;

	if (!distributed_object_index_valid(state->unit_index) || !network_objects_client_has(state->unit_index))
		return FALSE;
	/* (a body the host's unit got back up from: a Flood combat form that
	feigned death, which it does only once its body is at rest. Left dead,
	the ragdoll was dragged about by the host's positions. The host sends
	only living units.) */
	if (object_try_and_get_and_verify_type(state->unit_index, _object_mask_unit) &&
		TEST_FLAG(unit_get(state->unit_index)->object.damage_flags, _object_dead_bit) &&
		TEST_FLAG(unit_get(state->unit_index)->object.flags, _object_at_rest_bit) &&
		unit_get(state->unit_index)->unit.player_index == NONE && state->body_vitality > 0)
	{
		unit_port_resurrect(state->unit_index);
	}
	if (!actor_unit_valid(state->unit_index) ||
		state->animation_state >= NUMBER_OF_UNIT_ANIMATION_STATES ||
		state->aiming_speed >= NUMBER_OF_UNIT_AIMING_SPEEDS ||
		!VALID_FLAGS(state->control_flags, NUMBER_OF_UNIT_CONTROL_FLAGS) ||
		(state->impulse != NO_IMPULSE && state->impulse >= NUMBER_OF_ACTOR_ACTIONS) ||
		(state->speech_sound != NONE && !distributed_tag_of_group(state->speech_sound, SOUND_DEFINITION_TAG)) ||
		(state->user_animation_graph != NONE &&
			!distributed_graph_animation(state->user_animation_graph, state->user_animation)) ||
		!distributed_point_valid(&state->position, UNIT_WORLD_BOUND))
	{
		return FALSE;
	}
	distributed_vector_unpack(&state->velocity, DISTRIBUTED_VELOCITY_SCALE, &velocity);
	distributed_vector_unpack(&state->forward, DISTRIBUTED_UNIT_SCALE, &forward);
	distributed_vector_unpack(&state->up, DISTRIBUTED_UNIT_SCALE, &up);
	if (!distributed_axes_make_valid(&forward, &up))
		return FALSE;
	actor = client_actor_for(state->unit_index);
	if (!actor)
		return TRUE;

	/* the control, applied every tick until the next entry */
	actor->received_time = now;
	actor->control.animation_state = (char)state->animation_state;
	actor->control.aiming_speed = (char)state->aiming_speed;
	actor->control.control_flags = state->control_flags;
	actor->control.weapon_index = NONE;
	actor->control.grenade_index = NONE;
	actor->control.zoom_level = NONE;
	actor->control.throttle.i = (real)state->throttle[0] / 127.0f;
	actor->control.throttle.j = (real)state->throttle[1] / 127.0f;
	actor->control.throttle.k = (real)state->throttle[2] / 127.0f;
	actor->control.primary_trigger = (real)state->primary_trigger / 255.0f;
	angles_unpack(state->facing, &actor->control.facing_vector);
	angles_unpack(state->aiming, &actor->control.aiming_vector);
	angles_unpack(state->looking, &actor->control.looking_vector);
	if (state->impulse != NO_IMPULSE && state->impulse_number != actor->played_impulse_number)
	{
		real yaw = distributed_angle_unpack(state->impulse_alignment, FALSE);

		actor->impulse = state->impulse;
		actor->impulse_number = state->impulse_number;
		actor->impulse_aligned = TEST_FLAG(state->flags, _distributed_actor_impulse_aligned_bit);
		actor->impulse_alignment.i = (real)cos(yaw);
		actor->impulse_alignment.j = (real)sin(yaw);
	}

	if (state->speech_sound != NONE && state->speech_number != actor->played_speech_number)
	{
		actor->speech_sound = state->speech_sound;
		actor->speech_number = state->speech_number;
	}
	if (state->user_animation_graph != NONE && state->user_animation_number != actor->played_user_animation_number)
	{
		actor->user_animation_graph = state->user_animation_graph;
		actor->user_animation = state->user_animation;
		actor->user_animation_interpolate = state->user_animation_interpolate != 0;
		actor->user_animation_number = state->user_animation_number;
	}

	/* the state: seat, shields and health, camouflage, running blindly, and
	position (a unit in a vehicle goes where the vehicle goes) */
	unit = unit_get(state->unit_index);
	if (TEST_FLAG(state->flags, _distributed_actor_rides_bit))
	{
		if (unit->object.parent_object_index != state->vehicle_index ||
			unit->unit.parent_seat_index != state->seat_index)
		{
			network_objects_set_seat(state->unit_index, state->vehicle_index, state->seat_index);
		}
	}
	else if (unit->object.parent_object_index != NONE)
	{
		network_objects_set_seat(state->unit_index, NONE, NONE);
	}
	damage_get_network_state(state->unit_index, &damage);
	damage.shield_depleted = TEST_FLAG(state->flags, _distributed_actor_shield_depleted_bit);
	damage.shield_charging = TEST_FLAG(state->flags, _distributed_actor_shield_charging_bit);
	damage.shield_over_charging = TEST_FLAG(state->flags, _distributed_actor_shield_over_charging_bit);
	damage.body_vitality = distributed_vitality_unpack(state->body_vitality);
	damage.shield_vitality = distributed_vitality_unpack(state->shield_vitality);
	damage_set_network_state(state->unit_index, &damage);
	SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit,
		TEST_FLAG(state->flags, _distributed_actor_camouflaged_bit));
	SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit,
		TEST_FLAG(state->flags, _distributed_actor_super_camouflaged_bit));
	unit->unit.active_camouflage = (real)state->active_camouflage / 255.0f;
	/* (the game's 10 teams, game_allegiance.c) */
	if (state->team != NO_TEAM && state->team < 10)
		unit->object.owner_team_index = state->team;
	SET_FLAG(unit->unit.flags, _unit_running_blindly_bit,
		TEST_FLAG(state->flags, _distributed_actor_running_blindly_bit));
	if (TEST_FLAG(state->flags, _distributed_actor_running_blindly_bit))
		unit->unit.run_blindly_angle = distributed_angle_unpack(state->run_blindly_angle, FALSE);
	/* vehicles are placed only by the host's object states (network_objects.c) */
	if (unit->object.parent_object_index == NONE && unit->object.type != _object_type_vehicle)
	{
		real dx = state->position.x - unit->object.position.x;
		real dy = state->position.y - unit->object.position.y;
		real dz = state->position.z - unit->object.position.z;

		if (dx * dx + dy * dy + dz * dz > REMOTE_CORRECTION_TOLERANCE * REMOTE_CORRECTION_TOLERANCE &&
			network_objects_reconcile(state->unit_index, &state->position, &forward, &up, &velocity, NULL,
				REMOTE_BLEND_DISTANCE, _network_vehicle_write_state))
		{
			distributed_count_correction();
		}
	}
	return TRUE;
}

/* Client: plays a sound at the unit's head, as unit_dialogue_update starts
a unit's speech. */
static void speech_play(
	long unit_index,
	long sound_definition_index)
{
	struct object_marker marker;
	real_point3d position = *global_origin3d;
	real_vector3d forward = *global_forward3d;
	short node_index = 0;

	if (object_get_marker_by_name(unit_index, "head", &marker, 1))
	{
		node_index = marker.node_index;
		position = marker.node_matrix.position;
		forward = marker.node_matrix.forward;
	}
	object_impulse_sound_new(unit_index, sound_definition_index, node_index, &position, &forward, 1.0f);
}

/* ---------- public code */

void network_actors_new_game(
	void)
{
	host_actor_count = 0;
	client_actor_count = 0;
}

/* host: unit_control calls this when an actor drives its unit */
void network_actors_note_control(
	long unit_index,
	struct unit_control_data const *control_data)
{
	struct host_actor *actor;

	if (game_connection() != _game_connection_network_server)
		return;
	actor = host_actor_for(unit_index);
	if (!actor)
		return;
	actor->noted = TRUE;
	actor->control = *control_data;
}

/* host: an impulse or action for an AI unit, sent in the next few entries */
static void note_action(
	long unit_index,
	short action,
	real_vector2d const *alignment_vector)
{
	struct host_actor *actor;

	if (game_connection() != _game_connection_network_server)
		return;
	actor = host_actor_for(unit_index);
	if (!actor)
		return;
	actor->impulse = action;
	actor->impulse_sends = IMPULSE_REPEAT_TICKS;
	actor->impulse_number = next_event_number();
	actor->impulse_aligned = alignment_vector != NULL;
	if (alignment_vector)
		actor->impulse_alignment = *alignment_vector;
}

/* host: unit_start_animation_impulse calls this for an AI unit */
void network_actors_note_impulse(
	long unit_index,
	short animation_impulse,
	real_vector2d const *alignment_vector)
{
	note_action(unit_index, animation_impulse, alignment_vector);
}

/* host: unit_melee_attack_begin calls this when an AI unit starts a melee attack */
void network_actors_note_melee(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	note_action(unit_index, _actor_action_melee, alignment_vector);
}

/* host: unit_leap_begin calls this when an AI unit leaps */
void network_actors_note_leap(
	long unit_index,
	real_vector2d const *alignment_vector)
{
	note_action(unit_index, _actor_action_leap, alignment_vector);
}

/* host: unit_dialogue_update calls this when an AI unit starts saying
something (not a pain or death sound, which clients play themselves) */
void network_actors_note_speech(
	long unit_index,
	long sound_definition_index)
{
	struct host_actor *actor;

	if (game_connection() != _game_connection_network_server)
		return;
	actor = host_actor_for(unit_index);
	if (!actor)
		return;
	actor->speech_sound = sound_definition_index;
	actor->speech_sends = IMPULSE_REPEAT_TICKS;
	actor->speech_number = next_event_number();
}

/* host: unit_start_user_animation calls this when an AI unit starts a
custom animation (co-op's events carry them in co-op) */
void network_actors_note_user_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index,
	boolean interpolate)
{
	struct host_actor *actor;

	if (game_connection() != _game_connection_network_server || network_coop_active())
		return;
	actor = host_actor_for(unit_index);
	if (!actor)
		return;
	actor->user_animation_graph = animation_graph_index;
	actor->user_animation = animation_index;
	actor->user_animation_interpolate = interpolate;
	actor->user_animation_sends = IMPULSE_REPEAT_TICKS;
	actor->user_animation_number = next_event_number();
}

/* host: the damage of the AI units taking some, to every client (and once
more when it ends, so a flare goes) */
static void host_send_damage(
	void)
{
	struct distributed_actor_damage_message message;
	short count = 0;
	short index;

	for (index = 0; index < host_actor_count; index++)
	{
		struct host_actor *actor = &host_actors[index];
		struct damage_network_state damage;
		boolean damaged;

		if (!actor_unit_valid(actor->unit_index))
			continue;
		damage_get_network_state(actor->unit_index, &damage);
		damaged = damage.current_shield_damage > 0.0f || damage.recent_shield_damage > 0.0f ||
			damage.current_body_damage > 0.0f || damage.recent_body_damage > 0.0f;
		if (!damaged && !actor->damaged)
			continue;
		actor->damaged = damaged;
		message.units[count].unit_index = actor->unit_index;
		message.units[count].current_shield_damage = distributed_vitality_pack(damage.current_shield_damage);
		message.units[count].recent_shield_damage = distributed_vitality_pack(damage.recent_shield_damage);
		message.units[count].current_body_damage = distributed_vitality_pack(damage.current_body_damage);
		message.units[count].recent_body_damage = distributed_vitality_pack(damage.recent_body_damage);
		if (++count == MAXIMUM_ENTRIES_PER_MESSAGE)
		{
			distributed_send(&message, _distributed_message_actor_damage, count,
				(word)(sizeof(message.header) + count * sizeof(message.units[0])), _distributed_to_clients);
			count = 0;
		}
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_actor_damage, count,
			(word)(sizeof(message.header) + count * sizeof(message.units[0])), _distributed_to_clients);
	}
}

/* Host, after each tick: sends each client the AI units driven this tick.
Units near the client's players go every tick, others less often. A unit
with a fresh impulse, action, speech or animation goes to everyone in each
of the next IMPULSE_REPEAT_TICKS ticks, and one starting a one-tick action
goes that tick. A unit no actor drove this tick is dropped from the table,
unless it is fleeing or seated: fleeing units move without their actor's
control, and an idle seated actor still needs its seat sent to the clients. */
void network_actors_host_tick(
	void)
{
	static struct distributed_actor_state states[MAXIMUM_NETWORK_ACTORS];
	/* sent to every client this tick, whatever its period */
	static boolean urgent[MAXIMUM_NETWORK_ACTORS];
	struct distributed_actor_state_message message;
	long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short machine_count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	long now = game_time_get();
	short state_count = 0;
	short index;
	short machine_number;

	for (index = 0; index < host_actor_count; index++)
	{
		struct host_actor *actor = &host_actors[index];

		if (!actor_unit_valid(actor->unit_index) ||
			(!actor->noted && !TEST_FLAG(unit_get(actor->unit_index)->unit.flags, _unit_running_blindly_bit) &&
			(!vehicle_riding_enabled() || unit_get(actor->unit_index)->object.parent_object_index == NONE ||
			unit_get(actor->unit_index)->unit.parent_seat_index == NONE)))
		{
			host_actors[index--] = host_actors[--host_actor_count];
			continue;
		}
		actor_state_from_unit(actor, &states[state_count]);
		urgent[state_count] = states[state_count].impulse != NO_IMPULSE || states[state_count].speech_sound != NONE ||
			states[state_count].user_animation_graph != NONE ||
			(states[state_count].control_flags & ~actor->sent_control_flags & ONE_SHOT_CONTROL_FLAGS) != 0;
		actor->sent_control_flags = states[state_count].control_flags;
		state_count++;
		actor->noted = FALSE;
		if (actor->impulse_sends > 0)
			actor->impulse_sends--;
		if (actor->speech_sends > 0)
			actor->speech_sends--;
		if (actor->user_animation_sends > 0)
			actor->user_animation_sends--;
	}
	host_send_damage();
	for (machine_number = 0; machine_number < machine_count; machine_number++)
	{
		long machine_index = machine_indices[machine_number];
		short count = 0;

		for (index = 0; index < state_count; index++)
		{
			struct distributed_actor_state const *state = &states[index];
			short period = network_objects_send_period(machine_index, &state->position);

			if (!urgent[index] && (now + DATUM_INDEX_TO_ABSOLUTE_INDEX(state->unit_index)) % period != 0)
				continue;
			message.states[count++] = *state;
			if (count == MAXIMUM_ENTRIES_PER_MESSAGE || count == DATAGRAM_ENTRIES(struct distributed_actor_state))
			{
				distributed_send_to_machine(machine_index, &message, _distributed_message_actor_states, count,
					(word)(sizeof(message.header) + count * sizeof(struct distributed_actor_state)));
				count = 0;
			}
		}
		if (count)
		{
			distributed_send_to_machine(machine_index, &message, _distributed_message_actor_states, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_actor_state)));
		}
	}
}

word network_actors_damage_entry_size(
	void)
{
	return sizeof(struct distributed_actor_damage);
}

/* client: the damage the host's AI units are taking */
void network_actors_handle_damage(
	void const *entries,
	short count)
{
	struct distributed_actor_damage const *units = entries;
	short index;

	for (index = 0; index < count; index++)
	{
		struct damage_network_state damage;

		if (!distributed_object_index_valid(units[index].unit_index) ||
			!network_objects_client_has(units[index].unit_index) || !actor_unit_valid(units[index].unit_index))
		{
			continue;
		}
		damage_get_network_state(units[index].unit_index, &damage);
		damage.current_shield_damage = distributed_vitality_unpack(units[index].current_shield_damage);
		damage.recent_shield_damage = distributed_vitality_unpack(units[index].recent_shield_damage);
		damage.current_body_damage = distributed_vitality_unpack(units[index].current_body_damage);
		damage.recent_body_damage = distributed_vitality_unpack(units[index].recent_body_damage);
		damage_set_network_state(units[index].unit_index, &damage);
	}
}

word network_actors_entry_size(
	void)
{
	return sizeof(struct distributed_actor_state);
}

/* client: entries from the host */
void network_actors_handle_states(
	void const *entries,
	short count)
{
	struct distributed_actor_state const *states = entries;
	long now = game_time_get();
	short index;

	for (index = 0; index < count; index++)
		actor_state_apply(&states[index], now);
}

/* Client: the unit no longer driven by the host's control: no longer
actively controlled, and a vehicle left as a recording leaves it when it ends
(recorded_animations_update) */
static void client_actor_release(
	struct client_actor *actor)
{
	if (actor->driven && object_try_and_get_and_verify_type(actor->unit_index, _object_mask_unit))
	{
		unit_set_actively_controlled(actor->unit_index, FALSE);
		if (object_get(actor->unit_index)->object.type == _object_type_vehicle)
		{
			unit_set_possessed(actor->unit_index, FALSE);
			object_set_automatic_deactivation(actor->unit_index, TRUE);
		}
	}
	actor->driven = FALSE;
}

/* Client: called at the point in the tick where the host runs the AI.
Drives each AI unit with the last control the host sent. */
void network_actors_drive(
	void)
{
	long now = game_time_get();
	short index;

	for (index = 0; index < client_actor_count; index++)
	{
		struct client_actor *actor = &client_actors[index];
		boolean vehicle;

		if (!actor_unit_valid(actor->unit_index) || now - actor->received_time > CONTROL_HELD_TICKS)
		{
			client_actor_release(actor);
			client_actors[index--] = client_actors[--client_actor_count];
			continue;
		}
		/* port: a vehicle too, as a recording takes it on the host
		(recorded_animation_play_internal): possessed, and kept active away
		from the players. Only a recording gives a vehicle a control of its own
		(the AI and the players drive bipeds, whose vehicles take their
		driver's): a cutscene's dropships, Mercury Rising II's Falcons, which
		fly the dropships' recordings, and its intro's Warthog. A possessed
		vehicle keeps that control rather than take its driver's
		(unit_update), and what it makes of the control is what its functions
		read: its speed, its wheels' turn, its driver seat's power, so its
		rotors, wheels and engine animate by them. Kept active, it updates
		where a cutscene's camera watches it, far from the players, who alone
		keep the objects about them active. Its place is still the host's:
		network_objects.c carries a vehicle no player drives along the host's
		path after the tick's physics. Left out, a client's copy kept none of
		that (a Falcon flew with its rotors still). But not out of the map,
		where the host's flies on unseen and sends no place of it
		(distributed_host_placed_object): there the copy is left as it was
		until the host's states bring it back in, rather than fly the control
		by itself from wherever it last was. */
		vehicle = object_get(actor->unit_index)->object.type == _object_type_vehicle;
		if (vehicle && TEST_FLAG(object_get(actor->unit_index)->object.flags, _object_outside_of_map_bit))
		{
			client_actor_release(actor);
			continue;
		}
		if (!actor->driven)
		{
			unit_set_actively_controlled(actor->unit_index, TRUE);
			if (vehicle)
			{
				unit_set_possessed(actor->unit_index, TRUE);
				object_set_automatic_deactivation(actor->unit_index, FALSE);
			}
			actor->driven = TRUE;
		}
		unit_control(actor->unit_index, &actor->control);
		if (actor->impulse != NONE && actor->impulse_number != actor->played_impulse_number)
		{
			real_vector2d *alignment = actor->impulse_aligned ? &actor->impulse_alignment : NULL;

			if (actor->impulse == _actor_action_melee)
				unit_melee_attack_begin(actor->unit_index, FALSE, alignment);
			else if (actor->impulse == _actor_action_leap)
				unit_leap_begin(actor->unit_index, alignment);
			else
				unit_start_animation_impulse(actor->unit_index, actor->impulse, alignment);
			actor->played_impulse_number = actor->impulse_number;
			actor->impulse = NONE;
		}
		if (actor->speech_sound != NONE)
		{
			speech_play(actor->unit_index, actor->speech_sound);
			actor->played_speech_number = actor->speech_number;
			actor->speech_sound = NONE;
		}
		if (actor->user_animation_graph != NONE)
		{
			unit_port_play_user_animation(actor->unit_index, actor->user_animation_graph, actor->user_animation,
				actor->user_animation_interpolate, 0);
			actor->played_user_animation_number = actor->user_animation_number;
			actor->user_animation_graph = NONE;
		}
	}
}

/*
NETWORK_COOP.C

Campaign co-op over the network (port/linux/NETCODE.md).

A network game on a campaign map, with no game engine running, is co-op.
Only the host runs the map's scripts (game.c), so everything the scripts do
that clients need to see is sent from this file:

- Presentation, every tick: whether a cinematic is playing, the letterbox,
  the camera, the screen fade, the HUD settings the scripts control (what
  is shown, the mission timer), the players' maximum vitality, and the
  cutscene skip vote. A client plays them one a tick by the host's clock, a
  couple of ticks behind the newest, so the camera keeps an even pace. It
  starts and stops its own cinematic to match, and looks through the host's
  camera meanwhile. If the host goes quiet for two seconds, the client ends
  the cinematic so its players aren't stuck.
- Events, which happen once: script sounds (dialogue, music, ambience),
  chapter titles, help and objective text, "Checkpoint" messages, screen
  shake, nav points, and custom animations on units and scenery. Each is
  sent in three ticks' messages in case one is lost, and numbered so a
  client applies each exactly once, in order. The engine calls the
  network_coop_note_ functions (network_coop.h) to queue them.
- Devices (doors, elevators, switches, lights). A device moves toward its
  device group's value (devices.c). On a client nothing sets those values
  but this file: doors a player walks up to and switches a player uses are
  decided by the host (the player's action is relayed to it). The host
  sends a group's value for three ticks when it changes, plus a few groups
  every tick in rotation, those that have moved since the map loaded more
  often. A scenario group is identified by its index, which is the same on
  every machine; a device's own group by the device.
- Breakable surfaces (glass) the host breaks, each once as it breaks and
  a few at a time again in rotation (for a late joiner or a lost message).
- Named objects (scenery and devices) the scripts create or destroy.
  network_objects.c already handles units, vehicles, weapons and equipment.
  Twice a second the host sends which object names currently exist. A
  client that sees the same difference twice in a row, on the same BSP,
  creates or deletes its copy.
- State a late joiner would otherwise lack, resent every two seconds and on
  the tick a machine joins (host_resend): where scripted scenery and
  machines have moved, how objects look (permutations, scale), what the
  scripts have attached, the looping sounds playing, the full-screen
  cinematic effect and the nav points; on a join also the objective, the
  object names and every device that has moved.
- The view of a client with nobody to spectate yet: the host's view from
  behind, eased so it doesn't jerk.

The host also makes up for scripts written for one player: vitality they
set on player0 is set on every player (a10's shields), and a Pelican's
Warthog drop brings more Warthogs in a larger game.

Friendly fire between two players is also each player's own choice
(network.coop_own_friendly_fire): both must play with it. A client's input
says its choice, and the host, which alone deals damage, keeps everyone's.
A player away from the controls for over 70 seconds is hurt by every
player, whatever either chose, until their next input.

Skipping a cutscene is a vote. Pressing the skip key during a skippable
cutscene votes (a client sends its vote to the host every tick), and the
host skips once more than half the machines have voted. The skip itself is
the campaign's: the host reverts to the state saved when the cutscene began
and the script carries on past it. Clients catch up through the object,
device and name syncs. The host keeps its clock running through the revert,
because the netcode needs it to only go forward (network_coop_reverted).
A failed mission (game_lost, or the scripts' game_revert) reverts the same
way, rather than leaving the players stuck in the failure's cutscene.

Objects (scenery and devices) that aren't synced by network_objects.c are
found on a client by their scenario name, or, unnamed, by their object
index and tag, since the map placed them at the same index everywhere.
*/

/* ---------- headers */

#include "cseries.h"
#include "mcc_object_names.h"
#include "cseries/errors.h"
#include "cache/cache_files.h"
#include "camera/camera_scripting.h"
#include "camera/dead_camera.h"
#include "camera/director.h"
#include "camera/observer.h"
#include "cutscene/cinematics.h"
#include "devices/devices.h"
#include "effects/effect_definitions.h"
#include "effects/player_effects.h"
#include "game/game.h"
#include "game/game_allegiance.h"
#include "game/game_engine.h"
#include "game/players.h"
#include "game/player_queues_new.h"
#include "hs/hs.h"
#include "hs/hs_library_external.h"
#include "interface/hud.h"
#include "interface/hud_definitions.h"
#include "interface/hud_messaging.h"
#include "interface/hud_unit.h"
#include "interface/hud_weapon.h"
#include "main/main.h"
#include "models/model_animation_definitions.h"
#include "models/model_definitions.h"
#include "models/models.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "objects/object_definitions.h"
#include "objects/objects.h"
#include "objects/object_types.h"
#include "objects/damage.h"
#include "objects/scenery.h"
#include "physics/breakable_surfaces.h"
#include "structures/structure_bsp_definitions.h"
#include "physics/collisions.h"
#include "rasterizer/rasterizer_cinematics.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "sound/game_sound.h"
#include "sound/sound_manager.h"
#include "sound/sound_definitions.h"
#include "units/units.h"
#include "coop_enemies.h"
#include "coop_pickups.h"
#include "coop_vehicles.h"
#include "coop_spectate.h"
#include "coop_sound_sync.h"
#include "network_coop.h"
#include "vehicle_riding.h"
#include "network_distributed.h"
#include "../src/server_events.h" /* port: port/linux/src/server_events.c */
#include "../src/server_json.h" /* port: port/linux/src/server_json.c */
#include "../src/server_log.h" /* port: port/linux/src/server_log.c */

/* players.c's ownership helpers, absent from its original header. */
void network_player_detach_unit(long player_index);
void local_player_set_player_index(short local_player_index, long player_index);

/* ---------- constants */

/* A joining player with nobody to spectate yet watches the host's view
from this far behind and above the host's eyes, easing a fraction of the way
toward it each tick. */
#define WATCH_HOST_DISTANCE 3.0f
#define WATCH_HOST_HEIGHT 1.0f
#define WATCH_HOST_FOLLOW 0.2f

/* how often the host sends again the state a client that joined since, or
missed a message, would lack (host_resend) */
#define OBJECT_REFRESH_TICKS (2 * TICKS_PER_SECOND)

/* the screen effect a client takes from its host: the rasterizer's
convolution types (none, blur, warp) and video overbright modes
(rasterizer_xbox_screen_effect.c), and at most this many extra convolution
passes, more than any script asks for */
#define SCREEN_EFFECT_CONVOLUTION_TYPES 3
#define SCREEN_EFFECT_OVERBRIGHT_MODES 3
#define SCREEN_EFFECT_MAXIMUM_EXTRA_PASSES 8

enum
{
	/* the farthest an object the cutscene camera films can be from it
	(camera_object_get) */
	CAMERA_OBJECT_DISTANCE = 8,
	/* a client ends the cinematic after this long without hearing from the host */
	PRESENTATION_SILENCE_TICKS = 2 * TICKS_PER_SECOND,
	/* A client plays the host's presentations (its cutscene camera, fades,
	letterbox) one a tick, about this many ticks behind the newest it has,
	so they keep an even pace however unevenly the network delivers them. */
	PRESENTATION_DELAY_TICKS = 2,
	PRESENTATION_BUFFER_COUNT = 8,
	/* the field of view (radians) is sent as a word, scaled by this */
	FIELD_OF_VIEW_SCALE = 10000,

	MAXIMUM_QUEUED_EVENTS = 128,
	/* each event is sent in this many ticks' messages */
	EVENT_SENDS = 3,

	/* the size of devices.c's device group array (devices_initialize's;
	every index into the arrays below is checked against this too) */
	MAXIMUM_DEVICE_GROUPS = 1024,
	/* a changed group is sent in this many ticks' messages */
	DEVICE_GROUP_SENDS = 3,
	/* unchanged groups sent each tick, in rotation */
	DEVICE_GROUP_REFRESHES_PER_TICK = 8,

	OBJECT_NAMES_INTERVAL_TICKS = TICKS_PER_SECOND / 2,
	OBJECT_NAME_BYTES = MAXIMUM_OBJECT_NAMES_PER_SCENARIO / 8,
	/* the types network_objects.c syncs; the object names sync skips them */
	NETWORKED_OBJECT_TYPES = _object_mask_biped | _object_mask_vehicle | _object_mask_weapon | _object_mask_equipment,

	/* a player with no input for longer than this is away from the controls:
	every player's fire hurts them */
	FRIENDLY_FIRE_AWAY_TICKS = 70 * TICKS_PER_SECOND,
	/* the host forgets a client's skip vote it hasn't heard again for this long */
	SKIP_VOTE_HELD_TICKS = TICKS_PER_SECOND,
	/* After a skip the cutscene stays skippable until the script reaches
	cinematic_skip_stop; votes still in flight must not skip it again. */
	SKIP_COOLDOWN_TICKS = 2 * TICKS_PER_SECOND,

	/* a Pelican's Warthog drop: one Warthog for every this many players,
	at most this many in all */
	PLAYERS_PER_DROPPED_VEHICLE = 4,
	MAXIMUM_DROPPED_VEHICLES = 5,
};

/* (devices.c's literal count: a change there is one here) */
typedef char device_groups_count_assert[MAXIMUM_DEVICE_GROUPS == 1024 ? 1 : -1];

/* distributed_coop_event.kind */
enum
{
	_coop_event_sound,
	_coop_event_title,
	_coop_event_hud,
	_coop_event_player_effect,
	_coop_event_nav_point,
	_coop_event_unit_animation,
	_coop_event_scenery_animation,
	_coop_event_effect,
	_coop_event_attach,
	/* the host skipped the cutscene: a client stops its dialogue */
	/* the host reverted to its last saved state */
	_coop_event_reverted,
	/* a unit opened (value TRUE) or closed: a dropship's doors */
	_coop_event_unit_open,
	/* a breakable surface broke: value its index, frame its BSP, reals the
	damage's epicenter */
	_coop_event_surface_broken,
};

/* distributed_coop_event.type of an effect: at a cutscene flag (value), or on
an object's marker (value: its index in the object's model's markers) */
enum
{
	_coop_effect_at_flag,
	_coop_effect_on_marker,
};

/* distributed_coop_event.type of an attach event */
enum
{
	_coop_attach,
	_coop_detach,
};

/* which of a device's two groups an entry is */
enum
{
	_device_group_role_power,
	_device_group_role_position,
};

/* distributed_coop_presentation.flags */
enum
{
	_presentation_cinematic_bit = 0,
	_presentation_letterbox_bit,
	_presentation_fading_out_bit,
	_presentation_skippable_bit,
	_presentation_show_hud_bit,
	_presentation_show_help_text_bit,
	_presentation_timer_paused_bit,
	_presentation_timer_enabled_bit,
};

/* ---------- structures */

struct distributed_coop_presentation
{
	byte flags;
	byte fade_color[3];
	short fade_ticks;
	/* ticks since the fade began, capped at SHORT_MAX */
	short fade_elapsed;
	/* the host's game time when the fade began; a new value means a new fade */
	long fade_start_time;
	real_point3d camera_position;
	struct distributed_vector camera_forward;
	struct distributed_vector camera_up;
	word camera_field_of_view;
	/* the skip vote: machines that voted, out of how many */
	byte skip_votes;
	byte skip_voters;
	/* the scripts' HUD settings (hud_unit.c, hud_weapon.c) */
	long unit_hud_flags;
	long weapon_hud_flags;
	/* the script timer (hud_messaging.c); the clocks agree, so its
	reference time means the same on every machine */
	long timer_reference_time;
	short timer_ticks;
	short timer_flash_cutoff;
	short timer_x;
	short timer_y;
	short timer_corner;
	/* whether the scripts have the host's camera (camera_control): else a
	cutscene leaves it with the player, and a client spectates */
	byte camera_scripted;
	/* whether the scripts hold the players' controls (player_enable_input) */
	byte input_disabled;
	/* the object the camera films (camera_object_get) and the camera's
	offset from it: a client puts its camera by its own copy, which its
	timing may have a tick from the host's */
	long camera_object_index;
	real_vector3d camera_object_offset;
	/* whether the scripts' screen shake is running on the host. A client
	whose shake outlives it (the stop was skipped with a cutscene, or lost)
	ends its own. */
	byte scripted_shake;
	/* whether the scripts have set the players' maximum vitality, and to
	what (network_coop_set_players_vitality) */
	byte players_vitality_set;
	/* the cluster the scripts keep active (object_pvs_activate) plus one,
	0 for none: a cutscene filmed far from every player (b30's Elite at the
	shaft door) is active on the clients too. Plus one so the zeroed padding
	of a host without it reads as none. */
	short activating_cluster;
	real players_maximum_body_vitality;
	real players_maximum_shield_vitality;
	/* which teams are allies and friends (game_allegiance.c): the scripts
	make them, so a client's would leave the marines its enemies */
	unsigned long ally_teams[GAME_ALLEGIANCE_BITVECTOR_LONGS];
	unsigned long friendly_teams[GAME_ALLEGIANCE_BITVECTOR_LONGS];
};

struct distributed_coop_presentation_message
{
	struct distributed_message_header header;
	struct distributed_coop_presentation presentation;
};

/* Something that happens once. Which fields mean what depends on the kind:

	sound             type: _coop_sound_ kind; tag_index; object_index (the source, or NONE); reals[0]: scale;
	                  target: a looping sound's start that the host resends, one that plays once,
	                  how many milliseconds ago it started it (NONE: just now; coop_sound_sync.c)
	title             value: title index; reals[0]: delay in seconds
	hud               type: _coop_hud_ kind; value
	player_effect     type: _coop_player_effect_ kind; reals: the function's arguments
	nav_point         type: _coop_nav_point_ kind; value: nav index (NONE to deactivate);
	                  target: team or unit; object_index: cutscene flag or object; reals[0]: offset
	unit_animation    object_index: unit; tag_index: graph (NONE: stop); value: animation;
	                  frame; interpolate
	scenery_animation object_index, name_index, definition_index: the scenery; tag_index: graph;
	                  value: animation; frame */
struct distributed_coop_event
{
	byte kind;
	byte type;
	word number;
	short value;
	short frame;
	short name_index;
	byte interpolate;
	byte pad;
	long object_index;
	long target;
	long tag_index;
	long definition_index;
	real reals[3];
};

struct distributed_coop_events_message
{
	struct distributed_message_header header;
	struct distributed_coop_event events[MAXIMUM_QUEUED_EVENTS];
};

/* A device group's state. group_index is set for a scenario group. For a
device's own group it is NONE, and the device is found by name_index, or
(unnamed) by object_index and definition_index. */
struct distributed_coop_device_group
{
	short group_index;
	short name_index;
	long object_index;
	long definition_index;
	byte role;
	byte flags;
	/* counts the host's immediate sets (wraps at 256) */
	byte snaps;
	/* sent because it changed, rather than in the rotation */
	byte changed;
	real value;
};

#define MAXIMUM_DEVICE_GROUPS_PER_MESSAGE 64

struct distributed_coop_device_groups_message
{
	struct distributed_message_header header;
	struct distributed_coop_device_group groups[MAXIMUM_DEVICE_GROUPS];
};

/* one bit per scenario object name: whether the host has that object */
struct distributed_coop_object_names
{
	short structure_bsp_index;
	short name_count;
	byte present[OBJECT_NAME_BYTES];
};

struct distributed_coop_object_names_message
{
	struct distributed_message_header header;
	struct distributed_coop_object_names names;
};

/* Where one of the host's scenery or machines is. Scripts move them (a
cutscene's drop pod) and nothing else tells a client. Found as the
scenery animations' are (object_find). */
struct distributed_coop_object_transform
{
	short name_index;
	short pad;
	long object_index;
	long definition_index;
	real_point3d position;
	struct distributed_vector forward;
	struct distributed_vector up;
};

#define MAXIMUM_OBJECT_TRANSFORMS_PER_MESSAGE 64
#define COOP_MOVED_OBJECTS (_object_mask_scenery | _object_mask_machine)

struct distributed_coop_object_transforms_message
{
	struct distributed_message_header header;
	struct distributed_coop_object_transform transforms[MAXIMUM_OBJECT_TRANSFORMS_PER_MESSAGE];
};

/* How one of the host's objects looks: the permutation of each region of
its model (NONE for none) and its scale. The scripts change them (a
helmet off, a ship scaled) and so does damage. */
struct distributed_coop_object_look
{
	short name_index;
	short pad;
	long object_index;
	long definition_index;
	byte region_permutations[MAXIMUM_REGIONS_PER_OBJECT];
	real scale;
};

#define MAXIMUM_OBJECT_LOOKS_PER_MESSAGE 64

struct distributed_coop_object_looks_message
{
	struct distributed_message_header header;
	struct distributed_coop_object_look looks[MAXIMUM_OBJECT_LOOKS_PER_MESSAGE];
};

/* a client's vote to skip the cinematic, sent every tick while it stands */
struct distributed_coop_skip_vote
{
	byte voted;
	byte pad[3];
};

struct distributed_coop_skip_vote_message
{
	struct distributed_message_header header;
	struct distributed_coop_skip_vote vote;
};

/* ---------- globals */

/* client: the cinematic it started to match the host's */
static struct
{
	boolean cinematic_started;
	/* looking through the host's camera: its scripted cutscene camera, or
	its view for a player with nothing else to look at */
	boolean host_camera;
	/* the presentations in a row that wanted that camera and had one that
	never moved (client_host_camera_unmoved: logged after a second, as a
	dedicated server of this build's moves it within a tick of a map's first
	cutscene), and the view watched in its place (client_watch_cutscene) */
	short host_camera_unmoved_ticks;
	struct dead_camera unmoved_camera;
	/* the unit the host's scripts last started a custom animation on in this
	cinematic, as a cutscene's actors are animated: the one that view watches
	(NONE outside a cinematic) */
	long cutscene_subject;
	/* the eased view behind the host, used while there is nothing else to
	watch (watching_host_valid is FALSE until it is first set) */
	boolean watching_host_valid;
	real_point3d watching_host_position;
	real_vector3d watching_host_forward;
	long heard_time;
	long fade_start_time;
	/* the host's presentations as they arrived, by the host's tick; the
	newest tick heard and the tick last played (nothing is played until
	one has been heard) */
	struct distributed_coop_presentation buffered[PRESENTATION_BUFFER_COUNT];
	long buffered_times[PRESENTATION_BUFFER_COUNT];
	boolean heard_any;
	boolean playing;
	long newest_time;
	long played_time;
} coop_presentation;

/* host: events still to be sent; client: the last one applied */
static struct
{
	struct distributed_coop_event events[MAXIMUM_QUEUED_EVENTS];
	short sends[MAXIMUM_QUEUED_EVENTS];
	short count;
	word next_number;
	word applied_number;
	boolean applied_any;
} coop_events;

/* host: each device group as last sent, indexed by group */
static struct
{
	/* false until the first tick, which records the map's starting state */
	boolean started;
	real values[MAXIMUM_DEVICE_GROUPS];
	byte flags[MAXIMUM_DEVICE_GROUPS];
	byte snaps[MAXIMUM_DEVICE_GROUPS];
	/* ticks this group is still to be sent as a change */
	byte sends[MAXIMUM_DEVICE_GROUPS];
	/* immediate sets so far (devices.c calls network_coop_note_device_snap) */
	byte snap_counts[MAXIMUM_DEVICE_GROUPS];
	/* whether this group has changed since the map loaded */
	boolean moved[MAXIMUM_DEVICE_GROUPS];
	short refresh_next, moved_refresh_next;
	struct distributed_coop_device_groups_message message;
} host_devices;

/* The players' maximum vitality, once the scripts have set a player's: on
the host as they set it, on a client as the host's presentation has it.
Every player's unit is kept at it, a new one included. */
static struct
{
	boolean set;
	real maximum_body;
	real maximum_shield;
} players_vitality;

/* host: the client machines it had last tick. On the tick a new one
appears, and every OBJECT_REFRESH_TICKS, the state a client that joined
since would lack is sent again (refresh), and on the join every device that
has moved since the map loaded too (joined). */
static struct
{
	long machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short machine_count;
	boolean joined;
	boolean refresh;
} host_resend;

/* Checkpoints restore player_data, but not the live machines' player
table. A team wipe may rewind only while both still name the saved team. */
struct coop_checkpoint_players
{
	long players[MAXIMUM_TRACKED_PLAYERS];
	long machine_players[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_LOCAL_PLAYERS];
	short count;
	boolean valid;
};

static struct coop_checkpoint_players checkpoint_players;

/* client: the host's snap count last seen for each of its groups */
static struct
{
	byte snaps[MAXIMUM_DEVICE_GROUPS];
	boolean seen[MAXIMUM_DEVICE_GROUPS];
} client_devices;

/* client: the script sounds (impulse ones: dialogue) it played lately,
which a skipped cutscene stops */
enum
{
	CLIENT_SCRIPT_SOUND_COUNT = 16,
};

static struct
{
	long definition_indices[CLIENT_SCRIPT_SOUND_COUNT];
	short next;
} client_script_sounds;

/* client: the object names that differed from the host's last time */
static byte client_names_differing[OBJECT_NAME_BYTES];

/* The cutscene skip vote. The host records when it last heard from each
client that is showing the cutscene, and when that client last voted. A
client keeps the host's latest tally for display. */
static struct
{
	boolean voted;
	boolean offered;
	short votes;
	short voters;
	long client_heard_times[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	long client_vote_times[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	/* host: no voting before this game time (set after a skip) */
	long cooldown_until;
	/* host: the vote passed and main_skip_cinematic was called once */
	boolean requested;
	/* host: the save the script makes as the cutscene becomes skippable is
	written (a skip reverts to it) */
	boolean skip_save_written;
} skip_vote;

/* ---------- prototypes */

static void client_play_presentation(void);
/* unit_scripting_commands.c */
void unit_scripting_set_current_vitality_of(long unit_index, real body_vitality, real shield_vitality);
/* port_config.c */
int config_boolean(char const *name);
long config_integer(char const *name);
char const *config_string(char const *name);
int halo_server_mode(void);
/* p2p_lobby.c: chosen visibility, independent of listing transport state */
int p2p_hosting_listed(void);

/* ---------- private code */

static boolean coop_game(
	void)
{
	return network_coop_active();
}

static boolean coop_host(
	void)
{
	return game_connection() == _game_connection_network_server && coop_game();
}

static boolean coop_client(
	void)
{
	return game_connection() == _game_connection_network_client && coop_game();
}

/* host: sends count entries of entry_size in message to every client */
static void send_to_clients(
	void *message,
	byte type,
	short count,
	word entry_size)
{
	distributed_send(message, type, count, (word)(sizeof(struct distributed_message_header) + count * entry_size),
		_distributed_to_clients);
}

/* host, each tick: whether a client machine joined since the last, and so
whether to resend this tick (host_resend) */
static void host_resend_update(
	void)
{
	long machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short count = distributed_client_machines(machines, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	short index, known;

	host_resend.joined = FALSE;
	for (index = 0; index < count && !host_resend.joined; index++)
	{
		for (known = 0; known < host_resend.machine_count && host_resend.machines[known] != machines[index]; known++)
			;
		host_resend.joined = known == host_resend.machine_count;
	}
	csmemcpy(host_resend.machines, machines, count * sizeof(machines[0]));
	host_resend.machine_count = count;
	host_resend.refresh = host_resend.joined || game_time_get() % OBJECT_REFRESH_TICKS == 0;
	if (host_resend.joined)
		error(_error_silent, "co-op: a machine joined; resending the game's state");
}

/* whether the object is a player's unit or carries one (scripts never delete those) */
static boolean object_holds_player(
	long object_index)
{
	long child_index;

	if (player_index_from_unit_index(object_index) != NONE)
		return TRUE;
	for (child_index = object_get(object_index)->object.first_child_object_index; child_index != NONE;
		child_index = object_get(child_index)->object.next_object_index)
	{
		if (object_holds_player(child_index))
			return TRUE;
	}

	return FALSE;
}

/* the object type a scenario object name refers to, or NONE */
static short object_name_type(
	short name_index)
{
	struct scenario *scenario = global_scenario_get();

	if (name_index < 0 || name_index >= scenario->object_names.count)
		return NONE;

	return TAG_BLOCK_GET_ELEMENT(&scenario->object_names, name_index, struct scenario_object_name)->runtime_object_type;
}

/* Client: finds the local copy of an object the host named, of one of the
types in type_mask, and returns its index (or NONE). An unnamed object sits
at the same object index on every machine; if its salt differs here,
matching the absolute index is enough, as long as the tag matches too. */
static long object_find(
	short name_index,
	long object_index,
	long definition_index,
	unsigned long type_mask)
{
	struct object_datum *object;

	if (name_index != NONE)
		object_index = object_index_from_name_index(name_index);
	if (object_index == NONE)
		return NONE;
	object = object_try_and_get_and_verify_type(object_index, type_mask);
	if (!object && name_index == NONE)
	{
		struct object_iterator iterator;
		struct object_datum *candidate;

		object_iterator_new(&iterator, type_mask, 0);
		while (!object && (candidate = object_iterator_next(&iterator)) != NULL)
		{
			if ((iterator.index & 0xFFFF) == (object_index & 0xFFFF))
			{
				object = candidate;
				object_index = iterator.index;
			}
		}
	}
	if (!object || object->definition_index != definition_index ||
		(name_index == NONE && object->object.name_index != NONE))
	{
		return NONE;
	}

	return object_index;
}

/* host: a new event, filled in by the caller; NULL if it isn't wanted or the queue is full */
static struct distributed_coop_event *event_new(
	byte kind)
{
	struct distributed_coop_event *event;

	if (!coop_host())
		return NULL;
	if (coop_events.count == MAXIMUM_QUEUED_EVENTS)
	{
		error(_error_silent, "co-op: event queue full; an event of kind %d is not sent", kind);
		return NULL;
	}
	event = &coop_events.events[coop_events.count];
	csmemset(event, 0, sizeof(*event));
	event->kind = kind;
	event->number = ++coop_events.next_number;
	event->object_index = NONE;
	event->target = NONE;
	event->tag_index = NONE;
	event->definition_index = NONE;
	event->name_index = NONE;
	coop_events.sends[coop_events.count] = 0;
	coop_events.count++;

	return event;
}

static void host_send_events(
	void)
{
	struct distributed_coop_events_message message;
	short index;

	if (!coop_events.count)
		return;
	csmemcpy(message.events, coop_events.events, coop_events.count * sizeof(message.events[0]));
	send_to_clients(&message, _distributed_message_coop_events, coop_events.count, sizeof(message.events[0]));
	/* drop the ones sent EVENT_SENDS times, keeping the rest in order */
	for (index = 0; index < coop_events.count; )
	{
		if (++coop_events.sends[index] < EVENT_SENDS)
		{
			index++;
			continue;
		}
		coop_events.count--;
		csmemmove(&coop_events.events[index], &coop_events.events[index + 1],
			(coop_events.count - index) * sizeof(coop_events.events[0]));
		csmemmove(&coop_events.sends[index], &coop_events.sends[index + 1],
			(coop_events.count - index) * sizeof(coop_events.sends[0]));
	}
}

/* Fills in the group's value, flags and snap count. Returns FALSE if the
group doesn't exist. */
static boolean device_group_state(
	short group_index,
	struct distributed_coop_device_group *entry)
{
	word flags;
	boolean runtime;

	if (group_index < 0 || group_index >= MAXIMUM_DEVICE_GROUPS ||
		!device_group_network_get(group_index, &entry->value, &flags, &runtime))
	{
		return FALSE;
	}
	entry->flags = (byte)flags;
	entry->snaps = host_devices.snap_counts[group_index];
	entry->changed = FALSE;

	return TRUE;
}

/* Host: lists every device group, scenario groups first, then each device's
own groups. group_indices gets each entry's group index. Returns the count. */
static short device_group_entries(
	struct distributed_coop_device_group *entries,
	short *group_indices)
{
	struct scenario *scenario = global_scenario_get();
	struct object_iterator iterator;
	struct device_datum *device;
	short count = 0, group_index;

	for (group_index = 0; group_index < scenario->device_groups.count && count < MAXIMUM_DEVICE_GROUPS; group_index++)
	{
		struct distributed_coop_device_group *entry = &entries[count];

		entry->group_index = group_index;
		entry->name_index = NONE;
		entry->object_index = NONE;
		entry->definition_index = NONE;
		entry->role = _device_group_role_power;
		if (device_group_state(group_index, entry))
			group_indices[count++] = group_index;
	}
	object_iterator_new(&iterator, _object_mask_device, 0);
	while ((device = object_iterator_next(&iterator)) != NULL)
	{
		short groups[2];
		short role;

		groups[_device_group_role_power] = device->device.power_group_index;
		groups[_device_group_role_position] = device->device.position_group_index;
		for (role = 0; role < NUMBEROF(groups) && count < MAXIMUM_DEVICE_GROUPS; role++)
		{
			struct distributed_coop_device_group *entry = &entries[count];
			real value;
			word flags;
			boolean runtime;

			/* scenario groups were listed above */
			if (!device_group_network_get(groups[role], &value, &flags, &runtime) || !runtime)
				continue;
			entry->group_index = NONE;
			entry->name_index = device->object.name_index;
			entry->object_index = iterator.index;
			entry->definition_index = device->definition_index;
			entry->role = (byte)role;
			if (device_group_state(groups[role], entry))
				group_indices[count++] = groups[role];
		}
	}

	return count;
}

/* client: the local group a host entry refers to, or NONE */
static short device_group_find(
	struct distributed_coop_device_group const *entry)
{
	struct device_datum *device;
	long device_index;
	real value;
	word flags;
	boolean runtime;
	short group_index;

	if (entry->group_index != NONE)
	{
		group_index = entry->group_index;
		return group_index >= 0 && group_index < global_scenario_get()->device_groups.count &&
			group_index < MAXIMUM_DEVICE_GROUPS &&
			device_group_network_get(group_index, &value, &flags, &runtime) && !runtime ? group_index : NONE;
	}
	device_index = object_find(entry->name_index, entry->object_index, entry->definition_index, _object_mask_device);
	if (device_index == NONE)
		return NONE;
	device = object_get_and_verify_type(device_index, _object_mask_device);
	switch (entry->role)
	{
	case _device_group_role_power: group_index = device->device.power_group_index; break;
	case _device_group_role_position: group_index = device->device.position_group_index; break;
	default: return NONE;
	}

	return group_index >= 0 && group_index < MAXIMUM_DEVICE_GROUPS &&
		device_group_network_get(group_index, &value, &flags, &runtime) && runtime ? group_index : NONE;
}

/* host: sends the groups that changed, plus the next few in each rotation:
one through every group, and a quicker one through the groups that have
moved since the map loaded. A late joiner loaded the same map, so those are
the ones it lacks, and a level's few moved devices (a light bridge) reach it
within a tick or two instead of after a pass through hundreds. */
static void host_send_device_groups(
	void)
{
	static short group_indices[MAXIMUM_DEVICE_GROUPS];
	struct distributed_coop_device_group *entries = host_devices.message.groups;
	short count = device_group_entries(entries, group_indices);
	short moved_count = 0, moved_index = 0;
	short index, sent = 0;

	for (index = 0; index < count; index++)
	{
		struct distributed_coop_device_group const *entry = &entries[index];
		short group_index = group_indices[index];

		if (!host_devices.started ||
			entry->value != host_devices.values[group_index] ||
			entry->flags != host_devices.flags[group_index] ||
			entry->snaps != host_devices.snaps[group_index])
		{
			host_devices.values[group_index] = entry->value;
			host_devices.flags[group_index] = entry->flags;
			host_devices.snaps[group_index] = entry->snaps;
			/* clients loaded the same map, so the starting state isn't sent */
			if (host_devices.started)
			{
				host_devices.sends[group_index] = DEVICE_GROUP_SENDS;
				host_devices.moved[group_index] = TRUE;
			}
		}
		if (host_devices.moved[group_index])
			moved_count++;
	}
	if (host_devices.refresh_next >= count)
		host_devices.refresh_next = 0;
	if (host_devices.moved_refresh_next >= moved_count)
		host_devices.moved_refresh_next = 0;
	for (index = 0; index < count; index++)
	{
		struct distributed_coop_device_group *entry = &entries[index];
		short group_index = group_indices[index];
		boolean refresh = (index - host_devices.refresh_next + count) % count < DEVICE_GROUP_REFRESHES_PER_TICK;

		if (host_devices.moved[group_index])
		{
			refresh = refresh || host_resend.joined ||
				(moved_index - host_devices.moved_refresh_next + moved_count) % moved_count < DEVICE_GROUP_REFRESHES_PER_TICK;
			moved_index++;
		}
		entry->changed = host_devices.sends[group_index] != 0;
		if (entry->changed)
			host_devices.sends[group_index]--;
		else if (!refresh)
			continue;
		entries[sent++] = *entry;
	}
	host_devices.started = TRUE;
	host_devices.refresh_next = count ? (short)((host_devices.refresh_next + DEVICE_GROUP_REFRESHES_PER_TICK) % count) : 0;
	host_devices.moved_refresh_next = moved_count ?
		(short)((host_devices.moved_refresh_next + DEVICE_GROUP_REFRESHES_PER_TICK) % moved_count) : 0;
	/* (in messages of at most MAXIMUM_DEVICE_GROUPS_PER_MESSAGE, since a
	message counts its entries in a byte: each moved to the front, where the
	message's entries begin, over those already sent) */
	for (index = 0; index < sent; index += MAXIMUM_DEVICE_GROUPS_PER_MESSAGE)
	{
		short message_count = (short)MIN(sent - index, MAXIMUM_DEVICE_GROUPS_PER_MESSAGE);

		if (index > 0)
			csmemcpy(entries, &entries[index], message_count * sizeof(entries[0]));
		send_to_clients(&host_devices.message, _distributed_message_coop_device_groups, message_count,
			sizeof(entries[0]));
	}
}

/* Where one of the host's devices is: its position (open, closed, how far
up an elevator has gone) and power, which the device groups' values only
aim it at. Found as the scenery animations' are (object_find). */
struct distributed_coop_device_state
{
	short name_index;
	byte moving;
	byte pad;
	long object_index;
	long definition_index;
	real position;
	real power;
};

#define MAXIMUM_DEVICE_STATES_PER_MESSAGE 64

struct distributed_coop_device_states_message
{
	struct distributed_message_header header;
	struct distributed_coop_device_state states[MAXIMUM_DEVICE_STATES_PER_MESSAGE];
};

/* a client puts its device where the host's is when the host's is at rest
there, or when the two are this far apart while it moves (of the device's
travel, 0 to 1) */
#define DEVICE_POSITION_TOLERANCE 0.03f

/* host: each device's position and power as last sent, by its absolute
index, and whether they have changed since the map placed it */
static struct
{
	long object_index;
	boolean moved;
	real position;
	real power;
} host_sent_devices[MAXIMUM_OBJECTS_PER_MAP];

/* host: the structure BSP each client player's machine last said it has,
NONE before its first input */
static short host_player_structure_bsps[MAXIMUM_TRACKED_PLAYERS];

/* host: each player's friendly fire, by the player's absolute index: whether
they chose to play without it, the buttons of their last input, and the ticks
since an input showed them at the controls */
static struct friendly_fire_player
{
	long player_index;
	boolean declined;
	unsigned long control_flags;
	long idle_ticks;
} host_friendly_fire[MAXIMUM_TRACKED_PLAYERS];

/* host, each tick: the devices that moved or changed power (and, with the
resent state, all that ever have) */
static void host_send_device_states(
	void)
{
	struct distributed_coop_device_states_message message;
	struct object_iterator iterator;
	struct device_datum *device;
	short count = 0;

	object_iterator_new(&iterator, _object_mask_device, 0);
	while ((device = object_iterator_next(&iterator)) != NULL)
	{
		short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		struct distributed_coop_device_state *state;
		boolean changed;

		if (absolute_index < 0 || absolute_index >= MAXIMUM_OBJECTS_PER_MAP)
			continue;
		if (host_sent_devices[absolute_index].object_index != iterator.index)
		{
			/* (as the map placed it, on every machine) */
			host_sent_devices[absolute_index].object_index = iterator.index;
			host_sent_devices[absolute_index].moved = FALSE;
			host_sent_devices[absolute_index].position = device->device.position;
			host_sent_devices[absolute_index].power = device->device.power;
			continue;
		}
		changed = host_sent_devices[absolute_index].position != device->device.position ||
			host_sent_devices[absolute_index].power != device->device.power;
		if (!changed && !(host_resend.refresh && host_sent_devices[absolute_index].moved))
			continue;
		host_sent_devices[absolute_index].moved = TRUE;
		host_sent_devices[absolute_index].position = device->device.position;
		host_sent_devices[absolute_index].power = device->device.power;
		state = &message.states[count++];
		state->name_index = device->object.name_index;
		state->moving = (byte)(device->device.position_velocity != 0.0f);
		state->pad = 0;
		state->object_index = iterator.index;
		state->definition_index = device->definition_index;
		state->position = device->device.position;
		state->power = device->device.power;
		if (count == MAXIMUM_DEVICE_STATES_PER_MESSAGE)
		{
			send_to_clients(&message, _distributed_message_coop_device_states, count, sizeof(message.states[0]));
			count = 0;
		}
	}
	if (count > 0)
		send_to_clients(&message, _distributed_message_coop_device_states, count, sizeof(message.states[0]));
}

/* host: where each scenery or machine was last sent, by its absolute
index, and whether it has moved since the map placed it */
static struct
{
	long object_index;
	boolean moved;
	real_point3d position;
	real_vector3d forward;
} host_sent_transforms[MAXIMUM_OBJECTS_PER_MAP];

/* host, each tick: the scenery and machines that moved (and, every
OBJECT_REFRESH_TICKS, all that ever have) */
static void host_send_object_transforms(
	void)
{
	struct distributed_coop_object_transforms_message message;
	struct object_iterator iterator;
	struct object_datum *object;
	short count = 0;

	object_iterator_new(&iterator, COOP_MOVED_OBJECTS, 0);
	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		struct distributed_coop_object_transform *transform;
		boolean moving;

		if (object->object.parent_object_index != NONE || absolute_index < 0 || absolute_index >= MAXIMUM_OBJECTS_PER_MAP)
			continue;
		if (host_sent_transforms[absolute_index].object_index != iterator.index)
		{
			/* an object seen for the first time is where the map placed it, on
			every machine, so there is nothing to send yet */
			host_sent_transforms[absolute_index].object_index = iterator.index;
			host_sent_transforms[absolute_index].moved = FALSE;
			host_sent_transforms[absolute_index].position = object->object.position;
			host_sent_transforms[absolute_index].forward = object->object.forward;
			continue;
		}
		moving = distance_squared3d(&host_sent_transforms[absolute_index].position, &object->object.position) >= 0.0001f ||
			dot_product3d(&host_sent_transforms[absolute_index].forward, &object->object.forward) <= 0.9999f;
		if (!moving && !(host_resend.refresh && host_sent_transforms[absolute_index].moved))
			continue;
		host_sent_transforms[absolute_index].moved = TRUE;
		host_sent_transforms[absolute_index].position = object->object.position;
		host_sent_transforms[absolute_index].forward = object->object.forward;
		transform = &message.transforms[count++];
		transform->name_index = object->object.name_index;
		transform->pad = 0;
		transform->object_index = iterator.index;
		transform->definition_index = object->definition_index;
		transform->position = object->object.position;
		distributed_vector_pack(&object->object.forward, DISTRIBUTED_UNIT_SCALE, &transform->forward);
		distributed_vector_pack(&object->object.up, DISTRIBUTED_UNIT_SCALE, &transform->up);
		if (count == MAXIMUM_OBJECT_TRANSFORMS_PER_MESSAGE)
		{
			send_to_clients(&message, _distributed_message_coop_object_transforms, count, sizeof(message.transforms[0]));
			count = 0;
		}
	}
	if (count > 0)
		send_to_clients(&message, _distributed_message_coop_object_transforms, count, sizeof(message.transforms[0]));
}

/* host: how each object looked when last sent, by its absolute index, and
whether that has changed since it was made */
static struct
{
	long object_index;
	boolean changed;
	byte region_permutations[MAXIMUM_REGIONS_PER_OBJECT];
	real scale;
} host_sent_looks[MAXIMUM_OBJECTS_PER_MAP];

/* host, each tick: the objects whose looks changed (and, every
OBJECT_REFRESH_TICKS, all whose looks ever have) */
static void host_send_object_looks(
	void)
{
	struct distributed_coop_object_looks_message message;
	struct object_iterator iterator;
	struct object_datum *object;
	short count = 0;

	object_iterator_new(&iterator, _object_mask_all, 0);
	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		short absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.index);
		struct distributed_coop_object_look *look;
		boolean changing;

		if (absolute_index < 0 || absolute_index >= MAXIMUM_OBJECTS_PER_MAP)
			continue;
		if (host_sent_looks[absolute_index].object_index != iterator.index)
		{
			/* an object seen for the first time looks the same on every machine */
			host_sent_looks[absolute_index].object_index = iterator.index;
			host_sent_looks[absolute_index].changed = FALSE;
			host_sent_looks[absolute_index].scale = object->object.scale;
			csmemcpy(host_sent_looks[absolute_index].region_permutations, object->object.region_permutations,
				sizeof(object->object.region_permutations));
			continue;
		}
		changing = host_sent_looks[absolute_index].scale != object->object.scale ||
			csmemcmp(host_sent_looks[absolute_index].region_permutations, object->object.region_permutations,
				sizeof(object->object.region_permutations));
		if (!changing && !(host_resend.refresh && host_sent_looks[absolute_index].changed))
			continue;
		host_sent_looks[absolute_index].changed = TRUE;
		host_sent_looks[absolute_index].scale = object->object.scale;
		csmemcpy(host_sent_looks[absolute_index].region_permutations, object->object.region_permutations,
			sizeof(object->object.region_permutations));
		look = &message.looks[count++];
		look->name_index = object->object.name_index;
		look->pad = 0;
		look->object_index = iterator.index;
		look->definition_index = object->definition_index;
		csmemcpy(look->region_permutations, object->object.region_permutations, sizeof(look->region_permutations));
		look->scale = object->object.scale;
		if (count == MAXIMUM_OBJECT_LOOKS_PER_MESSAGE)
		{
			send_to_clients(&message, _distributed_message_coop_object_looks, count, sizeof(message.looks[0]));
			count = 0;
		}
	}
	if (count > 0)
		send_to_clients(&message, _distributed_message_coop_object_looks, count, sizeof(message.looks[0]));
}

/* ---------- the cinematic screen effect

The scripts' full-screen effects (cinematic_screen_effect_*: blur, warp,
light enhancement, desaturation, the video effect) are state the host's
scripts set and a client's never do. The host sends it when it changes and
every OBJECT_REFRESH_TICKS, for a client that joined since or missed it. */

struct distributed_coop_screen_effect_message
{
	struct distributed_message_header header;
	struct rasterizer_screen_effect_port_state state;
};

/* host: the state last sent */
static struct rasterizer_screen_effect_port_state host_sent_screen_effect;

static void host_send_screen_effect(
	void)
{
	struct distributed_coop_screen_effect_message message;

	rasterizer_screen_effect_port_get(&message.state);
	if (!host_resend.refresh && !csmemcmp(&message.state, &host_sent_screen_effect, sizeof(message.state)))
		return;
	host_sent_screen_effect = message.state;
	send_to_clients(&message, _distributed_message_coop_screen_effect, 1, sizeof(message.state));
}

word network_coop_screen_effect_entry_size(
	void)
{
	return sizeof(struct rasterizer_screen_effect_port_state);
}

void network_coop_handle_screen_effect(
	void const *entries,
	short count)
{
	struct rasterizer_screen_effect_port_state state;
	/* every field from the tint onward is a real (rasterizer_cinematics.h) */
	real const *reals = &state.filter_desaturation_tint.red;
	short real_count = (short)((sizeof(state) -
		offsetof(struct rasterizer_screen_effect_port_state, filter_desaturation_tint)) / sizeof(real));
	short index;

	if (!coop_client() || count != 1)
		return;
	csmemcpy(&state, entries, sizeof(state));
	for (index = 0; index < real_count; index++)
	{
		if (!distributed_real_valid(reals[index]))
			return;
	}
	/* (what the rasterizer can draw: its convolution types and overbright
	modes, a script's few extra passes, and video with no convolution, as
	rasterizer_screen_effect_set_video leaves it) */
	state.convolution_type = (short)PIN(state.convolution_type, 0, SCREEN_EFFECT_CONVOLUTION_TYPES - 1);
	state.convolution_extra_passes = (short)PIN(state.convolution_extra_passes, 0, SCREEN_EFFECT_MAXIMUM_EXTRA_PASSES);
	state.video_overbright_mode = (short)PIN(state.video_overbright_mode, 0, SCREEN_EFFECT_OVERBRIGHT_MODES - 1);
	if (state.video_on)
	{
		state.convolution_type = 0;
		state.convolution_extra_passes = 0;
		state.convolution_radius[0] = state.convolution_radius[1] = 0.0f;
	}
	/* the game can only blur one window, so split screen goes without */
	if (main_get_window_count() > 1)
	{
		state.convolution_type = 0;
		state.convolution_radius[0] = state.convolution_radius[1] = 0.0f;
	}
	rasterizer_screen_effect_port_set(&state);
}

/* host: what its scripts have attached, sent again every
OBJECT_REFRESH_TICKS for a client that joined since */
enum
{
	MAXIMUM_ATTACHMENTS = 32,
};

static struct
{
	long parent_index;
	long child_index;
	short parent_marker_index;
	short child_marker_index;
} host_attachments[MAXIMUM_ATTACHMENTS];
static short host_attachment_count;

/* host: the HUD state the scripts set that a late joiner would lack: the
objective, and the nav points active (their kind, nav index, target, marker
and offset as the note was given), resent by host_send_hud_state */
enum
{
	MAXIMUM_NAV_POINTS = 16,
};

static struct
{
	short objective;
	short nav_point_count;
	struct
	{
		short kind;
		short nav_index;
		long target;
		long marker;
		real vertical_offset;
	} nav_points[MAXIMUM_NAV_POINTS];
} host_hud_state;

/* host: the looping sounds the scripts have started (music, ambience).
They are resent every OBJECT_REFRESH_TICKS so a late joiner hears them.
One that plays once (a cutscene's music or dialogue: a fake impulse one)
ends when the sound system has played it, which a host that draws nothing
(a dedicated server: debug_no_drawing) never does: there it is ended once
it has had the time to play (length, the most its sounds can take, and
LOOPING_SOUND_END_MARGIN_TICKS), as the scripts' stop would end it, and is
not resent in that margin's time before then, when a client's copy may
have ended and a client of an earlier build would start it again. */
enum
{
	MAXIMUM_LOOPING_SOUNDS = 32,
	LOOPING_SOUND_END_MARGIN_TICKS = 2 * TICKS_PER_SECOND,
	/* (a sound's time, its linked permutations together, taken as an hour at most) */
	MAXIMUM_LOOPING_SOUND_PART_MILLISECONDS = 60 * 60 * 1000,
};

static struct
{
	long definition_index;
	long object_index;
	real scale;
	long start_time;
	/* (ticks; 0: it is never ended here) */
	long length;
} host_looping_sounds[MAXIMUM_LOOPING_SOUNDS];
static short host_looping_sound_count;

/* client: the looping sounds that play once (a cutscene's music or
dialogue: fake impulse ones) the host's events started here. One that is no
longer playing here has played out, but the host's copy can play on longer
(a dedicated server, which renders no sound, never finishes it), and its
resends would start it again from the beginning, over and over. The host's
stop forgets it, so the scripts can start it again. */
static struct
{
	long definition_indices[MAXIMUM_LOOPING_SOUNDS];
	short count;
} client_looping_sounds;

static void host_send_object_names(
	void)
{
	struct distributed_coop_object_names_message message;
	struct distributed_coop_object_names *names = &message.names;
	short name_index;

	/* Expanded MCC maps carry all names in their dedicated messages. */
	if (mcc_object_names_active()) return;
	csmemset(names, 0, sizeof(*names));
	names->structure_bsp_index = global_structure_bsp_index_get();
	names->name_count = (short)MIN(global_scenario_get()->object_names.count, MAXIMUM_OBJECT_NAMES_PER_SCENARIO);
	for (name_index = 0; name_index < names->name_count; name_index++)
	{
		if (object_index_from_name_index(name_index) != NONE)
			names->present[name_index / 8] |= (byte)(1 << (name_index % 8));
	}
	send_to_clients(&message, _distributed_message_coop_object_names, 1, sizeof(message.names));
}

static void skip_vote_clear(
	void)
{
	short index;

	skip_vote.voted = FALSE;
	skip_vote.votes = 0;
	for (index = 0; index < NUMBEROF(skip_vote.client_vote_times); index++)
	{
		skip_vote.client_heard_times[index] = NONE;
		skip_vote.client_vote_times[index] = NONE;
	}
}

/* host: counts the votes to skip the cinematic, and skips when more than
half the machines showing it have voted (one still loading, or joined since
it began, has no say until it shows it) */
static void host_count_skip_votes(
	void)
{
	long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short machine_count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	long now = game_time_get();
	boolean skippable = cinematic_in_progress() && cinematic_can_be_skipped();
	boolean was_offered = skip_vote.offered;
	short index;

	/* Wait until the save the script makes when the cutscene becomes
	skippable has been written; reverting earlier would go back to the save
	before it. A save requested later in the cutscene doesn't hold up the
	vote, since the skip cancels it, as in a solo game. */
	if (!skippable)
		skip_vote.skip_save_written = FALSE;
	else if (!main_saving_map())
		skip_vote.skip_save_written = TRUE;
	skip_vote.offered = skippable && skip_vote.skip_save_written && now >= skip_vote.cooldown_until;
	if (skip_vote.offered && !was_offered)
		error(_error_silent, "co-op: the cutscene can be skipped");
	if (!skip_vote.offered)
	{
		skip_vote_clear();
		skip_vote.voters = 0;
		skip_vote.requested = FALSE;
		return;
	}
	/* already requested: main.c skips at the end of this frame */
	if (skip_vote.requested)
		return;
	/* the host votes too, unless it is a dedicated server with no player */
	skip_vote.voters = local_player_get_next(NONE) != NONE ? 1 : 0;
	skip_vote.votes = skip_vote.voted ? 1 : 0;
	for (index = 0; index < machine_count; index++)
	{
		long heard_time = skip_vote.client_heard_times[machine_indices[index]];
		long vote_time = skip_vote.client_vote_times[machine_indices[index]];

		if (heard_time == NONE || now - heard_time > SKIP_VOTE_HELD_TICKS)
			continue;
		skip_vote.voters++;
		if (vote_time != NONE && now - vote_time <= SKIP_VOTE_HELD_TICKS)
			skip_vote.votes++;
	}
	if (skip_vote.votes * 2 > skip_vote.voters)
	{
		error(_error_silent, "co-op: skipping the cutscene (%d of %d voted)", skip_vote.votes, skip_vote.voters);
		skip_vote.requested = TRUE;
		main_skip_cinematic();
	}
}

/* the least speed (squared, world units a tick) of an object the cutscene
camera films when the scripts don't name it */
#define CAMERA_OBJECT_SPEED 0.01f
/* the camera an observer accepts (observer.c) */
#define CAMERA_WORLD_BOUND 5000.0f
#define CAMERA_MINIMUM_FIELD_OF_VIEW 0.001f
#define CAMERA_MAXIMUM_FIELD_OF_VIEW (_pi / 2.0f)

/* Host: the object the cutscene camera films: the nearest vehicle moving
close to it (a camera riding along with a drop pod or a Pelican), else the
one the scripts set it relative to; NONE for a camera on its own. The last
one is kept while it stays close, so the camera doesn't jump between two. */
static long camera_object_get(
	struct observer_result const *camera)
{
	static long filmed = NONE;
	real nearest = CAMERA_OBJECT_DISTANCE * CAMERA_OBJECT_DISTANCE;
	struct object_iterator iterator;
	struct object_datum *object;

	object = object_try_and_get_and_verify_type(filmed, _object_mask_vehicle);
	if (object && distance_squared3d(&camera->position, &object->object.position) < nearest)
		return filmed;
	filmed = NONE;
	object_iterator_new(&iterator, _object_mask_vehicle, 0);
	while ((object = object_iterator_next(&iterator)) != NULL)
	{
		real distance = distance_squared3d(&camera->position, &object->object.position);

		if (distance < nearest && object->object.parent_object_index == NONE &&
			magnitude_squared3d(&object->object.translational_velocity) > CAMERA_OBJECT_SPEED)
		{
			nearest = distance;
			filmed = iterator.index;
		}
	}
	if (filmed != NONE)
		return filmed;
	return object_try_and_get(scripted_camera_object_relative_to()) ? scripted_camera_object_relative_to() : NONE;
}

/* host: the presentation sent to every client this tick */
static void host_presentation(
	struct distributed_coop_presentation *presentation)
{
	/* the first local player's camera (director.c's), or, on a host with no
	player of its own (a dedicated server), the first observer's while it
	films the scripts' camera (director_unattended_scripted_camera): else
	none */
	short observer_index = local_player_get_next(NONE);
	struct observer_result const *camera;
	struct hud_timer_state timer;
	real_rgb_color fade_color;
	boolean fading_out;
	long elapsed;

	if (observer_index == NONE && director_unattended_scripted_camera(0))
		observer_index = 0;
	camera = observer_get_camera(observer_index);
	csmemset(presentation, 0, sizeof(*presentation));
	SET_FLAG(presentation->flags, _presentation_cinematic_bit, cinematic_in_progress());
	SET_FLAG(presentation->flags, _presentation_letterbox_bit, cinematic_globals->show_letterbox);
	player_effect_port_screen_fade_get(&fade_color, &presentation->fade_ticks, &fading_out,
		&presentation->fade_start_time);
	SET_FLAG(presentation->flags, _presentation_fading_out_bit, fading_out);
	presentation->fade_color[0] = (byte)(PIN(fade_color.red, 0.0f, 1.0f) * 255.0f + 0.5f);
	presentation->fade_color[1] = (byte)(PIN(fade_color.green, 0.0f, 1.0f) * 255.0f + 0.5f);
	presentation->fade_color[2] = (byte)(PIN(fade_color.blue, 0.0f, 1.0f) * 255.0f + 0.5f);
	elapsed = game_time_get() - presentation->fade_start_time;
	presentation->fade_elapsed = (short)PIN(elapsed, 0, SHORT_MAX);
	if (camera)
	{
		presentation->camera_position = camera->position;
		distributed_vector_pack(&camera->forward, DISTRIBUTED_UNIT_SCALE, &presentation->camera_forward);
		distributed_vector_pack(&camera->up, DISTRIBUTED_UNIT_SCALE, &presentation->camera_up);
		presentation->camera_field_of_view =
			(word)PIN(camera->field_of_view * FIELD_OF_VIEW_SCALE + 0.5f, 1, UNSIGNED_SHORT_MAX);
		presentation->camera_object_index = camera_object_get(camera);
		if (presentation->camera_object_index != NONE)
		{
			vector_from_points3d(&object_get(presentation->camera_object_index)->object.position, &camera->position,
				&presentation->camera_object_offset);
		}
	}
	else
		presentation->camera_object_index = NONE;

	presentation->camera_scripted = (byte)(*director_camera_scripted != FALSE);
	presentation->input_disabled = (byte)!player_input_enabled();
	presentation->scripted_shake = (byte)player_effect_port_scripted_active();
	presentation->players_vitality_set = (byte)players_vitality.set;
	presentation->activating_cluster = (short)(objects_get_activating_cluster_index() + 1);
	presentation->players_maximum_body_vitality = players_vitality.maximum_body;
	presentation->players_maximum_shield_vitality = players_vitality.maximum_shield;
	game_allegiance_get_teams(presentation->ally_teams, presentation->friendly_teams);
	SET_FLAG(presentation->flags, _presentation_skippable_bit, skip_vote.offered);
	presentation->skip_votes = (byte)MIN(skip_vote.votes, 255);
	presentation->skip_voters = (byte)MIN(skip_vote.voters, 255);

	SET_FLAG(presentation->flags, _presentation_show_hud_bit, hud_scripted_globals->show_hud);
	SET_FLAG(presentation->flags, _presentation_show_help_text_bit, hud_scripted_globals->show_hud_help_text);
	presentation->unit_hud_flags = hud_unit_port_script_flags();
	presentation->weapon_hud_flags = hud_weapon_port_script_flags();
	hud_messaging_port_timer_get(&timer);
	SET_FLAG(presentation->flags, _presentation_timer_paused_bit, timer.paused);
	SET_FLAG(presentation->flags, _presentation_timer_enabled_bit, timer.enabled);
	presentation->timer_reference_time = timer.reference_time;
	presentation->timer_ticks = timer.ticks;
	presentation->timer_flash_cutoff = timer.flash_cutoff;
	presentation->timer_x = timer.x;
	presentation->timer_y = timer.y;
	presentation->timer_corner = timer.corner;
}

static void client_cinematic_end(
	void)
{
	coop_presentation.cutscene_subject = NONE;
	if (!coop_presentation.cinematic_started)
		return;
	coop_presentation.cinematic_started = FALSE;
	cinematic_stop();
}

/* client with nothing else to watch: moves the camera (position, forward,
up) behind and above the host's eyes, like spectating a teammate. It eases
toward the target so the host looking around doesn't jerk it. Returns
whether the result is a camera the observer can accept. */
static boolean client_watch_host_from_behind(
	real_point3d *position,
	real_vector3d *forward,
	real_vector3d *up)
{
	real_point3d behind;
	real_vector3d level = { forward->i, forward->j, 0.0f };

	if (normalize3d(&level) == 0.0f)
		level = *forward;
	point_from_line3d(position, &level, -WATCH_HOST_DISTANCE, &behind);
	behind.z += WATCH_HOST_HEIGHT;
	if (!coop_presentation.watching_host_valid)
	{
		coop_presentation.watching_host_valid = TRUE;
		coop_presentation.watching_host_position = behind;
		coop_presentation.watching_host_forward = *forward;
	}
	else
	{
		points_interpolate(&coop_presentation.watching_host_position, &behind, WATCH_HOST_FOLLOW,
			&coop_presentation.watching_host_position);
		vectors_interpolate(&coop_presentation.watching_host_forward, forward, WATCH_HOST_FOLLOW,
			&coop_presentation.watching_host_forward);
	}
	if (normalize3d(&coop_presentation.watching_host_forward) == 0.0f)
		return FALSE;
	*position = coop_presentation.watching_host_position;
	*forward = coop_presentation.watching_host_forward;
	/* World up is not square to a host looking up or down. Rebuild up from
	   the look direction when that pair would fail the camera check. */
	up->i = 0.0f;
	up->j = 0.0f;
	up->k = 1.0f;
	if (!distributed_axes_make_valid(forward, up))
		observer_up_from_forward(forward, up);
	return valid_real_vector3d_axes2(forward, up);
}

/* client: whether the host's camera is one that never moved, as a map's
start leaves an observer (observer_clear: at the world's origin, looking
down x). A dedicated server of an earlier build films no cutscene: it has no
player, and the camera it sends, its first observer's, stays so (its
director_unattended_scripted_camera came later). Its clients would watch
every cutscene from the origin through it */
static boolean client_host_camera_unmoved(
	struct distributed_coop_presentation const *presentation)
{
	struct distributed_vector forward;
	struct distributed_vector up;

	distributed_vector_pack(global_forward3d, DISTRIBUTED_UNIT_SCALE, &forward);
	distributed_vector_pack(global_up3d, DISTRIBUTED_UNIT_SCALE, &up);
	return presentation->camera_position.x == global_origin3d->x &&
		presentation->camera_position.y == global_origin3d->y &&
		presentation->camera_position.z == global_origin3d->z &&
		presentation->camera_forward.i == forward.i && presentation->camera_forward.j == forward.j &&
		presentation->camera_forward.k == forward.k &&
		presentation->camera_up.i == up.i && presentation->camera_up.j == up.j && presentation->camera_up.k == up.k;
}

/* client: in place of a host's camera that never moved, the cutscene's
camera watches its actor, the unit the host's scripts last animated in it
(cutscene_subject), else this machine's player, else the first player with
a unit, as spectating frames a teammate (coop_spectate_camera: from in
front of one the scripts hold, as a cutscene holds them all). The camera is
set relative to the unit, at its eyes' height (its face, not its middle,
where a wall brings the camera close) and a depth back from it, which the
observer shortens where something is in the way */
static void client_watch_cutscene(
	void)
{
	struct dead_camera *camera = &coop_presentation.unmoved_camera;
	short local_player_index = local_player_get_next(NONE);
	long player_index = local_player_index != NONE ? local_player_get_player_index(local_player_index) : NONE;
	long unit_index = player_index != NONE ? player_get(player_index)->unit_index : NONE;
	struct object_datum *unit;
	real_vector3d forward;
	real_vector3d up;
	real_point3d eye;
	real_point3d behind;
	real height;

	if (coop_presentation.cutscene_subject != NONE &&
		object_try_and_get_and_verify_type(coop_presentation.cutscene_subject, _object_mask_unit))
	{
		unit_index = coop_presentation.cutscene_subject;
	}
	if (unit_index == NONE)
	{
		struct data_iterator iterator;
		struct player_datum *player;

		data_iterator_new(&iterator, player_data);
		while (unit_index == NONE && (player = data_iterator_next(&iterator)) != NULL)
			unit_index = player->unit_index;
	}
	if (unit_index == NONE || !(unit = object_try_and_get(unit_index)))
		return;
	camera->unit_index = unit_index;
	coop_spectate_camera(camera);
	vector3d_from_euler_angles2d(&forward, &camera->facing);
	observer_up_from_forward(&forward, &up);
	unit_get_camera_position(unit_index, &eye);
	height = PIN(eye.z - unit->object.bounding_sphere_center.z, 0.0f, unit->object.bounding_sphere_radius);
	behind.x = -forward.i * camera->distance;
	behind.y = -forward.j * camera->distance;
	behind.z = -forward.k * camera->distance + height;
	if (valid_real_vector3d_axes2(&forward, &up))
		scripted_camera_set_camera_point_relative(&behind, &forward, &up, camera->field_of_view, 0, unit_index);
}

/* client: looks through the host's camera, or with its own */
static void client_host_camera_set(
	boolean host_camera)
{
	if (coop_presentation.host_camera == host_camera)
		return;
	coop_presentation.host_camera = host_camera;
	director_script_camera(host_camera);
}

static void client_apply_hud(
	struct distributed_coop_event const *event)
{
	switch (event->type)
	{
	case _coop_hud_help_text:
		hud_scripted_globals->show_hud_help_text = event->frame != 0;
		if (event->value >= 0 && event->value < hud_messaging_port_message_count())
			scripted_hud_set_state_message(event->value);
		break;
	case _coop_hud_objective:
		if (event->value >= 0 && event->value < hud_messaging_port_message_count())
			scripted_hud_set_objective(event->value);
		break;
	case _coop_hud_help_flash:
		scripted_hud_set_flashing_state(event->value != 0);
		break;
	case _coop_hud_messages_clear:
		scripted_hud_messages_clear();
		break;
	case _coop_hud_checkpoint:
		hud_autosave(event->value != 0);
		break;
	default:
		break;
	}
}

static void client_apply_player_effect(
	struct distributed_coop_event const *event)
{
	real const *reals = event->reals;

	switch (event->type)
	{
	case _coop_player_effect_translation:
		/* (the camera's shake, which moves it no further than an observer
		accepts) */
		scripted_player_effect_set_translation(PIN(reals[0], -CAMERA_WORLD_BOUND, CAMERA_WORLD_BOUND),
			PIN(reals[1], -CAMERA_WORLD_BOUND, CAMERA_WORLD_BOUND), PIN(reals[2], -CAMERA_WORLD_BOUND, CAMERA_WORLD_BOUND));
		break;
	case _coop_player_effect_rotation:
		scripted_player_effect_set_rotation(reals[0], reals[1], reals[2]);
		break;
	case _coop_player_effect_start:
		scripted_player_effect_start(PIN(reals[0], 0.0f, 1.0f), PIN(reals[1], 0.0f, 60.0f));
		break;
	case _coop_player_effect_stop:
		scripted_player_effect_stop(PIN(reals[0], 0.0f, 60.0f));
		break;
	default:
		break;
	}
}

static void client_apply_nav_point(
	struct distributed_coop_event const *event)
{
	struct scenario *scenario = global_scenario_get();
	boolean activate = event->value != NONE;
	boolean flag = event->type == _coop_nav_point_team_flag || event->type == _coop_nav_point_unit_flag;
	boolean team = event->type == _coop_nav_point_team_flag || event->type == _coop_nav_point_team_object;
	real offset = event->reals[0];

	if (activate && (event->value < 0 || event->value >= hud_globals->waypoint.arrows.count))
		return;
	if (flag && (event->object_index < 0 || event->object_index >= scenario->cutscene_flags.count))
		return;
	if (team && (event->target < 0 || event->target >= NUMBER_OF_SOLO_CAMPAIGN_TEAMS))
		return;
	if (!team && !object_try_and_get_and_verify_type(event->target, _object_mask_unit))
		return;
	switch (event->type)
	{
	case _coop_nav_point_team_flag:
		if (activate)
			hud_activate_team_nav_point_with_flag(event->value, (short)event->target, (short)event->object_index, offset);
		else
			hud_deactivate_team_nav_point_with_flag((short)event->target, (short)event->object_index);
		break;
	case _coop_nav_point_team_object:
		if (activate)
			hud_activate_team_nav_point_with_object(event->value, (short)event->target, event->object_index, offset);
		else
			hud_deactivate_team_nav_point_with_object((short)event->target, event->object_index);
		break;
	case _coop_nav_point_unit_flag:
		if (activate)
			hud_unit_activate_nav_point_with_flag(event->value, event->target, (short)event->object_index, offset);
		else
			hud_unit_deactivate_nav_point_with_flag(event->target, (short)event->object_index);
		break;
	case _coop_nav_point_unit_object:
		if (activate)
			hud_unit_activate_nav_point_with_object(event->value, event->target, event->object_index, offset);
		else
			hud_unit_deactivate_nav_point_with_object(event->target, event->object_index);
		break;
	default:
		break;
	}
}

static void client_apply_unit_animation(
	struct distributed_coop_event const *event)
{
	if (!distributed_object_index_valid(event->object_index) || !network_objects_client_has(event->object_index) ||
		!object_try_and_get_and_verify_type(event->object_index, _object_mask_unit))
	{
		return;
	}
	if (event->tag_index == NONE)
	{
		unit_stop_custom_animation(event->object_index);
	}
	else if (distributed_graph_animation(event->tag_index, event->value))
	{
		unit_port_play_user_animation(event->object_index, event->tag_index, event->value, event->interpolate, event->frame);
		/* (a cutscene's actor, for a host whose camera never moved:
		client_watch_cutscene) */
		coop_presentation.cutscene_subject = event->object_index;
	}
}

static void client_apply_scenery_animation(
	struct distributed_coop_event const *event)
{
	long scenery_index = object_find(event->name_index, event->object_index, event->definition_index,
		_object_mask_scenery);
	struct animation *animation = distributed_graph_animation(event->tag_index, event->value);

	/* scenery animations have no random permutations, so the name finds the same one */
	if (scenery_index != NONE && animation)
		scenery_animation_start_at_frame(scenery_index, event->tag_index, animation->name, event->frame);
}

/* index of the looping sound in client_looping_sounds, or NONE */
static short client_looping_sound_find(
	long definition_index)
{
	short index;

	for (index = 0; index < client_looping_sounds.count; index++)
	{
		if (client_looping_sounds.definition_indices[index] == definition_index)
			return index;
	}
	return NONE;
}

static void client_looping_sound_started(
	long definition_index)
{
	struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);

	if (TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit) &&
		definition->runtime_scripting_sound_index != NONE &&
		client_looping_sound_find(definition_index) == NONE &&
		client_looping_sounds.count < MAXIMUM_LOOPING_SOUNDS)
	{
		client_looping_sounds.definition_indices[client_looping_sounds.count++] = definition_index;
	}
}

static void client_looping_sound_forget(
	long definition_index)
{
	short index = client_looping_sound_find(definition_index);

	if (index != NONE)
		client_looping_sounds.definition_indices[index] = client_looping_sounds.definition_indices[--client_looping_sounds.count];
}

static void client_apply_sound(
	struct distributed_coop_event const *event)
{
	long object_index = event->object_index != NONE && network_objects_client_has(event->object_index) ?
		event->object_index : NONE;
	real scale = PIN(event->reals[0], 0.0f, 1.0f);

	switch (event->type)
	{
	case _coop_sound_impulse:
		if (distributed_tag_of_group(event->tag_index, SOUND_DEFINITION_TAG))
		{
			scripted_sound_new(event->tag_index, object_index, scale);
			client_script_sounds.definition_indices[client_script_sounds.next] = event->tag_index;
			client_script_sounds.next = (short)((client_script_sounds.next + 1) % CLIENT_SCRIPT_SOUND_COUNT);
		}
		break;
	case _coop_sound_looping_start:
		/* a resent sound that is already playing here is left alone, and so
		is one that has played out here (client_looping_sounds) */
		if (distributed_tag_of_group(event->tag_index, LOOPING_SOUND_DEFINITION_TAG) &&
			looping_sound_definition_get(event->tag_index)->runtime_scripting_sound_index == NONE &&
			client_looping_sound_find(event->tag_index) == NONE)
		{
			/* (one that plays once starts where the host has reached) */
			coop_sound_sync_begin(event->tag_index,
				TEST_FLAG(looping_sound_definition_get(event->tag_index)->flags,
					_looping_sound_fake_impulse_sound_bit) ? event->target : NONE);
			scripted_looping_sound_start(event->tag_index, object_index, scale);
			client_looping_sound_started(event->tag_index);
		}
		break;
	case _coop_sound_looping_stop:
		if (distributed_tag_of_group(event->tag_index, LOOPING_SOUND_DEFINITION_TAG))
		{
			client_looping_sound_forget(event->tag_index);
			coop_sound_sync_end(event->tag_index);
			scripted_looping_sound_stop(event->tag_index);
		}
		break;
	default:
		break;
	}
}

static void client_apply_effect(
	struct distributed_coop_event const *event)
{
	long object_index;
	struct object_datum *object;
	struct model *model;

	if (!distributed_tag_of_group(event->tag_index, EFFECT_DEFINITION_TAG))
		return;
	if (event->type == _coop_effect_at_flag)
	{
		if (event->value >= 0 && event->value < global_scenario_get()->cutscene_flags.count)
			hs_effect_new(event->tag_index, event->value);
		return;
	}
	object_index = object_find(event->name_index, event->object_index, event->definition_index, _object_mask_all);
	object = object_index != NONE ? object_get(object_index) : NULL;
	model = object && object_definition_get(object->definition_index)->object.model.index != NONE ?
		model_definition_get(object_definition_get(object->definition_index)->object.model.index) : NULL;
	if (model && event->value >= 0 && event->value < model->markers.count)
	{
		hs_effect_new_from_object_marker(event->tag_index, object_index,
			TAG_BLOCK_GET_ELEMENT(&model->markers, event->value, struct model_marker)->name);
	}
}

/* the name of a marker of the object's model by its index there; "" (the
object's origin) for NONE or one it lacks */
static char const *object_marker_name(
	long object_index,
	short marker_index)
{
	long model_index = object_definition_get(object_get(object_index)->definition_index)->object.model.index;
	struct model *model = model_index != NONE ? model_definition_get(model_index) : NULL;

	if (!model || marker_index < 0 || marker_index >= model->markers.count)
		return "";
	return TAG_BLOCK_GET_ELEMENT(&model->markers, marker_index, struct model_marker)->name;
}

/* the index of a marker of the object's model by its name, NONE for "" or
one it lacks */
static short object_marker_index(
	long object_index,
	char const *marker_name)
{
	long model_index = object_definition_get(object_get(object_index)->definition_index)->object.model.index;

	return model_index != NONE && marker_name && marker_name[0] ? model_find_marker(model_index, marker_name) : NONE;
}

/* An attach or a detach. The parent is object_index, name_index and
definition_index; the child is target, with its definition in tag_index and
its name index in reals[0]; value and frame are the parent's and the child's
markers. */
static void client_apply_attach(
	struct distributed_coop_event const *event)
{
	long parent_index = object_find(event->name_index, event->object_index, event->definition_index, _object_mask_all);
	long child_index = object_find((short)PIN(event->reals[0], -1.0f, 32767.0f), event->target, event->tag_index,
		_object_mask_all);
	long ancestor_index;

	if (parent_index == NONE || child_index == NONE || parent_index == child_index)
		return;
	/* a resent attach this machine already has */
	if (event->type == _coop_attach && object_get(child_index)->object.parent_object_index == parent_index)
		return;
	/* (not to one of its own children, which objects.c cannot do: copies
	of the hierarchy that differ here, or a host's mistake) */
	for (ancestor_index = object_get(parent_index)->object.parent_object_index; event->type == _coop_attach &&
		ancestor_index != NONE; ancestor_index = object_get(ancestor_index)->object.parent_object_index)
	{
		if (ancestor_index == child_index)
			return;
	}
	if (event->type == _coop_detach)
		objects_scripting_detach(parent_index, child_index);
	else
	{
		objects_scripting_attach(parent_index, object_marker_name(parent_index, event->value), child_index,
			object_marker_name(child_index, event->frame));
	}
}

/* client: the cutscene was skipped, so the dialogue it started stops (the
host's went with its revert; its music stops through host_send_looping_sounds) */
static void client_stop_script_sounds(
	void)
{
	short index;

	for (index = 0; index < CLIENT_SCRIPT_SOUND_COUNT; index++)
	{
		long definition_index = client_script_sounds.definition_indices[index];

		if (definition_index != NONE && distributed_tag_of_group(definition_index, SOUND_DEFINITION_TAG))
			sound_stop_impulse(sound_definition_get(definition_index)->scripting_sound_index);
		client_script_sounds.definition_indices[index] = NONE;
	}
}

static void client_apply_event(
	struct distributed_coop_event const *event)
{
	switch (event->kind)
	{
	case _coop_event_sound:
		client_apply_sound(event);
		break;
	case _coop_event_title:
		if (event->value >= 0 && event->value < global_scenario_get()->cutscene_chapter_titles.count)
			cinematic_set_title_delayed(event->value, PIN(event->reals[0], 0.0f, 60.0f));
		break;
	case _coop_event_hud:
		client_apply_hud(event);
		break;
	case _coop_event_player_effect:
		client_apply_player_effect(event);
		break;
	case _coop_event_nav_point:
		client_apply_nav_point(event);
		break;
	case _coop_event_unit_animation:
		client_apply_unit_animation(event);
		break;
	case _coop_event_scenery_animation:
		client_apply_scenery_animation(event);
		break;
	case _coop_event_effect:
		client_apply_effect(event);
		break;
	case _coop_event_attach:
		client_apply_attach(event);
		break;
	case _coop_event_reverted:
		client_stop_script_sounds();
		break;
	case _coop_event_surface_broken:
		if (event->frame == global_structure_bsp_index)
		{
			real_point3d epicenter = { event->reals[0], event->reals[1], event->reals[2] };

			breakable_surface_port_break(event->value, &epicenter);
		}
		break;
	case _coop_event_unit_open:
		if (distributed_object_index_valid(event->object_index) && network_objects_client_has(event->object_index) &&
			object_try_and_get_and_verify_type(event->object_index, _object_mask_unit))
		{
			if (event->value)
				unit_open(event->object_index);
			else
				unit_close(event->object_index);
		}
		break;
	default:
		break;
	}
}

/* client: the HUD settings in the host's presentation */
static void client_apply_hud_state(
	struct distributed_coop_presentation const *presentation)
{
	struct hud_timer_state timer;

	hud_scripted_globals->show_hud = TEST_FLAG(presentation->flags, _presentation_show_hud_bit);
	hud_scripted_globals->show_hud_help_text = TEST_FLAG(presentation->flags, _presentation_show_help_text_bit);
	hud_unit_port_set_script_flags(presentation->unit_hud_flags);
	hud_weapon_port_set_script_flags(presentation->weapon_hud_flags);
	timer.reference_time = presentation->timer_reference_time;
	timer.ticks = presentation->timer_ticks;
	timer.flash_cutoff = presentation->timer_flash_cutoff;
	timer.x = presentation->timer_x;
	timer.y = presentation->timer_y;
	timer.corner = presentation->timer_corner;
	timer.paused = TEST_FLAG(presentation->flags, _presentation_timer_paused_bit);
	timer.enabled = TEST_FLAG(presentation->flags, _presentation_timer_enabled_bit);
	hud_messaging_port_timer_set(&timer);
}

/* the players in the game */
static short coop_player_count(
	void)
{
	struct data_iterator iterator;
	short count = 0;

	data_iterator_new(&iterator, player_data);
	while (data_iterator_next(&iterator))
		count++;

	return count;
}

/* host: the player's friendly fire, begun again for a new player in the place
of one who left (NULL: no such player) */
static struct friendly_fire_player *host_friendly_fire_player(
	long player_index)
{
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	struct friendly_fire_player *entry;

	if (player_index == NONE || absolute_index >= MAXIMUM_TRACKED_PLAYERS)
		return NULL;
	entry = &host_friendly_fire[absolute_index];
	if (entry->player_index != player_index)
	{
		csmemset(entry, 0, sizeof(*entry));
		entry->player_index = player_index;
	}
	return entry;
}

/* host, each tick: the host's own players' choice, and how long each player
has been away from the controls. The input the tick ran for them shows them
there if a button was pressed or let go, or a stick or the trigger is held
(the game itself turns a player's view and chooses their weapon). */
static void host_friendly_fire_update(
	void)
{
	long update_number = update_server_ticked_update_number();
	short action_count = 0;
	struct player_action const *actions =
		update_number != NONE ? update_server_update_actions(update_number, &action_count) : NULL;
	struct data_iterator iterator;
	struct player_datum *player;

	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		short absolute_index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index);
		struct friendly_fire_player *entry = host_friendly_fire_player(iterator.datum_index);
		struct player_action const *action;

		if (!entry)
			continue;
		if (player->local_player_index != NONE)
			entry->declined = !network_coop_own_friendly_fire();
		if (!actions || absolute_index >= action_count)
			continue;
		action = &actions[absolute_index];
		if (action->control_flags != entry->control_flags || action->throttle.i != 0.0f ||
			action->throttle.j != 0.0f || action->primary_trigger != 0.0f)
		{
			entry->idle_ticks = 0;
		}
		else if (entry->idle_ticks <= FRIENDLY_FIRE_AWAY_TICKS)
		{
			entry->idle_ticks++;
		}
		entry->control_flags = action->control_flags;
	}
}

static boolean tag_name_has(
	long definition_index,
	char const *text)
{
	return definition_index != NONE && strstr(tag_get_name(definition_index), text) != NULL;
}

/* each tick: every player's unit at the players' maximum vitality. The
host's own units get their current vitality full, as the scripts' set does;
a client's follows the host's damage states. */
static void players_vitality_keep(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	if (!players_vitality.set)
		return;
	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		struct object_datum *unit = object_try_and_get(player->unit_index);

		if (!unit || (unit->object.maximum_body_vitality == players_vitality.maximum_body &&
			unit->object.maximum_shield_vitality == players_vitality.maximum_shield))
		{
			continue;
		}
		if (coop_host())
			object_initialize_vitality(player->unit_index, &players_vitality.maximum_body, &players_vitality.maximum_shield);
		else
		{
			unit->object.maximum_body_vitality = players_vitality.maximum_body;
			unit->object.maximum_shield_vitality = players_vitality.maximum_shield;
		}
	}
}

/* ---------- public code */

void network_coop_new_game(
	void)
{
	csmemset(&coop_presentation, 0, sizeof(coop_presentation));
	coop_presentation.cutscene_subject = NONE;
	csmemset(&coop_events, 0, sizeof(coop_events));
	csmemset(&host_devices, 0, sizeof(host_devices));
	csmemset(&client_devices, 0, sizeof(client_devices));
	csmemset(client_names_differing, 0, sizeof(client_names_differing));
	csmemset(client_script_sounds.definition_indices, NONE, sizeof(client_script_sounds.definition_indices));
	csmemset(host_sent_transforms, 0, sizeof(host_sent_transforms));
	csmemset(host_sent_looks, 0, sizeof(host_sent_looks));
	csmemset(host_sent_devices, 0, sizeof(host_sent_devices));
	csmemset(host_player_structure_bsps, NONE, sizeof(host_player_structure_bsps));
	csmemset(host_friendly_fire, 0, sizeof(host_friendly_fire));
	csmemset(&host_sent_screen_effect, 0, sizeof(host_sent_screen_effect));
	csmemset(&host_resend, 0, sizeof(host_resend));
	csmemset(&checkpoint_players, 0, sizeof(checkpoint_players));
	csmemset(&players_vitality, 0, sizeof(players_vitality));
	host_attachment_count = 0;
	host_looping_sound_count = 0;
	client_looping_sounds.count = 0;
	coop_sound_sync_reset();
	host_hud_state.objective = NONE;
	host_hud_state.nav_point_count = 0;
	skip_vote_clear();
	skip_vote.offered = FALSE;
	skip_vote.voters = 0;
	skip_vote.cooldown_until = 0;
	skip_vote.skip_save_written = FALSE;
	coop_enemies_new_game();
	coop_pickups_new_game();
	coop_vehicles_new_game();
}

/* game_state.c calls only after the checkpoint file was written. This
metadata stays outside the saved image, as the live machine table does. */
void network_coop_checkpoint_saved(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;
	short machine_index;
	short local_player_index;

	checkpoint_players.valid = FALSE;
	checkpoint_players.count = 0;
	if (!coop_host())
		return;
	csmemset(checkpoint_players.players, NONE, sizeof(checkpoint_players.players));
	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		short index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index);

		if (player->quit_out_of_game)
			continue;
		if (!VALID_INDEX(index, MAXIMUM_TRACKED_PLAYERS))
			return;
		checkpoint_players.players[index] = iterator.datum_index;
		checkpoint_players.count++;
	}
	for (machine_index = 0; machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES; machine_index++)
	{
		long *players = machine_get_player_list(machine_index);

		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
			checkpoint_players.machine_players[machine_index][local_player_index] = players[local_player_index];
	}
	checkpoint_players.valid = checkpoint_players.count > 0;
}

static boolean checkpoint_players_match(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;
	short count = 0;
	short machine_index;
	short local_player_index;

	if (!checkpoint_players.valid)
		return FALSE;
	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		short index = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index);

		if (player->quit_out_of_game)
			continue;
		if (!VALID_INDEX(index, MAXIMUM_TRACKED_PLAYERS) ||
			checkpoint_players.players[index] != iterator.datum_index)
		{
			return FALSE;
		}
		count++;
	}
	if (count != checkpoint_players.count)
		return FALSE;
	for (machine_index = 0; machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES; machine_index++)
	{
		long *players = machine_get_player_list(machine_index);

		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
		{
			if (checkpoint_players.machine_players[machine_index][local_player_index] != players[local_player_index])
				return FALSE;
		}
	}
	return TRUE;
}

/* Only the host chooses. Unknown values use the automatic policy too;
PUBLIC is a visibility choice, including games protected by a password. */
/* whether the game hosted is public, for network.coop_team_wipe "auto": a
player's listed lobby (p2p_hosting_listed), or a dedicated server's
server.public with internet play, which its lobby's own flag does not say */
static boolean network_coop_hosting_public(
	void)
{
	if (halo_server_mode())
		return config_boolean("server.public") && config_boolean("network.online");
	return p2p_hosting_listed() != 0;
}

boolean network_coop_wipe_reverts(
	void)
{
	char const *mode;

	if (!coop_host())
		return FALSE;
	mode = config_string("network.coop_team_wipe");
	if (!strcmp(mode, "respawn") || (strcmp(mode, "revert") && network_coop_hosting_public()))
		return FALSE;
	if (!checkpoint_players_match())
	{
		error(_error_silent, "co-op: team wipe respawns without a revert: no checkpoint with the current players and machines");
		return FALSE;
	}
	return TRUE;
}

/* A network game on a campaign scenario with no game engine. Checking the
scenario matters: the main menu's scene keeps running while a network game
is set up, also with no game engine, and it is not co-op. */
boolean network_coop_active(
	void)
{
	short connection = game_connection();

	return (connection == _game_connection_network_server || connection == _game_connection_network_client) &&
		global_scenario && global_scenario->type == _scenario_type_solo && !game_engine_running();
}

boolean network_coop_player_collisions(
	void)
{
	struct network_game *game;

	if (!network_coop_active() || !(game = network_game_get_game()))
		return TRUE;
	return !TEST_FLAG(game->cooperative_flags, _network_game_cooperative_no_player_collisions_bit);
}

boolean network_coop_ally_friendly_fire(
	void)
{
	struct network_game *game;

	if (!network_coop_active() || !(game = network_game_get_game()))
		return TRUE;
	return !TEST_FLAG(game->cooperative_flags, _network_game_cooperative_no_ally_friendly_fire_bit);
}

boolean network_coop_own_friendly_fire(
	void)
{
	return config_boolean("network.coop_own_friendly_fire") != 0;
}

void network_coop_note_player_friendly_fire(
	long player_index,
	boolean friendly_fire)
{
	struct friendly_fire_player *entry = coop_host() ? host_friendly_fire_player(player_index) : NULL;

	if (entry)
		entry->declined = !friendly_fire;
}

boolean network_coop_players_friendly_fire(
	long attacker_player_index,
	long victim_player_index)
{
	struct friendly_fire_player *attacker, *victim;

	/* (a client deals no damage, and a player's own fire is the game's) */
	if (!coop_host() || attacker_player_index == victim_player_index)
		return TRUE;
	attacker = host_friendly_fire_player(attacker_player_index);
	victim = host_friendly_fire_player(victim_player_index);
	if (!attacker || !victim)
		return TRUE;
	/* one who is away is hurt whatever either chose; else both must play
	with it */
	return victim->idle_ticks > FRIENDLY_FIRE_AWAY_TICKS || (!attacker->declined && !victim->declined);
}

boolean network_coop_player_away(
	long player_index)
{
	struct friendly_fire_player *entry;

	if (!coop_host())
		return FALSE;
	entry = host_friendly_fire_player(player_index);
	return entry && entry->idle_ticks > FRIENDLY_FIRE_AWAY_TICKS;
}

boolean network_coop_host_dedicated(
	void)
{
	struct network_game *game;

	if (game_connection() == _game_connection_network_server)
		return halo_server_mode() != 0;
	if (!network_coop_active() || !(game = network_game_get_game()))
		return FALSE;
	return TEST_FLAG(game->cooperative_flags, _network_game_cooperative_dedicated_bit);
}

boolean network_coop_bsp_switch_majority(
	void)
{
	return coop_host() && halo_server_mode() && strcmp(config_string("server.coop_bsp_switch"), "anyone") != 0;
}

long network_coop_bsp_switch_cooldown_ticks(
	void)
{
	long seconds;

	if (!coop_host() || !halo_server_mode())
		return 0;
	seconds = config_integer("server.coop_bsp_switch_cooldown_seconds");
	return seconds < 0 ? 0 : MIN(seconds, 300) * TICKS_PER_SECOND;
}

/* how often a player waiting at a loading zone is told so */
#define BSP_SWITCH_WAITING_TICKS (3 * TICKS_PER_SECOND)

void network_coop_tell_bsp_switch_waiting(
	long player_index,
	long more,
	long in,
	long total)
{
	static long told_times[MAXIMUM_TRACKED_PLAYERS];
	long absolute_index = DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index);
	long machine_index = distributed_player_machine((short)absolute_index);
	char text[128];

	if (!coop_host() || absolute_index >= MAXIMUM_TRACKED_PLAYERS ||
		(told_times[absolute_index] && game_time_get() - told_times[absolute_index] < BSP_SWITCH_WAITING_TICKS &&
			game_time_get() >= told_times[absolute_index]))
	{
		return;
	}
	told_times[absolute_index] = game_time_get() ? game_time_get() : 1;
	if (more > 0)
		snprintf(text, sizeof(text), "Waiting for %ld more player%s to cross (%ld of %ld)", more, more == 1 ? "" : "s",
			in, total);
	else
		snprintf(text, sizeof(text), "The loading zone opens in %ld seconds", total);
	if (machine_index == NONE)
	{
		short local_player_index = player_get(player_index)->local_player_index;
		wchar_t wide[128];
		long index;

		if (local_player_index == NONE)
			return;
		for (index = 0; text[index] && index < (long)NUMBEROF(wide) - 1; index++)
			wide[index] = (wchar_t)(unsigned char)text[index];
		wide[index] = 0;
		hud_print_message(local_player_index, wide);
		return;
	}
	distributed_send_notice_to_machine(machine_index, text);
}

void network_coop_note_bsp_switch(
	short from,
	short to,
	long const *player_indices,
	long count)
{
	char names[96] = "", notice[160];
	struct server_json json;
	long index;

	if (!coop_host())
		return;
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "event", "coop_bsp_switch");
	server_json_integer(&json, "from", from);
	server_json_integer(&json, "to", to);
	server_json_array_begin(&json, "players");
	for (index = 0; index < count; index++)
	{
		char name[16] = "";

		if (player_indices[index] == NONE)
			continue;
		{
			struct player_datum *player = player_get(player_indices[index]);
			long character;

			/* (printable ASCII of the player's name) */
			for (character = 0; character < (long)NUMBEROF(player->name) && player->name[character] &&
				character < (long)sizeof(name) - 1; character++)
			{
				name[character] = player->name[character] >= 0x20 && player->name[character] < 0x7F ?
					(char)player->name[character] : '?';
			}
			name[character] = 0;
		}
		server_json_string(&json, NULL, name);
		if (strlen(names) + strlen(name) + 3 < sizeof(names))
		{
			if (names[0])
				strcat(names, index == count - 1 ? " and " : ", ");
			strcat(names, name);
		}
	}
	server_json_array_end(&json);
	server_json_object_end(&json);
	if (halo_server_mode())
	{
		server_event_add("game", server_json_text(&json));
		server_log(_server_log_info, "coop", "the loading zone to part %d crossed by %s (from part %d)", to,
			names[0] ? names : "nobody", from);
	}
	server_json_free(&json);
	snprintf(notice, sizeof(notice), "%s moved the team on", names[0] ? names : "A player");
	distributed_send_notice(notice);
}

/* a dedicated server's co-op entry's riding rule (NONE: the entry gives
none), set for every entry it applies so the next cannot inherit it */
static short riding_entry_rule = NONE;

void network_game_port_vehicle_riding_entry(
	short rule)
{
	riding_entry_rule = rule == NONE || rule == FALSE ? rule : TRUE;
}

/* the host's decision for the game about to start, the only writer of the
riding bit: it is made from this game alone, so nothing of an earlier game
(a co-op game's option, a gametype's flag) is carried over */
void network_game_port_vehicle_riding_start(
	struct network_game *game)
{
	boolean riding = vehicle_riding_decide(
		game->variant.game_engine_index != game_engine_none,
		TEST_FLAG(game->variant.universal_variant.flags, _game_variant_vehicle_riding_bit),
		config_boolean("network.vehicle_riding") != 0,
		riding_entry_rule,
		halo_server_mode() != 0,
		config_boolean("server.vehicle_riding_override_gametype") != 0);

	SET_FLAG(game->cooperative_flags, _network_game_cooperative_vehicle_riding_bit, riding);
}

/* whether the host's decision for the running game is to carry players standing on a vehicle (bit 3 of the
cooperative_flags, which travel with every settings send: vehicle_riding.c) */
boolean network_game_port_vehicle_riding(
	void)
{
	struct network_game *game = network_game_get_game();

	return game && TEST_FLAG(game->cooperative_flags, _network_game_cooperative_vehicle_riding_bit);
}

boolean network_coop_devices_remote(
	void)
{
	return coop_client();
}

/* units.c: a vehicle left the vehicle carrying it. When a Pelican drops a
Warthog in a larger game, the host drops more beside it, so every player has
a ride: one Warthog for every four players, at most five. They go side by
side, alternately left and right, each only where no wall stands between it
and the first, and reach the clients as any vehicle does. */
void network_coop_vehicle_dropped(
	long vehicle_index,
	long carrier_index)
{
	struct object_datum const *vehicle = object_get(vehicle_index);
	short wanted = (short)MIN((coop_player_count() + PLAYERS_PER_DROPPED_VEHICLE - 1) / PLAYERS_PER_DROPPED_VEHICLE,
		MAXIMUM_DROPPED_VEHICLES) - 1;
	real_vector3d left;
	short spot, placed = 0;

	if (!coop_host() || wanted <= 0 || !tag_name_has(vehicle->definition_index, "warthog") ||
		!tag_name_has(object_get(carrier_index)->definition_index, "pelican"))
	{
		return;
	}
	cross_product3d(&vehicle->object.up, &vehicle->object.forward, &left);
	for (spot = 1; spot <= 2 * MAXIMUM_DROPPED_VEHICLES && placed < wanted; spot++)
	{
		/* (1.5 world units apart: a Warthog's width and a gap) */
		real distance = 1.5f * ((spot + 1) / 2) * (spot % 2 ? 1.0f : -1.0f);
		struct object_placement_data data;
		struct collision_result collision;
		real_vector3d offset;

		scale_vector3d(&left, distance, &offset);
		if (collision_test_vector(FLAG(_collision_test_structure_bit), &vehicle->object.position, &offset, NONE,
			&collision))
		{
			continue;
		}
		object_placement_data_new(&data, vehicle->definition_index, NONE);
		point_from_line3d(&vehicle->object.position, &offset, 1.0f, &data.position);
		data.forward = vehicle->object.forward;
		data.up = vehicle->object.up;
		data.translational_velocity = vehicle->object.translational_velocity;
		if (object_new(&data) != NONE)
			placed++;
	}
	error(_error_silent, "co-op: a Pelican dropped %d more Warthogs for %d players", placed, coop_player_count());
}

/* unit_scripting_commands.c: the scripts set a unit's maximum (maximum
TRUE) or current vitality. Campaign scripts name only player0 for what
every player should have, so in co-op a player's is set on every player's
unit, and a maximum is kept for those spawned later and sent to the
clients. FALSE if the unit isn't a co-op player's, for the caller to set. */
boolean network_coop_set_players_vitality(
	long unit_index,
	boolean maximum,
	real body,
	real shield)
{
	struct data_iterator iterator;
	struct player_datum *player;

	if (!coop_host() || player_index_from_unit_index(unit_index) == NONE)
		return FALSE;
	if (maximum)
	{
		players_vitality.set = TRUE;
		players_vitality.maximum_body = body;
		players_vitality.maximum_shield = shield;
	}
	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		struct object_datum *unit = object_try_and_get(player->unit_index);

		if (!unit || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
			continue;
		if (maximum)
			object_initialize_vitality(player->unit_index, &body, &shield);
		else
			unit_scripting_set_current_vitality_of(player->unit_index, body, shield);
	}
	return TRUE;
}

void network_coop_note_device_snap(
	short group_index)
{
	if (coop_host() && group_index >= 0 && group_index < MAXIMUM_DEVICE_GROUPS)
		host_devices.snap_counts[group_index]++;
}

/* index of the looping sound in host_looping_sounds, or NONE */
static short host_looping_sound_find(
	long definition_index)
{
	short index;

	for (index = 0; index < host_looping_sound_count; index++)
	{
		if (host_looping_sounds[index].definition_index == definition_index)
			return index;
	}
	return NONE;
}

/* the most a sound plays, its linked permutations together, in
milliseconds, or 0 for none (coop_sound_sync.c: the tag's
longest_permutation_length is a single permutation's, a little over a second
of a cutscene's music of 86) */
static long looping_sound_part_milliseconds(
	long sound_index)
{
	long length;

	if (sound_index == NONE)
		return 0;
	length = coop_sound_sync_sound_milliseconds(sound_index);
	return PIN(length, 0, MAXIMUM_LOOPING_SOUND_PART_MILLISECONDS);
}

/* the most ticks a looping sound that plays once can take (each track's
start, loop and stop sounds, the longer of a loop or stop and its
alternate), or 0 for one that does not play once or whose sounds give no
length */
static long looping_sound_play_once_ticks(
	long definition_index)
{
	struct looping_sound_definition *definition = looping_sound_definition_get(definition_index);
	long longest = 0;
	long track_index;

	if (!TEST_FLAG(definition->flags, _looping_sound_fake_impulse_sound_bit))
		return 0;
	for (track_index = 0; track_index < definition->tracks.count && track_index < MAXIMUM_TRACKS_PER_LOOPING_SOUND;
		track_index++)
	{
		struct looping_sound_track *track =
			TAG_BLOCK_GET_ELEMENT(&definition->tracks, track_index, struct looping_sound_track);
		long length = looping_sound_part_milliseconds(track->start_sound.index) +
			MAX(looping_sound_part_milliseconds(track->loop_sound.index),
				looping_sound_part_milliseconds(track->alternate_loop_sound.index)) +
			MAX(looping_sound_part_milliseconds(track->stop_sound.index),
				looping_sound_part_milliseconds(track->alternate_stop_sound.index));

		longest = MAX(longest, length);
	}
	return longest * TICKS_PER_SECOND / 1000;
}

/* host, each tick: a looping sound that stopped without the scripts
stopping it (it ended, or a skipped cutscene's revert took it) is stopped on
the clients too; one that plays once is ended where nothing draws once it
has had the time to play (host_looping_sounds); the rest are started again
with the resent state */
static void host_send_looping_sounds(
	void)
{
	short index = 0;

	while (index < host_looping_sound_count)
	{
		long definition_index = host_looping_sounds[index].definition_index;
		long length = debug_no_drawing ? host_looping_sounds[index].length : 0;
		long played = game_time_get() - host_looping_sounds[index].start_time;
		struct distributed_coop_event *event;

		if (length > 0 && played >= length + LOOPING_SOUND_END_MARGIN_TICKS &&
			looping_sound_definition_get(definition_index)->runtime_scripting_sound_index != NONE)
		{
			/* (its stop, sent and taken out of the list as the scripts' is;
			the next pass takes it out if no stop could be sent) */
			scripted_looping_sound_stop(definition_index);
			continue;
		}
		if (looping_sound_definition_get(definition_index)->runtime_scripting_sound_index == NONE)
		{
			if ((event = event_new(_coop_event_sound)) != NULL)
			{
				event->type = _coop_sound_looping_stop;
				event->tag_index = definition_index;
			}
			host_looping_sounds[index] = host_looping_sounds[--host_looping_sound_count];
			continue;
		}
		if (host_resend.refresh && !(length > 0 && played + LOOPING_SOUND_END_MARGIN_TICKS >= length) &&
			(event = event_new(_coop_event_sound)) != NULL)
		{
			event->type = _coop_sound_looping_start;
			event->tag_index = definition_index;
			event->object_index = host_looping_sounds[index].object_index;
			event->reals[0] = host_looping_sounds[index].scale;
			/* (one that plays once: how long ago, so a machine that starts it
			now starts it where this one is) */
			if (host_looping_sounds[index].length > 0 && played > 0)
				event->target = played * 1000 / TICKS_PER_SECOND;
		}
		index++;
	}
}

void network_coop_note_sound(
	short kind,
	long definition_index,
	long object_index,
	real scale)
{
	struct distributed_coop_event *event = event_new(_coop_event_sound);
	short index;

	if (!event)
		return;
	event->type = (byte)kind;
	event->tag_index = definition_index;
	event->object_index = object_index;
	event->reals[0] = scale;

	index = host_looping_sound_find(definition_index);
	if (kind == _coop_sound_looping_stop && index != NONE)
	{
		host_looping_sounds[index] = host_looping_sounds[--host_looping_sound_count];
	}
	else if (kind == _coop_sound_looping_start)
	{
		if (index == NONE && host_looping_sound_count < MAXIMUM_LOOPING_SOUNDS)
			index = host_looping_sound_count++;
		if (index != NONE)
		{
			host_looping_sounds[index].definition_index = definition_index;
			host_looping_sounds[index].object_index = object_index;
			host_looping_sounds[index].scale = scale;
			host_looping_sounds[index].start_time = game_time_get();
			host_looping_sounds[index].length = looping_sound_play_once_ticks(definition_index);
		}
	}
}

void network_coop_note_title(
	short title_index,
	real delay)
{
	struct distributed_coop_event *event = event_new(_coop_event_title);

	if (!event)
		return;
	event->value = title_index;
	event->reals[0] = delay;
}

void network_coop_note_hud(
	short kind,
	short value)
{
	struct distributed_coop_event *event = event_new(_coop_event_hud);

	if (!event)
		return;
	event->type = (byte)kind;
	event->value = value;
	/* (help text shows only while the scripts show it; the presentation
	saying so comes a little after the event, so the event carries it) */
	event->frame = (short)hud_scripted_globals->show_hud_help_text;
	if (kind == _coop_hud_objective)
		host_hud_state.objective = value;
}

void network_coop_note_player_effect(
	short kind,
	real a,
	real b,
	real c)
{
	struct distributed_coop_event *event = event_new(_coop_event_player_effect);

	if (!event)
		return;
	event->type = (byte)kind;
	event->reals[0] = a;
	event->reals[1] = b;
	event->reals[2] = c;
}

static void send_nav_point(
	short kind,
	short nav_index,
	long target,
	long marker,
	real vertical_offset)
{
	struct distributed_coop_event *event = event_new(_coop_event_nav_point);

	if (!event)
		return;
	event->type = (byte)kind;
	event->value = nav_index;
	event->target = target;
	event->object_index = marker;
	event->reals[0] = vertical_offset;
}

void network_coop_note_nav_point(
	short kind,
	short nav_index,
	long target,
	long marker,
	real vertical_offset)
{
	short index;

	if (!coop_host())
		return;
	send_nav_point(kind, nav_index, target, marker, vertical_offset);
	for (index = 0; index < host_hud_state.nav_point_count; index++)
	{
		if (host_hud_state.nav_points[index].kind == kind && host_hud_state.nav_points[index].target == target &&
			host_hud_state.nav_points[index].marker == marker)
		{
			break;
		}
	}
	if (nav_index == NONE)
	{
		if (index < host_hud_state.nav_point_count)
			host_hud_state.nav_points[index] = host_hud_state.nav_points[--host_hud_state.nav_point_count];
		return;
	}
	if (index == host_hud_state.nav_point_count)
	{
		if (index == MAXIMUM_NAV_POINTS)
			return;
		host_hud_state.nav_point_count++;
	}
	host_hud_state.nav_points[index].kind = kind;
	host_hud_state.nav_points[index].nav_index = nav_index;
	host_hud_state.nav_points[index].target = target;
	host_hud_state.nav_points[index].marker = marker;
	host_hud_state.nav_points[index].vertical_offset = vertical_offset;
}

/* host: the active nav points again (host_resend), and the objective to a
machine that joined (it shows on screen when it comes, so not more often) */
static void host_send_hud_state(
	void)
{
	short index;

	if (!host_resend.refresh)
		return;
	if (host_resend.joined && host_hud_state.objective != NONE)
	{
		struct distributed_coop_event *event = event_new(_coop_event_hud);

		if (event)
		{
			event->type = _coop_hud_objective;
			event->value = host_hud_state.objective;
		}
	}
	for (index = 0; index < host_hud_state.nav_point_count; index++)
	{
		send_nav_point(host_hud_state.nav_points[index].kind, host_hud_state.nav_points[index].nav_index,
			host_hud_state.nav_points[index].target, host_hud_state.nav_points[index].marker,
			host_hud_state.nav_points[index].vertical_offset);
	}
}

void network_coop_note_unit_animation(
	long unit_index,
	long animation_graph_index,
	short animation_index,
	boolean interpolate)
{
	struct distributed_coop_event *event = event_new(_coop_event_unit_animation);

	if (!event)
		return;
	event->object_index = unit_index;
	event->tag_index = animation_graph_index;
	event->value = animation_index;
	event->interpolate = (byte)interpolate;
}

void network_coop_note_unit_open(
	long unit_index,
	boolean open)
{
	struct distributed_coop_event *event = event_new(_coop_event_unit_open);

	if (!event)
		return;
	event->object_index = unit_index;
	event->value = (short)open;
}

/* unit_custom_animation_at_frame starts the animation and then sets the
frame in the same call, so the start is still queued and unsent: the frame
goes into it */
void network_coop_note_unit_animation_frame(
	long unit_index,
	short frame_index)
{
	short index;

	if (!coop_host())
		return;
	for (index = coop_events.count - 1; index >= 0; index--)
	{
		struct distributed_coop_event *event = &coop_events.events[index];

		if (event->kind == _coop_event_unit_animation && event->object_index == unit_index)
		{
			if (coop_events.sends[index] == 0)
				event->frame = frame_index;
			return;
		}
	}
}

void network_coop_note_scenery_animation(
	long object_index,
	long animation_graph_index,
	short animation_index,
	short frame_index)
{
	struct object_datum *object = object_try_and_get(object_index);
	struct distributed_coop_event *event;

	if (!object || !(event = event_new(_coop_event_scenery_animation)))
		return;
	event->object_index = object_index;
	event->name_index = object->object.name_index;
	event->definition_index = object->definition_index;
	event->tag_index = animation_graph_index;
	event->value = animation_index;
	event->frame = frame_index;
}

/* An attach (with the markers' indexes) or a detach (NONE markers). */
static void send_attach(
	byte type,
	long parent_index,
	short parent_marker_index,
	long child_index,
	short child_marker_index)
{
	struct object_datum *parent = object_try_and_get(parent_index);
	struct object_datum *child = object_try_and_get(child_index);
	struct distributed_coop_event *event;

	if (!parent || !child || !(event = event_new(_coop_event_attach)))
		return;
	event->type = type;
	event->object_index = parent_index;
	event->name_index = parent->object.name_index;
	event->definition_index = parent->definition_index;
	event->target = child_index;
	event->tag_index = child->definition_index;
	event->reals[0] = child->object.name_index;
	event->value = parent_marker_index;
	event->frame = child_marker_index;
}

/* the child's attachment in host_attachments, or NONE */
static short host_attachment_find(
	long child_index)
{
	short index;

	for (index = 0; index < host_attachment_count; index++)
	{
		if (host_attachments[index].child_index == child_index)
			return index;
	}
	return NONE;
}

static void host_send_attachments(
	void)
{
	short index = 0;

	if (!host_resend.refresh)
		return;
	while (index < host_attachment_count)
	{
		struct object_datum *child = object_try_and_get(host_attachments[index].child_index);

		/* the child is gone or has a new parent: forget it */
		if (!child || child->object.parent_object_index != host_attachments[index].parent_index)
		{
			host_attachments[index] = host_attachments[--host_attachment_count];
			continue;
		}
		send_attach(_coop_attach, host_attachments[index].parent_index, host_attachments[index].parent_marker_index,
			host_attachments[index].child_index, host_attachments[index].child_marker_index);
		index++;
	}
}

void network_coop_note_attach(
	long parent_index,
	char const *parent_marker_name,
	long child_index,
	char const *child_marker_name)
{
	short parent_marker_index, child_marker_index;
	short index;

	if (!coop_host() || !object_try_and_get(parent_index) || !object_try_and_get(child_index))
		return;
	parent_marker_index = object_marker_index(parent_index, parent_marker_name);
	child_marker_index = object_marker_index(child_index, child_marker_name);
	send_attach(_coop_attach, parent_index, parent_marker_index, child_index, child_marker_index);

	index = host_attachment_find(child_index);
	if (index == NONE)
	{
		if (host_attachment_count == MAXIMUM_ATTACHMENTS)
			return;
		index = host_attachment_count++;
	}
	host_attachments[index].parent_index = parent_index;
	host_attachments[index].child_index = child_index;
	host_attachments[index].parent_marker_index = parent_marker_index;
	host_attachments[index].child_marker_index = child_marker_index;
}

void network_coop_note_detach(
	long parent_index,
	long child_index)
{
	short index;

	if (!coop_host())
		return;
	send_attach(_coop_detach, parent_index, NONE, child_index, NONE);
	index = host_attachment_find(child_index);
	if (index != NONE)
		host_attachments[index] = host_attachments[--host_attachment_count];
}

void network_coop_note_effect(
	long effect_definition_index,
	short cutscene_flag_index)
{
	struct distributed_coop_event *event = event_new(_coop_event_effect);

	if (!event)
		return;
	event->type = _coop_effect_at_flag;
	event->tag_index = effect_definition_index;
	event->value = cutscene_flag_index;
}

void network_coop_note_surface_broken(
	short breakable_surface_index,
	real_point3d const *epicenter)
{
	struct distributed_coop_event *event = event_new(_coop_event_surface_broken);

	if (!event)
		return;
	event->value = breakable_surface_index;
	event->frame = global_structure_bsp_index;
	event->reals[0] = epicenter->x;
	event->reals[1] = epicenter->y;
	event->reals[2] = epicenter->z;
}

/* host: a few broken surfaces again each refresh, round them all, for a
machine that joined since or lost the message (broken from their middle) */
static void host_send_broken_surfaces(
	void)
{
	enum { SURFACES_PER_REFRESH = 8 };
	static short cursor;
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short count = (short)structure_bsp->breakable_surfaces.count;
	short sent = 0;
	short step;

	if (!host_resend.refresh || count <= 0)
		return;
	for (step = 0; step < count && sent < SURFACES_PER_REFRESH; step++)
	{
		short index = (short)((cursor + step) % count);

		if (!breakable_surface_extant(index))
		{
			struct structure_breakable_surface *surface = TAG_BLOCK_GET_ELEMENT(&structure_bsp->breakable_surfaces,
				index, struct structure_breakable_surface);

			network_coop_note_surface_broken(index, &surface->centroid);
			sent++;
		}
	}
	cursor = (short)((cursor + step) % count);
}

void network_coop_note_object_effect(
	long effect_definition_index,
	long object_index,
	char const *marker_name)
{
	struct object_datum *object = object_try_and_get(object_index);
	long model_index = object ? object_definition_get(object->definition_index)->object.model.index : NONE;
	short marker_index = model_index != NONE ? model_find_marker(model_index, marker_name) : NONE;
	struct distributed_coop_event *event;

	if (marker_index == NONE || !(event = event_new(_coop_event_effect)))
		return;
	event->type = _coop_effect_on_marker;
	event->tag_index = effect_definition_index;
	event->object_index = object_index;
	event->name_index = object->object.name_index;
	event->definition_index = object->definition_index;
	event->value = marker_index;
}

boolean network_coop_skip_offered(
	void)
{
	return coop_client() && skip_vote.offered && coop_presentation.cinematic_started;
}

boolean network_coop_vote_skip(
	void)
{
	if (!coop_host() && !coop_client())
		return FALSE;
	skip_vote.voted = TRUE;
	return TRUE;
}

/* A checkpoint holds gameplay players, but the session roster and the machine
table live outside it. Keep the host's current membership when loading an older
checkpoint: a departed player must not come back, nor a late joiner disappear.
This runs before the saved-game camera and input caches are rebuilt. */
void network_coop_restore_players(void)
{
	struct network_game *game;
	struct data_iterator iterator;
	struct player_datum *player;
	short machine, local;
	long slot;

	if (!coop_host())
		return;
	game = network_game_get_game();
	if (!game || !player_data || !player_data->valid)
		return;
	for (local = 0; local < MAXIMUM_LOCAL_PLAYERS; local++)
		local_player_set_player_index(local, NONE);
	for (machine = 0; machine < HALO_PORT_MAXIMUM_NETWORK_MACHINES; machine++)
		csmemset(machine_get_player_list(machine), NONE, MAXIMUM_LOCAL_PLAYERS * sizeof(long));

	data_iterator_new(&iterator, player_data);
	while ((player = data_iterator_next(&iterator)) != NULL)
	{
		struct network_player *member;
		long unit_index;

		slot = DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index);
		member = slot < HALO_PORT_MAXIMUM_NETWORK_PLAYERS ? &game->players[slot] : NULL;
		if (network_player_is_valid(member) && member->player_list_index == slot &&
			member->machine_index == player->network_player_data.machine_index &&
			member->controller_index == player->network_player_data.controller_index)
		{
			player->network_player_data = *member;
			csmemcpy(player->name, member->name, sizeof(player->name));
			player->quit_out_of_game = FALSE;
			player->quit_out_of_game_time = NONE;
			machine_get_player_list(member->machine_index)[member->controller_index] = iterator.datum_index;
			if (network_game_player_is_local(member))
				local_player_set_player_index(member->controller_index, iterator.datum_index);
		}
		else
		{
			/* Retain a quit datum until its slot is reused, as a normal leave
			does. network_game_spawn_player then forgets all its object owners. */
			unit_index = player->unit_index;
			network_player_detach_unit(iterator.datum_index);
			if (unit_index != NONE)
				object_delete(unit_index);
			player->quit_out_of_game = TRUE;
			player->quit_out_of_game_time = NONE;
		}
	}
	for (slot = 0; slot < HALO_PORT_MAXIMUM_NETWORK_PLAYERS; slot++)
	{
		struct network_player *member = &game->players[slot];
		long player_index;

		if (!network_player_is_valid(member) || member->player_list_index != slot)
			continue;
		player = distributed_player((short)slot);
		if (player && !player->quit_out_of_game)
			continue;
		if (!network_game_spawn_player(member))
		{
			error(_error_silent, "co-op: could not restore current player %ld", slot);
			continue;
		}
		player_index = distributed_player_from_byte((byte)slot);
		if (network_game_player_is_local(member))
			local_player_set_player_index(member->controller_index, player_index);
	}
}

/* Retire queued actions and client playback from the discarded world. Keep
the host's comparison baselines: treating a restored device as newly loaded
would suppress the state clients need. Checkpoint and session metadata stay. */
void network_coop_reset_recovery(void)
{
	if (coop_client())
	{
		client_stop_script_sounds();
		client_cinematic_end();
		client_host_camera_set(FALSE);
	}
	csmemset(&coop_presentation, 0, sizeof(coop_presentation));
	coop_presentation.cutscene_subject = NONE;
	csmemset(&coop_events, 0, sizeof(coop_events));
	csmemset(&client_devices, 0, sizeof(client_devices));
	csmemset(client_names_differing, 0, sizeof(client_names_differing));
	csmemset(&host_resend, 0, sizeof(host_resend));
	csmemset(host_player_structure_bsps, NONE, sizeof(host_player_structure_bsps));
	skip_vote_clear();
}

/* The host reverted its game state, clock included. The clock is
put back, since clients and the netcode expect it to only go forward, and
the script threads' wake times move by the same amount so they still wake
when they would have. The revert also renumbered the input queues from the
old clock (update_queues_reset_and_fill_with_lies); they are renumbered
from the restored one, or no tick would run until they caught up. And the
revert is stamped with the restored time, since the script's game_reverted
compares the stamp with the clock: otherwise it would play the cinematic
it was meant to skip. */
void network_coop_reverted(
	long now)
{
	long ticks;

	if (!coop_host())
		return;
	ticks = now - game_time_get();
	if (ticks > 0)
	{
		game_time_set_distributed(now);
		update_queues_reset_and_fill_with_lies();
		game_state_port_restamp_revert_time();
		hs_runtime_port_shift_sleep_times(ticks);
	}
	/* (the dropships' riders kept are of the game state reverted from) */
	coop_enemies_reset();
	network_distributed_reverted();
	error(_error_silent, "co-op: reverted %ld ticks, clock kept at %ld", ticks, now);
	event_new(_coop_event_reverted);
}

void network_coop_skip_done(
	void)
{
	if (!coop_host())
		return;
	skip_vote_clear();
	skip_vote.requested = FALSE;
	skip_vote.cooldown_until = game_time_get() + SKIP_COOLDOWN_TICKS;
}

boolean network_coop_skip_vote_status(
	short *votes,
	short *voters,
	boolean *voted)
{
	boolean offered = coop_host() ? skip_vote.offered : network_coop_skip_offered();

	if (!offered || skip_vote.voters <= 0)
		return FALSE;
	*votes = skip_vote.votes;
	*voters = skip_vote.voters;
	*voted = skip_vote.voted;
	return TRUE;
}

/* host, after each tick */
void network_coop_host_tick(
	void)
{
	struct distributed_coop_presentation_message message;

	if (!coop_game())
		return;
	host_resend_update();
	players_vitality_keep();
	host_friendly_fire_update();
	coop_enemies_update();
	coop_pickups_update();
	coop_vehicles_update();
	host_count_skip_votes();
	host_presentation(&message.presentation);
	send_to_clients(&message, _distributed_message_coop_presentation, 1, sizeof(message.presentation));
	host_send_device_groups();
	if (host_resend.joined || game_time_get() % OBJECT_NAMES_INTERVAL_TICKS == 0)
		host_send_object_names();
	host_send_object_transforms();
	host_send_device_states();
	host_send_object_looks();
	host_send_screen_effect();
	host_send_attachments();
	host_send_looping_sounds();
	host_send_hud_state();
	host_send_broken_surfaces();
	host_send_events();
}

/* client, after each tick */
void network_coop_client_tick(
	void)
{
	/* (a multiplayer game's input and camera are its own) */
	if (!coop_client())
		return;
	client_play_presentation();
	players_vitality_keep();
	if (game_time_get() - coop_presentation.heard_time > PRESENTATION_SILENCE_TICKS)
	{
		client_cinematic_end();
		client_host_camera_set(FALSE);
		player_input_enable(TRUE);
	}
	/* sent every tick while a skip is offered, voted or not, because the
	host only counts machines it hears from */
	if (!network_coop_skip_offered())
		skip_vote.voted = FALSE;
	else
	{
		struct distributed_coop_skip_vote_message message;

		csmemset(&message.vote, 0, sizeof(message.vote));
		message.vote.voted = skip_vote.voted;
		distributed_send(&message, _distributed_message_coop_skip_vote, 1, (word)sizeof(message), _distributed_to_host);
	}
}

word network_coop_presentation_entry_size(
	void)
{
	return sizeof(struct distributed_coop_presentation);
}

word network_coop_event_entry_size(
	void)
{
	return sizeof(struct distributed_coop_event);
}

word network_coop_device_group_entry_size(
	void)
{
	return sizeof(struct distributed_coop_device_group);
}

void network_coop_note_player_structure_bsp(
	short player_index,
	short structure_bsp_index)
{
	if (player_index >= 0 && player_index < MAXIMUM_TRACKED_PLAYERS)
		host_player_structure_bsps[player_index] = structure_bsp_index;
}

boolean network_coop_player_has_structure_bsp(
	long player_index)
{
	struct player_datum *player;

	if (!network_coop_active())
		return TRUE;
	/* (no player, or none of the tracked: whose BSP is not known, which
	does not keep their predictions out) */
	player = player_index != NONE ? player_try_and_get(player_index) : NULL;
	if (!player || DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) >= MAXIMUM_TRACKED_PLAYERS)
		return TRUE;
	return player->local_player_index != NONE ||
		host_player_structure_bsps[DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index)] == global_structure_bsp_index_get();
}

word network_coop_device_state_entry_size(
	void)
{
	return sizeof(struct distributed_coop_device_state);
}

/* client: its devices where the host's are (DEVICE_POSITION_TOLERANCE) */
void network_coop_handle_device_states(
	void const *entries,
	short count)
{
	struct distributed_coop_device_state const *states = entries;
	short index;

	if (!coop_client())
		return;
	for (index = 0; index < count; index++)
	{
		struct distributed_coop_device_state const *state = &states[index];
		long device_index = object_find(state->name_index, state->object_index, state->definition_index,
			_object_mask_device);
		struct device_datum *device;
		real difference;

		if (device_index == NONE || !distributed_real_valid(state->position) || !distributed_real_valid(state->power))
			continue;
		device = object_get_and_verify_type(device_index, _object_mask_device);
		difference = device->device.position - state->position;
		if (difference < 0.0f)
			difference = -difference;
		device_port_set_state(device_index, state->power,
			(!state->moving && difference > 0.001f) || difference > DEVICE_POSITION_TOLERANCE, state->position);
	}
}

word network_coop_object_transform_entry_size(
	void)
{
	return sizeof(struct distributed_coop_object_transform);
}

void network_coop_handle_object_transforms(
	void const *entries,
	short count)
{
	struct distributed_coop_object_transform const *transforms = entries;
	short index;

	if (!coop_client())
		return;
	for (index = 0; index < count; index++)
	{
		struct distributed_coop_object_transform const *transform = &transforms[index];
		long object_index = object_find(transform->name_index, transform->object_index, transform->definition_index,
			COOP_MOVED_OBJECTS);
		real_vector3d forward, up;

		if (object_index == NONE || object_get(object_index)->object.parent_object_index != NONE)
			continue;
		distributed_vector_unpack(&transform->forward, DISTRIBUTED_UNIT_SCALE, &forward);
		distributed_vector_unpack(&transform->up, DISTRIBUTED_UNIT_SCALE, &up);
		if (distributed_point_valid(&transform->position, UNIT_WORLD_BOUND) && distributed_axes_make_valid(&forward, &up))
		{
			real_point3d before = object_get(object_index)->object.position;
			real_vector3d before_forward = object_get(object_index)->object.forward;
			real_vector3d before_up = object_get(object_index)->object.up;

			object_set_position(object_index, &transform->position, &forward, &up);
			network_objects_vehicle_written(object_index, &before, &before_forward, &before_up, _network_vehicle_write_coop);
		}
	}
}

word network_coop_object_look_entry_size(
	void)
{
	return sizeof(struct distributed_coop_object_look);
}

void network_coop_handle_object_looks(
	void const *entries,
	short count)
{
	struct distributed_coop_object_look const *looks = entries;
	short index;

	if (!coop_client())
		return;
	for (index = 0; index < count; index++)
	{
		struct distributed_coop_object_look const *look = &looks[index];
		long object_index = object_find(look->name_index, look->object_index, look->definition_index, _object_mask_all);
		struct object_datum *object = object_index != NONE ? object_get(object_index) : NULL;
		long model_index = object ? object_definition_get(object->definition_index)->object.model.index : NONE;
		struct model *model = model_index != NONE ? model_definition_get(model_index) : NULL;
		short region_index;

		if (!object)
			continue;
		/* only a permutation its model has, or none */
		for (region_index = 0; model && region_index < model->regions.count &&
			region_index < MAXIMUM_REGIONS_PER_OBJECT; region_index++)
		{
			byte permutation_index = look->region_permutations[region_index];

			if (permutation_index == (byte)NONE ||
				permutation_index < TAG_BLOCK_GET_ELEMENT(&model->regions, region_index, struct model_region)->permutations.count)
			{
				object->object.region_permutations[region_index] = permutation_index;
			}
		}
		if (distributed_real_valid(look->scale) && look->scale > 0.0f && look->scale < 100.0f &&
			look->scale != object->object.scale)
		{
			objects_scripting_set_scale(object_index, look->scale, 0);
		}
	}
}

word network_coop_object_names_entry_size(
	void)
{
	return sizeof(struct distributed_coop_object_names);
}

word network_coop_skip_vote_entry_size(
	void)
{
	return sizeof(struct distributed_coop_skip_vote);
}

/* host: a client's skip vote */
void network_coop_handle_skip_vote(
	long machine_index,
	void const *entries)
{
	struct distributed_coop_skip_vote const *vote = entries;

	if (!coop_host() || machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return;
	skip_vote.client_heard_times[machine_index] = game_time_get();
	skip_vote.client_vote_times[machine_index] = vote->voted ? game_time_get() : NONE;
}

void network_coop_handle_device_groups(
	void const *entries,
	short count)
{
	struct distributed_coop_device_group const *groups = entries;
	short index;

	if (!coop_client())
		return;
	for (index = 0; index < count; index++)
	{
		struct distributed_coop_device_group const *entry = &groups[index];
		short group_index = device_group_find(entry);
		real value, here;
		word flags;
		boolean runtime, snap;

		if (group_index == NONE || !device_group_network_get(group_index, &here, &flags, &runtime))
			continue;
		if (!distributed_real_valid(entry->value))
			continue;
		value = PIN(entry->value, 0.0f, 1.0f);
		/* Jump straight to the value when the host did, or when a rotation
		entry disagrees with us (we missed the change, or joined late).
		A change entry animates, as it did on the host. */
		snap = (client_devices.seen[group_index] && entry->snaps != client_devices.snaps[group_index]) ||
			(!entry->changed && value != here);
		if (snap || value != here || entry->flags != (byte)flags)
			device_group_network_set(group_index, value, entry->flags, snap);
		client_devices.snaps[group_index] = entry->snaps;
		client_devices.seen[group_index] = TRUE;
	}
}

/* Client: creates or deletes named scenery and devices to match the host.
A difference must show up twice in a row, on the same BSP, so a message
that crosses a BSP switch or a script's create can't cause a false one. */
void network_coop_handle_object_names(
	void const *entries)
{
	struct distributed_coop_object_names const *names = entries;
	short name_index;

	if (mcc_object_names_active()) return;

	if (!coop_client())
		return;
	if (names->structure_bsp_index != global_structure_bsp_index_get() ||
		names->name_count != MIN(global_scenario_get()->object_names.count, MAXIMUM_OBJECT_NAMES_PER_SCENARIO))
	{
		csmemset(client_names_differing, 0, sizeof(client_names_differing));
		return;
	}
	for (name_index = 0; name_index < names->name_count; name_index++)
	{
		byte *differing = &client_names_differing[name_index / 8];
		byte bit = (byte)(1 << (name_index % 8));
		boolean host_has = (names->present[name_index / 8] & bit) != 0;
		long object_index = object_index_from_name_index(name_index);
		short type = object_name_type(name_index);

		if (type < 0 || type >= NUMBER_OF_OBJECT_TYPES || TEST_FLAG(NETWORKED_OBJECT_TYPES, type) ||
			host_has == (object_index != NONE))
		{
			*differing &= (byte)~bit;
			continue;
		}
		if (!(*differing & bit))
		{
			*differing |= bit;
			continue;
		}
		*differing &= (byte)~bit;
		if (host_has)
			object_new_by_name(name_index);
		else if (!object_holds_player(object_index))
			object_delete(object_index);
	}
}

/* client: applies each event once, in order */
void network_coop_handle_events(
	void const *entries,
	short count)
{
	struct distributed_coop_event const *events = entries;
	short index;

	if (!coop_client())
		return;
	for (index = 0; index < count; index++)
	{
		/* already applied (the comparison handles the number wrapping) */
		if (coop_events.applied_any && (short)(events[index].number - coop_events.applied_number) <= 0)
			continue;
		coop_events.applied_number = events[index].number;
		coop_events.applied_any = TRUE;
		/* (not with a number that isn't one) */
		if (!distributed_real_valid(events[index].reals[0]) || !distributed_real_valid(events[index].reals[1]) ||
			!distributed_real_valid(events[index].reals[2]))
		{
			continue;
		}
		client_apply_event(&events[index]);
	}
}

/* client: keeps the host's presentation of tick host_time, to be played
in its turn (client_play_presentation) */
void network_coop_handle_presentation(
	void const *entries,
	long host_time)
{
	short slot;

	if (!coop_client() || host_time < 0)
		return;
	coop_presentation.heard_time = game_time_get();
	if (coop_presentation.playing && host_time <= coop_presentation.played_time)
		return;
	slot = (short)(host_time % PRESENTATION_BUFFER_COUNT);
	coop_presentation.buffered[slot] = *(struct distributed_coop_presentation const *)entries;
	coop_presentation.buffered_times[slot] = host_time;
	if (!coop_presentation.heard_any || host_time > coop_presentation.newest_time)
		coop_presentation.newest_time = host_time;
	coop_presentation.heard_any = TRUE;
}

static void client_presentation_apply(
	struct distributed_coop_presentation const *presentation)
{
	boolean cinematic = TEST_FLAG(presentation->flags, _presentation_cinematic_bit);
	boolean host_camera_wanted;
	boolean host_camera_unmoved;

	player_input_enable(!presentation->input_disabled);
	if (presentation->players_vitality_set && distributed_real_valid(presentation->players_maximum_body_vitality) &&
		distributed_real_valid(presentation->players_maximum_shield_vitality) &&
		presentation->players_maximum_body_vitality > 0.0f && presentation->players_maximum_shield_vitality >= 0.0f)
	{
		players_vitality.set = TRUE;
		players_vitality.maximum_body = presentation->players_maximum_body_vitality;
		players_vitality.maximum_shield = presentation->players_maximum_shield_vitality;
	}
	game_allegiance_set_teams(presentation->ally_teams, presentation->friendly_teams);
	if (!presentation->scripted_shake && player_effect_port_scripted_active())
		player_effect_port_scripted_end();

	/* a machine that joined mid-cutscene may already have started it */
	if (cinematic && !coop_presentation.cinematic_started)
	{
		if (!cinematic_in_progress())
			cinematic_start();
		coop_presentation.cinematic_started = TRUE;
	}
	else if (!cinematic)
	{
		client_cinematic_end();
	}
	/* Use the host's camera while its scripts control it, a dead player's
	too: the cutscene's camera wins over spectating (shots filmed by a moving
	Pelican follow this machine's copy of it: camera_object_offset). Spectate
	instead only when the cutscene leaves the camera with the player. A
	player with no unit and nobody to watch also gets the host's view,
	rather than a dead camera at the world origin. A camera that never moved
	(a dedicated server of an earlier build's: client_host_camera_unmoved)
	is not looked through: the cutscene's camera watches its actors instead
	(client_watch_cutscene), rather than the world's origin. */
	host_camera_wanted = (coop_presentation.cinematic_started && presentation->camera_scripted) ||
		coop_spectate_nothing_to_watch(0);
	host_camera_unmoved = host_camera_wanted && client_host_camera_unmoved(presentation);
	if (!host_camera_unmoved)
		coop_presentation.host_camera_unmoved_ticks = 0;
	else if (coop_presentation.host_camera_unmoved_ticks < TICKS_PER_SECOND &&
		++coop_presentation.host_camera_unmoved_ticks == TICKS_PER_SECOND)
	{
		error(_error_silent, "co-op: the host's camera stays at the world's origin (a dedicated server of an "
			"earlier build films no cutscene): this machine watches the cutscene's actors");
	}
	client_host_camera_set(host_camera_wanted);
	/* (also outside a cinematic: a cutscene that leaves the players their
	controls shows the bars too) */
	cinematic_show_letterbox(TEST_FLAG(presentation->flags, _presentation_letterbox_bit));
	objects_port_set_activating_cluster((short)(presentation->activating_cluster - 1));
	if (coop_presentation.host_camera && host_camera_unmoved)
	{
		client_watch_cutscene();
	}
	else if (coop_presentation.host_camera)
	{
		real_vector3d forward, up;

		distributed_vector_unpack(&presentation->camera_forward, DISTRIBUTED_UNIT_SCALE, &forward);
		distributed_vector_unpack(&presentation->camera_up, DISTRIBUTED_UNIT_SCALE, &up);
		real_point3d position = presentation->camera_position;
		struct object_datum *filmed = presentation->camera_object_index != NONE &&
			distributed_object_index_valid(presentation->camera_object_index) ?
			object_try_and_get(presentation->camera_object_index) : NULL;

		/* place the camera relative to this machine's copy of the filmed
		object, which may be a tick behind the host's */
		if (filmed && distributed_real_valid(presentation->camera_object_offset.i) &&
			distributed_real_valid(presentation->camera_object_offset.j) &&
			distributed_real_valid(presentation->camera_object_offset.k) &&
			magnitude_squared3d(&presentation->camera_object_offset) < CAMERA_OBJECT_DISTANCE * CAMERA_OBJECT_DISTANCE * 4.0f)
		{
			point_from_line3d(&filmed->object.position, &presentation->camera_object_offset, 1.0f, &position);
		}
		/* (where and how widely an observer can look: observer.c's
		match_assert_valid_observer_command) */
		if (distributed_point_valid(&position, CAMERA_WORLD_BOUND) && distributed_axes_make_valid(&forward, &up))
		{
			boolean camera_ready = TRUE;

			if (!(coop_presentation.cinematic_started && presentation->camera_scripted))
				camera_ready = client_watch_host_from_behind(&position, &forward, &up);
			if (camera_ready && valid_real_vector3d_axes2(&forward, &up))
				scripted_camera_set_camera_point_relative(&position, &forward, &up,
					PIN((real)presentation->camera_field_of_view / FIELD_OF_VIEW_SCALE, CAMERA_MINIMUM_FIELD_OF_VIEW,
						CAMERA_MAXIMUM_FIELD_OF_VIEW), 0, NONE);
		}
	}
	else
	{
		coop_presentation.watching_host_valid = FALSE;
	}

	/* start each new fade, backdated to when the host started it */
	if (presentation->fade_start_time != coop_presentation.fade_start_time)
	{
		real_rgb_color color;

		color.red = presentation->fade_color[0] / 255.0f;
		color.green = presentation->fade_color[1] / 255.0f;
		color.blue = presentation->fade_color[2] / 255.0f;
		player_effect_port_screen_fade_set(&color, presentation->fade_ticks,
			TEST_FLAG(presentation->flags, _presentation_fading_out_bit),
			game_time_get() - presentation->fade_elapsed);
		coop_presentation.fade_start_time = presentation->fade_start_time;
	}

	skip_vote.offered = TEST_FLAG(presentation->flags, _presentation_skippable_bit);
	skip_vote.votes = presentation->skip_votes;
	skip_vote.voters = presentation->skip_voters;
	client_apply_hud_state(presentation);
}

/* client, each tick: plays the next of the host's presentations. It moves
one host tick a tick, two when it has fallen behind, and waits when it has
caught up with the newest; a lost tick's place is taken by the one before. */
static void client_play_presentation(
	void)
{
	struct distributed_coop_presentation const *presentation = NULL;
	long lag, best_time = NONE;
	short index;

	if (!coop_presentation.heard_any)
		return;
	lag = coop_presentation.newest_time - coop_presentation.played_time;
	if (!coop_presentation.playing || lag > PRESENTATION_BUFFER_COUNT || lag < -PRESENTATION_BUFFER_COUNT)
		coop_presentation.played_time = coop_presentation.newest_time - PRESENTATION_DELAY_TICKS;
	else if (lag > PRESENTATION_DELAY_TICKS + 1)
		coop_presentation.played_time += 2;
	else if (lag > 0)
		coop_presentation.played_time++;
	else
		return;
	coop_presentation.playing = TRUE;
	for (index = 0; index < PRESENTATION_BUFFER_COUNT; index++)
	{
		long time = coop_presentation.buffered_times[index];

		if (time <= coop_presentation.played_time && time > coop_presentation.played_time - PRESENTATION_BUFFER_COUNT &&
			time > best_time)
		{
			best_time = time;
			presentation = &coop_presentation.buffered[index];
		}
	}
	if (presentation)
		client_presentation_apply(presentation);
}

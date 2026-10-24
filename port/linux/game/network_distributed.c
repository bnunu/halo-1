/*
NETWORK_DISTRIBUTED.C

The distributed netcode's own messages (port/linux/NETCODE.md): the game's
"data" message kind (message header type 2, which the Xbox game never sent),
beside its packets, and handled here (network_*_message_handler.c).

The host decides; its clients predict their own players and show the
rest as the host has it:

- The game's objects (units, vehicles, weapons, equipment) are the host's,
  at the same datum index on every machine (network_objects.c): the host
  says which it has, where they are and what units carry.
- Every tick, a client sends the host its own players' input, each tick's
  buttons again with the next three ticks' (a press is lost only with four
  datagrams in a row), and the host takes each tick's buttons once. The
  host sends every client the other players' input as its tick ran it, the
  same way, and the clients drive those players with it
  (player_queues_new.c).
- Every tick, a client sends the host where its own players' units are (it
  predicts them from its own input); the host takes that as they are,
  within a tolerance, as later Halo engines do, at its next tick (riding
  an elevator, moving or underfoot at rest, only across: its own elevator
  takes them up and down).
- Every tick, the host sends every client every player's unit: which unit
  the player has, alive or not, the seat it rides, its shields and health
  (down, recharging, the damage they show), and where it is (dead: who
  killed it, which a client announces when its copy dies). A client binds,
  kills, seats and places its copies to match (it decides no deaths or
  spawns itself), and its own only when far off (a respawn, a teleport).
  Players far from a client's own, or out of their sight, are sent to it
  (their units and their input) less often, but at once when they come into
  sight, and their input every tick while their buttons change.
- Twice a second, and with every kill, the host sends the players'
  statistics (kills, deaths, ...) that changed, and once a second a few
  more round them all, which clients take as they are; a machine that has
  loaded, all of them.
- The host sends the game type's state (the scores, the flags, the balls
  and the hill) when it changes (looked at five times a second), once a
  second, and to a machine that has loaded, which clients take as it is
  (game_engine_write_network_state).
- Damage is the host's: a client reports its own players' hits, which the
  host checks and deals, and replays the damage the host deals for its
  effects (network_damage.c).

The unreliable messages of a tick to a machine go in as few datagrams (a
batch) as they fill, and each carries its tick (its header's game time):
one that arrives after a newer of its kind is dropped. Vectors travel in 16
bits a part, and a unit's state and a player's input only with the parts
that are not their defaults.

Players are named by their absolute index, which is the same on every
machine (their datum identifiers need not be).
*/

#include "cseries.h"
#include "mcc_cache.h"
#include "cseries/errors.h"
#include "cache/cache_files.h"
#include "devices/devices.h"
#include "models/model_animation_definitions.h"
#include "game/game.h"
#include "game/game_globals.h"
#include "game/players.h"
#include "game/game_engine.h"
#include "main/main.h"
#include "game/player_queues_new.h"
#include "networking/network_game_globals.h"
#include "networking/network_connection.h"
#include "objects/objects.h"
#include "objects/damage.h"
#include "scenario/scenario.h"
#include "scenario/scenario_definitions.h"
#include "structures/structure_bsp_definitions.h"
#include "units/units.h"
#include "units/biped_definitions.h"
#include "units/bipeds.h"
#include "vehicle_riding.h"
#include "network_coop.h"
#include "network_distributed.h"
#include "network_link.h"
#include "network_voice.h"
#include "mcc_network.h"
#include "mcc_script_effects.h"
#include "mcc_object_names_network.h"
#include "mcc_game_speed.h"
#include "network_chat.h"
#include "network_aim_assist.h"
#include "network_shield_effects.h"
#include "network_riding_map.h"
#include "map_share.h"
#include "mcc_share.h"
#include "mcc_admission.h"
#include "mcc_load_network.h"
#include "network_afk.h"
#include "network_map_vote.h"
#include "latency_meter.h"
#include "profile_sections.h"
#include "server_director.h"

#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/* network_game_globals.c's and network_server_message_handler.c's */
boolean network_distributed_client_send(void *message, word size);
boolean network_distributed_client_send_reliably(void *message, word size);
boolean network_distributed_server_send_to_all_reliably(void *message, word size);
boolean network_distributed_server_send_to_machine(long machine_index, void *message, word size);
boolean network_distributed_server_send_to_machine_reliably(long machine_index, void *message, word size);
short network_distributed_server_machines(long *machine_indices, short maximum);
/* players.c's */
void network_player_attach_unit(long player_index, long unit_index);
void network_player_detach_unit(long player_index);
void network_player_show_pickup(long player_index, short kind, long definition_index, short count);
/* game_engine.c's */
long game_engine_write_network_state(byte *buffer, long size);
void game_engine_read_network_state(byte const *buffer, long size);
/* physics.c's (world units a tick, each tick) */
extern real global_gravity;
/* network_server_manager.c's, cseries_windows.c's, console.c's, p2p.c's */
void network_game_server_kick_machine(long machine_index, boolean kept_out);
unsigned long network_game_server_machine_address(long machine_index);
char const *network_game_server_machine_hardware_id(long machine_index);
void p2p_hardware_id_sanitize(char *destination, int size, const char *source);
void p2p_discord_sanitize(char *destination, int size, const char *source, int name);
void p2p_discord_identity(char *id, int id_size, char *name, int name_size);
unsigned long p2p_peer_endpoint_address(unsigned long virtual_address);
int p2p_peer_endpoint_text(unsigned long virtual_address, char *text, int size);
/* network_connection.c's */
boolean network_connection_last_read_was_unreliable(void);
unsigned long system_milliseconds(void);
void platform_log(char const *format, ...);
/* main.c's and port_mods.c's: this build, and the mods it plays */
void main_build_identity(long *number, char *flavor, size_t flavor_size, char *repository, size_t repository_size,
	char *version, size_t version_size);
int port_mods_list(char *text, unsigned long size);
void console_warning(const char *format, ...);

/* the host: a client's game run faster than its clock (a speed hack, which
speeds up the machine's own clock: nothing on it can tell), found by its
ticks, which its messages are stamped with, going by faster than the
host's own clock's time, and ahead of the host's (a client's clock never
is: it starts at the host's, and recovers toward reliable host time only when
behind; a host that stalled has every client ahead, their ticks going by as
fast as time). Measured over each window this long (milliseconds)... */
#define CLIENT_CLOCK_WINDOW_MILLISECONDS 2000
/* ... faster than this many times as fast as time, and this many ticks
ahead of the host at its end: its players' predictions not taken (the
host's copies go as its own ticks have them) ... */
#define CLIENT_CLOCK_FAST_RATE 1.1f
#define CLIENT_CLOCK_AHEAD_TICKS 15
/* ... and so many windows in a row, dropped */
#define CLIENT_CLOCK_FAST_WINDOWS 5
/* the longest notice's text (_distributed_message_notice) */
#define MAXIMUM_NOTICE_LENGTH 160
/* how far a datagram's tick may be from the latest a message from its
sender had, either way (distributed_sender_times): as long as a machine may
go silent before it is dropped */
#define DISTRIBUTED_TIME_WINDOW_TICKS (15 * TICKS_PER_SECOND)
/* a Discord user's id and name as kept, with their ends (p2p.h's
P2P_DISCORD_ID_SIZE and P2P_DISCORD_NAME_SIZE) */
#define DISCORD_ID_SIZE 24
#define DISCORD_NAME_SIZE 40
/* where the host logs the players it dropped for cheating, a line each */
#define CHEATERS_FILE "d:\\cheaters.txt"
/* ... and those it bans (by hand, and cheaters): their addresses refused */
#define BANS_FILE "d:\\bans.txt"

/* a client's Discord user, as told (_distributed_message_client_identity) */
struct distributed_client_identity
{
	char discord_id[DISCORD_ID_SIZE];
	char discord_name[DISCORD_NAME_SIZE];
};

enum
{
	STATISTICS_INTERVAL_TICKS = 15,
	/* the players' statistics sent unchanged once a second, round them all
	(the message is unreliable) */
	STATISTICS_REFRESH_TICKS = TICKS_PER_SECOND,
	STATISTICS_REFRESH_PLAYERS = 16,
	/* the game type's state looked at, and sent unchanged */
	GAME_STATE_INTERVAL_TICKS = 6,
	/* ... when it changes (reliably: no refresh), no more often than this
	(the king's and the ball's scores change every tick), but the game's end
	at once */
	GAME_STATE_MINIMUM_TICKS = 2 * GAME_STATE_INTERVAL_TICKS,
	MAXIMUM_GAME_STATE_SIZE = 0xF00,
	MAXIMUM_STATISTICS_PER_MESSAGE = 64,
	/* the players' pings sent, for the scoreboard */
	PING_INTERVAL_TICKS = 2 * TICKS_PER_SECOND,
	MAXIMUM_PINGS_PER_MESSAGE = 128,
	/* a ping not known */
	UNKNOWN_PING = 0xFFFF,
	MAXIMUM_PICKUPS_PER_TICK = 64,
	/* ticks a client's own player may ride where the host says it does not
	(or the other way round) before it is put where the host has it: its
	own prediction reaches the host and comes back in about a round trip */
	SEAT_DISAGREEMENT_TICKS = 15,
	/* how long a client waits for a player's killing blow once the host's
	states say the player died, before the unit dies without it (an
	actor's waits as long: network_objects.c) */
	DEATH_BLOW_WAIT_TICKS = TICKS_PER_SECOND / 2,
	/* the machines, and the host: a client's messages' sender */
	MAXIMUM_SENDERS = HALO_PORT_MAXIMUM_NETWORK_MACHINES + 1,
	HOST_SENDER = HALO_PORT_MAXIMUM_NETWORK_MACHINES,
	/* a round trip before one is measured, and the longest taken (ticks) */
	DEFAULT_ROUND_TRIP_TICKS = 6,
	MAXIMUM_ROUND_TRIP_TICKS = 60,
	/* ... and beyond it, the ticks a client's own player may ride otherwise
	than the host has it */
	SEAT_DISAGREEMENT_SLACK_TICKS = 6,
	/* the latency meter (latency_meter.c): the same round trips timed in
	milliseconds, by when each of the last ticks was sent (a power of two,
	more than the longest round trip); the longest shown; how often what is
	shown is refreshed; and how long the host (to a client) or a client (to
	the host) says nothing before it is said to have a connection problem */
	TICK_SENT_TIMES = 64,
	MAXIMUM_PING_MILLISECONDS = 9999,
	PING_REFRESH_TICKS = TICKS_PER_SECOND / 2,
	CONNECTION_PROBLEM_MILLISECONDS = 2000,
	/* a client: where its own players' units were, the last ticks (a power
	of two, more than the longest round trip) */
	OWN_POSITION_TICKS = 64,
	/* the host: the ticks after it took a client's player's position that it
	still tells the client which of its ticks it has them at */
	PREDICTION_ECHO_TICKS = 3,
	/* ... and the client's ticks it takes as run since the anchor beyond
	the host's (its messages delayed, then bunched) */
	PREDICTION_JITTER_TICKS = 6,
	/* ... and the most of a client's round trip it takes a client's player
	in the air to be ahead of its copy by (distributed_on_foot_ceiling) */
	PREDICTION_CEILING_LEAD_TICKS = 15,
	/* ... how long it measures predictions from one it took, the anchor,
	before it takes a newer as the anchor (the jitter and the blend granted
	once a second, not once a tick) */
	PREDICTION_ANCHOR_TICKS = TICKS_PER_SECOND,
	/* ... and how long it remembers how fast its own copy of a client's
	player went (an anchor's second, and the client's round trip: a client
	learns of an explosion that throws its player a round trip late) */
	PREDICTION_SPEED_TICKS = PREDICTION_ANCHOR_TICKS + MAXIMUM_ROUND_TRIP_TICKS,
};

/* struct distributed_unit_state flags */
enum
{
	/* the player has a unit that is alive */
	_distributed_unit_alive_bit = 0,
	/* ... and not riding (its position is its own) */
	_distributed_unit_placed_bit,
	/* (dead) how it died: killed by a teammate, or by an empty vehicle */
	_distributed_unit_friendly_fire_bit,
	_distributed_unit_killed_by_vehicle_bit,
	/* (alive) its shields: down, charging, overcharging */
	_distributed_unit_shield_depleted_bit,
	_distributed_unit_shield_charging_bit,
	_distributed_unit_shield_over_charging_bit,
	/* (alive) the host has the unit where the client's own prediction had
	it at the tick in predicted_time */
	_distributed_unit_predicted_bit,
};

/* struct distributed_unit_state unit flags */
enum
{
	/* (alive) camouflaged, and doubly so */
	_distributed_unit_camouflaged_bit = 0,
	_distributed_unit_super_camouflaged_bit,
	_distributed_unit_suspended_bit,
};

/* a unit state's parts on the wire: which of those that are not always
there it has (distributed_unit_state_write); one it has not is its default */
enum
{
	/* (alive) the vehicle it rides and the seat */
	_distributed_unit_part_riding_bit = 0,
	/* (alive, placed) its up, when not straight up */
	_distributed_unit_part_up_bit,
	/* (alive) the damage its shields and body show, when any */
	_distributed_unit_part_damage_bit,
	/* (alive) how long its powerups have left, when any */
	_distributed_unit_part_powerups_bit,
	/* (alive) its camouflage, when any */
	_distributed_unit_part_camouflage_bit,
	/* (dead) who killed it, when anyone */
	_distributed_unit_part_killer_bit,
	/* (alive) host-controlled facing, aim and look while input is inhibited */
	_distributed_unit_part_scripted_pose_bit,
	/* (alive, placed) where on a vehicle's deck it is, in that vehicle's
	window (vehicle_riding.h's struct vehicle_riding_deck) */
	_distributed_unit_part_deck_bit,
	NUMBER_OF_DISTRIBUTED_UNIT_PARTS
};

/* the parts are the bits of one byte on the wire: a ninth would be lost without a word */
typedef char distributed_unit_parts_assert[NUMBER_OF_DISTRIBUTED_UNIT_PARTS <= 8 ? 1 : -1];

enum
{
	_distributed_pose_facing,
	_distributed_pose_aiming,
	_distributed_pose_looking,
	_distributed_pose_current_aiming,
	_distributed_pose_current_looking,
	NUMBER_OF_DISTRIBUTED_POSE_VECTORS
};

/* world units: how far apart the host's run of a client's player and the
client's may be, a tick from the same place, and agree (distributed_divergence:
well past the last bits' rounding, well short of a player running 1% faster) */
#define DIVERGENCE_APART_DISTANCE 0.0001f

/* world units: how far a client's own player's unit may be from the host's
before the host takes it no longer, and before the client is put where the
host has it (no further than that: between the two they would disagree for
good, the host's player somewhere its own is not) */
#define HOST_ACCEPT_TOLERANCE 3.5f
#define LOCAL_CORRECTION_TOLERANCE 3.0f
/* world units: how far below or above the host's echo of its prediction a
client's own player standing on the same elevator may be and keep its own
floor's height (distributed_correct_own_unit). The two machines' elevators
are apart by what the platform travels in a round trip (the history below
holds 64 ticks, two seconds); a greater difference is not that lag but a
teleport, a fall or a platform out of step, and is corrected. */
#define ELEVATOR_PHASE_TOLERANCE 4.0f
/* how fast a client's own player's unit moves at most (world units a tick) */
#define MAXIMUM_PREDICTED_SPEED 2.0f
/* ... on foot: this many times as fast as a player runs and jumps, or as
fast as the host's ticks threw its copy lately, whichever is more; and how
much further a tick than that it may be (world units a tick) */
#define PREDICTED_ON_FOOT_SPEED_SCALE 2.0f
#define PREDICTED_ON_FOOT_SPEED_MARGIN 0.05f
/* ... and how much higher than a jump takes it above where the host last
had it on the ground it may be (world units: steps climbed in the ticks a
prediction is late, where the host and the client have the ground) */
#define PREDICTED_RISE_TOLERANCE 1.0f

/* how far players are from a client's own (world units) before the host
sends them to it (their units and their input) every second tick, every
third, and every fourth; one no cluster of the client's players' can see
(the map's potentially visible set), or dead, every sixth. The set errs on
the side of seeing: a player comes into view in it before any line of sight
does, and is sent at once when they do, as a player whose buttons have
changed in the ticks their input carries is sent every tick. A player a client's player
aims near is sent at least every second tick, and every tick through a
scope (a sniper sees a far player as well as a near one, as Ares does).
(Cosines of the half angles.) */
#define NEAR_PLAYER_DISTANCE 25.0f
#define MIDDLE_PLAYER_DISTANCE 60.0f
#define FAR_PLAYER_DISTANCE 120.0f
#define AIMED_AT_COSINE 0.819f /* 35 degrees: on the screen */
#define SCOPED_AT_COSINE 0.940f /* 20 degrees: in a scope's view, and round it */
enum
{
	HIDDEN_PLAYER_PERIOD_TICKS = 6,
};

/* shields, health and the damage they show in 16 bits: 0 to 4 */
#define VITALITY_SCALE 16384.0f

/* a player's unit, as this machine has it (its bytes on the wire are fewer:
distributed_unit_state_write) */
struct distributed_unit_state
{
	byte player_index;
	byte flags;
	/* (dead) the player who killed it, NO_PLAYER for none */
	byte killing_player_index;
	byte unit_flags;
	/* the player's unit (the host's), NONE for none */
	long unit_index;
	/* the vehicle it rides and its seat, NONE for none */
	long vehicle_index;
	short seat_index;
	/* how camouflaged the unit is, of 255 */
	byte active_camouflage;
	byte pad;
	real_point3d position;
	struct distributed_vector velocity;
	struct distributed_vector forward;
	struct distributed_vector up;
	/* (_distributed_unit_predicted_bit) the client's tick, its low 16 bits */
	short predicted_time;
	/* (VITALITY_SCALE) */
	word body_vitality;
	word shield_vitality;
	/* what the shields' and the HUD's effects show */
	word current_body_damage;
	word recent_body_damage;
	word current_shield_damage;
	word recent_shield_damage;
	/* the player's powerups: how long each has left */
	short powerup_durations[NUMBER_OF_PLAYER_POWERUPS];
	boolean scripted_pose;
	struct distributed_vector pose[NUMBER_OF_DISTRIBUTED_POSE_VECTORS];
	/* (alive, placed) its player in a vehicle's window: the deck part, when
	deck_present (never on the wire: distributed_deck_put) */
	boolean deck_present;
	struct vehicle_riding_deck deck;
};

/* a client's player's input at one of its ticks, with the buttons of the
ticks before it */
struct distributed_player_input
{
	byte player_index;
	/* co-op: the structure BSP the client has loaded */
	byte structure_bsp_index;
	/* co-op: the player chose to play without friendly fire */
	byte coop_no_friendly_fire;
	byte pad;
	/* the client's tick */
	long tick;
	/* the latest host tick the client has had a message of (the host
	measures the round trip by it) */
	long host_time;
	struct player_action action;
	unsigned short control_flags[DISTRIBUTED_INPUT_HISTORY];
};

typedef char distributed_player_input_size_assert[
	sizeof(struct distributed_player_input) == 12 + 0x20 + 2 * DISTRIBUTED_INPUT_HISTORY ? 1 : -1];
typedef char distributed_player_input_action_offset_assert[
	offsetof(struct distributed_player_input, action) == 12 ? 1 : -1];

/* a player's input as the host ran it, as this machine has it: the host's
update (the same for every player of a tick) goes once a message, and on
the wire the ticks' buttons before the latest only where they differ from
the tick after (distributed_relayed_action_write) */
struct distributed_relayed_action
{
	byte player_index;
	signed char desired_weapon_index;
	signed char desired_grenade_index;
	signed char desired_zoom_level;
	unsigned short control_flags[DISTRIBUTED_INPUT_HISTORY];
	/* of a turn */
	short yaw;
	short pitch;
	/* of 127, and 255 */
	signed char throttle_i;
	signed char throttle_j;
	byte primary_trigger;
};

/* a relayed action's parts on the wire: which of the ticks' buttons before
the latest it has (bit h - 1 for tick h), the others those of the tick
after */
typedef char distributed_relayed_action_parts_assert[DISTRIBUTED_INPUT_HISTORY - 1 <= 8 ? 1 : -1];

enum
{
	/* a unit state's bytes on the wire: its player, flags and parts, the
	most it has, and a relayed action's (past the update each message has
	first) */
	DISTRIBUTED_UNIT_STATE_MINIMUM_SIZE = 3,
	/* (its vehicle, node, place, velocity, ticks, flags and peak) */
	DISTRIBUTED_DECK_SIZE = 4 + 2 + 12 + 6 + 1 + 1 + 2,
	DISTRIBUTED_UNIT_STATE_MAXIMUM_SIZE = 3 + 4 + (4 + 2) + (12 + 3 * 6) + DISTRIBUTED_DECK_SIZE + 2 + 2 * 2 + 4 * 2 +
		2 * NUMBER_OF_PLAYER_POWERUPS + 2 + 1 + NUMBER_OF_DISTRIBUTED_POSE_VECTORS * 6,
	DISTRIBUTED_RELAYED_ACTION_MINIMUM_SIZE = 14,
	DISTRIBUTED_RELAYED_ACTION_MAXIMUM_SIZE = 14 + 2 * (DISTRIBUTED_INPUT_HISTORY - 1),
};

struct distributed_pickup
{
	byte player_index;
	byte kind;
	short count;
	long definition_index;
};

typedef char distributed_pickup_size_assert[sizeof(struct distributed_pickup) == 8 ? 1 : -1];

struct distributed_player_statistics
{
	short player_index;
	short pad;
	struct game_statistics statistics;
};

typedef char distributed_player_statistics_size_assert[
	sizeof(struct distributed_player_statistics) == 4 + sizeof(struct game_statistics) ? 1 : -1];

struct distributed_statistics_message
{
	struct distributed_message_header header;
	struct distributed_player_statistics players[MAXIMUM_STATISTICS_PER_MESSAGE];
};

/* a player's ping, in milliseconds (UNKNOWN_PING: not known) */
struct distributed_player_ping
{
	byte player_index;
	byte pad;
	word milliseconds;
};

typedef char distributed_player_ping_size_assert[sizeof(struct distributed_player_ping) == 4 ? 1 : -1];

struct distributed_pings_message
{
	struct distributed_message_header header;
	struct distributed_player_ping players[MAXIMUM_PINGS_PER_MESSAGE];
};

/* a batch's datagram: its header, then each message's size and its bytes
past its message header */
struct distributed_batch
{
	word size;
	byte data[DATAGRAM_MAXIMUM_SIZE];
};

/* what each message in a batch takes besides its entries: its size, and
its header but for its message header */
#define BATCH_MESSAGE_OVERHEAD \
	((word)(sizeof(word) + sizeof(struct distributed_message_header) - sizeof(message_header)))

/* messages of entries of a size of their own, to one machine (or the host),
each as long as the batch has room for (distributed_packer_add): nothing
else goes into that batch while it packs */
struct distributed_packer
{
	short sender;
	byte type;
	short count;
	/* the bytes every message has first (the relayed actions' update), and
	the bytes past the header so far, of room */
	word prefix_size;
	word size;
	word room;
	struct
	{
		struct distributed_message_header header;
		byte data[DATAGRAM_MAXIMUM_SIZE];
	} message;
};

/* ---------- globals */

static long distributed_last_sent_time = NONE;

/* the tick's steps, timed in the profiling build */
PROFILE_SECTION(distributed_tick_apply_predictions_section, "network_distributed_tick.apply_predictions")
PROFILE_SECTION(distributed_tick_apply_vehicle_predictions_section, "network_distributed_tick.apply_vehicle_predictions")
PROFILE_SECTION(distributed_tick_objects_host_tick_section, "network_distributed_tick.objects_host_tick")
PROFILE_SECTION(distributed_tick_plan_players_section, "network_distributed_tick.plan_players")
PROFILE_SECTION(distributed_tick_damage_host_tick_section, "network_distributed_tick.damage_host_tick")
PROFILE_SECTION(distributed_tick_send_statistics_section, "network_distributed_tick.send_statistics")
PROFILE_SECTION(distributed_tick_send_pings_section, "network_distributed_tick.send_pings")
PROFILE_SECTION(distributed_tick_send_structure_bsp_section, "network_distributed_tick.send_structure_bsp")
PROFILE_SECTION(distributed_tick_send_players_section, "network_distributed_tick.send_players")
PROFILE_SECTION(distributed_tick_actors_host_tick_section, "network_distributed_tick.actors_host_tick")
PROFILE_SECTION(distributed_tick_coop_host_tick_section, "network_distributed_tick.coop_host_tick")
PROFILE_SECTION(distributed_tick_send_pickups_section, "network_distributed_tick.send_pickups")
PROFILE_SECTION(distributed_tick_send_game_state_section, "network_distributed_tick.send_game_state")
PROFILE_SECTION(distributed_tick_note_own_positions_section, "network_distributed_tick.note_own_positions")
PROFILE_SECTION(distributed_tick_send_inputs_section, "network_distributed_tick.send_inputs")
PROFILE_SECTION(distributed_tick_send_predictions_section, "network_distributed_tick.send_predictions")
PROFILE_SECTION(distributed_tick_objects_client_tick_section, "network_distributed_tick.objects_client_tick")
PROFILE_SECTION(distributed_tick_damage_client_tick_section, "network_distributed_tick.damage_client_tick")
PROFILE_SECTION(distributed_tick_coop_client_tick_section, "network_distributed_tick.coop_client_tick")
PROFILE_SECTION(distributed_tick_batches_flush_section, "network_distributed_tick.batches_flush")

/* how each player last died, by absolute index: the host's own, which it
sends its clients, and a client's copy of the host's */
static struct distributed_death
{
	boolean valid;
	/* the killer's absolute index, or NONE */
	short killing_player_index;
	boolean friendly_fire;
	boolean killed_by_vehicle;
	/* a client: when the host's states first said the player's living unit
	here was dead (NONE: not), to wait for the killing blow
	(DEATH_BLOW_WAIT_TICKS) */
	long dead_since;
} distributed_deaths[MAXIMUM_TRACKED_PLAYERS];
/* the host: a kill this tick, whose statistics the clients should have
with it */
static boolean distributed_statistics_due;
/* the host: what players on other machines picked up this tick */
static struct distributed_pickup distributed_pickups[MAXIMUM_PICKUPS_PER_TICK];
static short distributed_pickup_count;
/* a client: the ticks each of its own players has ridden other than as
the host has it */
static short distributed_seat_disagreements[MAXIMUM_TRACKED_PLAYERS];

/* the host: each client's player's latest prediction, taken at the next
tick, and the client's tick of the one taken last and when */
static struct
{
	boolean valid;
	long time;
	struct distributed_unit_state state;
	long taken_time;
	long taken_host_time;
} distributed_predictions[MAXIMUM_TRACKED_PLAYERS];
/* the host: the anchor, where it took each client's player to be (one
taken, a newer once a second: the next may be no further from it than the
player moves in the client's ticks since), at which of the client's ticks
and its own; and when it last took one, and where that left its unit */
static struct distributed_accepted_state
{
	boolean valid;
	long unit_index;
	long time;
	long host_time;
	real_point3d position;
	long taken_host_time;
	real_point3d taken_host_position;
	long vehicle_index;
	real_point3d place;
	real_point3d taken_host_place;
	boolean transition_sampled;
	boolean transition_client_spent;
	long transition_unit_index;
	long transition_until;
	short transition_host_ticks;
	short transition_client_ticks;
	long transition_host_vehicle;
	long transition_client_vehicle;
	long transition_leash_time;
	long transition_board_vehicle;
	long transition_board_until;
	long taken_air_unit_index;
	long taken_air_vehicle;
} distributed_accepted[MAXIMUM_TRACKED_PLAYERS];
/* fixed before either pass, since taking a word clears its valid bit */
static boolean distributed_rider_pass[MAXIMUM_TRACKED_PLAYERS];
/* ... what its own ticks did to its copy of each client's player's unit on
foot, beyond the client's word it took (an explosion's throw, a jump): how
fast they sent it, and how fast up, the most of this span of
PREDICTION_SPEED_TICKS and of the one before; the unit (NONE: none on
foot), the velocity it took (what its next tick started from) and whether
the unit was on the ground then; where it last had the unit on the ground
(or where it started: a new life, a ride's end, a teleporter) and the
highest it has had it since; and those speeds kept through a flight begun
while it had them (a throw's flight outlasting the span) */
static struct distributed_host_speed
{
	long time;
	real speeds[2];
	real rises[2];
	real flight_speed;
	real flight_rise;
	long unit_index;
	real_vector3d velocity;
	real_vector3d frame_velocity;
	boolean window;
	long vehicle_index;
	boolean grounded;
	real ground_height;
	real ground_frame_k;
	real ground_place_height;
	real top_height;
	/* the last tick it had the unit on the ground (or where it started) */
	long ground_time;
} distributed_host_speeds[MAXIMUM_TRACKED_PLAYERS];
/* a client: where each of its own players' units was at its last ticks,
and how long the host takes to have them (ticks) */
static struct distributed_own_position
{
	long time;
	long unit_index;
	real_point3d position;
	/* local history only: NONE after jumping or walking off the platform */
	long elevator_index;
	boolean deck_present;
	struct vehicle_riding_deck deck;
	long window;
	long air_episode;
	boolean air_baseline_valid;
} distributed_own_positions[MAXIMUM_LOCAL_PLAYERS][OWN_POSITION_TICKS];
static long distributed_own_windows[MAXIMUM_LOCAL_PLAYERS];
static long distributed_own_air_episodes[MAXIMUM_LOCAL_PLAYERS];
static real distributed_own_round_trip;
/* the host: the players each machine had when it last told the host that it
had loaded (a machine new at its index has none of the old one's state) */
static long distributed_machine_players[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_LOCAL_PLAYERS];
/* the host: what each player's unit was last sent as (a change goes to
every client at once) */
static struct
{
	byte flags;
	long unit_index;
	long vehicle_index;
	short seat_index;
} distributed_sent_units[MAXIMUM_TRACKED_PLAYERS];

/* the host: whether each client's players could see each player last tick
(one who comes into sight is sent at once) */
static boolean distributed_seen[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_TRACKED_PLAYERS];
/* the host: where each client's players were when last alive (a client
whose players are dead watches from there) */
static struct distributed_viewer
{
	boolean valid;
	short cluster_index;
	real_point3d position;
	real_vector3d aim;
} distributed_viewers[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_LOCAL_PLAYERS];
/* the host, this tick: every player's unit and input, what each client is
sent of each player (decided before the damage, which goes where the
players it hurt do), and the client each player is on (NONE for none: the
host's own) */
enum
{
	_distributed_send_state_bit = 0,
	_distributed_send_action_bit,
	/* the client's players can see the player (whether its state goes this
	tick or not): what hurts the player it is told of */
	_distributed_send_sees_bit,
};
static struct distributed_unit_state distributed_host_states[MAXIMUM_TRACKED_PLAYERS];
static struct distributed_relayed_action distributed_host_actions[MAXIMUM_TRACKED_PLAYERS];
static long distributed_host_update_number;
static byte distributed_sends[HALO_PORT_MAXIMUM_NETWORK_MACHINES][MAXIMUM_TRACKED_PLAYERS];
static long distributed_player_machines[MAXIMUM_TRACKED_PLAYERS];
/* the host: each player's statistics as last sent (only a change is sent,
and once a second a few players' whatever they are, round them all) */
static unsigned long distributed_sent_statistics[MAXIMUM_TRACKED_PLAYERS];
static short distributed_statistics_cursor;
/* the host: the game type's state as last sent (its checksum and its
leading postgame state), and when */
static unsigned long distributed_game_state_checksum;
static long distributed_game_state_postgame;
static long distributed_game_state_time;

/* the latest tick of each kind of unreliable message had from each sender
(a machine, or the host), NONE for none */
static long distributed_received_times[MAXIMUM_SENDERS][NUMBER_OF_DISTRIBUTED_MESSAGES];
/* the latest tick of any message had from each sender, NONE for none: a
datagram, which anyone can send as from another machine, is dropped when
its tick is further from it than DISTRIBUTED_TIME_WINDOW_TICKS (one stamped
far ahead would have every newer one of the sender's taken for stale, and
its clock for fast); a message over the sender's stream, which only it can
send, is taken whatever its tick, and moves it
(distributed_message_time_bounded) */
static long distributed_sender_times[MAXIMUM_SENDERS];
/* Outside saved game state. A restored handle may name a new life; packets
from the world before the restore must not act on it. Zero means that a
client is still waiting for the host's ordered boundary. */
static unsigned long distributed_checkpoint_epoch;
static unsigned long distributed_host_epoch;
static boolean distributed_checkpoint_synchronized;
/* the host: each client machine's clock, measured (CLIENT_CLOCK_WINDOW_MILLISECONDS):
its latest tick, and its tick and the host's time as the window began
(NONE: none begun); how many windows in a row it went fast, and whether
the last did; and whether, since they began, a message that came over its
stream was that far ahead too (a datagram's source address is all that
says which machine sent it, and anyone can send one as from another's: a
machine is banned only on its stream's word, distributed_note_client_clock) */
/* the host: each client machine's Discord user, as it told it, the text
kept only of what is allowed (p2p_discord_sanitize) */
static struct distributed_client_identity distributed_client_identities[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
/* a client: its Discord user as last sent this game, and whether it was */
static struct distributed_client_identity distributed_sent_identity;
/* the host: each client's build, as it told it, and whether it did; a
client: whether it sent its own this game */
static struct distributed_client_build distributed_client_builds[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
static boolean distributed_client_build_known[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
static boolean distributed_build_sent;
static boolean distributed_identity_sent;
static struct distributed_client_clock
{
	long latest_tick;
	long window_tick;
	unsigned long window_time;
	short fast_windows;
	boolean fast;
	boolean ahead_on_stream;
	/* (its datagrams' word logged, unverified, while they go on saying so) */
	boolean logged_unverified;
} distributed_client_clocks[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
/* the host: whether the message being handled came over its machine's
stream (network_distributed_handle_stream_message) */
static boolean distributed_handling_stream_message;
/* a client: the host's latest tick it has had a message of, and when that
message came (system_milliseconds) */
static long distributed_host_time = NONE;
static unsigned long distributed_host_time_received;
/* the host: each client's round trip, in ticks, and its jitter; the same
in milliseconds (the latency meter's); and when the client was last heard
from (system_milliseconds) */
static struct
{
	boolean valid;
	real average;
	real deviation;
	boolean milliseconds_valid;
	real milliseconds;
	unsigned long heard;
	/* (logged: silent since when it was heard, 0 for not silent) */
	unsigned long silent_since;
} distributed_round_trips[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
/* the latency meter: when each of this machine's last ticks was sent
(system_milliseconds), by tick; a round trip coming back naming one of
them is timed by it */
static struct
{
	long time;
	unsigned long milliseconds;
} distributed_tick_sent_times[TICK_SENT_TIMES];
/* ... a client: its own round trip in milliseconds, as the host has its
players and tells it back, and whether it is measured; and since when the
host has been silent (logged), 0 for not silent */
static boolean distributed_own_ping_valid;
static real distributed_own_ping_milliseconds;
static unsigned long distributed_host_silent_since;
/* ... what is shown (refreshed in the tick every PING_REFRESH_TICKS, so
the render reads it whole): each player's round trip as this machine times
it (the host: each client's, its own players' 0; a client: its own
players'), a client's own, and the host's slowest client's; NONE for none */
static long distributed_shown_pings[MAXIMUM_TRACKED_PLAYERS];
static long distributed_shown_own_ping = NONE;
static long distributed_shown_slowest_ping = NONE;
/* a client: each player's ping as the host last told it (UNKNOWN_PING: not
told) */
static word distributed_player_pings[MAXIMUM_TRACKED_PLAYERS];

/* the host: its clients' machines, found once a tick */
static struct
{
	boolean in_tick;
	boolean valid;
	short count;
	long indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
} distributed_machines;

/* the unreliable messages of this tick, a batch for each machine and one
for the host */
static struct distributed_batch distributed_batches[MAXIMUM_SENDERS];
/* (the largest messages, packed a machine at a time) */
static struct distributed_packer distributed_packer;

/* for the automated tests' reports (network_test.c) */
static struct
{
	long sent;
	long received;
	long corrections;
} distributed_statistics;
/* ... and (the host) how far apart a client's player moved in a tick on the
client and on the host, where the host took the client's last prediction
the tick before, that was of the client's tick before, and the host's tick
ran the player with the client's input of that tick: both machines then
moved the player one tick with the same input from much the same place, so
builds that simulate alike move it alike to the last bit (but where the
player touched something the two have in different places), and a build
that moves a player otherwise moves it otherwise, every tick the player
moves (tools/net_compare.py). The move, not where it ends: the host's
copy is put only halfway to a prediction a little off, and would be as far
off the next for ticks after. Since network_distributed_divergence last
read them: how many such ticks, how many of them further apart than
DIVERGENCE_APART_DISTANCE, and the mean and the largest distance. */
static struct
{
	long samples;
	long apart;
	real total;
	real largest;
	/* the tick of the client's and of the host's last prediction taken */
	long taken_time[MAXIMUM_TRACKED_PLAYERS];
	long taken_host_time[MAXIMUM_TRACKED_PLAYERS];
	/* where that prediction had the player, and where it left the host's */
	real_point3d taken_position[MAXIMUM_TRACKED_PLAYERS];
	real_point3d taken_host_position[MAXIMUM_TRACKED_PLAYERS];
	/* the client's tick of the latest input the host has of each player
	(what the host's tick ran it with) */
	long input_time[MAXIMUM_TRACKED_PLAYERS];
} distributed_divergence;

/* ---------- shared (network_distributed.h) */

void network_distributed_statistics(
	long *sent,
	long *received,
	long *corrections)
{
	*sent = distributed_statistics.sent;
	*received = distributed_statistics.received;
	*corrections = distributed_statistics.corrections;
}

void distributed_count_correction(
	void)
{
	distributed_statistics.corrections++;
}

void network_distributed_divergence(
	long *samples,
	long *apart,
	real *mean,
	real *largest)
{
	*samples = distributed_divergence.samples;
	*apart = distributed_divergence.apart;
	*mean = distributed_divergence.samples ? distributed_divergence.total / (real)distributed_divergence.samples : 0.0f;
	*largest = distributed_divergence.largest;
	distributed_divergence.samples = 0;
	distributed_divergence.apart = 0;
	distributed_divergence.total = 0.0f;
	distributed_divergence.largest = 0.0f;
}

boolean distributed_real_valid(
	real value)
{
	/* (NaN and the infinities less themselves are not 0) */
	return value - value == 0.0f;
}

boolean distributed_point_valid(
	real_point3d const *point,
	real bound)
{
	return fabsf(point->x) <= bound && fabsf(point->y) <= bound && fabsf(point->z) <= bound;
}

/* whether a tag index from the host really is a tag of that group */
boolean distributed_tag_of_group(
	long tag_index,
	unsigned long group_tag)
{
	struct tag_iterator iterator;
	long index;

	tag_iterator_new(&iterator, group_tag);
	while ((index = tag_iterator_next(&iterator)) != NONE)
	{
		if (index == tag_index)
			return TRUE;
	}

	return FALSE;
}

/* the graph's animation, if the graph tag and index are valid */
struct animation *distributed_graph_animation(
	long animation_graph_index,
	short animation_index)
{
	struct animation_graph *graph;

	if (!distributed_tag_of_group(animation_graph_index, ANIMATION_GRAPH_TAG))
		return NULL;
	graph = animation_graph_definition_get(animation_graph_index);
	if (animation_index < 0 || animation_index >= graph->animations.count)
		return NULL;

	return TAG_BLOCK_GET_ELEMENT(&graph->animations, animation_index, struct animation);
}

boolean distributed_object_index_valid(
	long object_index)
{
	return object_index != NONE && DATUM_INDEX_TO_IDENTIFIER(object_index) != 0;
}

boolean distributed_axes_make_valid(
	real_vector3d *forward,
	real_vector3d *up)
{
	real forward_length = (real)sqrt(forward->i * forward->i + forward->j * forward->j + forward->k * forward->k);
	real up_length = (real)sqrt(up->i * up->i + up->j * up->j + up->k * up->k);
	real dot;
	real length;

	if (!(forward_length > 0.5f && forward_length < 1.5f && up_length > 0.5f && up_length < 1.5f))
		return FALSE;
	forward->i /= forward_length;
	forward->j /= forward_length;
	forward->k /= forward_length;
	dot = (forward->i * up->i + forward->j * up->j + forward->k * up->k) / up_length;
	if (!(fabsf(dot) < 0.1f))
		return FALSE;
	/* (up square to forward) */
	up->i -= dot * up_length * forward->i;
	up->j -= dot * up_length * forward->j;
	up->k -= dot * up_length * forward->k;
	length = (real)sqrt(up->i * up->i + up->j * up->j + up->k * up->k);
	up->i /= length;
	up->j /= length;
	up->k /= length;
	return TRUE;
}

void distributed_vector_pack(
	real_vector3d const *vector,
	real scale,
	struct distributed_vector *result)
{
	real parts[3];
	short index;

	parts[0] = vector->i;
	parts[1] = vector->j;
	parts[2] = vector->k;
	for (index = 0; index < 3; index++)
	{
		real value = parts[index] * scale;

		value = value > 32767.0f ? 32767.0f : value < -32767.0f ? -32767.0f : value;
		parts[index] = (real)floor(value + 0.5f);
	}
	result->i = (short)parts[0];
	result->j = (short)parts[1];
	result->k = (short)parts[2];
}

void distributed_vector_unpack(
	struct distributed_vector const *vector,
	real scale,
	real_vector3d *result)
{
	result->i = (real)vector->i / scale;
	result->j = (real)vector->j / scale;
	result->k = (real)vector->k / scale;
}

void distributed_unit_vector_unpack(
	struct distributed_vector const *vector,
	real_vector3d *result)
{
	real length;

	distributed_vector_unpack(vector, DISTRIBUTED_UNIT_SCALE, result);
	length = (real)sqrt(result->i * result->i + result->j * result->j + result->k * result->k);
	if (length > 0.0f)
	{
		result->i /= length;
		result->j /= length;
		result->k /= length;
	}
}

word distributed_vitality_pack(
	real value)
{
	value *= VITALITY_SCALE;
	value = value > 65535.0f ? 65535.0f : value < 0.0f ? 0.0f : value;
	return (word)(long)floor(value + 0.5f);
}

real distributed_vitality_unpack(
	word value)
{
	return (real)value / VITALITY_SCALE;
}

short distributed_angle_pack(
	real angle)
{
	real turns = angle / (2.0f * _pi);

	turns -= (real)floor(turns);
	return (short)(word)((long)floor(turns * 65536.0f + 0.5f) & 0xFFFF);
}

real distributed_angle_unpack(
	short value,
	boolean signed_angle)
{
	real angle = (real)(word)value * (2.0f * _pi) / 65536.0f;

	if (signed_angle && angle >= _pi)
		angle -= 2.0f * _pi;
	return angle;
}

static void distributed_batch_flush(
	short sender)
{
	struct distributed_batch *batch = &distributed_batches[sender];
	struct distributed_message_header *header = (struct distributed_message_header *)batch->data;

	if (!batch->size)
		return;
	header->type = _distributed_message_batch;
	header->count = 0;
	header->game_time = game_time_get();
	header->checkpoint_epoch = distributed_checkpoint_epoch;
	header->header = 0;
	build_message_header(&header->header, batch->size, 2, 0);
	if (sender == HOST_SENDER)
		network_distributed_client_send(batch->data, batch->size);
	else
		network_distributed_server_send_to_machine(sender, batch->data, batch->size);
	batch->size = 0;
}

/* the bytes a message's entries (past its header) have in the machine's
batch now; when fewer than minimum, the batch is sent first, and they are
an empty batch's */
static word distributed_batch_room(
	short sender,
	word minimum)
{
	struct distributed_batch *batch = &distributed_batches[sender];
	word used = batch->size ? batch->size : (word)sizeof(struct distributed_message_header);

	if (used + BATCH_MESSAGE_OVERHEAD + minimum > DATAGRAM_MAXIMUM_SIZE)
	{
		distributed_batch_flush(sender);
		used = sizeof(struct distributed_message_header);
	}
	return (word)(DATAGRAM_MAXIMUM_SIZE - used - BATCH_MESSAGE_OVERHEAD);
}

/* a message's header and entries into the machine's batch, which has room
for them */
static void distributed_batch_append(
	short sender,
	struct distributed_message_header const *header,
	byte const *entries,
	word entries_size)
{
	struct distributed_batch *batch = &distributed_batches[sender];
	word length = (word)(sizeof(*header) - sizeof(message_header) + entries_size);

	if (!batch->size)
		batch->size = sizeof(struct distributed_message_header);
	csmemcpy(batch->data + batch->size, &length, sizeof(word));
	csmemcpy(batch->data + batch->size + sizeof(word), (byte const *)header + sizeof(message_header),
		sizeof(*header) - sizeof(message_header));
	csmemcpy(batch->data + batch->size + BATCH_MESSAGE_OVERHEAD, entries, entries_size);
	batch->size += (word)(BATCH_MESSAGE_OVERHEAD + entries_size);
}

/* where a message's bytes past its message header go: a machine's batch,
sent at the tick's end (or sooner, full). A message of entries of
entry_size bytes each is split where a batch is full, the rest going in
the next; one not to be split (entry_size 0) goes whole, and one larger
than a datagram, which is a mistake, is dropped. */
static void distributed_batch_add(
	short sender,
	void const *message,
	word size,
	word entry_size)
{
	struct distributed_message_header header;
	byte const *entries = (byte const *)message + sizeof(header);
	word entries_size;
	short count;

	if (size < sizeof(header))
		return;
	csmemcpy(&header, message, sizeof(header));
	entries_size = (word)(size - sizeof(header));
	count = header.count;
	if (!entry_size || !count || entries_size != count * entry_size)
	{
		if (distributed_batch_room(sender, entries_size) < entries_size)
		{
			error(_error_silent, "distributed message of type #%d is #%d bytes, too large for a datagram; dropped",
				header.type, size);
			return;
		}
		distributed_batch_append(sender, &header, entries, entries_size);
		return;
	}
	while (count > 0)
	{
		short fit = (short)MIN(count, distributed_batch_room(sender, entry_size) / entry_size);

		if (fit <= 0)
		{
			error(_error_silent,
				"distributed message of type #%d has entries of #%d bytes, too large for a datagram; dropped",
				header.type, entry_size);
			return;
		}
		header.count = (byte)fit;
		distributed_batch_append(sender, &header, entries, (word)(fit * entry_size));
		entries += fit * entry_size;
		count -= fit;
	}
}

/* (the machines' datagrams in a turn that starts one further each tick:
none always last) */
static void distributed_batches_flush(
	void)
{
	short first = (short)((unsigned long)game_time_get() % MAXIMUM_SENDERS);
	short step;

	for (step = 0; step < MAXIMUM_SENDERS; step++)
		distributed_batch_flush((short)((first + step) % MAXIMUM_SENDERS));
}

static boolean distributed_fill_header(
	void *message,
	byte type,
	short count,
	word size)
{
	struct distributed_message_header *header = (struct distributed_message_header *)message;

	/* Until the ordered snapshot finishes, surviving local replicas are
	still being replaced. They cannot supply new-world input or hit reports. */
	if (game_in_progress() && game_connection() == _game_connection_network_client &&
		!distributed_checkpoint_synchronized && type != _distributed_message_client_ready)
		return FALSE;
	header->type = type;
	header->count = (byte)count;
	header->game_time = game_time_get();
	header->checkpoint_epoch = distributed_checkpoint_epoch;
	header->header = 0;
	build_message_header(&header->header, size, 2, 0);
	distributed_statistics.sent++;
	return TRUE;
}

/* the size of each of a message's entries, when they are alike, else 0 */
static word distributed_entry_size(
	short count,
	word size)
{
	word entries_size = (word)(size - sizeof(struct distributed_message_header));

	if (count <= 0 || size <= sizeof(struct distributed_message_header) || entries_size % count)
		return 0;
	return (word)(entries_size / count);
}

void distributed_send(
	void *message,
	byte type,
	short count,
	word size,
	short destination)
{
	if (!distributed_fill_header(message, type, count, size))
		return;
	switch (destination)
	{
	case _distributed_to_clients:
	{
		long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
		short machine_count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
		short index;

		for (index = 0; index < machine_count; index++)
			distributed_batch_add((short)machine_indices[index], message, size, distributed_entry_size(count, size));
		break;
	}
	case _distributed_to_clients_reliably: network_distributed_server_send_to_all_reliably(message, size); break;
	case _distributed_to_host:
		distributed_batch_add(HOST_SENDER, message, size, distributed_entry_size(count, size));
		break;
	case _distributed_to_host_reliably: network_distributed_client_send_reliably(message, size); break;
	}
}

void distributed_send_to_machine(
	long machine_index,
	void *message,
	byte type,
	short count,
	word size)
{
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return;
	if (!distributed_fill_header(message, type, count, size))
		return;
	distributed_batch_add((short)machine_index, message, size, distributed_entry_size(count, size));
}

void distributed_send_to_machine_reliably(
	long machine_index,
	void *message,
	byte type,
	short count,
	word size)
{
	if (!distributed_fill_header(message, type, count, size))
		return;
	network_distributed_server_send_to_machine_reliably(machine_index, message, size);
}

void distributed_checkpoint_to_machine(long machine_index)
{
	struct distributed_message_header message;

	if (game_connection() != _game_connection_network_server ||
		machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return;
	distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_checkpoint, 0, sizeof(message));
}

boolean distributed_checkpoint_ready(void)
{
	return distributed_checkpoint_synchronized;
}

/* a packer's messages of the type, to the machine (or the host), each
beginning with the prefix */
static void distributed_packer_begin(
	struct distributed_packer *packer,
	short sender,
	byte type,
	void const *prefix,
	word prefix_size)
{
	packer->sender = sender;
	packer->type = type;
	packer->count = 0;
	packer->prefix_size = prefix_size;
	packer->size = prefix_size;
	packer->room = 0;
	if (prefix_size)
		csmemcpy(packer->message.data, prefix, prefix_size);
}

/* the message packed so far, into the batch */
static void distributed_packer_flush(
	struct distributed_packer *packer)
{
	word size = (word)(sizeof(packer->message.header) + packer->size);

	if (!packer->count)
		return;
	if (distributed_fill_header(&packer->message, packer->type, packer->count, size))
		distributed_batch_add(packer->sender, &packer->message, size, 0);
	packer->count = 0;
	packer->size = packer->prefix_size;
}

/* an entry into the message, which goes into the batch first when the
entry would not fit it (and a batch full goes first then) */
static void distributed_packer_add(
	struct distributed_packer *packer,
	void const *entry,
	word entry_size)
{
	if (packer->count && (packer->size + entry_size > packer->room || packer->count >= 255))
		distributed_packer_flush(packer);
	if (!packer->count)
		packer->room = MIN(distributed_batch_room(packer->sender, (word)(packer->prefix_size + entry_size)),
			(word)sizeof(packer->message.data));
	if (packer->size + entry_size > packer->room)
		return;
	csmemcpy(packer->message.data + packer->size, entry, entry_size);
	packer->size += entry_size;
	packer->count++;
}

/* the bytes of what a message says, in order: written, and read no further
than its end (FALSE past it) */
static byte *distributed_put(
	byte *cursor,
	void const *data,
	short size)
{
	csmemcpy(cursor, data, size);
	return cursor + size;
}

static boolean distributed_take(
	byte const **cursor,
	byte const *end,
	void *data,
	short size)
{
	if (end - *cursor < size)
		return FALSE;
	csmemcpy(data, *cursor, size);
	*cursor += size;
	return TRUE;
}

struct player_datum *distributed_player(
	short player_index)
{
	struct player_datum *player;

	if (player_index < 0 || player_index >= player_data->maximum_count)
		return NULL;
	player = (struct player_datum *)((byte *)player_data->data + player_index * player_data->size);
	return player->identifier ? player : NULL;
}

byte distributed_player_to_byte(
	long player_index)
{
	return player_index != NONE ? (byte)DATUM_INDEX_TO_ABSOLUTE_INDEX(player_index) : NO_PLAYER;
}

long distributed_player_from_byte(
	byte player_index)
{
	struct player_datum *player = player_index != NO_PLAYER ? distributed_player(player_index) : NULL;

	return player ? DATUM_INDEX_NEW(player_index, player->identifier) : NONE;
}

boolean distributed_player_is_local(
	long player_index)
{
	struct player_datum *player = player_index != NONE ? player_try_and_get(player_index) : NULL;

	return player && player->local_player_index != NONE;
}

long distributed_living_unit(
	struct player_datum const *player)
{
	if (!player || player->unit_index == NONE || !object_try_and_get(player->unit_index) ||
		TEST_FLAG(object_get(player->unit_index)->object.damage_flags, _object_dead_bit))
	{
		return NONE;
	}
	return player->unit_index;
}

boolean distributed_machine_has_player(
	long machine_index,
	short player_index)
{
	long *player_list = machine_get_player_list(machine_index);
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		if (player_list[local_player_index] != NONE &&
			DATUM_INDEX_TO_ABSOLUTE_INDEX(player_list[local_player_index]) == player_index)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* whether the machine's players are this machine's (the host's own machine
is in the game as its clients are) */
static boolean distributed_machine_is_local(
	long machine_index)
{
	long *player_list = machine_get_player_list(machine_index);
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		struct player_datum *player = player_list[local_player_index] != NONE ?
			player_try_and_get(player_list[local_player_index]) : NULL;

		if (player && player->local_player_index != NONE)
			return TRUE;
	}
	return FALSE;
}

short distributed_client_machines(
	long *machine_indices,
	short maximum)
{
	short index;

	/* (found once a tick: the machines do not change during one) */
	if (!distributed_machines.valid)
	{
		short count = network_distributed_server_machines(distributed_machines.indices,
			HALO_PORT_MAXIMUM_NETWORK_MACHINES);

		distributed_machines.count = 0;
		for (index = 0; index < count; index++)
		{
			if (distributed_machines.indices[index] >= 0 &&
				distributed_machines.indices[index] < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
				!distributed_machine_is_local(distributed_machines.indices[index]))
			{
				distributed_machines.indices[distributed_machines.count++] = distributed_machines.indices[index];
			}
		}
		distributed_machines.valid = distributed_machines.in_tick;
	}
	for (index = 0; index < distributed_machines.count && index < maximum; index++)
		machine_indices[index] = distributed_machines.indices[index];
	return index;
}

/* (the host) a machine has loaded the game: if it is new at its index (its
players are not those the index had), none of the old machine's state, and
TRUE (it has none of the game's either); FALSE for one that asks again */
static boolean distributed_machine_loaded(
	long machine_index)
{
	long *player_list;
	short type;
	short player_index;

	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return FALSE;
	player_list = machine_get_player_list(machine_index);
	if (!csmemcmp(distributed_machine_players[machine_index], player_list,
		sizeof(distributed_machine_players[machine_index])))
	{
		return FALSE;
	}
	csmemcpy(distributed_machine_players[machine_index], player_list,
		sizeof(distributed_machine_players[machine_index]));
	for (type = 0; type < NUMBER_OF_DISTRIBUTED_MESSAGES; type++)
		distributed_received_times[machine_index][type] = NONE;
	distributed_sender_times[machine_index] = NONE;
	csmemset(&distributed_round_trips[machine_index], 0, sizeof(distributed_round_trips[machine_index]));
	csmemset(distributed_viewers[machine_index], 0, sizeof(distributed_viewers[machine_index]));
	csmemset(&distributed_client_clocks[machine_index], 0, sizeof(distributed_client_clocks[machine_index]));
	distributed_client_clocks[machine_index].window_tick = NONE;
	csmemset(&distributed_client_identities[machine_index], 0, sizeof(distributed_client_identities[machine_index]));
	csmemset(&distributed_client_builds[machine_index], 0, sizeof(distributed_client_builds[machine_index]));
	distributed_client_build_known[machine_index] = FALSE;
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		distributed_seen[machine_index][player_index] = FALSE;
	return TRUE;
}

long distributed_latest_host_time(
	void)
{
	return distributed_host_time;
}

real distributed_own_round_trip_ticks(
	void)
{
	return distributed_own_round_trip;
}

long distributed_player_machine(
	short player_index)
{
	return player_index >= 0 && player_index < MAXIMUM_TRACKED_PLAYERS ?
		distributed_player_machines[player_index] : NONE;
}

boolean distributed_machine_sees_player(
	long machine_index,
	short player_index)
{
	return machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
		player_index >= 0 && player_index < MAXIMUM_TRACKED_PLAYERS &&
		TEST_FLAG(distributed_sends[machine_index][player_index], _distributed_send_sees_bit);
}

long distributed_player_ping(
	short player_index)
{
	long machine_index;

	if (player_index < 0 || player_index >= MAXIMUM_TRACKED_PLAYERS || !distributed_player(player_index))
		return NONE;
	if (game_connection() == _game_connection_network_client)
		return distributed_player_pings[player_index] == UNKNOWN_PING ? NONE : (long)distributed_player_pings[player_index];
	if (game_connection() != _game_connection_network_server)
		return NONE;
	/* (the host: its own players' none, a client's its machine's) */
	machine_index = distributed_player_machines[player_index];
	if (machine_index == NONE)
		return 0;
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES ||
		!distributed_round_trips[machine_index].valid)
	{
		return NONE;
	}
	return (long)(distributed_round_trips[machine_index].average * 1000.0f / TICKS_PER_SECOND + 0.5f);
}

real distributed_machine_round_trip_ticks(
	long machine_index)
{
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES ||
		!distributed_round_trips[machine_index].valid)
	{
		return (real)DEFAULT_ROUND_TRIP_TICKS;
	}
	return distributed_round_trips[machine_index].average + 2.0f * distributed_round_trips[machine_index].deviation;
}

/* ---------- the latency meter's round trips */

/* the milliseconds since this machine sent the messages of its tick (at
most MAXIMUM_PING_MILLISECONDS), NONE if that tick is not one of the last
TICK_SENT_TIMES */
static long distributed_milliseconds_since_tick(
	long time)
{
	unsigned long elapsed;

	if (time == NONE || distributed_tick_sent_times[time & (TICK_SENT_TIMES - 1)].time != time)
		return NONE;
	elapsed = system_milliseconds() - distributed_tick_sent_times[time & (TICK_SENT_TIMES - 1)].milliseconds;
	return elapsed > (unsigned long)MAXIMUM_PING_MILLISECONDS ? MAXIMUM_PING_MILLISECONDS : (long)elapsed;
}

/* a round trip in milliseconds into a smoothed one (as TCP smooths its own,
an eighth), the first as it is */
static void distributed_smooth_ping(
	boolean *valid,
	real *milliseconds,
	long sample)
{
	if (!*valid)
	{
		*valid = TRUE;
		*milliseconds = (real)sample;
	}
	else
	{
		*milliseconds += ((real)sample - *milliseconds) / 8.0f;
	}
}

/* (a client) the milliseconds since the host's latest message came, NONE
before the first */
static long distributed_host_silence(
	void)
{
	return distributed_host_time == NONE ? NONE : (long)(system_milliseconds() - distributed_host_time_received);
}

/* (the host) whether the client machine has said nothing for a while */
static boolean distributed_machine_silent(
	long machine_index)
{
	return game_engine_in_play() && machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
		distributed_round_trips[machine_index].heard &&
		system_milliseconds() - distributed_round_trips[machine_index].heard > CONNECTION_PROBLEM_MILLISECONDS;
}

/* (a client) the host has its players (or its vehicle) at that tick of this
machine's, as told now: its own round trip timed */
void distributed_note_own_round_trip(
	long time)
{
	long sample = distributed_milliseconds_since_tick(time);

	if (sample != NONE && game_connection() == _game_connection_network_client)
		distributed_smooth_ping(&distributed_own_ping_valid, &distributed_own_ping_milliseconds, sample);
}

/* what the latency meter shows, refreshed in the tick (with Latency meter
On): a client's own round trip, the host's slowest client's (one silent
for a while as long as it has been); and in the log, each connection
problem as it starts and ends, and the round trips every ten seconds */
static void distributed_refresh_pings(
	void)
{
	short connection = game_connection();
	short player_index;

	distributed_shown_own_ping = NONE;
	distributed_shown_slowest_ping = NONE;
	if (connection == _game_connection_network_client && distributed_own_ping_valid)
		distributed_shown_own_ping = (long)(distributed_own_ping_milliseconds + 0.5f);
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		long ping = NONE;

		if (!distributed_player(player_index))
			;
		else if (connection == _game_connection_network_server)
		{
			long machine_index = distributed_player_machines[player_index];

			if (machine_index == NONE)
				ping = 0;
			else if (machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
				distributed_round_trips[machine_index].milliseconds_valid)
			{
				ping = (long)(distributed_round_trips[machine_index].milliseconds + 0.5f);
			}
		}
		else if (distributed_player_is_local(player_index))
		{
			ping = distributed_shown_own_ping;
		}
		distributed_shown_pings[player_index] = ping;
	}
	if (connection == _game_connection_network_server)
	{
		long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
		short count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
		boolean log_round_trips = game_time_get() % (10 * TICKS_PER_SECOND) == 0;
		char line[512];
		int length = 0;
		short index;

		line[0] = 0;
		for (index = 0; index < count; index++)
		{
			long machine_index = machine_indices[index];
			boolean silent;
			long ping;

			if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
				continue;
			silent = distributed_machine_silent(machine_index);
			if (silent && !distributed_round_trips[machine_index].silent_since)
			{
				distributed_round_trips[machine_index].silent_since = distributed_round_trips[machine_index].heard;
				platform_log("latency: machine %ld silent for %lu ms: connection problem", machine_index,
					system_milliseconds() - distributed_round_trips[machine_index].heard);
			}
			else if (!silent && distributed_round_trips[machine_index].silent_since)
			{
				platform_log("latency: machine %ld heard again after %lu ms", machine_index,
					distributed_round_trips[machine_index].heard - distributed_round_trips[machine_index].silent_since);
				distributed_round_trips[machine_index].silent_since = 0;
			}
			if (!distributed_round_trips[machine_index].milliseconds_valid)
				continue;
			ping = (long)(distributed_round_trips[machine_index].milliseconds + 0.5f);
			if (silent)
			{
				ping = MAX(ping, (long)MIN(system_milliseconds() - distributed_round_trips[machine_index].heard,
					(unsigned long)MAXIMUM_PING_MILLISECONDS));
			}
			distributed_shown_slowest_ping = MAX(distributed_shown_slowest_ping, ping);
			if (log_round_trips && length < (int)sizeof(line) - 64)
			{
				length += snprintf(line + length, sizeof(line) - length, "%s machine %ld %ld ms (%.1f ticks)%s",
					length ? "," : "", machine_index, ping, distributed_round_trips[machine_index].average,
					silent ? " silent" : "");
			}
		}
		if (length)
			platform_log("latency: the clients' round trips:%s", line);
	}
	else if (connection == _game_connection_network_client)
	{
		boolean problem = distributed_connection_problem();

		if (problem && !distributed_host_silent_since)
		{
			distributed_host_silent_since = distributed_host_time_received ? distributed_host_time_received : 1;
			platform_log("latency: the host silent for %ld ms: connection problem", distributed_host_silence());
		}
		else if (!problem && distributed_host_silent_since)
		{
			platform_log("latency: the host heard again after %lu ms",
				distributed_host_time_received - distributed_host_silent_since);
			distributed_host_silent_since = 0;
		}
		if (game_time_get() % (10 * TICKS_PER_SECOND) == 0 && distributed_own_ping_valid)
		{
			platform_log("latency: round trip to the host %ld ms (%.1f ticks)%s", distributed_shown_own_ping,
				distributed_own_round_trip, problem ? ", connection problem" : "");
		}
	}
}

long distributed_player_measured_ping(
	short player_index)
{
	return player_index >= 0 && player_index < MAXIMUM_TRACKED_PLAYERS ? distributed_shown_pings[player_index] : NONE;
}

long distributed_own_ping(
	void)
{
	return game_connection() == _game_connection_network_client ? distributed_shown_own_ping : NONE;
}

long distributed_slowest_client_ping(
	void)
{
	return game_connection() == _game_connection_network_server ? distributed_shown_slowest_ping : NONE;
}

boolean distributed_connection_problem(
	void)
{
	/* (a game over: the host sends nothing of it) */
	if (game_connection() == _game_connection_network_client && game_engine_in_play())
	{
		long silence = distributed_host_silence();

		return silence != NONE && silence > CONNECTION_PROBLEM_MILLISECONDS;
	}
	return FALSE;
}

/* ---------- units */

/* Relayed input does not describe a scripted pose. A late join can process
input before the presentation disables it, and then retain that local aim
indefinitely. Body orientation is not a substitute: the host's desired and
current aim/look can differ from it and from each other. Ordinary input keeps
its prediction path and sends none of these additional vectors. */
static void distributed_scripted_pose_from_unit(struct unit_datum const *unit,
	struct distributed_unit_state *state)
{
	if (game_connection() != _game_connection_network_server)
		return;
	SET_FLAG(state->unit_flags, _distributed_unit_suspended_bit,
		TEST_FLAG(unit->unit.flags, _unit_suspended_bit));
	state->scripted_pose = !player_input_enabled() || TEST_FLAG(unit->unit.flags, _unit_suspended_bit);
	if (!state->scripted_pose)
		return;
	distributed_vector_pack(&unit->unit.desired_facing_vector, DISTRIBUTED_UNIT_SCALE, &state->pose[_distributed_pose_facing]);
	distributed_vector_pack(&unit->unit.desired_aiming_vector, DISTRIBUTED_UNIT_SCALE, &state->pose[_distributed_pose_aiming]);
	distributed_vector_pack(&unit->unit.desired_looking_vector, DISTRIBUTED_UNIT_SCALE, &state->pose[_distributed_pose_looking]);
	distributed_vector_pack(&unit->unit.aiming_vector, DISTRIBUTED_UNIT_SCALE, &state->pose[_distributed_pose_current_aiming]);
	distributed_vector_pack(&unit->unit.looking_vector, DISTRIBUTED_UNIT_SCALE, &state->pose[_distributed_pose_current_looking]);
}

static boolean distributed_scripted_pose_valid(struct distributed_unit_state const *state)
{
	short index;
	for (index = 0; index < NUMBER_OF_DISTRIBUTED_POSE_VECTORS; index++)
	{
		real_vector3d vector;
		real length_squared;
		distributed_vector_unpack(&state->pose[index], DISTRIBUTED_UNIT_SCALE, &vector);
		length_squared = vector.i * vector.i + vector.j * vector.j + vector.k * vector.k;
		/* Unit normals quantized to signed 16-bit components; reject zero
		and non-unit data before any of the decoded state is applied. */
		if (length_squared < 0.99f || length_squared > 1.01f)
			return FALSE;
	}
	return TRUE;
}

static void distributed_scripted_pose_apply(struct unit_datum *unit,
	struct distributed_unit_state const *state)
{
	SET_FLAG(unit->unit.flags, _unit_suspended_bit,
		TEST_FLAG(state->unit_flags, _distributed_unit_suspended_bit));
	if (!state->scripted_pose)
		return;
	distributed_unit_vector_unpack(&state->pose[_distributed_pose_facing], &unit->unit.desired_facing_vector);
	distributed_unit_vector_unpack(&state->pose[_distributed_pose_aiming], &unit->unit.desired_aiming_vector);
	distributed_unit_vector_unpack(&state->pose[_distributed_pose_looking], &unit->unit.desired_looking_vector);
	distributed_unit_vector_unpack(&state->pose[_distributed_pose_current_aiming], &unit->unit.aiming_vector);
	distributed_unit_vector_unpack(&state->pose[_distributed_pose_current_looking], &unit->unit.looking_vector);
}

/* a deck part's bytes (DISTRIBUTED_DECK_SIZE): its vehicle, node and
place, its velocity, ticks and flags, and its peak, the velocity and the
peak at the velocity's scale */
static byte *distributed_deck_put(
	byte *cursor,
	struct vehicle_riding_deck const *deck)
{
	struct distributed_vector velocity;
	struct distributed_vector peak;
	real_vector3d peak_vector;

	distributed_vector_pack(&deck->velocity, DISTRIBUTED_VELOCITY_SCALE, &velocity);
	peak_vector.i = deck->peak;
	peak_vector.j = 0.0f;
	peak_vector.k = 0.0f;
	distributed_vector_pack(&peak_vector, DISTRIBUTED_VELOCITY_SCALE, &peak);
	cursor = distributed_put(cursor, &deck->vehicle_index, sizeof(deck->vehicle_index));
	cursor = distributed_put(cursor, &deck->node_index, sizeof(deck->node_index));
	cursor = distributed_put(cursor, &deck->place, sizeof(deck->place));
	cursor = distributed_put(cursor, &velocity, sizeof(velocity));
	cursor = distributed_put(cursor, &deck->ticks, sizeof(deck->ticks));
	cursor = distributed_put(cursor, &deck->flags, sizeof(deck->flags));
	return distributed_put(cursor, &peak.i, sizeof(peak.i));
}

/* a deck part from the wire: FALSE when it is cut short; present only
whole, its vehicle one this machine has by the salted lookup and every
value one it can be (vehicle_riding_deck_valid), else absent and the
deck left as it was */
static boolean distributed_deck_take(
	byte const **cursor,
	byte const *end,
	boolean *present,
	struct vehicle_riding_deck *deck)
{
	struct vehicle_riding_deck taken;
	struct distributed_vector velocity;
	struct distributed_vector peak;
	real_vector3d peak_vector;

	csmemset(&taken, 0, sizeof(taken));
	csmemset(&peak, 0, sizeof(peak));
	if (!distributed_take(cursor, end, &taken.vehicle_index, sizeof(taken.vehicle_index)) ||
		!distributed_take(cursor, end, &taken.node_index, sizeof(taken.node_index)) ||
		!distributed_take(cursor, end, &taken.place, sizeof(taken.place)) ||
		!distributed_take(cursor, end, &velocity, sizeof(velocity)) ||
		!distributed_take(cursor, end, &taken.ticks, sizeof(taken.ticks)) ||
		!distributed_take(cursor, end, &taken.flags, sizeof(taken.flags)) ||
		!distributed_take(cursor, end, &peak.i, sizeof(peak.i)))
	{
		return FALSE;
	}
	distributed_vector_unpack(&velocity, DISTRIBUTED_VELOCITY_SCALE, &taken.velocity);
	distributed_vector_unpack(&peak, DISTRIBUTED_VELOCITY_SCALE, &peak_vector);
	taken.peak = peak_vector.i;
	*present = vehicle_riding_deck_vehicle_named(taken.vehicle_index) &&
		vehicle_riding_deck_valid(&taken, biped_riding_vehicle_node_count(taken.vehicle_index));
	if (*present)
		*deck = taken;
	return TRUE;
}

static void distributed_state_from_player(
	short player_index,
	struct distributed_unit_state *state)
{
	struct player_datum *player = distributed_player(player_index);
	long unit_index = distributed_living_unit(player);

	csmemset(state, 0, sizeof(*state));
	state->player_index = (byte)player_index;
	state->unit_index = NONE;
	state->vehicle_index = NONE;
	state->seat_index = NONE;
	if (unit_index != NONE)
	{
		struct unit_datum *unit = unit_get(unit_index);
		struct damage_network_state damage;
		real camouflage = unit->unit.active_camouflage;

		state->unit_index = unit_index;
		SET_FLAG(state->flags, _distributed_unit_alive_bit, TRUE);
		SET_FLAG(state->flags, _distributed_unit_placed_bit, unit->object.parent_object_index == NONE);
		if (unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE)
		{
			state->vehicle_index = unit->object.parent_object_index;
			state->seat_index = unit->unit.parent_seat_index;
		}
		state->position = unit->object.position;
		distributed_vector_pack(&unit->object.translational_velocity, DISTRIBUTED_VELOCITY_SCALE, &state->velocity);
		distributed_vector_pack(&unit->object.forward, DISTRIBUTED_UNIT_SCALE, &state->forward);
		distributed_vector_pack(&unit->object.up, DISTRIBUTED_UNIT_SCALE, &state->up);
		/* (in a vehicle's window: where on its deck, as it is now) */
		state->deck_present = TEST_FLAG(state->flags, _distributed_unit_placed_bit) &&
			biped_riding_deck(unit_index, &state->deck);
		damage_get_network_state(unit_index, &damage);
		SET_FLAG(state->flags, _distributed_unit_shield_depleted_bit, damage.shield_depleted);
		SET_FLAG(state->flags, _distributed_unit_shield_charging_bit, damage.shield_charging);
		SET_FLAG(state->flags, _distributed_unit_shield_over_charging_bit, damage.shield_over_charging);
		state->body_vitality = distributed_vitality_pack(damage.body_vitality);
		state->shield_vitality = distributed_vitality_pack(damage.shield_vitality);
		state->current_body_damage = distributed_vitality_pack(damage.current_body_damage);
		state->recent_body_damage = distributed_vitality_pack(damage.recent_body_damage);
		state->current_shield_damage = distributed_vitality_pack(damage.current_shield_damage);
		state->recent_shield_damage = distributed_vitality_pack(damage.recent_shield_damage);
		csmemcpy(state->powerup_durations, player->powerup_durations, sizeof(state->powerup_durations));
		SET_FLAG(state->unit_flags, _distributed_unit_camouflaged_bit,
			TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit));
		SET_FLAG(state->unit_flags, _distributed_unit_super_camouflaged_bit,
			TEST_FLAG(unit->unit.flags, _unit_super_camouflaged_bit));
		camouflage = camouflage > 1.0f ? 1.0f : camouflage < 0.0f ? 0.0f : camouflage;
		state->active_camouflage = (byte)(long)floor(camouflage * 255.0f + 0.5f);
		distributed_scripted_pose_from_unit(unit, state);
		/* (the host) a client's player where the client had them: at which of
		its ticks (with the ticks run since) */
		if (game_connection() == _game_connection_network_server && player_index < MAXIMUM_TRACKED_PLAYERS &&
			distributed_predictions[player_index].taken_host_time != NONE &&
			game_time_get() - distributed_predictions[player_index].taken_host_time < PREDICTION_ECHO_TICKS)
		{
			SET_FLAG(state->flags, _distributed_unit_predicted_bit, TRUE);
			state->predicted_time = (short)(word)(distributed_predictions[player_index].taken_time +
				game_time_get() - distributed_predictions[player_index].taken_host_time);
		}
	}
	state->killing_player_index = NO_PLAYER;
	if (unit_index == NONE && player_index < MAXIMUM_TRACKED_PLAYERS && distributed_deaths[player_index].valid)
	{
		struct distributed_death const *death = &distributed_deaths[player_index];

		if (death->killing_player_index != NONE)
			state->killing_player_index = (byte)death->killing_player_index;
		SET_FLAG(state->flags, _distributed_unit_friendly_fire_bit, death->friendly_fire);
		SET_FLAG(state->flags, _distributed_unit_killed_by_vehicle_bit, death->killed_by_vehicle);
	}
}

/* the state's bytes on the wire (at most DISTRIBUTED_UNIT_STATE_MAXIMUM_SIZE):
its player, flags and parts, then (alive) its unit, the vehicle and seat it
rides, (placed) where it is, how fast it moves and which way it faces, and
where on a vehicle's deck, (predicted) the client's tick, its shields and
health, the damage they show, its powerups and camouflage, or (dead) who
killed it; what is its default is left out (distributed_unit_state_read). The
predicted tick is its own client's alone (own). */
static word distributed_unit_state_write(
	struct distributed_unit_state const *state,
	boolean own,
	byte *buffer)
{
	byte flags = state->flags;
	byte parts = 0;
	byte *cursor = buffer + DISTRIBUTED_UNIT_STATE_MINIMUM_SIZE;

	if (!own)
		SET_FLAG(flags, _distributed_unit_predicted_bit, FALSE);
	if (TEST_FLAG(flags, _distributed_unit_alive_bit))
	{
		cursor = distributed_put(cursor, &state->unit_index, sizeof(state->unit_index));
		if (state->vehicle_index != NONE)
		{
			SET_FLAG(parts, _distributed_unit_part_riding_bit, TRUE);
			cursor = distributed_put(cursor, &state->vehicle_index, sizeof(state->vehicle_index));
			cursor = distributed_put(cursor, &state->seat_index, sizeof(state->seat_index));
		}
		if (TEST_FLAG(flags, _distributed_unit_placed_bit))
		{
			cursor = distributed_put(cursor, &state->position, sizeof(state->position));
			cursor = distributed_put(cursor, &state->velocity, sizeof(state->velocity));
			cursor = distributed_put(cursor, &state->forward, sizeof(state->forward));
			if (state->up.i || state->up.j || state->up.k != (short)DISTRIBUTED_UNIT_SCALE)
			{
				SET_FLAG(parts, _distributed_unit_part_up_bit, TRUE);
				cursor = distributed_put(cursor, &state->up, sizeof(state->up));
			}
			/* (whole or not at all: never a value not finite) */
			if (state->deck_present && vehicle_riding_deck_values_valid(&state->deck))
			{
				SET_FLAG(parts, _distributed_unit_part_deck_bit, TRUE);
				cursor = distributed_deck_put(cursor, &state->deck);
			}
		}
		if (TEST_FLAG(flags, _distributed_unit_predicted_bit))
			cursor = distributed_put(cursor, &state->predicted_time, sizeof(state->predicted_time));
		cursor = distributed_put(cursor, &state->body_vitality, sizeof(state->body_vitality));
		cursor = distributed_put(cursor, &state->shield_vitality, sizeof(state->shield_vitality));
		if (state->current_body_damage || state->recent_body_damage || state->current_shield_damage ||
			state->recent_shield_damage)
		{
			SET_FLAG(parts, _distributed_unit_part_damage_bit, TRUE);
			cursor = distributed_put(cursor, &state->current_body_damage, sizeof(state->current_body_damage));
			cursor = distributed_put(cursor, &state->recent_body_damage, sizeof(state->recent_body_damage));
			cursor = distributed_put(cursor, &state->current_shield_damage, sizeof(state->current_shield_damage));
			cursor = distributed_put(cursor, &state->recent_shield_damage, sizeof(state->recent_shield_damage));
		}
		{
			short powerup_index;

			for (powerup_index = 0; powerup_index < NUMBER_OF_PLAYER_POWERUPS; powerup_index++)
			{
				if (state->powerup_durations[powerup_index])
					break;
			}
			if (powerup_index < NUMBER_OF_PLAYER_POWERUPS)
			{
				SET_FLAG(parts, _distributed_unit_part_powerups_bit, TRUE);
				cursor = distributed_put(cursor, state->powerup_durations, sizeof(state->powerup_durations));
			}
		}
		if (state->unit_flags || state->active_camouflage)
		{
			SET_FLAG(parts, _distributed_unit_part_camouflage_bit, TRUE);
			cursor = distributed_put(cursor, &state->unit_flags, sizeof(state->unit_flags));
			cursor = distributed_put(cursor, &state->active_camouflage, sizeof(state->active_camouflage));
		}
		if (state->scripted_pose)
		{
			SET_FLAG(parts, _distributed_unit_part_scripted_pose_bit, TRUE);
			cursor = distributed_put(cursor, state->pose, sizeof(state->pose));
		}
	}
	else if (state->killing_player_index != NO_PLAYER)
	{
		SET_FLAG(parts, _distributed_unit_part_killer_bit, TRUE);
		cursor = distributed_put(cursor, &state->killing_player_index, sizeof(state->killing_player_index));
	}
	buffer[0] = state->player_index;
	buffer[1] = flags;
	buffer[2] = parts;
	return (word)(cursor - buffer);
}

/* a state from its bytes on the wire, no further than end: the bytes it
took, 0 for none that make one */
static word distributed_unit_state_read(
	byte const *buffer,
	byte const *end,
	struct distributed_unit_state *state)
{
	byte const *cursor = buffer;
	byte parts;

	csmemset(state, 0, sizeof(*state));
	state->unit_index = NONE;
	state->vehicle_index = NONE;
	state->seat_index = NONE;
	state->killing_player_index = NO_PLAYER;
	state->up.k = (short)DISTRIBUTED_UNIT_SCALE;
	if (!distributed_take(&cursor, end, &state->player_index, sizeof(state->player_index)) ||
		!distributed_take(&cursor, end, &state->flags, sizeof(state->flags)) ||
		!distributed_take(&cursor, end, &parts, sizeof(parts)))
	{
		return 0;
	}
	if (TEST_FLAG(parts, _distributed_unit_part_scripted_pose_bit) &&
		!TEST_FLAG(state->flags, _distributed_unit_alive_bit))
		return 0;
	/* (a deck part where no place is: the bytes after it cannot be read, so
	neither can the state) */
	if (TEST_FLAG(parts, _distributed_unit_part_deck_bit) &&
		!(TEST_FLAG(state->flags, _distributed_unit_alive_bit) && TEST_FLAG(state->flags, _distributed_unit_placed_bit)))
	{
		return 0;
	}
	if (TEST_FLAG(state->flags, _distributed_unit_alive_bit))
	{
		if (!distributed_take(&cursor, end, &state->unit_index, sizeof(state->unit_index)))
			return 0;
		if (TEST_FLAG(parts, _distributed_unit_part_riding_bit) &&
			(!distributed_take(&cursor, end, &state->vehicle_index, sizeof(state->vehicle_index)) ||
				!distributed_take(&cursor, end, &state->seat_index, sizeof(state->seat_index))))
		{
			return 0;
		}
		if (TEST_FLAG(state->flags, _distributed_unit_placed_bit) &&
			(!distributed_take(&cursor, end, &state->position, sizeof(state->position)) ||
				!distributed_take(&cursor, end, &state->velocity, sizeof(state->velocity)) ||
				!distributed_take(&cursor, end, &state->forward, sizeof(state->forward)) ||
				(TEST_FLAG(parts, _distributed_unit_part_up_bit) &&
					!distributed_take(&cursor, end, &state->up, sizeof(state->up)))))
		{
			return 0;
		}
		if (TEST_FLAG(parts, _distributed_unit_part_deck_bit) &&
			!distributed_deck_take(&cursor, end, &state->deck_present, &state->deck))
		{
			return 0;
		}
		if (TEST_FLAG(state->flags, _distributed_unit_predicted_bit) &&
			!distributed_take(&cursor, end, &state->predicted_time, sizeof(state->predicted_time)))
		{
			return 0;
		}
		if (!distributed_take(&cursor, end, &state->body_vitality, sizeof(state->body_vitality)) ||
			!distributed_take(&cursor, end, &state->shield_vitality, sizeof(state->shield_vitality)))
		{
			return 0;
		}
		if (TEST_FLAG(parts, _distributed_unit_part_damage_bit) &&
			(!distributed_take(&cursor, end, &state->current_body_damage, sizeof(state->current_body_damage)) ||
				!distributed_take(&cursor, end, &state->recent_body_damage, sizeof(state->recent_body_damage)) ||
				!distributed_take(&cursor, end, &state->current_shield_damage, sizeof(state->current_shield_damage)) ||
				!distributed_take(&cursor, end, &state->recent_shield_damage, sizeof(state->recent_shield_damage))))
		{
			return 0;
		}
		if (TEST_FLAG(parts, _distributed_unit_part_powerups_bit) &&
			!distributed_take(&cursor, end, state->powerup_durations, sizeof(state->powerup_durations)))
		{
			return 0;
		}
		if (TEST_FLAG(parts, _distributed_unit_part_camouflage_bit) &&
			(!distributed_take(&cursor, end, &state->unit_flags, sizeof(state->unit_flags)) ||
				!distributed_take(&cursor, end, &state->active_camouflage, sizeof(state->active_camouflage))))
		{
			return 0;
		}
		if (TEST_FLAG(parts, _distributed_unit_part_scripted_pose_bit))
		{
			if (!distributed_take(&cursor, end, state->pose, sizeof(state->pose)))
				return 0;
			/* a pose that is not unit vectors (a unit's desired vectors are
			not normalized, and may be zero) is left out: the unit's other
			state, and the message's other units, are still taken */
			state->scripted_pose = distributed_scripted_pose_valid(state);
		}
	}
	else if (TEST_FLAG(parts, _distributed_unit_part_killer_bit) &&
		!distributed_take(&cursor, end, &state->killing_player_index, sizeof(state->killing_player_index)))
	{
		return 0;
	}
	return (word)(cursor - buffer);
}

/* moves the unit toward the state (at position) if it is further than
tolerance from it: within blend_distance part of the way, further all of it,
no faster than maximum_speed across and up and maximum_fall_speed down (0
for any); a negative tolerance always reconciles; FALSE for a state that cannot be */
static boolean distributed_apply_state(
	long unit_index,
	struct distributed_unit_state const *state,
	real_point3d const *position,
	real_vector3d const *velocity,
	real tolerance,
	real blend_distance,
	real maximum_speed,
	real maximum_fall_speed)
{
	struct object_datum *object = object_get(unit_index);
	real_vector3d error;
	real_vector3d taken_velocity;
	real_vector3d forward;
	real_vector3d up;

	if (velocity)
		taken_velocity = *velocity;
	else
		distributed_vector_unpack(&state->velocity, DISTRIBUTED_VELOCITY_SCALE, &taken_velocity);
	distributed_vector_unpack(&state->forward, DISTRIBUTED_UNIT_SCALE, &forward);
	distributed_vector_unpack(&state->up, DISTRIBUTED_UNIT_SCALE, &up);
	if (!distributed_point_valid(position, UNIT_WORLD_BOUND) || !distributed_axes_make_valid(&forward, &up))
		return FALSE;
	if (maximum_speed > 0.0f)
	{
		real rise = taken_velocity.k > 0.0f ? taken_velocity.k : 0.0f;
		real speed = (real)sqrt(taken_velocity.i * taken_velocity.i + taken_velocity.j * taken_velocity.j + rise * rise);

		if (speed > maximum_speed)
		{
			taken_velocity.i *= maximum_speed / speed;
			taken_velocity.j *= maximum_speed / speed;
			if (taken_velocity.k > 0.0f)
				taken_velocity.k *= maximum_speed / speed;
		}
		if (maximum_fall_speed > 0.0f && taken_velocity.k < -maximum_fall_speed)
			taken_velocity.k = -maximum_fall_speed;
	}
	/* (a deck-frame take also commits velocity when the position is exact) */
	if (velocity)
		object->object.translational_velocity = *velocity;
	error.i = position->x - object->object.position.x;
	error.j = position->y - object->object.position.y;
	error.k = position->z - object->object.position.z;
	/* A scripted player can turn without moving. Input prediction cannot
	recover that orientation, even when position already agrees. */
	if (tolerance >= 0.0f && !state->scripted_pose && error.i * error.i + error.j * error.j + error.k * error.k <= tolerance * tolerance)
		return TRUE;
	if (network_objects_reconcile(unit_index, position, &forward, &up, &taken_velocity, NULL, blend_distance, _network_vehicle_write_state))
		distributed_statistics.corrections++;
	return TRUE;
}

/* the elevator a unit stands on this tick, or NONE. A remembered elevator
lasts after a jump. Only a current floor contact allows ignoring a height
difference caused by the platforms' different phases. biped_update_moving
(bipeds.c) sets elevator_ticks to 60 each tick the biped's physics finds it
on an elevator, and counts it down from then on. */
static long distributed_supported_elevator(
	long unit_index)
{
	struct biped_datum *biped = (struct biped_datum *)object_try_and_get_and_verify_type(unit_index,
		_object_mask_biped);
	struct device_datum *elevator;

	if (!biped || TEST_FLAG(biped->biped.flags, _biped_airborne_bit) ||
		biped->biped.support_surface_index == NONE || biped->biped.elevator_ticks != 60)
		return NONE;
	elevator = (struct device_datum *)object_try_and_get_and_verify_type(biped->biped.elevator_object_index,
		_object_mask_machine);
	return elevator ? biped->biped.elevator_object_index : NONE;
}

/* whether a state's deck part is whole here: its vehicle one this machine has, its node one of that vehicle's */
static boolean distributed_deck_whole(
	struct distributed_unit_state const *state)
{
	return state->deck_present &&
		vehicle_riding_deck_valid(&state->deck, biped_riding_vehicle_node_count(state->deck.vehicle_index));
}

/* a remote rider's place belongs to this machine's copy of its deck */
static void distributed_apply_remote_unit(
	long unit_index,
	struct distributed_unit_state const *state)
{
	struct vehicle_riding_deck const *deck = &state->deck;
	real_point3d target;
	real_point3d base;
	real_point3d target_base;
	real_vector3d frame;
	real_vector3d velocity;
	real height;
	real width;

	/* (the host's cabin verdict is taken before any branch below can return: a rider within tolerance carries it too) */
	biped_riding_cabin_host(unit_index, distributed_deck_whole(state) ? deck : NULL, TRUE);
	if (state->deck_present && deck->ticks == RIDING_TICKS &&
		vehicle_riding_deck_valid(deck, biped_riding_vehicle_node_count(deck->vehicle_index)) &&
		biped_riding_deck_position(unit_index, deck, &target))
	{
		biped_get_physics_pill(unit_index, &base, &height, &width);
		/* (the frame speed is measured at the target pill base, not the object origin) */
		target_base.x = target.x + base.x - object_get(unit_index)->object.position.x;
		target_base.y = target.y + base.y - object_get(unit_index)->object.position.y;
		target_base.z = target.z + base.z - object_get(unit_index)->object.position.z;
		if (biped_riding_frame_velocity(unit_index, deck, &target_base, &frame))
		{
			struct vehicle_riding_state riding;
			real_point3d before = object_get(unit_index)->object.position;
			real tolerance = REMOTE_CORRECTION_TOLERANCE;
			boolean same_window = biped_riding_get_state(unit_index, &riding) && riding.ticks > 0 &&
				riding.object_index == deck->vehicle_index && riding.node_index == deck->node_index;

			if (same_window)
			{
				real_point3d place;

				/* (only the saved-frame error decides a correction in this window) */
				tolerance = -1.0f;
				/* (the base has not yet taken the motion since this snapshot) */
				vehicle_riding_point_to_local(&riding.vehicle_position, &riding.vehicle_forward,
					&riding.vehicle_up, &base, &place);
				if (distance_squared3d(&place, &deck->place) <=
					REMOTE_CORRECTION_TOLERANCE * REMOTE_CORRECTION_TOLERANCE)
				{
					target = before;
					tolerance = 0.0f;
				}
			}
			add_vectors3d(&deck->velocity, &frame, &velocity);
			/* (an unchanged unit keeps the carry its current window has not taken yet) */
			if (distributed_apply_state(unit_index, state, &target, &velocity, tolerance,
				REMOTE_BLEND_DISTANCE, 0.0f, 0.0f) &&
				(!same_window || tolerance < 0.0f))
			{
				biped_riding_take_deck(unit_index, deck);
				/* (a window this word opened has no cabin yet: the verdict seeds it) */
				biped_riding_cabin_host(unit_index, deck, TRUE);
			}
			return;
		}
	}
	else if (state->deck_present && deck->ticks < RIDING_TICKS &&
		vehicle_riding_deck_valid(deck, biped_riding_vehicle_node_count(deck->vehicle_index)))
	{
		/* (in the air the host's own place stands: the window's anchor is the body's, taken after) */
		distributed_vector_unpack(&state->velocity, DISTRIBUTED_VELOCITY_SCALE, &velocity);
		distributed_apply_state(unit_index, state, &state->position, &velocity, REMOTE_CORRECTION_TOLERANCE,
			REMOTE_BLEND_DISTANCE, 0.0f, 0.0f);
		biped_riding_take_deck(unit_index, deck);
		biped_riding_cabin_host(unit_index, deck, TRUE);
		return;
	}
	biped_riding_drop(unit_index);
	distributed_apply_state(unit_index, state, &state->position, NULL, REMOTE_CORRECTION_TOLERANCE,
		REMOTE_BLEND_DISTANCE, 0.0f, 0.0f);
}

/* (a client) where its own players' units are this tick, noted */
static void distributed_note_own_positions(
	void)
{
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct player_datum *player = player_index != NONE ? player_try_and_get(player_index) : NULL;
		long unit_index = distributed_living_unit(player);
		struct distributed_own_position *own =
			&distributed_own_positions[local_player_index][game_time_get() & (OWN_POSITION_TICKS - 1)];
		struct distributed_own_position const *previous =
			&distributed_own_positions[local_player_index][(game_time_get() - 1) & (OWN_POSITION_TICKS - 1)];
		boolean continued;

		/* (not riding: a rider's position is its seat's) */
		if (unit_index != NONE && object_get(unit_index)->object.parent_object_index != NONE)
			unit_index = NONE;
		own->time = game_time_get();
		own->unit_index = unit_index;
		own->elevator_index = unit_index != NONE ? distributed_supported_elevator(unit_index) : NONE;
		own->deck_present = unit_index != NONE && biped_riding_deck(unit_index, &own->deck);
		continued = previous->time == game_time_get() - 1 && previous->unit_index == unit_index &&
			previous->deck_present && own->deck_present && previous->deck.vehicle_index == own->deck.vehicle_index;
		if (own->deck_present && !continued)
			distributed_own_windows[local_player_index]++;
		if (own->deck_present && own->deck.ticks < RIDING_TICKS &&
			(!continued || previous->deck.ticks == RIDING_TICKS))
			distributed_own_air_episodes[local_player_index]++;
		own->window = distributed_own_windows[local_player_index];
		own->air_episode = distributed_own_air_episodes[local_player_index];
		own->air_baseline_valid = TRUE;
		if (unit_index != NONE)
			own->position = object_get(unit_index)->object.position;
	}
}

/* (a client) whether the tick a host's word of its own player answers (the
echoed one) was in the window the player is in now: a word of an older window
of the same vehicle says nothing of this one, nor does a jump's word of an
older air episode; none without an echoed tick */
static boolean distributed_own_window_current(
	struct player_datum const *player,
	long unit_index,
	struct distributed_unit_state const *state)
{
	short local_player_index = player->local_player_index;
	struct vehicle_riding_deck current_deck;
	struct distributed_own_position const *own;
	long now = game_time_get();
	long time;

	if (!TEST_FLAG(state->flags, _distributed_unit_predicted_bit) ||
		local_player_index < 0 || local_player_index >= MAXIMUM_LOCAL_PLAYERS)
		return FALSE;
	time = now - (long)(word)((word)now - (word)state->predicted_time);
	own = &distributed_own_positions[local_player_index][time & (OWN_POSITION_TICKS - 1)];
	if (own->time != time || own->unit_index != unit_index)
		return FALSE;
	return !own->deck_present || (own->window == distributed_own_windows[local_player_index] &&
		(state->deck.ticks >= RIDING_TICKS ||
		own->air_episode == distributed_own_air_episodes[local_player_index]) &&
		biped_riding_deck(unit_index, &current_deck) && current_deck.vehicle_index == own->deck.vehicle_index);
}

/* an airborne word needs an actual deck capture, never the pre-acquisition shortcut */
static boolean distributed_own_episode_current(
	struct player_datum const *player,
	long unit_index,
	struct distributed_unit_state const *state)
{
	short local_player_index = player->local_player_index;
	struct distributed_own_position const *own;
	long now = game_time_get();
	long time;

	if (!state->deck_present || !distributed_own_window_current(player, unit_index, state))
		return FALSE;
	time = now - (long)(word)((word)now - (word)state->predicted_time);
	own = &distributed_own_positions[local_player_index][time & (OWN_POSITION_TICKS - 1)];
	return own->deck_present && own->deck.vehicle_index == state->deck.vehicle_index;
}

/* (a client) its own riding player's unit, where the host has it on the deck
this machine has it riding too, when the state carries no tick of this
machine's own to compare with: judged by its place on the deck, never by the
world places of two copies of a moving vehicle, which a round trip's lag
holds apart by more than the tolerance. False when the unit or the state is
not of one deck (the world correction is then what is left) */
static boolean distributed_correct_own_unechoed_rider(
	long unit_index,
	struct distributed_unit_state const *state)
{
	struct vehicle_riding_state riding;
	struct vehicle_riding_deck current_deck;
	struct vehicle_riding_deck target_deck;
	struct object_datum *object = object_get(unit_index);
	real_vector3d deck_error;
	real_vector3d frame;
	real_vector3d velocity;
	real_point3d before = object->object.position;
	real_point3d position;
	real_point3d base;
	real height;
	real width;

	if (!distributed_deck_whole(state) || state->deck.ticks != RIDING_TICKS ||
		!biped_riding_get_state(unit_index, &riding) || !vehicle_riding_is_riding(&riding) ||
		!biped_riding_deck(unit_index, &current_deck) || current_deck.ticks != RIDING_TICKS ||
		current_deck.vehicle_index != state->deck.vehicle_index || current_deck.node_index != state->deck.node_index)
		return FALSE;
	vector_from_points3d(&current_deck.place, &state->deck.place, &deck_error);
	if (!(magnitude_squared3d(&deck_error) > LOCAL_CORRECTION_TOLERANCE * LOCAL_CORRECTION_TOLERANCE))
		return TRUE;
	target_deck = current_deck;
	target_deck.place.x += deck_error.i;
	target_deck.place.y += deck_error.j;
	target_deck.place.z += deck_error.k;
	if (!biped_riding_deck_position(unit_index, &target_deck, &position))
		return TRUE;
	biped_get_physics_pill(unit_index, &base, &height, &width);
	base.x += position.x - before.x;
	base.y += position.y - before.y;
	base.z += position.z - before.z;
	if (!biped_riding_frame_velocity(unit_index, &target_deck, &base, &frame))
		return TRUE;
	add_vectors3d(&state->deck.velocity, &frame, &velocity);
	if (!distributed_apply_state(unit_index, state, &position, &velocity, LOCAL_CORRECTION_TOLERANCE, 0.0f, 0.0f, 0.0f))
		return TRUE;
	biped_riding_rebase(unit_index);
	return TRUE;
}

/* (a client) its own player's unit where the host has it, if that is
further than a tolerance from where this machine had it at the tick the
host has it at (its own prediction come back): moved by the difference,
keeping what it has done since (no rubber band a round trip long); without
that tick, from where it is */
static void distributed_correct_own_unit(
	struct player_datum const *player,
	long unit_index,
	struct distributed_unit_state const *state)
{
	short local_player_index = player->local_player_index;
	real_point3d position = state->position;
	struct vehicle_riding_claim *claim = vehicle_riding_player_claim(state->player_index);
	long host_vehicle = state->deck_present ? state->deck.vehicle_index : NONE;
	short host_node = state->deck_present ? state->deck.node_index : NONE;
	boolean current_episode = distributed_own_episode_current(player, unit_index, state);

	/* (the host's cabin verdict is taken before any branch below can return: an echo within tolerance carries it too) */
	biped_riding_cabin_host(unit_index, distributed_deck_whole(state) ? &state->deck : NULL,
		state->deck_present && state->deck.ticks < RIDING_TICKS &&
		!TEST_FLAG(state->deck.flags, _vehicle_riding_deck_cabin_bit) ? current_episode :
		distributed_own_window_current(player, unit_index, state));
	biped_riding_air_verdict_host(unit_index, distributed_deck_whole(state) ? &state->deck : NULL, current_episode);
	/* (the host ended this claim, even when no standing-history branch will run) */
	if (claim && claim->taken && (claim->vehicle_index != host_vehicle || claim->node_index != host_node))
	{
		claim->taken = FALSE;
		claim->disagreement = 0;
	}

	/* (co-op: the host has its player in a BSP this machine hasn't loaded
	yet, the switch still on its way. Moved there, it would fall with no
	floor under it, so it stays on the floor it has until the BSP loads.
	Tested at the unit's middle, not its feet: a teleport can leave the feet
	a hair below the floor's plane, which is outside the BSP.) */
	if (network_coop_active())
	{
		struct object_datum *object = object_get(unit_index);
		real_point3d host_middle;

		host_middle.x = state->position.x + object->object.bounding_sphere_center.x - object->object.position.x;
		host_middle.y = state->position.y + object->object.bounding_sphere_center.y - object->object.position.y;
		host_middle.z = state->position.z + object->object.bounding_sphere_center.z - object->object.position.z;
		if (scenario_leaf_index_from_point(&host_middle) == NONE &&
			scenario_leaf_index_from_point(&object->object.bounding_sphere_center) != NONE)
		{
			return;
		}
	}
	if (!state->scripted_pose && TEST_FLAG(state->flags, _distributed_unit_predicted_bit) &&
		local_player_index >= 0 && local_player_index < MAXIMUM_LOCAL_PLAYERS)
	{
		long now = game_time_get();
		long time = now - (long)(word)((word)now - (word)state->predicted_time);
		struct distributed_own_position *own =
			&distributed_own_positions[local_player_index][time & (OWN_POSITION_TICKS - 1)];

		if (own->time == time && own->unit_index == unit_index)
		{
			struct object_datum *object = object_get(unit_index);
			real_vector3d error;
			short index;
			struct vehicle_riding_deck current_deck;
			boolean current_present = biped_riding_deck(unit_index, &current_deck);
			boolean own_riding = own->deck_present && own->deck.ticks == RIDING_TICKS;
			boolean host_riding = state->deck_present && state->deck.ticks == RIDING_TICKS;
			boolean host_air = state->deck_present && state->deck.ticks > 0 && state->deck.ticks < RIDING_TICKS &&
				vehicle_riding_deck_valid(&state->deck, biped_riding_vehicle_node_count(host_vehicle));
			boolean dropping = FALSE;
			boolean air_claim = FALSE;
			boolean stale_window = own->deck_present && (own->window != distributed_own_windows[local_player_index] ||
				!current_present || current_deck.vehicle_index != own->deck.vehicle_index);

			/* (how long the host takes to have this machine's players, as
			the round trip is smoothed on the host) */
			if (distributed_own_round_trip <= 0.0f)
				distributed_own_round_trip = (real)(now - time);
			else
				distributed_own_round_trip += ((real)(now - time) - distributed_own_round_trip) / 8.0f;
			distributed_note_own_round_trip(time);
			if (host_air && current_present && current_deck.ticks == RIDING_TICKS &&
				current_episode && current_deck.vehicle_index == host_vehicle && claim && !claim->taken)
			{
				short grace = (short)MAX(SEAT_DISAGREEMENT_TICKS,
					(short)ceil(distributed_own_round_trip) + SEAT_DISAGREEMENT_SLACK_TICKS);
				struct vehicle_riding_claim pending;

				pending = *claim;
				/* (standing is not an air claim, even on the same vehicle and body node) */
				air_claim = vehicle_riding_claim_grace(&pending, NONE, NONE, host_vehicle, host_node, grace);
				if (air_claim)
				{
					pending.taken = FALSE;
					pending.disagreement = grace;
				}
				*claim = pending;
				if (!air_claim)
					return;
			}
			if ((own_riding || host_riding) && !host_air)
			{
				struct vehicle_riding_state riding;
				long own_vehicle = own->deck_present ? own->deck.vehicle_index : NONE;
				short own_node = own->deck_present ? own->deck.node_index : NONE;
				short grace = (short)MAX(SEAT_DISAGREEMENT_TICKS,
					(short)ceil(distributed_own_round_trip) + SEAT_DISAGREEMENT_SLACK_TICKS);

				/* (dead or seated units take no deck transaction) */
				if (!biped_riding_get_state(unit_index, &riding))
					return;
				/* (an old window can judge only the claim held now, never its historical place) */
				if (stale_window)
				{
					own_vehicle = current_present ? current_deck.vehicle_index : NONE;
					own_node = current_present ? current_deck.node_index : NONE;
					if (own_vehicle == host_vehicle && own_node == host_node)
						vehicle_riding_claim_grace(claim, own_vehicle, own_node, host_vehicle, host_node, grace);
				}
				if (host_riding && (host_vehicle != own_vehicle || host_node != own_node))
				{
					real_point3d target;
					real_point3d base;
					real_vector3d frame;
					real_vector3d velocity;
					real height;
					real width;
					struct vehicle_riding_claim pending;
					boolean taking;

					/* (crossing claims are never distances between two world frames) */
					if (!claim)
						return;
					pending = *claim;
					taking = vehicle_riding_claim_grace(&pending, own_vehicle, own_node, host_vehicle, host_node, grace);
					if (taking)
					{
						/* (expiry stays pending until both the position and riding state take) */
						pending.taken = FALSE;
						pending.disagreement = grace;
					}
					*claim = pending;
					if (taking &&
						vehicle_riding_deck_valid(&state->deck, biped_riding_vehicle_node_count(host_vehicle)) &&
						biped_riding_deck_position(unit_index, &state->deck, &target))
					{
						biped_get_physics_pill(unit_index, &base, &height, &width);
						base.x += target.x - object->object.position.x;
						base.y += target.y - object->object.position.y;
						base.z += target.z - object->object.position.z;
						if (biped_riding_frame_velocity(unit_index, &state->deck, &base, &frame))
						{
							add_vectors3d(&state->deck.velocity, &frame, &velocity);
							if (distributed_apply_state(unit_index, state, &target, &velocity, 0.0f, 0.0f, 0.0f, 0.0f) &&
								biped_riding_take_deck(unit_index, &state->deck))
							{
								pending.taken = TRUE;
								pending.disagreement = 0;
								*claim = pending;
							}
						}
					}
					return;
				}
				/* (an old acquisition cannot move or seed the current window) */
				if (stale_window)
					return;
				if (own_riding && host_riding && own_node == host_node)
				{
					struct vehicle_riding_deck target_deck = current_deck;
					real_vector3d deck_error;
					real_vector3d frame;
					real_vector3d velocity;
					real_point3d before = object->object.position;
					real_point3d base;
					real height;
					real width;

					vehicle_riding_claim_grace(claim, own_vehicle, own_node, host_vehicle, host_node, grace);
					if (current_deck.ticks != RIDING_TICKS || current_deck.node_index != own_node)
						return;
					vector_from_points3d(&own->deck.place, &state->deck.place, &deck_error);
					if (!(magnitude_squared3d(&deck_error) > LOCAL_CORRECTION_TOLERANCE * LOCAL_CORRECTION_TOLERANCE))
						return;
					target_deck.place.x += deck_error.i;
					target_deck.place.y += deck_error.j;
					target_deck.place.z += deck_error.k;
					if (!biped_riding_deck_position(unit_index, &target_deck, &position))
						return;
					biped_get_physics_pill(unit_index, &base, &height, &width);
					base.x += position.x - before.x;
					base.y += position.y - before.y;
					base.z += position.z - before.z;
					if (!biped_riding_frame_velocity(unit_index, &target_deck, &base, &frame))
						return;
					add_vectors3d(&state->deck.velocity, &frame, &velocity);
					if (!distributed_apply_state(unit_index, state, &position, &velocity, LOCAL_CORRECTION_TOLERANCE, 0.0f, 0.0f, 0.0f))
						return;
					biped_riding_rebase(unit_index);
					vector_from_points3d(&before, &object->object.position, &error);
					/* (both histories moved by this take, including ticks echoed later) */
					for (index = 0; index < OWN_POSITION_TICKS; index++)
					{
						struct distributed_own_position *noted = &distributed_own_positions[local_player_index][index];
						if (noted->unit_index != unit_index)
							continue;
						noted->position.x += error.i;
						noted->position.y += error.j;
						noted->position.z += error.k;
						if (noted->deck_present && noted->window == own->window && noted->deck.vehicle_index == own_vehicle)
						{
							noted->deck.place.x += deck_error.i;
							noted->deck.place.y += deck_error.j;
							noted->deck.place.z += deck_error.k;
						}
					}
					return;
				}
				if (own_riding && !state->deck_present)
				{
					biped_riding_drop(unit_index);
					dropping = TRUE;
				}
			}
			error.i = state->position.x - own->position.x;
			error.j = state->position.y - own->position.y;
			error.k = state->position.z - own->position.z;
			/* The host echoes our prediction on its own elevator's floor. If
			we still stand on that platform, a small height error is its phase
			lag, not player motion. Keep local floor contact; teleports across,
			a height error past the lag, a jump, a different platform and
			unpredicted states correct normally. */
			if (!dropping && !air_claim && own->elevator_index != NONE &&
				own->elevator_index == distributed_supported_elevator(unit_index) &&
				error.i * error.i + error.j * error.j <=
					LOCAL_CORRECTION_TOLERANCE * LOCAL_CORRECTION_TOLERANCE &&
				error.k >= -ELEVATOR_PHASE_TOLERANCE && error.k <= ELEVATOR_PHASE_TOLERANCE)
				error.k = 0.0f;
			if (!dropping && !air_claim && !(error.i * error.i + error.j * error.j + error.k * error.k >
				LOCAL_CORRECTION_TOLERANCE * LOCAL_CORRECTION_TOLERANCE))
			{
				return;
			}
			{
				real_point3d before = object->object.position;
				real_vector3d air_difference;
				real_vector3d world_velocity = object->object.translational_velocity;
				boolean air_rebased = FALSE;
				boolean air_taken = FALSE;

				position.x = before.x + error.i;
				position.y = before.y + error.j;
				position.z = before.z + error.k;
				if (dropping && current_present)
				{
					struct vehicle_riding_vehicle vehicle;
					real_point3d base;
					real height;
					real width;
					struct vehicle_riding_deck target_deck = current_deck;

					if (biped_riding_vehicle(current_deck.vehicle_index, current_deck.node_index, &vehicle))
					{
						biped_get_physics_pill(unit_index, &base, &height, &width);
						base.x += position.x - before.x;
						base.y += position.y - before.y;
						base.z += position.z - before.z;
						vehicle_riding_point_to_local(&vehicle.position, &vehicle.forward, &vehicle.up, &base, &target_deck.place);
						/* (a late drop cannot put the pill through a rising floor) */
						if (target_deck.place.z < current_deck.place.z)
							target_deck.place.z = current_deck.place.z;
						biped_riding_deck_position(unit_index, &target_deck, &position);
					}
				}
				if (!distributed_apply_state(unit_index, state, &position, host_air ? &world_velocity : NULL,
					(dropping || air_claim) ? 0.0f : LOCAL_CORRECTION_TOLERANCE, 0.0f, 0.0f, 0.0f))
					return;
				if (air_claim)
				{
					/* (the host ends standing and supplies the whole air frame in one take) */
					if (!biped_riding_air_take(unit_index, &state->deck))
						return;
					biped_riding_drop(unit_index);
					if (!biped_riding_take_deck(unit_index, &state->deck))
						return;
					air_taken = TRUE;
					claim->taken = TRUE;
					claim->disagreement = 0;
				}
				else if (host_air)
				{
					/* (only one jump's airborne echo is a frame velocity, never a standing relative velocity) */
					if (own->deck_present && own->deck.ticks > 0 && own->deck.ticks < RIDING_TICKS &&
						own->air_baseline_valid &&
						!stale_window && own->air_episode == distributed_own_air_episodes[local_player_index] &&
						own->deck.vehicle_index == host_vehicle)
					{
						subtract_vectors3d(&state->deck.velocity, &own->deck.velocity, &air_difference);
						air_rebased = biped_riding_air_correct(unit_index, &air_difference, state->deck.peak,
							(short)((state->deck.flags >> VEHICLE_RIDING_DECK_VERDICT_SHIFT) & VEHICLE_RIDING_DECK_VERDICT_MASK));
					}
					else
						air_taken = biped_riding_air_take(unit_index, &state->deck);
				}
				/* (the ticks noted since moved as it was, not corrected again) */
				error.i = object->object.position.x - before.x;
				error.j = object->object.position.y - before.y;
				error.k = object->object.position.z - before.z;
				for (index = 0; index < OWN_POSITION_TICKS; index++)
				{
					struct distributed_own_position *noted = &distributed_own_positions[local_player_index][index];

					if (noted->unit_index == unit_index)
					{
						noted->position.x += error.i;
						noted->position.y += error.j;
						noted->position.z += error.k;
						if (air_rebased && noted->deck_present && noted->deck.ticks > 0 && noted->deck.ticks < RIDING_TICKS &&
							noted->window == own->window && noted->air_episode == own->air_episode &&
							noted->deck.vehicle_index == host_vehicle)
							add_vectors3d(&noted->deck.velocity, &air_difference, &noted->deck.velocity);
						/* (a whole take supersedes this jump's old differences, not its stored frame values) */
						if (air_taken && current_present && noted->deck_present &&
							(air_claim || (noted->deck.ticks > 0 && noted->deck.ticks < RIDING_TICKS)) &&
							noted->window == distributed_own_windows[local_player_index] &&
							noted->air_episode == distributed_own_air_episodes[local_player_index] &&
							noted->deck.vehicle_index == current_deck.vehicle_index)
							noted->air_baseline_valid = FALSE;
					}
				}
				if (air_claim)
					distributed_own_windows[local_player_index]++;
				return;
			}
		}
	}
	/* (no echoed tick to judge by: a rider is judged on its deck all the same) */
	if (distributed_correct_own_unechoed_rider(unit_index, state))
		return;
	distributed_apply_state(unit_index, state, &position, NULL, LOCAL_CORRECTION_TOLERANCE, 0.0f, 0.0f, 0.0f);
}

/* (a client) its own players' units, to the host */
static void distributed_client_send_predictions(
	void)
{
	struct data_iterator iterator;
	struct player_datum *player;

	distributed_packer_begin(&distributed_packer, HOST_SENDER, _distributed_message_player_prediction, NULL, 0);
	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		long unit_index = distributed_living_unit(player);
		struct distributed_unit_state state;
		byte entry[DISTRIBUTED_UNIT_STATE_MAXIMUM_SIZE];

		/* a client speaks for its own players only, where they are */
		if (player->local_player_index == NONE || unit_index == NONE ||
			object_get(unit_index)->object.parent_object_index != NONE)
		{
			continue;
		}
		distributed_state_from_player((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index), &state);
		distributed_packer_add(&distributed_packer, entry, distributed_unit_state_write(&state, TRUE, entry));
	}
	distributed_packer_flush(&distributed_packer);
}

/* (the host) a client's own players: the latest of each, taken at the next
tick */
static void distributed_handle_predictions(
	long machine_index,
	long time,
	byte const *data,
	byte const *end,
	short count)
{
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_unit_state state;
		word size = distributed_unit_state_read(data, end, &state);

		if (!size)
			break;
		data += size;
		/* Scripted poses and suspension belong to the host, never predictions. */
		if (state.player_index >= MAXIMUM_TRACKED_PLAYERS ||
			!distributed_machine_has_player(machine_index, state.player_index) ||
			state.scripted_pose ||
			TEST_FLAG(state.unit_flags, _distributed_unit_suspended_bit) ||
			!TEST_FLAG(state.flags, _distributed_unit_alive_bit) ||
			!TEST_FLAG(state.flags, _distributed_unit_placed_bit))
		{
			continue;
		}
		distributed_predictions[state.player_index].valid = TRUE;
		distributed_predictions[state.player_index].time = time;
		distributed_predictions[state.player_index].state = state;
	}
}

/* (the host) how fast a client's player's unit on foot may go, a tick, and
how high (distributed_on_foot_bound) */
struct distributed_on_foot_bound
{
	/* across and up (world units a tick) */
	real speed;
	real frame_speed;
	/* the highest the host has had it since it last had it on the ground,
	and how far above where it had it then it may be (world units) */
	real top_height;
	real rise;
	/* ... in the air, the speed up it may have left the ground at and the
	height it may have had then, and the ticks the host's copy has been in
	the air (none on the ground): as high as a thing thrown up so, falling
	since (distributed_on_foot_ceiling) */
	real rise_speed;
	real rise_base;
	long airborne_ticks;
};

/* (the host) how fast a client's player's unit on foot may go, a tick, and
how high: across and up twice as fast as the player runs and jumps (their
speed in the game, double speed), or as fast as the host's own ticks sent
its copy lately (an explosion's throw, which the client learns of a round
trip late), whichever is more, no more than MAXIMUM_PREDICTED_SPEED; down
as fast as that and a fall from the highest it has had it since it was on
the ground (distributed_on_foot_fall_speed); up no higher above where it
last had it on the ground than a jump takes it (or a run up a slope, and
the legs drawn up) and what its ticks threw it up. What its ticks sent it
is its speed less as much as the velocity it took of the client was faster
than the player goes of their own, and up what its tick added to that, but
a jump's: a client that says it goes faster than that gains nothing by it
(its gravity on a copy said to hover and fall, its turn at a wall), nor
any height. */
static void distributed_riding_transition(short player_index, long unit_index, struct vehicle_riding_state const *host);

static void distributed_on_foot_bound(
	short player_index,
	long unit_index,
	real_vector3d const *frame_velocity,
	struct distributed_on_foot_bound *bound)
{
	struct distributed_host_speed *noted = &distributed_host_speeds[player_index];
	struct object_datum *object = object_get(unit_index);
	struct biped_datum *biped = (struct biped_datum *)object_try_and_get_and_verify_type(unit_index,
		_object_mask_biped);
	struct player_datum *player = distributed_player(player_index);
	struct game_globals *globals = scenario_get_game_globals();
	long now = game_time_get();
	real_vector3d const *velocity = &object->object.translational_velocity;
	real_vector3d relative;
	real_vector3d taken_relative;
	struct vehicle_riding_state riding;
	boolean riding_applies = biped_riding_get_state(unit_index, &riding);
	boolean window = riding_applies && riding.ticks > 0;
	long vehicle_index = window ? riding.object_index : NONE;
	real host_speed;
	real taken_speed;
	struct game_globals_player_information *information = NULL;
	real run_speed;
	real jump_speed = 0.0f;
	real crouch_rise = 0.0f;
	real base_speed;
	real added_rise;
	real rise_speed;

	/* (the host's transitions count even when a client's words stop) */
	if (riding_applies)
		distributed_riding_transition(player_index, unit_index, &riding);
	subtract_vectors3d(velocity, frame_velocity, &relative);
	subtract_vectors3d(&noted->velocity, &noted->frame_velocity, &taken_relative);
	host_speed = magnitude3d(&relative);
	taken_speed = magnitude3d(&taken_relative);
	/* (a unit new on foot, a new life or a ride's end: all its speed its
	ticks', from where it is) */
	if (noted->unit_index != unit_index)
	{
		noted->unit_index = unit_index;
		noted->velocity.i = 0.0f;
		noted->velocity.j = 0.0f;
		noted->velocity.k = 0.0f;
		noted->frame_velocity.i = 0.0f;
		noted->frame_velocity.j = 0.0f;
		noted->frame_velocity.k = 0.0f;
		taken_relative = noted->velocity;
		noted->window = FALSE;
		noted->vehicle_index = NONE;
		noted->grounded = FALSE;
		noted->flight_speed = 0.0f;
		noted->flight_rise = 0.0f;
		noted->ground_height = object->object.position.z;
		noted->ground_frame_k = frame_velocity->k;
		noted->top_height = object->object.position.z;
		noted->ground_time = now;
		taken_speed = 0.0f;
	}
	/* (a boarding or a window's end is not a throw in either frame) */
	if (noted->window != window || (window && noted->vehicle_index != vehicle_index))
	{
		noted->velocity = *velocity;
		noted->frame_velocity = *frame_velocity;
		taken_relative = relative;
		taken_speed = host_speed;
	}
	noted->window = window;
	noted->vehicle_index = vehicle_index;
	/* (the most of each span) */
	if (now < noted->time || now - noted->time >= 2 * PREDICTION_SPEED_TICKS)
	{
		noted->time = now;
		noted->speeds[0] = 0.0f;
		noted->speeds[1] = 0.0f;
		noted->rises[0] = 0.0f;
		noted->rises[1] = 0.0f;
	}
	else if (now - noted->time >= PREDICTION_SPEED_TICKS)
	{
		noted->time = now;
		noted->speeds[1] = noted->speeds[0];
		noted->speeds[0] = 0.0f;
		noted->rises[1] = noted->rises[0];
		noted->rises[0] = 0.0f;
	}
	if (globals && globals->player_information.count > 0)
	{
		information = TAG_BLOCK_GET_ELEMENT(&globals->player_information, 0,
			struct game_globals_player_information);
	}
	if (biped)
	{
		struct biped_definition *definition = biped_definition_get(biped->definition_index);

		jump_speed = definition->biped.jump_velocity;
		crouch_rise = definition->biped.collision_height_standing - definition->biped.collision_height_crouching;
	}
	/* (the run alone is also the rise allowed up a slope) */
	run_speed = vehicle_riding_run_jump_speed(information != NULL,
		information ? information->run_forward_speed : 0.f, information ? information->run_backward_speed : 0.f,
		information ? information->run_sideways_speed : 0.f, game_players_are_double_speed(),
		information ? information->double_speed_multiplier : 1.f, player ? player->speed_multiplier : 1.f, 0.f);
	base_speed = PREDICTED_ON_FOOT_SPEED_SCALE * vehicle_riding_run_jump_speed(information != NULL,
		information ? information->run_forward_speed : 0.f, information ? information->run_backward_speed : 0.f,
		information ? information->run_sideways_speed : 0.f, game_players_are_double_speed(),
		information ? information->double_speed_multiplier : 1.f, player ? player->speed_multiplier : 1.f, jump_speed);
	/* (so written that a tag's speed not a number is none) */
	if (!(base_speed >= 0.0f))
		base_speed = 0.0f;
	if (!(jump_speed >= 0.0f))
		jump_speed = 0.0f;
	if (!(crouch_rise >= 0.0f))
		crouch_rise = 0.0f;
	/* (what its tick sent it, noted; so written that a speed not a number
	is none) */
	if (taken_speed > base_speed)
		host_speed -= taken_speed - base_speed;
	added_rise = MAX(relative.k, 0.0f) - MAX(taken_relative.k, 0.0f);
	if (noted->grounded)
		added_rise -= jump_speed;
	if (host_speed > noted->speeds[0])
		noted->speeds[0] = host_speed;
	if (added_rise > noted->rises[0])
		noted->rises[0] = added_rise;
	/* (how high: where it is on the ground, or the highest since; and in
	the air, a throw's speeds kept till it lands, a few seconds at most) */
	noted->grounded = !biped || !TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
	if (noted->grounded)
	{
		noted->ground_height = object->object.position.z;
		/* (the falling frame can change in flight; the takeoff ground cannot) */
		noted->ground_frame_k = frame_velocity->k;
		noted->ground_time = now;
		if (window && vehicle_riding_is_riding(&riding))
		{
			real_point3d base, place;
			real height, width;

			biped_get_physics_pill(unit_index, &base, &height, &width);
			vehicle_riding_point_to_local(&riding.body_position, &riding.body_forward, &riding.body_up, &base, &place);
			noted->ground_place_height = place.z;
		}
	}
	if (noted->grounded || object->object.position.z > noted->top_height)
		noted->top_height = object->object.position.z;
	if (noted->grounded || biped->biped.airborne_ticks >= SCHAR_MAX)
	{
		noted->flight_speed = 0.0f;
		noted->flight_rise = 0.0f;
	}
	else
	{
		noted->flight_speed = MAX(noted->flight_speed, MAX(noted->speeds[0], noted->speeds[1]));
		noted->flight_rise = MAX(noted->flight_rise, MAX(noted->rises[0], noted->rises[1]));
	}
	bound->speed = MAX(base_speed, MAX(MAX(noted->speeds[0], noted->speeds[1]), noted->flight_speed));
	/* (some: none is no bound, distributed_apply_state) */
	bound->speed = MAX(bound->speed, PREDICTED_ON_FOOT_SPEED_MARGIN);
	bound->speed = MIN(bound->speed, MAXIMUM_PREDICTED_SPEED);
	/* (only flight uses a world bound; a deck still measures relative motion) */
	bound->frame_speed = window && !vehicle_riding_is_riding(&riding) ? magnitude3d(&riding.velocity) : 0.f;
	bound->speed += bound->frame_speed;
	bound->top_height = noted->top_height;
	/* (a jump's rise, as fast up as it or a run up a slope is: its speed
	squared over twice gravity) */
	rise_speed = MAX(MAX(noted->rises[0], noted->rises[1]), noted->flight_rise);
	/* (a throw of it on the ground was noted less a jump, which its own
	client may have made as it came: a jump's again on top) */
	if (rise_speed > 0.0f)
		rise_speed += jump_speed;
	rise_speed += MAX(jump_speed, run_speed);
	bound->rise = UNIT_WORLD_BOUND;
	if (biped && global_gravity > 0.0f)
		bound->rise = rise_speed * rise_speed / (2.0f * global_gravity) + crouch_rise + PREDICTED_RISE_TOLERANCE;
	bound->rise_speed = rise_speed;
	bound->rise_base = crouch_rise + PREDICTED_RISE_TOLERANCE;
	bound->airborne_ticks = noted->grounded ? 0 : now - noted->ground_time;
	if (!(bound->rise >= 0.0f))
		bound->rise = UNIT_WORLD_BOUND;
}

/* (the host) how fast a client's player's unit on foot may fall at a
height (world units a tick): as fast as it goes across, and as fast as a
fall from the highest the host has had it since it was on the ground, no
more than MAXIMUM_PREDICTED_SPEED */
static real distributed_on_foot_fall_speed(
	struct distributed_on_foot_bound const *bound,
	real height)
{
	real drop = bound->top_height - height;
	real speed = bound->speed;

	if (drop > 0.0f && global_gravity > 0.0f)
		speed += (real)sqrt(2.0f * global_gravity * drop);
	return MIN(speed, MAXIMUM_PREDICTED_SPEED + bound->frame_speed);
}

/* (the host) how far above where the host last had a client's player's
unit on the ground it may be: a jump's height (or a throw's), and in the
air no higher than a thing thrown up at that speed falls to since (the
ticks its copy has been in the air, less the client's round trip and
jitter, which are the client's ahead): no hovering in the air, nor coming
down slowly */
static real distributed_on_foot_ceiling(
	short player_index,
	struct distributed_on_foot_bound const *bound)
{
	real rise = bound->rise;
	/* (a round trip a client makes long gains it no more than this) */
	real ticks = (real)bound->airborne_ticks -
		MIN(distributed_machine_round_trip_ticks(distributed_player_machine(player_index)),
			(real)PREDICTION_CEILING_LEAD_TICKS) -
		(real)PREDICTION_JITTER_TICKS;

	if (bound->airborne_ticks > 0 && ticks > 0.0f && global_gravity > 0.0f)
	{
		real ceiling = bound->rise_base + bound->rise_speed * ticks - 0.5f * global_gravity * ticks * ticks;

		if (ceiling < rise)
			rise = ceiling;
	}
	return rise;
}

/* (the host) whether a client's player's unit on foot goes from one point
to the other in the ticks: across and up no further than at its speed, down
no further than it falls to there, a little more a tick (and a blend) */
static boolean distributed_on_foot_move_valid(
	struct distributed_on_foot_bound const *bound,
	real_point3d const *from,
	real_point3d const *to,
	long ticks)
{
	real dx = to->x - from->x;
	real dy = to->y - from->y;
	real dz = to->z - from->z;
	real rise = dz > 0.0f ? dz : 0.0f;
	real reach = MIN(bound->speed + PREDICTED_ON_FOOT_SPEED_MARGIN, MAXIMUM_PREDICTED_SPEED + bound->frame_speed) * (real)ticks +
		HOST_BLEND_DISTANCE;
	real fall_reach = MIN(distributed_on_foot_fall_speed(bound, to->z) + PREDICTED_ON_FOOT_SPEED_MARGIN,
		MAXIMUM_PREDICTED_SPEED + bound->frame_speed) * (real)ticks + HOST_BLEND_DISTANCE;

	/* (so written that a position not a number is not within) */
	return dx * dx + dy * dy + rise * rise <= reach * reach && -dz <= fall_reach;
}

/* (the host) whether a unit rides an elevator: one that is moving (bipeds.c's
elevator, kept a moment after the unit leaves its floor, as in a jump), or
one at rest whose floor it stands on, since a stop or reversal must not
admit a delayed height under the floor. The remembered elevator alone, after
leaving a stopped one, is not riding it. */
static boolean distributed_riding_elevator(
	long unit_index)
{
	struct biped_datum *biped = (struct biped_datum *)object_try_and_get_and_verify_type(unit_index,
		_object_mask_biped);
	struct device_datum *elevator;

	if (!biped || biped->biped.elevator_object_index == NONE)
		return FALSE;
	elevator = (struct device_datum *)object_try_and_get_and_verify_type(biped->biped.elevator_object_index,
		_object_mask_machine);
	return elevator && (elevator->device.position_velocity != 0.0f ||
		distributed_supported_elevator(unit_index) != NONE);
}

/* the vehicle a word's deck part has its rider airborne on (a jump's ticks, never
the riding or standing ones), or NONE */
static long distributed_air_part_vehicle(
	struct distributed_unit_state const *state)
{
	return state->deck_present && state->deck.ticks > 0 && state->deck.ticks < RIDING_TICKS ?
		state->deck.vehicle_index : NONE;
}

/* the host's transaction for a unit its copy has riding a vehicle: the word
is checked in the body's frame and no accepted state changes until every
check passes. A word with no deck part (or one that is not of this vehicle's
deck) is checked and kept the same way, so a client's part coming and going
changes nothing about the anchor it is held to */
static boolean distributed_take_riding(
	short player_index,
	long unit_index,
	struct distributed_on_foot_bound const *bound,
	boolean deck_part)
{
	struct distributed_unit_state const *state = &distributed_predictions[player_index].state;
	struct vehicle_riding_deck const *deck = &state->deck;
	struct object_datum *object = object_get(unit_index);
	struct vehicle_riding_state host;
	struct vehicle_riding_vehicle node, body, host_node;
	struct distributed_accepted_state accepted = distributed_accepted[player_index];
	long now = game_time_get();
	real_point3d base, host_place, place, host_now, anchor, taken_host;
	real_point3d target;
	real_vector3d world_velocity;
	real height, width;
	real ground_place_height = distributed_host_speeds[player_index].ground_place_height;
	boolean reset = FALSE;
	boolean leash;
	boolean landing;
	boolean keep_air;
	boolean client_node;
	short path;

	if (!biped_riding_get_state(unit_index, &host) || !biped_riding_vehicle(host.object_index, NONE, &body))
		return FALSE;
	biped_get_physics_pill(unit_index, &base, &height, &width);
	/* (the unit's last tick used this snapshot, before the vehicle pass) */
	vehicle_riding_point_to_local(&host.body_position, &host.body_forward, &host.body_up, &base, &host_place);
	vehicle_riding_point_to_world(&body.position, &body.forward, &body.up, &host_place, &host_now);
	/* (the word names the unit's position; the deck's names a pill base) */
	target = state->position;
	target.x += base.x - object->object.position.x;
	target.y += base.y - object->object.position.y;
	target.z += base.z - object->object.position.z;
	/* (a deck word of this vehicle is held to the host's own deck, never to the world: the leash
	would put the copy where the client's copy of the vehicle stood, off its support) */
	leash = now < accepted.transition_until && !deck_part &&
		vehicle_riding_host_leash(&target, &host_now, vehicle_riding_transition_leash());
	landing = deck_part && !leash && accepted.taken_air_unit_index == unit_index &&
		accepted.taken_air_vehicle == host.object_index;
	/* (the leash puts the copy at the word with no blend as well, so the riding word it takes is part of the same landing: the
	record waits for the deck take) */
	keep_air = leash && accepted.taken_air_unit_index == unit_index && accepted.taken_air_vehicle != NONE &&
		state->deck_present && state->deck.ticks == RIDING_TICKS && state->deck.vehicle_index == accepted.taken_air_vehicle;
	path = vehicle_riding_host_path(&host, state->deck_present, deck, leash);
	client_node = state->deck_present &&
		vehicle_riding_deck_valid(deck, biped_riding_vehicle_node_count(deck->vehicle_index)) &&
		biped_riding_vehicle(deck->vehicle_index, deck->node_index, &node);
	if (!biped_riding_vehicle(host.object_index, host.node_index, &host_node))
		return FALSE;
	vehicle_riding_host_target(path, client_node && !leash ? &node : NULL, &host_node,
		&host_now, deck, &target, &target);
	/* (a word without a part, or with an airborne part of another vehicle, names no place above the host's: the client's
	copy of the vehicle or its fall is behind the host's, so it would raise the rider off the deck. A riding part of another
	vehicle is a rider that stepped up onto it, and is followed) */
	if (vehicle_riding_is_riding(&host) && (!state->deck_present ||
		(deck->vehicle_index != host.object_index && deck->ticks != RIDING_TICKS)))
		vehicle_riding_host_unlifted(&host_node, &host_now, &target);
	vehicle_riding_point_to_local(&body.position, &body.forward, &body.up, &target, &place);
	if (!(distance_squared3d(&place, &host_place) <= HOST_ACCEPT_TOLERANCE * HOST_ACCEPT_TOLERANCE))
		return FALSE;
	if (accepted.valid && (accepted.unit_index != unit_index || accepted.vehicle_index != host.object_index))
	{
		accepted.valid = FALSE;
		ground_place_height = host_place.z;
		reset = TRUE;
	}
	if (accepted.valid)
	{
		long ticks = MIN(distributed_predictions[player_index].time - accepted.time,
			now - accepted.host_time + PREDICTION_JITTER_TICKS);

		/* (both ends carried by the same body now, never its world travel) */
		vehicle_riding_point_to_world(&body.position, &body.forward, &body.up, &accepted.taken_host_place, &taken_host);
		vehicle_riding_point_to_world(&body.position, &body.forward, &body.up, &accepted.place, &anchor);
		/* (in its window the host copy goes its own way in the world, never
		with the vehicle: a vehicle that brakes or snaps is no reason to
		start the anchor again, or a client could climb by each reset) */
		if (!vehicle_riding_is_riding(&host))
		{
			taken_host = accepted.taken_host_position;
			host_now = object->object.position;
		}
		if (!distributed_on_foot_move_valid(bound, &taken_host, &host_now, now - accepted.taken_host_time))
		{
			accepted.valid = FALSE;
			ground_place_height = host_place.z;
			reset = TRUE;
		}
		else if (ticks <= 0 || !distributed_on_foot_move_valid(bound, &anchor, &target, ticks))
		{
			return FALSE;
		}
	}
	/* (the part names a pill base; reconcile takes the object's position) */
	target.x -= base.x - object->object.position.x;
	target.y -= base.y - object->object.position.y;
	target.z -= base.z - object->object.position.z;
	if (leash)
	{
		/* (a position allowance never takes the client's velocity) */
		world_velocity = object->object.translational_velocity;
	}
	else if (deck_part)
	{
		vehicle_riding_host_velocity(&deck->velocity, &host.velocity, bound->speed,
			distributed_on_foot_fall_speed(bound, target.z), &world_velocity);
	}
	/* (in the air its ground travels at the host's kept takeoff velocity,
	not the deck's later motion. The bound's elapsed ticks carry that ground;
	the ceiling alone allows the client's lead and jitter in the jump) */
	if (!((vehicle_riding_is_riding(&host) ? place.z - ground_place_height :
		target.z - distributed_host_speeds[player_index].ground_height -
		distributed_host_speeds[player_index].ground_frame_k * (real)MAX(bound->airborne_ticks, 0)) <=
		distributed_on_foot_ceiling(player_index, bound)))
	{
		return FALSE;
	}
	if (deck_part || leash)
	{
		/* (the word's place passed every check, while the copy was put in the air by an aim at the client's world place (a world take, a leash take or an along route) off it by the two
		vehicle copies' offset: half of that would be the state this tick sends, and a word further than the blend
		would be put there in full anyway) */
		if (!distributed_apply_state(unit_index, state, &target, &world_velocity, 0.0f,
			leash || landing ? 0.f : HOST_BLEND_DISTANCE, 0.0f, 0.0f))
			return FALSE;
		if (leash)
			accepted.transition_leash_time = now;
		biped_riding_rebase(unit_index);
	}
	else if (!distributed_apply_state(unit_index, state, &target, NULL, 0.0f, HOST_BLEND_DISTANCE, bound->speed,
		distributed_on_foot_fall_speed(bound, target.z)))
	{
		return FALSE;
	}
	distributed_predictions[player_index].taken_time = distributed_predictions[player_index].time;
	distributed_predictions[player_index].taken_host_time = now;
	accepted.taken_air_unit_index = unit_index;
	if (!keep_air)
		accepted.taken_air_vehicle = distributed_air_part_vehicle(state);
	if (!accepted.valid || now - accepted.host_time >= PREDICTION_ANCHOR_TICKS)
	{
		accepted.valid = TRUE;
		accepted.unit_index = unit_index;
		accepted.time = distributed_predictions[player_index].time;
		accepted.host_time = now;
		accepted.position = target;
		accepted.vehicle_index = host.object_index;
		accepted.place = place;
	}
	accepted.taken_host_time = now;
	accepted.taken_host_position = object->object.position;
	biped_get_physics_pill(unit_index, &base, &height, &width);
	vehicle_riding_point_to_local(&body.position, &body.forward, &body.up, &base, &accepted.taken_host_place);
	distributed_accepted[player_index] = accepted;
	if (reset)
	{
		distributed_host_speeds[player_index].ground_place_height = ground_place_height;
		distributed_host_speeds[player_index].ground_height = object->object.position.z;
		distributed_host_speeds[player_index].top_height = object->object.position.z;
		distributed_host_speeds[player_index].ground_time = now;
	}
	return TRUE;
}

/* a client can ease one disagreement, but only the host can grant another */
static void distributed_riding_transition(
	short player_index,
	long unit_index,
	struct vehicle_riding_state const *host)
{
	struct distributed_accepted_state *accepted = &distributed_accepted[player_index];
	struct distributed_unit_state const *state = &distributed_predictions[player_index].state;
	short host_ticks = host->ticks;
	short client_ticks = state->deck_present ? state->deck.ticks : 0;
	long host_vehicle = host_ticks > 0 ? host->object_index : NONE;
	long client_vehicle = client_ticks > 0 ? state->deck.vehicle_index : NONE;
	long now = game_time_get();
	long length = (long)distributed_machine_round_trip_ticks(distributed_player_machine(player_index)) + PREDICTION_JITTER_TICKS;
	boolean host_changed, client_changed, acknowledged;

	if (!accepted->transition_sampled || accepted->transition_unit_index != unit_index)
	{
		accepted->transition_sampled = TRUE;
		accepted->transition_unit_index = unit_index;
		accepted->transition_until = 0;
		accepted->transition_client_spent = FALSE;
		accepted->transition_leash_time = NONE;
		accepted->transition_board_vehicle = NONE;
		accepted->transition_board_until = 0;
	}
	else
	{
		/* (the air countdown is not a new takeoff each tick) */
		host_changed = (host_ticks > 0) != (accepted->transition_host_ticks > 0) ||
			(host_ticks == RIDING_TICKS) != (accepted->transition_host_ticks == RIDING_TICKS) ||
			host_vehicle != accepted->transition_host_vehicle;
		client_changed = (client_ticks > 0) != (accepted->transition_client_ticks > 0) ||
			(client_ticks == RIDING_TICKS) != (accepted->transition_client_ticks == RIDING_TICKS) ||
			client_vehicle != accepted->transition_client_vehicle;
		/* (the word taken is read from the accepted state: the prediction's own stamp is cleared before each take) */
		if (host_changed && vehicle_riding_transition_agreed(accepted->transition_host_ticks, accepted->transition_host_vehicle,
			host_ticks, host_vehicle, client_ticks, client_vehicle, state->deck_present,
			accepted->taken_host_time != NONE && accepted->taken_host_time >= now - length))
		{
			/* (both ends make this change themselves: nothing to leash, and the client's entitlement for a later one stays whole) */
			accepted->transition_until = 0;
			accepted->transition_client_spent = FALSE;
			accepted->transition_board_vehicle = NONE;
			accepted->transition_board_until = 0;
			if (host_ticks == RIDING_TICKS && !state->deck_present)
			{
				/* (the client's word has not boarded yet: it will, and that is no change of its own to leash) */
				accepted->transition_board_vehicle = host_vehicle;
				accepted->transition_board_until = now + length;
			}
		}
		else
		{
			/* (the host's flip right after the leash wrote the copy to the client's place is that write's doing, not a new change) */
			if (host_changed && accepted->transition_host_ticks == RIDING_TICKS && host_ticks > 0 && host_ticks < RIDING_TICKS &&
				host_vehicle == accepted->transition_host_vehicle && accepted->transition_leash_time != NONE &&
				now == accepted->transition_leash_time + 1)
			{
				host_changed = FALSE;
			}
			acknowledged = !host_changed && client_changed && accepted->transition_board_vehicle != NONE &&
				client_ticks == RIDING_TICKS && client_vehicle == accepted->transition_board_vehicle &&
				now <= accepted->transition_board_until &&
				host_ticks == RIDING_TICKS && host_vehicle == accepted->transition_board_vehicle;
			if (host_changed)
				accepted->transition_client_spent = FALSE;
			if (acknowledged)
			{
				/* (the boarding the host agreed to, arriving: used up, with the entitlement as it was) */
				accepted->transition_board_vehicle = NONE;
			}
			else if (!host_changed && client_changed &&
				vehicle_riding_word_on_air_deck(host_ticks, host_vehicle, client_ticks, client_vehicle))
			{
				/* (the word rides the deck the copy is over: the copy's take puts it on its own deck, so nothing to leash and a leash already running is ended, with the client's entitlement for a later change as it was) */
				accepted->transition_until = 0;
			}
			else if (host_changed || (client_changed && !accepted->transition_client_spent))
			{
				accepted->transition_until = now + length;
				accepted->transition_board_vehicle = NONE;
				if (!host_changed)
					accepted->transition_client_spent = TRUE;
			}
		}
	}
	accepted->transition_host_ticks = host_ticks;
	accepted->transition_client_ticks = client_ticks;
	accepted->transition_host_vehicle = host_vehicle;
	accepted->transition_client_vehicle = client_vehicle;
}

/* (the host) a client's player's prediction of its unit on foot, taken
where it says within a tolerance (distributed_apply_predictions). Riding an
elevator (distributed_riding_elevator), only across: a client's elevator
follows the host's (network_coop.c), behind it by the time the host's word
takes to come, and the client's prediction a round trip behind the host's
elevator when it comes. Taken up and down too, it would put the host's unit
under its own elevator's floor going up, to fall through it, and the
client's after it (distributed_correct_own_unit); each machine's elevator
carries its own unit up and down as it moves, and they meet where it
stops. */
static void distributed_take_prediction(
	short player_index,
	long unit_index,
	struct distributed_on_foot_bound const *bound)
{
	struct distributed_unit_state const *state = &distributed_predictions[player_index].state;
	struct object_datum *object = object_get(unit_index);
	long now = game_time_get();
	real_point3d position = state->position;
	real dx, dy, dz;
	struct vehicle_riding_state host;
	struct vehicle_riding_vehicle vehicle;
	real_point3d base;
	real height, width;
	boolean leash;
	boolean riding_applies;
	real_vector3d kept_velocity = object->object.translational_velocity;

	/* MCC movement effects stay host-simulated, rather than widening the
	 * security bounds on client-supplied positions and velocities. */
	if (mcc_script_effects_host_movement())
		return;
	csmemset(&host, 0, sizeof(host));
	riding_applies = biped_riding_get_state(unit_index, &host);
	if (riding_applies)
		distributed_riding_transition(player_index, unit_index, &host);
	leash = riding_applies && now < distributed_accepted[player_index].transition_until &&
		vehicle_riding_host_leash(&position, &object->object.position, vehicle_riding_transition_leash());

	/* (re-resolve after the vehicle pass: a deleted salt takes the world path.
	A copy in the air of the vehicle its anchor is of is held to that anchor
	too, or each switch between riding and that air would restart it) */
	if (biped_riding_get_state(unit_index, &host) &&
		(vehicle_riding_is_riding(&host) ||
		(host.ticks > 0 && distributed_accepted[player_index].valid &&
		distributed_accepted[player_index].unit_index == unit_index &&
		distributed_accepted[player_index].vehicle_index == host.object_index)) &&
		biped_riding_vehicle(host.object_index, NONE, &vehicle) && !distributed_riding_elevator(unit_index))
	{
		distributed_take_riding(player_index, unit_index, bound,
			vehicle_riding_host_path(&host, state->deck_present, &state->deck, FALSE) == _vehicle_riding_host_deck &&
			vehicle_riding_deck_valid(&state->deck, biped_riding_vehicle_node_count(state->deck.vehicle_index)) &&
			biped_riding_vehicle(state->deck.vehicle_index, state->deck.node_index, &vehicle));
		return;
	}
	if (distributed_accepted[player_index].valid && distributed_accepted[player_index].vehicle_index != NONE)
		distributed_accepted[player_index].valid = FALSE;
	if (riding_applies && !leash && state->deck_present && state->deck.ticks == RIDING_TICKS &&
		vehicle_riding_deck_valid(&state->deck, biped_riding_vehicle_node_count(state->deck.vehicle_index)) &&
		biped_riding_vehicle(state->deck.vehicle_index, state->deck.node_index, &vehicle))
	{
		biped_get_physics_pill(unit_index, &base, &height, &width);
		vehicle_riding_host_target(_vehicle_riding_host_world, &vehicle, NULL, &base, &state->deck, &base, &position);
		position.x -= base.x - object->object.position.x;
		position.y -= base.y - object->object.position.y;
		position.z -= base.z - object->object.position.z;
	}

	if (distributed_riding_elevator(unit_index))
		position.z = object->object.position.z;
	dx = position.x - object->object.position.x;
	dy = position.y - object->object.position.y;
	dz = position.z - object->object.position.z;
	/* (for the tests' reports: a tick on from the last taken, on both, with
	the input the client's tick ran: an input's tick is the client's time
	before it ran, a prediction's after; the prediction's time is the
	client's word, kept from overflowing) */
	if (distributed_divergence.taken_host_time[player_index] == now - 1 &&
		distributed_predictions[player_index].time != LONG_MIN &&
		distributed_divergence.taken_time[player_index] == distributed_predictions[player_index].time - 1 &&
		distributed_divergence.input_time[player_index] == distributed_predictions[player_index].time - 1)
	{
		real_point3d const *client_from = &distributed_divergence.taken_position[player_index];
		real_point3d const *host_from = &distributed_divergence.taken_host_position[player_index];
		real mx = (position.x - client_from->x) - (object->object.position.x - host_from->x);
		real my = (position.y - client_from->y) - (object->object.position.y - host_from->y);
		real mz = (position.z - client_from->z) - (object->object.position.z - host_from->z);
		real distance = (real)sqrt(mx * mx + my * my + mz * mz);

		distributed_divergence.samples++;
		if (distance > DIVERGENCE_APART_DISTANCE)
			distributed_divergence.apart++;
		distributed_divergence.total += distance;
		if (distance > distributed_divergence.largest)
			distributed_divergence.largest = distance;
	}
	distributed_divergence.taken_host_time[player_index] = NONE;
	if (!(dx * dx + dy * dy + dz * dz <= HOST_ACCEPT_TOLERANCE * HOST_ACCEPT_TOLERANCE))
		return;
	if (distributed_accepted[player_index].valid)
	{
		long ticks = MIN(distributed_predictions[player_index].time - distributed_accepted[player_index].time,
			now - distributed_accepted[player_index].host_time + PREDICTION_JITTER_TICKS);

		/* (the host's own unit moved further than a unit moves since the
		last taken, which only the host moves it by: taken from where it
		is, a new anchor, and its height where it starts) */
		if (!distributed_on_foot_move_valid(bound, &distributed_accepted[player_index].taken_host_position,
			&object->object.position, now - distributed_accepted[player_index].taken_host_time))
		{
			distributed_accepted[player_index].valid = FALSE;
			distributed_host_speeds[player_index].ground_height = object->object.position.z;
			distributed_host_speeds[player_index].top_height = object->object.position.z;
			distributed_host_speeds[player_index].ground_time = now;
		}
		else if (ticks <= 0 || !distributed_on_foot_move_valid(bound, &distributed_accepted[player_index].position,
			&position, ticks))
		{
			return;
		}
	}
	/* (no higher above where the host last had it on the ground than a
	jump, or a throw its ticks gave it, takes it, and in the air falling:
	past that the host's copy falls as its ticks have it) */
	if (!(position.z - distributed_host_speeds[player_index].ground_height <=
		distributed_on_foot_ceiling(player_index, bound)))
	{
		return;
	}
	/* (the client told which of its ticks the host has it at) */
	if (distributed_apply_state(unit_index, state, &position, leash ? &kept_velocity : NULL, 0.0f,
		leash ? 0.f : HOST_BLEND_DISTANCE, leash ? 0.f : bound->speed,
		leash ? 0.f : distributed_on_foot_fall_speed(bound, position.z)))
	{
		distributed_predictions[player_index].taken_time = distributed_predictions[player_index].time;
		distributed_predictions[player_index].taken_host_time = now;
		distributed_divergence.taken_time[player_index] = distributed_predictions[player_index].time;
		distributed_divergence.taken_host_time[player_index] = now;
		distributed_divergence.taken_position[player_index] = position;
		distributed_divergence.taken_host_position[player_index] = object->object.position;
		if (leash)
			distributed_accepted[player_index].transition_leash_time = now;
		/* (a new anchor: the first, one a second on, or after a jump) */
		if (!distributed_accepted[player_index].valid ||
			now - distributed_accepted[player_index].host_time >= PREDICTION_ANCHOR_TICKS)
		{
			distributed_accepted[player_index].valid = TRUE;
			distributed_accepted[player_index].unit_index = unit_index;
			distributed_accepted[player_index].time = distributed_predictions[player_index].time;
			distributed_accepted[player_index].host_time = now;
			distributed_accepted[player_index].position = position;
			distributed_accepted[player_index].vehicle_index = NONE;
		}
		distributed_accepted[player_index].taken_host_time = now;
		distributed_accepted[player_index].taken_host_position = object->object.position;
		distributed_accepted[player_index].taken_air_unit_index = unit_index;
		distributed_accepted[player_index].taken_air_vehicle = distributed_air_part_vehicle(state);
	}
}

/* (the host) the clients' players where they say, within a tolerance: a
little off closed by half, more put there. Each accepted moves the host's
unit, and the next is measured from there, so a client could take its
player HOST_ACCEPT_TOLERANCE further each tick: a prediction is also no
further from the anchor (one taken, a newer once a second) than the player
moves in the client's ticks since (distributed_on_foot_bound, and a little
more), those no more than the host's since and a little jitter, no higher
than it climbs or jumps; its velocity is no faster. A teleporter moves the
host's own unit too, further than it moves since the last taken: it is
measured from where the host has it only, a new anchor (as it is after a
new life, or a ride). */
static boolean distributed_prediction_rides(
	short player_index,
	long unit_index)
{
	struct vehicle_riding_state host;

	return (unit_index != NONE && biped_riding_get_state(unit_index, &host) && host.ticks > 0) ||
		distributed_predictions[player_index].state.deck_present;
}

static void distributed_apply_predictions(
	boolean riders)
{
	short player_index;

	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		struct distributed_unit_state const *state = &distributed_predictions[player_index].state;
		struct distributed_on_foot_bound bound;
		long unit_index;
		struct vehicle_riding_state host;
		real_vector3d frame_velocity = { 0.0f, 0.0f, 0.0f };

		if (distributed_rider_pass[player_index] != riders)
			continue;
		unit_index = distributed_living_unit(distributed_player(player_index));
		if (unit_index != NONE && object_get(unit_index)->object.parent_object_index != NONE)
			unit_index = NONE;
		if (!distributed_predictions[player_index].valid && !distributed_accepted[player_index].valid &&
			distributed_host_speeds[player_index].unit_index == NONE)
		{
			continue;
		}
		if (distributed_accepted[player_index].valid && distributed_accepted[player_index].unit_index != unit_index)
			distributed_accepted[player_index].valid = FALSE;
		/* (how fast it may go, a tick, what the host's tick did to its own
		copy noted before the client's word is taken; riding or dead, noted
		afresh on foot) */
		if (unit_index != NONE)
		{
			if (biped_riding_get_state(unit_index, &host) && host.ticks > 0)
				frame_velocity = host.velocity;
			distributed_on_foot_bound(player_index, unit_index, &frame_velocity, &bound);
		}
		else
			distributed_host_speeds[player_index].unit_index = NONE;
		if (distributed_predictions[player_index].valid)
		{
			/* (of the unit it has now: not one of a life before; and not
			from a co-op client still loading the host's BSP, whose player
			falls there with no floor under it) */
			boolean eligible = unit_index != NONE && unit_index == state->unit_index &&
				unit_get(unit_index)->unit.player_index != NONE &&
				network_coop_player_has_structure_bsp(unit_get(unit_index)->unit.player_index);

			distributed_predictions[player_index].valid = FALSE;
			distributed_predictions[player_index].taken_host_time = NONE;
			if (eligible)
				distributed_take_prediction(player_index, unit_index, &bound);
		}
		/* (what the host's next tick starts from) */
		if (unit_index != NONE)
		{
			struct object_datum *object = object_get(unit_index);

			distributed_host_speeds[player_index].velocity = object->object.translational_velocity;
			distributed_host_speeds[player_index].frame_velocity = frame_velocity;
			if (object->object.position.z > distributed_host_speeds[player_index].top_height)
				distributed_host_speeds[player_index].top_height = object->object.position.z;
		}
	}
}

/* (a client) the host's word on a player's unit */
static void distributed_handle_unit_state(
	long host_time,
	struct distributed_unit_state const *state)
{
	struct player_datum *player = distributed_player(state->player_index);
	long player_index;
	long unit_index;
	boolean alive = TEST_FLAG(state->flags, _distributed_unit_alive_bit);
	boolean local;

	if (!player)
		return;
	player_index = DATUM_INDEX_NEW(state->player_index, player->identifier);
	local = player->local_player_index != NONE;
	/* how it died, for when this machine's copy dies
	(network_distributed_player_killed) */
	if (state->player_index < MAXIMUM_TRACKED_PLAYERS)
	{
		struct distributed_death *death = &distributed_deaths[state->player_index];

		death->valid = !alive;
		death->killing_player_index = state->killing_player_index != NO_PLAYER ?
			state->killing_player_index : NONE;
		death->friendly_fire = TEST_FLAG(state->flags, _distributed_unit_friendly_fire_bit);
		death->killed_by_vehicle = TEST_FLAG(state->flags, _distributed_unit_killed_by_vehicle_bit);
	}
	unit_index = distributed_living_unit(player);
	if (!alive)
	{
		/* died on the host, who counts it. Its killing blow (network_damage.c)
		kills it here with the death the host's had, so it is given a while
		to come before the unit dies without one, as it falls. */
		if (unit_index != NONE && state->player_index < MAXIMUM_TRACKED_PLAYERS)
		{
			long *dead_since = &distributed_deaths[state->player_index].dead_since;

			if (*dead_since == NONE)
				*dead_since = game_time_get();
			else if (game_time_get() - *dead_since >= DEATH_BLOW_WAIT_TICKS)
				unit_kill_no_statistics(unit_index);
		}
		return;
	}
	if (state->player_index < MAXIMUM_TRACKED_PLAYERS)
		distributed_deaths[state->player_index].dead_since = NONE;
	network_objects_expect_live_unit(state->unit_index);
	/* spawned on the host: the host's unit is the player's here too, once
	this machine has it (network_objects.c) */
	if (state->unit_index == NONE || !network_objects_client_has(state->unit_index) ||
		!object_try_and_get_and_verify_type(state->unit_index, _object_mask_unit) ||
		TEST_FLAG(object_get(state->unit_index)->object.damage_flags, _object_dead_bit))
	{
		return;
	}
	if (player->unit_index != state->unit_index)
	{
		if (player->unit_index != NONE)
			network_player_detach_unit(player_index);
		network_player_attach_unit(player_index, state->unit_index);
		/* (a unit of a life this machine missed the end of, no player's
		now: unit_kill_no_statistics is for players' units only) */
		if (unit_index != NONE && unit_index != state->unit_index && !mcc_script_effects_preserve_body())
			unit_kill(unit_index);
	}
	unit_index = state->unit_index;
	distributed_scripted_pose_apply(unit_get(unit_index), state);
	/* the seat it rides: a client's own player's, once it has ridden
	otherwise for longer than its prediction takes to reach the host and
	come back (a player within the tracked ones, as players.c's datums are) */
	if (state->player_index < MAXIMUM_TRACKED_PLAYERS)
	{
		struct unit_datum *unit = unit_get(unit_index);
		long vehicle_index = unit->object.parent_object_index != NONE && unit->unit.parent_seat_index != NONE ?
			unit->object.parent_object_index : NONE;
		boolean same = vehicle_index == state->vehicle_index &&
			(vehicle_index == NONE || unit->unit.parent_seat_index == state->seat_index);
		short *disagreement = &distributed_seat_disagreements[state->player_index];

		if (same)
			*disagreement = 0;
		else if (!local || ++*disagreement > MAX(SEAT_DISAGREEMENT_TICKS,
			(short)ceil(distributed_own_round_trip) + SEAT_DISAGREEMENT_SLACK_TICKS))
		{
			network_objects_set_seat(unit_index, state->vehicle_index, state->seat_index);
			*disagreement = 0;
		}
	}
	{
		struct damage_network_state damage;

		damage.shield_depleted = TEST_FLAG(state->flags, _distributed_unit_shield_depleted_bit);
		damage.shield_charging = TEST_FLAG(state->flags, _distributed_unit_shield_charging_bit);
		damage.shield_over_charging = TEST_FLAG(state->flags, _distributed_unit_shield_over_charging_bit);
		damage.body_vitality = distributed_vitality_unpack(state->body_vitality);
		damage.shield_vitality = distributed_vitality_unpack(state->shield_vitality);
		damage.current_body_damage = distributed_vitality_unpack(state->current_body_damage);
		damage.recent_body_damage = distributed_vitality_unpack(state->recent_body_damage);
		damage.current_shield_damage = distributed_vitality_unpack(state->current_shield_damage);
		damage.recent_shield_damage = distributed_vitality_unpack(state->recent_shield_damage);
		damage_set_network_state(unit_index, &damage);
	}
	/* the host's powerups (the host decides pickups) */
	{
		struct unit_datum *unit = unit_get(unit_index);

		csmemcpy(player->powerup_durations, state->powerup_durations, sizeof(player->powerup_durations));
		SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit,
			TEST_FLAG(state->unit_flags, _distributed_unit_camouflaged_bit));
		SET_FLAG(unit->unit.flags, _unit_super_camouflaged_bit,
			TEST_FLAG(state->unit_flags, _distributed_unit_super_camouflaged_bit));
		unit->unit.active_camouflage = (real)state->active_camouflage / 255.0f;
	}
	if (TEST_FLAG(state->flags, _distributed_unit_placed_bit) &&
		object_get(unit_index)->object.parent_object_index == NONE)
	{
		real_point3d before = object_get(unit_index)->object.position;

		if (local)
			distributed_correct_own_unit(player, unit_index, state);
		else
		{
			distributed_apply_remote_unit(unit_index, state);
		}
		/* moved further than a step at once (the host's teleporter sent
		it): on the teleporter it lands on, as a teleporter leaves the
		player it sends, so that this machine's does not send it back each
		time the host's word puts it there until it steps off
		(game_engine_update_teleporter) */
		if (distance_squared3d(&before, &object_get(unit_index)->object.position) > 1.0f)
		{
			player->teleporter_index = find_netgame_flag(&object_get(unit_index)->object.position, 1.0f, 0.0f,
				_netgame_flag_teleporter_source, NONE);
		}
	}
}

/* (a client) the host's word on every player's unit */
static void distributed_handle_unit_states(
	long host_time,
	byte const *data,
	byte const *end,
	short count)
{
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_unit_state state;
		word size = distributed_unit_state_read(data, end, &state);

		if (!size)
			break;
		data += size;
		distributed_handle_unit_state(host_time, &state);
	}
}

/* ---------- input */

/* (a client) its own players' input of the tick just run, with the buttons
of the ticks before it, to the host */
static void distributed_client_send_inputs(
	void)
{
	struct
	{
		struct distributed_message_header header;
		struct distributed_player_input inputs[MAXIMUM_LOCAL_PLAYERS];
	} message;
	short count = 0;
	short local_player_index;

	for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		long player_index = local_player_get_player_index(local_player_index);
		struct distributed_player_input *input = &message.inputs[count];

		if (player_index == NONE)
			continue;
		csmemset(input, 0, sizeof(*input));
		if (!update_client_distributed_input(local_player_index, &input->tick, &input->action, input->control_flags))
			continue;
		input->player_index = distributed_player_to_byte(player_index);
		input->structure_bsp_index = (byte)global_structure_bsp_index_get();
		input->coop_no_friendly_fire = network_coop_active() && !network_coop_own_friendly_fire();
		input->host_time = distributed_host_time;
		count++;
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_player_inputs, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_player_input)), _distributed_to_host);
	}
}

/* (the host, in co-op) where each client player's input last faced, by
absolute index: how far it turns between inputs is its looking around */
static struct
{
	boolean valid;
	real_euler_angles2d facing;
} distributed_input_facings[MAXIMUM_TRACKED_PLAYERS];

/* (the host, in co-op) a client player's input counted in the scripts'
action tests, as a local player's is (player_control_action_test_note) */
static void distributed_note_input_actions(
	short player_index,
	struct player_action const *action)
{
	real_euler_angles2d turn = { 0.0f, 0.0f };

	if (distributed_input_facings[player_index].valid)
	{
		turn.yaw = action->desired_facing.yaw - distributed_input_facings[player_index].facing.yaw;
		turn.pitch = action->desired_facing.pitch - distributed_input_facings[player_index].facing.pitch;
		/* (the short way round) */
		if (turn.yaw > _pi)
			turn.yaw -= 2.0f * _pi;
		else if (turn.yaw < -_pi)
			turn.yaw += 2.0f * _pi;
	}
	distributed_input_facings[player_index].valid = TRUE;
	distributed_input_facings[player_index].facing = action->desired_facing;
	player_control_action_test_note(action->control_flags, &turn, &action->throttle, action->primary_trigger);
}

/* (the host) a client's players' input, and from it (once a message) how
long a message takes that client and back */
static void distributed_handle_inputs(
	long machine_index,
	struct distributed_player_input const *inputs,
	short count)
{
	long host_time = NONE;
	short index;

	for (index = 0; index < count; index++)
	{
		struct distributed_player_input const *input = &inputs[index];
		struct player_datum *player;
		struct player_action action;

		if (input->player_index >= MAXIMUM_TRACKED_PLAYERS ||
			!distributed_machine_has_player(machine_index, input->player_index))
		{
			continue;
		}
		player = distributed_player(input->player_index);
		if (!player)
			continue;
		/* (the sticks and the trigger no further than a controller's) */
		action = input->action;
		/* Only the defined input bits are accepted; forced-off wins. */
		action.control_flags &= 0x1FFFFUL;
		action.throttle.i = PIN(action.throttle.i, -1.0f, 1.0f);
		action.throttle.j = PIN(action.throttle.j, -1.0f, 1.0f);
		action.primary_trigger = PIN(action.primary_trigger, 0.0f, 1.0f);
		/* (a spectator's buttons pick whom it watches: not counted) */
		if (network_coop_active() && player->unit_index != NONE)
			distributed_note_input_actions(input->player_index, &action);
		network_coop_note_player_structure_bsp(input->player_index, input->structure_bsp_index);
		network_coop_note_player_friendly_fire(DATUM_INDEX_NEW(input->player_index, player->identifier),
			!input->coop_no_friendly_fire);
		update_server_handle_distributed_input(DATUM_INDEX_NEW(input->player_index, player->identifier), input->tick,
			&action, input->control_flags, DISTRIBUTED_INPUT_HISTORY);
		/* (for the tests' reports: the newest, as the input taken is, or one
		a second or more older, the client's clock set back; the subtraction
		kept from overflowing, as the tick is the client's word) */
		{
			long newest = distributed_divergence.input_time[input->player_index];

			if (newest == NONE || input->tick > newest ||
				(newest >= LONG_MIN + TICKS_PER_SECOND && input->tick < newest - TICKS_PER_SECOND))
			{
				distributed_divergence.input_time[input->player_index] = input->tick;
			}
		}
		if (input->host_time != NONE && (host_time == NONE || input->host_time > host_time))
			host_time = input->host_time;
	}
	if (host_time != NONE && machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES)
	{
		long sample = game_time_get() - host_time;

		/* (a longer one as the longest taken) */
		if (sample >= 0)
		{
			sample = MIN(sample, MAXIMUM_ROUND_TRIP_TICKS);
			/* (as TCP smooths its own: an eighth, a quarter) */
			if (!distributed_round_trips[machine_index].valid)
			{
				distributed_round_trips[machine_index].valid = TRUE;
				distributed_round_trips[machine_index].average = (real)sample;
				distributed_round_trips[machine_index].deviation = (real)sample / 2.0f;
			}
			else
			{
				real difference = (real)sample - distributed_round_trips[machine_index].average;

				distributed_round_trips[machine_index].average += difference / 8.0f;
				distributed_round_trips[machine_index].deviation +=
					((real)fabs(difference) - distributed_round_trips[machine_index].deviation) / 4.0f;
			}
		}
		/* (the same timed in milliseconds, for the latency meter: by when
		this machine sent that tick) */
		{
			long milliseconds = distributed_milliseconds_since_tick(host_time);

			if (milliseconds != NONE)
			{
				distributed_smooth_ping(&distributed_round_trips[machine_index].milliseconds_valid,
					&distributed_round_trips[machine_index].milliseconds, milliseconds);
			}
		}
	}
}

/* the cluster the object (or what it rides) is in, NONE for none */
static short distributed_object_cluster(
	long object_index)
{
	struct object_datum *object = object_get(object_index);

	while (object->object.parent_object_index != NONE)
		object = object_get(object->object.parent_object_index);
	return object->object.location.cluster_index;
}

/* the player's input as the host's tick ran it, in fewer bytes; whether
their buttons or choices changed in the ticks it carries */
static boolean distributed_relayed_action_from(
	short player_index,
	struct player_action const **recent,
	short const *recent_counts,
	struct distributed_relayed_action *relayed)
{
	struct player_action const *action = &recent[0][player_index];
	real throttle_i = action->throttle.i > 1.0f ? 1.0f : action->throttle.i < -1.0f ? -1.0f : action->throttle.i;
	real throttle_j = action->throttle.j > 1.0f ? 1.0f : action->throttle.j < -1.0f ? -1.0f : action->throttle.j;
	real trigger = action->primary_trigger > 1.0f ? 1.0f : action->primary_trigger < 0.0f ? 0.0f :
		action->primary_trigger;
	boolean changed = FALSE;
	short history;

	csmemset(relayed, 0, sizeof(*relayed));
	relayed->player_index = (byte)player_index;
	relayed->desired_weapon_index = (signed char)action->desired_weapon_index;
	relayed->desired_grenade_index = (signed char)action->desired_grenade_index;
	relayed->desired_zoom_level = (signed char)MIN(action->desired_zoom_level, 127);
	for (history = 0; history < DISTRIBUTED_INPUT_HISTORY; history++)
	{
		struct player_action const *past = recent[history] && player_index < recent_counts[history] ?
			&recent[history][player_index] : NULL;

		relayed->control_flags[history] = past ? (unsigned short)past->control_flags : 0;
		if (history > 0 && (!past || relayed->control_flags[history] != relayed->control_flags[history - 1] ||
			past->desired_weapon_index != action->desired_weapon_index ||
			past->desired_grenade_index != action->desired_grenade_index ||
			past->desired_zoom_level != action->desired_zoom_level))
		{
			changed = TRUE;
		}
	}
	relayed->yaw = distributed_angle_pack(action->desired_facing.yaw);
	relayed->pitch = distributed_angle_pack(action->desired_facing.pitch);
	relayed->throttle_i = (signed char)(long)floor(throttle_i * 127.0f + 0.5f);
	relayed->throttle_j = (signed char)(long)floor(throttle_j * 127.0f + 0.5f);
	relayed->primary_trigger = (byte)(long)floor(trigger * 255.0f + 0.5f);
	return changed;
}

/* the action's bytes on the wire (at most
DISTRIBUTED_RELAYED_ACTION_MAXIMUM_SIZE): its player and parts, the
choices, facing, sticks and trigger, the latest tick's buttons, and those of
the ticks before only where they differ from the tick after's
(distributed_relayed_action_read) */
static word distributed_relayed_action_write(
	struct distributed_relayed_action const *relayed,
	byte *buffer)
{
	byte parts = 0;
	byte *cursor = buffer + 2;
	short history;

	cursor = distributed_put(cursor, &relayed->desired_weapon_index, sizeof(relayed->desired_weapon_index));
	cursor = distributed_put(cursor, &relayed->desired_grenade_index, sizeof(relayed->desired_grenade_index));
	cursor = distributed_put(cursor, &relayed->desired_zoom_level, sizeof(relayed->desired_zoom_level));
	cursor = distributed_put(cursor, &relayed->yaw, sizeof(relayed->yaw));
	cursor = distributed_put(cursor, &relayed->pitch, sizeof(relayed->pitch));
	cursor = distributed_put(cursor, &relayed->throttle_i, sizeof(relayed->throttle_i));
	cursor = distributed_put(cursor, &relayed->throttle_j, sizeof(relayed->throttle_j));
	cursor = distributed_put(cursor, &relayed->primary_trigger, sizeof(relayed->primary_trigger));
	cursor = distributed_put(cursor, &relayed->control_flags[0], sizeof(relayed->control_flags[0]));
	for (history = 1; history < DISTRIBUTED_INPUT_HISTORY; history++)
	{
		if (relayed->control_flags[history] != relayed->control_flags[history - 1])
		{
			SET_FLAG(parts, history - 1, TRUE);
			cursor = distributed_put(cursor, &relayed->control_flags[history], sizeof(relayed->control_flags[history]));
		}
	}
	buffer[0] = relayed->player_index;
	buffer[1] = parts;
	return (word)(cursor - buffer);
}

/* an action from its bytes on the wire, no further than end: the bytes it
took, 0 for none that make one */
static word distributed_relayed_action_read(
	byte const *buffer,
	byte const *end,
	struct distributed_relayed_action *relayed)
{
	byte const *cursor = buffer;
	byte parts;
	short history;

	csmemset(relayed, 0, sizeof(*relayed));
	if (!distributed_take(&cursor, end, &relayed->player_index, sizeof(relayed->player_index)) ||
		!distributed_take(&cursor, end, &parts, sizeof(parts)) ||
		!distributed_take(&cursor, end, &relayed->desired_weapon_index, sizeof(relayed->desired_weapon_index)) ||
		!distributed_take(&cursor, end, &relayed->desired_grenade_index, sizeof(relayed->desired_grenade_index)) ||
		!distributed_take(&cursor, end, &relayed->desired_zoom_level, sizeof(relayed->desired_zoom_level)) ||
		!distributed_take(&cursor, end, &relayed->yaw, sizeof(relayed->yaw)) ||
		!distributed_take(&cursor, end, &relayed->pitch, sizeof(relayed->pitch)) ||
		!distributed_take(&cursor, end, &relayed->throttle_i, sizeof(relayed->throttle_i)) ||
		!distributed_take(&cursor, end, &relayed->throttle_j, sizeof(relayed->throttle_j)) ||
		!distributed_take(&cursor, end, &relayed->primary_trigger, sizeof(relayed->primary_trigger)) ||
		!distributed_take(&cursor, end, &relayed->control_flags[0], sizeof(relayed->control_flags[0])))
	{
		return 0;
	}
	for (history = 1; history < DISTRIBUTED_INPUT_HISTORY; history++)
	{
		if (!TEST_FLAG(parts, history - 1))
			relayed->control_flags[history] = relayed->control_flags[history - 1];
		else if (!distributed_take(&cursor, end, &relayed->control_flags[history],
			sizeof(relayed->control_flags[history])))
		{
			return 0;
		}
	}
	return (word)(cursor - buffer);
}

/* (the host) what each client is sent this tick of every player: their unit,
and their input as its last tick ran it (with the buttons of the ticks
before it). Those near the client's own players every tick, those further,
or out of their sight, less often (NEAR_PLAYER_DISTANCE), and those dead or
not yet spawned as those out of sight; whose life, seat or shields' state
changed, or who came into sight, at once; whose buttons changed lately,
their input every tick; one who has left the game, only a change. A
client's own players' units every tick, their input never (it has its own),
and no dead player's input (it drives nothing). A client whose players are
dead watches from where they last were alive. Decided before the damage is
sent (network_damage.c), which goes where the players it hurt do; sent
after it (distributed_host_send_players). */
static void distributed_host_plan_players(
	void)
{
	static boolean present[MAXIMUM_TRACKED_PLAYERS];
	static boolean changed[MAXIMUM_TRACKED_PLAYERS];
	static boolean has_action[MAXIMUM_TRACKED_PLAYERS];
	static boolean action_changed[MAXIMUM_TRACKED_PLAYERS];
	static boolean quit[MAXIMUM_TRACKED_PLAYERS];
	static real_point3d origins[MAXIMUM_TRACKED_PLAYERS];
	static short clusters[MAXIMUM_TRACKED_PLAYERS];
	static boolean placed[MAXIMUM_TRACKED_PLAYERS];
	long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short machine_count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	struct structure_bsp *structure_bsp = global_structure_bsp_get();
	short cluster_count = structure_bsp ? (short)structure_bsp->clusters.count : 0;
	long now = game_time_get();
	struct player_action const *recent[DISTRIBUTED_INPUT_HISTORY];
	short recent_counts[DISTRIBUTED_INPUT_HISTORY];
	short player_index;
	short machine_number;
	short history;

	distributed_host_update_number = update_server_ticked_update_number();
	for (history = 0; history < DISTRIBUTED_INPUT_HISTORY; history++)
	{
		recent[history] = distributed_host_update_number != NONE ?
			update_server_update_actions(distributed_host_update_number - history, &recent_counts[history]) : NULL;
	}
	csmemset(distributed_sends, 0, sizeof(distributed_sends));
	/* (the client each player is on) */
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		distributed_player_machines[player_index] = NONE;
	for (machine_number = 0; machine_number < machine_count; machine_number++)
	{
		long *player_list = machine_get_player_list(machine_indices[machine_number]);
		short local_player_index;

		for (local_player_index = 0; local_player_index < MAXIMUM_LOCAL_PLAYERS; local_player_index++)
		{
			short index = player_list[local_player_index] != NONE ?
				(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(player_list[local_player_index]) : NONE;

			if (index >= 0 && index < MAXIMUM_TRACKED_PLAYERS)
				distributed_player_machines[index] = machine_indices[machine_number];
		}
	}
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		struct player_datum *player = distributed_player(player_index);
		struct distributed_unit_state *state = &distributed_host_states[player_index];
		long unit_index;

		present[player_index] = player != NULL;
		placed[player_index] = FALSE;
		has_action[player_index] = FALSE;
		if (!player)
			continue;
		quit[player_index] = player->quit_out_of_game;
		distributed_state_from_player(player_index, state);
		unit_index = distributed_living_unit(player);
		if (unit_index != NONE)
		{
			object_get_origin(unit_index, &origins[player_index]);
			clusters[player_index] = distributed_object_cluster(unit_index);
			placed[player_index] = TRUE;
		}
		changed[player_index] =
			/* (the predicted bit is its own client's alone) */
			distributed_sent_units[player_index].flags != (byte)(state->flags & ~FLAG(_distributed_unit_predicted_bit)) ||
			distributed_sent_units[player_index].unit_index != state->unit_index ||
			distributed_sent_units[player_index].vehicle_index != state->vehicle_index ||
			distributed_sent_units[player_index].seat_index != state->seat_index;
		distributed_sent_units[player_index].flags = (byte)(state->flags & ~FLAG(_distributed_unit_predicted_bit));
		distributed_sent_units[player_index].unit_index = state->unit_index;
		distributed_sent_units[player_index].vehicle_index = state->vehicle_index;
		distributed_sent_units[player_index].seat_index = state->seat_index;
		if (placed[player_index] && recent[0] && player_index < recent_counts[0])
		{
			has_action[player_index] = TRUE;
			action_changed[player_index] = distributed_relayed_action_from(player_index, recent, recent_counts,
				&distributed_host_actions[player_index]);
		}
	}
	for (machine_number = 0; machine_number < machine_count; machine_number++)
	{
		long machine_index = machine_indices[machine_number];
		long *player_list = machine_get_player_list(machine_index);
		real_point3d viewers[MAXIMUM_LOCAL_PLAYERS];
		unsigned long *viewer_pvs[MAXIMUM_LOCAL_PLAYERS];
		real_vector3d viewer_aims[MAXIMUM_LOCAL_PLAYERS];
		boolean viewer_scoped[MAXIMUM_LOCAL_PLAYERS];
		short viewer_count = 0;
		short index;

		/* its players where they are, or were when last alive (each viewer's
		row of the map's potentially visible set) */
		for (index = 0; index < MAXIMUM_LOCAL_PLAYERS; index++)
		{
			short viewer_index = player_list[index] != NONE ?
				(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(player_list[index]) : NONE;
			struct distributed_viewer *viewer = &distributed_viewers[machine_index][index];
			boolean scoped = FALSE;

			if (viewer_index < 0 || viewer_index >= MAXIMUM_TRACKED_PLAYERS)
				viewer->valid = FALSE;
			else if (placed[viewer_index])
			{
				struct unit_datum *unit = unit_get(distributed_living_unit(distributed_player(viewer_index)));

				viewer->valid = TRUE;
				viewer->cluster_index = clusters[viewer_index];
				viewer->position = origins[viewer_index];
				viewer->aim = unit->unit.aiming_vector;
				/* (NONE unzoomed: a char, which is unsigned on ARM) */
				scoped = (signed char)unit->unit.current_zoom_level >= 0;
			}
			if (!viewer->valid)
				continue;
			viewers[viewer_count] = viewer->position;
			viewer_pvs[viewer_count] = viewer->cluster_index >= 0 && viewer->cluster_index < cluster_count ?
				structure_bsp_get_cluster_pvs(structure_bsp, viewer->cluster_index) : NULL;
			viewer_aims[viewer_count] = viewer->aim;
			viewer_scoped[viewer_count++] = scoped;
		}
		for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		{
			boolean own = distributed_player_machines[player_index] == machine_index;
			byte *sends = &distributed_sends[machine_index][player_index];
			boolean send = FALSE;
			boolean seen = TRUE;
			short period = 1;

			if (!present[player_index] || (quit[player_index] && !own && !changed[player_index]))
				continue;
			if (!own && !placed[player_index])
			{
				/* (dead, or not yet spawned: a spawn is a change, sent at once) */
				period = HIDDEN_PLAYER_PERIOD_TICKS;
				distributed_seen[machine_index][player_index] = TRUE;
			}
			/* (a client that has had no player alive watches anyone) */
			else if (!own && viewer_count)
			{
				real distances_squared[MAXIMUM_LOCAL_PLAYERS];
				boolean sees[MAXIMUM_LOCAL_PLAYERS];
				short to = clusters[player_index];
				real nearest = -1.0f;
				boolean visible = FALSE;

				for (index = 0; index < viewer_count; index++)
				{
					real dx = origins[player_index].x - viewers[index].x;
					real dy = origins[player_index].y - viewers[index].y;
					real dz = origins[player_index].z - viewers[index].z;

					distances_squared[index] = dx * dx + dy * dy + dz * dz;
					if (nearest < 0.0f || distances_squared[index] < nearest)
						nearest = distances_squared[index];
					/* (outside the map's clusters: in sight, to be safe) */
					sees[index] = !viewer_pvs[index] || to < 0 || to >= cluster_count ||
						BIT_VECTOR_TEST_FLAG(viewer_pvs[index], to);
					visible |= sees[index];
				}
				period = !visible ? HIDDEN_PLAYER_PERIOD_TICKS :
					nearest < NEAR_PLAYER_DISTANCE * NEAR_PLAYER_DISTANCE ? 1 :
					nearest < MIDDLE_PLAYER_DISTANCE * MIDDLE_PLAYER_DISTANCE ? 2 :
					nearest < FAR_PLAYER_DISTANCE * FAR_PLAYER_DISTANCE ? 3 : 4;
				/* (came into sight: at once) */
				if (visible && !distributed_seen[machine_index][player_index])
					send = TRUE;
				distributed_seen[machine_index][player_index] = visible;
				seen = visible;
				/* aimed near, when not sent this tick otherwise: the cosine of the
				angle off the aim, compared squared (along / distance >= cosine) */
				if (!send && !changed[player_index] && period > 1 && (now + player_index) % period != 0)
				{
					short aimed_period = HIDDEN_PLAYER_PERIOD_TICKS;

					for (index = 0; index < viewer_count; index++)
					{
						real along;

						if (!sees[index])
							continue;
						along = (origins[player_index].x - viewers[index].x) * viewer_aims[index].i +
							(origins[player_index].y - viewers[index].y) * viewer_aims[index].j +
							(origins[player_index].z - viewers[index].z) * viewer_aims[index].k;
						if (along > 0.0f && viewer_scoped[index] &&
							along * along >= SCOPED_AT_COSINE * SCOPED_AT_COSINE * distances_squared[index])
						{
							aimed_period = 1;
						}
						else if (along > 0.0f &&
							along * along >= AIMED_AT_COSINE * AIMED_AT_COSINE * distances_squared[index])
						{
							aimed_period = MIN(aimed_period, 2);
						}
					}
					period = MIN(period, aimed_period);
				}
			}
			else
			{
				distributed_seen[machine_index][player_index] = TRUE;
			}
			send |= changed[player_index] || (now + player_index) % period == 0;
			if (seen || own)
				SET_FLAG(*sends, _distributed_send_sees_bit, TRUE);
			if (send || own)
				SET_FLAG(*sends, _distributed_send_state_bit, TRUE);
			if (!own && has_action[player_index] && (send || action_changed[player_index]))
				SET_FLAG(*sends, _distributed_send_action_bit, TRUE);
		}
	}
}

/* (the host) to each client what distributed_host_plan_players decided, the
input before the units, as the host's tick had them */
static void distributed_host_send_players(
	void)
{
	long machine_indices[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short machine_count = distributed_client_machines(machine_indices, HALO_PORT_MAXIMUM_NETWORK_MACHINES);
	short machine_number;

	for (machine_number = 0; machine_number < machine_count; machine_number++)
	{
		long machine_index = machine_indices[machine_number];
		short player_index;

		distributed_packer_begin(&distributed_packer, (short)machine_index, _distributed_message_relayed_actions,
			&distributed_host_update_number, sizeof(distributed_host_update_number));
		for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		{
			byte entry[DISTRIBUTED_RELAYED_ACTION_MAXIMUM_SIZE];

			if (TEST_FLAG(distributed_sends[machine_index][player_index], _distributed_send_action_bit))
			{
				distributed_packer_add(&distributed_packer, entry,
					distributed_relayed_action_write(&distributed_host_actions[player_index], entry));
			}
		}
		distributed_packer_flush(&distributed_packer);
		distributed_packer_begin(&distributed_packer, (short)machine_index, _distributed_message_unit_states, NULL, 0);
		for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		{
			byte entry[DISTRIBUTED_UNIT_STATE_MAXIMUM_SIZE];

			if (TEST_FLAG(distributed_sends[machine_index][player_index], _distributed_send_state_bit))
			{
				distributed_packer_add(&distributed_packer, entry,
					distributed_unit_state_write(&distributed_host_states[player_index],
						distributed_player_machines[player_index] == machine_index, entry));
			}
		}
		distributed_packer_flush(&distributed_packer);
	}
}

/* (a client) the host's input for its players, for the others it drives:
the host's update, then each player's */
static void distributed_handle_actions(
	byte const *data,
	byte const *end,
	short count)
{
	long update_number;
	short index;

	if (!distributed_take(&data, end, &update_number, sizeof(update_number)))
		return;
	for (index = 0; index < count; index++)
	{
		struct distributed_relayed_action relayed;
		struct player_action action;
		word size = distributed_relayed_action_read(data, end, &relayed);

		if (!size)
			break;
		data += size;
		csmemset(&action, 0, sizeof(action));
		action.control_flags = relayed.control_flags[0];
		action.desired_facing.yaw = distributed_angle_unpack(relayed.yaw, FALSE);
		action.desired_facing.pitch = distributed_angle_unpack(relayed.pitch, TRUE);
		action.throttle.i = PIN((real)relayed.throttle_i / 127.0f, -1.0f, 1.0f);
		action.throttle.j = PIN((real)relayed.throttle_j / 127.0f, -1.0f, 1.0f);
		action.primary_trigger = (real)relayed.primary_trigger / 255.0f;
		action.desired_weapon_index = relayed.desired_weapon_index;
		action.desired_grenade_index = relayed.desired_grenade_index;
		action.desired_zoom_level = relayed.desired_zoom_level;
		update_client_handle_relayed_action(relayed.player_index, update_number, &action,
			relayed.control_flags, DISTRIBUTED_INPUT_HISTORY);
	}
}

/* ---------- deaths */

/* a player died (game_engine_player_killed, before it announces who killed
whom): the host notes who killed them for its clients; on a client, whose
copy of the death knows nothing of the killer, the host's killer, as this
machine has them */
void network_distributed_player_killed(
	long *killing_player_index,
	long *killing_object_index,
	long dead_player_index,
	boolean *friendly_fire)
{
	short dead_absolute_index = (short)DATUM_INDEX_TO_ABSOLUTE_INDEX(dead_player_index);
	struct distributed_death *death;

	if (dead_absolute_index < 0 || dead_absolute_index >= MAXIMUM_TRACKED_PLAYERS)
		return;
	death = &distributed_deaths[dead_absolute_index];
	if (game_connection() == _game_connection_network_server)
	{
		struct object_datum *killing_object = *killing_object_index != NONE ?
			object_try_and_get(*killing_object_index) : NULL;

		death->valid = TRUE;
		death->killing_player_index = *killing_player_index != NONE ?
			(short)DATUM_INDEX_TO_ABSOLUTE_INDEX(*killing_player_index) : NONE;
		death->friendly_fire = *friendly_fire;
		death->killed_by_vehicle = *killing_player_index == NONE && killing_object &&
			killing_object->object.type == _object_type_vehicle;
		distributed_statistics_due = TRUE;
		server_event_player_killed(*killing_player_index, *killing_object_index, dead_player_index, *friendly_fire);
	}
	else if (game_connection() == _game_connection_network_client && death->valid)
	{
		struct player_datum *killing_player = death->killing_player_index != NONE ?
			distributed_player(death->killing_player_index) : NULL;

		*friendly_fire = death->friendly_fire;
		*killing_player_index = NONE;
		if (killing_player)
		{
			*killing_player_index = DATUM_INDEX_NEW(death->killing_player_index, killing_player->identifier);
			*killing_object_index = killing_player->unit_index;
		}
		/* (an empty vehicle's: the one that did it, the same object here) */
		else if (!death->killed_by_vehicle || *killing_object_index == NONE ||
			!object_try_and_get(*killing_object_index) ||
			object_get(*killing_object_index)->object.type != _object_type_vehicle)
		{
			*killing_object_index = NONE;
		}
	}
}

/* (a client) the host's word on how a player died, with the damage that
killed them (network_damage.c), before this machine's copy dies */
void distributed_set_death(
	short dead_player_index,
	byte killing_player_index,
	boolean friendly_fire,
	boolean killed_by_vehicle)
{
	struct distributed_death *death;

	if (dead_player_index < 0 || dead_player_index >= MAXIMUM_TRACKED_PLAYERS)
		return;
	death = &distributed_deaths[dead_player_index];
	death->valid = TRUE;
	death->killing_player_index = killing_player_index != NO_PLAYER ? killing_player_index : NONE;
	death->friendly_fire = friendly_fire;
	death->killed_by_vehicle = killed_by_vehicle;
}

/* (the host) how a player died, for the damage that killed them */
boolean distributed_get_death(
	short dead_player_index,
	byte *killing_player_index,
	boolean *friendly_fire,
	boolean *killed_by_vehicle)
{
	struct distributed_death const *death;

	if (dead_player_index < 0 || dead_player_index >= MAXIMUM_TRACKED_PLAYERS)
		return FALSE;
	death = &distributed_deaths[dead_player_index];
	*killing_player_index = death->killing_player_index != NONE ? (byte)death->killing_player_index : NO_PLAYER;
	*friendly_fire = death->friendly_fire;
	*killed_by_vehicle = death->killed_by_vehicle;
	return death->valid;
}

/* ---------- pickups */

/* (the host) a player on another machine picked something up (players.c),
for that machine to show */
void network_distributed_player_picked_up(
	long player_index,
	short kind,
	long definition_index,
	short count)
{
	struct distributed_pickup *pickup;

	if (game_connection() != _game_connection_network_server ||
		distributed_pickup_count >= MAXIMUM_PICKUPS_PER_TICK)
	{
		return;
	}
	pickup = &distributed_pickups[distributed_pickup_count++];
	pickup->player_index = distributed_player_to_byte(player_index);
	pickup->kind = (byte)kind;
	pickup->count = count;
	pickup->definition_index = definition_index;
}

/* (the host) each machine what its players picked up (no other machine
shows it) */
static void distributed_send_pickups(
	void)
{
	struct
	{
		struct distributed_message_header header;
		struct distributed_pickup pickups[MAXIMUM_PICKUPS_PER_TICK];
	} message;
	short index;

	for (index = 0; index < distributed_pickup_count; index++)
	{
		long machine_index = distributed_player_machine(distributed_pickups[index].player_index);
		short other;
		short count = 0;

		/* (each machine once, with all of its players') */
		for (other = 0; other < index; other++)
		{
			if (distributed_player_machine(distributed_pickups[other].player_index) == machine_index)
				break;
		}
		if (machine_index == NONE || other < index)
			continue;
		for (other = index; other < distributed_pickup_count; other++)
		{
			if (distributed_player_machine(distributed_pickups[other].player_index) == machine_index)
				message.pickups[count++] = distributed_pickups[other];
		}
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_pickups, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_pickup)));
	}
	distributed_pickup_count = 0;
}

/* ---------- statistics */

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

/* (the host) every player's ping (distributed_player_ping), for the
clients' scoreboards */
static void distributed_send_pings(
	void)
{
	struct distributed_pings_message message;
	short limit = MIN(MAXIMUM_PINGS_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_player_ping));
	short count = 0;
	short player_index;

	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		long ping = distributed_player_ping(player_index);

		if (!distributed_player(player_index))
			continue;
		message.players[count].player_index = (byte)player_index;
		message.players[count].pad = 0;
		message.players[count].milliseconds = ping == NONE ? UNKNOWN_PING : (word)MIN(ping, UNKNOWN_PING - 1);
		count++;
		if (count == limit)
		{
			distributed_send(&message, _distributed_message_pings, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_player_ping)),
				_distributed_to_clients);
			count = 0;
		}
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_pings, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_player_ping)),
			_distributed_to_clients);
	}
}

/* Co-op: the host's structure BSP. Clients switch BSP only when told
(their own trigger volumes don't, players.c). A switch is sent at once and
reliably; it is also resent regularly so a client that joined late catches
up. */
#define STRUCTURE_BSP_INTERVAL_TICKS (TICKS_PER_SECOND / 2)

static short distributed_sent_structure_bsp_index = NONE;

struct distributed_structure_bsp
{
	short structure_bsp_index;
	short pad;
};

struct distributed_structure_bsp_message
{
	struct distributed_message_header header;
	struct distributed_structure_bsp structure_bsp;
};

static void distributed_send_structure_bsp(
	void)
{
	struct distributed_structure_bsp_message message;
	short index = global_structure_bsp_index_get();
	boolean switched = index != distributed_sent_structure_bsp_index;

	message.structure_bsp.structure_bsp_index = index;
	message.structure_bsp.pad = 0;
	distributed_send(&message, _distributed_message_structure_bsp, 1, (word)sizeof(message),
		switched ? _distributed_to_clients_reliably : _distributed_to_clients);
	distributed_sent_structure_bsp_index = index;
}

static void distributed_handle_structure_bsp(
	struct distributed_structure_bsp const *structure_bsp)
{
	short index = structure_bsp->structure_bsp_index;

	if (network_coop_active() && index >= 0 && index < global_scenario_get()->structure_bsp_references.count &&
		index != global_structure_bsp_index_get())
	{
		main_switch_structure_bsp(index);
	}
}

/* the players' statistics that changed since they were last sent, and when
refreshing, STATISTICS_REFRESH_PLAYERS more whatever they are, round them
all (a client that lost a change has it again within eight seconds) */
static void distributed_send_statistics(
	boolean refresh)
{
	struct distributed_statistics_message message;
	short limit = MIN(MAXIMUM_STATISTICS_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_player_statistics));
	short count = 0;
	short refreshed = refresh ? 0 : STATISTICS_REFRESH_PLAYERS;
	/* (where this round starts: the cursor moves past the last player it
	refreshed, which must not shift the players this round visits) */
	short first = distributed_statistics_cursor;
	short step;

	for (step = 0; step < MAXIMUM_TRACKED_PLAYERS; step++)
	{
		short player_index = (short)((first + step) % MAXIMUM_TRACKED_PLAYERS);
		struct player_datum *player = distributed_player(player_index);
		unsigned long checksum;

		if (!player)
			continue;
		checksum = distributed_checksum(&player->statistics, sizeof(player->statistics));
		if (checksum == distributed_sent_statistics[player_index])
		{
			if (refreshed >= STATISTICS_REFRESH_PLAYERS)
				continue;
			refreshed++;
			distributed_statistics_cursor = (short)((player_index + 1) % MAXIMUM_TRACKED_PLAYERS);
		}
		distributed_sent_statistics[player_index] = checksum;
		message.players[count].player_index = player_index;
		message.players[count].pad = 0;
		message.players[count].statistics = player->statistics;
		count++;
		if (count == limit)
		{
			distributed_send(&message, _distributed_message_player_statistics, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_player_statistics)),
				_distributed_to_clients);
			count = 0;
		}
	}
	if (count)
	{
		distributed_send(&message, _distributed_message_player_statistics, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_player_statistics)),
			_distributed_to_clients);
	}
}

/* every player's statistics, to a machine that has loaded (the others', and
what was last sent them, as they are) */
static void distributed_send_all_statistics(
	long machine_index)
{
	struct distributed_statistics_message message;
	short limit = MIN(MAXIMUM_STATISTICS_PER_MESSAGE, RELIABLE_ENTRIES(struct distributed_player_statistics));
	short count = 0;
	short player_index;

	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		struct player_datum *player = distributed_player(player_index);

		if (!player)
			continue;
		message.players[count].player_index = player_index;
		message.players[count].pad = 0;
		message.players[count].statistics = player->statistics;
		count++;
		if (count == limit)
		{
			distributed_send_to_machine(machine_index, &message, _distributed_message_player_statistics, count,
				(word)(sizeof(message.header) + count * sizeof(struct distributed_player_statistics)));
			count = 0;
		}
	}
	if (count)
	{
		distributed_send_to_machine(machine_index, &message, _distributed_message_player_statistics, count,
			(word)(sizeof(message.header) + count * sizeof(struct distributed_player_statistics)));
	}
}

/* ---------- the game type's state */

/* the game type's state (larger than a datagram: reliably) to a machine
that has loaded, or (NONE) to every client when it changed (at most every
other look, unless the game ended) */
static void distributed_send_game_state(
	long machine_index)
{
	struct
	{
		struct distributed_message_header header;
		byte data[MAXIMUM_GAME_STATE_SIZE];
	} message;
	long size = game_engine_write_network_state(message.data, sizeof(message.data));
	unsigned long checksum;

	if (size <= 0)
		return;
	if (machine_index != NONE)
	{
		distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_game_state, 0,
			(word)(sizeof(message.header) + size));
		return;
	}
	checksum = distributed_checksum(message.data, size);
	if (distributed_game_state_time != NONE &&
		(checksum == distributed_game_state_checksum ||
			(game_time_get() - distributed_game_state_time < GAME_STATE_MINIMUM_TICKS &&
				(size < (long)sizeof(long) ||
					csmemcmp(message.data, &distributed_game_state_postgame, sizeof(long)) == 0))))
	{
		return;
	}
	if (size >= (long)sizeof(long))
		csmemcpy(&distributed_game_state_postgame, message.data, sizeof(long));
	distributed_game_state_checksum = checksum;
	distributed_game_state_time = game_time_get();
	distributed_send(&message, _distributed_message_game_state, 0, (word)(sizeof(message.header) + size),
		_distributed_to_clients_reliably);
}

/* ---------- the game */

/* Predictions, delayed effects and send caches belong to one world, even
when a checkpoint restores the same object handles. Connection identity,
permissions, round trips and the host's current roster do not. */
static void distributed_reset_checkpoint_state(boolean reset_received_times)
{
	short sender, type, player_index;

	distributed_last_sent_time = NONE;
	csmemset(distributed_deaths, 0, sizeof(distributed_deaths));
	csmemset(distributed_seat_disagreements, 0, sizeof(distributed_seat_disagreements));
	csmemset(distributed_predictions, 0, sizeof(distributed_predictions));
	csmemset(distributed_accepted, 0, sizeof(distributed_accepted));
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		distributed_accepted[player_index].taken_air_unit_index = NONE;
		distributed_accepted[player_index].taken_air_vehicle = NONE;
	}
	csmemset(distributed_host_speeds, 0, sizeof(distributed_host_speeds));
	csmemset(distributed_input_facings, 0, sizeof(distributed_input_facings));
	csmemset(distributed_seen, 0, sizeof(distributed_seen));
	csmemset(distributed_viewers, 0, sizeof(distributed_viewers));
	csmemset(distributed_sends, 0, sizeof(distributed_sends));
	csmemset(distributed_host_states, 0, sizeof(distributed_host_states));
	csmemset(distributed_host_actions, 0, sizeof(distributed_host_actions));
	csmemset(distributed_sent_statistics, 0, sizeof(distributed_sent_statistics));
	distributed_statistics_cursor = 0;
	distributed_statistics_due = TRUE;
	distributed_game_state_checksum = 0;
	distributed_game_state_time = NONE;
	distributed_host_update_number = NONE;
	distributed_sent_structure_bsp_index = NONE;
	distributed_pickup_count = 0;
	distributed_machines.valid = FALSE;
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
	{
		distributed_deaths[player_index].dead_since = NONE;
		distributed_sent_units[player_index].flags = 0;
		distributed_sent_units[player_index].unit_index = NONE;
		distributed_sent_units[player_index].vehicle_index = NONE;
		distributed_sent_units[player_index].seat_index = NONE;
		distributed_predictions[player_index].taken_host_time = NONE;
		distributed_host_speeds[player_index].unit_index = NONE;
		distributed_player_machines[player_index] = NONE;
		distributed_divergence.taken_time[player_index] = NONE;
		distributed_divergence.taken_host_time[player_index] = NONE;
		distributed_divergence.input_time[player_index] = NONE;
	}
	for (sender = 0; sender < MAXIMUM_SENDERS; sender++)
	{
		distributed_batches[sender].size = 0;
		if (reset_received_times)
		{
			distributed_sender_times[sender] = NONE;
			for (type = 0; type < NUMBER_OF_DISTRIBUTED_MESSAGES; type++)
				distributed_received_times[sender][type] = NONE;
		}
	}
	for (sender = 0; sender < MAXIMUM_LOCAL_PLAYERS; sender++)
	{
		for (type = 0; type < OWN_POSITION_TICKS; type++)
		{
			distributed_own_positions[sender][type].time = NONE;
			distributed_own_positions[sender][type].unit_index = NONE;
		}
	}
	update_queues_distributed_reset();
}

static void distributed_advance_checkpoint(void)
{
	/* Unsigned rollover is defined; zero remains the unbound client state. */
	if (++distributed_host_epoch == 0)
		++distributed_host_epoch;
	distributed_checkpoint_epoch = distributed_host_epoch;
}

static void distributed_restore_checkpoint(void)
{
	distributed_reset_checkpoint_state(TRUE);
	network_objects_discard_client_effects();
	network_objects_reverted();
	network_damage_new_game();
	network_actors_new_game();
	network_coop_reset_recovery();
}

void network_distributed_reverted(void)
{
	long machines[HALO_PORT_MAXIMUM_NETWORK_MACHINES];
	short count, index;

	if (game_connection() != _game_connection_network_server || !network_coop_active())
		return;
	distributed_advance_checkpoint();
	distributed_restore_checkpoint();
	count = distributed_client_machines(machines, NUMBEROF(machines));
	/* A snapshot may also broadcast creates/deletes. Every loaded client
	must receive the boundary before the first snapshot is constructed. */
	for (index = 0; index < count; index++)
		distributed_checkpoint_to_machine(machines[index]);
	network_objects_host_reverted();
}

/* a new map loading (game.c), before any of the new game's messages can
apply: nothing sent or had yet */
void network_distributed_new_game(
	void)
{
	short type;
	short player_index;

	if (game_connection() == _game_connection_network_server)
		distributed_advance_checkpoint();
	else
		distributed_checkpoint_epoch = 0;
	distributed_checkpoint_synchronized = FALSE;

	csmemset(&distributed_divergence, 0, sizeof(distributed_divergence));
	distributed_reset_checkpoint_state(TRUE);
	csmemset(distributed_round_trips, 0, sizeof(distributed_round_trips));
	csmemset(distributed_client_clocks, 0, sizeof(distributed_client_clocks));
	csmemset(distributed_client_identities, 0, sizeof(distributed_client_identities));
	distributed_identity_sent = FALSE;
	csmemset(distributed_client_builds, 0, sizeof(distributed_client_builds));
	csmemset(distributed_client_build_known, 0, sizeof(distributed_client_build_known));
	distributed_build_sent = FALSE;
	{
		short machine_index;

		for (machine_index = 0; machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES; machine_index++)
			distributed_client_clocks[machine_index].window_tick = NONE;
	}
	distributed_game_state_postgame = 0;
	distributed_own_round_trip = 0.0f;
	csmemset(distributed_player_pings, 0xFF, sizeof(distributed_player_pings));
	distributed_own_ping_valid = FALSE;
	distributed_own_ping_milliseconds = 0.0f;
	distributed_host_silent_since = 0;
	distributed_shown_own_ping = NONE;
	distributed_shown_slowest_ping = NONE;
	for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		distributed_shown_pings[player_index] = NONE;
	for (type = 0; type < TICK_SENT_TIMES; type++)
		distributed_tick_sent_times[type].time = NONE;
	csmemset(distributed_machine_players, 0xFF, sizeof(distributed_machine_players));
	distributed_machines.in_tick = FALSE;
	distributed_host_time = NONE;
	distributed_statistics_due = FALSE;
	network_objects_new_game();
	network_damage_new_game();
	network_actors_new_game();
	network_coop_new_game();
	network_aim_assist_reset();
	network_shield_effects_reset();
	network_riding_map_reset();
	network_votekick_new_game();
	network_afk_new_game();
	network_map_vote_reset();
}

/* after each tick (game_time.c) */
void network_distributed_tick(
	void)
{
	short connection = game_connection();

	if (game_time_get() == distributed_last_sent_time)
		return;
	distributed_last_sent_time = game_time_get();
	distributed_machines.in_tick = TRUE;
	if (connection == _game_connection_network_server)
	{
		short player_index;

		/* (a living player's last death is behind them: a death the game
		does not note, one it does not score, has no killer) */
		for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
		{
			if (distributed_deaths[player_index].valid &&
				distributed_living_unit(distributed_player(player_index)) != NONE)
			{
				distributed_deaths[player_index].valid = FALSE;
			}
		}
		/* the clients' players where they say they are, then the objects
		first (created before anything names them), the damage dealt this
		tick before the units it hurt and killed (but where the units go
		decided first: the damage goes where the players it hurt do), and a
		kill's statistics before the kill, so that a client announcing it
		counts it (a double kill, a killing spree) */
		{
			short player_index;

			for (player_index = 0; player_index < MAXIMUM_TRACKED_PLAYERS; player_index++)
			{
				distributed_rider_pass[player_index] = distributed_prediction_rides(player_index,
					distributed_living_unit(distributed_player(player_index)));
			}
		}
		profile_scope(distributed_tick_apply_predictions_section, distributed_apply_predictions(FALSE);)
		profile_scope(distributed_tick_apply_vehicle_predictions_section, network_objects_apply_vehicle_predictions();)
		profile_scope(distributed_tick_apply_predictions_section, distributed_apply_predictions(TRUE);)
		profile_scope(distributed_tick_objects_host_tick_section, network_objects_host_tick();)
		profile_scope(distributed_tick_plan_players_section, distributed_host_plan_players();)
		profile_scope(distributed_tick_damage_host_tick_section, network_damage_host_tick();)
		if (distributed_statistics_due || game_time_get() % STATISTICS_INTERVAL_TICKS == 0)
			profile_scope(distributed_tick_send_statistics_section,
				distributed_send_statistics(game_time_get() % STATISTICS_REFRESH_TICKS == 0);)
		distributed_statistics_due = FALSE;
		if (game_time_get() % PING_INTERVAL_TICKS == 0)
			profile_scope(distributed_tick_send_pings_section, distributed_send_pings();)
		if (game_time_get() % PING_REFRESH_TICKS == 0 && latency_meter_enabled())
			distributed_refresh_pings();
		/* (before the players, so clients load a new BSP before they hear
		where the host moved everyone into it) */
		if (network_coop_active() && (global_structure_bsp_index_get() != distributed_sent_structure_bsp_index ||
			game_time_get() % STRUCTURE_BSP_INTERVAL_TICKS == 0))
		{
			profile_scope(distributed_tick_send_structure_bsp_section, distributed_send_structure_bsp();)
		}
		profile_scope(distributed_tick_send_players_section, distributed_host_send_players();)
		profile_scope(distributed_tick_actors_host_tick_section, network_actors_host_tick();)
		profile_scope(distributed_tick_coop_host_tick_section, network_coop_host_tick();)
		profile_scope(distributed_tick_send_pickups_section, distributed_send_pickups();)
		if (game_time_get() % GAME_STATE_INTERVAL_TICKS == 0)
			profile_scope(distributed_tick_send_game_state_section, distributed_send_game_state(NONE);)
		network_votekick_host_tick();
		network_afk_host_tick();
	}
	else if (connection == _game_connection_network_client)
	{
		profile_scope(distributed_tick_note_own_positions_section, distributed_note_own_positions();)
		if (game_time_get() % PING_REFRESH_TICKS == 0 && latency_meter_enabled())
			distributed_refresh_pings();
		profile_scope(distributed_tick_send_inputs_section, distributed_client_send_inputs();)
		profile_scope(distributed_tick_send_predictions_section, distributed_client_send_predictions();)
		profile_scope(distributed_tick_objects_client_tick_section, network_objects_client_tick();)
		profile_scope(distributed_tick_damage_client_tick_section, network_damage_client_tick();)
		profile_scope(distributed_tick_coop_client_tick_section, network_coop_client_tick();)
	}
	/* (when this tick's messages were sent: the round trips that name it
	are timed by it) */
	distributed_tick_sent_times[game_time_get() & (TICK_SENT_TIMES - 1)].time = game_time_get();
	distributed_tick_sent_times[game_time_get() & (TICK_SENT_TIMES - 1)].milliseconds = system_milliseconds();
	profile_scope(distributed_tick_batches_flush_section, distributed_batches_flush();)
	distributed_machines.in_tick = FALSE;
	distributed_machines.valid = FALSE;
}

/* whether a message's tick is near enough the latest had from its sender
to be taken (distributed_sender_times); from_stream: it came over the
sender's stream (or a client's connection to the host), not in a datagram */
static boolean distributed_message_time_bounded(
	long machine_index,
	long game_time,
	boolean from_stream)
{
	short sender = machine_index == NONE ? HOST_SENDER : (short)machine_index;
	long *latest;
	unsigned long distance;
	short type;

	if (sender < 0 || sender >= MAXIMUM_SENDERS)
		return FALSE;
	latest = &distributed_sender_times[sender];
	if (*latest == NONE)
	{
		*latest = game_time;
		return TRUE;
	}
	/* (unsigned: no tick, however far, overflows it) */
	distance = game_time > *latest ?
		(unsigned long)game_time - (unsigned long)*latest :
		(unsigned long)*latest - (unsigned long)game_time;
	if (distance > DISTRIBUTED_TIME_WINDOW_TICKS)
	{
		static unsigned long last_logged_time;
		static boolean logged;

		if (!from_stream)
		{
			unsigned long now = system_milliseconds();

			if (!logged || now - last_logged_time >= 1000)
			{
				error(_error_silent, "distributed message stamped %ld ticks from its sender's latest in a datagram; dropped",
					game_time > *latest ? (long)distance : -(long)distance);
				last_logged_time = now;
				logged = TRUE;
			}
			return FALSE;
		}
		/* (the sender's own word: what a datagram sent as from it had moved
		its latest to is forgotten) */
		*latest = game_time;
		for (type = 0; type < NUMBER_OF_DISTRIBUTED_MESSAGES; type++)
			distributed_received_times[sender][type] = NONE;
		return TRUE;
	}
	if (game_time > *latest)
		*latest = game_time;
	return TRUE;
}

/* whether an unreliable message of the kind is older than one had already
(then it is dropped: the newer has overtaken it) */
static boolean distributed_message_stale(
	long machine_index,
	struct distributed_message_header const *header)
{
	short sender = machine_index == NONE ? HOST_SENDER : (short)machine_index;
	long *latest;

	switch (header->type)
	{
	case _distributed_message_player_prediction:
	case _distributed_message_unit_states:
	case _distributed_message_player_statistics:
	case _distributed_message_inventories:
	case _distributed_message_object_states:
	case _distributed_message_vehicle_prediction:
	case _distributed_message_player_inputs:
	case _distributed_message_relayed_actions:
	case _distributed_message_damage_events:
	case _distributed_message_pings:
	case _distributed_message_actor_states:
	case _distributed_message_structure_bsp:
	case _distributed_message_coop_presentation:
	case _distributed_message_coop_device_groups:
	case _distributed_message_coop_object_names:
	case _distributed_message_coop_skip_vote:
	case _distributed_message_coop_events:
	case _distributed_message_coop_object_transforms:
	case _distributed_message_actor_damage:
	case _distributed_message_coop_object_looks:
	case _distributed_message_damage_animations:
	case _distributed_message_coop_screen_effect:
	case _distributed_message_coop_device_states:
	case _distributed_message_votekick_status:
	case _distributed_message_mcc_grenades:
	case _distributed_message_mcc_campaign:
	case _distributed_message_mcc_object_names:
		break;
	default:
		return FALSE;
	}
	if (sender < 0 || sender >= MAXIMUM_SENDERS)
		return TRUE;
	latest = &distributed_received_times[sender][header->type];
	/* (a tick's several messages of a kind have its time alike) */
	if (*latest != NONE && header->game_time < *latest)
		return TRUE;
	*latest = header->game_time;
	return FALSE;
}

/* (the host) a text shown in red on every machine's console: its own, and
every client's (_distributed_message_notice) */
void distributed_send_notice(
	char const *text)
{
	struct
	{
		struct distributed_message_header header;
		char text[MAXIMUM_NOTICE_LENGTH];
	} message;
	long length = csstrlen(text);

	if (length > MAXIMUM_NOTICE_LENGTH - 1)
		length = MAXIMUM_NOTICE_LENGTH - 1;
	csmemset(&message, 0, sizeof(message));
	csmemcpy(message.text, text, length);
	/* (the host's own players see it as the clients' do: network_chat.c) */
	network_chat_host_notice(message.text);
	error(_error_log, "%s", message.text);
	distributed_send(&message, _distributed_message_notice, 0, (word)(sizeof(message.header) + length + 1),
		_distributed_to_clients_reliably);
}

/* (the host) ... on one client's console only, or (NONE) the host's own */
void distributed_send_notice_to_machine(
	long machine_index,
	char const *text)
{
	struct
	{
		struct distributed_message_header header;
		char text[MAXIMUM_NOTICE_LENGTH];
	} message;
	long length = csstrlen(text);

	if (length > MAXIMUM_NOTICE_LENGTH - 1)
		length = MAXIMUM_NOTICE_LENGTH - 1;
	csmemset(&message, 0, sizeof(message));
	csmemcpy(message.text, text, length);
	if (machine_index == NONE)
	{
		network_chat_host_notice(message.text);
		return;
	}
	distributed_send_to_machine_reliably(machine_index, &message, _distributed_message_notice, 0,
		(word)(sizeof(message.header) + length + 1));
}

/* (the host) the names of a client machine's players, in ASCII, for a
notice ("?" for what is not ASCII) */
void distributed_machine_player_names(
	long machine_index,
	char *names,
	long size)
{
	struct data_iterator iterator;
	struct player_datum *player;
	long length = 0;

	names[0] = 0;
	data_iterator_new(&iterator, player_data);
	while ((player = (struct player_datum *)data_iterator_next(&iterator)) != NULL)
	{
		short index;

		if (distributed_player_machine((short)DATUM_INDEX_TO_ABSOLUTE_INDEX(iterator.datum_index)) != machine_index)
			continue;
		if (length && length + 2 < size)
		{
			names[length++] = ',';
			names[length++] = ' ';
		}
		for (index = 0; index < (short)NUMBEROF(player->name) && player->name[index] && length + 1 < size; index++)
			names[length++] = player->name[index] >= 32 && player->name[index] < 127 ? (char)player->name[index] : '?';
		names[length] = 0;
	}
	if (!length)
		snprintf(names, size, "machine #%ld", machine_index);
}

/* (a client) its Discord user, as its Discord told it (none without one:
not running, or internet play off), to the host: once it is a machine the
host takes messages of (its ready sent: network_objects_client_tick), and
again when it changes (Discord connecting later), looked at once a second */
void distributed_client_send_identity(
	void)
{
	struct
	{
		struct distributed_message_header header;
		struct distributed_client_identity identity;
	} message;

	if (distributed_identity_sent && game_time_get() % TICKS_PER_SECOND != 0)
		return;
	csmemset(&message, 0, sizeof(message));
	p2p_discord_identity(message.identity.discord_id, sizeof(message.identity.discord_id),
		message.identity.discord_name, sizeof(message.identity.discord_name));
	if (distributed_identity_sent &&
		!csmemcmp(&message.identity, &distributed_sent_identity, sizeof(distributed_sent_identity)))
	{
		return;
	}
	distributed_sent_identity = message.identity;
	distributed_identity_sent = TRUE;
	distributed_send(&message, _distributed_message_client_identity, 1, (word)sizeof(message),
		_distributed_to_host_reliably);
}

/* keeps of text what a build's field may hold: printable ASCII, and line
ends where lines says; ended, and its trailing spaces and line ends left
out (another machine's text is untrusted) */
static void distributed_build_text(
	char *destination,
	size_t size,
	char const *source,
	boolean lines)
{
	size_t length = 0;
	size_t index;

	for (index = 0; source[index] && length < size - 1; index++)
	{
		char character = source[index];

		if ((character >= ' ' && character <= '~') || (lines && character == '\n'))
			destination[length++] = character;
	}
	while (length > 0 && (destination[length - 1] == ' ' || destination[length - 1] == '\n'))
		length--;
	destination[length] = 0;
}

/* (a client) its build, to the host: once a machine the host takes
messages of (its ready sent), once a game */
void distributed_client_send_build(
	void)
{
	struct
	{
		struct distributed_message_header header;
		struct distributed_client_build build;
	} message;
	char flavor[DISTRIBUTED_CLIENT_BUILD_FLAVOR_SIZE];
	char repository[DISTRIBUTED_CLIENT_BUILD_REPOSITORY_SIZE];
	char version[DISTRIBUTED_CLIENT_BUILD_VERSION_SIZE];
	char mods[DISTRIBUTED_CLIENT_BUILD_MODS_SIZE];
	long number;

	if (distributed_build_sent)
		return;
	csmemset(&message, 0, sizeof(message));
	main_build_identity(&number, flavor, sizeof(flavor), repository, sizeof(repository), version, sizeof(version));
	port_mods_list(mods, sizeof(mods));
	snprintf(message.build.number, sizeof(message.build.number), "%ld", number);
	distributed_build_text(message.build.flavor, sizeof(message.build.flavor), flavor, FALSE);
	distributed_build_text(message.build.repository, sizeof(message.build.repository), repository, FALSE);
	distributed_build_text(message.build.version, sizeof(message.build.version), version, FALSE);
	distributed_build_text(message.build.mods, sizeof(message.build.mods), mods, TRUE);
	distributed_build_sent = TRUE;
	distributed_send(&message, _distributed_message_client_build, 1, (word)sizeof(message),
		_distributed_to_host_reliably);
}

boolean distributed_machine_client_build(
	long machine_index,
	struct distributed_client_build *build)
{
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES ||
		!distributed_client_build_known[machine_index])
	{
		return FALSE;
	}
	*build = distributed_client_builds[machine_index];
	return TRUE;
}

/* (the host) a client machine's address (host byte order) as it really is:
an internet play peer's endpoint for its stand-in in 100.64.0.0/10 (p2p.c,
which numbers the stand-in from the peer's identifier: a peer makes a new
one at will), 0 if that is not known */
unsigned long distributed_real_address(
	unsigned long address)
{
	if ((address & 0xFFC00000) == 0x64400000)
	{
		/* (p2p.c's in network byte order: the first number the lowest byte) */
		unsigned long network = (address >> 24) | ((address >> 8) & 0xFF00) | ((address << 8) & 0xFF0000) |
			(address << 24);
		unsigned long real = p2p_peer_endpoint_address(network);

		return (real >> 24) | ((real >> 8) & 0xFF00) | ((real << 8) & 0xFF0000) | (real << 24);
	}
	return address;
}

unsigned long distributed_machine_real_address(
	long machine_index)
{
	return distributed_real_address(network_game_server_machine_address(machine_index));
}

/* (the host) a client machine's address as text: its real one, for an
internet play peer's stand-in (p2p.c; an IPv6 peer's /64). Whole, for the
host's own CHEATERS_FILE and BANS_FILE, which match on it: never for the
log (log_address.h) */
static void distributed_address_text(
	unsigned long address,
	char *text,
	long size)
{
	/* 100.64.0.0/10: an internet play peer's, by its real address */
	if ((address & 0xFFC00000) == 0x64400000)
	{
		unsigned long network = (address >> 24) | ((address >> 8) & 0xFF00) | ((address << 8) & 0xFF0000) |
			(address << 24);

		if (p2p_peer_endpoint_text(network, text, (int)size))
			return;
	}
	if (address)
	{
		snprintf(text, size, "%lu.%lu.%lu.%lu", (address >> 24) & 255, (address >> 16) & 255, (address >> 8) & 255,
			address & 255);
	}
	else
	{
		snprintf(text, size, "unknown");
	}
}

/* (the host) a client machine's address as text (distributed_address_text) */
static void distributed_machine_address_text(
	long machine_index,
	char *text,
	long size)
{
	distributed_address_text(network_game_server_machine_address(machine_index), text, size);
}

/* text of a player's (their names) kept to printable ASCII, no longer
than the size: no line of theirs breaks or runs on */
static void distributed_printable(
	char *destination,
	long size,
	char const *source)
{
	long length = 0;

	for (; source && *source && length < size - 1; source++)
		destination[length++] = *source >= 32 && *source < 127 ? *source : '?';
	destination[length] = 0;
}

/* (the host) a client machine's Discord account: a verified one's (it showed
a pass that checked: network_link.c), else the one it told (only of a
machine in a game, unless any_time), kept to what is allowed; TRUE if it is
verified */
static boolean distributed_machine_discord(
	long machine_index,
	char *discord_id,
	long discord_id_size,
	char *discord_name,
	long discord_name_size,
	boolean any_time)
{
	discord_id[0] = 0;
	discord_name[0] = 0;
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return FALSE;
	if (network_link_machine_verified(machine_index, discord_id, discord_id_size, discord_name, discord_name_size))
		return TRUE;
	if (any_time || game_in_progress())
	{
		p2p_discord_sanitize(discord_id, (int)discord_id_size, distributed_client_identities[machine_index].discord_id,
			0);
		p2p_discord_sanitize(discord_name, (int)discord_name_size,
			distributed_client_identities[machine_index].discord_name, 1);
	}
	return FALSE;
}

boolean network_distributed_machine_discord(
	long machine_index,
	char *discord_id,
	long discord_id_size,
	char *discord_name,
	long discord_name_size)
{
	return distributed_machine_discord(machine_index, discord_id, discord_id_size, discord_name, discord_name_size,
		TRUE);
}

/* (the host) a line in a list of players (CHEATERS_FILE, BANS_FILE): when,
their address, Discord user and names, and why; separated by tabs, each
part kept to the characters allowed and their lengths (what a player could
tell: their Discord user and names). The Discord user of the machine at
the index, if it is one in the game (NONE: none) */
static boolean distributed_write_record(
	char const *file_name,
	char const *address,
	char const *hardware_id,
	char const *discord_id,
	char const *discord_name,
	boolean discord_verified,
	char const *names,
	char const *reason,
	unsigned long until);

static void distributed_write_player_record(
	char const *file_name,
	char const *address,
	long machine_index,
	char const *names,
	char const *reason,
	unsigned long until)
{
	char discord_id[DISCORD_ID_SIZE] = "";
	char discord_name[DISCORD_NAME_SIZE] = "";
	char hardware_id[40] = "";
	boolean verified = distributed_machine_discord(machine_index, discord_id, sizeof(discord_id), discord_name,
		sizeof(discord_name), FALSE);

	if (machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		p2p_hardware_id_sanitize(hardware_id, 33, network_game_server_machine_hardware_id(machine_index));
	distributed_write_record(file_name, address, hardware_id, discord_id, discord_name, verified, names, reason, until);
}

/* (the host) the line itself: its parts as distributed_write_player_record
says (a hardware id, a Discord id or name empty for none; the Discord
account a verified one, or as the machine told it); TRUE if it was written */
static boolean distributed_write_record(
	char const *file_name,
	char const *address,
	char const *hardware_id,
	char const *discord_id,
	char const *discord_name,
	boolean discord_verified,
	char const *names,
	char const *reason,
	unsigned long until)
{
	char kept_names[96];
	char kept_reason[96];
	char kept_address[32];
	char when[32] = "";
	time_t now = time(NULL);
	struct tm *local = localtime(&now);
	FILE *file;
	boolean written;

	distributed_printable(kept_names, sizeof(kept_names), names);
	distributed_printable(kept_reason, sizeof(kept_reason), reason);
	distributed_printable(kept_address, sizeof(kept_address), address);
	if (local)
		strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", local);
	file = fopen(file_name, "a");
	if (!file)
	{
		error(_error_log, "could not open %s to add a player to it", file_name);
		return FALSE;
	}
	/* (the Discord user as the machine told it, which it may say is anyone's,
	marked so; a verified one's, a pass that checked, not: network_distributed_banned
	matches it) */
	fprintf(file, "%s\tip=%s\thwid=%s\tdiscord_username=%s%s\tdiscord_id=%s%s\tplayers=%s\treason=%s", when,
		kept_address, hardware_id[0] ? hardware_id : "none",
		discord_name[0] ? discord_name : "none", discord_name[0] && !discord_verified ? " (self-reported)" : "",
		discord_id[0] ? discord_id : "none", discord_id[0] && !discord_verified ? " (self-reported)" : "",
		kept_names, kept_reason);
	/* (a ban for a while: its end, in seconds since 1970; a line without one
	is for ever) */
	if (until)
		fprintf(file, "\tuntil=%lu", until);
	fputc('\n', file);
	written = !ferror(file);
	written = fclose(file) == 0 && written;
	return written;
}

/* (the host) a player dropped for cheating: in CHEATERS_FILE, and banned
(BANS_FILE) */
static void distributed_log_cheater(
	long machine_index,
	char const *names,
	char const *reason)
{
	char address[32];

	distributed_machine_address_text(machine_index, address, sizeof(address));
	distributed_write_player_record(CHEATERS_FILE, address, machine_index, names, reason, 0);
	distributed_write_player_record(BANS_FILE, address, machine_index, names, reason, 0);
}

/* (the host: a dedicated server's ban command, server_commands.c) the next
ban's reason and end (seconds since 1970; 0: for ever), for its line */
static char distributed_next_ban_reason[96];
static unsigned long distributed_next_ban_until;
/* (sv_ban_address's: the next ban's line written already) */
static boolean distributed_next_ban_unrecorded;

void network_distributed_ban_details(
	char const *reason,
	unsigned long until)
{
	snprintf(distributed_next_ban_reason, sizeof(distributed_next_ban_reason), "%s", reason ? reason : "");
	distributed_next_ban_until = until;
}

void network_distributed_ban_unrecorded(
	boolean unrecorded)
{
	distributed_next_ban_unrecorded = unrecorded;
}

boolean network_distributed_ban_record(
	char const *address,
	char const *hardware_id,
	char const *discord_id,
	char const *names,
	char const *reason,
	unsigned long until)
{
	char kept_hardware_id[40];
	char kept_discord_id[DISCORD_ID_SIZE];

	p2p_hardware_id_sanitize(kept_hardware_id, 33, hardware_id);
	p2p_discord_sanitize(kept_discord_id, sizeof(kept_discord_id), discord_id ? discord_id : "", 0);
	return distributed_write_record(BANS_FILE, address, kept_hardware_id, kept_discord_id, "", kept_discord_id[0] != 0,
		names, reason, until);
}

/* (the host: network_server_manager.c) whether a machine of this address
(host byte order; an internet play peer's by its real one), hardware id or
verified Discord account (its id; empty: none) is banned: its address one
of BANS_FILE's (each line's "ip=", which a host may add or take out by
hand), its hardware id, or its account a line's verified one (a
"discord_id=" without "(self-reported)" after it) */
boolean network_distributed_banned(
	unsigned long address,
	char const *hardware_id,
	char const *discord_id)
{
	char text[32];
	char line[512];
	char kept_hardware_id[40];
	char kept_discord_id[DISCORD_ID_SIZE];
	FILE *file;
	boolean banned = FALSE;
	size_t length, hardware_id_length, discord_id_length;

	p2p_hardware_id_sanitize(kept_hardware_id, 33, hardware_id);
	p2p_discord_sanitize(kept_discord_id, sizeof(kept_discord_id), discord_id ? discord_id : "", 0);
	discord_id_length = csstrlen(kept_discord_id);
	if (address)
		distributed_address_text(address, text, sizeof(text));
	else
		text[0] = 0;
	length = csstrlen(text);
	hardware_id_length = csstrlen(kept_hardware_id);
	file = fopen(BANS_FILE, "r");
	if (!file)
		return FALSE;
	while (!banned && fgets(line, sizeof(line), file))
	{
		char const *ip = strstr(line, "\tip=");
		char const *hwid = strstr(line, "hwid=");
		char const *until = strstr(line, "\tuntil=");

		/* (a ban for a while that has ended) */
		if (until && strtoul(until + 7, NULL, 10) <= (unsigned long)time(NULL))
			continue;
		ip = ip ? ip + 1 : strstr(line, "ip=");

		/* (the whole address: 1.2.3.4 is not 1.2.3.45) */
		if (length && ip && !strncmp(ip + 3, text, length) &&
			(ip[3 + length] == '\t' || ip[3 + length] == '\n' || ip[3 + length] == '\r' ||
				ip[3 + length] == ' ' || ip[3 + length] == 0))
		{
			banned = TRUE;
		}
		/* (or the whole hardware id) */
		if (hardware_id_length && hwid && !strncmp(hwid + 5, kept_hardware_id, hardware_id_length) &&
			(hwid[5 + hardware_id_length] == '\t' || hwid[5 + hardware_id_length] == '\n' ||
				hwid[5 + hardware_id_length] == '\r' || hwid[5 + hardware_id_length] == ' ' ||
				hwid[5 + hardware_id_length] == 0))
		{
			banned = TRUE;
		}
		/* (or the whole verified Discord id, which a line has without
		"(self-reported)": what a machine said of itself is never matched,
		since anyone could say it) */
		if (discord_id_length)
		{
			char const *discord = strstr(line, "\tdiscord_id=");

			if (discord && !strncmp(discord + 12, kept_discord_id, discord_id_length) &&
				(discord[12 + discord_id_length] == '\t' || discord[12 + discord_id_length] == '\n' ||
					discord[12 + discord_id_length] == '\r' || discord[12 + discord_id_length] == 0))
			{
				banned = TRUE;
			}
		}
	}
	fclose(file);
	return banned;
}

/* (the host: network_server_manager.c, its ban command) a machine banned
by the host: its line in BANS_FILE, and every machine told (the Discord
user of the machine at the index, if it is one in the game; its address,
host byte order). The ban's end (network_distributed_ban_details: 0, for
ever) */
unsigned long network_distributed_ban(
	long machine_index,
	unsigned long address,
	char const *names)
{
	char text[32];
	char kept_names[64];
	char discord_id[DISCORD_ID_SIZE] = "";
	char discord_name[DISCORD_NAME_SIZE] = "";
	char discord[DISCORD_ID_SIZE + DISCORD_NAME_SIZE + 16] = "";
	char notice[MAXIMUM_NOTICE_LENGTH];
	unsigned long until = distributed_next_ban_until;

	distributed_address_text(address, text, sizeof(text));
	if (!distributed_next_ban_unrecorded)
	{
		distributed_write_player_record(BANS_FILE, text, machine_index, names,
			distributed_next_ban_reason[0] ? distributed_next_ban_reason : "banned by the host", until);
	}
	distributed_next_ban_reason[0] = 0;
	distributed_next_ban_until = 0;
	distributed_next_ban_unrecorded = FALSE;
	distributed_printable(kept_names, sizeof(kept_names), names);
	distributed_machine_discord(machine_index, discord_id, sizeof(discord_id), discord_name, sizeof(discord_name),
		FALSE);
	if (discord_id[0] || discord_name[0])
		snprintf(discord, sizeof(discord), " (Discord: %s, %s)", discord_name, discord_id);
	snprintf(notice, sizeof(notice), "%s%s banned by the host", kept_names, discord);
	/* (to every client in the game: in the lobby, the host's own) */
	if (game_in_progress())
		distributed_send_notice(notice);
	else
	{
		console_warning("%s", notice);
		error(_error_log, "%s", notice);
	}
	return until;
}

/* (the host: network_server_manager.c, its kick command) players kicked by
the host, which may join again: every machine told, nothing kept (no line
in BANS_FILE) */
void network_distributed_kick(
	char const *names)
{
	char kept_names[64];
	char notice[MAXIMUM_NOTICE_LENGTH];

	distributed_printable(kept_names, sizeof(kept_names), names);
	snprintf(notice, sizeof(notice), "%s kicked by the host", kept_names);
	/* (to every client in the game: in the lobby, the host's own) */
	if (game_in_progress())
		distributed_send_notice(notice);
	else
	{
		console_warning("%s", notice);
		error(_error_log, "%s", notice);
	}
}

/* (the host) a client machine's tick, which one of its messages is
stamped with: its clock measured, each window, against the host's; one
whose game runs fast (distributed_client_clock) has its players'
predictions refused, and if it goes on, is dropped */
static void distributed_note_client_clock(
	long machine_index,
	long tick)
{
	struct distributed_client_clock *clock;
	unsigned long now = system_milliseconds();
	unsigned long elapsed;

	/* MCC clocks include only host-approved speed changes. Other maps use wall time. */
	now = mcc_game_speed_clock(now);
	if (machine_index < 0 || machine_index >= HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		return;
	clock = &distributed_client_clocks[machine_index];
	if (distributed_handling_stream_message && tick - game_time_get() > CLIENT_CLOCK_AHEAD_TICKS)
		clock->ahead_on_stream = TRUE;
	if (clock->window_tick == NONE)
	{
		clock->latest_tick = tick;
		clock->window_tick = tick;
		clock->window_time = now;
		return;
	}
	if (tick > clock->latest_tick)
		clock->latest_tick = tick;
	elapsed = now - clock->window_time;
	if (elapsed < CLIENT_CLOCK_WINDOW_MILLISECONDS)
		return;
	{
		real rate = (real)(clock->latest_tick - clock->window_tick) /
			((real)elapsed * (real)TICKS_PER_SECOND / 1000.0f);
		long ahead = clock->latest_tick - game_time_get();

		clock->fast = rate > CLIENT_CLOCK_FAST_RATE && ahead > CLIENT_CLOCK_AHEAD_TICKS;
		if (!clock->fast)
		{
			clock->fast_windows = 0;
			clock->ahead_on_stream = FALSE;
			clock->logged_unverified = FALSE;
		}
		else if (++clock->fast_windows == 1)
		{
			error(_error_log, "machine #%ld's game runs %.2f times as fast as this host's (%ld ticks ahead): "
				"its players' predictions refused", machine_index, rate, ahead);
		}
		else if (clock->fast_windows >= CLIENT_CLOCK_FAST_WINDOWS && clock->ahead_on_stream)
		{
			error(_error_log, "machine #%ld's game ran %.2f times as fast as this host's for %d seconds "
				"(%ld ticks ahead): dropped", machine_index, rate,
				CLIENT_CLOCK_FAST_WINDOWS * CLIENT_CLOCK_WINDOW_MILLISECONDS / 1000, ahead);
			{
				char names[64];
				char discord_id[DISCORD_ID_SIZE];
				char discord_name[DISCORD_NAME_SIZE];
				char discord[DISCORD_ID_SIZE + DISCORD_NAME_SIZE + 16] = "";
				char reason[96];
				char text[MAXIMUM_NOTICE_LENGTH];

				distributed_machine_player_names(machine_index, names, sizeof(names));
				distributed_machine_discord(machine_index, discord_id, sizeof(discord_id), discord_name,
					sizeof(discord_name), TRUE);
				if (discord_id[0] || discord_name[0])
					snprintf(discord, sizeof(discord), " (Discord: %s, %s)", discord_name, discord_id);
				snprintf(reason, sizeof(reason), "speed hack (game ran %.2f times as fast)", rate);
				snprintf(text, sizeof(text), "%s%s kicked by the host: their game ran %.2f times as fast (a speed hack)",
					names, discord, rate);
				distributed_send_notice(text);
				distributed_log_cheater(machine_index, names, reason);
				server_event_speed_hack(machine_index, names, rate, ahead, "kicked");
			}
			network_game_server_kick_machine(machine_index, TRUE);
			clock->fast_windows = 0;
			clock->ahead_on_stream = FALSE;
		}
		/* dropped, banned and kept out only when its stream said so too; on
		its datagrams' word alone (which another machine could have sent as
		from its address, to have it dropped and logged as a cheater) it
		stays, its players' predictions refused while they say so, and is
		logged once, unverified (dropped as above if its stream says so later) */
		else if (clock->fast_windows >= CLIENT_CLOCK_FAST_WINDOWS)
		{
			clock->fast_windows = CLIENT_CLOCK_FAST_WINDOWS;
			if (!clock->logged_unverified)
			{
				char names[64];
				char reason[96];
				char address[32];

				error(_error_log, "machine #%ld's datagrams said its game ran %.2f times as fast as this host's "
					"for %d seconds (%ld ticks ahead), its stream not: not dropped (unverified)", machine_index, rate,
					CLIENT_CLOCK_FAST_WINDOWS * CLIENT_CLOCK_WINDOW_MILLISECONDS / 1000, ahead);
				distributed_machine_player_names(machine_index, names, sizeof(names));
				snprintf(reason, sizeof(reason), "unverified speed hack (its datagrams only, not dropped: "
					"game ran %.2f times as fast)", rate);
				distributed_machine_address_text(machine_index, address, sizeof(address));
				distributed_write_player_record(CHEATERS_FILE, address, machine_index, names, reason, 0);
				server_event_speed_hack(machine_index, names, rate, ahead, "unverified");
				clock->logged_unverified = TRUE;
			}
		}
	}
	clock->window_tick = clock->latest_tick;
	clock->window_time = now;
}

/* whether a client machine's game runs fast (distributed_note_client_clock):
its players' predictions not taken */
boolean distributed_machine_clock_fast(
	long machine_index)
{
	return machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
		distributed_client_clocks[machine_index].fast;
}

/* whether the messages handled now came in a batch (the unreliable ones;
one sent reliably comes on its own): a killing blow sent again reliably is
not overtaken by the newer damage sent with the ticks since
(distributed_message_stale) */
static boolean distributed_handling_batch;

/* Only the host's ordered boundary can change worlds. A new datagram may
overtake it, and an old reliable kill may trail newer state in its old
world: neither is authority to adopt a different epoch. */
static boolean distributed_message_epoch_valid(
	long machine_index,
	struct distributed_message_header const *header,
	word size)
{
	if (header->type == _distributed_message_checkpoint)
	{
		if (machine_index != NONE || game_connection() != _game_connection_network_client ||
			network_connection_last_read_was_unreliable() || distributed_handling_batch ||
			size != sizeof(*header) || header->count != 0 || header->checkpoint_epoch == 0)
			return FALSE;
		if (distributed_checkpoint_epoch != 0 &&
			(long)(header->checkpoint_epoch - distributed_checkpoint_epoch) <= 0)
			return FALSE;
		distributed_checkpoint_epoch = header->checkpoint_epoch;
		distributed_checkpoint_synchronized = FALSE;
		distributed_restore_checkpoint();
		/* Initial ready can precede the boundary. Its identity/build may
		have gone with epoch zero and been refused by the host. */
		distributed_identity_sent = FALSE;
		distributed_build_sent = FALSE;
		return FALSE;
	}
	/* A late joiner does not yet know the epoch. Readiness may request the
	current boundary/snapshot, within the existing host resend rate limit;
	it cannot change the epoch or authorize predictions and hit reports. */
	if (header->type == _distributed_message_client_ready)
		return game_connection() == _game_connection_network_server &&
			machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
			distributed_handling_stream_message && !distributed_handling_batch &&
			size == sizeof(*header) && header->count <= 1;
	if (distributed_checkpoint_epoch == 0 || header->checkpoint_epoch != distributed_checkpoint_epoch)
		return FALSE;
	if (game_connection() == _game_connection_network_client)
	{
		if (machine_index != NONE)
			return FALSE;
		if (header->type == _distributed_message_object_changes &&
			(network_connection_last_read_was_unreliable() || distributed_handling_batch))
			return FALSE;
		if (header->type == _distributed_message_objects_synchronized)
		{
			if (network_connection_last_read_was_unreliable() || distributed_handling_batch ||
				size != sizeof(*header) || header->count != 0)
				return FALSE;
			if (!distributed_checkpoint_synchronized)
			{
				/* The client still simulates during transfer. Retire hits,
				projectiles and delayed effects produced by those provisional
				replicas before gameplay resumes. Keep the ordered snapshot's
				receive times: older same-epoch datagrams must remain stale. */
				network_objects_discard_client_effects();
				network_damage_new_game();
				distributed_reset_checkpoint_state(FALSE);
				/* These producers remember a send before the common sender
				can suppress it during an incomplete snapshot. */
				distributed_identity_sent = FALSE;
				distributed_build_sent = FALSE;
			}
			distributed_checkpoint_synchronized = TRUE;
		}
		/* A current-world kill may overtake the live description in the
		initial snapshot. Apply gameplay only after its ordered end marker;
		otherwise that description could resurrect the newly killed copy. */
		if (!distributed_checkpoint_synchronized)
			return header->type == _distributed_message_object_changes ||
				(header->type == _distributed_message_inventories &&
					!network_connection_last_read_was_unreliable() && !distributed_handling_batch);
	}
	return TRUE;
}

/* a message of the distributed kind; machine_index is the sender's on the
host, NONE on a client */
void network_distributed_handle_message(
	long machine_index,
	word const *message,
	word size)
{
	struct distributed_message_header header;
	void const *entries = (byte const *)message + sizeof(header);
	short index;
	word entry_size;

	/* (voice chat's, in the lobby too: network_voice.c) */
	if (network_aim_assist_handles_message(message, size))
	{
		network_aim_assist_handle_message(machine_index, message, size);
		return;
	}
	if (network_riding_map_handles_message(message, size))
	{
		network_riding_map_handle_message(machine_index, message, size);
		return;
	}
	if (network_shield_effects_handles_message(message, size))
	{
		network_shield_effects_handle_message(machine_index, message, size);
		return;
	}
	if (network_voice_handles_message(message, size))
	{
		network_voice_handle_message(machine_index, message, size);
		return;
	}
	if (mcc_load_network_handles_message(message, size))
	{
		mcc_load_network_handle_message(machine_index, message, size, machine_index != NONE ?
			distributed_handling_stream_message : !network_connection_last_read_was_unreliable());
		return;
	}
	/* MCC validation has its own bounded hold, independent of file sharing. */
	if (mcc_admission_handles_message(message, size))
	{
		mcc_admission_handle_message(machine_index, message, size, machine_index != NONE ?
			distributed_handling_stream_message : !network_connection_last_read_was_unreliable());
		return;
	}
	/* (MCC package sharing's, the same way: mcc_share.c) */
	if (mcc_share_game_handles_message(message, size))
	{
		mcc_share_game_handle_message(machine_index, message, size, machine_index != NONE ?
			distributed_handling_stream_message : !network_connection_last_read_was_unreliable());
		return;
	}
	/* (map sharing's, in the lobby too, over the host's connection alone:
	map_share.c) */
	if (map_share_handles_message(message, size))
	{
		map_share_handle_message(machine_index, message, size, machine_index != NONE ?
			distributed_handling_stream_message : !network_connection_last_read_was_unreliable());
		return;
	}
	/* (a dedicated server's map vote's, in the lobby too, over the host's
	connection alone: network_map_vote.c) */
	if (network_map_vote_handles_message(message, size))
	{
		network_map_vote_handle_message(machine_index, message, size, machine_index != NONE ?
			distributed_handling_stream_message : !network_connection_last_read_was_unreliable());
		return;
	}
	/* (none between games: loading, or in the menus) */
	if (size < sizeof(header) || !game_in_progress())
		return;
	csmemcpy(&header, message, sizeof(header));
	/* MCC messages never execute in an ordinary OpenCE session, even in a batch. */
	if (header.type >= _distributed_message_mcc_grenades && !mcc_cache_tags_loaded())
		return;
	/* a tick's messages in one: each as if it came alone */
	if (header.type == _distributed_message_batch)
	{
		word offset = sizeof(header);

		while (offset + sizeof(word) <= size)
		{
			word buffer[(sizeof(message_header) + DATAGRAM_MAXIMUM_SIZE + 1) / sizeof(word)];
			word length;

			csmemcpy(&length, (byte const *)message + offset, sizeof(word));
			offset += sizeof(word);
			if (length > size - offset || length < sizeof(header) - sizeof(message_header) ||
				sizeof(message_header) + length > sizeof(buffer))
			{
				break;
			}
			csmemcpy(buffer, message, sizeof(message_header));
			csmemcpy((byte *)buffer + sizeof(message_header), (byte const *)message + offset, length);
			offset += length;
			/* (no batch in a batch) */
			if (((struct distributed_message_header const *)buffer)->type != _distributed_message_batch)
			{
				distributed_handling_batch = TRUE;
				network_distributed_handle_message(machine_index, buffer, (word)(sizeof(message_header) + length));
				distributed_handling_batch = FALSE;
			}
		}
		return;
	}
	/* (entries of a size of their own: their least here, and each read no
	further than the message's end) */
	switch (header.type)
	{
	case _distributed_message_player_prediction:
	case _distributed_message_unit_states: entry_size = DISTRIBUTED_UNIT_STATE_MINIMUM_SIZE; break;
	case _distributed_message_player_statistics: entry_size = sizeof(struct distributed_player_statistics); break;
	case _distributed_message_pings: entry_size = sizeof(struct distributed_player_ping); break;
	case _distributed_message_mcc_grenades: entry_size = mcc_network_inventory_entry_size(); break;
	case _distributed_message_mcc_campaign: entry_size = mcc_network_campaign_entry_size(); break;
	case _distributed_message_mcc_game_speed:
		/* A clock command is host-only and ordered, never accepted from datagrams. */
		if (machine_index != NONE || network_connection_last_read_was_unreliable() ||
			header.count != 1 || size != sizeof(header) + sizeof(real)) return;
		entry_size = sizeof(real);
		break;
	case _distributed_message_mcc_object_names: entry_size = mcc_object_names_entry_size(); break;
	case _distributed_message_actor_states: entry_size = network_actors_entry_size(); break;
	case _distributed_message_structure_bsp: entry_size = sizeof(struct distributed_structure_bsp); break;
	case _distributed_message_coop_presentation: entry_size = network_coop_presentation_entry_size(); break;
	case _distributed_message_coop_events: entry_size = network_coop_event_entry_size(); break;
	case _distributed_message_coop_skip_vote: entry_size = network_coop_skip_vote_entry_size(); break;
	case _distributed_message_coop_device_groups: entry_size = network_coop_device_group_entry_size(); break;
	case _distributed_message_coop_object_names: entry_size = network_coop_object_names_entry_size(); break;
	case _distributed_message_coop_object_transforms: entry_size = network_coop_object_transform_entry_size(); break;
	case _distributed_message_actor_damage: entry_size = network_actors_damage_entry_size(); break;
	case _distributed_message_coop_object_looks: entry_size = network_coop_object_look_entry_size(); break;
	case _distributed_message_damage_animations: entry_size = network_objects_damage_animation_entry_size(); break;
	case _distributed_message_coop_screen_effect: entry_size = network_coop_screen_effect_entry_size(); break;
	case _distributed_message_coop_device_states: entry_size = network_coop_device_state_entry_size(); break;
	case _distributed_message_pickups: entry_size = sizeof(struct distributed_pickup); break;
	case _distributed_message_votekick: entry_size = network_votekick_request_entry_size(); break;
	case _distributed_message_votekick_status: entry_size = network_votekick_status_entry_size(); break;
	case _distributed_message_chat_up:
	case _distributed_message_chat_down: entry_size = network_chat_line_entry_size(); break;
	case _distributed_message_chat_config: entry_size = network_chat_config_entry_size(); break;
	case _distributed_message_player_inputs: entry_size = sizeof(struct distributed_player_input); break;
	case _distributed_message_relayed_actions: entry_size = DISTRIBUTED_RELAYED_ACTION_MINIMUM_SIZE; break;
	case _distributed_message_game_state:
	case _distributed_message_checkpoint:
	case _distributed_message_objects_synchronized:
	case _distributed_message_notice:
	case _distributed_message_client_ready: entry_size = 0; break;
	case _distributed_message_client_identity: entry_size = sizeof(struct distributed_client_identity); break;
	case _distributed_message_client_build: entry_size = sizeof(struct distributed_client_build); break;
	case _distributed_message_damage_events:
	case _distributed_message_hit_reports:
	case _distributed_message_fire_origins: entry_size = network_damage_entry_size(header.type); break;
	default: entry_size = network_objects_entry_size(header.type); break;
	}
	if (header.type == 0 || header.type >= NUMBER_OF_DISTRIBUTED_MESSAGES ||
		size < sizeof(header) + header.count * entry_size)
	{
		return;
	}
	/* (the kinds whose handlers read one entry: not without it) */
	switch (header.type)
	{
	case _distributed_message_structure_bsp:
	case _distributed_message_coop_presentation:
	case _distributed_message_coop_object_names:
	case _distributed_message_coop_skip_vote:
	case _distributed_message_votekick:
	case _distributed_message_votekick_status:
	case _distributed_message_chat_up:
	case _distributed_message_chat_down:
	case _distributed_message_chat_config:
		if (header.count < 1)
			return;
		break;
	default:
		break;
	}
	distributed_statistics.received++;

	/* (the kinds a client sends only over its stream: in a datagram, which
	anyone can send as from it, they are not its) */
	if ((header.type == _distributed_message_client_ready ||
			header.type == _distributed_message_client_identity ||
			header.type == _distributed_message_client_build ||
			header.type == _distributed_message_hit_reports) &&
		!distributed_handling_stream_message)
	{
		return;
	}
	/* (each kind from the host, or from a client) */
	switch (header.type)
	{
	case _distributed_message_player_prediction:
	case _distributed_message_client_ready:
	case _distributed_message_client_identity:
	case _distributed_message_client_build:
	case _distributed_message_hit_reports:
	case _distributed_message_vehicle_prediction:
	case _distributed_message_player_inputs:
	case _distributed_message_coop_skip_vote:
	case _distributed_message_votekick:
	case _distributed_message_chat_up:
	case _distributed_message_fire_origins:
		if (machine_index == NONE || game_connection() != _game_connection_network_server)
			return;
		/* (heard from: the latency meter's connection problem) */
		if (machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES)
			distributed_round_trips[machine_index].heard = system_milliseconds();
		break;
	default:
		if (game_connection() != _game_connection_network_client)
			return;
		break;
	}
	if (!distributed_message_epoch_valid(machine_index, &header, size))
		return;
	/* Readiness can name an earlier world; it requests the current boundary
	but supplies no clock or simulation evidence for that world. */
	if (header.type != _distributed_message_client_ready &&
		!distributed_message_time_bounded(machine_index, header.game_time,
		machine_index == NONE ? !network_connection_last_read_was_unreliable() : distributed_handling_stream_message))
	{
		return;
	}
	/* (a client: the host's latest tick, which its input messages tell it
	back; and when it came, the latency meter's) */
	if (machine_index == NONE &&
		(distributed_host_time == NONE || header.game_time > distributed_host_time))
	{
		distributed_host_time = header.game_time;
		distributed_host_time_received = system_milliseconds();
	}
	if ((distributed_handling_batch || header.type != _distributed_message_damage_events) &&
		distributed_message_stale(machine_index, &header))
	{
		return;
	}
	/* (the host: a client's clock, by its messages' ticks; and its players'
	predictions not taken while its game runs fast) */
	if (machine_index != NONE && header.type != _distributed_message_client_ready)
	{
		distributed_note_client_clock(machine_index, header.game_time);
		if ((header.type == _distributed_message_player_prediction ||
				header.type == _distributed_message_vehicle_prediction) &&
			distributed_machine_clock_fast(machine_index))
		{
			return;
		}
	}

	switch (header.type)
	{
	case _distributed_message_player_prediction:
		distributed_handle_predictions(machine_index, header.game_time, (byte const *)entries,
			(byte const *)message + size, header.count);
		break;
	case _distributed_message_unit_states:
		distributed_handle_unit_states(header.game_time, (byte const *)entries, (byte const *)message + size,
			header.count);
		break;
	case _distributed_message_actor_states:
		network_actors_handle_states(entries, header.count);
		break;
	case _distributed_message_structure_bsp:
		distributed_handle_structure_bsp((struct distributed_structure_bsp const *)entries);
		break;
	case _distributed_message_coop_presentation:
		network_coop_handle_presentation(entries, header.game_time);
		break;
	case _distributed_message_coop_events:
		network_coop_handle_events(entries, header.count);
		break;
	case _distributed_message_coop_skip_vote:
		network_coop_handle_skip_vote(machine_index, entries);
		break;
	case _distributed_message_votekick:
		network_votekick_handle_request(machine_index, entries, distributed_handling_stream_message);
		break;
	case _distributed_message_votekick_status:
		network_votekick_handle_status(entries);
		break;
	case _distributed_message_chat_up:
		network_chat_handle_up(machine_index, entries, distributed_handling_stream_message);
		break;
	/* (the host's, over its connection's stream alone: a datagram from its
	address could be anyone's) */
	case _distributed_message_chat_down:
		network_chat_handle_down(entries, !network_connection_last_read_was_unreliable());
		break;
	case _distributed_message_chat_config:
		network_chat_handle_config(entries, !network_connection_last_read_was_unreliable());
		break;
	case _distributed_message_coop_device_groups:
		network_coop_handle_device_groups(entries, header.count);
		break;
	case _distributed_message_coop_object_names:
		network_coop_handle_object_names(entries);
		break;
	case _distributed_message_coop_object_transforms:
		network_coop_handle_object_transforms(entries, header.count);
		break;
	case _distributed_message_actor_damage:
		network_actors_handle_damage(entries, header.count);
		break;
	case _distributed_message_coop_object_looks:
		network_coop_handle_object_looks(entries, header.count);
		break;
	case _distributed_message_damage_animations:
		network_objects_handle_damage_animations(entries, header.count);
		break;
	case _distributed_message_coop_screen_effect:
		network_coop_handle_screen_effect(entries, header.count);
		break;
	case _distributed_message_coop_device_states:
		network_coop_handle_device_states(entries, header.count);
		break;
	case _distributed_message_player_statistics:
	{
		/* the host's count of kills, deaths, ... */
		struct distributed_player_statistics const *players = (struct distributed_player_statistics const *)entries;

		for (index = 0; index < header.count; index++)
		{
			struct player_datum *player = distributed_player(players[index].player_index);

			if (player)
				player->statistics = players[index].statistics;
		}
		break;
	}
	case _distributed_message_pings:
	{
		/* the host's measure of each player's ping */
		struct distributed_player_ping ping;

		for (index = 0; index < header.count; index++)
		{
			csmemcpy(&ping, (byte const *)entries + index * sizeof(ping), sizeof(ping));
			if (ping.player_index < MAXIMUM_TRACKED_PLAYERS)
				distributed_player_pings[ping.player_index] = ping.milliseconds;
		}
		break;
	}
	case _distributed_message_inventories:
		network_objects_handle_inventories(entries, header.count);
		break;
	case _distributed_message_mcc_grenades:
		mcc_network_handle_inventories(entries, header.count);
		break;
	case _distributed_message_mcc_campaign:
		mcc_network_handle_campaign(entries, header.count);
		break;
	case _distributed_message_mcc_game_speed:
		mcc_game_speed_receive(entries, header.count);
		break;
	case _distributed_message_mcc_object_names:
		mcc_object_names_receive(entries, header.count, header.game_time);
		break;
	case _distributed_message_object_changes:
		network_objects_handle_changes(entries, header.count);
		break;
	case _distributed_message_object_states:
		network_objects_handle_states(entries, header.count, header.game_time);
		break;
	case _distributed_message_game_state:
		game_engine_read_network_state((byte const *)entries, size - sizeof(header));
		break;
	case _distributed_message_objects_synchronized:
		network_objects_handle_synchronized();
		break;
	case _distributed_message_client_identity:
		/* (a client's Discord user, as it tells it: kept only of what is
		allowed, whatever it sent; a machine whose pass checked is its
		verified account's, and what it tells is not taken: network_link.c) */
		if (header.count >= 1 && machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES &&
			!network_link_machine_verified(machine_index, NULL, 0, NULL, 0))
		{
			struct distributed_client_identity identity;

			csmemcpy(&identity, entries, sizeof(identity));
			identity.discord_id[sizeof(identity.discord_id) - 1] = 0;
			identity.discord_name[sizeof(identity.discord_name) - 1] = 0;
			p2p_discord_sanitize(distributed_client_identities[machine_index].discord_id,
				sizeof(distributed_client_identities[machine_index].discord_id), identity.discord_id, 0);
			p2p_discord_sanitize(distributed_client_identities[machine_index].discord_name,
				sizeof(distributed_client_identities[machine_index].discord_name), identity.discord_name, 1);
		}
		break;
	case _distributed_message_client_build:
		/* (a client's build, as it tells it: its text kept printable, and
		said once in the log) */
		if (header.count >= 1 && machine_index >= 0 && machine_index < HALO_PORT_MAXIMUM_NETWORK_MACHINES)
		{
			struct distributed_client_build told;
			struct distributed_client_build *kept = &distributed_client_builds[machine_index];
			char names[64];
			char mods[DISTRIBUTED_CLIENT_BUILD_MODS_SIZE];
			long index;

			csmemcpy(&told, entries, sizeof(told));
			told.number[sizeof(told.number) - 1] = 0;
			told.flavor[sizeof(told.flavor) - 1] = 0;
			told.repository[sizeof(told.repository) - 1] = 0;
			told.version[sizeof(told.version) - 1] = 0;
			told.mods[sizeof(told.mods) - 1] = 0;
			distributed_build_text(kept->number, sizeof(kept->number), told.number, FALSE);
			distributed_build_text(kept->flavor, sizeof(kept->flavor), told.flavor, FALSE);
			distributed_build_text(kept->repository, sizeof(kept->repository), told.repository, FALSE);
			distributed_build_text(kept->version, sizeof(kept->version), told.version, FALSE);
			distributed_build_text(kept->mods, sizeof(kept->mods), told.mods, TRUE);
			distributed_client_build_known[machine_index] = TRUE;
			distributed_machine_player_names(machine_index, names, sizeof(names));
			snprintf(mods, sizeof(mods), "%s", kept->mods);
			for (index = 0; mods[index]; index++)
			{
				if (mods[index] == '\n')
					mods[index] = ';';
			}
			platform_log("network: machine %ld (%s) plays %s, from %s; mods: %s", machine_index,
				names[0] ? names : "no player yet", kept->version[0] ? kept->version : "a build it does not name",
				kept->repository[0] ? kept->repository : "a repository it does not name", mods[0] ? mods : "none");
		}
		break;
	case _distributed_message_notice:
	{
		/* the host's text, in red on the console: printable, and ended */
		char text[MAXIMUM_NOTICE_LENGTH];
		long length = size - sizeof(header);
		long index;

		if (length > MAXIMUM_NOTICE_LENGTH - 1)
			length = MAXIMUM_NOTICE_LENGTH - 1;
		csmemcpy(text, entries, length);
		text[length] = 0;
		for (index = 0; index < length && text[index]; index++)
		{
			if (text[index] < 32 || text[index] > 126)
				text[index] = '?';
		}
		/* (shown whatever game.console_log says, which keeps the console's
		own lines off the screen by default: the chat's notices) */
		network_chat_host_notice(text);
		error(_error_log, "the host: %s", text);
		break;
	}
	case _distributed_message_client_ready:
	{
		boolean loaded = distributed_machine_loaded(machine_index);

		/* (its count 1: the client asks again, having failed to make one of
		the host's objects) */
		network_aim_assist_send_policy(machine_index);
		network_shield_effects_send(machine_index);
		network_riding_map_send(machine_index);
		network_objects_client_asked(machine_index, header.count != 0);
		/* (a machine new at its index, which may have joined the game in
		progress: every player's statistics and the game type's state, after
		the objects it names) */
		if (loaded)
		{
			distributed_send_all_statistics(machine_index);
			distributed_send_game_state(machine_index);
		}
		break;
	}
	case _distributed_message_damage_events:
		network_damage_handle_events(entries, header.count);
		break;
	case _distributed_message_hit_reports:
		network_damage_handle_reports(machine_index, entries, header.count);
		break;
	case _distributed_message_fire_origins:
		network_damage_handle_fire_origins(machine_index, entries, header.count);
		break;
	case _distributed_message_vehicle_prediction:
		network_objects_handle_vehicle_prediction(machine_index, entries, header.count);
		break;
	case _distributed_message_player_inputs:
		distributed_handle_inputs(machine_index, (struct distributed_player_input const *)entries, header.count);
		break;
	case _distributed_message_relayed_actions:
		distributed_handle_actions((byte const *)entries, (byte const *)message + size, header.count);
		break;
	case _distributed_message_pickups:
	{
		/* what the host says this machine's players picked up */
		struct distributed_pickup const *pickups = (struct distributed_pickup const *)entries;

		for (index = 0; index < header.count; index++)
		{
			long player_index = distributed_player_from_byte(pickups[index].player_index);

			if (distributed_player_is_local(player_index))
			{
				network_player_show_pickup(player_index, pickups[index].kind, pickups[index].definition_index,
					pickups[index].count);
			}
		}
		break;
	}
	}
}

/* (the host: network_server_message_handler.c) a client machine's message
of the distributed kind that came over its stream, which no other machine
can send as it: as network_distributed_handle_message */
void network_distributed_handle_stream_message(
	long machine_index,
	word const *message,
	word size)
{
	distributed_handling_stream_message = TRUE;
	network_distributed_handle_message(machine_index, message, size);
	distributed_handling_stream_message = FALSE;
}

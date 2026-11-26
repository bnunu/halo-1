/*
HALO_PORT_LIMITS.H

Multiplayer session limits of the native builds (Windows, Linux, Android),
force-included by halo_linux_prefix.h and halo_windows_prefix.h.

The Xbox game allows 16 players on at most 4 machines (up to 4 players each
on split screen). The native builds allow 128 players on up to 128
machines; split screen stays at 4 players per machine.

128 is the largest session that fits the game's existing records: player,
machine and team indices are stored in signed chars (0..127 with NONE), and
a finishing place in 7 bits.
*/

#ifndef __HALO_PORT_LIMITS_H
#define __HALO_PORT_LIMITS_H

/* ---------- session limits */

#define HALO_PORT_MAXIMUM_NETWORK_PLAYERS 128
#define HALO_PORT_MAXIMUM_NETWORK_MACHINES 128

/* a host polls its listening socket and one socket per machine; the Xbox's
Winsock headers default to 64 (the prefix headers define FD_SETSIZE from
this before any of them is read) */
#define HALO_PORT_FD_SETSIZE 256

/* ---------- memory capacity for these limits (game state and pools) */

#include "halo_port_capacity.h"

/* ---------- struct network_game layout

The game settings record (struct network_game) is declared separately in
several networking and interface units; its layout follows from the limits.
Every copy checks its offsets against these values. The Xbox values (4
machines, 16 players) are 0x226 and 0x434. */

#define HALO_PORT_NETWORK_MACHINE_SIZE 0x44
#define HALO_PORT_NETWORK_PLAYER_SIZE 0x20
#define HALO_PORT_NETWORK_GAME_MACHINES_OFFSET 0x114
#define HALO_PORT_NETWORK_GAME_PLAYER_COUNT_OFFSET \
	(HALO_PORT_NETWORK_GAME_MACHINES_OFFSET + HALO_PORT_MAXIMUM_NETWORK_MACHINES * HALO_PORT_NETWORK_MACHINE_SIZE)
#define HALO_PORT_NETWORK_GAME_PLAYERS_OFFSET (HALO_PORT_NETWORK_GAME_PLAYER_COUNT_OFFSET + 2)
#define HALO_PORT_NETWORK_GAME_PLAYERS_END \
	(HALO_PORT_NETWORK_GAME_PLAYERS_OFFSET + HALO_PORT_MAXIMUM_NETWORK_PLAYERS * HALO_PORT_NETWORK_PLAYER_SIZE)
#define HALO_PORT_NETWORK_GAME_RANDOM_SEED_OFFSET (HALO_PORT_NETWORK_GAME_PLAYERS_END + 2)
/* (then the gametype's PC options, struct game_variant_options: 0x2C bytes) */
#define HALO_PORT_NETWORK_GAME_VARIANT_OPTIONS_OFFSET (HALO_PORT_NETWORK_GAME_PLAYERS_END + 0xA)
#define HALO_PORT_NETWORK_GAME_LOCAL_DATA_OFFSET (HALO_PORT_NETWORK_GAME_PLAYERS_END + 0x36)
#define HALO_PORT_NETWORK_GAME_SIZE (HALO_PORT_NETWORK_GAME_PLAYERS_END + 0x3A + 64) /* MCC content and metadata SHA-256 */

/* ---------- system link protocol

The native builds' messages differ from the Xbox game's (longer arrays, the
game settings record in fragments), so they search for games with their own
protocol version and never see the Xbox game's, or it theirs. */

#define HALO_PORT_NETWORK_GAME_MESSAGE_VERSION 2

/* The native builds' network code has a version of its own (an unsigned
16-bit number). A host advertises a version, with its netcode, in its game's
advertisement's reserved bytes (network_server_message_handler.c), which
hosts built before there was a version send as zeros: version 0. A client
joins a host of a version this build plays with
(HALO_PORT_NETWORK_VERSION_PLAYS), and otherwise tells the player which is
newer (network_client_manager.c); a host takes a client of one.
Raise it with any change to what the machines send each other. */
/* Version 42 integrates MCC admission/sharing and shared checkpoint, boat,
clock and scripted-pose agreement. Every map family uses the same version.
Version 43: an MCC map's damage impulses follow the damage's direction, a
vehicle on its side throws its riders, chat marks the host's lines, a co-op
cutscene's sound carries how far the host has played it, and a machine's
MCC conversion holds a round's start out of an allowance; a player standing
on a vehicle rides it (a unit's state carries the deck it stands on, the map's
riding choice has its own message: port/linux/NETCODE.md).
Version 44: MCC package offers are automatic under Map Sharing, and a joiner
may satisfy omitted base resources with its own validated local files. */
#define HALO_PORT_NETWORK_VERSION 44
/* The oldest network version this build plays with. A version raised for a
change that machines of the version before it still play with (what they
send each other laid out as before, and nothing either side does that the
other's game needs to know) leaves it where it is; a change that older
machines cannot play with raises it to the new version. 43 plays with 44:
44 changed MCC package sharing alone (port/linux/network_format.txt is the
same; NETCODE.md, "Versions"), so a build of 44 joins a host of 43 and takes
its machines, but not on an MCC map: a host of 44 leaves out of a package
the base resources a machine of 43 cannot find on its own, so an MCC game
takes no machine older than HALO_PORT_NETWORK_VERSION_OLDEST_MCC. */
#define HALO_PORT_NETWORK_VERSION_OLDEST 43
#define HALO_PORT_NETWORK_VERSION_OLDEST_MCC 44
/* The version a host advertises when network.advertise_version chooses no
other (p2p_lobby.c's network_version_advertised). A build released before
there was a range joins only a host that advertises its own version, so
while a newer version plays with an older one, keep this at the version the
builds players have are of: theirs still join, and this build's join a host
of either. */
#define HALO_PORT_NETWORK_VERSION_ADVERTISED HALO_PORT_NETWORK_VERSION
/* whether a machine of network version theirs plays with this one (mcc: in
a game on an MCC map) */
#define HALO_PORT_NETWORK_VERSION_PLAYS(theirs, mcc) \
	((unsigned int)(theirs) >= ((mcc) ? HALO_PORT_NETWORK_VERSION_OLDEST_MCC : HALO_PORT_NETWORK_VERSION_OLDEST) && \
	(unsigned int)(theirs) <= HALO_PORT_NETWORK_VERSION)
/* ... the advertisement's reserved bytes: the version (a little-endian word),
then flags, then the game's kind (an OpenCE gametype's mode, and Hardcore:
port/linux/game/game_modes.h's game_modes_kind), for the server browser */
#define HALO_PORT_ADVERTISED_VERSION_OFFSET 0
#define HALO_PORT_ADVERTISED_FLAGS_OFFSET 2
#define HALO_PORT_ADVERTISED_KIND_OFFSET 3
/* ... the host plays the distributed netcode (always, since the lockstep
netcode was removed; hosts of version 4 built before then may not) */
#define HALO_PORT_ADVERTISED_DISTRIBUTED_FLAG 0x01
/* ... the game is under way (loading, playing or over), not in its lobby:
the menus show it before joining it (hosts built before then never set it) */
#define HALO_PORT_ADVERTISED_IN_PROGRESS_FLAG 0x02
/* ... the host is a dedicated server, with no player of its own (halo
--server: port/linux/src/server_mode.c): the menus say so (from version 39) */
#define HALO_PORT_ADVERTISED_DEDICATED_FLAG 0x04

/* a message header's 12-bit length allows messages of up to 0xFFF bytes,
header included; the per-tick update of 128 players is 3,857 */
#define HALO_PORT_MAXIMUM_NETWORK_MESSAGE_SIZE 0x1000

/* the packet codec's limit on a decoded or encoded packet; the per-tick
update of 128 players decodes to 0x1010 bytes */
#define HALO_PORT_NETWORK_PACKET_SIZE 0x1100

/* the game settings record (HALO_PORT_NETWORK_GAME_SIZE, 13,200 bytes at 128
machines and players) does not fit one message; it is sent in pieces of
this many bytes (4 pieces), each 3,594 bytes on the wire */
#define HALO_PORT_NETWORK_GAME_SETTINGS_FRAGMENT_SIZE 0xE00

#endif /* __HALO_PORT_LIMITS_H */

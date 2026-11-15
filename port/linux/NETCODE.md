# Distributed netcode

The Xbox game plays system link in lockstep: clients send their input to
the host, the host sends every machine every player's input for each 30 Hz
tick, and every machine simulates the whole game from them, waiting for
each tick's update. A client therefore sees its own movement and shots a
full round trip late, and any machine whose simulation differs in the last
bit goes out of sync.

The native builds replace that (they no longer have the lockstep netcode)
with the model of later Halo engines (the "distributed" simulation of the
MonkeyNuts/Ares source)
with ideas from VALORANT's netcode articles, keeping the 30 Hz tick:

- **Every machine ticks on its own clock.** Nobody waits for anybody: a
  client no longer runs only the ticks the host has sent. A client's clock
  starts with the host's first game update and takes the host's time from
  it (at a game's start as at a late join), so every machine reads the
  game's timers alike.
- **Own player predicted.** A client drives its own player (and the
  vehicle it drives) from its local input at once. Remote players are
  driven by the inputs the host relays every tick, the latest one held
  until a newer arrives.
- **Actors driven.** Only the host runs the AI. Each tick it sends its
  clients the control its actors gave their units (how they move, where
  they face, aim and look, their trigger and buttons, their animation
  impulses) with the units' state, those near a client's players every
  tick and others less often; a client drives each unit with the latest
  it has, as it drives a remote player's, until it hears nothing of it for
  two seconds (`port/linux/game/network_actors.c`).
- **Co-op.** A network game on a campaign level with no game engine
  (Create Game's Map screen, its SINGLEPLAYER maps, over LAN and the
  internet) is co-op. Only the host runs the
  level's scripts and spawns players. `network_coop.c` sends the clients
  everything the scripts do that they would otherwise miss:
  - every tick: the cinematic, camera, screen fade, the HUD settings the
    scripts control (what is shown, the mission timer), the skip vote, and
    which teams are allies and friends (the scripts' allegiances, so the
    marines are the players' allies on every machine);
  - once each, numbered so nothing is applied twice: script sounds,
    chapter titles, help and objective text, "Checkpoint" messages, screen
    shake, nav points, custom animations on units and scenery, and units
    opening and closing (dropships' doors);
  - device groups (doors, elevators, switches; a client sets none itself,
    and its player's use of one is relayed to the host), and each device's
    position and power as it changes: a client puts its device where the
    host's is once that stops, or if they drift apart while it moves;
  - which named objects exist, so scripted creates and deletes match.

  The camera sent is the host's first local player's. A dedicated server
  has no local player, so while the scripts have the camera
  (`camera_control`) its first observer films their camera as a player's
  would (`director_unattended_scripted_camera`, director.c); before, it
  never ran, and every client watched a cutscene from the world's origin.
  Only the camera's values changed: no network version. A dedicated server
  of an earlier build still sends that camera, which stays as a map's start
  leaves an observer (at the world's origin, looking down x), where no shot
  is. A client that gets it keeps the cutscene's view but watches the
  cutscene's actors instead (`client_watch_cutscene`, network_coop.c): the
  unit the host's scripts last started a custom animation on, which every
  build sends, else its own player, framed as spectating frames a teammate
  the scripts hold.

  The scripts' looping sounds (music, ambience, a cutscene's music and
  dialogue) are resent every two seconds, so a machine that joins hears
  them. The resends of one that plays once (a fake impulse looping sound)
  say how many milliseconds ago the host started it (the event's `target`),
  and a machine that starts it from a resend starts it there
  (`coop_sound_sync.c`): a start sound that has played out is skipped, the
  part playing starts at the permutation and block that time falls in, and a
  sound that has played out does not start. Each machine still picks its own
  pitches and permutations. A start with no time (`NONE`: the scripts' own
  start, and every start from a host built before this) plays from the
  beginning, and a client built before this ignores the time, so neither
  needs a new network version.

  Script actions naming player0 also reach the players beyond player1.
  They use the same reverse datum order as the script's `(players)` list,
  skipping players without units, so a scripted BSP transition does not
  leave the extra players in the unloaded part of the level.

  Every machine follows the host's structure BSP: a switch is sent at once
  and reliably, and each client's input says which BSP it has loaded.
  Until a client has the host's, the host takes none of its players'
  movement and none of its loading zones, and after any switch no loading
  zone switches again until every machine has the new BSP (ten seconds at
  most). Only the host's crossing of a loading zone switches the BSP, and
  it brings every player to the host, however far behind, so no one
  running ahead or doubling back drags the team through the level (while
  none of the host's players are alive, anyone's crossing does); a client
  standing on a loading zone is told it waits for the host. A player
  outside the loaded BSP and falling for two seconds is brought back beside
  the host, else a teammate. A dead
  player watches a living teammate (`coop_spectate.c`) and comes back
  beside one once it is safe. With everyone dead, TEAM WIPE in Co-op
  Options (`network.coop_team_wipe`) defaults to AUTO: private games revert
  the checkpoint; public games respawn at checkpoint positions without
  reverting the world. REVERT CHECKPOINT and RESPAWN ONLY override the
  visibility choice, including public games with passwords. Reverting
  requires a saved checkpoint with the same players and machine
  assignments; otherwise the host logs the fallback and respawns them.
  A mission the scripts fail
  with players still alive reverts to the last checkpoint on the host, as a
  skipped cutscene does (below); a client never reverts on its own. A level
  won ends the round as in multiplayer, and the next round is the
  campaign's next level.

  Cutscenes are skipped by vote: more than half the machines must press
  skip. The host then reverts as single player does, but keeps its clock
  moving forward (the netcode depends on that) and moves the script
  threads' wake times along with it. The object, device and name syncs
  bring the clients up to date.

  The host's EXTRA ENEMIES (`coop_enemies.c`, `network.coop_enemies_mode`)
  give each squad of enemies a level places more of itself: PER PLAYER, a
  percentage of itself for each player past the first, to 8 times its size
  at most (more filled the clients' objects); STATIC MULTIPLIER,
  that many times itself for any number of players. They stand around its
  starting locations on free ground (the same floor, clear of crates and
  other actors, with room to stand), or where none is left on rings about
  them as before, and never take the actors a level needs for its own (the
  actor pool, `halo_port_capacity.h`, holds 1024). Riders a dropship has no
  seats for are kept, and placed beside its riders once they get out. Only
  the host runs the AI, so the clients see them as the host's other actors.

  Each player chooses whether to play with friendly fire between players
  (`network.coop_own_friendly_fire`, FRIENDLY FIRE in the pause menu): two
  players hurt each other only if both do. A client's input tells the host
  its choice. A player with no input for over 70 seconds (no button pressed
  or let go, no stick or trigger held) is hurt by every player, whatever
  either chose, until their next input.
- **Host authoritative.** The host alone decides damage, deaths, spawns,
  pickups, scores and the game's objects; clients do not decide them but
  apply what the host sends.
- **Corrections.** The host sends each client the authoritative state of
  the players' units and the game's moving objects; a client moves its
  copies toward it, a small error half of the way each tick, a larger one
  at once, drawn gliding from where they were (a player's moved at once
  is on the teleporter it lands on, as a teleporter leaves one it sends,
  so the client's does not send it back). A client's own unit and
  vehicle are only corrected past a tolerance, so prediction does not
  rubber-band: the host tells the client which of the client's ticks it
  has its player at (its prediction come back), and the client compares
  that with where it had the player at that tick, not now, and moves the
  player by the difference, keeping what they did since. The host takes a
  client's player where the client says within a tolerance of its own,
  and no further from an anchor (one it took, a newer once a second) than
  a player moves in the client's ticks since, those no more than its own
  since and a little jitter (so the tolerance and the jitter are not gained
  again each tick); a player on foot moves across and up (and the host
  takes their velocity) no faster than twice as fast as they run and jump,
  or as fast as the host's own ticks sent its copy of them in the last few
  seconds or the flight they began then (an explosion's throw, which the
  client learns of a round trip late), whichever is more; down no faster
  than that and a fall from the highest the host has had them since they
  were on the ground; no more than 2 world units a tick, with a twentieth
  of a world unit a tick more for where they are; and no higher above
  where the host last had them on the ground than a jump takes them (as
  fast up as a jump or a run, the legs drawn up, and a world unit more)
  and a throw its ticks gave them, and in the air no higher than a thing
  thrown up at that speed falls to since (the ticks since the host's copy
  left the ground, less the client's round trip, half a second at most,
  and jitter): past it, the
  host's copy falls as its own ticks have it, so no one hovers, walks on
  air or comes down slowly. What the host's ticks sent its copy is
  its speed less as much as the velocity it took of the client was faster
  than the player goes of their own (and up, what its tick added but a
  jump's), so a client that says it goes faster (a copy said to hover and
  fall, gaining the host's gravity each tick) gains nothing by it. A
  teleporter, which moves the host's own copy too, starts afresh. Riding an
  elevator (moving, or at rest with the player on its floor) the host takes
  only where a client's player is across,
  not how high: the client's elevator follows the host's (co-op's device
  sync), so its player's height reaches the host a round trip behind the
  host's elevator, under its floor going up, where the host's copy would
  fall through it. Each machine's elevator carries its own copy up and
  down, and they meet where it stops. Floor contact keeps that protection
  through a stop or reversal; the remembered attachment alone after leaving
  a stopped platform does not. A client echoing a prediction while still on
  the same elevator keeps its local floor height, rather than applying the
  difference between the platforms' phases to its current height, when that
  difference is within 4 world units (the platform's travel over a round
  trip); a greater one, jumps, leaving the platform and authoritative
  teleports correct normally. Rider carry uses the platform's current
  collision displacement, including phase corrections, rather than its
  previous tick's displacement (network version 30).
- **Shooter's hits.** A client reports what its own players hit; the host
  checks the report (the player's, a weapon they carry, fired from within
  its reach, the target where the host had it when the shooter saw it, no
  faster than weapons fire) and deals the damage. What the shooter saw
  hit, hits.

A host's game advertisement says that it plays this netcode. A client does
not join a host that does not (one of network version 4 built before the
lockstep netcode was removed, set to play it): it tells the player why.

## OpenCE network agreement

OpenCE network version 42 replaced version 41 for Xbox, Custom Edition and MCC sessions;
version 44 (below, "Versions") replaces it.
Advertisements and ordinary join requests carry this one version; mismatched
versions are rejected before admission. There is no separate MCC capability
envelope or optional shoulder-camera protocol. Retired fork message IDs 101 and
102 remain unused. The historical version-41 record is retained as a test fixture,
not a crossplay promise.

Every settings transfer sends the complete shared `network_game` record, including
the two 32-byte MCC digests (zero for Xbox/CE). The reassembler pins its total size,
checks offsets and the terminated map name, and clears interrupted staging before
accepting another record. MCC identity resolution and content validation remain
MCC-owned and run only for MCC map names.

The distributed header adds a checkpoint epoch. The host controls checkpoint
recovery; stale updates from an earlier epoch cannot mutate the restored world.
Shared co-op recovery restores the current roster and resets replica state.
Small client-clock deficits recover through bounded actual simulation ticks.

Scripted player poses carry five signed-16-bit direction vectors (30 bytes) for
desired facing/aim/look and current aim/look, plus suspension in the unit flags.
The host sends this extension when input is inhibited or a unit is suspended.
Clients validate the entire extension (a pose that is not unit vectors is
left out, the unit's other state and the message's other units kept); normal
input keeps local prediction and omits the vectors. Matching position does
not suppress orientation correction.

Version 42 also requires initialized three-point boat controls and the corrected
shared console-syntax/checkpoint lifetime. MCC snapshots include extra grenades,
campaign state, named objects and game speed (messages 97-100). Package sharing,
installed-content validation and conversion progress use messages 103-106, with
their own bounds, host authority and map-family checks described below.

The version increase is necessary for both changed wire layouts and simulation
behavior peers must agree on. Tests cover ordinary join admission, exact settings
round trips, message direction, checkpoint epochs, recovery and scripted poses.
Runtime validation and its limits are recorded in the MCC upstream validation log;
passing unit checks alone does not establish every-map compatibility.

## Versions

The native builds' network code has a version, an unsigned 16-bit number
(`HALO_PORT_NETWORK_VERSION` in `port/linux/include/halo_port_limits.h`),
raised with any change to what the machines send each other. A host puts it
in its game's advertisement (reserved bytes that hosts built before there
was a version send as zeros, so they are version 0). A client does not join
a host of another version: it shows a message box that says which of the
two is newer, with both versions ("update the game" or "ask the host to
update"), and stays in the list of games. Version 1 was the first of this
netcode; version 2 lets a machine join a game in progress; version 3 puts
each player in its slot of the host's player list on every machine;
version 4 plays the European (PAL) maps as the North American ones
(`port/linux/game/pal_tags.c`); version 5 corrects a client's own player
against where it had them at the tick the host has them at, and checks
what the machines send each other more closely; version 6 sends a unit's
state and a player's input without the parts that are their defaults, fills
each datagram, sends damage and pickups only to the machines they concern,
stamps a hit with the host's tick the client had heard of, and checks hits
and a client's own player's moves more closely; version 7 sends with a
killing blow its killer's score after it, and with a body's state that it
is dead; version 8 is the first whose clients play by the host's rules
(below), so a build without them joins no host of it; version 9 tells
every machine of a player the host dropped for cheating, each client
tells the host its Discord user, and a machine's join request carries its
hardware id; version 10 sends every player's ping for the scoreboard;
version 11 sends with the game's settings its gametype's PC options;
version 12 plays the campaign together (co-op, above), drives the host's
actors on its clients and sends the flinches and deaths the host picked;
version 13 drives up to 1056 of the host's AI units on its clients (co-op's
extra enemies), where 12 drove 288; version 14 sends co-op's device positions
and its units opening and closing; version 15 sends co-op's allegiances with
its presentation, a message of another size; version 16 has a client's input
say which structure BSP it has loaded (co-op); version 17 breaks the host's
glass and destructible scenery on every machine (and takes a client's hits on
scenery), sends the cluster a co-op cutscene keeps active, and leaves a
failed co-op mission's revert to the host; version 18 sends with an object
the bitmap of its shaders it draws with when its actor variant set one (co-op:
the Elite major's and commander's armor); version 19 sends with the game's
settings whether co-op's players collide with each other (Server Setup's PLAYER
COLLISIONS: each machine's players then pass through the others'); version 20
lists a public game with a password with its invite's token sealed with the
password's key (`p2p_lobby.c`), a listing of another layout; version 21
sends each killing blow again reliably and an object come to rest three
times (a client waits for a player's blow before its body dies without one),
and switches co-op's BSP on the host's crossing alone; version 22 names a
Custom Edition map `custom_maps\<name>`, which older clients cannot load or
explain; version 23 keeps the blocks of a Custom Edition map that 22 cut to
the Xbox tools' limits, so both place the same objects; version 24 sends the
gametype's PC vehicle set; version 25 sends a unit's integrated light with
its object state; version 26 has each machine say its WebRTC in internet
play's signalling (a native build's certificate, a browser's ICE credentials
too), so that browsers play (port/linux/README.md, "Browsers"); version 27
shares Custom Edition maps (below, "Map sharing"): a client asks the host for
a map it lacks, and the game's players reach each other to send it; version
28 holds the players of a machine still downloading the map out of the game
until it has the map, keeps a multiplayer map's first scripted BSP, leaves
dynamic lights and the flashlight on past the Xbox game's player and vehicle
limits, and readies a second weapon picked up while firing; version 29 names
a Custom Edition map by its own name (`custom_maps\hugeass`, which every
screen shows) and says which version it is by the map version of the game
record, its hash's first 32 bits (the header checksum before), which a
client finds its file by (below, "Map sharing"); version 30 carries an
elevator's riders by its movement of the same tick, which the Xbox game did
a tick late, so that every machine moves them alike; version 31 puts the
Xbox game's begin game, graceful exit and pregame keep-alive field lists
under their own messages (its table had each on its neighbour's) and makes
the keep-alive a long, as it was sent: not a byte on the wire moves but the
keep-alive's last two, the sender's stack before and zero now, and the
table's record changed with it; and with it, every machine drops a datagram
of the netcode stamped more than 15 seconds from its sender's latest, the
host takes a client's hits, Discord user and readiness only over its
connection, and the host answers each machine's datagrams where they come
from, so that a second copy of the game on one computer, on a port of its
own, plays too; version 32 sends the host's mouse aim assist policy
(distributed message 90, `network_aim_assist.c`), which a client must not
ignore, and has a client's input say whether its player plays co-op with
friendly fire; version 33 sends with the game's settings whether co-op's
players hurt their AI allies (Server Setup's ALLY FRIENDLY FIRE: the host
then deals the players' hits of them no damage); version 34 takes clients'
hits on, and sends the host's damage to, the map's scenery and devices
whose shield is all damage can break (Rat Race's energy shields, a Custom
Edition map's tripmines), which version 17 left out as having no body to
destroy, and has every machine run those shields' stun and recharge itself;
and the host deals what such an object deals itself (its shield or body
breaking: a tripmine a client's player threw going off under the host's
vehicle), which a client no longer reports and the host took from no one.
One made in the game, a tripmine, is at an index of its own on each machine:
the host takes a client's hit on its copy as one on the host's nearest where
the client had it, and a client the host's damage to one as damage to its own
copy nearest the damage. Then version 35 sends with the game's settings the
options of OpenCE's gametypes (SWAT, Infection, Gun Game and Hardcore:
`port/linux/game/game_modes.c`), which make the gametype's PC options 0x2C
bytes where they were 0x1C, and the game settings record 16 bytes longer; and
in an Infection game the host sends its round in the game type's state
(`struct distributed_infection_state`: each player's role and score, the
round's phase and winner) in place of Slayer's, which each client checks whole
before taking it. Then version 36 sends the host's shield effects (distributed
message 91, `network_shield_effects.c`: the shield glow, its Light Show reach
and the overshield effect, Server Setup's SHIELD EFFECTS), which every client
draws in place of its player's own, so that a build that would draw its own
cannot join. Then version 37 sends with a game's LAN advertisement (the fourth
of its reserved bytes) and internet listing (the first byte of its stamp) the
game's kind: an OpenCE gametype's mode and Hardcore
(`port/linux/game/game_modes.h`'s `game_modes_kind`), which the server
browser names in its gametype column ("SWAT", "HC CTF"); a browser takes a
mode for a Slayer game only, and one it does not know as none. Then version 38
lets the host's shield effects be `players` (SHIELD EFFECTS' PLAYERS' OWN), a
value of distributed message 91 that tells each client to draw its own
settings', not synced. Then version 39 marks a dedicated server's game (a
host with no player of its own, `halo --server`: "Dedicated servers"
below) with a flag of its LAN advertisement (`HALO_PORT_ADVERTISED_DEDICATED_FLAG`,
0x04 of the third reserved byte) and of its internet listing (32), which
the server browser shows. With no new version, each client then tells the
host its build (distributed message 92, `_distributed_message_client_build`,
reliably once a game after its ready, over its connection alone): its
number, configuration, the label it shows ("OpenCE Linux | build 282
(release)"), the GitHub repository it is made from (owner/name, which the
build scripts take from CI's `GITHUB_REPOSITORY` or the checkout's origin
remote) and the mods it plays, a line each ("folder version", in the load
order); the host keeps it printable and logs it once, and a dedicated
server shows it in `/api/players` (`client`). A host without the message
drops it as a kind it does not know.
Distributed message 93 (`network_damage.c`) needs no new version either,
as a host that does not know it drops it: a client whose players' shots
leave from elsewhere than their units' cameras (a VR build, whose weapon
fires from the player's hand) sends the host where from, unreliably when it
changes and every eighth tick: an offset from the camera of no more than 0.4
world units (a player leaning out and an arm). The host keeps it a second,
for that machine's own players only. When none of the shooter's eyes (nor a
vehicle's gun) had a line to a reported hit's target, the host's line of
sight check then also looks from the hand, with the line from the eye to it
clear (a hand through a wall does not hit) and the hand where one firing at
the target can be: no more than 0.05 world units behind the eye along the
line to the target, 0.2 to its side and 0.1 above it (a client cannot claim
a hand far past a corner). Such hits are counted in the host's log, and the
host's `network.fire_origins` (true by default) turns the hand off.
Then version 40 adds to the join request (`message_client_join_game_request`,
0x70 bytes before) a Discord account's pass, 170 bytes, and its proof, 64
(`port/linux/src/player_link.c`; `docs/master_protocol.md`, "Player keys,
links and passes"), all zero for none: a dedicated server refuses a machine
from a VPN's, Tor exit's or datacenter's address with one of five new
rejection codes (`_rejection_code_vpn_link_required` and the four after it,
`port/linux/game/network_link.c`), and a client refused with the first joins
again at once with its pass, bound to the tunnel's session. A build before
them shows the codes as the game being closed.

Then version 41 changes both vote messages' entries (`network_votekick.c`:
the request's first byte, once the player's index, is the vote's kind; the
status's first byte, once the player's index, is the kind, and it carries
the no votes), adds `_rejection_code_address_full` (a host's
`server.max_machines_per_address`), and a dedicated server's map vote
(`network_map_vote.c`), in the lobby too, over the machines' streams as map
sharing's: the host's choices (map and gametype, 8 at most), their votes,
the time left and the winner (`_distributed_message_map_vote`, 94, to each
client reliably every second and as it changes), and a client's choice
(`_distributed_message_map_vote_choice`, 95), taken only from the machine's
own stream, at most four a second, one counted per machine. A co-op
game's flags also tell its clients whether the host is a dedicated server
(`_network_game_cooperative_dedicated_bit`: its loading zones then ask for
most of the living players).

OpenCE network version 42 adds MCC package identity, admission and host-only
sharing to the shared agreement. Join requests carry this version explicitly;
settings carry a zero MCC identity for Xbox/CE. Checkpoint epochs, recovery,
scripted poses and shared boat/console fixes require matching peers.

Then version 43 changes what the machines simulate and send: on an MCC map a
vector damage impulse follows the damage's direction (`mcc_damage.c`), and on
every map a vehicle on its side (`vehicle_is_flipped`: its up below 0.2)
throws its riders, as the retail game does, not only one upside down. Chat
lines carry `_chat_line_host_bit`, and the chat settings the host's machine
(`_chat_config_host_machine_bit`, `network_chat.c`), so every machine draws
the host's name white; a co-op cutscene's sound event carries in `target` the
milliseconds the host has played of it, so a machine that joins during it
starts it there (`coop_sound_sync.c`). A host now waits for a machine's MCC
conversion (message 106) out of that machine's allowance: holding a round's
start beyond the ordinary wait spends it, time not holding earns it back at a
third of the rate, up to the round's limit (`mcc_load_network.h`), so a
machine that pulses without converting cannot hold every round's start. A
version 42 build computes MCC impulses the other way, and its scripted poses'
validation drops the message's other units with a bad pose.

Version 43 also carries a rider standing on a vehicle: the deck part of a unit's
state and the host's riding decision for the game, made at its start and
kept until the map ends, a co-op flag of the game's settings
(`_network_game_cooperative_vehicle_riding_bit`, bit 3;
`port/linux/game/vehicle_riding.c`).
It also carries a rider's cabin: its enclosure verdict while it stands as
well as in the air. It ships with the rest of version 43, which no released
build has.
The deck part's verdict bits (1 open, 2 enclosed, 0 unknown) are sent in
every window tick, riding included. Riding, they carry the standing cabin
verdict (its grace counted by the sender: the host for its broadcast, a
client for its own riders' predictions); in a protected jump,
its raw sample; in a shallow jump, the air verdict as before. Bit 3 of the
flags (the part stays 28 bytes) says that a verdict in the air is cabin
evidence, a protected jump's sample or the host's word for it, rather than
that legacy one. A client takes the host's verdict for the cabin of its own
player and of every other rider, whatever its own samples say.

The host takes a rider's word in its vehicle's frame and checks it there; a
client places riders on its own copy of the deck. The host takes the words of
the players riding in a second pass, after the vehicles of the tick have moved
(the first takes those on foot), so a word naming a place on the deck is
checked against the deck where it now is.
For a round trip and a jitter after the host's copy or the client's changed
between riding and not (a take-off or a landing the two disagree about), a
word within `debug.riding_transition_leash` (0.25 world units, host only) of
where the host's copy stands is taken as the place it names in the world
rather than as a deck place: a position allowance during the disagreement,
never a speed. A vehicle's netcode writes (the host's snaps, a client's corrections)
carry its riders along but are recorded apart from its motion, in a
correction each vehicle keeps, so they never count as the deck's speed. A
vehicle a client follows (the host's word is its place, not its updates) is
measured by the time elapsed since the rider's last tick, and a change of the
vehicle's role (simulated or followed) takes no correction across it.
None of this adds a byte: the cabin bit (bit 3 of the flags) is the only change to the deck part,
which stays 28 bytes, and the version stays 43.

Message 107 carries the host's riding rules: the map's choice (its boolean
`vehicle_riding` global or startup call; none when the map makes none) and
the host's latched `debug.riding_` values (damage speed, teleport step, moved
alone, air grip, look carry, enclosure grace ticks; not the transition
leash, which only the host uses). The host sends it reliably to every client
at each map start, again when either changes, and to a late joiner at its
ready. A client prefers the choice to the cooperative bit and takes the
values over its own config (clamped as its own latch clamps them) until the
next game; before the message it keeps its own. The values are the latched
ones, so a config change on the host waits for the next map; a co-op host's
revert or saved-game load reads the config again, and the new values are
sent again then. It ships with
the rest of version 43, which stays 43.

Version 44 unifies MCC offers under Map Sharing and lets a receiver satisfy
omitted base resources with its own validated local files. Message layouts
are unchanged; the changed sharing agreement requires matching peers.

`port/linux/network_format.txt` records the wire format of the current
version (`tools/network_format.py`): `test_linux_port.py` fails when the
sources' messages, kinds, bits, structs sent whole or the sizes of their
arrays differ from it.

A client plays by its host's rules: in another's game (searching for it,
in its lobby, or playing it) the developer console, the telnet console
and the cheat buttons run only commands that change nothing of the game
(what the machine shows and how its controls feel: `hs_compile_and_evaluate`),
and each frame and tick the game's cheats, speed, autoaim, magnetism and
rider ejection are put back to the host's, and what it draws of the world
to what everyone's draws (no wireframe, debug drawing mode, environment
left out, fog, grass or water off), as set before joining too
(`cheats_network_client_enforce`); its camera stays the player's own
(no flying or following camera). The host keeps its own.

The host also finds a client whose game runs faster than time (a speed
hack, which speeds up the machine's own clock, so that nothing on it can
tell): every two seconds it measures how fast the ticks the client's
messages are stamped with go by against its own clock's time, and how far
ahead of its own time they are. A client's clock starts at the host's and
only ever jumps forward to it when behind, so an honest one is never ahead
while going faster (one that caught up, or a host that stalled, is one or
the other, not both). One more than a tenth faster and half a second ahead
has its players' predictions refused at once (the host's copies go as its
own ticks have them). After ten seconds of it, if a message that came over
its connection's stream was that far ahead too, it is dropped, every
machine is told who, in red on its console and in its `debug.txt`, and its
address is kept out of the host's games while the host runs. A datagram is
known to be the machine's only by the address it came from, which another
machine can send one as, so on its datagrams alone it is not dropped: its
players' predictions stay refused while it goes on, and it is logged once,
as unverified (dropped as above if its stream says so later)
(`distributed_note_client_clock`, `network_game_server_kick_machine`,
`_distributed_message_notice`). The host also adds a line to
`cheaters.txt` beside its `debug.txt`: when, the player's address (an
internet play peer's real one), their Discord user and their players'
names, and why (an unverified one says so). The Discord user is marked
`(self-reported)` there and in `bans.txt`. A client tells the host its Discord user as the Discord
client signed in on its machine says (its id and name, none without one:
not running, or internet play off), once it is in the game and again
when it changes; it says what it likes, so the host keeps of it only digits
in the id and letters, digits, "_", "." and "-" in the name, 23 and 39 of
them at most (`p2p_discord_sanitize`), and names it as the player said.
A joining machine tells the host its hardware id: a keyed hash (HMAC-SHA-256,
16 bytes as hex) of what its machine is known by (Windows' SMBIOS UUID, else
its MachineGuid; Linux's `/etc/machine-id`; Android's `ANDROID_ID`, which the
launcher writes to `hardware_id.txt`: `p2p_hardware_id`), kept by the host
as hex only. A player dropped for cheating (on its stream's word, as
above), and one the host bans with the
console's `ban <player name>` (Tab completes the name; the host's alone), is
added to `bans.txt` beside `debug.txt` (a line each, as in `cheaters.txt`,
with `ip=` and `hwid=`): the host refuses a machine joining whose address or
hardware id is in it (a line taken out unbans). Both are as the player's
machine tells them: anyone with administrator or root access can change
them, and players behind one address share it. The console's `kick <player
name>` drops a player as `ban` does (every machine told), but adds no line
and keeps no address out: the player may join again at once. Besides
`bans.txt`, the host keeps out a machine dropped for cheating or banned for
ever while it runs, by its address as it joined (an internet play peer's
stand-in), its real address and its hardware id, any of them; a dedicated
server's ban for a while (`ban <player> 30m`) is left to its `bans.txt`
line, whose `until=` ends it, and its `unban` lets the machine in again by
the line's address or hardware id (`network_game_server_port_forget_kicked_address`).

The players' votes to kick a player and to end the game
(`network_votekick.c`): a client sends `_distributed_message_votekick` (the
vote's kind, the player's absolute index for a kick, and yes or no)
reliably, and the host takes it only from the client's stream. The host
alone counts:
one vote per real address (an internet play peer's endpoint, not its
stand-in), and votes again deduplicated by hardware id, which only ever
removes votes (the players who may vote are counted by address alone, so a
copied hardware id cannot lower the votes needed). A vote needs yes votes
scaled with the players who may vote, the target counted (more than half
up to 8, then 40% and at least 5 up to 32, then 30% and at least 13), or a
fixed percentage of them (`network.votekick_threshold` `static`,
`network.votekick_percent`), at least two, and more yes than no; one the no votes have made lost ends at
once;
starting one needs `network.votekick_minutes` played on this host in its
own ticks, voting two minutes; one vote runs at a time, and a failed vote
cools its starter down and protects its target. A vote that passes kicks as
the `kick` command does, and the host refuses the address and hardware id
for `network.votekick_ban_minutes`. The host sends each client
`_distributed_message_votekick_status` every second while a vote runs (and
as it changes: its kind, yes, no and needed), for the vote's box at the
left of the HUD (F1 votes yes, F2 no) and the scoreboard, whose right click
frees the mouse to pick a player; the host's own scoreboard adds Kick and
Ban. A vote to end the game ("!endgame" in the chat, `network.vote_end_game`)
counts as a kick's with no target, from 30 seconds played and not in a
game's first minute, and ends the game as `sv_end_game` does.

The text chat (`network_chat.c`): a client sends its player's line to the
host (`_distributed_message_chat_up`, reliably), which takes it only from
the client's own stream and from one of that machine's players, at most 5
lines at once and one each 1.5 seconds after (more are dropped; a machine
gone from the game has its count let go, so one new to its slot starts
afresh), its text cut to 63 characters of printable ASCII but `|` (which
the game's text drawing takes for the start of a code; it becomes `?`, on
the host and again on each client). The host alone sends the line on
(`_distributed_message_chat_down`, reliably, with the sender's absolute
index and its channel, the line's flags): to every client, a team's line
only to the machines with a player of the sender's team (to every one in a
game without teams), and a vehicle's line only to those with a player
riding in the sender's vehicle (the outermost object each unit is a child
of, so any map's vehicles), as the host has them; the sender's machine gets
its own line whatever it is. The host shows it itself if one of its own
players hears it or said it. A client shows only
the host's lines, its own too, so every machine shows them in one order,
and takes them (and the chat settings below) only over the host's
connection's stream: a datagram from the host's address could be anyone's.
Each local player's view shows the lines that player hears.
A person on the machine running the game has their name drawn white, on
that machine and on each client. The host marks the line (the next flag
after the notice). Other names keep their team's colors, and the words
stay pale blue. The line is still 68 bytes. A client whose host does not
mark the line still draws white for a person on the machine the chat
settings name, or on the first machine.
The host's own `/mute <player> [minutes]` and `/unmute <player>` are
commands, not lines: it keeps a mute by the player's name (in ASCII,
matched in either case, as the host keeps names apart) and its minutes by
its own clock, drops the lines of a muted player's machine
when they come up, and sends that machine alone a notice of the minutes
left; it tells every machine of each mute and unmute. A notice is a down
line with `_chat_line_notice_bit` set, its text alone (no sender). A
client's `/mute` is never sent, and the host takes the text of a line that
came up as a line, never a command. The host's scoreboard Mute for everyone
is the same mute (20 minutes), and the host relays none of a muted player's
machine's voice either (`network_voice.c`'s `voice_host_relay`, by the
names of the network game's players, which the lobby has too). A
machine's own scoreboard mute stays its own: it plays none of that
machine's voice and shows none of its players' lines, and tells no one.
The host's chat settings (`network.text_chat`: on, links allowed, or off;
always off while a player's game is listed in the server browser,
`p2p_hosting_listed`, but a dedicated server's own, `chat_listing_allows`)
go to every client in
`_distributed_message_chat_config` (one byte of flags), reliably, as they
change and every 3 seconds. A client without them (from a host without the
chat, or before they come) has the chat off, and so does a client in a
public game it joined from the server browser, whatever its host sends (a
changed host could turn the chat on): while the tunnel reaches the host of
the listing it joined (`p2p_join_listing`, `p2p_joined_listed_host`; any
other invite joined since forgets it), it takes no chat line or notice and
sends none, unless the listing was a dedicated server's that a master
server lists (`p2p_joined_listed_dedicated`: the master's word, from an
approved operator's key; a listing's own dedicated flag, which any host can
set, does not count), whose settings it follows. A client refuses a line the
settings refuse before sending it, and the host refuses it again whatever
the client knew, telling its machine why: a changed build gets no further.
Links are found in the line's text (`chat_text_has_link`): a scheme, `www.`,
an address of four numbers, or a name and a common top-level domain, its
dots also written `(dot)`, `[dot]`, `{dot}`, `(.)`, `[.]` or ` dot `, with
spaces about them or not, or a dot with spaces both sides; a domain that is
a word as well (`me`, `to`, `is`) counts only with a path, a port or another
domain after it (`chat_link_domain`).
Builds without the chat drop the three kinds as unknown, so the network
version is unchanged.

Voice chat (`network_voice.c`, the sound in `port/linux/src/voice_audio.c`)
travels as distributed messages too, but in the lobby as well as in a game:
`network_voice_handles_message` takes them before the netcode's checks that
a game is running (`network_distributed_handle_message`, and the host's
stream and datagram paths in `network_server_message_handler.c`). A client
sends its 20 ms Opus frames to the host unreliably
(`_distributed_message_voice_up`), each with the key the host gave it over
its stream (`_distributed_message_voice_config`, with the host's settings,
again every ten seconds and when they change), so a datagram sent as from
another machine is dropped; an empty one every two seconds says it hears
voices. The host checks the key, the frame's size against its quality, a
rate of 50 frames a second (a burst of 10), and at most 8 machines talking,
then sends the frame to each machine that may hear it
(`_distributed_message_voice_down`, with that machine's key) by its
`network.voice_mode`, `network.voice_lobby` and `network.voice_proximity`,
or plays it itself. The host never decodes a frame it only relays, and
never sends a machine its own. A listener sets the gain and pan of a voice
heard for being near by its own view of the speaker's unit. Mutes are each
machine's own. Builds without voice chat never say they hear voices, so the
host sends them none, and drop the kinds as unknown.
A speed hack of less than a tenth is let be: the host's bounds on how far
and how fast a client's player moves and fires hold it to the host's time
anyway.

The host's AIM ASSIST option in Player Options defaults to ALLOWED.
BLOCK M+KB overrides optional mouse magnetism on every machine, including
the host. Aiming with the controller or touch retains its existing behavior;
the player's saved preference and the bullets' autoaim are unchanged.
`network_aim_assist.c` sends the policy reliably on client ready and on
change. Clients take it only from the host and default to blocked until it
arrives. This adds distributed message 90; network version 32 requires
matching builds so an older client cannot silently ignore the rule.

## Joining a game in progress

A game stays open when it starts (on the Xbox it closed, since every
machine had to simulate it from its first tick), and the game list shows
it. A machine that joins it is accepted as in the
pregame, and its players are added as the game adds a player in game:
every machine in the game spawns them, told by the game's own
`_message_server_add_player_ingame`. Then the host sends that machine alone
the game's settings and its start, with the host's game time (the start
message carries 16 bits of it); the machine loads the game, sets its clock
to that time and the ticks it spent loading, and takes the rest of the time
from the first game update if it is ahead (so that the game's timers read
as the host's). It takes up the host's count of updates where it is. When it has loaded, the distributed netcode
gives it the host's objects (network_objects.c), every player's statistics
and the game type's state, and it plays on as any other client.

The netcode names a player by its datum's index, which must be the same on
every machine, the one that joined too. That machine has not the players
who left (their datums stay until the game ends), nor the order in which
the others added players. So each player's datum is
its slot in the host's player list: every machine makes it there, and the
host gives a player added to the game in progress a slot whose datum is
free (`network_game_manager.c`). A player added to the game in progress
also gets its team and the game type's data, as the players at the start
do (in free for all, a team of its own).

The game is closed to joins while its machines load and after it ends.
A machine must join within 10 seconds of connecting; in a game, the host
drops a machine it has heard nothing from for 15 seconds
(`network.client_timeout_seconds`), and one that
joined the game in progress and has not loaded in two minutes.

Until it has loaded, the machine hears none of the game's messages (which a
machine in the pregame refuses, and which the others no longer need), only
a pregame keep-alive every five seconds from the host
(`network_server_manager.c`, `network_server_message_handler.c`).

## Dedicated servers

A dedicated server (`halo --server`: `docs/dedicated_server.md`) is a host
as any other, with these differences, all on the host's side:

- **No player of its own.** The host machine is in the game as machine 0
  but has no player, so its lobby counts the joined machines' players only
  (`network_server_manager.c`'s `server_ok_to_countdown`, which asks
  `server_director_ok_to_countdown` for `server.min_players` and
  `server.team_start`), and a game with one team's players may start (the
  next players to join balance the teams). The host waits for no player of
  its own to precache, and a client's player that quits the game does not
  end the host's (`network_client_manager.c`: only a local player's quit
  leaves).
- **No ticks of its own player.** The host's own action queue may hold no
  action (`player_queues_new.c`), as a game with no local player has none.
- **The director.** `port/linux/game/server_director.c` drives the lobby as
  a player at the pregame screen would: it sets the map and gametype of the
  rotation's entry, starts the countdown once the server's own rule allows
  it (`network_game_server_port_ok_to_countdown`, the rule the server checks
  each tick, then `network_game_server_port_countdown_begin`), and after the postgame
  goes back to the lobby with the next entry. The clients see a lobby, a
  game and a postgame as with any host.
- **Shutdown.** On SIGTERM, a game in progress goes to its postgame, and
  the host tells every machine it goes (`network_game_server_graceful_shutdown`)
  before it exits, so that the clients show the scores rather than a lost
  connection.
- **The flags.** Since version 39 a dedicated server's LAN advertisement
  carries `HALO_PORT_ADVERTISED_DEDICATED_FLAG` and its internet listing the
  dedicated flag (32); a listing published to the master servers is the
  same signed listing (`docs/master_protocol.md`), and the master's probe
  is answered on the tunnel's socket (`p2p.c`, `server_master_probe`).
- **The timeout.** `network.client_timeout_seconds` (15 seconds by
  default) sets how long a host waits for a silent machine, and two more
  waits with it: how long a stream's writes may stall before its connection
  is dropped (`network_connection.c`), on every machine, clients too; and
  how long a host in a game rides out its network going (an address
  renewed, a wifi drop: `network_game_server_network_lost`). So it is not
  the host's alone, and not a dedicated server's only: each machine reads
  its own setting, and nothing on the wire changes (the machines need not
  agree on it).

## Map sharing

A client may join a game on a Custom Edition map it does not have
(`port/linux/game/map_share.c`, `map_share_files.c`). A game's level name is
the map's own (`custom_maps\hugeass`), which every screen and the server
browser show, and the game record's map version is the first 32 bits of the
map's hash (`map_share_hash_version`), which says which version of the map it
is: a client plays its file of that name and version, so two versions never
meet. A file's hash is BLAKE2b's (32 bytes) of a label, its size and the
BLAKE2b hashes of its pieces (256 KiB each); a map is hashed when it is
needed (a host's game is put on it, a client's settings name its name) and
kept for the run. No map is renamed. A map downloaded beside another version
of its name is named for its hash (`hugeass_0e2b3ee4.map`), and the menus
show it as `Hugeass (2)`; a map an earlier build named so, with no map of
its name alone beside it, has its name given back as the map lists find it.

- **Asking.** A client whose settings name a map it lacks, or lacks of
  their version (`cache_files_map_present`), or one whose resource maps it
  lacks, asks the host over its connection
  (`_distributed_message_map_share_request`: the level, and which of the
  map, bitmaps.map, sounds.map and loc.map it lacks) instead of leaving, if
  `network.map_download` lets it. Its players wait to join until the host's settings
  have named the map, and for a game in progress until it is downloaded; in
  the lobby they join at once. A host that starts while a machine is still
  downloading starts the others without it: the machine's players are held
  out of the game, and so is any player it asks for meanwhile (a machine
  whose download began in the lobby does not know the game has started),
  and the machine joins the game in progress once the map is there. Not
  told to load, it is neither waited for nor timed out while the others
  load, even if the map is there before they have loaded. A
  client told to start before its download is done (by a host of an earlier
  build) puts the start off until the map is there, rather than loading a
  map it does not have yet (the damaged disc error, which stops the game).
- **The offer.** A host that shares maps (`network.map_share`) hashes the
  map, and the resource maps the map needs (its tag index says which: a tag
  whose entry's word at 0x18 is set is held whole by bitmaps.map for a
  bitmap, sounds.map for a sound, loc.map for a font, string list or HUD
  message text) that the client lacks, on a thread of their own, and
  answers with each file's size and hash (`_offer`, or why it does not:
  sharing off, or not the game's map), and which of its other machines may
  have them (`_sources`: an address on a LAN, internet play's key hash on the
  internet), again every ten seconds. A map that is not of the game's version is
  not offered. The client checks that the map is of the game's version, and
  that no file is larger than the cache loader takes, and refuses a map
  name Windows takes for a device (`CON`, `NUL`, `COM1` and the like,
  before the first dot). It connects to a LAN machine the host names only
  at an address of the host's kind (a private network's if the host's is,
  loopback only if the host's is; never 0.0.0.0, multicast or internet
  play's), so that a host cannot send its players anywhere.
- **The swarm.** The client connects to the host and up to eight of the
  others on port 5153 (of two downloading, the one of the lower machine
  connects). Each end says which files it has, whole or by piece, and tells
  the other of each piece it gets; the client asks for the rarest pieces
  first (from a random place, so that clients ask for different pieces),
  eight at a time of the host (as many as every version sends at once: it
  refuses more, so this needs no network version; two at a time held a
  download from it to about 15 MB/s, a piece coming a round trip, about two
  frames, after it is asked for), two of another player (who sends 10 MB/s
  at most, so that a slow one holds few of the last pieces), thirty-two in
  all, and the last few again of
  whoever is quicker. The pieces' hashes come from the host (from the
  others it named after ten seconds without them, and from the connections
  other machines made after thirty), all of them within a minute, checked
  against the file's hash; a frame of none is refused. Each piece is
  checked against its own as it arrives, and an end that sends a wrong one
  is not asked again. A machine
  sends of the game's files (its map, the resource maps the map needs, a
  download's pieces) no faster than `network.map_share_upload` (the host,
  1024 KB/s) or `network.map_seed_upload` (a player, 256 KB/s), round all
  its connections a 16 KiB block at a time, so that the game's own traffic
  keeps the rest of the upload; `network.map_seed = false` sends none. A
  player that has downloaded the map sends it on at once.
- **Connections.** A machine takes up to 24 connections, of which 15 others
  made (the rest are kept for its own download's), four from one address,
  eight at once that have not said hello (the oldest makes way); a
  connection says hello within ten seconds, and one that sends no whole
  frame for 30 seconds is closed. The host takes connections only from its
  game's machines' addresses.
- **Internet play.** The players of an internet game reach its host alone.
  A player that shares answers the holders of the invite it joined with as
  its host does (`p2p_set_answering_players`), and a downloading client asks
  another player, by the key hash the host told, for a session as it asked
  the host (`p2p_reach_player`: up to eight at once, alongside the request to
  the host, in one topic of the invite's token). Its map sharing connection
  then goes through their tunnel, as the game's connections go.
- **Writing.** A download is written to `<file>.part` beside its file in
  custom_maps, each piece at its place as it comes (the file is not made its
  size at once), and renamed when every piece is had. A part file found
  again is checked piece by piece on the hashing thread and taken up where
  it was. A download that stops deletes a part file that holds none of its
  file, and keeps one map's part file at most (the one kept before is
  deleted), so that a host cannot fill a disk with offers.
  The client checks the map again once it has it (`custom_edition_cache_present`),
  and deletes the files the download put in custom_maps if it cannot be
  played (or if one of them could not be put there), so that none is taken
  for present; it tells the host its progress every two seconds
  (`_progress`). The host does not drop a machine downloading (and for 20
  seconds after) for adding no players or loading slowly, while what it says
  it has grew by a piece (or to the whole, and no more than the files
  offered) in the last 80 seconds, counted from the first offer of the
  game's map however often, and for whatever level, the machine asks again.
  It takes a machine's requests five seconds apart at most: one that comes
  sooner waits its turn (the last of them kept). A connection forgets which
  pieces the other end has of the download's files when the download ends.
  A download that brings nothing for a minute, or a host that does not
  answer in 30 seconds, leaves the game, saying why.
- The download's progress is drawn at the top of the screen.

## MCC package sharing

MCC supports host-only delivery for eligible local and Workshop packages.
It uses MCC-owned messages **103** (request) and **104** (response),
separate from CE sharing 83-86. Every map family uses network version **44** for automatic sharing and local base dependencies.
The shared settings record includes two 32-byte MCC digests, zero for Xbox/CE:
the gameplay/resource identity and the required-JSON identity. The distributed
header is 12 bytes, including its checkpoint epoch.

- **Permission and admission.** The existing `network.map_share` setting
  enables both CE and MCC sharing. MCC offers only the current validated
  package automatically; no manual approval list is required. Loose maps and
  `_shared` are not packages to offer. An existing `mcc_share.txt` may supply
  optional maker attribution but cannot enable or block sharing. The client requires `network.map_download`
  (existing default on) and explicit acceptance of the size/maker prompt.
  Before acceptance it receives only an offer; it creates no stage and requests
  no file chunks. If either setting is off, the download action is absent.
  Valid Workshop IDs offer a Workshop page; other content requires manual
  installation. A Workshop page alone does not install anything in a browser.
  Manual-install and failed-download guidance leaves the unjoinable session
  before displaying at the main menu. Only its copied text and Workshop origin
  survive that deliberate disconnect; it holds no host slot or transfer alive.
  Dismissal, another session or leaving the menu clears it. Live download
  consent still requires the current host's exact content identity.
- **The game's start.** A machine fetching the package from the host (its
  download prompt, then the transfer) does not hold the game's start, as a
  Custom Edition map's downloader does not: the host leaves it and its players
  out of the start (`network_game_server_hold_map_fetcher_players`) and it
  joins the game in progress once it has the package. A machine still checking
  its own copy does hold the start, until its admission ends: told to load
  before then, it could not.
- **Messages and bounds.** `MANIFEST` responses carry up to 16 file entries per
  page. The full offer has at most 168 entries, each with kind, a 128-byte path
  buffer, size and SHA-256. Exactly one map is required. `CHUNK` requests up to
  3072 bytes; the client window is 16 and the host supports four transfers.
  Reserved fields, counts, lengths, session IDs, offsets and sender roles are
  checked. Only the sender's established reliable stream is accepted; datagrams
  and responses from other peers cannot supply content. Backpressure retries
  preserve pending responses, and manifest/data traffic consumes the shared
  `network.map_share_upload` allowance.
- **Selected files.** `mcc_share_package_build` uses the validated resource
  selection cached by `mcc_validation_resources`; it does not scan arbitrary
  siblings. Only used package `bitmaps.map`/`sounds.map` are eligible, including
  their resolved Workshop fallback paths. Root/shared base resources and
  package copies matching an installed base are omitted. Joiners must provide
  matching local dependencies; the complete gameplay fingerprint still covers
  their contents, and missing or mismatched resources prevent admission. Recognized
  package/campaign/mission JSON and referenced supported PNG previews travel
  with the map; `credits.txt`, `readme.txt`, `license.txt` and a valid bounded
  Workshop sidecar preserve attribution. `ModInfo.json` is parsed metadata,
  not an unchecked credit file. Map-only local packages are supported.
- **Metadata and size checks.** JSON is limited to 256 KiB per file; PNG to
  8 MiB and 2048 by 2048 pixels, using the existing supported decoder subset.
  The metadata planner allows at most 160 entries and 24 MiB; the staging
  envelope independently caps metadata at 32 MiB, each map/resource at 768 MiB
  and the whole offer at 1 GiB. Credit text is capped at 64 KiB. The client
  checks free space for the offer plus 64 MiB before writing. Nested paths are
  allowed only for recognized JSON/PNG; devices, traversal, duplicate names
  and file/directory collisions are refused. Required JSON is reparsed and
  hashed separately from gameplay. Unreferenced files fail validation; invalid
  optional PNGs are discarded before publication, retaining placeholder art.
- **Staging and identity.** Files are exclusively created in an owned
  `mcc_staging/dl_<digest>` directory and checked in order. The staged gameplay
  and required-metadata identities must match the advertised values. Native
  publication atomically moves the complete directory to a separate
  `mcc_maps/dl_<digest>` without replacing any destination. The exact installed
  snapshot is rechecked before successful `DONE` and joining resumes; existing local and Steam
  packages remain untouched. Cleanup is limited to owned staging entries.
  Native path checks refuse symlinks/reparse points and special files. These
  checks address untrusted remote paths, not concurrent filesystem replacement
  by another process with the same local user's authority.
- **Lifecycle.** The offer expires after two minutes; a waiting client sends
  bounded heartbeats. Initial unanswered requests time out after 30 seconds,
  stalled downloads after 60 seconds. Leaving, map/identity changes, disabling
  downloads and host preparation revision changes invalidate stale transfers
  and consent. Requests remain bound to the host's current package. Required
  campaign JSON is delivered upfront; subsequent mission maps/resources are
  obtained before their own admission, under the same host-only policy.
  A validated successful `DONE` releases its transfer slot and grants at most
  15 seconds for the client's following player-add request. That admission
  grace protects against the host's older playerless-machine deadline, but
  does not report content as still missing or hold players out of the game.
  Player admission, peer/session removal, map/identity changes, host opt-out
  or expiry clears it. Cancelled, expired, premature or replayed `DONE`
  messages cannot grant or renew the grace period.

Installed packages use a separate bounded validation hold, even when both
sharing preferences are off. Reliable MCC message **105** binds a client
generation to the exact advertised map name/origin, checksum, gameplay digest
and metadata digest. The host acknowledges before the client begins its
asynchronous file checks. Five-second heartbeats keep a twenty-second liveness
window, but never extend the absolute two-minute validation deadline. Expired
or completed holds cannot be restarted by replaying `BEGIN` on the same
connection and host revision. `READY` grants only the existing fifteen-second
player-add grace; it does not prove that an untrusted client checked its files.
Actual map loading still fingerprints content independently.

Local validation enumerates distinct usable files only for resource types the
map references. An explicit package override stays authoritative; an unreadable
or invalid override reports its file error instead of trying base alternatives.
Each distinct combination still requires the full gameplay SHA-256. An unchanged
failed exact path is not hashed again after discovery rescans; changed files or
required metadata permit a fresh check. These are local work and diagnostic
changes, with no change to message layouts, fingerprint framing or version 44.

After admission, synchronous MCC conversion has a separate bounded progress
contract in `mcc_load_network.c` and `mcc_load_network_game.c`. Reliable message
**106** carries the exact admitted name/origin, checksum and both digests,
the game round, and an increasing work sequence (204 bytes after the unchanged
12-byte distributed header). Both sending and accepted state advancement are
limited to once per five seconds. A host waits for a progressing joined peer
for at most thirty seconds without new progress and at most four minutes from
the original round binding. Replays cannot restart either expired hold. The
client also has the four-minute absolute bound; its existing 75-second
pre-start transport-silence timeout remains unchanged. Progress never marks a
machine loaded, bypasses file identity checks or reuses the missing-map state.

The server's ordinary distributed-data gate requires an already loaded player,
so minimal MCC-only routes in `network_server_message_handler.c` admit messages
105/106 from joined reliable connections before that gate. Their MCC handlers
still check joined-peer state, exact framing, identity and sender roles; datagrams
cannot establish these holds. The small start, timeout and loaded-ack hooks in
the server/client managers connect the MCC state to the existing lifecycle.
Terminal loaded acknowledgement retires the peer, all-loaded clears the host
binding, and the client's first game update clears its binding. Disconnect,
peer departure, changed identity/revision and round changes invalidate stale
state. Xbox/CE retain their established loading and timeout paths.

`mcc_loading` feeds this contract independently of its visual feedback: null
renderers and disabled loading presentation still begin/end the network work
lifecycle. Conversion pulses only write bounded reliable transport messages.
They never idle or dispatch the network, advance simulation, run widgets or
invoke normal frame callbacks while tags are incomplete.

Downloaded-file verification stays within its existing transfer. Manifest
heartbeats keep the connection responsive while the bounded checker runs,
without resetting the eighty-second last-data-progress bound. Publication
promotes SHA, CRC and selected-resource facts only after the complete file
snapshot remains unchanged. Local installed identity is established before
successful `DONE`, avoiding a second full-content pass. Cancellation joins the
owned checker and closes its readers before any stage cleanup. A package
already published survives a subsequent admission failure.

Implementation lives in `mcc_share.c`, `mcc_share_package.c`,
`mcc_share_session.c`, `mcc_share_stage.c` and the MCC metadata helpers.
Managed-copy receipts and the removal UI preserve separate ownership of
downloaded snapshots.
Browser publication uses an owned OPFS journal and commit record instead of a
directory rename; see [browser storage](../../docs/reading/browser_imports.md).
Browser gameplay, downloads and persistence have their own Milestone 14
validation gates; native results do not establish browser compatibility.

## Stages

1. (Done) Decoupled ticks: clients tick on their own clock with local input
   for local players and the latest relayed input for remote ones; the host
   no longer waits for clients; taps are accumulated so a quick button
   press is never lost; out-of-sync checks off. The lockstep netcode is
   gone: the host's per-tick game update carries no actions, and only
   keeps the clients' count of its ticks.
2. (Done) Authority: clients skip damage, deaths, spawns, pickups, item
   spawns, and scoring, and apply the host's state for them
   (`port/linux/game/network_distributed.c`):
   - every tick, every player's unit: which it is, alive or not, the seat
     it rides, shields and health (down, recharging, the damage they show),
     its powerups (camouflage and how long each has left), where it is (a
     client's own player's position is its own, within a tolerance, and the
     host takes it), and when dead who killed it;
   - what a client's players pick up, which the host decides: the client
     shows it (the HUD's message, the sound, a powerup's screen flash);
     only that client is told;
   - every two seconds, each player's ping (unreliable): the round trip
     of its machine's messages to the host and back, as the host smooths
     it, 0 for the host's own players, for the scoreboard;
   - twice a second and with every kill, the players' statistics that
     changed (once a second a few more, round them all, as the message is
     unreliable); when it changes (looked at five times a second, sent at
     most every other look unless the game ended; reliably, so never
     again unchanged), the game type's state (scores, the flags, the balls
     and their carriers, the king's hill, the players' speeds) and whether
     the game is over. A machine that has loaded is sent all of both at
     once. It counts the game type's events (captures, grabs and returns
     of the flags, laps, the balls reset, a ball passed to its carrier's
     killer), and a client announces those it has not had,
     as the host does its own; a client runs the rest of the game type's
     update that only shows the game (waypoints, messages, sounds, a
     carrier's speed).

   The messages are a kind of their own (the game's unused "data" message
   type), unreliable per tick, reliable for what must not be lost.
3. (Done) Object identity (`port/linux/game/network_objects.c`): the
   game's units, vehicles, weapons and equipment are the host's, at the same
   datum index (identifier and all) on every machine, so a message names
   one by its index.
   - The host tells its clients (reliably) of each such object it makes
     (what it is, where, how it looks) and each it deletes; clients make
     and delete theirs to match. A client that has loaded asks for all the
     host's objects and is told when it has them; from then on it deletes
     any such object the host has not told it of, and nothing but the
     host's word deletes the host's. That word has deleted a CTF flag or
     an oddball, which the game itself never deletes (two assertions,
     in `objects.c` and `weapons.c`, stop a build that is not `--release`
     there). Why the host deleted it is not known; the client follows
     the host's word whatever the reason: it takes the flag or ball out
     of its carrier's hands, and the goal its game type shows at it goes
     with it. The two assertions are not made for anything a client
     deletes in `distributed_client_delete`: the host's objects on its
     word, one in the place of an object the host creates, and its own
     the host has not told it of.
   - The objects placed when the map loads are placed alike everywhere:
     the host's word finds a client's already there. Past loading, a
     client's own objects (projectiles, effects: what only it sees) take
     indices from the upper half of the object array, clear of the host's.
   - Ten times a second, what every unit carries (the host's weapons, slot
     for slot, their ammunition, the weapon in hand, the grenades); a
     client moves the same weapon objects in and out of its units, which
     carry no grenades until the host says (not the unit tag's: a player
     spawned with the button held threw one the host's copy had not). A
     change of weapons or grenades goes to every client at once; one of
     ammunition only to the unit's player's machine at once, and to the
     others as often as they are sent that player. A client takes its own
     players' ammunition and grenade counts from the host only once a
     round trip has passed since it last fired, reloaded or threw (what
     the host says before then is from before). How old what a unit
     carries is travels in 16 bits. A client keeps an inventory whose
     unit or weapons have not come yet (the stream can trail the datagram)
     and applies it in full once they have, the newest of a unit's winning;
     one is dropped `EMPTY_INVENTORY_TICKS` after the newest came.
   - Players take the units the host spawns them with, seats are the
     host's (a client's own player's once it has ridden otherwise for
     longer than a round trip), and the CTF flags and oddballs are the same
     objects everywhere.
4. (Done) Corrections: the host sends each client where its moving objects
   are (vehicles, items, bodies) as often as they are near that client's
   nearest player (every tick within 25 world units, every second within
   60, every third within 120, every fourth further off), to every client
   three times over half a second as an object comes to rest (one lost
   would leave a falling body hanging until its turn round all of them),
   and a few of those at rest, round them all. A client takes the host's
   word on whether each is at rest even when its copy is close enough to
   leave where it is. A body's pose goes with it: one lying limp that the
   host moves stops lying limp (the blast that moved the host's reaches
   only the machines that see the body's player), and one the host has at
   rest is on the ground, so it lies limp again where it stops instead of
   keeping the pose of its fall in the air. A client puts its copies there, and the difference is
   drawn fading over a few ticks (`render_interpolation.c`) instead of a
   jump. A vehicle no player drives, a recording's lifepod as well as a
   Pelican, moves toward each new position the host sends at its path's
   speed (the last two positions' difference over the host's ticks between
   them), and goes on along the path for a few ticks if the next is late,
   coming back to the newest if none comes. A
   jump further than a step of the path is left where the host put it. A client drives its own player's vehicle and sends where it is,
   which the host takes within a tolerance, as it does its own player's
   unit, and no further from an anchor (one it took, a newer once a
   second) than the vehicle moves in the client's ticks since, those no
   more than its own since and a little jitter (twice the tag's top speed,
   or as fast as the host's own ticks sent its copy in the last few
   seconds beyond the velocity it took of the client, whichever is more,
   no more than 3 world units a tick, and a tenth more a tick, with the
   blend distance; its velocity no faster, without the tenth, so that the
   client's word does not raise it, so a vehicle falls no faster than
   that and a tick's gravity: a long fall's last moves are the host's; a
   teleporter falls back to the tolerance); a vehicle that does not fly,
   float or stay (not a Banshee, a boat or a turret), in the air, no
   higher above where it left the ground than a thing thrown up as fast
   as it went up then, and a tenth of a world unit a tick, falls to since
   (less the client's round trip, half a second at most, and jitter), with
   2 world units more (as a player on foot); a copy at rest that the
   client's word moves wakes, so that it falls and counts its time in the
   air. The host sends the client its own
   vehicle every third tick, with the client's tick it took the vehicle at
   (`_distributed_object_predicted_bit` and a 16-bit time in the object's
   state); the client compares that with where it had the vehicle at that
   tick, and past 4 world units moves it by the difference, as it does
   its own player's unit.
5. (Done) Hits (`port/linux/game/network_damage.c`):
   - A client deals no damage. What its own players' shots, grenades,
     melee and vehicles hit, it reports to the host (reliably), with the
     host's latest tick it had heard of when it made the report.
   - The host deals a report once it has checked it: from that machine's
     player; damage one of their weapons (a vehicle's a driver's or
     gunner's; now or in the last ten seconds), a grenade the host's own game
     saw them throw in the last ten seconds that has not gone off (each
     throw's explosion is taken once, its other hits and its other damage
     (a frag grenade's shock wave) that tick with it: holding grenades
     deals nothing) or the vehicle they drove (in the
     last ten seconds: its collisions) can deal (its projectiles' impacts
     and detonations, followed through the tags), no harder than it can be
     (all of it, but an airborne melee blow's half again); of the shape the
     game gives that damage: what hits at a point (a projectile's impact,
     or its detonation on what it sticks to, and an explosion) there, its
     origin its epicenter; a melee blow from the striker's head (its
     origin) and body (its epicenter), both within a few world units of
     where the host had the player; a collision from the vehicle (its
     epicenter) within a few world units and the vehicle's size of where
     the host had the vehicle the player drove, the target (where the
     client had it) no further from it than their sizes and the push the
     vehicle gives it; the origin at the target in each (an explosion's and
     a melee blow's within its reach); what hits at a point within its
     reach of where the host had the player (their unit, or the vehicle it
     rode, and its size) since it could have been fired: the projectile's
     range, or its speed for as long as its timer runs, what it sets off
     going off from there, with 6 world units more (the host keeps thirteen
     seconds of where each player was, every third tick, as a rocket flies
     and a grenade outlives its thrower, and looks no further back than
     the client's round trip and a second before the report came; not checked for what the tags do
     not bound: a projectile that sticks, which what it sticks to carries,
     one whose timer starts once it bounces or rests, a weapon's own
     detonation; no line of sight); the target within a few world units of
     where the host had it at the tick the report was made at (a player's
     unit or vehicle: the host keeps a second of where they were) or of
     where it is (more for a fast one; also for one that is no longer a
     player's, a body or a vehicle left);
     and no more reports than the weapon that deals them fires (its rate of
     fire and projectiles a shot, with a margin; an explosion's hits count
     as one, when its damage has a reach, and no object is hit twice by
     one, the explosions of each player's reports told apart on their
     own). A report made more than three seconds ago (a burst of them held
     back while the network was out) is refused. A report is paid for
     before the host looks through its history, so a flood of them costs
     the sender its hits. The damage is dealt as a client's own hit can
     be: only the flags such a hit has, area damage as the game deals that
     damage (the report's only for damage dealt both ways), an explosion's
     direction from its epicenter to the target's centre (where the client
     had it) and its scale no more than its fall off over that distance
     gives (unless it does not fall off), any other's direction one long,
     and the host's multiplier and team, not the report's, its owner the
     player's unit (or the vehicle it rides, or a unit of theirs); a
     report with a number that is not finite, or a node, region or
     material the target does not have, is refused. Its own copies of a
     client's projectiles deal nothing (the report does), but once that
     client has left the game they deal what they hit, as its own do (a
     hit the client reported just before it left may so be dealt twice).
   - The host sends its clients the damage it dealt to units, and a client
     replays what it does besides the harm (which the units' states
     bring): the player's screen flash and shake, the unit's flinch, pain
     sound, knockback and stun, the scope it knocks the player out of, and
     who the HUD shows hit them. A killing blow it replays whole, so the
     body falls as the shot had it and the kill is announced with the
     host's killer (and the killer's score after it, as the host's game
     type has it: the game type's state, which brings the scores, may come
     after the blow), and a telefrag's message (a client does not decide a
     telefrag itself: it sees who blocks a teleporter a latency late); an actor's (a biped no player's, alive until then)
     too, counted by no one there (the host's statistics come as they
     are). A killing blow goes to every client; other damage to
     the machines of the unit's player, its riders and the damage's owner,
     and of the clients sent that player this tick (who can see them); a
     player's screen effects to that player's machine alone, but for a
     weapon's own shake of the player firing it (no one's damage), which
     that player's machine shows itself at once. The killing blow is sent
     with the tick's other damage and once more reliably (a client replays
     one blow of a unit only), so every body falls as the host's did: a
     body the host says is dead (the objects' states say so of an actor's,
     the units' states of a player's) that is still alive half a second on
     is killed with nothing to show.
   - A client's own projectiles respond to what they hit as the game has
     them: the host's shields and health, which the client has, say
     whether the shield or the body took the hit, and how much is left of
     it (the host's own copies of a client's projectiles likewise).

## Transport

What reaches the other machines, and how, decides how the game feels over
a real network as much as the model does (compared with Quake III, Source,
Unity's Netcode for Entities, lightyear, netfox and the Ares source):

- **Datagrams carry only what is theirs.** A datagram is known to be a
  machine's only by the address it came from, which anyone can send one
  as. So a client takes from a datagram only what the host sends in one: its
  game's advertisement, its answer to a ping, and the distributed netcode's
  messages (checked as the host's below); the game's own messages (a player
  added or removed, the game begun or over, its settings), which the host
  sends over its connection, are ignored in a datagram
  (`network_client_message_handler.c`). The host takes a client's hits,
  Discord user and readiness only over its connection too, and every
  machine drops a datagram of the netcode stamped with a tick more than 15
  seconds from the latest it had from that machine (one stamped far ahead
  would have the machine's newer ones taken for stale); a message over the
  connection is taken whatever its tick (`network_distributed.c`).
- **Nothing held back.** The game's connections (the reliable messages:
  objects made and deleted, the game type's state, hits, pickups) send each
  write at once (`TCP_NODELAY`, in `xnet.c` for the game's sockets and in
  `p2p.c` for internet play's): with Nagle's algorithm a small write waited
  for the other end's delayed acknowledgement, up to 200 ms on Windows.
- **Input every tick, unreliably, each tick's buttons several times.** A
  client sends the host its players' input after each tick, with the
  buttons of the three ticks before it, and the host sends every client
  every player's input as its tick ran it, the same way. Each tick's
  buttons are taken once, from whichever message brings them first
  (`player_queues_new.c`), so a press is lost only with four datagrams lost
  in a row, and nothing waits for a lost one to be sent again (the game's
  own per-tick update, reliable and so held up by any loss, now carries no
  input). The host's clock still reaches the clients in it, and the game's
  own client update, whose input the host no longer takes, goes ten times
  a second instead of sixty; the host takes its own players' input at
  each of its ticks.
- **As few datagrams a tick as they fill.** The unreliable messages of a
  tick to a machine go together (`_distributed_message_batch`), saving each
  one's headers (internet play's tunnel adds 36 bytes to every datagram: 15
  of its header, 16 of its tag and 5 of its own inside), each message split
  between two where one is full, so none goes out part empty; the machines'
  datagrams go in a turn that starts one further each tick.
- **Stamped with their tick.** Every message carries the sender's tick
  (its header's game time); an unreliable one (the units, the input, the
  objects, the statistics, what units carry, the damage) that arrives after
  a newer of its kind is dropped, so a late datagram never puts anything
  back.
- **Taken at the tick.** The host takes a client's players' and vehicles'
  positions (the latest of each) at its next tick, not as each arrives.
- **Fewer bytes.** Vectors travel in 16 bits a part, shields and health in
  16 bits; a unit's state goes without the parts that are their defaults
  (the vehicle and seat of one that rides nothing, the position of one that
  rides, its up when straight up, the damage its shields and body show,
  powerups and camouflage when it has none, the tick its own client
  predicted it at to any other client), 35 bytes for a player on foot
  instead of 64; a player's input goes with its tick's update once a
  message, and the buttons of the three ticks before only where they
  differ from the tick after (14 to 20 bytes instead of 24); what a unit
  carries is sent when it changes (and once a second); the moving objects
  as often as they are near the client's players, the objects at rest
  round all of them, four a tick; the players' statistics when
  they change (with every kill, twice a second), with sixteen more
  players' once a second round them all; the game type's state when it
  changes, at most two and a half times a second.
- **The players each client needs, when it needs them.** The host sends a
  client every player's unit and input every tick when they are within 25
  world units of the client's own players, every second tick within 60,
  every third within 120, every fourth further off, and every sixth when no
  cluster of the client's players' can see theirs (the map's potentially
  visible set, which errs on the side of seeing: Halo's are coarse, and
  most of a map sees most of it) or they are dead. A client whose players
  are dead is sent the others as from where its players last were alive.
  None is ever left out, but a player who has left the game, whose unit
  goes only when it changes, and a dead player's input, which drives
  nothing. Whatever changes what a client sees goes at once:
  - a player's death, spawn or seat, and a player coming into sight;
  - a player's input every tick while their buttons or weapon choices
    change (the ticks it carries), so no jump, grenade or melee of theirs
    is missed, even out of sight;
  - a player a client's player aims at within 35 degrees at least every
    second tick, and within 20 degrees through a scope every tick, so a
    sniper sees a far player move as smoothly as a near one (as Ares
    raises the priority of what a player zooms onto).

  A client's own players' units go to it every tick, their input never (it
  has its own).
- **The round trip.** A client's input messages tell the host the latest
  host tick the client has had, which gives the host each client's round
  trip (smoothed as TCP smooths its own, once a message). The host keeps a
  second (32 ticks) of where players' units and vehicles were, and checks a
  client's hit against where the target was from the host tick the client
  had last heard of when it made the report (three ticks more), instead of
  against where it is now with a wide margin: however long the report
  took to come (the reliable channel's resending holds it up), that is
  when the shooter saw the target (as far back as the host keeps). A
  report made more than three seconds ago is refused.
- **The latency meter** (`latency_meter.c`, `display.latency_meter`) shows
  the same round trips, timed in milliseconds by when each machine sent the
  tick they come back naming (the last 64 ticks' send times kept), with
  nothing more sent: the host each client's, by the host tick its input
  names; a client its own, by its tick the host's state of its player (or,
  driving, its vehicle) says the host has it at. Each is the network both
  ways plus each machine's wait for its next tick, so a LAN reads 15 to 35
  ms. A client that has heard nothing from the host for two seconds shows
  "Connection problem"; the host logs a client silent as long. With the
  meter on, the scoreboard shows these where the machine has them (the
  host every client's, a client its own players'), and the host's measure
  in ticks, which every machine is sent, for the others.

## Testing

`debug.network_test` (`port/linux/game/network_test.c`) hosts or joins a
game without the menus (in a team game the joining player takes the other
team; `local:<map>` plays a local split screen game instead, which the
bots of `bots.count` join), and `debug.test_input` plays controller 1 with a scripted bot (or,
`look:<seed>`, one that stands still, only turning and looking up and
down); each machine logs every player's position, health and shields,
where they aim and face, their animation state and how hard they move,
weapons, grenades, score, kills and deaths every second, with the objects
made and removed and the hits reported, dealt, rejected and replayed, so
two machines' views of one game can be compared. `debug.network_test_kill`,
`debug.network_test_shoot`, `debug.network_test_vehicle` and
`debug.network_test_pickup` script kills, hits, a vehicle ride and a weapon
swap the bots' wandering does not reach (`debug.network_test_pickup_weapon`
picks the weapon: the first whose tag name has it in it, as "sniper"),
`debug.network_test_chat` has each machine's player say a chat line to
everyone, one to the team and one to the vehicle (the host seats the last
two players in one first), then the host mutes the last player for a
minute, each says a line, the host unmutes them and each says another,
each says a line with a link (a client's sent past its own check, for the
host to refuse), the host turns the text chat off (and on again) as each
says one more, the host mutes the last player for everyone (as its
scoreboard does) around a line each, and the last player's machine mutes
the host's for itself (as a client's scoreboard does) before another,
which every machine's `debug.txt` records as
it shows them (the notices as `chat notice:`), and `debug.network_test_score`
shortens the game, to test the next (`host:<map>:<variant>,<variant>...`
plays the variants in turn, the next once a game is over, as the host's
button on the scores does). `debug.network_test_public` lists the host's
game in the server browser, and `browse` joins the one listed there with the
most players (not co-op, if it can), as the server browser does
(port/web/tests/internet.mjs plays the native build and browsers so).
`debug.network_latency` and
`debug.network_loss` hold back what a machine receives and drop some of its
datagrams, to test as over the internet. `debug.network_corrupt` damages
that share of the datagrams a machine receives at random (bytes changed,
cut short, stretched to a full datagram, or replaced throughout: `xnet.c`),
and `debug.network_corrupt_stream` that share of its reads of connections
(a damaged connection is closed, so a little goes a long way), from
`debug.network_corrupt_after` seconds after the start (what a host sends its
own client over the loopback is damaged too, so the game is set up and
started first), to test that nothing another machine sends can crash the
game: a host and a client with a third of their datagrams damaged must play
on, their logs noting what they refused.

With `log`, `debug.network_test` hosts and joins nothing: it logs the game
one machine plays (a `map_name` in `init.txt` plays one without the menus),
as above. `debug.network_test_cues` has the host, or that machine, run
console lines at their ticks, from the first tick every player is in.

The host logs to `debug.txt` when a player on another machine presses the
action button where the host has nothing for them to pick up, with where it
has them and the nearest item: a client that sees a pickup the host does
not.

### Comparing two builds

`tools/net_compare.py` checks that two builds play a network game in step:
a build with a change to the simulation, the compiler or the maths against
one without. It plays them together on the loopback, build A
hosting and B joining (and, with `--swap`, the other way round), on each
map given, each copy's player a seeded bot (`debug.test_input`
`bot:<seed>`, `--events` adds `debug.network_test_shoot` and
`debug.network_test_kill`), headless (`debug.null_renderer`, with no
display; or with `--window` a hidden window, on the display the tool was
started with, `DISPLAY` or `WAYLAND_DISPLAY`, or `--display`'s: a virtual
one, Xvfb, rather than a desktop), and compares the two machines' logs:

```sh
python tools/net_compare.py --a build/linux/halo --b <other checkout>/build/linux/halo \
    --data <folder holding maps/> --maps bloodgulch,beavercreek,hangemhigh --swap
```

It needs the retail maps, so it runs on a developer's machine or a
self-hosted runner, not in the public CI; run it before merging a change to
a build's compiler, maths or the simulation itself. A build
is its executable: a build's (`build/linux/halo`) or a release's. Each copy
gets its own folder (an executable, `config.toml` written in the copy's
folder, a data root linking the maps, and saves) in `build/net_compare/`,
removed when every game passes. It prints PASS, FAIL (with the first
divergent player, tick and field) or ERROR (a copy that did not start, join or log) for each game and
a line of figures; a copy's log and the end of its `debug.txt` are kept for
each game. It exits 0 when every game passes, 1 when any fails, even if
another was an error (a divergence is the finding; an error only a gap in
it), else 2 when any was an error. Each copy runs in a process group of its
own, which the tool ends however it ends: stopped with Ctrl-C, SIGTERM or
SIGHUP, it stops the copies and exits 128 plus the signal's number, and
killed outright, the kernel ends the copies (`PR_SET_PDEATHSIG`). `--compare <host log>
<client log>` judges two logs already written, and
`--a-wrapper`/`--b-wrapper` run a build under another program (a tracer, a
debugger). Both builds
must log the drift (below): one built before it is an error.

The host decides and a client predicts its own player, so the comparison
is not lockstep's bit for bit, but of what the netcode allows:

- **Each second's state.** For every player, what the host logged and what
  the client logged at the same tick (the machines' clocks agree: the
  ticks match). Positions within 1 world unit: a remote player's word
  comes a tick or so late, a client's prediction reaches the host as late,
  and a player runs about 0.08 world units a tick, so identical builds are
  0.04 apart on average and 0.24 at most, and a client puts its copy of a
  remote player where the host has it past `REMOTE_BLEND_DISTANCE` (1
  world unit); out for 2 seconds in a row fails. Aim within 15 degrees,
  body and shield within 0.15, and alive or dead, riding, grenades,
  weapons, score, kills, deaths and team the same: out for 3 seconds in a
  row fails, as the host's word on damage, a death, a pickup or a score is
  a round trip, or a death blow's wait, late, and a player thrown by a
  blast aims as no input says for a second or two. Rounds within 3: out
  for 6 seconds fails, as a client keeps its own player's ammunition
  while it fires and takes the host's (a pickup's) only once it stops
  (`network_objects.c`).
- **Drift.** Those agree between builds that simulate differently too,
  as the netcode corrects every copy toward the host's. What does not is
  how the host moves a client's player: each tick the host moves it on
  from where it took the client's last prediction, with the client's
  input, and then hears the client's prediction of the same tick. Where
  the host took the last a tick ago, it was of the client's tick before,
  and the host has the input of the client's tick, both machines moved
  the player one tick with the same input from much the same place, and
  `distributed_take_prediction` (network_distributed.c) notes how far
  apart the two moves were, and whether further than
  `DIVERGENCE_APART_DISTANCE` (0.0001 world units, far above the last
  bits' rounding). (The moves, not where they end: the host puts its copy
  only halfway to a prediction a little off, so where it ends would be as
  far off for ticks after anything that set them apart.) Identical builds
  move it alike to the last bit but where the player touched something
  the two machines have in slightly different places (the other player,
  a grenade's blast, an item knocked about). A build that moves a player
  otherwise is apart in most of the ticks the player moves. More than 60%
  of the ticks apart fails.

The per-second line each machine logs (`network_test_log_players`) is:

```
network test: tick <tick> player <index>: (<x> <y> <z>) h<body>/<shield>[ riding][ camo] g<grenades>/<grenades> w
  a<aim yaw>/<aim pitch> f<facing yaw> l<look yaw>/<look pitch> as<aiming screen>/<looking screen> st<animation state>
  thr<throttle>[ <weapon tag>:<rounds>...] s<score> k<kills> d<deaths> f<team kills> t<team> m<machine>
  [player <index>: dead s<score> k<kills> d<deaths> f<team kills> t<team> m<machine>] ...
  | items <on the ground> (+<created> -<deleted> !<failed> x<removed>) | <playing|game over> to <score to win>
  | sent <n> received <n> corrected <n> | hits <n> dealt <n> rejected <n> replayed <n>
  | local <player> camera (<x> <y> <z>) respawn <ticks> | drift <ticks> <apart> <mean> <largest>
```

(on one line; a riding player's position is the vehicle's, angles are in
degrees, a weapon's tag is its tag index's low 16 bits in hexadecimal, and
its rounds are those loaded and in reserve). `drift` is the host's: how
many ticks of the second it compared a client's move of its player with
its own (above), how many of them were apart, and the mean and largest
distance apart (world units); a client's is 0. net_compare.py reads every
field but the items, the hits and the camera, and compares the last game a
log has.

Calibration (on one 24-thread computer, with up to four games played at
once, 60 seconds each, both ways round, on bloodgulch, beavercreek,
hangemhigh, chillout, prisoner, ratrace, longest and carousel):

| Builds | Games | Result | Ticks apart |
| --- | --- | --- | --- |
| The Linux build against itself | 64 | 63 pass (one copy crashed, an error) | 0-13%, up to 42% with the frames uneven |
| The Linux build against itself, `--events` | 12 | 12 pass | 7-36% |
| B's players 0.5% faster | 10 | 10 fail, at the first second compared | 77-88% |
| B's players 2% faster | 6 | 6 fail, at the first second compared | 88-93% |
| B's gravity 0.5% or 2% stronger | 14 | 14 pass | 25-38% |

(B a build of the same sources with `movement_desired` or `global_gravity`
scaled in `biped_update_physics`, a change made for the test.) A change to
how players move on the ground is caught in the first second; one that
changes only the ticks in the air, as gravity does, is within what
identical builds show, and not caught. Positions agree to 0.04 world units
on average, 0.24 at most, in every game, perturbed or not: the netcode
corrects them.

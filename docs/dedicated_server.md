# The dedicated server: reference

`halo --server` runs the game as a host with no window, sound or player of
its own: other machines join it on the LAN, by its invite link or from the
server browser, as they join any host. `port/linux/README.md`, "Dedicated
server", says how to run one; this document is the reference for what it
takes and answers: the command line, the exit codes, the commands, the
HTTP API, the remote console (RCON) over WebSocket, the events, the log and
the files. `docs/master_protocol.md` is the master servers' protocol, which
the server publishes to and the server browser reads.

The code is `port/linux/src/server_*.c` (the platform side: the mode, the
standard input, the log, the rotation's grammar, the HTTP and WebSocket
server, the master servers' publisher) and `port/linux/game/server_*.c`
(the game side: the director that plays one game after another, the
commands, the gametypes, the status). The Android and web builds have no
dedicated server: they compile those files to nothing, but for
`server_json.c` (the JSON reader, which the server browser's list of master
servers uses in every build) and `server_rotation.c` (the rotation's
grammar and the gametype rules), which every build compiles.

## Panels and fleets

A game panel (Pterodactyl, PufferPanel, MCSManager: `deploy/`) runs one
server and reads its standard output; `deploy/systemd` and `deploy/docker`
run servers on one machine. For many servers on many machines, `opence-fleet`
([github.com/OpenCommunityEdition/opence-fleet](https://github.com/OpenCommunityEdition/opence-fleet))
is one control plane over them all: an agent on each machine runs the
servers and drives each through this document's API, as the owner from
the machine itself (`server.web_owner_password`, below). What it relies on
is named in the status's and the RCON's `capabilities`: `owner_web` (the
owner's role over the web, from loopback), `ban_address` (`sv_ban_address`,
`POST /api/bans`), `stop` (`sv_stop`, `POST /api/stop`, the `drain` event),
`event_addresses` (addresses in the player events) and `identity` (the
status's `pid`, `instance`...).

## Command line

| Argument | What it does |
| --- | --- |
| `--server` | Dedicated server mode (as `HALO_SERVER=1` or `server.enabled = true`) |
| `--config <path>` | The settings file; default `config.toml` beside the executable. `HALO_CONFIG_FILE`, when it is set, wins over `--config` |
| `--version` | Prints `OpenCE build-<n> (network version <v>)` and exits 0 |
| `--keygen` | Writes a new publisher key (`server.publisher_key_file`) unless one exists, prints its public key and key id, exits 0; exits 3 when the key cannot be made, read or written |
| `--sign <text>` | Prints the publisher key's Ed25519 signature of the text, in hex (an application's proof of key), exits 0; exits 3 without a text or a usable key |
| `--probe [<invite>]` | A look at a server, for a list site that shows its state: joins nothing and never starts a server (whatever `--server`, `HALO_SERVER` or `server.enabled` say). It finds the invite's game over its tunnel (without an invite, the LAN's first) and prints it as one line of JSON, the game of the invite's host alone (never another game that answers), and exits 0; when nothing answers in 30 seconds it prints `{"ok":false,"error":"no game answered in 30 seconds"}` and exits 1. Internet play is as set (`network.online`, on by default), which an invite needs |
| `--help` | Prints these, exits 0 |

`--version`, `--help`, `--keygen`, `--sign` and `--probe` write no settings: the
settings file is read, never completed or made. `--probe` wins over
`--server`, `HALO_SERVER` and `server.enabled`: a probe never writes to
`config.toml`.

A probe's line:

```json
{"ok":true,"name":"OpenCE Server","map":"bloodgulch","gametype":"ctf","players":5,"max_players":16,
 "teams":true,"in_progress":true,"open":true,"dedicated":true,"network_version":38,"kind":0}
```

`gametype` is the game engine's name (`ctf`, `slayer`, `oddball`, `king`,
`race`, or `none`), `kind` the OpenCE gametype's kind the host sends (0 for
none), `open` whether it takes players now.

The release has a `halo-server` script beside `halo`, which runs
`halo --server` with its arguments.

## Exit codes and signals

| Code | Meaning |
| --- | --- |
| 0 | Stopped: `quit`, `sv_stop` (now, or at the end of its drain), SIGTERM or SIGINT |
| 1 | A fatal error in the game (a halt, a map that failed to load); the last error lines were logged. The process exits within 5 seconds of the error, even when what runs at the exit (a hook, a listing's withdrawal) does not return |
| 2 | No usable game data: `maps/ui.map`, or every rotation map, missing or not a cache file; nothing was bound |
| 3 | Bad server configuration: no rotation entry that can be played (and `server.rotation_fallback` off), `server.web_port` past 65535, a web port the server may not bind (a port below 1024 without the right to it) or whose `server.web_bind` is an address the machine does not have, or a web password (admin, moderator or viewer) shorter than 8 characters with the web port bound beyond loopback (on loopback, a warning), or the owner's (`server.web_owner_password`) shorter than 8 wherever it is bound; `--keygen` or `--sign` failing. An entry that cannot be played (a bad gametype file among them) is left out with a warning: the server stops only when none is left |
| 4 | A port in use: the tunnel's UDP port, or the web port (another program listens on it) |
| 5 | Restart asked for (`sv_restart`, now or at the end of its drain, or an update installed: `server.auto_update = "install"`, the default, installs a new release as soon as a check finds it, on a thread of its own, the server going on meanwhile. Then, of a release of the same network version, it stops with 5 once the server has been empty for 10 minutes; of a new network version (one its players' games no longer join), it says "This server updates after this game" and stops with 5 when the game ends (in the lobby, after 30 seconds' notice), a game going on past `server.update_game_minutes` (30) since the install being ended as `sv_end_game` ends it (`game_ended`'s reason `update`). An install that fails is tried again at the next check: "Updates", below) |
| 128 + n | Stopped by signal n: a second SIGTERM or SIGINT, or the shutdown not done 5 seconds after the first |

SIGTERM and SIGINT stop the server: a game in progress goes to its
postgame, every machine is told, the listings are withdrawn, and it exits 0.
SIGHUP reloads the server's settings, the rotation and the gametype files
(`sv_reload`). SIGUSR1 prints the status on standard output. SIGPIPE and
SIGTTIN are ignored.

## Standard output

One line per event, for the operator and the panel:

```
2026-10-19T18:03:11Z INFO server: ready (tunnel 2302/udp, web 127.0.0.1:8080, listing public)
2026-10-19T18:03:40Z INFO player: Milenko joined (slot 0, machine 1)
2026-10-19T18:04:02Z INFO game: started: CTF on bloodgulch, 2 players
```

The fields are the time (UTC), the level (`INFO`, `WARN`, `ERROR`), the
category (`server`, `lobby`, `game`, `player`, `chat`, `moderation`, `net`,
`master`, `console`, `health`) and the text. `server: ready` is printed once,
when the lobby is open and, for a public server, its listing has been
published (or 10 seconds on, as the brokers are other people's): it is the
line a panel waits for. With `server.log_format = "json"` each line is an
object instead, and the line a panel waits for holds `"msg":"ready (`:

```json
{"time":"2026-10-19T18:03:11Z","level":"info","cat":"server","msg":"ready (tunnel 2302/udp, ...)"}
```

Every line is also kept in the log the web's viewers read (`/api/log`,
`sv_log`, the RCON's `log` events), but for these, which go to standard
output alone: the web password a first start makes, the invite (a
private server's lets anyone in; `/api/status` gives it to admins), and
the owner's console, the commands typed on standard input or read from the
init file and what they answered (a password's command is shown as `(a
password's command)` there too).

A health line comes once a minute: `health: players=3 frame_ms_max=4.1
frame_ms_avg=1.20 cpu=2.1% rss=96MB` (the longest and the mean frame of
the last minute, in milliseconds). Standard error keeps the platform
layer's `halo-linux:` lines, and `debug.txt` everything, as for a player's
game.

## Commands

The same commands come from standard input, `server.init_file` (run once at
the start, a command a line, `#`, `;` and `//` starting comments), the telnet
console (`debug.telnet_console`, loopback), the RCON, `POST /api/command`
and the web page. Each runs on the game's thread between frames. Every
command has a role; a caller may run it if their role is that one or above:
**viewer** < **moderator** < **admin** < **owner** (standard input, the
init file and the telnet console are the owner's: whoever holds the
process; so is a sign-in to the web or the RCON with
`server.web_owner_password` from the machine itself: "Signing in"). When
standard input ends and `server.console_exit_on_eof` is on, its last line
runs, whether or not a line's end ended it, and then the server stops.

A `<player>` is a slot number (`sv_players` lists them) or the start of a
name; a name that begins more than one player's is refused. A `<duration>`
is a number and a unit (`45s`, `30m`, `2h`, `7d`, `1w`), a bare number of
minutes, or `forever`; ten years at most. A word in quotes (`"two words"`)
is one word.

A command's text (a notice, a reason, `sv_set`'s value, a name) is kept to
printable ASCII: control characters (a line's end, a tab) and other bytes
outside it are dropped, as are the quotes around the whole text, and
standard input's and the init file's lines are cleaned so before they run.
The web is stricter where it takes a value as it is: `PUT /api/settings`
refuses a value with a control character (`schema`), and nothing is changed
for that setting. A command's reply is sent whole, however many lines.

Commands that are not the server's (the game's own console commands, the
scripts' globals and functions) need the owner role: from the web or the
RCON they are refused (`not a command of the server's`) but to the owner's
session. The server's
commands come first, so `kick` and `ban` are always the server's below, not
the game's console commands of those names.

| Command | Role | What it does |
| --- | --- | --- |
| `help [command]` | viewer | The commands, or one command's use |
| `sv_status [--json]` | viewer | The server's state, map, gametype, players, uptime, games played, network version, listing and next game |
| `sv_players [--json]` | viewer | Each player's slot, name, team, score, kills, deaths, assists, ping and machine; with the owner role or `server.log_addresses`, their address and hardware id |
| `sv_rotation` (`sv_mapcycle`), `sv_maps`, `sv_gametypes`, `sv_version`, `sv_log [n]` | viewer | Lists: the rotation, the maps there are, the gametypes, the build, the log's last n lines |
| `sv_bans` (`sv_banlist`), `sv_settings` | admin | Lists: the bans of `bans.txt`, the settings `sv_set` changes |
| `sv_masters`, `sv_publisher` | viewer | The master servers and how each answered; the publisher key's id and public key (never its seed) |
| `say <text>` (`sv_say`) | moderator | A notice on every machine's screen |
| `tell <player> <text>` | moderator | A notice on one player's machine |
| `kick <player>` (`sv_kick`) | moderator | Drops the player's machine; it may join again |
| `ban <player> [<duration>] [<reason>]` (`sv_ban`) | moderator (a day at most), admin | Drops the player's machine and bans its address and hardware id, for the duration (else for ever). A moderator must give a duration of a day or less: a ban for ever, or longer, is refused |
| `unban <number\|address\|hardware id>` (`sv_unban`) | admin | Takes a ban out of `bans.txt`; the machine may join again at once, by the LAN or the internet |
| `sv_ban_address <address\|hardware id\|discord:<id>> [<duration>] [<reason>]` | admin | Bans an IPv4 address (`a.b.c.d`), a hardware id (up to 32 hex digits) or a verified Discord account (`discord:` and its id, below), its machine here or not: its line in `bans.txt` as `ban` writes it (the other part `none`), so that `sv_bans`, `unban` and `GET /api/bans` see it, and a machine present of that address or hardware id is dropped as `ban` drops it (`player_left`'s reason `banned`). Answers `ban <n> added` (`n`: its number in `sv_bans`) |
| `sv_vpn` | viewer | The VPN policy, the netlist held (its masters, serial, ranges, age) and the joins refused for it ("VPNs and linked Discord accounts") |
| `sv_vpn_check <address>` | moderator | Whether an IPv4 address is refused as a VPN's, and by what (`netlist`, `vpn_block.txt`, `vpn_allow.txt`) |
| `sv_vpn_fetch` | admin | Fetches the masters' netlists now |
| `mute <player> [<minutes>]`, `unmute <player>` | moderator | The host's mute of a player's chat and voice for everyone |
| `sv_map <map> [<gametype>]`, `sv_map <entry>` | moderator | Plays that game (or a rotation entry, `map:gametype+rule=value`; a co-op one too), below |
| `sv_map_next` (`sv_next`) | moderator | Ends the game without its scores and plays the rotation's next |
| `sv_end_game` | moderator | Ends the game: its scores, then the next |
| `sv_rotation_set <entries>`, `sv_rotation_add <entry>`, `sv_rotation_del <n>`, `sv_rotation_begin [<n>]` (`sv_mapcycle_add`, `_del`, `_begin`) | admin | Changes the rotation (written to `server.rotation`, 4096 characters at most) |
| `sv_set <setting> <value>` | admin | Changes one of the server's settings, now where it can be, and in `config.toml` (below); a setting its environment variable sets is refused. A value is 256 characters at most |
| `sv_name <name>`, `sv_maxplayers <n>`, `sv_password [<password>]`, `sv_public <0\|1>`, `sv_motd <text>` | admin | `sv_set` of `server.name`, `server.max_players`, `server.password`, `server.public`, `server.motd` |
| `sv_reload` | admin | Reads `config.toml`'s server settings, the rotation and the gametype files again (as SIGHUP) |
| `sv_stop [now\|game\|empty] [<notice>]`, `sv_stop cancel` | owner | Stops the server (exit code 0): `now` (the default) as `quit` does; `game` once the game being played is over, `empty` once nobody plays (a drain, below); `cancel` the drain waiting. The notice is said to every machine first |
| `sv_restart [now\|game\|empty] [<notice>]`, `sv_restart cancel` | owner | As `sv_stop`, with exit code 5 (restart me) |
| `quit` (`exit`) | owner | Stops the server (exit code 0) |

`sv_map`'s game is played as soon as it can be, and its answer says when:
`next: <entry> (now)` in the lobby before its start was sent (the lobby
takes the entry, and the rotation's entry it replaced is played after it),
`(now: this game ends)` while a game is played (it ends at once, without
its scores), and `(once this game is over)` once the lobby's start was sent
or while a map loads (it is the next game). `sv_map` of the game being
played plays it again. The rotation goes on after it.

A drain (`sv_stop` or `sv_restart` with `game` or `empty`, `POST
/api/stop`) is a stop that waits: the listings are withdrawn as `sv_public
0` withdraws them (a new invite, the masters told), without the setting
changing; new machines are refused as while a map loads (the game is not
open: the LAN's search shows it closed, a join by the invite is told the
game is not open); then the server stops. With `game` it stops once no
game is played nor about to be: at once in the lobby with nobody in it, or
with no countdown running (fewer players than `server.min_players`);
otherwise once the game played (or about to start) is over and its
postgame shown. With `empty` it stops once nobody is on the server (at once
if nobody is now). `sv_stop cancel` (or `sv_restart cancel`) ends the
drain: the server is listed again as `server.public` says and takes new
players. A `drain` event says each (`after`: `now`, `game`, `empty` or
`cancel`), and the stop that ends a drain has the `shutdown` reason
`drained`. `/api/status`'s `drain` is the drain waiting, or `null`.

A co-op entry (`<level>:coop`) is ended as a game is: by `sv_end_game`,
`sv_map_next`, `sv_map`, `server.empty_seconds` and `server.idle_minutes`.
It has no scores: the server goes back to the lobby at once (no postgame),
and a level won ends it with `game_ended`'s reason `score`.

`sv_set` (and the settings of the web page, `PUT /api/settings`) changes
these settings, which `sv_settings` and `GET /api/settings` list:
`server.name`, `server.max_players`, `server.min_players`,
`server.password`, `server.public`, `server.rotation_fallback`,
`server.lobby_seconds`, `server.postgame_seconds`, `server.empty_seconds`,
`server.idle_minutes`, `server.team_start`, `server.motd`,
`server.frame_rate`, `server.web_password`,
`server.web_moderator_password`, `server.log_format`,
`server.log_addresses`, `server.status_file`, `server.auto_update`,
`server.coop_difficulty`, `server.region`, `server.master_publish`,
`server.chat_filter`, `server.chat_filter_slurs`, `server.chat_filter_swearing`,
`server.chat_filter_insults`,
`server.master_publish_address`, `server.metrics`, `server.vpn_policy`,
`server.vpn_fail`, `server.vpn_check_lan`, `server.link_min_age_days`,
`server.link_one_seat`, `server.update_check_minutes`,
`server.update_game_minutes`, `server.max_machines_per_address`,
`server.afk_kick_minutes`, `server.afk_kick_when_full`,
`server.afk_kick_full_minutes`, `server.coop_bsp_switch`,
`server.coop_bsp_switch_cooldown_seconds`, `server.map_vote`,
`server.map_vote_choices`, `server.map_vote_seconds`, and
`network.text_chat`,
`network.voice_lobby`, `network.voice_mode`, `network.voice_kbps`,
`network.votekick`, `network.vote_end_game`, `network.votekick_threshold`,
`network.votekick_percent`, `network.votekick_minutes`,
`network.votekick_ban_minutes`, `network.map_share`,
`network.map_share_upload`, `network.public_address`,
`network.coop_friendly_fire`, `network.coop_player_collisions`,
`network.coop_ally_friendly_fire`, `network.coop_team_wipe`, `network.shield_effects` and
`network.fire_origins`. Of them,
`server.status_file`, `server.auto_update` and `server.log_addresses` need
the owner role: from the web's admin, `sv_set` of one of them (`POST
/api/command`) answers `403 forbidden`, and `PUT /api/settings` leaves it
out, in its `errors` with the code `forbidden` and the message `<setting>
needs the owner role (the server's console)`; the owner's session changes
them both ways. A web password is empty or
8 characters or more. A value outside a setting's choices or range is
refused (`400 failed` from `sv_set`, `schema` in `PUT /api/settings`'s
`errors`), with `<setting> takes a whole number from <min> to <max>` (or
`from <min>`) or `<setting> is one of <a>|<b>|<c>`; a choice may be given
in either case, and is kept as the table spells it:

| Setting | Choices or range |
| --- | --- |
| `server.max_players` | 2 to 127 |
| `server.min_players` | 1 to 16 |
| `server.lobby_seconds`, `server.postgame_seconds` | 0 to 300 |
| `server.empty_seconds`, `server.idle_minutes` | 0 or more |
| `server.frame_rate` | 30 to 120 |
| `server.team_start` | `one`, `both`, `skip` |
| `server.log_format` | `text`, `json` |
| `server.auto_update` | `off`, `check`, `install` |
| `server.coop_difficulty` | `easy`, `normal`, `hard`, `impossible` |
| `network.text_chat` | `on`, `links`, `off` |
| `server.chat_filter` | `off`, `censor`, `block` |
| `server.vpn_policy` | `off`, `link`, `block` |
| `server.vpn_fail` | `open`, `closed` |
| `server.link_min_age_days` | 0 to 3650 |
| `server.update_check_minutes` | 5 to 1440 |
| `server.update_game_minutes` | 0 to 240 |
| `server.max_machines_per_address` | 0 to 128 |
| `server.afk_kick_minutes` | 0 to 60 |
| `server.afk_kick_full_minutes` | 2 to 60 |
| `server.coop_bsp_switch` | `majority`, `anyone` |
| `server.coop_bsp_switch_cooldown_seconds` | 0 to 300 |
| `server.map_vote_choices` | 2 to 8 |
| `server.map_vote_seconds` | 5 to 120 |
| `server.chat_filter_slurs`, `server.chat_filter_swearing`, `server.chat_filter_insults` | `off`, `severe`, `strong`, `mild` |
| `network.voice_mode` | `off`, `team_proximity`, `team_enemy_proximity`, `team_global`, `team_global_enemy_proximity` |
| `network.voice_kbps` | 8 to 64 |
| `network.votekick_threshold` | `dynamic`, `static` |
| `network.votekick_percent` | 1 to 100 |
| `network.votekick_minutes` | 0 to 60 |
| `network.votekick_ban_minutes` | 1 to 1440 |
| `network.map_share_upload` | 16 to 20480 |
| `network.coop_friendly_fire` | `off`, `on`, `shields_only`, `explosives_only` |
| `network.coop_team_wipe` | `auto`, `revert`, `respawn` (`auto`: a public server, `server.public` with internet play, only respawns the team) |
| `network.shield_effects` | `stock`, `glow`, `light_show`, `overshield`, `glow_overshield`, `light_show_overshield`, `players` |

A dedicated server may also keep words out of its chat and its players'
names (`server.chat_filter`, off by default): the words of streamer mode's
list, by group, each group from the least severity its setting names
(`server.chat_filter_slurs`, `mild` by default: every slur;
`server.chat_filter_swearing` and `server.chat_filter_insults`, `off`).
`censor` shows a line's words as stars to everyone; `block` refuses the
line (its sender is told "Your line was not sent: it has words this server
does not allow") and refuses at the door a machine whose player's name has
them, which an updated game shows as "The host refused: your player name"
(an older one says the game is closed). Each is logged (`chat filter: …`,
`refusing a machine … its name has words the server's filter keeps out`).
Links are `network.text_chat`'s, refused by default. A list of words is
not a moderator: spaced letters and look-alikes get past it.

### Votes, the map vote and players away

The players vote to kick a player (`network.votekick`: the scoreboard's
right-click, or the console's `votekick <name>`) and to end the game
(`network.vote_end_game`: `!endgame` in the chat). While a vote runs every
player sees its box at the left of the screen, and F1 votes yes, F2 no
(`controls.vote_yes`, `controls.vote_no`), or a controller's D-pad left and
right, which then do nothing else (`!yes` and `!no` in the chat do too).
The yes votes needed are `network.votekick_threshold`'s, of the players who
may vote: `dynamic` (the default) scales them, more than half up to 8, then
40% (at least 5) up to 32, then 30% (at least 13); `static` asks
`network.votekick_percent` (50 by default) of them, rounded up, and for a
kick never more than those who can vote yes. At least 2 either way, and
more yes than no; a vote the no votes have made lost ends at once. A vote to end the game may start from 30 seconds played on the
server, not in a game's first minute, and ends it as `sv_end_game` does,
its scores and postgame shown (`game_ended`'s reason `vote`).

`server.map_vote` (off by default) lets the players choose the next game:
as the lobby opens after a game with players in it, the server offers
`server.map_vote_choices` (4) entries of the rotation picked at random (not
the one just played, nor one twice), every player's lobby opens its vote
screen, and after `server.map_vote_seconds` (20), or once every machine has
voted, the entry with the most votes is played next (a tie, or no vote, at
random among them); the rotation goes on after it. `sv_map` (an entry
interposed) and a changed rotation set the vote aside. `map_vote_started`
and `map_vote_ended` events say each.

A player away from the controls (no button pressed or let go, no stick or
trigger held, the view not turned) for `server.afk_kick_minutes` (5; 0:
never) is told a minute before and then kicked (`player_left`'s reason
`afk`, an `afk_kick` event), free to join again at once. Only time in a
game counts: not the lobby, the postgame, a cinematic, nor a player dead.
With `server.afk_kick_when_full` a player is kicked only while the game is
full, after `server.afk_kick_full_minutes` (3).

`server.max_machines_per_address` (0: no limit) refuses a machine whose
address (an internet play peer's real one) has that many in the game
already (`join_refused`'s reason `address_full`); an updated game shows
"The host refused: too many players from your address".

In a co-op game a loading zone (a trigger that moves the team to another
part of the level) switches only once more than half of the living players
who are not away stand in zones to the same part (`server.coop_bsp_switch`
`majority`, the default; `anyone`: the first player in one). The others see
"Waiting for N more players to cross"; a mission script's switch is not
held. After a switch the zones wait `server.coop_bsp_switch_cooldown_seconds`
(15) before another, the players told when they open; each switch is told
to everyone, logged and a `coop_bsp_switch` event names who crossed.

### Updates

`server.auto_update` is `install` by default (the Docker image's, `check`:
its image is updated instead). The server asks GitHub for the latest
release `server.update_check_minutes` (30) after the last check (the first
two minutes after it starts), and when there is a newer one says so (a
`warning` event) and, with `install`, installs it on a thread of its own.
The release's notes tell its network version: one of the same version
restarts the server (exit code 5) once it has been empty for 10 minutes;
one of a new version, which its players' games no longer join, tells the
players "This server updates after this game" and restarts it once the game
ends, in the lobby after 30 seconds' notice, and a game still going on
`server.update_game_minutes` (30; 0: no limit) after the install is ended
as `sv_end_game` ends it, its scores shown.

### VPNs and linked Discord accounts

A dedicated server refuses by default machines that join from the address
of a VPN, a Tor exit or a datacenter, as the master servers' netlist says
(`docs/master_protocol.md`, "The netlist"), and lets one in that shows a
pass of a Discord account linked at a master the server publishes to ("Player
keys, links and passes" there): a player on a VPN links a Discord account
once, and joins. `server.vpn_policy`, live:

| Value | A machine from a listed address |
| --- | --- |
| `link` (the default) | is refused with "This server blocks VPNs: link Discord in Settings, Account to join, or turn your VPN off", unless it shows a pass (which an updated game does at once, by itself, when it holds one) |
| `block` | is refused, pass or not ("This server does not allow VPNs") |
| `off` | joins |

The server fetches the netlist of each master it publishes to (the masters
of `server.masters_file`) every hour, keeps the newest of each on disk
(`netlist-<master key id>.bin` beside `bans.txt`) and checks the union of
them. Until it holds one, `server.vpn_fail` decides: `open` (the default)
lets every machine in, `closed` refuses those from the internet as
`block` would. `server.vpn_list` names a netlist file checked besides
the masters' (a full path, or beside `config.toml`; for a server that
publishes nowhere, or a test), read at the start and by `sv_reload`. Beside
`bans.txt`, `vpn_allow.txt` names ranges never refused and `vpn_block.txt`
ranges refused besides, one a line (`203.0.113.0/24`, `198.51.100.7`, or
`198.51.100.1-198.51.100.9`; `#` starts a comment), read at the start and by
`sv_reload`. Only a machine's real internet address is checked (an
internet play peer's endpoint, `distributed_real_address`): a LAN's
machines are not, unless `server.vpn_check_lan` (for tests) says so.

A pass is taken from the masters the server publishes to alone (with
`server.master_publish` and a publisher key): a server that publishes
nowhere takes none, and a player from a listed address joins it only with
`server.vpn_policy` `off`. With a pass, the server also refuses an account
younger than `server.link_min_age_days` (365 by default: a year, from the
account's id; "This server needs a Discord account a year old to join over
a VPN") and, with `server.link_one_seat` (on by default), a second machine
of an account already in the game ("Your Discord account is already
playing on this server"). A pass that does not check (another master's, out
of date, not this connection's) is refused as "Your Discord link could not
be checked". The machine of a pass is a verified Discord account: its id
and username (`discord_id`, `discord_name`, `discord_verified` in
`/api/players` and `player_joined`, with the addresses: for the owner, or
every role with `server.log_addresses`) replace what
the machine says of itself, and `ban` bans the account too. Each refusal
is logged (`refusing a machine @<address>: vpn …`) and is a `join_refused`
event. An older game is told the game is closed.

A dedicated server's text chat is its `network.text_chat`'s, listed or
not: `on` by default (lines with links refused), `links`, or `off`, live
(`sv_set network.text_chat off`). A game a player hosts has no text chat
while it is listed in the server browser; a dedicated server is the
exception, and the players who join it from the browser's dedicated servers
have the chat as it says, as long as a master server lists it (the
masters' word that it is an approved operator's; a listing's own flag is
not enough, so a dedicated server listed by the brokers alone, before its
operator is approved, has none for them).

The settings `sv_set` does not change (`server.web_viewer_password`,
`server.web_owner_password`, `server.web_port`, `server.web_bind`, `server.web_trusted_proxy`, the
paths, the master servers' files...) are changed in `config.toml` or their
environment variables: `sv_reload` reads the file's `server.*` settings
again, and the web port and its address are bound once, at the start. The
rotation is changed by the `sv_rotation_*` commands.

## HTTP

The server listens on `server.web_port` (8080; 0 for none) at
`server.web_bind` (`127.0.0.1`, an IPv4 address). Bound beyond loopback,
every web password that is set (admin, moderator, viewer) must have 8
characters or more, else the server exits with code 3; the owner's
(`server.web_owner_password`) must have 8 or more wherever the port is
bound. The page (`/`), the API
(`/api/...`), the RCON (`/ws`), `/healthz` and `/metrics` share the port.

Requests are HTTP/1.1 (or 1.0), `GET`, `HEAD`, `POST`, `PUT` or `DELETE`
(`HEAD` is answered as `GET` is, without the body, and counts as a read,
as a `GET`; `/api/login` takes `POST` alone), a head of 8 KB at most whose
lines end with CRLF (a bare LF, a CR inside a line, or more after the empty
line that ends it: `400 bad_request`) and a body of 64 KB at most, sent within 10 seconds of the
connection's opening or of its last answer (a connection that sends
nothing is closed then too). `Transfer-Encoding` and `Expect` are refused.
24 connections are taken at once; when they are all taken, the oldest that
waits for a request (no RCON, no request being answered) is closed for a
new one. A client that does not read its answers is read no further once
256 KB of them wait, and is closed past 2 MB.

Every request but `/healthz` counts against its address's allowance, the
page's files, `/metrics`, a sign-in and the RCON's upgrades (`/ws`) too: 60
a minute, or 600 with a session (429 `rate_limited` past it, with
`Retry-After: 60`). Behind `server.web_trusted_proxy`, a request's address
is the last entry of its `X-Forwarded-For` (the one the proxy appended: an
IPv4 address, `a.b.c.d` or `a.b.c.d:port`, or an IPv6 one, `v6`, `[v6]` or
`[v6]:port`), and its `X-Forwarded-Host` and `X-Forwarded-Proto` are
believed, whether or not it sends `X-Forwarded-For`; from any other
address they are ignored. IPv6 clients are counted by their /64: one
host's addresses share its allowance and its lockouts. Every answer is JSON
(`Content-Type: application/json`) unless said, with
`Cache-Control: no-store`, `X-Content-Type-Options: nosniff`,
`Referrer-Policy: same-origin` (under `no-referrer` a browser would send
the page's own changes with `Origin: null`) and
`Content-Security-Policy: default-src 'self'; frame-ancestors 'none'`.

### Signing in

`POST /api/login` with `{"password":"…"}` answers
`{"ok":true,"token":"<64 hex>","role":"admin","csrf":"<32 hex>","expires":<unix seconds>}`
and sets the cookie `opence_session=<token>; HttpOnly; SameSite=Strict;
Path=/`, with `Secure` when the request came through
`server.web_trusted_proxy` with `X-Forwarded-Proto: https` (never from
another address, whatever it says). The admin password
(`server.web_password`) gives the admin role, the moderator password
(`server.web_moderator_password`) the moderator role, the viewer password
(`server.web_viewer_password`) the viewer role; `{"by":"…"}` beside the
password names who signed in, in the log. A session lasts 12 hours, or 30
minutes without a request (the RCON's messages do not count: a page left
open asks `GET /api/session` now and then); 16 are kept at most, of every
role.

The owner's password (`server.web_owner_password`, empty by default: none)
gives the owner role, for a supervisor on the server's own machine (a
fleet's agent, a panel's daemon): what standard input may do, over the
API and the RCON (`quit`, `sv_stop`, `sv_restart`, `POST /api/stop`, the
owner's settings by `sv_set` and `PUT /api/settings`, the game's own
console commands by `POST /api/command`, the players' addresses and
hardware ids in `/api/players` and the events). It signs in only when the
connection's own peer address is in `127.0.0.0/8` and is not the trusted
proxy's: never by `X-Forwarded-For`, and never through
`server.web_trusted_proxy`, even one on the machine itself. From any other
address the right owner's password is refused with `403 forbidden` (the
RCON's `auth`: `{"code":"forbidden"}`, and the connection closes 1008),
and counts toward the address's lockout as a wrong password does. An
owner's session is used from the machine itself alone, as it was begun
(from elsewhere its token is no session: `401`, the RCON's `auth` with it
`denied`). A password that is also another role's gives the owner role
from the machine itself and the other role from elsewhere: give the owner
a password of its own.

A script passes `Authorization: Bearer <token>`. The page passes the
cookie, and on every request that changes something the header
`X-CSRF-Token: <csrf>`; the `Origin` of such a request (and of the
WebSocket's) must be the server's own. An `Origin: null` is taken only
when the browser's `Sec-Fetch-Site` says `same-origin`.

`POST /api/logout` ends the session (with the `X-CSRF-Token` by the
cookie; `GET` answers 405). `GET /api/session` answers
`{"ok":true,"role":…,"csrf":…,"expires":…}`, or 401.

When `server.web_password`, `server.web_moderator_password`,
`server.web_viewer_password` or `server.web_owner_password` changes
(`sv_set` and the settings change the first two; `sv_reload` reads any of
them from `config.toml`), the sessions of its role end and that role's RCON
connections close (1008, `the password changed`).

Five wrong passwords from an address (an IPv6 one's /64) in 10 minutes
lock it out for 10 minutes (429), and no more than 30 passwords a minute
are checked in all. `/api/login` takes `POST` alone (any other method:
405).

### Errors

```json
{"ok":false,"error":{"code":"forbidden","message":"needs the admin role"}}
```

| Status | Codes |
| --- | --- |
| 400 | `bad_request`, `schema` (a field missing or out of range), `failed` (the command ran and failed: its lines in `lines`), `rejected` (rotation entries: `rejected[]`), `invalid` (a gametype file: `line`) |
| 401 | `denied` (no session, or a wrong password) |
| 403 | `forbidden` (the role; the owner's password from beyond the machine), `csrf`, `origin` |
| 404 | `not_found` (also a command that is not one, a player or a ban that is not there) |
| 405 | `method` |
| 409 | `pinned` (a setting its environment variable sets), `changed` (a player's action whose slot holds another player now: the current name in `name`, nothing done) |
| 413 | `too_large` |
| 426 | `bad_request` (the WebSocket's version is not 13) |
| 429 | `rate_limited`, `locked` (with `Retry-After`) |
| 500 | `internal` (out of memory, a file that cannot be written) |
| 503 | `loading` (the game did not answer in 20 seconds, as while a map loads; with `Retry-After: 5`), `starting`, `busy` (8 RCON connections already; with `Retry-After: 5`) |

### Endpoints

| Method, path | Role | Body → answer |
| --- | --- | --- |
| `GET /healthz` | none | `200 {"state":"lobby"}` (or `loading`, `playing`, `postgame`) while the server hosts and its frames run; `503` with `{"state":"starting"}`, `{"state":"stalled"}` (no frame for a minute) or the other state otherwise |
| `GET /metrics` | none (`server.metrics`) | Prometheus text |
| `GET /api/status` | viewer | The status (below) |
| `GET /api/players` | viewer | `{"ok":true,"players":[<player>…]}` (with `address` and `hwid` for the owner, or every role with `server.log_addresses`; and `discord_id`, `discord_name` and `discord_verified` with them: a verified account's, else what the machine says, `""` for none) |
| `POST /api/players/{slot}/kick` | moderator | `{"name":"<the player's name, checked>"}` → `{"ok":true,"lines":[…]}`, or `409 changed` when the slot holds another player |
| `POST /api/players/{slot}/ban` | moderator | `{"name":…,"duration":"1d","reason":"…"}` (no duration: for ever, which a moderator's ban cannot be: `400 failed`) |
| `POST /api/players/{slot}/mute` | moderator | `{"name":…,"minutes":10}` |
| `POST /api/players/{slot}/unmute` | moderator | `{"name":…}` |
| `POST /api/players/{slot}/tell` | moderator | `{"name":…,"text":"…"}` |
| `POST /api/say` | moderator | `{"text":"…"}` |
| `GET /api/rotation` | viewer | `{"ok":true,"index":0,"entries":[<entry>…],"next":<entry>}` |
| `PUT /api/rotation` | admin | `{"entries":["bloodgulch:ctf",…]}` → the rotation, or `400 rejected` with `"error":{…,"rejected":[{"index":1,"entry":"…","reason":"…"}]}` and nothing changed |
| `POST /api/rotation/next` | moderator | `{}`: `sv_map_next` |
| `POST /api/rotation/play` | moderator | `{"map":"…","gametype":"…"}`: `sv_map` (`map` may be a whole rotation entry, `map:gametype+rule=value`, 255 characters at most, as the page sends one with rules; a map or gametype that cannot be played: `400 failed`) |
| `GET /api/gametypes` | viewer | `{"ok":true,"builtin":["slayer",…],"schema":{"fields":[<field>…]},"files":[{"name":…,"base":…,"description":…,"error":null}]}` (`builtin`: the Xbox's gametypes, then OpenCE's; a file that cannot be read has `base` and `description` empty and `error` saying why) |
| `GET /api/gametypes/{name}` | viewer | `{"ok":true,"name":…,"text":"<the TOML>"}` |
| `PUT /api/gametypes/{name}` | admin | `{"text":"<TOML>"}` → `{"ok":true}`, or `400 invalid` with `line` and `message` (a name is `a`–`z`, `0`–`9`, `_` and `-`, 32 at most) |
| `DELETE /api/gametypes/{name}` | admin | `{"ok":true}` |
| `GET /api/maps` | viewer | `{"ok":true,"maps":[{"name":"bloodgulch","level":"levels\\test\\bloodgulch\\bloodgulch","kind":"xbox","title":"Blood Gulch"}…]}` (`kind`: `xbox` or `custom`; `title` is the map's display name, else its `name`) |
| `GET /api/bans` | admin | `{"ok":true,"bans":[{"n":0,"when":"2026-10-19 18:00:00","ip":"…","hwid":"…","discord_id":"…","discord_verified":true,"players":"…","reason":"…","until":<unix>\|null}…]}` |
| `GET /api/vpn` | viewer | `{"ok":true,"policy":"link","fail":"open","lists":[{"master":"<url>","key_id":"…","serial":12,"generated":<unix>,"fetched":<unix>\|null,"ranges":30582,"error":""}…],"file":"<server.vpn_list>","allow_ranges":0,"block_ranges":0,"held":true,"refused":{"vpn_link_required":0,"vpn_blocked":0,"vpn_link_invalid":0,"vpn_link_too_new":0,"vpn_link_in_use":0}}` (`held`: a list is held, from a master or the file) |
| `POST /api/vpn/check` | moderator | `{"ip":"203.0.113.5"}` → `{"ok":true,"listed":true,"by":"netlist"}` (`by`: `netlist`, `vpn_block.txt`, `vpn_allow.txt`, or `""`) |
| `POST /api/bans` | admin | `{"ip":"203.0.113.5","hwid":"…","discord_id":"…","names":"…","duration":"1d","reason":"…"}` (`ip`, `hwid` or `discord_id` (a verified account's: up to 20 digits), or more than one; the rest may be left out: no duration is for ever) → `{"ok":true,"n":<the new ban's number in GET /api/bans>}`: `sv_ban_address`, a machine present of the address or the hardware id dropped; `400 schema` for an address that is not IPv4, a hardware id that is not hex, a duration that is not one |
| `DELETE /api/bans/{n}` | admin | `{"ok":true}` |
| `GET /api/settings` | admin | `{"ok":true,"settings":[<setting>…]}` |
| `PUT /api/settings` | admin | `{"server.name":"…","server.max_players":16,"server.public":true,…}` (a value as JSON, a boolean, a number or a string, or its text: `"true"`, `"16"`; `null` empties it) → `{"ok":true,"applied":["server.name"],"errors":[{"name":…,"code":…,"message":…}]}`; each setting is changed or refused on its own |
| `POST /api/command` | the command's | `{"line":"sv_status"}` (511 characters at most) → `{"ok":true,"lines":["…"]}`, or a refusal (`failed`, `forbidden`, `not_found`, `pinned`) with the command's `lines` |
| `POST /api/stop` | owner | `{"after":"now\|game\|empty","notice":"…","restart":false}` (`notice` and `restart` may be left out) or `{"cancel":true}` → `{"ok":true,"lines":["…"]}`: `sv_stop` (`sv_restart` with `restart`) `after notice`, or `sv_stop cancel` (`400 failed` with no drain waiting); `400 schema` for another `after` |
| `GET /api/log?after=<n>&limit=<m>` | viewer | `{"ok":true,"lines":[{"n":12,"time":<ms>,"level":"info","cat":"game","text":"…"}…],"next":12,"missed":0}` (`limit` 1 to 1024, 256 by default; the chat's lines for admins only) |
| `GET /api/events?after=<seq>` | viewer | `{"ok":true,"events":[<event>…],"next":<seq>,"missed":0}` (`player_joined` and `player_left` with `address` and `hwid` for the owner, or every role with `server.log_addresses`) |
| `GET /ws` | | The RCON (below) |
| `GET /`, `/app.js`, `/app.css`, `/icon.svg` | none | The page (`server.web_ui`) |

`next` is the `after` of the next request: the last line's `n` (or event's
`seq`) read. An `after` past the latest (as after a restart) reads from the
start. `missed` counts lines or events that left the ring (2048 lines, 512
events) before they were asked for.

### The status

```json
{"ok":true,"time":1760900000123,"name":"OpenCE Server","state":"playing",
 "build":"build-206","network_version":38,"uptime":3600,"games_hosted":4,
 "map":"bloodgulch","level":"levels\\test\\bloodgulch\\bloodgulch","gametype":"ctf","variant":"CTF","teams":true,
 "players":5,"max_players":16,"machines":5,"countdown":-1,"game_seconds":312,"team_scores":[2,1],
 "listing":{"online":true,"public":true,"password":false,"invite":"halo://join/…"},
 "rotation":{"index":0,"count":3,"next":{"text":"sidewinder:team_slayer","map":"sidewinder","gametype":"team_slayer"}},
 "frame_ms_max":4.1,"frame_ms_avg":1.2,"tick_ms_avg":1.2,
 "masters":[{"url":"https://master1.example.org","listed":true,"probe":"ok","code":"","message":"","updated":1760899990}],
 "update":{"available":false,"build":""},
 "drain":null,
 "pid":4021,"started":1760896400,"instance":"eu-1","region":"eu","tunnel_port":2302,
 "publisher_key_id":"3f9a2c7d1e5b8a04","web_port":8080,
 "capabilities":["events","status","rotation","settings","bans","owner_web","ban_address","stop","event_addresses","identity"]}
```

`state` is `waiting` (the menus settling), `hosting` (making the game),
`lobby`, `loading`, `playing`, `postgame` or `stopping`. `countdown` is the
lobby's seconds left, `-1` when it is not counting. `team_scores` is `null`
in a game without teams. `frame_ms_max` and `frame_ms_avg` are the longest
and the mean frame of the last minute (`tick_ms_avg` is the mean again, for
older readers). `invite` is given to admins only (empty for others). An entry of the rotation is
`{"text":"sidewinder:team_slayer+score=50","map":"sidewinder","gametype":"team_slayer","rules":{"score":50},"rejected":null}`
(`rejected` is why it cannot be played, else `null`). `drain` is the
drain waiting (`sv_stop game`...), `{"after":"game","restart":false,"since":<unix
seconds>}`, else `null`. `pid` is the server's process, `started` when it
started (seconds since 1970), `instance` the name the master servers know
it by (`server.instance`, else the slug of `server.name` they are given),
`region` `server.region`, `tunnel_port` the tunnel's UDP port
(`network.tunnel_port`; 0 when `network.online` is off), `publisher_key_id`
the publisher key's id (16 hex digits; empty without a key: `sv_publisher`)
and `web_port` `server.web_port`. `capabilities` is the RCON's hello's. The rotation's
`index` (in the status and `GET /api/rotation`) is the entry being played,
`-1` while an `sv_map` game is played or before the first lobby; `next` is
the entry after the one played last (the first while none has been).

A player is
`{"slot":0,"name":"Milenko","team":0,"score":3,"kills":3,"deaths":1,"assists":0,"ping":42,"machine":1,"joined":1760899000,"muted":false}`;
with the owner role, or `server.log_addresses`, also
`"address":"203.0.113.5","hwid":"…"`. `team` is 0 (red), 1 (blue) or -1
(no teams); `ping` is in milliseconds, `null` when it is not known yet.
`client` is the player's machine's build as its game tells it (shortly
after it joins), for every role:
`{"build":282,"flavor":"release","version":"OpenCE Linux | build 282 (release)","repository":"OpenCommunityEdition/OpenCE","mods":["trollface 1.0"]}`
(`build` 0 for a build CI did not number, `repository` empty when the build
does not know it, `mods` the mods it plays in their load order, each its
folder and its version), or `null` from a game that does not tell it. It is
what the player's game says, which a changed build can make anything.

A setting is
`{"name":"server.name","type":"string","value":"OpenCE Server","default":"OpenCE Server","comment":"…","env":"HALO_SERVER_NAME","pinned":false,"secret":false,"choices":null,"min":null,"max":null}`;
`type` is `boolean`, `integer`, `real` or `string`, and `value` and
`default` are of that type in JSON (`true`, `16`, `"OpenCE Server"`); a
reader should also take their text (`"true"`, `"16"`), which older servers
sent. `choices` lists a word setting's words, `min` and `max` bound a
number's, each `null` where there is none (the table above). `env` is
`null` for a setting no environment variable sets. A secret one (the
passwords) has `value` and `default` empty and `"set":true` when it is
set. `pinned` is a setting
its environment variable (`env`) sets: it cannot be changed.

A gametype schema field is
`{"key":"score_to_win","alias":"score","table":"universal","engine":"all","type":"integer","min":0,"max":999,"choices":null,"default":null,"description":"…"}`
(`table`: the gametype file's table it goes in, `universal`, `options`,
`ctf`, `slayer`, `oddball`, `king` or `race`; `engine`: `all`, or the game
engine whose table it is; `type`: `integer`, `boolean` or `choice`).

## The RCON

`GET /ws` with `Upgrade: websocket` and the subprotocol `opence-rcon-1`.
Every frame is text holding one JSON object. The server speaks first:

```
→ {"type":"hello","protocol":1,"server":"OpenCE Server","build":"build-206","network_version":38,
   "capabilities":["events","status","rotation","settings","bans","owner_web","ban_address","stop","event_addresses","identity","vpn","link"]}
← {"type":"auth","password":"…","by":"paul"}       or {"type":"auth","token":"<session token>"}
→ {"type":"auth","ok":true,"role":"admin"}
→ {"type":"auth","ok":false,"error":{"code":"denied"}}  and the connection closes (1008)
← {"type":"command","id":1,"line":"kick Milenko"}    or {"type":"command","id":1,"command":"say","args":["two words"]}
→ {"type":"reply","id":1,"ok":true,"lines":["Milenko kicked by the host"]}
→ {"type":"reply","id":2,"ok":false,"lines":["…why…"]}                       (the command ran and failed)
→ {"type":"reply","id":3,"ok":false,"lines":[],"error":{"code":"busy","message":"…"}}   (not run)
← {"type":"subscribe","events":["player","chat","game","moderation","status","log","master"],"after":1830}
→ {"type":"event","seq":1831,"time":1760900000123,"event":"player_joined","slot":3,"name":"Milenko",…,"replay":true}
→ {"type":"event","seq":1831,"time":…,"event":"status","status":{…the status…}}   (at the subscribe, then every 2 s)
→ {"type":"event","time":…,"event":"log","n":5120,"level":"info","cat":"game","text":"…"}   (no seq: n is its sequence)
← {"type":"ping"}
→ {"type":"pong"}
→ {"type":"overflow"}  and the connection closes (1013): the client read too slowly
→ {"type":"error","id":…,"error":{"code":"…","message":"…"}}   (an unknown type, a malformed message)
```

A connection signed in with the page's cookie (on the same origin) needs no
`auth` message: its `hello` is followed by `{"type":"auth","ok":true,…}`.
The `capabilities` say what the server has beyond the first release's
(`events`, `status`, `rotation`, `settings`, `bans`): `owner_web`,
`ban_address`, `stop`, `event_addresses` and `identity` ("Panels and
fleets"), `vpn` (the VPN policy, `/api/vpn`, `join_refused`) and `link`
(verified Discord accounts: `discord_*` in players, bans by `discord_id`);
a client checks for one before it uses it. The owner's password
signs in from the server's own machine alone ("Signing in").
A command is a `line`, or a `command` and its `args`, which the server
joins with spaces, quoting an argument that holds a space or is empty (511
characters in all at most). Ids are the client's (a number or a string),
echoed in the reply; each command gets exactly one reply. A connection
runs one command at a time: a command sent before the last one's reply is
not run. A reply whose command ran has `ok` and the command's `lines`, and
no `error`, whether it succeeded or failed; one that was not run has `ok`
false, no lines and an `error`: `denied` (not signed in), `rate_limited`
(32 commands a second, in bursts of 64), `bad_request` (no command, or too
long), `busy` (another command's reply is awaited) or `loading` (the game
did not answer in 20 seconds; the command may still have run, and is never
sent again by the server). `after` replays the events still in the ring
since that sequence number, marked `"replay":true`. A connection holds 1024
messages for the client at most; 8 connections are taken at once (a ninth
upgrade answers `503 busy`), and each upgrade counts against its address's
allowance (above). The
event groups are `player` (`player_joined`, `player_left`, `map_share`), `chat`
(`chat`), `game` (`lobby_opened`, `countdown`, `game_started`,
`game_ended`, `postgame`, `score`, `kill`, `afk_kick`, `coop_bsp_switch`,
`map_vote_started`, `map_vote_ended`), `moderation` (`kick`, `ban`,
`unban`, `mute`, `unmute`, `speed_hack`, `join_refused`), `status` (`status`), `log`
(`log`, each log line as `{"n","level","cat","text"}`), `master`
(`master`), and `server` (`server_started`, `reloaded`, `warning`,
`drain`, `shutdown`), which every subscriber gets.
The log's lines of the `chat` category go to the `chat` group, the others
to `log`.
`status` and `log` events go to their subscribers as they come and are
not kept in the ring: `after` does not replay them (`/api/log` reads the
log's own ring). A `status` event's `seq` is the latest event's (no new
number: the same one comes again until another event), and a `log` event
has no `seq`: its `n`, the log line's number and `/api/log`'s cursor, is
its sequence. A client keeps one cursor for the events (`seq`, of the
events of the ring) and another for the log's lines (`n`); a client that
reconnects with `after` may get events it had, and drops those whose `seq`
it has seen (a `status` event is never dropped so).

## Events

Every event has `seq` (rising, never reused while the server runs; a
`status` event has the latest one's, a `log` event none: above), `time`
(milliseconds since the epoch) and `event`:

| Event | Fields |
| --- | --- |
| `server_started` | `name, build, network_version, pid, started, instance, region, tunnel_port, publisher_key_id, web_port` (as the status's), `rotation[]` (the entries' texts) |
| `lobby_opened` | `map, gametype, variant` (the gametype's name as the game shows it), `teams, index` (the rotation's entry) |
| `countdown` | `seconds` (-1: paused) |
| `game_started` | `map, gametype, variant, teams, team_scores[]` (team games), `players[]` |
| `game_ended` | `map, gametype, variant, teams, reason` (`score`, `time`, `idle`, `empty`, `command`, `next`, `update`: a new network version's install, `vote`: the players' vote to end it), `duration` (seconds), `team_scores[]` (team games), `players[]` |
| `postgame` | `seconds` |
| `player_joined` | `slot, name, team, machine`; `address, hwid, discord_id, discord_name, discord_verified` for the owner, or every role with `server.log_addresses` |
| `join_refused` | `name` (the machine's), `reason` (`vpn_link_required`, `vpn_blocked`, `vpn_link_invalid`, `vpn_link_too_new`, `vpn_link_in_use`, `address_full`); `address, hwid` as `player_joined`'s |
| `player_left` | `slot, name, reason` (`left`; `kicked` and `banned` when the server's `kick`, `ban` or `sv_ban_address` dropped the machine, `banned` for a speed hack's drop too, `afk` for a player away); `address, hwid` (as it joined) as `player_joined`'s |
| `afk_kick` | `players` (the machine's players' names), `seconds` (away) |
| `coop_bsp_switch` | `from, to` (the level's parts), `players[]` (the names of those who crossed) |
| `map_vote_started` | `seconds`, `choices[]` (`{"map","gametype","index"}`: the rotation's entries) |
| `map_vote_ended` | `map, gametype, index, votes` (the winner) |
| `chat` | `channel` (`everyone`, `team`, `vehicle`), `slot, name, text` (admins only; the lines the host sends on) |
| `kill` | `killer, victim` (slots; -1 for none), `weapon` (the killer's weapon's or the killing object's tag name, or null), `friendly_fire` |
| `score` | `map, gametype, variant, teams, team_scores[]` (team games), `players[]` (once a second at most, on a change) |
| `kick`, `ban`, `unban`, `mute`, `unmute` | `name, by, reason?, until?` |
| `speed_hack` | `machine, names, rate, ahead_ticks, action` (`kicked`, or `unverified`: its datagrams alone said so, and it stays) |
| `map_share` | `machine, map, state` (`asked`, `downloading` at each tenth, `downloaded` or `failed` once for each request), `progress` (0 to 100: a percentage already) |
| `warning` | `text` (a newer release, with `server.auto_update`) |
| `reloaded` | `rotation_changed, rejected[]` (`{"entry","reason"}` of each entry left out) |
| `master` | `url, listed, probe, code, message` |
| `drain` | `after` (`now`, `game`, `empty`, `cancel`), `restart` (exit code 5), `notice` (empty for none) |
| `shutdown` | `reason` (`quit`, `sv_stop`, `sv_restart`, `drained`: a drain's end, `signal <n>`, `update installed`, `nothing to play`, `debug.exit_after`), `code` (the exit code) |

Players in events are as in `/api/players` for the viewer role (no
addresses), but for `player_joined` and `player_left`, which carry the
player's address and hardware id (`""` when not known) to the owner, and to
every role when `server.log_addresses` is on (`/api/players`' rule), in the
RCON's events, their replay and `GET /api/events` alike. `variant` is the gametype's name as the game shows it (`Co-op`
in a co-op entry).

## Bans

A ban is a line of `bans.txt` (in `paths.logs`, else the data root). A
timed ban is kept out by its line's `until=` alone, and ends then. A ban
for ever, and a machine dropped for cheating, also keep the machine out
for the rest of the run by its address, its real address or its hardware
id. `sv_ban_address` and `POST /api/bans` write such a line for an address
or a hardware id with no player of it needed (the part not given is
`none`). `unban` takes the line out and readmits the machine by
the line's address or hardware id, whether it joins on the LAN or by
internet play.

A line's `discord_id` is a verified Discord account's when it has no
`(self-reported)` after it: `ban` writes it so for a machine that showed a
pass, and `sv_ban_address discord:<id>` and `POST /api/bans`' `discord_id`
write it. Such a line keeps out every machine that shows a pass of that
account, from any address. What a machine says of its Discord account
itself is written with `(self-reported)` and never matched: anyone could
say it.

## The rotation

`server.rotation` (and `sv_rotation_set`, `PUT /api/rotation`) is a list of
entries, `map[:gametype[+rule=value...]]`, separated by commas, semicolons
or new lines; a line that begins with `#` is a comment (to its line's end,
commas and all). The text is 4096 characters at most (the entries past it
are left out, with a warning), an entry 255 characters and the list 64
entries. `config.toml` keeps a string setting of up to 4096 characters, so
a rotation that long is saved whole and read back after a restart.

- The map is a multiplayer map's name (`bloodgulch`: the Xbox's
  `maps/<name>.map`, else a Custom Edition map of `custom_maps`),
  `custom_maps\<name>` (a Custom Edition map; `/` is taken for `\`), or a
  level's path (`levels\test\bloodgulch\bloodgulch`). A campaign level is
  played as co-op only: `<level>:coop`, at `server.coop_difficulty`.
- The gametype (`slayer` when there is none) is a built-in's name
  (`slayer`, `team_slayer`, `slayer_pro`, `elimination`, `phantoms`,
  `endurance`, `rockets`, `snipers`, `stalker`, `ctf`, `ironctf`,
  `ctf_pro`, `invasion`, `oddball`, `team_oddball`, `reverse_tag`,
  `accumulation`, `juggernaut`, `king`, `team_king`, `king_pro`,
  `crazy_king`, `race`, `team_race`, `rally`, `team_rally`, and Halo PC's
  names `ffa`, `ffa_slayer`, `teamslayer`, `tslayer`, `koth`, `team_koth`,
  `iron_ctf`, `teamrace`, `teamoddball`), an OpenCE gametype's (`swat`,
  `infection`, `gun_game`, `hardcore`, `hardcore_ctf`, `hardcore_oddball`,
  `hardcore_king`), a gametype file's name (`gametypes/<name>.toml`), or
  `file=<name>` (a gametype a player's game saved, below). A gametype's
  name is letters, digits, `_`, `-` and `.`, in any case.
- A rule is a gametype rule's name or its short name (`score`, `time`,
  `weapons`, `ff`...; `GET /api/gametypes`'s schema lists them), with a
  value it takes; a rule of another game engine than the gametype's is
  refused. 48 rules at most; a rule given twice is the last.

An entry that cannot be played (a map that is not there, a bad gametype
file, a rule out of range) is left out with a warning, at the start and at
`sv_reload`; `sv_rotation_set` and `PUT /api/rotation` change nothing when
one of their entries cannot be played.

A change of the rotation (`sv_rotation_*`, `PUT /api/rotation`,
`sv_reload`) keeps its place: the next game is the entry after the one
played last, found again in the new list (the first entry only when none
has been played yet). `sv_reload` takes the rotation again when its text
changed, and also when only its entries' rejections changed (a map or a
gametype file added or taken out since).

`server.team_start` decides how a team game starts. `one` (the default)
starts it with the players of one team. `both` waits, saying nothing,
for a player on each team. `skip`, when one player waits for a team game
that `sv_map` did not ask for, plays the rotation's next entry without
teams instead (if it has one); it is checked when the lobby opens and
again when its countdown would begin. With no entry left, the server
stops with exit code 3 (2 when no rotation map is there), or plays
`bloodgulch:slayer` with `server.rotation_fallback`.

## Gametype files

A gametype file, `gametypes/<name>.toml` (the name `a`–`z`, `0`–`9`, `_`
and `-`), is a built-in or OpenCE gametype (`base`) and rules over it:

```toml
base = "team_king"     # the gametype it starts from
name = "Moving Hill"   # the name the game shows: 11 characters of printable ASCII

[universal]
score_to_win = 3

[options]
time_limit = 15
friendly_fire = "off"

[king]
moving_hill = true
```

Each rule goes in its table (the schema's `table`): `[universal]`,
`[options]`, and one game engine's, `[ctf]`, `[slayer]`, `[oddball]`,
`[king]` or `[race]`. A file gives each rule its full name
(`vehicle_set`, `team_vehicles`, `score_to_win`): the short names
(`vehicles`, `score`) are the rotation's alone. A file is 64 KB at most.
`deploy/gametypes` has examples. The server reads the `gametypes` folder
beside `config.toml` if there is one, else the data root's, and writes the
web page's files to it.

A gametype a player's game saved is played with `file=<name>`: copy the
saved gametype, as `<name>.gametype`, into the same `gametypes` folder
(beside `config.toml` if there is one, else the data root's).

## The listing

A public server (`server.public`, with `network.online` and
`network.public_lobby`) is published to each master server of
`masters.txt` (`docs/master_protocol.md`). A change that a list shows (the
listing made public or private, a player joining or leaving, the name, the
map) is published within 5 seconds. Made private, the server makes a new
invite and sends the masters a closed publish, which takes it off their
lists;
`network.public_lobby = false` stops the publishing too. A master's
`Retry-After` is kept (a day at most). `key_revoked` and
`publisher_revoked` stop the publishing to that master for the run;
`unknown_key`, `publisher_pending`, `application_denied` and
`listing_taken` are logged once a minute while they last.

## Files

| File | What |
| --- | --- |
| `config.toml` | Every setting (beside the executable; `HALO_CONFIG_FILE`, else `--config`, names another) |
| `init.txt` (`server.init_file`) | Commands run at the start, in the data root unless a full path |
| `gametypes/<name>.toml` | Gametype files: the `gametypes` folder beside `config.toml` if there is one, else the data root's |
| `gametypes/<name>.gametype` | Gametypes saved by a player's game (`file=<name>` in the rotation): the same `gametypes` folder |
| `publisher.key` | The publisher key for the master servers (beside `config.toml`). On Windows, a server's `config.toml` and `publisher.key` are given an access list of their owner alone (the entries inherited for Administrators and SYSTEM taken out) |
| `masters.txt` | The master servers (`server.masters_file`: beside `config.toml`, else beside the executable); one that names none leaves the official master, built in |
| `bans.txt`, `cheaters.txt`, `debug.txt` | In `paths.logs`, else the data root |
| `vpn_allow.txt`, `vpn_block.txt`, `netlist-<master key id>.bin` | Beside `bans.txt`: ranges never refused as a VPN's, ranges refused besides, and the masters' netlists held ("VPNs and linked Discord accounts") |
| `status.json` (`server.status_file`) | The status, written every 2 seconds |
| A password's file (`HALO_SERVER_WEB_PASSWORD_FILE`, `HALO_SERVER_WEB_OWNER_PASSWORD_FILE`, `HALO_SERVER_WEB_MODERATOR_PASSWORD_FILE`, `HALO_SERVER_WEB_VIEWER_PASSWORD_FILE`, `HALO_SERVER_PASSWORD_FILE`) | A container's secret: when the plain variable is not set, the file's first line (its CR and LF off) is the setting's value, which is then pinned as the plain variable's would be (`sv_set` and `PUT /api/settings` refuse it, `409 pinned`). Read once, at the start; a file that cannot be read is logged, and the setting is `config.toml`'s |

## Releases

Each release has `SHA256SUMS` (the SHA-256 of each of its files, as
`sha256sum` writes them: `<hex>  <name>`, written by `tools/ci_build.py
sums`) and `SHA256SUMS.sig`: the Ed25519 signature of that file's exact
bytes by the project's release key, in 128 lowercase hex digits and a
newline (the form `opence-master-server`'s `cmd/release-sign` writes). The
public key is `deploy/release-key.pub` (64 hex digits:
`d04e8b101007d1641cace8b95fcca76f7f44709f6885c3d031b52394dbdf9376`).
`tools/release_sign.sh verify SHA256SUMS` checks the signature (OpenSSL 3),
and `sha256sum -c --ignore-missing SHA256SUMS` the files downloaded. The
build workflow signs with the `RELEASE_SIGNING_KEY` secret (the key's seed,
64 hex digits) and checks the signature against `deploy/release-key.pub`
before it publishes the release.

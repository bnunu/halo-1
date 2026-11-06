#!/usr/bin/env python3
"""The dedicated server's protocol simulator: the HTTP API, the RCON over
WebSocket and the admin page of docs/dedicated_server.md, with a scripted
server behind them and no game.

    python3 tools/server_sim.py --port 8099      then open http://127.0.0.1:8099/

It serves the page's files from port/assets/server_ui (read at each request,
so an edit shows on a reload), plays games one after another with players
joining, chatting, killing and leaving, a speed hack and a kick, and keeps
the doc's rules: the headers, sessions, CSRF and Origin, the roles,
lockout. It is the page's development server and the reference that the
page's tests (tools/server_ui_tests) and third-party tools test against.

Like the real server: --trusted-proxy <address> believes that proxy's
X-Forwarded-For (its last entry), -Host and -Proto (and only then marks the
cookie Secure); --request-limit counts each address's requests (60 a minute
without a session, 600 with one, WebSocket upgrades too: off by default, as
the tests sign in often). --owner-password signs in as the owner from
loopback alone (a connection's own address in 127.0.0.0/8, not the trusted
proxy's), as server.web_owner_password does. A stop (quit, sv_stop,
sv_restart, a drain's end) is said (the shutdown event) and the simulator
goes on, unless --exit-on-stop: then it exits with the stop's code.

Fault injection: --slow-consumer (each WebSocket is written slowly and a
burst follows a subscription, so its queue overflows), --drop (WebSockets
are cut without a close frame a few seconds after they open, and one API
request in ten loses its connection unanswered), --big-replies (command
replies of more than 1 MiB), --fragment (every message the server sends is
split into continuation frames, with a ping between them).

The first line on standard output is `listening on http://<address>:<port>`
(with --port 0 a free port is taken). Standard library only.
"""

import argparse
import base64
import hashlib
import hmac
import http
import ipaddress
import json
import os
import random
import re
import secrets
import socket
import socketserver
import struct
import sys
import threading
import time
from collections import deque
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
UI_DIR = ROOT / "port" / "assets" / "server_ui"

ROLES = ["viewer", "moderator", "admin", "owner"]
SUBPROTOCOL = "opence-rcon-1"
WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
MAXIMUM_HEAD = 8 * 1024
MAXIMUM_BODY = 64 * 1024
HEAD_SECONDS = 10
SESSION_LIFETIME = 12 * 3600
SESSION_IDLE = 30 * 60
MAXIMUM_SESSIONS = 16
LOG_RING = 2048
EVENT_RING = 512
WS_QUEUE = 1024
WS_CONNECTIONS = 8
WS_FRAME = 64 * 1024
WS_MESSAGE = 256 * 1024
BUILD = "build-206"
COMMAND_LINE = 511
ROTATION_TEXT = 4096
ENTRY_TEXT = 255
NOTICE_SIZE = 128
NETWORK_VERSION = 38
# the RCON's hello's capabilities, and the status's (server_http.c's server_http_capabilities)
CAPABILITIES = ["events", "status", "rotation", "settings", "bans", "owner_web", "ban_address", "stop",
                "event_addresses", "identity", "vpn", "link"]
# the VPN policy's netlist (server_netlist.c): a master's list the simulator holds, and the addresses it lists
VPN_LISTED = ("198.51.100.0", "198.51.100.255")
VPN_REASONS = ("vpn_link_required", "vpn_blocked", "vpn_link_invalid", "vpn_link_too_new", "vpn_link_in_use")
PUBLISHER_KEY_ID = "3f9a2c7d1e5b8a04"
DRAINS = ("now", "game", "empty", "cancel")

CSP = "default-src 'self'; frame-ancestors 'none'"
SECURITY_HEADERS = [
    ("Cache-Control", "no-store"),
    ("X-Content-Type-Options", "nosniff"),
    # (same-origin, not no-referrer: under no-referrer a browser sends the page's own posts with Origin: null)
    ("Referrer-Policy", "same-origin"),
    ("Content-Security-Policy", CSP),
]
ASSETS = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/index.html": ("index.html", "text/html; charset=utf-8"),
    "/app.js": ("app.js", "text/javascript; charset=utf-8"),
    "/app.css": ("app.css", "text/css; charset=utf-8"),
    "/icon.svg": ("icon.svg", "image/svg+xml"),
}

# ---------- the CSP check on the page's files (the build's embedder uses it too)

_JS_FORBIDDEN = [
    (r"\binnerHTML\b|\bouterHTML\b|\binsertAdjacentHTML\b", "HTML set from a string"),
    (r"\bdocument\.write", "document.write"),
    (r"(?<![\w.])eval\s*\(", "eval"),
    (r"\bnew\s+Function\b", "new Function"),
    (r"\bset(?:Timeout|Interval)\s*\(\s*['\"`]", "a timer given a string"),
    (r"setAttribute\(\s*['\"](?:style|on[a-z]+)['\"]", "a style or event handler attribute"),
    (r"\.style\.cssText|\.style\s*=", "an inline style"),
    (r"https?://", "an external address"),
]


def csp_problems(name, text):
    """what in one of the page's files the page's Content-Security-Policy
    (default-src 'self') would block or the project forbids: inline scripts
    and styles, style= and on*= attributes, string evaluation, external
    requests; an empty list when there is nothing"""
    problems = []
    kind = name.rsplit(".", 1)[-1].lower()
    if kind in ("html", "svg"):
        for match in re.finditer(r"<script\b([^>]*)>(.*?)</script\s*>", text, re.I | re.S):
            if kind == "svg" or not re.search(r"\bsrc\s*=", match.group(1), re.I) or match.group(2).strip():
                problems.append("an inline <script>")
        if re.search(r"<style\b", text, re.I):
            problems.append("a <style> element")
        for tag in re.finditer(r"<[A-Za-z][^>]*>", text):
            if re.search(r"\sstyle\s*=", tag.group(0), re.I):
                problems.append("a style= attribute: " + tag.group(0)[:60])
            if re.search(r"\son[a-z]+\s*=", tag.group(0), re.I):
                problems.append("an on*= attribute: " + tag.group(0)[:60])
        if re.search(r"javascript:", text, re.I):
            problems.append("a javascript: address")
        for match in re.finditer(r"\b(?:src|href|xlink:href|action|srcset|poster|data)\s*=\s*[\"']?\s*((?:https?:)?//)",
                                 text, re.I):
            tag_start = text.rfind("<", 0, match.start())
            tag = text[tag_start:match.start()]
            if not re.match(r"<a\b", tag, re.I):
                problems.append("an external request: " + text[match.start():match.start() + 60])
        if kind == "svg" and re.search(r"<foreignObject\b", text, re.I):
            problems.append("a foreignObject")
    elif kind == "js":
        for pattern, what in _JS_FORBIDDEN:
            for match in re.finditer(pattern, text):
                line = text.count("\n", 0, match.start()) + 1
                problems.append(f"{what} (line {line})")
    elif kind == "css":
        if re.search(r"@import\b", text, re.I):
            problems.append("an @import")
        for match in re.finditer(r"url\(\s*['\"]?([^'\")]*)", text, re.I):
            target = match.group(1).strip()
            if re.match(r"(?:[a-z]+:|//)", target, re.I):
                problems.append("a url() the policy blocks: " + target[:60])
        if re.search(r"expression\s*\(", text, re.I):
            problems.append("an expression()")
    return problems


# ---------- what the simulated server has

MAPS = [
    ("beavercreek", "Battle Creek"), ("bloodgulch", "Blood Gulch"), ("boardingaction", "Boarding Action"),
    ("carousel", "Derelict"), ("chillout", "Chill Out"), ("damnation", "Damnation"),
    ("dangercanyon", "Danger Canyon"), ("deathisland", "Death Island"), ("gephyrophobia", "Gephyrophobia"),
    ("hangemhigh", "Hang 'Em High"), ("icefields", "Ice Fields"), ("infinity", "Infinity"),
    ("longest", "Longest"), ("prisoner", "Prisoner"), ("putput", "Chiron TL-34"), ("ratrace", "Rat Race"),
    ("sidewinder", "Sidewinder"), ("timberland", "Timberland"), ("wizard", "Wizard"),
]
# (a Custom Edition map whose name the game shows is unknown: its title is its name)
CUSTOM_MAPS = [("hugeass", "Hugeass"), ("bigassv2", None)]

# the built-in gametypes (port/linux/game/server_gametypes.c's, then OpenCE's
# of game_modes.c): name -> (engine, teams, variant); /api/gametypes sends
# their names alone
BUILTINS = {
    "slayer": ("slayer", False, "Slayer"), "team_slayer": ("slayer", True, "Team Slayer"),
    "slayer_pro": ("slayer", False, "Slayer Pro"), "elimination": ("slayer", False, "Elimination"),
    "phantoms": ("slayer", False, "Phantoms"), "endurance": ("slayer", False, "Endurance"),
    "rockets": ("slayer", False, "Rockets"), "snipers": ("slayer", False, "Snipers"),
    "stalker": ("oddball", False, "Stalker"),
    "ctf": ("ctf", True, "CTF"), "ironctf": ("ctf", True, "Iron CTF"), "ctf_pro": ("ctf", True, "CTF Pro"),
    "invasion": ("ctf", True, "Invasion"),
    "oddball": ("oddball", False, "Oddball"), "team_oddball": ("oddball", True, "Team Ball"),
    "reverse_tag": ("oddball", False, "Reverse Tag"), "accumulation": ("oddball", False, "Accumulate"),
    "juggernaut": ("oddball", False, "Juggernaut"),
    "king": ("king", False, "King"), "team_king": ("king", True, "Team King"), "king_pro": ("king", False, "King Pro"),
    "crazy_king": ("king", False, "Crazy King"),
    "race": ("race", False, "Race"), "team_race": ("race", True, "Team Race"), "rally": ("race", False, "Rally"),
    "team_rally": ("race", True, "Team Rally"),
    "swat": ("slayer", True, "SWAT"), "infection": ("slayer", True, "Infection"), "gun_game": ("slayer", False, "Gun Game"),
    "hardcore": ("slayer", True, "Hardcore"), "hardcore_ctf": ("ctf", True, "HC CTF"),
    "hardcore_oddball": ("oddball", True, "HC Oddball"), "hardcore_king": ("king", True, "HC King"),
}
GAMETYPE_ALIASES = {"ffa": "slayer", "ffa_slayer": "slayer", "teamslayer": "team_slayer", "tslayer": "team_slayer",
                    "koth": "king", "team_koth": "team_king", "iron_ctf": "ironctf", "teamrace": "team_race",
                    "teamoddball": "team_oddball"}

LOADOUT_WEAPONS = ["none", "random", "assault_rifle", "pistol", "shotgun", "sniper_rifle", "rocket_launcher",
                   "plasma_pistol", "plasma_rifle", "needler"]


def _field(table, key, kind, description, engine="all", alias=None, low=None, high=None, choices=None):
    if kind == "boolean":
        low, high = None, None
    return {"key": key, "alias": alias, "table": table, "engine": engine, "type": kind, "min": low, "max": high,
            "choices": choices, "default": None, "description": description}


# (port/linux/src/server_rotation.c's server_rule_definitions)
SCHEMA = [
    _field("universal", "score_to_win", "integer", "The score that wins (kills, captures, laps, seconds of the ball "
           "or the hill...); 0 for no limit", alias="score", low=0, high=999),
    _field("universal", "lives", "integer", "Lives each player has; 0 for no limit", low=0, high=99),
    _field("universal", "respawn_time", "integer", "Seconds before a dead player respawns", alias="respawn", low=0,
           high=300),
    _field("universal", "respawn_time_growth", "integer", "Seconds added to the respawn at each death", low=0, high=300),
    _field("universal", "suicide_penalty", "integer", "Seconds added to a suicide's respawn", low=0, high=300),
    _field("universal", "health", "integer", "Players' health, in percent", low=25, high=400),
    _field("universal", "teams", "boolean", "Teams (CTF always has them)"),
    _field("universal", "odd_man_out", "boolean", "The player in the lead is shown to everyone"),
    _field("universal", "radar", "boolean", "Players on the motion tracker"),
    _field("universal", "friend_indicators", "boolean", "Teammates' markers over their heads"),
    _field("universal", "infinite_grenades", "boolean", "Grenades never run out"),
    _field("universal", "no_shields", "boolean", "Players have no shields"),
    _field("universal", "invisible", "boolean", "Players are always camouflaged"),
    _field("universal", "generic_equipment", "boolean", "Every map starts players with the same weapons"),
    _field("universal", "weapon_set", "choice", "The weapons on the map", alias="weapons",
           choices=["normal", "pistols", "rifles", "plasma", "sniper", "no_sniping", "rocket_launchers", "shotguns",
                    "short_range", "human", "no_grenades", "covenant", "classic", "heavy"]),
    _field("universal", "vehicle_set", "choice", "The vehicles on the map", alias="vehicles",
           choices=["default", "none", "warthog", "ghost", "scorpion"]),
    _field("universal", "goal_radar", "choice", "How the flag, the ball or the hill is shown",
           choices=["motion_tracker", "nav_point", "none"]),
    _field("options", "time_limit", "integer", "The game's minutes; 0 for no limit", alias="time", low=0, high=1440),
    _field("options", "friendly_fire", "choice", "Damage to teammates", alias="ff",
           choices=["on", "off", "shields_only", "explosives_only"]),
    _field("options", "friendly_fire_penalty", "integer", "Seconds added to a team killer's respawn", low=0, high=300),
    _field("options", "vehicle_respawn_time", "integer", "Seconds before a destroyed vehicle respawns; 0 for never",
           low=0, high=300),
    _field("options", "auto_team_balance", "boolean", "Joining players go to the smaller team", alias="balance"),
    _field("options", "radar_players", "choice", "Which players the motion tracker shows",
           choices=["all", "friends", "none"]),
    _field("options", "loadout", "choice", "The starting weapons: the weapon set's (category), or primary and "
           "secondary (custom)", choices=["category", "custom"]),
    _field("options", "primary_weapon", "choice", "A custom loadout's first weapon", choices=LOADOUT_WEAPONS),
    _field("options", "secondary_weapon", "choice", "A custom loadout's second weapon", choices=LOADOUT_WEAPONS),
    _field("options", "no_map_weapons", "boolean", "No weapons lie on the map"),
    _field("options", "team_vehicles", "choice", "Each team's vehicles: the vehicle set's (gametype), every one the "
           "map places (pc), or the counts below (custom)", choices=["gametype", "pc", "custom"]),
    _field("options", "warthogs", "integer", "Warthogs of each team, with team_vehicles custom", low=0, high=4),
    _field("options", "ghosts", "integer", "Ghosts of each team, with team_vehicles custom", low=0, high=4),
    _field("options", "scorpions", "integer", "Scorpions of each team, with team_vehicles custom", low=0, high=4),
    _field("options", "rocket_warthogs", "integer", "Rocket Warthogs of each team, with team_vehicles custom", low=0,
           high=4),
    _field("options", "banshees", "integer", "Banshees of each team, with team_vehicles custom", low=0, high=4),
    _field("options", "gun_turrets", "integer", "Gun turrets of each team, with team_vehicles custom", low=0, high=4),
    _field("options", "hardcore", "boolean", "Hardcore: no motion sensor, no aim assist, a kill restores the "
           "killer's health and shields"),
    _field("ctf", "assault", "boolean", "One team defends the flag, the other attacks", engine="ctf"),
    _field("ctf", "reset_on_capture", "boolean", "Both flags go home at a capture", engine="ctf"),
    _field("ctf", "flag_must_reset", "boolean", "A dropped flag goes home only by its timer", engine="ctf"),
    _field("ctf", "flag_at_home_to_score", "boolean", "A team's own flag must be home to score", engine="ctf"),
    _field("ctf", "single_flag_time", "integer", "Seconds of each side's turn in a single flag game", engine="ctf",
           low=0, high=600),
    _field("slayer", "no_death_bonus", "boolean", "No point for a kill after a death", engine="slayer"),
    _field("slayer", "no_kill_penalty", "boolean", "No point taken for a suicide", engine="slayer"),
    _field("slayer", "kill_in_order", "boolean", "Players must be killed in order", engine="slayer"),
    _field("king", "moving_hill", "boolean", "The hill moves", engine="king"),
    _field("oddball", "random_start", "boolean", "The ball starts at a random place", engine="oddball"),
    _field("oddball", "ball_spawn_delay", "boolean", "The ball respawns after a delay", engine="oddball"),
    _field("oddball", "speed_with_ball", "integer", "The ball carrier's speed", engine="oddball", low=0, high=3),
    _field("oddball", "trait_with_ball", "integer", "The ball carrier's trait", engine="oddball", low=0, high=3),
    _field("oddball", "trait_without_ball", "integer", "The others' trait", engine="oddball", low=0, high=3),
    _field("oddball", "ball_type", "integer", "The kind of ball game", engine="oddball", low=0, high=2),
    _field("oddball", "ball_spawn_count", "integer", "Balls on the map", engine="oddball", low=1, high=16),
    _field("race", "race_type", "integer", "The kind of race", engine="race", low=0, high=2),
    _field("race", "team_scoring", "integer", "How a team's laps are counted", engine="race", low=0, high=2),
]
SCHEMA_BY_KEY = {field["key"]: field for field in SCHEMA}
SCHEMA_BY_ALIAS = {field["alias"]: field for field in SCHEMA if field["alias"]}

GAMETYPE_FILES = {
    "btb_slayer": 'base = "team_slayer"\nname = "BTB Slayer"\n\n'
                  '[universal]\nscore_to_win = 50\nweapon_set = "normal"\n\n[options]\ntime_limit = 15\n'
                  'friendly_fire = "off"\nteam_vehicles = "pc"\n',
    "pistols-only": 'base = "team_slayer"\nname = "Pistols"\n\n'
                    '[universal]\nno_shields = true\nradar = false\nweapon_set = "pistols"\n',
}


def _setting(name, kind, default, comment, env, secret=False, choices=None, low=None, high=None):
    return {"name": name, "type": kind, "value": default, "default": default, "comment": comment, "env": env,
            "pinned": False, "secret": secret, "choices": choices, "min": low, "max": high}


def default_settings():
    """the settings sv_set changes (server_commands.c's settable_settings, in
    port_config.c's order), as GET /api/settings sends them: typed values,
    and choices, min and max where they are known (null otherwise)"""
    return [
        _setting("network.shield_effects", "string", "stock", "Host: the shield effects every player of a network "
                 "game sees, whatever their own.", "HALO_NET_SHIELD_EFFECTS",
                 choices=["stock", "glow", "light_show", "overshield", "glow_overshield", "light_show_overshield",
                          "players"]),
        _setting("network.voice_lobby", "boolean", True, "Hosting: voice chat in the lobby, before and after a game.",
                 "HALO_NET_VOICE_LOBBY"),
        _setting("network.voice_mode", "string", "team_global_enemy_proximity", "Hosting: voice chat in a game.",
                 "HALO_NET_VOICE_MODE", choices=["off", "team_proximity", "team_enemy_proximity", "team_global",
                                                 "team_global_enemy_proximity"]),
        _setting("network.voice_kbps", "integer", 24, "Hosting: the voices' quality, in kilobits a second (8 to 64).",
                 "HALO_NET_VOICE_KBPS", low=8, high=64),
        _setting("network.text_chat", "string", "on", "Hosting: the text chat: \"on\" (links in lines are refused), "
                 "\"links\" or \"off\".", "HALO_NET_TEXT_CHAT", choices=["on", "links", "off"]),
        _setting("network.votekick", "boolean", True, "Hosting: let the players vote to kick a player.",
                 "HALO_NET_VOTEKICK"),
        _setting("network.vote_end_game", "boolean", True, "Hosting: let the players vote to end the game "
                 "(\"!endgame\" in the chat).", "HALO_NET_VOTE_END_GAME"),
        _setting("network.votekick_threshold", "string", "dynamic", "Hosting: the yes votes a vote needs: \"dynamic\" "
                 "(scaled with the players) or \"static\" (network.votekick_percent of them).",
                 "HALO_NET_VOTEKICK_THRESHOLD", choices=["dynamic", "static"]),
        _setting("network.votekick_percent", "integer", 50, "Hosting: with a static threshold, the percentage of the "
                 "players who may vote whose yes passes a vote (1 to 100).", "HALO_NET_VOTEKICK_PERCENT", low=1,
                 high=100),
        _setting("network.votekick_minutes", "integer", 5, "Hosting: the minutes a player must have played on this "
                 "server to start a vote to kick (0 to 60).", "HALO_NET_VOTEKICK_MINUTES", low=0, high=60),
        _setting("network.votekick_ban_minutes", "integer", 30, "Hosting: the minutes a player kicked by a vote "
                 "cannot join again (1 to 1440).", "HALO_NET_VOTEKICK_BAN_MINUTES", low=1, high=1440),
        _setting("network.map_share", "boolean", True, "Hosting: send a player who joins without the game's Custom "
                 "Edition map the map.", "HALO_NET_MAP_SHARE"),
        _setting("network.map_share_upload", "integer", 1024, "Hosting: the most kilobytes a second the host sends "
                 "of maps (16 to 20480).", "HALO_NET_MAP_SHARE_UPLOAD", low=16, high=20480),
        _setting("network.coop_friendly_fire", "string", "on", "Whether the players of an online co-op game hurt "
                 "each other.", "HALO_NET_COOP_FRIENDLY_FIRE", choices=["off", "on", "shields_only", "explosives_only"]),
        _setting("network.coop_ally_friendly_fire", "boolean", True, "Whether the players of an online co-op game "
                 "hurt their AI allies.", "HALO_NET_COOP_ALLY_FRIENDLY_FIRE"),
        _setting("network.coop_team_wipe", "string", "auto", "When every co-op player dies: \"auto\" reverts the "
                 "checkpoint in a private game and only respawns in a public one; \"revert\" or \"respawn\".",
                 "HALO_NET_COOP_TEAM_WIPE", choices=["auto", "revert", "respawn"]),
        _setting("network.coop_player_collisions", "boolean", True, "Whether the players of an online co-op game bump "
                 "into each other.", "HALO_NET_COOP_PLAYER_COLLISIONS"),
        _setting("network.public_address", "string", "", "Hosting: the internet address joiners reach this machine "
                 "at, as host[:port].", "HALO_NET_PUBLIC_ADDRESS"),
        _setting("server.name", "string", "OpenCE Server", "The game's name in the lobby and the server browser: 15 "
                 "characters of printable ASCII.", "HALO_SERVER_NAME"),
        _setting("server.max_players", "integer", 16, "The most players (2 to 127).", "HALO_SERVER_MAX_PLAYERS",
                 low=2, high=127),
        _setting("server.min_players", "integer", 1, "Players in the lobby before it counts down to a game (1 to 16).",
                 "HALO_SERVER_MIN_PLAYERS", low=1, high=16),
        _setting("server.password", "string", "", "The password the server browser asks for (the invite link still "
                 "joins); empty for none. Never logged.", "HALO_SERVER_PASSWORD", secret=True),
        _setting("server.public", "boolean", True, "List the server in everyone's server browser.",
                 "HALO_SERVER_PUBLIC"),
        _setting("server.rotation_fallback", "boolean", False, "With no playable rotation entry, play "
                 "bloodgulch:slayer rather than stop (exit code 3).", "HALO_SERVER_ROTATION_FALLBACK"),
        _setting("server.lobby_seconds", "integer", 15, "Seconds the lobby counts down once min_players are in (0 to "
                 "300; 0 starts at once).", "HALO_SERVER_LOBBY_SECONDS", low=0, high=300),
        _setting("server.postgame_seconds", "integer", 12, "Seconds the scores show after a game (0 to 300).",
                 "HALO_SERVER_POSTGAME_SECONDS", low=0, high=300),
        _setting("server.empty_seconds", "integer", 30, "Seconds a game nobody is in goes on before it ends (0: "
                 "never).", "HALO_SERVER_EMPTY_SECONDS"),
        _setting("server.idle_minutes", "integer", 0, "Minutes a game goes on with no score before it ends (0: "
                 "never).", "HALO_SERVER_IDLE_MINUTES"),
        _setting("server.team_start", "string", "one", "A team game: \"one\" starts with one team's players, "
                 "\"both\" waits for a player on each team, \"skip\" skips team games while one player waits.",
                 "HALO_SERVER_TEAM_START", choices=["one", "both", "skip"]),
        _setting("server.motd", "string", "", "A notice each machine is shown as it joins (127 characters); empty "
                 "for none.", "HALO_SERVER_MOTD"),
        _setting("server.frame_rate", "integer", 60, "Frames a second (30 to 120); the game's ticks stay 30 a second.",
                 "HALO_SERVER_FRAME_RATE", low=30, high=120),
        _setting("server.web_password", "string", "", "The web page's admin password (8 characters or more).",
                 "HALO_SERVER_WEB_PASSWORD", secret=True),
        _setting("server.web_moderator_password", "string", "", "A second password, with the moderator role; empty "
                 "for none.", "HALO_SERVER_WEB_MODERATOR_PASSWORD", secret=True),
        _setting("server.metrics", "boolean", True, "Serve /metrics (Prometheus) on the web port without a password.",
                 "HALO_SERVER_METRICS"),
        _setting("server.log_format", "string", "text", "The standard output's lines: \"text\" or \"json\".",
                 "HALO_SERVER_LOG_FORMAT", choices=["text", "json"]),
        _setting("server.log_addresses", "boolean", False, "Players' internet addresses and hardware ids in the log, "
                 "the events and the API for every role (the owner's console has them always).",
                 "HALO_SERVER_LOG_ADDRESSES"),
        _setting("server.status_file", "string", "", "A file the status (as /api/status) is written to every 2 "
                 "seconds, for a panel without HTTP; empty for none.", "HALO_SERVER_STATUS_FILE"),
        _setting("server.auto_update", "string", "install", "\"off\"; \"check\" logs a newer release; \"install\" "
                 "installs it at once and restarts when empty (a new network version: after the game).",
                 "HALO_SERVER_AUTO_UPDATE", choices=["off", "check", "install"]),
        _setting("server.coop_difficulty", "string", "normal", "Co-op rotation entries' (<level>:coop) difficulty.",
                 "HALO_SERVER_COOP_DIFFICULTY", choices=["easy", "normal", "hard", "impossible"]),
        _setting("server.master_publish", "boolean", True, "Publish a public server to the official master servers.",
                 "HALO_SERVER_MASTER_PUBLISH"),
        _setting("server.region", "string", "", "A region shown in the server browser (eu, na-east...): 16 "
                 "characters; empty for none.", "HALO_SERVER_REGION"),
        _setting("server.master_publish_address", "boolean", False, "Let the master servers give this server's "
                 "address to players.", "HALO_SERVER_MASTER_PUBLISH_ADDRESS"),
        _setting("server.chat_filter", "string", "off", "The words the server keeps out of its chat and its players' "
                 "names: \"off\", \"censor\" (starred) or \"block\" (a line refused, a name refused at the door).",
                 "HALO_SERVER_CHAT_FILTER", choices=["off", "censor", "block"]),
        _setting("server.chat_filter_slurs", "string", "mild", "With server.chat_filter, the slurs kept out.",
                 "HALO_SERVER_CHAT_FILTER_SLURS", choices=["off", "severe", "strong", "mild"]),
        _setting("server.chat_filter_swearing", "string", "off", "With server.chat_filter, the swearing kept out.",
                 "HALO_SERVER_CHAT_FILTER_SWEARING", choices=["off", "severe", "strong", "mild"]),
        _setting("server.chat_filter_insults", "string", "off", "With server.chat_filter, the insults kept out.",
                 "HALO_SERVER_CHAT_FILTER_INSULTS", choices=["off", "severe", "strong", "mild"]),
        _setting("server.vpn_policy", "string", "link", "Machines joining from the address of a VPN, a Tor exit or "
                 "a datacenter: \"link\", refused unless the player shows a Discord account linked at a master "
                 "server this server publishes to; \"block\", refused; \"off\", let in.", "HALO_SERVER_VPN_POLICY",
                 choices=["off", "link", "block"]),
        _setting("server.vpn_fail", "string", "open", "Until a netlist is held: \"open\" lets every machine in, "
                 "\"closed\" refuses those from the internet.", "HALO_SERVER_VPN_FAIL", choices=["open", "closed"]),
        _setting("server.vpn_check_lan", "boolean", False, "Check the LAN's machines' addresses against the netlist "
                 "too (tests).", "HALO_SERVER_VPN_CHECK_LAN"),
        _setting("server.link_min_age_days", "integer", 365, "A machine joining over a VPN with a linked Discord "
                 "account is refused when the account is younger than this many days (0: any age).",
                 "HALO_SERVER_LINK_MIN_AGE_DAYS", low=0, high=3650),
        _setting("server.link_one_seat", "boolean", True, "Refuse a second machine of a linked Discord account "
                 "already in the game.", "HALO_SERVER_LINK_ONE_SEAT"),
        _setting("server.update_check_minutes", "integer", 30, "The minutes between checks for a newer release.",
                 "HALO_SERVER_UPDATE_CHECK_MINUTES", low=5, high=1440),
        _setting("server.update_game_minutes", "integer", 30, "After a new network version's install, the minutes a "
                 "game may go on before it is ended (0: until it ends).", "HALO_SERVER_UPDATE_GAME_MINUTES", low=0,
                 high=240),
        _setting("server.max_machines_per_address", "integer", 0, "The machines of one address that may play at once "
                 "(0: no limit).", "HALO_SERVER_MAX_MACHINES_PER_ADDRESS", low=0, high=128),
        _setting("server.afk_kick_minutes", "integer", 5, "The minutes a player may be away before being kicked "
                 "(0: never).", "HALO_SERVER_AFK_KICK_MINUTES", low=0, high=60),
        _setting("server.afk_kick_when_full", "boolean", False, "Kick players away only while the game is full.",
                 "HALO_SERVER_AFK_KICK_WHEN_FULL"),
        _setting("server.afk_kick_full_minutes", "integer", 3, "With server.afk_kick_when_full, the minutes away "
                 "before a kick.", "HALO_SERVER_AFK_KICK_FULL_MINUTES", low=2, high=60),
        _setting("server.coop_bsp_switch", "string", "majority", "A co-op loading zone switches once most living "
                 "players cross (\"majority\"), or at the first (\"anyone\").", "HALO_SERVER_COOP_BSP_SWITCH",
                 choices=["majority", "anyone"]),
        _setting("server.coop_bsp_switch_cooldown_seconds", "integer", 15, "The seconds after a loading zone's "
                 "switch before another may.", "HALO_SERVER_COOP_BSP_SWITCH_COOLDOWN_SECONDS", low=0, high=300),
        _setting("server.map_vote", "boolean", False, "The players vote for the next game in the lobby.",
                 "HALO_SERVER_MAP_VOTE"),
        _setting("server.map_vote_choices", "integer", 4, "The entries a map vote offers.",
                 "HALO_SERVER_MAP_VOTE_CHOICES", low=2, high=8),
        _setting("server.map_vote_seconds", "integer", 20, "How long a map vote runs.", "HALO_SERVER_MAP_VOTE_SECONDS",
                 low=5, high=120),
    ]


# what the owner's console alone changes (server_commands.c's owner_settings)
OWNER_SETTINGS = {"server.status_file", "server.auto_update", "server.log_addresses"}

PLAYER_NAMES = ["Milenko", "Kestrel", "Wren", "Odin", "Harlow", "Juno", "Vega", "Corvid", "Tamsin", "Brask",
                "Ilse", "Rook", "Sable", "Quill", "Fenwick", "Marlo"]
CHAT_LINES = ["gg", "nice shot", "who has the flag?", "rockets up top", "lag?", "one more", "camping sniper again",
              "rematch after this", "behind you!", "ez"]

DURATION_UNITS = {"": 60, "m": 60, "s": 1, "h": 3600, "d": 86400, "w": 7 * 86400, "y": 365 * 86400}


class CommandError(Exception):
    """a refusal: an HTTP status and a code (before a line runs), or (with
    status None) a command's failure, its message the line it says"""

    def __init__(self, code, message, status=400, lines=None, **extra):
        super().__init__(message)
        self.code = code
        self.message = message
        self.status = status
        self.lines = lines or []
        self.extra = extra


class Failed(Exception):
    """a command that failed: the line it said (the real server's commands
    say why, and the API reads the code from that line)"""

    def __init__(self, line):
        super().__init__(line)
        self.line = line


def now_ms():
    return int(time.time() * 1000)


def json_bytes(value):
    return json.dumps(value, separators=(",", ":"), ensure_ascii=False).encode("utf-8")


def split_words(text):
    """the words of a command line (server_commands.c's split_words): spaces
    apart, a "quoted" word whole (no escapes), 8 at most"""
    words = []
    for match in re.finditer(r'"([^"]*)"?|([^\s"][^ \t]*)', text):
        words.append(match.group(1) if match.group(1) is not None else match.group(2))
        if len(words) == 8:
            break
    return words


def rest_after_words(line, count):
    """the text after the line's first count words (a notice's, a reason's)"""
    position = 0
    for _ in range(count):
        while position < len(line) and line[position] in " \t":
            position += 1
        if position < len(line) and line[position] == '"':
            end = line.find('"', position + 1)
            position = len(line) if end < 0 else end + 1
        else:
            while position < len(line) and line[position] not in " \t":
                position += 1
    return line[position:].lstrip(" \t")


def printable_text(text, size):
    """a text kept to printable ASCII, its surrounding quotes off, size - 1
    characters at most (server_commands.c's printable_text)"""
    text = text.strip("".join(chr(code) for code in range(0x21)) + "\x7f")
    if len(text) > 1 and text[0] == text[-1] == '"':
        text = text[1:-1]
    return "".join(character for character in text if 0x20 <= ord(character) < 0x7F)[:size - 1].rstrip(" ")


def line_text(text):
    """a text the API puts into a line: no quotes, no control characters,
    127 at most (server_http.c's line_text)"""
    return "".join(character for character in text if 0x20 <= ord(character) < 0x7F and character != '"')[:127]


def parse_duration(text):
    """30m, 2h, 7d, 1w, 45s, a bare number of minutes, or forever (0): the
    seconds, -1 if it is none (ten years at most)"""
    if text.lower() in ("forever", "permanent") or text == "0":
        return 0
    match = re.fullmatch(r"(\d+)([smhdwy]?)", text or "")
    if not match:
        return -1
    number = int(match.group(1))
    if number <= 0 or number > 100000:
        return -1
    seconds = number * DURATION_UNITS[match.group(2)]
    return -1 if seconds > 10 * 365 * 86400 else seconds


def rule_value(field, text):
    """a rule's value as the real server keeps it: a whole number (true 1,
    false 0, a choice its index); ValueError with the real server's reason"""
    lowered = text.lower()
    if field["type"] == "integer":
        if len(text) > 9 or not re.fullmatch(r"-?\d+", text):
            raise ValueError(f"{field['key']} takes a whole number, not \"{text}\"")
        value = int(text)
        if value < field["min"] or value > field["max"]:
            raise ValueError(f"{field['key']} takes {field['min']} to {field['max']}, not {value}")
        return value
    if field["type"] == "boolean":
        if lowered in ("true", "yes", "on", "1"):
            return 1
        if lowered in ("false", "no", "off", "0"):
            return 0
        raise ValueError(f"{field['key']} takes true or false, not \"{text}\"")
    choices = field["choices"]
    if lowered in choices:
        return choices.index(lowered)
    if re.fullmatch(r"\d{1,9}", text) and int(text) < len(choices):
        return int(text)
    raise ValueError(f"{field['key']} takes {', '.join(choices[:-1])} or {choices[-1]}, not \"{text}\"")


def find_rule(key):
    key = key.lower()
    return SCHEMA_BY_KEY.get(key) or SCHEMA_BY_ALIAS.get(key)


# ---------- gametype files: the TOML the doc's files hold, a small part of the language

ENGINE_TABLES = ("ctf", "slayer", "king", "oddball", "race")


def parse_toml(text):
    """{table: (its line, {key: (value, line)})} with '' the top level;
    GametypeError(line, message)"""
    tables = {"": (0, {})}
    current = ""
    for number, raw in enumerate(text.split("\n"), 1):
        line = _strip_comment(raw).strip()
        if not line:
            continue
        table = re.fullmatch(r"\[\s*([A-Za-z_][A-Za-z0-9_-]*)\s*\]", line)
        if table:
            current = table.group(1)
            if current in tables:
                raise GametypeError(number, f"[{current}] is defined twice (line {number})")
            tables[current] = (number, {})
            continue
        pair = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_-]*)\s*=\s*(.+)", line)
        if not pair:
            raise GametypeError(number, f"not a key = value line (line {number})")
        key, value_text = pair.group(1), pair.group(2).strip()
        if key in tables[current][1]:
            raise GametypeError(number, f"{key} is defined twice (line {number})")
        tables[current][1][key] = (_toml_value(value_text, number), number)
    return tables


def _strip_comment(line):
    quoted = False
    for index, character in enumerate(line):
        if character == '"' and (index == 0 or line[index - 1] != "\\"):
            quoted = not quoted
        elif character == "#" and not quoted:
            return line[:index]
    return line


def _toml_value(text, number):
    if text in ("true", "false"):
        return text == "true"
    if re.fullmatch(r"[+-]?\d+", text):
        return int(text)
    if re.fullmatch(r'"((?:[^"\\]|\\.)*)"', text):
        return re.sub(r"\\(.)", lambda match: {"n": "\n", "t": "\t"}.get(match.group(1), match.group(1)), text[1:-1])
    if re.fullmatch(r"\[[\d\s,+-]*\]", text):
        return [int(item) for item in text[1:-1].replace(" ", "").split(",") if item]
    raise GametypeError(number, f"{text[:40]} is not a value (line {number})")


class GametypeError(Exception):
    def __init__(self, line, message):
        super().__init__(message)
        self.line = line
        self.message = message


def _toml_text(value):
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, str)):
        return str(value)
    return None


def validate_gametype(text):
    """(base, name, rules) of a gametype file as server_toml.c and
    server_gametypes.c check it (top-level base and name only, a rule by
    its full name in its own table), or GametypeError (line 0: the file's
    whole, as the base's)"""
    if len(text) > 64 * 1024:
        raise GametypeError(0, "the file is larger than 64 KB")
    tables = parse_toml(text)
    base, name, rules, engine_table = "", "", {}, ""
    for key, (value, number) in tables[""][1].items():
        if key == "base" and isinstance(value, str) and len(value) < 64:
            base = value.lower()
        elif key == "name" and isinstance(value, str):
            if len(value) > 11:
                raise GametypeError(number, "name is longer than 11 characters")
            if any(not 0x20 <= ord(character) < 0x7F for character in value):
                raise GametypeError(number, "name has a character that is not printable ASCII")
            name = value
        elif key in ("base", "name"):
            raise GametypeError(number, f"{key} is a string of {'63' if key == 'base' else '11'} characters at most")
        else:
            raise GametypeError(number, f"{key} is not base or name (a rule goes in its table: [universal], "
                                        "[options], [ctf] ... [race])")
    for table, (table_line, keys) in tables.items():
        if table == "":
            continue
        if table not in ("universal", "options") + ENGINE_TABLES:
            raise GametypeError(table_line, f"[{table}] is no table of a gametype file")
        if table in ENGINE_TABLES:
            if engine_table and engine_table != table:
                raise GametypeError(table_line, f"[{engine_table}] and [{table}]: a gametype has one game engine")
            engine_table = table
        for key, (value, number) in keys.items():
            field = SCHEMA_BY_KEY.get(key)
            if not field:
                raise GametypeError(number, f"no rule is named {key} (in [{table}])")
            if field["table"] != table:
                raise GametypeError(number, f"{key} goes in [{field['table']}], not [{table}]")
            value_text = _toml_text(value)
            if value_text is None:
                raise GametypeError(number, f"{key} takes a number, true or false, or a word")
            try:
                rules[key] = rule_value(field, value_text)
            except ValueError as failure:
                raise GametypeError(number, str(failure)) from None
    if not base:
        raise GametypeError(1, "there is no base = \"<gametype>\" (slayer, team_slayer, ctf, ...)")
    base = GAMETYPE_ALIASES.get(base, base)
    if base not in BUILTINS:
        raise GametypeError(0, f"base = \"{base}\" is no built-in gametype")
    engine = BUILTINS[base][0]
    if engine_table and engine_table != engine:
        raise GametypeError(0, f"[{engine_table}] is not its base's game engine ({engine})")
    return base, name, rules


def gametype_file_name_valid(name):
    return re.fullmatch(r"[a-z0-9_-]{1,32}", name) is not None


# ---------- the simulated server


class Player:
    def __init__(self, slot, name, machine, team):
        self.slot = slot
        self.name = name
        self.machine = machine
        self.team = team
        self.score = self.kills = self.deaths = self.assists = 0
        self.ping = random.randint(20, 140)
        self.joined = int(time.time())
        self.muted_until = None
        self.muted = False
        self.address = f"203.0.113.{10 + machine}"
        self.hwid = hashlib.sha256(name.encode()).hexdigest()[:32]
        # (its Discord account: a verified one's, a pass that checked, for every other machine; else what it says)
        self.discord_id = str(80351110224678912 + machine)
        self.discord_name = name.lower()
        self.discord_verified = machine % 2 == 0
        # (its machine's build, as its game tells the host)
        self.client = {"build": 282, "flavor": "release", "version": "OpenCE Linux | build 282 (release)",
                       "repository": "OpenCommunityEdition/OpenCE",
                       "mods": ["trollface 1.0"] if machine % 2 else []}

    def json(self, addresses, playing=True):
        """as server_status.c's server_status_player: the ping null until the
        player is in a game, its machine's build (client), the address and
        hardware id for the owner (or with server.log_addresses)"""
        value = {"slot": self.slot, "name": self.name, "team": self.team, "score": self.score, "kills": self.kills,
                 "deaths": self.deaths, "assists": self.assists, "ping": self.ping if playing else None,
                 "machine": self.machine, "joined": self.joined, "muted": self.muted, "client": self.client}
        if addresses:
            value.update(address=self.address, hwid=self.hwid, discord_id=self.discord_id,
                         discord_name=self.discord_name, discord_verified=self.discord_verified)
        return value


class Session:
    def __init__(self, role, by):
        self.token = secrets.token_hex(32)
        self.csrf = secrets.token_hex(16)
        self.role = role
        self.by = by
        self.created = time.time()
        self.used = self.created

    @property
    def expires(self):
        """(as the real server's: the session's lifetime's end; it also ends
        after SESSION_IDLE without a request)"""
        return int(self.created + SESSION_LIFETIME)

    def valid(self, at):
        return at < self.created + SESSION_LIFETIME and at < self.used + SESSION_IDLE


# the server's commands (server_commands.c's table): name, alias, role,
# usage, description
COMMANDS = [
    ("help", None, "viewer", "[command]", "the commands, or one's use"),
    ("sv_status", None, "viewer", "[--json]", "the server and its game"),
    ("sv_players", None, "viewer", "[--json]", "the players"),
    ("sv_rotation", "sv_mapcycle", "viewer", "", "the rotation"),
    ("sv_maps", None, "viewer", "", "the multiplayer maps there are"),
    ("sv_gametypes", None, "viewer", "", "the gametypes there are"),
    ("sv_bans", "sv_banlist", "admin", "", "bans.txt's bans"),
    ("sv_settings", None, "admin", "", "the settings sv_set changes"),
    ("sv_version", None, "viewer", "", "the build and its network version"),
    ("sv_log", None, "viewer", "[lines]", "the log's last lines"),
    ("sv_masters", None, "viewer", "", "the master servers and their answers"),
    ("sv_publisher", None, "viewer", "", "the publisher key's id and public key"),
    ("say", "sv_say", "moderator", "<text>", "a notice on every machine"),
    ("tell", None, "moderator", "<player> <text>", "a notice on a player's machine"),
    ("kick", "sv_kick", "moderator", "<player>", "drops a player's machine (it may join again)"),
    ("ban", "sv_ban", "moderator", "<player> [30m|2h|7d|forever] [reason]",
     "drops a player's machine and bans its address and hardware id (a moderator's: a day at most)"),
    ("unban", "sv_unban", "admin", "<number|address|hardware id|discord:id>", "takes a ban out of bans.txt"),
    ("sv_ban_address", None, "admin", "<address|hardware id|discord:id> [30m|2h|7d|forever] [reason]",
     "bans an address, hardware id or verified Discord account, its player here or not"),
    ("sv_vpn", None, "viewer", "", "the VPN policy, the netlists held and the joins refused"),
    ("sv_vpn_check", None, "moderator", "<address>", "whether an IPv4 address is refused as a VPN's"),
    ("sv_vpn_fetch", None, "admin", "", "fetches the master servers' netlists now"),
    ("mute", None, "moderator", "<player> [minutes]", "mutes a player's chat and voice for everyone"),
    ("unmute", None, "moderator", "<player>", "unmutes a player"),
    ("sv_map", None, "moderator", "<map> [gametype]", "plays that game now (the rotation goes on after)"),
    ("sv_map_next", "sv_next", "moderator", "", "ends the game, no scores, and plays the next"),
    ("sv_end_game", None, "moderator", "", "ends the game: its scores, then the next"),
    ("sv_rotation_set", None, "admin", "<entries>", "replaces the rotation"),
    ("sv_rotation_add", "sv_mapcycle_add", "admin", "<entry>", "adds an entry to the rotation"),
    ("sv_rotation_del", "sv_mapcycle_del", "admin", "<number>", "takes an entry out of the rotation"),
    ("sv_rotation_begin", "sv_mapcycle_begin", "admin", "[number]", "the rotation's next game is that entry"),
    ("sv_set", None, "admin", "<setting> <value>", "changes a setting (sv_settings lists them)"),
    ("sv_name", None, "admin", "<name>", "the server's name (15 characters)"),
    ("sv_maxplayers", None, "admin", "<2-127>", "the most players"),
    ("sv_password", None, "admin", "[password]", "the server browser's password (none without one)"),
    ("sv_public", None, "admin", "<0|1>", "listed in the server browser, or by invite only"),
    ("sv_motd", None, "admin", "<text>", "the notice each machine sees as it joins"),
    ("sv_reload", None, "admin", "", "reads the server's settings and the rotation again"),
    ("sv_stop", None, "owner", "[now|game|empty|cancel] [notice]",
     "stops the server now, once the game is over, or once it is empty"),
    ("sv_restart", None, "owner", "[now|game|empty|cancel] [notice]", "as sv_stop, with exit code 5 (to be started again)"),
    ("quit", "exit", "owner", "", "stops the server"),
]
SETTING_ALIASES = {"sv_name": "server.name", "sv_maxplayers": "server.max_players", "sv_password": "server.password",
                   "sv_public": "server.public", "sv_motd": "server.motd"}


def line_is_secret(line):
    """a password's command, never logged (server_console.c's)"""
    words = split_words(line)
    if words and words[0].lower() == "sv_password":
        return True
    return len(words) >= 2 and words[0].lower() == "sv_set" and ("password" in words[1] or
                                                                 "publisher_key" in words[1])


def find_command(word):
    word = word.lower()
    return next((command for command in COMMANDS if word in (command[0], command[1])), None)


class Sim:
    def __init__(self, options):
        self.options = options
        self.lock = threading.RLock()
        self.started = time.time()
        self.random = random.Random(options.seed)
        random.seed(options.seed)
        self.settings = default_settings()
        self.setting("server.web_password")["value"] = options.password
        self.setting("server.web_moderator_password")["value"] = options.moderator_password or ""
        # (one setting its environment variable sets, as a container's would)
        self.setting("server.frame_rate")["pinned"] = True
        self.sessions = {}
        self.addresses = {}
        self.checks = deque()
        self.say_minute = 0
        self.say_count = 0
        self.seq = 0
        self.events = deque(maxlen=EVENT_RING)
        self.log_n = 0
        self.log_lines = deque(maxlen=LOG_RING)
        self.connections = []
        self.players = {}
        self.next_machine = 1
        self.names = deque(PLAYER_NAMES)
        self.gametype_files = dict(GAMETYPE_FILES)
        self.bans = [
            {"when": "2026-10-19 18:00:00", "ip": "198.51.100.23", "hwid": "4f1c9a0e7b2d4c6e8a1f3b5d7c9e0a2b",
             "discord_id": "", "discord_verified": False, "players": "Griefer", "reason": "team killing",
             "until": None},
            # (a timed ban that has ended: bans.txt keeps it, the server lets its machine in)
            {"when": "2026-10-18 09:30:00", "ip": "198.51.100.40", "hwid": "", "discord_id": "",
             "discord_verified": False, "players": "Spammer", "reason": "chat spam (by the moderator)",
             "until": int(time.time()) - 3600},
        ]
        self.vpn_refused = {reason: 0 for reason in VPN_REASONS}
        self.rotation = []
        self.rotation_index = 0
        self.rotation_text = ("bloodgulch:ctf, sidewinder:team_slayer+score=50+time=15, prisoner:slayer, "
                              "beavercreek:ctf+moving_hill=true, custom_maps\\hugeass:btb_slayer")
        self.rotation = self.parse_rotation(self.rotation_text)[0]
        self.games_hosted = 0
        self.api_requests = 0
        self.state = "starting"
        self.state_ticks = 0
        self.countdown = -1
        self.game_seconds = 0
        self.team_scores = [0, 0]
        self.current = self.rotation[0]
        self.interposed = None
        self.score_changed = False
        self.masters = [
            {"url": "https://master1.example.org", "listed": True, "probe": "ok", "code": "", "message": "",
             "updated": int(time.time())},
            {"url": "https://master2.example.net", "listed": False, "probe": "pending", "code": "", "message": "",
             "updated": int(time.time())},
        ]
        self.invite = "halo://join/" + secrets.token_hex(32)
        self.script_tick = 0
        # a stop that waits (sv_stop, sv_restart: game or empty): {"after", "restart", "since"}, or None
        self.drain = None

    # ----- the parts

    def setting(self, name):
        return next((setting for setting in self.settings if setting["name"] == name), None)

    def value(self, name):
        return self.setting(name)["value"]

    def owner_password(self):
        return getattr(self.options, "owner_password", "") or ""

    def instance(self):
        """server.instance as the masters get it: else a slug of the server's name"""
        name = getattr(self.options, "instance", "") or self.value("server.name")
        slug = re.sub(r"-+", "-", re.sub(r"[ .]", "-", "".join(character for character in name.lower()
                                                                  if character.isalnum() or character in " .-_")))
        return slug.strip("-_")[:32].strip("-") or "main"

    def identity(self):
        """the status's and server_started's identity (server_status.c's server_status_identity)"""
        return {"pid": os.getpid(), "started": int(self.started), "instance": self.instance(),
                "region": self.value("server.region"), "tunnel_port": 2302, "publisher_key_id": PUBLISHER_KEY_ID,
                "web_port": getattr(self.options, "port", 0)}

    def addresses_for(self, role):
        return ROLES.index(role) >= ROLES.index("owner") or self.value("server.log_addresses") or \
            self.options.log_addresses

    def log(self, level, cat, text):
        """a line of the log, and its event: a log event has the log's own
        sequence (seq and n are the line's number) and takes no number of the
        events'; chat's lines are the chat group's"""
        with self.lock:
            self.log_n += 1
            line = {"n": self.log_n, "time": now_ms(), "level": level, "cat": cat, "text": text}
            self.log_lines.append(line)
            if self.options.verbose:
                stamp = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime(line["time"] / 1000))
                print(f"{stamp} {level.upper()} {cat}: {text}", flush=True)
            event = {"type": "event", "seq": line["n"], "time": line["time"], "event": "log", "n": line["n"],
                     "level": level, "cat": cat, "text": text}
            for connection in list(self.connections):
                connection.offer("chat" if cat == "chat" else "log", event)

    def emit(self, _event, _group, _addressed=None, **fields):
        """an event to the ring and to every subscriber; _addressed, the fields
        the readers who see players' addresses get besides (the owner, or
        every role with server.log_addresses)"""
        with self.lock:
            self.seq += 1
            event = {"type": "event", "seq": self.seq, "time": now_ms(), "event": _event}
            event.update(fields)
            addressed = dict(event, **_addressed) if _addressed else None
            self.events.append((_group, event, addressed))
            for connection in list(self.connections):
                connection.offer(_group, addressed if addressed and self.addresses_for(connection.role or "viewer")
                                 else event)
            return event

    def playing(self):
        return self.state == "playing"

    def players_json(self, role):
        addresses = self.addresses_for(role)
        return [player.json(addresses, self.playing()) for player in sorted(self.players.values(), key=lambda p: p.slot)]

    def entry_json(self, entry):
        return {"text": entry["text"], "map": entry["map"], "gametype": entry["gametype"], "rules": entry["rules"],
                "rejected": entry["rejected"]}

    def next_entry(self):
        playable = [index for index, entry in enumerate(self.rotation) if not entry["rejected"]]
        if not playable:
            return None
        later = [index for index in playable if index > self.rotation_index]
        return (later or playable)[0]

    def team_scores_json(self):
        return list(self.team_scores) if self.current["teams"] else None

    def status(self, role):
        entry = self.current
        next_index = self.next_entry()
        admin = ROLES.index(role) >= ROLES.index("admin")
        frame = round(3.5 + self.random.random() * 2, 1)
        mean = round(1 + self.random.random() * 0.6, 2)
        return {
            "ok": True, "time": now_ms(), "name": self.value("server.name"), "state": self.state,
            "build": BUILD, "network_version": NETWORK_VERSION, "uptime": int(time.time() - self.started),
            "games_hosted": self.games_hosted, "map": entry["map"], "level": entry["level"],
            "gametype": entry["gametype"], "variant": entry["variant"], "teams": entry["teams"],
            "players": len(self.players), "max_players": self.value("server.max_players"),
            "machines": len({player.machine for player in self.players.values()}),
            "countdown": self.countdown, "game_seconds": self.game_seconds,
            "team_scores": self.team_scores_json(),
            "listing": {"online": True, "public": bool(self.value("server.public")),
                        "password": bool(self.value("server.password")), "invite": self.invite if admin else ""},
            "rotation": {"index": self.rotation_index, "count": len(self.rotation),
                         "next": self.entry_json(self.rotation[next_index]) if next_index is not None else None},
            "frame_ms_max": frame, "frame_ms_avg": mean, "tick_ms_avg": mean,
            "masters": [dict(master) for master in self.masters],
            "update": {"available": False, "build": ""},
            "drain": dict(self.drain) if self.drain else None,
            **self.identity(),
            "capabilities": list(CAPABILITIES),
        }

    # ----- the rotation (server_rotation.c's grammar)

    def find_map(self, kind, name):
        """(level, kind) of a map there is, or None"""
        if kind in ("name", "path") and name in dict(MAPS):
            return f"levels\\test\\{name}\\{name}", "xbox"
        if kind in ("name", "custom") and name in dict(CUSTOM_MAPS):
            return f"custom_maps\\{name}", "custom"
        return None

    def parse_entry(self, text):
        """an entry of the rotation, its rejected the reason it cannot be
        played; None for an empty one or a comment"""
        text = text.strip(" \t\r\n")
        if not text or text.startswith("#"):
            return None
        entry = {"text": text, "map": "", "gametype": "slayer", "rules": {}, "rejected": None, "level": "",
                 "engine_base": "slayer", "teams": False, "variant": ""}
        if len(text) > ENTRY_TEXT:
            entry["text"] = text[:ENTRY_TEXT - 3] + "..."
            entry["rejected"] = f"longer than {ENTRY_TEXT} characters"
            return entry
        if any(not 0x20 <= ord(character) < 0x7F for character in text):
            entry["rejected"] = "has a character that is not printable ASCII"
            return entry
        map_text, colon, after = text.partition(":")
        gametype_text, _, rules_text = after.partition("+")
        has_rules = "+" in after
        map_text = map_text.strip().replace("/", "\\")
        plain = re.compile(r"[A-Za-z0-9_\-][A-Za-z0-9_.\-]*")
        if map_text[:12].lower() == "custom_maps\\":
            kind, name = "custom", map_text[12:]
            if not plain.fullmatch(name):
                entry["rejected"] = f"\"{name}\" is not a map's name"
                return entry
        elif "\\" in map_text:
            kind, name = "path", map_text.rsplit("\\", 1)[1]
            if map_text.startswith("\\") or any(not part or set(part) == {"."} or not re.fullmatch(r"[A-Za-z0-9_.\-]+", part)
                                                for part in map_text.split("\\")):
                entry["rejected"] = f"\"{map_text}\" is not a level's path"
                return entry
        else:
            kind, name = "name", map_text
            if not plain.fullmatch(name):
                entry["rejected"] = f"\"{name}\" is not a map's name"
                return entry
        name = name.lower()
        entry["map"] = name
        if name == "ui":
            entry["rejected"] = "ui is the main menu's map"
            return entry
        if colon:
            gametype = gametype_text.strip()
            if not gametype or len(gametype) >= 32 or not plain.fullmatch(gametype[5:] if gametype.startswith("file=")
                                                                          else gametype):
                entry["rejected"] = f"\"{gametype}\" is not a gametype's name"
                return entry
            entry["gametype"] = gametype.lower()
        for rule in (rules_text.split("+") if has_rules else []):
            key, equals, value = rule.partition("=")
            key, value = key.strip(), value.strip()
            if not equals:
                entry["rejected"] = f"the rule \"{rule.strip()}\" has no value (key=value)"
                return entry
            field = find_rule(key)
            if not field:
                entry["rejected"] = f"no rule is named \"{key}\""
                return entry
            try:
                entry["rules"][field["key"]] = rule_value(field, value)
            except ValueError as failure:
                entry["rejected"] = str(failure)
                return entry
        found = self.find_map(kind, name)
        if not found:
            entry["rejected"] = {
                "name": f"there is no map {name} (maps/{name}.map, custom_maps/{name}.map)",
                "custom": f"there is no Custom Edition map {name} (custom_maps/{name}.map)",
                "path": f"there is no map file maps/{name}.map for {map_text}"}[kind]
            return entry
        entry["level"] = found[0] if kind != "path" else map_text
        gametype = GAMETYPE_ALIASES.get(entry["gametype"], entry["gametype"])
        if gametype in BUILTINS:
            base, variant = gametype, BUILTINS[gametype][2]
        elif gametype.startswith("file="):
            entry["rejected"] = f"gametypes/{gametype[5:]}.gametype is missing, or not a saved gametype"
            return entry
        elif gametype in self.gametype_files:
            try:
                base, variant, _ = validate_gametype(self.gametype_files[gametype])
            except GametypeError as failure:
                entry["rejected"] = f"gametypes/{gametype}.toml" + (f" line {failure.line}" if failure.line else "") + \
                    f": {failure.message}"
                return entry
            variant = variant or BUILTINS[base][2]
        else:
            entry["rejected"] = f"no gametype is named {gametype} (a built-in's, OpenCE's, or gametypes/{gametype}.toml)"
            return entry
        engine, teams, _ = BUILTINS[base]
        entry["engine_base"], entry["variant"], entry["teams"] = base, variant, teams
        for key, value in entry["rules"].items():
            field = SCHEMA_BY_KEY[key]
            if field["engine"] not in ("all", engine):
                entry["rejected"] = f"{key} is a {field['table']} gametype's rule, and this one is {engine}"
                return entry
            if key == "teams":
                entry["teams"] = bool(value)
        return entry

    def parse_rotation(self, text):
        """(entries, truncated): separated by commas, ';' or lines, a '#'
        comment to its line's end; 4096 characters and 64 entries at most"""
        truncated = False
        if len(text) > ROTATION_TEXT:
            cut = ROTATION_TEXT
            while cut > 0 and text[cut] not in ",\n;":
                cut -= 1
            text, truncated = text[:cut], True
        entries = []
        start = 0
        while start < len(text):
            end = start
            while end < len(text) and text[end] in " \t\r":
                end += 1
            # (a comment runs to its line's end, commas and all)
            if end < len(text) and text[end] == "#":
                newline = text.find("\n", end)
                start = len(text) if newline < 0 else newline + 1
                continue
            while end < len(text) and text[end] not in ",\n;":
                end += 1
            entry = self.parse_entry(text[start:end])
            if entry is not None:
                if len(entries) >= 64:
                    truncated = True
                else:
                    entries.append(entry)
            start = end + 1
        return entries, truncated

    def set_rotation(self, text):
        """the rotation from a text: (True, []) or (False, rejected) with
        nothing changed"""
        entries, truncated = self.parse_rotation(text)
        rejected = [{"index": index, "entry": entry["text"], "reason": entry["rejected"]}
                    for index, entry in enumerate(entries) if entry["rejected"]]
        if rejected or not entries or truncated:
            return False, rejected
        self.rotation = entries
        self.rotation_text = ", ".join(entry["text"] for entry in entries)
        self.rotation_index = len(entries) - 1
        return True, []

    def rotation_json(self):
        next_index = self.next_entry()
        return {"ok": True, "index": self.rotation_index, "entries": [self.entry_json(e) for e in self.rotation],
                "next": self.entry_json(self.rotation[next_index]) if next_index is not None else None}

    # ----- players

    def free_slot(self):
        slot = 0
        while slot in self.players:
            slot += 1
        return slot

    def add_player(self, name=None):
        # (a drain refuses new machines: the game is closed)
        if len(self.players) >= self.value("server.max_players") or self.drain:
            return None
        if name is None:
            taken = {player.name for player in self.players.values()}
            free = [candidate for candidate in self.names if candidate not in taken]
            if not free:
                return None
            name = free[0]
            self.names.remove(name)
            self.names.append(name)
        slot = self.free_slot()
        teams = self.current["teams"]
        red = sum(1 for player in self.players.values() if player.team == 0)
        blue = sum(1 for player in self.players.values() if player.team == 1)
        player = Player(slot, name, self.next_machine, (0 if red <= blue else 1) if teams else -1)
        self.next_machine += 1
        self.players[slot] = player
        self.log("info", "player", f"{name} joined (slot {slot}, machine {player.machine})")
        self.emit("player_joined", "player", {"address": player.address, "hwid": player.hwid,
                                              "discord_id": player.discord_id, "discord_name": player.discord_name,
                                              "discord_verified": player.discord_verified},
                  slot=slot, name=name, team=player.team, machine=player.machine)
        return player

    def remove_player(self, player, reason):
        """a player gone: reason left, kicked or banned"""
        self.players.pop(player.slot, None)
        self.log("info", "player", f"{player.name} left (slot {player.slot})")
        self.emit("player_left", "player", {"address": player.address, "hwid": player.hwid,
                                            "discord_id": player.discord_id, "discord_name": player.discord_name,
                                            "discord_verified": player.discord_verified},
                  slot=player.slot, name=player.name, reason=reason)

    def find_player(self, command, text):
        """a player by slot or the start of their name, or Failed"""
        if not text:
            raise Failed(f"{command}: give a player's slot (sv_players) or name")
        if re.fullmatch(r"\d+", text):
            player = self.players.get(int(text))
            if not player:
                raise Failed(f"{command}: no player in slot {int(text)} (sv_players lists them)")
            return player
        found = [player for player in self.players.values() if player.name.lower().startswith(text.lower())]
        exact = [player for player in found if len(player.name) == len(text)]
        if len(exact) == 1:
            return exact[0]
        if len(exact) > 1:
            raise Failed(f"{command}: {len(exact)} players are named \"{text}\": give a slot (sv_players)")
        if len(found) > 1:
            raise Failed(f"{command}: {len(found)} players' names begin with \"{text}\": give more of it, or a slot")
        if not found:
            raise Failed(f"{command}: no player's name begins with \"{text}\"")
        return found[0]

    # ----- the game, one after another

    def game_event(self, name, reason=None, variant=False):
        """server_director.c's server_game_event: the map, gametype, teams,
        team scores and the players as viewers see them"""
        entry = self.current
        fields = {"map": entry["map"], "gametype": entry["gametype"]}
        if variant:
            fields["variant"] = entry["variant"]
        fields["teams"] = entry["teams"]
        if reason:
            fields.update(reason=reason, duration=self.game_seconds)
        if entry["teams"]:
            fields["team_scores"] = list(self.team_scores)
        fields["players"] = [player.json(False, self.playing())
                             for player in sorted(self.players.values(), key=lambda p: p.slot)]
        self.emit(name, "game", **fields)

    def open_lobby(self, entry):
        self.current = entry
        self.state = "lobby"
        self.state_ticks = 0
        self.game_seconds = 0
        self.team_scores = [0, 0]
        self.countdown = -1
        for player in self.players.values():
            player.score = player.kills = player.deaths = player.assists = 0
            if entry["teams"] and player.team < 0:
                player.team = player.slot % 2
            elif not entry["teams"]:
                player.team = -1
        self.log("info", "lobby", f"next game: {entry['gametype']} on {entry['map']}")
        self.emit("lobby_opened", "game", map=entry["map"], gametype=entry["gametype"], variant=entry["variant"],
                  teams=entry["teams"], index=self.rotation_index)

    def start_loading(self):
        self.state = "loading"
        self.state_ticks = 0
        self.countdown = -1

    def start_game(self):
        self.state = "playing"
        self.state_ticks = 0
        self.games_hosted += 1
        entry = self.current
        self.log("info", "game", f"started: {entry['variant']} on {entry['map']}, {len(self.players)} players")
        self.game_event("game_started", variant=True)

    def end_game(self, reason, postgame=True):
        self.log("info", "game", f"ended ({reason}) after {self.game_seconds} s")
        self.game_event("game_ended", reason)
        seconds = min(self.value("server.postgame_seconds"), 10) if postgame else 0
        self.emit("postgame", "game", seconds=seconds)
        if not postgame:
            self.advance()
            return
        self.state = "postgame"
        self.state_ticks = 0
        self.countdown = -1

    def advance(self):
        if self.interposed is not None:
            entry, self.interposed = self.interposed, None
            self.open_lobby(entry)
            self.start_loading()
            return
        index = self.next_entry()
        if index is None:
            self.state = "stopping"
            return
        self.rotation_index = index
        self.open_lobby(self.rotation[index])

    def step(self):
        """one simulated second"""
        self.state_ticks += 1
        for player in self.players.values():
            player.ping = max(8, min(400, player.ping + self.random.randint(-6, 6)))
            if player.muted_until and time.time() >= player.muted_until:
                player.muted, player.muted_until = False, None
        if self.state == "starting":
            self.state = "waiting"
            self.emit("server_started", "server", name=self.value("server.name"), build=BUILD,
                      network_version=NETWORK_VERSION, **self.identity(),
                      rotation=[entry["text"] for entry in self.rotation])
            self.log("info", "server", f"ready (tunnel 2302/udp, web 127.0.0.1:{self.options.port}, listing public)")
            for entry in self.rotation:
                if entry["rejected"]:
                    self.log("warn", "server", f"rotation: {entry['text']} left out: {entry['rejected']}")
            # (a machine from a VPN's address with no Discord account, refused at the door: network_link.c)
            self.refuse_vpn("Hidden", "198.51.100.77", "vpn_link_required")
            self.rotation_index = 0
            if self.options.still:
                for _ in range(5):
                    self.add_player()
                self.open_lobby(self.rotation[self.rotation_index])
                self.start_game()
            else:
                for _ in range(3):
                    self.add_player()
                self.open_lobby(self.rotation[self.rotation_index])
            return
        if self.drain and self.drain_due():
            self.stop(5 if self.drain["restart"] else 0, "drained")
            return
        if self.state == "lobby":
            minimum = self.value("server.min_players")
            if len(self.players) >= minimum and self.countdown < 0:
                self.countdown = min(self.value("server.lobby_seconds"), 10)
                self.emit("countdown", "game", seconds=self.countdown)
            elif len(self.players) < minimum and self.countdown >= 0:
                self.countdown = -1
                self.emit("countdown", "game", seconds=-1)
            elif self.countdown > 0:
                self.countdown -= 1
                if self.countdown % 5 == 0:
                    self.emit("countdown", "game", seconds=self.countdown)
            elif self.countdown == 0:
                self.start_loading()
            if not self.options.still and self.state_ticks == 2 and len(self.players) < 6:
                self.add_player()
        elif self.state == "loading":
            if self.state_ticks >= 2:
                self.start_game()
        elif self.state == "playing":
            self.game_seconds += 1
            if not self.options.still:
                self.script()
            if self.score_changed:
                self.score_changed = False
                fields = {"team_scores": list(self.team_scores)} if self.current["teams"] else {}
                self.emit("score", "game", players=self.players_json("viewer"), **fields)
        elif self.state == "postgame":
            if self.state_ticks >= 10:
                self.advance()

    def script(self):
        """what the players do: kills, chat, joins and leaves, a map shared,
        a speed hack and its kick, and the end of the game"""
        self.script_tick += 1
        tick = self.game_seconds
        players = list(self.players.values())
        if len(players) >= 2 and self.random.random() < 0.6:
            killer, victim = self.random.sample(players, 2)
            killer.kills += 1
            killer.score += 1
            victim.deaths += 1
            if killer.team >= 0:
                self.team_scores[killer.team] += 1
            self.emit("kill", "game", killer=killer.slot, victim=victim.slot,
                      weapon=self.random.choice(["assault rifle", "pistol", "sniper rifle", "rocket launcher", None]),
                      friendly_fire=False)
            self.score_changed = True
        if players and tick % 7 == 3:
            speaker = self.random.choice(players)
            text = self.random.choice(CHAT_LINES)
            self.log("info", "chat", f"{speaker.name}: {text}")
            self.emit("chat", "chat", channel=self.random.choice(["everyone", "everyone", "team"]), slot=speaker.slot,
                      name=speaker.name, text=text)
        if tick in (10, 30):
            self.add_player()
        if self.current["level"].startswith("custom_maps") and tick in (11, 12, 13):
            state, progress = {11: ("asked", 0), 12: ("downloading", 50), 13: ("downloaded", 100)}[tick]
            self.emit("map_share", "player", machine=self.next_machine - 1, map=self.current["map"], state=state,
                      progress=progress)
        if tick == 35 and players:
            cheater = players[-1]
            self.log("warn", "moderation", f"speed hack: machine {cheater.machine} ({cheater.name}) at 1.40x")
            self.emit("speed_hack", "moderation", machine=cheater.machine, names=cheater.name, rate=1.4,
                      ahead_ticks=42, action="warned")
        if tick == 45 and players:
            cheater = players[-1]
            self.log("info", "moderation", f"{cheater.name} kicked (speed hack)")
            self.emit("kick", "moderation", name=cheater.name, by="owner", reason="speed hack")
            self.remove_player(cheater, "kicked")
        if tick == 52 and len(players) > 3:
            self.remove_player(players[0], "left")
        if tick == 20 and self.masters[1]["probe"] == "pending":
            self.masters[1].update(probe="failed", code="unreachable", updated=int(time.time()),
                                   message="the probe got no answer on 2302/udp")
            master = self.masters[1]
            self.log("warn", "master", f"{master['url']}: {master['message']}")
            self.emit("master", "master", url=master["url"], listed=False, probe="failed", code="unreachable",
                      message=master["message"])
        if self.script_tick % 60 == 0:
            self.log("info", "health", f"players={len(self.players)} frame_ms_max=4.1 frame_ms_avg=1.20 cpu=2.1% "
                     "rss=96MB")
        limit = self.current["rules"].get("score_to_win") or 25
        best = max([player.score for player in self.players.values()] + self.team_scores + [0])
        if best >= limit:
            self.end_game("score")
        elif tick >= 90:
            self.end_game("time")

    # ----- commands

    def run(self, line, role, by):
        """a command line as server_commands.c runs it: (succeeded, its
        lines); a failure's first line says why"""
        with self.lock:
            self.log("info", "console", f"{by}: " + ("(a password's command)" if line_is_secret(line) else line))
            try:
                ok, lines = True, self._run(line.lstrip(" \t"), role, by)
            except Failed as failure:
                ok, lines = False, [failure.line]
            if self.options.big_replies and ok:
                lines = lines + [f"{index:07d} " + "x" * 120 for index in range(9000)]
            return ok, lines

    def _run(self, line, role, by):
        words = split_words(line)
        if not words:
            raise Failed("give a command (help lists them)")
        command = find_command(words[0])
        if not command:
            if ROLES.index(role) < ROLES.index("owner"):
                raise Failed(f"{words[0]}: not a command of the server's, and the game's console needs the owner role "
                             "(the server's console)")
            return [f"(simulated: the game's console runs {words[0]})"]
        if ROLES.index(role) < ROLES.index(command[2]):
            raise Failed(f"{command[0]}: needs the {command[2]} role")
        handler = getattr(self, "c_" + command[0])
        return handler(line, words, role)

    def moderation_event(self, event, name, role, reason=None, until=None):
        fields = {"name": name, "by": role}
        if reason:
            fields["reason"] = reason
        if until:
            fields["until"] = until
        self.emit(event, "moderation", **fields)

    def c_help(self, line, words, role):
        if len(words) >= 2:
            command = find_command(words[1])
            if not command:
                raise Failed(f"help: no command is named {words[1]}")
            lines = [f"{command[0]} {command[3]}", f"  {command[4]} ({command[2]})"]
            if command[1]:
                lines.append(f"  also {command[1]}")
            return lines
        return ["the server's commands (help <command> says more; the game's console commands run too):"] + \
            [f"  {name:<18} {description}" for name, _, needed, _, description in COMMANDS
             if ROLES.index(needed) <= ROLES.index(role)]

    def c_sv_status(self, line, words, role):
        status = self.status(role)
        if len(words) >= 2 and words[1] == "--json":
            return [json.dumps(status, separators=(",", ":"))]
        uptime = status["uptime"]
        lines = [f"{status['name']}: {status['state']}, up {uptime // 3600}h{uptime // 60 % 60:02d}m, "
                 f"{self.games_hosted} game{'' if self.games_hosted == 1 else 's'} played; OpenCE {BUILD}, network "
                 f"version {NETWORK_VERSION}",
                 f"game: {self.current['gametype']} on {self.current['map']}, {len(self.players)} of "
                 f"{self.value('server.max_players')} players"]
        entry = status["rotation"]["next"]
        if entry:
            lines.append(f"next: {entry['gametype']} on {entry['map']}")
        lines.append(f"listing: {'public' if status['listing']['public'] else 'by invite only'}"
                     f"{', with a password' if status['listing']['password'] else ''}")
        if ROLES.index(role) >= ROLES.index("admin"):
            lines.append(f"invite: {self.invite}")
        return lines

    def c_sv_players(self, line, words, role):
        players = self.players_json(role)
        if len(words) >= 2 and words[1] == "--json":
            return [json.dumps({"players": players}, separators=(",", ":"))]
        lines = []
        for p in players:
            ping = "-0" if p["ping"] is None else str(p["ping"])
            head = f"{p['slot']:3} {p['name']:<12} team {p['team']}  machine {p['machine']}  ping {ping}"
            if "address" in p:
                head += f"  {p['address']}  hwid {p['hwid'] or 'none'}"
            lines.append(head)
            if self.playing():
                lines.append(f"    score {p['score']}  kills {p['kills']}  deaths {p['deaths']}  assists {p['assists']}")
        return lines or ["no players"]

    def c_sv_rotation(self, line, words, role):
        lines = []
        for index, entry in enumerate(self.rotation):
            lines.append(f"{'>' if index == self.rotation_index else ' '}{index:2} {entry['text']}"
                         + (f"  (left out: {entry['rejected']}" if entry["rejected"] else ""))
            if entry["rejected"]:
                lines.append("      )")
        return lines or ["the rotation is empty"]

    def c_sv_maps(self, line, words, role):
        return [f"{map_['name']:<24} {map_['kind']}" for map_ in self.maps()]

    def c_sv_gametypes(self, line, words, role):
        return ["built in: " + ", ".join(BUILTINS),
                "and gametypes/<name>.toml files, and file=<name> for gametypes/<name>.gametype"]

    def ban_line(self, ban):
        fields = [ban["when"], f"ip={ban['ip']}", f"hwid={ban['hwid']}", f"players={ban['players']}",
                  f"reason={ban['reason']}"] + ([f"until={ban['until']}"] if ban["until"] else [])
        return "\t".join(fields)

    def c_sv_bans(self, line, words, role):
        return [f"{index:3} {'(ended) ' if ban['until'] and ban['until'] <= time.time() else ''}{self.ban_line(ban)}"
                for index, ban in enumerate(self.bans)] or ["no bans"]

    def c_sv_settings(self, line, words, role):
        return [f"{setting['name']:<32} " + (("(set)" if setting["value"] else "(none)") if setting["secret"]
                                              else self.setting_text(setting))
                + ("  (set by its environment variable)" if setting["pinned"] else "") for setting in self.settings]

    def c_sv_version(self, line, words, role):
        return [f"OpenCE {BUILD}, network version {NETWORK_VERSION}"]

    def c_sv_log(self, line, words, role):
        count = 20
        if len(words) >= 2:
            if not re.fullmatch(r"\d{1,3}", words[1]) or not 1 <= int(words[1]) <= 200:
                raise Failed("sv_log: give how many lines (1 to 200)")
            count = int(words[1])
        return [f"{line_['cat']}: {line_['text']}" for line_ in list(self.log_lines)[-count:]]

    def c_sv_masters(self, line, words, role):
        return [json.dumps({"masters": self.masters}, separators=(",", ":"))]

    def c_sv_publisher(self, line, words, role):
        return ["publisher key id 3f9a2c7d1e5b8a04, public key ed25519:" + "ab" * 32]

    def notice(self, text):
        """(in the lobby a notice reaches no machine: said back)"""
        if not self.playing():
            return [f"(in the lobby: no machine sees a notice until a game runs) {text[:NOTICE_SIZE - 1]}"]
        return []

    def c_say(self, line, words, role):
        text = printable_text(rest_after_words(line, 1), NOTICE_SIZE)
        if not text:
            raise Failed("say: give the text")
        if ROLES.index(role) < ROLES.index("owner"):
            minute = int(time.time() // 60)
            if minute != self.say_minute:
                self.say_minute, self.say_count = minute, 0
            self.say_count += 1
            if self.say_count > 20:
                raise Failed("say: 20 lines a minute at most")
        lines = self.notice(text)
        self.log("info", "chat", f"server: {text}")
        return lines

    def c_tell(self, line, words, role):
        if len(words) < 3:
            raise Failed("tell: give a player and the text")
        self.find_player("tell", words[1])
        return self.notice(printable_text(rest_after_words(line, 2), NOTICE_SIZE))

    def c_kick(self, line, words, role):
        if len(words) < 2:
            raise Failed("kick: give a player's slot (sv_players) or name")
        player = self.find_player("kick", words[1])
        self.log("info", "moderation", f"{player.name} kicked ({role})")
        self.moderation_event("kick", player.name, role)
        self.remove_player(player, "kicked")
        return []

    def c_ban(self, line, words, role):
        if len(words) < 2:
            raise Failed("ban: give a player's slot (sv_players) or name, then a duration (30m, 2h, 7d, forever) and "
                         "a reason")
        seconds, reason_words = 0, 2
        if len(words) >= 3:
            seconds = parse_duration(words[2])
            if seconds >= 0:
                reason_words = 3
            else:
                seconds = 0
        if ROLES.index(role) < ROLES.index("admin") and (seconds == 0 or seconds > 86400):
            raise Failed("ban: a moderator's ban is for a day at most (give a duration: 30m, 2h, 1d)")
        player = self.find_player("ban", words[1])
        reason = printable_text(rest_after_words(line, reason_words), 96).replace("\t", " ")
        until = int(time.time()) + seconds if seconds else None
        shown = (reason or "banned by the server") + ("" if role == "owner" else f" (by the {role})")
        self.bans.append({"when": time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()), "ip": player.address,
                          "hwid": player.hwid, "discord_id": player.discord_id,
                          "discord_verified": player.discord_verified, "players": player.name, "reason": shown,
                          "until": until})
        lines = []
        if until:
            lines.append(f"{player.name} banned for {seconds // 60} minutes")
            self.log("info", "moderation", f"{player.name} banned until {until} ({role})")
        else:
            self.log("info", "moderation", f"{player.name} banned ({role})")
        self.moderation_event("ban", player.name, role, reason or None, until)
        self.remove_player(player, "banned")
        return lines

    def c_unban(self, line, words, role):
        if len(words) < 2:
            raise Failed("unban: give a ban's number (sv_bans), an address or a hardware id")
        target = words[1]
        for index, ban in enumerate(self.bans):
            if (re.fullmatch(r"\d+", target) and int(target) == index) or target in (ban["ip"], ban["hwid"]) or \
                    (target.startswith("discord:") and target[8:] == ban.get("discord_id")):
                del self.bans[index]
                self.log("info", "moderation", f"ban {index} taken out ({role})")
                self.moderation_event("unban", ban["players"] or f"ban {index}", role)
                return [f"unbanned: {self.ban_line(ban)}"]
        raise Failed(f"unban: no ban {target} (sv_bans lists them)")

    def ban_address(self, ip, hwid, names, seconds, reason, role, discord_id=""):
        """server_commands.c's ban_address: a line of bans.txt for an address,
        a hardware id, a verified Discord account or more than one, and each
        player present of them dropped as ban drops one; the new ban's number,
        or CommandError (400)"""
        ip, hwid, discord_id = ip or "", hwid or "", discord_id or ""
        if not ip and not hwid and not discord_id:
            raise CommandError("schema", "give an address (a.b.c.d), a hardware id or a Discord account's id")
        if discord_id and not re.fullmatch(r"\d{1,20}", discord_id):
            raise CommandError("schema", "a Discord account's id is up to 20 digits")
        if ip:
            match = re.fullmatch(r"(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})", ip)
            if not match or any(int(part) > 255 for part in match.groups()) or \
                    all(int(part) == 0 for part in match.groups()):
                raise CommandError("schema", f"{ip} is not an IPv4 address (a.b.c.d)")
            ip = ".".join(str(int(part)) for part in match.groups())
        if hwid:
            if not re.fullmatch(r"[0-9A-Fa-f]{1,32}", hwid):
                raise CommandError("schema", "a hardware id is up to 32 hex digits")
            hwid = hwid.lower()
        names = printable_text(names or "", 64)
        reason = printable_text(reason or "", 96)
        until = int(time.time()) + seconds if seconds else None
        shown = (reason or "banned by the server") + ("" if role == "owner" else f" (by the {role})")
        self.bans.append({"when": time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()), "ip": ip or "none",
                          "hwid": hwid or "none", "discord_id": discord_id, "discord_verified": bool(discord_id),
                          "players": names, "reason": shown, "until": until})
        number = len(self.bans) - 1
        dropped = [player for player in self.players.values() if (ip and player.address == ip) or
                   (hwid and player.hwid == hwid) or
                   (discord_id and player.discord_verified and player.discord_id == discord_id)]
        self.log("info", "moderation", f"ban {number} added ({role})")
        self.moderation_event("ban", names or ", ".join(player.name for player in dropped), role, reason or None,
                              until)
        for player in dropped:
            self.remove_player(player, "banned")
        return number

    def c_sv_ban_address(self, line, words, role):
        if len(words) < 2:
            raise Failed("sv_ban_address: give an address (a.b.c.d) or a hardware id, then a duration (30m, 2h, 7d, "
                         "forever) and a reason")
        seconds, reason_words = 0, 2
        if len(words) >= 3:
            seconds = parse_duration(words[2])
            if seconds >= 0:
                reason_words = 3
            else:
                seconds = 0
        address = "." in words[1]
        discord = words[1].startswith("discord:")
        try:
            number = self.ban_address(words[1] if address else "", "" if address or discord else words[1], "",
                                      seconds, rest_after_words(line, reason_words), role,
                                      words[1][8:] if discord else "")
        except CommandError as failure:
            raise Failed(f"sv_ban_address: {failure.message}") from None
        return [f"ban {number} added" + (f", for {seconds // 60} minutes" if seconds else "")]

    def vpn_listed(self, address):
        """server_netlist.c's server_netlist_listed: (listed, by)"""
        def number(text):
            return int.from_bytes(bytes(int(part) for part in text.split(".")), "big")
        listed = number(VPN_LISTED[0]) <= number(address) <= number(VPN_LISTED[1])
        return listed, "netlist" if listed else ""

    def vpn_json(self):
        """server_netlist.c's server_netlist_status: /api/vpn's members"""
        return {"policy": self.value("server.vpn_policy"), "fail": self.value("server.vpn_fail"),
                "lists": [{"master": "https://master.opencommunityedition.com", "key_id": "89f4eda71229f11f",
                           "serial": 12, "generated": int(self.started) - 3600, "fetched": int(self.started),
                           "ranges": 1, "error": ""}],
                "file": "", "allow_ranges": 0, "block_ranges": 0, "held": True,
                "refused": dict(self.vpn_refused)}

    def c_sv_vpn(self, line, words, role):
        info = self.vpn_json()
        return [f"VPN policy {info['policy']} (server.vpn_fail {info['fail']})",
                "https://master.opencommunityedition.com: serial 12, 1 ranges, 1 hours old",
                "vpn_allow.txt: 0 ranges, vpn_block.txt: 0 ranges",
                "refused: " + ", ".join(f"{count} {reason}" for reason, count in info["refused"].items())]

    def c_sv_vpn_check(self, line, words, role):
        if len(words) < 2 or not re.fullmatch(r"(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})", words[1]) or \
                any(int(part) > 255 for part in words[1].split(".")):
            raise Failed("sv_vpn_check: give an IPv4 address (a.b.c.d)")
        listed, by = self.vpn_listed(words[1])
        if listed:
            return [f"{words[1]} is listed ({by}): refused as server.vpn_policy says ({self.value('server.vpn_policy')})"]
        return [f"{words[1]} is not listed"]

    def c_sv_vpn_fetch(self, line, words, role):
        return ["the master servers' netlists are being fetched (sv_vpn says when)"]

    def refuse_vpn(self, name, address, reason):
        """network_link.c's refusal: counted, logged, and a join_refused event"""
        self.vpn_refused[reason] += 1
        self.log("info", "player", f"{name} refused at the door ({reason})")
        self.emit("join_refused", "moderation", {"address": address, "hwid": hashlib.sha256(name.encode()).hexdigest()[:32]},
                  name=name, reason=reason)

    def c_mute(self, line, words, role):
        mute = words[0].lower() != "unmute"
        command = "mute" if mute else "unmute"
        if len(words) < 2:
            raise Failed(f"{command}: give a player's slot (sv_players) or name")
        minutes = 0
        if mute and len(words) >= 3:
            if not re.fullmatch(r"\d{1,4}", words[2]) or not 1 <= int(words[2]) <= 1440:
                raise Failed("mute: give the minutes (1 to 1440)")
            minutes = int(words[2])
        player = self.find_player(command, words[1])
        if not mute and not player.muted:
            raise Failed(f"unmute: {player.name} is not muted")
        player.muted = mute
        player.muted_until = time.time() + minutes * 60 if mute and minutes else None
        self.moderation_event(command, player.name, role)
        return []

    c_unmute = c_mute

    def c_sv_map(self, line, words, role):
        if len(words) < 2:
            raise Failed("sv_map: give a map, and a gametype: sv_map bloodgulch ctf")
        text = f"{words[1]}:{words[2]}" if len(words) >= 3 and ":" not in words[1] else words[1]
        entry = self.parse_entry(text)
        if entry is None:
            raise Failed("sv_map: give a map, and a gametype: sv_map bloodgulch ctf")
        if entry["rejected"]:
            raise Failed(f"sv_map: {entry['rejected']}")
        self.interposed = entry
        if self.state == "playing":
            self.end_game("next", postgame=False)
        elif self.state in ("lobby", "postgame", "waiting"):
            self.advance()
        return [f"next: {text}"]

    def c_sv_map_next(self, line, words, role):
        if self.state != "playing":
            raise Failed(f"{words[0]}: no game is being played")
        self.end_game("next", postgame=False)
        return []

    def c_sv_end_game(self, line, words, role):
        if self.state != "playing":
            raise Failed("sv_end_game: no game is being played")
        self.end_game("command")
        return []

    def c_sv_rotation_set(self, line, words, role):
        text = rest_after_words(line, 1)
        if not text:
            raise Failed("sv_rotation_set: give the entries, separated by commas")
        ok, rejected = self.set_rotation(text)
        if not ok:
            raise Failed("sv_rotation_set: not changed: " + json.dumps({"rejected": rejected}, separators=(",", ":")))
        return []

    def c_sv_rotation_add(self, line, words, role):
        text = rest_after_words(line, 1)
        if len(words) >= 3 and ":" not in words[1]:
            text = f"{words[1]}:{words[2]}"
        entry = self.parse_entry(text) if text else None
        if entry is None:
            raise Failed(f"{words[0]}: give an entry: map:gametype")
        ok, _ = self.set_rotation(", ".join([item["text"] for item in self.rotation] + [entry["text"]]))
        if not ok:
            raise Failed(f"{words[0]}: {entry['text']} cannot be played" + (f": {entry['rejected']}"
                                                                            if entry["rejected"] else ""))
        return []

    def c_sv_rotation_del(self, line, words, role):
        if len(words) < 2 or not re.fullmatch(r"\d+", words[1]) or int(words[1]) >= len(self.rotation):
            raise Failed(f"{words[0]}: give an entry's number (sv_rotation lists them)")
        texts = [entry["text"] for entry in self.rotation]
        del texts[int(words[1])]
        if not self.set_rotation(", ".join(texts))[0]:
            raise Failed(f"{words[0]}: the rotation would have nothing playable")
        return []

    def c_sv_rotation_begin(self, line, words, role):
        index = 0
        if len(words) >= 2:
            if not re.fullmatch(r"\d+", words[1]) or int(words[1]) > 63:
                raise Failed(f"{words[0]}: give an entry's number (sv_rotation lists them)")
            index = int(words[1])
        if index >= len(self.rotation) or self.rotation[index]["rejected"]:
            raise Failed(f"{words[0]}: entry {index} cannot be played")
        self.rotation_index = (index - 1) % len(self.rotation)
        return [f"the next game is entry {index}"]

    def c_sv_set(self, line, words, role):
        if len(words) < 2:
            raise Failed("sv_set: give a setting and its value (sv_settings lists them)")
        value = printable_text(rest_after_words(line, 2), 300)
        try:
            self.set_setting(words[1], value, role)
        except CommandError as failure:
            raise Failed(f"sv_set: {failure.message}") from None
        setting = self.setting(words[1])
        return [f"{words[1]} = " + (("(set)" if value else "(none)") if setting["secret"] else value)]

    def c_sv_name(self, line, words, role):
        name = words[0].lower()
        setting_name = SETTING_ALIASES[name]
        value = printable_text(rest_after_words(line, 1), 300)
        if setting_name == "server.name" and len(value) > 15:
            raise Failed(f"{name}: a name has 15 characters at most")
        if setting_name == "server.max_players" and (not re.fullmatch(r"-?\d+", value) or
                                                     not 2 <= int(value) <= 127):
            raise Failed(f"{name}: give a number from 2 to 127")
        try:
            self.set_setting(setting_name, value, role)
        except CommandError as failure:
            raise Failed(f"{name}: {failure.message}") from None
        setting = self.setting(setting_name)
        return [f"{setting_name} = " + (("(set)" if value else "(none)") if setting["secret"] else value)]

    c_sv_maxplayers = c_sv_password = c_sv_public = c_sv_motd = c_sv_name

    def c_sv_reload(self, line, words, role):
        self.emit("reloaded", "server", rotation_changed=False, rejected=[
            {"index": index, "entry": entry["text"], "reason": entry["rejected"]}
            for index, entry in enumerate(self.rotation) if entry["rejected"]])
        self.log("info", "server", "reloaded")
        return []

    def stop(self, code, reason):
        """the server stops (server_director.c's server_director_quit): the
        shutdown event; the simulator then exits with the code
        (--exit-on-stop), else goes on as before"""
        self.drain = None
        self.log("info", "server", f"stopping ({reason})")
        self.emit("shutdown", "server", reason=reason, code=code)
        if getattr(self.options, "exit_on_stop", False):
            self.state = "stopping"
            timer = threading.Timer(0.5, lambda: os._exit(code))
            timer.daemon = True
            timer.start()

    def drain_due(self):
        """a drain's moment (server_director.c's server_drain_update): for
        empty, nobody on the server; for game, no game played nor coming (a
        lobby with nobody, or no countdown)"""
        if self.drain["after"] == "empty":
            return not self.players
        return self.state in ("waiting", "starting") or (self.state == "lobby" and (not self.players or
                                                                                   self.countdown < 0))

    def c_sv_stop(self, line, words, role):
        restart = words[0].lower() == "sv_restart"
        after, notice_words = "now", 1
        if len(words) >= 2:
            after = words[1].lower()
            if after not in DRAINS:
                raise Failed(f"{words[0].lower()}: now, game, empty or cancel, then a notice for the players")
            notice_words = 2
        notice = printable_text(rest_after_words(line, notice_words), NOTICE_SIZE)
        lines = []
        if notice:
            lines += self.notice(notice)
            self.log("info", "chat", f"server: {notice}")
        code = 5 if restart else 0
        if after == "cancel":
            if not self.drain:
                raise Failed(f"{words[0].lower()}: no stop is waiting")
            self.emit("drain", "server", after="cancel", restart=self.drain["restart"], notice=notice)
            self.drain = None
            self.log("info", "server", "the stop is cancelled: listed (as server.public says) and open again")
            return lines + ["the stop is cancelled: the server is listed and open again"]
        self.emit("drain", "server", after=after, restart=restart, notice=notice)
        if after == "now":
            self.stop(code, "sv_restart" if restart else "sv_stop")
            return lines + [f"stopping (exit code {code})"]
        since = self.drain["since"] if self.drain else int(time.time())
        self.drain = {"after": after, "restart": restart, "since": since}
        self.log("info", "server", f"stopping (exit code {code}) once "
                 f"{'the game is over' if after == 'game' else 'nobody plays'}: the listings withdrawn, new players "
                 "refused")
        return lines + [f"stopping once {'this game is over' if after == 'game' else 'nobody plays'} (exit code {code}); "
                        "the listings withdrawn, new players refused"]

    c_sv_restart = c_sv_stop

    def c_quit(self, line, words, role):
        self.stop(0, "quit")
        return ["(simulated: the server would stop)"]

    # ----- settings

    @staticmethod
    def setting_text(setting):
        value = setting["value"]
        if isinstance(value, bool):
            return "true" if value else "false"
        return str(value)

    def set_setting(self, name, value, role):
        """a setting changed from its text (server_commands.c's
        server_commands_set_setting): CommandError with the status the web's
        PUT gives it (404, 409, 403, else 400)"""
        setting = self.setting(name)
        if not setting:
            raise CommandError("not_found", f"{name} is not a setting sv_set changes (sv_settings lists them)", 404)
        if setting["pinned"]:
            raise CommandError("pinned", f"{name} is set by {setting['env']} for this run", 409)
        if name in OWNER_SETTINGS and ROLES.index(role) < ROLES.index("owner"):
            raise CommandError("forbidden", f"{name} needs the owner role (the server's console)", 403)
        if any(ord(character) < 0x20 or ord(character) == 0x7F for character in value):
            raise CommandError("schema", f"{name}'s value has a control character")
        kind = setting["type"]
        if kind == "integer":
            if not re.fullmatch(r"-?\d{1,10}", value) or abs(int(value)) > 2147483647:
                raise CommandError("schema", f"{name} takes a whole number")
            typed = int(value)
            if (setting["min"] is not None and typed < setting["min"]) or \
                    (setting["max"] is not None and typed > setting["max"]):
                raise CommandError("schema", f"{name} takes {setting['min']} to {setting['max']}")
        elif kind == "boolean":
            if value.lower() not in ("true", "false", "1", "0", "on", "off"):
                raise CommandError("schema", f"{name} takes true or false")
            typed = value.lower() in ("true", "1", "on")
        else:
            typed = value
            if setting["choices"] and value not in setting["choices"]:
                raise CommandError("schema", f"{name} takes {', '.join(setting['choices'])}")
        if name in ("server.web_password", "server.web_moderator_password") and value and len(value) < 8:
            raise CommandError("schema", "a web password has 8 characters or more")
        if len(value) > 256:
            raise CommandError("schema", "the value is longer than 256 characters")
        setting["value"] = typed

    def settings_json(self):
        result = []
        for setting in self.settings:
            value = dict(setting)
            if setting["secret"]:
                value["set"] = bool(setting["value"])
                value["value"] = ""
                value["default"] = ""
            result.append(value)
        return result

    def maps(self):
        """/api/maps' maps: the name, level, kind and title (the map's name
        as the game shows it; else its name)"""
        return [{"name": name, "level": f"levels\\test\\{name}\\{name}", "kind": "xbox", "title": title}
                for name, title in MAPS] + \
            [{"name": name, "level": f"custom_maps\\{name}", "kind": "custom", "title": title or name}
             for name, title in CUSTOM_MAPS]

    def gametypes_json(self):
        files = []
        for name, text in sorted(self.gametype_files.items()):
            try:
                base, shown, _ = validate_gametype(text)
                files.append({"name": name, "base": base, "description": shown, "error": None})
            except GametypeError as failure:
                files.append({"name": name, "base": "", "description": "", "error": failure.message})
        return {"ok": True, "builtin": list(BUILTINS), "schema": {"fields": SCHEMA}, "files": files}

    # ----- sign-in

    def password_role(self, password, local=True):
        """the role a password gives: the owner's from loopback alone ("forbidden"
        from elsewhere), else the admin's, the moderator's or the viewer's"""
        owner = self.owner_password()
        forbidden = False
        if owner and password and hmac.compare_digest(password.encode(), owner.encode()):
            if local:
                return "owner"
            forbidden = True
        role = None
        for candidate, wanted in (("admin", self.options.password), ("moderator", self.options.moderator_password),
                                  ("viewer", self.options.viewer_password)):
            if wanted and hmac.compare_digest(password.encode(), wanted.encode()):
                role = role or candidate
        return role or ("forbidden" if forbidden else None)

    def address_record(self, address):
        return self.addresses.setdefault(address, {"window": 0.0, "requests": 0, "failures": [], "locked": 0.0})

    def allows_request(self, address, authenticated):
        """(--request-limit) the address's allowance: 60 requests a minute
        without a session, 600 with one"""
        if not self.options.request_limit:
            return True
        with self.lock:
            record = self.address_record(address)
            at = time.time()
            if at - record["window"] >= 60:
                record["window"], record["requests"] = at, 0
            record["requests"] += 1
            return record["requests"] <= (600 if authenticated else 60)

    def check_password(self, address, password, local=True):
        """the role a password gives, or CommandError (denied, locked,
        rate_limited, forbidden: the owner's password from beyond loopback,
        counted as a wrong one); counts failures and locks an address out"""
        with self.lock:
            at = time.time()
            record = self.address_record(address)
            if record["locked"] > at:
                raise CommandError("locked", "too many tries: wait", 429, retry_after=int(record["locked"] - at) + 1)
            while self.checks and self.checks[0] < at - 60:
                self.checks.popleft()
            if len(self.checks) >= 30:
                raise CommandError("rate_limited", "too many tries: wait", 429,
                                   retry_after=int(self.checks[0] + 60 - at) + 1)
            self.checks.append(at)
            role = self.password_role(password if isinstance(password, str) else "", local)
            if role and role != "forbidden":
                record["failures"] = []
                return role
            record["failures"] = [stamp for stamp in record["failures"] if stamp > at - 600] + [at]
            if len(record["failures"]) >= 5:
                record["locked"] = at + 600
                record["failures"] = []
                self.log("warn", "server", f"web: {address} locked out for 10 minutes after 5 wrong passwords")
            if role == "forbidden":
                self.log("warn", "server", "web: the owner's password was given from another address: refused (it "
                         "signs in from this machine alone)")
                raise CommandError("forbidden", "the owner's password signs in from the server's own machine alone",
                                   403)
            raise CommandError("denied", "wrong password", 401)

    def new_session(self, role, by):
        with self.lock:
            self.prune_sessions()
            if len(self.sessions) >= MAXIMUM_SESSIONS:
                oldest = min(self.sessions.values(), key=lambda session: session.used)
                del self.sessions[oldest.token]
            session = Session(role, by)
            self.sessions[session.token] = session
            return session

    def prune_sessions(self):
        at = time.time()
        for token in [token for token, session in self.sessions.items() if not session.valid(at)]:
            del self.sessions[token]

    def session(self, token):
        with self.lock:
            session = self.sessions.get(token or "")
            if session and session.valid(time.time()):
                session.used = time.time()
                return session
            return None


# ---------- HTTP


def forwarded_address(text):
    """X-Forwarded-For's last entry (the one the trusted proxy appended):
    an IPv4 or IPv6 address, with or without a port; None if it is none"""
    entry = text.split(",")[-1].strip()
    match = re.fullmatch(r"\[([0-9A-Fa-f:.]+)\](?::\d{1,5})?", entry)
    if match:
        entry = match.group(1)
    elif re.fullmatch(r"[\d.]+:\d{1,5}", entry):
        entry = entry.rsplit(":", 1)[0]
    try:
        return str(ipaddress.ip_address(entry))
    except ValueError:
        return None


class Request:
    def __init__(self, method, target, version, headers, body, address, trusted_proxy=""):
        self.method = method
        self.target = target
        self.path, _, self.query_text = target.partition("?")
        self.version = version
        self.headers = headers
        self.body = body
        self.address = address
        self.trusted_proxy = trusted_proxy
        # (behind a trusted proxy, the client's address is its word: the entry it appended)
        self.real_address = address
        if trusted_proxy and self.header("x-forwarded-for") and address == trusted_proxy:
            self.real_address = forwarded_address(self.header("x-forwarded-for")) or address
        self.query = {}
        for pair in self.query_text.split("&"):
            if pair:
                key, _, value = pair.partition("=")
                self.query[key] = value

    def header(self, name):
        return self.headers.get(name.lower(), "")

    def cookie(self, name):
        for part in self.header("cookie").split(";"):
            key, _, value = part.strip().partition("=")
            if key == name:
                return value
        return ""

    def behind_proxy(self):
        return self.real_address != self.address

    @property
    def local(self):
        """the connection is this machine's: its own address in 127.0.0.0/8, and
        not the trusted proxy's (the owner's role is never given through it)"""
        try:
            loopback = ipaddress.ip_address(self.address) in ipaddress.ip_network("127.0.0.0/8")
        except ValueError:
            loopback = False
        return loopback and not (self.trusted_proxy and self.address == self.trusted_proxy)

    def host(self):
        if self.header("x-forwarded-host") and self.behind_proxy():
            return self.header("x-forwarded-host")
        return self.header("host")

    def origin_matches(self):
        """the request's Origin is the server's own (its host; the scheme is
        the proxy's); null only when the browser says same-origin"""
        origin = self.header("origin")
        if origin == "null":
            return self.header("sec-fetch-site") == "same-origin"
        scheme, separator, host = origin.partition("://")
        return bool(separator and self.host() and host.lower() == self.host().lower())


class Response:
    def __init__(self, status, body=None, headers=None, content_type="application/json", close=False):
        self.status = status
        self.headers = list(headers or [])
        self.close = close
        self.content_type = content_type
        if isinstance(body, (dict, list)):
            self.body = json_bytes(body)
        elif isinstance(body, str):
            self.body = body.encode("utf-8")
        else:
            self.body = body or b""

    def bytes(self, head_only=False):
        reason = http.HTTPStatus(self.status).phrase
        lines = [f"HTTP/1.1 {self.status} {reason}"]
        names = {name.lower() for name, _ in self.headers}
        headers = list(self.headers)
        if self.status != 304:
            headers.append(("Content-Type", self.content_type))
        for name, value in SECURITY_HEADERS:
            if name.lower() not in names:
                headers.append((name, value))
        headers.append(("Content-Length", str(len(self.body))))
        if self.close:
            headers.append(("Connection", "close"))
        lines += [f"{name}: {value}" for name, value in headers]
        return ("\r\n".join(lines) + "\r\n\r\n").encode("latin-1") + (b"" if head_only else self.body)


def error(status, code, message, headers=None, close=False, **extra):
    """{"ok":false,"error":{"code","message",…}}: a refusal's other fields
    (field, line, rejected, name, retry_after) are the error's"""
    body = {"ok": False, "error": {"code": code, "message": message}}
    body["error"].update(extra)
    headers = list(headers or [])
    if extra.get("retry_after"):
        headers.append(("Retry-After", str(extra["retry_after"])))
    return Response(status, body, headers, close=close)


def command_error(failure):
    response = error(failure.status, failure.code, failure.message, **failure.extra)
    if failure.lines:
        response.body = json_bytes(dict(json.loads(response.body), lines=failure.lines))
    return response


def command_answer(ok, lines):
    """a command's answer as server_http.c's ticket_command gives it: ok and
    its lines, else the code its first line says (needs the … role: 403
    forbidden; no player, no ban, not a command: 404 not_found; set by its
    environment variable: 409 pinned; else 400 failed)"""
    if ok:
        return Response(200, {"ok": True, "lines": lines})
    first = lines[0] if lines else "the command failed"
    status, code = 400, "failed"
    if "needs the " in first and " role" in first:
        status, code = 403, "forbidden"
    elif "not a command of the server's" in first or "no player" in first or ": no ban " in first:
        status, code = 404, "not_found"
    elif "set by HALO_" in first or "is set by " in first:
        status, code = 409, "pinned"
    return Response(status, {"ok": False, "error": {"code": code, "message": first}, "lines": lines})


ROUTES = []


def route(method, pattern, role, game=True):
    """a route: its method, path, role, and whether the game's thread
    answers it (a request that waits for it while a map loads is answered
    503 loading)"""
    def wrap(function):
        ROUTES.append((method, re.compile(pattern + r"\Z"), role, game, function))
        return function
    return wrap


class Api:
    def __init__(self, sim):
        self.sim = sim

    def handle(self, request):
        sim = self.sim
        method = "GET" if request.method == "HEAD" else request.method
        if method not in ("GET", "POST", "PUT", "DELETE"):
            return error(405, "method", "GET, POST, PUT or DELETE", [("Allow", "GET, HEAD, POST, PUT, DELETE")])
        if request.header("origin") and not request.origin_matches():
            return error(403, "origin", "a request from another site")
        if request.path == "/healthz":
            return self.call(request, method, None, False, h_healthz, ())
        if request.path == "/api/login":
            if method != "POST":
                return error(405, "method", "/api/login takes POST", [("Allow", "POST")])
            if not sim.allows_request(request.real_address, False):
                return error(429, "rate_limited", "too many requests: wait a minute", retry_after=60)
            return self.call(request, method, None, False, h_login, ())
        session, kind = self.find_session(request)
        if not sim.allows_request(request.real_address, session is not None):
            return error(429, "rate_limited", "too many requests: wait a minute", retry_after=60)
        if method == "GET" and request.path in ASSETS:
            return self.asset(request)
        if not request.path.startswith("/api/") and request.path != "/metrics":
            if method != "GET":
                return error(405, "method", "the page takes GET", [("Allow", "GET, HEAD")])
            return error(404, "not_found", f"nothing at {request.path}")
        allowed = []
        for route_method, pattern, role, game, function in ROUTES:
            match = pattern.match(request.path)
            if not match:
                continue
            if route_method != method:
                allowed.append(route_method)
                continue
            if request.path.startswith("/api/"):
                with sim.lock:
                    sim.api_requests += 1
                    count = sim.api_requests
                if sim.options.drop and count % 10 == 0:
                    return None
            return self.call(request, method, role, game, function, match.groups(), session, kind)
        if allowed:
            if "GET" in allowed:
                allowed.insert(allowed.index("GET") + 1, "HEAD")
            return error(405, "method", f"{request.path} takes {', '.join(allowed)}", [("Allow", ", ".join(allowed))])
        if not session:
            return error(401, "denied", "sign in first")
        return error(404, "not_found", "nothing is there")

    def find_session(self, request):
        """the request's session and how it came (an owner's session is used
        from this machine alone, where it was begun)"""
        bearer = re.match(r"Bearer\s+(\S+)", request.header("authorization"))
        if bearer:
            session, kind = self.sim.session(bearer.group(1)), "bearer"
        else:
            session, kind = self.sim.session(request.cookie("opence_session")), "cookie"
        if session and session.role == "owner" and not request.local:
            session = None
        return session, kind

    def call(self, request, method, role, game, function, groups, session=None, kind=None):
        sim = self.sim
        mutation = method in ("POST", "PUT", "DELETE")
        if role is not None:
            if not session:
                return error(401, "denied", "sign in first")
            if mutation and kind == "cookie":
                if not request.header("origin"):
                    return error(403, "origin", "a change from the page needs its Origin")
                if not hmac.compare_digest(request.header("x-csrf-token").encode(), session.csrf.encode()):
                    return error(403, "csrf", "the X-CSRF-Token header is missing or wrong")
            if ROLES.index(session.role) < ROLES.index(role):
                return error(403, "forbidden", f"needs the {role} role")
        body = {}
        if mutation and request.body.strip():
            try:
                body = json.loads(request.body.decode("utf-8"))
            except (ValueError, UnicodeDecodeError):
                return error(400, "bad_request", "the body is not JSON")
            if not isinstance(body, dict):
                return error(400, "bad_request", "the body is not a JSON object")
        if game and sim.state == "loading":
            return error(503, "loading", "the game did not answer in time (a map loading?)", retry_after=5)
        try:
            with sim.lock:
                return function(self, request, session, body, *groups)
        except CommandError as failure:
            return command_error(failure)

    def asset(self, request):
        if not self.sim.options.web_ui:
            return error(404, "not_found", "the page is off (server.web_ui)")
        name, content_type = ASSETS[request.path]
        try:
            data = (self.sim.options.ui_dir / name).read_bytes()
        except OSError:
            return error(404, "not_found", f"no {name} in {self.sim.options.ui_dir}")
        tag = '"' + hashlib.sha256(data).hexdigest()[:16] + '"'
        headers = [("ETag", tag), ("Cache-Control", "no-cache")]
        if request.header("if-none-match") == tag:
            return Response(304, b"", headers)
        return Response(200, data, headers, content_type)


def text_field(body, name, message, maximum=None):
    """a text the body must hold (not empty), cut to maximum"""
    value = body.get(name)
    if not isinstance(value, str) or not value:
        raise CommandError("schema", message, field=name)
    return value[:maximum] if maximum else value


@route("GET", r"/healthz", None, game=False)
def h_healthz(api, request, session, body):
    state = api.sim.state
    if state in ("starting", "stopping"):
        return Response(503, {"state": "starting" if state == "starting" else state})
    return Response(200, {"state": state})


@route("GET", r"/metrics", None, game=False)
def h_metrics(api, request, session, body):
    sim = api.sim
    status = sim.status("viewer")
    lines = [
        "# HELP opence_players Players in the game.", "# TYPE opence_players gauge", f"opence_players {status['players']}",
        "# TYPE opence_max_players gauge", f"opence_max_players {status['max_players']}",
        "# TYPE opence_machines gauge", f"opence_machines {status['machines']}",
        "# TYPE opence_uptime_seconds counter", f"opence_uptime_seconds {status['uptime']}",
        "# TYPE opence_games_hosted_total counter", f"opence_games_hosted_total {status['games_hosted']}",
        "# TYPE opence_frame_ms_max gauge", f"opence_frame_ms_max {status['frame_ms_max']}",
        "# TYPE opence_tick_ms_avg gauge", f"opence_tick_ms_avg {status['tick_ms_avg']}",
        "# TYPE opence_build_info gauge",
        f'opence_build_info{{build="{BUILD}",network_version="{NETWORK_VERSION}"}} 1',
    ]
    return Response(200, "\n".join(lines) + "\n", content_type="text/plain; version=0.0.4; charset=utf-8")


@route("POST", r"/api/login", None, game=False)
def h_login(api, request, session, body):
    sim = api.sim
    password = body.get("password")
    if not isinstance(password, str):
        raise CommandError("schema", "password is needed", field="password")
    by = body.get("by") if isinstance(body.get("by"), str) else ""
    role = sim.check_password(request.real_address, password, request.local)
    session = sim.new_session(role, by[:32] or f"web {role}")
    sim.log("info", "server", f"web: {session.by} signed in ({role})")
    cookie = f"opence_session={session.token}; HttpOnly; SameSite=Strict; Path=/"
    # (Secure only behind the trusted proxy, whose X-Forwarded-Proto is believed)
    if request.header("x-forwarded-proto").lower() == "https" and request.behind_proxy():
        cookie += "; Secure"
    return Response(200, {"ok": True, "token": session.token, "role": role, "csrf": session.csrf,
                          "expires": session.expires}, [("Set-Cookie", cookie)])


@route("POST", r"/api/logout", "viewer", game=False)
def h_logout(api, request, session, body):
    api.sim.sessions.pop(session.token, None)
    return Response(200, {"ok": True}, [("Set-Cookie", "opence_session=; HttpOnly; SameSite=Strict; Path=/; Max-Age=0")])


@route("GET", r"/api/session", "viewer", game=False)
def h_session(api, request, session, body):
    return Response(200, {"ok": True, "role": session.role, "csrf": session.csrf, "expires": session.expires})


@route("GET", r"/api/status", "viewer", game=False)
def h_status(api, request, session, body):
    return Response(200, api.sim.status(session.role))


@route("GET", r"/api/players", "viewer", game=False)
def h_players(api, request, session, body):
    return Response(200, {"ok": True, "players": api.sim.players_json(session.role)})


@route("POST", r"/api/players/(\d{1,3})/(kick|ban|mute|unmute|tell)", "moderator")
def h_player_action(api, request, session, body, slot, action):
    sim = api.sim
    name = body.get("name")
    if not isinstance(name, str) or not name:
        raise CommandError("schema", "name is needed (the player's, checked)", field="name")
    player = sim.players.get(int(slot))
    if not player:
        raise CommandError("not_found", f"no player in slot {int(slot)}", 404)
    if player.name != name:
        raise CommandError("changed", f"slot {int(slot)} is {player.name} now, not {name}: nothing was done", 409,
                           name=player.name)
    if action == "ban":
        duration = body.get("duration") if isinstance(body.get("duration"), str) else ""
        reason = body.get("reason") if isinstance(body.get("reason"), str) else ""
        line = f"ban {player.slot} {duration or 'forever'} {line_text(reason)}"
    elif action == "mute":
        minutes = body.get("minutes")
        if minutes is not None and (isinstance(minutes, bool) or not isinstance(minutes, (int, float)) or
                                    not 0 <= minutes <= 1440):
            raise CommandError("schema", "minutes is 1 to 1440", field="minutes")
        line = f"mute {player.slot}" + (f" {int(minutes)}" if minutes and minutes >= 1 else "")
    elif action == "tell":
        line = f"tell {player.slot} {line_text(text_field(body, 'text', 'text is needed'))}"
    else:
        line = f"{action} {player.slot}"
    return command_answer(*sim.run(line, session.role, session.by))


@route("POST", r"/api/say", "moderator")
def h_say(api, request, session, body):
    text = text_field(body, "text", "text is needed")
    return command_answer(*api.sim.run("say " + line_text(text), session.role, session.by))


@route("GET", r"/api/rotation", "viewer")
def h_rotation(api, request, session, body):
    return Response(200, api.sim.rotation_json())


@route("PUT", r"/api/rotation", "admin")
def h_rotation_put(api, request, session, body):
    entries = body.get("entries")
    refusal = CommandError("schema", "entries is a list of 1 to 64 entries (map:gametype), with no commas",
                           field="entries")
    if not isinstance(entries, list) or not 1 <= len(entries) <= 64:
        raise refusal
    if not all(isinstance(entry, str) and entry and "," not in entry and "\n" not in entry and len(entry) <= 255
               for entry in entries):
        raise refusal
    text = ", ".join(entries)
    if len(text) > ROTATION_TEXT:
        raise CommandError("schema", f"the rotation is {ROTATION_TEXT} characters at most", field="entries")
    ok, rejected = api.sim.set_rotation(text)
    if not ok:
        raise CommandError("rejected", "some entries cannot be played: nothing changed", rejected=rejected)
    api.sim.log("info", "server", f"rotation set by {session.by}: {api.sim.rotation_text}")
    return Response(200, api.sim.rotation_json())


@route("POST", r"/api/rotation/next", "moderator")
def h_rotation_next(api, request, session, body):
    return command_answer(*api.sim.run("sv_map_next", session.role, session.by))


@route("POST", r"/api/rotation/play", "moderator")
def h_rotation_play(api, request, session, body):
    map_text = body.get("map")
    if not isinstance(map_text, str) or not map_text or len(map_text) > 255 or " " in map_text or '"' in map_text:
        raise CommandError("schema", "map is needed: a map or a rotation entry, 255 characters at most, with no spaces",
                           field="map")
    gametype = body.get("gametype") if isinstance(body.get("gametype"), str) else ""
    if " " in gametype or '"' in gametype:
        raise CommandError("schema", "a gametype holds no space", field="gametype")
    return command_answer(*api.sim.run(f"sv_map {map_text} {gametype}", session.role, session.by))


@route("GET", r"/api/gametypes", "viewer")
def h_gametypes(api, request, session, body):
    return Response(200, api.sim.gametypes_json())


def gametype_name(name):
    if not gametype_file_name_valid(name):
        raise CommandError("not_found", "a gametype file's name is a-z, 0-9, _ and -, 32 at most", 404)
    return name


@route("GET", r"/api/gametypes/([^/]*)", "viewer")
def h_gametype(api, request, session, body, name):
    text = api.sim.gametype_files.get(gametype_name(name))
    if text is None:
        raise CommandError("not_found", "no such gametype file", 404)
    return Response(200, {"ok": True, "name": name, "text": text})


@route("PUT", r"/api/gametypes/([^/]*)", "admin")
def h_gametype_put(api, request, session, body, name):
    gametype_name(name)
    text = body.get("text")
    if not isinstance(text, str):
        raise CommandError("schema", "text is needed (the file's TOML)", field="text")
    try:
        validate_gametype(text)
    except GametypeError as failure:
        raise CommandError("invalid", failure.message, line=failure.line) from None
    api.sim.gametype_files[name] = text
    api.sim.log("info", "server", f"gametypes/{name}.toml written by {session.by}")
    return Response(200, {"ok": True})


@route("DELETE", r"/api/gametypes/([^/]*)", "admin")
def h_gametype_delete(api, request, session, body, name):
    if gametype_name(name) not in api.sim.gametype_files:
        raise CommandError("not_found", "no such gametype file", 404)
    del api.sim.gametype_files[name]
    api.sim.log("info", "server", f"gametypes/{name}.toml deleted by {session.by}")
    return Response(200, {"ok": True})


@route("GET", r"/api/maps", "viewer")
def h_maps(api, request, session, body):
    return Response(200, {"ok": True, "maps": api.sim.maps()})


@route("GET", r"/api/vpn", "viewer")
def h_vpn(api, request, session, body):
    return Response(200, {"ok": True, **api.sim.vpn_json()})


@route("POST", r"/api/vpn/check", "moderator")
def h_vpn_check(api, request, session, body):
    ip = body.get("ip")
    if not isinstance(ip, str) or not re.fullmatch(r"(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})", ip) or \
            any(int(part) > 255 for part in ip.split(".")):
        raise CommandError("schema", "ip is an IPv4 address (a.b.c.d)", field="ip")
    listed, by = api.sim.vpn_listed(ip)
    return Response(200, {"ok": True, "listed": listed, "by": by})


@route("GET", r"/api/bans", "admin")
def h_bans(api, request, session, body):
    return Response(200, {"ok": True, "bans": [dict(ban, n=index) for index, ban in enumerate(api.sim.bans)]})


@route("DELETE", r"/api/bans/(\d{1,5})", "admin")
def h_ban_delete(api, request, session, body, number):
    return command_answer(*api.sim.run(f"unban {int(number)}", session.role, session.by))


@route("POST", r"/api/bans", "admin")
def h_ban_add(api, request, session, body):
    """a ban of an address or hardware id (sv_ban_address): {"ok":true,"n":…}"""
    sim = api.sim
    fields = {}
    for name in ("ip", "hwid", "discord_id", "names", "duration", "reason"):
        value = body.get(name)
        if value is not None and not isinstance(value, str):
            raise CommandError("schema", f"{name} is a text", field=name)
        fields[name] = value or ""
    seconds = 0
    if fields["duration"]:
        seconds = parse_duration(fields["duration"])
        if seconds < 0:
            raise CommandError("schema", "duration is 30m, 2h, 7d, 1w or forever (ten years at most)",
                               field="duration")
    sim.log("info", "console", f"{session.by}: (a ban of an address or hardware id)")
    number = sim.ban_address(fields["ip"], fields["hwid"], fields["names"], seconds, fields["reason"], session.role,
                             fields["discord_id"])
    return Response(200, {"ok": True, "n": number})


@route("POST", r"/api/stop", "owner")
def h_stop(api, request, session, body):
    """sv_stop or sv_restart, now, after the game or once empty, or the stop
    waiting cancelled: {"ok":true,"lines":[…]}"""
    for name in ("cancel", "restart"):
        if name in body and not isinstance(body[name], bool):
            raise CommandError("schema", "cancel and restart are true or false, notice a text", field=name)
    if "notice" in body and not isinstance(body["notice"], str):
        raise CommandError("schema", "cancel and restart are true or false, notice a text", field="notice")
    if body.get("cancel"):
        return command_answer(*api.sim.run("sv_stop cancel", session.role, session.by))
    after = body.get("after")
    if after not in ("now", "game", "empty"):
        raise CommandError("schema", "after is now, game or empty", field="after")
    line = f"{'sv_restart' if body.get('restart') else 'sv_stop'} {after} {line_text(body.get('notice') or '')}"
    return command_answer(*api.sim.run(line.rstrip(), session.role, session.by))


@route("GET", r"/api/settings", "admin")
def h_settings(api, request, session, body):
    return Response(200, {"ok": True, "settings": api.sim.settings_json()})


@route("PUT", r"/api/settings", "admin")
def h_settings_put(api, request, session, body):
    """each setting from its JSON value (a string's text, else the JSON's own:
    true, 16, null for empty), as the session's role: the owner's settings
    refused to an admin"""
    applied, errors = [], []
    for name, value in body.items():
        if isinstance(value, str):
            text = value
        elif value is None:
            text = ""
        else:
            text = json.dumps(value)
        try:
            api.sim.set_setting(name, text, session.role)
            applied.append(name)
            api.sim.log("info", "server", f"{name} changed by {session.by}")
        except CommandError as failure:
            # (server_http.c names each refusal by its status: 409 pinned, 404
            # not_found, 403 forbidden (an owner's setting), every other schema)
            code = {409: "pinned", 404: "not_found", 403: "forbidden"}.get(failure.status, "schema")
            errors.append({"name": name, "code": code, "message": failure.message})
    return Response(200, {"ok": True, "applied": applied, "errors": errors})


@route("POST", r"/api/command", "viewer")
def h_command(api, request, session, body):
    line = body.get("line")
    if not isinstance(line, str) or not line or len(line) > COMMAND_LINE:
        raise CommandError("schema", f"line is a command ({COMMAND_LINE} characters at most)", field="line")
    return command_answer(*api.sim.run(line, session.role, session.by))


def query_number(request, name, default):
    text = request.query.get(name, "")
    if not text:
        return default
    if not re.fullmatch(r"-?\d{1,12}", text):
        raise CommandError("bad_request", f"{name} is a number")
    return int(text)


@route("GET", r"/api/log", "viewer", game=False)
def h_log(api, request, session, body):
    sim = api.sim
    after = max(0, query_number(request, "after", 0))
    limit = max(1, min(1024, query_number(request, "limit", 256)))
    lines = list(sim.log_lines)
    if after > sim.log_n:
        after = 0
    oldest = lines[0]["n"] if lines else sim.log_n + 1
    missed = max(0, oldest - after - 1) if after else 0
    chosen = [line for line in lines if line["n"] > after][:limit]
    cursor = chosen[-1]["n"] if chosen else after
    if session.role not in ("admin", "owner"):
        chosen = [line for line in chosen if line["cat"] != "chat"]
    return Response(200, {"ok": True, "lines": chosen, "next": cursor, "missed": missed})


@route("GET", r"/api/events", "viewer", game=False)
def h_events(api, request, session, body):
    sim = api.sim
    after = max(0, query_number(request, "after", 0))
    limit = max(1, min(EVENT_RING, query_number(request, "limit", 256)))
    if after > sim.seq:
        after = 0
    ring = list(sim.events)
    oldest = ring[0][1]["seq"] if ring else sim.seq + 1
    admin = ROLES.index(session.role) >= ROLES.index("admin")
    addresses = sim.addresses_for(session.role)
    taken = [(group, addressed if addressed and addresses else event) for group, event, addressed in ring
             if event["seq"] > after][:limit]
    chosen = [event for group, event in taken if group != "chat" or admin]
    missed = max(0, oldest - after - 1) if after else 0
    return Response(200, {"ok": True, "events": chosen, "next": taken[-1][1]["seq"] if taken else after,
                          "missed": missed})


# ---------- WebSocket


GROUPS = {"player", "chat", "game", "moderation", "status", "log", "master", "server"}


class WsClose(Exception):
    def __init__(self, code, reason=""):
        super().__init__(reason)
        self.code = code
        self.reason = reason


class WsConnection:
    def __init__(self, sim, sock, buffer, address, local=False):
        self.sim = sim
        self.sock = sock
        self.buffer = buffer
        self.address = address
        # (the connection is this machine's: the owner's password and sessions are taken)
        self.local = local
        self.role = None
        self.by = ""
        self.groups = set()
        self.queue = deque()
        self.ready = threading.Condition()
        self.send_lock = threading.Lock()
        self.closed = False
        self.overflowed = False
        self.opened = time.time()
        self.bucket = 64.0
        self.bucket_time = time.time()

    # ----- out

    def offer(self, group, event):
        if self.role is None or (group not in self.groups and group != "server"):
            return
        if group == "chat" and ROLES.index(self.role) < ROLES.index("admin"):
            return
        self.put(event)

    def put(self, message):
        with self.ready:
            if self.closed or self.overflowed:
                return
            if len(self.queue) >= WS_QUEUE:
                self.overflowed = True
                self.queue.clear()
                self.queue.append({"type": "overflow"})
                self.queue.append(WsClose(1013, "overflow"))
            else:
                self.queue.append(message)
            self.ready.notify()

    def writer(self):
        try:
            while True:
                with self.ready:
                    while not self.queue and not self.closed:
                        self.ready.wait(1)
                    if self.closed and not self.queue:
                        return
                    message = self.queue.popleft()
                if isinstance(message, WsClose):
                    self.close(message.code, message.reason)
                    return
                self.send_text(json_bytes(message))
                if self.sim.options.slow_consumer:
                    time.sleep(0.05)
        except OSError:
            self.closed = True

    def send_frame(self, opcode, payload, fin=True):
        first = (0x80 if fin else 0) | opcode
        size = len(payload)
        if size < 126:
            head = struct.pack("!BB", first, size)
        elif size < 65536:
            head = struct.pack("!BBH", first, 126, size)
        else:
            head = struct.pack("!BBQ", first, 127, size)
        with self.send_lock:
            self.sock.sendall(head + payload)

    def send_text(self, payload):
        if not self.sim.options.fragment or len(payload) < 2:
            self.send_frame(0x1, payload)
            return
        size = max(1, min(4096, len(payload) // 3 + 1))
        parts = [payload[index:index + size] for index in range(0, len(payload), size)]
        if len(parts) == 1:
            parts = [payload[:1], payload[1:]]
        for index, part in enumerate(parts):
            self.send_frame(0x1 if index == 0 else 0x0, part, fin=index == len(parts) - 1)
            if index == 0:
                self.send_frame(0x9, b"fragment")

    def close(self, code, reason=""):
        with self.ready:
            if self.closed:
                return
            self.closed = True
            self.ready.notify_all()
        try:
            self.send_frame(0x8, struct.pack("!H", code) + reason.encode()[:120])
            self.sock.shutdown(socket.SHUT_WR)
        except OSError:
            pass

    # ----- in

    def read(self, count):
        while len(self.buffer) < count:
            chunk = self.sock.recv(65536)
            if not chunk:
                raise ConnectionError("closed")
            self.buffer += chunk
        data, self.buffer = self.buffer[:count], self.buffer[count:]
        return data

    def frame(self):
        first, second = self.read(2)
        fin, opcode = first & 0x80, first & 0x0F
        if first & 0x70:
            raise WsClose(1002, "reserved bits")
        if not second & 0x80:
            raise WsClose(1002, "a client's frames are masked")
        size = second & 0x7F
        if size == 126:
            size = struct.unpack("!H", self.read(2))[0]
        elif size == 127:
            size = struct.unpack("!Q", self.read(8))[0]
        if opcode >= 0x8 and (size > 125 or not fin):
            raise WsClose(1002, "a bad control frame")
        if size > WS_FRAME:
            raise WsClose(1009, "64 KB a frame at most")
        mask = self.read(4)
        payload = bytearray(self.read(size))
        for index in range(size):
            payload[index] ^= mask[index % 4]
        return bool(fin), opcode, bytes(payload)

    def messages(self):
        """the client's text messages, whole; answers its pings and closes"""
        pieces, total = None, 0
        while True:
            fin, opcode, payload = self.frame()
            if opcode == 0x8:
                code = struct.unpack("!H", payload[:2])[0] if len(payload) >= 2 else 1000
                raise WsClose(1000 if code in (1000, 1001) else 1000, "goodbye")
            if opcode == 0x9:
                self.send_frame(0xA, payload)
                continue
            if opcode == 0xA:
                continue
            if opcode == 0x2:
                raise WsClose(1003, "text frames only")
            if opcode == 0x1:
                if pieces is not None:
                    raise WsClose(1002, "a new message inside a fragmented one")
                pieces, total = [], 0
            elif opcode == 0x0:
                if pieces is None:
                    raise WsClose(1002, "a continuation without a message")
            else:
                raise WsClose(1002, "an unknown opcode")
            pieces.append(payload)
            total += len(payload)
            if total > WS_MESSAGE:
                raise WsClose(1009, "256 KB a message at most")
            if fin:
                data, pieces = b"".join(pieces), None
                try:
                    yield data.decode("utf-8")
                except UnicodeDecodeError:
                    raise WsClose(1007, "not UTF-8") from None

    def run(self, auto_session):
        sim = self.sim
        threading.Thread(target=self.writer, daemon=True).start()
        if sim.options.drop:
            threading.Timer(sim.options.drop_after, self.cut).start()
        self.put({"type": "hello", "protocol": 1, "server": sim.value("server.name"), "build": BUILD,
                  "network_version": NETWORK_VERSION, "capabilities": list(CAPABILITIES)})
        if auto_session:
            self.role, self.by = auto_session.role, auto_session.by
            self.put({"type": "auth", "ok": True, "role": self.role})
        timer = threading.Timer(10, self.auth_timeout)
        timer.daemon = True
        timer.start()
        try:
            for text in self.messages():
                self.message(text)
        except WsClose as close:
            self.close(close.code, close.reason)
        except (ConnectionError, OSError, struct.error, ValueError):
            pass
        finally:
            timer.cancel()
            with self.ready:
                self.closed = True
                self.ready.notify_all()
            with sim.lock:
                if self in sim.connections:
                    sim.connections.remove(self)

    def cut(self):
        """(--drop) the connection lost without a close frame"""
        with self.ready:
            self.closed = True
            self.ready.notify_all()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass

    def auth_timeout(self):
        if self.role is None and not self.closed:
            self.put({"type": "auth", "ok": False, "error": {"code": "denied", "message": "no auth in 10 seconds"}})
            self.put(WsClose(1008, "no auth"))

    def message(self, text):
        sim = self.sim
        try:
            message = json.loads(text)
        except ValueError:
            message = None
        if not isinstance(message, dict):
            self.error(None, "bad_request", "not a JSON object")
            return
        kind = message.get("type")
        identifier = message.get("id")
        if not isinstance(identifier, (str, int, float)) or isinstance(identifier, bool):
            identifier = None
        if not isinstance(kind, str):
            self.error(identifier, "bad_request", "a message has a type")
            return
        if kind == "ping":
            self.put({"type": "pong"})
            return
        if kind == "auth":
            if self.role is not None:
                self.error(identifier, "bad_request", "signed in already")
                return
            try:
                if isinstance(message.get("token"), str):
                    session = sim.session(message["token"])
                    if not session or (session.role == "owner" and not self.local):
                        raise CommandError("denied", "no such session", 401)
                    role, by = session.role, session.by
                else:
                    role = sim.check_password(self.address, message.get("password"), self.local)
                    by = message.get("by") if isinstance(message.get("by"), str) else ""
                    by = by[:32] or f"rcon {role}"
            except CommandError as failure:
                refusal = {"code": failure.code, "message": failure.message}
                if failure.extra.get("retry_after"):
                    refusal["retry_after"] = failure.extra["retry_after"]
                self.put({"type": "auth", "ok": False, "error": refusal})
                self.put(WsClose(1008, failure.code))
                return
            self.role, self.by = role, by
            sim.log("info", "server", f"rcon: {by} signed in" + (f" from {self.address}" if sim.value(
                "server.log_addresses") else "") + f" ({role})")
            self.put({"type": "auth", "ok": True, "role": role})
            return
        if kind == "command":
            self.command(message, identifier)
            return
        if self.role is None:
            self.error(identifier, "denied", "sign in first")
            return
        if kind == "subscribe":
            groups = message.get("events")
            if not isinstance(groups, list) or not all(isinstance(group, str) and group in GROUPS for group in groups):
                self.error(identifier, "bad_request", "events is a list of event groups")
                return
            after = message.get("after")
            with sim.lock:
                self.groups = set(groups)
                if isinstance(after, (int, float)) and not isinstance(after, bool) and after >= 0:
                    admin = ROLES.index(self.role) >= ROLES.index("admin")
                    addresses = sim.addresses_for(self.role)
                    for group, event, addressed in list(sim.events):
                        if event["seq"] > after and (group in self.groups or group == "server") and \
                                (group != "chat" or admin):
                            self.put(dict(addressed if addressed and addresses else event, replay=True))
                if "status" in self.groups:
                    self.put(sim.emit_status_for(self))
                if sim.options.slow_consumer:
                    for index in range(WS_QUEUE + 100):
                        sim.log("info", "console", f"slow-consumer filler {index}")
            return
        self.error(identifier, "bad_request", f"no message type {kind[:32]}")

    def error(self, identifier, code, message):
        """server_http.c's ws_error: its id only when the message had one"""
        answer = {"type": "error"}
        if identifier is not None:
            answer["id"] = identifier
        answer["error"] = {"code": code, "message": message}
        self.put(answer)

    def refused(self, identifier, code, message):
        """a command refused before it ran (server_http.c's ws_reply_refused)"""
        self.put({"type": "reply", "id": identifier, "ok": False, "lines": [],
                  "error": {"code": code, "message": message}})

    def command_line(self, message):
        """the command's line: "line", or "command" and "args" (a word with a
        space, or an empty one, in quotes); None if there is none"""
        if "line" in message:
            line = message["line"]
            return line if isinstance(line, str) and line else None
        command = message.get("command")
        if not isinstance(command, str) or not command:
            return None
        line = command
        for word in message.get("args") if isinstance(message.get("args"), list) else []:
            if not isinstance(word, str):
                return None
            line += f' "{word}"' if " " in word or not word else f" {word}"
        return line

    def command(self, message, identifier):
        sim = self.sim
        if self.role is None:
            self.refused(identifier, "denied", "sign in first")
            return
        at = time.time()
        self.bucket = min(64.0, self.bucket + (at - self.bucket_time) * 32)
        self.bucket_time = at
        if self.bucket < 1:
            self.refused(identifier, "rate_limited", "32 commands a second at most")
            return
        self.bucket -= 1
        line = self.command_line(message)
        if line is None or len(line) > COMMAND_LINE:
            self.refused(identifier, "bad_request", f"line is a command ({COMMAND_LINE} characters at most)")
            return
        if sim.state == "loading":
            self.refused(identifier, "loading", "the game did not answer in time (a map loading?)")
            return
        # (the real server's answer: its lines and ok, a failure's first line saying why, no error object)
        ok, lines = sim.run(line, self.role, self.by)
        self.put({"type": "reply", "id": identifier, "lines": lines, "ok": ok})


def emit_status_for(sim, connection):
    """the status event as the connection's role sees it: its seq the latest
    event's (a status event takes no number of its own)"""
    with sim.lock:
        return {"type": "event", "seq": sim.seq, "time": now_ms(), "event": "status",
                "status": sim.status(connection.role)}


Sim.emit_status_for = emit_status_for


def status_ticker(sim):
    """the status event, every 2 seconds, to each connection subscribed to it
    (the invite by each one's role)"""
    while True:
        time.sleep(2)
        with sim.lock:
            subscribed = [connection for connection in sim.connections
                          if connection.role is not None and "status" in connection.groups]
            for connection in subscribed:
                connection.put(sim.emit_status_for(connection))


def director(sim):
    while True:
        time.sleep(sim.options.tick)
        with sim.lock:
            sim.step()


class Handler(socketserver.BaseRequestHandler):
    def handle(self):
        sim = self.server.sim
        sock = self.request
        address = self.client_address[0]
        buffer = b""
        while True:
            start = time.monotonic()
            try:
                while b"\r\n\r\n" not in buffer:
                    if len(buffer) > MAXIMUM_HEAD:
                        sock.sendall(error(413, "too_large", "8 KB of head at most", close=True).bytes())
                        return
                    remaining = HEAD_SECONDS - (time.monotonic() - start)
                    if remaining <= 0:
                        return
                    sock.settimeout(remaining)
                    chunk = sock.recv(65536)
                    if not chunk:
                        return
                    buffer += chunk
                head, buffer = buffer.split(b"\r\n\r\n", 1)
                if len(head) > MAXIMUM_HEAD:
                    sock.sendall(error(413, "too_large", "8 KB of head at most", close=True).bytes())
                    return
                request = self.parse(head, address)
                if isinstance(request, Response):
                    sock.sendall(request.bytes())
                    return
                length_text = request.header("content-length") or "0"
                if not length_text.isdigit():
                    sock.sendall(error(400, "bad_request", "a bad Content-Length", close=True).bytes())
                    return
                length = int(length_text)
                if length > MAXIMUM_BODY:
                    sock.sendall(error(413, "too_large", "64 KB of body at most", close=True).bytes())
                    return
                while len(buffer) < length:
                    remaining = HEAD_SECONDS - (time.monotonic() - start)
                    if remaining <= 0:
                        return
                    sock.settimeout(remaining)
                    chunk = sock.recv(65536)
                    if not chunk:
                        return
                    buffer += chunk
                request.body, buffer = buffer[:length], buffer[length:]
                sock.settimeout(None)
            except (OSError, socket.timeout):
                return
            if request.path == "/ws" and "websocket" in request.header("upgrade").lower():
                self.websocket(request, buffer)
                return
            if request.path == "/ws":
                response = error(400, "bad_request", "/ws is the RCON: a WebSocket (Upgrade: websocket)")
            else:
                try:
                    response = Api(sim).handle(request)
                except Exception as failure:  # (a bug of the simulator's: answered, and shown)
                    print(f"server_sim: {request.method} {request.path}: {failure!r}", file=sys.stderr, flush=True)
                    response = error(500, "internal", str(failure))
            if response is None:
                return
            keep = request.version == "HTTP/1.1" and request.header("connection").lower() != "close" or \
                request.version == "HTTP/1.0" and request.header("connection").lower() == "keep-alive"
            response.close = response.close or not keep
            try:
                # (HEAD: GET's answer, its head alone)
                sock.sendall(response.bytes(head_only=request.method == "HEAD"))
            except OSError:
                return
            if response.close:
                return

    def parse(self, head, address):
        try:
            text = head.decode("latin-1")
        except UnicodeDecodeError:
            return error(400, "bad_request", "not a request", close=True)
        lines = text.split("\r\n")
        parts = lines[0].split(" ")
        if len(parts) != 3 or parts[2] not in ("HTTP/1.0", "HTTP/1.1") or not parts[1].startswith("/"):
            return error(400, "bad_request", "HTTP/1.0 or 1.1 requests only", close=True)
        method, target, version = parts
        headers = {}
        for line in lines[1:]:
            name, colon, value = line.partition(":")
            if not colon or not name or name != name.strip():
                return error(400, "bad_request", "a bad header line", close=True)
            name = name.lower()
            value = value.strip()
            if name in headers:
                headers[name] += ("; " if name == "cookie" else ", ") + value
            else:
                headers[name] = value
        if "transfer-encoding" in headers:
            return error(400, "bad_request", "Transfer-Encoding is not taken (send Content-Length)", close=True)
        if "expect" in headers:
            return error(400, "bad_request", "Expect is not taken", close=True)
        path = target.partition("?")[0]
        if ".." in path or "//" in path or "\\" in path or "%" in path:
            return error(400, "bad_request", "a path with .., //, \\ or % escapes", close=True)
        return Request(method, target, version, headers, b"", address, self.server.sim.options.trusted_proxy)

    def websocket(self, request, buffer):
        sim = self.server.sim
        sock = self.request
        key = request.header("sec-websocket-key")
        protocols = [item.strip() for item in request.header("sec-websocket-protocol").split(",")]
        origin = request.header("origin")
        api = Api(sim)

        def refuse(response):
            response.close = True
            sock.sendall(response.bytes())

        # (an upgrade counts against its address's allowance as a request does)
        if not sim.allows_request(request.real_address, api.find_session(request)[0] is not None):
            return refuse(error(429, "rate_limited", "too many requests: wait a minute", retry_after=60))
        if request.method != "GET" or "upgrade" not in request.header("connection").lower():
            return refuse(error(400, "bad_request", "/ws is the RCON: a WebSocket (Upgrade: websocket)"))
        if request.header("sec-websocket-version") != "13":
            return refuse(error(426, "bad_request", "WebSocket version 13 only", [("Sec-WebSocket-Version", "13")]))
        try:
            if len(base64.b64decode(key, validate=True)) != 16:
                raise ValueError
        except ValueError:
            return refuse(error(400, "bad_request", "a bad Sec-WebSocket-Key"))
        if SUBPROTOCOL not in protocols:
            return refuse(error(400, "bad_request", f"the subprotocol is {SUBPROTOCOL}"))
        if origin and not request.origin_matches():
            return refuse(error(403, "origin", "a connection from another site"))
        session = None
        bearer = re.match(r"Bearer\s+(\S+)", request.header("authorization"))
        if bearer:
            session = sim.session(bearer.group(1))
        elif origin and request.cookie("opence_session"):
            session = sim.session(request.cookie("opence_session"))
        if session and session.role == "owner" and not request.local:
            session = None
        connection = WsConnection(sim, sock, buffer, request.real_address, request.local)
        with sim.lock:
            if len(sim.connections) >= WS_CONNECTIONS:
                return refuse(error(503, "busy", f"{WS_CONNECTIONS} remote consoles at once at most", retry_after=5))
            sim.connections.append(connection)
        accept = base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode()
        sock.sendall(("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                      f"Sec-WebSocket-Accept: {accept}\r\nSec-WebSocket-Protocol: {SUBPROTOCOL}\r\n\r\n").encode())
        connection.run(session)


class Server(socketserver.ThreadingTCPServer):
    daemon_threads = True
    allow_reuse_address = True
    request_queue_size = 64


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--port", type=int, default=8099, help="the port (0: a free one)")
    parser.add_argument("--bind", default="127.0.0.1", help="the address (loopback by default)")
    parser.add_argument("--password", default="opence-admin", help="the admin password")
    parser.add_argument("--moderator-password", default="opence-mod", help="the moderator password")
    parser.add_argument("--viewer-password", default="", help="a viewer password (the simulator's own)")
    parser.add_argument("--owner-password", default="opence-owner",
                        help="the owner's password (server.web_owner_password): signs in from loopback alone; "
                        "empty for none")
    parser.add_argument("--instance", default="", help="server.instance (else a slug of the server's name)")
    parser.add_argument("--exit-on-stop", action="store_true",
                        help="a stop (quit, sv_stop, sv_restart, a drain's end) exits with its code (0 or 5)")
    parser.add_argument("--ui-dir", type=Path, default=UI_DIR, help="where the page's files are")
    parser.add_argument("--no-web-ui", dest="web_ui", action="store_false", help="no page (server.web_ui = false)")
    parser.add_argument("--tick", type=float, default=1.0, help="real seconds a simulated second takes")
    parser.add_argument("--still", action="store_true",
                        help="no scripted events: five players in a game that lasts until a command ends it")
    parser.add_argument("--seed", type=int, default=1, help="the script's random seed")
    parser.add_argument("--log-addresses", action="store_true", help="players' addresses for every role")
    parser.add_argument("--slow-consumer", action="store_true", help="WebSockets written slowly, a burst after "
                        "each subscription: their queues overflow")
    parser.add_argument("--drop", action="store_true", help="WebSockets cut a few seconds after they open, one "
                            "API request in ten left unanswered")
    parser.add_argument("--drop-after", type=float, default=5.0, help="seconds before --drop cuts a WebSocket")
    parser.add_argument("--big-replies", action="store_true", help="command replies of more than 1 MiB")
    parser.add_argument("--fragment", action="store_true", help="every message sent in continuation frames")
    parser.add_argument("--verbose", action="store_true", help="the log's lines on standard output")
    parser.add_argument("--trusted-proxy", default="", help="the reverse proxy's address (server.web_trusted_proxy): "
                        "its X-Forwarded-For (the last entry), -Host and -Proto are believed")
    parser.add_argument("--request-limit", action="store_true", help="each address's allowance, as the real "
                        "server's: 60 requests a minute without a session, 600 with one (WebSocket upgrades too)")
    options = parser.parse_args(argv)
    if options.bind not in ("127.0.0.1", "::1", "localhost"):
        short = [name for name, password in (("admin", options.password), ("moderator", options.moderator_password),
                                             ("viewer", options.viewer_password), ("owner", options.owner_password))
                 if password and len(password) < 8]
        if short or not options.password:
            print("server_sim: beyond loopback every web password is 8 characters or more", file=sys.stderr)
            return 3
    try:
        server = Server((options.bind, options.port), Handler)
    except OSError as failure:
        print(f"server_sim: {failure}", file=sys.stderr)
        return 4
    options.port = server.server_address[1]
    sim = Sim(options)
    server.sim = sim
    threading.Thread(target=director, args=(sim,), daemon=True).start()
    threading.Thread(target=status_ticker, args=(sim,), daemon=True).start()
    print(f"listening on http://{options.bind}:{options.port}", flush=True)
    print(f"admin password: {options.password}", flush=True)
    if options.moderator_password:
        print(f"moderator password: {options.moderator_password}", flush=True)
    if options.owner_password:
        print(f"owner password (from loopback): {options.owner_password}", flush=True)
    try:
        server.serve_forever(poll_interval=0.2)
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())

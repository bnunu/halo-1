"""The real parsing of a dedicated server's web port (port/linux/src/server_http.c): a request's head, a connection's
bytes taken a request at a time, the WebSocket handshake (SHA-1, the accept key, its refusals), a client's frames, and
the Origin, address, query and password checks; a connection's output and its limits, the requests held while its
answers wait, the addresses' table, the connection a new one evicts, its deadlines, and a close after the queue; and
the sign-in's decisions (handle_request's Origin, CSRF and roles, the sessions' expiry and cap, the passwords' lockout,
budget and changes), on stand-ins for the HMAC, the random bytes and the settings."""

import hashlib
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, mutated, read, run, structure  # noqa: E402

CASES = [
    "sha1", "accept-key", "upgrade-refusals", "head", "head-refusals", "content-length", "cookie", "connection",
    "forwarded", "addresses", "origin", "query", "compare", "frames", "frame-refusals", "fragments",
    "content-length-duplicate", "output", "pipelining", "address-table", "eviction", "deadlines", "close-order",
    "bare-lf", "auth-csrf", "auth-denied", "route-roles", "session-expiry", "session-cap", "lockout", "login-budget",
    "empty-password", "password-change", "fetch-site", "ws-allowance", "owner-local", "rotation-play",
]
FUNCTIONS = (
    "sha1_rotate", "sha1", "text_equal_case", "constant_time_equal", "address_text", "parse_address", "hex_digit_value",
    "parse_address6", "port_text", "forwarded_address", "out_pending", "out_append", "server_http_parse_head",
    "query_number", "connection_is_local", "address_record", "address_allows_request", "address_locked",
    "address_login_failed",
    "session_hash", "session_find", "session_new", "passwords_check", "password_role", "request_host",
    "origin_matches", "ws_frame", "ws_close", "ws_read_frames", "request_session", "body_string", "rotation_play_line",
    "serve_login",
    "route_needs_game", "route_role", "handle_request", "server_http_capabilities", "handle_upgrade", "connection_take",
    "connection_to_evict",
    "connection_overdue",
)
STRUCTURES = ("http_request", "ws_message", "connection", "server_ticket", "session", "address_record")
# the messages hashed against Python's SHA-1: every length across two blocks' padding
SHA1_LENGTHS = 200

# a fault in server_http.c, the function it is in, and the case that must catch it
NEGATIVE_CONTROLS = {
    "null-origin-taken": (('return !strcmp(connection->request.fetch_site, "same-origin");', "return 1;"),
                          "origin_matches", "origin"),
    "sha1-bytes-not-bits": (("bits = (unsigned long long)size * 8;", "bits = (unsigned long long)size;"), "sha1",
                            "sha1"),
    "sha1-block-short": (("unsigned long total = size + 9 + 63;", "unsigned long total = size + 8 + 63;"), "sha1",
                         "sha1"),
    "accept-guid-wrong": (("258EAFA5-E914-47DA-95CA-C5AB0DC85B11", "258EAFA5-E914-47DA-95CA-C5AB0DC85B12"),
                          "handle_upgrade", "accept-key"),
    "cookie-from-anywhere": (("if (session && (!by_cookie || origin_matches(connection)))", "if (session)"),
                             "handle_upgrade", "accept-key"),
    "upgrade-origin-unchecked": (("if (request->origin[0] && !origin_matches(connection))", "if (0)"),
                                 "handle_upgrade", "upgrade-refusals"),
    "websockets-unbounded": (("if (count >= MAXIMUM_WEBSOCKETS)", "if (count > MAXIMUM_WEBSOCKETS)"),
                             "handle_upgrade", "upgrade-refusals"),
    "names-case-sensitive": (("if (tolower((unsigned char)*a) != tolower((unsigned char)*b))", "if (*a != *b)"),
                             "text_equal_case", "head"),
    "transfer-encoding-taken": (("if (request->has_transfer_encoding || request->has_expect)",
                                 "if (request->has_expect)"), "server_http_parse_head", "head-refusals"),
    "dot-dot-taken": (("strstr(target, \"..\") ||", ""), "server_http_parse_head", "head-refusals"),
    "length-signs-taken": (("if (!value[0] || !isdigit((unsigned char)value[0]) || has_content_length)",
                            "if (!value[0] || has_content_length)"), "server_http_parse_head", "content-length"),
    "length-digits-unbounded": (("if (number < 0 || value_length > 9)", "if (number < 0)"), "server_http_parse_head",
                                "content-length"),
    "length-junk-too-large": (("if (*number_end)\n\t\t\t\t\treturn 0;", "if (0)\n\t\t\t\t\treturn 0;"),
                              "server_http_parse_head", "content-length"),
    "second-length-taken": (("has_content_length = 1;", "has_content_length = 0;"), "server_http_parse_head", "content-length-duplicate"),
    "body-unbounded": (("if (request->content_length > MAXIMUM_BODY)", "if (0)"), "server_http_parse_head",
                       "content-length"),
    "cookie-inside-names": (("while (found && found != value && found[-1] != ' ' && found[-1] != ';')", "while (0)"),
                            "server_http_parse_head", "cookie"),
    "unended-head-waited-for": (("if (connection->in_length > MAXIMUM_HEAD)",
                                 "if (connection->in_length > 2 * MAXIMUM_HEAD)"), "connection_take", "connection"),
    "head-past-most-taken": (("if (connection->head_length > MAXIMUM_HEAD ||",
                              "if (connection->head_length > MAXIMUM_HEAD + 2 ||"), "connection_take", "connection"),
    "proxy-unchecked": (("parse_address(config_string(\"server.web_trusted_proxy\"), &proxy) && "
                         "proxy == connection->address", "1"), "connection_take", "forwarded"),
    "forwarded-flag-from-anyone": (("connection->forwarded = 0;", "connection->forwarded = 1;"), "connection_take",
                                   "forwarded"),
    "ipv6-unread": (("else if (colon && strchr(colon + 1, ':'))", "else if (0)"), "forwarded_address", "forwarded"),
    "ipv6-whole-address-keyed": (("bytes, 8, digest);", "bytes, 16, digest);"), "forwarded_address", "forwarded"),
    "ipv4-mapped-hashed": (('if (!memcmp(bytes, "\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\xff\\xff", 12))', "if (0)"),
                           "forwarded_address", "forwarded"),
    "ipv6-two-gaps": (("if (gap >= 0)\n\t\t\t\treturn 0;", "if (0)\n\t\t\t\treturn 0;"), "parse_address6", "forwarded"),
    "ports-unbounded": (("strtol(text + 1, NULL, 10) <= 65535", "strtol(text + 1, NULL, 10) <= 99999"), "port_text",
                        "forwarded"),
    "bare-lf-taken": (("else if (line_end == line || line_end[-1] != '\\r')\n\t\t\treturn 0;",
                       "else if (0)\n\t\t\treturn 0;"), "server_http_parse_head", "bare-lf"),
    "bare-cr-taken": (("if (memchr(line, '\\r', (size_t)line_length))", "if (0)"), "server_http_parse_head", "bare-lf"),
    "after-empty-line-taken": (("if (line < end)\n\t\t\t\treturn 0;", "if (0)\n\t\t\t\treturn 0;"),
                               "server_http_parse_head", "bare-lf"),
    "cookie-change-without-origin": (("if (!request->origin[0])\n\t\t{", "if (0)\n\t\t{"), "handle_request",
                                     "auth-csrf"),
    "cookie-change-without-csrf": (("if (!constant_time_equal(request->csrf, session->csrf))", "if (0)"),
                                   "handle_request", "auth-csrf"),
    "bearer-needs-csrf": (("if (mutation && by_cookie)", "if (mutation)"), "handle_request", "auth-csrf"),
    "no-session-taken": (("return connection->request.session_cookie[0] ? session_find(connection->request.session_cookie) "
                          ": NULL;", "return http.sessions[0].used ? &http.sessions[0] : NULL;"), "request_session",
                         "auth-denied"),
    "moderator-routes-viewer": (("return _server_role_moderator;", "return _server_role_viewer;"), "route_role",
                                "route-roles"),
    "admin-routes-moderator": (("if (!strncmp(path, \"/api/bans\", 9) || !strcmp(path, \"/api/settings\"))\n\t\treturn "
                                "_server_role_admin;", "if (!strncmp(path, \"/api/bans\", 9) || !strcmp(path, "
                                "\"/api/settings\"))\n\t\treturn _server_role_moderator;"), "route_role", "route-roles"),
    "roles-unchecked": (("if (session->role < route_role(request->method, path))", "if (0)"), "handle_request",
                        "route-roles"),
    "sessions-forever": (("if (now - session->created > SESSION_SECONDS || now - session->used_at > SESSION_IDLE_SECONDS)",
                          "if (now - session->used_at > SESSION_IDLE_SECONDS)"), "session_find", "session-expiry"),
    "sessions-never-idle": (("if (now - session->created > SESSION_SECONDS || now - session->used_at > SESSION_IDLE_SECONDS)",
                             "if (now - session->created > SESSION_SECONDS)"), "session_find", "session-expiry"),
    "sessions-evict-newest": (("if (http.sessions[index].used_at < oldest->used_at)",
                               "if (http.sessions[index].used_at > oldest->used_at)"), "session_new", "session-cap"),
    "lockout-late": (("if (++record->failures >= LOGIN_FAILURES)", "if (++record->failures > LOGIN_FAILURES)"),
                     "address_login_failed", "lockout"),
    "lockout-windowless": (("if (now - record->failures_start >= LOGIN_WINDOW_SECONDS)", "if (0)"),
                           "address_login_failed", "lockout"),
    "lockout-ignored": (("if (locked)\n\t{", "if (0)\n\t{"), "password_role", "lockout"),
    "budget-unbounded": (("if (++http.budget_used > LOGIN_BUDGET)", "if (++http.budget_used > 1000)"), "password_role",
                         "login-budget"),
    "empty-password-matches": (("if (password && password[0])\n\t{\n\t\tif (admin[0] && constant_time_equal(password, admin))",
                                "if (password)\n\t{\n\t\tif (constant_time_equal(password, admin))"), "password_role",
                               "empty-password"),
    "password-change-unseen": (("changed |= 1UL << role;", "changed |= 0;"), "passwords_check", "password-change"),
    "password-change-every-role": (("(changed & (1UL << http.sessions[index].role)))", "changed)"), "passwords_check",
                                   "password-change"),
    "password-change-console-code": (("ws_close(connection, 1008, \"the password changed\");",
                                      "ws_close(connection, 1000, \"the password changed\");"), "passwords_check",
                                     "password-change"),
    "fetch-site-unread": (("HEADER(\"Sec-Fetch-Site\", fetch_site);", "HEADER(\"Sec-Fetch-Mode\", fetch_site);"),
                          "server_http_parse_head", "fetch-site"),
    "fetch-site-any": (('return !strcmp(connection->request.fetch_site, "same-origin");',
                        'return connection->request.fetch_site[0] != 0;'), "origin_matches", "fetch-site"),
    "upgrades-uncounted": (("if (!address_allows_request(connection->real_address, session != NULL))", "if (0)"),
                           "handle_upgrade", "ws-allowance"),
    "forwarded-leftmost": (("const char *last = strrchr(list, ',');", "const char *last = NULL;"), "forwarded_address",
                           "forwarded"),
    "forwarded-start-kept": (("value_length < sizeof(request->forwarded_for) ? value :", "1 ? value :"),
                             "server_http_parse_head", "forwarded"),
    "output-unbounded": (("if (out_pending(connection) > OUTPUT_PAUSE && out_pending(connection) + size > OUTPUT_MOST)",
                          "if (0)"), "out_append", "output"),
    "output-not-compacted": (("connection->out_length -= connection->out_sent;\n\t\tconnection->out_sent = 0;",
                              "connection->out_length -= connection->out_sent;"), "out_append", "output"),
    "pipelining-unpaused": (("while (!connection->ticket && !connection->close_after_write && "
                             "out_pending(connection) < OUTPUT_PAUSE)",
                             "while (!connection->ticket && !connection->close_after_write)"), "connection_take",
                            "pipelining"),
    "frames-unpaused": (("while (connection->in_length >= 2 && !connection->closing && "
                         "out_pending(connection) < OUTPUT_PAUSE)",
                         "while (connection->in_length >= 2 && !connection->closing)"), "ws_read_frames", "pipelining"),
    "addresses-first-unlocked": (("else if (!unlocked || record->last_seen < unlocked->last_seen)",
                                  "else if (!unlocked)"), "address_record", "address-table"),
    "addresses-first-locked": (("if (!locked || record->locked_until < locked->locked_until)", "if (!locked)"),
                               "address_record", "address-table"),
    "addresses-unseen": (("record->last_seen = now;\n\treturn record;", "return record;"), "address_record",
                         "address-table"),
    "evicts-busy": (("if (connection->socket < 0 || connection->websocket || connection->ticket || "
                     "out_pending(connection) > 0)", "if (connection->socket < 0)"), "connection_to_evict",
                    "eviction"),
    "socket-zero-free": (("if (connection->socket < 0 || connection->websocket", "if (connection->socket <= 0 || "
                          "connection->websocket"), "connection_to_evict", "eviction"),
    "evicts-newest": (("connection->head_started < http.connections[oldest].head_started",
                       "connection->head_started > http.connections[oldest].head_started"), "connection_to_evict",
                      "eviction"),
    "silent-connections-kept": (("if (!connection->websocket && !connection->ticket && out_pending(connection) == 0 &&",
                                 "if (!connection->websocket && !connection->ticket && out_pending(connection) == 0 && "
                                 "connection->in_length > 0 &&"), "connection_overdue", "deadlines"),
    "closing-websockets-kept": (("!(connection->websocket && connection->role != _server_role_none && "
                                 "!connection->closing)",
                                 "!(connection->websocket && connection->role != _server_role_none)"),
                                "connection_overdue", "deadlines"),
    "close-before-queue": (("while (connection->queue_head)", "while (0)"), "ws_close", "close-order"),
    "address-parts-unbounded": (("if (!digits || parts[index] > 255 ||", "if (!digits ||"), "parse_address",
                                "addresses"),
    "origin-prefix": (("return text_equal_case(after_scheme + 3, host);",
                       "return !strncasecmp(after_scheme + 3, host, strlen(host));"), "origin_matches", "origin"),
    "forwarded-host-from-anyone": (("if (connection->request.forwarded_host[0] && connection->forwarded)",
                                    "if (connection->request.forwarded_host[0])"), "request_host", "origin"),
    "query-inside-names": (("at = strchr(at, '&');\n\t\tif (at)\n\t\t\tat++;", "at++;"), "query_number", "query"),
    "compare-prefix": (("unsigned char difference = (unsigned char)(length_a != length_b);",
                        "unsigned char difference = 0;"), "constant_time_equal", "compare"),
    "unmasked-taken": (("if (!(data[1] & 0x80))", "if (0)"), "ws_read_frames", "frame-refusals"),
    "control-frames-long": (("if (opcode >= 0x8 && (size > 125 || !fin))", "if (opcode >= 0x8 && !fin)"),
                            "ws_read_frames", "frame-refusals"),
    "frames-unbounded": (("if (size > WS_FRAME)", "if (size > WS_FRAME + 1)"), "ws_read_frames", "frame-refusals"),
    "pong-empty": (("ws_frame(connection, 0xA, data + head, (int)size);", "ws_frame(connection, 0xA, data + head, 0);"),
                   "ws_read_frames", "frames"),
    "messages-interleaved": (("if ((opcode == 0x1) == (connection->message_open != 0))",
                              "if (opcode == 0x0 && !connection->message_open)"), "ws_read_frames", "fragments"),
    "messages-unbounded": (("if (connection->message_length + (int)size > WS_MESSAGE)", "if (0)"), "ws_read_frames",
                           "fragments"),
    "owner-from-anywhere": (("if (local)\n\t\t\treturn _server_role_owner;", "if (1)\n\t\t\treturn _server_role_owner;"),
                            "password_role", "owner-local"),
    "owner-through-proxy": (("return (connection->address >> 24) == 127 && !connection->forwarded;",
                             "return (connection->address >> 24) == 127;"), "connection_is_local", "owner-local"),
    "owner-session-anywhere": (("if (session && session->role >= _server_role_owner && !connection_is_local(connection))"
                                "\n\t\tsession = NULL;", "if (0)\n\t\tsession = NULL;"), "handle_request", "owner-local"),
    "owner-elsewhere-uncounted": (("address_login_failed(address, address_shown);\n\tif (owner_elsewhere)",
                                   "if (!owner_elsewhere)\n\t\taddress_login_failed(address, address_shown);\n\t"
                                   "if (owner_elsewhere)"), "password_role", "owner-local"),
    "owner-password-change-unseen": (('"server.web_owner_password" };', '"server.web_password" };'), "passwords_check",
                                     "owner-local"),
    "play-entries-short": (("char map[SERVER_ENTRY_TEXT_SIZE]", "char map[128]"), "rotation_play_line", "rotation-play"),
}


def generated(fault=None, faulty_function=None):
    source = read("port/linux/src/server_http.c")
    state = re.search(r"^static struct\s*\{.*?^\} http;", source, re.M | re.S)
    if not state:
        raise LookupError("server_http.c's state (static struct { ... } http;) not found")
    config = enum_with(source, "MAXIMUM_HEAD = ") + "\n"
    config += enum_with(source, "_ticket_api") + "\n"
    config += enum_with(read("port/linux/src/server_console.h"), "_server_role_none") + "\n"
    config += f"enum {{ P2P_SHA256_SIZE = {constant(read('port/linux/src/p2p_internal.h'), 'P2P_SHA256_SIZE')}, "
    config += f"SERVER_EVENT_GROUP_SIZE = {constant(read('port/linux/src/server_events.h'), 'SERVER_EVENT_GROUP_SIZE')}, "
    config += f"SERVER_ENTRY_TEXT_SIZE = {constant(read('port/linux/src/server_rotation.h'), 'SERVER_ENTRY_TEXT_SIZE')} }};\n"
    config += "struct push;\n"
    for name in STRUCTURES:
        config += structure(source, name) + "\n"
    config += state.group(0) + "\n"
    config += function(read("port/linux/src/p2p.c"), "p2p_hex") + "\n"
    message = bytes((index * 7 + 3) & 255 for index in range(SHA1_LENGTHS))
    config += f"#define SHA1_LENGTHS {SHA1_LENGTHS}\nstatic const char *const sha1_digests[] = {{\n"
    config += "".join(f'\t"{hashlib.sha1(message[:length]).hexdigest()}",\n' for length in range(SHA1_LENGTHS))
    config += "};\n"
    json = mutated(read("port/linux/src/server_json.c"), '#include "../../third_party/jsmn/jsmn.h"', '#include "jsmn.h"')
    text = ""
    for name in FUNCTIONS:
        code = function(source, name)
        if fault and name == faulty_function:
            code = mutated(code, *fault)
        # (connection_take's cases hand their requests to the test's handle_request; the sign-in's call the real one)
        if name == "handle_request":
            code = mutated(code, "static void handle_request(", "static void real_handle_request(")
        text += code + "\n"
    return (("server_json.h", read("port/linux/src/server_json.h")), ("jsmn.h", read("port/third_party/jsmn/jsmn.h")),
            ("config.inc", config), ("server_json.inc", json), ("under_test.inc", text))


@pytest.mark.parametrize("case", CASES)
def test_case(case):
    status, output = run(build("server_http", generated()), case)
    assert status == 0, output


@pytest.mark.parametrize("control", NEGATIVE_CONTROLS)
def test_negative_control(control):
    fault, faulty_function, case = NEGATIVE_CONTROLS[control]
    status, output = run(build("server_http", generated(fault, faulty_function)), case)
    assert status == CHECK_FAILED, f"server_http.c with a fault ({control}) passed '{case}': the test cannot see it"

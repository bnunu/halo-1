/*
SERVER_HTTP.C

A dedicated server's web port (server_http.h; docs/dedicated_server.md,
"HTTP" and "The RCON"): HTTP/1.1 and WebSocket on non-blocking sockets,
served by a thread of its own. What only reads the game is answered from
the snapshots the game's thread leaves every half second (the status, the
players, the health); what needs the game is a ticket the game's thread
runs between frames (server_http_main_thread), its answer sent when it is
done, or "503 loading" after 20 seconds (a map loading holds the game's
thread). Every request is untrusted: its head, body, frames and messages
are bounded, its session, role, CSRF token and Origin checked, its address
rate limited, and its password guesses locked out.
*/

#include "platform.h"
#include "port_config.h"
#include "server_http.h"
#include "server_mode.h"

#if HALO_SERVER_BUILD

#include "p2p_internal.h"
#include "posix.h"
#include "posix_server.h"
#include "server_console.h"
#include "server_events.h"
#include "server_json.h"
#include "server_log.h"
#include "server_master.h"
#include "server_netlist.h"
#include "server_rotation.h"
#include "server_toml.h"

#include <SDL3/SDL.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- the game side's (port/linux/game/server_*.c) */

void server_status_write(struct server_json *json, int role);
void server_status_players(struct server_json *json, int role);
void server_status_maps(struct server_json *json);
void server_status_gametypes(struct server_json *json);
void server_status_rotation(struct server_json *json);
short server_director_state(void);
char const *server_status_state_name(short state);
char *server_commands_run(char const *line, int role, int origin, unsigned char *succeeded);
int server_director_set_rotation_text(char const *text, struct server_json *rejected);
int server_commands_set_setting_text(char const *name, char const *value, int role, char *reason, long reason_size,
	int *code);
int server_commands_ban_address_text(char const *ip, char const *hwid, char const *discord_id, char const *names,
	char const *duration, char const *reason, int role, char *message, long message_size, long *number);
void server_commands_settings_json(struct server_json *json);
void server_commands_bans_json(struct server_json *json);
int server_status_player_matches(long slot, char const *name, char *current, long size);
int server_gametype_check_text(char const *text, int length, char *reason, long reason_size, int *line);
void server_gametype_files_json(struct server_json *json);

/* the admin page's files (tools/server_ui_embed.py: port/assets/server_ui) */
struct server_ui_asset
{
	const char *path;
	const char *content_type;
	const unsigned char *data;
	unsigned long size;
	const char *etag;
};
extern const struct server_ui_asset server_ui_assets[];
extern const unsigned long server_ui_asset_count;

/* ---------- limits */

enum
{
	MAXIMUM_CONNECTIONS = 24,
	MAXIMUM_WEBSOCKETS = 8,
	MAXIMUM_HEAD = 8 * 1024,
	MAXIMUM_BODY = 64 * 1024,
	/* a connection's answers waiting for its client: past the first, its
	requests are not read; past the second, it is closed (one answer,
	however long, is taken onto output below the first) */
	OUTPUT_PAUSE = 256 * 1024,
	OUTPUT_MOST = 2 * 1024 * 1024,
	HEAD_SECONDS = 10,
	IDLE_SECONDS = 60,
	TICKET_SECONDS = 20,
	AUTH_SECONDS = 10,
	WS_FRAME = 64 * 1024,
	WS_MESSAGE = 256 * 1024,
	WS_QUEUE = 1024,
	MAXIMUM_SESSIONS = 16,
	SESSION_SECONDS = 12 * 60 * 60,
	SESSION_IDLE_SECONDS = 30 * 60,
	TOKEN_BYTES = 32,
	CSRF_BYTES = 16,
	/* five wrong passwords from an address in ten minutes lock it for ten */
	LOGIN_FAILURES = 5,
	LOGIN_WINDOW_SECONDS = 10 * 60,
	LOGIN_LOCK_SECONDS = 10 * 60,
	/* passwords checked a minute, in all */
	LOGIN_BUDGET = 30,
	MAXIMUM_ADDRESSES = 128,
	REQUESTS_UNAUTHENTICATED = 60,
	REQUESTS_AUTHENTICATED = 600,
	COMMANDS_PER_SECOND = 32,
	COMMANDS_BURST = 64,
	SNAPSHOT_MILLISECONDS = 500,
	STATUS_EVENT_MILLISECONDS = 2000,
	TICKETS_PER_FRAME = 16,
	NAME_SIZE = 33,
};

/* the event groups (subscribe's) */
static const char *const event_groups[] = { "player", "chat", "game", "moderation", "status", "log", "master", "server" };
#define NUMBER_OF_EVENT_GROUPS ((int)(sizeof(event_groups) / sizeof(event_groups[0])))

/* ---------- SHA-1 (FIPS 180-4), for the WebSocket handshake alone */

static unsigned long sha1_rotate(unsigned long value, int bits)
{
	return ((value << bits) | (value >> (32 - bits))) & 0xFFFFFFFFUL;
}

static void sha1(const unsigned char *data, unsigned long size, unsigned char digest[20])
{
	unsigned long state[5] = { 0x67452301UL, 0xEFCDAB89UL, 0x98BADCFEUL, 0x10325476UL, 0xC3D2E1F0UL };
	unsigned long total = size + 9 + 63;
	unsigned long blocks = total / 64;
	unsigned long block;

	for (block = 0; block < blocks; block++)
	{
		unsigned char chunk[64];
		unsigned long words[80];
		unsigned long a, b, c, d, e;
		int index;

		for (index = 0; index < 64; index++)
		{
			unsigned long position = block * 64 + (unsigned long)index;

			if (position < size)
				chunk[index] = data[position];
			else if (position == size)
				chunk[index] = 0x80;
			else if (position >= blocks * 64 - 8)
			{
				unsigned long long bits = (unsigned long long)size * 8;

				chunk[index] = (unsigned char)(bits >> (8 * (blocks * 64 - 1 - position)));
			}
			else
				chunk[index] = 0;
		}
		for (index = 0; index < 16; index++)
		{
			words[index] = (unsigned long)chunk[4 * index] << 24 | (unsigned long)chunk[4 * index + 1] << 16 |
				(unsigned long)chunk[4 * index + 2] << 8 | chunk[4 * index + 3];
		}
		for (index = 16; index < 80; index++)
			words[index] = sha1_rotate(words[index - 3] ^ words[index - 8] ^ words[index - 14] ^ words[index - 16], 1);
		a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
		for (index = 0; index < 80; index++)
		{
			unsigned long f, k, temporary;

			if (index < 20) { f = (b & c) | (~b & d); k = 0x5A827999UL; }
			else if (index < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1UL; }
			else if (index < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCUL; }
			else { f = b ^ c ^ d; k = 0xCA62C1D6UL; }
			temporary = (sha1_rotate(a, 5) + (f & 0xFFFFFFFFUL) + e + k + words[index]) & 0xFFFFFFFFUL;
			e = d; d = c; c = sha1_rotate(b, 30); b = a; a = temporary;
		}
		state[0] = (state[0] + a) & 0xFFFFFFFFUL;
		state[1] = (state[1] + b) & 0xFFFFFFFFUL;
		state[2] = (state[2] + c) & 0xFFFFFFFFUL;
		state[3] = (state[3] + d) & 0xFFFFFFFFUL;
		state[4] = (state[4] + e) & 0xFFFFFFFFUL;
	}
	for (block = 0; block < 5; block++)
	{
		digest[4 * block] = (unsigned char)(state[block] >> 24);
		digest[4 * block + 1] = (unsigned char)(state[block] >> 16);
		digest[4 * block + 2] = (unsigned char)(state[block] >> 8);
		digest[4 * block + 3] = (unsigned char)state[block];
	}
}

/* ---------- structures */

struct http_request
{
	char method[8];
	char path[256];
	char query[512];
	int minor_version;
	char host[256];
	char origin[256];
	/* (the browser's word on the request's site: same-origin...) */
	char fetch_site[32];
	char session_cookie[2 * TOKEN_BYTES + 1];
	char authorization[160];
	char csrf[2 * CSRF_BYTES + 1];
	char upgrade[32];
	char connection[64];
	char websocket_key[64];
	char websocket_version[8];
	char websocket_protocol[128];
	char forwarded_for[64];
	char forwarded_proto[16];
	char forwarded_host[256];
	char if_none_match[64];
	long content_length;
	int has_transfer_encoding;
	int has_expect;
	int keep_alive;
};

/* a message queued for a WebSocket */
struct ws_message
{
	struct ws_message *next;
	int length;
	char text[1];
};

struct server_ticket;

struct connection
{
	/* -1: the slot is free */
	int socket;
	/* host byte order: the peer's, and the client's (behind the trusted
	proxy, the address it forwarded: an IPv6 one's key, forwarded_address),
	with its text for the log */
	unsigned long address;
	unsigned long real_address;
	char real_text[64];
	/* the request came from the trusted proxy: its X-Forwarded-Host and
	X-Forwarded-Proto are its word */
	int forwarded;
	int websocket;
	unsigned char *in;
	int in_length;
	unsigned char *out;
	int out_length;
	int out_sent;
	int out_capacity;
	/* its output passed OUTPUT_MOST: closed at once */
	int out_overflowed;
	Uint64 head_started;
	Uint64 last_activity;
	int close_after_write;
	/* a request whose head is read, its body coming */
	int have_head;
	int head_length;
	struct http_request request;
	struct server_ticket *ticket;
	/* a WebSocket's */
	int role;
	char by[NAME_SIZE];
	unsigned long groups;
	unsigned char *message;
	int message_length;
	int message_open;
	struct ws_message *queue_head, *queue_tail;
	int queued;
	int overflowed;
	double bucket;
	Uint64 bucket_time;
	Uint64 auth_deadline;
	int closing;
};

enum
{
	_ticket_api,
	_ticket_ws_command,
};

struct server_ticket
{
	struct server_ticket *next;
	int kind;
	int role;
	char by[NAME_SIZE];
	char method[8];
	char path[256];
	char *body;
	int body_length;
	/* the WebSocket command's id, as the client gave it (JSON text) */
	char id[32];
	/* the answer */
	int status;
	char *response;
	int retry_after;
	int done;
	Uint64 deadline;
	/* its connection gave it up (it closed, or was answered "loading" when
	the deadline passed): from then the ticket is the game's thread's, which
	frees it once it has run it, and the HTTP thread never reads it again */
	int abandoned;
};

struct session
{
	int used;
	unsigned char hash[P2P_SHA256_SIZE];
	char csrf[2 * CSRF_BYTES + 1];
	int role;
	char by[NAME_SIZE];
	unsigned long created;
	unsigned long used_at;
};

struct address_record
{
	unsigned long address;
	unsigned long window_start;
	int requests;
	unsigned long failures_start;
	int failures;
	unsigned long locked_until;
	unsigned long last_seen;
};

/* an event or log line for the WebSockets, from another thread: its text,
and after it (at addressed, 0 for none) its text with the players'
addresses */
struct push
{
	struct push *next;
	char group[SERVER_EVENT_GROUP_SIZE];
	int length;
	int addressed;
	char text[1];
};

static struct
{
	int started;
	int listening;
	SDL_Thread *thread;
	SDL_Mutex *lock;
	struct connection connections[MAXIMUM_CONNECTIONS];
	struct session sessions[MAXIMUM_SESSIONS];
	struct address_record addresses[MAXIMUM_ADDRESSES];
	unsigned char session_key[32];
	unsigned long budget_minute;
	int budget_used;
	/* the web passwords' hashes (keyed by session_key), a role's each, as
	config_changes() last read them: a change signs that role out */
	unsigned char password_hashes[_server_role_owner + 1][P2P_SHA256_SIZE];
	unsigned long password_changes;
	int passwords_known;
	/* under lock: the tickets waiting for the game's thread, and those it
	ran */
	struct server_ticket *pending;
	/* under lock: the snapshots the game's thread leaves */
	char *status_viewer;
	char *status_admin;
	char *players_viewer;
	char *players_admin;
	char *players_owner;
	char state[16];
	Uint64 snapshot_time;
	long metrics_players, metrics_max_players, metrics_machines, metrics_games;
	double metrics_frame_longest, metrics_frame_mean;
	/* under lock: what is to go to the WebSockets */
	struct push *pushes_head, *pushes_tail;
	int pushes;
	unsigned long pushes_dropped;
	/* the HTTP thread's */
	Uint64 last_status_event;
	unsigned long events_dropped;
	unsigned long requests_total;
	unsigned long logins_failed;
	/* the game's thread's */
	Uint64 last_snapshot;
} http;

/* ---------- small things */

static unsigned long now_seconds(void)
{
	return posix_server_unix_seconds();
}

static int text_equal_case(const char *a, const char *b)
{
	for (; *a && *b; a++, b++)
	{
		if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
			return 0;
	}
	return !*a && !*b;
}

static int constant_time_equal(const char *a, const char *b)
{
	size_t length_a = strlen(a), length_b = strlen(b), index;
	unsigned char difference = (unsigned char)(length_a != length_b);

	for (index = 0; index < length_a; index++)
		difference |= (unsigned char)(a[index] ^ b[index % (length_b ? length_b : 1)]);
	return difference == 0;
}

static void address_text(unsigned long address, char *text, int size)
{
	snprintf(text, (size_t)size, "%lu.%lu.%lu.%lu", (address >> 24) & 255, (address >> 16) & 255, (address >> 8) & 255,
		address & 255);
}

/* whether a connection is the server's own machine's: its socket's peer in
127.0.0.0/8, and not the trusted proxy's (a proxy on this machine passes
others' requests on: the owner's role is never given through it) */
static int connection_is_local(struct connection *connection)
{
	return (connection->address >> 24) == 127 && !connection->forwarded;
}

/* whether a reader of the role sees the players' addresses in the events:
the owner, or every role with server.log_addresses (as /api/players) */
static int role_sees_addresses(int role)
{
	return role >= _server_role_owner || config_boolean("server.log_addresses");
}

static int parse_address(const char *text, unsigned long *address)
{
	unsigned long parts[4];
	int index;

	for (index = 0; index < 4; index++)
	{
		int digits = 0;

		parts[index] = 0;
		while (*text >= '0' && *text <= '9' && digits < 3)
		{
			parts[index] = parts[index] * 10 + (unsigned long)(*text++ - '0');
			digits++;
		}
		if (!digits || parts[index] > 255 || (index < 3 && *text++ != '.'))
			return 0;
	}
	*address = parts[0] << 24 | parts[1] << 16 | parts[2] << 8 | parts[3];
	return *text == 0 || *text == ',' || *text == ' ';
}

static int hex_digit_value(char character)
{
	if (character >= '0' && character <= '9')
		return character - '0';
	if (character >= 'a' && character <= 'f')
		return character - 'a' + 10;
	if (character >= 'A' && character <= 'F')
		return character - 'A' + 10;
	return -1;
}

/* an IPv6 address's text, length characters (RFC 4291 2.2: up to eight
groups of up to four hex digits, one "::" for a run of zero groups, and a
dotted IPv4 address for the last two), into its 16 bytes: 0 if it is none
(a zone is none) */
static int parse_address6(const char *text, int length, unsigned char *bytes)
{
	unsigned char parsed[16];
	int count = 0, gap = -1, at = 0;

	if (length >= 2 && text[0] == ':' && text[1] == ':')
	{
		gap = 0;
		at = 2;
	}
	else if (length < 1 || text[0] == ':')
	{
		return 0;
	}
	while (at < length)
	{
		unsigned long group = 0;
		int digits = 0;

		/* (the last 32 bits as a dotted IPv4 address) */
		if (memchr(text + at, '.', (size_t)(length - at)) && !memchr(text + at, ':', (size_t)(length - at)))
		{
			char dotted[16];
			unsigned long ipv4;
			int rest = length - at;

			if (count > 12 || rest >= (int)sizeof(dotted))
				return 0;
			memcpy(dotted, text + at, (size_t)rest);
			dotted[rest] = 0;
			if ((int)strspn(dotted, "0123456789.") != rest || !parse_address(dotted, &ipv4))
				return 0;
			parsed[count++] = (unsigned char)(ipv4 >> 24);
			parsed[count++] = (unsigned char)(ipv4 >> 16);
			parsed[count++] = (unsigned char)(ipv4 >> 8);
			parsed[count++] = (unsigned char)ipv4;
			break;
		}
		while (at < length && digits < 4 && hex_digit_value(text[at]) >= 0)
		{
			group = group << 4 | (unsigned long)hex_digit_value(text[at++]);
			digits++;
		}
		if (!digits || count >= 16)
			return 0;
		parsed[count++] = (unsigned char)(group >> 8);
		parsed[count++] = (unsigned char)group;
		if (at == length)
			break;
		if (text[at++] != ':' || at == length)
			return 0;
		if (text[at] == ':')
		{
			if (gap >= 0)
				return 0;
			gap = count;
			at++;
		}
	}
	if (gap < 0 ? count != 16 : count > 14)
		return 0;
	memset(bytes, 0, 16);
	if (gap < 0)
		gap = count;
	memcpy(bytes, parsed, (size_t)gap);
	memcpy(bytes + 16 - (count - gap), parsed + gap, (size_t)(count - gap));
	return 1;
}

/* ":<port>", a port of 0 to 65535 */
static int port_text(const char *text)
{
	size_t digits = strspn(text + 1, "0123456789");

	return text[0] == ':' && digits >= 1 && digits <= 5 && !text[1 + digits] && strtol(text + 1, NULL, 10) <= 65535;
}

/* the address the trusted proxy put in X-Forwarded-For: its last entry,
the one it appended (those before it are the client's word, which anyone
may forge), as the addresses' table's key, and its text: an IPv4 address
("a.b.c.d", or "a.b.c.d:port") itself, an IPv6 one ("v6", "[v6]" or
"[v6]:port") a keyed hash of its /64 (one host's addresses, as many as it
likes, share its limits and lockouts), an IPv4-mapped one its IPv4
address; 0 if the entry is none of these */
static int forwarded_address(const char *list, unsigned long *address, char *text, int size)
{
	const char *last = strrchr(list, ',');
	unsigned char bytes[16];
	char entry[64];
	char *colon;
	int length;

	last = last ? last + 1 : list;
	while (*last == ' ' || *last == '\t')
		last++;
	length = (int)strlen(last);
	while (length && (last[length - 1] == ' ' || last[length - 1] == '\t'))
		length--;
	if (!length || length >= (int)sizeof(entry))
		return 0;
	memcpy(entry, last, (size_t)length);
	entry[length] = 0;
	colon = strchr(entry, ':');
	if (entry[0] == '[')
	{
		char *close = strchr(entry, ']');

		if (!close || !parse_address6(entry + 1, (int)(close - entry - 1), bytes) || (close[1] && !port_text(close + 1)))
			return 0;
		*close = 0;
		snprintf(text, (size_t)size, "%s", entry + 1);
	}
	else if (colon && strchr(colon + 1, ':'))
	{
		if (!parse_address6(entry, length, bytes))
			return 0;
		snprintf(text, (size_t)size, "%s", entry);
	}
	else
	{
		if (colon)
		{
			if (!port_text(colon))
				return 0;
			*colon = 0;
		}
		if (strspn(entry, "0123456789.") != strlen(entry) || !parse_address(entry, address))
			return 0;
		address_text(*address, text, size);
		return 1;
	}
	if (!memcmp(bytes, "\0\0\0\0\0\0\0\0\0\0\xff\xff", 12))
	{
		*address = (unsigned long)bytes[12] << 24 | (unsigned long)bytes[13] << 16 | (unsigned long)bytes[14] << 8 |
			bytes[15];
		address_text(*address, text, size);
		return 1;
	}
	{
		unsigned char digest[P2P_SHA256_SIZE];

		p2p_hmac_sha256(http.session_key, sizeof(http.session_key), bytes, 8, digest);
		*address = (unsigned long)digest[0] << 24 | (unsigned long)digest[1] << 16 | (unsigned long)digest[2] << 8 |
			digest[3];
	}
	return 1;
}

/* ---------- output */

/* the bytes written for the client and not yet sent */
static int out_pending(struct connection *connection)
{
	return connection->out_length - connection->out_sent;
}

static void out_append(struct connection *connection, const void *data, int size)
{
	if (size <= 0 || connection->out_overflowed)
		return;
	/* (a client that does not read its answers: past OUTPUT_PAUSE nothing
	more of its is read, so only it fills its output this far) */
	if (out_pending(connection) > OUTPUT_PAUSE && out_pending(connection) + size > OUTPUT_MOST)
	{
		connection->out_overflowed = 1;
		connection->out_length = connection->out_sent = 0;
		connection->close_after_write = 1;
		connection->closing = connection->websocket;
		return;
	}
	if (connection->out_length + size > connection->out_capacity && connection->out_sent > 0)
	{
		/* (what was sent makes room first) */
		memmove(connection->out, connection->out + connection->out_sent, (size_t)out_pending(connection));
		connection->out_length -= connection->out_sent;
		connection->out_sent = 0;
	}
	if (connection->out_length + size > connection->out_capacity)
	{
		int capacity = connection->out_capacity ? connection->out_capacity : 4096;
		unsigned char *grown;

		while (capacity < connection->out_length + size)
			capacity *= 2;
		grown = realloc(connection->out, (size_t)capacity);
		if (!grown)
		{
			connection->close_after_write = 1;
			return;
		}
		connection->out = grown;
		connection->out_capacity = capacity;
	}
	memcpy(connection->out + connection->out_length, data, (size_t)size);
	connection->out_length += size;
}

static const char *status_text(int status)
{
	switch (status)
	{
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 304: return "Not Modified";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 409: return "Conflict";
	case 413: return "Payload Too Large";
	case 426: return "Upgrade Required";
	case 429: return "Too Many Requests";
	case 500: return "Internal Server Error";
	case 503: return "Service Unavailable";
	default: return "OK";
	}
}

/* a whole response: its status, type and body, and extra header lines
(each ending "\r\n") */
static void respond(struct connection *connection, int status, const char *content_type, const void *body, int size,
	const char *extra_headers, int no_store)
{
	char head[2048];
	int length;

	length = snprintf(head, sizeof(head),
		"HTTP/1.1 %d %s\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %d\r\n"
		"Cache-Control: %s\r\n"
		"X-Content-Type-Options: nosniff\r\n"
		/* (same-origin, not no-referrer: under no-referrer a browser sends
		the page's own posts with Origin: null) */
		"Referrer-Policy: same-origin\r\n"
		"Content-Security-Policy: default-src 'self'; frame-ancestors 'none'\r\n"
		"X-Frame-Options: DENY\r\n"
		"Connection: %s\r\n"
		"%s\r\n",
		status, status_text(status), content_type, size, no_store ? "no-store" : "no-cache",
		connection->close_after_write || !connection->request.keep_alive ? "close" : "keep-alive",
		extra_headers ? extra_headers : "");
	if (!connection->request.keep_alive)
		connection->close_after_write = 1;
	out_append(connection, head, length);
	if (strcmp(connection->request.method, "HEAD"))
		out_append(connection, body, size);
}

static void respond_json(struct connection *connection, int status, const char *json, const char *extra_headers)
{
	respond(connection, status, "application/json", json, (int)strlen(json), extra_headers, 1);
}

/* {"ok":false,"error":{"code","message"}} */
static void respond_error(struct connection *connection, int status, const char *code, const char *message,
	int retry_after)
{
	struct server_json json;
	char headers[64] = "";

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 0);
	server_json_object_begin(&json, "error");
	server_json_string(&json, "code", code);
	server_json_string(&json, "message", message);
	if (retry_after > 0)
		server_json_integer(&json, "retry_after", retry_after);
	server_json_object_end(&json);
	server_json_object_end(&json);
	if (retry_after > 0)
		snprintf(headers, sizeof(headers), "Retry-After: %d\r\n", retry_after);
	respond_json(connection, status, server_json_text(&json), headers);
	server_json_free(&json);
}

/* ---------- the request's head (a pure parser: the harness tests it) */

/* the head's lines read into request: 0 if it is not a request this
server takes, with the status and code to answer */
int server_http_parse_head(const char *head, int length, struct http_request *request, int *status, const char **code)
{
	const char *line = head, *end = head + length;
	int line_number = 0;
	int has_content_length = 0;

	memset(request, 0, sizeof(*request));
	request->content_length = 0;
	*status = 400;
	*code = "bad_request";
	while (line < end)
	{
		const char *line_end = memchr(line, '\n', (size_t)(end - line));
		int line_length;
		char text[MAXIMUM_HEAD];

		/* (lines end with CRLF: a bare LF, or a CR inside a line, which a proxy
		in front may read otherwise, is refused) */
		if (!line_end)
			line_end = end;
		else if (line_end == line || line_end[-1] != '\r')
			return 0;
		line_length = (int)(line_end - line);
		if (line_length > 0 && line[line_length - 1] == '\r')
			line_length--;
		if (memchr(line, '\r', (size_t)line_length))
			return 0;
		memcpy(text, line, (size_t)line_length);
		text[line_length] = 0;
		line = line_end + 1;
		if (!line_number++)
		{
			char target[1024];
			char version[16];
			char *question;
			int index;

			if (sscanf(text, "%7s %1023s %15s", request->method, target, version) != 3 ||
				strncmp(version, "HTTP/1.", 7) || (version[7] != '0' && version[7] != '1') || version[8])
			{
				return 0;
			}
			request->minor_version = version[7] - '0';
			request->keep_alive = request->minor_version == 1;
			if (strcmp(request->method, "GET") && strcmp(request->method, "POST") && strcmp(request->method, "PUT") &&
				strcmp(request->method, "DELETE") && strcmp(request->method, "HEAD"))
			{
				*status = 405;
				*code = "method";
				return 0;
			}
			question = strchr(target, '?');
			if (question)
			{
				*question = 0;
				snprintf(request->query, sizeof(request->query), "%s", question + 1);
			}
			if (target[0] != '/' || strlen(target) >= sizeof(request->path) || strstr(target, "..") ||
				strstr(target, "//") || strchr(target, '\\') || strchr(target, '%'))
			{
				return 0;
			}
			for (index = 0; target[index]; index++)
			{
				if ((unsigned char)target[index] <= 0x20 || (unsigned char)target[index] >= 0x7F)
					return 0;
			}
			snprintf(request->path, sizeof(request->path), "%s", target);
			continue;
		}
		/* (the empty line ends the head: nothing may follow it) */
		if (!line_length)
		{
			if (line < end)
				return 0;
			break;
		}
		{
			char *colon = strchr(text, ':');
			char *value;
			size_t value_length;

			if (!colon || colon == text)
				return 0;
			*colon = 0;
			value = colon + 1;
			while (*value == ' ' || *value == '\t')
				value++;
			value_length = strlen(value);
			while (value_length && (value[value_length - 1] == ' ' || value[value_length - 1] == '\t'))
				value[--value_length] = 0;
#define HEADER(name, field) else if (text_equal_case(text, name)) snprintf(request->field, sizeof(request->field), "%s", value)
			if (text_equal_case(text, "Content-Length"))
			{
				char *number_end;
				long number;

				/* (digits alone, and once: two lengths, which a proxy in front may
				read otherwise, are refused: RFC 9112, 6.3) */
				if (!value[0] || !isdigit((unsigned char)value[0]) || has_content_length)
					return 0;
				has_content_length = 1;
				number = strtol(value, &number_end, 10);
				if (*number_end)
					return 0;
				if (number < 0 || value_length > 9)
				{
					*status = 413;
					*code = "too_large";
					return 0;
				}
				request->content_length = number;
			}
			else if (text_equal_case(text, "Transfer-Encoding"))
				request->has_transfer_encoding = 1;
			else if (text_equal_case(text, "Expect"))
				request->has_expect = 1;
			else if (text_equal_case(text, "Cookie"))
			{
				const char *found = strstr(value, "opence_session=");

				while (found && found != value && found[-1] != ' ' && found[-1] != ';')
					found = strstr(found + 1, "opence_session=");
				if (found)
				{
					size_t taken = strcspn(found + 15, "; ");

					if (taken < sizeof(request->session_cookie))
						snprintf(request->session_cookie, sizeof(request->session_cookie), "%.*s", (int)taken, found + 15);
				}
			}
			else if (text_equal_case(text, "Connection"))
			{
				snprintf(request->connection, sizeof(request->connection), "%s", value);
				if (strstr(value, "close") || strstr(value, "Close"))
					request->keep_alive = 0;
				else if (strstr(value, "keep-alive") || strstr(value, "Keep-Alive"))
					request->keep_alive = 1;
			}
			HEADER("Host", host);
			HEADER("Origin", origin);
			HEADER("Sec-Fetch-Site", fetch_site);
			HEADER("Authorization", authorization);
			HEADER("X-CSRF-Token", csrf);
			HEADER("Upgrade", upgrade);
			HEADER("Sec-WebSocket-Key", websocket_key);
			HEADER("Sec-WebSocket-Version", websocket_version);
			HEADER("Sec-WebSocket-Protocol", websocket_protocol);
			else if (text_equal_case(text, "X-Forwarded-For"))
			{
				/* (its end, if it is longer than the field: the trusted proxy's
				entry is the last) */
				snprintf(request->forwarded_for, sizeof(request->forwarded_for), "%s",
					value_length < sizeof(request->forwarded_for) ? value :
					value + value_length - (sizeof(request->forwarded_for) - 1));
			}
			HEADER("X-Forwarded-Proto", forwarded_proto);
			HEADER("X-Forwarded-Host", forwarded_host);
			HEADER("If-None-Match", if_none_match);
#undef HEADER
		}
	}
	if (!request->method[0])
		return 0;
	if (request->has_transfer_encoding || request->has_expect)
		return 0;
	if (request->content_length > MAXIMUM_BODY)
	{
		*status = 413;
		*code = "too_large";
		return 0;
	}
	return 1;
}

/* a query's parameter's value: the default if it is not there or no number */
static long query_number(const char *query, const char *name, long fallback)
{
	size_t length = strlen(name);
	const char *at = query;

	while (at && *at)
	{
		if (!strncmp(at, name, length) && at[length] == '=')
		{
			char *end;
			long value = strtol(at + length + 1, &end, 10);

			return end != at + length + 1 ? value : fallback;
		}
		at = strchr(at, '&');
		if (at)
			at++;
	}
	return fallback;
}

/* ---------- addresses: rate limits and lockouts */

/* the address's record, seen now; made if it has none, in an unused one,
else the least lately seen one that is not locked, else (every one locked)
the one whose lock ends soonest: no address is refused for want of room */
static struct address_record *address_record(unsigned long address)
{
	struct address_record *unused = NULL, *unlocked = NULL, *locked = NULL, *record;
	unsigned long now = now_seconds();
	int index;

	for (index = 0; index < MAXIMUM_ADDRESSES; index++)
	{
		record = &http.addresses[index];
		if (!record->last_seen)
		{
			if (!unused)
				unused = record;
		}
		else if (record->address == address)
		{
			record->last_seen = now;
			return record;
		}
		else if (record->locked_until > now)
		{
			if (!locked || record->locked_until < locked->locked_until)
				locked = record;
		}
		else if (!unlocked || record->last_seen < unlocked->last_seen)
		{
			unlocked = record;
		}
	}
	record = unused ? unused : unlocked ? unlocked : locked;
	memset(record, 0, sizeof(*record));
	record->address = address;
	record->last_seen = now;
	return record;
}

/* a request from the address: 0 if it is past its allowance a minute */
static int address_allows_request(unsigned long address, int authenticated)
{
	struct address_record *record = address_record(address);
	unsigned long now = now_seconds();

	if (now - record->window_start >= 60)
	{
		record->window_start = now;
		record->requests = 0;
	}
	record->requests++;
	return record->requests <= (authenticated ? REQUESTS_AUTHENTICATED : REQUESTS_UNAUTHENTICATED);
}

/* the seconds the address is locked out for (0: not) */
static int address_locked(unsigned long address)
{
	struct address_record *record = address_record(address);
	unsigned long now = now_seconds();

	return record->locked_until > now ? (int)(record->locked_until - now) : 0;
}

static void address_login_failed(unsigned long address, const char *text)
{
	struct address_record *record = address_record(address);
	unsigned long now = now_seconds();

	http.logins_failed++;
	if (now - record->failures_start >= LOGIN_WINDOW_SECONDS)
	{
		record->failures_start = now;
		record->failures = 0;
	}
	if (++record->failures >= LOGIN_FAILURES)
	{
		record->locked_until = now + LOGIN_LOCK_SECONDS;
		record->failures = 0;
		server_log(_server_log_warning, "server", "web: %s locked out for %d minutes after %d wrong passwords",
			config_boolean("server.log_addresses") ? text : "an address", LOGIN_LOCK_SECONDS / 60, LOGIN_FAILURES);
	}
}

/* ---------- sessions */

static void session_hash(const char *token, unsigned char *hash)
{
	p2p_hmac_sha256(http.session_key, sizeof(http.session_key), token, (int)strlen(token), hash);
}

/* the session of a token, refreshed; NULL if there is none (or it lapsed) */
static struct session *session_find(const char *token)
{
	unsigned char hash[P2P_SHA256_SIZE];
	unsigned long now = now_seconds();
	int index;

	if (strlen(token) != 2 * TOKEN_BYTES)
		return NULL;
	session_hash(token, hash);
	for (index = 0; index < MAXIMUM_SESSIONS; index++)
	{
		struct session *session = &http.sessions[index];

		if (!session->used || !p2p_equal(session->hash, hash, P2P_SHA256_SIZE))
			continue;
		if (now - session->created > SESSION_SECONDS || now - session->used_at > SESSION_IDLE_SECONDS)
		{
			session->used = 0;
			return NULL;
		}
		session->used_at = now;
		return session;
	}
	return NULL;
}

/* a new session (the oldest one gives way), its token in token */
static struct session *session_new(int role, const char *by, char *token)
{
	struct session *oldest = &http.sessions[0];
	unsigned char bytes[TOKEN_BYTES];
	int index;

	for (index = 0; index < MAXIMUM_SESSIONS; index++)
	{
		if (!http.sessions[index].used)
		{
			oldest = &http.sessions[index];
			break;
		}
		if (http.sessions[index].used_at < oldest->used_at)
			oldest = &http.sessions[index];
	}
	memset(oldest, 0, sizeof(*oldest));
	posix_random_bytes(bytes, sizeof(bytes));
	p2p_hex(bytes, TOKEN_BYTES, token);
	session_hash(token, oldest->hash);
	posix_random_bytes(bytes, CSRF_BYTES);
	p2p_hex(bytes, CSRF_BYTES, oldest->csrf);
	oldest->role = role;
	snprintf(oldest->by, sizeof(oldest->by), "%s", by);
	oldest->created = oldest->used_at = now_seconds();
	oldest->used = 1;
	return oldest;
}

static void ws_close(struct connection *connection, int code, const char *reason);

/* the web passwords, checked for changes (sv_set, the settings' page,
sv_reload): the sessions of a role whose password changed end, and its
remote consoles close */
static void passwords_check(void)
{
	static const char *const settings[_server_role_owner + 1] = {
		"server.web_viewer_password", "server.web_moderator_password", "server.web_password",
		"server.web_owner_password" };
	unsigned long changes = config_changes();
	unsigned long changed = 0;
	int role, index;

	if (http.passwords_known && changes == http.password_changes)
		return;
	http.password_changes = changes;
	for (role = 0; role <= _server_role_owner; role++)
	{
		const char *password = config_string(settings[role]);
		unsigned char hash[P2P_SHA256_SIZE];

		p2p_hmac_sha256(http.session_key, sizeof(http.session_key), password, (int)strlen(password), hash);
		if (http.passwords_known && !p2p_equal(hash, http.password_hashes[role], P2P_SHA256_SIZE))
			changed |= 1UL << role;
		memcpy(http.password_hashes[role], hash, P2P_SHA256_SIZE);
	}
	http.passwords_known = 1;
	if (!changed)
		return;
	for (index = 0; index < MAXIMUM_SESSIONS; index++)
	{
		if (http.sessions[index].used && http.sessions[index].role >= 0 &&
			(changed & (1UL << http.sessions[index].role)))
		{
			http.sessions[index].used = 0;
		}
	}
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		struct connection *connection = &http.connections[index];

		if (connection->socket >= 0 && connection->websocket && connection->role >= 0 &&
			connection->role <= _server_role_owner && (changed & (1UL << connection->role)))
		{
			connection->role = _server_role_none;
			ws_close(connection, 1008, "the password changed");
		}
	}
	for (role = 0; role <= _server_role_owner; role++)
	{
		if (changed & (1UL << role))
			server_log(_server_log_info, "server", "web: the %s password changed: its sessions are signed out",
				server_role_name(role));
	}
}

/* the role a password gives (checked against the budget and the address's
lockout; the address's text for the log): _server_role_none, and the
status and code why. The owner's password gives the owner's role to a
local connection (connection_is_local) alone: from anywhere else it is
refused "forbidden", and counted as a wrong password is (no cheaper to
guess than another) */
static int password_role(unsigned long address, const char *address_shown, const char *password, int local,
	int *status, const char **code, int *retry_after)
{
	unsigned long minute = now_seconds() / 60;
	const char *admin = config_string("server.web_password");
	const char *moderator = config_string("server.web_moderator_password");
	const char *viewer = config_string("server.web_viewer_password");
	const char *owner = config_string("server.web_owner_password");
	int locked = address_locked(address);
	int owner_elsewhere = 0;

	/* (a password changed since the thread's loop began: its sessions end
	before one is begun with the new one) */
	passwords_check();
	*retry_after = 0;
	if (locked)
	{
		*status = 429;
		*code = "locked";
		*retry_after = locked;
		return _server_role_none;
	}
	if (minute != http.budget_minute)
	{
		http.budget_minute = minute;
		http.budget_used = 0;
	}
	if (++http.budget_used > LOGIN_BUDGET)
	{
		*status = 429;
		*code = "rate_limited";
		*retry_after = 60 - (int)(now_seconds() % 60);
		return _server_role_none;
	}
	if (password && password[0] && owner[0] && constant_time_equal(password, owner))
	{
		if (local)
			return _server_role_owner;
		owner_elsewhere = 1;
	}
	if (password && password[0])
	{
		if (admin[0] && constant_time_equal(password, admin))
			return _server_role_admin;
		if (moderator[0] && constant_time_equal(password, moderator))
			return _server_role_moderator;
		if (viewer[0] && constant_time_equal(password, viewer))
			return _server_role_viewer;
	}
	address_login_failed(address, address_shown);
	if (owner_elsewhere)
	{
		server_log(_server_log_warning, "server", "web: the owner's password was given from %s: refused (it signs in "
			"from this machine alone)", config_boolean("server.log_addresses") ? address_shown : "another address");
		*status = 403;
		*code = "forbidden";
		return _server_role_none;
	}
	*status = 401;
	*code = "denied";
	return _server_role_none;
}

/* ---------- origins */

/* the host the request was made to (a trusted proxy's word for it) */
static const char *request_host(struct connection *connection)
{
	if (connection->request.forwarded_host[0] && connection->forwarded)
		return connection->request.forwarded_host;
	return connection->request.host;
}

/* whether the request's Origin is the server's own (scheme ignored: the
proxy's) */
static int origin_matches(struct connection *connection)
{
	const char *origin = connection->request.origin;
	const char *host = request_host(connection);
	const char *after_scheme = strstr(origin, "://");

	/* (an Origin of "null", a browser's for a page whose referrer policy
	hides it, taken only when the browser itself says the request is
	same-origin: Sec-Fetch-Site, which no page can set) */
	if (!strcmp(origin, "null"))
		return !strcmp(connection->request.fetch_site, "same-origin");
	if (!origin[0] || !host[0] || !after_scheme)
		return 0;
	return text_equal_case(after_scheme + 3, host);
}

/* ---------- tickets */

static struct server_ticket *ticket_new(int kind, int role, const char *by)
{
	struct server_ticket *ticket = calloc(1, sizeof(*ticket));

	if (!ticket)
		return NULL;
	ticket->kind = kind;
	ticket->role = role;
	snprintf(ticket->by, sizeof(ticket->by), "%s", by ? by : "");
	ticket->deadline = SDL_GetTicksNS() + (Uint64)TICKET_SECONDS * 1000000000ULL;
	return ticket;
}

static void ticket_free(struct server_ticket *ticket)
{
	if (!ticket)
		return;
	free(ticket->body);
	free(ticket->response);
	free(ticket);
}

static void ticket_submit(struct server_ticket *ticket)
{
	SDL_LockMutex(http.lock);
	ticket->next = http.pending;
	http.pending = ticket;
	SDL_UnlockMutex(http.lock);
}

/* ---------- WebSocket messages */

static void ws_frame(struct connection *connection, int opcode, const void *payload, int size)
{
	unsigned char head[10];
	int length = 0;

	head[length++] = (unsigned char)(0x80 | opcode);
	if (size < 126)
	{
		head[length++] = (unsigned char)size;
	}
	else if (size < 65536)
	{
		head[length++] = 126;
		head[length++] = (unsigned char)(size >> 8);
		head[length++] = (unsigned char)size;
	}
	else
	{
		int index;

		head[length++] = 127;
		for (index = 7; index >= 0; index--)
			head[length++] = index >= 4 ? 0 : (unsigned char)((unsigned long)size >> (8 * index));
	}
	out_append(connection, head, length);
	out_append(connection, payload, size);
}

static void ws_close(struct connection *connection, int code, const char *reason)
{
	unsigned char payload[128];
	int length = (int)strlen(reason);

	if (connection->closing)
		return;
	if (length > 120)
		length = 120;
	/* (the messages queued before it go before it) */
	while (connection->queue_head)
	{
		struct ws_message *message = connection->queue_head;

		connection->queue_head = message->next;
		ws_frame(connection, 0x1, message->text, message->length);
		free(message);
	}
	connection->queue_tail = NULL;
	connection->queued = 0;
	payload[0] = (unsigned char)(code >> 8);
	payload[1] = (unsigned char)code;
	memcpy(payload + 2, reason, (size_t)length);
	ws_frame(connection, 0x8, payload, length + 2);
	connection->closing = 1;
	connection->close_after_write = 1;
}

/* a message for the client, queued (sent as the socket takes it); past
WS_QUEUE, the queue goes and the client is told it overflowed */
static void ws_send(struct connection *connection, const char *text)
{
	int length = (int)strlen(text);
	struct ws_message *message;

	if (connection->closing || connection->overflowed)
		return;
	if (connection->queued >= WS_QUEUE)
	{
		struct ws_message *each = connection->queue_head;

		while (each)
		{
			struct ws_message *next = each->next;

			free(each);
			each = next;
		}
		connection->queue_head = connection->queue_tail = NULL;
		connection->queued = 0;
		connection->overflowed = 1;
		http.events_dropped++;
		ws_frame(connection, 0x1, "{\"type\":\"overflow\"}", 19);
		ws_close(connection, 1013, "overflow");
		return;
	}
	message = malloc(sizeof(*message) + (size_t)length);
	if (!message)
		return;
	message->next = NULL;
	message->length = length;
	memcpy(message->text, text, (size_t)length + 1);
	if (connection->queue_tail)
		connection->queue_tail->next = message;
	else
		connection->queue_head = message;
	connection->queue_tail = message;
	connection->queued++;
}

/* the queue onto the socket's buffer, as it drains */
static void ws_flush_queue(struct connection *connection)
{
	while (connection->queue_head && out_pending(connection) < OUTPUT_PAUSE)
	{
		struct ws_message *message = connection->queue_head;

		connection->queue_head = message->next;
		if (!connection->queue_head)
			connection->queue_tail = NULL;
		connection->queued--;
		ws_frame(connection, 0x1, message->text, message->length);
		free(message);
	}
}

static void ws_error(struct connection *connection, const char *id, const char *code, const char *message)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "type", "error");
	if (id && id[0])
		server_json_raw(&json, "id", id);
	server_json_object_begin(&json, "error");
	server_json_string(&json, "code", code);
	server_json_string(&json, "message", message);
	server_json_object_end(&json);
	server_json_object_end(&json);
	ws_send(connection, server_json_text(&json));
	server_json_free(&json);
}

static void ws_reply_refused(struct connection *connection, const char *id, const char *code, const char *message)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "type", "reply");
	server_json_raw(&json, "id", id && id[0] ? id : "null");
	server_json_boolean(&json, "ok", 0);
	server_json_array_begin(&json, "lines");
	server_json_array_end(&json);
	server_json_object_begin(&json, "error");
	server_json_string(&json, "code", code);
	server_json_string(&json, "message", message);
	server_json_object_end(&json);
	server_json_object_end(&json);
	ws_send(connection, server_json_text(&json));
	server_json_free(&json);
}

static void ws_auth_answer(struct connection *connection, int ok, const char *code, const char *message, int retry_after)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_string(&json, "type", "auth");
	server_json_boolean(&json, "ok", ok);
	if (ok)
	{
		server_json_string(&json, "role", server_role_name(connection->role));
	}
	else
	{
		server_json_object_begin(&json, "error");
		server_json_string(&json, "code", code);
		server_json_string(&json, "message", message);
		if (retry_after > 0)
			server_json_integer(&json, "retry_after", retry_after);
		server_json_object_end(&json);
	}
	server_json_object_end(&json);
	ws_send(connection, server_json_text(&json));
	server_json_free(&json);
	if (!ok)
		ws_close(connection, 1008, code);
}

static int group_index(const char *name)
{
	int index;

	for (index = 0; index < NUMBER_OF_EVENT_GROUPS; index++)
	{
		if (!strcmp(name, event_groups[index]))
			return index;
	}
	return -1;
}

/* whether the connection takes an event of the group */
static int ws_wants(struct connection *connection, const char *group)
{
	int index = group_index(group);

	if (!connection->websocket || connection->role == _server_role_none || connection->closing)
		return 0;
	if (!strcmp(group, "chat") && connection->role < _server_role_admin)
		return 0;
	return !strcmp(group, "server") || (index >= 0 && (connection->groups & (1UL << index)));
}

struct replay_context
{
	struct connection *connection;
};

/* whether the connection's events carry the players' addresses */
static int ws_addresses(struct connection *connection)
{
	return role_sees_addresses(connection->role);
}

static void ws_replay_one(void *context, unsigned long sequence, const char *group, const char *text)
{
	struct replay_context *replay = context;
	size_t length = strlen(text);
	char *marked;

	(void)sequence;
	if (!ws_wants(replay->connection, group) || length < 2)
		return;
	marked = malloc(length + 16);
	if (!marked)
		return;
	/* (,"replay":true before its closing brace) */
	memcpy(marked, text, length - 1);
	strcpy(marked + length - 1, ",\"replay\":true}");
	ws_send(replay->connection, marked);
	free(marked);
}

/* the status event, as the connection's role sees it */
static void ws_send_status(struct connection *connection)
{
	char *text;
	const char *status;
	size_t length;
	/* (before the lock: the events' lock is taken before this one, as an
	event is added) */
	unsigned long latest = server_events_latest();

	SDL_LockMutex(http.lock);
	status = connection->role >= _server_role_admin ? http.status_admin : http.status_viewer;
	if (!status)
	{
		SDL_UnlockMutex(http.lock);
		return;
	}
	length = strlen(status) + 128;
	text = malloc(length);
	if (text)
	{
		snprintf(text, length, "{\"type\":\"event\",\"seq\":%lu,\"time\":%lld,\"event\":\"status\",\"status\":%s}",
			latest, posix_server_unix_milliseconds(), status);
	}
	SDL_UnlockMutex(http.lock);
	if (text)
	{
		ws_send(connection, text);
		free(text);
	}
}

/* a WebSocket command's line from its message: "line", or "command" and
"args" */
static int ws_command_line(struct server_json_document const *document, char *line, int size)
{
	int token = server_json_member(document, 0, "line");
	int command, arguments, index, length;

	if (token >= 0)
		return server_json_get_string(document, token, line, size) && line[0];
	command = server_json_member(document, 0, "command");
	if (command < 0 || !server_json_get_string(document, command, line, size) || !line[0])
		return 0;
	arguments = server_json_member(document, 0, "args");
	length = (int)strlen(line);
	for (index = 0; server_json_element(document, arguments, index) >= 0; index++)
	{
		char word[256];
		int quote;

		if (!server_json_get_string(document, server_json_element(document, arguments, index), word, sizeof(word)))
			return 0;
		quote = strchr(word, ' ') != NULL || !word[0];
		length += snprintf(line + length, (size_t)(size - length), quote ? " \"%s\"" : " %s", word);
		if (length >= size)
			return 0;
	}
	return 1;
}

static void ws_message(struct connection *connection, const char *text, int length)
{
	struct server_json_document document;
	char type[32];
	char id[32] = "";
	int token;

	if (!server_json_parse(&document, text, length, 16) || document.tokens[0].type != _server_json_object)
	{
		if (document.tokens)
			server_json_document_free(&document);
		ws_error(connection, NULL, "bad_request", "not a JSON object");
		return;
	}
	token = server_json_member(&document, 0, "id");
	if (token >= 0 && document.tokens[token].end - document.tokens[token].start < (int)sizeof(id) - 2)
	{
		if (document.tokens[token].type == _server_json_string_token)
			snprintf(id, sizeof(id), "\"%.*s\"", document.tokens[token].end - document.tokens[token].start,
				text + document.tokens[token].start);
		else
			snprintf(id, sizeof(id), "%.*s", document.tokens[token].end - document.tokens[token].start,
				text + document.tokens[token].start);
		/* (an id holds no quote or backslash of its own: it is echoed raw) */
		if (strchr(id + 1, '\\') || (id[0] == '"' && strchr(id + 1, '"') != id + strlen(id) - 1))
			id[0] = 0;
	}
	if (!server_json_get_string(&document, server_json_member(&document, 0, "type"), type, sizeof(type)))
	{
		ws_error(connection, id, "bad_request", "a message has a type");
		server_json_document_free(&document);
		return;
	}
	if (!strcmp(type, "ping"))
	{
		ws_send(connection, "{\"type\":\"pong\"}");
	}
	else if (!strcmp(type, "auth"))
	{
		char secret[160] = "";
		char by[NAME_SIZE] = "";

		if (connection->role != _server_role_none)
		{
			ws_error(connection, id, "bad_request", "signed in already");
		}
		else if (server_json_get_string(&document, server_json_member(&document, 0, "token"), secret, sizeof(secret)))
		{
			struct session *session = session_find(secret);

			/* (an owner's session is used from this machine alone, where it
			was begun) */
			if (!session || (session->role >= _server_role_owner && !connection_is_local(connection)))
			{
				ws_auth_answer(connection, 0, "denied", "no such session", 0);
			}
			else
			{
				connection->role = session->role;
				snprintf(connection->by, sizeof(connection->by), "%s", session->by);
				ws_auth_answer(connection, 1, NULL, NULL, 0);
			}
		}
		else
		{
			int status, retry_after, role;
			const char *code;

			server_json_get_string(&document, server_json_member(&document, 0, "password"), secret, sizeof(secret));
			server_json_get_string(&document, server_json_member(&document, 0, "by"), by, sizeof(by));
			role = password_role(connection->real_address, connection->real_text, secret, connection_is_local(connection),
				&status, &code, &retry_after);
			if (role == _server_role_none)
			{
				ws_auth_answer(connection, 0, code, status == 401 ? "wrong password" : status == 403 ?
					"the owner's password signs in from the server's own machine alone" : "too many tries: wait",
					retry_after);
			}
			else
			{
				connection->role = role;
				if (!by[0])
					snprintf(by, sizeof(by), "rcon %s", server_role_name(role));
				snprintf(connection->by, sizeof(connection->by), "%s", by);
				server_log(_server_log_info, "server", "rcon: %s signed in%s%s (%s)", connection->by,
					config_boolean("server.log_addresses") ? " from " : "", config_boolean("server.log_addresses") ?
					connection->real_text : "", server_role_name(role));
				ws_auth_answer(connection, 1, NULL, NULL, 0);
			}
		}
		memset(secret, 0, sizeof(secret));
	}
	else if (!strcmp(type, "command"))
	{
		char line[SERVER_COMMAND_LINE_SIZE];
		Uint64 now = SDL_GetTicksNS();

		if (connection->role == _server_role_none)
		{
			ws_reply_refused(connection, id, "denied", "sign in first");
		}
		else
		{
			connection->bucket += (double)(now - connection->bucket_time) / 1e9 * COMMANDS_PER_SECOND;
			if (connection->bucket > COMMANDS_BURST)
				connection->bucket = COMMANDS_BURST;
			connection->bucket_time = now;
			if (connection->bucket < 1.0)
			{
				ws_reply_refused(connection, id, "rate_limited", "32 commands a second at most");
			}
			else if (!ws_command_line(&document, line, sizeof(line)))
			{
				connection->bucket -= 1.0;
				ws_reply_refused(connection, id, "bad_request", "line is a command (511 characters at most)");
			}
			else if (connection->ticket)
			{
				ws_reply_refused(connection, id, "busy", "one command at a time: wait for its reply");
			}
			else
			{
				struct server_ticket *ticket = ticket_new(_ticket_ws_command, connection->role, connection->by);

				connection->bucket -= 1.0;
				if (ticket)
				{
					ticket->body = strdup(line);
					ticket->body_length = ticket->body ? (int)strlen(line) : 0;
					snprintf(ticket->id, sizeof(ticket->id), "%s", id[0] ? id : "null");
					connection->ticket = ticket;
					ticket_submit(ticket);
				}
			}
		}
	}
	else if (connection->role == _server_role_none)
	{
		ws_error(connection, id, "denied", "sign in first");
	}
	else if (!strcmp(type, "subscribe"))
	{
		int events = server_json_member(&document, 0, "events");
		int index;
		unsigned long groups = 0;
		double after;
		int ok = events >= 0 && document.tokens[events].type == _server_json_array;

		for (index = 0; ok && server_json_element(&document, events, index) >= 0; index++)
		{
			char group[SERVER_EVENT_GROUP_SIZE];
			int found;

			if (!server_json_get_string(&document, server_json_element(&document, events, index), group, sizeof(group)) ||
				(found = group_index(group)) < 0)
			{
				ok = 0;
				break;
			}
			groups |= 1UL << found;
		}
		if (!ok)
		{
			ws_error(connection, id, "bad_request", "events is a list of event groups");
		}
		else
		{
			connection->groups = groups;
			if (server_json_get_number(&document, server_json_member(&document, 0, "after"), &after) && after >= 0)
			{
				struct replay_context replay;
				unsigned long next, missed;

				replay.connection = connection;
				server_events_read((unsigned long)after, SERVER_EVENTS_RING_SIZE, ws_addresses(connection), ws_replay_one,
					&replay, &next, &missed);
			}
			if (groups & (1UL << group_index("status")))
				ws_send_status(connection);
		}
	}
	else
	{
		char message[64];

		snprintf(message, sizeof(message), "no message type %.32s", type);
		ws_error(connection, id, "bad_request", message);
	}
	server_json_document_free(&document);
}

/* the frames read so far: each one taken; 0 if the connection is to close */
static int ws_read_frames(struct connection *connection)
{
	while (connection->in_length >= 2 && !connection->closing && out_pending(connection) < OUTPUT_PAUSE)
	{
		unsigned char *data = connection->in;
		int fin = data[0] & 0x80, opcode = data[0] & 0x0F;
		unsigned long long size = data[1] & 0x7F;
		int head = 2, index;
		unsigned char mask[4];

		if (data[0] & 0x70)
		{
			ws_close(connection, 1002, "reserved bits");
			return 1;
		}
		if (!(data[1] & 0x80))
		{
			ws_close(connection, 1002, "a client's frames are masked");
			return 1;
		}
		if (size == 126)
		{
			if (connection->in_length < 4)
				return 1;
			size = (unsigned long long)data[2] << 8 | data[3];
			head = 4;
		}
		else if (size == 127)
		{
			if (connection->in_length < 10)
				return 1;
			for (size = 0, index = 2; index < 10; index++)
				size = size << 8 | data[index];
			head = 10;
		}
		if (opcode >= 0x8 && (size > 125 || !fin))
		{
			ws_close(connection, 1002, "a bad control frame");
			return 1;
		}
		if (size > WS_FRAME)
		{
			ws_close(connection, 1009, "64 KB a frame at most");
			return 1;
		}
		if ((unsigned long long)connection->in_length < (unsigned long long)head + 4 + size)
			return 1;
		memcpy(mask, data + head, 4);
		head += 4;
		for (index = 0; index < (int)size; index++)
			data[head + index] ^= mask[index & 3];
		switch (opcode)
		{
		case 0x8:
			ws_close(connection, 1000, "goodbye");
			break;
		case 0x9:
			ws_frame(connection, 0xA, data + head, (int)size);
			break;
		case 0xA:
			break;
		case 0x1:
		case 0x0:
			if ((opcode == 0x1) == (connection->message_open != 0))
			{
				ws_close(connection, 1002, opcode == 0x1 ? "a new message inside a fragmented one" :
					"a continuation without a message");
				break;
			}
			if (connection->message_length + (int)size > WS_MESSAGE)
			{
				ws_close(connection, 1009, "256 KB a message at most");
				break;
			}
			{
				unsigned char *grown = realloc(connection->message, (size_t)connection->message_length + (size_t)size + 1);

				if (!grown)
				{
					ws_close(connection, 1011, "out of memory");
					break;
				}
				connection->message = grown;
				memcpy(connection->message + connection->message_length, data + head, (size_t)size);
				connection->message_length += (int)size;
				connection->message[connection->message_length] = 0;
			}
			connection->message_open = !fin;
			if (fin)
			{
				ws_message(connection, (const char *)connection->message, connection->message_length);
				connection->message_length = 0;
			}
			break;
		case 0x2:
			ws_close(connection, 1003, "text frames only");
			break;
		default:
			ws_close(connection, 1002, "an unknown opcode");
			break;
		}
		memmove(connection->in, connection->in + head + size, (size_t)(connection->in_length - head - (int)size));
		connection->in_length -= head + (int)size;
	}
	return 1;
}

/* ---------- the routes */

/* the session the request carries (Bearer, else the cookie): NULL if none;
by_cookie says which */
static struct session *request_session(struct connection *connection, int *by_cookie)
{
	const char *authorization = connection->request.authorization;

	*by_cookie = 0;
	if (!strncmp(authorization, "Bearer ", 7))
		return session_find(authorization + 7);
	*by_cookie = 1;
	return connection->request.session_cookie[0] ? session_find(connection->request.session_cookie) : NULL;
}

static int body_string(const char *body, int length, const char *key, char *value, int size)
{
	struct server_json_document document;
	int found;

	value[0] = 0;
	if (!server_json_parse(&document, body, length, 8))
		return 0;
	found = document.tokens[0].type == _server_json_object &&
		server_json_get_string(&document, server_json_member(&document, 0, key), value, size);
	server_json_document_free(&document);
	return found;
}

/* POST /api/rotation/play's sv_map line, from its map (a map's name, or a
rotation entry's whole text, map:gametype+rule=value, as the page sends an
entry with rules: up to a rotation entry's length) and its gametype; NULL,
else why it cannot be played */
static const char *rotation_play_line(const char *body, int length, char *line, int size)
{
	char map[SERVER_ENTRY_TEXT_SIZE], gametype[64] = "";

	if (!body_string(body, length, "map", map, sizeof(map)) || !map[0] || strchr(map, ' ') || strchr(map, '"'))
		return "map is needed: a map or a rotation entry, 255 characters at most, with no spaces";
	body_string(body, length, "gametype", gametype, sizeof(gametype));
	if (strchr(gametype, ' ') || strchr(gametype, '"'))
		return "a gametype holds no space";
	snprintf(line, (size_t)size, "sv_map %s %s", map, gametype);
	return NULL;
}

static void serve_asset(struct connection *connection)
{
	const char *path = connection->request.path;
	unsigned long index;
	char headers[128];

	if (!strcmp(path, "/"))
		path = "/index.html";
	if (!config_boolean("server.web_ui"))
	{
		respond_error(connection, 404, "not_found", "the page is off (server.web_ui)", 0);
		return;
	}
	for (index = 0; index < server_ui_asset_count; index++)
	{
		const struct server_ui_asset *asset = &server_ui_assets[index];

		if (strcmp(asset->path, path + 1))
			continue;
		snprintf(headers, sizeof(headers), "ETag: \"%s\"\r\n", asset->etag);
		if (connection->request.if_none_match[0] && strstr(connection->request.if_none_match, asset->etag))
		{
			respond(connection, 304, asset->content_type, "", 0, headers, 0);
			return;
		}
		respond(connection, 200, asset->content_type, asset->data, (int)asset->size, headers, 0);
		return;
	}
	respond_error(connection, 404, "not_found", "nothing is there", 0);
}

static void serve_healthz(struct connection *connection)
{
	char state[16];
	char body[64];
	Uint64 age;
	int hosting;

	SDL_LockMutex(http.lock);
	snprintf(state, sizeof(state), "%s", http.state[0] ? http.state : "starting");
	age = http.snapshot_time ? SDL_GetTicksNS() - http.snapshot_time : 0;
	SDL_UnlockMutex(http.lock);
	hosting = !strcmp(state, "lobby") || !strcmp(state, "loading") || !strcmp(state, "playing") ||
		!strcmp(state, "postgame");
	/* (healthy: hosting, its frames running; a map loading holds them some
	seconds, a minute without one is a game stuck) */
	if (!http.snapshot_time)
		respond_json(connection, 503, "{\"state\":\"starting\"}", NULL);
	else if (age > 60ULL * 1000000000ULL)
		respond_json(connection, 503, "{\"state\":\"stalled\"}", NULL);
	else
	{
		snprintf(body, sizeof(body), "{\"state\":\"%s\"}", state);
		respond_json(connection, hosting ? 200 : 503, body, NULL);
	}
}

static void serve_metrics(struct connection *connection)
{
	char text[4096];
	int length;

	if (!config_boolean("server.metrics"))
	{
		respond_error(connection, 404, "not_found", "the metrics are off (server.metrics)", 0);
		return;
	}
	SDL_LockMutex(http.lock);
	length = snprintf(text, sizeof(text),
		"# HELP opence_up Whether the server hosts.\n# TYPE opence_up gauge\nopence_up %d\n"
		"# TYPE opence_state gauge\nopence_state{state=\"%s\"} 1\n"
		"# HELP opence_players Players in the game.\n# TYPE opence_players gauge\nopence_players %ld\n"
		"# TYPE opence_players_max gauge\nopence_players_max %ld\n"
		"# TYPE opence_machines gauge\nopence_machines %ld\n"
		"# TYPE opence_games_total counter\nopence_games_total %ld\n"
		"# HELP opence_frame_seconds_max The longest frame of the last minute.\n"
		"# TYPE opence_frame_seconds_max gauge\nopence_frame_seconds_max %.6f\n"
		"# TYPE opence_frame_seconds_mean gauge\nopence_frame_seconds_mean %.6f\n"
		"# TYPE opence_events_dropped_total counter\nopence_events_dropped_total %lu\n"
		"# TYPE opence_http_requests_total counter\nopence_http_requests_total %lu\n"
		"# TYPE opence_logins_failed_total counter\nopence_logins_failed_total %lu\n"
		"# TYPE opence_process_resident_bytes gauge\nopence_process_resident_bytes %lu\n"
		"# TYPE opence_process_cpu_seconds_total counter\nopence_process_cpu_seconds_total %.3f\n"
		"# TYPE opence_uptime_seconds counter\nopence_uptime_seconds %.0f\n"
		"# TYPE opence_build_info gauge\nopence_build_info{build=\"%s\",network_version=\"%d\"} 1\n",
		strcmp(http.state, "lobby") && strcmp(http.state, "playing") && strcmp(http.state, "loading") &&
			strcmp(http.state, "postgame") ? 0 : 1,
		http.state[0] ? http.state : "starting", http.metrics_players, http.metrics_max_players, http.metrics_machines,
		http.metrics_games, http.metrics_frame_longest / 1000.0, http.metrics_frame_mean / 1000.0,
		http.events_dropped + http.pushes_dropped, http.requests_total, http.logins_failed,
		posix_server_resident_bytes(), posix_server_cpu_milliseconds() / 1000.0, server_mode_uptime(),
		server_mode_build(), HALO_PORT_NETWORK_VERSION);
	SDL_UnlockMutex(http.lock);
	respond(connection, 200, "text/plain; version=0.0.4; charset=utf-8", text, length, NULL, 1);
}

struct log_context
{
	struct server_json *json;
	int role;
};

static void serve_log(struct connection *connection, int role)
{
	static struct server_log_line lines[1024];
	unsigned long after = (unsigned long)(query_number(connection->request.query, "after", 0) < 0 ? 0 :
		query_number(connection->request.query, "after", 0));
	long limit = query_number(connection->request.query, "limit", 256);
	unsigned long next, missed;
	struct server_json json;
	int count, index;

	if (limit < 1)
		limit = 1;
	if (limit > 1024)
		limit = 1024;
	/* (a cursor past the latest, as after a restart: from the start) */
	if (after > server_log_latest())
		after = 0;
	count = server_log_read(after, lines, (int)limit, &next, &missed);
	if (!after)
		missed = 0;
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_json_array_begin(&json, "lines");
	for (index = 0; index < count; index++)
	{
		if (!strcmp(lines[index].category, "chat") && role < _server_role_admin)
			continue;
		server_json_object_begin(&json, NULL);
		server_json_integer(&json, "n", (long long)lines[index].number);
		server_json_integer(&json, "time", lines[index].time);
		server_json_string(&json, "level", server_log_level_name(lines[index].level));
		server_json_string(&json, "cat", lines[index].category);
		server_json_string(&json, "text", lines[index].text);
		server_json_object_end(&json);
	}
	server_json_array_end(&json);
	server_json_integer(&json, "next", (long long)next);
	server_json_integer(&json, "missed", (long long)missed);
	server_json_object_end(&json);
	respond_json(connection, 200, server_json_text(&json), NULL);
	server_json_free(&json);
}

static void events_collect(void *context, unsigned long sequence, const char *group, const char *text)
{
	struct log_context *collected = context;

	(void)sequence;
	if (!strcmp(group, "chat") && collected->role < _server_role_admin)
		return;
	server_json_raw(collected->json, NULL, text);
}

static void serve_events(struct connection *connection, int role)
{
	unsigned long after = (unsigned long)(query_number(connection->request.query, "after", 0) < 0 ? 0 :
		query_number(connection->request.query, "after", 0));
	long limit = query_number(connection->request.query, "limit", 256);
	unsigned long next, missed;
	struct server_json json;
	struct log_context collected;

	if (limit < 1)
		limit = 1;
	if (limit > SERVER_EVENTS_RING_SIZE)
		limit = SERVER_EVENTS_RING_SIZE;
	if (after > server_events_latest())
		after = 0;
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_json_array_begin(&json, "events");
	collected.json = &json;
	collected.role = role;
	server_events_read(after, (int)limit, role_sees_addresses(role), events_collect, &collected, &next, &missed);
	server_json_array_end(&json);
	server_json_integer(&json, "next", (long long)next);
	server_json_integer(&json, "missed", after ? (long long)missed : 0);
	server_json_object_end(&json);
	respond_json(connection, 200, server_json_text(&json), NULL);
	server_json_free(&json);
}

static void serve_snapshot(struct connection *connection, const char *text)
{
	if (!text)
	{
		respond_error(connection, 503, "starting", "the server is starting", 5);
		return;
	}
	respond_json(connection, 200, text, NULL);
}

static void serve_login(struct connection *connection, const char *body, int length)
{
	char password[256], by[NAME_SIZE], token[2 * TOKEN_BYTES + 1];
	char headers[256];
	int status, retry_after, role;
	const char *code;
	struct session *session;
	struct server_json json;

	if (!body_string(body, length, "password", password, sizeof(password)))
	{
		respond_error(connection, 400, "schema", "password is needed", 0);
		return;
	}
	body_string(body, length, "by", by, sizeof(by));
	role = password_role(connection->real_address, connection->real_text, password, connection_is_local(connection),
		&status, &code, &retry_after);
	memset(password, 0, sizeof(password));
	if (role == _server_role_none)
	{
		respond_error(connection, status, code, status == 401 ? "wrong password" : status == 403 ?
			"the owner's password signs in from the server's own machine alone" : "too many tries: wait", retry_after);
		return;
	}
	if (!by[0])
		snprintf(by, sizeof(by), "web %s", server_role_name(role));
	session = session_new(role, by, token);
	server_log(_server_log_info, "server", "web: %s signed in (%s)", session->by, server_role_name(role));
	snprintf(headers, sizeof(headers), "Set-Cookie: opence_session=%s; HttpOnly; SameSite=Strict; Path=/%s\r\n", token,
		text_equal_case(connection->request.forwarded_proto, "https") && connection->forwarded ?
			"; Secure" : "");
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_json_string(&json, "token", token);
	server_json_string(&json, "role", server_role_name(role));
	server_json_string(&json, "csrf", session->csrf);
	server_json_integer(&json, "expires", session->created + SESSION_SECONDS);
	server_json_object_end(&json);
	respond_json(connection, 200, server_json_text(&json), headers);
	server_json_free(&json);
	memset(token, 0, sizeof(token));
}

/* the routes that need the game's thread: a ticket of the request */
static int route_needs_game(const char *method, const char *path)
{
	return !strncmp(path, "/api/players/", 13) || !strcmp(path, "/api/say") || !strncmp(path, "/api/rotation", 13) ||
		!strncmp(path, "/api/gametypes", 14) || !strcmp(path, "/api/maps") || !strncmp(path, "/api/bans", 9) ||
		!strcmp(path, "/api/settings") || !strcmp(path, "/api/command") || !strcmp(path, "/api/stop") ||
		!strncmp(path, "/api/vpn", 8);
	(void)method;
}

/* the role a route needs */
static int route_role(const char *method, const char *path)
{
	int mutation = strcmp(method, "GET") && strcmp(method, "HEAD");

	if (!strcmp(path, "/api/stop"))
		return _server_role_owner;
	if (!strncmp(path, "/api/players/", 13) || !strcmp(path, "/api/say") || !strcmp(path, "/api/rotation/next") ||
		!strcmp(path, "/api/rotation/play") || !strcmp(path, "/api/vpn/check"))
	{
		return _server_role_moderator;
	}
	if ((!strncmp(path, "/api/rotation", 13) || !strncmp(path, "/api/gametypes", 14)) && mutation)
		return _server_role_admin;
	if (!strncmp(path, "/api/bans", 9) || !strcmp(path, "/api/settings"))
		return _server_role_admin;
	return _server_role_viewer;
}

static void handle_request(struct connection *connection, const char *body, int length)
{
	struct http_request *request = &connection->request;
	const char *path = request->path;
	int mutation = strcmp(request->method, "GET") && strcmp(request->method, "HEAD");
	int by_cookie = 0;
	struct session *session = NULL;

	http.requests_total++;
	if (request->origin[0] && !origin_matches(connection))
	{
		respond_error(connection, 403, "origin", "a request from another site", 0);
		return;
	}
	if (!strcmp(path, "/healthz"))
	{
		serve_healthz(connection);
		return;
	}
	if (!strcmp(path, "/api/login"))
	{
		if (strcmp(request->method, "POST"))
			respond_error(connection, 405, "method", "/api/login takes POST", 0);
		else if (!address_allows_request(connection->real_address, 0))
			respond_error(connection, 429, "rate_limited", "too many requests: wait a minute", 60);
		else
			serve_login(connection, body, length);
		return;
	}
	/* (every request but the health's counts against its address's
	allowance, the page's files and the metrics too; a sign in's, above, as
	one without a session) */
	session = request_session(connection, &by_cookie);
	/* (an owner's session is used from this machine alone, where it was
	begun) */
	if (session && session->role >= _server_role_owner && !connection_is_local(connection))
		session = NULL;
	if (!address_allows_request(connection->real_address, session != NULL))
	{
		respond_error(connection, 429, "rate_limited", "too many requests: wait a minute", 60);
		return;
	}
	if (!strcmp(path, "/metrics"))
	{
		serve_metrics(connection);
		return;
	}
	if (strncmp(path, "/api/", 5))
	{
		if (mutation)
			respond_error(connection, 405, "method", "the page takes GET", 0);
		else
			serve_asset(connection);
		return;
	}
	if (!strcmp(path, "/api/logout") && strcmp(request->method, "POST"))
	{
		/* (a change: a POST, with the CSRF token if by the cookie) */
		respond_error(connection, 405, "method", "/api/logout takes POST", 0);
		return;
	}
	if (!session)
	{
		respond_error(connection, 401, "denied", "sign in first", 0);
		return;
	}
	if (mutation && by_cookie)
	{
		if (!request->origin[0])
		{
			respond_error(connection, 403, "origin", "a change from the page needs its Origin", 0);
			return;
		}
		if (!constant_time_equal(request->csrf, session->csrf))
		{
			respond_error(connection, 403, "csrf", "the X-CSRF-Token header is missing or wrong", 0);
			return;
		}
	}
	if (session->role < route_role(request->method, path))
	{
		char message[64];

		snprintf(message, sizeof(message), "needs the %s role", server_role_name(route_role(request->method, path)));
		respond_error(connection, 403, "forbidden", message, 0);
		return;
	}
	if (!strcmp(path, "/api/logout"))
	{
		session->used = 0;
		respond_json(connection, 200, "{\"ok\":true}",
			"Set-Cookie: opence_session=; HttpOnly; SameSite=Strict; Path=/; Max-Age=0\r\n");
		return;
	}
	if (!strcmp(path, "/api/session"))
	{
		struct server_json json;

		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_json_string(&json, "role", server_role_name(session->role));
		server_json_string(&json, "csrf", session->csrf);
		server_json_integer(&json, "expires", session->created + SESSION_SECONDS);
		server_json_object_end(&json);
		respond_json(connection, 200, server_json_text(&json), NULL);
		server_json_free(&json);
		return;
	}
	if (!strcmp(path, "/api/status") && !mutation)
	{
		SDL_LockMutex(http.lock);
		serve_snapshot(connection, session->role >= _server_role_admin ? http.status_admin : http.status_viewer);
		SDL_UnlockMutex(http.lock);
		return;
	}
	if (!strcmp(path, "/api/players") && !mutation)
	{
		SDL_LockMutex(http.lock);
		serve_snapshot(connection, session->role >= _server_role_owner ? http.players_owner :
			session->role >= _server_role_admin ? http.players_admin : http.players_viewer);
		SDL_UnlockMutex(http.lock);
		return;
	}
	if (!strcmp(path, "/api/log") && !mutation)
	{
		serve_log(connection, session->role);
		return;
	}
	if (!strcmp(path, "/api/events") && !mutation)
	{
		serve_events(connection, session->role);
		return;
	}
	if (route_needs_game(request->method, path))
	{
		struct server_ticket *ticket = ticket_new(_ticket_api, session->role, session->by);

		if (!ticket)
		{
			respond_error(connection, 500, "internal", "out of memory", 0);
			return;
		}
		snprintf(ticket->method, sizeof(ticket->method), "%s", request->method);
		snprintf(ticket->path, sizeof(ticket->path), "%s", path);
		if (length > 0)
		{
			ticket->body = malloc((size_t)length + 1);
			if (ticket->body)
			{
				memcpy(ticket->body, body, (size_t)length);
				ticket->body[length] = 0;
				ticket->body_length = length;
			}
		}
		connection->ticket = ticket;
		ticket_submit(ticket);
		return;
	}
	respond_error(connection, 404, "not_found", "nothing is there", 0);
}

void server_http_capabilities(struct server_json *json, const char *key)
{
	static const char *const capabilities[] = {
		"events", "status", "rotation", "settings", "bans",
		/* (the fleets': the owner's password from this machine, a ban of an
		address, a drain and stop, addresses in the events, the server's
		identity in the status; the VPN policy's /api/vpn and join_refused,
		and verified Discord accounts in the players and the bans) */
		"owner_web", "ban_address", "stop", "event_addresses", "identity", "vpn", "link" };
	int index;

	server_json_array_begin(json, key);
	for (index = 0; index < (int)(sizeof(capabilities) / sizeof(capabilities[0])); index++)
		server_json_string(json, NULL, capabilities[index]);
	server_json_array_end(json);
}

static void handle_upgrade(struct connection *connection)
{
	struct http_request *request = &connection->request;
	char accept_input[128];
	unsigned char digest[20];
	char accept[32];
	char head[512];
	int length, count = 0, index, by_cookie;
	struct session *session;

	/* (counted against the address's allowance, as every request but the
	health's is) */
	http.requests_total++;
	session = request_session(connection, &by_cookie);
	if (session && session->role >= _server_role_owner && !connection_is_local(connection))
		session = NULL;
	if (!address_allows_request(connection->real_address, session != NULL))
	{
		respond_error(connection, 429, "rate_limited", "too many requests: wait a minute", 60);
		return;
	}
	if (strcmp(request->method, "GET") || !text_equal_case(request->upgrade, "websocket") || !request->websocket_key[0])
	{
		respond_error(connection, 400, "bad_request", "/ws is the RCON: a WebSocket (Upgrade: websocket)", 0);
		return;
	}
	if (strcmp(request->websocket_version, "13"))
	{
		static const char refusal[] = "{\"ok\":false,\"error\":{\"code\":\"bad_request\",\"message\":\"WebSocket version 13\"}}";

		respond(connection, 426, "application/json", refusal, (int)strlen(refusal), "Sec-WebSocket-Version: 13\r\n", 1);
		return;
	}
	if (!strstr(request->websocket_protocol, "opence-rcon-1"))
	{
		respond_error(connection, 400, "bad_request", "the subprotocol is opence-rcon-1", 0);
		return;
	}
	if (request->origin[0] && !origin_matches(connection))
	{
		respond_error(connection, 403, "origin", "a connection from another site", 0);
		return;
	}
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		count += http.connections[index].socket >= 0 && http.connections[index].websocket;
	if (count >= MAXIMUM_WEBSOCKETS)
	{
		respond_error(connection, 503, "busy", "8 remote consoles at once at most", 5);
		return;
	}
	if (strlen(request->websocket_key) > 60)
	{
		respond_error(connection, 400, "bad_request", "a bad Sec-WebSocket-Key", 0);
		return;
	}
	snprintf(accept_input, sizeof(accept_input), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", request->websocket_key);
	sha1((const unsigned char *)accept_input, (unsigned long)strlen(accept_input), digest);
	server_base64_encode(digest, 20, accept);
	length = snprintf(head, sizeof(head),
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: %s\r\n"
		"Sec-WebSocket-Protocol: opence-rcon-1\r\n\r\n", accept);
	out_append(connection, head, length);
	connection->websocket = 1;
	connection->role = _server_role_none;
	connection->bucket = COMMANDS_BURST;
	connection->bucket_time = SDL_GetTicksNS();
	connection->auth_deadline = SDL_GetTicksNS() + (Uint64)AUTH_SECONDS * 1000000000ULL;
	{
		struct server_json json;

		server_json_begin(&json);
		server_json_object_begin(&json, NULL);
		server_json_string(&json, "type", "hello");
		server_json_integer(&json, "protocol", 1);
		server_json_string(&json, "server", config_string("server.name"));
		server_json_string(&json, "build", server_mode_build());
		server_json_integer(&json, "network_version", HALO_PORT_NETWORK_VERSION);
		server_http_capabilities(&json, "capabilities");
		server_json_object_end(&json);
		ws_send(connection, server_json_text(&json));
		server_json_free(&json);
	}
	/* (signed in already: the page's cookie, from its own origin, or a
	script's Bearer token) */
	if (session && (!by_cookie || origin_matches(connection)))
	{
		connection->role = session->role;
		snprintf(connection->by, sizeof(connection->by), "%s", session->by);
		ws_auth_answer(connection, 1, NULL, NULL, 0);
	}
}

/* ---------- connections */

static void connection_free(struct connection *connection)
{
	struct ws_message *message = connection->queue_head;

	while (message)
	{
		struct ws_message *next = message->next;

		free(message);
		message = next;
	}
	if (connection->ticket)
	{
		int done;

		/* (a ticket the game's thread has answered is freed here; one it
		still holds is abandoned to it, which frees it once it is done) */
		SDL_LockMutex(http.lock);
		done = connection->ticket->done;
		if (!done)
			connection->ticket->abandoned = 1;
		SDL_UnlockMutex(http.lock);
		if (done)
			ticket_free(connection->ticket);
		connection->ticket = NULL;
	}
	if (connection->socket >= 0)
		posix_socket_close(connection->socket);
	free(connection->in);
	free(connection->out);
	free(connection->message);
	memset(connection, 0, sizeof(*connection));
	connection->socket = -1;
}

/* the bytes read so far: requests answered, frames taken; 0 to close */
static int connection_take(struct connection *connection)
{
	if (connection->websocket)
		return ws_read_frames(connection);
	/* (a request at a time; none while its answers wait past OUTPUT_PAUSE
	for a client that does not read them: taken again as they go) */
	while (!connection->ticket && !connection->close_after_write && out_pending(connection) < OUTPUT_PAUSE)
	{
		const unsigned char *end_of_head;
		int status;
		const char *code;

		if (!connection->have_head)
		{
			end_of_head = NULL;
			{
				int index;

				for (index = 3; index < connection->in_length; index++)
				{
					if (connection->in[index] == '\n' && connection->in[index - 1] == '\r' && connection->in[index - 2] == '\n' &&
						connection->in[index - 3] == '\r')
					{
						end_of_head = connection->in + index + 1;
						break;
					}
				}
			}
			if (!end_of_head)
			{
				if (connection->in_length > MAXIMUM_HEAD)
				{
					connection->request.keep_alive = 0;
					respond_error(connection, 413, "too_large", "8 KB of head at most", 0);
				}
				return 1;
			}
			connection->head_length = (int)(end_of_head - connection->in);
			if (connection->head_length > MAXIMUM_HEAD ||
				!server_http_parse_head((const char *)connection->in, connection->head_length - 2, &connection->request,
					&status, &code))
			{
				connection->request.keep_alive = 0;
				if (connection->head_length > MAXIMUM_HEAD)
					respond_error(connection, 413, "too_large", "8 KB of head at most", 0);
				else
					respond_error(connection, status, code, status == 405 ? "GET, POST, PUT or DELETE" :
						status == 413 ? "64 KB of body at most" : "a request this server does not take", 0);
				return 1;
			}
			connection->have_head = 1;
			/* (behind a trusted proxy, the client's address is its word: the
			entry it appended; and so are the host and scheme it forwards) */
			connection->real_address = connection->address;
			address_text(connection->address, connection->real_text, sizeof(connection->real_text));
			connection->forwarded = 0;
			{
				unsigned long proxy;

				if (config_string("server.web_trusted_proxy")[0] &&
					parse_address(config_string("server.web_trusted_proxy"), &proxy) && proxy == connection->address)
				{
					unsigned long forwarded;
					char text[sizeof(connection->real_text)];

					connection->forwarded = 1;
					if (connection->request.forwarded_for[0] &&
						forwarded_address(connection->request.forwarded_for, &forwarded, text, sizeof(text)))
					{
						connection->real_address = forwarded;
						snprintf(connection->real_text, sizeof(connection->real_text), "%s", text);
					}
				}
			}
		}
		if (connection->in_length < connection->head_length + connection->request.content_length)
			return 1;
		if (!strcmp(connection->request.path, "/ws"))
			handle_upgrade(connection);
		else
			handle_request(connection, (const char *)connection->in + connection->head_length,
				(int)connection->request.content_length);
		{
			int used = connection->head_length + (int)connection->request.content_length;

			memmove(connection->in, connection->in + used, (size_t)(connection->in_length - used));
			connection->in_length -= used;
		}
		connection->have_head = 0;
		connection->head_started = SDL_GetTicksNS();
		if (connection->websocket)
			return ws_read_frames(connection);
	}
	return 1;
}

/* a ticket the game's thread answered: its response written */
static void connection_ticket_done(struct connection *connection)
{
	struct server_ticket *ticket = connection->ticket;

	connection->ticket = NULL;
	if (ticket->kind == _ticket_ws_command)
	{
		ws_send(connection, ticket->response ? ticket->response : "{\"type\":\"reply\",\"ok\":false,\"lines\":[]}");
	}
	else
	{
		char headers[64] = "";

		if (ticket->retry_after > 0)
			snprintf(headers, sizeof(headers), "Retry-After: %d\r\n", ticket->retry_after);
		respond_json(connection, ticket->status ? ticket->status : 500, ticket->response ? ticket->response :
			"{\"ok\":false,\"error\":{\"code\":\"internal\",\"message\":\"no answer\"}}", headers);
	}
	ticket_free(ticket);
	if (!connection->websocket)
		connection_take(connection);
}

/* the connection to close for a new one when every slot is taken: the
oldest that waits for a request (its head or body: idle, or slow), none
of a WebSocket, a ticket or answers being sent; -1 if there is none */
static int connection_to_evict(void)
{
	int index, oldest = -1;

	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		struct connection *connection = &http.connections[index];

		if (connection->socket < 0 || connection->websocket || connection->ticket || out_pending(connection) > 0)
			continue;
		if (oldest < 0 || connection->head_started < http.connections[oldest].head_started)
			oldest = index;
	}
	return oldest;
}

/* whether a connection is to close for its time: a request (its head and
body) not in within HEAD_SECONDS of the connection's opening or its last
answer, silence past IDLE_SECONDS (a signed-in WebSocket's, unless it is
closing, is kept); times later than now (a request answered since it was
read) are not past */
static int connection_overdue(struct connection *connection, Uint64 now)
{
	if (!connection->websocket && !connection->ticket && out_pending(connection) == 0 &&
		now > connection->head_started && now - connection->head_started > (Uint64)HEAD_SECONDS * 1000000000ULL)
	{
		return 1;
	}
	return !connection->ticket && now > connection->last_activity &&
		now - connection->last_activity > (Uint64)IDLE_SECONDS * 1000000000ULL &&
		!(connection->websocket && connection->role != _server_role_none && !connection->closing);
}

static void accept_connections(int listener)
{
	for (;;)
	{
		unsigned char address[16];
		int address_length = sizeof(address);
		int socket = posix_socket_accept(listener, address, &address_length);
		int index;

		if (socket < 0)
			return;
		for (index = 0; index < MAXIMUM_CONNECTIONS && http.connections[index].socket >= 0; index++)
		{
		}
		if (index == MAXIMUM_CONNECTIONS)
		{
			/* (full: the oldest connection that waits for a request gives way,
			else the new one goes) */
			index = connection_to_evict();
			if (index < 0)
			{
				posix_socket_close(socket);
				continue;
			}
			connection_free(&http.connections[index]);
		}
		posix_socket_set_nonblocking(socket, 1);
		memset(&http.connections[index], 0, sizeof(http.connections[index]));
		http.connections[index].socket = socket;
		http.connections[index].address = (unsigned long)address[4] << 24 | (unsigned long)address[5] << 16 |
			(unsigned long)address[6] << 8 | address[7];
		http.connections[index].real_address = http.connections[index].address;
		address_text(http.connections[index].address, http.connections[index].real_text,
			sizeof(http.connections[index].real_text));
		http.connections[index].head_started = http.connections[index].last_activity = SDL_GetTicksNS();
		http.connections[index].role = _server_role_none;
	}
}

/* the pushes onto the WebSockets that take them, and the status events */
static void distribute_pushes(void)
{
	struct push *pushes;
	Uint64 now = SDL_GetTicksNS();
	int index;

	SDL_LockMutex(http.lock);
	pushes = http.pushes_head;
	http.pushes_head = http.pushes_tail = NULL;
	http.pushes = 0;
	SDL_UnlockMutex(http.lock);
	while (pushes)
	{
		struct push *next = pushes->next;

		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			if (http.connections[index].socket >= 0 && ws_wants(&http.connections[index], pushes->group))
			{
				ws_send(&http.connections[index], pushes->addressed && ws_addresses(&http.connections[index]) ?
					pushes->text + pushes->addressed : pushes->text);
			}
		}
		free(pushes);
		pushes = next;
	}
	if (now - http.last_status_event >= (Uint64)STATUS_EVENT_MILLISECONDS * 1000000ULL)
	{
		http.last_status_event = now;
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			if (http.connections[index].socket >= 0 && ws_wants(&http.connections[index], "status"))
				ws_send_status(&http.connections[index]);
		}
	}
}

static int SDLCALL http_thread_main(void *listener_pointer)
{
	int listener = (int)(intptr_t)listener_pointer;

	for (;;)
	{
		int reads[MAXIMUM_CONNECTIONS + 1], writes[MAXIMUM_CONNECTIONS], errors[1];
		int read_count = 0, write_count = 0, error_count = 0;
		Uint64 now;
		int index;

		passwords_check();
		reads[read_count++] = listener;
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			struct connection *connection = &http.connections[index];

			if (connection->socket < 0)
				continue;
			if (connection->websocket)
				ws_flush_queue(connection);
			/* (nothing more is read from a client that does not read its
			answers) */
			if ((!connection->close_after_write || connection->out_sent < connection->out_length) &&
				out_pending(connection) < OUTPUT_PAUSE)
			{
				reads[read_count++] = connection->socket;
			}
			if (connection->out_sent < connection->out_length)
				writes[write_count++] = connection->socket;
		}
		posix_socket_select(reads, &read_count, writes, &write_count, errors, &error_count, 0, 20000, 0);
		for (index = 0; index < read_count; index++)
		{
			if (reads[index] == listener)
				accept_connections(listener);
		}
		/* (after the new connections' times, which are no later than it) */
		now = SDL_GetTicksNS();
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			struct connection *connection = &http.connections[index];
			int readable = 0, writable = 0, check;

			if (connection->socket < 0)
				continue;
			for (check = 0; check < read_count; check++)
				readable |= reads[check] == connection->socket;
			for (check = 0; check < write_count; check++)
				writable |= writes[check] == connection->socket;
			if (readable)
			{
				unsigned char buffer[16384];
				int count = posix_socket_recv(connection->socket, buffer, sizeof(buffer), 0);

				if (count <= 0)
				{
					int error = count < 0 ? posix_socket_last_error() : 0;

					/* (WSAEWOULDBLOCK: nothing yet) */
					if (count == 0 || error != 10035)
					{
						connection_free(connection);
						continue;
					}
				}
				else
				{
					int limit = connection->websocket ? WS_FRAME + 16 + WS_MESSAGE : MAXIMUM_HEAD + MAXIMUM_BODY + 16;

					connection->last_activity = now;
					if (connection->in_length + count > limit)
					{
						connection_free(connection);
						continue;
					}
					{
						unsigned char *grown = realloc(connection->in, (size_t)connection->in_length + (size_t)count);

						if (!grown)
						{
							connection_free(connection);
							continue;
						}
						connection->in = grown;
					}
					memcpy(connection->in + connection->in_length, buffer, (size_t)count);
					connection->in_length += count;
					if (!connection_take(connection))
					{
						connection_free(connection);
						continue;
					}
				}
			}
			if (writable && connection->out_sent < connection->out_length)
			{
				int count = posix_socket_send(connection->socket, connection->out + connection->out_sent,
					connection->out_length - connection->out_sent, 0);

				if (count < 0 && posix_socket_last_error() != 10035)
				{
					connection_free(connection);
					continue;
				}
				if (count > 0)
				{
					connection->out_sent += count;
					connection->last_activity = now;
				}
				if (connection->out_sent == connection->out_length)
				{
					connection->out_sent = connection->out_length = 0;
					/* (the next request's time from the answer's end) */
					if (!connection->have_head)
						connection->head_started = now;
					if (connection->close_after_write && !connection->queue_head)
					{
						connection_free(connection);
						continue;
					}
				}
				/* (the requests and frames held while the answers waited) */
				if (count > 0 && connection->in_length > 0 && out_pending(connection) < OUTPUT_PAUSE &&
					!connection_take(connection))
				{
					connection_free(connection);
					continue;
				}
			}
			/* (output past OUTPUT_MOST, a connection to close with nothing
			left to send, a request too slow to come, a connection silent too
			long, a WebSocket that never signed in) */
			if (connection->out_overflowed ||
				(connection->close_after_write && !out_pending(connection) && !connection->queue_head) ||
				connection_overdue(connection, now))
			{
				connection_free(connection);
				continue;
			}
			if (connection->websocket && connection->role == _server_role_none && !connection->closing &&
				now > connection->auth_deadline)
			{
				ws_auth_answer(connection, 0, "denied", "no auth in 10 seconds", 0);
			}
		}
		/* (the tickets the game's thread answered, or that waited too long:
		one abandoned is the game's thread's from then, never read here
		again) */
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			struct connection *connection = &http.connections[index];
			char id[sizeof(connection->ticket->id)];
			int done, expired;

			if (connection->socket < 0 || !connection->ticket)
				continue;
			SDL_LockMutex(http.lock);
			done = connection->ticket->done;
			expired = !done && now > connection->ticket->deadline;
			if (expired)
			{
				memcpy(id, connection->ticket->id, sizeof(id));
				connection->ticket->abandoned = 1;
				connection->ticket = NULL;
			}
			SDL_UnlockMutex(http.lock);
			if (done)
			{
				connection_ticket_done(connection);
			}
			else if (expired)
			{
				if (connection->websocket)
				{
					ws_reply_refused(connection, id, "loading", "the game did not answer in time (a map loading?)");
				}
				else
				{
					respond_error(connection, 503, "loading", "the game did not answer in time (a map loading?)", 5);
					connection_take(connection);
				}
			}
		}
		distribute_pushes();
	}
	return 0;
}

/* ---------- the pushes from other threads */

static void push_add(const char *group, const char *text, const char *addressed_text)
{
	int length = (int)strlen(text);
	int addressed_length = addressed_text ? (int)strlen(addressed_text) + 1 : 0;
	struct push *push;

	if (!http.started)
		return;
	push = malloc(sizeof(*push) + (size_t)length + (size_t)addressed_length);
	if (!push)
		return;
	push->next = NULL;
	snprintf(push->group, sizeof(push->group), "%s", group);
	push->length = length;
	memcpy(push->text, text, (size_t)length + 1);
	/* (the addressed text after the text and its end) */
	push->addressed = addressed_text ? length + 1 : 0;
	if (addressed_text)
		memcpy(push->text + length + 1, addressed_text, (size_t)addressed_length);
	SDL_LockMutex(http.lock);
	if (http.pushes >= WS_QUEUE * 4)
	{
		http.pushes_dropped++;
		SDL_UnlockMutex(http.lock);
		free(push);
		return;
	}
	if (http.pushes_tail)
		http.pushes_tail->next = push;
	else
		http.pushes_head = push;
	http.pushes_tail = push;
	http.pushes++;
	SDL_UnlockMutex(http.lock);
}

static void event_listener(unsigned long sequence, const char *group, const char *text, const char *addressed_text)
{
	(void)sequence;
	push_add(group, text, addressed_text);
}

static void log_listener(struct server_log_line const *line)
{
	struct server_json json;
	char head[96];

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_integer(&json, "n", (long long)line->number);
	server_json_string(&json, "level", server_log_level_name(line->level));
	server_json_string(&json, "cat", line->category);
	server_json_string(&json, "text", line->text);
	server_json_object_end(&json);
	/* (no seq: a log line is no event of the ring, and its own number, n,
	is its sequence, /api/log's cursor) */
	snprintf(head, sizeof(head), "{\"type\":\"event\",\"time\":%lld,\"event\":\"log\",", line->time);
	{
		const char *object = server_json_text(&json);
		size_t length = strlen(head) + strlen(object);
		char *text = malloc(length + 1);

		if (text && object[0] == '{')
		{
			strcpy(text, head);
			strcat(text, object + 1);
			push_add(!strcmp(line->category, "chat") ? "chat" : "log", text, NULL);
		}
		free(text);
	}
	server_json_free(&json);
}

/* ---------- start */

int server_http_start(void)
{
	long port = config_integer("server.web_port");
	const char *bind_text = config_string("server.web_bind");
	unsigned long bind_address = 0;
	unsigned char address[16];
	int listener, bound;
	int reuse = 1;

	if (port <= 0)
		return 1;
	if (!strcmp(bind_text, "localhost"))
		bind_address = 0x7F000001UL;
	else if (!parse_address(bind_text, &bind_address))
	{
		server_log(_server_log_error, "server", "server.web_bind \"%s\" is no IPv4 address", bind_text);
		server_mode_exit(SERVER_EXIT_CONFIGURATION);
	}
	http.lock = SDL_CreateMutex();
	posix_random_bytes(http.session_key, sizeof(http.session_key));
	{
		int index;

		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
			http.connections[index].socket = -1;
	}
	memset(address, 0, sizeof(address));
	address[0] = 2;
	address[2] = (unsigned char)(port >> 8);
	address[3] = (unsigned char)port;
	address[4] = (unsigned char)(bind_address >> 24);
	address[5] = (unsigned char)(bind_address >> 16);
	address[6] = (unsigned char)(bind_address >> 8);
	address[7] = (unsigned char)bind_address;
	listener = posix_socket(2, 1, 6);
	if (listener < 0)
	{
		server_log(_server_log_error, "server", "the web port cannot be opened");
		return 0;
	}
	/* (SO_REUSEADDR, Winsock's level and name: a restart takes its port at
	once; not on Windows, where it lets another program take the port too) */
#ifndef _WIN32
	posix_socket_setsockopt(listener, 0xFFFF, 0x0004, &reuse, sizeof(reuse));
#else
	(void)reuse;
#endif
	bound = posix_socket_bind(listener, address, sizeof(address)) == 0;
	if (!bound || posix_socket_listen(listener, 16) != 0)
	{
		/* (Winsock's codes, as posix_socket_last_error has them: in use, an
		address this machine lacks and a port it may not take are told
		apart; a port in use exits 4, the others are the configuration's) */
		int error = posix_socket_last_error();
		char reason[64];

		posix_socket_close(listener);
		if (error == 10048)
		{
			server_log(_server_log_error, "server", "the web port %s:%ld (server.web_port) is taken by another program; "
				"exiting", bind_text, port);
			return 0;
		}
		if (error == 10013)
			snprintf(reason, sizeof(reason), "not allowed (a port below 1024 needs privileges)");
		else if (error == 10049)
			snprintf(reason, sizeof(reason), "this machine has no such address");
		else
			snprintf(reason, sizeof(reason), "socket error %d", error);
		server_log(_server_log_error, "server", "the web port %s:%ld (server.web_port, server.web_bind) cannot be %s: %s; "
			"exiting", bind_text, port, bound ? "listened on" : "bound", reason);
		if (error == 10013 || error == 10049)
			server_mode_exit(SERVER_EXIT_CONFIGURATION);
		return 0;
	}
	posix_socket_set_nonblocking(listener, 1);
	http.listening = 1;
	http.started = 1;
	server_events_set_listener(event_listener);
	server_log_set_listener(log_listener);
	http.thread = SDL_CreateThread(http_thread_main, "server http", (void *)(intptr_t)listener);
	if (!http.thread)
	{
		server_log(_server_log_error, "server", "the web port's thread cannot start");
		return 0;
	}
	server_log(_server_log_info, "server", "web: http://%s:%ld/ (the admin page%s, the API, the RCON)", bind_text, port,
		config_boolean("server.web_ui") ? "" : " off");
	return 1;
}

/* ---------- the game's thread */

/* the answer of a ticket: its status and JSON */
static void ticket_answer(struct server_ticket *ticket, int status, struct server_json *json)
{
	ticket->status = status;
	ticket->response = strdup(server_json_text(json));
}

static void ticket_error(struct server_ticket *ticket, int status, const char *code, const char *message)
{
	struct server_json json;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 0);
	server_json_object_begin(&json, "error");
	server_json_string(&json, "code", code);
	server_json_string(&json, "message", message);
	server_json_object_end(&json);
	server_json_object_end(&json);
	ticket_answer(ticket, status, &json);
	server_json_free(&json);
}

/* a command line run as the ticket's role: its lines into json's "lines";
1 if it succeeded */
static int ticket_run_line(struct server_ticket *ticket, const char *line, struct server_json *json)
{
	unsigned char succeeded = 0;
	char *output;

	server_log(_server_log_info, "console", "%s: %s", ticket->by, server_console_line_is_secret(line) ?
		"(a password's command)" : line);
	output = server_commands_run(line, ticket->role, _server_origin_web, &succeeded);
	server_json_array_begin(json, "lines");
	if (output)
	{
		const char *cursor = output;

		while (*cursor)
		{
			const char *end = strchr(cursor, '\n');
			int length = end ? (int)(end - cursor) : (int)strlen(cursor);

			if (length > 0)
				server_json_string_length(json, NULL, cursor, length);
			cursor += length + (end != NULL);
		}
		free(output);
	}
	server_json_array_end(json);
	return succeeded;
}

/* a command's answer: ok and its lines, or refused (its first line said
why: a role's, an unknown command's, a usage's) */
static void ticket_command(struct server_ticket *ticket, const char *line)
{
	struct server_json json;
	int succeeded;

	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	succeeded = ticket_run_line(ticket, line, &json);
	server_json_object_end(&json);
	if (succeeded)
	{
		ticket_answer(ticket, 200, &json);
	}
	else
	{
		/* {"ok":false,"error":{...},"lines":[...]} from the same lines */
		struct server_json_document document;
		struct server_json refused;
		char first[512] = "the command failed";
		const char *code = "failed";
		int status = 400;
		const char *text = server_json_text(&json);

		if (server_json_parse(&document, text, (int)strlen(text), 8))
		{
			server_json_get_string(&document, server_json_element(&document, server_json_member(&document, 0, "lines"), 0),
				first, sizeof(first));
			server_json_document_free(&document);
		}
		if (strstr(first, "needs the ") && strstr(first, " role"))
		{
			code = "forbidden";
			status = 403;
		}
		else if (strstr(first, "not a command of the server's") || strstr(first, "no player") ||
			strstr(first, ": no ban "))
		{
			code = "not_found";
			status = 404;
		}
		else if (strstr(first, "set by HALO_") || strstr(first, "is set by "))
		{
			code = "pinned";
			status = 409;
		}
		server_json_begin(&refused);
		server_json_object_begin(&refused, NULL);
		server_json_boolean(&refused, "ok", 0);
		server_json_object_begin(&refused, "error");
		server_json_string(&refused, "code", code);
		server_json_string(&refused, "message", first);
		server_json_object_end(&refused);
		{
			const char *lines = strstr(text, "\"lines\":");

			if (lines)
			{
				char *copy = strdup(lines + 8);

				if (copy && copy[0])
				{
					/* (the object's closing brace off) */
					copy[strlen(copy) - 1] = 0;
					server_json_raw(&refused, "lines", copy);
				}
				free(copy);
			}
		}
		server_json_object_end(&refused);
		ticket_answer(ticket, status, &refused);
		server_json_free(&refused);
	}
	server_json_free(&json);
}

/* a player's slot, checked to be the named player (the page's view may be
old): 0 and answered if not */
static int ticket_player(struct server_ticket *ticket, long slot, char *name, int size)
{
	char wanted[64];
	char current[32];

	if (!body_string(ticket->body ? ticket->body : "", ticket->body_length, "name", wanted, sizeof(wanted)) || !wanted[0])
	{
		ticket_error(ticket, 400, "schema", "name is needed (the player's, checked)");
		return 0;
	}
	if (!server_status_player_matches(slot, wanted, current, sizeof(current)))
	{
		char message[128];

		if (!current[0])
		{
			snprintf(message, sizeof(message), "no player in slot %ld", slot);
			ticket_error(ticket, 404, "not_found", message);
		}
		else
		{
			struct server_json json;

			snprintf(message, sizeof(message), "slot %ld is %s now, not %s: nothing was done", slot, current, wanted);
			server_json_begin(&json);
			server_json_object_begin(&json, NULL);
			server_json_boolean(&json, "ok", 0);
			server_json_object_begin(&json, "error");
			server_json_string(&json, "code", "changed");
			server_json_string(&json, "message", message);
			server_json_string(&json, "name", current);
			server_json_object_end(&json);
			server_json_object_end(&json);
			ticket_answer(ticket, 409, &json);
			server_json_free(&json);
		}
		return 0;
	}
	snprintf(name, (size_t)size, "%s", current);
	return 1;
}

static int body_number(const char *body, int length, const char *key, double *value)
{
	struct server_json_document document;
	int found;

	if (!server_json_parse(&document, body, length, 8))
		return 0;
	found = server_json_get_number(&document, server_json_member(&document, 0, key), value);
	server_json_document_free(&document);
	return found;
}

/* a text a line takes: no quotes, no control characters, 127 at most */
static void line_text(const char *source, char *destination, int size)
{
	int length = 0;

	for (; *source && length < size - 1 && length < 127; source++)
	{
		if ((unsigned char)*source >= 0x20 && (unsigned char)*source < 0x7F && *source != '"')
			destination[length++] = *source;
	}
	destination[length] = 0;
}

static void ticket_api(struct server_ticket *ticket)
{
	const char *path = ticket->path;
	const char *method = ticket->method;
	const char *body = ticket->body ? ticket->body : "";
	int length = ticket->body_length;
	struct server_json json;

	server_json_begin(&json);
	if (!strncmp(path, "/api/players/", 13) && !strcmp(method, "POST"))
	{
		char action[16];
		char name[32];
		char line[SERVER_COMMAND_LINE_SIZE];
		long slot;
		char *after;

		slot = strtol(path + 13, &after, 10);
		if (after == path + 13 || *after != '/' || slot < 0 || slot > 127 || strlen(after + 1) >= sizeof(action))
		{
			ticket_error(ticket, 404, "not_found", "nothing is there");
		}
		else if (snprintf(action, sizeof(action), "%s", after + 1), strcmp(action, "kick") && strcmp(action, "ban") &&
			strcmp(action, "mute") && strcmp(action, "unmute") && strcmp(action, "tell"))
		{
			ticket_error(ticket, 404, "not_found", "nothing is there");
		}
		else if (ticket_player(ticket, slot, name, sizeof(name)))
		{
			if (!strcmp(action, "ban"))
			{
				char duration[32] = "forever", reason[160] = "", kept[160];

				body_string(body, length, "duration", duration, sizeof(duration));
				body_string(body, length, "reason", reason, sizeof(reason));
				line_text(reason, kept, sizeof(kept));
				snprintf(line, sizeof(line), "ban %ld %s %s", slot, duration[0] ? duration : "forever", kept);
			}
			else if (!strcmp(action, "mute"))
			{
				double minutes = 0;

				if (body_number(body, length, "minutes", &minutes) && (minutes < 0 || minutes > 1440))
				{
					ticket_error(ticket, 400, "schema", "minutes is 1 to 1440");
					server_json_free(&json);
					return;
				}
				if (minutes >= 1)
					snprintf(line, sizeof(line), "mute %ld %ld", slot, (long)minutes);
				else
					snprintf(line, sizeof(line), "mute %ld", slot);
			}
			else if (!strcmp(action, "tell"))
			{
				char text[256], kept[160];

				if (!body_string(body, length, "text", text, sizeof(text)) || !text[0])
				{
					ticket_error(ticket, 400, "schema", "text is needed");
					server_json_free(&json);
					return;
				}
				line_text(text, kept, sizeof(kept));
				snprintf(line, sizeof(line), "tell %ld %s", slot, kept);
			}
			else
			{
				snprintf(line, sizeof(line), "%s %ld", action, slot);
			}
			ticket_command(ticket, line);
		}
	}
	else if (!strcmp(path, "/api/say") && !strcmp(method, "POST"))
	{
		char text[256], kept[160], line[SERVER_COMMAND_LINE_SIZE];

		if (!body_string(body, length, "text", text, sizeof(text)) || !text[0])
		{
			ticket_error(ticket, 400, "schema", "text is needed");
		}
		else
		{
			line_text(text, kept, sizeof(kept));
			snprintf(line, sizeof(line), "say %s", kept);
			ticket_command(ticket, line);
		}
	}
	else if (!strcmp(path, "/api/command") && !strcmp(method, "POST"))
	{
		char line[SERVER_COMMAND_LINE_SIZE];

		if (!body_string(body, length, "line", line, sizeof(line)) || !line[0])
			ticket_error(ticket, 400, "schema", "line is a command (511 characters at most)");
		else
			ticket_command(ticket, line);
	}
	else if (!strcmp(path, "/api/rotation") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_status_rotation(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strcmp(path, "/api/rotation") && !strcmp(method, "PUT"))
	{
		struct server_json_document document;
		char *text = malloc(4200);
		int entries, index, ok = 1, text_length = 0;

		if (!text || !server_json_parse(&document, body, length, 8))
		{
			ticket_error(ticket, 400, "bad_request", "the body is not JSON");
			free(text);
			server_json_free(&json);
			return;
		}
		entries = server_json_member(&document, 0, "entries");
		text[0] = 0;
		if (entries < 0 || document.tokens[entries].type != _server_json_array || !document.tokens[entries].size ||
			document.tokens[entries].size > 64)
		{
			ok = 0;
		}
		for (index = 0; ok && server_json_element(&document, entries, index) >= 0; index++)
		{
			char entry[256];

			if (!server_json_get_string(&document, server_json_element(&document, entries, index), entry, sizeof(entry)) ||
				strchr(entry, ',') || strchr(entry, '\n') || !entry[0])
			{
				ok = 0;
				break;
			}
			text_length += snprintf(text + text_length, (size_t)(4200 - text_length), "%s%s",
				index ? ", " : "", entry);
			if (text_length >= 4200 - 1)
				ok = 0;
		}
		server_json_document_free(&document);
		if (!ok)
		{
			ticket_error(ticket, 400, "schema", "entries is a list of 1 to 64 entries (map:gametype), with no commas");
		}
		else
		{
			struct server_json rejected;

			server_json_begin(&rejected);
			server_json_object_begin(&rejected, NULL);
			if (server_director_set_rotation_text(text, &rejected))
			{
				server_log(_server_log_info, "server", "rotation set by %s: %s", ticket->by, text);
				server_json_object_begin(&json, NULL);
				server_json_boolean(&json, "ok", 1);
				server_status_rotation(&json);
				server_json_object_end(&json);
				ticket_answer(ticket, 200, &json);
			}
			else
			{
				const char *rejected_text;

				server_json_object_end(&rejected);
				rejected_text = server_json_text(&rejected);
				server_json_object_begin(&json, NULL);
				server_json_boolean(&json, "ok", 0);
				server_json_object_begin(&json, "error");
				server_json_string(&json, "code", "rejected");
				server_json_string(&json, "message", "some entries cannot be played: nothing changed");
				/* ({"rejected":[...]}'s array) */
				if (strlen(rejected_text) > 13)
				{
					char *array = strdup(rejected_text + 12);

					if (array)
					{
						array[strlen(array) - 1] = 0;
						server_json_raw(&json, "rejected", array);
					}
					free(array);
				}
				server_json_object_end(&json);
				server_json_object_end(&json);
				ticket_answer(ticket, 400, &json);
			}
			server_json_free(&rejected);
		}
		free(text);
	}
	else if (!strcmp(path, "/api/rotation/next") && !strcmp(method, "POST"))
	{
		ticket_command(ticket, "sv_map_next");
	}
	else if (!strcmp(path, "/api/rotation/play") && !strcmp(method, "POST"))
	{
		char line[SERVER_COMMAND_LINE_SIZE];
		const char *refused = rotation_play_line(body, length, line, sizeof(line));

		if (refused)
			ticket_error(ticket, 400, "schema", refused);
		else
			ticket_command(ticket, line);
	}
	else if (!strcmp(path, "/api/maps") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_status_maps(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strcmp(path, "/api/gametypes") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_status_gametypes(&json);
		server_gametype_files_json(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strncmp(path, "/api/gametypes/", 15))
	{
		const char *name = path + 15;
		char folder[1024], file[1200], temporary[1210];
		int index, valid = name[0] != 0 && strlen(name) <= 32;

		for (index = 0; valid && name[index]; index++)
		{
			valid = (name[index] >= 'a' && name[index] <= 'z') || (name[index] >= '0' && name[index] <= '9') ||
				name[index] == '_' || name[index] == '-';
		}
		if (!valid)
		{
			ticket_error(ticket, 404, "not_found", "a gametype file's name is a-z, 0-9, _ and -, 32 at most");
			server_json_free(&json);
			return;
		}
		server_gametypes_folder(folder, sizeof(folder));
		snprintf(file, sizeof(file), "%s/%s.toml", folder, name);
		if (!strcmp(method, "GET"))
		{
			size_t size = 0;
			char *text = config_file_read(file, &size);

			if (!text)
			{
				ticket_error(ticket, 404, "not_found", "no such gametype file");
			}
			else
			{
				server_json_object_begin(&json, NULL);
				server_json_boolean(&json, "ok", 1);
				server_json_string(&json, "name", name);
				server_json_string_length(&json, "text", text, (int)size);
				server_json_object_end(&json);
				ticket_answer(ticket, 200, &json);
				free(text);
			}
		}
		else if (!strcmp(method, "PUT"))
		{
			char *text = malloc((size_t)length + 1);
			char reason[256];
			int line = 0;

			if (!text || !body_string(body, length, "text", text, length + 1))
			{
				ticket_error(ticket, 400, "schema", "text is needed (the file's TOML)");
			}
			else if (!server_gametype_check_text(text, (int)strlen(text), reason, sizeof(reason), &line))
			{
				server_json_object_begin(&json, NULL);
				server_json_boolean(&json, "ok", 0);
				server_json_object_begin(&json, "error");
				server_json_string(&json, "code", "invalid");
				server_json_string(&json, "message", reason);
				server_json_integer(&json, "line", line);
				server_json_object_end(&json);
				server_json_object_end(&json);
				ticket_answer(ticket, 400, &json);
			}
			else
			{
				FILE *handle;
				int written = 0;

				posix_make_directory(folder);
				snprintf(temporary, sizeof(temporary), "%s.tmp", file);
				handle = fopen(temporary, "wb");
				if (handle)
				{
					written = fwrite(text, 1, strlen(text), handle) == strlen(text);
					written = fclose(handle) == 0 && written && posix_server_rename(temporary, file);
				}
				if (!written)
				{
					ticket_error(ticket, 500, "internal", "the gametypes folder cannot be written");
				}
				else
				{
					server_log(_server_log_info, "server", "gametypes/%s.toml written by %s", name, ticket->by);
					server_json_object_begin(&json, NULL);
					server_json_boolean(&json, "ok", 1);
					server_json_object_end(&json);
					ticket_answer(ticket, 200, &json);
				}
			}
			free(text);
		}
		else if (!strcmp(method, "DELETE"))
		{
			if (!posix_server_remove(file))
			{
				ticket_error(ticket, 404, "not_found", "no such gametype file");
			}
			else
			{
				server_log(_server_log_info, "server", "gametypes/%s.toml deleted by %s", name, ticket->by);
				server_json_object_begin(&json, NULL);
				server_json_boolean(&json, "ok", 1);
				server_json_object_end(&json);
				ticket_answer(ticket, 200, &json);
			}
		}
		else
		{
			ticket_error(ticket, 405, "method", "GET, PUT or DELETE");
		}
	}
	else if (!strcmp(path, "/api/vpn") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_netlist_status(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strcmp(path, "/api/vpn/check") && !strcmp(method, "POST"))
	{
		char ip[64], by[32];
		unsigned long address = 0;
		int part, listed;
		const char *cursor = ip;

		ip[0] = 0;
		body_string(body, length, "ip", ip, sizeof(ip));
		/* (a dotted IPv4 address, each part 0 to 255) */
		for (part = 0; part < 4; part++)
		{
			char *end;
			unsigned long number = strtoul(cursor, &end, 10);

			if (end == cursor || number > 255 || (part < 3 && *end != '.') || (part == 3 && *end))
				break;
			address = address << 8 | number;
			cursor = end + 1;
		}
		if (part < 4 || !isdigit((unsigned char)ip[0]))
		{
			ticket_error(ticket, 400, "schema", "ip is an IPv4 address (a.b.c.d)");
		}
		else
		{
			listed = server_netlist_listed(address, by, sizeof(by));
			server_json_object_begin(&json, NULL);
			server_json_boolean(&json, "ok", 1);
			server_json_boolean(&json, "listed", listed);
			server_json_string(&json, "by", by);
			server_json_object_end(&json);
			ticket_answer(ticket, 200, &json);
		}
	}
	else if (!strcmp(path, "/api/bans") && !strcmp(method, "POST"))
	{
		char ip[64], hwid[64], discord_id[64], names[96], duration[32], reason[160], message[160];
		long number = 0;
		int answer;

		body_string(body, length, "ip", ip, sizeof(ip));
		body_string(body, length, "hwid", hwid, sizeof(hwid));
		discord_id[0] = 0;
		body_string(body, length, "discord_id", discord_id, sizeof(discord_id));
		body_string(body, length, "names", names, sizeof(names));
		body_string(body, length, "duration", duration, sizeof(duration));
		body_string(body, length, "reason", reason, sizeof(reason));
		/* (as the command, with the session's role: checked and logged as
		sv_ban_address is) */
		answer = server_commands_ban_address_text(ip, hwid, discord_id, names, duration, reason, ticket->role, message,
			sizeof(message), &number);
		if (answer != 200)
		{
			ticket_error(ticket, answer, answer == 500 ? "internal" : "schema", message);
		}
		else
		{
			server_json_object_begin(&json, NULL);
			server_json_boolean(&json, "ok", 1);
			server_json_integer(&json, "n", number);
			server_json_object_end(&json);
			ticket_answer(ticket, 200, &json);
		}
	}
	else if (!strcmp(path, "/api/stop") && !strcmp(method, "POST"))
	{
		struct server_json_document document;
		char after[16] = "", notice[256] = "", kept[160], line[SERVER_COMMAND_LINE_SIZE];
		int cancel = 0, restart = 0, token, ok;

		if (!server_json_parse(&document, body, length, 8) || document.tokens[0].type != _server_json_object)
		{
			ticket_error(ticket, 400, "bad_request", "the body is a JSON object: after, notice, restart (or cancel)");
			server_json_free(&json);
			return;
		}
		token = server_json_member(&document, 0, "cancel");
		ok = token < 0 || server_json_get_boolean(&document, token, &cancel);
		token = server_json_member(&document, 0, "restart");
		ok = ok && (token < 0 || server_json_get_boolean(&document, token, &restart));
		token = server_json_member(&document, 0, "notice");
		ok = ok && (token < 0 || server_json_get_string(&document, token, notice, sizeof(notice)));
		token = server_json_member(&document, 0, "after");
		if (token >= 0 && !server_json_get_string(&document, token, after, sizeof(after)))
			after[0] = '?';
		server_json_document_free(&document);
		if (!ok)
		{
			ticket_error(ticket, 400, "schema", "cancel and restart are true or false, notice a text");
		}
		else if (cancel)
		{
			ticket_command(ticket, "sv_stop cancel");
		}
		else if (strcmp(after, "now") && strcmp(after, "game") && strcmp(after, "empty"))
		{
			ticket_error(ticket, 400, "schema", "after is now, game or empty");
		}
		else
		{
			line_text(notice, kept, sizeof(kept));
			snprintf(line, sizeof(line), "%s %s %s", restart ? "sv_restart" : "sv_stop", after, kept);
			ticket_command(ticket, line);
		}
	}
	else if (!strcmp(path, "/api/bans") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_commands_bans_json(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strncmp(path, "/api/bans/", 10) && !strcmp(method, "DELETE"))
	{
		char *after;
		long number = strtol(path + 10, &after, 10);

		if (after == path + 10 || *after || number < 0)
			ticket_error(ticket, 404, "not_found", "nothing is there");
		else
		{
			/* (as the command, with the session's role: logged and checked as
			one typed) */
			char line[32];

			snprintf(line, sizeof(line), "unban %ld", number);
			ticket_command(ticket, line);
		}
	}
	else if (!strcmp(path, "/api/settings") && !strcmp(method, "GET"))
	{
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_commands_settings_json(&json);
		server_json_object_end(&json);
		ticket_answer(ticket, 200, &json);
	}
	else if (!strcmp(path, "/api/settings") && !strcmp(method, "PUT"))
	{
		struct server_json_document document;
		int member, index;

		if (!server_json_parse(&document, body, length, 8) || document.tokens[0].type != _server_json_object)
		{
			ticket_error(ticket, 400, "bad_request", "the body is a JSON object of settings");
			server_json_free(&json);
			return;
		}
		server_json_object_begin(&json, NULL);
		server_json_boolean(&json, "ok", 1);
		server_json_array_begin(&json, "applied");
		member = 1;
		{
			struct server_json errors;

			server_json_begin(&errors);
			server_json_array_begin(&errors, NULL);
			for (index = 0; index < document.tokens[0].size; index++)
			{
				char name[64], value[300], reason[160] = "";
				int value_token = member + 1, code = 0, value_read = 1, applied = 0;
				struct server_json_token const *token = &document.tokens[value_token];

				if (!server_json_get_string(&document, member, name, sizeof(name)))
					name[0] = 0;
				/* (a string it cannot read whole, too long for value or with a
				\u0000, is refused: read, it would be the empty string, which
				the setting would take) */
				if (token->type == _server_json_string_token)
					value_read = server_json_get_string(&document, value_token, value, sizeof(value));
				else
					snprintf(value, sizeof(value), "%.*s", token->end - token->start, document.text + token->start);
				if (!name[0])
				{
					code = 404;
					snprintf(reason, sizeof(reason), "a setting's name is a string of at most 63 characters");
				}
				else if (!value_read)
				{
					snprintf(reason, sizeof(reason), "%s's value is longer than 256 characters, or has a control character",
						name);
				}
				else if (server_commands_set_setting_text(name, value, ticket->role, reason, sizeof(reason), &code))
				{
					applied = 1;
					server_json_string(&json, NULL, name);
					server_log(_server_log_info, "server", "%s changed by %s", name, ticket->by);
				}
				if (!applied)
				{
					server_json_object_begin(&errors, NULL);
					server_json_string(&errors, "name", name);
					server_json_string(&errors, "code", code == 409 ? "pinned" : code == 404 ? "not_found" :
						code == 403 ? "forbidden" : "schema");
					server_json_string(&errors, "message", reason);
					server_json_object_end(&errors);
				}
				member = server_json_skip(&document, member);
			}
			server_json_array_end(&errors);
			server_json_array_end(&json);
			server_json_raw(&json, "errors", server_json_text(&errors));
			server_json_free(&errors);
		}
		server_json_object_end(&json);
		server_json_document_free(&document);
		ticket_answer(ticket, 200, &json);
	}
	else
	{
		ticket_error(ticket, 405, "method", "that method is not taken there");
	}
	server_json_free(&json);
}

static void refresh_snapshots(void)
{
	struct server_json json;
	char *status_viewer, *status_admin, *players_viewer, *players_admin, *players_owner;
	const char *text;
	struct server_json_document document;
	short state = server_director_state();

	server_json_begin(&json);
	server_status_write(&json, _server_role_viewer);
	status_viewer = strdup(server_json_text(&json));
	server_json_free(&json);
	server_json_begin(&json);
	server_status_write(&json, _server_role_admin);
	status_admin = strdup(server_json_text(&json));
	server_json_free(&json);
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_status_players(&json, _server_role_viewer);
	server_json_object_end(&json);
	players_viewer = strdup(server_json_text(&json));
	server_json_free(&json);
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_status_players(&json, _server_role_admin);
	server_json_object_end(&json);
	players_admin = strdup(server_json_text(&json));
	server_json_free(&json);
	server_json_begin(&json);
	server_json_object_begin(&json, NULL);
	server_json_boolean(&json, "ok", 1);
	server_status_players(&json, _server_role_owner);
	server_json_object_end(&json);
	players_owner = strdup(server_json_text(&json));
	server_json_free(&json);
	SDL_LockMutex(http.lock);
	free(http.status_viewer);
	free(http.status_admin);
	free(http.players_viewer);
	free(http.players_admin);
	free(http.players_owner);
	http.status_viewer = status_viewer;
	http.status_admin = status_admin;
	http.players_viewer = players_viewer;
	http.players_admin = players_admin;
	http.players_owner = players_owner;
	snprintf(http.state, sizeof(http.state), "%s", server_status_state_name(state));
	http.snapshot_time = SDL_GetTicksNS();
	/* (the metrics' numbers, from the status) */
	text = status_viewer ? status_viewer : "";
	if (server_json_parse(&document, text, (int)strlen(text), 16))
	{
		double value;

		if (server_json_get_number(&document, server_json_member(&document, 0, "players"), &value))
			http.metrics_players = (long)value;
		if (server_json_get_number(&document, server_json_member(&document, 0, "max_players"), &value))
			http.metrics_max_players = (long)value;
		if (server_json_get_number(&document, server_json_member(&document, 0, "machines"), &value))
			http.metrics_machines = (long)value;
		if (server_json_get_number(&document, server_json_member(&document, 0, "games_hosted"), &value))
			http.metrics_games = (long)value;
		server_json_document_free(&document);
	}
	server_mode_frame_times(&http.metrics_frame_longest, &http.metrics_frame_mean);
	SDL_UnlockMutex(http.lock);
}

void server_http_main_thread(void)
{
	struct server_ticket *tickets, *ticket;
	Uint64 now = SDL_GetTicksNS();
	int count = 0;

	if (!http.started)
		return;
	SDL_LockMutex(http.lock);
	tickets = http.pending;
	http.pending = NULL;
	SDL_UnlockMutex(http.lock);
	/* (in the order they came) */
	{
		struct server_ticket *reversed = NULL;

		while (tickets)
		{
			struct server_ticket *next = tickets->next;

			tickets->next = reversed;
			reversed = tickets;
			tickets = next;
		}
		tickets = reversed;
	}
	while (tickets)
	{
		ticket = tickets;
		tickets = ticket->next;
		ticket->next = NULL;
		if (count++ >= TICKETS_PER_FRAME)
		{
			/* (the rest next frame) */
			SDL_LockMutex(http.lock);
			ticket->next = http.pending;
			http.pending = ticket;
			SDL_UnlockMutex(http.lock);
			continue;
		}
		if (ticket->kind == _ticket_ws_command)
		{
			struct server_json json;
			int succeeded;

			server_json_begin(&json);
			server_json_object_begin(&json, NULL);
			server_json_string(&json, "type", "reply");
			server_json_raw(&json, "id", ticket->id);
			succeeded = ticket_run_line(ticket, ticket->body ? ticket->body : "", &json);
			server_json_boolean(&json, "ok", succeeded);
			server_json_object_end(&json);
			ticket->response = strdup(server_json_text(&json));
			server_json_free(&json);
		}
		else
		{
			ticket_api(ticket);
		}
		SDL_LockMutex(http.lock);
		ticket->done = 1;
		if (ticket->abandoned)
		{
			SDL_UnlockMutex(http.lock);
			ticket_free(ticket);
			continue;
		}
		SDL_UnlockMutex(http.lock);
		/* (a change shows in the status at once) */
		http.last_snapshot = 0;
	}
	if (now - http.last_snapshot >= (Uint64)SNAPSHOT_MILLISECONDS * 1000000ULL)
	{
		http.last_snapshot = now;
		refresh_snapshots();
	}
}

#else

int server_http_start(void)
{
	return 1;
}

void server_http_main_thread(void)
{
}

void server_http_capabilities(struct server_json *json, const char *key)
{
	(void)json;
	(void)key;
}

#endif

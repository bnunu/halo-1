/*
SERVER_HTTP.C (test)

The real parsing of a dedicated server's web port
(port/linux/src/server_http.c): a request's head (its line, headers, body's
length, cookie), a connection's bytes taken a request at a time, the
WebSocket handshake (its SHA-1 and accept key, its refusals), a client's
frames, and the small checks (Origin, addresses, query numbers, passwords
compared). test_server_http.py takes the limits, roles and structures
(config.inc), the JSON writer (server_json.inc) and the functions
(under_test.inc); what they call besides is the test's. Also a
connection's output (its compaction, its pause and its most), its
requests held while its answers wait, the addresses' table (its
allowance, the record that makes room), the connection a new one evicts,
and a connection's deadlines; and the sign-in's decisions (handle_request,
serve_login, the sessions, the passwords' checks, their budget and
lockouts), on stand-ins for the HMAC, the random bytes and the settings.
*/

#include "harness.h"

#include <ctype.h>
#include <stdint.h>

typedef uint64_t Uint64;
typedef struct SDL_Mutex SDL_Mutex;
typedef struct SDL_Thread SDL_Thread;

#define HALO_PORT_NETWORK_VERSION 1
#define SDL_LockMutex(lock) ((void)0)
#define SDL_UnlockMutex(lock) ((void)0)
#define server_log(...) ((void)0)

#include "config.inc"
#include "server_json.inc"

static Uint64 SDL_GetTicksNS(void) { return 1000; }

static unsigned long test_now = 1000;

static unsigned long now_seconds(void) { return test_now; }

/* the settings the code reads, and their changes (config_changes) */
static const char *trusted_proxy = "";
static const char *setting_admin = "";
static const char *setting_moderator = "";
static const char *setting_viewer = "";
static const char *setting_owner = "";
static unsigned long setting_changes;

static const char *config_string(const char *name)
{
	if (!strcmp(name, "server.web_trusted_proxy"))
		return trusted_proxy;
	if (!strcmp(name, "server.web_password"))
		return setting_admin;
	if (!strcmp(name, "server.web_moderator_password"))
		return setting_moderator;
	if (!strcmp(name, "server.web_viewer_password"))
		return setting_viewer;
	if (!strcmp(name, "server.web_owner_password"))
		return setting_owner;
	return "a server";
}

static int config_boolean(const char *name)
{
	(void)name;
	return 0;
}

static unsigned long config_changes(void) { return setting_changes; }

static const char *server_mode_build(void) { return "build-0"; }

static const char *server_role_name(int role)
{
	static const char *const names[] = { "viewer", "moderator", "admin", "owner" };

	return role >= _server_role_viewer && role <= _server_role_owner ? names[role] : "none";
}

/* a stand-in for HMAC-SHA-256: a digest that differs for each key and
text, which is all the sessions and passwords need of it */
static void p2p_hmac_sha256(const unsigned char *key, int key_size, const void *data, int size, unsigned char *digest)
{
	unsigned long long state = 1469598103934665603ULL;
	int index;

	for (index = 0; index < key_size; index++)
		state = (state ^ key[index]) * 1099511628211ULL;
	state = (state ^ 0xFF) * 1099511628211ULL;
	for (index = 0; index < size; index++)
		state = (state ^ ((const unsigned char *)data)[index]) * 1099511628211ULL;
	for (index = 0; index < P2P_SHA256_SIZE; index++)
	{
		state = (state ^ (unsigned long long)index) * 1099511628211ULL;
		digest[index] = (unsigned char)(state >> 40);
	}
}

static int p2p_equal(const void *first, const void *second, int size)
{
	return !memcmp(first, second, (size_t)size);
}

/* random bytes: a generator's, the same each run */
static unsigned long long random_state = 12345;

static void posix_random_bytes(void *buffer, unsigned long size)
{
	unsigned char *bytes = buffer;

	while (size--)
	{
		random_state = random_state * 6364136223846793005ULL + 1442695040888963407ULL;
		*bytes++ = (unsigned char)(random_state >> 56);
	}
}

static void out_append(struct connection *connection, const void *data, int size);
static void ws_close(struct connection *connection, int code, const char *reason);

/* what was answered (respond and respond_error close a connection that is
not kept alive, as they do) */
static struct
{
	int count;
	int status;
	const char *code;
	int retry_after;
	char extra_headers[256];
	char body[512];
} answered;

static void respond(struct connection *connection, int status, const char *content_type, const void *body, int size,
	const char *extra_headers, int no_store)
{
	answered.count++;
	answered.status = status;
	answered.code = NULL;
	answered.retry_after = 0;
	snprintf(answered.extra_headers, sizeof(answered.extra_headers), "%s", extra_headers ? extra_headers : "");
	snprintf(answered.body, sizeof(answered.body), "%.*s", size, (const char *)body);
	if (!connection->request.keep_alive)
		connection->close_after_write = 1;
}

static void respond_error(struct connection *connection, int status, const char *code, const char *message,
	int retry_after)
{
	respond(connection, status, "application/json", "", 0, NULL, 1);
	answered.code = code;
	answered.retry_after = retry_after;
}

static void respond_json(struct connection *connection, int status, const char *json, const char *extra_headers)
{
	respond(connection, status, "application/json", json, (int)strlen(json), extra_headers, 1);
}

/* what handle_request handed on: the snapshot or page served, the ticket
for the game's thread */
static const char *served;

static void serve_healthz(struct connection *connection) { served = "healthz"; respond(connection, 200, "", "", 0, NULL, 1); }
static void serve_metrics(struct connection *connection) { served = "metrics"; respond(connection, 200, "", "", 0, NULL, 1); }
static void serve_asset(struct connection *connection) { served = "asset"; respond(connection, 200, "", "", 0, NULL, 1); }
static void serve_snapshot(struct connection *connection, const char *text)
{
	served = "snapshot";
	respond(connection, 200, "", "", 0, NULL, 1);
}
static void serve_log(struct connection *connection, int role) { served = "log"; respond(connection, 200, "", "", 0, NULL, 1); }
static void serve_events(struct connection *connection, int role)
{
	served = "events";
	respond(connection, 200, "", "", 0, NULL, 1);
}

static struct server_ticket ticket_made;
static int tickets;

static struct server_ticket *ticket_new(int kind, int role, const char *by)
{
	memset(&ticket_made, 0, sizeof(ticket_made));
	ticket_made.kind = kind;
	ticket_made.role = role;
	snprintf(ticket_made.by, sizeof(ticket_made.by), "%s", by ? by : "");
	return &ticket_made;
}

static void ticket_submit(struct server_ticket *ticket)
{
	tickets++;
	served = "ticket";
}

/* the requests handed on, the WebSocket messages taken and sent */
static struct
{
	int count;
	char path[256];
	char body[256];
	int length;
} handled;

/* the bytes each request handed on answers, if any */
static int answer_size;
static unsigned char answer[256 * 1024];

/* (the test's, which connection_take calls; the real one, which the
sign-in's cases call, is real_handle_request) */
static void handle_request(struct connection *connection, const char *body, int length)
{
	if (answer_size > 0)
		out_append(connection, answer, answer_size);
	handled.count++;
	snprintf(handled.path, sizeof(handled.path), "%s", connection->request.path);
	snprintf(handled.body, sizeof(handled.body), "%.*s", length, body);
	handled.length = length;
}

static struct
{
	int count;
	char text[256];
	int length;
} message_taken;

static void ws_message(struct connection *connection, const char *text, int length)
{
	message_taken.count++;
	snprintf(message_taken.text, sizeof(message_taken.text), "%.*s", length, text);
	message_taken.length = length;
}

static int sent_count;
static char sent[512];

static void ws_send(struct connection *connection, const char *text)
{
	sent_count++;
	snprintf(sent, sizeof(sent), "%s", text);
}

static int auth_answered;
static char auth_role[16];

static void ws_auth_answer(struct connection *connection, int ok, const char *code, const char *message, int retry_after)
{
	auth_answered = ok ? 1 : -1;
	snprintf(auth_role, sizeof(auth_role), "%s", ok ? server_role_name(connection->role) : "");
}

#include "under_test.inc"

/* ---------- the test's */

static unsigned char input[400 * 1024];
static struct connection connection;

static void connection_reset(int websocket)
{
	free(connection.out);
	free(connection.message);
	memset(&connection, 0, sizeof(connection));
	connection.socket = 5;
	connection.in = input;
	connection.websocket = websocket;
	connection.address = 0x0A000002UL;
	connection.real_address = connection.address;
	connection.role = _server_role_none;
	memset(&answered, 0, sizeof(answered));
	memset(&handled, 0, sizeof(handled));
	memset(&message_taken, 0, sizeof(message_taken));
	sent_count = 0;
	auth_answered = 0;
	auth_role[0] = 0;
	answer_size = 0;
	served = NULL;
	tickets = 0;
}

/* the sign-in's state as the server starts: no sessions, no addresses
seen, the passwords as set */
static void sign_in_reset(const char *admin, const char *moderator, const char *viewer)
{
	int index;

	memset(http.sessions, 0, sizeof(http.sessions));
	memset(http.addresses, 0, sizeof(http.addresses));
	memset(http.session_key, 7, sizeof(http.session_key));
	http.budget_minute = 0;
	http.budget_used = 0;
	http.passwords_known = 0;
	setting_admin = admin;
	setting_moderator = moderator;
	setting_viewer = viewer;
	setting_owner = "";
	setting_changes++;
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		http.connections[index].socket = -1;
	passwords_check();
}

/* request_from's requests come through the trusted proxy (connection_take
marks them so) */
static int request_forwarded;

/* a request handed to handle_request as connection_take would: its head
read from text (CRLF lines), from the address */
static void request_from(unsigned long address, const char *head, const char *body)
{
	static char text[4096];
	int status;
	const char *code;

	connection_reset(0);
	connection.address = connection.real_address = address;
	connection.forwarded = request_forwarded;
	snprintf(text, sizeof(text), "%s", head);
	CHECK(server_http_parse_head(text, (int)strlen(text), &connection.request, &status, &code),
		"the test's request was refused (%d %s): %s", status, code, head);
	real_handle_request(&connection, body ? body : "", body ? (int)strlen(body) : 0);
}

/* a sign in: POST /api/login with the password, from the address; the
token made, if one was */
static int sign_in(unsigned long address, const char *password, char *token)
{
	char body[256];
	const char *found;

	snprintf(body, sizeof(body), "{\"password\":\"%s\"}", password);
	request_from(address, "POST /api/login HTTP/1.1\r\nHost: server.example\r\n", body);
	token[0] = 0;
	if (answered.status != 200)
		return 0;
	found = strstr(answered.body, "\"token\":\"");
	CHECK(found && sscanf(found + 9, "%64[0-9a-f]", token) == 1, "a sign in answered no token: %s", answered.body);
	return 1;
}

static void feed(const void *bytes, int size)
{
	memcpy(connection.in + connection.in_length, bytes, (size_t)size);
	connection.in_length += size;
}

static void feed_text(const char *text)
{
	feed(text, (int)strlen(text));
}

/* a client's frame: its length in the shortest form, or in form (16 or 64
bits) if asked; masked with the RFC 6455's example key unless not */
static int client_frame(unsigned char *frame, int first, int opcode, const void *payload, unsigned long long size,
	int form, int masked)
{
	static const unsigned char mask[4] = { 0x37, 0xFA, 0x21, 0x3D };
	int length = 0, index;
	unsigned long long byte;

	frame[length++] = (unsigned char)opcode | (first ? 0x80 : 0);
	if (size < 126 && !form)
	{
		frame[length++] = (unsigned char)size;
	}
	else if (size < 65536 && form != 64)
	{
		frame[length++] = 126;
		frame[length++] = (unsigned char)(size >> 8);
		frame[length++] = (unsigned char)size;
	}
	else
	{
		frame[length++] = 127;
		for (index = 7; index >= 0; index--)
			frame[length++] = (unsigned char)(size >> (8 * index));
	}
	if (masked)
	{
		frame[1] |= 0x80;
		memcpy(frame + length, mask, 4);
		length += 4;
	}
	for (byte = 0; payload && byte < size; byte++)
		frame[length++] = ((const unsigned char *)payload)[byte] ^ (masked ? mask[byte & 3] : 0);
	return length;
}

static unsigned char frame[400 * 1024];

static void feed_frame(int fin, int opcode, const void *payload, unsigned long long size)
{
	feed(frame, client_frame(frame, fin, opcode, payload, size, 0, 1));
}

/* the server's frames written: the opcode and payload of the which'th, 0
if there are not that many */
static int server_frame(int which, int *opcode, const unsigned char **payload, int *size)
{
	int at = 0;

	while (at + 2 <= connection.out_length)
	{
		const unsigned char *data = connection.out + at;
		int length = data[1] & 0x7F, head = 2;

		if (length == 126)
		{
			length = data[2] << 8 | data[3];
			head = 4;
		}
		else if (length == 127)
		{
			length = data[6] << 24 | data[7] << 16 | data[8] << 8 | data[9];
			head = 10;
		}
		if (!which--)
		{
			*opcode = data[0] & 0x0F;
			*payload = data + head;
			*size = length;
			return 1;
		}
		at += head + length;
	}
	return 0;
}

/* the close frame's code the server wrote, 0 for none */
static int close_code(void)
{
	const unsigned char *payload;
	int opcode, size, which;

	for (which = 0; server_frame(which, &opcode, &payload, &size); which++)
	{
		if (opcode == 0x8 && size >= 2)
			return payload[0] << 8 | payload[1];
	}
	return 0;
}

static int parse(const char *head, struct http_request *request, int *status, const char **code)
{
	return server_http_parse_head(head, (int)strlen(head), request, status, code);
}

/* a request's head taken, or refused with the status and code */
static int head_refused(const char *head, int expected_status, const char *expected_code)
{
	static struct http_request request;
	int status;
	const char *code;

	return !parse(head, &request, &status, &code) && status == expected_status && !strcmp(code, expected_code);
}

static long content_length(const char *value, int *status)
{
	static struct http_request request;
	char head[256];
	const char *code;

	snprintf(head, sizeof(head), "POST /api/x HTTP/1.1\r\nContent-Length: %s\r\n", value);
	*status = 0;
	return parse(head, &request, status, &code) ? request.content_length : -1;
}

static const char *cookie(const char *value)
{
	static struct http_request request;
	char head[512];
	int status;
	const char *code;

	snprintf(head, sizeof(head), "GET / HTTP/1.1\r\nCookie: %s\r\n", value);
	if (!parse(head, &request, &status, &code))
		return "(refused)";
	return request.session_cookie;
}

static void hex(unsigned char const *digest, char *text)
{
	int index;

	for (index = 0; index < 20; index++)
		snprintf(text + 2 * index, 3, "%02x", digest[index]);
}

static void upgrade_request(const char *extra_headers)
{
	connection_reset(0);
	connection.request.keep_alive = 1;
	feed_text("GET /ws HTTP/1.1\r\nHost: server.example:8080\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n");
	feed_text(extra_headers);
	feed_text("\r\n");
}

static const char *accept_header(void)
{
	static char text[64];
	const char *at;

	text[0] = 0;
	if (connection.out && (at = strstr((const char *)connection.out, "Sec-WebSocket-Accept: ")) != NULL)
		sscanf(at + 22, "%63[^\r]", text);
	return text;
}

int main(int argc, char **argv)
{
	char const *case_name = argc > 1 ? argv[1] : "";
	int slot;

	/* (the connections' slots free, as server_http_start leaves them) */
	for (slot = 0; slot < MAXIMUM_CONNECTIONS; slot++)
		http.connections[slot].socket = -1;

	/* FIPS 180's examples, and every length across a block's padding (the
	digests are Python's) */
	CASE("sha1")
	{
		static const char *const examples[][2] = {
			{ "abc", "a9993e364706816aba3e25717850c26c9cd0d89d" },
			{ "", "da39a3ee5e6b4b0d3255bfef95601890afd80709" },
			{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1" },
		};
		unsigned char message[SHA1_LENGTHS], digest[20];
		char text[41];
		int index;

		for (index = 0; index < (int)(sizeof(examples) / sizeof(examples[0])); index++)
		{
			sha1((const unsigned char *)examples[index][0], (unsigned long)strlen(examples[index][0]), digest);
			hex(digest, text);
			CHECK(!strcmp(text, examples[index][1]), "\"%s\" hashed to %s", examples[index][0], text);
		}
		for (index = 0; index < SHA1_LENGTHS; index++)
			message[index] = (unsigned char)(index * 7 + 3);
		for (index = 0; index < SHA1_LENGTHS; index++)
		{
			sha1(message, (unsigned long)index, digest);
			hex(digest, text);
			CHECK(!strcmp(text, sha1_digests[index]), "%d bytes hashed to %s, not %s", index, text, sha1_digests[index]);
		}
		return 0;
	}
	/* RFC 6455's handshake (1.3's example key and accept): the subprotocol, the hello */
	CASE("accept-key")
	{
		upgrade_request("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: chat, opence-rcon-1\r\nOrigin: https://Server.Example:8080\r\n");
		CHECK(connection_take(&connection), "the connection was closed");
		CHECK(!answered.count && connection.websocket, "the upgrade was refused (%d %s)", answered.status,
			answered.code ? answered.code : "");
		CHECK(!strncmp((const char *)connection.out, "HTTP/1.1 101 Switching Protocols\r\n", 34), "no 101 was written");
		CHECK(!strcmp(accept_header(), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="), "the accept key was %s", accept_header());
		CHECK(strstr((const char *)connection.out, "\r\nSec-WebSocket-Protocol: opence-rcon-1\r\n\r\n") != NULL,
			"the subprotocol was not answered");
		CHECK(sent_count == 1 && strstr(sent, "\"type\":\"hello\"") && strstr(sent, "\"protocol\":1"),
			"the hello was %s", sent);
		CHECK(connection.role == _server_role_none && !auth_answered, "a connection signed in by nothing");
		CHECK(connection.in_length == 0, "the request's bytes were left");
		/* (signed in already: a Bearer token anywhere, a cookie from the
		page's own origin) */
		{
			char token[2 * TOKEN_BYTES + 1], headers[512];
			struct session *session;

			sign_in_reset("", "", "");
			session = session_new(_server_role_admin, "admin", token);
			CHECK(session && strlen(token) == 2 * TOKEN_BYTES, "no session was made");
			snprintf(headers, sizeof(headers), "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nOrigin: http://server.example:8080\r\n"
				"Cookie: opence_session=%s\r\n", token);
			upgrade_request(headers);
			connection_take(&connection);
			CHECK(auth_answered == 1 && connection.role == _server_role_admin && !strcmp(connection.by, "admin"),
				"a cookie from the page did not sign in");
			snprintf(headers, sizeof(headers), "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nCookie: opence_session=%s\r\n", token);
			upgrade_request(headers);
			connection_take(&connection);
			CHECK(auth_answered == 0 && connection.role == _server_role_none, "a cookie without an Origin signed in");
			snprintf(headers, sizeof(headers), "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nAuthorization: Bearer %s\r\n", token);
			upgrade_request(headers);
			connection_take(&connection);
			CHECK(auth_answered == 1 && connection.role == _server_role_admin, "a Bearer token did not sign in");
			snprintf(headers, sizeof(headers), "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nAuthorization: Bearer %.63s0\r\n", token);
			upgrade_request(headers);
			connection_take(&connection);
			CHECK(auth_answered == 0 && connection.role == _server_role_none, "a wrong Bearer token signed in");
		}
		return 0;
	}
	/* a handshake that is not one this server takes */
	CASE("upgrade-refusals")
	{
		static const char good[] = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: opence-rcon-1\r\n";
		static const struct
		{
			const char *headers;
			int status;
			const char *code;
		} refusals[] = {
			{ "Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: opence-rcon-1\r\n", 400, "bad_request" },
			{ "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 8\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\n", 426, NULL },
			{ "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n", 400, "bad_request" },
			{ "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nOrigin: https://elsewhere.example\r\n", 403, "origin" },
			{ "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nOrigin: https://server.example:8081\r\n", 403, "origin" },
			{ "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
				"Sec-WebSocket-Protocol: opence-rcon-1\r\nOrigin: null\r\n", 403, "origin" },
			{ "Sec-WebSocket-Key: 0123456789012345678901234567890123456789012345678901234567890\r\n"
				"Sec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: opence-rcon-1\r\n", 400, "bad_request" },
		};
		int index;

		for (index = 0; index < (int)(sizeof(refusals) / sizeof(refusals[0])); index++)
		{
			upgrade_request(refusals[index].headers);
			connection_take(&connection);
			CHECK(answered.count == 1 && answered.status == refusals[index].status && !connection.websocket &&
				(!refusals[index].code || !strcmp(answered.code, refusals[index].code)),
				"refusal %d was answered %d %s", index, answered.status, answered.code ? answered.code : "");
		}
		upgrade_request("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 8\r\n"
			"Sec-WebSocket-Protocol: opence-rcon-1\r\n");
		connection_take(&connection);
		CHECK(!strcmp(answered.extra_headers, "Sec-WebSocket-Version: 13\r\n"), "426 did not name version 13");
		/* (a POST, and a GET with no Upgrade) */
		connection_reset(0);
		feed_text("POST /ws HTTP/1.1\r\nUpgrade: websocket\r\n");
		feed_text(good);
		feed_text("\r\n");
		connection_take(&connection);
		CHECK(answered.status == 400 && !connection.websocket, "a POST was upgraded");
		connection_reset(0);
		feed_text("GET /ws HTTP/1.1\r\n");
		feed_text(good);
		feed_text("\r\n");
		connection_take(&connection);
		CHECK(answered.status == 400 && !connection.websocket, "a GET without Upgrade was upgraded");
		/* (no more than MAXIMUM_WEBSOCKETS at once) */
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
			http.connections[index].socket = -1;
		for (index = 0; index < MAXIMUM_WEBSOCKETS; index++)
		{
			http.connections[index].socket = 10 + index;
			http.connections[index].websocket = 1;
		}
		upgrade_request(good);
		connection_take(&connection);
		CHECK(answered.status == 503 && !strcmp(answered.code, "busy") && !connection.websocket,
			"a remote console past the most was upgraded");
		http.connections[0].websocket = 0;
		upgrade_request(good);
		connection_take(&connection);
		CHECK(!answered.count && connection.websocket, "a remote console below the most was refused");
		return 0;
	}
	/* a request's line and headers read (names in any case, values trimmed) */
	CASE("head")
	{
		struct http_request request;
		int status;
		const char *code;

		CHECK(parse("GET /api/log?after=5&limit=10 HTTP/1.1\r\nhOST: Server:8080 \r\nx-csrf-token:\tabc\t\r\n"
			"ORIGIN: http://server:8080\r\nsec-fetch-site: same-origin\r\nUnknown: kept out\r\n\r\n", &request, &status,
			&code), "a request was refused");
		CHECK(!strcmp(request.fetch_site, "same-origin"), "Sec-Fetch-Site was read as \"%s\"", request.fetch_site);
		CHECK(!strcmp(request.method, "GET") && !strcmp(request.path, "/api/log") &&
			!strcmp(request.query, "after=5&limit=10") && request.minor_version == 1 && request.keep_alive,
			"read %s %s ? %s (1.%d, %d)", request.method, request.path, request.query, request.minor_version,
			request.keep_alive);
		CHECK(!strcmp(request.host, "Server:8080") && !strcmp(request.csrf, "abc") &&
			!strcmp(request.origin, "http://server:8080") && request.content_length == 0,
			"read host %s, csrf %s, origin %s", request.host, request.csrf, request.origin);
		CHECK(parse("POST /api/login HTTP/1.0\r\nContent-Length: 12\r\nAuthorization: Bearer x\r\n", &request, &status,
			&code) && request.minor_version == 0 && !request.keep_alive && request.content_length == 12 &&
			!strcmp(request.authorization, "Bearer x"), "an HTTP/1.0 request was not read");
		CHECK(parse("GET / HTTP/1.0\r\nConnection: Keep-Alive\r\n", &request, &status, &code) && request.keep_alive,
			"HTTP/1.0 with Keep-Alive was not kept alive");
		CHECK(parse("GET / HTTP/1.1\r\nConnection: close\r\n", &request, &status, &code) && !request.keep_alive,
			"Connection: close was kept alive");
		CHECK(parse("HEAD /index.html HTTP/1.1\r\n", &request, &status, &code) && !strcmp(request.method, "HEAD"),
			"HEAD was refused");
		CHECK(parse("DELETE /api/bans/3 HTTP/1.1\r\n", &request, &status, &code) && parse("PUT /a?b HTTP/1.1", &request,
			&status, &code) && !strcmp(request.query, "b"), "DELETE or PUT was refused");
		CHECK(parse("GET /ws HTTP/1.1\r\nUpgrade: websocket\r\nSec-WebSocket-Key: k==\r\nSec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: opence-rcon-1\r\nX-Forwarded-For: 1.2.3.4\r\nX-Forwarded-Host: h\r\n"
			"X-Forwarded-Proto: https\r\nIf-None-Match: \"e\"\r\n", &request, &status, &code) &&
			!strcmp(request.upgrade, "websocket") && !strcmp(request.websocket_key, "k==") &&
			!strcmp(request.websocket_version, "13") && !strcmp(request.websocket_protocol, "opence-rcon-1") &&
			!strcmp(request.forwarded_for, "1.2.3.4") && !strcmp(request.forwarded_host, "h") &&
			!strcmp(request.forwarded_proto, "https") && !strcmp(request.if_none_match, "\"e\""),
			"a header was not read");
		/* (a value longer than its field is cut to it) */
		{
			char head[700];

			snprintf(head, sizeof(head), "GET / HTTP/1.1\r\nHost: %0400d\r\n", 0);
			CHECK(parse(head, &request, &status, &code) && strlen(request.host) == sizeof(request.host) - 1,
				"a long Host was kept as %d", (int)strlen(request.host));
		}
		return 0;
	}
	/* a request line, path or header this server does not take */
	CASE("head-refusals")
	{
		static const char *const bad[] = {
			"", "\r\n", "GET\r\n", "GET /\r\n", "GET / HTTP/2.0\r\n", "GET / HTTP/1.10\r\n", "GET / HTTP/1.2\r\n",
			"GET / http/1.1\r\n", "GET api HTTP/1.1\r\n", "GET /a/../b HTTP/1.1\r\n", "GET /a//b HTTP/1.1\r\n",
			"GET /a\\b HTTP/1.1\r\n", "GET /%2e%2e/ HTTP/1.1\r\n", "GET /caf\xc3\xa9 HTTP/1.1\r\n",
			"GET /a\x01 HTTP/1.1\r\n", "GET /a\x7f HTTP/1.1\r\n",
			"GET / HTTP/1.1\r\nNo colon\r\n", "GET / HTTP/1.1\r\n: no name\r\n",
			"POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n", "POST / HTTP/1.1\r\nexpect: 100-continue\r\n", NULL,
		};
		char head[1200];
		int index;

		/* (shown to the first byte that is not plain) */
		for (index = 0; bad[index]; index++)
		{
			CHECK(head_refused(bad[index], 400, "bad_request"), "\"%.*s\" was taken",
				(int)strcspn(bad[index], "\r\x7f\x01\xc3"), bad[index]);
		}
		CHECK(!head_refused("GET /?x=..%2F HTTP/1.1\r\n", 400, "bad_request"), "dots in a query were refused");
		CHECK(head_refused("PATCH / HTTP/1.1\r\n", 405, "method") && head_refused("OPTIONS * HTTP/1.1\r\n", 405, "method")
			&& head_refused("get / HTTP/1.1\r\n", 405, "method"), "a method this server does not take was taken");
		/* (a path of 255 characters, and 256) */
		head[0] = 0;
		strcat(head, "GET /");
		memset(head + 5, 'p', 254);
		strcpy(head + 5 + 254, " HTTP/1.1\r\n");
		CHECK(!head_refused(head, 400, "bad_request"), "a path of 255 characters was refused");
		strcpy(head + 5 + 254, "p HTTP/1.1\r\n");
		CHECK(head_refused(head, 400, "bad_request"), "a path of 256 characters was taken");
		return 0;
	}
	/* Content-Length: digits alone, nine at most, MAXIMUM_BODY at most */
	CASE("content-length")
	{
		static const struct
		{
			const char *value;
			long length;
			int status;
		} lengths[] = {
			{ "0", 0, 0 }, { "17", 17, 0 }, { " 42\t", 42, 0 }, { "65536", MAXIMUM_BODY, 0 }, { "000000005", 5, 0 },
			{ "65537", -1, 413 }, { "999999999", -1, 413 }, { "1234567890", -1, 413 }, { "0000000005", -1, 413 },
			{ "-1", -1, 400 }, { "+5", -1, 400 }, { "", -1, 400 }, { "0x10", -1, 400 }, { "5 5", -1, 400 },
			{ "5,5", -1, 400 }, { "99999999999999999999", -1, 413 },
		};
		int index, status;

		for (index = 0; index < (int)(sizeof(lengths) / sizeof(lengths[0])); index++)
		{
			long length = content_length(lengths[index].value, &status);

			CHECK(length == lengths[index].length && (length >= 0 || status == lengths[index].status),
				"Content-Length \"%s\" was read as %ld (%d)", lengths[index].value, length, status);
		}
		return 0;
	}
	/* two lengths that differ are refused (RFC 9112 6.3: a request
	smuggled past a proxy that reads the other) */
	CASE("content-length-duplicate")
	{
		CHECK(head_refused("POST /api/x HTTP/1.1\r\nContent-Length: 5\r\nContent-Length: 0\r\n", 400, "bad_request"),
			"two Content-Lengths were taken");
		return 0;
	}
	/* the session cookie among others, whole names only */
	CASE("cookie")
	{
		static const char token[] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
		char value[256];

		CHECK(!strcmp(cookie("opence_session=abc"), "abc"), "read %s", cookie("opence_session=abc"));
		CHECK(!strcmp(cookie("theme=dark; opence_session=abc; other=1"), "abc"), "read %s",
			cookie("theme=dark; opence_session=abc; other=1"));
		CHECK(!strcmp(cookie("theme=dark;opence_session=abc"), "abc"), "read %s", cookie("theme=dark;opence_session=abc"));
		CHECK(!strcmp(cookie("xopence_session=evil"), ""), "read %s", cookie("xopence_session=evil"));
		CHECK(!strcmp(cookie("a=opence_session=evil; opence_session=good"), "good"), "read %s",
			cookie("a=opence_session=evil; opence_session=good"));
		CHECK(!strcmp(cookie("theme=dark"), ""), "read %s", cookie("theme=dark"));
		snprintf(value, sizeof(value), "opence_session=%s", token);
		CHECK(!strcmp(cookie(value), token), "a whole token was not read");
		snprintf(value, sizeof(value), "opence_session=%s0", token);
		CHECK(!strcmp(cookie(value), ""), "a token too long was read as %s", cookie(value));
		return 0;
	}
	/* a connection's bytes: a head waited for, its body waited for, each
	request handed on, heads too long answered 413 */
	CASE("connection")
	{
		static char large[MAXIMUM_HEAD + 64];
		int index;

		connection_reset(0);
		feed_text("POST /api/x HTTP/1.1\r\nContent-Length: 5\r\n\r");
		CHECK(connection_take(&connection) && !handled.count && !answered.count, "part of a head was taken");
		feed_text("\nab");
		CHECK(connection_take(&connection) && !handled.count && connection.have_head,
			"part of a body was taken (%d)", handled.count);
		feed_text("cde");
		feed_text("GET /next HTTP/1.1\r\n\r\nGET /third");
		CHECK(connection_take(&connection) && handled.count == 2 && !strcmp(handled.path, "/next"),
			"the requests were handed on %d times, the last %s", handled.count, handled.path);
		CHECK(connection.in_length == 10 && !memcmp(connection.in, "GET /third", 10) && !connection.have_head,
			"the bytes after the requests were not kept");
		/* (each byte at a time) */
		connection_reset(0);
		{
			static const char request[] = "PUT /api/y HTTP/1.1\r\ncontent-length: 2\r\n\r\nhi";

			for (index = 0; request[index]; index++)
			{
				feed(request + index, 1);
				connection_take(&connection);
				CHECK(handled.count == !request[index + 1], "the request was handed on after %d bytes", index + 1);
			}
			CHECK(!strcmp(handled.body, "hi") && handled.length == 2, "the body was %s", handled.body);
		}
		/* (a head that does not end within MAXIMUM_HEAD) */
		connection_reset(0);
		memset(large, 'a', sizeof(large));
		memcpy(large, "GET / HTTP/1.1\r\nX: ", 19);
		feed(large, MAXIMUM_HEAD);
		connection_take(&connection);
		CHECK(!answered.count, "a head of MAXIMUM_HEAD bytes was refused before it ended");
		feed(large, 1);
		connection_take(&connection);
		CHECK(answered.status == 413 && connection.close_after_write && !handled.count,
			"a head that did not end was answered %d", answered.status);
		/* (a head that ends past MAXIMUM_HEAD) */
		connection_reset(0);
		memcpy(large + MAXIMUM_HEAD - 2, "\r\n\r\n", 4);
		feed(large, MAXIMUM_HEAD + 2);
		connection_take(&connection);
		CHECK(answered.status == 413 && !handled.count, "a head ending past the most was answered %d", answered.status);
		/* (one ending at the most is read) */
		connection_reset(0);
		memcpy(large + MAXIMUM_HEAD - 4, "\r\n\r\n", 4);
		feed(large, MAXIMUM_HEAD);
		connection_take(&connection);
		CHECK(!answered.count && handled.count == 1, "a head of MAXIMUM_HEAD bytes was answered %d", answered.status);
		/* (a request refused closes the connection) */
		connection_reset(0);
		feed_text("GET /../etc HTTP/1.1\r\n\r\nGET / HTTP/1.1\r\n\r\n");
		connection_take(&connection);
		CHECK(answered.count == 1 && answered.status == 400 && connection.close_after_write && !handled.count,
			"a refused request was answered %d times, %d", answered.count, answered.status);
		connection_reset(0);
		feed_text("POST /api/x HTTP/1.1\r\nContent-Length: 70000\r\n\r\n");
		connection_take(&connection);
		CHECK(answered.status == 413 && !strcmp(answered.code, "too_large"), "a body too large was answered %d",
			answered.status);
		return 0;
	}
	/* behind the trusted proxy alone, the client's address is its word */
	CASE("forwarded")
	{
		connection_reset(0);
		connection.address = 0x0A000002UL;
		trusted_proxy = "10.0.0.2";
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 203.0.113.9\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == 0xCB007109UL, "the proxy's word was not taken: %lx", connection.real_address);
		/* (the entry the proxy appended is the last: those before it are the
		client's, forged or not) */
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 198.51.100.7, 192.0.2.1,\t203.0.113.9\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == 0xCB007109UL, "an entry before the proxy's was taken: %lx",
			connection.real_address);
		/* (a list longer than the field: its end is kept) */
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 1.1.1.1, 1.1.1.2, 1.1.1.3, 1.1.1.4, 1.1.1.5, 1.1.1.6, 1.1.1.7, "
			"1.1.1.8, 1.1.1.9, 203.0.113.9\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == 0xCB007109UL, "a long list's last entry was not taken: %lx",
			connection.real_address);
		/* (a last entry that is no address: the proxy's own) */
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 203.0.113.9, unknown\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == connection.address, "a bad last entry gave %lx", connection.real_address);
		connection_reset(0);
		connection.address = 0x0A000003UL;
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 203.0.113.9\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == 0x0A000003UL && !connection.forwarded, "another address's word was taken");
		/* (an entry with its port, IPv6 with and without brackets, IPv4 in
		IPv6) */
		{
			static const struct
			{
				const char *entry;
				unsigned long address;
				const char *text;
			} entries[] = {
				{ "203.0.113.9:4711", 0xCB007109UL, "203.0.113.9" },
				{ "::ffff:203.0.113.9", 0xCB007109UL, "203.0.113.9" },
				{ "[::FFFF:cb00:7109]:80", 0xCB007109UL, "203.0.113.9" },
				{ "[2001:db8::1]:443", 1, "2001:db8::1" },
				{ "2001:db8::1", 1, "2001:db8::1" },
				{ "[2001:db8:0:0:ffff::2]", 1, "2001:db8:0:0:ffff::2" },
				{ "2001:0db8:0000:0000:0000:0000:0000:0001", 1, "2001:0db8:0000:0000:0000:0000:0000:0001" },
				{ "2001:db8:0:1::1", 2, "2001:db8:0:1::1" },
				{ "::", 3, "::" },
			};
			static const char *const refused[] = {
				"203.0.113.9:", "203.0.113.9:65536", "203.0.113.9:4x", "203.0.113.9:1:2", "[2001:db8::1", "2001:db8:::1",
				"2001:db8::1::2", "1:2:3:4:5:6:7:8:9", "1:2:3:4:5:6:7", "2001:db8::1%eth0", "[2001:db8::1]:x",
				"[2001:db8::1]x", ":1::2", "1::2:", "12345::1", "::ffff:1.2.3", "::1.2.3.4.5", "[]", "unknown",
				"2001:db8::g", NULL,
			};
			unsigned long keys[4] = { 0, 0, 0, 0 };
			int index;

			for (index = 0; index < (int)(sizeof(entries) / sizeof(entries[0])); index++)
			{
				char head[256];

				connection_reset(0);
				snprintf(head, sizeof(head), "GET / HTTP/1.1\r\nX-Forwarded-For: 198.51.100.7, %s\r\n\r\n",
					entries[index].entry);
				feed_text(head);
				connection_take(&connection);
				CHECK(connection.forwarded && connection.real_address != connection.address &&
					!strcmp(connection.real_text, entries[index].text), "%s gave %lx (%s)", entries[index].entry,
					connection.real_address, connection.real_text);
				if (entries[index].address > 4)
				{
					CHECK(connection.real_address == entries[index].address, "%s was read as %lx", entries[index].entry,
						connection.real_address);
				}
				else if (!keys[entries[index].address])
				{
					keys[entries[index].address] = connection.real_address;
				}
				else
				{
					CHECK(keys[entries[index].address] == connection.real_address, "%s is another /64's key",
						entries[index].entry);
				}
			}
			CHECK(keys[1] != keys[2] && keys[1] != keys[3] && keys[2] != keys[3],
				"two /64s had one key (%lx, %lx, %lx)", keys[1], keys[2], keys[3]);
			for (index = 0; refused[index]; index++)
			{
				char head[256];

				connection_reset(0);
				snprintf(head, sizeof(head), "GET / HTTP/1.1\r\nX-Forwarded-For: 203.0.113.9, %s\r\n\r\n",
					refused[index]);
				feed_text(head);
				connection_take(&connection);
				CHECK(connection.real_address == connection.address && !strcmp(connection.real_text, "10.0.0.2"),
					"%s was taken (as %lx, %s)", refused[index], connection.real_address, connection.real_text);
			}
		}
		/* (the trusted proxy's X-Forwarded-Host and -Proto, with no
		X-Forwarded-For too, or one for the proxy's own address) */
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-Host: panel.example\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.forwarded && connection.real_address == connection.address, "the trusted proxy was not taken");
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 10.0.0.2\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.forwarded, "the trusted proxy forwarding for its own address was not taken");
		trusted_proxy = "";
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX-Forwarded-For: 203.0.113.9\r\n\r\n");
		connection_take(&connection);
		CHECK(connection.real_address == connection.address && !connection.forwarded,
			"a word was taken with no proxy trusted");
		return 0;
	}
	/* dotted addresses: four parts of 0 to 255, then the end, a comma or a
	space */
	CASE("addresses")
	{
		static const char *const refused[] = { "", "1.2.3", "1.2.3.256", "1.2.3.4.5", "1..3.4", "a.b.c.d", "1.2.3.4x",
			"1234.1.1.1", "-1.2.3.4", " 1.2.3.4", "1.2.3.", NULL };
		unsigned long address;
		int index;

		CHECK(parse_address("192.168.0.1", &address) && address == 0xC0A80001UL, "192.168.0.1 was %lx", address);
		CHECK(parse_address("0.0.0.0", &address) && address == 0, "0.0.0.0 was %lx", address);
		CHECK(parse_address("255.255.255.255", &address) && address == 0xFFFFFFFFUL, "the last address was refused");
		CHECK(parse_address("1.2.3.4, 5.6.7.8", &address) && address == 0x01020304UL, "a list's first was not read");
		CHECK(parse_address("1.2.3.4 ", &address) && parse_address("001.002.003.004", &address) &&
			address == 0x01020304UL, "an address before a space, or with leading zeros, was refused");
		for (index = 0; refused[index]; index++)
			CHECK(!parse_address(refused[index], &address), "\"%s\" was taken", refused[index]);
		return 0;
	}
	/* a page's Origin is the host's (the proxy's word for it, from the
	proxy alone), in any case, with its port */
	CASE("origin")
	{
		connection_reset(0);
		snprintf(connection.request.host, sizeof(connection.request.host), "Server.Example:8080");
		snprintf(connection.request.origin, sizeof(connection.request.origin), "http://server.example:8080");
		CHECK(origin_matches(&connection), "the host's own origin did not match");
		snprintf(connection.request.origin, sizeof(connection.request.origin), "https://SERVER.EXAMPLE:8080");
		CHECK(origin_matches(&connection), "another scheme or case did not match");
		snprintf(connection.request.origin, sizeof(connection.request.origin), "http://server.example");
		CHECK(!origin_matches(&connection), "another port matched");
		snprintf(connection.request.origin, sizeof(connection.request.origin), "http://server.example:8080.evil");
		CHECK(!origin_matches(&connection), "a longer host matched");
		snprintf(connection.request.origin, sizeof(connection.request.origin), "server.example:8080");
		CHECK(!origin_matches(&connection), "an origin without a scheme matched");
		connection.request.origin[0] = 0;
		CHECK(!origin_matches(&connection), "no origin matched");
		/* (a page's own post under a referrer policy that hides it: Origin
		null, which the browser's Sec-Fetch-Site alone vouches for) */
		snprintf(connection.request.origin, sizeof(connection.request.origin), "null");
		CHECK(!origin_matches(&connection), "Origin null matched with no Sec-Fetch-Site");
		snprintf(connection.request.fetch_site, sizeof(connection.request.fetch_site), "cross-site");
		CHECK(!origin_matches(&connection), "Origin null matched from another site");
		snprintf(connection.request.fetch_site, sizeof(connection.request.fetch_site), "same-origin");
		CHECK(origin_matches(&connection), "the page's own post with Origin null did not match");
		connection.request.fetch_site[0] = 0;
		snprintf(connection.request.origin, sizeof(connection.request.origin), "https://panel.example");
		connection.request.host[0] = 0;
		CHECK(!origin_matches(&connection), "an origin matched no host");
		snprintf(connection.request.host, sizeof(connection.request.host), "10.0.0.5:8080");
		snprintf(connection.request.forwarded_host, sizeof(connection.request.forwarded_host), "panel.example");
		connection.forwarded = 0;
		connection.real_address = 0xCB007109UL;
		CHECK(!origin_matches(&connection), "a forwarded host was taken from a client");
		connection.forwarded = 1;
		connection.real_address = connection.address;
		CHECK(origin_matches(&connection), "the trusted proxy's forwarded host was not taken");
		return 0;
	}
	/* a query's numbers, by whole names */
	CASE("query")
	{
		CHECK(query_number("after=5&limit=10", "after", 0) == 5 && query_number("after=5&limit=10", "limit", 0) == 10,
			"after or limit was not read");
		CHECK(query_number("xafter=5&after=7", "after", 0) == 7, "a name was found inside another");
		CHECK(query_number("after=x", "after", 3) == 3 && query_number("after=", "after", 3) == 3 &&
			query_number("after", "after", 3) == 3 && query_number("", "after", 3) == 3 &&
			query_number("limit=1", "after", 3) == 3, "a missing number was not the default");
		CHECK(query_number("after=-4", "after", 0) == -4 && query_number("after=12abc", "after", 0) == 12,
			"a number was not read as strtol reads it");
		return 0;
	}
	/* passwords compared whole, and names in any case */
	CASE("compare")
	{
		CHECK(constant_time_equal("secret", "secret") && constant_time_equal("", ""), "the same text was not equal");
		CHECK(!constant_time_equal("secret", "secre") && !constant_time_equal("secre", "secret") &&
			!constant_time_equal("secret", "") && !constant_time_equal("", "secret") &&
			!constant_time_equal("secret", "Secret") && !constant_time_equal("abab", "ab"),
			"different text was equal");
		CHECK(text_equal_case("WebSocket", "websocket") && !text_equal_case("websocket", "websockets") &&
			!text_equal_case("websockets", "websocket") && text_equal_case("", ""), "names were compared wrong");
		return 0;
	}
	/* a client's frames: masked, each length's form, waited for whole,
	pings answered, the limits kept */
	CASE("frames")
	{
		/* (RFC 6455 5.7's masked "Hello") */
		static const unsigned char hello[] = { 0x81, 0x85, 0x37, 0xFA, 0x21, 0x3D, 0x7F, 0x9F, 0x4D, 0x51, 0x58 };
		static unsigned char payload[WS_FRAME + 1];
		const unsigned char *pong;
		int opcode, size, length, index;

		connection_reset(1);
		feed(hello, sizeof(hello));
		ws_read_frames(&connection);
		CHECK(message_taken.count == 1 && !strcmp(message_taken.text, "Hello") && connection.in_length == 0,
			"RFC 6455's example was taken as %s", message_taken.text);
		/* (each byte at a time, in each length's form) */
		memset(payload, 'x', sizeof(payload));
		for (index = 0; index < 3; index++)
		{
			int byte;

			connection_reset(1);
			length = client_frame(frame, 1, 0x1, "abc", 3, index == 0 ? 0 : index == 1 ? 16 : 64, 1);
			for (byte = 0; byte < length; byte++)
			{
				feed(frame + byte, 1);
				ws_read_frames(&connection);
				CHECK(message_taken.count == (byte == length - 1), "a frame of %d bytes was taken after %d", length, byte + 1);
			}
			CHECK(!strcmp(message_taken.text, "abc") && !connection.closing, "the frame's text was %s", message_taken.text);
		}
		connection_reset(1);
		feed_frame(1, 0x1, payload, 300);
		feed_frame(1, 0x1, payload, WS_FRAME);
		ws_read_frames(&connection);
		CHECK(message_taken.count == 2 && message_taken.length == WS_FRAME && !connection.closing,
			"frames of 300 and %d bytes were taken %d times", WS_FRAME, message_taken.count);
		/* (two frames in one read, and a ping between them) */
		connection_reset(1);
		feed_frame(1, 0x1, "one", 3);
		feed_frame(1, 0x9, "beat", 4);
		feed_frame(1, 0xA, "unasked", 7);
		feed_frame(1, 0x1, "two", 3);
		ws_read_frames(&connection);
		CHECK(message_taken.count == 2 && !strcmp(message_taken.text, "two"), "two frames were taken %d times",
			message_taken.count);
		CHECK(server_frame(0, &opcode, &pong, &size) && opcode == 0xA && size == 4 && !memcmp(pong, "beat", 4) &&
			!server_frame(1, &opcode, &pong, &size), "the ping was not answered once with its payload");
		CHECK(connection.out[0] == 0x8A, "the pong was not one final frame");
		/* (a close is answered with one) */
		connection_reset(1);
		feed_frame(1, 0x8, "\x03\xe8", 2);
		feed_frame(1, 0x1, "after", 5);
		ws_read_frames(&connection);
		CHECK(close_code() == 1000 && connection.closing && !message_taken.count, "a close was answered %d", close_code());
		return 0;
	}
	/* frames that close the connection, and why */
	CASE("frame-refusals")
	{
		static unsigned char payload[200];
		static const struct
		{
			int fin, opcode;
			unsigned long long size;
			int form, masked;
			int code;
		} refusals[] = {
			{ 1, 0x1, 5, 0, 0, 1002 },
			{ 1, 0x1 | 0x40, 5, 0, 1, 1002 },
			{ 1, 0x1 | 0x10, 5, 0, 1, 1002 },
			{ 1, 0x1, WS_FRAME + 1, 64, 1, 1009 },
			{ 1, 0x1, 0x8000000000000000ULL, 64, 1, 1009 },
			{ 1, 0x1, 0x100000000ULL, 64, 1, 1009 },
			{ 1, 0x9, 126, 0, 1, 1002 },
			{ 0, 0x9, 4, 0, 1, 1002 },
			{ 1, 0x8, 126, 0, 1, 1002 },
			{ 1, 0x2, 5, 0, 1, 1003 },
			{ 1, 0x3, 5, 0, 1, 1002 },
			{ 1, 0xB, 5, 0, 1, 1002 },
			{ 1, 0x0, 5, 0, 1, 1002 },
		};
		int index;

		for (index = 0; index < (int)(sizeof(refusals) / sizeof(refusals[0])); index++)
		{
			int length;

			connection_reset(1);
			length = client_frame(frame, refusals[index].fin, refusals[index].opcode,
				refusals[index].size <= sizeof(payload) ? payload : NULL, refusals[index].size, refusals[index].form,
				refusals[index].masked);
			feed(frame, length);
			ws_read_frames(&connection);
			CHECK(close_code() == refusals[index].code && connection.closing && connection.close_after_write &&
				!message_taken.count, "frame %d was closed %d, not %d", index, close_code(), refusals[index].code);
			/* (nothing after the close is read) */
			feed_frame(1, 0x1, "more", 4);
			ws_read_frames(&connection);
			CHECK(!message_taken.count, "frame %d's next frame was taken", index);
		}
		return 0;
	}
	/* a message in fragments: one message, control frames between them,
	WS_MESSAGE at most */
	CASE("fragments")
	{
		static unsigned char payload[WS_FRAME];
		int index;

		connection_reset(1);
		feed_frame(0, 0x1, "frag", 4);
		feed_frame(1, 0x9, "", 0);
		feed_frame(0, 0x0, "men", 3);
		ws_read_frames(&connection);
		CHECK(!message_taken.count && connection.message_open, "part of a message was taken");
		feed_frame(1, 0x0, "ted", 3);
		feed_frame(1, 0x1, "whole", 5);
		ws_read_frames(&connection);
		CHECK(message_taken.count == 2 && !strcmp(message_taken.text, "whole") && !connection.message_open &&
			!connection.closing, "the messages were taken %d times, the last %s", message_taken.count,
			message_taken.text);
		connection_reset(1);
		feed_frame(0, 0x1, "frag", 4);
		feed_frame(1, 0x0, "mented", 6);
		ws_read_frames(&connection);
		CHECK(message_taken.count == 1 && !strcmp(message_taken.text, "fragmented"), "a fragmented message was %s",
			message_taken.text);
		/* (a continuation of nothing, a message inside another) */
		connection_reset(1);
		feed_frame(1, 0x0, "lost", 4);
		ws_read_frames(&connection);
		CHECK(close_code() == 1002 && !message_taken.count, "a continuation of nothing was closed %d", close_code());
		connection_reset(1);
		feed_frame(0, 0x1, "one", 3);
		feed_frame(1, 0x1, "two", 3);
		ws_read_frames(&connection);
		CHECK(close_code() == 1002 && !message_taken.count, "a message inside another was closed %d", close_code());
		/* (WS_MESSAGE in fragments, and a byte more) */
		connection_reset(1);
		memset(payload, 'm', sizeof(payload));
		for (index = 0; index < WS_MESSAGE / WS_FRAME; index++)
		{
			feed_frame(index == WS_MESSAGE / WS_FRAME - 1, index ? 0x0 : 0x1, payload, WS_FRAME);
			ws_read_frames(&connection);
		}
		CHECK(message_taken.count == 1 && message_taken.length == WS_MESSAGE && !connection.closing,
			"a message of WS_MESSAGE bytes was not taken");
		connection_reset(1);
		for (index = 0; index < WS_MESSAGE / WS_FRAME; index++)
		{
			feed_frame(0, index ? 0x0 : 0x1, payload, WS_FRAME);
			ws_read_frames(&connection);
		}
		feed_frame(1, 0x0, "!", 1);
		ws_read_frames(&connection);
		CHECK(close_code() == 1009 && !message_taken.count, "a message past WS_MESSAGE was closed %d", close_code());
		return 0;
	}
	/* a connection's output: what was sent makes room, one answer however
	long is taken below OUTPUT_PAUSE, and past it OUTPUT_MOST closes the
	connection */
	CASE("output")
	{
		static unsigned char large[3 * 1024 * 1024];
		static unsigned char tail[5000];

		connection_reset(0);
		out_append(&connection, "abcdef", 6);
		connection.out_sent = 3;
		memset(tail, 'x', sizeof(tail));
		out_append(&connection, tail, sizeof(tail));
		CHECK(connection.out_sent == 0 && connection.out_length == 3 + (int)sizeof(tail) &&
			!memcmp(connection.out, "def", 3) && connection.out[3] == 'x' && connection.out[connection.out_length - 1] == 'x',
			"the output was not kept in order (%d sent of %d)", connection.out_sent, connection.out_length);
		/* (one answer longer than OUTPUT_MOST, onto output below the pause) */
		connection_reset(0);
		out_append(&connection, large, sizeof(large));
		CHECK(!connection.out_overflowed && connection.out_length == (int)sizeof(large), "a long answer was refused");
		out_append(&connection, "more", 4);
		CHECK(connection.out_overflowed && connection.close_after_write && out_pending(&connection) == 0,
			"output past OUTPUT_MOST was taken (%d)", out_pending(&connection));
		out_append(&connection, "more", 4);
		CHECK(out_pending(&connection) == 0, "output was taken after the overflow");
		/* (past the pause, answers up to OUTPUT_MOST in all) */
		connection_reset(1);
		out_append(&connection, large, OUTPUT_PAUSE + 1);
		out_append(&connection, large, OUTPUT_MOST - OUTPUT_PAUSE - 1);
		CHECK(!connection.out_overflowed && out_pending(&connection) == OUTPUT_MOST, "output to OUTPUT_MOST was refused");
		out_append(&connection, "!", 1);
		CHECK(connection.out_overflowed && connection.closing && connection.close_after_write,
			"a WebSocket's output past OUTPUT_MOST was taken");
		return 0;
	}
	/* pipelined requests are taken while their answers wait below
	OUTPUT_PAUSE, and the rest as the answers go */
	CASE("pipelining")
	{
		int index;

		connection_reset(0);
		answer_size = 100 * 1024;
		for (index = 0; index < 6; index++)
			feed_text("GET /app.js HTTP/1.1\r\n\r\n");
		connection_take(&connection);
		CHECK(handled.count == 3 && connection.in_length == 3 * 24, "%d requests were taken past the pause",
			handled.count);
		connection_take(&connection);
		CHECK(handled.count == 3, "a request was taken while the answers waited");
		connection.out_sent += 200 * 1024;
		connection_take(&connection);
		CHECK(handled.count == 5, "the answers went and %d requests were taken", handled.count);
		connection.out_sent = connection.out_length;
		connection_take(&connection);
		CHECK(handled.count == 6 && connection.in_length == 0, "the last request was not taken (%d)", handled.count);
		CHECK(!connection.out_overflowed, "pipelined requests overflowed");
		/* (a WebSocket's frames as well) */
		connection_reset(1);
		connection.out_length = OUTPUT_PAUSE;
		feed_frame(1, 0x1, "held", 4);
		ws_read_frames(&connection);
		CHECK(!message_taken.count, "a frame was taken while the answers waited");
		connection.out_sent = connection.out_length;
		ws_read_frames(&connection);
		CHECK(message_taken.count == 1 && !strcmp(message_taken.text, "held"), "a held frame was not taken");
		connection.out_length = connection.out_sent = 0;
		return 0;
	}
	/* the addresses' table: a minute's allowance each, and room for a new
	address made by the least lately seen that is not locked, else by the
	lock that ends soonest */
	CASE("address-table")
	{
		int index, found;

		memset(http.addresses, 0, sizeof(http.addresses));
		test_now = 5000;
		for (index = 0; index < REQUESTS_UNAUTHENTICATED; index++)
			CHECK(address_allows_request(0x0A000001UL, 0), "request %d was refused", index + 1);
		CHECK(!address_allows_request(0x0A000001UL, 0), "a request past the allowance was taken");
		CHECK(address_allows_request(0x0A000001UL, 1), "a signed-in request was refused");
		CHECK(address_allows_request(0x0A000002UL, 0), "another address was refused");
		test_now += 60;
		CHECK(address_allows_request(0x0A000001UL, 0), "a request a minute later was refused");
		/* (a table full: its records seen in the reverse of their order) */
		memset(http.addresses, 0, sizeof(http.addresses));
		for (index = 0; index < MAXIMUM_ADDRESSES; index++)
		{
			test_now = 1000 + (unsigned long)(MAXIMUM_ADDRESSES - index);
			address_allows_request(0x0A000000UL + (unsigned long)index, 0);
		}
		CHECK(http.addresses[MAXIMUM_ADDRESSES - 1].address == 0x0A000000UL + MAXIMUM_ADDRESSES - 1 &&
			http.addresses[MAXIMUM_ADDRESSES - 1].last_seen == 1001, "the table was not filled in order");
		test_now = 2000;
		http.addresses[MAXIMUM_ADDRESSES - 1].locked_until = test_now + LOGIN_LOCK_SECONDS;
		CHECK(address_allows_request(0x0B000001UL, 0), "a new address was refused with the oldest locked");
		CHECK(http.addresses[MAXIMUM_ADDRESSES - 2].address == 0x0B000001UL,
			"the least lately seen unlocked record did not make room");
		CHECK(http.addresses[MAXIMUM_ADDRESSES - 1].address == 0x0A000000UL + MAXIMUM_ADDRESSES - 1 &&
			address_locked(0x0A000000UL + MAXIMUM_ADDRESSES - 1) == LOGIN_LOCK_SECONDS, "the locked record went");
		/* (every one locked: the lock that ends soonest goes) */
		for (index = 0; index < MAXIMUM_ADDRESSES; index++)
			http.addresses[index].locked_until = test_now + LOGIN_LOCK_SECONDS + (unsigned long)(index == 7 ? 0 : 60);
		CHECK(address_allows_request(0x0C000001UL, 0), "a new address was refused with every record locked");
		for (index = 0, found = -1; index < MAXIMUM_ADDRESSES; index++)
		{
			if (http.addresses[index].address == 0x0C000001UL)
				found = index;
		}
		CHECK(found == 7 && !address_locked(0x0C000001UL), "the new address took record %d", found);
		return 0;
	}
	/* when every slot is taken, the oldest connection that waits for a
	request makes room: no WebSocket, ticket or answer in hand */
	CASE("eviction")
	{
		static unsigned char pending[16];
		int index;

		memset(http.connections, 0, sizeof(http.connections));
		for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
		{
			http.connections[index].socket = 10 + index;
			http.connections[index].head_started = 100 + (Uint64)index;
		}
		http.connections[0].websocket = 1;
		http.connections[0].head_started = 1;
		http.connections[1].ticket = (struct server_ticket *)pending;
		http.connections[1].head_started = 2;
		http.connections[2].out = pending;
		http.connections[2].out_length = 8;
		http.connections[2].head_started = 3;
		http.connections[9].head_started = 50;
		http.connections[5].head_started = 40;
		http.connections[5].have_head = 1;
		CHECK(connection_to_evict() == 5, "connection %d was evicted, not the oldest waiting", connection_to_evict());
		/* (a socket 0 is a connection: -1 is a free slot) */
		http.connections[6].socket = 0;
		http.connections[6].head_started = 30;
		CHECK(connection_to_evict() == 6, "a connection on socket 0 was taken for a free slot (%d)", connection_to_evict());
		http.connections[6].socket = -1;
		CHECK(connection_to_evict() == 5, "a free slot was evicted (%d)", connection_to_evict());
		for (index = 3; index < MAXIMUM_CONNECTIONS; index++)
			http.connections[index].websocket = 1;
		CHECK(connection_to_evict() == -1, "a busy connection was evicted (%d)", connection_to_evict());
		memset(http.connections, 0, sizeof(http.connections));
		return 0;
	}
	/* a connection's deadlines: a request within HEAD_SECONDS of its
	opening or its last answer (one that sends nothing too), and silence
	within IDLE_SECONDS but for a signed-in WebSocket */
	CASE("deadlines")
	{
		static unsigned char pending[16];
		const Uint64 second = 1000000000ULL;

		connection_reset(0);
		CHECK(!connection_overdue(&connection, HEAD_SECONDS * second), "a connection was closed in time");
		CHECK(connection_overdue(&connection, HEAD_SECONDS * second + 1), "a silent connection was kept past its head's time");
		feed_text("GET / HTTP/1.1\r\n");
		CHECK(connection_overdue(&connection, HEAD_SECONDS * second + 1), "a slow head was kept");
		connection.ticket = (struct server_ticket *)pending;
		CHECK(!connection_overdue(&connection, 2 * IDLE_SECONDS * second), "a connection waiting for its ticket was closed");
		connection.ticket = NULL;
		connection.out = pending;
		connection.out_length = 8;
		CHECK(!connection_overdue(&connection, IDLE_SECONDS * second), "an answer being sent was closed");
		CHECK(connection_overdue(&connection, IDLE_SECONDS * second + 1), "an answer unread too long was kept");
		connection.out = NULL;
		connection.out_length = 0;
		connection_reset(1);
		connection.role = _server_role_admin;
		CHECK(!connection_overdue(&connection, 10 * IDLE_SECONDS * second), "a signed-in WebSocket was closed");
		connection.closing = 1;
		CHECK(connection_overdue(&connection, IDLE_SECONDS * second + 1), "a closing WebSocket was kept unread");
		connection_reset(1);
		connection.role = _server_role_none;
		connection.last_activity = 20 * second;
		CHECK(!connection_overdue(&connection, 20 * second), "a WebSocket was held to a request's time");
		CHECK(connection_overdue(&connection, (20 + IDLE_SECONDS) * second + 1), "a silent WebSocket was kept");
		/* (a time later than now, as a request answered since it was read) */
		connection_reset(0);
		connection.head_started = connection.last_activity = 100 * second;
		CHECK(!connection_overdue(&connection, 50 * second), "a connection was closed for a time to come");
		return 0;
	}
	/* the messages queued before a close go before its frame */
	CASE("close-order")
	{
		static const char *const texts[] = { "one", "two" };
		const unsigned char *payload;
		int opcode, size, index;

		connection_reset(1);
		for (index = 0; index < 2; index++)
		{
			struct ws_message *message = malloc(sizeof(*message) + 4);

			message->next = NULL;
			message->length = 3;
			memcpy(message->text, texts[index], 4);
			if (connection.queue_tail)
				connection.queue_tail->next = message;
			else
				connection.queue_head = message;
			connection.queue_tail = message;
			connection.queued++;
		}
		ws_close(&connection, 1008, "denied");
		CHECK(!connection.queue_head && !connection.queue_tail && !connection.queued, "the queue was left");
		for (index = 0; index < 2; index++)
		{
			CHECK(server_frame(index, &opcode, &payload, &size) && opcode == 0x1 && size == 3 &&
				!memcmp(payload, texts[index], 3), "frame %d was not the queued message", index);
		}
		CHECK(server_frame(2, &opcode, &payload, &size) && opcode == 0x8 && close_code() == 1008,
			"the close did not follow the queued messages");
		return 0;
	}
	/* a head's lines end with CRLF: a bare LF (an empty line of one
	included, which a proxy in front may not see end the head), a CR inside
	a line, and anything after the empty line are refused */
	CASE("bare-lf")
	{
		static const char *const refused[] = {
			"GET / HTTP/1.1\nHost: x\r\n", "GET / HTTP/1.1\r\nHost: x\n", "GET / HTTP/1.1\r\n\n",
			"GET / HTTP/1.1\r\nX: a\n\r\n", NULL,
		};
		int index;

		for (index = 0; refused[index]; index++)
			CHECK(head_refused(refused[index], 400, "bad_request"), "bare LF %d was taken", index);
		CHECK(head_refused("GET / HTTP/1.1\r\nX: a\rb\r\n", 400, "bad_request"), "a CR inside a line was taken");
		CHECK(head_refused("GET / HTTP/1.1\r\nX: a\r\n\r\nGET /evil HTTP/1.1\r\n", 400, "bad_request"),
			"a line after the empty one was taken");
		CHECK(!head_refused("GET / HTTP/1.1\r\nX: a\r\n\r\n", 400, "bad_request") &&
			!head_refused("GET / HTTP/1.1\r\nX: a\r\n", 400, "bad_request"), "a head of CRLF lines was refused");
		/* (through a connection: what ends at CRLFCRLF, with a bare-LF
		empty line before it) */
		connection_reset(0);
		feed_text("GET / HTTP/1.1\r\nX: a\n\nPOST /api/say HTTP/1.1\r\nContent-Length: 0\r\n\r\n");
		connection_take(&connection);
		CHECK(answered.count == 1 && answered.status == 400 && !handled.count && connection.close_after_write,
			"a head with a bare-LF empty line was answered %d, handed on %d times", answered.status, handled.count);
		return 0;
	}
	/* a change by the page's cookie needs its Origin (403 origin) and its
	X-CSRF-Token (403 csrf); a read needs neither, nor a change by a Bearer
	token, which no browser sends unasked */
	CASE("auth-csrf")
	{
		char token[2 * TOKEN_BYTES + 1], other_token[2 * TOKEN_BYTES + 1], csrf[2 * CSRF_BYTES + 1];
		char wrong[2 * CSRF_BYTES + 1], head[768];
		struct session *session;

		sign_in_reset("", "", "");
		session = session_new(_server_role_admin, "admin", token);
		snprintf(csrf, sizeof(csrf), "%s", session->csrf);
		snprintf(wrong, sizeof(wrong), "%s", csrf);
		wrong[0] = wrong[0] == 'a' ? 'b' : 'a';
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nCookie: opence_session=%s\r\n"
			"X-CSRF-Token: %s\r\n", token, csrf);
		request_from(0x0A000009UL, head, "{\"text\":\"hi\"}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "origin") && !tickets,
			"a change by the cookie without its Origin was answered %d %s", answered.status,
			answered.code ? answered.code : "");
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: https://server.example\r\n"
			"Cookie: opence_session=%s\r\n", token);
		request_from(0x0A000009UL, head, "{}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "csrf") && !tickets,
			"a change by the cookie without its CSRF token was answered %d", answered.status);
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: https://server.example\r\n"
			"Cookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, wrong);
		request_from(0x0A000009UL, head, "{}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "csrf") && !tickets,
			"a change by the cookie with a wrong CSRF token was answered %d", answered.status);
		/* (another session's CSRF token is not this one's) */
		session_new(_server_role_admin, "other", other_token);
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: https://server.example\r\n"
			"Cookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", other_token, csrf);
		request_from(0x0A000009UL, head, "{}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "csrf"), "another session's CSRF token was taken");
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: https://server.example\r\n"
			"Cookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, csrf);
		request_from(0x0A000009UL, head, "{}");
		CHECK(!answered.count && tickets == 1 && !strcmp(ticket_made.by, "admin") &&
			ticket_made.role == _server_role_admin, "a change by the cookie with both was answered %d", answered.status);
		/* (an Origin of another site: refused, whatever the session) */
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: https://evil.example\r\n"
			"Cookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, csrf);
		request_from(0x0A000009UL, head, "{}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "origin") && !tickets, "another site's change was taken");
		/* (a read by the cookie) */
		snprintf(head, sizeof(head), "GET /api/status HTTP/1.1\r\nHost: server.example\r\nCookie: opence_session=%s\r\n",
			token);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 200 && served && !strcmp(served, "snapshot"), "a read by the cookie was answered %d",
			answered.status);
		/* (a change by a Bearer token) */
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %s\r\n", token);
		request_from(0x0A000009UL, head, "{}");
		CHECK(!answered.count && tickets == 1, "a change by a Bearer token was answered %d %s", answered.status,
			answered.code ? answered.code : "");
		/* (a sign out by the cookie is a change too) */
		snprintf(head, sizeof(head), "POST /api/logout HTTP/1.1\r\nHost: server.example\r\nOrigin: https://server.example\r\n"
			"Cookie: opence_session=%s\r\n", token);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 403 && !strcmp(answered.code, "csrf") && session_find(token),
			"a sign out without the CSRF token was taken");
		return 0;
	}
	/* no session: 401 denied, but for the health, the page and the sign in */
	CASE("auth-denied")
	{
		static const char unknown[] = "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";
		char token[2 * TOKEN_BYTES + 1], head[512];

		sign_in_reset("secret", "", "");
		session_new(_server_role_admin, "admin", token);
		request_from(0x0A000009UL, "GET /api/status HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(answered.status == 401 && !strcmp(answered.code, "denied"), "a read without a session was answered %d",
			answered.status);
		snprintf(head, sizeof(head), "GET /api/status HTTP/1.1\r\nHost: server.example\r\nCookie: opence_session=%s\r\n",
			unknown);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 401, "an unknown cookie was answered %d", answered.status);
		snprintf(head, sizeof(head), "GET /api/status HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %s\r\n",
			unknown);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 401, "an unknown Bearer token was answered %d", answered.status);
		snprintf(head, sizeof(head), "GET /api/status HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %.32s\r\n",
			token);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 401, "half a token was answered %d", answered.status);
		snprintf(head, sizeof(head), "GET /api/status HTTP/1.1\r\nHost: server.example\r\nAuthorization: Basic %s\r\n",
			token);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 401, "a token not Bearer was answered %d", answered.status);
		request_from(0x0A000009UL, "POST /api/logout HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(answered.status == 401, "a sign out without a session was answered %d", answered.status);
		request_from(0x0A000009UL, "GET /api/logout HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(answered.status == 405, "GET /api/logout was answered %d", answered.status);
		request_from(0x0A000009UL, "GET /api/login HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(answered.status == 405, "GET /api/login was answered %d", answered.status);
		request_from(0x0A000009UL, "GET /healthz HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(served && !strcmp(served, "healthz"), "the health was not served");
		request_from(0x0A000009UL, "GET /index.html HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(served && !strcmp(served, "asset"), "the page was not served");
		request_from(0x0A000009UL, "POST /index.html HTTP/1.1\r\nHost: server.example\r\n", NULL);
		CHECK(answered.status == 405, "a POST to the page was answered %d", answered.status);
		snprintf(head, sizeof(head), "GET /api/session HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %s\r\n",
			token);
		request_from(0x0A000009UL, head, NULL);
		CHECK(answered.status == 200 && strstr(answered.body, "\"role\":\"admin\""), "the session was answered %d %s",
			answered.status, answered.body);
		return 0;
	}
	/* each route's role: a session's below it is answered 403 forbidden */
	CASE("route-roles")
	{
		static const struct
		{
			const char *method, *path;
			int role;
		} routes[] = {
			{ "GET", "/api/status", _server_role_viewer }, { "GET", "/api/players", _server_role_viewer },
			{ "GET", "/api/log", _server_role_viewer }, { "GET", "/api/events", _server_role_viewer },
			{ "GET", "/api/rotation", _server_role_viewer }, { "GET", "/api/gametypes", _server_role_viewer },
			{ "GET", "/api/maps", _server_role_viewer }, { "POST", "/api/command", _server_role_viewer },
			{ "GET", "/api/session", _server_role_viewer },
			{ "POST", "/api/players/3/kick", _server_role_moderator }, { "POST", "/api/say", _server_role_moderator },
			{ "POST", "/api/rotation/next", _server_role_moderator },
			{ "POST", "/api/rotation/play", _server_role_moderator },
			{ "PUT", "/api/rotation", _server_role_admin }, { "POST", "/api/gametypes/ctf.txt", _server_role_admin },
			{ "DELETE", "/api/gametypes/ctf.txt", _server_role_admin }, { "GET", "/api/bans", _server_role_admin },
			{ "DELETE", "/api/bans/1", _server_role_admin }, { "GET", "/api/settings", _server_role_admin },
			{ "PUT", "/api/settings", _server_role_admin }, { "POST", "/api/bans", _server_role_admin },
			{ "POST", "/api/stop", _server_role_owner },
		};
		char tokens[_server_role_admin + 1][2 * TOKEN_BYTES + 1];
		int index, role;

		sign_in_reset("", "", "");
		for (role = _server_role_viewer; role <= _server_role_admin; role++)
			session_new(role, server_role_name(role), tokens[role]);
		for (index = 0; index < (int)(sizeof(routes) / sizeof(routes[0])); index++)
		{
			CHECK(route_role(routes[index].method, routes[index].path) == routes[index].role, "%s %s needs %d, not %d",
				routes[index].method, routes[index].path, route_role(routes[index].method, routes[index].path),
				routes[index].role);
			for (role = _server_role_viewer; role <= _server_role_admin; role++)
			{
				char head[512];
				int forbidden;

				snprintf(head, sizeof(head), "%s %s HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %s\r\n",
					routes[index].method, routes[index].path, tokens[role]);
				request_from(0x0A000009UL, head, "{}");
				forbidden = answered.status == 403 && answered.code && !strcmp(answered.code, "forbidden");
				CHECK(forbidden == (role < routes[index].role), "%s %s by a %s was answered %d", routes[index].method,
					routes[index].path, server_role_name(role), answered.status);
				CHECK(forbidden || tickets || served || answered.status == 200, "%s %s by a %s was not handed on (%d)",
					routes[index].method,
					routes[index].path, server_role_name(role), answered.status);
			}
		}
		return 0;
	}
	/* a session lasts 12 hours at most, and 30 minutes unused */
	CASE("session-expiry")
	{
		char token[2 * TOKEN_BYTES + 1];
		unsigned long created;
		int index, used;

		sign_in_reset("", "", "");
		test_now = created = 100000;
		session_new(_server_role_viewer, "v", token);
		test_now += SESSION_IDLE_SECONDS;
		CHECK(session_find(token), "a session was closed after exactly 30 minutes unused");
		while (test_now + SESSION_IDLE_SECONDS <= created + SESSION_SECONDS)
		{
			test_now += SESSION_IDLE_SECONDS;
			CHECK(session_find(token), "a session used every 30 minutes was closed at %lu s", test_now - created);
		}
		test_now = created + SESSION_SECONDS;
		CHECK(session_find(token), "a session was closed at exactly 12 hours");
		test_now++;
		CHECK(!session_find(token), "a session was kept past 12 hours");
		for (index = 0, used = 0; index < MAXIMUM_SESSIONS; index++)
			used += http.sessions[index].used;
		CHECK(!used, "a lapsed session kept its slot");
		CHECK(!session_find(token), "a lapsed session came back");
		session_new(_server_role_viewer, "v", token);
		test_now += SESSION_IDLE_SECONDS + 1;
		CHECK(!session_find(token), "a session unused past 30 minutes was kept");
		return 0;
	}
	/* MAXIMUM_SESSIONS at once: a new one takes the place of the one used
	least lately */
	CASE("session-cap")
	{
		char tokens[MAXIMUM_SESSIONS + 1][2 * TOKEN_BYTES + 1];
		int index, used;

		sign_in_reset("", "", "");
		test_now = 200000;
		for (index = 0; index < MAXIMUM_SESSIONS; index++)
		{
			test_now++;
			session_new(_server_role_viewer, "v", tokens[index]);
		}
		for (index = 0, used = 0; index < MAXIMUM_SESSIONS; index++)
			used += http.sessions[index].used;
		CHECK(used == MAXIMUM_SESSIONS, "%d sessions of %d", used, MAXIMUM_SESSIONS);
		test_now++;
		CHECK(session_find(tokens[0]), "the first session was not found");
		test_now++;
		session_new(_server_role_viewer, "v", tokens[MAXIMUM_SESSIONS]);
		CHECK(!session_find(tokens[1]), "the session used least lately was kept");
		CHECK(session_find(tokens[0]) && session_find(tokens[MAXIMUM_SESSIONS]), "a session used lately went");
		for (index = 2; index < MAXIMUM_SESSIONS; index++)
			CHECK(session_find(tokens[index]), "session %d went", index);
		return 0;
	}
	/* five wrong passwords from an address in ten minutes lock it for ten
	(429 locked, with when to try again), its right password too; others'
	are not */
	CASE("lockout")
	{
		char token[2 * TOKEN_BYTES + 1];
		int index;

		sign_in_reset("right", "", "");
		test_now = 300000;
		for (index = 0; index < LOGIN_FAILURES; index++)
		{
			CHECK(!sign_in(0x0A000011UL, "wrong", token) && answered.status == 401 && !strcmp(answered.code, "denied"),
				"wrong password %d was answered %d", index + 1, answered.status);
		}
		CHECK(!sign_in(0x0A000011UL, "right", token) && answered.status == 429 && !strcmp(answered.code, "locked") &&
			answered.retry_after == LOGIN_LOCK_SECONDS, "a locked address was answered %d %s (%d)", answered.status,
			answered.code ? answered.code : "", answered.retry_after);
		test_now += 60;
		CHECK(!sign_in(0x0A000011UL, "right", token) && answered.retry_after == LOGIN_LOCK_SECONDS - 60,
			"a minute into the lock, Retry-After was %d", answered.retry_after);
		CHECK(sign_in(0x0A000012UL, "right", token), "another address was locked out");
		test_now += LOGIN_LOCK_SECONDS - 60;
		CHECK(sign_in(0x0A000011UL, "right", token), "the lock did not end after ten minutes (%d)", answered.status);
		/* (failures further apart than the window) */
		for (index = 0; index < LOGIN_FAILURES - 1; index++)
			sign_in(0x0A000013UL, "wrong", token);
		test_now += LOGIN_WINDOW_SECONDS;
		for (index = 0; index < LOGIN_FAILURES - 1; index++)
			sign_in(0x0A000013UL, "wrong", token);
		CHECK(sign_in(0x0A000013UL, "right", token), "failures in two windows locked the address (%d)", answered.status);
		return 0;
	}
	/* 30 passwords a minute, from every address in all: past them, 429
	rate_limited until the minute's end */
	CASE("login-budget")
	{
		char token[2 * TOKEN_BYTES + 1];
		int index;

		sign_in_reset("right", "", "");
		test_now = 400020;
		for (index = 0; index < LOGIN_BUDGET; index++)
			CHECK(sign_in(0x0B000000UL + (unsigned long)index, "right", token), "password %d was refused", index + 1);
		test_now += 15;
		CHECK(!sign_in(0x0C000001UL, "right", token) && answered.status == 429 && !strcmp(answered.code, "rate_limited")
			&& answered.retry_after == 45, "a password past the budget was answered %d %s (%d)", answered.status,
			answered.code ? answered.code : "", answered.retry_after);
		test_now += 45;
		CHECK(sign_in(0x0C000001UL, "right", token), "a password of the next minute was refused");
		return 0;
	}
	/* an empty password is no password: it never signs in, whichever are
	set */
	CASE("empty-password")
	{
		char token[2 * TOKEN_BYTES + 1];

		sign_in_reset("", "", "");
		CHECK(!sign_in(0x0A000021UL, "", token) && answered.status == 401, "an empty password signed in with none set");
		CHECK(!sign_in(0x0A000021UL, "x", token) && answered.status == 401, "a password signed in with none set");
		sign_in_reset("", "", "view");
		CHECK(!sign_in(0x0A000022UL, "", token) && answered.status == 401, "an empty password signed in");
		CHECK(sign_in(0x0A000022UL, "view", token) && strstr(answered.body, "\"role\":\"viewer\""),
			"the viewer's password was answered %d", answered.status);
		sign_in_reset("admin", "mod", "view");
		CHECK(sign_in(0x0A000023UL, "mod", token) && strstr(answered.body, "\"role\":\"moderator\"") &&
			sign_in(0x0A000023UL, "admin", token) && strstr(answered.body, "\"role\":\"admin\""),
			"the moderator's or admin's password gave another role");
		CHECK(!sign_in(0x0A000023UL, "admi", token) && !sign_in(0x0A000023UL, "adminx", token),
			"part of a password, or more, signed in");
		request_from(0x0A000024UL, "POST /api/login HTTP/1.1\r\nHost: server.example\r\n", "{\"by\":\"x\"}");
		CHECK(answered.status == 400 && !strcmp(answered.code, "schema"), "a sign in without a password was answered %d",
			answered.status);
		return 0;
	}
	/* a password changed ends its role's sessions and closes its remote
	consoles (1008); the other roles' are kept, and a change of other
	settings ends nothing */
	CASE("password-change")
	{
		char admin_token[2 * TOKEN_BYTES + 1], viewer_token[2 * TOKEN_BYTES + 1];
		struct connection *admin_console = &http.connections[0], *viewer_console = &http.connections[1];

		sign_in_reset("admin1", "mod1", "view1");
		CHECK(sign_in(0x0A000031UL, "admin1", admin_token) && sign_in(0x0A000032UL, "view1", viewer_token),
			"the passwords did not sign in");
		memset(admin_console, 0, sizeof(*admin_console));
		admin_console->socket = 20;
		admin_console->websocket = 1;
		admin_console->role = _server_role_admin;
		memset(viewer_console, 0, sizeof(*viewer_console));
		viewer_console->socket = 21;
		viewer_console->websocket = 1;
		viewer_console->role = _server_role_viewer;
		setting_changes++;
		passwords_check();
		CHECK(session_find(admin_token) && session_find(viewer_token) && !admin_console->closing &&
			!viewer_console->closing, "a change of other settings ended a session");
		setting_admin = "admin2";
		setting_changes++;
		passwords_check();
		CHECK(!session_find(admin_token), "the admin's session was kept after its password changed");
		CHECK(session_find(viewer_token) && !viewer_console->closing && viewer_console->role == _server_role_viewer,
			"the viewer's session or console ended with the admin's password");
		CHECK(admin_console->closing && admin_console->role == _server_role_none && admin_console->out_length >= 4 &&
			admin_console->out[0] == 0x88 && (admin_console->out[2] << 8 | admin_console->out[3]) == 1008,
			"the admin's console was not closed 1008");
		CHECK(!sign_in(0x0A000031UL, "admin1", admin_token) && sign_in(0x0A000031UL, "admin2", admin_token),
			"the old password signed in, or the new one did not");
		free(admin_console->out);
		memset(admin_console, 0, sizeof(*admin_console));
		memset(viewer_console, 0, sizeof(*viewer_console));
		admin_console->socket = viewer_console->socket = -1;
		return 0;
	}
	/* the owner's password signs in from this machine alone (127.0.0.0/8,
	not through the trusted proxy): from anywhere else it is refused
	"forbidden" and counted as a wrong password; its session is used from
	this machine alone, and ends when the password changes */
	CASE("owner-local")
	{
		char token[2 * TOKEN_BYTES + 1], other[2 * TOKEN_BYTES + 1], head[512];
		int index;

		sign_in_reset("admin-pass", "", "");
		setting_owner = "owner-pass";
		setting_changes++;
		test_now = 700000;
		CHECK(sign_in(0x7F000001UL, "owner-pass", token) && strstr(answered.body, "\"role\":\"owner\""),
			"the owner's password from this machine was answered %d: %s", answered.status, answered.body);
		snprintf(head, sizeof(head), "GET /api/session HTTP/1.1\r\nHost: server.example\r\nAuthorization: Bearer %s\r\n",
			token);
		request_from(0x7F000002UL, head, NULL);
		CHECK(answered.status == 200, "the owner's session from this machine was answered %d", answered.status);
		request_from(0x0A000051UL, head, NULL);
		CHECK(answered.status == 401, "the owner's session from another address was answered %d", answered.status);
		request_forwarded = 1;
		request_from(0x7F000001UL, head, NULL);
		CHECK(answered.status == 401, "the owner's session through the proxy was answered %d", answered.status);
		CHECK(!sign_in(0x7F000001UL, "owner-pass", other) && answered.status == 403 && !strcmp(answered.code, "forbidden"),
			"the owner's password through the proxy was answered %d", answered.status);
		request_forwarded = 0;
		for (index = 0; index < LOGIN_FAILURES; index++)
		{
			CHECK(!sign_in(0x0A000052UL, "owner-pass", other) && answered.status == 403 &&
				!strcmp(answered.code, "forbidden"), "the owner's password from another address was answered %d (%d)",
				answered.status, index + 1);
		}
		CHECK(!sign_in(0x0A000052UL, "admin-pass", other) && answered.status == 429,
			"the owner's password from elsewhere was not counted as a wrong one (%d)", answered.status);
		CHECK(sign_in(0x7F000003UL, "admin-pass", other) && strstr(answered.body, "\"role\":\"admin\""),
			"the admin's password from this machine gave another role");
		setting_owner = "owner-pass-2";
		setting_changes++;
		passwords_check();
		request_from(0x7F000002UL, head, NULL);
		CHECK(answered.status == 401, "the owner's session was kept after its password changed (%d)", answered.status);
		return 0;
	}
	/* Origin: null from the page itself, which only the browser's
	Sec-Fetch-Site: same-origin vouches for */
	CASE("fetch-site")
	{
		char token[2 * TOKEN_BYTES + 1], head[768];
		struct session *session;
		static const char *const sites[] = { "cross-site", "same-site", "none", "Same-Origin", NULL };
		int index;

		sign_in_reset("", "", "");
		session = session_new(_server_role_admin, "admin", token);
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: null\r\n"
			"Sec-Fetch-Site: same-origin\r\nCookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, session->csrf);
		request_from(0x0A000041UL, head, "{}");
		CHECK(!answered.count && tickets == 1, "the page's own change with Origin null was answered %d", answered.status);
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: null\r\n"
			"sec-fetch-site:  same-origin \r\nCookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, session->csrf);
		request_from(0x0A000041UL, head, "{}");
		CHECK(!answered.count && tickets == 1, "Sec-Fetch-Site in lower case was not read");
		for (index = 0; sites[index]; index++)
		{
			snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: null\r\n"
				"Sec-Fetch-Site: %s\r\nCookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", sites[index], token,
				session->csrf);
			request_from(0x0A000041UL, head, "{}");
			CHECK(answered.status == 403 && !strcmp(answered.code, "origin") && !tickets,
				"Origin null with Sec-Fetch-Site %s was answered %d", sites[index], answered.status);
		}
		snprintf(head, sizeof(head), "POST /api/say HTTP/1.1\r\nHost: server.example\r\nOrigin: null\r\n"
			"Cookie: opence_session=%s\r\nX-CSRF-Token: %s\r\n", token, session->csrf);
		request_from(0x0A000041UL, head, "{}");
		CHECK(answered.status == 403 && !strcmp(answered.code, "origin"), "Origin null alone was answered %d",
			answered.status);
		return 0;
	}
	/* POST /api/rotation/play's sv_map line: a map and gametype, or a
	rotation entry's whole text, which the page sends for an entry with
	rules, as long as a rotation entry may be */
	CASE("rotation-play")
	{
		static const char entry_text[] = "custom_maps/tsce_multiplayerv1:team_slayer+ff=off+team_vehicles=custom+"
			"warthogs=4+rocket_warthogs=4+ghosts=4+scorpions=4+banshees=4+gun_turrets=4+vehicle_respawn_time=60";
		char body[1024], line[512], expected[512], longest[SERVER_ENTRY_TEXT_SIZE + 1];
		const char *refused;

		snprintf(body, sizeof(body), "{\"map\":\"bloodgulch\",\"gametype\":\"ctf\"}");
		refused = rotation_play_line(body, (int)strlen(body), line, sizeof(line));
		CHECK(!refused && !strcmp(line, "sv_map bloodgulch ctf"), "a map and gametype: %s", refused ? refused : line);
		CHECK(sizeof(entry_text) - 1 > 127, "the entry is %d characters", (int)sizeof(entry_text) - 1);
		snprintf(body, sizeof(body), "{\"map\":\"%s\"}", entry_text);
		snprintf(expected, sizeof(expected), "sv_map %s ", entry_text);
		refused = rotation_play_line(body, (int)strlen(body), line, sizeof(line));
		CHECK(!refused && !strcmp(line, expected), "an entry of %d characters: %s", (int)sizeof(entry_text) - 1,
			refused ? refused : line);
		/* (a rotation entry's most, 255 characters, and not one more) */
		memset(longest, 'a', SERVER_ENTRY_TEXT_SIZE - 1);
		longest[SERVER_ENTRY_TEXT_SIZE - 1] = 0;
		snprintf(body, sizeof(body), "{\"map\":\"%s\"}", longest);
		refused = rotation_play_line(body, (int)strlen(body), line, sizeof(line));
		CHECK(!refused && strlen(line) == 7 + SERVER_ENTRY_TEXT_SIZE - 1 + 1, "255 characters: %s",
			refused ? refused : line);
		longest[SERVER_ENTRY_TEXT_SIZE - 1] = 'a';
		longest[SERVER_ENTRY_TEXT_SIZE] = 0;
		snprintf(body, sizeof(body), "{\"map\":\"%s\"}", longest);
		refused = rotation_play_line(body, (int)strlen(body), line, sizeof(line));
		CHECK(refused && strstr(refused, "255 characters at most"), "256 characters: %s", refused ? refused : line);
		/* (the refusals: no map, a space in it or in the gametype) */
		snprintf(body, sizeof(body), "{\"gametype\":\"ctf\"}");
		CHECK(rotation_play_line(body, (int)strlen(body), line, sizeof(line)), "no map was taken");
		snprintf(body, sizeof(body), "{\"map\":\"\"}");
		CHECK(rotation_play_line(body, (int)strlen(body), line, sizeof(line)), "an empty map was taken");
		snprintf(body, sizeof(body), "{\"map\":\"bloodgulch ctf\"}");
		CHECK(rotation_play_line(body, (int)strlen(body), line, sizeof(line)), "a map with a space was taken");
		snprintf(body, sizeof(body), "{\"map\":\"bloodgulch\",\"gametype\":\"ctf sv_stop\"}");
		refused = rotation_play_line(body, (int)strlen(body), line, sizeof(line));
		CHECK(refused && !strcmp(refused, "a gametype holds no space"), "a gametype with a space: %s",
			refused ? refused : line);
		return 0;
	}
	/* a /ws upgrade counts against its address's allowance, as other
	requests do: 429 past it */
	CASE("ws-allowance")
	{
		static const char good[] = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n"
			"Sec-WebSocket-Protocol: opence-rcon-1\r\n";
		char token[2 * TOKEN_BYTES + 1], headers[512];
		int index;

		sign_in_reset("", "", "");
		test_now = 500000;
		for (index = 0; index < REQUESTS_UNAUTHENTICATED; index++)
		{
			upgrade_request(good);
			connection_take(&connection);
			CHECK(connection.websocket, "upgrade %d was refused (%d)", index + 1, answered.status);
		}
		upgrade_request(good);
		connection_take(&connection);
		CHECK(!connection.websocket && answered.status == 429 && !strcmp(answered.code, "rate_limited"),
			"an upgrade past the allowance was answered %d", answered.status);
		/* (a signed-in one's allowance, a Bearer token's) */
		session_new(_server_role_admin, "admin", token);
		snprintf(headers, sizeof(headers), "%sAuthorization: Bearer %s\r\n", good, token);
		upgrade_request(headers);
		connection_take(&connection);
		CHECK(connection.websocket && auth_answered == 1, "a signed-in upgrade was refused (%d)", answered.status);
		test_now += 60;
		upgrade_request(good);
		connection_take(&connection);
		CHECK(connection.websocket, "an upgrade a minute later was refused (%d)", answered.status);
		return 0;
	}
	fprintf(stderr, "unknown case: %s\n", case_name);
	return 2;
}

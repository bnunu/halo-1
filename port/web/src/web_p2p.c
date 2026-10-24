/*
WEB_P2P.C

Internet play's WebRTC in the browser (port/linux/src/p2p_internal.h's
p2p_webrtc_*, which the native builds have in p2p_webrtc.c). A browser has
no UDP socket: its tunnel to another machine of internet play, a native
build or a browser, is a data channel of an RTCPeerConnection the page
makes (port/web/site/p2p.js). p2p.c seals and opens the tunnel's packets as
it does on UDP; here they go to the page, and what the page receives is
handed to the tunnel's socket (web_net.c's web_net_inject) as if it came
from 198.18.x.y, the connection's stand-in address.

The page keeps a connection ready (a spare: its offer made, its addresses
gathered), so that a message to another machine can carry this end's ICE
credentials and addresses at once; a request to join takes it, as does a
joiner a host made a session for, and the page makes another. Each
connection's offer is the page's; the other end's description the page
makes up from what signalling told: its credentials (a native build's come
from the session's secret, p2p_webrtc_credentials), its certificate's hash
and its addresses. A native build is an ICE-lite agent and a DTLS server; of
two browsers, the host is the DTLS server.

The page's calls come on its own thread; the p2p thread's run under
p2p_lock, and what both touch (a connection's description) under
description_lock.
*/

#include "platform.h"
#include "posix.h"
#include "p2p_internal.h"

#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	/* a peer's each, a request's and spares */
	MAXIMUM_CONNECTIONS = P2P_MAXIMUM_PEERS + 4,
	/* the text of a description or an answer's details */
	TEXT_SIZE = 512,
	/* the stand-in port of a connection's address */
	STAND_IN_PORT = 5000,
};

enum
{
	_connection_unused,
	/* made, for the next that needs one */
	_connection_spare,
	/* the current request to join's */
	_connection_request,
	_connection_peer,
};

/* what the page is asked (web_js_p2p) */
enum
{
	_command_create = 1,
	_command_connect,
	_command_candidates,
	_command_send,
	_command_close,
};

struct connection
{
	int role;
	int peer;
	char peer_name[2 * P2P_IDENTIFIER_SIZE + 1];
	int is_host;
	/* this end's, as the page described it (under description_lock) */
	int described;
	char ufrag[P2P_ICE_UFRAG_SIZE];
	char password[P2P_ICE_PASSWORD_SIZE];
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	int candidate_count;
	/* (its own addresses, as mDNS names) */
	int name_count;
	char names[P2P_MAXIMUM_WEBRTC_NAMES][P2P_WEBRTC_NAME_SIZE];
	unsigned short name_ports[P2P_MAXIMUM_WEBRTC_NAMES];
	/* the other end's, and what the page was told of it */
	struct p2p_webrtc remote;
	struct p2p_candidate remote_candidates[P2P_MAXIMUM_CANDIDATES];
	int remote_candidate_count;
	int connect_sent;
	int candidates_sent;
};

/* web_library.js's: what the page is asked to do with a connection, and a
copy of the text or the packet it is given (freed by the page) */
void web_js_p2p(int command, int connection, unsigned char *data, int size);
/* web_net.c's */
void web_net_inject(unsigned int from_ip, unsigned short from_port, unsigned short to_port, const void *data,
	int size);

static struct connection connections[MAXIMUM_CONNECTIONS];
static pthread_mutex_t description_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned char fingerprint[P2P_FINGERPRINT_SIZE];
static volatile int has_fingerprint;
static int started;

static unsigned short network_short(unsigned short value)
{
	return (unsigned short)(value << 8 | value >> 8);
}

static void post(int command, int connection, const void *data, int size)
{
	unsigned char *copy = NULL;

	if (size > 0)
	{
		copy = malloc((size_t)size + 1);
		if (!copy)
			return;
		memcpy(copy, data, (size_t)size);
		copy[size] = 0;
	}
	web_js_p2p(command, connection, copy, size > 0 ? size : 0);
}

static void address_text(const struct p2p_candidate *candidate, char *text, int size)
{
	unsigned char bytes[4];

	memcpy(bytes, &candidate->address, 4);
	snprintf(text, (size_t)size, " %u.%u.%u.%u:%u", bytes[0], bytes[1], bytes[2], bytes[3],
		network_short(candidate->port));
}

/* ---------- connections */

static int connection_new(int role)
{
	int index;

	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		if (connections[index].role == _connection_unused)
		{
			pthread_mutex_lock(&description_lock);
			memset(&connections[index], 0, sizeof(connections[index]));
			connections[index].role = role;
			connections[index].peer = -1;
			pthread_mutex_unlock(&description_lock);
			post(_command_create, index, NULL, 0);
			return index;
		}
	}
	return -1;
}

static void connection_free(int index)
{
	post(_command_close, index, NULL, 0);
	pthread_mutex_lock(&description_lock);
	memset(&connections[index], 0, sizeof(connections[index]));
	pthread_mutex_unlock(&description_lock);
}

/* a spare is kept made */
static void keep_spare(void)
{
	int index;

	started = 1;
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		if (connections[index].role == _connection_spare)
			return;
	}
	connection_new(_connection_spare);
}

/* the spare (described, if one is), taken for role; or a new one */
static int take_spare(int role)
{
	int found = -1;
	int index;

	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		if (connections[index].role == _connection_spare && (found < 0 || connections[index].described))
			found = index;
	}
	if (found >= 0)
		connections[found].role = role;
	else
		found = connection_new(role);
	keep_spare();
	return found;
}

static int find_role(int role)
{
	int index;

	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		if (connections[index].role == role)
			return index;
	}
	return -1;
}

/* (under description_lock) this end's credentials and addresses */
static int copy_description(const struct connection *connection, struct p2p_webrtc *local,
	struct p2p_candidate *candidates, int maximum_count)
{
	int count = connection->candidate_count < maximum_count ? connection->candidate_count : maximum_count;

	if (!connection->described)
		return 0;
	strcpy(local->ufrag, connection->ufrag);
	strcpy(local->password, connection->password);
	local->name_count = connection->name_count;
	memcpy(local->names, connection->names, sizeof(local->names));
	memcpy(local->name_ports, connection->name_ports, sizeof(local->name_ports));
	memcpy(candidates, connection->candidates, sizeof(*candidates) * (size_t)count);
	return count;
}

/* the page is told the other end, once it has its credentials; and its
addresses that came since */
static void tell_page(int index)
{
	struct connection *connection = &connections[index];
	char text[TEXT_SIZE];
	int length;
	int candidate;

	if (!connection->remote.ufrag[0])
		return;
	if (!connection->connect_sent)
	{
		char hex[2 * P2P_FINGERPRINT_SIZE + 1];

		p2p_hex(connection->remote.fingerprint, P2P_FINGERPRINT_SIZE, hex);
		/* a native build: ICE-lite, the DTLS server; of two browsers, the
		host is the DTLS server */
		length = snprintf(text, sizeof(text), "%s %s %s %s %s",
			connection->remote.kind == _p2p_webrtc_native ? "lite" : "full",
			connection->remote.kind == _p2p_webrtc_native || connection->is_host ? "passive" : "active",
			connection->remote.ufrag, connection->remote.password, hex);
		for (candidate = 0; candidate < connection->remote_candidate_count; candidate++)
		{
			address_text(&connection->remote_candidates[candidate], text + length, (int)sizeof(text) - length);
			length += (int)strlen(text + length);
		}
		/* (a browser's own, by name: one on the same network finds it) */
		for (candidate = 0; candidate < connection->remote.name_count; candidate++)
		{
			snprintf(text + length, sizeof(text) - (size_t)length, " %s:%u", connection->remote.names[candidate],
				network_short(connection->remote.name_ports[candidate]));
			length += (int)strlen(text + length);
		}
		post(_command_connect, index, text, length);
		connection->connect_sent = 1;
		connection->candidates_sent = connection->remote_candidate_count;
		return;
	}
	if (connection->candidates_sent == connection->remote_candidate_count)
		return;
	length = 0;
	text[0] = 0;
	for (candidate = connection->candidates_sent; candidate < connection->remote_candidate_count; candidate++)
	{
		address_text(&connection->remote_candidates[candidate], text + length, (int)sizeof(text) - length);
		length += (int)strlen(text + length);
	}
	post(_command_candidates, index, text, length);
	connection->candidates_sent = connection->remote_candidate_count;
}

/* ---------- p2p.c's side (p2p_internal.h) */

int p2p_webrtc_ready(void)
{
	int index;

	if (!started)
		keep_spare();
	if (!has_fingerprint)
		return 0;
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		if (connections[index].role == _connection_spare && connections[index].described)
			return 1;
	}
	return 0;
}

int p2p_webrtc_describe(const unsigned char *identifier, int proven, struct p2p_webrtc *local,
	struct p2p_candidate *candidates, int maximum_count)
{
	char name[2 * P2P_IDENTIFIER_SIZE + 1];
	int found = -1;
	int count = 0;
	int index;

	memset(local, 0, sizeof(*local));
	if (!has_fingerprint)
		return 0;
	local->kind = _p2p_webrtc_browser;
	memcpy(local->fingerprint, fingerprint, P2P_FINGERPRINT_SIZE);
	p2p_hex(identifier, P2P_IDENTIFIER_SIZE, name);
	for (index = 0; index < MAXIMUM_CONNECTIONS && found < 0; index++)
	{
		if (connections[index].role == _connection_peer && !strcmp(connections[index].peer_name, name))
			found = index;
	}
	/* (no session yet: a request's own; a host answers a request not
	proven yet with none, as it makes no connection for one) */
	if (found < 0 && proven)
	{
		found = find_role(_connection_request);
		if (found < 0)
			found = take_spare(_connection_request);
	}
	if (found < 0)
		return 0;
	pthread_mutex_lock(&description_lock);
	count = copy_description(&connections[found], local, candidates, maximum_count);
	pthread_mutex_unlock(&description_lock);
	return count;
}

void p2p_webrtc_new_request(void)
{
	int index = find_role(_connection_request);

	if (index >= 0)
		connection_free(index);
}

int p2p_webrtc_offered(int index, int peer, const unsigned char *secret, const struct p2p_webrtc *remote,
	const struct p2p_candidate *candidates, int count, int is_host)
{
	struct connection *connection;
	int candidate;

	if (index < 0)
	{
		/* (a machine that has no WebRTC cannot be reached from a page) */
		if (remote->kind == _p2p_webrtc_none)
			return -1;
		/* the host's: the request's connection, which its credentials went
		with; a joiner's: the spare, whose went with the answer */
		index = is_host ? find_role(_connection_request) : -1;
		if (index >= 0)
			connections[index].role = _connection_peer;
		else
			index = take_spare(_connection_peer);
		if (index < 0)
			return -1;
		connection = &connections[index];
		connection->peer = peer;
		connection->is_host = is_host;
		p2p_peer_name(peer, connection->peer_name);
		connection->remote.kind = remote->kind;
		memcpy(connection->remote.fingerprint, remote->fingerprint, P2P_FINGERPRINT_SIZE);
		if (remote->kind == _p2p_webrtc_native)
			p2p_webrtc_credentials(secret, connection->remote.ufrag, connection->remote.password);
	}
	connection = &connections[index];
	/* (a browser host's credentials come once the joiner proved its
	request) */
	if (!connection->remote.ufrag[0] && remote->ufrag[0] && remote->kind == _p2p_webrtc_browser)
	{
		strcpy(connection->remote.ufrag, remote->ufrag);
		strcpy(connection->remote.password, remote->password);
		connection->remote.name_count = remote->name_count;
		memcpy(connection->remote.names, remote->names, sizeof(remote->names));
		memcpy(connection->remote.name_ports, remote->name_ports, sizeof(remote->name_ports));
	}
	for (candidate = 0; candidate < count; candidate++)
	{
		int known;

		/* (a native build's ICE is on its IPv4 tunnel alone) */
		if (candidates[candidate].ipv6)
			continue;
		for (known = 0; known < connection->remote_candidate_count; known++)
		{
			if (connection->remote_candidates[known].address == candidates[candidate].address &&
				connection->remote_candidates[known].port == candidates[candidate].port)
				break;
		}
		if (known == connection->remote_candidate_count && connection->remote_candidate_count < P2P_MAXIMUM_CANDIDATES)
			connection->remote_candidates[connection->remote_candidate_count++] = candidates[candidate];
	}
	tell_page(index);
	return index;
}

void p2p_webrtc_close(int index)
{
	if (index >= 0 && index < MAXIMUM_CONNECTIONS && connections[index].role != _connection_unused)
		connection_free(index);
}

void p2p_webrtc_send(int index, const void *packet, int size)
{
	if (index >= 0 && index < MAXIMUM_CONNECTIONS && connections[index].role == _connection_peer)
		post(_command_send, index, packet, size);
}

int p2p_webrtc_received(const unsigned char *packet, int size, unsigned long address, unsigned short port)
{
	(void)packet;
	(void)size;
	(void)address;
	(void)port;
	return 0;
}

int p2p_webrtc_update(void)
{
	return 0;
}

/* ---------- the page's (port/web/site/p2p.js), on its thread */

/* this run's certificate's SHA-256 (32 bytes), which every connection has */
EMSCRIPTEN_KEEPALIVE void web_p2p_fingerprint(const unsigned char *bytes)
{
	memcpy(fingerprint, bytes, P2P_FINGERPRINT_SIZE);
	has_fingerprint = 1;
}

/* whether text is ICE's (letters, digits, "+" and "/"), of a length from
minimum to maximum */
static int ice_text(const char *text, int minimum, int maximum)
{
	int length = (int)strlen(text);

	return length >= minimum && length <= maximum && (int)strspn(text,
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+/") == length;
}

/* a connection's credentials and IPv4 addresses, as its offer and gathering
gave them, and its own addresses' mDNS names: "ufrag password address:port
... name.local:port ..." */
EMSCRIPTEN_KEEPALIVE void web_p2p_described(int index, const char *text)
{
	char ufrag[P2P_ICE_UFRAG_SIZE], password[P2P_ICE_PASSWORD_SIZE];
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	char names[P2P_MAXIMUM_WEBRTC_NAMES][P2P_WEBRTC_NAME_SIZE];
	unsigned short name_ports[P2P_MAXIMUM_WEBRTC_NAMES];
	int count = 0;
	int name_count = 0;
	int offset = 0;

	if (index < 0 || index >= MAXIMUM_CONNECTIONS || !text ||
		sscanf(text, "%32s %64s%n", ufrag, password, &offset) != 2 || !ice_text(ufrag, 4, P2P_ICE_UFRAG_SIZE - 1) ||
		!ice_text(password, 22, P2P_ICE_PASSWORD_SIZE - 1))
	{
		return;
	}
	for (;;)
	{
		unsigned int a, b, c, d, port;
		char name[P2P_WEBRTC_NAME_SIZE];
		int used = 0;

		if (sscanf(text + offset, " %u.%u.%u.%u:%u%n", &a, &b, &c, &d, &port, &used) == 5)
		{
			offset += used;
			if (a > 255 || b > 255 || c > 255 || d > 255 || !port || port > 65535 || count == P2P_MAXIMUM_CANDIDATES)
				continue;
			p2p_candidate_ipv4(&candidates[count], a | b << 8 | c << 16 | d << 24,
				network_short((unsigned short)port));
			count++;
		}
		else if (sscanf(text + offset, " %63[a-zA-Z0-9.-]:%u%n", name, &port, &used) == 2)
		{
			int length = (int)strlen(name);

			offset += used;
			if (length <= 6 || strcmp(name + length - 6, ".local") || strchr(name, '.') != name + length - 6 ||
				!port || port > 65535 || name_count == P2P_MAXIMUM_WEBRTC_NAMES)
			{
				continue;
			}
			strcpy(names[name_count], name);
			name_ports[name_count++] = network_short((unsigned short)port);
		}
		else
			break;
	}
	pthread_mutex_lock(&description_lock);
	if (connections[index].role != _connection_unused)
	{
		strcpy(connections[index].ufrag, ufrag);
		strcpy(connections[index].password, password);
		memcpy(connections[index].candidates, candidates, sizeof(candidates));
		connections[index].candidate_count = count;
		memcpy(connections[index].names, names, sizeof(names));
		memcpy(connections[index].name_ports, name_ports, sizeof(name_ports));
		connections[index].name_count = name_count;
		connections[index].described = 1;
	}
	pthread_mutex_unlock(&description_lock);
}

/* a message the connection's data channel received (a copy the page frees):
to the tunnel's socket, from the connection's stand-in address */
EMSCRIPTEN_KEEPALIVE void web_p2p_receive(int index, const unsigned char *data, int size)
{
	unsigned int stand_in = 198u | 18u << 8 | (unsigned int)(index >> 8 & 255) << 16 | (unsigned int)(index & 255) << 24;

	if (index < 0 || index >= MAXIMUM_CONNECTIONS || !data || size <= 0)
		return;
	web_net_inject(stand_in, network_short(STAND_IN_PORT), p2p_tunnel_local_port(), data, size);
}

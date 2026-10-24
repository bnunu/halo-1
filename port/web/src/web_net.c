/*
WEB_NET.C

The sockets of port/linux/src/posix.h for the web build: a network inside
the page, and the browsers of a room as a local network.

Browsers have no UDP or plain TCP, but the game opens sockets even when it
plays alone: a split screen game is a network game whose host and clients
are the same machine, connected through Winsock
(transport_endpoint_winsock.c), and system link looks for games by
broadcast. Here every socket belongs to one machine with two addresses,
loopback and its address on the network (local_address: 10.0.0.1, or the one
the page gives it in a room). A datagram sent to either, or to a broadcast
address, goes to the sockets bound to its port; a stream socket connects to
the socket listening on its port, and the two exchange bytes through each
other's queue.

The other browsers of a room (port/web/site/net.js) have addresses of their
own in 10.0.0.0/8. What the game sends to one, or broadcasts, becomes a frame
the page carries over WebRTC: datagrams on a channel that may lose them,
streams' opening, bytes and closing on one that does not. The page hands what
arrives to web_net_receive. So system link finds and plays the room's games
as a local network's. Anything sent to an address nobody has is lost, as on
a network. A frame comes from another machine: it is checked before it is
used.

As with Winsock, each call returns -1 on failure with the error code in
posix_socket_last_error(); blocking calls wait on one condition, which every
change signals.

Internet play (port/linux/src/p2p_signal.c) reaches its MQTT brokers over
WebSockets here: a broker's address is its wss:// URL, which
posix_resolve_ipv4 gives a stand-in address in 198.19.0.0/24, and a stream
socket connected to that address is the page's WebSocket to the URL. Its
tunnel goes over WebRTC (web_p2p.c): what arrives is handed to its socket
here (web_net_inject).

The rest of posix.h's network half, which a browser has no use for, answers
"none" here: the command line's (but for what the page started the game
with), the desktop's link handler, Discord.
*/

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "posix.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

/* Winsock error codes (winerror.h) */
#define WSAEFAULT 10014
#define WSAEINVAL 10022
#define WSAEMFILE 10024
#define WSAEWOULDBLOCK 10035
#define WSAENOTSOCK 10038
#define WSAEMSGSIZE 10040
#define WSAENOPROTOOPT 10042
#define WSAEPROTONOSUPPORT 10043
#define WSAEAFNOSUPPORT 10047
#define WSAEADDRINUSE 10048
#define WSAEADDRNOTAVAIL 10049
#define WSAENETUNREACH 10051
#define WSAECONNRESET 10054
#define WSAENOBUFS 10055
#define WSAEISCONN 10056
#define WSAENOTCONN 10057
#define WSAESHUTDOWN 10058
#define WSAECONNREFUSED 10061

/* the Winsock and BSD values the game passes */
#define FAMILY_INET 2
#define TYPE_STREAM 1
#define TYPE_DATAGRAM 2
#define WINSOCK_SOL_SOCKET 0xffff
#define WINSOCK_SO_ERROR 0x1007
#define WINSOCK_SO_TYPE 0x1008
#define WINSOCK_SO_SNDBUF 0x1001
#define WINSOCK_SO_RCVBUF 0x1002
#define MESSAGE_PEEK 0x2

/* in network byte order: 127.0.0.1; 10.0.0.1, the machine's address on its
own network; the network's broadcast addresses */
#define LOOPBACK_ADDRESS 0x0100007fu
#define DEFAULT_LOCAL_ADDRESS 0x0100000au
#define NETWORK_BROADCAST 0xffffff0au
#define LIMITED_BROADCAST 0xffffffffu
#define IS_NETWORK(ip) (((ip) & 0xffu) == 10u)

/* descriptors apart from the file system's */
#define SOCKET_BASE 0x4000
#define MAXIMUM_SOCKETS 256
#define MAXIMUM_QUEUED_DATAGRAMS 256
#define MAXIMUM_DATAGRAM 65507
#define STREAM_CAPACITY (512 * 1024)
#define MAXIMUM_BACKLOG 16
#define FIRST_EPHEMERAL_PORT 49152

/* ---------- the frames between the browsers of a room

A frame's first byte is its type, with REMOTE_FRAME_OPENER set when its
sender is the end that opened the connection it is about; the rest, in
network byte order:
- a datagram: destination address (4), source port (2), destination port
  (2), then the datagram;
- a stream's opening: connection (4), source port (2), destination port (2);
- a stream's bytes: connection (4), then the bytes;
- a stream's end (closed, refused or lost): connection (4).
A connection is numbered by the end that opened it; with which end sent a
frame, that and the sender's address name it. */
enum
{
	_frame_datagram = 1,
	_frame_open,
	_frame_data,
	_frame_close,
};

#define REMOTE_FRAME_OPENER 0x80
#define REMOTE_DATAGRAM_HEADER 9
#define REMOTE_STREAM_HEADER 5
/* a stream's bytes go in frames of at most this much (WebRTC's messages are
best kept below 16 KB) */
#define REMOTE_STREAM_CHUNK 16000
/* the address of every browser of the room, for the page */
#define REMOTE_EVERYONE LIMITED_BROADCAST

/* web_library.js's (port/web/tests/web_net_test.c's for the tests): a
frame for the browser at address (REMOTE_EVERYONE: every one), on the
channel that does not lose it or the one that may; the frame is a copy
the page frees */
void web_js_net_send(unsigned int address, int reliable, unsigned char *frame, int size);
/* a WebSocket for a socket (its index, and its connection's number, which
the page's calls back name it by) to a URL; bytes for it (a copy the page
frees); and its closing */
void web_js_websocket_open(int index, unsigned int connection, const char *url);
void web_js_websocket_send(int index, unsigned int connection, unsigned char *bytes, int size);
void web_js_websocket_close(int index, unsigned int connection);

/* the brokers' URLs, by their stand-in addresses: 198.19.0.1 and on */
#define MAXIMUM_WEBSOCKET_URLS 8
#define WEBSOCKET_URL_SIZE 256
#define IS_WEBSOCKET_ADDRESS(ip) (((ip) & 0xffffffu) == (198u | 19u << 8))

/* sockaddr_in, as Winsock and BSD share it */
struct address
{
	unsigned short family;
	unsigned short port; /* network byte order */
	unsigned int ip; /* network byte order */
	unsigned char zero[8];
};

struct datagram
{
	struct datagram *next;
	struct address from;
	int length;
	unsigned char data[];
};

enum
{
	_socket_free = 0,
	_socket_open,
	_socket_listening,
	_socket_connected,
};

struct web_socket
{
	int state;
	int type;
	int nonblocking;
	int bound;
	struct address local;
	struct address remote;
	/* datagrams: a queue */
	struct datagram *first, *last;
	int datagram_count;
	/* streams: the other end here (an index, or -1 once it closed), the
	bytes received, and whether either side shut down */
	int peer;
	unsigned char *bytes;
	unsigned int read_position, byte_count;
	int receive_shut, send_shut;
	/* a stream to another browser (remote.ip): its connection's number,
	whether this end opened it, and whether the other end has gone */
	int is_remote;
	uint32_t connection;
	int opened_here;
	int remote_closed;
	/* a WebSocket (to remote.ip's URL), numbered by connection too, and
	whether it has opened */
	int is_websocket;
	int websocket_open;
	/* listening: the connections waiting for accept, as indices */
	int pending[MAXIMUM_BACKLOG];
	int pending_count, backlog;
};

static struct web_socket sockets[MAXIMUM_SOCKETS];
static pthread_mutex_t net_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t net_changed = PTHREAD_COND_INITIALIZER;
static __thread int last_error;
static unsigned short next_ephemeral_port = FIRST_EPHEMERAL_PORT;
static uint32_t next_connection = 1;
static unsigned int local_address_value;
static char websocket_urls[MAXIMUM_WEBSOCKET_URLS][WEBSOCKET_URL_SIZE];
static int websocket_url_count;

static unsigned short swap16(unsigned short value)
{
	return (unsigned short)((value >> 8) | (value << 8));
}

static int fail(int error)
{
	last_error = error;
	return -1;
}

static int succeed(int result)
{
	last_error = 0;
	return result;
}

/* a dotted quad, in network byte order; 0 if it is none */
static unsigned int parse_ipv4(const char *text)
{
	unsigned int parts[4];
	char extra;

	if (text && sscanf(text, "%u.%u.%u.%u%c", &parts[0], &parts[1], &parts[2], &parts[3], &extra) == 4 &&
		parts[0] < 256 && parts[1] < 256 && parts[2] < 256 && parts[3] < 256)
	{
		return parts[0] | (parts[1] << 8) | (parts[2] << 16) | (parts[3] << 24);
	}
	return 0;
}

/* the machine's address on the network: the room's (HALO_WEB_ADDRESS, which
the page sets: port/web/site/net.js), in 10.0.0.0/8, else 10.0.0.1 */
static unsigned int local_address(void)
{
	if (!local_address_value)
	{
		unsigned int address = parse_ipv4(getenv("HALO_WEB_ADDRESS"));

		local_address_value = IS_NETWORK(address) && address != NETWORK_BROADCAST ? address : DEFAULT_LOCAL_ADDRESS;
	}
	return local_address_value;
}

static int is_local(unsigned int ip)
{
	return ip == 0 || ip == LOOPBACK_ADDRESS || ip == local_address();
}

static int is_broadcast(unsigned int ip)
{
	return ip == LIMITED_BROADCAST || ip == NETWORK_BROADCAST;
}

/* another browser's address (whether one has it or not) */
static int is_remote(unsigned int ip)
{
	return IS_NETWORK(ip) && !is_local(ip) && !is_broadcast(ip);
}

/* (under net_lock) the open socket of a descriptor, or NULL */
static struct web_socket *socket_get(int descriptor)
{
	int index = descriptor - SOCKET_BASE;

	if (index < 0 || index >= MAXIMUM_SOCKETS || sockets[index].state == _socket_free)
		return NULL;
	return &sockets[index];
}

static int socket_index(const struct web_socket *socket)
{
	return (int)(socket - sockets);
}

static int port_in_use(int type, unsigned short port)
{
	int index;

	for (index = 0; index < MAXIMUM_SOCKETS; index++)
	{
		if (sockets[index].state != _socket_free && sockets[index].type == type && sockets[index].bound &&
			sockets[index].local.port == port)
		{
			return 1;
		}
	}
	return 0;
}

/* a free port (network byte order) */
static unsigned short ephemeral_port(int type)
{
	int tries;

	for (tries = 0; tries < 65536 - FIRST_EPHEMERAL_PORT; tries++)
	{
		unsigned short port = swap16(next_ephemeral_port);

		next_ephemeral_port = next_ephemeral_port == 65535 ? FIRST_EPHEMERAL_PORT : next_ephemeral_port + 1;
		if (!port_in_use(type, port))
			return port;
	}
	return 0;
}

static void bind_implicitly(struct web_socket *socket)
{
	if (socket->bound)
		return;
	memset(&socket->local, 0, sizeof(socket->local));
	socket->local.family = FAMILY_INET;
	socket->local.port = ephemeral_port(socket->type);
	socket->bound = 1;
}

static void datagrams_free(struct web_socket *socket)
{
	while (socket->first)
	{
		struct datagram *next = socket->first->next;

		free(socket->first);
		socket->first = next;
	}
	socket->last = NULL;
	socket->datagram_count = 0;
}

/* ---------- frames out */

static void put32(unsigned char *at, uint32_t value)
{
	at[0] = (unsigned char)(value >> 24);
	at[1] = (unsigned char)(value >> 16);
	at[2] = (unsigned char)(value >> 8);
	at[3] = (unsigned char)value;
}

static uint32_t get32(const unsigned char *at)
{
	return ((uint32_t)at[0] << 24) | ((uint32_t)at[1] << 16) | ((uint32_t)at[2] << 8) | at[3];
}

/* a frame of a header and data to the browser at address; the page's to
free. Nothing happens (the frame is lost, as a network may lose it) if
there is no memory for it */
static void remote_send(unsigned int address, int reliable, const unsigned char *header, int header_size,
	const void *data, int size)
{
	unsigned char *frame = malloc((size_t)(header_size + size));

	if (!frame)
		return;
	memcpy(frame, header, (size_t)header_size);
	if (size)
		memcpy(frame + header_size, data, (size_t)size);
	web_js_net_send(address, reliable, frame, header_size + size);
}

/* a stream frame about a remote stream socket */
static void remote_stream_frame(const struct web_socket *socket, int type, const void *data, int size)
{
	unsigned char header[REMOTE_STREAM_HEADER];

	header[0] = (unsigned char)(type | (socket->opened_here ? REMOTE_FRAME_OPENER : 0));
	put32(header + 1, socket->connection);
	remote_send(socket->remote.ip, 1, header, sizeof(header), data, size);
}

/* (under net_lock) frees a socket; its stream's other end sees it go */
static void socket_release(int index)
{
	struct web_socket *socket = &sockets[index];
	int pending;

	datagrams_free(socket);
	if (socket->is_websocket)
		web_js_websocket_close(index, socket->connection);
	if (socket->state == _socket_connected && socket->is_remote && !socket->remote_closed)
		remote_stream_frame(socket, _frame_close, NULL, 0);
	else if (socket->state == _socket_connected && socket->peer >= 0)
		sockets[socket->peer].peer = -1;
	for (pending = 0; pending < socket->pending_count; pending++)
		socket_release(socket->pending[pending]);
	free(socket->bytes);
	memset(socket, 0, sizeof(*socket));
}

/* ---------- sockets */

int posix_socket_last_error(void)
{
	return last_error;
}

/* (under net_lock) a free socket's index, made ready for its type, or -1 */
static int socket_new(int type)
{
	int index;

	for (index = 0; index < MAXIMUM_SOCKETS && sockets[index].state != _socket_free; index++)
	{
	}
	if (index == MAXIMUM_SOCKETS)
		return -1;
	memset(&sockets[index], 0, sizeof(sockets[index]));
	sockets[index].state = _socket_open;
	sockets[index].type = type;
	sockets[index].peer = -1;
	return index;
}

int posix_socket(int family, int type, int protocol)
{
	int index;

	(void)protocol;
	if (family != FAMILY_INET)
		return fail(WSAEAFNOSUPPORT);
	if (type != TYPE_STREAM && type != TYPE_DATAGRAM)
		return fail(WSAEPROTONOSUPPORT);
	pthread_mutex_lock(&net_lock);
	index = socket_new(type);
	pthread_mutex_unlock(&net_lock);
	return index < 0 ? fail(WSAEMFILE) : succeed(SOCKET_BASE + index);
}

int posix_socket_close(int descriptor)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAENOTSOCK);
	}
	socket_release(socket_index(socket));
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
	return succeed(0);
}

static int read_address(const void *address, int address_length, struct address *result)
{
	if (!address || address_length < (int)sizeof(*result) - 8)
		return 0;
	memset(result, 0, sizeof(*result));
	memcpy(result, address, address_length < (int)sizeof(*result) ? (size_t)address_length : sizeof(*result));
	return 1;
}

int posix_socket_bind(int descriptor, const void *address, int address_length)
{
	struct web_socket *socket;
	struct address wanted;

	if (!read_address(address, address_length, &wanted))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAENOTSOCK);
	}
	if (socket->bound)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAEINVAL);
	}
	if (!is_local(wanted.ip))
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAEADDRNOTAVAIL);
	}
	if (!wanted.port)
		wanted.port = ephemeral_port(socket->type);
	else if (port_in_use(socket->type, wanted.port))
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAEADDRINUSE);
	}
	wanted.family = FAMILY_INET;
	socket->local = wanted;
	socket->bound = 1;
	pthread_mutex_unlock(&net_lock);
	return succeed(0);
}

int posix_socket_listen(int descriptor, int backlog)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket || socket->type != TYPE_STREAM || socket->state == _socket_connected)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(socket ? WSAEINVAL : WSAENOTSOCK);
	}
	bind_implicitly(socket);
	socket->state = _socket_listening;
	socket->backlog = backlog < 1 ? 1 : backlog > MAXIMUM_BACKLOG ? MAXIMUM_BACKLOG : backlog;
	pthread_mutex_unlock(&net_lock);
	return succeed(0);
}

/* (under net_lock) the socket listening on a port, or -1 */
static int find_listener(unsigned short port)
{
	int index;

	for (index = 0; index < MAXIMUM_SOCKETS; index++)
	{
		if (sockets[index].state == _socket_listening && sockets[index].local.port == port)
			return index;
	}
	return -1;
}

/* (under net_lock) a new connected stream socket with its receive queue, or
-1 */
static int new_stream(void)
{
	int index = socket_new(TYPE_STREAM);

	if (index < 0)
		return -1;
	sockets[index].bytes = malloc(STREAM_CAPACITY);
	if (!sockets[index].bytes)
	{
		memset(&sockets[index], 0, sizeof(sockets[index]));
		return -1;
	}
	sockets[index].state = _socket_connected;
	sockets[index].bound = 1;
	return index;
}

/* (under net_lock) a stream to another browser: it is taken as connected at
once, as a connection that is refused is reset (its listener's browser
answers with the stream's end) */
static int connect_remote(struct web_socket *socket, const struct address *to)
{
	unsigned char header[REMOTE_STREAM_HEADER + 4];

	if (!(socket->bytes = malloc(STREAM_CAPACITY)))
		return fail(WSAENOBUFS);
	socket->state = _socket_connected;
	socket->is_remote = 1;
	socket->opened_here = 1;
	socket->connection = next_connection++;
	socket->remote = *to;
	socket->remote.family = FAMILY_INET;
	socket->local.ip = local_address();
	header[0] = _frame_open | REMOTE_FRAME_OPENER;
	put32(header + 1, socket->connection);
	memcpy(header + 5, &socket->local.port, 2);
	memcpy(header + 7, &to->port, 2);
	remote_send(to->ip, 1, header, sizeof(header), NULL, 0);
	return succeed(0);
}

static int wait_changed(const struct timespec *deadline);

/* (under net_lock) a stream to a broker's URL: connecting until the page's
WebSocket opens, which makes it writeable (or fails, which ends it) */
static int connect_websocket(struct web_socket *socket, const struct address *to)
{
	unsigned int number = to->ip >> 24;

	if (number < 1 || number > (unsigned int)websocket_url_count)
		return fail(WSAENETUNREACH);
	if (!(socket->bytes = malloc(STREAM_CAPACITY)))
		return fail(WSAENOBUFS);
	socket->state = _socket_connected;
	socket->is_websocket = 1;
	socket->connection = next_connection++;
	socket->remote = *to;
	socket->remote.family = FAMILY_INET;
	socket->local.ip = local_address();
	web_js_websocket_open(socket_index(socket), socket->connection, websocket_urls[number - 1]);
	return fail(WSAEWOULDBLOCK);
}

int posix_socket_connect(int descriptor, const void *address, int address_length)
{
	struct web_socket *socket;
	struct address to;
	int listener, accepted, result;

	if (!read_address(address, address_length, &to))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAENOTSOCK);
	}
	bind_implicitly(socket);
	if (socket->type == TYPE_DATAGRAM)
	{
		/* a datagram socket's default destination */
		socket->remote = to;
		pthread_mutex_unlock(&net_lock);
		return succeed(0);
	}
	if (socket->state == _socket_connected)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAEISCONN);
	}
	if (is_remote(to.ip))
	{
		result = connect_remote(socket, &to);
		pthread_mutex_unlock(&net_lock);
		return result;
	}
	if (IS_WEBSOCKET_ADDRESS(to.ip))
	{
		result = connect_websocket(socket, &to);
		/* (a blocking socket waits for it to open, or fail) */
		while (!socket->nonblocking && socket->state == _socket_connected && !socket->websocket_open &&
			!socket->remote_closed)
		{
			wait_changed(NULL);
		}
		if (!socket->nonblocking)
			result = socket->websocket_open ? succeed(0) : fail(WSAECONNREFUSED);
		pthread_mutex_unlock(&net_lock);
		return result;
	}
	listener = is_local(to.ip) ? find_listener(to.port) : -1;
	if (listener < 0)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(is_local(to.ip) ? WSAECONNREFUSED : WSAENETUNREACH);
	}
	if (sockets[listener].pending_count >= sockets[listener].backlog ||
		!(socket->bytes = malloc(STREAM_CAPACITY)) || (accepted = new_stream()) < 0)
	{
		free(socket->bytes);
		socket->bytes = NULL;
		pthread_mutex_unlock(&net_lock);
		return fail(WSAECONNREFUSED);
	}
	/* the two ends, each the other's peer */
	socket->state = _socket_connected;
	socket->remote = to;
	socket->remote.family = FAMILY_INET;
	if (!socket->remote.ip)
		socket->remote.ip = LOOPBACK_ADDRESS;
	socket->peer = accepted;
	if (!socket->local.ip)
		socket->local.ip = socket->remote.ip;
	sockets[accepted].local = socket->remote;
	sockets[accepted].remote = socket->local;
	sockets[accepted].peer = socket_index(socket);
	sockets[listener].pending[sockets[listener].pending_count++] = accepted;
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
	return succeed(0);
}

/* (under net_lock) waits for any change, up to the deadline (NULL: none);
0 once it has passed */
static int wait_changed(const struct timespec *deadline)
{
	if (!deadline)
	{
		pthread_cond_wait(&net_changed, &net_lock);
		return 1;
	}
	return pthread_cond_timedwait(&net_changed, &net_lock, deadline) == 0;
}

static void copy_address(const struct address *source, void *address, int *address_length)
{
	if (address && address_length && *address_length > 0)
	{
		memcpy(address, source, *address_length < (int)sizeof(*source) ? (size_t)*address_length : sizeof(*source));
		*address_length = (int)sizeof(*source);
	}
}

int posix_socket_accept(int descriptor, void *address, int *address_length)
{
	struct web_socket *socket;
	int accepted;

	pthread_mutex_lock(&net_lock);
	for (;;)
	{
		socket = socket_get(descriptor);
		if (!socket || socket->state != _socket_listening)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(socket ? WSAEINVAL : WSAENOTSOCK);
		}
		if (socket->pending_count)
			break;
		if (socket->nonblocking)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAEWOULDBLOCK);
		}
		wait_changed(NULL);
	}
	accepted = socket->pending[0];
	memmove(socket->pending, socket->pending + 1, sizeof(socket->pending[0]) * (size_t)(socket->pending_count - 1));
	socket->pending_count--;
	copy_address(&sockets[accepted].remote, address, address_length);
	pthread_mutex_unlock(&net_lock);
	return succeed(SOCKET_BASE + accepted);
}

/* (under net_lock) a datagram from an address to the sockets bound to its
destination's port here (all of them for a broadcast); 0, or a Winsock
error */
static int deliver_local_datagram(const struct address *from, const struct address *to, const void *buffer, int length)
{
	int index;

	for (index = 0; index < MAXIMUM_SOCKETS; index++)
	{
		struct web_socket *socket = &sockets[index];
		struct datagram *datagram;

		if (socket->state == _socket_free || socket->type != TYPE_DATAGRAM || !socket->bound ||
			socket->local.port != to->port)
		{
			continue;
		}
		/* bound to an address of its own: only what is sent to it (or
		broadcast) */
		if (socket->local.ip && !is_broadcast(to->ip) && to->ip && socket->local.ip != to->ip)
			continue;
		/* a full queue drops it, as a full receive buffer does */
		if (socket->datagram_count >= MAXIMUM_QUEUED_DATAGRAMS)
			continue;
		datagram = malloc(sizeof(*datagram) + (size_t)length);
		if (!datagram)
			return WSAENOBUFS;
		datagram->next = NULL;
		datagram->from = *from;
		datagram->length = length;
		memcpy(datagram->data, buffer, (size_t)length);
		if (socket->last)
			socket->last->next = datagram;
		else
			socket->first = datagram;
		socket->last = datagram;
		socket->datagram_count++;
	}
	pthread_cond_broadcast(&net_changed);
	return 0;
}

/* (under net_lock) a datagram a socket here sends: to the sockets here it
is for, and to the other browsers it is for; 0, or a Winsock error */
static int deliver_datagram(const struct web_socket *from, const void *buffer, int length, const struct address *to)
{
	struct address source;

	if (length < 0 || length > MAXIMUM_DATAGRAM)
		return WSAEMSGSIZE;
	if (is_broadcast(to->ip) || is_remote(to->ip))
	{
		unsigned char header[REMOTE_DATAGRAM_HEADER];

		header[0] = _frame_datagram;
		memcpy(header + 1, &to->ip, 4);
		memcpy(header + 5, &from->local.port, 2);
		memcpy(header + 7, &to->port, 2);
		remote_send(is_broadcast(to->ip) ? REMOTE_EVERYONE : to->ip, 0, header, sizeof(header), buffer, length);
		if (!is_broadcast(to->ip))
			return 0;
	}
	if (!is_local(to->ip) && !is_broadcast(to->ip))
		return 0;
	memset(&source, 0, sizeof(source));
	source.family = FAMILY_INET;
	source.port = from->local.port;
	source.ip = from->local.ip ? from->local.ip : to->ip == LOOPBACK_ADDRESS ? LOOPBACK_ADDRESS : local_address();
	return deliver_local_datagram(&source, to, buffer, length);
}

/* (under net_lock) bytes onto the other end's queue; the count sent, or -1
with the error */
static int stream_send(struct web_socket *socket, const unsigned char *buffer, int length)
{
	struct web_socket *peer;
	unsigned int sent = 0;

	if (socket->is_websocket)
	{
		unsigned char *copy;

		if (socket->send_shut)
			return fail(WSAESHUTDOWN);
		if (socket->remote_closed)
			return fail(WSAECONNRESET);
		if (!socket->websocket_open)
			return fail(WSAEWOULDBLOCK);
		if (!(copy = malloc((size_t)length + 1)))
			return fail(WSAENOBUFS);
		memcpy(copy, buffer, (size_t)length);
		web_js_websocket_send(socket_index(socket), socket->connection, copy, length);
		return succeed(length);
	}
	if (socket->is_remote)
	{
		int offset;

		if (socket->send_shut)
			return fail(WSAESHUTDOWN);
		if (socket->remote_closed)
			return fail(WSAECONNRESET);
		for (offset = 0; offset < length; offset += REMOTE_STREAM_CHUNK)
		{
			int chunk = length - offset < REMOTE_STREAM_CHUNK ? length - offset : REMOTE_STREAM_CHUNK;

			remote_stream_frame(socket, _frame_data, buffer + offset, chunk);
		}
		return succeed(length);
	}
	for (;;)
	{
		if (socket->send_shut)
			return fail(WSAESHUTDOWN);
		if (socket->peer < 0)
			return fail(WSAECONNRESET);
		peer = &sockets[socket->peer];
		if (peer->receive_shut)
			return fail(WSAECONNRESET);
		if (peer->byte_count < STREAM_CAPACITY)
			break;
		if (socket->nonblocking)
			return fail(WSAEWOULDBLOCK);
		wait_changed(NULL);
	}
	while (sent < (unsigned int)length && peer->byte_count < STREAM_CAPACITY)
	{
		unsigned int position = (peer->read_position + peer->byte_count) % STREAM_CAPACITY;
		unsigned int chunk = STREAM_CAPACITY - position;

		if (chunk > STREAM_CAPACITY - peer->byte_count)
			chunk = STREAM_CAPACITY - peer->byte_count;
		if (chunk > (unsigned int)length - sent)
			chunk = (unsigned int)length - sent;
		memcpy(peer->bytes + position, buffer + sent, chunk);
		peer->byte_count += chunk;
		sent += chunk;
	}
	pthread_cond_broadcast(&net_changed);
	return succeed((int)sent);
}

int posix_socket_send(int descriptor, const void *buffer, int length, int flags)
{
	struct web_socket *socket;
	int result;

	(void)flags;
	if (length < 0 || (length && !buffer))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
		result = fail(WSAENOTSOCK);
	else if (socket->type == TYPE_STREAM)
		result = socket->state == _socket_connected ? stream_send(socket, buffer, length) : fail(WSAENOTCONN);
	else if (!socket->remote.port)
		result = fail(WSAENOTCONN);
	else
	{
		int error = deliver_datagram(socket, buffer, length, &socket->remote);

		result = error ? fail(error) : succeed(length);
	}
	pthread_mutex_unlock(&net_lock);
	return result;
}

int posix_socket_sendto(int descriptor, const void *buffer, int length, int flags,
	const void *address, int address_length)
{
	struct web_socket *socket;
	struct address to;
	int result;

	if (!read_address(address, address_length, &to))
		return posix_socket_send(descriptor, buffer, length, flags);
	if (length < 0 || (length && !buffer))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
		result = fail(WSAENOTSOCK);
	else if (socket->type == TYPE_STREAM)
		result = socket->state == _socket_connected ? stream_send(socket, buffer, length) : fail(WSAENOTCONN);
	else
	{
		int error;

		bind_implicitly(socket);
		error = deliver_datagram(socket, buffer, length, &to);
		result = error ? fail(error) : succeed(length);
	}
	pthread_mutex_unlock(&net_lock);
	return result;
}

/* (under net_lock) whether a read would not wait */
static int readable(const struct web_socket *socket)
{
	if (socket->type == TYPE_DATAGRAM)
		return socket->first != NULL;
	if (socket->state == _socket_listening)
		return socket->pending_count > 0;
	if (socket->state != _socket_connected)
		return 0;
	/* (data, or the end: the other end closed or shut down its sending) */
	if (socket->byte_count > 0 || socket->receive_shut)
		return 1;
	return socket->is_remote || socket->is_websocket ? socket->remote_closed :
		socket->peer < 0 || sockets[socket->peer].send_shut;
}

static int receive(int descriptor, void *buffer, int length, int flags, void *address, int *address_length)
{
	struct web_socket *socket;
	int peek = (flags & MESSAGE_PEEK) != 0;

	if (length < 0 || (length && !buffer))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	for (;;)
	{
		socket = socket_get(descriptor);
		if (!socket)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAENOTSOCK);
		}
		if (socket->type == TYPE_STREAM && socket->state != _socket_connected)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAENOTCONN);
		}
		if (readable(socket))
			break;
		if (socket->nonblocking)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAEWOULDBLOCK);
		}
		if (socket->type == TYPE_DATAGRAM)
			bind_implicitly(socket);
		wait_changed(NULL);
	}
	if (socket->type == TYPE_DATAGRAM)
	{
		struct datagram *datagram = socket->first;
		int copied = datagram->length < length ? datagram->length : length;
		int truncated = datagram->length > length;

		memcpy(buffer, datagram->data, (size_t)copied);
		copy_address(&datagram->from, address, address_length);
		if (!peek)
		{
			socket->first = datagram->next;
			if (!socket->first)
				socket->last = NULL;
			socket->datagram_count--;
			free(datagram);
		}
		pthread_mutex_unlock(&net_lock);
		/* (as Winsock: the datagram's start, and WSAEMSGSIZE) */
		return truncated ? fail(WSAEMSGSIZE) : succeed(copied);
	}
	else
	{
		unsigned int count = socket->byte_count < (unsigned int)length ? socket->byte_count : (unsigned int)length;
		unsigned int index;

		/* (nothing left and the other end gone, or either side shut down:
		the end of the stream, 0) */
		for (index = 0; index < count; index++)
			((unsigned char *)buffer)[index] = socket->bytes[(socket->read_position + index) % STREAM_CAPACITY];
		if (!peek)
		{
			socket->read_position = (socket->read_position + count) % STREAM_CAPACITY;
			socket->byte_count -= count;
			pthread_cond_broadcast(&net_changed);
		}
		copy_address(&socket->remote, address, address_length);
		pthread_mutex_unlock(&net_lock);
		return succeed((int)count);
	}
}

int posix_socket_recv(int descriptor, void *buffer, int length, int flags)
{
	return receive(descriptor, buffer, length, flags, NULL, NULL);
}

int posix_socket_recvfrom(int descriptor, void *buffer, int length, int flags,
	void *address, int *address_length)
{
	return receive(descriptor, buffer, length, flags, address, address_length);
}

int posix_socket_shutdown(int descriptor, int how)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (!socket)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAENOTSOCK);
	}
	if (socket->type == TYPE_STREAM && socket->state != _socket_connected)
	{
		pthread_mutex_unlock(&net_lock);
		return fail(WSAENOTCONN);
	}
	/* 0 receiving, 1 sending, 2 both (SD_RECEIVE, SD_SEND, SD_BOTH) */
	if (how == 0 || how == 2)
		socket->receive_shut = 1;
	if ((how == 1 || how == 2) && !socket->send_shut)
	{
		socket->send_shut = 1;
		/* (another browser's end sees the stream end: it reads to the end
		of the bytes, and then 0) */
		if (socket->is_remote && !socket->remote_closed)
			remote_stream_frame(socket, _frame_close, NULL, 0);
	}
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
	return succeed(0);
}

int posix_socket_set_nonblocking(int descriptor, int nonblocking)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (socket)
		socket->nonblocking = nonblocking != 0;
	pthread_mutex_unlock(&net_lock);
	return socket ? succeed(0) : fail(WSAENOTSOCK);
}

int posix_socket_set_nodelay(int descriptor)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	pthread_mutex_unlock(&net_lock);
	return socket ? succeed(0) : fail(WSAENOTSOCK);
}

int posix_socket_bytes_available(int descriptor, posix_ulong *count)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (socket)
	{
		if (socket->type == TYPE_DATAGRAM)
			*count = socket->first ? (posix_ulong)socket->first->length : 0;
		else
			*count = socket->byte_count;
	}
	pthread_mutex_unlock(&net_lock);
	return socket ? succeed(0) : fail(WSAENOTSOCK);
}

int posix_socket_setsockopt(int descriptor, int level, int name, const void *value, int length)
{
	struct web_socket *socket;

	(void)level;
	(void)name;
	(void)value;
	(void)length;
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	pthread_mutex_unlock(&net_lock);
	/* (broadcasts, reuse, buffer sizes: nothing to set in the page) */
	return socket ? succeed(0) : fail(WSAENOTSOCK);
}

int posix_socket_getsockopt(int descriptor, int level, int name, void *value, int *length)
{
	struct web_socket *socket;
	int answer;

	if (!value || !length || *length < (int)sizeof(int))
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	answer = socket ? socket->type : 0;
	pthread_mutex_unlock(&net_lock);
	if (!socket)
		return fail(WSAENOTSOCK);
	if (level != WINSOCK_SOL_SOCKET)
		return fail(WSAENOPROTOOPT);
	switch (name)
	{
	case WINSOCK_SO_ERROR: answer = 0; break;
	case WINSOCK_SO_TYPE: break;
	case WINSOCK_SO_SNDBUF:
	case WINSOCK_SO_RCVBUF: answer = STREAM_CAPACITY; break;
	default: return fail(WSAENOPROTOOPT);
	}
	memcpy(value, &answer, sizeof(answer));
	*length = (int)sizeof(answer);
	return succeed(0);
}

int posix_socket_getsockname(int descriptor, void *address, int *address_length)
{
	struct web_socket *socket;
	struct address local;

	if (!address || !address_length)
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (socket)
	{
		if (!socket->bound)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAEINVAL);
		}
		local = socket->local;
	}
	pthread_mutex_unlock(&net_lock);
	if (!socket)
		return fail(WSAENOTSOCK);
	copy_address(&local, address, address_length);
	return succeed(0);
}

int posix_socket_getpeername(int descriptor, void *address, int *address_length)
{
	struct web_socket *socket;
	struct address remote;

	if (!address || !address_length)
		return fail(WSAEFAULT);
	pthread_mutex_lock(&net_lock);
	socket = socket_get(descriptor);
	if (socket)
		remote = socket->remote;
	pthread_mutex_unlock(&net_lock);
	if (!socket)
		return fail(WSAENOTSOCK);
	if (!remote.port)
		return fail(WSAENOTCONN);
	copy_address(&remote, address, address_length);
	return succeed(0);
}

/* (under net_lock) whether a write would not wait */
static int writeable(const struct web_socket *socket)
{
	if (socket->type == TYPE_DATAGRAM)
		return 1;
	if (socket->state != _socket_connected)
		return 0;
	/* (a WebSocket once it opened, or failed: as a connection's end) */
	if (socket->is_websocket)
		return socket->websocket_open || socket->remote_closed;
	if (socket->is_remote)
		return 1;
	return socket->peer < 0 || sockets[socket->peer].byte_count < STREAM_CAPACITY;
}

/* (under net_lock) keeps the ready descriptors of a list; how many; -1 if
one is not a socket */
static int keep_ready(int *descriptors, int *count, int (*ready)(const struct web_socket *), int keep)
{
	int index, kept = 0;

	if (!descriptors || !count)
		return 0;
	for (index = 0; index < *count; index++)
	{
		struct web_socket *socket = socket_get(descriptors[index]);

		if (!socket)
			return -1;
		if (ready && ready(socket))
		{
			if (keep)
				descriptors[kept] = descriptors[index];
			kept++;
		}
	}
	if (keep)
		*count = kept;
	return kept;
}

int posix_socket_select(int *read, int *read_count, int *write, int *write_count,
	int *error, int *error_count, posix_long timeout_seconds, posix_long timeout_microseconds, int infinite)
{
	struct timespec deadline;
	int result;

	if (!infinite)
	{
		clock_gettime(CLOCK_REALTIME, &deadline);
		deadline.tv_sec += timeout_seconds + timeout_microseconds / 1000000;
		deadline.tv_nsec += (long)(timeout_microseconds % 1000000) * 1000L;
		if (deadline.tv_nsec >= 1000000000L)
		{
			deadline.tv_sec++;
			deadline.tv_nsec -= 1000000000L;
		}
	}
	pthread_mutex_lock(&net_lock);
	for (;;)
	{
		int read_ready = keep_ready(read, read_count, readable, 0);
		int write_ready = keep_ready(write, write_count, writeable, 0);
		int error_ready = keep_ready(error, error_count, NULL, 0);

		if (read_ready < 0 || write_ready < 0 || error_ready < 0)
		{
			pthread_mutex_unlock(&net_lock);
			return fail(WSAENOTSOCK);
		}
		result = read_ready + write_ready;
		if (result || (!infinite && !wait_changed(&deadline)))
			break;
		if (infinite)
			wait_changed(NULL);
	}
	keep_ready(read, read_count, readable, 1);
	keep_ready(write, write_count, writeable, 1);
	keep_ready(error, error_count, NULL, 1);
	pthread_mutex_unlock(&net_lock);
	/* (as Winsock: nothing ready leaves the last error as it was) */
	if (result > 0)
		last_error = 0;
	return result;
}

/* ---------- frames in, from the other browsers of the room */

/* (under net_lock) the remote stream socket a frame is about: its sender's
address, the connection's number and which end the sender is */
static struct web_socket *remote_stream(unsigned int from, uint32_t connection, int sender_opened)
{
	int index;

	for (index = 0; index < MAXIMUM_SOCKETS; index++)
	{
		struct web_socket *socket = &sockets[index];

		if (socket->state == _socket_connected && socket->is_remote && socket->remote.ip == from &&
			socket->connection == connection && socket->opened_here == !sender_opened)
		{
			return socket;
		}
	}
	return NULL;
}

/* (under net_lock) another browser opens a stream to a port here: to the
socket listening there, or it ends at once */
static void remote_open(unsigned int from, uint32_t connection, unsigned short source_port,
	unsigned short destination_port)
{
	int listener = find_listener(destination_port);
	int accepted;

	if (remote_stream(from, connection, 1))
		return;
	if (listener < 0 || sockets[listener].pending_count >= sockets[listener].backlog || (accepted = new_stream()) < 0)
	{
		unsigned char header[REMOTE_STREAM_HEADER];

		header[0] = _frame_close;
		put32(header + 1, connection);
		remote_send(from, 1, header, sizeof(header), NULL, 0);
		return;
	}
	sockets[accepted].is_remote = 1;
	sockets[accepted].connection = connection;
	sockets[accepted].local.family = FAMILY_INET;
	sockets[accepted].local.ip = local_address();
	sockets[accepted].local.port = destination_port;
	sockets[accepted].remote.family = FAMILY_INET;
	sockets[accepted].remote.ip = from;
	sockets[accepted].remote.port = source_port;
	sockets[listener].pending[sockets[listener].pending_count++] = accepted;
}

/* (under net_lock) bytes another browser sent on a stream: onto its queue;
more than the queue holds ends the stream (the other end does not wait for
room, so the bytes beyond would be lost) */
static void remote_data(struct web_socket *socket, const unsigned char *data, unsigned int size)
{
	unsigned int index;

	if (socket->remote_closed || socket->receive_shut)
		return;
	if (size > STREAM_CAPACITY - socket->byte_count)
	{
		socket->remote_closed = 1;
		remote_stream_frame(socket, _frame_close, NULL, 0);
		return;
	}
	for (index = 0; index < size; index++)
		socket->bytes[(socket->read_position + socket->byte_count + index) % STREAM_CAPACITY] = data[index];
	socket->byte_count += size;
}

/* a frame another browser (at from, the address the page knows it by) sent;
the page's thread calls it (net.js), with a copy it frees. Every field is
checked: it comes from another machine */
EMSCRIPTEN_KEEPALIVE void web_net_receive(unsigned int from, int reliable, const unsigned char *frame, int size)
{
	int type;

	if (!frame || size < 1 || !is_remote(from))
		return;
	type = frame[0] & ~REMOTE_FRAME_OPENER;
	pthread_mutex_lock(&net_lock);
	if (type == _frame_datagram && size >= REMOTE_DATAGRAM_HEADER && !(frame[0] & REMOTE_FRAME_OPENER))
	{
		struct address source, destination;

		memset(&source, 0, sizeof(source));
		memset(&destination, 0, sizeof(destination));
		source.family = destination.family = FAMILY_INET;
		source.ip = from;
		memcpy(&destination.ip, frame + 1, 4);
		memcpy(&source.port, frame + 5, 2);
		memcpy(&destination.port, frame + 7, 2);
		if ((destination.ip == local_address() || is_broadcast(destination.ip)) && source.port && destination.port &&
			size - REMOTE_DATAGRAM_HEADER <= MAXIMUM_DATAGRAM)
		{
			deliver_local_datagram(&source, &destination, frame + REMOTE_DATAGRAM_HEADER, size - REMOTE_DATAGRAM_HEADER);
		}
	}
	else if (reliable && size >= REMOTE_STREAM_HEADER && type >= _frame_open && type <= _frame_close)
	{
		uint32_t connection = get32(frame + 1);
		int sender_opened = (frame[0] & REMOTE_FRAME_OPENER) != 0;

		if (type == _frame_open)
		{
			unsigned short source_port, destination_port;

			if (size == REMOTE_STREAM_HEADER + 4 && sender_opened)
			{
				memcpy(&source_port, frame + 5, 2);
				memcpy(&destination_port, frame + 7, 2);
				if (source_port && destination_port)
					remote_open(from, connection, source_port, destination_port);
			}
		}
		else
		{
			struct web_socket *socket = remote_stream(from, connection, sender_opened);

			if (socket && type == _frame_data)
				remote_data(socket, frame + REMOTE_STREAM_HEADER, (unsigned int)(size - REMOTE_STREAM_HEADER));
			else if (socket)
				socket->remote_closed = 1;
		}
	}
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
}

/* the page lost the browser at address (it left the room, or its
connection failed): its streams end */
EMSCRIPTEN_KEEPALIVE void web_net_peer_lost(unsigned int address)
{
	int index;

	pthread_mutex_lock(&net_lock);
	for (index = 0; index < MAXIMUM_SOCKETS; index++)
	{
		if (sockets[index].state == _socket_connected && sockets[index].is_remote && sockets[index].remote.ip == address)
			sockets[index].remote_closed = 1;
	}
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
}

/* ---------- WebSockets, from the page (web_library.js) */

/* (under net_lock) a WebSocket socket by the page's names for it, or NULL
(closed since: the socket may be another's now) */
static struct web_socket *websocket_of(int index, unsigned int connection)
{
	if (index < 0 || index >= MAXIMUM_SOCKETS || !sockets[index].is_websocket ||
		sockets[index].connection != connection)
	{
		return NULL;
	}
	return &sockets[index];
}

EMSCRIPTEN_KEEPALIVE void web_net_websocket_opened(int index, unsigned int connection)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = websocket_of(index, connection);
	if (socket)
		socket->websocket_open = 1;
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
}

/* bytes that arrived (a copy the page frees); more than the queue holds
ends the stream, as for another browser's */
EMSCRIPTEN_KEEPALIVE void web_net_websocket_data(int index, unsigned int connection, const unsigned char *data,
	int size)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = websocket_of(index, connection);
	if (socket && size > 0 && !socket->remote_closed && !socket->receive_shut)
	{
		if ((unsigned int)size > STREAM_CAPACITY - socket->byte_count)
			socket->remote_closed = 1;
		else
		{
			int offset;

			for (offset = 0; offset < size; offset++)
				socket->bytes[(socket->read_position + socket->byte_count + offset) % STREAM_CAPACITY] = data[offset];
			socket->byte_count += (unsigned int)size;
		}
	}
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
}

EMSCRIPTEN_KEEPALIVE void web_net_websocket_closed(int index, unsigned int connection)
{
	struct web_socket *socket;

	pthread_mutex_lock(&net_lock);
	socket = websocket_of(index, connection);
	if (socket)
		socket->remote_closed = 1;
	pthread_cond_broadcast(&net_changed);
	pthread_mutex_unlock(&net_lock);
}

/* ---------- internet play's tunnel (web_p2p.c) */

void web_net_inject(unsigned int from_ip, unsigned short from_port, unsigned short to_port, const void *data, int size)
{
	struct address source, destination;

	if (size <= 0 || size > MAXIMUM_DATAGRAM || !to_port)
		return;
	memset(&source, 0, sizeof(source));
	memset(&destination, 0, sizeof(destination));
	source.family = destination.family = FAMILY_INET;
	source.ip = from_ip;
	source.port = from_port;
	destination.ip = local_address();
	destination.port = to_port;
	pthread_mutex_lock(&net_lock);
	deliver_local_datagram(&source, &destination, data, size);
	pthread_mutex_unlock(&net_lock);
}

/* ---------- addresses */

posix_ulong posix_local_ipv4_address(void)
{
	return local_address();
}

posix_ulong posix_resolve_ipv4(const char *host)
{
	int index;

	/* a broker's WebSocket URL: its stand-in address (connect_websocket) */
	if (host && (!strncmp(host, "ws://", 5) || !strncmp(host, "wss://", 6)) && strlen(host) < WEBSOCKET_URL_SIZE)
	{
		pthread_mutex_lock(&net_lock);
		for (index = 0; index < websocket_url_count && strcmp(websocket_urls[index], host); index++)
			;
		if (index == websocket_url_count && websocket_url_count < MAXIMUM_WEBSOCKET_URLS)
			strcpy(websocket_urls[websocket_url_count++], host);
		pthread_mutex_unlock(&net_lock);
		return index < websocket_url_count ? 198u | 19u << 8 | (unsigned int)(index + 1) << 24 : 0;
	}
	/* otherwise dotted quads only: a page cannot look names up */
	return parse_ipv4(host);
}

/* ---------- IPv6: none in a page (its WebRTC connections carry its
tunnel, web_p2p.c) */

int posix_socket_udp6(unsigned short port, unsigned short *bound_port)
{
	(void)port;
	(void)bound_port;
	return fail(WSAEAFNOSUPPORT);
}

int posix_socket_sendto6(int socket, const void *buffer, int length, const unsigned char *address,
	unsigned short port)
{
	(void)socket;
	(void)buffer;
	(void)length;
	(void)address;
	(void)port;
	return fail(WSAEAFNOSUPPORT);
}

int posix_socket_recvfrom6(int socket, void *buffer, int length, unsigned char *address, unsigned short *port)
{
	(void)socket;
	(void)buffer;
	(void)length;
	(void)address;
	(void)port;
	return fail(WSAEAFNOSUPPORT);
}

int posix_local_ipv6_address(unsigned char *address)
{
	(void)address;
	return 0;
}

void posix_random_bytes(void *buffer, posix_ulong size)
{
	unsigned char *cursor = buffer;

	/* (the browser's crypto.getRandomValues, 256 bytes at a time) */
	while (size)
	{
		size_t chunk = size > 256 ? 256 : size;

		if (getentropy(cursor, chunk) != 0)
			abort();
		cursor += chunk;
		size -= chunk;
	}
}

/* ---------- the process and the desktop: none in a page */

static int argument_count;
static char **arguments;

/* the arguments the page started the game with (web_main.c): an invite
link among them is joined (p2p.c) */
void web_net_arguments(int count, char **values)
{
	argument_count = count;
	arguments = values;
}

int posix_command_line_argument(int index, char *buffer, posix_ulong size)
{
	if (!buffer || !size)
		return 0;
	buffer[0] = 0;
	if (index < 0 || index >= argument_count || !arguments[index])
		return 0;
	snprintf(buffer, size, "%s", arguments[index]);
	return 1;
}

posix_ulong posix_process_id(void)
{
	return 1;
}

int posix_register_url_scheme(const char *scheme, const char *description)
{
	(void)scheme;
	(void)description;
	return 0;
}

int posix_user_secret(unsigned char *secret, int size)
{
	(void)secret;
	(void)size;
	return 0;
}

int posix_discord_connect(void)
{
	return -1;
}

int posix_discord_write(int handle, const void *buffer, int length)
{
	(void)handle;
	(void)buffer;
	(void)length;
	return -1;
}

int posix_discord_read(int handle, void *buffer, int length)
{
	(void)handle;
	(void)buffer;
	(void)length;
	return -1;
}

void posix_discord_close(int handle)
{
	(void)handle;
}

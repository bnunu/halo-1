/*
P2P_WEBRTC.C

Internet play with browsers (p2p.c): a browser has no UDP socket, only
WebRTC's data channels, so a native build carries a browser peer's tunnel
packets over one, on the tunnel's own socket. The packets are internet
play's, sealed as to any peer; WebRTC is only their way through.

- ICE: this end is an ICE-lite agent (RFC 8445): it answers the browser's
  connectivity checks (STUN with the session's credentials) and sends none,
  and the browser chooses the path. Its credentials come from the session's
  secret (p2p_webrtc_credentials), which both ends have and which never
  travels; the browser is told its addresses, and it is told the browser's,
  through signalling. Until the browser is reached it sends STUN binding
  indications to the browser's addresses every PUNCH_INTERVAL, which open
  this end's NAT to the browser's checks (the browser's checks open its own).
- DTLS (posix_dtls.c): this end is the server. The browser's certificate
  must have the hash the browser signalled (sealed, and bound to its key by
  the signalling's proof), and this end's, which the browser checks, is
  signalled the same way.
- SCTP over DTLS (RFC 8261, 8831), as little of it as one data channel
  takes: the browser opens the association and this end answers; the
  channel is negotiated beforehand (id 0, unordered, no retransmissions:
  the tunnel's datagrams, whose streams KCP makes reliable, as on UDP).
  Messages larger than a packet are split and joined again. What this end
  sends and the peer has not acknowledged in ABANDON_TIME is given up
  (FORWARD-TSN, RFC 3758); what it receives is acknowledged at once, every
  run past a gap (a SACK of only some has the peer count the others in
  flight, and fill its window), and again each SACK_INTERVAL while a gap
  stays. What is missing before a TSN that came GAP_TIME ago is given up as
  the peer's FORWARD-TSN would give it up (the channel's messages are
  never sent again, and the peer sent it before that TSN): Firefox's SCTP,
  after one loss, gives up others only once its cumulative acknowledgement
  has passed the first, which the gaps after it stop, and the channel would
  stall for seconds, then fall out of TSN_WINDOW.

Everything runs on the p2p thread, under p2p_lock.
*/

#include "posix.h"
#include "p2p_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the platform layer's (platform.h, which this file needs no more of: a
test builds it for the computer it runs on, port/web/tests/webrtc_native.c) */
void platform_log(const char *format, ...);

enum
{
	MAXIMUM_CONNECTIONS = P2P_MAXIMUM_PEERS,
	/* the addresses ICE checks passed from (a browser checks several) */
	MAXIMUM_PATHS = 4,
	MAXIMUM_DATAGRAM = 1500,
	/* the largest tunnel packet (p2p.c's), and SCTP's data in a packet
	(under posix_dtls.c's MTU, with SCTP's and DTLS's headers) */
	MAXIMUM_MESSAGE = 1500,
	MAXIMUM_CHUNK_DATA = 1100,
	/* the TSNs past the cumulative one that are kept track of, each way */
	TSN_WINDOW = 1024,
	/* fragments of messages kept for joining */
	MAXIMUM_FRAGMENTS = 16,
	/* the runs past the cumulative TSN a SACK reports: all of them, at a
	tenth of the datagrams lost (a SACK of 12 + 4 * this bytes) */
	MAXIMUM_GAP_BLOCKS = 128,
	SCTP_PORT = 5000,
	/* the window this end says it has: messages are passed on at once */
	RECEIVE_WINDOW = 1 << 20,
	MAXIMUM_STREAMS = 1024,

	/* milliseconds */
	PUNCH_INTERVAL = 200,
	PUNCH_TIME = 30000,
	ABANDON_TIME = 300,
	FORWARD_INTERVAL = 200,
	SACK_INTERVAL = 200,
	/* a gap given up by this end (longer than ABANDON_TIME: what is only
	late still comes, and the peer may give it up first) */
	GAP_TIME = 1000,
	/* a path not checked in this long is not one the browser uses */
	PATH_TIME = 30000,
};

/* STUN (RFC 5389) */
enum
{
	STUN_HEADER_SIZE = 20,
	STUN_COOKIE = 0x2112A442,
	_stun_binding_request = 0x0001,
	_stun_binding_indication = 0x0011,
	_stun_binding_success = 0x0101,
	_stun_username = 0x0006,
	_stun_message_integrity = 0x0008,
	_stun_xor_mapped_address = 0x0020,
	_stun_use_candidate = 0x0025,
	_stun_fingerprint = 0x8028,
};

/* SCTP (RFC 9260) */
enum
{
	_chunk_data = 0,
	_chunk_init = 1,
	_chunk_init_ack = 2,
	_chunk_sack = 3,
	_chunk_heartbeat = 4,
	_chunk_heartbeat_ack = 5,
	_chunk_abort = 6,
	_chunk_shutdown = 7,
	_chunk_shutdown_ack = 8,
	_chunk_cookie_echo = 10,
	_chunk_cookie_ack = 11,
	_chunk_shutdown_complete = 14,
	_chunk_reconfig = 130,
	_chunk_forward_tsn = 192,

	_data_end = 0x01,
	_data_begin = 0x02,
	_data_unordered = 0x04,

	_parameter_state_cookie = 7,
	_parameter_outgoing_reset = 13,
	_parameter_reconfig_response = 16,
	_parameter_supported_extensions = 0x8008,
	_parameter_forward_tsn_supported = 0xC000,

	/* WebRTC's payload protocols: binary and string messages, and the empty
	ones */
	_ppid_string = 51,
	_ppid_binary = 53,
	_ppid_string_empty = 56,
	_ppid_binary_empty = 57,
};

enum
{
	_sctp_closed,
	/* answered the browser's INIT; awaiting its COOKIE ECHO */
	_sctp_cookie_sent,
	_sctp_established,
};

struct fragment
{
	int used;
	unsigned int tsn;
	unsigned char flags;
	int size;
	unsigned char data[MAXIMUM_CHUNK_DATA + 128];
};

struct path
{
	unsigned long address;
	unsigned short port;
	unsigned long checked_time;
};

struct connection
{
	int used;
	int peer;
	/* the peer's name, and whether it is the host (as p2p.c logs them) */
	char name[2 * P2P_IDENTIFIER_SIZE + 1];
	int is_host;
	unsigned long created_time;
	/* ICE: this end's credentials, the addresses checks passed from, and
	where DTLS goes (the nominated path, or the last DTLS came from) */
	char ufrag[P2P_ICE_UFRAG_SIZE];
	char password[P2P_ICE_PASSWORD_SIZE];
	struct path paths[MAXIMUM_PATHS];
	int has_destination;
	struct path destination;
	/* the browser's addresses, punched until it is reached */
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	int candidate_count;
	unsigned long punch_time;
	/* DTLS: the handle, and whether the browser's certificate was checked */
	int dtls;
	int secure;
	int failed;
	unsigned char remote_fingerprint[P2P_FINGERPRINT_SIZE];
	/* SCTP */
	int state;
	unsigned int local_tag;
	unsigned int peer_tag;
	unsigned char cookie[16];
	/* receiving: the cumulative TSN, which past it arrived (by TSN modulo
	TSN_WINDOW), the highest, and whether a SACK is owed */
	unsigned int cumulative;
	unsigned int highest;
	unsigned char received[TSN_WINDOW];
	/* (system time) when each TSN past the cumulative one came */
	unsigned long arrived[TSN_WINDOW];
	int sack_due;
	/* when the last SACK went */
	unsigned long sack_time;
	struct fragment fragments[MAXIMUM_FRAGMENTS];
	/* sending: the next TSN, the peer's cumulative acknowledgement, when
	each TSN past it was sent (by TSN modulo TSN_WINDOW), and the last
	FORWARD-TSN */
	unsigned int next_tsn;
	unsigned int acknowledged;
	unsigned long sent_times[TSN_WINDOW];
	unsigned int forwarded;
	unsigned long forward_time;
};

static struct connection *connections[MAXIMUM_CONNECTIONS];
static unsigned int crc32_table[256], crc32c_table[256];

/* ---------- helpers */

static int elapsed(unsigned long since, unsigned long time)
{
	return (unsigned int)(p2p_now() - since) >= (unsigned int)time;
}

static unsigned int get_long(const unsigned char *bytes)
{
	return (unsigned int)bytes[0] << 24 | (unsigned int)bytes[1] << 16 | (unsigned int)bytes[2] << 8 | bytes[3];
}

static unsigned short get_short(const unsigned char *bytes)
{
	return (unsigned short)(bytes[0] << 8 | bytes[1]);
}

static void put_long(unsigned char *bytes, unsigned int value)
{
	bytes[0] = (unsigned char)(value >> 24);
	bytes[1] = (unsigned char)(value >> 16);
	bytes[2] = (unsigned char)(value >> 8);
	bytes[3] = (unsigned char)value;
}

static void put_short(unsigned char *bytes, unsigned short value)
{
	bytes[0] = (unsigned char)(value >> 8);
	bytes[1] = (unsigned char)value;
}

/* TSNs in serial number arithmetic: a after b */
static int tsn_after(unsigned int a, unsigned int b)
{
	return (int)(a - b) > 0;
}

static unsigned int random_long(void)
{
	unsigned int value;

	posix_random_bytes(&value, sizeof(value));
	return value;
}

static void make_tables(void)
{
	unsigned int index;

	if (crc32_table[1])
		return;
	for (index = 0; index < 256; index++)
	{
		unsigned int crc = index, crc_c = index;
		int bit;

		for (bit = 0; bit < 8; bit++)
		{
			crc = crc & 1 ? 0xEDB88320 ^ crc >> 1 : crc >> 1;
			crc_c = crc_c & 1 ? 0x82F63B78 ^ crc_c >> 1 : crc_c >> 1;
		}
		crc32_table[index] = crc;
		crc32c_table[index] = crc_c;
	}
}

/* CRC-32 (STUN's FINGERPRINT) and CRC-32C (SCTP's checksum) */
static unsigned int checksum(const unsigned int *table, const unsigned char *data, int size)
{
	unsigned int crc = 0xFFFFFFFF;
	int index;

	make_tables();
	for (index = 0; index < size; index++)
		crc = table[(crc ^ data[index]) & 255] ^ crc >> 8;
	return crc ^ 0xFFFFFFFF;
}

static struct connection *connection_of(int index)
{
	return index >= 0 && index < MAXIMUM_CONNECTIONS ? connections[index] : NULL;
}

/* ---------- ICE */

/* a STUN attribute's place in a message; NULL if it has none */
static const unsigned char *stun_attribute(const unsigned char *message, int size, int type, int *length,
	int *offset_found)
{
	int offset;

	for (offset = STUN_HEADER_SIZE; offset + 4 <= size; )
	{
		int attribute = get_short(message + offset);
		int value_length = get_short(message + offset + 2);

		if (offset + 4 + value_length > size)
			return NULL;
		if (attribute == type)
		{
			*length = value_length;
			if (offset_found)
				*offset_found = offset;
			return message + offset + 4;
		}
		offset += 4 + ((value_length + 3) & ~3);
	}
	return NULL;
}

/* whether a message's MESSAGE-INTEGRITY (at offset) is right with the key */
static int stun_integrity_right(const unsigned char *message, int offset, const char *key)
{
	unsigned char copy[MAXIMUM_DATAGRAM];
	unsigned char digest[20];

	if (offset > (int)sizeof(copy) - 24)
		return 0;
	memcpy(copy, message, (size_t)offset);
	/* (the length as if the message ended after the integrity) */
	put_short(copy + 2, (unsigned short)(offset + 24 - STUN_HEADER_SIZE));
	posix_hmac_sha1(key, (int)strlen(key), copy, offset, digest);
	return p2p_equal(digest, message + offset + 4, 20);
}

/* ends a message (its header's type and transaction, and its attributes so
far, at size) with MESSAGE-INTEGRITY (if key) and FINGERPRINT; returns its
size */
static int stun_finish(unsigned char *message, int size, const char *key)
{
	if (key)
	{
		put_short(message + 2, (unsigned short)(size + 24 - STUN_HEADER_SIZE));
		put_short(message + size, _stun_message_integrity);
		put_short(message + size + 2, 20);
		posix_hmac_sha1(key, (int)strlen(key), message, size, message + size + 4);
		size += 24;
	}
	put_short(message + 2, (unsigned short)(size + 8 - STUN_HEADER_SIZE));
	put_short(message + size, _stun_fingerprint);
	put_short(message + size + 2, 4);
	put_long(message + size + 4, checksum(crc32_table, message, size) ^ 0x5354554E);
	return size + 8;
}

static void path_checked(struct connection *connection, unsigned long address, unsigned short port,
	int nominated)
{
	struct path *oldest = &connection->paths[0];
	int index;

	for (index = 0; index < MAXIMUM_PATHS; index++)
	{
		struct path *path = &connection->paths[index];

		if (path->address == address && path->port == port)
		{
			oldest = path;
			break;
		}
		if ((long)(path->checked_time - oldest->checked_time) < 0 || !path->address)
			oldest = path;
	}
	oldest->address = address;
	oldest->port = port;
	oldest->checked_time = p2p_now() | 1;
	if (nominated || !connection->has_destination)
	{
		connection->destination = *oldest;
		connection->has_destination = 1;
	}
}

/* a browser's connectivity check: answered if it has a connection's
credentials */
static void binding_request(const unsigned char *message, int size, unsigned long address, unsigned short port)
{
	const unsigned char *username;
	unsigned char answer[128];
	int length, integrity_length, integrity_offset, separator;
	int answer_size;
	int index;

	username = stun_attribute(message, size, _stun_username, &length, NULL);
	if (!username || !stun_attribute(message, size, _stun_message_integrity, &integrity_length,
		&integrity_offset) || integrity_length != 20)
	{
		return;
	}
	/* (this end's ufrag, a colon, the browser's) */
	for (separator = 0; separator < length && username[separator] != ':'; separator++)
		;
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		struct connection *connection = connections[index];
		int nominated;

		if (!connection || connection->peer < 0 || (int)strlen(connection->ufrag) != separator ||
			memcmp(connection->ufrag, username, (size_t)separator))
		{
			continue;
		}
		if (!stun_integrity_right(message, integrity_offset, connection->password))
			return;
		nominated = stun_attribute(message, size, _stun_use_candidate, &length, NULL) != NULL;
		path_checked(connection, address, port, nominated);
		/* the answer: the address it came from, sealed with the password */
		memset(answer, 0, STUN_HEADER_SIZE);
		put_short(answer, _stun_binding_success);
		memcpy(answer + 4, message + 4, 16);
		answer_size = STUN_HEADER_SIZE;
		put_short(answer + answer_size, _stun_xor_mapped_address);
		put_short(answer + answer_size + 2, 8);
		answer[answer_size + 4] = 0;
		answer[answer_size + 5] = 1;
		{
			unsigned char raw[6];
			int byte;

			memcpy(raw, &port, 2);
			memcpy(raw + 2, &address, 4);
			for (byte = 0; byte < 6; byte++)
				answer[answer_size + 6 + byte] = (unsigned char)(raw[byte] ^ message[4 + (byte < 2 ? byte : byte - 2)]);
		}
		answer_size += 12;
		answer_size = stun_finish(answer, answer_size, connection->password);
		p2p_tunnel_send(address, port, answer, answer_size);
		return;
	}
}

static void punch(struct connection *connection)
{
	unsigned char indication[STUN_HEADER_SIZE + 8];
	int size;
	int index;

	memset(indication, 0, STUN_HEADER_SIZE);
	put_short(indication, _stun_binding_indication);
	put_long(indication + 4, STUN_COOKIE);
	posix_random_bytes(indication + 8, 12);
	size = stun_finish(indication, STUN_HEADER_SIZE, NULL);
	for (index = 0; index < connection->candidate_count; index++)
		p2p_tunnel_send(connection->candidates[index].address, connection->candidates[index].port, indication, size);
	connection->punch_time = p2p_now();
}

/* ---------- DTLS */

static void dtls_flush(struct connection *connection)
{
	unsigned char datagram[MAXIMUM_DATAGRAM];
	int size;

	while ((size = posix_dtls_output(connection->dtls, datagram, sizeof(datagram))) > 0)
	{
		if (connection->has_destination)
			p2p_tunnel_send(connection->destination.address, connection->destination.port, datagram, size);
	}
}

static void connection_fail(struct connection *connection, const char *reason)
{
	if (!connection->failed)
		platform_log("Internet play: WebRTC with %s %s: %s", connection->is_host ? "host" : "player", connection->name,
			reason);
	connection->failed = 1;
	connection->state = _sctp_closed;
}

/* ---------- SCTP: what this end sends */

/* one SCTP packet: the common header, then the chunks (already in packet
past it, size bytes in all) */
static void sctp_send_packet(struct connection *connection, unsigned char *packet, int size, unsigned int tag)
{
	unsigned int crc;

	put_short(packet, SCTP_PORT);
	put_short(packet + 2, SCTP_PORT);
	put_long(packet + 4, tag);
	put_long(packet + 8, 0);
	/* (CRC-32C, its bytes in the order computed) */
	crc = checksum(crc32c_table, packet, size);
	packet[8] = (unsigned char)crc;
	packet[9] = (unsigned char)(crc >> 8);
	packet[10] = (unsigned char)(crc >> 16);
	packet[11] = (unsigned char)(crc >> 24);
	posix_dtls_send(connection->dtls, packet, size);
	dtls_flush(connection);
}

/* a packet of a single chunk */
static void sctp_send_chunk(struct connection *connection, int type, int flags, const unsigned char *value,
	int length)
{
	unsigned char packet[12 + 4 + MAXIMUM_CHUNK_DATA + 64];
	int padded = (length + 3) & ~3;

	if (length > (int)sizeof(packet) - 16)
		return;
	packet[12] = (unsigned char)type;
	packet[13] = (unsigned char)flags;
	put_short(packet + 14, (unsigned short)(4 + length));
	memcpy(packet + 16, value, (size_t)length);
	memset(packet + 16 + length, 0, (size_t)(padded - length));
	sctp_send_packet(connection, packet, 16 + padded, connection->peer_tag);
}

static void sctp_send_sack(struct connection *connection)
{
	unsigned char value[12 + 4 * MAXIMUM_GAP_BLOCKS];
	int blocks = 0;
	unsigned int tsn;

	put_long(value, connection->cumulative);
	put_long(value + 4, RECEIVE_WINDOW);
	/* the runs received past the cumulative TSN */
	for (tsn = connection->cumulative + 1; !tsn_after(tsn, connection->highest) && blocks < MAXIMUM_GAP_BLOCKS; tsn++)
	{
		unsigned int start;

		if (!connection->received[tsn % TSN_WINDOW])
			continue;
		start = tsn;
		while (!tsn_after(tsn + 1, connection->highest) && connection->received[(tsn + 1) % TSN_WINDOW])
			tsn++;
		put_short(value + 12 + blocks * 4, (unsigned short)(start - connection->cumulative));
		put_short(value + 14 + blocks * 4, (unsigned short)(tsn - connection->cumulative));
		blocks++;
	}
	put_short(value + 8, (unsigned short)blocks);
	put_short(value + 10, 0);
	sctp_send_chunk(connection, _chunk_sack, 0, value, 12 + blocks * 4);
	connection->sack_due = 0;
	connection->sack_time = p2p_now();
}

/* a message on the channel (stream 0, unordered), split as needed */
static void sctp_send_message(struct connection *connection, const unsigned char *data, int size)
{
	unsigned char value[12 + MAXIMUM_CHUNK_DATA];
	int offset = 0;

	if (connection->state != _sctp_established || size <= 0 || size > MAXIMUM_MESSAGE)
		return;
	/* (the peer far behind: lost, until FORWARD-TSN catches up) */
	if (connection->next_tsn - connection->acknowledged >= TSN_WINDOW - 4)
		return;
	while (offset < size)
	{
		int length = size - offset > MAXIMUM_CHUNK_DATA ? MAXIMUM_CHUNK_DATA : size - offset;
		int flags = _data_unordered | (offset == 0 ? _data_begin : 0) | (offset + length == size ? _data_end : 0);
		unsigned int tsn = connection->next_tsn++;

		put_long(value, tsn);
		put_short(value + 4, 0);
		put_short(value + 6, 0);
		put_long(value + 8, _ppid_binary);
		memcpy(value + 12, data + offset, (size_t)length);
		connection->sent_times[tsn % TSN_WINDOW] = p2p_now();
		sctp_send_chunk(connection, _chunk_data, flags, value, 12 + length);
		offset += length;
	}
}

/* what this end sent and the peer has not acknowledged in ABANDON_TIME is
given up: FORWARD-TSN moves the peer's cumulative TSN past it */
static void sctp_forward(struct connection *connection)
{
	unsigned int last = connection->acknowledged;
	unsigned char value[4];

	while (tsn_after(connection->next_tsn, last + 1) && elapsed(connection->sent_times[(last + 1) % TSN_WINDOW],
		ABANDON_TIME))
	{
		last++;
	}
	if (!tsn_after(last, connection->acknowledged))
		return;
	if (last == connection->forwarded && !elapsed(connection->forward_time, FORWARD_INTERVAL))
		return;
	put_long(value, last);
	sctp_send_chunk(connection, _chunk_forward_tsn, 0, value, 4);
	connection->forwarded = last;
	connection->forward_time = p2p_now();
}

/* ---------- SCTP: what the peer sends */

static void deliver(struct connection *connection, const unsigned char *data, int size)
{
	struct path const *from = &connection->destination;

	if (connection->peer >= 0 && size > 0)
		p2p_tunnel_packet(data, size, from->address, from->port);
}

/* a message's fragments, once all have come (unordered: consecutive TSNs) */
static void join_fragments(struct connection *connection)
{
	int index;

	for (index = 0; index < MAXIMUM_FRAGMENTS; index++)
	{
		struct fragment *first = &connection->fragments[index];
		unsigned char message[MAXIMUM_MESSAGE];
		int parts[MAXIMUM_FRAGMENTS];
		int count = 0;
		int size = 0;
		unsigned int tsn;

		if (!first->used || !(first->flags & _data_begin))
			continue;
		for (tsn = first->tsn; count < MAXIMUM_FRAGMENTS; tsn++)
		{
			int found;

			for (found = 0; found < MAXIMUM_FRAGMENTS; found++)
			{
				if (connection->fragments[found].used && connection->fragments[found].tsn == tsn)
					break;
			}
			if (found == MAXIMUM_FRAGMENTS)
				break;
			parts[count++] = found;
			if (connection->fragments[found].flags & _data_end)
				break;
		}
		if (!count || !(connection->fragments[parts[count - 1]].flags & _data_end))
			continue;
		{
			int part;
			int fits = 1;

			for (part = 0; part < count; part++)
			{
				struct fragment *fragment = &connection->fragments[parts[part]];

				if (size + fragment->size > (int)sizeof(message))
					fits = 0;
				else
					memcpy(message + size, fragment->data, (size_t)fragment->size);
				size += fragment->size;
				fragment->used = 0;
			}
			if (fits)
				deliver(connection, message, size);
		}
	}
}

static void data_received(struct connection *connection, int flags, const unsigned char *value, int length)
{
	unsigned int tsn;
	unsigned int ppid;
	const unsigned char *data = value + 12;
	int size = length - 12;

	if (length < 12)
		return;
	tsn = get_long(value);
	ppid = get_long(value + 8);
	connection->sack_due = 1;
	if (!tsn_after(tsn, connection->cumulative) || tsn - connection->cumulative > TSN_WINDOW - 1 ||
		connection->received[tsn % TSN_WINDOW])
	{
		/* (again, or too far ahead: acknowledged as it stands) */
		return;
	}
	connection->received[tsn % TSN_WINDOW] = 1;
	connection->arrived[tsn % TSN_WINDOW] = p2p_now();
	if (tsn_after(tsn, connection->highest))
		connection->highest = tsn;
	while (connection->received[(connection->cumulative + 1) % TSN_WINDOW] &&
		tsn_after(connection->highest + 1, connection->cumulative + 1))
	{
		connection->cumulative++;
		connection->received[connection->cumulative % TSN_WINDOW] = 0;
	}
	/* (the channel's messages: binary, or strings, which none of this
	version's sends; not the channel's control messages) */
	if (ppid != _ppid_binary && ppid != _ppid_binary_empty && ppid != _ppid_string && ppid != _ppid_string_empty)
		return;
	if (ppid == _ppid_binary_empty || ppid == _ppid_string_empty)
		size = 0;
	if ((flags & (_data_begin | _data_end)) == (_data_begin | _data_end))
	{
		deliver(connection, data, size);
		return;
	}
	{
		struct fragment *slot = NULL;
		int index;

		for (index = 0; index < MAXIMUM_FRAGMENTS && !slot; index++)
		{
			if (!connection->fragments[index].used)
				slot = &connection->fragments[index];
		}
		/* (full: the oldest goes, a message lost) */
		if (!slot)
		{
			slot = &connection->fragments[0];
			for (index = 1; index < MAXIMUM_FRAGMENTS; index++)
			{
				if (tsn_after(slot->tsn, connection->fragments[index].tsn))
					slot = &connection->fragments[index];
			}
		}
		if (size > (int)sizeof(slot->data))
			return;
		slot->used = 1;
		slot->tsn = tsn;
		slot->flags = (unsigned char)flags;
		slot->size = size;
		memcpy(slot->data, data, (size_t)size);
		join_fragments(connection);
	}
}

/* what the peer gave up (FORWARD-TSN), or this end did: the cumulative TSN
moved to forward and past what came after it, and the fragments of messages
given up let go */
static void skip_to(struct connection *connection, unsigned int forward)
{
	int index;

	connection->sack_due = 1;
	if (!tsn_after(forward, connection->cumulative))
		return;
	if (forward - connection->cumulative >= TSN_WINDOW)
		memset(connection->received, 0, sizeof(connection->received));
	else
	{
		while (tsn_after(forward, connection->cumulative))
		{
			connection->cumulative++;
			connection->received[connection->cumulative % TSN_WINDOW] = 0;
		}
	}
	connection->cumulative = forward;
	if (tsn_after(forward, connection->highest))
		connection->highest = forward;
	while (connection->received[(connection->cumulative + 1) % TSN_WINDOW] &&
		tsn_after(connection->highest + 1, connection->cumulative + 1))
	{
		connection->cumulative++;
		connection->received[connection->cumulative % TSN_WINDOW] = 0;
	}
	/* (what is past it of a message given up will never be whole) */
	for (index = 0; index < MAXIMUM_FRAGMENTS; index++)
	{
		if (connection->fragments[index].used && !tsn_after(connection->fragments[index].tsn, forward))
			connection->fragments[index].used = 0;
	}
}

static void forward_tsn_received(struct connection *connection, const unsigned char *value, int length)
{
	if (length >= 4)
		skip_to(connection, get_long(value));
}

/* the gaps past the cumulative TSN: what is missing before a TSN that came
GAP_TIME ago given up, and the rest told again each SACK_INTERVAL */
static void gap_check(struct connection *connection)
{
	unsigned int tsn;

	if (!tsn_after(connection->highest, connection->cumulative))
		return;
	for (tsn = connection->highest; tsn_after(tsn, connection->cumulative); tsn--)
	{
		if (connection->received[tsn % TSN_WINDOW] && elapsed(connection->arrived[tsn % TSN_WINDOW], GAP_TIME))
		{
			skip_to(connection, tsn);
			break;
		}
	}
	if (connection->sack_due || (tsn_after(connection->highest, connection->cumulative) &&
		elapsed(connection->sack_time, SACK_INTERVAL)))
	{
		sctp_send_sack(connection);
	}
}

static void sack_received(struct connection *connection, const unsigned char *value, int length)
{
	unsigned int cumulative;

	if (length < 12)
		return;
	cumulative = get_long(value);
	if (tsn_after(cumulative, connection->acknowledged) && !tsn_after(cumulative, connection->next_tsn - 1))
		connection->acknowledged = cumulative;
}

static void init_received(struct connection *connection, const unsigned char *value, int length)
{
	unsigned char answer[20 + 4 + 16 + 4 + 2 + 2 + 4];
	unsigned int tag;
	int size = 0;
	int outbound, inbound;

	if (length < 16)
		return;
	tag = get_long(value);
	if (!tag)
		return;
	/* a new association (the first, or the peer's after a restart) */
	if (connection->state != _sctp_cookie_sent || tag != connection->peer_tag)
	{
		connection->peer_tag = tag;
		connection->local_tag = random_long() | 1;
		posix_random_bytes(connection->cookie, sizeof(connection->cookie));
		connection->cumulative = get_long(value + 12) - 1;
		connection->highest = connection->cumulative;
		memset(connection->received, 0, sizeof(connection->received));
		memset(connection->fragments, 0, sizeof(connection->fragments));
		connection->next_tsn = random_long();
		connection->acknowledged = connection->next_tsn - 1;
		connection->forwarded = connection->acknowledged;
		connection->state = _sctp_cookie_sent;
	}
	outbound = get_short(value + 8);
	inbound = get_short(value + 10);
	put_long(answer + size, connection->local_tag);
	put_long(answer + size + 4, RECEIVE_WINDOW);
	put_short(answer + size + 8, (unsigned short)(inbound < MAXIMUM_STREAMS ? inbound : MAXIMUM_STREAMS));
	put_short(answer + size + 10, (unsigned short)(outbound < MAXIMUM_STREAMS ? outbound : MAXIMUM_STREAMS));
	put_long(answer + size + 12, connection->next_tsn);
	size += 16;
	/* the state cookie, which its COOKIE ECHO returns */
	put_short(answer + size, _parameter_state_cookie);
	put_short(answer + size + 2, 4 + sizeof(connection->cookie));
	memcpy(answer + size + 4, connection->cookie, sizeof(connection->cookie));
	size += 4 + sizeof(connection->cookie);
	/* FORWARD-TSN and RE-CONFIG (which WebRTC's SCTP requires) */
	put_short(answer + size, _parameter_supported_extensions);
	put_short(answer + size + 2, 6);
	answer[size + 4] = _chunk_forward_tsn;
	answer[size + 5] = _chunk_reconfig;
	answer[size + 6] = 0;
	answer[size + 7] = 0;
	size += 8;
	put_short(answer + size, _parameter_forward_tsn_supported);
	put_short(answer + size + 2, 4);
	size += 4;
	sctp_send_chunk(connection, _chunk_init_ack, 0, answer, size);
}

static void cookie_echo_received(struct connection *connection, const unsigned char *value, int length)
{
	if (length != (int)sizeof(connection->cookie) || connection->state == _sctp_closed ||
		!p2p_equal(value, connection->cookie, sizeof(connection->cookie)))
	{
		return;
	}
	if (connection->state != _sctp_established)
		platform_log("Internet play: WebRTC connected to %s %s's browser", connection->is_host ? "host" : "player",
			connection->name);
	connection->state = _sctp_established;
	sctp_send_chunk(connection, _chunk_cookie_ack, 0, NULL, 0);
}

/* a stream reset (a channel closing): done, as the channel is the only one */
static void reconfig_received(struct connection *connection, const unsigned char *value, int length)
{
	int offset;

	for (offset = 0; offset + 4 <= length; )
	{
		int type = get_short(value + offset);
		int parameter_length = get_short(value + offset + 2);

		if (parameter_length < 4 || offset + parameter_length > length)
			return;
		if (type == _parameter_outgoing_reset && parameter_length >= 16)
		{
			unsigned char answer[12];

			put_short(answer, _parameter_reconfig_response);
			put_short(answer + 2, 12);
			memcpy(answer + 4, value + offset + 4, 4);
			/* Success - Performed */
			put_long(answer + 8, 1);
			sctp_send_chunk(connection, _chunk_reconfig, 0, answer, sizeof(answer));
		}
		offset += (parameter_length + 3) & ~3;
	}
}

static void sctp_received(struct connection *connection, const unsigned char *packet, int size)
{
	unsigned char copy[MAXIMUM_DATAGRAM];
	unsigned int crc;
	unsigned int tag;
	int offset;

	if (size < 16 || size > (int)sizeof(copy))
		return;
	memcpy(copy, packet, (size_t)size);
	memset(copy + 8, 0, 4);
	crc = checksum(crc32c_table, copy, size);
	if (packet[8] != (unsigned char)crc || packet[9] != (unsigned char)(crc >> 8) ||
		packet[10] != (unsigned char)(crc >> 16) || packet[11] != (unsigned char)(crc >> 24))
	{
		return;
	}
	tag = get_long(packet + 4);
	for (offset = 12; offset + 4 <= size; )
	{
		int type = packet[offset];
		int flags = packet[offset + 1];
		int length = get_short(packet + offset + 2);
		const unsigned char *value = packet + offset + 4;

		if (length < 4 || offset + length > size)
			break;
		/* (an INIT has no tag yet; everything else, this end's) */
		if (type == _chunk_init)
		{
			if (tag == 0)
				init_received(connection, value, length - 4);
			break;
		}
		if (tag != connection->local_tag || connection->state == _sctp_closed)
			break;
		switch (type)
		{
		case _chunk_cookie_echo:
			cookie_echo_received(connection, value, length - 4);
			break;
		case _chunk_data:
			if (connection->state == _sctp_established)
				data_received(connection, flags, value, length - 4);
			break;
		case _chunk_sack:
			sack_received(connection, value, length - 4);
			break;
		case _chunk_forward_tsn:
			forward_tsn_received(connection, value, length - 4);
			break;
		case _chunk_heartbeat:
			sctp_send_chunk(connection, _chunk_heartbeat_ack, 0, value, length - 4);
			break;
		case _chunk_reconfig:
			reconfig_received(connection, value, length - 4);
			break;
		case _chunk_shutdown:
			sctp_send_chunk(connection, _chunk_shutdown_ack, 0, NULL, 0);
			connection_fail(connection, "the browser closed the connection");
			return;
		case _chunk_abort:
		case _chunk_shutdown_complete:
			connection_fail(connection, "the browser closed the connection");
			return;
		}
		/* (a chunk may have closed it) */
		if (connection->state == _sctp_closed)
			return;
		offset += (length + 3) & ~3;
	}
}

/* ---------- the connection's DTLS records */

static void connection_receive(struct connection *connection)
{
	unsigned char record[MAXIMUM_DATAGRAM];
	int size;

	while (!connection->failed &&
		(size = posix_dtls_receive(connection->dtls, (posix_ulong)p2p_now(), record, sizeof(record))) != 0)
	{
		if (size < 0)
		{
			connection_fail(connection, "DTLS failed or closed");
			break;
		}
		if (!connection->secure)
		{
			unsigned char fingerprint[P2P_FINGERPRINT_SIZE];

			if (!posix_dtls_peer_fingerprint(connection->dtls, fingerprint) ||
				!p2p_equal(fingerprint, connection->remote_fingerprint, P2P_FINGERPRINT_SIZE))
			{
				connection_fail(connection, "its certificate is not the one it signalled");
				break;
			}
			connection->secure = 1;
		}
		sctp_received(connection, record, size);
	}
	/* the handshake's own, and whatever the records' answers sent */
	if (!connection->secure && !connection->failed)
	{
		unsigned char fingerprint[P2P_FINGERPRINT_SIZE];

		if (posix_dtls_peer_fingerprint(connection->dtls, fingerprint))
		{
			if (p2p_equal(fingerprint, connection->remote_fingerprint, P2P_FINGERPRINT_SIZE))
				connection->secure = 1;
			else
				connection_fail(connection, "its certificate is not the one it signalled");
		}
	}
	if (connection->sack_due && connection->state == _sctp_established)
		sctp_send_sack(connection);
	dtls_flush(connection);
}

/* ---------- p2p.c's side */

int p2p_webrtc_describe(const unsigned char *identifier, int proven, struct p2p_webrtc *local,
	struct p2p_candidate *candidates, int maximum_count)
{
	(void)identifier;
	(void)proven;
	memset(local, 0, sizeof(*local));
	/* (no certificate: no browsers, only other native builds) */
	local->kind = posix_dtls_fingerprint(local->fingerprint) ? _p2p_webrtc_native : _p2p_webrtc_none;
	return p2p_local_candidates(candidates, maximum_count);
}

int p2p_webrtc_ready(void)
{
	return 1;
}

void p2p_webrtc_new_request(void)
{
}

int p2p_webrtc_offered(int index, int peer, const unsigned char *secret, const struct p2p_webrtc *remote,
	const struct p2p_candidate *candidates, int count, int is_host)
{
	struct connection *connection = connection_of(index);
	int candidate;

	if (!connection)
	{
		unsigned char fingerprint[P2P_FINGERPRINT_SIZE];

		/* another native build's tunnel is plain UDP */
		if (remote->kind != _p2p_webrtc_browser || !posix_dtls_fingerprint(fingerprint))
			return -1;
		for (index = 0; index < MAXIMUM_CONNECTIONS && connections[index]; index++)
			;
		if (index == MAXIMUM_CONNECTIONS)
			return -1;
		connection = calloc(1, sizeof(*connection));
		if (!connection)
			return -1;
		connection->dtls = posix_dtls_open();
		if (connection->dtls < 0)
		{
			free(connection);
			return -1;
		}
		connection->used = 1;
		connection->peer = peer;
		connection->created_time = p2p_now();
		p2p_webrtc_credentials(secret, connection->ufrag, connection->password);
		memcpy(connection->remote_fingerprint, remote->fingerprint, P2P_FINGERPRINT_SIZE);
		connections[index] = connection;
	}
	p2p_peer_name(peer, connection->name);
	connection->is_host = is_host;
	for (candidate = 0; candidate < count; candidate++)
	{
		int known;

		/* (a browser's ICE is reached on the IPv4 tunnel alone) */
		if (candidates[candidate].ipv6)
			continue;
		for (known = 0; known < connection->candidate_count; known++)
		{
			if (connection->candidates[known].address == candidates[candidate].address &&
				connection->candidates[known].port == candidates[candidate].port)
				break;
		}
		if (known == connection->candidate_count && connection->candidate_count < P2P_MAXIMUM_CANDIDATES)
			connection->candidates[connection->candidate_count++] = candidates[candidate];
	}
	return index;
}

void p2p_webrtc_close(int index)
{
	struct connection *connection = connection_of(index);

	if (!connection)
		return;
	/* (the browser learns at once, not when its checks lapse) */
	if (connection->state == _sctp_established)
		sctp_send_chunk(connection, _chunk_abort, 0, NULL, 0);
	/* (let go of in the next update: this may be called from within its
	own delivery) */
	connection->peer = -1;
}

void p2p_webrtc_send(int index, const void *packet, int size)
{
	struct connection *connection = connection_of(index);

	if (connection && connection->peer >= 0 && connection->secure && !connection->failed)
		sctp_send_message(connection, packet, size);
}

int p2p_webrtc_received(const unsigned char *packet, int size, unsigned long address, unsigned short port)
{
	int index;

	if (size < 1)
		return 0;
	/* an ICE check (a STUN request; answers are p2p.c's STUN servers') */
	if (size >= STUN_HEADER_SIZE && packet[0] == 0 && packet[1] == 1 && get_long(packet + 4) == STUN_COOKIE)
	{
		if (STUN_HEADER_SIZE + get_short(packet + 2) == size)
			binding_request(packet, size, address, port);
		return 1;
	}
	/* DTLS (RFC 7983): from a path a connection's checks passed on */
	if (packet[0] < 20 || packet[0] > 63)
		return 0;
	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		struct connection *connection = connections[index];
		int path;

		if (!connection || connection->peer < 0 || connection->failed)
			continue;
		for (path = 0; path < MAXIMUM_PATHS; path++)
		{
			if (connection->paths[path].address == address && connection->paths[path].port == port &&
				connection->paths[path].checked_time && !elapsed(connection->paths[path].checked_time, PATH_TIME))
			{
				break;
			}
		}
		if (path == MAXIMUM_PATHS)
			continue;
		/* (the browser's DTLS goes where it came from) */
		connection->destination = connection->paths[path];
		connection->has_destination = 1;
		posix_dtls_input(connection->dtls, packet, size);
		connection_receive(connection);
		return 1;
	}
	return 1;
}

int p2p_webrtc_update(void)
{
	int busy = 0;
	int index;

	for (index = 0; index < MAXIMUM_CONNECTIONS; index++)
	{
		struct connection *connection = connections[index];

		if (!connection)
			continue;
		if (connection->peer < 0)
		{
			posix_dtls_close(connection->dtls);
			free(connection);
			connections[index] = NULL;
			continue;
		}
		busy = 1;
		if (connection->failed)
			continue;
		/* (the handshake's retransmissions) */
		connection_receive(connection);
		if (connection->state == _sctp_established)
		{
			sctp_forward(connection);
			gap_check(connection);
		}
		else if (!elapsed(connection->created_time, PUNCH_TIME) && elapsed(connection->punch_time, PUNCH_INTERVAL))
			punch(connection);
	}
	return busy;
}

/*
STEAM_CM.C

steam_cm.h's session. The layouts are Steam's (SteamKit's and
DepotDownloader's reading of them):

- TCP: each packet its length (32 bits, little-endian), "VT01", then its
  bytes. The server's first is ChannelEncryptRequest (the old header: EMsg,
  target and source jobs; a protocol version, the universe and 16 random
  bytes); the client's answer, ChannelEncryptResponse, holds a random
  32-byte session key and those bytes RSA'd (OAEP, SHA-1) under the
  universe's key, that blob's CRC-32 and a zero; ChannelEncryptResult's
  EResult opens the channel. From then on each packet is the AES-256 of its
  bytes with the HMAC'd vector (steam_crypto.h).
- a message: its EMsg with the protocol-buffer bit, its header's length,
  the header (CMsgProtoBufHeader: the Steam id, the session, the jobs, the
  service method's name, the EResult), then its body. Multi holds others,
  gzipped when size_unzipped says so.

Only the messages and fields this needs are read; others are let go.
*/

#include "steam_cm.h"
#include "steam_crypto.h"
#include "steam_net.h"
#include "posix.h"
#include "zlib_prefixed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	EMSG_MULTI = 1,
	EMSG_SERVICE_METHOD_RESPONSE = 147,
	EMSG_SERVICE_METHOD_CALL_FROM_CLIENT = 151,
	EMSG_CLIENT_HEARTBEAT = 703,
	EMSG_CLIENT_LOGON_RESPONSE = 751,
	EMSG_CLIENT_LOGGED_OFF = 757,
	EMSG_CHANNEL_ENCRYPT_REQUEST = 1303,
	EMSG_CHANNEL_ENCRYPT_RESPONSE = 1304,
	EMSG_CHANNEL_ENCRYPT_RESULT = 1305,
	EMSG_CLIENT_GET_DEPOT_DECRYPTION_KEY = 5438,
	EMSG_CLIENT_GET_DEPOT_DECRYPTION_KEY_RESPONSE = 5439,
	EMSG_CLIENT_LOGON = 5514,
	EMSG_CLIENT_PICS_PRODUCT_INFO_REQUEST = 8903,
	EMSG_CLIENT_PICS_PRODUCT_INFO_RESPONSE = 8904,
	EMSG_CLIENT_PICS_ACCESS_TOKEN_REQUEST = 8905,
	EMSG_CLIENT_PICS_ACCESS_TOKEN_RESPONSE = 8906,
	EMSG_SERVICE_METHOD_CALL_FROM_CLIENT_NON_AUTHED = 9804,
	EMSG_CLIENT_HELLO = 9805,

	PROTO_MASK = 0x80000000u,
	PROTOCOL_VERSION = 65580,
	/* EOSType's Windows 11, as the Steam client of a Windows machine says */
	CLIENT_OS_TYPE = 20,
	TCP_MAGIC = 0x31305456, /* "VT01" */
	MAXIMUM_PACKET = 16 * 1024 * 1024,
	CONNECT_MILLISECONDS = 10000,
	ANSWER_MILLISECONDS = 30000,
	MAXIMUM_SERVERS = 8
};

#define JOB_NONE 0xFFFFFFFFFFFFFFFFull
/* (posix_socket_getsockopt takes Winsock's level and names: posix_net.c) */
#define WINSOCK_SOL_SOCKET 0xffff
#define WINSOCK_SO_ERROR 0x1007
#define SOONER(a, b) ((a) < (b) ? (a) : (b))

struct cm_message
{
	struct cm_message *next;
	uint32_t emsg;
	uint64_t job_target;
	int32_t eresult;
	uint64_t steam_id;
	int32_t session_id;
	unsigned char *body;
	size_t body_size;
};

struct steam_cm
{
	int socket;
	int connected;
	int encrypted;
	struct steam_aes aes;
	unsigned char hmac_key[16];
	uint64_t steam_id;
	int32_t session_id;
	int logged_on;
	uint64_t next_job;
	unsigned long heartbeat_milliseconds;
	unsigned long last_sent;
	/* the messages a Multi held, not yet looked at */
	struct cm_message *queued;
	char error[256];
};

char const *steam_cm_error(struct steam_cm const *cm)
{
	return cm ? cm->error : "";
}

static void cm_fail(struct steam_cm *cm, char const *format, char const *detail)
{
	snprintf(cm->error, sizeof(cm->error), format, detail ? detail : "");
	steam_log("steam: %s", cm->error);
}

/* (the bytes a message held wiped as they go: a sign-in's answer has the
refresh token) */
static void wipe_free(void *memory, size_t size)
{
	if (memory)
		steam_wipe(memory, size);
	free(memory);
}

static void message_free(struct cm_message *message)
{
	if (message)
	{
		wipe_free(message->body, message->body_size + 1);
		free(message);
	}
}

static void cm_drop_connection(struct steam_cm *cm)
{
	if (cm->socket >= 0)
		posix_socket_close(cm->socket);
	cm->socket = -1;
	cm->connected = 0;
	cm->logged_on = 0;
}

/* ---------- the socket */

static int socket_wait(int socket, unsigned long milliseconds, int writing)
{
	int list[1];
	int count = 1;
	int none = 0;

	list[0] = socket;
	if (posix_socket_select(writing ? NULL : list, writing ? &none : &count, writing ? list : NULL,
		writing ? &count : &none, NULL, &none, (posix_long)(milliseconds / 1000),
		(posix_long)(milliseconds % 1000) * 1000, 0) < 0)
		return -1;
	return count > 0;
}

static int send_all(struct steam_cm *cm, unsigned char const *data, size_t size)
{
	while (size)
	{
		int sent;

		if (socket_wait(cm->socket, ANSWER_MILLISECONDS, 1) <= 0)
			return 0;
		sent = posix_socket_send(cm->socket, data, (int)(size > 65536 ? 65536 : size), 0);
		if (sent <= 0)
			return 0;
		data += sent;
		size -= (size_t)sent;
	}
	return 1;
}

static int receive_all(struct steam_cm *cm, unsigned char *data, size_t size, unsigned long milliseconds)
{
	while (size)
	{
		int got;
		int ready = socket_wait(cm->socket, milliseconds, 0);

		if (ready <= 0)
			return ready;
		got = posix_socket_recv(cm->socket, data, (int)(size > 65536 ? 65536 : size), 0);
		if (got <= 0)
			return -1;
		data += got;
		size -= (size_t)got;
		/* (the rest of a packet begun comes soon) */
		milliseconds = ANSWER_MILLISECONDS;
	}
	return 1;
}

/* a packet's bytes out, encrypted once the channel is */
static int cm_send_packet(struct steam_cm *cm, unsigned char const *data, size_t size)
{
	unsigned char *frame;
	size_t payload = cm->encrypted ? steam_aes_encrypted_size(size) : size;
	int ok;

	if (!cm->connected)
		return 0;
	frame = malloc(8 + payload);
	if (!frame)
		return 0;
	frame[0] = (unsigned char)payload; frame[1] = (unsigned char)(payload >> 8);
	frame[2] = (unsigned char)(payload >> 16); frame[3] = (unsigned char)(payload >> 24);
	frame[4] = 'V'; frame[5] = 'T'; frame[6] = '0'; frame[7] = '1';
	if (cm->encrypted)
		steam_aes_encrypt_hmac(&cm->aes, cm->hmac_key, data, size, frame + 8);
	else
		memcpy(frame + 8, data, size);
	ok = send_all(cm, frame, 8 + payload);
	free(frame);
	if (!ok)
	{
		cm_fail(cm, "the connection to Steam was lost%s", NULL);
		cm_drop_connection(cm);
	}
	else
		cm->last_sent = steam_milliseconds();
	return ok;
}

/* the next packet's bytes (malloc'd), decrypted: 1, 0 when none came in
time, -1 when the connection failed */
static int cm_receive_packet(struct steam_cm *cm, unsigned long milliseconds, unsigned char **data, size_t *size)
{
	unsigned char head[8];
	uint32_t length;
	unsigned char *bytes;
	int result;

	*data = NULL;
	*size = 0;
	if (!cm->connected)
		return -1;
	result = receive_all(cm, head, 8, milliseconds);
	if (result <= 0)
	{
		if (result < 0)
		{
			cm_fail(cm, "the connection to Steam closed%s", NULL);
			cm_drop_connection(cm);
		}
		return result;
	}
	length = (uint32_t)head[0] | (uint32_t)head[1] << 8 | (uint32_t)head[2] << 16 | (uint32_t)head[3] << 24;
	if (memcmp(head + 4, "VT01", 4) || length > MAXIMUM_PACKET)
	{
		cm_fail(cm, "Steam sent something that is not a packet%s", NULL);
		cm_drop_connection(cm);
		return -1;
	}
	bytes = malloc(length ? length : 1);
	if (!bytes || receive_all(cm, bytes, length, ANSWER_MILLISECONDS) <= 0)
	{
		free(bytes);
		cm_fail(cm, "the connection to Steam closed%s", NULL);
		cm_drop_connection(cm);
		return -1;
	}
	if (cm->encrypted)
	{
		size_t plain;

		if (!steam_aes_decrypt_hmac(&cm->aes, cm->hmac_key, bytes, length, bytes, &plain))
		{
			free(bytes);
			cm_fail(cm, "a packet from Steam failed its check%s", NULL);
			cm_drop_connection(cm);
			return -1;
		}
		length = (uint32_t)plain;
	}
	*data = bytes;
	*size = length;
	return 1;
}

/* ---------- messages */

static uint32_t read32(unsigned char const *bytes)
{
	return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 | (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
}

static uint64_t read64(unsigned char const *bytes)
{
	return (uint64_t)read32(bytes) | (uint64_t)read32(bytes + 4) << 32;
}

static void write32(unsigned char *bytes, uint32_t value)
{
	bytes[0] = (unsigned char)value; bytes[1] = (unsigned char)(value >> 8);
	bytes[2] = (unsigned char)(value >> 16); bytes[3] = (unsigned char)(value >> 24);
}

/* a message with a protocol-buffer header; job_source and job_name are the
request's (JOB_NONE, NULL for none) */
static int cm_send(struct steam_cm *cm, uint32_t emsg, uint64_t job_source, char const *job_name,
	struct steam_pb const *body)
{
	struct steam_pb header;
	struct steam_pb packet;
	unsigned char head[8];
	int ok;

	memset(&header, 0, sizeof(header));
	memset(&packet, 0, sizeof(packet));
	steam_pb_fixed64(&header, 1, cm->steam_id);
	steam_pb_varint(&header, 2, (uint64_t)(uint32_t)cm->session_id);
	if (job_source != JOB_NONE)
		steam_pb_fixed64(&header, 10, job_source);
	if (job_name)
		steam_pb_string(&header, 12, job_name);
	write32(head, emsg | PROTO_MASK);
	write32(head + 4, (uint32_t)header.size);
	steam_pb_raw(&packet, head, 8);
	steam_pb_raw(&packet, header.bytes, header.size);
	if (body && body->size)
		steam_pb_raw(&packet, body->bytes, body->size);
	ok = !header.failed && !packet.failed && !(body && body->failed) && cm_send_packet(cm, packet.bytes, packet.size);
	steam_pb_free(&header);
	steam_pb_free(&packet);
	return ok;
}

static void heartbeat_if_due(struct steam_cm *cm)
{
	if (cm->logged_on && cm->heartbeat_milliseconds &&
		steam_milliseconds() - cm->last_sent >= cm->heartbeat_milliseconds)
	{
		struct steam_pb none;

		memset(&none, 0, sizeof(none));
		cm_send(cm, EMSG_CLIENT_HEARTBEAT, JOB_NONE, NULL, &none);
	}
}

static int gunzip(unsigned char const *data, size_t size, size_t unzipped, unsigned char **out)
{
	z_stream stream;
	int result;

	*out = malloc(unzipped ? unzipped : 1);
	if (!*out)
		return 0;
	memset(&stream, 0, sizeof(stream));
	if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK)
	{
		free(*out);
		return 0;
	}
	stream.next_in = (Bytef *)data;
	stream.avail_in = (uInt)size;
	stream.next_out = *out;
	stream.avail_out = (uInt)unzipped;
	result = inflate(&stream, Z_FINISH);
	inflateEnd(&stream);
	if (result != Z_STREAM_END || stream.total_out != unzipped)
	{
		free(*out);
		*out = NULL;
		return 0;
	}
	return 1;
}

static void queue_add(struct steam_cm *cm, struct cm_message *message)
{
	struct cm_message **end = &cm->queued;

	while (*end)
		end = &(*end)->next;
	message->next = NULL;
	*end = message;
}

/* a packet's message (its bytes taken), or NULL when it is not one; a
Multi's messages queued, NULL returned for it */
static struct cm_message *cm_parse(struct steam_cm *cm, unsigned char *bytes, size_t size, int depth)
{
	struct cm_message *message;
	uint32_t raw;

	if (size < 4)
	{
		wipe_free(bytes, size);
		return NULL;
	}
	message = calloc(1, sizeof(*message));
	if (!message)
	{
		wipe_free(bytes, size);
		return NULL;
	}
	raw = read32(bytes);
	message->emsg = raw & ~PROTO_MASK;
	message->job_target = JOB_NONE;
	message->eresult = STEAM_RESULT_OK;
	if (raw & PROTO_MASK)
	{
		uint32_t header_size;
		struct steam_pb_reader reader;
		int field, wire;
		uint64_t value;
		unsigned char const *data;
		size_t data_size;

		if (size < 8 || (header_size = read32(bytes + 4)) > size - 8)
		{
			wipe_free(bytes, size);
			free(message);
			return NULL;
		}
		reader.at = bytes + 8;
		reader.end = bytes + 8 + header_size;
		reader.failed = 0;
		while (steam_pb_next(&reader, &field, &wire, &value, &data, &data_size))
		{
			if (field == 1) message->steam_id = value;
			else if (field == 2) message->session_id = (int32_t)value;
			else if (field == 11) message->job_target = value;
			else if (field == 13) message->eresult = (int32_t)value;
		}
		message->body_size = size - 8 - header_size;
		message->body = malloc(message->body_size + 1);
		if (message->body)
			memcpy(message->body, bytes + 8 + header_size, message->body_size);
	}
	else
	{
		/* (the old header: EMsg, target job, source job) */
		if (size < 20)
		{
			wipe_free(bytes, size);
			free(message);
			return NULL;
		}
		message->job_target = read64(bytes + 4);
		message->body_size = size - 20;
		message->body = malloc(message->body_size + 1);
		if (message->body)
			memcpy(message->body, bytes + 20, message->body_size);
	}
	wipe_free(bytes, size);
	if (!message->body)
	{
		free(message);
		return NULL;
	}
	if (message->emsg == EMSG_MULTI && depth < 4)
	{
		struct steam_pb_reader reader;
		int field, wire;
		uint64_t value;
		unsigned char const *data;
		size_t data_size;
		uint64_t unzipped = 0;
		unsigned char const *inner = NULL;
		size_t inner_size = 0;
		unsigned char *expanded = NULL;
		size_t at;

		reader.at = message->body;
		reader.end = message->body + message->body_size;
		reader.failed = 0;
		while (steam_pb_next(&reader, &field, &wire, &value, &data, &data_size))
		{
			if (field == 1) unzipped = value;
			else if (field == 2) { inner = data; inner_size = data_size; }
		}
		if (inner && unzipped)
		{
			if (unzipped > MAXIMUM_PACKET || !gunzip(inner, inner_size, (size_t)unzipped, &expanded))
			{
				message_free(message);
				return NULL;
			}
			inner = expanded;
			inner_size = (size_t)unzipped;
		}
		for (at = 0; inner && at + 4 <= inner_size;)
		{
			uint32_t length = read32(inner + at);
			unsigned char *copy;
			struct cm_message *part;

			if (length > inner_size - at - 4)
				break;
			copy = malloc(length ? length : 1);
			if (!copy)
				break;
			memcpy(copy, inner + at + 4, length);
			part = cm_parse(cm, copy, length, depth + 1);
			if (part)
				queue_add(cm, part);
			at += 4 + length;
		}
		wipe_free(expanded, expanded ? inner_size : 0);
		message_free(message);
		return NULL;
	}
	return message;
}

/* what every message does to the session itself */
static void cm_note(struct steam_cm *cm, struct cm_message const *message)
{
	if (message->emsg == EMSG_CLIENT_LOGON_RESPONSE)
	{
		struct steam_pb_reader reader;
		int field, wire;
		uint64_t value;
		unsigned char const *data;
		size_t size;
		int32_t result = STEAM_RESULT_FAIL;
		int32_t heartbeat = 9;

		reader.at = message->body;
		reader.end = message->body + message->body_size;
		reader.failed = 0;
		while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
		{
			if (field == 1) result = (int32_t)value;
			else if (field == 2) heartbeat = (int32_t)value;
		}
		if (result == STEAM_RESULT_OK)
		{
			cm->steam_id = message->steam_id;
			cm->session_id = message->session_id;
			cm->logged_on = 1;
			cm->heartbeat_milliseconds = (unsigned long)(heartbeat > 0 ? heartbeat : 9) * 1000;
		}
	}
	else if (message->emsg == EMSG_CLIENT_LOGGED_OFF)
	{
		cm->logged_on = 0;
		cm_fail(cm, "Steam signed this session off%s", NULL);
	}
}

/* the next message of the kind emsg (0: any) answering job (JOB_NONE: any),
within milliseconds; NULL on a timeout or a failure */
static struct cm_message *cm_wait(struct steam_cm *cm, uint32_t emsg, uint64_t job, unsigned long milliseconds)
{
	unsigned long started = steam_milliseconds();

	for (;;)
	{
		struct cm_message *message = NULL;
		unsigned long waited = steam_milliseconds() - started;

		heartbeat_if_due(cm);
		if (cm->queued)
		{
			message = cm->queued;
			cm->queued = message->next;
		}
		else
		{
			unsigned char *bytes;
			size_t size;
			int result;

			if (waited >= milliseconds)
			{
				cm_fail(cm, "Steam did not answer%s", NULL);
				return NULL;
			}
			result = cm_receive_packet(cm, SOONER(milliseconds - waited, 1000UL), &bytes, &size);
			if (result < 0)
				return NULL;
			if (!result)
				continue;
			message = cm_parse(cm, bytes, size, 0);
			if (!message)
				continue;
		}
		cm_note(cm, message);
		if ((!emsg || message->emsg == emsg) && (job == JOB_NONE || message->job_target == job))
			return message;
		message_free(message);
	}
}

/* a request answered under its job's number */
static struct cm_message *cm_job(struct steam_cm *cm, uint32_t emsg, char const *job_name, struct steam_pb const *body,
	uint32_t answer)
{
	uint64_t job = cm->next_job++;

	if (!cm_send(cm, emsg, job, job_name, body))
		return NULL;
	return cm_wait(cm, answer, job, ANSWER_MILLISECONDS);
}

/* one of Steam's services' methods ("Service.Method#1"): its response, NULL
with the error said on a failure */
static struct cm_message *cm_call(struct steam_cm *cm, char const *method, struct steam_pb const *request)
{
	struct cm_message *answer = cm_job(cm, cm->logged_on ? EMSG_SERVICE_METHOD_CALL_FROM_CLIENT :
		EMSG_SERVICE_METHOD_CALL_FROM_CLIENT_NON_AUTHED, method, request, EMSG_SERVICE_METHOD_RESPONSE);

	if (answer && answer->eresult != STEAM_RESULT_OK)
	{
		char text[64];

		snprintf(text, sizeof(text), "%.40s (result %d)", method, (int)answer->eresult);
		cm_fail(cm, "Steam refused %s", text);
		message_free(answer);
		return NULL;
	}
	return answer;
}

/* ---------- the connection */

/* up to MAXIMUM_SERVERS of the directory's TCP servers, "a.b.c.d:port" */
static int cm_servers(char (*servers)[64], int capacity, char *error, int error_size)
{
	unsigned char *body;
	unsigned long size;
	int status;
	int count = 0;
	char const *at;

	if (!steam_https_get("https://api.steampowered.com/ISteamDirectory/GetCMListForConnect/v1/?cellid=0&cmtype=netfilter",
		256 * 1024, &body, &size, &status, error, error_size))
		return 0;
	if (status != 200)
	{
		snprintf(error, (size_t)error_size, "Steam's directory answered %d", status);
		free(body);
		return 0;
	}
	/* (each server's "endpoint":"host:port" of type netfilter) */
	for (at = (char const *)body; count < capacity && (at = strstr(at, "\"endpoint\":\"")) != NULL;)
	{
		char const *start = at + 12;
		char const *end = strchr(start, '"');
		char const *type = strstr(start, "\"type\":\"");

		at = start;
		if (!end || end - start >= 64 || !type || strncmp(type + 8, "netfilter", 9))
			continue;
		memcpy(servers[count], start, (size_t)(end - start));
		servers[count][end - start] = 0;
		count++;
	}
	free(body);
	if (!count)
		snprintf(error, (size_t)error_size, "Steam's directory listed no server");
	return count;
}

static int cm_connect_to(struct steam_cm *cm, char const *server)
{
	char host[64];
	char const *colon = strrchr(server, ':');
	struct steam_sockaddr address;
	posix_ulong ip;
	int port;

	if (!colon || colon - server >= (long)sizeof(host))
		return 0;
	memcpy(host, server, (size_t)(colon - server));
	host[colon - server] = 0;
	port = atoi(colon + 1);
	ip = posix_resolve_ipv4(host);
	if (!ip || port <= 0 || port > 65535)
		return 0;
	cm->socket = posix_socket(2 /* AF_INET */, 1 /* SOCK_STREAM */, 0);
	if (cm->socket < 0)
		return 0;
	steam_address(&address, (uint32_t)ip, port);
	posix_socket_set_nonblocking(cm->socket, 1);
	if (posix_socket_connect(cm->socket, &address, sizeof(address)) != 0)
	{
		int error = 0;
		int length = sizeof(error);

		/* (in progress: done when it can be written to, if no error) */
		if (socket_wait(cm->socket, CONNECT_MILLISECONDS, 1) <= 0 ||
			posix_socket_getsockopt(cm->socket, WINSOCK_SOL_SOCKET, WINSOCK_SO_ERROR, &error, &length) != 0 || error)
		{
			posix_socket_close(cm->socket);
			cm->socket = -1;
			return 0;
		}
	}
	posix_socket_set_nonblocking(cm->socket, 0);
	posix_socket_set_nodelay(cm->socket);
	cm->connected = 1;
	return 1;
}

/* the channel's handshake: 1 when it is encrypted */
static int cm_handshake(struct steam_cm *cm)
{
	unsigned char *bytes;
	size_t size;
	uint32_t universe;
	unsigned char challenge[16];
	unsigned char session_key[32];
	unsigned char blob[48];
	unsigned char response[20 + 8 + 128 + 8];
	struct cm_message *result;

	if (cm_receive_packet(cm, ANSWER_MILLISECONDS, &bytes, &size) <= 0)
		return 0;
	if (size < 20 + 8 + 16 || read32(bytes) != EMSG_CHANNEL_ENCRYPT_REQUEST)
	{
		free(bytes);
		cm_fail(cm, "Steam did not ask for an encrypted channel%s", NULL);
		return 0;
	}
	universe = read32(bytes + 24);
	memcpy(challenge, bytes + 28, 16);
	free(bytes);
	if (universe != 1)
	{
		cm_fail(cm, "the server is not of Steam's public universe%s", NULL);
		return 0;
	}
	steam_random(session_key, sizeof(session_key));
	memcpy(blob, session_key, 32);
	memcpy(blob + 32, challenge, 16);
	/* ChannelEncryptResponse: the old header, protocol 1, a 128-byte key,
	the blob, its CRC and a zero */
	memset(response, 0, sizeof(response));
	write32(response, EMSG_CHANNEL_ENCRYPT_RESPONSE);
	memset(response + 4, 0xFF, 16);
	write32(response + 20, 1);
	write32(response + 24, 128);
	if (!steam_rsa_encrypt_public_universe(blob, sizeof(blob), response + 28))
	{
		steam_wipe(session_key, sizeof(session_key));
		steam_wipe(blob, sizeof(blob));
		return 0;
	}
	steam_wipe(blob, sizeof(blob));
	write32(response + 28 + 128, steam_crc32(response + 28, 128));
	write32(response + 28 + 128 + 4, 0);
	if (!cm_send_packet(cm, response, sizeof(response)))
		return 0;
	steam_aes_key(&cm->aes, session_key);
	memcpy(cm->hmac_key, session_key, 16);
	steam_wipe(session_key, sizeof(session_key));
	if (cm_receive_packet(cm, ANSWER_MILLISECONDS, &bytes, &size) <= 0)
		return 0;
	result = cm_parse(cm, bytes, size, 0);
	if (!result || result->emsg != EMSG_CHANNEL_ENCRYPT_RESULT || result->body_size < 4 ||
		read32(result->body) != STEAM_RESULT_OK)
	{
		message_free(result);
		cm_fail(cm, "Steam did not accept the channel's key%s", NULL);
		return 0;
	}
	message_free(result);
	cm->encrypted = 1;
	return 1;
}

struct steam_cm *steam_cm_open(char *error, int error_size)
{
	char servers[MAXIMUM_SERVERS][64];
	int count, index;
	struct steam_cm *cm;

	count = cm_servers(servers, MAXIMUM_SERVERS, error, error_size);
	if (!count)
		return NULL;
	cm = calloc(1, sizeof(*cm));
	if (!cm)
	{
		snprintf(error, (size_t)error_size, "out of memory");
		return NULL;
	}
	cm->socket = -1;
	cm->next_job = 1;
	for (index = 0; index < count; index++)
	{
		struct steam_pb hello;

		if (!cm_connect_to(cm, servers[index]))
			continue;
		if (!cm_handshake(cm))
		{
			cm_drop_connection(cm);
			cm->encrypted = 0;
			continue;
		}
		memset(&hello, 0, sizeof(hello));
		steam_pb_varint(&hello, 1, PROTOCOL_VERSION);
		cm_send(cm, EMSG_CLIENT_HELLO, JOB_NONE, NULL, &hello);
		steam_pb_free(&hello);
		steam_log("steam: connected to %s", servers[index]);
		return cm;
	}
	snprintf(error, (size_t)error_size, "%s", cm->error[0] ? cm->error : "no Steam server could be reached");
	free(cm);
	return NULL;
}

void steam_cm_close(struct steam_cm *cm)
{
	if (!cm)
		return;
	cm_drop_connection(cm);
	while (cm->queued)
	{
		struct cm_message *next = cm->queued->next;

		message_free(cm->queued);
		cm->queued = next;
	}
	/* (the channel's key with it) */
	wipe_free(cm, sizeof(*cm));
}

int steam_cm_connected(struct steam_cm const *cm)
{
	return cm && cm->connected;
}

uint64_t steam_cm_steam_id(struct steam_cm const *cm)
{
	return cm ? cm->steam_id : 0;
}

/* ---------- signing in */

static void device_details(struct steam_pb *pb, char const *name)
{
	struct steam_pb details;

	memset(&details, 0, sizeof(details));
	steam_pb_string(&details, 1, name);
	steam_pb_varint(&details, 2, 1); /* EAuthTokenPlatformType_SteamClient */
	steam_pb_varint(&details, 3, CLIENT_OS_TYPE);
	steam_pb_varint(&details, 4, 1);
	steam_pb_bytes(pb, 3, details.bytes, details.size);
	steam_pb_free(&details);
}

int steam_cm_qr_begin(struct steam_cm *cm, char const *device_name, struct steam_qr *qr)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;

	memset(qr, 0, sizeof(*qr));
	memset(&request, 0, sizeof(request));
	steam_pb_string(&request, 1, device_name);
	steam_pb_varint(&request, 2, 1);
	device_details(&request, device_name);
	steam_pb_string(&request, 4, "Client");
	answer = cm_call(cm, "Authentication.BeginAuthSessionViaQR#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return 0;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1) qr->client_id = value;
		else if (field == 2) steam_pb_text(data, size, qr->url, sizeof(qr->url));
		else if (field == 3 && size <= sizeof(qr->request_id)) { memcpy(qr->request_id, data, size); qr->request_id_size = size; }
		else if (field == 4) { uint32_t bits = (uint32_t)value; memcpy(&qr->interval, &bits, 4); }
	}
	message_free(answer);
	if (!(qr->interval >= 1.0f && qr->interval <= 10.0f))
		qr->interval = 5.0f;
	if (!qr->url[0] || !qr->request_id_size)
	{
		cm_fail(cm, "Steam gave no sign-in code%s", NULL);
		return 0;
	}
	return 1;
}

int steam_cm_qr_poll(struct steam_cm *cm, struct steam_qr *qr, char *token, size_t token_size, char *account,
	size_t account_size)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;

	token[0] = 0;
	account[0] = 0;
	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 1, qr->client_id);
	steam_pb_bytes(&request, 2, qr->request_id, qr->request_id_size);
	answer = cm_call(cm, "Authentication.PollAuthSessionStatus#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return -1;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1 && value) qr->client_id = value;
		else if (field == 2 && size) steam_pb_text(data, size, qr->url, sizeof(qr->url));
		else if (field == 3) steam_pb_text(data, size, token, token_size);
		else if (field == 6) steam_pb_text(data, size, account, account_size);
	}
	message_free(answer);
	return token[0] ? 1 : 0;
}

static int hex_value(char ch)
{
	return ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 :
		ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : -1;
}

int steam_cm_password_begin(struct steam_cm *cm, char const *account, char const *password, char const *device_name,
	struct steam_qr *qr, uint64_t *steam_id, unsigned *confirmations)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	char modulus_hex[1100];
	char exponent_hex[32];
	uint64_t timestamp = 0;
	unsigned char modulus[512];
	unsigned char encrypted[512];
	char encoded[700];
	size_t modulus_size = 0, i;
	uint32_t exponent;
	static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t out = 0;

	memset(qr, 0, sizeof(*qr));
	*steam_id = 0;
	*confirmations = 0;
	modulus_hex[0] = exponent_hex[0] = 0;
	/* (the key to encrypt the password with, the account's) */
	memset(&request, 0, sizeof(request));
	steam_pb_string(&request, 1, account);
	answer = cm_call(cm, "Authentication.GetPasswordRSAPublicKey#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return 0;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1) steam_pb_text(data, size, modulus_hex, sizeof(modulus_hex));
		else if (field == 2) steam_pb_text(data, size, exponent_hex, sizeof(exponent_hex));
		else if (field == 3) timestamp = value;
	}
	message_free(answer);
	for (i = 0; modulus_hex[i] && modulus_hex[i + 1] && modulus_size < sizeof(modulus); i += 2)
	{
		int high = hex_value(modulus_hex[i]), low = hex_value(modulus_hex[i + 1]);

		if (high < 0 || low < 0)
			break;
		modulus[modulus_size++] = (unsigned char)(high * 16 + low);
	}
	exponent = (uint32_t)strtoul(exponent_hex, NULL, 16);
	if (modulus_size < 64 || !exponent ||
		!steam_rsa_encrypt_pkcs1(modulus, modulus_size, exponent, (unsigned char const *)password, strlen(password),
			encrypted))
	{
		cm_fail(cm, "Steam gave no key for this account's password%s", NULL);
		return 0;
	}
	while (modulus_size && !modulus[0])
		memmove(modulus, modulus + 1, --modulus_size);
	/* (base64 of the encryption) */
	for (i = 0; i < modulus_size; i += 3)
	{
		uint32_t triple = (uint32_t)encrypted[i] << 16 | (uint32_t)(i + 1 < modulus_size ? encrypted[i + 1] : 0) << 8 |
			(i + 2 < modulus_size ? encrypted[i + 2] : 0);

		encoded[out++] = alphabet[(triple >> 18) & 63];
		encoded[out++] = alphabet[(triple >> 12) & 63];
		encoded[out++] = i + 1 < modulus_size ? alphabet[(triple >> 6) & 63] : '=';
		encoded[out++] = i + 2 < modulus_size ? alphabet[triple & 63] : '=';
	}
	encoded[out] = 0;
	memset(&request, 0, sizeof(request));
	steam_pb_string(&request, 1, device_name);
	steam_pb_string(&request, 2, account);
	steam_pb_string(&request, 3, encoded);
	steam_pb_varint(&request, 4, timestamp);
	steam_pb_varint(&request, 5, 1);
	steam_pb_varint(&request, 6, 1); /* EAuthTokenPlatformType_SteamClient */
	steam_pb_varint(&request, 7, 1); /* ESessionPersistence_Persistent */
	steam_pb_string(&request, 8, "Client");
	{
		struct steam_pb details;

		memset(&details, 0, sizeof(details));
		steam_pb_string(&details, 1, device_name);
		steam_pb_varint(&details, 2, 1);
		steam_pb_varint(&details, 3, CLIENT_OS_TYPE);
		steam_pb_varint(&details, 4, 1);
		steam_pb_bytes(&request, 9, details.bytes, details.size);
		steam_pb_free(&details);
	}
	answer = cm_call(cm, "Authentication.BeginAuthSessionViaCredentials#1", &request);
	steam_pb_free(&request);
	if (!answer)
	{
		/* (EResult 5: the name or the password is not right) */
		if (strstr(cm->error, "result 5)"))
			cm_fail(cm, "Steam did not accept that account name and password%s.", NULL);
		return 0;
	}
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1) qr->client_id = value;
		else if (field == 2 && size <= sizeof(qr->request_id)) { memcpy(qr->request_id, data, size); qr->request_id_size = size; }
		else if (field == 3) { uint32_t bits = (uint32_t)value; memcpy(&qr->interval, &bits, 4); }
		else if (field == 5) *steam_id = value;
		else if (field == 4 && data)
		{
			struct steam_pb_reader inner;
			int inner_field, inner_wire;
			uint64_t inner_value;
			unsigned char const *inner_data;
			size_t inner_size;

			inner.at = data;
			inner.end = data + size;
			inner.failed = 0;
			while (steam_pb_next(&inner, &inner_field, &inner_wire, &inner_value, &inner_data, &inner_size))
				if (inner_field == 1 && inner_value < 32)
					*confirmations |= 1u << inner_value;
		}
	}
	message_free(answer);
	if (!(qr->interval >= 1.0f && qr->interval <= 10.0f))
		qr->interval = 5.0f;
	if (!qr->request_id_size)
	{
		cm_fail(cm, "Steam did not begin the sign-in%s", NULL);
		return 0;
	}
	return 1;
}

int steam_cm_guard_code(struct steam_cm *cm, struct steam_qr const *qr, uint64_t steam_id, char const *code, int email)
{
	struct steam_pb request;
	struct cm_message *answer;

	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 1, qr->client_id);
	steam_pb_fixed64(&request, 2, steam_id);
	steam_pb_string(&request, 3, code);
	steam_pb_varint(&request, 4, email ? 2 : 3);
	answer = cm_call(cm, "Authentication.UpdateAuthSessionWithSteamGuardCode#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return STEAM_RESULT_FAIL;
	message_free(answer);
	return STEAM_RESULT_OK;
}

int steam_cm_revoke_token(struct steam_cm *cm, char const *token)
{
	struct steam_pb request;
	struct cm_message *answer;

	/* (CAuthentication_Token_Revoke_Request: the token, and
	k_EAuthTokenRevokePermanent; Steam then refuses it, wherever a copy of
	it is) */
	memset(&request, 0, sizeof(request));
	steam_pb_string(&request, 1, token);
	steam_pb_varint(&request, 2, 1);
	answer = cm_call(cm, "Authentication.RevokeToken#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return STEAM_RESULT_FAIL;
	message_free(answer);
	return STEAM_RESULT_OK;
}

/* a JWT's subject (the account's Steam id) */
static uint64_t token_steam_id(char const *token)
{
	char const *first = strchr(token, '.');
	char const *second = first ? strchr(first + 1, '.') : NULL;
	unsigned char json[1024];
	long length;
	char const *sub;

	if (!second)
		return 0;
	length = steam_base64_decode(first + 1, (size_t)(second - first - 1), json, sizeof(json) - 1);
	if (length <= 0)
		return 0;
	json[length] = 0;
	sub = strstr((char const *)json, "\"sub\"");
	if (!sub)
		return 0;
	sub = strchr(sub + 5, '"');
	return sub ? strtoull(sub + 1, NULL, 10) : 0;
}

size_t steam_cm_machine_id(unsigned char const seed[16], unsigned char *out, size_t capacity)
{
	/* a binary key-value object, MessageObject, of three strings: the
	SHA-1s (hex) of three facts of the machine, as SteamKit makes them;
	here of this install's seed */
	static char const *const names[3] = { "BB3", "FF2", "3B3" };
	size_t at = 0;
	int i;

	if (capacity < 16 + 3 * (5 + 41) + 2)
		return 0;
	out[at++] = 0;
	memcpy(out + at, "MessageObject", 14);
	at += 14;
	for (i = 0; i < 3; i++)
	{
		unsigned char input[17];
		unsigned char digest[20];

		memcpy(input, seed, 16);
		input[16] = (unsigned char)i;
		steam_sha1(input, sizeof(input), digest);
		out[at++] = 1;
		memcpy(out + at, names[i], 4);
		at += 4;
		steam_hex(digest, 20, (char *)out + at);
		at += 41;
	}
	out[at++] = 8;
	out[at++] = 8;
	return at;
}

static int cm_logon(struct steam_cm *cm, struct steam_pb const *body)
{
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	int32_t result = STEAM_RESULT_FAIL;

	cm->session_id = 0;
	if (!cm_send(cm, EMSG_CLIENT_LOGON, JOB_NONE, NULL, body))
		return STEAM_RESULT_FAIL;
	answer = cm_wait(cm, EMSG_CLIENT_LOGON_RESPONSE, JOB_NONE, ANSWER_MILLISECONDS);
	if (!answer)
		return STEAM_RESULT_FAIL;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
		if (field == 1)
			result = (int32_t)value;
	message_free(answer);
	if (result != STEAM_RESULT_OK)
	{
		char text[32];

		snprintf(text, sizeof(text), "%d", (int)result);
		cm_fail(cm, "Steam refused the sign-in (result %s)", text);
	}
	return result;
}

int steam_cm_logon_token(struct steam_cm *cm, char const *token, char const *machine_name,
	unsigned char const *machine_id, size_t machine_id_size)
{
	struct steam_pb logon;
	struct steam_pb address;
	int result;

	cm->steam_id = token_steam_id(token);
	if (!cm->steam_id)
	{
		cm_fail(cm, "the saved Steam sign-in is not readable%s", NULL);
		return STEAM_RESULT_FAIL;
	}
	memset(&logon, 0, sizeof(logon));
	memset(&address, 0, sizeof(address));
	steam_pb_varint(&logon, 1, PROTOCOL_VERSION);
	steam_pb_varint(&logon, 3, 0);
	steam_pb_varint(&logon, 5, 1771);
	steam_pb_string(&logon, 6, "english");
	steam_pb_varint(&logon, 7, CLIENT_OS_TYPE);
	steam_pb_varint(&logon, 8, 1);
	/* (a private address, obfuscated as Steam's client does: none here) */
	steam_pb_fixed32(&address, 1, 0xBAADF00Du);
	steam_pb_bytes(&logon, 11, address.bytes, address.size);
	if (machine_id && machine_id_size)
		steam_pb_bytes(&logon, 30, machine_id, machine_id_size);
	steam_pb_varint(&logon, 33, 2);
	steam_pb_string(&logon, 96, machine_name);
	steam_pb_varint(&logon, 102, 1);
	steam_pb_string(&logon, 108, token);
	result = cm_logon(cm, &logon);
	steam_pb_free(&logon);
	steam_pb_free(&address);
	return result;
}

int steam_cm_logon_anonymous(struct steam_cm *cm)
{
	struct steam_pb logon;
	int result;

	cm->steam_id = 0x01A0000000000000ull;
	memset(&logon, 0, sizeof(logon));
	steam_pb_varint(&logon, 1, PROTOCOL_VERSION);
	steam_pb_varint(&logon, 3, 0);
	steam_pb_string(&logon, 6, "english");
	steam_pb_varint(&logon, 7, CLIENT_OS_TYPE);
	steam_pb_varint(&logon, 102, 1);
	result = cm_logon(cm, &logon);
	steam_pb_free(&logon);
	return result;
}

/* ---------- content */

char *steam_cm_app_info(struct steam_cm *cm, uint32_t app)
{
	struct steam_pb request;
	struct steam_pb entry;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	uint64_t access_token = 0;
	char *text = NULL;
	uint64_t job;
	int pending = 1;

	/* (the app's access token first, which a licence may give) */
	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 2, app);
	answer = cm_job(cm, EMSG_CLIENT_PICS_ACCESS_TOKEN_REQUEST, NULL, &request, EMSG_CLIENT_PICS_ACCESS_TOKEN_RESPONSE);
	steam_pb_free(&request);
	if (answer)
	{
		reader.at = answer->body;
		reader.end = answer->body + answer->body_size;
		reader.failed = 0;
		while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
		{
			if (field == 3 && data)
			{
				struct steam_pb_reader inner;
				int inner_field, inner_wire;
				uint64_t inner_value;
				unsigned char const *inner_data;
				size_t inner_size;

				inner.at = data;
				inner.end = data + size;
				inner.failed = 0;
				while (steam_pb_next(&inner, &inner_field, &inner_wire, &inner_value, &inner_data, &inner_size))
					if (inner_field == 2)
						access_token = inner_value;
			}
		}
		message_free(answer);
	}
	memset(&request, 0, sizeof(request));
	memset(&entry, 0, sizeof(entry));
	steam_pb_varint(&entry, 1, app);
	if (access_token)
		steam_pb_varint(&entry, 2, access_token);
	steam_pb_bytes(&request, 2, entry.bytes, entry.size);
	steam_pb_varint(&request, 3, 0);
	job = cm->next_job++;
	if (!cm_send(cm, EMSG_CLIENT_PICS_PRODUCT_INFO_REQUEST, job, NULL, &request))
		pending = 0;
	steam_pb_free(&request);
	steam_pb_free(&entry);
	/* (an answer in parts, the last without response_pending) */
	while (pending && (answer = cm_wait(cm, EMSG_CLIENT_PICS_PRODUCT_INFO_RESPONSE, job, ANSWER_MILLISECONDS)) != NULL)
	{
		pending = 0;
		reader.at = answer->body;
		reader.end = answer->body + answer->body_size;
		reader.failed = 0;
		while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
		{
			if (field == 6)
				pending = value != 0;
			else if (field == 1 && data && !text)
			{
				struct steam_pb_reader inner;
				int inner_field, inner_wire;
				uint64_t inner_value;
				unsigned char const *inner_data;
				size_t inner_size;

				inner.at = data;
				inner.end = data + size;
				inner.failed = 0;
				while (steam_pb_next(&inner, &inner_field, &inner_wire, &inner_value, &inner_data, &inner_size))
				{
					if (inner_field == 5 && inner_data && (text = malloc(inner_size + 1)) != NULL)
					{
						memcpy(text, inner_data, inner_size);
						text[inner_size] = 0;
					}
				}
			}
		}
		message_free(answer);
	}
	if (!text)
		cm_fail(cm, "Steam gave no information about the game%s", NULL);
	return text;
}

int steam_cm_depot_key(struct steam_cm *cm, uint32_t depot, uint32_t app, unsigned char key[32])
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	int32_t result = STEAM_RESULT_FAIL;
	int have = 0;

	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 1, depot);
	steam_pb_varint(&request, 2, app);
	answer = cm_job(cm, EMSG_CLIENT_GET_DEPOT_DECRYPTION_KEY, NULL, &request,
		EMSG_CLIENT_GET_DEPOT_DECRYPTION_KEY_RESPONSE);
	steam_pb_free(&request);
	if (!answer)
		return STEAM_RESULT_FAIL;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1) result = (int32_t)value;
		else if (field == 3 && size == 32) { memcpy(key, data, 32); have = 1; }
	}
	message_free(answer);
	return result == STEAM_RESULT_OK && !have ? STEAM_RESULT_FAIL : result;
}

uint64_t steam_cm_manifest_code(struct steam_cm *cm, uint32_t app, uint32_t depot, uint64_t manifest)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	uint64_t code = 0;

	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 1, app);
	steam_pb_varint(&request, 2, depot);
	steam_pb_varint(&request, 3, manifest);
	steam_pb_string(&request, 4, "public");
	answer = cm_call(cm, "ContentServerDirectory.GetManifestRequestCode#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return 0;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
		if (field == 1)
			code = value;
	message_free(answer);
	if (!code)
		cm_fail(cm, "Steam gave no code for the files' list (not owned?)%s", NULL);
	return code;
}

/* ---------- the Workshop */

static void file_read(unsigned char const *data, size_t size, struct steam_workshop_file *file)
{
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *bytes;
	size_t length;
	char short_description[sizeof(file->description)];

	memset(file, 0, sizeof(*file));
	short_description[0] = 0;
	reader.at = data;
	reader.end = data + size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &bytes, &length))
	{
		switch (field)
		{
		case 2: file->id = value; break;
		case 8: file->size = value; break;
		case 11: steam_pb_text(bytes, length, file->preview_url, sizeof(file->preview_url)); break;
		case 14: file->manifest = value; break;
		case 16: steam_pb_text(bytes, length, file->title, sizeof(file->title)); break;
		case 17: steam_pb_text(bytes, length, file->description, sizeof(file->description)); break;
		case 18: steam_pb_text(bytes, length, short_description, sizeof(short_description)); break;
		case 20: file->updated = (uint32_t)value; break;
		case 36: file->subscriptions = (uint32_t)value; break;
		case 52:
		{
			struct steam_pb_reader inner;
			int inner_field, inner_wire;
			uint64_t inner_value;
			unsigned char const *inner_bytes;
			size_t inner_length;

			inner.at = bytes;
			inner.end = bytes + length;
			inner.failed = 0;
			while (bytes && steam_pb_next(&inner, &inner_field, &inner_wire, &inner_value, &inner_bytes, &inner_length))
			{
				size_t used = strlen(file->tags);

				if (inner_field == 1 && inner_length && used + inner_length + 2 < sizeof(file->tags))
				{
					if (used)
						file->tags[used++] = ',';
					memcpy(file->tags + used, inner_bytes, inner_length);
					file->tags[used + inner_length] = 0;
				}
			}
			break;
		}
		default: break;
		}
	}
	if (short_description[0])
		memcpy(file->description, short_description, sizeof(file->description));
}

int steam_cm_workshop_query(struct steam_cm *cm, uint32_t app, char const *tags, char const *search,
	enum steam_workshop_sort sort, int page, struct steam_workshop_file *files, int capacity, uint32_t *total)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	int count = 0;
	char tag[64];
	char const *at;

	*total = 0;
	memset(&request, 0, sizeof(request));
	steam_pb_varint(&request, 1, search && search[0] ? STEAM_WORKSHOP_SEARCH : (uint64_t)sort);
	steam_pb_varint(&request, 2, (uint64_t)(page < 1 ? 1 : page));
	steam_pb_varint(&request, 3, (uint64_t)capacity);
	steam_pb_varint(&request, 5, app);
	for (at = tags; at && *at;)
	{
		char const *end = strchr(at, ',');
		size_t length = end ? (size_t)(end - at) : strlen(at);

		if (length && length < sizeof(tag))
		{
			memcpy(tag, at, length);
			tag[length] = 0;
			steam_pb_string(&request, 6, tag);
		}
		at = end ? end + 1 : NULL;
	}
	if (search && search[0])
		steam_pb_string(&request, 11, search);
	steam_pb_varint(&request, 18, 1);
	steam_pb_varint(&request, 22, 1);
	steam_pb_varint(&request, 37, 1);
	steam_pb_varint(&request, 38, 1);
	answer = cm_call(cm, "PublishedFile.QueryFiles#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return -1;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1)
			*total = (uint32_t)value;
		else if (field == 2 && data && count < capacity)
			file_read(data, size, &files[count++]);
	}
	message_free(answer);
	return count;
}

int steam_cm_workshop_details(struct steam_cm *cm, uint64_t id, struct steam_workshop_file *file)
{
	struct steam_pb request;
	struct cm_message *answer;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *data;
	size_t size;
	int found = 0;

	memset(&request, 0, sizeof(request));
	steam_pb_fixed64(&request, 1, id);
	steam_pb_varint(&request, 8, 1);
	steam_pb_varint(&request, 15, 1);
	answer = cm_call(cm, "PublishedFile.GetDetails#1", &request);
	steam_pb_free(&request);
	if (!answer)
		return 0;
	reader.at = answer->body;
	reader.end = answer->body + answer->body_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &data, &size))
	{
		if (field == 1 && data && !found)
		{
			file_read(data, size, file);
			found = file->id == id;
		}
	}
	message_free(answer);
	return found;
}

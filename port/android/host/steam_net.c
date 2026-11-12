/*
STEAM_NET.C

steam_net.h's: the HTTPS GET (the host's HTTPS client, posix_update.c's
update_https_request, with verified certificates), plain TCP and HTTP over
the host's posix_socket calls, the clock, the wipe, and protocol buffers.
In the Android app's host (port/android/host).
*/

#include "steam_net.h"
#include "host.h"
#include "posix.h"
#include "../../linux/src/update.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

unsigned long steam_milliseconds(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (unsigned long)now.tv_sec * 1000ul + (unsigned long)(now.tv_nsec / 1000000);
}

void steam_wipe(void *memory, size_t size)
{
	/* (through a volatile pointer: a store the compiler may not drop as dead) */
	volatile unsigned char *at = memory;

	while (size--)
		*at++ = 0;
}

int steam_https_get(char const *url, unsigned long maximum, unsigned char **body, unsigned long *size, int *status,
	char *error, int error_size)
{
	struct update_response response;
	unsigned char *ended;

	*body = NULL;
	*size = 0;
	*status = 0;
	error[0] = 0;
	if (maximum >= 0x7fffffff)
	{
		snprintf(error, (size_t)error_size, "too large a body asked for");
		return 0;
	}
	if (!update_https_request("GET", url, NULL, NULL, 0, maximum, &response, error, error_size))
		return 0;
	/* (with a terminator after it: a body is often parsed as text) */
	ended = realloc(response.body, response.body_size + 1);
	if (!ended)
	{
		free(response.body);
		snprintf(error, (size_t)error_size, "out of memory");
		return 0;
	}
	ended[response.body_size] = 0;
	*body = ended;
	*size = response.body_size;
	*status = response.status;
	return 1;
}

/* ---------- plain TCP and HTTP */

/* (posix_socket_getsockopt takes Winsock's level and names, and the
address calls Winsock's sockaddr_in: posix_net.c) */
#define WINSOCK_SOL_SOCKET 0xffff
#define WINSOCK_SO_ERROR 0x1007

void steam_address(struct steam_sockaddr *address, uint32_t ip, int port)
{
	memset(address, 0, sizeof(*address));
	address->family = 2; /* AF_INET */
	address->port = (uint16_t)(((port & 255) << 8) | ((port >> 8) & 255));
	address->ip = ip;
}
#define HTTP_MILLISECONDS 20000

int steam_socket_wait(int socket, unsigned long milliseconds, int writing)
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

int steam_tcp_connect(char const *host, int port, unsigned long milliseconds)
{
	struct steam_sockaddr address;
	posix_ulong ip = posix_resolve_ipv4(host);
	int socket;

	if (!ip || port <= 0 || port > 65535)
		return -1;
	socket = posix_socket(2 /* AF_INET */, 1 /* SOCK_STREAM */, 0);
	if (socket < 0)
		return -1;
	steam_address(&address, (uint32_t)ip, port);
	posix_socket_set_nonblocking(socket, 1);
	if (posix_socket_connect(socket, &address, sizeof(address)) != 0)
	{
		int error = 0;
		int length = sizeof(error);

		/* (in progress: done when it can be written to, if no error) */
		if (steam_socket_wait(socket, milliseconds, 1) <= 0 ||
			posix_socket_getsockopt(socket, WINSOCK_SOL_SOCKET, WINSOCK_SO_ERROR, &error, &length) != 0 || error)
		{
			posix_socket_close(socket);
			return -1;
		}
	}
	posix_socket_set_nonblocking(socket, 0);
	posix_socket_set_nodelay(socket);
	return socket;
}

void steam_http_init(struct steam_http *http)
{
	http->socket = -1;
	http->host[0] = 0;
}

void steam_http_close(struct steam_http *http)
{
	if (http->socket >= 0)
		posix_socket_close(http->socket);
	steam_http_init(http);
}

static int http_send(int socket, char const *data, size_t size)
{
	while (size)
	{
		int sent;

		if (steam_socket_wait(socket, HTTP_MILLISECONDS, 1) <= 0)
			return 0;
		sent = posix_socket_send(socket, data, (int)size, 0);
		if (sent <= 0)
			return 0;
		data += sent;
		size -= (size_t)sent;
	}
	return 1;
}

/* some bytes into buffer (at most size): the count, 0 at the end, -1 on a failure */
static int http_read(int socket, char *buffer, size_t size)
{
	if (steam_socket_wait(socket, HTTP_MILLISECONDS, 0) <= 0)
		return -1;
	return posix_socket_recv(socket, buffer, (int)size, 0);
}

static int strncasecmp_portable(char const *a, char const *b, size_t length)
{
	size_t i;

	for (i = 0; i < length; i++)
	{
		int x = a[i] >= 'A' && a[i] <= 'Z' ? a[i] + 32 : a[i];
		int y = b[i] >= 'A' && b[i] <= 'Z' ? b[i] + 32 : b[i];

		if (x != y)
			return x - y;
		if (!x)
			return 0;
	}
	return 0;
}

/* a header's value as a number (-1 when absent) */
static long header_number(char const *headers, char const *name)
{
	char const *at = headers;
	size_t name_length = strlen(name);

	while ((at = strchr(at, '\n')) != NULL)
	{
		at++;
		if (!strncasecmp_portable(at, name, name_length) && at[name_length] == ':')
			return strtol(at + name_length + 1, NULL, 10);
	}
	return -1;
}

static int header_has(char const *headers, char const *name, char const *value)
{
	char const *at = headers;
	size_t name_length = strlen(name);

	while ((at = strchr(at, '\n')) != NULL)
	{
		at++;
		if (!strncasecmp_portable(at, name, name_length) && at[name_length] == ':')
		{
			char const *v = at + name_length + 1;

			while (*v == ' ')
				v++;
			return value ? !strncasecmp_portable(v, value, strlen(value)) : 1;
		}
	}
	return 0;
}

/* one request on the open connection: 1 answered, 0 failed (the
connection then closed) */
static int http_exchange(struct steam_http *http, char const *path, unsigned long maximum, unsigned char **body,
	unsigned long *size, int *status, char *error, int error_size)
{
	char request[600];
	char head[8192];
	size_t have = 0;
	char *end = NULL;
	unsigned char *data;
	size_t body_have, length_needed;
	long content_length;
	int chunked, closing;

	snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Valve/Steam HTTP Client 1.0\r\n"
		"Accept: */*\r\nConnection: keep-alive\r\n\r\n", path, http->host);
	if (!http_send(http->socket, request, strlen(request)))
	{
		snprintf(error, (size_t)error_size, "the content server took no request");
		return 0;
	}
	/* (the head, up to its blank line) */
	while (!end)
	{
		int got;

		if (have >= sizeof(head) - 1)
		{
			snprintf(error, (size_t)error_size, "the content server's answer is not HTTP");
			return 0;
		}
		got = http_read(http->socket, head + have, sizeof(head) - 1 - have);
		if (got <= 0)
		{
			snprintf(error, (size_t)error_size, "the content server closed the connection");
			return 0;
		}
		have += (size_t)got;
		head[have] = 0;
		end = strstr(head, "\r\n\r\n");
	}
	*end = 0;
	if (strncmp(head, "HTTP/1.", 7) || strlen(head) < 12)
	{
		snprintf(error, (size_t)error_size, "the content server's answer is not HTTP");
		return 0;
	}
	*status = atoi(head + 9);
	content_length = header_number(head, "Content-Length");
	chunked = header_has(head, "Transfer-Encoding", "chunked");
	closing = header_has(head, "Connection", "close");
	body_have = have - (size_t)(end + 4 - head);
	if (!chunked && (content_length < 0 || (unsigned long)content_length > maximum))
	{
		snprintf(error, (size_t)error_size, "the content server's answer is too large or has no length");
		return 0;
	}
	length_needed = chunked ? maximum : (size_t)content_length;
	data = malloc(length_needed + 1);
	if (!data)
	{
		snprintf(error, (size_t)error_size, "out of memory");
		return 0;
	}
	if (!chunked)
	{
		if (body_have > length_needed)
			body_have = length_needed;
		memcpy(data, end + 4, body_have);
		while (body_have < length_needed)
		{
			int got = http_read(http->socket, (char *)data + body_have, length_needed - body_have);

			if (got <= 0)
			{
				free(data);
				snprintf(error, (size_t)error_size, "the content server's answer was cut short");
				return 0;
			}
			body_have += (size_t)got;
		}
	}
	else
	{
		/* (chunked: each part's hex length line, its bytes and a CRLF, to a
		part of none and the trailer's blank line) */
		char *stream = malloc(maximum + 65536);
		size_t stream_have = body_have, at = 0, out = 0;
		int done = 0;

		if (!stream)
		{
			free(data);
			snprintf(error, (size_t)error_size, "out of memory");
			return 0;
		}
		memcpy(stream, end + 4, body_have);
		while (!done)
		{
			char *line_end;
			unsigned long part;

			stream[stream_have] = 0;
			line_end = strstr(stream + at, "\r\n");
			if (line_end)
			{
				part = strtoul(stream + at, NULL, 16);
				if (!part)
				{
					done = 1;
					break;
				}
				if (out + part > maximum)
					break;
				if (stream_have >= (size_t)(line_end + 2 - stream) + part + 2)
				{
					memcpy(data + out, line_end + 2, part);
					out += part;
					at = (size_t)(line_end + 2 - stream) + part + 2;
					continue;
				}
			}
			if (stream_have >= maximum + 65536 - 1)
				break;
			{
				int got = http_read(http->socket, stream + stream_have, maximum + 65536 - 1 - stream_have);

				if (got <= 0)
					break;
				stream_have += (size_t)got;
			}
		}
		free(stream);
		if (!done)
		{
			free(data);
			snprintf(error, (size_t)error_size, "the content server's answer was cut short");
			return 0;
		}
		/* (the trailer's blank line may be left unread: the connection is
		not kept) */
		closing = 1;
		body_have = out;
	}
	data[body_have] = 0;
	*body = data;
	*size = (unsigned long)body_have;
	if (closing)
		steam_http_close(http);
	return 1;
}

int steam_http_get(struct steam_http *http, char const *host, char const *path, unsigned long maximum,
	unsigned char **body, unsigned long *size, int *status, char *error, int error_size)
{
	int attempt;

	*body = NULL;
	*size = 0;
	*status = 0;
	if (strcmp(http->host, host))
		steam_http_close(http);
	/* (a kept connection may have been closed by the server meanwhile: one
	more try on a new one) */
	for (attempt = 0; attempt < 2; attempt++)
	{
		int fresh = http->socket < 0;

		if (fresh)
		{
			char name[128];
			char const *colon = strchr(host, ':');
			size_t length = colon ? (size_t)(colon - host) : strlen(host);

			if (length >= sizeof(name))
				return 0;
			memcpy(name, host, length);
			name[length] = 0;
			http->socket = steam_tcp_connect(name, colon ? atoi(colon + 1) : 80, 10000);
			if (http->socket < 0)
			{
				snprintf(error, (size_t)error_size, "the content server %s could not be reached", name);
				return 0;
			}
			snprintf(http->host, sizeof(http->host), "%s", host);
		}
		if (http_exchange(http, path, maximum, body, size, status, error, error_size))
			return 1;
		steam_http_close(http);
		if (fresh)
			return 0;
	}
	return 0;
}

/* ---------- protocol buffers */

/* (a message can hold a password's ciphertext or a refresh token: its
bytes are wiped before they go back to the heap) */
void steam_pb_free(struct steam_pb *pb)
{
	if (pb->bytes)
		steam_wipe(pb->bytes, pb->capacity);
	free(pb->bytes);
	memset(pb, 0, sizeof(*pb));
}

static void pb_put(struct steam_pb *pb, void const *data, size_t size)
{
	if (pb->failed)
		return;
	if (pb->size + size > pb->capacity)
	{
		size_t capacity = pb->capacity ? pb->capacity : 256;
		unsigned char *bytes;

		while (capacity < pb->size + size)
			capacity *= 2;
		/* (not realloc: the old block is wiped before it is let go) */
		bytes = malloc(capacity);
		if (!bytes)
		{
			pb->failed = 1;
			return;
		}
		if (pb->bytes)
		{
			memcpy(bytes, pb->bytes, pb->size);
			steam_wipe(pb->bytes, pb->capacity);
			free(pb->bytes);
		}
		pb->bytes = bytes;
		pb->capacity = capacity;
	}
	memcpy(pb->bytes + pb->size, data, size);
	pb->size += size;
}

static void pb_put_varint(struct steam_pb *pb, uint64_t value)
{
	unsigned char bytes[10];
	int count = 0;

	while (value > 127)
	{
		bytes[count++] = (unsigned char)((value & 127) | 128);
		value >>= 7;
	}
	bytes[count++] = (unsigned char)value;
	pb_put(pb, bytes, (size_t)count);
}

void steam_pb_varint(struct steam_pb *pb, int field, uint64_t value)
{
	pb_put_varint(pb, (uint64_t)field << 3 | STEAM_PB_VARINT);
	pb_put_varint(pb, value);
}

void steam_pb_fixed64(struct steam_pb *pb, int field, uint64_t value)
{
	unsigned char bytes[8];
	int i;

	for (i = 0; i < 8; i++)
		bytes[i] = (unsigned char)(value >> (8 * i));
	pb_put_varint(pb, (uint64_t)field << 3 | STEAM_PB_FIXED64);
	pb_put(pb, bytes, 8);
}

void steam_pb_fixed32(struct steam_pb *pb, int field, uint32_t value)
{
	unsigned char bytes[4];
	int i;

	for (i = 0; i < 4; i++)
		bytes[i] = (unsigned char)(value >> (8 * i));
	pb_put_varint(pb, (uint64_t)field << 3 | STEAM_PB_FIXED32);
	pb_put(pb, bytes, 4);
}

void steam_pb_float(struct steam_pb *pb, int field, float value)
{
	uint32_t bits;

	memcpy(&bits, &value, 4);
	steam_pb_fixed32(pb, field, bits);
}

void steam_pb_bytes(struct steam_pb *pb, int field, void const *data, size_t size)
{
	pb_put_varint(pb, (uint64_t)field << 3 | STEAM_PB_BYTES);
	pb_put_varint(pb, size);
	pb_put(pb, data, size);
}

void steam_pb_string(struct steam_pb *pb, int field, char const *text)
{
	steam_pb_bytes(pb, field, text, strlen(text));
}

void steam_pb_raw(struct steam_pb *pb, void const *data, size_t size)
{
	pb_put(pb, data, size);
}

static int pb_get_varint(struct steam_pb_reader *reader, uint64_t *value)
{
	uint64_t result = 0;
	int shift = 0;

	while (reader->at < reader->end && shift < 64)
	{
		unsigned char byte = *reader->at++;

		result |= (uint64_t)(byte & 127) << shift;
		if (!(byte & 128))
		{
			*value = result;
			return 1;
		}
		shift += 7;
	}
	reader->failed = 1;
	return 0;
}

int steam_pb_next(struct steam_pb_reader *reader, int *field, int *wire, uint64_t *value,
	unsigned char const **data, size_t *size)
{
	uint64_t tag;
	int i;

	*value = 0;
	*data = NULL;
	*size = 0;
	if (reader->failed || reader->at >= reader->end)
		return 0;
	if (!pb_get_varint(reader, &tag))
		return 0;
	*field = (int)(tag >> 3);
	*wire = (int)(tag & 7);
	switch (*wire)
	{
	case STEAM_PB_VARINT:
		return pb_get_varint(reader, value);
	case STEAM_PB_FIXED64:
	case STEAM_PB_FIXED32:
	{
		int count = *wire == STEAM_PB_FIXED64 ? 8 : 4;

		if (reader->end - reader->at < count)
			break;
		for (i = 0; i < count; i++)
			*value |= (uint64_t)reader->at[i] << (8 * i);
		reader->at += count;
		return 1;
	}
	case STEAM_PB_BYTES:
	{
		uint64_t length;

		if (!pb_get_varint(reader, &length) || length > (uint64_t)(reader->end - reader->at))
			break;
		*data = reader->at;
		*size = (size_t)length;
		reader->at += length;
		return 1;
	}
	default:
		break;
	}
	reader->failed = 1;
	return 0;
}

void steam_pb_text(unsigned char const *data, size_t size, char *text, size_t capacity)
{
	if (!capacity)
		return;
	if (size > capacity - 1)
		size = capacity - 1;
	if (data)
		memcpy(text, data, size);
	text[data ? size : 0] = 0;
}

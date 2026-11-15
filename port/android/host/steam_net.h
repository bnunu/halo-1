/*
STEAM_NET.H

What the Steam client sends and reads, below its messages: an HTTPS GET
(the host's HTTPS client), plain TCP and HTTP over the host's
posix_socket calls, and protocol buffers, written and read field by field
(steam_cm.c's messages are Steam's, as SteamKit and DepotDownloader document
them). The client is the Android app's host's: the guest only shows what steam_downloads.c lets it have, and never holds a
Steam token.
*/

#ifndef STEAM_NET_H
#define STEAM_NET_H

#include <stddef.h>
#include <stdint.h>

#include "host.h"

/* the host's log (logcat and halo.log) */
#define steam_log(...) host_logf(HOST_LOG_INFO, __VA_ARGS__)

/* a monotonic clock, in milliseconds */
unsigned long steam_milliseconds(void);
/* memory set to zeros in a way the compiler keeps: passwords, tokens and
keys before they are let go */
void steam_wipe(void *memory, size_t size);

/* a GET of an https:// url, at most maximum bytes of its body (malloc'd,
NUL terminated past its size: the caller frees it); 1 with the status,
else 0 and why in error */
int steam_https_get(char const *url, unsigned long maximum, unsigned char **body, unsigned long *size, int *status,
	char *error, int error_size);

/* ---------- plain TCP and HTTP (the content servers: Steam's own client
reads depots over HTTP, as LAN caches rely on; the chunks are encrypted and
checked against their SHA-1s, the manifests encrypted where it matters) */

/* the address posix_socket_connect takes (Winsock's sockaddr_in, which
every host's posix_net.c reads): an IPv4 address in network order */
struct steam_sockaddr
{
	uint16_t family;
	uint16_t port;
	uint32_t ip;
	unsigned char zero[8];
};
void steam_address(struct steam_sockaddr *address, uint32_t ip, int port);

/* a TCP connection to host:port within milliseconds: its socket, blocking,
or -1 */
int steam_tcp_connect(char const *host, int port, unsigned long milliseconds);
/* whether the socket can be read (or written, writing) within milliseconds:
1, 0 at the time, -1 on an error */
int steam_socket_wait(int socket, unsigned long milliseconds, int writing);

/* a kept-alive HTTP connection (one thread's) */
struct steam_http
{
	int socket;
	char host[128];
};

void steam_http_init(struct steam_http *http);
void steam_http_close(struct steam_http *http);
/* a GET of http://host/path on the connection (made, or made again, as
needed), at most maximum bytes of body (malloc'd, NUL terminated past
its size): 1 with the status, else 0 and why in error */
int steam_http_get(struct steam_http *http, char const *host, char const *path, unsigned long maximum,
	unsigned char **body, unsigned long *size, int *status, char *error, int error_size);

/* ---------- protocol buffers */

enum
{
	STEAM_PB_VARINT = 0,
	STEAM_PB_FIXED64 = 1,
	STEAM_PB_BYTES = 2,
	STEAM_PB_FIXED32 = 5
};

/* a message being written: bytes grow as needed; failed once memory ran out */
struct steam_pb
{
	unsigned char *bytes;
	size_t size;
	size_t capacity;
	int failed;
};

void steam_pb_free(struct steam_pb *pb);
void steam_pb_varint(struct steam_pb *pb, int field, uint64_t value);
void steam_pb_fixed64(struct steam_pb *pb, int field, uint64_t value);
void steam_pb_fixed32(struct steam_pb *pb, int field, uint32_t value);
void steam_pb_float(struct steam_pb *pb, int field, float value);
void steam_pb_bytes(struct steam_pb *pb, int field, void const *data, size_t size);
void steam_pb_string(struct steam_pb *pb, int field, char const *text);
/* (raw bytes, no field: a header's or a nested message's own) */
void steam_pb_raw(struct steam_pb *pb, void const *data, size_t size);

/* a message being read */
struct steam_pb_reader
{
	unsigned char const *at;
	unsigned char const *end;
	int failed;
};

/* the next field: 1 with its number, its wire type and its value (a varint
or a fixed's in value; bytes' in data and size), 0 at the end or when the
message is malformed (failed then) */
int steam_pb_next(struct steam_pb_reader *reader, int *field, int *wire, uint64_t *value,
	unsigned char const **data, size_t *size);
/* a bytes field's text into text (cut at capacity - 1) */
void steam_pb_text(unsigned char const *data, size_t size, char *text, size_t capacity);

#endif

/*
P2P_TUNNEL_CHECK.C

Internet play between two copies of this program, as two copies of the game
play it: port/linux/src/p2p.c and p2p_signal.c whole, with stand-ins for the
rest of the game (no WebRTC, server browser, Discord, UPnP or STUN). One
hosts (its game listening, as the game's does), the other joins it with its
invite through the signalling brokers, and both wait until the tunnel
reaches the other. Built and run by tools/test_p2p_tunnel.py:

    p2p_tunnel_check host <family>
    p2p_tunnel_check join <invite> <family>

The family (ipv4, ipv6 or any) is of the addresses each offers: ipv6 is two
machines behind carrier NATs that let no IPv4 connection through. The host
prints "invite <link>"; each prints "connected <the log's line>" once the
tunnel reaches the other, the joiner also "real <text> <address>" (the host's
real address as a host's bans and votes see a player's: p2p.h), then "PASS",
or "FAIL <why>", or "SKIP <why>" (ipv6 on a machine without a global IPv6
address). HARNESS_BROKERS names the brokers (host:port, comma-separated;
the game's own list otherwise).
*/

#include "platform.h"
#include "posix.h"
#include "port_config.h"
#include "p2p_internal.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum
{
	TIMEOUT_SECONDS = 60,
	/* how long the host stays after it is reached, for the joiner's end */
	LINGER_SECONDS = 5,
};

static const char *family = "any";
static volatile int connected;
static char connected_line[512];

/* ---------- the game's, which p2p.c and p2p_signal.c call */

DWORD WINAPI GetTickCount(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (DWORD)((unsigned long long)now.tv_sec * 1000ULL + (unsigned long long)now.tv_nsec / 1000000ULL);
}

VOID WINAPI Sleep(DWORD milliseconds)
{
	struct timespec wait;

	wait.tv_sec = milliseconds / 1000;
	wait.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
	nanosleep(&wait, NULL);
}

void platform_log(const char *format, ...)
{
	char line[512];
	va_list arguments;

	va_start(arguments, format);
	vsnprintf(line, sizeof(line), format, arguments);
	va_end(arguments);
	fprintf(stderr, "%s\n", line);
	if (!connected && strstr(line, "Internet play: connected to "))
	{
		snprintf(connected_line, sizeof(connected_line), "%s", line);
		connected = 1;
	}
}

int config_boolean(const char *name)
{
	/* (whole addresses in the log, which the test reads; no URL scheme, no
	router, no listing) */
	return !strcmp(name, "network.online") || !strcmp(name, "network.ipv6") ||
		!strcmp(name, "debug.log_addresses") || !strcmp(name, "debug.hidden_window") ||
		!strcmp(name, "debug.null_renderer");
}

long config_integer(const char *name)
{
	(void)name;
	return 0;
}

double config_real(const char *name)
{
	return !strcmp(name, "debug.exit_after") ? 1.0 : 0.0;
}

const char *config_string(const char *name)
{
	return !strcmp(name, "network.brokers_file") ? "brokers.txt" : "";
}

void config_folder(char *path, size_t size)
{
	if (size)
		path[0] = 0;
}

char *config_file_read(const char *path, size_t *size)
{
	const char *brokers = getenv("HARNESS_BROKERS");
	char *text;
	size_t index;

	(void)path;
	if (!brokers || !brokers[0])
		return NULL;
	*size = strlen(brokers);
	text = malloc(*size + 1);
	if (!text)
		return NULL;
	memcpy(text, brokers, *size + 1);
	for (index = 0; index < *size; index++)
	{
		if (text[index] == ',')
			text[index] = '\n';
	}
	return text;
}

#ifdef _WIN32
/* (win32_p2p.c's and win32_crash.c's; posix_net.c has the first three on
Linux) */
int posix_command_line_argument(int index, char *buffer, posix_ulong size)
{
	(void)index;
	(void)buffer;
	(void)size;
	return 0;
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

int posix_hardware_id_source(char *text, int size)
{
	(void)text;
	(void)size;
	return 0;
}

int crash_reporter_process(void)
{
	return 0;
}
#endif

int posix_upnp_forward_udp(unsigned short port, unsigned short preferred_port, posix_ulong *external_address,
	unsigned short *external_port, char *error, int error_size)
{
	(void)port;
	(void)preferred_port;
	(void)external_address;
	(void)external_port;
	snprintf(error, (size_t)error_size, "none here");
	return 0;
}

void posix_upnp_stop_forwarding_udp(unsigned short external_port)
{
	(void)external_port;
}

/* ---------- WebRTC: a native build's, which connects no browser here, and
the addresses offered of the family asked */

static int family_wanted(const struct p2p_candidate *candidate)
{
	if (!strcmp(family, "ipv4"))
		return !candidate->ipv6;
	if (!strcmp(family, "ipv6"))
		return candidate->ipv6;
	return 1;
}

int p2p_webrtc_describe(const unsigned char *identifier, int proven, struct p2p_webrtc *local,
	struct p2p_candidate *candidates, int maximum_count)
{
	struct p2p_candidate all[P2P_MAXIMUM_CANDIDATES];
	int count = p2p_local_candidates(all, maximum_count < P2P_MAXIMUM_CANDIDATES ? maximum_count :
		P2P_MAXIMUM_CANDIDATES);
	int kept = 0;
	int index;

	(void)identifier;
	(void)proven;
	/* (a native build's, as p2p_webrtc.c says it with its certificate: its
	IPv6 addresses go with it, put_addresses) */
	memset(local, 0, sizeof(*local));
	local->kind = _p2p_webrtc_native;
	memset(local->fingerprint, 0x5A, sizeof(local->fingerprint));
	for (index = 0; index < count; index++)
	{
		if (family_wanted(&all[index]))
			candidates[kept++] = all[index];
	}
	return kept;
}

int p2p_webrtc_ready(void) { return 1; }
void p2p_webrtc_new_request(void) { }
int p2p_webrtc_offered(int connection, int peer, const unsigned char *secret, const struct p2p_webrtc *remote,
	const struct p2p_candidate *candidates, int count, int is_host)
{
	(void)connection; (void)peer; (void)secret; (void)remote; (void)candidates; (void)count; (void)is_host;
	return -1;
}
void p2p_webrtc_close(int connection) { (void)connection; }
void p2p_webrtc_send(int connection, const void *packet, int size) { (void)connection; (void)packet; (void)size; }
int p2p_webrtc_received(const unsigned char *packet, int size, unsigned long address, unsigned short port)
{
	(void)packet; (void)size; (void)address; (void)port;
	return 0;
}
int p2p_webrtc_update(void) { return 0; }

/* ---------- the server browser and Discord: none */

void p2p_lobby_update(const unsigned char *token, int player_count, int maximum_player_count)
{
	(void)token; (void)player_count; (void)maximum_player_count;
}
int p2p_lobby_listed(void) { return 0; }
int p2p_lobby_browsing(void) { return 0; }
void p2p_lobby_slot_heard(const char *hash_text, const unsigned char *payload, int size, int retained)
{
	(void)hash_text; (void)payload; (void)size; (void)retained;
}
void p2p_lobby_query_heard(void) { }
void p2p_lobby_probed(const unsigned char *identifier, int ping) { (void)identifier; (void)ping; }
void p2p_lobby_quit(void) { }
void p2p_lobby_slot_topic(const unsigned char *key_hash, char *topic, int size)
{
	char hex[2 * P2P_KEY_HASH_SIZE + 1];

	p2p_hex(key_hash, P2P_KEY_HASH_SIZE, hex);
	snprintf(topic, (size_t)size, "%s%s", P2P_LOBBY_SLOT_PREFIX, hex);
}
void p2p_discord_update(void) { }
void p2p_discord_user(char *id, int id_size, char *name, int name_size)
{
	if (id_size)
		id[0] = 0;
	if (name_size)
		name[0] = 0;
}
void p2p_discord_set_hosting(const char *secret, int player_count, int maximum_player_count)
{
	(void)secret; (void)player_count; (void)maximum_player_count;
}

/* ---------- the check */

static int has_family(void)
{
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	struct p2p_webrtc webrtc;
	int count;

	pthread_mutex_lock(&p2p_lock);
	count = p2p_webrtc_describe(NULL, 0, &webrtc, candidates, P2P_MAXIMUM_CANDIDATES);
	pthread_mutex_unlock(&p2p_lock);
	return count > 0;
}

/* the identifier of an invite's host, from its key hash */
static int invite_host(const char *invite, unsigned char *identifier)
{
	const char *hex = strstr(invite, "join/");
	unsigned char hash[P2P_KEY_HASH_SIZE];
	int index;

	if (!hex)
		return 0;
	hex += 5;
	for (index = 0; index < P2P_KEY_HASH_SIZE; index++)
	{
		unsigned int value;

		if (sscanf(hex + 2 * index, "%2x", &value) != 1)
			return 0;
		hash[index] = (unsigned char)value;
	}
	p2p_identifier_from_hash(hash, identifier);
	return 1;
}

static int wait_connected(void)
{
	int tenths;

	for (tenths = 0; tenths < TIMEOUT_SECONDS * 10 && !connected; tenths++)
		Sleep(100);
	return connected;
}

int main(int argc, char **argv)
{
	int hosting = argc == 3 && !strcmp(argv[1], "host");
	int joining = argc == 4 && !strcmp(argv[1], "join");

	setvbuf(stdout, NULL, _IONBF, 0);
	if (!hosting && !joining)
	{
		printf("usage: p2p_tunnel_check host <ipv4|ipv6|any> | join <invite> <ipv4|ipv6|any>\n");
		return 2;
	}
	family = argv[argc - 1];
	/* (the game's local address: its stand-ins listen there) */
	p2p_initialize(0x0100007FUL);
	if (!has_family())
	{
		printf("SKIP this machine offers no %s address\n", family);
		return 0;
	}
	if (hosting)
	{
		char link[P2P_LINK_SIZE];
		int tenths;

		/* the game listening for connections: it hosts */
		p2p_socket_port(1000, 1, 1, (unsigned short)(2303 >> 8 | (2303 & 255) << 8));
		for (tenths = 0; tenths < 100 && !p2p_invite_link(link, sizeof(link)); tenths++)
			Sleep(100);
		if (!link[0])
		{
			printf("FAIL no invite\n");
			return 1;
		}
		printf("invite %s\n", link);
		if (!wait_connected())
		{
			printf("FAIL the joiner never reached this host\n");
			return 1;
		}
		printf("connected %s\n", connected_line);
		Sleep(LINGER_SECONDS * 1000);
		printf("PASS\n");
		return 0;
	}
	{
		unsigned char host[P2P_IDENTIFIER_SIZE];
		unsigned long virtual_address;
		unsigned long real;
		char text[64];

		if (!invite_host(argv[2], host) || !p2p_join_invite(argv[2]))
		{
			printf("FAIL not an invite: %s\n", argv[2]);
			return 1;
		}
		if (!wait_connected())
		{
			printf("FAIL never reached the host\n");
			return 1;
		}
		printf("connected %s\n", connected_line);
		if (!p2p_peer_reached(host, &virtual_address) || !p2p_peer_endpoint_text(virtual_address, text, sizeof(text)))
		{
			printf("FAIL the host's real address is not known\n");
			return 1;
		}
		real = p2p_peer_endpoint_address(virtual_address);
		printf("real %s %u.%u.%u.%u\n", text, (unsigned int)(real & 255), (unsigned int)(real >> 8 & 255),
			(unsigned int)(real >> 16 & 255), (unsigned int)(real >> 24 & 255));
		printf("PASS\n");
		return 0;
	}
}

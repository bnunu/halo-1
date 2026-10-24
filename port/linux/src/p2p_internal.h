/*
P2P_INTERNAL.H

Shared internals of internet play (p2p.c, p2p_signal.c, p2p_crypto.c,
p2p_discord.c; see p2p.c for the design).
*/

#ifndef __HALO_LINUX_P2P_INTERNAL_H
#define __HALO_LINUX_P2P_INTERNAL_H

#include "p2p.h"

#include <pthread.h>

enum
{
	/* a machine's identifier: from the hash of its public key (which is new
	each run), and also its XNADDR's abEnet */
	P2P_IDENTIFIER_SIZE = 6,
	/* what an invite holds of the host's public key: the first bytes of its
	SHA-256 (of which the identifier is the first 6), so that no other key
	can be found to pass for it */
	P2P_KEY_HASH_SIZE = 16,
	/* an invite's secret */
	P2P_TOKEN_SIZE = 16,
	/* the addresses a machine offers to be reached at (IPv4 and IPv6) */
	P2P_MAXIMUM_CANDIDATES = 6,
	/* an invite link's text: "halo://join/", the host's key hash and the
	token in hexadecimal, and a terminator */
	P2P_LINK_SIZE = 12 + 2 * (P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE) + 1,
	/* the most machines one tunnels to: a host and the rest of a system
	link game's 128 machines (include/halo_port_limits.h) */
	P2P_MAXIMUM_PEERS = 127,
	/* the other players of its game a machine asks to reach at once
	(p2p_reach_player) */
	P2P_MAXIMUM_PLAYER_JOINS = 8,
	/* the server browser's public games' hosts probed at once, for their
	ping (p2p_probe_start) */
	P2P_MAXIMUM_PROBES = 4,
};

typedef char p2p_player_key_hash_size_assert[P2P_PLAYER_KEY_HASH_SIZE == P2P_KEY_HASH_SIZE ? 1 : -1];

/* who offered a session (p2p_peer_offered): a joiner (to this machine,
hosting or answering its game's players), the invite's host, another player
of the host's game this machine asked for (p2p_reach_player), which answers
as a host does, or a public game's host probed for its ping
(p2p_probe_start) */
enum
{
	_p2p_offered_joiner,
	_p2p_offered_host,
	_p2p_offered_player,
	_p2p_offered_probe,
};

typedef char p2p_listing_invite_size_assert[P2P_LISTING_INVITE_SIZE == P2P_LINK_SIZE ? 1 : -1];

/* the prefix of a public game's slot (a key hash in hex follows), and the
topic of queries (p2p_lobby.c) */
#define P2P_LOBBY_SLOT_PREFIX "hceu/3/lobby/s/"
#define P2P_LOBBY_QUERY_TOPIC "hceu/3/lobby/q"

/* internet play's state (p2p.c), which the game's threads and the p2p
thread share */
extern pthread_mutex_t p2p_lock;

struct p2p_candidate
{
	/* network byte order: an IPv4 address (0 for an IPv6 one) */
	unsigned long address;
	unsigned short port;
	/* nonzero: an IPv6 address, address6 (first byte first), not address.
	Make a candidate with p2p_candidate_ipv4 or p2p_candidate_ipv6, never
	field by field */
	unsigned char ipv6;
	unsigned char address6[16];
};

/* ---------- p2p.c: candidates */

void p2p_candidate_ipv4(struct p2p_candidate *candidate, unsigned long address, unsigned short port);
void p2p_candidate_ipv6(struct p2p_candidate *candidate, const unsigned char *address, unsigned short port);
/* whether two are the same address and port */
int p2p_candidate_equal(const struct p2p_candidate *a, const struct p2p_candidate *b);

/* ---------- p2p.c: what the signalling side calls back */

/* the milliseconds of a monotonic clock */
unsigned long p2p_now(void);
/* looks up a host name (posix_resolve_ipv4), letting go of the p2p lock
while it waits; the p2p thread's */
unsigned long p2p_resolve(const char *host);
/* registers this executable for links of scheme (posix_register_url_scheme),
unless it is an automated run (debug.exit_after, a hidden window, no
renderer), which must not take the links over. The p2p thread's: it lets
go of the p2p lock while it may wait for a program */
void p2p_register_url_scheme(const char *scheme, const char *description);
/* formats bytes as lower-case hexadecimal (text holds 2 * size + 1) */
void p2p_hex(const unsigned char *bytes, int size, char *text);
/* the addresses this machine can be reached at; returns their count */
int p2p_local_candidates(struct p2p_candidate *candidates, int maximum_count);
/* this run's X25519 public key (P2P_KEY_SIZE bytes), whose hash the
identifier is (p2p_identifier) */
const unsigned char *p2p_public_key(void);
/* this run's Ed25519 public key (the X25519 one's, p2p_ed25519_to_x25519),
and a signature with it */
const unsigned char *p2p_signing_key(void);
void p2p_sign(const void *message, int size, unsigned char *signature);
/* the identifier of the machine with this public key */
void p2p_identifier_for(const unsigned char *public_key, unsigned char *identifier);
/* the hash of a public key an invite holds (P2P_KEY_HASH_SIZE bytes), and
the identifier of the machine whose key has that hash */
void p2p_key_hash(const unsigned char *public_key, unsigned char *hash);
void p2p_identifier_from_hash(const unsigned char *hash, unsigned char *identifier);
/* the X25519 secret this machine shares with the one with that public key;
0 if the key is unusable (one giving a known secret). The p2p thread's: it
lets go of the p2p lock while it works it out */
int p2p_shared_secret(const unsigned char *public_key, unsigned char *shared);
/* a joiner (on the host) or the host (on a joiner) offered its addresses and
its WebRTC through signalling, with its public key and the secret of a
session (P2P_SHA256_SIZE bytes) its tunnel's keys come from; the tunnel
starts reaching it. offered is who it is (_p2p_offered_*). Returns 0 if it
was turned away: another session with that machine lives (it must lapse
first), this one has ended, or there is no room */
struct p2p_webrtc;
int p2p_peer_offered(const unsigned char *identifier, const unsigned char *public_key, const unsigned char *secret,
	const struct p2p_candidate *candidates, int count, const struct p2p_webrtc *webrtc, int offered);
/* whether p2p_peer_offered would turn a new session with that machine away
now (it is this machine, a session with it lives, there is no room, or, as
a host, too many players are being reached): checked before its secret is
worked out */
int p2p_peer_turned_away(const unsigned char *identifier, int is_host);
/* ... more addresses of a machine whose session (that secret's) lives, and
its WebRTC (a browser host's credentials come once the request is proven):
0 if none does (a session that has ended is never taken up again: its keys'
packet numbers would start again) */
int p2p_peer_reoffered(const unsigned char *identifier, const unsigned char *secret,
	const struct p2p_candidate *candidates, int count, const struct p2p_webrtc *webrtc);
/* an invite that arrived on the p2p thread (from Discord, or another copy
of the game) */
void p2p_invite_received(const char *text);
/* a new invite (token) for the game hosted, if its invite was listed in the
server browser (going private: those who saw it must not get in); under
p2p_lock */
void p2p_new_invite_if_listed(void);
/* the token of the game hosted was listed elsewhere than the brokers (a
master server's listing: p2p_lobby_host_listing), so going private makes a
new invite too; under p2p_lock */
void p2p_token_listed(const unsigned char *token);

/* ---------- p2p_signal.c: signalling through public MQTT brokers */

/* connects to the brokers, if not already; called from the p2p thread */
void p2p_signal_start(void);
/* adds the signalling sockets to the p2p thread's select lists */
void p2p_signal_select_sets(int *read, int *read_count, int *write, int *write_count, int maximum_count);
/* services the sockets and timers; called from the p2p thread each pass */
void p2p_signal_update(const int *read, int read_count, const int *write, int write_count);
/* hosting: listen for joiners who hold this token */
void p2p_signal_host(const unsigned char *token);
void p2p_signal_stop_hosting(void);
/* joining: ask the host whose public key has this hash (p2p_key_hash),
holding this token, until it answers (or p2p_signal_stop_joining); each call
asks anew, with a new nonce (as after the session with the host ended
before the tunnel reached it: the host makes one session of a request) */
void p2p_signal_join(const unsigned char *host_hash, const unsigned char *token);
void p2p_signal_stop_joining(void);
/* ... and another player of the host's game (whose key has this hash, as
the host told), which answers holders of the token as its host does
(p2p_signal_host with the token it joined with), alongside the request to
the host; 0 if there is no room, or the request to the host is of another
token. Stopped by its identifier */
int p2p_signal_join_player(const unsigned char *player_hash, const unsigned char *token);
void p2p_signal_stop_joining_player(const unsigned char *identifier);
/* ... and a public game's host (whose key has this hash) with its listing's
token, only to time the tunnel to it (p2p_probe_start): a request of its own
token, whose answers come to a topic of their own; 0 if there is no room.
Stopped by its identifier */
int p2p_signal_probe(const unsigned char *host_hash, const unsigned char *token);
void p2p_signal_stop_probe(const unsigned char *identifier);
/* whether any broker is connected */
int p2p_signal_connected(void);
/* every broker's connection made anew at once: after the p2p thread was
stopped (the process was suspended, or the machine slept), the old ones
may carry nothing, and would be noticed only at their silence timeout */
void p2p_signal_reconnect(void);
/* the server browser's topics: the own slot and the queries (a listed
game), and every slot (browsing) */
void p2p_signal_lobby_topics(int listed, int browsing);
/* publishes a listing to the own slot on every broker, retained, and again
on each broker that connects later; closing: a tombstone, after which the
slot is cleared and nothing is published again */
void p2p_signal_lobby_publish(const unsigned char *listing, int size, int closing);
/* asks the hosts to publish again */
void p2p_signal_lobby_query(void);
/* the game is quitting: the tombstone published (p2p_signal_lobby_publish)
and the slot cleared on every ready broker now, whatever their buckets, and
the connections closed cleanly */
void p2p_signal_lobby_quit(void);

/* ---------- p2p_crypto.c */

enum
{
	P2P_SHA256_SIZE = 32,
	/* an X25519 secret or public key, and a shared secret */
	P2P_KEY_SIZE = 32,
	/* ChaCha20-Poly1305's nonce and tag */
	P2P_NONCE_SIZE = 12,
	P2P_TAG_SIZE = 16,
	/* what p2p_seal adds: a random nonce and the tag */
	P2P_SEAL_OVERHEAD = P2P_NONCE_SIZE + P2P_TAG_SIZE,
};

void p2p_sha256(const void *data, int size, unsigned char *digest);
void p2p_hmac_sha256(const unsigned char *key, int key_size, const void *data, int size, unsigned char *digest);
/* ChaCha20-Poly1305: encrypts plaintext with a 32-byte key and a nonce used
with that key once, and authenticates it and the additional data, into
sealed (size + P2P_TAG_SIZE bytes); returns the sealed size */
int p2p_aead_seal(const unsigned char *key, const unsigned char *nonce, const void *additional,
	int additional_size, const void *plaintext, int size, unsigned char *sealed);
/* the reverse: the plaintext size, or -1 if sealed (or the additional data)
was not made so or was altered */
int p2p_aead_open(const unsigned char *key, const unsigned char *nonce, const void *additional,
	int additional_size, const unsigned char *sealed, int size, unsigned char *plaintext);
/* the same with a random nonce ahead of the ciphertext (size +
P2P_SEAL_OVERHEAD bytes) and no additional data */
int p2p_seal(const unsigned char *key, const void *plaintext, int size, unsigned char *sealed);
int p2p_open(const unsigned char *key, const unsigned char *sealed, int size, unsigned char *plaintext);
/* whether two byte strings are the same, compared in constant time */
int p2p_equal(const void *first, const void *second, int size);
/* X25519: scalar times point (NULL: the base point, which gives the public
key of the secret key scalar) */
void p2p_x25519(unsigned char *result, const unsigned char *scalar, const unsigned char *point);

enum
{
	/* an Ed25519 seed (a run's key comes from one) and signature */
	P2P_SEED_SIZE = 32,
	P2P_SIGNATURE_SIZE = 64,
	P2P_SHA512_SIZE = 64,
};

void p2p_sha512(const void *data, int size, unsigned char *digest);
/* Ed25519 (with SHA-512): a seed's public key (P2P_KEY_SIZE bytes) and,
unless NULL, its X25519 secret key (the scalar it signs with, whose X25519
public key is p2p_ed25519_to_x25519 of the Ed25519 one) */
void p2p_ed25519_public(const unsigned char *seed, unsigned char *public_key, unsigned char *x25519_secret);
void p2p_ed25519_sign(const unsigned char *seed, const unsigned char *public_key, const void *message, int size,
	unsigned char *signature);
/* whether the signature is the key's, of the message (never for a key of
small order, which anyone can sign for) */
int p2p_ed25519_verify(const unsigned char *public_key, const void *message, int size,
	const unsigned char *signature);
/* the X25519 public key of an Ed25519 one; 0 if it has a small order */
int p2p_ed25519_to_x25519(const unsigned char *public_key, unsigned char *x25519_public);

enum
{
	/* a password-protected listing's token, sealed (p2p_seal_token): its
	nonce (24), its tag (16), then the token sealed */
	P2P_PASSWORD_KEY_SIZE = 32,
	P2P_SEALED_TOKEN_SIZE = 24 + 16 + 16,
};

/* the key of a password (Argon2id: P2P_PASSWORD_KEY_SIZE bytes), for the
host whose Ed25519 key salt is (P2P_KEY_SIZE bytes): it takes a few
milliseconds, so that guessing passwords at a listing takes long */
void p2p_password_key(const char *password, const unsigned char *salt, unsigned char *key);
/* a token (P2P_TOKEN_SIZE bytes) sealed with a password's key, bound to the
host's Ed25519 key (P2P_SEALED_TOKEN_SIZE bytes); and opened: 0 if the key
is not the one it was sealed with (a wrong password), or it was altered */
void p2p_seal_token(const unsigned char *key, const unsigned char *signing_key, const unsigned char *token,
	unsigned char *sealed);
int p2p_unseal_token(const unsigned char *key, const unsigned char *signing_key, const unsigned char *sealed,
	unsigned char *token);

/* ---------- WebRTC: tunnels to and from browsers (p2p_webrtc.c in the
native builds, port/web/src/web_p2p.c in the browser) */

enum
{
	/* what a machine says of itself in signalling: nothing (no machine of
	this version), a native build (which takes browsers' WebRTC on its
	tunnel's socket), or a browser (which has WebRTC alone) */
	_p2p_webrtc_none = 0,
	_p2p_webrtc_native,
	_p2p_webrtc_browser,
	/* the SHA-256 of a DTLS certificate */
	P2P_FINGERPRINT_SIZE = 32,
	/* a browser's own addresses, which it hides behind mDNS names (another
	browser on its network can look them up), and a name's text with its
	end */
	P2P_MAXIMUM_WEBRTC_NAMES = 2,
	P2P_WEBRTC_NAME_SIZE = 64,
	/* ICE's username fragment and password, as text with their end */
	P2P_ICE_UFRAG_SIZE = 33,
	P2P_ICE_PASSWORD_SIZE = 65,
};

/* a machine's WebRTC, as signalled */
struct p2p_webrtc
{
	int kind;
	unsigned char fingerprint[P2P_FINGERPRINT_SIZE];
	/* a browser's ICE credentials (empty until it has a connection for the
	other machine: a host's, once the joiner proved its request). A native
	build's come from the session's secret (p2p_webrtc_credentials) */
	char ufrag[P2P_ICE_UFRAG_SIZE];
	char password[P2P_ICE_PASSWORD_SIZE];
	/* a browser's addresses on its own network, as mDNS names
	("<uuid>.local") and ports (network byte order) */
	int name_count;
	char names[P2P_MAXIMUM_WEBRTC_NAMES][P2P_WEBRTC_NAME_SIZE];
	unsigned short name_ports[P2P_MAXIMUM_WEBRTC_NAMES];
};

/* whether this machine's WebRTC can be told yet (a browser's: its
certificate and a connection ready; a native build's at once) */
int p2p_webrtc_ready(void);
/* this machine's WebRTC for a message to the machine with this identifier
(the host joined, or a joiner answered: proven, or not yet), and the
addresses it offers it; returns their count */
int p2p_webrtc_describe(const unsigned char *identifier, int proven, struct p2p_webrtc *local,
	struct p2p_candidate *candidates, int maximum_count);
/* a new request to join (p2p_signal_join): a browser's connection for the
last one, not taken up by a session, goes */
void p2p_webrtc_new_request(void);
/* a session with the peer (p2p.c's index) whose machine has this WebRTC
was offered, or offered again (more addresses, a browser's credentials):
the WebRTC connection that carries its tunnel (an index, kept by p2p.c), or
-1 if none does (two native builds: their UDP tunnel). connection is the one
returned before (-1 the first time) */
int p2p_webrtc_offered(int connection, int peer, const unsigned char *secret, const struct p2p_webrtc *remote,
	const struct p2p_candidate *candidates, int count, int is_host);
void p2p_webrtc_close(int connection);
/* a tunnel packet to the connection's peer (dropped until it is open) */
void p2p_webrtc_send(int connection, const void *packet, int size);
/* a datagram that arrived on the tunnel's socket: 1 if it was WebRTC's (an
ICE check, DTLS), taken */
int p2p_webrtc_received(const unsigned char *packet, int size, unsigned long address, unsigned short port);
/* timers (DTLS, SCTP) and hole punching; called from the p2p thread each
pass: whether a connection is opening or open (the thread passes often) */
int p2p_webrtc_update(void);
/* a native build's ICE credentials from a session's secret (both ends work
them out: they do not travel); p2p_crypto.c's */
void p2p_webrtc_credentials(const unsigned char *secret, char *ufrag, char *password);

/* p2p.c's, which WebRTC calls: a tunnel packet a connection carried, a
datagram it sends on the tunnel's socket, and a peer's name as logged (its
identifier in hex: 2 * P2P_IDENTIFIER_SIZE + 1 characters) */
void p2p_tunnel_packet(const unsigned char *packet, int size, unsigned long address, unsigned short port);
void p2p_tunnel_send(unsigned long address, unsigned short port, const void *data, int size);
void p2p_peer_name(int peer, char *name);
/* the tunnel's socket's port (network byte order) */
unsigned short p2p_tunnel_local_port(void);
/* the server browser's probe of a public game's host (its key's hash, its
listing's token): the tunnel to it made, timed and closed again, its ping
told p2p_lobby_probed (at once for this machine's own game, and a host the
tunnel reaches already); 0 if it cannot start now (no room, the public
address not known yet). The probes end as the browser closes. Under
p2p_lock, on the p2p thread */
int p2p_probe_start(const unsigned char *host_hash, const unsigned char *token);

/* ---------- p2p_lobby.c: public games' listings */

/* the p2p thread's pass: the token of the game hosted for the internet
(NULL if none) and its player counts */
void p2p_lobby_update(const unsigned char *token, int player_count, int maximum_player_count);
/* whether the game hosted is listed now, and whether the browser is open */
int p2p_lobby_listed(void);
int p2p_lobby_browsing(void);
/* a message on a slot (its key hash in hex) through a broker; retained: the
slot's retained copy, sent on subscribing */
void p2p_lobby_slot_heard(const char *hash_text, const unsigned char *payload, int size, int retained);
/* a query was heard */
void p2p_lobby_query_heard(void);
/* a probe of a host (its identifier) ended: its ping in milliseconds, -1 if
it was not reached; under p2p_lock */
void p2p_lobby_probed(const unsigned char *identifier, int ping);
/* the game is quitting: a listed game's tombstone, and its slot cleared,
written to the brokers at once; under p2p_lock */
void p2p_lobby_quit(void);
/* a key hash's slot */
void p2p_lobby_slot_topic(const unsigned char *key_hash, char *topic, int size);

/* ---------- p2p_discord.c: rich presence and invites through the Discord
desktop client */

/* called from the p2p thread each pass */
void p2p_discord_update(void);
/* the Discord user signed in, as told (empty if none): under p2p_lock */
void p2p_discord_user(char *id, int id_size, char *name, int name_size);
/* what to show: hosting with an invite link's secret and player counts, or
not (secret NULL) */
void p2p_discord_set_hosting(const char *secret, int player_count, int maximum_player_count);

#endif

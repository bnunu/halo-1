/*
P2P_LOBBY_CHECK.C

A check of internet play's signatures and the server browser's listings
(port/linux/src/p2p_crypto.c, port/linux/src/p2p_lobby.c), built with the
platform layer's flags by tools/test_linux_port.py: RFC 8032's Ed25519
vectors; the X25519 key a seed gives against its Ed25519 key's conversion;
bad signatures, other keys and small-order keys turned away; and a listing
from a host (this program, through stand-ins for p2p.c and p2p_signal.c) to
a browser (this program again): taken, and a tampered one, one on another's
slot, an older one, a retained one of long ago, a tombstone and a listing
older than it each handled as they must be; a game with a password,
listed locked (its token nowhere in the listing), opened by its password
alone; and the hosts' pings: a game's host probed with its listing's token,
its ping listed and kept, probed again on Refresh (marked as Refresh's
until that probe ends, a probe of its own two minutes on not), and a game
with a password never; and the probes started in the order the games are
shown (p2p_lobby_shown), those not shown last, past one refused, none
while listings wait to be checked. Prints PASS or the failures.
*/

#include "p2p_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- stand-ins for the rest of internet play */

pthread_mutex_t p2p_lock = PTHREAD_MUTEX_INITIALIZER;

static unsigned long clock_now = 1000;
/* the clock's advance for each key hashed (a listing's check takes time) */
static unsigned long check_cost;
static unsigned char seed[P2P_SEED_SIZE] = { 42 };
static unsigned char signing_key[P2P_KEY_SIZE], x25519_secret[P2P_KEY_SIZE], x25519_public[P2P_KEY_SIZE];
static unsigned char published[512];
static int published_size, published_closing, publish_count;
/* the game hosted (its invite's token; NULL: none) */
static const unsigned char *hosting_token;

void posix_random_bytes(void *buffer, unsigned long size) { memset(buffer, 0x5a, size); }
unsigned long p2p_now(void) { return clock_now; }
int config_boolean(const char *name) { (void)name; return 1; }
void platform_log(const char *format, ...) { (void)format; }

void p2p_hex(const unsigned char *bytes, int size, char *text)
{
	int index;

	for (index = 0; index < size; index++)
		sprintf(text + 2 * index, "%02x", bytes[index]);
	text[2 * size] = 0;
}

void p2p_key_hash(const unsigned char *key, unsigned char *hash)
{
	unsigned char digest[P2P_SHA256_SIZE];

	clock_now += check_cost;
	p2p_sha256(key, P2P_KEY_SIZE, digest);
	memcpy(hash, digest, P2P_KEY_HASH_SIZE);
}

void p2p_identifier_from_hash(const unsigned char *hash, unsigned char *identifier)
{
	memcpy(identifier, hash, P2P_IDENTIFIER_SIZE);
	identifier[0] = (unsigned char)((identifier[0] & 0xFC) | 0x02);
}

const unsigned char *p2p_public_key(void) { return x25519_public; }
const unsigned char *p2p_signing_key(void) { return signing_key; }
void p2p_sign(const void *message, int size, unsigned char *signature)
{
	p2p_ed25519_sign(seed, signing_key, message, size, signature);
}
void p2p_new_invite_if_listed(void) {}
void p2p_signal_lobby_topics(int listed, int browsing) { (void)listed; (void)browsing; }
void p2p_signal_lobby_query(void) {}
void p2p_signal_lobby_publish(const unsigned char *listing, int size, int closing)
{
	memcpy(published, listing, (size_t)size);
	published_size = size;
	published_closing = closing;
	publish_count++;
}
void p2p_signal_lobby_quit(void) {}
/* (not a dedicated server, and no master servers: master_list.c) */
int halo_server_mode(void) { return 0; }
void master_list_browse(int on) { (void)on; }
void master_list_refresh(void) {}
/* (the token handed out with a listing, and the masters' prompt publish:
p2p.c, server_master.c) */
void p2p_token_listed(const unsigned char *token) { (void)token; }
void server_master_changed(void) {}

/* the probes asked for (p2p.c's), the last one's host and token, and the
hosts in the order probed: each ends at once, its host not reached (as
p2p.c's does when it needs no tunnel), but refused_host's is refused, and
none ends while probes_held; more than 64 asked for in one pass stops the
check */
static const unsigned char *refused_host;
static int probes_held;
static int probe_count, probe_asked;
static unsigned char probed_hash[P2P_KEY_HASH_SIZE], probed_token[P2P_TOKEN_SIZE];
static unsigned char probed_hosts[16][P2P_IDENTIFIER_SIZE];
int p2p_probe_start(const unsigned char *host_hash, const unsigned char *token)
{
	unsigned char host[P2P_IDENTIFIER_SIZE];

	if (++probe_asked > 64)
	{
		printf("FAIL: a game is probed again and again\n");
		exit(1);
	}
	memcpy(probed_hash, host_hash, P2P_KEY_HASH_SIZE);
	memcpy(probed_token, token, P2P_TOKEN_SIZE);
	p2p_identifier_from_hash(host_hash, host);
	if (refused_host && !memcmp(host, refused_host, P2P_IDENTIFIER_SIZE))
		return 0;
	memcpy(probed_hosts[probe_count % 16], host, P2P_IDENTIFIER_SIZE);
	probe_count++;
	if (!probes_held)
		p2p_lobby_probed(host, -1);
	return 1;
}

/* the p2p thread's calls: under p2p_lock, as there (p2p_lobby_update lets
go of it while it checks signatures) */
static void lobby_update(const unsigned char *token, int player_count, int maximum_player_count)
{
	probe_asked = 0;
	pthread_mutex_lock(&p2p_lock);
	p2p_lobby_update(token, player_count, maximum_player_count);
	pthread_mutex_unlock(&p2p_lock);
}

/* ---------- the checks */

static int failures;

static void check(int good, const char *what)
{
	if (!good)
	{
		failures++;
		printf("FAIL: %s\n", what);
	}
}

static void unhex(const char *text, unsigned char *bytes)
{
	size_t index;

	for (index = 0; index < strlen(text) / 2; index++)
	{
		unsigned int byte;

		sscanf(text + 2 * index, "%2x", &byte);
		bytes[index] = (unsigned char)byte;
	}
}

static void rfc8032(const char *seed_hex, const char *public_hex, const char *message_hex, const char *signature_hex)
{
	unsigned char test_seed[32], expected_public[32], public_key[32], message[64], expected[64], signature[64];
	int size = (int)strlen(message_hex) / 2;

	unhex(seed_hex, test_seed);
	unhex(public_hex, expected_public);
	unhex(message_hex, message);
	unhex(signature_hex, expected);
	p2p_ed25519_public(test_seed, public_key, NULL);
	p2p_ed25519_sign(test_seed, public_key, message, size, signature);
	check(!memcmp(public_key, expected_public, 32), "RFC 8032 public key");
	check(!memcmp(signature, expected, 64), "RFC 8032 signature");
	check(p2p_ed25519_verify(public_key, message, size, signature), "RFC 8032 verification");
}

static void crypto_checks(void)
{
	unsigned char test_seed[32], ed[32], secret[32], derived[32], converted[32], signature[64], other_ed[32];
	unsigned char small[32] = { 1 };
	unsigned char forged[64] = { 1 };
	int index;

	rfc8032("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60",
		"d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a", "",
		"e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b");
	rfc8032("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb",
		"3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c", "72",
		"92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00");
	/* the X25519 key from a seed is its Ed25519 key's conversion, and
	agrees on secrets */
	for (index = 0; index < 16; index++)
	{
		unsigned char other[32], other_public[32], first[32], second[32];

		memset(test_seed, index * 13 + 1, sizeof(test_seed));
		p2p_ed25519_public(test_seed, ed, secret);
		p2p_x25519(derived, secret, NULL);
		check(p2p_ed25519_to_x25519(ed, converted) && !memcmp(converted, derived, 32), "X25519 key of a seed");
		memset(other, index + 99, sizeof(other));
		p2p_x25519(other_public, other, NULL);
		p2p_x25519(first, secret, other_public);
		p2p_x25519(second, other, converted);
		check(!memcmp(first, second, 32), "secrets agree");
	}
	memset(test_seed, 3, sizeof(test_seed));
	p2p_ed25519_public(test_seed, ed, NULL);
	p2p_ed25519_sign(test_seed, ed, "listing", 7, signature);
	check(p2p_ed25519_verify(ed, "listing", 7, signature), "a good signature");
	check(!p2p_ed25519_verify(ed, "listinG", 7, signature), "another message");
	signature[5] ^= 1;
	check(!p2p_ed25519_verify(ed, "listing", 7, signature), "an altered R");
	signature[5] ^= 1;
	signature[40] ^= 1;
	check(!p2p_ed25519_verify(ed, "listing", 7, signature), "an altered S");
	signature[40] ^= 1;
	memset(test_seed, 4, sizeof(test_seed));
	p2p_ed25519_public(test_seed, other_ed, NULL);
	check(!p2p_ed25519_verify(other_ed, "listing", 7, signature), "another key");
	/* the identity, an order-8 point, and a signature anyone makes for the
	identity */
	check(!p2p_ed25519_to_x25519(small, converted), "the identity's conversion");
	unhex("c7176a703d4dd84fba3c0b760d10670f2a2053fa2c39ccc64ec7fd7792ac037a", small);
	check(!p2p_ed25519_to_x25519(small, converted), "an order-8 key's conversion");
	memset(small, 0, sizeof(small));
	small[0] = 1;
	check(!p2p_ed25519_verify(small, "anything", 8, forged), "a small-order key's signature");
}

static int games(struct p2p_listing *listing)
{
	struct p2p_listing all[4];
	int count = p2p_lobby_games(all, 4);

	if (count && listing)
		*listing = all[0];
	return count;
}

/* a copy of a listing with another time, signed again (as its host would
have, then) */
static void restamp(const unsigned char *payload, int size, long offset, unsigned char *copy)
{
	static const char label[] = "hceu-lobby-1";
	unsigned char data[sizeof(label) - 1 + 512];
	unsigned long when = (unsigned long)time(NULL) + (unsigned long)offset;
	int signed_size = size - P2P_SIGNATURE_SIZE;

	memcpy(copy, payload, (size_t)size);
	copy[10] = (unsigned char)(when >> 24);
	copy[11] = (unsigned char)(when >> 16);
	copy[12] = (unsigned char)(when >> 8);
	copy[13] = (unsigned char)when;
	memcpy(data, label, sizeof(label) - 1);
	memcpy(data + sizeof(label) - 1, copy, (size_t)signed_size);
	p2p_sign(data, (int)(sizeof(label) - 1) + signed_size, copy + signed_size);
}

/* a payload heard on the host's slot (or another), then the browser's pass */
static void hear(const unsigned char *payload, int size, int retained, const char *slot)
{
	unsigned char hash[P2P_KEY_HASH_SIZE];
	char own[2 * P2P_KEY_HASH_SIZE + 1];

	p2p_key_hash(x25519_public, hash);
	p2p_hex(hash, P2P_KEY_HASH_SIZE, own);
	pthread_mutex_lock(&p2p_lock);
	p2p_lobby_slot_heard(slot ? slot : own, payload, size, retained);
	pthread_mutex_unlock(&p2p_lock);
	lobby_update(hosting_token, 3, 16);
}

static void lobby_checks(void)
{
	static const unsigned char token[P2P_TOKEN_SIZE] = { 7, 7, 7 };
	unsigned char first[512], tampered[512], old[512];
	int first_size, count;
	struct p2p_listing listing;
	char expected_invite[P2P_LINK_SIZE];

	p2p_ed25519_public(seed, signing_key, x25519_secret);
	p2p_x25519(x25519_public, x25519_secret, NULL);
	hosting_token = token;
	/* hosting a public game: listed */
	p2p_set_hosting_public(1);
	p2p_set_game_listing("Test game", "bloodgulch", "Slayer", 2, 0x81, 1, 0, 1);
	lobby_update(token, 3, 16);
	check(publish_count == 1 && !published_closing && p2p_lobby_listed(), "the host publishes its listing");
	memcpy(first, published, (size_t)published_size);
	first_size = published_size;
	/* the browser takes it */
	p2p_lobby_browse(1);
	hear(first, first_size, 0, NULL);
	count = games(&listing);
	check(count == 1, "the browser takes the listing");
	{
		unsigned char bytes[P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE];
		char text[2 * (P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE) + 1];

		p2p_key_hash(x25519_public, bytes);
		memcpy(bytes + P2P_KEY_HASH_SIZE, token, P2P_TOKEN_SIZE);
		p2p_hex(bytes, sizeof(bytes), text);
		snprintf(expected_invite, sizeof(expected_invite), "halo://join/%s", text);
	}
	check(count == 1 && !strcmp(listing.invite, expected_invite), "the listing's invite is the host's");
	{
		unsigned char hash[P2P_KEY_HASH_SIZE];

		p2p_key_hash(x25519_public, hash);
		check(probe_count == 1 && !memcmp(probed_hash, hash, P2P_KEY_HASH_SIZE) &&
			!memcmp(probed_token, token, P2P_TOKEN_SIZE), "the browser probes the host with the listing's token");
	}
	check(count == 1 && listing.ping == -1, "a host not reached has no ping");
	pthread_mutex_lock(&p2p_lock);
	p2p_lobby_probed(listing.identifier, 42);
	pthread_mutex_unlock(&p2p_lock);
	check(games(&listing) == 1 && listing.ping == 42, "the host's ping is listed");
	check(count == 1 && !strcmp(listing.name, "Test game") && !strcmp(listing.map, "bloodgulch") &&
		!strcmp(listing.gametype, "Slayer") && listing.player_count == 3 && listing.maximum_player_count == 16 &&
		listing.engine_type == 2 && listing.kind == 0x81 && listing.open && listing.has_teams && !listing.in_progress,
		"the listing's details");
	/* the next pass in the same millisecond as the probe's end (an even one) */
	count = probe_count;
	lobby_update(token, 3, 16);
	check(probe_count == count, "a host just timed is not probed again in the same millisecond");
	/* a change: published again after TRIGGER_INTERVAL */
	p2p_set_game_listing(NULL, NULL, NULL, 2, 0x81, 1, 1, 1);
	clock_now += 1000;
	lobby_update(token, 3, 16);
	check(publish_count == 1, "no publish sooner than 5 s after the last");
	clock_now += 5000;
	lobby_update(token, 3, 16);
	check(publish_count == 2, "a change published within 5 s");
	hear(published, published_size, 0, NULL);
	check(games(&listing) == 1 && listing.in_progress, "the browser takes the newer listing");
	check(listing.ping == 42 && probe_count == 1, "the ping is kept, and not timed again soon");
	p2p_lobby_refresh();
	lobby_update(token, 3, 16);
	check(probe_count == 2 && games(&listing) == 1 && listing.ping == 42,
		"Refresh times the ping again; one not reached keeps the last");
	/* Refresh's probe held open: the game marked as Refresh's until it
	ends; the next probe, past two minutes on (the host heard on its 30 s),
	not */
	probes_held = 1;
	p2p_lobby_refresh();
	check(games(&listing) == 1 && listing.refreshing, "Refresh marks the game it times again");
	lobby_update(token, 3, 16);
	check(probe_count == 3 && games(&listing) == 1 && listing.probing && listing.refreshing && listing.ping == 42,
		"the game is marked while Refresh's probe is under way");
	pthread_mutex_lock(&p2p_lock);
	p2p_lobby_probed(listing.identifier, 40);
	pthread_mutex_unlock(&p2p_lock);
	check(games(&listing) == 1 && !listing.probing && !listing.refreshing && listing.ping == 40,
		"the probe's end takes the mark off");
	for (count = 0; count < 5; count++)
	{
		clock_now += 30000;
		lobby_update(token, 3, 16);
		hear(published, published_size, 0, NULL);
	}
	check(probe_count == 4 && games(&listing) == 1 && listing.probing && !listing.refreshing && listing.ping == 40,
		"a probe two minutes on is not marked as Refresh's");
	pthread_mutex_lock(&p2p_lock);
	p2p_lobby_probed(listing.identifier, 42);
	pthread_mutex_unlock(&p2p_lock);
	probes_held = 0;
	/* the older one again: no change */
	hear(first, first_size, 0, NULL);
	check(games(&listing) == 1 && listing.in_progress, "an older listing is ignored");
	/* tampered: a newer sequence and another name, with the old signature */
	memcpy(tampered, published, (size_t)published_size);
	tampered[9] = (unsigned char)(tampered[9] + 50);
	tampered[66] = 'X';
	hear(tampered, published_size, 0, NULL);
	check(games(&listing) == 1 && listing.name[0] == 'T', "a tampered listing is ignored");
	/* on another's slot */
	hear(published, published_size, 0, "00112233445566778899aabbccddeeff");
	check(games(NULL) == 1, "a listing on another's slot is ignored");
	/* an emptied slot is no news */
	hear(published, 0, 0, NULL);
	check(games(NULL) == 1, "an emptied slot is not a delete");
	/* expiry: 90 s unheard */
	clock_now += 91000;
	lobby_update(token, 3, 16);
	check(games(NULL) == 0, "a listing unheard for 90 s expires");
	/* signed long ago, published again by someone who kept it: ignored,
	live or retained */
	restamp(published, published_size, -2 * 3600, old);
	hear(old, published_size, 0, NULL);
	check(games(NULL) == 0, "a live listing of long ago is ignored");
	hear(old, published_size, 1, NULL);
	check(games(NULL) == 0, "a retained listing of long ago is ignored");
	/* a host whose clock is half an hour off: still listed live */
	restamp(published, published_size, -1800, old);
	hear(old, published_size, 0, NULL);
	check(games(NULL) == 1, "a live listing half an hour off is taken");
	clock_now += 91000;
	lobby_update(token, 3, 16);
	/* the slot's retained copy, of about now: taken (the host published it
	again on its 30 s) */
	lobby_update(token, 3, 16);
	hear(published, published_size, 1, NULL);
	check(games(NULL) == 1, "a retained listing of about now is taken");
	/* the host stops: a tombstone, which removes the game */
	p2p_set_hosting_public(0);
	lobby_update(token, 3, 16);
	check(published_closing && !p2p_lobby_listed(), "the host publishes a tombstone");
	hear(published, published_size, 0, NULL);
	check(games(NULL) == 0, "a tombstone removes the game");
	/* the listing before the tombstone does not bring it back */
	hear(first, first_size, 0, NULL);
	check(games(NULL) == 0, "a listing older than the tombstone is ignored");
	/* listed again (a new sequence): shown again */
	p2p_set_hosting_public(1);
	clock_now += 10000;
	lobby_update(token, 3, 16);
	hear(published, published_size, 0, NULL);
	check(games(NULL) == 1, "a listing newer than the tombstone is taken");
	/* a password: listed locked, its token sealed */
	p2p_set_hosting_password("hunter2");
	clock_now += 6000;
	lobby_update(token, 3, 16);
	hear(published, published_size, 0, NULL);
	check(games(&listing) == 1 && listing.locked && !listing.invite[0], "a game with a password is listed locked");
	count = probe_count;
	p2p_lobby_refresh();
	lobby_update(token, 3, 16);
	check(probe_count == count && games(&listing) == 1 && !listing.refreshing,
		"a game with a password is not probed, nor marked by Refresh");
	{
		int offset, found = 0;

		for (offset = 0; offset + P2P_TOKEN_SIZE <= published_size; offset++)
			found |= !memcmp(published + offset, token, P2P_TOKEN_SIZE);
		check(!found, "a locked listing does not hold its token");
	}
	check(!p2p_listing_unlock(&listing, "hunter3") && !listing.invite[0], "a wrong password opens nothing");
	check(!p2p_listing_unlock(&listing, "") && !listing.invite[0], "no password opens nothing");
	check(p2p_listing_unlock(&listing, "hunter2") && !strcmp(listing.invite, expected_invite),
		"the password opens the host's invite");
	/* the password taken off: listed open again */
	p2p_set_hosting_password(NULL);
	clock_now += 6000;
	lobby_update(token, 3, 16);
	hear(published, published_size, 0, NULL);
	check(games(&listing) == 1 && !listing.locked && !strcmp(listing.invite, expected_invite),
		"a game whose password is taken off is listed open");
}

/* the server browser: the games shown in the order's (reverse: the other
way), and of those the first count only */
static void show(int order, int reverse, int count)
{
	struct p2p_listing all[8];
	int found = p2p_lobby_games(all, 8);

	p2p_lobby_sort(all, found, order, reverse);
	p2p_lobby_shown(all, count < found ? count : found);
}

/* four games, heard as Alpha, Bravo, Charlie, Delta (Charlie's join failed),
are shown as Bravo, Delta, Charlie, Alpha: their hosts are probed in the
order shown, past Bravo's refused, none while listings wait to be checked;
then in another order, and one not shown last */
static void probe_order_checks(void)
{
	static const unsigned char token[P2P_TOKEN_SIZE] = { 9, 9, 9 };
	static const char *names[] = { "Alpha", "Bravo", "Charlie", "Delta" };
	static const int player_counts[] = { 2, 9, 5, 5 }, first_wave[] = { 1, 2, 0, 3 };
	unsigned char hash[P2P_KEY_HASH_SIZE], payloads[4][512], identifiers[4][P2P_IDENTIFIER_SIZE];
	char slots[4][2 * P2P_KEY_HASH_SIZE + 1];
	int sizes[4], index;

	/* (the games before expire) */
	clock_now += 91000;
	lobby_update(token, 0, 16);
	hosting_token = token;
	for (index = 0; index < 4; index++)
	{
		seed[0] = (unsigned char)(index + 1);
		p2p_ed25519_public(seed, signing_key, x25519_secret);
		p2p_x25519(x25519_public, x25519_secret, NULL);
		p2p_set_hosting_public(1);
		p2p_set_game_listing(names[index], "bloodgulch", "Slayer", 2, 0, 1, 0, 0);
		clock_now += 6000;
		lobby_update(token, player_counts[index], 16);
		memcpy(payloads[index], published, (size_t)published_size);
		sizes[index] = published_size;
		p2p_key_hash(x25519_public, hash);
		p2p_hex(hash, P2P_KEY_HASH_SIZE, slots[index]);
		p2p_identifier_from_hash(hash, identifiers[index]);
		p2p_set_hosting_public(0);
		lobby_update(token, 0, 16);
	}
	p2p_lobby_mark_failed(identifiers[2]);
	/* heard in one pass, Bravo refused */
	probe_count = 0;
	refused_host = identifiers[1];
	pthread_mutex_lock(&p2p_lock);
	for (index = 0; index < 4; index++)
		p2p_lobby_slot_heard(slots[index], payloads[index], sizes[index], 0);
	pthread_mutex_unlock(&p2p_lock);
	lobby_update(token, 0, 16);
	refused_host = NULL;
	check(probe_count == 3, "the games after one refused are probed");
	/* gone, then heard again in a burst that takes four passes to check */
	clock_now += 91000;
	lobby_update(token, 0, 16);
	probe_count = 0;
	check_cost = 5;
	pthread_mutex_lock(&p2p_lock);
	for (index = 0; index < 4; index++)
		p2p_lobby_slot_heard(slots[index], payloads[index], sizes[index], 0);
	pthread_mutex_unlock(&p2p_lock);
	for (index = 0; index < 3; index++)
	{
		lobby_update(token, 0, 16);
		check(probe_count == 0, "no probe starts while listings wait to be checked");
		show(P2P_LOBBY_SORT_SERVERS, 0, 4);
	}
	lobby_update(token, 0, 16);
	check_cost = 0;
	check(probe_count == 4, "the four games are probed once checked");
	/* (Delta, checked in the pass the probes start, is not shown yet) */
	for (index = 0; index < 4; index++)
		check(!memcmp(probed_hosts[index], identifiers[first_wave[index]], P2P_IDENTIFIER_SIZE),
			"the games are probed in the order shown, one not shown yet last");
	/* sorted by name, then Refresh: Alpha, Bravo, Charlie, Delta */
	show(P2P_LOBBY_SORT_NAME, 0, 4);
	probe_count = 0;
	p2p_lobby_refresh();
	lobby_update(token, 0, 16);
	check(probe_count == 4, "the four games are probed again on Refresh");
	for (index = 0; index < 4; index++)
		check(!memcmp(probed_hosts[index], identifiers[index], P2P_IDENTIFIER_SIZE),
			"the games are probed in the order the list is sorted by now");
	/* by name the other way, Alpha filtered out: Delta, Charlie, Bravo, then
	Alpha */
	show(P2P_LOBBY_SORT_NAME, 1, 3);
	probe_count = 0;
	p2p_lobby_refresh();
	lobby_update(token, 0, 16);
	for (index = 0; index < 4; index++)
		check(!memcmp(probed_hosts[index], identifiers[3 - index], P2P_IDENTIFIER_SIZE),
			"a game not shown is probed after those shown");
	/* Alpha listed by a master server too: neither marked by Refresh nor
	timed */
	check(p2p_lobby_take_master_listing(payloads[0], sizes[0], "", "EU", 0, 1, NULL), "a master server's listing is taken");
	probe_count = 0;
	p2p_lobby_refresh();
	{
		struct p2p_listing all[4];
		int count = p2p_lobby_games(all, 4), marked = 0;

		for (index = 0; index < count; index++)
			marked += all[index].refreshing;
		check(count == 4 && marked == 3, "Refresh marks the games it times, not a master server's");
	}
	lobby_update(token, 0, 16);
	check(probe_count == 3, "a master server's game is not timed on Refresh");
}

int main(void)
{
	crypto_checks();
	lobby_checks();
	probe_order_checks();
	printf("%s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
	return failures != 0;
}

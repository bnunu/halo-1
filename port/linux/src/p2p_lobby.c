/*
P2P_LOBBY.C

Internet play's public games (the server browser, Join Game > Server
Browser): a public game's host publishes a signed listing of it through the
signalling brokers (p2p_signal.c), and browsers gather the listings and join
a game by the invite a listing holds, as an invite link would.

The listing (network byte order) is signed with the host's Ed25519 key,
whose X25519 form's hash is the invite's host part (p2p.c), so no one but
the host can list its invite, alter its listing, or list another's under
false details:

	"HL", format 1, the lobby version (2: HALO_PORT_NETWORK_VERSION; browsers
	hide others), flags (open, under way, teams, closed, password), sequence
	(4: newest wins), Unix time (4), the Ed25519 key (32), the invite's token
	(16; a password's game's sealed with the password's key, 56:
	p2p_seal_token; a tombstone's zero), players, most players, the
	gametype's engine (1 each), the game's name,
	map and gametype (a length byte, then up to 32, 32 and 24 characters of
	printable ASCII), a stamp (8: the game's kind, an OpenCE gametype's mode
	and Hardcore, port/linux/game/game_modes.h's game_modes_kind; the rest
	reserved, zero), and the signature (64) of "hceu-lobby-1" and all before
	it.

Each host has a slot, hceu/3/lobby/s/<its key's hash in hex>: its listing,
retained on every broker (and expiring there after LISTING_EXPIRY seconds
on MQTT 5), and the broker's will (set at the connection, p2p_signal.c)
clears it if the host vanishes. Browsers subscribe to every slot and ask the
hosts to publish again (hceu/3/lobby/q). Brokers are other people's: what
one deletes or keeps is no authority, so
- a host publishes every REPUBLISH_INTERVAL, and sooner (at most once each
  TRIGGER_INTERVAL) when its game changes, when asked, and when its slot is
  heard to hold anything but its listing (cleared, another's, or an older
  one of its own): a deletion is undone in seconds;
- a browser takes only a signed listing whose key's hash is its slot's,
  keeps the newest of each host (by sequence), drops one not heard in
  GAME_EXPIRY (on its own clock; a retained copy is taken only if its time
  is within RETAINED_WINDOW of this machine's, from a host that died and
  left it, and a live one only within LIVE_WINDOW, so an old listing
  published again by someone else does not list a game long gone), takes
  a host's closing listing (a tombstone) as its end, and ignores an
  emptied slot (a wipe is not a delete).
A host that stops publishes a tombstone, then clears its slot. Going private
makes a new invite (p2p.c), so a listing seen before lets no one in.

A game with a password (Server Setup's PASSWORD: p2p_set_hosting_password)
is listed with its token sealed with the password's key (Argon2id of the
password, salted with the host's key: p2p_password_key), so that only who
knows the password can join it from the server browser
(p2p_listing_unlock); its invite link, which holds the token, still joins it
as any invite does. Setting or changing the password makes a new invite, as
going private does.

Checking signatures takes work, and anyone can send listings: at most
VERIFY_BUDGET milliseconds of it each pass of the p2p thread, from a queue
of MAXIMUM_QUEUED; the rest is dropped (hosts publish again). A listing that
could not be shown (another network version's, or one outside its time
window) is dropped before it takes a place there (listing_wanted): on
subscribing, a broker sends every slot it keeps, and on a public one most
are games long gone, which filled the queue and crowded out live games.

Each game's host is probed for its ping (p2p_probe_start: the tunnel to it
timed, p2p.c), a few at once, again each PROBE_REFRESH and on Refresh. A
game with a password is not: its token is sealed.
*/

#include "platform.h"
#include "posix.h"
#include "port_config.h"
#include "p2p_internal.h"
#include "halo_port_limits.h"
#include "server_mode.h"
#include "server_master.h"
#include "master_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum
{
	LISTING_FORMAT = 1,
	_listing_open = 1,
	_listing_in_progress = 2,
	_listing_has_teams = 4,
	/* a tombstone: the host stopped */
	_listing_closed = 8,
	/* its token sealed with a password's key */
	_listing_password = 16,
	/* a dedicated server's (halo --server: server_mode.c) */
	_listing_dedicated = 32,
	STAMP_SIZE = 8,
	/* the signed part's end: everything but the signature */
	MAXIMUM_LISTING_SIZE = 2 + 1 + 2 + 1 + 4 + 4 + P2P_KEY_SIZE + P2P_SEALED_TOKEN_SIZE + 3 +
		(1 + P2P_LISTING_NAME_SIZE) + (1 + P2P_LISTING_MAP_SIZE) + (1 + P2P_LISTING_GAMETYPE_SIZE) + STAMP_SIZE +
		P2P_SIGNATURE_SIZE,
	MINIMUM_LISTING_SIZE = 2 + 1 + 2 + 1 + 4 + 4 + P2P_KEY_SIZE + P2P_TOKEN_SIZE + 3 + 3 + STAMP_SIZE +
		P2P_SIGNATURE_SIZE,
	MAXIMUM_GAMES = 256,
	MAXIMUM_TOMBSTONES = 256,
	MAXIMUM_QUEUED = 128,

	/* milliseconds */
	REPUBLISH_INTERVAL = 30000,
	TRIGGER_INTERVAL = 5000,
	QUERY_JITTER = 2000,
	GAME_EXPIRY = 90000,
	PROBE_REFRESH = 120000,
	TOMBSTONE_TIME = 600000,
	VERIFY_BUDGET = 2,
	/* seconds */
	RETAINED_WINDOW = 600,
	/* (wide: a host whose clock is off is still listed) */
	LIVE_WINDOW = 3600,
};

static const char signature_label[] = "hceu-lobby-1";

/* (p2p.h's sizes of a locked listing's are p2p_internal.h's) */
typedef char check_listing_sizes[P2P_LISTING_SIGNING_KEY_SIZE == P2P_KEY_SIZE &&
	P2P_LISTING_KEY_HASH_SIZE == P2P_KEY_HASH_SIZE && P2P_LISTING_SEALED_TOKEN_SIZE == P2P_SEALED_TOKEN_SIZE ? 1 : -1];

/* a listing, read */
struct listing
{
	int version;
	int flags;
	unsigned long sequence;
	unsigned long time;
	unsigned char key[P2P_KEY_SIZE];
	unsigned char token[P2P_TOKEN_SIZE];
	/* (_listing_password: its token, sealed) */
	unsigned char sealed_token[P2P_SEALED_TOKEN_SIZE];
	int player_count, maximum_player_count, engine_type, kind;
	char name[P2P_LISTING_NAME_SIZE + 1];
	char map[P2P_LISTING_MAP_SIZE + 1];
	char gametype[P2P_LISTING_GAMETYPE_SIZE + 1];
	/* the signed part's size */
	int signed_size;
};

struct game
{
	int used;
	unsigned char key_hash[P2P_KEY_HASH_SIZE];
	unsigned long sequence;
	unsigned long heard_time;
	/* as it came (one the same again needs no check) */
	unsigned char payload[MAXIMUM_LISTING_SIZE];
	int payload_size;
	struct p2p_listing listing;
	/* its invite's token (none: a password's game); its host's ping
	(-1: not known), whether it is being probed, and when it last was (0:
	due; else odd, and never ahead of the clock, which elapsed reads as
	long past) */
	int has_token;
	unsigned char token[P2P_TOKEN_SIZE];
	int ping;
	int probing;
	unsigned long probed_time;
	/* listed by a master server (master_list.c) lately: not probed (the
	player's address is not told to every server listed), and when */
	int master;
	/* (a master's entry's word that the game is a dedicated server's: a
	player's public game a publisher key publishes is not) */
	int master_dedicated;
	unsigned long master_time;
};

struct tombstone
{
	unsigned char key_hash[P2P_KEY_HASH_SIZE];
	unsigned long sequence;
	unsigned long time;
};

struct queued
{
	unsigned char key_hash[P2P_KEY_HASH_SIZE];
	unsigned char payload[MAXIMUM_LISTING_SIZE];
	int size;
	int retained;
};

static struct
{
	/* hosting: the game as the game's server tells it
	(p2p_set_game_listing), and whether it is public (p2p_set_hosting_public) */
	int public;
	char name[P2P_LISTING_NAME_SIZE + 1];
	char map[P2P_LISTING_MAP_SIZE + 1];
	char gametype[P2P_LISTING_GAMETYPE_SIZE + 1];
	int engine_type;
	int kind;
	int flags;
	int player_count, maximum_player_count;
	/* the counts the last publish was made with (a change publishes again) */
	int published_player_count, published_maximum_player_count;
	/* the password's key (p2p_set_hosting_password), if it has one */
	int has_password;
	unsigned char password_key[P2P_PASSWORD_KEY_SIZE];
	/* hosting for the internet: its invite's token, for the master servers'
	listing too (p2p_lobby_host_listing) */
	int hosting_token_valid;
	unsigned char hosting_token[P2P_TOKEN_SIZE];
	/* listed: with this token; its listing as last published */
	int listed;
	unsigned char token[P2P_TOKEN_SIZE];
	unsigned long sequence;
	unsigned char listing[MAXIMUM_LISTING_SIZE];
	int listing_size;
	unsigned long published_time;
	/* a publish wanted (the game changed, a query, the slot needs mending),
	not before this */
	int republish_wanted;
	unsigned long republish_time;

	/* browsing */
	int browsing;
	int query_wanted;
	struct queued queue[MAXIMUM_QUEUED];
	int queue_count;
	struct game games[MAXIMUM_GAMES];
	struct tombstone tombstones[MAXIMUM_TOMBSTONES];
	int next_tombstone;
	/* the identifiers of games joining failed */
	unsigned char failed[MAXIMUM_GAMES][P2P_IDENTIFIER_SIZE];
	int failed_count;
	/* the hosts heard listing a game of a newer network version, which are
	not shown (an old build sees none of the games: the browser says why),
	each forgotten as a game is (GAME_EXPIRY) */
	struct
	{
		unsigned char key_hash[P2P_KEY_HASH_SIZE];
		unsigned long heard_time;
		int used;
	} newer[MAXIMUM_GAMES];
} lobby;

static int elapsed(unsigned long since, unsigned long time)
{
	return (unsigned int)(p2p_now() - since) >= (unsigned int)time;
}

/* printable ASCII, as the menus' font has it (others: '?'), cut to size */
static void sanitize(char *destination, int size, const char *source, int source_size)
{
	int index;

	for (index = 0; index < size && index < source_size && source[index]; index++)
	{
		unsigned char character = (unsigned char)source[index];

		destination[index] = character >= 0x20 && character < 0x7F ? (char)character : '?';
	}
	destination[index] = 0;
}

void p2p_lobby_slot_topic(const unsigned char *key_hash, char *topic, int size)
{
	char text[2 * P2P_KEY_HASH_SIZE + 1];

	p2p_hex(key_hash, P2P_KEY_HASH_SIZE, text);
	snprintf(topic, (size_t)size, "%s%s", P2P_LOBBY_SLOT_PREFIX, text);
}

/* the hash of an Ed25519 key's X25519 form (an invite's host part); 0 if
the key has a small order */
static int signing_key_hash(const unsigned char *key, unsigned char *hash)
{
	unsigned char x25519[P2P_KEY_SIZE];

	if (!p2p_ed25519_to_x25519(key, x25519))
		return 0;
	p2p_key_hash(x25519, hash);
	return 1;
}

/* ---------- the listing */

static void put_long(unsigned char *bytes, unsigned long value)
{
	bytes[0] = (unsigned char)(value >> 24);
	bytes[1] = (unsigned char)(value >> 16);
	bytes[2] = (unsigned char)(value >> 8);
	bytes[3] = (unsigned char)value;
}

static unsigned long get_long(const unsigned char *bytes)
{
	return (unsigned long)bytes[0] << 24 | (unsigned long)bytes[1] << 16 | (unsigned long)bytes[2] << 8 | bytes[3];
}

static int put_text(unsigned char *bytes, const char *text, int maximum)
{
	int length = (int)strlen(text);

	length = length > maximum ? maximum : length;
	bytes[0] = (unsigned char)length;
	memcpy(bytes + 1, text, (size_t)length);
	return 1 + length;
}

/* the listing of the game hosted, signed: flags (_listing_closed for a
tombstone), with the invite's token */
static int listing_make_token(unsigned char *bytes, int flags, const unsigned char *token)
{
	unsigned char *signed_part = bytes;
	unsigned char data[sizeof(signature_label) - 1 + MAXIMUM_LISTING_SIZE];
	int size = 0;

	if (lobby.has_password && !(flags & _listing_closed))
		flags |= _listing_password;
	if (halo_server_mode())
		flags |= _listing_dedicated;
	bytes[size++] = 'H';
	bytes[size++] = 'L';
	bytes[size++] = LISTING_FORMAT;
	bytes[size++] = (unsigned char)(HALO_PORT_NETWORK_VERSION >> 8);
	bytes[size++] = (unsigned char)HALO_PORT_NETWORK_VERSION;
	bytes[size++] = (unsigned char)flags;
	put_long(bytes + size, ++lobby.sequence);
	size += 4;
	put_long(bytes + size, (unsigned long)time(NULL));
	size += 4;
	memcpy(bytes + size, p2p_signing_key(), P2P_KEY_SIZE);
	size += P2P_KEY_SIZE;
	if (flags & _listing_password)
	{
		p2p_seal_token(lobby.password_key, p2p_signing_key(), token, bytes + size);
		size += P2P_SEALED_TOKEN_SIZE;
	}
	else
	{
		/* (a tombstone's none: it may end a game with a password) */
		if (flags & _listing_closed)
			memset(bytes + size, 0, P2P_TOKEN_SIZE);
		else
			memcpy(bytes + size, token, P2P_TOKEN_SIZE);
		size += P2P_TOKEN_SIZE;
	}
	bytes[size++] = (unsigned char)(lobby.player_count > 255 ? 255 : lobby.player_count);
	bytes[size++] = (unsigned char)(lobby.maximum_player_count > 255 ? 255 : lobby.maximum_player_count);
	bytes[size++] = (unsigned char)lobby.engine_type;
	size += put_text(bytes + size, lobby.name, P2P_LISTING_NAME_SIZE);
	size += put_text(bytes + size, lobby.map, P2P_LISTING_MAP_SIZE);
	size += put_text(bytes + size, lobby.gametype, P2P_LISTING_GAMETYPE_SIZE);
	memset(bytes + size, 0, STAMP_SIZE);
	bytes[size] = (unsigned char)lobby.kind;
	size += STAMP_SIZE;
	memcpy(data, signature_label, sizeof(signature_label) - 1);
	memcpy(data + sizeof(signature_label) - 1, signed_part, (size_t)size);
	p2p_sign(data, (int)(sizeof(signature_label) - 1) + size, bytes + size);
	return size + P2P_SIGNATURE_SIZE;
}

static int listing_make(unsigned char *bytes, int flags)
{
	return listing_make_token(bytes, flags, lobby.token);
}

static int get_text(const unsigned char *bytes, int size, int *offset, char *text, int maximum)
{
	int length;

	if (*offset >= size)
		return 0;
	length = bytes[(*offset)++];
	if (length > maximum || *offset + length > size)
		return 0;
	sanitize(text, maximum, (const char *)bytes + *offset, length);
	*offset += length;
	return 1;
}

/* reads a listing (not its signature); 0 if it is malformed */
static int listing_read(const unsigned char *bytes, int size, struct listing *listing)
{
	int offset = 0;

	if (size < MINIMUM_LISTING_SIZE || size > MAXIMUM_LISTING_SIZE || bytes[0] != 'H' || bytes[1] != 'L' ||
		bytes[2] != LISTING_FORMAT)
	{
		return 0;
	}
	offset = 3;
	listing->version = bytes[offset] << 8 | bytes[offset + 1];
	offset += 2;
	listing->flags = bytes[offset++];
	listing->sequence = get_long(bytes + offset);
	offset += 4;
	listing->time = get_long(bytes + offset);
	offset += 4;
	memcpy(listing->key, bytes + offset, P2P_KEY_SIZE);
	offset += P2P_KEY_SIZE;
	if (listing->flags & _listing_password)
	{
		/* (its token sealed, which is longer: the counts after it too) */
		if (offset + P2P_SEALED_TOKEN_SIZE + 3 > size)
			return 0;
		memset(listing->token, 0, P2P_TOKEN_SIZE);
		memcpy(listing->sealed_token, bytes + offset, P2P_SEALED_TOKEN_SIZE);
		offset += P2P_SEALED_TOKEN_SIZE;
	}
	else
	{
		memcpy(listing->token, bytes + offset, P2P_TOKEN_SIZE);
		offset += P2P_TOKEN_SIZE;
	}
	listing->player_count = bytes[offset++];
	listing->maximum_player_count = bytes[offset++];
	listing->engine_type = bytes[offset++];
	if (!get_text(bytes, size, &offset, listing->name, P2P_LISTING_NAME_SIZE) ||
		!get_text(bytes, size, &offset, listing->map, P2P_LISTING_MAP_SIZE) ||
		!get_text(bytes, size, &offset, listing->gametype, P2P_LISTING_GAMETYPE_SIZE))
	{
		return 0;
	}
	/* (the stamp: the game's kind first; a listing too short for it fails
	the size check below) */
	listing->kind = offset < size ? bytes[offset] : 0;
	offset += STAMP_SIZE;
	if (offset + P2P_SIGNATURE_SIZE != size)
		return 0;
	listing->signed_size = offset;
	return 1;
}

/* whether a listing is of about now: a slot's retained copy within
RETAINED_WINDOW (a host that died left it, and nothing cleared it), a live
one within LIVE_WINDOW (not one long gone, published again by someone who
kept it) */
static int listing_current(const struct listing *listing, int retained)
{
	long difference = (long)(listing->time - (unsigned long)time(NULL));
	long window = retained ? RETAINED_WINDOW : LIVE_WINDOW;

	return difference <= window && difference >= -window;
}

static int listing_signed(const unsigned char *bytes, const struct listing *listing)
{
	unsigned char data[sizeof(signature_label) - 1 + MAXIMUM_LISTING_SIZE];

	memcpy(data, signature_label, sizeof(signature_label) - 1);
	memcpy(data + sizeof(signature_label) - 1, bytes, (size_t)listing->signed_size);
	return p2p_ed25519_verify(listing->key, data, (int)(sizeof(signature_label) - 1) + listing->signed_size,
		bytes + listing->signed_size);
}

/* ---------- hosting */

static void publish(void)
{
	lobby.listing_size = listing_make(lobby.listing, lobby.flags);
	lobby.published_time = p2p_now();
	lobby.republish_wanted = 0;
	p2p_signal_lobby_publish(lobby.listing, lobby.listing_size, 0);
}

/* a publish soon: at once if none was lately, else when TRIGGER_INTERVAL
has passed (plus jitter) */
static void republish_soon(unsigned long jitter)
{
	unsigned long earliest = lobby.published_time + TRIGGER_INTERVAL;
	unsigned long when = p2p_now() + jitter;

	if ((long)(earliest - when) > 0)
		when = earliest;
	if (!lobby.republish_wanted || (long)(lobby.republish_time - when) > 0)
		lobby.republish_time = when;
	lobby.republish_wanted = 1;
}

static void stop_listing(void)
{
	unsigned char tombstone[MAXIMUM_LISTING_SIZE];
	int size = listing_make(tombstone, _listing_closed);

	lobby.listed = 0;
	lobby.republish_wanted = 0;
	p2p_signal_lobby_publish(tombstone, size, 1);
	p2p_signal_lobby_topics(0, lobby.browsing);
	platform_log("Internet play: the game is no longer listed in the server browser");
}

static void update_hosting(const unsigned char *token, int player_count, int maximum_player_count)
{
	/* (a dedicated server is listed by the master servers, not among the
	games players host: server_master.c; server.list_on_brokers, until
	there are masters, lists it here too) */
	int want = token && lobby.public && config_boolean("network.public_lobby") &&
		(!halo_server_mode() || config_boolean("server.list_on_brokers"));

	lobby.hosting_token_valid = token != NULL;
	if (token)
		memcpy(lobby.hosting_token, token, P2P_TOKEN_SIZE);
	if (token)
	{
		lobby.player_count = player_count;
		lobby.maximum_player_count = maximum_player_count;
	}

	if (lobby.listed && (!want || memcmp(token, lobby.token, P2P_TOKEN_SIZE)))
		stop_listing();
	if (!want)
		return;
	if (player_count != lobby.published_player_count || maximum_player_count != lobby.published_maximum_player_count)
	{
		lobby.published_player_count = player_count;
		lobby.published_maximum_player_count = maximum_player_count;
		if (lobby.listed)
			republish_soon(0);
		/* (and to the master servers, if they list it) */
		server_master_changed();
	}
	if (!lobby.listed)
	{
		memcpy(lobby.token, token, P2P_TOKEN_SIZE);
		lobby.listed = 1;
		p2p_signal_lobby_topics(1, lobby.browsing);
		publish();
		platform_log("Internet play: the game is listed in everyone's server browser (public)");
		return;
	}
	if ((lobby.republish_wanted && (long)(p2p_now() - lobby.republish_time) >= 0) ||
		elapsed(lobby.published_time, REPUBLISH_INTERVAL))
	{
		publish();
	}
}

void p2p_lobby_query_heard(void)
{
	if (lobby.listed)
	{
		unsigned short jitter;

		posix_random_bytes(&jitter, sizeof(jitter));
		republish_soon(jitter % QUERY_JITTER);
	}
}

/* the host's own slot was heard: anything but its listing is mended */
static void own_slot_heard(const unsigned char *payload, int size, int retained)
{
	struct listing listing;

	if (!lobby.listed || (size == lobby.listing_size && !memcmp(payload, lobby.listing, (size_t)size)))
		return;
	/* (an older listing of its own, forwarded live, is the echo of a publish
	before the last; as the slot's retained copy, the slot holds it) */
	if (!retained && listing_read(payload, size, &listing) && !memcmp(listing.key, p2p_signing_key(), P2P_KEY_SIZE) &&
		listing.sequence < lobby.sequence)
	{
		return;
	}
	republish_soon(0);
}

/* ---------- browsing */

static struct game *find_game(const unsigned char *key_hash)
{
	int index;

	for (index = 0; index < MAXIMUM_GAMES; index++)
	{
		if (lobby.games[index].used && !memcmp(lobby.games[index].key_hash, key_hash, P2P_KEY_HASH_SIZE))
			return &lobby.games[index];
	}
	return NULL;
}

static struct tombstone *find_tombstone(const unsigned char *key_hash)
{
	int index;

	for (index = 0; index < MAXIMUM_TOMBSTONES; index++)
	{
		struct tombstone *tombstone = &lobby.tombstones[index];

		if (tombstone->time && !elapsed(tombstone->time, TOMBSTONE_TIME) &&
			!memcmp(tombstone->key_hash, key_hash, P2P_KEY_HASH_SIZE))
		{
			return tombstone;
		}
	}
	return NULL;
}

static int identifier_failed(const unsigned char *identifier)
{
	int index;

	for (index = 0; index < lobby.failed_count; index++)
	{
		if (!memcmp(lobby.failed[index], identifier, P2P_IDENTIFIER_SIZE))
			return 1;
	}
	return 0;
}

static int listing_wanted(const unsigned char *payload, int size, int retained, const unsigned char *key_hash);

void p2p_lobby_slot_heard(const char *hash_text, const unsigned char *payload, int size, int retained)
{
	unsigned char key_hash[P2P_KEY_HASH_SIZE];
	struct game *game;
	struct queued *queued;
	int index;

	if (strlen(hash_text) != 2 * P2P_KEY_HASH_SIZE || strspn(hash_text, "0123456789abcdef") != 2 * P2P_KEY_HASH_SIZE)
		return;
	for (index = 0; index < P2P_KEY_HASH_SIZE; index++)
	{
		unsigned int byte;

		sscanf(hash_text + 2 * index, "%2x", &byte);
		key_hash[index] = (unsigned char)byte;
	}
	{
		unsigned char own[P2P_KEY_HASH_SIZE];

		p2p_key_hash(p2p_public_key(), own);
		if (!memcmp(own, key_hash, P2P_KEY_HASH_SIZE))
			own_slot_heard(payload, size, retained);
	}
	/* (an emptied slot is no news: a wipe is not a delete) */
	if (!lobby.browsing || size < MINIMUM_LISTING_SIZE || size > MAXIMUM_LISTING_SIZE)
		return;
	/* the same listing again: heard, no work */
	game = find_game(key_hash);
	if (game && game->payload_size == size && !memcmp(game->payload, payload, (size_t)size))
	{
		game->heard_time = p2p_now();
		return;
	}
	if (!listing_wanted(payload, size, retained, key_hash))
		return;
	for (index = 0; index < lobby.queue_count; index++)
	{
		if (lobby.queue[index].size == size && !memcmp(lobby.queue[index].payload, payload, (size_t)size))
			return;
	}
	if (lobby.queue_count == MAXIMUM_QUEUED)
		return;
	queued = &lobby.queue[lobby.queue_count++];
	memcpy(queued->key_hash, key_hash, P2P_KEY_HASH_SIZE);
	memcpy(queued->payload, payload, (size_t)size);
	queued->size = size;
	queued->retained = retained;
}

/* a queued listing, its signature checked: taken or not */
static void listing_take(const struct queued *queued, const struct listing *listing)
{
	struct tombstone *tombstone = find_tombstone(queued->key_hash);
	struct game *game = find_game(queued->key_hash);
	struct p2p_listing *shown;
	unsigned char bytes[P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE];
	char text[2 * (P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE) + 1];
	int index;

	if (tombstone && listing->sequence <= tombstone->sequence)
		return;
	if (game && listing->sequence <= game->sequence)
		return;
	if (listing->flags & _listing_closed)
	{
		if (!tombstone)
		{
			tombstone = &lobby.tombstones[lobby.next_tombstone];
			lobby.next_tombstone = (lobby.next_tombstone + 1) % MAXIMUM_TOMBSTONES;
			memcpy(tombstone->key_hash, queued->key_hash, P2P_KEY_HASH_SIZE);
		}
		tombstone->sequence = listing->sequence;
		tombstone->time = p2p_now() ? p2p_now() : 1;
		if (game)
			game->used = 0;
		return;
	}
	if (!game)
	{
		/* the room of one gone longest, if there is none free */
		for (index = 0; index < MAXIMUM_GAMES && lobby.games[index].used; index++)
			;
		if (index == MAXIMUM_GAMES)
		{
			int oldest = 0;

			for (index = 1; index < MAXIMUM_GAMES; index++)
			{
				if ((long)(lobby.games[index].heard_time - lobby.games[oldest].heard_time) < 0)
					oldest = index;
			}
			index = oldest;
		}
		game = &lobby.games[index];
		memset(game, 0, sizeof(*game));
		memcpy(game->key_hash, queued->key_hash, P2P_KEY_HASH_SIZE);
		game->ping = -1;
		game->used = 1;
	}
	game->sequence = listing->sequence;
	game->heard_time = p2p_now();
	memcpy(game->payload, queued->payload, (size_t)queued->size);
	game->payload_size = queued->size;
	shown = &game->listing;
	shown->locked = (listing->flags & _listing_password) != 0;
	game->has_token = !shown->locked;
	memcpy(game->token, listing->token, P2P_TOKEN_SIZE);
	if (shown->locked)
	{
		/* (its invite once the password opens its token: p2p_listing_unlock) */
		shown->invite[0] = 0;
		memcpy(shown->key_hash, queued->key_hash, P2P_KEY_HASH_SIZE);
		memcpy(shown->signing_key, listing->key, P2P_KEY_SIZE);
		memcpy(shown->sealed_token, listing->sealed_token, P2P_SEALED_TOKEN_SIZE);
	}
	else
	{
		memcpy(bytes, queued->key_hash, P2P_KEY_HASH_SIZE);
		memcpy(bytes + P2P_KEY_HASH_SIZE, listing->token, P2P_TOKEN_SIZE);
		p2p_hex(bytes, sizeof(bytes), text);
		snprintf(shown->invite, sizeof(shown->invite), "halo://join/%s", text);
	}
	p2p_identifier_from_hash(queued->key_hash, shown->identifier);
	memcpy(shown->name, listing->name, sizeof(shown->name));
	memcpy(shown->map, listing->map, sizeof(shown->map));
	memcpy(shown->gametype, listing->gametype, sizeof(shown->gametype));
	shown->player_count = (unsigned char)listing->player_count;
	shown->maximum_player_count = (unsigned char)listing->maximum_player_count;
	shown->engine_type = (unsigned char)listing->engine_type;
	shown->kind = (unsigned char)listing->kind;
	shown->open = (listing->flags & _listing_open) != 0;
	shown->in_progress = (listing->flags & _listing_in_progress) != 0;
	shown->has_teams = (listing->flags & _listing_has_teams) != 0;
	/* (a dedicated server's on a master's word alone: the listing's own
	flag is its host's, which any host can set, so a game the brokers alone
	list is the players' however it calls itself) */
	shown->dedicated = game->master && game->master_dedicated;
	shown->network_version = (unsigned short)listing->version;
}

/* the game as the browser shows it */
static void game_listed(const struct game *game, struct p2p_listing *listing)
{
	*listing = game->listing;
	listing->failed = (unsigned char)identifier_failed(listing->identifier);
	listing->ping = (short)game->ping;
	listing->probing = (unsigned char)game->probing;
}

/* the hosts' pings: those not known lately probed, while there is room (a
few refused, and the rest wait for a later pass) */
static void update_probes(void)
{
	int refused = 0;
	int index;

	for (index = 0; index < MAXIMUM_GAMES && refused < P2P_MAXIMUM_PROBES; index++)
	{
		struct game *game = &lobby.games[index];

		if (!game->used || !game->has_token || game->probing || game->master ||
			(game->probed_time && !elapsed(game->probed_time, PROBE_REFRESH)))
		{
			continue;
		}
		/* (set first: a probe that needs no tunnel ends at once,
		p2p_lobby_probed) */
		game->probing = 1;
		game->probed_time = (p2p_now() - 1) | 1;
		if (!p2p_probe_start(game->key_hash, game->token))
		{
			game->probing = 0;
			game->probed_time = 0;
			refused++;
		}
	}
}

/* a host heard listing a game of a newer network version (lobby.newer) */
static void newer_heard(const unsigned char *key_hash)
{
	int index, free_index = -1;

	for (index = 0; index < MAXIMUM_GAMES; index++)
	{
		if (lobby.newer[index].used && !memcmp(lobby.newer[index].key_hash, key_hash, P2P_KEY_HASH_SIZE))
		{
			lobby.newer[index].heard_time = p2p_now();
			return;
		}
		if (!lobby.newer[index].used && free_index < 0)
			free_index = index;
	}
	if (free_index < 0)
		return;
	memcpy(lobby.newer[free_index].key_hash, key_hash, P2P_KEY_HASH_SIZE);
	lobby.newer[free_index].heard_time = p2p_now();
	lobby.newer[free_index].used = 1;
}

/* whether a listing heard is worth a queue place: one this version can show,
of about now. Another version's is not (a newer one's host counted:
newer_heard), nor one outside its time window. Its header is not verified
yet, so a forged one can only keep its own listing out; the queue's checks,
the signature's included, still decide what is shown. */
static int listing_wanted(const unsigned char *payload, int size, int retained, const unsigned char *key_hash)
{
	struct listing listing;

	if (!listing_read(payload, size, &listing))
		return 0;
	if (listing.version != HALO_PORT_NETWORK_VERSION)
	{
		if (listing.version > HALO_PORT_NETWORK_VERSION && !(listing.flags & _listing_closed))
			newer_heard(key_hash);
		return 0;
	}
	return listing_current(&listing, retained);
}

static void update_browsing(void)
{
	unsigned long start = p2p_now();
	int index;

	if (lobby.query_wanted)
	{
		lobby.query_wanted = 0;
		p2p_signal_lobby_query();
	}
	/* the queue, within the budget (one at least) */
	while (lobby.queue_count && (p2p_now() == start || !elapsed(start, VERIFY_BUDGET)))
	{
		struct queued queued = lobby.queue[0];
		struct listing listing;
		unsigned char key_hash[P2P_KEY_HASH_SIZE];
		int good;

		memmove(lobby.queue, lobby.queue + 1, sizeof(*lobby.queue) * (size_t)(--lobby.queue_count));
		/* (listing_wanted let it in; its key and its time are checked
		again here, where its signature is: the time it was queued at may
		have run out) */
		if (!listing_read(queued.payload, queued.size, &listing) || listing.version != HALO_PORT_NETWORK_VERSION ||
			!signing_key_hash(listing.key, key_hash) || memcmp(key_hash, queued.key_hash, P2P_KEY_HASH_SIZE) ||
			!listing_current(&listing, queued.retained))
		{
			continue;
		}
		/* (the work, without the lock: the game's threads need not wait) */
		pthread_mutex_unlock(&p2p_lock);
		good = listing_signed(queued.payload, &listing);
		pthread_mutex_lock(&p2p_lock);
		if (good && lobby.browsing)
			listing_take(&queued, &listing);
	}
	if (lobby.queue_count && !lobby.browsing)
		lobby.queue_count = 0;
	for (index = 0; index < MAXIMUM_GAMES; index++)
	{
		if (lobby.games[index].used && elapsed(lobby.games[index].heard_time, GAME_EXPIRY))
			lobby.games[index].used = 0;
		if (lobby.games[index].used && lobby.games[index].master && elapsed(lobby.games[index].master_time, GAME_EXPIRY))
		{
			lobby.games[index].master = 0;
			lobby.games[index].listing.master = 0;
			lobby.games[index].listing.master_key_id_count = 0;
		}
		if (lobby.newer[index].used && elapsed(lobby.newer[index].heard_time, GAME_EXPIRY))
			lobby.newer[index].used = 0;
	}
	if (lobby.browsing)
		update_probes();
}

void p2p_lobby_probed(const unsigned char *identifier, int ping)
{
	int index;

	for (index = 0; index < MAXIMUM_GAMES; index++)
	{
		struct game *game = &lobby.games[index];

		if (game->used && !memcmp(game->listing.identifier, identifier, P2P_IDENTIFIER_SIZE))
		{
			game->probing = 0;
			game->probed_time = (p2p_now() - 1) | 1;
			/* (one not reached this time keeps the last it had) */
			if (ping >= 0)
				game->ping = ping;
		}
	}
}

/* ---------- the master servers' (master_list.c, server_master.c) */

int p2p_lobby_host_listing(unsigned char *bytes, int size)
{
	int made = 0;

	if (size < MAXIMUM_LISTING_SIZE)
		return 0;
	pthread_mutex_lock(&p2p_lock);
	/* (public, as the brokers' listing is: network.public_lobby too) */
	if (lobby.hosting_token_valid && lobby.public && config_boolean("network.public_lobby"))
	{
		made = listing_make_token(bytes, lobby.flags, lobby.hosting_token);
		/* (its token listed: going private makes a new invite, p2p.c) */
		p2p_token_listed(lobby.hosting_token);
	}
	pthread_mutex_unlock(&p2p_lock);
	return made;
}

int p2p_lobby_take_master_listing(const unsigned char *payload, int size, const char *publisher, const char *region,
	int network_version_shown, int dedicated, const unsigned char *master_key_id)
{
	struct listing listing;
	struct queued queued;
	struct game *game;
	int taken = 0;

	(void)network_version_shown;
	if (size < MINIMUM_LISTING_SIZE || size > MAXIMUM_LISTING_SIZE || !listing_read(payload, size, &listing) ||
		(listing.flags & _listing_closed) || !signing_key_hash(listing.key, queued.key_hash) ||
		!listing_signed(payload, &listing))
	{
		return 0;
	}
	memcpy(queued.payload, payload, (size_t)size);
	queued.size = size;
	queued.retained = 0;
	pthread_mutex_lock(&p2p_lock);
	if (lobby.browsing)
	{
		game = find_game(queued.key_hash);
		/* (the same listing again: heard; a newer one: taken; another
		version's is shown, greyed, by the menus: network_version) */
		if (game && game->payload_size == size && !memcmp(game->payload, payload, (size_t)size))
		{
			game->heard_time = p2p_now();
		}
		else
		{
			if (game)
				game->master = 1;
			listing_take(&queued, &listing);
			game = find_game(queued.key_hash);
		}
		if (game)
		{
			game->master = 1;
			game->master_dedicated = dedicated != 0;
			game->master_time = p2p_now();
			game->listing.dedicated = dedicated != 0;
			game->listing.master = 1;
			snprintf(game->listing.publisher, sizeof(game->listing.publisher), "%s", publisher ? publisher : "");
			snprintf(game->listing.region, sizeof(game->listing.region), "%s", region ? region : "");
			if (game->listing.master_count < 255)
				game->listing.master_count++;
			/* (the master among those that list it: a Discord link's pass
			is shown from one of them, player_link.c) */
			if (master_key_id)
			{
				int known;

				for (known = 0; known < game->listing.master_key_id_count &&
					memcmp(game->listing.master_key_ids[known], master_key_id, P2P_LISTING_MASTER_KEY_ID_SIZE); known++)
				{
				}
				if (known == game->listing.master_key_id_count && known < P2P_LISTING_MAXIMUM_MASTERS)
				{
					memcpy(game->listing.master_key_ids[known], master_key_id, P2P_LISTING_MASTER_KEY_ID_SIZE);
					game->listing.master_key_id_count++;
				}
			}
			taken = 1;
		}
	}
	pthread_mutex_unlock(&p2p_lock);
	return taken;
}

void p2p_lobby_masters_heard(void)
{
	int index;

	/* (each game's count of the masters listing it, counted again) */
	pthread_mutex_lock(&p2p_lock);
	for (index = 0; index < MAXIMUM_GAMES; index++)
		lobby.games[index].listing.master_count = 0;
	pthread_mutex_unlock(&p2p_lock);
}

/* ---------- p2p.c's side */

void p2p_lobby_update(const unsigned char *token, int player_count, int maximum_player_count)
{
	update_hosting(token, player_count, maximum_player_count);
	update_browsing();
}

int p2p_lobby_listed(void)
{
	return lobby.listed;
}

void p2p_lobby_quit(void)
{
	if (!lobby.listed)
		return;
	stop_listing();
	/* (the game is ending: written now, not on the coming passes) */
	p2p_signal_lobby_quit();
}

/* ---------- the game's side */

void p2p_set_hosting_public(int public)
{
	int changed;

	pthread_mutex_lock(&p2p_lock);
	/* (a new invite going private: p2p.c) */
	if (lobby.public && !public)
		p2p_new_invite_if_listed();
	changed = lobby.public != (public ? 1 : 0);
	lobby.public = public ? 1 : 0;
	pthread_mutex_unlock(&p2p_lock);
	/* (the master servers told at once: listed, or withdrawn) */
	if (changed)
		server_master_changed();
}

int p2p_hosting_listed(void)
{
	int listed;

	pthread_mutex_lock(&p2p_lock);
	listed = lobby.public && config_boolean("network.public_lobby");
	pthread_mutex_unlock(&p2p_lock);
	return listed;
}

void p2p_set_hosting_password(const char *password)
{
	unsigned char key[P2P_PASSWORD_KEY_SIZE];
	int has_password = password && *password;

	/* (the key takes a while: not under the lock) */
	if (has_password)
		p2p_password_key(password, p2p_signing_key(), key);
	pthread_mutex_lock(&p2p_lock);
	if (has_password != lobby.has_password ||
		(has_password && !p2p_equal(key, lobby.password_key, P2P_PASSWORD_KEY_SIZE)))
	{
		/* (a new invite, if one was listed: who saw it, with no password or
		another, cannot join with it) */
		p2p_new_invite_if_listed();
		lobby.has_password = has_password;
		if (has_password)
			memcpy(lobby.password_key, key, sizeof(key));
		else
			memset(lobby.password_key, 0, sizeof(lobby.password_key));
		if (lobby.listed)
			republish_soon(0);
	}
	pthread_mutex_unlock(&p2p_lock);
	memset(key, 0, sizeof(key));
}

void p2p_set_game_listing(const char *name, const char *map, const char *gametype, int engine_type, int kind,
	int open, int in_progress, int has_teams)
{
	char new_name[P2P_LISTING_NAME_SIZE + 1];
	char new_map[P2P_LISTING_MAP_SIZE + 1];
	char new_gametype[P2P_LISTING_GAMETYPE_SIZE + 1];
	int flags = (open ? _listing_open : 0) | (in_progress ? _listing_in_progress : 0) |
		(has_teams ? _listing_has_teams : 0);

	sanitize(new_name, P2P_LISTING_NAME_SIZE, name ? name : lobby.name, P2P_LISTING_NAME_SIZE);
	sanitize(new_map, P2P_LISTING_MAP_SIZE, map ? map : lobby.map, P2P_LISTING_MAP_SIZE);
	sanitize(new_gametype, P2P_LISTING_GAMETYPE_SIZE, gametype ? gametype : lobby.gametype,
		P2P_LISTING_GAMETYPE_SIZE);
	/* (only the game's server writes these: unchanged, it need not wait for
	the lock) */
	if (!strcmp(new_name, lobby.name) && !strcmp(new_map, lobby.map) && !strcmp(new_gametype, lobby.gametype) &&
		engine_type == lobby.engine_type && (kind & 0xFF) == lobby.kind && flags == lobby.flags)
	{
		return;
	}
	pthread_mutex_lock(&p2p_lock);
	memcpy(lobby.name, new_name, sizeof(lobby.name));
	memcpy(lobby.map, new_map, sizeof(lobby.map));
	memcpy(lobby.gametype, new_gametype, sizeof(lobby.gametype));
	lobby.engine_type = engine_type;
	lobby.kind = kind & 0xFF;
	lobby.flags = flags;
	if (lobby.listed)
		republish_soon(0);
	pthread_mutex_unlock(&p2p_lock);
	server_master_changed();
}

void p2p_lobby_browse(int on)
{
	pthread_mutex_lock(&p2p_lock);
	on = on && config_boolean("network.public_lobby") ? 1 : 0;
	if (on != lobby.browsing)
	{
		lobby.browsing = on;
		if (!on)
		{
			memset(lobby.games, 0, sizeof(lobby.games));
			memset(lobby.newer, 0, sizeof(lobby.newer));
			lobby.queue_count = 0;
		}
		lobby.query_wanted = on;
		p2p_signal_lobby_topics(lobby.listed, on);
	}
	pthread_mutex_unlock(&p2p_lock);
	/* (the master servers' lists too, while the browser is open) */
	master_list_browse(on);
}

void p2p_lobby_refresh(void)
{
	int index;

	pthread_mutex_lock(&p2p_lock);
	if (lobby.browsing)
		lobby.query_wanted = 1;
	/* (the pings timed again) */
	for (index = 0; index < MAXIMUM_GAMES; index++)
	{
		if (!lobby.games[index].probing)
			lobby.games[index].probed_time = 0;
	}
	pthread_mutex_unlock(&p2p_lock);
	master_list_refresh();
}

int p2p_lobby_browsing(void)
{
	return lobby.browsing;
}

/* the order shown: the most players first; then those joining has not
failed, the open ones, and by name */
static int listing_order(const void *first, const void *second)
{
	const struct p2p_listing *a = first, *b = second;

	if (a->dedicated != b->dedicated)
		return b->dedicated - a->dedicated;
	if (a->player_count != b->player_count)
		return b->player_count - a->player_count;
	if (a->failed != b->failed)
		return a->failed - b->failed;
	if (a->open != b->open)
		return b->open - a->open;
	return strcmp(a->name, b->name);
}

int p2p_lobby_games(struct p2p_listing *games, int maximum_count)
{
	int count = 0;
	int index;

	pthread_mutex_lock(&p2p_lock);
	for (index = 0; index < MAXIMUM_GAMES && count < maximum_count; index++)
	{
		if (!lobby.games[index].used)
			continue;
		game_listed(&lobby.games[index], &games[count]);
		count++;
	}
	pthread_mutex_unlock(&p2p_lock);
	qsort(games, (size_t)count, sizeof(*games), listing_order);
	return count;
}

int p2p_listing_unlock(struct p2p_listing *listing, const char *password)
{
	unsigned char key[P2P_PASSWORD_KEY_SIZE];
	unsigned char bytes[P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE];
	char text[2 * (P2P_KEY_HASH_SIZE + P2P_TOKEN_SIZE) + 1];
	int opened;

	if (!listing->locked)
		return 1;
	p2p_password_key(password ? password : "", listing->signing_key, key);
	memcpy(bytes, listing->key_hash, P2P_KEY_HASH_SIZE);
	opened = p2p_unseal_token(key, listing->signing_key, listing->sealed_token, bytes + P2P_KEY_HASH_SIZE);
	if (opened)
	{
		p2p_hex(bytes, sizeof(bytes), text);
		snprintf(listing->invite, sizeof(listing->invite), "halo://join/%s", text);
	}
	memset(key, 0, sizeof(key));
	memset(bytes, 0, sizeof(bytes));
	return opened;
}

int p2p_lobby_newer_games(void)
{
	int count = 0;
	int index;

	pthread_mutex_lock(&p2p_lock);
	for (index = 0; index < MAXIMUM_GAMES; index++)
		count += lobby.newer[index].used;
	pthread_mutex_unlock(&p2p_lock);
	return count;
}

void p2p_lobby_mark_failed(const unsigned char *identifier)
{
	pthread_mutex_lock(&p2p_lock);
	if (!identifier_failed(identifier) && lobby.failed_count < MAXIMUM_GAMES)
		memcpy(lobby.failed[lobby.failed_count++], identifier, P2P_IDENTIFIER_SIZE);
	pthread_mutex_unlock(&p2p_lock);
}

int p2p_lobby_brokers_reached(void)
{
	int reached;

	pthread_mutex_lock(&p2p_lock);
	reached = p2p_signal_connected();
	pthread_mutex_unlock(&p2p_lock);
	return reached;
}

/* ---------- what the server browser shows (pure: the harness tests these) */

static const char *const sort_names[NUMBER_OF_P2P_LOBBY_SORTS] =
{
	"servers", "players", "name", "map", "type", "locked",
};

int p2p_lobby_filter(struct p2p_listing *games, int count, const struct p2p_lobby_choices *choices,
	int (*map_state)(const char *map))
{
	int read, written = 0;

	for (read = 0; read < count; read++)
	{
		const struct p2p_listing *game = &games[read];
		int state = map_state ? map_state(game->map) : P2P_LOBBY_MAP_INSTALLED;

		if ((choices->source == P2P_LOBBY_SOURCE_DEDICATED && !game->dedicated) ||
			(choices->source == P2P_LOBBY_SOURCE_PLAYERS && game->dedicated))
		{
			continue;
		}
		if (!choices->show_empty && game->player_count == 0)
			continue;
		if (!choices->show_full && game->maximum_player_count && game->player_count >= game->maximum_player_count)
			continue;
		if (choices->engine_type && game->engine_type != choices->engine_type)
			continue;
		if ((choices->maps == P2P_LOBBY_MAPS_NO && state != P2P_LOBBY_MAP_INSTALLED) ||
			(choices->maps == P2P_LOBBY_MAPS_DOWNLOADABLE && state == P2P_LOBBY_MAP_MISSING))
		{
			continue;
		}
		if (written != read)
			games[written] = games[read];
		written++;
	}
	return written;
}

/* names A to Z, whatever their letters' case */
static int text_order(const char *first, const char *second)
{
	for (;; first++, second++)
	{
		int a = *first >= 'A' && *first <= 'Z' ? *first + 'a' - 'A' : (unsigned char)*first;
		int b = *second >= 'A' && *second <= 'Z' ? *second + 'a' - 'A' : (unsigned char)*second;

		if (a != b || !a)
			return a - b;
	}
}

/* the order's: below 0 if first comes first */
static int sort_order(const struct p2p_listing *first, const struct p2p_listing *second, int order)
{
	int difference = 0;

	switch (order)
	{
	case P2P_LOBBY_SORT_PLAYERS:
		difference = second->player_count - first->player_count;
		break;
	case P2P_LOBBY_SORT_MAP:
		difference = text_order(first->map, second->map);
		break;
	case P2P_LOBBY_SORT_TYPE:
		difference = first->engine_type != second->engine_type ? first->engine_type - second->engine_type :
			first->kind != second->kind ? first->kind - second->kind : text_order(first->gametype, second->gametype);
		break;
	case P2P_LOBBY_SORT_LOCKED:
		difference = second->locked - first->locked;
		break;
	case P2P_LOBBY_SORT_SERVERS:
		/* (as p2p_lobby_games has them: then those joining has not failed,
		and the open ones) */
		difference = second->dedicated - first->dedicated;
		if (!difference)
			difference = second->player_count - first->player_count;
		if (!difference)
			difference = first->failed - second->failed;
		if (!difference)
			difference = second->open - first->open;
		break;
	}
	return difference ? difference : text_order(first->name, second->name);
}

void p2p_lobby_sort(struct p2p_listing *games, int count, int order, int reverse)
{
	int index;

	/* (an insertion sort: stable, and the games are a few hundred at most) */
	for (index = 1; index < count; index++)
	{
		struct p2p_listing game = games[index];
		int other_version = game.network_version != HALO_PORT_NETWORK_VERSION;
		int place = index;

		while (place > 0)
		{
			const struct p2p_listing *before = &games[place - 1];
			int before_other = before->network_version != HALO_PORT_NETWORK_VERSION;
			int difference = sort_order(before, &game, order);

			if (before_other != other_version ? !before_other : (reverse ? -difference : difference) <= 0)
				break;
			games[place] = games[place - 1];
			place--;
		}
		games[place] = game;
	}
}

int p2p_lobby_sort_parse(const char *text, int *reverse)
{
	int order;

	*reverse = text && *text == '-';
	for (order = 0; text && order < NUMBER_OF_P2P_LOBBY_SORTS; order++)
	{
		if (!strcmp(text + *reverse, sort_names[order]))
			return order;
	}
	*reverse = 0;
	return P2P_LOBBY_SORT_SERVERS;
}

void p2p_lobby_sort_text(int order, int reverse, char *text, int size)
{
	if (order < 0 || order >= NUMBER_OF_P2P_LOBBY_SORTS)
		order = P2P_LOBBY_SORT_SERVERS;
	snprintf(text, (size_t)size, "%s%s", reverse ? "-" : "", sort_names[order]);
}

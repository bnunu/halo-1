/*
P2P_SIGNAL.C

Internet play's signalling (p2p.c): how a joiner and the host of an invite
tell each other where they can be reached, through public MQTT brokers
(those in brokers.txt, network.brokers_file; MQTT 5 over TCP, or 3.1.1 with
a broker that refuses 5). Every broker is used at once, so any one of them
working is enough (an answer goes back through each broker a request came
through).
They also carry the server browser's listings of public games (p2p_lobby.c):
retained in each host's slot, published at least once (QoS 1, sent again
until the broker acknowledges them), expiring at the broker after
LISTING_EXPIRY seconds (MQTT 5), and cleared by the broker if the host's
connection dies (the will every connection sets: harmless when not hosting).

A broker may drop a connection's publishes past about 10 a second without a
word (EMQX), and a host's answers to a flood of JOINs would reach that: each
broker's publishes spend from a bucket (PUBLISH_BURST at once, one more each
PUBLISH_INTERVAL). Answers to proven joiners come first, then the listing
(kept to publish later), then a joiner's requests and the browser's queries;
answers to requests not proven yet only while some are left over (the
joiner asks again).

Everything that passes through them is sealed with a key derived from the
invite's token, and goes to topics that are hashes of it, so the brokers
(and anyone watching them) learn nothing and can join nothing:

- the host listens on hceu/3/<HMAC(token, "host" | host)>, where a joiner
  sends JOIN: its public key (its identifier is the key's hash), a nonce,
  and its addresses;
- the joiner listens on hceu/3/<HMAC(token, "joiner" | joiner)>, where the
  host answers ACCEPT: its public key, the joiner's nonce, one of its own,
  its addresses, and a tag that only the two of them can make (from their
  keys);
- the joiner then repeats its JOIN with the host's nonce and a tag of its
  own, made the same way, which shows that it holds the key it gave. Only
  then does the host make a session.

Their tunnel's keys come from their X25519 shared secret and the two
nonces, and never travel: another holder of the invite reads the messages
but cannot work them out, and cannot answer as the host (whose key must
have the hash in the invite: 16 bytes, not only the identifier's 6, which
a key could be made to have). It can send a JOIN in another
machine's name, but cannot prove it: the host answers it (to that machine)
and makes no session of it. A session no one could complete would keep that
machine out while it lived (a session with a machine is not replaced while
it lives, p2p.c), and a JOIN every so often would keep it out for good.

The host's nonce is a hash of the request with a key of the host's and the
time (it changes every HOST_NONCE_PERIOD), so the host need not remember
what it answered: anyone with the invite can ask in a machine's name as
often as it likes, pushing out whatever the host remembered. What it does
keep of requests not proven yet (the work of their keys) only saves work.
It answers them sparingly, as anyone with the invite can send them from as
many keys as it likes: a request once each ANSWER_INTERVAL through each
broker, and 20 a second in all (MAXIMUM_UNPROVEN_ANSWERS), each through the
broker the request came through only (the joiner asks through them all,
again every JOIN_INTERVAL, so a broker that loses the answers keeps no
other's out).

A joiner repeats its JOIN until the tunnel reaches the host. The host makes
one session of a request (a public key and nonce) at most: anyone watching
the brokers could send the proven JOIN again after the session ended, and
have the host reach for the joiner's old addresses in its name, keeping it
out (with the same keys again). It remembers the request at least while its
host nonce lasts. So a joiner asks with a new nonce when its session with
the host ends before the tunnel reached it, or when the host has not
answered it in a while.
*/

#include "platform.h"
#include "posix.h"
#include "port_config.h"
#include "p2p_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	MAXIMUM_BROKERS = 4,
	/* the joiners a host made sessions for, so a repeated request gets the
	same session: as many as it takes */
	MAXIMUM_JOINERS = P2P_MAXIMUM_PEERS + 1,
	/* the requests to join a machine makes at once: the invite's host, and
	other players of its game (p2p_signal_join_player) */
	MAXIMUM_JOINS = 1 + P2P_MAXIMUM_PLAYER_JOINS,
	/* the nonces of an asker's key it keeps the answer times of */
	ASKER_NONCES = 4,
	/* the requests not proven yet whose keys' work a host keeps, one a key;
	only to save work, but as many as a flood of new keys takes seconds to
	push out (one each UNPROVEN_ANSWER_INTERVAL at most, after the first
	few: about 11 seconds), so a joiner's proof, which follows its answer at
	once, finds its own */
	MAXIMUM_ASKERS = 256,
	/* the requests a session was made from, which make none again (a
	host takes at most a few new players a minute: this is hours of them;
	one is not forgotten while its host nonce lasts: USED_REQUEST_TIME) */
	MAXIMUM_USED_REQUESTS = 1024,
	/* (a slot's: hceu/3/lobby/s/ and a key hash in hex) */
	TOPIC_SIZE = 64,
	NONCE_SIZE = 8,
	/* an ACCEPT's or a proven JOIN's tag: the first half of an HMAC-SHA256 */
	TAG_SIZE = 16,
	/* a proven JOIN's end: the host's nonce, and the tag */
	PROOF_SIZE = NONCE_SIZE + TAG_SIZE,
	BUFFER_SIZE = 4096,
	MAXIMUM_MESSAGE_SIZE = 512,
	/* a listing's most (p2p_lobby.c's MAXIMUM_LISTING_SIZE, 268 with a
	password's sealed token, and some) */
	MAXIMUM_LISTING_SIZE = 320,
	/* the publishes awaiting acknowledgement on a broker */
	MAXIMUM_IN_FLIGHT = 4,
	/* a broker's publishes: at most PUBLISH_BURST at once, one more each
	PUBLISH_INTERVAL milliseconds (EMQX drops a connection's publishes past
	about 10 a second, without a word) */
	PUBLISH_BURST = 8,
	PUBLISH_INTERVAL = 125,
	/* what is kept back of the bucket for answers to proven joiners and the
	listing: a request's answer, and a query, need this many left */
	UNPROVEN_RESERVE = 3,
	/* seconds a listing lasts at the broker (MQTT 5's Message Expiry
	Interval): a dead host's goes even from a broker that kept it through a
	restart */
	LISTING_EXPIRY = 90,

	/* milliseconds */
	CONNECT_TIMEOUT = 10000,
	/* a broker that failed is tried again after this, twice as long each
	failure after, to MAXIMUM_RETRY_INTERVAL */
	RETRY_INTERVAL = 15000,
	MAXIMUM_RETRY_INTERVAL = 120000,
	/* a publish at least once not acknowledged is sent again */
	RESEND_INTERVAL = 5000,
	KEEP_ALIVE_SECONDS = 60,
	PING_INTERVAL = 30000,
	SILENCE_TIMEOUT = 90000,
	JOIN_INTERVAL = 2000,
	ANSWER_INTERVAL = 1000,
	/* a joiner the host has not answered in this long asks anew, with a
	new nonce (a fresh start: the host makes nothing of a request it did
	not answer, as it makes a session only of a proven one) */
	UNANSWERED_TIME = 20000,
	/* a host nonce is taken in the period it is made in and the next */
	HOST_NONCE_PERIOD = 30000,
	USED_REQUEST_TIME = 2 * HOST_NONCE_PERIOD,
	/* a host's work of the keys of requests from keys it has not met (a
	millisecond or more each, on the thread the tunnels run on): this many
	at once, and one more each this often */
	MAXIMUM_KEY_WORK = 32,
	KEY_WORK_INTERVAL = 50,
	/* a host's answers to requests not proven yet, which anyone with the
	invite can send from any number of keys: this many at once, and one more
	each this often */
	MAXIMUM_UNPROVEN_ANSWERS = 32,
	UNPROVEN_ANSWER_INTERVAL = 50,
	/* the reads of a broker's messages in one pass of the thread, whose
	tunnels a flood of them would otherwise starve */
	MAXIMUM_BROKER_READS = 8,
};

enum
{
	_broker_idle,
	_broker_connecting,
	_broker_awaiting_acknowledgement,
	_broker_ready,
};

enum
{
	_topic_host,
	_topic_join,
	/* the server browser's (p2p_lobby.c): the own slot, queries, all slots,
	and each probe's answers (p2p_signal_probe) */
	_topic_own_slot,
	_topic_query,
	_topic_slots,
	_topic_probe,
	/* the topics a broker is subscribed to */
	NUMBER_OF_TOPICS = _topic_probe + P2P_MAXIMUM_PROBES,
};

/* MQTT 5's properties */
enum
{
	_property_message_expiry = 0x02,
	_property_server_keep_alive = 0x13,
	_property_receive_maximum = 0x21,
	_property_retain_available = 0x25,
	_property_wildcard_available = 0x28,
};

enum
{
	_message_join = 'J',
	_message_accept = 'A',
	/* 3: a JOIN proves its key before the host makes a session; 4: both
	messages say the machine's WebRTC (p2p_webrtc.c), so browsers join */
	MESSAGE_VERSION = 4,
	/* the IPv4 addresses a message holds: an older build takes no more */
	MAXIMUM_MESSAGE_ADDRESSES = 4,
};

/* an IPv6 address a native build offers, in place of one of the mDNS names
a browser offers ("ip6-", its 16 bytes in hexadecimal, ".local"): a native
build's names were never used, so an older build reads it and leaves it,
and the messages and network version stay as they were */
#define IPV6_NAME_PREFIX "ip6-"
#define IPV6_NAME_SUFFIX ".local"

struct broker
{
	char host[128];
	unsigned short port;
	unsigned long address;
	int looked_up;
	int socket;
	int state;
	unsigned long state_time;
	unsigned long sent_time;
	unsigned long heard_time;
	int failures;
	unsigned short packet_identifier;
	/* 5 (MQTT 5), or 4 (3.1.1: the broker refused 5) */
	int protocol;
	/* as its CONNACK said (MQTT 5; 3.1.1 brokers have both): without
	either, it carries only signalling, not listings */
	int retain_available;
	int wildcard_available;
	/* seconds; its own if it said */
	int keep_alive;
	int receive_maximum;
	/* the bucket its publishes spend from */
	int publish_tokens;
	unsigned long publish_time;
	/* the topics it has been asked for (_topic_*) */
	char topics[NUMBER_OF_TOPICS][TOPIC_SIZE];
	/* the listing's version it was given (signalling.lobby_version); and
	whether it has yet to have its slot cleared (after a tombstone) */
	int lobby_version;
	int lobby_clear_pending;
	/* a query to send once subscribed to the slots */
	int query_pending;
	/* publishes at least once, awaiting PUBACK */
	struct
	{
		int used;
		unsigned short identifier;
		unsigned long sent_time;
		/* a listing, which a newer one replaces (not a tombstone or a
		clearing) */
		int listing;
		/* refused (MQTT 5's PUBACK said why): sent again as a new publish
		(the refusal ended the old one's identifier) */
		int refused;
		int size;
		unsigned char payload[MAXIMUM_LISTING_SIZE];
	} in_flight[MAXIMUM_IN_FLIGHT];
	unsigned char input[BUFFER_SIZE];
	int input_size;
	unsigned char output[BUFFER_SIZE];
	int output_size;
};

/* a request: of a session made (joiners), or not proven yet (askers: one a
key, with its latest nonce, and no host nonce or secret; and its latest
nonces answered, and when) */
struct joiner
{
	unsigned char identifier[P2P_IDENTIFIER_SIZE];
	unsigned char public_key[P2P_KEY_SIZE];
	/* its and the host's */
	unsigned char nonce[NONCE_SIZE];
	unsigned char host_nonce[NONCE_SIZE];
	/* pair_base's, and their session's secret */
	unsigned char base[P2P_SHA256_SIZE];
	unsigned char secret[P2P_SHA256_SIZE];
	unsigned long answered_time;
	/* (a joiner's) when it was answered through each broker */
	unsigned long answered_broker_times[MAXIMUM_BROKERS];
	/* (an asker's) its latest nonces answered, through which broker, when */
	unsigned char answered_nonces[ASKER_NONCES][NONCE_SIZE];
	signed char answered_nonce_brokers[ASKER_NONCES];
	unsigned long answered_nonce_times[ASKER_NONCES];
	int used;
};

/* a request to join: to the invite's host, or to another player of its
game (who answers as a host does) */
struct join
{
	int active;
	/* the identifier of the machine asked, and the hash of its key (the
	invite's, or as the game's host told) */
	unsigned char host[P2P_IDENTIFIER_SIZE];
	unsigned char host_hash[P2P_KEY_HASH_SIZE];
	unsigned char nonce[NONCE_SIZE];
	/* its public key, once it answered, and pair_base's */
	int has_base;
	unsigned char host_public[P2P_KEY_SIZE];
	unsigned char base[P2P_SHA256_SIZE];
	char host_topic[TOPIC_SIZE];
	unsigned long sent_time;
	/* when nonce was made, and whether the machine answered it (with
	host_nonce, which the proof carries) */
	unsigned long nonce_time;
	int answered;
	unsigned char host_nonce[NONCE_SIZE];
};

/* work allowed: some at once, and one more each interval */
struct budget
{
	int left;
	/* as of when */
	unsigned long time;
};

/* a public game's host asked only to time the tunnel to it
(p2p_signal_probe): a request of its listing's token, whose answers come
sealed with its key to its topic */
struct probe
{
	struct join join;
	unsigned char key[P2P_SHA256_SIZE];
	char topic[TOPIC_SIZE];
};

struct used_request
{
	/* the joiner's identifier and nonce */
	unsigned char request[P2P_IDENTIFIER_SIZE + NONCE_SIZE];
	unsigned long time;
};

static struct
{
	int started;
	struct broker brokers[MAXIMUM_BROKERS];
	int broker_count;
	char client_identifier[24];

	/* the server browser (p2p_lobby.c): this machine's slot (its will
	clears it), whether its game is listed and whether it browses; the
	listing to publish (none: size 0), its version, and whether it is a
	tombstone (the slot is cleared after it) */
	char own_slot[TOPIC_SIZE];
	int lobby_listed;
	int lobby_browsing;
	unsigned char lobby_listing[MAXIMUM_LISTING_SIZE];
	int lobby_listing_size;
	int lobby_version;
	int lobby_closing;

	/* hosting */
	int hosting;
	unsigned char host_token[P2P_TOKEN_SIZE];
	unsigned char host_key[P2P_SHA256_SIZE];
	char host_topic[TOPIC_SIZE];
	struct joiner joiners[MAXIMUM_JOINERS];
	int next_joiner;
	struct joiner askers[MAXIMUM_ASKERS];
	int next_asker;
	/* the key of the host's nonces (host_nonce_for), for the run */
	int has_nonce_key;
	unsigned char nonce_key[P2P_SHA256_SIZE];
	/* each request a session was made from (kept while hosting stops and
	starts: an invite lasts the run) */
	struct used_request used_requests[MAXIMUM_USED_REQUESTS];
	int used_request_count;
	int used_request_next;
	/* the key work it may do now (MAXIMUM_KEY_WORK), and the answers it
	may give requests not proven yet (MAXIMUM_UNPROVEN_ANSWERS) */
	struct budget key_work;
	struct budget unproven_answers;

	/* joining: the invite's host (joins[0]), and the other players of its
	game this machine asks for as their host does for it (p2p_signal_join_player);
	all of one token, whose answers come to one topic */
	struct join joins[MAXIMUM_JOINS];
	unsigned char join_token[P2P_TOKEN_SIZE];
	unsigned char join_key[P2P_SHA256_SIZE];
	char join_topic[TOPIC_SIZE];

	/* the server browser's probes (p2p_signal_probe) */
	struct probe probes[P2P_MAXIMUM_PROBES];
} signalling;

static int elapsed(unsigned long since, unsigned long time)
{
	/* (unsigned, as the clock wraps) */
	return (unsigned int)(p2p_now() - since) >= (unsigned int)time;
}

static unsigned short network_short(unsigned short value)
{
	return (unsigned short)(value << 8 | value >> 8);
}

/* whether some of a budget is left now (maximum at once, one more each
interval); spend takes one */
static int budget_left(struct budget *budget, int maximum, int interval, int spend)
{
	unsigned long now = p2p_now();
	unsigned long earned = (now - budget->time) / (unsigned long)interval;

	if (earned >= (unsigned long)maximum || budget->left + (int)earned >= maximum)
	{
		budget->left = maximum;
		budget->time = now;
	}
	else if (earned)
	{
		budget->left += (int)earned;
		budget->time += earned * (unsigned long)interval;
	}
	if (budget->left <= 0)
		return 0;
	if (spend)
		budget->left--;
	return 1;
}

static int joins_active(void);
static void send_join(struct join *join);
static void sync_all_topics(void);

/* ---------- what is derived from a token */

static void derive(const unsigned char *token, const char *label, const unsigned char *identifier,
	unsigned char *digest)
{
	unsigned char data[16 + P2P_IDENTIFIER_SIZE];
	int size = (int)strlen(label);

	memcpy(data, label, (size_t)size);
	if (identifier)
	{
		memcpy(data + size, identifier, P2P_IDENTIFIER_SIZE);
		size += P2P_IDENTIFIER_SIZE;
	}
	p2p_hmac_sha256(token, P2P_TOKEN_SIZE, data, size, digest);
}

static void make_topic(const unsigned char *token, const char *label, const unsigned char *identifier,
	char *topic)
{
	unsigned char digest[P2P_SHA256_SIZE];
	char text[2 * P2P_SHA256_SIZE + 1];

	derive(token, label, identifier, digest);
	p2p_hex(digest, 16, text);
	snprintf(topic, TOPIC_SIZE, "hceu/3/%s", text);
}

/* ---------- MQTT */

static void broker_close(struct broker *broker, int failed)
{
	if (broker->socket >= 0)
		posix_socket_close(broker->socket);
	broker->socket = -1;
	broker->state = _broker_idle;
	broker->state_time = p2p_now();
	broker->input_size = 0;
	broker->output_size = 0;
	memset(broker->topics, 0, sizeof(broker->topics));
	memset(broker->in_flight, 0, sizeof(broker->in_flight));
	/* (the will cleared the slot: the listing again once connected) */
	broker->lobby_version = 0;
	broker->lobby_clear_pending = 0;
	broker->query_pending = 0;
	if (failed)
		broker->failures++;
}

static void broker_flush(struct broker *broker)
{
	while (broker->output_size > 0)
	{
		int sent = posix_socket_send(broker->socket, broker->output, broker->output_size, 0);

		if (sent < 0)
		{
			int error = posix_socket_last_error();

			if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
				broker_close(broker, 1);
			return;
		}
		memmove(broker->output, broker->output + sent, (size_t)(broker->output_size - sent));
		broker->output_size -= sent;
	}
}

static int put_variable(unsigned char *bytes, int value)
{
	int size = 0;

	do
	{
		unsigned char byte = (unsigned char)(value & 127);

		value >>= 7;
		bytes[size++] = (unsigned char)(value ? byte | 128 : byte);
	} while (value);
	return size;
}

/* reads a variable byte integer; 0 if it does not fit */
static int get_variable(const unsigned char *bytes, int size, int *offset, int *value)
{
	int shift = 0;

	*value = 0;
	for (;;)
	{
		unsigned char byte;

		if (*offset >= size || shift > 21)
			return 0;
		byte = bytes[(*offset)++];
		*value |= (byte & 127) << shift;
		shift += 7;
		if (!(byte & 128))
			return 1;
	}
}

/* queues a packet: its fixed header's first byte, and its body */
static void broker_send(struct broker *broker, unsigned char type, const unsigned char *body, int size)
{
	unsigned char header[5];
	int header_size;

	if (broker->socket < 0)
		return;
	header[0] = type;
	header_size = 1 + put_variable(header + 1, size);
	if (broker->output_size + header_size + size > BUFFER_SIZE)
	{
		broker_close(broker, 1);
		return;
	}
	memcpy(broker->output + broker->output_size, header, (size_t)header_size);
	memcpy(broker->output + broker->output_size + header_size, body, (size_t)size);
	broker->output_size += header_size + size;
	broker->sent_time = p2p_now();
	broker_flush(broker);
}

static int put_string(unsigned char *body, const char *text)
{
	int size = (int)strlen(text);

	body[0] = (unsigned char)(size >> 8);
	body[1] = (unsigned char)size;
	memcpy(body + 2, text, (size_t)size);
	return size + 2;
}

static unsigned short next_packet_identifier(struct broker *broker)
{
	if (++broker->packet_identifier == 0)
		broker->packet_identifier = 1;
	return broker->packet_identifier;
}

/* whether the broker may publish now, spending from its bucket: reserve is
how many must be left after (UNPROVEN_RESERVE for what may wait) */
static int broker_may_publish(struct broker *broker, int reserve)
{
	unsigned long now = p2p_now();
	int gained = (int)((now - broker->publish_time) / PUBLISH_INTERVAL);

	if (gained > 0)
	{
		broker->publish_tokens = broker->publish_tokens + gained > PUBLISH_BURST ? PUBLISH_BURST :
			broker->publish_tokens + gained;
		broker->publish_time += (unsigned long)gained * PUBLISH_INTERVAL;
		if (broker->publish_tokens == PUBLISH_BURST)
			broker->publish_time = now;
	}
	if (broker->publish_tokens <= reserve)
		return 0;
	broker->publish_tokens--;
	return 1;
}

static void broker_topic(struct broker *broker, const char *topic, int subscribe, int no_local)
{
	unsigned char body[5 + TOPIC_SIZE + 1];
	unsigned short identifier = next_packet_identifier(broker);
	int size = 0;

	body[size++] = (unsigned char)(identifier >> 8);
	body[size++] = (unsigned char)identifier;
	/* (MQTT 5: no properties) */
	if (broker->protocol == 5)
		body[size++] = 0;
	size += put_string(body + size, topic);
	/* at most once; MQTT 5: No Local, not this connection's own publishes */
	if (subscribe)
		body[size++] = (unsigned char)(broker->protocol == 5 && no_local ? 0x04 : 0);
	broker_send(broker, subscribe ? 0x82 : 0xA2, body, size);
}

/* a publish at most once, not retained: signalling's, and queries */
static void broker_publish(struct broker *broker, const char *topic, const unsigned char *payload, int payload_size)
{
	unsigned char body[3 + TOPIC_SIZE + MAXIMUM_MESSAGE_SIZE + P2P_SEAL_OVERHEAD];
	int size = put_string(body, topic);

	if (broker->protocol == 5)
		body[size++] = 0;
	memcpy(body + size, payload, (size_t)payload_size);
	broker_send(broker, 0x30, body, size + payload_size);
}

/* a publish at least once to the own slot, retained: a listing (expiring
at the broker), or an empty one, which clears it; again (duplicate) if the
first was not acknowledged */
static void broker_publish_slot(struct broker *broker, unsigned short identifier, const unsigned char *payload,
	int payload_size, int duplicate)
{
	unsigned char body[2 + TOPIC_SIZE + 2 + 8 + MAXIMUM_LISTING_SIZE];
	int size = put_string(body, signalling.own_slot);

	body[size++] = (unsigned char)(identifier >> 8);
	body[size++] = (unsigned char)identifier;
	if (broker->protocol == 5)
	{
		if (payload_size)
		{
			body[size++] = 5;
			body[size++] = _property_message_expiry;
			body[size++] = 0;
			body[size++] = 0;
			body[size++] = 0;
			body[size++] = LISTING_EXPIRY;
		}
		else
		{
			body[size++] = 0;
		}
	}
	if (payload_size)
		memcpy(body + size, payload, (size_t)payload_size);
	/* PUBLISH, QoS 1, retained */
	broker_send(broker, (unsigned char)(0x30 | (duplicate ? 0x08 : 0) | 0x02 | 0x01), body, size + payload_size);
	broker->sent_time = p2p_now();
}

/* a listing (or a clearing: size 0) to the own slot, at least once: 0 if
there is no room for it yet */
static int broker_publish_listing(struct broker *broker, const unsigned char *payload, int size, int listing)
{
	int maximum = broker->receive_maximum < MAXIMUM_IN_FLIGHT ? broker->receive_maximum : MAXIMUM_IN_FLIGHT;
	int count = 0;
	int free_index = -1;
	int index;

	/* (a listing replaces an older one not acknowledged yet) */
	for (index = 0; index < MAXIMUM_IN_FLIGHT; index++)
	{
		if (broker->in_flight[index].used && broker->in_flight[index].listing && listing)
			broker->in_flight[index].used = 0;
		if (broker->in_flight[index].used)
			count++;
		else if (free_index < 0)
			free_index = index;
	}
	if (free_index < 0 || count >= maximum || !broker_may_publish(broker, 0))
		return 0;
	broker->in_flight[free_index].used = 1;
	broker->in_flight[free_index].identifier = next_packet_identifier(broker);
	broker->in_flight[free_index].sent_time = p2p_now();
	broker->in_flight[free_index].listing = listing;
	broker->in_flight[free_index].refused = 0;
	broker->in_flight[free_index].size = size;
	if (size)
		memcpy(broker->in_flight[free_index].payload, payload, (size_t)size);
	broker_publish_slot(broker, broker->in_flight[free_index].identifier, payload, size, 0);
	/* (not sent if the send closed the broker: published again once it is
	connected, as its will cleared the slot) */
	return broker->state == _broker_ready;
}

/* whether the broker carries the server browser: retained messages and
wildcards */
static int broker_carries_lobby(const struct broker *broker)
{
	return broker->retain_available && broker->wildcard_available;
}

/* the topics a ready broker should be subscribed to */
static void broker_sync_topics(struct broker *broker)
{
	const char *wanted[NUMBER_OF_TOPICS];
	int lobby = broker_carries_lobby(broker);
	int index;

	if (broker->state != _broker_ready)
		return;
	wanted[_topic_host] = signalling.hosting ? signalling.host_topic : "";
	wanted[_topic_join] = joins_active() ? signalling.join_topic : "";
	wanted[_topic_own_slot] = lobby && signalling.lobby_listed ? signalling.own_slot : "";
	wanted[_topic_query] = lobby && signalling.lobby_listed ? P2P_LOBBY_QUERY_TOPIC : "";
	wanted[_topic_slots] = lobby && signalling.lobby_browsing ? P2P_LOBBY_SLOT_PREFIX "+" : "";
	for (index = 0; index < P2P_MAXIMUM_PROBES; index++)
		wanted[_topic_probe + index] = signalling.probes[index].join.active ? signalling.probes[index].topic : "";
	for (index = 0; index < NUMBER_OF_TOPICS; index++)
	{
		char *had = broker->topics[index];

		if (!strcmp(wanted[index], had))
			continue;
		if (had[0])
			broker_topic(broker, had, 0, 0);
		if (wanted[index][0])
			broker_topic(broker, wanted[index], 1, index == _topic_own_slot);
		/* (a send that failed closed the broker, which forgot its topics:
		they are asked for again once it is ready again) */
		if (broker->state != _broker_ready)
			return;
		/* (once subscribed to the slots, the hosts are asked to publish:
		retained copies come at once, but may be old) */
		if (index == _topic_slots && wanted[index][0])
			broker->query_pending = 1;
		strcpy(had, wanted[index]);
	}
}

static void publish_everywhere(const char *topic, const unsigned char *payload, int size)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		struct broker *broker = &signalling.brokers[index];

		if (broker->state == _broker_ready && broker_may_publish(broker, 0))
			broker_publish(broker, topic, payload, size);
	}
}

/* the listing's publishes due on a broker: the listing, or a tombstone and
then the slot's clearing; acknowledgements not come sent again */
static void broker_update_lobby(struct broker *broker)
{
	int index;

	if (broker->state != _broker_ready || !broker_carries_lobby(broker))
		return;
	for (index = 0; index < MAXIMUM_IN_FLIGHT; index++)
	{
		if (broker->in_flight[index].used && elapsed(broker->in_flight[index].sent_time, RESEND_INTERVAL) &&
			broker_may_publish(broker, 0))
		{
			int duplicate = !broker->in_flight[index].refused;

			if (!duplicate)
				broker->in_flight[index].identifier = next_packet_identifier(broker);
			broker->in_flight[index].refused = 0;
			broker->in_flight[index].sent_time = p2p_now();
			broker_publish_slot(broker, broker->in_flight[index].identifier, broker->in_flight[index].payload,
				broker->in_flight[index].size, duplicate);
		}
	}
	if (broker->lobby_version != signalling.lobby_version && signalling.lobby_listing_size &&
		broker_publish_listing(broker, signalling.lobby_listing, signalling.lobby_listing_size,
		!signalling.lobby_closing))
	{
		broker->lobby_version = signalling.lobby_version;
		broker->lobby_clear_pending = signalling.lobby_closing;
	}
	if (broker->lobby_clear_pending && broker->lobby_version == signalling.lobby_version &&
		broker_publish_listing(broker, NULL, 0, 0))
	{
		broker->lobby_clear_pending = 0;
	}
	if (broker->query_pending && broker->topics[_topic_slots][0] && broker_may_publish(broker, UNPROVEN_RESERVE))
	{
		unsigned char nonce[NONCE_SIZE];

		posix_random_bytes(nonce, sizeof(nonce));
		broker_publish(broker, P2P_LOBBY_QUERY_TOPIC, nonce, sizeof(nonce));
		broker->query_pending = 0;
	}
}

static void broker_connected(struct broker *broker)
{
	unsigned char body[32 + sizeof(signalling.client_identifier) + TOPIC_SIZE];
	int size = 0;

	if (!broker->protocol)
		broker->protocol = 5;
	size += put_string(body, "MQTT");
	body[size++] = (unsigned char)broker->protocol;
	/* a clean session, and a will: an empty retained message to this
	machine's slot, which clears its listing if the connection dies */
	body[size++] = 0x02 | 0x04 | 0x20;
	body[size++] = 0;
	body[size++] = KEEP_ALIVE_SECONDS;
	if (broker->protocol == 5)
		body[size++] = 0;
	size += put_string(body + size, signalling.client_identifier);
	if (broker->protocol == 5)
		body[size++] = 0;
	size += put_string(body + size, signalling.own_slot);
	body[size++] = 0;
	body[size++] = 0;
	broker->state = _broker_awaiting_acknowledgement;
	broker->state_time = p2p_now();
	broker_send(broker, 0x10, body, size);
}

static void broker_connect(struct broker *broker)
{
	struct sockaddr_in address;

	if (!broker->looked_up || !broker->address || broker->failures >= 2)
	{
		/* may wait for DNS; only at the start, or after failing (twice: the
		broker may have moved) */
		broker->address = p2p_resolve(broker->host);
		broker->looked_up = 1;
		if (!broker->address)
		{
			if (!broker->failures)
				platform_log("Internet play: cannot look up the signalling broker %s", broker->host);
			broker_close(broker, 1);
			return;
		}
	}
	broker->socket = posix_socket(AF_INET, SOCK_STREAM, 0);
	if (broker->socket < 0)
	{
		broker_close(broker, 1);
		return;
	}
	posix_socket_set_nonblocking(broker->socket, 1);
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = broker->port;
	address.sin_addr.s_addr = broker->address;
	broker->state = _broker_connecting;
	broker->state_time = p2p_now();
	if (posix_socket_connect(broker->socket, &address, sizeof(address)) == 0)
	{
		broker_connected(broker);
	}
	else
	{
		int error = posix_socket_last_error();

		if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
			broker_close(broker, 1);
	}
}

/* ---------- addresses */

/* this machine's addresses and its WebRTC (p2p_webrtc_describe), for a
message to the machine with that identifier: the count of IPv4 addresses
(MAXIMUM_MESSAGE_ADDRESSES at most), each address and port, then the
WebRTC's kind and, unless none, the certificate's hash, the ICE username
fragment and password, each after its length, and the count of mDNS names,
each after its length and before its port. A native build's IPv6 addresses
go as names (IPV6_NAME_PREFIX), which needs a WebRTC kind: one without its
certificate offers none. Returns the bytes written */
static int put_addresses(unsigned char *message, const unsigned char *identifier, int proven)
{
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	struct p2p_webrtc webrtc;
	int count = p2p_webrtc_describe(identifier, proven, &webrtc, candidates, P2P_MAXIMUM_CANDIDATES);
	int ipv4_count = 0;
	int size;
	int index;

	size = 1;
	for (index = 0; index < count; index++)
	{
		if (candidates[index].ipv6)
		{
			if (webrtc.kind == _p2p_webrtc_native && webrtc.name_count < P2P_MAXIMUM_WEBRTC_NAMES)
			{
				char hex[2 * 16 + 1];

				p2p_hex(candidates[index].address6, 16, hex);
				snprintf(webrtc.names[webrtc.name_count], P2P_WEBRTC_NAME_SIZE, "%s%s%s", IPV6_NAME_PREFIX, hex,
					IPV6_NAME_SUFFIX);
				webrtc.name_ports[webrtc.name_count++] = candidates[index].port;
			}
			continue;
		}
		if (ipv4_count == MAXIMUM_MESSAGE_ADDRESSES)
			continue;
		memcpy(message + size, &candidates[index].address, 4);
		memcpy(message + size + 4, &candidates[index].port, 2);
		size += 6;
		ipv4_count++;
	}
	message[0] = (unsigned char)ipv4_count;
	message[size++] = (unsigned char)webrtc.kind;
	if (webrtc.kind == _p2p_webrtc_none)
		return size;
	memcpy(message + size, webrtc.fingerprint, P2P_FINGERPRINT_SIZE);
	size += P2P_FINGERPRINT_SIZE;
	message[size] = (unsigned char)strlen(webrtc.ufrag);
	memcpy(message + size + 1, webrtc.ufrag, message[size]);
	size += 1 + message[size];
	message[size] = (unsigned char)strlen(webrtc.password);
	memcpy(message + size + 1, webrtc.password, message[size]);
	size += 1 + message[size];
	message[size++] = (unsigned char)webrtc.name_count;
	for (index = 0; index < webrtc.name_count; index++)
	{
		message[size] = (unsigned char)strlen(webrtc.names[index]);
		memcpy(message + size + 1, webrtc.names[index], message[size]);
		size += 1 + message[size];
		memcpy(message + size, &webrtc.name_ports[index], 2);
		size += 2;
	}
	return size;
}

/* whether text is an mDNS name of a browser's address: letters, digits and
"-" before ".local" (a browser puts it in its session description) */
static int webrtc_name_valid(const char *text)
{
	int length = (int)strlen(text);

	return length > 6 && !strcmp(text + length - 6, ".local") && (int)strspn(text,
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-") == length - 6;
}

/* ICE's text (RFC 8839's ice-char: letters, digits, "+" and "/"), of a
length from minimum to maximum, or none; a browser puts it in its session
description */
static int get_ice_text(const unsigned char *message, int size, int *offset, int minimum, int maximum,
	char *text)
{
	int length;
	int index;

	if (*offset >= size)
		return 0;
	length = message[(*offset)++];
	if ((length && (length < minimum || length > maximum)) || *offset + length > size)
		return 0;
	for (index = 0; index < length; index++)
	{
		char character = (char)message[*offset + index];

		if (!((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
			(character >= '0' && character <= '9') || character == '+' || character == '/'))
		{
			return 0;
		}
		text[index] = character;
	}
	text[length] = 0;
	*offset += length;
	return 1;
}

/* the IPv6 address a name of IPV6_NAME_PREFIX holds (put_addresses), into
address (16 bytes); 0 if it holds none */
static int ipv6_from_name(const char *name, unsigned char *address)
{
	int prefix = (int)strlen(IPV6_NAME_PREFIX);
	int index;

	if ((int)strlen(name) != prefix + 2 * 16 + (int)strlen(IPV6_NAME_SUFFIX) ||
		strncmp(name, IPV6_NAME_PREFIX, (size_t)prefix) || strcmp(name + prefix + 2 * 16, IPV6_NAME_SUFFIX))
	{
		return 0;
	}
	for (index = 0; index < 2 * 16; index++)
	{
		char digit = name[prefix + index];
		int value = digit >= '0' && digit <= '9' ? digit - '0' : digit >= 'a' && digit <= 'f' ? digit - 'a' + 10 : -1;

		if (value < 0)
			return 0;
		if (index & 1)
			address[index / 2] = (unsigned char)(address[index / 2] | value);
		else
			address[index / 2] = (unsigned char)(value << 4);
	}
	return 1;
}

/* the reverse: the bytes read, or -1 if they are not made so. A native
build's IPv6 addresses (its names) come after its IPv4 ones in candidates,
and are not left among its names */
static int get_addresses(const unsigned char *message, int size, struct p2p_candidate *candidates, int *count,
	struct p2p_webrtc *webrtc)
{
	int offset;
	int index;
	int kept;

	memset(webrtc, 0, sizeof(*webrtc));
	if (size < 2)
		return -1;
	*count = message[0];
	if (*count > MAXIMUM_MESSAGE_ADDRESSES || size < 2 + *count * 6)
		return -1;
	for (index = 0; index < *count; index++)
	{
		unsigned int address;
		unsigned short port;

		memcpy(&address, message + 1 + index * 6, 4);
		memcpy(&port, message + 1 + index * 6 + 4, 2);
		p2p_candidate_ipv4(&candidates[index], address, port);
	}
	offset = 1 + *count * 6;
	webrtc->kind = message[offset++];
	if (webrtc->kind == _p2p_webrtc_none)
		return offset;
	if ((webrtc->kind != _p2p_webrtc_native && webrtc->kind != _p2p_webrtc_browser) ||
		offset + P2P_FINGERPRINT_SIZE > size)
	{
		return -1;
	}
	memcpy(webrtc->fingerprint, message + offset, P2P_FINGERPRINT_SIZE);
	offset += P2P_FINGERPRINT_SIZE;
	if (!get_ice_text(message, size, &offset, 4, P2P_ICE_UFRAG_SIZE - 1, webrtc->ufrag) ||
		!get_ice_text(message, size, &offset, 22, P2P_ICE_PASSWORD_SIZE - 1, webrtc->password) ||
		!webrtc->ufrag[0] != !webrtc->password[0] || offset >= size)
	{
		return -1;
	}
	webrtc->name_count = message[offset++];
	if (webrtc->name_count > P2P_MAXIMUM_WEBRTC_NAMES)
		return -1;
	for (index = 0; index < webrtc->name_count; index++)
	{
		int length;

		if (offset >= size)
			return -1;
		length = message[offset++];
		if (length >= P2P_WEBRTC_NAME_SIZE || offset + length + 2 > size)
			return -1;
		memcpy(webrtc->names[index], message + offset, (size_t)length);
		webrtc->names[index][length] = 0;
		memcpy(&webrtc->name_ports[index], message + offset + length, 2);
		offset += length + 2;
		if (!webrtc_name_valid(webrtc->names[index]) || !webrtc->name_ports[index])
			return -1;
	}
	/* (a browser's names stay names: they are its own) */
	for (index = 0, kept = 0; index < webrtc->name_count; index++)
	{
		unsigned char address[16];

		if (webrtc->kind == _p2p_webrtc_native && ipv6_from_name(webrtc->names[index], address))
		{
			if (*count < P2P_MAXIMUM_CANDIDATES)
				p2p_candidate_ipv6(&candidates[(*count)++], address, webrtc->name_ports[index]);
			continue;
		}
		if (kept != index)
		{
			memcpy(webrtc->names[kept], webrtc->names[index], P2P_WEBRTC_NAME_SIZE);
			webrtc->name_ports[kept] = webrtc->name_ports[index];
		}
		kept++;
	}
	webrtc->name_count = kept;
	return offset;
}

/* ---------- the keys: what only a joiner and the host can work out */

/* HMAC(their X25519 shared secret, "hceu/2" | the joiner's public key | the
host's); other is the one that is not this machine's. 0 if it is unusable */
static int pair_base(const unsigned char *joiner_public, const unsigned char *host_public,
	const unsigned char *other, unsigned char *base)
{
	unsigned char shared[P2P_KEY_SIZE];
	unsigned char data[6 + 2 * P2P_KEY_SIZE];

	if (!p2p_shared_secret(other, shared))
		return 0;
	memcpy(data, "hceu/2", 6);
	memcpy(data + 6, joiner_public, P2P_KEY_SIZE);
	memcpy(data + 6 + P2P_KEY_SIZE, host_public, P2P_KEY_SIZE);
	p2p_hmac_sha256(shared, P2P_KEY_SIZE, data, sizeof(data), base);
	return 1;
}

/* a session's secret, which the tunnel's keys come from (p2p.c) */
static void session_secret(const unsigned char *base, const unsigned char *nonce, const unsigned char *host_nonce,
	unsigned char *secret)
{
	unsigned char data[7 + 2 * NONCE_SIZE];

	memcpy(data, "session", 7);
	memcpy(data + 7, nonce, NONCE_SIZE);
	memcpy(data + 7 + NONCE_SIZE, host_nonce, NONCE_SIZE);
	p2p_hmac_sha256(base, P2P_SHA256_SIZE, data, sizeof(data), secret);
}

/* an ACCEPT's tag (label "accept") or a proven JOIN's ("join"), over what
precedes it */
static void message_tag(const unsigned char *base, const char *label, const unsigned char *message, int size,
	unsigned char *tag)
{
	unsigned char data[6 + MAXIMUM_MESSAGE_SIZE];
	unsigned char digest[P2P_SHA256_SIZE];
	int label_size = (int)strlen(label);

	memcpy(data, label, (size_t)label_size);
	memcpy(data + label_size, message, (size_t)size);
	p2p_hmac_sha256(base, P2P_SHA256_SIZE, data, label_size + size, digest);
	memcpy(tag, digest, TAG_SIZE);
}

/* whether a message's tag (its last TAG_SIZE bytes) is right */
static int tag_right(const unsigned char *base, const char *label, const unsigned char *message, int size)
{
	unsigned char tag[TAG_SIZE];

	message_tag(base, label, message, size - TAG_SIZE, tag);
	return p2p_equal(tag, message + size - TAG_SIZE, TAG_SIZE);
}

/* the host's nonce for a request (a joiner's public key and nonce) in a
period of HOST_NONCE_PERIOD */
static void host_nonce_for(const unsigned char *public_key, const unsigned char *nonce, unsigned long period,
	unsigned char *host_nonce)
{
	unsigned char data[4 + P2P_KEY_SIZE + NONCE_SIZE];
	unsigned char digest[P2P_SHA256_SIZE];
	int index;

	for (index = 0; index < 4; index++)
		data[index] = (unsigned char)(period >> (index * 8));
	memcpy(data + 4, public_key, P2P_KEY_SIZE);
	memcpy(data + 4 + P2P_KEY_SIZE, nonce, NONCE_SIZE);
	p2p_hmac_sha256(signalling.nonce_key, P2P_SHA256_SIZE, data, sizeof(data), digest);
	memcpy(host_nonce, digest, NONCE_SIZE);
}

/* whether the host answered a request with this nonce lately: in this
period, or the last */
static int host_nonce_current(const unsigned char *public_key, const unsigned char *nonce,
	const unsigned char *host_nonce)
{
	unsigned long period = p2p_now() / HOST_NONCE_PERIOD;
	unsigned char expected[NONCE_SIZE];

	host_nonce_for(public_key, nonce, period, expected);
	if (p2p_equal(expected, host_nonce, NONCE_SIZE))
		return 1;
	/* (the last before the clock wraps, before the first) */
	host_nonce_for(public_key, nonce, period ? period - 1 : 0xFFFFFFFFUL / HOST_NONCE_PERIOD, expected);
	return p2p_equal(expected, host_nonce, NONCE_SIZE);
}

/* ---------- the messages */

/* the probe a request is, or NULL (one of the joins) */
static struct probe *join_probe(struct join *join)
{
	int index;

	for (index = 0; index < P2P_MAXIMUM_PROBES; index++)
	{
		if (join == &signalling.probes[index].join)
			return &signalling.probes[index];
	}
	return NULL;
}

static void send_join(struct join *join)
{
	struct probe const *probe = join_probe(join);
	unsigned char message[MAXIMUM_MESSAGE_SIZE];
	unsigned char sealed[MAXIMUM_MESSAGE_SIZE + P2P_SEAL_OVERHEAD];
	int size = 0;

	message[size++] = _message_join;
	message[size++] = MESSAGE_VERSION;
	memcpy(message + size, p2p_public_key(), P2P_KEY_SIZE);
	size += P2P_KEY_SIZE;
	memcpy(message + size, join->nonce, NONCE_SIZE);
	size += NONCE_SIZE;
	size += put_addresses(message + size, join->host, 1);
	/* answered: the proof that this machine holds its key, of which the host
	makes the session */
	if (join->answered)
	{
		memcpy(message + size, join->host_nonce, NONCE_SIZE);
		size += NONCE_SIZE;
		message_tag(join->base, "join", message, size, message + size);
		size += TAG_SIZE;
	}
	size = p2p_seal(probe ? probe->key : signalling.join_key, message, size, sealed);
	publish_everywhere(join->host_topic, sealed, size);
	join->sent_time = p2p_now();
}

/* whether any request to join is under way */
static int joins_active(void)
{
	int index;

	for (index = 0; index < MAXIMUM_JOINS; index++)
	{
		if (signalling.joins[index].active)
			return 1;
	}
	return 0;
}

/* starts a request to the machine whose key has this hash, holding this
token, with a new nonce (the host makes one session of a request) */
static void join_start(struct join *join, const unsigned char *host_hash, const unsigned char *token)
{
	memset(join, 0, sizeof(*join));
	memcpy(join->host_hash, host_hash, P2P_KEY_HASH_SIZE);
	p2p_identifier_from_hash(host_hash, join->host);
	make_topic(token, "host", join->host, join->host_topic);
	posix_random_bytes(join->nonce, NONCE_SIZE);
	join->nonce_time = p2p_now();
	join->active = 1;
	sync_all_topics();
	send_join(join);
}

/* the request of a key and nonce (NULL: any) in a list */
static struct joiner *find_joiner(struct joiner *list, int count, const unsigned char *public_key,
	const unsigned char *nonce)
{
	int index;

	for (index = 0; index < count; index++)
	{
		if (list[index].used && !memcmp(list[index].public_key, public_key, P2P_KEY_SIZE) &&
			(!nonce || !memcmp(list[index].nonce, nonce, NONCE_SIZE)))
		{
			return &list[index];
		}
	}
	return NULL;
}

/* pair_base with a joiner: kept from a request of its, else worked out */
static int joiner_base(const unsigned char *public_key, unsigned char *base)
{
	int index;

	for (index = 0; index < MAXIMUM_JOINERS + MAXIMUM_ASKERS; index++)
	{
		struct joiner const *joiner = index < MAXIMUM_JOINERS ? &signalling.joiners[index] :
			&signalling.askers[index - MAXIMUM_JOINERS];

		if (joiner->used && !memcmp(joiner->public_key, public_key, P2P_KEY_SIZE))
		{
			memcpy(base, joiner->base, P2P_SHA256_SIZE);
			return 1;
		}
	}
	/* (none left: the request is not answered, and asked again) */
	if (!budget_left(&signalling.key_work, MAXIMUM_KEY_WORK, KEY_WORK_INTERVAL, 1))
		return 0;
	return pair_base(public_key, p2p_public_key(), public_key, base);
}

static int request_used(const unsigned char *request)
{
	int index;

	for (index = 0; index < signalling.used_request_count; index++)
	{
		if (!memcmp(signalling.used_requests[index].request, request, P2P_IDENTIFIER_SIZE + NONCE_SIZE))
			return 1;
	}
	return 0;
}

/* the host's answer to a request, through the broker it came through */
static void send_accept(struct broker *broker, const unsigned char *identifier, const unsigned char *nonce,
	const unsigned char *host_nonce, const unsigned char *base, int proven)
{
	unsigned char answer[MAXIMUM_MESSAGE_SIZE];
	unsigned char sealed[MAXIMUM_MESSAGE_SIZE + P2P_SEAL_OVERHEAD];
	char topic[TOPIC_SIZE];
	int size = 0;

	answer[size++] = _message_accept;
	answer[size++] = MESSAGE_VERSION;
	memcpy(answer + size, p2p_public_key(), P2P_KEY_SIZE);
	size += P2P_KEY_SIZE;
	memcpy(answer + size, nonce, NONCE_SIZE);
	size += NONCE_SIZE;
	memcpy(answer + size, host_nonce, NONCE_SIZE);
	size += NONCE_SIZE;
	size += put_addresses(answer + size, identifier, proven);
	message_tag(base, "accept", answer, size, answer + size);
	size += TAG_SIZE;
	size = p2p_seal(signalling.host_key, answer, size, sealed);
	make_topic(signalling.host_token, "joiner", identifier, topic);
	/* (a proven joiner's first: its session is made; others ask again) */
	if (broker->state == _broker_ready && broker_may_publish(broker, proven ? 0 : UNPROVEN_RESERVE))
		broker_publish(broker, topic, sealed, size);
}

/* the host: a joiner asked (through broker); with a proof (the host's
nonce, and a tag) once the host answered it */
static void join_received(struct broker *broker, const unsigned char *message, int size)
{
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	struct p2p_webrtc webrtc;
	static const struct p2p_webrtc no_webrtc;
	unsigned char identifier[P2P_IDENTIFIER_SIZE];
	unsigned char request[P2P_IDENTIFIER_SIZE + NONCE_SIZE];
	unsigned char host_nonce[NONCE_SIZE];
	unsigned char base[P2P_SHA256_SIZE];
	unsigned char secret[P2P_SHA256_SIZE];
	const unsigned char *public_key = message + 2;
	const unsigned char *nonce = public_key + P2P_KEY_SIZE;
	int fixed = 2 + P2P_KEY_SIZE + NONCE_SIZE;
	int broker_index = (int)(broker - signalling.brokers);
	struct joiner *joiner;
	struct used_request *used;
	int proven;
	int count;
	int read;

	/* (a browser answers once its WebRTC can be told: the joiner asks
	again) */
	if (size < fixed + 1 || !p2p_webrtc_ready())
		return;
	read = get_addresses(message + fixed, size - fixed, candidates, &count, &webrtc);
	if (read < 0)
		return;
	proven = size - fixed - read;
	if (proven != 0 && proven != PROOF_SIZE)
		return;
	p2p_identifier_for(public_key, identifier);
	joiner = find_joiner(signalling.joiners, MAXIMUM_JOINERS, public_key, nonce);
	if (joiner)
	{
		/* a request a session was made from, again (through another broker,
		or repeated until the tunnel reaches the host): that session's
		answer, while it lasts; never another. The addresses only from a
		proof (anyone can send the rest) */
		if (proven && (memcmp(message + size - PROOF_SIZE, joiner->host_nonce, NONCE_SIZE) ||
			!tag_right(joiner->base, "join", message, size)))
		{
			return;
		}
		if (!elapsed(joiner->answered_broker_times[broker_index], ANSWER_INTERVAL) ||
			!p2p_peer_reoffered(identifier, joiner->secret, candidates, proven ? count : 0,
			proven ? &webrtc : &no_webrtc) ||
			(!proven && !budget_left(&signalling.unproven_answers, MAXIMUM_UNPROVEN_ANSWERS,
			UNPROVEN_ANSWER_INTERVAL, 1)))
		{
			return;
		}
		joiner->answered_time = p2p_now();
		joiner->answered_broker_times[broker_index] = joiner->answered_time;
		send_accept(broker, identifier, joiner->nonce, joiner->host_nonce, joiner->base, proven);
		return;
	}
	memcpy(request, identifier, P2P_IDENTIFIER_SIZE);
	memcpy(request + P2P_IDENTIFIER_SIZE, nonce, NONCE_SIZE);
	if (request_used(request))
		return;
	if (!proven)
	{
		/* an answer (to the machine whose key it is, which alone can prove
		the request), and nothing else: the host keeps nothing of it that it
		needs. A request's once each ANSWER_INTERVAL (a key's with another
		nonce too: anyone may send its key with theirs, which must not keep
		its own out), and few in all */
		struct joiner *asker = find_joiner(signalling.askers, MAXIMUM_ASKERS, public_key, NULL);
		int slot = 0;

		if (asker)
		{
			int index;

			for (index = 0; index < ASKER_NONCES; index++)
			{
				if (!memcmp(asker->answered_nonces[index], nonce, NONCE_SIZE) &&
					asker->answered_nonce_brokers[index] == broker_index)
				{
					slot = index;
					break;
				}
				if ((long)(asker->answered_nonce_times[index] - asker->answered_nonce_times[slot]) < 0)
					slot = index;
			}
			if (index < ASKER_NONCES && !elapsed(asker->answered_nonce_times[index], ANSWER_INTERVAL))
				return;
		}
		/* (checked before the work of the keys, which anyone with the invite
		can ask for as often as they like) */
		if (p2p_peer_turned_away(identifier, 0) ||
			!budget_left(&signalling.unproven_answers, MAXIMUM_UNPROVEN_ANSWERS, UNPROVEN_ANSWER_INTERVAL, 0))
		{
			return;
		}
		if (!asker)
		{
			if (!joiner_base(public_key, base))
				return;
			asker = &signalling.askers[signalling.next_asker];
			signalling.next_asker = (signalling.next_asker + 1) % MAXIMUM_ASKERS;
			memset(asker, 0, sizeof(*asker));
			memcpy(asker->identifier, identifier, P2P_IDENTIFIER_SIZE);
			memcpy(asker->public_key, public_key, P2P_KEY_SIZE);
			memcpy(asker->base, base, P2P_SHA256_SIZE);
			asker->used = 1;
		}
		budget_left(&signalling.unproven_answers, MAXIMUM_UNPROVEN_ANSWERS, UNPROVEN_ANSWER_INTERVAL, 1);
		memcpy(asker->nonce, nonce, NONCE_SIZE);
		asker->answered_time = p2p_now();
		memcpy(asker->answered_nonces[slot], nonce, NONCE_SIZE);
		asker->answered_nonce_brokers[slot] = (signed char)broker_index;
		asker->answered_nonce_times[slot] = asker->answered_time;
		host_nonce_for(public_key, nonce, p2p_now() / HOST_NONCE_PERIOD, host_nonce);
		send_accept(broker, identifier, nonce, host_nonce, asker->base, 0);
		return;
	}
	/* proven: with a nonce the host answered the request with lately, and a
	tag only the key's holder can make. A new session (p2p.c turns it away
	while another with that machine lives) */
	memcpy(host_nonce, message + size - PROOF_SIZE, NONCE_SIZE);
	if (!host_nonce_current(public_key, nonce, host_nonce) || p2p_peer_turned_away(identifier, 0) ||
		!joiner_base(public_key, base) || !tag_right(base, "join", message, size))
	{
		return;
	}
	/* (a request is remembered while its proof lasts, at least: a copy of it
	would make the session again, with the same keys) */
	used = &signalling.used_requests[signalling.used_request_next];
	if (signalling.used_request_count == MAXIMUM_USED_REQUESTS && !elapsed(used->time, USED_REQUEST_TIME))
		return;
	session_secret(base, nonce, host_nonce, secret);
	if (!p2p_peer_offered(identifier, public_key, secret, candidates, count, &webrtc, _p2p_offered_joiner))
		return;
	memcpy(used->request, request, sizeof(request));
	used->time = p2p_now();
	signalling.used_request_next = (signalling.used_request_next + 1) % MAXIMUM_USED_REQUESTS;
	if (signalling.used_request_count < MAXIMUM_USED_REQUESTS)
		signalling.used_request_count++;
	joiner = &signalling.joiners[signalling.next_joiner];
	signalling.next_joiner = (signalling.next_joiner + 1) % MAXIMUM_JOINERS;
	memcpy(joiner->identifier, identifier, P2P_IDENTIFIER_SIZE);
	memcpy(joiner->public_key, public_key, P2P_KEY_SIZE);
	memcpy(joiner->nonce, nonce, NONCE_SIZE);
	memcpy(joiner->host_nonce, host_nonce, NONCE_SIZE);
	memcpy(joiner->base, base, P2P_SHA256_SIZE);
	memcpy(joiner->secret, secret, P2P_SHA256_SIZE);
	joiner->answered_time = p2p_now();
	memset(joiner->answered_broker_times, 0, sizeof(joiner->answered_broker_times));
	joiner->answered_broker_times[broker_index] = joiner->answered_time;
	joiner->used = 1;
	send_accept(broker, identifier, nonce, host_nonce, base, 1);
}

/* the joiner: the host (or the player) asked answered */
static void accept_received(const unsigned char *message, int size)
{
	struct p2p_candidate candidates[P2P_MAXIMUM_CANDIDATES];
	struct p2p_webrtc webrtc;
	unsigned char hash[P2P_KEY_HASH_SIZE];
	unsigned char secret[P2P_SHA256_SIZE];
	const unsigned char *host_public = message + 2;
	const unsigned char *nonce = host_public + P2P_KEY_SIZE;
	const unsigned char *host_nonce = nonce + NONCE_SIZE;
	int fixed = 2 + P2P_KEY_SIZE + 2 * NONCE_SIZE;
	struct join *join = NULL;
	int count;
	int index;

	if (size < fixed + 1 + TAG_SIZE)
		return;
	/* (the request it answers: by its nonce) */
	for (index = 0; index < MAXIMUM_JOINS + P2P_MAXIMUM_PROBES && !join; index++)
	{
		struct join *asked = index < MAXIMUM_JOINS ? &signalling.joins[index] :
			&signalling.probes[index - MAXIMUM_JOINS].join;

		if (asked->active && !memcmp(nonce, asked->nonce, NONCE_SIZE))
			join = asked;
	}
	if (!join)
		return;
	/* the first answer to the request holds: the proof carries its host
	nonce (the host's changes every HOST_NONCE_PERIOD) */
	if (join->answered && (memcmp(host_public, join->host_public, P2P_KEY_SIZE) ||
		memcmp(host_nonce, join->host_nonce, NONCE_SIZE)))
	{
		return;
	}
	/* the machine asked: its key has the hash in the invite (or the one
	the game's host told) */
	p2p_key_hash(host_public, hash);
	if (memcmp(hash, join->host_hash, P2P_KEY_HASH_SIZE))
		return;
	if (!join->has_base || memcmp(join->host_public, host_public, P2P_KEY_SIZE))
	{
		if (!pair_base(p2p_public_key(), host_public, host_public, join->base))
			return;
		memcpy(join->host_public, host_public, P2P_KEY_SIZE);
		join->has_base = 1;
	}
	/* and the answer is its */
	if (!tag_right(join->base, "accept", message, size))
		return;
	if (get_addresses(message + fixed, size - TAG_SIZE - fixed, candidates, &count, &webrtc) !=
		size - TAG_SIZE - fixed)
	{
		return;
	}
	session_secret(join->base, nonce, host_nonce, secret);
	if (!p2p_peer_offered(join->host, host_public, secret, candidates, count, &webrtc,
		join_probe(join) ? _p2p_offered_probe : join == &signalling.joins[0] ? _p2p_offered_host :
		_p2p_offered_player) || join->answered)
	{
		return;
	}
	memcpy(join->host_nonce, host_nonce, NONCE_SIZE);
	join->answered = 1;
	/* the proof, at once */
	send_join(join);
}

static void publish_received(struct broker *broker, const char *topic, const unsigned char *payload, int size,
	int retained)
{
	unsigned char message[MAXIMUM_MESSAGE_SIZE];
	int message_size;

	if (size > MAXIMUM_MESSAGE_SIZE + P2P_SEAL_OVERHEAD)
		return;
	if (!strncmp(topic, P2P_LOBBY_SLOT_PREFIX, sizeof(P2P_LOBBY_SLOT_PREFIX) - 1))
	{
		p2p_lobby_slot_heard(topic + sizeof(P2P_LOBBY_SLOT_PREFIX) - 1, payload, size, retained);
		return;
	}
	if (!strcmp(topic, P2P_LOBBY_QUERY_TOPIC))
	{
		p2p_lobby_query_heard();
		return;
	}
	if (signalling.hosting && !strcmp(topic, signalling.host_topic))
	{
		message_size = p2p_open(signalling.host_key, payload, size, message);
		if (message_size >= 2 && message[0] == _message_join && message[1] == MESSAGE_VERSION)
			join_received(broker, message, message_size);
	}
	else if (joins_active() && !strcmp(topic, signalling.join_topic))
	{
		message_size = p2p_open(signalling.join_key, payload, size, message);
		if (message_size >= 2 && message[0] == _message_accept && message[1] == MESSAGE_VERSION)
			accept_received(message, message_size);
	}
	else
	{
		int index;

		for (index = 0; index < P2P_MAXIMUM_PROBES; index++)
		{
			struct probe const *probe = &signalling.probes[index];

			if (!probe->join.active || strcmp(topic, probe->topic))
				continue;
			message_size = p2p_open(probe->key, payload, size, message);
			if (message_size >= 2 && message[0] == _message_accept && message[1] == MESSAGE_VERSION)
				accept_received(message, message_size);
			break;
		}
	}
}

/* a CONNACK: 0 if the connection is closed (refused, or to try 3.1.1) */
static int broker_acknowledged(struct broker *broker, const unsigned char *body, int size)
{
	int code = body[1];
	int offset = 2;
	int properties = 0;

	broker->retain_available = 1;
	broker->wildcard_available = 1;
	broker->keep_alive = KEEP_ALIVE_SECONDS;
	broker->receive_maximum = MAXIMUM_IN_FLIGHT;
	/* a broker that has not MQTT 5 answers 0x84 (5's unsupported protocol
	version), or 3.1.1's 1 (unacceptable protocol version) */
	if (broker->protocol == 5 && (code == 0x84 || (size == 2 && code == 1)))
	{
		platform_log("Internet play: the signalling broker %s has not MQTT 5; using 3.1.1", broker->host);
		broker->protocol = 4;
		broker_close(broker, 0);
		/* (again at once) */
		broker->state_time = p2p_now() - MAXIMUM_RETRY_INTERVAL;
		return 0;
	}
	if (code != 0)
	{
		platform_log("Internet play: the signalling broker %s refused the connection", broker->host);
		broker_close(broker, 1);
		return 0;
	}
	if (broker->protocol == 5 && get_variable(body, size, &offset, &properties))
	{
		int end = offset + properties > size ? size : offset + properties;

		while (offset < end)
		{
			int property = body[offset++];

			switch (property)
			{
			/* a byte */
			case 0x01: case 0x17: case 0x19: case 0x24: case 0x25: case 0x28: case 0x29: case 0x2A:
				if (offset < end)
				{
					if (property == _property_retain_available)
						broker->retain_available = body[offset];
					else if (property == _property_wildcard_available)
						broker->wildcard_available = body[offset];
				}
				offset += 1;
				break;
			/* two */
			case 0x13: case 0x21: case 0x22: case 0x23:
				if (offset + 2 <= end)
				{
					int value = body[offset] << 8 | body[offset + 1];

					if (property == _property_server_keep_alive && value > 0)
						broker->keep_alive = value;
					else if (property == _property_receive_maximum && value > 0)
						broker->receive_maximum = value;
				}
				offset += 2;
				break;
			/* four */
			case 0x02: case 0x11: case 0x18: case 0x27:
				offset += 4;
				break;
			/* a variable byte integer */
			case 0x0B:
			{
				int value;

				if (!get_variable(body, end, &offset, &value))
					offset = end;
				break;
			}
			/* a string or binary data: a length, then that */
			case 0x03: case 0x08: case 0x09: case 0x12: case 0x15: case 0x16: case 0x1A: case 0x1C: case 0x1F:
				offset += offset + 2 <= end ? 2 + (body[offset] << 8 | body[offset + 1]) : 2;
				break;
			/* a pair of strings */
			case 0x26:
			{
				int pair;

				for (pair = 0; pair < 2; pair++)
					offset += offset + 2 <= end ? 2 + (body[offset] << 8 | body[offset + 1]) : 2;
				break;
			}
			default:
				/* (unknown: the rest can't be read) */
				offset = end;
				break;
			}
		}
	}
	broker->state = _broker_ready;
	broker->failures = 0;
	broker->publish_tokens = PUBLISH_BURST;
	broker->publish_time = p2p_now();
	broker_sync_topics(broker);
	/* a joiner's first requests need not wait for the next repeat */
	{
		int index;

		for (index = 0; index < MAXIMUM_JOINS; index++)
		{
			if (signalling.joins[index].active)
				send_join(&signalling.joins[index]);
		}
	}
	return 1;
}

/* the packets that arrived whole */
static void broker_parse(struct broker *broker)
{
	for (;;)
	{
		int remaining = 0;
		int shift = 0;
		int header_size = 1;
		int total;
		unsigned char type;

		for (;;)
		{
			unsigned char byte;

			if (header_size >= broker->input_size)
				return;
			byte = broker->input[header_size++];
			remaining |= (byte & 127) << shift;
			shift += 7;
			if (!(byte & 128))
				break;
			if (shift > 21)
			{
				broker_close(broker, 1);
				return;
			}
		}
		total = header_size + remaining;
		if (total > BUFFER_SIZE)
		{
			broker_close(broker, 1);
			return;
		}
		if (total > broker->input_size)
			return;
		type = broker->input[0];
		if ((type & 0xF0) == 0x20 && remaining >= 2)
		{
			if (!broker_acknowledged(broker, broker->input + header_size, remaining))
				return;
		}
		else if ((type & 0xF0) == 0x30 && remaining >= 2)
		{
			/* PUBLISH */
			const unsigned char *body = broker->input + header_size;
			int topic_size = body[0] << 8 | body[1];
			int quality = (type >> 1) & 3;
			int offset = 2 + topic_size;
			int properties = 0;

			if (quality)
			{
				/* (subscriptions ask for at most once; acknowledged all the
				same) */
				if (offset + 2 <= remaining && quality == 1)
					broker_send(broker, 0x40, body + offset, 2);
				offset += 2;
			}
			if (broker->protocol == 5 && get_variable(body, remaining, &offset, &properties))
				offset += properties;
			if (topic_size < TOPIC_SIZE && offset <= remaining)
			{
				char topic[TOPIC_SIZE];

				memcpy(topic, body + 2, (size_t)topic_size);
				topic[topic_size] = 0;
				publish_received(broker, topic, body + offset, remaining - offset, type & 1);
			}
		}
		else if ((type & 0xF0) == 0x40 && remaining >= 2)
		{
			/* PUBACK: MQTT 5's reason, if any, of 0x80 or more is a refusal
			(a quota, a rate): sent again later */
			const unsigned char *body = broker->input + header_size;
			unsigned short identifier = (unsigned short)(body[0] << 8 | body[1]);
			int refused = remaining >= 3 && body[2] >= 0x80;
			int index;

			for (index = 0; index < MAXIMUM_IN_FLIGHT; index++)
			{
				if (broker->in_flight[index].used && broker->in_flight[index].identifier == identifier)
				{
					broker->in_flight[index].used = refused;
					broker->in_flight[index].refused = refused;
				}
			}
		}
		else if ((type & 0xF0) == 0xE0)
		{
			/* DISCONNECT (MQTT 5): the broker's */
			broker_close(broker, 1);
			return;
		}
		/* (what it sent in answer may have closed it, emptying input) */
		if (broker->socket < 0)
			return;
		memmove(broker->input, broker->input + total, (size_t)(broker->input_size - total));
		broker->input_size -= total;
	}
}

static void broker_readable(struct broker *broker)
{
	int reads;

	/* (the rest in the next pass) */
	for (reads = 0; reads < MAXIMUM_BROKER_READS; reads++)
	{
		int size = posix_socket_recv(broker->socket, broker->input + broker->input_size,
			BUFFER_SIZE - broker->input_size, 0);

		if (size == 0)
		{
			broker_close(broker, 1);
			return;
		}
		if (size < 0)
		{
			int error = posix_socket_last_error();

			if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
				broker_close(broker, 1);
			return;
		}
		broker->input_size += size;
		broker->heard_time = p2p_now();
		broker_parse(broker);
		if (broker->socket < 0 || broker->input_size == BUFFER_SIZE)
			return;
	}
}

/* ---------- p2p.c's side */

/* port/assets/network/brokers.txt's brokers, for a game with no such file
beside its config.toml (one run from somewhere the builds did not put it).
Keep it the same as that file (test_linux_port.py checks) */
#define DEFAULT_BROKERS "opence.milenko.org:1883,broker.emqx.io:1883,broker.hivemq.com:1883,test.mosquitto.org:1883"

/* the brokers in network.brokers_file (beside config.toml, unless a full
path: port/assets/network/brokers.txt, which the builds put there), one on
each line, "#" starting a comment, into text: host:port entries separated by
commas; DEFAULT_BROKERS if the file cannot be read */
static void brokers_list(char *text, size_t size)
{
	const char *name = config_string("network.brokers_file");
	char path[1024];
	char *file;
	size_t file_size = 0, index, length = 0;
	int comment = 0;

	text[0] = 0;
#ifdef HALO_WEB
	{
		/* a page reaches brokers over WebSockets alone (web_net.c): the
		WebSocket URLs of brokers.txt's that have them, or those the page
		names (?brokers=, for tests) */
		const char *page = getenv("HALO_WEB_BROKERS");

		(void)name, (void)path, (void)file, (void)file_size, (void)index, (void)length, (void)comment;
		snprintf(text, size, "%s", page && *page ? page :
			"wss://broker.emqx.io:8084/mqtt,wss://broker.hivemq.com:8884/mqtt,wss://test.mosquitto.org:8081/mqtt");
		return;
	}
#endif
	if (name[0] == '/' || name[0] == '\\' || (name[0] && name[1] == ':'))
		snprintf(path, sizeof(path), "%s", name);
	else
	{
		config_folder(path, sizeof(path));
		snprintf(path + strlen(path), sizeof(path) - strlen(path), "%s", name);
	}
	file = config_file_read(path, &file_size);
	if (!file)
	{
		platform_log("Internet play: the brokers' file %s cannot be read (network.brokers_file); "
			"using the game's own list", path);
		snprintf(text, size, "%s", DEFAULT_BROKERS);
		return;
	}
	for (index = 0; index < file_size && length + 1 < size; index++)
	{
		char character = file[index];

		if (character == '\n' || character == '\r')
			comment = 0;
		else if (character == '#')
			comment = 1;
		if (!comment)
			text[length++] = character == '\n' || character == '\r' || character == '\t' ? ',' : character;
	}
	text[length] = 0;
	free(file);
}

void p2p_signal_start(void)
{
	char list[1024];
	const char *text;
	unsigned char random[8];
	char hex[17];

	if (signalling.started)
		return;
	signalling.started = 1;
	posix_random_bytes(random, sizeof(random));
	p2p_hex(random, sizeof(random), hex);
	snprintf(signalling.client_identifier, sizeof(signalling.client_identifier), "hceu-%s", hex);
	{
		unsigned char hash[P2P_KEY_HASH_SIZE];

		p2p_key_hash(p2p_public_key(), hash);
		p2p_lobby_slot_topic(hash, signalling.own_slot, sizeof(signalling.own_slot));
	}
	brokers_list(list, sizeof(list));
	text = list;
	while (*text && signalling.broker_count < MAXIMUM_BROKERS)
	{
		const char *end = text + strcspn(text, ",");
		struct broker *broker = &signalling.brokers[signalling.broker_count];
		char *colon;
		int length;

		while (text < end && *text == ' ')
			text++;
		length = (int)(end - text);
		while (length > 0 && text[length - 1] == ' ')
			length--;
		if (length > 0 && length < (int)sizeof(broker->host))
		{
			memset(broker, 0, sizeof(*broker));
			memcpy(broker->host, text, (size_t)length);
			broker->socket = -1;
			broker->port = network_short(1883);
			colon = strchr(broker->host, ':');
			/* (a WebSocket URL is its own address: web_net.c) */
			if (colon && strncmp(broker->host, "ws://", 5) && strncmp(broker->host, "wss://", 6))
			{
				broker->port = network_short((unsigned short)atoi(colon + 1));
				*colon = 0;
			}
			/* connect at once */
			broker->state_time = p2p_now() - RETRY_INTERVAL;
			signalling.broker_count++;
		}
		text = *end ? end + 1 : end;
	}
	if (!signalling.broker_count)
		platform_log("Internet play: no signalling brokers (network.brokers_file), so invites cannot work");
	else if (text[strspn(text, ", ")])
		platform_log("Internet play: only the first %d signalling brokers are used", MAXIMUM_BROKERS);
}

void p2p_signal_select_sets(int *read, int *read_count, int *write, int *write_count, int maximum_count)
{
	int index;
	int added = 0;

	for (index = 0; index < signalling.broker_count && added < maximum_count; index++)
	{
		struct broker *broker = &signalling.brokers[index];

		if (broker->socket < 0)
			continue;
		read[(*read_count)++] = broker->socket;
		if (broker->state == _broker_connecting || broker->output_size)
			write[(*write_count)++] = broker->socket;
		added++;
	}
}

static int list_holds(const int *list, int count, int socket)
{
	int index;

	for (index = 0; index < count; index++)
	{
		if (list[index] == socket)
			return 1;
	}
	return 0;
}

void p2p_signal_update(const int *read, int read_count, const int *write, int write_count)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		struct broker *broker = &signalling.brokers[index];
		int retry = broker->failures < 1 ? RETRY_INTERVAL :
			broker->failures >= 4 ? MAXIMUM_RETRY_INTERVAL : RETRY_INTERVAL << (broker->failures - 1);

		switch (broker->state)
		{
		case _broker_idle:
			if (elapsed(broker->state_time, (unsigned long)retry))
				broker_connect(broker);
			break;
		case _broker_connecting:
			if (list_holds(write, write_count, broker->socket))
				broker_connected(broker);
			else if (elapsed(broker->state_time, CONNECT_TIMEOUT))
				broker_close(broker, 1);
			break;
		case _broker_awaiting_acknowledgement:
			if (elapsed(broker->state_time, CONNECT_TIMEOUT))
				broker_close(broker, 1);
			break;
		}
		if (broker->socket < 0 || broker->state == _broker_connecting)
			continue;
		if (list_holds(write, write_count, broker->socket))
			broker_flush(broker);
		if (broker->socket >= 0 && list_holds(read, read_count, broker->socket))
			broker_readable(broker);
		if (broker->state != _broker_ready)
			continue;
		if (elapsed(broker->heard_time, (unsigned long)broker->keep_alive * 1500 > SILENCE_TIMEOUT ?
			(unsigned long)broker->keep_alive * 1500 : SILENCE_TIMEOUT))
		{
			broker_close(broker, 1);
			continue;
		}
		if (elapsed(broker->sent_time, (unsigned long)broker->keep_alive * 500 < PING_INTERVAL ?
			(unsigned long)broker->keep_alive * 500 : PING_INTERVAL))
		{
			broker_send(broker, 0xC0, NULL, 0);
		}
		broker_update_lobby(broker);
	}
	for (index = 0; index < MAXIMUM_JOINS + P2P_MAXIMUM_PROBES; index++)
	{
		struct join *join = index < MAXIMUM_JOINS ? &signalling.joins[index] :
			&signalling.probes[index - MAXIMUM_JOINS].join;

		if (!join->active)
			continue;
		if (!join->answered && elapsed(join->nonce_time, UNANSWERED_TIME))
		{
			posix_random_bytes(join->nonce, NONCE_SIZE);
			join->nonce_time = p2p_now();
			send_join(join);
		}
		if (elapsed(join->sent_time, JOIN_INTERVAL))
			send_join(join);
	}
}

int p2p_signal_connected(void)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		if (signalling.brokers[index].state == _broker_ready)
			return 1;
	}
	return 0;
}

void p2p_signal_reconnect(void)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		struct broker *broker = &signalling.brokers[index];

		if (broker->state == _broker_idle && broker->socket < 0 && !broker->failures)
			continue;
		broker_close(broker, 0);
		broker->failures = 0;
		/* (again at once) */
		broker->state_time = p2p_now() - MAXIMUM_RETRY_INTERVAL;
	}
}

static void sync_all_topics(void)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
		broker_sync_topics(&signalling.brokers[index]);
}

void p2p_signal_host(const unsigned char *token)
{
	if (!signalling.has_nonce_key)
	{
		posix_random_bytes(signalling.nonce_key, P2P_SHA256_SIZE);
		signalling.has_nonce_key = 1;
	}
	memcpy(signalling.host_token, token, P2P_TOKEN_SIZE);
	derive(token, "seal", NULL, signalling.host_key);
	make_topic(token, "host", p2p_identifier(), signalling.host_topic);
	signalling.hosting = 1;
	sync_all_topics();
}

void p2p_signal_stop_hosting(void)
{
	signalling.hosting = 0;
	memset(signalling.joiners, 0, sizeof(signalling.joiners));
	memset(signalling.askers, 0, sizeof(signalling.askers));
	sync_all_topics();
}

void p2p_signal_join(const unsigned char *host_hash, const unsigned char *token)
{
	int index;

	/* (the requests to the last game's players end: they hold another
	token, whose answers come to another topic) */
	for (index = 1; index < MAXIMUM_JOINS; index++)
		signalling.joins[index].active = 0;
	memcpy(signalling.join_token, token, P2P_TOKEN_SIZE);
	derive(token, "seal", NULL, signalling.join_key);
	make_topic(token, "joiner", p2p_identifier(), signalling.join_topic);
	/* (new each time: the host makes one session of a request; a browser's
	connection for the last goes, unless a session took it up) */
	p2p_webrtc_new_request();
	join_start(&signalling.joins[0], host_hash, token);
}

void p2p_signal_stop_joining(void)
{
	signalling.joins[0].active = 0;
	sync_all_topics();
}

int p2p_signal_join_player(const unsigned char *player_hash, const unsigned char *token)
{
	struct join *free_join = NULL;
	int index;

	/* (another token's requests would be answered on another topic) */
	if (signalling.joins[0].active && memcmp(token, signalling.join_token, P2P_TOKEN_SIZE))
		return 0;
	for (index = 1; index < MAXIMUM_JOINS; index++)
	{
		struct join *join = &signalling.joins[index];

		if (join->active && !memcmp(join->host_hash, player_hash, P2P_KEY_HASH_SIZE))
			return 1;
		if (!join->active && !free_join)
			free_join = join;
	}
	if (!free_join)
		return 0;
	if (!joins_active() || memcmp(token, signalling.join_token, P2P_TOKEN_SIZE))
	{
		memcpy(signalling.join_token, token, P2P_TOKEN_SIZE);
		derive(token, "seal", NULL, signalling.join_key);
		make_topic(token, "joiner", p2p_identifier(), signalling.join_topic);
		for (index = 1; index < MAXIMUM_JOINS; index++)
			signalling.joins[index].active = 0;
	}
	join_start(free_join, player_hash, token);
	return 1;
}

void p2p_signal_stop_joining_player(const unsigned char *identifier)
{
	int index;

	for (index = 1; index < MAXIMUM_JOINS; index++)
	{
		if (signalling.joins[index].active && !memcmp(signalling.joins[index].host, identifier, P2P_IDENTIFIER_SIZE))
		{
			signalling.joins[index].active = 0;
			sync_all_topics();
		}
	}
}

int p2p_signal_probe(const unsigned char *host_hash, const unsigned char *token)
{
	int index;

	for (index = 0; index < P2P_MAXIMUM_PROBES; index++)
	{
		struct probe *probe = &signalling.probes[index];

		if (probe->join.active)
			continue;
		derive(token, "seal", NULL, probe->key);
		make_topic(token, "joiner", p2p_identifier(), probe->topic);
		join_start(&probe->join, host_hash, token);
		return 1;
	}
	return 0;
}

void p2p_signal_stop_probe(const unsigned char *identifier)
{
	int index;

	for (index = 0; index < P2P_MAXIMUM_PROBES; index++)
	{
		struct join *join = &signalling.probes[index].join;

		if (join->active && !memcmp(join->host, identifier, P2P_IDENTIFIER_SIZE))
		{
			join->active = 0;
			sync_all_topics();
		}
	}
}

void p2p_signal_lobby_topics(int listed, int browsing)
{
	signalling.lobby_listed = listed;
	signalling.lobby_browsing = browsing;
	sync_all_topics();
}

void p2p_signal_lobby_publish(const unsigned char *listing, int size, int closing)
{
	if (size > MAXIMUM_LISTING_SIZE)
		return;
	memcpy(signalling.lobby_listing, listing, (size_t)size);
	signalling.lobby_listing_size = size;
	signalling.lobby_closing = closing;
	signalling.lobby_version++;
	/* (at once where the bucket allows; the rest in the coming passes) */
	{
		int index;

		for (index = 0; index < signalling.broker_count; index++)
			broker_update_lobby(&signalling.brokers[index]);
	}
}

void p2p_signal_lobby_query(void)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		if (signalling.brokers[index].topics[_topic_slots][0])
			signalling.brokers[index].query_pending = 1;
		broker_update_lobby(&signalling.brokers[index]);
	}
}

void p2p_signal_lobby_quit(void)
{
	int index;

	for (index = 0; index < signalling.broker_count; index++)
	{
		struct broker *broker = &signalling.brokers[index];
		static const unsigned char disconnect[2] = { 0, 0 };

		if (broker->state != _broker_ready)
			continue;
		/* (unless sent already: the tombstone and the clearing) */
		if (broker_carries_lobby(broker) && signalling.lobby_listing_size &&
			(broker->lobby_version != signalling.lobby_version || broker->lobby_clear_pending))
		{
			broker_publish_slot(broker, next_packet_identifier(broker), signalling.lobby_listing,
				signalling.lobby_listing_size, 0);
			broker_publish_slot(broker, next_packet_identifier(broker), NULL, 0, 0);
		}
		/* DISCONNECT (MQTT 5: a normal one, its reason and no properties) */
		broker_send(broker, 0xE0, disconnect, broker->protocol == 5 ? 2 : 0);
		broker_flush(broker);
	}
}

#include "harness.h"

/* The server browser's intake (port/linux/src/p2p_lobby.c, listing_wanted):
which listings heard take a place in the queue, and a broker's burst on
subscribing (its kept slots, most of them games long gone) leaving every live
game a place */
#include "config.inc"

static int newer_count;
static void newer_heard(const unsigned char *key_hash)
{
	(void)key_hash;
	newer_count++;
}

#include "under_test.inc"

/* a listing as a host makes it (listing_make), its signature zeroes: the
intake does not check it */
static int make(unsigned char *bytes, int version, int flags, long age)
{
	int size = 0;
	unsigned long when = (unsigned long)(time(NULL) - age);
	const char *texts[3] = { "a game", "bloodgulch", "slayer" };
	int text;

	bytes[size++] = 'H';
	bytes[size++] = 'L';
	bytes[size++] = LISTING_FORMAT;
	bytes[size++] = (unsigned char)(version >> 8);
	bytes[size++] = (unsigned char)version;
	bytes[size++] = (unsigned char)flags;
	memset(bytes + size, 0, 4);
	size += 4;
	bytes[size++] = (unsigned char)(when >> 24);
	bytes[size++] = (unsigned char)(when >> 16);
	bytes[size++] = (unsigned char)(when >> 8);
	bytes[size++] = (unsigned char)when;
	memset(bytes + size, 7, P2P_KEY_SIZE);
	size += P2P_KEY_SIZE;
	memset(bytes + size, 9, P2P_TOKEN_SIZE);
	size += P2P_TOKEN_SIZE;
	bytes[size++] = 1;
	bytes[size++] = 16;
	bytes[size++] = 2;
	for (text = 0; text < 3; text++)
	{
		int length = (int)strlen(texts[text]);

		bytes[size++] = (unsigned char)length;
		memcpy(bytes + size, texts[text], (size_t)length);
		size += length;
	}
	memset(bytes + size, 0, STAMP_SIZE + P2P_SIGNATURE_SIZE);
	return size + STAMP_SIZE + P2P_SIGNATURE_SIZE;
}

static int wanted(int version, int flags, long age, int retained)
{
	unsigned char bytes[MAXIMUM_LISTING_SIZE];
	unsigned char key_hash[P2P_KEY_HASH_SIZE] = { 0 };
	int size = make(bytes, version, flags, age);

	return listing_wanted(bytes, size, retained, key_hash);
}

int main(int argc, char **argv)
{
	char const *case_name = argc == 2 ? argv[1] : "";
	const int current = HALO_PORT_NETWORK_VERSION;

	CASE("filter")
	{
		unsigned char key_hash[P2P_KEY_HASH_SIZE] = { 0 };

		CHECK(wanted(current, _listing_open, 0, 0), "a live game of this version");
		CHECK(wanted(current, _listing_open, 120, 1), "a kept slot of two minutes ago");
		CHECK(wanted(current, _listing_closed, 60, 1), "a host's recent tombstone (it ends a game)");
		CHECK(!wanted(current, _listing_open, RETAINED_WINDOW + 60, 1), "a kept slot of a game long gone");
		CHECK(!wanted(current, _listing_open, LIVE_WINDOW + 60, 0), "a live listing long gone, sent again");
		CHECK(wanted(HALO_PORT_NETWORK_VERSION_OLDEST, _listing_open, 0, 0) && newer_count == 0,
			"an older version this build plays with: let in, not counted");
		CHECK(!wanted(HALO_PORT_NETWORK_VERSION_OLDEST - 1, _listing_open, 0, 0) && newer_count == 0,
			"a version older than the oldest it plays with: not counted");
		CHECK(!wanted(current + 1, _listing_open, 0, 0) && newer_count == 1, "a newer version's: counted");
		CHECK(!wanted(current + 1, _listing_closed, 0, 0) && newer_count == 1, "a newer version's tombstone");
		CHECK(!listing_wanted((const unsigned char *)"HL", 2, 0, key_hash), "a short message");
	}
	else CASE("burst")
	{
		/* a public broker's kept slots, as test.mosquitto.org sent them on
		2026-10-24 (about 1,450, some 70 live): this version's games long
		gone, older versions' hosts still running and long gone, a few
		newer ones; the live games spread among them */
		int heard, queued = 0, live_queued = 0;

		for (heard = 0; heard < 1470; heard++)
		{
			int live = heard % 21 == 20;
			int is_wanted;

			if (live)
				is_wanted = wanted(current, _listing_open, heard % 7, 1);
			else if (heard % 3 == 0)
				is_wanted = wanted(current, _listing_open, 86400 * (1 + heard % 30), 1);
			else if (heard % 3 == 1)
				is_wanted = wanted(HALO_PORT_NETWORK_VERSION_OLDEST - 1 - heard % 20, _listing_open,
					heard % 2 ? 30 : 86400, 1);
			else
				is_wanted = wanted(heard % 50 ? HALO_PORT_NETWORK_VERSION_OLDEST - 2 : current + 1, _listing_open, 5, 1);
			if (is_wanted && queued < MAXIMUM_QUEUED)
			{
				queued++;
				live_queued += live;
			}
		}
		CHECK(live_queued == 70, "%d of the 70 live games had a place in the queue (%d places taken)",
			live_queued, queued);
	}
	else
		return 2;
	return 0;
}

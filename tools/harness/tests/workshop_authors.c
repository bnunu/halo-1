/*
WORKSHOP_AUTHORS.C (test)

The Steam Workshop items' authors (port/linux/src/workshop_authors.c): Steam's details answer read for the item's
creator, a profile's name read and cleaned, and the items asked about one at a time, again only after a while once
Steam could not say, and never replaced while being asked about. test_workshop_authors.py takes the whole file, with
server_json.c, under_test.inc; its pthread.h is one thread's, and Steam is update_https_request here, answering
from the texts a case gives it.
*/

#include "harness.h"

#include <stdarg.h>

/* (the clock the retries are timed by: workshop_authors.c's time(NULL), after <time.h> has declared it) */
static long long test_now = 1000000;
#define time(pointer) ((time_t)test_now)

static int test_threads_started;
static int test_signals;

#include "under_test.inc"

/* Steam: the answers to the requests made, in turn (status 0: no answer came), and the requests */
static struct
{
	int calls;
	int statuses[8];
	const char *bodies[8];
	char methods[8][8];
	char urls[8][160];
	char sent[8][160];
	/* an item wanted while the next request is under way, as another thread would */
	const char *wanted_meanwhile;
} steam;

void platform_log(const char *format, ...)
{
	(void)format;
}

int update_https_request(const char *method, const char *url, const char *headers, const void *body,
	unsigned long body_size, unsigned long maximum, struct update_response *response, char *error, int error_size)
{
	int call = steam.calls++;

	(void)headers;
	(void)maximum;
	memset(response, 0, sizeof(*response));
	if (steam.wanted_meanwhile)
	{
		char name[WORKSHOP_AUTHORS_NAME_SIZE];
		const char *id = steam.wanted_meanwhile;

		steam.wanted_meanwhile = NULL;
		workshop_authors_get(id, name, sizeof(name));
	}
	if (call >= 8)
		return 0;
	snprintf(steam.methods[call], sizeof(steam.methods[call]), "%s", method);
	snprintf(steam.urls[call], sizeof(steam.urls[call]), "%s", url);
	snprintf(steam.sent[call], sizeof(steam.sent[call]), "%.*s", (int)body_size, body ? (const char *)body : "");
	if (!steam.statuses[call])
	{
		snprintf(error, (size_t)error_size, "offline");
		return 0;
	}
	response->status = steam.statuses[call];
	response->body_size = (unsigned long)strlen(steam.bodies[call]);
	response->body = malloc(response->body_size + 1);
	memcpy(response->body, steam.bodies[call], response->body_size + 1);
	return 1;
}

static const char details[] =
	"{\"response\":{\"result\":1,\"resultcount\":2,\"publishedfiledetails\":["
	"{\"publishedfileid\":\"111\",\"result\":1,\"creator\":\"76561198000000001\",\"title\":\"Other\"},"
	"{\"publishedfileid\":\"2898969630\",\"result\":1,\"creator\":\"76561198167031068\",\"creator_app_id\":1695795,"
	"\"title\":\"Coldsnap\",\"description\":\"A large ice plateau\",\"tags\":[{\"tag\":\"Multiplayer\"}]}]}}";
static const char not_found[] =
	"{\"response\":{\"result\":1,\"resultcount\":1,\"publishedfiledetails\":[{\"publishedfileid\":\"2898969630\","
	"\"result\":9}]}}";
static const char profile[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><profile>\n"
	"\t<steamID64>76561198167031068</steamID64>\n"
	"\t<steamID><![CDATA[aLTis94]]></steamID>\n"
	"\t<onlineState>offline</onlineState>\n</profile>";

static int creator_of(const char *text, const char *item, char creator[24])
{
	return workshop_authors_read_creator(text, (int)strlen(text), item, creator);
}

static int name_of(const char *text, char name[WORKSHOP_AUTHORS_NAME_SIZE])
{
	return workshop_authors_read_name(text, (int)strlen(text), name);
}

/* the item wanted, then asked about as the thread would: its author */
static const char *asked(const char *item)
{
	static char name[WORKSHOP_AUTHORS_NAME_SIZE];

	CHECK(!workshop_authors_get(item, name, sizeof(name)), "%s known before Steam was asked", item);
	CHECK(workshop_authors_next(), "nothing wanted after %s was", item);
	workshop_authors_get(item, name, sizeof(name));
	return name;
}

static void answers(int status_1, const char *body_1, int status_2, const char *body_2)
{
	memset(&steam, 0, sizeof(steam));
	steam.statuses[0] = status_1;
	steam.bodies[0] = body_1;
	steam.statuses[1] = status_2;
	steam.bodies[1] = body_2;
}

int main(int argc, char **argv)
{
	const char *test = argc > 1 ? argv[1] : "";
	char creator[24], name[WORKSHOP_AUTHORS_NAME_SIZE];

	if (!strcmp(test, "creator"))
	{
		CHECK(creator_of(details, "2898969630", creator), "no creator read");
		CHECK(!strcmp(creator, "76561198167031068"), "creator %s", creator);
	}
	else if (!strcmp(test, "creator-other-item"))
	{
		/* (the answer's other items' creators are not the asked item's) */
		CHECK(creator_of(details, "111", creator) && !strcmp(creator, "76561198000000001"), "111's: %s", creator);
		CHECK(!creator_of(details, "222", creator) && !creator[0], "an item not in the answer: %s", creator);
	}
	else if (!strcmp(test, "creator-refused"))
	{
		CHECK(!creator_of(not_found, "2898969630", creator) && !creator[0], "a removed item's: %s", creator);
		CHECK(!creator_of("{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"1\",\"creator\":\"7656x\"}]}}",
			"1", creator), "a creator of more than digits");
		CHECK(!creator_of("{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"1\",\"creator\":7656}]}}",
			"1", creator), "a creator that is a number, not Steam's string");
		CHECK(!creator_of("{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"1\",\"creator\":"
			"\"123456789012345678901\"}]}}", "1", creator), "21 digits");
		CHECK(!creator_of("{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"1\",\"creator\":\"7\"", "1",
			creator), "broken JSON");
		CHECK(!creator_of("<html>Unauthorized</html>", "1", creator), "an error page");
		CHECK(!workshop_authors_read_creator(NULL, 0, "1", creator), "no answer");
	}
	else if (!strcmp(test, "name-cdata"))
	{
		CHECK(name_of(profile, name) && !strcmp(name, "aLTis94"), "name '%s'", name);
		CHECK(name_of("<steamID><![CDATA[  a <b> & c  ]]></steamID>", name) && !strcmp(name, "a <b> & c"),
			"CDATA taken as it is, its ends' spaces trimmed: '%s'", name);
	}
	else if (!strcmp(test, "name-escaped"))
	{
		CHECK(name_of("<steamID>Tom &amp; Jerry &lt;3 &quot;x&quot; &apos;y&apos; &copy;</steamID>", name) &&
			!strcmp(name, "Tom & Jerry <3 \"x\" 'y' &copy;"), "escaped name '%s'", name);
	}
	else if (!strcmp(test, "name-controls"))
	{
		/* (Å and the snowman kept; line breaks, tabs, DEL, a lone continuation byte, a cut character and an
		invalid lead left out) */
		CHECK(name_of("<steamID><![CDATA[A\r\n\x7F" "b\tc\xC3\x85\x80\xE2\x98\x83\xE2\x98" "d\xFF" "e]]></steamID>", name),
			"no name");
		CHECK(!strcmp(name, "Abc\xC3\x85\xE2\x98\x83" "de"), "cleaned name '%s'", name);
		CHECK(!name_of("<steamID><![CDATA[\r\n\t]]></steamID>", name) && !name[0], "only controls: '%s'", name);
	}
	else if (!strcmp(test, "name-long"))
	{
		char text[400];
		int index;

		/* (a name too long is cut before a character that does not fit, never in it: two-byte characters, the
		last of which would end past the room) */
		strcpy(text, "<steamID><![CDATA[");
		for (index = 0; index < 60; index++)
			strcat(text, "\xC3\x85");
		strcat(text, "]]></steamID>");
		CHECK(name_of(text, name), "no name");
		CHECK((int)strlen(name) == (WORKSHOP_AUTHORS_NAME_SIZE - 1) / 2 * 2, "%d bytes kept", (int)strlen(name));
		CHECK((unsigned char)name[strlen(name) - 1] == 0x85, "a character cut: ends in %02x",
			(unsigned char)name[strlen(name) - 1]);
	}
	else if (!strcmp(test, "name-missing"))
	{
		CHECK(!name_of("<profile><steamID64>76561198167031068</steamID64></profile>", name), "steamID64 taken");
		CHECK(!name_of("<steamID><![CDATA[aLTis94</steamID>", name), "an unended CDATA");
		CHECK(!name_of("<steamID>aLTis94", name), "an unended name");
		CHECK(!name_of("<steamID><![CDATA[]]></steamID>", name), "an empty name");
		CHECK(!name_of("<response><error><![CDATA[The specified profile could not be found.]]></error></response>",
			name), "an error");
		CHECK(!workshop_authors_read_name(NULL, 0, name), "no answer");
	}
	else if (!strcmp(test, "ask"))
	{
		const char *author;

		answers(200, details, 200, profile);
		author = asked("2898969630");
		CHECK(!strcmp(author, "aLTis94"), "author '%s'", author);
		CHECK(steam.calls == 2, "%d requests", steam.calls);
		CHECK(!strcmp(steam.methods[0], "POST") && !strcmp(steam.urls[0],
			"https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/"), "details: %s %s",
			steam.methods[0], steam.urls[0]);
		CHECK(!strcmp(steam.sent[0], "itemcount=1&publishedfileids%5B0%5D=2898969630"), "details asked: %s",
			steam.sent[0]);
		CHECK(!strcmp(steam.methods[1], "GET") &&
			!strcmp(steam.urls[1], "https://steamcommunity.com/profiles/76561198167031068/?xml=1"), "profile: %s %s",
			steam.methods[1], steam.urls[1]);
		CHECK(test_threads_started == 1, "%d threads", test_threads_started);
		/* (known: not asked again) */
		CHECK(workshop_authors_get("2898969630", name, sizeof(name)) && !strcmp(name, "aLTis94"), "'%s'", name);
		CHECK(!workshop_authors_next() && steam.calls == 2, "asked again");
		CHECK(test_threads_started == 1, "%d threads", test_threads_started);
	}
	else if (!strcmp(test, "retry"))
	{
		answers(200, not_found, 0, NULL);
		CHECK(!asked("2898969630")[0], "an author of a removed item");
		CHECK(steam.calls == 1, "%d requests: a profile asked of no creator", steam.calls);
		/* (not again at once, each frame it is wanted) */
		CHECK(!workshop_authors_get("2898969630", name, sizeof(name)), "known");
		CHECK(!workshop_authors_next(), "asked again at once");
		test_now += 599;
		CHECK(!workshop_authors_get("2898969630", name, sizeof(name)) && !workshop_authors_next(), "asked again early");
		test_now += 1;
		answers(200, details, 0, NULL);
		CHECK(!workshop_authors_get("2898969630", name, sizeof(name)), "known");
		CHECK(workshop_authors_next(), "not asked again after 10 minutes");
		CHECK(steam.calls == 2 && !strcmp(steam.methods[1], "GET"), "the profile not asked: %d", steam.calls);
		CHECK(!workshop_authors_get("2898969630", name, sizeof(name)), "a name of no profile");
	}
	else if (!strcmp(test, "ids-refused"))
	{
		static const char *const ids[] = { "", "12a", "-1", "1 ", "123456789012345678901", NULL };
		int index;

		answers(200, details, 200, profile);
		for (index = 0; index < 6; index++)
		{
			strcpy(name, "untouched");
			CHECK(!workshop_authors_get(ids[index], name, sizeof(name)) && !name[0], "id %d", index);
		}
		CHECK(!workshop_authors_next() && !steam.calls && !test_threads_started, "asked about a bad id");
		CHECK(!workshop_authors_get("12345678901234567890", name, sizeof(name)) && workshop_authors_next(),
			"20 digits are an id");
	}
	else if (!strcmp(test, "full-while-asking"))
	{
		char id[24];
		int index;

		/* (the first item's ask under way when a 65th item is wanted: the next oldest gives way) */
		for (index = 0; index < 64; index++)
		{
			snprintf(id, sizeof(id), "%d", 1000 + index);
			CHECK(!workshop_authors_get(id, name, sizeof(name)), "%s known", id);
		}
		answers(200, "{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"1000\",\"creator\":\"5\"}]}}",
			200, profile);
		steam.wanted_meanwhile = "2000";
		CHECK(workshop_authors_next(), "nothing asked");
		CHECK(workshop_authors_get("1000", name, sizeof(name)) && !strcmp(name, "aLTis94"),
			"the item asked about lost its answer: '%s'", name);
		/* (2000 took 1001's place, and is asked about next) */
		CHECK(!workshop_authors_get("2000", name, sizeof(name)), "2000 known");
		answers(200, "{\"response\":{\"publishedfiledetails\":[{\"publishedfileid\":\"2000\",\"creator\":\"6\"}]}}",
			200, profile);
		CHECK(workshop_authors_next() && strstr(steam.sent[0], "=2000"), "next asked: %s", steam.sent[0]);
	}
	else
	{
		fprintf(stderr, "no case %s\n", test);
		return 2;
	}
	return 0;
}

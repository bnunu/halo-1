/*
WORKSHOP_AUTHORS.C

The Steam Workshop items' authors (workshop_authors.h): Steam's answers read, and the items asked about on a thread
of their own.
*/

#include "workshop_authors.h"
#include "server_json.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* whether text is 1 to maximum decimal digits */
static int workshop_authors_decimal(const char *text, int maximum)
{
	int length;

	for (length = 0; text[length]; length++)
	{
		if (text[length] < '0' || text[length] > '9' || length == maximum)
			return 0;
	}
	return length > 0;
}

/* where needle first is in the length bytes at text, or NULL */
static const char *workshop_authors_find(const char *text, int length, const char *needle)
{
	int size = (int)strlen(needle), at;

	for (at = 0; at + size <= length; at++)
	{
		if (!memcmp(text + at, needle, (size_t)size))
			return text + at;
	}
	return NULL;
}

/* ---------- Steam's answers */

int workshop_authors_read_creator(const char *text, int length, const char *item_id, char creator[24])
{
	struct server_json_document document;
	int details, element, index, found = 0;
	char id[24];

	creator[0] = 0;
	if (!text || length <= 0 || !server_json_parse(&document, text, length, SERVER_JSON_MAXIMUM_DEPTH))
		return 0;
	details = server_json_member(&document, server_json_member(&document, 0, "response"), "publishedfiledetails");
	for (index = 0; (element = server_json_element(&document, details, index)) >= 0; index++)
	{
		if (!server_json_get_string(&document, server_json_member(&document, element, "publishedfileid"), id,
			sizeof(id)) || strcmp(id, item_id))
		{
			continue;
		}
		found = server_json_get_string(&document, server_json_member(&document, element, "creator"), creator, 24) &&
			workshop_authors_decimal(creator, 20);
		break;
	}
	server_json_document_free(&document);
	if (!found)
		creator[0] = 0;
	return found;
}

int workshop_authors_read_name(const char *text, int length, char name[WORKSHOP_AUTHORS_NAME_SIZE])
{
	static const char *const entities[][2] = { { "&amp;", "&" }, { "&lt;", "<" }, { "&gt;", ">" },
		{ "&quot;", "\"" }, { "&apos;", "'" } };
	const char *at, *end;
	int used = 0, plain;

	name[0] = 0;
	if (!text || length <= 0 || !(at = workshop_authors_find(text, length, "<steamID>")))
		return 0;
	at += 9;
	/* (Steam writes it as CDATA; plain text, escaped, is read too) */
	plain = text + length - at < 9 || memcmp(at, "<![CDATA[", 9);
	if (!plain)
		at += 9;
	end = workshop_authors_find(at, (int)(text + length - at), plain ? "</steamID>" : "]]>");
	if (!end)
		return 0;
	while (at < end)
	{
		unsigned char first = (unsigned char)*at;
		int size = first < 0x80 ? 1 : first >= 0xC2 && first <= 0xDF ? 2 : first >= 0xE0 && first <= 0xEF ? 3 :
			first >= 0xF0 && first <= 0xF4 ? 4 : 0;
		int taken = 1, index;
		const char *character = at;

		for (index = 0; plain && first == '&' && index < (int)(sizeof(entities) / sizeof(entities[0])); index++)
		{
			int entity = (int)strlen(entities[index][0]);

			if (end - at >= entity && !memcmp(at, entities[index][0], (size_t)entity))
			{
				character = entities[index][1];
				taken = entity;
				break;
			}
		}
		if (size > 1)
		{
			for (taken = 1; taken < size && at + taken < end && ((unsigned char)at[taken] & 0xC0) == 0x80; taken++)
			{
			}
		}
		at += taken;
		/* (a broken character, and control characters, which would break the texts' lines, are left out) */
		if (!size || taken < size || first < 0x20 || first == 0x7F)
			continue;
		if (used + size >= WORKSHOP_AUTHORS_NAME_SIZE)
			break;
		memcpy(name + used, character, (size_t)size);
		used += size;
	}
	while (used && name[used - 1] == ' ')
		used--;
	name[used] = 0;
	for (at = name; *at == ' '; at++)
	{
	}
	memmove(name, at, strlen(at) + 1);
	return name[0] != 0;
}

/* ---------- asking Steam */

#if defined(HALO_WEB)
int workshop_authors_get(const char *item_id, char *name, int size)
{
	(void)item_id;
	if (size > 0)
		name[0] = 0;
	return 0;
}
#elif defined(HALO_ANDROID)
/* the app's host (port/android/host/steam_downloads.c) */
int host_steam_author(unsigned int item_id, unsigned int name, int name_size);

int workshop_authors_get(const char *item_id, char *name, int size)
{
	return host_steam_author((unsigned int)(uintptr_t)item_id, (unsigned int)(uintptr_t)name, size);
}
#else
#include "update.h"

#include <pthread.h>
#include <time.h>

#if defined(__ANDROID__)
#include "host.h"
#define workshop_authors_log(...) host_logf(HOST_LOG_INFO, __VA_ARGS__)
#else
void platform_log(const char *format, ...);
#define workshop_authors_log platform_log
#endif

enum
{
	MAXIMUM_ITEMS = 64,
	MAXIMUM_ANSWER = 1024 * 1024,
	RETRY_SECONDS = 600,
};

enum
{
	_item_wanted,
	_item_asking,
	_item_known,
	_item_failed,
};

static struct
{
	pthread_mutex_t lock;
	pthread_cond_t wake;
	int started;
	int count;
	/* the item replaced next once all are taken */
	int next;
	struct
	{
		char id[24];
		int state;
		time_t failed_at;
		char name[WORKSHOP_AUTHORS_NAME_SIZE];
	} items[MAXIMUM_ITEMS];
} authors = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };

/* the item's author, asked of Steam: 1 with the name */
static int workshop_authors_ask(const char *item_id, char name[WORKSHOP_AUTHORS_NAME_SIZE])
{
	struct update_response response;
	char body[64], url[96], creator[24], error[256];
	int found;

	snprintf(body, sizeof(body), "itemcount=1&publishedfileids%%5B0%%5D=%s", item_id);
	if (!update_https_request("POST", "https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/",
		"Content-Type: application/x-www-form-urlencoded\r\n", body, (unsigned long)strlen(body), MAXIMUM_ANSWER,
		&response, error, sizeof(error)))
	{
		workshop_authors_log("workshop authors: %s: %s", item_id, error);
		return 0;
	}
	found = response.status == 200 &&
		workshop_authors_read_creator(response.body, (int)response.body_size, item_id, creator);
	free(response.body);
	if (!found)
	{
		workshop_authors_log("workshop authors: %s: Steam names no creator (status %d)", item_id, response.status);
		return 0;
	}
	snprintf(url, sizeof(url), "https://steamcommunity.com/profiles/%s/?xml=1", creator);
	if (!update_https_request("GET", url, NULL, NULL, 0, MAXIMUM_ANSWER, &response, error, sizeof(error)))
	{
		workshop_authors_log("workshop authors: %s: %s", item_id, error);
		return 0;
	}
	found = response.status == 200 && workshop_authors_read_name(response.body, (int)response.body_size, name);
	free(response.body);
	if (found)
		workshop_authors_log("workshop authors: %s is %s's", item_id, name);
	else
		workshop_authors_log("workshop authors: %s: no name on its creator's profile (status %d)", item_id,
			response.status);
	return found;
}

/* (authors.lock held, and let go while Steam is asked) the next wanted item asked about: 0 if none is wanted */
static int workshop_authors_next(void)
{
	char id[24], name[WORKSHOP_AUTHORS_NAME_SIZE];
	int index, known;

	for (index = 0; index < authors.count && authors.items[index].state != _item_wanted; index++)
	{
	}
	if (index == authors.count)
		return 0;
	authors.items[index].state = _item_asking;
	strcpy(id, authors.items[index].id);
	pthread_mutex_unlock(&authors.lock);
	known = workshop_authors_ask(id, name);
	pthread_mutex_lock(&authors.lock);
	/* (an item being asked about is never replaced, so it is still this one) */
	if (known)
	{
		strcpy(authors.items[index].name, name);
		authors.items[index].state = _item_known;
	}
	else
	{
		authors.items[index].state = _item_failed;
		authors.items[index].failed_at = time(NULL);
	}
	return 1;
}

static void *workshop_authors_thread(void *argument)
{
	(void)argument;
	pthread_mutex_lock(&authors.lock);
	for (;;)
	{
		if (!workshop_authors_next())
			pthread_cond_wait(&authors.wake, &authors.lock);
	}
	return NULL;
}

int workshop_authors_get(const char *item_id, char *name, int size)
{
	int index, known = 0;

	if (size > 0)
		name[0] = 0;
	if (!item_id || size < 1 || !workshop_authors_decimal(item_id, 20))
		return 0;
	pthread_mutex_lock(&authors.lock);
	for (index = 0; index < authors.count && strcmp(authors.items[index].id, item_id); index++)
	{
	}
	if (index == authors.count)
	{
		/* (all taken: the oldest given up, unless it is being asked about) */
		if (authors.count < MAXIMUM_ITEMS)
			index = authors.count++;
		else
		{
			if (authors.items[authors.next].state == _item_asking)
				authors.next = (authors.next + 1) % MAXIMUM_ITEMS;
			index = authors.next;
			authors.next = (authors.next + 1) % MAXIMUM_ITEMS;
		}
		memset(&authors.items[index], 0, sizeof(authors.items[index]));
		strcpy(authors.items[index].id, item_id);
		authors.items[index].state = _item_wanted;
		pthread_cond_signal(&authors.wake);
	}
	else if (authors.items[index].state == _item_known)
	{
		snprintf(name, (size_t)size, "%s", authors.items[index].name);
		known = 1;
	}
	else if (authors.items[index].state == _item_failed && time(NULL) - authors.items[index].failed_at >= RETRY_SECONDS)
	{
		authors.items[index].state = _item_wanted;
		pthread_cond_signal(&authors.wake);
	}
	if (!authors.started)
	{
		pthread_t thread;

		/* (once, whether or not it starts: not again each frame) */
		authors.started = 1;
		if (pthread_create(&thread, NULL, workshop_authors_thread, NULL) == 0)
			pthread_detach(thread);
	}
	pthread_mutex_unlock(&authors.lock);
	return known;
}
#endif

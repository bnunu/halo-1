/*
STEAM_DEPOT.C

steam_depot.h's: the content servers, a depot's manifest and its files'
chunks (DepotDownloader's reading of the layouts):

- a manifest: GET /depot/<depot>/manifest/<gid>/5/<request code>, a zip of
  one file holding sections, each its magic, its length and its bytes: the
  payload (0x71F617D0: ContentManifestPayload, the files and their
  chunks), the metadata (0x1F4812BE: whether the names are encrypted),
  the signature, and the end (0x32C415AB). An encrypted name is base64 of
  Steam's symmetric layout under the depot's key;
- a chunk: GET /depot/<depot>/chunk/<sha-1 hex>, the symmetric layout
  around a packed chunk (steam_unpack.c).

A file's chunks are fetched by a few threads at once and written in order
by this one, so a file larger than a 32-bit guest's seek can be written
from start to end; missing ranges (a file of zeros) are written as zeros.
*/

#include "steam_depot.h"
#include "steam_crypto.h"
#include "steam_net.h"
#include "steam_cm.h"

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum
{
	MAXIMUM_SERVERS = 16,
	MAXIMUM_MANIFEST = 64 * 1024 * 1024,
	MAXIMUM_CHUNK = 4 * 1024 * 1024,
	ATTEMPTS = 4,
	/* the wait before the next server after one refuses (401, 403) */
	REFUSED_WAIT_MILLISECONDS = 500,
	/* how long a server that refused is left out of the turns */
	REFUSED_SKIP_MILLISECONDS = 5 * 60 * 1000,
	FETCHERS = 4,
	MANIFEST_PAYLOAD = 0x71F617D0,
	MANIFEST_METADATA = 0x1F4812BE,
	MANIFEST_END = 0x32C415AB,
	FLAG_DIRECTORY = 64,
	/* a manifest's bounds, far past any MCC item's (a package is a few files
	of at most a few hundred megabytes): what lies outside them is refused */
	MAXIMUM_MANIFEST_FILES = 65536,
	MAXIMUM_FILE_CHUNKS = 65536
};

/* (the same, past an enum's int) */
#define MAXIMUM_FILE_BYTES ((uint64_t)16 << 30)
#define MAXIMUM_MANIFEST_BYTES ((uint64_t)64 << 30)

static struct
{
	pthread_mutex_t lock;
	int count;
	char hosts[MAXIMUM_SERVERS][128];
	unsigned next;
	/* when each that refused (401, 403) is taken again (0: it has not) */
	unsigned long refused_until[MAXIMUM_SERVERS];
} servers = { PTHREAD_MUTEX_INITIALIZER, 0, { { 0 } }, 0, { 0 } };

static unsigned long depot_milliseconds(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (unsigned long)now.tv_sec * 1000ul + (unsigned long)(now.tv_nsec / 1000000);
}

/* (servers.lock held) whether the server of an index is left out, having
refused within REFUSED_SKIP_MILLISECONDS */
static int server_skipped(int index, unsigned long now)
{
	return servers.refused_until[index] && (long)(servers.refused_until[index] - now) > 0;
}

/* (servers.lock held) the index of a host, or -1 */
static int server_index(char const *host)
{
	int i;

	for (i = 0; i < servers.count; i++)
		if (!strcmp(servers.hosts[i], host))
			return i;
	return -1;
}

/* ---------- the servers */

int steam_depot_servers(char *error, int error_size)
{
	unsigned char *body;
	unsigned long size;
	int status;
	char *at;
	int count = 0;
	char found[MAXIMUM_SERVERS][128];

	pthread_mutex_lock(&servers.lock);
	count = servers.count;
	pthread_mutex_unlock(&servers.lock);
	if (count)
		return 1;
	if (!steam_https_get("https://api.steampowered.com/IContentServerDirectoryService/GetServersForSteamPipe/v1/?cell_id=0",
		512 * 1024, &body, &size, &status, error, error_size))
		return 0;
	if (status != 200)
	{
		snprintf(error, (size_t)error_size, "Steam's content directory answered %d", status);
		free(body);
		return 0;
	}
	/* each server an object: {"type":"SteamCache"|"CDN", ..., "host", "vhost",
	"https_support", and for some "allowed_app_ids" or "steam_china_only"} */
	for (at = (char *)body; count < MAXIMUM_SERVERS && (at = strchr(at, '{')) != NULL; at++)
	{
		char *end = strchr(at + 1, '}');
		char *field;
		char host[128];
		char saved;

		if (!end)
			break;
		saved = *end;
		*end = 0;
		host[0] = 0;
		if ((strstr(at, "\"type\":\"SteamCache\"") || strstr(at, "\"type\":\"CDN\"")) &&
			!strstr(at, "\"https_support\":\"unavailable\"") && !strstr(at, "\"allowed_app_ids\"") &&
			!strstr(at, "\"steam_china_only\":true") &&
			((field = strstr(at, "\"vhost\":\"")) != NULL || (field = strstr(at, "\"host\":\"")) != NULL))
		{
			char *start = strchr(field + 2, ':') + 2;
			char *quote = strchr(start, '"');
			size_t length = quote ? (size_t)(quote - start) : 0;
			size_t i;

			if (length && length < sizeof(host))
			{
				memcpy(host, start, length);
				host[length] = 0;
				for (i = 0; i < length; i++)
					if (!((host[i] >= 'a' && host[i] <= 'z') || (host[i] >= 'A' && host[i] <= 'Z') ||
						(host[i] >= '0' && host[i] <= '9') || host[i] == '.' || host[i] == '-' || host[i] == ':'))
						host[0] = 0;
			}
		}
		*end = saved;
		if (host[0])
			snprintf(found[count++], sizeof(found[0]), "%s", host);
		at = end;
	}
	free(body);
	if (!count)
	{
		snprintf(error, (size_t)error_size, "Steam named no content server");
		return 0;
	}
	pthread_mutex_lock(&servers.lock);
	memcpy(servers.hosts, found, sizeof(found));
	servers.count = count;
	pthread_mutex_unlock(&servers.lock);
	return 1;
}

static void (*depot_notice)(char const *text);

void steam_depot_set_notice(void (*notice)(char const *text))
{
	depot_notice = notice;
}

/* a depot's path from one of the servers, the next after a failure: over
the kept HTTP connection (Steam's client's way), HTTPS when that fails; over
HTTPS alone when secure (a manifest, whose chunk hashes are what the chunks
are checked against); a server that refuses it (401, 403: some caches serve
only some content) is left for the next, REFUSED_WAIT_MILLISECONDS later,
until each has been asked (404: no server has it) */
static int depot_get(struct steam_http *http, uint32_t depot, char const *path, unsigned long maximum, int secure,
	unsigned char **body, unsigned long *size, char *error, int error_size)
{
	int attempt, refused = 0;

	for (attempt = 0; attempt < ATTEMPTS + refused; attempt++)
	{
		char host[128];
		char full[256];
		char url[512];
		int status = 0;
		unsigned index;
		int got;

		pthread_mutex_lock(&servers.lock);
		{
			unsigned long now = depot_milliseconds();
			int kept = http->host[0] ? server_index(http->host) : -1;
			int tried;

			host[0] = 0;
			/* (a connection stays with its server while it answers) */
			if (http->host[0] && !attempt && (kept < 0 || !server_skipped(kept, now)))
				snprintf(host, sizeof(host), "%s", http->host);
			/* (else the next in turn, past those that refused lately) */
			for (tried = 0; !host[0] && tried < servers.count; tried++)
			{
				index = servers.next++ % (unsigned)servers.count;
				if (!server_skipped((int)index, now))
					snprintf(host, sizeof(host), "%s", servers.hosts[index]);
			}
		}
		pthread_mutex_unlock(&servers.lock);
		if (!host[0])
		{
			snprintf(error, (size_t)error_size, "every content server refused it");
			return 0;
		}
		if (strcmp(http->host, host))
			steam_http_close(http);
		snprintf(full, sizeof(full), "/depot/%u/%s", (unsigned)depot, path);
		got = !secure && steam_http_get(http, host, full, maximum, body, size, &status, error, error_size);
		if (!got)
		{
			snprintf(url, sizeof(url), "https://%s%s", host, full);
			got = steam_https_get(url, maximum, body, size, &status, error, error_size);
		}
		if (got)
		{
			if (status == 200)
				return 1;
			free(*body);
			*body = NULL;
			snprintf(error, (size_t)error_size, "a content server answered %d", status);
			steam_log("steam: %s%s: %d", host, full, status);
			if (status == 404)
				return 0;
			if (status == 401 || status == 403)
			{
				char notice[256];
				struct timespec wait;
				int skipped = 0, left = 0, i;

				/* (left out for REFUSED_SKIP_MILLISECONDS: the files after
				this one go to the others straight away) */
				pthread_mutex_lock(&servers.lock);
				{
					unsigned long now = depot_milliseconds();
					int at = server_index(host);

					if (at >= 0)
						servers.refused_until[at] = (now + REFUSED_SKIP_MILLISECONDS) | 1;
					for (i = 0; i < servers.count; i++)
					{
						if (server_skipped(i, now))
							skipped++;
						else
							left++;
					}
				}
				pthread_mutex_unlock(&servers.lock);
				refused++;
				steam_log("steam: %s left out for %d minutes (%d of %d servers now)", host,
					REFUSED_SKIP_MILLISECONDS / 60000, skipped, skipped + left);
				if (!left)
				{
					snprintf(error, (size_t)error_size, "every content server refused it (%d)", status);
					return 0;
				}
				snprintf(notice, sizeof(notice), "%s refused the download (%d): trying another server...",
					host, status);
				if (depot_notice)
					depot_notice(notice);
				steam_http_close(http);
				wait.tv_sec = REFUSED_WAIT_MILLISECONDS / 1000;
				wait.tv_nsec = (REFUSED_WAIT_MILLISECONDS % 1000) * 1000000L;
				nanosleep(&wait, NULL);
				continue;
			}
		}
		else
			steam_log("steam: %s%s: %s", host, full, error);
		steam_http_close(http);
	}
	return 0;
}

/* ---------- the manifest */

static uint32_t get32(unsigned char const *at)
{
	return (uint32_t)at[0] | (uint32_t)at[1] << 8 | (uint32_t)at[2] << 16 | (uint32_t)at[3] << 24;
}

static int chunk_compare(void const *a, void const *b)
{
	struct steam_depot_chunk const *first = a;
	struct steam_depot_chunk const *second = b;

	return first->offset < second->offset ? -1 : first->offset > second->offset;
}

/* one FileMapping into file */
static int mapping_read(unsigned char const *data, size_t size, struct steam_aes const *aes, int encrypted,
	struct steam_depot_file *file)
{
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *bytes;
	size_t length;
	int capacity = 0;

	memset(file, 0, sizeof(*file));
	reader.at = data;
	reader.end = data + size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &bytes, &length))
	{
		if (field == 1 && bytes)
		{
			if (encrypted)
			{
				unsigned char decoded[1024];
				long decoded_size = steam_base64_decode((char const *)bytes, length, decoded, sizeof(decoded));
				size_t plain;

				if (decoded_size <= 0 || !steam_aes_decrypt(aes, decoded, (size_t)decoded_size, decoded, &plain))
					return 0;
				while (plain && !decoded[plain - 1])
					plain--;
				steam_pb_text(decoded, plain, file->name, sizeof(file->name));
			}
			else
				steam_pb_text(bytes, length, file->name, sizeof(file->name));
		}
		else if (field == 2)
			file->size = value;
		else if (field == 3)
			file->directory = (value & FLAG_DIRECTORY) != 0;
		else if (field == 6 && bytes)
		{
			struct steam_pb_reader inner;
			int inner_field, inner_wire;
			uint64_t inner_value;
			unsigned char const *inner_bytes;
			size_t inner_length;
			struct steam_depot_chunk chunk;
			uint64_t size = 0, packed = 0;
			int have_sha = 0;

			if (file->chunk_count == capacity)
			{
				struct steam_depot_chunk *grown;

				capacity = capacity ? 2 * capacity : 8;
				grown = realloc(file->chunks, (size_t)capacity * sizeof(*grown));
				if (!grown)
					return 0;
				file->chunks = grown;
			}
			memset(&chunk, 0, sizeof(chunk));
			inner.at = bytes;
			inner.end = bytes + length;
			inner.failed = 0;
			while (steam_pb_next(&inner, &inner_field, &inner_wire, &inner_value, &inner_bytes, &inner_length))
			{
				if (inner_field == 1 && inner_length == 20) { memcpy(chunk.sha, inner_bytes, 20); have_sha = 1; }
				else if (inner_field == 3) chunk.offset = inner_value;
				else if (inner_field == 4) size = inner_value;
				else if (inner_field == 5) packed = inner_value;
			}
			/* (a chunk whose record is malformed, or that no download could be:
			the whole list refused, not the file left out) */
			if (inner.failed || !have_sha || !size || size > MAXIMUM_CHUNK || !packed || packed > MAXIMUM_CHUNK ||
				file->chunk_count >= MAXIMUM_FILE_CHUNKS)
				return 0;
			chunk.size = (uint32_t)size;
			chunk.packed = (uint32_t)packed;
			file->chunks[file->chunk_count++] = chunk;
		}
	}
	{
		char *slash;

		for (slash = file->name; *slash; slash++)
			if (*slash == '\\')
				*slash = '/';
	}
	if (reader.failed || !file->name[0] || file->size > MAXIMUM_FILE_BYTES)
		return 0;
	if (file->chunk_count > 1)
		qsort(file->chunks, (size_t)file->chunk_count, sizeof(file->chunks[0]), chunk_compare);
	/* (each chunk inside the file, after the one before it) */
	{
		uint64_t end = 0;
		int i;

		for (i = 0; i < file->chunk_count; i++)
		{
			struct steam_depot_chunk const *chunk = &file->chunks[i];

			if (chunk->offset < end || chunk->size > file->size || chunk->offset > file->size - chunk->size)
				return 0;
			end = chunk->offset + chunk->size;
		}
	}
	return 1;
}

int steam_depot_manifest(uint32_t depot, uint64_t manifest, uint64_t request_code, unsigned char const key[32],
	struct steam_depot_manifest *result, char *error, int error_size)
{
	char path[128];
	unsigned char *zipped;
	unsigned long zipped_size;
	unsigned char *content;
	size_t content_size;
	struct steam_http http;
	int fetched, parsed;

	memset(result, 0, sizeof(*result));
	if (!steam_depot_servers(error, error_size))
		return 0;
	snprintf(path, sizeof(path), "manifest/%llu/5/%llu", (unsigned long long)manifest, (unsigned long long)request_code);
	steam_http_init(&http);
	fetched = depot_get(&http, depot, path, MAXIMUM_MANIFEST, 1, &zipped, &zipped_size, error, error_size);
	steam_http_close(&http);
	if (!fetched)
		return 0;
	steam_log("steam: depot %u manifest %llu: %lu bytes", (unsigned)depot, (unsigned long long)manifest, zipped_size);
	content_size = steam_unzip_one(zipped, zipped_size, &content);
	free(zipped);
	if (!content_size)
	{
		snprintf(error, (size_t)error_size, "the files' list is not readable");
		return 0;
	}
	parsed = steam_depot_manifest_parse(content, content_size, key, result, error, error_size);
	free(content);
	return parsed;
}

int steam_depot_manifest_parse(unsigned char const *content, size_t content_size, unsigned char const key[32],
	struct steam_depot_manifest *result, char *error, int error_size)
{
	unsigned char const *payload = NULL;
	size_t payload_size = 0;
	int encrypted = 0;
	size_t at;
	struct steam_aes aes;
	struct steam_pb_reader reader;
	int field, wire;
	uint64_t value;
	unsigned char const *bytes;
	size_t length;
	int capacity = 0;
	uint64_t total = 0;

	memset(result, 0, sizeof(*result));
	for (at = 0; at + 8 <= content_size;)
	{
		uint32_t magic = get32(content + at);
		uint32_t section = get32(content + at + 4);

		if (magic == MANIFEST_END || section > content_size - at - 8)
			break;
		if (magic == MANIFEST_PAYLOAD)
		{
			payload = content + at + 8;
			payload_size = section;
		}
		else if (magic == MANIFEST_METADATA)
		{
			reader.at = content + at + 8;
			reader.end = reader.at + section;
			reader.failed = 0;
			while (steam_pb_next(&reader, &field, &wire, &value, &bytes, &length))
				if (field == 4)
					encrypted = value != 0;
			if (reader.failed)
			{
				snprintf(error, (size_t)error_size, "the files' list could not be read (malformed)");
				return 0;
			}
		}
		at += 8 + section;
	}
	if (!payload)
	{
		snprintf(error, (size_t)error_size, "the files' list has no files");
		return 0;
	}
	steam_aes_key(&aes, key);
	reader.at = payload;
	reader.end = payload + payload_size;
	reader.failed = 0;
	while (steam_pb_next(&reader, &field, &wire, &value, &bytes, &length))
	{
		if (field != 1 || !bytes)
			continue;
		if (result->file_count >= MAXIMUM_MANIFEST_FILES)
		{
			reader.failed = 1;
			break;
		}
		if (result->file_count == capacity)
		{
			struct steam_depot_file *grown;

			capacity = capacity ? 2 * capacity : 64;
			grown = realloc(result->files, (size_t)capacity * sizeof(*grown));
			if (!grown)
			{
				reader.failed = 1;
				break;
			}
			result->files = grown;
		}
		/* (one malformed file, or past the bounds, and the whole list is
		refused: a list cut short would install part of a package) */
		if (!mapping_read(bytes, length, &aes, encrypted, &result->files[result->file_count]) ||
			result->files[result->file_count].size > MAXIMUM_MANIFEST_BYTES - total)
		{
			free(result->files[result->file_count].chunks);
			reader.failed = 1;
			break;
		}
		total += result->files[result->file_count].size;
		result->file_count++;
	}
	if (reader.failed || !result->file_count)
	{
		steam_depot_manifest_free(result);
		snprintf(error, (size_t)error_size, "the files' list could not be read (malformed, or the key is not "
			"the depot's?)");
		return 0;
	}
	return 1;
}

void steam_depot_manifest_free(struct steam_depot_manifest *manifest)
{
	int i;

	for (i = 0; i < manifest->file_count; i++)
		free(manifest->files[i].chunks);
	free(manifest->files);
	memset(manifest, 0, sizeof(*manifest));
}

static int same_name(char const *a, char const *b)
{
	for (; *a && *b; a++, b++)
	{
		char x = *a >= 'A' && *a <= 'Z' ? (char)(*a + 32) : *a;
		char y = *b >= 'A' && *b <= 'Z' ? (char)(*b + 32) : *b;

		if (x != y)
			return 0;
	}
	return !*a && !*b;
}

struct steam_depot_file const *steam_depot_find(struct steam_depot_manifest const *manifest, char const *name)
{
	int i;

	for (i = 0; i < manifest->file_count; i++)
		if (!manifest->files[i].directory && same_name(manifest->files[i].name, name))
			return &manifest->files[i];
	return NULL;
}

/* ---------- a file's chunks */

struct fetch_slot
{
	/* 0 waiting, 1 fetched, -1 failed */
	int state;
	unsigned char *bytes;
};

struct fetch_job
{
	pthread_mutex_t lock;
	pthread_cond_t changed;
	uint32_t depot;
	struct steam_aes aes;
	struct steam_depot_file const *file;
	/* the next chunk a fetcher takes, and how far the writer is */
	int next;
	int written;
	int stop;
	struct fetch_slot *slots;
	char error[256];
};

/* a chunk fetched, decrypted, unpacked and checked (malloc'd), or NULL */
static unsigned char *chunk_get(struct fetch_job *job, struct steam_http *http, struct steam_depot_chunk const *chunk,
	char *error, int error_size)
{
	char path[64];
	char hex[41];
	unsigned char *body;
	unsigned long size;
	size_t plain;
	unsigned char *out;
	unsigned char digest[20];

	steam_hex(chunk->sha, 20, hex);
	snprintf(path, sizeof(path), "chunk/%s", hex);
	if (!depot_get(http, job->depot, path, MAXIMUM_CHUNK, 0, &body, &size, error, error_size))
		return NULL;
	if (!steam_aes_decrypt(&job->aes, body, size, body, &plain))
	{
		free(body);
		snprintf(error, (size_t)error_size, "a chunk could not be decrypted");
		return NULL;
	}
	out = malloc(chunk->size ? chunk->size : 1);
	if (!out || !steam_unpack_chunk(body, plain, out, chunk->size))
	{
		free(body);
		free(out);
		snprintf(error, (size_t)error_size, "a chunk could not be unpacked");
		return NULL;
	}
	free(body);
	steam_sha1(out, chunk->size, digest);
	if (memcmp(digest, chunk->sha, 20))
	{
		free(out);
		snprintf(error, (size_t)error_size, "a chunk failed its check");
		return NULL;
	}
	return out;
}

static void *fetcher(void *context)
{
	struct fetch_job *job = context;
	struct steam_http http;

	steam_http_init(&http);
	for (;;)
	{
		int index;
		unsigned char *bytes;
		char error[256];

		pthread_mutex_lock(&job->lock);
		/* (at most a few chunks ahead of the writer, to bound the memory) */
		while (!job->stop && job->next < job->file->chunk_count && job->next >= job->written + 2 * FETCHERS)
			pthread_cond_wait(&job->changed, &job->lock);
		if (job->stop || job->next >= job->file->chunk_count)
		{
			pthread_mutex_unlock(&job->lock);
			steam_http_close(&http);
			return NULL;
		}
		index = job->next++;
		pthread_mutex_unlock(&job->lock);
		error[0] = 0;
		bytes = chunk_get(job, &http, &job->file->chunks[index], error, sizeof(error));
		pthread_mutex_lock(&job->lock);
		job->slots[index].bytes = bytes;
		job->slots[index].state = bytes ? 1 : -1;
		if (!bytes && !job->error[0])
			snprintf(job->error, sizeof(job->error), "%s", error);
		pthread_cond_broadcast(&job->changed);
		pthread_mutex_unlock(&job->lock);
	}
}

static int write_zeros(FILE *file, uint64_t count)
{
	static unsigned char const zeros[4096];

	while (count)
	{
		size_t take = count > sizeof(zeros) ? sizeof(zeros) : (size_t)count;

		if (fwrite(zeros, 1, take, file) != take)
			return 0;
		count -= take;
	}
	return 1;
}

int steam_depot_download(uint32_t depot, unsigned char const key[32], struct steam_depot_file const *file,
	char const *path, struct steam_depot_progress const *progress, char *error, int error_size)
{
	struct fetch_job job;
	pthread_t threads[FETCHERS];
	int started = 0;
	char part[1024];
	FILE *out;
	uint64_t at = 0;
	int ok = 1;
	int i;

	if (!steam_depot_servers(error, error_size))
		return 0;
	steam_log("steam: downloading %s (%llu bytes, %d chunks) to %s", file->name, (unsigned long long)file->size,
		file->chunk_count, path);
	snprintf(part, sizeof(part), "%s.part", path);
	out = fopen(part, "wb");
	if (!out)
	{
		snprintf(error, (size_t)error_size, "%s could not be written", path);
		return 0;
	}
	memset(&job, 0, sizeof(job));
	pthread_mutex_init(&job.lock, NULL);
	pthread_cond_init(&job.changed, NULL);
	job.depot = depot;
	job.file = file;
	steam_aes_key(&job.aes, key);
	job.slots = calloc((size_t)(file->chunk_count ? file->chunk_count : 1), sizeof(job.slots[0]));
	if (!job.slots)
		ok = 0;
	for (i = 0; ok && i < FETCHERS && i < file->chunk_count; i++)
		if (pthread_create(&threads[started], NULL, fetcher, &job) == 0)
			started++;
	if (ok && file->chunk_count && !started)
	{
		ok = 0;
		snprintf(job.error, sizeof(job.error), "no thread to download with");
	}
	for (i = 0; ok && i < file->chunk_count; i++)
	{
		struct steam_depot_chunk const *chunk = &file->chunks[i];
		unsigned char *bytes;

		pthread_mutex_lock(&job.lock);
		while (!job.slots[i].state && !job.stop)
		{
			pthread_mutex_unlock(&job.lock);
			if (progress && progress->cancelled && progress->cancelled(progress->context))
			{
				pthread_mutex_lock(&job.lock);
				job.stop = 1;
				snprintf(job.error, sizeof(job.error), "cancelled");
				break;
			}
			pthread_mutex_lock(&job.lock);
			if (!job.slots[i].state)
			{
				struct timespec until;

				/* (looked at again every quarter second, for a cancel) */
				clock_gettime(CLOCK_REALTIME, &until);
				until.tv_nsec += 250000000L;
				if (until.tv_nsec >= 1000000000L)
				{
					until.tv_sec++;
					until.tv_nsec -= 1000000000L;
				}
				pthread_cond_timedwait(&job.changed, &job.lock, &until);
			}
		}
		if (job.stop || job.slots[i].state < 0)
		{
			job.stop = 1;
			pthread_cond_broadcast(&job.changed);
			pthread_mutex_unlock(&job.lock);
			ok = 0;
			break;
		}
		bytes = job.slots[i].bytes;
		job.slots[i].bytes = NULL;
		pthread_mutex_unlock(&job.lock);
		if (chunk->offset < at || !write_zeros(out, chunk->offset - at) ||
			fwrite(bytes, 1, chunk->size, out) != chunk->size)
		{
			free(bytes);
			snprintf(job.error, sizeof(job.error), "%s could not be written (is the disk full?)", path);
			ok = 0;
			pthread_mutex_lock(&job.lock);
			job.stop = 1;
			pthread_cond_broadcast(&job.changed);
			pthread_mutex_unlock(&job.lock);
			break;
		}
		free(bytes);
		at = chunk->offset + chunk->size;
		pthread_mutex_lock(&job.lock);
		job.written = i + 1;
		pthread_cond_broadcast(&job.changed);
		pthread_mutex_unlock(&job.lock);
		if (progress && progress->progress)
			progress->progress(at, progress->context);
	}
	if (ok && at < file->size && !write_zeros(out, file->size - at))
	{
		ok = 0;
		snprintf(job.error, sizeof(job.error), "%s could not be written (is the disk full?)", path);
	}
	pthread_mutex_lock(&job.lock);
	job.stop = 1;
	pthread_cond_broadcast(&job.changed);
	pthread_mutex_unlock(&job.lock);
	for (i = 0; i < started; i++)
		pthread_join(threads[i], NULL);
	for (i = 0; job.slots && i < file->chunk_count; i++)
		free(job.slots[i].bytes);
	free(job.slots);
	pthread_cond_destroy(&job.changed);
	pthread_mutex_destroy(&job.lock);
	if (fclose(out) != 0)
		ok = 0;
	if (ok)
	{
		remove(path);
		if (rename(part, path) != 0)
		{
			ok = 0;
			snprintf(job.error, sizeof(job.error), "%s could not be put in place", path);
		}
	}
	if (ok)
		steam_log("steam: %s done", file->name);
	if (!ok)
	{
		remove(part);
		snprintf(error, (size_t)error_size, "%s", job.error[0] ? job.error : "the download failed");
		steam_log("steam: %s: %s", file->name, error);
	}
	return ok;
}

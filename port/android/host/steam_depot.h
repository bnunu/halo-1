/*
STEAM_DEPOT.H

A depot's files from Steam's content servers (HTTPS, the servers
IContentServerDirectoryService lists): a manifest (a zip of protocol
buffers: the files, their chunks, the names encrypted with the depot's
key), and each file's chunks, AES'd with the key and packed with LZMA
("VZa"), Zstandard ("VSZa") or zip, each checked against its SHA-1 as it
is unpacked (steam_unpack.c), and written in order to the file.

The depot's key and the manifest's request code come from a signed-in
session (steam_cm.h): the account must own what it downloads.
*/

#ifndef STEAM_DEPOT_H
#define STEAM_DEPOT_H

#include <stddef.h>
#include <stdint.h>

struct steam_depot_chunk
{
	unsigned char sha[20];
	uint64_t offset;
	uint32_t size;
	uint32_t packed;
};

struct steam_depot_file
{
	/* its path in the depot, '/' separated, as its maker spelled it */
	char name[260];
	uint64_t size;
	int directory;
	int chunk_count;
	struct steam_depot_chunk *chunks;
};

struct steam_depot_manifest
{
	int file_count;
	struct steam_depot_file *files;
};

/* the content servers' host names (asked once, then kept): 1 when there is one */
int steam_depot_servers(char *error, int error_size);

/* a manifest of the depot, read with its key: 1, or 0 and why in error */
int steam_depot_manifest(uint32_t depot, uint64_t manifest, uint64_t request_code, unsigned char const key[32],
	struct steam_depot_manifest *result, char *error, int error_size);
void steam_depot_manifest_free(struct steam_depot_manifest *manifest);
/* the file of a name (case aside), or NULL */
struct steam_depot_file const *steam_depot_find(struct steam_depot_manifest const *manifest, char const *name);

/* the download of a file: progress(done bytes, context) after each chunk,
and cancelled(context) asked as it goes (both may be NULL) */
struct steam_depot_progress
{
	void (*progress)(uint64_t done, void *context);
	int (*cancelled)(void *context);
	void *context;
};

/* the file's chunks into path (a host path, made anew: written to
path.part, renamed over path when whole): 1, or 0 and why in error */
int steam_depot_download(uint32_t depot, unsigned char const key[32], struct steam_depot_file const *file,
	char const *path, struct steam_depot_progress const *progress, char *error, int error_size);

/* ---------- unpacking (steam_unpack.c) */

/* a decrypted chunk's bytes into out (size bytes exactly): 1 when they
unpack to size bytes */
int steam_unpack_chunk(unsigned char const *data, size_t data_size, unsigned char *out, size_t size);
/* the one file of a zip, inflated (malloc'd): its size, or 0 */
size_t steam_unzip_one(unsigned char const *data, size_t size, unsigned char **out);

#endif

/*
HOST_MCC_HASH.C

An MCC map's content fingerprint (host_mcc_hash.h), worked out in the
Android app's host.

Before a lobby takes an MCC map, the guest validates its package
(port/linux/game/mcc_validation.inl): the map's tags against its resources,
then a SHA-256 of the map and the resources it uses, and a CRC32 of the
resources (mcc_resource_check.inl). The guest did that hashing itself, 16 KB
at a time on a worker thread: a package whose bitmaps.map is 310 MB took a
phone 30 to 60 seconds, the lobby showing its stock map the while.

Here it is the host's, at the speed of the card: the CPU's SHA-256 and
CRC32 instructions when it has them (ARMv8's), on
two threads, one reading the file and doing the CRC while the other does
the SHA-256 of what was read. What it works out is kept on the card
(mcc_fingerprints.txt of the data root), by each file's path, size and time,
so the lobby asks again for nothing it already knows; and a package the
Steam Workshop screen installs (steam_downloads.c) is worked out as it is
installed: each of its maps with its package's resources, in each of the
ways the guest may ask (with neither, either or both of bitmaps.map and
sounds.map, as the map's tags use them), each resource read once for every
map. A map put into mcc_maps by hand is worked out when it is chosen.

What is worked out is the guest's to the byte (mcc_resource_check.inl):
  SHA-256 of "GulchCE MCC content v1", then for the map (type 0) and each
  resource used (1 bitmaps, 2 sounds): its type, its size as 8 bytes (least
  significant first) and its bytes;
  CRC32 (zlib's) from the map header's checksum, over each resource used:
  its type as a byte, its bytes, then 0xFF.
The CRC's resource part is kept from 0 with its length, and joined to the
map's checksum when asked (crc32_combine's way), so one entry serves each.
*/

#ifndef HOST_MCC_HASH_TEST
#include "host.h"
#endif
#include "host_mcc_hash.h"

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

#if defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#if defined(__ANDROID__)
#include <sys/auxv.h>
#endif
#endif

#ifndef HOST_MCC_HASH_TEST
#define hash_log(...) host_logf(HOST_LOG_INFO, __VA_ARGS__)
#else
#define hash_log(...) fprintf(stderr, __VA_ARGS__)
#endif

static unsigned long hash_milliseconds(void)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	return (unsigned long)now.tv_sec * 1000ul + (unsigned long)(now.tv_nsec / 1000000);
}

#define FINGERPRINT_PREFIX "GulchCE MCC content v1"
#define CHUNK_SIZE (512 * 1024)
#define RING 4

/* ---------- SHA-256 */

static const uint32_t sha_constants[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

struct sha {
	uint32_t state[8];
	unsigned char block[64];
	uint64_t length;
	unsigned used;
};

#define ROTATE(value, count) (((value) >> (count)) | ((value) << (32 - (count))))

static void sha_blocks_portable(uint32_t state[8], unsigned char const *data, size_t blocks)
{
	while (blocks--) {
		uint32_t w[64], a, b, c, d, e, f, g, h;
		int i;

		for (i = 0; i < 16; i++)
			w[i] = (uint32_t)data[i * 4] << 24 | (uint32_t)data[i * 4 + 1] << 16 | (uint32_t)data[i * 4 + 2] << 8 |
				data[i * 4 + 3];
		for (i = 16; i < 64; i++) {
			uint32_t s0 = ROTATE(w[i - 15], 7) ^ ROTATE(w[i - 15], 18) ^ (w[i - 15] >> 3);
			uint32_t s1 = ROTATE(w[i - 2], 17) ^ ROTATE(w[i - 2], 19) ^ (w[i - 2] >> 10);

			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}
		a = state[0]; b = state[1]; c = state[2]; d = state[3];
		e = state[4]; f = state[5]; g = state[6]; h = state[7];
		for (i = 0; i < 64; i++) {
			uint32_t t1 = h + (ROTATE(e, 6) ^ ROTATE(e, 11) ^ ROTATE(e, 25)) + ((e & f) ^ (~e & g)) + sha_constants[i] + w[i];
			uint32_t t2 = (ROTATE(a, 2) ^ ROTATE(a, 13) ^ ROTATE(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));

			h = g; g = f; f = e; e = d + t1;
			d = c; c = b; b = a; a = t1 + t2;
		}
		state[0] += a; state[1] += b; state[2] += c; state[3] += d;
		state[4] += e; state[5] += f; state[6] += g; state[7] += h;
		data += 64;
	}
}

#if defined(__aarch64__)
/* ARMv8's SHA-256 instructions (FEAT_SHA256) */
__attribute__((target("+sha2"), unused))
static void sha_blocks_arm(uint32_t state[8], unsigned char const *data, size_t blocks)
{
	uint32x4_t abcd = vld1q_u32(state), efgh = vld1q_u32(state + 4);

	while (blocks--) {
		uint32x4_t m0, m1, m2, m3, t, abcd_saved = abcd, efgh_saved = efgh, previous;

		m0 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data)));
		m1 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 16)));
		m2 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 32)));
		m3 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 48)));
#define ROUNDS4(m, k) \
		t = vaddq_u32(m, vld1q_u32(sha_constants + (k))); \
		previous = abcd; \
		abcd = vsha256hq_u32(abcd, efgh, t); \
		efgh = vsha256h2q_u32(efgh, previous, t)
#define SCHEDULE(a, b, c, d) a = vsha256su1q_u32(vsha256su0q_u32(a, b), c, d)
		ROUNDS4(m0, 0); SCHEDULE(m0, m1, m2, m3);
		ROUNDS4(m1, 4); SCHEDULE(m1, m2, m3, m0);
		ROUNDS4(m2, 8); SCHEDULE(m2, m3, m0, m1);
		ROUNDS4(m3, 12); SCHEDULE(m3, m0, m1, m2);
		ROUNDS4(m0, 16); SCHEDULE(m0, m1, m2, m3);
		ROUNDS4(m1, 20); SCHEDULE(m1, m2, m3, m0);
		ROUNDS4(m2, 24); SCHEDULE(m2, m3, m0, m1);
		ROUNDS4(m3, 28); SCHEDULE(m3, m0, m1, m2);
		ROUNDS4(m0, 32); SCHEDULE(m0, m1, m2, m3);
		ROUNDS4(m1, 36); SCHEDULE(m1, m2, m3, m0);
		ROUNDS4(m2, 40); SCHEDULE(m2, m3, m0, m1);
		ROUNDS4(m3, 44); SCHEDULE(m3, m0, m1, m2);
		ROUNDS4(m0, 48);
		ROUNDS4(m1, 52);
		ROUNDS4(m2, 56);
		ROUNDS4(m3, 60);
#undef ROUNDS4
#undef SCHEDULE
		abcd = vaddq_u32(abcd, abcd_saved);
		efgh = vaddq_u32(efgh, efgh_saved);
		data += 64;
	}
	vst1q_u32(state, abcd);
	vst1q_u32(state + 4, efgh);
}
#endif

static void (*sha_blocks)(uint32_t state[8], unsigned char const *data, size_t blocks) = sha_blocks_portable;

/* ---------- CRC32 (zlib's: reflected 0xEDB88320) */

static uint32_t crc_table[8][256];

static uint32_t crc_portable(uint32_t crc, unsigned char const *data, size_t size)
{
	crc = ~crc;
	while (size && ((uintptr_t)data & 7)) {
		crc = crc_table[0][(crc ^ *data++) & 0xFF] ^ (crc >> 8);
		size--;
	}
	while (size >= 8) {
		uint32_t low = crc ^ ((uint32_t)data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 | (uint32_t)data[3] << 24);
		uint32_t high = (uint32_t)data[4] | (uint32_t)data[5] << 8 | (uint32_t)data[6] << 16 | (uint32_t)data[7] << 24;

		crc = crc_table[7][low & 0xFF] ^ crc_table[6][(low >> 8) & 0xFF] ^ crc_table[5][(low >> 16) & 0xFF] ^
			crc_table[4][low >> 24] ^ crc_table[3][high & 0xFF] ^ crc_table[2][(high >> 8) & 0xFF] ^
			crc_table[1][(high >> 16) & 0xFF] ^ crc_table[0][high >> 24];
		data += 8;
		size -= 8;
	}
	while (size--)
		crc = crc_table[0][(crc ^ *data++) & 0xFF] ^ (crc >> 8);
	return ~crc;
}

#if defined(__aarch64__)
/* ARMv8's CRC32 instructions (the same polynomial as zlib's) */
__attribute__((target("+crc"), unused))
static uint32_t crc_arm(uint32_t crc, unsigned char const *data, size_t size)
{
	crc = ~crc;
	while (size && ((uintptr_t)data & 7)) {
		crc = __crc32b(crc, *data++);
		size--;
	}
	while (size >= 8) {
		uint64_t word;

		memcpy(&word, data, 8);
		crc = __crc32d(crc, word);
		data += 8;
		size -= 8;
	}
	while (size--)
		crc = __crc32b(crc, *data++);
	return ~crc;
}
#endif

static uint32_t (*crc_update)(uint32_t crc, unsigned char const *data, size_t size) = crc_portable;

/* zlib's crc32_combine: the CRC of A then B from A's, B's (from 0) and B's length */
static uint32_t gf2_times(uint32_t const *matrix, uint32_t vector)
{
	uint32_t sum = 0;

	while (vector) {
		if (vector & 1)
			sum ^= *matrix;
		vector >>= 1;
		matrix++;
	}
	return sum;
}

static void gf2_square(uint32_t *square, uint32_t const *matrix)
{
	int n;

	for (n = 0; n < 32; n++)
		square[n] = gf2_times(matrix, matrix[n]);
}

static uint32_t crc_combine(uint32_t crc1, uint32_t crc2, uint64_t length2)
{
	uint32_t even[32], odd[32], row = 1;
	int n;

	if (!length2)
		return crc1;
	odd[0] = 0xEDB88320u;
	for (n = 1; n < 32; n++) {
		odd[n] = row;
		row <<= 1;
	}
	gf2_square(even, odd);
	gf2_square(odd, even);
	do {
		gf2_square(even, odd);
		if (length2 & 1)
			crc1 = gf2_times(even, crc1);
		length2 >>= 1;
		if (!length2)
			break;
		gf2_square(odd, even);
		if (length2 & 1)
			crc1 = gf2_times(odd, crc1);
		length2 >>= 1;
	} while (length2);
	return crc1 ^ crc2;
}

static pthread_once_t hash_once = PTHREAD_ONCE_INIT;
static char const *sha_kind = "portable", *crc_kind = "portable";

static void hash_setup(void)
{
	uint32_t n, k, c;

	for (n = 0; n < 256; n++) {
		c = n;
		for (k = 0; k < 8; k++)
			c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
		crc_table[0][n] = c;
	}
	for (n = 0; n < 256; n++)
		for (k = 1; k < 8; k++)
			crc_table[k][n] = crc_table[0][crc_table[k - 1][n] & 0xFF] ^ (crc_table[k - 1][n] >> 8);
#if defined(HOST_MCC_HASH_PORTABLE)
	return;
#elif defined(__aarch64__)
	{
		int sha2 = 1, crc32 = 1;
#if defined(__ANDROID__) || defined(__linux__)
		unsigned long hwcap = getauxval(AT_HWCAP);

		/* (HWCAP_SHA2, HWCAP_CRC32) */
		sha2 = (hwcap & (1ul << 6)) != 0;
		crc32 = (hwcap & (1ul << 7)) != 0;
#endif
		if (sha2) {
			sha_blocks = sha_blocks_arm;
			sha_kind = "ARMv8 SHA-256";
		}
		if (crc32) {
			crc_update = crc_arm;
			crc_kind = "ARMv8 CRC32";
		}
	}
#endif
}

static void sha_begin(struct sha *sha)
{
	static const uint32_t initial[8] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
	};

	memcpy(sha->state, initial, sizeof(initial));
	sha->length = 0;
	sha->used = 0;
}

static void sha_add(struct sha *sha, void const *data, size_t size)
{
	unsigned char const *bytes = data;

	sha->length += size;
	if (sha->used) {
		size_t count = 64 - sha->used < size ? 64 - sha->used : size;

		memcpy(sha->block + sha->used, bytes, count);
		sha->used += (unsigned)count;
		bytes += count;
		size -= count;
		if (sha->used < 64)
			return;
		sha_blocks(sha->state, sha->block, 1);
		sha->used = 0;
	}
	if (size >= 64) {
		sha_blocks(sha->state, bytes, size / 64);
		bytes += size & ~(size_t)63;
		size &= 63;
	}
	memcpy(sha->block, bytes, size);
	sha->used = (unsigned)size;
}

static void sha_end(struct sha const *source, unsigned char digest[32])
{
	struct sha sha = *source;
	uint64_t bits = sha.length * 8;
	unsigned char tail[72];
	size_t padding = sha.used < 56 ? 56 - sha.used : 120 - sha.used;
	int i;

	memset(tail, 0, sizeof(tail));
	tail[0] = 0x80;
	for (i = 0; i < 8; i++)
		tail[padding + i] = (unsigned char)(bits >> (56 - i * 8));
	sha_add(&sha, tail, padding + 8);
	for (i = 0; i < 8; i++) {
		digest[i * 4] = (unsigned char)(sha.state[i] >> 24);
		digest[i * 4 + 1] = (unsigned char)(sha.state[i] >> 16);
		digest[i * 4 + 2] = (unsigned char)(sha.state[i] >> 8);
		digest[i * 4 + 3] = (unsigned char)sha.state[i];
	}
}

/* the fingerprint's header of a file: its type and its size */
static void sha_file_header(struct sha *sha, unsigned type, uint64_t size)
{
	unsigned char header[9];
	int i;

	header[0] = (unsigned char)type;
	for (i = 0; i < 8; i++)
		header[i + 1] = (unsigned char)(size >> (i * 8));
	sha_add(sha, header, sizeof(header));
}

/* ---------- a file read once into several SHA-256s and a CRC, on two threads:
this one reads, and does the CRC and half the SHA-256s; a helper the other
half (the first, and the only one when there is one) */

struct pass {
	FILE *file;
	struct sha **shas;
	int count, split;
	uint32_t *crc;
	volatile int const *cancel;
	void (*progress)(unsigned long long read, void *context);
	void *progress_context;

	pthread_mutex_t lock;
	pthread_cond_t changed;
	unsigned char *ring[RING];
	size_t sizes[RING];
	unsigned long read_count, mine, helper;
	int finished, failed, helper_running;
};

static void pass_helper_share(struct pass *pass, unsigned long chunk)
{
	int i;

	for (i = 0; i < pass->split; i++)
		sha_add(pass->shas[i], pass->ring[chunk % RING], pass->sizes[chunk % RING]);
}

static void pass_own_share(struct pass *pass, unsigned long chunk)
{
	int i;

	for (i = pass->split; i < pass->count; i++)
		sha_add(pass->shas[i], pass->ring[chunk % RING], pass->sizes[chunk % RING]);
	if (pass->crc)
		*pass->crc = crc_update(*pass->crc, pass->ring[chunk % RING], pass->sizes[chunk % RING]);
	/* (and the helper's too, when there is none) */
	if (!pass->helper_running)
		pass_helper_share(pass, chunk);
}

static void *pass_helper(void *parameter)
{
	struct pass *pass = parameter;

	pthread_mutex_lock(&pass->lock);
	for (;;) {
		unsigned long chunk;

		while (pass->helper == pass->read_count && !pass->finished && !pass->failed)
			pthread_cond_wait(&pass->changed, &pass->lock);
		if (pass->failed || pass->helper == pass->read_count)
			break;
		chunk = pass->helper;
		pthread_mutex_unlock(&pass->lock);
		pass_helper_share(pass, chunk);
		pthread_mutex_lock(&pass->lock);
		pass->helper++;
		pthread_cond_broadcast(&pass->changed);
	}
	pthread_mutex_unlock(&pass->lock);
	return NULL;
}

/* the open file read to its end into the SHA-256s and the CRC: 1, or 0 if it
could not be read or was cancelled */
static int pass_run(struct pass *pass)
{
	pthread_t helper;
	unsigned long long total = 0;
	int i, ok;

	pthread_mutex_init(&pass->lock, NULL);
	pthread_cond_init(&pass->changed, NULL);
	pass->read_count = pass->mine = pass->helper = 0;
	pass->finished = pass->failed = 0;
	for (i = 0; i < RING; i++)
		if (!(pass->ring[i] = malloc(CHUNK_SIZE)))
			pass->failed = 1;
	pass->split = pass->count > 0 ? (pass->count + 1) / 2 : 0;
	pass->helper_running = !pass->failed && pass->split > 0 && pthread_create(&helper, NULL, pass_helper, pass) == 0;
	pthread_mutex_lock(&pass->lock);
	while (!pass->failed) {
		unsigned long oldest = pass->helper_running && pass->helper < pass->mine ? pass->helper : pass->mine;

		if (pass->cancel && *pass->cancel) {
			pass->failed = 1;
			break;
		}
		if (!pass->finished && pass->read_count - oldest < RING) {
			unsigned long chunk = pass->read_count;
			size_t got;

			pthread_mutex_unlock(&pass->lock);
			got = fread(pass->ring[chunk % RING], 1, CHUNK_SIZE, pass->file);
			pthread_mutex_lock(&pass->lock);
			if (got < CHUNK_SIZE && ferror(pass->file))
				pass->failed = 1;
			else if (!got)
				pass->finished = 1;
			else {
				pass->sizes[chunk % RING] = got;
				pass->read_count++;
				total += got;
				if (got < CHUNK_SIZE)
					pass->finished = 1;
				if (pass->progress) {
					pthread_mutex_unlock(&pass->lock);
					pass->progress(total, pass->progress_context);
					pthread_mutex_lock(&pass->lock);
				}
			}
			pthread_cond_broadcast(&pass->changed);
			continue;
		}
		if (pass->mine < pass->read_count) {
			unsigned long chunk = pass->mine;

			pthread_mutex_unlock(&pass->lock);
			pass_own_share(pass, chunk);
			pthread_mutex_lock(&pass->lock);
			pass->mine++;
			pthread_cond_broadcast(&pass->changed);
			continue;
		}
		if (pass->finished && (!pass->helper_running || pass->helper == pass->read_count))
			break;
		pthread_cond_wait(&pass->changed, &pass->lock);
	}
	pthread_cond_broadcast(&pass->changed);
	ok = !pass->failed;
	pthread_mutex_unlock(&pass->lock);
	if (pass->helper_running)
		pthread_join(helper, NULL);
	for (i = 0; i < RING; i++)
		free(pass->ring[i]);
	pthread_cond_destroy(&pass->changed);
	pthread_mutex_destroy(&pass->lock);
	return ok;
}

/* a file by its path read into the SHA-256s (after their header for it) and
the CRC (after its type marker, and its 0xFF after): its size, or -1 */
static long long hash_file(char const *path, unsigned type, struct sha **shas, int count, uint32_t *crc,
	volatile int const *cancel, void (*progress)(unsigned long long, void *), void *progress_context)
{
	struct pass pass;
	struct stat information;
	unsigned char marker;
	int i, ok;

	memset(&pass, 0, sizeof(pass));
	if (stat(path, &information) || !(pass.file = fopen(path, "rb")))
		return -1;
	for (i = 0; i < count; i++)
		sha_file_header(shas[i], type, (uint64_t)information.st_size);
	if (crc) {
		marker = (unsigned char)type;
		*crc = crc_update(*crc, &marker, 1);
	}
	pass.shas = shas;
	pass.count = count;
	pass.crc = crc;
	pass.cancel = cancel;
	pass.progress = progress;
	pass.progress_context = progress_context;
	ok = pass_run(&pass);
	fclose(pass.file);
	if (!ok)
		return -1;
	if (crc) {
		marker = 0xFF;
		*crc = crc_update(*crc, &marker, 1);
	}
	return (long long)information.st_size;
}

/* ---------- what is known, kept on the card */

#define CACHE_FILE "mcc_fingerprints.txt"
#define CACHE_HEADER "mcc fingerprints 1\n"
#define CACHE_LIMIT 2048
#define KEY_SIZE 1100

struct known {
	/* each file's lower-cased path, size and time, "|" between */
	char key[KEY_SIZE];
	unsigned char digest[32];
	/* the resources' CRC from 0, and its length */
	uint32_t crc;
	uint64_t length;
};

static pthread_mutex_t known_lock = PTHREAD_MUTEX_INITIALIZER;
static struct known *known;
static int known_count, known_read;
static char known_path[1024];

/* a part of a key: the file's path (lower case), size and time; 0 if it
cannot be looked at */
static int key_append(char *key, size_t size, char const *path)
{
	struct stat information;
	size_t used = strlen(key);
	char lowered[1024];
	size_t i;

	if (stat(path, &information) || strlen(path) >= sizeof(lowered))
		return 0;
	for (i = 0; path[i]; i++)
		lowered[i] = (char)(path[i] >= 'A' && path[i] <= 'Z' ? path[i] + 32 : path[i]);
	lowered[i] = 0;
	return snprintf(key + used, size - used, "%s%s,%llu,%lld", used ? "|" : "", lowered,
		(unsigned long long)information.st_size, (long long)information.st_mtime) < (int)(size - used);
}

/* (known_lock held) */
static void known_load(char const *data_root)
{
	FILE *file;
	char line[KEY_SIZE + 128];

	if (known_read || !data_root || !data_root[0])
		return;
	known_read = 1;
	snprintf(known_path, sizeof(known_path), "%s/%s", data_root, CACHE_FILE);
	known = calloc(CACHE_LIMIT, sizeof(*known));
	if (!known || !(file = fopen(known_path, "r")))
		return;
	if (!fgets(line, sizeof(line), file) || strcmp(line, CACHE_HEADER)) {
		fclose(file);
		return;
	}
	while (known_count < CACHE_LIMIT && fgets(line, sizeof(line), file)) {
		struct known *entry = &known[known_count];
		char digest[65];
		unsigned long crc;
		unsigned long long length;
		int used = 0, i;
		size_t key_length;

		if (sscanf(line, "%64s %lx %llu %n", digest, &crc, &length, &used) != 3 || !used || strlen(digest) != 64)
			continue;
		key_length = strcspn(line + used, "\r\n");
		/* (a line cut short by an interrupted write is not trusted) */
		if (!key_length || key_length >= sizeof(entry->key) || line[used + key_length] != '\n')
			continue;
		for (i = 0; i < 32; i++) {
			unsigned value;

			if (sscanf(digest + i * 2, "%2x", &value) != 1)
				break;
			entry->digest[i] = (unsigned char)value;
		}
		if (i < 32)
			continue;
		memcpy(entry->key, line + used, key_length);
		entry->key[key_length] = 0;
		entry->crc = (uint32_t)crc;
		entry->length = length;
		known_count++;
	}
	fclose(file);
}

/* (known_lock held) */
static void known_save(void)
{
	char temporary[1100];
	FILE *file;
	int index, i;

	if (!known_path[0] || snprintf(temporary, sizeof(temporary), "%s.part", known_path) >= (int)sizeof(temporary) ||
		!(file = fopen(temporary, "w")))
		return;
	fputs(CACHE_HEADER, file);
	for (index = 0; index < known_count; index++) {
		for (i = 0; i < 32; i++)
			fprintf(file, "%02x", known[index].digest[i]);
		fprintf(file, " %lx %llu %s\n", (unsigned long)known[index].crc, (unsigned long long)known[index].length,
			known[index].key);
	}
	if (fclose(file) == 0) {
		remove(known_path);
		rename(temporary, known_path);
	}
}

static int known_find(char const *data_root, char const *key, struct known *found)
{
	int index, hit = 0;

	pthread_mutex_lock(&known_lock);
	known_load(data_root);
	for (index = 0; known && index < known_count && !hit; index++) {
		if (!strcmp(known[index].key, key)) {
			*found = known[index];
			hit = 1;
		}
	}
	pthread_mutex_unlock(&known_lock);
	return hit;
}

/* entries kept: the ones of the same keys replaced; saved once */
static void known_store(char const *data_root, struct known const *entries, int count)
{
	int n, index;

	pthread_mutex_lock(&known_lock);
	known_load(data_root);
	for (n = 0; known && n < count; n++) {
		for (index = 0; index < known_count && strcmp(known[index].key, entries[n].key); index++) {
		}
		if (index == known_count) {
			/* (full: the oldest goes) */
			if (known_count == CACHE_LIMIT) {
				memmove(&known[0], &known[1], (CACHE_LIMIT - 1) * sizeof(*known));
				known_count--;
				index--;
			}
			index = known_count++;
		}
		known[index] = entries[n];
	}
	if (known)
		known_save();
	pthread_mutex_unlock(&known_lock);
}

/* ---------- a package's maps, as the Steam Workshop screen installs it */

static int is_resource_name(char const *name)
{
	return !strcasecmp(name, "bitmaps.map") || !strcasecmp(name, "sounds.map") || !strcasecmp(name, "loc.map");
}

struct package_progress {
	void (*report)(unsigned long long, unsigned long long, void *);
	void *context;
	unsigned long long before, total;
};

static void package_progressed(unsigned long long done, void *context)
{
	struct package_progress *progress = context;

	if (progress->report)
		progress->report(progress->before + done, progress->total, progress->context);
}

int host_mcc_hash_package(char const *data_root, char const *folder, volatile int const *cancel,
	void (*report)(unsigned long long, unsigned long long, void *), void *context)
{
	enum { MAPS = 64 };
	char (*maps)[1024] = NULL;
	char bitmaps[1024], sounds[1024];
	struct sha *base = NULL, *with_bitmaps = NULL, *with_sounds = NULL, *with_both = NULL, **shas = NULL;
	struct known *entries = NULL;
	struct package_progress progress;
	struct stat information;
	int count = 0, have_bitmaps, have_sounds, i, entry_count = 0, done = 0;
	uint32_t crc_bitmaps = 0, crc_sounds = 0;
	uint64_t length_bitmaps = 0, length_sounds = 0;
	unsigned long began;
	DIR *directory;
	struct dirent *found;

	pthread_once(&hash_once, hash_setup);
	if (!folder || !(directory = opendir(folder)))
		return -1;
	maps = calloc(MAPS, sizeof(*maps));
	if (!maps) {
		closedir(directory);
		return -1;
	}
	memset(&progress, 0, sizeof(progress));
	progress.report = report;
	progress.context = context;
	while (count < MAPS && (found = readdir(directory)) != NULL) {
		char const *name = found->d_name;
		size_t length = strlen(name);

		if (length > 4 && !strcasecmp(name + length - 4, ".map") && !is_resource_name(name) &&
			snprintf(maps[count], sizeof(maps[count]), "%s/%s", folder, name) < (int)sizeof(maps[count]) &&
			!stat(maps[count], &information) && !S_ISDIR(information.st_mode)) {
			progress.total += (unsigned long long)information.st_size;
			count++;
		}
	}
	closedir(directory);
	snprintf(bitmaps, sizeof(bitmaps), "%s/bitmaps.map", folder);
	snprintf(sounds, sizeof(sounds), "%s/sounds.map", folder);
	have_bitmaps = !stat(bitmaps, &information) && !S_ISDIR(information.st_mode);
	if (have_bitmaps)
		progress.total += (unsigned long long)information.st_size;
	have_sounds = !stat(sounds, &information) && !S_ISDIR(information.st_mode);
	if (have_sounds)
		progress.total += (unsigned long long)information.st_size;
	if (!count)
		goto finished;
	began = hash_milliseconds();
	base = calloc((size_t)count, sizeof(*base));
	with_bitmaps = calloc((size_t)count, sizeof(*with_bitmaps));
	with_sounds = calloc((size_t)count, sizeof(*with_sounds));
	with_both = calloc((size_t)count, sizeof(*with_both));
	shas = calloc((size_t)count * 2, sizeof(*shas));
	entries = calloc((size_t)count * 4, sizeof(*entries));
	if (!base || !with_bitmaps || !with_sounds || !with_both || !shas || !entries)
		goto finished;
	/* each map: the fingerprint up to its end */
	for (i = 0; i < count; i++) {
		long long size;

		sha_begin(&base[i]);
		sha_add(&base[i], FINGERPRINT_PREFIX, sizeof(FINGERPRINT_PREFIX) - 1);
		shas[0] = &base[i];
		size = hash_file(maps[i], 0, shas, 1, NULL, cancel, package_progressed, &progress);
		if (size < 0)
			goto finished;
		progress.before += (unsigned long long)size;
	}
	/* bitmaps.map once, into each map's */
	if (have_bitmaps) {
		long long size;

		for (i = 0; i < count; i++) {
			with_bitmaps[i] = base[i];
			shas[i] = &with_bitmaps[i];
		}
		size = hash_file(bitmaps, 1, shas, count, &crc_bitmaps, cancel, package_progressed, &progress);
		if (size < 0)
			goto finished;
		length_bitmaps = (uint64_t)size + 2;
		progress.before += (unsigned long long)size;
	}
	/* sounds.map once, into each map's alone and after its bitmaps.map */
	if (have_sounds) {
		long long size;
		int n = 0;

		for (i = 0; i < count; i++) {
			with_sounds[i] = base[i];
			shas[n++] = &with_sounds[i];
			if (have_bitmaps) {
				with_both[i] = with_bitmaps[i];
				shas[n++] = &with_both[i];
			}
		}
		size = hash_file(sounds, 2, shas, n, &crc_sounds, cancel, package_progressed, &progress);
		if (size < 0)
			goto finished;
		length_sounds = (uint64_t)size + 2;
		progress.before += (unsigned long long)size;
	}
	/* each map's four ways, kept */
	for (i = 0; i < count; i++) {
		int way;

		for (way = 0; way < 4; way++) {
			struct known *entry = &entries[entry_count];
			int uses_bitmaps = way & 1, uses_sounds = (way & 2) != 0;

			if ((uses_bitmaps && !have_bitmaps) || (uses_sounds && !have_sounds))
				continue;
			memset(entry, 0, sizeof(*entry));
			if (!key_append(entry->key, sizeof(entry->key), maps[i]) ||
				(uses_bitmaps && !key_append(entry->key, sizeof(entry->key), bitmaps)) ||
				(uses_sounds && !key_append(entry->key, sizeof(entry->key), sounds)))
				continue;
			sha_end(uses_bitmaps && uses_sounds ? &with_both[i] : uses_bitmaps ? &with_bitmaps[i] :
				uses_sounds ? &with_sounds[i] : &base[i], entry->digest);
			if (uses_bitmaps && uses_sounds) {
				entry->crc = crc_combine(crc_bitmaps, crc_sounds, length_sounds);
				entry->length = length_bitmaps + length_sounds;
			} else if (uses_bitmaps) {
				entry->crc = crc_bitmaps;
				entry->length = length_bitmaps;
			} else if (uses_sounds) {
				entry->crc = crc_sounds;
				entry->length = length_sounds;
			}
			entry_count++;
		}
	}
	known_store(data_root, entries, entry_count);
	done = count;
	hash_log("mcc fingerprints: %d maps of %s in %lu ms (%llu MB read; %s, %s, 2 threads)", count, folder,
		hash_milliseconds() - began, progress.total >> 20, sha_kind, crc_kind);
finished:
	if (!done && count)
		hash_log("mcc fingerprints: %s was not finished%s", folder, cancel && *cancel ? " (cancelled)" : "");
	free(maps);
	free(base);
	free(with_bitmaps);
	free(with_sounds);
	free(with_both);
	free(shas);
	free(entries);
	return done;
}

/* ---------- the guest's: one map's, in the lobby */

/* (the guest's request: its addresses are the host's, 32-bit) */
struct guest_request {
	uint32_t data_root;
	/* the map, bitmaps.map and sounds.map (0: not used), absolute */
	uint32_t paths[3];
	uint32_t crc;
};

struct guest_result {
	uint32_t crc;
	unsigned char digest[32];
};

enum { JOBS = 4 };
enum { JOB_FREE, JOB_RUNNING, JOB_DONE, JOB_FAILED, JOB_ABANDONED };

/* a file's plain SHA-256 and size, read a megabyte at a time into a buffer
of the caller's */
static int sha256_file_sized(char const *path, unsigned char digest[32], unsigned long long *size,
	volatile int const *cancel, unsigned char *buffer, size_t buffer_size)
{
	struct sha sha;
	FILE *file;
	size_t got;
	unsigned long long total = 0;
	int ok = 1;

	pthread_once(&hash_once, hash_setup);
	file = fopen(path, "rb");
	if (!file)
		return 0;
	sha_begin(&sha);
	while ((got = fread(buffer, 1, buffer_size, file)) > 0) {
		if (cancel && *cancel) {
			ok = 0;
			break;
		}
		sha_add(&sha, buffer, got);
		total += got;
	}
	if (ferror(file))
		ok = 0;
	fclose(file);
	if (ok)
		sha_end(&sha, digest);
	if (size)
		*size = total;
	return ok;
}

struct job {
	int state;
	volatile int cancel;
	char data_root[1024];
	char paths[3][1024];
	uint32_t crc;
	struct guest_result result;
};

static pthread_mutex_t jobs_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t jobs_changed = PTHREAD_COND_INITIALIZER;
static struct job jobs[JOBS];

static void *job_main(void *parameter)
{
	struct job *job = parameter;
	struct known entry;
	unsigned long began = hash_milliseconds();
	int ok = 1, type, hit;

	memset(&entry, 0, sizeof(entry));
	for (type = 0; type < 3 && ok; type++)
		if (job->paths[type][0])
			ok = key_append(entry.key, sizeof(entry.key), job->paths[type]);
	hit = ok && known_find(job->data_root, entry.key, &entry);
	if (ok && !hit) {
		struct sha sha;
		struct sha *shas[1] = { &sha };
		uint32_t crc = 0;
		uint64_t length = 0;

		sha_begin(&sha);
		sha_add(&sha, FINGERPRINT_PREFIX, sizeof(FINGERPRINT_PREFIX) - 1);
		for (type = 0; type < 3 && ok; type++) {
			long long size;

			if (!job->paths[type][0])
				continue;
			size = hash_file(job->paths[type], (unsigned)type, shas, 1, type ? &crc : NULL, &job->cancel, NULL, NULL);
			ok = size >= 0;
			if (type)
				length += (uint64_t)size + 2;
		}
		if (ok) {
			sha_end(&sha, entry.digest);
			entry.crc = crc;
			entry.length = length;
			known_store(job->data_root, &entry, 1);
		}
	}
	if (ok) {
		job->result.crc = crc_combine(job->crc, entry.crc, entry.length);
		memcpy(job->result.digest, entry.digest, 32);
		hash_log("mcc fingerprints: %s %s in %lu ms (%s, %s)", job->paths[0], hit ? "known" : "worked out",
			hash_milliseconds() - began, sha_kind, crc_kind);
	} else if (!job->cancel)
		hash_log("mcc fingerprints: %s could not be read", job->paths[0]);
	pthread_mutex_lock(&jobs_lock);
	job->state = job->state == JOB_ABANDONED ? JOB_FREE : ok ? JOB_DONE : JOB_FAILED;
	pthread_cond_broadcast(&jobs_changed);
	pthread_mutex_unlock(&jobs_lock);
	return NULL;
}

int host_mcc_hash_start(unsigned int request_address)
{
	struct guest_request const *request = (struct guest_request const *)(uintptr_t)request_address;
	pthread_t thread;
	int id, type;

	pthread_once(&hash_once, hash_setup);
	if (!request || !request->paths[0])
		return -1;
	pthread_mutex_lock(&jobs_lock);
	for (id = 0; id < JOBS && jobs[id].state != JOB_FREE; id++) {
	}
	if (id == JOBS) {
		pthread_mutex_unlock(&jobs_lock);
		return -1;
	}
	memset(&jobs[id], 0, sizeof(jobs[id]));
	jobs[id].state = JOB_RUNNING;
	pthread_mutex_unlock(&jobs_lock);
	if (request->data_root)
		snprintf(jobs[id].data_root, sizeof(jobs[id].data_root), "%s", (char const *)(uintptr_t)request->data_root);
	for (type = 0; type < 3; type++)
		if (request->paths[type])
			snprintf(jobs[id].paths[type], sizeof(jobs[id].paths[type]), "%s",
				(char const *)(uintptr_t)request->paths[type]);
	jobs[id].crc = request->crc;
	if (pthread_create(&thread, NULL, job_main, &jobs[id]) != 0) {
		pthread_mutex_lock(&jobs_lock);
		jobs[id].state = JOB_FREE;
		pthread_mutex_unlock(&jobs_lock);
		return -1;
	}
	pthread_detach(thread);
	return id;
}

int host_mcc_hash_wait(int id, int milliseconds, unsigned int result_address)
{
	struct guest_result *result = (struct guest_result *)(uintptr_t)result_address;
	struct timespec until;
	int status;

	if (id < 0 || id >= JOBS)
		return -1;
	clock_gettime(CLOCK_REALTIME, &until);
	until.tv_sec += milliseconds / 1000;
	until.tv_nsec += (long)(milliseconds % 1000) * 1000000L;
	if (until.tv_nsec >= 1000000000L) {
		until.tv_sec++;
		until.tv_nsec -= 1000000000L;
	}
	pthread_mutex_lock(&jobs_lock);
	while (jobs[id].state == JOB_RUNNING && milliseconds > 0 &&
		pthread_cond_timedwait(&jobs_changed, &jobs_lock, &until) == 0) {
	}
	status = jobs[id].state == JOB_DONE ? 1 : jobs[id].state == JOB_RUNNING ? 0 : -1;
	if (status == 1 && result)
		*result = jobs[id].result;
	if (status != 0 && jobs[id].state != JOB_FREE && jobs[id].state != JOB_ABANDONED)
		jobs[id].state = JOB_FREE;
	pthread_mutex_unlock(&jobs_lock);
	return status;
}

void host_mcc_hash_cancel(int id)
{
	if (id < 0 || id >= JOBS)
		return;
	pthread_mutex_lock(&jobs_lock);
	if (jobs[id].state == JOB_RUNNING) {
		/* (its thread frees it when it stops) */
		jobs[id].cancel = 1;
		jobs[id].state = JOB_ABANDONED;
	} else if (jobs[id].state != JOB_ABANDONED)
		jobs[id].state = JOB_FREE;
	pthread_mutex_unlock(&jobs_lock);
}

/* ---------- a file's plain SHA-256 for the guest, on a thread of its own
(host_sha256_start/_poll/_cancel: an MCC package's files, which a host
hashes before it offers them to a joiner, mcc_share_session.c; in the game's
frame, a few megabytes at a time in software, a 360 MB package outlasted a
joiner's wait for the offer, and took the frames with it) */

enum { SHA_JOBS = 4 };

struct sha_job {
	int state;
	volatile int cancel;
	char path[1024];
	unsigned char digest[32];
	unsigned long long size;
};

static struct sha_job sha_jobs[SHA_JOBS];

static void *sha_job_main(void *parameter)
{
	struct sha_job *job = parameter;
	unsigned char *buffer = malloc(1 << 20);
	int ok;

	ok = buffer && sha256_file_sized(job->path, job->digest, &job->size, &job->cancel, buffer, 1 << 20);
	free(buffer);
	pthread_mutex_lock(&jobs_lock);
	job->state = job->state == JOB_ABANDONED ? JOB_FREE : ok ? JOB_DONE : JOB_FAILED;
	pthread_cond_broadcast(&jobs_changed);
	pthread_mutex_unlock(&jobs_lock);
	return NULL;
}

int host_sha256_start(unsigned int path_address)
{
	char const *path = (char const *)(uintptr_t)path_address;
	pthread_t thread;
	int id;

	if (!path || !path[0] || strlen(path) >= sizeof(sha_jobs[0].path))
		return -1;
	pthread_mutex_lock(&jobs_lock);
	for (id = 0; id < SHA_JOBS && sha_jobs[id].state != JOB_FREE; id++) {
	}
	if (id == SHA_JOBS) {
		pthread_mutex_unlock(&jobs_lock);
		return -1;
	}
	memset(&sha_jobs[id], 0, sizeof(sha_jobs[id]));
	sha_jobs[id].state = JOB_RUNNING;
	strcpy(sha_jobs[id].path, path);
	pthread_mutex_unlock(&jobs_lock);
	if (pthread_create(&thread, NULL, sha_job_main, &sha_jobs[id]) != 0) {
		pthread_mutex_lock(&jobs_lock);
		sha_jobs[id].state = JOB_FREE;
		pthread_mutex_unlock(&jobs_lock);
		return -1;
	}
	pthread_detach(thread);
	return id;
}

/* 1 done (the digest and the size, low and high, written: the job freed), 0
still reading, -1 failed (freed) */
int host_sha256_poll(int id, unsigned int digest_address, unsigned int size_address)
{
	int status;

	if (id < 0 || id >= SHA_JOBS)
		return -1;
	pthread_mutex_lock(&jobs_lock);
	status = sha_jobs[id].state == JOB_DONE ? 1 : sha_jobs[id].state == JOB_RUNNING ? 0 : -1;
	if (status == 1) {
		memcpy((void *)(uintptr_t)digest_address, sha_jobs[id].digest, 32);
		((uint32_t *)(uintptr_t)size_address)[0] = (uint32_t)sha_jobs[id].size;
		((uint32_t *)(uintptr_t)size_address)[1] = (uint32_t)(sha_jobs[id].size >> 32);
	}
	if (status != 0 && sha_jobs[id].state != JOB_FREE && sha_jobs[id].state != JOB_ABANDONED)
		sha_jobs[id].state = JOB_FREE;
	pthread_mutex_unlock(&jobs_lock);
	return status;
}

void host_sha256_cancel(int id)
{
	if (id < 0 || id >= SHA_JOBS)
		return;
	pthread_mutex_lock(&jobs_lock);
	if (sha_jobs[id].state == JOB_RUNNING) {
		sha_jobs[id].cancel = 1;
		sha_jobs[id].state = JOB_ABANDONED;
	} else if (sha_jobs[id].state != JOB_ABANDONED)
		sha_jobs[id].state = JOB_FREE;
	pthread_mutex_unlock(&jobs_lock);
}

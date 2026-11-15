/*
STEAM_UNPACK.C

The packings of Steam's depot chunks (steam_depot.h's steam_unpack_chunk):

- "VZa": 3 bytes, 4 more (a timestamp or CRC), the 5 LZMA properties, the
  raw LZMA stream, and a 10-byte footer (CRC-32, size, "zv"); the LZMA SDK's
  decoder (port/third_party/lzma, public domain);
- "VSZa": 4 bytes, 4 more, a Zstandard frame, and a 15-byte footer;
  Zstandard's decompressor (port/third_party/zstd, BSD), built here as one
  unit the way its single-file build is;
- a zip of one file (old chunks, and every manifest), inflated by the
  port's zlib.
*/

#include "steam_depot.h"
#include "zlib_prefixed.h"

#include <stdlib.h>
#include <string.h>

/* ---------- LZMA */

#include "../../third_party/lzma/LzmaDec.c"

static void *lzma_alloc(ISzAllocPtr allocator, size_t size)
{
	(void)allocator;
	return malloc(size);
}

static void lzma_free(ISzAllocPtr allocator, void *address)
{
	(void)allocator;
	free(address);
}

static int unpack_lzma(unsigned char const *props, unsigned char const *data, size_t data_size, unsigned char *out,
	size_t size)
{
	ISzAlloc allocator = { lzma_alloc, lzma_free };
	SizeT out_size = size;
	SizeT in_size = data_size;
	ELzmaStatus status;
	SRes result = LzmaDecode(out, &out_size, data, &in_size, props, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status,
		&allocator);

	return result == SZ_OK && out_size == size;
}

/* ---------- Zstandard */

#define DEBUGLEVEL 0
#define MEM_MODULE
#undef XXH_NAMESPACE
#define XXH_NAMESPACE ZSTD_
#undef XXH_PRIVATE_API
#define XXH_PRIVATE_API
#undef XXH_INLINE_ALL
#define XXH_INLINE_ALL
#define ZSTD_LEGACY_SUPPORT 0
#define ZSTD_STRIP_ERROR_STRINGS
#define ZSTD_TRACE 0
#define ZSTD_DISABLE_ASM 1
#define ZSTD_DEPS_NEED_MALLOC
/* (no SIMD headers: the guests have none of the compilers' own) */
#define ZSTD_NO_INTRINSICS
#include "../../third_party/zstd/lib/common/zstd_deps.h"
#include "../../third_party/zstd/lib/common/debug.c"
#include "../../third_party/zstd/lib/common/entropy_common.c"
#include "../../third_party/zstd/lib/common/error_private.c"
#include "../../third_party/zstd/lib/common/fse_decompress.c"
#include "../../third_party/zstd/lib/common/zstd_common.c"
#include "../../third_party/zstd/lib/decompress/huf_decompress.c"
#include "../../third_party/zstd/lib/decompress/zstd_ddict.c"
#include "../../third_party/zstd/lib/decompress/zstd_decompress.c"
#include "../../third_party/zstd/lib/decompress/zstd_decompress_block.c"

static int unpack_zstd(unsigned char const *data, size_t data_size, unsigned char *out, size_t size)
{
	size_t result = ZSTD_decompress(out, size, data, data_size);

	return !ZSTD_isError(result) && result == size;
}

/* ---------- zip */

static uint32_t get32(unsigned char const *at)
{
	return (uint32_t)at[0] | (uint32_t)at[1] << 8 | (uint32_t)at[2] << 16 | (uint32_t)at[3] << 24;
}

static unsigned get16(unsigned char const *at)
{
	return (unsigned)at[0] | (unsigned)at[1] << 8;
}

size_t steam_unzip_one(unsigned char const *data, size_t size, unsigned char **out)
{
	long end;
	uint32_t central, packed, unpacked, local;
	unsigned method;
	size_t start;

	*out = NULL;
	if (size < 22)
		return 0;
	for (end = (long)size - 22; end >= 0 && get32(data + end) != 0x06054b50; end--)
		;
	if (end < 0)
		return 0;
	central = get32(data + end + 16);
	if ((size_t)central + 46 > size || get32(data + central) != 0x02014b50)
		return 0;
	method = get16(data + central + 10);
	packed = get32(data + central + 20);
	unpacked = get32(data + central + 24);
	local = get32(data + central + 42);
	if ((size_t)local + 30 > size || get32(data + local) != 0x04034b50)
		return 0;
	start = (size_t)local + 30 + get16(data + local + 26) + get16(data + local + 28);
	if (start > size || packed > size - start || unpacked > 256u * 1024 * 1024)
		return 0;
	*out = malloc(unpacked ? unpacked : 1);
	if (!*out)
		return 0;
	if (method == 0 && packed == unpacked)
	{
		memcpy(*out, data + start, unpacked);
		return unpacked;
	}
	if (method == 8)
	{
		z_stream stream;
		int result;

		memset(&stream, 0, sizeof(stream));
		if (inflateInit2(&stream, -MAX_WBITS) == Z_OK)
		{
			stream.next_in = (Bytef *)(data + start);
			stream.avail_in = packed;
			stream.next_out = *out;
			stream.avail_out = unpacked;
			result = inflate(&stream, Z_FINISH);
			inflateEnd(&stream);
			if (result == Z_STREAM_END && stream.total_out == unpacked)
				return unpacked;
		}
	}
	free(*out);
	*out = NULL;
	return 0;
}

int steam_unpack_chunk(unsigned char const *data, size_t data_size, unsigned char *out, size_t size)
{
	if (data_size >= 12 + 10 && data[0] == 'V' && data[1] == 'Z' && data[2] == 'a')
		return unpack_lzma(data + 7, data + 12, data_size - 12 - 10, out, size);
	if (data_size >= 8 + 15 && data[0] == 'V' && data[1] == 'S' && data[2] == 'Z' && data[3] == 'a')
		return unpack_zstd(data + 8, data_size - 8 - 15, out, size);
	if (data_size >= 4 && data[0] == 'P' && data[1] == 'K')
	{
		unsigned char *unzipped;
		size_t length = steam_unzip_one(data, data_size, &unzipped);
		int ok = length == size;

		if (ok)
			memcpy(out, unzipped, size);
		free(unzipped);
		return ok;
	}
	return 0;
}

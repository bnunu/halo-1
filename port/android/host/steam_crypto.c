/*
STEAM_CRYPTO.C

steam_crypto.h's: SHA-1 (FIPS 180-4), HMAC (RFC 2104), AES-256 (FIPS 197,
its tables made once from the field's arithmetic rather than typed in),
RSA-OAEP with SHA-1 (RFC 8017) by a small bignum (the public exponent is
17: four squarings and a product), CRC-32 and base64.
*/

#include "steam_crypto.h"
#include "posix.h"

#include <string.h>

/* ---------- SHA-1 */

static uint32_t rotl32(uint32_t value, int bits)
{
	return (value << bits) | (value >> (32 - bits));
}

static void sha1_block(struct steam_sha1 *sha, unsigned char const block[64])
{
	uint32_t w[80];
	uint32_t a, b, c, d, e;
	int i;

	for (i = 0; i < 16; i++)
		w[i] = (uint32_t)block[4 * i] << 24 | (uint32_t)block[4 * i + 1] << 16 |
			(uint32_t)block[4 * i + 2] << 8 | block[4 * i + 3];
	for (; i < 80; i++)
		w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	a = sha->state[0]; b = sha->state[1]; c = sha->state[2]; d = sha->state[3]; e = sha->state[4];
	for (i = 0; i < 80; i++)
	{
		uint32_t f, k, t;

		if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
		else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
		else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
		else { f = b ^ c ^ d; k = 0xCA62C1D6; }
		t = rotl32(a, 5) + f + e + k + w[i];
		e = d; d = c; c = rotl32(b, 30); b = a; a = t;
	}
	sha->state[0] += a; sha->state[1] += b; sha->state[2] += c; sha->state[3] += d; sha->state[4] += e;
}

void steam_sha1_begin(struct steam_sha1 *sha)
{
	sha->state[0] = 0x67452301; sha->state[1] = 0xEFCDAB89; sha->state[2] = 0x98BADCFE;
	sha->state[3] = 0x10325476; sha->state[4] = 0xC3D2E1F0;
	sha->length = 0;
	sha->used = 0;
}

void steam_sha1_add(struct steam_sha1 *sha, void const *data, size_t size)
{
	unsigned char const *bytes = data;

	sha->length += size;
	while (size)
	{
		size_t take = 64 - sha->used;

		if (take > size)
			take = size;
		memcpy(sha->block + sha->used, bytes, take);
		sha->used += (unsigned)take;
		bytes += take;
		size -= take;
		if (sha->used == 64)
		{
			sha1_block(sha, sha->block);
			sha->used = 0;
		}
	}
}

void steam_sha1_end(struct steam_sha1 *sha, unsigned char digest[20])
{
	uint64_t bits = sha->length * 8;
	unsigned char tail[8];
	unsigned char one = 0x80, zero = 0;
	int i;

	steam_sha1_add(sha, &one, 1);
	while (sha->used != 56)
		steam_sha1_add(sha, &zero, 1);
	for (i = 0; i < 8; i++)
		tail[i] = (unsigned char)(bits >> (56 - 8 * i));
	steam_sha1_add(sha, tail, 8);
	for (i = 0; i < 20; i++)
		digest[i] = (unsigned char)(sha->state[i / 4] >> (24 - 8 * (i % 4)));
}

void steam_sha1(void const *data, size_t size, unsigned char digest[20])
{
	struct steam_sha1 sha;

	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, data, size);
	steam_sha1_end(&sha, digest);
}

void steam_hmac_sha1(void const *key, size_t key_size, void const *data, size_t size, unsigned char digest[20])
{
	unsigned char pad[64];
	unsigned char inner[20];
	unsigned char hashed_key[20];
	struct steam_sha1 sha;
	int i;

	if (key_size > 64)
	{
		steam_sha1(key, key_size, hashed_key);
		key = hashed_key;
		key_size = 20;
	}
	memset(pad, 0, sizeof(pad));
	memcpy(pad, key, key_size);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, data, size);
	steam_sha1_end(&sha, inner);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36 ^ 0x5C;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, inner, 20);
	steam_sha1_end(&sha, digest);
}

/* ---------- AES-256 */

static struct
{
	int made;
	unsigned char sbox[256];
	unsigned char inverse[256];
	uint32_t te[256];
	uint32_t td[256];
} aes_tables;

static unsigned char gf_multiply(unsigned char a, unsigned char b)
{
	unsigned char product = 0;

	while (b)
	{
		if (b & 1)
			product ^= a;
		a = (unsigned char)((a << 1) ^ ((a & 0x80) ? 0x1B : 0));
		b >>= 1;
	}
	return product;
}

static uint32_t rotr32(uint32_t value, int bits)
{
	return (value >> bits) | (value << (32 - bits));
}

static void aes_tables_make(void)
{
	int x;

	if (aes_tables.made)
		return;
	for (x = 0; x < 256; x++)
	{
		/* the multiplicative inverse (x^254), then the affine transform */
		unsigned char inverse = 1, power = (unsigned char)x, s;
		int bit;

		for (bit = 0; bit < 8; bit++)
		{
			if (254 & (1 << bit))
				inverse = gf_multiply(inverse, power);
			power = gf_multiply(power, power);
		}
		if (!x)
			inverse = 0;
		s = inverse;
		s = (unsigned char)(s ^ ((inverse << 1) | (inverse >> 7)) ^ ((inverse << 2) | (inverse >> 6)) ^
			((inverse << 3) | (inverse >> 5)) ^ ((inverse << 4) | (inverse >> 4)) ^ 0x63);
		aes_tables.sbox[x] = s;
		aes_tables.inverse[s] = (unsigned char)x;
	}
	for (x = 0; x < 256; x++)
	{
		unsigned char s = aes_tables.sbox[x];
		unsigned char i = aes_tables.inverse[x];

		aes_tables.te[x] = (uint32_t)gf_multiply(s, 2) << 24 | (uint32_t)s << 16 | (uint32_t)s << 8 | gf_multiply(s, 3);
		aes_tables.td[x] = (uint32_t)gf_multiply(i, 14) << 24 | (uint32_t)gf_multiply(i, 9) << 16 |
			(uint32_t)gf_multiply(i, 13) << 8 | gf_multiply(i, 11);
	}
	aes_tables.made = 1;
}

static uint32_t sub_word(uint32_t word)
{
	return (uint32_t)aes_tables.sbox[word >> 24] << 24 | (uint32_t)aes_tables.sbox[(word >> 16) & 255] << 16 |
		(uint32_t)aes_tables.sbox[(word >> 8) & 255] << 8 | aes_tables.sbox[word & 255];
}

void steam_aes_key(struct steam_aes *aes, unsigned char const key[32])
{
	uint32_t rcon = 0x01000000;
	int i, round;

	aes_tables_make();
	for (i = 0; i < 8; i++)
		aes->encrypt[i] = (uint32_t)key[4 * i] << 24 | (uint32_t)key[4 * i + 1] << 16 |
			(uint32_t)key[4 * i + 2] << 8 | key[4 * i + 3];
	for (i = 8; i < 60; i++)
	{
		uint32_t temp = aes->encrypt[i - 1];

		if (i % 8 == 0)
		{
			temp = sub_word(rotl32(temp, 8)) ^ rcon;
			rcon = (uint32_t)gf_multiply((unsigned char)(rcon >> 24), 2) << 24;
		}
		else if (i % 8 == 4)
			temp = sub_word(temp);
		aes->encrypt[i] = aes->encrypt[i - 8] ^ temp;
	}
	/* (the equivalent inverse cipher's: the rounds reversed, the middle
	ones' through InvMixColumns, which td of sbox gives) */
	for (round = 0; round <= 14; round++)
	{
		for (i = 0; i < 4; i++)
		{
			uint32_t word = aes->encrypt[4 * (14 - round) + i];

			if (round && round < 14)
				word = aes_tables.td[aes_tables.sbox[word >> 24]] ^
					rotr32(aes_tables.td[aes_tables.sbox[(word >> 16) & 255]], 8) ^
					rotr32(aes_tables.td[aes_tables.sbox[(word >> 8) & 255]], 16) ^
					rotr32(aes_tables.td[aes_tables.sbox[word & 255]], 24);
			aes->decrypt[4 * round + i] = word;
		}
	}
}

static void load_block(unsigned char const in[16], uint32_t s[4])
{
	int i;

	for (i = 0; i < 4; i++)
		s[i] = (uint32_t)in[4 * i] << 24 | (uint32_t)in[4 * i + 1] << 16 | (uint32_t)in[4 * i + 2] << 8 | in[4 * i + 3];
}

static void store_block(uint32_t const s[4], unsigned char out[16])
{
	int i;

	for (i = 0; i < 4; i++)
	{
		out[4 * i] = (unsigned char)(s[i] >> 24);
		out[4 * i + 1] = (unsigned char)(s[i] >> 16);
		out[4 * i + 2] = (unsigned char)(s[i] >> 8);
		out[4 * i + 3] = (unsigned char)s[i];
	}
}

void steam_aes_encrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16])
{
	uint32_t s[4], t[4];
	uint32_t const *rk = aes->encrypt;
	int round, i;

	load_block(in, s);
	for (i = 0; i < 4; i++)
		s[i] ^= rk[i];
	for (round = 1; round < 14; round++)
	{
		rk += 4;
		for (i = 0; i < 4; i++)
			t[i] = aes_tables.te[s[i] >> 24] ^ rotr32(aes_tables.te[(s[(i + 1) & 3] >> 16) & 255], 8) ^
				rotr32(aes_tables.te[(s[(i + 2) & 3] >> 8) & 255], 16) ^ rotr32(aes_tables.te[s[(i + 3) & 3] & 255], 24) ^
				rk[i];
		memcpy(s, t, sizeof(s));
	}
	rk += 4;
	for (i = 0; i < 4; i++)
		t[i] = ((uint32_t)aes_tables.sbox[s[i] >> 24] << 24 | (uint32_t)aes_tables.sbox[(s[(i + 1) & 3] >> 16) & 255] << 16 |
			(uint32_t)aes_tables.sbox[(s[(i + 2) & 3] >> 8) & 255] << 8 | aes_tables.sbox[s[(i + 3) & 3] & 255]) ^ rk[i];
	store_block(t, out);
}

void steam_aes_decrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16])
{
	uint32_t s[4], t[4];
	uint32_t const *rk = aes->decrypt;
	int round, i;

	load_block(in, s);
	for (i = 0; i < 4; i++)
		s[i] ^= rk[i];
	for (round = 1; round < 14; round++)
	{
		rk += 4;
		for (i = 0; i < 4; i++)
			t[i] = aes_tables.td[s[i] >> 24] ^ rotr32(aes_tables.td[(s[(i + 3) & 3] >> 16) & 255], 8) ^
				rotr32(aes_tables.td[(s[(i + 2) & 3] >> 8) & 255], 16) ^ rotr32(aes_tables.td[s[(i + 1) & 3] & 255], 24) ^
				rk[i];
		memcpy(s, t, sizeof(s));
	}
	rk += 4;
	for (i = 0; i < 4; i++)
		t[i] = ((uint32_t)aes_tables.inverse[s[i] >> 24] << 24 |
			(uint32_t)aes_tables.inverse[(s[(i + 3) & 3] >> 16) & 255] << 16 |
			(uint32_t)aes_tables.inverse[(s[(i + 2) & 3] >> 8) & 255] << 8 | aes_tables.inverse[s[(i + 1) & 3] & 255]) ^
			rk[i];
	store_block(t, out);
}

/* CBC with PKCS#7 padding after the vector, in place in out */
static int cbc_decrypt(struct steam_aes const *aes, unsigned char const iv[16], unsigned char const *in, size_t size,
	unsigned char *out, size_t *plain)
{
	unsigned char previous[16], block[16], save[16];
	size_t at;
	unsigned pad, i;

	if (!size || size % 16)
		return 0;
	memcpy(previous, iv, 16);
	for (at = 0; at < size; at += 16)
	{
		memcpy(save, in + at, 16);
		steam_aes_decrypt_block(aes, save, block);
		for (i = 0; i < 16; i++)
			out[at + i] = block[i] ^ previous[i];
		memcpy(previous, save, 16);
	}
	pad = out[size - 1];
	if (!pad || pad > 16)
		return 0;
	for (i = 0; i < pad; i++)
		if (out[size - 1 - i] != pad)
			return 0;
	*plain = size - pad;
	return 1;
}

int steam_aes_decrypt(struct steam_aes const *aes, unsigned char const *in, size_t in_size,
	unsigned char *out, size_t *size)
{
	unsigned char iv[16];

	if (in_size < 32 || in_size % 16)
		return 0;
	steam_aes_decrypt_block(aes, in, iv);
	/* (out may be in: the data moves down by one block as it is decrypted) */
	return cbc_decrypt(aes, iv, in + 16, in_size - 16, out, size);
}

size_t steam_aes_encrypted_size(size_t size)
{
	return 16 + (size / 16 + 1) * 16;
}

static void cbc_encrypt(struct steam_aes const *aes, unsigned char const iv[16], unsigned char const *in, size_t size,
	unsigned char *out)
{
	unsigned char previous[16], block[16];
	size_t at = 0, total = (size / 16 + 1) * 16;
	unsigned i;

	memcpy(previous, iv, 16);
	for (at = 0; at < total; at += 16)
	{
		for (i = 0; i < 16; i++)
		{
			unsigned char byte = at + i < size ? in[at + i] : (unsigned char)(total - size);

			block[i] = byte ^ previous[i];
		}
		steam_aes_encrypt_block(aes, block, out + at);
		memcpy(previous, out + at, 16);
	}
}

void steam_aes_encrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t size, unsigned char *out)
{
	unsigned char iv[16];
	unsigned char digest[20];
	struct steam_sha1 sha;
	unsigned char pad[64];
	unsigned char inner[20];
	int i;

	steam_random(iv + 13, 3);
	/* HMAC-SHA1 of (the three random bytes, the plain text), by hand to
	spare a copy of the text */
	memset(pad, 0, sizeof(pad));
	memcpy(pad, hmac_key, 16);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, iv + 13, 3);
	steam_sha1_add(&sha, in, size);
	steam_sha1_end(&sha, inner);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36 ^ 0x5C;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, inner, 20);
	steam_sha1_end(&sha, digest);
	memcpy(iv, digest, 13);
	steam_aes_encrypt_block(aes, iv, out);
	cbc_encrypt(aes, iv, in, size, out + 16);
}

int steam_aes_decrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t in_size, unsigned char *out, size_t *size)
{
	unsigned char iv[16];
	unsigned char digest[20];
	unsigned char pad[64];
	unsigned char inner[20];
	struct steam_sha1 sha;
	int i;

	if (in_size < 32 || in_size % 16)
		return 0;
	steam_aes_decrypt_block(aes, in, iv);
	if (!cbc_decrypt(aes, iv, in + 16, in_size - 16, out, size))
		return 0;
	memset(pad, 0, sizeof(pad));
	memcpy(pad, hmac_key, 16);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, iv + 13, 3);
	steam_sha1_add(&sha, out, *size);
	steam_sha1_end(&sha, inner);
	for (i = 0; i < 64; i++)
		pad[i] ^= 0x36 ^ 0x5C;
	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, pad, 64);
	steam_sha1_add(&sha, inner, 20);
	steam_sha1_end(&sha, digest);
	return !memcmp(digest, iv, 13);
}

/* ---------- RSA */

/* the public universe's key's modulus (SteamKit's KeyDictionary, its
SubjectPublicKeyInfo's integer; the exponent is 17) */
static unsigned char const universe_modulus[128] =
{
	0xDF, 0xEC, 0x1A, 0xD6, 0x2C, 0x10, 0x66, 0x2C, 0x17, 0x35, 0x3A, 0x14, 0xB0, 0x7C, 0x59, 0x11,
	0x7F, 0x9D, 0xD3, 0xD8, 0x2B, 0x7A, 0xE3, 0xE0, 0x15, 0xCD, 0x19, 0x1E, 0x46, 0xE8, 0x7B, 0x87,
	0x74, 0xA2, 0x18, 0x46, 0x31, 0xA9, 0x03, 0x14, 0x79, 0x82, 0x8E, 0xE9, 0x45, 0xA2, 0x49, 0x12,
	0xA9, 0x23, 0x68, 0x73, 0x89, 0xCF, 0x69, 0xA1, 0xB1, 0x61, 0x46, 0xBD, 0xC1, 0xBE, 0xBF, 0xD6,
	0x01, 0x1B, 0xD8, 0x81, 0xD4, 0xDC, 0x90, 0xFB, 0xFE, 0x4F, 0x52, 0x73, 0x66, 0xCB, 0x95, 0x70,
	0xD7, 0xC5, 0x8E, 0xBA, 0x1C, 0x7A, 0x33, 0x75, 0xA1, 0x62, 0x34, 0x46, 0xBB, 0x60, 0xB7, 0x80,
	0x68, 0xFA, 0x13, 0xA7, 0x7A, 0x8A, 0x37, 0x4B, 0x9E, 0xC6, 0xF4, 0x5D, 0x5F, 0x3A, 0x99, 0xF9,
	0x9E, 0xC4, 0x3A, 0xE9, 0x63, 0xA2, 0xBB, 0x88, 0x19, 0x28, 0xE0, 0xE7, 0x14, 0xC0, 0x42, 0x89,
};
/* MGF1 with SHA-1 */
static void mgf1_xor(unsigned char const *seed, size_t seed_size, unsigned char *out, size_t size)
{
	uint32_t counter;
	size_t done = 0;

	for (counter = 0; done < size; counter++)
	{
		unsigned char digest[20];
		unsigned char count[4];
		struct steam_sha1 sha;
		size_t i;

		count[0] = (unsigned char)(counter >> 24);
		count[1] = (unsigned char)(counter >> 16);
		count[2] = (unsigned char)(counter >> 8);
		count[3] = (unsigned char)counter;
		steam_sha1_begin(&sha);
		steam_sha1_add(&sha, seed, seed_size);
		steam_sha1_add(&sha, count, 4);
		steam_sha1_end(&sha, digest);
		for (i = 0; i < 20 && done < size; i++)
			out[done++] ^= digest[i];
	}
}

/* 32-bit limbs, the least first: up to 4096-bit keys */
#define RSA_MAXIMUM_LIMBS 128

/* bytes (big-endian, size of them) into limbs limbs */
static void big_from_bytes(unsigned char const *bytes, size_t size, uint32_t *big, int limbs)
{
	size_t i;

	memset(big, 0, sizeof(uint32_t) * (size_t)limbs);
	for (i = 0; i < size; i++)
	{
		size_t bit_byte = size - 1 - i;

		big[bit_byte / 4] |= (uint32_t)bytes[i] << (8 * (bit_byte % 4));
	}
}

static void big_to_bytes(uint32_t const *big, unsigned char *bytes, size_t size)
{
	size_t i;

	for (i = 0; i < size; i++)
	{
		size_t bit_byte = size - 1 - i;

		bytes[i] = (unsigned char)(big[bit_byte / 4] >> (8 * (bit_byte % 4)));
	}
}

/* result = a * b mod n (each below n, limbs long), by the product's bits from the top */
static void big_multiply_mod(uint32_t const *a, uint32_t const *b, uint32_t const *n, int limbs, uint32_t *result)
{
	uint32_t product[2 * RSA_MAXIMUM_LIMBS];
	uint32_t r[RSA_MAXIMUM_LIMBS + 1];
	int i, j, bit;

	memset(product, 0, sizeof(uint32_t) * 2 * (size_t)limbs);
	for (i = 0; i < limbs; i++)
	{
		uint64_t carry = 0;

		for (j = 0; j < limbs; j++)
		{
			uint64_t sum = (uint64_t)a[i] * b[j] + product[i + j] + carry;

			product[i + j] = (uint32_t)sum;
			carry = sum >> 32;
		}
		product[i + limbs] = (uint32_t)carry;
	}
	memset(r, 0, sizeof(uint32_t) * ((size_t)limbs + 1));
	for (bit = 2 * limbs * 32 - 1; bit >= 0; bit--)
	{
		int greater_or_equal = 1;

		for (j = limbs; j > 0; j--)
			r[j] = (r[j] << 1) | (r[j - 1] >> 31);
		r[0] = (r[0] << 1) | ((product[bit / 32] >> (bit % 32)) & 1);
		if (!r[limbs])
		{
			for (j = limbs - 1; j >= 0; j--)
			{
				if (r[j] != n[j])
				{
					greater_or_equal = r[j] > n[j];
					break;
				}
			}
		}
		if (greater_or_equal)
		{
			uint64_t borrow = 0;

			for (j = 0; j < limbs; j++)
			{
				uint64_t difference = (uint64_t)r[j] - n[j] - borrow;

				r[j] = (uint32_t)difference;
				borrow = (difference >> 32) & 1;
			}
			r[limbs] -= (uint32_t)borrow;
		}
	}
	memcpy(result, r, sizeof(uint32_t) * (size_t)limbs);
}

/* out = message^exponent mod modulus, each size bytes, big-endian */
static int rsa_raw(unsigned char const *modulus, size_t size, uint32_t exponent, unsigned char const *message,
	unsigned char *out)
{
	uint32_t m[RSA_MAXIMUM_LIMBS], n[RSA_MAXIMUM_LIMBS], c[RSA_MAXIMUM_LIMBS];
	int limbs = (int)((size + 3) / 4);
	int bit, top = 31;

	if (!size || limbs > RSA_MAXIMUM_LIMBS || !exponent)
		return 0;
	big_from_bytes(message, size, m, limbs);
	big_from_bytes(modulus, size, n, limbs);
	/* (square and multiply, from the exponent's top bit down) */
	while (top > 0 && !(exponent >> top))
		top--;
	memcpy(c, m, sizeof(uint32_t) * (size_t)limbs);
	for (bit = top - 1; bit >= 0; bit--)
	{
		big_multiply_mod(c, c, n, limbs, c);
		if ((exponent >> bit) & 1)
			big_multiply_mod(c, m, n, limbs, c);
	}
	big_to_bytes(c, out, size);
	return 1;
}

int steam_rsa_encrypt_public_universe(unsigned char const *message, size_t size, unsigned char out[128])
{
	enum { K = 128, H = 20 };
	unsigned char encoded[K];
	unsigned char *seed = encoded + 1;
	unsigned char *block = encoded + 1 + H;

	if (size > K - 2 * H - 2)
		return 0;
	/* EM = 0, masked seed, masked (lHash, zeros, 1, the message) */
	memset(encoded, 0, sizeof(encoded));
	steam_sha1("", 0, block);
	block[K - H - 1 - size - 1] = 1;
	memcpy(block + K - H - 1 - size, message, size);
	steam_random(seed, H);
	mgf1_xor(seed, H, block, K - H - 1);
	mgf1_xor(block, K - H - 1, seed, H);
	return rsa_raw(universe_modulus, K, 17, encoded, out);
}

int steam_rsa_encrypt_pkcs1(unsigned char const *modulus, size_t modulus_size, uint32_t exponent,
	unsigned char const *message, size_t size, unsigned char *out)
{
	unsigned char encoded[RSA_MAXIMUM_LIMBS * 4];
	size_t i, pad;

	while (modulus_size && !modulus[0])
	{
		modulus++;
		modulus_size--;
	}
	if (modulus_size > sizeof(encoded) || modulus_size < 11 || size > modulus_size - 11)
		return 0;
	/* 0, 2, random nonzero bytes, 0, the message */
	pad = modulus_size - 3 - size;
	encoded[0] = 0;
	encoded[1] = 2;
	steam_random(encoded + 2, pad);
	for (i = 0; i < pad; i++)
		while (!encoded[2 + i])
			steam_random(encoded + 2 + i, 1);
	encoded[2 + pad] = 0;
	memcpy(encoded + 3 + pad, message, size);
	return rsa_raw(modulus, modulus_size, exponent, encoded, out);
}

/* ---------- the rest */

uint32_t steam_crc32(void const *data, size_t size)
{
	static uint32_t table[256];
	static int made;
	unsigned char const *bytes = data;
	uint32_t crc = 0xFFFFFFFF;
	size_t i;

	if (!made)
	{
		uint32_t n;

		for (n = 0; n < 256; n++)
		{
			uint32_t value = n;
			int k;

			for (k = 0; k < 8; k++)
				value = (value & 1) ? 0xEDB88320 ^ (value >> 1) : value >> 1;
			table[n] = value;
		}
		made = 1;
	}
	for (i = 0; i < size; i++)
		crc = table[(crc ^ bytes[i]) & 255] ^ (crc >> 8);
	return crc ^ 0xFFFFFFFF;
}

long steam_base64_decode(char const *text, size_t length, unsigned char *out, size_t capacity)
{
	uint32_t bits = 0;
	int count = 0;
	size_t written = 0, i;

	for (i = 0; i < length; i++)
	{
		char ch = text[i];
		int value;

		if (ch >= 'A' && ch <= 'Z') value = ch - 'A';
		else if (ch >= 'a' && ch <= 'z') value = ch - 'a' + 26;
		else if (ch >= '0' && ch <= '9') value = ch - '0' + 52;
		else if (ch == '+' || ch == '-') value = 62;
		else if (ch == '/' || ch == '_') value = 63;
		else if (ch == '=' || ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') continue;
		else return -1;
		bits = (bits << 6) | (uint32_t)value;
		count += 6;
		if (count >= 8)
		{
			count -= 8;
			if (written >= capacity)
				return -1;
			out[written++] = (unsigned char)(bits >> count);
		}
	}
	return (long)written;
}

void steam_random(void *buffer, size_t size)
{
	posix_random_bytes(buffer, (posix_ulong)size);
}

void steam_hex(unsigned char const *bytes, size_t size, char *text)
{
	static char const digits[] = "0123456789abcdef";
	size_t i;

	for (i = 0; i < size; i++)
	{
		text[2 * i] = digits[bytes[i] >> 4];
		text[2 * i + 1] = digits[bytes[i] & 15];
	}
	text[2 * size] = 0;
}

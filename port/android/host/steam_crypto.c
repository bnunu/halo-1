/*
STEAM_CRYPTO.C

steam_crypto.h's, on the host's Mbed TLS (port/third_party/mbedtls): SHA-1,
HMAC-SHA1, AES-256 in Steam's two layouts, and RSA with OAEP (SHA-1) or
PKCS#1 v1.5 padding; CRC-32 by zlib; and a base64 decoder of its own, which
takes the URL-safe alphabet as well (a refresh token's).
*/

#include "steam_crypto.h"
#include "posix.h"
#include "zlib_prefixed.h"

#include "mbedtls/md.h"
#include "mbedtls/rsa.h"

#include <string.h>

/* ---------- SHA-1 */

void steam_sha1_begin(struct steam_sha1 *sha)
{
	mbedtls_sha1_init(&sha->context);
	mbedtls_sha1_starts(&sha->context);
}

void steam_sha1_add(struct steam_sha1 *sha, void const *data, size_t size)
{
	mbedtls_sha1_update(&sha->context, data, size);
}

void steam_sha1_end(struct steam_sha1 *sha, unsigned char digest[20])
{
	mbedtls_sha1_finish(&sha->context, digest);
	mbedtls_sha1_free(&sha->context);
}

void steam_sha1(void const *data, size_t size, unsigned char digest[20])
{
	mbedtls_sha1(data, size, digest);
}

void steam_hmac_sha1(void const *key, size_t key_size, void const *data, size_t size, unsigned char digest[20])
{
	mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), key, key_size, data, size, digest);
}

/* ---------- AES-256 */

void steam_aes_key(struct steam_aes *aes, unsigned char const key[32])
{
	mbedtls_aes_init(&aes->encrypt);
	mbedtls_aes_init(&aes->decrypt);
	mbedtls_aes_setkey_enc(&aes->encrypt, key, 256);
	mbedtls_aes_setkey_dec(&aes->decrypt, key, 256);
}

/* (Mbed TLS's contexts are not const, though a block leaves them as they are) */
void steam_aes_encrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16])
{
	mbedtls_aes_crypt_ecb((mbedtls_aes_context *)&aes->encrypt, MBEDTLS_AES_ENCRYPT, in, out);
}

void steam_aes_decrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16])
{
	mbedtls_aes_crypt_ecb((mbedtls_aes_context *)&aes->decrypt, MBEDTLS_AES_DECRYPT, in, out);
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

/* the connection's vector's HMAC-SHA1: of the three random bytes, then the
plain text (in two parts, to spare a copy of the text) */
static void vector_hmac(unsigned char const key[16], unsigned char const three[3], unsigned char const *data,
	size_t size, unsigned char digest[20])
{
	mbedtls_md_context_t md;

	mbedtls_md_init(&md);
	mbedtls_md_setup(&md, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 1);
	mbedtls_md_hmac_starts(&md, key, 16);
	mbedtls_md_hmac_update(&md, three, 3);
	mbedtls_md_hmac_update(&md, data, size);
	mbedtls_md_hmac_finish(&md, digest);
	mbedtls_md_free(&md);
}

void steam_aes_encrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t size, unsigned char *out)
{
	unsigned char iv[16];
	unsigned char digest[20];

	steam_random(iv + 13, 3);
	vector_hmac(hmac_key, iv + 13, in, size, digest);
	memcpy(iv, digest, 13);
	steam_aes_encrypt_block(aes, iv, out);
	cbc_encrypt(aes, iv, in, size, out + 16);
}

int steam_aes_decrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t in_size, unsigned char *out, size_t *size)
{
	unsigned char iv[16];
	unsigned char digest[20];

	if (in_size < 32 || in_size % 16)
		return 0;
	steam_aes_decrypt_block(aes, in, iv);
	if (!cbc_decrypt(aes, iv, in + 16, in_size - 16, out, size))
		return 0;
	vector_hmac(hmac_key, iv + 13, out, *size, digest);
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

static int random_bytes(void *unused, unsigned char *out, size_t size)
{
	(void)unused;
	steam_random(out, size);
	return 0;
}

/* message encrypted under the key (modulus big-endian), padded as padding
says (MBEDTLS_RSA_PKCS_V21: OAEP with SHA-1); out holds the modulus's size */
static int rsa_encrypt(unsigned char const *modulus, size_t modulus_size, uint32_t exponent, int padding,
	unsigned char const *message, size_t size, unsigned char *out)
{
	mbedtls_rsa_context rsa;
	unsigned char e[4];
	int ok;

	e[0] = (unsigned char)(exponent >> 24);
	e[1] = (unsigned char)(exponent >> 16);
	e[2] = (unsigned char)(exponent >> 8);
	e[3] = (unsigned char)exponent;
	mbedtls_rsa_init(&rsa);
	ok = !mbedtls_rsa_set_padding(&rsa, padding, padding == MBEDTLS_RSA_PKCS_V21 ? MBEDTLS_MD_SHA1 : MBEDTLS_MD_NONE) &&
		!mbedtls_rsa_import_raw(&rsa, modulus, modulus_size, NULL, 0, NULL, 0, NULL, 0, e, sizeof(e)) &&
		!mbedtls_rsa_complete(&rsa) &&
		!mbedtls_rsa_pkcs1_encrypt(&rsa, random_bytes, NULL, size, message, out);
	mbedtls_rsa_free(&rsa);
	return ok;
}

int steam_rsa_encrypt_public_universe(unsigned char const *message, size_t size, unsigned char out[128])
{
	return rsa_encrypt(universe_modulus, sizeof(universe_modulus), 17, MBEDTLS_RSA_PKCS_V21, message, size, out);
}

int steam_rsa_encrypt_pkcs1(unsigned char const *modulus, size_t modulus_size, uint32_t exponent,
	unsigned char const *message, size_t size, unsigned char *out)
{
	while (modulus_size && !modulus[0])
	{
		modulus++;
		modulus_size--;
	}
	return rsa_encrypt(modulus, modulus_size, exponent, MBEDTLS_RSA_PKCS_V15, message, size, out);
}

/* ---------- the rest */

uint32_t steam_crc32(void const *data, size_t size)
{
	return (uint32_t)crc32(crc32(0L, Z_NULL, 0), data, (uInt)size);
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

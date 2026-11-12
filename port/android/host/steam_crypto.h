/*
STEAM_CRYPTO.H

What the Steam client (steam_cm.c, steam_depot.c) needs of cryptography, in
plain C for every build (the guests have no TLS or crypto library of their
own): SHA-1 and HMAC-SHA1, AES-256 (Steam's "vector encrypted alone, then
CBC" layout, with and without the connection's HMAC'd vector), RSA
encryption with OAEP and SHA-1 under the public universe's key (the
connection's handshake), CRC-32 and base64.

None of it protects a secret of this machine's: it speaks Steam's formats.
*/

#ifndef STEAM_CRYPTO_H
#define STEAM_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

/* ---------- SHA-1 */

struct steam_sha1
{
	uint32_t state[5];
	uint64_t length;
	unsigned char block[64];
	unsigned used;
};

void steam_sha1_begin(struct steam_sha1 *sha);
void steam_sha1_add(struct steam_sha1 *sha, void const *data, size_t size);
void steam_sha1_end(struct steam_sha1 *sha, unsigned char digest[20]);
void steam_sha1(void const *data, size_t size, unsigned char digest[20]);
void steam_hmac_sha1(void const *key, size_t key_size, void const *data, size_t size, unsigned char digest[20]);

/* ---------- AES-256 */

struct steam_aes
{
	/* the round keys, encrypting and decrypting (FIPS 197's equivalent
	inverse cipher) */
	uint32_t encrypt[60];
	uint32_t decrypt[60];
};

void steam_aes_key(struct steam_aes *aes, unsigned char const key[32]);
void steam_aes_encrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16]);
void steam_aes_decrypt_block(struct steam_aes const *aes, unsigned char const in[16], unsigned char out[16]);

/* Steam's symmetric layout: the vector encrypted alone (ECB), then the data
CBC with PKCS#7 padding. Decrypting: the plain text's size into *size
(at most size - 16, written to out, which may be in); 0 when malformed */
int steam_aes_decrypt(struct steam_aes const *aes, unsigned char const *in, size_t in_size,
	unsigned char *out, size_t *size);
/* the connection's: the vector is the HMAC-SHA1 (key the session key's
first 16 bytes) of three random bytes and the plain text, its first 13
bytes, then those three; out holds steam_aes_encrypted_size(size) */
size_t steam_aes_encrypted_size(size_t size);
void steam_aes_encrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t size, unsigned char *out);
/* and decrypting, its vector checked: 1, or 0 when it is not the HMAC's */
int steam_aes_decrypt_hmac(struct steam_aes const *aes, unsigned char const hmac_key[16],
	unsigned char const *in, size_t in_size, unsigned char *out, size_t *size);

/* ---------- RSA */

/* the 128-byte encryption, OAEP with SHA-1, of message (at most 86 bytes)
under Steam's public universe's key (SteamKit's KeyDictionary); 1 on success */
int steam_rsa_encrypt_public_universe(unsigned char const *message, size_t size, unsigned char out[128]);

/* RSA with PKCS#1 v1.5 padding under a key Steam gives (its modulus's
bytes, big-endian, and its exponent): out holds modulus_size bytes; 1 on
success (the password sign-in's) */
int steam_rsa_encrypt_pkcs1(unsigned char const *modulus, size_t modulus_size, uint32_t exponent,
	unsigned char const *message, size_t size, unsigned char *out);

/* ---------- the rest */

uint32_t steam_crc32(void const *data, size_t size);
/* text's base64 into out (at most capacity bytes): the byte count, or -1 */
long steam_base64_decode(char const *text, size_t length, unsigned char *out, size_t capacity);
void steam_random(void *buffer, size_t size);
/* 40 lowercase hex digits */
void steam_hex(unsigned char const *bytes, size_t size, char *text);

#endif

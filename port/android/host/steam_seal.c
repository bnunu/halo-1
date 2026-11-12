/*
STEAM_SEAL.C

steam_seal.h's: the Steam sign-in's place and seal on each host.
*/

#include "steam_seal.h"
#include "steam_crypto.h"
#include "steam_net.h"
#include "posix.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__ANDROID__)
#include <SDL3/SDL.h>
#include <jni.h>
#endif

enum
{
	/* the most a token sealed can be (a refresh token is some 600 bytes) */
	SEALED_MAXIMUM = 4096
};

static void hex_into(unsigned char const *bytes, size_t size, char *out)
{
	static char const digits[] = "0123456789abcdef";
	size_t i;

	for (i = 0; i < size; i++)
	{
		out[2 * i] = digits[bytes[i] >> 4];
		out[2 * i + 1] = digits[bytes[i] & 15];
	}
	out[2 * size] = 0;
}

static int digit(char ch)
{
	return ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : -1;
}

/* hex into bytes (at most capacity): the count, or -1 */
static long hex_from(char const *text, unsigned char *out, size_t capacity)
{
	size_t length = strlen(text), i;

	if (length % 2 || length / 2 > capacity)
		return -1;
	for (i = 0; i < length / 2; i++)
	{
		int high = digit(text[2 * i]), low = digit(text[2 * i + 1]);

		if (high < 0 || low < 0)
			return -1;
		out[i] = (unsigned char)(high * 16 + low);
	}
	return (long)(length / 2);
}

FILE *steam_seal_create(char const *path)
{
	int descriptor = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	FILE *file;

	if (descriptor < 0)
		return NULL;
	file = fdopen(descriptor, "w");
	if (!file)
		close(descriptor);
	return file;
}

#if defined(__ANDROID__)

/* ---------- Android: the Keystore's (HaloActivity.steamSeal, steamUnseal,
steamForget: static methods, found on the activity's class, as
host_main.c's calls are, from whatever thread asks; SDL attaches it) */

void steam_seal_folder(char const *root, char *folder, size_t size)
{
	char const *internal = SDL_GetAndroidInternalStoragePath();

	(void)root;
	snprintf(folder, size, "%s/steam", internal ? internal : ".");
	posix_make_directory(folder);
}

/* bytes through one of the activity's static byte[] (byte[]) methods: the
answer's count into out, or -1 */
static long activity_bytes(char const *name, unsigned char const *in, size_t in_size, unsigned char *out,
	size_t capacity)
{
	JNIEnv *env = (JNIEnv *)SDL_GetAndroidJNIEnv();
	jobject activity = env ? (jobject)SDL_GetAndroidActivity() : NULL;
	jclass activity_class;
	jmethodID method;
	jbyteArray argument = NULL, answer = NULL;
	long count = -1;

	if (!env || !activity)
		return -1;
	activity_class = (*env)->GetObjectClass(env, activity);
	method = activity_class ? (*env)->GetStaticMethodID(env, activity_class, name, "([B)[B") : NULL;
	if (method)
		argument = (*env)->NewByteArray(env, (jsize)in_size);
	if (argument)
	{
		(*env)->SetByteArrayRegion(env, argument, 0, (jsize)in_size, (jbyte const *)in);
		answer = (jbyteArray)(*env)->CallStaticObjectMethod(env, activity_class, method, argument);
		/* (the argument may be the token itself: not left in the Java heap
		as it was) */
		{
			jbyte *bytes = (*env)->GetByteArrayElements(env, argument, NULL);

			if (bytes)
			{
				steam_wipe(bytes, in_size);
				(*env)->ReleaseByteArrayElements(env, argument, bytes, 0);
			}
		}
	}
	if ((*env)->ExceptionCheck(env))
	{
		(*env)->ExceptionClear(env);
		answer = NULL;
	}
	if (answer)
	{
		jsize length = (*env)->GetArrayLength(env, answer);

		if (length >= 0 && (size_t)length <= capacity)
		{
			jbyte *bytes = (*env)->GetByteArrayElements(env, answer, NULL);

			if (bytes)
			{
				memcpy(out, bytes, (size_t)length);
				steam_wipe(bytes, (size_t)length);
				(*env)->ReleaseByteArrayElements(env, answer, bytes, 0);
				count = length;
			}
		}
		(*env)->DeleteLocalRef(env, answer);
	}
	if (argument)
		(*env)->DeleteLocalRef(env, argument);
	if (activity_class)
		(*env)->DeleteLocalRef(env, activity_class);
	(*env)->DeleteLocalRef(env, activity);
	return count;
}

int steam_seal(char const *text, char *out, size_t out_size)
{
	static unsigned char sealed[SEALED_MAXIMUM];
	long count;

	out[0] = 0;
	count = activity_bytes("steamSeal", (unsigned char const *)text, strlen(text), sealed, sizeof(sealed));
	if (count <= 0 || out_size < sizeof("keystore:") + 2 * (size_t)count)
		return 0;
	snprintf(out, out_size, "keystore:");
	hex_into(sealed, (size_t)count, out + strlen(out));
	return 1;
}

int steam_unseal(char const *sealed, char *out, size_t out_size)
{
	static unsigned char bytes[SEALED_MAXIMUM];
	static unsigned char plain[SEALED_MAXIMUM];
	long size, count;

	out[0] = 0;
	if (strncmp(sealed, "keystore:", 9) || (size = hex_from(sealed + 9, bytes, sizeof(bytes))) <= 0)
		return 0;
	count = activity_bytes("steamUnseal", bytes, (size_t)size, plain, sizeof(plain));
	if (count <= 0 || (size_t)count >= out_size)
	{
		steam_wipe(plain, sizeof(plain));
		return 0;
	}
	memcpy(out, plain, (size_t)count);
	out[count] = 0;
	steam_wipe(plain, sizeof(plain));
	return 1;
}

void steam_seal_forget(void)
{
	unsigned char none[1];

	activity_bytes("steamForget", none, 0, none, sizeof(none));
}

#else

/* ---------- elsewhere (tools/test_steam_client.py's native build): a key
of a secret of the device's own, the test's */

void steam_seal_folder(char const *root, char *folder, size_t size)
{
	snprintf(folder, size, "%s/steam", root);
	posix_make_directory(folder);
}

/* the device's own secret, digested: 1, or 0 where there is none */
static int device_secret(unsigned char digest[20])
{
	struct steam_sha1 sha;
	int have = 0;

	steam_sha1_begin(&sha);
	steam_sha1_add(&sha, "halo steam seal v1", 18);
#if defined(STEAM_SEAL_TEST)
	{
		/* (tools/test_steam_client.py's native build: a secret
		of the test's choosing) */
		char const *secret = getenv("STEAM_SEAL_TEST_SECRET");

		if (secret && *secret)
		{
			steam_sha1_add(&sha, secret, strlen(secret));
			have = 1;
		}
	}
#endif
	steam_sha1_end(&sha, digest);
	if (!have)
		steam_wipe(digest, 20);
	return have;
}

/* the AES key and the HMAC key, from the device's secret */
static int device_keys(struct steam_aes *aes, unsigned char hmac_key[16])
{
	unsigned char secret[20];
	unsigned char part[20];
	unsigned char key[40];

	if (!device_secret(secret))
		return 0;
	steam_hmac_sha1(secret, sizeof(secret), "aes 1", 5, key);
	steam_hmac_sha1(secret, sizeof(secret), "aes 2", 5, key + 20);
	steam_aes_key(aes, key);
	steam_hmac_sha1(secret, sizeof(secret), "hmac", 4, part);
	memcpy(hmac_key, part, 16);
	steam_wipe(secret, sizeof(secret));
	steam_wipe(part, sizeof(part));
	steam_wipe(key, sizeof(key));
	return 1;
}

int steam_seal(char const *text, char *out, size_t out_size)
{
	static unsigned char sealed[SEALED_MAXIMUM];
	struct steam_aes aes;
	unsigned char hmac_key[16];
	size_t size = strlen(text), sealed_size = steam_aes_encrypted_size(size);

	out[0] = 0;
	if (sealed_size > sizeof(sealed) || out_size < sizeof("device:") + 2 * sealed_size || !device_keys(&aes, hmac_key))
		return 0;
	steam_aes_encrypt_hmac(&aes, hmac_key, (unsigned char const *)text, size, sealed);
	snprintf(out, out_size, "device:");
	hex_into(sealed, sealed_size, out + strlen(out));
	steam_wipe(&aes, sizeof(aes));
	steam_wipe(hmac_key, sizeof(hmac_key));
	return 1;
}

int steam_unseal(char const *sealed, char *out, size_t out_size)
{
	static unsigned char bytes[SEALED_MAXIMUM];
	static unsigned char plain[SEALED_MAXIMUM];
	struct steam_aes aes;
	unsigned char hmac_key[16];
	size_t plain_size = 0;
	long size;
	int ok;

	out[0] = 0;
	if (strncmp(sealed, "device:", 7) || (size = hex_from(sealed + 7, bytes, sizeof(bytes))) <= 0 ||
		!device_keys(&aes, hmac_key))
		return 0;
	ok = steam_aes_decrypt_hmac(&aes, hmac_key, bytes, (size_t)size, plain, &plain_size) && plain_size < out_size;
	if (ok)
	{
		memcpy(out, plain, plain_size);
		out[plain_size] = 0;
	}
	steam_wipe(plain, sizeof(plain));
	steam_wipe(&aes, sizeof(aes));
	steam_wipe(hmac_key, sizeof(hmac_key));
	return ok;
}

void steam_seal_forget(void)
{
	/* (the device's secret is not this program's to forget) */
}

#endif

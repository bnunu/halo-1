/*
STEAM_CM.H

A session with one of Steam's connection servers, as Steam's own client
holds it, over TCP (the "netfilter" servers ISteamDirectory lists): the
channel encrypted with a session key sent under the public universe's RSA
key, then Steam's protocol-buffer messages, AES'd each with an HMAC'd
vector (SteamKit's EnvelopeEncryptedConnection). On it: a QR sign-in the
Steam mobile app approves, signing in with the token that gives (or
anonymously), product information, depot keys, manifest request codes and
Workshop queries.

Every call blocks until Steam answers or a timeout (a worker thread's:
steam_downloads.c), and keeps the connection's heartbeat while it waits. One
thread uses a session at a time.
*/

#ifndef STEAM_CM_H
#define STEAM_CM_H

#include <stddef.h>
#include <stdint.h>

enum
{
	STEAM_APP_MCC = 976730,
	/* Halo: Combat Evolved Anniversary, a DLC of MCC, whose depot holds halo1/ */
	STEAM_APP_HALO1 = 1064221,
	/* EResult's */
	STEAM_RESULT_OK = 1,
	STEAM_RESULT_FAIL = 2,
	STEAM_RESULT_ACCESS_DENIED = 15,
	STEAM_RESULT_EXPIRED = 27,
	STEAM_RESULT_FILE_NOT_FOUND = 9,
	STEAM_TOKEN_SIZE = 1024,
	STEAM_ACCOUNT_SIZE = 64,
	STEAM_URL_SIZE = 256
};

struct steam_cm;

/* the session's last failure, for the player */
char const *steam_cm_error(struct steam_cm const *cm);

/* a connection server found (ISteamDirectory over HTTPS), connected and its
channel encrypted; NULL and why in error */
struct steam_cm *steam_cm_open(char *error, int error_size);
void steam_cm_close(struct steam_cm *cm);
/* whether the connection still stands */
int steam_cm_connected(struct steam_cm const *cm);

/* ---------- signing in */

/* a QR sign-in begun: the address the code shows (it changes: poll), the
seconds between polls */
struct steam_qr
{
	uint64_t client_id;
	unsigned char request_id[64];
	size_t request_id_size;
	float interval;
	char url[STEAM_URL_SIZE];
};

int steam_cm_qr_begin(struct steam_cm *cm, char const *device_name, struct steam_qr *qr);
/* 1 once approved (the refresh token and the account's name written),
0 while waiting (the qr's url may have changed), -1 on failure */
int steam_cm_qr_poll(struct steam_cm *cm, struct steam_qr *qr, char *token, size_t token_size, char *account,
	size_t account_size);

/* a sign-in by account name and password instead (for a device that is
itself the phone with the Steam app): the session begun into qr (its url
empty), Steam's id of the account, and the confirmations it allows (bits:
STEAM_GUARD_*); then steam_cm_qr_poll as for a QR sign-in, after
steam_cm_guard_code when a code is asked for. 1, or 0 with the error said */
enum
{
	STEAM_GUARD_NONE = 1 << 1,
	STEAM_GUARD_EMAIL_CODE = 1 << 2,
	STEAM_GUARD_DEVICE_CODE = 1 << 3,
	STEAM_GUARD_DEVICE_CONFIRMATION = 1 << 4,
	STEAM_GUARD_EMAIL_CONFIRMATION = 1 << 5
};
int steam_cm_password_begin(struct steam_cm *cm, char const *account, char const *password, char const *device_name,
	struct steam_qr *qr, uint64_t *steam_id, unsigned *confirmations);
/* a Steam Guard code typed (the app's, or the email's: email nonzero): the EResult */
int steam_cm_guard_code(struct steam_cm *cm, struct steam_qr const *qr, uint64_t steam_id, char const *code, int email);

/* signs in with a refresh token (machine_id: the bytes of this install's
identity, steam_cm_machine_id's); the EResult (STEAM_RESULT_OK) */
int steam_cm_logon_token(struct steam_cm *cm, char const *token, char const *machine_name,
	unsigned char const *machine_id, size_t machine_id_size);
int steam_cm_logon_anonymous(struct steam_cm *cm);
/* the token (a signed-in session's own) revoked: Steam takes it no more, from
this device or any copy of it. The EResult */
int steam_cm_revoke_token(struct steam_cm *cm, char const *token);
/* the ClientLogon machine_id of an install, from its random seed bytes */
size_t steam_cm_machine_id(unsigned char const seed[16], unsigned char *out, size_t capacity);
uint64_t steam_cm_steam_id(struct steam_cm const *cm);

/* ---------- content */

/* an app's product information, its text (malloc'd: free it); NULL on failure */
char *steam_cm_app_info(struct steam_cm *cm, uint32_t app);
/* a depot's key: the EResult, the key on STEAM_RESULT_OK */
int steam_cm_depot_key(struct steam_cm *cm, uint32_t depot, uint32_t app, unsigned char key[32]);
/* a manifest's request code (0 when Steam gives none) */
uint64_t steam_cm_manifest_code(struct steam_cm *cm, uint32_t app, uint32_t depot, uint64_t manifest);

/* ---------- the Workshop */

enum steam_workshop_sort
{
	STEAM_WORKSHOP_VOTES = 0,
	STEAM_WORKSHOP_NEWEST = 1,
	STEAM_WORKSHOP_TREND = 3,
	STEAM_WORKSHOP_SEARCH = 11,
	STEAM_WORKSHOP_SUBSCRIPTIONS = 12
};

struct steam_workshop_file
{
	uint64_t id;
	/* the item's manifest in the app's Workshop depot (0: none) */
	uint64_t manifest;
	uint64_t size;
	uint32_t updated;
	uint32_t subscriptions;
	char title[96];
	char description[600];
	char preview_url[STEAM_URL_SIZE];
	/* its tags, comma separated */
	char tags[160];
};

/* a page (from 1) of the app's Workshop items with all of the tags (comma
separated; "" for any) and, if search is not empty, matching it: the count
written to files (at most capacity), the total in *total; -1 on failure */
int steam_cm_workshop_query(struct steam_cm *cm, uint32_t app, char const *tags, char const *search,
	enum steam_workshop_sort sort, int page, struct steam_workshop_file *files, int capacity, uint32_t *total);
/* one item's details (its manifest and title, for an id typed in); 1 found */
int steam_cm_workshop_details(struct steam_cm *cm, uint64_t id, struct steam_workshop_file *file);

#endif

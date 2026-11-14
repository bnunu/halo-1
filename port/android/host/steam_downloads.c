/*
STEAM_DOWNLOADS.C

steam_downloads.h's model, in the host (from Halo Content Evolved's
port/linux/src/steam_downloads.c, which ran in the guest). Three threads of
its own do the work: the session's (signing in and out, the Workshop's
pages, asking Steam about the installed packages), the files' (downloads one
at a time, from a queue the screen adds to, removals, and reading what
mcc_maps holds) and the previews'. The connection server is taken in turn
(session_lock).

The screen's calls (the guest's, each frame, through host_steam_*: the end
of this file) read only what those threads left under state_lock: no file
or network is touched there, and the lock is held only to copy.

The session's file (steam_seal.h: the app's private storage): "account=", "seed=" (32 hex
digits: this install's identity, from which the logon's machine id is
made) and "sealed=" (the refresh token a sign-in gave, sealed with a key of
the device's: steam_seal.c). Where it cannot be sealed, the token is not
written at all, and the sign-in lasts until the app closes.
The token never leaves the host: the guest is shown the account's name.
Logging out revokes it with Steam as well as forgetting it here. What was
downloaded is in steam/downloads.txt of the data root: "shared <manifest>"
and "<item id> <manifest>" lines, so an item updated since shows UPDATE.
*/

/* (GCC's -Wformat-truncation: the paths and texts here are cut at their
buffers on purpose, the way snprintf is made) */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
#include "steam_downloads.h"
#include "steam_cm.h"
#include "steam_crypto.h"
#include "steam_depot.h"
#include "steam_net.h"
#include "steam_seal.h"
#include "host_mcc_hash.h"
#include "posix.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

enum
{
	WORKSHOP_PAGE_ITEMS = 20,
	/* the browse list's rows before its items: search, sort and type */
	BROWSE_FILTER_ROWS = 3,
	MAXIMUM_INSTALLED = 512,
	MAXIMUM_PACKAGES = 256,
	/* the preview pictures kept (each PREVIEW_SIDE square, 0xAARRGGBB) */
	PREVIEWS = 12,
	PREVIEW_SIDE = 256,
	/* how often a download's speed is measured */
	SPEED_MILLISECONDS = 1000,
	/* how long a download's error stays on the screen */
	ERROR_SHOWN_MILLISECONDS = 2000,
	STEAM_HALO1_DEPOT = 976733,
	TEXT_SIZE = 128,
	/* the downloads waiting their turn behind the one under way */
	DOWNLOAD_QUEUE = 32
};

/* (the name Steam lists the sign-in under, in the account's devices) */
#define DEVICE_NAME "OpenCE"

enum session_state
{
	SESSION_OUT,
	SESSION_CONNECTING,
	/* a sign-in by name and password waiting: for the Steam app's
	approval, or a Steam Guard code */
	SESSION_GUARD,
	SESSION_SIGNING_IN,
	SESSION_IN,
	SESSION_UNAVAILABLE
};

/* what the screen is asked to type (steam_downloads_text_wanted) */
enum
{
	TEXT_NONE,
	TEXT_SEARCH,
	TEXT_ACCOUNT,
	TEXT_PASSWORD,
	TEXT_CODE
};

static struct shared_file
{
	char const *from;
	char const *folder;
	char const *name;
} const shared_files[] =
{
	{ "halo1/maps/bitmaps.map", "_shared/singleplayer", "bitmaps.map" },
	{ "halo1/maps/custom_edition/bitmaps.map", "_shared/multiplayer", "bitmaps.map" },
	{ "halo1/maps/custom_edition/sounds.map", "_shared/multiplayer", "sounds.map" },
};
#define SHARED_FILES ((int)(sizeof(shared_files) / sizeof(shared_files[0])))

/* the screen's lists: the hub (signed out: the two ways to sign in), and
the hub's three */
enum view
{
	VIEW_HOME,
	VIEW_ACCOUNT,
	VIEW_FILES,
	VIEW_BROWSE
};

static char const *const sort_names[] = { "TRENDING", "MOST SUBSCRIBED", "NEWEST", "TOP RATED" };
static enum steam_workshop_sort const sorts[] =
	{ STEAM_WORKSHOP_TREND, STEAM_WORKSHOP_SUBSCRIPTIONS, STEAM_WORKSHOP_NEWEST, STEAM_WORKSHOP_VOTES };
/* the Workshop's kinds of Halo 1 item, by their tags */
static char const *const type_names[] = { "ALL", "CAMPAIGN", "MULTIPLAYER" };
static char const *const type_tags[] = { "Halo1", "Halo1,Campaign", "Halo1,Multiplayer" };

struct installed
{
	uint64_t id;
	uint64_t manifest;
};

/* a Workshop package in mcc_maps (MCC FILE STATUS' list): its folder, the
item it is, its title, and the item as Steam last told of it (file.id 0:
not asked yet) */
struct package
{
	char folder[64];
	uint64_t id;
	char title[96];
	struct steam_workshop_file file;
};

struct preview
{
	uint64_t id;
	/* 0 asked, 1 here, -1 none to be had; taken: the thread has the ask */
	int state;
	int taken;
	uint32_t *pixels;
	unsigned long used;
};

static struct package scanned_packages[MAXIMUM_PACKAGES];

static struct
{
	pthread_mutex_t state_lock;
	/* each thread's own (one waiter each, rather than a broadcast to
	several), signalled together by wake() */
	pthread_cond_t session_wake;
	pthread_mutex_t session_lock;
	pthread_cond_t files_wake;
	pthread_cond_t preview_wake;
	int threads_started;
	/* (the data root, the guest's, and its mcc_maps, found once) */
	char data_root[1024];
	char root[1024];

	/* the session thread's work */
	int want_resume;
	int want_password;
	int want_sign_out;
	int want_page;
	/* the page asked for by PREVIOUS or NEXT: its first item chosen once it
	has come */
	int page_jump;
	int want_updates;
	int cancel_sign_in;
	char typed_account[TEXT_SIZE];
	char typed_password[TEXT_SIZE];
	char typed_code[TEXT_SIZE];
	int code_typed;
	/* the files thread's: the downloads queued, taken from the front one at
	a time (what: -1 the shared sets, 1 an item), a removal, a look at
	mcc_maps */
	struct queued_download
	{
		int what;
		struct steam_workshop_file item;
	} queue[DOWNLOAD_QUEUE];
	int queue_count;
	int cancel_download;
	char want_remove[64];
	int want_refresh;

	/* what is shown */
	enum session_state state;
	/* (a name-and-password sign-in's: what Steam allows) */
	unsigned confirmations;
	char account[STEAM_ACCOUNT_SIZE];
	uint64_t steam_id;
	char message[256];
	int sort;
	int type;
	char search[TEXT_SIZE];
	int page;
	int loading;
	uint32_t total;
	int item_count;
	struct steam_workshop_file items[WORKSHOP_PAGE_ITEMS];
	/* the download under way: 0 none, -1 shared, 1 item (its id) */
	int downloading;
	uint64_t downloading_id;
	uint64_t done;
	uint64_t size;
	/* the download's speed (bytes a second; 0 until measured), and the
	bytes and time it was last measured from */
	uint64_t speed;
	uint64_t speed_bytes;
	unsigned long speed_clock;
	char downloading_name[96];
	int rescan;
	/* -1 not known, 0 no, 1 yes */
	int owns_halo1;
	struct installed installed[MAXIMUM_INSTALLED];
	int installed_count;
	uint64_t shared_manifest;
	/* what mcc_maps holds, as last looked at */
	int shared_present[SHARED_FILES];
	int files_known;
	struct package packages[MAXIMUM_PACKAGES];
	int package_count;
	/* the previews, and the one the thread is to fetch next */
	struct preview previews[PREVIEWS];
	uint64_t want_preview;
	char want_preview_url[STEAM_URL_SIZE];
	unsigned long preview_clock;
	/* what the screen is to have typed (TEXT_*), once */
	int want_text;
	/* what the threads are doing (shown, with how long), "" when idle */
	char session_activity[96];
	unsigned long session_since;
	char files_activity[96];
	unsigned long files_since;
	/* a download's error, shown for ERROR_SHOWN_MILLISECONDS whatever
	message follows it (a content server's refusal, the next download's
	start), and gone then */
	char held_error[256];
	unsigned long held_until;

	/* the screen's: the list shown, the hub's place kept while another is,
	and the row whose A asked to be pressed again (-1: none) */
	enum view view;
	short first;
	short chosen;
	short home_first;
	short home_chosen;
	int confirm;
	int open;

	/* the session itself (session_lock) */
	struct steam_cm *cm;
	char token[STEAM_TOKEN_SIZE];
	unsigned char seed[16];
	int seeded;
} steam =
{
	.state_lock = PTHREAD_MUTEX_INITIALIZER,
	.session_wake = PTHREAD_COND_INITIALIZER,
	.session_lock = PTHREAD_MUTEX_INITIALIZER,
	.files_wake = PTHREAD_COND_INITIALIZER,
	.preview_wake = PTHREAD_COND_INITIALIZER,
};

/* ---------- files (never under state_lock) */

static void data_path(char *path, size_t size, char const *relative)
{
	snprintf(path, size, "%s/%s", steam.data_root, relative);
}

static void mcc_root_find(char *path, size_t size)
{
	char found[256];

	if (posix_find_entry_case_insensitive(steam.data_root, "mcc_maps", found, sizeof(found)))
		snprintf(path, size, "%s/%s", steam.data_root, found);
	else
		snprintf(path, size, "%s/mcc_maps", steam.data_root);
}

static int path_exists(char const *path, int *directory)
{
	struct posix_file_information information;

	if (posix_stat(path, &information) != 0)
		return 0;
	if (directory)
		*directory = (information.flags & _posix_file_is_directory) != 0;
	return 1;
}

/* the folders of path made, as far as its last '/' (or the whole of it) */
static void make_folders(char const *path, int whole)
{
	char partial[1024];
	size_t i, length = strlen(path);

	if (length >= sizeof(partial))
		return;
	for (i = 1; i <= length; i++)
	{
		if (path[i] == '/' || path[i] == '\\' || (whole && i == length))
		{
			memcpy(partial, path, i);
			partial[i] = 0;
			if (!path_exists(partial, NULL))
				posix_make_directory(partial);
		}
	}
}

/* a folder and what is in it, gone */
static void remove_tree(char const *path)
{
	void *directory = posix_directory_open(path);
	char name[256];

	while (directory && posix_directory_next(directory, name, sizeof(name)))
	{
		char child[1024];
		int is_directory = 0;

		snprintf(child, sizeof(child), "%s/%s", path, name);
		if (!path_exists(child, &is_directory))
			continue;
		if (is_directory)
			remove_tree(child);
		else if (!posix_mcc_share_remove(child, 0))
			steam_log("steam: not removed (a link, or not a managed path): %s", child);
	}
	if (directory)
		posix_directory_close(directory);
	/* (the MCC helpers' refusals stand: a link inside the tree is neither
	followed nor removed, and what it leads to is left alone) */
	if (!posix_mcc_share_remove(path, 1))
		steam_log("steam: not removed (not empty, a link, or not a managed path): %s", path);
}

/* from to to, which must not be there (posix_mcc_share_publish's, whose
refusal of a link or an unmanaged path stands) */
static int move_folder(char const *from, char const *to)
{
	return posix_mcc_share_publish(from, to);
}

/* the session file's path */
static void session_path(char *path, size_t size, char const *name)
{
	char folder[1024];

	steam_seal_folder(steam.data_root, folder, sizeof(folder));
	snprintf(path, size, "%s/%s", folder, name);
}

/* (session_lock held) the account, the seed and the token, sealed where the
device can; no token: the file holds no sign-in */
static void session_save(void)
{
	static char sealed[2 * 4096 + 64];
	char path[1100];
	char part[1100];
	char seed[33];
	FILE *file;
	int is_sealed = 0;

	session_path(path, sizeof(path), "session.txt");
	snprintf(part, sizeof(part), "%s.part", path);
	/* (a token is kept on the device only sealed: where it cannot be, the
	sign-in lasts as long as the app does) */
	if (steam.token[0])
	{
		is_sealed = steam_seal(steam.token, sealed, sizeof(sealed));
		if (!is_sealed)
			steam_log("steam: the sign-in could not be sealed: it is kept for this run alone");
	}
	file = steam_seal_create(part);
	if (!file)
	{
		steam_wipe(sealed, sizeof(sealed));
		steam_log("steam: the sign-in could not be saved");
		return;
	}
	steam_hex(steam.seed, sizeof(steam.seed), seed);
	fprintf(file, "account=%s\nseed=%s\n", steam.account, seed);
	if (is_sealed)
		fprintf(file, "sealed=%s\n", sealed);
	steam_wipe(sealed, sizeof(sealed));
	if (fflush(file) != 0 || fclose(file) != 0)
	{
		remove(part);
		steam_log("steam: the sign-in could not be saved");
		return;
	}
	remove(path);
	if (rename(part, path) != 0)
		steam_log("steam: the sign-in could not be saved");
}

/* (session_lock held) */
static void session_load(void)
{
	static char line[2 * 4096 + 64];
	char path[1100];
	FILE *file;

	session_path(path, sizeof(path), "session.txt");
	file = fopen(path, "r");
	while (file && fgets(line, sizeof(line), file))
	{
		char *value = strchr(line, '=');
		size_t length;

		if (!value)
			continue;
		*value++ = 0;
		length = strcspn(value, "\r\n");
		value[length] = 0;
		if (!strcmp(line, "account"))
			snprintf(steam.account, sizeof(steam.account), "%s", value);
		else if (!strcmp(line, "sealed"))
		{
			if (!steam_unseal(value, steam.token, sizeof(steam.token)))
				steam_log("steam: the saved sign-in is another device's, or its key is gone: sign in again");
		}
		else if (!strcmp(line, "seed") && length == 32)
		{
			int i;

			for (i = 0; i < 16; i++)
			{
				unsigned byte = 0;

				sscanf(value + 2 * i, "%2x", &byte);
				steam.seed[i] = (unsigned char)byte;
			}
			steam.seeded = 1;
		}
		steam_wipe(line, sizeof(line));
	}
	if (file)
		fclose(file);
	if (!steam.seeded)
	{
		steam_random(steam.seed, sizeof(steam.seed));
		steam.seeded = 1;
	}
}

/* (session_lock held) the sign-in forgotten: the token wiped, the file
removed, the device's key let go */
static void session_forget(void)
{
	char path[1100];

	steam_wipe(steam.token, sizeof(steam.token));
	session_path(path, sizeof(path), "session.txt");
	remove(path);
	/* (and one a save left unfinished) */
	session_path(path, sizeof(path), "session.txt.part");
	remove(path);
	steam_seal_forget();
}

/* downloads.txt written from a copy taken under state_lock */
static void installed_save(void)
{
	static struct installed copy[MAXIMUM_INSTALLED];
	char path[1024];
	FILE *file;
	int i, count;
	uint64_t shared;

	pthread_mutex_lock(&steam.state_lock);
	count = steam.installed_count;
	memcpy(copy, steam.installed, sizeof(copy[0]) * (size_t)count);
	shared = steam.shared_manifest;
	pthread_mutex_unlock(&steam.state_lock);
	data_path(path, sizeof(path), "steam");
	make_folders(path, 1);
	data_path(path, sizeof(path), "steam/downloads.txt");
	file = fopen(path, "w");
	if (!file)
		return;
	if (shared)
		fprintf(file, "shared %llu\n", (unsigned long long)shared);
	for (i = 0; i < count; i++)
		fprintf(file, "%llu %llu\n", (unsigned long long)copy[i].id, (unsigned long long)copy[i].manifest);
	fclose(file);
}

/* (before the threads start) */
static void installed_load(void)
{
	char path[1024];
	char line[128];
	FILE *file;

	steam.installed_count = 0;
	data_path(path, sizeof(path), "steam/downloads.txt");
	file = fopen(path, "r");
	while (file && fgets(line, sizeof(line), file))
	{
		unsigned long long first = 0, second = 0;

		if (!strncmp(line, "shared ", 7))
			steam.shared_manifest = strtoull(line + 7, NULL, 10);
		else if (sscanf(line, "%llu %llu", &first, &second) == 2 && steam.installed_count < MAXIMUM_INSTALLED)
		{
			steam.installed[steam.installed_count].id = first;
			steam.installed[steam.installed_count].manifest = second;
			steam.installed_count++;
		}
	}
	if (file)
		fclose(file);
}

/* (state_lock held) */
static void installed_set(uint64_t id, uint64_t manifest)
{
	int i;

	for (i = 0; i < steam.installed_count; i++)
	{
		if (steam.installed[i].id == id)
		{
			steam.installed[i].manifest = manifest;
			return;
		}
	}
	if (steam.installed_count < MAXIMUM_INSTALLED)
	{
		steam.installed[steam.installed_count].id = id;
		steam.installed[steam.installed_count].manifest = manifest;
		steam.installed_count++;
	}
}

/* what mcc_maps holds looked at (the files thread's), then published: the
shared sets' files, and the folders that are Workshop items (named by
their id, or with a workshop_id.txt saying it) */
static void files_refresh(void)
{
	char root[1024];
	char name[256];
	void *directory;
	int present[SHARED_FILES];
	int count = 0, i;

	pthread_mutex_lock(&steam.state_lock);
	snprintf(root, sizeof(root), "%s", steam.root);
	pthread_mutex_unlock(&steam.state_lock);
	for (i = 0; i < SHARED_FILES; i++)
	{
		char path[1200];

		snprintf(path, sizeof(path), "%s/%s/%s", root, shared_files[i].folder, shared_files[i].name);
		present[i] = path_exists(path, NULL);
	}
	directory = posix_directory_open(root);
	while (directory && posix_directory_next(directory, name, sizeof(name)) && count < MAXIMUM_PACKAGES)
	{
		char path[1300];
		char line[160];
		int is_directory = 0;
		struct package *package = &scanned_packages[count];
		FILE *sidecar;
		size_t j;
		int digits = name[0] != 0;

		snprintf(path, sizeof(path), "%s/%s", root, name);
		if (name[0] == '_' || name[0] == '.' || strlen(name) >= sizeof(package->folder) ||
			!path_exists(path, &is_directory) || !is_directory)
			continue;
		memset(package, 0, sizeof(*package));
		snprintf(package->folder, sizeof(package->folder), "%s", name);
		for (j = 0; name[j]; j++)
			digits &= name[j] >= '0' && name[j] <= '9';
		if (digits)
			package->id = strtoull(name, NULL, 10);
		snprintf(path, sizeof(path), "%s/%s/workshop_id.txt", root, name);
		sidecar = fopen(path, "r");
		if (sidecar)
		{
			if (fgets(line, sizeof(line), sidecar))
				package->id = strtoull(line, NULL, 10);
			if (fgets(line, sizeof(line), sidecar))
			{
				line[strcspn(line, "\r\n")] = 0;
				snprintf(package->title, sizeof(package->title), "%s", line);
			}
			fclose(sidecar);
		}
		if (package->id)
			count++;
	}
	if (directory)
		posix_directory_close(directory);
	pthread_mutex_lock(&steam.state_lock);
	memcpy(steam.shared_present, present, sizeof(present));
	/* (what Steam said of each before, kept) */
	for (i = 0; i < count; i++)
	{
		int k;

		for (k = 0; k < steam.package_count; k++)
		{
			if (steam.packages[k].id == scanned_packages[i].id)
			{
				scanned_packages[i].file = steam.packages[k].file;
				if (!scanned_packages[i].title[0])
					memcpy(scanned_packages[i].title, steam.packages[k].title, sizeof(scanned_packages[i].title));
			}
		}
	}
	memcpy(steam.packages, scanned_packages, sizeof(scanned_packages[0]) * (size_t)count);
	steam.package_count = count;
	steam.files_known = 1;
	pthread_mutex_unlock(&steam.state_lock);
	steam_log("steam: mcc_maps: %d of %d shared files, %d Workshop packages", present[0] + present[1] + present[2],
		SHARED_FILES, count);
}

/* ---------- state for the screen (state_lock held where said) */

static void say(char const *format, char const *detail)
{
	pthread_mutex_lock(&steam.state_lock);
	snprintf(steam.message, sizeof(steam.message), format, detail ? detail : "");
	pthread_mutex_unlock(&steam.state_lock);
	steam_log("steam: %s", steam.message);
}

/* a download's error: said, and kept on the screen for
ERROR_SHOWN_MILLISECONDS at least */
static void say_error(char const *format, char const *detail)
{
	say(format, detail);
	pthread_mutex_lock(&steam.state_lock);
	snprintf(steam.held_error, sizeof(steam.held_error), "%s", steam.message);
	steam.held_until = steam_milliseconds() + ERROR_SHOWN_MILLISECONDS;
	pthread_mutex_unlock(&steam.state_lock);
}

/* steam_depot.c's: a content server refused, another is tried after a wait */
static void depot_notice(char const *text)
{
	say_error("%s", text);
}

static void set_state(enum session_state state)
{
	pthread_mutex_lock(&steam.state_lock);
	steam.state = state;
	pthread_mutex_unlock(&steam.state_lock);
}

/* what a thread is doing now ("" for nothing), shown on the screen */
static void activity(int files, char const *what)
{
	pthread_mutex_lock(&steam.state_lock);
	snprintf(files ? steam.files_activity : steam.session_activity, sizeof(steam.files_activity), "%s", what);
	*(files ? &steam.files_since : &steam.session_since) = steam_milliseconds();
	pthread_mutex_unlock(&steam.state_lock);
	if (what[0])
		steam_log("steam: %s", what);
}

/* (state_lock held) the package of an item, or NULL */
static struct package *package_of(uint64_t id)
{
	int i;

	for (i = 0; i < steam.package_count; i++)
		if (steam.packages[i].id == id)
			return &steam.packages[i];
	return NULL;
}

/* (state_lock held) the manifest recorded for an item when it was downloaded here */
static uint64_t recorded_manifest(uint64_t id)
{
	int i;

	for (i = 0; i < steam.installed_count; i++)
		if (steam.installed[i].id == id)
			return steam.installed[i].manifest;
	return 0;
}

/* (state_lock held) 0 not here, 1 here and current, 2 here but older */
static int item_installed(uint64_t id, uint64_t manifest)
{
	uint64_t recorded;

	if (!package_of(id))
		return 0;
	recorded = recorded_manifest(id);
	return recorded && manifest && recorded != manifest ? 2 : 1;
}

/* (state_lock held) */
static int shared_installed(void)
{
	return steam.shared_present[0] && steam.shared_present[1] && steam.shared_present[2];
}

/* ---------- the session (session_lock held) */

static void session_drop(void)
{
	steam_cm_close(steam.cm);
	steam.cm = NULL;
}

/* connected and signed in with the saved token: 1 */
static int session_ensure(char *error, int error_size)
{
	unsigned char machine_id[256];
	size_t machine_id_size;
	int result;

	if (steam.cm && steam_cm_connected(steam.cm))
		return 1;
	session_drop();
	if (!steam.token[0])
	{
		snprintf(error, (size_t)error_size, "Sign in with Steam first.");
		return 0;
	}
	steam.cm = steam_cm_open(error, error_size);
	if (!steam.cm)
		return 0;
	machine_id_size = steam_cm_machine_id(steam.seed, machine_id, sizeof(machine_id));
	result = steam_cm_logon_token(steam.cm, steam.token, DEVICE_NAME, machine_id, machine_id_size);
	if (result != STEAM_RESULT_OK)
	{
		snprintf(error, (size_t)error_size, "%s", steam_cm_error(steam.cm));
		session_drop();
		/* (a token Steam no longer takes: signed out) */
		if (result == STEAM_RESULT_ACCESS_DENIED || result == STEAM_RESULT_EXPIRED || result == 5 || result == 84)
		{
			session_forget();
			set_state(SESSION_OUT);
			snprintf(error, (size_t)error_size, "Steam no longer accepts this sign-in: sign in again.");
		}
		return 0;
	}
	steam_log("steam: signed in");
	pthread_mutex_lock(&steam.state_lock);
	steam.steam_id = steam_cm_steam_id(steam.cm);
	pthread_mutex_unlock(&steam.state_lock);
	return 1;
}

/* a sign-in by the name and password typed, then Steam polled until it
approves (the app's approval, or a code typed meanwhile) */
static void do_sign_in(void)
{
	char error[256];
	char token[STEAM_TOKEN_SIZE];
	char account[STEAM_ACCOUNT_SIZE];
	char typed_account[TEXT_SIZE];
	char password[TEXT_SIZE];
	struct steam_auth auth;
	struct steam_cm *cm;
	uint64_t steam_id = 0;
	unsigned confirmations = 0;
	int begun;

	set_state(SESSION_CONNECTING);
	say("Connecting to Steam...%s", NULL);
	pthread_mutex_lock(&steam.state_lock);
	snprintf(typed_account, sizeof(typed_account), "%s", steam.typed_account);
	snprintf(password, sizeof(password), "%s", steam.typed_password);
	steam_wipe(steam.typed_password, sizeof(steam.typed_password));
	steam.code_typed = 0;
	pthread_mutex_unlock(&steam.state_lock);
	cm = steam_cm_open(error, sizeof(error));
	if (!cm)
	{
		steam_wipe(password, sizeof(password));
		set_state(SESSION_OUT);
		say("%s", error);
		return;
	}
	begun = steam_cm_password_begin(cm, typed_account, password, DEVICE_NAME, &auth, &steam_id, &confirmations);
	steam_wipe(password, sizeof(password));
	if (!begun)
	{
		set_state(SESSION_OUT);
		say("%s", steam_cm_error(cm));
		steam_cm_close(cm);
		return;
	}
	steam_log("steam: signing in by name: confirmations 0x%x", confirmations);
	pthread_mutex_lock(&steam.state_lock);
	steam.state = SESSION_GUARD;
	steam.confirmations = confirmations;
	/* (a code is asked for at once when the app cannot approve it) */
	if (!(confirmations & (STEAM_GUARD_DEVICE_CONFIRMATION | STEAM_GUARD_EMAIL_CONFIRMATION)) &&
		(confirmations & (STEAM_GUARD_DEVICE_CODE | STEAM_GUARD_EMAIL_CODE)))
		steam.want_text = TEXT_CODE;
	steam.message[0] = 0;
	pthread_mutex_unlock(&steam.state_lock);
	for (;;)
	{
		unsigned long until = steam_milliseconds() + (unsigned long)(auth.interval * 1000.0f);
		int result;
		int cancelled = 0;
		int code = 0;
		char typed[TEXT_SIZE];

		while (!cancelled && !code && (long)(until - steam_milliseconds()) > 0)
		{
			struct timespec pause = { 0, 100000000L };

			nanosleep(&pause, NULL);
			pthread_mutex_lock(&steam.state_lock);
			cancelled = steam.cancel_sign_in || !steam.open;
			code = steam.code_typed;
			if (code)
			{
				snprintf(typed, sizeof(typed), "%s", steam.typed_code);
				steam_wipe(steam.typed_code, sizeof(steam.typed_code));
				steam.code_typed = 0;
			}
			pthread_mutex_unlock(&steam.state_lock);
		}
		if (cancelled)
		{
			steam_cm_close(cm);
			set_state(SESSION_OUT);
			say("Signing in was cancelled.%s", NULL);
			return;
		}
		if (code)
		{
			int email = !(confirmations & STEAM_GUARD_DEVICE_CODE) && (confirmations & STEAM_GUARD_EMAIL_CODE);

			int accepted = steam_cm_guard_code(cm, &auth, steam_id, typed, email) == STEAM_RESULT_OK;

			steam_wipe(typed, sizeof(typed));
			if (!accepted)
			{
				pthread_mutex_lock(&steam.state_lock);
				snprintf(steam.message, sizeof(steam.message), "Wrong code: A types it again.");
				pthread_mutex_unlock(&steam.state_lock);
				continue;
			}
		}
		result = steam_cm_auth_poll(cm, &auth, token, sizeof(token), account, sizeof(account));
		if (result < 0)
		{
			say("Sign-in refused or timed out (%s).", steam_cm_error(cm));
			steam_cm_close(cm);
			set_state(SESSION_OUT);
			return;
		}
		if (result > 0)
			break;
	}
	steam_cm_close(cm);
	set_state(SESSION_SIGNING_IN);
	pthread_mutex_lock(&steam.session_lock);
	snprintf(steam.token, sizeof(steam.token), "%s", token);
	steam_wipe(token, sizeof(token));
	pthread_mutex_lock(&steam.state_lock);
	snprintf(steam.account, sizeof(steam.account), "%s", account[0] ? account : typed_account);
	pthread_mutex_unlock(&steam.state_lock);
	session_save();
	session_drop();
	if (!session_ensure(error, sizeof(error)))
	{
		pthread_mutex_unlock(&steam.session_lock);
		set_state(SESSION_OUT);
		say("%s", error);
		return;
	}
	pthread_mutex_unlock(&steam.session_lock);
	pthread_mutex_lock(&steam.state_lock);
	steam.state = SESSION_IN;
	snprintf(steam.message, sizeof(steam.message), "Signed in.");
	pthread_mutex_unlock(&steam.state_lock);
}

static void do_resume(void)
{
	char error[256];
	int ok;

	pthread_mutex_lock(&steam.session_lock);
	if (!steam.token[0])
	{
		pthread_mutex_unlock(&steam.session_lock);
		set_state(SESSION_OUT);
		return;
	}
	set_state(SESSION_SIGNING_IN);
	ok = session_ensure(error, sizeof(error));
	pthread_mutex_unlock(&steam.session_lock);
	pthread_mutex_lock(&steam.state_lock);
	/* (still signed in when Steam could not be reached: tried again with
	the next thing asked of it) */
	steam.state = steam.token[0] ? SESSION_IN : SESSION_OUT;
	snprintf(steam.message, sizeof(steam.message), "%s", ok ? "" : error);
	pthread_mutex_unlock(&steam.state_lock);
	if (!ok)
		steam_log("steam: resuming the sign-in: %s", error);
}

static void do_sign_out(void)
{
	char error[256];
	int revoked = 0;

	pthread_mutex_lock(&steam.session_lock);
	/* (revoked with Steam first, so that no copy of the token is any good
	after; when Steam cannot be reached it is forgotten here all the same,
	and the account's Steam settings can deauthorize it) */
	if (steam.token[0] && session_ensure(error, sizeof(error)))
		revoked = steam_cm_revoke_token(steam.cm, steam.token) == STEAM_RESULT_OK;
	steam_log("steam: logging out: the sign-in %s", revoked ? "revoked with Steam" : "not revoked (Steam not reached)");
	session_drop();
	session_forget();
	pthread_mutex_lock(&steam.state_lock);
	steam.account[0] = 0;
	steam.steam_id = 0;
	steam.item_count = 0;
	steam.total = 0;
	steam.state = SESSION_OUT;
	snprintf(steam.message, sizeof(steam.message), "%s", revoked ? "Logged out, and revoked with Steam." :
		"Logged out here; Steam was out of reach to revoke it (Manage Steam Guard can).");
	pthread_mutex_unlock(&steam.state_lock);
	pthread_mutex_unlock(&steam.session_lock);
	steam_log("steam: logged out");
}

static void do_page(void)
{
	static struct steam_workshop_file files[WORKSHOP_PAGE_ITEMS];
	char error[256];
	char search[TEXT_SIZE];
	uint32_t total = 0;
	int count = -1;
	int page, sort, type;

	pthread_mutex_lock(&steam.state_lock);
	page = steam.page < 1 ? 1 : steam.page;
	sort = steam.sort;
	type = steam.type;
	snprintf(search, sizeof(search), "%s", steam.search);
	steam.loading = 1;
	pthread_mutex_unlock(&steam.state_lock);
	error[0] = 0;
	pthread_mutex_lock(&steam.session_lock);
	if (session_ensure(error, sizeof(error)))
	{
		count = steam_cm_workshop_query(steam.cm, STEAM_APP_MCC, type_tags[type], search, sorts[sort], page, files,
			WORKSHOP_PAGE_ITEMS, &total);
		if (count < 0)
		{
			snprintf(error, sizeof(error), "%s", steam_cm_error(steam.cm));
			session_drop();
		}
	}
	pthread_mutex_unlock(&steam.session_lock);
	steam_log("steam: workshop page %d (%s, %s, \"%s\"): %d items of %u%s%s", page, sort_names[sort],
		type_names[type], search, count, (unsigned)total, count < 0 ? ": " : "", count < 0 ? error : "");
	pthread_mutex_lock(&steam.state_lock);
	steam.loading = 0;
	if (count >= 0)
	{
		int i, kept = 0;

		/* (the items with files: an item without a manifest has none to download) */
		for (i = 0; i < count; i++)
			if (files[i].manifest)
				steam.items[kept++] = files[i];
		steam.item_count = kept;
		steam.total = total;
		steam.message[0] = 0;
		/* (the screen moves its focus to it: steam_update, menu_functions.c) */
		if (steam.page_jump && kept && steam.view == VIEW_BROWSE)
			steam.chosen = BROWSE_FILTER_ROWS;
	}
	else
		snprintf(steam.message, sizeof(steam.message), "The Workshop could not be listed: %s", error);
	steam.page_jump = 0;
	pthread_mutex_unlock(&steam.state_lock);
}

/* Steam asked about each installed package's item: its newest version */
static void do_updates(void)
{
	char error[256];
	int index;

	for (index = 0;; index++)
	{
		uint64_t id = 0;
		struct steam_workshop_file file;
		int found = 0;

		pthread_mutex_lock(&steam.state_lock);
		if (index < steam.package_count)
			id = steam.packages[index].id;
		pthread_mutex_unlock(&steam.state_lock);
		if (!id)
			return;
		pthread_mutex_lock(&steam.session_lock);
		if (session_ensure(error, sizeof(error)))
			found = steam_cm_workshop_details(steam.cm, id, &file);
		pthread_mutex_unlock(&steam.session_lock);
		pthread_mutex_lock(&steam.state_lock);
		if (found)
		{
			struct package *package = package_of(id);

			if (package)
			{
				package->file = file;
				if (!package->title[0])
					snprintf(package->title, sizeof(package->title), "%s", file.title);
			}
		}
		pthread_mutex_unlock(&steam.state_lock);
	}
}

static void *session_thread(void *unused)
{
	(void)unused;
	for (;;)
	{
		int resume, password, sign_out, page, updates;

		pthread_mutex_lock(&steam.state_lock);
		while (!steam.want_resume && !steam.want_password && !steam.want_sign_out &&
			!steam.want_page && !steam.want_updates)
			pthread_cond_wait(&steam.session_wake, &steam.state_lock);
		resume = steam.want_resume;
		password = steam.want_password;
		sign_out = steam.want_sign_out;
		page = steam.want_page;
		updates = steam.want_updates;
		steam.want_resume = steam.want_password = steam.want_sign_out = 0;
		steam.want_page = steam.want_updates = 0;
		steam.cancel_sign_in = 0;
		pthread_mutex_unlock(&steam.state_lock);
		if (sign_out)
		{
			/* (the downloads need the session: the queue dropped, the one
			under way cancelled) */
			pthread_mutex_lock(&steam.state_lock);
			steam.queue_count = 0;
			if (steam.downloading)
				steam.cancel_download = 1;
			pthread_mutex_unlock(&steam.state_lock);
			activity(0, "logging out");
			do_sign_out();
			activity(0, "");
			continue;
		}
		if (password)
			do_sign_in();
		else if (resume)
		{
			activity(0, "signing in with the saved account");
			do_resume();
			activity(0, "");
		}
		pthread_mutex_lock(&steam.state_lock);
		page = page && steam.state == SESSION_IN;
		updates = updates && steam.state == SESSION_IN;
		pthread_mutex_unlock(&steam.state_lock);
		if (page)
		{
			activity(0, "asking Steam for the Workshop's page");
			do_page();
			activity(0, "");
		}
		if (updates)
		{
			activity(0, "asking Steam about the installed items");
			do_updates();
			activity(0, "");
		}
	}
	return NULL;
}

/* ---------- the files thread */

static void download_progress(uint64_t done, void *context)
{
	uint64_t base = *(uint64_t const *)context;

	pthread_mutex_lock(&steam.state_lock);
	steam.done = base + done;
	/* (the speed over each second or more, smoothed) */
	{
		unsigned long now = steam_milliseconds();
		unsigned long elapsed = now - steam.speed_clock;

		if (steam.done < steam.speed_bytes)
		{
			steam.speed_bytes = steam.done;
			steam.speed_clock = now;
		}
		else if (elapsed >= SPEED_MILLISECONDS)
		{
			uint64_t measured = (steam.done - steam.speed_bytes) * 1000 / elapsed;

			steam.speed = steam.speed ? (steam.speed + measured) / 2 : measured;
			steam.speed_bytes = steam.done;
			steam.speed_clock = now;
		}
	}
	pthread_mutex_unlock(&steam.state_lock);
}

/* (state_lock held) a download's start: its speed not yet known */
static void speed_reset(void)
{
	steam.speed = steam.speed_bytes = 0;
	steam.speed_clock = steam_milliseconds();
}

static int download_cancelled(void *context)
{
	int cancelled;

	(void)context;
	pthread_mutex_lock(&steam.state_lock);
	cancelled = steam.cancel_download;
	pthread_mutex_unlock(&steam.state_lock);
	return cancelled;
}

/* a depot's key and a manifest of it, from the session */
static int depot_open_shown(uint32_t depot, uint64_t manifest, unsigned char key[32], struct steam_depot_manifest *files,
	char *error, int error_size, int shown)
{
	uint64_t code = 0;
	int result = STEAM_RESULT_FAIL;

	if (shown)
		activity(1, "asking Steam for the files' key");
	pthread_mutex_lock(&steam.session_lock);
	if (session_ensure(error, error_size))
	{
		result = steam_cm_depot_key(steam.cm, depot, STEAM_APP_MCC, key);
		steam_log("steam: depot %u: key result %d", (unsigned)depot, result);
		if (result == STEAM_RESULT_OK)
			code = steam_cm_manifest_code(steam.cm, STEAM_APP_MCC, depot, manifest);
		if (!code && result == STEAM_RESULT_OK)
			snprintf(error, (size_t)error_size, "%s", steam_cm_error(steam.cm));
		else if (result != STEAM_RESULT_OK)
			snprintf(error, (size_t)error_size, "Steam gives this account no key to these files (result %d): it "
				"must own Halo: The Master Chief Collection%s.", result,
				depot == STEAM_HALO1_DEPOT ? " and Halo: Combat Evolved Anniversary" : "");
	}
	pthread_mutex_unlock(&steam.session_lock);
	if (!code)
		return 0;
	if (shown)
		activity(1, "reading the files' list from a content server");
	return steam_depot_manifest(depot, manifest, code, key, files, error, error_size);
}

static int depot_open(uint32_t depot, uint64_t manifest, unsigned char key[32], struct steam_depot_manifest *files,
	char *error, int error_size)
{
	return depot_open_shown(depot, manifest, key, files, error, error_size, 1);
}

/* the public manifest of a depot of the app's information: 0 if none */
static uint64_t public_manifest(char const *info, uint32_t depot)
{
	char name[32];
	char const *at;
	char const *gid;

	snprintf(name, sizeof(name), "\"%u\"", (unsigned)depot);
	at = strstr(info, name);
	if (!at || !(at = strstr(at, "\"manifests\"")) || !(at = strstr(at, "\"public\"")))
		return 0;
	at += 8;
	while (*at == ' ' || *at == '\t' || *at == '\r' || *at == '\n')
		at++;
	/* ("public" "<gid>", or "public" { "gid" "<gid>" ... }) */
	if (*at == '{')
	{
		gid = strstr(at, "\"gid\"");
		if (!gid)
			return 0;
		at = gid + 5;
		while (*at == ' ' || *at == '\t' || *at == '\r' || *at == '\n')
			at++;
	}
	return *at == '"' ? strtoull(at + 1, NULL, 10) : 0;
}

static void download_shared(void)
{
	char error[256];
	unsigned char key[32];
	struct steam_depot_manifest files;
	char *info = NULL;
	uint64_t manifest = 0;
	uint64_t base = 0, total = 0;
	char root[1024];
	int i;

	error[0] = 0;
	activity(1, "asking Steam for MCC's information");
	pthread_mutex_lock(&steam.session_lock);
	if (session_ensure(error, sizeof(error)))
	{
		info = steam_cm_app_info(steam.cm, STEAM_APP_MCC);
		if (!info)
			snprintf(error, sizeof(error), "%s", steam_cm_error(steam.cm));
	}
	pthread_mutex_unlock(&steam.session_lock);
	if (info)
	{
		manifest = public_manifest(info, STEAM_HALO1_DEPOT);
		free(info);
		steam_log("steam: Halo 1's depot manifest %llu", (unsigned long long)manifest);
		if (!manifest)
			snprintf(error, sizeof(error), "Steam's information names no files of Halo: CE Anniversary.");
	}
	if (!manifest || !depot_open(STEAM_HALO1_DEPOT, manifest, key, &files, error, sizeof(error)))
	{
		pthread_mutex_lock(&steam.state_lock);
		if (strstr(error, "no key"))
			steam.owns_halo1 = 0;
		pthread_mutex_unlock(&steam.state_lock);
		say_error("The shared files could not be downloaded: %s", error);
		return;
	}
	pthread_mutex_lock(&steam.state_lock);
	steam.owns_halo1 = 1;
	snprintf(root, sizeof(root), "%s", steam.root);
	pthread_mutex_unlock(&steam.state_lock);
	for (i = 0; i < SHARED_FILES; i++)
	{
		struct steam_depot_file const *file = steam_depot_find(&files, shared_files[i].from);

		if (!file)
		{
			steam_depot_manifest_free(&files);
			say("Halo: CE Anniversary's files have no %s.", shared_files[i].from);
			return;
		}
		total += file->size;
	}
	pthread_mutex_lock(&steam.state_lock);
	steam.size = total;
	steam.done = 0;
	speed_reset();
	pthread_mutex_unlock(&steam.state_lock);
	for (i = 0; i < SHARED_FILES; i++)
	{
		struct steam_depot_file const *file = steam_depot_find(&files, shared_files[i].from);
		struct steam_depot_progress progress;
		char path[1200];

		snprintf(path, sizeof(path), "%s/%s/%s", root, shared_files[i].folder, shared_files[i].name);
		make_folders(path, 0);
		pthread_mutex_lock(&steam.state_lock);
		snprintf(steam.downloading_name, sizeof(steam.downloading_name), "%s", shared_files[i].from);
		pthread_mutex_unlock(&steam.state_lock);
		progress.progress = download_progress;
		progress.cancelled = download_cancelled;
		progress.context = &base;
		if (!steam_depot_download(STEAM_HALO1_DEPOT, key, file, path, &progress, error, sizeof(error)))
		{
			steam_depot_manifest_free(&files);
			(strcmp(error, "cancelled") ? say_error : say)(strcmp(error, "cancelled") ?
				"The shared files could not be downloaded: %s" : "The download was %s.", error);
			return;
		}
		base += file->size;
	}
	steam_depot_manifest_free(&files);
	pthread_mutex_lock(&steam.state_lock);
	steam.shared_manifest = manifest;
	steam.rescan = 1;
	pthread_mutex_unlock(&steam.state_lock);
	installed_save();
	say("MCC's shared files are in mcc_maps/_shared.%s", NULL);
}

/* where a file of an item goes in its package: its path there
(lowercase), or 0 when the package does not keep it */
static int item_path(char const *name, char *path, size_t size)
{
	char lower[260];
	char const *leaf;
	size_t length = strlen(name), i;
	int map;

	if (!length || length >= sizeof(lower))
		return 0;
	for (i = 0; i <= length; i++)
		lower[i] = name[i] >= 'A' && name[i] <= 'Z' ? (char)(name[i] + 32) : name[i];
	/* (no part a path could climb or hide with) */
	if (lower[0] == '/' || strstr(lower, "..") || strstr(lower, "//") || strchr(lower, ':'))
		return 0;
	map = length > 4 && !strcmp(lower + length - 4, ".map");
	if (map)
	{
		/* (a level's or resource's map: of maps/ or halo1/maps/, at the
		package's top) */
		if (!strncmp(lower, "maps/", 5))
			leaf = lower + 5;
		else if (!strncmp(lower, "halo1/maps/", 11))
			leaf = lower + 11;
		else
			return 0;
		if (strchr(leaf, '/') || !*leaf)
			return 0;
		snprintf(path, size, "%s", leaf);
		return 1;
	}
	if ((length > 5 && !strcmp(lower + length - 5, ".json")) || (length > 4 && !strcmp(lower + length - 4, ".png")))
	{
		snprintf(path, size, "%s", lower);
		return 1;
	}
	return 0;
}

static void fingerprint_progress(unsigned long long done, unsigned long long total, void *context);

static int download_item(struct steam_workshop_file const *item)
{
	char error[256];
	unsigned char key[32];
	struct steam_depot_manifest files;
	char staging[1024];
	char root[1024];
	char destination[1100];
	char old[1100];
	char folder[64];
	char existing[64];
	uint64_t base = 0, total = 0;
	int i, maps = 0;
	FILE *sidecar;

	steam_log("steam: downloading item %llu (%s)", (unsigned long long)item->id, item->title);
	if (!depot_open(STEAM_APP_MCC, item->manifest, key, &files, error, sizeof(error)))
	{
		say_error("The item could not be downloaded: %s", error);
		return 0;
	}
	for (i = 0; i < files.file_count; i++)
	{
		char path[260];

		if (!files.files[i].directory && item_path(files.files[i].name, path, sizeof(path)))
		{
			total += files.files[i].size;
			maps += strstr(path, ".map") != NULL;
		}
	}
	if (!maps)
	{
		steam_depot_manifest_free(&files);
		say("This item has no Halo 1 maps to install.%s", NULL);
		return 0;
	}
	snprintf(folder, sizeof(folder), "%llu", (unsigned long long)item->id);
	pthread_mutex_lock(&steam.state_lock);
	snprintf(root, sizeof(root), "%s", steam.root);
	/* (a package of the item already here, under whatever folder name) */
	existing[0] = 0;
	if (package_of(item->id))
		snprintf(existing, sizeof(existing), "%s", package_of(item->id)->folder);
	steam.size = total;
	steam.done = 0;
	speed_reset();
	pthread_mutex_unlock(&steam.state_lock);
	data_path(staging, sizeof(staging), "mcc_staging/steam_");
	strncat(staging, folder, sizeof(staging) - strlen(staging) - 1);
	remove_tree(staging);
	make_folders(staging, 1);
	for (i = 0; i < files.file_count; i++)
	{
		struct steam_depot_file const *file = &files.files[i];
		struct steam_depot_progress progress;
		char relative[260];
		char path[1300];

		if (file->directory || !item_path(file->name, relative, sizeof(relative)))
			continue;
		snprintf(path, sizeof(path), "%s/%s", staging, relative);
		make_folders(path, 0);
		pthread_mutex_lock(&steam.state_lock);
		snprintf(steam.downloading_name, sizeof(steam.downloading_name), "%s", relative);
		pthread_mutex_unlock(&steam.state_lock);
		progress.progress = download_progress;
		progress.cancelled = download_cancelled;
		progress.context = &base;
		if (!steam_depot_download(STEAM_APP_MCC, key, file, path, &progress, error, sizeof(error)))
		{
			steam_depot_manifest_free(&files);
			remove_tree(staging);
			(strcmp(error, "cancelled") ? say_error : say)(strcmp(error, "cancelled") ?
				"The item could not be downloaded: %s" : "The download was %s.", error);
			return 0;
		}
		base += file->size;
	}
	steam_depot_manifest_free(&files);
	/* (the item's id and title, as the browser's importer writes them:
	mcc_workshop.c's sidecar) */
	snprintf(destination, sizeof(destination), "%s/workshop_id.txt", staging);
	sidecar = fopen(destination, "w");
	if (sidecar)
	{
		char title[41];
		size_t length = 0;
		char const *at;

		for (at = item->title; *at && length < sizeof(title) - 1; at++)
			if (*at >= ' ' && *at <= '~')
				title[length++] = *at;
		title[length] = 0;
		fprintf(sidecar, "%s\n%s\n", folder, title);
		fclose(sidecar);
	}
	/* in place: an older copy moved aside first, then removed */
	make_folders(root, 1);
	snprintf(old, sizeof(old), "%s.old", staging);
	remove_tree(old);
	if (existing[0])
	{
		char previous[1100];

		snprintf(previous, sizeof(previous), "%s/%s", root, existing);
		if (!move_folder(previous, old))
		{
			remove_tree(staging);
			say_error("The older copy in mcc_maps/%s could not be replaced.", existing);
			return 0;
		}
	}
	snprintf(destination, sizeof(destination), "%s/%s", root, folder);
	if (!move_folder(staging, destination))
	{
		if (existing[0])
		{
			char previous[1100];

			snprintf(previous, sizeof(previous), "%s/%s", root, existing);
			move_folder(old, previous);
		}
		remove_tree(staging);
		say_error("The item could not be put in mcc_maps/%s.", folder);
		return 0;
	}
	remove_tree(old);
	pthread_mutex_lock(&steam.state_lock);
	installed_set(item->id, item->manifest);
	steam.rescan = 1;
	pthread_mutex_unlock(&steam.state_lock);
	installed_save();
	/* (its maps' fingerprints now, kept on the card: the lobby then takes
	them at once, host_mcc_hash.c) */
	{
		int fingerprinted;

		activity(1, "fingerprinting the maps");
		fingerprinted = host_mcc_hash_package(steam.data_root, destination, &steam.cancel_download,
			fingerprint_progress, NULL);
		activity(1, "");
		if (fingerprinted < 0)
			steam_log("steam: %s's maps were not fingerprinted", folder);
	}
	say("Installed in mcc_maps/%s.", folder);
	return 1;
}

/* host_mcc_hash_package's progress, on the files thread's line (not logged) */
static void fingerprint_progress(unsigned long long done, unsigned long long total, void *context)
{
	static unsigned shown = 101;
	unsigned percent = total ? (unsigned)(done * 100 / total) : 0;

	(void)context;
	if (percent == shown)
		return;
	shown = percent;
	pthread_mutex_lock(&steam.state_lock);
	snprintf(steam.files_activity, sizeof(steam.files_activity), "fingerprinting the maps: %u%%", percent);
	pthread_mutex_unlock(&steam.state_lock);
}

static void remove_package(char const *folder)
{
	char root[1024];
	char path[1100];
	uint64_t id = 0;
	int i;

	pthread_mutex_lock(&steam.state_lock);
	snprintf(root, sizeof(root), "%s", steam.root);
	for (i = 0; i < steam.package_count; i++)
		if (!strcmp(steam.packages[i].folder, folder))
			id = steam.packages[i].id;
	pthread_mutex_unlock(&steam.state_lock);
	snprintf(path, sizeof(path), "%s/%s", root, folder);
	remove_tree(path);
	pthread_mutex_lock(&steam.state_lock);
	for (i = 0; i < steam.installed_count; i++)
	{
		if (steam.installed[i].id == id)
		{
			steam.installed[i] = steam.installed[--steam.installed_count];
			break;
		}
	}
	steam.rescan = 1;
	pthread_mutex_unlock(&steam.state_lock);
	installed_save();
	say("Removed mcc_maps/%s.", folder);
}

static void *files_thread(void *unused)
{
	(void)unused;
	for (;;)
	{
		int what, refresh;
		char remove_folder[64];
		struct steam_workshop_file item;

		pthread_mutex_lock(&steam.state_lock);
		while (!steam.queue_count && !steam.want_remove[0] && !steam.want_refresh)
			pthread_cond_wait(&steam.files_wake, &steam.state_lock);
		what = 0;
		if (steam.queue_count)
		{
			what = steam.queue[0].what;
			item = steam.queue[0].item;
			steam.queue_count--;
			memmove(&steam.queue[0], &steam.queue[1], sizeof(steam.queue[0]) * (size_t)steam.queue_count);
		}
		snprintf(remove_folder, sizeof(remove_folder), "%s", steam.want_remove);
		refresh = steam.want_refresh;
		steam.want_remove[0] = 0;
		steam.want_refresh = 0;
		if (what)
		{
			steam.cancel_download = 0;
			steam.downloading = what;
			steam.downloading_id = what > 0 ? item.id : 0;
			steam.done = steam.size = 0;
			steam.downloading_name[0] = 0;
		}
		pthread_mutex_unlock(&steam.state_lock);
		if (what < 0)
		{
			activity(1, "downloading the shared files");
			download_shared();
		}
		else if (what > 0)
		{
			activity(1, "downloading a Workshop item");
			download_item(&item);
		}
		if (remove_folder[0])
		{
			activity(1, "removing a package");
			remove_package(remove_folder);
		}
		pthread_mutex_lock(&steam.state_lock);
		steam.downloading = 0;
		steam.downloading_id = 0;
		pthread_mutex_unlock(&steam.state_lock);
		if (what || remove_folder[0] || refresh)
		{
			activity(1, "looking at mcc_maps");
			files_refresh();
		}
		activity(1, "");
	}
	return NULL;
}


/* ---------- a joiner's install of an MCC package from the Workshop

A player joining a game on an MCC map it lacks, which is a Workshop item,
with a Steam sign-in: the item installed whole in mcc_maps on a thread of its
own (port/linux/game/mcc_share.c's mcc_share_install_begin), which the guest
then checks against the host's identity. */

/* (state_lock) the install asked, its item, and how it went (0 under way,
1 installed, -1 not) */
static struct
{
	pthread_cond_t wake;
	int install_want;
	uint64_t install_id;
	int install_state;
} fetch = { .wake = PTHREAD_COND_INITIALIZER };

/* a whole Workshop item installed in mcc_maps, as the screen's DOWNLOAD
installs it (download_item: its maps fingerprinted then too) */
static void install_run(uint64_t id)
{
	char error[256];
	struct steam_workshop_file item;
	int found = 0, have, installed;

	pthread_mutex_lock(&steam.session_lock);
	have = steam.token[0] != 0;
	error[0] = 0;
	memset(&item, 0, sizeof(item));
	if (have && session_ensure(error, sizeof(error)))
		found = steam_cm_workshop_details(steam.cm, id, &item);
	pthread_mutex_unlock(&steam.session_lock);
	if (!have || !found || !item.manifest)
	{
		steam_log("steam install: item %llu: %s", (unsigned long long)id, !have ? "no Steam sign-in" :
			error[0] ? error : "not found on the Workshop");
		pthread_mutex_lock(&steam.state_lock);
		fetch.install_state = -1;
		pthread_mutex_unlock(&steam.state_lock);
		return;
	}
	pthread_mutex_lock(&steam.state_lock);
	steam.cancel_download = 0;
	/* (its progress from nothing: host_steam_install_progress) */
	steam.done = steam.size = 0;
	speed_reset();
	pthread_mutex_unlock(&steam.state_lock);
	/* (by what download_item says, not item_installed: the screen's list of
	packages, which that asks, is looked at again only by the files thread;
	nor by the item's record alone, which a folder removed by hand leaves) */
	installed = download_item(&item);
	pthread_mutex_lock(&steam.state_lock);
	fetch.install_state = installed ? 1 : -1;
	steam.rescan = 1;
	pthread_mutex_unlock(&steam.state_lock);
	steam_log("steam install: item %llu (%s): %s", (unsigned long long)id, item.title,
		installed ? "installed" : "not installed");
}

static void *fetch_thread(void *unused)
{
	(void)unused;
	for (;;)
	{
		uint64_t install_id;

		pthread_mutex_lock(&steam.state_lock);
		while (!fetch.install_want)
			pthread_cond_wait(&fetch.wake, &steam.state_lock);
		install_id = fetch.install_id;
		fetch.install_want = 0;
		pthread_mutex_unlock(&steam.state_lock);
		install_run(install_id);
	}
	return NULL;
}

/* ---------- previews (the Workshop's pictures, decoded by Android:
HaloActivity.steamPreview) */

/* (state_lock held) the kept preview of an item, or NULL */
static struct preview *preview_find(uint64_t id)
{
	int i;

	for (i = 0; i < PREVIEWS; i++)
		if (steam.previews[i].id == id)
			return &steam.previews[i];
	return NULL;
}

/* a picture's bytes, fitted to PREVIEW_SIDE square on black (malloc'd), or NULL */
static uint32_t *preview_decode(unsigned char const *data, unsigned long size)
{
#if defined(__ANDROID__)
	uint32_t *pixels = malloc(sizeof(*pixels) * PREVIEW_SIDE * PREVIEW_SIDE);

	if (pixels && steam_activity_bytes("steamPreview", data, size, (unsigned char *)pixels,
		sizeof(*pixels) * PREVIEW_SIDE * PREVIEW_SIDE) == (long)(sizeof(*pixels) * PREVIEW_SIDE * PREVIEW_SIDE))
	{
		return pixels;
	}
	free(pixels);
	return NULL;
#else
	(void)data;
	(void)size;
	return NULL;
#endif
}

static void *preview_thread(void *unused)
{
	struct steam_http http;

	(void)unused;
	steam_http_init(&http);
	for (;;)
	{
		uint64_t id;
		char url[STEAM_URL_SIZE + 128];
		unsigned char *body = NULL;
		unsigned long size = 0;
		int status = 0;
		char error[256];
		uint32_t *pixels = NULL;
		struct preview *preview;
		char host[128];
		char const *path;

		pthread_mutex_lock(&steam.state_lock);
		while (!steam.want_preview)
			pthread_cond_wait(&steam.preview_wake, &steam.state_lock);
		id = steam.want_preview;
		/* (Steam's image server fits the picture to a square itself) */
		snprintf(url, sizeof(url), "%s%cimw=%d&imh=%d&ima=fit&impolicy=Letterbox&imcolor=%%23000000&letterbox=true",
			steam.want_preview_url, strchr(steam.want_preview_url, '?') ? '&' : '?', PREVIEW_SIDE, PREVIEW_SIDE);
		steam.want_preview = 0;
		preview = preview_find(id);
		if (preview)
			preview->taken = 1;
		pthread_mutex_unlock(&steam.state_lock);
		/* (over a kept HTTP connection: a picture is no secret, and a TLS
		handshake each would take seconds on a phone) */
		host[0] = 0;
		path = NULL;
		if (!strncmp(url, "https://", 8) || !strncmp(url, "http://", 7))
		{
			char const *start = strstr(url, "//") + 2;
			char const *slash = strchr(start, '/');

			if (slash && slash - start < (long)sizeof(host))
			{
				memcpy(host, start, (size_t)(slash - start));
				host[slash - start] = 0;
				path = slash;
			}
		}
		if (path && steam_http_get(&http, host, path, 2 * 1024 * 1024, &body, &size, &status, error, sizeof(error)) &&
			status == 200)
			pixels = preview_decode(body, size);
		else if (!strncmp(url, "https://", 8))
		{
			free(body);
			body = NULL;
			if (steam_https_get(url, 2 * 1024 * 1024, &body, &size, &status, error, sizeof(error)) && status == 200)
				pixels = preview_decode(body, size);
		}
		free(body);
		if (!pixels)
			steam_log("steam: preview of %llu: none (%d)", (unsigned long long)id, status);
		pthread_mutex_lock(&steam.state_lock);
		preview = preview_find(id);
		if (preview)
		{
			free(preview->pixels);
			preview->pixels = pixels;
			preview->state = pixels ? 1 : -1;
		}
		else
			free(pixels);
		pthread_mutex_unlock(&steam.state_lock);
	}
	return NULL;
}

/* (state_lock held) the preview of an item asked for, if not had or asked */
static void preview_want(struct steam_workshop_file const *file)
{
	struct preview *preview;
	int i, oldest = 0;

	if (!file->preview_url[0])
		return;
	preview = preview_find(file->id);
	if (preview)
	{
		preview->used = ++steam.preview_clock;
		/* (here, none to be had, being fetched, or asked for already) */
		if (preview->state || preview->taken || steam.want_preview == file->id)
			return;
	}
	else
	{
		for (i = 1; i < PREVIEWS; i++)
			if (steam.previews[i].used < steam.previews[oldest].used)
				oldest = i;
		preview = &steam.previews[oldest];
		free(preview->pixels);
		memset(preview, 0, sizeof(*preview));
		preview->id = file->id;
		preview->used = ++steam.preview_clock;
	}
	steam.want_preview = file->id;
	snprintf(steam.want_preview_url, sizeof(steam.want_preview_url), "%s", file->preview_url);
	pthread_cond_signal(&steam.preview_wake);
}

/* ---------- the screen's (the game thread's: memory alone) */

static void wake(void)
{
	pthread_cond_signal(&steam.session_wake);
	pthread_cond_signal(&steam.files_wake);
	pthread_cond_signal(&steam.preview_wake);
}

/* (state_lock held) */
static void view_open(enum view view)
{
	if (steam.view == VIEW_HOME && view != VIEW_HOME)
	{
		steam.home_first = steam.first;
		steam.home_chosen = steam.chosen;
	}
	steam.first = view == VIEW_HOME ? steam.home_first : 0;
	steam.chosen = view == VIEW_HOME ? steam.home_chosen : 0;
	steam.view = view;
	steam.confirm = -1;
	if (view == VIEW_FILES)
	{
		steam.want_refresh = 1;
		steam.want_updates = 1;
	}
	if (view == VIEW_BROWSE && !steam.item_count)
		steam.want_page = 1;
	wake();
}

static void *fetch_thread(void *unused);

/* the threads started and the saved sign-in read, once: by the screen's
opening, or by a joiner's fetch of an MCC package's resources (below) */
static void threads_start(char const *data_root)
{
	int start;

	pthread_mutex_lock(&steam.state_lock);
	start = !steam.threads_started;
	steam.threads_started = 1;
	pthread_mutex_unlock(&steam.state_lock);
	if (start)
	{
		pthread_t thread;
		char root[1024];

		steam_depot_set_notice(depot_notice);
		/* (once, before the threads: the files read here are the threads' after) */
		snprintf(steam.data_root, sizeof(steam.data_root), "%s", data_root);
		mcc_root_find(root, sizeof(root));
		pthread_mutex_lock(&steam.session_lock);
		session_load();
		pthread_mutex_unlock(&steam.session_lock);
		pthread_mutex_lock(&steam.state_lock);
		snprintf(steam.root, sizeof(steam.root), "%s", root);
		installed_load();
		steam.owns_halo1 = -1;
		steam.page = 1;
		pthread_mutex_unlock(&steam.state_lock);
		if (pthread_create(&thread, NULL, session_thread, NULL) == 0)
			pthread_detach(thread);
		if (pthread_create(&thread, NULL, files_thread, NULL) == 0)
			pthread_detach(thread);
		if (pthread_create(&thread, NULL, preview_thread, NULL) == 0)
			pthread_detach(thread);
		if (pthread_create(&thread, NULL, fetch_thread, NULL) == 0)
			pthread_detach(thread);
	}
}

void steam_downloads_open(char const *data_root)
{
	threads_start(data_root);
	pthread_mutex_lock(&steam.state_lock);
	steam.open = 1;
	steam.view = VIEW_HOME;
	steam.confirm = -1;
	steam.first = steam.chosen = steam.home_first = steam.home_chosen = 0;
	if (steam.state == SESSION_OUT && steam.token[0])
		steam.want_resume = 1;
	steam.want_refresh = 1;
	wake();
	pthread_mutex_unlock(&steam.state_lock);
}

void steam_downloads_close(void)
{
	pthread_mutex_lock(&steam.state_lock);
	steam.open = 0;
	steam.cancel_sign_in = 1;
	steam.want_text = TEXT_NONE;
	pthread_mutex_unlock(&steam.state_lock);
}


/* what a row is */
enum row_kind
{
	ROW_NONE,
	ROW_SIGN_IN_ACCOUNT,
	ROW_HOME_ACCOUNT,
	ROW_HOME_FILES,
	ROW_HOME_BROWSE,
	ROW_ACCOUNT_NAME,
	ROW_LOG_OUT,
	ROW_SHARED,
	ROW_PACKAGE,
	ROW_SEARCH,
	ROW_SORT,
	ROW_TYPE,
	ROW_ITEM,
	ROW_PREVIOUS,
	ROW_NEXT
};

/* (state_lock held) the row at a place of the list shown; *index the
package's or item's */
static enum row_kind row_kind(int place, int *index)
{
	*index = -1;
	if (place < 0)
		return ROW_NONE;
	switch (steam.view)
	{
	case VIEW_HOME:
		if (steam.state != SESSION_IN)
			return place == 0 ? ROW_SIGN_IN_ACCOUNT : ROW_NONE;
		return place == 0 ? ROW_HOME_ACCOUNT : place == 1 ? ROW_HOME_FILES : place == 2 ? ROW_HOME_BROWSE : ROW_NONE;
	case VIEW_ACCOUNT:
		return place == 0 ? ROW_ACCOUNT_NAME : place == 1 ? ROW_LOG_OUT : ROW_NONE;
	case VIEW_FILES:
		if (place == 0)
			return ROW_SHARED;
		if (place - 1 < steam.package_count)
		{
			*index = place - 1;
			return ROW_PACKAGE;
		}
		return ROW_NONE;
	case VIEW_BROWSE:
		if (place == 0)
			return ROW_SEARCH;
		if (place == 1)
			return ROW_SORT;
		if (place == 2)
			return ROW_TYPE;
		place -= BROWSE_FILTER_ROWS;
		if (place < steam.item_count)
		{
			*index = place;
			return ROW_ITEM;
		}
		place -= steam.item_count;
		if (steam.page > 1 && place-- == 0)
			return ROW_PREVIOUS;
		if ((uint32_t)steam.page * WORKSHOP_PAGE_ITEMS < steam.total && place == 0)
			return ROW_NEXT;
		return ROW_NONE;
	}
	return ROW_NONE;
}

/* (state_lock held) */
static int rows_count(void)
{
	switch (steam.view)
	{
	case VIEW_HOME:
		return steam.state == SESSION_IN ? 3 : 1;
	case VIEW_ACCOUNT: return 2;
	case VIEW_FILES: return 1 + steam.package_count;
	case VIEW_BROWSE:
		return BROWSE_FILTER_ROWS + steam.item_count + (steam.page > 1) +
			((uint32_t)steam.page * WORKSHOP_PAGE_ITEMS < steam.total);
	}
	return 0;
}


static void size_text(uint64_t bytes, char *text, int size)
{
	if (bytes >= 1024ull * 1024 * 1024)
		snprintf(text, (size_t)size, "%.1f GB", (double)bytes / (1024.0 * 1024 * 1024));
	else if (bytes >= 1024 * 1024)
		snprintf(text, (size_t)size, "%.0f MB", (double)bytes / (1024.0 * 1024));
	else
		snprintf(text, (size_t)size, "%.0f KB", (double)bytes / 1024.0);
}

static void percent_text(char *text, int size)
{
	unsigned percent = steam.size ? (unsigned)(steam.done * 100 / steam.size) : 0;

	if (!steam.size)
		snprintf(text, (size_t)size, "STARTING");
	else
		snprintf(text, (size_t)size, "%u%%", percent > 100 ? 100 : percent);
}

static void progress_text(char *text, int size)
{
	char done[32], total[32];

	if (!steam.size)
	{
		snprintf(text, (size_t)size, "Asking Steam... A cancels.");
		return;
	}
	size_text(steam.done, done, sizeof(done));
	size_text(steam.size, total, sizeof(total));
	if (steam.speed)
	{
		char speed[32];

		/* (to a tenth of a megabyte) */
		if (steam.speed >= 1024 * 1024)
			snprintf(speed, sizeof(speed), "%.1f MB", (double)steam.speed / (1024.0 * 1024));
		else
			snprintf(speed, sizeof(speed), "%.0f KB", (double)steam.speed / 1024.0);
		snprintf(text, (size_t)size, "%s of %s (%s/s). A cancels.", done, total, speed);
	}
	else
		snprintf(text, (size_t)size, "%s of %s. A cancels.", done, total);
	if (steam.queue_count)
	{
		size_t used = strlen(text);

		snprintf(text + used, (size_t)size - used, " %d queued.", steam.queue_count);
	}
}

/* (state_lock held) the place in the queue (from 0) of the shared sets'
download (what -1) or an item's (1, id), or -1 */
static int queue_find(int what, uint64_t id)
{
	int i;

	for (i = 0; i < steam.queue_count; i++)
		if (steam.queue[i].what == what && (what < 0 || steam.queue[i].item.id == id))
			return i;
	return -1;
}

/* (state_lock held) a queued download taken out of the queue */
static void queue_remove(int place)
{
	steam.queue_count--;
	memmove(&steam.queue[place], &steam.queue[place + 1],
		sizeof(steam.queue[0]) * (size_t)(steam.queue_count - place));
	snprintf(steam.message, sizeof(steam.message), "Taken out of the download queue.");
}

/* (state_lock held) the body of a queued download's details */
static void queued_text(int place, char *text, int size)
{
	snprintf(text, (size_t)size, "Queued, %d of %d. A takes it out.", place + 1, steam.queue_count);
}

/* (state_lock held) the state shown of an item */
static void item_state(struct steam_workshop_file const *file, char *state, int state_size)
{
	int installed;

	if (steam.downloading > 0 && steam.downloading_id == file->id)
		percent_text(state, state_size);
	else if (queue_find(1, file->id) >= 0)
		snprintf(state, (size_t)state_size, "QUEUED");
	else if ((installed = item_installed(file->id, file->manifest)) == 1)
		snprintf(state, (size_t)state_size, "INSTALLED");
	else if (installed == 2)
		snprintf(state, (size_t)state_size, "UPDATE");
	else
		size_text(file->size, state, state_size);
}

/* (state_lock held) */
static int package_outdated(struct package const *package)
{
	return package->file.id && item_installed(package->id, package->file.manifest) == 2;
}

/* a state of the user's (an account's name, a search) as the row's state box
holds it: 15 characters, a longer one cut to 12 and "..." */
static void state_text(char *state, int state_size, char const *text)
{
	if (strlen(text) > 15)
		snprintf(state, (size_t)state_size, "%.12s...", text);
	else
		snprintf(state, (size_t)state_size, "%s", text);
}

/* (state_lock held) */
static void row_build(int place, char *name, int name_size, char *state, int state_size)
{
	int index;

	name[0] = state[0] = 0;
	switch (row_kind(place, &index))
	{
	case ROW_SIGN_IN_ACCOUNT:
		snprintf(name, (size_t)name_size, "SIGN IN WITH ACCOUNT");
		if (steam.state == SESSION_UNAVAILABLE)
			snprintf(state, (size_t)state_size, "UNAVAILABLE");
		else if (steam.state == SESSION_CONNECTING)
			snprintf(state, (size_t)state_size, "CONNECTING");
		else if (steam.state == SESSION_GUARD)
			snprintf(state, (size_t)state_size, "%s", steam.confirmations & STEAM_GUARD_DEVICE_CONFIRMATION ?
				"APPROVE IN APP" : "ENTER CODE");
		else if (steam.state == SESSION_SIGNING_IN)
			snprintf(state, (size_t)state_size, "SIGNING IN");
		break;
	case ROW_HOME_ACCOUNT:
		snprintf(name, (size_t)name_size, "MANAGE ACCOUNT");
		break;
	case ROW_HOME_FILES:
		snprintf(name, (size_t)name_size, "MCC FILE STATUS");
		if (steam.downloading)
		{
			percent_text(state, state_size);
			if (steam.queue_count)
			{
				size_t used = strlen(state);

				snprintf(state + used, (size_t)state_size - used, " +%d", steam.queue_count);
			}
		}
		else if (steam.files_known)
			snprintf(state, (size_t)state_size, "%s", shared_installed() ? "READY" : "MISSING FILES");
		break;
	case ROW_HOME_BROWSE:
		snprintf(name, (size_t)name_size, "BROWSE WORKSHOP");
		break;
	case ROW_ACCOUNT_NAME:
		snprintf(name, (size_t)name_size, "STEAM ACCOUNT");
		state_text(state, state_size, steam.account[0] ? steam.account : "SIGNED IN");
		break;
	case ROW_LOG_OUT:
		snprintf(name, (size_t)name_size, "LOG OUT");
		if (steam.confirm == place)
			snprintf(state, (size_t)state_size, "LOG OUT?");
		break;
	case ROW_SHARED:
		snprintf(name, (size_t)name_size, "MCC SHARED FILES");
		if (steam.downloading < 0)
			percent_text(state, state_size);
		else if (queue_find(-1, 0) >= 0)
			snprintf(state, (size_t)state_size, "QUEUED");
		else if (steam.confirm == place)
			snprintf(state, (size_t)state_size, "DOWNLOAD AGAIN?");
		else if (steam.files_known)
			snprintf(state, (size_t)state_size, "%s", shared_installed() ? "INSTALLED" :
				steam.owns_halo1 == 0 ? "NOT OWNED" : "DOWNLOAD");
		break;
	case ROW_PACKAGE:
	{
		struct package const *package = &steam.packages[index];

		snprintf(name, (size_t)name_size, "%s", package->title[0] ? package->title : package->folder);
		if (steam.downloading > 0 && steam.downloading_id == package->id)
			percent_text(state, state_size);
		else if (queue_find(1, package->id) >= 0)
			snprintf(state, (size_t)state_size, "QUEUED");
		else if (steam.confirm == place)
			snprintf(state, (size_t)state_size, "REMOVE?");
		else
			snprintf(state, (size_t)state_size, "%s", package_outdated(package) ? "UPDATE" : "INSTALLED");
		break;
	}
	case ROW_SEARCH:
		snprintf(name, (size_t)name_size, "SEARCH");
		state_text(state, state_size, steam.search[0] ? steam.search : "ANY");
		break;
	case ROW_SORT:
		snprintf(name, (size_t)name_size, "SORT");
		snprintf(state, (size_t)state_size, "%s", steam.search[0] ? "RELEVANCE" : sort_names[steam.sort]);
		break;
	case ROW_TYPE:
		snprintf(name, (size_t)name_size, "TYPE");
		snprintf(state, (size_t)state_size, "%s", type_names[steam.type]);
		break;
	case ROW_ITEM:
		snprintf(name, (size_t)name_size, "%s", steam.items[index].title[0] ? steam.items[index].title : "(untitled)");
		item_state(&steam.items[index], state, state_size);
		break;
	case ROW_PREVIOUS:
		snprintf(name, (size_t)name_size, "PREVIOUS PAGE");
		snprintf(state, (size_t)state_size, "%d", steam.page - 1);
		break;
	case ROW_NEXT:
		snprintf(name, (size_t)name_size, "NEXT PAGE");
		snprintf(state, (size_t)state_size, "%d OF %u", steam.page + 1,
			(unsigned)((steam.total + WORKSHOP_PAGE_ITEMS - 1) / WORKSHOP_PAGE_ITEMS));
		break;
	default:
		break;
	}
}

/* (state_lock held) the shared sets' files, each here or not */
static void shared_text(char *text, int size)
{
	int i;
	size_t used;

	snprintf(text, (size_t)size, "Resources many maps need:");
	for (i = 0; i < SHARED_FILES; i++)
	{
		used = strlen(text);
		snprintf(text + used, (size_t)size - used, "\n%s %.*s: %s",
			strstr(shared_files[i].folder, "singleplayer") ? "Campaign" : "MP",
			(int)(strchr(shared_files[i].name, '.') - shared_files[i].name), shared_files[i].name,
			!steam.files_known ? "..." : steam.shared_present[i] ? "here" : "MISSING");
	}
}

/* (state_lock held) */
static void details_build(int place, char *title, int title_size, char *text, int text_size)
{
	int index;
	char body[1024];

	body[0] = 0;
	title[0] = 0;
	switch (row_kind(place, &index))
	{
	case ROW_SIGN_IN_ACCOUNT:
		snprintf(title, (size_t)title_size, "Sign in with account");
		if (steam.state == SESSION_GUARD)
			snprintf(body, sizeof(body), "%s%s B cancels.",
				steam.confirmations & STEAM_GUARD_DEVICE_CONFIRMATION ?
				"Approve it in the Steam app. " : "",
				steam.confirmations & STEAM_GUARD_DEVICE_CODE ? "Or A types its Steam Guard code." :
				steam.confirmations & STEAM_GUARD_EMAIL_CODE ? "A types the code Steam emailed." :
				steam.confirmations & STEAM_GUARD_EMAIL_CONFIRMATION ? "Approve it from Steam's email." : "");
		else if (steam.state == SESSION_CONNECTING || steam.state == SESSION_SIGNING_IN)
			snprintf(body, sizeof(body), "Reaching Steam...");
		else
			snprintf(body, sizeof(body), "A asks for your account name and password, then Steam Guard. The password "
				"is not kept.");
		break;
	case ROW_HOME_ACCOUNT:
		snprintf(title, (size_t)title_size, "Manage account");
		snprintf(body, sizeof(body), "The account downloads use, and logging out.");
		break;
	case ROW_HOME_FILES:
		snprintf(title, (size_t)title_size, "MCC file status");
		shared_text(body, sizeof(body));
		snprintf(body + strlen(body), sizeof(body) - strlen(body), "\n%d Workshop items installed.",
			steam.package_count);
		break;
	case ROW_HOME_BROWSE:
		snprintf(title, (size_t)title_size, "Browse Workshop");
		snprintf(body, sizeof(body), "MCC's Halo 1 maps and campaigns. A on one installs it.");
		break;
	case ROW_ACCOUNT_NAME:
		snprintf(title, (size_t)title_size, "%s", steam.account[0] ? steam.account : "Steam account");
		snprintf(body, sizeof(body), "Signed in. It is kept on this device until you log out.");
		break;
	case ROW_LOG_OUT:
		snprintf(title, (size_t)title_size, "Log out");
		snprintf(body, sizeof(body), "%s Downloads stay in mcc_maps.", steam.confirm == place ? "A again logs out." :
			"Forgets this device's sign-in.");
		break;
	case ROW_SHARED:
		snprintf(title, (size_t)title_size, "MCC shared files");
		if (steam.downloading < 0)
			progress_text(body, sizeof(body));
		else if (queue_find(-1, 0) >= 0)
			queued_text(queue_find(-1, 0), body, sizeof(body));
		else
		{
			shared_text(body, sizeof(body));
			snprintf(body + strlen(body), sizeof(body) - strlen(body), "\n%s",
				!shared_installed() ? "A downloads them." : steam.confirm == place ?
				"A again downloads them anew." : "A twice downloads them again.");
		}
		break;
	case ROW_PACKAGE:
	{
		struct package const *package = &steam.packages[index];

		snprintf(title, (size_t)title_size, "%s", package->title[0] ? package->title : package->folder);
		if (steam.downloading > 0 && steam.downloading_id == package->id)
			progress_text(body, sizeof(body));
		else if (queue_find(1, package->id) >= 0)
			queued_text(queue_find(1, package->id), body, sizeof(body));
		else if (package_outdated(package))
		{
			char size[32];

			size_text(package->file.size, size, sizeof(size));
			snprintf(body, sizeof(body), "An update is out (%s): A downloads it.", size);
		}
		else
			snprintf(body, sizeof(body), "%s%s", package->file.id ? "Up to date. " : "",
				steam.confirm == place ? "A again removes it." : "A twice removes it.");
		if (package->file.id)
			preview_want(&package->file);
		break;
	}
	case ROW_SEARCH:
	case ROW_SORT:
	case ROW_TYPE:
		snprintf(title, (size_t)title_size, "Filters");
		snprintf(body, sizeof(body), "A changes it; an empty search clears it. %u items.%s", (unsigned)steam.total,
			steam.loading ? " Loading..." : "");
		break;
	case ROW_ITEM:
	{
		struct steam_workshop_file const *file = &steam.items[index];
		char size[32];
		int installed = item_installed(file->id, file->manifest);

		snprintf(title, (size_t)title_size, "%s", file->title[0] ? file->title : "(untitled)");
		size_text(file->size, size, sizeof(size));
		if (steam.downloading > 0 && steam.downloading_id == file->id)
			progress_text(body, sizeof(body));
		else if (queue_find(1, file->id) >= 0)
			queued_text(queue_find(1, file->id), body, sizeof(body));
		else
			snprintf(body, sizeof(body), "%s, %u subscribers. %s\n%.300s", size, (unsigned)file->subscriptions,
				installed == 1 ? "Installed: A again downloads it." :
				installed == 2 ? "Update: A downloads it." : "A installs it.",
				file->description);
		preview_want(file);
		break;
	}
	case ROW_PREVIOUS:
	case ROW_NEXT:
		snprintf(title, (size_t)title_size, "Page %d", steam.page);
		snprintf(body, sizeof(body), "%u items.%s", (unsigned)steam.total, steam.loading ? " Loading..." : "");
		break;
	default:
		break;
	}
	{
		/* (what the threads are about, with how long they have been, after the
		row's text; a download's error and the latest message before it, that
		the details' few lines never cut them off) */
		size_t used;
		unsigned long now = steam_milliseconds();

		text[0] = 0;
		/* (a download's error goes once its time is up: from the message too,
		if nothing has taken its place) */
		if (steam.held_error[0] && (long)(steam.held_until - now) <= 0)
		{
			if (!strcmp(steam.message, steam.held_error))
				steam.message[0] = 0;
			steam.held_error[0] = 0;
		}
		/* (a download's error first while it is held, if another message
		has taken its place) */
		if (steam.held_error[0] && (long)(steam.held_until - now) > 0 && strcmp(steam.held_error, steam.message))
			snprintf(text, (size_t)text_size, "%s\n", steam.held_error);
		if (steam.message[0])
		{
			used = strlen(text);
			snprintf(text + used, (size_t)text_size - used, "%s\n", steam.message);
		}
		used = strlen(text);
		snprintf(text + used, (size_t)text_size - used, "%s", body);
		if (steam.session_activity[0])
		{
			used = strlen(text);
			snprintf(text + used, (size_t)text_size - used, "\nSteam: %s (%lus)", steam.session_activity,
				(now - steam.session_since) / 1000);
		}
		if (steam.files_activity[0])
		{
			used = strlen(text);
			snprintf(text + used, (size_t)text_size - used, "\nFiles: %s (%lus)", steam.files_activity,
				(now - steam.files_since) / 1000);
		}
	}
}

/* (state_lock held) a download asked for: started, or queued behind the
one under way (the files thread takes them in turn) */
static void download_ask(int what, struct steam_workshop_file const *item)
{
	struct queued_download *queued;

	if (steam.queue_count == DOWNLOAD_QUEUE)
	{
		snprintf(steam.message, sizeof(steam.message), "The download queue is full: wait for some to finish.");
		return;
	}
	steam.message[0] = 0;
	queued = &steam.queue[steam.queue_count++];
	memset(queued, 0, sizeof(*queued));
	queued->what = what;
	if (item)
		queued->item = *item;
	/* (QUEUED a moment, till the files thread takes it: it then shows as the
	download under way) */
	if (steam.downloading)
		snprintf(steam.message, sizeof(steam.message), "Queued (%d waiting).", steam.queue_count);
}

/* (state_lock held) whether a download is under way or waiting */
static int downloads_busy(void)
{
	return steam.downloading || steam.queue_count;
}

void steam_downloads_choose(int place)
{
	int index;
	int confirm;

	pthread_mutex_lock(&steam.state_lock);
	confirm = steam.confirm;
	steam.confirm = -1;
	switch (row_kind(place, &index))
	{
	case ROW_SIGN_IN_ACCOUNT:
		if (steam.state == SESSION_OUT)
			steam.want_text = TEXT_ACCOUNT;
		else if (steam.state == SESSION_GUARD &&
			(steam.confirmations & (STEAM_GUARD_DEVICE_CODE | STEAM_GUARD_EMAIL_CODE)))
			steam.want_text = TEXT_CODE;
		break;
	case ROW_HOME_ACCOUNT:
		view_open(VIEW_ACCOUNT);
		break;
	case ROW_HOME_FILES:
		view_open(VIEW_FILES);
		break;
	case ROW_HOME_BROWSE:
		view_open(VIEW_BROWSE);
		break;
	case ROW_LOG_OUT:
		if (confirm == place)
			steam.want_sign_out = 1;
		else
			steam.confirm = place;
		break;
	case ROW_SHARED:
		if (steam.downloading < 0 && queue_find(-1, 0) < 0)
			steam.cancel_download = 1;
		else if (queue_find(-1, 0) >= 0)
			queue_remove(queue_find(-1, 0));
		/* (installed already: downloaded again only on a second A) */
		else if (steam.files_known && shared_installed() && confirm != place)
			steam.confirm = place;
		else
			download_ask(-1, NULL);
		break;
	case ROW_PACKAGE:
	{
		struct package const *package = &steam.packages[index];

		if (queue_find(1, package->id) >= 0)
			queue_remove(queue_find(1, package->id));
		else if (steam.downloading > 0 && steam.downloading_id == package->id)
			steam.cancel_download = 1;
		else if (package_outdated(package) && package->file.manifest)
			download_ask(1, &package->file);
		else if (confirm == place)
			snprintf(steam.want_remove, sizeof(steam.want_remove), "%s", package->folder);
		else
			steam.confirm = place;
		break;
	}
	case ROW_SEARCH:
		steam.want_text = TEXT_SEARCH;
		break;
	case ROW_SORT:
		steam.sort = (steam.sort + 1) % (int)(sizeof(sorts) / sizeof(sorts[0]));
		steam.page = 1;
		steam.want_page = 1;
		break;
	case ROW_TYPE:
		steam.type = (steam.type + 1) % (int)(sizeof(type_names) / sizeof(type_names[0]));
		steam.page = 1;
		steam.want_page = 1;
		break;
	case ROW_ITEM:
		if (queue_find(1, steam.items[index].id) >= 0)
			queue_remove(queue_find(1, steam.items[index].id));
		else if (steam.downloading > 0 && steam.downloading_id == steam.items[index].id)
			steam.cancel_download = 1;
		else
			download_ask(1, &steam.items[index]);
		break;
	case ROW_PREVIOUS:
		steam.page--;
		steam.want_page = 1;
		steam.page_jump = 1;
		break;
	case ROW_NEXT:
		steam.page++;
		steam.want_page = 1;
		steam.page_jump = 1;
		break;
	default:
		break;
	}
	wake();
	pthread_mutex_unlock(&steam.state_lock);
}

int steam_downloads_back(void)
{
	int taken = 0;

	pthread_mutex_lock(&steam.state_lock);
	if (steam.state == SESSION_GUARD)
	{
		steam.cancel_sign_in = 1;
		taken = 1;
	}
	else if (steam.view != VIEW_HOME)
	{
		view_open(VIEW_HOME);
		taken = 1;
	}
	else if (downloads_busy())
	{
		/* (the screen is not left while maps are coming: the queue would
		go on with no screen to show it, and a map half here is no use) */
		snprintf(steam.message, sizeof(steam.message), "%d download%s going: wait, or cancel %s, to leave.",
			(steam.downloading ? 1 : 0) + steam.queue_count,
			(steam.downloading ? 1 : 0) + steam.queue_count == 1 ? "" : "s",
			(steam.downloading ? 1 : 0) + steam.queue_count == 1 ? "it" : "them");
		taken = 1;
	}
	pthread_mutex_unlock(&steam.state_lock);
	return taken;
}



void steam_downloads_text_typed(int kind, char const *text, int saved)
{
	pthread_mutex_lock(&steam.state_lock);
	switch (kind)
	{
	case TEXT_SEARCH:
		if (saved)
		{
			snprintf(steam.search, sizeof(steam.search), "%s", text);
			steam.page = 1;
			steam.want_page = 1;
		}
		break;
	case TEXT_ACCOUNT:
		if (saved && text[0])
		{
			snprintf(steam.typed_account, sizeof(steam.typed_account), "%s", text);
			steam.want_text = TEXT_PASSWORD;
		}
		break;
	case TEXT_PASSWORD:
		if (saved && text[0] && steam.state == SESSION_OUT)
		{
			snprintf(steam.typed_password, sizeof(steam.typed_password), "%s", text);
			steam.want_password = 1;
		}
		break;
	case TEXT_CODE:
		if (saved && text[0] && steam.state == SESSION_GUARD)
		{
			snprintf(steam.typed_code, sizeof(steam.typed_code), "%s", text);
			steam.code_typed = 1;
		}
		break;
	default:
		break;
	}
	wake();
	pthread_mutex_unlock(&steam.state_lock);
}

/* ---------- the frame's snapshot

What the screen draws is copied out once a frame (steam_downloads_poll),
with a try of the lock that never waits: when a thread has it, the last
frame's copy is drawn again. The game thread then holds the lock once a
frame, briefly, and never stands in the threads' way. */

enum { SNAPSHOT_ROWS = 24 };

static struct
{
	int count;
	int view;
	/* the rows built: from first_built, SNAPSHOT_ROWS of them */
	int first_built;
	char names[SNAPSHOT_ROWS][96];
	char states[SNAPSHOT_ROWS][48];
	char title[128];
	char text[1600];
	uint64_t picture_key;
	uint32_t picture[PREVIEW_SIDE * PREVIEW_SIDE];
	/* the text to type, taken from the model once */
	int want_text;
	char initial[TEXT_SIZE];
} snapshot;

int steam_downloads_poll(void)
{
	int rescan = 0;
	int index, i;
	struct steam_workshop_file const *file = NULL;
	struct preview *preview;

	if (pthread_mutex_trylock(&steam.state_lock) != 0)
		return 0;
	rescan = steam.rescan;
	steam.rescan = 0;
	/* (signed out, or never in: the hub alone) */
	if (steam.state != SESSION_IN && steam.view != VIEW_HOME)
		view_open(VIEW_HOME);
	snapshot.count = rows_count();
	snapshot.view = (int)steam.view;
	snapshot.first_built = steam.first > 4 ? steam.first - 4 : 0;
	for (i = 0; i < SNAPSHOT_ROWS; i++)
		row_build(snapshot.first_built + i, snapshot.names[i], (int)sizeof(snapshot.names[i]), snapshot.states[i],
			(int)sizeof(snapshot.states[i]));
	details_build(steam.chosen, snapshot.title, (int)sizeof(snapshot.title), snapshot.text, (int)sizeof(snapshot.text));
	switch (row_kind(steam.chosen, &index))
	{
	case ROW_ITEM: file = &steam.items[index]; break;
	case ROW_PACKAGE: file = steam.packages[index].file.id ? &steam.packages[index].file : NULL; break;
	default: break;
	}
	if (file && (preview = preview_find(file->id)) != NULL && preview->state == 1 && preview->pixels)
	{
		if (snapshot.picture_key != file->id)
		{
			memcpy(snapshot.picture, preview->pixels, sizeof(snapshot.picture));
			snapshot.picture_key = file->id;
		}
	}
	else
		snapshot.picture_key = 0;
	if (steam.want_text && !snapshot.want_text)
	{
		snapshot.want_text = steam.want_text;
		snapshot.initial[0] = 0;
		if (steam.want_text == TEXT_SEARCH)
			snprintf(snapshot.initial, sizeof(snapshot.initial), "%s", steam.search);
		else if (steam.want_text == TEXT_ACCOUNT)
			snprintf(snapshot.initial, sizeof(snapshot.initial), "%s", steam.typed_account);
		steam.want_text = TEXT_NONE;
	}
	pthread_mutex_unlock(&steam.state_lock);
	return rescan;
}

int steam_downloads_count(void)
{
	return snapshot.count;
}

int steam_downloads_view(void)
{
	return snapshot.view;
}

void steam_downloads_row(int place, char *name, int name_size, char *state, int state_size)
{
	int at = place - snapshot.first_built;

	name[0] = state[0] = 0;
	if (at < 0 || at >= SNAPSHOT_ROWS || place >= snapshot.count)
		return;
	snprintf(name, (size_t)name_size, "%s", snapshot.names[at]);
	snprintf(state, (size_t)state_size, "%s", snapshot.states[at]);
}

void steam_downloads_details(int place, char *title, int title_size, char *text, int text_size)
{
	(void)place;
	snprintf(title, (size_t)title_size, "%s", snapshot.title);
	snprintf(text, (size_t)text_size, "%s", snapshot.text);
}

int steam_downloads_text_wanted(char *initial, int initial_size, int *secret)
{
	int kind = snapshot.want_text;

	snapshot.want_text = TEXT_NONE;
	*secret = kind == TEXT_PASSWORD;
	snprintf(initial, (size_t)initial_size, "%s", kind ? snapshot.initial : "");
	return kind;
}

char const *steam_downloads_text_caption(int kind)
{
	switch (kind)
	{
	case TEXT_SEARCH: return "Search the Workshop";
	case TEXT_ACCOUNT: return "Steam account name";
	case TEXT_PASSWORD: return "Steam password";
	case TEXT_CODE: return "Steam Guard code";
	default: return "";
	}
}

int steam_downloads_picture(uint64_t *key, uint32_t *pixels)
{
	*key = snapshot.picture_key;
	if (!snapshot.picture_key)
		return 0;
	if (pixels)
		memcpy(pixels, snapshot.picture, sizeof(snapshot.picture));
	return PREVIEW_SIDE;
}

/* ---------- the guest's calls (port/linux/src/steam_screen.c: its
addresses, 32-bit, are the host's) */

#define GUEST_POINTER(address) ((void *)(uintptr_t)(address))

void host_steam_open(unsigned int data_root)
{
	steam_downloads_open((char const *)GUEST_POINTER(data_root));
}

void host_steam_close(void)
{
	steam_downloads_close();
}

int host_steam_poll(void)
{
	return steam_downloads_poll();
}

int host_steam_count(void)
{
	return steam_downloads_count();
}

int host_steam_view(void)
{
	return steam_downloads_view();
}

int host_steam_first(void)
{
	return steam.first;
}

void host_steam_set_first(int first)
{
	steam.first = (short)first;
}

int host_steam_chosen(void)
{
	return steam.chosen;
}

void host_steam_set_chosen(int chosen)
{
	steam.chosen = (short)chosen;
}

void host_steam_row(int place, unsigned int name, int name_size, unsigned int state, int state_size)
{
	steam_downloads_row(place, GUEST_POINTER(name), name_size, GUEST_POINTER(state), state_size);
}

void host_steam_details(int place, unsigned int title, int title_size, unsigned int text, int text_size)
{
	steam_downloads_details(place, GUEST_POINTER(title), title_size, GUEST_POINTER(text), text_size);
}

void host_steam_choose(int place)
{
	steam_downloads_choose(place);
}

int host_steam_back(void)
{
	return steam_downloads_back();
}

int host_steam_picture(unsigned int key, unsigned int pixels)
{
	uint64_t value = 0;
	int side = steam_downloads_picture(&value, pixels ? GUEST_POINTER(pixels) : NULL);

	/* (the key as two 32-bit halves, low first) */
	((uint32_t *)GUEST_POINTER(key))[0] = (uint32_t)value;
	((uint32_t *)GUEST_POINTER(key))[1] = (uint32_t)(value >> 32);
	return side;
}

int host_steam_text_wanted(unsigned int initial, int initial_size, unsigned int secret)
{
	return steam_downloads_text_wanted(GUEST_POINTER(initial), initial_size, GUEST_POINTER(secret));
}

void host_steam_text_caption(int kind, unsigned int text, int text_size)
{
	snprintf(GUEST_POINTER(text), (size_t)text_size, "%s", steam_downloads_text_caption(kind));
}

void host_steam_text_typed(int kind, unsigned int text, int saved)
{
	steam_downloads_text_typed(kind, GUEST_POINTER(text), saved);
}

/* (state_lock held) whether a file of MCC's shared sets is missing from mcc_maps */
static int shared_missing(void)
{
	int i;

	for (i = 0; i < SHARED_FILES; i++)
	{
		char path[1200];
		int directory = 0;

		snprintf(path, sizeof(path), "%s/%s/%s", steam.root, shared_files[i].folder, shared_files[i].name);
		if (!path_exists(path, &directory) || directory)
			return 1;
	}
	return 0;
}

/* MCC's shared sets for a joiner's fetch (the base resources a host never
sends): 0 here already, 1 downloading or queued (queued now when missing),
-1 no Steam sign-in to fetch them with */
int host_steam_shared_fetch(unsigned int data_root)
{
	int have, state;

	threads_start((char const *)GUEST_POINTER(data_root));
	pthread_mutex_lock(&steam.session_lock);
	have = steam.token[0] != 0;
	pthread_mutex_unlock(&steam.session_lock);
	pthread_mutex_lock(&steam.state_lock);
	if (!steam.threads_started)
		state = -1;
	else if (steam.downloading < 0 || queue_find(-1, 0) >= 0)
		state = 1;
	else if (!shared_missing())
		state = 0;
	else if (!have || steam.queue_count == DOWNLOAD_QUEUE)
		state = -1;
	else
	{
		download_ask(-1, NULL);
		pthread_cond_signal(&steam.files_wake);
		steam_log("steam fetch: MCC's shared files are missing: downloading them");
		state = 1;
	}
	pthread_mutex_unlock(&steam.state_lock);
	return state;
}

/* the same, without queueing: 0 here, 1 downloading or queued, -1 missing */
int host_steam_shared_state(void)
{
	int state;

	pthread_mutex_lock(&steam.state_lock);
	state = !steam.threads_started ? -1 : steam.downloading < 0 || queue_find(-1, 0) >= 0 ? 1 :
		shared_missing() ? -1 : 0;
	pthread_mutex_unlock(&steam.state_lock);
	return state;
}

/* a whole Workshop item installed in mcc_maps for a joiner (its id as two
halves, low first): 1 begun, 0 when there is no Steam sign-in */
int host_steam_install_start(unsigned int data_root, unsigned int id_low, unsigned int id_high)
{
	int have;

	threads_start((char const *)GUEST_POINTER(data_root));
	pthread_mutex_lock(&steam.session_lock);
	have = steam.token[0] != 0;
	pthread_mutex_unlock(&steam.session_lock);
	if (!have)
		return 0;
	pthread_mutex_lock(&steam.state_lock);
	fetch.install_id = (uint64_t)id_low | (uint64_t)id_high << 32;
	fetch.install_state = 0;
	fetch.install_want = 1;
	pthread_cond_signal(&fetch.wake);
	pthread_mutex_unlock(&steam.state_lock);
	return 1;
}

/* 0 under way, 1 installed, -1 not */
int host_steam_install_status(void)
{
	int state;

	pthread_mutex_lock(&steam.state_lock);
	state = fetch.install_state;
	pthread_mutex_unlock(&steam.state_lock);
	return state;
}

/* the joiner's install's progress, in KB: done, of all (0 not yet known),
and a second (0 not yet measured) */
void host_steam_install_progress(unsigned int progress_address)
{
	uint32_t *progress = GUEST_POINTER(progress_address);

	pthread_mutex_lock(&steam.state_lock);
	progress[0] = (uint32_t)(steam.done >> 10);
	progress[1] = (uint32_t)(steam.size >> 10);
	progress[2] = (uint32_t)(steam.speed >> 10);
	pthread_mutex_unlock(&steam.state_lock);
}

/* nonzero once when files were put in mcc_maps since the last ask (an
install for a joiner, the shared sets, the screen's downloads): the guest
rescans its MCC catalog, the Workshop screen open or not */
int host_steam_take_rescan(void)
{
	int rescan;

	if (pthread_mutex_trylock(&steam.state_lock) != 0)
		return 0;
	rescan = steam.rescan;
	steam.rescan = 0;
	pthread_mutex_unlock(&steam.state_lock);
	return rescan;
}

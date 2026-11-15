/* MCC package sharing in a network game: the game side of mcc_share_session.c.
 * See mcc_share.h and port/linux/NETCODE.md, "MCC package sharing". */
#include "cseries.h"
#include "cseries/errors.h"
#include "bungie_net/common/message_header.h"
#include "game/game.h"
#include "interface/interface.h"
#include "interface/ui_widget.h"
#include "main/main.h"
#include "networking/network_game_globals.h"
#include "networking/network_game_manager.h"
#include "networking/network_client_manager.h"
#include "render/render.h"
#include "text/draw_string.h"
#include "text/font_group.h"
#include "rasterizer/rasterizer.h"
#include "network_distributed.h"
#include "mcc_cache.h"
#include "mcc_identity.h"
#include "mcc_validation.h"
#include "mcc_admission.h"
#include "mcc_load_network.h"
#include "mcc_maps.h"
#include "mcc_package.h"
#include "mcc_share.h"
#include "mcc_share_session.h"
#include "mcc_share_package.h"
#include "mcc_share_storage.h"
#include "../src/mcc_share_metadata.h"
#include "../src/posix.h"
#include "tag_files/tag_groups.h"
#include <stdlib.h>
#include "mcc_workshop.h"
#include <xtl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef HALO_WEB
#include "../../web/src/web_mcc_storage.h"
#endif

/* port_config.c's, network_game_globals.c's, network_server_message_handler.c's,
cseries_windows.c's */
int config_boolean(const char *name);
long config_integer(const char *name);
boolean network_distributed_client_send_reliably(void *message, word size);
boolean network_distributed_server_send_to_machine_reliably(long machine_index, void *message, word size);
boolean network_connection_last_read_was_unreliable(void);
unsigned long system_milliseconds(void);

enum
{
	/* the list is a small text file */
	MCC_SHARE_LIST_BYTES = 0x10000,
	/* text centred at the top of the screen, as the Custom Edition download's is */
	_mcc_share_text_centered = 2,
};

/* the one share: the host's side serves, the joiner's fetches. Nothing is created
until the first frame of a game (mcc_share_game_update). */
static struct mcc_share mcc_share_globals;
static boolean mcc_share_ready;
/* the host's game record as the joiner was given it (the join that began the download) */
static char mcc_share_name[128];
static unsigned char mcc_share_digest[32], mcc_share_metadata[32];
static boolean mcc_share_prompt_pending, mcc_share_prompt_visible;
static boolean mcc_share_prompt_download;
/* A package that is a Workshop item, with a Steam sign-in: its whole item
 * installed from Steam (mcc_share_install_begin), the host's offer let go, and
 * the package checked again once it is in; the level tried that way, so a
 * second offer of it (the Workshop's not the host's) asks the player and the
 * host sends it. */
static boolean mcc_share_installing;
static char mcc_share_install_tried[64];
/* where Steam installs it: the item's own folder (mcc_maps\<item id>\<map>),
which the host's may not be (a package the host downloaded from another
host is in a dl_ folder of its own) */
static char mcc_share_install_level[64];
/* Once it is in, the package is checked here against the host's identity, as
 * a download from the host is (mcc_share_identity_check), not by asking the
 * host for a second validation window: a host gives each machine one a join
 * (mcc_admission.c: a closed lease is a tombstone), and the join's own check
 * used it. Joining: the check passed and the players are no longer held; the
 * host's offer is let go only once the host has this machine's player (or
 * its request, a game in progress), since a machine with neither a player
 * nor a transfer is refused. */
static struct mcc_validation_check *mcc_share_install_check;
static struct mcc_validation_proof mcc_share_install_proof;
static boolean mcc_share_install_joining;
static unsigned long mcc_share_install_joining_at;
#define MCC_SHARE_INSTALL_JOIN_MS 3000u
/* Manual guidance outlives the refused join, but never a new session. */
static enum {
    MCC_SHARE_MANUAL_NONE,
    MCC_SHARE_MANUAL_LEAVING,
    MCC_SHARE_MANUAL_MENU
} mcc_share_manual;
static struct widget_instance *mcc_share_prompt;
static char mcc_share_prompt_message[512];
static char mcc_share_installed[64];
static char mcc_share_text[256];
static struct mcc_validation_check *mcc_share_check;
static char mcc_share_check_reason[320];
static struct mcc_validation_proof mcc_share_proof;
static uint32_t mcc_share_validation_token;
static unsigned long mcc_share_host_revision;
static uint32_t mcc_share_admission_at[MCC_SHARE_HOST_MACHINES];
static byte mcc_share_admission_pending[MCC_SHARE_HOST_MACHINES];
static char mcc_share_admission_level[128];
void platform_translate_path(char const *, char *, unsigned long);
void platform_open_url(char const *);
void ui_widget_port_go_back(struct widget_instance *widget);
static void mcc_share_offered(void *, uint64_t, char const *);
static int mcc_share_client_current(void);
static void mcc_share_host_lifecycle(void);
struct network_game_server_client_machine *network_game_server_get_client_machine_at_index(struct network_game_server *,long);
boolean network_game_server_client_machine_is_joined_to_game(struct network_game_server *,struct network_game_server_client_machine *);

static void mcc_share_stop_host(void)
{
    mcc_share_host_stop(&mcc_share_globals);
    memset(mcc_share_admission_pending,0,sizeof(mcc_share_admission_pending));
    mcc_share_admission_level[0]=0;
}

static void mcc_share_host_finished(void *user,long machine)
{
    struct network_game *game=network_game_get_game();
    (void)user;
    if (!game || machine<0 || machine>=MCC_SHARE_HOST_MACHINES) return;
    /* DONE precedes the joiner's AddPlayer. Its old
     * joining deadline may already have elapsed during an honest download. */
    mcc_share_admission_at[machine]=(uint32_t)system_milliseconds();
    mcc_share_admission_pending[machine]=1;
    snprintf(mcc_share_admission_level,sizeof(mcc_share_admission_level),"%s",game->map.name);
}

/* ---------- the engine's files and folders */

static int mcc_share_io_make(char const *path) { return CreateDirectoryA(path, NULL) != 0; }
static int mcc_share_io_remove(char const *path) { return RemoveDirectoryA(path) != 0; }
static int mcc_share_io_move(char const *from, char const *to) { return MoveFileA(from, to) != 0; }
static int mcc_share_io_delete(char const *path) { return DeleteFileA(path) != 0; }
static int mcc_share_io_exists(char const *path)
{
	unsigned long attributes = GetFileAttributesA(path);

	return attributes != (unsigned long)-1 && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

static int mcc_share_io_safe(char const *path)
{
    char native[1024];
    platform_translate_path(path,native,sizeof(native));
    return posix_mcc_share_path_safe(native);
}
static int mcc_share_io_publish(char const *from,char const *to)
{
    char a[1024],b[1024];
    platform_translate_path(from,a,sizeof(a)); platform_translate_path(to,b,sizeof(b));
    return posix_mcc_share_publish(a,b);
}

/* ---------- messages */

static void mcc_share_fill_header(
	struct distributed_message_header *header,
	byte type,
	word size)
{
	header->header = 0;
	build_message_header(&header->header, size, 2, 0);
	header->type = type;
	header->count = 1;
	header->game_time = game_in_progress() ? game_time_get() : 0;
}

static int mcc_share_send_request(void *user, struct mcc_share_request const *request)
{
	struct
	{
		struct distributed_message_header header;
		struct mcc_share_request request;
	} message;

	(void)user;
	csmemset(&message, 0, sizeof(message));
	mcc_share_fill_header(&message.header, _distributed_message_mcc_share_request, (word)sizeof(message));
	message.request = *request;
	return network_distributed_client_send_reliably(&message, (word)sizeof(message));
}

static int mcc_share_send_response(void *user, long machine, void const *payload, unsigned bytes)
{
	static struct
	{
		struct distributed_message_header header;
		byte payload[MCC_SHARE_RESPONSE_MAXIMUM];
	} message;
	word size = (word)(sizeof(message.header) + bytes);

	(void)user;
	if (bytes > sizeof(message.payload))
		return 0;
	csmemset(&message.header, 0, sizeof(message.header));
	mcc_share_fill_header(&message.header, _distributed_message_mcc_share_response, size);
	csmemcpy(message.payload, payload, bytes);
	return network_distributed_server_send_to_machine_reliably(machine, &message, size);
}

boolean mcc_share_game_handles_message(
	word const *message,
	word size)
{
	struct distributed_message_header header;

	if (size < sizeof(header))
		return FALSE;
	csmemcpy(&header, message, sizeof(header));
	return header.type == _distributed_message_mcc_share_request || header.type == _distributed_message_mcc_share_response;
}

void mcc_share_game_handle_message(
	long machine_index,
	word const *message,
	word size,
	boolean from_stream)
{
	struct distributed_message_header header;
	byte const *body = (byte const *)message + sizeof(header);

	/* (over the sender's connection alone: a datagram can be sent as from anyone) */
	if (!mcc_share_game_handles_message(message, size) || !from_stream || !mcc_share_ready)
		return;
	csmemcpy(&header, message, sizeof(header));
    if (header.count!=1 || GET_MESSAGE_FLAGS(header.header) ||
        GET_MESSAGE_TYPE(header.header)!=2 || GET_MESSAGE_SIZE(header.header)!=size) return;
    mcc_share_host_lifecycle();
	if (header.type == _distributed_message_mcc_share_request)
	{
		/* (the host takes requests from the machines of its game) */
        if (machine_index >= 0 && machine_index < MCC_SHARE_HOST_MACHINES && global_network_game_server_get() &&
            network_game_get_game() && mcc_level_name(network_game_get_game()->map.name) &&
            network_game_server_client_machine_is_joined_to_game(global_network_game_server_get(),
                network_game_server_get_client_machine_at_index(global_network_game_server_get(),machine_index)))
            mcc_share_host_request(&mcc_share_globals, machine_index, body, size - sizeof(header));
	}
	else if (machine_index == NONE && !global_network_game_server_get())
	{
		mcc_share_client_response(&mcc_share_globals, body, size - sizeof(header));
	}
}

/* ---------- the host's admission */

/* (the host) a package's files hashed on the host's threads with the CPU's
SHA-256 (host_mcc_hash.c's host_sha256_*), not a few megabytes a frame on
the game's: hashed in the frame, a 360 MB package outlasted a joiner's wait
for the offer and took its frames */
#if defined(HALO_ANDROID)
int host_sha256_start(unsigned int path);
int host_sha256_poll(int id, unsigned int digest, unsigned int size);
void host_sha256_cancel(int id);

static int mcc_share_hash_begin(void *user, char const *path)
{
    (void)user;
    return host_sha256_start((unsigned int)(uintptr_t)path);
}

static int mcc_share_hash_poll(void *user, int job, uint8_t digest[32], uint64_t *size)
{
    static uint8_t done_digest[32];
    static uint32_t done_size[2];
    int status;
    (void)user;
    status=host_sha256_poll(job,(unsigned int)(uintptr_t)done_digest,(unsigned int)(uintptr_t)done_size);
    if (status==1) {
        memcpy(digest,done_digest,32);
        *size=(uint64_t)done_size[0]|(uint64_t)done_size[1]<<32;
    }
    return status;
}

static void mcc_share_hash_cancel(void *user, int job)
{
    (void)user;
    host_sha256_cancel(job);
}
#endif

/* (the host) a transfer refused before it was offered: the joiner is told the
result alone, so the reason is said here */
static void mcc_share_host_refused(void *user, long machine, char const *level, int result, char const *why)
{
    (void)user;
    error(_error_silent,"mcc share: machine #%ld's download of %s refused (result %d): %s",machine,level,result,why);
}

static void mcc_share_host_package_files(void *user, char const *level,
    struct mcc_share_host_package *package)
{
    (void)user;
    mcc_share_package_build(level,package);
}

/* the host's limit, as map_share.c takes it (16 KB/s to 250 MB/s) */
static uint32_t mcc_share_upload_limit(
	void *user)
{
	long kilobytes = config_integer("network.map_share_upload");

	(void)user;
	return (uint32_t)PIN(kilobytes, 16, 256000) * 1024;
}

static uint64_t mcc_share_free_bytes(
	void *user)
{
#ifdef HALO_WEB
    (void)user;
    return web_mcc_free_bytes();
#else
	ULARGE_INTEGER available;
	ULARGE_INTEGER total;
	ULARGE_INTEGER free_total;

	(void)user;
	if (!GetDiskFreeSpaceExA("d:\\", &available, &total, &free_total))
		return 0;
	return available.QuadPart;
#endif
}

static uint32_t mcc_share_now(
	void *user)
{
	(void)user;
	return (uint32_t)system_milliseconds();
}

/* ---------- the joiner */

/* whether an installed map has this package's folder (any level of it): a new package never
goes beside or over it */
static int mcc_share_package_taken(void *user, char const *level)
{
    (void)user; (void)level;
    /* A managed snapshot has its own destination, never the host's folder. */
    return 0;
}

static int mcc_share_destination(void *user, char const *level,
    struct mcc_share_file const *files, int count, char folder[64])
{
    struct mcc_sha256 hash;
    unsigned char digest[32];
    char hex[65];
    int i;
    (void)user;
    mcc_sha256_begin(&hash);
    mcc_sha256_add(&hash,mcc_share_digest,32);
    mcc_sha256_add(&hash,mcc_share_metadata,32);
    for (i=0;i<count;i++) {
        mcc_sha256_add(&hash,files[i].leaf,(int)strlen(files[i].leaf)+1);
        mcc_sha256_add(&hash,files[i].sha256,32);
    }
    mcc_sha256_end(&hash,digest); mcc_share_list_hex(digest,hex);
    snprintf(folder,64,"dl_%.20s",hex);
    return snprintf(mcc_share_installed,sizeof(mcc_share_installed),"mcc_maps\\%s\\%s",
        folder,strrchr(level,'\\')+1)<(int)sizeof(mcc_share_installed);
}

/* the staged files are the host's content: fingerprinted where they are, with the package's own
files read from the staging folder, against the identity in the host's game record */
#if defined(HALO_ANDROID)
int host_steam_shared_fetch(unsigned int data_root);
int host_steam_shared_state(void);
#endif
/* MCC's shared sets asked of Steam with the download's resources
(mcc_share_shared_begin): the identity check reads them, so it waits for them */
static int mcc_share_shared_asked;

static int mcc_share_shared_pending(void)
{
#if defined(HALO_ANDROID)
    return mcc_share_shared_asked && host_steam_shared_state()==1;
#else
    return 0;
#endif
}

static int mcc_share_identity_check(void *user, char const *stage_dir)
{
    struct mcc_share_metadata_entry *entries;
    unsigned char actual[32],keep[MCC_SHARE_METADATA_FILES];
    int indices[MCC_SHARE_METADATA_FILES],count=0,i,ok=0;
    char native[1024],reason[320];
    char const *leaf=strrchr(mcc_share_globals.level,'\\')+1;
    (void)user;
    if (!mcc_share_check && mcc_share_shared_pending()) return MCC_SHARE_CHECK_PENDING;
    if (mcc_share_check) {
        ok=mcc_validation_check_step(mcc_share_check,&mcc_share_proof,mcc_share_check_reason,sizeof(mcc_share_check_reason));
        if (!ok) return MCC_SHARE_CHECK_PENDING;
        return ok==1 && !memcmp(mcc_share_proof.digest,mcc_share_digest,32) ?
            MCC_SHARE_CHECK_READY : MCC_SHARE_CHECK_FAILED;
    }
    mcc_share_check_reason[0]=0;
    entries=malloc(MCC_SHARE_METADATA_FILES*sizeof(*entries));
    if (!entries) return 0;
    memset(entries,0,MCC_SHARE_METADATA_FILES*sizeof(*entries));
    for (i=0;i<mcc_share_globals.stage.count;i++) {
        struct mcc_share_file const *file=&mcc_share_globals.stage.files[i];
        if (file->kind<MCC_SHARE_FILE_JSON) continue;
        if (count>=MCC_SHARE_METADATA_FILES) goto done;
        strcpy(entries[count].path,file->leaf);
        entries[count].size=file->size;
        entries[count].kind=file->kind==MCC_SHARE_FILE_JSON ? MCC_SHARE_METADATA_JSON :
            file->kind==MCC_SHARE_FILE_PNG ? MCC_SHARE_METADATA_PNG : MCC_SHARE_METADATA_WORKSHOP;
        indices[count++]=i;
    }
    platform_translate_path(stage_dir,native,sizeof(native));
    if (!mcc_share_metadata_validate(native,-1,leaf,entries,count,keep,reason) ||
        !mcc_share_metadata_digest(native,-1,leaf,actual,reason) || memcmp(actual,mcc_share_metadata,32)) goto done;
    for (i=0;i<count;i++) if (!keep[i] &&
        !mcc_share_stage_discard_preview(&mcc_share_globals.stage,indices[i])) goto done;
    mcc_share_check=mcc_validation_check_begin(mcc_share_globals.level,stage_dir);
    mcc_validation_check_expect(mcc_share_check,mcc_share_digest);
    if (mcc_share_check) ok=MCC_SHARE_CHECK_PENDING;
 done:
    free(entries);
    return ok;
}

static char const *mcc_share_identity_error(void *user)
{
    (void)user;
    return mcc_share_check_reason;
}

static void mcc_share_cancel_check(void *user)
{
    (void)user;
    /* The session reads the failure reason after cancelling the stage. */
    mcc_validation_check_end(mcc_share_check);
    mcc_share_check=NULL;
    memset(&mcc_share_proof,0,sizeof(mcc_share_proof));
}

static int mcc_share_installed_ready(void *user)
{
    (void)user;
    /* Publication preserves the validated file stamps. Promote those facts
     * under the installed alias before DONE releases the host's preload hold. */
    if (!mcc_validation_publish(mcc_share_installed,&mcc_share_proof)) return 0;
    mcc_maps_rescan();
    return mcc_identity_accept_installed(mcc_share_name,mcc_share_digest,mcc_share_metadata,mcc_share_installed);
}

static int mcc_share_prepare_install(void *user,struct mcc_share_stage *stage)
{
    struct mcc_workshop_item item;
    (void)user;
    return mcc_share_storage_record(stage,mcc_share_name,mcc_share_globals.credit,
        mcc_workshop_wire_unpack(mcc_share_name,&item) ? &item : NULL);
}

#if defined(HALO_ANDROID)
const char *platform_data_root(void);
#define MCC_SHARE_HOST_ADDRESS(pointer) ((unsigned int)(uintptr_t)(pointer))
/* (the data root, as the host's Steam calls are given it) */
static char mcc_share_steam_root[1024];
#endif

/* the download accepted: MCC's shared sets queued from Steam when they are
missing, as a host never sends them */
static void mcc_share_shared_begin(void)
{
    mcc_share_shared_asked=0;
#if defined(HALO_ANDROID)
    snprintf(mcc_share_steam_root,sizeof(mcc_share_steam_root),"%s",platform_data_root());
    mcc_share_shared_asked=host_steam_shared_fetch(MCC_SHARE_HOST_ADDRESS(mcc_share_steam_root))==1;
    if (mcc_share_shared_asked)
        error(_error_silent,"mcc share: MCC's shared files are missing here: downloading them from Steam too");
#endif
}

static int mcc_share_reuse_file(void *user,struct mcc_share_file const *file,char path[1024])
{
    char root[1024];
    struct mcc_share_io io;
    (void)user;
    platform_translate_path("d:\\mcc_maps",root,sizeof(root));
    mcc_share_storage_native_io(&io);
    return mcc_share_storage_find_resource(root,&io,file,path);
}

/* the session's reads of a package's files (the host's hashing and serving,
the joiner's copies of a resource it has): mcc_share_storage.c's opener,
and when it cannot, why, in debug.txt (a path its safety check refuses, the
system's error, or something there that is not a file) */
static FILE *mcc_share_open_native_said(char const *path)
{
    FILE *file=mcc_share_open_native(path);
    if (!file) {
        int error_number=errno,safe=posix_mcc_share_path_safe(path);
        struct posix_file_information information;
        int present=!posix_stat(path,&information);
        error(_error_silent,"mcc share: cannot read %s: %s (errno %d, %s, %lu bytes)",path,
            safe ? "the system would not open it" : "its path is not one the downloads may read",error_number,
            !present ? "not there" : information.flags&_posix_file_is_directory ? "a folder" : "a file",
            present ? (unsigned long)information.size_low : 0ul);
    }
    return file;
}

static void mcc_share_release_validation(void)
{
    mcc_admission_client_release(mcc_share_validation_token);
    mcc_share_validation_token=0;
}

static void mcc_share_finished(
	void *user,
	int ok,
	char const *message)
{
	(void)user;
    mcc_share_check_reason[0]=0;
    mcc_share_release_validation();
	if (!ok)
	{
		error(_error_silent, "mcc share: the download of %s failed: %s", mcc_share_globals.level, message);
        mcc_share_prompt_download=FALSE;
        snprintf(mcc_share_prompt_message,sizeof(mcc_share_prompt_message),"%s",message);
        mcc_share_prompt_pending=TRUE;
        return;
	}
	error(_error_silent, "mcc share: installed %s from the host (credit: %s)", mcc_share_globals.level,
		message[0] ? message : "none given");
}

/* The joiner's package matches nothing installed. Ask for an offer when downloads
 * are enabled; obtaining the manifest does not accept its files. */
static int mcc_share_missing(char const name[128], unsigned char const digest[32],
    unsigned char const metadata[32],char const *reason)
{
    char canonical[64];
    if (!mcc_share_ready || global_network_game_server_get() || !global_network_game_client_get() ||
        !mcc_package_name(name,canonical) || !strchr(canonical+9,'\\')) return 0;
    if (mcc_share_client_busy(&mcc_share_globals)) {
        if (mcc_share_client_level(&mcc_share_globals,canonical) &&
            !memcmp(mcc_share_digest,digest,32) && !memcmp(mcc_share_metadata,metadata,32)) {
            mcc_share_validation_token=mcc_admission_client_token();
            return 1;
        }
        mcc_share_client_cancel(&mcc_share_globals);
    }
    memcpy(mcc_share_name,name,sizeof(mcc_share_name));
    memcpy(mcc_share_digest,digest,32); memcpy(mcc_share_metadata,metadata,32);
    mcc_share_validation_token=mcc_admission_client_token();
    if (!config_boolean("network.map_download")) {
        mcc_share_release_validation();
        mcc_share_prompt_download=FALSE;
        snprintf(mcc_share_prompt_message,sizeof(mcc_share_prompt_message),
            "%s Download Maps is OFF. Install matching content manually.",
            reason && reason[0] ? reason : "The host's package is missing or differs.");
        mcc_share_prompt_pending=TRUE;
        return 1;
    }
    return mcc_share_client_begin(&mcc_share_globals,canonical,digest);
}

boolean mcc_share_game_downloading(
	char const *level_name)
{
	char canonical[64];

	if (mcc_identity_pending(level_name)) return TRUE;
	return mcc_share_ready && mcc_share_manual==MCC_SHARE_MANUAL_NONE &&
        level_name && mcc_package_name(level_name, canonical) &&
		((!mcc_share_install_joining && mcc_share_client_level(&mcc_share_globals, canonical)) ||
        (mcc_share_installing && !strcmp(canonical,mcc_share_install_tried)) ||
        ((mcc_share_prompt_pending || mcc_share_prompt_visible) && !strcmp(canonical,mcc_share_name)));
}

boolean mcc_share_game_holds_players(
	void)
{
	return mcc_identity_pending(NULL) || mcc_admission_client_pending() || mcc_share_installing ||
        (mcc_share_ready && mcc_share_manual==MCC_SHARE_MANUAL_NONE &&
        ((mcc_share_client_busy(&mcc_share_globals) && !mcc_share_install_joining) ||
        mcc_share_prompt_pending || mcc_share_prompt_visible));
}

boolean mcc_share_game_machine_active(
	long machine_index)
{
    struct network_game *game;
    if (mcc_admission_host_active(machine_index)) return TRUE;
    if (mcc_share_game_machine_needs_map(machine_index)) return TRUE;
    if (!mcc_share_ready || machine_index<0 || machine_index>=MCC_SHARE_HOST_MACHINES ||
        !mcc_share_admission_pending[machine_index]) return FALSE;
    game=network_game_get_game();
    return game && global_network_game_server_get() && config_boolean("network.map_share") &&
        mcc_share_host_revision==mcc_identity_host_revision() &&
        !strcmp(game->map.name,mcc_share_admission_level) &&
        (uint32_t)((uint32_t)system_milliseconds()-mcc_share_admission_at[machine_index])<15000u;
}

boolean mcc_share_game_machine_needs_map(long machine_index)
{
    return mcc_admission_host_needs_map(machine_index) ||
        (mcc_share_ready && machine_index>=0 && machine_index<MCC_SHARE_HOST_MACHINES &&
        mcc_share_host_active(&mcc_share_globals,machine_index));
}

#if defined(HALO_ANDROID)
int host_steam_install_start(unsigned int data_root, unsigned int id_low, unsigned int id_high);
int host_steam_install_status(void);
void host_steam_install_progress(unsigned int progress);
int host_steam_take_rescan(void);
#endif
/* the host's offer kept alive while the Workshop installs: asked again this
often (a host gives an offer not taken up two minutes, and while it lives
counts the joiner as fetching the package, starting the game without it) */
#define MCC_SHARE_INSTALL_RENEW_MS 90000u
static unsigned long mcc_share_install_renewed_at;

/* (the host's offer in) a package that is a Workshop item, with a Steam
sign-in: its whole item installed from the Workshop, MCC's shared sets too
when they are missing (no host sends them), and the host's offer let go; 1
when begun. The host sends it only when the Workshop cannot. */
static int mcc_share_install_begin(void)
{
#if defined(HALO_ANDROID)
    struct mcc_workshop_item item;
    unsigned long long id;
    if (!strcmp(mcc_share_install_tried,mcc_share_globals.level)) {
        error(_error_silent,"mcc share: %s from the Workshop was tried: asking the player (the host's alone)",
            mcc_share_globals.level);
        return 0;
    }
    if (!mcc_workshop_wire_unpack(mcc_share_name,&item) || !item.id[0] || !(id=strtoull(item.id,NULL,10))) {
        error(_error_silent,"mcc share: %s is not a Workshop item: asking the player (the host's alone)",
            mcc_share_globals.level);
        return 0;
    }
    snprintf(mcc_share_steam_root,sizeof(mcc_share_steam_root),"%s",platform_data_root());
    if (!host_steam_install_start(MCC_SHARE_HOST_ADDRESS(mcc_share_steam_root),(unsigned int)id,(unsigned int)(id>>32))) {
        error(_error_silent,"mcc share: no Steam sign-in: asking the player for %s (the host's alone)",
            mcc_share_globals.level);
        return 0;
    }
    snprintf(mcc_share_install_tried,sizeof(mcc_share_install_tried),"%s",mcc_share_globals.level);
    {
        char const *leaf=strrchr(mcc_share_globals.level,'\\');
        snprintf(mcc_share_install_level,sizeof(mcc_share_install_level),"mcc_maps\\%s\\%s",item.id,
            leaf ? leaf+1 : mcc_share_globals.level);
    }
    error(_error_silent,"mcc share: installing Workshop item %s for %s from Steam (as %s) instead of the host's download",
        item.id,mcc_share_globals.level,mcc_share_install_level);
    mcc_share_shared_asked=host_steam_shared_fetch(MCC_SHARE_HOST_ADDRESS(mcc_share_steam_root))==1;
    if (mcc_share_shared_asked)
        error(_error_silent,"mcc share: MCC's shared files are missing here: downloading them from Steam too");
    mcc_share_installing=TRUE;
    /* (the offer left waiting, not let go: while it lives the host counts this
       machine as fetching the package, and starts the game without it, which
       joins it once its package is in) */
    mcc_share_install_renewed_at=system_milliseconds();
    return 1;
#else
    return 0;
#endif
}

static void mcc_share_install_end(void)
{
    mcc_share_installing=FALSE;
    mcc_share_install_joining=FALSE;
    if (mcc_share_install_check) mcc_validation_check_end(mcc_share_install_check);
    mcc_share_install_check=NULL;
}

/* (the host's offer asked again before it lapses) */
static void mcc_share_install_renew(void)
{
#if defined(HALO_ANDROID)
    if (system_milliseconds()-mcc_share_install_renewed_at<MCC_SHARE_INSTALL_RENEW_MS) return;
    mcc_share_install_renewed_at=system_milliseconds();
    mcc_share_client_cancel(&mcc_share_globals);
    mcc_share_client_begin(&mcc_share_globals,mcc_share_install_tried,mcc_share_digest);
#endif
}

/* (each frame while installing) installed: the package checked here against
the host's identity, and, the host's, this machine's players let join, the
offer let go once the host has one; not installed, or not the host's: the
host asked again, and the player asked (the level tried) */
static void mcc_share_install_update(void)
{
#if defined(HALO_ANDROID)
    struct network_game_client *client;
    struct network_game *game;
    char const *why="could not be installed";
    int status;
    if (!mcc_share_installing && !mcc_share_install_joining) return;
    client=global_network_game_client_get();
    game=client ? network_game_client_get_game(client) : NULL;
    if (!game || global_network_game_server_get() || !mcc_share_name[0] ||
        memcmp(game->mcc_content_sha256,mcc_share_digest,32)) {
        mcc_share_install_end();
        return;
    }
    if (mcc_share_install_joining) {
        int i,joined=0;
        for (i=0;i<NUMBEROF(game->players);i++)
            joined|=network_player_is_valid(&game->players[i]) &&
                game->players[i].machine_index==network_game_client_get_machine_index(client);
        if (joined || system_milliseconds()-mcc_share_install_joining_at>=MCC_SHARE_INSTALL_JOIN_MS) {
            mcc_share_install_joining=FALSE;
            mcc_share_client_cancel(&mcc_share_globals);
        }
        return;
    }
    if (!mcc_share_install_check) {
        status=host_steam_install_status();
        /* (the shared sets, which the check reads, in first too) */
        if (!status || (status>0 && mcc_share_shared_pending())) {
            mcc_share_install_renew();
            return;
        }
        mcc_maps_rescan();
        if (status>0) {
            error(_error_silent,"mcc share: %s installed from the Workshop: checking it against the host's",
                mcc_share_install_level);
            mcc_share_install_check=mcc_validation_check_begin(mcc_share_install_level,NULL);
            mcc_validation_check_expect(mcc_share_install_check,mcc_share_digest);
            if (mcc_share_install_check) return;
        }
    } else {
        char reason[320];
        status=mcc_validation_check_step(mcc_share_install_check,&mcc_share_install_proof,reason,sizeof(reason));
        if (!status) {
            mcc_share_install_renew();
            return;
        }
        mcc_validation_check_end(mcc_share_install_check);
        mcc_share_install_check=NULL;
        why="is not the host's";
        if (status>0 && !memcmp(mcc_share_install_proof.digest,mcc_share_digest,32) &&
            (!game->map.version || mcc_share_install_proof.checksum==(uint32_t)game->map.version) &&
            mcc_validation_publish(mcc_share_install_level,&mcc_share_install_proof) &&
            mcc_identity_accept_installed(mcc_share_name,mcc_share_digest,mcc_share_metadata,mcc_share_install_level)) {
            error(_error_silent,"mcc share: %s from the Workshop is the host's: joining",mcc_share_install_level);
            mcc_share_release_validation();
            mcc_share_installing=FALSE;
            mcc_share_install_joining=TRUE;
            mcc_share_install_joining_at=system_milliseconds();
            return;
        }
    }
    mcc_share_install_end();
    mcc_share_client_cancel(&mcc_share_globals);
    error(_error_silent,"mcc share: Workshop item for %s %s: asking the host",mcc_share_install_tried,why);
    mcc_share_client_begin(&mcc_share_globals,mcc_share_install_tried,mcc_share_digest);
#endif
}

static void mcc_share_offered(void *user,uint64_t bytes,char const *credit)
{
    (void)user;
    /* The host's complete offer proves that its transfer hold exists. Keep
     * the validation lease until here, including request queue backpressure. */
    mcc_share_release_validation();
    mcc_share_prompt_download=TRUE;
    snprintf(mcc_share_prompt_message,sizeof(mcc_share_prompt_message),
        "The host's package is missing or differs. Download up to %.1f MiB from the host?\n"
        "Maps, required metadata/resources and supported previews are included.\n"
        "Your existing installation will be kept.\nMaker: %s",
        (double)bytes/(1024.0*1024.0),credit[0] ? credit : "Not provided");
    /* (an offer asked again while the Workshop installs: left waiting) */
    if ((mcc_share_installing || mcc_share_install_joining) &&
        !strcmp(mcc_share_install_tried,mcc_share_globals.level)) return;
    if (mcc_share_install_begin()) return;
    /* (the host the only source: the player asked) */
    mcc_share_prompt_pending=TRUE;
}

static int mcc_share_client_current(void)
{
    struct network_game_client *client=global_network_game_client_get();
    struct network_game *game=client ? network_game_client_get_game(client) : NULL;
    char canonical[64];
    return !global_network_game_server_get() && game && mcc_share_name[0] &&
        mcc_package_name(game->map.name,canonical) && !strcmp(canonical,mcc_share_name) &&
        !memcmp(game->mcc_content_sha256,mcc_share_digest,32) &&
        !memcmp(game->mcc_metadata_sha256,mcc_share_metadata,32);
}

static void mcc_share_close_prompt(void)
{
    mcc_share_prompt_pending=FALSE;
    /* Live consent returns to its lobby. Detached guidance has no lobby
     * history; its action queues the engine's selected main-menu root. */
    if (mcc_share_prompt) {
        if (mcc_share_manual==MCC_SHARE_MANUAL_MENU) ui_widget_delete(mcc_share_prompt);
        else ui_widget_port_go_back(mcc_share_prompt);
    }
    mcc_share_prompt_visible=FALSE;
}

int mcc_share_game_can_download(void)
{
    return mcc_share_client_current() && mcc_share_prompt_download && config_boolean("network.map_download") &&
        mcc_share_globals.state==MCC_SHARE_CLIENT_WAITING;
}
int mcc_share_game_has_workshop(void)
{
    struct mcc_workshop_item item;
    return mcc_share_name[0] && mcc_workshop_wire_unpack(mcc_share_name,&item);
}
void mcc_share_game_prompt_text(wchar_t text[512])
{
    unsigned i;
    for (i=0;mcc_share_prompt_message[i] && i<511;i++)
        text[i]=mcc_share_prompt_message[i]=='|' ? '/' : (unsigned char)mcc_share_prompt_message[i];
    text[i]=0;
}
void mcc_share_game_prompt_closed(void)
{
    mcc_share_prompt=NULL;
    mcc_share_prompt_visible=FALSE;
    if (mcc_share_manual==MCC_SHARE_MANUAL_MENU) {
        mcc_share_manual=MCC_SHARE_MANUAL_NONE;
        mcc_share_prompt_pending=FALSE;
        mcc_share_name[0]=0;
    }
}
void mcc_share_game_prompt_action(int action)
{
    struct mcc_workshop_item item;
#ifndef HALO_WEB
    char url[128];
#endif
    int manual=mcc_share_manual==MCC_SHARE_MANUAL_MENU;
    int current=manual ? !global_network_game_client_get() && !global_network_game_server_get() &&
        main_menu_is_active() : mcc_share_client_current();
    int download=mcc_share_game_can_download();
    int workshop=mcc_workshop_wire_unpack(mcc_share_name,&item);
    if (current && workshop && (action==1 || action==3)) {
#ifdef HALO_WEB
        /* Dedicated browser-side click; never broaden the account-link bridge. */
        extern void web_js_post(int kind,char const *text);
        web_js_post(6,item.id);
#else
        snprintf(url,sizeof(url),action==1 ? "steam://url/CommunityFilePage/%s" :
            "https://steamcommunity.com/sharedfiles/filedetails/?id=%s",item.id);
        platform_open_url(url);
#endif
        /* Keep both actions available: dispatch success cannot establish that
         * Steam navigated, and the player may need the browser fallback. */
        return;
    }
    mcc_share_close_prompt();
    if (!current) {
        mcc_share_client_cancel(&mcc_share_globals);
        return;
    }
    if (!manual && action==0 && download) {
        mcc_share_prompt_download=FALSE;
        if (mcc_share_client_accept(&mcc_share_globals)) mcc_share_shared_begin();
        return;
    }
    if (manual) {
        main_goto_main_menu();
        return;
    }
    mcc_share_client_cancel(&mcc_share_globals);
    network_game_abort();
}

void mcc_share_game_disconnect(void)
{
    int manual=mcc_share_manual==MCC_SHARE_MANUAL_LEAVING;
    /* (a Workshop install of the level tried, or under way, is this session's) */
    mcc_share_install_end();
    mcc_share_install_tried[0]=0;
    mcc_admission_disconnect();
    mcc_load_network_disconnect();
    mcc_share_validation_token=0;
    mcc_share_check_reason[0]=0;
    mcc_share_close_prompt();
    if (mcc_share_ready) {
        mcc_share_client_disconnect(&mcc_share_globals);
        mcc_share_stop_host();
    }
    mcc_share_prompt_pending=mcc_share_prompt_visible=mcc_share_prompt_download=FALSE;
    mcc_share_manual=manual ? MCC_SHARE_MANUAL_MENU : MCC_SHARE_MANUAL_NONE;
    if (manual) mcc_share_prompt_pending=TRUE;
    else mcc_share_name[0]=0;
}

void mcc_share_game_peer_left(struct network_game_server_client_machine *machine)
{
    struct network_game_server *server=global_network_game_server_get();
    long index;
    if (!server || !machine) return;
    for (index=0;index<MCC_SHARE_HOST_MACHINES;index++)
        if (network_game_server_get_client_machine_at_index(server,index)==machine) {
            mcc_admission_peer_left(index);
            mcc_load_network_machine_left(index);
            if (mcc_share_ready) mcc_share_host_forget(&mcc_share_globals,index);
            mcc_share_admission_pending[index]=0;
        }
}

static void mcc_share_host_lifecycle(void)
{
    struct network_game_server *server=global_network_game_server_get();
    struct network_game *game=network_game_get_game();
    int i;
    unsigned long revision=mcc_identity_host_revision();
    if (revision!=mcc_share_host_revision) {
        mcc_share_stop_host();
        mcc_share_host_revision=revision;
    }
    if (!server || !game || !config_boolean("network.map_share") || !mcc_level_name(game->map.name)) {
        mcc_share_stop_host();
        return;
    }
    if (strcmp(game->map.name,mcc_share_admission_level))
        memset(mcc_share_admission_pending,0,sizeof(mcc_share_admission_pending));
    for (i=0;i<NUMBEROF(game->players);i++) {
        struct network_player *player=&game->players[i];
        if (network_player_is_valid(player) && player->machine_index>=0 && player->machine_index<MCC_SHARE_HOST_MACHINES)
            mcc_share_admission_pending[player->machine_index]=0;
    }
    for (i=0;i<MCC_SHARE_HOST_MACHINES;i++)
        if (mcc_share_admission_pending[i] &&
            ((uint32_t)((uint32_t)system_milliseconds()-mcc_share_admission_at[i])>=15000u ||
             !network_game_server_client_machine_is_joined_to_game(server,
                 network_game_server_get_client_machine_at_index(server,i))))
            mcc_share_admission_pending[i]=0;
    for (i=0;i<MCC_SHARE_HOST_TRANSFERS;i++) {
        struct mcc_share_host_transfer *transfer=&mcc_share_globals.transfers[i];
        if (transfer->used && (strcmp(transfer->level,game->map.name) ||
            !network_game_server_client_machine_is_joined_to_game(server,
                network_game_server_get_client_machine_at_index(server,transfer->machine))))
            mcc_share_host_forget(&mcc_share_globals,transfer->machine);
    }
}

/* ---------- every frame */

void mcc_share_game_update(
	void)
{
	if (!mcc_share_ready)
	{
		struct mcc_share_env env;

		csmemset(&env, 0, sizeof(env));
		env.now = mcc_share_now;
		env.send_request = mcc_share_send_request;
		env.package_taken = mcc_share_package_taken;
		env.identity_check = mcc_share_identity_check;
        env.identity_error = mcc_share_identity_error;
        env.cancel_check = mcc_share_cancel_check;
        env.installed_ready = mcc_share_installed_ready;
        env.prepare_install = mcc_share_prepare_install;
        env.reuse_file = mcc_share_reuse_file;
		env.finished = mcc_share_finished;
		env.send_response = mcc_share_send_response;
		env.host_package = mcc_share_host_package_files;
		env.upload_limit = mcc_share_upload_limit;
        env.host_finished = mcc_share_host_finished;
        env.host_refused = mcc_share_host_refused;
#if defined(HALO_ANDROID)
        env.hash_begin = mcc_share_hash_begin;
        env.hash_poll = mcc_share_hash_poll;
        env.hash_cancel = mcc_share_hash_cancel;
#endif
		env.free_bytes = mcc_share_free_bytes;
		env.offered = mcc_share_offered;
		env.destination = mcc_share_destination;
		env.io.make_directory = mcc_share_io_make;
		env.io.remove_directory = mcc_share_io_remove;
		env.io.directory_exists = mcc_share_io_exists;
		env.io.move_file = mcc_share_io_move;
		env.io.delete_file = mcc_share_io_delete;
		env.io.path_safe = mcc_share_io_safe;
		env.io.publish_directory = mcc_share_io_publish;
        env.io.open_native_read = mcc_share_open_native_said;
#ifdef HALO_WEB
        web_mcc_share_io(&env.io);
#endif
		mcc_share_init(&mcc_share_globals, &env);
		mcc_share_ready = TRUE;
		mcc_identity_set_missing(mcc_share_missing);
	}
    if (mcc_share_manual==MCC_SHARE_MANUAL_MENU &&
        (global_network_game_client_get() || global_network_game_server_get() ||
         (mcc_share_prompt_visible && !main_menu_is_active()))) {
        mcc_share_close_prompt();
        mcc_share_manual=MCC_SHARE_MANUAL_NONE;
        mcc_share_name[0]=0;
    }
    mcc_admission_update();
    mcc_identity_update();
    /* Consent belongs to this host's exact settings, including the metadata. */
    if (mcc_share_manual==MCC_SHARE_MANUAL_NONE &&
        (mcc_share_client_busy(&mcc_share_globals) || mcc_share_prompt_pending || mcc_share_prompt_visible)) {
        if (!mcc_share_client_current()) {
            if (global_network_game_client_get()) mcc_share_client_cancel(&mcc_share_globals);
            else mcc_share_client_disconnect(&mcc_share_globals);
            mcc_share_close_prompt();
            mcc_share_prompt_download=FALSE;
            mcc_share_name[0]=0;
        } else if (mcc_share_client_busy(&mcc_share_globals) && !config_boolean("network.map_download")) {
            mcc_share_client_cancel(&mcc_share_globals);
            mcc_share_release_validation();
            mcc_share_prompt_download=FALSE;
            snprintf(mcc_share_prompt_message,sizeof(mcc_share_prompt_message),
                "Download Maps is OFF. Install matching content manually or visit its Workshop page when available.");
            mcc_share_prompt_pending=!mcc_share_prompt_visible;
        }
    }
    mcc_share_host_lifecycle();
    mcc_share_update(&mcc_share_globals);
#if defined(HALO_ANDROID)
    /* (files the Steam downloader put in mcc_maps, whatever put them there) */
    if (host_steam_take_rescan()) mcc_maps_rescan();
#endif
    mcc_share_install_update();
    if ((mcc_share_prompt_pending || mcc_share_prompt_visible) && !mcc_share_prompt_download &&
        mcc_share_manual==MCC_SHARE_MANUAL_NONE && mcc_share_client_current()) {
        /* There is no admission path without this content. Leave now rather
         * than letting the host's playerless timeout replace the guidance.
         * Disconnect retains only the text/origin for the main-menu prompt. */
        mcc_share_close_prompt();
        mcc_share_manual=MCC_SHARE_MANUAL_LEAVING;
        network_game_abort();
        return;
    }
    if (mcc_share_prompt_pending && !mcc_share_prompt_visible &&
        (mcc_share_manual==MCC_SHARE_MANUAL_MENU ? main_menu_is_active() :
         mcc_share_manual==MCC_SHARE_MANUAL_NONE && mcc_share_client_current())) {
        /* Deferred until frame update: receiving a packet never opens a widget. */
        mcc_share_prompt=ui_widget_load_by_name_or_tag(
            "pc\\main_menu\\settings_select\\player_setup\\player_profile_edit\\network_setup\\mcc_download\\screen",
            NONE,NULL,0,mcc_share_manual==MCC_SHARE_MANUAL_MENU ? NONE :
                tag_loaded('DeLa',"pc\\main_menu\\multiplayer_type_select\\lobby\\lobby_screen"),NONE,NONE);
        if (mcc_share_prompt) { mcc_share_prompt_pending=FALSE; mcc_share_prompt_visible=TRUE; }
    }
}

void mcc_share_game_render(
	void)
{
	long font_tag_index;
	real_argb_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
	rectangle2d bounds;
	char const *name;
	int percent;

	/* The consent screen owns this space until the player accepts its offer. */
	if (!mcc_share_ready || (!mcc_share_client_busy(&mcc_share_globals) && !mcc_share_installing) ||
        mcc_share_install_joining || mcc_share_prompt_pending || mcc_share_prompt_visible)
		return;
	font_tag_index = interface_get_tag_index(_interface_font_terminal);
	if (font_tag_index == NONE)
		return;
	name = strrchr(mcc_share_globals.level, '\\');
	name = name ? name + 1 : mcc_share_globals.level;
	percent = mcc_share_client_percent(&mcc_share_globals);
	if (mcc_share_installing)
	{
		/* (from the Workshop: its bytes and speed, in KB, once Steam has said how many) */
		uint32_t progress[3] = { 0, 0, 0 };
		char speed[32];

#if defined(HALO_ANDROID)
		host_steam_install_progress(MCC_SHARE_HOST_ADDRESS(progress));
#endif
		speed[0] = 0;
		if (progress[2] >= 1024)
			snprintf(speed, sizeof(speed), " %.1f MB/s", (double)progress[2] / 1024.0);
		else if (progress[2])
			snprintf(speed, sizeof(speed), " %lu KB/s", (unsigned long)progress[2]);
		if (progress[0] > progress[1])
			progress[0] = progress[1];
		if (!progress[1])
			snprintf(mcc_share_text, sizeof(mcc_share_text), "Downloading %s from the Workshop...", name);
		/* (in MB past 150 MB, where kilobytes are too many digits to read) */
		else if (progress[1] > 150 * 1024)
			snprintf(mcc_share_text, sizeof(mcc_share_text), "Downloading %s from the Workshop %lu/%lu MB%s", name,
				(unsigned long)(progress[0] / 1024), (unsigned long)(progress[1] / 1024), speed);
		else
			snprintf(mcc_share_text, sizeof(mcc_share_text), "Downloading %s from the Workshop %lu/%lu KB%s", name,
				(unsigned long)progress[0], (unsigned long)progress[1], speed);
	}
	else if (mcc_share_globals.state==MCC_SHARE_CLIENT_VERIFYING)
        snprintf(mcc_share_text,sizeof(mcc_share_text),"Verifying the downloaded map %s...",name);
	else if (percent <= 0)
		snprintf(mcc_share_text, sizeof(mcc_share_text), "Asking the host for the map %s...", name);
	else
		snprintf(mcc_share_text, sizeof(mcc_share_text), "Getting the map %s from the host: %d%%", name, percent);
	bounds = render.camera.window_bounds;
	offset_rectangle2d(&bounds, -render.camera.viewport_bounds.x0, -render.camera.viewport_bounds.y0);
	bounds.y0 = 24;
	bounds.y1 = 42;
	draw_string_set_draw_mode(font_tag_index, NONE, _mcc_share_text_centered, 0, &color);
	rasterizer_draw_string(&bounds, NULL, NULL, 0, mcc_share_text);
}

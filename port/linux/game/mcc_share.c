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
static int mcc_share_identity_check(void *user, char const *stage_dir)
{
    struct mcc_share_metadata_entry *entries;
    unsigned char actual[32],keep[MCC_SHARE_METADATA_FILES];
    int indices[MCC_SHARE_METADATA_FILES],count=0,i,ok=0;
    char native[1024],reason[320];
    char const *leaf=strrchr(mcc_share_globals.level,'\\')+1;
    (void)user;
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

static int mcc_share_reuse_file(void *user,struct mcc_share_file const *file,char path[1024])
{
    char root[1024];
    struct mcc_share_io io;
    (void)user;
    platform_translate_path("d:\\mcc_maps",root,sizeof(root));
    mcc_share_storage_native_io(&io);
    return mcc_share_storage_find_resource(root,&io,file,path);
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
		(mcc_share_client_level(&mcc_share_globals, canonical) ||
        ((mcc_share_prompt_pending || mcc_share_prompt_visible) && !strcmp(canonical,mcc_share_name)));
}

boolean mcc_share_game_holds_players(
	void)
{
	return mcc_identity_pending(NULL) || mcc_admission_client_pending() ||
        (mcc_share_ready && mcc_share_manual==MCC_SHARE_MANUAL_NONE && (mcc_share_client_busy(&mcc_share_globals) ||
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
        mcc_share_client_accept(&mcc_share_globals);
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
        env.io.open_native_read = mcc_share_open_native;
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
	if (!mcc_share_ready || !mcc_share_client_busy(&mcc_share_globals) ||
        mcc_share_prompt_pending || mcc_share_prompt_visible)
		return;
	font_tag_index = interface_get_tag_index(_interface_font_terminal);
	if (font_tag_index == NONE)
		return;
	name = strrchr(mcc_share_globals.level, '\\');
	name = name ? name + 1 : mcc_share_globals.level;
	percent = mcc_share_client_percent(&mcc_share_globals);
	if (mcc_share_globals.state==MCC_SHARE_CLIENT_VERIFYING)
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

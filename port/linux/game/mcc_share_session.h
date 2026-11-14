/* MCC package sharing between a host and a joiner: the request and response
 * messages, the host's serving of one package and the joiner's staged download.
 *
 * This is a message pair of its own for MCC games (mcc_share.c carries them);
 * the Custom Edition map sharing messages are not used or changed, and only
 * peers with the current MCC agreement exchange these. The core has no engine
 * dependencies: everything it
 * needs from the game (the clock, the connection, the host's admission, the
 * joiner's identity check) comes through struct mcc_share_env, so tests run
 * a host and a joiner against each other with real files. */
#ifndef MCC_SHARE_SESSION_H
#define MCC_SHARE_SESSION_H
#include <stdint.h>
#include "mcc_share_stage.h"
#include "mcc_share_list.h"
#include "mcc_share_rules.h"

#define MCC_SHARE_LEVEL_SIZE 64
#define MCC_SHARE_CHUNK_BYTES 3072
#define MCC_SHARE_WINDOW 16
/* The joiner's chunk requests in flight: every host queues MCC_SHARE_WINDOW * 2
 * (32) of a transfer and ignores more, which stalls it, so never above that. */
#define MCC_SHARE_CLIENT_WINDOW (MCC_SHARE_WINDOW * 2)
#define MCC_SHARE_HOST_TRANSFERS 4
#define MCC_SHARE_HOST_MACHINES 128
#define MCC_SHARE_MANIFEST_PAGE 16

enum mcc_share_op {
    MCC_SHARE_OP_MANIFEST = 1,   /* request: what files make the level; response: the list */
    MCC_SHARE_OP_CHUNK = 2,      /* request: bytes of one file; response: those bytes */
    MCC_SHARE_OP_DONE = 3,       /* request only: the joiner has finished (flags 1) or given up (0) */
    MCC_SHARE_OP_HAVE = 4        /* request only: this resource was copied locally and verified */
};

enum mcc_share_result {
    MCC_SHARE_RESULT_OK,
    MCC_SHARE_RESULT_OFF,            /* the host does not share MCC packages */
    MCC_SHARE_RESULT_NOT_SHAREABLE,  /* stock, loose, _shared, unknown or not the game's map */
    MCC_SHARE_RESULT_NO_CREDIT,      /* no maker is credited for it: the joiner is pointed to the maker */
    MCC_SHARE_RESULT_BUSY,
    MCC_SHARE_RESULT_UNAVAILABLE,    /* the host could not read or hash the files */
    MCC_SHARE_RESULT_BAD_REQUEST,
    MCC_SHARE_RESULT_CHANGED         /* the transfer is not (or no longer) the host's */
};

/* What goes on the wire after the distributed message header. Integers are
 * the machines' own (all supported machines are little-endian). */
struct mcc_share_request {
    uint8_t op;
    uint8_t file;                    /* chunk: the manifest's file number */
    uint8_t flags;                   /* done: 1 for a successful install */
    uint8_t reserved;                /* zero */
    uint32_t session;                /* the host's number from its manifest; 0 for the manifest request */
    uint32_t offset;
    uint32_t length;                 /* chunk: at most MCC_SHARE_CHUNK_BYTES */
    char level[MCC_SHARE_LEVEL_SIZE];/* the game's level name, mcc_maps\<package>\<level> */
};
struct mcc_share_response {
    uint8_t op;
    uint8_t result;                  /* enum mcc_share_result */
    uint8_t file;
    uint8_t count;                   /* manifest: total entries across all pages */
    uint32_t session;
    uint32_t offset;                 /* manifest: first entry index; chunk: byte offset */
    uint32_t length;                 /* manifest: entry count in page; chunk: bytes following */
    char level[MCC_SHARE_LEVEL_SIZE];
    char credit[MCC_SHARE_CREDIT_SIZE]; /* manifest: the maker the host credits */
};
struct mcc_share_entry {
    uint8_t kind;                    /* enum mcc_share_file_kind */
    uint8_t reserved[3];
    uint32_t size;
    char leaf[MCC_SHARE_LEAF_SIZE];
    uint8_t sha256[32];
};
typedef char mcc_share_request_size[sizeof(struct mcc_share_request) == 80 ? 1 : -1];
typedef char mcc_share_response_size[sizeof(struct mcc_share_response) == 144 ? 1 : -1];
typedef char mcc_share_entry_size[sizeof(struct mcc_share_entry) == MCC_SHARE_LEAF_SIZE + 40 ? 1 : -1];
typedef char mcc_share_manifest_page_size[sizeof(struct mcc_share_entry)*MCC_SHARE_MANIFEST_PAGE <= MCC_SHARE_CHUNK_BYTES ? 1 : -1];
typedef char mcc_share_manifest_count_bound[MCC_SHARE_STAGE_FILES <= 255 ? 1 : -1];
/* The largest response: a head and either a full manifest or a full chunk. */
#define MCC_SHARE_RESPONSE_MAXIMUM (sizeof(struct mcc_share_response) + MCC_SHARE_CHUNK_BYTES)

/* What the host's admission found for the game's package. */
struct mcc_share_host_package {
    enum mcc_share_result refusal;   /* OK when the package may be offered */
    char credit[MCC_SHARE_CREDIT_SIZE];
    int count;
    struct {
        uint8_t kind;
        char leaf[MCC_SHARE_LEAF_SIZE];
        char path[1024];             /* where to read it */
    } files[MCC_SHARE_STAGE_FILES];
};

struct mcc_share_env {
    void *user;
    uint32_t (*now)(void *user);                                 /* milliseconds */
    /* joiner */
    /* Send callbacks return nonzero only after the reliable stream accepts the message. */
    int (*send_request)(void *user, struct mcc_share_request const *request);
    int (*package_taken)(void *user, char const *level);         /* an installed map has this name */
    int (*identity_check)(void *user, char const *stage_dir);    /* enum mcc_share_check_result */
    /* Optional local diagnostic retained through cancel_check until the next check. */
    char const *(*identity_error)(void *user);
    void (*cancel_check)(void *user);                          /* release only this download's identity job */
    /* Promote the completed proof and bind the published content before DONE.
     * Failure leaves the installed package intact but refuses admission. */
    int (*installed_ready)(void *user);
    /* Optional local receipt/companion creation after verification, before publication.
     * Use stage-owned companion APIs so cancellation also removes partial work. */
    int (*prepare_install)(void *user, struct mcc_share_stage *stage);
    /* Optional verified-receipt candidate for bitmaps/sounds only. Return zero
     * when absent; the core copies and hashes the actual bytes before HAVE. */
    int (*reuse_file)(void *user, struct mcc_share_file const *file, char path[1024]);
    /* Optional, bitmaps/sounds only, asked when the host's turn comes to a file
     * reuse_file did not find: nonzero holds the host's request for it a while
     * (the file is on its way from elsewhere, sooner than the host would send
     * it: host_rate is the host's bytes a second so far, 0 not known). The core
     * asks again each update and holds a file at most MCC_SHARE_HOLD_LIMIT. */
    int (*reuse_wait)(void *user, struct mcc_share_file const *file, uint32_t host_rate);
    void (*finished)(void *user, int ok, char const *message);   /* installed, or why not */
    /* host */
    int (*send_response)(void *user, long machine, void const *message, unsigned bytes);
    /* Admission for a request naming a level: sets package->refusal, and when OK the files.
     * The core still checks the manifest it builds. */
    void (*host_package)(void *user, char const *level, struct mcc_share_host_package *package);
    uint32_t (*upload_limit)(void *user);                        /* bytes a second */
    /* Valid successful DONE, once after releasing its transfer slot. The
     * adapter may allow bounded time for the client's following AddPlayer. */
    void (*host_finished)(void *user, long machine);
    /* Optional: a transfer the host gave up before offering it (the reply says only
     * its result), and why, for the host's log. */
    void (*host_refused)(void *user, long machine, char const *level, int result, char const *why);
    /* Optional: a file's SHA-256 and size worked out elsewhere (the host's
     * threads), not a slice a frame here. begin: a job, or -1 (hashed here);
     * poll: 1 done, 0 not yet, -1 failed; cancel releases a job not done. */
    int (*hash_begin)(void *user, char const *path);
    int (*hash_poll)(void *user, int job, uint8_t digest[32], uint64_t *size);
    void (*hash_cancel)(void *user, int job);
    /* joiner: the bytes free where the package will go, or 0 when that is not known */
    uint64_t (*free_bytes)(void *user);
    /* No files are written before accept. NULL retains automatic acceptance for callers
     * without a UI; the game always supplies this callback. */
    void (*offered)(void *user, uint64_t total_bytes, char const *credit);
    /* Select a managed snapshot folder after consent, without changing the wire level. */
    int (*destination)(void *user, char const *level, struct mcc_share_file const *files,
        int count, char folder[64]);
    struct mcc_share_io io;
    char root[200];                  /* prefix of the staging and mcc_maps folders, "" or ending in / */
};

struct mcc_share_host_transfer {
    int used;
    long machine;
    int hashing;                     /* nonzero until every file has been hashed */
    uint32_t session, heard_at, advanced_at, offered_at;
    int manifest_pending;
    int offer_complete;              /* first complete offer starts consent expiry once */
    unsigned manifest_cursor;
    int request_file;
    uint32_t request_offset;
    char level[MCC_SHARE_LEVEL_SIZE];
    char credit[MCC_SHARE_CREDIT_SIZE];
    struct mcc_share_host_package package;
    struct mcc_share_file files[MCC_SHARE_STAGE_FILES];
    int hash_file;                   /* hashing: the file in hand and how far */
    uint32_t hash_offset;
    struct mcc_sha256 hash;
    FILE *hash_stream;
    /* hash_begin's job for the file in hand (hash_job_active: there is one),
     * and what releases it should the transfer end first */
    int hash_job, hash_job_active;
    void (*hash_job_cancel)(void *user, int job);
    void *hash_job_user;
    /* the file being served, kept open between chunks (serve_file: its number, -1 none) */
    FILE *serve_stream;
    int serve_file;
    uint32_t serve_offset;
    struct {
        uint8_t file;
        uint32_t offset, length;
    } queue[MCC_SHARE_WINDOW * 2];
    int queued;
    uint32_t sent_bytes;
    unsigned reused_files;
};

enum mcc_share_client_state {
    MCC_SHARE_CLIENT_IDLE,
    MCC_SHARE_CLIENT_ASKING,
    MCC_SHARE_CLIENT_WAITING,
    MCC_SHARE_CLIENT_TRANSFERRING,
    MCC_SHARE_CLIENT_VERIFYING
};

struct mcc_share {
    struct mcc_share_env env;
    struct mcc_share_host_transfer transfers[MCC_SHARE_HOST_TRANSFERS];
    uint32_t next_session;
    uint32_t budget_at, budget_fraction;
    int32_t budget;                  /* One upload allowance shared by all clients. */
    unsigned serve_first;
    uint32_t manifest_at[MCC_SHARE_HOST_MACHINES];  /* the last manifest request, per machine */
    uint8_t manifest_seen[MCC_SHARE_HOST_MACHINES];
    /* The last package hashed, so that new transfers of it (another machine,
     * or one that asks again) do not read it all again: the SHA-256 of its
     * level, kinds, leaves and paths, and its files' sizes and digests. */
    struct {
        int valid;
        uint32_t at;
        uint8_t key[32];
        int count;
        struct mcc_share_file files[MCC_SHARE_STAGE_FILES];
    } hashed;
    /* the joiner's download */
    enum mcc_share_client_state state;
    char level[MCC_SHARE_LEVEL_SIZE];
    uint8_t digest[32];
    uint32_t session, asked_at, progress_at, reported_at;
    int ask_pending;
    int terminal_pending;
    uint32_t terminal_at;
    struct mcc_share_request terminal; /* retry DONE only on the original connection */
    struct mcc_share_file offer_files[MCC_SHARE_STAGE_FILES];
    int offer_count;
    int offer_received;
    uint64_t offer_total;
    int request_file;
    uint32_t request_offset;
    int outstanding;
    FILE *reuse_stream;
    int reuse_checked_file, reuse_file, have_pending;
    /* the file whose host request reuse_wait holds, and since when */
    int hold_file;
    uint32_t hold_since;
    /* the host's bytes a second (smoothed), and the bytes and time it was last measured from */
    uint32_t host_rate, host_rate_bytes, host_rate_at, host_bytes;
    char credit[MCC_SHARE_CREDIT_SIZE];
    struct mcc_share_stage stage;
};

/* Host admission requires sharing enabled and a validated package identity.
 * The optional legacy list supplies attribution, never permission or identity.
 * credit may be empty. The package planner separately excludes base resources. */
enum mcc_share_result mcc_share_admit(char const *level, int opt_in, enum mcc_share_sidecar sidecar,
    unsigned char const digest[32], int digest_known, char const *list, unsigned long list_length,
    char credit[MCC_SHARE_CREDIT_SIZE]);

void mcc_share_init(struct mcc_share *share, struct mcc_share_env const *env);
/* Every frame: the host hashes and serves, the joiner checks timeouts and
 * finishes (verify, install, report). */
void mcc_share_update(struct mcc_share *share);

/* ---------- the host */
void mcc_share_host_request(struct mcc_share *share, long machine, void const *message, unsigned bytes);
/* Whether the machine's transfer is going on (it asked lately and its bytes grew lately). */
int mcc_share_host_active(struct mcc_share const *share, long machine);
void mcc_share_host_forget(struct mcc_share *share, long machine);
/* The game ended or this machine stopped hosting: drop every transfer. */
void mcc_share_host_stop(struct mcc_share *share);

/* ---------- the joiner */
/* Ask the host for the level's package. digest is the host's content identity
 * (the game record's). 0 when the name is not a shareable package name or a
 * download is already going. */
int mcc_share_client_begin(struct mcc_share *share, char const *level, unsigned char const digest[32]);
/* Starts the staged transfer only for the current validated, unexpired offer. */
int mcc_share_client_accept(struct mcc_share *share);
void mcc_share_client_response(struct mcc_share *share, void const *message, unsigned bytes);
void mcc_share_client_cancel(struct mcc_share *share);
/* Disconnect/host replacement: discard notification retries for the old peer. */
void mcc_share_client_disconnect(struct mcc_share *share);
int mcc_share_client_busy(struct mcc_share const *share);
int mcc_share_client_level(struct mcc_share const *share, char const *level);
/* 0 to 100, or -1 when no download goes on */
int mcc_share_client_percent(struct mcc_share const *share);
void mcc_share_refusal_text(enum mcc_share_result result, char const *level, char *text, unsigned size);
#endif

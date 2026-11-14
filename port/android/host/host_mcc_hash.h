/*
HOST_MCC_HASH.H

An MCC map's content fingerprint, worked out by the host
(host_mcc_hash.c): for the guest's package validation in the lobby
(host_imports.list's host_mcc_hash_*), and for a package the Steam
Workshop screen has just installed (steam_downloads.c), whose maps' results
it keeps on the card so the lobby has nothing to read again.
*/

#ifndef HOST_MCC_HASH_H
#define HOST_MCC_HASH_H

/* each map of an installed package folder (an absolute path) fingerprinted
and kept, with the package's own bitmaps.map and sounds.map, in every way
the guest may ask for it; cancel (may be NULL) stops it when it becomes
nonzero; progress (may be NULL) is told the bytes read of all to read.
The maps it did; -1 if it could not begin. */
int host_mcc_hash_package(char const *data_root, char const *folder, volatile int const *cancel,
	void (*progress)(unsigned long long done, unsigned long long total, void *context), void *context);

/* a file's plain SHA-256 (the CPU's instructions when it has them): 1, or 0
when it could not be read or cancel (may be NULL) became nonzero */
int host_sha256_file(char const *path, unsigned char digest[32], volatile int const *cancel);

/* the guest's (guest_host.h) */
int host_mcc_hash_start(unsigned int request);
int host_mcc_hash_wait(int id, int milliseconds, unsigned int result);
void host_mcc_hash_cancel(int id);
/* a file's plain SHA-256 and size on a thread of its own: start (the path's
guest address; -1 none free), poll (1 done, 0 reading, -1 failed), cancel */
int host_sha256_start(unsigned int path);
int host_sha256_poll(int id, unsigned int digest, unsigned int size);
void host_sha256_cancel(int id);

#endif

/*
WORKSHOP_AUTHORS.H

The names of Steam Workshop items' authors, for the lobby's map panel (menu_functions.c) and Android's Steam
Workshop screen (port/android/host/steam_downloads.c). No file on disk names an item's author, so Steam is asked as
an item is first wanted: its creator from the public web API (ISteamRemoteStorage/GetPublishedFileDetails, which
needs no key), then the name on that account's profile (steamcommunity.com/profiles/<id>/?xml=1). Each item is asked
about once a run, on a thread of its own, and nothing is written to disk.

The desktop game and the Android app's host make the requests; the Android guest asks its host
(host_steam_author), and the web build has no author: Steam's pages refuse a page's requests from another site.
*/

#ifndef __HALO_LINUX_WORKSHOP_AUTHORS_H
#define __HALO_LINUX_WORKSHOP_AUTHORS_H

enum
{
	/* Steam's names are at most 32 characters; this many bytes of their UTF-8, with its terminator */
	WORKSHOP_AUTHORS_NAME_SIZE = 100,
};

/* the author's name of the Workshop item `item_id` (decimal digits): 1 with the name in `name` once Steam has said;
else 0, the item asked about (once a run, or again 10 minutes after Steam could not say) */
int workshop_authors_get(const char *item_id, char *name, int size);

/* ---------- Steam's answers (untrusted text, `length` bytes, not NUL terminated) */

/* GetPublishedFileDetails' answer: the decimal id of the creator of the item `item_id`, 1 if it names one */
int workshop_authors_read_creator(const char *text, int length, const char *item_id, char creator[24]);
/* a profile's ?xml=1 answer: its <steamID>, as printable UTF-8; 1 if it holds a name */
int workshop_authors_read_name(const char *text, int length, char name[WORKSHOP_AUTHORS_NAME_SIZE]);

#endif

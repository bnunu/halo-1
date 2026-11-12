/*
STEAM_DOWNLOADS.H

The Steam Workshop screen's model, in the host (Settings > Network Setup >
STEAM WORKSHOP: tools/port_settings.py's _steam_workshop, the guest's
port/linux/src/steam_screen.c and menu_functions.c's "port steam"
functions, which reach it through the host_steam_* imports). Signing in
with Steam (a QR code the Steam mobile app scans, or the account's name and
password; the refresh token kept sealed: steam_seal.h), then a hub of three
lists: MANAGE ACCOUNT (the account, LOG OUT, which revokes the sign-in with
Steam), MCC FILE STATUS (the shared sets, and the Workshop packages in
mcc_maps: updated or removed) and BROWSE WORKSHOP (the Halo 1 Workshop by
sort and type, a page at a time, with each item's preview picture).
Downloads go into mcc_maps, where the MCC loader reads them
(docs/mcc_maps.md):

- the shared sets, from Halo: CE Anniversary's depot of MCC (976733):
  halo1/maps/bitmaps.map to mcc_maps/_shared/singleplayer/bitmaps.map,
  halo1/maps/custom_edition/bitmaps.map and sounds.map to
  mcc_maps/_shared/multiplayer/;
- an item, from MCC's Workshop depot, to mcc_maps/<item id>/: its .map files
  (of maps/ or halo1/maps/) at the folder's top, its JSON and PNG files at
  their paths, lowercase, and workshop_id.txt (its id and title), as the
  map lists' IMPORT lays a package out (host_mcc_import.c). It is made in
  mcc_staging/steam_<id> and moved into place whole, an older copy of the
  item replaced.

The account must own MCC and Halo: CE Anniversary: Steam gives the depots'
keys only then. Steam's servers are reached by threads of this module's;
the screen asks for what it shows each frame.

From Halo Content Evolved (github.com/Watanoge/halo-content-evolved,
AGPL-3.0-or-later, as this port), moved from its guest to the hosts.
*/

#ifndef STEAM_DOWNLOADS_H
#define STEAM_DOWNLOADS_H

#include <stdint.h>

/* the screen opened (data_root: the guest's data folder; a sign-in kept is
resumed at the opening) and closed */
void steam_downloads_open(char const *data_root);
void steam_downloads_close(void);
/* each frame the screen is up: nonzero once when files were put in
mcc_maps (the caller rescans the MCC catalog: mcc_maps_rescan) */
int steam_downloads_poll(void);

/* the rows of the list shown */
int steam_downloads_count(void);
void steam_downloads_row(int place, char *name, int name_size, char *state, int state_size);
void steam_downloads_details(int place, char *title, int title_size, char *text, int text_size);
/* A on a row */
void steam_downloads_choose(int place);
/* B: nonzero when it was taken (a sign-in under way cancelled, a list left
for the hub, or, at the hub, the screen kept while downloads are under way
or queued) */
int steam_downloads_back(void);

/* the list shown: 0 the hub, 1 the account, 2 MCC's files and the
installed Workshop items, 3 the Workshop */
int steam_downloads_view(void);

/* the chosen row's preview picture (a Workshop item's), 256 by 256
0xAARRGGBB pixels copied to pixels (may be NULL): 256, with *key the item's
id, or 0 while there is none */
int steam_downloads_picture(uint64_t *key, uint32_t *pixels);

/* text the screen is to have typed (the on-screen keyboard: a search, the
account's name, its password, a Steam Guard code): its kind (0: none),
asked for once, with the text to start from and whether it is a secret;
and what was typed (saved: OK pressed, not cancelled) */
int steam_downloads_text_wanted(char *initial, int initial_size, int *secret);
char const *steam_downloads_text_caption(int kind);
void steam_downloads_text_typed(int kind, char const *text, int saved);

/* the sign-in's QR code, while there is one to scan: its side in modules,
each a byte (1 dark) of modules (side * side of them, at most capacity),
with *serial changing with the code; 0 when there is none */
int steam_downloads_qr(unsigned char *modules, int capacity, unsigned *serial);

/* ---------- the guest's (port/android/host_imports.list; guest_host.h
declares them, port/linux/src/steam_screen.c wraps them): its addresses */

void host_steam_open(unsigned int data_root);
void host_steam_close(void);
int host_steam_poll(void);
int host_steam_count(void);
int host_steam_view(void);
int host_steam_first(void);
void host_steam_set_first(int first);
int host_steam_chosen(void);
void host_steam_set_chosen(int chosen);
void host_steam_row(int place, unsigned int name, int name_size, unsigned int state, int state_size);
void host_steam_details(int place, unsigned int title, int title_size, unsigned int text, int text_size);
void host_steam_choose(int place);
int host_steam_back(void);
int host_steam_picture(unsigned int key, unsigned int pixels);
int host_steam_text_wanted(unsigned int initial, int initial_size, unsigned int secret);
void host_steam_text_caption(int kind, unsigned int text, int text_size);
void host_steam_text_typed(int kind, unsigned int text, int saved);
int host_steam_qr(unsigned int modules, int capacity, unsigned int serial);

#endif

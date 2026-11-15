/* MCC-only integration with the dedicated server's rotation and catalog. */
#ifndef MCC_SERVER_H
#define MCC_SERVER_H
struct server_entry;
/* Reload requests wait for a pregame frame; HTTP reads never trigger scans. */
void mcc_server_request_rescan(void);
void mcc_server_update(int pregame);
/* Display only: NULL leaves the legacy filename label in place. */
wchar_t const *mcc_server_map_title(char const *level);
/* 0: legacy request, 1: quoted MCC command, -1: invalid MCC input. */
int mcc_server_play_line(char const *map,char const *gametype,char *line,int size);
void mcc_server_resolve(struct server_entry *entry);
int mcc_server_maps_each(void (*each)(void *, const char *, const char *, const char *), void *context);
#endif

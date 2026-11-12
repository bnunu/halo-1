/*
STEAM_SCREEN.H

The Steam Workshop screen's calls in the guest (menu_functions.c's "port
steam" functions): its model and its Steam client are the host's
(port/android/host/steam_downloads.c, which says what each of these does),
reached through the host_steam_* imports. The guest never holds a Steam
token: it shows rows, details, a QR code and a picture, and passes on what
the player typed (a password included, wiped here once passed).
*/

#ifndef STEAM_SCREEN_H
#define STEAM_SCREEN_H

/* the kinds of text steam_screen_text_wanted asks for */
enum
{
	STEAM_TEXT_NONE,
	STEAM_TEXT_SEARCH,
	STEAM_TEXT_ACCOUNT,
	STEAM_TEXT_PASSWORD,
	STEAM_TEXT_CODE
};

enum
{
	/* the preview picture's side (0xAARRGGBB pixels) */
	STEAM_PICTURE_SIDE = 256,
	/* the most modules a sign-in's QR code has (version 10: 57 a side) */
	STEAM_QR_MODULES = 57 * 57
};

void steam_screen_open(void);
void steam_screen_close(void);
int steam_screen_poll(void);
int steam_screen_count(void);
int steam_screen_view(void);
int steam_screen_first(void);
void steam_screen_set_first(int first);
int steam_screen_chosen(void);
void steam_screen_set_chosen(int chosen);
void steam_screen_row(int place, char *name, int name_size, char *state, int state_size);
void steam_screen_details(int place, char *title, int title_size, char *text, int text_size);
void steam_screen_choose(int place);
int steam_screen_back(void);
/* pixels: STEAM_PICTURE_SIDE squared, or NULL to ask only */
int steam_screen_picture(unsigned long long *key, unsigned int *pixels);
int steam_screen_text_wanted(char *initial, int initial_size, int *secret);
void steam_screen_text_caption(int kind, char *text, int text_size);
/* (text wiped once the host has it) */
void steam_screen_text_typed(int kind, char *text, int saved);
/* modules: STEAM_QR_MODULES bytes */
int steam_screen_qr(unsigned char *modules, unsigned *serial);

#endif

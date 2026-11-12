/*
STEAM_SCREEN.C

steam_screen.h's: the host's Steam Workshop model, called with the guest's
addresses (32-bit numbers, which are the host's).
*/

#include "steam_screen.h"

#include <stdint.h>
#include <string.h>

const char *platform_data_root(void);

#if defined(HALO_ANDROID)
/* the host's (port/android/guest/runtime/guest_host.h) */
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
#else
/* (no host outside Android: the screen's row is Android's alone,
tools/port_settings.py, and these keep the other builds linking) */
static void host_steam_open(unsigned int data_root) { (void)data_root; }
static void host_steam_close(void) {}
static int host_steam_poll(void) { return 0; }
static int host_steam_count(void) { return 0; }
static int host_steam_view(void) { return 0; }
static int host_steam_first(void) { return 0; }
static void host_steam_set_first(int first) { (void)first; }
static int host_steam_chosen(void) { return 0; }
static void host_steam_set_chosen(int chosen) { (void)chosen; }
static void host_steam_row(int place, unsigned int name, int name_size, unsigned int state, int state_size)
{
	(void)place; (void)name; (void)name_size; (void)state; (void)state_size;
}
static void host_steam_details(int place, unsigned int title, int title_size, unsigned int text, int text_size)
{
	(void)place; (void)title; (void)title_size; (void)text; (void)text_size;
}
static void host_steam_choose(int place) { (void)place; }
static int host_steam_back(void) { return 0; }
static int host_steam_picture(unsigned int key, unsigned int pixels) { (void)key; (void)pixels; return 0; }
static int host_steam_text_wanted(unsigned int initial, int initial_size, unsigned int secret)
{
	(void)initial; (void)initial_size; (void)secret;
	return 0;
}
static void host_steam_text_caption(int kind, unsigned int text, int text_size) { (void)kind; (void)text; (void)text_size; }
static void host_steam_text_typed(int kind, unsigned int text, int saved) { (void)kind; (void)text; (void)saved; }
static int host_steam_qr(unsigned int modules, int capacity, unsigned int serial)
{
	(void)modules; (void)capacity; (void)serial;
	return 0;
}
#endif

#define HOST_ADDRESS(pointer) ((unsigned int)(uintptr_t)(pointer))

void steam_screen_open(void)
{
	static char root[1024];

	strncpy(root, platform_data_root(), sizeof(root) - 1);
	host_steam_open(HOST_ADDRESS(root));
}

void steam_screen_close(void)
{
	host_steam_close();
}

int steam_screen_poll(void)
{
	return host_steam_poll();
}

int steam_screen_count(void)
{
	return host_steam_count();
}

int steam_screen_view(void)
{
	return host_steam_view();
}

int steam_screen_first(void)
{
	return host_steam_first();
}

void steam_screen_set_first(int first)
{
	host_steam_set_first(first);
}

int steam_screen_chosen(void)
{
	return host_steam_chosen();
}

void steam_screen_set_chosen(int chosen)
{
	host_steam_set_chosen(chosen);
}

void steam_screen_row(int place, char *name, int name_size, char *state, int state_size)
{
	host_steam_row(place, HOST_ADDRESS(name), name_size, HOST_ADDRESS(state), state_size);
}

void steam_screen_details(int place, char *title, int title_size, char *text, int text_size)
{
	host_steam_details(place, HOST_ADDRESS(title), title_size, HOST_ADDRESS(text), text_size);
}

void steam_screen_choose(int place)
{
	host_steam_choose(place);
}

int steam_screen_back(void)
{
	return host_steam_back();
}

int steam_screen_picture(unsigned long long *key, unsigned int *pixels)
{
	uint32_t halves[2] = { 0, 0 };
	int side = host_steam_picture(HOST_ADDRESS(halves), HOST_ADDRESS(pixels));

	*key = (unsigned long long)halves[1] << 32 | halves[0];
	return side;
}

int steam_screen_text_wanted(char *initial, int initial_size, int *secret)
{
	return host_steam_text_wanted(HOST_ADDRESS(initial), initial_size, HOST_ADDRESS(secret));
}

void steam_screen_text_caption(int kind, char *text, int text_size)
{
	host_steam_text_caption(kind, HOST_ADDRESS(text), text_size);
}

void steam_screen_text_typed(int kind, char *text, int saved)
{
	volatile char *at = text;

	host_steam_text_typed(kind, HOST_ADDRESS(text), saved);
	while (*at)
		*at++ = 0;
}

int steam_screen_qr(unsigned char *modules, unsigned *serial)
{
	return host_steam_qr(HOST_ADDRESS(modules), STEAM_QR_MODULES, HOST_ADDRESS(serial));
}

/* Halo 1 MCC map discovery. This list never includes the Xbox or CE folders. */
#ifndef __MCC_MAPS_H
#define __MCC_MAPS_H

#define MCC_MAPS_DIRECTORY "d:\\mcc_maps\\"
#define MCC_MAPS_LEVEL_PREFIX "mcc_maps\\"
#define MCC_MAPS_FIRST_DISPLAY_INDEX 0x1000
#define MCC_MAPS_MAXIMUM 1024

void mcc_maps_rescan(void);
short mcc_maps_count(boolean multiplayer_only);
short mcc_maps_index(short row, boolean multiplayer_only);
/* Exact scenario-type lists; display indices still use the full catalog. */
short mcc_maps_type_count(boolean campaign);
short mcc_maps_type_index(short row, boolean campaign);
short mcc_maps_find(char const *level_name);
char const *mcc_maps_level_name(short index);
wchar_t const *mcc_maps_name(short index);
wchar_t const *mcc_maps_description(short index);
/* the Steam Workshop item a map is from (decimal digits); "" for none */
char const *mcc_maps_item_id(short index);
boolean mcc_maps_campaign(short index);
boolean mcc_maps_level_campaign(char const *level_name);
/* 0 standalone/disabled, 1 next mission, 2 completed, -1 unavailable next
 * mission. Only the explicit manifest supplies an order. */
int mcc_maps_next_mission(char const *level_name, char next[64]);
/* The title of the Steam Workshop item with this id when it is subscribed here and has one. */
boolean mcc_maps_workshop_title(char const *item_id, char *title, unsigned size);

/* Menu-only views. Identities, not catalog indices, survive a rescan. */
#include "../src/workshop_collections.h"
#define MCC_MAP_BROWSER_MAXIMUM (MCC_MAPS_MAXIMUM * WORKSHOP_COLLECTION_MAXIMUM_PER_ITEM)
struct mcc_map_browser
{
	/* the open folder: a package, or a Workshop collection (`collection`, its number in group); "" at the top */
	char package[64], selected[64];
	short entries[MCC_MAP_BROWSER_MAXIMUM];
	/* a folder row's collection (workshop_collections.h), or NONE for a package's */
	short groups[MCC_MAP_BROWSER_MAXIMUM];
	boolean folders[MCC_MAP_BROWSER_MAXIMUM];
	short count, chosen, group, selected_group;
	boolean campaign, collection, selected_folder;
};

void mcc_map_browser_open(struct mcc_map_browser *browser, boolean campaign);
void mcc_map_browser_select(struct mcc_map_browser *browser, short row);
boolean mcc_map_browser_enter(struct mcc_map_browser *browser, short row);
boolean mcc_map_browser_back(struct mcc_map_browser *browser);
short mcc_map_browser_index(struct mcc_map_browser const *browser, short row);
void mcc_map_browser_text(struct mcc_map_browser const *browser, short row, wchar_t text[64]);

#endif

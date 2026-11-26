/*
MODS.H

The mods' tags played over a map's (mods.c), and the files they come from
(port/linux/src/port_mods.c).
*/

#ifndef __MODS_H
#define __MODS_H
#pragma once

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* a tag file's header (Halo PC's, big-endian, as the Halo Editing Kit and
Invader write it): its group, its header's size, its group's version and
its signature, then its tag's root */
#define MOD_TAG_FILE_HEADER_SIZE 0x40
#define MOD_TAG_FILE_GROUP_OFFSET 0x24
#define MOD_TAG_FILE_HEADER_SIZE_OFFSET 0x2C
#define MOD_TAG_FILE_VERSION_OFFSET 0x38
#define MOD_TAG_FILE_SIGNATURE_OFFSET 0x3C

/* the largest tag file read whole (the largest of Halo PC's sounds is a few
MB; a bitmap's pixels are read as they are drawn) */
#define MAXIMUM_MOD_TAG_FILE_SIZE (64UL * 1024 * 1024)

/* ---------- structures */

struct sound_permutation;
struct bitmap_data;
struct vertex_buffer;

/* ---------- prototypes/PORT_MODS.C (the platform layer's: port_mods.h) */

long port_mods_scan(void);
unsigned long port_mods_scans(void);
long port_mods_played(void);
long port_mods_file_find(char const *tag_file, char const *map_name);
int port_mods_file_stamp(long file, unsigned long *size, unsigned long stamp[2]);
int port_mods_file_read(long file, unsigned long offset, void *buffer, unsigned long size);
long port_mods_file_order(long file);
char const *port_mods_file_mod(long file);
char const *port_mods_file_path(long file);
long port_mods_count(void);
long port_mods_at(long position);
char const *port_mods_folder(long index);
int port_mods_describe(long index, char *text, unsigned long size, char const *map_name);

/* ---------- prototypes/MODS.C */

/* the game's (cache_files.c, scenario_tags_load and scenario_tags_unload) */
void mods_tags_loaded(char const *map_name);
void mods_tags_unloaded(void);
/* the console's (hs.c) */
void mods_reload(void);
void mods_enable(boolean enabled);
void mods_list(void);
/* the Mods menu's (menu_functions.c): the mods played again as it left them */
void mods_apply(void);
/* for each group's: whether its update is the console's, to say what it did
there */
boolean mods_say(void);

/* for each group's: a tag file's bytes */
unsigned long mods_read_be16(byte const *bytes);
unsigned long mods_read_be32(byte const *bytes);
short mods_read_short(byte const *bytes);
real mods_read_real(byte const *bytes);
boolean mods_in_file(unsigned long offset, unsigned long size, unsigned long file_size);
boolean mods_finite_real(real value);
/* whether the first bytes of a tag file are a header of the group's, of
its version */
boolean mods_tag_file_header_valid(byte const *file, unsigned long group_tag, short version);
/* the file of the mods played that is the tag `tag_index`'s with its
group's `extension` (".sound"), or NONE */
long mods_tag_file_find(long tag_index, char const *extension);
/* the whole file, in a buffer of the game's allocator (free() it); NULL
with *reason saying why not */
byte *mods_tag_file_read(long file, unsigned long *size, char const **reason);
/* what the mods' tags keep in memory, of all their groups: FALSE when
`size` more would be too much */
boolean mods_memory_take(unsigned long size);
void mods_memory_give(unsigned long size);
unsigned long mods_memory_left(void);
/* the map loaded's name ("a10"), and whether its tags are loaded */
char const *mods_map_name(void);
boolean mods_map_loaded(void);

/* ---------- prototypes/MOD_SOUNDS.C */

/* every one of a group's: the map's tags played from the mods' files (or,
remove_all, none); as the map loads (at_load) nothing plays yet, and
nothing is said at the console. How many of its tags the mods play */
long mod_sounds_update(boolean remove_all, boolean at_load);
void mod_sounds_tags_unloaded(void);
/* (sound_cache_start_loading_sound) whether a permutation is a mod's,
read at once into buffer */
boolean mod_sounds_read(struct sound_permutation const *permutation, void *buffer);

/* ---------- prototypes/MOD_BITMAPS.C */

long mod_bitmaps_update(boolean remove_all, boolean at_load);
void mod_bitmaps_tags_unloaded(void);
/* (texture_cache_start_loading_bitmap) whether a bitmap is a mod's, its
pixels read at once into buffer and made the hardware's */
boolean mod_bitmaps_read(struct bitmap_data *bitmap, void *buffer);
/* whether a bitmap is a mod's (whose linear rows are padded as they load) */
boolean mod_bitmaps_contain(struct bitmap_data const *bitmap);

/* ---------- prototypes/MOD_STRINGS.C */

long mod_strings_update(boolean remove_all, boolean at_load);
void mod_strings_tags_unloaded(void);

/* ---------- prototypes/MOD_MODELS.C */

long mod_models_update(boolean remove_all, boolean at_load);
/* (before the tags go: the map's models are given back, unless give_back
is FALSE (the tags are gone already), and the mods' buffers let go) */
void mod_models_tags_unloaded(boolean give_back);
/* (custom_edition_part_palette) the nodes a mod's part of a model of many
nodes is drawn with, by its vertex buffer: their count, 0 for any other */
short mod_models_part_palette(struct vertex_buffer const *vertex_buffer, byte const **nodes);

/* ---------- prototypes/MOD_SHADERS.C */

long mod_shaders_update(boolean remove_all, boolean at_load);
/* (before the tags go: the map's model and environment shaders are given
back, unless give_back is FALSE (the tags are gone already)) */
void mod_shaders_tags_unloaded(boolean give_back);

#endif

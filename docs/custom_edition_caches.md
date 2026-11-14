# Halo Custom Edition maps

The native builds (Linux, Windows, Android) load and run Halo Custom Edition
caches (`.map`, cache version 609), with the Custom Edition resource maps
`bitmaps.map`, `sounds.map` and `loc.map`. A cache that needs OpenSauce
(its OpenSauce header asks for memory upgrades, mod data files or the like)
is refused, and `.yelo` files are never looked for; a `.map` that only
carries OpenSauce's header and its `project_yellow` tags (SPV3's
backwards-compatible releases) is run as stock Custom Edition runs it,
neither read. The loader and the conversions come from DamnationCE
(xshxdex98/DamnationCE), whose documentation this file grew from; in this
build a Custom Edition map's tags are also checked by the tag validator, as
the Xbox maps' are, before anything reads them
(`port/linux/game/tag_validate.c`, [Checks](#checks)).

## Running a map

Custom Edition maps run when the `game.custom_edition` setting is on, as it
is by default (`custom_edition = true` under `[game]` in `config.toml`, or
`HALO_CUSTOM_EDITION`). Put the maps and Custom Edition's `bitmaps.map`,
`sounds.map` and `loc.map` in the data root's `custom_maps` folder, beside
its `maps` folder, which holds the game's own maps only; or set
`paths.custom_edition` to a Halo Custom Edition install, whose `maps` folder
(the Xbox drive `h:\`) is looked in after `custom_maps`.

A Custom Edition map's level name is `custom_maps\<name>`
(`custom_edition_cache.h`, `CUSTOM_EDITION_LEVEL_NAME_PREFIX`), which none
of the game's own levels has: only such a name is loaded from the Custom
Edition folders, and never from `maps`. So a map named as one of the game's
own levels (CMT's `a30.map`, Custom Edition's `bloodgulch.map`) is played as
itself, apart from the level of its name, in the menus, the loader and
network games, and is never taken for one of the campaign's levels (saved
games, progress, the next level). The game engine keeps 63 characters of a
level name, so a map whose file name is longer than 51 is not listed.

Maps keep their names. A network game's level name is the map's own
(`custom_maps\hugeass`), and its game record's map version is the first 32
bits of the map's hash (`port/linux/game/map_share_files.c`), so a client
plays its file of that name and version, whatever else it has of the name
(`custom_edition_map_find_version`); a player without it downloads it
(`port/linux/NETCODE.md`, "Map sharing"). A download beside another version
of its name is named `hugeass_0e2b3ee4.map`, the first 8 hexadecimal digits
of its hash after `_`, and the menus show it as "Hugeass (2)". A level name
without the hash (a saved last map, a test's) finds the one map of that
name in `custom_maps`, and one with it (an earlier build's, which named
every map so) the map given its name back. Maps in a Custom Edition
install's folder are found and shared as `custom_maps`' are.

A map needs only the resource maps that hold its tags: its tag index says
which (`cache_file_resource_maps_needed`, `cache_file_formats.c`: a tag
whose entry's word at 0x18 is set is held whole by `bitmaps.map` for a
bitmap, `sounds.map` for a sound, `loc.map` for a font, string list or HUD
message text). A map without such tags of a kind plays without that
resource map; one that lacks a resource map it needs is refused, saying
which.

### In the menus

The map lists (New Game's, and the Map screen of Create Game's INTERNET and
LAN and of split screen) have four kinds in their first row's chooser:
SINGLEPLAYER (the campaign's levels), MULTIPLAYER (the Xbox's maps), and
CUSTOM SINGLEPLAYER and CUSTOM MULTIPLAYER, the Custom Edition maps of the
custom maps folders by the scenario type their files give (solo or
multiplayer). A map is played as its kind says
(`port/linux/game/menu_functions.c`):

- **New Game**: a CUSTOM SINGLEPLAYER map is played as the campaign's
  levels are, at the difficulty chosen next (alone, or with split screen
  co-op's two players); a CUSTOM MULTIPLAYER map is walked around alone, as
  MULTIPLAYER's are.
- **The Map screen**: a CUSTOM SINGLEPLAYER map lists the difficulties and
  is hosted as a network co-op game (Server Setup, then the lobby), as the
  campaign's levels are; a CUSTOM MULTIPLAYER map goes on to the gametypes,
  as MULTIPLAYER's do. Split screen hosts only the multiplayer kinds.

The Xbox menus' multiplayer level list offers the Custom Edition multiplayer
maps after the thirteen Xbox levels. The pregame lobby shows a Custom
Edition map by its name and picture, multiplayer or campaign, and the game
lists by its name (`port/linux/game/custom_edition_maps.c`). The folders are
looked in once, and again each time a map list opens, which then shows maps
added since. A map is listed under its file's name, `the_bay_of_pigs.map` as
"The Bay Of Pigs". Two optional files beside the map give it what the Xbox
levels have:

- `<name>.bmp`, its picture: an uncompressed 24-bit or 32-bit Windows
  bitmap, up to 8192 pixels a side; the middle of it is shown with the
  shape of the level pictures, 140 by 114 (`port/linux/game/bmp_files.c`,
  which reads it as untrusted input).
- `<name>.txt`, its description: plain text, its lines shown as written.
  Without one, the map is described as "Halo Custom Edition map".

A machine joining a network game on a map it has not (a Custom Edition map
not in its `custom_maps`, or one of the game's own not in `maps`) leaves it
with an error that names the map and the folder to copy it into, rather
than the damaged disc error precaching a missing map gives
(`cache_files_map_present`, called as the host's settings arrive): also when
the map cannot run there (Custom Edition maps turned off, the file not a
Custom Edition cache, or `bitmaps.map`, `sounds.map` and `loc.map` missing),
when its copy of a Custom Edition map is another version than the host's
(the host sends its map's hash's first 32 bits as the game record's map
version, which the Xbox left 0: `cache_files_map_version`; a host that sends
0 is not checked), and when the host names a Custom Edition map as other
versions of this port do (`levels\test\<name>\<name>`) and the file is in
`custom_maps`. The
error's text is wrapped to its dialog, in the menus' smaller font when it
would not fit; `debug.txt` has the details.

A Custom Edition campaign map has no next level: winning it ends the game
as the campaign's last level does (alone) or plays it again (network
co-op), and it is not the campaign's saved game. A campaign whose scripts
name the next level with `map_name` (`(map_name "c20")`) moves on to it:
the level ends as a won one does, and the next is that Custom Edition
campaign map (`custom_maps\c20`), alone or for every machine of a network
co-op game (a dedicated server plays its rotation's next entry, as after
any won level). A map's script may name no other map, and one that names no
campaign map present does nothing (`debug.txt` says so); the console's
`map_name` loads any map, as before (`main_script_map_name`, main.c).
Global initializers may not change maps, including through a static script:
the map is not ready yet. Such a call is logged and does nothing.

### From the console

`map_name` takes the level name: `custom_maps\<name>` for a Custom Edition
map (`custom_maps\hugeass` plays `custom_maps\hugeass.map`). A multiplayer
scenario needs a game variant, or no starting location qualifies and no
player spawns:

```
game_variant slayer
map_name custom_maps\hugeass
```

## What the native builds do

- **With `game.custom_edition` on**, the platform reserves the address
  window Custom Edition tag data is linked to, `0x40440000`-`0x41B40000`
  (`port/linux/src/xbox_memory.c`, at start-up; 23 MB of address space,
  backed as it is touched), and a Custom Edition map is loaded into it,
  converted as below, checked, and run. It is read in place, never copied
  to the cache partition: every read the game makes of it (structure BSPs,
  bitmap pixels, sound samples) is served from the map, `bitmaps.map`,
  `sounds.map` or the sounds decoded at load, according to where its offset
  falls in their combined offset space (`custom_edition_cache_read`). A map
  that fails any check is logged in `debug.txt` and not loaded: the game
  goes back to its menus.
- **With it off**, a Custom Edition cache is named and refused, instead of
  being rejected as "an old version" of this build's caches; so is one put
  in `maps`, which is told to go in `custom_maps`.
- **Xbox caches** take the path they took before.

The desktop builds' Xbox memory window is 512 MB (Android's 128 MB), in
which the texture cache is 256 MB: Custom Edition maps draw more texture
than the Xbox's 22 MB cache holds, and their geometry is made into
Direct3D buffers in the same memory (`port/linux/include/halo_port_capacity.h`).

## Loading and conversion

### The loader (`cache_file_formats.c`)

`port/linux/game/cache_file_formats.c` (`cache_file_formats.h`) holds the
format code: standalone C with fixed-width types and no game dependencies,
compiled into the native builds (as every `port/linux/game` unit is) and
into the report tool. Map files are untrusted input: every field is read
little-endian from a bounds-checked buffer, never through a structure laid
over the file, and every offset, count and size is checked before use, with
overflow-safe arithmetic. `port/tools/cache_file_report.c` reports what can
be done with each file:

```sh
clang --target=i686-pc-windows-msvc -fuse-ld=lld -Iport/linux/game \
    port/linux/game/cache_file_formats.c port/tools/cache_file_report.c -o cache_file_report.exe
cache_file_report [--maps DIRECTORY] [--dump-tags FILE] FILE...
```

Resource maps are looked for next to the cache (or in `--maps`):
`bitmaps.map`, `sounds.map` and `loc.map`. `--dump-tags` writes the converted
tags to a file, as they would sit at `0x40440000`.

Loading, step by step (`custom_edition_cache_load`):

1. The header: signatures, version 609, terminated strings, file length (0,
   as Invader leaves it, for the whole file) within the file and the size
   limit (`0x30000000`: Halo PC's was `0x18000000`, and Invader builds
   larger maps, which Chimera runs), no compression, tag data within the
   file and the tag cache (23 MB). A cache with OpenSauce's header (its
   `yelo` signature at offset `0x70`, which Custom Edition leaves as
   padding) that sets any of its flags (memory upgrades, mod data files and
   the rest) is refused; one that sets none is a Custom Edition cache.
2. The tag data is read to the start of the tag cache, which stands for
   `0x40440000`.
3. The tag index (`tags` signature), every tag instance (its handle must
   match its position, its name must lie in the tag data and be terminated,
   its address must lie in the tag data; only structure BSPs may have none;
   only bitmaps, sounds, fonts, unicode string lists and HUD message text may
   be held by resource maps; OpenSauce's `project_yellow` and
   `project_yellow_globals`, which nothing reads, are left as they are), the
   model data range, and the scenario tag.
4. Every structure BSP: its tag, its range in the file, its place in the tag
   cache (above the tag data, within the tag cache), its header (`sbsp`
   signature, no Xbox vertex buffers, its structure pointer inside it), and
   its lightmaps and their materials: every block within the BSP and aligned,
   every material's uncompressed environment vertices (56 bytes) and
   lightmap vertices (20 bytes) within the BSP and of the size their counts
   give, at most 64000 of each (the game's limit, to which the validator
   cuts a material's counts), lightmap vertices exactly when the lightmap
   has a bitmap, and every normal, binormal, tangent and incident radiosity
   within ±1.005 (the game's vertex compressor asserts on anything further
   out).
5. The header checksum is computed as OpenSauce computes it, before anything
   changes the tag data; a mismatch is reported, not refused.
6. The tags held by resource maps are read into the tag cache after the tag
   data, below the lowest structure BSP, and their pointers relocated. A
   sound's header in the map lacks its sample rate, encoding, compression
   and longest permutation length; they are taken from the header's copy in
   `sounds.map` ([Evidence](#observed-not-stated-by-the-sources-established-on-the-sample-maps)).
7. Every bitmap, sound, font, unicode string list, HUD message text, model
   (`mod2`) and transparent chicago shader (`schi`, `scex`) is walked through
   its blocks: every block and data pointer must lie within the loaded tags,
   every bitmap's pixels and sound's samples within their file, and every
   model part's strip and vertices within the model data, of the kinds Custom
   Edition writes (a precompiled strip, uncompressed 68-byte vertices), with
   at most 65535 vertices and at most 22 local nodes.

Then `custom_edition_cache_convert` changes what only needs its bytes
changed:

- **Shader types.** Custom Edition inserted *transparent chicago extended*
  as shader type 7, so its water, glass, meter and plasma shaders are types
  8 to 11 where this build has 7 to 10 (and would draw a Custom Edition glass
  shader as meter, and so on, unnoticed). Every shader gets this build's
  number for its group; a type field saying otherwise (as map protection
  leaves them) is counted and given the group's.
- **Transparent chicago extended shaders** (`scex`) become transparent
  chicago shaders (`schi`): the two layouts agree up to the maps, of which
  `scex` has two sets, for four and for two texture stages; the four-stage
  maps are kept (this build draws four stages), or the two-stage ones when
  there are no others, and the extra flags move to where `schi` has them.
- **Bitmaps and sound permutations** get the state of ones not yet drawn or
  played, and name their own tags: `bitmaps.map` and `sounds.map` hold the
  tag handles of whatever map they were built with.
- **Sounds this build cannot decode** (Custom Edition's Ogg Vorbis) are made
  unplayable, by emptying their pitch ranges, which the game skips.
- **Animation overlays naming animations that do not exist** are made to
  name none, which the game skips. Maps built with the editing kit can have
  them; Custom Edition reads past the graph's animations there, and this
  build's debug builds stop on it.
- **HUD elements drawn from double-resolution bitmaps.** Halo PC added a
  third scaling flag to HUD placements, *use high resolution scale*
  (OpenSauce `hud_definitions.hpp`), and draws a flagged element at half the
  size of its bitmap. Custom Edition's HUD bitmaps are made for that: in
  `bloodgulch.map` every flagged element draws a bitmap exactly twice the
  size of the one its Xbox counterpart draws (256×64 for 128×32, 512×512 for
  256×256), with the same scale, and every unflagged one a bitmap of the same
  size ([Evidence](#observed-not-stated-by-the-sources-established-on-the-sample-maps)).
  This build ignores the flag, so it drew them twice too large; the
  placements of the unit, weapon and grenade HUD interfaces and the HUD
  globals' messages that have it get half their scale, and lose the flag (32
  in `bloodgulch.map`). Halo PC reads the flag only on statics, meters and
  numbers (Chimera's `hud_bitmap_scale.cpp`): crosshair and overlay items
  keep their scale. A number's scale is not its
  digits', so a flagged number keeps the flag and `hud_draw_numbers` draws
  its digits at half their size, spaced as the digits tag says. A bitmap may ask the
  same of every element that draws it, with Halo PC's bitmap flags *half hud
  scale* and *force hud use highres scale* (Invader's `bitmap.json`): an
  element drawing such a bitmap (a static or meter, its bitmap after its
  placement; a crosshair's or overlay's items, their crosshair's or
  overlay's) gets half its scale too. None of the 18 Custom Edition maps on
  hand, nor `bitmaps.map`, sets either.
- **Maps made around Halo PC's own behaviour.** Chimera fixes Halo PC to draw
  as the Xbox does, and keeps a list of the maps made around Halo PC's way
  instead, by map name (in lower case) and tag data checksum, with the
  behaviours each relies on (`map_hacks_config.json`, by SnowyMouse; here
  `port/linux/game/custom_edition_behaviours.inc`, generated from it). This
  build draws as the Xbox does, so for a listed map it follows these where it
  can, and logs each: HUD multitexture overlays' blend functions in Halo PC's
  order (`gearbox_multitexture_blend_modes`: Halo PC picks its shader by the
  Xbox's value from shaders in alphabetical order), overlays not drawn
  (`block_multitexture_overlays`), the HUD digits' metrics halved and every
  number's digits drawn at half size (`hud_number_scale`), bitmaps' HUD scale
  flags cleared (`disable_bitmap_hud_scale_flags`), and model shaders'
  detail after reflection flag flipped (`invert_detail_after_reflection`).
  Not yet: Halo PC's fixed-function meters (`gearbox_meters`), its
  transparent chicago multiply, bump attenuation and environment shader
  types, the old widescreen HUD and embedded Lua.
- **The score hint.** String 100 of `ui\multiplayer_game_text` is Halo PC's
  `Hold "%s" for score`, which Halo PC fills in with its score key; this
  build copies it as it is (`game_engine.c`, the press-back-for-score
  message), so `"%s"` becomes `BACK`, the button this build reads for the
  score, in the same four characters.

### In the game (`custom_edition_cache.c`, `custom_edition_geometry.c`, `custom_edition_bitmaps.c`)

- **Models.** Every `mod2` part is given this build's part layout (the first
  0x68 bytes of the 0x84-byte Custom Edition part are the same fields; the
  parts are repacked in place), its vertices are compressed from the map's
  model data by the game's own `rasterizer_geometry_compress_vertices`, its
  node indices made the model's when its model's parts have local nodes, and
  its strip copied unchanged; the game's own `rasterizer_vertex_buffer_new`
  and `rasterizer_triangle_buffer_new` make its buffers, and the tags become
  `mode` tags. Before any of that, every index a part holds is checked
  against what it names (shaders, nodes, parts, vertices). This build's
  renderer skins at most 43 nodes at once, so a model of more is drawn a
  part's own nodes at a time: a model of more whose parts have no local
  nodes is given them, each part the nodes its vertices name, and is refused
  only when a part's vertices name more than a part holds.
- **Structure BSPs.** When the game loads a BSP, every material's vertices
  are compressed the same way (environment and lightmap vertices), and the
  material gets buffers and the compressed vertices the game reads for
  visibility, collision and lights (`scenario_structure_bsp_load` in
  `cache_files.c` calls `custom_edition_structure_bsp_load`).
- **Bitmaps.** Every bitmap must pass the game's own `bitmap_verify` and be
  of a kind the texture cache and the swizzling code handle (a format with a
  hardware texture, a compressed flag that agrees with the format,
  power-of-two sizes unless linear, square cube maps, all its pixels
  present). Halo PC draws a 2D bitmap of any size, so an uncompressed one
  whose sides are not powers of two is made linear as the map loads, and
  drawn from its first level (birdcage-plus's 3840 by 64 needler plasma).
  Its pixels are laid out for Halo PC, levels as the tag stores them,
  unswizzled; as they arrive, the game's own
  `rasterizer_xbox_bitmap_rebuild_hardware_format` lays them out as an Xbox
  cache would (swizzled, cube maps face by face, padded).
- **Chicago extra layers.** January's transparent chicago shader draws its
  extra layers in a loop that never advances
  (`rasterizer_xbox_transparent_geometry.c`, marked as a preserved bug). No
  Xbox map has such layers, so January never hung on it, but Custom
  Edition's editing kit can make them: the native builds advance the loop.
- **Channel orders.** Halo PC keeps two kinds of texture in other channels
  than this build reads them from (`enum custom_edition_channel_order`,
  [Evidence](#observed-not-stated-by-the-sources-established-on-the-sample-maps)):
  a model shader's *multipurpose map*, whose auxiliary (detail) mask,
  self-illumination, specular and color change masks Halo PC keeps in red,
  green, blue and alpha, where this build's model shaders read specular from
  red, self-illumination from green, color change from blue and the
  auxiliary mask from alpha; and a *HUD meter*, whose shape Halo PC keeps in
  color and fill order in alpha, where this build's meter shader
  (`rasterizer_xbox_dynavobgeom.c`) compares the color with the meter's value
  and discards what has no alpha. Reordering the pixels would mean
  decompressing them, which the texture cache has no room for, so the game
  finds the bitmaps drawn as multipurpose maps (by model shaders) and as
  meters (by unit and weapon HUD interfaces) when the map loads, and tells
  the renderer where each bitmap's pixels arrive and in which order their
  channels are; the renderer samples them with each channel taken from where
  Halo PC keeps it (`port/linux/src/xbox_textures.c`, a texture swizzle,
  which leaves them compressed). A bitmap also drawn another way (a
  multipurpose map that is also a base map, say) keeps its channels, since
  the renderer has one order for each texture, and is logged: 3 in
  `bloodgulch.map`, all multipurpose maps.
- **Reads.** The renderer write-protects the memory it has made textures of
  and learns of changes from the faults writes take; the kernel fails a read
  into such memory instead. Every read of the map is therefore made into a
  64 KB staging buffer and copied, as the platform's file layer does.
- **Texture memory.** A frame of `Elite_Alpha_Siege.map` draws more than the
  22 MB of textures the Xbox texture cache holds, and textures that did not
  fit were drawn as the default one. The desktop builds' texture cache is
  256 MB, in a 512 MB memory window (`halo_port_capacity.h`), for every map;
  Android's is the Xbox's.
- **Scripts.** A compiled script names each function it calls, and each
  engine global it uses, by its index in the engine's tables, and Halo PC's
  tables have entries this build's do not, in among the ones both have: from
  some point on, an index names another entry here
  ([Evidence](#observed-not-stated-by-the-sources-established-on-the-sample-maps)).
  `hugeass.map`'s day and night script would call `playback` instead of
  `switch_bsp`. A compiled script also keeps every name, in its string data,
  so every call and every engine global is found again by name with the
  game's own `hs_find_function_by_name` and `hs_find_global_by_name`
  (`custom_edition_scripts.c`); one this build does not have does nothing
  and is logged by name (Limits, below). The value types are numbered
  alike in both builds.
- **Multiplayer vehicle placement.** This build places a multiplayer game's
  vehicles by type: only the first three of the globals' multiplayer
  vehicles are created, and of those the ones the variant's vehicle set
  names (`game_engine_remap_vehicle`; the built-in variants name the first).
  Custom Edition maps are made for retail Halo's rules: a vehicle placement's
  multiplayer spawn flags name the game types it is placed in by default,
  and a script may create any vehicle. `hugeass.map` depends on that (above).
  For a Custom Edition map in slayer, capture the flag, king of the hill or
  oddball, the native builds place the vehicles whose spawn flags name the
  game type by default and let any vehicle be created
  (`custom_edition_objects.c`, called from `object_types_place_all` and
  `game_engine_remap_vehicle`); a variant without
  vehicles still has none, and race, which has no spawn flag, keeps this
  build's rule.
- **Multiplayer vehicles.** `game_engine_predict_resources` takes the three
  multiplayer vehicles Xbox globals always have; a Custom Edition map's
  globals can list fewer, and oddball stopped on such a map. With fewer than
  three, the native builds predict none (`game_engine.c`).

## Checks

The loader (`cache_file_formats.c`) checks the container: the header, the
tag index, every structure BSP, every resource-held tag and its relocated
pointers, every bitmap pixel and sound sample range, every model part's
geometry in the model data. Then, once the loader and the conversion of
bytes have made the tags this build's layout, the tag validator checks
every tag as it checks an Xbox map's (`tag_validate_custom_edition_tags`,
called by `custom_edition_cache_tags_load`): every block and data within
the loaded tags and overlapping no other's, counts cut to the game's
capacities, indices, enums and tag references corrected, runtime values
reset, then the graph checks (collision BSPs, node trees, scripts' syntax),
and a tag name that lies in any tag's block or data is emptied (what the
game writes there could take away its end). A map whose pointers cannot be
trusted is refused; what can be corrected is, and logged. What differs
from an Xbox map:

- the tag header has no vertex or index buffers, and the tag cache is the
  Custom Edition one (23 MB);
- bitmap pixels and sound samples are in several files: their offsets must
  lie in the map, the sounds decoded at load, `bitmaps.map` or `sounds.map`;
- models are gbxmodels (`mod2`), whose parts keep their geometry in the
  map's model data and a table of local nodes, which are checked against
  the model's (`tag_schema_custom_edition_groups`, in `tag_schema_models.c`);
  the game takes them as models;
- a structure BSP's material vertices are uncompressed, and are checked
  against its uncompressed vertex data.

Only after the validator has passed the tags are the scripts given this
build's function and global indices, the bitmaps checked for the texture
cache, and the models and each structure BSP (checked as it loads, as an
Xbox map's) given compressed vertices and buffers. The script function
allowlist (`hs.c`) applies to Custom Edition maps as to every map.

`build/linux/map_validate` runs the loader, the conversion of bytes and the
validator on maps outside the game, with the resource maps beside each map
or in `--maps`; `--fuzz` changes words of a map's file at random.

## Tested

### Automated tests

- `python -m pytest tools/test_cache_file_formats.py` (130 tests; needs
  clang): synthetic Custom Edition caches and resource maps, caches that
  need OpenSauce refused and those that only carry its data loaded, every
  check of the loader and the conversion, and seeded random corruptions,
  through `port/tools/cache_file_report.c`. With `HALO_CUSTOM_EDITION_MAPS`
  naming a folder of maps, four more check real maps: against DamnationCE's
  recorded results (which are of its copies of the stock maps), and that any
  `.yelo` map among them is either refused or a Custom Edition cache.
- `python -m pytest tools/test_bmp_files.py` (41 tests): the map pictures'
  reader.
- `tools/test_linux_port.py`: with `HALO_CUSTOM_EDITION_MAPS` (or the
  gitignored `assets/custom_edition`), every map is loaded and checked and
  none refused, and three are fuzzed (100 runs each); the retail Xbox maps
  still need no correction.

### Maps

Two folders were checked with `map_validate`: the 20 stock Custom Edition
maps (the halopc-restored r220 copies) and 19 community maps (`area53`,
`bigass`, `deltaruins`, `Elite_Alpha_Siege`, `extinction`, `Fission_Point`,
`Floodgulch5.1`, `hugeass`, `immure`, `tactics`, `the_bay_of_pigs`,
`the_rise_of_asis`, `WW_battlefield` and copies of stock levels). None is
refused. The stock maps need one correction each, a character of
`loc.map`'s gamespy font 5 pixels narrower than nothing; the community maps
some more, their own data's mistakes (an enum past its values, a region
index past its block, a particle state of a sequence with no sprites).
Fuzzing, 200 runs of each of those 39 maps (7,800 runs): no crash, no hang,
and every map let through clean when checked again.

Run in the game (Linux release build, 40 to 50 seconds each, slayer):
`hugeass`, `bigass`, `extinction`, `deltaruins`, `Elite_Alpha_Siege`,
`the_rise_of_asis`, `area53`, `tactics`, `Floodgulch5.1`, `immure`,
`WW_battlefield`, `the_bay_of_pigs` and `Fission_Point` load, are checked,
converted and played to the end of the run without a crash; `hugeass`'s
hangar, HUD and first-person weapon were seen drawn.

Two campaign maps were checked with `map_validate`: CMT's `a30.map` (2006,
"Halo"; 13 corrections) and CMT SPV3's backwards-compatible `a50.map` (2012,
The Truth and Reconciliation, with OpenSauce's header setting no flags and
its two `project_yellow` tags; 43 corrections); `a30` was also run. From the
menus (on a virtual display): New Game's CUSTOM SINGLEPLAYER `a30` played as
a campaign level at the difficulty chosen; the Map screen's (LAN) listed its
difficulties, set up co-op in Server Setup and showed it in the lobby as
co-op. With `debug.network_test`, two machines on loopback played
`custom_maps\a30` as network co-op and `custom_maps\hugeass` as slayer; a
joining machine with a maps folder missing the host's Xbox map was shown the
error and left the game. A joining machine without `hugeass` (or without
`sounds.map` and `loc.map` too) downloaded them and played (map sharing,
`port/linux/NETCODE.md`): in the lobby, into a game in progress, from the
host and another player at once with a second machine downloading beside it
(on loopback, and through internet play's tunnels with a local broker), each
file the same as the host's; with `network.map_download` off, or a host with
`network.map_share` off, it was shown the error and left.

### Not tested

- **Playing**: nobody played the maps through.
- **Android** builds, and was not run.

## Evidence

### Sources

- **OpenSauce** (GPL-3.0, Kornner Studios): the `OpenSauce-master.zip`
  archive of the upstream repository (SHA-256
  `f1097c9cf5975a208506c78c3af47d6b893f998a9f47dddf89c9bbcc1f4b7c73`; newest
  entry dated 2016-01-15; `yelo_version.hpp` names OpenSauce 4.0.0).
- **Reclaimer** (GPL-3.0, Gravemind2401): the `Reclaimer-master.zip` archive
  (SHA-256 `72fd5270abbdf662ff07a54e9cce80a6158e479c51ecfdcd979f7f9fbd6c2d03`;
  newest entry dated 2026-09-20).
- **The Xbox maps of retail build 01.10.12.2276** (`bloodgulch.map`,
  decompressed): not the 01.01.14.2342 build this repository reconstructs,
  but their model parts have its 0x68-byte layout and their shader types its
  numbering, which makes them a check on what the conversion produces.

No code from OpenSauce or Reclaimer (both GPL-3.0) was copied: they were
read as documentation of the file formats, and every layout used was then
checked against the sample maps.

### Documented facts (source, checked against the sample maps)

| Fact | Source |
| --- | --- |
| Cache header layout, version 609, `head`/`foot` signatures | OpenSauce `blamlib/Halo1/cache/cache_files_structures.hpp` (`s_cache_header`) |
| Tag index (0x28 bytes, instances at 0x28, model data as file offsets) and tag instance (0x20 bytes, resource-map flag at 0x18) | same file (`s_cache_tag_header`, `s_cache_tag_instance`) |
| OpenSauce's header at 0x70 begins with the `yelo` signature, where Custom Edition's header has padding, and has its flags (memory upgrades, mod data files, protected, game state upgrades, compression parameters) at 0x76 | OpenSauce `YeloLib/Halo1/cache/cache_files_structures_yelo.hpp` (`s_cache_header_yelo`) |
| Tag cache at `0x40440000`, 23 MB; file size limits | OpenSauce `cache_constants.hpp`, `saved_game_constants.hpp` |
| Resource map header and entries | OpenSauce `data_file_structures.hpp` |
| Groups held by resource maps: bitmaps, sounds, fonts, unicode string lists, HUD message text | OpenSauce `blamlib/Halo1/cache/cache_files.cpp` (`cache_file_data_load`) |
| Structure BSP reference (0x20), header (0x18), structure, lightmap and material layouts | OpenSauce `scenario_definitions.hpp`, `structure_bsp_definitions.hpp`; this build's `structure_bsp_definitions.h` agrees |
| Bitmap, sequence, sprite, bitmap data layouts; pixels in `bitmaps.map` flag | OpenSauce `bitmaps/bitmap_group.hpp` |
| Sound definition, pitch range, permutation layouts; samples-in-`sounds.map` flag | OpenSauce `sound/sound_definitions.hpp` |
| Gbxmodel layout: the part (0x84 bytes) is this build's 0x68-byte part followed by a local node count and a 22-entry node table | OpenSauce `models/model_definitions.hpp` (`gbxmodel_geometry_part`) |
| Where a part's strip and vertices lie in the model data (triangle count, strip offset, vertex count and offset in the buffer fields; 68-byte vertices) | Reclaimer `Blam/Halo1/GbxmodelTag.cs` (`ReadPCMeshes`) |
| Shader types: *transparent chicago extended* inserted at 7, before water, glass, meter and plasma; its two map sets | OpenSauce `shaders/shader_definitions.hpp` |
| Script syntax nodes: 19001 in the stock engine | OpenSauce `hs_constants.hpp` |
| HUD message text layout | OpenSauce `interface/hud_messaging_definitions.hpp` |
| Font (156 bytes) and unicode string list layouts | BlamLib `Blam/Halo1/Tags/Definitions/Misc.cs`, `Resources.cs` |
| Bitmap tags in `bitmaps.map` by index, pixels by absolute offset | Reclaimer `Blam/Halo1/CacheFile.cs`, `BitmapTag.cs`, `BitmapsAddressTranslator.cs` |
| The header checksum: CRC-32 of the structure BSPs (packed from 0x800), the model data and the tag data, not inverted | OpenSauce `cache_files_yelo.cpp` (`CalculateChecksumFromMemoryMap`), `memory_interface_base.cpp` (`CRC`); reproduces the stored checksum of 23 of the 24 sample caches |

### Observed (not stated by the sources; established on the sample maps)

| Fact | Evidence |
| --- | --- |
| Structure BSPs load at the top of the tag cache (`address + size` = `0x41B40000`) | all 24 caches (those with OpenSauce's memory upgrades at the top of its larger tag cache) |
| Bitmaps, fonts, unicode string lists and HUD text held by resource maps: the tag's address field is the entry index, the entry's name is the tag's path, the entry is the whole tag with addresses counting from its start | 9,550 bitmaps, 68 fonts, 2,248 string lists, 23 HUD texts across the sample |
| Sounds held by `sounds.map`: the map keeps the 0xA4-byte header, whose pitch range block has a count and no address; the entry named by the tag path holds the header again, then the pitch ranges and permutations, whose addresses count from the first pitch range | 7,397 sounds, 21,870 sample ranges |
| The map's copy of a resource-held sound's header has its compression and longest permutation length zero, and its encoding and sample rate zero too; the entry's copy has them as the Xbox has them | in `bloodgulch.map` and `beavercreek_halo3.yelo`, the two copies of all 527 resource-held sounds differ only in those four fields, in pointers, and in the map's own promotion sound reference; for the 315 sounds the Xbox `bloodgulch.map` shares, the entry's sample rate, encoding and the runtime fields after the longest permutation length equal the Xbox's; its compression does except for the 38 that Custom Edition has as Ogg Vorbis, and its longest permutation length for 273 (the others differ by a few milliseconds or more, among them the Ogg Vorbis ones); its pitch ranges' runtime fields all do |
| Sound permutations and resource-held bitmaps name tags of another map (the one the resource map was built with) | 632 of 676 bitmaps of `bloodgulch.map`; every resource-held permutation differs from the Xbox's own sound handle |
| Model parts: the strip and vertex data are in the model data, never in the part's blocks; the triangle buffer is a precompiled strip and the vertices uncompressed; the part flag 2 is set on exactly the parts that have a node table | all 24 maps (10,934 parts) |
| Converting a part's vertices with the game's compressor reproduces the Xbox's | for the 335 parts of the 60 models the Custom Edition and the Xbox `bloodgulch.map` share: identical strips; positions equal to within float rounding; texture coordinates and node weights exactly what the compressor makes of the Custom Edition values (clamped to ±1, as the Xbox also has them: `plant_broadleaf_short` has coordinates up to 3.0 in both); node indices the Custom Edition ones times 3, except that the Xbox writes an unweighted second node as -1 where the compressor writes the node Custom Edition names; normals within 0.002 |
| Every node index of a vertex names a node of its part's table (in models whose parts have local nodes) or of its model, every table entry a node of the model, and every vector is within ±1.0001 | all 24 maps |
| Structure BSP materials: environment vertices uncompressed (type 0), with lightmap vertices (as many as the environment ones) exactly in lightmaps that have a bitmap, and no compressed vertices | all 24 maps (the lightmap vertex type field is 0 or 2 regardless) |
| Custom Edition shader type values are 7 to 11 for `scex`, `swat`, `sgla`, `smet`, `spla`; the Xbox's are 8, 9, 10 for `sgla`, `smet`, `spla` | every shader of the 24 maps, and of the Xbox `bloodgulch.map` |
| Chicago shaders with extra layers: none in the stock maps or the Xbox `bloodgulch.map`; 2 in `beavercreek_halo3.yelo`, 2 in `celer_exile_odst_v2.yelo`, 4 in `extinctionrevanepic2.map`, 1 in `fy_killzone.yelo` | every `schi` and `scex` of the sample |
| Compressed color plates are left out of caches: size kept, address 0 | every bitmap in `bitmaps.map` |
| Font style references in `loc.map` are all `NONE` | all 3 fonts |
| Sound compression value 3 is Ogg Vorbis, value 1 is Xbox ADPCM | in `sounds.map`, all 106 value-3 permutations start with `OggS`; all 1,365 value-1 permutations are whole 36-byte blocks |
| `bitmaps.map` bitmaps are not swizzled, and cube maps hold each level's six faces together | all 1,467; 38 of 40 cube maps match that order when checked against their next level (one is uniform, one does not match) |
| Multipurpose maps: Custom Edition's red, green, blue and alpha hold what the Xbox's alpha, green, red and blue do | the multipurpose maps the Custom Edition and the Xbox `bloodgulch.map` share, decoded and compared channel by channel (the cyborg's, the warthog's, the boulders') |
| HUD meters: Custom Edition's alpha holds the Xbox's color (the fill order), and its color the Xbox's alpha (the shape) | `hud_ammo_meters`: Custom Edition's 512×512 A8R8G8B8 averaged down to the Xbox's 256×256 A8Y8 differs from the Xbox's luminance by 6.6 on average in alpha and 23 in color, and from its alpha by 12.3 in color and 23.4 in alpha; `hud_unit_meters` has its sprites rearranged, and shows the same swap when viewed. January's meter shader reads the fill order from color and discards texels without alpha |
| HUD elements flagged *use high resolution scale* draw bitmaps twice the Xbox's size; others the same size | every unit and weapon HUD placement of `bloodgulch.map` whose tag the Xbox map also has (the motion sensor's foreground, unflagged, uses a 128×128 bitmap in both) |
| Vehicle placements: the byte at 0x58 is the multiplayer team and the word at 0x5A the multiplayer spawn flags, default bits 0 to 3 (slayer, capture the flag, king of the hill, oddball) and allowed bits 8 to 11 | `hugeass.map`'s 78 vehicle placements: the byte is 1 on exactly the blue team's; the word is 0x0303 on warthogs, tachikomas and pelicans, 0x0F0F on ghosts, 0x0404 on 16 unnamed jets, 0x0808 on the night jet and 0 on vehicles its scripts create; OpenSauce leaves these bytes unnamed |
| Script function and engine global tables: Halo PC's have entries this build's lacks, so a compiled index names another entry from some point on; value types are numbered alike | the compiled scripts of the sample, each call's function index against the name its first child keeps, and each engine global's index against its name: `timberland.map` 23 calls shifted (`player_effect_start` by 29), `ui.map` 11 (`camera_set` by 1), `hugeass.map` 3 calls and 1 global (`rider_ejection`, 158 against 147), `extinctionrevanepic2.map` 9 calls, and 4 of a function this build lacks; no other stock map has scripts; each call's value type is the type of its function here (1,485 of 1,485 in `hugeass.map`), or one the call site casts to |
| Bitmaps the Xbox keeps in monochrome formats and Custom Edition as 32-bit color at the same size keep their channels (A8Y8 as A8R8G8B8 with red the luminance and alpha the alpha) | 24 of the 26 such bitmaps of `bloodgulch.map` decode to exactly the Xbox's values; the other 2 have other content |

### Assumptions (not verified)

- **Placement of resource-held tags.** They are placed after the tag data,
  aligned to 4 bytes, as OpenSauce's unimplemented loader outlines
  (`s_cache_file_data_load_state`); how Custom Edition itself places them is
  not known, and nothing found depends on it.
- **Which chicago extended maps.** The four-stage maps are kept, as the
  renderer draws four stages; that Custom Edition's two-stage maps are a
  fallback for older hardware is an inference (BlamLib's exporter makes the
  same choice).
- **Centroid nodes of local-node parts** are taken to be the model's nodes:
  they are within the model's node count in every map, and they are only
  used to sort transparent parts.
- **Editing-kit pointers are cleared.** The definition pointers of relocated
  blocks and data are set to 0: they refer to nothing in the game process.
- **Name comparison** is exact (case-sensitive), which matched every sample
  sound.

## Limits and remaining work

- **What the validator corrects is lost.** A community map's data that the
  validator corrects (an index past its block, an enum past its values) is
  what the game then runs; Halo PC read past such data unchecked.
- **Ogg Vorbis sounds** are decoded when the map loads (stb_vorbis, with
  added bounds checks: `port/third_party/stb_vorbis`) and encoded again as
  Xbox ADPCM in memory (`port/linux/game/custom_edition_sounds.c`); one that
  cannot be decoded is silenced, and still logs when played.
- **Models of 44 nodes or more** are drawn a part's nodes at a time, given
  local nodes when they have none; one whose part's vertices name more
  nodes than a part holds is refused.
- **Linear bitmaps whose rows are not a multiple of 64 bytes**, and
  textures larger than 4096 pixels a side, are not drawn.
- **Halo PC's HUD meters' minimum alpha** (where the Xbox's meters have an
  overlays block) is dropped, and **its weapon functions "primary firing on"
  and "secondary firing on"** are the Xbox's primary and secondary firing.
- **Button prompts on replaced icon sheets** show what the map's icon bitmap
  has where the Xbox's buttons are.
- **Channel orders are per texture**: multipurpose maps and meters also drawn
  another way keep Halo PC's channels.
- **Scripts that use what this build does not have**: a call of a function
  this build lacks, or a read of an engine global it lacks, does nothing
  (a constant of its type's harmless value, logged by name:
  `custom_edition_scripts.c`); only one that gives a script's index refuses
  the map. Maps that need OpenSauce's runtime features are refused; a map
  that only carries its tags (`project_yellow`) runs without them, as on
  stock Custom Edition.
- **Halo PC behaviours** some maps were made around (Chimera's map list,
  `custom_edition_behaviours.inc`) are followed where this build can, and
  each is logged.
- **The conversions are one-way and in memory**: nothing is written to the
  map files.

### How far Custom Edition tags are from the Xbox tags

`tools/custom_edition_tag_footprints.py` compares the tags a Custom Edition
cache and the Xbox cache of the same level both keep (same group, same name),
without any tag definitions. A tag's *footprint* is the distance from its
address to the next tag's (its structure and the block data after it); its
*structure size* is estimated as the distance to the lowest address in the
footprint that the footprint points at (its first child block). Equal
footprints, level after level, suggest a group's layout is unchanged; they do
not prove it.

The Xbox caches available are the retail ones, build 01.10.12.2276, not the
01.01.14.2342 build this repository reconstructs: they stand in for it, and
differences between the two Xbox builds are not measured. Run on the 14
levels both sets have (13 multiplayer levels and `ui`):

```
python tools/custom_edition_tag_footprints.py --blocks assets/custom_edition "<Xbox 2276>/maps"
```

- **48 groups have the same footprint for every shared tag in every level**:
  `Soul actv ant! bipd colo cont deca eqip flag fog font foot grhi hmt hud#
  hudg itmc jpt! lens lifi ligh lsnd mach metr mgs2 mply part pctl phys pphy
  scen schi senv sgla sky smet snde soso spla ssce str# swat trak udlg unhi
  vcky vehi wind`. The shader groups among them have the same layout, but
  not the same type values (above).
- **Groups whose footprints differ, but whose structures match**: UI widgets
  (`DeLa`, 464 of 657 footprints equal, 423 of 423 structures), animations
  (`antr`, 213/239), collision models (`coll`, 331/345), effects (`effe`,
  4140/4166), projectiles (`proj`, 197/210), weapon HUDs (`wphi`, 196/222),
  weapons (`weap`, 39/182, structures 182/182), globals (`matg`, 1/14,
  structures 14/14) and scenarios (`scnr`, 0/14, structures 13/14), plus, in
  `ui` only, bitmaps, sounds and string lists. These differ in their block
  data. The flamethrower's tags account for every `effe`, `proj` and `wphi`
  difference and all but one `coll` difference; 11 of the 14 weapons (every
  player weapon and both vehicle guns) differ in every level.
- **What differs in those blocks** (`--blocks`: the top-level blocks of each
  differing tag, found without definitions and compared): almost always
  the number of elements, or whether a block has any, which is content:
  weapons 143 count and 26 presence differences, UI widgets 158 and 63,
  scenarios 36 and 13, projectiles 13, weapon HUDs 78 presence, collision
  models 26 count. Every weapon difference checked in Blood
  Gulch lies in a predicted-resources list (8-byte elements, weapon
  offset `0x4E4`; OpenSauce `weapon_definitions.hpp`), or, for the
  flamethrower, also in its attachments; the globals differ in their
  cheat powerups (16 against 14). The rest (flamethrower effects 26,
  globals 13, animations 39, one collision model, and 3 in `ui`) keep their element counts but
  span a different amount of data up to the next block, which this method
  cannot split into element size and nested content: for the globals, the
  block in question (`player_info`, 0xF4-byte elements without blocks of
  their own) spans more than its elements, so other data sits in the span.
  No difference found has to be a layout change, and the two maps run did
  not show one.
- **Groups only Custom Edition has**: `mod2` (models), `scex` (a transparent
  shader type), `devc` and `tagc`, in all 14 levels; **only the Xbox has**:
  `mode` (models) and `sotr` (a transparent shader type), besides the groups
  Custom Edition keeps in resource maps.

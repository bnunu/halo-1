# Mods

A mod is a folder of Halo PC tag files that the game plays over a map's own
tags on your machine, as a resource pack does: sounds (a high-res audio
pack), bitmaps (a texture pack, a HUD redraw), text (a translation), models
(higher-detail weapons, vehicles and scenery) and model and environment
shaders (how those are lit and shine). Only the machine that
has a mod plays it. Nothing of a mod is sent to another machine, and a mod
changes nothing that the machines of a network game must agree on, so you
can play with mods in any game, with players who have other mods or none. (A host's mod sounds of other lengths change how long
the host's scripts and AI wait for dialogue to end; the host decides those
for every machine, so they still agree.)

The game's code is `port/linux/game/mods.c` (with `mod_sounds.c`,
`mod_bitmaps.c`, `mod_strings.c`, `mod_models.c` and `mod_shaders.c`, one
for each kind of tag) and `port/linux/src/port_mods.c` (the mods folder and its files).

## A mod's folder

The mods are in the `mods` folder of the data root (the folder with
`maps/`; on Android the app's data folder, on the web the browser's storage
for the site). Each mod is a folder in it, with a `mod.toml` and a `tags`
folder:

```
mods/
  hd_audio/
    mod.toml
    tags/
      sound/sfx/weapons/assault rifle/fire.sound
      sound/dialog/...
  german/
    mod.toml
    tags/
      ui/hud/hud_item_messages.unicode_string_list
```

The files in `tags` have the paths of the tags they replace: a tag's name,
then its group's extension, as the Halo Editing Kit and Invader keep them.
`tags/sound/sfx/weapons/assault rifle/fire.sound` replaces the sound tag
`sound\sfx\weapons\assault rifle\fire` of every map that has one. Names are
not case-sensitive. The game only looks up the names of the map's tags among
a mod's files: a mod cannot add tags, and no tag name of a map makes a path
outside the mod.

`mod.toml` says what the mod is. Every key is optional:

```toml
name = "High-res audio"       # what the Mods menu shows (else the folder's name)
id = "hd_audio"               # what other mods name it by (else the folder's name)
version = "1.0"
authors = ["Someone", "Someone else"]   # or one string
description = "The game's sounds at 44 kHz"
maps = ["a10", "bloodgulch"]  # the maps it is for, by their files' names (else every map)
load_after = ["hd_textures"]  # mods it plays over: they load before it
load_before = ["fixes"]       # mods that play over it: they load after it
incompatible = ["other_audio"]  # mods it cannot be played with
format = 1                    # the mods' format it is made for
channel_order = "pc"          # "xbox": its multipurpose maps and HUD meters are in the Xbox's channel order
```

`load_after`, `load_before` and `incompatible` name other mods by their `id`
or their folder's name, not case-sensitive. A mod they name that is not in
the mods folder is passed over. `maps = []` is every map.

A folder without `mod.toml` is not a mod. A mod made for a later `format`
than the build's, or whose `maps` names no map, is not played. The game
logs anything else wrong in `mod.toml` (an unknown key, a value of the
wrong type) and uses the rest; a `mod.toml` that is not TOML at all is
logged, and the mod played without what it says.

The game's log is its output: the terminal on Linux, `halo.log` beside
`halo.exe` on Windows, logcat on Android, and the page's log on the web.

Limits: 256 mods, folder names of 63 bytes at most, 262144 files in all, 24
folders deep under `tags`, and 64 maps in a mod's `maps`.

## The load order

Every mod in the mods folder is on until you turn it off. The mods load in
an order, and where two mods have a file for the same tag, the one loaded
later plays.

- The default order is the folders' names in alphabetical order (not
  case-sensitive), except that the mods a mod must load after (those it
  asks to load after, and those that ask to load before it) are taken to
  just before it. Mods that ask in a ring (a after b, b after a) load
  alphabetically, and the game's log says so.
- The main menu's **Mods** (in the PC version's menus, `display.menus =
  "pc"`, the default) is laid out as the game lobby is: the mods on the
  left as a resource pack list has them, the one at the top playing over
  all those below it (the load order upside down: the mod loaded last is at
  the top), and the chosen mod's details in the frame on the right (its
  name, what is wrong with it, whether it is on, its priority, 1 the top,
  and its version, authors, folder, maps and description). Choosing a mod
  (A, or a click on the chosen one: the first click chooses it) turns it on
  or off (ON, OFF); left and right, or **Move up** and **Move down**, move
  the chosen mod (up: over one more mod). **OK** keeps the order and plays
  it at once (on a map that is loaded, too); **Cancel** leaves it as it was.
- A warning sign beside a mod means it is on the wrong side of a mod it
  asks to load before or after (both on): a mod that loads after another is
  above it in the menu. A red cross means a mod that is on says it cannot be
  played with this one, or that it cannot be played at all (made for a later
  build, or its `maps` names none): it stays off. The details say which.

The menu keeps the order, and the mods turned off, in `mods/load_order.toml`,
by the mods' folders' names:

```toml
order = ["german", "hd_audio", "hd_textures"]
disabled = ["hd_textures"]
```

A mod added to the folder later is put where the default order puts it,
after the mod before it there (but on the side it asks for of the mods it
names), and is on. A mod gone from the folder is left out. When the file
cannot be written, the game says so in its log and keeps the order until it
quits.

### Mods that cannot be played together

When a mod that is on names another that is on as `incompatible`, the game
asks as it starts, before the window opens:

- **Play anyway** plays them all (closing the box does too);
- **Turn off ...** turns off the mods named as incompatible (if mod A says
  it cannot be played with mod B, B is turned off), and keeps that in
  `load_order.toml`;
- **Quit** closes the game.

The Mods menu turns them on again. On the web, **Quit** starts the page
again. A run nobody watches (`debug.hidden_window`, `debug.null_renderer`,
`debug.exit_after` or `debug.network_test`) is not asked: it plays them all,
and the game's log says which conflict.

At the console:

| Command | What it does |
| --- | --- |
| `mods_list` | Lists the mods in the mods folder in their load order, and which are on, then how many of the map's string lists, bitmaps, sounds and models the mods play. |
| `mods_reload` | Finds the mods again and reads their files again: a changed file is read again (one that now fails a check leaves what was played before), a new one is played, and the map's tag comes back for a file that is gone. If a sound changed, all sounds stop. It does nothing while `mods_enable false`. |
| `mods_enable false` | Plays the map's own tags until `mods_enable true` or the next map. |

These commands work in any game, a client's too: they change nothing of the
game.

### Getting a mod onto each platform

- **Linux, Windows**: put the mod's folder in `mods/` next to `maps/`.
- **Android**: put it in `mods/` in the app's data folder,
  `/sdcard/Android/data/com.halo.decomp/files/mods`, for example with
  `adb push hd_audio /sdcard/Android/data/com.halo.decomp/files/mods/`.
- **Web**: on the page, **Mods**, **Add a mod folder…** copies a mod's
  `mod.toml` and `tags` folder into the browser's storage for the site.
  **Remove** deletes it again.

## What a mod can replace

The game plays these groups from a mod. It does not read the files of any
other group (a `.weapon`, `.vehicle`, `.model_collision_geometry` or
`.scenario`, for example): they would change what the machines of a network
game must agree on.

| Group | Extension | What it is |
| --- | --- | --- |
| Sound | `.sound` | Sounds, music and dialogue |
| Bitmap | `.bitmap` | Textures, sprites, the HUD's pictures |
| Unicode string list | `.unicode_string_list` | The text the game shows |
| String list | `.string_list` | Other text |
| Model | `.gbxmodel` (Halo PC's), `.model` (the Xbox's) | What objects look like: weapons, first-person weapons and arms, vehicles, scenery, characters |
| Model shader | `.shader_model` | How an object's model is drawn: its maps, self-illumination, specular and reflection |
| Environment shader | `.shader_environment` | How a level's surfaces are drawn: base, detail, bump, self-illumination, specular and reflection maps |

A file must be a Halo PC tag file of its group, as the Halo Editing Kit's
tools and Invader write them (a model also as `invader-extract` writes an
Xbox map's: `.model`). The game checks every count, size and offset
in it against the file, and every value against what the game reads, before
it uses the file. A file that fails a check is left out (the map's tag
plays), and `debug.txt` and the console say why.

### Sounds

- The samples can be 16-bit PCM, Xbox ADPCM or Ogg Vorbis, mono or stereo,
  at 22 or 44 kHz. The game converts them to the Xbox ADPCM that it plays,
  as it does for Custom Edition maps: mono at 22 kHz, stereo at its own rate.
- Empty values are filled in as a map's build fills them in: the defaults
  of the sound's class, the playback rates and the bend.
- A sound must have permutations: a file with an empty pitch range (some
  Halo PC translations have them) is left out.
- A permutation must fit in a block of the sound cache (one sound's samples:
  `MAXIMUM_SOUND_DATA_SIZE`).

### Bitmaps

- A bitmap's types and formats are those the game draws: 2D, 3D and cube
  maps; A8, Y8, AY8, A8Y8, R5G6B5, A1R5G5B5, A4R4G4B4, X8R8G8B8, A8R8G8B8,
  DXT1, DXT3, DXT5 and P8 bump. Sides are 4096 at most (512 for a 3D
  bitmap). A bitmap that is not a power of two on each side is drawn
  linear, as a Custom Edition map's is: it must be an uncompressed 2D
  bitmap.
- One bitmap's pixels may take a quarter of the texture cache: 64 MB on
  Linux and Windows, 5.5 MB on Android and the web (a 2048 x 2048 DXT5 with
  its mipmaps fits on all of them).
- A bitmap replaces the map's bitmap of its name, and the map's other tags
  use it as the map's: it must have at least as many bitmaps, of the same
  types, and as many sequences, each with at least as many sprites. It can
  be larger: a redraw at twice the size is drawn at the map's size, sharper.
- The HUD draws a bitmap at its own size. So for each HUD element that draws
  a mod's bitmap, the game scales the element by the map's size of what it
  draws (its sprite, or with no sprites its bitmap) to the mod's, when that
  is the same ratio for all of them. A bitmap the HUD draws that is not the
  map's redrawn so (Halo PC's HUD bitmaps lay their sprites out to other
  sizes) is left out. The HUD's digits can be the map's size or twice it.
- The menus draw a bitmap in their widgets' bounds, whatever its size.
- Halo PC keeps the channels of a model shader's multipurpose maps and of
  the HUD's meters in another order than the Xbox. The game finds those
  bitmaps among the mod's and draws them in the Xbox's order, as it does for
  Custom Edition maps (`custom_edition_bitmaps.c`). A mod whose bitmaps are
  already in the Xbox's order (taken from an Xbox map, say) says
  `channel_order = "xbox"` in `mod.toml`, and they are drawn as they are.
  It is the whole mod's: keep bitmaps of the two orders in two mods.
- The pixels are read from the file when the texture cache loads them, not
  kept in memory.

### Text

- A list keeps the map's number of strings: a mod's strings past it are
  left out, and the map's own strings stay where the mod's list is shorter.
- The game uses many strings as the format of what it prints
  (`%s was killed by %s`). A mod's string replaces the map's only if it has
  the same conversions in the same order (`%s`, `%d`, ...); otherwise the
  map's string stays. A translation keeps the conversions of the original.
- Unicode strings are UTF-16, as the tool writes them.
- The main menu is the port's own (the PC version's menus, `port/assets/menus`),
  whose text is in tags named `pc\...`. A mod's `ui\shell` lists change the
  Xbox's menus (`display.menus = "xbox"`) and the text shown while playing
  (HUD messages, the multiplayer messages, the pause menu).

### Models

A model is more than what is drawn: its nodes (its skeleton) are what its
animations move, its markers are where projectiles leave a weapon, where a
unit sits and where things attach, and its regions and permutations are what
damage and an object's variant choose among. The machines of a network game
agree on those, so a mod's model must have the map's:

- the same nodes: their names, hierarchy and default positions;
- the same markers: their names, regions, permutations, nodes, positions
  and rotations (on the model's permutations, as the Halo Editing Kit keeps
  them, or on the model, as a map does);
- the same regions and permutations: their names and flags.

Positions and rotations may differ from the map's by 0.001 at most (a model
exported again is not always bit for bit the map's). Only what is drawn is
the mod's: the geometries, their parts and vertices, which geometry each
permutation draws at each detail level, the detail levels' cutoffs, the
base map's scale, and which shaders each part is drawn with. A detail level
the map draws must draw one of the mod's geometries, and one the map draws
nothing at stays so. Every shader must be one the map has (a mod cannot add
tags); a mod's shaders and bitmaps can redraw them.

The model is checked against each map's own model of its name as that map
loads. On a map whose model of that name has other nodes, markers or
regions (a custom map's own assault rifle, say), or lacks a shader the mod
names, the mod's model is left out and the map's drawn; the mod's other
files still play there. `maps` in `mod.toml` keeps a mod to the maps it is
made for.

- Where a mod has both, its `.gbxmodel` plays; otherwise the file of the mod
  loaded later, whichever its format.
- What the tool works out as it builds a map is worked out as it does: a
  part's strip is its triangles block, its centroid's nodes are its most
  weighted, its links of 0 to the parts drawn before and after it are none,
  a base map scale of 0 is 1, and a model whose tag asks for it has the
  normals of its vertices that share a position blended. The vertices are
  compressed as the game draws them.
- A part's links to the parts drawn before and after it (for transparency)
  must not go round.
- A model of 44 nodes or more (the renderer skins 43 at once; only Custom
  Edition maps' have so many) is drawn a part's own nodes at a time, as a
  Custom Edition map's is: a part's vertices may name 22 nodes at most.
- The collision geometry, the animations and the object's tag are the map's:
  a larger weapon still hits and is hit where the map's does, and an object
  is still left undrawn by the map's bounds of it (a much larger model may
  vanish at the screen's edges).
- Every model made from a file is kept until the map changes, a file edited
  and read again by `mods_reload` too, and counts against the mods' memory.

### Shaders

A mod's `.shader_model` and `.shader_environment` files replace the map's
model and environment shaders of their names: everything Halo PC's tag has
(its flags, maps, scales, functions, colors, self-illumination, specular and
reflection) is the file's. The other shader groups (transparent, glass,
water, meter, plasma and the chicago shaders) are the map's.

- A shader's material type says how projectiles, footsteps and impacts meet
  what it covers, and the machines of a network game agree on those: it is
  the map's, as are its physics flags (a mod's shader of another material
  is drawn as the file has it but hit as the map's is).
- A mod cannot add tags: every bitmap the file names, and an environment's
  lens flare, must be one the map has, of the kind the field draws (a cube
  map where a reflection is drawn, a 2D texture elsewhere), and an
  environment's base map must have bitmaps. A mod's bitmaps can redraw them. A
  file that names a tag the map has not got is left out on that map.
- Every enum must be within its values and every number finite, as for a
  map's own shader.
- What the tool works out as it builds a map is worked out as it does: an
  environment's bump map scale for x and y is its bump map scale, a black
  material color is white, a model's map scales of 0 are 1, and a
  reflection falloff distance past its cutoff makes both 0.
- What Halo PC's tag has not got (where it is padding, the Xbox has fields)
  is the map's.

A model's parts can be drawn with a shader of every group Halo PC has but
one: `shader_transparent_chicago_extended`, which Gearbox added for Halo PC.
The Xbox game does not have it, so do not use it: use a
`shader_transparent_chicago` shader instead. (A Custom Edition map's
chicago extended shaders become chicago shaders as the map loads, keeping
only their four-stage maps: see
[custom_edition_caches.md](custom_edition_caches.md).) As for every
shader, it must be one the map has.

## Making a mod

1. Get the tags you want to change as tag files. `invader-extract` extracts
   them from a map, and the Halo Editing Kit's tags are Halo PC's own.
2. Change them with the Halo Editing Kit or Invader (`invader-bitmap`,
   `invader-sound` and `invader-model` build bitmaps, sounds and models from
   images, audio files and JMS files). Keep each tag's name.
3. Put them in `tags/` of a folder in `mods/`, with a `mod.toml`.
4. Load a map (a new mod is on). `mods_reload` at the console reads the
   files again as you change them, without loading the map again.

`debug.txt` has a line for every file the game leaves out, and why, and a
summary of what each kind of tag plays from the mods:

```
mods: 9 string lists from their files (9 new), 0 given back, 0 left out
mods: 66 HUD placements scaled to draw larger bitmaps at the map's size
mods: 354 bitmaps from their files (354 new), 0 given back, 6 left out
mods: 313 sounds from their files (313 new), 0 given back, 2 left out
mods: 4 models from their files (4 new), 0 given back, 1 left out
mods: 12 shaders from their files (12 new), 0 given back, 0 left out
```

## How it operates

- When a map's tags load, at `mods_reload` and at the Mods menu's **OK**,
  the game lists the files of the mods that are on, once, in a table of
  their tag paths. Each of the map's sound, bitmap, string list, model and
  model and environment shader tags is looked up in it.
- A tag replaced by a mod takes the file's values in place, so everything
  that holds the tag keeps it, and the map's own values come back when the
  file goes. A sound playing or a texture in the cache is never changed
  under the game: before a reload changes anything, the sounds stop and the
  caches let go of what they hold.
- The mods' tags keep at most 512 MB in memory (192 MB on Android and the
  web): their files, their sounds' samples, their bitmaps' tags and their
  models' geometry.
- A Custom Edition map takes mods as an Xbox map does.

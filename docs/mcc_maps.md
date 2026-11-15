# Halo 1 MCC custom maps

The companion MCC implementation adds an independent loader for modern Halo 1
MCC version-13 caches and **MCC SINGLEPLAYER** / **MCC MULTIPLAYER** choices in the map
menus. It is an experimental
runtime adapter, with explicit format limits below. A successful menu
probe or file audit is not proof that a map plays correctly.

This document describes the MCC map format the port reads, how the
implementation is laid out and what it does not support. Its notes on
particular maps describe the maps examined during development; they are not
a guarantee for every map. The [reading guide](reading/README.md#scope-and-provenance)
follows the implementation through the sources.

## Loading feedback

MCC map conversion displays OpenCE's original animated loading effect. Its
progress follows resource checks, conversion and validation, then initial
scenario setup. It represents completed work, not a time estimate. Long audio
and texture operations refresh the display between bounded pieces of work;
a single blocking read or allocation can still pause it briefly. A refused
map closes the effect and shows its package/level and the failed check. Missing
or incompatible resources name the resource file and where to install the
matching MCC copy. Unsupported script names and conversion stages are reported
when known; `debug.txt` retains the detailed checks. The existing menu return
and network-session abort behaviour is unchanged.

The screen uses the Xbox installation's `maps/loading.tga` imported from the
disc image. If that file is missing, the original renderer has no loading
picture to show. Lobby/catalog checks before loading are unchanged. The effect
does not run gameplay or alter network readiness, authority or map validation.

The [loading presentation setting](loading_presentation.md) also applies to
MCC conversion: widescreen is on by default, and turning it off restores the
central 4:3 framing. The original mask and effects remain, and the Xbox chant
stops when loading finishes or fails. PC Settings > Video > Graphics exposes
the toggle, including through the PC pause menu; Xbox menus are unchanged.

## Installation and menus

OpenCE's stock selector offers MCC singleplayer and multiplayer categories,
package folders, campaign difficulty selection, previews and completion badges.
The same menu runs on every platform. See
[MCC menu images and badges](reading/mcc_level_previews.md) for metadata and
completion-record limits. Fork-specific Modern and Xbox-style menus are not
part of this upstream contribution.

Put the map in the game data root's `mcc_maps` directory, beside `maps` and
`custom_maps`. For example:

```text
maps/                 Xbox game data
custom_maps/          Halo Custom Edition data
mcc_maps/a10.map       Halo 1 MCC custom map
mcc_maps/a10.txt       Optional plain-text description
mcc_maps/sounds.map    Matching MCC resources, when required by the package
mcc_maps/bitmaps.map   Matching MCC resources, when required by the package
```

You can also copy an entire downloaded Halo 1 Workshop folder from
`steamapps/workshop/content/976730/<item id>` into `mcc_maps/<item id>`.
Leave its `maps`, metadata, images and `halo1/maps` resource folders intact.
Local discovery does not require Steam or the Steam Discovery setting.
The menu reads the package title from `ModInfo.json` and level names and PNG
previews from the referenced metadata. Missing metadata retains the normal
filename labels and placeholder image; a numeric package without a title
shows "MCC package". Unsupported or damaged images also use the placeholder.

Both disk layouts use `mcc_maps\<package>\<level>` internally. Local lookup
prefers a file directly in the package, then the same file in `maps`; for
`bitmaps.map` and `sounds.map`, it also checks the package's `halo1/maps`.
A present invalid direct file is not replaced by a nested or Steam copy.
Nested lookup rejects linked paths and special files. It searches only these
fixed locations, never arbitrary subdirectories or other packages. Manual
folders containing only level files remain valid. Copying a package does
not supply missing shared base resources; the existing resource requirements
and automatic sharing policy still apply.

The level identity is `mcc_maps\a10`. The filename is retained even when
the internal scenario name differs. The supplied Mercury Rising file is
an example: its filename is `a10.map`, while its header names
`mercury_falling`. The namespaces prevent it replacing either Xbox `a10`
or Custom Edition `custom_maps\a10`.

Some campaigns keep audio and textures in separate resource files. For manual
packages, keep each package's supplied files beside its levels. With desktop
Workshop discovery enabled, packaged and installed MCC resources are found in
place as described below. For installations without Steam discovery, install
MCC's two standard sets once:

```text
mcc_maps/_shared/singleplayer/bitmaps.map
mcc_maps/_shared/multiplayer/bitmaps.map
mcc_maps/_shared/multiplayer/sounds.map
mcc_maps/solar_flare/m0.map
mcc_maps/solar_flare/m1.map
mcc_maps/solar_flare/sounds.map
mcc_maps/solar_flare/bitmaps.map
```

Copy the `singleplayer` file from MCC's `halo1/maps/bitmaps.map`. Copy the
`multiplayer` files from MCC's `halo1/maps/custom_edition/`. Both must come
from MCC; do not substitute a Custom Edition installation. These folder names
are installation labels, not a restriction on the maps that can use them.
`_shared` is not a level package and its nested resource folders are not menus.

Selection is independent for bitmaps and sounds. A package resource is
selected first, then a legacy `mcc_maps/bitmaps.map` or `sounds.map` if present.
These explicit overrides are authoritative: unreadable, damaged or incompatible
files fail rather than being silently replaced. If neither exists, every
external reference of that type is checked against each installed shared set.
A complete matching file is required; partial matches are never combined.
Two complete but different matching files are ambiguous: put the intended file
beside the level. Byte-identical matches are interchangeable.

Self-contained levels do not open or fingerprint resources. Only required types
are considered. Chosen paths and failures are logged in `debug.txt`. The MCC resource reader
uses `bitmaps.map` and `sounds.map`, not CE's localization resource `loc.map`.
Native MCC maps carry their localization tags within the level; MCC's editing
kit does not use `loc.map` except for Custom Edition backwards compatibility
([editing-kit format notes](https://c20.reclaimers.net/h1/h1-ek/)). Android and
browser package importers can retain a bundled `loc.map`, but the MCC runtime
does not consume it. Custom Edition retains its own `loc.map` support.
Nothing searches other packages, `maps` or `custom_maps`.
A package level's identity is `mcc_maps\solar_flare\m0`.
Network peers may use different package folder names; the level filename and
complete map/required resource contents must match the host. Optional
`.txt` descriptions remain beside the corresponding level. No manifest is
required.

A package from the Steam Workshop can say which item it is, so a joiner who
cannot match the host's map is told what to do. The item is a `workshop_id.txt`
in the folder (the id on the first line and, optionally, the title on the
second), or the package folder when Steam's own name (the item id, all digits)
is kept, or else the `ModIdentifier.HostedModIds.SteamWorkshopId` that MCC's
tools write into the package's `ModInfo.json` (a number or a string, not 0, with
`Title.Neutral` as its title), so a package copied in under a name of its own
is still known. The host sends the id and title in the map name's reserved tail
(`mcc_workshop.c`). The original fork introduced this advisory extension under
MCC protocol 64. The upstream contribution uses one shared OpenCE protocol, 42,
including MCC content/metadata digests and checkpoint lifetime fencing; see the
[wire contract](../port/linux/NETCODE.md#opence-network-agreement).

The item is advisory: the level filename and content fingerprint still decide
admission. A known local item is recognized even if its older package lacks
the requested level. A matching filename without an item id does not prove
the item is installed. The refusal distinguishes an identified package that
does not match from a package that cannot be identified locally. Both messages
name the item and tell the player to copy the host's matching map and required
resources into `mcc_maps`; a map with no item id gets the fingerprint text,
naming the level file. A mismatch does not establish which player's copy is
newer. This advisory identity does not itself discover Steam libraries or synchronize
manually copied packages. Desktop discovery uses the separate Workshop catalog
integration described next.

### Subscribed Steam Workshop items

Installed packages can also supply [level previews](reading/mcc_level_previews.md).
Keep their listed map/mission metadata and PNGs together. Stock menu panels use `Images.Large`, then `Images.Thumbnail`, with
the question mark as fallback. Pictures fit and clip to the existing thumbnail
rectangle, and MCC titles/descriptions wrap within their text boxes. This also
works for complete Android imports without Steam and does not enable sharing.

With `game.workshop_maps = true` (the default; `HALO_WORKSHOP_MAPS=1`), desktop
builds list Halo CE maps from the Workshop items a player is subscribed to
without copying anything into the data root. `steam_workshop.c` looks in Steam's
libraries for `steamapps/workshop/content/976730/<item id>/`: the usual Steam
folders (on Windows `%ProgramFiles(x86)%\Steam`; on Linux `~/.steam/steam`,
`~/.local/share/Steam`, and the Flatpak and Snap places), the `paths.steam`
setting (`HALO_STEAM_ROOT`), and every library their `libraryfolders.vdf`
names. On Windows it also reads `SteamPath` from the current user's
`Software\Valve\Steam` registry key and `InstallPath` from both the 32-bit and
64-bit machine-wide views of that key. Thus an installation such as `C:\Steam`
is discovered without editing the configuration or running Steam. The manual
`paths.steam` root is checked first, followed by registered installations and
standard folders; its copy of a duplicate item takes precedence. Missing,
unreadable, non-string or oversized registry values are ignored. The detected
path is not written to the configuration: an empty setting continues to use
automatic discovery on the next scan.

On Linux, after the manual override, discovery checks the literal absolute path
in `STEAM_COMPAT_CLIENT_INSTALL_PATH` when the launch environment supplies it,
then `$XDG_DATA_HOME/Steam`. It also keeps the home-folder fallbacks above and
adds `~/.steam/debian-installation`. The existing `~/.steam/steam` and
`~/.steam/root` links can point to an installation anywhere the user can read;
each discovered root's library list can locate libraries on other drives.
For example, `XDG_DATA_HOME=/mnt/data/player` finds
`/mnt/data/player/Steam` with `paths.steam` empty. Missing, stale, relative,
control-character or oversized environment hints do not prevent fallback
discovery. Paths are not expanded as shell expressions or written to config.

The location evidence is the [XDG base-directory specification](https://specifications.freedesktop.org/basedir/0.8/),
Valve's use of the client-install hint in [Proton's launcher](https://github.com/ValveSoftware/Proton/blob/proton_10.0/proton),
and the [Valve maintainer's description of Debian's installation folder](https://github.com/ValveSoftware/steam-for-linux/issues/9347).
These are discovery hints, not an exhaustive drive search. An installation with
no accessible hint, link or known location still needs `paths.steam`; sandbox
filesystem permissions can also prevent discovery.

An item is used when its `ModInfo.json` says `"Engine": "Halo1"`; each
path in `GameModContents.MultiplayerMaps` is a map json whose `ScenarioFile`
names the `.map` file in the item's `maps` folder, whose `Title.Neutral` is the
map's title and whose `Images.Thumbnail` is its preview picture. Folder names
inside an item are found without regard to case, as Steam's files are spelled
by their makers. An item seen through two paths to one library is listed once.

A multiplayer item may list up to 1,024 maps, matching the existing total
Workshop catalog capacity. The former 64-entry item limit rejected whole
collections, including an inspected 75-entry Halo 1 package; it was a reader
capacity, not an MCC format restriction. The reader validates every listed
path and retains the 256 KiB JSON limit. Larger lists are refused with a
diagnostic naming the package title, entry count and supported limit. The
combined catalog still holds at most 1,024 maps across all packages. Campaign
progression retains its independent 64-mission limit.

`tools/test_steam_workshop.py` exercises 64/65/75/1,024-entry manifests,
unrelated package identities, late malformed paths, catalog saturation and
disable/re-enable/rescan behavior through the real reader and discovery code.
Local menu metadata and previews use the same multiplayer bound, including
Android/web packages; `tools/test_mcc_previews.py` checks titles and images
from entries beyond the old limit. Discovery does not establish that every
map's gameplay content is supported.

Discovery defaults to off. Android and web use installed `mcc_maps` packages;
they do not search for Steam. Local package folders take precedence regardless
of their letter case. The path translation hook is necessary because the shared
file reader opens virtual Xbox paths; it handles only `mcc_maps` requests and
leaves `maps` and `custom_maps` on their established paths.

Each map is the level `mcc_maps\<item id>\<map name>`, so it is the same
package identity as a folder named by the item id (above), and the host's item
id and title and a joiner's refusal text work as for any such package. The path
translation (`platform_translate_path`) opens `mcc_maps\<item id>\` as the
item's `maps` folder in Steam's library. A package folder of that name in the
data root's `mcc_maps` wins, so a player can still override an item by hand.
The MCC menus show the map's Workshop title, and a package folder shows the
item's title instead of its id. The resources a map needs (`bitmaps.map`,
`sounds.map`) are selected independently. A Workshop resource beside the level
in the item's `maps` folder takes precedence; when absent, the same item's
`halo1/maps` file is used. An existing entry remains authoritative even if it
is damaged or unreadable. Nothing borrows another Workshop item's resources.
A local `mcc_maps/<item id>` package also prevents borrowing that item's files.

If package and legacy root resources are absent, the existing shared-set
selection checks both standard sets. A manually installed file under
`mcc_maps/_shared` wins. Otherwise, with Workshop discovery enabled, each set
resolves in the first discovered Steam library containing the corresponding
MCC file, under `steamapps/common/Halo The Master Chief Collection/`:

- `halo1/maps/bitmaps.map` for the singleplayer set;
- `halo1/maps/custom_edition/bitmaps.map` and `sounds.map` for the multiplayer set.

These are MCC installation directories, not the separate Custom Edition game
or the data root's `custom_maps`. The setting's existing library order applies;
`libraryfolders.vdf` includes installations on other drives. Disabling discovery
keeps manually installed package/root/shared paths. Android and web retain
installed/imported packages and do not search Steam.

Discovery only supplies paths. `mcc_resources_select` still validates the file
and every external reference, rejects partial matches and ambiguous different
matching shared files, and fingerprints the selected bytes rather than their
location. Self-contained maps open no unrelated resources. Selected streams
are owned by the current MCC runtime and closed on unload or failed loading.
No map names, checksums or Workshop-ID exceptions select this behavior.

The layout evidence is the installed Halo 1 MCC shared directories and multiple
Workshop items with authored resources under `halo1/maps`. Their metadata can
say `HasSharedFiles: false` while those files are present, so discovery uses the
bounded, conventional resource paths and the reader validates the contents;
that metadata flag does not override an installed package resource. The synthetic
regressions in `tools/test_steam_workshop.py` exercise unrelated item identities,
case-insensitive paths, overrides, missing types, library roots and bounds.
`tools/test_mcc_resources.py` checks complete-reference matching, ambiguity,
malformed resources, independent bitmap/sound selection and content identity.

The catalog is read the first time MCC maps are needed, and again when a joiner
cannot find the host's map (so an item subscribed just now is found). Dedicated
servers also refresh after `sv_reload`, on a pregame frame; a request during a
game waits until the lobby. Web map-list reads remain cached. The json
files are the item makers' text: one that is too large (256 KB), too deep, not
an object, or that names a path leaving its item, refuses that item or map and
nothing else.

### Workshop collections

Several Workshop items belong to a collection that their makers published as a
set (a competitive pack, a social pack, a 2v2 set). `workshop_collections.c`
holds a fixed, curated set of Halo CE collections as data: a name and the ids of
its items. It does not query current Steam collection membership.
`game.mcc_collections` (`HALO_MCC_COLLECTIONS=1`) enables these groups by
default. Reopen the map list after changing it. Imported packages can identify
their collection through `workshop_id.txt`, without Steam. At the
top of the MCC map menus a collection the player has an item of is a folder
(`[PRO EDITION]`), before the package folders; it holds the maps of the
player's items that are in it, by their Workshop titles. An item that is in two
collections is in both folders; an item in none keeps the package folder of its
own, named by its title. A collection with no subscribed item has no folder.
Only the menu changes: levels are still `mcc_maps\<item id>\<map name>`, so
a host's map and a joiner's messages do not depend on collections. A collection
the table does not know is shown as its items' own folders.

The two MCC categories show package folders first (alphabetically, in brackets),
then loose maps. Open a folder to choose one of its levels; the rows omit the
repeated package suffix, while the selected map's details retain its identity.
Only folders with maps of the selected category appear. A mixed package appears
in both categories, with campaign and multiplayer maps filtered separately.
Back returns from a folder to the package list; at the root it exits as before.
Campaign difficulty selection still follows mission selection. Returning from
that screen restores the MCC folder and selected mission. Each category remembers
its folder/selection for the current session, separately for New Game and hosting.
A rescan restores selections by identity, and returns to the root if a package
has disappeared. `_shared`, resources and empty/unsupported-only folders are not
selectable packages.

Folder grouping lives in `mcc_maps.c`. The existing MCC menu callbacks bind its
rows to the shared widgets. MCC Back handlers on the enclosing screens and
lists are needed because screens consume Back before their children; two
MCC-only description widgets show folder and empty-category text. The common description update
hides those widgets when leaving MCC. Xbox/CE lists, map identities, loading,
resource selection, checkpoints and network messages are unchanged.

Open the map-kind chooser and select **MCC SINGLEPLAYER** for campaign
maps or **MCC MULTIPLAYER** for multiplayer maps. The cache header's scenario
type determines the category, independently of the filename; both lists
read `mcc_maps` and its immediate package folders. New Game's campaign maps proceed to difficulty
selection, and multiplayer maps can be explored alone. The network host
map screen routes campaign maps to cooperative setup and multiplayer maps
to gametypes. Split-screen multiplayer skips **MCC SINGLEPLAYER**, just as
it skips the existing singleplayer categories. These
are menu routes in the implementation; multiplayer interoperability and
complete campaigns require gameplay testing.
Without campaign metadata enabled, each selected MCC campaign map remains an
independent scenario. **CAMPAIGN MISSIONS** under Settings > Network Setup >
MCC Packages (`game.mcc_campaigns`) is enabled by default to discover Workshop
campaigns and use their authored mission order. The same screen exposes desktop
Steam discovery and collection folders, also enabled by default. Players can
turn each option off; saved choices and environment overrides are respected.

`ModInfo.json` must identify `Halo1` and `GameModContents.HasCampaign = true`.
`CampaignInfo.json` supplies an explicit `CampaignMaps` list; each referenced
mission JSON supplies `ScenarioFile` and an optional `Title.Neutral`.
The order in that list is authoritative, even when filenames sort differently.
The last component of `ScenarioFile` names the compiled map, following the
[public Excession format](https://learn.microsoft.com/en-us/halo-master-chief-collection/excession/creatingmodpackage/specifyingproperties).
A package contains one campaign, whose mission list can be reordered in
[Excession](https://learn.microsoft.com/en-us/halo-master-chief-collection/excession/creatingmodpackage/campaignmod).
The bounded reader supports 1-64 missions, rejects duplicate paths or filenames,
and accepts the default mission kind. Hub/flashback semantics and insertion-point
selection are not implemented; missions start normally at their beginning.

For local packages, keep `ModInfo.json` and `CampaignInfo.json` at the package
root and the referenced mission metadata in its original subfolders.
The maps may remain in the Workshop `maps` folder or sit directly in the root.
The Android importer performs this layout conversion and preserves the Workshop
ID in `workshop_id.txt`. Metadata never redirects loading to another package,
Xbox or CE. A local override never borrows metadata from its Steam counterpart.
Missing map binaries remain in the declared order: completion stops with a
diagnostic instead of skipping a mission. Unlisted maps remain independently
selectable and do not acquire an inferred next mission.

Solo completion queues the next explicit MCC level with the current difficulty;
the last mission returns to the menu. In co-op only the host chooses the next
mission, using the existing round transition and content-admission messages.
The lobby selects that mission; the final mission returns to the lobby without
wrapping to the first. Clients need matching map/resource content, not identical
metadata or package folder names. No network fields or agreement rules changed.
The minimal `main_won_map_private` hook is necessary because the established
completion table cannot represent MCC package identities; all selection logic
remains in MCC modules, behind loaded-campaign and setting guards. Xbox and CE
completion paths are preserved.

MCC checkpoints retain the canonical `mcc_maps\[package\]<filename>` scenario
identity, including when the original tag names an Xbox campaign level.
Continue and Load Game have MCC-specific routes with separate saved-name
storage; they do not turn an MCC checkpoint into a stock campaign level.

The catalog scans `mcc_maps`, one package directory level and each package's
fixed `maps` subfolder. It reads each candidate's 2,048-byte header,
and refreshes when a map menu opens. It accepts at most 1,024 entries, with
relative level paths of at most 54 characters before `.map`, including the
package name and separator; the physical `maps` subfolder adds no identity
characters. Other deeper nesting, traversal and invalid names are
rejected. Moving an existing loose map into a package changes its saved-game
identity; keep it at its old location to continue an existing checkpoint. A description is optional;
the default identifies the scenario type. Previews use explicit package metadata,
not guessed image sidecars. There is no
dependency on the Custom Edition setting or its resource directories.

The loader opens source maps read-only. A rejected map reports the failing
stage in `debug.txt` and returns a menu error. Network admission compares all
32 bytes of a SHA-256 fingerprint covering the complete map and each required
external resource file. The MCC tag reader determines those dependencies; the
same validated resource selection is used for conversion and fingerprinting.
File types and lengths frame the hash input, while paths and unused resources
are excluded. Identical bytes in package, root or shared locations match.

The host sends this fingerprint in its existing settings record. Clients first
verify the exact installation path, then look for a unique matching same-filename
map in their MCC catalog. Ambiguous fallback matches, missing/mismatched resources,
or more than 16 same-filename candidates are refused with an explanatory error.
The selected local path stays local; readiness replies and game options retain
the host's session name. A fresh fingerprint is checked at actual loading too.
The host's advertised name still follows the existing network path validation.
Map changes and disconnects discard the binding. No client gains host authority,
and content matching does not prove that an untrusted client executes honest code.

The issue #51 metadata cache still avoids full resource reads on each lobby
readiness poll. `mcc_cache_checksum` prepares the host's SHA-256 identity
during map-version calculation, before opening an initially MCC lobby.
Fast setup's early MCC selections wait in `mcc_ui_network` until the host's
local client has joined its temporary lobby; they then use the normal map
change path. Starting the temporary map is blocked while a selection waits;
legacy selections and session teardown cancel the pending request.
Deferring hashing until the first settings send blocked the host's local client beyond
its five-second join deadline on large packages. `mcc_identity_prepare_host`
retains the existing host cache while map/resource metadata stays unchanged;
settings sends recheck those stamps and changed files require a fresh hash.
The local admission can reuse that prepared fingerprint only for an identical
canonical path, complete file snapshot and all 32 digest bytes. This avoids
hashing the host's unchanged package again while a remote client waits to join.
Other machines still fingerprint their own installation, and actual loading
always fingerprints the opened content independently. The timeout values,
wire format and fingerprint bytes are unchanged. Large packages still take
time to validate; this does not make synchronous I/O immune to stalls.
The previous 32-bit checksum
remains for checkpoint identity and engine bookkeeping; it is no longer the
MCC network content proof. Existing save identities are unchanged by SHA-256.
The MCC checksum consumers share the port's prefixed zlib CRC implementation;
resource selection/framing, saved checksum bytes and zero-identity handling
remain unchanged. See the [checksum reading route](reading/maps_and_resources.md).
All peers require the shared OpenCE protocol 42. Xbox and Custom Edition retain
their own map matching/loading behavior; the common wire and simulation agreement
is specified in [the network boundary](../port/linux/NETCODE.md#opence-network-agreement). Network
support uses OpenCE's host-authoritative engine, not official MCC clients.

The shared identity integration uses the settings record's appended digest,
the two host settings senders, client settings admission, and client teardown.
These hooks are necessary because MCC modules alone cannot attach data to the
existing host record or resolve a local path before its generic presence check.
Startup coordination also needs small server map-selection, idle and start
hooks: the server owns when a requested map is applied and when its round can
start. The pending request and its cleanup remain in `mcc_ui_network.inl`,
using the existing MCC server-disposal hook. Xbox/CE requests proceed through
their existing map-selection code immediately.
All hashing, package selection and binding lifetime policy lives in MCC modules;
Xbox/CE settings use a zero digest trailer and keep their original map checks. The
protocol recorder includes the full MCC settings record and its size/offset
bounds; all three map families use the shared OpenCE version 43 contract.

MCC's `halo1/maps/bitmaps.map` stores individual texture entries. Its
`halo1/maps/custom_edition` resources store grouped `__pixels` and
`__permutations` entries. Custom packages may use either layout.
The MCC reader supports both. Grouped reads require a unique exact tag-group name
and the original requested offset entirely within its validated payload. Grouped
slices are never relocated by guesswork. Bounds and name checks still apply.

### In-game pause menus

MCC maps use a generated, stock-style pause menu for campaign, cooperative
and multiplayer games. The full, half and quarter viewport layouts follow
the local player arrangement. They use separate MCC-owned widgets, strings,
panels and selection highlights; the default pause route does not open the
map's embedded pause or Settings widgets. Loaded font tags are referenced
without changing their contents.

| Context | Available actions |
| --- | --- |
| Local campaign | Resume Game, Settings, Revert to Saved, Restart Level, Save and Quit. |
| Cooperative host and client | The campaign actions plus Friendly Fire On/Off. Clients cannot perform host-only checkpoint/restart actions. |
| Local multiplayer and multiplayer client | Resume Game, Settings and Leave Game. |
| Multiplayer host | Resume Game, Settings, End Game and Leave Game. |

The full-screen menus match the Xbox layouts and content, including the
port's Settings and co-op Friendly Fire additions. The extra MCC Choose Team
and multiplayer Restart Game entries have been removed for content parity.
Authored widgets still use the existing MCC action adapter. Split-screen
players use Settings from the main menu, since the editor needs a full viewport.

Revert and Restart use the Xbox ATTENTION dialog: B/Back cancels and
A/Start accepts. Save and Quit uses the three-second SAVING screen and
preserves the last valid checkpoint; it does not create a new checkpoint.
The wording, including the inherited Xbox-console saving message, follows
the Xbox reference. Multiplayer Leave and host End Game act directly, as
on Xbox. Only local campaign menus pause simulation; network games continue.

Footers use separate B=CANCEL and A=SELECT widgets, with B=CANCEL and A=OK
on confirmations. Their bitmap placeholders reference OpenCE's embedded
small menu-button redraws, independently of the map's gameplay glyphs.
Panel and alert bitmap descriptors borrow the native menu loader's redraw
textures read-only, with MCC-owned groups and frame selections. Unload drops
these descriptors before the native menu loader releases the textures.
Selection highlights are drawn independently with antialiased rounded edges.

Friendly Fire changes only the existing `network.coop_own_friendly_fire`
preference, through the same configuration service as Xbox. The generated
button reloads its label after a successful change. This introduces no new
network message or authority: checkpoint/restart and End Game still require
the host; clients cannot turn the preference into control of other players.
The menu callbacks close only their initiating controller's tree and history.

All full-screen Settings entries open OpenCE's existing in-game audio, video
and controls editor, with upstream's option filtering and profile saving.

The MCC Settings route uses the initiating controller's profile and widget
history. It does not replace another active profile editor. Its ownership
ends on native Save/Cancel, menu closure, replacement editing or map unload,
so a later native editor cannot be closed by stale MCC state.

Campaign layouts display the scenario's current mission objectives through
the native objective-text callback. Owned objective and confirmation text
boxes wrap their instance copies using the current font's metrics and clip
to their bounds. The map's objective data, custom HUD
and embedded widgets remain present. Scripts that explicitly open embedded
widgets can still use the MCC callback adapter described below. The new
pause labels and confirmation text are currently English; localization of
these generated strings is not implemented.

## MCC sharing eligibility

The MCC-owned policy in `mcc_share_rules.c` applies to local and Workshop
packages. The host's `network.map_share` setting enables eligible packages
automatically after their gameplay content has been validated. Loose maps,
`_shared`, unsafe names and invalid origin states are refused. A Workshop ID
is advisory; it neither grants redistribution permission nor proves which bytes
are present. An existing `mcc_share.txt` can supply attribution, but no approval
list is required. Required JSON is separately identified and validated.

The planner uses the loader's validated resource selection. Self-contained maps
do not transfer unrelated resources; used package dependencies follow their
actual resolved paths, including Workshop `halo1/maps`. A used root/shared
resource is never transferred: the joiner must supply its matching local copy.
Package copies identified as base resources are excluded too. An invalid
package override cannot be silently skipped in favor of another resource.

### Matching local shared resources

When joining a host, OpenCE checks the installed base-resource candidates
against the host's full SHA-256 content fingerprint. An older file in the
MCC root does not hide a matching copy in either shared folder. Package-owned
resources keep precedence; a broken package override is not silently replaced.
The loader uses the exact resource selection that passed validation and checks
the loaded content again before joining. Missing matches require the player to
install the matching resources locally; the host never supplies base resources.

Host sharing remembers whether a package resource duplicates a local base file.
Repeated requests reuse that classification only while the map and every resource
candidate retain their file stamps. A changed candidate forces a fresh comparison.

## Sharing a package with a joiner

The host delivers MCC packages itself, with the joiner's consent: eligible
Workshop packages, recognized JSON/PNG assets and separate managed snapshots.
The messages are request **103** and response **104**; see
[NETCODE.md](../port/linux/NETCODE.md#mcc-package-sharing).
Custom Edition sharing and its messages remain independent.

Enable host `network.map_share` and joiner `network.map_download` (both default
on). The same host setting covers CE maps and MCC packages. A missing/differing package
prompts before any chunk download or stage creation. When either permission
is off, only manual installation or a valid Workshop page is offered as a
content-acquisition action, plus cancellation. Matching installed content
needs no download; content is not assumed newer/older solely from a mismatch.

The current mission's map and selected resources travel with required package,
campaign and mission JSON, supported referenced PNGs and attribution text.
Local map-only folders are valid; no JSON, artwork or Steam identifier is
invented as a prerequisite. Campaign metadata is delivered upfront and later
mission binaries are obtained as needed. Received metadata cannot introduce
arbitrary paths or remote downloads; invalid optional artwork is discarded,
while invalid required metadata blocks installation.

On-demand campaign progression uses the connected host's exact identity for
each next mission. A downloaded mission remains individually selectable, but
separate downloaded snapshots do not assemble an offline or rehosted campaign.
Campaign JSON specifies names/order, not every mission's binary digest; even
identical JSON and Workshop provenance cannot prove that another snapshot is
the intended revision. If the current snapshot lacks its next mission, the
existing progression handler refuses that transition. Install the complete
campaign from Workshop or a manual package for offline/rehosted progression,
or rejoin the original host for its next mission.

The receiver verifies files, metadata references and both advertised identities
inside its owned `mcc_staging/dl_<digest>` directory. Native publication moves
the whole directory without replacement into `mcc_maps/dl_<digest>`, preserving
existing manual and Steam copies. It rechecks that installed snapshot before
joining. These filesystem checks protect against remote paths; they are not a
sandbox against a malicious process with the same local filesystem authority.

Managed snapshots are listed, stored and removed from the menus; removing one
keeps the original package. See the
[implementation walkthrough](reading/maps_and_resources.md#mcc-package-sharing-between-machines).

## Repository architecture and isolation

The contribution targets OpenCE. `source/` contains the reconstructed
Xbox game systems, including scenario/tag access, cache I/O, the script
interpreter, rendering, audio, objects, game variants and UI. The native
platform layers under `port/linux/`, `port/windows/` and `port/android/`
provide the host services used by those systems. The desktop renderer
implements the game's Xbox interfaces using the existing native graphics
backend. Menu XML and generated settings define the native map chooser.
The `tools/` tests and native harnesses exercise both portable utilities and
selected game subsystems.

Existing Custom Edition support has its own catalog, cache reader,
conversion, resource and behavior modules. Those modules and their state
are not an MCC implementation dependency. The independent components are:

| Component | Responsibility |
| --- | --- |
| `port/linux/game/mcc_package.c` | Validated package paths and explicit resource overrides. |
| `port/linux/game/mcc_maps.c` | Catalog, display identities, names and descriptions. |
| `port/linux/game/mcc_cache_format.c` | Portable little-endian version-13 reader and file-range audit. |
| `port/linux/game/mcc_cache.c` | MCC path admission, read-only file lifetime, conversion and streaming dispatch. |
| `port/linux/game/mcc_identity.c`, `mcc_sha256.c` and `mcc_precache.c` | SHA-256 session content checks, local package resolution and metadata-based readiness caching. |
| `port/linux/game/mcc_main.c` | MCC load-failure recovery through the native main-menu lifecycle. |
| `port/linux/game/mcc_ui.c` and `mcc_ui_network.inl` | MCC in-map widget actions, settings routing, checkpoint/quit behavior and synchronized network restarts. |
| `port/linux/game/mcc_pause.c` and `mcc_pause_runtime.c` | Generated stock-style pause layouts, objective display, independent UI art and atomic publication/lifetime of the appended tags. |
| `port/linux/game/mcc_ui_teams.c` | Authenticated team-only requests during MCC multiplayer rounds, with host balance checks and normal respawn replication. |
| `port/linux/src/mcc_memory.c` | Separate 64 MiB linked tag window; never replaces an existing allocation. |
| `port/linux/game/mcc_geometry.c` | New model descriptors, model vertex/index conversion, external BSP vertex streams and node palettes. |
| `port/linux/game/mcc_audio.c` | MCC sound decoding, resampling and Xbox ADPCM encoding into an MCC virtual stream. |
| `port/linux/game/mcc_resources.c` | MCC-only resource selection, dependency fingerprints, indexed reads and file lifetime. |
| `port/linux/game/mcc_vorbis.c` | Private namespaced Vorbis decoder, bounded Ogg validation and empty residue vector compatibility. |
| `port/linux/game/mcc_bitmaps.c` | MCC pixel layout, BC7 decoding and shader/HUD channel normalization into an MCC virtual stream. |
| `port/linux/game/mcc_texture_cache.c` and `port/linux/src/mcc_texture_bridge.c` | Direct bindings of checked MCC pixel allocations and lazy GPU textures, independent of the Xbox/CE staging cache. |
| `port/linux/game/mcc_tags.c` | MCC metadata, shader-type, HUD placement and widget normalization. |
| `port/linux/game/mcc_hud.c` | MCC bitmap/placement scaling, including nested weapon and grenade HUD items. |
| `port/linux/game/mcc_hud_draw.c` | MCC canvas, glyph geometry and text advances; neutral dispatch for Xbox/CE. |
| `port/linux/game/mcc_terminal.c` | MCC terminal-only font scaling; preserves the map's HUD fonts and all legacy rendering. |
| `port/linux/game/mcc_scripts.c` | Name-based MCC function/global linking and supported MCC native functions. |
| `port/linux/game/mcc_campaign.c` | Local-player/authority queries, gravity, campaign markers and scoped global permissions. |
| `port/linux/game/mcc_syntax.c` | MCC's 32,767-slot syntax validation and traversal workspace. |
| `port/linux/game/mcc_script_parameters.c`, `mcc_script_runtime.inl`, `mcc_script_threads.inl` | MCC parameter metadata/scopes and bounded, checkpointed interpreter stacks. |
| `port/linux/game/mcc_objects.c` | MCC multiplayer vehicle placement masks. |
| `port/linux/game/mcc_grenades.c` | MCC slots 2/3, salted unit inventories, pickup/throw/drop behavior and HUD. |
| `port/linux/game/mcc_player.c` | MCC grenade selection/action checks and starting-profile counts. |
| `port/linux/game/mcc_network.c` | MCC host inventory and campaign-state messages over the existing distributed transport. |
| `port/linux/game/mcc_checkpoint.c` | MCC inventory/campaign snapshot inside proven-unused CPU save memory. |
| `port/linux/game/mcc_tag_validate.c` | Independent range/ownership state while applying stock tag schemas and their validators. |

Shared engine entry points have additive MCC dispatch for an MCC-namespaced
level or active MCC state. They route cache lifetime, raw resources, BSPs,
script extensions, validation callbacks, menu text and model palettes to
the new modules. For other levels those MCC checks do not select a loader.
The new code does not call the Custom Edition parser, converter, allocator,
script adapter, bitmap converter, sound encoder or behavior table. MCC
conversion writes only MCC-owned allocations, and unload releases them.

The pause builder appends its definitions and art to a copy of the live
tag index only after construction succeeds. It preserves the existing tag
records and restores the original index on unload. Its action IDs use the
separate `0x7000` range and require exact ownership of a generated widget;
an embedded map widget cannot acquire those permissions by copying an ID.
Allocation or graphics-resource failure releases the partial construction
without publishing it. Xbox and Custom Edition pause selection remains on
the existing route.

Maps with two through four grenade definitions are supported. The original
unit datum and player-action layouts are retained; the extra two counts
live in a separately owned table keyed by full salted unit handles.
Selection remains in the native unit fields. MCC profile, input, HUD,
pickup, throwing and inventory-network routes handle these slots.

MCC saves put the additional counts and campaign state into a 66,392-byte slot at the CPU
arena's end only if the allocator's current high-water mark leaves that
entire slot unused. No `game_state_malloc` call, pool capacity, native
allocation checksum or save-image size changes. Native checkpoint, core
and persistent writers already serialize the complete CPU/GPU arena;
the persistent image checksum therefore includes this snapshot. An MCC
footer adds version, canonical map identity, cache checksum and payload
CRC32. Before accepting the image, the adapter checks the payload and
remaps incoming object-header pointers to verify every salted unit record.
Unload restores the original unused bytes, including nonzero contents.
The version-2 payload includes gravity, navigation markers, dialogue gain
and the campaign's infinite-ammunition flag. Version-1 inventory snapshots
remain readable, with the new campaign state initialized to its baseline.
The footer remains at its original address. Images are validated before
restoring either state.

Campaign snapshots use the existing OpenCE host/client transport, authority
checks and stale-tick filtering. They are sent only in MCC games; the native
inventory and co-op message layouts are unchanged. Protocol version 27 is
required on every peer because this adds message type 84. Version 26 was
already used by the separately prepared OpenCE contribution branch.
This is an additional message in OpenCE's protocol, not an MCC networking
stack or compatibility with the retail MCC client. Client cheat restrictions
remain in force; only the host decides ammunition consumption.

This is an independently written implementation of documented wire formats,
informed by reading the existing engine's public interfaces and schemas.
No Custom Edition implementation was copied or renamed. Generic engine
operations, including vertex compression and GPU buffer construction, are
shared. MCC compiles a private namespaced instance of the unchanged generic
`stb_vorbis` library to decode Ogg; Xbox/CE retain their existing instance. The
MIT-licensed `port/third_party/bcdec` dependency decodes BC7 and DXT blocks;
its accompanying license and provenance are retained. This statement does
not claim the legal two-team meaning of formal clean-room reverse
engineering.

## Wire format and conversions

The reader admits little-endian `head`/`foot` caches with version 13 and
singleplayer or multiplayer scenario type. Tag memory uses the base
inferred from the tag-array pointer minus the 40-byte MCC tag header.
Each index entry is 32 bytes. Linked pointers are checked against the
loaded data before access. File and multiplication ranges use subtraction
or wider arithmetic to avoid wrapping.

Models use the MCC `mod2` part layout and uncompressed 68-byte vertices.
The adapter creates new 104-byte Xbox part descriptors, compresses vertices
to 32 bytes and uploads the index strips. Local node indices are mapped to
global nodes when possible; models requiring more engine palette slots
use an MCC-owned palette per part. No CE palette storage is consulted.

Version-13 BSP vertex data is external to the BSP tag allocation but can be
embedded in the same map file. The adapter follows the BSP header's vertex
byte count and file offset, reads the material's 56-byte environment and
20-byte lightmap streams, and compresses them into adjacent 32-byte and
8-byte streams. The converted streams have a separate tracked lifetime;
BSP changes release only the previous BSP's buffers.

Bitmap conversion supports the common Halo pixel formats, including P8,
DXT1/3/5 and MCC's BC7 format 18. BC7 is decoded to ARGB8888 for the existing
renderer. Pixel rows, Morton ordering and cube/mip layout are packed by
the independent MCC adapter. The MCC environment bitmap flag is metadata, so it is cleared
when constructing the Xbox descriptor. Compressed and palette flags are
derived from the resulting format. Model flag `0x40` and HUD meter flag
`0x20` identify resources already in Xbox channel order. Those channels
are preserved; references with the flags clear receive Gearbox-to-Xbox
channel conversion. A bitmap used by consumers
that require different channel layouts gets separate MCC-owned tag and
pixel copies, with the relevant consumer's reference redirected. The tag
index can grow into a separate checked allocation for those copies.

Unflagged (Gearbox-order) MCC meters also omit the Xbox RGB opacity tint.
Their converted clone stores fill order in red/blue and authored shape coverage
in green/alpha. The MCC texture bridge selects the renderer's existing filtered
coverage and discrete-threshold interfaces for that clone only. This preserves
the scene beneath partially covered edge pixels, filters curved fills like
their adjacent outlines, and avoids interpolating ammunition segment IDs.
The original bitmap, explicitly Xbox-order MCC meters, and Xbox/CE texture
paths keep their existing sampling. Descent provided concrete examples:
`decent\ui\hud\odst_eva` uses opacity 1 for its unflagged shield meter;
its shield and health shapes occupy different curves within 512-by-64 images.
Those authored shapes and the space between them are retained, not enlarged
to fill their surrounding outline.

Unflagged MCC HUD placements use a 960p canvas, converted to native 480p
coordinates in MCC-owned tags. Legacy high-resolution placements (flag 4)
already carry 480p anchor offsets and retain them. Bitmap size conversion
is independent of that offset flag; the flag is consumed before native drawing. Bitmap half-scale flags are a separate factor, so high
resolution images retain their original texels. Anchors, number advances,
waypoint/damage margins and direct icon geometry are normalized; the
motion-sensor radius and font-relative icon offsets/advances already use
native units and are retained. Five native
HUD/UI files contain additive dispatch only: every pre-existing statement
is retained. Xbox/CE dispatch is neutral before any bitmap lookup or draw.
The pause A/B glyphs and pickup icons use their own bitmap flags, with
matching cursor advances so text remains beside the correctly sized icons.
Pause glyphs align to the active UI font's capitals instead of inheriting
a map's custom HUD icon baseline.

Gameplay A/B/X/Y prompts use OpenCE's embedded hand-drawn button artwork,
with visible bounds centered on the active font's capitals. Controller entries
are identified by their HUD-global entry, not their sprite sequence, so custom
weapon/pickup pictures remain map-authored. MCC owns and releases these bitmap
placeholders on unload; the pause-menu path is unchanged.

Weapon HUD static elements, meters, numbers and overlays also honor MCC's
per-element child anchors (0 inherits, 1..9 select an absolute anchor).
The field occupies the native header's padding at byte 6, as documented by
[Invader's weapon HUD format](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/weapon_hud_interface.json).
Each draw resolves a temporary placement without changing the parent tag.
This places A50_Revisited's visor at the screen edges; invalid child anchors
retain the parent placement. Xbox/CE returns the original parent untouched.

HUD normalization first plans ownership from pristine loaded bytes in
`mcc_hud_ownership.inl`. Same-group aliases of a root are converted once.
Shared mutable blocks receive separate consumer storage before any placement
or pointer is written. Pointer-free crosshair and overlay items are always
snapshotted: an MCC cache can intern them within storage used outside the HUD
tree. This prevents one consumer's scaling or frame marker from changing the
authored input seen by another. It also makes supported scalar interning
independent of the later validator's traversal order. This is a correction
within the MCC compatibility engine, not a map-specific exception.

Pointer-bearing aliases retain their largest original extent so the later
validator still rejects conflicting owners, cross-schema roots and partial
overlaps. Planning is bounded by the validator's four-million-visit budget.
All copies precede pointer publication; allocation failure refuses the load.
Planning records are freed on every exit, and copied blocks belong to the
runtime tag arena, released by normal unload or failed-load cleanup. Neither
original map files nor Xbox/CE conversion paths are changed.

Stock corner HUDs use the native safe frame instead of the wider MCC frame.
`mcc_hud_layout.inl` recognizes component layouts before conversion from their
anchors, placements, number layout and overlay counts. Unit HUD recognition
also includes bitmap dimensions/registration points and sprite geometry.
Bitmap names remain bounded and validated but do not select the frame.
Weapon and grenade counter panels deliberately exclude artwork identity and
sprite choices: a custom replacement bitmap does not relocate a stock counter.
Grenade icons retain their own authored size and offset within the counter's
frame. Bitmap and overlay references are still bounds-checked. Handles,
padding, colors and independently centered crosshairs do not select the frame.
The fingerprints were measured from locally installed MCC version-13 a10,
b30, b40, d40, bloodgulch, deathisland and carousel caches. No map assets are
embedded. Unknown layouts retain the authored MCC frame; this is deliberately
conservative and is not a security or content-identity check.

These structural profiles remain a compatibility exception: the supported
anchor and scale fields do not establish whether an author intended the native
or wider safe margin. Removing the profiles requires evidence of a reliable
format or usage rule that preserves both stock and custom layouts, with
near-miss coverage. The Stage 5 inventory found the same classification for all
487 component HUDs in ten installed Workshop maps after removing bitmap-name
dependence; changing only names also preserved classification. This bounded
result does not establish that the profiles recognize every stock derivative.
One load diagnostic reports native/authored frame counts and ownership copies;
no diagnostic runs per draw.

Recognition is read-only and runs once on each distinct pristine root.
Conversion then writes the chosen marker into runtime-owned MCC placement
padding, replacing map-supplied values. A weapon and its inherited parent HUDs share one
frame: `mcc_hud_weapon_frame_begin` follows the outermost populated parent
layout in the bounded chain, ignoring empty/crosshair-only parents. A chain
with distinct authored anchors retains the MCC frame. This lets a custom ammo
meter inherit its stock parent counter's margin without moving shared tags or
classifying the whole map as stock. `mcc_hud_weapon_frame_end` clears the choice
after that draw. The three-line additive hook in `hud_weapon.c` is needed
because a parent tag can be shared by both custom and stock weapon HUDs;
conversion alone cannot know which child is currently drawing it. No shared
parent tag is mutated during rendering. Xbox/CE dispatch remains neutral.

Stock top-left messages (zero horizontal offset, 60 native pixels down) use
the same native safe margin. Other message offsets and anchors retain their
authored MCC placement. With High-Res HUD enabled, MCC controller glyphs use
the native HUD's dimensions, capital-center alignment and text advance, with
the selected button style and split-screen scaling. Low-resolution glyphs
retain their existing geometry. Mercury Falling's pistol, ability counters
and vehicle prompts exposed these separate cases; the rules do not depend
on map or weapon names. Descent's custom layouts remain separate.

The asset-free ownership and normalization cases in
`tools/test_mcc_hud_normalization.py` exercise the whole converter, including
shared roots/blocks, order independence, separate frames and allocation failure.
`tools/test_mcc_tag_validation.py` combines conversion with the actual validator
to check supported scalar interning and malformed pointer-bearing overlaps.
The layout, channel and texture-ownership tests are in
`tools/test_mcc_hud_layout.py`, `tools/test_mcc_shader_channels.py` and
`tools/test_mcc_texture_bridge.py`. `tools/test_mcc_hud_meter_render.py` renders
synthetic converted meters through the production GPU shader, checking full,
empty and partial fill with fractional coverage and placement. Visual checks
must also cover real stock/custom MCC HUDs at native resolution; synthetic
tests alone cannot establish that every authored layout matches MCC.

MCC Chicago shaders that explicitly request Custom Edition blending (extra
flag `0x4`) use black-neutral modulation for double multiply. Frogfoot's
`waterb` layer sets this flag; ordinary double multiply turns its dark pixels
into black water. The MCC adapter uses `destination * (1 + source)` and fades
the source toward black, preserving the background at zero contribution.
This is an independently implemented interpretation of the black-transparent
behaviour documented in [the Chicago format reference](https://c20.reclaimers.net/h1/tags/shader/shader_transparent_chicago/),
validated against Frogfoot's authored layer and local visual comparisons;
exact pixel parity with native MCC has not been established. Other blend
modes and unflagged shaders retain their original behaviour. The adapter
runs after native Chicago state setup because tag conversion alone cannot
express this blend and its matching fade. Each subsequent shader installs
its normal blend state; the adapter retains no per-map state.

MCC `player_add_equipment` calls use the MCC script registry and
`mcc_player_add_equipment`. A reset deletes the held weapon through the
normal object-deletion service before applying the native starting profile.
The inherited Xbox inventory clear keeps the held weapon;
Meincrap 2's campaign selector readies a dummy weapon, so keeping it caused
repeated profile changes with a blank viewmodel and a stale hotbar selection.
The local version-13 `meincrap2.map` contains `weaps` and `set_weap_and_hp`:
the former detects the dummy or flashlight state, and the latter applies a
reset profile twice across a one-tick sleep, restoring vitality each time.
Each profile supplies a real weapon and another dummy. This inspected script
flow, and a reproduced ordinary switch-button failure, motivate the MCC-only
reset semantics; the Xbox/CE clear routine is unchanged.

The adapter validates the player unit and profile before deleting anything,
leaves add-only calls intact, and runs only on the authoritative machine.
It targets exactly the unit supplied by the script; MCC's per-player selectors
must not inherit the legacy co-op follower expansion. It retains no sidecar
state and uses existing object/inventory replication and deletion cleanup.
MCC admission version 61 separates the changed equipment behavior from older
MCC peers; the OpenCE wire format and Xbox/CE admission version stay unchanged.

MCC equipment scripts may delete the currently displayed weapon before
creating its replacement. A deletion notification clears matching local
first-person references while the old object still exists; unrelated weapons
and Xbox/CE maps keep the original path. This notification must run at the
engine's weapon-deletion boundary, which the script adapter alone cannot
cover (including replicated deletions). MCC also admits positive authored
item scales from `0.001` up to the native `0.5` lower bound, as used by
MeinCrap's block weapons. The item update already applies these scales;
only its validation gains the guarded MCC allowance. The native upper limit,
zero/invalid rejection and all Xbox/CE limits remain unchanged.

MCC textures bind their immutable converted allocations directly through an
MCC-owned hardware-header registry. Exact registered header identity grants
access to the pixels; a copied descriptor or arbitrary physical-address word
does not. GPU textures upload lazily and remain owned until MCC unload,
which unbinds and deletes them before freeing their source pixels. This
avoids another pixel copy and allocation in the Xbox/CE texture staging
cache, whose capacity and policy remain unchanged. BC7 decoding retains
the full decoded texels; there is no additional lossy recompression or
resolution reduction. Host memory and GPU texture capacity still bound
the maps the machine can render. The same owned pixels remain available
to CPU object-lighting samples. Two-dimensional DXT resources preserve
their complete 2-by-2 and 1-by-1 source mip blocks for those samples, while
the GPU descriptor retains its native mip limit. Unload clears only CPU
base pointers that still name the MCC-owned allocation.

The audio adapter accepts embedded or externally indexed PCM16, Xbox ADPCM and Ogg Vorbis
permutations. It uses independent bounds checks and codec state. Mono
output uses the game's 22,050 Hz format; stereo output follows the admitted
22,050/44,100 Hz tag rate. Necessary conversions produce an MCC-only ADPCM
stream. Unsupported or damaged audio rejects the map instead of silently
removing sound.
External ADPCM that already matches the native sample rate is validated
and retained byte-for-byte. The independent resource reader checks the
16-byte resource header, bounded names/index tables, resource type and
payload ranges. Sound names identify the tag, pitch range and permutation;
bitmap names identify the tag and image. Indexed whole-tag replacements
are still unsupported.

Compiled script function and engine-global indices are linked by the
names retained in the script strings. The adapter provides
`objects_distance_to_object` and `mcc_mission_segment`, and maps
`player_effect_set_max_vibrate` to the engine's rumble operation.
Other unresolved names reject the map. `mcc_mission_segment` evaluates its
string argument normally, records a bounded local diagnostic event and
returns true for a supplied string. This acknowledges the event locally;
it does not implement the MCC telemetry service. The boolean result retains
the conditional `sleep` calls used in shipped and Ruby campaign scripts.
It does not call `core_save_name`: that Xbox function writes a raw debug
save, and remains unchanged and unavailable to map scripts.

An MCC campaign mission's scripts may call `map_name`, which H1A custom
campaigns use to end a mission and load the next: `(map_name "x05_captured")`
names a mission of the same package (`mcc_maps\<package>\x05_captured`),
whose script ends the level as `game_won` does, with that mission next, with
or without a campaign manifest (`mcc_main_campaign_won`). Alone it loads; a
network co-op host takes every machine there; a dedicated server plays its
rotation's next entry, as after any won level. A name that is no campaign
mission of the package does nothing (logged), and no map's script may load
any other map. Before, the refused call disabled its whole script: the
missions' endings (their last cutscene and level change) never ran.
Global initializers may not change maps, even through a static script; the
call is logged and ignored until made by a script thread or the console.
`mcc_main_script_level_resolve` owns the same-package destination lookup;
the shared `main_script_level_resolve` delegates only MCC level names to it.

MCC scenarios may call the zero-argument, void `ai_deselect` operation through
the MCC registry in [`mcc_scripts.c`](../port/linux/game/mcc_scripts.c).
Its evaluator calls the existing `ai_scripting_deselect` service in
[`ai_script.c`](../source/ai/ai_script.c). When AI is initialized for the map,
that service clears the selected encounter and actor through the native debug
selection routines, retaining their evaluation-record and idle-look cleanup.
The adapter adds no persistent state or checkpoint fields. Registry lookup
requires loaded MCC tags, so unloading removes access through the existing
lifecycle. Xbox/CE keep their original map-function allowlist; `ai_select`
and other debug operations receive no new permission. Client console authority
checks also remain unchanged.

The installed Quarantine Workshop map has one valid `(ai_deselect)` call in
its continuous `life_lost_0` script. The previous native map-policy refusal
disabled that entire script, including its cooperative game-over sequence.
The fix admits the operation for every accepted MCC map, without selecting a
map name or replacing it with a no-op. Enabling previously refused authored
gameplay originally raised MCC to protocol 65, while that change left message
layouts and Xbox/CE's then-current OpenCE 41 contract unchanged. Current
agreements are listed in [NETCODE.md](../port/linux/NETCODE.md#opence-network-agreement). Source-derived tests exercise native selection
clearing, inactive-AI behavior, native map refusal and client console policy;
the MCC script harness covers linking, malformed arguments and unload guards.
These checks establish the operation and its boundaries, not a complete
Quarantine playthrough.

Additional MCC campaign operations include `local_players`,
`game_is_authoritative`, `objects_distance_to_position`, `sleep_forever`
(current or named script), gravity setting/reset and breadcrumb navigation
at positions, flags or objects. Local players excludes remote and dead
units. Sleep uses the existing interpreter stack and native wake semantics.
Navigation has eight independently owned team markers and uses the native
HUD drawing services; the host replicates the complete marker state and
gravity periodically, including after changes and late joins.

The recognized `debug_ice_cream_flavor_status_*` globals for IWHBYD,
Grunt Birthday Party and Bandanna report false. This does not implement
skull gameplay. A literal assignment of false is accepted because the
skull is already disabled; attempts to enable one or assign a computed
value are refused. MCC maps may
set `sound_gain_under_dialog` and `cheat_infinite_ammo`; the adapter captures
their previous values and restores them on unload. The native global table
and Xbox/CE write permissions remain unchanged.
Compiled MCC static/stub scripts support up to 16 typed parameters.
Signatures, scoped local references and call arguments are checked before
loading. The MCC interpreter extension keeps arguments, mutable local
values and the evaluation cursor in the existing thread stack; object-list
references survive sleeps and release on return or thread termination.
Nested and recursive calls retain separate scopes. After an MCC `if` selects
a branch of the same result type, its forwarding frame can be released before
evaluating that branch. This prevents MeinCrap 2's nested equipment selectors
from exhausting the stack, while retaining script parameter scopes and all
live state in the checkpointed buffer. Branches requiring a cast use the native
evaluation path. MCC uses 4,096-byte bounded stacks in its own 256-thread array;
Xbox/Custom Edition retain their existing 512-byte stacks and thread layouts.
`mcc_script_threads.inl` selects this array only for accepted MCC maps. Halo
Kart's `main` exhausted 512 bytes both while selecting NPC steering commands
and while awarding an item, stopping both features. The larger bound supports
these nested calls without rewriting the map's scripts or removing overflow
checks. It is a compatibility capacity, not a claim about MCC's own limit.

The array occupies approximately 1 MiB below the existing MCC checkpoint
slots, only after the CPU allocator's high-water mark proves the region unused.
The native allocation records and checksum remain unchanged. The small arena
bounds accessor and interpreter hooks are necessary because those private
engine state and stack bounds have no existing external interface. All storage
ownership, validation and migration live in MCC modules. Unload, failed load
and later Xbox/CE loads restore the original array and unused bytes.

Complete saves, including the host's co-op revert image, include the array.
Before accepting it, the MCC checkpoint path checks its version, checksum, array metadata, thread
bounds, frame chain and result destinations. Supported old checkpoints with
native inline stacks are checked and migrated with their intra-thread pointers
relocated. MCC protocol 57 requires matching script-stack capacity on peers;
the verified OpenCE protocol is unchanged. Script initialization can run while
loading on either peer, but only the host runs the race scripts and AI during
co-op. Existing object, actor and scenery-animation replication carries their
effects; thread stacks are not a new network message. This adds no client
command or permission to execute scripts on the host. Malformed or insufficient
storage is refused rather than truncated.

Halo Kart C34 also exposed passenger AI entering the native danger-avoidance
action, whose firing-position selector asserts that the actor is not a
passenger. `mcc_seat_blocks_avoidance` prevents that action for MCC passengers
and completes it if scripted seating happens after entry. Two additive hooks
in `action_avoid.c` are needed at those action boundaries; they leave the
existing Xbox/CE path and movable MCC actors unchanged. The diagnostic run
identified action 13 (avoid), evaluation mode 6 (avoid), and driver type 0
(not a driver). This guard does not suppress the selector's assertion or
rewrite the race's steering commands.

Dynamic console calls with
parameters are not supported by this extension; the cached map's calls
are the supported path. MCC syntax now retains its full 32,767-slot arena,
including unused slots for dynamic console expressions. This is the Halo 1
MCC format's signed 16-bit capacity, not 65,535. MCC's independent validator
checks the complete node span, occupied count, salted references and cycles
before the interpreter follows links. Its traversal and parameter workspaces
cover the full arena. Three additive dispatches in `hs.c` select MCC's
validation/allocation-admission path; Xbox and Custom Edition keep their
original 19,001-slot constants, validation storage and statements.
Syntax remains in MCC-owned tag memory, with unchanged datum handles and
thread/checkpoint layouts. No shared data-array structure or allocator changes.
The MCC tag walker also retains the full syntax byte span during its later
schema pass; the shared Xbox/CE schema still specifies 19,001 slots.

Ruby's Rebalanced was audited across all ten campaign maps: each reserves
32,767 slots. `a10` has a high-water count of 19,723, including 18,480 occupied
nodes; the other nine high-water counts are below 19,001. Its existing holes
are retained rather than compacting or renumbering the graph. Across these
maps, the only unsupported native name was `mcc_mission_segment` (225 calls,
42 in boolean contexts and 183 with a discarded return value).

Native Ruby loading exposed two additional MCC bitmap layouts. RGB565
lightmaps with flags `0x1281` are normalized only when their owning group is
a lightmap, they have no mipmaps, and the pixel range is complete and tightly
packed. Bit 12's broader meaning is not inferred. The existing descriptor
verifier still checks the normalized result. Four maps also contain a Wraith
HUD DXT1 texture whose tiny mip tail is sized using unrounded pixel counts.
For that exact full-chain size pattern, the converter retains every complete
mip level and logs the omitted tail; it never reads beyond the declared range.
Arbitrary truncation, missing base images and other unknown flags still fail.

Ruby's `sound\\music\\spooky1\\in` (including `b30` tag #3583) uses an
empty residue vector codebook. The generic decoder aborted the rest of the
residue after encountering it, producing an error and incorrect PCM. MCC's
private decoder normalizes only empty additive residue vector references
before the first overlap packet, matching Xiph's zero-contribution behavior.
Empty scalar floor/classification books are not covered by that rule.
Ogg checksums, page continuity, EOS, sample count and remaining decoder errors
are still checked. Decoder buffers return to their own allocator, independently
of the game's debug allocator and the later ADPCM format conversion.

An independent PCM comparison across all ten Ruby maps covered 50,186 Vorbis
references and 8,325 distinct streams: all decoded successfully and every
sample differed from libsndfile/libvorbis by at most one signed-16-bit unit.
These codec checks do not establish a complete campaign playthrough.

Solar Flare and Spasm Playground also use valid final Vorbis granule trims
across long/short window transitions. MCC decoding retains the requested
PCM frames while validating the remaining packets and a bounded overlap
tail. Page checksums, sequence, monotonic granules and decoder errors are
still checked. A comparison of 247 permutations from the two reported
sound tags matched libsndfile/libvorbis within one signed-16-bit unit.

Combat Revolved's material-less model part retains the native renderer's
skip behavior. Its 8192-pixel credits strip uses the MCC texture route;
the legacy texture cache's size policy is unchanged. Each bitmap tag has
its own virtual stream with exact tag-handle checks, allowing the total
decoded texture storage to exceed the former single 512 MiB offset range.

## Current boundaries

- Modern uncompressed version-13 maps are the target. Earlier Anniversary
  chunk-compressed caches, Halo 2 or later MCC maps and external indexed tags
  are not supported. Indexed MCC bitmap/sound payload resources are supported
  when matching files are installed as described above.
- The portable inspector accepts files up to `INT32_MAX`; runtime raw files
  must be smaller than `0x50000000` bytes because higher offsets identify
  MCC virtual resources. Runtime tag windows must be free, 64 KiB-aligned,
  64 MiB allocations based between `0x42000000` and `0x70000000` inclusive.
  The observed MCC base is `0x50000000`.
  Windows delay-loads `SDL3.dll` and reserves this address range at constructor
  priority 101, before CE's settings read (priority 102) first calls SDL.
  SDL retains ASLR; it is not patched to use a fixed address. Windows
  retains the reservation between maps. Pages are committed only during an
  MCC load and decommitted on unload; other accepted linked bases can replace
  only our inactive reservation. An occupied range is never overwritten.
  Allocation refusals report the address, size and OS error in `debug.txt`.
  Early reservation failures are retained until `windows_startup` opens the
  port log, then reported to stderr (normally `halo.log` for release builds).
  The map loader still retries the reservation and refuses occupied memory.
  MCC network load failures schedule session teardown and a menu reload before
  the next frame's input update, rather than entering the fatal error screen.
  A player reported that disabling XFire and restarting the game resolved this
  allocation refusal. This supports interference from third-party software,
  but the specific allocation conflict has not been captured. Early reservation
  is not a guarantee of compatibility with XFire; disabling it and restarting
  remains the confirmed workaround for that report.
  Separately, [issue 87's follow-up](https://github.com/chestahh/GulchCE/issues/87#issuecomment-6486929272)
  records two launches with the unmodified SDL DLL inside the MCC window and
  a successful launch with SDL's ASLR disabled locally. Those post-startup
  snapshots do not establish which module first occupied the range. They do
  demonstrate why a normal load-time SDL import can defeat any constructor.
  Delay loading removes that SDL race; other eager or injected DLLs can still
  prevent reservation and must not be overwritten.
- There are at most 65,535 tags and 32 BSP references. Runtime conversion
  descriptors must fit below the lowest BSP in the tag window. Geometry
  currently admits at most 64 nodes per model, 22 palette nodes per part,
  256 geometries per model and 128 parts per geometry. Each part's vertex
  and triangle counts must be nonzero and at most 65,535.
- Non-volume bitmap dimensions are limited to 8,192 by 8,192 with depth one;
  volumes are limited to 512 by 512 by 256. There are at most 13
  mip levels after the base level. Individual raw/decoded allocations are
  capped at 128 MiB; aggregate normalized bitmap storage is capped at 1 GiB,
  with each tag's virtual stream smaller than 512 MiB. Actual allocation and
  GPU texture-size support remain required, especially on mobile devices.
  Linear textures must be two-dimensional, unmipped and uncompressed.
  Audio permutations are capped at 64 MiB of input and 16,777,216 decoded
  frames, with at most 256 MiB of converted audio.
- Reusing stock schemas and gameplay/render systems does not implement
  every MCC engine extension. MCC-only object behaviors, new shader
  semantics and UI callbacks require separate
  semantic work and representative maps. Embedded MCC widget event bytes
  are preserved: compatible callback IDs use the existing engine functions,
  while identified MCC-specific actions dispatch through the MCC adapter.
  This remains relevant to script-opened custom interfaces; the default
  pause menus use the separately generated definitions above. Unsupported
  embedded callbacks fail without performing the event's close/open actions.
  Native Settings is available in all MCC modes with one local player.
  HUD placement normalization and overlay handling
  still require visual comparison against the map's intended appearance.
- No claim is made here of complete campaign/gameplay fidelity or
  synchronization under all conditions, performance parity, or tested
  Android/Linux runtime behavior. See the bounded test status below.

## Reference fixture

The supplied `MercuryRising/halo1/maps/a10.map` was inspected without adding
its proprietary contents to the repository. Its observed inventory is:

| Field | Observed value |
| --- | --- |
| File size | 407,040,369 bytes |
| Internal name / build | `mercury_falling` / `01.06.66.HELL` |
| Version / scenario type / flags | 13 / 0 (singleplayer) / `0x0007` |
| Tag base / bytes / count | `0x50000000` / `0x012D661C` / 4,575 |
| Models / model parts | 153 / 842 |
| BSPs / BSP materials | 1 / 263 |
| Bitmap records / BC7 records | 1,122 / 38 |
| Sound permutations | 6,629, all embedded |
| Scripts / globals / parameterized scripts | 54 / 12 / 0 |
| Reserved syntax slots / active high-water count | 32,767 / 2,697 |

Deferred team-change deaths have their own records keyed by full player and
unit handles. Only the matching no-statistics death receives neutral kill
attribution before network replication; this preserves native respawn and
objective cleanup without a Slayer suicide penalty. Tests include ordinary
deaths, recycled handles, client authority and unloading the map.

The reported red/green Mercury cliff bands match the native allocation
failure texture: `DEFAULT_BITMAP_PIXEL0/1` are transparent red and opaque
green. The captured `stabbed.txt` identifies the 2K cliff normal map as a
failed 22,369,664-byte allocation. Its source is a 5,592,432-byte BC7 mip
chain, expanded to ARGB for the native renderer. About 189 MiB of the
256 MiB staging cache was locked in that frame; its largest available
contiguous span was only about 11 MiB. Repeated retries explain the
associated stalls. The direct MCC binding above removes this staging
constraint. The reported black cliff faces are consistent with a failed
bump/lightmap sample, but the screenshots alone do not prove their cause.
A bounded native comparison rendered twelve camera angles at the captured
failure location. Both builds showed normal cliff textures in that fresh
run: the intermittent baseline failure was not reproduced, so those frames
are a regression check rather than a before/after reproduction. The fixed
Debug renderer also completed Mercury -> Xbox UI -> Xbox A10 -> Nitra ->
Mercury -> Xbox UI in one process without assertions, texture allocation
failures or stale-resource errors. The later generated-pause interaction
checks are described above; broader multiplayer and cooperative playtesting
remains necessary.

An independent script audit compared 748 live function calls, using 103
distinct function names, against the Xbox definitions. It found no
parameter-type drift for functions present in both. The only missing
Xbox names were the four vibrate-alias calls and six
`objects_distance_to_object` calls addressed above. Eight nonuniform
uncompressed cubemaps in the fixture confirmed mip-major storage (all six
faces within each mip): lower mip pixels matched upper mip averages to
less than 0.26/255 mean RGB error. Face-major interpretation produced
37â80/255 error. BC7 follows the same layout in the adapter; that particular
layout comparison did not independently test BC7.

The second supplied fixture, `Nitra v2/dangercanyon.map`, also passes the
parser and geometry harness. It is a 102,888,276-byte multiplayer cache,
internally `nitra_v2` with build `01.03.43.0000`. It has 2,353 tags occupying
5,911,224 bytes at `0x50000000`, 55 models, one BSP with 139 materials, 731
bitmap records (no BC7), and 992 embedded sound permutations. Its 11 scripts
and four globals have no parameters. There are 244 high-water syntax slots,
173 live nodes and 56 function calls using 21 distinct names. The only name
missing from Xbox is the vibrate alias (four calls); no parameter-type
drift or missing engine globals were found. Geometry produces 1,112 buffers
and releases all of them.

Live Mercury loading now reaches the rendered world and runs its scripts
after passing media, tag and BSP validation. The validator corrects one
genuinely empty one-frame animation in
`vehicles\falcon_destroyed\left_wing\one frame empty`: both default-pose and
frame payloads are absent, so its node count becomes zero. This is not a
conversion that discards a present animation payload. A two-process Nitra
test ran 1,020 ticks and accepted ten remote hits with no rejected hits.
That run exposed a NULL-free assertion during geometry teardown; all
nullable geometry cleanup paths were guarded, and the harness now rejects
NULL frees like the native debug allocator. The corrected build subsequently
completed an MCC-to-Xbox-to-CE-to-MCC transition in one process without
rejection or assertions. Mercury save, revert and core reload accepted a
snapshot containing four nonzero extra grenade inventories. A two-process
Mercury cooperative run lasted about 1,320 ticks, ran opening scripts and
cinematics, and spawned both players alive at matching positions. Its host
core contained eight extra grenade records; the client's save request was
refused as host-authoritative. A final cooperative run lasted about 1,350
ticks and found the same eight nonzero type-2/type-3 inventories on host
and client, including exact salted handles, at 40, 50 and 60 seconds.
Both processes shut down cleanly and the client returned to the Xbox menu.

### Solar Flare, Spasm and Combat Revolved compatibility

B40's first failing expression called `core_save_name` as a boolean.
The legacy Xbox function with that name returns void and writes a debug
save. H1A repurposed it for campaign segments, as documented in the
[public scripting reference](https://c20.reclaimers.net/h1/scripting/#functions).
The MCC linker now gives this spelling its own boolean function definition
and the existing MCC segment evaluator. It logs the evaluated segment and
acknowledges it without writing a core file or contacting MCC telemetry.
The native Xbox/CE function table, compiler, validator and save functions
are unchanged. MCC console lookups use the same segment meaning; use the
existing `core_save`/`core_load` commands for an explicit debug core instead.

Solar Flare M1 exposed a later runtime failure that a startup check cannot
cover. Its five pilot/dropship recordings contain `0xFF00` in their initial
weapon selection field. The Xbox playback code reads this as the signed
short `-256`, outside the four-slot unit inventory. Triggering `r_a_ds1` on
`a_pilot` in the published Windows release build 32 reproduced the reported
`units.c` assertions at original lines 1512 and 8371. The release's matching
symbols place the subsequent invalid object lookup in `unit_can_use_weapon`.
For a targeted reproduction in M1, create the pilot with
`(object_create_anew a_pilot)` and start `(recording_play a_pilot r_a_ds1)`
through the console. This exercises the control stream independently of the
mission trigger volumes.

`mcc_recordings.c` validates each recording stream before normalizing invalid
initial or event weapon selections to `NONE` (retain the unit's current
weapon). Valid slots 0 through 3 and the existing `NONE` sentinel are unchanged,
as are event timing, movement, flags and the remaining control fields. The
conversion writes only to the MCC loader's private tag buffer through
`mcc_tags.c`; it changes neither the Xbox playback interpreter nor Xbox/CE
loaders, unit code, inventory limits or network messages. Unsupported or
truncated streams are refused with a recording-specific diagnostic;
explicitly disabled empty recordings remain disabled.

Generated regression fixtures cover both playback codecs, all supported
control versions, selection events, truncation at every byte, unknown formats,
idempotence and checked scenario pointers/counts. A private audit of 377
recordings across the 17 supplied MCC maps found no parsing failures. It
normalized 23 invalid selections: five in each Solar Flare map, one in Combat
Revolved's credits and two in its D40. No map contents are included in tests.
The native startup checker now also treats `EXCEPTION assert` as a failure,
even if the process subsequently exits normally.

Run the isolated tests with a C compiler and Python/pytest available:

```text
python -m pytest -q tools/test_mcc_cache_format.py tools/test_mcc_geometry.py
python -m pytest -q tools/test_mcc_tag_validation.py tools/test_mcc_checkpoint.py
python -m pytest -q tools/test_mcc_share.py tools/test_mcc_share_metadata.py tools/test_mcc_share_rules.py
python -m pytest -q tools/test_mcc_share_stage.py tools/test_mcc_share_storage.py
python -m pytest -q tools/harness -k mcc
```

Set `MCC_TEST_MAP` to an existing map path to opt into the real-fixture
checks. The parser tests are portable; the geometry harness currently
requires the x86 Windows game headers/toolchain and skips elsewhere.
`port/tools/mcc_cache_report.c` builds with `mcc_cache_format.c` and only
the C standard library, so files can also be audited outside the game.
None of these commands downloads or bundles a game map.

## Additional script and HUD compatibility

The MCC function registry also supports `print_if`, `bitwise_and`,
`bitwise_or`, `bitwise_left_shift`, `bitwise_right_shift`, `bit_test`,
`bit_toggle`, `objects_distance_to_flag` and `sound_impulse_predict`.
The integer operators use defined unsigned 32-bit operations; right shift
fills with zero. Negative shift counts produce zero, while nonnegative
counts use the low five bits. Bit-test/toggle use a bit position, and
toggle's third argument selects set or clear. The prediction function
requests all sound permutations from the existing sound cache without
playing them or consuming random numbers. Its boolean selects a blocking
request. Public signatures and the supplied compiled scripts were checked;
this is not an exhaustive comparison with every MCC runtime edge case.

When referenced, `developer_mode` is linked to an MCC scenario-owned short
global, initially zero. Script reads, writes and native checkpoint storage
therefore work without adding a global to the Xbox interpreter or enabling
engine developer privileges. Loading a different scenario discards it.

MCC HUD anchors 5 through 8 (top, bottom, left and right center) retain
their authored values. MCC-only drawing handles their position, bitmap
bounds and number alignment using the current player's window. The native
anchor enum and its five existing cases are unchanged. Combat Revolved's
unit HUD uses anchor 5; previously the native schema clamped it to top left.

Oversized MCC terminal fonts are scaled at draw time to a 15-pixel line at
the engine's 480p reference resolution. The supplied Combat Revolved font
has a 33-pixel line. This affects only the console input/output, including
wrapping and line spacing; it does not resize HUD text or modify font tags.
Xbox/CE paths and MCC fonts at or below the nominal size remain unchanged.

### Extended MCC object names (11 October 2026)

MCC maps may now contain up to 32,767 named objects. Maps with 513 or more
names use `mcc_object_names.c`; maps with at most 512 names retain the native
table. Xbox and Custom Edition keep their existing limits, allocations and
validation. This extends script names without increasing live-object
capacity. The scenery palette is extended separately below. Counts above
32,767 are refused explicitly; script targets are never dropped to fit.

The separate table follows creation, naming, deletion, restart and map
unload. An MCC checkpoint stores the table in a checked unused part of the
native save image, alongside the existing MCC state. Its length, checksum
and object handles are validated before restoration. Checkpoints for smaller
MCC maps retain their existing format. A save without enough unused memory
is refused rather than overwriting engine allocations.

Singleplayer uses the table locally. Network games use OpenCE's existing
host-authoritative replication for units and items, and MCC-only bounded
presence messages for named scenery and devices. The host sends chunks of
512 names every half second. The client requires two distinct matching
updates before creating or deleting an object; packet ranges, padding,
map name count, BSP and timestamps are checked. These messages work in
co-op and multiplayer and require network version 34 on every peer.

B30 Evolved's 640-name list is retained. Its script refusals were traced to
writes to `f0`-`f4`, `sound_enable` and `map_reset`, and are resolved:

- MCC `f0`-`f5` references use private scenario real variables initialized
  to zero. These script counters survive checkpoints without changing the
  native renderer's debug controls. Appended names preserve the console's
  separate 1 KiB text reserve.
- `sound_enable` controls an MCC map-local mute, including music, carried
  in the campaign snapshot's flag bit 3. It preserves player volume settings,
  survives checkpoints, follows the host, and clears on map disposal.
- `map_reset` uses the existing restart operation: a local reset in
  singleplayer, and the MCC action adapter's reliable round restart in a network
  game. A client cannot request it through this script function.

B30 Evolved also exposed two rendering problems, handled only for MCC:

- The CMT fluid-burst effect spans more than the native eight bitmap pages.
  `mcc_sprites.c` submits each full batch through the existing renderer and
  starts another with the remaining sprite budget. It retains the native
  eight-group structure, shader, bitmap pages and first-person projection;
  excess pages are not silently discarded.
- A signed object function can produce an intermediate RGB color below zero.
  MCC's object adapter preserves RGB extrapolation so the native object code
  can apply darkening and its existing final clamp. This avoids the native
  interpolation helper's premature assertion, without clamping the function
  itself or changing Xbox/CE interpolation. HSV interpolation is unchanged.

MCC scenery palettes also retain up to 32,767 entries. The original 100-entry
limit is an authoring/schema limit: placement and synchronization already
read the palette's dynamic tag block through checked signed-short indices.
Only the MCC validator raises it; Xbox/CE schemas and live-object capacity
remain unchanged. Oversized or out-of-bounds blocks are refused, and invalid
placement indices retain the existing validation behavior. B30 Evolved's
101st entry, referenced by scenery placement 1997/name 639, is no longer
truncated. A hidden Windows test created this entry, and a two-instance
co-op test verified its creation, deletion and recreation after a reliable
scripted restart on both peers. A checkpoint retest restored this same
name 639 and the private script counter, restarted the map and returned to
the menu without assertions. An earlier timed probe missed its expected
checkpoint values; the focused retest produced both explicit success markers.
This does not claim a complete campaign playthrough.

## Solar Flare M0 cinematic and completion fixes

The supplied M0 has a 100-degree field of view in camera `cin_x4` (index 3).
The native scripted camera and observer accept at most 90 degrees; the
observer also clamps its output to that maximum. The reported assertions
were reproduced as that camera became active. MCC metadata conversion now
limits wider, valid cinematic lenses to 90 degrees and logs the camera
index. This narrows those shots; it does not implement wider cinematic
lenses. Zero still selects the native default, and already supported
angles remain unchanged. Invalid angles (including NaN and infinity) and
out-of-bounds camera blocks are refused. Only the in-memory MCC copy is
changed; the map file and Xbox/CE camera code are untouched.

M0's `cin_a` script calls `game_won` at the end of its cinematics. In local
play, an unknown campaign level previously fell through to the Xbox credits
movie immediately after loading and precaching the menu. A separate access
violation was reproduced through menu -> M0 -> menu, even after fixing the
camera: the cache worker was copying a pending read into protected cache
memory during that movie transition. The matching Windows symbols identify
`read_at` / `ReadFileEx` / `cached_map_issue_async_request` on the failing
thread. Direct startup into M0 did not reproduce this second failure.

MCC scenario completion now queues the ordinary return to the menu, without
starting the Xbox campaign outro. Its helper lives in `mcc_main.c`, selected
by one additive dispatch in `main_roll_credits`. When MCC is inactive the
original statements execute unchanged. The existing network co-op win
branch still returns to its lobby before this path. No network message or
compatibility-version change is involved. This is an MCC-specific avoidance
of the credits/cache interaction, not a general change to the movie player
or background I/O. Xbox and CE completion remain unchanged.

At the time of that fix, automatic MCC campaign progression was unsupported.
Packages without an enabled explicit campaign manifest still require manual
mission selection. An unrelated saved-map identity rejection is
normal, and the map's equipment/grenade warnings are not suppressed here.

Startup profile cleanup can enumerate directories concurrently, so MCC
discovery owns and closes a platform directory handle of its own, without
touching the Xbox or CE catalogs.

## Additional campaign compatibility (October 2026)

The MCC function table additionally supports `list_count_not_dead`,
`script_finished`, `game_time_authoritative`, `sound_get_effects_gain`, and
`sound_set_effects_gain`. Living-object counts skip dead and stale objects.
`script_finished` checks the named script thread's completed state; dormant,
sleeping, and unknown scripts are not reported as completed. The authoritative
clock uses local/server game ticks, or the latest received host tick on clients.

Effects gain is an MCC-owned multiplier (default 1, accepted range 0â4).
It affects non-music sound classes without changing saved user volume settings.
The small additive connection in the common sound mixer returns a neutral
multiplier outside an active MCC campaign. The MCC writable-global whitelist
also permits `object_light_ambient_base`, `object_light_ambient_scale`,
`loud_dialog_hack`, and `ai_debug_ignore_player`. Original values are captured
and restored when MCC unloads. AI-ignore changes remain host-authoritative.
The existing Xbox/Custom Edition global tables and script implementations are
not extended or replaced.

These values accompany the existing MCC campaign checkpoint/network state.
The version-2 campaign snapshot is 764 bytes; version-1 752-byte checkpoints
are migrated with defaults for the added state. Network compatibility advances
from 27 to 28: **all peers must use matching versions**, including when playing
Xbox or Custom Edition maps. There is no separate MCC transport or matchmaking.

Boom, Sputnik, and Catch skull globals report disabled. Literal assignments
that keep them disabled are accepted; enabling them or computing an assignment
is refused rather than pretending their gameplay effects exist. Similarly,
`object_prediction` reports enabled and accepts a literal enable only, matching
the existing engine prediction policy. Converted constant expressions preserve
their expected result types, including a void result for a `set` statement.

Several data corrections are confined to MCC conversion:

- An animation graph's unused root sibling is cleared only after validating
  the reachable child tree and parent relationships. Orphans and child cycles
  still fail validation. This handles DP's brute graph.
- Invalid model tangent frames can be reconstructed from valid geometry and
  UV derivatives. Degenerate UVs use a deterministic perpendicular basis.
  Invalid positions, normals, UVs, or indices still fail; valid frames are
  retained. This repairs ML's pelican data without modifying the source map.
- Shared-resource lookup can resolve a relocated payload by a unique full
  resource name and exact size in the supplied cache's validated directory.
  Duplicate names, size mismatches, and invalid ranges are refused. This is
  structural validation, not a content hash: supply the correct package's
  resources. Mercury Falling needs this for a relocated footstep sample.
- Non-power-of-two 2D bitmaps retain their dimensions in a linear ARGB texture.
  Their declared source mip chain is validated, but only the base level is
  retained because the native linear texture path has no mipmaps. This handles
  Aerowalk's 1920Ã1080 HUD bitmap.

Judgement's `game_time_authoritative` conversion is supported, but the supplied
Workshop package declares `InheritSharedFiles: FromMCC` and references 687
external bitmap descriptors. It does not bundle the shared resources. The
available Aerowalk and Mercury caches do not contain all required textures;
the loader correctly refuses missing data instead of substituting unrelated
assets. A complete Judgement load requires its matching MCC shared resources.

Regression tests exercise actual MCC handlers, script conversion, snapshot
migration, malformed resources, animation graphs, geometry, textures, and the
sound-mixer connection. Native smoke tests use an isolated Windows data root,
one hidden process at a time, and a console marker after 150 simulation ticks.

## Public format evidence

These are wire-format references, not copied implementation code. Current
version-13 fields were checked against the supplied file where available;
older CEA documentation sometimes describes different compression and
memory limits and must not be applied indiscriminately.

| Fact | Primary source / observed check |
| --- | --- |
| Version 13, 40-byte PC tag header, 64 MiB tag capacity | [Invader map definitions](https://github.com/SnowyMouse/invader/blob/master/include/invader/hek/map.hpp), [engine configuration](https://github.com/SnowyMouse/invader/blob/master/src/hek/map.cpp); fixture tag pointer `0x50000028`. |
| Infer linked base from tag array; current v13 is uncompressed | [Invader map reader](https://github.com/SnowyMouse/invader/blob/master/src/map/map.cpp); header and index read directly from fixture. |
| MCC BSP external environment/lightmap vertex streams | [Invader BSP compiler](https://github.com/SnowyMouse/invader/blob/master/src/tag/parser/compile/scenario_structure_bsp.cpp), [build layout](https://github.com/SnowyMouse/invader/blob/master/src/build/build_workload.cpp), [BSP schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/scenario_structure_bsp.json); fixture header has `0x2B5D0C` bytes at file offset `0x800`. |
| BC7 format 18, environment flag bit 9 and external resource flag bit 8 | [Invader bitmap schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/bitmap.json); fixture contains 38 BC7 records and environment-flagged bitmaps. |
| MCC script parameters, expanded syntax capacity | [Invader scenario schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/scenario.json), [MCC editing kit change documentation](https://c20.reclaimers.net/h1/h1-ek/); fixture syntax is `56 + 32767 * 20` bytes. |
| MCC Halo 1 syntax limit is 32,767 | [Invader engine configuration](https://github.com/SnowyMouse/invader/blob/696830ff80af227e84e7237c2ef26eb2301ed110/src/hek/map.cpp#L94) uses `INT16_MAX`; all ten Ruby map headers reserve exactly this count. |
| Mission-segment boolean/string contract | [Sapien's generated function documentation](https://github.com/Sigmmma/c20/blob/master/src/data/hs_docs/h1/hs_doc_sapien.txt) and [shipped MCC c10 scripts](https://github.com/NervyDestroyer/Halo-MCC-Scripts/blob/main/H1/levels/c10/scripts/mission_c10.hsc); Ruby's 225 calls also take one string, with boolean conditions used before `sleep`. |
| Parameter wire records and local references | [Scenario schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/scenario.json) defines 36-byte records, maximum 16 and local-variable flag bit 4; [public script compiler](https://github.com/SnowyMouse/invader/blob/master/src/tag/parser/compile/scenario/pre_compile.cpp) confirms primitive/global/local flags and the parameter slot in node data. |
| Sound compression formats and cached sound data | [Invader sound compiler](https://github.com/SnowyMouse/invader/blob/master/src/tag/parser/compile/sound.cpp), existing Xbox sound definitions; fixture samples and tag fields checked directly. |
| MCC HUD canvas and independent bitmap half-scale | [Restored tagset author's format notes](https://github.com/Aerocatia/halopc-restored#hud-scale-is-still-480p); Ruby shield offsets and number advances are exactly twice their stock Xbox counterparts. |
| Xbox-order model and HUD meter flags | [Invader model shader schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/shader_model.json), [HUD meter schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/hud_interface_types.json); Ruby and Mercury contain mixed flag states. |
| Empty Vorbis residue vectors contribute zero | [Xiph Vorbis 1.3.7 codebook implementation](https://github.com/xiph/vorbis/blob/v1.3.7/lib/codebook.c), `vorbis_book_decodevs_add`, `vorbis_book_decodev_add`, `vorbis_book_decodevv_add`; synthetic multi-pass streams and independent Ruby PCM comparison. |
| Header and historical format differences | [Reclaimers map documentation](https://c20.reclaimers.net/h1/maps/), [SnowyMouse CEA format research](https://gist.github.com/SnowyMouse/39168bddd597549038a35d78aee39513); historical chunk compression is outside this implementation's scope. |

### Gameplay glyph and HUD placement follow-up

Gameplay A/B/X/Y use the embedded OpenCE redraws with compact text-fragment
spacing. The modern shoulder labels use Xbox white (LB) and black (RB),
matching `xinput_sdl.c`; the two neutral variants reuse the button redraw's
oval. Other controls and custom pickup artwork retain their existing path.
Pause menus are unchanged. All replacements are selected by controller-entry
identity, never by a shared sprite sequence number.

A50_Revisited mixes legacy high-resolution placements with unflagged visor
and number placements. Halving every anchor offset compressed the ammo
layout, moved the flashlight meter toward the centre, and overlapped the
shield panel with the visor. Preserving the flagged offsets corrects those
relationships without map-name checks or changing the Xbox/CE renderer.
MCC anchors also use a 2.5% title-safe inset, matching the supplied reference,
rather than Xbox's 7.5%; this keeps the flashlight above the bottom visor.
Centre-anchored crosshairs and the inner split-screen seam retain their
existing calculations.
The supplied [Workshop reference](https://images.steamusercontent.com/ugc/2013710564213466041/A0A23236FCCA6F4C077C15F6428B257FED2FEF8A/)
and a hidden native-renderer capture were used to compare the visor join and
ammo layout. A separate bounded capture enabled the flashlight and verified
its meter above the bottom visor. Regression cases cover split-screen seams,
zoom inset preservation, controller mappings, and neutral Xbox/CE dispatch.

## October 12 package compatibility fixes

- Bigass calls `abs_real`. The MCC adapter implements real-valued absolute
  value, including negative zero, without changing native functions.
- Warthog Rally passes short script parameters and globals where long integers
  are expected. MCC validation and evaluation now agree on signed widening.
  Invalid signatures and scopes still fail with more specific diagnostics.
- BioHalo deduplicates same-group tag roots and shares structured blocks,
  prefixes, names and data streams. The MCC validator tracks compatible
  allocations and isolates overlapping scalar, empty and selected raw-data
  records before correction. Bounds, schema, cycle, depth and traversal-budget
  checks remain enforced. Nonzero pointer-bearing partial overlaps are refused.
- Canyon and Canyon-plus contain a 15360-by-256 DXT1 effect strip. Converted
  linear pixels use an MCC descriptor with dimensions and pitch beyond the
  Xbox header's fields. Their 8192-pixel texture also exposed a too-small Morton
  address table in the MCC converter; its capacity and bounds now agree.
  The MCC-only GPU uploader checks the device's actual size limit. Hardware
  unable to upload the strip does not gain support through downsampling.
- Cursed Halo's Sophia animation graph lacks its parent hierarchy. Recovery
  requires an associated model with identical ordered node names and agreement
  between multiple associated models. The recovered tree still passes normal
  validation. This fixes the graph rejection, not all campaign features.
- MacWorld's Elite model references material 14 with only 14 entries (valid
  indices 0 through 13) on geometry 40, part 2: an 88-vertex damaged torso.
  All supplied campaign variants have the same missing entry. With explicit
  approval, MCC conversion applies the engine validator's existing material-0
  fallback and logs the tag, geometry and part. Geometry remains present; the
  intended appearance is unknown and may differ. Empty material lists and
  invalid geometry buffers are still refused.

These are MCC-owned changes. The sole shared renderer edit extends the existing
MCC upload function because the game-side converter cannot query OpenGL device
limits. Xbox/Custom Edition texture paths, script permissions and network
messages are unchanged. Validation bookkeeping uses the game allocator's
matching allocation/free pair, including cleanup when leaving an MCC map.
The regression harness now checks this allocation pairing too.

Resource investigation found two distinct MCC bitmap files. The direct
`halo1/maps/bitmaps.map` contains Mirror of Autumn's pipe texture at offset
52930688 (10936 bytes), and Warthog Rally's swamp bark at 146997993 (43704 bytes),
matching their references exactly. Those names are absent from MCC's
`halo1/maps/custom_edition/bitmaps.map`. Selecting by game mode or assuming
the two files are interchangeable would therefore be incorrect.

The existing MCC cast hook now also handles short-to-long widening for globals;
parameter-only handling was insufficient when Warthog Rally compiled its timer
expression. The guarded hook is necessary because native expression compilation
and evaluation use the shared interpreter. Xbox and Custom Edition retain the
original cast table, and the tests cover the inactive-MCC case.
At this stage Cursed Halo loaded after graph repair, but 15 scripts remained disabled because
of restricted engine cheats (vehicle spawning, jetpack, super-jump, possession,
Medusa, bottomless ammunition and omnipotence). The new diagnostics identify
individual scripts. No cheat permission or client-authority restriction is
bypassed; this fix does not certify Cursed Halo as playable.

## Shared resource selection and client HUD verification

The selected resource streams are retained through conversion. Map admission,
conversion and checkpoint identity use the same dependency selection. Required
resources contribute to the fingerprint regardless of their installation path;
unused resources and `loc.map` do not. Network version 36 prevents older peers
from using the previous fingerprint rules. Existing MCC checkpoints can become
incompatible when their resource fingerprint changes. This is a consistency
check, not authentication of a client or a map's contents.

Bigass's HUD globals omit `defaults.default_weapon_hud` (reference at 0x2C0,
handle NONE). A joining multiplayer client can briefly have a living unit but
no weapon before replication completes. The native unarmed HUD update then
tries to resolve that missing handle, causing cache assertions and repeated
parent-HUD traversal. The MCC converter supplies one owned, empty weapon HUD
only for omitted defaults, with no parent or visual elements. Authored defaults
remain unchanged and invalid nonempty references still undergo validation.
No shared HUD or networking code changes are needed. The generated tag has the
same deterministic identity on both peers and is released with the MCC cache.
Tests cover reuse, authored defaults, tag capacity and allocation failures.

## Weapon-authored cameras

MCC weapon flags bit 17 (`0x20000`) requests a third-person camera, as
documented by https://c20.reclaimers.net/h1/tags/object/item/weapon/.
Spasm Playground uses it on its Digsite weapons; its player bipeds carry
a single authored camera track. `mcc_camera` selects the native following
camera and its existing unit/default track, interpolation and collision
handling. Unflagged weapons and zoom use first person; existing seat,
cinematic and death camera handling takes precedence. This is independent
of the optional OpenCE third-person mode and has no toggle or preference.

The additive hook in `director_desired_perspective` is necessary because
both camera selection and deterministic aiming use that shared function;
there is no registered camera-selection extension. The MCC adapter checks
the active map before reading inventory, holds no state across map changes,
and leaves Xbox/Custom Edition paths unchanged. Existing host-controlled
inventory synchronization is sufficient, but network version 38 prevents
peers with the previous aiming behavior from joining.

## Large texture sets, cinematic abort and incomplete animation links

The MCC bitmap adapter permits up to 2 GiB of converted texture storage per
map. Individual allocations remain limited to 128 MiB, and each tag retains
its existing bounded virtual stream. Allocation failures refuse the map with
a diagnostic; the larger budget is not a guarantee that every platform has
enough free address space or memory. Unconverted pixels no longer require an
extra intermediate copy. The MCC bitmap-group callback checks owned converted
allocations and sequence ranges instead of applying the legacy file-backed
bitmap budget, which would otherwise empty XAM's 4096-pixel textures. Xbox and
Custom Edition retain their own validation and storage limits.

XAM A30 additionally has a graph with all links omitted and several graphs
with missing parent indices. Fully omitted links can be recovered only from
an associated model with matching ordered node names; associated models must
agree. An omitted parent can be derived from an explicit child link only while
validating a complete, uniquely reachable tree. Flood Forgotten Exile's Flood
Banshee has a separate pattern: its root names the last sibling rather than
the first child. Recovery requires matching names and every non-root link to
agree with the associated model. Conflicting links, cycles and unreachable
nodes remain rejected. Repairs are logged and confined to converted MCC tags.

The MCC-only zero-argument, void `cinematic_abort` adapter schedules OpenCE's
existing cinematic skip operation. This preserves its skippable-checkpoint
requirements and host-controlled co-op revert. A network client cannot invoke
that operation through this adapter. No new packet or client authority is
introduced. The signature is documented at
https://andrew.gg/scripts/00reference.html; the evaluator mapping to cinematic
skip is independently documented at
https://surreptitiousresearch.github.io/halocea_docs/21_script_presentation.html.
Implementation uses the existing OpenCE interface, not MCC implementation code.

All implementation changes are in MCC modules. No shared engine source,
Xbox/Custom Edition loader or network message layout was changed.
For authored biped seats and their network isolation, see [MCC seats](mcc_seats.md).

### Cursed Halo authored effects (October 13)

`mcc_script_effects.c` supports the map-authored jetpack, super-jump,
bottomless-clip, possession, Medusa, omnipotence and deathless flags. The
MCC campaign adapter also exposes vehicle spawning through the existing
engine service. This enables the 15 previously restricted Cursed Halo scripts;
it does not add Twitch/Crowd Control connectivity. Evidence comes from the
supplied CursedHalo A10's compiled scripts: its dispatcher calls timed `fx_*`
scripts, whose continuous updates clear the flags when their timers expire.
No proprietary MCC source was used.

The host runs these scripts and owns damage, spawning and possession. Clients
receive bounded effect flags in the existing MCC campaign snapshot and use
them for movement and ammunition prediction, while native client cheat globals
remain disabled. Jetpack and super-jump keep movement host-simulated from
validated inputs rather than widening acceptable client position/velocity
bounds. This can require corrections under latency; it does not authorize a
client's position, possession target or cheat request. State version 3 retains
the snapshot size and accepts older checkpoint versions with the new effects
off. Network version 40 is required because peers must agree on these effects
and on live-body player transfers.

The shared hooks are necessary because the native movement and weapon code
reads cheat globals directly, the bump handler uses a local-only possession
helper, and client player attachment previously killed the former body.
Guarded calls in `bipeds.c`, `units.c` and `weapons.c` consult MCC-owned state;
`network_distributed.c` leaves MCC former-body death to host replication and
declines client movement corrections during movement effects. Their inactive
MCC paths retain Xbox/Custom Edition behavior. Effect state is restored at
checkpoints and the pre-MCC settings are restored on unload.

The same A10 intro passes an actor-variant tag to `object_type_predict`.
The MCC adapter precaches that variant's referenced unit after group checks,
instead of interpreting actor-variant data as an object. Other non-object
references produce a diagnostic; the native function is unchanged.

Final Windows executable SHA-256:
`f2c16365d22ab7c5d84f7ffcbd400f3ec0b62e6ff9638cc2979b859e20809f8c`.

### MCC clock global and alternate environment attenuation (issue 49)

`game_speed_value` is linked to an MCC scenario-local real global, initialized
to one. Its read/write adapter uses the native host's game-time speed, which
already belongs to saved game state. Finite values from zero through 100 are
accepted, matching the native speed setter; this is a safety bound, not a
recommendation to run maps at high speed. The public
[H1 scripting reference](https://c20.reclaimers.net/h1/scripting/) documents
the MCC global and warns against speeds above 20. No reporter could identify
the original failing map, so runtime validation uses private copies of
Bigass and Mercury Falling with this global added; originals remain unchanged.

`mcc_game_speed.c` owns synchronization and client state. Only the host may
change speed. Clients retain the native cheat enforcement and receive a
separate multiplier from the host on its reliable, ordered connection.
Message 90 requires exactly one float, an exact packet size and the host
connection; unreliable packets and client senders are rejected. Invalid float
values are ignored. Updates continue in wall-clock time at speed zero, and
repeat for joining clients. They are deliberately outside the tick-only batch
and stale-tick filter, so pause/resume and checkpoint rewinds remain possible.
Network version **41** is required for this new message and timing agreement.
The existing engine admits new players on simulation ticks: joining a host
already at speed zero can time out before admission. Resume before admitting
a new player. Existing players can pause/resume together; a late join at half
speed also receives the correct multiplier. This change does not alter the
shared player-admission implementation.

The host's existing client speed-hack detector still runs. Its elapsed time
uses an MCC-owned clock integrating only host-approved speed changes. At zero
speed it retains a small positive budget rather than exempting clients. Client
movement acceptance and other host-authority checks are unchanged. MCC unload
and new-map initialization clear remote speed and send/clock bookkeeping.

The interpreter's global reconciliation and the frame/tick clock read native
state directly, so MCC modules alone cannot implement these semantics.
Small hooks in `hs_runtime.c` and `game_time.c` invoke the MCC adapter;
`network_distributed.c` adds dispatch and a clock query. Xbox/CE lookups and
clock behavior retain their original values when the MCC guards are inactive.

Bigass's day, night and snow maps each contain 21 environment shaders with
flag `0x8`, including `altis\\levels\\bigass\\shaders\\bigassgrass` in the day map.
The [environment shader format reference](https://c20.reclaimers.net/h1/tags/shader/shader_environment/)
identifies this as alternate bump attenuation, correcting repeated diffuse
attenuation that can darken terrain and expose triangle boundaries. The flag
layout is corroborated by Invader's public shader_environment schema. No
proprietary shader implementation or SDK material was used.

`mcc_environment_shader.c` scopes this behavior to a flagged MCC lightmap
draw, excluding specular-mask-only bump maps. The platform companion emits
an independently reconstructed response: `max(0, 1 + dot(normal, light) -
light.z)`, followed by the existing combiner clamp. A flat normal therefore
preserves the baked lightmap response instead of attenuating it again, while
normal variations still affect it. **This is an approximation of the documented
intent, not a verified copy of MCC's exact shader equation.** The supplied
native MCC screenshots establish the visual symptom; controlled
same-camera comparisons show the reduction in dark terrain triangles. Exact
brightness in every MCC material remains a playtest limitation.

This cannot be implemented purely by changing converted texture bytes: it
depends on the incident-light vector of each fragment. The necessary shared
additions scope the environment draw, carry one bit in the compiled shader
cache key, and select the MCC emitter only for the matching bump-dot combiner.
All other combiners retain the original generator. The draw clears its state
immediately afterward, so subsequent materials and map families cannot inherit
it. No texture, global brightness, Xbox shader flag or CE loader is rewritten.

## MCC lobby readiness (issue #51)

The native pregame client polls map readiness once per second. The MCC hook
previously reread tag data and fingerprinted every required resource on each
poll, blocking menu frames. Mercury Falling's supplied bitmap and sound
resources total roughly 687 MB; repeated reads reproduced the reported stalls.

`mcc_precache.c` retains one successful readiness result while the map and all
eight possible bitmap/sound resource candidates have unchanged file metadata.
Missing candidates are recorded too: adding a package override or another
shared resource set invalidates the result. Size, creation/write time and
attribute changes trigger full validation; access time does not. Failed
validation is never cached, unreadable metadata prevents reuse, and a change
during validation cannot establish a cached ready result. Rescanning the MCC
catalog and unloading MCC tags clear the result.

This is a polling optimization, not a trusted content fingerprint. The original
issue #51 fix left network admission, map-version calculation and actual loading
on independent resource validation paths. The issue #74 extension below reuses
unchanged checksum validation between the first two paths; actual loading still
validates independently. Neither change alters host-authority rules or wire values.

The existing MCC branch in `cache_files_give_time_to_precache` must select the
new polling function because its old callee also serves admission callers,
which retain their admission checks. This was the only shared-engine change
for issue #51. Xbox and Custom
Edition readiness branches retain their existing code and behavior.

## MCC map-selection validation (issue #74)

Selecting a resource-backed MCC map computed its resource checksum, then
computed it again for local admission and the first readiness poll. The
separate readiness cache did not cover those calls, and opening the map browser
invalidated readiness even when no files changed. These synchronous reads can
stall the menu while it is open or returning to the lobby.

The original issue #74 fix made `mcc_cache_probe` retain one successful checksum with the complete
`mcc_precache_snapshot`. Every reuse checks the name and all nine file stamps;
failed validation or unavailable metadata cannot populate the cache. A change
while validating refuses the result. The requested network checksum and admitted
SHA-256 identity are still checked on every admission. Unload and disconnect
clear the result, while catalog rescans can safely reuse unchanged inputs.
The loader still validates the opened resources independently. All changes are
in MCC-owned modules; no Xbox/CE implementation or network wire value changes.

That initial fix also opened loading feedback for uncached probes. It was
insufficient: disconnect cleared the result and a settings send during teardown
could prepare the host identity again. Selecting a map also checked readiness
synchronously through `main_set_multiplayer_map_name`.

The follow-up keeps host preparation in MCC-owned pregame updates. Menu actions
schedule a check, host readiness polls completed results, and settings sends
never initiate validation. A cancellable worker checks complete CRC and SHA-256
values using only private file readers, with incremental pregame work as a
fallback when a worker is unavailable. Early start requests wait in pregame;
four successful file-stamped results survive session teardown, while session
bindings and unfinished readers are cleared. Any changed resource candidate
invalidates reuse. Actual loads retain their independent content checks and
loading display. The renderer, preparation settings, Android protections and
Xbox/CE paths are unchanged. See the
[reader's guide](reading/maps_and_resources.md#mcc-loading-feedback) for the
call path, slice limits and lifecycle. The measurements below describe the
original fix, not validation of this follow-up.

### Missing-unit script calls (issues #50 and #63)

The build-111 executable and its matching published PDB resolve issue #63's
fault to `object_get_and_verify_type`, called by `unit_kill_silent` from a
scenario script. The datum is `NONE`. Mercury2's `bsp_switching_credits`
script kills three named scientist units, which may be absent; this is a
plausible call site, not confirmation of the reporter's playthrough location.
Calling `(unit_kill_silent none)` in the original build reproduces the same
fault and relative instruction address. Tiny Halo's startup independently
reproduces the corresponding `unit_kill` fault before its player proxy exists.
The public [c20 script reference](https://c20.reclaimers.net/h1/scripting/)
also documents the pre-MCC crash when either command targets a missing unit.

The MCC campaign adapters now evaluate the argument normally, preserve a
yielding evaluation, and validate the full datum and unit type before calling
the existing kill routines. Missing, stale and non-unit targets return without
a kill. Valid units retain the existing death behavior. Execution requires an
active MCC cache and local/host authority. The kill adapters do not change any
Xbox or Custom Edition function table, evaluator or shared-engine source. The new entries are
appended to preserve existing MCC function indices. There is no new state or
message to synchronize: scripts run on the host and deaths use the existing
replication. These adapters alone do not require a protocol change; the
recording policy below does require matching MCC peers.

### Weapon-only MCC recordings (Tiny Halo)

Tiny Halo's two 78-byte version-4/control-version-4 weapon-switch recordings
have zero throttle and all three initial direction vectors zero. Their only
effective event changes the weapon slot, followed by a timed end event.
Preserving the unit's current directions is an explicit compatibility policy
for this non-directional input, inferred from those inspected streams and
their weapon-switch script use; it is not a claim to reproduce MCC's private
playback implementation. The converter logs which recordings use the policy.

The existing bounds-checked MCC recording walker recognizes this case only
when the entire stream through its end is valid, initial control flags and
movement/direction vectors are zero, and it contains a weapon event with no
other effective event type. Signed zero is accepted. Authored directions,
motion, control/animation-state events, older codecs and malformed streams
do not select the fallback. Recording bytes, event timings, weapon slots and
the remaining controller fields are not rewritten by this policy.

`mcc_recording_control.c` initializes only the playback controller's three
directions from the target unit's valid current orientation. It requires an
active MCC cache, MCC-owned stream storage and a valid unit. A minimal `port:`
hook in `recorded_animations.c` supplies its private controller immediately
after native decoding. Conversion alone cannot supply a live unit orientation:
the same recording can be played on different units facing different ways.
The hook returns without changing anything outside MCC or for ordinary
recordings. It adds no persistent state or allocation to leak across reloads,
checkpoints or map-family transitions. Existing playback still owns the
recording's lifetime, possession, events and completion.

In co-op the host continues to initiate scenario/AI recordings; existing unit
controls and object synchronization carry their results to clients. Multiplayer
clients can also run local scripts, so MCC protocol 46 requires matching
recording behavior on both peers. No packet layout or client authority was
added, and the OpenCE/Xbox/Custom Edition protocol remains 29. The asset-free
tests exercise the production walker and runtime
hook, including every stream truncation, nonzero/NaN/infinite directions,
other event types, distinct current directions, invalid units, foreign data,
and inactive-MCC behavior. The combined focused suite passed 188 tests.

### Mercury2 Doozy boat controls (issue #62)

Mercury2's Doozy uses the three-powered-point boat controller. Its inspected
`crashsite\levels\splashsite\doozy\doozy` physics tag (index 3733 in the
supplied map) has finite mass/inertia data and water-friction plus water-lift
flags (`0x0a`) on all three powered points. The engine's boat controller
assigns `water_friction_velocity` only on point zero; `physics_compute_new`
reads it on points one and two as well. Those stack values were uninitialized.
A non-finite value can contaminate the boat's force, position and orientation;
the issue reports precisely those invalid vectors. This also explains why a
client's earlier visible copy cannot be boarded on the authoritative host.

`update_human_boat_physics` in `source/units/vehicles.c` zeroes the three
temporary controls before assigning its rotations, lift and propeller speed.
This shared producer correction replaces the former MCC-only initializer.
Conversion alone cannot initialize this private per-tick stack storage. The
native engine accepts the same powered-point flags, so Xbox and Custom Edition
boats need the correction too. Other vehicle controllers and boat point counts retain their paths.
No authored tags, network permissions, packet fields or seat rules change.
There is no persistent state to clean up across map-family transitions.

The original fix introduced MCC protocol 47 to require this initialization on
every peer, including a client predicting a boat it drives; OpenCE's separate
protocol was then 29. The current agreements are recorded in
[NETCODE.md](../port/linux/NETCODE.md#opence-network-agreement).

The [boat harness](../tools/harness/tests/test_mcc_boat.py) extracts the real
controller, scratch structure, force consumers and linear integration. It uses
non-finite and finite stale control values, checks observable forces, active
controls, alternate point counts and adjacent storage, and removes the
initializer in an isolated negative control. Its world queries and transforms
are controlled fixtures; this is not a complete vehicle simulation.

#### Shared-engine investigation, 7 November 2026

The refactor audit confirmed that this is a general controller defect, rather
than an MCC layout difference. `vehicle_update` in
[vehicles.c](../source/units/vehicles.c) allocates the powered-point array on
each call. Its boat branch assigns all rotations and lift ratios but only
point zero's water-friction velocity. Native vehicle validation accepts boat
type 2, and native physics validation accepts three powered points with these
flags. The unchanged functions in
[physics.c](../source/physics/physics.c) consume those controls independently
of map family. They clear the separate mass-point output array and derive
rotation matrices; neither action initializes the control scalars. No prior
tick's state belongs in this local array.

The new reproduction feeds identical accepted content through the controller
and force consumer with MCC inactive and active. With MCC inactive, poisoned
hydrofoil controls produce non-finite forces or incorrect finite friction.
The current MCC initialization avoids both failures. This establishes a class
of supported inputs, without a map name, checksum or asset selector; it does
not establish that stock Xbox maps contain such boats.

The minimal shared correction initializes exactly three records inside
the existing three-point branch, retaining its arithmetic and the other-count
fallback, and retires the MCC helper. Clients predict driven vehicles, so the shared correction requires updated
peers through the agreements documented in
[NETCODE.md](../port/linux/NETCODE.md#opence-network-agreement). It is independent
of optional camera mode and map identity.

The twelve historical legacy failures are now ordinary passing regressions.
Isolated mutations reintroducing the MCC guard reproduce the defect;
adjacent
storage, active controls and alternate point-count checks remain. Source
inspection also found the omission in upstream `fc9deb0b1`; no unmodified
upstream compatibility is claimed.

## Cutscene title layout

Issue [#76](https://github.com/chestahh/GulchCE/issues/76) reports truncated
Warthog Rally titles. Inspection of the local `warthog_rally.map` found 17
scenario titles, all centered, with horizontal bounds 240..400 and text flags
zero (no wrapping). The `gold` title references the complete trophy message;
its string is not truncated in the map. A hidden Windows run of `bed24ca3`
displayed only "You won the gold", confirming clipping at the authored box.

The MCC compatibility policy lets non-wrapping titles use the available
screen width around their authored alignment anchor. This is based on the
intact authored messages and observed clipping, not a claim that the original
MCC renderer has been executed or precisely reproduced. Wrapped, inverted and
already offscreen rectangles retain their bounds. Vertical layout, font,
colors, fades, title timing and script/network state are unchanged.

`mcc_hud_title_bounds` in `port/linux/game/mcc_hud_draw.c` owns the adjustment.
The one additive call in `source/cutscene/cinematics.c` is needed after native
widescreen positioning: changing the tag rectangle at load time would also
change that positioning, particularly for left/right-aligned titles. The
helper returns immediately outside MCC and changes only the temporary draw
rectangle; no allocation, retained state or unload cleanup is needed. The
native text layout and Xbox/Custom Edition title implementations remain intact.

The asset-free title tests execute the real renderer dispatch and MCC helper,
covering legacy bounds, wide screens, wrapping, alignment, tag immutability,
and MCC-to-legacy transitions. A negative control removes the integration
call and must reproduce the failure. These checks cover title layout, not
full map compatibility or campaign completion.

## Issue 79 map compatibility

The 2026-10-29 investigation used the supplied version-13 maps against testing
base `bed24ca319abae84f1de99351642a8ba27710f09`. The implementation stays in
MCC-owned modules, using existing engine interfaces. No Xbox/CE implementation,
map directory, native function table, network packet or checkpoint layout changes.

- **Bot Bootcamp A10:** its syntax node 8865 assigns literal true to
  `breadcrumbs_navpoints_enabled_override`. MCC already exposes breadcrumb
  availability as true. This assignment and boolean reads now agree with that
  policy; computed assignments and disables remain explicit errors.
- **Bot Bootcamp D40:** `characters\ai_bbcmp\aicyborg` omits the neck-to-head
  child edge. Its associated cyborg model supplies that edge with matching
  ordered names and parents.
- **Arbiter's Demise DE1:** `np\objects\vehicles\ghost\hr\ghost` omits
  sibling edges and stores zero in every parent slot. Its associated model has
  the matching ten nodes and a complete hierarchy. Recovery fills omissions
  only when retained edges agree; the complete tree still undergoes cycle,
  parent and reachability checks.
- **Super Wacky Funtime Ultimate MCC Edition C20:** the Flood collision model's
  node 3, BSP 0, surfaces 3 and 5 name starts belonging to the other surface.
  Their own four-edge rings are intact. MCC selects a start from each complete
  owned ring, preserving all vertices, planes and edge links, then invokes the
  existing collision check. Related models expose the same defect. The map also
  uses `cheat_all_weapons`; an MCC handler invokes the existing service only on
  the local game or host. Existing host object replication carries the spawns.
- **Halo CE Extended A30_2:** sound tag 3267,
  `sound\dialog\infinite\perez\startle`, has sample rate NONE and one empty
  pitch range. Its unused header is normalized without inventing audio. Nonempty
  unknown-rate sounds still fail. The next blocker was the old aggregate audio
  allocation reaching the engine's strict 256 MiB allocation bound. Conversion
  now retains separate tag-owned streams: this fixture converts 17,992
  permutations into 401,105,808 ADPCM bytes. Cache reads already carry the full
  sound tag, and MCC validation now checks the same ownership. Individual
  streams stay below 256 MiB and aggregate allocated capacity is bounded to
  1 GiB. All streams are freed on failed load and unload.

The new script entry is appended, preserving existing MCC function indices.
The weapon operation uses an existing host-only service and replication format;
audio offsets are local cache data, and newly recovered graphs retain their node
indices. No new peer agreement or message format is introduced, so the native
and MCC network versions remain unchanged.

Focused asset-free coverage exercises partial animation links, conflicting
models and cycles, misplaced collision starts, incomplete rings, sound ownership
and stale handles, truncated reads, unknown-rate payload rejection, allocation
failures and teardown. The native sound harness compiles the real converter;
codec-vector tests continue to verify its output. Campaign tests check weapon
spawning on the host/local game, refusal on clients and after teardown, and
restart. The reader's route is in [Maps and resources](reading/maps_and_resources.md).

Disposable map fixtures, generated caches and duplicate test binaries were
removed. Useful logs were retained, and test saves were archived with their
original relative paths and SHA-256 hashes; every archived entry was verified
before removing its loose copy. The final Windows executable and debug symbols
remain available for the coordinating chat. Original supplied maps and saves
were not modified.

Remaining limitations established during this investigation:

- **CE Third Person Armory** enables I Would Have Been Your Daddy and Grunt
  Birthday Party. These skull effects are not implemented. The refusal now names
  the offending global and syntax node; accepting an enable as a no-op would
  change authored gameplay and is intentionally not done.
- **PFD** loads and advances simulation during a three-minute idle run, without
  reproducing the reported post-transition crash. Its assault rifle and M4A1
  reference the model shader `deathstar\project_subterfuge\shader model\tag_334`
  as an object modifier. That shader kind remains unsupported in the modifier
  renderer, and the warning is retained. The original crash still needs a
  reproducible route or checkpoint; startup success does not resolve it.
- Windows debug portable builds were used, with PGO and LTO disabled. These are
  bounded startup/simulation checks, not completed campaigns or visual parity
  verification. Android and web builds were attempted but their generated shell
  commands cannot run on this Windows host; WSL is absent. Platform integration
  must repeat those builds in a supported environment.

## Issue 89 script extensions and animation recovery

The supplied version-13 maps reproduce three separate admission failures:
Ruined Halo `b30h3` at animation tag `ea4008cc`, Mission Z9 `z9` at
`abs_integer`, and Halo RPG `halo_rpg` at `kill_thread` and
`get_executing_running_thread`.

`mcc_scripts.c` registers the three functions outside the native Xbox/CE
function table. `abs_integer` takes and returns a 32-bit long; negation uses
unsigned arithmetic, retaining `0x80000000` for the unrepresentable positive
absolute value of `LONG_MIN`. This edge policy is explicit and tested; no
observation of MCC's `LONG_MIN` result is claimed. The public
[H1 script reference](https://c20.reclaimers.net/h1/scripting/#math)
documents the ordinary integer signature. Z9 calls it on the difference
between its marine count and `ai_living_count`.

Halo RPG contains 40 compiled self-termination calls of the form
`(kill_thread (get_executing_running_thread))`. The MCC runtime adapter
returns the full salted thread handle and checks that handle before cancelling
an invocation. It releases parameter-held object lists, clears the stack and
marks a scenario thread terminated, so ordinary `wake` cannot resume it.
Self-termination does not return through discarded frames. The native scheduler
still owns deletion of its current console thread. Invalid, unsalted, deleted
and stale handles do not cancel another thread. These operations add no external
state: thread state retains its existing checkpoint, restart and unload lifetime.
The previously supported named `sleep_forever` reset operation is unchanged.

Ruined Halo's graph `reach\objects\vehicles\human\warthog\warthog`
has 20 named nodes. Its root starts at suspension node 2, leaving cargo node 1
and its descendants outside the root's child chain; several of those descendants
also carry zero parent indices. All three objects referencing the graph point
to a model with the same ordered names and a complete hierarchy. The MCC
validator recovers incomplete chains only from agreeing associated models.
Nonzero retained parents must match, siblings must belong to the same parent,
children must belong to their stated parent, and cycles still fail. Zero parents
are treated as omitted only in this incomplete-chain recovery path. Complete
hierarchies are preserved. The recovered graph still passes full reachability
and permutation checks; no animation or vertex indices are remapped.

Mission Z9's `watch_land` writes `rasterizer_model_lighting_ambient` to
`0.1`, then `0`. This is implemented by `mcc_campaign.c`, using the existing
native lighting global and renderer. A positive value selects constant model
ambient lighting; zero returns to ordinary model lighting. MCC captures the
pre-load value, restores it on unload and script startup (including `map_reset`),
and includes the live value in host-owned campaign snapshots. The existing
`mcc_hs_threads_activate` lifecycle entry calls the MCC lighting reset; no new
shared-engine hook or renderer branch is needed. Xbox/CE write permissions and
loading paths are unchanged.

Campaign state version 6 is 772 bytes, with the new real after the previous
768-byte version 5. Older checkpoint states default the absent lighting value
to the captured pre-load value. Snapshot validation rejects nonfinite and
out-of-range values. MCC protocol 63 introduced the larger snapshot on both peers
while the shared agreement remained OpenCE 41. That was an MCC-only agreement
change. The current shared checkpoint lifetime fix separately requires updated
OpenCE 42 peers for every map family; see the
[current protocol boundary](../port/linux/NETCODE.md#opence-network-agreement).

`mcc_checkpoint.c` keeps the existing 768-byte campaign reservation and all
thread/object-name arena addresses. Checkpoint footer version 3 uses four of
its eight reserved bytes for the lighting extension and four for a footer CRC.
It reassembles the full campaign snapshot for validation and restore. Versions
1 and 2 retain their original layouts and checks. Increasing the reservation
instead would invalidate pointers inside older saved thread stacks.

A longer Ruined Halo run also exposed a later particle-system assertion:
`particle_system_new_particle_jet` read constant index 2 from a two-entry block.
The supplied map contains two-entry jet types in dirt, blood and water effects,
and a needler type that selects jet creation in a later state. The pair contains
velocity and spread; the January engine additionally reads a rotates-up switch.
`mcc_particle_type_constants` preserves the pair and appends a false switch in
bounded MCC-owned runtime tag storage. Existing three-entry blocks are unchanged.
This is an explicit compatibility default, not a claim of verified MCC visual
parity. The same third slot is also used as intensity if a later state switches
to explosion creation; an absent slot defaults to zero there. Shorter required
blocks, invalid extents and allocation failures are refused before runtime.
The public [Invader particle-system schema](https://github.com/SnowyMouse/invader/blob/master/src/tag/hek/definition/particle_system.json)
corroborates the block layouts and mode values; this implementation was written
from the local engine consumer and inspected map data, without copied code.

These changes remain in MCC-owned modules and their tests. The host continues
to run campaign scripts. Regression tests exercise the real linker, evaluator,
validator, campaign snapshot and checkpoint adapters: integer limits, argument
shape, stale thread handles, object-list cleanup, malformed graph links, jet
constants, lighting ownership and older checkpoints.

## Flamethrower particle-system views (issue #33)

Ruby Rebalanced C20 attaches `defoliant` and `defoliant2` to the world-model
`defoliant spawn` marker, with particle-type flags `0x10041` and `0x10001`.
Its `fp_defoliant3` and `fp_defoliant2` attachments instead name `spawn fire`,
which exists only on the first-person model, with flags `0x20041` and
`0x20001`. These are inspected map fields, not filename-based special cases.
The [public particle-system format](https://c20.reclaimers.net/h1/tags/particle_system/)
identifies bits 16 and 17 as the first- and third-person drawing exclusions.

The inherited particle-system emitter queries only world-model markers, and
its renderer does not apply these exclusions. Thus the first-person stream
has no emission marker while the lower world-model stream appears in the
player's view. In a hidden Windows reproduction, the original firing effect
and projectile markers both resolved; that did not establish that the
separate attached particle systems were working.

[mcc_particles.c](../port/linux/game/mcc_particles.c) supplies the missing
MCC behavior. `mcc_particle_system_markers` uses the active local viewmodel's
marker for first-person-only types; a missing marker, remote or holstered
weapon, deleted handle or third-person camera does not fall back to the world
origin. `mcc_particle_system_visible` applies the exclusions for each viewer,
so another split-screen player can see the world stream without seeing the
owner's first-person stream. Types without view exclusions retain their
existing behavior. The two `port:` hooks in
[particle_systems.c](../source/effects/particle_systems.c) are necessary because
emission and drawing are private engine operations without registration
interfaces. Xbox and Custom Edition retain the native queries and drawing.

The adapter has no persistent state or allocations. Existing particle systems
own their emitted particles and dispose them on map unload; orphaned
first-person-only streams remain hidden. Weapon switching and camera changes
are evaluated from current engine state. Projectile origins, damage, network
messages and deterministic simulation are unchanged; the particle system
uses the local visual random seed, so no network-version change is needed.

`tools/test_mcc_particles.py` exercises the actual adapter functions, including
marker failures, invalid attachments, view flags, split-screen ownership,
weapon deletion, view changes and MCC-to-legacy activation transitions.
Private before/after Windows captures of the same Ruby C20 fixture showed the
missing barrel stream before and a stream leaving the barrel after the fix.
The fixture changed only a starting profile's weapon to obtain the
flamethrower, and used temporary stationary firing input; neither change is
shipped. This is a bounded visual check, not full campaign completion.

The final release/portable Windows build passed, as did 149 focused regression
checks (two skipped: an optional MCC map fixture and a POSIX-only socket
harness). Both changed translation units also compiled to Android guest ELF
and web Wasm objects with the generated target flags. Full Android and web
builds were attempted but remain unverified: their generated Unix shell
commands and the Android NDK wrapper cannot run unchanged on this Windows
host, which has no WSL. These object checks do not replace full platform
builds, multiplayer testing or visual checks on those platforms.

The final Windows binary also completed a single-process sequence from Ruby
C20 to Xbox `a10`, to Custom Edition `deathisland`, then back to Ruby C20,
with console responses after each load and a normal timed exit. This checks
bounded loading and activation transitions, not full legacy visual parity.

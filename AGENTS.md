# Notes for agents

This repository ports the Halo: Combat Evolved decompilation (Xbox build
01.01.14.2342, `cachebeta.exe`) to Linux, Windows, Android and the web. The game's C
sources are the decompilation; the port adds a platform layer that
implements the Xbox APIs the game calls, and features the Xbox game did not
have (a new netcode, internet play, the PC version's menus, Custom Edition
maps, touch controls). The project is published as OpenCE.

Start with [README.md](README.md). The other documents are listed at the
end.

## Layout

| Path | Contents |
| --- | --- |
| `source/` | The game: 466 C files of the decompilation, compiled as they are with few changes (each marked, below) |
| `port/linux/src/` | The platform layer the three ports share: Direct3D 8 on OpenGL (`d3d8_gl.c`), DirectSound and XInput on SDL3, files, threads, memory, sockets, settings (`port_config.c`), the mods folder (`port_mods.c`), internet play (`p2p*.c`), the dedicated server's platform side (`server_*.c`: the mode, the log, the rotation's grammar, HTTP, RCON, the master servers' publisher) and the master servers' list (`master_list.c`) |
| `port/linux/game/` | The port's game-side code, compiled as the game's sources are: the distributed netcode (`network_*.c`), co-op, OpenCE's gametypes (`game_modes.c`, `infection.c`, `gun_game.c`), map sharing (`map_share*.c`), the menus (`menu_tags.c`, `menu_functions.c`), Custom Edition maps (`custom_edition_*.c`, `cache_file_formats.c`), the tag validator (`tag_validate.c`, `tag_schema_*.c`), mods (`mods.c`, `mod_*.c`), streamer mode's word filter (`text_filter*.c`), the dedicated server's director, commands, gametypes and status (`server_*.c`), touch, FOV and more |
| `port/linux/include/` | The port's headers: the limits (`halo_port_limits.h`, with `HALO_PORT_NETWORK_VERSION`), capacities (`halo_port_capacity.h`), the prefix header, `halo_linux_source_fixups.h` (port functions the game's sources call) |
| `port/include/xdk/` | Stand-ins for the Xbox SDK's headers (declarations only) |
| `port/windows/` | The Windows build's own files (`win32_*.c`, headers) |
| `port/android/` | The Android app: the guest runtime, the host library (`host/`), the Java app (`app/`), host imports (`host_imports.list`) |
| `port/web/` | The web build: its runtime (`src/`: entry, sockets inside the page, memory watch), the page and its service worker (`site/`), the SDL port, tests |
| `port/assets/` | What the builds embed or ship: high-res HUD (`hud/`), fonts, titles, menus (`menus/`), icons, network brokers and master servers, the dedicated server's web page (`server_ui/`) |
| `port/third_party/` | Vendored libraries, each with a README naming its upstream, version and checksum |
| `port/tools/` | Stand-alone tools (`cache_file_report.c`) |
| `tools/` | Build scripts (`linux_build.py`, `windows_build.py`, `android_build.py`, `web_build.py`, `ci_build.py`, `web_serve.py`), generators (`ce_menus.py`, `port_settings.py`, `hud_assets.py`, `title_assets.py`, `embed_assets.py`, `server_ui_embed.py`, `xdk_headers.py`), the dedicated server's protocol simulator and remote console (`server_sim.py`, `opence_rcon.py`), tests, and `stagekit/` (scenarios staged and instrumented on the real engine, no retail assets) |
| `docs/` | Longer design notes (`custom_edition_caches.md`, `mods.md`, `dedicated_server.md`, `master_protocol.md`) |
| `pgo/` | Profile-guided optimisation profiles |
| `deploy/` | Running a dedicated server: systemd, Docker, Pterodactyl, Helm, PufferPanel, MCSManager, example gametypes |
| `assets/` | Game data for local runs (gitignored: `maps/`, `custom_maps/`) |
| `.github/workflows/` | CI: builds every platform on each push, publishes releases from `main` |

## Building

```sh
python configure.py            # debug build; add --release, --portable, --profile, --pgo=off, --lto=off
ninja linux                    # build/linux/halo
ninja android                  # the Android guest image and native libraries
ninja android_apk              # the APK (Gradle)
ninja web                      # build/web/site (Emscripten: port/web/README.md)
ninja windows                  # on Windows, or on Linux after configure.py --windows-cross
python tools/ci_build.py linux release   # what CI builds
```

- `configure.py` writes `build.ninja`; `configure_args` at its top records
  the options it was run with. Restore them after trying other options.
- The Linux and Windows builds are 32-bit x86 with clang, in MSVC's ABI
  (`-fms-extensions -fshort-wchar -malign-double -fcommon`), C89 (`gnu89`):
  declarations come before statements.
- Every native build compiles without fused multiply-add and with the
  vendored musl maths (`halo_math.h`), so that every machine of a network
  game computes the same results. Do not call the system's maths functions.
- Android: the game is an ILP32 AArch64 guest (`arm64_32`) in a 64-bit app.
  Guest code can call only the host imports declared in
  `port/android/host_imports.list` and `port/android/guest/runtime/guest_host.h`,
  and only the SDL functions `guest_sdl.c` bridges. Run `ninja android`
  after changing anything it compiles: `port/linux/src`, `port/linux/game`,
  `port/android`, `source/`, the build scripts.
- Web: the game is `wasm32` (also ILP32), run on a Web Worker; calls must
  match their callee's type exactly (the link fails on a mismatch), and
  WebGL 2 is ES 3.0. Run `ninja web` after changing what it compiles (as
  for Android, and `port/web`). Refer to [port/web/README.md](port/web/README.md).

### Defines

| Define | Set by | Meaning |
| --- | --- | --- |
| `HALO_ANDROID` | Android | The app: its display, input, files and lifecycle |
| `HALO_GLES` | Android, web | The OpenGL ES renderer |
| `HALO_ARM64_GUEST` | Android | The guest's ABI (ILP32 AArch64) |
| `HALO_WEB` | Web | The browser: its frames, storage, sockets and WebGL 2's limits (with `HALO_GLES`, not `HALO_ANDROID`) |
| `HALO_WINDOWS` | Windows (`halo_windows_prefix.h`) | The Windows build |
| `HALO_RELEASE` | `--release` | No assertions checked |
| `HALO_PROFILE` | `--profile` | The profiling build; a normal build must be unchanged by it |

`HALO_LINUX` and `HALO_CUSTOM_EDITION` are **not** defined by any build:
code guarded by them compiles to nothing. Custom Edition support is a
setting (`game.custom_edition`), checked at run time with
`custom_edition_cache_tags_loaded()`.

## Testing

```sh
python -m pytest -q tools/harness tools/test_touch_menu.py   # the engine's functions in a fake world
python -m pytest -q tools/stagekit/tests                    # scenarios on the real engine, no retail assets (Linux)
python -m pytest -q tools/test_linux_port.py                # the build, the maps' checks (needs assets/maps)
python -m pytest -q tools/test_mods.py                      # mods played over a map (needs assets/maps)
python -m pytest -q tools/test_bots.py                      # the bots in local games (needs assets/maps)
python -m pytest -q tools/test_cache_file_formats.py tools/test_bmp_files.py tools/test_profile.py
python tools/test_touch_layout.py                           # the Android touch layout (JDK 17+)
python tools/test_light_storage.py
python tools/test_death_timing.py
python -m pytest -q tools/test_web_build.py                 # the web build's graph, sockets, menus
node --test "port/web/tests/*.test.js"                      # the web page's importer and service worker
node port/web/tests/webrtc_native.mjs chromium firefox      # the native builds' WebRTC with browsers (Playwright)
python -m pytest -q tools/test_dedicated_server.py          # the dedicated server with clients (needs assets/maps)
python -m pytest -q tools/test_server_sim.py tools/test_egg.py   # the server's web page and protocol simulator, the egg
python -m pytest -q tools/test_p2p_tunnel.py                # internet play's invite and tunnel, over IPv4 and over IPv6 alone (brokers)
python -m pytest -q tools/test_net_compare.py               # net_compare.py's judging of logs, its copies and their processes
python tools/net_compare.py --a build/linux/halo --b <other build> --data <game data> --swap
                                                            # two builds in one network game, in step (retail maps: local only)
```

`tools/harness/README.md` explains how to add an asset-free test: the code
under test is taken from the sources, not copied. `tools/stagekit/README.md`
explains how to add a scenario: the real engine, with no retail assets.

Running the game without a person at it:

- `debug.network_test` (`HALO_NETWORK_TEST=host:<map>` or `join`) hosts or
  joins a game without the menus and logs each player's state every second;
  several copies play on one machine on different loopback addresses
  (`HALO_NET_ADDRESS`, `HALO_NET_BROADCAST`). `NETCODE.md` describes the
  test settings.
- `halo --server` is a dedicated server (`docs/dedicated_server.md`), and
  `halo --probe [<invite>]` prints a game's advertisement as JSON and exits;
  `tools/test_dedicated_server.py` runs both with clients.
- `debug.exit_after`, `debug.hidden_window`, `debug.null_renderer`,
  `debug.menu_open` (start on one menu screen), `debug.telnet_console`.
- Use a virtual display (Xvfb, with `SDL_VIDEO_DRIVER=x11`) for windowed
  runs, never the desktop of the person you work for, and stop only the
  processes you started.

## Rules of the project

### The game's sources

- Change `source/` as little as possible, and mark each change from the
  original with a comment that starts `port:`. Write like the surrounding
  code: its naming, its comment density, and the project's plain,
  descriptive comment style.
- A port function the game's sources call is declared in
  `halo_linux_source_fixups.h` or included with a note naming its file
  (`#include "view_fov.h" /* port: port/linux/game/view_fov.c */`).
- Notes such as `BUG (original, preserved for exact matching)` mark the
  original's bugs. Fixing one is allowed; say so in a `port:` comment.

### The Xbox SDK

`port/include/xdk` was written without the SDK's headers, from the January
build's debug information and public documentation. Never read the SDK's
headers or anything derived from them, never copy from them, and do not
read `xbox/include`. To add a name, follow `port/include/xdk/README.md`.

### Networking

- Raise `HALO_PORT_NETWORK_VERSION` (`port/linux/include/halo_port_limits.h`)
  with any change to what the machines send each other, or to anything they
  must agree on, and say why in the commit. Machines of different versions
  do not play together.
- Only the host decides; clients predict their own players. Read
  `port/linux/NETCODE.md` before changing the netcode.
- Internet play is the same code in every build, the web's too: a browser's
  tunnel is a WebRTC data channel (`port/web/src/web_p2p.c`), which the
  native builds take on their tunnel's socket (`p2p_webrtc.c`, `posix_dtls.c`).
  Check a change with both (`port/web/tests/webrtc_native.mjs`, and with the
  maps `port/web/tests/internet.mjs`).
- Anything another machine sends is untrusted: check sizes, indices and
  rates, as the existing handlers do. `debug.network_corrupt` tests this.

### Menus

- The menus must match Halo Custom Edition's. Their XML (`port/assets/menus/ce/`)
  is generated by `tools/ce_menus.py` from the PC version's tags; the
  settings screens come from `tools/port_settings.py`. Do not edit the XML
  by hand: change the generator and run it (the command is in
  `port/assets/menus/README.md`, "Writing them again"), then check that
  `git status` shows only the screens meant to change.
- Functions the menus call but the port has not wired are listed in
  `port/assets/menus/UNWIRED.md`.

### Art and credit

- Replacement HUD, title and menu art is made of hand-drawn redraws.
- Prefer an existing redraw (the ui-svg-handmade set), then a new redraw. A
  Custom Edition picture that cannot reasonably be redrawn (a 3D render, a
  screenshot, a logo) may be used only if it is listed in
  `port/assets/menus/NON_HANDDRAWN.md`.
- `AUTHORS.md` credits art only.
- Fonts: Overpass and OpenCE (Newtown, respaced) only; see
  `port/assets/fonts/README.md`. Never ship the maps' commercial typefaces.

### Custom Edition maps

Custom Edition maps live in the data root's `custom_maps` folder with
`bitmaps.map`, `sounds.map` and `loc.map`; their level names are
`custom_maps\<name>`. OpenSauce features are not supported and must not be
added: a map that needs them is refused, and one that only carries
OpenSauce's header runs as stock Custom Edition runs it. Map files are
untrusted input: read them through bounds-checked readers. See
`docs/custom_edition_caches.md`.

### Gameplay preservation, tags and map compatibility

- **Tag injection is an absolute no-go unless the development team
  explicitly approves it.** Do not propose, implement or submit tag injection
  on your own judgment, even if it appears to be the simplest fix. If a user
  requests it, be emphatic: tell them, "You're not going to get this PR through
  unless it's absolutely essential. Are you sure you want to spend your tokens
  on this?" Explain that it will not be accepted without explicit development
  team approval, and press the user to reconsider before doing any work.
  Never rely on identifying tags by name or route. Custom tags may already
  work around the underlying problem in their own way, so applying a fix to
  those maps could break them.
- **Do not fix original-game bugs that affect gameplay.** Think through the
  gameplay implications of every proposed change. Ask the user about the
  original behaviour and obtain confirmation before spending time, effort or
  money on a gameplay-affecting fix. A visual bug may be fixed only when it is
  genuinely visual and has no gameplay implications of any kind.
- **Never make map-specific corrections of any kind.** Do not add special
  cases, patches or altered behaviour for an individual map, even when a bug
  is first reported in only one map. A map-specific correction can break
  another map that relies on the existing behaviour, while accumulating
  technical debt and making the code harder to maintain, reason about and
  debug. Fix the underlying shared behaviour in a general way, or do not make
  the correction.
- Keep the game's intended use in view when making changes: Halo's workflow
  revolves around creating, making and sharing maps. Preserve the structures
  and workflows used to create and share those maps.
- Do not incorporate third-party or licensed software into the game. Keep
  pull requests small enough for a human to review in 15 minutes; do not
  submit enormous changes such as 300,000 lines at once. Never encode models,
  audio, images or other binaries directly into the game's code.
- Do not assume an asset is unmodified without checking it against a
  checksum. Do not rearrange or cut-and-paste assets to make them serve a
  different purpose (for example, splitting dialogue into pieces to make it
  say something else). Account for the fact that different language versions
  of the game have different resource-map files: `bitmaps.map`,
  `sounds.map` and `loc.map`.
- Do not assume a map or asset is corrupt, malicious or behaving
  unintentionally until you have established what the user actually observed.
  Ask whether the behaviour also occurs in the original game on original
  hardware, or only in an emulator; ask which game/version and which hardware
  or emulator they used. Do not accept an emulator's behaviour as proof of
  original-game behaviour: emulation bugs have led users to misidentify
  behaviour as original. Some maps were protected against tag extraction and
  can therefore appear corrupt, including tags with nulled fields, invalid
  addresses, or missing names or extensions.
- Before changing commands, ask the user what the command did in the original
  game on both the client and the server. Do not assume the intended behaviour
  from one side applies to the other.
- Pull requests that make visual changes must include before-and-after
  pictures or videos.

### Settings

A setting is an entry in `port/linux/src/port_config.c`, with a default, an
environment variable and a comment that `config.toml` gets. Document it in
the settings table of `port/linux/README.md` (and the Android README for an
Android-only one). A departure from the stock game's look or behaviour
should be a setting, off or stock by default, unless the project decides
otherwise.

### Third-party code and licences

The repository is `AGPL-3.0-or-later` (`LICENSE`); the commits before the
change, and the releases built from them, stay CC0 (`LICENSES/CC0-1.0.txt`).
The README's "Licence" section says what each part is under. Vendored code
goes in `port/third_party/<name>`, copied unchanged, with a README naming
the upstream, the version or commit, the licence, the checksum and what uses
it. Its licence must let it be combined with the AGPL 3.0: MIT, BSD (without
an advertising clause), ISC, zlib, Apache 2.0, MPL 2.0, LGPL or GPL 3.0 or
later, public domain. GPL-2.0-only code and the 4-clause BSD licence cannot
be. Code taken from another project, GPL 3.0 code included, keeps its
notices and its licence, and says where it came from.
The decompilation's upstream, punpckhdq/halo, is AGPL 3.0 from 17 October
2026; bnunu/halo-1 is CC0.

### Commits and pull requests

- Commit subjects are plain sentences saying what changed; the body says
  why and what was checked.
- A pull request is merged with a merge commit (`Merge pull request #N from
  <owner>/<branch>`), and follow-up changes go in separate commits whose
  subject ends with `(#N)`.
- A commit taken from a fork keeps its author and is made with
  `git cherry-pick -x`.
- Code follows [CODINGSTANDARDS.md](CODINGSTANDARDS.md): comments, tests
  and how the work is done.
- Release notes, pull request titles and descriptions, and the READMEs are
  written for their readers. Use the player-facing style below for release
  notes and feature summaries; keep the technical detail needed to review,
  use, build or troubleshoot the change in PRs and documentation:
  - Describe what shipped without a list of unrelated omissions. Include
    material limitations, compatibility constraints and untested cases
    wherever leaving them out would mislead the reader.
  - Lead with what a player sees ("Players can stand on & ride moving
    vehicles"), not a feature name ("Vehicle riding support") or how it
    works ("Grow a device's render sphere to hold its model wherever its
    animations carry it"). One phrase can cover several features.
  - A feature is a bare phrase saying what you can now do ("Riding is a
    gametype option", "Shared maps download in seconds"). Say when it
    matters, not how it works ("on emulators", not "switches write tracking
    to page hashes").
  - A fix starts with "Fixed" and names what was wrong ("Fixed a voice
    fading out of range jumping to a garbage gain"), not where it was seen
    ("Fixed a sound bug on Blood Gulch").
  - No code names (functions, macros, tags, files) except where a reviewer
    needs them to find the change: "Fixed a crash in debug builds of the
    MCC geometry check", not "Don't free a BSP's material runs when it has
    none".
  - Numbers a player feels, as before and after: "41 s instead of 3:33 for
    a 208 MB map", not "1089.28 world units".
  - No selling: "seamless", "fully", "thoroughly tested", "Key Features".
  - One idea per bullet: split "X & Y. Z" into two bullets.
  - Remove repeated or unnecessary words, but keep the ones that stop a
    misreading ("stand on & ride", since "ride" alone could mean a
    passenger seat).
  - One term for the same thing, and precise words ("system link", not
    "LAN"; "dedicated server", not "server").
  - No double negatives.
  - Short and personal is fine: fragments, "&", parentheses ("(like the
    Longsword)"), first person ("On my Galaxy S23").
  - Write for a reader who never saw the conversation behind the text:
    don't answer a question only the person you worked with asked ("neither
    needs any maps"). If the reader wouldn't wonder about it, leave it out.
  - Headings, lists and tables when they help the reader.
  - Compare against `testing`, not `main`.
- A pull request title is a sentence, not an "Area: fragment" ("Split
  screen: the Xbox's plasma behind ADD PLAYER's quarters").
- A pull request description:
  - opens with the effect, then the cause in a line or two ("The needler's
    needles sank during a reload and popped back up", not a paragraph of
    `weapon_magazine_finish_reload` and `MIN(rounds_total, ...)`);
  - has no file-by-file lists ("`gpu_profile.c` / `.h`: resolves...");
  - covers one feature (not "Forge Mode, Tag Vault, 3D Tag Previewer and a
    map builder" in one);
  - states what was checked, the results, and relevant risks, limitations
    or skipped checks. Keep these even when shortening the description.
- A README has no "today", no hidden or experimental modes in its features
  list and no unnecessary implementation detail. Include commands, settings
  and technical explanations where readers need them to use, build or
  troubleshoot the game; label experimental or diagnostic instructions.
- Review the final text for clarity and accuracy. A writing skill such as
  [humanizer](https://github.com/blader/humanizer) is optional; an equivalent
  manual review is sufficient. Preserve technical meaning and caveats.
- A pull request is small enough to review in 15 minutes where it can be
  ("Gameplay preservation, tags and map compatibility", above). One that
  cannot, of more than 3,000 changed lines, gets a reviewer's guide, posted
  as a comment, not in the description. Check every file and
  function name in it against the branch first. It is short, in the style
  above, with these sections:
  - Where to start: the main files in reading order, a line each on what
    the file does in this pull request.
  - Decompiled source: which `source/` files changed and why.
  - What should hold: the few invariants a reviewer should check
    (authority, behaviour with the feature off, when decisions are made,
    precedence).
  - Tests: what they are, and roughly how much of the diff they make up.
  - Trying it: the shortest steps to see the change in the game.

## Documents

| Document | About |
| --- | --- |
| [README.md](README.md) | Downloads, game data, building, build options, profiling builds |
| [CODINGSTANDARDS.md](CODINGSTANDARDS.md) | Comments, tests and how work is planned, built and reviewed |
| [port/linux/README.md](port/linux/README.md) | The Linux build, controls, menus, every setting, updates, frame rate, field of view, system link, internet play, voice chat, the server browser, map sharing, security, map checks, how the port operates, the game source changes |
| [port/linux/NETCODE.md](port/linux/NETCODE.md) | The distributed netcode, network versions, joining a game in progress, map sharing, transport, testing |
| [port/windows/README.md](port/windows/README.md) | The Windows build, its headers, inline functions, crash reports |
| [port/android/README.md](port/android/README.md) | The Android build, the guest and host, touch controls, Android settings, finding problems |
| [port/web/README.md](port/web/README.md) | The web build, the page, the game data in the browser, limits, how it operates, hosting on GitHub Pages |
| [port/include/xdk/README.md](port/include/xdk/README.md) | How the SDK declarations were written and are checked |
| [docs/custom_edition_caches.md](docs/custom_edition_caches.md) | Loading and converting Custom Edition maps, and the evidence for each layout |
| [docs/mcc_maps.md](docs/mcc_maps.md) | MCC maps: their format, installation, sharing, the implementation's layout and limits ([docs/reading](docs/reading/README.md) follows the sources) |
| [docs/dedicated_server.md](docs/dedicated_server.md) | The dedicated server: its command line, exit codes, commands, HTTP API, RCON, events and files |
| [docs/master_protocol.md](docs/master_protocol.md) | The master servers' protocol: publishing, the probe, the signed list |
| [docs/steam_workshop.md](docs/steam_workshop.md) | Android's Steam Workshop screen: signing in with Steam and downloading MCC content from its depots, in the app's host |
| [docs/mods.md](docs/mods.md) | Mods: folders of Halo PC tag files whose sounds, bitmaps, text and models play over a map's on one machine |
| [port/assets/text_filter/README.md](port/assets/text_filter/README.md) | Streamer mode's words: Surge AI's list, the review, the allowed words |
| [port/assets/menus/README.md](port/assets/menus/README.md) | The menu files' format and how to write them again |
| [port/assets/menus/NON_HANDDRAWN.md](port/assets/menus/NON_HANDDRAWN.md) | Menu pictures that are not redraws |
| [port/assets/menus/UNWIRED.md](port/assets/menus/UNWIRED.md) | Menu functions that do nothing yet |
| [port/assets/fonts/README.md](port/assets/fonts/README.md) | The fonts and their licences |
| [tools/harness/README.md](tools/harness/README.md) | Asset-free regression tests |
| [tools/stagekit/README.md](tools/stagekit/README.md) | Scenarios staged and instrumented on the real engine, no retail assets |
| `port/third_party/*/README.md` | Each vendored library |
| [AUTHORS.md](AUTHORS.md) | Art credits |

# Steam Workshop

Settings > Network Setup > **STEAM WORKSHOP** signs in with Steam and downloads
MCC content into `mcc_maps` on Android. The Steam client comes from
[Halo Content Evolved](https://github.com/Watanoge/halo-content-evolved)
(AGPL-3.0-or-later, like this port). Here it runs in the Android app's host
rather than the guest: `port/android/host/steam_*.c`. On Windows and Linux,
Steam's own client downloads Workshop items and the game finds them in place
(`port/linux/src/steam_workshop.c`), so the screen is Android's alone.

## What it does

- **Sign in** with the account name and password, typed in a text field on
  the screen's row with the system's keyboard. The password shows as stars, is
  RSA-encrypted under Steam's key for the account, and is never stored. Steam
  then asks for approval in the Steam app (which works with the app on the same
  phone) or for a Steam Guard code.

  The account must own Halo: The Master Chief Collection and Halo: Combat
  Evolved Anniversary, because Steam only gives the depots' keys to owners.
- **MCC FILE STATUS:**
  - Downloads the three shared resource maps from Halo: CE Anniversary's depot
    (976733) into `mcc_maps/_shared/singleplayer` and `mcc_maps/_shared/multiplayer`.
  - Lists the Workshop packages in `mcc_maps`, and updates or removes them.
    Each one's details end with its publisher.
- **BROWSE WORKSHOP:** the Halo 1 Workshop, 20 items at a time. You can search,
  sort and filter by type, and each item shows its preview picture, decoded
  by Android (`HaloActivity.steamPreview`), and its publisher under its
  description.
  - Installing an item builds it in `mcc_staging/steam_<id>`, then moves it into
    `mcc_maps/<id>/` whole, replacing an older copy.
  - The package has the layout of an MCC package in `mcc_maps` (`docs/mcc_maps.md`).
  - `steam/downloads.txt` records which version each item came from, which is
    how an update is told.
- **MANAGE ACCOUNT > LOG OUT:** revokes the sign-in with Steam
  (`Authentication.RevokeToken`), then forgets it on the device. If Steam can't
  be reached, the sign-in is forgotten on the device only. The screen then says
  that Steam's own settings can deauthorize it.

## Publishers

An item's publisher is the Steam name of the account that published it, who
may not be the original maker of what it holds ("Publisher" on the screens). Nothing
on the card names it, so the app's host asks Steam the first time an item is
shown in a run (`port/linux/src/workshop_authors.c`): the item's creator from
the public Web API (`ISteamRemoteStorage/GetPublishedFileDetails`, which needs
no key or sign-in), then the name on that account's profile
(`steamcommunity.com/profiles/<id>/?xml=1`). The lobby asks the same way for a
Workshop map, through the host (`host_steam_author`), and the desktop builds
ask from the game. Nothing is written to disk, and an item Steam could not
answer for is asked about again after 10 minutes at the soonest.

## Joining an MCC game

A joiner signed in with Steam installs a host's MCC package from Steam when
the package is a Workshop item (`port/linux/game/mcc_share.c`,
`steam_downloads.c`'s joiner's install), with no prompt, since the host is
not the only source. MCC's shared files come from Steam too when they're
missing; no host sends them. The host's offer is kept meanwhile, so the host
keeps counting the joiner as downloading, and the package is checked against
the host's fingerprint once it's in. If it doesn't match, or the install
fails, the joiner is asked to download it from the host as before.

Nothing new goes between the machines, so any host takes it, and a joiner
without a sign-in downloads from the host. The fingerprints the lobby and the
host's offer need are worked out in the app's host with the CPU's SHA-256 and
CRC32 instructions (`host_mcc_hash.c`) and kept on the card
(`mcc_fingerprints.txt`); an item is fingerprinted as it is installed.

## How it reaches Steam

- **Connection:** a TCP session with one of Steam's connection servers, found
  through `ISteamDirectory` over the host's verified HTTPS.
  - The channel key is sent encrypted under Steam's public universe RSA key,
    then every message is AES-encrypted with HMAC-derived IVs, as SteamKit does.
  - A server lookup that was tampered with can't read or change the session.
- **Downloads:** depot chunks come from the content servers over plain HTTP on
  kept-alive connections, as Steam's own client fetches them.
  - Each chunk is encrypted with the depot key and checked against its SHA-1
    after unpacking (LZMA, Zstandard or zip: `steam_unpack.c`).

## The sign-in on the device

A refresh token is as good as the account to whoever holds it until it's
revoked, so the session file (`steam_seal.h`) is kept in the app's private
storage, not the data folder, and sealed with an AES-GCM key in the Android
Keystore that never leaves it (`HaloActivity.steamSeal`).

- **A copied file opens nowhere else:** a copy of the file doesn't open on
  another phone. It also stops opening once the app is uninstalled or the
  player logs out, because the key is deleted.
- **Without a key:** if the Keystore can't seal the token, it isn't written
  at all. The sign-in lasts until the app closes, and the log says so.
- **How the file is written:** to a `.part` file with mode 0600 (where the file
  system has modes), then renamed over the old one.
- **The token stays in the host:**
  - The guest only gets rows, texts and preview pictures.
  - What the player types passes through the guest once, and the guest wipes its
    copy.
  - The host wipes the password, the Steam Guard code, the token, the channel's
    keys and the message buffers that held any of them before freeing them.

## Tests

`tools/test_steam_client.py` builds the client natively with `-Wall -Wextra
-Werror` and checks everything that works offline:

- the crypto against known answers;
- the session file and its seal (with a test secret standing in for the
  Keystore);
- the chunk packings;
- the paths an item's files may take.

The sign-in, the downloads and the revoke need Steam and an account that owns
MCC, so they're only tested on devices.

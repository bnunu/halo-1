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
- **BROWSE WORKSHOP:** the Halo 1 Workshop, 20 items at a time. You can search,
  sort and filter by type, and each item shows its preview picture, decoded
  by Android (`HaloActivity.steamPreview`).
  - Installing an item builds it in `mcc_staging/steam_<id>`, then moves it into
    `mcc_maps/<id>/` whole, replacing an older copy.
  - The package has the layout of an MCC package in `mcc_maps` (`docs/mcc_maps.md`).
  - `steam/downloads.txt` records which version each item came from, which is
    how an update is told.
- **MANAGE ACCOUNT > LOG OUT:** revokes the sign-in with Steam
  (`Authentication.RevokeToken`), then forgets it on the device. If Steam can't
  be reached, the sign-in is forgotten on the device only. The screen then says
  that Steam's own settings can deauthorize it.

## Joining an MCC game

A joiner signed in with Steam gets a host's MCC package from Steam when the
package is a Workshop item (`port/linux/game/mcc_share.c`,
`steam_downloads.c`'s joiner's fetch):

- **The whole item from Steam**, when the Workshop has it as the host has it:
  installed into `mcc_maps` and checked against the host's fingerprint, with
  no prompt, since the host is not the only source.
- **Its resource maps from Steam**, otherwise: the package's own
  `bitmaps.map` and `sounds.map` (most of its size) come from Steam while the
  host sends the rest. Each is checked against the host's SHA-256, then
  copied into the staged download, and the host is told the joiner has it,
  as with a local copy (`MCC_SHARE_OP_HAVE`). A file that does not match is
  the host's to send.

Nothing new goes between the machines, so any host takes it, and a joiner
without a sign-in downloads from the host as before. The fingerprints the
lobby and the host's offer need are worked out in the app's host with the
CPU's SHA-256 and CRC32 instructions (`host_mcc_hash.c`) and kept on the
card (`mcc_fingerprints.txt`); an item is fingerprinted as it is installed.

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

# LZMA decoder

`LzmaDec.c` and the headers it needs, from the LZMA SDK by Igor Pavlov
(public domain; taken from the 7-Zip repository's `C` folder). The game's
Steam client uses it for the "VZa" chunks of Steam's depots
(port/android/host/steam_unpack.c, in the Android app's host).

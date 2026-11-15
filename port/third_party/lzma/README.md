# LZMA decoder

`LzmaDec.c` and the headers it needs, from the LZMA SDK by Igor Pavlov
(public domain; taken from the 7-Zip repository's `C` folder). `LzmaDec.c`,
`LzmaDec.h` and `7zTypes.h` are copied unchanged; `Precomp.h` is an empty
stand-in for the SDK's, whose compiler settings (`Compiler.h`) the decoder
does without. The game's Steam client uses it for the "VZa" chunks of
Steam's depots (port/android/host/steam_unpack.c, in the Android app's host).

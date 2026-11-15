# Zstandard decompressor

The decompression half of Zstandard 1.5.7 (`lib/common`, `lib/decompress`;
BSD licence, LICENSE), built as one unit the way upstream's
`build/single_file_libs/zstddeclib-in.c` combines it, without its assembly.
The game's Steam client uses it for the "VSZa" chunks of Steam's depots
(port/android/host/steam_unpack.c, in the Android app's host).

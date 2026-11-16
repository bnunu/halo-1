# MCC conversion errors: before and after

This catalogue covers the diagnostic changes for MCC map loading and tag conversion.
It lists the message templates, including previously silent checks. `%lu`, `%d`, `%s`
and similar sequences are replaced by the actual values in `debug.txt`. Repeated
checks with the same message are listed once per function. Conversion rules,
resource selection, network protocol and Xbox/Custom Edition loading are unchanged.

## Reading the new messages

The normal conversion refusal has this form:

```text
mcc: '<original tag path>.<tag extension>', <field or element>: <specific reason>; map refused (tag <eight hex digits>)
```

Open that path under the source project's `tags` directory in Guerilla, or follow
the named scenario element in Sapien. All bracketed indices are **zero-based**.
A bitmap's `bitmaps[3]` is image 3 inside that bitmap tag, not item 3 in the map's
global texture-conversion list. Sounds identify both pitch range and permutation,
including their names. Models identify geometry and part, and BSPs identify
lightmap and material. Validation carries nested paths such as
`children[0].data`, instead of only the final field name.

The original entry `cannot convert texture 420 of tag e91e07aa` cannot establish
which bitmap or why it failed on its own: 420 was a global conversion-list slot;
`0x07aa` (1962) is the tag-table index within that exact compiled map. The revised
loader resolves the source tag and bitmap index itself. The following is an
**illustrative** failure, not a diagnosis of that old entry:

```text
mcc: 'weapons\assault rifle\bitmaps\ar_multipurpose.bitmap', bitmaps[3] (zero-based): pixel payload is too short: 4 bytes available, 8 required for format 14, 4 x 4 x 1, 0 mipmaps; map refused (tag e91e07aa)
```

A sound example:

```text
mcc: 'sound\weapons\rifle\fire.sound', pitch_ranges[0] 'automatic'.permutations[2] 'burst': Ogg page CRC checksum does not match its contents; codec 3 (Ogg Vorbis), payload offset 0x00001000, 256 bytes; map refused (tag e91e07aa)
```

The existing final `mcc: refused <map>: <stage>` line still identifies the map.
Package/resource errors name the required resource and file. Allocation failures
name the operation that ran out of memory; they do not imply the authored tag is
invalid. Invalid names use `(tag path unavailable)` plus the handle. Map-wide
failures use `(map)`. A compiled map cannot recover an authoring path it no longer
contains, nor the original source-file line for stripped script sources. Script
errors provide syntax node/source offsets and script/global names where available.

Runtime diagnostic names are read with bounds checks, control characters are replaced with `?`, and
long diagnostic fields are bounded (tag 319, location 191, reason 383 characters).
Source names beyond 255 characters end in `...` in runtime diagnostics. The
original scenario path is retained before its runtime identity changes. Bitmap
copies resolve back to their source bitmap; converted models retain `.gbxmodel`.
Failure-only source reads recover the original class without adding file I/O to
successful conversion. Diagnostic state belongs to the current MCC load and is
cleared on failed-load cleanup/unload. The loader retains its established concise
menu explanations separately from these detailed log messages.

## Catalogue conventions

In the tables below, **After** is the reason inside the normal refusal format
above, except for reader and resource messages, whose complete formats appear in
their sections. Where a helper previously failed silently, the table identifies the caller's generic refusal. A slash between old geometry messages denotes the model/BSP
caller variants of that shared check. A `%s` reason is expanded by the detailed rows in the associated
codec, recording, syntax or parameter section. Numeric details are retained for
correlating a compiled map, but are no longer the only way to identify a tag.

## Bitmap conversion

Locations: `bitmaps[index]`, or the owning shader/HUD bitmap reference field. Setup failures use `(map), bitmap conversion setup`. Read/packing/allocation failures append pixel file offset, required source bytes and allocation bytes. Normalized-descriptor failure includes signature, flags, format, type, dimensions and mip count; the subordinate check text is listed as well.

[Implementation](../port/linux/game/mcc_bitmaps.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_bitmap_clone_block` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap data block or source tag is outside loaded data` |
| `mcc_bitmap_clone_block` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `not enough tag memory for independent bitmap metadata` |
| `mcc_bitmap_specialize` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap specialization exceeds the tag table capacity (%lu)` |
| `mcc_bitmap_specialize` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap data block or source tag is outside loaded data` |
| `mcc_bitmap_specialize` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `not enough tag memory for independent bitmap metadata` |
| `mcc_bitmap_specialize` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `not enough tag memory for bitmap copy name` |
| `mcc_bitmap_specialize` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap specialization exceeds the texture table capacity` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap reference %08lx has the wrong group, a stale handle, or an out-of-range header` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `referenced bitmap has an out-of-range bitmaps block` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `referenced bitmap has an out-of-range sequences block` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap sequence %d has an out-of-range sprites block` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap sequence %d sprites[%lu].bitmap_index=%d is outside 0..%ld` |
| `mcc_bitmap_mark` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `bitmap sequence %d range (first %d, count %d) exceeds %ld bitmaps` |
| `mcc_bitmap_usages` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `shader_model header is outside loaded data` |
| `mcc_bitmap_usages` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `unit HUD header is outside loaded data` |
| `mcc_bitmap_usages` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `auxiliary HUD meters or overlays block is outside loaded data` |
| `mcc_bitmap_usages` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `weapon HUD header is outside loaded data` |
| `mcc_bitmap_usages` | `mcc bitmaps: invalid bitmap reference in shader or HUD` | `weapon HUD %s elements block is outside loaded data` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `pixel packing failed for the normalized hardware layout` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `dimensions %d x %d x %d exceed supported positive limits (16384 x 16384 x 512)` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `unsupported bitmap type %d (expected 0=2D, 1=3D, 2=cubemap)` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `mipmap count %d is outside 0..13` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `pixel payload size %ld must be positive` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `unsupported pixel format %d` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `nonlinear texture %d x %d exceeds the 8192-pixel dimension limit` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `dimensions %d x %d x %d are invalid for type %d (3D maximum 512 x 512 x 256; other types require depth 1)` |
| `mcc_texture_prepare` | `mcc bitmap: tag %08lx has conflicting pixel-channel uses; then cannot convert texture %lu of tag %08lx` | `conflicting pixel-channel uses (mask %u)` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `pixel payload is too short: %lu bytes available, %llu required for format %d, %d x %d x %d, %d mipmaps` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `texture exceeds the 128 MiB allocation limit (source %llu bytes, decoded %llu bytes)` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `non-power-of-two texture needs %llu decoded bytes; limit is 128 MiB` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `bitmap tag header is outside loaded data` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `unsupported lightmap layout: usage %d, flags 0x%04x, format %d, type %d, dimensions %d x %d x %d, mipmaps %d, bytes %ld; bit 12 requires a tightly packed RGB565 lightmap` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `format/type dimension or mipmap constraint` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `bitmap signature is not bitm` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `unsupported normalized bitmap flags` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `mipmap count exceeds the texture dimensions` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `normalized descriptor failed bitmap validation (%s): signature %08lx, flags 0x%04x, format %d, type %d, dimensions %d x %d x %d, mipmaps %d` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `nonlinear texture dimensions must be powers of two` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `linear layout requires a 2D uncompressed texture with no mipmaps` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `hardware texture storage limit exceeded (hardware %lu, allocation %lu, tag stream %lu, total %lu bytes)` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `external bitmap resource name is outside loaded data or exceeds 255 characters` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `external pixel payload could not be read or matched in the selected MCC bitmaps.map; check the package's matching resources` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `embedded pixel payload could not be read from the map` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `not enough memory for texture conversion` |
| `mcc_texture_prepare` | `mcc bitmaps: cannot convert texture %lu of tag %08lx; map refused` | `%s (pixel offset 0x%08lx, source bytes %llu, allocation %lu)` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `not enough memory for bitmap conversion state` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `not enough tag memory for expanded bitmap tag table` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `not enough memory for bitmap copy table` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `not enough memory for bitmap descriptor table` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `bitmap header or bitmaps block is outside loaded data` |
| `mcc_bitmaps_prepare` | `(No specific reason; generic texture-conversion refusal.)` | `bitmap count exceeds the count validated in the original map` |
## Sound conversion

Locations: sound header or `pitch_ranges[i] 'name'.permutations[j] 'name'`. The final sound error appends codec number and name, original payload offset and byte count. Decode/store failures also print `mcc sound payload: codec %d, %d channels, %lu Hz, offset 0x%08lx, %ld bytes, %s, decoder error %d`. Supported codec labels are `16-bit PCM`, `Xbox ADPCM`, `Ogg Vorbis`; codec 2 is `IMA ADPCM (unsupported)` and other values are `unknown`.

[Implementation](../port/linux/game/mcc_audio.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `PCM byte count is not a whole 16-bit sample frame` |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Xbox ADPCM byte count is not a whole 36-byte block per channel` |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `unsupported sound codec (supported: 0=16-bit PCM, 1=Xbox ADPCM, 3=Ogg Vorbis)` |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `decoded frame count is zero or exceeds 16777216 frames` |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `not enough memory for decoded sound samples` |
| `mcc_pcm_open` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `invalid Xbox ADPCM block header (step index exceeds 88 or reserved byte is nonzero)` |
| `mcc_audio_reserve` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: ADPCM encoding, allocation, or 256 MiB per-sound or 1 GiB total audio limit; map refused` | `audio storage limit exceeded (less than 256 MiB per sound; 1 GiB total)` |
| `mcc_audio_reserve` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: ADPCM encoding, allocation, or 256 MiB per-sound or 1 GiB total audio limit; map refused` | `not enough memory to grow the converted sound buffer` |
| `mcc_audio_store` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: ADPCM encoding, allocation, or 256 MiB per-sound or 1 GiB total audio limit; map refused` | `resampled sound exceeds the 16777216-frame limit or is empty` |
| `mcc_audio_prepare` | `(No specific reason; generic sound-conversion refusal.)` | `map size or tag count exceeds sound converter limits` |
| `mcc_audio_prepare` | `(No specific reason; generic sound-conversion refusal.)` | `not enough memory for sound conversion state` |
| `mcc_audio_prepare` | `(No specific reason; generic sound-conversion refusal.)` | `not enough memory for sound storage table` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: sound header; map refused` | `sound header is outside loaded tag data` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: sound header; map refused` | `unsupported encoding %d (expected 0=mono or 1=stereo)` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: sound header; map refused` | `unsupported sample_rate %d (expected -1=unspecified, 0=22050 Hz or 1=44100 Hz)` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: pitch range; map refused` | `pitch range is outside loaded tag data` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: permutation header; map refused` | `permutation header is outside loaded tag data` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: permutation header; map refused` | `sample payload size %ld is outside 0..67108864 bytes` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: nonempty sound has unspecified sample rate; map refused` | `nonempty sound has unspecified sample rate` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: input allocation; map refused` | `not enough memory to read the source sound payload` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: sample read; map refused` | `embedded sound payload could not be read from the map` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: external samples (selected MCC sounds.map data does not match); map refused` | `external samples (selected MCC sounds.map data does not match)` |
| `mcc_audio_prepare` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: %s; map refused` | `%s; codec %ld (%s), payload offset 0x%08lx, %ld bytes` |
## Ogg/Vorbis reasons

These reasons replace `codec decode or sample format` in the owning sound-permutation error above. The decoder error number is supplementary; the text identifies the failed framing, setup, packet or frame-count check.

[Implementation](../port/linux/game/mcc_vorbis.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg contains data after the end-of-stream page` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg page header is truncated` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg page has an invalid signature, version or flags` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg segment table is truncated` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg page payload is truncated` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg packet continuation flags disagree between pages` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg first page lacks the beginning-of-stream flag` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg page stream serial, sequence or beginning flag is inconsistent` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg page CRC checksum does not match its contents` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg granule position is too large or moves backwards` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg end-of-stream page ends mid-packet or has an unsupported granule` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `empty Ogg terminal page has no preceding packet` |
| `mcc_ogg_validate` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg end-of-stream page is missing` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg payload is empty or exceeds the decoder's input limit` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Ogg declared frame count is zero or exceeds the decoded frame/allocation limit` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `not enough memory to normalize an empty Ogg terminal page` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis setup headers were rejected by the decoder` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `unsupported Vorbis floor/residue codebook layout` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis requires 1 or 2 channels and a sample rate of 8000..192000 Hz` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `not enough memory for decoded Vorbis samples` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis first audio packet could not be decoded` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis audio packet decode failed` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis decoded frame positions disagree with Ogg page granules` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis final-packet trimming exceeds the permitted block/window boundary` |
| `mcc_vorbis_decode_ex` | `mcc audio: sound tag #%lu '%s', range %ld, permutation %ld, codec %ld: codec decode or sample format; map refused` | `Vorbis decoded fewer frames than the Ogg end-of-stream declaration` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `not enough memory inside the Vorbis decoder` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis stream uses an unsupported floor/codec feature` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis channel count exceeds the decoder limit` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis packet ended before the decoder could read its required data` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis setup packet or block-size declaration is invalid` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis audio packet is invalid` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis decoder could not find the Ogg page signature` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis decoder encountered an unsupported Ogg version` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis decoder encountered an invalid packet continuation flag` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis decoder encountered inconsistent Ogg stream serial numbers` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis identification/header page is invalid` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis packet type is invalid for this decoding stage` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Vorbis decoder could not find the final Ogg page` |
| `mcc_vorbis_decoder_reason` | `codec decode or sample format` | `Ogg Skeleton streams are unsupported by the Vorbis decoder` |

When the decoder supplies a recognized error code, its specific reason above replaces
the broader setup/packet-stage text. The numeric code is still logged.

## Model and BSP geometry

Locations: model `geometries[g].parts[p] (shader index n)`; BSP `lightmaps[l].materials[m]`; or the named model/BSP header. Vertex and triangle indices are included in the reason when that element is known. GPU failure means allocation/upload failed; it is not asserted to be bad authoring data.

[Implementation](../port/linux/game/mcc_geometry.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mg_float_vertices` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `%s[%lu].component[%lu] is not finite` |
| `mg_float_vertices` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `%s[%lu].component[%lu]=%g is outside the normal/tangent range -1.005..1.005` |
| `mg_model_tangents` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `triangle_indices[%lu]=%u exceeds vertex count %lu` |
| `mg_model_tangents` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `vertices[%lu].%s[%lu]=%g is non-finite or outside the normal range -1.005..1.005` |
| `mg_model_tangents` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `vertices[%lu].texture_coordinates are not finite` |
| `mg_model_tangents` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `not enough memory to reconstruct model tangent frames` |
| `mg_model_tangents` | `MCC geometry: model vertices, node indices or hardware buffers could not be converted / invalid BSP vertex payload` | `vertices[%lu].normal is too short to reconstruct its tangent frame` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `not enough memory for model vertex/index conversion` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `vertex count %lu must be 1..65535`; `model strip has %lu triangles (supported range 1..%lu)` (current limit: 1,048,576 triangles) |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `unsupported part stream types: triangles %u (expected 1), vertices %u (expected 4)` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `model node count %ld must be 1..64` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `part shader index %u has no valid shader table (count %ld)` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `previous/next part indices %d/%d must be NONE or within %lu parts` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `model stream range is outside the declared data: vertex offset 0x%08lx (%lu bytes), index offset 0x%08lx (%lu bytes)` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `local node palette count %u must be 1..22` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `local node palette[%lu]=%u references a missing node (model has %ld)` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `model vertex or triangle stream could not be read from the map` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `vertices[%lu].node_weights contain non-finite values` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `vertices[%lu].nodes[%d] references %d; valid range is 0..%d` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `model part uses more than 22 distinct bones` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `triangle_indices[%lu] references vertex %u; vertex count is %lu` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `centroid node references a missing local bone` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `could not allocate or upload model vertex buffer` |
| `mg_part` | `MCC geometry: invalid model part descriptor / model vertices, node indices or hardware buffers could not be converted` | `could not allocate or upload model triangle buffer` |
| `mcc_geometry_prepare` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `geometry converter was called with missing or already initialized runtime state` |
| `mcc_geometry_prepare` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `not enough memory for geometry conversion state` |
| `mcc_geometry_prepare` | `MCC geometry: model header lies outside loaded tags` | `model header lies outside loaded tags` |
| `mcc_geometry_prepare` | `MCC geometry: invalid model geometry block` | `invalid model geometry block` |
| `mcc_geometry_prepare` | `MCC geometry: no room for model geometry descriptors` | `no room for model geometry descriptors` |
| `mcc_geometry_prepare` | `MCC geometry: invalid model parts block` | `invalid model parts block` |
| `mcc_geometry_prepare` | `MCC geometry: no room for model part descriptors` | `no room for model part descriptors` |
| `mcc_geometry_prepare` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `not enough memory to track converted model ownership` |
| `mg_bsp_material` | `MCC geometry: invalid BSP material vertex range` | `invalid BSP material vertex range` |
| `mg_bsp_material` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `not enough memory for BSP vertex conversion` |
| `mg_bsp_material` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `BSP vertex payload could not be read from the map` |
| `mg_bsp_material` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `could not allocate or upload BSP vertex buffer` |
| `mg_bsp_material` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `could not allocate or upload BSP lightmap vertex buffer` |
| `mcc_geometry_bsp_load` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `BSP load has no geometry state or output buffer` |
| `mcc_geometry_bsp_load` | `MCC geometry: BSP request differs from validated reference` | `BSP request differs from validated reference` |
| `mcc_geometry_bsp_load` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `BSP could not be read at map offset 0x%08lx (%lu bytes)` |
| `mcc_geometry_bsp_load` | `MCC geometry: invalid BSP lightmaps` | `invalid BSP lightmaps` |
| `mcc_geometry_bsp_load` | `MCC geometry: BSP lightmaps point outside BSP` | `BSP lightmaps point outside BSP` |
| `mcc_geometry_bsp_load` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `material block count exceeds the loaded BSP size` |
| `mcc_geometry_bsp_load` | `(No specific reason, or MCC geometry: BSP material conversion failed.)` | `material block address is outside the loaded BSP` |
## HUD layout and strings

Locations identify the unit/grenade/weapon HUD reference, auxiliary overlay/meter, static/meter/number element, crosshair/screen overlay, HUD globals, or `strings[100] (scoreboard hint)`. Header failures identify the original tag.

[Implementation](../port/linux/game/mcc_hud.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `unicode string list header is outside loaded tag data` |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `unicode strings block is outside loaded tag data` |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `UTF-16 string size %lu must be even and at least 2 bytes` |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `UTF-16 string is misaligned or outside loaded data` |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `scoreboard hint has no terminating UTF-16 NUL` |
| `mcc_hud_multiplayer_text` | `(No per-tag reason; generic map-data conversion refusal.)` | `could not allocate or format the scoreboard hint` |
| `mcc_hud_half_bitmap` | `(No per-tag reason; generic map-data conversion refusal.)` | `bitmap reference %08lx is outside the tag table` |
| `mcc_hud_half_bitmap` | `(No per-tag reason; generic map-data conversion refusal.)` | `reference %08lx does not identify a bitmap tag` |
| `mcc_hud_half_bitmap` | `(No per-tag reason; generic map-data conversion refusal.)` | `referenced bitmap header is outside loaded data` |
| `mcc_hud_items` | `(No per-tag reason; generic map-data conversion refusal.)` | `HUD overlay items block is outside loaded tag data` |
| `mcc_hud_default` | `(No per-tag reason; generic map-data conversion refusal.)` | `cannot add omitted default_weapon_hud: tag count exceeds the signed 16-bit index limit` |
| `mcc_hud_default` | `(No per-tag reason; generic map-data conversion refusal.)` | `not enough tag memory for the omitted default_weapon_hud` |
| `mcc_hud_bitmaps_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `HUD tag header is outside loaded data` |
| `mcc_hud_bitmaps_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `HUD block count or address is outside loaded tag data` |
| `mcc_hud_bitmaps_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `grenade HUD header is outside loaded data` |
| `mcc_hud_bitmaps_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `HUD globals header is outside loaded data` |
| `mcc_hud_bitmaps_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `referenced bitmap header is outside loaded data` |
## Tag metadata, cameras and recordings

Locations include `recorded_animations[i] 'name'`, `cutscene_camera_points[i] 'name'.field_of_view`, HUD block names, or the metadata root. Damage/weapon fields and widget event handlers are named in the reason.

[Implementation](../port/linux/game/mcc_tags.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_scenario_recordings` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recorded_animations block is outside loaded tag data (count %lu)` |
| `mcc_scenario_recordings` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `%s at stream byte %lu (version %u, unit version %u, size %lu)` |
| `mcc_scenario_cameras` | `(No per-tag reason; generic map-data conversion refusal.)` | `cutscene_camera_points block is outside loaded tag data (count %lu)` |
| `mcc_scenario_cameras` | `mcc: cutscene camera #%lu has an invalid field of view` | `field of view %g radians must be zero or between 0.001 and pi` |
| `mcc_hud` | `(No per-tag reason; generic map-data conversion refusal.)` | `HUD placement block is outside loaded tag data (count %lu)` |
| `mcc_tags_prepare` | `mcc: metadata for tag %lu is outside the tag data` | `tag header at 0x%08lx requires %lu bytes outside loaded data` |
| `mcc_tags_prepare` | `mcc: damage tag #%lu has non-finite acceleration component %u` | `acceleration[%u] is not finite (%g)` |
| `mcc_tags_prepare` | `mcc: weapon tag #%lu has unsupported movement penalties (mode %u, forward %g, sideways %g)` | `movement penalties: mode %u must be 0..2; forward %g and sideways %g must be finite and within 0..2` |
| `mcc_tags_prepare` | `(No per-tag reason; generic map-data conversion refusal.)` | `event_handlers block is outside loaded tag data (count %lu)` |
## Recorded-animation control-stream reasons

Each reason is expanded into the recording error as `%s at stream byte %lu (version %u, unit version %u, size %lu)`, with the scenario path and recorded-animation index/name. The byte offset is inside that recording's control stream.

[Implementation](../port/linux/game/mcc_recordings.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording data is outside loaded tags` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording version must be 1..4 and unit control version 0..4` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording initial control state is truncated` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording event header is truncated` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording event type is outside 0..22` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording event payload is truncated` |
| `mcc_recording_walk` | `mcc: recorded animation #%lu has an unsupported or damaged control stream` | `recording has no end event` |
## Script conversion

Locations identify the original `.scenario` and `syntax_nodes[i] (source offset n, string offset n)` where possible. Unsupported function/global reporting retains the existing limit of 16 reported names.

[Implementation](../port/linux/game/mcc_scripts.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `scenario header is outside loaded data` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `parameterized scripts have no syntax data` |
| `mcc_scripts_prepare` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `script syntax or string data is outside loaded memory` |
| `mcc_scripts_prepare` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `%s (capacity %d nodes)` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `function call references an invalid function-name node (handle %08lx)` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `script name is outside the string table or has no terminator` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `literal enable cannot be converted to requested result type %d` |
| `mcc_scripts_prepare` | `mcc: %s supports only a literal enable (syntax node %ld)` | `%s supports only a literal enable; computed values and disabling are unsupported` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `literal disable cannot be converted to requested result type %d` |
| `mcc_scripts_prepare` | `mcc: unsupported skull assignment to %s (syntax node %ld); only a literal disable is supported` | `unsupported skull assignment to %s; only a literal disable is supported` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `global %s requires boolean type; received type %d` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `skull global %s requires boolean type; received type %d` |
| `mcc_scripts_prepare` | `mcc: no scenario storage for game_speed_value` | `no scenario storage for global game_speed_value (global/node capacity or allocation exhausted)` |
| `mcc_scripts_prepare` | `mcc: no scenario storage for debug_score` | `no scenario storage for global debug_score (global/node capacity or allocation exhausted)` |
| `mcc_scripts_prepare` | `mcc: no scenario storage for developer_mode` | `no scenario storage for global developer_mode (global/node capacity or allocation exhausted)` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `script strings lack the required 1024-byte console source reserve` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `not enough tag memory to expand scratch-global names` |
| `mcc_scripts_prepare` | `(No specific reason; generic script-preparation refusal.)` | `no scenario storage for scratch global %s` |
| `mcc_scripts_prepare` | `mcc: unsupported script function or global '%s'` | `unsupported script function or global '%s'` |
## Script syntax validation reasons

These reasons appear in the scenario refusal with `(capacity 32767 nodes)`. Invalid graph links and cycles identify the offending syntax node and source/string offsets; header/count failures identify the script data as a whole.

[Implementation](../port/linux/game/mcc_syntax.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax header is missing, truncated or misaligned` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax header signature is not d@t@ or the valid flag is clear` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax node size differs from the required 20-byte layout` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax maximum_count is outside 1..32767` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax count is negative or exceeds maximum_count` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax actual_count is negative or exceeds count` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax first_free_absolute_index is outside 0..maximum_count` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax byte size differs from header plus maximum_count nodes` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `expression child handle is stale or outside the syntax array` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `next_node_index is stale or outside the syntax array` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax expression graph contains a cycle` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax graph traversal exceeds its node count` |
| `mcc_syntax_graph_check` | `mcc: invalid script syntax header or graph (MCC capacity: %d nodes)` | `syntax actual_count differs from the number of live nodes` |
## Script parameters and scope

Locations identify `scripts[i] 'name'`, `globals[i] 'name'`, signature arguments, or `syntax_nodes[i].arguments[j]`. Scope failures also include the offending syntax-node index/handle in the reason.

[Implementation](../port/linux/game/mcc_script_parameters.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_parameter_scope` | `mcc: invalid %s at entry #%u, argument #%u` | `root expression handle %08lx is invalid` |
| `mcc_parameter_scope` | `mcc: invalid %s at entry #%u, argument #%u` | `syntax_nodes[%u]: local parameter slot %ld has invalid flags, is outside the signature, or belongs to another script` |
| `mcc_parameter_scope` | `mcc: script #%d parameter node #%u cannot cast type %d to %d` | `syntax_nodes[%u]: parameter type %d cannot cast to type %d` |
| `mcc_parameter_scope` | `mcc: invalid %s at entry #%u, argument #%u` | `syntax_nodes[%u].%s has invalid handle %08lx` |
| `mcc_parameter_scope` | `mcc: invalid %s at entry #%u, argument #%u` | `syntax_nodes[%u]: expression traversal exceeds the node count` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `syntax, script or global table count exceeds its supported limit` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `scripts or globals table is outside loaded tag data` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `only static/stub scripts may have parameters; maximum is 16 parameters` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `parameter records are outside loaded tag data` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `parameter name has no terminator within 32 bytes` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `parameter type is invalid or void` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `local parameter node has no owning script` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `script call index or function-name node handle is invalid` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `script call must name a static/stub script through a primitive function-name node` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `argument node is missing or has a different type from the script parameter` |
| `mcc_parameters_prepare` | `mcc: invalid %s at entry #%u, argument #%u` | `script call has more arguments than its signature` |
## Converted-tag and BSP validation

Previously the report named only a hexadecimal handle and the current leaf field. It now adds the original tag path and complete nested field/block indices. Existing correction policy is unchanged. The original `MCC tag ...: rejected: ...` report is retained with the nested path; the new refusal line carries the same reason. Messages supplied by existing schema callbacks use this same transformation, including their runtime values; the callback checks themselves are unchanged.

[Implementation](../port/linux/game/mcc_tag_validate.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_collision_surface_edges` | `MCC tag %08lx (%s): rejected: misplaced collision start has no complete unique owned edge ring` | `misplaced collision start has no complete unique owned edge ring` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: schema structure is outside loaded data or excessively nested` | `schema structure is outside loaded data or excessively nested` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: schema field leaves its structure` | `schema field leaves its structure` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: object-name count exceeds MCC's 32767-name capacity; names cannot be truncated` | `object-name count exceeds MCC's 32767-name capacity; names cannot be truncated` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: scenery palette exceeds MCC's 32767-entry capacity; entries cannot be truncated` | `scenery palette exceeds MCC's 32767-entry capacity; entries cannot be truncated` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: invalid block count` | `invalid block count` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: no room to isolate interned block` | `no room to isolate interned block` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: MCC requires two to four grenade kinds` | `MCC requires two to four grenade kinds` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: negative data size` | `negative data size` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: file data leaves MCC stream` | `file data leaves MCC stream` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: no room to isolate shared data` | `no room to isolate shared data` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: data overlaps another allocation` | `data overlaps another allocation` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: data leaves loaded bytes and owned geometry` | `data leaves loaded bytes and owned geometry` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: schema callback rejected data` | `schema callback rejected data` |
| `mcc_fields` | `MCC tag %08lx (%s): rejected: invalid schema index target` | `invalid schema index target` |
| `mcc_walk` | `MCC tag %08lx (%s): rejected: invalid schema size` | `invalid schema size` |
| `mcc_walk` | `MCC tag %08lx (%s): rejected: tag traversal exceeds validation budget` | `tag traversal exceeds validation budget` |
| `mcc_walk` | `MCC tag %08lx (%s): rejected: cyclic tag block` | `cyclic tag block` |
| `mcc_walk` | `MCC tag %08lx (%s): rejected: misaligned tag or block element` | `misaligned tag or block element` |
| `mcc_walk` | `MCC tag %08lx (%s): rejected: block nesting exceeds limit` | `block nesting exceeds limit` |
| `mcc_tags_validate` | `(No specific reason; generic converted-map validation refusal.)` | `tag validation requires an aligned loaded tag region within the MCC capacity` |
| `mcc_tags_validate` | `(No specific reason; generic converted-map validation refusal.)` | `not enough memory for tag validation ownership tracking` |
| `mcc_tags_validate` | `MCC tag %08lx (%s): rejected: invalid tag header or instance table` | `invalid tag header or instance table` |
| `mcc_tags_validate` | `(No specific reason; generic converted-map validation refusal.)` | `not enough memory for tag validation root/alias tables` |
| `mcc_tags_validate` | `MCC tag %08lx (%s): rejected: invalid tag handle or unsupported tag group` | `invalid tag handle or unsupported tag group` |
| `mcc_tags_validate` | `MCC tag %08lx (%s): rejected: tag root outside cache` | `tag root outside cache` |
| `mcc_tags_validate` | `MCC tag %08lx (%s): rejected: tag root overlaps another allocation` | `tag root overlaps another allocation` |
| `mcc_tags_validate` | `MCC tag %08lx (%s): rejected: scenario handle does not identify a scenario` | `scenario handle does not identify a scenario` |
| `mcc_bsp_validate` | `(No specific reason; generic converted-map validation refusal.)` | `BSP validation state is missing, or the loaded BSP range is invalid/misaligned` |
| `mcc_bsp_validate` | `(No specific reason; generic converted-map validation refusal.)` | `BSP allocation overlaps loaded tag data` |
| `mcc_bsp_validate` | `(No specific reason; generic converted-map validation refusal.)` | `BSP handle is stale, has the wrong tag group, or has no validation schema` |
| `mcc_bsp_validate` | `MCC tag %08lx (%s): rejected: invalid BSP header or root` | `invalid BSP header or root` |
## Compiled-map reader

Before: `mcc: <coarse status> (tag <index>, location <hex>)`, or only a generic preflight refusal. After: `mcc: '<map>', <source tag path or map header>: <detail>; map refused (tag index <n>, reader location 0x<hex>)`. Asset/BSP/script details are listed below. Reader locations are compiled-map addresses/offsets, not editor indices. Unknown tag groups are shown as `[group xxxx]`. The standalone `mcc_cache_report` tool also prints `problem_path` and `problem_detail`.

[Implementation](../port/linux/game/mcc_cache_format.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `BSP file range or linked address is invalid (offset 0x%08lx, size %lu, address 0x%08lx)` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `not enough memory to inspect BSP (%lu bytes)` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `BSP could not be read at file offset 0x%08lx (%lu bytes)` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `BSP header signature or vertex file range is invalid` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `BSP root is outside its loaded data` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `lightmaps count %lu exceeds BSP size` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `lightmaps block is outside BSP data` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `lightmaps[%lu].materials count %lu exceeds BSP size` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `lightmaps[%lu].materials block is outside BSP data` |
| `mcc_check_bsp` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `lightmaps[%lu].materials[%lu]: vertex ranges leave the stream or lightmap vertex count %lu differs from rendered count %lu` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `bitmap header at 0x%08lx is outside tag data` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `bitmaps block (count %lu) is outside tag data` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `bitmaps[%lu]: invalid dimensions %u x %u x %u or unsupported format %u (maximum 18)` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `bitmaps[%lu].pixels: offset 0x%08lx and size %lu leave the map file` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `sound header at 0x%08lx is outside tag data` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `pitch_ranges block (count %lu) is outside tag data` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `pitch_ranges[%lu] '%.32s'.permutations: block count %lu is outside tag data` |
| `mcc_check_assets` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `pitch_ranges[%lu] '%.32s'.permutations[%lu] '%.32s'.samples: offset 0x%08lx and size %lu leave the map file` |
| `mcc_cache_load` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `hs_syntax_data has an invalid size/range, or hs_scripts/hs_globals blocks leave tag data (syntax bytes %lu)` |
| `mcc_cache_load` | `mcc: %s (tag %lu, location %08lx), with only the coarse cache status` | `structure_bsps: count %lu must be 1..%u and the reference block must lie inside tag data` |
## Script-thread setup

Location: the source scenario, `script thread storage`. Other map-opening, address-window, identity and package-checksum failures already name the map and retain their existing explanations.

[Implementation](../port/linux/game/mcc_cache.c).

| Check/function | Before | After (reason template) |
| --- | --- | --- |
| `mcc_cache_tags_load` | `mcc: refused %s: Not enough memory to prepare its scripts. Close other applications and try again.` | `not enough memory to allocate script thread state` |
The old supplemental bitmap messages for storage exhaustion, out-of-memory and
external-resource mismatch remain available alongside the new source-tag refusal:
`mcc bitmap: storage limit (used %lu, requested %lu)`,
`mcc bitmap: out of memory for tag %08lx (stored %lu, texture %lu bytes)`, and
`mcc bitmap: external '%s' does not match the selected MCC bitmaps.map`.
The old extra numeric model-part, camera, recording, damage and weapon failure
lines are replaced by the corresponding source-path diagnostic in the tables.

## Resource selection

These failures occur before a tag has been converted. Candidate files that do
not match are still skipped during selection; a refusal is logged only when the
required authoritative file fails or no complete matching file is available.

| Before | After |
| --- | --- |
| `mcc resources: %s does not satisfy every external reference in %s` | `mcc resources: <selected resource path> required by <map>: '<tag>.bitmap', bitmaps[i]: resource '<name>_<i>' (offset 0x<hex>, <bytes> bytes) is missing, ambiguous or outside its matching resource entry; map refused` |
| Same generic authoritative-file error, for sound resources | `mcc resources: <selected resource path> required by <map>: '<tag>.sound', pitch_ranges[i].permutations[j]: resource '<name>__<i>__<j>' (offset 0x<hex>, <bytes> bytes) is missing, ambiguous or outside its matching resource entry; map refused` |
| `mcc resources: no complete matching %s for %s; check the package and MCC shared folders` | The same file/map summary plus the first unsatisfied bitmap/sound reference in the formats above. If no candidate exists, the first required reference is reported. |
| Generic resource mismatch for an overlong tag path | `resource tag path exceeds 255 characters: <first 255 characters>...`, within the resource refusal above. |

The resource lookup API does not distinguish missing, ambiguous and out-of-range
entries; the message names those possibilities accurately and supplies the exact
reference to inspect. It never guesses which candidate file an author intended.
Existing resource open/header/read, duplicate-package and fingerprint errors
already name the file/package and are unchanged.

## Reader statuses that retain their existing reason

The reader's remaining status reasons are unchanged, but are now carried by the
map/path-aware reader refusal format above. Before they were printed through
`mcc: %s (tag %lu, location %08lx)` or reduced to preflight feedback:

- `invalid reader argument`
- `map read failed`
- `invalid MCC cache header`
- `not a Halo 1 MCC version-13 cache`
- `invalid MCC map file length`
- `unterminated MCC header string`
- `MCC map is not singleplayer or multiplayer`
- `invalid MCC tag data range`
- `insufficient memory for MCC tags`
- `invalid MCC tag index`
- `invalid MCC tag entry`
- `invalid MCC scenario`
- `invalid MCC model data range`
- `invalid MCC BSP or vertex range`
- `invalid MCC bitmap data`
- `invalid MCC sound data`
- `invalid MCC script data`
- `external indexed MCC tags require a resource provider`

A damaged header/index may prevent safe tag-name lookup. In that case the reader
reports the map/header or `(tag path unavailable)` and keeps the index/location.
Pre-existing informational conversion totals and recoverable correction/warning
messages are outside this refusal catalogue.

## Implementation and checks

Read [mcc_diagnostics.c](../port/linux/game/mcc_diagnostics.c) first for bounded
identity, formatting and preservation of player prompts, then the relevant converter
linked above. [mcc_runtime.h](../port/linux/game/mcc_runtime.h) owns the state;
[mcc_cache.c](../port/linux/game/mcc_cache.c) captures scenario identity and clears
it on unload. [mcc_tag_validate.c](../port/linux/game/mcc_tag_validate.c) builds
nested validation paths and resolves bitmap copies back to their source handle.
No shared-engine hooks or Xbox/Custom Edition loading changes are needed.

The focused regression tests exercise the production diagnostic formatter, cache
reader, geometry converter and nested validator with synthetic malformed inputs.
They check source paths, model source classes, scenario renaming, bounded/invalid
names, preservation of player prompts, nested validation, geometry elements and
cleanup. These tests do not cover every converter or establish that a particular
retail/custom map loads or renders correctly in the game.

The established player prompts remain unchanged: for example, `MCC <map>: Its textures could not be converted. See debug.txt for the failing texture check.` The compact dialog retains its existing length limits and specific resource/script explanations. Detailed tag paths, element locations and reasons are logged separately; they do not replace the player explanation.

### Additional checks on the current testing branch

These retain testing's existing validation and add the same original-tag context.

| Check | Before | After (reason following tag and field) |
| --- | --- | --- |
| Bitmap descriptor count | Generic invalid bitmap block | `bitmap count exceeds the number of descriptors that can fit in tag data` |
| BSP material ownership allocation | No specific reason | `not enough memory for BSP material ownership runs` |
| Overlapping BSP material runs | Generic BSP material conversion failure | `material block partially overlaps another lightmap material block`, with original `lightmaps[index].materials` |
| HUD ownership planning | Generic allocation refusal or `mcc hud: tag #... root is outside loaded data` | `HUD ownership tag count must be 1..65535`, `not enough memory for HUD ownership frames`, `HUD root is outside loaded tag data`, or the existing allocation reason, now with original tag context where applicable |
| HUD block ownership | `mcc hud: tag #... block +... has an invalid range or exceeds the traversal budget` | Original HUD tag and named block: `HUD block count or address is outside loaded tag data`, `HUD block exceeds the ownership traversal budget of %lu records`, or `not enough memory or ownership records to track the HUD block` |
| HUD block isolation | `mcc hud: no room to isolate a %lu-byte HUD block` | Original HUD tag and named block: `HUD block isolation exhausted runtime tag space` |

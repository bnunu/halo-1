# MCC support reading guide

MCC compatibility uses independent modules and the `mcc_maps` namespace.
Xbox and Custom Edition retain their existing loaders and resource roots.

## Scope and provenance

The implementation is in this repository (`port/linux/game/mcc_*.c`,
`port/linux/src/mcc_*.c`, and the platforms' import code). These documents
came with it from the source branch it was developed on,
[GET-YOUR-MCC-HERE at 1db929c5](https://github.com/chestahh/GulchCE/tree/1db929c5e34101a45f644436e031016b5584c7bf).
That branch has more tests than were brought here; links to tests that are
not in this repository point to its commits. Its logs keep their original
branch names, protocol numbers and test commands. MCC maps play with OpenCE's
network version (`HALO_PORT_NETWORK_VERSION`).

## Reading order

- [Loading, resources and host sharing](maps_and_resources.md)
- [Console syntax and checkpoint lifetime](console_checkpoints.md)
- [Map formats, installation and compatibility evidence](../mcc_maps.md)
- [Stock-menu package previews and completion](mcc_level_previews.md)
- [Browser imports, storage ownership and recovery](browser_imports.md)
- [Host-only package sharing and protocol agreement](../../port/linux/NETCODE.md#mcc-package-sharing)

The format reference separates inspected structures from inferred behavior.
The validation record distinguishes compilation, automated checks and bounded
runtime observations; it does not claim complete campaign playthroughs.

## Optional input prompts

[Keyboard prompts and PC-menu glyphs](keyboard_prompts.md) describes the opt-in
renderer and its MCC-owned pause footer adapters. It also records the unchanged
controller, Xbox-menu and disabled-feature paths.

## Shared audio processing

[Xbox audio processing](xbox_audio.md) covers measured HRTF, reverb, resampling,
levels and their limitations across all map families.

## Optional shield presentation

[Arm shields and the visible overshield](shield_presentation.md) explains the
first-person displacement limit, remaining-layer colors, retained Shield Color
and Shield Glow, and presentation state at map boundaries.

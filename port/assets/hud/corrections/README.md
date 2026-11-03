# Supplied HUD corrections

The project owner supplied these two images from `highreshudcorrections` on
26 October 2026. `source-sha256.json` records the original filenames and hashes;
the retained PNGs are unchanged. This records their supplied provenance, not an
independent identification of their original artist.

The owner explicitly requested using these pictures directly, without tracing,
redrawing, font substitution or geometric reconstruction. `tools/hud_corrections.py`
packs the rocket, sniper reticle, 2x and 10x pictures into the existing texture
cells. Black becomes transparent; grayscale edges retain their antialiasing.
Resizing is proportional. The reticle and the Xbox 2x and 10x are sized to
the stock artwork's visible glyphs, not to their padded sprite cells. The game
tints the resulting monochrome coverage.
CE's existing 8x artwork remains untouched, as explicitly requested.

Run `python tools/hud_corrections.py` from the repository root. It requires
Pillow, NumPy and SciPy. These supplied pictures never go through an SVG renderer.

See [the runtime walkthrough](../../../../docs/reading/hud_corrections.md) for
stock identification, texture ownership and verification limits.

# No op BSS pragma cleanup

The three existing section-directive blocks can be removed without changing
the compiled program in the stock January build. This is zero-credit cleanup:
every complete object, function verdict and progress total remains unchanged.
Owner approval is conditional on preserving functionality and byte matching.

Main base: `73677cf26df696770a58e1838dc8ebc1cba20655`.
Exact-pilots base: `538efa0177ec385c13f1a7eed973fd0f1bdf1023`.
These bases differ only in README.md; their separate README contents are kept.

## Scope

Remove the opening and closing `#pragma bss_seg` lines in:

- `source/math/random_math.c`, around static `random_math_globals`.
- `source/objects/object_types.c`, around static `processed_bsp_flags`.
- `source/text/unicode.c`, around static `bss_004c1a08`.

Also remove object_types.c's claim that its static definition would otherwise
be emitted as COMMON. The measured object disproves that claim: without the
directive, its storage, section and offsets are identical.

The three definitions keep their types, names, initialisers and position.
No function body, assertion, header, atlas, manifest, compiler option,
provider, Matching status, park, scorer or vendor source is changed. Other
section directives and packed structures remain outside this packet.
The source diff is three files and seven deleted lines, retaining CRLF.

## Verification

The removals passed individually and combined in scratch. Fresh clean
whole-board builds at the actual landing directory also establish:

- All 621 rebuilt and 833 split objects are identical after excluding only
  the four-byte COFF compilation timestamp. No local-label or debug-record
  exclusion is needed for the same-directory before/after comparison.
- Consequently code, data, relocations, section flags, symbol/storage
  records and provider-selection attributes are unchanged in full, including
  uncredited sections. No provider or helper copy is added or removed.
- All 8,252 strict target rows are identical: 7,656 exact rows across Halo,
  vendor and helpers; zero gains, zero losses and no residual movement.
- The Ninja all_source command stream is identical; SHA-256:
  `2712ffc0317c1f7382cbfa8274a65199f637e958cd662c6b71da4f4d9d3f9058`.
- The tools suite passes before and after: 1,663 tests, five skips and 159
  passing subtests. The test suite and production comparators are unchanged.
- Separate /W3 compilations cover 612 CL units, with no failures and 4,091
  warnings on each side. Diagnostic content and counts are identical; physical
  source line positions move following the seven source-line deletions.
- Admission reports are identical: nine candidates, none contradicted,
  rejected or revoked. The same 26 inherited fake-scan leads remain.
- All 59 parks validate. COMMON credit stays at 93 records and +661,012 bytes.

Halo totals stay at 7,483 / 7,573 functions, 1,603,071 / 1,770,166 meaningful
code bytes, 3,311,591 / 3,922,163 data bytes and 396 / 468 Matching objects.
Whitespace checks pass; no matching or functionality gain is claimed.

Functionality preservation is established by identical complete compiler
outputs and linker inputs for this build, not by a game-launch test or a claim
about other compiler/platform configurations. Reference executables, SDK and
private symbol assets are not executed or published.

Private receipts remain under `scratch/pragma_cleanup_20261009` in the landing
checkout; the individual controls remain in the separate scratch experiment.
Publication uses the same packet on both repository branches, with fresh
verification and fast-forward-only pushes. Canonical's unrelated README edits
and research files are preserved when it is advanced to the exact-pilots tip.

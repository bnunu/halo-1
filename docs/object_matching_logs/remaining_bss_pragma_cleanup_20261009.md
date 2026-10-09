# Remaining BSS pragma cleanup

The two remaining section-directive blocks are replaced by ordinary explicit
zero initialization of their external globals. Function matching, code and
data credit, warnings, parks and admission statuses remain unchanged. The
rasterizer's two BSS offsets now agree with January and its object audit passes.

Exact-pilots base: `cbbcb3d66e2ae6b66ddf8aabbd819346074ec9aa`.
Halo-1 main base: `14ec9fc4a294b6b881362c99c76192e3f414cd89`.
Their separate README contents are preserved. Owner approval requires no loss
of byte matching or functionality in the tested build.

## Source scope

In material_effects.c, remove the pragma pair and write
`boolean debug_material_effects = FALSE;`. In rasterizer.c, remove its pair and
write `real_argb_color *global_rasterizer_model_ambient_reflection_tint = NULL;`.
The adjacent `static long bss_004662ec;` stays uninitialized and unchanged.

Both public globals are defined in January's own object BSS, not in the COMMON
pool. Explicit zero initialization preserves that ownership without a section
directive; the spelling is inferred, not recovered source. No initializer is
added to the file static merely to select a declaration-order layout.

No function body, type, name, header, atlas, manifest, compiler flag, scorer,
Matching status, park, provider or vendor source changes. The source diff is
two files, two inserted lines and six deleted lines. Existing CRLF is retained.

## Compiled output

The replacements pass separately and together in scratch. Fresh full clean
builds at the landing directory repeat the combined comparison:

- Material-effects is identical as a complete object, excluding only the COFF
  compilation timestamp. No helper or surplus section changes.
- Of 621 rebuilt and 833 split objects, only rasterizer.obj changes.
- Rasterizer's 8-byte BSS moves from section 8 to section 3; old sections 3-7
  shift to 4-8. Symbol indexes and relocation indexes in three functions follow
  that permutation. It is not raw-object equality.
- All 226 section payloads, flags, sizes, alignment and named relocation
  targets, offsets, types and addends are unchanged. Symbol and auxiliary
  records are unchanged apart from the section/index bookkeeping and two
  verified BSS offsets: `_bss_004662ec` 4 to 0, and the external tint pointer
  0 to 4. Storage classes remain static 3 and external 2 respectively.
- The corrected offsets match January. Whole-object audits pass for both
  units; rasterizer improves from FAIL with two symbol-offset differences.
  Selected-provider links pass in both input orders for both units.

The scratch test's initial raw prediction omitted the section/index permutation
and stopped. That failed receipt is retained. A separate read-only diagnostic
then verified the exact permutation and every payload, named reference and
symbol attribute. No production comparator or credit rule was modified.

## Regression checks

All 8,252 strict target rows, including residual fingerprints, are identical:
zero exact gains or losses. The Ninja all_source command stream is unchanged.
The tools suite passes 1,663 tests, with five skips and 159 passing subtests.
The separate W3 sweep covers 612 CL units with zero failures and 4,091 warnings
before and after; only deleted-source-line positions may differ.

Admission reports are identical and the same 26 inherited fake-scan leads
remain. All 59 parks validate. COMMON credit stays at 93 records and 661,012
bytes. Halo totals remain 7,483 of 7,573 functions, 1,603,071 of 1,770,166 code
bytes, 3,311,591 of 3,922,163 data bytes and 396 of 468 Matching objects.

Functionality preservation rests on unchanged emitted logic, data contents,
zero initialization, symbol linkage and named references in the stock January
build. This is not a game-launch test, a completed matching executable link or
a guarantee for other compiler configurations. Supplied game executables are
not run, and private SDK assets are not published.

Private scratch and landing receipts retain the failed prediction, individual
controls and fresh gates. Publication keeps each branch's README and preserves
canonical's unrelated README edits and research files.

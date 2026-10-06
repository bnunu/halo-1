# Upstream review and byte-inert cinematic cleanup — 2026-10-06

Canonical base: `fccf773db3c0adcf55f110eda5034b2c0aac4987`.
Donor: [punpckhdq/halo](https://github.com/punpckhdq/halo/tree/bc4507c1e330037fa49f9b95773f0482972ff8f9),
`bc4507c1e330037fa49f9b95773f0482972ff8f9` (cutscene #67 and the
recent models, scenario, shaders, sound, camera and network packets).

## Independent comparison

The donor's 468 source outputs were rebuilt with the stock local XDK compiler.
For the initial donor census only, the donor's own include/PCH context was used.
Our frozen strict COFF comparator then compared those objects against our own
January split, not a donor-generated split or donor progress score. Canonical
was separately rebuilt with its existing flags, headers and pinned tools.

Nine donor functions are strict exact against January and not exact in this
canonical base. Their sizes below are padded section sizes, not credited gains.

| Function | Padded bytes | Disposition |
| --- | ---: | --- |
| `_ai_debug_render_actor` | 24,976 | The donor uses the held two-local `cross_product3d` header body. No DD2 or header change imported. |
| `_poll_endpoint_set` | 560 | Immediately tested staging boolean; the existing AB #9 / Q-EU2 hold remains. |
| `_connect_async_thread_proc@4` | 304 | Unassigned thread pointer on the mutex-failure path; no new original-defect admission. |
| `_observer_update_positions` | 1,568 | Includes unused locals and held flattened-array/vector pointer views. No import. |
| `_cinematic_render` | 1,280 | Coherent body tested under canonical headers; it remains the existing residual. See below. |
| `_collision_surface_test_sphere` | 880 | Depends on a `fast_distance_squared3d` shared-header SSE helper absent from canonical. Existing reduction/helper hold remains; no helper or header import. |
| `_bsp3d_test_pill_recursive` | 1,504 | The specific negated-argument parenthesis hold remains. |
| `_bitmap_draw_string` | 304 | Donor dereferences `bounds` in the `!bounds` arm. Existing original-defect hold remains. |
| `_connected_geometry_find_or_add_vertex` | 192 | Donor uses the invented `POINT_COORDINATES_EQUAL` macro, including whole-value argument parentheses. Existing source-policy hold remains. |
| **Total, not an approved delta** | **31,568** | **No new function credited by this reconciliation.** |

All source units were available. Seventy-seven nonexact canonical function
symbols were absent from the donor objects, so no claim of donor completeness
is made. Vendor work is not counted as Halo progress. The recent upstream
object-status and scoring changes are not automatically imported.

## Cinematic port and retained cleanup

The park permits research on a natural same-compiler source donor. A prediction
was recorded before compiling the complete meaningful donor statement shape,
adapted only to canonical type/member names, existing narrowing conversions,
and the unsigned shadow-alpha shift. No header, declaration-count, function
order, PCH or flag experiment was used to force the result.

An initial adaptation mistakenly treated donor `MASK(24)` as available here;
canonical has no such macro. Replacing that with the existing `0x00FFFFFF`
literal removes the accidental implicit call. The corrected port reproduces
canonical's *existing* 1,280-byte / 57-relocation residual section exactly:
`1a9d49fc4eace03cb02268a6434f252b45496477fac007eba679c72e4b5c8d1c`.
January's hash remains
`a89dcee38e6a239615cde9f2d1f3b2fc577ced717e620de08a6397f74a0e6288`.
The remaining donor-context dependency was not isolated and is not claimed as
recovered January source. The broad body replacement did not land.

Only the already approved OQ-FI6 B1 cleanup is retained: compute shadow alpha
inside `rasterizer_text_set_shadow_color`, instead of naming `shadow_alpha`.
The later `/Od` and demo line-table evidence for this direct expression was
previously recorded in the lane. Existing conversions, arithmetic, macros,
types and control flow are preserved. This is a zero-credit fidelity cleanup,
not a match, a recovered debit or an object admission.

The cinematic park, its reopen criterion and the existing uncredited
`_fast_ftol` helper copy remain unchanged. No Matching status is flipped.

## Verification contract

Before publication: clean full build; strict whole-board snapshot (zero gains,
zero losses); normalized fingerprints of all 621 built objects identical;
owned data and provider inventory unchanged; common-pool credit unchanged;
parks and admission audit unchanged; tools tests and fake scan; unchanged
whole-board /W3 and cinematic /W4 warning multisets. Compiler, scorer,
verifiers, pins, allowlists, atlas, headers and denominators are untouched.

Private binary inputs, scratch objects, donor checkout and receipts are not
published. This log records the unsuccessful exact port and the exclusions so
that upstream progress is not mistaken for additive canonical byte credit.

### Fresh result

The full before/after builds and gates passed. All 621 normalized object
fingerprints are identical, including provider attributes and section order.
Strict snapshot: 8,252 target functions, 7,656 exact on both sides (includes
vendor/helper rows); gains 0, losses 0. Credited Halo progress remains:

- Functions: 7,483 / 7,573.
- Meaningful code: 1,603,071 / 1,770,166 bytes.
- Data: 3,311,591 / 3,922,163 bytes.
- Matching objects: 396 / 468; active parks: 59, stale 0, invalid 0.
- Admission audit: 9 candidates, 0 contradictions, 0 rejections, 0 revocations.
- Tools tests: 1,663 passed, 5 skipped, 159 passing subtests, both before and
  after; fake scan: the same 26 inherited review leads.
- /W3: identical per-unit warning multisets, 4,091 board-wide warnings.
  Cinematic /W4: identical multiset, 67 warnings.

Production compiler inputs, tool hashes and common-pool credit are unchanged.
No runtime executable was launched. Only the approved expression cleanup and
this disclosure are committed.

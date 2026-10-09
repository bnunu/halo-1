# Empty-header and unused-enum cleanup — 2026-10-09

Exact-pilots base: `a71a4ac1350500e344df6b4e9bd950c17302af5b`.
Halo-1 main base: `b43c70168ad7040ec1a663842f5faa7e1a1bc0cd`.
Their source, library, configuration and tooling trees agree; their separate
READMEs are preserved. Owner approval: implement the tested removals and push
to both repositories only without loss of byte matching or functionality.

## Scope and provenance

Remove all 51 currently empty reconstructed headers, the 50 corresponding
project-header list rows, and four now-useless includes. The four includes
become explanatory one-line comments, retaining existing assertion locations.
No remaining declaration, helper, type or owning interface is moved.

Forty-seven removed headers entered in `4bd27e82` ("Push all applicable HCEX
header stubs"); three entered in the initial reconstruction; the memory-pool
header entered in `1ad31494`. The compile/runtime HS headers and memory-pool
header once held prototypes, removed by the approved ownership reconciliation
in `f3aadeab`. Their known later-build filenames are not declared fictitious:
recreate an appropriate header when authentic content is recovered. This is
cleanup of their current empty state, not a new ownership ruling.

Also remove the unused `NUMBER_OF_PIXEL_SHADER_STAGES = 8` enum definitions
from `rasterizer_xbox_environment_fog.c` and
`shader_transparent_generic_preprocessor.c`. They entered in `4ad9bd84` and
`e49fee8f`. Direct use scans and stock VC7 preprocessing show the identifier
occurs only at its definition in each unit. Other repeated enum families are
untouched; no shared owner header or replacement constant is invented.

No code body, symbol atlas, credit manifest, verifier, pin, compiler flag,
library, Matching status or park is changed. No credited gain is claimed.

## Fresh landing checks

Clean full builds before and after, at the actual exact-pilots landing HEAD:

- All 621 compiler commands are identical; command-stream SHA-256:
  `84f787de49aa7664c364d1cd42b44eb53669a00c7209c2be45869d6545ae7512`.
- All 8,252 strict target function rows are identical, including residual
  fingerprints: 7,656 strict-exact rows on each side, including vendor/helper
  rows. Gained 0, lost 0; no function or data re-credit.
- All 833 split objects are identical except COFF timestamps. Of 621 rebuilt
  objects, 619 are identical under that same timestamp-only exclusion.
- The remaining two objects differ only in compiler-local `$L` names:
  `hs_runtime.obj` has 64 renamings; `rasterizer_xbox_environment.obj` has 12.
  Each keeps its symbol-table index, storage, type, section and offset. The
  complete raw objects are identical after masking only timestamps and those
  76 independently verified name fields. No comparator was changed to accept
  this: the frozen strict gate already verifies every function unchanged.
- Thus runtime section bytes, relocation records, provider code/selection
  attributes, owned data, source-owned symbols and storage offsets are
  unchanged. No new provider or helper copy is emitted.
- Admission audit is identical: 9 candidates, 0 contradicted, 0 rejected,
  0 revoked. Fake scan retains the same 26 inherited review leads.
- Tools suite before and after: 1,663 passed, 5 skipped, 159 passing subtests.
- Separate `/W3` compilations: 612 CL units on each side, 0 failures, 4,091
  warnings. Diagnostics are identical; only two physical source positions
  move in the fog unit: C4700 `previous_camera_matrix`, 944 -> 939; C4244
  double-to-real conversion, 1015 -> 1010. No warning is added or silenced.
- Whitespace checks pass. Source files retain the checkout's CRLF convention.

Credited Halo totals are unchanged:

- Functions: 7,483 / 7,573.
- Meaningful code: 1,603,071 / 1,770,166 bytes.
- Data: 3,311,591 / 3,922,163 bytes.
- Matching objects: 396 / 468.
- Parks: 59 valid.
- COMMON credit: 93 records, +661,012 bytes, unchanged.

Private receipts and before/after objects remain local under
`scratch/empty_header_cleanup_20261009`. No private binary, SDK, PDB or scratch
tool is published. No reconstructed executable was launched. Behavior
preservation here is established by unchanged compiled runtime sections and
relocations, not a gameplay smoke test or a promise about other configurations.

The same source/configuration packet is applied to Halo-1 main without changing
its README. Publication must remain fast-forward, with the remote bases
rechecked before each push; any new remote code requires fresh reconciliation.

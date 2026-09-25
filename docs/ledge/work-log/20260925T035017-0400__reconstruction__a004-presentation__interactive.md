DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T03:50:17-04:00
COMPLETED_AT=2026-09-25T04:03:48-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=7e7253a12626deb6f845a03cbb860318b3d39c73
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A004 P8 manual MPEG CALIBRATION region source

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`7e7253a12626deb6f845a03cbb860318b3d39c73`.

The governing Foreman authority was State revision `0061`, which had already
accepted A003 R24 final restored-RFB reveal and activated exactly one
Reconstruction packet:

`A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `ARCHITECTURE_BLOCKER=NONE`;
- ordinary MPEG product activation still deferred;
- DESKTOP CALIBRATION explicitly outside this packet.

The worker consumed:

- Foreman State revision `0061`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted A004 P1 `src/ui/mpeg_calibration.{c,h}` core;
- normalized physical-controller facts in `src/input/controller.h`;
- the A004 presentation/calibration audit;
- frozen H1 MPEG-calibration interaction/render/raster lineage only as behavioral
  evidence.

The worker also read clean-source naming/topology/tooling policy before adding
the new maintained UI source.

The GitHub-connected Worker seat cannot observe external Pi/local checkout
staging, untracked files or local worktree dirtiness. No local reset, clean,
stash, checkout-over or shell execution is claimed.

## Result

P8 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`40841daec820c8be01ba2fbaa93d954dea43b506`

P8 adds one narrow clean UI-owned manual MPEG CALIBRATION region source:

- `src/ui/mpeg_calibration_manual.h`;
- `src/ui/mpeg_calibration_manual.c`.

It deliberately composes the accepted P1 calibration core instead of creating a
second geometry/state owner.

### Explicit begin; no permanent entry trigger

`pstvnc_mpeg_calibration_manual_begin()` is the only P8 foreground entry seam.

The P8 product implementation contains no controller polling, hold timer or
permanent START+SELECT entry detector. Entry gesture selection and consumption
of the triggering observation remain future Application/orchestration policy.

Focused evidence sends START+SELECT while the source is inactive and proves
that P8 does not begin calibration or consume that sample. Explicit begin is
required.

### Neutral controller mapping

Once P1 owns foreground, P8 maps only normalized
`pstvnc_controller_state_t` facts into the already-accepted P1 semantic
actions:

- D-pad -> base resize;
- R1 + D-pad -> pixel-precision base movement;
- R2 + D-pad -> inner-matte edit;
- L2 + D-pad -> outer/suppression-matte edit;
- START -> reset;
- CROSS -> review / release-repress acceptance;
- CIRCLE -> review back-out or edit cancellation.

P8 does not poll libpad and does not depend on PS2SDK controller numeric
vocabulary.

SELECT and the frozen H1 Controls/Triangle state are not promoted into this
clean region source.

### Foreground consumption and release quarantine

`pstvnc_mpeg_calibration_manual_service_controller()` reports
`consume_controller_state` while the accepted P1 core owns EDIT, REVIEW or
post-exit release quarantine.

This is a fact for a future coordinator. P8 itself does not suspend/resume the
input runtime, neutralize/rebase pointer state, alter OSK routing or acquire RFB
ownership.

Acceptance and cancellation are copied only from the exact P1 transition
effects.

The accepted result contains a value copy of the exact
`pstvnc_mpeg_calibration_region_t`; it is not an alias to mutable calibration
storage.

Focused evidence proves:

- held CROSS after review entry does not accept;
- CROSS release arms acceptance;
- the next fresh CROSS press creates exactly one accepted copied region;
- later mutation of P1 committed state cannot alter the copied result;
- quarantine remains consumed until complete physical release;
- cancellation creates no accepted region and retains consumption until
  quarantine completion.

### Geometry semantics remain P1-owned

P8 does not recalculate calibration geometry.

`pstvnc_mpeg_calibration_manual_prepare_plan()` copies the current P1 candidate
and calls only `pstvnc_mpeg_calibration_resolve_geometry()` for:

- exact base rectangle;
- distinct inner-content rectangle;
- clipped outer/suppression footprint.

Tests compare the plan geometry byte-for-byte with a direct P1 resolution of
the same candidate.

P1 therefore remains sole owner of:

- 16-pixel macroblock alignment for base width/height;
- pixel-precision X/Y placement;
- inner-matte extents;
- outer/suppression extents;
- review/accept/cancel state;
- release quarantine.

### Platform-neutral visible plan and caller-owned CT16 raster

P8 exposes only EDIT and REVIEW visible plan kinds. It does not reconstruct the
frozen H1 Controls screen or its layered adapter/checkpoint state machines.

The plan contains:

- copied candidate region;
- exact P1-resolved geometry;
- canvas dimensions;
- review acceptance-arm fact;
- compact title/help text.

`pstvnc_mpeg_calibration_manual_rasterize()` accepts an immutable caller-owned
frozen CT16 desktop and writes a distinct caller-owned CT16 output surface.

The raster:

1. validates the full plan, capacities and input/output non-overlap before any
   write;
2. copies the frozen desktop into the output;
3. darkens the exact P1 suppression footprint;
4. restores the exact base rectangle from the immutable frozen desktop;
5. paints the inner matte as the complement of P1's resolved inner-content
   rectangle;
6. draws the selector on the exact P1 base edge;
7. uses a small deterministic EDIT/REVIEW marker.

No product geometry is independently derived from the raw region during raster
service.

Focused evidence proves the frozen source buffer remains byte-identical, the
inner-content sample retains its desktop pixel, matte/selector/suppression
samples reflect the resolved P1 meanings, and malformed geometry, insufficient
capacity or overlapping buffers fail closed.

### Deliberate non-activation boundary

P8 produces calibration region data and local caller-owned visual data only.

It does not:

- modify ordinary `src/app.c`;
- modify `src/app_mpeg_run.*`;
- start or retire MPEG;
- alter RFB/P2/session/parser behavior;
- suspend input runtime or acquire physical-controller ownership;
- modify P3/compositor/Platform graphics;
- modify MPEG decoder/worker/runtime;
- modify Transport or Wire protocol;
- modify Pi product source or persist configuration;
- alter AUDIO;
- touch DESKTOP CALIBRATION.

Future automatic or Application-pinned MPEG region sources can therefore
produce the same accepted region value without depending on this manual
controller adapter.

## Dictionary/tooling reconciliation finding

The first repository-authorized no-op reconciliation trigger was:

`58fc8aeb311f8341cbe2b1fafebeb583eec56cdf`

Its workflow `36110312660` correctly showed that P8 behavior/tests/compile/link
were healthy but dictionary/project gates remained incomplete.

Inspection of the canonical
`scripts/reconcile-source-dictionaries.py` contract established that its
explicit mechanically safe target set intentionally excludes `src/ui`.
The tool states that product dictionaries outside that set remain governed by
the canonical validator and are not rewritten merely because they exist.

The Worker did not broaden that tooling contract.

Instead, the exact 177 missing P8 definitions reported by the canonical
long/complete/strict validator were added to the existing directory-owned
`src/ui/SYMBOLS.md` in:

`da4567278901f7649bd8ad7a72068407014d7101`

A second normal reconciliation trigger:

`5530f27ae9371173b4e7a9b795fe6258027dd652`

then proved strict dictionary completeness and generated only the required
portal update:

`a39fd67e5a02ff99d25119b7c1554ba54426d299`

The exact source-identical final verification commit is:

`40841daec820c8be01ba2fbaa93d954dea43b506`

No reconciliation script/tooling implementation was changed.

## Commits

The complete pre-log Worker range is eight commits ahead / zero behind the
starting Foreman authority:

1. `f593c06a16f7901bdee40af4e5f22dc9325078cb` —
   `ui: add P8 manual MPEG calibration region source`;
2. `e0ea50a0e7a2f138e5eb58d409688eb10ad5c2b2` —
   `test(ui): prove P8 manual MPEG calibration source`;
3. `3558071de63fcf96447602934d243f3e4b952f16` —
   `docs: record P8 manual MPEG calibration region source`;
4. `58fc8aeb311f8341cbe2b1fafebeb583eec56cdf` —
   first deterministic dictionary reconciliation trigger, which exposed the
   intentional `src/ui` tooling boundary;
5. `da4567278901f7649bd8ad7a72068407014d7101` —
   `docs(symbols): index P8 manual calibration source`;
6. `5530f27ae9371173b4e7a9b795fe6258027dd652` —
   second deterministic dictionary reconciliation trigger;
7. `a39fd67e5a02ff99d25119b7c1554ba54426d299` —
   `docs(symbols): reconcile current clean definitions`;
8. `40841daec820c8be01ba2fbaa93d954dea43b506` —
   `test: verify final P8 manual MPEG calibration authority`.

No concurrent branch movement or write collision occurred during the Worker
writes.

## Changed paths

The exact compare from starting authority
`7e7253a12626deb6f845a03cbb860318b3d39c73` to final pre-log authority
`40841daec820c8be01ba2fbaa93d954dea43b506` is eight commits ahead / zero
behind and changes only:

- `src/ui/mpeg_calibration_manual.c` — new;
- `src/ui/mpeg_calibration_manual.h` — new;
- `tests/unit/mpeg_calibration_manual_test.c` — new;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `src/ui/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No DESKTOP CALIBRATION file changed.

No ordinary Application, RFB, input-runtime/physical-controller, Display/P3/
compositor/Platform, MPEG execution, Transport, Configuration, Pi product,
AUDIO or Wire protocol implementation changed.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A004-P8-C1 EXPLICIT_BEGIN_WITHOUT_PERMANENT_START_SELECT_TRIGGER` — MET.
  P8 has only an explicit begin API; inactive START+SELECT focused proof does not
  begin or consume calibration.
- `A004-P8-C2 NORMALIZED_CONTROLLER_MAPPING_USES_ONLY_ACCEPTED_MANUAL_ACTIONS`
  — MET. Focused tests prove D-pad resize, R1 move, R2 inner, L2 outer, START
  reset, CROSS review/accept and CIRCLE back/cancel through neutral controller
  facts.
- `A004-P8-C3 FOREGROUND_AND_RELEASE_QUARANTINE_CONSUMPTION_IS_EXPLICIT` —
  MET. P8 reports consumption throughout P1 EDIT/REVIEW/quarantine ownership and
  releases it only on P1's exact quarantine-complete edge.
- `A004-P8-C4 ACCEPTANCE_IS_ONE_SHOT_P1_EDGE_WITH_COPIED_REGION` — MET. Held
  CROSS cannot accept; release + fresh press accepts exactly once and the result
  owns a value copy independent of later P1 mutation.
- `A004-P8-C5 CANCEL_NEVER_ACCEPTS_AND_QUARANTINE_COMPLETES_EXACTLY` — MET.
  Cancel produces no accepted edge/region and retains foreground consumption
  until the all-released sample.
- `A004-P8-C6 VISUAL_PLAN_USES_EXACT_P1_BASE_INNER_SUPPRESSION_MEANINGS` —
  MET. The plan uses P1 resolve_geometry directly and focused proof compares the
  complete geometry value with independent P1 resolution.
- `A004-P8-C7 CALLER_OWNED_RASTER_PRESERVES_FROZEN_RFB_AND_DOES_NOT_DUPLICATE_GEOMETRY`
  — MET. The input desktop remains byte-identical; raster uses only resolved
  plan geometry; invalid capacity/overlap/plan conditions fail before writes.
- `A004-P8-C8 NO_FROZEN_H1_EXPERIMENTAL_UI_STATE_OR_ADAPTER_PROLIFERATION` —
  MET. Clean source contains one manual source around accepted P1, with only
  EDIT/REVIEW visible plan kinds; H1 Controls/foreground/RFB-flow/runtime/
  interaction adapter stacks were not migrated.
- `A004-P8-C9 OUTPUT_IS_REGION_DATA_ONLY_AND_DOES_NOT_ACTIVATE_MPEG` — MET.
  Acceptance returns only the copied P1 region; P8 contains no Application,
  Transport, MPEG-run or Pi activation path.
- `A004-P8-C10 NO_INPUT_RFB_DISPLAY_TRANSPORT_PI_AUDIO_SCOPE_CREEP` — MET.
  Exact final compare touches only the authorized UI source, focused tests,
  documentation and dictionaries.
- `A004-P8-C11 DESKTOP_CALIBRATION_UNTOUCHED_AND_FUTURE_REGION_SOURCES_REMAIN_POSSIBLE`
  — MET. No DESKTOP CALIBRATION source changed and P8's accepted output is the
  existing neutral P1 region value rather than a manual-source-specific runtime
  handle.
- `A004-P8-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact final pre-log workflow `36110790554`, attempt 1, completed SUCCESS
  across host, project, strict dictionary, pinned PS2 compile, link and
  current-source reproducibility.

All twelve P8 criteria are `MET` for independent Foreman review.

## Machine evidence

Early source/test workflows established that the P8 behavior itself compiled
and tested successfully before dictionary maintenance was complete.

At test authority
`e0ea50a0e7a2f138e5eb58d409688eb10ad5c2b2`, workflow
`36110222223` observed:

- host-unit — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- project-check — failure only for missing local dictionary file coverage;
- dictionary-long — failure only for the new P8 dictionary symbols.

Focused host output included:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`.

The first no-op dictionary trigger workflow
`36110312660` confirmed the automatic reconciler's documented `src/ui`
exclusion. It was an integration-maintenance finding, not a product-source
failure.

After manual directory-owned UI indexing and generated portal reconciliation,
exact final pre-log authority:

`40841daec820c8be01ba2fbaa93d954dea43b506`

ran canonical workflow:

`36110790554`

attempt:

`1`

with conclusion:

`SUCCESS`

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected because dictionaries/portal were
  already synchronized.

Observed final host proof includes:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`.

Observed final repository/build proof includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=206 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact final linked identity:

- `ELF_PRISTINE_SHA256=d9fa5af6749f21f4a7e5ab6dc727bebb66c584c37bc487a4c909b96abcdfb549`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`;
- `PT_LOAD_BYTES=500244`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

This PT_LOAD identity is exactly unchanged from Foreman-accepted R24. P8 is not
wired into ordinary product activation, so it changes maintained source/tests/
documentation but not current product loadable bytes.

No new hardware qualification is claimed. The unchanged R24 identity remains
hardware-pending under current Foreman authority.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_P8`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`LOADABLE_BYTES_CHANGED=NO`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO_NEW_CLAIM`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0061`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted A004 P1 calibration core;
- normalized controller fact vocabulary;
- frozen H1 manual-calibration behavior evidence.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no DESKTOP CALIBRATION revision;
- no ordinary MPEG product activation;
- no Application or lower-owner mechanism revision;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No P8 source blocker remains within the authorized packet.

The accepted P1 calibration core and normalized controller fact seam were
sufficient. No cross-domain implementation had to be broadened.

The only integration issue encountered was the canonical mechanical dictionary
reconciler's intentional `src/ui` exclusion. It was resolved through the
directory-owned `src/ui/SYMBOLS.md` authority and normal generated portal,
without changing the tool.

Ordinary MPEG product trigger/service wiring remains deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_P8_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect the P8 source/evidence/log, decide
acceptance, and select any later bounded packet. Reconstruction must not
self-accept P8 or begin ordinary MPEG product activation.

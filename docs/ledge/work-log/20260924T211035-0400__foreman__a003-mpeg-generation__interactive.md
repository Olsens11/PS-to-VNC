DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T21:10:35-04:00
COMPLETED_AT=2026-09-24T21:17:44-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=2f4a03ff38f851b6d9f73ab5f5305e6751e532d5
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Foreman acceptance — R24 accepted; manual MPEG CALIBRATION region source P8 activated

## Objective and recovered authority

Receive the Reconstruction baton after
`A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24`, independently recover live
repository authority, inspect the actual R24 source/test/evidence range, decide
acceptance, then choose exactly one next dependency-correct Reconstruction
packet without beginning product behavior from the Foreman seat.

Truthful shift start:

`STARTED_AT=2026-09-24T21:10:35-04:00`

Live pickup authority:

`2f4a03ff38f851b6d9f73ab5f5305e6751e532d5`

The branch was seven commits ahead / zero behind the previous Foreman closeout
`5b329dd46066602cd95eb1813be226395d3d07a0`.

The governing authorities consumed included:

- Foreman State revision 0060;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- Wire Runtime Decisions revision 0011, especially Q6/Q7;
- Architecture Overlay revision 0007;
- Q1-Q12 Reconciliation revision 0001;
- A004 Calibration Separation Invariant revision 0001;
- accepted corrected R23/R23C retirement authority;
- accepted P2 RFB flow-policy semantics;
- accepted P3 retirement/reveal ownership state;
- accepted compositor synchronized reveal seam;
- accepted P1 MPEG CALIBRATION core;
- accepted A004 P1-P7 source/evidence;
- A006 Application orchestration audit;
- frozen H1 MPEG presentation-calibration lineage at
  `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The newest Reconstruction handoff consumed was:

`docs/ledge/work-log/20260924T205847-0400__reconstruction__a003-mpeg-generation__interactive.md`.

No newer Validation/Continuity/Diagnostics/support commit appeared in the
recovered R24 range.

## Exact R24 worker range

Reconstruction started from:

`5b329dd46066602cd95eb1813be226395d3d07a0`

and landed six pre-log commits:

1. `bbeece1989ab8e89a19c66e0e88ae1e456a7f40c` —
   `app: add exact R24 restored RFB reveal transaction`;
2. `d9e27a1179f9f8b65d409a8f5f0519a9c59d6207` —
   `test(app): prove R24 final restored RFB reveal`;
3. `6032262e7e8b71dcac3fd60832c3ffc16347138a` —
   `docs: record R24 final restored RFB reveal`;
4. `14d0aeb07ffdc4b228f273c2dbc217d6fa01d4be` —
   `tooling(symbols): run deterministic dictionary reconciliation`;
5. `c5e44ab5f418dd3eabf73a23a223919c293ab6e9` —
   `docs(symbols): reconcile current clean definitions`;
6. `d777835c53a0e829e423f76bd0f78097c88e0ffb` —
   `test: verify final R24 restored RFB reveal authority`.

Final pre-log source authority:

`d777835c53a0e829e423f76bd0f78097c88e0ffb`

The next and only following commit was the required immutable Reconstruction
record:

`2f4a03ff38f851b6d9f73ab5f5305e6751e532d5`.

Independent compare proved six commits ahead / zero behind to final source and
exactly one log-only commit afterward.

The complete R24 source range changed only:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No ordinary `src/app.c`, RFB implementation, P3/compositor/Platform
implementation, P7, MPEG worker/backend/runtime, Transport, Configuration, Pi
product source, AUDIO, Input/UI/calibration or protocol source changed.

## Independent R24 source review

R24 closes the trigger-agnostic Application-owned visible handoff without
duplicating lower-owner mechanisms.

Verified behavior:

1. final restoration is admitted only from exact RESTORE_PENDING or an
   already-sealed REVEAL_PENDING retry state;
2. the coordinator re-proves the R23C execution-retired fact set: nonzero exact
   generation, no teardown requirement, R18 closed, R5/R4/P7 retired, START and
   RETIRE facts present, exact completion consumed, producer-done published and
   worker joined;
3. initial RESTORE_PENDING requires exact P3 RETIRING authority;
4. R24 adds one run-scoped `rfb_restoration_presented` fact and the narrow
   `pstvnc_app_mpeg_run_record_restored_rfb_presented()` seam;
5. that fact cannot be recorded until P2 is thawed/publication-enabled, has no
   outstanding request and reports INCREMENTAL, proving the post-thaw FULL debt
   is no longer pending/in flight;
6. the API contract explicitly requires the caller to record the fact only
   after the corresponding authoritative FULL-refreshed RFB desktop has crossed
   the existing successful desktop presentation/upload boundary;
7. protocol freshness by itself therefore does not authorize reveal;
8. exact fresh-presented proof plus P3 RETIRING permits exactly one P3 seal;
9. Application verifies P3 REVEAL_PENDING before recording its own explicit
   REVEAL_PENDING state;
10. final physical handoff invokes only the accepted
    `pstvnc_mpeg_compositor_reveal_retired()` seam;
11. Application never calls Platform reveal or P3 commit directly;
12. compositor PLATFORM_FAILED and SYNC_INVALID remain retryable pre-sync
    outcomes that preserve exact generation/proof/P3 snapshot, do not reseal,
    and do not set teardown-required merely for those results;
13. INVALID, retirement-commit failure, wrong authority and contradictory
    OK/effects fail closed;
14. success requires truthful synchronized + retirement_revealed effects,
    exact P3 RFB_ONLY, no retained P3 snapshot and no MPEG visual ownership;
15. only exact physical success clears current-run/transient state;
16. `last_allocated_generation` survives while `current_generation` returns
    to zero;
17. external P2 remains thawed and the external session media-clock object is
    not reset/mutated;
18. focused evidence proves the same coordinator can allocate generation N+1
    after an external caller re-establishes the accepted frozen-P2 start
    precondition.

No R24 product defect was found.

## R24 acceptance criteria

A003-R24-C1=MET
A003-R24-C2=MET
A003-R24-C3=MET
A003-R24-C4=MET
A003-R24-C5=MET
A003-R24-C6=MET
A003-R24-C7=MET
A003-R24-C8=MET
A003-R24-C9=MET
A003-R24-C10=MET
A003-R24-C11=MET
A003-R24-C12=MET

Overall disposition:

`R24_DISPOSITION=FOREMAN_ACCEPTED`

## Exact R24 machine evidence

Exact final source:

`d777835c53a0e829e423f76bd0f78097c88e0ffb`

GitHub Actions:

`36080636735`, attempt 1 — SUCCESS.

Observed successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile skipped as expected.

Host logs explicitly include:

- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app R15/R16B/R19 tests: PASS`;
- `app_mpeg_run_test: PASS`.

Project/build logs explicitly include:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=204 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked identity:

`ELF_PRISTINE_SHA256=d9fa5af6749f21f4a7e5ab6dc727bebb66c584c37bc487a4c909b96abcdfb549`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`
`PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

Evidence classification:

SOURCE_COMPLETE=YES
HOST_TESTED=PASS
PROJECT_CHECK=PASS
STRICT_DICTIONARIES=PASS
PS2_COMPILE=PASS
PS2_LINK=PASS
CURRENT_SOURCE_REPRODUCIBILITY=PASS
MACHINE_EVIDENCE=GITHUB_ACTIONS
INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO
HARDWARE_PENDING=YES
LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE

No workflow/combined-status record was exposed for the later immutable
Reconstruction-log head during this review, so no log-head CI result is claimed.

## Next dependency analysis

R24 completes the Application MPEG run coordinator, but ordinary product
activation is still not dependency-complete.

Wire Runtime Decisions revision 0011 Q6 defines MPEG activation as an
Application transaction consuming an MPEG region definition. For the first
implementation, manual MPEG CALIBRATION is an acceptable region source, but the
manual UI must not become the architecture definition of MPEG activation.

The accepted A004 P1 source currently supplies only:

- MPEG CALIBRATION value/state ownership;
- base/inner/outer geometry;
- edit/review/accept/cancel semantics;
- held-CROSS review safety;
- release quarantine;
- explicit begin;
- geometry resolution.

P1 deliberately excluded:

- normalized physical-controller adaptation;
- START+SELECT timing/entry policy;
- foreground input-runtime ownership;
- pointer neutralization/rebase;
- RFB freeze/thaw composition;
- visible calibration render/presentation;
- Application activation;
- Pi persistence.

Current ordinary `src/app.c` likewise has no MPEG CALIBRATION or
`app_mpeg_run` integration.

Frozen H1 evidence confirms the missing concerns were separable:

- `h1_mpeg_calibration_adapter.*` mapped neutral controller state into
  calibration semantics and exposed controller-consumption facts;
- `h1_mpeg_calibration_foreground.*` coordinated cross-domain mouse/RFB
  ownership and therefore is not UI-core ownership;
- `h1_mpeg_calibration_render.*` and `h1_mpeg_calibration_raster.*`
  represented/rendered the visible calibration surface without owning GS;
- `h1_mpeg_calibration_entry_hold.*` owned the temporary 750-ms START+SELECT
  policy separately;
- `h1_mpeg_calibration_interaction_binding.*` was experiment-local
  cross-domain composition and is evidence rather than a structure to copy.

The clean next dependency is therefore the UI-owned manual region source:
controller-semantic adaptation plus platform-neutral visible calibration
presentation. Cross-domain RFB/input-runtime ownership and product activation
remain later Application work.

## Next packet selected

`A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8`

The packet:

- preserves explicit begin and does not hardwire START+SELECT/750-ms timing;
- maps neutral controller facts into the already-accepted P1 action vocabulary;
- exposes calibration ownership/consumption through edit/review/quarantine;
- propagates only the one-shot P1 accepted edge and a copied accepted region;
- preserves cancel/quarantine behavior;
- adds a platform-neutral EDIT/REVIEW visual plan;
- reconstructs the smallest useful deterministic caller-owned CT16 calibration
  surface from frozen H1 render/raster evidence;
- derives every visual geometry from P1 candidate/resolved base/inner/outer
  semantics;
- never mutates authoritative RFB framebuffer storage;
- does not reintroduce an H1-only CONTROLS state or adapter proliferation unless
  current authority proves it necessary;
- produces region data only and does not activate MPEG;
- does not touch P2, RFB session, input runtime, pointer state, ordinary
  Application, Display/Platform, Transport, MPEG owners, Pi or protocol;
- keeps DESKTOP CALIBRATION completely separate.

This packet deliberately leaves the subsequent cross-domain manual-calibration
foreground/Application wiring as the next dependency after P8 review.

## Foreman-owned state publication

Published Foreman State revision 0061:

`4a62a8fd0d33ad4530247b05e7e2a8d2a862c9b1`

message:

`docs(foreman): accept R24 and activate manual MPEG region source`

State 0061:

- accepts R24;
- records exact R24 source/evidence/hardware-debt identity;
- marks final Application MPEG reveal Foreman-accepted;
- activates exactly one next packet, A004 P8;
- keeps ordinary product MPEG activation deferred.

Immediately after State 0061 publication the live branch remained exactly at
that state commit.

No workflow or combined-status record was exposed for the docs-only state commit
at log close, so no state-head CI claim is manufactured.

## Hardware debt

Current fully Foreman-accepted behavior-bearing identity:

`PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`
`PT_LOAD_BYTES=500244`

It is repository-reproducible and not physically hardware-qualified.

P8 may create a later loadable identity if its new UI source is enrolled in the
clean linked product. Any such identity remains hardware-pending unless
separately qualified.

HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

## Exact next pickup

NEXT_PACKET=A004-MPEG-CALIBRATION-MANUAL-REGION-SOURCE-P8
NEXT_PICKUP=RECONSTRUCTION_EXECUTE_ONLY_ACTIVE_P8_FROM_FOREMAN_STATE_0061

The Reconstruction worker must independently recover live branch authority,
consume State 0061, execute only P8, create exactly one immutable Reconstruction
record using WORK_ITEM_KEY `a004-presentation`, stop, and return the baton.

Do not begin ordinary `src/app.c` MPEG activation, RFB/input-runtime
foreground composition, START+SELECT product trigger policy, Pi calibration
persistence, MPEG START/RETIRE product wiring, AUDIO, or unrelated UI work in
the same shift.

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T06:58:36-04:00
COMPLETED_AT=2026-09-25T07:16:48-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b1a6ed31207d65ce5cd2a717a5bef47129f778ad
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — A004 P9 Application MPEG CALIBRATION foreground

## Recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`b1a6ed31207d65ce5cd2a717a5bef47129f778ad`.

The governing Foreman authority was State revision `0062`, which had accepted
A004 P8 and activated exactly:

`A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a004-presentation`;
- `WORKER_KEY=interactive`;
- `ARCHITECTURE_BLOCKER=NONE`;
- ordinary MPEG product trigger/service wiring deferred;
- DESKTOP CALIBRATION explicitly outside P9.

The worker consumed the current Reconstruction Contract, Work Log Contract,
Architecture Overlay, Wire Runtime Decisions, Foreman State 0062, the latest
Foreman handoff log, accepted P8/P1 MPEG CALIBRATION authority, P2 RFB flow
policy, input-runtime mouse-suspension APIs, P3 presentation state, local-UI
foreground state and the ordinary Platform desktop presentation seam.

The GitHub-connected Worker seat cannot observe external Pi/local checkout
staging, untracked files or local worktree dirtiness. No local reset, clean,
stash or checkout-over is claimed.

## Result

P9 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5`

The packet adds one Application-owned foreground coordinator:

- `src/app_mpeg_calibration.h`;
- `src/app_mpeg_calibration.c`.

It composes existing public owner seams without changing ordinary
`src/app.c` or any lower-owner mechanism.

## P9 transaction reconstructed

### Trigger-agnostic admission

`pstvnc_app_mpeg_calibration_begin()` is explicit and contains no permanent
entry detector.

Admission requires:

- coordinator IDLE;
- P8 inactive with no release quarantine;
- local UI ordinary DESKTOP with no transition quarantine;
- P3 exact RFB_ONLY with no snapshot and generation zero;
- P2 thawed;
- valid, separate caller-owned frozen/work CT16 surfaces;
- a sufficiently large, non-overlapping caller-supplied last-presented desktop
  source.

Rejected admission does not freeze P2 or rewrite the pre-existing coordinator
state. Focused evidence covers OSK, local quarantine, non-RFB-only P3,
unexpected P3 snapshot, pre-frozen P2, already-active P8 and a pre-existing
ACCEPTED_PROTECTED endpoint.

### Protection/input/snapshot order

After admission the implementation performs:

1. P2 freeze;
2. input-runtime mouse suspension;
3. optional exact RFB pointer button-up at the last successfully-published
   cursor when published click state is nonzero;
4. Application published-click authority becomes neutral only after that send
   succeeds;
5. suspended mouse rebase to the same cursor and neutral click state;
6. immutable copy of the caller's last physically presented ordinary desktop
   into dedicated frozen storage;
7. P8 begin;
8. P8 plan/raster into a distinct work surface;
9. Platform desktop presentation of that work surface with no MPEG surface.

The focused composition witness records this order directly. Its input suspend
stub requires real P2 already frozen, and its P8-begin stub requires the frozen
snapshot already byte-equal to the caller source.

A neutral published click skips remote release serialization. A failed pointer
release leaves the published click state unchanged, never reaches rebase/P8
begin, never resumes input and leaves P2 frozen in FAULTED containment.

### Immutable desktop and visible calibration

P9 owns no RFB framebuffer storage. The begin caller supplies the last
successfully presented ordinary CT16 desktop; P9 copies it into dedicated frozen
storage.

P8 raster output goes to a distinct caller-owned work surface. The frozen input
and original caller source remain unchanged while EDIT/REVIEW presentations
advance.

Visible P8 EDIT and REVIEW plans are presented only through
`pstvnc_ps2_graphics_present(work_surface, ..., NULL)`.

A Platform presentation failure after P2/input protection fails closed:
coordinator FAULTED, P2 frozen, mouse suspension retained and no guessed
resume/thaw.

### Controller consumption

While ACTIVE, P9 accepts exactly one normalized controller observation through
P8 and publishes P8's `consume_controller_state` fact for later ordinary
main-loop integration.

A sample that P8 fails to consume while P9 claims the foreground is treated as
a state contradiction and fails closed. P9 does not modify local-controller,
OSK or ordinary `app.c` routing.

### Accepted geometry conversion

P8's one-shot accepted region is resolved only by:

`pstvnc_mpeg_calibration_resolve_geometry()`

The returned P1 base, inner-content and suppression rectangles are copied
field-for-field into one
`pstvnc_mpeg_presentation_geometry_t`.

No independent MPEG calibration base/inner/suppression arithmetic exists in
P9.

Focused evidence injects distinct P1-resolved rectangle values, requires one
resolve call, and compares all three retained presentation rectangles exactly.

### Release quarantine and physical restoration

Accept/cancel edges only latch the pending endpoint. They do not complete
Application foreground ownership.

P2 remains frozen and mouse interpretation remains suspended while P8 release
quarantine is pending.

On P8's exact release-quarantine-complete edge, P9 first presents the exact
frozen ordinary desktop through the Platform desktop seam. Only after that
successful physical restore does it resume mouse interpretation.

The focused event witness requires:

`PRESENT_RESTORE < RESUME`.

Restore failure therefore cannot leave stale calibration pixels while silently
resuming input.

### Divergent endpoints

Cancel:

- exact frozen desktop restore;
- mouse resume;
- one P2 thaw;
- return IDLE;
- P2's ordinary frozen->thawed FULL-refresh debt preserved.

Focused evidence also begins cancel with an older P2 request outstanding:
post-thaw `next_request` remains HOLD until that request completes, then
becomes FULL.

Accept:

- exact frozen desktop restore;
- mouse resume;
- enter explicit ACCEPTED_PROTECTED;
- retain exact resolved presentation geometry;
- keep P2 frozen;
- require P3 still exact RFB_ONLY;
- start no MPEG run.

`pstvnc_app_mpeg_calibration_copy_accepted_geometry()` is read-only and leaves
protection unchanged.

`pstvnc_app_mpeg_calibration_abort_accepted()` is an explicit no-MPEG escape
from ACCEPTED_PROTECTED. It thaws P2 exactly once, preserves normal FULL debt,
clears retained protected geometry and returns IDLE.

## Deliberate non-scope

The exact pre-log compare does not modify ordinary `src/app.c`,
`src/app_mpeg_run.*`, P8/P1 behavior, RFB flow/session/parser implementation,
input-runtime implementation, local-controller/OSK behavior, Display/P3/
compositor implementation, Platform graphics implementation, MPEG
decoder/worker/runtime, Transport, Configuration, Pi product source, AUDIO,
DESKTOP CALIBRATION or Wire bytes.

A direct P9 source-scope scan at final pre-log authority found:

- no `pstvnc_app_mpeg_run_start/service/retire/reveal` call;
- no P3 lifecycle-mutator call;
- no Transport call;
- no MPEG decoder/worker/runtime call;
- no 750-ms literal;
- no START+SELECT trigger literal in the implementation source.

P9 therefore remains a protected foreground transaction only; later authority
must connect ACCEPTED_PROTECTED to ordinary MPEG product activation.

## Build/topology enrollment

Because P9 is a current Application coordinator, current root topology was
deliberately expanded to include:

- `src/app_mpeg_calibration.c`;
- `src/app_mpeg_calibration.h`.

Current linked PS2 build enrollment now includes P9 plus the accepted P8/P1
modules it calls:

- `app_mpeg_calibration.o`;
- `mpeg_calibration_manual.o`;
- `mpeg_calibration.o`.

The pinned current-source PS2 compile source set was expanded equivalently.

This is current build enrollment, not ordinary `app.c` activation: no normal
product trigger calls P9 yet.

## Dictionary reconciliation

The root `src/` dictionary is inside the canonical mechanically safe
reconciliation target set.

After behavior/build stabilization, the Worker emitted the exact no-tree-change
trigger:

`9af174a74fe58712de6135bd22d2aba31480b99c`

message:

`tooling(symbols): run deterministic dictionary reconciliation`

The repository-authorized reconciliation produced:

`4d500bda06b91e251f253f03da38705ed9fd6552`

message:

`docs(symbols): reconcile current clean definitions`

and changed only:

- `src/SYMBOLS.md` — 161 P9 definition rows added;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` — generated count/portal
  update.

No dictionary tooling implementation was changed.

The exact source-identical final verification commit is:

`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5`

message:

`test: verify final P9 calibration foreground authority`.

## Commits

The complete pre-log Worker range is nine commits ahead / zero behind the
starting Foreman authority:

1. `e1b07b9a9910bef55f781cbb3943bb9fbc86332a` —
   `app: add P9 calibration foreground coordinator`;
2. `92c5ff4ba23c76039c599aace1d8ef9d52d2e936` —
   `test(app): prove P9 calibration foreground transaction`;
3. `2d1d281754187e88a9d5e5d7426f3485980bddef` —
   `build: enroll P9 calibration foreground source`;
4. `21684275e8b2cdf01efb97be52f36dc5b5a906f6` —
   `test(app): add libpad stub to P9 host target`;
5. `f4cdf929ce0c9112dd9eede5b3dfabc768da8894` —
   `test(app): preserve frozen P2 admission witness`;
6. `7c40f48bab83657055124fadd7de518827149276` —
   `test(app): preserve rejected P9 coordinator state`;
7. `9af174a74fe58712de6135bd22d2aba31480b99c` —
   deterministic dictionary reconciliation trigger;
8. `4d500bda06b91e251f253f03da38705ed9fd6552` —
   generated clean-source dictionary/portal reconciliation;
9. `b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5` —
   final source-identical P9 verification commit.

The two focused-test follow-ups corrected harness assumptions only:
the missing existing `tests/support/libpad.h` include path, and rejection
helpers that had incorrectly expected a deliberately pre-frozen P2/pre-existing
ACCEPTED_PROTECTED state to be rewritten. Product P9 source did not change after
its initial implementation commit.

No concurrent branch movement or write collision occurred during Worker writes.
The generated dictionary commit was the expected repository-authorized movement
following the exact trigger.

## Changed paths

Exact compare from starting authority
`b1a6ed31207d65ce5cd2a717a5bef47129f778ad` to final pre-log authority
`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5` is nine commits ahead / zero
behind.

Changed paths are only:

- `src/app_mpeg_calibration.c` — new;
- `src/app_mpeg_calibration.h` — new;
- `tests/unit/app_mpeg_calibration_test.c` — new;
- `tests/Makefile`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `scripts/continuity-check.sh`;
- `docs/development/source-topology.md`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A004-P9-C1 ADMISSION_REQUIRES_RFB_ONLY_DESKTOP_FOREGROUND_AND_THAWED_P2`
  — MET. Focused tests reject OSK/quarantine, non-RFB-only or snapshot-bearing
  P3, frozen P2, active P8 and existing protected acceptance without mutation.
- `A004-P9-C2 P2_FREEZE_PRECEDES_CALIBRATION_FOREGROUND_AND_VISUAL_SNAPSHOT`
  — MET. The event witness requires P2 frozen before input suspension, snapshot
  and P8 begin.
- `A004-P9-C3 INPUT_SUSPEND_POINTER_NEUTRALIZE_REBASE_ORDER_IS_EXACT` — MET.
  The held-click event order is suspend -> button-up -> rebase, with P2 already
  frozen; failed serialization cannot falsely neutralize/rebase/resume.
- `A004-P9-C4 CALIBRATION_RASTER_USES_IMMUTABLE_LAST_PRESENTED_DESKTOP_NOT_RFB_AUTHORITY`
  — MET. Caller desktop/frozen/work surfaces are separate; original and frozen
  data remain immutable while work-surface raster/presentation advances.
- `A004-P9-C5 VISIBLE_P8_EDIT_REVIEW_PRESENTATION_IS_PLATFORM_BOUND_AND_FAIL_CLOSED`
  — MET. EDIT/REVIEW service creates Platform desktop-present attempts; a
  protected presentation failure leaves P2 frozen/input contained.
- `A004-P9-C6 CALIBRATION_CONTROLLER_SAMPLES_CANNOT_LEAK_TO_ORDINARY_LOCAL_ROUTING`
  — MET. P9 exposes P8's consume fact and treats a non-consumed ACTIVE sample as
  contradiction; ordinary local-controller/OSK implementation is untouched.
- `A004-P9-C7 ACCEPTED_REGION_CONVERTS_TO_PRESENTATION_GEOMETRY_ONLY_THROUGH_P1_RESOLUTION`
  — MET. Accepted region calls P1 resolve once and copies all three rectangles
  field-for-field; no independent calibration geometry math exists in P9.
- `A004-P9-C8 RELEASE_QUARANTINE_COMPLETES_BEFORE_FOREGROUND_EXIT` — MET.
  Accept/cancel latch pending endpoints while P2/input protection remains;
  finalization occurs only on P8 quarantine-complete.
- `A004-P9-C9 FROZEN_DESKTOP_IS_PHYSICALLY_RESTORED_BEFORE_MOUSE_RESUME` —
  MET. Focused event evidence requires restore presentation before resume;
  restore failure cannot resume.
- `A004-P9-C10 CANCEL_THAWS_WITH_FULL_DEBT_ACCEPT_REMAINS_PROTECTED` — MET.
  Cancel restore/resume/thaw returns idle with P2 FULL debt and correct HOLD
  interaction for an older outstanding request; accept retains P2 frozen,
  exact RFB_ONLY P3 and protected geometry.
- `A004-P9-C11 ACCEPTED_PROTECTED_ABORT_IS_EXPLICIT_AND_GEOMETRY_READ_DOES_NOT_THAW`
  — MET. Geometry copy has no event/protection mutation; explicit abort alone
  thaws once, creates normal FULL debt, clears geometry and returns idle.
- `A004-P9-C12 NO_TRIGGER_MPEG_START_ACTIVE_RECALIBRATION_OR_LOWER_OWNER_SCOPE_CREEP`
  — MET. Exact diff and direct source scan show no ordinary app activation,
  permanent trigger, app_mpeg_run call, P3 mutation, Transport/MPEG execution,
  lower-owner edits, Pi/AUDIO/desktop-calibration/Wire change.

All twelve P9 criteria are `MET` for independent Foreman review.

## Machine evidence

The final exact pre-log authority:

`b3908f6b5e7262839b8c8706c6cfd19a0cfed0c5`

ran canonical workflow:

`36128369370`

attempt:

`1`

with conclusion:

`SUCCESS`.

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected because final dictionaries were
  already synchronized.

Observed final focused/regression evidence includes:

- `MPEG_CALIBRATION_UNIT=PASS`;
- `MPEG_CALIBRATION_MANUAL_TEST=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `app_test: PASS`;
- `app R15/R16B/R19 tests: PASS`.

The repository has no standalone host target named
`input_runtime_test`. P9 did not modify input-runtime implementation; the
unchanged `src/input/input_runtime.c` passed the pinned current-source PS2
compile and the existing ordinary Application/input regression suite remained
green.

Observed final repository/build proof includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=208 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact new linked identity:

- `ELF_PRISTINE_SHA256=aad95787527a9668a9cceb29affb25dbf1eb546f14c739fafae1d1d07830ecf5`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=b57964cb50464bcbe8239865f7d54d890e0fffe9d5bf4479b79d178c0293270a`;
- `PT_LOAD_BYTES=510996`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

The linked loadable changed from the Foreman-accepted R24/P8 identity:

- previous `PT_LOAD_SHA256=0b4810977f794cb5c34e5ce4a90a54be628622a53ac74fe519df12f6e58efc4e`;
- previous `PT_LOAD_BYTES=500244`.

That change is expected because P9 plus accepted P8/P1 are now enrolled into
the current linked source image. They remain uncalled by ordinary `src/app.c`
until later activation authority.

The new identity is reproducible build evidence only. No new hardware
qualification is claimed.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_P9`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`LOADABLE_BYTES_CHANGED=YES`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO_NEW_CLAIM`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0062`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted P8/P1 MPEG CALIBRATION authority;
- accepted P2, input-runtime, P3, local-UI and Platform public seams.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no DESKTOP CALIBRATION revision;
- no permanent trigger;
- no ordinary `src/app.c` activation;
- no MPEG run start/retire/service/reveal;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No P9 source blocker remains within the authorized packet.

The existing public owner seams were sufficient; no lower-owner contract had to
be broadened.

The two intermediate focused-test failures were test-harness assumptions, not
product defects:

1. the new host target initially omitted the repository's existing
   `tests/support/libpad.h` include path required transitively by
   `input_runtime.h`;
2. the admission helper initially expected deliberately pre-frozen P2 and
   pre-existing ACCEPTED_PROTECTED state to be rewritten on rejection rather
   than preserved.

Both were corrected in test-only commits. Product P9 source remained unchanged
after its initial implementation commit.

Ordinary MPEG product trigger/service wiring and active-MPEG recalibration
remain deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_P9_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect P9 source/evidence/log, decide acceptance,
and select any later bounded packet. Reconstruction must not self-accept P9 or
begin ordinary MPEG product activation.

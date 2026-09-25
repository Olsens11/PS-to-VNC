DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T20:27:45-04:00
COMPLETED_AT=2026-09-24T20:37:59-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=5a7b26436ea8b72148945e44f65f94c99d5a92f5
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R23C Q7 restoration overlap correction

## Objective and recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`5a7b26436ea8b72148945e44f65f94c99d5a92f5`.

The live branch contained Foreman State revision `0059` at
`6969457f0f7b1e388b391c3adf2dbfb256704f61` plus its immutable Foreman
review log
`5a7b26436ea8b72148945e44f65f94c99d5a92f5`.

State 0059 made exactly one Reconstruction packet active:

`A003-APPLICATION-MPEG-Q7-RESTORE-OVERLAP-R23C`

with:

- `APPLICATION_MPEG_RETIREMENT=CORRECTION_REQUIRED`;
- `APPLICATION_MPEG_Q7_RESTORE_OVERLAP=RECONSTRUCTION_ACTIVE`;
- `APPLICATION_MPEG_FINAL_REVEAL=DEPENDENCY_QUEUED`;
- `ORDINARY_MPEG_PRODUCT_ACTIVATION=DEFERRED`;
- `ARCHITECTURE_BLOCKER=NONE`.

The worker consumed:

- Foreman State revision `0059`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Wire Runtime Decisions revision `0011`, especially governing Q7;
- Architecture Overlay revision `0007`;
- accepted R22 Application live-service authority;
- the R23 correction-required source/evidence at final source
  `3b4087d7b65992f0c3edda5fcf34a627ad6f1d28`;
- accepted P2 RFB flow-policy public semantics;
- accepted P3 Presentation RETIRING state;
- accepted P7 frame drain;
- accepted R20 exact-generation RETIRE projection;
- accepted R18 Transport run finalization;
- accepted R4/R5 MPEG worker/runtime lifecycle.

Repository authority explicitly limited R23C to the smallest justified subset
of `src/app_mpeg_run.{c,h}`, focused Application MPEG tests, directly affected
documentation/dictionaries and build/check manifests genuinely required by the
focused test.

External Pi/local checkout staging, untracked files and worktree cleanliness are
not observable through this GitHub-connected worker seat. No local reset,
clean, stash, checkout-over, `scripts/resume-state.sh` or shell execution is
claimed.

## Result

R23C is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`da091d1e1fcb2398bd695cae1516c4730213cf55`

The correction preserves all useful R23 safe-stop/drain mechanics while
restoring the older governing Q7 overlap sequence.

### Exact corrected lifecycle

Normal retirement admission remains unchanged in meaning:

- Application must be exact-generation `MPEG_OWNED`;
- P3 must be exact-generation `MPEG_OWNED`;
- P7/R4/R5/R18 owners must be healthy/live;
- P2 must already be frozen and must deny remote publication.

Application still advances P3 to exact-generation `RETIRING` first.

Application still records one irreversible RETIRE invocation and calls the
accepted R20 exact-generation RETIRE seam second.

The R23C change occurs only after RETIRE returns `PSTVNC_TRANSPORT_OK`.

Application then invokes P2's already-accepted public transition:

`pstvnc_rfb_flow_policy_set_frozen(run->rfb_flow_policy, 0)`

No P2 private field is written by Application.

The new Application validation then proves, through existing public P2
semantics:

- P2 is thawed;
- remote publication is allowed;
- if a real request is already outstanding, P2 reports HOLD;
- otherwise P2 reports FULL, proving that the real frozen-to-thawed transition
  created its one-shot FULL-refresh debt.

Application does not call
`pstvnc_rfb_flow_policy_record_request_sent()` or
`pstvnc_rfb_flow_policy_record_update_complete()` in product source and does
not clear FULL debt or manufacture freshness.

### RFB restoration overlap under visible RETIRING MPEG

After the successful Q7 release, `pstvnc_app_mpeg_run_retirement_service()`
requires P2 restoration to remain started: P2 must remain thawed and must allow
remote publication.

The earlier R23 requirement that P2 stay frozen during RETIRING drain was
removed from retirement-service validation and from the successful endpoint.

P3 remains exact-generation `RETIRING`; R23C does not seal, reveal, commit or
remove the retained MPEG composition.

P7 remains the sole MPEG frame-drain mechanism and continues accepted
IDLE/WAIT/PRESENTED/DROPPED behavior unchanged.

Because P2 is thawed, the already-accepted ordinary R19 scheduler can observe
and act on the real P2 FULL/HOLD request decision while P7 continues RETIRING
MPEG drain. R23C itself sends no RFB request and performs no RFB parser or
framebuffer work.

An unexpected later P2 re-freeze is now an ownership/lifecycle contradiction and
fails closed before further retirement service.

### Preserved R23 fences

R23C does not change the accepted R23 execution-retirement order:

1. exact RETIRE completion;
2. producer-done publication;
3. natural worker completion with no outstanding P7 borrow;
4. worker join;
5. exact-generation `COMPLETED` outcome;
6. P7 retirement;
7. R4 release;
8. R5 release;
9. R18 finalization.

The normal path still never requests worker stop.

All post-RETIRE failure paths preserve the ownership facts already reached.
R23C adds no call that re-freezes P2 after restoration has been released.

Therefore drain, completion, producer, join/outcome, R4 release, R5 release or
R18 finalization failure does not manufacture restoration rollback.

### Correct successful endpoint

Successful execution retirement remains:

`PSTVNC_APP_MPEG_RUN_RESTORE_PENDING`

with:

- exact `current_generation` retained;
- P3 exact-generation `RETIRING`;
- P3 immutable geometry/generation snapshot retained;
- P2 thawed;
- remote RFB publication allowed.

The coordinator deliberately does not infer whether the P2 FULL obligation is
still pending, in flight, or already completed by the outer ordinary RFB loop.

R23C does not:

- seal P3 into final reveal-pending authority;
- execute synchronized no-MPEG reveal;
- commit P3 back to RFB_ONLY;
- fabricate fresh RFB state;
- modify ordinary `src/app.c`;
- choose a controller/menu/calibration trigger;
- activate the Pi MPEG product factory;
- change Pi product source;
- change P2 implementation;
- change P3/Display/compositor/P7 implementation;
- change MPEG worker/backend/runtime implementation;
- change Transport implementation or protocol bytes;
- change AUDIO, Input/UI or calibration.

## Focused deterministic evidence

The focused Application fixture now links the real accepted
`src/rfb/flow_policy.c` implementation rather than providing its earlier
publication-only stub.

That allows the R23C tests to exercise actual P2 public accounting while the
other lower owners remain represented through their accepted public seams.

The fixture proves:

1. P2 is frozen before normal retirement admission;
2. the RETIRE stub observes P2 still frozen when RETIRE is invoked;
3. the existing event record proves P3 begin-retirement occurs before RETIRE;
4. after successful RETIRE return, Application observes P2 thawed and remote
   publication allowed;
5. with no prior request outstanding, the real P2 next-request decision is FULL;
6. a focused overlap test records that FULL request only through P2's public
   request-sent seam, deliberately does not record update completion, and proves
   P7 WAIT continues correctly while the RFB request is genuinely in flight;
7. a second test constructs a real prior outstanding request exclusively through
   P2 public transitions/accounting, then proves the R23C thaw yields HOLD while
   preserving restoration release;
8. RETIRE failure leaves P2 frozen and emits no normal-path Q7 release;
9. duplicate retirement begin cannot cross another RETIRE/Q7 edge;
10. post-thaw P7 IDLE/WAIT/PRESENTED/DROPPED drain remains valid with P3
    RETIRING;
11. an unexpected re-freeze causes retirement service to fail closed before P7
    service or RETIRE-completion consumption;
12. existing completion -> producer-done -> natural worker -> join/outcome ->
    P7/R4/R5 -> R18 ordering tests remain green;
13. representative post-thaw worker-outcome, R5 release and R18-finalize failure
    tests explicitly prove P2 remains released;
14. successful retirement ends RESTORE_PENDING with P3 RETIRING and P2 thawed.

No R23C test manually clears P2 FULL debt or fabricates framebuffer-update
completion.

The canonical P2 regression itself also passes independently as
`RFB_FLOW_POLICY_TEST=PASS`.

## Commits

The complete pre-log Worker range is five commits ahead / zero behind the
starting Foreman authority:

1. `ae06725832ea0a77b3526d33464d404801225da8` —
   `app: restore Q7 overlap during MPEG retirement`;
2. `0b9946ceea9239175f8771820d39b8ea1d1e7d86` —
   `docs: record R23C Q7 restoration overlap`;
3. `4e5d6a29c4146e9201b60156883eead668d8fb9c` —
   `tooling(symbols): run deterministic dictionary reconciliation`;
4. `af1d70024633d40982451ce42dbf66ca46a3fb92` —
   `docs(symbols): reconcile current clean definitions`;
5. `da091d1e1fcb2398bd695cae1516c4730213cf55` —
   `test: verify final R23C Q7 retirement overlap authority`.

The reconciliation commit was produced by the repository-authorized
deterministic dictionary workflow and changed only the root clean-source
dictionary and generated portal.

No concurrent branch movement or write collision occurred during the Worker
writes.

## Changed paths

The complete pre-log compare from
`5a7b26436ea8b72148945e44f65f94c99d5a92f5` to
`da091d1e1fcb2398bd695cae1516c4730213cf55` is five commits ahead / zero
behind and changes only:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `tests/Makefile`;
- `docs/development/mpeg-generation-control.md`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No lower-owner implementation, ordinary `src/app.c`, Pi source, protocol,
AUDIO, Input/UI or calibration source changed.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R23C-C1 BEGIN_RETIREMENT_STILL_REQUIRES_PREEXISTING_P2_FREEZE` —
  MET. The admission helper still requires frozen P2/publication denial; focused
  thawed-admission testing fails closed before RETIRE.
- `A003-R23C-C2 P3_RETIRING_AND_EXACT_RETIRE_PRECEDE_Q7_THAW` — MET. P3's
  recorded begin-retirement event precedes RETIRE, and the RETIRE stub proves P2
  is still frozen during that invocation; P2 is observed thawed only after
  successful return.
- `A003-R23C-C3 SUCCESSFUL_RETIRE_SERIALIZATION_CAUSES_EXACTLY_ONE_P2_THAW` —
  MET. The only normal-path thaw call is immediately after the one successful
  RETIRE call. RETIRE failure emits none, and state transition prevents a second
  retirement begin.
- `A003-R23C-C4 THAW_CREATES_REAL_P2_FULL_REFRESH_DEBT_WITHOUT_FABRICATION` —
  MET. Product invokes the real P2 set-frozen seam and only inspects public
  FULL/HOLD consequence; focused tests link the real P2 implementation. Product
  never clears the debt or records completion.
- `A003-R23C-C5 RFB_RESTORATION_CAN_PROGRESS_UNDER_VISIBLE_RETIRING_MPEG` —
  MET. Focused proof records a real FULL request through P2 while P3 remains
  RETIRING and then successfully services P7 WAIT without fabricated update
  completion.
- `A003-R23C-C6 P7_RETIRING_DRAIN_REMAINS_OWNER_CORRECT_AFTER_THAW` — MET.
  Existing P7 IDLE/WAIT/PRESENTED/DROPPED drain and exact-generation/P3 checks
  remain green after Q7 release.
- `A003-R23C-C7 RETIREMENT_SERVICE_REQUIRES_STARTED_RESTORATION_NOT_FROZEN_P2`
  — MET. Service now requires thaw/publication permission; explicit unexpected
  re-freeze faults before P7 service.
- `A003-R23C-C8 EXISTING_RETIRE_COMPLETION_WORKER_RECLAIM_FINALIZE_FENCES_PRESERVED`
  — MET. No R23 fence implementation was reordered; all direct R23 ordering and
  failure-path tests remain green.
- `A003-R23C-C9 POST_THAW_FAILURE_NEVER_REFREEZES_OR_MANUFACTURES_RESTORATION`
  — MET. Product has no post-release freeze call; representative failure tests
  explicitly preserve thawed P2 and no update completion is manufactured.
- `A003-R23C-C10 RESTORE_PENDING_RETAINS_P3_AND_EXACT_GENERATION_WITH_P2_THAWED`
  — MET. Focused endpoint proof retains exact generation and P3 RETIRING
  snapshot while P2 is thawed/publication-enabled.
- `A003-R23C-C11 NO_FINAL_REVEAL_ORDINARY_APP_PI_AUDIO_INPUT_OR_PROTOCOL_SCOPE`
  — MET. Final compare is limited to the authorized Application coordinator,
  focused test/build linkage, lifecycle documentation and generated
  dictionaries; prohibited product surfaces are untouched.
- `A003-R23C-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact final pre-log authority workflow `36078318661`, attempt 1, completed
  SUCCESS across all required canonical jobs.

All twelve R23C criteria are therefore `MET` for independent Foreman review.

## Machine evidence

Initial behavior authority
`ae06725832ea0a77b3526d33464d404801225da8` ran workflow
`36078170071`.

Observed:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-long — expected strict failure only for new R23C symbols before
  deterministic reconciliation.

The same substantive result was repeated on documentation authority
`0b9946ceea9239175f8771820d39b8ea1d1e7d86`.

The authorized reconciliation trigger
`4e5d6a29c4146e9201b60156883eead668d8fb9c` produced:

`af1d70024633d40982451ce42dbf66ca46a3fb92`

changing only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

Exact final pre-log authority:

`da091d1e1fcb2398bd695cae1516c4730213cf55`

Workflow:

`36078318661`

Attempt:

`1`

Conclusion:

`SUCCESS`

Final job evidence:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed host regressions include:

- `app_mpeg_run_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- Pi MPEG-generation fixture: 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`.

Observed project/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=202 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact final linked identity:

- `ELF_PRISTINE_SHA256=c5fc16d2b958dd9689697ea6c15556da59d2c440539cedbcc3bfab33be4e3704`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=c7443e28a6eb4a6580a153768e816d4011b2b2784dabb8ef274954cb36c081fa`;
- `PT_LOAD_BYTES=498964`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

R23C therefore changes loadable identity from the correction-required R23
candidate:

- R23 `PT_LOAD_SHA256=31b83c61377b5d647ed3692940aa1b9358d861e591d4b2113b3f1e50572aa0dd`;
- R23 `PT_LOAD_BYTES=498708`.

The R23C identity is repository-reproducible build evidence only and does not
inherit physical qualification.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R23C`
`HOST_TESTED=PASS`
`PROJECT_CHECK=PASS`
`STRICT_DICTIONARIES=PASS`
`PS2_COMPILE=PASS`
`PS2_LINK=PASS`
`CURRENT_SOURCE_REPRODUCIBILITY=PASS`
`MACHINE_EVIDENCE=GITHUB_ACTIONS`
`INDEPENDENT_VALIDATION=NOT_RUN`
`OPERATOR_OBSERVED=NO`
`HARDWARE_QUALIFIED=NO`
`HARDWARE_PENDING=YES`
`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State/contract accounting

Consumed:

- Foreman State revision `0059`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Wire Runtime Decisions revision `0011` Q7;
- Architecture Overlay revision `0007`;
- accepted P2/P3/P7/R18/R20/R4/R5 seams;
- correction-required R23 source/evidence.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no lower-owner mechanism revision;
- no final P3 seal/reveal;
- no ordinary MPEG product activation;
- no Pi MPEG product activation;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No R23C source blocker remains within the authorized packet.

The public P2 seam was sufficient; no lower-owner modification was required.

The new R23C loadable identity remains hardware-pending. Governing authority
states hardware debt does not block unrelated source work; this Worker does not
convert host/build/reproducibility evidence into physical qualification.

Final RFB freshness/seal/synchronized reveal and ordinary MPEG product activation
remain downstream/deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R23C_REVIEW_R23_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect the R23C source/evidence/log, decide whether
the R23/R23C combined retirement transaction is accepted, and select any later
bounded packet. Reconstruction must not self-accept R23C or begin final reveal/
ordinary MPEG product activation.

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T19:58:01-04:00
COMPLETED_AT=2026-09-24T20:11:45-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ac905f8eefd0585f648ac7c03832d17f35534dc9
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R23 Application MPEG retirement drain

## Objective and recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`ac905f8eefd0585f648ac7c03832d17f35534dc9` and consumed Foreman State
revision `0058`, whose active packet was:

`A003-APPLICATION-MPEG-RETIREMENT-DRAIN-R23`

with `PACKET_STATUS=ACTIVE`, `PACKET_OWNER=RECONSTRUCTION` and
`ARCHITECTURE_BLOCKER=NONE`.

The worker also consumed Reconstruction Contract revision `0006`, Work Log
Contract revision `0007`, Architecture Overlay revision `0007`, Wire
Runtime Decisions revision `0011`, accepted R22 source authority
`345e726effee8f01d1be71938e8f09efde030c0b`, accepted R22 closeout
`00015df0ef2c3a249e0d707b1f8218e3c43f95aa`, and the already-accepted
P3/P7/R18/R20/R4/R5/P2 public seams named by State 0058.

External Pi/local checkout staging, untracked files and worktree cleanliness are
not observable through this GitHub-connected worker seat. No local
`scripts/resume-state.sh`, reset, clean, stash or local shell execution is
claimed.

## Result

R23 is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`3b4087d7b65992f0c3edda5fcf34a627ad6f1d28`

The production behavior itself landed at:

`253d98ebabff8863f72c1df552e721ac5ba23136`

R23 extends the existing trigger-agnostic Application MPEG coordinator with two
truthful states:

- `PSTVNC_APP_MPEG_RUN_RETIRING`;
- `PSTVNC_APP_MPEG_RUN_RESTORE_PENDING`.

It adds two Application-owned operations:

- `pstvnc_app_mpeg_run_begin_retirement()`;
- `pstvnc_app_mpeg_run_retirement_service()`.

No lower-owner implementation was changed.

### Retirement admission and irreversible RETIRE edge

Retirement admission is legal only from the exact healthy Application
`MPEG_OWNED` run. Before crossing RETIRE, the coordinator proves:

- exact nonzero current generation;
- live R18 Transport run;
- live R4 worker and R5 runtime ownership;
- initialized exact-generation P7 consumer;
- P3 exact-generation `MPEG_OWNED`;
- nonfaulted exact-generation P7 status;
- worker not already finished, stop-requested or joined;
- P2 still frozen and remote publication still denied.

Application first invokes the accepted P3
`pstvnc_mpeg_presentation_begin_retirement()`, then independently verifies P3
is exact-generation `RETIRING`.

Only after that visible-drain state is established does Application set
`retire_invoked` and call the accepted R20
`pstvnc_transport_mpeg_send_retire()` with exactly
`current_generation`.

The invocation flag is set immediately before the Transport call. Therefore a
non-OK RETIRE result cannot be rolled back or retried as though no control bytes
could have escaped. The coordinator faults, requires outer session teardown,
and leaves P3 RETIRING rather than manufacturing MPEG_OWNED or clean retirement.

### RETIRING drain

While exact RETIRE completion is pending, R23 services frames only through the
existing P7 consumer in P3 RETIRING.

Accepted P7 drain outcomes remain:

- IDLE;
- WAIT;
- PRESENTED;
- DROPPED.

WAIT retains P7's exact claim, picture ordinal and absolute deadline. R23 does
not duplicate claim/mapping/scheduler/compositor behavior.

Every drain service independently confirms:

- P7 return/result agreement;
- exact-generation, nonfaulted P7 status;
- service/status claim agreement;
- P3 still exact-generation RETIRING;
- P2 still frozen.

Negative P7 results and contradictions fail closed through the existing
post-START fault path without inventing cleanup.

### RETIRE completion and producer fence

R23 calls the accepted R20 completion seam
`pstvnc_transport_mpeg_take_retire_completion()`.

`PSTVNC_TRANSPORT_WOULD_BLOCK` is benign pending retirement and preserves the
RETIRING state.

Only an exact nonzero completion matching `current_generation` is accepted.
Wrong-generation or completion-read failure faults.

After exact completion is taken, and not before, R23 calls
`pstvnc_transport_mpeg_mark_producer_done()` exactly once. The
`retire_completion_taken` and `producer_done_published` facts prevent
duplicate take/publication on later service calls.

A producer-done publication failure faults while preserving the truthful fact
that RETIRE completion was already consumed.

### Natural worker completion and reclaim fence

Worker completion observed before producer-done publication is a failure, not
normal retirement.

After producer-done, R23 continues P7 drain. Natural worker completion is
eligible for reclaim only when the same service step proves:

- P7 returned IDLE;
- no P7 service-result borrow is outstanding;
- P7 status reports no outstanding borrow;
- R4 status is exact-generation;
- worker is finished;
- no stop was requested;
- decoder is no longer live;
- thread was not already joined;
- R4 slot is EMPTY.

The clean path never invokes `pstvnc_mpeg_worker_request_stop()`.

R23 then requires exact join and exact outcome proof. Clean retirement accepts
only:

- exact current generation;
- outcome kind `PSTVNC_MPEG_WORKER_OUTCOME_COMPLETED`;
- worker result OK;
- decoder result COMPLETE;
- decoder release result COMPLETE.

STOPPED, FAILED, wrong-generation, join failure, or other contradictions cannot
be promoted into clean retirement.

After that proof and only with no borrow outstanding, R23 retires the now-empty
P7 value, releases R4 worker ownership, releases R5 runtime resources, and calls
R18 `pstvnc_transport_mpeg_run_finalize()` last.

Failure at any reclaim step faults while preserving the ownership facts already
reached. The coordinator does not manufacture rollback.

### Successful R23 endpoint

On complete execution retirement, Application enters:

`PSTVNC_APP_MPEG_RUN_RESTORE_PENDING`

and deliberately retains:

- exact `current_generation`;
- P3 state `PSTVNC_MPEG_PRESENTATION_RETIRING`;
- P3 immutable generation/geometry snapshot;
- P2 frozen state and publication denial.

The successful endpoint has retired P7/R4/R5 and finalized R18, but it does not
claim visual restoration.

R23 does not:

- seal P3 into REVEAL_PENDING;
- perform a synchronized no-MPEG reveal;
- commit P3 back to RFB_ONLY;
- thaw P2;
- request or consume post-thaw FULL refresh;
- modify ordinary `src/app.c`;
- choose a menu/controller/calibration trigger;
- activate Pi MPEG product runtime;
- alter AUDIO, Input/UI/calibration, Wire bytes, Transport internals,
  Presentation internals, P7 internals, MPEG worker/runtime/backend internals.

Those remain later authority.

## Focused deterministic evidence

The final `tests/unit/app_mpeg_run_test.c` retains all R21/R22 tests and adds
R23 coverage proving:

1. IDLE, WAIT_FIRST_FRAME, FAULTED and fabricated/contradictory ownership cannot
   begin normal retirement;
2. P3 begin-retirement precedes RETIRE;
3. RETIRE uses the exact current generation and is recorded as irreversible;
4. RETIRE failure faults without Presentation rollback, worker stop or
   Transport pre-START abort;
5. duplicate retirement begin does not emit another RETIRE;
6. RETIRING P7 IDLE/WAIT/PRESENTED/DROPPED remain benign and WAIT preserves
   exact claim/deadline;
7. completion WOULD_BLOCK is benign pending;
8. wrong-generation completion and completion failure fault without producer
   done;
9. exact completion precedes producer-done and later service does not re-take or
   republish;
10. producer-done publication failure preserves completion-taken truth;
11. worker-finished before producer-done faults and never joins;
12. after producer-done, worker-finished while a P7 WAIT borrow remains
    outstanding faults before join/reclaim;
13. clean completion requires exact empty-slot worker status, join and exact
    COMPLETED outcome;
14. STOPPED and FAILED outcomes are rejected as non-clean;
15. explicit join failure faults before outcome/reclaim;
16. worker release failure preserves R4/R5/R18 ownership facts and never
    finalizes;
17. runtime release failure preserves R5/R18 ownership facts and never
    finalizes;
18. Transport-finalize WOULD_BLOCK/failure faults after already-proven execution
    reclaim without falsely closing the Transport run;
19. successful ordering is worker status < join < outcome < R4 release < R5
    release < R18 finalize;
20. success ends RESTORE_PENDING with exact generation retained, P3 RETIRING
    snapshot retained and P2 frozen.

The same canonical host run also executes accepted P7, P2/P3,
R18/R20/Transport, MPEG decoder/worker/scheduler/compositor, Pi MPEG-generation,
ordinary Application and RFB-provider regressions.

## Commits

The complete pre-log Worker range is:

1. `253d98ebabff8863f72c1df552e721ac5ba23136` —
   `app: add R23 MPEG retirement drain transaction`
2. `9f6c1f0fe48e54b769c1bad61561a067761c8136` —
   `tooling(symbols): run deterministic dictionary reconciliation`
3. `6e3c4138404df8b9a223a6b2de348beb96598bf5` —
   `docs(symbols): reconcile current clean definitions`
4. `7413b1dedae2a32430f58969e4403e2d8b0350a7` —
   `test: verify final R23 MPEG retirement drain authority`
5. `5e54a577d46b33490232d7340797f01eea1c11de` —
   `test(app): complete R23 retirement failure evidence`
6. `3b4087d7b65992f0c3edda5fcf34a627ad6f1d28` —
   `test(app): correct R23 failed-outcome fixture`

The reconciliation commit was produced by the repository's authorized
dictionary automation after the exact trigger commit. It changed only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The final two commits are test-only evidence hardening. The first hardening
commit used nonexistent fixture constant `PSTVNC_MPEG_DECODER_FAILED`;
workflow `36076256414` correctly rejected that test compile. The next commit
changed it to the real `PSTVNC_MPEG_DECODER_PICTURE_FAILED` enum. No product
source was changed by either evidence-hardening commit, and final exact-head
workflow `36076291875` is fully green.

No unrelated concurrent branch movement or write collision occurred.

## Changed paths

Behavioral/documentation source:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `docs/development/mpeg-generation-control.md`.

Generated dictionary maintenance:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No other product implementation changed.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R23-C1 RETIREMENT_ADMISSION_REQUIRES_EXACT_HEALTHY_MPEG_OWNED_RUN` —
  MET. Admission proves exact live owner/P3/P7/P2/worker facts and rejects all
  other Application states before RETIRE.
- `A003-R23-C2 P3_RETIRING_PRECEDES_RETIRE_AND_PRESERVES_VISIBLE_DRAIN` —
  MET. Focused event ordering proves P3 begin-retirement before RETIRE; all
  drain/success tests retain P3 RETIRING.
- `A003-R23-C3 EXACTLY_ONE_CURRENT_GENERATION_RETIRE_CROSSES_IRREVERSIBLE_BOUNDARY`
  — MET. RETIRE carries only current generation, invocation is recorded before
  the call, duplicate begin emits no second RETIRE, and send failure faults
  without rollback.
- `A003-R23-C4 RETIRING_DRAIN_USES_ONLY_EXISTING_P7_AND_PRESERVES_WAIT_CLAIM` —
  MET. Existing P7 is the sole frame-service mechanism and focused tests retain
  exact WAIT ordinal/claim/deadline.
- `A003-R23-C5 RETIRE_COMPLETION_WOULD_BLOCK_IS_PENDING_AND_EXACT_COMPLETION_IS_FENCED`
  — MET. WOULD_BLOCK is benign; wrong generation/failure faults; exact
  completion is consumed once and repeated service does not re-take it.
- `A003-R23-C6 PRODUCER_DONE_OCCURS_ONLY_AFTER_EXACT_RETIRE_COMPLETION` — MET.
  Focused ordering proves take < producer-done, no producer-done on pending or
  bad completion, exactly-once publication, and truthful publication failure.
- `A003-R23-C7 CLEAN_WORKER_COMPLETION_IS_POST_FENCE_NATURAL_JOINED_COMPLETED_OUTCOME`
  — MET. Early finish faults; clean path contains no worker-stop call; direct
  tests reject join failure, STOPPED and FAILED outcomes.
- `A003-R23-C8 P7_R4_R5_RECLAIM_REQUIRES_NO_OUTSTANDING_BORROW` — MET.
  Post-fence finished+WAIT-borrow faults before join; clean path proves empty P7
  and R4 slot before P7/R4/R5 reclaim. Worker/runtime release failures preserve
  truthful ownership evidence.
- `A003-R23-C9 R18_FINALIZE_IS_LAST_EXECUTION_RECLAIM_STEP` — MET. Focused
  event ordering proves R4 release < R5 release < R18 finalize, and no finalize
  occurs after earlier reclaim failure.
- `A003-R23-C10 SUCCESS_ENDS_RESTORE_PENDING_WITH_P3_RETAINED_AND_P2_FROZEN` —
  MET. Focused status/state proof retains current generation, P3 RETIRING
  snapshot and frozen P2 after successful finalize.
- `A003-R23-C11 NO_THAW_FULL_REFRESH_REVEAL_PRODUCT_OR_LOWER_OWNER_SCOPE_CREEP`
  — MET. Final diff modifies no ordinary app/Pi/RFB/P3/P7/Transport/MPEG lower
  implementation and adds no thaw, FULL refresh, seal/reveal or product trigger.
- `A003-R23-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact final pre-log authority workflow `36076291875`, attempt 1, completed
  SUCCESS across host-unit, project-check, dictionary-long, ps2-compile and
  ps2-link/current-source reproducibility.

## Machine evidence

Initial behavioral source authority
`253d98ebabff8863f72c1df552e721ac5ba23136`, workflow
`36075828952`:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-long — expected strict failure only for newly introduced R23
  symbols prior to deterministic reconciliation.

The canonical reconciliation trigger
`9f6c1f0fe48e54b769c1bad61561a067761c8136` produced generated dictionary
authority `6e3c4138404df8b9a223a6b2de348beb96598bf5`.

Exact reconciled source verification
`7413b1dedae2a32430f58969e4403e2d8b0350a7`, workflow
`36076064549`, completed SUCCESS across all canonical jobs.

After direct failure-path test hardening and the one test-fixture typo
correction, final pre-log authority
`3b4087d7b65992f0c3edda5fcf34a627ad6f1d28` ran:

- workflow: `Ledge reconstruction checks`;
- run: `36076291875`;
- attempt: `1`;
- conclusion: `success`;
- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed final host evidence includes:

- `app_mpeg_run_test: PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi MPEG-generation fixture: 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`.

Observed final project/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Final linked identity:

- `ELF_PRISTINE_SHA256=7840ce4964e4285e30b68c291e9281ba9d0dfa063d2a11e639a78b430042b0b7`;
- `PT_LOAD_SEGMENTS=1`;
- `PT_LOAD_SHA256=31b83c61377b5d647ed3692940aa1b9358d861e591d4b2113b3f1e50572aa0dd`;
- `PT_LOAD_BYTES=498708`;
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

R23 changes loadable bytes from accepted R22
(`PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`,
`PT_LOAD_BYTES=496404`). The R23 identity is repository-reproducible build evidence only
and does not inherit physical qualification.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R23`
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

- Foreman State revision `0058`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted R22 Application live-service authority;
- accepted P2/P3/P7/R18/R20/R4/R5 lower-owner seams.

Produced:

- no Foreman state revision;
- no contract revision;
- no Wire/protocol revision;
- no P3 seal/reveal;
- no P2 thaw/FULL-refresh;
- no ordinary product activation;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No R23 source blocker remains within the authorized packet.

The new R23 loadable identity remains hardware-pending. Current governing
authority states that hardware debt does not block unrelated source work; this
Worker does not convert compile/link/reproducibility evidence into physical
qualification.

RFB restoration/reveal and ordinary MPEG product activation remain intentionally
deferred.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R23_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect this R23 source/evidence/log, decide
acceptance, and select any later bounded packet. Reconstruction must not
self-accept R23 or begin RFB restoration/reveal.

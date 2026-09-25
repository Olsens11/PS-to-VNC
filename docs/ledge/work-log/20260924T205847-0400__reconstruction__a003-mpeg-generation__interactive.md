DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T20:58:47-04:00
COMPLETED_AT=2026-09-24T21:08:47-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=5b329dd46066602cd95eb1813be226395d3d07a0
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R24 final restored-RFB reveal

## Objective and recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`5b329dd46066602cd95eb1813be226395d3d07a0`.

The branch contained Foreman State revision `0060` at its published authority
plus immutable Foreman closeout
`5b329dd46066602cd95eb1813be226395d3d07a0`.

State 0060 independently accepted the corrected R23/R23C retirement transaction
and activated exactly one Reconstruction packet:

`A003-APPLICATION-MPEG-FINAL-RFB-REVEAL-R24`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `APPLICATION_MPEG_FINAL_REVEAL=RECONSTRUCTION_ACTIVE`;
- `ORDINARY_MPEG_PRODUCT_ACTIVATION=DEFERRED`;
- `ARCHITECTURE_BLOCKER=NONE`.

The worker consumed:

- Foreman State revision `0060`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Wire Runtime Decisions revision `0011`;
- Architecture Overlay revision `0007`;
- fully Foreman-accepted corrected R23C source authority
  `da091d1e1fcb2398bd695cae1516c4730213cf55`;
- accepted P2 RFB flow-policy freshness/accounting semantics;
- accepted P3 RETIRING -> REVEAL_PENDING -> RFB_ONLY ownership seam;
- accepted synchronized compositor reveal seam;
- accepted R21-R23C Application generation/start/live/retirement authority.

External Pi/local checkout staging, untracked files and worktree cleanliness are
not observable through this GitHub-connected worker seat. No local reset,
clean, stash, checkout-over, `scripts/resume-state.sh` or shell execution is
claimed.

## Result

R24 is source-complete as a Reconstruction candidate for independent Foreman
review.

Final pre-log authority:

`d777835c53a0e829e423f76bd0f78097c88e0ffb`

R24 adds only the Application-owned final visible handoff required by State
0060. Ordinary MPEG product activation remains untouched.

### Exact RESTORE_PENDING admission

Final restoration work is admitted only from Application
`PSTVNC_APP_MPEG_RUN_RESTORE_PENDING` or, after an already-successful seal,
the explicit retry state
`PSTVNC_APP_MPEG_RUN_REVEAL_PENDING`.

The coordinator independently requires the exact retired execution facts:

- nonzero current generation;
- no session-teardown requirement;
- R18 Transport run already closed;
- R5 runtime ownership already released;
- R4 worker ownership already released;
- P7 consumer already retired;
- P3 run association still retained;
- START invoked;
- RETIRE invoked;
- exact RETIRE completion consumed;
- producer-done published;
- worker joined;
- Presentation, P2 and session media-clock associations still present.

For initial RESTORE_PENDING work, P3 must still expose the exact current
generation in `PSTVNC_MPEG_PRESENTATION_RETIRING`.

Fabricated ownership facts, wrong generation or impossible P3 state fail closed
without sealing or revealing.

### Explicit graphics-fresh RFB proof

R24 adds one run-scoped Application fact:

`rfb_restoration_presented`

and one narrow API:

`pstvnc_app_mpeg_run_record_restored_rfb_presented()`.

The marker can be recorded only for the exact current RESTORE_PENDING
generation after P2 proves the post-thaw FULL transaction is protocol-complete:

- P2 remains thawed;
- remote publication is allowed;
- no request is outstanding;
- `pstvnc_rfb_flow_policy_next_request()` reports ordinary INCREMENTAL.

The API contract explicitly requires its caller to record this fact only after
the authoritative FULL-refreshed RFB desktop has also crossed the existing
successful desktop presentation/upload boundary while retained MPEG is still
visually present.

Application does not infer this graphics fact from P2. It does not send the FULL
request, record request transmission, record update completion, clear P2 debt,
touch framebuffer storage, or call Platform graphics to manufacture freshness.

Focused evidence proves:

- FULL still pending rejects marker recording;
- FULL sent but response still outstanding rejects marker recording;
- once the real P2 FULL response is complete, reveal still remains blocked until
  the explicit graphics-presentation marker is recorded;
- wrong-generation marker recording fails closed.

### Seal and explicit REVEAL_PENDING authority

R24 adds:

`PSTVNC_APP_MPEG_RUN_REVEAL_PENDING`

as an explicit Application state.

`pstvnc_app_mpeg_run_reveal_restored()` requires, before first seal:

- exact retired execution authority;
- current P2 protocol freshness;
- the explicit run-scoped RFB graphics-presentation proof;
- exact P3 RETIRING snapshot for the current generation.

Only then does Application call
`pstvnc_mpeg_presentation_seal_retirement()`.

Application verifies P3 independently reports exact-generation
`PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING` and then records its own
REVEAL_PENDING state.

The seal is therefore attempted only once for a successful pending run. A later
retry never re-seals.

### Physical reveal uses only accepted compositor ownership

R24 invokes only:

`pstvnc_mpeg_compositor_reveal_retired()`

with the exact current generation.

Application does not call Platform graphics directly and does not duplicate
`pstvnc_mpeg_presentation_commit_reveal()`.

The accepted compositor remains sole owner of:

- cached desktop/local-overlay physical presentation without retained MPEG;
- the synchronization boundary;
- the exact final P3 logical commit to RFB_ONLY.

### Retryable pre-sync failure

Compositor:

- `PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED`;
- `PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID`;

are preserved as retryable Application outcomes:

- `PSTVNC_APP_MPEG_RUN_REVEAL_PLATFORM_FAILED`;
- `PSTVNC_APP_MPEG_RUN_REVEAL_SYNC_INVALID`.

Those outcomes preserve:

- Application REVEAL_PENDING;
- exact current generation;
- explicit RFB graphics-presentation proof;
- P3 REVEAL_PENDING retained snapshot;
- session_teardown_required == 0.

A later exact retry invokes compositor reveal again and does not call P3 seal a
second time.

Current P2 protocol freshness is rechecked on every reveal attempt. A new
ordinary RFB request in flight therefore blocks a reveal/retry until its P2
accounting is current again without discarding the run-scoped graphics proof.

### Nonretryable contradictions fail closed

Nonretryable compositor errors, including INVALID or
RETIREMENT_COMMIT_FAILED, cause Application fault/containing recovery rather
than false success.

An OK compositor return is also insufficient by itself. R24 requires:

- `effects.synchronized != 0`;
- `effects.retirement_revealed != 0`;
- P3 state exactly `RFB_ONLY`;
- P3 no longer owns an MPEG visual;
- no current P3 snapshot remains.

An OK/effects/P3 contradiction faults the coordinator and retains the current
Application generation/proof evidence rather than pretending reusable IDLE.

### Exact successful endpoint and generation reuse discipline

Only the exact synchronized compositor/P3 success path clears the current run.

R24 then returns the coordinator to:

`PSTVNC_APP_MPEG_RUN_IDLE`

with:

- `current_generation=0`;
- current run/transient ownership cleared;
- `last_allocated_generation` preserved;
- external P2 object still thawed;
- external session media-clock object byte-for-byte untouched.

Focused evidence then has the external caller re-establish the already-existing
start precondition by freezing P2 again and starts the same coordinator. The
next run receives generation N+1, proving that final reveal enables reuse
without generation reuse.

R24 does not reset persistent calibration/configuration ownership and does not
activate an ordinary trigger/main-loop path.

## Focused deterministic evidence

The final `tests/unit/app_mpeg_run_test.c` retains R21-R23C coverage and adds
R24 proof for:

1. IDLE/other non-RESTORE_PENDING state rejection before seal;
2. already-FAULTED rejection;
3. fabricated RESTORE_PENDING execution ownership fault before seal;
4. P2 FULL-pending marker rejection;
5. P2 outstanding-response marker rejection;
6. protocol-fresh but unpresented RFB reveal rejection;
7. wrong-generation restoration marker failure before seal;
8. exact marker + protocol freshness permitting one P3 seal;
9. seal-before-compositor event ordering;
10. exact-generation compositor invocation;
11. synchronized + retirement_revealed successful effects;
12. exact P3 RFB_ONLY/no-snapshot/no-MPEG-visual success proof;
13. external P2 remaining thawed after success;
14. external media-clock object remaining unchanged;
15. PLATFORM_FAILED retry preserving REVEAL_PENDING without teardown;
16. SYNC_INVALID second retry without a second seal;
17. eventual successful third reveal returning IDLE;
18. a new P2 incremental request blocking reveal before seal until completion;
19. wrong P3 generation faulting before seal;
20. compositor INVALID failing closed;
21. RETIREMENT_COMMIT_FAILED failing closed with P3 REVEAL_PENDING evidence;
22. contradictory compositor OK/effects failing closed rather than returning
    IDLE;
23. successful generation N clearing current-generation state while preserving
    generation history;
24. external P2 refreeze followed by the same coordinator allocating N+1.

The focused fixture continues to link the real accepted
`src/rfb/flow_policy.c` implementation. Presentation/compositor/worker/
Transport owners remain represented through their accepted public seams so the
fixture tests Application composition rather than duplicating lower mechanisms.

Canonical host execution independently retains the real P2, P3, compositor,
P7, MPEG worker/runtime/backend, Transport and ordinary Application regression
fixtures.

## Commits

The complete pre-log Worker range is six commits ahead / zero behind the
starting Foreman authority:

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

The reconciliation commit was generated by the repository-authorized
deterministic dictionary workflow and changed only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No concurrent branch movement or write collision occurred during the Worker
writes.

## Changed paths

The complete pre-log compare from
`5b329dd46066602cd95eb1813be226395d3d07a0` to
`d777835c53a0e829e423f76bd0f78097c88e0ffb` is six commits ahead / zero
behind and changes only:

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

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R24-C1 FINAL_RESTORE_ADMISSION_REQUIRES_EXACT_RETIRED_EXECUTION_AND_P3_RETIRING`
  — MET. Initial final-restoration admission proves exact retired execution,
  generation and P3 RETIRING state; fabricated ownership fails before seal.
- `A003-R24-C2 RFB_GRAPHICS_FRESHNESS_IS_EXPLICIT_RUN_SCOPED_PROOF_NOT_P2_INFERENCE`
  — MET. R24 introduces the run-scoped
  `rfb_restoration_presented` marker/API with an explicit successful graphics
  presentation caller contract; P2 freshness alone does not set it.
- `A003-R24-C3 FULL_PENDING_OUTSTANDING_FROZEN_OR_UNPRESENTED_RFB_BLOCKS_REVEAL`
  — MET. P2 protocol freshness requires thawed/publication-enabled, no
  outstanding request and INCREMENTAL next-request state; missing marker
  independently blocks seal/reveal.
- `A003-R24-C4 EXACT_P3_SEAL_OCCURS_ONLY_AFTER_FRESH_PRESENTED_RFB_PROOF` —
  MET. Focused event proof observes a single seal only after exact marker plus
  current protocol freshness.
- `A003-R24-C5 APPLICATION_HAS_EXPLICIT_REVEAL_PENDING_STATE_AFTER_SEAL` —
  MET. P3 seal is independently verified before Application records explicit
  REVEAL_PENDING.
- `A003-R24-C6 PHYSICAL_HANDOFF_USES_ONLY_EXACT_ACCEPTED_COMPOSITOR_REVEAL` —
  MET. Product source calls only the exact accepted compositor reveal seam;
  prohibited Platform/direct-P3-commit calls are absent.
- `A003-R24-C7 PRE_SYNC_REVEAL_FAILURE_IS_RETRYABLE_WITHOUT_RESEAL_OR_TEARDOWN`
  — MET. PLATFORM_FAILED and SYNC_INVALID focused cases preserve
  REVEAL_PENDING/generation/marker/P3 snapshot, do not require teardown and
  subsequent attempts contain no second seal.
- `A003-R24-C8 POST_SYNC_OR_STATE_CONTRADICTIONS_FAIL_CLOSED_WITH_EVIDENCE` —
  MET. INVALID, retirement-commit failure, wrong P3 authority and contradictory
  OK/effects fault without pretending IDLE.
- `A003-R24-C9 SUCCESS_REQUIRES_SYNCHRONIZED_REVEAL_AND_EXACT_P3_RFB_ONLY` —
  MET. Success requires synchronized + retirement_revealed effects, exact
  RFB_ONLY, no snapshot and no MPEG visual ownership.
- `A003-R24-C10 SUCCESS_CLEARS_ONLY_CURRENT_RUN_AND_PRESERVES_GENERATION_HISTORY`
  — MET. Exact success clears current transient ownership/current_generation,
  preserves last_allocated_generation and leaves the external clock object
  untouched.
- `A003-R24-C11 SAME_COORDINATOR_CAN_ALLOCATE_NEXT_GENERATION_AFTER_EXTERNAL_REFREEZE`
  — MET. Focused test externally refreezes P2 after generation N reveal and the
  same coordinator starts generation N+1.
- `A003-R24-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN_WITH_NO_SCOPE_CREEP`
  — MET. Exact final pre-log workflow `36080636735`, attempt 1, is fully green
  and final compare remains inside the authorized source/test/docs/dictionary
  surface.

All twelve R24 criteria are therefore `MET` for independent Foreman review.

## Machine evidence

The first complete source+focused-test authority
`d9e27a1179f9f8b65d409a8f5f0519a9c59d6207` ran GitHub Actions workflow
`36080449987`.

Observed there:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-long — expected strict failure only for newly introduced R24
  symbols before deterministic reconciliation.

The documentation authority
`6032262e7e8b71dcac3fd60832c3ffc16347138a` was followed by authorized
dictionary trigger
`14d0aeb07ffdc4b228f273c2dbc217d6fa01d4be`.

The deterministic workflow produced dictionary authority:

`c5e44ab5f418dd3eabf73a23a223919c293ab6e9`

changing only the root clean-source dictionary and generated portal.

Exact final pre-log authority:

`d777835c53a0e829e423f76bd0f78097c88e0ffb`

Workflow:

`36080636735`

Attempt:

`1`

Conclusion:

`SUCCESS`

Final jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected.

Observed host regressions include:

- `app_mpeg_run_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- Pi MPEG-generation fixture: 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`.

Observed project/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS records=204 grandfathered=9 format_compat=2 stamp_compat=1`;
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

R24 changes loadable identity from accepted R23C:

- R23C `PT_LOAD_SHA256=c7443e28a6eb4a6580a153768e816d4011b2b2784dabb8ef274954cb36c081fa`;
- R23C `PT_LOAD_BYTES=498964`.

The R24 identity is repository-reproducible build evidence only and does not
inherit physical qualification.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R24`
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

- Foreman State revision `0060`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Wire Runtime Decisions revision `0011`;
- Architecture Overlay revision `0007`;
- accepted corrected R23C source;
- accepted P2/P3/compositor/P7/R18/R20/R4/R5 seams.

Produced:

- no Foreman state revision;
- no architecture/contract revision;
- no Wire/protocol revision;
- no lower-owner mechanism revision;
- no ordinary MPEG product activation;
- no Pi MPEG product activation;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No R24 source blocker remains within the authorized packet.

The accepted P3/compositor and P2 public seams were sufficient; no lower-owner
implementation change was required.

The new R24 loadable identity remains hardware-pending. Governing authority
states hardware debt does not block unrelated source work; this Worker does not
convert host/build/reproducibility evidence into physical qualification.

Ordinary product trigger/service wiring remains deferred authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R24_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect the R24 source/evidence/log, decide
acceptance, and select any later bounded packet. Reconstruction must not
self-accept R24 or begin ordinary MPEG product activation.

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T16:47:02-04:00
COMPLETED_AT=2026-09-24T17:00:23-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=07b9dc4ec91c2947b7da62164bda6f214ab5b78e
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R22 Application MPEG live service

## Objective and recovered authority

This interactive Reconstruction worker recovered live
`ledge/h1-all-guns` authority at
`07b9dc4ec91c2947b7da62164bda6f214ab5b78e` and consumed Foreman State
revision `0057`, whose active packet was:

`A003-APPLICATION-MPEG-LIVE-SERVICE-R22`

with `PACKET_OWNER=RECONSTRUCTION` and
`ARCHITECTURE_BLOCKER=NONE`.

The worker also consumed Reconstruction Contract revision `0006`, Work Log
Contract revision `0007`, Architecture Overlay revision `0007`, Wire
Runtime Decisions revision `0011`, the accepted R21 source authority
`fc8bb2652d94d8163e1e5b375e33c9e37b73017b`, and the accepted R21/P7/P3
public seams named by State 0057.

External Pi/local checkout staging, untracked files and local worktree
cleanliness are not observable through this GitHub-connected worker seat.
No local `scripts/resume-state.sh` or local shell execution is claimed.

## Result

R22 is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log source authority:

`345e726effee8f01d1be71938e8f09efde030c0b`

R22 adds exactly one explicit Application live state:

`PSTVNC_APP_MPEG_RUN_MPEG_OWNED`

and exactly one trigger-agnostic live service API:

`pstvnc_app_mpeg_run_service()`

The service operation invokes only the existing P7 frame consumer for the
coordinator's exact current generation and returns the detailed P7 service
result unchanged for future caller/main-loop policy.

### Pre-first service

While Application is
`PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME`:

- P7 `IDLE` with `worker_finished == 0` preserves the exact waiting state
  and generation;
- P7 `PRESENTED` may promote Application only when the result proves
  synchronized physical presentation plus first-frame promotion, P7 status
  remains exact-generation/non-faulted, and P3 independently reports
  `MPEG_OWNED` for that exact generation;
- no frame receipt by itself is sufficient to promote Application.

Application never arms the media clock. P7/compositor remain the sole owners of
physical first-frame synchronization, media-clock arm, and P3 first-frame
promotion.

### Post-first live service

After exact first-frame proof, Application records
`PSTVNC_APP_MPEG_RUN_MPEG_OWNED`.

P7 `IDLE`, `WAIT`, `PRESENTED`, and `DROPPED` are ordinary live
outcomes. The wrapper preserves the exact generation and P7 service result,
including a WAIT claim, held ordinal, and absolute deadline.

The wrapper independently verifies that P7 status remains exact-generation and
non-faulted and that P3 remains exact-generation `MPEG_OWNED`.

### Fail-closed behavior

The Application run faults and requires outer session teardown on:

- negative P7 service results;
- a faulted/invalid P7 status after a benign service result;
- unexpected `worker_finished`;
- exact-generation contradictions;
- impossible P3/Application state;
- first-frame `PRESENTED` without exact synchronized promotion proof;
- mismatched P7 return/result or claim-status facts.

R22 deliberately performs no direct cleanup on these failures. It does not
clear P7, abort Presentation, stop/join/release the worker, release the MPEG
runtime, send RETIRE, finalize Transport, thaw/reveal RFB, or manufacture
successful retirement. Embedded lower-owner state/evidence is retained for
later failure/retirement orchestration.

## Scope preserved

The R22 behavioral diff is confined to:

- `src/app_mpeg_run.c`
- `src/app_mpeg_run.h`
- `tests/unit/app_mpeg_run_test.c`
- `docs/development/mpeg-generation-control.md`

Deterministic dictionary reconciliation additionally changed only:

- `src/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

R22 did not modify:

- `src/app.c`;
- `src/app_mpeg_frame.*`;
- Display/Presentation/compositor/scheduler implementation;
- MPEG worker/backend/runtime implementation;
- Transport implementation or Wire bytes;
- RFB flow policy;
- Configuration;
- Pi product source;
- AUDIO;
- Input/UI/calibration.

Retirement and ordinary MPEG product activation remain deferred.

## Focused deterministic evidence

The extended `tests/unit/app_mpeg_run_test.c` proves:

1. service from IDLE, FAULTED, or a fabricated live state without a successful
   R21 owner set rejects before P7 service;
2. pre-first benign P7 IDLE preserves exact WAIT_FIRST_FRAME state/generation
   with no cleanup side effect;
3. exact first P7 PRESENTED plus synchronized/first-frame-promoted effects and
   exact P3 MPEG_OWNED advances Application once to MPEG_OWNED;
4. first PRESENTED without exact promotion proof or exact P3 ownership faults;
5. post-first WAIT preserves exact claim/ordinal/deadline and MPEG_OWNED state;
6. post-first PRESENTED, DROPPED and IDLE preserve MPEG_OWNED authority;
7. IDLE with worker_finished faults both before and after first-frame promotion;
8. every current negative P7 result class from INVALID through FAULTED faults
   Application, requires teardown, and preserves returned/embedded P7 evidence;
9. wrong embedded P7 generation, impossible P3 state, and faulted P7 status fail
   closed;
10. failure tests prove R22 does not invoke Presentation abort, worker stop, or
    Transport pre-START abort as an Application cleanup response.

The same host-unit run also executes and passes the accepted R21 start tests,
P7 frame-consumer tests, P2/P3 Presentation/flow-policy tests, Transport
runtime/MPEG tests, MPEG decoder/worker/scheduler/compositor tests, Pi MPEG
generation tests, ordinary Application regressions, and the RFB provider
termination proof.

## Commits

The pre-log R22/source-evidence range is:

1. `b8fab72ad354b14d6f56f35f59baa0f4afea73b6` —
   `app: compose R22 MPEG live frame service`
2. `dea41fa46e0a22ee395336af22d1ee1084bf262e` —
   `tooling(symbols): run deterministic dictionary reconciliation`
3. `ed8887bd0b6199ab633a5154b3445e8516042529` —
   `docs(symbols): reconcile current clean definitions`
4. `345e726effee8f01d1be71938e8f09efde030c0b` —
   `test: verify final R22 MPEG live service authority`

The reconciliation commit was produced by the repository's authorized
dictionary automation after the worker's exact trigger commit. No unrelated
concurrent branch movement or write collision occurred.

## Acceptance-criterion Worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A003-R22-C1 RUN_OWNER_HAS_ONE_EXPLICIT_MPEG_OWNED_LIVE_STATE` — MET.
  One new Application live state is exposed and reported through existing run
  status.
- `A003-R22-C2 LIVE_SERVICE_USES_ONLY_EXISTING_EXACT_GENERATION_P7_CONSUMER` —
  MET. The new API calls P7 with `run->current_generation`; no P7 mechanics
  were copied or modified.
- `A003-R22-C3 PRE_FIRST_IDLE_PRESERVES_WAIT_FIRST_FRAME_WITHOUT_SIDE_EFFECT` —
  MET. Focused host proof preserves exact wait/generation and observes no
  cleanup event.
- `A003-R22-C4 FIRST_PRESENTED_FRAME_EXACTLY_PROMOTES_RUN_AFTER_P3_CONFIRMATION`
  — MET. Promotion requires exact P7 synchronized/first-frame effects plus
  independent exact-generation P3 MPEG_OWNED.
- `A003-R22-C5 MEDIA_CLOCK_AND_PHYSICAL_FIRST_SYNC_OWNERSHIP_REMAINS_P7_DISPLAY`
  — MET. R22 adds no media-clock arm or physical presentation action; P7 source
  is unchanged.
- `A003-R22-C6 MPEG_OWNED_IDLE_WAIT_PRESENTED_DROPPED_PRESERVE_LIVE_AUTHORITY` —
  MET. Focused host proof covers all four benign post-first outcomes and exact
  WAIT claim/deadline preservation.
- `A003-R22-C7 UNEXPECTED_WORKER_FINISH_IS_FAILURE_NOT_NORMAL_RETIREMENT` — MET.
  Focused proof faults before and after first-frame promotion.
- `A003-R22-C8 P7_OR_STATE_CONTRADICTION_FAULTS_WITHOUT_ERASING_OWNER_EVIDENCE`
  — MET. Negative P7 classes, P7 fault status, generation/P3 contradictions and
  exact returned evidence retention are covered.
- `A003-R22-C9 NO_RETIREMENT_STOP_FINALIZE_THAW_OR_REVEAL_SCOPE` — MET.
  R22 service failure only faults/sets teardown-required; no such orchestration
  was added.
- `A003-R22-C10 RUN_STATUS_REMAINS_EXACT_GENERATION_AND_STATE_AUTHORITY` — MET.
  Focused status checks prove WAIT_FIRST_FRAME and MPEG_OWNED with exact current
  generation.
- `A003-R22-C11 NO_ORDINARY_APP_PI_INPUT_AUDIO_OR_PROTOCOL_SCOPE_CREEP` — MET.
  Final diff contains no changes to those product/mechanism surfaces.
- `A003-R22-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.
  Exact final source head workflow `36058496825`, attempt 1, completed
  SUCCESS across host-unit, project-check, dictionary-long, ps2-compile and
  ps2-link/current-source reproducibility.

## Machine evidence

Initial behavioral source authority
`b8fab72ad354b14d6f56f35f59baa0f4afea73b6` ran workflow
`36058270541`:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link/current-source reproducibility — SUCCESS;
- dictionary-long — expected FAIL_STRICT only because the 31 newly introduced
  R22 source symbols had not yet been reconciled.

The exact dictionary failure reported only `MISSING_DICTIONARY_SYMBOL` entries;
definition discovery was READY. The canonical reconciliation trigger then
generated `ed8887bd...`, changing only the root source dictionary and portal.

Final exact pre-log source authority
`345e726effee8f01d1be71938e8f09efde030c0b`:

- workflow: `Ledge reconstruction checks`;
- run: `36058496825`;
- attempt: `1`;
- conclusion: `success`;
- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected on the final non-trigger commit.

Observed final host evidence includes:

- `app_mpeg_run_test: PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `transport_runtime_test: PASS`;
- `transport_mpeg_test: PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- Pi MPEG generation fixture: 12 tests, OK;
- `app R15/R16B/R19 tests: PASS`;
- `transport_rfb_provider_failure_test: PASS`.

Observed project/dictionary/build evidence includes:

- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Final linked identity:

- `ELF_PRISTINE_SHA256=71412c76f5f748913b1f8247e6e1c18c5e4a4ffe7e8a89da98f65c6f343ab5d1`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=543f14c376e4a35b95cebaaa54de44a409c6ec0ebe247c9b63d34b7358beace8`
- `PT_LOAD_BYTES=496404`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R22 changes loadable bytes from accepted R21
(`PT_LOAD_SHA256=910597af49fe2121ab730a2db82aa4259177256156062853bd2a008f0d4d6045`,
`PT_LOAD_BYTES=494996`). The new R22 identity is repository-reproducible build
evidence only and does not inherit physical qualification.

Evidence classification:

`SOURCE_COMPLETE=YES_WITHIN_R22`
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

- Foreman State revision `0057`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- Architecture Overlay revision `0007`;
- Wire Runtime Decisions revision `0011`;
- accepted R21 run-start and P7/P3/lower-owner seams.

Produced:

- no Foreman state revision;
- no contract revision;
- no Wire/protocol revision;
- no retirement;
- no ordinary product activation;
- no Foreman acceptance;
- no independent Validation result.

## Findings / blockers

No R22 source blocker remains within the authorized packet.

The newly changed R22 loadable identity remains hardware-pending. Current
governing authority explicitly states that such debt does not block unrelated
source work; this Worker does not convert reproducible build evidence into
hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R22_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION`

The Reconstruction worker stops here. The Foreman must independently recover
live repository authority, inspect this source/evidence/log, decide R22
acceptance, and select any later bounded packet. Reconstruction must not
self-accept R22 or begin retirement.

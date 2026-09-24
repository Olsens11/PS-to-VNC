DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-24T12:50:11-04:00
COMPLETED_AT=2026-09-24T12:58:27-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=fcdc8789bddca31c0ee2792177aaa4843a38e299
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R20E fail-closed Transport drain outcome

## Objective and authority consumed

This interactive Reconstruction worker recovered current authority from
`ledge/h1-all-guns` and consumed Foreman State revision `0055`, whose only
active Reconstruction packet was:

`A001-TRANSPORT-DRAIN-FAIL-CLOSED-R20E`

The assigning branch authority was:

`fcdc8789bddca31c0ee2792177aaa4843a38e299`

R20E was activated after Foreman review accepted the normal R20C/R20D
synchronization shape but found that a failed outbound-submitter drain could
still be followed by signaling `receiver_done_semaphore_id`, allowing an
unproven drain to masquerade as reclaim completion.

R20E therefore had to preserve the sound R20C/R20D normal path while making
drain failure or contradiction permanently non-reclaiming. R21 remained queued
and was not authorized.

Governing authority consumed included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` revision 0006;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0055;
- accepted R20 private MPEG identity authority;
- preserved R20C/R20D correction candidate source/log authority;
- current `src/transport/runtime.*`, Transport lifecycle tests and directly
  affected lifecycle documentation.

The GitHub connector exposes repository/branch authority but not the external
developer Pi worktree. External staged/unstaged/untracked state remains
`PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR`; no claim is made about
external checkout cleanliness.

## Result

R20E is source-complete as a Reconstruction candidate for independent Foreman
review at final pre-log authority:

`316ad217bef229c9ca0134b9b922a213cb5af247`

The correction introduces one Transport-private receiver-completion outcome:

- `PENDING` — no final terminal outcome has been published;
- `PROVEN` — R20C owner-terminal work completed and R20D submitter drain
  actually proved count zero;
- `FAILED` — reclaim proof is absent and can never be upgraded for this
  runtime.

The receiver completion semaphore is now a terminal-outcome rendezvous rather
than unconditional reclaim-success authority.

Only `PROVEN` permits
`pstvnc_transport_runtime_wait_receiver_done()` or
`pstvnc_transport_runtime_release()` to return reclaim success.

A `FAILED` outcome may still wake waiters so they return failure rather than
hang, but repeated observation, later submitter count zero, or retry cannot
convert that runtime into reclaim authority.

## Fail-closed drain behavior

The accepted R20D registration/count semantics are unchanged.

A submitter still:

1. atomically registers before any outbound slot/ready/done semaphore touch;
2. remains registered while waiting for the slot, owning the slot, waiting for
   outbound completion and releasing the slot;
3. unregisters only after its final outbound-rendezvous touch.

Receiver terminality still atomically closes new admission.

R20E changes only the final outcome:

- successful R20D drain -> `PROVEN`;
- drain `WaitSema()` failure -> `FAILED`;
- drain wake followed by protected nonzero submitter count -> `FAILED`;
- receiver-completion event publication failure -> `FAILED`.

On drain-wait failure the private
`outbound_submitter_drain_waiting` fact is cleared truthfully before the helper
returns failure.

The completion-fence observer now:

1. returns failure immediately for already-latched `FAILED`;
2. otherwise waits on the terminal-outcome event;
3. restores the latched event;
4. succeeds only if the observed private outcome is exactly `PROVEN`;
5. converts any impossible non-PROVEN event or rendezvous failure to
   irreversible `FAILED`.

No timeout, sleep, priority assumption, diagnostic counter or repeated
observation substitutes for proof.

## Deterministic R20E failure evidence

The host Transport fixture now includes two direct failure proofs.

### Drain wait failure

`test_outbound_drain_wait_failure_is_permanently_nonreclaiming()`:

1. starts a valid runtime;
2. holds the sole I/O owner inside one readiness pass;
3. registers one direct outbound submitter and proves it is blocked on
   outbound-done;
4. holds that submitter after terminal outbound-done wake but before its final
   slot release;
5. injects failure only into the private drain `WaitSema()`;
6. drives receiver terminality with the existing invalid-frame mechanism;
7. proves the receiver publishes `FAILED`, not reclaim success;
8. proves repeated `wait_receiver_done()` calls return failure;
9. proves repeated `release()` calls return failure;
10. proves no `ReferThreadStatus`, `TerminateThread`, `DeleteThread`,
    physical release, queue/stack reclaim or outbound/drain semaphore deletion
    occurs;
11. later permits the submitter to finish and count to reach zero;
12. proves that later truth cannot upgrade the already-failed outcome.

### Contradictory drain wake

`test_outbound_drain_false_wake_is_permanently_nonreclaiming()`:

1. registers a real outbound submitter before terminality;
2. seeds one impossible private drain token while that submitter is still live;
3. holds the submitter after its terminal done wake;
4. lets the receiver consume the false drain token;
5. proves the protected submitter count is still nonzero;
6. proves the contradiction latches `FAILED`;
7. proves no reclaim operation or resource deletion occurs;
8. later lets the submitter leave and count reach zero;
9. proves repeated wait/release still cannot become success.

The retained R20C/R20D normal synchronization tests remain green, including the
active+queued submitter drain and post-terminal caller rejection.

Fresh-runtime reuse now explicitly proves
`receiver_completion_outcome == PENDING` after a genuinely successful prior
release/reinitialize path.

## Commits

The complete pre-log R20E range is six commits ahead / zero behind the assigning
Foreman authority:

1. `1a9cb57d49eb257a7e89ed2ef8e66017688b44ea` —
   `transport: fail closed on unproven submitter drain`
2. `fb0a987989899c3c0b458b1d12c7fbb19d926f47` —
   `test(transport): prove R20E drain failures stay nonreclaiming`
3. `fd995cb9df82313f33fabd4a72245ac21fd0ae2e` —
   `docs(transport): record R20E fail-closed completion outcome`
4. `04471124d942a88cb0d131c279058f0cfb841d3a` —
   `tooling(symbols): run deterministic dictionary reconciliation`
5. `1f4eccfbe910ddd545d0ab633db2c90f6a6cf394` —
   `docs(symbols): reconcile current clean definitions`
6. `316ad217bef229c9ca0134b9b922a213cb5af247` —
   `test: verify final R20E drain failure authority`

## Changed paths

The complete assigning-Foreman-to-pre-log diff is confined to R20E-authorized
surfaces:

- `src/transport/runtime.c`
- `src/transport/runtime.h`
- `tests/unit/transport_runtime_test.c`
- `docs/development/module-lifecycle.md`
- `src/transport/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

Notably unchanged:

- `src/app.c` and all R21 Application source;
- `src/app_mpeg_frame.*`;
- Pi product source;
- Transport protocol/frame codecs;
- Transport bridge R20 MPEG semantic API;
- RFB parser/session/flow-policy product source;
- AUDIO product policy;
- MPEG decoder/worker/backend;
- Display/Presentation;
- calibration/Input/UI;
- Configuration product source.

## Acceptance-criterion worker dispositions

These are Reconstruction dispositions for independent Foreman review, not
Foreman acceptance.

- `A001-R20E-C1 RECLAIM_SUCCESS_REQUIRES_EXPLICIT_SUCCESSFUL_DRAIN_PROOF` — MET.
- `A001-R20E-C2 DRAIN_WAIT_FAILURE_CANNOT_PUBLISH_SUCCESSFUL_COMPLETION` — MET.
- `A001-R20E-C3 NONZERO_COUNT_AFTER_DRAIN_WAKE_CANNOT_PUBLISH_SUCCESS` — MET.
- `A001-R20E-C4 WAIT_RECEIVER_DONE_NEVER_SUCCEEDS_FROM_UNPROVEN_DRAIN` — MET.
- `A001-R20E-C5 RELEASE_PRESERVES_ALL_OWNERSHIP_ON_UNPROVEN_DRAIN` — MET.
- `A001-R20E-C6 NORMAL_R20D_ACTIVE_AND_QUEUED_DRAIN_BEHAVIOR_UNCHANGED` — MET.
- `A001-R20E-C7 SUCCESSFUL_COMPLETION_REMAINS_LATCHED_AND_REOBSERVABLE` — MET.
- `A001-R20E-C8 FAILED_PATH_CANNOT_BECOME_FRESH_SESSION_AUTHORITY` — MET.
- `A001-R20E-C9 R20C_FAILED_SHUTDOWN_AND_POST_PROOF_RETRY_CONTRACTS_UNCHANGED` — MET.
- `A001-R20E-C10 NO_TIMEOUT_SLEEP_OR_DIAGNOSTIC_SUCCESS_SUBSTITUTE` — MET.
- `A001-R20E-C11 NO_APPLICATION_PI_MEDIA_PRESENTATION_OR_PROTOCOL_SCOPE_CREEP` — MET.
- `A001-R20E-C12 HOST_PROJECT_DICTIONARY_PS2_BUILD_EVIDENCE_GREEN` — MET.

## Exact final machine evidence

Final pre-log source authority:

`316ad217bef229c9ca0134b9b922a213cb5af247`

Canonical workflow:

- workflow: `Ledge reconstruction checks`
- run: `36030820124`
- final observed attempt: `2`
- final conclusion: `success`
- head SHA: `316ad217bef229c9ca0134b9b922a213cb5af247`

Attempt 1 completed all canonical gates successfully before the repeated-host
evidence was requested.

Attempt 2 reran the same exact source without source, delay, timeout or fixture
changes and again completed successfully.

Attempt-2 job results:

- `host-unit` — SUCCESS
  - `transport protocol tests passed`
  - `transport bridge tests passed`
  - `transport_runtime_test: PASS`
  - `transport_mpeg_test: PASS`
  - `RFB_FLOW_POLICY_TEST=PASS`
  - Pi R17 MPEG generation fixture: 12 tests, OK
  - `app R15/R16B/R19 tests: PASS`
  - `transport_rfb_provider_failure_test: PASS`
- `project-check` — SUCCESS
  - `WORK_LOG_CHECK=PASS`
  - `SOURCE_DICTIONARIES=PASS`
  - `PS_TO_VNC_PROJECT_CHECK=PASS`
- `dictionary-long` — SUCCESS
  - `SOURCE_DICTIONARY_CHECK_MODE=LONG`
  - `SOURCE_DICTIONARIES=PASS`
- `ps2-compile` — SUCCESS
- `ps2-link` — SUCCESS
  - `ISSUE7_LINKED_BUILD=PASS`
  - `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`
- `dictionary-reconcile` — SKIPPED as expected on the non-trigger final
  evidence commit.

Final linked identity:

- `ELF_PRISTINE_SHA256=c838f0afda456c8026f8ea30afed1aad68eb7b0480142d0a3d956ed0e636274c`
- `PT_LOAD_SEGMENTS=1`
- `PT_LOAD_SHA256=c280849d1310b0a530daa77da736f22b0eae175a0adbdc43a0cb856c3c560adc`
- `PT_LOAD_BYTES=492308`
- `PS2IP_SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`

R20D candidate authority had:

`PT_LOAD_SHA256=df81e351eb67b77560a124279e7cb664843462f89581bb57aa7fc9993125d8d9`
`PT_LOAD_BYTES=492052`

R20E therefore changes loadable bytes and creates a newer provisional hardware
debt identity. Repository reproducibility does not transfer physical
qualification.

## Dictionary reconciliation

The canonical deterministic reconciler was triggered at:

`04471124d942a88cb0d131c279058f0cfb841d3a`

It produced:

`1f4eccfbe910ddd545d0ab633db2c90f6a6cf394`

and changed only:

- `src/transport/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No product source changed during reconciliation.

## Evidence gaps / hardware debt

- `PENDING_LOCAL_NOT_OBSERVABLE_VIA_GITHUB_CONNECTOR` — external Pi worktree
  cleanliness/staging state was not observable.
- `R20E_INDEPENDENT_VALIDATION=NOT_RUN`
- `R20E_OPERATOR_OBSERVED=NO`
- `R20E_HARDWARE_QUALIFIED=NO`
- `HARDWARE_PENDING=YES`

Machine/host evidence proves lifecycle behavior and reproducibility only.

## State/contract accounting

Consumed:

- Foreman State revision `0055`;
- Reconstruction Contract revision `0006`;
- Work Log Contract revision `0007`;
- accepted R20 authority;
- preserved R20C/R20D normal synchronization candidate authority.

Produced:

- no Foreman state revision;
- no Reconstruction-contract revision;
- no Wire/protocol version revision;
- no R21 source;
- no packet selection or Foreman acceptance.

## Findings / blockers

R20E has no remaining known source blocker within the authorized packet.

The cumulative candidate reclaim fence now has three explicit layers:

1. early `receiver_done` closes new outbound admission;
2. R20C completes sole-I/O-owner terminal/wakeup work;
3. R20D drains every pre-terminal outbound submitter;
4. R20E requires explicit `PROVEN` drain outcome before reclaim and latches any
   unproven drain as permanently non-reclaiming.

R21 remains unexecuted and dependency-queued. It requires independent Foreman
review/acceptance of the cumulative R20C/R20D/R20E correction before becoming
Reconstruction authority.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R20E_REVIEW_ACCEPTANCE_AND_R21_DEPENDENCY_DECISION`

The Reconstruction worker stops here. The Foreman must independently recover
current repository authority, inspect the R20E source/evidence/log, decide
acceptance, update Foreman-owned state as appropriate, and decide whether queued
R21 may become active. Reconstruction must not self-accept R20E or begin R21
from this record.

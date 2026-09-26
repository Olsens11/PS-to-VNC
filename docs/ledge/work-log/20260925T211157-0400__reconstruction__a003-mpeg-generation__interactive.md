DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T21:11:57-04:00
COMPLETED_AT=2026-09-25T21:27:57-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=82cd9affd068ec835c1d2c1a930cedc4c1b95bdd
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Reconstruction shift — R34P pre-START partial MPEG session dormancy

## Recovered authority

This shift independently recovered live `ledge/h1-all-guns` authority before
behavior-bearing writes.

Starting branch authority:

`82cd9affd068ec835c1d2c1a930cedc4c1b95bdd`

message:

`docs(work-log): review R34 block and hand off R34P dormancy`

Current Foreman State revision consumed:

`0074`

Exact active packet:

`PACKET_ID=A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P`

with:

- `PACKET_STATUS=ACTIVE`;
- `PACKET_OWNER=RECONSTRUCTION`;
- `WORK_ITEM_KEY=a003-mpeg-generation`;
- `WORKER_KEY=interactive`;
- `EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`;
- `USER_TERMINAL_POLICY=EXCEPTION_ONLY`;
- `PI_LOCAL_USER_PROXY_REQUIRED=NO`;
- `R34_APPLICATION_CONTINUATION=DEFERRED`;
- `R34_PARTIAL_APPLICATION_SOURCE_MUTATION=FORBIDDEN`;
- `NORMAL_MPEG_RETIREMENT_R23_R24=UNCHANGED`;
- `TRANSPORT_R33_TWO_PHASE_ABORT=UNCHANGED`.

The packet was based on accepted R21 source
`fc8bb2652d94d8163e1e5b375e33c9e37b73017b`, accepted R33 source
`c48d460aa3d2e095d079289d07321f96151113a9`, blocked R34 partial source
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`, and blocked R34 log
`488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4`.

The GitHub-connected Worker seat cannot observe an external Pi/local checkout's
untracked files, staging area or dirtiness. No local reset, clean, stash or
overwrite is claimed.

## Objective completed

R34P closes the lower-owner lifecycle gap that blocked R34: after the enclosing
Transport session has been terminalized while its runtime storage is retained,
the Application MPEG run owner can now prove local dormancy for each reachable
R21 pre-START cleanup-failure prefix without synthesizing START, normal
retirement, producer completion, Transport MPEG finalization, P2 thaw or P3
seal/reveal semantics.

The implementation deliberately does **not** resume or modify ordinary R34
Application composition. R34 remains deferred/unaccepted pending independent
Foreman review and a later bounded continuation decision.

## Reachable pre-START ownership classification

Accepted R21 start order was reconstructed directly from source and focused
tests.

Before the irreversible START boundary, R21 may own, in order:

1. one exact Transport MPEG run;
2. one PS2 worker runtime;
3. one MPEG worker;
4. optional armed P3 presentation;
5. optional initialized P7 frame consumer;
6. only then `start_invoked=1` and Transport START.

The accepted reverse unwind already handles ordinary failures. When that unwind
itself cannot prove cleanup, reachable retained asynchronous prefixes include:

- a truly started exact-generation worker after failed stop/join/release,
  necessarily with its PS2 worker runtime and open Transport MPEG run still
  represented;
- a started worker already joined internally but not released because concrete
  thread destruction/release failed;
- a create-success/start-failure worker whose immediate destroy also failed,
  leaving an initialized created-but-never-started thread plus stack;
- residual PS2 worker-runtime ownership after worker cleanup;
- residual open Transport MPEG-run ownership after later cleanup failure;
- later pure local diagnostic/P3 evidence after asynchronous worker/runtime
  owners have already been retired.

A successful P7 initialization has no pre-START failure edge before
`start_invoked=1`. If P7 initialization fails, R21 never publishes
`frame_consumer_initialized`. Therefore an admitted pre-START R34P prefix
cannot contain an Application-serviced live P7 claim.

Clean R21 pre-START rollback still clears the attempt and returns to IDLE; it is
not admitted by the abnormal R34P session-abort path.

## MPEG worker owner change

Added one narrow public worker-owner seam:

`pstvnc_mpeg_worker_reclaim_unstarted()`

in:

- `src/mpeg/worker.c`;
- `src/mpeg/worker.h`.

It accepts only the exact create-success/start-failure/destroy-failure shape:

- exact generation;
- worker initialized;
- thread created;
- thread never started;
- thread never joined;
- thread not already destroyed;
- valid retained thread ID;
- retained worker stack;
- no stop request;
- no live decoder;
- no worker-finished fact;
- EMPTY frame slot;
- no terminal worker outcome.

The seam retries only the injected thread-destroy operation. A destroy failure
returns `PSTVNC_MPEG_WORKER_THREAD_DESTROY_FAILED` and leaves thread ID,
created ownership, stack and initialized worker ownership intact.

Only successful concrete destruction:

- marks the thread destroyed/not-created;
- clears its thread ID;
- releases the exact worker stack;
- clears worker initialized ownership.

It never fabricates join, stop, worker-finished, decoder outcome or a normal
terminal worker outcome.

The existing PS2 worker-runtime thread-destroy adapter already re-proves kernel
`THS_DORMANT` before `DeleteThread` when dormancy was not previously proven.
No Platform source change was required.

## Application MPEG run owner change

`src/app_mpeg_run.c/.h` now keeps the accepted post-START R33 admission
contract separately from an exact pre-START abnormal-admission contract.

The post-START validator retains R33's original full live-owner requirements.

The new pre-START admission requires all relevant facts, including:

- run state FAULTED;
- `session_teardown_required`;
- nonzero current generation;
- exact old Transport access ticket;
- `start_invoked=0`;
- no initialized P7 consumer;
- no retirement/producers/restoration facts;
- worker/runtime/Transport combinations matching reachable R21 cleanup prefixes;
- exact created-but-never-started worker shape when that partial owner exists;
- PS2 runtime ownership flag consistent with the runtime owner's own
  `resources_owned` evidence.

The session-abort service still proves
`pstvnc_transport_session_abort_storage_retained()` **before** local worker or
runtime reclamation.

For a truly started pre-START worker:

- if the original unwind did not join it, the existing stop/status/join path is
  used;
- stop is requested at most once per abort epoch;
- true worker completion is required; there is no timeout-as-success;
- exact terminal outcome is preserved;
- worker release remains owner-driven and retryable;
- PS2 worker-runtime release remains later than worker no-touch proof.

If the original unwind already joined the worker and failed only during
release, R34P observes the worker's truthful joined/finished/no-decoder/stop
facts and does not issue a second stop or join. It then resumes at outcome and
worker release.

For an initialized created-but-never-started partial worker, the Application
run owner delegates only to
`pstvnc_mpeg_worker_reclaim_unstarted()`. Failed destruction remains
represented and retryable.

A successful abnormal pre-START cleanup reaches
`PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY` while preserving
`start_invoked=0` and old-run diagnostic/generation evidence. It does not
clear the run into reusable IDLE.

## Post-START R33 preservation

Existing post-START R33 meaning remains intact.

The historical source-boundary check was narrowed only because R34P adds an
earlier pre-START status-only probe. It now anchors R33's ordering proof at the
actual post-START stop branch and continues to assert:

retained Transport proof -> P7 claim abandonment -> worker stop -> worker status
-> join -> outcome -> worker release -> PS2 runtime release.

All existing R33 behavior tests remain green.

The R33/R34P abnormal service still contains no:

- RETIRE send/take;
- producer-done publication;
- Transport MPEG run finalize;
- P2 thaw;
- P3 retirement seal;
- compositor reveal;
- P7 normal frame service.

## Focused deterministic evidence

Worker tests prove:

- create success + start failure + initial destroy failure retains the exact
  partial worker;
- wrong-generation reclaim cannot mutate it;
- ordinary started-worker release/reclaim paths are not confused;
- repeated destroy failure retains thread ID/created state/stack;
- successful later destroy releases the stack exactly once and clears only the
  partial owner;
- the partial path never joins or fabricates worker outcome.

Application run tests prove:

- the existing R21
  `test_cleanup_failure_faults_and_blocks_retry()` started-worker shape can
  progress only after retained Transport proof;
- retained-Transport proof failure performs no worker/runtime reclamation;
- a pre-START stop failure can be retried after session terminality;
- a still-running worker remains pending rather than timeout-success;
- join, worker release and runtime release failures stay represented and
  retryable at their owner boundaries;
- an already-joined pre-START worker is not stopped/joined twice;
- the never-started partial worker uses only the new reclaim seam;
- a pre-START fault whose worker/runtime owners were already cleaned does not
  fabricate worker outcome;
- clean pre-START rollback remains IDLE and is rejected by abnormal abort;
- no START, RETIRE, producer-done, finalize, P2 thaw, P3 seal or reveal is
  emitted by pre-START abnormal cleanup;
- existing post-START R33 regressions remain green.

## Commit range

From starting authority
`82cd9affd068ec835c1d2c1a930cedc4c1b95bdd`
to final pre-log source/document authority
`c6e1dcf514adfc29296633374e75020b3480e5e0`,
GitHub compare reports thirteen commits ahead and zero behind:

1. `9038b46b65e35e1cc266b6188c3e251a704ec4cb` —
   `mpeg(worker): define retryable unstarted reclaim seam`;
2. `88d00e5736a6acfded3389186b01a2096d72b5e7` —
   `mpeg(worker): reclaim exact never-started partial owner`;
3. `1243fdbbe509610c9167ffc0bea1575eeee51426` —
   `test(mpeg): prove exact partial-start worker reclamation`;
4. `a866bb87c92ec3dbb22fd4ce86ee43573ed050cc` —
   `app(mpeg): admit exact pre-start abort prefixes`;
5. `5ac5d9c81822401fd7e01f4c32838ba9cd169c30` —
   `app(mpeg): document pre-start session dormancy contract`;
6. `7472349ace0d57bae2ca52e369a7eee04aa12d0b` —
   `test(app): prove R34P pre-start session dormancy`;
7. `41a71124bfa4e556039dbccd2652b2989b39ee6c` —
   `app(mpeg): place partial reclaim only in session abort`;
8. `aa1121c41bff27cedf94a3aeebdfeacdf1f1b344` —
   `test(app): preserve post-start R33 ordering under R34P`;
9. `0769ae53d9c02735eec661d1a21430e54ce80f64` —
   `test(mpeg): measure partial-reclaim release delta`;
10. `ff1338a04e3238f1099326e6fae7af8f5f690103` —
    exact empty trigger
    `tooling(symbols): run deterministic dictionary reconciliation`;
11. `e8cc97afe7bb3e40a1dbcd090cf19f2815674f3c` —
    automation-generated
    `docs(symbols): reconcile current clean definitions`;
12. `3f6f34d862c8c63e6a1677576833cca56251c84a` —
    `docs(mpeg): define R34P pre-start dormancy contract`;
13. `c6e1dcf514adfc29296633374e75020b3480e5e0` —
    `docs(mpeg): record partial-worker reclaim ownership`.

An initial implementation placement caused the new partial-reclaim block to
appear in R21's original pre-START unwind. The first compile run caught the
result. Commit `41a71124...` removed that block from R21 unwind and placed it
only in retained-session abort. The final source therefore preserves R21's
accepted fail-closed unwind behavior.

## Final changed-path scope

The complete start-to-pre-log changed path set is:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `src/mpeg/worker.c`;
- `src/mpeg/worker.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `tests/unit/mpeg_worker_test.c`;
- `tests/unit/app_mpeg_session_abort_source_test.py`;
- `docs/development/module-lifecycle.md`;
- `docs/development/source-topology.md`;
- generated/reconciled `src/SYMBOLS.md`;
- generated/reconciled `src/mpeg/SYMBOLS.md`;
- generated/reconciled
  `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `src/app.c/.h`, `src/app_mpeg_product.c/.h`,
`src/app_product_bindings.*`, P9/P10 source, Input, UI, Configuration,
Management, Transport, RFB, Display/compositor, Pi, AUDIO or H1/B4A forensic
source changed in this R34P range.

The dictionary reconciliation intentionally includes metadata for the
already-landed blocked R34 files so branch-wide topology/dictionary gates are
truthful. It does not change or accept R34 product behavior.

## Canonical final evidence

Exact final pre-log source authority:

`c6e1dcf514adfc29296633374e75020b3480e5e0`

Canonical GitHub Actions run:

`36208396947`

Run conclusion:

`SUCCESS`

Jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected on the final non-trigger commit.

Observed host output includes:

- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `INPUT_RUNTIME_PRODUCT_ACTION_SOURCE_TEST=PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- all broader host-unit regressions observed in the canonical job PASS.

Project evidence includes:

- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=232 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

Strict long dictionary job:

`SOURCE_DICTIONARIES=PASS`.

Pinned PS2 compile explicitly includes the modified
`src/app_mpeg_run.c` and `src/mpeg/worker.c`, and reports:

`CLEAN_PS2_COMPILE_CHECK=PASS`.

Linked/current-source reproducibility is deterministic twice:

`ELF_PRISTINE_SHA256=a7127b71ede10efa4a91e1914dc55a64bf75d6dffe7169df825d444c77615ccd`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`

`PT_LOAD_BYTES=525716`

`ISSUE7_LINKED_BUILD=PASS`

`LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

These loadable bytes differ from both the newest fully Foreman-accepted R33
identity and the blocked R34 partial identity. They are therefore
**HARDWARE_PENDING** and are not physically qualified.

No hardware qualification, operator observation or independent Validation was
performed or claimed.

## Required-behavior disposition

These are Reconstruction Worker dispositions only; they are not Foreman
acceptance.

1. EXACT_REACHABLE_PRESTART_RESIDUAL_SHAPES — MET.
2. WORKER_OWNER_RECLAIMS_NEVER_STARTED_PARTIAL — MET.
3. FAILED_PARTIAL_RECLAIM_STAYS_OWNED — MET.
4. PRESTART_APPLICATION_ABORT_ADMISSION_IS_EXACT — MET.
5. RETAINED_TRANSPORT_PROOF_COMES_FIRST — MET.
6. STARTED_WORKER_RETIREMENT_REMAINS_PROOF_DRIVEN — MET.
7. NO_OUTSTANDING_LIVE_BORROW_IS_ERASED — MET.
8. PS2_WORKER_RUNTIME_RELEASE_FOLLOWS_WORKER_NO_TOUCH — MET.
9. ENCLOSING_SESSION_SEMANTICS_REMAIN_ABNORMAL — MET.
10. ABORT_READY_PROVES_LOCAL_DORMANCY_NOT_NORMAL_RUN_SUCCESS — MET.
11. POSTSTART_R33_MEANING_REMAINS_UNCHANGED — MET.
12. BRANCH_WIDE_DETERMINISTIC_GATES_RESTORED — MET.

All twelve packet requirements are MET at the Reconstruction Worker level.

## Evidence classification

`SOURCE_COMPLETE=YES_WITHIN_R34P`

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

`R34_APPLICATION_CONTINUATION=DEFERRED`

`R34_PRODUCT_ACCEPTED=NO`

`LOCAL_WORKTREE_STATUS=NOT_OBSERVABLE`

## State and scope accounting

Consumed:

- Foreman State revision `0074`;
- accepted R21 start/unwind authority;
- accepted post-START R33 abnormal-session retirement;
- blocked R34 prerequisite finding;
- Work Log Contract revision `0007`.

Produced:

- exact pre-START residual ownership classifier;
- retryable never-started worker reclaim seam;
- proof-driven pre-START abnormal session dormancy;
- focused worker/run retry evidence;
- preserved post-START R33 source and behavior proof;
- reconciled current branch dictionaries/topology;
- lifecycle/topology documentation;
- green complete canonical build/test evidence.

Not produced:

- no Foreman State update;
- no Foreman acceptance;
- no R34 Application continuation;
- no R34 product acceptance;
- no normal R23/R24 retirement/reveal changes;
- no Transport two-phase-abort mechanism changes;
- no Input/UI/config/management/Pi/AUDIO behavior;
- no hardware qualification.

## Next pickup

`NEXT_PICKUP=FOREMAN_INDEPENDENT_R34P_REVIEW_ACCEPTANCE_AND_R34_RESUME_DECISION`

The Reconstruction Worker stops here. The Foreman must independently recover
live repository authority, verify this R34P source/evidence rather than relying
on this report, decide R34P acceptance, and only then decide whether the blocked
R34 Application packet may resume.

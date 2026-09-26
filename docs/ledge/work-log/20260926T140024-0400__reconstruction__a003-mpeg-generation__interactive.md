# Ledge Reconstruction work log — R35P partial-retirement session dormancy

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T14:00:24-04:00
COMPLETED_AT=2026-09-26T14:13:47-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=ec6280390c1ec4afc5cf9096b2bc7cd0fb51c8f8
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Reconstruction round received the baton at live branch authority
`ec6280390c1ec4afc5cf9096b2bc7cd0fb51c8f8`, current Foreman State revision
0076.

The exact active packet was:

`A003-MPEG-PARTIAL-RETIREMENT-SESSION-DORMANCY-R35P`.

The packet is Reconstruction-owned, uses `WORK_ITEM_KEY=a003-mpeg-generation`
and `WORKER_KEY=interactive`, and authorizes only the bounded Application MPEG
run/session-abort prerequisite needed before ordinary product retirement.

The round independently recovered accepted R23/R24, R33, R34P and R34 authority
and confirmed the exact gap: accepted normal R23/R24 failure prefixes can already
retire P7, worker, worker runtime or Transport MPEG-run ownership, while the
existing R33 admission required the original full post-START live-owner shape.

No user terminal, Pi-local proxy, hardware action, Foreman-state mutation,
ordinary product retirement, recalibration policy or Validation work was
performed.

## Final pre-log source authority

Final pre-log source/test/dictionary head:

`6fc0c755d4a3d51a8b5f87ad12c553b2e74b7385`
— `test(app): prove R35P outcome retry prefix`.

The complete R35P range from the assigning Foreman/log head is nine commits
ahead and zero behind:

1. `9f43e394606ee0e03f2677c0e222d0ad50ea733b`
   — `app(mpeg): model partial retirement abort evidence`
2. `6f8aceaa7cc0f3299a4fcec77e3a9dee66042d92`
   — `app(mpeg): resume abort from partial retirement owners`
3. `1be64750a865739c25ca47a36821329e1aaab457`
   — `fix(app): keep retirement outcome on run owner`
4. `9393ab20afda2c0f01fa9993ac614c4048101e62`
   — `test(app): prove R35P partial retirement dormancy`
5. `6d4ec722705b6f4ad72be2d628e41bd5cddc765b`
   — `test(app): enforce R35P abort ownership boundary`
6. `6ddf366fe5d0a74eab0a4916947beacb671a7a7c`
   — `docs(app): describe R35P partial retirement dormancy`
7. `c77bbd169d48b470bfffcaa70d4c516d1d417cbc`
   — exact empty deterministic dictionary trigger
8. `c057384665c163d31485270b14b21142804fa27c`
   — automation-generated clean dictionary reconciliation
9. `6fc0c755d4a3d51a8b5f87ad12c553b2e74b7385`
   — explicit R23 worker-outcome retry proof

The exact changed path set is confined to the authorized R35P surface:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- `tests/unit/app_mpeg_run_test.c`;
- `tests/unit/app_mpeg_session_abort_source_test.py`;
- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No `src/app.c`, `src/app_mpeg_product.*`, MPEG worker/runtime implementation,
Transport implementation, Input/UI, RFB/Display/compositor implementation,
media-clock, Configuration/Management, Pi, AUDIO or forensic source changed.

## R35P implementation

R23 now retains the exact successful natural terminal worker outcome before P7
and worker-owner reclamation. That evidence remains local to the run owner and
allows later enclosing-session abort to resume from truthful ownership without
re-querying already-released state.

A new exact partial-retirement admission classifier accepts only concrete
monotonic R23/R24 prefixes. It rejects arbitrary FAULTED state and distinguishes:

- natural-finish worker join/outcome prefixes while P7/worker ownership still
  exists;
- worker-release failure after P7 has been cleared and the exact natural worker
  outcome has been proven;
- runtime-release failure after worker ownership is gone;
- Transport-finalize failure after worker/runtime ownership is gone;
- exact post-finalize nonretryable R23/R24 restore/reveal contradictions after
  all asynchronous execution owners are gone.

Retryable R24 `PSTVNC_APP_MPEG_RUN_REVEAL_PLATFORM_FAILED` and
`PSTVNC_APP_MPEG_RUN_REVEAL_SYNC_INVALID` remain same-session
`REVEAL_PENDING` results with no teardown requirement and are not admitted by
R35P.

Every abnormal attempt still proves exact retained Transport storage first.
After that proof, cleanup resumes only from owners that truthfully remain:

- represented P7 claims use the existing abandon seam before join;
- a naturally finished retirement worker retries join without an abnormal stop;
- an already-joined worker retries outcome/release without duplicate stop/join;
- a worker-release prefix reuses the already-proven natural terminal outcome;
- runtime-only prefixes retry runtime release only;
- Transport-run-only prefixes perform no normal Transport MPEG finalization;
- post-finalize fault prefixes can become locally abort-ready without fabricated
  worker/runtime/RETIRE/producer-done/finalize/reveal work.

The abnormal path still issues no new RETIRE, RETIRE-completion take,
producer-done publication, Transport MPEG finalize, P2 thaw, P3 seal or
compositor reveal. Final old-session Transport reclamation remains owned by the
enclosing `pstvnc_transport_session_close()` after local dormancy is proven.

R33 full-live semantics and R34P pre-START semantics remain distinct and are
preserved.

## Focused deterministic evidence

The existing R33 claim test continues to prove a represented P7 claim is
abandoned before worker stop/join.

New R35P focused tests additionally prove:

- natural R23 join failure retries join without abnormal worker stop;
- R23 worker-outcome failure retries the outcome seam with no duplicate stop or
  join;
- worker-release failure after P7 clear retries only worker release and then
  runtime release, preserving the exact natural completed outcome;
- runtime-only residual ownership retries runtime release only;
- Transport-finalize failure performs no second finalize and can become
  `SESSION_ABORT_READY` while leaving final Transport reclamation to the
  enclosing session;
- retained-Transport proof failure performs no local reclaim and preserves the
  old-run owner evidence;
- a nonretryable post-finalize R24 reveal contradiction becomes abort-ready
  without reveal, rollback or normal retirement work;
- retryable PLATFORM_FAILED and SYNC_INVALID remain non-teardown
  `REVEAL_PENDING` states and are rejected by direct abnormal admission.

The R33/R34P source-boundary proof was extended to require exact R35P admission
results, retained natural outcome evidence, and continued absence of normal
retirement/finalize/reveal effects from abnormal cleanup.

## Canonical machine evidence

Exact final-source GitHub Actions run:

`36261728269`

checked out exact head
`6fc0c755d4a3d51a8b5f87ad12c553b2e74b7385` on
`ledge/h1-all-guns` and completed SUCCESS.

Successful canonical jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED on the final non-trigger commit.

Observed focused/regression evidence includes:

- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- `app_mpeg_run_test: PASS`;
- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_MPEG_CALIBRATION_TEST=PASS`;
- `APP_MPEG_ACTIVATION_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS records=236 grandfathered=9 format_compat=2 stamp_compat=1`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The deterministic dictionary reconciliation changed only
`src/SYMBOLS.md` and
`docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`; final project authority
reports `SOURCE_DICTIONARY_COUNT=src:940` and
`SOURCE_DICTIONARY_TOTAL=6842`.

## Exact linked identity and qualification boundary

Final R35P linked identity:

`ELF_PRISTINE_SHA256=df4b86c726417677575b6a964d2b6646836d21622bd09cac7d36815d12615915`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=39e08e49a5d6aaf32f12d134305a4f9925f7f419d75c4b15e6dde9ad17449fc9`

`PT_LOAD_BYTES=526612`

The newest Foreman-accepted R34 PT_LOAD identity was
`e90c2f8de4729ca2c273a6cc81edbb4806e851cd3000e938f0c65efa8e8851a6`,
525844 bytes.

Therefore:

- `R35P_SOURCE_COMPLETE=YES_WITHIN_PACKET`
- `R35P_HOST_TESTED=PASS`
- `R35P_PROJECT_CHECK=PASS`
- `R35P_STRICT_DICTIONARIES=PASS`
- `R35P_PS2_COMPILE=PASS`
- `R35P_PS2_LINK=PASS`
- `R35P_CURRENT_SOURCE_REPRODUCIBILITY=PASS`
- `R35P_MACHINE_EVIDENCE=GITHUB_ACTIONS`
- `R35P_PS2_PT_LOAD_CHANGED=YES`
- `R35P_INDEPENDENT_VALIDATION=NOT_RUN`
- `R35P_OPERATOR_OBSERVED=NO`
- `R35P_HARDWARE_QUALIFIED=NO`
- `R35P_HARDWARE_PENDING=YES`

No qualification is transferred from R34 or any earlier image.

## Reconstruction Worker requirement disposition

These are Reconstruction Worker dispositions only. They are not Foreman
acceptance.

1. Exact reachable R23/R24 failure-prefix admission — MET.
2. Retained Transport proof remains first — MET.
3. Full-live/post-START R33 meaning preserved — MET.
4. Already-retired P7 is not recreated; represented claim abandonment remains
   owner-seam based — MET.
5. Joined/finished worker prefixes resume truthful owner state without duplicate
   stop/join — MET.
6. Runtime-only residual ownership is supported — MET.
7. Transport-run-only residual ownership is supported without normal finalize —
   MET.
8. Post-finalize nonretryable R24 fault prefixes can become abort-ready — MET.
9. Retryable R24 PLATFORM_FAILED/SYNC_INVALID remain same-session states — MET.
10. Abnormal teardown never resumes normal retirement semantics — MET.
11. R34P pre-START behavior remains distinct and green — MET.
12. Focused plus canonical host/project/dictionary/PS2/reproducibility evidence
    closes the R35P fence — MET.

## Scope and no-claim boundary

R35P does not wire a user action into ordinary retirement, call normal R23/R24
from `src/app.c`, implement auto-recalibration, change successful normal
retirement/reveal semantics, modify Transport two-phase abort, activate AUDIO,
add persistence/editor/reload, change Pi/Wire producer behavior, add timeout or
watchdog success, or claim hardware qualification.

Ordinary action-driven MPEG retirement remains deferred to Foreman review and
packet selection.

## Next pickup

The Interactive Reconstruction Worker stops here.

The Foreman must independently inspect the exact returned range, source owner
semantics, focused tests, canonical run `36261728269`, changed linked identity
and this immutable closeout before accepting or rejecting R35P and deciding any
next packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R35P_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION

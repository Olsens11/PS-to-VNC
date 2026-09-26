# Ledge Foreman work log — R34 blocked review / pre-START MPEG dormancy prerequisite

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T21:02:08-04:00
COMPLETED_AT=2026-09-25T21:08:07-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

This Foreman round received the baton after the Interactive Reconstruction
Worker returned R34 BLOCKED.

Live pickup authority was:

`488c6cc9c0e8c8d36d2bc44c94dc8b3c78cddbe4`
— `docs(work-log): block R34 on pre-start MPEG dormancy`.

The round independently recovered current repository authority, read the current
Foreman state and newest Reconstruction/Foreman immutable logs, re-read the
governing development/reconstruction/lifecycle authorities, inspected the actual
21-commit R34 partial source range, inspected exact machine evidence, and
verified the claimed lower-owner blocker against accepted R21/R33 source rather
than accepting the Worker report at face value.

Relevant governing authority included:

- `AGENTS.md`;
- `CONTRIBUTING.md`;
- `docs/PROJECT_INTENT.md`;
- `docs/CLEAN_ARCHITECTURE.md`;
- `docs/development/source-naming-and-symbols.md`;
- `docs/development/source-topology.md`;
- `docs/development/module-lifecycle.md`;
- Reconstruction Contract revision 0006;
- Work Log Contract revision 0007;
- current Ledge architecture/Wire/runtime authority;
- A003, A004, A005 and A006 audit authority;
- Foreman State revision 0073;
- accepted R21 pre-START start/unwind behavior;
- accepted R33 two-phase enclosing-session abort behavior.

No user-terminal proxy or physical action was required.

## Returned R34 range independently recovered

Assigning Foreman/log authority was:

`39eef5de7061c43e76ad1a19d1d12f944b949106`.

Final pre-log partial R34 source authority was:

`6f1acf56d0007a05c081c0217f099a5fcbc4a754`.

Independent GitHub compare proves:

- 21 commits ahead;
- 0 behind;
- no merge divergence.

Changed paths are confined to the authorized Application composition/test/build/
topology surface:

- `src/app.c`;
- new `src/app_mpeg_product.c/.h`;
- focused R34 Application tests;
- two source-boundary fixtures updated to permit the later authorized public
  composition while retaining lower-owner boundaries;
- build/test enrollment;
- topology/dictionary explanatory metadata.

No Input, Configuration, Management, RFB, Transport, MPEG worker/runtime,
Display, media-clock, AUDIO or Pi product implementation entered the partial R34
range.

## Independent R34 disposition

R34 remains **BLOCKED and not Foreman-accepted**.

Independent criterion disposition:

- A006-R34-C1=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C2=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C3=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C4=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C5=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C6=PARTIAL_BLOCKED
- A006-R34-C7=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C8=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C9=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C10=BLOCKED
- A006-R34-C11=PARTIAL_SOURCE_SUPPORTED_NOT_ACCEPTED
- A006-R34-C12=NOT_MET

The supported partial criteria are not independently promoted into accepted
product contracts. R34 must later be completed and reviewed as one packet.

## Blocker independently confirmed

The Worker's central blocker is correct.

Accepted R21 startup order is:

1. Transport MPEG run-open;
2. PS2 worker-runtime initialization;
3. MPEG worker start;
4. P3 arm;
5. P7 initialization;
6. only then the irreversible START invocation boundary.

R21's pre-START unwind intentionally fails closed. If reverse cleanup cannot be
proven, it preserves exact owner facts, faults the run, and sets
`session_teardown_required=1`.

The existing accepted deterministic
`test_cleanup_failure_faults_and_blocks_retry()` proves a real reachable case:

- worker was genuinely started;
- a later pre-START stage failed;
- P3 abort was attempted;
- worker stop failed;
- worker join did not occur;
- worker runtime was not released;
- Transport pre-start abort did not occur;
- START was never invoked;
- the run remains FAULTED/teardown-required.

R33 cannot safely consume that state. Its current entry validation requires a
complete post-START owner shape, including:

- nonzero generation;
- exact old Transport ticket;
- Transport run open;
- worker runtime owned;
- started worker;
- initialized P7;
- armed P3;
- `start_invoked=1`.

The pre-START retained state can legitimately have P7 already cleared, P3
already aborted, and `start_invoked=0` while still retaining asynchronous
worker/runtime ownership.

Using legacy one-shot Transport abort would violate the accepted R33 rationale:
Transport terminal/reclaim proof is not module dormancy proof.

## Additional independent lower-owner finding

Foreman review found one further reachable R21 cleanup prefix that the Worker
closeout did not fully enumerate and that the prerequisite must close.

`pstvnc_mpeg_worker_start()` behaves as follows when thread start fails:

- the worker thread may already have been created;
- it immediately attempts thread destroy;
- if that destroy succeeds, stack/thread ownership is cleared;
- if that destroy itself fails, the function returns
  `PSTVNC_MPEG_WORKER_THREAD_START_FAILED` while the worker remains
  initialized with created-but-never-started thread/stack ownership.

R21 records `worker_started=0` because worker start returned failure, then its
pre-START unwind sees `worker.initialized` and correctly refuses to pretend the
partial worker is clean.

The current public MPEG worker cleanup path cannot later reclaim that exact
object: ordinary release requires a joined worker, but a never-started thread
cannot truthfully become worker-finished/joined merely for cleanup.

Therefore the prerequisite must include a narrow **worker-owned partial-start
reclamation contract** in addition to generalizing Application run abnormal
session abort for reachable pre-START retained prefixes.

This is an MPEG-worker/Application-run ownership gap, not a reason to change
Transport or to widen R34 Application composition.

## Exact R34 partial machine evidence

At exact partial source authority
`6f1acf56d0007a05c081c0217f099a5fcbc4a754`, GitHub Actions run
`36206592591` produced:

- host-unit: SUCCESS;
- ps2-compile: SUCCESS;
- ps2-link/current-source reproducibility: SUCCESS;
- project-check: FAILURE only at missing local-file topology coverage for the two
  new R34 source files;
- dictionary-long: FAILURE because R34 generated dictionary coverage was not
  reconciled after the blocker was discovered;
- dictionary-reconcile: SKIPPED.

Observed host evidence includes:

- `APP_MPEG_PRODUCT_SOURCE_TEST=PASS`;
- `APP_MPEG_PRODUCT_TEST=PASS`;
- `APP_PRODUCT_BINDINGS_SOURCE_TEST=PASS`;
- `APP_MPEG_SESSION_ABORT_SOURCE_TEST=PASS`;
- P9/P10 tests PASS;
- R28-R33 regressions PASS;
- `app R15/R16B/R19/R27/R32/R34 tests: PASS`;
- `app_mpeg_run_test: PASS`;
- `transport_mpeg_test: PASS`;
- `MPEG_WORKER_TEST=PASS`.

Exact partial linked identity:

`ELF_PRISTINE_SHA256=8cc45464d0e8d71111677a865ddb5bca7f09c58013f1ad88e70f683b86533adc`
`PT_LOAD_SEGMENTS=1`
`PT_LOAD_SHA256=185f1de7fd1bd658f350e3927bae3e7d69fb0733fb6194baeee37e7bd35ff8cd`
`PT_LOAD_BYTES=524820`.

This is reproducible build evidence only. It is not R34 acceptance and is not
hardware qualification.

The newest fully Foreman-accepted linked product authority remains R33:

`PT_LOAD_SHA256=c07ef5062c8d467d2075dfaa3162681924bfd1a344bd2a4d56c45516f3493812`.

## Foreman state / planning work

Foreman State advanced from revision 0073 to revision 0074 in:

`c12919cde5bef2c1a997810faa25181d7648da3f`
— `docs(ledge): block R34 and issue prestart dormancy prerequisite`.

Revision 0074:

- records R34 as BLOCKED_NOT_ACCEPTED;
- retains the 21-commit partial R34 source as unaccepted branch authority;
- independently records the exact pre-START ownership blocker;
- records the additional created-but-never-started partial worker gap;
- keeps the newest fully accepted product authority at R33;
- publishes exactly one lower-owner prerequisite packet;
- forbids the next Worker from resuming or expanding R34 Application
  composition;
- permits branch-wide dictionary/topology reconciliation only so canonical
  evidence can be truthful.

No Foreman-owned product behavior was written.

## Packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a003-mpeg-generation`
`WORKER_KEY=interactive`
`EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION`
`USER_TERMINAL_POLICY=EXCEPTION_ONLY`
`PI_LOCAL_USER_PROXY_REQUIRED=NO`.

Its objective is to make every actually reachable R21 pre-START retained owner
safely closable after Transport begin-abort/retained-old-session proof.

The packet specifically requires:

- exact classification of reachable R21 residual owner shapes;
- a worker-owned retryable cleanup contract for the created-but-never-started
  thread/stack case;
- exact pre-START abnormal session-abort admission in the Application MPEG run
  owner;
- proof-driven stop/status/join/outcome/release for truly started workers;
- no erasure of a live P7 borrow;
- PS2 worker-runtime release only after worker no-touch proof;
- no START/RETIRE/producer-done/finalize/P2-thaw/P3-reveal synthesis;
- terminal abort-ready old-run state without reuse;
- unchanged post-START R33 semantics;
- full branch-wide canonical project/dictionary/PS2 build evidence.

Authorized behavior-bearing source is limited to:

- `src/app_mpeg_run.c/.h`;
- `src/mpeg/worker.c/.h` for the exact partial-start cleanup seam.

R34 `src/app.c` and `src/app_mpeg_product.*` are explicitly off-limits.

## Pending evidence / debt

R34 remains incomplete.

R34P has not yet been executed.

The branch carries partial R34 linked bytes which are reproducible but
unaccepted and hardware-pending.

Project/dictionary gates are currently red solely because blocked R34 stopped
before final topology/dictionary reconciliation; the R34P packet requires those
branch-wide bookkeeping gates to be reconciled and green without treating that
metadata as R34 behavior acceptance.

INDEPENDENT_VALIDATION=NOT_RUN
OPERATOR_OBSERVED=NO
HARDWARE_QUALIFIED=NO

## Next pickup

The permanent Interactive Reconstruction Worker should recover current live
repository authority and execute **only**:

`A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P`.

It must not resume R34 Application composition. It must stop after truthfully
completing, partially completing, or blocking R34P, emit exactly one immutable
Reconstruction work log, and return the baton.

NEXT_PICKUP=A003-MPEG-PRESTART-PARTIAL-SESSION-DORMANCY-R34P

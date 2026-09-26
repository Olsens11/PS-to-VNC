# Ledge Foreman work log — accept R34P and resume R34 activation

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-25T21:31:36-04:00
COMPLETED_AT=2026-09-25T21:35:17-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c5fdd7f1acdaa2dcb526a5113f2e538614f2d976
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R34P pre-START partial MPEG session dormancy work.

Live pickup authority was:

`c5fdd7f1acdaa2dcb526a5113f2e538614f2d976`
— `docs(work-log): record completed R34P pre-start dormancy`.

The round independently recovered current repository authority, current Foreman
State revision 0074, the exact R34P immutable Worker closeout, accepted R21/R33
lower-owner authority, the blocked partial R34 source, current Application MPEG
composition, and exact GitHub Actions evidence.

No user terminal or hardware action was required.

## Returned R34P range

Assigning Foreman/log authority:

`82cd9affd068ec835c1d2c1a930cedc4c1b95bdd`.

Final pre-log R34P source/document authority:

`c6e1dcf514adfc29296633374e75020b3480e5e0`.

Immutable Reconstruction closeout:

`c5fdd7f1acdaa2dcb526a5113f2e538614f2d976`.

Independent compare proves the source range is exactly:

- 13 commits ahead;
- 0 behind.

Changed behavior-bearing source is confined to the packet's authorized owners:

- `src/app_mpeg_run.c/.h`;
- `src/mpeg/worker.c/.h`;
- focused run/worker tests.

The remaining changed files are lifecycle/topology documentation and generated
or reconciled symbol dictionaries.

The blocked R34 product-composition implementation was not behaviorally changed:
no `src/app.c`, `src/app_mpeg_product.*`, Input, Configuration, Management,
Transport, RFB, Display, Pi or AUDIO product source entered the R34P range.

## Independent source findings

The Worker report's core R34P claims were independently verified against final
source.

### Exact never-started MPEG worker reclamation

`pstvnc_mpeg_worker_reclaim_unstarted()` is narrower than normal worker
release and requires the exact created-but-never-started owner shape:

- valid exact generation;
- initialized worker;
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

It retries only injected thread destruction. A failed destroy leaves thread ID,
created state, stack and initialized ownership intact. A successful destroy
precedes stack release and initialized-owner clearance. No join, stop,
worker-finished or outcome fact is fabricated.

### Exact pre-START Application run admission

The R33 post-START validator remains a distinct exact full-owner predicate.

R34P adds a separate pre-START session-abort admission requiring:

- FAULTED run;
- session teardown required;
- nonzero generation;
- exact retained Transport access ticket;
- START not invoked;
- no P7 frame consumer;
- no retirement/producers/restoration facts;
- only reachable R21 retained worker/runtime/Transport combinations;
- worker runtime flag consistent with the runtime owner's own
  `resources_owned` evidence.

A retained truly started worker must still be initialized/thread-started and
retain PS2 runtime plus open Transport MPEG run. A retained unstarted partial
worker must match the exact worker-owner partial-start shape and cannot carry an
armed presentation.

### Teardown ordering

`pstvnc_app_mpeg_run_session_abort_service()` still proves
`pstvnc_transport_session_abort_storage_retained()` before local reclamation.

For a started pre-START worker:

- an already joined worker is recognized through truthful worker status rather
  than stopped/joined twice;
- otherwise stop is requested at most once per abort epoch;
- status must prove real finish and no decoder/slot contradiction;
- join is a real visibility fence;
- exact terminal outcome is retained;
- worker release remains owner-driven;
- PS2 worker-runtime release occurs afterward.

For the never-started partial worker, the run owner delegates only to
`pstvnc_mpeg_worker_reclaim_unstarted()`.

Successful abnormal pre-START cleanup reaches
`PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY` and remains terminal for the old
run/session. The path does not synthesize START, RETIRE, producer-done,
Transport MPEG finalization, P2 thaw, P3 seal/reveal or normal MPEG success.

Existing post-START R33 semantics remain intact.

## Independent R34P criterion disposition

All twelve prerequisite requirements are independently accepted:

- A003-R34P-C1=MET
- A003-R34P-C2=MET
- A003-R34P-C3=MET
- A003-R34P-C4=MET
- A003-R34P-C5=MET
- A003-R34P-C6=MET
- A003-R34P-C7=MET
- A003-R34P-C8=MET
- A003-R34P-C9=MET
- A003-R34P-C10=MET
- A003-R34P-C11=MET
- A003-R34P-C12=MET

R34P_SOURCE_COMPLETE=YES
R34P_FOREMAN_ACCEPTED=YES

## Exact machine evidence

GitHub Actions run:

`36208396947`

was independently fetched and confirms:

- event: push;
- branch: `ledge/h1-all-guns`;
- exact head:
  `c6e1dcf514adfc29296633374e75020b3480e5e0`;
- attempt: 1;
- conclusion: SUCCESS.

Jobs:

- host-unit — SUCCESS;
- project-check — SUCCESS;
- dictionary-long — SUCCESS;
- ps2-compile — SUCCESS;
- ps2-link — SUCCESS;
- dictionary-reconcile — SKIPPED as expected at the final non-trigger commit.

Observed focused/canonical output includes:

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
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`;
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact linked branch identity:

`ELF_PRISTINE_SHA256=a7127b71ede10efa4a91e1914dc55a64bf75d6dffe7169df825d444c77615ccd`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=76a9596819bead5147b0e8cc172c559d3fe59b22fe9d9f2550c4e6e1f6f1c0c3`

`PT_LOAD_BYTES=525716`.

Evidence classification:

- SOURCE_COMPLETE=YES_WITHIN_R34P
- HOST_TESTED=PASS
- PROJECT_CHECK=PASS
- STRICT_DICTIONARIES=PASS
- PS2_COMPILE=PASS
- PS2_LINK=PASS
- CURRENT_SOURCE_REPRODUCIBILITY=PASS
- MACHINE_EVIDENCE=GITHUB_ACTIONS
- INDEPENDENT_VALIDATION=NOT_RUN
- OPERATOR_OBSERVED=NO
- HARDWARE_QUALIFIED=NO

The linked identity contains the already-landed but still-unaccepted partial R34
Application composition. Accepting R34P therefore does not promote this entire
loadable image to fully accepted product authority. R33 remains the newest fully
Foreman-accepted product image until R34 itself closes.

## R34 continuation dependency decision

With R34P accepted, the blocked R34 Application composition now has the
lower-owner contract it lacked.

Independent inspection of the current partial R34 product composition found the
remaining exact gap:

- `pstvnc_app_mpeg_product_has_started_run()` is correctly a full started-run
  predicate and is used for R22 current-tick live service;
- `retire_attempt_owners()` currently reuses that same predicate to decide
  whether two-phase Transport abort/local MPEG dormancy is needed;
- therefore a pre-START P10 teardown-required R21 owner is still incorrectly
  classified as "no MPEG" because START was never invoked;
- that classification would select legacy one-shot Transport abort instead of
  accepted R34P.

The correct Application continuation is not to broaden the live predicate.

Application needs a second semantic owner predicate/seam that means:

"this attempt owns an MPEG lifecycle object that must reach local abnormal
dormancy before retained Transport storage may be reclaimed."

That predicate includes:

- a healthy or failed post-START owner needing R33;
- a pre-START teardown-required owner needing R34P;

and excludes:

- clean IDLE/zero-generation P10 rollback;
- attempts that never acquired MPEG lifecycle ownership.

## Foreman state write

Foreman State advanced from revision 0074 to revision 0075 in:

`e2cb8da1183d195cf7b9dec8c2487b28ee2f5b22`
— `docs(ledge): accept R34P and resume R34 activation`.

Revision 0075:

- accepts R34P at exact source authority;
- keeps R34 itself unaccepted;
- preserves the blocked partial R34 source as continuation input;
- records the current linked identity as reproducible but not fully
  product-accepted;
- publishes exactly one R34 continuation packet;
- keeps normal R23/R24 retirement/recalibration deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34C`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a006-orchestration-shutdown`
`WORKER_KEY=interactive`.

R34C must finish the original R34 packet by adding only the missing Application
teardown-selection distinction.

Its central contract is:

1. keep live R22 service restricted to genuinely started runs;
2. add one Application-level abnormal MPEG teardown-owner predicate/seam;
3. send pre-START P10 teardown-required owners through Transport begin-abort ->
   accepted R34P -> final close;
4. keep post-START owners on begin-abort -> R33 -> final close;
5. keep clean/no-MPEG attempts on accepted R16B one-shot abort;
6. preserve Input dormancy before dependent MPEG teardown;
7. do not enter normal same-session retirement/recalibration.

Authorized behavior-bearing source is limited to:

- `src/app.c/.h`;
- `src/app_mpeg_product.c/.h`;
- focused Application composition tests.

Accepted R34P/R33/P9/P10/Input/Transport/lower-owner implementation is read-only
unless a genuine new prerequisite defect is independently demonstrated.

## Pending evidence / debt

R34 product activation remains unaccepted until R34C returns and all original
A006-R34-C1 through C12 are independently re-reviewed as one complete packet.

Normal R23/R24 user-facing retirement/reveal/recalibration remains deferred.

Independent Validation, operator observation and hardware qualification remain
not run/not claimed.

## Next pickup

The permanent Interactive Reconstruction Worker must recover current live
repository authority and execute only:

`A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34C`.

It must not enter normal retirement/recalibration. At completion or block it must
emit exactly one immutable Reconstruction work log and return the baton.

NEXT_PICKUP=A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34C

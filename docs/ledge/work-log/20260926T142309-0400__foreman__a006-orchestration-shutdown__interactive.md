# Ledge Foreman work log — accept R35P and issue normal MPEG retirement

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T14:23:09-04:00
COMPLETED_AT=2026-09-26T14:29:53-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f6b20fa7aaf3dddb1e4d858ac24021af6aa2272f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority recovered

This Foreman round received the baton after the Interactive Reconstruction
Worker returned completed R35P partial-retirement session-dormancy work.

Live pickup authority was:

`f6b20fa7aaf3dddb1e4d858ac24021af6aa2272f`
— `docs(work-log): record R35P partial retirement dormancy`.

The round independently recovered current Foreman State revision 0076, the
exact R35P Reconstruction closeout, accepted R23/R24/R33/R34P/R34 authority,
the nine-commit returned range, focused run/session-abort source, canonical
machine evidence, and the current ordinary Application MPEG product path.

No user terminal, Pi-local proxy, hardware action, Validation work, or Foreman
product-behavior patch was required.

## Returned R35P range

Assigning Foreman/log authority:

`ec6280390c1ec4afc5cf9096b2bc7cd0fb51c8f8`.

Final pre-log R35P source/document authority:

`6fc0c755d4a3d51a8b5f87ad12c553b2e74b7385`.

Immutable Reconstruction closeout:

`f6b20fa7aaf3dddb1e4d858ac24021af6aa2272f`.

Independent compare proves the source range is exactly:

- 9 commits ahead;
- 0 behind.

Changed behavior-bearing source is confined to:

- `src/app_mpeg_run.c`;
- `src/app_mpeg_run.h`;
- focused `app_mpeg_run` and session-abort tests.

The remaining changed files are exact generated/reconciled dictionaries.

No ordinary `src/app.c`, `src/app_mpeg_product.*`, MPEG worker/runtime,
Transport, Input/UI, RFB/Display/compositor, media-clock,
Configuration/Management, Pi, AUDIO or forensic implementation changed.

## Independent R35P findings

R23 now records the exact successful natural worker outcome before clearing P7
or attempting worker release. That evidence remains owned by the run coordinator
until successful attempt clear.

R35P adds an exact partial-retirement abnormal-admission classifier rather than
a generic FAULTED escape hatch.

It accepts only concrete monotonic R23/R24 prefixes after natural retirement has
already established:

- START invoked;
- exact generation retained;
- RETIRE invoked;
- RETIRE completion taken;
- producer-done published;
- worker joined;
- P7 retired;
- exact natural completed worker outcome retained.

From that common prefix it accepts only truthful remaining owner shapes:

- worker-release failure with joined worker still represented;
- runtime-release failure after worker ownership is gone;
- Transport-finalize failure after worker/runtime ownership is gone;
- exact post-finalize nonretryable R23/R24 restoration/reveal faults.

Earlier R23 failures that still retain the full live owner shape remain handled
through the existing R33 post-START admission. In particular natural worker
join failure is re-observed as natural completion and retries join without an
abnormal stop.

The abnormal service still proves
`pstvnc_transport_session_abort_storage_retained()` first.

A represented P7 claim is abandoned through its owner seam before join. A
naturally finished worker is not stopped merely to fit R33's original shape.
An already-joined worker is not joined twice. The retained successful natural
outcome may be copied into session-abort evidence before worker release.
Runtime-only cleanup retries only runtime release.

A Transport-run-only prefix does not call normal
`pstvnc_transport_mpeg_run_finalize()` again. Post-finalize fault prefixes may
become `SESSION_ABORT_READY` with no fabricated worker/runtime ownership.

The abnormal path contains no normal RETIRE, RETIRE-completion take,
producer-done, Transport MPEG finalize, P2 thaw, P3 seal, compositor reveal or
ordinary frame-service call.

R24 `PSTVNC_APP_MPEG_RUN_REVEAL_PLATFORM_FAILED` and
`PSTVNC_APP_MPEG_RUN_REVEAL_SYNC_INVALID` remain retryable
`REVEAL_PENDING` same-session outcomes with no teardown debt and are excluded
from partial-retirement abnormal admission.

Existing full-live R33 and pre-START R34P paths remain distinct and green.

## Independent criterion disposition

All twelve R35P requirements are independently accepted:

- A003-R35P-C1=MET
- A003-R35P-C2=MET
- A003-R35P-C3=MET
- A003-R35P-C4=MET
- A003-R35P-C5=MET
- A003-R35P-C6=MET
- A003-R35P-C7=MET
- A003-R35P-C8=MET
- A003-R35P-C9=MET
- A003-R35P-C10=MET
- A003-R35P-C11=MET
- A003-R35P-C12=MET

R35P_SOURCE_COMPLETE=YES
R35P_FOREMAN_ACCEPTED=YES

## Exact machine evidence

Final-source GitHub Actions run:

`36261728269`

independently confirms:

- event: push;
- branch: `ledge/h1-all-guns`;
- exact head:
  `6fc0c755d4a3d51a8b5f87ad12c553b2e74b7385`;
- attempt: 1;
- conclusion: SUCCESS.

Successful jobs:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link;
- dictionary-reconcile correctly SKIPPED.

Observed focused/canonical output includes:

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
- `SOURCE_DICTIONARIES=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact immutable-log-head run:

`36261850645`

at `f6b20fa7aaf3dddb1e4d858ac24021af6aa2272f` also completed SUCCESS with
host-unit, project-check, dictionary-long, ps2-compile and ps2-link all
successful.

Exact accepted linked identity:

`ELF_PRISTINE_SHA256=df4b86c726417677575b6a964d2b6646836d21622bd09cac7d36815d12615915`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=39e08e49a5d6aaf32f12d134305a4f9925f7f419d75c4b15e6dde9ad17449fc9`

`PT_LOAD_BYTES=526612`.

Evidence classification:

- SOURCE_COMPLETE=YES_WITHIN_R35P
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
- HARDWARE_PENDING=YES

This becomes the newest fully Foreman-accepted source/build identity. Physical
qualification does not transfer from earlier images.

## Next dependency review

With R35P accepted, the lower-owner failure fence required for normal
same-session retirement is closed.

Independent inspection of the ordinary Application/product path finds no
additional lower-owner prerequisite before composing accepted R23/R23C/R24.

The existing P9 foreground already resumes mouse interpretation before entering
`ACCEPTED_PROTECTED`, so R35 does not need a new Input-resume mechanism after
final reveal.

The configured semantic binding context remains authoritative. R29 receives one
Application-supplied DESKTOP eligibility bit:

- an explicitly configured DESKTOP binding remains ineligible while MPEG owns
  presentation;
- an explicitly configured GLOBAL binding may still emit the semantic
  `PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION` while MPEG is live;
- Application must consume only that already-published semantic fact and may not
  reinterpret a DESKTOP binding as GLOBAL.

R23/R23C/R24 already provide the exact lower seams needed for the next packet:

- begin retirement from exact MPEG_OWNED;
- nonblocking retirement service with current session-clock tick;
- P2 thaw/FULL-refresh debt through the existing RFB flow policy;
- explicit record of a successfully presented graphics-fresh restored desktop;
- retryable final reveal and exact return to RFB_ONLY/IDLE.

One Application integration detail is mandatory: when P2 protocol freshness is
finally reached for RESTORE_PENDING, the ordinary desktop presentation/upload
boundary must occur before the R24 restoration marker even if the refreshed
framebuffer pixels compare unchanged. Protocol completion alone is not physical
presentation proof.

## Foreman state write

Foreman State advanced from revision 0076 to revision 0077 in:

`228b3c2009f803601a4ed42738e753f9d6f3be3e`
— `docs(ledge): accept R35P and issue normal MPEG retirement`.

Revision 0077:

- accepts R35P at exact source/log authority;
- records the new accepted linked identity;
- publishes exactly one ordinary R35 retirement/restoration/reveal packet;
- preserves configured DESKTOP/GLOBAL binding-context semantics;
- keeps automatic recalibration, AUDIO and persistence/editor/reload deferred.

No Foreman-owned product behavior was written.

## Active packet issued

Exactly one Reconstruction packet is active:

`PACKET_ID=A006-ORDINARY-MPEG-ACTION-RETIRE-RESTORE-REVEAL-R35`
`PACKET_STATUS=ACTIVE`
`PACKET_OWNER=RECONSTRUCTION`
`WORK_ITEM_KEY=a006-orchestration-shutdown`
`WORKER_KEY=interactive`.

R35 must compose accepted R23/R23C/R24 into the ordinary product path.

Its central contract is:

1. exact idle semantic action retains R34 P9-begin behavior;
2. an already-published semantic action while exact MPEG_OWNED may begin R23;
3. WAIT_FIRST_FRAME/RETIRING/RESTORE_PENDING/REVEAL_PENDING cannot duplicate
   RETIRE or start calibration;
4. RETIRING is serviced through R23 with the exact current tick, separate from
   R22 live service;
5. P2/R19 remains the sole restoration-request path;
6. successful graphics presentation must precede the R24 restoration marker;
7. retryable R24 reveal outcomes remain same-session pending states;
8. nonretryable R23/R24 faults converge through accepted R35P;
9. success returns to ordinary desktop/IDLE and stops there;
10. no automatic recalibration or new action/context semantics enter R35.

## Pending dependency order

Current order is:

1. R35 — ordinary same-session retirement/restoration/reveal;
2. after R35 acceptance, decide the product policy for automatic one-action
   recalibration versus requiring a fresh semantic action from restored desktop;
3. remaining A006/A007 closure work, including AUDIO/product completeness, only
   in dependency order.

## Next pickup

The permanent Interactive Reconstruction Worker must recover current live
repository authority and execute only:

`A006-ORDINARY-MPEG-ACTION-RETIRE-RESTORE-REVEAL-R35`.

It must not enter automatic recalibration, AUDIO, persistence/editor/reload, or
new binding semantics. At completion or block it must emit exactly one immutable
Reconstruction work log and return the baton.

NEXT_PICKUP=A006-ORDINARY-MPEG-ACTION-RETIRE-RESTORE-REVEAL-R35

# Foreman shift — accept A003 R4, repair immutable log policy, activate A004 P7

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T02:13:35-04:00
COMPLETED_AT=2026-09-21T02:21:51-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=4d2139d0b2443e65e5e3884b3ac5558bb1a0ddad
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4` baton,
independently verify its source/evidence, repair the already-immutable malformed
Reconstruction log through explicit policy rather than destructive history
editing, and publish the next bounded A004 frame-consumer packet.

## Live authority at pickup

Branch HEAD:

`4d2139d0b2443e65e5e3884b3ac5558bb1a0ddad`

Foreman State:

`STATE_REVISION=0031`

Worker pre-log source authority:

`e56d9cb47439ef60c82e3dd78328d7ea6732995e`

Worker immutable log:

`docs/ledge/work-log/20260921T020723-0400__reconstruction__a003-mpeg-generation__interactive.md`

The Reconstruction range after previous Foreman head
`3bbbd8ecbfa54a1cd155c1195e990ac4101a33b2` was:

1. `51f45cdb59cd61565246159e1915f002b0c7de2d`
   — MPEG asynchronous worker / one-slot borrowed-frame rendezvous;
2. `785ec520812a9c7e07862717a4fab87febc6c623`
   — deterministic dictionary-reconciliation trigger;
3. `d63e8af665a689acf38e3ce58f6cce62f3bf61a1`
   — generated current-clean MPEG dictionary reconciliation;
4. `e56d9cb47439ef60c82e3dd78328d7ea6732995e`
   — maintained MPEG ownership prose;
5. `4d2139d0b2443e65e5e3884b3ac5558bb1a0ddad`
   — immutable Reconstruction log.

## Independent R4 source acceptance

Foreman independently confirmed:

- worker start copies exactly one nonzero caller-owned run generation and never
  mints/increments identity;
- decoder initialize, step and release occur only on the worker thread;
- the public owner stop path uses the already-accepted synchronized decoder stop
  request and does not change stream/EOF semantics;
- one synchronized frame slot owns exactly
  `EMPTY -> AVAILABLE -> CLAIMED -> EMPTY`;
- worker publication copies only the R2 borrowed metadata, not a full frame;
- no decoder step can occur while the slot remains AVAILABLE or CLAIMED;
- nonblocking claim validates exact run and returns the borrowed frame plus the
  ordinal-derived claim token;
- release validates exact run and exact claim token, clears once and signals the
  retained event;
- stale/wrong generation, duplicate release, wrong token and unavailable claim
  fail closed;
- the event contract explicitly requires retained signal-before-wait behavior,
  and the worker re-checks synchronized predicates after every wake rather than
  relying on a polling timer;
- stop may discard AVAILABLE, but CLAIMED remains pinned and cannot be
  overwritten or freed until exact release;
- stop before decoder call prevents the call;
- stop during decoder call preserves R2's real feed/Transport semantics,
  accounts a completed picture once and suppresses publication from the stopping
  boundary;
- COMPLETE, STOPPED and failure terminal outcomes remain distinct and preserve
  decoder result/report/release truth;
- a later decoder failure is not hidden by a previously delivered frame;
- join refuses an outstanding slot and an unfinished worker;
- join is the worker-write visibility fence;
- release requires join before thread destruction and worker-stack reclamation;
- no Display, compositor, Presentation, Platform graphics, RFB, Application,
  media-clock or Transport-private dependency entered the worker.

A003-R4-C1 through A003-R4-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

## R4 machine evidence

Final coherent source authority:

`e56d9cb47439ef60c82e3dd78328d7ea6732995e`

Workflow:

`35566964942` — run #318 — SUCCESS first attempt.

Observed evidence includes:

- host-unit PASS;
- `MPEG_WORKER_TEST=PASS`;
- `mpeg_decoder_test: PASS`;
- existing scheduler/compositor/RFB/Transport host tests PASS;
- project-check PASS;
- complete strict dictionaries PASS;
- direct R5900 compile including `src/mpeg/worker.c` PASS;
- SMS dedicated compile preservation PASS;
- linked clean build PASS;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

The later worker-log head run:

`35567149530` — run #319

had all host, dictionary, PS2 compile and PS2 link jobs green. Its sole
project-check failure was:

`filename stamp 20260921T020723-0400 != STARTED_AT-derived 20260921T015200-0400`.

No product-source defect was implicated.

## Immutable log compatibility repair

The already-committed R4 log truthfully contains:

`STARTED_AT=2026-09-21T01:52:00-04:00`

and:

`COMPLETED_AT=2026-09-21T02:07:23-04:00`.

Its filename accidentally used the completion stamp:

`20260921T020723-0400__reconstruction__a003-mpeg-generation__interactive.md`.

The governing contract explicitly prohibits rename, rewrite, replacement,
deletion or corrected duplicate creation after commit.

This Foreman shift therefore published **work-log contract revision 0007** as a
narrow compatibility policy:

- the malformed historical path remains untouched;
- it is not added to the broad nine-record grandfather bucket;
- its exact frozen filename stamp is pinned;
- its exact truthful STARTED_AT is pinned;
- only ordinary filename-stamp == STARTED_AT-derived-stamp equality is waived;
- every other canonical filename grammar, required metadata, document,
  LOG_FORMAT_REVISION, status, SELF_PAUSED, COMPLETED_AT and
  role/work-item/worker-key check remains active;
- no pattern/date/role-wide timestamp exception exists.

Updated:

- `docs/ledge/work-log/README.md` -> DOCUMENT_REVISION=0007;
- `scripts/work-log-check.py` -> one exact
  `FILENAME_STAMP_EXCEPTIONS` record.

The checker now reports the compatibility separately as
`stamp_compat=1`.

## Foreman State / next packet

Published State revision 0032 in:

`9ba5d2589b1c5d2043ab0b6d02a100b64cdfdf00`

message:

`docs(foreman): accept A003 R4 and repair immutable log policy`

State now consumes:

- Reconstruction Contract rev 0006;
- Work-log Contract rev 0007.

Current phase:

`A003_R4_INTEGRATED__A004_P7_MAIN_THREAD_FRAME_CONSUMER_RECONSTRUCTION_ACTIVE__PS2_WORKER_BINDING_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

Active packet:

`A004-MAIN-THREAD-FRAME-CONSUMER-P7`

with:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a004-presentation`;
- WORKER_KEY=`interactive`;
- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`;
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`;
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`.

P7 is bounded to the Application/main-thread one-frame bridge:

- claim one exact R4 borrowed frame;
- validate/map RGB16 metadata to the neutral A004 surface;
- ordinal 1 -> sole P4 first-sync compositor;
- initialize P6 only after truthful synchronized first-frame effects;
- later ordinals -> P6 WAIT/PRESENT/DROP;
- WAIT retains the exact R4 claim and returns to the main loop without sleeping;
- PRESENT uses the sole compositor then releases exact claim;
- DROP performs presentation-only discard then releases exact claim;
- RETIRING remains drain-compatible;
- REVEAL_PENDING/RFB_ONLY fail closed;
- validation/scheduler/compositor failure cannot silently release borrowed
  storage before exact worker stop containment is established.

P7 explicitly does not own:

- calibration persistence;
- worker creation/start;
- concrete PS2 worker thread/event binding;
- START/producer activation;
- P5 begin-retirement/seal/reveal;
- RETIRE/ACK;
- Transport residual/credit finalization;
- RFB restoration;
- Wire-loss restoration;
- final all-in Application MPEG lifecycle.

## Policy/state-head CI

State/policy commit:

`9ba5d2589b1c5d2043ab0b6d02a100b64cdfdf00`

Workflow:

`35568071686` — run #320 — SUCCESS.

Observed:

- host-unit PASS;
- project-check PASS;
- dictionary-long PASS;
- ps2-compile PASS;
- ps2-link/current-source reproducibility PASS;
- dictionary-reconcile SKIPPED as expected.

Project-check explicitly reports:

`WORK_LOG_CHECK=PASS records=149 grandfathered=9 format_compat=2 stamp_compat=1`

and:

`PS_TO_VNC_PROJECT_CHECK=PASS`.

This proves the R4 historical record is accepted only through the exact
revision-0007 compatibility path while the full repository check is restored.

## Evidence boundaries

PENDING_LOCAL=NONE_FOR_FOREMAN_POLICY_STATE_PUBLICATION
R4_PHYSICAL_PS2_WORKER_DECODE=NOT_RUN_NOT_CLAIMED
R4_CONCRETE_PS2_THREAD_EVENT_BINDING=NOT_IMPLEMENTED
A004_P7_MAIN_THREAD_FRAME_CONSUMER=ACTIVE_RECONSTRUCTION_PACKET
START_PRODUCER_ACTIVATION=NOT_IMPLEMENTED
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_RESTORATION=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes:

`A004-MAIN-THREAD-FRAME-CONSUMER-P7`

and emits exactly one immutable log using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a004-presentation`;
- WORKER_KEY=`interactive`.

Do not begin worker binding, producer START/RETIRE, Q7 retirement or final
Application orchestration in the same shift.

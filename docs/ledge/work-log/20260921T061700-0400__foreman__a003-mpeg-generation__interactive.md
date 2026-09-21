# Foreman shift — accept A003 R5 and activate MPEG generation-control relay

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T06:17:00-04:00
COMPLETED_AT=2026-09-21T06:29:13-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=8b198603ace2f7a34bb4dce5753da385aa08b348
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A003-PS2-WORKER-EXECUTION-BINDING-R5` Reconstruction
baton, independently verify the concrete PS2 execution mechanics and evidence,
then identify and publish the smallest remaining owner-correct prerequisite
before live Application MPEG activation.

## Live authority at pickup

Branch HEAD:

`8b198603ace2f7a34bb4dce5753da385aa08b348`

Foreman State:

`STATE_REVISION=0033`

Final pre-log R5 source/dictionary/build authority:

`99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`

Worker immutable log:

`docs/ledge/work-log/20260921T054912-0400__reconstruction__a003-mpeg-generation__interactive.md`

## R5 worker range independently reviewed

After previous Foreman head
`f3f69cb9d659b1d8d31e2d0d40b6d9b9a22f1664`:

1. `d361baef50fc0eece2737e5147256d863d162aff`
   — concrete MPEG-owned PS2 worker execution binding;
2. `f03e51ea05137060fa2c73350a18ce13efc2be30`
   — deterministic dictionary-reconciliation trigger;
3. `6cb92ae8d8a48da96b24587e67c593936e1fd27c`
   — generated current-clean MPEG dictionary reconciliation;
4. `99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`
   — maintained MPEG responsibility prose;
5. `8b198603ace2f7a34bb4dce5753da385aa08b348`
   — required immutable Reconstruction work log.

## Independent R5 acceptance

Foreman independently confirmed:

- `src/mpeg/ps2_worker_runtime.{c,h}` owns one explicit PS2 mechanism binding
  for one MPEG worker interval;
- no process-global mutable binding singleton or generic Platform-thread
  framework was introduced;
- init requires explicit nonzero caller join-poll delay/count and chooses no
  worker stack, priority or timing defaults;
- decoder and worker allocations use exact requested byte count/alignment with
  `memalign()`, verify the returned alignment and pair with `free()`;
- decoder and worker/slot synchronization use distinct count-1 PS2 semaphores;
- the frame event is a separate count-0/max-1 semaphore, so signal-before-wait
  retains one wake and redundant signals may safely coalesce;
- event wait uses blocking `WaitSema()` and no timer-backed poll loop;
- EE thread create receives the exact R4 stack pointer/size/priority and sets
  `gp_reg = &_gp`;
- a private trampoline invokes only the saved portable R4 entry/argument and
  then calls `ExitThread()`;
- only one thread slot may be active per binding and stale IDs fail closed;
- join uses caller-bounded `ReferThreadStatus()` observations and succeeds only
  after `THS_DORMANT` is proven;
- R4's portable `worker_finished` fact is not treated as kernel thread-exit
  proof;
- historical H1 1-ms/3000-loop join values were not restored as defaults;
- no `TerminateThread()` or force-delete path exists;
- `DeleteThread()` occurs only after a prior dormancy proof or a direct final
  `ReferThreadStatus()` proof;
- StartThread-failure cleanup also refuses deletion unless dormancy is proven;
- R4's own join/release ordering ensures EMPTY slot and finished portable worker
  before EE join, then dormant thread destruction before stack free;
- binding release refuses while any thread slot or binding-issued allocation is
  still live;
- partial semaphore init/delete failures preserve explicit resource ownership
  and IDs for safe retry rather than pretending cleanup succeeded;
- no decoder-step/libmpeg/IPU, Display/P7/scheduler/compositor, media-clock,
  producer, Transport, RFB, GS or generation-allocation behavior entered R5.

A003-R5-C1 through A003-R5-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

## R5 machine evidence

Final coherent pre-log source authority:

`99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`

Workflow:

`35586198869` — run #331 — final attempt SUCCESS.

The first attempt's only red canonical job was the repository's known
timing-sensitive Transport runtime host fixture. R5 changed no Transport source;
the unchanged retry passed.

Observed final evidence includes:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `PS2_COMPILE=src/mpeg/ps2_worker_runtime.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Worker-log head workflow:

`35586567057` — run #332 — final attempt SUCCESS.

Its first host attempt reproduced the familiar Transport-runtime timing cascade
beginning at assertions 787/789 and dependent quiesce assertions. Every
non-host job passed; the identical-commit host rerun passed.

No physical PS2 worker-thread/semaphore/event/decode/presentation run occurred or
is inferred.

## Why Application activation is still dependency-queued

Current clean source has the local execution pieces:

- R2 one-picture decoder;
- R3 SMS/IPU backend;
- R4 asynchronous one-slot worker;
- R5 concrete PS2 execution binding;
- P3-P6 Display/presentation/scheduler mechanisms;
- P7 main-thread frame consumer.

But current Transport authority still lacks the owner-correct generation-control
relay required to start and retire a remote MPEG producer:

- protocol code already preserves START kind 11 / control channel 0 / flags 0 /
  exact 44-byte v1 and RETIRE kind 10 / control channel 0 / flags 0 / exact
  12-byte v1;
- current Transport's single physical-I/O runtime already owns generic serialized
  outbound work;
- public Transport bridge exposes no session-bound START/RETIRE send;
- receiver dispatch currently accepts only DATA, so the Pi's valid kind-10
  RETIRE completion would presently be treated as a fatal unknown frame;
- the old direct physical-stream START helper predates the governing single-I/O
  architecture and cannot be the Application send path.

Frozen exact-retirement evidence records one directional protocol fact still
compatible with Q1-Q12: PS2 sends exact RETIRE; Pi echoes the identical exact
RETIRE payload only after its remote cleanup; higher owner state distinguishes
request from completion.

Transport owns moving that exact control frame under current Wire Session
authority. Application later owns whether the returned session/generation value
matches its pending MPEG run and when the ordered lifecycle may advance.

## State / next packet publication

Published Foreman State revision 0034 at:

`3c99fb688faff4e37c15e0356bd3ad4d94f0ec97`

message:

`docs(foreman): accept A003 R5 and activate MPEG control relay`

Current phase:

`A003_R5_INTEGRATED__A003_MPEG_GENERATION_CONTROL_RELAY_RECONSTRUCTION_ACTIVE__PI_WIRE_CONTROL_OWNER_DEPENDENCY_QUEUED__APPLICATION_ACTIVATION_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

Active packet:

`A003-MPEG-GENERATION-CONTROL-RELAY-R6`

Key R6 requirements:

- session-bound public START/RETIRE/take-completion Transport seams;
- outbound START/RETIRE through existing
  `pstvnc_transport_runtime_submit_frame()` and sole physical-I/O owner;
- exact accepted START and RETIRE envelopes/codecs;
- inbound PS2 kind-10 RETIRE completion accepted only in exact envelope form;
- inbound START remains invalid direction;
- one bounded synchronized pending completion slot with one-shot nonblocking
  take;
- stale Session-A access cannot send or consume Session-B completion;
- Transport stores/relays payload identity but does not decide active-generation
  business semantics;
- receiving RETIRE completion does not synthesize producer_done/EOF;
- no residual discard/credit finalization;
- 44-byte DATA/channel4 remains ordinary opaque MPEG media;
- old direct physical-stream START helper cannot remain a product bypass around
  sole-I/O Transport;
- no Pi product implementation or Application lifecycle is included.

## State-head CI

State commit:

`3c99fb688faff4e37c15e0356bd3ad4d94f0ec97`

Workflow:

`35588925688` — run #333 — SUCCESS first attempt.

Observed canonical jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

## Evidence boundaries

PENDING_LOCAL=NONE_FOR_FOREMAN_STATE_PUBLICATION
A003_R5_PHYSICAL_PS2_EXECUTION=NOT_RUN_NOT_CLAIMED
A003_R6_MPEG_GENERATION_CONTROL_RELAY=ACTIVE_RECONSTRUCTION_PACKET
PI_PRODUCT_WIRE_CONTROL_OWNER=NOT_IMPLEMENTED
PI_PRODUCT_MPEG_PRODUCER_OWNER=NOT_IMPLEMENTED
MPEG_ACTIVATION_REQUIRED_VALUE_AUTHORITY=INCOMPLETE
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
P7_WIRED_INTO_LIVE_APP_LOOP=NO
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes:

`A003-MPEG-GENERATION-CONTROL-RELAY-R6`

and emits exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a003-mpeg-generation`;
- WORKER_KEY=`interactive`.

Do not begin Pi product Wire-server/control ownership, Application activation,
producer lifecycle, residual finalization, RFB restoration or P5/Q7 retirement
orchestration in the same shift.

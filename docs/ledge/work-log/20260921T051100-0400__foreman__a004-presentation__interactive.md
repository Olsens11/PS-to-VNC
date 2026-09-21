# Foreman shift — accept A004 P7 and activate A003 PS2 worker execution binding

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T05:11:00-04:00
COMPLETED_AT=2026-09-21T05:18:09-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a004-presentation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=0d718851860ad15ab525ea0485c27165859f7be3
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A004-MAIN-THREAD-FRAME-CONSUMER-P7` Reconstruction
baton, independently verify the Application-owned frame bridge and exact
machine evidence, then publish the next bounded dependency required to make the
accepted A003 worker executable on real PS2 EE mechanics.

## Live authority at pickup

Branch HEAD:

`0d718851860ad15ab525ea0485c27165859f7be3`

Foreman State:

`STATE_REVISION=0032`

Final pre-log P7 source/test/dictionary/build authority:

`d0c9bcb99725bfdaf6b07c5396a0fa1255039225`

Worker immutable log:

`docs/ledge/work-log/20260921T022804-0400__reconstruction__a004-presentation__interactive.md`

## P7 worker range independently reviewed

After previous Foreman head
`a04c775cf0bff2d9b89a01f1764c769ef59fe1a9`:

1. `c9141fe436ab099383e3f5db84eacacfd3802720`
   — Application MPEG frame consumer, focused tests and build wiring;
2. `8d0110fab005010173b0f0c547128f435992bc35`
   — deterministic dictionary-reconciliation trigger;
3. `6f2844d92789500f6b9893f245c9228343bf5267`
   — generated current-clean root dictionary reconciliation;
4. `5fb13033e83e2d7626af6f9596b59018e439ec5c`
   — maintained Application ownership/topology prose;
5. `d0c9bcb99725bfdaf6b07c5396a0fa1255039225`
   — scheduler-decision containment test closure;
6. `0d718851860ad15ab525ea0485c27165859f7be3`
   — required immutable Reconstruction work log.

## Independent P7 acceptance

Foreman independently confirmed:

- `src/app_mpeg_frame.{c,h}` is Application/main-thread coordination rather
  than MPEG or Display private mechanism;
- init requires one nonzero caller-owned run generation, exact
  WAIT_FIRST_FRAME Presentation generation, one R4 worker accepting that
  generation, one existing session media clock and one caller scheduler
  profile;
- generation is copied verbatim and never minted/incremented/rebased;
- a no-held-frame service call claims at most one exact R4 frame;
- FRAME_UNAVAILABLE is ordinary nonblocking IDLE;
- worker terminal state is not reinterpreted by P7;
- claimed R2/R4 storage is mapped zero-copy into the neutral RGB16 macroblock
  compositor value only after pointer/alignment/bpp/usable-byte/capacity/
  geometry validation;
- claim token equals exact picture ordinal;
- first ordinal must be 1 and every later consumed ordinal must be exactly +1;
- ordinal 1 bypasses P6 and reaches only P4 while Presentation is
  WAIT_FIRST_FRAME;
- P7 does not directly arm the media clock or promote Presentation;
- successful first-frame service requires truthful synchronized and
  first-frame-promoted P4 effects;
- P6 initializes exactly once only after that real first sync using P4's
  observed synchronized tick and the existing session clock;
- later frames preserve P6 WAIT/PRESENT/DROP exactly;
- WAIT pins the same R4 claim, returns the absolute deadline, and performs no
  release, second claim, spin, sleep or VBlank wait;
- PRESENT invokes the sole P4 compositor and releases the exact claim only after
  successful synchronized presentation;
- DROP invokes no compositor and only releases presentation ownership of that
  frame;
- RETIRING remains drain-compatible for accepted later frames;
- P7 does not begin, seal or reveal retirement;
- REVEAL_PENDING/RFB_ONLY prevent new claims;
- a sealed-state transition with an already-held claim enters borrow-safe
  failure containment;
- mapping, ordinal, scheduler-init/decision, compositor and sealed-state
  failures request exact R4 worker stop before intentionally releasing the held
  borrow;
- if stop cannot be established, the claim remains pinned and observable;
- release failure faults the coordinator and preserves the outstanding-claim
  fact;
- compositor physical effects remain truthful even when later clock/promotion
  work fails;
- P7 never joins/destroys the MPEG worker and never interprets decoder outcome;
- no Transport-private, RFB, direct GS/dmaKit, producer control, P5 transition,
  media-clock arm, delay/wait helper or generation allocator entered the
  module;
- root Application placement is explicitly documented in source-topology
  authority.

A004-P7-C1 through A004-P7-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

## P7 machine evidence

Exact final source authority:

`d0c9bcb99725bfdaf6b07c5396a0fa1255039225`

Workflow:

`35569493813` — run #325 — SUCCESS first attempt.

Observed:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`;
- project-check PASS;
- complete strict dictionaries PASS;
- `PS2_COMPILE=src/app_mpeg_frame.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Exact worker-log head workflow:

`35569778609` — run #326 — SUCCESS first attempt across all canonical jobs.

No physical PS2 MPEG presentation/timing/concurrency evidence is inferred.

## Next dependency selected

Current clean source now has:

- the R2 synchronous one-picture decoder seam;
- the R3 concrete SMS/IPU decoder backend;
- the R4 asynchronous one-slot worker/rendezvous;
- the P7 main-thread frame consumer.

But no clean source currently supplies R2/R4's injected PS2 execution mechanics.
There is no concrete MPEG worker thread/event/memory binding in the current
product tree.

Current clean input runtime and frozen H1 evidence both establish the required
EE primitives:

- `CreateThread`;
- `StartThread`;
- `ReferThreadStatus`;
- `DeleteThread`;
- `DelayThread`;
- `CreateSema`;
- `WaitSema`;
- `SignalSema`;
- `ExitThread`;
- `_gp`.

The portable R4 entry returns normally, unlike current clean input and frozen H1
workers which explicitly terminate with `ExitThread()`. Therefore a concrete
PS2 worker adapter requires a trampoline that invokes the saved portable entry
and then exits the EE thread explicitly.

H1's 64-KiB stack, priority 67 and 1-ms/3000-loop join values were experimental
choices and are not promoted to product defaults.

## State / packet publication

Published Foreman State revision 0033 at:

`631dad7847ff0a775a61afcb2b8fdfced88da918`

message:

`docs(foreman): accept A004 P7 and activate PS2 worker binding`

Current phase:

`A004_P7_INTEGRATED__A003_PS2_WORKER_EXECUTION_BINDING_RECONSTRUCTION_ACTIVE__APPLICATION_ACTIVATION_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

Active packet:

`A003-PS2-WORKER-EXECUTION-BINDING-R5`

The packet requires:

- explicit MPEG-owned PS2 binding lifetime;
- aligned decoder and worker memory operations;
- distinct decoder and worker synchronization resources;
- exact caller-provided stack bytes and worker priority;
- `_gp`-correct EE thread creation;
- a trampoline ending in `ExitThread()`;
- one active thread slot;
- caller-bounded `ReferThreadStatus`/DelayThread dormancy proof;
- no force termination;
- DeleteThread only after proven dormancy;
- retained signal-before-wait event semantics with one pending wake;
- no DelayThread polling for the frame-slot event;
- teardown only after exact worker join/release;
- no Application activation, Display/P7, producer, Transport or RFB behavior.

## State-head CI

State commit:

`631dad7847ff0a775a61afcb2b8fdfced88da918`

Workflow:

`35582395601` — run #327 — SUCCESS first attempt.

Observed canonical jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

## Evidence boundaries

PENDING_LOCAL=NONE_FOR_FOREMAN_STATE_PUBLICATION
A004_P7_PHYSICAL_PRESENTATION=NOT_RUN_NOT_CLAIMED
A003_R5_PS2_WORKER_EXECUTION_BINDING=ACTIVE_RECONSTRUCTION_PACKET
P7_WIRED_INTO_LIVE_APP_LOOP=NO
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM_FROM_CALIBRATION=NOT_IMPLEMENTED
PI_START_PRODUCER_ADMISSION=NOT_IMPLEMENTED
Q7_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes:

`A003-PS2-WORKER-EXECUTION-BINDING-R5`

and emits exactly one immutable Reconstruction log using:

- ROLE_KEY=`reconstruction`;
- WORK_ITEM_KEY=`a003-mpeg-generation`;
- WORKER_KEY=`interactive`.

Do not begin Application activation, producer START/RETIRE, P7 live-loop wiring
or Q7 retirement orchestration in the same shift.

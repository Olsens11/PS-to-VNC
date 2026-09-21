# Reconstruction shift — A003 PS2 worker execution binding R5

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T05:49:12-04:00
COMPLETED_AT=2026-09-21T06:01:05-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=f3f69cb9d659b1d8d31e2d0d40b6d9b9a22f1664
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-PS2-WORKER-EXECUTION-BINDING-R5`

The bounded objective was to reconstruct the MPEG-owned concrete PlayStation 2
execution binding that makes the already accepted R2 synchronous decoder and R4
asynchronous MPEG worker instantiable against PS2SDK mechanisms.

The binding had to supply exact aligned memory operations, independent decoder
and worker synchronization resources, one retained signal-before-wait frame-slot
event, and one cooperative EE worker-thread slot whose completion is proven from
kernel thread state.

It was explicitly not allowed to start an MPEG run, choose worker stack size or
priority, choose hidden join timing defaults, call the decoder step itself,
perform libmpeg/IPU work, call P7/Display/scheduler/compositor, arm the media
clock, allocate generation identity, own producer/Transport/RFB lifecycle, or
perform later Application activation/retirement orchestration.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0033;
- Reconstruction Contract revision 0006;
- work-log contract revision 0007;
- current Wire Runtime Decisions and Architecture Overlay;
- accepted R2 `src/mpeg/decoder.{c,h}`;
- accepted R3 `src/mpeg/ps2_decoder_backend.{c,h}`;
- accepted R4 `src/mpeg/worker.{c,h}`;
- current `src/input/input_runtime.c` as clean PS2 EE-thread/semaphore precedent;
- frozen CP2P MPEG worker/safe-stop material only as forensic mechanism evidence;
- current canonical direct-R5900 and linked-build manifests.

Starting branch authority:

`f3f69cb9d659b1d8d31e2d0d40b6d9b9a22f1664`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## New MPEG-owned PS2 binding

Added:

- `src/mpeg/ps2_worker_runtime.h`;
- `src/mpeg/ps2_worker_runtime.c`.

The explicit binding object owns only mechanism/resources for one worker
interval:

- caller-supplied bounded join poll delay/count values;
- one decoder-lock semaphore;
- one distinct worker/slot-lock semaphore;
- one retained frame-event semaphore;
- one active EE thread ID slot;
- the exact saved R4 thread entry and argument;
- ownership facts for live allocations and kernel resources;
- thread-created/started/dormancy-proof facts.

There is no process-global mutable binding singleton.

The R3 SMS backend's existing single-instance restriction remains separate and
unchanged.

## Binding API

Public operations are:

- `pstvnc_mpeg_ps2_worker_runtime_init()`;
- `pstvnc_mpeg_ps2_worker_runtime_operations()`;
- `pstvnc_mpeg_ps2_worker_runtime_release()`.

Initialization requires explicit nonzero:

- `join_poll_delay_us`;
- `join_poll_max_count`.

The operations function binds one initialized object into the accepted R2/R4
operation contracts:

- decoder memory ops;
- decoder sync ops;
- worker memory ops;
- worker thread ops;
- worker sync ops;
- frame-event ops.

No worker stack-size, priority, generation, scheduling or other product default
is added by R5.

## Exact aligned memory mechanics

Both R2 decoder memory and R4 worker-stack memory map to the same binding-owned
aligned allocator seam because their byte/alignment requirements are already
explicit in the accepted contracts.

Allocation:

1. requires nonzero byte count;
2. requires a power-of-two alignment;
3. calls PS2 libc `memalign(alignment, byte_count)` with the exact requested
   values;
4. verifies the returned pointer actually satisfies the requested alignment;
5. records one live allocation for teardown fencing.

Release calls `free()` and decrements the binding's owned allocation count.

No size substitution, hidden 64-KiB H1 worker stack, extra full-frame copy, or
alignment rounding policy is introduced by the binding.

## Independent synchronization layout

Initialization creates three distinct PS2 semaphore resources.

Decoder lock:

- initial count 1;
- maximum count 1;
- used only by the R2 decoder synchronization callbacks.

Worker/slot lock:

- initial count 1;
- maximum count 1;
- used only by R4 worker/slot synchronization callbacks.

Frame event:

- initial count 0;
- maximum count 1;
- used only by R4 event signal/wait callbacks.

Decoder and worker synchronization therefore never alias the same lock
authority.

Lock/unlock map directly to blocking `WaitSema()` / `SignalSema()` and
return failure when the callback contract can report failure.

## Retained signal-before-wait event

The frame event uses a count-0/max-1 PS2 semaphore.

Therefore:

- a signal before wait leaves one pending wake;
- a later blocking wait consumes that wake immediately;
- repeated signals may coalesce while one wake is already pending;
- the worker still rechecks its protected R4 slot/stop predicate after every
  wake.

Event wait calls only blocking `WaitSema()`.

It contains no DelayThread polling loop and no hidden event timer.

## Exact EE-thread creation

R4 still allocates its own stack through the injected worker memory ops and
supplies the exact stack pointer, stack byte count and priority to thread create.

The PS2 binding copies those exact values into `ee_thread_t`:

- `thread.stack` = exact R4 supplied pointer;
- `thread.stack_size` = exact R4 supplied byte count;
- `thread.initial_priority` = exact R4 supplied priority;
- `thread.gp_reg = &_gp`;
- no hidden priority or stack default.

One binding exposes only one active thread slot. A second create while the slot
is active fails, and start/destroy require the exact current thread ID.

## Required EE trampoline

R4's portable worker entry returns normally.

The PS2 binding therefore creates the EE thread on a private trampoline rather
than on the portable entry directly.

`StartThread()` receives the binding object as its trampoline argument.

The trampoline:

1. reads the exact R4 entry saved during create;
2. reads the exact saved R4 argument;
3. calls only that entry with that argument;
4. after the portable entry returns, calls `ExitThread()`.

The binding performs no product behavior from the trampoline.

## Dormancy-proven bounded join

R4's synchronized `worker_finished` fact is not treated as PS2 kernel
termination proof.

The concrete join callback repeatedly calls `ReferThreadStatus()` and succeeds
only when the exact EE thread reports:

`THS_DORMANT`

The number of observations and inter-observation delay come only from the
caller's explicit nonzero:

- `join_poll_max_count`;
- `join_poll_delay_us`.

`DelayThread()` appears only between those bounded join observations.

No H1 1-ms / 3000-loop timing value was restored.

There is no `TerminateThread()`, force termination, or live-thread delete.

## Delete ordering and start-failure containment

Normal R4 release reaches thread destroy only after R4 join has completed.

The binding deletes the EE thread with `DeleteThread()` only after
`THS_DORMANT` has been proven.

The destroy callback can also prove dormancy directly for the narrow
StartThread-failure cleanup path, where R4 may ask to destroy a created thread
that never successfully entered the normal joined interval.

If dormancy cannot be proven or `DeleteThread()` fails:

- the active thread slot remains owned;
- the stack allocation remains fenced by R4;
- final binding release refuses to reclaim lock/event resources.

No possibly-live worker resource is guessed free.

## Resource lifetime and partial initialization

Binding release refuses while either:

- one thread slot is still active; or
- any allocation issued through the binding remains live.

Only after those facts clear does it delete:

1. frame-event semaphore;
2. worker/slot-lock semaphore;
3. decoder-lock semaphore.

Initialization also tracks partial ownership.

If later semaphore creation fails, already-created semaphores are unwound in
reverse order. If a kernel delete itself fails, the remaining semaphore ID and
`resources_owned` fact are preserved so release can be retried rather than
silently abandoning ownership.

The operation bundle is unavailable unless full initialization succeeded.

## Owner-boundary review

Static source review at final pre-log authority confirms the adapter contains:

- no `TerminateThread`;
- no decoder-step call;
- no libmpeg/IPU decode call;
- no P7/Application frame-consumer call;
- no Display scheduler/compositor/presentation call;
- no media-clock call;
- no Transport call;
- no RFB call;
- no generation allocation/mutation;
- no producer START/RETIRE action;
- no GS/dmaKit ownership.

The only delay call is the bounded caller-parameterized dormancy poll.

The frame event blocks on its semaphore rather than polling.

## Build integration

Updated:

- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`.

The current linked object list now includes:

`build/reconstruction/issue7/mpeg_ps2_worker_runtime.o`

Strict direct PS2 compilation now includes:

`src/mpeg/ps2_worker_runtime.c`

No R2/R3/R4/P7 test source or product source was modified.

## Dictionary integration

Canonical deterministic reconciliation added 120 maintained MPEG definitions for
the two new binding files.

Generated reconciliation commit changed only:

- `src/mpeg/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The MPEG portal count advanced from 491 to 611 definitions.

A follow-up maintained-prose commit now explicitly says that `src/mpeg/` owns:

- the concrete PS2 worker execution binding;
- aligned memory mechanics;
- independent decoder/worker locks;
- one retained frame event;
- dormancy-proven EE thread lifecycle;
- caller-supplied worker stack/priority/join-poll values.

The existing exclusions of producer, Transport, RFB, Display/GS, scheduler,
calibration and Application orchestration remain explicit.

## Commit chronology

R5 worker range after Foreman base
`f3f69cb9d659b1d8d31e2d0d40b6d9b9a22f1664`:

1. `d361baef50fc0eece2737e5147256d863d162aff`
   — `feat(mpeg): add PS2 worker execution binding`;
2. `f03e51ea05137060fa2c73350a18ce13efc2be30`
   — tree-identical canonical dictionary-reconciliation trigger;
3. `6cb92ae8d8a48da96b24587e67c593936e1fd27c`
   — automation-created `docs(symbols): reconcile current clean definitions`;
4. `99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`
   — `docs(symbols): describe PS2 worker execution binding`.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `src/mpeg/SYMBOLS.md`;
- `src/mpeg/ps2_worker_runtime.c`;
- `src/mpeg/ps2_worker_runtime.h`.

No existing decoder, worker, SMS backend, Application, Display, Platform
graphics, Transport, RFB, media-clock, audio, input, UI, framebuffer or config
source changed.

## CI history / unrelated host-test flake

Behavior-head run:

`35585969300`

at:

`d361baef50fc0eece2737e5147256d863d162aff`

proved:

- host-unit PASS;
- direct PS2 compile PASS;
- linked current-source build PASS;
- expected project/dictionary red gates only because the two new MPEG files had
  not yet been reconciled.

The direct compile log explicitly reported:

`PS2_COMPILE=src/mpeg/ps2_worker_runtime.c`

The tree-identical reconciliation-trigger run:

`35586074163`

proved:

- host-unit PASS;
- direct PS2 compile PASS;
- linked current-source build PASS;
- deterministic dictionary reconciliation SUCCESS.

Its pre-reconciliation project/dictionary jobs were expected red because they
ran before the generated dictionary commit landed.

The host log on that identical product-source tree explicitly reported:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`.

## Final coherent machine evidence

Final pre-log authority:

`99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`

GitHub Actions run:

`35586198869`

initially had every non-host canonical job PASS while the pre-existing
timing-sensitive Transport runtime host test produced unrelated assertions.
R5 changed no Transport source.

The single failed host job was rerun on the identical commit with no tree
change.

Workflow attempt 2 completed SUCCESS.

Final observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host rerun explicitly reports:

- `transport_runtime_test: PASS`;
- `transport_audio_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_WORKER_TEST=PASS`;
- `APP_MPEG_FRAME_TEST=PASS`.

Final direct PS2 evidence explicitly reports:

- `PS2_COMPILE=src/mpeg/ps2_worker_runtime.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked evidence explicitly compiles
`src/mpeg/ps2_worker_runtime.c` into
`build/reconstruction/issue7/mpeg_ps2_worker_runtime.o`, links that object into
the current ELF in both clean reproducibility builds, and reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

Final project/dictionary evidence reports:

- `SOURCE_DICTIONARIES=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_TOPOLOGY_CONTRACT=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`.

This remains repository/source/build evidence rather than physical PS2 execution
qualification.

## A003 R5 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R5-C1 EXPLICIT_PS2_BINDING_OWNER — MET

One explicit MPEG-owned runtime object owns all R5 PS2 semaphore/thread
mechanics for one worker interval. No global mutable binding singleton or generic
Platform thread framework was introduced.

### A003-R5-C2 ALIGNED_MEMORY_FIDELITY — MET

Decoder and worker memory callbacks pass exact caller byte/alignment requests to
`memalign()`, verify alignment and free through the paired callback. No hidden
stack-size substitution or frame copy exists.

### A003-R5-C3 INDEPENDENT_LOCKS — MET

Decoder and worker/slot synchronization use two independently created count-1
PS2 semaphores. Neither path aliases the other lock or depends on
unsynchronized volatile state.

### A003-R5-C4 EE_THREAD_TRAMPOLINE_EXIT — MET

Create installs the PS2 trampoline with `&_gp`; StartThread receives the
binding context; the trampoline invokes only the exact saved R4 entry/argument
and then calls `ExitThread()`.

### A003-R5-C5 CALLER_THREAD_VALUES — MET

The exact R4-supplied stack pointer, stack byte count and priority flow directly
into `ee_thread_t`. Join delay/count are separate explicit caller values.
No H1 scheduling/timing default was restored.

### A003-R5-C6 DORMANCY_PROVEN_JOIN — MET

Join calls `ReferThreadStatus()` and succeeds only on `THS_DORMANT`.
R4's portable `worker_finished` fact is not treated as kernel termination.

### A003-R5-C7 NO_FORCE_TERMINATION — MET

Join is bounded/cooperative. No `TerminateThread()` or live-thread
force-delete exists. Failure preserves resource ownership.

### A003-R5-C8 RETAINED_EVENT_WAKE — MET

The frame event is a count-0/max-1 PS2 semaphore. Signal-before-wait is retained,
repeated signals may coalesce, and wait is a blocking `WaitSema()` rather than
polling.

### A003-R5-C9 RESOURCE_LIFETIME — MET

Binding release refuses while a thread slot or any binding allocation remains
live. Semaphore teardown follows worker join/destroy/stack release, and partial
init/delete failures retain explicit ownership for retry.

### A003-R5-C10 SINGLE_ACTIVE_THREAD — MET

Create refuses an already-active slot. Start/destroy require the exact current
thread ID. IDs/entry/argument state clear only after successful dormant
`DeleteThread()`.

### A003-R5-C11 OWNER_BOUNDARY — MET

The adapter contains no decoder step, libmpeg/IPU behavior,
Display/P7/scheduler/compositor, media-clock, producer, Transport, RFB,
generation-allocation or GS ownership.

### A003-R5-C12 CLEAN_PS2_EVIDENCE — MET

The exact final pre-log authority passes preserved R2/R3/R4/P7 host coverage,
complete strict dictionaries, project/topology checks, direct R5900 compilation,
SMS dedicated compilation and linked current-source reproducibility.

Physical qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A003_R5_REQUIRED_REPOSITORY_MACHINE_GATES
PS2_WORKER_BINDING_COMPILED_AND_LINKED=YES
PS2_WORKER_BINDING_WIRED_INTO_CURRENT_APP_LIVE_LOOP=NO
APPLICATION_MPEG_ACTIVATION=NOT_IMPLEMENTED
PRESENTATION_ARM_FROM_CALIBRATION=NOT_IMPLEMENTED
PRODUCER_START_ACTIVATION=NOT_IMPLEMENTED
P5_RETIREMENT_ORCHESTRATION=NOT_IMPLEMENTED
PRODUCER_RETIRE_ACK=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
WIRE_LOSS_MPEG_RECOVERY=NOT_IMPLEMENTED
FINAL_APPLICATION_MPEG_TRANSACTION=NOT_IMPLEMENTED
PHYSICAL_PS2_WORKER_THREAD_LIFECYCLE=NOT_RUN_NOT_CLAIMED
PHYSICAL_PS2_MPEG_DECODE_PRESENTATION=NOT_RUN_NOT_CLAIMED
FULL_A003_A004_RUNTIME_COMPLETION=NOT_CLAIMED
HARDWARE_PENDING=YES

R5 creates no obligation for immediate physical testing because its assigned
acceptance surface is repository/source/build evidence. Any later physical
qualification remains a distinct Foreman-authorized hardware step.

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`99d1b46dddad0dfc9b7ec05d1ed239cba6143dc2`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- explicit one-binding lifetime;
- exact aligned-memory fidelity;
- distinct decoder/worker locks;
- count-0/max-1 retained event semantics;
- exact EE stack/priority/`&_gp` mapping;
- trampoline + `ExitThread()`;
- caller-parametric bounded dormancy proof;
- no force termination;
- dormant-only `DeleteThread()`;
- failure-safe resource ownership;
- single active thread slot;
- strict MPEG owner boundary;
- final host/direct-PS2/link/dictionary evidence.

The Foreman, not this worker, chooses the next activation, retirement or final
Application orchestration packet.

This Reconstruction worker must stop after this R5 handoff.

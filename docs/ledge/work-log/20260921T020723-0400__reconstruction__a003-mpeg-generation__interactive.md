# Reconstruction shift — A003 MPEG worker / one-slot borrowed-frame rendezvous R4

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-21T01:52:00-04:00
COMPLETED_AT=2026-09-21T02:07:23-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=3bbbd8ecbfa54a1cd155c1195e990ac4101a33b2
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Timing note

The baton arrived with conversation-local time available only to the minute
(`2026-09-21T01:52-04:00`). No seconds-resolution clock was captured before the
initial authority refresh. STARTED_AT therefore records that truthful minute
boundary rather than inventing seconds. COMPLETED_AT was captured directly from
the local-time utility.

## Objective

Execute only Foreman packet:

`A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4`

The bounded objective was to add the MPEG-owned asynchronous decoder worker and a
single synchronized borrowed-frame slot between the accepted R2/R3 synchronous
decoder and a later main/Application-thread consumer.

The worker had to remain the sole decoder initialize/step/release owner; copy one
nonzero caller generation verbatim; publish exactly one borrowed MPEG-domain
picture at a time; prevent decoder run-ahead while that picture remained
AVAILABLE or CLAIMED; provide exact claim/release fencing; preserve lost-wake
safety without polling; preserve the R2 no-synthetic-EOF stop rule; expose
terminal truth only across an explicit join fence; and remain independent from
Display/Presentation/GS/Application runtime behavior.

This shift did not begin A003->A004 runtime consumption, scheduler/compositor
calls, first-frame presentation promotion, presentation-drop execution, P5
retirement transitions, final concrete PS2 thread/event adapter work, producer
START/RETIRE orchestration, RFB restoration, Transport residual finalization, or
final Application orchestration.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0031;
- Reconstruction Contract revision 0006;
- A003 MPEG-generation audit revision 0001;
- accepted R2 `src/mpeg/decoder.{c,h}`;
- accepted R3 `src/mpeg/ps2_decoder_backend.{c,h}`;
- current A004 compositor/scheduler/presentation headers only as prohibited
  downstream ownership context;
- current audio session lifecycle/thread abstraction as style evidence only;
- current canonical host, strict PS2 compile and linked-build manifests.

Starting branch authority:

`3bbbd8ecbfa54a1cd155c1195e990ac4101a33b2`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## Frozen worker / safe-stop evidence

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Freshly traced:

- `experiments/media-harness-h1/h1_cp2p_mpeg_worker.h`;
- `experiments/media-harness-h1/h1_cp2p_mpeg_worker.c`;
- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`.

Recovered useful facts:

- one MPEG worker owns decoder-call/thread lifetime;
- asynchronous stop is lifecycle state, not data-callback EOF;
- worker teardown must wait until the active decoder call returns;
- join/delete occurs only after worker completion;
- a stopping decoder boundary may account a completed picture but must not
  manufacture additional stream semantics.

Historical shapes deliberately not restored:

- hardcoded worker stack/priority values;
- polling-based 1ms stop/join loops;
- direct worker calls into graphics/presentation;
- worker ownership of common-clock or generation-presentation state;
- H1 direct clear-video/old retirement ordering.

## Current style evidence

Current `src/audio/session.{c,h}` was used only as clean lifecycle style
evidence:

- caller-supplied stack bytes and priority;
- injected memory/thread/synchronization mechanics;
- explicit start/stop/join/release phases;
- join as the worker-write visibility fence;
- worker-visible memory not reclaimed before join.

R4 does not import audio types or behavior.

## New MPEG worker owner

Added:

- `src/mpeg/worker.h`;
- `src/mpeg/worker.c`.

The public worker object owns:

- one embedded synchronous `pstvnc_mpeg_decoder_t`;
- copied caller decoder configuration and decoder memory/sync/platform ops;
- copied caller worker values;
- injected worker memory/thread/sync/event ops;
- worker stack/thread lifecycle facts;
- exact caller run generation;
- one synchronized frame slot;
- final synchronized outcome.

It stores no Display, scheduler, compositor, Presentation, GS, RFB, Application,
media-clock or Transport-private state.

## Caller-owned execution mechanics

`pstvnc_mpeg_worker_start()` requires explicit caller values:

- worker stack byte count;
- worker priority;
- decoder configuration;
- decoder memory ops;
- decoder sync ops;
- decoder platform ops;
- worker stack memory ops;
- thread create/start/join/destroy ops;
- worker lock/unlock ops;
- retained-signal event signal/wait ops.

R4 adds no CONFIG/profile defaults and no hidden H1 tuning.

Stack alignment is an explicit MPEG-worker contract
(`PSTVNC_MPEG_WORKER_STACK_ALIGNMENT=16`), while stack size and priority remain
caller supplied.

## Exact caller generation

Start requires one nonzero generation and copies it verbatim into
`worker->run_generation`.

The worker never increments, decrements or mints generation identity.

The following public operations validate the exact generation:

- request stop;
- claim;
- release frame;
- status;
- join;
- outcome;
- release.

Wrong/stale generation fails closed before changing the slot or lifecycle.

## Sole decoder-call ownership

Only the private worker thread calls:

- `pstvnc_mpeg_decoder_initialize()`;
- `pstvnc_mpeg_decoder_step()`;
- `pstvnc_mpeg_decoder_release()`.

The owner thread may call the existing synchronized
`pstvnc_mpeg_decoder_request_stop()` lifecycle request while the decoder is
live, but it never performs initialize/step/release and never enters the
libmpeg-facing data callback.

Before decoder resource release, the worker marks its decoder-live fact false
under worker synchronization, then releases the decoder from the worker thread.

## One-slot state machine

The synchronized slot states are:

`EMPTY -> AVAILABLE -> CLAIMED -> EMPTY`

The worker publishes a PICTURE_READY result only when the slot is EMPTY.

The slot contains exactly one copied
`pstvnc_mpeg_decoded_picture_t` metadata value. The pixels remain the decoder's
borrowed storage.

After publication the worker calls only the injected retained-signal event wait
until the protected slot predicate returns to EMPTY or stop handling discards an
unclaimed AVAILABLE frame.

Therefore the worker cannot enter another decoder step while a frame is
AVAILABLE or CLAIMED.

No second full-frame buffer/copy was introduced.

## Exact claim / release

`pstvnc_mpeg_worker_claim()` is nonblocking.

It succeeds only for:

- exact run generation;
- current AVAILABLE slot.

Success atomically changes AVAILABLE -> CLAIMED and copies to the consumer:

- the exact borrowed decoder picture metadata;
- exact run generation;
- claim token equal to the immutable decoder picture ordinal.

Claim does not release decoder storage and does not wake/advance the worker.

`pstvnc_mpeg_worker_release_frame()` requires:

- exact run generation;
- nonzero exact claim token;
- current CLAIMED state;
- token equal to the slot picture ordinal.

Only then does it clear CLAIMED -> EMPTY exactly once and signal the worker
event.

Wrong generation, wrong token, stale release and duplicate release fail closed.

## Lost-wake safety

The injected event contract explicitly requires signal retention:

- `signal()` retains one pending wake;
- a later `wait()` may consume that earlier signal immediately.

The worker never depends on a polling cadence or timeout.

After every wake the worker reacquires its synchronization boundary and
re-evaluates:

- stop requested;
- slot state.

The deterministic host fixture models a retained pending bit and deliberately
causes claim/release or stop/release from inside the wait hook before the wait
consumes the pending signal. The fixture asserts that the pending wake survives
that signal-before-consume ordering.

## Stop while AVAILABLE

An exact stop request:

- records worker stop state under synchronization;
- if slot is AVAILABLE, clears it immediately to EMPTY;
- therefore makes that unclaimed frame unclaimable;
- forwards stop to the synchronous decoder if decoder ownership is live;
- signals the worker event.

The worker wakes/rechecks the predicate, observes stop, and cannot decode another
frame.

Focused evidence proves:

- one picture was decoded/published;
- stop discards the AVAILABLE frame;
- subsequent claim returns FRAME_UNAVAILABLE;
- no second picture call occurs;
- final outcome is STOPPED;
- decoder report retains `pictures_decoded=1`.

## Stop while CLAIMED

Exact stop does not clear CLAIMED.

The borrowed picture pointer and metadata remain pinned and the worker cannot
advance or release decoder storage while the exact consumer claim exists.

Focused evidence proves before exact release:

- slot remains CLAIMED;
- picture buffer contents remain stable;
- no second picture call occurs;
- decoder destroy has not occurred;
- join returns FRAME_OUTSTANDING without invoking the injected thread join;
- wrong-generation release fails;
- wrong-token release fails.

Exact release changes the slot to EMPTY, signals the worker, and only then may
the worker observe stop and complete decoder teardown.

## Stop before decoder call

If stop is requested before worker thread execution:

- the worker still establishes its owned decoder interval;
- observes worker stop before first step;
- forwards that request into the existing decoder stop owner;
- first decoder step returns STOPPED without one platform picture call.

Focused evidence proves:

- picture call count = 0;
- no frame-slot wait;
- final outcome STOPPED;
- decoded picture accounting remains zero.

## Stop during active decoder call

The public stop path may set the decoder's existing synchronized stop request
while `pstvnc_mpeg_decoder_step()` owns its injected picture call.

R2 semantics remain unchanged:

- decoder feed callback never reads stop;
- no Transport activity/EOF is synthesized;
- the active picture call returns normally from real decoder/Transport facts;
- successful picture accounting occurs exactly once;
- R2 post-call stop observation returns STOPPED rather than PICTURE_READY.

Therefore the worker publishes no frame and never enters its slot wait.

Focused R4 evidence proves:

- one picture call;
- no published claim;
- no worker event wait;
- final STOPPED;
- decoder report `pictures_decoded=1`.

## Blocked-decoder caveat

R4 does not signal Transport's private MPEG waiter and does not manufacture
Transport activity or exhaustion.

If the worker is blocked inside decoder/feed wait, a local R4 stop request alone
does not force that call to return.

Final Application orchestration remains responsible for producer/Transport
lifecycle facts required to make that active call progress to a real boundary.

## Terminal outcome truth

Worker outcome distinguishes:

- COMPLETED;
- STOPPED;
- FAILED.

It also preserves:

- worker result;
- exact decoder result;
- decoder release result;
- final decoder report;
- exact run generation.

Earlier successful frames do not mask a later decoder failure.

Focused evidence publishes/releases one frame, fails the next picture call, and
proves final outcome remains FAILED with
`PSTVNC_MPEG_DECODER_PICTURE_FAILED` and prior
`pictures_decoded=1`.

## Join / release lifetime

`pstvnc_mpeg_worker_join()` is the worker-write visibility fence.

It refuses:

- wrong generation;
- any non-empty slot;
- worker not yet synchronized-finished.

Only after EMPTY + finished does it invoke injected thread join and mark the
thread joined.

`pstvnc_mpeg_worker_outcome()` is unavailable until worker completion plus
join.

`pstvnc_mpeg_worker_release()` refuses an unjoined worker; after join it:

- destroys injected thread state;
- frees only the worker-owned stack;
- leaves decoder resources already released by the worker thread;
- retires the worker object.

Therefore worker-visible stack state and borrowed decoder picture storage cannot
be reclaimed while a claim is outstanding.

## Build integration

Updated:

- `tests/Makefile`;
- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`.

Focused host target:

`mpeg-worker-unit`

builds:

- `tests/unit/mpeg_worker_test.c`;
- `src/mpeg/worker.c`;
- `src/mpeg/decoder.c`.

Strict direct PS2 compile now includes:

`src/mpeg/worker.c`

The current linked build now contains:

`build/reconstruction/issue7/mpeg_worker.o`

R3's stable pinned SMS dependency and its dedicated
`-G8192 -mgpopt -mno-abicalls` compile model remain unchanged.

## Commit chronology

R4 worker range after Foreman base
`3bbbd8ecbfa54a1cd155c1195e990ac4101a33b2`:

1. `51f45cdb59cd61565246159e1915f002b0c7de2d`
   — `feat(mpeg): add one-slot decoder worker rendezvous`;
2. `785ec520812a9c7e07862717a4fab87febc6c623`
   — tree-identical canonical dictionary-reconciliation trigger;
3. `d63e8af665a689acf38e3ce58f6cce62f3bf61a1`
   — automation-created `docs(symbols): reconcile current clean definitions`;
4. `e56d9cb47439ef60c82e3dd78328d7ea6732995e`
   — `docs(symbols): describe MPEG worker rendezvous ownership`.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `src/mpeg/SYMBOLS.md`;
- `src/mpeg/worker.c`;
- `src/mpeg/worker.h`;
- `tests/Makefile`;
- `tests/unit/mpeg_worker_test.c`.

No decoder core, concrete SMS backend, Display, Platform graphics, Transport,
RFB, Application, media-clock, UI, audio, config, framebuffer or input source
changed.

## Dictionary integration

Canonical reconciliation added 229 symbols for the two new MPEG worker files and
removed zero existing MPEG symbols.

Only:

- `src/mpeg/SYMBOLS.md`;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`

changed in the reconciliation commit.

Maintained MPEG responsibility prose now explicitly names:

- MPEG-owned asynchronous decoder/thread lifecycle;
- exact-run one-slot borrowed-frame claim/release rendezvous;

while retaining explicit exclusion of GS/Presentation/Application ownership.

## Static owner-boundary review

At final pre-log authority
`e56d9cb47439ef60c82e3dd78328d7ea6732995e`:

worker source/header contain:

- no Display scheduler/compositor/presentation dependency;
- no gsKit/dmaKit/Platform graphics dependency;
- no RFB dependency;
- no Application dependency;
- no media-clock dependency;
- no Transport private runtime/channel/queue dependency;
- no generation increment/mint;
- no correctness-critical polling/delay mechanism.

The worker consumes only the accepted public MPEG decoder seam plus injected
caller mechanics.

## Host evidence

First behavior-head run:

`35566728875`

at:

`51f45cdb59cd61565246159e1915f002b0c7de2d`

had:

- strict PS2 compile PASS;
- linked current-source build PASS;
- expected dictionary/project red gates for the two new MPEG files;
- host-unit stopped before the new worker target because the known unrelated
  timing-sensitive `transport_audio_test` assertion at line 698 fired:
  `available == 4u && producer_done == 1`.

No Transport/audio source changed in R4.

Unchanged-source reconciliation-trigger run:

`35566789017`

then produced:

- host-unit PASS;
- `transport_runtime_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`;
- direct PS2 compile PASS;
- linked reproducibility PASS;
- deterministic dictionary reconciliation SUCCESS.

The trigger tree's project/dictionary jobs were expected red because they ran
before the generated reconciliation commit existed.

## Final coherent machine evidence

Final pre-log authority:

`e56d9cb47439ef60c82e3dd78328d7ea6732995e`

GitHub Actions run:

`35566964942`

completed SUCCESS first attempt.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected.

Final host logs explicitly report:

- `transport_runtime_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `MPEG_WORKER_TEST=PASS`.

Direct PS2 compile explicitly reports:

- `PS2_COMPILE=src/mpeg/worker.c`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Linked current-source build compiles and links `mpeg_worker.o` in both clean
reproducibility builds and reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

This remains source/machine/build evidence rather than physical thread/decode
qualification.

## A003 R4 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R4-C1 WORKER_DECODER_OWNER — MET

Only the private MPEG worker thread initializes, steps and releases the decoder.
Owner-thread stop uses only the decoder's accepted synchronized lifecycle request.

### A003-R4-C2 EXACT_RUN_FENCE — MET

Start requires one nonzero caller generation, copies it verbatim, never mints or
increments it, and public lifecycle/slot operations reject wrong generations.

### A003-R4-C3 ONE_SLOT_BORROW — MET

Synchronized slot is exactly EMPTY -> AVAILABLE -> CLAIMED -> EMPTY and carries
the exact R2 borrowed picture metadata without a full-frame copy.

### A003-R4-C4 CLAIM_RELEASE — MET

Claim is nonblocking and exact-run only. Release requires exact run plus
picture-ordinal claim token, changes CLAIMED -> EMPTY once, and wakes the worker.
Stale/duplicate/wrong releases fail closed.

### A003-R4-C5 BUFFER_LIFETIME — MET

Worker performs no next decoder step while slot is AVAILABLE/CLAIMED. CLAIMED
survives stop unchanged and decoder teardown is delayed until exact release.

### A003-R4-C6 LOST_WAKE_SAFETY — MET

Injected event contract retains signal-before-wait; worker rechecks synchronized
predicates after every wake and contains no polling timer. Host fixture proves
pending signals are consumed after release/stop state changes.

### A003-R4-C7 STOP_PUBLICATION_FENCE — MET

Stop discards unclaimed AVAILABLE, pins CLAIMED until exact release, prevents
another decoder step, and stop during the active decoder call suppresses the
stopping frame publication.

### A003-R4-C8 SAFE_STOP_STREAM_BOUNDARY — MET

R4 adds no Transport wake/EOF mechanism and does not modify the R2 feed callback.
Blocked-decoder caveat is explicit. Existing decoder safe-stop tests remain
green.

### A003-R4-C9 TERMINAL_TRUTH — MET

COMPLETED/STOPPED/FAILED outcome preserves exact decoder result, decoder-release
result and final report. Earlier frames do not mask later failure.

### A003-R4-C10 THREAD_LIFETIME — MET

Join refuses outstanding frames and unfinished worker state; join is the outcome
visibility fence; release requires join before thread destruction/stack reclaim.
Decoder release occurs on worker thread after borrowed-slot obligations clear.

### A003-R4-C11 OWNER_BOUNDARY — MET

No Display, GS, Presentation, RFB, Application, media-clock or Transport-private
runtime dependency entered the worker. No generation allocator was introduced.

### A003-R4-C12 CLEAN_EVIDENCE — MET

Focused/full host suite, project check, complete strict dictionaries, direct PS2
compile, SMS dedicated compile preservation, and linked current-source
reproducibility pass on exact final source authority.

Physical qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A003_R4_REQUIRED_REPOSITORY_MACHINE_GATES
CONCRETE_PS2_WORKER_THREAD_ADAPTER=NOT_IMPLEMENTED_IN_R4
CONCRETE_PS2_EVENT_ADAPTER=NOT_IMPLEMENTED_IN_R4
A003_TO_A004_MAIN_THREAD_CONSUMER=NOT_IMPLEMENTED
BORROWED_FRAME_TO_COMPOSITOR_FRAME_MAPPING=NOT_IMPLEMENTED
FIRST_FRAME_RUNTIME_PROMOTION=NOT_RUNTIME_WIRED
SCHEDULER_RUNTIME_WAIT_DROP_PRESENT=NOT_IMPLEMENTED
P5_RETIREMENT_RUNTIME_ORCHESTRATION=NOT_IMPLEMENTED
PRODUCER_START_RETIRE=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
PHYSICAL_PS2_WORKER_DECODE=NOT_RUN_NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`e56d9cb47439ef60c82e3dd78328d7ea6732995e`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- sole worker decoder-call ownership;
- exact generation fencing;
- one-slot state transitions;
- exact ordinal claim/release;
- no-run-ahead and borrowed-buffer lifetime;
- retained-signal lost-wake behavior;
- stop-before-call / stop-during-call / AVAILABLE / CLAIMED interleavings;
- terminal outcome precedence;
- join/release lifetime fence;
- absence of cross-domain runtime ownership;
- final host/direct-PS2/link evidence.

The Foreman, not this worker, chooses the bounded A003->A004 main-thread
runtime-consumer seam or any still-needed concrete PS2 mechanics packet.

This Reconstruction worker must not begin that later work in this shift.

# Reconstruction shift — A003 decoded-picture handoff R2

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T20:36:00-04:00
COMPLETED_AT=2026-09-20T20:43:39-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=c88a9fcf0011eacc5c43bdc11184af29c320907f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-DECODED-PICTURE-HANDOFF-R2`

The bounded objective was to reconstruct one MPEG-owned decoder step that makes
exactly one completed decoded picture observable to a caller without moving
feed, sequence, Transport, safe-stop or decoder-call ownership out of MPEG.

The step had to expose a small borrowed decoded-picture value with explicit
lifetime, preserve the proven no-synthetic-EOF stop rule, preserve existing
`pstvnc_mpeg_decoder_run()` behavior/reporting, and remain independent from
Presentation, scheduling, graphics, clocks, generations and Application
orchestration.

This shift did not begin an MPEG worker thread, worker/main-thread mailbox,
scheduler/compositor invocation, concrete new libmpeg backend, START/RETIRE
orchestration, Pi lifecycle, RFB restoration, or final Application wiring.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0029;
- Reconstruction Contract revision 0006;
- work-log contract revision 0006 / log format revision 0001;
- A003 MPEG-generation audit revision 0001;
- accepted A004 P1-P6 source only as downstream-boundary context;
- current `src/mpeg/decoder.{c,h}`;
- current `tests/unit/mpeg_decoder_test.c`;
- current neutral Display MPEG-frame contract only to understand the later
  adapter boundary;
- current canonical host/PS2 build manifests.

Starting branch authority was:

`c88a9fcf0011eacc5c43bdc11184af29c320907f`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No user terminal proxy or physical PS2 action was required or requested.

## Current decoder evidence inspected

Current decoder source established:

- MPEG owns the one injected platform `picture()` call and all callback
  registration/lifetime;
- sequence callback validates width/height and returns the decoder-owned reusable
  picture allocation;
- feed callback consumes bytes only through:
  - `pstvnc_transport_mpeg_activity_snapshot()`;
  - `pstvnc_transport_mpeg_read_available()`;
  - `pstvnc_transport_mpeg_wait_activity()`;
- feed callback deliberately never reads local `stop_requested`;
- local stop is observed before entering `picture()` and again only after
  decoder-call ownership returns;
- existing `run()` increments `pictures_decoded` once for a successful
  picture before its post-call stop observation;
- finite Transport exhaustion plus decoder end maps to COMPLETE;
- decoder end without real exhaustion/owner stop maps to UNEXPECTED_END;
- sequence, Transport, transfer, platform-picture, synchronization and
  accounting failures already have distinct result values;
- release rejects active decoder-call ownership and destroys/releases only after
  call ownership has returned.

Current focused tests already covered:

- feed payload versus aligned DMA transfer accounting;
- finite exhaustion;
- event-driven empty-queue wake;
- stop requested inside active picture without callback EOF synthesis;
- release fenced while picture call is active;
- invalid sequence;
- unexpected decoder end;
- Transport and submit failures;
- setup/allocation failures.

## Frozen authority traced

Frozen authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Freshly traced at least:

- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- `experiments/media-harness-h1/h1_cp2p_mpeg_worker.c`;
- `experiments/media-harness-h1/h1_cp2p_session_coordinator.c`.

The A003 audit was also re-read for the known false-EOF boundary.

Recovered rule:

- an asynchronous stop request must never be translated into EOF from inside an
  active `MPEG_Picture()` data callback;
- active picture/decode ownership must be allowed to return normally using only
  real Transport data/exhaustion/failure facts;
- local stop is observed at the completed-picture/project-code boundary after
  decoder ownership returns;
- existing successful-picture accounting remains truthful even when stop becomes
  visible during that call;
- a stopping boundary is lifecycle completion, not a new frame publication
  opportunity.

That frozen/current rule directly governs the new step seam.

## API shape

Extended only:

- `src/mpeg/decoder.h`;
- `src/mpeg/decoder.c`.

Added result:

`PSTVNC_MPEG_DECODER_PICTURE_READY = 2`

Added value:

`pstvnc_mpeg_decoded_picture_t`

with fields:

- `const void *pixels`
  - read-only caller view of decoder-owned reusable picture storage;
- `size_t byte_count`
  - usable macroblock-backed byte extent for the validated current sequence;
- `size_t capacity_bytes`
  - decoder-owned picture allocation capacity;
- `uint32_t width`
  - validated sequence width;
- `uint32_t height`
  - validated sequence height;
- `uint32_t bytes_per_pixel`
  - decoder configuration pixel-width fact;
- `uint32_t picture_ordinal`
  - monotonically increasing decoder-local decoded-picture ordinal.

No generation/session/run identity is present.

Added public operation:

`pstvnc_mpeg_decoder_step(decoder, picture, report)`

A successful call:

1. observes owner stop before decoder entry;
2. enters the exact existing decoder-call ownership fence;
3. invokes `platform_ops.picture()` at most once;
4. leaves decoder-call ownership;
5. applies the same sequence/feed/picture/terminal classifications as the prior
   `run()` body;
6. increments existing picture accounting exactly once for a successful picture;
7. observes local stop only after decoder ownership returned;
8. only when no stop is then visible fills the borrowed picture value and
   returns PICTURE_READY.

It returns before another `picture()` call begins.

## Borrowed-frame lifetime

The picture allocation remains owned by the decoder.

The public header explicitly defines the borrowed value as valid only until:

- the next `pstvnc_mpeg_decoder_step()` call on the same decoder; or
- decoder release;

whichever occurs first.

A caller requiring longer lifetime must consume/copy the picture before allowing
the decoder to advance.

The decoder stores the current sequence's actual macroblock-backed byte extent in
`sequence_picture_bytes`, independently from the larger allocation capacity.

Focused tests use a 32x16 sequence inside the existing 64x32 allocation and prove:

- usable `byte_count = 1024`;
- allocation `capacity_bytes = 4096`;
- width=32;
- height=16;
- bytes-per-pixel=2;
- ordinal 1 is returned by the first successful step;
- the next step reuses the same picture pointer and overwrites its contents,
  demonstrating why the prior borrowed view cannot be retained as immutable
  caller storage;
- decoder release releases that exact allocation.

Every callable step with a non-null picture output clears that output before
validation/terminal handling, so non-PICTURE_READY results cannot leave stale
borrow metadata consumable.

## Stop-before-call behavior

If owner stop is already visible before a step begins:

- `platform_ops.picture()` is not called;
- result is STOPPED;
- report reflects existing accounting;
- returned picture value is invalid/zero;
- no new picture ordinal is manufactured.

Focused test proves `picture_calls == 0` and
`pictures_decoded == 0`.

## Stop-during-active-picture behavior

The libmpeg-facing feed callback remains structurally unchanged and still never
consults local stop.

The focused active-stop step fixture:

1. enters one `picture()` call;
2. requests local stop from inside that active call;
3. still feeds seven real Transport bytes through the normal callback;
4. returns from decoder ownership;
5. records one successful decoded-picture accounting fact;
6. observes stop only at the existing post-call authority point;
7. returns STOPPED;
8. leaves the borrowed picture output invalid.

Evidence in that fixture:

- one picture call;
- one feed submit;
- payload bytes consumed = 7;
- padded transfer bytes submitted = 16;
- pictures decoded = 1;
- no caller-consumable frame is published.

Therefore local stop still cannot become synthetic callback EOF and a stopping
boundary does not publish a new frame.

## First-picture neutrality

Ordinal 1 is now directly caller-observable through PICTURE_READY.

The MPEG decoder still imports/calls no:

- Display scheduler;
- Display compositor;
- Presentation state;
- Platform graphics;
- media clock;
- RFB;
- Application.

The step performs no timing wait, presentation/drop decision, GS operation,
clock arm or visible-ownership transition.

Generation remains entirely outside decoder state/value/API.

## Existing run compatibility

`pstvnc_mpeg_decoder_run()` remains public.

Its implementation now loops over `pstvnc_mpeg_decoder_step()`:

- PICTURE_READY means continue decoding;
- every terminal/error result is returned directly.

This preserves the prior lifecycle behavior while avoiding a second competing
decoder loop.

Existing decoder tests pass unchanged semantics for:

- finite COMPLETE;
- owner STOPPED;
- SEQUENCE_INVALID;
- TRANSPORT_FAILED;
- TRANSFER_FAILED;
- PICTURE_FAILED path classification;
- UNEXPECTED_END;
- synchronization/accounting behavior;
- report counters.

The original active-stop `run()` test also still passes unchanged, including
`pictures_decoded == 1` on the stop-during-picture boundary.

## Accounting

No duplicate accounting path was introduced.

Exactly one code path in `step()` now owns:

- picture ordinal / `pictures_decoded` increment;
- successful picture publication.

Feed callback remains the sole owner of:

- payload bytes consumed;
- padded transfer bytes submitted;
- feed callback count;
- Transport result facts.

`run()` does not add any separate accounting; it only loops over step results.

## Transport boundary

The decoder still consumes channel-4 data only through the existing public
Transport MPEG access/wake seam.

No queue, semaphore, credit, receiver or physical-stream internals were imported.

No local stop path changes `feed_exhausted` or synthesizes Transport EOF.

## Commits

A003 R2 worker source/integration range after Foreman base:

1. `bf6ec2bf2a870287b624de65b44c14835fed2e8f`
   — `feat(mpeg): add one-picture decoder handoff`;
2. `7a3a43274fae3d975f9450e10e20911f52f504e8`
   — tree-identical canonical dictionary-reconciliation trigger;
3. `d1fd65633292952d1c12683433a06c4234a03085`
   — automation-created `docs(symbols): reconcile current clean definitions`;
4. `ed5c03e9c3666d784d073b2922d8bff02d10e800`
   — `refactor(mpeg): fail closed borrowed picture output`.

No build-manifest change was required because current decoder translation units
were already directly present in canonical strict PS2 compile and linked-build
manifests.

## Exact changed paths

Compared with Foreman base:

`c88a9fcf0011eacc5c43bdc11184af29c320907f`

final pre-log authority:

`ed5c03e9c3666d784d073b2922d8bff02d10e800`

changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `src/mpeg/SYMBOLS.md`;
- `src/mpeg/decoder.c`;
- `src/mpeg/decoder.h`;
- `tests/unit/mpeg_decoder_test.c`.

No Display, Platform, RFB, Transport, Application, UI, input, framebuffer,
audio, config or media-clock source changed.

## Dictionary integration

The first strict behavior-head dictionary audit identified exactly 23 new/current
symbols for the step/frame change.

Deterministic reconciliation changed only:

- `src/mpeg/SYMBOLS.md`;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

It removed only the former `run()`-local
`picture_result` and `stop_requested` rows because those locals moved into
`pstvnc_mpeg_decoder_step()`.

It added the new step function/declaration/parameters/locals, borrowed-picture
structure/type/fields, sequence byte-count field and PICTURE_READY enum value.

Maintained MPEG responsibility prose now explicitly names the one-picture
borrowed decoded-frame publication boundary.

## Static owner-boundary review

At final pre-log authority:

`ed5c03e9c3666d784d073b2922d8bff02d10e800`

decoder source/header contain:

- no Display dependency;
- no scheduler/compositor/presentation dependency;
- no gsKit/dmaKit or Platform graphics dependency;
- no RFB dependency;
- no Application dependency;
- no media-clock dependency;
- no MPEG generation/run-generation field or API;
- the expected public Transport MPEG activity/read/wait seam only.

## Final machine evidence

Final exact source/dictionary authority:

`ed5c03e9c3666d784d073b2922d8bff02d10e800`

GitHub Actions run:

`35548538065`

completed SUCCESS.

Observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on the coherent ordinary commit.

Host log explicitly reports:

- `mpeg_decoder_test: PASS`;
- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Strict PS2 compile explicitly reports:

- `PS2_COMPILE=src/mpeg/decoder.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

The linked current-source build compiles/links
`build/reconstruction/issue7/mpeg_decoder.o`
in both reproducibility builds and reports:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

This is source/machine/build evidence, not physical libmpeg hardware
qualification.

## A003 R2 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R2-C1 ONE_PICTURE_STEP — MET

One public step invokes at most one decoder picture call and returns
PICTURE_READY after one successful completed boundary.

### A003-R2-C2 FRAME_VALUE — MET

PICTURE_READY publishes read-only pointer, usable bytes, allocation capacity,
validated width/height, bytes-per-pixel and decoder-local ordinal.

No generation or Presentation authority is carried.

### A003-R2-C3 BUFFER_LIFETIME — MET

Lifetime is explicit: next step or release invalidates the borrowed value.
Focused tests prove same-buffer overwrite on next step and allocation release at
decoder release.

### A003-R2-C4 FIRST_PICTURE_NEUTRALITY — MET

Ordinal 1 is observable through PICTURE_READY. MPEG has no scheduler,
compositor, graphics, clock or visible-ownership side effect.

### A003-R2-C5 SAFE_STOP_PRECALL — MET

Pre-existing stop returns STOPPED with zero picture calls and no publication.

### A003-R2-C6 SAFE_STOP_ACTIVE_CALL — MET

Stop raised during the active picture call does not affect feed callback EOF,
real bytes continue through Transport, accounting remains truthful, stop is
observed after call ownership returns, and no new borrowed picture is published.

### A003-R2-C7 TERMINAL_COMPATIBILITY — MET

Finite exhaustion, unexpected end and established failure classes retain their
prior classifications; existing tests remain green.

### A003-R2-C8 RUN_COMPATIBILITY — MET

Existing `run()` now loops over the step seam and otherwise preserves terminal
behavior/reporting rather than maintaining a second decoder loop.

### A003-R2-C9 ACCOUNTING — MET

Picture/feed/payload/transfer accounting remains exactly once and monotonic.
Run adds no duplicate counters.

### A003-R2-C10 OWNER_BOUNDARY — MET

No Display scheduler/compositor, GS, RFB, Application, media-clock or
generation-lifecycle dependency entered MPEG.

### A003-R2-C11 TRANSPORT_BOUNDARY — MET

Existing public MPEG Transport activity/read/wait APIs remain the only ingest
boundary; local stop cannot synthesize exhaustion.

### A003-R2-C12 CLEAN_EVIDENCE — MET

Focused/full host tests, project check, complete strict dictionaries, direct PS2
compile and linked current-source reproducibility pass on the exact final source
authority.

Hardware claims remain separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A003_R2_REQUIRED_REPOSITORY_MACHINE_GATES
MPEG_WORKER_THREAD=NOT_IMPLEMENTED
WORKER_MAIN_THREAD_MAILBOX=NOT_IMPLEMENTED
BORROWED_FRAME_RUNTIME_RENDEZVOUS=NOT_IMPLEMENTED
SCHEDULER_COMPOSITOR_RUNTIME_CALLS=NOT_IMPLEMENTED
CONCRETE_NEW_PS2_LIBMPEG_ADAPTER=NOT_IMPLEMENTED_IN_R2
START_RETIRE_ORCHESTRATION=NOT_IMPLEMENTED_IN_R2
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED_IN_R2
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
PHYSICAL_PS2_MPEG_DECODE=NOT_RUN_NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`ed5c03e9c3666d784d073b2922d8bff02d10e800`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- exactly-one-picture step ownership;
- borrowed-frame fields and lifetime;
- ordinal-1 neutrality;
- pre-call and active-call stop behavior;
- unchanged false-EOF prevention;
- terminal and `run()` compatibility;
- exact accounting;
- strict owner/Transport boundary;
- final host/direct-PS2 evidence.

The Foreman, not this worker, chooses whether the next bounded seam is the MPEG
worker/main-thread rendezvous or a still-missing concrete platform adapter.

This Reconstruction worker must not begin either later seam in this shift.

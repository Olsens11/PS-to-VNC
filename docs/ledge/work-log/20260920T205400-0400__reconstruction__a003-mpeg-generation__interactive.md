# Reconstruction shift — A003 concrete PS2 SMS-libmpeg backend R3

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T20:54:00-04:00
COMPLETED_AT=2026-09-20T21:14:48-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a2a608522edd43c102973ebfb162e7fe3952c117
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Execute only Foreman packet:

`A003-PS2-LIBMPEG-BACKEND-R3`

The bounded objective was to instantiate the accepted
`pstvnc_mpeg_decoder_platform_ops_t` seam with the qualified PS2
SMS-libmpeg/IPU mechanism, promote the exact pinned mature SMS dependency into a
stable product dependency path, preserve its dedicated compilation model, and
make the complete backend/dependency part of canonical clean PS2 compile/link
authority.

This shift deliberately did not begin the MPEG worker/session, worker/main-thread
rendezvous, scheduler/compositor runtime calls, Presentation state transitions,
START/RETIRE orchestration, producer lifecycle, RFB restoration or final
Application orchestration.

## Timing note

The worker refreshed repository authority immediately after baton receipt but did
not capture a seconds-resolution clock value before that first authority read.
`STARTED_AT` therefore records the truthful minute-level start boundary rather
than inventing false second precision. `COMPLETED_AT` was captured directly from
the local-time utility.

## Authority consumed

Consumed current authority at wake:

- Foreman State revision 0030;
- Reconstruction Contract revision 0006;
- work-log contract revision 0006 / log format revision 0001;
- A003 MPEG-generation audit revision 0001;
- accepted A003 R1/R2 decoder source;
- current canonical strict PS2 compile and current-source linked-build manifests;
- current EXP3 mature SMS reference copy and provenance;
- current A004 Display/P4 source only as a prohibited ownership boundary.

Starting branch authority was:

`a2a608522edd43c102973ebfb162e7fe3952c117`

Execution classification:

`AUTONOMOUS_RECONSTRUCTION`

No live-Pi terminal proxy or physical PS2 action was required or requested.

## Current dependency / build evidence inspected

Before writing, current authority confirmed:

- clean product compiled `src/mpeg/decoder.c` but had no concrete
  `pstvnc_mpeg_decoder_platform_ops_t` implementation;
- the current linked build contained no SMS libmpeg source/object;
- product link flags did not use system `-lmpeg`;
- mature SMS authority existed only under
  `experiments/media-stream-exp3/vendor/sms-libmpeg/`;
- successful EXP3 SMS targets compiled the mature C and assembly sources with:
  - `-D_EE`;
  - `-O2`;
  - `-G8192`;
  - `-mgpopt`;
  - `-mno-abicalls`;
  - `-Wall`;
  - `-mno-check-zero-division`.

The EXP3 makefiles explicitly document why the SMS assembly core must not simply
be forced through the ordinary clean `-G0` model: symbolic operations in branch
delay slots can otherwise change instruction placement.

## Frozen mechanism evidence traced

Frozen H1 authority:

`3426f28b93de9519ca93e5f0e0aaf8b67cfca845`

Freshly traced:

- `experiments/media-harness-h1/h1_video_runtime.c` around:
  - `h1_video_reference_ipu_reset()`;
  - `h1_video_feed_ipu()`;
  - `MPEG_Initialize()`;
  - `_MPEG_Set16(1)`;
  - first and subsequent `MPEG_Picture()` calls;
  - `h1_video_release_session()`;
- `experiments/media-harness-h1/CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- relevant mature SMS EXP3 make targets.

Recovered hardware/backend rules:

1. before decoder initialization, disable/quiesce D3/D4 DMA ownership under the
   qualified DMAC-enable sequence, zero channel MADR/QWC, reset IPU, wait for
   BUSY to clear, issue BCLR, wait again, then restore the prior non-busy/reset
   IPU control value;
2. initialize ordinary TO_IPU DMA ownership;
3. call `MPEG_Initialize()` with a data callback, sequence callback and
   backend-local stream PTS storage;
4. select the qualified RGB16 path with `_MPEG_Set16(1)`;
5. feed already-padded bytes only after
   `dma_channel_wait(DMA_CHANNEL_toIPU, 0)`, then submit exact QWC through
   `dma_channel_send_normal()`;
6. first picture call uses `MPEG_Picture(NULL, &pts)`;
7. later calls use the decoder-owned picture pointer returned through the
   sequence callback;
8. before decoder destruction, wait for ordinary TO_IPU ownership and then call
   `MPEG_Destroy()`;
9. local asynchronous stop is not a backend callback/EOF fact;
10. historical GIF upload, GS setup/draw, VRAM, VBlank timing, scheduler and
    visible stage behavior are separate Presentation concerns and must not enter
    the clean decoder backend.

## Stable pinned SMS dependency

Promoted a stable product dependency path:

`vendor/sms-libmpeg/`

with:

- `UPSTREAM.txt`;
- `include/libmpeg.h`;
- `include/libmpeg_internal.h`;
- `src/libmpeg.c`;
- `src/libmpeg_core.S`.

The stable copy reuses the exact Git blob objects already present in the pinned
EXP3 reference.

Pinned upstream authority remains:

`c1898094725ad750ec20e10cc148b39d7c8a9c65`

Exact stable/reference blob identity:

- `UPSTREAM.txt`
  - `70ca1542e3c89f471a5994fbbbb87baf61c2d375`;
- `include/libmpeg.h`
  - `ee2195b52dc3a7112537046426a80aa0e603fa6c`;
- `include/libmpeg_internal.h`
  - `c2f80a9104380634c3ef6749baa2e5f0acb5c13e`;
- `src/libmpeg.c`
  - `f9e5f11689fa6ed3759365249c2d7cfb7335e2fb`;
- `src/libmpeg_core.S`
  - `93638fd62e58bfac8c6ed1c5fc84ef119d318438`.

The four upstream source/header files therefore remain byte-identical and retain
their original Eugene Plotnikov / PS2DEV authorship and Academic Free License
2.0 notices.

No third-party source was edited to fit the clean product.

## Concrete backend

Added:

- `src/mpeg/ps2_decoder_backend.h`;
- `src/mpeg/ps2_decoder_backend.c`.

The caller-owned backend context stores only:

- decoder-supplied feed callback/context;
- decoder-supplied sequence callback/context;
- sequence-callback-captured decoder picture pointer and capacity;
- backend-local current-stream and picture PTS scratch;
- known-state prepared fact;
- SMS decoder initialized fact;
- first-picture-pending fact.

It stores no:

- Transport queue/access state;
- stop state;
- generation/run identity;
- worker/thread/mailbox state;
- scheduler state;
- Presentation state;
- GS resources;
- media-clock authority;
- RFB state;
- Application transaction state.

Public helper:

`pstvnc_ps2_mpeg_decoder_backend_platform_ops()`

populates all accepted decoder platform operations with one explicit backend
context.

## Platform-op mapping

### prepare_known_state

`pstvnc_ps2_mpeg_prepare_known_state()`:

- fail-closes if another SMS backend already owns the process-global decoder;
- reserves the one active backend under the same interrupt-state pattern used by
  qualified H1 hardware code;
- executes the unconditional D3/D4 plus IPU reset/BCLR sequence;
- initializes `DMA_CHANNEL_toIPU`;
- records known-state ownership.

No old `VIDEO_IPU_RESET_EACH_SESSION` toggle exists.

### initialize

`pstvnc_ps2_mpeg_initialize()`:

- requires exact reserved/prepared ownership;
- stores only decoder feed/sequence callbacks and contexts;
- clears backend-local sequence picture / PTS state;
- calls pinned `MPEG_Initialize()`;
- selects `_MPEG_Set16(1)`;
- records one initialized SMS interval.

### SMS data callback

`pstvnc_ps2_mpeg_sms_feed()` delegates directly to the accepted decoder feed
callback.

It does not:

- inspect owner stop;
- read Transport directly;
- synthesize EOF;
- update decoder byte accounting.

Therefore R1/R2 false-EOF prevention remains owned by the decoder seam.

### SMS sequence callback

`pstvnc_ps2_mpeg_sms_sequence()`:

- rejects null/nonpositive signed SMS width/height;
- converts only validated positive dimensions to the decoder callback;
- delegates allocation/bounds authority to the accepted decoder sequence
  callback;
- captures only its returned decoder-owned picture pointer/capacity for later
  `MPEG_Picture()`;
- returns that same picture pointer to SMS.

No Display-frame or generation type is imported.

### submit_feed

`pstvnc_ps2_mpeg_submit_feed()`:

- accepts only the decoder-supplied buffer/request;
- requires nonzero payload;
- requires payload <= transfer bytes;
- requires nonzero transfer length divisible by 16;
- requires 16-byte-aligned source address;
- bounds transfer length to the representable DMA request;
- waits for `DMA_CHANNEL_toIPU`;
- submits exactly `transfer_bytes >> 4` QWC;
- does no repadding, Transport read, EOF decision or accounting.

### picture

`pstvnc_ps2_mpeg_picture()`:

- uses `MPEG_Picture(NULL, ...)` while first-picture-pending;
- after a successful sequence/picture boundary requires a captured decoder-owned
  picture pointer;
- clears first-picture-pending only on successful picture decode;
- passes that captured decoder-owned pointer to later `MPEG_Picture()` calls;
- uses only backend-local PTS scratch.

PTS state carries no common-clock/Presentation meaning.

### destroy / release_known_state

`pstvnc_ps2_mpeg_destroy()`:

- requires exact active initialized ownership;
- waits for TO_IPU ownership;
- calls `MPEG_Destroy()` once for that initialized interval;
- clears decoder-initialized / picture / PTS facts;
- leaves reusable dependency/resource ownership reserved until the decoder calls
  release-known-state.

`pstvnc_ps2_mpeg_release_known_state()`:

- rejects release while SMS remains initialized;
- clears the exact process-global owner reservation;
- zeros only backend-owned state.

It does not claim unsupported global hardware restoration. A future acquisition
must execute prepare-known-state again.

## Single-instance constraint

Mature SMS libmpeg exposes process-global entry points/state.

R3 therefore uses one module-global active-backend pointer and fail-closes a
second prepare while any backend owns the interval.

The reservation/release update uses the qualified EE interrupt-state pattern
rather than inventing a generic project-wide lock manager.

This is an explicit backend constraint, not a claim that independent SMS decoder
instances exist.

## Strict ownership boundary

Final static inspection of
`src/mpeg/ps2_decoder_backend.{c,h}` confirms its implementation includes only:

- the accepted decoder backend header;
- pinned `libmpeg.h`;
- pinned `libmpeg_internal.h`;
- PS2 DMA / EE-register / kernel / tamtypes hardware headers;
- standard integer/string headers.

It contains no:

- gsKit call;
- graph/draw/packet/GIF/GS/VBlank API;
- Display scheduler/compositor/presentation dependency;
- Platform graphics dependency;
- Transport API or header;
- RFB API/header;
- Application API/header;
- generation/run-generation field;
- media-clock API;
- local stop/request-stop API.

TO_IPU/IPU/DMAC hardware is the only device mechanism introduced.

## Canonical build integration

Updated:

- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`.

The current linked build now directly contains:

- `mpeg_ps2_decoder_backend.o`;
- `sms_libmpeg.o`;
- `sms_libmpeg_core.o`.

The product makefile uses:

`SMS_VENDOR = vendor/sms-libmpeg`

and does not link system `-lmpeg`.

Backend C is compiled through the ordinary clean product flags.

SMS C and assembly are compiled separately with:

`-D_EE -O2 -G8192 -mgpopt -mno-abicalls -Wall -mno-check-zero-division`

plus the stable pinned include path.

The link adds the ordinary PS2SDK DMA library required by the concrete TO_IPU
backend:

`-ldma`

The strict PS2 compile checker separately:

- compiles the clean backend;
- byte-compares stable SMS provenance and all four upstream files against the
  pinned EXP3 reference;
- confirms upstream pin
  `c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- compiles pinned `libmpeg.c` under dedicated SMS flags;
- compiles pinned `libmpeg_core.S` under dedicated SMS flags.

No ordinary clean translation unit was weakened from its existing strict
`-G0`/warning policy.

## Third-party compile observations

The unchanged mature SMS assembly emits its historical assembler warnings about
`$at` use without `.set noat`.

The current linker also reports warnings when the dedicated non-abicalls SMS
objects are linked with ordinary product objects.

R3 did not modify third-party source or discard the required
`-mno-abicalls` model merely to suppress these warnings.

Both full linked builds nevertheless complete successfully and reproduce
byte-for-byte under current linked reproducibility authority.

These warnings remain transparent build observations, not physical decoder
qualification.

## Host evidence policy

No fake host hardware backend was added.

The PS2 backend directly dereferences EE hardware registers and calls PS2 DMA/SMS
entry points; introducing a parallel host emulation layer solely to claim adapter
coverage would not qualify those mechanisms.

The already-accepted host decoder suite remains the behavioral evidence for the
platform-ops contract and safe-stop semantics. R3's concrete hardware
implementation is proven at repository/machine level by direct R5900 compile,
pinned-dependency identity, complete link and linked reproducibility.

Physical runtime remains separate.

## Commit chronology

R3 worker range after Foreman base
`a2a608522edd43c102973ebfb162e7fe3952c117`:

1. `18eb2d063c0f20970feb969a6d0771cc75512f4f`
   — `feat(mpeg): add pinned PS2 SMS decoder backend`;
2. `91eb3c67f684aef324296e995eb2b7b429c1e0cf`
   — `fix(tooling): verify pinned SMS bytes hermetically`;
3. `cbdd6bcad7222c73a1033df910aa3634f2ca3258`
   — tree-identical canonical dictionary-reconciliation trigger;
4. `c739bf6e8cec4d2105cf1d94051436d8144012a2`
   — automation-created `docs(symbols): reconcile current clean definitions`;
5. `16344f98329083802198b7b15437dda3055a6ec2`
   — `docs(symbols): describe concrete PS2 MPEG backend`.

## Exact changed paths

Compared with Foreman base, final pre-log authority changes only:

- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`;
- `mk/issue7-clean.mk`;
- `scripts/check-clean-ps2-compile.sh`;
- `src/mpeg/SYMBOLS.md`;
- `src/mpeg/ps2_decoder_backend.c`;
- `src/mpeg/ps2_decoder_backend.h`;
- `vendor/sms-libmpeg/UPSTREAM.txt`;
- `vendor/sms-libmpeg/include/libmpeg.h`;
- `vendor/sms-libmpeg/include/libmpeg_internal.h`;
- `vendor/sms-libmpeg/src/libmpeg.c`;
- `vendor/sms-libmpeg/src/libmpeg_core.S`.

No decoder core, Display, Platform graphics, Transport, RFB, Application,
media-clock, UI, audio, config, framebuffer or input product source changed.

## Dictionary integration

Canonical reconciliation added 76 current symbols for the two new MPEG backend
files and removed zero prior MPEG symbols.

Generated portal and strict dictionary state are coherent.

Maintained MPEG responsibility prose now explicitly states ownership of the
concrete PS2 SMS-libmpeg/IPU backend while explicitly excluding
GS/Presentation/Application ownership.

## CI chronology

Initial behavior-head run `35549846362` established:

- clean backend C itself compiled;
- linked current-source build passed with backend + SMS C + SMS assembly;
- project/dictionary failures were expected missing coverage for the two new MPEG
  source files;
- strict compile later failed only because the first identity checker used
  `git hash-object` inside a hermetic PS2 container that intentionally lacks
  `git`;
- host-unit hit the previously observed unrelated timing-sensitive
  `transport_runtime_test` lines 787/789.

The identity check was corrected to hermetic byte comparison against the pinned
EXP3 reference.

Canonical reconciliation run `35549946628` then proved:

- host-unit PASS;
- backend direct compile PASS;
- pinned SMS identity/provenance PASS;
- dedicated SMS C/assembly compile PASS;
- full PS2 link/reproducibility PASS;
- pre-reconciliation project/dictionary jobs red as expected on the trigger tree;
- dictionary reconciliation SUCCESS.

Final coherent-head run:

`35550113167`

at:

`16344f98329083802198b7b15437dda3055a6ec2`

completed SUCCESS on attempt 2.

Attempt 1's only failing job was the known unrelated timing-sensitive
`transport_runtime_test`, which also cascaded into later quiesce fixture
assertions. No Transport source changed in R3.

Only that failed host job was re-run on the same exact commit through GitHub
Actions; attempt 2 passed without source changes.

Final observed jobs:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on coherent ordinary authority.

Final host logs explicitly report:

- `transport_runtime_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`.

Final strict PS2 compile explicitly reports:

- `PS2_COMPILE=src/mpeg/ps2_decoder_backend.c`;
- stable identity PASS for all four upstream SMS files;
- `SMS_VENDOR_PROVENANCE_PASS=c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- `PS2_SMS_COMPILE=vendor/sms-libmpeg/src/libmpeg.c`;
- `PS2_SMS_COMPILE=vendor/sms-libmpeg/src/libmpeg_core.S`;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`.

Final linked logs explicitly show backend C under ordinary product flags and
both SMS units under:

`-G8192 -mgpopt -mno-abicalls`

then link all three objects into the current ELF and report:

- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## A003 R3 worker disposition

These are Reconstruction-worker dispositions for independent Foreman review.

### A003-R3-C1 CONCRETE_BACKEND — MET

One clean MPEG-owned PS2 backend populates every accepted decoder platform op
without changing decoder API ownership.

### A003-R3-C2 PINNED_SMS_IDENTITY — MET

Stable product dependency uses exact pinned EXP3/upstream SMS blobs and preserved
provenance/license notices. CI byte-compares all files and confirms upstream pin.

### A003-R3-C3 SMS_BUILD_MODEL — MET

SMS C/assembly compile separately under the qualified
`-G8192 -mgpopt -mno-abicalls` model, not ordinary product `-G0`.

### A003-R3-C4 KNOWN_STATE — MET

The qualified D3/D4 quiesce plus IPU reset/BCLR sequence is unconditional before
SMS initialization. The obsolete reset-each-session toggle is absent.

### A003-R3-C5 NO_GS_OWNERSHIP — MET

Backend contains no GIF/GS/VRAM/VBlank/graph/draw/packet or P4 graphics
mechanism.

### A003-R3-C6 CALLBACK_TRANSLATION — MET

SMS feed delegates only to decoder feed; SMS sequence validates signed dimensions
then delegates to decoder sequence and captures only its picture pointer/capacity.

### A003-R3-C7 RGB16 — MET

Backend explicitly selects `_MPEG_Set16(1)` with no runtime pixel-mode
experiment.

### A003-R3-C8 TO_IPU_DMA — MET

submit-feed validates exact already-padded/aligned decoder transfer bytes, waits
TO_IPU ownership and sends exact QWC without Transport reads, repadding, EOF or
accounting policy.

### A003-R3-C9 PICTURE_CALLS — MET

First call uses NULL; successful later calls use the decoder-owned
sequence-callback picture pointer. PTS scratch is backend-local only.

### A003-R3-C10 LIFECYCLE_FENCE — MET

Destroy requires exact active initialized ownership, waits TO_IPU and calls
`MPEG_Destroy()` once per initialized interval. Release rejects initialized
state, clears only exact owner state, and overlapping backend acquisition fails
closed.

### A003-R3-C11 SAFE_STOP_BOUNDARY — MET

Backend contains no stop API/state and SMS data callback delegates unchanged to
decoder feed, so local lifecycle stop cannot become backend-generated EOF.
Existing R2 decoder tests remain green.

### A003-R3-C12 CLEAN_BUILD_EVIDENCE — MET

Existing decoder host tests, full host suite, project/dictionary gates, clean
backend compile, pinned SMS identity, dedicated SMS C/assembly compile, complete
link and linked reproducibility all pass on exact final source authority.

Physical decoder qualification remains separate.

## Evidence gaps / non-claims

PENDING_LOCAL=NONE_FOR_A003_R3_REQUIRED_REPOSITORY_MACHINE_GATES
PHYSICAL_PS2_SMS_DECODE=NOT_RUN_NOT_CLAIMED
IPU_RESET_RUNTIME_EFFECT=NOT_PHYSICALLY_REQUALIFIED
TO_IPU_DMA_RUNTIME=NOT_PHYSICALLY_REQUALIFIED
REPEATED_SMS_SESSION_LIFECYCLE=NOT_PHYSICALLY_REQUALIFIED
MPEG_WORKER_THREAD=NOT_IMPLEMENTED
WORKER_MAIN_THREAD_RENDEZVOUS=NOT_IMPLEMENTED
SCHEDULER_COMPOSITOR_RUNTIME_CALLS=NOT_IMPLEMENTED
START_RETIRE_ORCHESTRATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
HARDWARE_PENDING=YES

## Exact next pickup

Return the baton to Foreman at pre-log authority:

`16344f98329083802198b7b15437dda3055a6ec2`

plus this immutable Reconstruction log commit.

Foreman should independently verify:

- stable SMS blob/provenance identity;
- dedicated SMS compile model;
- exact IPU/DMAC known-state mechanism;
- callback and TO_IPU DMA translation;
- first/subsequent picture semantics;
- single-instance/destroy/release fencing;
- absence of GS/Presentation/stop/runtime orchestration ownership;
- final direct compile/link/reproducibility evidence;
- observed unchanged-third-party assembler/linker warnings.

The Foreman, not this worker, chooses the bounded MPEG worker/main-thread
rendezvous packet from returned source.

This Reconstruction worker must not begin that later seam in this shift.

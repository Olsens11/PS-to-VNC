# Foreman shift — accept A003 R2 and activate PS2 libmpeg backend

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T20:51:02-04:00
COMPLETED_AT=2026-09-20T20:58:09-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=b67a077c0a56c0b4db2fdcaf5a19ed21b70f440d
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A003-DECODED-PICTURE-HANDOFF-R2` Reconstruction baton,
independently verify the one-picture decoder boundary and its evidence, then
choose and publish the smallest owner-correct next dependency before final MPEG
runtime integration.

## Authority refreshed

Live pickup branch authority was:

`b67a077c0a56c0b4db2fdcaf5a19ed21b70f440d`

Current Foreman State was revision 0029 with active packet:

`A003-DECODED-PICTURE-HANDOFF-R2`

Final worker pre-log source/dictionary authority was:

`ed5c03e9c3666d784d073b2922d8bff02d10e800`

Immutable Reconstruction log:

`docs/ledge/work-log/20260920T203600-0400__reconstruction__a003-mpeg-generation__interactive.md`

The worker did not overwrite Foreman state or begin later runtime work.

## Reconstruction commit range reviewed

After Foreman base
`c88a9fcf0011eacc5c43bdc11184af29c320907f`:

1. `bf6ec2bf2a870287b624de65b44c14835fed2e8f`
   — one-picture decoder handoff;
2. `7a3a43274fae3d975f9450e10e20911f52f504e8`
   — deterministic dictionary-reconciliation trigger;
3. `d1fd65633292952d1c12683433a06c4234a03085`
   — automated current-clean dictionary reconciliation;
4. `ed5c03e9c3666d784d073b2922d8bff02d10e800`
   — fail-closed borrowed-picture output plus maintained MPEG responsibility
   prose;
5. `b67a077c0a56c0b4db2fdcaf5a19ed21b70f440d`
   — required immutable Reconstruction log.

Changed product/test paths were bounded to:

- `src/mpeg/decoder.c`;
- `src/mpeg/decoder.h`;
- `tests/unit/mpeg_decoder_test.c`;
- `src/mpeg/SYMBOLS.md`;
- generated `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

No Display, Platform graphics, RFB, Transport implementation, media-clock,
Application, UI, audio, config or input source changed.

## Independent R2 source review

Foreman independently confirmed:

- `pstvnc_mpeg_decoder_step()` invokes at most one underlying platform
  `picture()` call and returns before any later picture call begins;
- successful publication is explicitly represented by
  `PSTVNC_MPEG_DECODER_PICTURE_READY`;
- `pstvnc_mpeg_decoded_picture_t` carries only:
  - decoder-owned read-only picture pointer;
  - usable macroblock-backed byte count;
  - allocation capacity;
  - validated sequence width/height;
  - bytes-per-pixel;
  - decoder-local picture ordinal;
- no Wire session identity, MPEG generation, Presentation state or Application
  authority enters that value;
- the borrowed picture is documented as valid only until the next decoder step
  or decoder release, whichever comes first;
- focused tests demonstrate that a second successful step reuses and overwrites
  the same picture allocation, proving that callers may not treat the borrow as
  immutable retained storage;
- every call with a writable output clears the decoded-picture value before
  validation or terminal handling, so every non-PICTURE_READY result fails
  closed rather than leaving stale borrowed metadata consumable;
- the sequence callback preserves the exact usable macroblock-backed byte extent
  separately from maximum allocation capacity;
- decoded ordinal 1 is caller-observable without any scheduler, media-clock,
  GS/compositor, visible-ownership or generation side effect;
- a stop already visible before the step prevents a new picture call;
- stop requested while `picture()` owns the call never affects the decoder data
  callback: real Transport data continues, successful-picture accounting occurs
  exactly once after decoder-call ownership returns, and STOPPED suppresses
  publication of that just-completed boundary;
- local stop therefore still cannot synthesize decoder EOF;
- real Transport exhaustion remains COMPLETE;
- decoder end without real exhaustion or owner stop remains UNEXPECTED_END;
- sequence, Transport, transfer, picture, synchronization and accounting
  failures retain their established classifications;
- `pstvnc_mpeg_decoder_run()` now loops over the one-picture step instead of
  maintaining a competing second decoder loop;
- feed/payload/padded-transfer accounting remains solely in the feed callback;
- picture accounting increments once in the step;
- channel-4 ingest still uses only the public Transport MPEG
  activity/read/wait seam;
- MPEG decoder source still imports no Display scheduler/compositor,
  Platform graphics, RFB, media clock or Application runtime dependency.

A003-R2-C1 through A003-R2-C12 are independently accepted as MET within the
bounded source/machine-evidence scope.

## R2 machine evidence

Final pre-log run:

`35548538065` — run #305 — SUCCESS.

Exact pickup-head run:

`35548659856` — run #306 — SUCCESS.

Exact-head required gates:

- host-unit — PASS;
- project-check — PASS;
- dictionary-long/complete/strict — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED as expected on a coherent ordinary head.

Exact-head logs explicitly reported:

- `mpeg_decoder_test: PASS`;
- `transport_runtime_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- direct `PS2_COMPILE=src/mpeg/decoder.c`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2 MPEG decode, repeated-session or stop/relaunch qualification was
performed or inferred.

## Next dependency selected from actual source

The worker return left two plausible next seams:

1. the MPEG worker/main-thread rendezvous; or
2. a still-missing concrete PS2 decoder platform backend.

Live clean product source resolves that ordering.

The clean tree has no implementation of
`pstvnc_mpeg_decoder_platform_ops_t`. The current product build compiles and
links only the abstract `src/mpeg/decoder.c` owner. It has no clean source that
calls:

- `MPEG_Initialize()`;
- `MPEG_Picture()`;
- `MPEG_Destroy()`;
- `_MPEG_Set16(1)`;
- ordinary TO_IPU DMA submission;
- the qualified IPU/DMAC known-state preparation.

The clean linked build also does not link the mature SMS libmpeg implementation.

Adding worker/thread/rendezvous complexity before proving the accepted decoder
platform seam against the real qualified PS2 mechanism would build concurrency
around an unrealized abstraction.

The concrete synchronous PS2 backend therefore comes first.

## Mature SMS dependency authority recovered

Current historical EXP3 authority contains the mature SMS dependency under:

`experiments/media-stream-exp3/vendor/sms-libmpeg/`

Its `UPSTREAM.txt` records:

- upstream repository: `https://github.com/ps2homebrew/SMS.git`;
- pinned upstream commit:
  `c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- copied unchanged:
  - `include/libmpeg.h`;
  - `include/libmpeg_internal.h`;
  - `src/libmpeg.c`;
  - `src/libmpeg_core.S`;
- Git blob identities:
  - `libmpeg.h` —
    `ee2195b52dc3a7112537046426a80aa0e603fa6c`;
  - `libmpeg_internal.h` —
    `c2f80a9104380634c3ef6749baa2e5f0acb5c13e`;
  - `libmpeg.c` —
    `f9e5f11689fa6ed3759365249c2d7cfb7335e2fb`;
  - `libmpeg_core.S` —
    `93638fd62e58bfac8c6ed1c5fc84ef119d318438`.

The upstream files retain their Academic Free License 2.0 notices and authorship
headers.

The successful SMS one-picture build authority also records the decoder-specific
compilation model:

- `-D_EE`;
- `-O2`;
- `-G8192`;
- `-mgpopt`;
- `-mno-abicalls`;
- `-Wall`;
- `-mno-check-zero-division`.

That special small-data model is materially important because the SMS assembly
core contains symbolic memory operations in branch-delay slots; forcing it
through the product's ordinary `-G0` model can alter instruction placement.

## Frozen backend mechanism recovered

Frozen H1/video authority confirms the qualified hardware mechanisms to preserve
while splitting presentation ownership away from MPEG:

- unconditional clean product preparation must establish known IPU/DMAC state
  before `MPEG_Initialize()`;
- historical preparation stops/clears TO_IPU/FROM_IPU DMA channel state,
  performs IPU reset/BCLR and restores the prior nonbusy/nonreset control state;
- the obsolete `VIDEO_IPU_RESET_EACH_SESSION` switch is laboratory vocabulary
  and must not return as a product toggle;
- the mature decoder is initialized with the decoder's feed and sequence
  callbacks;
- the qualified current picture path is RGB16 through `_MPEG_Set16(1)`;
- decoder feed DMA waits for ordinary TO_IPU ownership and submits aligned
  transfer bytes;
- mature SMS semantics use the first `MPEG_Picture(NULL,...)` form, then the
  decoder-owned picture buffer accepted through the sequence callback for later
  picture calls;
- PTS scratch required by SMS remains decoder/backend-local and is not the
  common Presentation clock;
- decoder teardown waits for decoder DMA ownership as required and calls
  `MPEG_Destroy()` only after active decoder-call ownership has returned.

Historical H1 GIF packets, GS initialization, texture VRAM, VBlank waits,
visible stage markers and draw scheduling are explicitly excluded because the
accepted P4 compositor is the sole current GS owner.

## Foreman state / packet publication

Published Foreman State revision 0030 in commit:

`de84cfb861b76cf533906e2064b1ec3c38e47d7f`

message:

`docs(foreman): accept A003 R2 and activate PS2 libmpeg backend`

State phase:

`A003_R2_INTEGRATED__A003_PS2_LIBMPEG_BACKEND_RECONSTRUCTION_ACTIVE__MPEG_WORKER_RENDEZVOUS_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

State flags:

- ARCHITECTURE_BLOCKER=NONE
- A004_P1_FOREMAN_ACCEPTED=YES
- A004_P2_FOREMAN_ACCEPTED=YES
- A004_P3_FOREMAN_ACCEPTED=YES
- A004_P4_FOREMAN_ACCEPTED=YES
- A004_P5_FOREMAN_ACCEPTED=YES
- A004_P6_FOREMAN_ACCEPTED=YES
- A003_DECODED_PICTURE_STEP_FOREMAN_ACCEPTED=YES
- A003_PS2_LIBMPEG_BACKEND_ACTIVE=YES
- A003_MPEG_WORKER_RENDEZVOUS=DEPENDENCY_QUEUED
- A003_APPLICATION_ORCHESTRATION=DEPENDENCY_QUEUED
- HARDWARE_DEBT_BLOCKS_UNRELATED_SOURCE=NO

Active packet:

`A003-PS2-LIBMPEG-BACKEND-R3`

Execution classification remains:

- EXECUTION_MODE=`AUTONOMOUS_RECONSTRUCTION`
- USER_TERMINAL_POLICY=`EXCEPTION_ONLY`
- PI_LOCAL_USER_PROXY_REQUIRED=`NO`

R3 is bounded to:

- concrete PS2 decoder platform operations;
- exact mature pinned SMS dependency/provenance;
- dedicated qualified SMS C/assembly build flags;
- unconditional IPU/DMAC known-state preparation;
- RGB16 libmpeg binding;
- callback translation;
- TO_IPU feed DMA;
- first/subsequent picture-call translation;
- destroy/release and single-instance fencing;
- canonical compile/link integration.

It explicitly excludes:

- the MPEG worker thread;
- worker/main-thread frame rendezvous;
- scheduler/compositor calls;
- any GS/GIF/VRAM/VBlank presentation mechanism;
- START/RETIRE control;
- Pi producer lifecycle;
- RFB restoration;
- final Application orchestration.

## State-publication CI and existing Transport host-test signal

State commit
`de84cfb861b76cf533906e2064b1ec3c38e47d7f`
triggered:

`35549238733` — run #307.

Attempt 1:

- project-check — PASS;
- dictionary-long — PASS;
- ps2-compile — PASS;
- ps2-link/current-source reproducibility — PASS;
- dictionary-reconcile — SKIPPED;
- host-unit — FAIL.

The host failure was exactly the already-known unchanged-source timing-sensitive
Transport fixture:

- `unit/transport_runtime_test.c:787` — expected RFB channel availability;
- `unit/transport_runtime_test.c:789` — expected activity-sequence advance.

No Transport or product behavior source changed in this Foreman commit.

Foreman re-ran the failed host job without changing source.

Attempt 2 completed SUCCESS. GitHub therefore records run #307 overall SUCCESS
on the exact revision-0030 state commit.

This recurrence remains evidence of nondeterminism/timing sensitivity in the
existing Transport host fixture, not evidence of an A003 R2 or R3-planning
product regression. Both the failed and successful attempts remain preserved.

## Checks / evidence for this Foreman shift

REMOTE_AUTHORITY_REFRESH=PASS
A003_R2_SOURCE_REVIEW=PASS
A003_R2_C1_C12_FOREMAN_ACCEPTANCE=PASS
A003_R2_EXACT_HEAD_CI=PASS
CONCRETE_PS2_BACKEND_MISSING=CONFIRMED
PINNED_SMS_DEPENDENCY_IDENTITY=RECOVERED
QUALIFIED_SMS_BUILD_MODEL=RECOVERED
QUALIFIED_IPU_DMAC_BACKEND_MECHANISM=RECOVERED
NEXT_OWNER_SEAM=A003_PS2_LIBMPEG_BACKEND_R3
FOREMAN_STATE_REVISION=0030
FOREMAN_STATE_COMMIT=de84cfb861b76cf533906e2064b1ec3c38e47d7f
FOREMAN_STATE_CI_RUN=35549238733
FOREMAN_STATE_CI_ATTEMPTS=2
FOREMAN_STATE_CI_FINAL=PASS
TRANSPORT_RUNTIME_TEST_FLAKE=OBSERVED_UNCHANGED_SOURCE
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
CONCRETE_CLEAN_PS2_LIBMPEG_BACKEND=ACTIVE_RECONSTRUCTION_PACKET
MPEG_WORKER_THREAD=NOT_IMPLEMENTED
WORKER_MAIN_THREAD_RENDEZVOUS=NOT_IMPLEMENTED
SCHEDULER_COMPOSITOR_RUNTIME_WIRING=NOT_IMPLEMENTED
START_RETIRE_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
PHYSICAL_PS2_MPEG_DECODE=NOT_RUN_NOT_CLAIMED
REPEATED_MPEG_GENERATION_HARDWARE=NOT_RUN_NOT_CLAIMED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes
`A003-PS2-LIBMPEG-BACKEND-R3`, emits exactly one immutable
`a003-mpeg-generation` Reconstruction log, and returns the baton to Foreman.

Do not begin the MPEG worker/rendezvous, Presentation runtime wiring or final
Application orchestration in the same shift.

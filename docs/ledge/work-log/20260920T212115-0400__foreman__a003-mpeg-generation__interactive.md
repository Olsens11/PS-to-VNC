# Foreman shift — accept A003 R3 and activate MPEG worker rendezvous

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-20T21:21:15-04:00
COMPLETED_AT=2026-09-20T21:35:53-04:00
ROLE_KEY=foreman
WORK_ITEM_KEY=a003-mpeg-generation
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=358365601c384c772ef224fe17ae04adcc4d6f94
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Consume the completed `A003-PS2-LIBMPEG-BACKEND-R3` Reconstruction baton,
independently verify the concrete PS2 backend, vendor identity/build model and
ownership boundaries, then publish the next bounded MPEG worker/frame-rendezvous
packet.

## Authority and timeout recovery

Live pickup authority was:

`358365601c384c772ef224fe17ae04adcc4d6f94`

Current Foreman State was revision 0030 with active packet:

`A003-PS2-LIBMPEG-BACKEND-R3`

Final worker pre-log source/dictionary/build authority:

`16344f98329083802198b7b15437dda3055a6ec2`

Worker immutable log:

`docs/ledge/work-log/20260920T205400-0400__reconstruction__a003-mpeg-generation__interactive.md`

During this Foreman round, message delivery timed out after the complete R3
technical review and next-packet design but before any branch ref update. Live
authority was rechecked afterward and was still exactly the worker head above
with State revision 0030. No partial Foreman state had landed and no recovery
rewrite was required.

## R3 worker range independently reviewed

After Foreman base
`a2a608522edd43c102973ebfb162e7fe3952c117`:

1. `18eb2d063c0f20970feb969a6d0771cc75512f4f`
   — concrete pinned PS2 SMS decoder backend and clean build integration;
2. `91eb3c67f684aef324296e995eb2b7b429c1e0cf`
   — hermetic pinned SMS identity verification;
3. `cbdd6bcad7222c73a1033df910aa3634f2ca3258`
   — deterministic dictionary-reconciliation trigger;
4. `c739bf6e8cec4d2105cf1d94051436d8144012a2`
   — automated current-clean dictionary reconciliation;
5. `16344f98329083802198b7b15437dda3055a6ec2`
   — maintained MPEG responsibility prose;
6. `358365601c384c772ef224fe17ae04adcc4d6f94`
   — required immutable Reconstruction work log.

## Independent R3 acceptance

Foreman independently confirmed:

- `src/mpeg/ps2_decoder_backend.{c,h}` implements every accepted decoder
  platform operation without altering the decoder API;
- decoder hardware ownership stays in MPEG;
- D3/D4 quiesce plus IPU reset/BCLR is unconditional before SMS initialization;
- TO_IPU is initialized and feed DMA waits/submits exact decoder-provided aligned
  transfer QWC;
- SMS data callback delegates only to decoder feed and cannot translate local
  stop into EOF;
- SMS sequence callback validates dimensions, delegates allocation/bounds to the
  decoder sequence callback and captures only its returned picture pointer and
  capacity;
- RGB16 is explicitly selected with `_MPEG_Set16(1)`;
- first picture uses `MPEG_Picture(NULL,...)`, later pictures use the
  decoder-owned sequence picture;
- PTS storage is backend-local only and carries no Presentation/common-clock
  authority;
- destroy waits TO_IPU and calls `MPEG_Destroy()` exactly once per initialized
  interval;
- release rejects initialized state, clears only backend-owned state and requires
  fresh preparation on future acquisition;
- mature SMS process-global decoder state is represented with one active backend
  reservation, so overlapping acquisition fails closed;
- backend contains no GS/GIF/VRAM/VBlank/draw, Presentation, scheduler,
  compositor, RFB, Transport-internal or Application ownership.

The stable product dependency under `vendor/sms-libmpeg/` is byte/Git-blob
identical to the existing pinned EXP3 reference, including provenance metadata.

Pinned SMS upstream commit:

`c1898094725ad750ec20e10cc148b39d7c8a9c65`

Verified blob identities:

- `UPSTREAM.txt` —
  `70ca1542e3c89f471a5994fbbbb87baf61c2d375`;
- `include/libmpeg.h` —
  `ee2195b52dc3a7112537046426a80aa0e603fa6c`;
- `include/libmpeg_internal.h` —
  `c2f80a9104380634c3ef6749baa2e5f0acb5c13e`;
- `src/libmpeg.c` —
  `f9e5f11689fa6ed3759365249c2d7cfb7335e2fb`;
- `src/libmpeg_core.S` —
  `93638fd62e58bfac8c6ed1c5fc84ef119d318438`.

The four upstream SMS source/header files remain unchanged with existing
authorship and Academic Free License 2.0 notices.

Canonical linked build includes backend + SMS C + SMS assembly. SMS units retain
their qualified dedicated build model:

`-D_EE -O2 -G8192 -mgpopt -mno-abicalls -Wall -mno-check-zero-division`

rather than ordinary clean `-G0`.

A003-R3-C1 through A003-R3-C12 are independently accepted as MET within the
bounded repository/machine-evidence scope.

## R3 machine evidence

Exact pickup-head run:

`35550373481` — run #313 — SUCCESS first attempt.

Observed exact-head evidence includes:

- host-unit PASS;
- project-check PASS;
- dictionary-long/strict PASS;
- ps2-compile PASS;
- ps2-link/current-source reproducibility PASS;
- dictionary-reconcile SKIPPED as expected;
- `transport_runtime_test: PASS`;
- `mpeg_decoder_test: PASS`;
- `MPEG_CALIBRATION_UNIT=PASS`;
- `RFB_FLOW_POLICY_TEST=PASS`;
- `MPEG_PRESENTATION_TEST=PASS`;
- `MPEG_SCHEDULER_TEST=PASS`;
- `MPEG_COMPOSITOR_TEST=PASS`;
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`;
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`;
- `WORK_LOG_CHECK=PASS`;
- `PS_TO_VNC_PROJECT_CHECK=PASS`;
- `PS2_COMPILE=src/mpeg/ps2_decoder_backend.c`;
- all four `SMS_VENDOR_IDENTITY_PASS` checks;
- `SMS_VENDOR_PROVENANCE_PASS=c1898094725ad750ec20e10cc148b39d7c8a9c65`;
- direct SMS C/assembly compilation;
- `SMS_DEDICATED_COMPILE_CHECK=PASS`;
- `CLEAN_PS2_COMPILE_CHECK=PASS`;
- `ISSUE7_LINKED_BUILD=PASS`;
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

No physical PS2 SMS decode, IPU reset, TO_IPU DMA, repeated-session or
stop/relaunch test occurred or is inferred.

## Next dependency selected

R2 proved that every decoded picture is borrowed decoder storage valid only until
the next decoder step or decoder release.

R3 made that decoder real against the qualified SMS/IPU mechanism.

Therefore the next owner-correct dependency is not direct Display wiring. It is
an MPEG-owned asynchronous worker plus exactly one synchronized borrowed-frame
slot.

The worker may decode and publish one exact R2 frame, but must not enter another
decoder step while that slot is AVAILABLE or CLAIMED. The main/Application side
will later claim it, present/drop it through A004, and release the exact frame.

This prevents decoder run-ahead from overwriting a live borrowed frame without
adding a second permanent full-frame copy, and keeps scheduler/compositor/GS
ownership outside MPEG.

## State / packet publication

Published Foreman State revision 0031 at:

`1822d286bbcf34288066750117a201078fd10bb0`

message:

`docs(foreman): accept A003 R3 and activate MPEG worker rendezvous`

State phase:

`A003_R3_INTEGRATED__A003_MPEG_WORKER_FRAME_RENDEZVOUS_RECONSTRUCTION_ACTIVE__A004_RUNTIME_HANDOFF_DEPENDENCY_QUEUED__FINAL_APPLICATION_ORCHESTRATION_DEPENDENCY_QUEUED`

Active packet:

`A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4`

Key R4 constraints include:

- exact nonzero caller-owned run generation;
- worker thread sole ownership of decoder initialize/step/release;
- one synchronized `EMPTY -> AVAILABLE -> CLAIMED -> EMPTY` slot;
- exact R2 borrowed picture, no full-frame copy;
- no decoder run-ahead while AVAILABLE/CLAIMED;
- fail-closed exact claim/release token;
- lost-wake-safe injected event contract;
- stop may discard AVAILABLE but cannot invalidate CLAIMED;
- decoder teardown cannot complete while a claim is live;
- local stop never becomes callback EOF;
- terminal outcome preserves decoder truth;
- explicit completion/join/destroy lifetime;
- no Display/GS/RFB/Application/Transport-internal dependency;
- no scheduler/compositor calls in this packet.

## State-head CI

State commit `1822d286bbcf34288066750117a201078fd10bb0`
triggered run:

`35551363193` — run #314.

Run #314 completed SUCCESS first attempt across:

- host-unit;
- project-check;
- dictionary-long;
- ps2-compile;
- ps2-link/current-source reproducibility;
- expected skipped dictionary reconciliation.

No Transport host-test retry was required for this state publication.

## Checks / evidence

REMOTE_AUTHORITY_REFRESH=PASS
TIMEOUT_RECOVERY_AUTHORITY_CHECK=PASS
A003_R3_SOURCE_REVIEW=PASS
A003_R3_C1_C12_FOREMAN_ACCEPTANCE=PASS
SMS_VENDOR_BLOB_IDENTITY=PASS
SMS_PINNED_UPSTREAM=PASS
SMS_DEDICATED_BUILD_MODEL=PASS
R3_EXACT_HEAD_CI=PASS
NEXT_OWNER_SEAM=A003_MPEG_WORKER_FRAME_RENDEZVOUS_R4
FOREMAN_STATE_REVISION=0031
FOREMAN_STATE_COMMIT=1822d286bbcf34288066750117a201078fd10bb0
FOREMAN_STATE_CI_RUN=35551363193
FOREMAN_STATE_CI_FINAL=PASS
FOREMAN_PRODUCT_BEHAVIOR_WRITE=NONE
PHYSICAL_HARDWARE_RUN=NOT_PERFORMED

## Evidence gaps

PENDING_LOCAL=NONE_FOR_FOREMAN_PACKET_PUBLICATION
PHYSICAL_PS2_SMS_DECODE=NOT_RUN_NOT_CLAIMED
PHYSICAL_IPU_DMAC_BACKEND=NOT_REQUALIFIED
MPEG_WORKER_FRAME_RENDEZVOUS=ACTIVE_RECONSTRUCTION_PACKET
A003_TO_A004_RUNTIME_CONSUMER=NOT_IMPLEMENTED
CONCRETE_FINAL_PS2_WORKER_ADAPTER=NOT_YET_REQUIRED
START_RETIRE_ORCHESTRATION=NOT_IMPLEMENTED
TRANSPORT_RESIDUAL_FINALIZATION=NOT_IMPLEMENTED
RFB_RESTORATION_ORCHESTRATION=NOT_IMPLEMENTED
FINAL_APPLICATION_ORCHESTRATION=NOT_IMPLEMENTED
HARDWARE_PENDING=YES

## Exact next pickup

Interactive Reconstruction executes
`A003-MPEG-WORKER-FRAME-RENDEZVOUS-R4`, emits exactly one immutable
`a003-mpeg-generation` Reconstruction log, and returns the baton to Foreman.

Do not begin A004 runtime consumption or final Application orchestration in the
same shift.

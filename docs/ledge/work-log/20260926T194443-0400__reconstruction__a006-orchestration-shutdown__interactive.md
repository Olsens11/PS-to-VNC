# Ledge Reconstruction work log — R41 Application AUDIO lifecycle coordinator

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-26T19:44:43-04:00
COMPLETED_AT=2026-09-26T20:04:05-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a006-orchestration-shutdown
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=e2bedc976769c3e9a755ca1bc2ab43533a832325
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Authority and packet

This round recovered Foreman State revision 0083 at
`e2bedc976769c3e9a755ca1bc2ab43533a832325` and executed only the active
`A006-AUDIO-APPLICATION-LIFECYCLE-COORDINATOR-R41` packet owned by
Reconstruction / `interactive`.

R41 was bounded to a separate Application AUDIO lifecycle coordinator. It did
not authorize or perform ordinary `src/app.c/.h` integration, ordinary
`pi/wire_runtime.py` AUDIO factory injection, selected R36 value changes,
Transport changes, AUDIO lower-owner changes, media-clock changes, MPEG/RFB
product-source changes, Input/UI/Display changes, Pi producer changes, Wire
version changes, or H1 forensic changes.

Final pre-log source authority is:

`d24226c47ff148b99c1ecfa71b9db11970918fcc`
— `test(app-audio): verify reconciled R41 source`.

That source is 16 commits ahead and zero behind the assigning Foreman head.

## New Application AUDIO lifecycle owner

R41 adds:

- `src/app_audio_product.h`
- `src/app_audio_product.c`.

The module is a cross-domain Application coordinator, not a new lower mechanism.
Its explicit lifecycle is:

- DORMANT
- STARTING
- ACTIVE
- FINITE_COMPLETE
- FAULTED
- ABORT_READY.

The coordinator stores one exact Transport access ticket, one copied selected
R36 AUDIO runtime profile, the current session common-clock pointer and R26 time
operations, R37 runtime/operation ownership, one accepted AUDIO session and the
exact post-join AUDIO outcome.

The public status surface preserves lifecycle state, lower-owner results,
ownership flags, activation-attempt/success facts, abort debt, retained-storage
proof and the exact AUDIO outcome when available.

## Inert construction

`pstvnc_app_audio_product_init()`:

1. validates the caller-owned exact Transport ticket, media clock and time ops;
2. copies the selected R36 AUDIO runtime profile;
3. validates that copied profile;
4. records those values into an otherwise inert DORMANT coordinator.

Initialization performs no:

- R37 runtime allocation;
- R40 AUDIO activation;
- AUDSRV operation;
- AUDIO worker create/start;
- media-clock arm/reset;
- MPEG presentation change.

Focused proof observes zero lower-owner start events during initialization and
the selected startup reservoir value of 458752 bytes.

## Explicit start ordering

`pstvnc_app_audio_product_start()` owns the exact cross-domain order:

1. initialize the R37 PS2 AUDIO runtime;
2. obtain the R37 memory/thread/sync operation tables;
3. record that the one-shot Transport activation edge is being attempted;
4. invoke R40 `pstvnc_transport_audio_activate()` for the stored ticket;
5. obtain the accepted AUDSRV service operation table;
6. invoke the accepted AUDIO session start with the copied R36 PCM/session
   values, the current session common clock and R26 time operations.

The focused event fixture proves:

`runtime init < runtime ops < Transport AUDIO activation < AUDIO session start`.

After the Transport activation call is attempted, the coordinator never permits
another AUDIO start in that Wire Session, even if the activation call itself
fails. A successful activation remains historically observable after later
finite AUDIO completion.

## Pre-activation failure ownership

Failure before the R40 activation edge is local Application/AUDIO ownership
debt only.

A partial R37 runtime acquisition is released locally. If that cleanup succeeds,
the coordinator returns to DORMANT and may be retried because no channel-2
activation occurred.

If local runtime cleanup itself fails, the coordinator remains FAULTED with the
original failure retained as first-failure evidence and the still-owned runtime
represented explicitly. It does not ask Transport for retained-session proof
because Transport AUDIO activation was never attempted. The local cleanup is
retryable and cannot become ABORT_READY until the exact remaining ownership is
actually released.

## Post-activation failure and one-shot policy

The coordinator publishes `transport_activation_attempted` before calling the
R40 activation API. This is deliberate: R40 may already have crossed DORMANT to
ACTIVATING and terminalized Transport even when its caller receives failure.

From that point forward:

- activation failure;
- AUDIO session start failure;
- worker completion observation failure;
- join failure;
- outcome failure;
- failed worker outcome;
- local release failure;
- Transport status failure

all become enclosing-session teardown debt.

No path resets the coordinator to DORMANT and no path sends a second AUDIO
activation in the same Wire Session.

A partial AUDIO session whose thread was created but never started remains an
owned, retryable lower-owner fact. A failed destroy/release is not converted to
ABORT_READY and does not authorize R37 runtime release.

## Nonblocking steady-state completion

ACTIVE steady-state service uses the accepted R38 completion publication.

- PENDING returns immediately.
- PENDING performs no join, outcome access or release.
- completion-observation uncertainty is failure, not progress;
- DONE is joined first;
- only after successful join is exact AUDIO outcome read;
- clean finite outcome then releases the AUDIO session and R37 runtime.

There is no timeout, delay count, poll count or Transport queue inference used
as AUDIO worker completion proof.

Clean finite outcomes are exactly:

- `PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY`;
- `PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK` with
  `PSTVNC_AUDIO_PLAYBACK_COMPLETE`.

Those outcomes transition to FINITE_COMPLETE. The exact outcome is retained,
local AUDIO session/runtime ownership is gone, and the historical successful
Transport activation fact remains true so successor ordinary integration can
still retire the enclosing Wire Session correctly.

Every other accepted AUDIO session outcome is retained as exact failure
evidence and requires enclosing-session teardown.

## First-MPEG-presentation AUDIO gate

R41 owns only the synchronization decision, not MPEG presentation itself.

`pstvnc_app_audio_product_first_presentation_ready()` uses the stored exact
Transport access and non-consuming `pstvnc_transport_audio_status()`.

READY is true only when either:

- available AUDIO bytes are at least the selected R36
  `startup_reservoir_bytes` = 458752; or
- the finite AUDIO producer is already done, including the empty case.

Otherwise READY is false.

Focused proof covers:

- 458751 live bytes => not ready;
- 458752 live bytes => ready;
- producer_done with zero bytes => ready.

The gate never calls AUDIO read/dequeue, never returns CREDIT, never waits or
delays, never infers readiness from elapsed time/poll count, never arms/resets
the common clock and never changes MPEG presentation state.

A Transport status error is a real post-activation session failure.

## Retained-session abnormal teardown fence

After any Transport AUDIO activation attempt, abnormal local cleanup may not
touch AUDIO session/runtime ownership until:

`pstvnc_transport_session_abort_storage_retained(stored_ticket)`

returns exact success for that old session ticket.

If retained proof is unavailable, the coordinator returns
`PSTVNC_APP_AUDIO_PRODUCT_RETAINED_STORAGE_REQUIRED` and performs no local
stop/join/release touch.

After proof, cleanup is retryable and ordered:

1. request AUDIO stop when a successfully started unjoined worker still owns
   execution;
2. use nonblocking R38 completion publication;
3. if PENDING, return immediately with ownership retained;
4. if DONE, join the exact worker;
5. preserve exact outcome;
6. release the AUDIO session;
7. release the R37 PS2 runtime;
8. only then publish ABORT_READY.

A never-started partial session skips fabricated stop/completion/join and uses
the lower owner's real release/destroy path. Failure there remains explicit
retryable ownership.

The focused fixture proves retained Transport proof occurs before any local
post-activation stop/reclaim, and proves no timeout/poll-count path can
manufacture ABORT_READY.

## Regression-boundary maintenance

Two accepted historical source tests originally implemented their
"ordinary Application has not consumed AUDIO yet" assertions by concatenating
every `src/app*.c` file.

R41 deliberately introduces a separate Application coordinator while ordinary
`src/app.c` remains deferred. Those R37/R38 tests were therefore narrowed to
inspect `src/app.c` specifically:

- `tests/unit/audio_ps2_runtime_source_test.py`;
- `tests/unit/audio_session_completion_source_test.py`.

No R37/R38 product source changed. Both historical source-boundary tests pass
on the final R41 head and continue to prove no ordinary `app.c` AUDIO
composition.

## Build, topology and documentation

The new root Application module is enrolled in:

- `tests/Makefile`;
- `scripts/check-clean-ps2-compile.sh`;
- `mk/issue7-clean.mk`;
- the root-source topology allowlist.

Living ownership documentation was updated in:

- `docs/development/module-lifecycle.md`;
- `docs/development/source-topology.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`.

Deterministic dictionary reconciliation updated only:

- `src/SYMBOLS.md`;
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`.

The root Application dictionary now contains 1109 symbols.

## Commits

R41 commits, oldest first:

- `814f72e5682e0e20b6194840f4e53eb5a6ae8e40`
  — define R41 coordinator interface.
- `f75c55a7597ab4ca6cc0a4804f04bd1f0781789f`
  — implement R41 coordinator.
- `5c39d558e10867ba83e01fe77e2fe471b293961a`
  — add focused lifecycle ownership proof.
- `ae1fa6a2b7b743f87cc4f1e148b4a16d9ff1e286`
  — add focused source-boundary proof.
- `35beaa4f8504e40b27ff9189dd39f9cefa615cd5`
  — enroll focused R41 tests.
- `dd8392f52a2e74b135b7a59ef8684fc57b5f9f0d`
  — enroll coordinator in PS2 compile check.
- `a6eaf7e8939854f1881fbebb291e6149768ada56`
  — link coordinator into the clean Issue #7 image.
- `59a10d45c6a303cec0b327c347d9fe911d75abf5`
  — admit root module into topology contract.
- `7930954f4dc29d151e3d73bfd52e660fb2437d8b`
  — narrow R38 ordinary-app regression boundary.
- `3441b93db849c13ee2f50d7632f8755efccd6e49`
  — record R41 root ownership.
- `8fbc4e3456214284b5baa1a9f116496bc290d5b1`
  — record R41 lifecycle.
- `3d3053336c4062a2ed6406d79e9ae2ac2fbcbef8`
  — update living file/service map.
- `08358cec85ee4d94dacbe74f2c281a57bc3797a3`
  — narrow R37 ordinary-app regression boundary.
- `f9fe75ea923be86d0d5e108c1593f7ab6d2daf31`
  — deterministic dictionary reconciliation trigger.
- `ec68825322a0423be81f7cc7794bc1fd3f1ad1e6`
  — automation-generated clean dictionary reconciliation.
- `d24226c47ff148b99c1ecfa71b9db11970918fcc`
  — exact reconciled-source verification head.

Changed paths versus the assigning Foreman head are exactly:

- `src/app_audio_product.c`
- `src/app_audio_product.h`
- `tests/unit/app_audio_product_test.c`
- `tests/unit/app_audio_product_source_test.py`
- `tests/unit/audio_ps2_runtime_source_test.py`
- `tests/unit/audio_session_completion_source_test.py`
- `tests/Makefile`
- `scripts/check-clean-ps2-compile.sh`
- `mk/issue7-clean.mk`
- `scripts/continuity-check.sh`
- `src/SYMBOLS.md`
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md`
- `docs/development/module-lifecycle.md`
- `docs/development/source-topology.md`
- `docs/reference/FILE_AND_SERVICE_MAP.md`.

No ordinary `src/app.c/.h`, Pi product source, lower AUDIO product source,
Transport source, selected R36 profile, media clock, MPEG/RFB product source,
Input/UI/Display product source, Wire version, or H1 source changed.

## Exact final evidence

Canonical exact-source GitHub Actions run:

`36281260844`

checked out exact head
`d24226c47ff148b99c1ecfa71b9db11970918fcc`, attempt 1, and completed
SUCCESS.

Canonical disposition:

- host-unit = SUCCESS
- project-check = SUCCESS
- dictionary-long = SUCCESS
- ps2-compile = SUCCESS
- ps2-link = SUCCESS
- dictionary-reconcile = correctly SKIPPED.

Selected exact-head evidence includes:

- `APP_AUDIO_PRODUCT_TEST=PASS`
- `APP_AUDIO_PRODUCT_SOURCE_TEST=PASS`
- `AUDIO_SESSION_COMPLETION_SOURCE_TEST=PASS`
- `audio_ps2_runtime_test: PASS`
- `AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS`
- `audio_session_test: PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS`
- `CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS`
- `TRANSPORT_AUDIO_ACTIVATION_SOURCE_TEST=PASS`
- `transport_audio_test: PASS`
- `transport_mpeg_test: PASS`
- `PI_AUDIO_PCM_PRODUCER_TEST=PASS`
- `PI_AUDIO_WIRE_SERVER_TEST=PASS`
- `media_clock_test: PASS`
- current MPEG Application/calibration/activation/frame regressions = PASS
- current RFB/Transport regressions = PASS
- `SOURCE_TOPOLOGY_LOCAL_FILE_COVERAGE=PASS`
- `SOURCE_TOPOLOGY_CONTRACT=PASS`
- `SOURCE_DICTIONARY_PORTAL_SYNC=PASS`
- `CLEAN_PRODUCT_SOURCE_TOPOLOGY=PASS`
- `DEVELOPMENT_CONTINUITY_CHECK=PASS`
- `WORK_LOG_CHECK=PASS`
- `SOURCE_DICTIONARIES=PASS`
- `PS_TO_VNC_PROJECT_CHECK=PASS`
- `ISSUE7_LINKED_BUILD=PASS`
- `LEDGE_CURRENT_LINKED_REPRODUCIBILITY=PASS`.

## Linked identity and hardware boundary

Accepted R40 PT_LOAD identity:

`PT_LOAD_SHA256=7216b06d319ce6cb3e81085427ef7696bf36a0361c46c05a1cd7fe8bc1baebff`

`PT_LOAD_BYTES=551316`.

R41 exact final-source identity:

`ELF_PRISTINE_SHA256=306ccd622c1a1c00bdb0ff24208c64b7c1832c8caa50d96797e752ad9f9ad997`

`PT_LOAD_SEGMENTS=1`

`PT_LOAD_SHA256=dd7d5f7bca743d3a505c809c53f78474559105244d91b29f62c5d83ec2bb4dc0`

`PT_LOAD_BYTES=555284`.

Therefore:

- R41_PS2_PT_LOAD_CHANGED=YES
- R41_HARDWARE_PENDING=YES
- R41_OPERATOR_OBSERVED=NO
- R41_HARDWARE_QUALIFIED=NO
- R41_INDEPENDENT_VALIDATION=NOT_RUN.

No physical behavior claim is made by this Reconstruction Worker.

## Worker criterion disposition

These are Worker findings, not Foreman acceptance.

1. Separate explicit Application AUDIO lifecycle coordinator with required
   states and owner facts — MET.
2. Inert init copies exact ticket/R36 profile/current clock/time ops with no
   activation or runtime side effect — MET.
3. Exact R37 runtime/ops -> R40 activation -> AUDIO session start ordering —
   MET.
4. Pre-activation failures clean locally or preserve unreleased local ownership
   without Transport-abort debt — MET.
5. From activation attempt onward, failure requires retained-session teardown
   and no same-session AUDIO restart/reactivation is possible — MET.
6. ACTIVE service uses R38 poll; PENDING is immediate; DONE joins before
   outcome/release; uncertainty is failure — MET.
7. EMPTY and playback COMPLETE become clean FINITE_COMPLETE; other outcomes are
   preserved failure evidence — MET.
8. First-presentation readiness is exact non-consuming 458752-byte reservoir or
   producer_done and has no time/count inference — MET.
9. Coordinator never arms/resets common clock and never modifies MPEG
   presentation — MET.
10. Abnormal post-activation local teardown is fenced by exact retained-ticket
    proof and then uses stop/poll/join/outcome/session-release/runtime-release
    ownership order — MET.
11. Partial never-started/destroy-failure ownership remains explicit and
    retryable; no fabricated ABORT_READY — MET.
12. Ordinary `app.c`/Pi AUDIO composition, selected R36 values, Transport,
    lower AUDIO owners, R26, MPEG/RFB/Input/UI/Display product source and Wire
    version remain unchanged; canonical gates are green — MET.

Evidence classification:

- R41_SOURCE_COMPLETE=YES_WITHIN_PACKET
- R41_HOST_TESTED=PASS
- R41_PROJECT_CHECK=PASS
- R41_STRICT_DICTIONARIES=PASS
- R41_PS2_COMPILE=PASS
- R41_PS2_LINK=PASS
- R41_CURRENT_SOURCE_REPRODUCIBILITY=PASS
- R41_MACHINE_EVIDENCE=GITHUB_ACTIONS
- R41_INDEPENDENT_VALIDATION=NOT_RUN
- R41_OPERATOR_OBSERVED=NO
- R41_HARDWARE_QUALIFIED=NO
- R41_HARDWARE_PENDING=YES

## Next pickup

There is no Reconstruction blocker at this stopping point.

The Interactive Reconstruction Worker stops here. The Foreman must independently
inspect the R41 source range, start ordering, first-presentation reservoir gate,
finite-completion policy, retained-session teardown fence, build/topology
enrollment, exact machine evidence, changed PT_LOAD identity and this immutable
closeout before accepting or rejecting R41 and selecting any successor packet.

NEXT_PICKUP=FOREMAN_INDEPENDENT_R41_REVIEW_ACCEPTANCE_AND_NEXT_PACKET_SELECTION

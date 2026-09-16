# Ledge Reconstruction Shift — A002 common media clock

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T10:01:12-04:00
COMPLETED_AT=2026-09-16T10:16:00-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a002-audio-clock
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=a0d09656306e8e17ec3b67d777536bf39241134f
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO
FOREMAN_GOAL_RESULT=MET

## Objective and authority consumed

Executed only the active A002 common-media-clock M1-M8 packet issued by `docs/ledge/LEDGE_FOREMAN_STATE.md` revision `0008` as one sustained interactive Reconstruction round.

Consumed current `ledge/h1-all-guns` authority, global work-state revision `0025`, Validation state `0006` / findings `0005` for the still-current A001 machine/source PASS boundary, reconstruction contract `0005`, work-log contract `0003`, `LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision `0001`, current clean source naming/topology policy, the completed A002 CONFIG/profile boundary, and pinned H1 media-clock evidence at `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`.

The round started from branch HEAD `a0d09656306e8e17ec3b67d777536bf39241134f`, the immutable Foreman CONFIG-integration/media-clock baton log. Branch authority was re-read before source mutation, during verification, and immediately before this immutable log create. No concurrent branch movement was observed through the final pre-log read at `4b13b4c658b604061c268c4e975e300da7fb7e56`.

The previously discovered malformed immutable A002 Reconstruction log filename/`STARTED_AT` mismatch was explicitly left untouched. Foreman revision `0008` assigns that historical-record policy defect outside Reconstruction behavior ownership.

## Reconstruction performed

Created a new clean `src/media/` responsibility because the shared session media epoch is neither configuration ownership nor a PS2 platform mechanism. The new owner consumes only the existing immutable `pstvnc_config_media_clock_profile_t` and does not pull CONFIG wire representation, application coordination, Transport ownership, PCM playback, or video presentation policy into timing state.

### Common clock owner

`src/media/clock.h` / `src/media/clock.c` now provide:

- one session-scoped clock state with one common `epoch_tick` and one `armed` state;
- immutable post-init copies of the A002 media-clock profile, synchronization contract, and timer tick rate;
- required acquire/release callbacks protecting all mutable epoch/armed publication and observation;
- one-shot/idempotent arm behavior: the first arm publishes `epoch_tick` before `armed` while the same lock is held, and later arm attempts return success without moving the epoch;
- synchronized `is_armed` and epoch observation APIs so production consumers do not read mutable state directly;
- one shared epoch feeding independent audio and neutral-video deadlines through the exact signed `int32_t` CONFIG offsets;
- signed offset conversion in the configured tick domain without reinterpretation as unsigned policy;
- negative-offset underflow saturation to zero;
- positive-offset overflow saturation to `UINT64_MAX`;
- additional tick-delay saturation to `UINT64_MAX`;
- epoch-lead addition saturation to `UINT64_MAX`;
- unarmed epoch/deadline results that fail closed with `PSTVNC_MEDIA_CLOCK_UNARMED` rather than fabricating a usable timestamp;
- injected timer/delay operations for deterministic waiting without embedding PS2 timer/DelayThread mechanisms in the common owner;
- an optional stop observer and explicit stop/timer/delay/synchronization failure results;
- deliberate zero-poll rejection as `PSTVNC_MEDIA_CLOCK_INVALID`, preventing an accidental unbounded busy-spin path rather than silently turning zero into a spin cadence.

The concrete PS2 synchronization/timer/delay binding is intentionally not selected in this packet. The Foreman blocker rule explicitly permits that binding to remain pending when the narrow synchronization/platform contract and deterministic semantics are established without prematurely wiring runtime media consumers.

### Deterministic behavior fixture

Added `tests/unit/media_clock_test.c`. Its synchronization and time fakes exercise the production `src/media/clock.c` implementation and prove:

- initialized/unarmed state;
- unavailable epoch/deadline before arm;
- one successful arm plus idempotent repeated arm;
- deterministic publication witness at release showing that `armed` is never published without the matching epoch in the protected transition;
- one shared epoch producing independent audio/video deadlines;
- ordinary positive and negative offsets;
- exact `INT32_MIN` and `INT32_MAX` interpretation;
- negative underflow saturation to zero;
- positive-offset overflow saturation to `UINT64_MAX`;
- additional-tick overflow saturation to `UINT64_MAX`;
- epoch-lead overflow saturation;
- wait remaining non-ready while unarmed and then reaching only the armed deadline;
- stop-before-ready behavior;
- timer-read failure propagation;
- delay failure propagation;
- zero-poll rejection without timer reads or delays;
- synchronization acquisition failure propagation.

### Clean-source topology adoption

Created `src/media/SYMBOLS.md` with `GENERATION=CLEAN_RECONSTRUCTION` and `COVERAGE=COMPLETE`, covering the definitions introduced in `clock.c` and `clock.h`.

Updated the source-side living topology authorities:

- `docs/development/source-topology.md` now records `src/media/` as the common session timing owner and explicitly excludes PCM/AUDSRV runtime, MPEG/video callsites, and concrete PS2 lock/timer bindings;
- `docs/reference/FILE_AND_SERVICE_MAP.md` now maps the same responsibility.

Per the Foreman packet's lane split, generated dictionary-portal reconciliation, canonical unit/build registration, source-topology checker integration, and linked-build evidence were not performed from Reconstruction; those remain Foreman integration/evidence chores.

## Exact Reconstruction commits

- `9f13f5875dcba5a679126961b0840b3b1fa7157e` — `reconstruct(a002): define common media clock boundary`
- `dfc211a217687ff0d79869511c62d6ca935610aa` — `reconstruct(a002): implement common media clock semantics`
- `8e132c46aa3842cdbfe6ae35f700e6eaedb5cc76` — `test(a002): add deterministic common media clock fixture`
- `636990a432d5e133ef441d55d53bf38a2fc7be75` — `docs(a002): inventory common media clock symbols`
- `8edf4e7e8a1a301eb8169a6c34f2a4f9d1350efa` — `docs(a002): adopt common media timing domain`
- `1fe64b7ccc85d51f10e3fc7968b208a9649bb38e` — `docs(a002): map common media timing ownership`
- `4b13b4c658b604061c268c4e975e300da7fb7e56` — `docs(a002): clarify media clock immutability contract`

The round diff from `a0d09656306e8e17ec3b67d777536bf39241134f` through `4b13b4c658b604061c268c4e975e300da7fb7e56` contains only:

- `src/media/clock.h`;
- `src/media/clock.c`;
- `src/media/SYMBOLS.md`;
- `tests/unit/media_clock_test.c`;
- `docs/development/source-topology.md`;
- `docs/reference/FILE_AND_SERVICE_MAP.md`.

No application, Transport/RFB runtime, CONFIG decoder, audio/AUDSRV runtime, or video implementation file changed.

## Checks and evidence

Direct strict host execution used the exact final Git blobs for the behavior-bearing clock/test files:

- `src/media/clock.c` Git blob `f8139957b4d33e6226f434069b8e1bacbd067a30`;
- final `src/media/clock.h` Git blob `3a572decd4604d818b442b77fc0a59f5fbadc1f0`;
- `tests/unit/media_clock_test.c` Git blob `67e129ed412c28eaff5855015f79ca3241829f5e`.

Strict host command profile:

`cc -O2 -std=c99 -Wall -Wextra -Werror -pedantic -Isrc tests/unit/media_clock_test.c src/media/clock.c -o media_clock_test_final`

Execution result:

`media_clock_test: PASS`

This is direct Reconstruction behavior evidence, not a claim that the new fixture is already part of canonical `make -C tests unit`.

PENDING_LOCAL=Foreman-owned canonical test registration; clean linked-build/include integration; `src/media` continuity/checker integration; generated dictionary reconciliation/portal update; canonical host/project/dictionary/PS2 compile-link/reproducibility execution; concrete PS2 synchronization/timer/delay binding.

The repository's already-known `scripts/check.sh` red state caused solely by the earlier immutable A002 work-log filename/`STARTED_AT` mismatch was not repaired or reclassified here.

HARDWARE_PENDING=A001 physical PS2 qualification remains pending. No A002 physical qualification is claimed by this round.

## M1-M8 criterion disposition

M1 `MET` — one clean session-scoped common-media-clock owner exists in `src/media/`; it copies only the narrow immutable media-clock subprofile and owns no cross-domain mutable global/config state.

M2 `MET` — arm is one-shot/idempotent and protected by an explicit acquire/release synchronization contract; epoch is written before armed under the same lock, and readers take the same synchronization path. The deterministic release witness proves the protected publication transition exposes the matching epoch and armed state together.

M3 `MET` — audio and neutral-video deadlines derive from one shared epoch, preserve exact signed CONFIG offsets, saturate negative underflow/positive overflow/additional overflow, and reject unarmed deadline requests.

M4 `MET` — timing state/math is platform-neutral and timer/delay/synchronization mechanisms are narrow injected contracts. The exact PS2 binding is deliberately deferred rather than copied from H1's `volatile`/`EE_SYNCL()` representation.

M5 `MET` — waits are arm/deadline gated, stop-aware, timer/delay/synchronization-failure aware, and reject zero polling cadence before any loop can spin.

M6 `MET` — deterministic host test source covers the required state, publication, shared-deadline, signed-boundary, saturation, stop, timer/delay failure, synchronization failure, and zero-poll behaviors; direct strict execution is PASS.

M7 `MET` — maintained C/H synopses, readable ownership comments, complete local `src/media/SYMBOLS.md`, topology policy, and living ownership map were added. Foreman-owned generated/canonical integration was deliberately not taken over.

M8 `MET` — A001 Transport/RFB and completed A002 CONFIG/profile behavior remain unchanged. No fixed timing fallback/default was introduced, no PCM/AUDSRV runtime was begun, no MPEG/video presentation callsite or policy was selected, and no inherited receive-poison or hardware behavior was silently changed.

FOREMAN_GOAL_RESULT=MET

## Exact next pickup

FOREMAN: independently inspect the M1-M8 commits/source/tests rather than relying on this prose; register the new host fixture and clean media-clock source in canonical test/build paths; integrate `src/media` into topology/checker and deterministic dictionary/portal surfaces; run canonical host/check/dictionary/PS2 linked/reproducibility evidence; decide the concrete PS2 synchronization/timer binding integration and the next dependency-ordered A002 packet. Preserve the separate historical work-log policy defect and A001 physical `HARDWARE_PENDING` status.

VALIDATION: no independent A002 PASS is claimed by Reconstruction. Judge the coherent A002 tranche only when handed the settled evidence through the normal lane.

RECONSTRUCTION: do not begin PCM/AUDSRV runtime or A003 behavior without a fresh bounded Foreman packet.

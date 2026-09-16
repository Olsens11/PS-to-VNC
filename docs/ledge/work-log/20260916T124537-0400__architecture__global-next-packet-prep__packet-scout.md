DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T12:45:37-04:00
COMPLETED_AT=2026-09-16T12:46:34-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=global-next-packet-prep
WORKER_KEY=packet-scout
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=2aef278925dca4ae3e36c15e721b39bf1f874314
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Next-Packet / H1 Archaeology Scout

## Objective and authority consumed

Prepare one dependency step beyond the active interactive Reconstruction packet without issuing Foreman authority or competing with its writes.

Authority/evidence consumed:

- `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0010: active phase `A002_PCM_PLAYBACK_CORE_PACKET_ISSUED`; the Transport AUDIO A1-A8 packet is Foreman-accepted MET and the active packet deliberately stops before playback-worker allocation/thread lifecycle and startup-reservoir/common-clock gating.
- `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` revision 0001.
- `docs/ledge/LEDGE_VALIDATION_STATE.md` revision 0006; A001 machine/source PASS remains independent authority and physical qualification remains pending.
- `docs/ledge/work-log/README.md` revision 0005.
- forensic H1 authority `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`, especially `experiments/media-harness-h1/h1_audio_runtime.{c,h}` and `h1_config.h`.
- current clean `src/media/clock.h` and the newly arriving `src/audio/playback.h` / `src/audio/audsrv_service.h` source.

The branch was actively moving during this scout pass. Starting authority was `2aef278925dca4ae3e36c15e721b39bf1f874314`; while evidence was being read, Reconstruction landed synchronous PCM/AUDSRV source including commits `103791438a4afae42edd67224466f0767a37b790`, `e1d6a8f60e54cf933d2b326182e184605850cb5f`, `ad36d3a88da9c3a0bc4520975ba6f8c6219afbf2`, and subsequent host seam work. Immediately before this log write live authority was refreshed at `2ce299ca6af66b0cb6c1accb69137a705949915d`. Therefore this dossier is intentionally conditional and must not be treated as review/acceptance of the moving PCM packet.

## Conditional next responsibility

`NON_AUTHORITATIVE_PENDING_FOREMAN`

If the interactive Foreman accepts the current synchronous PCM/AUDSRV packet, the next coherent A002 responsibility is **session-scoped audio worker lifecycle plus startup-reservoir/common-clock presentation gating**, built around the accepted synchronous playback core rather than reimplementing its Transport/AUDSRV semantics.

This is the natural next dependency because A002 still requires: (1) session worker resources that are never reclaimed live, (2) pre-presentation reservoir readiness, and (3) gating first presentation on the already-clean shared media epoch plus signed audio offset. H1 proves these mechanisms in one audio runtime, while Foreman revision 0010 explicitly deferred them from the current synchronous core.

## Historical H1 call chain and semantic evidence

Primary historical chain: `pstvnc_h1_audio_start()` -> allocate PCM buffer + aligned thread stack -> `CreateThread()` -> `StartThread()` -> `h1_audio_thread()` -> AUDSRV setup -> `h1_audio_wait_start_policy()` -> `pstvnc_h1_media_clock_wait_offset()` -> Transport AUDIO read loop -> AUDSRV wait/play -> `audsrv_stop_audio()` -> set `finished` -> `ExitThread()`. Shutdown is `pstvnc_h1_audio_shutdown()` -> set `stop_requested` -> observe `ReferThreadStatus()` until dormant -> `DeleteThread()` -> only then free stack/buffer.

Semantics that survive reconstruction:

- audio buffer and worker stack are session-scoped resources;
- reclamation occurs only after worker completion/dormancy is proven;
- startup reservoir readiness is distinct from presentation permission;
- target-reservoir mode waits until queued AUDIO bytes reach the configured target; immediate/delay were H1 laboratory policy alternatives, not automatically production requirements;
- after reservoir policy, audio waits on the shared media epoch with the signed audio offset before first presentation;
- stop must cancel reservoir/clock waiting and playback progression deterministically;
- resident AUDSRV/LIBSD lifetime remains outside session worker reclamation; session completion stops/mutes current audio but does not quit the service;
- first-error semantics survive failure/cleanup convergence.

Historical mechanism that should NOT be copied as product authority:

- H1's 3-second `ReferThreadStatus` polling budget (`1000us * 3000`) is defensive mechanism, not the semantic requirement;
- `audio_idle_delay_us` being reused for both reservoir and media-clock polling is experimental coupling;
- the H1 dynamic `audio_thread_priority`, `audio_thread_stack_size`, `audio_chunk_bytes`, start-mode/delay, and other 61-field laboratory knobs are qualified/tuning candidates, not permission to expose another giant production CONFIG object;
- direct inspection of `runtime->transport->end_received` and Transport queue internals must not return; clean audio must use public Transport bridge status/activity authority;
- H1 audio diagnostic stage writes into Transport are diagnostic-only and must not become correctness dependencies.

## Current clean receiving owners / seams

- `src/audio/playback.h`: current moving source defines a synchronous PCM playback owner with immutable `pstvnc_config_pcm_profile_t`, injected `pstvnc_audio_service_ops_t`, caller-owned buffer/capacity, explicit playback result vocabulary, and report accounting. Its synopsis explicitly excludes playback thread, chunk-size policy, startup reservoir, common-clock timing, application orchestration, and service quit. This is the correct core for the next lifecycle owner to call, subject to Foreman acceptance of the completed packet.
- `src/audio/audsrv_service.h`: declares the concrete resident AUDSRV adapter and intentionally exposes no per-session quit operation.
- `src/media/clock.h`: already owns the session common epoch, synchronization-safe publication, audio deadline/wait, signed profile offset, time operations, and stop observer. The next audio lifecycle should consume this API rather than create a second clock or copy H1 volatile fields.
- Transport already owns AUDIO bytes/activity/terminal state from the prior accepted packet. Audio lifecycle must remain a consumer, not a queue/credit owner.

## Likely packet shape

A substantial next packet can coherently include:

1. an audio session/runtime owner that allocates the caller-selected playback buffer and PS2 worker resources, starts exactly one playback worker, publishes completion/result, and refuses/freezes reclamation until completion is proven;
2. an explicit stop/cancellation path shared by reservoir wait, clock wait, and playback progression;
3. a startup policy owner that supports the production-selected reservoir requirement without importing H1's full start-mode/delay vocabulary by default;
4. first-presentation gating through `pstvnc_media_clock_wait_audio()` after reservoir readiness and before the first AUDSRV submission;
5. deterministic host lifecycle tests using injected thread/allocation/time/Transport/service seams where possible, including allocation/start failure, stop during reservoir wait, stop during clock wait, normal finite completion, playback failure, cleanup failure, and proof that stack/buffer reclaim occurs only after worker completion;
6. PS2-specific worker adapter/build evidence as a separate concrete boundary, with physical repeated-session/endurance qualification remaining hardware pending.

A useful stretch target, only if the lifecycle packet settles early, is narrow production binding of the audio worker to the already-qualified owner profile/resource policy. Do not stretch into MPEG/presentation or top-level all-guns orchestration.

## Host-testable versus PS2/hardware evidence

Host-testable:

- lifecycle state machine and first-error preservation;
- allocation/start/delete ordering through injected worker operations;
- no-free-before-completion invariant;
- reservoir-ready policy over a fake Transport AUDIO status/occupancy observer;
- cancellation during reservoir and clock waits;
- ordering `reservoir ready -> clock deadline satisfied -> first playback submission`;
- normal finite Transport exhaustion and error convergence;
- no per-session AUDSRV quit reachability.

PS2 compile/link evidence:

- EE thread API adapter, aligned stack handling, concrete AUDSRV adapter and clean link composition.

Hardware-only / qualification:

- actual EE scheduling/resource behavior for selected stack/priority/chunk values;
- repeated-session AUDSRV service survival;
- real reservoir/epoch timing and absence of premature audio presentation;
- stress proof that worker shutdown/reclaim is reliable under actual PS2 blocking/scheduling behavior.

## Foreman decisions that remain open

1. **Production worker/resource values:** A002 audit classifies H1 chunk size, worker priority and stack as qualified-profile/tuning candidates. The Foreman must identify existing qualified profile authority or deliberately defer concrete PS2 worker binding; the scout must not invent defaults.
2. **Reservoir policy representation:** decide whether production needs only one qualified target-reservoir value or a narrow mode representation. Do not automatically preserve H1 IMMEDIATE/TARGET/DELAY as public API.
3. **Buffer ownership seam:** the moving synchronous playback core accepts caller buffer/capacity. The lifecycle owner should probably own that session buffer, but the exact allocation/profile authority must be explicit.
4. **Clock polling cadence:** `pstvnc_media_clock_wait_audio()` requires `poll_us`; H1 reused `audio_idle_delay_us`, but the audit says that coupling is experimental convenience. A qualified narrow timing value or a better event-backed mechanism needs explicit authority.
5. **Completion proof on PS2:** semantic requirement is worker quiescence before reclamation; the Foreman should choose the clean completion mechanism rather than preserving H1's arbitrary 3-second polling loop.

## Dependency/order hazards

- Do not let the lifecycle wrapper consume AUDIO bytes merely to test reservoir readiness; readiness observation and actual dequeue/playback must remain distinct or Transport credit/accounting semantics can change.
- Do not arm the media clock from audio. A002 says the qualified video presentation boundary arms the epoch; audio only waits on its deadline. Until A004 supplies that callsite, host tests should inject/pre-arm the clock rather than inventing an audio arm path.
- Do not start audio presentation before the clock deadline simply because reservoir target is met.
- Stop/cancellation must wake/retire all waits without freeing a live worker.
- Do not couple correctness to H1 diagnostic stage markers.
- Do not let this packet drift into A003 MPEG lifecycle merely because common-clock/video authority is adjacent.

## Exact Foreman consumption point

Consume this dossier only after the interactive PCM/AUDSRV worker has produced its immutable handoff and the Foreman has independently accepted or corrected that packet. Re-read the then-current `src/audio/**`, CONFIG profile, Transport AUDIO bridge, media clock, current A002 audit/state, and any newer Validation/Continuity evidence. If the synchronous core's accepted interface differs from the moving source observed here, reformulate the lifecycle packet against accepted authority rather than forcing this draft.

PENDING_LOCAL=independent Foreman review/integration of the moving synchronous PCM/AUDSRV packet; exact qualified production worker/chunk/stack/priority/reservoir/poll values; canonical host/PS2 evidence for newly arriving audio source
HARDWARE_PENDING=A001 physical qualification remains pending; A002 repeated-session AUDSRV/worker/timing qualification remains future hardware evidence

## Exact next pickup

On the next scout wake, first determine whether Foreman accepted the synchronous PCM packet and whether it issued the worker-lifecycle/reservoir/clock packet. If yes, move one dependency step beyond that active packet rather than repeating this dossier. If not, refresh only changed evidence and avoid competing with the active worker.
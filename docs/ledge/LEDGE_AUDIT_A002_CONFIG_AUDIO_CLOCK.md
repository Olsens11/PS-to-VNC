# Ledge Semantic Audit A002 — CONFIG, PCM Audio, Shared Media Clock

DOCUMENT=LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T11:05:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_AUDIT_STATE_REVISION=0001
TEMPORAL_CLASS=AUDIT_TRANCHE
TEMPORAL_SEMANTICS=TRUE_AS_KNOWN_AT_RECORDED_TIME

This tranche classifies the H1 CONFIG/profile mechanism, PCM consumer/AUDSRV lifecycle, and shared media epoch. It does not authorize MPEG/presentation reconstruction; fields whose semantics depend on those unaudited domains remain profile candidates rather than product requirements.

## Evidence inspected

- forensic H1 commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- `experiments/media-harness-h1/h1_config.{c,h}`;
- `experiments/media-harness-h1/h1_audio_runtime.{c,h}`;
- `experiments/media-harness-h1/h1_media_clock.{c,h}`;
- `experiments/media-harness-h1/H1_INTEGRATION_INTENT.md`;
- ledge semantic audit revision 0002 and simplification register revision 0002;
- global continuity revision 0003, which directs A002 here.

## Required behavior

1. A connection/session receives one complete, versioned profile whose fields are decoded exactly once, duplicate/missing/unknown fields are rejected, and structural/API-impossible combinations are rejected before runtime allocation or media start.
2. Production must not inherit H1's 61-field laboratory vocabulary as an undifferentiated public configuration API. H1 intentionally exposes queue sizes, thread details, delays and diagnostic switches for sweeps; production should consume selected qualified values through narrow owner-specific profile structures.
3. PCM playback consumes bytes from the transport-owned AUDIO logical channel, preserves the proven `audsrv_wait_audio(bytes)` -> `audsrv_play_audio(bytes)` ordering, and records consumption only after successful playback submission.
4. PCM startup may prefill its reservoir before presentation synchronization. Presentation is then gated by the shared media epoch plus the signed audio offset.
5. `MEDIA_END` plus an empty AUDIO queue is normal finite-producer completion; it must retire the consumer without requiring an error or external kill.
6. AUDSRV/LIBSD module/service lifetime is resident-ELF scoped. Session shutdown stops/mutes the current stream but does not call `audsrv_quit()` and attempt unreliable per-session reinitialization.
7. Audio worker/session resources are session-scoped and must be reclaimed only after worker completion. Failure paths preserve first-error semantics and do not free a live worker's stack/buffer.
8. One session-scoped media epoch provides a common timing origin. In H1 the qualified video presentation boundary arms that epoch; audio can prefill before arming but cannot present before its deadline. Signed per-stream offsets are relative to that common origin.
9. The epoch publication ordering (`epoch_tick` before `armed`) is semantic. The historical volatile fields plus `EE_SYNCL()` are one implementation; reconstruction needs an explicit synchronization-safe publication mechanism appropriate to the clean owner.

## CONFIG v4 field classification

Classification vocabulary here is about production disposition, not wire compatibility with the forensic harness.

### Product-required semantic fields

These represent real session/product facts, though their clean representation may differ from H1's wire fields:

- 1 `SESSION_ID` — session identity/correlation;
- 2 `AUDIO_MODE` — feature activation; only OFF/PCM is implemented here;
- 3 `VIDEO_MODE` — feature activation, but detailed MPEG semantics remain for later audit;
- 21 `AUDIO_RATE`, 22 `AUDIO_CHANNELS`, 23 `AUDIO_BITS` — PCM format;
- 24 `AUDIO_VOLUME` — playback policy/user/profile value;
- 25 `AUDIO_PRESENTATION_OFFSET_US` — stream synchronization/profile value;
- 34 `VIDEO_PRESENTATION_OFFSET_US` — common-clock counterpart, subject to video audit;
- 54 `RFB_MODE` — feature activation; A001 owns its transport/session semantics.

### Qualified-profile / tuning candidates

These are legitimate mechanisms or resource/scheduling values but H1 exposes them primarily to discover a known-good profile. Production should normally consume selected defaults or mode profiles, not arbitrary runtime knobs:

- 4-13 AUDIO/MPEG queue and credit capacities/policies;
- 14-18 audio start reservoir/delay/chunk/idle policy;
- 19-20 audio worker priority/stack;
- 26-30 MPEG prefill/feed timing and size policy;
- 31-33 video rate/scheduler mode;
- 35-41 video pixel/maximum/draw geometry;
- 45-46 video drop policy/threshold;
- 47-48 sole receiver priority/stack;
- 49 max physical DATA payload;
- 50-51 socket receive/send buffer sizes;
- 52 queue allocation order;
- 53 media epoch lead;
- 55-56 video encode geometry;
- 57-61 RFB queue/credit capacities/policies.

These remain semantically owned by their respective clean components. This audit does not promote MPEG/video tuning values before those domains are audited.

### Diagnostic-only fields

- 42 `VIDEO_STAGE_MARKERS`;
- 43 `VIDEO_STAGE_HOLD_VSYNCS`.

These exist to expose/hold experimental presentation stages. They must not become production behavior authority. Retain only in development/qualification tooling if later presentation audit still needs them.

### Experimental recovery/tuning candidate requiring later video audit

- 44 `VIDEO_IPU_RESET_EACH_SESSION`.

This may encode a real decoder lifecycle requirement or merely an experimental recovery switch. It is explicitly **not** classified as discard or production-required until MPEG lifecycle audit proves which.

### Reserved vocabulary

The audio-mode enum reserves MP2-separate and MPEG-PS values that H1 does not implement. Those numeric reservations are historical protocol vocabulary, not current product behavior. Do not reconstruct inactive architectures merely to preserve enum numbering.

### Discard-before-production API surface

The **requirement that all 61 fields be supplied on every session** is laboratory scaffolding, not product semantics. Likewise, `profile_id` is useful as qualification identity but need not remain a giant dynamic runtime wire contract. Preserve H1 tooling/profile records as development evidence; production should expose only real user/product configuration and consume qualified internal profiles for implementation tuning.

## A002 responsibility disposition

| Responsibility | Historical mechanism | Disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Pipeline status |
| --- | --- | --- | --- | --- | --- | --- |
| CONFIG decode/completeness/structural validation | `h1_config.*`, complete 61-field v4 payload | `SPLIT` + `SIMPLIFY` | configuration owns product/profile values; transport/session bridge accepts negotiated profile; each media owner validates its own domain constraints | do not weaken rejection of contradictory/unrepresentable settings | pure host decode/duplicate/missing/range tests; owner-profile validation tests | `RECONSTRUCTION_READY` for profile mechanism, not all MPEG field semantics |
| PCM logical-channel consumption | `h1_audio_runtime.*` reading transport AUDIO queue | `KEEP_BEHAVIOR` | audio component; audio bridge process `session transport consumption` | preserve finite MEDIA_END+empty completion | fake transport queue tests including empty/end/error/partial chunk | `RECONSTRUCTION_READY` |
| AUDSRV playback ordering and resident service lifetime | `audsrv_init/set_format/set_volume`, wait->play, per-session `audsrv_stop_audio`, no `audsrv_quit()` | `KEEP_BEHAVIOR` | audio component lifecycle | do not reintroduce per-session `audsrv_quit()` unless separately proven | mocked host ordering where possible; repeated-session PS2 qualification | `RECONSTRUCTION_READY` |
| audio worker allocation/start/stop/reclaim | explicit aligned stack, chunk buffer, bounded dormant wait/delete | `SIMPLIFY` | audio component lifecycle | preserve no-free-while-worker-live invariant; bounded polling is mechanism not requirement | lifecycle/failure injection + repeated start/stop; PS2 stress | `RECONSTRUCTION_READY` |
| audio startup reservoir policy | immediate/target/delay then epoch wait | `SIMPLIFY` | audio profile + audio startup process | preserve distinction between reservoir readiness and presentation deadline | pure policy tests + integration timing evidence | `RECONSTRUCTION_READY` |
| shared media epoch/deadline calculation | `h1_media_clock.*`, video arms, signed offsets, lead, timer ticks | `KEEP_BEHAVIOR` + `REWRITE` synchronization | media timing owner/value service; audio/MPEG presentation bridges consume deadlines | preserve publication ordering and signed-offset saturation behavior | pure deadline math tests, publication/concurrency test, A/V integration and PS2 timing qualification | `RECONSTRUCTION_READY` for common-clock contract; video arm callsite awaits MPEG/presentation audit |
| audio E2xx/debug stage writes | writes into transport diagnostic state around wait/play/stop | `DIAGNOSTIC_ONLY` | diagnostics observer | never make audio correctness depend on stage markers | run lifecycle tests with diagnostics absent | audit complete for audio diagnostics |

## Clean process boundary

Configuration should not become a god-object shared by every module. The clean configuration/profile owner supplies narrow immutable session/profile values to transport, audio, RFB, MPEG and presentation owners. Cross-directory startup/shutdown coordination belongs in each component's single bridge body, grouped by process.

Audio owns AUDSRV interaction, PCM format, worker resources and playback lifecycle. Transport owns the AUDIO logical byte queue and its credit accounting. The audio bridge consumes transport bytes; audio must not reach into transport internals or mutate transport diagnostics.

The shared media clock is a small timing responsibility, not an application coordinator. It owns epoch publication and deadline calculation. Application/presentation policy decides when the qualified presentation boundary arms it; consumers request deadlines/waits through the timing interface.

## Known cautions

- H1 uses `volatile` plus `EE_SYNCL()` for cross-thread epoch publication. Treat the ordering requirement as essential but do not blindly copy the representation.
- `audio_idle_delay_us` is reused as polling cadence for reservoir and clock waits. That coupling is experimental convenience; clean timing/event waits may separate those concerns.
- The 3-second bounded shutdown polling loop is a diagnostic/defensive mechanism. The semantic requirement is deterministic worker quiescence before reclamation.
- No claim is made here that arbitrary H1 tuning values are safe production settings. H1 explicitly allowed allocation/hardware behavior to be the experimental authority.

## Remaining audit boundary

A002 does not classify MPEG decode/generation, video scheduler/presentation/drop policy, calibration/compositor behavior, interaction, or top-level all-guns orchestration. Those remain `UNCLASSIFIED` except where this tranche identifies a narrow shared contract.

## Exact next audit pickup

Audit MPEG ingest/decode + CP2P generation/start/retire orchestration as the next coherent tranche. Resolve field 44 (`VIDEO_IPU_RESET_EACH_SESSION`) and the product/tuning status of MPEG/video fields 26-41 and 45-46 against the proven decoder/presentation lifecycle. Preserve the already-ready common media-clock contract while identifying the exact qualified video boundary that arms it.
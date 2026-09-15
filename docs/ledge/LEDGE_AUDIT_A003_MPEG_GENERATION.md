# Ledge Semantic Audit A003 — MPEG Decode and Exact-Generation Lifecycle

DOCUMENT=LEDGE_AUDIT_A003_MPEG_GENERATION
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T11:58:11-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_AUDIT_STATE_REVISION=0002
BASED_ON_GLOBAL_STATE_REVISION=0004
TEMPORAL_CLASS=AUDIT_TRANCHE
TEMPORAL_SEMANTICS=TRUE_AS_KNOWN_AT_RECORDED_TIME

This tranche classifies MPEG ingest/decode plus CP2P exact-generation START/retire lifecycle. It deliberately stops before compositor/calibration policy except for the narrow presentation boundary needed to settle shared-clock arming.

## Evidence inspected

- forensic H1 source commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- `experiments/media-harness-h1/h1_video_runtime.c` and `h1_config.h`;
- `CP2P_PI_START_RECEIVE_VALIDATION.md`;
- `CP2P_LIVE_GENERATION_BOUNDARY.md`;
- `CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`;
- `CP2P_MPEG_EVENT_WAKE_HARDWARE_RESULT.md`;
- `CP2P_MPEG_SAFE_STOP_LIFECYCLE.md`;
- `H1_INTEGRATION_INTENT.md`;
- A002 CONFIG/audio/clock audit, semantic ledger and simplification register;
- global state revision 0004 and audit state revision 0002.

## Required behavior

1. MPEG-2 elementary-stream bytes remain one logical channel on the one physical PSTV connection. The sole transport receiver commits bytes to one bounded session/generation queue; MPEG never creates a competing socket or receive owner.
2. A START request is exact-session/exact-generation state, not an advisory hint. Its geometry is validated before producer activation. Only one generation may be prepared/active/retiring at a time and stale generations cannot replace it.
3. CONFIG authorizes capability only. Live activation remains an ordered process: accepted profile -> calibrated/qualified geometry -> generation worker arm -> START(N) -> Pi producer/capture/suppression preparation -> exact-generation emission admission -> MPEG DATA.
4. Generation retirement is an ordered stream fence, not per-packet generation tagging. Producer admission closes and any in-flight send lease finishes before retirement completion. The PS2 closes channel-4 admission on exact retirement completion, stops/joins the exact worker, then atomically discards residual old-generation queue bytes, returns owed credit, and only then permits a fresh generation.
5. MPEG DATA admission after exact retirement completion is a protocol error. Residual bytes discarded at finalization are not decoder-consumed bytes.
6. Decoder stop is observed only at a completed `MPEG_Picture()`/project-code boundary. A local stop request must not make libmpeg's active data callback synthesize EOF while a picture decode is waiting for input. The previously observed false-EOF path caused global loss of forward progress and is a known defect that must remain explicitly prevented.
7. Normal MPEG empty-queue waiting uses producer/receiver-driven wake semantics rather than timer-backed hot-path polling. The receiver publishes the sole MPEG consumer thread and wakes it after committing MPEG DATA; wake-before-sleep ordering must be safe. This mechanism sustained the 600-second all-guns active-runtime hardware workload.
8. Finite producer completion and exact retirement are distinct lifecycle facts. Ordinary end-of-stream may terminate decode only when transport state really establishes exhaustion; owner-requested generation cancellation follows the safe-stop boundary above.
9. Decoder/session ownership includes establishing a known IPU/DMAC state before `MPEG_Initialize`, owning decoder buffers/packets for the session/generation, calling `MPEG_Destroy()` after decoder-call ownership returns, and restoring/releasing local resources before reuse. The H1 `VIDEO_IPU_RESET_EACH_SESSION` toggle is laboratory vocabulary; the clean decoder owner should perform the required lifecycle preparation unconditionally where the qualified decoder path requires it rather than expose a product recovery switch.
10. Sequence dimensions are validated against the qualified maximum/profile before allocating/using the picture surface. Feed bytes are bounded, aligned for TO_IPU DMA, and actual payload byte accounting remains distinct from padded DMA bytes.
11. The shared media epoch is armed at the first real video presentation boundary, not at CONFIG, START, producer admission, decode start, or queue prefill. In the qualified absolute scheduler this boundary is the first display synchronization immediately before the first frame's presentation deadline is established. A002's common-clock contract therefore remains valid with video presentation as the arm owner.
12. Decoder worker exit without an owner stop request is a session failure even if an earlier frame was displayed. A previously displayed frame must not mask a later live decoder/runtime failure.

## Field 44 and MPEG/video field disposition

- 26 `MPEG_START_TARGET_BYTES`, 27 `MPEG_PREFILL_WAIT_US`, 28 `MPEG_PREFILL_MAX_LOOPS`, 30 `MPEG_FEED_BYTES`: qualified-profile/resource-policy candidates. Preserve the semantics (bounded prefill/feed sizing), not H1's dynamic-knob API.
- 29 `MPEG_EMPTY_DELAY_US`: historical timer/polling vocabulary. The active-runtime qualified event-wake result supersedes timer-backed empty-queue polling as the preferred mechanism. Do not make this a production hot-path timing knob.
- 31-32 `VIDEO_FPS_NUM/DEN`: real stream/profile timing facts used for frame-period calculation.
- 33 `VIDEO_SCHEDULER_MODE`: experimental scheduler-comparison vocabulary. Production should consume the qualified presentation policy; alternate modes remain qualification tooling unless later presentation audit establishes a user/product need.
- 34 `VIDEO_PRESENTATION_OFFSET_US`: real common-clock profile value, already recognized by A002.
- 35 `VIDEO_PIXEL_MODE`: decoder/presentation profile fact; qualified path includes RGB16 via `_MPEG_Set16(1)`. Do not expose arbitrary mode switching unless later presentation audit requires it.
- 36-37 `VIDEO_MAX_WIDTH/HEIGHT`: decoder resource/sequence bounds and therefore narrow qualified-profile facts.
- 38-41 `VIDEO_DRAW_WIDTH/HEIGHT/X/Y`: presentation geometry, not decoder internals. Their final ownership/disposition remains for A004 calibration/compositor audit; A003 only requires validated geometry to reach the decoder/presentation boundary.
- 44 `VIDEO_IPU_RESET_EACH_SESSION`: **discard as a product toggle, preserve the lifecycle preparation it guarded**. H1 source describes explicit IPU/DMAC quiesce before MPEG initialization as a qualified mechanism; clean MPEG ownership should establish the required known state as part of decoder acquisition rather than accept a session switch that can disable it.
- 45-46 `VIDEO_DROP_ENABLED/THRESHOLD`: presentation scheduling policy, not ingest/decode lifecycle. Keep as qualification/profile candidates and defer final production disposition to A004.
- 55-56 `VIDEO_ENCODE_WIDTH/HEIGHT`: producer/capture geometry facts. Preserve exact-generation geometry consistency; final calibration/profile ownership remains A004.

## A003 responsibility disposition

| Responsibility | Historical mechanism | Disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Pipeline status |
| --- | --- | --- | --- | --- | --- | --- |
| MPEG logical-channel ingest and event wake | transport MPEG queue + sole receiver wake of sole MPEG waiter | `KEEP_BEHAVIOR` + `SIMPLIFY` | transport owns queue/wake state; MPEG bridge process `generation data consumption` | do not restore timer-backed empty-queue polling as correctness mechanism | host queue/wake/lost-wakeup/error/end tests; PS2 endurance | `RECONSTRUCTION_READY` |
| decoder acquisition/feed/sequence validation/release | H1 video runtime + SMS libmpeg + IPU/DMAC/GIF ownership | `KEEP_BEHAVIOR` + `SPLIT` | MPEG component owns decoder/IPU/feed/buffers; presentation owns GS composition | preserve decoder-call ownership and resource release order | host policy tests where possible; canonical PS2 build; repeated decoder start/stop hardware qualification | `RECONSTRUCTION_READY` |
| IPU/DMAC reset switch | conditional field 44 guarding known-state preparation | `SIMPLIFY` | MPEG decoder acquisition lifecycle | preserve required quiesce/reset semantics; remove laboratory ability to disable required preparation | repeated-session and fault-recovery hardware tests | `RECONSTRUCTION_READY` |
| exact START validation/prepared generation | 44-byte START v1 + Pi prepared-generation state | `KEEP_BEHAVIOR` + `REWRITE` representation | application/MPEG process through transport and producer bridge | stale/mismatched generation remains rejection | pure START geometry/session/generation validation; stale/repeat tests | `RECONSTRUCTION_READY` |
| exact-generation producer emission fence | begin/finish emission lease around each MPEG send; one-way retiring latch | `KEEP_BEHAVIOR` | Pi producer/transport generation process; PS2 transport consumes channel | retiring generation must never reopen | concurrency/state-machine tests around close vs in-flight send | `RECONSTRUCTION_READY` |
| safe decoder stop boundary | stop request observed after `MPEG_Picture()` returns, never as synthetic callback EOF | `KEEP_BEHAVIOR` | MPEG worker lifecycle | preserve known false-EOF/global-freeze defect prevention explicitly | deterministic callback/stop interleavings plus PS2 stop/relaunch stress | `RECONSTRUCTION_READY` |
| retirement/final residual discard/credit return | ordered TCP retirement ACK + worker stop/join + queue finalization | `KEEP_BEHAVIOR` | application process coordinated through MPEG and transport bridges | residual discard must not count as decoder consumption | generation retire/reopen/credit tests; repeated generation hardware qualification | `RECONSTRUCTION_READY` |
| first-presentation media-epoch arm | first real display boundary before first presentation deadline | `KEEP_BEHAVIOR` | presentation process arms media timing service | do not arm at decode/START/prefill merely for convenience | clock/presentation integration tests and A/V timing hardware evidence | `RECONSTRUCTION_READY` for arm contract; A004 owns compositor/calibration implementation |
| scheduler/drop/geometry experiment surface | scheduler modes, drop threshold, draw/capture geometry knobs | `DEFER` | presentation/calibration/profile | do not silently choose new presentation behavior in decoder migration | A004 calibration/compositor audit and qualification | `UNCLASSIFIED` beyond narrow contracts above |
| MPEG timing/stage/result counters | broad result and stage telemetry | `DIAGNOSTIC_ONLY` | diagnostics observer | decoder correctness must not depend on witnesses | lifecycle tests with diagnostics disabled | audit complete for decoder diagnostics |

## Clean ownership / bridge boundary

MPEG owns decoder acquisition, IPU/DMAC feed, libmpeg call ownership, sequence/resource bounds, worker lifecycle and safe stop. Transport owns channel-4 storage, credit, receive dispatch and generation-admission transport facts. Presentation owns GS/composition policy and is the process that arms the shared media clock at the qualified first-presentation boundary. Application owns the high-level exact-generation transition and coordinates it only through each component's single process-organized bridge.

No component should import another component's internals to reproduce H1's coordinator. In particular, MPEG must not reach into transport queue/semaphore internals, and transport must not decide decoder EOF/cancellation semantics.

## Known defects and cautions

- The synthetic-EOF-on-stop path is a known proven defect; reconstruction must preserve its prevention rather than rediscover it.
- The 600-second event-wake checkpoint proved active runtime but did not prove finite session completion at that historical revision. Later safe-stop evidence proved asynchronous MPEG retirement separately. These are temporally distinct proofs and must not be flattened into a claim that every repeated-session path was hardware-qualified.
- Event wake removes a demonstrated problematic timer-backed MPEG hot path; it does not prove a universal PS2SDK timer defect.
- Presentation/calibration correctness, matte/suppression alignment, cursor/OSK layering and non-default geometry remain outside A003.

## Validation obligations

Machine validation should cover exact START parsing/geometry/session/generation rejection, stale generation prevention, one-way retiring state, send-lease close races, queue discard/credit accounting, wake-before-sleep, MEDIA_END/exhaustion, safe-stop interleavings, decoder result propagation, sequence/resource bounds, feed alignment/accounting, and bridge/dependency rules. Canonical PS2 build identity and PT_LOAD evidence are required for reconstructed hardware-facing code.

Hardware qualification remains separate and must include repeated start/stop/relaunch generations, finite session completion, active all-guns endurance, and repeated sessions. Repository evidence alone cannot qualify the reconstructed implementation.

## Progress and remaining boundary

Three coherent process tranches are now classified (A001-A003). A003 contributes seven reconstruction-ready responsibility groups plus one narrow ready clock-arm contract; scheduler/drop/calibration/compositor details remain deliberately deferred. Whole-build recursive source/symbol completeness remains incomplete.

## Exact next audit pickup

A004: presentation/compositor/calibration lifecycle. Trace calibration acceptance, draw/capture/suppression geometry ownership, RFB suppression/matte alignment, presentation ownership transitions (`RFB_ONLY` <-> MPEG), cursor/OSK/local-UI layering, scheduler/drop policy, and the clean bridge boundaries. Do not broaden into controller/keyboard interaction except where calibration UI input is necessary to classify the presentation process.
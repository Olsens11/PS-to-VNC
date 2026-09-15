# Ledge Semantic Audit A004 — Presentation, Compositor, and Calibration

DOCUMENT=LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T13:08:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_AUDIT_STATE_REVISION=0003
BASED_ON_GLOBAL_STATE_REVISION=0005
TEMPORAL_CLASS=AUDIT_TRANCHE
TEMPORAL_SEMANTICS=TRUE_AS_KNOWN_AT_RECORDED_TIME

This tranche classifies the coherent presentation/compositor/calibration process. It deliberately does not broaden into general controller/keyboard interaction or top-level all-guns shutdown beyond the foreground/calibration input facts required to define presentation ownership.

## Evidence inspected

- forensic H1 source commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- `experiments/media-harness-h1/h1_video_runtime.c`;
- `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`;
- H1 integration intent and the durable CP2P lifecycle records indexed by that history;
- A003 MPEG/generation audit revision 0001;
- global state revision 0005 and audit state revision 0003;
- base clean architecture version 1 and ledge architecture overlay revision 0001.

The experiment history is chronological evidence rather than timeless specification. Later qualified/current-path entries were used to resolve earlier incomplete states instead of projecting later facts backward.

## Required behavior

1. Calibration owns semantic controller interpretation only while its foreground is active. Physical pad acquisition continues. Mouse/ordinary desktop interpretation is suspended, and release quarantine prevents calibration-held controls from leaking into resumed desktop input.
2. Calibration geometry has three distinct authorities: the **base rectangle** is the exact Pi capture rectangle and PS2 MPEG presentation region; the **inner matte** is PS2-local presentation-only inset state; the **outer matte** expands the MPEG visual-ownership footprint and therefore determines the RFB suppression perimeter without changing capture geometry.
3. Base size remains MPEG-compatible/aligned where required; base movement remains independently precise. Acceptance produces one immutable committed geometry for the new generation. Do not duplicate an accepted-edge detector or a second geometry store merely to cross a component boundary.
4. Entering calibration freezes visible RFB evolution at a safe complete-message/request boundary while keeping the RFB/PSTV session alive. No new framebuffer-update requests are issued while the presentation owner requires RFB frozen. Local calibration UI is rendered over the frozen desktop.
5. Cancel restores ordinary RFB ownership and creates exactly one fresh nonincremental/full framebuffer-update obligation. Coincident restoration causes collapse to one full refresh, and that obligation is not consumed while another owner still requires RFB frozen.
6. Accept does **not** immediately transfer visual ownership to MPEG. Accepted geometry begins exact-generation preparation; RFB remains protected during the accept-to-first-frame gap. The Pi must install the exact generation-specific capture/suppression state before MPEG emission becomes live.
7. The first valid **physically presented** MPEG frame is the ownership-promotion boundary. This is also the A003-qualified first-real-presentation boundary that arms the shared media epoch. CONFIG, accepted calibration, START send, producer admission, queue prefill, decode start, or upload alone must not arm the clock or claim MPEG visual ownership.
8. While MPEG owns generation N, RFB presentation is forbidden from repainting the suppression rectangle derived from the accepted outer footprint. Suppression is presentation ownership, not a second framebuffer truth: the authoritative remote framebuffer may continue to receive valid RFB pixels while presentation excludes the MPEG-owned footprint.
9. Shared composition preserves one physical GS presentation owner/path. The recovered qualified composition order is conceptually remote desktop -> suppression/matte footprint -> MPEG base presentation -> inner matte -> local overlay -> one GS synchronization/presentation path -> physical flip. Do not link independent competing graphics owners.
10. MPEG retirement/failure/recalibration cannot restore RFB presentation before exact Pi generation retirement proves suppression/capture state is gone and the local MPEG owner/worker has retired. Restoration then schedules exactly one fresh full RFB request.
11. Local UI/OSK/calibration overlays remain the final local presentation layer and may temporarily own foreground semantics without becoming owners of RFB protocol state, MPEG decode state, or transport state.
12. H1's absolute video presentation scheduler is required semantic behavior at policy level: frame deadlines derive from the qualified stream rate/common epoch and presentation offset; timing policy must not be replaced by an arbitrary timer loop. Scheduler comparison modes and stage telemetry are laboratory machinery, not production user-facing modes.
13. Frame dropping, where retained by the qualified profile, is a presentation policy for frames already too late relative to the common clock. It must not alter generation ownership, transport credit accounting, decoder-consumed-byte accounting, or manufacture end-of-stream. The exact threshold is a qualified-profile value rather than a general product knob.
14. Draw geometry and Pi encode/capture geometry are generation/profile facts derived from the accepted base rectangle and qualified presentation transform. H1 CONFIG fields 38-41 and 55-56 are not independent mutable product authorities once runtime calibration is accepted.

## H1 field disposition completed by A004

- 38-41 `VIDEO_DRAW_WIDTH/HEIGHT/X/Y`: preserve presentation-rectangle semantics but remove them as an independent per-session laboratory authority when runtime accepted geometry is active. Presentation owns the resolved draw rectangle for the exact generation.
- 45 `VIDEO_DROP_ENABLED`: preserve only as part of the qualified presentation policy. It is not a user-facing product mode unless future evidence establishes such a need.
- 46 `VIDEO_DROP_THRESHOLD`: preserve the lateness-threshold semantic as a narrow qualified profile value when dropping is enabled; do not expose the H1 sweep knob as mandatory CONFIG.
- 55-56 `VIDEO_ENCODE_WIDTH/HEIGHT`: preserve exact capture/encode geometry consistency, but derive the live generation values from accepted base geometry rather than maintain a competing independent authority.
- `VIDEO_SCHEDULER_MODE` (field 33, deferred by A003): discard comparison-mode selection from production behavior. Preserve the qualified absolute common-clock scheduler.
- `VIDEO_STAGE_MARKERS` / `VIDEO_STAGE_HOLD_VSYNCS`: remain diagnostic-only as already classified; they are not presentation correctness mechanisms.

## A004 responsibility disposition

| Responsibility | Historical mechanism | Disposition | Intended clean owner/process/bridge | Known-defect treatment | Validation obligation | Pipeline status |
| --- | --- | --- | --- | --- | --- | --- |
| calibration foreground/acceptance | portable calibration core + adapter + foreground/runtime accepted edge | `KEEP_BEHAVIOR` + `SIMPLIFY` | calibration/local-UI process; application coordinates through single bridges | preserve release quarantine and X release/re-press acceptance; no duplicate accepted-edge detector | pure geometry/state tests, held-button/release tests, foreground routing tests | `RECONSTRUCTION_READY` |
| base/inner/outer geometry authority | committed base region + local inner matte + outer suppression footprint | `KEEP_BEHAVIOR` | calibration owns edit/commit; presentation consumes resolved geometry; Pi producer consumes START base/suppression only | do not collapse base and suppression or send inner matte to Pi | geometry/alignment/bounds/serialization tests | `RECONSTRUCTION_READY` |
| RFB freeze/request policy | calibration RFB gate + safe-boundary scheduler + one-shot full-refresh obligation | `KEEP_BEHAVIOR` + `REWRITE` representation | RFB/application presentation-ownership process | do not consume restoration while another owner freezes RFB; cancel/restoration require one full refresh | scheduler/state-machine tests at complete-message boundaries | `RECONSTRUCTION_READY` |
| accept-to-first-frame protection | accepted generation prepared while RFB remains protected | `KEEP_BEHAVIOR` | application process coordinates calibration, transport/producer, MPEG and presentation bridges | acceptance alone must never expose stale RFB/MPEG overlap | generation/ownership transition tests | `RECONSTRUCTION_READY` |
| first physical MPEG ownership + media epoch arm | first real synchronized MPEG presentation promotes owner and arms common epoch | `KEEP_BEHAVIOR` | presentation bridge process | do not arm/promote on START/decode/upload; preserve A003 clock contract | presentation/clock integration tests; PS2 A/V timing evidence | `RECONSTRUCTION_READY` |
| shared compositor layering | cumulative39 graphics wrapper replaces inherited graphics object and composes RFB+matte+MPEG+local overlay through one GS owner | `KEEP_BEHAVIOR` + `SIMPLIFY` | presentation component; one bridge body, process-organized composition section | duplicate graphics owners/linkage are invalid; do not create parallel GS presenters | dependency/link checks, render-plan tests where possible, PS2 visual qualification | `RECONSTRUCTION_READY` |
| MPEG-owned RFB suppression | generation-specific outer footprint excludes RFB presentation while remote framebuffer remains authoritative | `KEEP_BEHAVIOR` | presentation consumes ownership; Pi capture/suppression lifecycle through producer bridge | suppression must match exact generation and may not outlive retirement proof | suppression rectangle and generation tests; visual hardware checks | `RECONSTRUCTION_READY` |
| retirement/failure restoration | exact Pi retirement -> local worker/owner retire -> one fresh FULL RFB request | `KEEP_BEHAVIOR` | application ownership-transition process | never restore before Pi suppression/capture retirement; collapse duplicate refresh obligations | retire/recalibrate/failure interleavings; repeated-generation hardware tests | `RECONSTRUCTION_READY` |
| scheduler/drop policy | absolute common-clock deadlines plus optional lateness drop | `KEEP_BEHAVIOR` + `SIMPLIFY` | presentation timing policy/profile | dropping must not mutate transport/generation/EOF semantics | deterministic deadline/drop tests and PS2 timing/endurance | `RECONSTRUCTION_READY` |
| H1 draw/encode knobs and scheduler comparison modes | dynamic CONFIG sweep fields | `SIMPLIFY` | accepted generation + qualified presentation profile | do not create multiple live geometry authorities | profile/geometry consistency tests | `RECONSTRUCTION_READY` |
| stage colors/timing counters/calibration witnesses | broad experimental visual/timing instrumentation | `DIAGNOSTIC_ONLY` | diagnostics observer | correctness must not depend on stage holds/counters | behavior with diagnostics disabled | audit complete for A004 diagnostics |

## Clean ownership and bridge boundary

Calibration/local UI owns edit state, help/review state, accepted-edge semantics, committed base/inner/outer values, and foreground controller consumption while active. Presentation owns the one GS composition path, resolved draw geometry, matte/suppression rendering, ownership state used for drawing, physical presentation boundary, and common-clock arm callsite. RFB owns protocol parsing/framebuffer truth and request mechanics; it receives freeze/full-refresh policy through its bridge rather than learning MPEG internals. MPEG owns decode/upload readiness but does not promote itself merely because a decoded surface exists. Application owns the cross-domain transition sequence and coordinates only through each component's one process-organized bridge. Pi producer/transport owns exact-generation capture/suppression installation and retirement proof.

Within one presentation component directory, compositor/render/timing files may cooperate directly. Any call to calibration, RFB, MPEG, transport, or application crosses only through the owning component bridge; do not reproduce H1's historical adapter proliferation by destination.

## Simplifications justified by hindsight

- Keep one committed calibration geometry and propagate the existing accepted edge; remove compatibility wrappers/adapter layers that existed only to incrementally prove the experiment once clean callers can use the final narrow result.
- Keep one presentation/graphics owner. The historical cumulative39 wrapper/drop-in proved how to preserve Issue-39 graphics while adding MPEG; reconstruction should express the resulting responsibilities directly rather than retain source-inclusion symbol renaming or link two graphics implementations.
- Represent RFB freeze/full-refresh as explicit presentation-ownership/request-scheduler state rather than a collection of experiment-local gates.
- Derive live capture/draw/suppression geometry from the accepted generation plus qualified transform/profile. Do not retain independent H1 CONFIG authorities for the same rectangle.
- Preserve the qualified absolute scheduler and narrow lateness policy; remove scheduler-comparison and stage-marker experiment surfaces from product configuration.

## Known defects and cautions

- A held X must not enter REVIEW and immediately accept; release/re-press is required.
- Calibration-held controller state must not leak into resumed desktop control; release quarantine remains required.
- The accept-to-first-frame interval must not be mistaken for MPEG ownership. Premature promotion can expose stale/overlapping RFB presentation and would arm the shared clock too early.
- RFB restoration before exact Pi retirement would allow the desktop to repaint while generation-scoped suppression/capture is still active. Preserve the ordered retirement proof.
- Historical duplicate graphics-owner linkage was a structural composition failure; the recovered shared compositor established that there is one graphics owner, not two cooperating public presenters.
- Cursor sluggishness observed in earlier CP2N qualification is a performance observation, not an A004 correctness failure and must not be silently "fixed" as part of structural migration.
- Historical H1 evidence qualifies particular experimental artifacts/processes. It does not hardware-qualify reconstructed presentation code.

## Validation obligations

Machine validation should cover calibration state transitions, X release/re-press, release quarantine, geometry bounds/alignment, base-vs-inner-vs-outer invariants, accepted-edge single emission, immutable generation geometry, RFB safe-boundary freeze, one-shot full-refresh collapse, accept-to-first-frame protection, exact-generation suppression, retirement ordering, scheduler deadlines/drop classification, and dependency/one-bridge rules. Static/link checks must prove one presentation/graphics owner and no destination-specific bridge sprawl.

Canonical PS2 build identity and PT_LOAD evidence are required for reconstructed hardware-facing presentation code. Hardware qualification remains separate and must include visual geometry/matte correctness, first-frame ownership, no RFB repaint inside the owned footprint, local overlay layering, A/V timing, cancel/accept/recalibrate, retirement/failure restoration, repeated generations, and all-guns endurance. Repository evidence alone cannot qualify the reconstructed implementation.

## Progress and remaining boundary

Four coherent process tranches are now classified (A001-A004). A004 contributes nine reconstruction-ready responsibility groups plus one diagnostic-only group and closes the presentation/calibration disposition of H1 fields 33, 38-41, 45-46, and 55-56.

Remaining seeded major process families: 3 — general interaction/input composition; top-level all-guns orchestration/shutdown/recovery; residual diagnostics plus exhaustive recursive dependency/source/symbol completeness closure.

## Exact next audit pickup

A005: interaction/input composition. Trace controller acquisition versus semantic ownership, mouse/pointer/button/scroll routing, OSK/local-UI foreground transitions, keyboard tap/modifier serialization, calibration entry/exit handoff, release quarantine, and any RFB safe-boundary service requirements. Do not reopen A004 geometry/presentation policy except where an interaction transition directly consumes its already-classified foreground/ownership contract.
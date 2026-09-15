# Ledge Simplification Register

DOCUMENT=LEDGE_SIMPLIFICATION_REGISTER
DOCUMENT_REVISION=0003
RECORDED_AT=2026-09-15T13:09:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0002
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=REGISTER_TRUE_AS_KNOWN_AT_RECORDED_TIME

A simplification is not permission to remove behavior. Each accepted entry identifies the historical mechanism, essential semantics, proposed clean mechanism, why the reduction is legitimate, and validation needed.

## S001 — RFB commit-dispatch shutdown fence

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: `h1_rfb_wait_for_receiver_commit_dispatch()` infers receiver dispatch completion from `receiver_loop_count == frames_received` and bounded 1 ms `DelayThread()` polling after the RFB owner observes COMMIT.

Essential semantic: after a frame is counted, session/media teardown must not proceed while the sole PSTV receiver can still be suspended inside that frame's accept/activity-signal path. Ordered quiesce must prove receiver dispatch ownership has returned to a safe boundary.

Clean mechanism: make dispatch/quiescence state an explicit transport lifecycle fact (or equivalent event/ownership handoff) consumed by the shutdown process. Do not make production correctness depend on diagnostic progress counters or scheduler sleeps.

Why legitimate: the counters and delay are evidence mechanisms for a scheduling race; the invariant they revealed is the product requirement. `h1_transport_runtime.h` already demonstrates event-based receiver termination and activity rendezvous elsewhere, so explicit synchronization is consistent with the proven ownership model.

Validation required: deterministic host/state interleavings around counted-frame -> accept -> notify -> owner COMMIT handling where possible, plus PS2 stress/qualification for the changed PT_LOAD.

## S002 — Process-organized bridges

STATUS=POLICY_ACCEPTED

Historical mechanism: exploratory H1 cross-component calls evolved alongside experiments and may be distributed by implementation history.

Essential semantic: coherent outward-facing operations may touch multiple components and must preserve ordering/invariants across those interactions.

Clean mechanism: one bridge body per component, divided into coherent process sections. A section contains all cross-component coordination for that process rather than scattering it into one adapter per destination.

Validation required: architecture checker/import/include rules plus behavioral integration tests. The bridge must remain coordination/translation rather than accumulating component internals.

## S003 — Replace build-time RFB symbol renaming/global bind seam

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: the cumulative H1 makefile preprocessor-renames the clean RFB session's three `rfb_io` calls to H1 mux functions. `h1_rfb_mux_io` then binds one H1 runtime globally/session-scoped while preserving the old integer socket-handle API.

Essential semantic: the existing clean RFB parser/session operates over an exact byte stream carried by logical PSTV channel 1; there is still one physical socket and one receiver.

Clean mechanism: give the RFB session an explicit narrow byte-stream/transport interface at its owned I/O boundary and have the RFB bridge bind that interface to transport logical channel 1. No preprocessor symbol surgery and no reinterpretation of a physical socket fd as an indirect runtime lookup key.

Why legitimate: `H1_INTEGRATION_INTENT.md` explicitly identifies the mux as a transport upgrade beneath existing RFB behavior and permits a thin boundary adapter; the build-time rename exists to prove the seam without modifying clean source, not because it is desirable production architecture.

Validation required: unchanged RFB protocol/session tests against a fake byte stream, mux adapter tests, and integration proof that no second PS2-facing RFB TCP stream/receiver exists.

## S004 — Separate product synchronization from diagnostic witnesses

STATUS=SEMANTIC_ACCEPTED

Historical mechanism: H1 accumulated broad volatile stats and E2xx diagnostic stage fields for queue, audio, MPEG, receiver, RFB, and thread-census investigations. Some latest shutdown evidence temporarily reads progress counters to prove scheduling position.

Essential semantic: product lifecycle and flow-control state must remain observable enough to diagnose failures, but correctness cannot depend on disposable diagnostic counters unless the counter itself is the explicit protocol/state authority.

Clean mechanism: explicit owned state/events for synchronization; diagnostics receive snapshots/counters as observers. Retain only observability required by current qualification/debug contracts.

Validation required: architecture review that diagnostics cannot drive product decisions; tests proving lifecycle behavior with diagnostic emission disabled; retain exact runtime/qualification identity separately.

## S005 — One committed calibration geometry authority

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: H1 grew a portable calibration core, adapter, foreground layer, runtime result, binding wrappers, committed-region accessor, CONFIG draw/encode fields, and later generation START geometry while incrementally proving each seam.

Essential semantic: calibration emits one accepted edge and one immutable committed generation geometry. Base/capture, inner-matte, and outer-suppression meanings remain distinct. The accepted edge must cross outward exactly once and held controls must be quarantined on release.

Clean mechanism: keep one calibration-owned committed value and propagate its existing accepted result through the component's process-organized bridge. Derive generation draw/capture/suppression values from that authority plus the qualified presentation transform/profile. Remove compatibility wrappers and duplicate geometry stores that existed only to stage the experiment.

Why legitimate: the experiment history identifies the earlier accepted-edge loss as propagation failure and explicitly rejects a second detector; it also identifies the committed-region accessor as the existing geometry authority.

Validation required: geometry/state tests, single accepted-edge observation, held-X release/re-press, release quarantine, exact START geometry consistency, and repeated calibration/recalibration hardware checks.

## S006 — One presentation/graphics owner instead of cumulative wrapper mechanics

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: the CP2P cumulative39 graphics source wrapped/included the through-Issue-39 graphics implementation under private renamed symbols, then exported one public graphics API plus MPEG compositor behavior. Attempting to link both as independent graphics objects produced duplicate ownership/link failure.

Essential semantic: remote desktop, MPEG matte/presentation, and local overlays share one physical GS composition/presentation owner and one final synchronization/flip path while preserving the established Issue-39 graphics behavior.

Clean mechanism: reconstruct those responsibilities directly inside one presentation component. Internal presentation files may cooperate directly; cross-directory calls pass through the single presentation bridge. Do not retain source-inclusion symbol renaming and do not link competing public graphics owners.

Why legitimate: the wrapper was a compatibility technique for proving the experiment against inherited code; the duplicate-owner failure demonstrated that it was never two independent presentation domains.

Validation required: dependency/link checks proving one graphics owner, render-order policy tests where host-testable, canonical PS2 build/PT_LOAD identity, and physical visual qualification.

## S007 — Explicit presentation ownership replaces experiment-local RFB gates

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: calibration freeze gates, RFB request scheduler policy, MPEG presentation ownership, suppression state, and one-shot full-refresh obligations were added in successive experiment tranches.

Essential semantic: RFB can be frozen without killing its session; acceptance does not itself grant MPEG ownership; first physical MPEG presentation promotes ownership; exact-generation suppression excludes RFB presentation; cancel/retirement/failure restores RFB with exactly one full refresh only after all owners release the freeze.

Clean mechanism: represent these facts as explicit application/presentation ownership state plus narrow RFB request-policy operations. Preserve safe complete-message boundaries and generation retirement proof without retaining a stack of destination-specific gate wrappers.

Why legitimate: later H1 composition explicitly combined the separate gates into one ownership policy and proved that coincident full-refresh obligations collapse while freezes compose.

Validation required: deterministic ownership/freeze/full-refresh state-machine tests, accept-to-first-frame tests, retirement/recalibration races, and PS2 visual/endurance qualification.

## S008 — Qualified absolute scheduler, not scheduler-comparison surface

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: H1 exposed scheduler mode, presentation offset, drop enable/threshold, stage markers, and timing counters to compare alternatives during qualification.

Essential semantic: video presentation uses the shared media epoch, qualified stream-rate deadlines and presentation offset; optional lateness dropping is presentation policy only and cannot mutate transport/generation/EOF accounting.

Clean mechanism: retain the qualified absolute scheduler and narrow profile values required by it. Remove scheduler-comparison modes and stage-marker timing controls from product configuration; keep diagnostics observational.

Why legitimate: comparison knobs existed to discover the qualified mechanism, not to provide a product feature. A003 already fixed the first-real-presentation clock-arm boundary; A004 fixes the owning presentation policy.

Validation required: deterministic deadline/drop tests, A/V clock integration tests, diagnostics-disabled behavior, and PS2 timing/endurance evidence.
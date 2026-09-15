# Ledge Simplification Register

DOCUMENT=LEDGE_SIMPLIFICATION_REGISTER
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-15T15:00:35-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0003
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=REGISTER_TRUE_AS_KNOWN_AT_RECORDED_TIME

A simplification is not permission to remove behavior. Each accepted entry identifies the historical mechanism, essential semantics, proposed clean mechanism, why the reduction is legitimate, and validation needed. Detailed rationale for S001-S008 remains as recorded in revision 0003 and their status is unchanged.

## S001 — RFB commit-dispatch shutdown fence
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: session/media teardown must not proceed while the sole PSTV receiver can still be inside dispatch for a counted frame. Clean mechanism: explicit transport dispatch/quiescence state/event rather than diagnostic counter equality plus scheduler sleeps. Validation: deterministic dispatch interleavings plus PS2 stress/qualification.

## S002 — Process-organized bridges
STATUS=POLICY_ACCEPTED
Essential semantic: coherent cross-component operations preserve ordering/invariants. Clean mechanism: one bridge body per component divided into coherent process sections, not destination-specific adapter sprawl. Validation: architecture checks plus behavioral integration.

## S003 — Replace build-time RFB symbol renaming/global bind seam
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: clean RFB consumes an exact logical-channel byte stream over the sole PSTV transport. Clean mechanism: explicit narrow RFB byte-stream interface bound by the RFB bridge to transport channel 1. Validation: RFB fake-stream, mux adapter and sole-physical-owner integration tests.

## S004 — Separate product synchronization from diagnostic witnesses
STATUS=SEMANTIC_ACCEPTED
Essential semantic: lifecycle/flow-control state remains observable but disposable diagnostics cannot be synchronization authority. Clean mechanism: owned state/events for correctness and observer-only diagnostics. Validation: lifecycle behavior with diagnostics disabled.

## S005 — One committed calibration geometry authority
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: one accepted edge and immutable committed generation geometry, with distinct base/capture, inner-matte and outer-suppression meanings. Clean mechanism: one calibration-owned committed value propagated through the process bridge. Validation: geometry/state, accepted-edge, quarantine and repeated calibration tests.

## S006 — One presentation/graphics owner instead of cumulative wrapper mechanics
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: desktop, MPEG matte/presentation and local overlays share one physical GS composition/synchronization/flip owner. Clean mechanism: direct presentation-component ownership, no source-inclusion symbol renaming or competing graphics owner. Validation: link/dependency, render-order, canonical build/PT_LOAD and physical visual qualification.

## S007 — Explicit presentation ownership replaces experiment-local RFB gates
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: freeze without killing RFB; first physical MPEG presentation promotes ownership; exact-generation suppression; retirement/failure restores one full refresh after all freeze owners release. Clean mechanism: explicit application/presentation ownership state plus narrow RFB request policy. Validation: ownership/freeze/full-refresh state machine and PS2 visual/endurance.

## S008 — Qualified absolute scheduler, not scheduler-comparison surface
STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE
Essential semantic: shared-epoch absolute video deadlines and narrow lateness/drop presentation policy without altering transport/generation/EOF accounting. Clean mechanism: qualified scheduler plus narrow profile values; comparison/stage controls remain development diagnostics. Validation: deadline/drop, A/V clock, diagnostics-disabled and PS2 timing/endurance.

## S009 — Resident lifecycle state instead of checkpoint runner mechanics

STATUS=SEMANTIC_ACCEPTED_IMPLEMENTATION_REWRITE

Historical mechanism: H1's resident checkpoint `main()` combines boot, transport-connect retry, checkpoint-specific CONFIG predicates, component startup, polling waits, banners/counters, RESULT reporting, reverse teardown, a between-session delay and deliberately static input storage used to avoid unsafe reclamation if dormancy cannot be proven.

Essential semantic: boot prerequisites precede admission; one resident process serves repeated finite sessions; transport publishes validated CONFIG before optional consumers; component startup/retirement follows dependency ownership; failures converge monotonically to shutdown; resources are not reused until owner dormancy/retirement is proven; terminal result does not erase earlier failure; and an unexplained silent wait is not converted to success by a generic timeout.

Clean mechanism: application-owned lifecycle state/process sections (`resident startup`, `session admission`, `session startup`, `steady state`, `failure convergence`, `terminal result`, `session teardown/re-admission`) using explicit component lifecycle state/events through process-organized bridges. Replace correctness polling/delays and checkpoint predicates with owned events/state and qualified profile admission. Keep diagnostic output observational. Static storage is unnecessary when dormancy is proven; if it is not proven, fail closed and stop re-admission.

Why legitimate: H1 integration intent explicitly defines the harness as a proving ground rather than alternate product architecture. The checkpoint main exists to drive and observe the experiment. A001-A005 already identify the durable component owners and the exact lifecycle facts that must survive.

Validation required: deterministic startup-failure matrix; dependency-order assertions; missing/invalid/incompatible CONFIG; disconnect/reconnect; component failure during steady state; result-send failure after prior local failure; receiver/input/decoder dormancy barriers; at least two clean sessions with no stale config/generation/credit/pointer state; canonical PS2 build/PT_LOAD identity; and repeated-session hardware qualification.
# Ledge Simplification Register

DOCUMENT=LEDGE_SIMPLIFICATION_REGISTER
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-15T10:09:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0001
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
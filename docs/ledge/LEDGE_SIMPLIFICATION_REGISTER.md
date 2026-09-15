# Ledge Simplification Register

DOCUMENT=LEDGE_SIMPLIFICATION_REGISTER
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T09:43:34-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=REGISTER_TRUE_AS_KNOWN_AT_RECORDED_TIME

A simplification is not permission to remove behavior. Each accepted entry must identify the historical mechanism, essential semantics, proposed clean mechanism, why the reduction is legitimate, and validation needed.

## Candidate S001 — RFB commit-dispatch shutdown fence

STATUS=INVESTIGATE

Historical mechanism: `h1_rfb_wait_for_receiver_commit_dispatch()` polls transport error/stop state and waits until `receiver_loop_count == frames_received`, sleeping in bounded 1 ms steps before quiesce/shutdown proceeds.

Essential semantic candidate: do not allow the RFB owner to tear down media/session state while the sole receiver is still inside the dispatch/signal path for a just-counted frame.

Potential simplification: represent receiver/dispatch ownership or quiescence directly rather than inferring it from diagnostic/progress counters and scheduling sleeps.

Why not accepted yet: only the latest commit diff has been audited in this shift. Full transport/event ownership must be traced before changing the mechanism.

Validation required: concurrency/state tests where host-testable plus exact PS2 hardware qualification for any PT_LOAD-changing implementation.

## Candidate S002 — Process-organized bridges

STATUS=POLICY_ACCEPTED

Historical mechanism: exploratory H1 cross-component calls evolved alongside experiments and may be distributed by implementation history.

Essential semantic: coherent outward-facing operations may touch multiple components and must preserve ordering/invariants across those interactions.

Clean mechanism: one bridge body per component, divided into coherent process sections. A section contains all cross-component coordination for that process rather than scattering it into one adapter per destination.

Validation required: architecture checker/import/include rules plus behavioral integration tests. The bridge must remain coordination/translation rather than accumulating component internals.

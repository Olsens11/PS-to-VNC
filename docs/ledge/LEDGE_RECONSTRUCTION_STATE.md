# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0006
RECORDED_AT=2026-09-15T19:58:06-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0005
BASED_ON_AUDIT_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0010
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Current reconstruction phase

`A001_LOGICAL_RFB_RUNTIME_SOURCE_IN_PROGRESS`

## Authority and movement reconciled

- branch `ledge/h1-all-guns` began this interactive shift at `686780e89d5629ad13e18897d78e241b0b489266`;
- forensic H1 source authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0007` keeps A001 reconstruction-ready and the seeded audit complete;
- global state revision `0010` keeps A001 active and A002-A006 queued;
- validation V003 and V004 remain OPEN; no validation finding is rewritten here;
- governing reconstruction contract revision `0002` and immutable work-log contract revision `0001` remain current.

Repository authority was re-read between bounded commits. No competing committed reconstruction advance was observed during this interactive shift. Unknown external Pi-local dirty work remains outside this GitHub-native mutation surface and was neither overwritten nor declared absent.

## Progress

A001 now has a first higher Transport runtime above `physical_stream.*` and `rfb_channel.*`:

- `runtime.h/.c` owns CONFIG-sized logical RFB storage, one queue mutex, a producer-driven RFB activity event, one explicit receiver-completion event, receiver thread configuration/stack, flow-control state, and the sole receiver thread;
- only that receiver thread calls `pstvnc_transport_physical_stream_receive_frame()` in the reconstructed higher runtime;
- complete channel-1 DATA payloads become visible under Transport synchronization without exposing the physical socket to RFB;
- RFB activity uses protected sequence-plus-armed-event rendezvous rather than blind polling or diagnostic counters;
- parser exact reads consume committed bytes incrementally and return credit for bytes actually consumed, allowing reads larger than queue capacity to progress without a queue/credit deadlock;
- outbound logical RFB writes fragment at the configured DATA payload ceiling and reuse the existing serialized physical send path;
- terminal residual discard is explicit and does not earn parser-consumption credit;
- `quiesce.c` represents the ordered finite-RFB marker process Pi REQUEST -> PS2 BOUNDARY -> Pi COMMIT -> PS2 COMPLETE while leaving the application/RFB owner responsible for deciding when a complete RFB-message boundary has actually been reached;
- resource release refuses to reclaim receiver-visible storage while a started receiver is not complete;
- `src/transport/SYMBOLS.md` now indexes the principal new runtime/channel/quiesce surface;
- a backend-independent host test for logical RFB queue capacity, wraparound, incremental consumption, atomic short exact read, and terminal residual discard was added and wired into `tests/Makefile`.

## Forensic comparison and corrections made during reconstruction

Direct comparison against the pinned H1 logical-RFB implementation exposed two implementation hazards during this shift and they were corrected before handoff:

1. the clean `rfb_channel.*` API uses `0` for success, so an initial boolean interpretation in the new runtime was corrected before this state snapshot;
2. an initial exact-read implementation waited for the entire requested count to be resident before consuming it. That would deadlock when an RFB parser exact read exceeded queue capacity because no consumed-byte credit could return. The runtime now consumes available bytes incrementally, returns credit for those parser-consumed bytes, and waits only when no committed bytes remain.

These are reconstruction-time source corrections inside the new, unqualified A001 implementation. They do not alter the preserved H1 authority or silently claim the historical post-session receive-poison defect solved.

## Validation / evidence boundary

A001 remains **not VALIDATION_READY**.

PROVISIONAL by source inspection only:

- sole higher receiver ownership for the reconstructed RFB-only runtime;
- synchronized logical RFB queue/activity rendezvous;
- parser-consumption credit distinction;
- outbound RFB fragmentation through serialized physical send;
- ordered RFB quiesce-marker state and explicit terminal residual discard;
- receiver-completion event as the resource-reclamation prerequisite.

PENDING_LOCAL:

- compile of `runtime.*`, `quiesce.c`, and changed `rfb_channel.*` under the actual PS2DEV headers/toolchain;
- execution of the newly wired `transport_rfb_channel_test` and the full host unit suite;
- canonical `scripts/check.sh`;
- strict source-dictionary definition completeness and generated portal refresh (V004 remains OPEN; the current generated portal still omits `src/transport`);
- source-topology/build integration of the new transport runtime;
- public RFB bridge/session wiring;
- canonical clean-product build, reproducibility, exact ELF identity, and PT_LOAD evidence.

HARDWARE_PENDING is not yet promoted because no coherent machine-validated reconstructed DUT exists. Historical H1 hardware evidence remains forensic evidence only.

## Known-defect accounting / remaining lifecycle work

The historical H1 receiver/mailbox poison after a finite session remains unresolved and explicitly visible. This reconstruction introduces first-class receiver completion and ordered quiesce state, but no runtime or hardware evidence yet proves that historical failure mode eliminated. No `MSG_DONTWAIT` workaround or generic timeout was silently introduced.

The reconstructed receiver currently remains a blocking physical receiver until the peer/physical stream terminates. A001 still needs coherent session-close/error convergence and public bridge/lifecycle integration before teardown can be judged complete. The provisional `transport.h` interface is not yet wired to `runtime.*` and remains subject to the audited CONFIG/lifecycle requirements rather than guessed defaults.

## Exact next pickup

Continue `a001-sole-receiver` from `runtime.*`, `quiesce.c`, and the new host channel test. First compile/run the backend-independent channel test and run the available canonical checks on an execution-capable surface; resolve any source defects without weakening the audit contract. Then reconstruct the public Transport/RFB bridge and session lifecycle around the runtime, preserving raw-socket privacy, complete-RFB-message safe-boundary authority, explicit receiver completion before resource reclamation, and the known-defect boundary. Resolve V004 definition-level/generated-portal/topology completeness and build integration before any `VALIDATION_READY` handoff. Do not begin A002 while A001 remains incoherent.

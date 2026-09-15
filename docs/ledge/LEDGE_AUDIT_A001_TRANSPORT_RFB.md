# Ledge Semantic Audit A001 — Shared PSTV Transport + Logical RFB

DOCUMENT=LEDGE_AUDIT_A001_TRANSPORT_RFB
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T14:20:00-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
DERIVED_FROM_SEMANTIC_AUDIT_REVISION=0002
TEMPORAL_CLASS=AUDIT_DISPOSITION
TEMPORAL_SEMANTICS=PRESERVES_A001_CLASSIFICATION_FROM_PRIOR_LEDGER
PIPELINE_STATUS=RECONSTRUCTION_READY

This file preserves the detailed A001 disposition previously carried inline by `LEDGE_SEMANTIC_AUDIT` revision 0002 so the current ledger can serve as a compact tranche index without losing A001 reconstruction authority.

## Required behavior

1. One physical PSTV TCP connection has one sole receiver/`recv()` owner; AUDIO, MPEG2 and RFB are logical channels dispatched by that owner.
2. Logical RFB channel 1 has CONFIG-sized queue storage, synchronization/activity rendezvous, independent credit accounting, inbound enqueue/read and outbound fragmentation through the shared send path.
3. RFB parsing remains RFB-owned; the mux seam adapts the clean RFB I/O boundary rather than implementing RFB semantics.
4. Credit is earned from parser-consumed RFB bytes. Terminal residual discard remains distinct from consumption and must not return post-commit credit falsely.
5. RFB can sleep/wake on committed producer activity without blind timer polling.
6. Finite-session shutdown is request -> boundary -> commit -> complete, and teardown cannot race a receiver still inside the counted frame's dispatch/signal path.
7. Application/presentation service occurs only at complete RFB message boundaries.
8. Flow policy may hold/select framebuffer requests and suppress presentation while parser/framebuffer authority continues advancing.

## Dispositions

| Responsibility | Historical mechanism | Clean disposition / owner | Known-defect treatment | Validation | Status |
| --- | --- | --- | --- | --- | --- |
| physical PSTV receive/send + logical dispatch | `h1_transport_runtime.*` + common protocol/queue | `SPLIT`: transport owns physical session, framing, receive/dispatch and serialized send; one bridge with process sections for consumers | preserve ordering/errors; diagnostics are not product synchronization | host framing/dispatch/sequence/error; concurrency; PS2 after PT_LOAD | `RECONSTRUCTION_READY` |
| logical RFB queue/credit/fragmentation | `h1_rfb_channel.*` | `KEEP_BEHAVIOR`: transport logical RFB mechanism | residual != consumed | queue/fragment/credit host tests | `RECONSTRUCTION_READY` |
| RFB resource allocation | `h1_rfb_runtime_resources.*` | `SIMPLIFY`: transport channel resources only when enabled | preserve RFB-OFF no-allocation | lifecycle/failure/repeated-session | `RECONSTRUCTION_READY` |
| credit policy | `h1_rfb_credit_policy.*` | `KEEP_BEHAVIOR`: transport flow-control policy | none identified | exhaustive pure policy tests | `RECONSTRUCTION_READY` |
| live channel activity/quiesce | `h1_rfb_transport_live*` | `SPLIT`: transport internals + RFB bridge process | preserve residual/quiesce ownership checks | state/integration/PS2 | `RECONSTRUCTION_READY` |
| clean RFB I/O adaptation | build-time symbol renaming + `h1_rfb_mux_io.*` | `REWRITE`: explicit RFB byte-stream boundary bound by RFB bridge to logical channel 1 | no second socket/receiver | architecture checker, fake-stream RFB tests, one-stream integration | `RECONSTRUCTION_READY` |
| RFB protocol/framebuffer/safe service | `h1_rfb_session_runtime.*` around clean RFB/framebuffer | `SPLIT`: existing RFB/framebuffer owners; application coordinates service/presentation | preserve complete-message safe boundary | RFB host + boundary-yield ordering | `RECONSTRUCTION_READY` |
| terminal receiver-dispatch fence | counter equality + bounded `DelayThread()` | `SIMPLIFY`: explicit transport dispatch/quiescence state | race prevention survives; counter/sleep form does not | deterministic interleavings + PS2 stress | `RECONSTRUCTION_READY` |
| E2xx/stats witnesses | diagnostic stages/counters | `DIAGNOSTIC_ONLY` | never hidden semantic authority | prove behavior with diagnostics absent | audit closure |

## Bridge/process boundary

Transport owns the physical PSTV socket, receiver, framing/sequence validation, serialized send and logical channel resources. Its single bridge body groups outward coordination by coherent process: RFB channel delivery, audio channel delivery, MPEG generation delivery, and session shutdown/telemetry. RFB consumes an explicit logical byte stream. Mux framing does not leak into input/UI/RFB parsing.

## Known defect / caution

The H1 counter-equality plus bounded-delay shutdown fence exposed a real receiver-dispatch ownership race. Reconstruction must preserve the invariant with explicit quiescence state; removing the fence without replacement is a behavioral regression, while retaining diagnostic counters as correctness authority is not required.

## Validation obligation

Source/machine validation and physical qualification remain separate. A001 requires host framing/queue/credit/error/interleaving tests, architecture/bridge/dependency checks, canonical build and exact PT_LOAD identity, then physical PS2 qualification for changed PT_LOAD.
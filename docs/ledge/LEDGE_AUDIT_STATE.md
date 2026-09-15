# Ledge Semantic Audit — Lane State

DOCUMENT=LEDGE_AUDIT_STATE
STATE_REVISION=0003
RECORDED_AT=2026-09-15T12:12:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0002
BASED_ON_GLOBAL_STATE_REVISION=0004
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns semantic-audit continuity only. It does not supersede the global ledge state or another lane's state.

## Authority inspected

- branch `ledge/h1-all-guns` at audit start: `1c317d429d8a6320c8e24ebf44e926565301d472`;
- forensic H1 source: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- governing reconstruction contract: revision `0001`;
- global work state consumed: revision `0004`;
- prior audit state: revision `0002`;
- A002 detailed audit: revision `0001`;
- A003 detailed audit: `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` revision `0001`.

## Current audit phase

`DEPENDENCY_CLOSURE_AND_PROCESS_CLASSIFICATION`

## Completed this lane snapshot

- Preserved A001 and A002 readiness unchanged.
- Completed coherent audit tranche A003: MPEG logical-channel ingest/decode plus CP2P exact-generation START/retire lifecycle.
- Classified event-driven MPEG empty-queue wake as required qualified behavior and rejected timer-backed hot-path polling as the production mechanism.
- Classified exact-generation START, emission lease, one-way retiring state, ordered retirement ACK, worker stop/join and residual queue/credit finalization as one reconstruction-ready lifecycle.
- Preserved the proven safe-stop invariant: owner stop is observed only after `MPEG_Picture()` returns; an active libmpeg data callback must not synthesize lifecycle EOF.
- Resolved CONFIG field 44: discard `VIDEO_IPU_RESET_EACH_SESSION` as a product toggle while preserving the known-state IPU/DMAC preparation it guarded as decoder-acquisition lifecycle.
- Resolved the common-clock arm boundary narrowly: the first real video presentation boundary arms the epoch; CONFIG, START, prefill and decode start do not.
- Classified MPEG fields 26-37 at decoder/profile granularity and deferred draw/drop/calibration ownership (38-41, 45-46, 55-56) to A004 except for exact-generation geometry consistency.
- Preserved temporal distinction between the 600-second active-runtime event-wake proof and later asynchronous safe-stop proof; no reconstructed hardware qualification is inferred.
- No reconstructed product source, reconstruction state, validation findings, or global state was modified.

## Progress counts at this snapshot

- coherent process tranches fully classified: 3 (A001, A002, A003)
- A001 responsibility/invariant rows reconstruction-ready: 8
- A002 responsibility rows reconstruction-ready: 6
- A003 reconstruction-ready responsibility groups: 7 plus 1 narrow media-clock arm contract
- CONFIG field IDs classified: 61/61; MPEG-related field 44 ambiguity resolved
- remaining seeded major process families: 4 (presentation/calibration; interaction; top-level orchestration; residual diagnostics/completeness closure)
- whole-build dependency closure: recursive route identified; exhaustive file/symbol proof still incomplete

## Unresolved questions

- Final ownership and qualified policy for scheduler/drop and draw/capture geometry remain for A004.
- Calibration acceptance, RFB suppression/matte alignment, presentation ownership transitions, cursor/OSK/local-UI layering remain unaudited.
- Interaction and top-level all-guns shutdown remain unaudited.
- Exact repeated-session qualification status across all final H1 ledge mechanisms must remain evidence-specific; do not infer it from separate active-runtime and safe-stop checkpoints.
- Exhaustive recursive build/source/symbol completeness proof remains required before whole-audit completion.

## Exact next pickup

Audit presentation/compositor/calibration as A004. Trace calibration acceptance, draw/capture/suppression geometry, RFB suppression/matte alignment, presentation ownership transitions, cursor/OSK/local-UI layering, and scheduler/drop policy. Preserve the A003 first-presentation epoch-arm contract. Do not broaden into general controller/keyboard interaction except where calibration UI input is necessary to classify presentation behavior.
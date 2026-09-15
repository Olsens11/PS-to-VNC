# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0001
RECORDED_AT=2026-09-15T10:18:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_AUDIT_STATE_REVISION=0001
BASED_ON_GLOBAL_STATE_REVISION=0002
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede the global ledge state, semantic-audit dispositions, validation findings, or the primary clean architecture authority.

## Authority inspected

- branch `ledge/h1-all-guns` at run start: `9c0ff04209b31a186a4995a49b8631adca9de957`;
- forensic H1 source authority: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- semantic audit consumed: `LEDGE_SEMANTIC_AUDIT` revision `0002` and `LEDGE_AUDIT_STATE` revision `0001`;
- governing clean architecture: `docs/CLEAN_ARCHITECTURE.md`, `ARCHITECTURE_VERSION=1`, `PRIMARY_ARCHITECTURE_AUTHORITY=YES`;
- no validation-lane state/findings existed under `docs/ledge/` at inspection time.

## Current reconstruction phase

`A001_ARCHITECTURE_GATE`

## Ready input consumed

Audit tranche A001 marks the physical PSTV transport, logical RFB channel, RFB mux adaptation, session safe-boundary behavior, and terminal receiver-dispatch ordering invariant `RECONSTRUCTION_READY`.

## Blocking authority contradiction

A001's intended clean design requires a transport component that owns the one physical PSTV socket, sole receiver, logical channel dispatch, send serialization, channel resources, and explicit quiescence state. It also requires RFB to consume an explicit logical byte-stream interface rather than own a second physical socket.

The current primary clean architecture still states that the main/application thread is the sole owner of VNC socket operations and describes RFB as owning socket/session state with direct socket/PS2IP facilities. That document explicitly predates the media/transport expansion now being audited. Creating `src/transport` or rewiring `src/rfb` before the primary architecture is reconciled would silently override the declared architecture authority.

Therefore no reconstructed product source was modified in this shift. This is an architecture gate, not a rejection of A001 semantics.

## Reconstruction design held ready behind the gate

Once architecture authority explicitly admits the all-guns transport model, reconstruct A001 in this order:

1. Introduce one transport owner for physical PSTV framing/sequence validation, sole receive/dispatch, serialized send, and lifecycle/quiescence state.
2. Introduce transport-owned logical RFB channel storage/activity/credit accounting and outbound fragmentation; allocate it only when RFB is enabled.
3. Expose an explicit RFB logical byte-stream boundary through the RFB component's single bridge process section; do not preserve H1 build-time symbol renaming or fake socket-handle binding.
4. Preserve the complete-RFB-message safe application-service boundary and authoritative CPU framebuffer ownership in the existing RFB/framebuffer domains.
5. Replace the H1 counter-equality + bounded `DelayThread()` shutdown fence with explicit receiver dispatch/quiescence state while preserving the proven ordering invariant.
6. Keep E2xx/progress witnesses diagnostic-only; production synchronization must not depend on removable counters.
7. Add/update per-directory `SYMBOLS.md`, root symbol dictionary entries, file synopses, and host tests in the same coherent source tranche.

## Known-defect treatment

No known defect is silently fixed here. The receiver-dispatch race prevention is required behavior. Only its experimental synchronization mechanism is approved for simplification after the architecture gate clears.

## Validation obligations queued

- host framing/dispatch/sequence/error tests;
- logical RFB queue/full/empty/residual/credit tests;
- RFB-disabled no-allocation lifecycle tests;
- repeated-session resource lifecycle tests;
- explicit quiescence/receiver-dispatch ordering tests;
- RFB session tests against a mux/logical-stream fake;
- architecture/include/symbol-dictionary checks;
- one-physical-stream integration evidence;
- PS2 qualification after PT_LOAD-changing integration.

## Exact next pickup

Re-read the latest primary architecture authority and audit/global states. If an intervening architecture/governance revision explicitly reconciles the all-guns transport ownership model, implement A001 as one bounded reconstruction tranche beginning with transport ownership + logical RFB channel + explicit RFB stream bridge, then run canonical host/build/docs checks and hand the result to validation. If the contradiction remains, do not create competing source architecture; keep the gate blocked and consume only any newly audit-ready work that does not conflict with current primary architecture.
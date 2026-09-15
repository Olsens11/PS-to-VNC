# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0002
RECORDED_AT=2026-09-15T11:24:00-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0001
BASED_ON_AUDIT_STATE_REVISION=0002
BASED_ON_GLOBAL_STATE_REVISION=0003
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Authority inspected

- branch at run start: `5d376fe67eecfdad8f96ee1553177f82d753a31d`;
- forensic H1 source authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0002` preserves A001 ready and adds A002 ready;
- global state revision `0003` plus `LEDGE_ARCHITECTURE_OVERLAY` revision `0001` explicitly clears the A001 shared-transport architecture gate;
- validation finding V001 remains OPEN pending reconstruction/validation; V002 remains PASS for prior gate handling.

## Current reconstruction phase

`A001_SHARED_TRANSPORT_INTERFACE_IN_PROGRESS`

## Progress this shift

The architecture gate is cleared. Reconstruction began with the smallest non-fabricated A001 source boundary: `src/transport/transport.h` and its local `SYMBOLS.md`.

The interface deliberately removes the physical socket descriptor from RFB-facing byte-stream operations and defines application-requested/transport-owned session open, quiesce, and close operations. The comments preserve the essential invariant that quiesce cannot complete while receiver dispatch may still touch channel resources and that diagnostics are not synchronization authority.

This is an interface/topology tranche only. It does **not** claim that physical framing, the sole receiver, logical RFB buffering, fragmentation, serialized send, or quiescence mechanism are implemented yet. It is intentionally not wired into the product build or RFB session yet, so no PT_LOAD or hardware claim is made from this shift.

## Incomplete topology obligations

Creating `src/transport/` triggers the clean topology policy. Before this source tranche may be considered complete, the reconstruction lane must update the active topology policy, continuity topology contract, generated dictionary portal, build/include/test paths, and relevant architecture/navigation documentation, then run the long/complete/strict dictionary gate and canonical checks. Those obligations were not silently bypassed; the new directory remains explicitly IN_PROGRESS until they are satisfied.

## A001 implementation order from here

1. Implement transport-owned physical PSTV framing/sequence validation, sole receive/dispatch, serialized send, and explicit dispatch/quiescence state using the proven H1 behavior as semantic authority.
2. Implement transport-owned logical RFB channel storage/activity/credit accounting and outbound fragmentation, allocated only when RFB is enabled.
3. Adapt the RFB component's single bridge process section to the new logical byte-stream operations; remove direct physical socket authority without preserving H1 symbol-renaming/global-bind scaffolding.
4. Complete the new-domain topology obligations in the same coherent A001 source tranche.
5. Add host framing/dispatch/sequence/error, logical queue/residual/credit, disabled-channel, repeated-session, and quiescence-order tests.
6. Run canonical source/docs/build checks and produce exact build/PT_LOAD identity before handing the tranche to validation.

## A002 handling

A002 is reconstruction-ready but is not consumed into source in this shift. Do not begin audio/profile/media-clock implementation until A001 has at least a coherent buildable transport foundation; A002's PCM logical channel should attach to that owner rather than create a parallel transport mechanism.

## Known-defect treatment

No known defect was silently fixed. The H1 counter-equality + bounded-delay shutdown mechanism remains classified as experimental evidence machinery; its receiver-dispatch ordering invariant must be preserved by explicit quiescence state when implementation lands.

## Validation status

A001 is not VALIDATION_READY. V001's governance prerequisite is satisfied, but source validation must wait for the buildable implementation tranche. No hardware qualification has occurred.

## Exact next pickup

Continue A001, not A002: inspect the proven H1 mux framing/channel implementation and current PS2 network owner, then implement the smallest coherent transport body for one physical socket, sole receive/dispatch, logical RFB buffering, serialized send, and explicit quiescence. In that same tranche satisfy the new-directory topology/dictionary/build/test obligations. Do not wire PCM/MPEG until their transport-facing behavior is consumed deliberately from ready audit tranches.
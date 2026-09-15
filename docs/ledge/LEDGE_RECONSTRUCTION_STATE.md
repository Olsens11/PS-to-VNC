# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0003
RECORDED_AT=2026-09-15T16:13:18-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0002
BASED_ON_AUDIT_STATE_REVISION=0006
BASED_ON_GLOBAL_STATE_REVISION=0007
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Authority inspected

- branch at shift start: `a2b4dbf27dc9897a49480edcfceddce38b51e8b0`;
- branch before this state write: `cf4f10a45ca33300314f5a177142473e2eef3721`;
- forensic H1 source authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0006` marks A001-A006 reconstruction-ready;
- global state revision `0007` keeps A001 as the active dependency foundation;
- validation state revision `0002` / V003 still waits for a coherent A001 implementation/build/test handoff.

The repository connector exposes committed GitHub authority but not external Pi-local dirty state. This shift neither overwrote nor declared absent any unseen local work.

## Current reconstruction phase

`A001_SHARED_TRANSPORT_FRAMING_IN_PROGRESS`

## Progress

A001 now contains the first implementation body behind the previously declared transport interface: `src/transport/protocol.h` and `src/transport/protocol.c` reconstruct the proven PSTV v1 fixed-header vocabulary, stable logical channel identities, 16-byte header, 8192-byte payload ceiling, big-endian integer encoding, magic/version validation, and payload-length validation.

This is deliberately backend-independent framing logic. It does not own sockets, dispatch, queues, RFB parsing, media policy, or threading. The implementation is reconstructed from the proven portable H1 framing authority rather than copying H1 runtime scaffolding. `src/transport/SYMBOLS.md` now indexes the added framing symbols.

## Behavioral parity / defect treatment

The wire values and validation behavior are preserved. No known defect was silently corrected. H1 diagnostic counters/stages, symbol-renaming adapters, and shutdown delay/counter machinery were not introduced into this pure framing layer because they are not framing semantics.

## Validation status

A001 remains **not VALIDATION_READY**. This shift did not wire the new framing files into the clean product build and did not implement the sole physical receiver, sequence-state enforcement, logical RFB queue/credit/fragmentation, serialized send, dispatch/quiescence state, or RFB bridge adaptation.

PENDING_LOCAL: canonical clean-product build, source/dictionary checks that require the repository's execution environment, exact ELF/PT_LOAD identity, and any PS2DEV-dependent checks. No such check is claimed PASS from repository inspection alone.

HARDWARE_PENDING: none yet as a handoff state because A001 has not reached machine-validation completion; historical H1 hardware evidence remains forensic only.

## Commits in this shift so far

- `4d92cac4860259ed6956858eaedd16ac3e60dc09` — Reconstruction B STARTED_AT log event;
- `259be4a074bd6c84f9822f0ad06551b991055011` — reconstructed PSTV framing contract;
- `5aaf4b07702c4f85ae5cfc66e9d144889fc215fc` — reconstructed portable framing implementation;
- `cf4f10a45ca33300314f5a177142473e2eef3721` — transport symbol dictionary update.

## Work remaining

Continue A001 only: implement transport-owned physical socket/session state, direction-local sequence validation, sole receive/dispatch, serialized send, logical RFB queue/activity/credit accounting and outbound fragmentation, explicit dispatch/quiescence state, RFB bridge adaptation, topology/build/test integration, and host tests. Then run canonical checks and produce exact build/PT_LOAD evidence before `VALIDATION_READY`.

## Exact next pickup

Start from `src/transport/protocol.*` plus `transport.h`. Reconstruct the smallest transport runtime body that owns one adopted physical socket and serialized framed sends, then add sole receive/sequence validation and logical RFB dispatch without exposing the socket to RFB. Preserve the receiver-dispatch quiescence invariant explicitly; do not reproduce diagnostic counters as synchronization authority and do not begin A002 media transport in parallel with an incomplete A001 foundation.

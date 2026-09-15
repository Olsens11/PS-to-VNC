# Ledge Reconstruction — Lane State

DOCUMENT=LEDGE_RECONSTRUCTION_STATE
STATE_REVISION=0004
RECORDED_AT=2026-09-15T16:47:13-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0003
BASED_ON_AUDIT_STATE_REVISION=0006
BASED_ON_GLOBAL_STATE_REVISION=0008
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This lane state owns reconstruction/integration continuity only. It does not supersede global state, audit dispositions, validation findings, or governance authority.

## Authority inspected

- interactive shift branch authority at start: `f056ad828f4d520d9bca0eae350fe16240bdf82b`;
- branch before this state write: `ed6d929b24f1636603905926f93bc81051cb3e4c`;
- forensic H1 source authority remains `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`;
- audit state revision `0006` marks A001-A006 reconstruction-ready;
- global state revision `0008` keeps A001 as the active dependency foundation;
- validation state revision `0003` keeps V003 open while A001 remains incomplete.

The repository connector exposes committed GitHub authority but not external Pi-local dirty state. This shift neither overwrote nor declared absent any unseen local work.

## Current reconstruction phase

`A001_PHYSICAL_STREAM_SEND_IN_PROGRESS`

## Progress

A001 now contains the backend-independent PSTV framing layer plus the first transport-owned physical stream runtime unit. `src/transport/physical_stream.h` and `src/transport/physical_stream.c` own one adopted physical socket, a single send semaphore, and the next outbound PSTV sequence. All framed sends pass through one serialized header-plus-payload transaction; partial socket sends are completed before progress is accepted, and the outbound sequence advances only after the complete frame is committed through the socket API.

Socket ownership transfers only after send-lock creation succeeds. Release closes the adopted descriptor and deletes the send semaphore, with an explicit contract that the higher transport runtime must prove receiver/dispatch quiescence before release once receive exists. The descriptor remains transport-internal and is not exposed to RFB. `src/transport/SYMBOLS.md` indexes the new physical-stream symbols.

This small subtranche reconstructs the proven H1 physical-send ordering without carrying over H1 diagnostic counters/stages or media/RFB policy.

## Behavioral parity / defect treatment

The H1 send invariant is preserved: one serialized physical send path; exact-send loops for header and payload; sequence begins at 1 and advances only after complete frame send. A semaphore failure is still treated as send failure. No known defect was silently corrected.

The historical receiver-dispatch shutdown race is not claimed solved by this send-only unit. Its required explicit quiescence replacement remains outstanding and release is documented as requiring that higher-level proof before resource reclamation.

## Validation status

A001 remains **not VALIDATION_READY**. This shift did not implement sole receive, inbound sequence enforcement, logical channel dispatch, logical RFB queue/activity/credit/residual behavior, outbound RFB fragmentation, explicit receiver-dispatch quiescence, RFB bridge adaptation, or build/test integration.

PENDING_LOCAL: compile/build of the new physical-stream unit, canonical `scripts/check.sh`, strict source-dictionary/topology checks, clean-product build integration, exact ELF/PT_LOAD identity, and PS2DEV-dependent checks. No such check is claimed PASS from repository inspection alone.

HARDWARE_PENDING: none yet as a handoff state because A001 has not reached machine-validation completion; historical H1 hardware evidence remains forensic only.

## Commits in this interactive shift

- `ef37c18cf35fe58f1b696c14d82dcb8248405392` — interactive Reconstruction STARTED_AT log event;
- `e7d2196198391d3d17443fb8c7103f4278f46ab7` — transport physical-stream internal contract;
- `6de2203a6af44b8488c3241125a59a61947bd0a0` — serialized physical framed-send implementation;
- `ed6d929b24f1636603905926f93bc81051cb3e4c` — transport symbol dictionary update.

## Work remaining

Continue A001 only: add sole physical receive with direction-local sequence validation and logical dispatch, then logical RFB queue/activity/credit accounting and outbound fragmentation, explicit receiver-dispatch quiescence, RFB bridge adaptation, topology/build/test integration, and host tests. Then run canonical checks and produce exact build/PT_LOAD evidence before `VALIDATION_READY`.

## Exact next pickup

Start from `physical_stream.*`, `protocol.*`, and `transport.h`. Reconstruct the smallest sole-receiver path that reads and validates complete PSTV frames, enforces the expected inbound sequence, and dispatches channel 1 into transport-owned logical RFB storage without exposing the physical socket. Preserve explicit receiver-dispatch quiescence as a first-class lifecycle invariant; do not use diagnostic counters as synchronization authority and do not begin A002 media transport while A001 remains incoherent.

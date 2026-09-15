# Ledge Reconstruction — Current Work State

DOCUMENT=LEDGE_WORK_STATE
STATE_REVISION=0006
RECORDED_AT=2026-09-15T14:33:01-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0005
SUPERSEDES_STATE_REVISION=0005
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

`SOURCE_COMMIT=SELF` means this snapshot is authoritative as committed in the Git commit containing this file; repository branch/commit authority supplies the exact SHA.

This file is a point-in-time synthesis of the latest valid lane states. Earlier state revisions remain historical evidence and are not present-state authority when superseded here.

## Authority

- Ledge branch: `ledge/h1-all-guns`
- Branch authority inspected before governance writes: `b86baab399c26c7ded5816c69491c8c10e44c128`
- Branch ancestry from prior global-state commit `a65d483866ba795bb794c3cda08a25d813bb75c6`: fast-forward/ahead by 9 commits, behind by 0.
- Forensic H1 starting commit: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`
- Governing reconstruction contract: `LEDGE_RECONSTRUCTION_CONTRACT` revision `0001`
- Base clean architecture: `docs/CLEAN_ARCHITECTURE.md` version 1
- Governing ledge architecture amendment: `LEDGE_ARCHITECTURE_OVERLAY` revision `0001`
- Prior global state synthesized: revision `0005`
- Audit state synthesized: revision `0005`
- Reconstruction state synthesized: revision `0002`
- Validation state synthesized: revision `0002`

## Current phase

`PIPELINED_AUDIT_AND_A001_RECONSTRUCTION`

## Reconciled lane progress

Audit advanced from revision 0003, which global 0005 had synthesized, through revision 0004/A004 and revision 0005/A005. A001-A005 are now explicitly reconstruction-ready. A004 classifies calibration/presentation geometry, ownership promotion, shared compositor/GS ownership, exact-generation RFB suppression/restoration, and qualified scheduler/drop behavior. A005 classifies physical input observation versus semantic ownership, main-thread RFB publication, pointer/keyboard/OSK behavior, foreground handoff/quarantine, parser-safe interaction servicing, and fail-closed input-worker shutdown. Audit next owns A006 top-level orchestration/shutdown/recovery.

Reconstruction remains at repository-recorded state revision 0002. It has consumed only audit-ready A001 and remains in progress with `src/transport/transport.h` plus `src/transport/SYMBOLS.md`; the transport body, RFB bridge adaptation, topology/build/test integration, canonical build identity, and PT_LOAD evidence remain incomplete. No repository-recorded reconstruction state after revision 0002 changes that status. A002-A005 readiness does not authorize bypassing the active A001 dependency foundation.

Validation remains at revision 0002. V001 is resolved only as the architecture prerequisite, V002 passes prior lane discipline only, and V003 remains INFO/OPEN waiting for a coherent A001 implementation/build/test handoff. No behavioral A001 PASS, reconstructed PT_LOAD qualification, or hardware qualification exists.

## Pipeline queue at this snapshot

- `AUDIT`: A006 — top-level all-guns orchestration, CONFIG admission/activation ordering, transport/RFB/audio/MPEG startup dependencies, steady-state coordination, generation retirement, finite shutdown, END/RESULT accounting, failure convergence, repeated-session behavior, recovery policy, and resident-loop/test-control scaffolding. A007 residual diagnostics plus exhaustive dependency/source/symbol completeness remains after A006.
- `RECONSTRUCTION_READY`: A002 — qualified-profile + PCM/AUDSRV lifecycle + shared media-clock contract; A003 — MPEG ingest/decode + exact-generation START/retire + safe-stop + first-presentation arm contract; A004 — presentation/compositor/calibration ownership/geometry/scheduler contract; A005 — interaction/input composition and foreground/safe-boundary/shutdown contract. These are audit-ready but queued behind active A001 where dependency ordering requires the common transport foundation.
- `RECONSTRUCTING`: A001 — shared PSTV transport + logical RFB stream + RFB safe-boundary/quiescence semantics. Interface boundary exists; implementation/build/test tranche is incomplete.
- `VALIDATION_READY`: none.
- `PASS`: V002 gate-handling/lane discipline; provisional A001 interface ownership/SYMBOLS coverage only. Neither is behavioral A001 PASS.
- `BLOCKED`: none at governance level. V001 is validation-owned `GATE/RESOLVED`; V003 is an informational wait state.
- `HARDWARE_PENDING`: none for reconstructed product tranches because no reconstructed tranche has reached machine-validation completion with a PT_LOAD-changing DUT. Historical H1 hardware evidence remains forensic evidence only.

## Continuity and architecture checks

- Audit A001-A005 readiness records are represented as readiness, not implementation. A005 detailed disposition explicitly supplies required behavior, current mechanisms, disposition, clean owner/process/bridge, known-defect treatment, and validation obligations; audit state 0005 preserves A001-A004 readiness and records the same completeness criteria for A005: PASS.
- Reconstruction consumption remains bounded to A001, which was audit-ready and architecture-cleared before source work began: PASS.
- A002-A005 are not misread as reconstructed merely because audit has advanced: PASS.
- Validation findings are represented exactly: V001 prerequisite resolved, V002 discipline PASS, V003 coherent-tranche wait OPEN: PASS.
- Revision ancestry is temporally coherent: global 0005 -> audit 0004 -> audit 0005 -> this global 0006; reconstruction 0002 and validation 0002 are older point-in-time lane snapshots whose still-current claims are not contradicted by newer audit authority.
- Repository-visible branch ancestry from global 0005's containing commit to the pre-write branch head is fast-forward/ahead by 9 and behind by 0; no branch-history rewrite is visible.
- One-physical-PSTV-stream architecture remains authoritative under overlay 0001. A002 PCM and A003 MPEG logical channels must attach to that owner; A004/A005 cross-domain processes must not acquire physical transport ownership.
- A004 preserves one physical GS presentation owner/path. A005 preserves input transport-agnostic and GS-agnostic boundaries and main/application-thread RFB publication.
- Known defects remain explicit: A003 false-EOF decoder stop must not be reintroduced; A005 OSK unequal-row proportional navigation imperfection is preserved during structural migration; A005 worker shutdown remains fail-closed; controller continuity loss during suspension remains a hard reset boundary.
- No physical/hardware qualification is claimed from historical H1 evidence or repository-only inspection.

## Stale/conflicting state and blockers

Global revision 0005 is stale as a current pipeline snapshot because audit subsequently completed A004 and A005. Its historical claims remain true-at-recorded-time and are not rewritten here.

No cross-lane semantic contradiction is present in committed repository authority. Reconstruction state revision 0002 and validation state revision 0002 are older than audit revision 0005 but remain non-conflicting: A001 is still the active reconstruction tranche and validation still waits for its coherent handoff. Later audit readiness expands the queue; it does not silently advance reconstruction or validation stages.

The repository connector exposes committed branch authority but not an external Pi worktree's uncommitted status. Therefore this governance update makes no claim that an external/local worktree is clean; unknown dirty work must be preserved by any lane operating on that worktree.

Current practical blocker: A001 is not yet a coherent buildable transport implementation. Whole-build recursive dependency/source/symbol completeness also remains unfinished and is explicitly reserved for audit closure A007. Neither condition is a governance contradiction.

## Exact next actions by lane

### Audit
Execute A006 exactly from audit state 0005. Classify startup/session-loop ordering, CONFIG admission, transport/RFB/audio/MPEG dependencies, steady-state coordination, generation retirement, finite shutdown, END/RESULT accounting, input/media teardown, failure convergence, repeated-session behavior, and resident-loop/test-control scaffolding. Preserve the current no-generic-timeout policy for unexplained silent waits unless contrary proven authority is found. Then perform A007 residual diagnostics and exhaustive recursive dependency/source/symbol closure.

### Reconstruction
Continue A001 only until the common transport foundation is coherent: physical PSTV framing/sequence validation, sole receive/dispatch, serialized send, logical RFB storage/credit/fragmentation, explicit dispatch quiescence, RFB bridge adaptation, topology/dictionary/build/test obligations, canonical checks, and exact build/PT_LOAD identity. Then hand A001 to validation. Do not begin queued A002-A005 in a way that creates alternate transport, presentation, or interaction ownership.

### Validation
Preserve V003 while A001 remains partial. On explicit A001 `VALIDATION_READY` handoff, independently run the queued architecture, framing/sequence/error, sole-receiver/dispatch, logical RFB queue/credit/fragmentation, disabled-channel, repeated-session, quiescence-order, bridge/topology/SYMBOLS, host-test, canonical-build, reproducibility, and PT_LOAD checks. Classify any required physical PS2 qualification separately as `HARDWARE_PENDING` until operator evidence exists.

### Continuity/governance
At the next synthesis, verify branch authority and revision ancestry first. Reconcile A006/A007 audit progress, any reconstruction state after 0002, and any validation transition after 0002. Keep readiness, reconstruction, validation, machine PASS, and hardware qualification as separate stages; record any authority contradiction rather than choosing silently.

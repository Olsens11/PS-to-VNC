# Ledge Reconstruction — Current Work State

DOCUMENT=LEDGE_WORK_STATE
STATE_REVISION=0005
RECORDED_AT=2026-09-15T12:32:10-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0004
SUPERSEDES_STATE_REVISION=0004
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

`SOURCE_COMMIT=SELF` means this snapshot is authoritative as committed in the Git commit containing this file; repository branch/commit authority supplies the exact SHA.

This file is a point-in-time synthesis of the latest valid lane states. Earlier state revisions remain historical evidence and are not present-state authority when superseded here.

## Authority

- Ledge branch: `ledge/h1-all-guns`
- Branch authority inspected before governance writes: `0795390fced9139ce7bb4a1a7e205cf8df22bf00`
- Branch ancestry from the prior governance start authority `8e1b51ec9cd92230b46a71b4b33ee748bf858822`: fast-forward/ahead by 5 commits, behind by 0.
- Forensic H1 starting commit: `3426f28b93de9519ca93e5f0e0aaf8b67cfca845`
- Governing reconstruction contract: `LEDGE_RECONSTRUCTION_CONTRACT` revision `0001`
- Base clean architecture: `docs/CLEAN_ARCHITECTURE.md` version 1
- Governing ledge architecture amendment: `LEDGE_ARCHITECTURE_OVERLAY` revision `0001`
- Prior global state synthesized: revision `0004`
- Audit state synthesized: revision `0003`
- Reconstruction state synthesized: revision `0002`
- Validation state synthesized: revision `0002`

## Current phase

`PIPELINED_AUDIT_AND_A001_RECONSTRUCTION`

## Reconciled lane progress

Audit advanced from revision 0002 to 0003 against global state 0004 and completed A003. A001 and A002 readiness remain unchanged. A003 now explicitly reconstructs MPEG logical-channel ingest/decode, exact-generation START/retire, safe decoder stop, residual queue/credit finalization, known-state IPU/DMAC acquisition preparation, and the narrow first-real-presentation media-clock arm contract. The false-EOF shutdown defect is explicitly accounted for and must not be reintroduced. Audit next owns A004 presentation/compositor/calibration.

Reconstruction remains at repository-recorded state revision 0002. It has consumed only audit-ready A001 and remains in progress with `src/transport/transport.h` plus `src/transport/SYMBOLS.md`; the transport body, RFB bridge adaptation, topology/build/test integration, canonical build identity, and PT_LOAD evidence remain incomplete. No repository-recorded reconstruction state after revision 0002 changes that status.

Validation remains at revision 0002. V001 is resolved only as the architecture prerequisite, V002 passes prior lane discipline only, and V003 remains INFO/OPEN waiting for a coherent A001 implementation/build/test handoff. No behavioral A001 PASS or hardware qualification exists.

## Pipeline queue at this snapshot

- `AUDIT`: A004 — presentation/compositor/calibration: calibration acceptance, draw/capture/suppression geometry, RFB matte alignment, presentation ownership transitions, cursor/OSK/local-UI layering, scheduler/drop policy, and the concrete first-presentation epoch-arm callsite.
- `RECONSTRUCTION_READY`: A002 — qualified-profile + PCM/AUDSRV lifecycle + shared media-clock contract; A003 — MPEG ingest/decode + exact-generation START/retire + safe-stop + first-presentation arm contract. Both are ready by audit disposition but remain queued behind the active A001 transport foundation; readiness is not implementation.
- `RECONSTRUCTING`: A001 — shared PSTV transport + logical RFB stream + RFB safe-boundary/quiescence semantics. Interface boundary exists; implementation/build/test tranche is incomplete.
- `VALIDATION_READY`: none.
- `PASS`: V002 gate-handling/lane discipline; provisional A001 interface ownership/SYMBOLS coverage only. Neither is behavioral A001 PASS.
- `BLOCKED`: none at governance level. V001 is validation-owned `GATE/RESOLVED`; V003 is an informational wait state.
- `HARDWARE_PENDING`: none for reconstructed product tranches. Historical H1 hardware evidence remains forensic evidence and is not reconstructed-product qualification.

## Continuity and architecture checks

- Audit A001, A002, and A003 readiness records each carry explicit required behavior, current mechanism/disposition, clean ownership intent, defect/invariant treatment, simplification rationale, and validation obligations sufficient for staged reconstruction: PASS.
- Reconstruction consumption remains bounded to A001, which was audit-ready and architecture-cleared: PASS.
- A002/A003 are not misread as reconstructed merely because they are ready: PASS.
- Validation findings are represented exactly: V001 prerequisite resolved, V002 discipline PASS, V003 coherent-tranche wait OPEN: PASS.
- Revision ancestry is temporally coherent: global 0004 -> audit 0003; reconstruction 0002 and validation 0002 remain older lane snapshots whose still-current claims are not contradicted by newer repository authority; this global 0005 synthesizes all three.
- The current branch is a fast-forward descendant of the prior governance authority; no branch-history rewrite was detected in the repository-visible ancestry check.
- One-physical-PSTV-stream architecture remains authoritative under overlay 0001. A002 PCM and A003 MPEG logical channels must attach to that owner and must not create parallel physical transport ownership.
- A003's first-presentation epoch arm does not authorize presentation/compositor implementation before A004 classifies the concrete callsite and presentation policy.
- No physical/hardware qualification is claimed.

## Stale/conflicting state and blockers

No cross-lane semantic contradiction is present in repository authority. Global revision 0004 is stale only because audit revision 0003 completed A003 afterward; this revision supersedes that pipeline view without rewriting it.

Reconstruction state revision 0002 and validation state revision 0002 are older than audit revision 0003, but they are not contradictory: A001 remains the active reconstruction tranche and validation still waits for its coherent handoff. A002/A003 readiness does not force reconstruction to skip the active dependency foundation.

The repository connector exposes committed branch authority but not the Pi worktree's uncommitted status. Therefore this governance write makes no claim that an external/local worktree is clean; unknown local dirty work must still be preserved by any lane operating on that worktree.

Current practical blocker: A001 is not yet a coherent buildable transport implementation. Whole-build recursive dependency/source/symbol completeness also remains unfinished in audit.

## Exact next actions by lane

### Audit
Execute A004 exactly from audit state 0003. Preserve the A003 first-real-presentation epoch-arm contract and classify the concrete compositor/presentation boundary without broadening into general interaction except where calibration UI input is necessary.

### Reconstruction
Continue A001 only until the common transport foundation is coherent: physical PSTV framing/sequence validation, sole receive/dispatch, serialized send, logical RFB storage/credit/fragmentation, explicit dispatch quiescence, RFB bridge adaptation, topology/dictionary/build/test obligations, canonical checks, and exact build/PT_LOAD identity. Then hand A001 to validation. Do not begin A002/A003 by creating alternate transport mechanisms.

### Validation
Preserve V003 while A001 remains partial. On explicit A001 `VALIDATION_READY` handoff, independently run the queued architecture, framing/sequence/error, sole-receiver/dispatch, logical RFB queue/credit/fragmentation, disabled-channel, repeated-session, quiescence-order, bridge/topology/SYMBOLS, host-test, canonical-build, reproducibility, and PT_LOAD checks. Keep physical PS2 qualification separate.

### Continuity/governance
At the next synthesis, verify branch authority and revision ancestry first. Reconcile A004 audit progress, any new reconstruction state after 0002, and any validation transition after 0002. Keep A002/A003 visibly ready-but-queued until reconstruction records a safe dependency transition, and record any authority contradiction rather than choosing silently.

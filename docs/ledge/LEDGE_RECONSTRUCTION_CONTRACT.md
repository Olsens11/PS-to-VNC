# Ledge Reconstruction Contract

DOCUMENT=LEDGE_RECONSTRUCTION_CONTRACT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T09:43:34-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME

This revision records the governing reconstruction policy at the recorded time. Later valid policy revisions supersede conflicting earlier policy; earlier revisions remain authoritative evidence of the rules under which earlier work was performed.

## Objective

Reconstruct the proven H1 all-guns behavior into the clean product generation using hindsight rather than mechanically refactoring exploratory source. Preserve required behavior and evidence-supported hardware invariants while removing discovery scaffolding, redundant diagnostics, accidental coupling, and complexity that is not part of the behavior contract.

## Source authority

The forensic starting point is commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845` on `ledge/h1-all-guns`. H1 source under `experiments/media-harness-h1/` is evidence/reference until a behavior is deliberately reconstructed. Do not mutate historical evidence to make reconstruction easier.

## Reconstruction rules

1. Audit before moving behavior. Every meaningful H1 responsibility receives an explicit disposition.
2. Preserve semantics, not historical implementation shape. A many-function experimental mechanism may legitimately become a smaller mechanism when the ledger explains why.
3. Do not silently fix a known defect while performing structural reconstruction. Record defect disposition separately so behavioral preservation and bug correction remain distinguishable.
4. Prefer the smallest set of explicit owners and interfaces that makes the product easier to understand than H1.
5. A component directory is the local cooperation boundary. Files inside it may call each other directly when that is the clearest implementation.
6. Cross-component behavior is concentrated in that component's bridge body. Organize bridge code by coherent process/behavioral flow, not by destination component. Keep everything needed to understand one outward-facing process together and label its purpose, external participants, inputs/outputs, invariants, and failure consequences.
7. A bridge coordinates/translates; it must not become a second implementation of component internals.
8. Neutral stable value types and justified platform seams may cross directories without artificial forwarding layers. Do not create wrappers merely for architectural appearance.
9. Clean-generation source follows the repository naming, synopsis, comments, and directory-owned symbol-dictionary policy.
10. Source completion, machine validation, and physical hardware qualification are distinct authorities. Never infer physical success from source or machine evidence.
11. Use canonical repository tooling for established procedures. Preserve unknown dirty state and never reset/clean/discard it during catch-up.

## Worker continuity contract

Each worker begins from the newest valid `LEDGE_WORK_STATE.md`, verifies its ancestry and repository authority, reads this contract and the relevant ledgers/log entries, then resumes the exact active work item. Each worker performs bounded coherent work and stops rather than guessing across an unresolved authority conflict.

Before ending, every worker must:

- record exact work/evidence/validation and decisions in the append-only work log;
- update semantic and simplification ledgers for facts learned during the shift;
- create the next point-in-time work-state snapshot with explicit revision ancestry;
- identify incomplete work and one exact pickup point for the next worker;
- distinguish proven, inferred, provisional, and unproven claims;
- bind repository-changing work to exact commit identity where possible.

## Temporal interpretation

Temporal documents are never timeless truth. Each entry/revision says what was known, believed, completed, blocked, or governing at its timestamp. Later valid revisions proceed from earlier ones and supersede conflicting state. Earlier statements are not to be read as current merely because they were authoritative when written. When a later finding changes an earlier interpretation, preserve the earlier record and explicitly identify the superseded interpretation and new evidence.

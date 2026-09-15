# Ledge Reconstruction Contract

DOCUMENT=LEDGE_RECONSTRUCTION_CONTRACT
DOCUMENT_REVISION=0002
RECORDED_AT=2026-09-15T17:46:23-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0001
SUPERSEDES_DOCUMENT_REVISION=0001
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME

This revision records the governing reconstruction policy at the recorded time. Later valid policy revisions supersede conflicting earlier policy; earlier revisions remain authoritative evidence of the rules under which earlier work was performed.

Revision 0002 changes worker-history recording from shared append-only files to immutable one-file-per-shift records under `docs/ledge/work-log/`. This removes a demonstrated GitHub-native write-contention point without weakening temporal provenance. Pre-cutover append-only logs remain read-only historical evidence.

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

Each worker begins from the newest valid current-state authority, verifies its ancestry and repository authority, reads this contract, and then inspects the immutable work-log directory defined by `docs/ledge/work-log/README.md`.

At shift start every worker must:

- capture exact `STARTED_AT` for the eventual immutable shift record;
- list `docs/ledge/work-log/` newest-first;
- read the newest logs relevant to its role/work item plus enough recent cross-lane entries to understand current branch movement;
- reuse the exact existing `WORK_ITEM_KEY` when continuing an established task;
- inspect current branch/HEAD and current lane/global state before mutation;
- stop rather than guess across an unresolved authority conflict.

Workers do **not** append new shift history to the pre-cutover shared lane/global append-only logs. Those files are frozen historical evidence for the periods they cover.

Before ending, every worker must:

- create exactly one new immutable shift record in `docs/ledge/work-log/` using the canonical filename/content schema from `docs/ledge/work-log/README.md`;
- record exact work/evidence/validation, files/commits, state/contract revisions, pending local/hardware gates, blockers, self-pause state when applicable, and decisions in that shift record;
- update only the current-state/ledger files owned by its lane when a point-in-time state advance is actually required;
- identify incomplete work and one exact pickup point for the next worker, or explicitly record `NONE` when the work family is complete;
- distinguish proven, inferred, provisional, and unproven claims;
- bind repository-changing work to exact commit identity where possible;
- re-read branch authority immediately before creating the final immutable log record.

A worker must never rewrite, append to, rename, replace, or delete another worker's immutable shift record. If a later worker discovers an error in an earlier entry, preserve the earlier file and record the correction in the later shift's own log.

The canonical project check validates work-log naming and required metadata through `scripts/work-log-check.py`.

## Current state versus work history

Current-state snapshots and immutable work logs have different jobs:

- current state is the baton that says what is authoritative now;
- `docs/ledge/work-log/*.md` records how workers advanced, blocked, validated, or reconciled that baton;
- historical pre-cutover append-only logs remain true-at-time evidence but are no longer the write target for new shifts.

A worker should not replay the entire project diary when current state plus the newest relevant immutable logs are sufficient. Conversely, a current-state summary must not erase historical evidence.

## Temporal interpretation

Temporal documents are never timeless truth. Each entry/revision says what was known, believed, completed, blocked, or governing at its timestamp. Later valid revisions proceed from earlier ones and supersede conflicting state. Earlier statements are not to be read as current merely because they were authoritative when written. When a later finding changes an earlier interpretation, preserve the earlier record and explicitly identify the superseded interpretation and new evidence.

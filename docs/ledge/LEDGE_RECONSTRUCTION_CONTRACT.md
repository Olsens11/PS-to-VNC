# Ledge Reconstruction Contract

DOCUMENT=LEDGE_RECONSTRUCTION_CONTRACT
DOCUMENT_REVISION=0003
RECORDED_AT=2026-09-15T21:14:19-04:00
SOURCE_COMMIT=3426f28b93de9519ca93e5f0e0aaf8b67cfca845
BASED_ON_DOCUMENT_REVISION=0002
SUPERSEDES_DOCUMENT_REVISION=0002
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME

This revision records the governing reconstruction policy at the recorded time. Later valid policy revisions supersede conflicting earlier policy; earlier revisions remain authoritative evidence of the rules under which earlier work was performed.

Revision 0003 clarifies the reconstruction worker's job as a sustained engineering shift rather than a one-microtask invocation. It separates architecture granularity from shift granularity, defines the continue-work loop, and requires a real end-of-shift condition before a reconstruction worker may hand off while useful authorized work remains.

## Objective

Reconstruct the proven H1 all-guns behavior into the clean product generation using hindsight rather than mechanically refactoring exploratory source. Preserve required behavior and evidence-supported hardware invariants while removing discovery scaffolding, redundant diagnostics, accidental coupling, and complexity that is not part of the behavior contract.

A reconstruction worker is responsible for sustained forward engineering progress during its available shift window. The unit of architecture should remain small and understandable; the unit of a worker shift should normally contain multiple such checkpoints.

## Source authority

The forensic starting point is commit `3426f28b93de9519ca93e5f0e0aaf8b67cfca845` on `ledge/h1-all-guns`. H1 source under `experiments/media-harness-h1/` is evidence/reference until a behavior is deliberately reconstructed. Do not mutate historical evidence to make reconstruction easier.

## Reconstruction rules

1. Audit before moving behavior. Every meaningful H1 responsibility receives an explicit disposition.
2. Preserve semantics, not historical implementation shape. A many-function experimental mechanism may legitimately become a smaller mechanism when the ledger explains why.
3. Do not silently fix a known inherited H1 defect while performing structural reconstruction. Record defect disposition separately so behavioral preservation and bug correction remain distinguishable. This does not prohibit fixing a defect introduced by the current reconstruction itself.
4. Prefer the smallest set of explicit owners and interfaces that makes the product easier to understand than H1.
5. A component directory is the local cooperation boundary. Files inside it may call each other directly when that is the clearest implementation.
6. Cross-component behavior is concentrated in that component's bridge body. Organize bridge code by coherent process/behavioral flow, not by destination component. Keep everything needed to understand one outward-facing process together and label its purpose, external participants, inputs/outputs, invariants, and failure consequences.
7. A bridge coordinates/translates; it must not become a second implementation of component internals.
8. Neutral stable value types and justified platform seams may cross directories without artificial forwarding layers. Do not create wrappers merely for architectural appearance.
9. Clean-generation source follows the repository naming, synopsis, comments, and directory-owned symbol-dictionary policy.
10. Source completion, machine validation, and physical hardware qualification are distinct authorities. Never infer physical success from source or machine evidence.
11. Use canonical repository tooling for established procedures. Preserve unknown dirty state and never reset/clean/discard it during catch-up.

## Reconstruction shift execution model

The reconstruction worker's job is not "perform one bounded task and hand off." Its job is "use the available shift to advance the highest-priority safe reconstruction work continuously until a real stopping condition is reached."

Architecture granularity and shift granularity are deliberately different concepts:

- small files, helpers, tests, dictionary edits, and coherent commits are implementation/checkpoint units;
- they are normally checkpoints *inside* a shift, not reasons to end it;
- completing one commit does not complete the shift;
- completing one item from a handoff list does not complete the shift when the next safe item is already known.

During a reconstruction shift, repeat this loop:

1. establish current branch/lane authority and the highest-priority authorized work;
2. perform a coherent engineering increment;
3. commit or otherwise preserve that increment when appropriate;
4. re-read branch/lane authority and relevant forensic/architecture evidence;
5. correct any reconstruction-owned defect exposed by the deeper comparison when the correction is understood and safe;
6. select the next safe increment and continue;
7. only begin final handoff when a real shift-ending condition exists.

Priority when the current subtask cannot continue:

1. independent pending work in the same work item;
2. another pending substream in the same reconstruction item;
3. the highest-priority next reconstruction-ready item permitted by current global/audit dependency order;
4. stop only when none of the above is safe or authorized.

A design question is not automatically a blocker. Before stopping on an unresolved design issue, exhaust the repository authority that can safely answer or narrow it: current state, audit disposition, forensic H1 evidence, clean architecture, existing call chains/config owners, tests, and validation findings. Stop only if the remaining choice would require inventing behavior, crossing another lane's ownership, human/operator evidence, or hardware evidence.

A normal scheduled reconstruction shift should aim for roughly 20-30 minutes of substantive engineering progress when useful work remains and scheduling permits. This is a productivity expectation, never permission to fabricate timestamps or pad work. A shorter shift is valid when a genuine stopping condition is reached.

Before ending a reconstruction shift, perform a final continue-work check:

- Is there another safe step in the current subtask?
- Is there independent pending work in the same work item?
- Is there another reconstruction-ready item that current dependency authority permits and that will not collide with the sibling reconstruction worker?

If any answer is yes, continue working. Do not write the final shift log yet.

Valid early stopping conditions are limited to: the authorized work family is complete; all remaining reconstruction work is blocked by a real dependency/authority/hardware/human gate; repository or tool failure prevents safe continuation; continuing would create a material writer collision; or the remaining safe window is insufficient to begin another coherent increment without jeopardizing handoff integrity.

## Self-correction versus inherited defects

Reconstruction workers must correct defects they discover in the current reconstructed implementation when the error is reconstruction-owned, the evidence is sufficient, and the fix is safe. Examples include wrong return-value interpretation, ownership mistakes, deadlock introduced by the new design, incorrect credit/accounting behavior, or unsafe lifecycle ordering.

That is distinct from a known inherited H1 defect. An inherited defect remains visible and must not be silently behavior-changed during structural migration unless separate authority explicitly permits the change.

## Worker continuity contract

Each worker begins from the newest valid current-state authority, verifies its ancestry and repository authority, reads this contract, and then inspects the immutable work-log directory defined by `docs/ledge/work-log/README.md`.

At shift start every worker must:

- capture exact `STARTED_AT` for the eventual immutable shift record;
- list `docs/ledge/work-log/` newest-first;
- read the newest logs relevant to its role/work item plus enough recent cross-lane entries to understand current branch movement;
- reuse the exact existing `WORK_ITEM_KEY` when continuing an established task;
- inspect current branch/HEAD and current lane/global state before mutation;
- identify the highest-priority safe reconstruction work, not merely the smallest immediately visible edit;
- stop rather than guess across an unresolved authority conflict.

Workers do **not** append new shift history to the pre-cutover shared lane/global append-only logs. Those files are frozen historical evidence for the periods they cover.

Throughout the shift, branch/lane authority is re-read before repository-changing writes. If another worker advanced overlapping authority, rebase understanding and continue only if the remaining work is still safe and non-overlapping.

Before ending, every worker must:

- perform the final continue-work check from the reconstruction shift execution model above;
- create exactly one new immutable shift record in `docs/ledge/work-log/` using the canonical filename/content schema from `docs/ledge/work-log/README.md`;
- record exact work/evidence/validation, files/commits, state/contract revisions, pending local/hardware gates, blockers, self-pause state when applicable, and decisions in that shift record;
- update only the current-state/ledger files owned by its lane when a point-in-time state advance is actually required;
- identify incomplete work and one exact pickup point for the next worker, or explicitly record `NONE` when the work family is complete;
- if the shift ended in under ten minutes, record the real stopping condition and enough evidence to show why no other safe authorized reconstruction work could be progressed in the remaining window;
- distinguish proven, inferred, provisional, and unproven claims;
- bind repository-changing work to exact commit identity where possible;
- re-read branch authority immediately before creating the final immutable log record.

`STARTED_AT` and `COMPLETED_AT` must be truthful observed timestamps. Never project a future completion time, pad duration, or backfill invented elapsed time to satisfy the expected shift length.

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

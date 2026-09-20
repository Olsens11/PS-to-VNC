# Ledge Reconstruction Contract

DOCUMENT=LEDGE_RECONSTRUCTION_CONTRACT
DOCUMENT_REVISION=0006
RECORDED_AT=2026-09-20T17:47:46-04:00
SOURCE_COMMIT=SELF
BASED_ON_DOCUMENT_REVISION=0005
SUPERSEDES_DOCUMENT_REVISION=0005
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME

This revision records the governing reconstruction policy at the recorded time. Later valid policy revisions supersede conflicting earlier policy; earlier revisions remain authoritative evidence of the rules under which earlier work was performed.

Revision 0006 establishes the default execution model for Ledge Reconstruction and Foreman work: ordinary repository development is autonomous, GitHub/repository authority is the normal development surface, GitHub Actions is the normal machine-verification surface, and the user is not a substitute terminal or remote shell merely because a Pi worktree exists or is not mounted in a chat environment. User action is exceptional and must be explicitly classified as bounded local evidence or hardware work.

Revision 0005 replaces the standalone Integration/Evidence seat with a Reconstruction Foreman seat. The Foreman owns two responsibilities together: (1) assign each upcoming Reconstruction A/B shift a concrete bounded goal with objective acceptance criteria sized as a meaningful engineering tranche, and (2) use the remainder of its own shift for non-behavioral integration/evidence chores, preflight, and queue grooming. Reconstruction A/B become goal-execution seats rather than self-selecting one-microtask workers. The three seats run on a 90-minute cadence staggered by 30 minutes so a reconstruction-pipeline seat begins every half hour.

## Objective

Reconstruct the proven H1 all-guns behavior into the clean product generation using hindsight rather than mechanically refactoring exploratory source. Preserve required behavior and evidence-supported hardware invariants while removing discovery scaffolding, redundant diagnostics, accidental coupling, and complexity that is not part of the behavior contract.

The reconstruction pipeline is deliberately managed as a small engineering crew:

- the Foreman converts current authority into bounded, quantifiable shift goals and handles non-behavioral integration/evidence work;
- Reconstruction A and Reconstruction B execute product-behavior goals;
- Validation independently judges reconstructed behavior and prepared evidence;
- Continuity reconciles the global pipeline state.

Architecture units should remain small and understandable. Worker goals should be large enough to constitute a reasonable engineering shift rather than a single tiny edit.

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
9. Clean-generation source follows repository naming, synopsis, comments, and directory-owned symbol-dictionary policy.
10. Source completion, repository integration/evidence, machine validation, and physical hardware qualification are distinct authorities. Never infer physical success from source or machine evidence.
11. Use canonical repository tooling for established procedures. Preserve unknown dirty state and never reset/clean/discard it during catch-up.

## Foreman-directed shift model

Reconstruction A/B do not choose arbitrary microtasks at wake. The Foreman publishes the current bounded goal packets in `docs/ledge/LEDGE_FOREMAN_STATE.md`. Each goal packet names the target worker, expected work item, concrete objective, required deliverables, acceptance criteria, explicit non-goals/ownership boundaries, and a safe fallback or stretch target when useful.

A Foreman goal must be sized as a meaningful engineering tranche. The intended question is not "what is the smallest thing this worker can safely edit?" It is "what concrete amount of progress should a competent engineer reasonably complete in this shift if authority remains stable?" A goal may span several files and several commits when those changes form one coherent behavior objective.

Goal completion, not wall-clock padding, is the primary productivity measure. A worker that completes a well-sized bounded goal early has still delivered a legitimate shift. When safe and useful, the Foreman may include a stretch goal so early completion can produce additional progress. Timestamps must remain truthful; no worker pads work merely to consume time.

At wake, A/B must re-read current HEAD, reconstruction state, Foreman state, and newer relevant logs. If repository authority has materially invalidated the assigned goal, the worker does not blindly execute stale instructions. It preserves the Foreman's intent where possible, follows the packet's fallback if valid, and otherwise records the exact conflict/blocker rather than inventing new behavior.

A/B may correct a reconstruction-owned defect discovered while pursuing the assigned goal when the evidence is sufficient and the fix is safe. Such a correction counts toward coherent goal completion when it is necessary to deliver the acceptance criteria.

A/B should not stop after the first internal checkpoint. Small files, helper functions, test source, and individual commits remain checkpoints inside the assigned goal. The shift is complete when the goal's acceptance criteria are met, a Foreman-authorized stretch target is complete when attempted, or a genuine blocker/authority/collision/window boundary prevents further safe progress.

## Reconstruction execution policy

This section governs ordinary Reconstruction workers, interactive Reconstruction
workers, and the Reconstruction Foreman. It defines **how** authorized work is
executed; it does not broaden a packet's engineering scope or transfer behavior
ownership between roles.

### Default execution classification

For ordinary Reconstruction packets and ordinary Foreman repository work:

```text
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
```

GitHub/repository authority is the normal autonomous development surface.
Available GitHub tooling should be used directly for work such as:

- refreshing branch, commit, and history authority;
- source/history archaeology and repository document inspection;
- source, test, documentation, dictionary, and integration changes authorized
  to the current role;
- commit creation and branch/ref updates;
- workflow/run/job/log/artifact inspection;
- CI retries and repository follow-up repairs that remain inside role scope.

GitHub Actions is the canonical automated verification surface when its existing
Ledge jobs cover the claim being checked. Depending on the change, this includes
host-unit, project-check, strict dictionary audit, PS2 compile, PS2 link, linked
reproducibility, and preserved workflow/build artifacts.

The absence of a mounted Pi worktree in a worker's chat/tool environment is not
by itself a blocker and is not permission to turn the user into a general-purpose
terminal proxy.

Ordinary work such as git fetch/pull/fast-forward, repository inspection,
patching, host-test execution already represented by CI, dictionary
reconciliation, committing, pushing, and CI inspection must not be delegated to
the user merely because the worker cannot open a shell on the Pi.

### Pi execution seat

`/home/ps2/src/PS-to-VNC-ledge-manual` remains a legitimate local
development/worktree surface. Its existence does not make it the mandatory
execution backend for ordinary Reconstruction, and it does not make the user the
worker's remote shell.

A packet requires Pi-local execution only when the actual engineering question
materially depends on live Pi-local state that repository/CI authority cannot
supply.

### Explicit user-assisted classifications

When a bounded requirement genuinely needs local access the worker cannot obtain
autonomously, the step or packet must be explicitly classified as one of:

```text
USER_ASSISTED_LOCAL_EVIDENCE
USER_ASSISTED_HARDWARE
```

Examples of legitimate user-only actions include:

- physically operating the PS2;
- controller/input observations;
- television/display observations;
- cable, power, or physical-network changes;
- one bounded live-Pi observation unavailable through repository/CI authority;
- another genuinely physical/local action unavailable to the worker.

A user-assisted classification must identify the exact evidence/action required.
Ask for one coherent bounded action, then resume autonomous work. Do not silently
convert an autonomous packet into user-assisted execution.

### Missing-environment rule

If an authorized packet appears to require an environment the current worker
cannot access autonomously:

1. determine whether repository or GitHub Actions authority can satisfy the
   requirement;
2. if yes, continue autonomously through that authority;
3. if no, stop at that exact bounded requirement rather than issuing a sequence
   of user-run development commands;
4. record the infrastructure limitation and the exact unavailable evidence;
5. return the blocker to the Foreman;
6. the Foreman deliberately reclassifies or resizes the affected bounded step
   before user assistance is requested.

A missing local shell is therefore an execution/infrastructure fact, not an
automatic engineering blocker and not an automatic request for user terminal
proxy work.

### Packet-authoring requirement

Every newly published Reconstruction packet must state its execution
classification explicitly.

Unless the packet actually contains a bounded local/hardware requirement, the
Foreman publishes:

```text
EXECUTION_MODE=AUTONOMOUS_RECONSTRUCTION
USER_TERMINAL_POLICY=EXCEPTION_ONLY
PI_LOCAL_USER_PROXY_REQUIRED=NO
```

If user assistance is required, the packet must name the exact bounded step,
its classification, why repository/CI cannot provide the evidence, and what
proof returns the worker to autonomous execution.

Changing execution classification must not silently broaden or restart the
engineering packet. Scope, acceptance criteria, invariants, and non-goals remain
unchanged unless the Foreman separately and explicitly revises them.

## 90-minute staggered cadence

The reconstruction crew uses a repeating 90-minute cadence with seat starts staggered by 30 minutes. The intended phase ordering is:

- Reconstruction B at the `:30` phase anchor used for the current transition cycle;
- Foreman 30 minutes later;
- Reconstruction A 30 minutes after the Foreman;
- then Reconstruction B 30 minutes after A, repeating every 90 minutes per seat.

The exact schedule anchors are scheduler authority, but the invariant is one of these three seats begins every 30 minutes and each individual seat repeats every 90 minutes. This spacing gives the Foreman a regular opportunity to inspect the preceding worker's output and prepare quantified goals for the next A and B sessions before they wake.

## Reconstruction Foreman role

The Foreman is a planning/integration seat, not a third product-behavior writer and not Validation.

### Foreman planning ownership

At each Foreman wake:

1. read current branch HEAD, reconstruction/global/audit/validation authority, current Foreman state, and newest A/B/validation/continuity logs;
2. determine what the preceding reconstruction worker actually completed, what remains, and what dependencies changed;
3. publish/update two bounded goal packets: one for the next A shift and one for the next B shift;
4. make the goals complementary or sequential enough to avoid unnecessary writer collisions;
5. give each packet objective acceptance criteria that can be checked from repository evidence;
6. include an explicit fallback/stretch target where doing so would prevent idle capacity without violating dependency order.

Goal packets are current planning authority, not timeless instructions. The Foreman must update them when branch movement makes them stale.

### Foreman integration/evidence ownership

After goal planning, the Foreman uses the remainder of its shift for non-behavioral reconstruction chores and evidence preparation, including when applicable:

- registering/wiring host tests into canonical test entry points such as `tests/Makefile`;
- build inclusion/manifests and other linkage plumbing that does not define product behavior;
- directory `SYMBOLS.md` completeness and generated source-dictionary portal synchronization;
- `docs/development/source-topology.md` and reconstruction-related topology/checker allowlists;
- canonical repository checker/build invocation and preservation of machine-visible evidence actually executable from the worker environment;
- reproducibility/evidence scaffolding, identity manifests, and qualification-preparation metadata that do not alter DUT behavior;
- integration-only fixtures or checker support whose contents cannot change product runtime behavior;
- dependency/call-chain/config-owner/forensic preflight needed to size future A/B goals.

If those chores are temporarily exhausted, the Foreman grooms future work: inspect the next dependency-permitted tranche, identify likely seams/owners/evidence, and leave concrete bounded goal candidates without implementing the product behavior itself.

### Foreman exclusions

The Foreman must not redesign or implement protocol behavior, lifecycle policy, ownership semantics, flow-control behavior, presentation policy, parser semantics, or other DUT runtime behavior. If chores/preflight reveal a likely product-source defect, the Foreman records exact evidence and assigns/returns that correction to A/B.

The Foreman does not declare validation PASS or hardware qualification. It may run checks and preserve results; Validation owns readiness/finding dispositions.

## Reconstruction A/B role

A/B are product-behavior engineering seats. They own reconstructed DUT behavior/source: protocol mechanics, runtime/lifecycle behavior, component ownership/interfaces, flow/control semantics, application/RFB/platform behavior migration, and behavior-specific tests required to reason about the implementation.

They are subservient to the current Foreman goal packet unless newer repository authority makes that packet unsafe or stale. They should not spend a behavior shift doing routine Foreman-owned chores merely because those chores are easy to finish. Behavior-specific test source is appropriate in A/B; canonical registration, inventories, topology/checkers, portal synchronization, evidence packaging, and similar chores belong to the Foreman.

When the assigned goal contains several acceptance criteria, workers continue through internal checkpoints until those criteria are satisfied or a real blocker emerges. A one-file or one-commit result is acceptable only when that was genuinely the full bounded goal or when a documented blocker prevents the rest.

## Goal packet quality requirements

A valid A/B goal packet should be objectively assessable. It normally includes:

- `WORK_ITEM_KEY` and target worker;
- explicit execution classification, including `EXECUTION_MODE`,
  `USER_TERMINAL_POLICY`, and `PI_LOCAL_USER_PROXY_REQUIRED`, plus any exact
  bounded user-assisted step if one is genuinely required;
- current source/branch/reconstruction-state authority used to assign it;
- a plain-language objective describing the behavioral result;
- required deliverables (specific responsibilities/files/seams, not necessarily exact implementation details);
- acceptance criteria phrased as observable repository/source outcomes;
- preserved invariants and explicit non-goals;
- expected evidence or tests to create/inspect where behavior-specific;
- blocker conditions that justify stopping;
- fallback/stretch work if the primary goal completes early or one subpart blocks.

The Foreman should size goals using current complexity, recent worker throughput, dependencies, and collision risk. The goal should be substantial enough that completing it represents a reasonable shift even when it finishes earlier than expected.

## Self-correction versus inherited defects

Reconstruction workers must correct defects they discover in the current reconstructed implementation when the error is reconstruction-owned, the evidence is sufficient, and the fix is safe. Examples include wrong return-value interpretation, ownership mistakes, deadlock introduced by the new design, incorrect credit/accounting behavior, or unsafe lifecycle ordering.

That is distinct from a known inherited H1 defect. An inherited defect remains visible and must not be silently behavior-changed during structural migration unless separate authority explicitly permits the change.

## Worker continuity contract

Each worker begins from newest valid current-state authority, verifies ancestry/repository authority, reads this contract, and inspects the immutable work-log directory defined by `docs/ledge/work-log/README.md`.

At shift start every worker must:

- capture exact truthful `STARTED_AT`;
- list `docs/ledge/work-log/` newest-first;
- read newest logs relevant to its role/work item plus enough recent cross-lane entries to understand branch movement;
- reuse the exact existing `WORK_ITEM_KEY` when continuing an established task;
- inspect current branch/HEAD and current lane/global state before mutation;
- stop rather than guess across unresolved authority conflict.

A/B additionally read the newest Foreman state and their assigned goal packet before product-source work. The Foreman reads newest A/B output before assigning new goals.

Workers do not append new shift history to pre-cutover shared lane/global append-only logs. Those files are frozen historical evidence.

Throughout a shift, branch/lane authority is re-read before repository-changing writes. If another worker advanced overlapping authority, rebase understanding and continue only if remaining work is safe and non-overlapping.

Before ending, every worker must:

- create exactly one immutable shift record in `docs/ledge/work-log/` using the canonical filename/content schema from `docs/ledge/work-log/README.md`;
- record exact work/evidence, files/commits, state/contract revisions, pending local/hardware gates, blockers, and decisions;
- for A/B, state whether the assigned goal's acceptance criteria were `MET`, `PARTIAL`, or `BLOCKED`, with evidence;
- for the Foreman, record which goal packets were issued and what integration/evidence chores were completed;
- update only current-state/ledger files owned by the worker's lane when a point-in-time state advance is required;
- distinguish proven, inferred, provisional, and unproven claims;
- bind repository-changing work to exact commit identity where possible;
- re-read branch authority immediately before final immutable log creation.

`STARTED_AT` and `COMPLETED_AT` must be truthful observed timestamps. Never project future completion time, pad duration, or backfill invented elapsed time.

A worker must never rewrite, append to, rename, replace, or delete another worker's immutable shift record. If a later worker finds an error, preserve the historical file and record the correction in the later worker's own log.

The canonical project check validates work-log naming and required metadata through `scripts/work-log-check.py`.

## Current state versus work history

Current-state snapshots and immutable work logs have different jobs:

- reconstruction state is the product-behavior baton;
- Foreman state is the current quantified work plan plus Foreman/integration baton;
- `docs/ledge/work-log/*.md` records how workers executed, planned, validated, integrated, blocked, or reconciled those batons;
- historical pre-cutover append-only logs and superseded lane snapshots remain true-at-time evidence but are no longer the current planning authority.

A worker should not replay the entire project diary when current state plus newest relevant immutable logs are sufficient. Conversely, a current-state summary must not erase historical evidence.

## Temporal interpretation

Temporal documents are never timeless truth. Each entry/revision says what was known, believed, completed, blocked, or governing at its timestamp. Later valid revisions proceed from earlier ones and supersede conflicting state. Earlier statements are not current merely because they were authoritative when written. When later evidence changes an earlier interpretation, preserve the earlier record and explicitly identify the superseded interpretation and new evidence.

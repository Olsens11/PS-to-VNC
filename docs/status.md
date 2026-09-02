# Current Project Status

Temporal role: `SNAPSHOT`

Recorded at: `2026-09-02T15:01:00-04:00`

This file is the concise human-readable status snapshot **as recorded at the
time above**. It is not an eternal present-tense authority. Before repeating any
`current`, `active`, `next`, `blocked`, progress, or workstream claim from this
file, reconcile it with newer GitHub/repository/live state under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

Older versions of this file remain historical snapshots in Git history.

## Product / reconstruction principle

PS-to-VNC is the clean successor to the historical PS2VNC research project.

The governing reconstruction principle remains:

> **Rebuild PS-to-VNC as the program we would have written if we had known at
> the beginning everything the exploratory implementation taught us.**

Current clean architecture authority remains `docs/CLEAN_ARCHITECTURE.md`.
Historical/normalization-era architecture remains evidence/reference rather
than the structural blueprint for new work.

## Integrated repository authority at this snapshot

`main`:

`34b53d724dede1f3875d136a8adf59ce888fa680`

The semantic audit and clean architecture are already promoted to `main` and
remain the current integrated design foundation.

## Active workstreams at this snapshot

### Development continuity / major-investigation casebook / branch hygiene

- branch: `docs/fresh-session-reconstruction`
- PR: #16 — `Document complete fresh-session reconstruction`
- state: `ACTIVE`, draft
- purpose: fresh-session reconstruction, temporal/status semantics, major
  investigation casebook, branch/workstream index, branch lifecycle/cold-storage
  policy, and continuity improvements.

This branch is also performing the current read-only branch deobfuscation audit.
No branch may be deleted without explicit user approval for that specific branch.
The preferred retirement path is verified cold storage first.

### Clean Raspberry Pi foundation

- branch: `pi/issue5-clean-baseline`
- head: `87baebce32e3c07ffc12298d6a168ac890231cf2`
- issue: #5
- state: `ACTIVE`

This remains the current clean Pi foundation/dependency baseline.

### PS2-facing RFB endpoint lifecycle candidate

- branch: `pi/issue5-rfb-socket-activation-candidate`
- PR: #17 — `Evaluate PS2-facing RFB endpoint lifecycle`
- head observed during this snapshot: `66988ebcfb1eed57cb84f422fb84bdb6faf98952`
- state: `EVALUATING` / `CANDIDATE`

Socket activation and associated lifecycle choices are not architecture merely
because they are implemented on this branch. They require live qualification
and comparison against the conventional persistent-provider control before any
promotion.

### Clean minimal PS2 core

- branch: `reconstruct/issue7-minimal-core`
- PR: #15 — `Reconstruct minimal Raw 480p PS2↔Pi core`
- head observed during this snapshot: `e7b4a35f681101fe39e59c4b04b28740b0f536de`
- state: `ACTIVE`, draft

The clean minimal Raw/480p core remains the active Issue #7 workstream. Draft
status means its current branch state must not be described as merged `main`
authority.

### Preserved w0vncserver feasibility experiment

- branch: `archive/w0vncserver-1.16.2-experiment`
- state: `PROTECTED_ARCHIVE`

The experiment is preserved evidence, not current provider architecture.

## Major-investigation casebook at this snapshot

The casebook is being introduced on PR #16 under `docs/investigations/`.

- `MI-001` — `RESOLVED`: GS/EE HIRES HSync / `ExitHandler()` handoff.
- `MI-002` — `OPEN_NON_BLOCKING`: MTU1458 / SMAP RX corruption.
- `MI-003` — `OPEN_NON_BLOCKING`: PS2 receive burst tolerance / sender pacing.
- `MI-004` — `OPEN_NON_BLOCKING`: display-transition liveness / green-corruption
  family.
- `MI-005` — `OPEN_NON_BLOCKING`: persistent 576p low-level transition stall.

Open investigation status does not automatically block unrelated work. Resolved
cases remain relevant when they explain a current invariant.

## Branch clarity at this snapshot

The authoritative routing aid on PR #16 is:

`docs/development/BRANCH_WORKSTREAM_INDEX.md`

The branch list must be treated as a timestamped inventory, not memorized.
Several fully merged transition/audit branches are classified `SUPERSEDED` and
are candidates for later retirement review, while
`reconstruction/m1-minimal-core` still has unique-history review outstanding.
No retirement recommendation is deletion permission.

## Protected boundaries

- historical PS2VNC repository remains read-only historical/provenance authority;
- do not silently substitute PS2IP/network dependencies during unrelated work;
- do not treat exploratory providers or branch-local candidates as architecture;
- preserve machine evidence separately from physical/operator observation;
- do not reset/clean/discard dirty worktrees during context reconstruction;
- no branch deletion without branch-specific explicit user approval.

## Immediate work ordering recorded at this snapshot

1. Continue the **branch deobfuscation/read-only lifecycle audit** and clarify
   branch/workstream ownership before doing broader new-session infrastructure
   expansion.
2. Complete the special harvest/review of `reconstruction/m1-minimal-core` and
   the fully merged retirement candidates; do not delete anything.
3. Continue/finish PR #16 review and continuity/casebook integration when the
   branch picture is understood.
4. After the branch picture is clear, improve the fresh-conversation bootstrap
   further, deliberately mining the successful PS2VNC documentation/status/
   TestKit continuity system for reusable lessons.
5. Continue active Issue #5 / PR #17 and Issue #7 / PR #15 engineering according
   to their own qualification gates rather than allowing documentation cleanup
   to redefine their technical evidence.

## Freshness rule

If this timestamp is older than newer project activity, do **not** say that the
workstreams or next actions above are still current merely because this file uses
present tense.

Say instead:

> "The project status snapshot recorded at 2026-09-02T15:01:00-04:00 said ..."

Then reconcile against newer GitHub branch/PR/issue state, later status records,
and identity-valid live state before describing the present.

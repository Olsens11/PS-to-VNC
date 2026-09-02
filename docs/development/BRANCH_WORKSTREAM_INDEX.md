# Branch and Workstream Index

Status: `BRANCH_INVENTORY_SNAPSHOT`

Temporal role: `SNAPSHOT`

Recorded at: `2026-09-02T15:35:00-04:00`

This file exists so developers and fresh sessions do **not** have to remember
what every live GitHub branch means.

It is a routing/index snapshot, not architecture authority and not a timeless
statement of what is active. Before repeating a classification or head as
present-tense truth, reconcile it with current GitHub state under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

The detailed first branch-provenance audit is:

`docs/development/branch-audits/2026-09-02-live-branch-audit.md`

Branch purpose and retirement status must also be reconciled with the related
issue/PR, current project state, and `docs/development/BRANCH_LIFECYCLE.md`.

No branch listed here may be deleted merely because it is classified as
`SUPERSEDED` or becomes a cold-storage candidate. Branch deletion requires the
cold-storage/verification process and explicit user approval for that specific
branch.

## Inventory snapshot

The PR #16 branch is intentionally not given a permanent self-head: editing this
index advances that branch, so a future reader must inspect PR #16/GitHub for its
actual current head.

| Branch | Classification at snapshot | Exact head / relationship recorded at snapshot | Purpose and disposition recorded at snapshot |
| --- | --- | --- | --- |
| `main` | `MAIN_AUTHORITY` | `34b53d724dede1f3875d136a8adf59ce888fa680` | Integrated repository authority at snapshot time. |
| `docs/fresh-session-reconstruction` | `ACTIVE` | PR #16; **verify live head from GitHub** | Documentation continuity, fresh-session reconstruction, major-investigation casebook, temporal-state semantics, branch lifecycle/audit, and bootstrap improvements. |
| `pi/issue5-clean-baseline` | `ACTIVE` | `87baebce32e3c07ffc12298d6a168ac890231cf2`; 12 commits ahead of recorded `main` | Clean Raspberry Pi foundation authority for Issue #5 and base of PR #17. |
| `pi/issue5-rfb-socket-activation-candidate` | `CANDIDATE` | PR #17; `66988ebcfb1eed57cb84f422fb84bdb6faf98952`; 28 commits ahead of `pi/issue5-clean-baseline` | Evaluating PS2-facing RFB endpoint lifecycle/socket activation versus persistent-provider control. Not adopted. |
| `reconstruct/issue7-minimal-core` | `ACTIVE` | PR #15; `e7b4a35f681101fe39e59c4b04b28740b0f536de` | Clean minimal Raw/480p PS2 core and current Issue #7 authority. |
| `archive/w0vncserver-1.16.2-experiment` | `PROTECTED_ARCHIVE` | `13f0a0279f14c9d2c6ab3e00625b5c74f7607657` | Exact w0vncserver 1.16.2 feasibility experiment/binary evidence. Preserve as an archive. |
| `audit/behavioral-inventory` | `SUPERSEDED` / `COLD_STORAGE_CANDIDATE` | merged PR #12; `ea72838fd7e5a756bade0b0deb60297b4a122154`; 0 ahead of recorded `main` | Semantic-audit/clean-architecture work is fully reachable from `main`. No deletion permission implied. |
| `state/post-semantic-audit` | `SUPERSEDED` / `COLD_STORAGE_CANDIDATE` | merged PR #13; `f767a43f8bb118ad46f12cec8c509a92588a6199`; 0 ahead of recorded `main` | State-only reconciliation is fully reachable from `main`. No deletion permission implied. |
| `reboot/activate-preservation-semantic-audit` | `SUPERSEDED` / `COLD_STORAGE_CANDIDATE` | merged PR #11; `dbb82d92c2591f0eea58627257693246dbb8b17f`; 0 ahead of recorded `main` | Transitional reboot/state branch, fully merged. No deletion permission implied. |
| `reboot/intent-and-semantic-audit` | `SUPERSEDED` / `COLD_STORAGE_CANDIDATE` | merged PR #10; `64b72c90fa81e4cdbe3628323190a0d9331cb2a2`; 0 ahead of recorded `main` | Strategic reboot/intent transition, fully merged. No deletion permission implied. |
| `reconstruction/m1-minimal-core` | `SUPERSEDED` / `UNIQUE_HISTORY_HARVESTED` / `COLD_STORAGE_CANDIDATE` | `00a6a22e77a20ca830a3d2cca75abb9072900273`; 3 branch-only commits; diverged from Issue #7 | Early handshake-only M1 prototype. Its three unique commits and useful lessons have now been explicitly harvested in the branch audit. Do not cherry-pick it over current Issue #7; no deletion permission implied. |

## What is actually active at this snapshot

The branch count is larger than the number of real current directions. A fresh
session should think in workstreams:

1. **Integrated authority** — `main`.
2. **Development continuity / casebook / branch hygiene** — PR #16.
3. **Clean Pi foundation and endpoint lifecycle** — Issue #5, with
   `pi/issue5-clean-baseline` plus evaluating PR #17.
4. **Clean minimal PS2 core** — Issue #7 / PR #15.
5. **Preserved w0 feasibility evidence** — protected archive, not active product
   architecture.

Everything else in the live branch list is historical/superseded rather than a
parallel current implementation strategy.

## Retirement-review state at this snapshot

This is **not a deletion queue**.

### Fully merged cold-storage candidates

- `state/post-semantic-audit`
- `audit/behavioral-inventory`
- `reboot/activate-preservation-semantic-audit`
- `reboot/intent-and-semantic-audit`

All four had zero unique commits relative to `main` at audit time and merged PR
history. If the user later wants them removed from the live branch list, prepare
verified cold-storage bundles/manifests first and discuss each named branch.

### Unique-history branch whose harvest is now complete

- `reconstruction/m1-minimal-core`

The audit reviewed all three branch-only commits. Their durable lessons are now
recorded in:

`docs/development/branch-audits/2026-09-02-live-branch-audit.md`

The current Issue #7 branch independently supersedes the prototype with a much
stricter fixed-704x462 Raw/480p implementation, full framebuffer authority,
diagnostics/identity, reproducible build, and hardware qualification contract.

The M1 branch is therefore a **cold-storage candidate**, but it is deliberately
not called deleted or retirement-complete. A verified archive and explicit user
approval are still required before removing the live ref.

### Presumed retained

- `archive/w0vncserver-1.16.2-experiment` remains `PROTECTED_ARCHIVE` unless a
  later explicit archive-migration decision says otherwise.

## Update / freshness rule

When this index is refreshed, record a new ISO-8601 timestamp rather than
silently making an old snapshot appear timeless.

Refresh or supersede it when:

- a substantive branch/workstream is created;
- a PR is merged/closed in a way that changes branch purpose;
- a candidate is adopted/rejected/superseded;
- an archive branch is created;
- a retirement audit changes classification;
- a branch is cold-storage archived or, after explicit user approval, deleted.

Fresh-session reconstruction must still verify the current GitHub branch
inventory and related PRs. If verification occurs later than the `Recorded at`
timestamp, present this file as **the branch inventory recorded at that time**
until reconciled.
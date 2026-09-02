# Branch and Workstream Index

Status: `BRANCH_INVENTORY_SNAPSHOT`

Temporal role: `SNAPSHOT`

Recorded at: `2026-09-02T15:01:00-04:00`

This file exists so developers and fresh sessions do **not** have to remember
what every live GitHub branch means.

It is a routing/index snapshot, not architecture authority and not a timeless
statement of what is active. Before repeating a classification or head as
present-tense truth, reconcile it with current GitHub state under
`docs/development/TEMPORAL_STATE_SEMANTICS.md`.

Branch purpose and retirement status must also be reconciled with the related
issue/PR, current project state, and `docs/development/BRANCH_LIFECYCLE.md`.

No branch listed here may be deleted merely because it is classified as
`SUPERSEDED` or later becomes `SAFE_TO_RETIRE`. Branch deletion requires the
cold-storage/verification process and explicit user approval for that specific
branch.

## Inventory snapshot

The heads and relationships below were the observed branch-audit facts used to
build this snapshot. The PR #16 branch is intentionally not given a supposedly
permanent self-head: editing this very index advances that branch, so a future
reader must inspect PR #16/GitHub for its actual current head.

| Branch | Classification at snapshot | Exact head / relationship recorded at snapshot | Purpose and disposition recorded at snapshot |
| --- | --- | --- | --- |
| `main` | `MAIN_AUTHORITY` | `34b53d724dede1f3875d136a8adf59ce888fa680` | Integrated repository authority at the snapshot time. |
| `docs/fresh-session-reconstruction` | `ACTIVE` | PR #16; **verify live head from GitHub** | Documentation continuity, fresh-session reconstruction, major-investigation casebook, temporal-state semantics, branch-lifecycle policy, and this index. Intended for eventual review/promotion to `main`; not retirement material while PR #16 remains active. |
| `pi/issue5-clean-baseline` | `ACTIVE` | `87baebce32e3c07ffc12298d6a168ac890231cf2`; 12 commits ahead of the recorded `main` | Clean Raspberry Pi foundation authority for Issue #5 and base of PR #17 at this snapshot. |
| `pi/issue5-rfb-socket-activation-candidate` | `CANDIDATE` | PR #17; `66988ebcfb1eed57cb84f422fb84bdb6faf98952`; 28 commits ahead of `pi/issue5-clean-baseline` | Evaluating PS2-facing RFB endpoint lifecycle/socket activation versus persistent-provider control. Not adopted at this snapshot. |
| `reconstruct/issue7-minimal-core` | `ACTIVE` | PR #15; `e7b4a35f681101fe39e59c4b04b28740b0f536de` | Clean minimal Raw/480p PS2 core and Issue #7 workstream at this snapshot. Draft PR remained open. |
| `archive/w0vncserver-1.16.2-experiment` | `PROTECTED_ARCHIVE` | archived experiment branch, head `13f0a0279f14c9d2c6ab3e00625b5c74f7607657` | Preserves exact w0vncserver 1.16.2 feasibility experiment/binary evidence. Explicit archive branch; do not treat as ordinary cleanup material. |
| `audit/behavioral-inventory` | `SUPERSEDED` | merged PR #12; head `ea72838fd7e5a756bade0b0deb60297b4a122154`; 0 ahead / 4 behind recorded `main` | Semantic-audit/clean-architecture work was fully reachable from `main`. Candidate for eventual retirement review, but not deletion without cold-storage verification and explicit approval. |
| `state/post-semantic-audit` | `SUPERSEDED` | merged PR #13; head `f767a43f8bb118ad46f12cec8c509a92588a6199`; 0 ahead / 1 behind recorded `main` | Post-audit state reconciliation was fully reachable from `main`. Candidate for eventual retirement review only. |
| `reboot/activate-preservation-semantic-audit` | `SUPERSEDED` | merged PR #11; head `dbb82d92c2591f0eea58627257693246dbb8b17f`; 0 ahead / 35 behind recorded `main` | Transitional clean-reconstruction activation branch. Fully merged at this snapshot; preserve until retirement audit/cold storage/user approval. |
| `reboot/intent-and-semantic-audit` | `SUPERSEDED` | merged PR #10; head `64b72c90fa81e4cdbe3628323190a0d9331cb2a2`; 0 ahead / 37 behind recorded `main` | Strategic reboot/intent transition branch. Fully merged at this snapshot; preserve until retirement audit/cold storage/user approval. |
| `reconstruction/m1-minimal-core` | `SUPERSEDED` **with unique-history review required** | `00a6a22e77a20ca830a3d2cca75abb9072900273`; 3 commits ahead of recorded `main`; diverged from Issue #7 branch | Earlier M1 minimal-core reconstruction prototype. It was **not** fully merged and had three unique branch-only commits. Do not mark `SAFE_TO_RETIRE` until those commits are reviewed/harvested against the clean Issue #7 implementation and any useful evidence/design reasoning is deliberately preserved. |

## Workstreams recorded as active at this snapshot

A fresh session should use this section as historical routing help, not as proof
that these same workstreams are still active later.

1. **Integrated project authority** — `main`.
2. **Development continuity / casebook / branch hygiene** — PR #16 on
   `docs/fresh-session-reconstruction`.
3. **Clean Pi foundation** — `pi/issue5-clean-baseline`.
4. **Pi endpoint-lifecycle experiment** — PR #17 on
   `pi/issue5-rfb-socket-activation-candidate`.
5. **Clean minimal PS2 core** — PR #15 on `reconstruct/issue7-minimal-core`.
6. **Preserved w0 feasibility evidence** — protected archive branch, not active
   product direction.

At the snapshot time, the remaining branches were historical/superseded live
refs awaiting deliberate retirement review rather than parallel current
implementation directions.

## Retirement-review queue recorded at this snapshot

This section is **not a deletion queue** and may itself become stale. Re-check
GitHub before acting on it.

### Fully merged / simplest candidates at the snapshot time

- `state/post-semantic-audit`
- `audit/behavioral-inventory`
- `reboot/activate-preservation-semantic-audit`
- `reboot/intent-and-semantic-audit`

All four compared as zero commits ahead of the recorded `main`, and their PRs
were merged. They were therefore straightforward candidates for cold-storage
retirement review, but each still requires the lifecycle procedure and user
approval.

### Requires substantive harvest before any retirement recommendation

- `reconstruction/m1-minimal-core`

At the snapshot time this branch had three unique commits and was not an
ancestor of the active Issue #7 branch. Before recommending retirement:

1. inspect the three commits and their design/test rationale;
2. compare their implementation against the Issue #7 clean core;
3. preserve any useful historical lesson, rejected direction, test, or source
   that is not already represented elsewhere;
4. only then decide whether the branch becomes `SAFE_TO_RETIRE`, should become a
   protected archive, or still serves another purpose.

### Presumed retained

- `archive/w0vncserver-1.16.2-experiment` was `PROTECTED_ARCHIVE` at this
  snapshot unless a later explicit archive-migration discussion says otherwise.

## Update / freshness rule

When this index is refreshed, record a new ISO-8601 timestamp rather than
silently making an old snapshot appear timeless.

Refresh or supersede this snapshot when:

- a branch is created for a substantive workstream;
- a PR is merged/closed in a way that changes branch purpose;
- a candidate is adopted/rejected/superseded;
- an archive branch is created;
- a retirement audit changes classification;
- a branch is cold-storage archived or, after explicit user approval, deleted.

Do not let this file become a stale hand-maintained guess. Fresh-session
reconstruction must verify the current GitHub branch inventory and related PRs.
If that verification occurs later than the `Recorded at` timestamp, present the
contents of this file as **"the branch inventory recorded at that time"** until
reconciled.

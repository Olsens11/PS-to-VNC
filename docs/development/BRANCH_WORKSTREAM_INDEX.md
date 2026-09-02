# Branch and Workstream Index

Status: CURRENT BRANCH-INVENTORY AID

This file exists so developers and fresh sessions do **not** have to remember
what every live GitHub branch means.

It is a routing/index document, not architecture authority. Branch purpose and
retirement status must be reconciled with the related issue/PR, current project
state, and `docs/development/BRANCH_LIFECYCLE.md`.

No branch listed here may be deleted merely because it is classified as
`SUPERSEDED` or later becomes `SAFE_TO_RETIRE`. Branch deletion requires the
cold-storage/verification process and explicit user approval for that specific
branch.

## Current inventory — 2026-09-02

| Branch | Classification | Exact head / relationship | Purpose and disposition |
| --- | --- | --- | --- |
| `main` | `MAIN_AUTHORITY` | `34b53d724dede1f3875d136a8adf59ce888fa680` | Current integrated repository authority. |
| `docs/fresh-session-reconstruction` | `ACTIVE` | PR #16, current head `cad7634fc9c1da08e5213bcba6fcb10a121c3a25` | Documentation continuity, fresh-session reconstruction, major-investigation casebook, branch-lifecycle policy, and this index. Intended for eventual review/promotion to `main`; not retirement material while PR #16 is active. |
| `pi/issue5-clean-baseline` | `ACTIVE` | `87baebce32e3c07ffc12298d6a168ac890231cf2`; 12 commits ahead of `main` | Current clean Raspberry Pi foundation authority for Issue #5 and base of PR #17. Keep live while Pi baseline/dependency work is active. |
| `pi/issue5-rfb-socket-activation-candidate` | `CANDIDATE` | PR #17; `66988ebcfb1eed57cb84f422fb84bdb6faf98952`; 28 commits ahead of `pi/issue5-clean-baseline` | Evaluating PS2-facing RFB endpoint lifecycle/socket activation versus persistent-provider control. Not adopted; keep live through hands-on qualification/disposition. |
| `reconstruct/issue7-minimal-core` | `ACTIVE` | PR #15; `e7b4a35f681101fe39e59c4b04b28740b0f536de` | Clean minimal Raw/480p PS2 core and current Issue #7 authority. Draft PR remains open; keep live. |
| `archive/w0vncserver-1.16.2-experiment` | `PROTECTED_ARCHIVE` | archived experiment branch, head `13f0a0279f14c9d2c6ab3e00625b5c74f7607657` | Preserves exact w0vncserver 1.16.2 feasibility experiment/binary evidence. Explicit archive branch; do not treat as ordinary cleanup material. |
| `audit/behavioral-inventory` | `SUPERSEDED` | merged PR #12; head `ea72838fd7e5a756bade0b0deb60297b4a122154`; 0 ahead / 4 behind `main` | Semantic-audit/clean-architecture work is fully reachable from `main`. Candidate for eventual retirement review, but not deletion without cold-storage verification and explicit approval. |
| `state/post-semantic-audit` | `SUPERSEDED` | merged PR #13; head `f767a43f8bb118ad46f12cec8c509a92588a6199`; 0 ahead / 1 behind `main` | Post-audit state reconciliation is fully reachable from `main`. Candidate for eventual retirement review only. |
| `reboot/activate-preservation-semantic-audit` | `SUPERSEDED` | merged PR #11; head `dbb82d92c2591f0eea58627257693246dbb8b17f`; 0 ahead / 35 behind `main` | Transitional clean-reconstruction activation branch. Fully merged; preserve until retirement audit/cold storage/user approval. |
| `reboot/intent-and-semantic-audit` | `SUPERSEDED` | merged PR #10; head `64b72c90fa81e4cdbe3628323190a0d9331cb2a2`; 0 ahead / 37 behind `main` | Strategic reboot/intent transition branch. Fully merged; preserve until retirement audit/cold storage/user approval. |
| `reconstruction/m1-minimal-core` | `SUPERSEDED` **with unique-history review required** | `00a6a22e77a20ca830a3d2cca75abb9072900273`; 3 commits ahead of `main`; diverged from Issue #7 branch | Earlier M1 minimal-core reconstruction prototype. It is **not** fully merged and has three unique branch-only commits. Do not mark `SAFE_TO_RETIRE` until those commits are reviewed/harvested against the clean Issue #7 implementation and any useful evidence/design reasoning is deliberately preserved. |

## What is active right now

A fresh session should normally think in terms of these workstreams rather than
memorizing branch names:

1. **Integrated project authority** — `main`.
2. **Development continuity / casebook / branch hygiene** — PR #16 on
   `docs/fresh-session-reconstruction`.
3. **Clean Pi foundation** — `pi/issue5-clean-baseline`.
4. **Pi endpoint-lifecycle experiment** — PR #17 on
   `pi/issue5-rfb-socket-activation-candidate`.
5. **Clean minimal PS2 core** — PR #15 on `reconstruct/issue7-minimal-core`.
6. **Preserved w0 feasibility evidence** — protected archive branch, not active
   product direction.

The remaining branches are historical/superseded live refs awaiting deliberate
retirement review, not parallel current implementation directions.

## Retirement-review queue

This section is **not a deletion queue**. It records branches that are worth
reviewing for eventual cold-storage retirement after their provenance is
accounted for.

### Fully merged / simplest candidates

- `state/post-semantic-audit`
- `audit/behavioral-inventory`
- `reboot/activate-preservation-semantic-audit`
- `reboot/intent-and-semantic-audit`

All four currently compare as zero commits ahead of `main`. Their PRs are merged.
They may therefore be straightforward candidates for cold-storage retirement,
but each still requires the lifecycle procedure and user approval.

### Requires substantive harvest before any retirement recommendation

- `reconstruction/m1-minimal-core`

This branch has three unique commits and is not an ancestor of the active Issue
#7 branch. Before recommending retirement:

1. inspect the three commits and their design/test rationale;
2. compare their implementation against the Issue #7 clean core;
3. preserve any useful historical lesson, rejected direction, test, or source
   that is not already represented elsewhere;
4. only then decide whether the branch becomes `SAFE_TO_RETIRE`, should become a
   protected archive, or still serves another purpose.

### Presumed retained

- `archive/w0vncserver-1.16.2-experiment` remains `PROTECTED_ARCHIVE` unless a
  later explicit archive-migration discussion says otherwise.

## Update rule

Update this index when:

- a branch is created for a substantive workstream;
- a PR is merged/closed in a way that changes branch purpose;
- a candidate is adopted/rejected/superseded;
- an archive branch is created;
- a retirement audit changes classification;
- a branch is cold-storage archived or, after explicit user approval, deleted.

Do not let this file become a stale hand-maintained guess. Fresh-session
reconstruction must still verify the GitHub branch inventory and related PRs.
The index exists to make that verification fast and understandable, not to
replace it.

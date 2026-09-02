# PS-to-VNC Current Tool and Context Map

Status: `CURRENT_ROUTER`

This document is the clean-successor router for **where to look and what to use**
during development. It intentionally avoids embedding a volatile current-test or
branch-head block.

For mutable current workstreams and branch classifications, use the timestamped
`docs/development/BRANCH_WORKSTREAM_INDEX.md` and reconcile it with GitHub.

## Session bootstrap

| Need | Canonical authority |
| --- | --- |
| Start a development session | `AGENTS.md` |
| Complete fresh-session procedure | `docs/development/SESSION_RECONSTRUCTION.md` |
| Project intent / clean-reconstruction principle | `docs/PROJECT_INTENT.md` |
| Documentation router | `docs/README.md` |
| Mechanical compatibility index | `docs/INDEX.md` |

A new session should not invent a different bootstrap order from old chat memory.

## Current-state and freshness

| Need | Canonical authority |
| --- | --- |
| Human project-state snapshot | `docs/status.md` |
| Machine project-state snapshot | `runtime/PROJECT_STATE.env` |
| Snapshot/freshness semantics | `docs/development/TEMPORAL_STATE_SEMANTICS.md` |
| Snapshot-aware local resume report | `scripts/resume-state.sh` |
| Branch/workstream routing snapshot | `docs/development/BRANCH_WORKSTREAM_INDEX.md` |
| Deferred continuity work | `docs/development/CONTINUITY_FOLLOWUPS.md` |

`CURRENT`, `ACTIVE`, `NEXT`, `BLOCKED`, progress, or running-state claims from a
snapshot require reconciliation before present-tense reuse.

## Architecture and design decisions

| Need | Canonical authority |
| --- | --- |
| Current clean architecture | `docs/CLEAN_ARCHITECTURE.md` |
| Reconstruction philosophy | `docs/PROJECT_INTENT.md` |
| Current ADRs | `docs/adr/` |
| Semantic-audit derivation | `docs/audit/` |
| Historical normalization architecture | compatibility/historical documents routed by `docs/README.md` |

Do not promote a historical implementation detail merely because a historical
document is longer or more specific.

## Major failures and technical investigations

| Need | Canonical authority |
| --- | --- |
| Major-investigation router | `docs/investigations/README.md` |
| New-case structure | `docs/investigations/TEMPLATE.md` |
| Resolved/open technical case files | `docs/investigations/MI-*.md` |
| Copied primary historical source used by a case | `docs/investigations/source/` |

Read relevant **resolved as well as open** cases before changing a subsystem.
The GitHub issue remains working chronology; the repository case file is the
durable technical synthesis.

## Branches, worktrees, archives, and retirement

| Need | Canonical authority |
| --- | --- |
| Branch/workstream routing | `docs/development/BRANCH_WORKSTREAM_INDEX.md` + current GitHub state |
| Branch lifecycle / deletion policy | `docs/development/BRANCH_LIFECYCLE.md` |
| Detailed branch-audit records | `docs/development/branch-audits/` |

No branch deletion is implied by `SUPERSEDED`, `COLD_STORAGE_CANDIDATE`, or
`SAFE_TO_RETIRE`. Every deletion requires explicit approval for that specific
branch, normally after verified cold storage.

## Development-continuity design

| Need | Canonical authority |
| --- | --- |
| Continuity principles | `docs/development/README.md` |
| Historical PS2VNC continuity lessons and dispositions | `docs/development/PS2VNC_CONTINUITY_HARVEST.md` |
| Documentation practice | `docs/development/documentation.md` |
| Testing practice | `docs/development/testing.md` |
| Tooling practice | `docs/development/tooling.md` |
| Development lessons | `docs/development/lessons-learned.md` |

## Repository checks

| Need | Canonical tool |
| --- | --- |
| Combined successor project gate | `scripts/check.sh` |
| Development-continuity gate | `scripts/continuity-check.sh` |
| Documentation-index/coherence gate | `scripts/docs-check.sh` |
| Preserved migration/source coherence | `scripts/migration-check.sh` |
| Snapshot-aware resume report | `scripts/resume-state.sh` |

`migration-check.sh` exists because preserved migration authority still has
mechanical invariants. It is not a statement that migration-era `NEXT_ACTION`
remains the clean project's current roadmap.

## Product workstreams that are not yet merged to `main`

Do not make this router a duplicate volatile inventory of every feature branch.
For active branch-local work:

1. resolve the workstream from the current GitHub PR/issue/branch record and the
   freshness-checked branch index;
2. switch/read the exact authoritative branch without disturbing dirty state;
3. read that branch's subsystem documentation and tool dictionaries;
4. use its repository-managed build/test/deploy tools rather than recreating
   them from chat;
5. preserve branch-local evidence as branch-local until promotion.

Examples at the time this router was introduced included the clean Pi lifecycle
candidate and the clean Issue #7 PS2 core. Their exact heads and statuses belong
in GitHub and timestamped branch/state records, not in this timeless router.

## Historical/migration compatibility map

`docs/reference/FILE_AND_SERVICE_MAP.md` contains extensive migration-era file
and milestone routing accumulated before the clean-reconstruction reboot.

It remains useful historical context, but it is no longer the clean-current tool
router. When that file conflicts with this router about **current clean
navigation**, this document wins; when investigating a migration-era artifact,
the historical map may be the more relevant source.

## Router maintenance rule

Update this map when a **canonical responsibility or tool location** changes.
Do not update it merely because a branch head advances or a test result changes;
those are state/evidence updates, not routing changes.

If a fresh session repeatedly fails to discover a needed canonical tool or
context source, treat that as a routing defect and improve this map/bootstrap
rather than relying on the user to remember the missing path.
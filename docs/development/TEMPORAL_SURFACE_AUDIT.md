# Temporal Surface Audit

Status: `REVIEWED_REFERENCE`

Audit performed: `2026-09-02T15:45:00-04:00`

## Purpose

This audit identifies repository surfaces whose wording can be mistaken for
present-day status and assigns each one an explicit temporal role.

The governing rule is:

`docs/development/TEMPORAL_STATE_SEMANTICS.md`

The goal is not to timestamp every architecture invariant or every historical
test result. It is to prevent freshness-sensitive words such as `current`,
`active`, `next`, `blocked`, `running`, `in progress`, and progress percentages
from silently escaping the time at which they were true.

## Designated live/snapshot surfaces

| Surface | Temporal role | Timestamp / freshness behavior | Present-tense use |
| --- | --- | --- | --- |
| `docs/status.md` | `SNAPSHOT` | explicit `Recorded at` timestamp | only after reconciliation |
| `runtime/PROJECT_STATE.env` | `SNAPSHOT` | explicit `STATE_RECORDED_AT` | only after reconciliation |
| `docs/development/BRANCH_WORKSTREAM_INDEX.md` | `SNAPSHOT` | explicit `Recorded at`; GitHub branch/PR state must be rechecked | only after reconciliation |
| `docs/development/CONTINUITY_FOLLOWUPS.md` | `SNAPSHOT_BACKLOG` | explicit `Recorded at`; later work may complete/supersede items | only after reconciliation |
| `scripts/resume-state.sh` output | `RECONCILIATION_REPORT` | reports snapshot timestamp plus current local Git evidence | never promotes old `NEXT_ACTION` by itself |

These are the surfaces that should normally carry mutable present-state claims.

## Historical state/timeline surfaces

### `runtime/MIGRATION_STATE.env`

Temporal role: `HISTORICAL_MACHINE_SNAPSHOT`

Last historical update represented in current Git history:

`2026-08-31T02:45:58Z`

Historical source commit:

`d8ae5b13947f7ef3d250c0b18be967a10f8c76a8`

This file intentionally preserves the final migration-generation machine state,
including fields named `CURRENT_STAGE`, `CURRENT_SOURCE_HEAD`, and `NEXT_ACTION`.
Those field names mean **current within that historical migration snapshot**.
They do not describe the clean-reconstruction project today.

The successor machine snapshot explicitly routes it as:

`MIGRATION_STATE_ROLE=HISTORICAL_REFERENCE`

Do not rewrite historical values merely to make them look current.

### `docs/MIGRATION_STATE.md`

Temporal role: `HISTORICAL_CHECKPOINT_TIMELINE`

Last historical update represented in current Git history:

`2026-08-31T02:45:58Z`

The file accumulated chronological migration checkpoints. Headings such as
`Current stage`, `Current build state`, and `Next action` describe the local
checkpoint in which they were written. Later sections supersede earlier ones,
and the entire migration sequence was superseded as present-day project
direction by the strategic clean-reconstruction reboot.

Fresh sessions should not mine this file for a present `NEXT_ACTION`. Use it when
reconstructing migration history/provenance.

### `docs/PROJECT_STATE.md`

Temporal role: `HISTORICAL_CHECKPOINT_TIMELINE`

Last content update represented in current Git history:

`2026-08-30T22:35:37Z`

Historical content commit:

`e88c7bfd84ee3cf056b11ff9107aca8873cb1ce5`

This file likewise accumulated sequential migration-era `current` and `next`
statements. Its compatibility banner points readers to `docs/status.md`; the
internal present tense belongs to the recorded migration checkpoints, not to the
current clean-reconstruction project.

### `docs/reference/FILE_AND_SERVICE_MAP.md`

Temporal role: `MIGRATION_ERA_COMPATIBILITY_REFERENCE`

The file now carries an explicit banner pointing to the clean-current router:

`docs/reference/CURRENT_TOOL_AND_CONTEXT_MAP.md`

Its historical `current working source`, service, and migration-path language is
retained as chronology/reference rather than rewritten into successor truth.

### `docs/ROADMAP.md`

Temporal role: `DURABLE_DIRECTION_WITH_HISTORICAL_MIGRATION_SECTION`

The file now separates durable clean-reconstruction direction from the preserved
migration-era roadmap. Its historical `Immediate` heading and M0/M1 sequencing
are explicitly not live status.

## GitHub-native work surfaces

Issues, pull requests, branches, and commits already carry server-side chronology
and state. They do not need duplicate embedded timestamps merely to prove when a
branch last moved.

However, **our interpretation** of those surfaces can become stale. Therefore a
human-authored branch/workstream classification remains timestamped even though
GitHub can independently prove the branch head and PR activity.

## Empirical and provenance records

Evidence, hardware run directories, commit hashes, manifests, and case reports
are normally historical facts rather than mutable status. They should retain
exact dates/identities appropriate to the evidence, but they do not need to be
recast as `SNAPSHOT` merely because they are old.

Examples:

- an ELF SHA-256 observed in 2026 remains the SHA-256 that was observed;
- a hardware PASS remains a historical PASS for its exact DUT/workload;
- a resolved major case remains a durable engineering conclusion unless later
  evidence explicitly supersedes it.

Age changes the temporal **context**, not the historical fact.

## Architecture and policy documents

Architecture, project intent, development rules, and ADRs are not automatically
status snapshots. They remain active until explicitly superseded/replaced.

If such a document contains a mutable operational block (`currently running`,
`next test`, `active branch`, etc.), that block should be removed/routed to a
snapshot rather than forcing the whole architecture document to become
perishable.

## Mechanical enforcement

Current checks enforce explicit temporal metadata on the designated mutable
surfaces:

- `docs/status.md`;
- `runtime/PROJECT_STATE.env`;
- `docs/development/BRANCH_WORKSTREAM_INDEX.md`;
- `docs/development/CONTINUITY_FOLLOWUPS.md`.

`runtime/MIGRATION_STATE.env` remains deliberately byte/history-oriented
migration authority and is routed as historical rather than rewritten to today's
state.

Future status-like files should be added to this audit and to the mechanical
check if they become a source of mutable present-tense truth.

## Reader rule

When encountering the word `current` anywhere in the repository, do not assume
it means **current today**. Determine which of these it is:

1. a current architecture/policy authority that remains unsuperseded;
2. a freshness-sensitive timestamped snapshot;
3. a historical checkpoint using contemporaneous present tense;
4. a GitHub-native state that can be verified directly;
5. an accidental stale claim that should be repaired.

Only category 1, reconciled category 2, and verified category 4 may be repeated
as present-day truth without qualification.
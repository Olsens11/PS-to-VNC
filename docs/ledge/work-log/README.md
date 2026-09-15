# Ledge Immutable Worker Log Contract

DOCUMENT=LEDGE_WORK_LOG_CONTRACT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T17:45:02-04:00
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME
STATUS=OPERATIONAL

## Purpose

Worker history is stored as one immutable Markdown document per completed shift rather than by having multiple workers rewrite a shared append-only log. This removes the shared-history file as a write-contention point while preserving exact temporal/provenance evidence.

Historical lane/global append-only logs created before this cutover remain read-only historical evidence. New worker shifts must use this directory and must not append to those legacy logs merely to record a shift.

## Authoritative directory

All new worker shift records live directly under:

`docs/ledge/work-log/`

`README.md` is the contract and is not a shift record. Every other Markdown file in this directory must be one worker-shift record following the filename and content schema below.

Workers normally discover history by listing this directory newest-first, then reading the newest relevant entries. A generated human index may be added later, but no index is correctness authority and workers must not depend on one being current.

## Canonical filename

Every shift record uses exactly:

`<started-at>__<role-key>__<work-item-key>__<worker-key>.md`

where:

- `<started-at>` is the shift's exact `STARTED_AT`, made filename-safe by removing punctuation from the date, clock time, and UTC offset while retaining `T` and the offset sign;
- `<role-key>` identifies the role actually performed during the shift;
- `<work-item-key>` identifies the stable task/work item;
- `<worker-key>` identifies the worker/seat that performed the shift.

Example conversion:

`STARTED_AT=2026-09-15T17:30:12-04:00`

becomes:

`20260915T173012-0400`

Example complete names:

- `20260915T165358-0400__validation__a001-physical-send__interactive.md`
- `20260915T173012-0400__reconstruction__a001-sole-receiver__recon-b.md`
- `20260915T183000-0400__continuity__global-reconciliation__continuity.md`

The filename grammar is therefore:

`YYYYMMDDTHHMMSS+HHMM__role-key__work-item-key__worker-key.md`

or

`YYYYMMDDTHHMMSS-HHMM__role-key__work-item-key__worker-key.md`

The role, work-item, and worker keys must be lowercase ASCII slugs containing only `a-z`, `0-9`, and single hyphens between words. No spaces, underscores, uppercase letters, or free-form labels are permitted in those three filename fields.

## Stable search keys

Searchability depends on workers reusing the same canonical task key while continuing the same task.

### Role key

`ROLE_KEY` describes the job actually performed, not merely the scheduled seat name. Current examples are `audit`, `reconstruction`, `validation`, and `continuity`. Future dynamically assigned component roles may use stable keys such as `transport`, `rfb`, `audio`, `mpeg`, `presentation`, `input-ui`, `application`, or `integration` when those roles become operational.

### Work-item key

`WORK_ITEM_KEY` is the primary search handle for related shifts.

Rules:

1. If work belongs to an audited tranche, begin with the lowercase tranche ID, for example `a001-sole-receiver`, `a002-audio-clock`, or `a004-presentation`.
2. If work is centered on a validation finding, begin with the lowercase finding ID, for example `v004-symbol-dictionary`.
3. Global recurring work uses a stable `global-*` key, for example `global-reconciliation`.
4. A continuing task reuses the exact same key across shifts even when performed by another worker.
5. A materially different task receives a new key; do not rename an existing task merely for wording preference.
6. When recent relevant logs already establish a key for the same task, that existing key is authoritative unless current state explicitly supersedes the task identity.

### Worker key

`WORKER_KEY` identifies the seat/actor and remains stable across its shifts. Current scheduled examples are `recon-a`, `recon-b`, `validation`, `continuity`, and `audit`. Interactive work uses `interactive`. Future dynamic seats should receive stable worker keys rather than inventing a new label each shift.

## Shift-start read protocol

At the start of a shift, the worker must:

1. capture exact `STARTED_AT` for its eventual log record;
2. read the governing current state/contract documents;
3. list `docs/ledge/work-log/` newest-first;
4. read the newest logs relevant to its role/work item plus enough recent cross-lane logs to understand branch movement;
5. inspect current branch/HEAD authority before mutation;
6. reuse an existing canonical `WORK_ITEM_KEY` when continuing an established task.

Workers should normally read only the recent/relevant immutable entries rather than replaying the entire history. Current state remains the baton; work-log files explain how the baton arrived there.

## Shift-end write protocol

At the end of every shift, including a blocked or self-paused shift that learned something materially useful, create exactly one new shift record. Do not edit, append to, rename, replace, or delete another worker's record.

The create operation itself is intentionally concurrency-friendly: a new unique path is added rather than a shared historical file being replaced. Re-read branch authority immediately before creating the log file.

A shift record must contain at minimum:

```text
DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=<ISO-8601 local timestamp with numeric offset>
COMPLETED_AT=<ISO-8601 local timestamp with numeric offset>
ROLE_KEY=<exact filename role-key>
WORK_ITEM_KEY=<exact filename work-item-key>
WORKER_KEY=<exact filename worker-key>
STATUS=<COMPLETED|PARTIAL|BLOCKED|SELF_PAUSED|NOOP>
STARTING_BRANCH_COMMIT=<sha>
ENDING_BRANCH_COMMIT=<sha or SELF if the log commit itself is the ending authority>
SELF_PAUSED=<YES|NO>
```

The body must also state, as applicable:

- objective and authority consumed;
- exact work performed;
- exact files/commits changed;
- checks/evidence and exact results;
- `PENDING_LOCAL` / `HARDWARE_PENDING` evidence gaps;
- findings/blockers and known-defect accounting;
- state/contract revisions consumed or produced;
- exact next pickup or explicit `NONE` when the work family is complete.

A worker may update its lane's current-state snapshot when that lane owns the state, but the immutable shift log is separate from current state. State is the baton; the log is history.

## Immutability rule

After a shift record is committed, it is historical evidence and must not be rewritten to make later facts fit. If a later worker finds an error, the later worker records the correction and names the affected earlier log. Destructive history cleanup is prohibited.

## Legacy-log cutover

The pre-cutover files such as lane/global append-only logs remain valid historical evidence through their final revisions. They are frozen for worker-shift history after this contract becomes operational. Workers may read them when reconstructing earlier context, but new shift history belongs here.

## Enforcement

`scripts/work-log-check.py` validates every shift-record filename and the required metadata, including that the filename timestamp/role/work-item/worker keys exactly match the record body. `scripts/check.sh` runs that checker as part of the canonical project check.

A malformed name is therefore not merely a style issue: it is a repository check failure because inconsistent names would break deterministic worker discovery and search.

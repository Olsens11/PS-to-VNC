# Ledge Immutable Worker Log Contract

DOCUMENT=LEDGE_WORK_LOG_CONTRACT
DOCUMENT_REVISION=0004
RECORDED_AT=2026-09-16T10:45:36-04:00
TEMPORAL_CLASS=POLICY_REVISION
TEMPORAL_SEMANTICS=TRUE_AS_GOVERNING_POLICY_AT_RECORDED_TIME
STATUS=OPERATIONAL
SUPERSEDES_DOCUMENT_REVISION=0003

## Purpose

Worker history is stored as one immutable Markdown document per completed shift rather than by having multiple workers rewrite a shared append-only log. This removes the shared-history file as a write-contention point while preserving exact temporal/provenance evidence.

Historical lane/global append-only logs created before this cutover remain read-only historical evidence. New worker shifts must use this directory and must not append to those legacy logs merely to record a shift.

## Authoritative directory

All new worker shift records live directly under:

`docs/ledge/work-log/`

`README.md` is the contract and is not a shift record. Except for the exact immutable grandfathered records named below, every other Markdown file in this directory must be one worker-shift record following the filename and content schema below.

Workers normally discover history by listing this directory newest-first, then reading the newest relevant entries. A generated human index may be added later, but no index is correctness authority and workers must not depend on one being current. Individual immutable shift records are intentionally not duplicated into the shared `docs/INDEX.md`; that index contains this contract as their stable discovery route, while `scripts/work-log-check.py` validates the records themselves.

## Canonical filename

Every new shift record uses exactly:

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

## Exact immutable grandfather exceptions

Revision 0002 first resolved the contradiction between strict canonical checking and two already-committed Validation records that could not be destructively renamed or rewritten under the immutability rule. The first canonical CI execution after that policy revision exposed five additional already-frozen records with legacy or malformed schema details, which revision 0003 recorded explicitly.

A later canonical A002 integration run then exposed one additional already-committed Reconstruction record whose filename stamp does not match its recorded `STARTED_AT`. That immutable record was already part of repository history when the mismatch was discovered, so revision 0004 preserves it by exact path and frozen core metadata instead of renaming or rewriting historical evidence.

The following **eight exact existing paths only** are grandfathered as immutable legacy/malformed shift records:

- `2026-09-16T05-18-33-04-00__validation__v005-fatal-teardown__validation.md`
- `2026-09-16T06-20-13-04-00__validation__a001-sole-receiver__validation.md`
- `2026-09-16T07-27-40-04-00__continuity__global-reconciliation__continuity.md`
- `20260915T211407-0400__architecture__reconstruction-shift-contract__interactive.md`
- `20260915T232009-0400__validation__a001-sole-receiver__validation.md`
- `20260915T232241-0400__reconstruction__a001-sole-receiver__interactive-b.md`
- `20260915T233050-0400__continuity__global-reconciliation__continuity.md`
- `20260916T085615-0400__reconstruction__a002-audio-clock__interactive.md`

The checker must still prove for each grandfathered record that its readable core metadata names the expected `ROLE_KEY`, `WORK_ITEM_KEY`, `WORKER_KEY`, exact ISO `STARTED_AT`, and a valid ISO `COMPLETED_AT`. It may not require revision-0001-only metadata fields, canonical filename timestamp equality, or the revision-0001 status vocabulary from those already-frozen records.

The 07:27 Continuity record was created after revision 0002 by an older automation formatting path. The A002 Reconstruction record added by revision 0004 preserves `STARTED_AT=2026-09-16T08:33:00-04:00` while its already-frozen filename uses the `20260916T085615-0400` stamp. These inclusions are historical compatibility repairs, not permission for either formatting pattern to continue. All future workers and automations are governed by the canonical schema below.

This is not a reusable escape hatch. No later path, malformed filename, or noncanonical metadata shape is accepted through pattern matching, date ranges, role-wide exceptions, or operator discretion. Any future deviation from the canonical schema is a check failure and must be corrected before that new record is committed whenever possible; if one is nevertheless committed, a new explicit policy revision is required rather than silently extending this grandfather set.

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

A new shift record must contain at minimum:

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

The exact grandfather exceptions above demonstrate this rule: their noncanonical original paths/content remain intact, and policy/tooling records the narrowly scoped compatibility treatment rather than mutating the historical records.

## Legacy-log cutover

The pre-cutover files such as lane/global append-only logs remain valid historical evidence through their final revisions. They are frozen for worker-shift history after this contract becomes operational. Workers may read them when reconstructing earlier context, but new shift history belongs here.

## Enforcement

`scripts/work-log-check.py` validates every canonical shift-record filename and the required metadata, including that the filename timestamp/role/work-item/worker keys exactly match the record body. It separately validates the exact core metadata contract for the grandfathered immutable records listed in this revision. `scripts/check.sh` runs that checker as part of the canonical project check.

A malformed new name is therefore not merely a style issue: it is a repository check failure because inconsistent names would break deterministic worker discovery and search. Only the exact paths explicitly frozen by the current contract revision are accepted outside the canonical grammar.

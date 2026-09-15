# Interactive Architecture Shift — Immutable Worker Log Rollout

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T17:45:02-04:00
COMPLETED_AT=2026-09-15T17:47:44-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=work-log-protocol
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=839a3744397f7c04cdfe4480324b678bdf094c3c
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Replace the demonstrated shared append-only worker-log contention point with one immutable Markdown file per worker shift, while standardizing filenames so workers can deterministically discover/search related work by timestamp, role, work item, and worker identity.

## Authority consumed

- branch `ledge/h1-all-guns` starting at `839a3744397f7c04cdfe4480324b678bdf094c3c`;
- `LEDGE_RECONSTRUCTION_CONTRACT` revision 0001;
- completed semantic-audit state revision 0007;
- current reconstruction/validation/continuity authority and the latest Reconstruction B blocker report;
- active scheduled-task prompts for Reconstruction A, Reconstruction B, Validation, Continuity, and Semantic Audit.

## Work performed

- created `docs/ledge/work-log/README.md` revision 0001 as the operational immutable worker-log contract;
- standardized the filename grammar as `<started-at>__<role-key>__<work-item-key>__<worker-key>.md`;
- defined stable lowercase searchable ROLE_KEY / WORK_ITEM_KEY / WORKER_KEY rules and exact task-key reuse requirements;
- made the filename timestamp a direct filename-safe rendering of exact `STARTED_AT` so names and record bodies can be mechanically cross-checked;
- defined one immutable final record per shift and froze pre-cutover append-only lane/global logs as read-only historical evidence;
- created `scripts/work-log-check.py` to validate filename grammar, required metadata, timestamp identity, status values, and key agreement;
- wired the checker into `scripts/check.sh`;
- advanced `LEDGE_RECONSTRUCTION_CONTRACT` to revision 0002, making immutable per-shift logs governing current policy;
- updated all five active scheduled worker prompts to read the immutable log directory newest-first, reuse stable WORK_ITEM_KEY values, stop writing legacy append-only logs, and create final records using the exact same naming contract.

## Repository commits

- `6c53d0002a09688cdcc71c8e4342bb1164daf1a8` — define immutable worker-log contract;
- `916e42324ee527f3c803a7164852f1147b43c495` — add work-log filename/metadata checker;
- `22d4bf3ab1e759b96104400d2f1478eccc0f1c7e` — run work-log checker from canonical project check;
- `1e056b0a9223eb103b17feb6f8aba9c749e11d62` — advance reconstruction contract to immutable-log revision 0002;
- this commit is the first immutable shift record and therefore serves as a live schema example.

## Scheduled-worker contract updates

The active prompts now use stable worker identities:

- Reconstruction A: `ROLE_KEY=reconstruction`, `WORKER_KEY=recon-a`;
- Reconstruction B: `ROLE_KEY=reconstruction`, `WORKER_KEY=recon-b`;
- Validation: `ROLE_KEY=validation`, `WORKER_KEY=validation`;
- Continuity: `ROLE_KEY=continuity`, `WORKER_KEY=continuity`;
- Semantic Audit: `ROLE_KEY=audit`, `WORKER_KEY=audit`.

Each worker must reuse an existing WORK_ITEM_KEY when continuing the same task. Audited work begins with `a###-*`, finding-centered validation work begins with `v###-*`, and recurring global synthesis uses stable `global-*` keys.

## Checks / evidence

The checker implementation was statically reviewed against the new contract and this record was written to the exact canonical filename form. No external Pi shell/toolchain execution is claimed from this GitHub-native shift. The next canonical repository execution of `scripts/check.sh` will exercise `scripts/work-log-check.py` against this first real record.

PENDING_LOCAL: none required for the documentation/protocol semantics themselves.
HARDWARE_PENDING: not applicable.

## Concurrency consequence

A worker no longer needs to replace a growing shared history file before source work. It can read recent relevant immutable records, perform a bounded shift under current state/HEAD authority, and create its own unique log path at the end. Current-state files remain the baton; immutable shift files are history. The separate future active-claim mechanism remains a prerequisite only for true simultaneous same-component development and is not implied to be solved by this logging change.

## Exact next pickup

Allow the next scheduled workers to exercise the new protocol. Continuity should synthesize reconstruction-contract revision 0002 into current global state on its next reconciliation. Reconstruction A/B should continue A001 using a stable `a001-*` WORK_ITEM_KEY and should no longer be blocked merely by the need to append to a shared reconstruction log.

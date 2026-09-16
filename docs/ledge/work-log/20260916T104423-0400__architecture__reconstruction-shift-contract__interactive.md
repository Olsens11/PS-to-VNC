# Ledge Architecture Shift — repair frozen A002 work-log mismatch

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-16T10:44:23-04:00
COMPLETED_AT=2026-09-16T10:50:36-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=reconstruction-shift-contract
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=8dc8eb03a9dc646c8e58fa0a478f318d5d1c271b
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Repair the sole canonical project-check failure reported by the latest interactive Foreman round without rewriting immutable history or weakening work-log enforcement.

Authority consumed:

- live `ledge/h1-all-guns` starting authority `8dc8eb03a9dc646c8e58fa0a478f318d5d1c271b`;
- `AGENTS.md`, `CONTRIBUTING.md`, and `docs/CLEAN_ARCHITECTURE.md`;
- reconstruction contract revision `0005`;
- work-log contract revision `0003`;
- current `scripts/work-log-check.py`;
- frozen Reconstruction record `docs/ledge/work-log/20260916T085615-0400__reconstruction__a002-audio-clock__interactive.md`;
- Foreman state revision `0009` / immutable Foreman log identifying the work-log filename/`STARTED_AT` mismatch as the sole remaining project-check error.

The frozen Reconstruction record contains readable core authority `STARTED_AT=2026-09-16T08:33:00-04:00`, `COMPLETED_AT=2026-09-16T08:56:15-04:00`, `ROLE_KEY=reconstruction`, `WORK_ITEM_KEY=a002-audio-clock`, and `WORKER_KEY=interactive`, while its already-committed filename stamp is `20260916T085615-0400`. Under revision 0003, that immutable record could not be renamed or rewritten; the governing policy explicitly required a new exact policy revision if a later malformed record had nevertheless been committed.

## Policy/tooling repair

Two bounded non-product commits were made:

- `b1efd513e0424f217a87145c5f31e02b2dcae374` — `docs(ledge): grandfather frozen A002 log mismatch`
  - advances the work-log contract from revision `0003` to `0004`;
  - preserves the frozen A002 Reconstruction record by its exact path and exact core metadata;
  - expands the compatibility set from seven to eight exact paths only;
  - explicitly states that the exception is a historical compatibility repair and not a pattern, role-wide exception, date range, or reusable escape hatch;
  - preserves the canonical filename rule for every future shift.

- `e95335e3e6864b995633b0addea3135e310591af` — `tooling(ledge): enforce work-log policy revision 0004`
  - updates only the checker policy comment and exact `GRANDFATHERED_LOGS` table;
  - adds the one frozen A002 path with expected `ROLE_KEY=reconstruction`, `WORK_ITEM_KEY=a002-audio-clock`, `WORKER_KEY=interactive`, and `STARTED_AT=2026-09-16T08:33:00-04:00`;
  - retains the existing exact-core-metadata validation for grandfathered records and all strict canonical checks for every non-grandfathered record.

No immutable historical log was edited, renamed, replaced, or deleted. No product source, A002 Foreman packet, Validation authority, Continuity state, hardware status, or scheduled automation was changed.

## Canonical evidence

The policy-only commit `b1efd513...` triggered workflow run `35111033293`; it remained red because the checker implementation had not yet been updated. That intermediate failure was expected and was not treated as completion.

The checker commit `e95335e3...` triggered canonical ledge workflow run `35111119567`. The run completed `success`.

Exact job dispositions from that run:

- `project-check`: PASS, including the work-log checker under `scripts/check.sh`;
- `host-unit`: PASS;
- `dictionary-long`: PASS;
- `ps2-compile`: PASS;
- `ps2-link`: PASS;
- `dictionary-reconcile`: SKIPPED as expected because this was not a reconciliation trigger.

Therefore the red project-check identified by the preceding Foreman round is resolved under explicit policy revision `0004`, with immutable history preserved and no broad checker weakening.

## Boundaries

PENDING_LOCAL=NONE_FOR_THIS_POLICY_REPAIR
HARDWARE_PENDING=A001 physical PS2 qualification remains separately pending; this governance repair makes no hardware claim
PRODUCT_BEHAVIOR_CHANGED=NO
SCHEDULED_AUTOMATIONS_CHANGED=NO

## Revisions produced / consumed

CONSUMED_RECONSTRUCTION_CONTRACT_REVISION=0005
CONSUMED_WORK_LOG_CONTRACT_REVISION=0003
PRODUCED_WORK_LOG_CONTRACT_REVISION=0004
CONSUMED_FOREMAN_STATE_REVISION=0009

## Exact next pickup

NONE for this policy defect. Interactive Reconstruction may continue from the active A002 AUDIO-channel packet in current Foreman state. Foreman/Validation/Continuity should consume work-log contract revision `0004` on their next authority refresh and should no longer carry the 08:56 A002 filename mismatch as an open project-check blocker.

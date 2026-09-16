# Interactive Architecture — Foreman-directed reconstruction crew

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T21:33:01-04:00
COMPLETED_AT=2026-09-15T21:37:21-04:00
ROLE_KEY=architecture
WORK_ITEM_KEY=foreman-goal-control
WORKER_KEY=interactive
STATUS=COMPLETED
STARTING_BRANCH_COMMIT=807d3da59c2045b1022c7856d85c51d651dce6a3
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective

Replace the newly created standalone Integration/Evidence seat with a Foreman-directed reconstruction crew in which Reconstruction A and B receive substantial, objectively measurable goals for every scheduled session. Use goal completion rather than arbitrary microtask completion or artificial wall-clock consumption as the primary productivity measure, while retaining integration/evidence chores as Foreman work after planning.

## Governing changes

Advanced `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` from revision 0004 to revision 0005.

Revision 0005 establishes:

- a Reconstruction Foreman as planning/integration seat, not a third product-behavior writer;
- bounded goal packets for the next A and B sessions with required deliverables and objective acceptance criteria;
- primary goals sized as meaningful engineering tranches, normally expected to occupy roughly 20–30 minutes at recent complexity/throughput without making time a quota;
- explicit fallback/stretch targets so early completion or a blocked subpart does not automatically waste remaining capacity;
- A/B logs reporting `FOREMAN_GOAL_RESULT=MET|PARTIAL|BLOCKED` against packet criteria;
- Foreman ownership of host-test wiring, inventories, topology/checkers/portal, repository-native evidence, reproducibility scaffolding, and future-goal preflight after goal assignment;
- a 90-minute per-seat cadence with the three reconstruction-crew starts staggered by 30 minutes.

## State changes

Created `docs/ledge/LEDGE_FOREMAN_STATE.md` revision 0001 as the current quantified planning/integration baton. It records the transition cadence and requires the first Foreman wake to replace placeholders with concrete goals for A at 22:30 and B at 23:00.

Advanced `docs/ledge/LEDGE_INTEGRATION_STATE.md` to revision 0002 with `STATUS=SUPERSEDED_BY_FOREMAN`. The standalone Integration/Evidence backlog remains historical evidence and is inherited by the Foreman rather than operated as a separate seat.

## Scheduler changes

The active reconstruction crew is now scheduled as follows in `America/New_York`:

- Reconstruction B: anchor `2026-09-15T21:30:00-04:00`, repeat every 90 minutes;
- Reconstruction Foreman: anchor `2026-09-15T22:00:00-04:00`, repeat every 90 minutes;
- Reconstruction A: anchor `2026-09-15T22:30:00-04:00`, repeat every 90 minutes.

This produces one reconstruction-crew start every 30 minutes. The B shift that woke around 21:30 was already active during the transition and is explicitly treated as a pre-Foreman transition shift; the first goal-directed A/B sessions begin after the 22:00 Foreman planning wake.

The prior `Ledge Integration & Evidence` automation was repurposed in place as `Ledge Reconstruction Foreman`; no duplicate integration worker remains scheduled.

Reconstruction A/B prompts were rewritten as goal-execution seats. Validation and Continuity prompts were updated to consume `LEDGE_FOREMAN_STATE.md`, route integration/evidence gaps to the Foreman, and monitor goal-packet/result discipline rather than treating the superseded Integration state as current.

## Productivity model

The Foreman must plan before doing chores. A/B are subservient to the current packet unless newer repository authority makes it stale or unsafe. A worker that finishes a correctly sized goal early may legitimately end after completing its acceptance criteria (and any safe assigned stretch work); it does not need to pad time. Conversely, one tiny commit is no longer a plausible successful shift unless it actually satisfies the quantified packet or a real blocker prevents the remaining criteria.

This design makes shift quality measurable by promised-vs-delivered outcomes while keeping architecture/commit granularity independent from shift scope.

## Transition / concurrency note

Reconstruction B was active while this governance transition was written. Governance writes were limited to contract/state files and scheduler configuration, not B's product-source ownership. Future B invocations use the new goal-directed prompt/schedule. The first Foreman wake must inspect B's completed transition output before assigning A/B goals so it does not duplicate or contradict work completed during this changeover.

## Exact next pickup

At 22:00 the Foreman must read current branch/reconstruction/validation authority and the completed 21:30 B shift, then publish quantified packets for A 22:30 and B 23:00 before doing any integration/evidence chores. Continuity should reconcile contract revision 0005 and Foreman state into the next global snapshot.

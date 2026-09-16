# Ledge Reconstruction Foreman — Current State

DOCUMENT=LEDGE_FOREMAN_STATE
STATE_REVISION=0001
RECORDED_AT=2026-09-15T21:34:46-04:00
SOURCE_COMMIT=SELF
BASED_ON_RECONSTRUCTION_CONTRACT_REVISION=0005
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0011
BASED_ON_INTEGRATION_STATE_REVISION=0001
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This state is the current planning baton for the Reconstruction Foreman. It owns quantified A/B shift goals plus the non-behavioral integration/evidence chore queue. It does not supersede reconstruction product-source authority, audit dispositions, validation findings, or global continuity authority.

## Current foreman phase

`FOREMAN_TRANSITION_TO_GOAL_DIRECTED_90_MINUTE_CREW`

The prior standalone Integration/Evidence lane is folded into the Foreman under reconstruction contract revision 0005. `LEDGE_INTEGRATION_STATE.md` revision 0001 remains historical true-at-time evidence for the backlog inherited by the Foreman; new current planning/integration authority belongs here.

## Crew cadence

The reconstruction crew is transitioning to one seat start every 30 minutes with each seat repeating every 90 minutes:

- Reconstruction B anchor: `2026-09-15T21:30:00-04:00`, every 90 minutes;
- Foreman anchor: `2026-09-15T22:00:00-04:00`, every 90 minutes;
- Reconstruction A anchor: `2026-09-15T22:30:00-04:00`, every 90 minutes.

The B shift that began around the 21:30 transition point started before contract revision 0005/this Foreman state existed. Treat its completed output as input to the first Foreman planning shift; do not retroactively pretend it had a goal packet.

## Goal-packet protocol

At each Foreman shift, update this file with one bounded packet for the next A wake and one bounded packet for the next B wake. Each packet must include:

- target scheduled start and worker;
- exact `WORK_ITEM_KEY`;
- branch/reconstruction-state authority used to assign it;
- plain-language objective;
- required deliverables;
- objective acceptance criteria;
- preserved invariants / explicit non-goals;
- behavior-specific evidence/tests expected when appropriate;
- blocker conditions;
- fallback and/or stretch target when safe.

Goals should be sized so that completing the primary acceptance criteria represents a reasonable engineering shift, normally on the order of a 20–30 minute substantive tranche given recent complexity/throughput. They are not wall-clock quotas.

## Next goal packets

### Reconstruction A — next scheduled start

TARGET_START=2026-09-15T22:30:00-04:00
WORKER=recon-a
GOAL_STATUS=PENDING_FIRST_FOREMAN_WAKE

The Foreman must inspect the completed transition B shift plus newest validation/continuity/reconstruction authority at 22:00 and replace this placeholder with a quantified A goal before A wakes.

### Reconstruction B — next scheduled start

TARGET_START=2026-09-15T23:00:00-04:00
WORKER=recon-b
GOAL_STATUS=PENDING_FIRST_FOREMAN_WAKE

The Foreman must inspect the transition B shift and planned A goal, then assign B a complementary/sequential quantified goal that remains safe if A completes its packet.

## Inherited Foreman chore queue

Until current branch authority supersedes individual items, the Foreman inherits the integration/evidence backlog previously held by Integration state revision 0001:

1. wire current bridge behavior tests into canonical host-unit entry points when not already done by newer authority;
2. complete current RFB/Transport definition inventories;
3. adopt `src/transport` deliberately into source topology, checker/domain allowlists, and generated dictionary portal through canonical tooling;
4. run and preserve repository-native host/static evidence actually executable from the worker surface;
5. prepare build/linkage/reproducibility evidence plumbing as A001 becomes a coherent live path;
6. use remaining capacity for dependency/call-chain/config-owner/forensic preflight needed to size the next A/B goals.

The Foreman re-reads current HEAD/newest logs before acting on any chore so completed work is not duplicated.

## Current evidence boundary

A001 remains not `VALIDATION_READY` at this transition snapshot. Validation authority remains independent. No test/build/PS2DEV/ELF/PT_LOAD/hardware result is implied by creating this planning lane.

## Exact first Foreman pickup

At the 22:00 Foreman wake:

1. inspect the completed/active 21:30 Reconstruction B output and current branch/reconstruction/global/validation authority;
2. size and publish a concrete A packet for 22:30 and a concrete B packet for 23:00, with objective acceptance criteria and non-overlapping/sequential ownership;
3. then spend remaining Foreman capacity on the highest-priority still-pending integration/evidence chore from current authority;
4. leave one immutable `foreman` shift log and advance this state with the next goal epoch.

# Ledge Parallel Development Model — Future Design Note

DOCUMENT=LEDGE_PARALLEL_DEVELOPMENT_MODEL
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-15T16:14:39-04:00
TEMPORAL_CLASS=FUTURE_DESIGN_NOTE
STATUS=NOT_YET_OPERATIONAL

This document preserves a future development/workforce model to implement once the clean reconstruction has stable enough component seams. It is not current reconstruction authority and must not be used to bypass the active reconstruction contract, audit readiness gates, validation independence, or continuity authority.

## Intent

Design PS-to-VNC as a codebase that a real development team can work on coherently and simultaneously. Component boundaries should also be development-ownership boundaries. Human review should be intuitive from the source-tree layout, component contracts, and explicit seams.

The earlier "hooks and eyelets" concept is the project’s plain-language model for component APIs:

- hooks = interfaces/services a component provides;
- eyelets = interfaces/services a component requires;
- the seam contract includes more than function signatures: ownership, lifetime, thread context, blocking behavior, ordering, failure semantics, allocation/free responsibility, quiescence guarantees, and re-entry rules where relevant.

## Worker allocation model

The five scheduled-worker slots should be treated as roles/seats, not permanent identities. Roles may be reassigned as the project bottleneck moves.

Preferred future organization:

- Continuity/architecture remains mostly independent and authoritative for cross-lane reconciliation.
- Validation remains organizationally independent from implementation.
- Remaining engineering workers are dynamically assignable among reconstruction subgroups such as Transport, RFB, Audio/Media Clock, MPEG, Presentation, Input/UI, Application Orchestration, Integration, and API-contract/documentation work.

### Paired workers

Where a role benefits from extra cadence, permit at most two workers for that theoretical role.

- `MAX_WORKERS_PER_ROLE=2`
- `MAX_ACTIVE_WRITE_CLAIMS_PER_COMPONENT=1`

Paired workers should use identical engineering/safety rules but opposite primary affinities so they naturally spread across work when multiple components are ready.

Example:

- Worker A primary affinity: Transport -> RFB -> Integration, then eligible fallback work.
- Worker B primary affinity: Media -> Presentation/Input -> Integration, then eligible fallback work.

Primary affinity is a weighting, not a hard assignment. If the preferred role is blocked or claimed, the worker should choose the next eligible role rather than idle.

## Shift-start dispatch policy

At the beginning of each shift, an engineering worker should:

1. Read current global state, component contracts, dependency state, and active claims.
2. Determine all roles/work items for which it is eligible.
3. Exclude blocked, unready, or already-claimed write domains.
4. Prefer work using deterministic factors such as:
   - primary-role affinity;
   - readiness;
   - downstream items unblocked;
   - age of ready backlog;
   - integration criticality;
   - validation/integration backlog;
   - collision risk.
5. Claim exactly one bounded work item/component write domain for the shift.
6. Record role, work item, owned paths, consumed/provided contract revisions, and `STARTED_AT`.
7. Stay in that role for the entire shift; do not opportunistically cross component ownership boundaries.
8. At completion, release/advance the claim and record `COMPLETED_AT`, evidence, remaining work, and exact next pickup.

A simple deterministic priority table is preferred initially over a complex scoring algorithm.

## Component-team rules

Each component should eventually document:

- purpose;
- mutable state it owns;
- provided API/hooks;
- required API/eyelets;
- owned paths;
- paths it must not modify;
- lifecycle/state-machine obligations;
- thread/concurrency ownership;
- failure and shutdown semantics;
- tests/validation obligations;
- current interface/contract revision.

Implementation knowledge should stop at the seam. A component may consume another component's contract but must not depend on its internals.

A worker that discovers an insufficient foreign API must raise an explicit interface-change/integration request. It must not silently edit the provider component to make its own work easier.

## Branch/integration model

When true simultaneous component reconstruction begins, prefer separate component/work branches rather than concurrent workers committing unrelated component work directly to one shared branch.

Conceptual model:

- component team branch/work item;
- component-owned source changes and tests;
- explicit seam/contract revision;
- controlled integration into the ledge integration branch;
- independent validation after integration;
- hardware qualification remains a separate gate.

The integration owner coordinates seams and rejects incompatible contracts; it should not become a second implementation owner for every component.

## Safety requirements before activation

Do not activate this model merely because parallel work exists. Require at least:

1. stable-enough component boundaries and seam contracts;
2. explicit component/path ownership map;
3. repository-visible active-claim mechanism with stale-claim recovery;
4. compare-and-recheck authority before every write/merge;
5. one active writer per component;
6. at most two workers per theoretical role;
7. independent validation preserved;
8. integration ownership defined;
9. known-defect accounting preserved across component branches;
10. a rollback/recovery procedure that preserves evidence instead of force-resetting history.

## Known exposure / threat model

Parallel reconstruction exposes the project to several classes of failure that serial work largely avoids:

- **double claim race:** two workers select the same READY work before either sees the other's claim;
- **stale contract race:** one worker implements against an interface revision while another changes it;
- **cross-boundary creep:** a worker edits a foreign component because the seam is inconvenient;
- **merge-green / behavior-red:** branches merge cleanly textually but lifecycle/order semantics conflict;
- **integration-owner overload:** too much glue logic migrates into integration/bridge code and recreates a monolith;
- **validation contamination:** implementation workers implicitly certify their own work or validation adapts to implementation instead of contract;
- **dependency inversion drift:** components begin reaching around documented APIs to access foreign state;
- **orphan/stale claims:** a paused/failed worker leaves a component appearing permanently occupied;
- **branch authority races:** two workers create valid commits from different heads and one unintentionally supersedes the other;
- **documentation drift:** API docs, symbol dictionaries, and actual code diverge under parallel edits;
- **known-defect loss:** a team "fixes" or changes a preserved defect while another team assumes parity behavior;
- **hidden shared-resource conflicts:** files may differ while both teams change the same runtime resource, lifecycle phase, thread, queue, GS owner, socket, or teardown order;
- **false parallelism:** work appears component-separated but still depends on an unsettled shared invariant, leading to expensive rework later.

## Required countermeasures

Before true simultaneous development, implement or define:

- atomic or compare-and-swap style claim creation tied to an observed branch/state revision;
- claim owner, start time, lease/heartbeat or explicit stale-claim rule, and exact release condition;
- contract revision pinning in every work item;
- dependency graph declaring what contract revision unlocks each downstream item;
- branch-head re-read immediately before each write/merge;
- owned-path enforcement/checking where practical;
- explicit interface-change request path;
- combined integration tests targeting seams, ordering, repeated sessions, failure convergence, and shutdown—not only per-component unit tests;
- contract/API compatibility checks where practical;
- independent validation after integration rather than only before merge;
- automatic or continuity-owned detection of stale claims, conflicting contract revisions, and long-unintegrated branches;
- durable timestamps for claim/start/finish/merge/validation so throughput and collision data can be measured empirically.

## Activation trigger

The likely activation point is after A001 establishes a coherent shared transport foundation and the next reconstruction-ready tranches have stable enough ownership/API boundaries that at least two components can be developed independently without redesigning the seam each shift.

Until then, staggered sequential reconstruction shifts remain safer for the common foundation.

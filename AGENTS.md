# Development Session Instructions

This is the canonical entry point for development sessions working on
PS-to-VNC.

The repository must be sufficient to reconstruct an effective collaborator
context without depending on prior conversation history.

Conversation memory is useful context, but it is not project authority.

## Bootstrap

Before proposing source changes, tests, recovery actions, or architectural
changes:

1. read `docs/status.md`;
2. read `docs/README.md`;
3. read `docs/development/README.md`;
4. read architecture, ADR, testing, runbook, and reference material relevant
   to the active subsystem;
5. inspect:
   - current branch;
   - recent Git history;
   - `git status`;
   - staged changes;
   - relevant unstaged and untracked work;
6. run:
   - `scripts/resume-state.sh`
   - `scripts/check.sh`

Do not reset, clean, checkout over, or otherwise discard uncommitted work
during catch-up.

Treat dirty state as potentially important active development until it is
understood.

## Authority model

Different kinds of truth have different homes:

- current human state: `docs/status.md`;
- current machine state: `runtime/PROJECT_STATE.env`;
- preserved migration-era machine state: `runtime/MIGRATION_STATE.env`;
- current clean architecture: `docs/CLEAN_ARCHITECTURE.md`;
- historical/normalization architecture: the superseded architecture documents
  routed by `docs/README.md`;
- durable design rationale: `docs/adr/` and retained historical decisions;
- empirical evidence: `evidence/` and testing documentation;
- deferred work: `docs/ROADMAP.md`;
- chronological history: migration/history documents and Git;
- development rules: `AGENTS.md`, `CONTRIBUTING.md`, and
  `docs/development/`.

Do not silently choose between contradictory authorities. Investigate and
reconcile the discrepancy.

## Discover before inventing

Before redesigning a subsystem or constructing a development procedure, check
whether the repository or inherited project environment already contains:

- an implementation;
- a prior experiment;
- an architectural decision;
- a known-good checkpoint;
- preserved evidence;
- a canonical build, deployment, recovery, or test tool.

If a canonical saved procedure exists, use it.

Do not synthesize a parallel bespoke implementation in chat merely because its
underlying commands can be reconstructed.

Large one-off command packets are appropriate for genuinely new investigation,
tool creation, or tool repair—not for routine established mechanics.

Repeated useful procedures should graduate into repository tooling.

## Empirical testing

Before proposing or repeating a test, identify:

1. the last proven result;
2. the next intended test;
3. the exact DUT;
4. the prior workload;
5. the prior apparatus;
6. the operator procedure;
7. the failure-safety mechanism.

Preserve established apparatus unless the experiment deliberately changes it.

Apparatus changes are experimental variables and must be recorded.

Machine evidence and operator observation are separate authorities. Do not
claim a physical result from machine evidence alone.

For the current PS-to-VNC development contract, a changed PT_LOAD requires
hardware qualification unless an explicit recorded policy says otherwise.

## Documentation changes

Before materially changing documentation:

1. survey existing authority;
2. search for overlapping or contradictory current claims;
3. identify affected navigation and cross-references;
4. separate current truth from chronological history;
5. update the documentation as a set;
6. run the documentation checks.

Do not create a second current authority because the first is inconvenient.

If durable knowledge is required by a future development session, it belongs
in the repository rather than only in conversation history.

## Real-project agitation

The development-continuity system evolves under pressure from real project
work.

PS2VNC served as its first proving ground. PS-to-VNC continues that role.

When friction, repeated manual work, stale state, ambiguous evidence, or an
infrastructure failure occurs, distinguish whether the pressure is:

- product-specific;
- experiment-specific;
- development-infrastructure-specific.

Generalize only after a real recurring need has been demonstrated.

Project-specific mechanics remain in the project adapter. Reusable invariants
may later graduate into a portable development-continuity framework.

## Completion

A development step is not complete merely because code works.

Where applicable, completion includes:

- source authority;
- reproducible build identity;
- empirical evidence;
- hardware/operator qualification;
- current-state update;
- documentation update;
- deferred-work capture;
- reconciled repository state.

See `CONTRIBUTING.md` and `docs/development/` for detailed policy.

## Toolkit-first routine operations

Before generating one-off shell or helper code for a routine build,
deployment, hardware-test, identity, evidence, fingerprint, or other repeated
development procedure, inspect the established tooling first and reuse a
proven tool when one exists.

When an ad-hoc recovery or correction produces a reliable procedure that is
likely to recur and no equivalent canonical tool exists, promote that working
procedure into reusable successor-owned tooling, test it, document it, and use
the saved tool thereafter instead of regenerating the procedure.

New or changed reusable shell tooling must pass `bash -n` and its relevant
TestKit self-test before first operational use and before commit.

The historical repository at `/home/ps2/ps2vnc` remains immutable reference
authority when present. Issue #7 hardware qualification must not depend on that
checkout or invoke its TestKit; the active apparatus is successor-owned.
Historical mechanisms may still be consulted as evidence before deliberate
adoption.

## Frozen legacy repository immutability clarification

For `/home/ps2/ps2vnc`, immutable means:

- the required frozen Git HEAD must remain exact;
- tracked working-tree content must remain unchanged;
- the Git index must remain unchanged.

Pre-existing untracked forensic, runtime, log, cache, and test artifacts are
not tracked-source mutations. They must be preserved, must not be cleaned as
part of ordinary PS-to-VNC work, and do not by themselves violate legacy
immutability.

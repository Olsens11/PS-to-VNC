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
3. read `docs/PROJECT_INTENT.md`;
4. read `docs/CLEAN_ARCHITECTURE.md`;
5. read `docs/development/README.md`;
6. before proposing or writing clean product source, read
   `docs/development/source-naming-and-symbols.md`;
7. before adding, moving, or reorganizing clean product files or directories,
   read `docs/development/source-topology.md`;
8. read ADR, audit, testing, runbook, and reference material relevant to the
   active subsystem;
9. inspect:
   - current branch;
   - recent Git history;
   - `git status`;
   - staged changes;
   - relevant unstaged and untracked work;
10. run:
   - `scripts/resume-state.sh`
   - `scripts/check.sh`

Do not reset, clean, checkout over, or otherwise discard uncommitted work
during catch-up.

Treat dirty state as potentially important active development until it is
understood.

## Clean reconstruction design contract

A fresh collaborator must understand the design ethos before changing product
structure.

The normative authorities are:

    docs/PROJECT_INTENT.md
    docs/CLEAN_ARCHITECTURE.md

The clean reconstruction follows these working principles:

- rebuild PS-to-VNC as the program we would have written if the exploratory
  implementation's lessons had been known from the beginning;
- preserve proven behavior, hardware knowledge, invariants, and useful
  mechanisms without preserving accidental historical structure;
- design top-down from responsibilities, ownership, and interfaces, then
  implement bottom-up from the smallest useful executable foundation;
- Issue #7 is the deliberately minimal proven chassis. Later reconstruction
  stages grow coherent feature families around that chassis rather than
  gradually turning the coordinator into another monolith;
- mechanisms live with the owner that understands them. The application
  coordinator initializes, routes, sequences, and combines domains only where
  genuinely cross-domain product behavior requires orchestration;
- use one clear owner for important mutable state and keep dependency direction
  explicit;
- favor direct calls, ordinary structs and enums, readable control flow, and
  host-testable pure logic when those mechanisms express the requirement;
- do not create broad globals, hidden coupling, generic callback frameworks,
  message buses, wrappers, or abstractions merely for architectural appearance;
- do not respond to the old monolith by mechanically maximizing module or file
  count. Boundaries are earned by coherent responsibility, ownership,
  independent testing, hardware lifecycle, or future reworkability;
- "smallest useful" means the smallest coherent and growth-capable
  responsibility boundary, not the narrowest implementation possible;
- known neighboring reconstruction stages should be able to attach naturally
  without immediately replacing the boundary, while behavior not yet earned by
  the active stage remains unimplemented;
- sophisticated product behavior should arise from explicit composition of
  understandable components rather than teaching each component the entire
  application context;
- when a supposedly local change requires understanding several unrelated
  domains, reconsider the boundary;
- hardware-facing claims remain evidence-gated, and exact DUT/PT_LOAD identity
  remains part of what a result means.

The reconstruction pattern is therefore skeleton first, then meat on the bones:
establish durable owners and seams, prove them with the smallest real behavior
that exercises them, and then deepen each feature family without redistributing
its mechanism into unrelated code.

## Source-writing contract

Clean product source must be understandable both locally in the code and from
the repository's canonical lookup surfaces.

The normative naming, source-orientation, comment, and symbol-dictionary policy
is:

    docs/development/source-naming-and-symbols.md

Before designing or reviewing clean source, preserve these working rules:

- names communicate the thing or role represented, including distinctions that
  matter to correctness, authority, lifecycle, or representation;
- an index or symbol dictionary never excuses an unclear source name;
- every maintained C source/header begins with a synopsis stating what the file
  accomplishes, owns, excludes, and where its plain-language context lives;
- internal comments explain invariants, authority, ordering, failure
  consequences, hardware assumptions, and rationale rather than narrating
  syntax;
- source dictionaries supplement readable code; they do not replace readable
  code;
- clean product files live with the responsibility that owns them; `src/`
  itself is reserved for the executable entry point and application
  coordinator rather than becoming a catch-all feature directory;
- each clean product directory owns its companion `SYMBOLS.md`; creating a new
  clean domain directory requires deliberate topology/documentation/build/test
  integration in the same change;
- `scripts/check-issue7-linked-reproducibility.sh` is a historical Issue #7
  **stage-local** mechanism, not a general current-stage regression check; when
  encountered outside the exact final Issue #7 source authority it must perform
  no build, report `SKIPPED` / `NOT_APPLICABLE`, and return success so unrelated
  workflows continue; when actual Issue #7 reproducibility evidence is required
  from a later stage, use
  `scripts/check-historical-issue7-reproducibility.sh`, and never add
  current-stage objects to the historical Issue #7 object set;
- **Generalize portability; preserve purpose.** Making a tool independent of a
  particular worktree path, host path, or execution directory does not broaden
  the stage, subsystem, or evidence purpose for which that tool was designed;
- before invoking a development, test, qualification, or historical-analysis
  tool, identify why that tool was selected, what specific claim or uncertainty
  its result is intended to address, and why it is applicable to the current
  source stage, subsystem, environment, and task; presence in `scripts/`, a
  `SYMBOLS.md` entry, executable status, or ability to run is not by itself a
  reason to execute a tool;
- choose symbol names so a reader can readily understand the operation and
  ownership from ordinary source reading without repeatedly consulting an
  external index;
- comments should explain why a non-obvious choice exists, especially where a
  simpler-looking alternative would violate an invariant or ownership rule.

These source-writing rules and the clean reconstruction design contract are
engineering requirements, not cosmetic style. Repository continuity should let
a fresh development session determine not only what code exists, but why it is
shaped that way and how new code is expected to fit into it.

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

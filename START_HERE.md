# START HERE — PS-to-VNC

> **Compatibility entry point**
>
> Canonical development-session bootstrap: `AGENTS.md`.
>
> Canonical current state: `docs/status.md`.
>
> This file remains because migration-era workflows reference its path.
> Historical current-state material below is not the canonical current-state
> authority.
>
> CANONICAL_SESSION_BOOTSTRAP=AGENTS.md


This is the mandatory recovery entry point whenever conversational or
developer context is missing.

## Authority rule

Do not reconstruct current project state from chat memory.

The repository is authoritative.

## Recovery procedure

Read these files in order:

1. `START_HERE.md`
2. `AGENTS.md`
3. `docs/PROJECT_INTENT.md`
4. `docs/CLEAN_ARCHITECTURE.md`
5. `docs/development/README.md`
6. `docs/development/source-naming-and-symbols.md`
7. `docs/development/source-topology.md`
8. `runtime/PROJECT_STATE.env`
9. `docs/status.md`
10. `runtime/MIGRATION_STATE.env`
11. `docs/MIGRATION_STATE.md`
12. `docs/PROJECT_STATE.md`
13. `docs/MODULARIZATION.md`
14. `docs/reference/FILE_AND_SERVICE_MAP.md`

From the current PS-to-VNC worktree, run:

    scripts/resume-state.sh
    scripts/check.sh

Do not perform a project mutation until the current project check has been
reviewed.

## Tool selection before invocation

A repository tool is not part of the current workflow merely because it exists,
is executable, appears in a symbol dictionary, or can run from the current
checkout.

Before invoking a development, test, qualification, or historical-analysis
tool, establish:

1. why this tool was selected;
2. what specific claim, uncertainty, or evidence need its result addresses;
3. why its original purpose applies to the current source stage, subsystem,
   environment, and task.

Use this sequence:

    inventory -> applicability -> evidence need -> invocation

Generalize portability; preserve purpose. Location-independent execution must
not silently turn a deliberately stage-specific tool into a general project
check. Do not run tests merely to accumulate additional PASS results when they
do not close a relevant uncertainty.

## Issue #7 stage-local reproducibility command

Two similarly named Issue #7 commands have different purposes.

The original command:

    ./scripts/check-issue7-linked-reproducibility.sh

is **stage-local**. It performs the real two-build reproducibility proof only
when the checkout itself is the exact final Issue #7 source authority.

In the current tool version, any checkout other than the exact final Issue #7
source authority is intentionally non-blocking:

    ISSUE7_LINKED_REPRODUCIBILITY_APPLICABILITY=NOT_APPLICABLE
    ISSUE7_LINKED_REPRODUCIBILITY=SKIPPED
    ISSUE7_LINKED_REPRODUCIBILITY_BUILD_RUN=NO
    ISSUE7_LINKED_REPRODUCIBILITY_WORKFLOW_BLOCKING=NO

That result is normal. It means the check does not belong to the current stage;
it is not evidence of a project failure and should not interrupt an unrelated
workflow.

When actual historical Issue #7 reproducibility evidence is required from a
later checkout, explicitly run:

    ./scripts/check-historical-issue7-reproducibility.sh

That wrapper exports the exact final Issue #7 source authority into an isolated
temporary tree and runs the authentic historical proof there.

`SKIPPED` is not equivalent to `PASS`. A workflow that specifically requires
Issue #7 evidence must require the historical wrapper's real PASS result.

Never add current-stage modules to the historical Issue #7 makefile/object set
merely to make the stage-local command operate on later source.

For clean reconstruction work, recovering current state is not enough. A fresh
session must also recover the project's design contract before proposing source
structure:

- `docs/PROJECT_INTENT.md` explains the reconstruction ethos;
- `docs/CLEAN_ARCHITECTURE.md` defines current ownership and dependency
  direction;
- `docs/development/source-naming-and-symbols.md` defines how clean product
  source is named, oriented, commented, and indexed;
- `docs/development/source-topology.md` defines where clean product source
  belongs, which current domain directories are authoritative, and what must
  move together when a new domain is earned;
- `AGENTS.md` summarizes the non-negotiable design and source-writing rules that
  should be active during ordinary development.

The Issue #7 core is intentionally a minimal proven chassis. Later stages grow
coherent responsibility families around it. Do not infer that the application
coordinator should absorb each new feature merely because it activates or
routes that feature.

`runtime/MIGRATION_STATE.env` remains the preserved authority for the completed
exploratory migration/normalization history. It is not the current clean-
reconstruction roadmap. Current project direction is carried by
`runtime/PROJECT_STATE.env` and `docs/status.md`.

## Repository safety boundary

The historical PS2VNC repository and preserved legacy material are
read-only historical authority. Current development must not depend on that
repository being checked out at any particular filesystem location.

Current successor development occurs in the selected PS-to-VNC Git
worktree. Current tooling must not depend on a historical checkout path.

## Migration-stage rule

Every stage must durably record:

- last completed stage;
- current stage and status;
- current source authority;
- Git authority;
- build and hardware-validation state;
- source/module ownership;
- temporary architectural debt;
- documentation state;
- explicit next action.

A stage is not COMPLETE unless a fresh conversation can resume from repository
state alone.

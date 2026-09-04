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

1. START_HERE.md
2. docs/PROJECT_INTENT.md
3. runtime/PROJECT_STATE.env
4. docs/status.md
5. runtime/MIGRATION_STATE.env
6. docs/MIGRATION_STATE.md
7. docs/PROJECT_STATE.md
8. docs/MODULARIZATION.md
9. docs/reference/FILE_AND_SERVICE_MAP.md

From the current PS-to-VNC worktree, run:

    scripts/resume-state.sh
    scripts/check.sh

Do not perform a project mutation until the current project check has been
reviewed.

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

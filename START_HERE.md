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
7. `runtime/PROJECT_STATE.env`
8. `docs/status.md`
9. `runtime/MIGRATION_STATE.env`
10. `docs/MIGRATION_STATE.md`
11. `docs/PROJECT_STATE.md`
12. `docs/MODULARIZATION.md`
13. `docs/reference/FILE_AND_SERVICE_MAP.md`

From the current PS-to-VNC worktree, run:

    scripts/resume-state.sh
    scripts/check.sh

Do not perform a project mutation until the current project check has been
reviewed.

For clean reconstruction work, recovering current state is not enough. A fresh
session must also recover the project's design contract before proposing source
structure:

- `docs/PROJECT_INTENT.md` explains the reconstruction ethos;
- `docs/CLEAN_ARCHITECTURE.md` defines current ownership and dependency
  direction;
- `docs/development/source-naming-and-symbols.md` defines how clean product
  source is named, oriented, commented, and indexed;
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

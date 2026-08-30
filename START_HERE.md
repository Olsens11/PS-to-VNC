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
2. runtime/MIGRATION_STATE.env
3. docs/PROJECT_STATE.md
4. docs/MIGRATION_STATE.md
5. docs/MODULARIZATION.md
6. docs/reference/FILE_AND_SERVICE_MAP.md

Then run these commands:

    cd /home/ps2/PS-to-VNC
    scripts/resume-state.sh
    scripts/migration-check.sh
    scripts/docs-check.sh

Do not perform a project mutation until those checks pass.

## Repository safety boundary

Historical repository:

    /home/ps2/ps2vnc

This is read-only historical authority.

Current successor repository:

    /home/ps2/PS-to-VNC

All new development occurs here.

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

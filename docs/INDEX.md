# PS-to-VNC Documentation Index

This is the canonical entry point for current PS-to-VNC documentation.

## Project

- `docs/PROJECT_IDENTITY.md` — project identity, naming and repository roles.
- `docs/PROJECT_STATE.md` — current implementation and validation state.
- `docs/LEGACY_HANDOFF.md` — relationship to the preserved PS2VNC project.
- `docs/MODULARIZATION.md` — staged migration ledger and lockstep rules.
- `docs/ROADMAP.md` — ordered future development direction.

## Architecture

- `docs/architecture/ARCHITECTURE.md` — current target subsystem architecture.

## Reference

- `docs/reference/FILE_AND_SERVICE_MAP.md` — canonical living map of where
  source, evidence and responsibilities currently reside.

## Documentation contract

Historical evidence remains historical.

Living/current PS-to-VNC documentation must be updated in the same stage that
moves source ownership, changes interfaces, changes build authority or changes
project state.

No modularization stage is complete until its documentation is current and the
documentation checker passes.

## Migration recovery

- `START_HERE.md` — mandatory recovery entry point after context loss.
- `docs/MIGRATION_STATE.md` — human-readable migration authority.
- `runtime/MIGRATION_STATE.env` — machine-readable migration authority.
- `scripts/resume-state.sh` — reconstruct current migration state.
- `scripts/migration-check.sh` — prove migration/source/document coherence.

A fresh conversation begins with `START_HERE.md`, not conversational memory.

## Baseline reproduction

- `docs/M0_BASELINE_CONTRACT.md` — M0 build-reproduction and acceptance contract.

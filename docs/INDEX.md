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

## Historical build provenance

- `docs/M0_BUILD_PROVENANCE.md` — resolved M0 historical build environment, dependency authority, and reference object hashes.

## M0 reproduction result

- `docs/M0_BUILD_RESULT.md` — Outcome A byte-exact reproduction result and characterized slim-LTO intermediate variance.

## M0 hardware-validation resolution

- `docs/M0_HARDWARE_RESOLUTION.md` — explains why no new M0 hardware regression is required for the byte-identical B4A executable and distinguishes inherited evidence from a new physical observation.

## First modular extraction

- `docs/M1_EXTRACTION_CONTRACT.md` — governing contract for the first mechanical low-coupling module extraction after the byte-exact M0 baseline.

## M1 first extraction boundary

- `docs/M1A_EXTRACTION_BOUNDARY.md` — exact committed M1A selection of the first mechanical module-extraction boundary.

## GitHub publication

- `docs/GITHUB_PUBLICATION.md` — authority for the independent private PS-to-VNC GitHub repository and its publication checkpoint.

## M1B mechanical extraction

- `docs/M1B_EXTRACTION_RESULT.md` — exact result and source authority for the first mechanical module extraction.

## M1C first modular DUT

- `docs/M1C_DUT_CHARACTERIZATION.md` — build, reproducibility, linkage, host parity, and ELF characterization for the first modular PS-to-VNC executable.

## M1D hardware regression

- `docs/M1D_HARDWARE_REGRESSION.md` — deployment and physical/machine validation authority for the first modular DUT.

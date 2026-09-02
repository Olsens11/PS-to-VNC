# PS-to-VNC Documentation Index

> **Compatibility index**
>
> Canonical documentation router: `docs/README.md`.
>
> This index remains for migration-era tooling and historical completeness.
>
> CANONICAL_DOCS_ROUTER=docs/README.md


This is the canonical entry point for current PS-to-VNC documentation.

## Project

- `docs/PROJECT_INTENT.md` — seed of intent, clean-reconstruction principles,
  development values, and the preserve/clean/understand/reconstruct strategy.
- `docs/PROJECT_IDENTITY.md` — project identity, naming and repository roles.
- `docs/PROJECT_STATE.md` — preserved/current-at-recorded-time implementation and
  validation checkpoint; see temporal-state policy before treating status prose
  as present-tense truth.
- `docs/status.md` — concise timestamped human status snapshot.
- `docs/LEGACY_HANDOFF.md` — relationship to the preserved PS2VNC project.
- `docs/MODULARIZATION.md` — staged migration ledger and lockstep rules.
- `docs/ROADMAP.md` — ordered future development direction.

## Architecture

- `docs/architecture/ARCHITECTURE.md` — current/historical subsystem architecture reference.

## Reference

- `docs/reference/FILE_AND_SERVICE_MAP.md` — canonical living map of where
  source, evidence and responsibilities currently reside.
- `docs/reference/PRESERVATION_STATUS.md` — completed exploratory-reference
  preservation scope and disposition.

## Semantic audit

- `docs/audit/README.md` — semantic-audit contract, evidence classes, and
  maturity model.
- `docs/audit/BEHAVIORAL_INVENTORY.md` — complete product behavior inventory
  and current audit status.

## Major investigation casebook

- `docs/investigations/README.md` — casebook router, threshold, statuses, and
  reviewed-not-promoted topics.
- `docs/investigations/TEMPLATE.md` — required structure for new major cases.
- `docs/investigations/MI-001-GS-INTERRUPT-EXITHANDLER.md` — resolved GS/EE
  HSync / `ExitHandler()` interrupt-handoff case.
- `docs/investigations/MI-002-MTU1458-SMAP-RX.md` — open MTU1458 / SMAP RX
  corruption investigation.
- `docs/investigations/MI-003-PS2-RECEIVE-BURST-PACING.md` — open PS2 receive
  burst-tolerance / sender-pacing investigation.
- `docs/investigations/MI-004-DISPLAY-TRANSITION-LIVENESS.md` — open intermittent
  display-transition/reconstruction liveness family.
- `docs/investigations/MI-005-576P-LOW-LEVEL-TRANSITION.md` — open persistent
  576p low-level transition stall.
- `docs/investigations/source/MI-001-TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md`
  — exact copied full Test14 mechanism report retained locally in PS-to-VNC.

## Documentation contract

Historical evidence remains historical.

Living/current PS-to-VNC documentation must be updated in the same stage that
moves source ownership, changes interfaces, changes build authority or changes
project state.

No development stage is complete until its documentation is current and the
documentation checker passes.

The clean reconstruction does not erase completed migration evidence; it
reclassifies the former extraction-oriented next steps as historical unless
explicitly adopted into the new design.

Mutable status is temporal: a status/current/next-action document says what was
recorded at its timestamp. Read
`docs/development/TEMPORAL_STATE_SEMANTICS.md` before repeating freshness-sensitive
claims as present-tense truth.

## Migration recovery

- `START_HERE.md` — mandatory recovery entry point after context loss.
- `docs/MIGRATION_STATE.md` — human-readable migration authority.
- `runtime/MIGRATION_STATE.env` — machine-readable migration authority.
- `scripts/resume-state.sh` — produce a snapshot-aware resume report and expose
  migration compatibility fields.
- `scripts/migration-check.sh` — prove migration/source/document coherence.

A fresh conversation begins through `AGENTS.md` and the fresh-session procedure;
`START_HERE.md` remains a migration-era compatibility entry point.

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

## Incremental migration records

- `docs/M1_COMPLETION.md` — authoritative closure record for M1.
- `docs/M2A_EXTRACTION_BOUNDARY.md` — exact committed M2A scalar-parser extraction boundary.
- `docs/M2B_EXTRACTION_RESULT.md` — exact result and source authority for the M2 scalar-parser mechanical extraction.
- `docs/M2C_DUT_CHARACTERIZATION.md` — reproducible M2 DUT build, linkage, parser parity and validation authority.
- `docs/M2_COMPLETION.md` — authoritative M2 closure and accelerated-migration transition record.
- `docs/M3_MACRO_MIGRATION.md` — coarse-first hierarchical migration authority for the M3 implementation scaffold.

## Development continuity

Canonical successor entry points:

- `AGENTS.md`
- `CONTRIBUTING.md`
- `docs/README.md`
- `docs/status.md`
- `docs/PROJECT_INTENT.md`

Development-system policy and routing:

- `docs/development/README.md`
- `docs/development/SESSION_RECONSTRUCTION.md`
- `docs/development/TEMPORAL_STATE_SEMANTICS.md`
- `docs/development/BRANCH_LIFECYCLE.md`
- `docs/development/BRANCH_WORKSTREAM_INDEX.md`
- `docs/development/CONTINUITY_FOLLOWUPS.md`
- `docs/development/documentation.md`
- `docs/development/testing.md`
- `docs/development/tooling.md`
- `docs/development/lessons-learned.md`

Point-in-time branch audit:

- `docs/development/branch-audits/2026-09-02-live-branch-audit.md`

Decision records:

- `docs/adr/README.md`
- `docs/adr/0001-development-continuity-baseline.md`

Machine-readable development-system state:

- `runtime/DEVELOPMENT_SYSTEM.env`
- `runtime/PROJECT_STATE.env` — timestamped machine-state snapshot, not eternal
  present-tense authority.

Checks:

- `scripts/continuity-check.sh`
- `scripts/check.sh`

## Complete compatibility document inventory

The canonical curated documentation router is `docs/README.md`.

This section exists to preserve the migration-era invariant that every
Markdown document remains mechanically discoverable from `docs/INDEX.md`.
Its presence does not make every listed document a current authority.

- `docs/ARCHITECTURAL_NORMALIZATION.md`
- `docs/ARCHITECTURE.md`
- `docs/DEPENDENCY_RULES.md`
- `docs/NORMALIZATION_SEQUENCE.md`
- `docs/STATE_OWNERSHIP.md`
- `docs/TARGET_ARCHITECTURE.md`
- `docs/VERSIONING.md`

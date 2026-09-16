# PS-to-VNC Documentation Index

> **Compatibility index**
>
> Canonical documentation router: `docs/README.md`.
>
> This index remains for migration-era tooling and historical completeness.
>
> CANONICAL_DOCS_ROUTER=docs/README.md


This file remains the exhaustive compatibility inventory for PS-to-VNC
documentation. The current curated documentation entry point is
`docs/README.md`.

## Project

- `docs/PROJECT_INTENT.md` — seed of intent, clean-reconstruction principles,
  development values, and the preserve/clean/understand/reconstruct strategy.
- `docs/PROJECT_IDENTITY.md` — project identity, naming and repository roles.
- `docs/PROJECT_STATE.md` — current implementation and validation state.
- `docs/LEGACY_HANDOFF.md` — relationship to the preserved PS2VNC project.
- `docs/MODULARIZATION.md` — staged migration ledger and lockstep rules.
- `docs/ROADMAP.md` — ordered future development direction.

## Architecture

- `docs/CLEAN_ARCHITECTURE.md` — current clean-reconstruction architecture authority.
- `docs/architecture/ARCHITECTURE.md` — current/historical subsystem architecture reference.

## Reference

- `docs/reference/FILE_AND_SERVICE_MAP.md` — canonical living map of where
  source, evidence and responsibilities currently reside.
- `docs/reference/PRESERVATION_STATUS.md` — completed exploratory-reference
  preservation scope and disposition.
- `docs/reference/SOURCE_SYMBOL_DICTIONARIES.md` — generated portal for
  directory-owned clean-source symbol dictionaries.

## Semantic audit

- `docs/audit/README.md` — semantic-audit contract, evidence classes, and
  maturity model.
- `docs/audit/BEHAVIORAL_INVENTORY.md` — complete product behavior inventory
  and current audit status.
- `docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md` — detailed controller,
  keyboard, and local-UI behavior audit.
- `docs/audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md` — detailed
  configuration, recovery, and management behavior audit.
- `docs/audit/B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md` — detailed
  diagnostics, Pi companion, and development-infrastructure audit.
- `docs/audit/SOURCE_RESPONSIBILITY_MAP.md` — historical implementation,
  ownership, and coupling map.
- `docs/audit/HISTORICAL_LESSONS.md` — durable lessons recovered from prior
  experiments and failures.
- `docs/audit/CROSS_DOMAIN_SYNTHESIS.md` — cross-domain responsibility and
  architecture synthesis.
- `docs/audit/CROSS_DOMAIN_STATE_INTERFACES.md` — explicit state ownership,
  interfaces, and concurrency seams.
- `docs/audit/REBUILD_READY_PROMOTION.md` — promotion record establishing
  rebuild-ready audit maturity.

## Raspberry Pi companion

- `docs/pi/README.md` — finalized clean Pi-companion authority and router.
- `docs/pi/BASELINE.md` — virgin supported-OS Pi baseline.
- `docs/pi/DEPENDENCY_LEDGER.md` — Pi dependency/adoption ledger.
- `docs/pi/PROVISIONING.md` — tracked foundation provisioning contract.
- `docs/pi/RFB_ACTIVATION_RESEARCH.md` — preserved pre-qualification endpoint
  lifecycle research.
- `docs/pi/RFB_SOCKET_ACTIVATION.md` — adopted systemd socket-activation
  lifecycle and hardware qualification.
- `docs/pi/TIGERVNC_PACKAGE_AUTHORITY.md` — TigerVNC package/provider authority.
- `docs/pi/TIGERVNC_SESSION.md` — qualified bare TigerVNC provider/session
  contract.
- `docs/pi/MINIMUM_DESKTOP.md` — post-Issue5 pre-Issue40 Openbox +
  lxpanel-pi desktop candidate, session-isolation proof, and promotion gate.

## Reconstruction

- `docs/reconstruction/ISSUE7_MINIMAL_CORE.md` — clean Issue #7 minimal-core
  reconstruction contract and implementation boundary.
- `docs/reconstruction/ISSUE7_DUT_MANIFEST.md` — Issue #7 DUT identity and
  manifest contract.
- `docs/reconstruction/ISSUE7_HARDWARE_QUALIFICATION.md` — Issue #7 hardware
  qualification procedure and evidence contract.
- `docs/reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md` — dated record
  of the clean domain-directory/source-dictionary topology adoption during
  Issue #39 closeout.

## Ledge reconstruction authority

The ledge work-log directory is append-only operational history. Individual
shift records are discovered through `docs/ledge/work-log/` and validated by
the work-log checker rather than being appended to this shared compatibility
index after every shift.

- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` — ledge-specific architecture overlay.
- `docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md` — A001 Transport/RFB audit authority.
- `docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md` — A002 configuration/audio audit authority.
- `docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md` — A003 MPEG-generation audit authority.
- `docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md` — A004 presentation/calibration audit authority.
- `docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md` — Foreman packetization invariant preserving DESKTOP CALIBRATION versus MPEG CALIBRATION authority and lineage.
- `docs/ledge/LEDGE_AUDIT_A005_INTERACTION_INPUT.md` — A005 interaction/input audit authority.
- `docs/ledge/LEDGE_AUDIT_A006_ORCHESTRATION_SHUTDOWN.md` — A006 orchestration/shutdown audit authority.
- `docs/ledge/LEDGE_AUDIT_A007_DIAGNOSTIC_COMPLETENESS.md` — A007 diagnostics/completeness audit authority.
- `docs/ledge/LEDGE_AUDIT_LOG.md` — historical ledge audit log.
- `docs/ledge/LEDGE_AUDIT_STATE.md` — current ledge audit state snapshot.
- `docs/ledge/LEDGE_FOREMAN_STATE.md` — current Reconstruction Foreman planning/integration baton.
- `docs/ledge/LEDGE_INTEGRATION_STATE.md` — ledge integration-state authority.
- `docs/ledge/LEDGE_PARALLEL_DEVELOPMENT_MODEL.md` — ledge parallel-development model.
- `docs/ledge/LEDGE_RECONSTRUCTION_CONTRACT.md` — Reconstruction crew contract.
- `docs/ledge/LEDGE_RECONSTRUCTION_LOG.md` — historical Reconstruction log.
- `docs/ledge/LEDGE_RECONSTRUCTION_STATE.md` — Reconstruction state snapshot authority.
- `docs/ledge/LEDGE_SEMANTIC_AUDIT.md` — ledge semantic-audit synthesis.
- `docs/ledge/LEDGE_SIMPLIFICATION_REGISTER.md` — simplification disposition register.
- `docs/ledge/LEDGE_VALIDATION_FINDINGS.md` — Validation finding register.
- `docs/ledge/LEDGE_VALIDATION_LOG.md` — historical Validation log.
- `docs/ledge/LEDGE_VALIDATION_STATE.md` — Validation state snapshot authority.
- `docs/ledge/LEDGE_WORK_LOG.md` — historical/global ledge work log.
- `docs/ledge/LEDGE_WORK_STATE.md` — current global Continuity/reconstruction work state.
- `docs/ledge/work-log/README.md` — immutable per-shift work-log contract and discovery rules.

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

## Migration recovery

- `START_HERE.md` — mandatory recovery entry point after context loss.
- `docs/MIGRATION_STATE.md` — human-readable migration authority.
- `runtime/MIGRATION_STATE.env` — machine-readable migration authority.
- `scripts/resume-state.sh` — report current and preserved historical state.
- `scripts/check.sh` — run current project consistency checks.

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

## Incremental migration records

- `docs/M1_COMPLETION.md` — authoritative closure record for M1.
- `docs/M2A_EXTRACTION_BOUNDARY.md` — exact committed M2A scalar-parser extraction boundary.

## M2B mechanical extraction

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

Development-system policy:

- `docs/development/README.md`
- `docs/development/documentation.md`
- `docs/development/testing.md`
- `docs/development/tooling.md`
- `docs/development/lessons-learned.md`
- `docs/development/source-naming-and-symbols.md`
- `docs/development/source-topology.md`
- `docs/development/source-dictionary-validator.md`

Decision records:

- `docs/adr/README.md`
- `docs/adr/0001-development-continuity-baseline.md`
- `docs/adr/0002-use-libpad-directly-reference-opl.md`

Machine-readable development-system state:

- `runtime/DEVELOPMENT_SYSTEM.env`
- `runtime/SOURCE_DICTIONARY_STATE.env` — trusted comprehensive source-dictionary audit baseline authority.

Checks:

- `scripts/continuity-check.sh`
- `scripts/check.sh`

## Complete compatibility document inventory

The canonical curated documentation router is `docs/README.md`.

This section exists to preserve the migration-era invariant that every
maintained Markdown authority/reference document remains mechanically
discoverable from `docs/INDEX.md`. Immutable ledge shift records are discovered
by their directory contract and are not duplicated here.
Its presence does not make every listed document a current authority.

- `docs/ARCHITECTURAL_NORMALIZATION.md`
- `docs/ARCHITECTURE.md`
- `docs/DEPENDENCY_RULES.md`
- `docs/NORMALIZATION_SEQUENCE.md`
- `docs/STATE_OWNERSHIP.md`
- `docs/TARGET_ARCHITECTURE.md`
- `docs/VERSIONING.md`

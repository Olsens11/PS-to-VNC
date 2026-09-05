# PS-to-VNC Documentation

This is the canonical documentation router.

Development sessions begin with:

    ../AGENTS.md

## Project intent and current state

- `PROJECT_INTENT.md` — seed of intent, reconstruction principles, development
  values, and the clean-rebuild strategy.
- `status.md` — concise current human-readable state.
- `../runtime/PROJECT_STATE.env` — current machine-readable project state.
- `../runtime/MIGRATION_STATE.env` — preserved migration-era machine authority.
- `../runtime/` — additional machine-readable authority and historical state.

## Architecture

Current clean-reconstruction architecture authority:

- `CLEAN_ARCHITECTURE.md` — evidence-derived responsibility, state ownership,
  dependency, lifecycle, concurrency, Pi-boundary, and reconstruction model.

Supporting semantic derivation lives under `audit/`, especially:

- `audit/CROSS_DOMAIN_SYNTHESIS.md`;
- `audit/CROSS_DOMAIN_STATE_INTERFACES.md`;
- `audit/SOURCE_RESPONSIBILITY_MAP.md`;
- `audit/HISTORICAL_LESSONS.md`.

Normalization-era architecture remains historical/reference material:

- `ARCHITECTURE.md`;
- `TARGET_ARCHITECTURE.md`;
- `STATE_OWNERSHIP.md`;
- `DEPENDENCY_RULES.md`;
- `NORMALIZATION_SEQUENCE.md`;
- `ARCHITECTURAL_NORMALIZATION.md`.

Those documents retain useful principles and migration history, but their
preselected M4 decomposition is superseded by `CLEAN_ARCHITECTURE.md` for new
clean-reconstruction work.

Subsystem historical architecture may additionally live under `architecture/`.

## Semantic audit

The clean reconstruction first documented what PS-to-VNC actually does before
freezing replacement structure.

Audit work connects:

- product behavior;
- subsystem responsibility;
- B4A/current implementation references;
- historical tests and evidence;
- failure modes and invariants;
- lessons that must survive the rewrite;
- implications for the clean design.

Defined audit work is tracked in GitHub Issues; durable findings belong back in
this repository.

Current durable audit surfaces:

- `audit/README.md` — audit contract, evidence classes, and maturity model;
- `audit/BEHAVIORAL_INVENTORY.md` — complete top-level product behavior
  inventory;
- `audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md` — detailed input/UI tranche;
- `audit/B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md` — detailed config and
  recovery/management tranche;
- `audit/B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md` — diagnostics,
  companion, and engineering-infrastructure tranche;
- `audit/SOURCE_RESPONSIBILITY_MAP.md` — historical implementation/state/coupling
  map;
- `audit/HISTORICAL_LESSONS.md` — durable experiment/failure lessons;
- `audit/CROSS_DOMAIN_SYNTHESIS.md` — responsibility-level architecture
  derivation;
- `audit/CROSS_DOMAIN_STATE_INTERFACES.md` — explicit state ownership,
  interfaces, concurrency seams, and critical-flow verification.

`CLEAN_ARCHITECTURE.md` is the promoted design authority derived from these audit
outputs.

## Raspberry Pi companion

The finalized Issue #5 Pi foundation has been carried forward into the current
reconstruction without reconstructing it from live-machine folklore.

- `pi/README.md` — canonical Pi-companion router and finalized Issue #5
  foundation authority;
- `pi/PROVISIONING.md` — clean supported-OS provisioning contract;
- `pi/TIGERVNC_SESSION.md` — qualified bare TigerVNC provider contract;
- `pi/RFB_SOCKET_ACTIVATION.md` — adopted systemd socket lifecycle and hardware
  qualification record.

The finalized tracked source authority carried forward is
`e909ffc97563d678e87a7d8cf9febfdd452fe7b9`. Its hardware-qualification
evidence head is `b40f422a760a0b7b6f2ab41699c5e72fda83bb83`.

The qualified Issue #5 provider intentionally ends at a bare framebuffer.
Window-manager/session contents are a later layer and must be documented as new
work rather than rewritten into Issue #5 history.

## Reconstruction checkpoints

- `reconstruction/2026-09-05_SOURCE_TOPOLOGY_ADOPTION.md` — chronological
  record of the domain-directory topology adopted during Issue #39 closeout,
  including dictionary counts and qualification-transfer evidence.

## Development process

- `development/source-naming-and-symbols.md` — naming, file-orientation, and
  complete source-symbol dictionary policy.
- `development/source-topology.md` — current clean-product directory ownership,
  root-source rule, local-dictionary rule, and requirements for adding/moving
  product domains.
- `development/source-dictionary-validator.md` — validator implementation,
  language-aware discovery, maintenance-drift behavior, trusted-baseline
  contract, and deferred exact-instance ignore design.
- `development/README.md`
- `development/documentation.md`
- `development/testing.md`
- `development/tooling.md`
- `development/lessons-learned.md`

These documents describe how development itself is conducted and preserved.

## Decisions

- `adr/` — current Architecture Decision Records.
- retained historical decision material remains historical evidence until
  explicitly reconciled.

## Source dictionaries

- `reference/SOURCE_SYMBOL_DICTIONARIES.md` — generated portal for the
  directory-owned clean-source dictionaries.
- `development/source-naming-and-symbols.md` — governing naming, description,
  scope, and validation policy.

## Testing and evidence

Machine and hardware evidence lives under:

    ../evidence/

Testing policy is:

    development/testing.md

Historical evidence remains valid evidence even when the clean reconstruction
chooses a different architecture.

## Roadmap

Deferred work belongs in:

    ROADMAP.md

Capture useful deferred work before moving on rather than relying on
conversation memory.

The migration-era extraction sequence is historical from the strategic reboot
unless explicitly adopted into the new reconstruction roadmap.

## Migration/history

Migration state/history documents, Git history, and preserved evidence retain
the chronological development record.

They are not substitutes for the concise current state in `status.md`, the
project values in `PROJECT_INTENT.md`, or the current architecture in
`CLEAN_ARCHITECTURE.md`.

## Compatibility documents

Migration-era router and state files remain because existing migration tooling
still references their paths.

Where historical current-state or architecture text conflicts with `status.md`,
`PROJECT_INTENT.md`, or `CLEAN_ARCHITECTURE.md`, classify the older material as
historical rather than silently rewriting the record.

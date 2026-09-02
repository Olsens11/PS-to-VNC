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

## Major investigations

- `investigations/README.md` — canonical major-investigation casebook containing
  both ongoing investigations and resolved service-bulletin-style closeouts.
- `investigations/TEMPLATE.md` — required structure for a new major case.

Major cases are created only when a failure, workaround, cross-layer mechanism,
or unresolved platform behavior is significant enough that losing its reasoning
could harm future design. They begin while the issue is still unresolved and are
updated through final closeout rather than being written only after the fact.

GitHub issues remain useful working chronology/discussion surfaces; the durable
technical case report belongs under `investigations/` and links the exact test,
evidence, branch, source, and legacy authority instead of replacing them.

Before changing a subsystem, check the casebook for relevant open **and
resolved** cases. A resolved case may explain a current invariant just as an
open case may explain a temporary qualified workaround.

## Development process

- `development/README.md`
- `development/documentation.md`
- `development/testing.md`
- `development/tooling.md`
- `development/lessons-learned.md`
- `development/SESSION_RECONSTRUCTION.md`

These documents describe how development itself is conducted and preserved.

## Decisions

- `adr/` — current Architecture Decision Records.
- retained historical decision material remains historical evidence until
  explicitly reconciled.

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

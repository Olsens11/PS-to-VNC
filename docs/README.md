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

Current/historical architecture is rooted in:

- `ARCHITECTURE.md`
- `TARGET_ARCHITECTURE.md`
- `STATE_OWNERSHIP.md`
- `DEPENDENCY_RULES.md`
- `NORMALIZATION_SEQUENCE.md`
- `ARCHITECTURAL_NORMALIZATION.md`

Subsystem architecture may additionally live under `architecture/`.

During the semantic-audit phase, these are reference material rather than an
automatic structural blueprint for the clean reconstruction.

## Semantic audit

The clean reconstruction begins by documenting what PS-to-VNC actually does
before redesigning it.

Audit work should connect:

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

- `audit/README.md` — audit contract, evidence classes, and maturity model.
- `audit/BEHAVIORAL_INVENTORY.md` — complete top-level product behavior
  inventory and audit status.

## Development process

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

They are not substitutes for the concise current state in `status.md` or the
project values in `PROJECT_INTENT.md`.

## Compatibility documents

Migration-era router and state files remain because existing migration tooling
still references their paths.

Where historical current-state text conflicts with `status.md` or the clean
reconstruction intent, classify the older material as historical rather than
silently rewriting the record.

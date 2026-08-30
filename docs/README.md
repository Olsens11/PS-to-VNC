# PS-to-VNC Documentation

This is the canonical documentation router.

Development sessions begin with:

    ../AGENTS.md

## Current state

- `status.md` — concise current human-readable state.
- `../runtime/` — machine-readable current state.

## Architecture

Current architecture is rooted in:

- `ARCHITECTURE.md`
- `TARGET_ARCHITECTURE.md`
- `STATE_OWNERSHIP.md`
- `DEPENDENCY_RULES.md`
- `NORMALIZATION_SEQUENCE.md`
- `ARCHITECTURAL_NORMALIZATION.md`

Subsystem architecture may additionally live under `architecture/`.

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

## Roadmap

Deferred work belongs in:

    ROADMAP.md

Capture useful deferred work before moving on rather than relying on
conversation memory.

## Migration/history

Migration state/history documents, Git history, and preserved evidence retain
the chronological development record.

They are not substitutes for the concise current state in `status.md`.

## Compatibility documents

Migration-era router and state files remain because existing migration tooling
still references their paths.

Where historical current-state text conflicts with `status.md` or machine
state under `runtime/`, investigate the discrepancy rather than silently
choosing one.

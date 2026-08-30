# ADR 0001: Repository-centered development continuity

## Status

Accepted — 2026-08-30

## Context

PS2VNC developed a substantial documentation and development-infrastructure
system under pressure from long-running empirical work and conversation-limit
handoffs.

PS-to-VNC inherited many project artifacts but not all operating invariants.
That allowed documentation drift and unnecessary reconstruction of existing
test procedures.

The successor project has a more conventional modular architecture and should
not reproduce the legacy file topology verbatim.

## Decision

PS-to-VNC uses a repository-centered development-continuity model with
conventional entry points:

- `AGENTS.md` — development-session bootstrap;
- `CONTRIBUTING.md` — contributor workflow;
- `docs/README.md` — documentation router;
- `docs/status.md` — concise current human state;
- `docs/development/` — development-system policy;
- `docs/adr/` — durable architectural decisions;
- `runtime/` — machine-readable current state;
- `evidence/` — empirical evidence.

Migration-era documents remain as compatibility surfaces while current tooling
depends on them.

Canonical saved tooling must be discovered and reused for routine procedures.

The development-continuity system continues to evolve under real-project
agitation from PS-to-VNC rather than being designed only by abstraction.

## Consequences

A fresh session has one canonical bootstrap.

Current state is separated from chronological history.

Development-infrastructure failures can improve the development system without
being misclassified as product failures.

Some legacy documentation remains temporarily duplicated for compatibility.

The reusable continuity layer may later be extracted only after repeated
real-project use demonstrates stable boundaries.

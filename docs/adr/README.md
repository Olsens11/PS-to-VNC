# Architecture Decision Records

This directory contains Architecture Decision Records (ADRs).

Use an ADR when a decision:

- materially constrains architecture or development practice;
- has meaningful alternatives;
- will matter after the implementing code changes;
- benefits from preserving rationale and consequences.

ADRs are not current-state logs, empirical test logs, or chronological project
diaries.

Naming convention:

    NNNN-short-decision-title.md

Recommended sections:

- Status
- Context
- Decision
- Consequences
- Alternatives considered

Accepted ADRs are append-only in spirit. If a decision changes, prefer a new
ADR that supersedes the old one rather than rewriting historical rationale.

## Accepted records

- `0001-development-continuity-baseline.md` — repository-centered development
  continuity.
- `0002-use-libpad-directly-reference-opl.md` — use PS2SDK libpad directly,
  consult OPL as the preferred mature pad-usage reference, and add only
  project-specific pad/controller responsibility.

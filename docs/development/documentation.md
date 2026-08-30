# Documentation Policy

Documentation changes are repository changes and should be treated with the
same care as source changes.

## Authority survey

Before creating, reorganizing, or materially changing documentation:

1. search for existing coverage;
2. identify the current authority;
3. find overlapping or contradictory statements;
4. identify affected navigation and cross-references;
5. identify stale current-state claims;
6. distinguish current truth from historical evidence.

Do not update only the first plausible file found.

## Current state versus history

`docs/status.md` should remain concise and current.

Chronological migration detail belongs in migration/history records and Git.

Do not turn the current-state handoff into an ever-growing historical ledger.

## Architecture versus decisions

Architecture describes the intended current model.

ADRs preserve important decisions and rationale.

Neither is a test log.

## Evidence versus interpretation

Raw machine/test artifacts belong under `evidence/`.

Interpretation may summarize evidence but must not silently mutate or replace
the raw record.

## Routers and compatibility

When documentation is added, split, moved, superseded, or renamed:

- update the canonical router;
- update affected cross-references;
- retain a compatibility route while current tooling still requires an old
  path;
- run documentation checks.

## Conversation continuity

If a fact is needed for a future development session to operate correctly, it
must not exist only in chat history.

Record it in the appropriate repository authority while it is fresh.

## Validation

Run:

    scripts/check.sh

The goal is not merely valid Markdown. The goal is a coherent authority graph.

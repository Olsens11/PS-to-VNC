# Temporal State Semantics

## Purpose

PS-to-VNC contains documents that record what was believed to be current at a
particular point in development. Those records are useful historical evidence,
but they must not be mistaken for timeless present-tense authority.

A statement such as "we are working on X", "NEXT_ACTION=Y", or "service Z is
running" is only a claim about the present **at the time that state was
recorded** unless it has been reconciled with newer authority.

## Core rule

> **Current-state records are timestamped snapshots, not eternal facts.**

Every mutable current-status surface should carry an explicit ISO-8601
`RECORDED_AT` / `STATE_RECORDED_AT` timestamp with timezone offset, or an
equivalent visible "Recorded at" field.

When reading a status snapshot later:

- treat it as authoritative for what the project recorded at that timestamp;
- do not automatically describe its present-tense fields as true now;
- first compare it with newer repository commits, branch/PR/issue activity,
  machine/live state, and any later status snapshots;
- if freshness cannot be proved, say **"recorded as of <timestamp>"** rather
  than **"currently"**.

A month-old status document may still contain durable facts, but its
`ACTIVE_*`, `CURRENT_*`, `NEXT_*`, `BLOCKED_*`, `RUNNING_*`, progress, and
workstream claims are historical snapshots until reconciled.

## Two kinds of information inside a status record

### Freshness-sensitive state

These claims can become stale simply because development continues:

- active issue, PR, branch, test, or workstream;
- current phase;
- next intended action;
- blockers;
- running services/apparatus;
- current deployment;
- progress percentages;
- candidate/adoption state that may later be promoted or rejected;
- branch classifications and heads;
- operator/session status.

A reader must reconcile these against newer evidence before using present-tense
language.

### Durable recorded facts

These do not become false merely because the record is old, although a later
record may supersede their interpretation:

- a commit SHA that existed;
- a build or PT_LOAD digest;
- a hardware result that was actually observed;
- a decision that was adopted at that time;
- a test result or failure that occurred;
- a historical branch head;
- a timestamped measurement.

Age changes their temporal role from "current context" to "historical fact"; it
does not erase them.

## Freshness precedence

Do not use age alone as a magic staleness threshold. A five-minute-old snapshot
can already be stale if a later authoritative event occurred, while an older
architecture invariant can remain valid for months.

For freshness-sensitive claims, use this order:

1. reconciled live/local state when available and identity-valid;
2. later explicit state snapshots;
3. current GitHub issue/PR/branch/commit state;
4. current repository authority on the relevant branch/main;
5. older timestamped snapshots as historical context.

If two sources disagree, investigate the transition rather than silently
choosing one.

## Required metadata for status-like documents

A human-readable mutable status document should normally expose near its top:

```text
Temporal role: SNAPSHOT
Recorded at: 2026-09-02T15:01:00-04:00
```

A machine-readable state file should normally expose equivalent fields such as:

```text
STATE_TEMPORAL_ROLE=SNAPSHOT
STATE_RECORDED_AT=2026-09-02T15:01:00-04:00
```

Where useful, also record the exact repository/branch/head from which the
snapshot was produced.

The timestamp says **when the claims were recorded**, not that they remain
correct indefinitely.

## Historical status files

Do not rewrite old status snapshots merely to make their present-tense wording
look current. Preserve them as historical records when they are intentionally
retained.

If an old file is still named or routed as a current-state authority, either:

- update it with a new timestamp and reconciled contents; or
- demote/reroute it explicitly as historical/superseded.

Do not leave a stale file labeled `CURRENT` merely because nobody has updated
it yet.

## Fresh-session behavior

A fresh development session must inspect timestamps before repeating any
present-tense status claim.

For each status-like source, ask:

1. When was this recorded?
2. What exact branch/head or live identity did it describe?
3. Has anything authoritative happened since then?
4. Is the statement durable history or freshness-sensitive state?
5. Can I independently prove it is still current?

Until the last question is yes, use temporal language such as:

- "As of the 2026-08-20 snapshot..."
- "The repository recorded X at that time..."
- "This was the next action when the snapshot was written..."

Do not say "we are currently doing X" solely because an older status document
uses present tense.

## Relationship to branch/workstream indexes

Branch/workstream inventories are also snapshots. They must carry a timestamp
and exact observed heads. A later session should verify GitHub before relying on
the classifications.

This is particularly important for `ACTIVE`, `CANDIDATE`, `SUPERSEDED`, and
`SAFE_TO_RETIRE` classifications because branch state can change without the
older index changing with it.

## Mechanical enforcement direction

The repository should eventually make missing temporal metadata a mechanical
continuity error for designated status-like files. Until that check is added,
this document is the governing semantic rule and fresh-session reconstruction
must enforce it procedurally.

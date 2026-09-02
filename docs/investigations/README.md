# Major Investigation Casebook

## Purpose

This directory is the canonical index for **major technical investigations** that
must remain easy to find while they are active and after they are resolved.

A major investigation is not every bug, failed test, or implementation task. It
is a problem whose history, evidence, workaround, or root cause is important
enough that future design could be harmed if the reasoning is forgotten.

Typical reasons to open a case include:

- a failure spans more than one subsystem or initially points at the wrong layer;
- a workaround becomes part of the qualified runtime before its root cause is
  completely understood;
- the issue is intermittent, difficult to reproduce, or expensive to isolate;
- the issue creates an architectural constraint that future work must remember;
- hardware behavior, SDK behavior, protocol behavior, or undocumented platform
  behavior is involved;
- resolving the issue required a substantial experiment/research campaign;
- a future maintainer could reasonably rediscover the same wrong theory if the
  investigation is not preserved.

Do **not** create cases merely to manufacture a backlog. Ordinary implementation
work belongs in ordinary issues, PRs, tests, and current-state documentation.

## Case model

Each major investigation has two complementary surfaces:

1. a durable report under `docs/investigations/`;
2. a GitHub issue when ongoing discussion, experiments, or chronology benefit
   from a working thread.

The repository report is the durable technical case file. GitHub is the working
conversation/history. Evidence remains in its canonical evidence/test location;
the case report links it rather than copying raw evidence indiscriminately.

A case begins **before** root cause is known. It should be updated as hypotheses
are eliminated, new evidence arrives, workarounds are qualified, and the final
mechanism becomes clearer.

When a case is resolved, the same report becomes its closeout/service-bulletin
record. Do not replace the investigation history with a sanitized final answer.

## Status vocabulary

- `OPEN` — root cause or durable disposition remains unresolved.
- `OPEN_NON_BLOCKING` — unresolved, but a qualified workaround or safe boundary
  permits unrelated development to proceed.
- `MONITORING` — a corrective action is deployed, but more operating evidence is
  intentionally being accumulated before closeout.
- `RESOLVED` — the project has enough evidence to state the durable mechanism,
  corrective contract, qualification result, and remaining limits.
- `SUPERSEDED` — another case/report now owns the question; preserve the old case
  only as history and link the replacement.

`RESOLVED` does not require omniscience about every transistor, ROM-internal
cycle, or undocumented implementation detail. It requires enough understanding
to state what failed, why the chosen correction is justified, what evidence
supports it, what explanations were rejected, and what residual unknowns do not
invalidate the engineering conclusion.

## Required case contents

Every case should preserve, as applicable:

- symptom and impact;
- discovery/history;
- exact scope and affected layers;
- what is known with evidence grade;
- what remains suspected or unknown;
- hypotheses considered and rejected;
- experiments and discriminators that mattered;
- workaround/corrective action and its qualification status;
- architecture/design guardrails while unresolved;
- future experiment plan or closure criteria;
- exact source, evidence, issue, PR, branch, and external-reference pointers;
- final durable lesson once resolved.

Use `TEMPLATE.md` when opening a new case.

## Working rule

When a new major issue appears, create or promote its case report **early**.
Update that report during the investigation instead of relying on chat memory or
a long chain of disconnected issue comments. Substantive chronological comments
can remain in the GitHub issue, but durable conclusions belong back in the case
report.

Before changing a subsystem, check this casebook for open or resolved cases that
constrain the design. A resolved case may explain a current invariant just as an
open case may explain a temporary workaround.

## Current cases

| Case | Status | Subject | GitHub | Durable engineering rule |
| --- | --- | --- | --- | --- |
| `MI-001` | `RESOLVED` | GS/EE HIRES HSync interrupt handoff and `ExitHandler()` | resolved historical Test14 campaign; successor tracking issue to be linked here | HSync delivery is suppressed for the callback-owned transaction, ordinary HSINT ACK/work is preserved, delivery is restored immediately before `ExitHandler()`/return. |
| `MI-002` | `OPEN_NON_BLOCKING` | MTU1458 / SMAP RX corruption | Issue #18 | Keep the qualified MTU1458 workaround isolated at the PS2 networking seam; prefer eventual standard MTU1500 if the low-level cause can be fixed or avoided cleanly. |

Case reports:

- `MI-001-GS-INTERRUPT-EXITHANDLER.md`
- `MI-002-MTU1458-SMAP-RX.md`

## Relationship to other documentation

This casebook is not another current-architecture authority.

- `docs/status.md` says what is current now.
- `docs/CLEAN_ARCHITECTURE.md` says how the clean system is intended to be
  structured.
- `docs/audit/HISTORICAL_LESSONS.md` extracts durable cross-project lessons.
- `evidence/` and test documents contain empirical records.
- this directory explains major failures/investigations from symptom through
  closeout or current unresolved state.

If a case changes current architecture or status, update those authorities too.
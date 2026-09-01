# PS-to-VNC Semantic Audit

## Purpose

The semantic audit determines what PS-to-VNC actually is and what each
meaningful component does before clean implementation structure is chosen.

The governing product intent is:

    docs/PROJECT_INTENT.md

GitHub Issue #1 is the umbrella audit.
GitHub Issue #2 owns the behavioral inventory.

## Core rule

Historical implementation structure is evidence, not architecture authority for
the clean reconstruction.

This includes pre-reboot material under:

- `baseline/frozen-b4a/`;
- `working/b4a/`;
- existing pre-reboot `src/` modules;
- migration-era architecture and normalization documents;
- historical evidence and TestKit results.

A mechanism may be adopted because evidence shows it is useful or necessary.
A file/module boundary is not adopted merely because it already exists.

## Per-behavior audit contract

For each meaningful behavior or component, capture where applicable:

1. purpose and user-visible responsibility;
2. inputs and triggers;
3. owned mutable state;
4. normal behavior;
5. outputs and side effects;
6. dependencies;
7. failure behavior;
8. invariants;
9. B4A/current implementation references;
10. historical experiments and evidence;
11. durable lessons;
12. clean-rebuild implications.

## Evidence classes

Audit statements should distinguish these classes explicitly:

- `PRODUCT_REQUIREMENT` — deliberate desired product behavior;
- `SOURCE_SUPPORTED` — directly supported by preserved implementation;
- `TEST_SUPPORTED` — demonstrated by a controlled experiment/test;
- `HARDWARE_PROVEN` — directly qualified on physical PS2 hardware;
- `HISTORICAL_DESIGN` — prior design/architecture decision, not automatically
  binding on the clean reconstruction;
- `INFERENCE` — plausible interpretation not yet proven strongly enough for a
  stronger class;
- `OPEN_QUESTION` — unresolved behavior or ownership question.

Do not silently promote inference into fact.

## Entry maturity

Behavioral-inventory entries use these maturity states:

- `SEEDED` — responsibility is known to exist and evidence roots are identified;
- `SOURCE_MAPPED` — relevant implementation/state has been located;
- `EVIDENCE_SUPPORTED` — important behavior is connected to historical tests or
  evidence;
- `REBUILD_READY` — purpose, behavior, state, failures, invariants, and rebuild
  implications are sufficiently understood to design the clean replacement.

`SEEDED` does not mean audited.

## Audit sequence

The default order is:

1. complete the behavioral inventory;
2. map source/state ownership to those behaviors;
3. mine historical experiments and failures;
4. resolve contradictions and open questions;
5. derive clean module ownership only from the resulting behavioral model.

The audit may move between these steps when evidence requires it.

## Durable outputs

Current audit outputs:

- `BEHAVIORAL_INVENTORY.md` — complete behavior/responsibility inventory and
  per-domain audit state.

Additional focused documents should be created only when a domain needs enough
detail that keeping it in the inventory would make the inventory harder to use.

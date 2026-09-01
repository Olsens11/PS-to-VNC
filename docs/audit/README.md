# PS-to-VNC Semantic Audit

## Purpose

The semantic audit determines what PS-to-VNC actually is and what each
meaningful component does before clean implementation structure is chosen.

The governing product intent is:

    docs/PROJECT_INTENT.md

GitHub Issue #1 is the umbrella audit.
GitHub Issue #2 owns the behavioral inventory.
GitHub Issue #3 owns the historical source/responsibility map.
GitHub Issue #4 owns the durable historical lessons ledger.
GitHub Issue #6 owns the clean ownership/architecture derivation.

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
- `REBUILD_READY` — purpose, behavior, state, failures, invariants, ownership,
  interfaces, and rebuild implications are sufficiently understood to guide the
  clean replacement.

`SEEDED` does not mean audited.

`REBUILD_READY` does not mean a historical implementation/file split is accepted
unchanged, and it does not freeze every future implementation choice. It means
the behavior and ownership contract are strong enough to reconstruct without
rediscovering why the old program worked.

## Audit sequence

The completed semantic-audit sequence is:

1. build the B01-B14 behavioral inventory;
2. map historical source/state/coupling to those responsibilities;
3. mine historical experiments and failures for durable lessons;
4. reconcile evidence strength and preserved uncertainty;
5. derive cross-domain ownership, interfaces, concurrency, and lifecycle;
6. verify startup/live/display-transition/recovery flows against the ownership
   model;
7. promote the evidence-derived clean architecture.

Exact Pi package adoption and bottom-up implementation remain separate clean
reconstruction work; they do not reopen the semantic audit unless new evidence
contradicts an audited behavior.

## Durable outputs

Current audit outputs:

- `BEHAVIORAL_INVENTORY.md` — complete behavior/responsibility inventory;
- `B07_B09_INPUT_KEYBOARD_LOCAL_UI.md` — detailed input/UI audit;
- `B10_B11_CONFIGURATION_RECOVERY_MANAGEMENT.md` — detailed config/recovery
  audit;
- `B12_B14_DIAGNOSTICS_PI_DEVELOPMENT_INFRASTRUCTURE.md` — detailed final seeded
  tranche;
- `SOURCE_RESPONSIBILITY_MAP.md` — responsibility to historical source/state
  mapping and hidden coupling;
- `HISTORICAL_LESSONS.md` — durable experiment/failure lessons and preserved
  uncertainty;
- `CROSS_DOMAIN_SYNTHESIS.md` — responsibility-level architecture derivation;
- `CROSS_DOMAIN_STATE_INTERFACES.md` — state ownership, interfaces, concurrency,
  and critical-flow verification.

The promoted current architecture authority is:

    docs/CLEAN_ARCHITECTURE.md

Additional focused audit documents should be created only when new evidence or a
future subsystem requires enough detail that keeping it in the existing outputs
would make them harder to use.

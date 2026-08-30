# PS-to-VNC Modularization

## Rule

A modularization stage consists of both:

1. source migration;
2. documentation migration.

Stage M(n+1) must not begin until M(n) source, validation and documentation are
complete.

## Historical baseline

The immutable behavioral reference lives under:

`baseline/frozen-b4a/`

Never edit that directory as part of modular development.

The dependency analysis lives under:

`baseline/analysis/B4A-dependency-map/`

## Planned module families

The target architecture includes:

- core
- net
- rfb
- display
- input
- ui
- config
- management
- diag
- media

Exact files and APIs are established incrementally from the dependency graph,
not by arbitrary source splitting.

## Naming

New modular APIs normally use `pstvnc_`.

Imported historical symbols retain their existing names until a controlled
migration stage owns that change.

No global mass rename is permitted as part of a mechanical extraction stage.

## Stage ledger

| Stage | Purpose | Working source | Validation | Documentation | Status |
|---|---|---|---|---|---|
| Import | Create independent PS-to-VNC repository and immutable baseline | none | provenance/hash separation | successor docs created | IN PROGRESS |
| M0 | Reproduce exact B4A behavior as PS-to-VNC working authority | TBD | build + identity + hardware baseline as required | source map/state/build authority | NOT STARTED |
| M1+ | Mechanical module extraction | defined from dependency graph | build + staged regression | location/ownership ledger | NOT DEFINED |

## Required record per stage

Every stage must record:

- exact functions/types/constants moved;
- exact state moved;
- old location;
- new location;
- module owner;
- public API;
- private state;
- remaining shared state;
- deliberate temporary dependency debt;
- build identity;
- validation result;
- hardware result when required;
- source commit;
- documentation commit;
- next stage.

## Historical versus living documentation

Historical documentation is never rewritten merely because code has moved.

Living PS-to-VNC reference documentation must always identify the current
source and ownership location.

## Conversation-limit recovery invariant

Recoverability is part of stage completion.

Every migration stage must update both:

- `runtime/MIGRATION_STATE.env`
- `docs/MIGRATION_STATE.md`

A stage must leave an explicit `NEXT_ACTION`.

Stage M(n+1) must not begin while M(n) is still recorded as incomplete or
while `scripts/migration-check.sh` fails.

Every source-changing migration command should verify the stage and repository
authority it expects before performing the mutation.

Historical PS2VNC authority remains outside the writable successor boundary at:

`/home/ps2/ps2vnc`

The recovery system exists specifically so project state does not depend on
the lifespan of a conversation.

## Immutable evidence formatting

Files under the frozen historical evidence and analysis trees are preserved
for provenance, not reformatted for repository style.

In particular:

- `baseline/frozen-b4a/`
- `baseline/analysis/B4A-dependency-map/`

may contain historical trailing whitespace, patch formatting, generated TSV
formatting, or other byte-level characteristics inherited from the preserved
source material.

Do not modify immutable evidence merely to satisfy whitespace or formatting
lint.

For those paths, preservation and recorded identity take precedence over style.

Whitespace checking for a commit must still be strict for writable
PS-to-VNC-authored source, scripts, runtime state, and living documentation.

## Migration versus maturation

The migration phase prioritizes behavioral equivalence, provenance, and clear
ownership transitions over cosmetic cleanup.

Do not mix broad cleanup with mechanical module extraction.

After migration and regression establish a stable modular authority, a
separate maturation phase may deliberately improve formatting, naming,
organization, build hygiene, warnings, tooling, documentation, packaging, and
other maintainability concerns.

Frozen historical evidence remains provenance. Cleanup should target the
working project or explicitly derived copies rather than silently changing the
bytes used to identify the historical baseline.


## M0 completion / M1 entry

M0 is COMPLETE.

The successor independently reproduced the exact historical B4A ELF bytes and
resolved the M0 hardware requirement without claiming a new physical hardware
PASS.

M1 is the first mechanical module-extraction stage.

Its exact extraction candidate is intentionally selected in a read-only M1A
dependency-map pass before implementation mutation.

Authority:

    docs/M1_EXTRACTION_CONTRACT.md

## M1A selected boundary

The first mechanical extraction will move the configuration-text whitespace
helpers:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

into a separate translation unit.

The selection is intentionally small:

- 19 function lines;
- contiguous historical source block;
- no project globals;
- no project callees;
- only config-parser callers.

Authority:

    docs/M1A_EXTRACTION_BOUNDARY.md

M1B may not begin until the PS-to-VNC GitHub publication gate passes.

## M1 publication gate

The M1A publication gate is COMPLETE.

The independent PS-to-VNC GitHub repository is:

    Olsens11/PS-to-VNC

Visibility:

    PRIVATE

M1B may now begin the committed config-text mechanical extraction.

## M1B first source extraction complete

The configuration-text trim pair is now a separate translation unit.

Source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

The extraction remained mechanical and did not move any additional
configuration parser logic or state.

M1C must now perform the first clean modularized build and establish the new
DUT identity.

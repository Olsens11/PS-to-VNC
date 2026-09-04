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
while the preceding migration stage remains recorded as incomplete.

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

## M1C first modular build complete

The M1B config-text extraction builds successfully as a separate translation
unit.

The resulting ELF is a new executable identity:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

A second clean build reproduced that identity exactly.

The moved trim behavior passed a deterministic M0-vs-M1 host parity test.

M1D hardware regression remains mandatory before M1 may close.

## M1D hardware gate

The first extracted module passed the required real-hardware regression.

Machine result: PASS.

Physical result: PASS.

Startup gate: PASS.

The migration can proceed to M1E closure and selection of the next mechanical
extraction boundary.

## M2 — configuration scalar parsing

M1 is complete.

M2A extends the existing configuration-text module with two mechanically
selected scalar parsers:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

Both selected functions are free of global references and cross-module
outgoing project calls.

M2B will move the exact bodies without caller or behavior changes.

## M2B mechanical extraction

M2B moved the two selected scalar parsers into
`working/b4a/ps2vnc_config_text.c`.

The move changed linkage only as required to cross the translation-unit
boundary.

Callers remained unchanged.

The existing `ps2vnc_config_text.o` build linkage remained unchanged.

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

Next:

    M2C_build_and_characterize_scalar_parser_DUT

## Accelerated migration policy

M1 and M2 established the safe mechanical-extraction process.

Beginning with M3, migration units should be coherent clusters rather than
individual helpers whenever dependency evidence permits.

Low-risk waves use automated build, linkage, parity and structural checks.

Physical PS2 regression gates are reserved for hardware-facing or materially
coupled boundaries.

Next:

    M3A_plan_accelerated_extraction_waves

## Hierarchical macro migration

M3 no longer requires each intermediate split to be a final independent
translation unit.

Large implementation regions may first be moved intact, validated, and then
recursively subdivided.

The current macro body is:

    working/b4a/ps2vnc_macro_body.inc

Next:

    M3B_recursive_macro_split

## M3B top-level hierarchy

The 18,313-line macro implementation has been divided into three large
branches while remaining in the same translation unit.

The next pass recursively subdivides those branches rather than returning to
small one-function migrations.

Next:

    M3C_recursive_split_macro_regions

## M3C recursive hierarchy

The M3 implementation tree now contains six leaves rather than three
approximately 6,000-line regions.

The next step classifies these leaves by actual function/module content and
begins promoting coherent regions toward real subsystem boundaries.

Next:

    M3D_classify_six_macro_leaves_and_promote_subsystems

## First independently compiled large subsystem region

The migration scaffold has now crossed a genuine compiler/linker boundary.

Management/configuration and calibration/state compile in
ps2vnc_services.o independently of the main ps2ip.o runtime unit.

The current compatibility headers are intentionally broad migration
scaffolding. They preserve behavior while physical decomposition proceeds;
interface reduction remains later work.

Next:

    M3F_hardware_validate_first_real_translation_unit_split

## First real translation-unit boundary qualified on hardware

The first compiler/linker ownership boundary is now hardware-qualified.

Detached TU:

`management_config + calibration_state`

Original migrated-region authority:

`6379` source lines

This checkpoint proves that the successor can cross a substantial real
translation-unit boundary on the physical PS2 while preserving the validated
display-mode matrix. Continued M3 work should keep decomposing the remaining
large implementation regions rather than reverting to same-TU fragment-only
organization.

## M3G three-object architecture

M3G establishes three independently linked ownership regions:

1. shared startup-state shell
2. coarse runtime implementation
3. management/configuration/calibration services

The runtime region keeps its existing semantic leaves together for this move;
those leaves are the next recursive decomposition surface.

## M3H hardware-qualified three-object boundary

The current coarse architecture is now hardware-qualified:

1. `ps2ip.c` — canonical state/primary-source shell, 0 text symbols
2. `ps2vnc_runtime.c` — 11933-line coarse runtime implementation
3. `ps2vnc_services.c` — 6379-line services implementation

M3H passed startup plus the five-mode hardware sweep. The next decomposition
therefore proceeds recursively inside `ps2vnc_runtime.c`, rather than revisiting
the already-qualified services split.

# M1 First Mechanical Module Extraction Contract

## Purpose

M1 begins behavioral modularization of the PS-to-VNC application.

M1 is the first stage that may move implementation out of the historical
18,938-line monolith.

M1 is not a cleanup stage.

## Entry authority

M1 begins from the completed M0 authority:

    working/b4a/ps2ip.c

M0 source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

M0 reference/reproduced ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

At M1 entry the working source remains byte-identical to frozen B4A.

## Why the working path remains working/b4a

The `working/b4a/` name records the lineage of the current writable tree.

M1 will not rename the whole tree merely for aesthetics before extraction.

Path normalization and mature repository cleanup are separate changes.

Once M1 source mutation begins, the contents of this directory will no longer
be assumed byte-identical to B4A even though the lineage directory name
remains temporarily unchanged.

## M1 goal

Extract one deliberately selected low-coupling responsibility from the
monolith into a real separate translation unit with a narrow interface.

The extraction must be mechanical and behavior-preserving by design.

## M1A — exact target selection

M1A is read-only with respect to implementation source.

It must use the committed B4A dependency map under:

    baseline/analysis/B4A-dependency-map/

M1A must identify the exact first extraction candidate before any source is
moved.

Selection should prefer:

- small leaf or near-leaf functions;
- narrow responsibility;
- few callers;
- few referenced globals;
- little or no ownership of mutable shared state;
- no orchestration responsibility;
- no controller-thread or display-live-desktop giant;
- no simultaneous state-ownership redesign.

M1A must record:

- exact functions to move;
- exact types/macros/constants required;
- globals referenced;
- callers;
- callees;
- proposed new module path;
- proposed public header;
- symbols that remain private;
- any temporary shared-state debt;
- expected build/link consequences.

The exact extraction target is not chosen by M0G.

## M1B — mechanical extraction

Only after the M1A contract is committed may implementation source change.

The selected implementation should be moved with minimal textual change.

Do not combine extraction with:

- formatting cleanup;
- symbol renaming;
- API redesign beyond the minimum boundary required;
- algorithm changes;
- warning cleanup;
- configuration changes;
- display behavior changes;
- RFB behavior changes;
- controller behavior changes;
- recovery-policy changes.

## Translation-unit identity rule

Moving code into a separate translation unit can change:

- optimization;
- inlining;
- LTO behavior;
- section layout;
- debug information;
- symbol layout;
- final executable bytes.

Therefore the first modularized ELF is expected to be a new DUT identity even
when source behavior was intended to remain unchanged.

M1 must not require the modularized final ELF to retain the B4A SHA256.

## Validation rule

M1 must establish at minimum:

1. exact documented extraction boundary;
2. successful clean build;
3. no unexplained source changes outside the extraction boundary;
4. documented new ELF identity;
5. static/dependency review of the moved interface;
6. recovery/documentation coherence.

Because the executable DUT identity changes, M1 requires hardware regression
before M1 may be declared COMPLETE.

A successful compiler/linker result alone is not a hardware PASS.

## Hardware philosophy

Machine observations and physical observations remain separate.

A modularized build is new executable evidence.

Historical B4A hardware evidence remains the behavioral reference, but it
cannot automatically validate different M1 executable bytes.

## Documentation lockstep

Every M1 mutation must update the living documentation in the same stage.

At completion, a fresh conversation must be able to recover:

- what moved;
- where it moved from;
- where it lives now;
- module owner/responsibility;
- public API;
- private state;
- temporary coupling debt;
- source commit;
- ELF identity;
- build result;
- hardware result;
- next extraction stage.

## M1 phase plan

M1A — select and document the exact low-coupling extraction boundary.

M1B — create the new module and mechanically move the selected implementation.

M1C — build and characterize the new executable identity.

M1D — perform required regression validation.

M1E — close M1 and define the next extraction.

## Current status

M1A:

    COMPLETE

Selected first boundary:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Selection authority:

    docs/M1A_EXTRACTION_BOUNDARY.md

Implementation source mutation:

    NONE

Publication gate:

    REQUIRED BEFORE M1B

Next:

    PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository

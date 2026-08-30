# PS-to-VNC Architectural Normalization Sequence

## Strategy

Normalize from low-risk cohesive leaves toward high-coupling orchestration.

Each tranche should establish a real permanent module, migrate its state and
API deliberately, validate the build/load relation, and retire only the
migration scaffolding made obsolete by that tranche.

Do not perform a mass rewrite.

## Tranche 1: config text

Status: **COMPLETE — M4F**

Target:

    src/config/text.c
    src/config/text.h

Starting implementation:

    ps2vnc_config_text.c
    ps2vnc_config_text.h

Reasons to begin here:

- already a coherent module;
- four project-local helpers;
- hardware-independent;
- easy host-test target;
- establishes nested `src/` build support;
- establishes permanent `pstvnc_` naming;
- exercises the normalization workflow at low risk.

Add host unit coverage for:

- trim-left;
- trim-right;
- integer parsing;
- boolean parsing.

## Tranche 2: video mode and pure geometry

Status: **COMPLETE — M4G + M4H**

Move mode lookup/backend properties and pure geometry calculations out of
the runtime state warehouse.

These are relatively pure and strongly host-testable.

## Tranche 3: diagnostics

Status: **NEXT — M4I**

Assign debug/profiling state to diagnostics modules.

This reduces unrelated mutable state from runtime without changing product
behavior.

## Tranche 4: management

Separate:

- HTTP framing;
- bounded client transport;
- display transaction serialization;
- remote display policy.

Recovery policy and local display switching remain outside management.

## Tranche 5: RFB transport and session

Establish explicit ownership of:

- connection;
- receive buffering;
- send queue;
- handshake;
- negotiated encoding.

## Tranche 6: RFB update/decode

Separate protocol rectangle/update decoding and Hextile from video
presentation.

Introduce the smallest useful frame-sink/update interface.

## Tranche 7: video framebuffer/mapping/presentation

Consolidate framebuffer buffers, mapping caches, direct-write/publish logic,
and local presentation ownership.

## Tranche 8: PS2 platform modules

Move PS2-specific graphics, high-resolution GS support, network bootstrap,
and pad mechanisms behind recognizable PS2 platform modules where the seam
has real value.

Generated IRX C inputs move conceptually/build-wise under
`build/generated/irx`.

## Tranche 9: input

Normalize controller polling, hotkey/binding recognition, and logical input
actions.

Input stops directly owning product effects.

## Tranche 10: UI

Normalize:

- display menu;
- system menu;
- OSK;
- overlays/confirmation rendering.

Move UI state from the historical runtime global set.

## Tranche 11: calibration

Normalize calibration geometry/state and interaction after video, input,
and UI boundaries are established.

## Tranche 12: application orchestration

Move:

- startup reconciliation;
- display transition orchestration;
- rollback;
- recovery policy

into explicit app modules.

This is intentionally late because those responsibilities currently bridge
many subsystems.

## Tranche 13: main

Move `main` last.

The permanent `main.c` should become small naturally because it composes
already-normalized APIs.

Do not produce a cosmetically small `main.c` by simply moving its globals
into another giant helper.

## Tranche 14: retire migration scaffolding

Remove remaining:

- implementation `.inc` partitions;
- core/tail names;
- directional migration headers;
- broad compatibility headers;
- historical root working-source organization

only after their consumers have real module contracts.

## Per-tranche validation

For each architectural tranche:

1. establish exact pre-change source/binary authority;
2. make one coherent responsibility move;
3. compile with no unresolved ownership ambiguity;
4. add/maintain host tests where applicable;
5. build twice when the change affects the PS2 executable;
6. compare ELF and PT_LOAD identity to the preceding authority;
7. if load image is unchanged, hardware qualification is normally not
   required solely for structural movement;
8. if a new compiler boundary or semantic relocation produces a
   nonidentical load image, perform the appropriate PS2 hardware checkpoint;
9. land source authority before hardware qualification;
10. retire only the migration interfaces made obsolete by the tranche.

## Versioning

The existing tag:

    v0.1.0-alpha.1

remains immutable as the post-migration baseline.

Ordinary development commits after that tag remain under `Unreleased`.

A new product tag is created at a meaningful architecture/capability
checkpoint rather than for every normalization commit.

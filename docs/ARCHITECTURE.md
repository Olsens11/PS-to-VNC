# PS-to-VNC Architecture

## Status

This document is normative for new PS-to-VNC development beginning with
the `0.1.0-alpha.1` baseline.

The currently hardware-qualified implementation still resides primarily
under `working/b4a/` and retains migration-era `.inc` partitions. Those
files are the qualified starting point, not the desired permanent source
layout.

No implementation should be moved merely to make a file smaller. Future
decomposition is semantic.

## Architectural goals

PS-to-VNC should follow conventional, maintainable embedded-C design.

The architecture should make it possible for a developer unfamiliar with
the migration history to determine:

- which subsystem owns a responsibility;
- which module owns mutable state;
- which interfaces other modules may call;
- which code is PS2-platform-specific;
- which logic can be tested on a host computer;
- and which direction dependencies are intended to flow.

The repository should teach the architecture through ordinary source
organization rather than through migration archaeology.

## Core principles

### 1. Modules represent responsibilities

A source module exists because it owns a coherent software responsibility,
not because a historical file reached a particular size.

Ordinary implementation modules should normally use conventional adjacent
C source and header pairs:

    module.c
    module.h

A private header may be used when implementation shared by several files
within one subsystem genuinely requires it:

    module_internal.h

Private implementation details should remain private.

### 2. One clear owner for mutable state

Important mutable state must have an identifiable owning module.

Other modules should interact with that state through the owner's API
rather than through broadly visible writable globals.

New unowned global mutable state is not permitted.

Existing migration-era globals should be assigned owners during
architectural normalization.

### 3. Narrow interfaces

A symbol becomes externally visible because it is part of a deliberate
module contract, not merely because the linker requires access to it.

Implementation helpers should be `static` whenever practical.

Headers should expose only the types, constants, and functions required by
legitimate consumers.

### 4. Dependency direction is intentional

Circular dependency growth is not accepted as a normal consequence of
splitting files.

When two modules appear to require each other's internals, first determine
whether:

- responsibility is assigned incorrectly;
- a shared lower-level concept should be extracted;
- a callback or explicit interface should invert one dependency;
- or the modules should actually be one cohesive module.

Dependencies should generally flow from orchestration toward domain
services and then toward lower-level platform mechanisms.

### 5. Platform-specific code is contained

PS2 hardware and PS2SDK details should be recognizable as platform code.

Examples include:

- GS and display hardware operations;
- DMA and cache-sensitive operations;
- PS2 controller access;
- IOP/module handling;
- PS2SDK-specific networking;
- platform timing and low-level system facilities.

Containing platform code is valuable even if PS2 remains the only supported
platform. The goal is separation of concerns and testability, not
portability for its own sake.

### 6. Protocol logic and hardware presentation are different domains

RFB/VNC protocol handling should not be structurally fused to PS2 graphics
implementation.

Protocol code owns concepts such as:

- RFB negotiation;
- server messages;
- framebuffer-update framing;
- rectangles;
- encodings;
- Raw and Hextile decoding;
- protocol validation and reconnect/error semantics.

Video/platform code owns concepts such as:

- CPU framebuffer representation;
- display modes;
- display profiles;
- calibration;
- textures and presentation;
- GS/DMA implementation.

Their boundary should be explicit.

### 7. Policy and mechanism are distinguishable

Mechanisms should not unnecessarily encode product policy.

For example:

- reconnecting a socket is mechanism;
- deciding whether a silent stall should reconnect automatically is policy;
- restoring a display mode is mechanism;
- deciding when a restore should occur is policy.

This keeps recovery behavior and product decisions understandable and
changeable.

### 8. Important state machines are explicit

Subsystems with meaningful lifecycle states should model those states
deliberately rather than accumulate unrelated booleans.

Likely examples include:

- RFB session state;
- display transactions;
- display-mode transitions;
- recovery;
- UI/menu state.

Transitions and ownership should be documented with the owning module.

### 9. Host-testable logic should remain host-testable

Pure parsing, validation, geometry, protocol, and state-transition logic
should avoid unnecessary PS2 hardware dependencies.

This allows ordinary host-side unit tests to cover logic that does not
require a physical console.

Physical PS2 tests remain the integration authority for hardware-facing
behavior.

### 10. Do not over-abstract

PS-to-VNC is an embedded C application, not a framework.

The project should prefer small explicit APIs over generalized component
systems, home-grown object frameworks, or abstractions that do not solve a
real dependency or testing problem.

Simple and conventional is preferred.

## Target source hierarchy

The exact module assignment will be determined by the architectural
responsibility census. The intended subsystem-level shape is:

    src/
        app/
        rfb/
        video/
        input/
        ui/
        config/
        management/
        platform/
            ps2/
        common/

    tests/
        unit/
        integration/

    tools/
    scripts/
    docs/

This hierarchy is a design target, not authorization to move code before
the responsibility and dependency census.

Headers used only by application modules should normally remain adjacent
to their implementation.

A top-level public `include/` tree should be introduced only if PS-to-VNC
develops an API that is genuinely consumed as an external interface.
Creating one merely because other projects use that layout is not a goal.

## Expected subsystem responsibilities

### app

Top-level application/session orchestration and lifecycle policy.

It coordinates subsystems but should not absorb their implementation.

### rfb

RFB/VNC protocol, framing, encoding negotiation, rectangle decoding, and
protocol-level session behavior.

### video

Framebuffer representation, display-mode model, display profiles,
calibration model, and presentation-facing abstractions.

### input

Logical input actions, controller/key mapping, chords, and configurable
bindings.

PS2 hardware polling belongs in the platform layer.

### ui

Menus, overlays, status presentation, and user-facing interaction flow.

### config

Configuration model, parsing, defaults, validation, and persistence
interfaces.

Consumers use configuration values; they should not each implement their
own parsing.

### management

Communication and transaction semantics between the PS2 application and
the Pi-side management service.

HTTP transport helpers, backend selection, and display transactions should
be assigned deliberately rather than remain grouped by historical source
position.

### platform/ps2

PS2SDK and hardware-facing implementation.

Higher-level modules should not acquire unnecessary direct knowledge of
PS2 hardware details.

### common

Only genuinely cross-cutting, low-level definitions belong here.

`common` must not become a miscellaneous dumping ground used to avoid
choosing a real owner.

## Migration scaffolding policy

The following patterns are considered transitional when they contain
ordinary implementation logic:

- migration `.inc` partitions;
- `core` and `tail` names that describe historical cut position;
- directional migration headers such as `*_core_to_tail.h`;
- broad compatibility headers exposing historical shared scope.

They are not removed in a mass rewrite.

They are retired incrementally as real semantic modules acquire deliberate
APIs and owners.

Generated include fragments or data tables may continue to use `.inc`
where that is genuinely appropriate.

## Naming policy

New permanent project symbols should use the neutral `pstvnc_` prefix.

Historical `ps2vnc_` names are not renamed merely for cosmetic cleanup
during structural movement. They should migrate when the owning module is
normalized and the rename can be reviewed as part of a coherent change.

File and directory names should describe domain responsibility rather than
migration history.

## Build and hardware qualification

Architectural changes must preserve the reproducible-build discipline
established during migration.

A new real compiler boundary or other change producing a nonidentical PS2
load image requires appropriate hardware qualification before becoming the
hardware authority.

Host-side tests supplement but do not replace real PS2 integration tests.

## Architectural acceptance

Architectural normalization is complete when:

- ordinary application logic no longer depends on migration `.inc`
  partitions for organization;
- `core`/`tail` placement is no longer needed to understand ownership;
- major mutable state has an explicit owner;
- module APIs are deliberate and narrow;
- dependency cycles are absent or explicitly justified;
- PS2-specific mechanisms are recognizably contained;
- significant pure logic has host-testable seams;
- configuration, management, RFB, video, input, and UI responsibilities
  have clear homes;
- architecture documentation reflects the actual source tree;
- a new developer does not need migration history to understand where
  functionality belongs.

Line count alone is not an architectural acceptance criterion.

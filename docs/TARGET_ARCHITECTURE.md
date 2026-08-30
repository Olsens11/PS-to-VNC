# PS-to-VNC Target Architecture

## Status

This document is normative for architectural normalization after the
`0.1.0-alpha.1` baseline.

It defines the intended permanent organization of application source.
Migration-era file position does not determine permanent ownership.

The design follows conventional embedded-C principles:

- responsibility-oriented modules;
- explicit ownership of mutable state;
- narrow C interfaces;
- deliberate dependency direction;
- platform-specific code isolated from domain logic;
- host-testable pure logic;
- minimal abstraction beyond what solves a real coupling problem.

## Permanent source hierarchy

The target hierarchy is:

    src/
        app/
            main.c
            startup.c
            startup.h
            display_transition.c
            display_transition.h
            recovery.c
            recovery.h

        rfb/
            session.c
            session.h
            transport.c
            transport.h
            framebuffer_update.c
            framebuffer_update.h
            hextile.c
            hextile.h
            desktop_resize.c
            desktop_resize.h
            input.c
            input.h

        video/
            mode.c
            mode.h
            geometry.c
            geometry.h
            framebuffer.c
            framebuffer.h
            mapping.c
            mapping.h
            display.c
            display.h
            calibration.c
            calibration.h

        input/
            controller.c
            controller.h
            bindings.c
            bindings.h
            action.h

        ui/
            display_menu.c
            display_menu.h
            system_menu.c
            system_menu.h
            osk.c
            osk.h
            overlay.c
            overlay.h

        config/
            config.c
            config.h
            text.c
            text.h

        management/
            client.c
            client.h
            http.c
            http.h
            display_transaction.c
            display_transaction.h
            display_policy.c
            display_policy.h

        diagnostics/
            debug.c
            debug.h
            profiling.c
            profiling.h
            identity.c
            identity.h

        platform/
            ps2/
                bootstrap.c
                bootstrap.h
                network.c
                network.h
                pad.c
                pad.h
                graphics.c
                graphics.h
                graphics_hires.c
                graphics_hires.h

        common/

    build/
        generated/
            irx/

    tests/
        unit/
        integration/

Not every proposed file must exist merely to satisfy this diagram.
A module is created when there is real implementation responsibility for it.

Conversely, a module may later be split when two independently coherent
responsibilities emerge. File size alone does not trigger that split.

## Application layer

`app` owns top-level orchestration.

It coordinates subsystems but does not implement their internal mechanisms.

### main

`main.c` should ultimately contain:

- process entry;
- top-level initialization ordering;
- construction of application/subsystem state;
- the primary application loop;
- final shutdown/exit routing.

It should not contain protocol decoding, graphics implementation,
configuration parsing, controller interpretation, or remote transaction
details.

### startup

Startup reconciliation belongs in `app/startup`.

This includes the lifecycle currently represented by startup display
transaction state and authoritative/confirmed startup display profiles.

### display transition

`app/display_transition` owns orchestration that spans:

- local video mode changes;
- RFB desktop-size behavior;
- Pi management transactions;
- confirmation;
- restore/rollback;
- transition-level failure policy.

This distinction is important:

- changing GS/display hardware is a `video` mechanism;
- coordinating a safe product-level display transition is application
  policy/orchestration.

### recovery

`app/recovery` owns product recovery policy.

It may coordinate RFB, management, video, and UI APIs.

RFB reconnect implementation itself remains in `rfb`.

## RFB subsystem

`rfb` owns the VNC/RFB protocol domain.

It owns:

- connection/session state;
- socket transport behavior specific to an RFB session;
- exact send/receive framing;
- RFB handshake;
- framebuffer-update framing;
- rectangle parsing;
- Raw/Hextile decoding;
- ExtendedDesktopSize negotiation;
- pointer/key event serialization;
- protocol-level errors and reconnect mechanisms.

RFB must not own PS2 GS state or UI menu state.

Framebuffer decoding should target a deliberate sink/presentation interface
rather than reaching into unrelated video globals.

The interface should be no more general than necessary. A small callback or
frame-sink contract is preferred over a framework.

## Video subsystem

`video` owns display and framebuffer concepts independent of RFB framing.

It owns:

- supported video-mode model;
- active video mode/backend;
- logical desktop geometry;
- presented/output geometry;
- framebuffer storage;
- high-resolution mapping;
- presentation;
- safe-area/calibration geometry.

### mode

`video/mode` owns mode definitions, lookup, backend compatibility, and mode
properties.

### geometry

`video/geometry` owns pure calculations involving logical desktop size,
presented geometry, output coordinates, and safe-area geometry.

It should be strongly host-testable.

### framebuffer

`video/framebuffer` owns framebuffer representation and update/publish
mechanisms after protocol data has been decoded.

### mapping

`video/mapping` owns high-resolution raster/vertical mapping and mapping
caches.

### display

`video/display` owns local display creation, presentation, flip/sync, and
mode-switch mechanism.

PS2-specific GS operations are delegated to `platform/ps2`.

### calibration

`video/calibration` owns calibration model and interaction logic.

Product orchestration that decides when calibration occurs remains above
the video layer.

## Input subsystem

`input` owns physical-controller interpretation and configurable logical
bindings.

It owns:

- controller state;
- controller polling/thread lifecycle;
- button/chord interpretation;
- hotkey recognition;
- logical action events;
- configurable binding representation.

PS2 pad API calls belong in `platform/ps2/pad`.

Input does not send RFB packets itself.

It emits logical actions; the application routes those actions to UI,
recovery, display-transition, or RFB input APIs as appropriate.

## UI subsystem

`ui` owns user-facing menus, overlays, and the on-screen keyboard.

It owns:

- display menu state/rendering;
- system menu state/rendering;
- confirmation/restore overlays;
- OSK state/rendering;
- menu navigation behavior.

UI should produce semantic actions rather than manipulate sockets or remote
transactions directly.

## Configuration subsystem

`config` owns configuration representation, parsing, validation, defaults,
and persistence-facing serialization rules.

### text

`config/text` contains small reusable text parsing helpers.

It is the first normalization candidate because it is already a coherent,
hardware-independent module.

### config

`config/config` owns the overall configuration model and section-level
parsing/validation.

Configuration does not own video behavior. It may represent video-related
configuration values using stable domain types.

## Management subsystem

`management` owns communication and data contracts with the Pi management
service.

### http

`management/http` owns the deliberately small HTTP framing/parser needed by
the management protocol.

It is not intended to become a general HTTP library.

### client

`management/client` owns bounded management GET/POST transport.

### display_transaction

`management/display_transaction` owns serialization/parsing of remote
display transaction messages.

It does not decide product-level transition policy.

### display_policy

`management/display_policy` owns retrieval/update of Pi-side display policy.

## Diagnostics subsystem

`diagnostics` owns development and runtime diagnostic facilities:

- debug-stage reporting;
- UDP diagnostic output;
- profiling counters/timing;
- build/source identity.

Diagnostic APIs may be used broadly.

Diagnostics must not acquire application-domain ownership merely because
many modules call it.

## PS2 platform subsystem

`platform/ps2` contains PS2SDK and hardware-specific mechanisms.

It owns appropriate wrappers/implementations for:

- IOP/IRX bootstrap;
- network-interface initialization/status;
- pad hardware access;
- GS/dmaKit operations;
- PS2-specific high-resolution graphics support.

A wrapper is introduced only where it creates a real boundary or test seam.

The project will not wrap every PS2SDK call merely for architectural
appearance.

## Generated build inputs

The generated embedded IRX C files are build artifacts, not application
source modules.

Their eventual home is:

    build/generated/irx/

They remain reproducibly generated or captured by the build process.

## Common

`common` is reserved for genuinely low-level concepts shared across
otherwise independent domains.

It must not become a dumping ground.

A helper belongs in `common` only when assigning it to another domain would
create a false dependency.

## Header policy

Headers remain adjacent to their implementation modules.

PS-to-VNC is an application, not currently a reusable C library, so a
top-level public `include/` hierarchy is not required.

Cross-subsystem headers expose deliberate contracts.

Headers named after migration direction, such as:

    *_core_to_tail.h
    *_tail_to_core.h
    *_runtime_to_*.h

are transitional and should disappear as domain APIs replace them.

## State model

Application-wide writable extern state is not part of the target
architecture.

Mutable state belongs to one subsystem.

Subsystem state may be represented by explicit context structures when
that materially improves ownership, testing, or lifecycle management.

The project should not replace current globals with one unrestricted giant
application-state structure.

## Naming

New permanent symbols use the `pstvnc_` prefix.

Existing `ps2vnc_` symbols are renamed when their owning module is
normalized, not in a mass cosmetic rename.

## Completion

Architectural normalization is complete when a developer can understand the
program through the permanent module hierarchy and APIs without needing to
understand migration-era `core`, `tail`, or `.inc` boundaries.

# PS-to-VNC Dependency Rules

## Purpose

The target architecture is a directed dependency graph, not merely a set of
directories.

These rules prevent migration-era global coupling from reappearing through
new headers.

## Base rules

### No upward dependency on app

No subsystem depends on `app`.

`app` is the composition/orchestration layer.

### Platform is lower level

`platform/ps2` does not depend on:

- app;
- ui;
- management;
- rfb;
- config.

Platform code implements PS2 mechanisms.

### RFB and video are siblings

`rfb` does not depend on video implementation.

`video` does not depend on RFB implementation.

Where decoded RFB pixels must reach video, use a small explicit
frame-update/frame-sink contract.

Where a display transition requires both RFB and video work, `app` performs
the coordination.

### Management and video are siblings

Management transport does not directly change GS/video state.

Application display-transition orchestration uses both APIs.

### UI does not own transport

UI does not call socket operations or management transaction internals.

UI produces semantic actions.

### Input does not own remote effects

Input interprets physical controls and produces actions.

It does not decide whether an action becomes:

- a UI navigation event;
- an RFB key/pointer event;
- a recovery request;
- a display-mode request.

Application/UI routing decides that.

### Configuration does not execute behavior

Configuration parses, validates, stores, and serializes settings.

Applying behavior belongs to the owning domain/application orchestration.

### Diagnostics is cross-cutting but lower-level

Other modules may call diagnostics.

Diagnostics must not call back into application domains to make product
decisions.

## Allowed high-level dependency direction

A useful conceptual direction is:

    app
      |
      +--> ui
      +--> input
      +--> config
      +--> management
      +--> rfb
      +--> video
      +--> diagnostics

    ui
      +--> video presentation API where required
      +--> stable input/action types where required

    config
      +--> stable domain value types where required

    management
      +--> common
      +--> diagnostics
      +--> platform network/time mechanisms where required

    rfb
      +--> common
      +--> diagnostics
      +--> platform network/time mechanisms where required

    video
      +--> common
      +--> diagnostics
      +--> platform/ps2 graphics mechanisms

    input
      +--> common
      +--> diagnostics
      +--> platform/ps2 pad mechanisms

    platform/ps2
      +--> PS2SDK/gsKit/dmaKit/etc.

## Circular dependencies

New circular subsystem dependencies are prohibited unless explicitly
reviewed and documented.

When a proposed split produces a cycle, investigate whether:

- one responsibility has been assigned to the wrong module;
- orchestration should move upward to app;
- a small shared domain type belongs at a lower layer;
- a callback/interface should invert one dependency;
- or the split is artificial and the code should remain cohesive.

Do not solve cycles by creating a broad `globals.h`, `common.h`, or
`everything_internal.h`.

## Header rules

A domain header should describe that domain.

Directional migration headers are temporary.

New permanent headers must not be named after consumer/provider movement
such as:

    foo_to_bar.h

An interface should be named for the service or domain it represents.

## Platform abstraction rule

Do not wrap every libc or PS2SDK function.

Create a platform boundary when at least one of these is true:

- hardware behavior belongs to the PS2 domain;
- host testing benefits from the seam;
- initialization/lifecycle invariants need one owner;
- multiple higher-level modules would otherwise duplicate PS2-specific
  knowledge.

This prevents abstraction for abstraction's sake.

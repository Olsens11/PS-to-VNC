# PS-to-VNC Architecture

## Principle

PS-to-VNC is modular software.

Modules are divided by ownership and responsibility rather than by arbitrary
line count.

A module should expose a narrow public interface and retain implementation
details and state privately where practical.

## Planned PS-side families

### core

Application lifecycle and top-level composition.

### net

Network initialization and platform networking concerns.

### rfb

RFB transport, protocol handling, encodings and update processing.

### display

Video modes, geometry, framebuffer ownership, GS presentation, display
transactions and calibration.

### input

Controller acquisition, pointer/keyboard translation and hotkeys.

### ui

Menus, overlays, on-screen keyboard, transition curtains and user-facing state.

### config

Human-readable configuration parsing and persistence.

### management

Pi management protocol and remote display-control transactions.

### diag

Telemetry, diagnostics and low-overhead profiling.

### media

Dedicated non-RFB media-session and PS-oriented video/audio paths.

## Pi-side direction

The Pi remains the more capable protocol/service endpoint.

Future Cast Gateway architecture:

consumer device/application
→ standard LAN casting protocol
→ Pi Cast Gateway
→ normalized media session
→ PS-oriented transport
→ PS-side media renderer

Potential receiver adapters may include AirPlay, Google Cast, Miracast and
DLNA/UPnP where practical.

Consumer casting protocol complexity stays on the Pi rather than entering the
PS-side RFB client.

## Current implementation status

No modular working implementation exists yet.

D17AL-F8J2-B4A is preserved as immutable behavioral reference.

M0 establishes working-source authority before M1 begins decomposition.

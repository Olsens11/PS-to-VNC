# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=RAW_SESSION_CORE_COMPLETE
    HOST_TESTABLE=YES
    PS2_BUILD_INTEGRATED=NO
    HARDWARE_QUALIFIED=NO

This document tracks the first clean PS2-side implementation milestone after the
semantic audit. The rule is deliberately narrow:

> Rebuild the smallest proven product contract in ordinary C; do not reproduce
> the exploratory source shape.

The old B4A implementation is behavioral/evidence authority. Its `.inc` splits,
globals, and migration boundaries are not architecture.

## Fixed first target

Issue #7 starts with one known display target rather than the full display-mode
product:

- physical mode: DTV 480p;
- physical raster: 720x480;
- first logical RFB desktop: 704x462;
- first presentation backend: the proven Standard 480p path;
- no display-mode menu, persistence, calibration transaction, or HIRES work yet.

The 704x462 logical geometry is the existing qualified 480p geometry recorded in
`src/video/mode.c`. Using it here does not adopt that entire normalized module as
the permanent clean display architecture.

## Proven RFB connection contract

The B03 audit and frozen B4A source establish this startup sequence:

1. connect to the Pi on TCP 5900;
2. read the 12-byte server RFB banner;
3. send `RFB 003.008\n`;
4. read the offered security-type list and require SecurityType None (`1`);
5. select None and require successful SecurityResult;
6. send shared ClientInit (`1`);
7. read the 24-byte ServerInit prefix and then the declared desktop name;
8. validate the desktop geometry against owned framebuffer capacity/current
   startup authority;
9. request the GS-compatible 16-bpp/depth-15 true-color pixel format;
10. advertise Raw as the first pixel encoding;
11. request one non-incremental complete framebuffer before incremental service.

The exact qualified GS-compatible SetPixelFormat wire contract is retained:

- bits-per-pixel: 16;
- depth: 15;
- little-endian pixels;
- true-color;
- red/green/blue max: 31/31/31;
- shifts: red 0, green 5, blue 10.

Performance encodings such as Hextile remain later work; startup remains
Raw-first.

## RFB wire foundation

`src/rfb.c` / `src/rfb.h` own protocol-value parsing and wire-message
serialization that can be tested without PS2 hardware:

- strict RFB banner syntax parsing;
- exact RFB 3.8 client banner construction;
- SecurityType None selection;
- SecurityResult success decoding;
- shared ClientInit value;
- ServerInit fixed-prefix parsing;
- qualified GS555 SetPixelFormat construction;
- Raw-only SetEncodings construction;
- full/incremental FramebufferUpdateRequest construction.

`tests/unit/rfb_wire_test.c` byte-compares those messages against the qualified
historical wire contract, including the first 704x462 full-frame request.

## Owned desktop framebuffer

`src/framebuffer.c` / `src/framebuffer.h` own the conventional authoritative
remote image:

- caller-supplied 16-bit pixel storage and capacity;
- current logical geometry;
- explicit validity;
- dirty state and a bounding dirty rectangle.

Geometry changes validate against storage capacity and invalidate the previous
image. Rectangle writes are bounds/stride checked and union into dirty state. A
rectangle write does not make the framebuffer valid.

`framebuffer.valid` therefore means that a complete authoritative desktop has
been established, not merely that some pixels have arrived.

## Scripted RFB startup session

`src/rfb_session.c` / `src/rfb_session.h` express the actual startup transition
without owning platform socket mechanics.

The startup session:

- requires a syntactically valid RFB banner in the 3.8-or-later 3.x protocol
  family and responds with the qualified 3.8 client banner;
- consumes the complete server-rejection reason when security count is zero;
- requires SecurityType None and successful SecurityResult;
- sends shared ClientInit;
- parses ServerInit and requires the fixed target geometry supplied by the
  application;
- consumes the complete desktop-name field while retaining only a bounded
  diagnostic copy;
- sends the qualified GS555 SetPixelFormat;
- advertises Raw only;
- sends the first non-incremental full-frame request;
- transitions to `PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME` only after every prior
  operation succeeds.

`src/rfb_io.h` remains intentionally a two-function direct-call seam:

- `pstvnc_rfb_io_read_exact()`;
- `pstvnc_rfb_io_write_exact()`.

There are no callbacks, transport object hierarchy, or generalized dependency
injection layer. Host tests link scripted implementations; the PS2 build will
provide the real exact socket implementation.

## Shared Raw FramebufferUpdate parser

Startup and ordinary Raw live updates now use one parser rather than separate
framing implementations.

Before each requested FramebufferUpdate, the parser correctly consumes the legal
asynchronous server messages already proven by the B04 audit:

- type 1 `SetColorMapEntries` — consume header and `count * 6` RGB payload;
- type 2 `Bell` — no payload;
- type 3 `ServerCutText` — consume header and declared clipboard payload.

For every Raw FramebufferUpdate it:

- accepts Raw encoding only;
- rejects every rectangle outside the authoritative framebuffer geometry;
- reads the negotiated little-endian 16-bit pixels exactly;
- writes validated pixels into the owned framebuffer;
- unions all changed rectangles into one dirty rectangle for that completed
  update;
- resets dirty state at the start of each new FramebufferUpdate;
- fails closed on unknown server messages, malformed rectangles, unsupported
  encodings, or short payloads.

The startup wrapper applies the stronger historical full-frame contract:

- at least one rectangle is required;
- total Raw pixel payload must equal `width * height * 2`;
- framebuffer validity is published only after that condition passes;
- the session then transitions to `PSTVNC_RFB_SESSION_READY`.

The live wrapper requires an already valid framebuffer and a READY session. It
allows a legal zero-rectangle update, preserves framebuffer validity after a
successful partial update, and leaves dirty state describing only that completed
update. Any framing/I/O/protocol failure invalidates the framebuffer and fails
the session rather than leaving a half-updated image authoritative.

The session owns a bounded 1920-pixel row scratch buffer. The first Issue #7
target uses 704 pixels of it.

Host coverage now includes:

- exact successful startup output;
- security/protocol/geometry/startup I/O failures;
- bounded desktop-name/rejection-reason consumption;
- legal asynchronous message consumption;
- one-rectangle and multi-rectangle complete startup Raw frames;
- explicit little-endian pixel reconstruction;
- incomplete required startup-frame rejection;
- partial live Raw updates that preserve untouched pixels;
- multi-rectangle live dirty-union behavior;
- legal zero-rectangle live updates and per-update dirty reset;
- unsupported encoding, rectangle bounds, unknown message, and truncated-pixel
  failures;
- framebuffer invalidation on failed startup or live update.

## Deliberately not implemented yet

- real TCP connect/close and PS2 exact send/receive loops;
- explicit application-side request/receive cadence for live updates;
- nonblocking live receive and safe-boundary application yield;
- Hextile;
- ExtendedDesktopSize;
- PS2 Ethernet/module initialization;
- 480p GS presentation;
- diagnostics/runtime ELF identity integration;
- input/controller handling;
- Refresh/recovery policy;
- menus/configuration/management/multi-mode behavior.

## Next implementation order

1. implement the PS2 exact socket I/O/connect seam and one-socket-owner startup
   path;
2. wire that transport to the private PS2↔Pi Ethernet platform foundation;
3. wire the authoritative framebuffer to the fixed proven 480p presentation
   path;
4. add deterministic diagnostics/runtime identity before claiming a hardware
   milestone;
5. establish the clean PS2 build as a separately proven build path without
   silently repointing historical B4A tooling;
6. build and qualify the exact ELF on real PS2 hardware using the inherited
   TestKit/evidence rules;
7. only after that baseline, add controller/input-driven nonblocking live
   receive, safe-boundary benign yield, Hextile, and later recovery/display
   features.

No host/unit result is a substitute for the final PT_LOAD and real-hardware gate.

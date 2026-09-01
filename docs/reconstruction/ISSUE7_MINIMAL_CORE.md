# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=INITIAL_RAW_FRAME_FOUNDATION
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

That is the historical wire format which allowed received 15-bit pixels to map
directly into the PS2 presentation representation by adding the local GS alpha
bit. Performance encodings such as Hextile remain later work; startup remains
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

That preserves the historical `require_full` behavior at the correct boundary:

> `framebuffer.valid` means a complete authoritative desktop has been
> established, not merely that some pixels have arrived.

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

`src/rfb_io.h` is intentionally a two-function direct-call seam:

- `pstvnc_rfb_io_read_exact()`;
- `pstvnc_rfb_io_write_exact()`.

There are no callbacks, transport object hierarchy, or generalized dependency
injection layer. Host tests link scripted implementations; the PS2 build will
provide the real exact socket implementation.

## Required initial Raw frame

`pstvnc_rfb_session_receive_initial_frame()` now completes the first coherent
remote-desktop transaction.

Before the required FramebufferUpdate arrives, it correctly consumes the legal
asynchronous server messages already proven by the B04 audit:

- type 1 `SetColorMapEntries` — consume header and `count * 6` RGB payload;
- type 2 `Bell` — no payload;
- type 3 `ServerCutText` — consume header and declared clipboard payload.

For the first FramebufferUpdate it then:

- requires at least one rectangle;
- accepts Raw encoding only;
- rejects every rectangle outside the authoritative framebuffer geometry;
- reads the negotiated little-endian 16-bit pixels exactly;
- writes validated pixels into the owned framebuffer;
- accumulates the exact Raw pixel-byte count across all rectangles;
- requires that count to equal `width * height * 2`, matching the qualified
  historical `require_full` guard;
- marks the framebuffer valid only after that complete-frame condition passes;
- transitions the session to `PSTVNC_RFB_SESSION_READY`.

A short read, unknown server message, unsupported encoding, invalid rectangle,
empty required update, or incomplete full-frame byte total fails the session and
invalidates the framebuffer. Partially written pixels may remain in storage but
have no authority while `valid == 0`.

The session owns a bounded 1920-pixel row scratch buffer. This is sufficient for
the widest currently audited product display modes while keeping receive scratch
explicitly owned rather than hidden in global storage; the first Issue #7 target
uses only 704 pixels of it.

Host coverage now includes:

- exact successful startup output;
- security/protocol/geometry/startup I/O failures;
- bounded desktop-name/rejection-reason consumption;
- legal asynchronous message consumption before the first update;
- one-rectangle and multi-rectangle complete Raw frames;
- explicit little-endian pixel reconstruction;
- incomplete required frame rejection;
- unsupported encoding, rectangle bounds, unknown message, and truncated-pixel
  failures;
- framebuffer invalidation on failed update.

## Deliberately not implemented yet

- real TCP connect/close and PS2 exact send/receive loops;
- ordinary live incremental FramebufferUpdate servicing;
- nonblocking live receive and safe-boundary application yield;
- Hextile;
- ExtendedDesktopSize;
- PS2 Ethernet/module initialization;
- 480p GS presentation;
- diagnostics/runtime ELF identity integration;
- input/controller handling;
- Refresh/recovery policy;
- menus/configuration/management/multi-mode behavior.

Those are added only when the previous layer is explicit and testable.

## Next implementation order

1. add ordinary Raw incremental update servicing while retaining the same exact
   legal-server-message framing and fail-closed unknown-message rule;
2. implement the PS2 exact socket I/O/connect seam and one-socket-owner startup
   path;
3. wire the transport to the PS2 private Ethernet platform seam;
4. wire the authoritative framebuffer to the fixed proven 480p presentation
   path;
5. add deterministic diagnostics/runtime identity before claiming a hardware
   milestone;
6. build and qualify the exact ELF on real PS2 hardware using the inherited
   TestKit/evidence rules;
7. only after that baseline, add controller/input-driven nonblocking live receive,
   safe-boundary benign yield, Hextile, and later recovery/display features.

No host/unit result is a substitute for the final PT_LOAD and real-hardware gate.

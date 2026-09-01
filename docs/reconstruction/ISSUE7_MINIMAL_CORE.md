# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=RFB_WIRE_AND_FRAMEBUFFER_FOUNDATION
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

This code contains no sockets, PS2SDK calls, graphics calls, recovery policy,
controller logic, or global runtime state. It is intentionally a small protocol
leaf, not a framework.

## Owned desktop framebuffer

`src/framebuffer.c` / `src/framebuffer.h` introduce the first clean authoritative
desktop image object.

The framebuffer owns:

- caller-supplied 16-bit pixel storage and its capacity;
- current logical width and height;
- validity;
- dirty state and a bounding dirty rectangle.

It deliberately does **not** allocate memory, perform RFB parsing, convert pixels
to GS presentation format, or decide when a remote session is coherent.

Geometry changes validate against storage capacity and invalidate the existing
image. Rectangle writes are bounds/stride checked and union into the dirty
bounding rectangle. A rectangle write does not make the framebuffer valid.

That last rule preserves the historical `require_full` behavior at the correct
architectural boundary. The qualified implementation rejected a required full
startup/recovery Raw update unless the received Raw byte total equaled
`desktop_width * desktop_height * 2`. In the clean design, the RFB session will
apply that complete-frame contract and call `pstvnc_framebuffer_mark_valid()`
only after it succeeds. Therefore:

> `framebuffer.valid` means a complete authoritative desktop has been
> established, not merely that some pixels have arrived.

`tests/unit/framebuffer_test.c` covers storage/capacity setup, geometry changes,
rectangle writes, dirty-union behavior, bounds/stride rejection, and explicit
validity transitions.

## Deliberately not implemented yet

- TCP/socket ownership and exact send/receive loops;
- server rejection-reason consumption;
- desktop-name bounded consumption;
- Raw rectangle/session-message decoding into the owned framebuffer;
- asynchronous server-message framing;
- PS2 Ethernet/module initialization;
- 480p GS presentation;
- diagnostics/runtime ELF identity integration;
- input/controller handling;
- Hextile;
- ExtendedDesktopSize;
- Refresh/recovery policy;
- menus/configuration/management/multi-mode behavior.

Those are added only when the previous layer is explicit and testable.

## Next implementation order

1. add a small RFB session/transport layer with exact-length framing and one
   socket owner;
2. host-test handshake/session transitions with scripted byte streams where
   practical;
3. decode the first required complete Raw framebuffer into the owned framebuffer
   and mark it valid only after the full-frame contract succeeds;
4. wire the transport to the PS2 private Ethernet platform seam;
5. wire the authoritative framebuffer to the fixed proven 480p presentation
   path;
6. add deterministic diagnostics/runtime identity before claiming a hardware
   milestone;
7. build and qualify the exact ELF on real PS2 hardware using the inherited
   TestKit/evidence rules.

No host/unit result is a substitute for the final PT_LOAD and real-hardware gate.

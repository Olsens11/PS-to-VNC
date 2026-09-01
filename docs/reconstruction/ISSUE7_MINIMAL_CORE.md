# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=RFB_WIRE_FOUNDATION
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

## First clean implementation tranche

`src/rfb.c` / `src/rfb.h` now own only protocol-value parsing and wire-message
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

## Deliberately not implemented yet

- TCP/socket ownership and exact send/receive loops;
- server rejection-reason consumption;
- desktop-name bounded consumption;
- owned desktop framebuffer storage/validity;
- Raw rectangle decoding;
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

1. add the owned conventional desktop framebuffer for the fixed 704x462 target;
2. add a small RFB session/transport layer with exact-length framing and one
   socket owner;
3. host-test handshake/session transitions with scripted byte streams where
   practical;
4. wire the transport to the PS2 private Ethernet platform seam;
5. wire the authoritative framebuffer to the fixed proven 480p presentation
   path;
6. add deterministic diagnostics/runtime identity before claiming a hardware
   milestone;
7. build and qualify the exact ELF on real PS2 hardware using the inherited
   TestKit/evidence rules.

No host/unit result is a substitute for the final PT_LOAD and real-hardware gate.

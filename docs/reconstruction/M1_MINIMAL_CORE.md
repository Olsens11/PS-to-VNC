# Reconstruction Milestone 1 — Minimal PS2/Pi Core

## Status

    GITHUB_ISSUE=7
    MILESTONE=M1
    STATE=IN_PROGRESS
    HARDWARE_AUTHORITY=NONE_YET

M1 is the first clean implementation milestone after the semantic audit.

It proves the smallest PS2/Pi path needed before framebuffer/presentation,
controller/UI, configuration, or recovery behavior are allowed to grow around
it.

## M1A — deterministic startup + private Ethernet + RFB handshake

M1A owns only:

1. deterministic PS2 system/IOP bootstrap;
2. embedded DEV9/NETMAN/SMAP startup;
3. static private PS2 address `192.168.50.2/24` with Pi `192.168.50.1`;
4. explicit carrier wait;
5. one RFB TCP connection to Pi port 5900;
6. exact RFB 3.8 version contract;
7. SecurityType None negotiation;
8. SecurityResult validation;
9. shared ClientInit;
10. bounded ServerInit geometry parsing;
11. complete consumption of the variable-length desktop name;
12. clean socket close and deterministic return to OSDSYS.

M1A deliberately does **not** include:

- framebuffer allocation or pixel decode;
- SetPixelFormat/SetEncodings;
- fixed 480p presentation;
- controller thread/input;
- menus/OSK;
- configuration;
- management TCP 5959;
- recovery policy;
- display modes/calibration;
- Hextile;
- diagnostic UDP/runtime identity qualification.

The temporary M1A ServerInit acceptance envelope is `1280x720`, matching the
preserved companion desktop sufficiently to prove the transport/handshake path.
It is not display authority. M1B replaces this temporary handshake-only envelope
with the fixed-480p framebuffer/presentation contract.

### M1A source ownership

- `src/main.c` — process entry only;
- `src/app.c` — top-level milestone lifecycle/orchestration;
- `src/platform/ps2_system.c` — SIF/IOP bootstrap, visible console, timing,
  OSDSYS convergence;
- `src/platform/ps2_network.c` — PS2 network modules, static IP, carrier state;
- `src/rfb.c` — socket/session ownership and complete RFB handshake;
- `src/rfb_protocol.c` — pure wire parsing/validation used by RFB and host tests.

This is intentionally much smaller than the normalization-era target hierarchy.

### Build contract

M1A uses:

- the same pinned PS2DEV container image as the qualified exploratory build;
- the exact frozen `libps2ip_mtu1458_wscale128.a` dependency, verified by
  SHA-256 before use;
- a separate build definition `mk/reconstruction-m1.mk`;
- generated output only under `build/reconstruction/m1a/`;
- `scripts/test-reconstruction-m1-host.sh` before the PS2 build;
- `scripts/build-reconstruction-m1.sh` as the milestone build entrypoint.

The existing canonical exploratory-generation `scripts/build.sh` remains
untouched until the clean implementation earns replacement authority.

### M1A evidence status

Source existence is **not** hardware evidence.

Before M1A can advance:

1. host protocol tests must pass;
2. the clean PS2 source must compile/link in the pinned build environment;
3. reproducible ELF identity must be recorded;
4. the executable must receive a TestKit-compatible runtime identity path before
   hardware qualification is claimed;
5. the exact DUT must be deployed/read back under the established TestKit
   discipline;
6. machine evidence and physical/operator observation must be recorded
   separately.

M1A may be used as a compile/integration checkpoint without promoting it to
hardware authority.

## M1B — fixed 480p authoritative framebuffer and presentation

M1B begins only after M1A's source/build foundation is mechanically sound.

Planned scope:

- one conventional authoritative EE desktop framebuffer;
- one fixed 480p display profile;
- explicit GS-compatible 16-bit RFB pixel format;
- Raw encoding only;
- one complete non-incremental authoritative framebuffer;
- one simple Standard 480p presentation path;
- first real diagnostic UDP stage/identity path suitable for exact hardware DUT
  proof.

No controller thread, menus, multimode transaction state, recovery policy, or
performance optimization is added merely because B4A already had it.

## M1C — first controlled clean-hardware qualification

Once M1B provides a visible fixed desktop and runtime identity, qualify the exact
clean DUT through the existing immutable successor-to-legacy TestKit bridge.

Qualification must preserve:

- source/commit identity;
- whole ELF and PT_LOAD identity;
- runtime identity;
- exact deployment/readback where required;
- machine evidence;
- separate physical/operator observation;
- documented comparison against B01-B05/B12/B14 lessons relevant to the minimal
  core.

Only after that qualification does the first clean executable become hardware
authority.

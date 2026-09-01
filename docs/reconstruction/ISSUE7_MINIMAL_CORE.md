# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=APPLICATION_COORDINATOR_AND_LINKED_BUILD
    HOST_TESTABLE=YES
    PS2_PLATFORM_COMPILE_GATED=YES
    LINKED_CLEAN_BUILD_DEFINED=YES
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

The 704x462 logical geometry is the existing qualified 480p geometry. The clean
core uses it directly without adopting the historical display hierarchy.

## RFB connection and wire contract

The startup session implements the audited RFB 3.8 sequence:

1. read the exact 12-byte server banner;
2. require RFB 3.x with minor version at least 8;
3. send the exact `RFB 003.008\n` client banner;
4. require SecurityType None (`1`);
5. require successful SecurityResult;
6. send shared ClientInit (`1`);
7. parse ServerInit and consume the entire declared desktop name;
8. require the exact application-selected geometry;
9. send the qualified GS-compatible 16-bpp/depth-15 true-color pixel format;
10. advertise Raw only;
11. send one non-incremental full-frame request;
12. publish READY only after a complete authoritative first framebuffer exists.

The qualified wire pixel format remains:

- bits-per-pixel: 16;
- depth: 15;
- little-endian pixels;
- true-color;
- red/green/blue max: 31/31/31;
- shifts: red 0, green 5, blue 10.

`src/rfb.c` / `src/rfb.h` own pure wire parsing/serialization.
`src/rfb_session.c` / `src/rfb_session.h` own synchronized session behavior.
`src/rfb_io.h` remains the intentionally tiny exact-read/exact-write platform
seam.

The session now also owns full-desktop FramebufferUpdateRequest transmission via
`pstvnc_rfb_session_request_update()`. Application policy chooses full versus
incremental service; it does not construct or write RFB wire messages itself.

## Owned authoritative framebuffer

`src/framebuffer.c` / `src/framebuffer.h` own:

- caller-provided 16-bit pixel storage and capacity;
- logical geometry;
- validity;
- per-update dirty state;
- dirty bounding rectangle.

`framebuffer.valid` means a complete authoritative remote desktop exists. Partial
rectangle writes never publish initial authority.

Startup requires total Raw pixel payload equal to `width * height * 2` before
marking the framebuffer valid. Live updates preserve authority only when the
complete current server message is successfully framed and decoded.

## Shared Raw server-message parser

Startup and live service use the same parser. It understands:

- type 0 `FramebufferUpdate`;
- type 1 `SetColorMapEntries` by exact payload consumption;
- type 2 `Bell`;
- type 3 `ServerCutText` by exact declared-payload consumption.

Unknown server message types fail closed. Raw rectangles are bounds checked,
read at exact lengths, reconstructed from little-endian 16-bit wire pixels, and
written into the authoritative framebuffer. Unsupported encodings, malformed
rectangles, short reads, and framing failures fail the session; receive failures
invalidate framebuffer authority rather than leaving a partially updated image
trusted.

Live zero-rectangle FramebufferUpdates are legal. Dirty state describes only the
most recently completed update.

Hextile remains deliberately deferred.

## PS2 system and private-Ethernet platform seam

`src/platform/ps2_system.c` now owns the minimal qualified lifecycle mechanisms:

- SIF RPC initialization;
- IOP reset and synchronization;
- loadfile/IOP heap initialization;
- LMB patch enable;
- final convergence to `rom0:OSDSYS`;
- stopped-thread fallback if `LoadExecPS2()` unexpectedly returns.

`src/platform/ps2_network.c` owns the first private-link mechanisms:

- embedded DEV9, NETMAN, and SMAP module startup;
- `NetManInit()`;
- static PS2 IP `192.168.50.2/24`;
- Pi/gateway `192.168.50.1`;
- bounded carrier wait through NETMAN link status;
- one TCP connection to `192.168.50.1:5900`;
- exact write loop;
- 32 KiB buffered exact receive loop;
- socket close and buffered-stream reset.

The receive buffer deliberately prevents RFB framing from depending on individual
`recv()` boundaries.

The first clean milestone is blocking and single-threaded by design. Controller
queues, nonblocking refill, safe-boundary benign yield, Refresh interruption,
management polling, and silent-stall policy are not introduced before input/UI
concurrency requires them.

## Fixed Standard 480p presentation

`src/display.c` converts one valid authoritative 704x462 RFB framebuffer into a
separate GS16 presentation buffer. The remote framebuffer remains unchanged.

The B5:G5:R5 color bits already match GS CT16; presentation locally sets bit 15
(A1).

`src/platform/ps2_graphics.c` owns the first conventional hardware path:

- DMAKit initialization;
- gsKit global initialization;
- `GS_MODE_DTV_480P`;
- noninterlaced FRAME output;
- fixed 704x462 logical drawing surface;
- current Standard offsets X=-4, Y=3;
- CT16 texture;
- nearest filtering;
- full texture upload;
- one full-screen sprite;
- queue execution and synchronized flip.

This intentionally favors a boring coherent full presentation over historical
performance shortcuts. HIRES/direct-write paths remain deferred.

## First complete application coordinator

`src/main.c` is process entry only. `src/app.c` now composes the first complete
Raw fixed-480p product path:

1. prepare PS2 IOP/system foundation;
2. initialize the private Ethernet stack;
3. wait for carrier;
4. initialize one owned 704x462 framebuffer;
5. initialize Standard 480p presentation;
6. connect one TCP socket to the Pi VNC endpoint;
7. complete the RFB 3.8 startup session;
8. receive the required complete authoritative Raw framebuffer;
9. convert and present it;
10. repeatedly request one incremental full-desktop update;
11. receive one complete legal server-message/update sequence;
12. present only when that update produced dirty pixels;
13. on fatal failure, close owned transport/presentation resources and return to
    the process entry path, which converges to OSDSYS.

There is still only one application thread and one RFB socket owner. No input,
UI, recovery, management, or display transaction behavior is smuggled into this
milestone.

The two 704x462 16-bit application buffers are statically allocated and
128-byte-aligned: one authoritative remote image and one conventional GS
presentation image.

## Host and PS2 compile gates

Host unit coverage includes:

- wire message byte contracts;
- framebuffer geometry/validity/dirty semantics;
- scripted RFB startup success and failure cases;
- complete startup Raw-frame authority;
- legal asynchronous message consumption;
- partial/multi-rectangle/zero-rectangle live Raw updates;
- malformed/truncated live failure behavior;
- session-owned full and incremental update-request serialization;
- fixed 480p framebuffer-to-GS conversion.

`scripts/check-clean-ps2-compile.sh` compiles every clean Issue #7 translation
unit, including the coordinator, with the R5900 compiler under strict warnings.

The branch workflow runs host tests and the PS2 translation-unit compile gate in
the pinned PS2DEV container.

## Separate linked clean build

The clean executable now has a build path separate from historical
`scripts/build.sh`:

- `mk/issue7-clean.mk` defines only the clean Issue #7 source set;
- `scripts/build-issue7-clean.sh` verifies the qualified frozen PS2IP archive,
  stages it into generated build state, and links in the pinned PS2DEV image;
- generated output lives only under `build/reconstruction/issue7/`;
- `.gitignore` excludes generated build output;
- GitHub Actions builds the linked ELF as a separate gate/artifact.

The qualified PS2IP input remains:

    baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a
    SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

Historical `scripts/build.sh` remains untouched and continues to mean the frozen
B4A/reference build until the clean build earns replacement authority.

A successfully linked ELF is still **not hardware authority**.

## Deliberately not implemented yet

- deterministic runtime ELF identity/stamping integration for the clean build;
- diagnostic UDP/runtime-stage evidence suitable for exact DUT proof;
- real PS2 hardware qualification of the clean executable;
- controller/input handling;
- nonblocking live receive and safe-boundary application yield;
- Hextile;
- ExtendedDesktopSize;
- Refresh/recovery policy;
- menus/OSK/configuration/management;
- display-mode persistence, transactions, calibration, or HIRES;
- performance/hybrid-video work.

## Next implementation order

1. require the coordinator + linked-build CI tranche to pass exactly;
2. integrate deterministic diagnostics/runtime ELF identity without changing the
   clean ownership model;
3. record exact source, qualified dependency, whole-ELF, PT_LOAD, and runtime
   identity for the first DUT candidate;
4. deploy through the inherited qualified TestKit bridge;
5. collect machine evidence and separate physical/operator observation;
6. only after the boring Raw 480p baseline is hardware-qualified, restore
   controller/input-driven nonblocking receive and later features in audited
   order.

No host, compile, or linked-build result substitutes for the PT_LOAD and real
hardware gate.

# Issue #7 — Minimal PS2↔Pi Ethernet and RFB Core

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    PHASE=CLEAN_RECONSTRUCTION
    CURRENT_TRANCHE=PRE_HARDWARE_QUALIFICATION
    HOST_TESTABLE=YES
    PS2_PLATFORM_COMPILE_GATED=YES
    LINKED_CLEAN_BUILD_DEFINED=YES
    LINKED_CLEAN_BUILD_REPRODUCIBLE=YES
    RUNTIME_IDENTITY_INTEGRATED=YES
    SUCCESSOR_IDENTITY_PREPARATION=YES
    HARDWARE_QUALIFICATION_DEFINITION=READY
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

The qualified wire pixel format remains 16 bpp / depth 15, little-endian,
true-color, max 31/31/31, with shifts 0/5/10.

`src/rfb.c` / `src/rfb.h` own pure wire parsing/serialization.
`src/rfb_session.c` / `src/rfb_session.h` own synchronized session behavior.
`src/rfb_io.h` is the deliberately tiny exact-read/exact-write platform seam.

The session owns full-desktop FramebufferUpdateRequest transmission through
`pstvnc_rfb_session_request_update()`. Application policy chooses full versus
incremental service; it does not construct or write RFB wire messages itself.

## Owned authoritative framebuffer

`src/framebuffer.c` / `src/framebuffer.h` own caller-provided 16-bit storage,
logical geometry, validity, per-update dirty state, and the dirty bounding
rectangle.

`framebuffer.valid` means every pixel in the remote desktop is authoritative.
The strict startup path therefore proves **exact pixel coverage**, not merely a
matching Raw byte total. It tracks unique initial-frame coverage, rejects
repeated/overlapping pixels, and requires the unique covered-pixel count to equal
`width * height` before validity is published. This closes the overlap-plus-gap
case where the right total number of bytes could otherwise leave unseen pixels.

The coverage proof is startup-only. Ordinary incremental updates preserve the
already-authoritative framebuffer and may legally update any in-bounds subset.

## Shared Raw server-message parser

Startup and live service use one parser. It understands:

- type 0 `FramebufferUpdate`;
- type 1 `SetColorMapEntries` by exact payload consumption;
- type 2 `Bell`;
- type 3 `ServerCutText` by exact declared-payload consumption.

Unknown server message types fail closed. Raw rectangles are bounds checked,
read at exact lengths, reconstructed from little-endian 16-bit wire pixels, and
written into the authoritative framebuffer. Unsupported encodings, malformed
rectangles, short reads, framing failures, and invalid startup coverage fail the
session; receive failures invalidate framebuffer authority rather than leaving a
partially updated image trusted.

Live zero-rectangle FramebufferUpdates are legal. Dirty state describes only the
most recently completed update. Hextile remains deliberately deferred.

## PS2 system and private-Ethernet platform seam

`src/platform/ps2_system.c` owns the minimal lifecycle mechanisms: SIF RPC
initialization, IOP reset/synchronization, loadfile/IOP heap initialization, LMB
patch enable, final `rom0:OSDSYS` convergence, and stopped-thread fallback if
`LoadExecPS2()` unexpectedly returns.

`src/platform/ps2_network.c` owns embedded DEV9/NETMAN/SMAP startup,
`NetManInit()`, static PS2 IP `192.168.50.2/24`, Pi/gateway `192.168.50.1`, bounded
carrier wait, one TCP connection to `192.168.50.1:5900`, exact writes, a 32 KiB
buffered exact receive path, and socket close/stream reset.

The receive buffer deliberately prevents RFB framing from depending on individual
`recv()` boundaries.

The first clean milestone is blocking and single-threaded by design. Controller
queues, nonblocking refill, safe-boundary benign yield, Refresh interruption,
management polling, and silent-stall policy are not introduced before input/UI
concurrency requires them.

## Fixed Standard 480p presentation

`src/display.c` converts one valid authoritative 704x462 RFB framebuffer into a
separate GS16 presentation buffer. The remote framebuffer remains unchanged.
B5:G5:R5 already matches GS CT16 color bits; presentation locally sets A1.

`src/platform/ps2_graphics.c` owns DMAKit/gsKit initialization,
`GS_MODE_DTV_480P`, noninterlaced FRAME output, the fixed 704x462 logical surface,
Standard offsets X=-4/Y=3, CT16 texture upload, nearest filtering, full-screen
sprite presentation, queue execution, and synchronized flip.

This intentionally favors a boring coherent full presentation over historical
performance shortcuts. HIRES/direct-write paths remain deferred.

## Complete application coordinator

`src/main.c` is process entry only. `src/app.c` composes the first complete Raw
fixed-480p product path:

1. prepare PS2 IOP/system foundation;
2. initialize the private Ethernet stack and wait for carrier;
3. initialize optional diagnostics;
4. initialize one owned 704x462 framebuffer;
5. initialize Standard 480p presentation;
6. connect one TCP socket to the Pi VNC endpoint;
7. complete the RFB 3.8 startup session;
8. receive the required complete authoritative Raw framebuffer;
9. convert and present it;
10. repeatedly request one incremental full-desktop update;
11. receive one complete legal server-message/update sequence;
12. present only when that update produced dirty pixels;
13. on fatal failure, close owned transport/presentation/diagnostics resources
    and converge to OSDSYS.

There is still one application thread and one RFB socket owner. No input, UI,
recovery, management, or display transaction behavior is smuggled into this
milestone.

The two 704x462 16-bit application buffers are statically allocated and
128-byte-aligned: one authoritative remote image and one conventional GS
presentation image.

## Clean diagnostics and deterministic runtime identity

The first clean milestone has a deliberately small `src/diagnostics.c` /
`src/diagnostics.h` transport. It owns only the optional UDP socket/destination
and caller-supplied diagnostic datagram transport to `192.168.50.1:5999`.
Product state and failure policy remain owned by the application/subsystems.

The coordinator currently emits only a few fixed milestone literals such as
`NET_READY`, `GS_READY`, `DESKTOP_READY`, and `FATAL`. Diagnostics initialization
or send failure remains non-fatal to ordinary product startup.

`src/diagnostics/identity.c` / `identity.h` retain the hardware-proven TestKit
runtime identity contract. A fixed 146-byte stampable blob contains:

- magic `PS2VNCIDv1!BLOB!` plus NUL;
- 64-byte test-ID field;
- 65-byte 64-hex digest field plus NUL.

The pristine clean ELF carries `UNSTAMPED` plus an all-zero hexadecimal digest.
The linked build uses `-Wl,--wrap=sendto`; immediately before the first diagnostic
UDP datagram to port 5999, the wrapper attempts the deterministic identity packet:

    PS2VNC_ID version=1 test=<test-id> digest=<64-hex>

The serializer is the repaired bounded byte-copy implementation proven after the
historical 106-byte truncation failure. It does not use printf-family formatting
for the runtime identity message.

`scripts/check-issue7-identity-blob.py` fails closed unless the pristine linked
ELF contains exactly one correctly laid-out unstamped identity blob. The runtime
message self-test remains part of Issue #7 CI.

## Successor-owned identity preparation

The old TestKit stamp/verify behavior has now been recovered from durable sealed
repository authority rather than copied from the retired Pi filesystem.

Cross-fixture forensics over three independently sealed historical stamped ELFs
established exactly one consistent digest rule:

1. write the requested test ID into the fixed 64-byte field;
2. leave the digest field as 64 ASCII zeroes plus NUL;
3. SHA-256 the **entire ELF** in that normalized state;
4. write the resulting 64 lowercase hexadecimal characters plus NUL into the
   digest field.

Whole-ELF-pristine and PT_LOAD-based hypotheses do not reproduce the sealed
historical digests.

`scripts/testkit/elf-identity.py` now owns stamp/verify semantics in the successor
repository, with shell compatibility entry points:

- `scripts/testkit/stamp-elf-identity.sh`;
- `scripts/testkit/verify-elf-identity.sh`.

`successor-identity-compat-self-test.py` reconstructs pristine inputs from three
sealed historical stamped fixtures and requires the successor tool to reproduce
each historical stamped ELF byte-for-byte, including its known whole-ELF and
identity SHA values. It also requires already-stamped and overlong-ID attempts to
fail without mutating their inputs.

`scripts/testkit/prepare-hardware-elf.sh` now uses those successor-owned tools,
verifies the result, repeats stamping from the pristine input to prove
determinism, and records both whole-ELF and PT_LOAD effects. Routine identity
preparation no longer depends on `/home/ps2/ps2vnc/scripts/testkit`.

This does **not** abolish the separate frozen-legacy hardware/apparatus bridge.
Deployment/observer mechanics that genuinely rely on historical apparatus remain
a distinct qualification boundary until replaced from equally strong authority.

## Host, compile, link, identity, and reproducibility gates

Host coverage includes wire contracts, framebuffer authority/dirty semantics,
the overlap-plus-gap exact-coverage regression, scripted RFB startup/live
behavior, session-owned full/incremental update requests, display conversion,
deterministic runtime identity serialization, historical identity-digest
forensics, byte-exact successor stamp compatibility, successor preparation
self-tests, and disposable regression tests for clean-source dictionary tooling.

`scripts/check-clean-ps2-compile.sh` compiles every clean Issue #7 translation
unit, including diagnostics/identity, with the R5900 compiler under strict
warnings.

The linked clean build remains separate from historical `scripts/build.sh`:

- `mk/issue7-clean.mk` owns only the clean Issue #7 source set;
- `scripts/build-issue7-clean.sh` verifies the qualified frozen PS2IP archive and
  links in the pinned PS2DEV image;
- the link includes the TestKit-compatible `sendto()` identity wrapper;
- generated output lives only under `build/reconstruction/issue7/`;
- the build checks the pristine identity blob and records a PT_LOAD fingerprint;
- `scripts/check-issue7-linked-reproducibility.sh` performs two independent clean
  links and requires byte-identical whole ELFs plus identical PT_LOAD SHA/length;
- GitHub Actions preserves the result only after that gate, still explicitly as
  an **unqualified** ELF artifact.

### Live checkpoint authority

Exact branch-head and CI identities are intentionally **not** embedded here as a
"current" checkpoint. A tracked document that names its own current commit
becomes stale as soon as that document is committed.

For the live pre-hardware checkpoint, use the head of draft PR #15 together with
the GitHub Actions run for that exact head. The linked-build job is the authority
for the pristine whole-ELF SHA256 and PT_LOAD SHA256/byte count. Preserve older
exact head/CI/build identities in Git/CI history as historical checkpoints rather
than rewriting them into current truth.

The qualified PS2IP input remains:

    baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a
    SHA256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

Historical `scripts/build.sh` remains untouched and continues to mean the frozen
B4A/reference build until the clean build earns replacement authority.

A successful host test, compile, link, reproducibility proof, identity preparation,
or PT_LOAD fingerprint is still **not hardware authority**.

## Hardware qualification definition

`docs/reconstruction/ISSUE7_HARDWARE_QUALIFICATION.md` defines the first clean
hardware experiment before deployment. It separates:

- exact DUT/source/dependency/whole-ELF/PT_LOAD/runtime-identity authority;
- clean Pi endpoint prerequisites;
- machine evidence for startup and one deterministic incremental Raw update;
- physical/operator observations;
- explicit failure classifications.

The definition is ready; the experiment has not been executed.

## Deliberately not implemented or qualified yet

- a stamped/deployed first clean hardware DUT result;
- real PS2 runtime identity observation/qualification of the clean executable;
- real PS2 physical/operator qualification;
- controller/input handling;
- nonblocking live receive and safe-boundary application yield;
- Hextile;
- ExtendedDesktopSize;
- Refresh/recovery policy;
- menus/OSK/configuration/management;
- display-mode persistence, transactions, calibration, or HIRES;
- performance/hybrid-video work.

## Next implementation order

1. keep PR #15 draft and preserve exact green checkpoints in GitHub CI/history;
2. finish/validate the clean Pi TigerVNC endpoint required by the qualification
   definition;
3. prepare one named first-clean-DUT ELF with the successor-owned identity tools;
4. use only proven deployment/apparatus mechanics for the controlled hardware run;
5. require runtime identity plus the defined machine evidence;
6. collect separate physical/operator observation;
7. only after the boring Raw 480p baseline is hardware-qualified, restore
   controller/input-driven nonblocking receive and later features in audited
   order.

No host, compile, linked-build, reproducibility, identity, or PT_LOAD result
substitutes for the real-hardware gate.

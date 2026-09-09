# CP2K — visible RFB-only hardware candidate

Status: **HARDWARE QUALIFIED ON REAL PS2 — see `RFB_MUX_CP2K_HARDWARE_RESULT.md`**

CP2J already hardware-qualified the one-socket RFB transport, channel-1 credit
policy, unchanged through-Issue-39 parser, authoritative 704x462 CPU framebuffer,
incremental service, and clean REQUEST -> BOUNDARY -> COMMIT -> COMPLETE
quiescence. CP2K deliberately changes only the path from complete authoritative
RFB framebuffer states to the already-existing through-Issue-39 display/GS
presenter.

## Exact candidate identity

    source/build head=7555d2e5a3a032c6525d65ed0696cf75a135e7ba
    workflow=H1 RFB mux preparation checks
    workflow run=34313295461
    workflow result=PASS
    ELF=PS2VNC-H1-CP2K-VisibleRFB.ELF
    ELF_SHA256=2563cbe4e8909f6c35bff6494a17cd4b0377bbee4171513132315a183a07ffde
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=2afbb34787a7133118d46c7151fd2e6fdfc762c1e3b36331c0823843ab682cab
    PT_LOAD_BYTES=487188
    artifact=h1-cp2k-visible-rfb-unqualified-elf
    artifact_id=10089153724
    artifact_zip_sha256=590c6dabe7f5b100d6fe712f5850afe2953fba6be0c7915b5c622e0cb6e944c6

The CI artifact retains its original **unqualified** name because that was its
state when produced. The exact artifact/ELF was subsequently hardware-qualified
by CP2K HW2; the authoritative physical-run result is recorded in
`RFB_MUX_CP2K_HARDWARE_RESULT.md`.

## Runtime contract

CP2K reserves CONFIG `rfb_mode=2` as visible RFB-only. Mode 1 remains the exact
CP2J headless authority. Both modes are accepted by the cumulative-only RFB
activation gate only with:

- AUDIO OFF;
- MPEG OFF;
- the same explicit RFB queue/credit policy validated by CP2J.

For CP2K the mode-2 token is mechanically scoped onto the already-qualified
CP2J transport/session implementation through tiny wrapper translation units.
There is no queue, credit, mux framing, socket, parser, or quiesce algorithm
change.

The specialized CP2K coordinator initializes exactly one graphics owner: the
through-Issue-39 `pstvnc_ps2_graphics_*` path. It converts complete authoritative
RFB framebuffer states through `pstvnc_display_prepare_gs16()` and presents them
through `pstvnc_ps2_graphics_present()`.

Presentation occurs only:

1. after the complete initial framebuffer has passed parser coverage checks; and
2. after a complete incremental RFB update when the authoritative framebuffer is
   dirty.

The RFB session coordinator contains no gsKit-specific ownership; presentation
is behind an experiment-owned callback seam. A presenter failure fails the
session rather than allowing transport success to be reported as visible
success.

## Explicitly inactive

CP2K does **not** start or qualify:

- H1 MPEG video chassis/presenter;
- audio;
- controller input;
- pointer publication;
- keyboard publication;
- OSK;
- local UI;
- automatic MPEG-region detection;
- hybrid/shared compositor behavior;
- Issue #40.

This isolation is intentional. CP2K answers only one question: can the
hardware-qualified mux-fed RFB framebuffer once again become visibly correct on
the PS2 using the existing through-Issue-39 display path?

## CI evidence

Run 34313295461 completed both host-preflight and pinned PS2 build jobs with
`success`. Host checks passed for CONFIG v4 profiles, source/object mux seam,
headless RFB coordinator, CP2K visible presentation contract, mode-2 cumulative
activation gate, logical channel mechanics, credit policy, Pi byte bridge,
reader integration, cumulative bridge runner, and operator tooling.

The pinned build printed:

    CP2K_VISIBLE_RFB_CONTRACT=PASS
    CP2K_RFB_MODE=2_VISIBLE
    CP2K_PRESENTATION=THROUGH_ISSUE39_DISPLAY_AND_PS2_GRAPHICS
    CP2K_RFB_TRANSPORT=CP2J_MECHANICS_MECHANICALLY_SCOPED_TO_MODE2
    CP2K_AUDIO=OFF
    CP2K_MPEG=OFF
    CP2K_INPUT=OFF
    CP2K_OSK=OFF

## Hardware qualification gate

The gate below was the pre-run requirement and is now **satisfied by CP2K HW2**.
The authoritative hardware evidence and exact Pi runtime identity are recorded in
`RFB_MUX_CP2K_HARDWARE_RESULT.md`.

The first physical run had to use the exact candidate identity above, the
existing Pi upstream VNC provider, one PSTV socket on port 5902, and the CP2J RFB
credit values. The operator generated visible desktop activity from the
independent Windows VNC observer during the finite session.

CP2K could be called hardware-qualified only if all of the following were true:

- exact deployment/readback identity passes;
- RFB handshake and initial frame complete;
- incremental updates continue;
- transport/credit/quiesce result passes;
- at least one initial presentation occurs;
- incremental presentations occur while desktop activity is generated; and
- the operator confirms that the Pi desktop is visibly correct and updating on
  the PS2 television.

CP2K HW2 satisfied this gate: the exact deployed ELF passed readback, the
60-second session completed with 244 incremental updates and clean quiescence,
and the operator confirmed the visible Pi desktop updated on the PS2 television
while a terminal window was dragged from the independent Windows VNC client.

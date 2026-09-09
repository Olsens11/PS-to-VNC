# CP2N visible RFB + real Issue-39 interaction hardware candidate

## Status

**HARDWARE QUALIFIED FOR THE NARROW CP2N SCOPE.**

CP2N is the corrected successor to the CP2L visible-RFB-plus-mouse checkpoint.
It retains CP2L's already hardware-qualified one-physical-PSTV-connection RFB
path, visible desktop, and PS2 mouse path, then composes the existing clean
through-Issue-39 interaction modules rather than adding experiment-specific
controller or keyboard semantics.

The newly exercised clean-module composition is:

- the existing input runtime and mouse interpreter;
- the existing local-controller router;
- the existing local-UI foreground/quarantine state;
- the existing OSK model and OSK presentation path;
- the existing keyboard tap-sequence builder;
- the existing RFB session pointer/key event API;
- the existing display conversion and PS2 graphics presenter.

CP2N deliberately does **not** use the CP2M transient L1+D-pad keyboard-chord
experiment. CP2M remains historical experiment evidence and is not the
architectural parent of this checkpoint.

AUDIO and MPEG remain disabled. Hybrid media composition remains deferred.
Issue #40 has not been started.

The real-PS2 result is recorded in:

    experiments/media-harness-h1/RFB_MUX_CP2N_HARDWARE_RESULT.md

## Scope guard

- Issue frontier: through Issue #39.
- Issue #40 started: **NO**.
- Physical Pi-to-PS2 PSTV TCP streams: **1**.
- RFB logical channel: **1**.
- RFB mode: **2 (visible)**.
- Presentation: through-Issue-39 display/PS2 graphics path plus existing local
  overlay presentation.
- PS2 input runtime: **existing through-Issue-39 implementation**.
- Desktop mouse semantics: **existing through-Issue-39 implementation**.
- Local controller routing: **existing through-Issue-39 implementation**.
- Keyboard serialization: **existing through-Issue-39 keyboard builder and RFB
  key-event API**.
- OSK/local UI: **existing through-Issue-39 modules**.
- Audio: **OFF**.
- MPEG: **OFF**.
- Custom CP2M L1+D-pad keyboard behavior: **ABSENT**.

The existing Issue-39 local-controller semantics exercised by this checkpoint
are intentionally unchanged. On the desktop, Select opens the OSK. With the OSK
in the foreground, D-pad moves selection, Triangle toggles Shift, Cross activates
the selected key, Square emits Backspace, Start emits Enter, R1 emits Tab, and
Circle or Select closes the OSK. The existing transition quarantine must prevent
buttons held across the foreground boundary from becoming fresh actions.

## Exact source/build authority

Branch:

    experiment/h1-rfb-mux-prep

CP2N implementation commit:

    f55dc0c48a00eaa8d5049bb0563b83aa02f9ca66
    h1: add CP2N real interaction checkpoint

Exact CI/build head:

    7e047a9cb9dfef9283f66a80ccd865664f075c11
    ci: build CP2N real interaction checkpoint

The CI head's tree is:

    1596e0645653e2c695b980aac41752001a666c9a

Pinned CI authority:

    workflow=H1 RFB mux preparation checks
    workflow_run=34380104857
    workflow_run_number=163
    workflow_result=PASS
    head_sha=7e047a9cb9dfef9283f66a80ccd865664f075c11
    host_preflight=PASS
    ps2_build=PASS
    cp2n_source_contract=PASS
    cp2n_object_contract=PASS
    rfb_mux_seam=PASS
    pinned_toolchain=ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11

Exact CI-built CP2N ELF:

    ELF=PS2VNC-H1-CP2N-VisibleRFBInteraction.ELF
    ELF_SHA256=f55b2620903ea0ba2a64c53c4196b1e1959a6bb13be1c8a3ae6bf52a08df1000
    ELF_BYTES=3041300
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=544117b8bdf4c0cacefb7d6b2e8fa01eba90809a369cfd8bfdfc0abc36722914
    PT_LOAD_BYTES=491028

CI artifact:

    artifact_name=h1-cp2n-visible-rfb-interaction-unqualified-elf
    artifact_id=10115522072
    artifact_zip_sha256=6f660db5c70ceca4eeabe5c803420efff937e819a6d4e49a95c5d5ca49445f1a
    artifact_zip_bytes=1068424

The downloaded CI artifact was independently unpacked after the run. It
contained exactly one 3,041,300-byte ELF whose SHA-256 matched the CI console
identity. Independent inspection found one PT_LOAD segment at file offset
`0x000080` with file size `0x77e14` (491,028 bytes); hashing exactly those bytes
matched the CI PT_LOAD identity above.

## Architectural/build gates passed

The green workflow proves the CP2N source and linked-object contract, including:

- the existing one-socket PSTV/RFB mux seam remains in force;
- the coordinator references the real input runtime, local-controller, local-UI,
  OSK, keyboard, display, graphics, and RFB-session interfaces;
- the coordinator contains no CP2M keyboard-chord dependency and no L1-specific
  checkpoint behavior;
- the coordinator does not open, connect, or receive from a second socket;
- RFB pointer and keyboard messages remain serialized through the RFB-owning
  application/main execution path at the existing safe service boundary;
- entering local OSK foreground suspends mouse interpretation and neutralizes any
  published remote mouse-button state before local ownership proceeds;
- leaving OSK foreground retains the existing release/quarantine rule before
  desktop mouse interpretation resumes;
- local overlay presentation is composed through the existing local-UI
  presentation and PS2 graphics interfaces;
- AUDIO and MPEG remain inactive in the CP2N coordinator;
- cooperative input shutdown must prove worker dormancy before storage can be
  reused; otherwise CP2N halts instead of guessing.

These build/source gates were paired with the exact real-hardware qualification
record below; the CI facts alone are not treated as hardware authority.

## Hardware qualification result

The exact ELF above completed the CP2N real-PS2 hardware gate on 2026-09-09.

    test_id=H1-RFB-CP2N-HW1-20260909T221954Z
    session_id=3053165241
    evidence=/home/ps2/ps2vnc-evidence/h1-rfb-cp2n-hw1-20260909T221954Z
    deployment_evidence=/home/ps2/ps2vnc-evidence/deployments/H1-RFB-CP2N-HW1-20260909T221954Z.json
    deployment_readback=PASS
    quiesce=PASS_REQUEST_BOUNDARY_COMMIT_COMPLETE
    integrity_pass=1
    transport_error=0
    diagnostic_word=0xA200E184

The unique archival and rolling deployment targets both read back with the exact
candidate SHA-256 and byte count. The Pi runner reached CONFIG ACK, complete RFB
finite quiesce, normal post-result socket closure, H1 session validation, and
H1 Pi mux PASS.

The operator reported a full physical PASS: the live Pi desktop remained
visible; left-stick/D-pad pointer movement, Cross left click/drag, and Circle
right click worked; Select opened the real OSK without a phantom desktop action;
OSK foreground suspended desktop mouse interpretation; D-pad OSK navigation and
Cross character activation worked; Triangle Shift, Square Backspace, Start Enter,
and R1 Tab worked; Circle/Select closed the OSK; and mouse control resumed after
the release/quarantine boundary without a carried-through click/action.

The cursor felt somewhat sluggish/laggy compared with earlier behavior. This is
recorded as a deferred performance/optimization observation, not a functional
qualification failure.

    CP2N_HARDWARE_QUALIFIED=YES
    CURRENT_HARDWARE_AUTHORITY=CP2N_VISIBLE_RFB_REAL_ISSUE39_INTERACTION

RFB+AUDIO, RFB+MPEG, simultaneous combined workload behavior, hybrid
composition, performance optimization, production transport extraction, and
Issue #40 remain outside this qualification. Subsequent performance/stability
experiments should use the resident H1 ELF and canonical `h1_tool.py`
run/profile/sweep apparatus rather than bespoke one-off runners.

# CP2N visible RFB + real Issue-39 interaction hardware result

## Result

**PASS — HARDWARE QUALIFIED FOR THE NARROW CP2N SCOPE.**

CP2N extends the CP2L visible-RFB-plus-mouse authority by composing the existing
through-Issue-39 input runtime, local-controller router, local UI, OSK, keyboard
builder, RFB key/pointer serialization, display conversion, and PS2 graphics
presentation over the already-qualified one-physical-PSTV-connection RFB path.

The operator reported a full physical PASS: every required interaction worked as
designed. The cursor felt somewhat sluggish/laggy compared with earlier direct
behavior; that is recorded as a performance observation for later H1 profile and
sweep optimization, not as a functional qualification failure.

## Exact PS2 candidate authority

Implementation commit:

    f55dc0c48a00eaa8d5049bb0563b83aa02f9ca66

Exact CI/build head and hardware-run source head:

    7e047a9cb9dfef9283f66a80ccd865664f075c11

Exact hardware-run ELF:

    ELF=PS2VNC-H1-CP2N-VisibleRFBInteraction.ELF
    ELF_SHA256=f55b2620903ea0ba2a64c53c4196b1e1959a6bb13be1c8a3ae6bf52a08df1000
    ELF_BYTES=3041300
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=544117b8bdf4c0cacefb7d6b2e8fa01eba90809a369cfd8bfdfc0abc36722914
    PT_LOAD_BYTES=491028

Pinned build authority:

    workflow=H1 RFB mux preparation checks
    workflow_run=34380104857
    workflow_result=PASS
    artifact_id=10115522072
    artifact_name=h1-cp2n-visible-rfb-interaction-unqualified-elf
    artifact_zip_sha256=6f660db5c70ceca4eeabe5c803420efff937e819a6d4e49a95c5d5ca49445f1a
    artifact_zip_bytes=1068424
    pinned_toolchain=ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11

Immediately before deployment, the pinned CI toolchain was used to reproduce the
candidate locally. Whole-ELF and PT_LOAD identities matched the recorded CI
candidate exactly. No restamping or PT_LOAD-changing transformation was applied.

## Hardware session

    checkpoint=CP2N_HW1
    test_id=H1-RFB-CP2N-HW1-20260909T221954Z
    session_id=3053165241
    duration_seconds=90
    evidence=/home/ps2/ps2vnc-evidence/h1-rfb-cp2n-hw1-20260909T221954Z
    deployment_evidence=/home/ps2/ps2vnc-evidence/deployments/H1-RFB-CP2N-HW1-20260909T221954Z.json
    deployment_evidence_sha256=58934e9c87a5e560bdfbb3db9f3720e533f88333bc583d601a983272f21420db
    upstream_vnc=127.0.0.1:5903
    pstv_listener=0.0.0.0:5902
    ps2_peer=192.168.50.2:52432
    rfb_mode=2
    rfb_queue_capacity=32768
    rfb_initial_credit=32768
    rfb_credit_batch=8192
    rfb_credit_flush_on_empty=1
    rfb_credit_return_enabled=1
    receiver_thread_priority=63
    audio=OFF
    mpeg=OFF

The generic deployer wrote both the unique archival target and rolling launch
target, then read both back. Both readbacks matched the exact candidate SHA-256
and 3,041,300-byte size.

Observed host/protocol/result markers:

    H1_CONFIG_ACK=PASS
    H1_RFB_QUIESCE_REQUEST=YES
    H1_RFB_QUIESCE=COMPLETE boundary=1 bridge_stopped=1 commit=1 complete=1
    integrity_pass=1
    transport_error=0
    H1_POST_RESULT_SOCKET_CLOSE=PASS classification=normal_session_boundary
    H1_SESSION_VALIDATION=PASS
    H1_PI_MUX_SESSION=PASS

The result diagnostic word was:

    diagnostic_word_decimal=2717966724
    diagnostic_word_hex=0xA200E184

Under the existing CP2J/CP2K/CP2L RFB diagnostic encoding, the low 13 bits
correspond to 388 completed incremental RFB updates. The session reached normal
finite quiesce rather than being terminated by an arbitrary byte-stream cut.

## Resident multi-session behavior

CP2N is intentionally a resident multi-session H1 ELF. After a clean session
teardown, the PS2 returns to its session-wait loop and retries the same single
PSTV transport until the next H1 listener is available. Subsequent
`h1_tool.py run` or sweep cases therefore reconnect to the already-running ELF;
ordinary H1 testing does not require relaunching the ELF between sessions.

This resident behavior is part of the H1 testing apparatus and is distinct from
a silent-freeze recovery/watchdog. It does not mask a stalled in-session DUT.

## Operator-visible qualification

The real-PS2 observation gate passed in full. The operator confirmed that:

- the live Pi desktop remained visible and updated on the PS2 television;
- left-stick and D-pad pointer movement worked;
- Cross performed left click and click-and-drag;
- Circle performed right click;
- Select opened the existing Issue-39 OSK without an unintended desktop action;
- desktop mouse interpretation was suspended while the OSK owned foreground;
- D-pad changed OSK selection;
- Cross activated selected OSK keys and remote keyboard input was received;
- Triangle Shift behavior worked;
- Square Backspace worked;
- Start Enter worked;
- R1 Tab worked;
- Circle/Select closed the OSK;
- after the required release/quarantine transition, desktop mouse control resumed
  without a carried-through click/action; and
- the session remained functional through the finite hardware run and completed
  clean quiescence.

The operator described the cursor as somewhat sluggish/laggy compared with past
behavior. This does not invalidate CP2N. The current interaction service remains
correct and stable enough to advance the hardware-proof sequence; H1's explicit
profile/CONFIG knobs and reproducible sweeps are the intended later mechanism
for performance and stability optimization.

## Qualified boundary

CP2N now hardware-qualifies the following composition on a real PS2:

    local VNC :5903
        -> Pi raw RFB bridge
        -> logical PSTV channel 1
        -> one physical Pi<->PS2 PSTV TCP connection
        -> PS2 credit-controlled RFB queue
        -> unchanged through-Issue-39 RFB parser
        -> authoritative CPU framebuffer
        -> through-Issue-39 display / PS2 graphics presentation
        -> through-Issue-39 input runtime and mouse semantics
        -> through-Issue-39 local-controller routing
        -> through-Issue-39 local UI / OSK foreground ownership
        -> through-Issue-39 keyboard tap builder and RFB key serialization
        -> live remote pointer and keyboard interaction
        -> REQUEST/BOUNDARY/COMMIT/COMPLETE finite RFB quiescence

The current hardware authority is therefore:

    CP2N_HARDWARE_QUALIFIED=YES
    CURRENT_HARDWARE_AUTHORITY=CP2N_VISIBLE_RFB_REAL_ISSUE39_INTERACTION

## Explicit non-claims

This result does **not** yet qualify:

- RFB + AUDIO;
- RFB + MPEG;
- simultaneous RFB + AUDIO + MPEG workload behavior;
- hybrid RFB/MPEG media-object composition;
- an optimized RFB queue/credit/input-latency profile;
- production shared-transport extraction or architectural consolidation; or
- Issue #40 work.

Those remain later controlled H1 experiments or later deliberate integration
work. CP2N establishes the complete existing Issue-39 interaction composition on
the muxed RFB path; it does not claim the current starting profile is optimal.

# CP2L visible RFB + PS2 mouse-input hardware result

## Result

**PASS — HARDWARE QUALIFIED FOR THE NARROW CP2L SCOPE.**

CP2L extends the CP2K visible-RFB-only authority by activating the existing
through-Issue-39 PS2 controller/mouse semantic path while keeping AUDIO, MPEG,
keyboard, OSK, and local UI inactive.

The operator observed the Pi desktop live on the PS2 television and confirmed
that PS2-controller pointer movement and ordinary mouse-button behavior worked as
expected. Left-stick and D-pad pointer movement, Cross left-click, Cross
click-and-drag, and Circle right-click all worked. The operator described the
interaction as somewhat sluggish, which is recorded as a performance observation
rather than a functional qualification failure; optimization is deferred.

## Exact PS2 candidate authority

Source/build head:

    75ed9aae1764c256a8019b5a4a4ff3b5945721cc

Exact hardware-run ELF:

    ELF=PS2VNC-H1-CP2L-VisibleRFBInput.ELF
    ELF_SHA256=651da23aff616784cd38715a46fe639fa8adc985d290c2c80275b4c5b396b73f
    ELF_BYTES=3028556
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=7579a7d2a21fa8f80ba8290c69bcb7c094e2c814c313f32327cfc8188db797db
    PT_LOAD_BYTES=488980

Pinned build authority:

    workflow_run=34324553964
    workflow_result=PASS
    artifact_id=10093217940
    artifact_name=h1-cp2l-visible-rfb-input-unqualified-elf
    artifact_zip_sha256=d518bdd72633da039a5d5eb5d77984153ab55ef28f27fb15cbe143d67583ce8a

The hardware runner repeated the exact source-head and whole-ELF SHA-256 check
immediately before the session.

## Hardware session

    checkpoint=CP2L_HW1
    session_id=2627828139
    duration_seconds=60
    evidence=/home/ps2/ps2vnc-evidence/h1-rfb-cp2l-hw1-20260909T110649Z
    upstream_vnc=127.0.0.1:5903
    pstv_listener=0.0.0.0:5902
    ps2_peer=192.168.50.2:52432
    rfb_mode=2
    rfb_queue_capacity=32768
    rfb_initial_credit=32768
    rfb_credit_batch=8192
    rfb_credit_flush_on_empty=1
    rfb_credit_return_enabled=1
    audio=OFF
    mpeg=OFF
    keyboard=OFF
    osk=OFF
    local_ui=OFF

Observed protocol/result markers:

    H1_CONFIG_ACK=PASS
    H1_RFB_QUIESCE_REQUEST=YES
    H1_RFB_QUIESCE=COMPLETE boundary=1 bridge_stopped=1 commit=1 complete=1
    integrity_pass=1
    transport_error=0
    H1_POST_RESULT_SOCKET_CLOSE=PASS classification=normal_session_boundary
    H1_SESSION_VALIDATION=PASS
    H1_PI_MUX_SESSION=PASS
    RUN_RC=0

The result diagnostic word was:

    diagnostic_word_decimal=2717966649
    diagnostic_word_hex=0xA200E139

Under the already-established CP2J/CP2K RFB diagnostic encoding this records
READY state, no RFB session error, all clean-quiesce phases, and 313 completed
incremental RFB updates in the low 13 bits.

## Operator-visible qualification

The real-PS2 observation gate passed:

- the Pi desktop was visible on the PS2 television;
- left-stick pointer movement worked;
- D-pad pointer movement worked;
- Cross produced left-click behavior;
- Cross click-and-drag worked;
- Circle produced right-click behavior;
- the visible RFB session remained functional through the 60-second run and
  completed clean finite quiescence;
- interaction was noticeably sluggish but functionally correct.

The sluggishness does not invalidate this checkpoint. CP2L deliberately services
semantic input from the RFB-owning application thread at safe complete-message
boundaries, so latency/performance remains a later optimization topic rather than
a reason to widen this hardware gate.

## Qualified boundary

CP2L now hardware-qualifies the following composition on a real PS2:

    local VNC :5903
        -> Pi raw RFB bridge
        -> logical PSTV channel 1
        -> one physical Pi<->PS2 PSTV TCP connection
        -> PS2 credit-controlled RFB queue
        -> unchanged through-Issue-39 RFB parser
        -> 704x462 authoritative CPU framebuffer
        -> through-Issue-39 display / PS2 graphics presentation
        -> through-Issue-39 controller mouse semantic producer
        -> RFB-owner-thread pointer serialization
        -> live remote pointer movement and button interaction
        -> REQUEST/BOUNDARY/COMMIT/COMPLETE finite quiescence

## Explicit non-claims

This result does **not** qualify:

- keyboard serialization;
- OSK;
- local UI;
- RFB + AUDIO;
- RFB + MPEG;
- hybrid RFB/MPEG composition;
- input-latency optimization;
- Issue #40 work.

Issue #40 remains untouched.

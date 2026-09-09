# CP2K visible RFB-over-mux hardware result

## Status

**HARDWARE QUALIFIED for the narrow CP2K visible RFB-only milestone.**

This result extends the already-qualified CP2J RFB transport/parser path through
the existing through-Issue-39 display/PS2 graphics presenter. On real PS2
hardware the Pi desktop was visible on the television and continued to update
while the operator dragged a terminal window using the independent Windows VNC
client.

It does **not** qualify PS2 controller/mouse/keyboard input, OSK/local UI,
simultaneous RFB+MPEG or RFB+audio operation, hybrid/shared compositor behavior,
or any Issue #40 work.

## Scope guard

- Issue frontier: **through Issue #39**.
- Issue #40 started: **NO**.
- Physical Pi-to-PS2 PSTV TCP streams: **1**.
- RFB logical channel: **1**.
- RFB mode: **2 (visible RFB-only)**.
- Audio: **OFF**.
- MPEG/H1 video: **OFF**.
- PS2 input: **OFF**.
- OSK/local UI: **OFF**.
- Presentation path: through-Issue-39 display conversion and PS2 graphics path.

The separate Windows VNC connection was only an external observer/control path
to the Pi desktop. It was not a PS2-facing PSTV stream and did not violate the
one-physical-PSTV-connection invariant.

## Exact PS2 source/build authority

Branch:

    experiment/h1-rfb-mux-prep

Exact PS2 source/build head used for the deployed CP2K ELF:

    7555d2e5a3a032c6525d65ed0696cf75a135e7ba

Pinned CI authority:

    workflow=H1 RFB mux preparation checks
    workflow_run=34313295461
    workflow_result=PASS
    artifact_id=10089153724
    artifact=h1-cp2k-visible-rfb-unqualified-elf
    artifact_zip_sha256=590c6dabe7f5b100d6fe712f5850afe2953fba6be0c7915b5c622e0cb6e944c6

Exact hardware-tested PS2 ELF:

    ELF=PS2VNC-H1-CP2K-VisibleRFB.ELF
    ELF_SHA256=2563cbe4e8909f6c35bff6494a17cd4b0377bbee4171513132315a183a07ffde
    ELF_BYTES=3013044
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=2afbb34787a7133118d46c7151fd2e6fdfc762c1e3b36331c0823843ab682cab
    PT_LOAD_BYTES=487188

Deployment/readback evidence:

    /home/ps2/ps2vnc-evidence/deployments/H1-RFB-CP2K-HW1-20260909T060553Z.json

The deployment record reports:

    deployment_result=PASS
    ftp_readback=PASS
    repository_head=7555d2e5a3a032c6525d65ed0696cf75a135e7ba
    rolling_readback_sha256=2563cbe4e8909f6c35bff6494a17cd4b0377bbee4171513132315a183a07ffde
    unique_readback_sha256=2563cbe4e8909f6c35bff6494a17cd4b0377bbee4171513132315a183a07ffde
    rolling_readback_bytes=3013044
    unique_readback_bytes=3013044

Its `hardware_run=false` field describes only the deploy/readback operation.
The subsequent CP2K HW2 session below is the hardware qualification evidence;
there is no contradiction between those two records.

## Exact Pi host/runtime authority

The successful hardware session ran the Pi-side checkout at:

    f33ea72e3b756ee0bce2853ebc9caabfac642655

That checkout includes the Pi-side steady-state upstream RFB read fix:

    9622c1bad15e2cf7b6a1844ebc6ec47f5834899d  fix: keep upstream RFB reads blocking after connect

and its regression proof:

    f33ea72e3b756ee0bce2853ebc9caabfac642655  test: prove idle upstream RFB reads do not time out

These Pi-side commits postdate the PS2 ELF source/build head above. The tested
PS2 binary identity therefore remains the exact `7555d2e...` ELF and PT_LOAD;
the successful run used the later Pi host/runtime checkout without rebuilding or
changing that deployed PS2 ELF.

## Hardware run identity

Run evidence directory:

    /home/ps2/ps2vnc-evidence/h1-rfb-cp2k-hw2-20260909T063246Z

Session:

    profile=H1_RFB_ONLY
    profile_id=2
    rfb_mode=2
    session_id=394431234
    duration_seconds=60
    PS2_peer=192.168.50.2:58304
    PSTV_listener=0.0.0.0:5902
    upstream_RFB=127.0.0.1:5903
    desktop=704x462
    max_data_payload=8192
    receiver_thread_priority=63
    receiver_thread_stack_size=16384
    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1
    audio_mode=0
    video_mode=0

The loopback upstream returned `RFB 003.008` before the run. The PS2 advertised
cumulative capabilities `255`, received the 496-byte CONFIG v4 payload, and
returned `H1_CONFIG_ACK=PASS`.

## Bridge / credit evidence

Saved `rfb_quiesce.json` records:

    channel_bytes_received=2492
    channel_bytes_sent=28874009
    channel_frames_received=250
    channel_frames_sent=3946
    credit_bytes_received=28906777
    credit_frames_received=3892
    server_bytes_received=28874009
    server_bytes_sent=2492
    server_read_calls=3947
    server_write_calls=250

Thus 28,874,009 raw RFB server bytes crossed the Pi bridge into PSTV logical
channel 1 and 2,492 client-direction RFB bytes crossed back toward the upstream
VNC server during the 60-second session.

The 28,906,777 received credit bytes equal the 28,874,009 forwarded server bytes
plus the configured 32,768-byte initial credit window. This is consistent with
all consumed bytes being returned and the full channel-1 credit window being
restored at the clean session boundary. It does not establish 32 KiB as an
optimal final product queue size.

## PS2 parser / presentation evidence

The PS2 returned:

    integrity_pass=1
    transport_error=0
    diagnostic_word=2717966580
    validation_failures=[]

`2717966580` is `0xA200E0F4` under the exact CP2K source diagnostic encoding:

- `0xA.......` is the H1 RFB diagnostic marker;
- state nibble `2` is `PSTVNC_RFB_SESSION_READY`;
- error byte `00` is `PSTVNC_RFB_SESSION_ERROR_NONE`;
- phase bits `0xE000` mean BOUNDARY sent, COMMIT observed, and COMPLETE sent; and
- low 13 bits `0x00F4` record **244 completed incremental framebuffer updates**.

The CP2K coordinator publishes only complete authoritative framebuffer states:
after the complete initial framebuffer and after complete incremental updates
that actually dirty the framebuffer. The presenter uses the existing
through-Issue-39 display/PS2 graphics path.

The decisive physical observation was supplied by the operator during this
exact session: the Pi desktop appeared correctly on the PS2 television, and a
terminal window dragged from the independent Windows VNC client moved visibly on
the PS2 display. Together with the 244 completed incremental updates, this
satisfies the CP2K visible-presentation gate rather than merely proving a static
initial frame.

## Clean quiesce evidence

The saved quiesce record states:

    requested=true
    boundary_received=true
    bridge_quiesced=true
    commit_sent=true
    complete_received=true

The live runner reported:

    H1_RFB_QUIESCE=COMPLETE boundary=1 bridge_stopped=1 commit=1 complete=1
    H1_SESSION_VALIDATION=PASS
    H1_PI_MUX_SESSION=PASS
    RUN_RC=0

`summary.json` recorded `validation_failures=[]`, `integrity_pass=1`, and
`transport_error=0`. Therefore CP2K retains CP2J's parser-boundary-aware finite
termination while visible presentation is active.

## Qualification conclusion

CP2K is hardware-qualified for this exact narrow path:

    local VNC :5903
      -> Pi raw RFB bridge
      -> logical PSTV channel 1
      -> one physical PSTV TCP connection
      -> PS2 credit-controlled RFB queue
      -> unchanged through-Issue-39 RFB parser
      -> authoritative 704x462 CPU framebuffer
      -> through-Issue-39 display / PS2 graphics presenter
      -> live visible PS2 television output

The qualification belongs to the exact PS2 ELF/PT_LOAD identity above together
with the successful Pi host/runtime checkout. A later documentation-only commit
that records this result does not itself become a newly hardware-tested binary.

## Still unqualified / next controlled boundary

Do not broaden this result to PS2-side controller/pointer input, keyboard input,
OSK, local UI, RFB+audio, RFB+MPEG, or shared compositor operation. Those remain
separate checkpoints.

The next clean checkpoint, if deliberately started, is to reintroduce PS2-side
input on top of the now-qualified visible RFB path while keeping audio and MPEG
OFF. That checkpoint is not started by this result record.

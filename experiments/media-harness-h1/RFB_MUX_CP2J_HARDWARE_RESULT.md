# CP2J RFB-over-mux hardware result

## Status

**HARDWARE QUALIFIED for the narrow CP2J RFB-only milestone.**

This result qualifies the exact CP2J candidate for:

- one physical PSTV TCP connection with RFB carried on logical channel 1;
- the Pi raw-RFB bridge and configurable RFB credit flow;
- the unchanged through-Issue-39 RFB session/parser running through the H1 mux I/O seam;
- the authoritative 704x462 CPU framebuffer, including the initial framebuffer and live incremental updates; and
- clean finite RFB shutdown at a complete protocol boundary using REQUEST -> BOUNDARY -> COMMIT -> COMPLETE before ordinary H1 `MEDIA_END` / `SESSION_RESULT` termination.

It does **not** qualify visible RFB presentation, controller/mouse/keyboard input, OSK/local UI, simultaneous RFB+MPEG or RFB+audio operation, the future shared compositor/media-object path, or any Issue #40 work.

## Scope guard

- Issue frontier: **through Issue #39**.
- Issue #40 started: **NO**.
- Physical PS2-facing PSTV streams: **1**.
- RFB logical channel: **1**.
- First hardware mode: **RFB only**; audio OFF; MPEG/video presenter OFF.
- RFB presentation on the TV: **not part of this checkpoint**. The test deliberately stopped at CPU-framebuffer authority.

## Exact source/build authority

Branch:

    experiment/h1-rfb-mux-prep

CP2J source/build head:

    9966d2e4aee7fff39826267f356ef8dc286c0909

Pinned CI authority:

    workflow_run=34307964666
    artifact_id=10087332537

Exact hardware-tested ELF:

    ELF_SHA256=d51681273aab15eab9922913b4f6e91a39b21820b8de1cfc8953fbb9f6a86ab3
    ELF_BYTES=3014524
    PT_LOAD_SHA256=ed14abe54ea84b31ef0fead13f8a2dfaf5c4e5a08e923d840542718b10790951
    PT_LOAD_BYTES=488084

The rolling `/mass/0/PS2VNC.ELF` and archival
`/mass/0/PS2VNC-H1-RFB-CP2J-HW1-d5168127.ELF` were independently downloaded
from the PS2 after deployment and both matched the whole-file SHA-256 and byte
count above.

A deployment evidence-writing permission error occurred only after the
canonical deployer had already uploaded and read back both files. A separate
user-writable recovery record was therefore captured at:

    /home/ps2/ps2vnc-evidence/deployments/H1-RFB-CP2J-HW1-recovered-readback.json

That infrastructure permission issue did not alter the ELF or the hardware
run.

## Hardware run identity

Run evidence directory on the Pi:

    /home/ps2/ps2vnc-evidence/h1-rfb-cp2j-hw1-20260909T040217Z

Session:

    profile=H1_RFB_ONLY
    profile_id=2
    session_id=1240552675
    duration_seconds=30
    PS2_peer=192.168.50.2:52432
    PSTV_listener=0.0.0.0:5902
    upstream_RFB=127.0.0.1:5903

The upstream loopback TigerVNC endpoint returned an RFB 3.8 banner before the
run. A separate Windows VNC client remained connected to the Pi's external VNC
endpoint and was used to generate obvious desktop activity during the run; it
did not consume or create a second PS2-facing transport.

## Exact RFB profile

Relevant CP2J controls:

    desktop_width=704
    desktop_height=462
    max_data_payload=8192
    receiver_thread_priority=63
    receiver_thread_stack_size=16384
    rfb_mode=1
    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1
    audio_mode=0
    video_mode=0

The PS2 advertised cumulative capabilities `255`, received the exact CONFIG v4
payload, and returned `H1_CONFIG_ACK=PASS` for this session.

## Bridge / credit evidence

`rfb_quiesce.json` recorded:

    channel_bytes_received=1122
    channel_bytes_sent=17125191
    channel_frames_received=113
    channel_frames_sent=2214
    credit_bytes_received=17157959
    credit_frames_received=2199
    server_bytes_received=17125191
    server_bytes_sent=1122
    server_read_calls=2215
    server_write_calls=113

Thus 17,125,191 raw RFB server bytes crossed the Pi bridge into PSTV channel 1
and 1,122 client-direction RFB bytes crossed back toward the VNC server during
the finite session.

The bridge observed 17,157,959 bytes of credit grants. That is exactly
17,125,191 forwarded server bytes plus 32,768 bytes, matching the configured
initial/full RFB credit window. This is consistent with all consumed bytes
being returned and the full 32 KiB credit window being restored at the clean
session boundary; it is not evidence that 32 KiB is an optimal final product
queue size.

## PS2 parser / framebuffer evidence

The PS2 returned:

    integrity_pass=1
    transport_error=0
    diagnostic_word=2717966443
    validation_failures=[]

`2717966443` is `0xA200E06B` under the CP2J RFB diagnostic encoding:

- `0xA.......` = H1 RFB diagnostic marker;
- state nibble `2` = `PSTVNC_RFB_SESSION_READY`;
- error byte `00` = `PSTVNC_RFB_SESSION_ERROR_NONE`;
- phase bits `0xE000` = BOUNDARY sent, COMMIT observed, COMPLETE sent; and
- low 13 bits `0x006B` = **107 completed incremental framebuffer updates**.

This proves the run was not merely an idle connected socket. The unchanged RFB
parser completed the initial framebuffer and then processed substantial live
incremental desktop activity into the PS2 CPU framebuffer.

## Clean quiesce evidence

The saved quiesce record states:

    requested=true
    boundary_received=true
    bridge_quiesced=true
    commit_sent=true
    complete_received=true

The live runner likewise reported:

    H1_RFB_QUIESCE=COMPLETE boundary=1 bridge_stopped=1 commit=1 complete=1
    H1_POST_RESULT_SOCKET_CLOSE=PASS classification=normal_session_boundary
    H1_SESSION_VALIDATION=PASS
    H1_PI_MUX_SESSION=PASS

The PS2-side coordinator accepts COMMIT as a clean boundary only after the Pi
has stopped/joined the raw VNC reader and the PS2's channel-1 receive queue is
empty. Therefore this qualification includes the finite-session termination
mechanism rather than depending on cutting the raw RFB stream at an arbitrary
time.

## Qualification conclusion

CP2J converts RFB-over-mux from a repository/CI preparation claim into a real
PS2 hardware result. For this exact candidate, RFB transport, configurable
credit accounting, the unchanged RFB parser, the authoritative CPU framebuffer,
live incremental update service, and clean finite shutdown are qualified on
hardware while preserving the one-physical-PSTV-stream architecture.

Do not broaden this result beyond what was actually exercised.

## Next controlled checkpoint

The next logical hardware checkpoint is **visible RFB-only presentation** while
keeping audio and MPEG OFF. Reuse the through-Issue-39 presentation path or a
single clearly owned compositor boundary; do not introduce concurrent MPEG
presentation yet. This isolates GS/presentation ownership from the now-qualified
RFB transport/parser path.

After visible RFB-only presentation is independently qualified, controller /
mouse / keyboard / OSK can be reintroduced as a separate cumulative checkpoint.
Only after those RFB-side pieces are stable should the experiment attempt the
shared RFB-background + MPEG-media-object compositor.

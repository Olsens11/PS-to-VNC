# CP2J — first RFB-only hardware candidate

Status: **CI READY — HARDWARE UNQUALIFIED**

CP2J is the first checkpoint at which the cumulative H1 experiment has a
complete, deliberately narrow RFB-over-mux path that is suitable for an exact
hardware test.

It is not a claim that RFB is qualified. Qualification begins only when the
exact ELF/PT_LOAD recorded below is deployed and observed on real PS2 hardware.

## Exact candidate authority

Branch:

    experiment/h1-rfb-mux-prep

Repository head used by the final green preparation run:

    9966d2e4aee7fff39826267f356ef8dc286c0909

GitHub Actions:

    workflow=H1 RFB mux preparation checks
    run=34307964666
    result=PASS

Pinned PS2 toolchain:

    ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11

Pinned PS2IP:

    baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a
    sha256=b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

Candidate executable identity:

    ELF_SHA256=d51681273aab15eab9922913b4f6e91a39b21820b8de1cfc8953fbb9f6a86ab3
    ELF_BYTES=3014524
    PT_LOAD_SHA256=ed14abe54ea84b31ef0fead13f8a2dfaf5c4e5a08e923d840542718b10790951
    PT_LOAD_BYTES=488084

`ELF_BYTES` was independently read from the preserved CI artifact after verifying
its whole-file SHA256 matched the candidate identity above.

Preserved CI artifact:

    artifact_name=h1-rfb-mux-prep-unqualified-elf
    artifact_id=10087332537
    artifact_zip_sha256=283298c7a542b78a12fd1e3a22cf21f72f1902ef9cab9c82e8655c02d5078377

The artifact name intentionally retains `unqualified`: passing CI does not
transfer physical qualification.

## First hardware profile

Canonical operator profile:

    H1_RFB_ONLY

Starting transport values:

    audio_mode=0
    video_mode=0
    rfb_mode=1
    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1
    receiver_thread_priority=63
    receiver_thread_stack_size=16384
    max_data_payload=8192

Those RFB queue/credit values are ordinary CONFIG v4 knobs, not hard-coded
product limits. Hardware evidence may justify later sweeps through `h1_tool.py`.

## What is active in this milestone

The hardware path is intentionally limited to:

    local/upstream ordinary VNC server
        -> Pi raw-byte RFB bridge
        -> PSTV logical channel 1
        -> one physical PS2-facing PSTV TCP connection
        -> one PS2 receiver thread
        -> independent RFB queue / credit accounting
        -> unchanged through-Issue-39 RFB parser/session
        -> authoritative 704x462 CPU framebuffer

The cumulative build alone advertises `CAP_RFB`. The ordinary H1 validator still
rejects RFB ON; only the cumulative activation wrapper accepts the RFB-only
combination and delegates all unrelated validation back to the original
validator.

## What is deliberately NOT active

The first test does not enable:

- AUDIO;
- MPEG;
- the old through-Issue-39 gsKit RFB presenter;
- controller/pointer input;
- keyboard;
- OSK/local UI;
- shared RFB+MPEG presentation;
- automatic media-region detection;
- Issue #40 or anything later.

Therefore **a visible RFB desktop on the television is not an expected success
criterion for CP2J**. The RFB session is headless at the presentation layer and
proves transport/parser/framebuffer/lifecycle behavior first.

The existing H1 video chassis is still initialized once at resident process
startup because that known-good process-lifetime chassis has not been disturbed
for this transport checkpoint. CP2J does not start the separate through-Issue-39
RFB graphics presenter and therefore does not introduce a second RFB GS owner.

## Clean finite-session termination

An RFB byte stream cannot be cut at an arbitrary duration boundary. CP2J uses
the verified four-phase quiesce mechanism:

    Pi REQUEST
        -> PS2 reaches an RFB protocol boundary
        -> PS2 BOUNDARY
        -> Pi shuts down and joins upstream VNC reader
        -> Pi COMMIT
        -> PS2 requires channel-1 queue empty
        -> PS2 COMPLETE
        -> Pi sends ordinary H1 MEDIA_END
        -> PS2 sends SESSION_RESULT

All lifecycle markers are zero-length channel-1 PSTV DATA frames and are
intercepted outside the RFB parser. Every non-empty channel-1 DATA frame remains
raw RFB bytes.

The PS2 may honor REQUEST after a completed initial frame, after a completed
incremental update, or while the parser reports IDLE before consuming another
server message. The IDLE case is necessary for a static desktop whose
outstanding incremental request may legitimately receive no update before the
test duration expires.

The COMMIT-side queue-empty proof remains conservative: if bytes from another
server message raced into channel 1 before the Pi bridge stopped, the test fails
instead of calling that termination clean.

## CI evidence completed before hardware

The final preparation workflow proves, without substituting for hardware:

- CONFIG v4 profile self-test passes;
- the clean RFB parser resolves its I/O only to the H1 mux seam in this target;
- cumulative RFB activation-gate host tests pass;
- RFB queue/fragmentation mechanics pass;
- RFB receiver-credit policy tests pass;
- Pi raw-byte bridge tests pass;
- existing H1 physical-reader integration tests pass;
- clean REQUEST/BOUNDARY/COMMIT/COMPLETE quiesce tests pass;
- cumulative Pi-runner composition tests pass;
- `H1_RFB_ONLY --validate-only` passes through the actual cumulative bridge
  runner;
- `h1_tool.py` exposes the RFB knobs and named profile;
- pinned PS2 build and source/object ownership checks pass;
- whole-ELF and PT_LOAD identity are recorded above.

## Operator entry point

The canonical control surface is now:

    python3 experiments/media-harness-h1/h1_tool.py run \
        --profile H1_RFB_ONLY \
        --duration 30

Do not replace this with a second bespoke Pi RFB test program. `h1_tool.py`
delegates to the cumulative bridge runner and preserves the same evidence path
used by later sweeps.

Before the first physical run, deployment/read-back should verify the exact
ELF SHA and byte count above. Any source or PT_LOAD change creates a new
candidate and resets the hardware gate.

## First hardware success criteria

The first run should establish, with console/evidence/packet data where useful:

1. exact candidate ELF identity is deployed/read back;
2. one PS2-facing PSTV TCP connection is used;
3. HELLO advertises RFB capability;
4. CONFIG ACK accepts exactly `H1_RFB_ONLY`;
5. ordinary RFB 3.8 handshake completes through channel 1;
6. initial framebuffer is parsed into the CPU framebuffer;
7. incremental update activity or legitimate IDLE progress is observed;
8. queue/credit accounting remains coherent;
9. the finite-duration quiesce reaches COMPLETE;
10. MEDIA_END and SESSION_RESULT complete cleanly;
11. no unexplained silent stall is hidden by automatic recovery.

If the run fails, classify the lowest layer still proven to make progress before
changing queue capacity, scheduling, parser semantics, or presentation code.

## Next checkpoint after hardware

If the exact CP2J candidate passes the RFB-only hardware test, record that
physical evidence before introducing another major variable. Only then should
work proceed toward a single presentation/compositor owner and later RFB+MPEG
composition.

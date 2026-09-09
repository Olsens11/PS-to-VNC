# CP2H — Pi RFB bridge integrated with the existing H1 session reader

Status: **COMPLETE — HOST/CI VERIFIED, PS2 RFB GATE STILL CLOSED, NOT HARDWARE QUALIFIED**

CP2H completes the Pi-side P3 raw-byte bridge preparation without changing the
qualified H1 physical receive owner or opening the PS2 RFB activation gate.

## Preserved architecture

- The PS2 still has exactly one PSTV TCP connection to the Pi.
- `H1Session.reader()` remains the sole Pi-side reader of that physical PSTV
  connection; no second PSTV reader or PS2-facing socket was introduced.
- RFB remains logical PSTV channel 1.
- The Pi bridge opens only one ordinary upstream/local VNC connection, default
  `127.0.0.1:5900`, and only for an explicitly RFB-enabled profile.
- The bridge does not parse or invent RFB message boundaries. It moves raw bytes
  between the upstream VNC stream and PSTV channel 1.
- VNC-server reads are directly limited by PS2-granted RFB receiver credit. When
  outstanding RFB credit reaches zero the bridge stops reading upstream and
  relies on TCP backpressure instead of adding an arbitrary staging reservoir.
- Existing AUDIO/MPEG/control handling remains owned by the original H1 reader.
- RFB-OFF opens no VNC socket, starts no RFB bridge worker, and does not install
  the channel-1 receive shim.

## Narrow reader integration

`h1_rfb_session_adapter.py` deliberately avoids copying or replacing the
monolithic known-good H1 reader.

A registered experiment-local `receive_frame()` shim runs on the same existing
H1 reader thread. For a registered RFB session it consumes only channel-1
frames. Every non-RFB frame is returned untouched to the ordinary reader.

Because PSTV receive sequence numbers are global across all logical channels,
the shim performs the same expected-sequence check/increment for an intercepted
RFB frame before dispatching it. This allows RFB CREDIT/DATA to interleave with
AUDIO/MPEG/control traffic without creating a second sequence authority.

Channel-1 behavior is:

    PS2 CREDIT ch1
        -> add exactly that amount to upstream VNC read budget

    VNC bytes, while credit exists
        -> existing H1Session.send_frame(DATA, ch1, payload)
        -> existing Pi send lock / global PSTV send sequence

    PS2 DATA ch1
        -> exact sendall() to the already-owned upstream VNC socket

The PS2-to-VNC write is intentionally synchronous on the existing reader thread
for this checkpoint. RFB client writes are small and infrequent relative to
framebuffer traffic, and no evidence currently justifies inventing another
queue or worker. If hardware evidence later shows this path can block meaningful
PSTV progress, that evidence should drive a separate change.

## Cumulative runner composition

`h1_mux_server_cumulative39_rfb_bridge.py` composes the verified bridge with the
current cumulative through-Issue-39 thread-census runner rather than replacing
it.

It subclasses the already-composed `H1Session` and adds only optional upstream
RFB attachment. Capture geometry, AUDIO/MPEG scheduling, telemetry decoding,
result validation, and the original H1 reader implementation remain inherited.

The canonical `h1_tool.py` runner is deliberately **not** redirected to this
wrapper yet. `rfb_mode=1` is still rejected by the authoritative PS2 CONFIG
validator, so exposing the new wrapper as if it were operational hardware would
be premature.

## Host proof

The raw bridge test proves:

- no VNC-server bytes leave the Pi before channel-1 credit exists;
- an 8-byte credit grant releases exactly 8 bytes;
- the remainder waits for replenishment;
- payloads never exceed configured H1 maximum payload;
- PS2-to-VNC bytes are exact;
- an over-capacity credit window fails closed.

The H1 reader integration test additionally uses the real existing
`H1Session.reader()` with socket-pair endpoints and proves:

- RFB-OFF never invokes the VNC connector;
- ordinary HELLO and CONFIG ACK still use the base reader;
- intercepted RFB CREDIT advances the shared receive sequence correctly;
- an ordinary AUDIO CREDIT immediately after an RFB frame is accepted;
- PS2 channel-1 DATA reaches the VNC peer exactly;
- replenished RFB credit releases only the remaining upstream bytes;
- an ordinary MPEG CREDIT after RFB DATA/CREDIT is accepted;
- the final shared expected receive sequence is correct;
- the base H1 reader reports no error.

The cumulative runner composition test proves the RFB-OFF path is inert and the
RFB-ON preparation path opens exactly one configured upstream VNC attachment
with the selected 32768-byte queue and 8192-byte max PSTV payload.

## Verification

Final CP2H CI evidence:

    workflow=H1 RFB mux preparation checks
    run=34303403391
    result=PASS
    head=eda7dcffb8f03ddbc17dc86ad31f651c4075771b
    H1_PI_PROFILES_SELF_TEST=PASS
    H1_RFB_MUX_SEAM_SOURCE=PASS
    H1_RFB_CHANNEL_TEST=PASS
    H1_RFB_CREDIT_POLICY_TEST=PASS
    H1_RFB_PI_BRIDGE_TEST=PASS
    H1_RFB_SESSION_ADAPTER_TEST=PASS
    H1_CUMULATIVE39_RFB_BRIDGE_RUNNER_TEST=PASS
    pinned_ps2_build=PASS
    PS2_ELF_SHA256=1ee11891c6f60a793230555ae87c99e319fbc593492f4447b68fcef4d9832c98
    PS2_PT_LOAD_SHA256=a3f90c558083087ebdb522b334cd47944562c2ae70c7ad3ae90d0240b0e0639d
    PS2_PT_LOAD_BYTES=484628
    ARTIFACT_ID=10085753540

The PS2 ELF/PT_LOAD are intentionally unchanged from CP2G because CP2H changes
only Pi-side Python/orchestration and CI. This is useful evidence that the
bridge work did not silently alter the PS2 executable.

## Still deliberately closed

CP2H does **not**:

- accept `rfb_mode=1` in PS2 CONFIG;
- advertise `PSTVNC_TRANSPORT_CAP_RFB` in HELLO;
- run the through-Issue-39 RFB parser/session from the H1 PS2 coordinator;
- allocate the authoritative RFB framebuffer for an H1 RFB session;
- present RFB pixels through the GS;
- start input, keyboard, pointer, OSK, or local UI;
- combine RFB and MPEG graphics ownership;
- begin Issue #40.

No RFB hardware claim is authorized by this checkpoint.

## Next checkpoint

Before opening CONFIG/CAP_RFB, add the PS2-side RFB session/coordinator behind
the still-closed gate and add enough RFB diagnostics to identify progress.

The first coordinator step should remain transport-focused:

1. allocate the authoritative CPU RFB framebuffer only for `rfb_mode=ON`;
2. start the unchanged through-Issue-39 RFB session against the H1 mux I/O
   adapter using the already-owned H1 transport identity;
3. complete the ordinary RFB handshake and full initial framebuffer in CPU
   memory;
4. service incremental updates while recording parser/update/queue/credit
   progress;
5. do **not** start the old gsKit presenter, local input, or OSK in that same
   checkpoint;
6. keep `rfb_mode=ON` rejected and CAP_RFB unadvertised until the coordinator,
   diagnostics, and session termination contract are build-verified.

Presentation ownership remains a separate problem because the old through-39
RFB presenter and H1 MPEG path are distinct GS owners. Transport qualification
must not be used as an excuse to start both.

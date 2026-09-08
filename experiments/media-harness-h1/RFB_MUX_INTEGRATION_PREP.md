# H1 RFB mux integration preparation

Status: **preparation only; no RFB-on runtime implementation in this commit**.

This note records the controlled path for activating the already-reserved RFB
logical channel inside the resident H1 one-socket transport. It exists so the
next implementation step can proceed from repository authority rather than
reconstructing the integration plan from conversation history.

## Scope boundary

This work belongs to the H1 hybrid-media experiment. It does not begin clean
reconstruction Issue #40 and does not change the through-Issue-39 product
qualification authority.

The transport invariant is non-negotiable:

    one PS2-facing PSTV TCP connection
        CONTROL   channel 0
        RFB       channel 1
        AUDIO     channel 2
        TELEMETRY channel 3
        MPEG2     channel 4

Do not reintroduce a second competing PS2-facing RFB TCP stream.

The first implementation milestone is RFB-over-mux with AUDIO/MPEG disabled.
Only after that transport path is independently proven should shared
RFB+MPEG presentation be activated.

## Existing pieces already present

The branch already contains:

- reserved PSTV RFB channel/capability vocabulary;
- one H1 physical TCP connection and one EE receiver thread;
- independent AUDIO and MPEG queue/credit mechanisms;
- the clean through-Issue-39 RFB parser/session;
- the `pstvnc_rfb_io_read_exact`, `pstvnc_rfb_io_poll_receive`, and
  `pstvnc_rfb_io_write_exact` seam;
- the authoritative clean desktop framebuffer;
- through-Issue-39 input, keyboard, OSK, and local-UI modules linked into the
  cumulative H1 ELF but deliberately not started;
- the H1 operator tool, which already exposes `--rfb on` but correctly fails
  closed while the PS2 CONFIG validator rejects the reserved value.

The missing work is therefore integration, not a new RFB implementation.

## Current blockers to `--rfb on`

1. `pstvnc_h1_config_validate()` accepts only `PSTVNC_H1_RFB_OFF`.
2. H1 HELLO does not advertise RFB capability.
3. H1 transport runtime has only AUDIO and MPEG receive queues/credit accounting.
4. H1 DATA dispatch rejects channel 1 because no enabled RFB queue exists.
5. The clean RFB I/O seam is currently implemented by direct socket I/O in
   `src/platform/ps2_network.c`; H1 needs a mux-backed adapter instead.
6. The Pi H1 server does not bridge a local/upstream VNC byte stream to PSTV
   channel 1 in both directions.
7. The current clean RFB presenter and H1 raw-GS MPEG presenter are distinct GS
   owners. They must not be started together until presentation ownership is
   unified.

## Ownership decision for the first RFB-over-mux milestone

Keep the H1 EE receiver thread as the **sole physical PSTV `recv()` owner**.
RFB protocol code must never call `recv()` on that physical mux socket.

Give RFB its own logical byte queue, allocated only when `rfb_mode=ON`.
The RFB I/O adapter reads exact protocol bytes from that queue and serializes
RFB writes back through H1's existing send lock as channel-1 DATA frames.

This preserves both important existing invariants:

- one physical PSTV socket receiver; and
- one synchronized RFB protocol owner at the clean RFB-session level.

The adapter is an implementation of the existing `rfb_io.h` contract, not a
rewrite of `rfb_session.c` parsing semantics.

## Initial RFB queue size

Use 32768 bytes for the first controlled integration unless evidence requires a
change. This is not an arbitrary new buffer increase: the through-Issue-39
direct RFB transport already owns a 32768-byte receive-prefetch buffer in
`src/platform/ps2_network.c`.

Treat that value as an evidence-based starting point, not a permanent product
limit. The H1 control surface should make any later queue-capacity field explicit
rather than hiding a larger allocation.

## Proposed PS2 implementation sequence

### P1 — compile-time transport adapter with RFB still OFF

- Add H1-owned mux RFB I/O implementation behind the existing `rfb_io.h`
  function names for the H1 cumulative build only.
- Do not modify `rfb_session.c` parser behavior.
- Do not enable RFB CONFIG or capability advertising yet.
- Host/build checks must prove that the cumulative H1 target resolves exactly
  one implementation of each `pstvnc_rfb_io_*` symbol and does not pull in the
  direct RFB socket implementation from `ps2_network.c`.

This is a no-runtime-change structural seam check.

### P2 — channel-1 queue and symmetric send path

When `rfb_mode=ON`:

- allocate one RFB queue and semaphore;
- accept only channel-1 DATA into that queue;
- maintain independent queue/credit counters;
- provide exact-read and poll semantics from the queue;
- provide exact logical-write semantics by fragmenting arbitrary RFB writes into
  PSTV DATA payloads no larger than `max_data_payload` and sending them under
  the existing H1 send semaphore;
- return RFB receiver credit as bytes are consumed from the queue;
- leave AUDIO/MPEG queues completely absent when those modes are OFF.

Do not share AUDIO or MPEG queue storage with RFB.

### P3 — Pi byte-stream bridge

The Pi side owns two connections:

- the single PSTV connection to the PS2; and
- a local/upstream ordinary VNC TCP connection to the Pi desktop's VNC server.

Bridge bytes without interpreting RFB framing:

    VNC -> PSTV channel-1 DATA -> PS2 RFB queue
    PS2 channel-1 DATA -> VNC

The Pi bridge may schedule among active logical PSTV channels, but it must not
invent RFB message boundaries or modify RFB protocol bytes.

### P4 — RFB-only hardware qualification

First hardware profile:

    RFB   ON
    AUDIO OFF
    MPEG  OFF
    local input/UI initially OFF unless needed for the specific test

Prove:

- one PSTV physical TCP session;
- ordinary RFB 3.8 handshake through the bridge;
- complete initial framebuffer;
- incremental framebuffer updates;
- clean pointer/key writes if input is included;
- channel-1 queue/credit integrity;
- no second PS2-facing TCP flow;
- no unexplained silent stall introduced by the mux adapter.

Capture packet evidence at the PS2-facing interface so the one-physical-stream
claim is empirical rather than inferred from source alone.

## Presentation boundary after RFB transport is proven

Do **not** solve RFB transport by starting both existing graphics owners.

The later hybrid layer needs one compositor/presentation owner with explicit
order:

    authoritative RFB desktop
        -> MPEG semantic media object
            -> local cursor / OSK / local UI

RFB should continue decoding into the authoritative CPU framebuffer under an
active MPEG region even when RFB pixels are not presented there. When the MPEG
overlay disappears, the RFB image is therefore immediately current.

The media object's semantic geometry remains exact desktop/host-relative pixel
geometry. MPEG macroblock padding is a codec detail and must not redefine the
media object's bounds. See `MEDIA_OBJECT_COMPOSITION_NOTES.md`.

## Dirty-rectangle/media-discovery preparation

Do not build the media detector from the framebuffer's single conservative
unioned dirty rectangle alone.

Preserve or observe individual RFB update rectangles before they are collapsed
into the framebuffer dirty union. A later detector can accumulate temporal
activity in a convenient tile grid, identify sustained dense regions, attach a
candidate to the owning X11 window, then retain exact semantic pixel bounds.

The first detector milestone should only **report** its candidate; it must not
automatically steer MPEG until its result can be compared against visible
ground truth.

## Instrumentation required before hardware activation

Add diagnostics sufficient to answer which layer still progresses:

- PSTV channel-1 DATA frames/bytes received;
- RFB logical bytes consumed by the parser;
- RFB credit bytes returned;
- RFB logical bytes written toward Pi/VNC;
- RFB queue current/high-water;
- RFB parser/session state and last completed server-message/update count;
- existing H1 receiver heartbeat/thread census;
- no changes to the debugging policy for unexplained silent stalls.

A static desktop is not proof that RFB/application execution is alive.

## Fail-closed rules

- `rfb_mode=OFF` must allocate no RFB queue and start no RFB-specific runtime.
- HELLO advertises RFB only in a build/runtime that can actually support it.
- `rfb_mode=ON` must be rejected until all required resources and bridge
  semantics are present.
- Unexpected DATA on a disabled logical channel remains a transport error.
- RFB queue exhaustion is evidence/failure, not permission to silently grow the
  queue.
- A partial/failed RFB logical write fails the synchronized RFB session.
- Do not auto-reconnect unexplained silent freezes while this integration is
  being qualified.

## Operator-tool contract

`h1_tool.py` remains the canonical test control surface.

Once RFB activation exists, `--rfb on` should become operational without a new
parallel test script. Any new explicit H1 CONFIG field (for example an RFB queue
capacity if chosen) must automatically appear in `knobs` and the raw field
help, with a friendly alias only if repeated use demonstrates that one is
worthwhile.

RFB-only and later hybrid optimization sweeps should use the same `run`/`sweep`
mechanism so batch tests remain declarative and reproducible.

## Stop points requiring hardware/operator participation

Repository-only preparation can cover architecture audit, adapters, host tests,
Pi bridge logic, build integration, CONFIG/profile validation, CLI help, and
build identity.

Do not claim the changed ELF is qualified until David can participate in:

- physical launch/deployment confirmation;
- RFB-only runtime observation;
- packet/evidence capture where needed;
- subjective presentation/input checks;
- later combined RFB+MPEG compositor tests.

A changed PT_LOAD remains hardware-gated under project policy.

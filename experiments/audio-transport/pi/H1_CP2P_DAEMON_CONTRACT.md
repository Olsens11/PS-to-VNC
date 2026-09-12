# H1 CP2P Pi Daemon Reconstruction Contract

Status: current implementation contract for the H1 all-guns CP2P experiment.

This document is intentionally different from the chronological experiment history. The history records what was believed and proved at each point in time. This file records the **current behavior that a Pi-side daemon must recreate** if the daemon is rewritten, replaced, or reconstructed from scratch.

If this document and executable source disagree, stop and reconcile the discrepancy before hardware qualification. Do not silently choose one.

## 1. Architectural boundary

The CP2P daemon extends the existing PSTV mux architecture; it does not create a second MPEG control or data connection.

Hard invariants:

- one physical PS2-facing PSTV transport connection;
- one Pi-side reader owns reads from that connection;
- one serialized Pi-side writer owns writes to that connection;
- RFB, PCM audio, telemetry, MPEG control, and MPEG elementary-stream bytes remain logical channels of that same PSTV transport;
- PS2 -> Pi MPEG START is a control use of PSTV `DATA` on logical channel 4;
- Pi -> PS2 MPEG elementary-stream bytes are also PSTV `DATA` on logical channel 4; direction distinguishes the two uses;
- no second MPEG socket, second PS2 transport reader, or bypass writer is permitted.

The existing transport framing authority is `experiments/audio-transport/common/transport_protocol.h`.

## 2. Relevant PSTV framing

PSTV v1 frame header is 16 bytes:

| Bytes | Meaning |
| --- | --- |
| 0..3 | magic `0x50535456` (`PSTV`) |
| 4 | transport version = 1 |
| 5 | frame kind |
| 6 | logical channel |
| 7 | flags |
| 8..11 | direction-local sequence, big endian |
| 12..15 | payload length, big endian |

Relevant values:

- `FRAME_DATA = 3`
- `CHANNEL_RFB = 1`
- `CHANNEL_AUDIO = 2`
- `CHANNEL_MPEG2 = 4`
- maximum PSTV payload = 8192 bytes

All ordinary transport validation and direction-local sequence validation occurs before CP2P START interpretation.

## 3. H1 session identity

H1 CONFIG v4 contains `PSTVNC_H1_FIELD_SESSION_ID = 1` and the PS2 session coordinator uses the active transport config's `session_id` when it serializes START.

The Pi is therefore required to retain the authoritative session ID for the active H1 CONFIG and reject a START whose embedded `session_id` does not equal that active session.

A START from an old connection/session must never activate MPEG in a newer session.

## 4. PS2 -> Pi MPEG START message

A START is exactly one PSTV frame with:

- frame kind: `DATA` (3)
- channel: `MPEG2` (4)
- payload length: exactly 44 bytes
- direction: PS2 -> Pi

The 44-byte payload is eleven unsigned 32-bit words in big-endian order:

| Word | Offset | Meaning |
| ---: | ---: | --- |
| 0 | 0 | START wire version; currently 1 |
| 1 | 4 | H1 `session_id` |
| 2 | 8 | MPEG generation; nonzero |
| 3 | 12 | exact capture/draw X |
| 4 | 16 | exact capture/draw Y |
| 5 | 20 | exact capture/draw width |
| 6 | 24 | exact capture/draw height |
| 7 | 28 | RFB suppression X |
| 8 | 32 | RFB suppression Y |
| 9 | 36 | RFB suppression width |
| 10 | 40 | RFB suppression height |

The PS2 source authority is:

- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_wire.h`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_wire.c`
- `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_transport.c`

### Structural validation

The daemon must reject START unless all of the following are true:

- payload length is exactly 44;
- wire version is exactly 1;
- embedded session ID equals the active H1 session ID;
- generation is nonzero;
- all geometry words fit the implementation's nonnegative integer representation;
- draw X and Y are nonnegative;
- draw width and height are at least 16;
- draw width and height are multiples of 16;
- suppression X and Y are nonnegative;
- suppression width and height are positive;
- suppression rectangle fully contains the exact draw/capture rectangle;
- exact draw rectangle is inside the actual active desktop/capture surface;
- suppression rectangle is valid for the active RFB desktop surface.

The wire payload deliberately carries no duplicate canvas dimensions. The Pi must validate bounds against the **actual active capture/RFB desktop**, not against guessed constants.

Inner matte settings are PS2 presentation state and are intentionally not present on the wire.

## 5. Meaning of the two rectangles

The two rectangles are different authorities and must never be conflated.

### Exact draw/capture rectangle

`draw_x, draw_y, draw_width, draw_height` is the exact source rectangle the user confirmed in calibration. This exact rectangle drives Pi-side capture/encoding.

For an FFmpeg-style capture process, the equivalent semantic is:

`capture source = desktop region X,Y,W,H exactly as encoded in START`

Do not expand this rectangle to match suppression and do not substitute old CONFIG draw geometry.

### Suppression rectangle

`suppression_x, suppression_y, suppression_width, suppression_height` is the RFB protection footprint. It may be slightly larger than the exact capture rectangle so normal desktop repaint cannot create seams around the MPEG presentation.

The suppression rectangle must be installed before MPEG production for the generation becomes eligible to emit bytes.

The exact RFB suppression implementation mechanism remains a source-grounded implementation choice. The required externally visible semantics are fixed:

- RFB continues normally outside the suppression footprint;
- RFB must not repaint pixels covered by the active generation's suppression footprint;
- suppression belongs to one exact MPEG generation;
- removing/retiring a generation removes its suppression obligation;
- the mechanism must preserve the qualified single RFB connection and request/reply correctness.

Do not invent a second VNC connection merely to implement suppression.

## 6. Generation semantics

A generation is an immutable MPEG presentation epoch.

Do **not** model resize/move as mutation of an existing generation.

The settled PS2 lifecycle is:

1. a calibration transaction is confirmed;
2. the PS2 session coordinator calls `prepare_start()`;
3. `prepare_start()` allocates a fresh nonzero generation and arms PS2 presentation state as `WAIT_FIRST_FRAME`;
4. the PS2 sends exactly one START for that generation;
5. the Pi validates START, installs that generation's suppression, starts exact-region capture, and emits MPEG bytes tagged/owned by that generation's lifecycle;
6. the PS2 does not promote the generation to MPEG-owned merely because bytes arrive;
7. PS2 ownership becomes MPEG-owned only after a decoded frame for that generation is physically presented through the shared compositor/GS flip path.

A later calibration confirm creates a **new** generation even if geometry is unchanged.

There is no long-running history of edits to one generation.

## 7. START processing order on the Pi

The required order is deliberate:

```text
receive PSTV frame in sole PS2 reader
    -> verify transport frame + sequence
    -> identify PS2->Pi DATA/channel-4 START
    -> require exactly 44-byte payload
    -> decode wire version/session/generation/geometry
    -> validate active session and desktop bounds
    -> establish generation state
    -> install generation-scoped RFB suppression
    -> configure exact X,Y,W,H capture
    -> start/activate MPEG producer for that generation
    -> only then permit MPEG DATA for that generation onto PSTV channel 4
```

**Suppression before production is a hard ordering rule.** A producer must never race ahead and emit video while RFB is still allowed to repaint the same area.

If any validation or setup step before producer activation fails, that generation must not become active and must not emit MPEG bytes.

## 8. Producer startup state

For the all-guns CP2P session:

- RFB may be active from session startup;
- PCM audio may be active from session startup;
- MPEG capture/encoding must be dormant until a valid START is received;
- START supplies the live capture rectangle; startup CONFIG geometry is not a substitute for the user's accepted calibration;
- producer lifecycle must be generation-scoped;
- only one MPEG generation is active at a time for the current milestone.

The current historical mux daemon (`experiments/audio-transport/pi/mux_daemon.py`) already demonstrates the required single-reader/single-writer mux ownership for RFB + PCM, but it does **not** yet implement CP2P START handling or the deferred MPEG producer lifecycle. A reconstruction must preserve its transport ownership discipline while adding those pieces.

## 9. Recalibration / retirement semantics the daemon must support

START+SELECT is currently only a PS2 hardware-test shortcut for requesting calibration. It is **not** part of the Pi protocol and the Pi daemon must not depend on that chord. A future UI button is expected to replace it without changing this daemon contract.

The semantic lifecycle is trigger-agnostic:

- requesting calibration ends the currently active MPEG generation, if any;
- the last confirmed calibration rectangle/matte settings remain available on the PS2 as the seed for the next calibration transaction;
- the PS2 restores one full RFB presentation before freezing the screen for calibration;
- cancel/back-out creates no new MPEG generation and leaves the user in full-frame live RFB + PCM;
- confirm creates a fresh generation and a fresh START, whether the rectangle changed or not.

### Retirement wire status

At the time this document was created, the PS2-side semantic retirement behavior is settled, but a final Pi-facing retirement wire encoding has **not yet been established in the current source tree**. Do not infer retirement from silence, timeout, or a guessed START variant.

Before the all-guns candidate is sealed, the Pi implementation must have an explicit generation-safe way to stop/drain the old producer and remove its suppression when the PS2 retires that generation. Once that exact wire/control seam is implemented, update this section with the concrete encoding and source authority.

For the first RFB-only -> MPEG START path, no prior MPEG generation exists and therefore no retirement message is required before START.

## 10. Failure and stale-generation rules

The daemon must be conservative around stale state:

- malformed START: reject; do not alter the current valid generation;
- wrong session ID: reject;
- generation 0: reject;
- stale/retired generation traffic: must never reacquire active ownership;
- producer setup failure: do not emit MPEG for that generation;
- transport failure: terminate/quiesce session resources rather than silently opening an alternate socket;
- a newer generation must not coexist with an older producer for the current milestone;
- stopping one generation must be able to prove producer dormancy before resources/state are reused for another generation.

No automatic "best effort" geometry correction is permitted. Invalid geometry is an error, not something the daemon clamps behind the PS2's back.

## 11. Reconstructable Pi state model

A minimal daemon implementation needs only a small amount of MPEG lifecycle state:

```text
active_h1_session_id
active_desktop_width / active_desktop_height
mpeg_generation = none | N
mpeg_start_contract = none | immutable START(N)
mpeg_suppression_installed = false/true
mpeg_producer = dormant/running/stopping
```

There is no need to retain the historical evolution of prior generations. Once a generation is retired and its producer is dormant, its operational state can be discarded.

Persisted user calibration settings are a PS2/session-UI concern, not a Pi generation-history concern.

## 12. Suggested daemon decomposition

Keep responsibilities small and replaceable:

- `PSTV reader`: sole socket reader; framing + sequence ownership;
- `START decoder/validator`: pure 44-byte parser and geometry/session validation;
- `RFB suppression owner`: installs/removes one generation's suppression footprint;
- `MPEG producer`: owns FFmpeg/capture subprocess and exact X,Y,W,H source;
- `MPEG lifecycle`: orders suppression -> producer start and producer stop -> suppression removal;
- existing scheduler/writer: remains the sole Pi->PS2 frame writer and schedules RFB/PCM/MPEG logical-channel payloads.

Do not merge these into an opaque monolithic loop merely because the first prototype is small.

## 13. Required reconstruction tests

At minimum, a replacement daemon should prove these behaviors without PS2 hardware before qualification:

- valid 44-byte START decodes exactly;
- bad version/length/session/generation/geometry is rejected;
- draw rectangle and suppression rectangle remain distinct;
- desktop-bound validation rejects off-screen capture/suppression;
- suppression is installed before the producer can emit its first byte;
- exact START X,Y,W,H reaches the producer command/configuration;
- producer remains dormant before START;
- duplicate/stale generation input cannot create two active producers;
- retirement stops/drains the exact active generation once the retirement wire seam is defined;
- RFB and PCM ownership paths remain alive while MPEG is active;
- only the existing PSTV writer emits PS2-facing frames.

## 14. Source map for reconstruction

Current source authorities to consult together:

- PSTV framing/channel vocabulary: `experiments/audio-transport/common/transport_protocol.h`
- historical Pi mux ownership: `experiments/audio-transport/pi/mux_daemon.py`
- H1 session/config vocabulary: `experiments/media-harness-h1/h1_config.h`
- START in-memory handoff: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_handoff.[ch]`
- START wire: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_wire.[ch]`
- START transport sender: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_transport.[ch]`
- PS2 CP2P session coordinator: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`
- generation presentation owner: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.[ch]`
- recalibration lifecycle: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_recalibration.[ch]`
- combined RFB/MPEG flow policy: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.[ch]`
- chronological evidence/history: `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`

## 15. Current implementation boundary

At creation of this document:

- PS2 accepted -> fresh generation -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;
- the old experimental macro/global recalibration bridge has been removed;
- START+SELECT is explicitly only a replaceable test trigger;
- Pi START parsing, generation-scoped suppression, deferred exact-region MPEG producer activation, and explicit Pi retirement signaling are the next implementation work;
- physical all-guns hardware qualification has not yet occurred.

Update this file whenever one of those Pi-side mechanisms becomes concrete, but preserve the experiment history separately rather than rewriting historical entries.

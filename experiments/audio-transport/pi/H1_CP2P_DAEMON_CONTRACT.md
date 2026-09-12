# H1 CP2P Pi Integration / Reconstruction Contract

Status: current integration contract for the H1 all-guns CP2P experiment.

This document is intentionally different from the chronological experiment history. The history records what was believed and proved at each point in time. This file records the **current Pi-side behavior, components, dependencies, configuration, and lifecycle requirements that the eventual comprehensive PS-to-VNC Pi setup must be able to recreate**.

This experiment is **not** intended to become a separate permanent Pi installation stack. The long-term project will have one comprehensive Pi-side bootstrap/installer and runtime/service arrangement that prepares the Pi generally for PS-to-VNC: networking, SMB, VNC/desktop services, audio, media support, project utilities, dependencies, configuration, and service startup/recovery. CP2P contributes requirements and implementation pieces to that larger system.

The rule for this experiment is therefore:

> Anything CP2P creates, depends on, configures, or requires on the Pi must be recorded here well enough that the future comprehensive Pi bootstrap can install or recreate it without relying on this chat, an old shell history, or undocumented manual setup.

If this document and executable source disagree, stop and reconcile the discrepancy before hardware qualification. Do not silently choose one.

## 0. Relationship to the future comprehensive Pi bootstrap

The eventual general Pi bootstrap/service stack is the installation authority. It should absorb the proven pieces from this experiment rather than launching a second independent CP2P environment.

When CP2P is promoted out of the experiment, the comprehensive Pi setup must include, as applicable:

- the PSTV mux/runtime code that owns the single PS2 transport;
- the CP2P START decoder and lifecycle state;
- RFB suppression support;
- the MPEG capture/encoder producer and its exact-region startup logic;
- any new scheduler/channel support needed for MPEG data;
- runtime configuration/schema additions;
- service/unit integration needed to launch and recover the Pi-side runtime;
- every external executable/library/package required by those pieces;
- permissions, groups, environment variables, sockets, paths, or desktop/session assumptions they rely on;
- host-side tests and self-checks that prove the installed Pi has the required capabilities.

Do **not** treat a command installed manually during this experiment as an acceptable permanent dependency record. Whenever the experiment begins relying on a new Pi-side dependency or manual configuration step, update this contract at the same time with the executable/capability required and, once known, the canonical installation/configuration mechanism that the comprehensive bootstrap must perform.

Current executable-level dependencies already visible in the experimental Pi path include:

- Python 3 for the current mux daemon implementation;
- a local RFB/VNC provider reachable by the mux (the current experiment targets an isolated X0tigervnc provider);
- PipeWire command-line facilities used by the current PCM path, including `wpctl` and `pw-record`;
- an MPEG capture/encode facility will be required by CP2P; the exact command line and installation authority must be recorded here when the deferred producer is made concrete rather than guessed in advance.

Package names are deliberately not guessed here because they can vary by Pi OS/distribution. The comprehensive bootstrap should ultimately own the distro-specific package mapping.

## 1. Architectural boundary

CP2P extends the existing PSTV mux architecture; it does not create a second MPEG control or data connection.

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

The Pi runtime must reject START unless all of the following are true:

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

### Current Pi implementation authority (item #5)

The receive/validation side is now concrete and host-proven in the experimental tree. The future comprehensive Pi runtime must absorb the semantics currently implemented by:

- `experiments/media-harness-h1/h1_cp2p_start_receiver.py` — pure START decode/validation, same-reader dispatch, and immutable prepared-generation state;
- `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py` — narrow CP2P runner integration layered onto the existing visible-RFB/optional-PCM Pi runner;
- `experiments/media-harness-h1/h1_cp2p_start_receiver_test.py` — focused host contract for valid/invalid START, desktop bounds, generation state, and same-reader dispatch;
- `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md` — item-#5 implementation/proof contract.

The item-#5 implementation introduces no new external executable, OS package, socket, service, or receive thread. It uses Python 3 and the already-existing H1/RFB Pi runtime environment. Promotion into the comprehensive Pi stack is therefore principally a **runtime/code integration and self-test requirement**, not a new package-install requirement.

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

### Prepared-generation state proven by item #5

On the Pi, a structurally valid START does not immediately mean “producer running.” Item #5 introduced an intermediate immutable **prepared request**:

- at most one generation may be prepared at a time;
- a second START cannot replace an unreleased prepared generation;
- release must name the exact prepared generation;
- after a generation has been prepared/released, that generation or any older generation is stale;
- preparation stores the exact base and suppression rectangles without installing suppression, changing capture geometry, or starting MPEG.

This prepared state is the handoff that later Pi suppression/capture/producer work must consume rather than reparsing or reconstructing START independently.

## 7. START processing order on the Pi

The required order is deliberate:

```text
receive PSTV frame in sole PS2 reader
    -> verify transport frame + sequence
    -> identify PS2->Pi DATA/channel-4 START
    -> require exactly 44-byte payload
    -> decode wire version/session/generation/geometry
    -> validate active session and desktop bounds
    -> establish immutable prepared-generation state   [implemented/proven in item #5]
    -> install generation-scoped RFB suppression             [future item #6]
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

The historical base mux daemon (`experiments/audio-transport/pi/mux_daemon.py`) remains the transport-ownership precedent and does not itself contain CP2P START handling. Item #5 now supplies that handling experimentally through `h1_cp2p_start_receiver.py` and the narrow `h1_mux_server_cp2p_start_receiver.py` wrapper while preserving the same sole-reader call chain. Deferred suppression/capture/producer activation remains unimplemented. The permanent result must fold the proven START receiver/prepared-state semantics into the comprehensive Pi runtime/bootstrap rather than install the experimental wrapper as an unrelated second daemon.

## 9. Recalibration / retirement semantics the Pi runtime must support

START+SELECT is currently only a PS2 hardware-test shortcut for requesting calibration. It is **not** part of the Pi protocol and the Pi runtime must not depend on that chord. A future UI button is expected to replace it without changing this contract.

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

The Pi runtime must be conservative around stale state:

- malformed START: reject; do not alter the current valid generation;
- wrong session ID: reject;
- generation 0: reject;
- stale/retired generation traffic: must never reacquire active ownership;
- producer setup failure: do not emit MPEG for that generation;
- transport failure: terminate/quiesce session resources rather than silently opening an alternate socket;
- a newer generation must not coexist with an older producer for the current milestone;
- stopping one generation must be able to prove producer dormancy before resources/state are reused for another generation.

No automatic "best effort" geometry correction is permitted. Invalid geometry is an error, not something the Pi clamps behind the PS2's back.

## 11. Reconstructable Pi state model

A minimal implementation needs only a small amount of MPEG lifecycle state:

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

## 12. Suggested runtime decomposition

Keep responsibilities small and replaceable even if the comprehensive Pi runtime eventually hosts them in one service:

- `PSTV reader`: sole socket reader; framing + sequence ownership;
- `START decoder/validator`: pure 44-byte parser and geometry/session validation;
- `RFB suppression owner`: installs/removes one generation's suppression footprint;
- `MPEG producer`: owns the capture/encoder subprocess and exact X,Y,W,H source;
- `MPEG lifecycle`: orders suppression -> producer start and producer stop -> suppression removal;
- existing scheduler/writer: remains the sole Pi->PS2 frame writer and schedules RFB/PCM/MPEG logical-channel payloads.

Logical decomposition does not require separate permanent daemons. Prefer one comprehensively managed service architecture with clear internal ownership over a collection of competing background processes.

## 13. Required reconstruction/install tests

At minimum, the comprehensive bootstrap/runtime that absorbs CP2P should be able to prove these behaviors without PS2 hardware before qualification:

- every external CP2P executable/library capability is present after a fresh bootstrap;
- required service/user permissions and runtime paths are created reproducibly;
- valid 44-byte START decodes exactly (current host authority: `h1_cp2p_start_receiver_test.py`);
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

A fresh-Pi reconstruction is not complete merely because source files were copied. The bootstrap must reproduce the runtime dependencies and system configuration those files require.

## 14. Promotion inventory for the comprehensive Pi setup

As CP2P work proceeds, use this section as the handoff checklist. Every row that becomes concrete should point to an exact source/config/install authority rather than tribal knowledge.

| Area | Experimental/current authority | What the comprehensive Pi setup must eventually own |
| --- | --- | --- |
| PSTV framing | `experiments/audio-transport/common/transport_protocol.h` | matching protocol/runtime installation |
| single-reader/single-writer mux ownership | `experiments/audio-transport/pi/mux_daemon.py` | permanent Pi transport service/runtime |
| H1 config/session identity | `experiments/media-harness-h1/h1_config.h` | config parsing and active-session state |
| MPEG START wire | PS2: `mpeg_presentation_calibration/h1_mpeg_start_wire.[ch]`; Pi: `h1_cp2p_start_receiver.py` | preserve exact 44-byte decoder/validator and desktop-bound checks |
| MPEG START receive/routing | `h1_cp2p_start_receiver.py`; `h1_mux_server_cp2p_start_receiver.py` | absorb same-reader DATA/channel-4 routing into permanent Pi runtime; do not create another socket/reader |
| MPEG START send semantics | `mpeg_presentation_calibration/h1_mpeg_start_transport.[ch]` | preserve compatibility with inbound channel-4 START routing |
| generation lifecycle | PS2 start-handoff/presentation-owner/recalibration; Pi prepared state in `h1_cp2p_start_receiver.py` | preserve immutable prepared-generation state, exact release, and stale-generation rejection; later extend with producer lifecycle |
| RFB suppression | implementation still to be made concrete | install/configure chosen suppression mechanism |
| MPEG capture/encoder | implementation still to be made concrete | install encoder/capture dependency and exact-region launch logic |
| MPEG mux scheduling | implementation still to be made concrete | channel-4 queue/credit/scheduling support without second socket |
| retirement control | wire encoding still to be made concrete | exact-generation stop/drain/remove-suppression path |
| PCM capture | current mux uses PipeWire `wpctl` + `pw-record` | install/configure audio capture prerequisites |
| RFB provider | current mux expects isolated local VNC provider | install/configure VNC provider under the general desktop/VNC setup |
| service management | not finalized by this experiment | comprehensive service startup/restart/dependency ordering |
| verification | item #5: `h1_cp2p_start_receiver_test.py`, proof run `34702828032`; later all-guns qualification | include START decoder/prepared-state/same-reader regression in fresh-install self-check plus project regression tests |

When a row moves from “to be made concrete” to an implementation, update the row in the same tranche that introduces it.

## 15. Source map for reconstruction

Current source authorities to consult together:

- PSTV framing/channel vocabulary: `experiments/audio-transport/common/transport_protocol.h`
- historical Pi mux ownership: `experiments/audio-transport/pi/mux_daemon.py`
- H1 session/config vocabulary: `experiments/media-harness-h1/h1_config.h`
- START in-memory handoff: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_handoff.[ch]`
- START wire: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_wire.[ch]`
- START transport sender: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_transport.[ch]`
- Pi START decoder/prepared-state owner: `experiments/media-harness-h1/h1_cp2p_start_receiver.py`
- Pi START receiver integration runner: `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`
- Pi START host contract: `experiments/media-harness-h1/h1_cp2p_start_receiver_test.py`
- Pi START implementation/proof note: `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md`
- PS2 CP2P session coordinator: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`
- generation presentation owner: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.[ch]`
- recalibration lifecycle: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_recalibration.[ch]`
- combined RFB/MPEG flow policy: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.[ch]`
- chronological evidence/history: `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`

## 16. Current implementation boundary

At this document's current revision:

- PS2 accepted -> fresh generation -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;
- the old experimental macro/global recalibration bridge has been removed;
- START+SELECT is explicitly only a replaceable test trigger;
- Pi START receive/validation and immutable prepared-generation state are implemented and host-proven by item #5 without adding a second socket, reader, receive thread, producer, or package dependency;
- generation-scoped suppression, deferred exact-region MPEG producer activation, and explicit Pi retirement signaling remain future implementation work;
- the permanent comprehensive Pi bootstrap has **not** yet absorbed these experimental CP2P additions;
- item #5 proof authority is branch `experiment/h1-cp2p-pi-start-receive-validation`, tested/documented source head `c37c22c64d159d8f1814a3462ac6dbfa2e0b4557`, GitHub Actions run `34702828032` / job `103577491937`, with 14 focused START tests plus existing RFB regressions passing;
- the sealed item-#5 branch head is `5a839b16f2d9458bc1e5c4089b90ffc942e4f3e0`;
- physical all-guns hardware qualification has not yet occurred.

Update this file whenever a Pi-side mechanism, dependency, path, service requirement, or configuration step becomes concrete. Preserve the experiment history separately rather than rewriting historical entries.

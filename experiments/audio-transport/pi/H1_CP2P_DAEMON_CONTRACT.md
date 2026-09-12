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
- FFmpeg is now a live CP2P runtime dependency. Item #8 launches the exact START-derived x11grab command for one immutable generation after suppression and capture preparation succeed. The comprehensive bootstrap must eventually map the `ffmpeg` capability to the supported Pi OS package/install authority before this experiment is promoted.

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

The experimental RFB suppression implementation is now concrete and host-proven for item #6. CP2P substitutes `h1_cp2p_rfb_suppression.py` into the existing CP2O RFB construction seam: suppression is generation-scoped, begins pending, activates immediately before the first new FramebufferUpdateRequest after START, and filters Raw server FramebufferUpdate rectangles around the active suppression footprint. The required externally visible semantics remain:

- RFB continues normally outside the suppression footprint;
- RFB must not repaint pixels covered by the active generation's suppression footprint;
- suppression belongs to one exact MPEG generation;
- removing/retiring a generation removes its suppression obligation;
- the mechanism must preserve the qualified single RFB connection and request/reply correctness.

Do not invent a second VNC connection merely to implement suppression.

The suppression parser's Raw transaction bound is derived from the **current active desktop** as `desktop_width * desktop_height * 2`; no single resolution is hard-coded. The fixed factor two is the already-negotiated 16-bit RFB pixel format. Host coverage explicitly exercises 640x448, 704x462, 704x480, and 1280x720. The local `retire_suppression_exact(generation)` seam removes only the named generation; it is an implementation primitive for later item #11A, not by itself the cross-machine retirement control path.

### Current exact-capture preparation authority (item #7)

The Pi now prepares one immutable capture plan directly from the START base rectangle through `experiments/media-harness-h1/h1_cp2p_capture_geometry.py`. The active desktop dimensions are validation bounds only. The prepared x11grab source is `DISPLAY+X,Y` with `-video_size W×H` from the accepted START base rectangle; no fixed startup-profile rectangle, desktop-sized substitute, crop-to-fit stage, or scale workaround is allowed. The command is evidence/preparation state only until item #8 launches the producer.

Compound START preparation is atomic at the software boundary: if any step after immutable START validation fails, exact suppression (if installed), usable prepared START state, capture state, and prepared-state evidence are rolled back together. The failed generation remains above the item-#5 generation high-water and therefore remains stale/rejected rather than becoming reusable.

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

The required order is deliberate and the pre-producer portion is now concrete:

```text
receive PSTV frame in sole PS2 reader
    -> verify transport frame + sequence
    -> identify PS2->Pi DATA/channel-4 START
    -> require exactly 44-byte payload
    -> decode wire version/session/generation/geometry
    -> validate active session and current desktop bounds
    -> establish immutable prepared-generation state           [item #5 DONE]
    -> install same-generation RFB suppression as pending      [item #6 DONE]
    -> prepare exact START-base X,Y,W,H capture command        [item #7 DONE]
    -> launch one exact-generation local FFmpeg producer       [item #8 DONE]
    -> open exact-generation emission admission for accepted N  [item #10 DONE]
    -> send channel-4 MPEG only under N's emission lease         [item #10 DONE]
```

Pending suppression becomes active immediately before forwarding the first **new** FramebufferUpdateRequest after START. An older request already outstanding at acceptance may complete unchanged while PS2 presentation remains frozen in `WAIT_FIRST_FRAME`.

Suppression before production remains a hard ordering rule. A producer must never race ahead and emit video while RFB is still allowed to repaint the same area.

The START preparation/launch path is one fail-closed transaction. If suppression, capture-plan preparation/evidence, or producer launch fails, usable state for that generation is unwound only as far as quiescence can be proven while the generation high-water remains stale. Once a producer has launched, producer stop/drain is the first rollback step; if that exact producer cannot be proven quiescent, suppression and prepared-generation state are deliberately retained rather than exposing a potentially live MPEG source. No failed compound preparation may later become producer-eligible.

## 8. Producer startup state

Item #8 is now concrete at the software/pre-public-gate boundary:

- RFB may be active from session startup;
- PCM audio may be active from session startup;
- START supplies the live capture rectangle; startup CONFIG geometry is not a substitute for the user's accepted calibration;
- `h1_cp2p_capture_geometry.py` prepares the exact FFmpeg x11grab command from the immutable START base rectangle;
- only after same-generation suppression and exact capture preparation succeed does `h1_cp2p_mpeg_producer.py` launch one generation-owned FFmpeg process;
- the producer is attached to the existing H1 session as the video producer; no second PS2-facing socket, transport reader, or bypass writer is introduced;
- item #10 now opens the CP2P-only MPEG CONFIG/emission composition: START remains the sole producer authority, while each Pi->PS2 MPEG DATA send must acquire the exact active generation's emission lease and use the existing serialized PSTV writer;
- the all-guns profile is registered only by the CP2P runner; generic H1 and CP2O retain their previous composition policy and cannot accidentally select the CP2P MPEG lifecycle;
- only one MPEG producer generation may be active at a time; wrong-generation retirement is rejected;
- exact retirement first requests producer stop, drains/discards every locally buffered unsent byte, and proves the subprocess/reader quiescent before suppression may be removed;
- if graceful stop cannot prove quiescence, termination is attempted; if quiescence still cannot be proven, retirement fails closed and generation ownership/suppression are retained rather than acknowledged.

The current experimental runner `h1_mux_server_cp2p_start_receiver.py` owns the ordered START -> suppression -> exact capture -> producer transaction while preserving the same sole-reader call chain. `h1_cp2p_mpeg_producer_test.py`, the retirement-control contract, and the compound START rollback contract jointly prove the producer lifecycle and its fail-closed boundaries. The permanent comprehensive Pi runtime/bootstrap must absorb these semantics rather than install the experiment as an unrelated second daemon.

FFmpeg is therefore now a real runtime capability required by the experiment, not merely a planned command. The distro-specific installation/package authority still belongs to the future comprehensive Pi bootstrap and is deliberately not guessed here.

## 9. Recalibration / retirement semantics the Pi runtime must support

START+SELECT is currently only a PS2 hardware-test shortcut for requesting calibration. It is **not** part of the Pi protocol and the Pi runtime must not depend on that chord. A future UI button is expected to replace it without changing this contract.

The semantic lifecycle is trigger-agnostic:

- requesting calibration ends the currently active MPEG generation, if any;
- the last confirmed calibration rectangle/matte settings remain available on the PS2 as the seed for the next calibration transaction;
- the PS2 restores one full RFB presentation before freezing the screen for calibration;
- cancel/back-out creates no new MPEG generation and leaves the user in full-frame live RFB + PCM;
- confirm creates a fresh generation and a fresh START, whether the rectangle changed or not.

### Exact-generation retirement control (item #11A)

The Pi-facing retirement seam is now concrete and host/PS2-build proven. It uses one H1 experiment-local PSTV frame kind on **control channel 0**, not MPEG DATA channel 4:

- frame kind: `PSTVNC_H1_FRAME_MPEG_RETIRE = 10`;
- flags: `0`;
- payload length: exactly 12 bytes;
- payload: three unsigned big-endian 32-bit words: `version=1`, `session_id`, `generation`;
- request direction: PS2 -> Pi;
- completion direction: Pi -> PS2 using the identical payload;
- direction plus each side's strict one-pending-generation state distinguish request from completion.

The PS2 transport publishes the exact pending generation **before** sending the request so an immediate Pi completion cannot race the receiver thread. The sole receiver accepts completion only when version, active session id, nonzero generation, and exact pending generation all match. The application coordinator polls that receiver-owned completion asynchronously; it never blocks the sole transport receive owner.

The ordering is deliberately **Pi first** during recalibration. While generation N remains the authoritative PS2 MPEG owner, the coordinator sends RETIRE(N) and waits. The Pi completion is not sent until exact Pi runtime cleanup succeeds. Only after the PS2 observes completion does the existing local worker/presentation retirement run and create the fresh full-RFB restoration obligation. Therefore a failed or missing Pi retirement cannot leave the PS2 locally retired while the Pi still suppresses RFB, and no fresh full-RFB restoration can begin before Pi cleanup is proven complete.

Item #8 now extends this exact transaction with the real generation-owned producer rather than inventing a second retirement protocol. The Pi validates one matching prepared START, capture plan, suppression generation, and producer generation; exact producer stop/drain/discard is the first destructive step; only after the producer subprocess and reader are proven quiescent may exact-generation suppression be removed, prepared START/capture state and evidence be released, and completion be sent. An unexpected live legacy producer without the generation owner still blocks acknowledgement. If producer quiescence cannot be proven, cleanup stops fail-closed before suppression removal and no completion is sent. The prepared-generation high-water is retained, so the retired generation remains stale and cannot be reused.

Item #11B is now concrete and software-proven as the live data-plane generation boundary. It uses the existing ordered PSTV/TCP stream plus the existing bounded PS2 MPEG ring as an epoch boundary rather than adding per-packet generation tags or a second queue. The Pi closes exact-generation emission admission and waits for any in-flight send before its RETIRE completion; the sole PS2 receiver treats that exact completion as the wire-order fence, immediately closes channel-4 admission, then—after the old worker is stopped—atomically discards any residual generation-N ring bytes, returns their withheld credit, and clears the retirement latch. A fresh N+1 transport generation cannot open until the old queue/credit/retirement state is clean. Item #10 now uses this boundary for the public all-guns software composition: generation N is opened before START, each MPEG send is protected by N's exact emission lease, retirement makes the lease one-way closed, and no fresh generation can emit until #11B's old-epoch finalization is clean.

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
- valid 44-byte START decodes exactly (`h1_cp2p_start_receiver_test.py`);
- bad version/length/session/generation/geometry is rejected;
- draw rectangle and suppression rectangle remain distinct;
- desktop-bound validation rejects off-screen capture/suppression;
- suppression budget follows the current desktop rather than one fixed resolution (`h1_cp2p_rfb_suppression_test.py`);
- pending suppression activates before the first new RFB update request after START and clips Raw updates outside the owned footprint;
- exact START X,Y,W,H becomes the prepared `DISPLAY+X,Y` / `-video_size W×H` FFmpeg source (`h1_cp2p_capture_geometry_test.py`);
- the producer remains dormant through #5/#6/#7 preparation and launches only at item #8's final same-generation activation step;
- a post-suppression capture-preparation failure rolls back suppression/prepared/capture usable state while preserving stale-generation high-water (`h1_cp2p_start_preparation_transaction_test.py`);
- duplicate/stale generation input cannot create two active producers;
- exact RETIRE request/completion rejects wrong session/generation and does not acknowledge until exact Pi state is released (`h1_cp2p_retirement_control_test.py`);
- PS2 local MPEG ownership remains intact while Pi retirement is pending, and full-RFB restoration begins only after exact Pi completion (`h1_cp2p_session_coordinator_test.c`);
- item #8 exact retirement stops/drains/discards that exact producer's unsent local bytes and proves process/reader quiescence before suppression removal and completion (`h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_retirement_control_test.py`);
- failed post-launch rollback that cannot prove producer quiescence retains suppression/prepared state and fails closed (`h1_cp2p_start_preparation_transaction_test.py`);
- item #10 opens MPEG only for the CP2P all-guns composition; CP2O retains its prior visible-RFB/PCM policy (`h1_config_cp2p_activation_gate_test.c`, CP2O gate regression);
- START remains the sole producer authority; CONFIG enables capability but does not start FFmpeg; live channel-4 sends use the existing serialized writer only after an exact-generation emission lease is acquired (`h1_cp2p_item10_activation_test.py`);
- the item-#11B Pi emission fence is closed by default, exact-generation only, and RETIRE waits for an in-flight emission lease before producer stop/drain (`h1_cp2p_mpeg_producer_test.py`);
- the shared PS2 queue discard primitive empties wrapped residual data without reallocating or lowering queue high-water telemetry (`transport_queue_generation_boundary_test.c`);
- the PS2 coordinator opens one exact MPEG transport generation before START, aborts it on START-send failure, latches the ordered RETIRE completion, stops the old worker before residual queue discard/final-credit return, and cannot open N+1 until the old epoch is finalized (`h1_cp2p_session_coordinator_test.c`);
- the CP2P-only all-guns profile accepts MPEG2_ES without broadening generic H1/CP2O policy;
- finite-session MPEG accounting is derived from payloads actually sent on PSTV, including streaming picture/start-code observation rather than unsent producer archives;
- generated CP2P runtime contains no obsolete standalone-GS `transfer_packet`/`draw_packet` first-frame prerequisite, and unexpected worker exit is sticky-failed while expected retirement cancellation remains valid;
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
| MPEG START wire | PS2: `mpeg_presentation_calibration/h1_mpeg_start_wire.[ch]`; Pi: `h1_cp2p_start_receiver.py` | preserve exact 44-byte decoder/validator and current-desktop-bound checks |
| MPEG START receive/routing | `h1_cp2p_start_receiver.py`; `h1_mux_server_cp2p_start_receiver.py` | absorb same-reader DATA/channel-4 routing into permanent Pi runtime; do not create another socket/reader |
| MPEG START send semantics | `mpeg_presentation_calibration/h1_mpeg_start_transport.[ch]` | preserve compatibility with inbound channel-4 START routing |
| generation lifecycle | PS2 start-handoff/presentation-owner/recalibration; Pi prepared state in `h1_cp2p_start_receiver.py` | preserve immutable prepared-generation state, exact release, stale-generation rejection, and atomic rollback; later extend with producer lifecycle |
| RFB suppression | `h1_cp2p_rfb_suppression.py`; `h1_cp2p_rfb_suppression_test.py` | absorb generation-scoped pending/active Raw filtering, current-desktop byte budgeting, and exact retirement primitive without another VNC/PSTV reader |
| MPEG capture geometry | `h1_cp2p_capture_geometry.py`; `h1_cp2p_capture_geometry_test.py` | install/own exact START-base x11grab geometry and FFmpeg executable dependency; active desktop remains bounds authority only |
| compound START / producer activation | `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_start_preparation_transaction_test.py` | preserve ordered START -> suppression -> exact capture -> generation-owned producer launch and the fail-closed rollback barrier that retains suppression if producer quiescence cannot be proven |
| MPEG producer | `h1_cp2p_mpeg_producer.py`; `h1_cp2p_mpeg_producer_test.py`; integration in `h1_mux_server_cp2p_start_receiver.py` | own one exact START generation, launch only the prepared exact capture command, expose output only through the item-#10 leased scheduler, and prove exact stop/drain before suppression release; never become an alternate PSTV writer |
| MPEG mux scheduling / generation fence | Pi exact-generation emission lease in `h1_cp2p_mpeg_producer.py`; item-#10 scheduler integration in `h1_mux_server_cp2p_start_receiver.py`; PS2 epoch gate/finalizer in `h1_transport_runtime.[ch]`; queue discard in `experiments/audio-transport/common/transport_queue.[ch]`; `CP2P_LIVE_GENERATION_BOUNDARY.md` | preserve one ordered PSTV writer/reader, exact generation admission, leased MPEG DATA sends, RETIRE-as-wire-fence semantics, residual old-epoch queue discard + final credit, and fresh-generation refusal until the old epoch is clean |
| CP2P all-guns activation | PS2 `h1_config_cp2p_activation_gate.c`, CP2P main/makefile; Pi CP2P-only profile/scheduler in `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_item10_activation_test.py`; `CP2P_ITEM10_ALL_GUNS_ACTIVATION.md` | preserve CP2P-only MPEG capability, START-owned producer activation, exact-generation leased channel-4 sends, truthful sent-stream accounting, and unchanged CP2O policy |
| retirement control | Pi: `h1_cp2p_retirement_control.py`, `h1_cp2p_start_receiver.py`, `h1_mux_server_cp2p_start_receiver.py`, `h1_cp2p_mpeg_producer.py`; PS2: `h1_transport_runtime.[ch]`, `h1_cp2p_session_coordinator.[ch]`; tests: `h1_cp2p_retirement_control_test.py`, `h1_cp2p_mpeg_producer_test.py`, `h1_cp2p_session_coordinator_test.c` | preserve kind-10 exact retirement, Pi producer/emission quiescence before completion, completion as the ordered receive-side MPEG epoch fence, old-worker stop before residual queue discard/final-credit return, and only then local presentation retirement/full-RFB restoration |
| PCM capture | current mux uses PipeWire `wpctl` + `pw-record` | install/configure audio capture prerequisites |
| RFB provider | current mux expects isolated local VNC provider | install/configure VNC provider under the general desktop/VNC setup |
| service management | not finalized by this experiment | comprehensive service startup/restart/dependency ordering |
| verification | items #5/#6/#7/#8/#11A/#11B/#10 host contracts; clean #10 run `34718230541` (host `103619197119`, PS2 `103619197057`) | include START, same-reader, suppression/capture, generation-owned producer, fail-closed rollback, exact retirement, Pi emission lease/fence, wrapped-ring discard, ordered ACK epoch fence, old-worker-before-queue-finalize ordering, fresh-generation clean-state gate, CP2P-only all-guns CONFIG, leased MPEG scheduling, truthful wire accounting, generated-runtime first-frame guard regression, and CP2O preservation in fresh-install self-check plus project regressions |

When a row moves from future to concrete implementation, update this table in the same tranche.

## 15. Source map for reconstruction

Current source authorities to consult together:

- PSTV framing/channel vocabulary: `experiments/audio-transport/common/transport_protocol.h`
- historical Pi mux ownership: `experiments/audio-transport/pi/mux_daemon.py`
- H1 session/config vocabulary: `experiments/media-harness-h1/h1_config.h`
- START in-memory handoff: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_handoff.[ch]`
- START wire: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_wire.[ch]`
- START transport sender: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_transport.[ch]`
- Pi START decoder/prepared-state owner: `experiments/media-harness-h1/h1_cp2p_start_receiver.py`
- Pi START/suppression/capture integration runner: `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`
- Pi START host contract: `experiments/media-harness-h1/h1_cp2p_start_receiver_test.py`
- Pi generation-scoped RFB suppression: `experiments/media-harness-h1/h1_cp2p_rfb_suppression.py`
- dynamic/current-desktop suppression contract: `experiments/media-harness-h1/h1_cp2p_rfb_suppression_test.py`
- exact START-base capture plan: `experiments/media-harness-h1/h1_cp2p_capture_geometry.py`
- exact capture geometry contract: `experiments/media-harness-h1/h1_cp2p_capture_geometry_test.py`
- generation-owned START-driven Pi MPEG producer: `experiments/media-harness-h1/h1_cp2p_mpeg_producer.py`
- producer lifecycle + exact-generation emission-fence contract: `experiments/media-harness-h1/h1_cp2p_mpeg_producer_test.py`
- CP2P-only all-guns Pi scheduler/profile + finite-session accounting: `experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py`
- CP2P all-guns activation host contract: `experiments/media-harness-h1/h1_cp2p_item10_activation_test.py`
- PS2 CP2P-only CONFIG gate: `experiments/media-harness-h1/h1_config_cp2p_activation_gate.c`
- item-#10 implementation/proof note: `experiments/media-harness-h1/CP2P_ITEM10_ALL_GUNS_ACTIVATION.md`
- compound START/producer rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`
- live MPEG epoch-boundary design authority: `experiments/media-harness-h1/CP2P_LIVE_GENERATION_BOUNDARY.md`
- shared residual-queue discard authority: `experiments/audio-transport/common/transport_queue.[ch]` and `transport_queue_generation_boundary_test.c`
- Pi exact-generation retirement wire/control codec: `experiments/media-harness-h1/h1_cp2p_retirement_control.py`
- Pi exact-generation retirement contract: `experiments/media-harness-h1/h1_cp2p_retirement_control_test.py`
- PS2 exact retirement transport owner: `experiments/media-harness-h1/h1_transport_runtime.[ch]`
- PS2 retirement/recalibration coordinator ordering: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`
- Pi START implementation/proof note: `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md`
- PS2 CP2P session coordinator: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`
- generation presentation owner: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.[ch]`
- recalibration lifecycle: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_recalibration.[ch]`
- combined RFB/MPEG flow policy: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.[ch]`
- chronological evidence/history: `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`

## 16. Current implementation boundary

At this document's current revision:

- PS2 accepted -> fresh generation -> worker arm -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;
- START+SELECT remains explicitly only a replaceable test trigger;
- Pi START receive/validation and immutable prepared-generation state are implemented and host-proven by item #5 without adding a second PS2-facing socket, reader, or receive thread;
- item #6 generation-scoped Pi RFB suppression is concrete and host-proven: pending suppression activates on the first new update request, Raw updates are clipped around the exact suppression footprint, and the transaction budget derives from the current desktop dimensions rather than a hard-coded resolution;
- item #7 exact calibrated geometry is DONE at the current software boundary: START base X/Y/W/H becomes the exact FFmpeg x11grab source and item #8 consumes that same immutable plan in a live generation-owned producer; current desktop dimensions remain validation bounds only;
- item #8 START-driven MPEG production remains the producer-lifecycle authority: suppression and exact capture preparation precede one generation-owned FFmpeg launch; item #10 now exposes that producer's output through the exact-generation leased MPEG scheduler rather than creating a second activation path;
- item #8 extends item #11A retirement exactly as intended: exact producer stop/drain/discard and subprocess/reader quiescence precede suppression removal and Pi completion; a producer that cannot be proven quiescent leaves suppression/prepared state retained fail-closed and cannot be acknowledged;
- item #11A exact-generation Pi retirement control is therefore DONE with the live local producer present: kind-10 request/completion remains exact-session/exact-generation matched, PS2 local MPEG ownership remains intact while completion is pending, and fresh full-RFB restoration is created only after exact Pi completion;
- item #11B live cross-machine MPEG generation boundary is DONE: Pi exact-generation emission admission is fenced before RETIRE completion; ordered TCP/PSTV completion closes PS2 channel-4 generation admission; the old worker stops before residual old-generation ring bytes are discarded and all withheld credit is returned; N+1 cannot open until the old epoch is completely finalized; no per-packet generation tag, second MPEG queue, socket, reader, or writer was added;
- item #10 exact CP2P CONFIG/all-guns activation is DONE at the committed-source software-proof boundary: CP2P alone accepts MPEG2_ES, START remains the sole producer authority, every live channel-4 send is protected by the exact-generation emission lease and existing serialized writer, retirement cannot reopen a closing generation, sent-stream MPEG accounting is truthful, CP2O behavior remains unchanged, and generated CP2P first-frame presentation no longer depends on obsolete standalone-GS packets;
- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;
- item #8 product source authority is branch `experiment/h1-cp2p-start-driven-producer`, commit `0347e2f1a0295a83aa7051fc9b963b7f3ece85a2`; clean committed-source verification used wrapper head `51e1141f252edec2a7c1b19f445d941033e6fa19`, whose only product-independent delta is the one-line CI trigger comment;
- item #8 clean proof is GitHub Actions run `34712568840`, host job `103603873398`, PS2 regression job `103603873498`;
- item #11B product source authority is branch `experiment/h1-cp2p-live-generation-boundary`, commit `14d2b5fc68e11756356af397cfe3766e03c89008`; clean committed-source verification used wrapper head `bb56f36b4b0071e87819b338b4efa98a1e00740d`, whose only product-independent delta is the one-line CI trigger comment;
- item #11B clean proof is GitHub Actions run `34713445142`, host job `103606261825`, PS2 build job `103606261723`;
- item #11B pinned PS2 identity is ELF SHA256 `4f36d9b742598aa64c0bdd15b436ddc1d558b88278139a9a0edc19aa1e2f7ffd`, ELF bytes `3238440`, PT_LOAD SHA256 `8e91cde73f655a770c0c507a19bcde2081f43e74e4d4b29d47cc72d52a530fc3`, PT_LOAD bytes `512020`;
- item #10 official product source is branch `experiment/h1-cp2p-all-guns-activation`, commit `6c24fddbf4618d2e0bf69eba909be640dcff005f`, built as a clean one-commit continuation from the sealed #11B line; development product tree `28d86b1ca68b8582a77683e5f4306fe15e3c468e` was reparented without changing its durable tree;
- item #10 clean committed-source proof is GitHub Actions run `34718230541`, host job `103619197119`, PS2 build job `103619197057`; proof wrapper `a09fd93b02b4604370370d21626712b28c183ca8` differs from the product tree only by the clean-proof workflow;
- item #10 pinned all-guns software-candidate identity is ELF SHA256 `89d8d007ae76292be1542739e47897caf30292163a26950026126c71be905aad`, ELF bytes `3238996`, PT_LOAD SHA256 `4c7da3948483e27576583b9b80bb99e108b84f853aafa602f36ab66681657494`, PT_LOAD bytes `512276`; unqualified artifact `h1-cp2p-item10-all-guns-unqualified-elf` is artifact ID `10305431287`;
- physical all-guns hardware qualification has not yet occurred; item #12 is now the remaining qualification milestone.

Update this file whenever a Pi-side mechanism, dependency, path, service requirement, or configuration step becomes concrete. Preserve the experiment history separately rather than rewriting historical entries.

#!/usr/bin/env python3
"""One-shot item #6/#7 promotion patch for the current Pi reconstruction contract."""

from pathlib import Path

path = Path("experiments/audio-transport/pi/H1_CP2P_DAEMON_CONTRACT.md")
text = path.read_text()


def replace_once(old: str, new: str) -> None:
    global text
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"expected exactly one match, got {count}: {old[:100]!r}")
    text = text.replace(old, new, 1)


def replace_section(heading: str, next_heading: str | None, body: str) -> None:
    global text
    start = text.find(heading)
    if start < 0:
        raise SystemExit(f"missing section heading: {heading}")
    if text.find(heading, start + 1) >= 0:
        raise SystemExit(f"duplicate section heading: {heading}")
    if next_heading is None:
        end = len(text)
    else:
        end = text.find(next_heading, start + len(heading))
        if end < 0:
            raise SystemExit(f"missing next section heading: {next_heading}")
    text = text[:start] + body.rstrip() + "\n\n" + text[end:]


replace_once(
    "- an MPEG capture/encode facility will be required by CP2P; the exact command line and installation authority must be recorded here when the deferred producer is made concrete rather than guessed in advance.",
    "- FFmpeg is now the concrete MPEG capture/encode executable assumed by the prepared CP2P capture plan. Item #7 prepares its exact x11grab command but does not launch it; item #8 remains responsible for producer activation. The comprehensive bootstrap must eventually map the `ffmpeg` capability to the supported Pi OS package/install authority before the live producer is promoted.",
)

replace_once(
    "The exact RFB suppression implementation mechanism remains a source-grounded implementation choice. The required externally visible semantics are fixed:",
    "The experimental RFB suppression implementation is now concrete and host-proven for item #6. CP2P substitutes `h1_cp2p_rfb_suppression.py` into the existing CP2O RFB construction seam: suppression is generation-scoped, begins pending, activates immediately before the first new FramebufferUpdateRequest after START, and filters Raw server FramebufferUpdate rectangles around the active suppression footprint. The required externally visible semantics remain:",
)

replace_once(
    "Do not invent a second VNC connection merely to implement suppression.",
    "Do not invent a second VNC connection merely to implement suppression.\n\nThe suppression parser's Raw transaction bound is derived from the **current active desktop** as `desktop_width * desktop_height * 2`; no single resolution is hard-coded. The fixed factor two is the already-negotiated 16-bit RFB pixel format. Host coverage explicitly exercises 640x448, 704x462, 704x480, and 1280x720. The local `retire_suppression_exact(generation)` seam removes only the named generation; it is an implementation primitive for later item #11A, not by itself the cross-machine retirement control path.",
)

replace_once(
    "## 6. Generation semantics",
    "### Current exact-capture preparation authority (item #7)\n\nThe Pi now prepares one immutable capture plan directly from the START base rectangle through `experiments/media-harness-h1/h1_cp2p_capture_geometry.py`. The active desktop dimensions are validation bounds only. The prepared x11grab source is `DISPLAY+X,Y` with `-video_size W×H` from the accepted START base rectangle; no fixed startup-profile rectangle, desktop-sized substitute, crop-to-fit stage, or scale workaround is allowed. The command is evidence/preparation state only until item #8 launches the producer.\n\nCompound START preparation is atomic at the software boundary: if any step after immutable START validation fails, exact suppression (if installed), usable prepared START state, capture state, and prepared-state evidence are rolled back together. The failed generation remains above the item-#5 generation high-water and therefore remains stale/rejected rather than becoming reusable.\n\n## 6. Generation semantics",
)

replace_section(
    "## 7. START processing order on the Pi",
    "## 8. Producer startup state",
    """## 7. START processing order on the Pi

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
    -> prepare exact START-base X,Y,W,H capture command        [item #7 preparation DONE]
    -> start/activate MPEG producer for that generation        [item #8 OPEN]
    -> only then permit MPEG DATA for that generation onto PSTV channel 4
```

Pending suppression becomes active immediately before forwarding the first **new** FramebufferUpdateRequest after START. An older request already outstanding at acceptance may complete unchanged while PS2 presentation remains frozen in `WAIT_FIRST_FRAME`.

Suppression before production remains a hard ordering rule. A producer must never race ahead and emit video while RFB is still allowed to repaint the same area.

The first three Pi preparation phases are one fail-closed transaction. If suppression or capture-plan preparation/evidence fails, usable state for that generation is rolled back together while the generation high-water remains stale. No failed compound preparation may later become producer-eligible.
""",
)

replace_section(
    "## 8. Producer startup state",
    "## 9. Recalibration / retirement semantics the Pi runtime must support",
    """## 8. Producer startup state

For the all-guns CP2P session:

- RFB may be active from session startup;
- PCM audio may be active from session startup;
- MPEG capture/encoding remains dormant after items #5/#6/#7 preparation;
- START supplies the live capture rectangle; startup CONFIG geometry is not a substitute for the user's accepted calibration;
- `h1_cp2p_capture_geometry.py` now prepares the exact FFmpeg x11grab command from that immutable START rectangle;
- item #8 alone will consume that prepared command and launch the producer;
- producer lifecycle must be generation-scoped;
- only one MPEG generation is active at a time for the current milestone.

The current experimental runner `h1_mux_server_cp2p_start_receiver.py` now owns the ordered pre-producer transaction while preserving the same sole-reader call chain. It records suppression/capture evidence with the producer explicitly marked dormant. The permanent comprehensive Pi runtime/bootstrap must absorb these proven semantics rather than install the experiment as an unrelated second daemon.

No new Pi service/socket/thread/package was introduced by suppression itself. The capture plan makes the `ffmpeg` executable dependency concrete, but item #8 has not yet launched or qualified it as a live producer.
""",
)

replace_section(
    "## 13. Required reconstruction/install tests",
    "## 14. Promotion inventory for the comprehensive Pi setup",
    """## 13. Required reconstruction/install tests

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
- the producer remains dormant during #5/#6/#7 preparation;
- a post-suppression capture-preparation failure rolls back suppression/prepared/capture usable state while preserving stale-generation high-water (`h1_cp2p_start_preparation_transaction_test.py`);
- duplicate/stale generation input cannot create two active producers;
- retirement stops/drains the exact active generation once the retirement wire seam is defined;
- RFB and PCM ownership paths remain alive while MPEG is active;
- only the existing PSTV writer emits PS2-facing frames.

A fresh-Pi reconstruction is not complete merely because source files were copied. The bootstrap must reproduce the runtime dependencies and system configuration those files require.
""",
)

replace_section(
    "## 14. Promotion inventory for the comprehensive Pi setup",
    "## 15. Source map for reconstruction",
    """## 14. Promotion inventory for the comprehensive Pi setup

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
| compound START preparation | `h1_mux_server_cp2p_start_receiver.py`; `h1_cp2p_start_preparation_transaction_test.py` | preserve ordered START -> suppression -> exact capture preparation and fail-closed rollback before producer activation |
| MPEG producer | item #8 still open | launch the already-prepared exact capture command only after #11A retirement control exists; no independent geometry authority |
| MPEG mux scheduling | implementation still to be made concrete | channel-4 queue/credit/scheduling support without second socket |
| retirement control | wire/control seam still to be made concrete | exact-generation stop/drain/remove-suppression/release-capture path with completion semantics before fresh full-RFB restoration |
| PCM capture | current mux uses PipeWire `wpctl` + `pw-record` | install/configure audio capture prerequisites |
| RFB provider | current mux expects isolated local VNC provider | install/configure VNC provider under the general desktop/VNC setup |
| service management | not finalized by this experiment | comprehensive service startup/restart/dependency ordering |
| verification | items #5/#6/#7 host tests; proof runs `34702828032`, `34709167702`, final atomic proof `34710050027` | include START, same-reader, dynamic suppression, exact capture, and compound rollback contracts in fresh-install self-check plus project regressions |

When a row moves from future to concrete implementation, update this table in the same tranche.
""",
)

replace_section(
    "## 15. Source map for reconstruction",
    "## 16. Current implementation boundary",
    """## 15. Source map for reconstruction

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
- compound START preparation rollback contract: `experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py`
- Pi START implementation/proof note: `experiments/media-harness-h1/CP2P_PI_START_RECEIVE_VALIDATION.md`
- PS2 CP2P session coordinator: `experiments/media-harness-h1/h1_cp2p_session_coordinator.[ch]`
- generation presentation owner: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.[ch]`
- recalibration lifecycle: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_recalibration.[ch]`
- combined RFB/MPEG flow policy: `experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.[ch]`
- chronological evidence/history: `experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`
""",
)

replace_section(
    "## 16. Current implementation boundary",
    None,
    """## 16. Current implementation boundary

At this document's current revision:

- PS2 accepted -> fresh generation -> worker arm -> START(session_id, generation, exact geometry, suppression) is implemented and host/PS2-compile proven;
- START+SELECT remains explicitly only a replaceable test trigger;
- Pi START receive/validation and immutable prepared-generation state are implemented and host-proven by item #5 without adding a second socket, reader, receive thread, producer, or package dependency;
- item #6 generation-scoped Pi RFB suppression is concrete and host-proven: pending suppression activates on the first new update request, Raw updates are clipped around the exact suppression footprint, and the transaction budget derives from the current desktop dimensions rather than a hard-coded resolution;
- item #7 exact capture preparation is concrete and host-proven: START base X/Y/W/H becomes the prepared FFmpeg x11grab source while current desktop dimensions are validation bounds only;
- compound START preparation is atomic at the current software boundary: failed post-suppression capture setup removes usable suppression/prepared/capture state and evidence while preserving stale generation high-water;
- item #6 is DONE at the current host/software boundary; item #7 remains PARTIAL only because the prepared exact capture plan is not yet consumed by a live producer;
- item #8 producer activation remains deliberately dormant;
- item #11A exact-generation Pi retirement control remains the next architectural prerequisite before item #8; local `retire_suppression_exact()` is only a primitive, not the completed cross-machine control seam;
- the permanent comprehensive Pi bootstrap has not yet absorbed these experimental CP2P additions;
- item #6/#7 final host proof authority is branch `experiment/h1-cp2p-pi-suppression-geometry`, source head `81d150988e5bc0435c580465b3a826429aa7c139`, GitHub Actions run `34710050027` / job `103597045614`;
- physical all-guns hardware qualification has not yet occurred.

Update this file whenever a Pi-side mechanism, dependency, path, service requirement, or configuration step becomes concrete. Preserve the experiment history separately rather than rewriting historical entries.
""",
)

path.write_text(text)

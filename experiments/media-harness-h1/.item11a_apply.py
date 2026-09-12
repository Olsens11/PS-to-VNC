#!/usr/bin/env python3
"""One-shot, exact-source patch for CP2P item #11A retirement control."""

from pathlib import Path

ROOT = Path("experiments/media-harness-h1")


def read(rel: str) -> str:
    return (ROOT / rel).read_text()


def write(rel: str, text: str) -> None:
    (ROOT / rel).write_text(text.rstrip() + "\n")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one match, got {count}")
    return text.replace(old, new, 1)


def replace_between(text: str, start: str, end: str, replacement: str, label: str) -> str:
    i = text.find(start)
    if i < 0:
        raise SystemExit(f"{label}: start marker missing")
    j = text.find(end, i + len(start))
    if j < 0:
        raise SystemExit(f"{label}: end marker missing")
    if text.find(start, i + 1) >= 0:
        raise SystemExit(f"{label}: duplicate start marker")
    return text[:i] + replacement.rstrip() + "\n\n" + text[j:]


# ---------------------------------------------------------------------------
# PS2 transport: one control-frame request/completion transaction.
# ---------------------------------------------------------------------------
hdr = read("h1_transport_runtime.h")
hdr = replace_once(
    hdr,
    "#define PSTVNC_H1_FRAME_MEDIA_END 8u\n#define PSTVNC_H1_FRAME_SESSION_RESULT 9u\n",
    "#define PSTVNC_H1_FRAME_MEDIA_END 8u\n#define PSTVNC_H1_FRAME_SESSION_RESULT 9u\n#define PSTVNC_H1_FRAME_MPEG_RETIRE 10u\n\n#define PSTVNC_H1_MPEG_RETIRE_VERSION 1u\n#define PSTVNC_H1_MPEG_RETIRE_WORDS 3u\n#define PSTVNC_H1_MPEG_RETIRE_BYTES (PSTVNC_H1_MPEG_RETIRE_WORDS * 4u)\n",
    "transport retire frame constants",
)
hdr = replace_once(
    hdr,
    "    PSTVNC_H1_ERROR_END_METADATA = 17,\n    PSTVNC_H1_ERROR_OVERFLOW = 18\n",
    "    PSTVNC_H1_ERROR_END_METADATA = 17,\n    PSTVNC_H1_ERROR_OVERFLOW = 18,\n    PSTVNC_H1_ERROR_MPEG_RETIRE = 19\n",
    "transport retire error",
)
hdr = replace_once(
    hdr,
    "    volatile uint32_t rfb_quiesce_commit_received;\n    volatile uint32_t rfb_quiesce_complete_sent;\n\n    pstvnc_h1_config_t config;\n",
    "    volatile uint32_t rfb_quiesce_commit_received;\n    volatile uint32_t rfb_quiesce_complete_sent;\n\n    /* Exact CP2P Pi-retirement request/ack state; receiver thread owns ACK. */\n    volatile uint32_t mpeg_retire_pending_generation;\n    volatile uint32_t mpeg_retire_ack_generation;\n\n    pstvnc_h1_config_t config;\n",
    "transport retire runtime state",
)
hdr = replace_once(
    hdr,
    "int pstvnc_h1_transport_audio_exhausted(\n    pstvnc_h1_transport_runtime_t *runtime);\n",
    "int pstvnc_h1_transport_mpeg_retire_begin(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\n/* 1=exact completion consumed, 0=still pending, -1=invalid/failed transport. */\nint pstvnc_h1_transport_mpeg_retire_poll(\n    pstvnc_h1_transport_runtime_t *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_audio_exhausted(\n    pstvnc_h1_transport_runtime_t *runtime);\n",
    "transport retire public API",
)
write("h1_transport_runtime.h", hdr)

src = read("h1_transport_runtime.c")
retire_accept = r'''static int h1_accept_mpeg_retire(
    pstvnc_h1_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    uint32_t version;
    uint32_t session_id;
    uint32_t generation;

    if (runtime == NULL || header == NULL ||
        header->channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header->flags != 0u ||
        header->payload_length != PSTVNC_H1_MPEG_RETIRE_BYTES) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
        return 0;
    }

    version = pstvnc_transport_read_be32(&runtime->receiver_payload[0]);
    session_id = pstvnc_transport_read_be32(&runtime->receiver_payload[4]);
    generation = pstvnc_transport_read_be32(&runtime->receiver_payload[8]);

    if (version != PSTVNC_H1_MPEG_RETIRE_VERSION ||
        session_id != runtime->config.session_id ||
        generation == 0u ||
        runtime->mpeg_retire_pending_generation != generation ||
        runtime->mpeg_retire_ack_generation != 0u) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
        return 0;
    }

    runtime->mpeg_retire_ack_generation = generation;
    return 1;
}'''
src = replace_once(
    src,
    "static int h1_accept_frame(\n",
    retire_accept + "\n\nstatic int h1_accept_frame(\n",
    "transport retire ack receiver",
)
src = replace_once(
    src,
    "        case PSTVNC_H1_FRAME_MEDIA_END:\n            return h1_accept_end(runtime, header);\n\n        default:\n",
    "        case PSTVNC_H1_FRAME_MEDIA_END:\n            return h1_accept_end(runtime, header);\n\n        case PSTVNC_H1_FRAME_MPEG_RETIRE:\n            return h1_accept_mpeg_retire(runtime, header);\n\n        default:\n",
    "transport retire switch",
)
retire_api = r'''int pstvnc_h1_transport_mpeg_retire_begin(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    uint8_t payload[PSTVNC_H1_MPEG_RETIRE_BYTES];

    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->stop_requested || runtime->receiver_done ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u)
        return 0;

    pstvnc_transport_write_be32(
        &payload[0], PSTVNC_H1_MPEG_RETIRE_VERSION);
    pstvnc_transport_write_be32(
        &payload[4], runtime->config.session_id);
    pstvnc_transport_write_be32(&payload[8], generation);

    /* Arm exact completion matching before the request can reach the Pi. */
    runtime->mpeg_retire_pending_generation = generation;
    runtime->mpeg_retire_ack_generation = 0u;

    if (!h1_send_frame(
            runtime,
            PSTVNC_H1_FRAME_MPEG_RETIRE,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            payload,
            sizeof(payload))) {
        runtime->mpeg_retire_pending_generation = 0u;
        runtime->mpeg_retire_ack_generation = 0u;
        return 0;
    }

    return 1;
}

int pstvnc_h1_transport_mpeg_retire_poll(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        runtime->mpeg_retire_pending_generation != generation)
        return -1;

    if (runtime->mpeg_retire_ack_generation == generation) {
        runtime->mpeg_retire_ack_generation = 0u;
        runtime->mpeg_retire_pending_generation = 0u;
        return 1;
    }

    if (runtime->mpeg_retire_ack_generation != 0u ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->stop_requested || runtime->receiver_done)
        return -1;

    return 0;
}'''
src = replace_once(
    src,
    "int pstvnc_h1_transport_start(\n",
    retire_api + "\n\nint pstvnc_h1_transport_start(\n",
    "transport retire begin/poll API",
)
write("h1_transport_runtime.c", src)


# ---------------------------------------------------------------------------
# PS2 session coordinator: Pi completion precedes local clear/restoration.
# ---------------------------------------------------------------------------
ch = read("h1_cp2p_session_coordinator.h")
ch = replace_once(
    ch,
    "    pstvnc_h1_mpeg_start_contract_t current_start_contract;\n    uint32_t start_messages_sent;\n    unsigned current_start_contract_valid : 1;\n    unsigned initialized : 1;\n",
    "    pstvnc_h1_mpeg_start_contract_t current_start_contract;\n    uint32_t start_messages_sent;\n    uint32_t pi_retire_generation;\n    unsigned current_start_contract_valid : 1;\n    unsigned pi_retire_pending : 1;\n    unsigned initialized : 1;\n",
    "coordinator Pi retire state",
)
write("h1_cp2p_session_coordinator.h", ch)

cc = read("h1_cp2p_session_coordinator.c")
cc = replace_once(
    cc,
    "const pstvnc_h1_config_t *pstvnc_h1_transport_config(\n    const struct pstvnc_h1_transport_runtime *runtime);\n",
    "const pstvnc_h1_config_t *pstvnc_h1_transport_config(\n    const struct pstvnc_h1_transport_runtime *runtime);\n\nint pstvnc_h1_transport_mpeg_retire_begin(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n\nint pstvnc_h1_transport_mpeg_retire_poll(\n    struct pstvnc_h1_transport_runtime *runtime,\n    uint32_t generation);\n",
    "coordinator transport retire declarations",
)
new_gate = r'''static int h1_cp2p_session_calibration_entry_gate(
    void *context,
    int *enter_now)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;
    pstvnc_mpeg_calibration_t *calibration;
    uint32_t generation;
    int retire_poll;

    if (coordinator == NULL || !coordinator->initialized ||
        enter_now == NULL)
        return 0;

    if (coordinator->recalibration.awaiting_restored_full_rfb) {
        *enter_now = pstvnc_h1_mpeg_recalibration_take_entry_ready(
            &coordinator->recalibration,
            &coordinator->rfb_flow)
            ? 1
            : 0;
        return 1;
    }

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);

    if (coordinator->pi_retire_pending) {
        retire_poll = pstvnc_h1_transport_mpeg_retire_poll(
            coordinator->transport,
            coordinator->pi_retire_generation);
        if (retire_poll < 0)
            return 0;
        if (retire_poll == 0) {
            *enter_now = 0;
            return 1;
        }

        coordinator->pi_retire_pending = 0;
        coordinator->pi_retire_generation = 0u;
    } else if (owner_state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY) {
        generation = coordinator->mpeg_handoff.owner.generation;
        if (generation == 0u ||
            !pstvnc_h1_transport_mpeg_retire_begin(
                coordinator->transport, generation))
            return 0;

        coordinator->pi_retire_generation = generation;
        coordinator->pi_retire_pending = 1;
        *enter_now = 0;
        return 1;
    }

    /*
     * Pi completion is now proven for the exact old generation. Only here may
     * local worker/presentation state retire and the one-full-RFB restoration
     * obligation become visible to the request scheduler.
     */
    calibration = &coordinator->interaction.mpeg_calibration.runtime
        .foreground.adapter.calibration;

    if (!pstvnc_h1_mpeg_recalibration_begin(
            &coordinator->recalibration,
            &coordinator->mpeg_handoff,
            calibration,
            &coordinator->rfb_flow,
            coordinator->clear_mpeg,
            coordinator->clear_mpeg_context,
            &begin_result))
        return 0;

    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW) {
        *enter_now = 1;
        return 1;
    }

    if (begin_result != PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB)
        return 0;

    coordinator->current_start_contract_valid = 0;
    *enter_now = 0;
    return 1;
}'''
cc = replace_between(
    cc,
    "static int h1_cp2p_session_calibration_entry_gate(\n",
    "static int h1_cp2p_session_start_accepted_calibration(\n",
    new_gate,
    "coordinator deferred Pi retirement gate",
)
cc = replace_once(
    cc,
    "    coordinator->arm_mpeg = NULL;\n    coordinator->arm_mpeg_context = NULL;\n    coordinator->current_start_contract_valid = 0;\n",
    "    coordinator->arm_mpeg = NULL;\n    coordinator->arm_mpeg_context = NULL;\n    coordinator->pi_retire_generation = 0u;\n    coordinator->pi_retire_pending = 0;\n    coordinator->current_start_contract_valid = 0;\n",
    "coordinator shutdown retire reset",
)
write("h1_cp2p_session_coordinator.c", cc)


# ---------------------------------------------------------------------------
# Pi pure retire codec/control vocabulary.
# ---------------------------------------------------------------------------
write(
    "h1_cp2p_retirement_control.py",
    r'''#!/usr/bin/env python3
"""
File synopsis:
    Defines CP2P item #11A's exact-generation retirement control payload.

RETIRE is one H1 experiment-local PSTV frame kind on control channel 0. The
payload is exactly three big-endian 32-bit words:

    version, session_id, generation

The PS2 sends this payload as a request. The Pi sends the identical payload back
only after exact-generation runtime cleanup succeeds. Direction plus each side's
strict pending-generation state distinguish request from completion; MPEG2
channel-4 DATA remains pure media.
"""

from __future__ import annotations

from dataclasses import dataclass
import struct

import h1_mux_server as base

MPEG_RETIRE_FRAME_KIND = 10
MPEG_RETIRE_WIRE_VERSION = 1
MPEG_RETIRE_WIRE_WORDS = 3
MPEG_RETIRE_WIRE_BYTES = MPEG_RETIRE_WIRE_WORDS * 4


@dataclass(frozen=True)
class H1Cp2pRetireControl:
    session_id: int
    generation: int


def decode_mpeg_retire_payload(
    payload: bytes,
    *,
    expected_session_id: int,
) -> H1Cp2pRetireControl:
    if len(payload) != MPEG_RETIRE_WIRE_BYTES:
        raise base.ProtocolError(
            f"MPEG RETIRE length {len(payload)} != {MPEG_RETIRE_WIRE_BYTES}"
        )

    version, session_id, generation = struct.unpack(">3I", payload)
    if version != MPEG_RETIRE_WIRE_VERSION:
        raise base.ProtocolError(f"unsupported MPEG RETIRE version {version}")
    if session_id != int(expected_session_id):
        raise base.ProtocolError(
            "MPEG RETIRE session mismatch "
            f"actual={session_id} expected={int(expected_session_id)}"
        )
    if generation == 0:
        raise base.ProtocolError("MPEG RETIRE generation must be nonzero")

    return H1Cp2pRetireControl(
        session_id=int(session_id),
        generation=int(generation),
    )


def encode_mpeg_retire_payload(session_id: int, generation: int) -> bytes:
    session_id = int(session_id)
    generation = int(generation)
    if session_id < 0 or session_id > 0xFFFFFFFF:
        raise base.ProtocolError("MPEG RETIRE session id is outside uint32 range")
    if generation <= 0 or generation > 0xFFFFFFFF:
        raise base.ProtocolError("MPEG RETIRE generation must be nonzero uint32")
    return struct.pack(
        ">3I",
        MPEG_RETIRE_WIRE_VERSION,
        session_id,
        generation,
    )
''',
)


# ---------------------------------------------------------------------------
# Pi same-reader START shim becomes START+RETIRE control shim.
# ---------------------------------------------------------------------------
pr = read("h1_cp2p_start_receiver.py")
pr = replace_once(
    pr,
    "import h1_mux_server as base\n",
    "import h1_mux_server as base\nfrom h1_cp2p_retirement_control import (\n    MPEG_RETIRE_FRAME_KIND,\n    decode_mpeg_retire_payload,\n    encode_mpeg_retire_payload,\n)\n",
    "Pi retirement imports",
)
insert_methods = r'''    def retire_generation_exact(self, generation: int) -> H1Cp2pPreparedStart:
        """Release the exact prepared generation; subclasses extend runtime cleanup."""

        request = self.release_prepared_exact(generation)
        (Path(self.session.evidence) / "mpeg_start_prepared.json").unlink(
            missing_ok=True
        )
        return request

    def _handle_retire_frame(self, frame: base.Frame) -> None:
        if (
            frame.kind != MPEG_RETIRE_FRAME_KIND
            or frame.channel != base.CHANNEL_CONTROL
        ):
            raise base.ProtocolError("CP2P RETIRE receiver received a non-RETIRE frame")
        if frame.flags != 0:
            raise base.ProtocolError("MPEG RETIRE flags must be zero")

        control = decode_mpeg_retire_payload(
            frame.payload,
            expected_session_id=int(self.session.profile["session_id"]),
        )

        # ACK is deliberately after exact runtime cleanup. Failure leaves the
        # PS2 deferred and never falsely authorizes full-RFB restoration.
        self.retire_generation_exact(control.generation)

        evidence = {
            "session_id": int(control.session_id),
            "generation": int(control.generation),
            "state": "retired-complete",
        }
        (Path(self.session.evidence) / "mpeg_retire_completed.json").write_text(
            json.dumps(evidence, indent=2, sort_keys=True) + "\n"
        )

        self.session.send_frame(
            MPEG_RETIRE_FRAME_KIND,
            base.CHANNEL_CONTROL,
            encode_mpeg_retire_payload(
                control.session_id,
                control.generation,
            ),
        )
        print(
            "H1_CP2P_MPEG_RETIRE_COMPLETE="
            + json.dumps(evidence, sort_keys=True),
            flush=True,
        )'''
pr = replace_once(
    pr,
    "    def start(self) -> None:\n",
    insert_methods + "\n\n    def start(self) -> None:\n",
    "Pi retirement handler methods",
)
old_dispatch = r'''        if (
            receiver is None
            or frame.kind != base.FRAME_DATA
            or frame.channel != base.CHANNEL_MPEG2
        ):
            return frame

        session = receiver.session
        if frame.sequence != session.expected_rx_sequence:
            raise base.ProtocolError(
                "PS2 sequence mismatch "
                f"expected={session.expected_rx_sequence} actual={frame.sequence}"
            )
        session.expected_rx_sequence += 1

        receiver._handle_start_frame(frame)
        # START is complete. Stay in this same H1 reader call and receive the next
        # physical PSTV frame for the pre-existing dispatch chain/base reader.
'''
new_dispatch = r'''        is_start = (
            frame.kind == base.FRAME_DATA
            and frame.channel == base.CHANNEL_MPEG2
        )
        is_retire = (
            frame.kind == MPEG_RETIRE_FRAME_KIND
            and frame.channel == base.CHANNEL_CONTROL
        )
        if receiver is None or not (is_start or is_retire):
            return frame

        session = receiver.session
        if frame.sequence != session.expected_rx_sequence:
            raise base.ProtocolError(
                "PS2 sequence mismatch "
                f"expected={session.expected_rx_sequence} actual={frame.sequence}"
            )
        session.expected_rx_sequence += 1

        if is_start:
            receiver._handle_start_frame(frame)
        else:
            receiver._handle_retire_frame(frame)
        # CP2P control is complete. Stay in this same H1 reader call and receive
        # the next physical PSTV frame for the pre-existing dispatch/base reader.
'''
pr = replace_once(pr, old_dispatch, new_dispatch, "Pi same-reader retire dispatch")
write("h1_cp2p_start_receiver.py", pr)

runner = read("h1_mux_server_cp2p_start_receiver.py")
retire_override = r'''    def retire_generation_exact(self, generation: int):
        """Retire one exact prepared Pi generation before acknowledging the PS2."""

        generation = int(generation)
        request = self.peek_prepared()
        if request is None or request.generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE prepared-generation mismatch "
                f"prepared={getattr(request, 'generation', 0)} requested={generation}"
            )
        if self.capture_plan is None or self.capture_plan.generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE exact capture generation is not prepared"
            )
        if self.suppression_bridge.suppression_generation != generation:
            raise base.ProtocolError(
                "CP2P RETIRE suppression generation mismatch "
                f"active={self.suppression_bridge.suppression_generation} "
                f"requested={generation}"
            )

        # Item #8 will replace this dormant-producer guard with an exact
        # stop/drain operation before suppression is removed. Until then, never
        # acknowledge retirement over an unexpected live legacy producer.
        if getattr(self.session, "video_producer", None) is not None:
            raise base.ProtocolError(
                "CP2P RETIRE cannot acknowledge while MPEG producer is live"
            )

        self.suppression_bridge.retire_suppression_exact(generation)
        released = super().retire_generation_exact(generation)
        self.capture_plan = None

        for name in (
            "rfb_suppression_prepared.json",
            "mpeg_capture_prepared.json",
        ):
            (Path(self.session.evidence) / name).unlink(missing_ok=True)

        return released'''
runner = replace_once(
    runner,
    "    def _handle_start_frame(self, frame: base.Frame) -> None:\n",
    retire_override + "\n\n    def _handle_start_frame(self, frame: base.Frame) -> None:\n",
    "Pi exact runtime retirement",
)
write("h1_mux_server_cp2p_start_receiver.py", runner)


# ---------------------------------------------------------------------------
# Pi host contract for exact retirement and ACK ordering.
# ---------------------------------------------------------------------------
write(
    "h1_cp2p_retirement_control_test.py",
    r'''#!/usr/bin/env python3
"""Host contract for CP2P item #11A Pi exact-generation retirement control."""

from __future__ import annotations

from pathlib import Path
import socket
import struct
import tempfile
import unittest

import h1_mux_server as base
from h1_cp2p_retirement_control import (
    MPEG_RETIRE_FRAME_KIND,
    decode_mpeg_retire_payload,
    encode_mpeg_retire_payload,
)
from h1_cp2p_rfb_suppression import H1Cp2pRfbPiBridge
from h1_mux_server_cp2p_start_receiver import H1Cp2pSuppressionStartReceiver


class FakeSession:
    def __init__(self, evidence: Path) -> None:
        self.profile = {
            "session_id": 0x12345678,
            "desktop_width": 640,
            "desktop_height": 448,
        }
        self.evidence = evidence
        self.display = ":0"
        self.video_producer = None
        self.sent: list[tuple[int, int, bytes, int]] = []

    def send_frame(self, kind: int, channel: int, payload: bytes = b"", flags: int = 0) -> int:
        self.sent.append((kind, channel, bytes(payload), flags))
        return len(self.sent)


def make_start_frame(session_id: int, generation: int) -> base.Frame:
    x, y, width, height = 32, 48, 320, 240
    sx, sy = x - 1, y - 1
    sw, sh = width + 3, height + 2
    payload = struct.pack(
        ">11I",
        1,
        session_id,
        generation,
        x,
        y,
        width,
        height,
        sx,
        sy,
        sw,
        sh,
    )
    return base.Frame(base.FRAME_DATA, base.CHANNEL_MPEG2, 0, 0, payload)


def make_retire_frame(session_id: int, generation: int) -> base.Frame:
    return base.Frame(
        MPEG_RETIRE_FRAME_KIND,
        base.CHANNEL_CONTROL,
        0,
        0,
        encode_mpeg_retire_payload(session_id, generation),
    )


class RetirementControlTests(unittest.TestCase):
    def test_codec_rejects_wrong_session_and_zero_generation(self) -> None:
        payload = encode_mpeg_retire_payload(7, 9)
        decoded = decode_mpeg_retire_payload(payload, expected_session_id=7)
        self.assertEqual(decoded.session_id, 7)
        self.assertEqual(decoded.generation, 9)
        with self.assertRaisesRegex(base.ProtocolError, "session mismatch"):
            decode_mpeg_retire_payload(payload, expected_session_id=8)
        with self.assertRaisesRegex(base.ProtocolError, "generation must be nonzero"):
            decode_mpeg_retire_payload(
                struct.pack(">3I", 1, 7, 0),
                expected_session_id=7,
            )

    def _prepared_receiver(self, temp_dir: str):
        bridge_sock, peer = socket.socketpair()
        bridge = H1Cp2pRfbPiBridge(
            bridge_sock,
            queue_capacity=1024 * 1024,
            max_payload=8192,
            send_data=lambda payload: None,
            desktop_width=640,
            desktop_height=448,
        )
        session = FakeSession(Path(temp_dir))
        receiver = H1Cp2pSuppressionStartReceiver(session, bridge)
        receiver._handle_start_frame(
            make_start_frame(session.profile["session_id"], 7)
        )
        return session, receiver, bridge, peer

    def test_exact_retire_clears_runtime_state_before_ack_and_keeps_generation_stale(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())

                receiver._handle_retire_frame(
                    make_retire_frame(session.profile["session_id"], 7)
                )

                self.assertEqual(bridge.suppression_generation, 0)
                self.assertIsNone(receiver.capture_plan)
                self.assertIsNone(receiver.peek_prepared())
                self.assertEqual(len(session.sent), 1)
                kind, channel, payload, flags = session.sent[0]
                self.assertEqual(kind, MPEG_RETIRE_FRAME_KIND)
                self.assertEqual(channel, base.CHANNEL_CONTROL)
                self.assertEqual(flags, 0)
                ack = decode_mpeg_retire_payload(
                    payload,
                    expected_session_id=session.profile["session_id"],
                )
                self.assertEqual(ack.generation, 7)
                self.assertTrue(
                    (Path(temp_dir) / "mpeg_retire_completed.json").exists()
                )

                with self.assertRaisesRegex(base.ProtocolError, "stale MPEG START generation"):
                    receiver._handle_start_frame(
                        make_start_frame(session.profile["session_id"], 7)
                    )
            finally:
                bridge.stop()
                peer.close()

    def test_wrong_generation_never_cleans_or_acknowledges(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                with self.assertRaisesRegex(base.ProtocolError, "prepared-generation mismatch"):
                    receiver._handle_retire_frame(
                        make_retire_frame(session.profile["session_id"], 8)
                    )
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())
                self.assertEqual(session.sent, [])
            finally:
                bridge.stop()
                peer.close()

    def test_unexpected_live_producer_blocks_ack_and_preserves_generation(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            session, receiver, bridge, peer = self._prepared_receiver(temp_dir)
            try:
                session.video_producer = object()
                with self.assertRaisesRegex(base.ProtocolError, "producer is live"):
                    receiver._handle_retire_frame(
                        make_retire_frame(session.profile["session_id"], 7)
                    )
                self.assertEqual(bridge.suppression_generation, 7)
                self.assertIsNotNone(receiver.capture_plan)
                self.assertIsNotNone(receiver.peek_prepared())
                self.assertEqual(session.sent, [])
            finally:
                session.video_producer = None
                bridge.stop()
                peer.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)
''',
)


# ---------------------------------------------------------------------------
# Coordinator host contract: RETIRE completion gates local retirement/RFB full.
# ---------------------------------------------------------------------------
test = read("mpeg_presentation_calibration/h1_cp2p_session_coordinator_test.c")
test = replace_once(
    test,
    "static unsigned int clear_count;\nstatic uint32_t cleared_generation;\nstatic pstvnc_h1_config_t fake_config;\n",
    "static unsigned int clear_count;\nstatic uint32_t cleared_generation;\nstatic unsigned int retire_begin_count;\nstatic uint32_t retire_generation;\nstatic int retire_poll_result;\nstatic pstvnc_h1_config_t fake_config;\n",
    "coordinator test retire globals",
)
stubs = r'''int pstvnc_h1_transport_mpeg_retire_begin(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || retire_begin_count != 0u)
        return 0;
    retire_begin_count++;
    retire_generation = generation;
    return 1;
}

int pstvnc_h1_transport_mpeg_retire_poll(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || generation != retire_generation)
        return -1;
    return retire_poll_result;
}'''
test = replace_once(
    test,
    "static int clear_mpeg(void *context, uint32_t generation)\n",
    stubs + "\n\nstatic int clear_mpeg(void *context, uint32_t generation)\n",
    "coordinator test transport retire stubs",
)
old = r'''    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(clear_count == 1u);
    assert(cleared_generation == active_contract.generation);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(!coordinator.current_start_contract_valid);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.has_committed);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.x == region.x);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.outer_matte_x == region.outer_matte_x);
'''
new = r'''    /* First gate pass requests exact Pi retirement only. */
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(retire_begin_count == 1u);
    assert(retire_generation == active_contract.generation);
    assert(coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == active_contract.generation);
    assert(clear_count == 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);
    assert(coordinator.current_start_contract_valid);

    /* Deferred polling cannot clear local ownership before exact Pi ACK. */
    retire_poll_result = 0;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(retire_begin_count == 1u);
    assert(clear_count == 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);

    /* Exact Pi completion unlocks the existing local retire/restoration path. */
    retire_poll_result = 1;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(!coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == 0u);
    assert(clear_count == 1u);
    assert(cleared_generation == active_contract.generation);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(!coordinator.current_start_contract_valid);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.has_committed);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.x == region.x);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.outer_matte_x == region.outer_matte_x);
'''
test = replace_once(test, old, new, "coordinator retire-before-restoration contract")
write("mpeg_presentation_calibration/h1_cp2p_session_coordinator_test.c", test)

print("ITEM11A_PATCH=PASS")

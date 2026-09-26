#!/usr/bin/env python3
"""Deterministic R39 proof for optional AUDIO ownership inside WireServer."""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace
import socket
import sys
import threading
import unittest

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import wire_protocol as protocol
import wire_runtime
import wire_server


def read_exact(peer: socket.socket, byte_count: int) -> bytes:
    data = bytearray()
    while len(data) < byte_count:
        chunk = peer.recv(byte_count - len(data))
        if not chunk:
            raise EOFError("test Wire peer closed")
        data.extend(chunk)
    return bytes(data)


def read_frame(peer: socket.socket):
    header = protocol.decode_header(read_exact(peer, protocol.HEADER_BYTES))
    payload = read_exact(peer, header.payload_length)
    return header, payload


def serve_once(server):
    product, peer = socket.socketpair()
    peer.settimeout(2.0)
    result: dict[str, object] = {}

    def target() -> None:
        result["outcome"] = server.serve_connection(product)

    thread = threading.Thread(target=target, daemon=True)
    thread.start()
    return peer, thread, result


def establish(peer: socket.socket) -> int:
    peer.sendall(protocol.encode_hello_frame())
    header, payload = read_frame(peer)
    if not protocol.is_accept_header(header):
        raise AssertionError(f"expected ACCEPT, got {header}")
    return protocol.decode_accept_payload(payload)


def finish(peer, thread, result):
    try:
        peer.shutdown(socket.SHUT_WR)
    except OSError:
        pass
    thread.join(timeout=2.0)
    if thread.is_alive():
        raise AssertionError("WireServer session did not finish")
    try:
        peer.close()
    except OSError:
        pass
    return result["outcome"]


class FakeAudioOwner:
    def __init__(
        self,
        session_id: int,
        *,
        payload: bytes = b"abcd",
        close_result: bool = True,
    ) -> None:
        self.session_id = session_id
        self.profile = SimpleNamespace(frame_bytes=4)
        self.credit_bytes = 0
        self.payload = payload
        self.close_result = close_result
        self.closed = False
        self._wake_reader, self._wake_writer = socket.socketpair()
        self._wake_reader.setblocking(False)
        self._wake_writer.setblocking(False)

    @property
    def activity_reader(self):
        return self._wake_reader

    def _wake(self) -> None:
        try:
            self._wake_writer.send(b"X")
        except OSError:
            pass

    def acknowledge_activity(self) -> None:
        try:
            while self._wake_reader.recv(64):
                pass
        except BlockingIOError:
            pass

    def check_health(self) -> None:
        if self.closed:
            raise RuntimeError("fake AUDIO owner closed")

    def add_credit(self, amount: int) -> None:
        self.credit_bytes += amount
        self._wake()

    def begin_emission(self, maximum: int):
        if (
            not self.payload
            or self.credit_bytes < len(self.payload)
            or maximum < len(self.payload)
        ):
            return None
        payload = self.payload
        self.payload = b""
        self.credit_bytes -= len(payload)
        return payload

    def close(self) -> bool:
        if not self.closed:
            self.closed = True
            self._wake_reader.close()
            self._wake_writer.close()
        return self.close_result


class FakeRfbAttachment:
    def __init__(self) -> None:
        self._initial_credit = 8
        self.closed = False
        self.connecting_socket = None
        self.provider_socket = None
        self.quiesce_wake_reader = None
        self.wants_provider_read = False
        self.wants_provider_write = False
        self.wants_request_marker = False
        self.wants_commit_marker = False
        self.wants_provider_failure_report = False

    def take_initial_ps2_credit(self) -> int:
        amount = self._initial_credit
        self._initial_credit = 0
        return amount

    def add_ps2_credit(self, _amount: int) -> None:
        pass

    def close(self) -> None:
        self.closed = True


class FakeMpegOwner:
    def __init__(self, session_id: int) -> None:
        self.session_id = session_id
        self.credit = 0
        self.payload = b"MPEG"
        self.closed = False
        self._wake_reader, self._wake_writer = socket.socketpair()
        self._wake_reader.setblocking(False)
        self._wake_writer.setblocking(False)

    @property
    def activity_reader(self):
        return self._wake_reader

    def _wake(self) -> None:
        try:
            self._wake_writer.send(b"M")
        except OSError:
            pass

    def acknowledge_activity(self) -> None:
        try:
            while self._wake_reader.recv(64):
                pass
        except BlockingIOError:
            pass

    def add_credit(self, amount: int) -> None:
        self.credit += amount
        self._wake()

    def begin_emission(self, maximum: int):
        if not self.payload or self.credit < len(self.payload):
            return None
        payload = self.payload[:maximum]
        self.payload = self.payload[len(payload):]
        self.credit -= len(payload)
        return SimpleNamespace(generation=1, payload=payload)

    def finish_emission(self, _lease) -> None:
        pass

    def start_exact(self, _control) -> None:
        pass

    def retire_exact(self, control):
        return control

    def confirm_retire_completion(self, _control) -> None:
        pass

    def close(self) -> bool:
        if not self.closed:
            self.closed = True
            self._wake_reader.close()
            self._wake_writer.close()
        return True


class AudioWireProtocolTests(unittest.TestCase):
    def test_audio_credit_data_and_done_are_structurally_exact(self) -> None:
        credit = protocol.encode_audio_credit_frame(8, sequence=2)
        credit_header = protocol.decode_header(
            credit[: protocol.HEADER_BYTES]
        )
        self.assertTrue(protocol.is_audio_credit_header(credit_header))
        self.assertEqual(
            protocol.decode_audio_credit_payload(
                credit[protocol.HEADER_BYTES :]
            ),
            8,
        )

        data = protocol.encode_audio_data_frame(b"abcd", sequence=3)
        data_header = protocol.decode_header(data[: protocol.HEADER_BYTES])
        self.assertTrue(protocol.is_audio_data_header(data_header))
        self.assertFalse(protocol.is_audio_producer_done_header(data_header))
        self.assertEqual(data_header.channel, protocol.CHANNEL_AUDIO)

        done = protocol.encode_audio_producer_done_frame(sequence=4)
        done_header = protocol.decode_header(done[: protocol.HEADER_BYTES])
        self.assertTrue(protocol.is_audio_producer_done_header(done_header))
        self.assertFalse(protocol.is_audio_data_header(done_header))
        self.assertEqual(done[protocol.HEADER_BYTES :], b"")

        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_audio_credit_payload(0)
        with self.assertRaises(protocol.WireProtocolError):
            protocol.encode_audio_data_frame(b"", sequence=5)


class AudioWireServerTests(unittest.TestCase):
    def test_audio_credit_without_injected_owner_fails_closed(self) -> None:
        server = wire_server.WireServer()
        peer, thread, result = serve_once(server)
        establish(peer)
        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        thread.join(timeout=2.0)
        self.assertFalse(thread.is_alive())
        self.assertTrue(result["outcome"].accepted)
        self.assertTrue(result["outcome"].protocol_failed)
        peer.close()

    def test_audio_rfb_and_mpeg_share_one_global_send_sequence(self) -> None:
        audio_owners: list[FakeAudioOwner] = []
        mpeg_owners: list[FakeMpegOwner] = []
        attachments: list[FakeRfbAttachment] = []

        def make_audio(session_id: int):
            owner = FakeAudioOwner(session_id)
            audio_owners.append(owner)
            return owner

        def make_mpeg(session_id: int):
            owner = FakeMpegOwner(session_id)
            mpeg_owners.append(owner)
            return owner

        def make_rfb():
            attachment = FakeRfbAttachment()
            attachments.append(attachment)
            return attachment

        server = wire_server.WireServer(
            rfb_attachment_factory=make_rfb,
            mpeg_generation_factory=make_mpeg,
            audio_pcm_factory=make_audio,
        )
        peer, thread, result = serve_once(server)
        session_id = establish(peer)

        rfb_header, rfb_payload = read_frame(peer)
        self.assertTrue(protocol.is_rfb_credit_header(rfb_header))
        self.assertEqual(rfb_header.sequence, 2)
        self.assertEqual(
            protocol.decode_rfb_credit_payload(rfb_payload),
            8,
        )

        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        audio_header, audio_payload = read_frame(peer)
        self.assertTrue(protocol.is_audio_data_header(audio_header))
        self.assertEqual(audio_header.sequence, 3)
        self.assertEqual(audio_payload, b"abcd")

        peer.sendall(protocol.encode_mpeg_credit_frame(4, sequence=3))
        mpeg_header, mpeg_payload = read_frame(peer)
        self.assertTrue(protocol.is_mpeg_data_header(mpeg_header))
        self.assertEqual(mpeg_header.sequence, 4)
        self.assertEqual(mpeg_payload, b"MPEG")

        self.assertEqual(audio_owners[0].session_id, session_id)
        self.assertEqual(mpeg_owners[0].session_id, session_id)

        outcome = finish(peer, thread, result)
        self.assertFalse(outcome.protocol_failed)
        self.assertTrue(audio_owners[0].closed)
        self.assertTrue(mpeg_owners[0].closed)
        self.assertTrue(attachments[0].closed)

    def test_failed_audio_retirement_makes_session_failure(self) -> None:
        owners: list[FakeAudioOwner] = []

        def make_audio(session_id: int):
            owner = FakeAudioOwner(
                session_id,
                payload=b"",
                close_result=False,
            )
            owners.append(owner)
            return owner

        server = wire_server.WireServer(audio_pcm_factory=make_audio)
        peer, thread, result = serve_once(server)
        establish(peer)
        outcome = finish(peer, thread, result)
        self.assertTrue(outcome.accepted)
        self.assertTrue(outcome.protocol_failed)
        self.assertTrue(owners[0].closed)

    def test_each_wire_session_gets_fresh_audio_owner(self) -> None:
        owners: list[FakeAudioOwner] = []

        def make_audio(session_id: int):
            owner = FakeAudioOwner(session_id, payload=b"")
            owners.append(owner)
            return owner

        server = wire_server.WireServer(
            session_ids=wire_server.SessionIdAllocator(70),
            audio_pcm_factory=make_audio,
        )
        peer_a, thread_a, result_a = serve_once(server)
        session_a = establish(peer_a)
        outcome_a = finish(peer_a, thread_a, result_a)
        self.assertFalse(outcome_a.protocol_failed)

        peer_b, thread_b, result_b = serve_once(server)
        session_b = establish(peer_b)
        outcome_b = finish(peer_b, thread_b, result_b)
        self.assertFalse(outcome_b.protocol_failed)

        self.assertEqual((session_a, session_b), (70, 71))
        self.assertEqual(len(owners), 2)
        self.assertIsNot(owners[0], owners[1])
        self.assertEqual(owners[0].session_id, 70)
        self.assertEqual(owners[1].session_id, 71)

    def test_ordinary_product_runtime_still_injects_no_audio_owner(self) -> None:
        server = wire_runtime.build_product_wire_server()
        self.assertIsNone(server.audio_pcm_factory)
        source = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        self.assertNotIn("audio_pcm_factory=", source)
        self.assertNotIn("audio_pcm_producer", source)


if __name__ == "__main__":
    unittest.main(verbosity=2)

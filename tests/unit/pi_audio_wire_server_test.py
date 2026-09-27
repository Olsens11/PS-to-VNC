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
        fail_add_credit: bool = False,
    ) -> None:
        self.session_id = session_id
        self.profile = SimpleNamespace(frame_bytes=4)
        self.credit_bytes = 0
        self.payload = payload
        self.close_result = close_result
        self.fail_add_credit = fail_add_credit
        self.credit_amounts: list[int] = []
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
        self.credit_amounts.append(amount)
        if self.fail_add_credit:
            raise RuntimeError("injected AUDIO credit failure")
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
        self.assertEqual(audio_owners, [])

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

        self.assertEqual(len(audio_owners), 1)
        self.assertEqual(audio_owners[0].session_id, session_id)
        self.assertEqual(audio_owners[0].credit_amounts, [4])
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
        self.assertEqual(owners, [])
        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        outcome = finish(peer, thread, result)
        self.assertTrue(outcome.accepted)
        self.assertTrue(outcome.protocol_failed)
        self.assertTrue(owners[0].closed)

    def test_lazy_owner_is_absent_before_credit_reused_then_fresh_next_session(self) -> None:
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
        self.assertEqual(session_a, 70)
        self.assertEqual(owners, [])
        outcome_a = finish(peer_a, thread_a, result_a)
        self.assertFalse(outcome_a.protocol_failed)
        self.assertEqual(owners, [])

        peer_b, thread_b, result_b = serve_once(server)
        session_b = establish(peer_b)
        self.assertEqual(session_b, 71)
        self.assertEqual(owners, [])
        peer_b.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        peer_b.sendall(protocol.encode_audio_credit_frame(3, sequence=3))
        outcome_b = finish(peer_b, thread_b, result_b)
        self.assertFalse(outcome_b.protocol_failed)
        self.assertEqual(len(owners), 1)
        self.assertEqual(owners[0].session_id, 71)
        self.assertEqual(owners[0].credit_amounts, [4, 3])
        self.assertEqual(owners[0].credit_bytes, 7)

        peer_c, thread_c, result_c = serve_once(server)
        session_c = establish(peer_c)
        self.assertEqual(session_c, 72)
        peer_c.sendall(protocol.encode_audio_credit_frame(2, sequence=2))
        outcome_c = finish(peer_c, thread_c, result_c)
        self.assertFalse(outcome_c.protocol_failed)
        self.assertEqual(len(owners), 2)
        self.assertIsNot(owners[0], owners[1])
        self.assertEqual(owners[1].session_id, 72)
        self.assertEqual(owners[1].credit_amounts, [2])

    def test_lazy_factory_attach_and_credit_failures_are_terminal(self) -> None:
        def fail_factory(_session_id: int):
            raise RuntimeError("injected AUDIO factory failure")

        server = wire_server.WireServer(audio_pcm_factory=fail_factory)
        peer, thread, result = serve_once(server)
        establish(peer)
        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        thread.join(timeout=2.0)
        self.assertFalse(thread.is_alive())
        self.assertTrue(result["outcome"].protocol_failed)
        peer.close()

        wrong_session_owners: list[FakeAudioOwner] = []

        def wrong_session_factory(session_id: int):
            owner = FakeAudioOwner(session_id + 1, payload=b"")
            wrong_session_owners.append(owner)
            return owner

        server = wire_server.WireServer(
            audio_pcm_factory=wrong_session_factory
        )
        peer, thread, result = serve_once(server)
        establish(peer)
        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        thread.join(timeout=2.0)
        self.assertFalse(thread.is_alive())
        self.assertTrue(result["outcome"].protocol_failed)
        self.assertEqual(len(wrong_session_owners), 1)
        self.assertTrue(wrong_session_owners[0].closed)
        peer.close()

        credit_failure_owners: list[FakeAudioOwner] = []

        def credit_failure_factory(session_id: int):
            owner = FakeAudioOwner(
                session_id,
                payload=b"",
                fail_add_credit=True,
            )
            credit_failure_owners.append(owner)
            return owner

        server = wire_server.WireServer(
            audio_pcm_factory=credit_failure_factory
        )
        peer, thread, result = serve_once(server)
        establish(peer)
        peer.sendall(protocol.encode_audio_credit_frame(4, sequence=2))
        thread.join(timeout=2.0)
        self.assertFalse(thread.is_alive())
        self.assertTrue(result["outcome"].protocol_failed)
        self.assertEqual(len(credit_failure_owners), 1)
        self.assertEqual(credit_failure_owners[0].credit_amounts, [4])
        self.assertTrue(credit_failure_owners[0].closed)
        peer.close()

    def test_ordinary_product_runtime_injects_lazy_audio_factory(self) -> None:
        server = wire_runtime.build_product_wire_server()
        self.assertTrue(callable(server.audio_pcm_factory))
        self.assertIsNone(getattr(server, "_audio_pcm", None))

        source = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        self.assertIn("audio_pcm_factory=audio_pcm_factory", source)
        self.assertIn(
            "audio_product_profile.selected_audio_pcm_factory()",
            source,
        )


if __name__ == "__main__":
    program = unittest.main(verbosity=2, exit=False)
    if not program.result.wasSuccessful():
        raise SystemExit(1)
    print("PI_AUDIO_WIRE_SERVER_TEST=PASS")

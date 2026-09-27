#!/usr/bin/env python3
"""Deterministic R42 proof for ordinary Pi AUDIO product composition.

The fixture exercises the real composed WireServer while replacing only the
R39 AudioPcmProducer constructor. It proves ordinary product composition retains
one lazy exact-session factory, uses generated R36 PCM values plus the adopted
2.0-second retirement escalation horizon, and creates no capture owner until
the first exact channel-2 CREDIT.
"""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace
import socket
import sys
import threading
import time
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import audio_product_profile
import wire_protocol as protocol
import wire_runtime


class FakeAudioPcmProducer:
    """Minimal R39-shaped owner used to observe ordinary factory arguments."""

    items: list["FakeAudioPcmProducer"] = []

    def __init__(
        self,
        *,
        session_id: int,
        retirement_timeout_seconds: float,
        profile,
    ) -> None:
        self.session_id = session_id
        self.retirement_timeout_seconds = retirement_timeout_seconds
        self.profile = profile
        self.credit_amounts: list[int] = []
        self.closed = False
        self._wake_reader, self._wake_writer = socket.socketpair()
        self._wake_reader.setblocking(False)
        self._wake_writer.setblocking(False)
        type(self).items.append(self)

    @property
    def activity_reader(self):
        return self._wake_reader

    def acknowledge_activity(self) -> None:
        try:
            while self._wake_reader.recv(64):
                pass
        except BlockingIOError:
            pass
        except OSError:
            pass

    def check_health(self) -> None:
        if self.closed:
            raise RuntimeError("fake AUDIO owner is closed")

    def add_credit(self, amount: int) -> None:
        self.credit_amounts.append(amount)
        try:
            self._wake_writer.send(b"A")
        except OSError:
            pass

    def begin_emission(self, _maximum: int):
        return None

    def close(self) -> bool:
        if not self.closed:
            self.closed = True
            self._wake_reader.close()
            self._wake_writer.close()
        return True


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


def wait_for(predicate, description: str) -> None:
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.001)
    raise AssertionError(f"timed out waiting for {description}")


class AudioProductProfileTests(unittest.TestCase):
    def setUp(self) -> None:
        FakeAudioPcmProducer.items = []

    def test_selected_profile_uses_generated_r36_pcm_and_r42_horizon(self) -> None:
        profile = audio_product_profile.selected_audio_product_profile()

        self.assertEqual(profile.pcm.channel_window_bytes, 524288)
        self.assertEqual(profile.pcm.rate_hz, 48000)
        self.assertEqual(profile.pcm.channels, 2)
        self.assertEqual(profile.pcm.bits_per_sample, 16)
        self.assertEqual(profile.pcm.frame_bytes, 4)
        self.assertEqual(profile.retirement_timeout_seconds, 2.0)

        runtime_source = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        for scattered_literal in ("524288", "48000", "2.0"):
            self.assertNotIn(scattered_literal, runtime_source)

    def test_build_is_lazy_and_first_credit_creates_one_exact_session_owner(self) -> None:
        with mock.patch.object(
            audio_product_profile.audio,
            "AudioPcmProducer",
            FakeAudioPcmProducer,
        ):
            server = wire_runtime.build_product_wire_server()
            self.assertTrue(callable(server.audio_pcm_factory))
            self.assertEqual(FakeAudioPcmProducer.items, [])

            peer, thread, result = serve_once(server)
            session_id = establish(peer)

            # Q4, ordinary RFB composition and eager inert MPEG composition do
            # not create the AUDIO capture owner.
            self.assertEqual(FakeAudioPcmProducer.items, [])

            peer.sendall(protocol.encode_audio_credit_frame(8, sequence=2))
            wait_for(
                lambda: len(FakeAudioPcmProducer.items) == 1,
                "first exact-session AUDIO owner",
            )

            owner = FakeAudioPcmProducer.items[0]
            self.assertEqual(owner.session_id, session_id)
            self.assertEqual(owner.retirement_timeout_seconds, 2.0)
            self.assertEqual(owner.profile.channel_window_bytes, 524288)
            self.assertEqual(owner.profile.rate_hz, 48000)
            self.assertEqual(owner.profile.channels, 2)
            self.assertEqual(owner.profile.bits_per_sample, 16)
            self.assertEqual(owner.profile.frame_bytes, 4)
            self.assertEqual(owner.credit_amounts, [8])

            peer.sendall(protocol.encode_audio_credit_frame(4, sequence=3))
            wait_for(
                lambda: owner.credit_amounts == [8, 4],
                "second AUDIO credit reuse",
            )
            self.assertEqual(len(FakeAudioPcmProducer.items), 1)

            outcome = finish(peer, thread, result)
            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertTrue(owner.closed)

    def test_session_without_audio_credit_never_constructs_r39_owner(self) -> None:
        with mock.patch.object(
            audio_product_profile.audio,
            "AudioPcmProducer",
            FakeAudioPcmProducer,
        ):
            server = wire_runtime.build_product_wire_server()
            peer, thread, result = serve_once(server)
            establish(peer)
            outcome = finish(peer, thread, result)

            self.assertTrue(outcome.accepted)
            self.assertFalse(outcome.protocol_failed)
            self.assertEqual(FakeAudioPcmProducer.items, [])


class ScopeTests(unittest.TestCase):
    def test_r42_product_profile_does_not_take_capture_or_wire_mechanism(self) -> None:
        profile_source = (PI / "audio_product_profile.py").read_text(
            encoding="utf-8"
        )
        runtime_source = (PI / "wire_runtime.py").read_text(encoding="utf-8")
        combined = profile_source + "\n" + runtime_source

        for forbidden in (
            "sendall(",
            "recv(",
            "subprocess.Popen",
            "pw-record",
            "wpctl",
            "terminate(",
            "kill(",
        ):
            self.assertNotIn(forbidden, combined)


if __name__ == "__main__":
    program = unittest.main(verbosity=2, exit=False)
    if not program.result.wasSuccessful():
        raise SystemExit(1)
    print("PI_AUDIO_PRODUCT_COMPOSITION_TEST=PASS")

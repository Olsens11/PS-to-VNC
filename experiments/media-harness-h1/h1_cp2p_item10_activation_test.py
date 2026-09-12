#!/usr/bin/env python3
"""Host contracts for CP2P item #10's public all-guns activation seam."""

from pathlib import Path
from types import SimpleNamespace
import unittest

import h1_mux_server_cp2p_start_receiver as cp2p

base = cp2p.base


class FakeProducer:
    def check(self) -> None:
        pass


class FakeOwner:
    def __init__(self, generation: int) -> None:
        self.generation = generation
        self.archive_path = Path(f"/tmp/mpeg-generation-{generation}.m2v")
        self.open = False
        self.retiring = False
        self.in_flight = 0
        self.open_calls = 0
        self.producer = FakeProducer()

    def active_generation(self) -> int:
        return self.generation

    def open_emission_exact(self, generation: int) -> None:
        if self.generation != generation or self.retiring or self.open:
            raise base.ProtocolError("fake owner open mismatch")
        self.open = True
        self.open_calls += 1

    def begin_emission_exact(self, generation: int):
        if self.generation != generation:
            raise base.ProtocolError("fake owner begin mismatch")
        if not self.open or self.retiring:
            return None
        self.in_flight += 1
        return self.producer

    def finish_emission_exact(self, generation: int) -> None:
        if self.generation != generation or self.in_flight <= 0:
            raise base.ProtocolError("fake owner finish mismatch")
        self.in_flight -= 1


class FakeReceiver:
    def __init__(self, generation: int) -> None:
        self.request = SimpleNamespace(generation=generation)

    def peek_prepared(self):
        return self.request


class Item10Tests(unittest.TestCase):
    def test_profile_is_cp2p_only_and_exact(self) -> None:
        profile = dict(base.PROFILES["CP2P_ALL_GUNS"])
        self.assertEqual(profile["profile_id"], 3)
        self.assertEqual(profile["rfb_mode"], 2)
        self.assertEqual(profile["video_mode"], 1)
        self.assertEqual(profile["audio_mode"], base.AUDIO_PCM)

        session = object.__new__(cp2p.H1Cp2pStartReceiveSession)
        session._validate_composition_policy(profile)

        bad = dict(profile)
        bad["video_mode"] = 0
        with self.assertRaises(base.ProtocolError):
            session._validate_composition_policy(bad)

        parent = object.__new__(cp2p._ParentSession)
        with self.assertRaises(base.ProtocolError):
            parent._validate_composition_policy(profile)

    def test_scheduler_opens_once_and_sends_only_under_exact_lease(self) -> None:
        session = object.__new__(cp2p.H1Cp2pStartReceiveSession)
        owner = FakeOwner(7)
        session.cp2p_mpeg_producer = owner
        session.cp2p_start_receiver = FakeReceiver(7)
        session._cp2p_emission_opened_generations = set()
        session._cp2p_mpeg_generation_order = []
        session._cp2p_mpeg_sent_bytes = {}
        session._cp2p_mpeg_archive_paths = {}
        session.mpeg = base.ChannelState(base.CHANNEL_MPEG2, "mpeg")

        def send_from(producer, state, quantum, alignment):
            self.assertIs(producer, owner.producer)
            self.assertEqual(owner.in_flight, 1)
            self.assertEqual(state.channel, base.CHANNEL_MPEG2)
            self.assertEqual(alignment, 1)
            state.bytes_sent += 123
            return True

        session._send_from = send_from
        self.assertTrue(session._send_cp2p_mpeg_once())
        self.assertEqual(owner.open_calls, 1)
        self.assertEqual(owner.in_flight, 0)
        self.assertEqual(session._cp2p_mpeg_generation_order, [7])
        self.assertEqual(session._cp2p_mpeg_sent_bytes[7], 123)

        owner.open = False
        owner.retiring = True
        self.assertFalse(session._send_cp2p_mpeg_once())
        self.assertEqual(owner.open_calls, 1)

    def test_wrong_active_generation_never_sends(self) -> None:
        session = object.__new__(cp2p.H1Cp2pStartReceiveSession)
        owner = FakeOwner(9)
        session.cp2p_mpeg_producer = owner
        session.cp2p_start_receiver = FakeReceiver(10)
        session._cp2p_emission_opened_generations = set()
        session._cp2p_mpeg_generation_order = []
        session._cp2p_mpeg_sent_bytes = {}
        session._cp2p_mpeg_archive_paths = {}
        session.mpeg = base.ChannelState(base.CHANNEL_MPEG2, "mpeg")
        session._send_from = lambda *args, **kwargs: self.fail("unexpected MPEG send")

        self.assertFalse(session._send_cp2p_mpeg_once())
        self.assertEqual(owner.open_calls, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)

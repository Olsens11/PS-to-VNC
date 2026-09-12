#!/usr/bin/env python3
"""Host contracts for CP2P item #8's generation-scoped Pi MPEG producer."""

from __future__ import annotations

from pathlib import Path
import tempfile
import threading
import time
import unittest

import h1_mux_server as base
from h1_cp2p_capture_geometry import H1Cp2pCapturePlan
from h1_cp2p_mpeg_producer import H1Cp2pMpegProducer


class FakeProcess:
    def __init__(self) -> None:
        self.returncode: int | None = None

    def poll(self):
        return self.returncode

    def wait(self, timeout=None):
        if self.returncode is None:
            self.returncode = 0
        return self.returncode


class FakeThread:
    def join(self, timeout=None) -> None:
        pass

    def is_alive(self) -> bool:
        return False


class FakeProducer:
    def __init__(self, payload: bytes) -> None:
        self.buffer = bytearray(payload)
        self.bytes_produced = len(payload)
        self.stop_called = False
        self.terminate_called = False
        self.process = FakeProcess()
        self.thread = FakeThread()

    def stop(self) -> None:
        self.stop_called = True
        self.process.returncode = 0

    def terminate(self) -> None:
        self.terminate_called = True
        self.process.returncode = -15

    def check(self) -> None:
        pass

    def take(self, maximum: int, alignment: int = 1) -> bytes:
        count = min(maximum, len(self.buffer))
        count -= count % alignment
        data = bytes(self.buffer[:count])
        del self.buffer[:count]
        return data

    def done_and_empty(self) -> bool:
        return self.stop_called and not self.buffer


class StuckProducer(FakeProducer):
    def stop(self) -> None:
        self.stop_called = True

    def done_and_empty(self) -> bool:
        return False


class Clock:
    def __init__(self) -> None:
        self.now = 0.0

    def monotonic(self) -> float:
        return self.now

    def sleep(self, amount: float) -> None:
        self.now += max(amount, 0.1)


def plan(generation: int = 7) -> H1Cp2pCapturePlan:
    return H1Cp2pCapturePlan(
        session_id=0x1234ABCD,
        generation=generation,
        desktop_width=704,
        desktop_height=480,
        capture_x=32,
        capture_y=48,
        capture_width=320,
        capture_height=240,
        display=":0",
        command=(
            "ffmpeg", "-f", "x11grab", "-video_size", "320x240",
            "-i", ":0+32,48", "pipe:1",
        ),
    )


class ProducerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.evidence = Path(self.temp.name)

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_start_launches_exact_prepared_plan_and_emits_no_pstv_data(self) -> None:
        observed = []
        attached = []
        fake = FakeProducer(b"mpeg-bytes")

        def factory(capture_plan, archive_path):
            observed.append((capture_plan, archive_path))
            archive_path.write_bytes(b"mpeg-bytes")
            return fake

        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=factory,
            attach=attached.append,
        )
        evidence = owner.start_exact(plan())

        self.assertEqual(owner.active_generation(), 7)
        self.assertIs(observed[0][0], owner.plan)
        self.assertEqual(list(owner.plan.command), evidence["command"])
        self.assertEqual(evidence["state"], "live-local-producer-public-mpeg-gate-closed")
        self.assertFalse(evidence["pstv_mpeg_data_emission"])
        self.assertEqual(attached, [fake])
        self.assertTrue((self.evidence / "mpeg_producer_live.json").exists())

    def test_exact_retirement_stops_and_drains_all_unsent_bytes(self) -> None:
        payload = b"x" * 150000
        fake = FakeProducer(payload)
        attached = []
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
            attach=attached.append,
        )
        owner.start_exact(plan(9))
        result = owner.retire_exact(9)

        self.assertTrue(fake.stop_called)
        self.assertFalse(fake.terminate_called)
        self.assertEqual(result.bytes_produced, len(payload))
        self.assertEqual(result.bytes_discarded_unsent, len(payload))
        self.assertEqual(owner.active_generation(), 0)
        self.assertEqual(attached[-1], None)
        self.assertFalse((self.evidence / "mpeg_producer_live.json").exists())
        self.assertTrue((self.evidence / "mpeg_producer_retired.json").exists())

    def test_wrong_generation_cannot_retire_live_producer(self) -> None:
        fake = FakeProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(11))
        with self.assertRaises(base.ProtocolError):
            owner.retire_exact(12)
        self.assertEqual(owner.active_generation(), 11)
        self.assertFalse(fake.stop_called)

    def test_retirement_waits_for_inflight_emission_lease(self) -> None:
        fake = FakeProducer(b"queued-after-send")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(15))

        # #10 is the only future caller allowed to open this fence. Once open,
        # one scheduler lease represents DATA that may already be in send_frame.
        owner.open_emission_exact(15)
        self.assertIs(owner.begin_emission_exact(15), fake)

        result = []
        failure = []

        def retire():
            try:
                result.append(owner.retire_exact(15))
            except BaseException as exc:
                failure.append(exc)

        thread = threading.Thread(target=retire)
        thread.start()

        deadline = time.monotonic() + 1.0
        while owner.emission_open_for_generation(15) and time.monotonic() < deadline:
            time.sleep(0.001)

        self.assertFalse(owner.emission_open_for_generation(15))
        self.assertFalse(fake.stop_called)
        self.assertIsNone(owner.begin_emission_exact(15))

        owner.finish_emission_exact(15)
        thread.join(timeout=1.0)
        self.assertFalse(thread.is_alive())
        self.assertEqual(failure, [])
        self.assertEqual(len(result), 1)
        self.assertTrue(fake.stop_called)
        self.assertEqual(owner.active_generation(), 0)

    def test_emission_fence_is_closed_by_default_and_exact_generation_only(self) -> None:
        fake = FakeProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
        )
        owner.start_exact(plan(17))
        self.assertFalse(owner.emission_open_for_generation(17))
        self.assertIsNone(owner.begin_emission_exact(17))
        with self.assertRaises(base.ProtocolError):
            owner.open_emission_exact(18)
        owner.retire_exact(17)

    def test_stuck_producer_fails_closed_after_terminate_grace(self) -> None:
        clock = Clock()
        fake = StuckProducer(b"abc")
        owner = H1Cp2pMpegProducer(
            self.evidence,
            producer_factory=lambda capture_plan, archive: fake,
            monotonic=clock.monotonic,
            sleep=clock.sleep,
        )
        owner.start_exact(plan(13))
        with self.assertRaises(base.ProtocolError):
            owner.retire_exact(13, timeout=0.2)
        self.assertTrue(fake.stop_called)
        self.assertTrue(fake.terminate_called)
        self.assertEqual(owner.active_generation(), 13)


if __name__ == "__main__":
    unittest.main(verbosity=2)

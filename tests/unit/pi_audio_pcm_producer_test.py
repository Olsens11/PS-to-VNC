#!/usr/bin/env python3
"""Deterministic host proof for the R39 Pi AUDIO PCM producer owner."""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys
import threading
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import audio_pcm_producer as audio
import wire_protocol as protocol


class ControlledStdout:
    def __init__(self) -> None:
        self._condition = threading.Condition()
        self._data = bytearray()
        self._closed = False

    def feed(self, payload: bytes) -> None:
        with self._condition:
            self._data.extend(payload)
            self._condition.notify_all()

    def close_stream(self) -> None:
        with self._condition:
            self._closed = True
            self._condition.notify_all()

    def read(self, maximum: int) -> bytes:
        with self._condition:
            while not self._data and not self._closed:
                self._condition.wait()
            if self._data:
                count = min(maximum, len(self._data))
                result = bytes(self._data[:count])
                del self._data[:count]
                return result
            return b""


class FakeProcess:
    def __init__(self, *, stubborn: bool = False) -> None:
        self.stdout = ControlledStdout()
        self.returncode: int | None = None
        self.stubborn = stubborn
        self.terminate_calls = 0
        self.kill_calls = 0
        self.wait_calls = 0

    def poll(self):
        return self.returncode

    def terminate(self) -> None:
        self.terminate_calls += 1
        if not self.stubborn:
            self.returncode = -15
            self.stdout.close_stream()

    def kill(self) -> None:
        self.kill_calls += 1
        if not self.stubborn:
            self.returncode = -9
            self.stdout.close_stream()

    def wait(self, timeout=None):
        self.wait_calls += 1
        if self.returncode is None:
            raise subprocess.TimeoutExpired("pw-record", timeout)
        return self.returncode


class PopenFactory:
    def __init__(
        self,
        *,
        stubborn: bool = False,
        fail: bool = False,
    ) -> None:
        self.stubborn = stubborn
        self.fail = fail
        self.commands: list[list[str]] = []
        self.processes: list[FakeProcess] = []

    def __call__(self, command, **_kwargs):
        self.commands.append(list(command))
        if self.fail:
            raise OSError("injected pw-record launch failure")
        process = FakeProcess(stubborn=self.stubborn)
        self.processes.append(process)
        return process


def wpctl_result(stdout: str):
    def run(command, **_kwargs):
        if command != ["wpctl", "inspect", "@DEFAULT_AUDIO_SINK@"]:
            raise AssertionError(f"unexpected wpctl command: {command}")
        return subprocess.CompletedProcess(
            command,
            0,
            stdout=stdout,
            stderr="",
        )

    return run


def wait_for(predicate, description: str, timeout: float = 1.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.001)
    raise AssertionError(f"timed out waiting for {description}")


def make_owner(
    *,
    session_id: int = 7,
    stubborn: bool = False,
    fail_launch: bool = False,
):
    popen = PopenFactory(stubborn=stubborn, fail=fail_launch)
    owner = audio.AudioPcmProducer(
        session_id=session_id,
        retirement_timeout_seconds=0.05,
        run=wpctl_result(
            '  * node.name = "alsa_output.pci-test.analog-stereo"\n'
        ),
        popen=popen,
    )
    return owner, popen


class AudioProfileAndCaptureTests(unittest.TestCase):
    def test_selected_projection_is_exact_and_frame_alignment_is_derived(self) -> None:
        profile = audio.selected_audio_pcm_profile()
        self.assertEqual(profile.channel_window_bytes, 524288)
        self.assertEqual(profile.rate_hz, 48000)
        self.assertEqual(profile.channels, 2)
        self.assertEqual(profile.bits_per_sample, 16)
        self.assertEqual(profile.frame_bytes, 4)
        self.assertEqual(
            profile.frame_bytes,
            profile.channels * (profile.bits_per_sample // 8),
        )

    def test_default_sink_monitor_discovery_and_command_are_exact(self) -> None:
        monitor = audio.discover_default_sink_monitor(
            run=wpctl_result(
                'id 42, type PipeWire:Interface:Node\n'
                '    node.description = "Speakers"\n'
                '  * node.name = "alsa_output.usb-device.stereo"\n'
            )
        )
        self.assertEqual(
            monitor,
            "alsa_output.usb-device.stereo.monitor",
        )
        self.assertEqual(
            audio.build_capture_command(
                monitor,
                audio.selected_audio_pcm_profile(),
            ),
            (
                "pw-record",
                "--target",
                "alsa_output.usb-device.stereo.monitor",
                "--rate",
                "48000",
                "--format",
                "s16",
                "--channels",
                "2",
                "-",
            ),
        )

        with self.assertRaises(audio.AudioPcmProducerError):
            audio.discover_default_sink_monitor(
                run=wpctl_result('node.description = "missing name"\n')
            )
        with self.assertRaises(audio.AudioPcmProducerError):
            audio.discover_default_sink_monitor(
                run=wpctl_result('node.name = "bad target"\n')
            )

    def test_capture_launch_failure_is_owner_failure(self) -> None:
        popen = PopenFactory(fail=True)
        with self.assertRaises(audio.AudioPcmProducerError):
            audio.AudioPcmProducer(
                session_id=9,
                retirement_timeout_seconds=0.05,
                run=wpctl_result('node.name = "alsa_output.test"\n'),
                popen=popen,
            )


class AudioCreditAndFramingTests(unittest.TestCase):
    def test_credit_is_only_emission_authority_and_payload_is_bounded_aligned(self) -> None:
        owner, popen = make_owner()
        process = popen.processes[0]
        self.assertEqual(
            popen.commands[0],
            list(audio.build_capture_command(owner.monitor, owner.profile)),
        )

        process.stdout.feed(b"A" * 12)
        wait_for(
            lambda: owner.available_complete_bytes == 12,
            "initial PCM bytes",
        )

        self.assertIsNone(owner.begin_emission(protocol.MAX_PAYLOAD_BYTES))
        owner.add_credit(3)
        self.assertIsNone(owner.begin_emission(protocol.MAX_PAYLOAD_BYTES))
        owner.add_credit(1)
        first = owner.begin_emission(protocol.MAX_PAYLOAD_BYTES)
        self.assertEqual(first, b"A" * 4)
        self.assertEqual(owner.credit_bytes, 0)

        process.stdout.feed(b"B" * protocol.MAX_PAYLOAD_BYTES)
        wait_for(
            lambda: owner.available_complete_bytes >= protocol.MAX_PAYLOAD_BYTES,
            "Wire-maximum PCM payload",
        )
        owner.add_credit(protocol.MAX_PAYLOAD_BYTES)
        second = owner.begin_emission(protocol.MAX_PAYLOAD_BYTES + 100)
        self.assertIsNotNone(second)
        self.assertEqual(len(second), protocol.MAX_PAYLOAD_BYTES)
        self.assertEqual(len(second) % owner.profile.frame_bytes, 0)
        self.assertEqual(owner.credit_bytes, 0)

        self.assertTrue(owner.close())
        self.assertTrue(owner.close())

    def test_credit_overflow_is_rejected_and_spool_never_exceeds_window(self) -> None:
        owner, popen = make_owner(session_id=8)
        with self.assertRaises(audio.AudioPcmProducerError):
            owner.add_credit(owner.profile.channel_window_bytes + 1)

        process = popen.processes[0]
        process.stdout.feed(
            b"C" * (
                owner.profile.channel_window_bytes
                + protocol.MAX_PAYLOAD_BYTES
            )
        )
        wait_for(
            lambda: owner.available_complete_bytes
            == owner.profile.channel_window_bytes,
            "bounded AUDIO spool",
        )
        self.assertLessEqual(
            owner.available_complete_bytes,
            owner.profile.channel_window_bytes,
        )
        self.assertTrue(owner.close())


class AudioTerminalityTests(unittest.TestCase):
    def test_unexpected_eof_and_malformed_tail_fail_closed(self) -> None:
        owner, popen = make_owner(session_id=10)
        popen.processes[0].stdout.close_stream()
        wait_for(lambda: owner._eof, "unexpected AUDIO EOF")
        with self.assertRaises(audio.AudioPcmProducerError):
            owner.check_health()
        self.assertFalse(owner.close())
        self.assertFalse(owner.close())

        owner, popen = make_owner(session_id=11)
        popen.processes[0].stdout.feed(b"XYZ")
        wait_for(lambda: len(owner._buffer) == 3, "partial PCM frame")
        popen.processes[0].stdout.close_stream()
        wait_for(lambda: owner._eof, "malformed-tail AUDIO EOF")
        with self.assertRaises(audio.AudioPcmProducerError):
            owner.check_health()
        self.assertIn("incomplete PCM frame", str(owner._error))
        self.assertFalse(owner.close())

    def test_cleanup_requires_actual_process_and_reader_retirement(self) -> None:
        owner, popen = make_owner(session_id=12, stubborn=True)
        process = popen.processes[0]
        self.assertFalse(owner.close())
        self.assertFalse(owner.close())
        self.assertGreaterEqual(process.terminate_calls, 1)
        self.assertGreaterEqual(process.kill_calls, 1)

        # Test-only cleanup of the deliberately unretired daemon reader.
        process.stdout.close_stream()
        owner._thread.join(timeout=1.0)


if __name__ == "__main__":
    program = unittest.main(verbosity=2, exit=False)
    if not program.result.wasSuccessful():
        raise SystemExit(1)
    print("PI_AUDIO_PCM_PRODUCER_TEST=PASS")

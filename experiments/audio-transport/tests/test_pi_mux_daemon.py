#!/usr/bin/env python3

import importlib.util
import socket
import struct
import sys
import unittest
from pathlib import Path


MODULE_PATH = (
    Path(__file__).resolve().parents[1]
    / "pi"
    / "mux_daemon.py"
)

SPEC = importlib.util.spec_from_file_location(
    "pstvnc_mux_daemon",
    MODULE_PATH,
)

assert SPEC is not None
assert SPEC.loader is not None

mux = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = mux
SPEC.loader.exec_module(mux)


class ProtocolTests(unittest.TestCase):
    def test_frame_round_trip(self):
        left, right = socket.socketpair()

        try:
            expected = mux.Frame(
                kind=mux.FRAME_DATA,
                channel=mux.CHANNEL_RFB,
                flags=0x5A,
                sequence=0x12345678,
                payload=b"hello",
            )

            left.sendall(
                mux.encode_frame(expected)
            )

            actual = mux.receive_frame(right)

            self.assertEqual(
                actual,
                expected,
            )
        finally:
            left.close()
            right.close()

    def test_bad_magic_rejected(self):
        left, right = socket.socketpair()

        try:
            wire = mux.HEADER_STRUCT.pack(
                0x11111111,
                mux.VERSION,
                mux.FRAME_HELLO,
                mux.CHANNEL_CONTROL,
                0,
                1,
                0,
            )

            left.sendall(wire)

            with self.assertRaises(
                mux.ProtocolError
            ):
                mux.receive_frame(right)
        finally:
            left.close()
            right.close()


class CreditTests(unittest.TestCase):
    def make_channel(self):
        return mux.ChannelState(
            name="rfb",
            channel_id=mux.CHANNEL_RFB,
            quantum=8192,
            weight=12,
            policy_window=16384,
            host_buffer_limit=65536,
        )

    def test_initial_credit_and_consumption(self):
        channel = self.make_channel()

        channel.add_credit(32768)

        self.assertEqual(
            channel.physical_capacity,
            32768,
        )

        channel.append(
            b"x" * 20000
        )

        self.assertEqual(
            channel.send_budget(),
            8192,
        )

        payload = channel.take_for_send(
            8192
        )

        self.assertEqual(
            len(payload),
            8192,
        )

        self.assertEqual(
            channel.outstanding,
            8192,
        )

        channel.add_credit(4096)

        self.assertEqual(
            channel.outstanding,
            4096,
        )

    def test_policy_window_limits_outstanding(self):
        channel = self.make_channel()

        channel.add_credit(32768)
        channel.append(
            b"x" * 32768
        )

        channel.take_for_send(8192)
        channel.take_for_send(8192)

        self.assertEqual(
            channel.outstanding,
            16384,
        )

        self.assertEqual(
            channel.send_budget(),
            0,
        )


class SchedulerTests(unittest.TestCase):
    def test_smooth_weight_ratio(self):
        rfb = mux.ChannelState(
            name="rfb",
            channel_id=mux.CHANNEL_RFB,
            quantum=8192,
            weight=12,
            policy_window=32768,
            host_buffer_limit=65536,
        )

        audio = mux.ChannelState(
            name="audio",
            channel_id=mux.CHANNEL_AUDIO,
            quantum=4096,
            weight=4,
            policy_window=16384,
            host_buffer_limit=65536,
            alignment=4,
        )

        chooser = mux.SmoothWeightedChooser(
            {
                mux.CHANNEL_RFB: rfb,
                mux.CHANNEL_AUDIO: audio,
            }
        )

        counts = {
            mux.CHANNEL_RFB: 0,
            mux.CHANNEL_AUDIO: 0,
        }

        for _ in range(16):
            chosen = chooser.choose()
            counts[
                chosen.channel_id
            ] += 1

        self.assertEqual(
            counts[mux.CHANNEL_RFB],
            12,
        )

        self.assertEqual(
            counts[mux.CHANNEL_AUDIO],
            4,
        )


class TelemetryTests(unittest.TestCase):
    def test_telemetry_layout(self):
        values = tuple(range(24))

        payload = struct.pack(
            ">" + ("I" * 24),
            *values,
        )

        telemetry = mux.parse_telemetry(
            payload
        )

        self.assertEqual(
            telemetry["version"],
            0,
        )

        self.assertEqual(
            telemetry[
                "audio_bytes_played"
            ],
            22,
        )

        self.assertEqual(
            telemetry["reserved"],
            23,
        )


if __name__ == "__main__":
    unittest.main()

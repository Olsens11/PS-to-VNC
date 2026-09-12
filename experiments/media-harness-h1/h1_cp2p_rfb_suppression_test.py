#!/usr/bin/env python3
"""Host tests for CP2P generation-scoped Pi RFB suppression."""

from __future__ import annotations

from dataclasses import dataclass
import socket
import struct
import time
import unittest

from h1_cp2p_rfb_suppression import H1Cp2pRfbPiBridge, RfbBridgeError


@dataclass(frozen=True)
class DummyStart:
    generation: int
    suppression_x: int
    suppression_y: int
    suppression_width: int
    suppression_height: int


def make_bridge(width: int, height: int, captured: list[bytes] | None = None):
    bridge_sock, fake_vnc = socket.socketpair()
    if captured is None:
        captured = []
    bridge = H1Cp2pRfbPiBridge(
        bridge_sock,
        queue_capacity=max(256, width * height * 2 + 256),
        max_payload=64,
        send_data=lambda payload: captured.append(bytes(payload)),
        desktop_width=width,
        desktop_height=height,
    )
    return bridge, fake_vnc, captured


def framebuffer_request(width: int, height: int, incremental: int = 1) -> bytes:
    return struct.pack(">BBHHHH", 3, incremental, 0, 0, width, height)


def raw_update(width: int, height: int) -> bytes:
    pixels = bytearray()
    for pixel in range(width * height):
        pixels.extend(struct.pack("<H", pixel & 0xFFFF))
    return (
        b"\x00\x00\x00\x01"
        + struct.pack(">HHHHi", 0, 0, width, height, 0)
        + bytes(pixels)
    )


def parse_raw_update(payload: bytes):
    if len(payload) < 4 or payload[0] != 0:
        raise AssertionError("not a framebuffer update")
    count = struct.unpack(">H", payload[2:4])[0]
    offset = 4
    rectangles = []
    for _ in range(count):
        x, y, width, height, encoding = struct.unpack(
            ">HHHHi", payload[offset:offset + 12]
        )
        offset += 12
        if encoding != 0:
            raise AssertionError("non-Raw rectangle in filtered output")
        byte_count = width * height * 2
        raw = payload[offset:offset + byte_count]
        offset += byte_count
        rectangles.append((x, y, width, height, raw))
    if offset != len(payload):
        raise AssertionError("trailing bytes in filtered update")
    return rectangles


def wait_for(predicate, description: str, timeout: float = 2.0) -> None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.005)
    raise AssertionError(f"timed out waiting for {description}")


class DynamicDesktopTests(unittest.TestCase):
    def test_raw_budget_uses_current_desktop_not_one_fixed_resolution(self) -> None:
        for width, height in ((640, 448), (704, 462), (704, 480), (1280, 720)):
            bridge, fake_vnc, _ = make_bridge(width, height)
            try:
                self.assertEqual(
                    bridge.desktop_raw_byte_budget,
                    width * height * 2,
                )
            finally:
                bridge.stop()
                fake_vnc.close()

    def test_suppression_bounds_follow_current_desktop(self) -> None:
        bridge, fake_vnc, _ = make_bridge(1280, 720)
        try:
            bridge.install_suppression(DummyStart(9, 960, 480, 320, 240))
            pending, active, rect = bridge._suppression_state()
            self.assertTrue(pending)
            self.assertFalse(active)
            self.assertEqual((rect.right, rect.bottom), (1280, 720))  # type: ignore[union-attr]
        finally:
            bridge.stop()
            fake_vnc.close()

        bridge, fake_vnc, _ = make_bridge(640, 448)
        try:
            with self.assertRaises(Exception):
                bridge.install_suppression(DummyStart(9, 600, 400, 80, 64))
        finally:
            bridge.stop()
            fake_vnc.close()


class ActivationTests(unittest.TestCase):
    def test_start_is_pending_until_first_new_framebuffer_request(self) -> None:
        bridge, fake_vnc, _ = make_bridge(64, 48)
        try:
            bridge.install_suppression(DummyStart(7, 16, 16, 32, 16))

            # Key input after START does not activate the server-update filter.
            key = struct.pack(">BBHI", 4, 1, 0, 0x61)
            bridge.accept_client_data(key)
            self.assertEqual(fake_vnc.recv(len(key)), key)
            pending, active, _ = bridge._suppression_state()
            self.assertTrue(pending)
            self.assertFalse(active)

            request = framebuffer_request(64, 48)
            bridge.accept_client_data(request)
            self.assertEqual(fake_vnc.recv(len(request)), request)
            pending, active, _ = bridge._suppression_state()
            self.assertFalse(pending)
            self.assertTrue(active)
            self.assertEqual(bridge.suppression_stats.activations, 1)
        finally:
            bridge.stop()
            fake_vnc.close()


class FilterTests(unittest.TestCase):
    def test_full_raw_rectangle_is_split_around_suppression_hole(self) -> None:
        captured: list[bytes] = []
        bridge, fake_vnc, _ = make_bridge(8, 4, captured)
        bridge.start()
        try:
            bridge.install_suppression(DummyStart(3, 2, 1, 4, 2))
            request = framebuffer_request(8, 4)
            bridge.accept_client_data(request)
            self.assertEqual(fake_vnc.recv(len(request)), request)

            source = raw_update(8, 4)
            expected_pixel_bytes = (8 * 4 - 4 * 2) * 2
            expected_output_bytes = 4 + (4 * 12) + expected_pixel_bytes
            bridge.add_credit(expected_output_bytes)
            fake_vnc.sendall(source)

            wait_for(
                lambda: sum(len(part) for part in captured) == expected_output_bytes,
                "filtered framebuffer update",
            )
            rectangles = parse_raw_update(b"".join(captured))
            self.assertEqual(
                [(x, y, width, height) for x, y, width, height, _ in rectangles],
                [
                    (0, 0, 8, 1),
                    (0, 3, 8, 1),
                    (0, 1, 2, 2),
                    (6, 1, 2, 2),
                ],
            )
            self.assertEqual(bridge.suppression_stats.filtered_updates, 1)
            self.assertEqual(bridge.suppression_stats.input_rectangles, 1)
            self.assertEqual(bridge.suppression_stats.output_rectangles, 4)
            self.assertEqual(bridge.suppression_stats.input_pixel_bytes, 8 * 4 * 2)
            self.assertEqual(
                bridge.suppression_stats.output_pixel_bytes,
                expected_pixel_bytes,
            )
        finally:
            bridge.stop()
            fake_vnc.close()

    def test_update_pixel_budget_is_current_desktop_worth(self) -> None:
        bridge, fake_vnc, _ = make_bridge(4, 4)
        try:
            suppression = DummyStart(5, 1, 1, 2, 2)
            bridge.install_suppression(suppression)
            bridge._activate_pending_for_update_request()
            _, _, rect = bridge._suppression_state()
            self.assertIsNotNone(rect)

            # Two complete 4x4 Raw rectangles would carry two desktop-worths of
            # pixel data. The second rectangle must fail the dynamic 4x4 budget;
            # this guard scales with whatever desktop the session actually uses.
            one_raw = bytes(4 * 4 * 2)
            wire = (
                b"\x00\x00\x02"
                + struct.pack(">HHHHi", 0, 0, 4, 4, 0)
                + one_raw
                + struct.pack(">HHHHi", 0, 0, 4, 4, 0)
                + one_raw
            )
            fake_vnc.sendall(wire)
            with self.assertRaises(RfbBridgeError):
                bridge._read_filtered_framebuffer_update(b"\x00", rect)  # type: ignore[arg-type]
        finally:
            bridge.stop()
            fake_vnc.close()


if __name__ == "__main__":
    unittest.main(verbosity=2)

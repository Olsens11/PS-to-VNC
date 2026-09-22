#!/usr/bin/env python3
"""Run the R12 provider suite with R15/R16A-aware Wire assertions."""

from __future__ import annotations

import unittest

import pi_internal_rfb_provider_test_legacy as legacy


def test_r10_relay_protocol_and_current_service_remain_separated(self) -> None:
    # R15 intentionally changed the ordinary service entrypoint, and R16A
    # intentionally extends Wire protocol representation with the typed
    # channel-1 provider-terminal ERROR. Preserve exact R10 Relay identity while
    # checking that neither the protocol extension nor the current service has
    # acquired R12 provider endpoint/process policy.
    stable_blobs = {
        legacy.RFB_RELAY: "55e946f2cc8e7d449b83616a6fe8653695c06702",
    }
    for path, expected_blob in stable_blobs.items():
        self.assertEqual(legacy.git_blob_sha1(path), expected_blob, msg=str(path))

    wire_protocol = legacy.text(legacy.WIRE_PROTOCOL)
    wire_server = legacy.text(legacy.WIRE_SERVER)
    relay = legacy.text(legacy.RFB_RELAY)
    wire_unit = legacy.text(legacy.WIRE_UNIT)

    # R10's DATA/CREDIT logical-RFB vocabulary remains present; R16A's ERROR
    # representation is an additive, explicitly fenced extension rather than a
    # provider endpoint or process-selection responsibility.
    self.assertIn("FRAME_DATA = 3", wire_protocol)
    self.assertIn("FRAME_CREDIT = 4", wire_protocol)
    self.assertIn("CHANNEL_RFB = 1", wire_protocol)
    self.assertIn("FRAME_ERROR = 7", wire_protocol)
    self.assertIn("PRODUCT_ESTABLISHMENT_VERSION = 2", wire_protocol)

    self.assertIn("self.serve_connection(connection)", wire_server)
    self.assertIn("/usr/lib/ps-to-vnc/wire_runtime.py", wire_unit)

    for source in (wire_protocol, wire_server, relay, wire_unit):
        self.assertNotIn("127.0.0.1:5900", source)
        self.assertNotIn("127.0.0.1:5903", source)
        self.assertNotIn("ps-to-vnc-rfb-internal", source)
        self.assertNotIn("X0tigervnc", source)


legacy.InternalProviderAuthorityTests.test_r10_relay_protocol_and_default_service_remain_stable = (
    test_r10_relay_protocol_and_current_service_remain_separated
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)

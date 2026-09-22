#!/usr/bin/env python3
"""Run the R12 provider suite with R15-aware current Wire service assertions."""

from __future__ import annotations

import unittest

import pi_internal_rfb_provider_test_legacy as legacy


def test_r10_relay_protocol_and_current_service_remain_separated(self) -> None:
    # R15 intentionally changes only the ordinary service entrypoint. Preserve
    # exact R10 Relay/protocol identity while checking the current unit still
    # contains no internal-provider endpoint or provider process policy.
    stable_blobs = {
        legacy.RFB_RELAY: "55e946f2cc8e7d449b83616a6fe8653695c06702",
        legacy.WIRE_PROTOCOL: "a136640272ab942ce3e15096ff9da2089491745d",
    }
    for path, expected_blob in stable_blobs.items():
        self.assertEqual(legacy.git_blob_sha1(path), expected_blob, msg=str(path))

    wire_server = legacy.text(legacy.WIRE_SERVER)
    relay = legacy.text(legacy.RFB_RELAY)
    wire_unit = legacy.text(legacy.WIRE_UNIT)

    self.assertIn("self.serve_connection(connection)", wire_server)
    self.assertIn("/usr/lib/ps-to-vnc/wire_runtime.py", wire_unit)

    for source in (wire_server, relay, wire_unit):
        self.assertNotIn("127.0.0.1:5900", source)
        self.assertNotIn("127.0.0.1:5903", source)
        self.assertNotIn("ps-to-vnc-rfb-internal", source)
        self.assertNotIn("X0tigervnc", source)


legacy.InternalProviderAuthorityTests.test_r10_relay_protocol_and_default_service_remain_stable = (
    test_r10_relay_protocol_and_current_service_remain_separated
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)

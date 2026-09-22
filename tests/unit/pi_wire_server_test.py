#!/usr/bin/env python3
"""Run the established Pi Wire suite with R15/R16A product assertions."""

from __future__ import annotations

import unittest

import pi_wire_server_test_legacy as legacy


# R16A deliberately advances only product-establishment compatibility to v2.
# Fixed PSTV framing remains version 1. Keep the old golden suite intact except
# for the compatibility word whose semantic authority intentionally changed.
R16A_HELLO_GOLDEN = bytes.fromhex(
    "50535456"
    "01010000"
    "00000001"
    "00000008"
    "00000001"
    "00000002"
)


def test_r16a_exact_golden_establishment_bytes(self) -> None:
    self.assertEqual(legacy.protocol.encode_hello_frame(), R16A_HELLO_GOLDEN)
    self.assertEqual(
        legacy.protocol.encode_accept_frame(0x12345678),
        legacy.ACCEPT_GOLDEN,
    )
    self.assertEqual(
        legacy.protocol.encode_not_accepted_frame(
            legacy.protocol.REJECT_PRODUCT_VERSION
        ),
        legacy.REJECT_GOLDEN,
    )

    hello_header = legacy.protocol.decode_header(
        R16A_HELLO_GOLDEN[: legacy.protocol.HEADER_BYTES]
    )
    self.assertTrue(legacy.protocol.is_hello_header(hello_header))
    self.assertEqual(
        legacy.protocol.decode_hello_payload(
            R16A_HELLO_GOLDEN[legacy.protocol.HEADER_BYTES :]
        ),
        (legacy.protocol.WIRE_VERSION, 2),
    )

    accept_header = legacy.protocol.decode_header(
        legacy.ACCEPT_GOLDEN[: legacy.protocol.HEADER_BYTES]
    )
    self.assertTrue(legacy.protocol.is_accept_header(accept_header))
    self.assertEqual(
        legacy.protocol.decode_accept_payload(
            legacy.ACCEPT_GOLDEN[legacy.protocol.HEADER_BYTES :]
        ),
        0x12345678,
    )

    reject_header = legacy.protocol.decode_header(
        legacy.REJECT_GOLDEN[: legacy.protocol.HEADER_BYTES]
    )
    self.assertTrue(legacy.protocol.is_not_accepted_header(reject_header))
    self.assertEqual(
        legacy.protocol.decode_not_accepted_payload(
            legacy.REJECT_GOLDEN[legacy.protocol.HEADER_BYTES :]
        ),
        legacy.protocol.REJECT_PRODUCT_VERSION,
    )


def test_r16a_wire_and_product_version_rejections_never_activate(self) -> None:
    # Product-v1 is now the deliberately incompatible old peer. Both mismatches
    # are rejected during Q4, before ACTIVE traffic can be interpreted under
    # different channel-1 ERROR semantics.
    for wire_version, product_version, reason in (
        (2, 2, legacy.protocol.REJECT_WIRE_VERSION),
        (1, 1, legacy.protocol.REJECT_PRODUCT_VERSION),
    ):
        with self.subTest(reason=reason):
            wire_server = legacy.server.WireServer()
            outcome, response = legacy.run_session(
                wire_server,
                legacy.protocol.encode_hello_frame(
                    wire_version=wire_version,
                    product_version=product_version,
                ),
            )
            self.assertFalse(outcome.accepted)
            self.assertEqual(outcome.rejection_reason, reason)
            self.assertIsNone(outcome.session_id)
            self.assertEqual(
                legacy.protocol.decode_not_accepted_payload(
                    response[legacy.protocol.HEADER_BYTES :]
                ),
                reason,
            )


def test_r16a_provider_failure_wire_bytes_are_distinct(self) -> None:
    frame = legacy.protocol.encode_rfb_provider_failure_frame(
        legacy.protocol.RFB_PROVIDER_FAILURE_READ,
        sequence=7,
    )
    header = legacy.protocol.decode_header(frame[: legacy.protocol.HEADER_BYTES])
    payload = frame[legacy.protocol.HEADER_BYTES :]

    self.assertTrue(legacy.protocol.is_rfb_provider_failure_header(header))
    self.assertFalse(legacy.protocol.is_rfb_data_header(header))
    self.assertFalse(legacy.protocol.is_rfb_credit_header(header))
    self.assertEqual(header.kind, legacy.protocol.FRAME_ERROR)
    self.assertEqual(header.channel, legacy.protocol.CHANNEL_RFB)
    self.assertEqual(
        legacy.protocol.decode_rfb_provider_failure_payload(payload),
        legacy.protocol.RFB_PROVIDER_FAILURE_READ,
    )


def test_r15_wire_service_is_ordinary_supervised_service(self) -> None:
    unit = (
        legacy.REPO_ROOT / "systemd/pi/ps-to-vnc-wire.service"
    ).read_text(encoding="utf-8")
    self.assertIn("User=ps2", unit)
    self.assertIn("Group=ps2", unit)
    self.assertIn(
        "ExecStart=/usr/bin/python3 /usr/lib/ps-to-vnc/wire_runtime.py "
        "--listen 192.168.50.1 --port 5902",
        unit,
    )
    self.assertIn("Restart=on-failure", unit)
    self.assertNotIn("StandardInput=socket", unit)
    self.assertNotIn("ps-to-vnc-rfb", unit)
    for literal in ("32768", "8192", "16384", "5903"):
        self.assertNotIn(literal, unit)


def test_r15_stager_enrolls_profile_and_composition_files(self) -> None:
    installer = (
        legacy.REPO_ROOT / "scripts/pi/install-wire-runtime.sh"
    ).read_text(encoding="utf-8")
    for path in (
        "/usr/lib/ps-to-vnc/rfb_runtime_profile_generated.py",
        "/usr/lib/ps-to-vnc/rfb_runtime_profile.py",
        "/usr/lib/ps-to-vnc/wire_runtime.py",
    ):
        self.assertIn(path, installer)
    for symbol in (
        'verify_file 0644 "$RFB_PROFILE_GENERATED_SOURCE" "$RFB_PROFILE_GENERATED_DEST"',
        'verify_file 0644 "$RFB_PROFILE_SOURCE" "$RFB_PROFILE_DEST"',
        'verify_file 0644 "$RUNTIME_SOURCE" "$RUNTIME_DEST"',
    ):
        self.assertIn(symbol, installer)
    for forbidden in (
        "systemctl daemon-reload",
        "systemctl enable",
        "systemctl disable",
        "systemctl start",
        "systemctl stop",
        "systemctl restart",
    ):
        self.assertNotIn(forbidden, installer)
    self.assertIn("SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED", installer)
    self.assertIn("RFB_PROVIDER_STATE_CHANGED=NO", installer)


legacy.WireProtocolTests.test_exact_golden_establishment_bytes = (
    test_r16a_exact_golden_establishment_bytes
)
legacy.WireProtocolTests.test_r16a_provider_failure_wire_bytes_are_distinct = (
    test_r16a_provider_failure_wire_bytes_are_distinct
)
legacy.WireServerTests.test_wire_and_product_version_rejections_never_activate = (
    test_r16a_wire_and_product_version_rejections_never_activate
)
legacy.RepositoryBoundaryTests.test_wire_service_is_ordinary_supervised_service = (
    test_r15_wire_service_is_ordinary_supervised_service
)
legacy.RepositoryBoundaryTests.test_r15_stager_enrolls_profile_and_composition_files = (
    test_r15_stager_enrolls_profile_and_composition_files
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)

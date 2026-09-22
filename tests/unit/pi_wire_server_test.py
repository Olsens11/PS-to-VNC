#!/usr/bin/env python3
"""Run the established Wire-server suite with R15 service-entry assertions."""

from __future__ import annotations

import unittest

import pi_wire_server_test_legacy as legacy


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


legacy.RepositoryBoundaryTests.test_wire_service_is_ordinary_supervised_service = (
    test_r15_wire_service_is_ordinary_supervised_service
)
legacy.RepositoryBoundaryTests.test_r15_stager_enrolls_profile_and_composition_files = (
    test_r15_stager_enrolls_profile_and_composition_files
)


if __name__ == "__main__":
    unittest.main(module=legacy, verbosity=2)

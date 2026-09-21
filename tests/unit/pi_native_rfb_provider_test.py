#!/usr/bin/env python3
"""Host/static proof for A003 R11 selected native Pi RFB provider authority."""

from __future__ import annotations

import hashlib
from pathlib import Path
import subprocess
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]

DROPIN = (
    REPO_ROOT
    / "systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf"
)
SOCKET = REPO_ROOT / "systemd/pi/ps-to-vnc-rfb.socket"
BASE_SERVICE = REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-tigervnc.service"
PERSISTENT = (
    REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service"
)
STAGER = REPO_ROOT / "scripts/pi/install-rfb-activation-units.sh"
INSTALL_TIGERVNC = REPO_ROOT / "scripts/pi/install-tigervnc.sh"
VERIFY_FOUNDATION = REPO_ROOT / "scripts/pi/verify-foundation.sh"
WIRE_SERVER = REPO_ROOT / "pi/wire_server.py"
RFB_RELAY = REPO_ROOT / "pi/rfb_relay.py"
WIRE_UNIT = REPO_ROOT / "systemd/pi/ps-to-vnc-wire.service"

HISTORICAL_NATIVE_DROPIN_SHA256 = (
    "cf09bdf7b374f022b482e52201d8d6bc95c07687fa8fa827cb25ab74e8bcdf4d"
)
HISTORICAL_NATIVE_DROPIN_BYTES = 919


def text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


class NativeProviderAuthorityTests(unittest.TestCase):
    def test_exact_recovered_native_dropin_identity(self) -> None:
        data = DROPIN.read_bytes()
        self.assertEqual(len(data), HISTORICAL_NATIVE_DROPIN_BYTES)
        self.assertEqual(
            hashlib.sha256(data).hexdigest(),
            HISTORICAL_NATIVE_DROPIN_SHA256,
        )

    def test_dropin_selects_existing_lightdm_xorg_display(self) -> None:
        unit = text(DROPIN)

        for required in (
            "Requires=lightdm.service",
            "After=lightdm.service",
            "Environment=DISPLAY=:0",
            "Environment=XAUTHORITY=/home/ps2/.Xauthority",
            "StandardInput=null",
            "StandardOutput=journal",
            "StandardError=journal",
            "ExecStart=\nExecStart=/usr/bin/X0tigervnc ",
            "-display :0",
            "-rfbport -1",
            "-SecurityTypes None",
            "-AlwaysShared=1",
            "-AcceptPointerEvents=1",
            "-AcceptKeyEvents=1",
            "-AcceptSetDesktopSize=0",
            "-UseIPv6=0",
        ):
            self.assertIn(required, unit)

        # The recovered failed logger spelling belongs to Xtigervnc :1 and is
        # intentionally absent from the selected X0 provider override.
        self.assertNotIn("-Log ", unit)

    def test_selected_native_provider_creates_no_competing_listener(self) -> None:
        unit = text(DROPIN)

        self.assertIn("-rfbport -1", unit)
        self.assertNotIn("-rfbport 5900", unit)
        self.assertNotIn("-interface 192.168.50.1", unit)
        self.assertNotIn("127.0.0.1", unit)
        self.assertNotIn("5903", unit)

        socket_unit = text(SOCKET)
        self.assertIn("ListenStream=192.168.50.1:5900", socket_unit)
        self.assertIn("Accept=no", socket_unit)
        self.assertIn("BindToDevice=eth0", socket_unit)
        self.assertIn("FreeBind=yes", socket_unit)
        self.assertIn(
            "Service=ps-to-vnc-rfb-tigervnc.service",
            socket_unit,
        )

    def test_historical_dedicated_provider_definitions_are_preserved(self) -> None:
        base = text(BASE_SERVICE)
        persistent = text(PERSISTENT)

        self.assertIn("/usr/bin/Xtigervnc :1 -inetd", base)
        self.assertIn("StandardInput=socket", base)
        self.assertIn("-Log *:syslog:30", base)

        self.assertIn("/usr/bin/Xtigervnc :1", persistent)
        self.assertIn("-rfbport 5900", persistent)
        self.assertIn("-interface 192.168.50.1", persistent)
        self.assertIn("-Log *:syslog:30", persistent)

        for unit in (base, persistent):
            self.assertNotIn("X0tigervnc", unit)
            self.assertNotIn("DISPLAY=:0", unit)

    def test_stager_is_exact_inactive_and_manager_inert(self) -> None:
        script = text(STAGER)

        for required in (
            "{stage|verify|remove}",
            "90-native-x0vnc.conf",
            "/usr/bin/X0tigervnc",
            "refuse_live_rfb_lifecycle",
            "systemctl is-active --quiet",
            "systemctl is-enabled --quiet",
            'install_file 0644 "$DROPIN_SOURCE" "$DROPIN_DEST"',
            'verify_file 0644 "$DROPIN_SOURCE" "$DROPIN_DEST"',
            'cmp -s "$source" "$target"',
            'SYSTEMD_UNIT_PATH="$TMP_CANDIDATE:" systemd-analyze verify',
            "SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED",
            "RFB_UNIT_ENABLEMENT=NOT_CHANGED",
            "RFB_UNIT_RUNNING_STATE=NOT_CHANGED",
            "LIGHTDM_XORG_STATE=NOT_CHANGED",
        ):
            self.assertIn(required, script)

        for forbidden in (
            "systemctl daemon-reload",
            "systemctl enable ",
            "systemctl disable ",
            "systemctl start ",
            "systemctl stop ",
            "systemctl restart ",
        ):
            self.assertNotIn(forbidden, script)

    def test_selected_x0_package_dependency_is_exact(self) -> None:
        installer = text(INSTALL_TIGERVNC)
        verifier = text(VERIFY_FOUNDATION)

        for source in (installer, verifier):
            self.assertIn(
                "1.15.0+dfsg-2.1~deb13u1",
                source,
            )
            self.assertIn("tigervnc-scraping-server", source)
            self.assertIn("/usr/bin/X0tigervnc", source)

        self.assertIn("tigervnc-standalone-server", installer)
        self.assertIn("tigervnc-standalone-server", verifier)

    def test_changed_shell_tools_parse_cleanly(self) -> None:
        for script in (
            STAGER,
            INSTALL_TIGERVNC,
            VERIFY_FOUNDATION,
        ):
            result = subprocess.run(
                ["bash", "-n", str(script)],
                check=False,
                capture_output=True,
                text=True,
            )
            self.assertEqual(
                result.returncode,
                0,
                msg=f"{script}: {result.stderr}",
            )

    def test_r10_default_wire_service_remains_unattached(self) -> None:
        wire_server = text(WIRE_SERVER)
        relay = text(RFB_RELAY)
        wire_unit = text(WIRE_UNIT)

        self.assertIn("self.serve_connection(connection)", wire_server)
        self.assertNotIn("--rfb-provider", wire_server)
        self.assertNotIn("X0tigervnc", wire_server)
        self.assertNotIn("127.0.0.1:5903", wire_server)

        self.assertNotIn("X0tigervnc", relay)
        self.assertNotIn("127.0.0.1:5903", relay)

        self.assertNotIn("X0tigervnc", wire_unit)
        self.assertNotIn("5900", wire_unit)
        self.assertNotIn("5903", wire_unit)


if __name__ == "__main__":
    unittest.main(verbosity=2)

#!/usr/bin/env python3
"""Host/static proof for A003 R12 Pi-local RFB provider endpoint authority."""

from __future__ import annotations

import hashlib
from pathlib import Path
import subprocess
import unittest

REPO_ROOT = Path(__file__).resolve().parents[2]

INTERNAL_SOCKET = REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-internal.socket"
INTERNAL_SERVICE = (
    REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-internal-x0tigervnc.service"
)
INTERNAL_STAGER = (
    REPO_ROOT / "scripts/pi/install-rfb-internal-provider-units.sh"
)

DIRECT_SOCKET = REPO_ROOT / "systemd/pi/ps-to-vnc-rfb.socket"
DIRECT_SERVICE = REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-tigervnc.service"
DIRECT_PERSISTENT = (
    REPO_ROOT / "systemd/pi/ps-to-vnc-rfb-tigervnc-persistent.service"
)
DIRECT_DROPIN = (
    REPO_ROOT
    / "systemd/pi/ps-to-vnc-rfb-tigervnc.service.d/90-native-x0vnc.conf"
)

WIRE_SERVER = REPO_ROOT / "pi/wire_server.py"
RFB_RELAY = REPO_ROOT / "pi/rfb_relay.py"
WIRE_PROTOCOL = REPO_ROOT / "pi/wire_protocol.py"
WIRE_UNIT = REPO_ROOT / "systemd/pi/ps-to-vnc-wire.service"

DIRECT_UNIT_BLOBS = {
    DIRECT_SOCKET: "e24e4e4f389c155c7e6eff0fbd93c4c01602a361",
    DIRECT_SERVICE: "bb2e02bda68d40baa4b8c5152c19dbe825978038",
    DIRECT_PERSISTENT: "64874d9c5018d75e330140d3d833f5bb9ccd28d4",
    DIRECT_DROPIN: "4dfbdbe7a3fc8ef4964c5704a337631482296cbb",
}

R10_PRODUCT_BLOBS = {
    WIRE_SERVER: "d1accd8b3b786bea4969ca8a74ecdb03c548eddc",
    RFB_RELAY: "55e946f2cc8e7d449b83616a6fe8653695c06702",
    WIRE_PROTOCOL: "a136640272ab942ce3e15096ff9da2089491745d",
    WIRE_UNIT: "316c6d2eafdfdf5ecd8c89678e2dfe227e709556",
}

DIRECT_ALTERNATIVES = {
    "ps-to-vnc-rfb.socket",
    "ps-to-vnc-rfb-tigervnc.service",
    "ps-to-vnc-rfb-tigervnc-persistent.service",
}


def text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def git_blob_sha1(path: Path) -> str:
    data = path.read_bytes()
    header = f"blob {len(data)}\0".encode("ascii")
    return hashlib.sha1(header + data).hexdigest()


def directive_values(path: Path, key: str) -> list[str]:
    prefix = f"{key}="
    values: list[str] = []

    for raw_line in text(path).splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or not line.startswith(prefix):
            continue
        values.extend(line[len(prefix) :].split())

    return values


def nonempty_execstart(path: Path) -> list[str]:
    prefix = "ExecStart="
    values: list[str] = []
    for raw_line in text(path).splitlines():
        line = raw_line.strip()
        if line.startswith(prefix) and line != prefix:
            values.append(line[len(prefix) :])
    return values


class InternalProviderAuthorityTests(unittest.TestCase):
    def test_internal_socket_is_loopback_only_and_provider_local(self) -> None:
        self.assertEqual(
            directive_values(INTERNAL_SOCKET, "ListenStream"),
            ["127.0.0.1:5900"],
        )
        self.assertEqual(
            directive_values(INTERNAL_SOCKET, "Accept"),
            ["no"],
        )
        self.assertEqual(
            directive_values(INTERNAL_SOCKET, "Service"),
            ["ps-to-vnc-rfb-internal-x0tigervnc.service"],
        )

        selected_directives = "\n".join(
            line
            for line in text(INTERNAL_SOCKET).splitlines()
            if line and not line.lstrip().startswith("#")
        )
        self.assertNotIn("0.0.0.0", selected_directives)
        self.assertNotIn("[::]", selected_directives)
        self.assertNotIn("192.168.50.1", selected_directives)
        self.assertNotIn("BindToDevice=eth0", selected_directives)
        self.assertNotIn("5903", selected_directives)

    def test_internal_service_reuses_exact_r11_x0_provider_policy(self) -> None:
        service = text(INTERNAL_SERVICE)

        for required in (
            "Requires=ps-to-vnc-rfb-internal.socket lightdm.service",
            "After=ps-to-vnc-rfb-internal.socket lightdm.service",
            "Environment=DISPLAY=:0",
            "Environment=XAUTHORITY=/home/ps2/.Xauthority",
            "StandardInput=null",
            "StandardOutput=journal",
            "StandardError=journal",
            "/usr/bin/X0tigervnc",
            "-display :0",
            "-rfbport -1",
            "-SecurityTypes None",
            "-AlwaysShared=1",
            "-AcceptPointerEvents=1",
            "-AcceptKeyEvents=1",
            "-AcceptSetDesktopSize=0",
            "-UseIPv6=0",
        ):
            self.assertIn(required, service)

        self.assertEqual(
            nonempty_execstart(INTERNAL_SERVICE),
            nonempty_execstart(DIRECT_DROPIN),
        )
        self.assertNotIn("-Log ", service)
        self.assertNotIn("5903", service)

    def test_internal_units_make_direct_and_internal_paths_mutually_exclusive(self) -> None:
        for unit in (INTERNAL_SOCKET, INTERNAL_SERVICE):
            self.assertEqual(
                set(directive_values(unit, "Conflicts")),
                DIRECT_ALTERNATIVES,
            )
            self.assertTrue(
                DIRECT_ALTERNATIVES.issubset(
                    set(directive_values(unit, "After"))
                )
            )

    def test_r11_direct_and_historical_units_are_byte_identical(self) -> None:
        for path, expected_blob in DIRECT_UNIT_BLOBS.items():
            self.assertEqual(
                git_blob_sha1(path),
                expected_blob,
                msg=str(path),
            )

    def test_internal_stager_is_separate_exact_and_manager_inert(self) -> None:
        script = text(INTERNAL_STAGER)

        for required in (
            "{stage|verify|remove}",
            "INTERNAL_SOCKET_NAME='ps-to-vnc-rfb-internal.socket'",
            "INTERNAL_SERVICE_NAME='ps-to-vnc-rfb-internal-x0tigervnc.service'",
            "INTERNAL_SOCKET_DEST=\"$DEST_DIR/$INTERNAL_SOCKET_NAME\"",
            "INTERNAL_SERVICE_DEST=\"$DEST_DIR/$INTERNAL_SERVICE_NAME\"",
            "/usr/bin/X0tigervnc",
            "refuse_live_internal_lifecycle",
            "systemctl is-active --quiet",
            "systemctl is-enabled --quiet",
            'install_file 0644 "$INTERNAL_SOCKET_SOURCE" "$INTERNAL_SOCKET_DEST"',
            'install_file 0644 "$INTERNAL_SERVICE_SOURCE" "$INTERNAL_SERVICE_DEST"',
            'verify_file 0644 "$INTERNAL_SOCKET_SOURCE" "$INTERNAL_SOCKET_DEST"',
            'verify_file 0644 "$INTERNAL_SERVICE_SOURCE" "$INTERNAL_SERVICE_DEST"',
            'SYSTEMD_UNIT_PATH="$TMP_CANDIDATE:" systemd-analyze verify',
            "DIRECT_RFB_AUTHORITY_STAGED_BY_THIS_TOOL=NO",
            "SYSTEMD_MANAGER_RELOAD=NOT_PERFORMED",
            "RFB_INTERNAL_ENABLEMENT=NOT_CHANGED",
            "RFB_INTERNAL_RUNNING_STATE=NOT_CHANGED",
            "LIGHTDM_XORG_STATE=NOT_CHANGED",
            "RFB_INTERNAL_PROVIDER_REMOVE=PASS",
        ):
            self.assertIn(required, script)

        for forbidden in (
            'install_file 0644 "$DIRECT_',
            'verify_file 0644 "$DIRECT_',
            "systemctl daemon-reload",
            "systemctl enable ",
            "systemctl disable ",
            "systemctl start ",
            "systemctl stop ",
            "systemctl restart ",
            "127.0.0.1:5903",
        ):
            self.assertNotIn(forbidden, script)

    def test_internal_stager_static_validation_includes_both_alternatives(self) -> None:
        script = text(INTERNAL_STAGER)

        for required in (
            'cp "$INTERNAL_SOCKET_SOURCE" "$TMP_CANDIDATE/$INTERNAL_SOCKET_NAME"',
            'cp "$INTERNAL_SERVICE_SOURCE" "$TMP_CANDIDATE/$INTERNAL_SERVICE_NAME"',
            'cp "$DIRECT_SOCKET_SOURCE" "$TMP_CANDIDATE/$DIRECT_SOCKET_NAME"',
            'cp "$DIRECT_SERVICE_SOURCE" "$TMP_CANDIDATE/$DIRECT_SERVICE_NAME"',
            'cp "$DIRECT_PERSISTENT_SOURCE" "$TMP_CANDIDATE/$DIRECT_PERSISTENT_NAME"',
            'cp "$DIRECT_DROPIN_SOURCE" "$TMP_CANDIDATE/$DIRECT_DROPIN_RELATIVE"',
            '"$INTERNAL_SOCKET_NAME"',
            '"$INTERNAL_SERVICE_NAME"',
            '"$DIRECT_SOCKET_NAME"',
            '"$DIRECT_SERVICE_NAME"',
            '"$DIRECT_PERSISTENT_NAME"',
        ):
            self.assertIn(required, script)

    def test_internal_stager_shell_syntax_is_clean(self) -> None:
        result = subprocess.run(
            ["bash", "-n", str(INTERNAL_STAGER)],
            check=False,
            capture_output=True,
            text=True,
        )
        self.assertEqual(
            result.returncode,
            0,
            msg=result.stderr,
        )

    def test_r10_wire_and_relay_remain_byte_identical_and_unattached(self) -> None:
        for path, expected_blob in R10_PRODUCT_BLOBS.items():
            self.assertEqual(
                git_blob_sha1(path),
                expected_blob,
                msg=str(path),
            )

        wire_server = text(WIRE_SERVER)
        relay = text(RFB_RELAY)
        wire_unit = text(WIRE_UNIT)

        self.assertIn("self.serve_connection(connection)", wire_server)

        for source in (wire_server, relay, wire_unit):
            self.assertNotIn("127.0.0.1:5900", source)
            self.assertNotIn("127.0.0.1:5903", source)
            self.assertNotIn("ps-to-vnc-rfb-internal", source)
            self.assertNotIn("X0tigervnc", source)

    def test_selected_internal_authority_never_uses_development_5903(self) -> None:
        for path in (INTERNAL_SOCKET, INTERNAL_SERVICE, INTERNAL_STAGER):
            self.assertNotIn("5903", text(path), msg=str(path))


if __name__ == "__main__":
    unittest.main(verbosity=2)

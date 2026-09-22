#!/usr/bin/env python3
"""Host contract for A003 R14's canonical and Pi RFB profile projections."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
PI = ROOT / "pi"
if str(PI) not in sys.path:
    sys.path.insert(0, str(PI))

import rfb_attachment
import rfb_runtime_profile
import rfb_runtime_profile_generated as generated

CANONICAL = ROOT / "src/config/rfb_runtime_profile.json"
GENERATOR = ROOT / "scripts/generate-rfb-runtime-profile.py"


class CanonicalRfbRuntimeProfileTests(unittest.TestCase):
    def test_canonical_selected_values_and_provenance(self) -> None:
        profile = json.loads(CANONICAL.read_text(encoding="utf-8"))
        self.assertEqual(profile["rfb_mode"], "ON")
        self.assertEqual(profile["rfb_window_bytes"], 32768)
        self.assertEqual(profile["rfb_credit_batch_bytes"], 8192)
        self.assertTrue(profile["rfb_credit_flush_on_empty"])
        self.assertTrue(profile["rfb_credit_return_enabled"])
        self.assertEqual(profile["receiver_thread_priority"], 63)
        self.assertEqual(profile["receiver_thread_stack_size"], 16384)
        self.assertEqual(profile["max_data_payload"], 8192)
        self.assertEqual(profile["provenance"]["baseline_checkpoint"], "CP2N")
        self.assertFalse(
            profile["provenance"]["pi_provider_write_capacity_hardware_qualified"]
        )
        self.assertNotIn("session_id", profile)
        self.assertNotIn("audio_mode", profile)
        self.assertNotIn("mpeg", profile)

    def test_pi_projection_uses_one_window_and_off_is_absent(self) -> None:
        flow = rfb_runtime_profile.selected_rfb_flow_config()
        self.assertIsInstance(flow, rfb_attachment.RfbFlowConfig)
        assert flow is not None
        self.assertEqual(flow.provider_read_credit_limit, 32768)
        self.assertEqual(flow.provider_write_capacity, 32768)
        self.assertEqual(flow.max_data_payload, 8192)
        self.assertEqual(
            flow.provider_write_capacity, flow.provider_read_credit_limit
        )
        self.assertIsNone(rfb_runtime_profile.project_rfb_flow_config("OFF"))
        with self.assertRaises(ValueError):
            rfb_runtime_profile.project_rfb_flow_config("VISIBLE")

    def test_generated_projection_is_current_and_has_no_session_identity(self) -> None:
        completed = subprocess.run(
            [sys.executable, str(GENERATOR), "--check"],
            cwd=ROOT, check=False, capture_output=True, text=True,
        )
        self.assertEqual(completed.returncode, 0, completed.stderr)
        self.assertEqual(generated.RFB_MODE, "ON")
        self.assertEqual(generated.RFB_WINDOW_BYTES, 32768)
        for path in (
            ROOT / "src/config/rfb_runtime_profile_generated.h",
            ROOT / "pi/rfb_runtime_profile_generated.py",
        ):
            self.assertNotIn(
                "session_id", path.read_text(encoding="utf-8").lower()
            )

    def test_canonical_change_makes_checked_in_projection_stale(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            temp_root = Path(tmp)
            for relative in (
                Path("src/config/rfb_runtime_profile.json"),
                Path("src/config/rfb_runtime_profile_generated.h"),
                Path("pi/rfb_runtime_profile_generated.py"),
            ):
                destination = temp_root / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(ROOT / relative, destination)

            first = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(first.returncode, 0, first.stderr)

            data = json.loads(
                (temp_root / "src/config/rfb_runtime_profile.json").read_text(
                    encoding="utf-8"
                )
            )
            changed = copy.deepcopy(data)
            changed["rfb_window_bytes"] += 1
            (temp_root / "src/config/rfb_runtime_profile.json").write_text(
                json.dumps(changed, indent=2) + "\n", encoding="utf-8"
            )

            stale = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertNotEqual(stale.returncode, 0)
            self.assertIn("RFB_PROFILE_GENERATED_STALE=", stale.stderr)

            regenerate = subprocess.run(
                [sys.executable, str(GENERATOR), "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(regenerate.returncode, 0, regenerate.stderr)
            final = subprocess.run(
                [sys.executable, str(GENERATOR), "--check", "--root", str(temp_root)],
                check=False, capture_output=True, text=True,
            )
            self.assertEqual(final.returncode, 0, final.stderr)

    def test_default_runtime_activation_remains_absent(self) -> None:
        service = (
            ROOT / "systemd/pi/ps-to-vnc-wire.service"
        ).read_text(encoding="utf-8")
        wire_server = (ROOT / "pi/wire_server.py").read_text(encoding="utf-8")
        app = (ROOT / "src/app.c").read_text(encoding="utf-8")
        self.assertNotIn("rfb_runtime_profile", service)
        self.assertNotIn("rfb_runtime_profile", wire_server)
        self.assertNotIn("rfb_runtime_profile", app)
        self.assertIn("self.serve_connection(connection)", wire_server)
        self.assertIn("return pstvnc_app_run_with_transport_config(NULL);", app)


if __name__ == "__main__":
    unittest.main(verbosity=2)

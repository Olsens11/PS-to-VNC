#!/usr/bin/env python3
"""R31 source-boundary proof for read-only management config retrieval."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
MANAGEMENT_C = ROOT / "src/management/config_get.c"
MANAGEMENT_H = ROOT / "src/management/config_get.h"
PLATFORM_C = ROOT / "src/platform/ps2_network.c"
PLATFORM_H = ROOT / "src/platform/ps2_network.h"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


management_source = MANAGEMENT_C.read_text(encoding="utf-8")
management_header = MANAGEMENT_H.read_text(encoding="utf-8")
platform_source = PLATFORM_C.read_text(encoding="utf-8")
platform_header = PLATFORM_H.read_text(encoding="utf-8")
management_code = strip_comments(management_source + "\n" + management_header)
platform_code = strip_comments(platform_source + "\n" + platform_header)

for required in (
    '"GET /ps2vnc.conf HTTP/1.0\\r\\n"',
    '"Host: 192.168.50.1\\r\\n"',
    '"Connection: close\\r\\n"',
    "pstvnc_ps2_network_connect_management",
    "pstvnc_ps2_network_close",
    "PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES 1024u",
    "PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES 4096u",
):
    require(required in management_code, f"missing R31 mechanism: {required}")

require(
    "PSTVNC_PS2_MANAGEMENT_SERVER_PORT 5959" in platform_code,
    "management endpoint must remain fixed at TCP 5959",
)
require(
    'PSTVNC_PS2_MANAGEMENT_SERVER_IP "192.168.50.1"' in platform_code,
    "management endpoint must remain on the private Pi address",
)
require(
    re.search(
        r"pstvnc_ps2_network_connect_management\s*\([^)]*\)\s*\{.*?"
        r"PSTVNC_PS2_MANAGEMENT_SERVER_IP.*?"
        r"PSTVNC_PS2_MANAGEMENT_SERVER_PORT.*?\}",
        platform_code,
        flags=re.DOTALL,
    ) is not None,
    "management connect seam must use only the management endpoint constants",
)
require(
    re.search(
        r"pstvnc_ps2_network_connect_pstv\s*\([^)]*\)\s*\{.*?"
        r"PSTVNC_PS2_PSTV_SERVER_IP.*?PSTVNC_PS2_PSTV_SERVER_PORT.*?\}",
        platform_code,
        flags=re.DOTALL,
    ) is not None,
    "existing PSTV connect seam must retain its own endpoint authority",
)

for forbidden in (
    "pstvnc_ps2_network_connect_pstv",
    "PSTVNC_PS2_PSTV_SERVER_PORT",
    "pstvnc_config_product_action_bindings_parse",
    "pstvnc_input_runtime",
    "pstvnc_app_",
    "pstvnc_local_ui",
    "pstvnc_local_controller",
    "pstvnc_rfb_",
    "pstvnc_transport_",
    "pstvnc_mpeg_",
    "pstvnc_media_clock_",
    "pstvnc_audio_",
    '"POST ',
    "fopen(",
    "fwrite(",
    "malloc(",
    "free(",
    "select(",
    "SleepThread(",
    "SetAlarm(",
    "mailbox",
    "watchdog",
):
    require(forbidden not in management_code, f"R31 scope creep: {forbidden}")

for forbidden_header in (
    "config/product_action_bindings.h",
    "input/",
    "ui/",
    "rfb/",
    "transport/",
    "mpeg/",
    "media/clock.h",
    "audio/",
    "app.h",
):
    require(
        forbidden_header not in management_code,
        f"management gained forbidden owner dependency: {forbidden_header}",
    )

require(
    management_code.count("pstvnc_management_config_get") == 2,
    "R31 public management surface must remain one declaration plus definition",
)
require(
    management_code.count("pstvnc_ps2_network_connect_management") == 1,
    "R31 should consume exactly one management connect call",
)
require(
    "body_length = 0" not in management_code and
    "*body_length = 0" not in management_code,
    "R31 must not publish a failure-side zero length",
)

print("MANAGEMENT_CONFIG_GET_SOURCE_TEST=PASS")

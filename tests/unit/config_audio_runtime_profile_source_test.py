#!/usr/bin/env python3
"""R36 source-boundary proof for selected AUDIO profile authority."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/config/audio_runtime_profile.c").read_text(encoding="utf-8")
HEADER = (ROOT / "src/config/audio_runtime_profile.h").read_text(encoding="utf-8")
GENERATED = (ROOT / "src/config/audio_runtime_profile_generated.h").read_text(encoding="utf-8")
JSON_TEXT = (ROOT / "src/config/audio_runtime_profile.json").read_text(encoding="utf-8")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)


source = strip_comments(SOURCE)
header = strip_comments(HEADER)
combined = source + "\n" + header

for required in (
    "pstvnc_transport_audio_channel_config_t transport;",
    "pstvnc_config_pcm_profile_t pcm;",
    "pstvnc_audio_session_values_t session;",
):
    if required not in header:
        raise AssertionError(f"R36 lost existing owner type aggregation: {required}")

if "audio_presentation_offset" in source or    "audio_presentation_offset" in header or    "AUDIO_PRESENTATION_OFFSET" in GENERATED:
    raise AssertionError("R36 duplicated R26 media-clock offset authority")

for forbidden in (
    "malloc(",
    "calloc(",
    "free(",
    "CreateThread",
    "StartThread",
    "CreateSema",
    "audsrv_",
    "pstvnc_transport_session_open",
    "pstvnc_transport_access_acquire",
    "pstvnc_media_clock_arm",
    "pstvnc_media_clock_wait_audio",
    "pstvnc_app_",
    "pstvnc_ps2_",
):
    if forbidden in combined:
        raise AssertionError(f"R36 profile access acquired runtime side effect: {forbidden}")

for forbidden in (
    '"P11_COMPAT_PLUS_PCM"',
    '"3426f28b93de9519ca93e5f0e0aaf8b67cfca845"',
):
    if forbidden in source or forbidden in header:
        raise AssertionError("forensic identity leaked into runtime C authority")

if '"profile_id": "P11_COMPAT_PLUS_PCM"' in JSON_TEXT:
    raise AssertionError("H1 profile ID became production profile identity")

if "PSTVNC_CONFIG_AUDIO_RESERVOIR_POLL_US" not in GENERATED or    "PSTVNC_CONFIG_AUDIO_CLOCK_POLL_US" not in GENERATED:
    raise AssertionError("independent clean poll fields are missing")

print("CONFIG_AUDIO_RUNTIME_PROFILE_SOURCE_TEST=PASS")

#!/usr/bin/env python3
"""R37 source/build boundary proof for the PS2 AUDIO execution binding."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/audio/ps2_runtime.c").read_text(encoding="utf-8")
HEADER = (ROOT / "src/audio/ps2_runtime.h").read_text(encoding="utf-8")
MAKEFILE = (ROOT / "mk/issue7-clean.mk").read_text(encoding="utf-8")
APP = "\n".join(path.read_text(encoding="utf-8") for path in ROOT.glob("src/app*.c"))

def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*?$", "", text, flags=re.MULTILINE)

code = strip_comments(SOURCE + "\n" + HEADER)

for required in (
    'SifLoadModule("rom0:LIBSD"',
    "SifExecModuleBuffer(",
    "AUDSRV_irx",
    "size_AUDSRV_irx",
    "CreateSema(",
    "WaitSema(",
    "SignalSema(",
    "CreateThread(",
    "StartThread(",
    "ReferThreadStatus(",
    "THS_DORMANT",
    "DeleteThread(",
):
    if required not in code:
        raise AssertionError(f"R37 missing required PS2 mechanism: {required}")

for forbidden in (
    "audsrv_quit",
    "SifIopReset",
    "TerminateThread",
    "join_poll_max_count",
    "pstvnc_media_clock_time_ops_t",
    "GetTimerSystemTime",
    "kBUSCLK",
):
    if forbidden in code:
        raise AssertionError(f"R37 crossed a prohibited owner/reclaim boundary: {forbidden}")

if "pstvnc_audio_ps2_runtime" in APP or "pstvnc_audio_ps2_resident" in APP:
    raise AssertionError("R37 entered ordinary Application AUDIO composition")

for required in (
    "$(PS2SDK)/iop/irx/audsrv.irx",
    "bin2c $< $@ AUDSRV_irx",
    "$(BUILD_DIR)/AUDSRV_irx.o",
    "$(BUILD_DIR)/audio_ps2_runtime.o",
):
    if required not in MAKEFILE:
        raise AssertionError(f"R37 clean build lost pinned AUDSRV/runtime input: {required}")

if "PSTVNC_AUDIO_PS2_RUNTIME_MAX_ALLOCATIONS 2u" not in HEADER:
    raise AssertionError("R37 allocation ownership is not bounded to the AUDIO session contract")

print("AUDIO_PS2_RUNTIME_SOURCE_TEST=PASS")

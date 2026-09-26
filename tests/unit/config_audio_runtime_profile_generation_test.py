#!/usr/bin/env python3
"""R36 deterministic AUDIO profile generation/validation proof."""

import copy
import importlib.util
import json
from pathlib import Path
import tempfile

ROOT = Path(__file__).resolve().parents[2]
GEN = ROOT / "scripts/generate-audio-runtime-profile.py"

spec = importlib.util.spec_from_file_location("audio_profile_generator", GEN)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

profile = json.loads((ROOT / module.SOURCE_PATH).read_text(encoding="utf-8"))
module.validate_profile(copy.deepcopy(profile))

cases = []

def bad(path, value):
    candidate = copy.deepcopy(profile)
    target = candidate
    for key in path[:-1]:
        target = target[key]
    target[path[-1]] = value
    cases.append(candidate)

bad(("transport","queue_capacity"), 0)
bad(("transport","initial_credit_bytes"), 524289)
bad(("transport","credit_batch_bytes"), 524289)
bad(("transport","credit_flush_on_empty"), 2)
bad(("transport","credit_return_enabled"), False)
bad(("pcm","rate_hz"), 0)
bad(("pcm","channels"), 3)
bad(("pcm","bits_per_sample"), 24)
bad(("pcm","volume_percent"), 101)
bad(("session","playback_buffer_capacity"), 0)
bad(("session","startup_reservoir_bytes"), 524289)
bad(("session","worker_priority"), 0)
bad(("session","worker_stack_bytes"), 0x80000000)
bad(("session","reservoir_poll_us"), 0)
bad(("session","clock_poll_us"), 0)
bad(("provenance","qualified_lineage"), "other")

for index, candidate in enumerate(cases):
    try:
        module.validate_profile(candidate)
    except ValueError:
        pass
    else:
        raise AssertionError(f"invalid AUDIO profile case {index} was accepted")

rendered = module.render_c(profile)
if "AUDIO_PRESENTATION_OFFSET" in rendered or "audio_presentation_offset" in rendered:
    raise AssertionError("R36 duplicated media-clock offset into AUDIO profile")

if "PSTVNC_CONFIG_AUDIO_RESERVOIR_POLL_US" not in rendered or    "PSTVNC_CONFIG_AUDIO_CLOCK_POLL_US" not in rendered:
    raise AssertionError("R36 poll cadences are not independently represented")

if not module.check_output(ROOT):
    raise AssertionError("checked-in AUDIO generated projection is stale")

print("CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS")

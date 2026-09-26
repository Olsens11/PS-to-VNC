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
pi_rendered = module.render_pi(profile)
if "AUDIO_PRESENTATION_OFFSET" in rendered or "audio_presentation_offset" in rendered:
    raise AssertionError("R36 duplicated media-clock offset into AUDIO profile")

if "PSTVNC_CONFIG_AUDIO_RESERVOIR_POLL_US" not in rendered or    "PSTVNC_CONFIG_AUDIO_CLOCK_POLL_US" not in rendered:
    raise AssertionError("R36 poll cadences are not independently represented")

for required in (
    "CHANNEL_WINDOW_BYTES = 524288",
    "PCM_RATE_HZ = 48000",
    "PCM_CHANNELS = 2",
    "PCM_BITS_PER_SAMPLE = 16",
    "PCM_FRAME_BYTES = PCM_CHANNELS * (PCM_BITS_PER_SAMPLE // 8)",
):
    if required not in pi_rendered:
        raise AssertionError(f"R39 Pi AUDIO projection missing: {required}")

if not module.check_output(ROOT):
    raise AssertionError("checked-in AUDIO generated projection is stale")


with tempfile.TemporaryDirectory() as temp_name:
    temp_root = Path(temp_name)
    source = temp_root / module.SOURCE_PATH
    c_output = temp_root / module.OUTPUT_PATH
    pi_output = temp_root / module.PI_OUTPUT_PATH
    source.parent.mkdir(parents=True, exist_ok=True)
    c_output.parent.mkdir(parents=True, exist_ok=True)
    pi_output.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(json.dumps(profile), encoding="utf-8")
    c_output.write_text(module.render_c(profile), encoding="utf-8")
    pi_output.write_text("# stale\n", encoding="utf-8")
    if module.check_output(temp_root):
        raise AssertionError("stale generated Pi AUDIO projection was accepted")

print("CONFIG_AUDIO_RUNTIME_PROFILE_GENERATION_TEST=PASS")

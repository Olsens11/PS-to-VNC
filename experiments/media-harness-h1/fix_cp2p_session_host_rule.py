#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[2]
path = root / 'experiments/media-harness-h1/mpeg_presentation_calibration/Makefile.host'
text = path.read_text()
old = "-I. -I.. -Ihost_stubs -I../../audio-transport/common -I../../../src/input -I../../../src/framebuffer -I../../../src/rfb $(RECALIBRATION_SOURCES)"
new = "-I. -I.. -Ihost_stubs -I../../audio-transport/common -I../../../src -I../../../src/input -I../../../src/ui -I../../../src/framebuffer -I../../../src/rfb -I../../../src/display -I../../../src/platform $(RECALIBRATION_SOURCES)"
if old not in text:
    raise SystemExit('session coordinator host-rule anchor missing')
path.write_text(text.replace(old, new, 1))
Path(__file__).unlink()

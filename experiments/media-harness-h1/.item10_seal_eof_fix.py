#!/usr/bin/env python3
"""Normalize the one-shot item-#10 history append to exactly one trailing newline."""
from pathlib import Path

path = Path("experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md")
text = path.read_text()
path.write_text(text.rstrip() + "\n")
print("H1_CP2P_ITEM10_HISTORY_EOF=NORMALIZED")

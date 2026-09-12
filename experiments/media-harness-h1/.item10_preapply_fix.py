#!/usr/bin/env python3
"""Tighten one ambiguous constructor anchor in the disposable item-10 patcher."""
from pathlib import Path

path = Path("experiments/media-harness-h1/.item10_apply_existing.py")
text = path.read_text()
old = '''s = replace_once(s, "        self._emission_in_flight = 0\\n", "        self._emission_in_flight = 0\\n        self._emission_retiring = False\\n", "producer latch init")'''
new = '''s = replace_once(s, "        self._emission_condition = threading.Condition()\\n        self._emission_open = False\\n        self._emission_in_flight = 0\\n", "        self._emission_condition = threading.Condition()\\n        self._emission_open = False\\n        self._emission_in_flight = 0\\n        self._emission_retiring = False\\n", "producer latch init")'''
if text.count(old) != 1:
    raise SystemExit(f"item10 preapply anchor count={text.count(old)}")
path.write_text(text.replace(old, new, 1))
print("H1_CP2P_ITEM10_PREAPPLY_FIX=PASS")

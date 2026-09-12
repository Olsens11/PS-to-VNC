#!/usr/bin/env python3
"""Tighten item #11B one-shot patch idempotency markers before applying it."""

from pathlib import Path

path = Path("experiments/media-harness-h1/.item11b_patch.py")
text = path.read_text()

replacements = (
    (
        '''    "h1_cp2p_session_clear_mpeg_after_pi_retire",\n    "coordinator finalize callback",\n''',
        '''    "coordinator->pi_retire_pending\\n                ? h1_cp2p_session_clear_mpeg_after_pi_retire",\n    "coordinator finalize callback",\n''',
        "coordinator callback marker",
    ),
    (
        '''    "self._emission_condition.notify_all()",\n    "producer retirement fence reset",\n''',
        '''    "self.archive_path = None\\n        with self._emission_condition:\\n            self._emission_open = False",\n    "producer retirement fence reset",\n''',
        "producer successful-retire reset marker",
    ),
    (
        '''    "transport_generation != generation",\n    "coordinator test retire begin exact generation",\n''',
        '''    "retire_begin_count != 0u ||\\n        transport_generation != generation",\n    "coordinator test retire begin exact generation",\n''',
        "coordinator retire-begin test marker",
    ),
)

changed = False
for old, new, label in replacements:
    if new in text:
        continue
    if text.count(old) != 1:
        raise SystemExit(f"{label}: expected one broad marker, found {text.count(old)}")
    text = text.replace(old, new, 1)
    changed = True

if changed:
    path.write_text(text)
    print("H1_CP2P_ITEM11B_PATCH_FIX=APPLIED")
else:
    print("H1_CP2P_ITEM11B_PATCH_FIX=ALREADY_APPLIED")

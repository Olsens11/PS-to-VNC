#!/usr/bin/env python3
"""Tighten ambiguous anchors in the disposable item-10 patcher."""
from pathlib import Path

path = Path("experiments/media-harness-h1/.item10_apply_existing.py")
text = path.read_text()

replacements = [
    (
        '''s = replace_once(s, "        self._emission_in_flight = 0\\n", "        self._emission_in_flight = 0\\n        self._emission_retiring = False\\n", "producer latch init")''',
        '''s = replace_once(s, "        self._emission_condition = threading.Condition()\\n        self._emission_open = False\\n        self._emission_in_flight = 0\\n", "        self._emission_condition = threading.Condition()\\n        self._emission_open = False\\n        self._emission_in_flight = 0\\n        self._emission_retiring = False\\n", "producer latch init")''',
        "producer latch init",
    ),
    (
        '''s = replace_once(s, '        if int(profile["rfb_mode"]) != 2:\\n            raise base.ProtocolError("CP2O runner requires visible RFB mode 2")\\n        if int(profile["video_mode"]) != 0:\\n            raise base.ProtocolError("CP2O runner requires MPEG/video OFF")\\n        if int(profile["audio_mode"]) not in (0, base.AUDIO_PCM):\\n            raise base.ProtocolError("CP2O runner permits only audio OFF or PCM")\\n\\n', '        self._validate_composition_policy(profile)\\n\\n', "CP2O policy dispatch")''',
        '''s = replace_once(s, '        self.rfb_session_adapter: H1RfbSessionAdapter | None = None\\n\\n        if int(profile["rfb_mode"]) != 2:\\n            raise base.ProtocolError("CP2O runner requires visible RFB mode 2")\\n        if int(profile["video_mode"]) != 0:\\n            raise base.ProtocolError("CP2O runner requires MPEG/video OFF")\\n        if int(profile["audio_mode"]) not in (0, base.AUDIO_PCM):\\n            raise base.ProtocolError("CP2O runner permits only audio OFF or PCM")\\n\\n', '        self.rfb_session_adapter: H1RfbSessionAdapter | None = None\\n\\n        self._validate_composition_policy(profile)\\n\\n', "CP2O policy dispatch")''',
        "CP2O policy dispatch",
    ),
]

for old, new, label in replacements:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"item10 preapply {label} anchor count={count}")
    text = text.replace(old, new, 1)

path.write_text(text)
print("H1_CP2P_ITEM10_PREAPPLY_FIX=PASS")

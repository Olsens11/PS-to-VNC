#!/usr/bin/env python3
"""Correct the staged item #8 patch against the sealed #11A test vocabulary."""

from pathlib import Path

path = Path("experiments/media-harness-h1/.item8_patch.py")
text = path.read_text()

old = '''insert_marker = "    def test_wrong_generation_or_session_does_not_mutate_state(self) -> None:\\n"\nnew_test = r\'''    def test_live_generation_producer_retires_before_suppression_and_ack(self) -> None:\n        producer = FakeGenerationProducer(0, self.bridge)\n        receiver = H1Cp2pSuppressionStartReceiver(\n            self.session,\n            self.bridge,\n            producer,\n        )\n        frame = base.Frame(\n            base.FRAME_DATA,\n            base.CHANNEL_MPEG2,\n            0,\n            1,\n            start_payload(self.session.profile["session_id"], 8),\n        )\n        receiver._handle_start_frame(frame)\n        self.assertEqual(producer.active_generation(), 8)\n        self.assertEqual(self.bridge.suppression_generation, 8)\n\n        receiver._handle_retire_frame(self.retire_frame(8))\n        self.assertEqual(producer.retired, [8])\n        self.assertEqual(self.bridge.suppression_generation, 0)\n        self.assertEqual(len(self.session.sent), 1)\n        self.assertEqual(self.session.sent[0][0], base.FRAME_MPEG_RETIRE)\n\n\'''\nif insert_marker not in retirement:\n    raise SystemExit("retirement test marker missing")\nretirement = retirement.replace(insert_marker, new_test + insert_marker, 1)\n'''

new = '''insert_marker = "    def test_wrong_generation_never_cleans_or_acknowledges(self) -> None:\\n"\nnew_test = r\'''    def test_live_generation_producer_retires_before_suppression_and_ack(self) -> None:\n        with tempfile.TemporaryDirectory() as temp_dir:\n            bridge_sock, peer = socket.socketpair()\n            bridge = H1Cp2pRfbPiBridge(\n                bridge_sock,\n                queue_capacity=1024 * 1024,\n                max_payload=8192,\n                send_data=lambda payload: None,\n                desktop_width=640,\n                desktop_height=448,\n            )\n            session = FakeSession(Path(temp_dir))\n            producer = FakeGenerationProducer(0, bridge)\n            receiver = H1Cp2pSuppressionStartReceiver(session, bridge, producer)\n            try:\n                receiver._handle_start_frame(\n                    make_start_frame(session.profile["session_id"], 8)\n                )\n                self.assertEqual(producer.active_generation(), 8)\n                self.assertEqual(bridge.suppression_generation, 8)\n\n                receiver._handle_retire_frame(\n                    make_retire_frame(session.profile["session_id"], 8)\n                )\n                self.assertEqual(producer.retired, [8])\n                self.assertEqual(bridge.suppression_generation, 0)\n                self.assertEqual(len(session.sent), 1)\n                self.assertEqual(session.sent[0][0], MPEG_RETIRE_FRAME_KIND)\n            finally:\n                bridge.stop()\n                peer.close()\n\n\'''\nif insert_marker not in retirement:\n    raise SystemExit("retirement test marker missing")\nretirement = retirement.replace(insert_marker, new_test + insert_marker, 1)\n'''

if text.count(old) != 1:
    raise SystemExit("item8 staged retirement block no longer matches expected draft")
text = text.replace(old, new, 1)

old_message = "CP2P RETIRE found live MPEG producer without generation owner"
new_message = "CP2P RETIRE cannot acknowledge while MPEG producer is live without generation owner"
if text.count(old_message) != 1:
    raise SystemExit("item8 staged legacy-producer diagnostic no longer matches")
text = text.replace(old_message, new_message, 1)

path.write_text(text)
print("ITEM8_PATCH_FIX=PASS")

#!/usr/bin/env python3
"""Harden item #8 rollback so live producer failure cannot expose unsuppressed video."""

from pathlib import Path

runner_path = Path("experiments/media-harness-h1/h1_mux_server_cp2p_start_receiver.py")
runner = runner_path.read_text()

old = '''        if self.producer is not None and self.producer.active_generation() == int(generation):
            try:
                self.producer.retire_exact(generation)
            except BaseException as exc:
                rollback_error = exc

        if suppression_installed:
'''
new = '''        if self.producer is not None:
            active_producer_generation = int(self.producer.active_generation())
            if active_producer_generation != 0:
                if active_producer_generation != int(generation):
                    raise base.ProtocolError(
                        "CP2P START rollback producer generation mismatch "
                        f"active={active_producer_generation} requested={int(generation)}; "
                        "suppression retained fail-closed"
                    )
                try:
                    self.producer.retire_exact(generation)
                except BaseException as exc:
                    # A producer that cannot be proven stopped/drained must stay
                    # protected by its exact RFB suppression. Do not continue
                    # destructive rollback and expose a still-live video source.
                    raise base.ProtocolError(
                        f"CP2P START generation {int(generation)} producer rollback "
                        "failed; suppression retained fail-closed"
                    ) from exc

        if suppression_installed:
'''
if runner.count(new) == 1:
    print("ITEM8_SAFETY_PATCH=ALREADY_APPLIED")
else:
    if runner.count(old) != 1:
        raise SystemExit("item8 rollback producer block no longer matches")
    runner_path.write_text(runner.replace(old, new, 1))


test_path = Path("experiments/media-harness-h1/h1_cp2p_start_preparation_transaction_test.py")
test = test_path.read_text()
marker = "\n\nif __name__ == \"__main__\":\n"
new_test = r'''

class FailingRollbackProducer:
    def __init__(self, generation: int) -> None:
        self.generation = int(generation)
        self.retire_calls = 0

    def active_generation(self) -> int:
        return self.generation

    def retire_exact(self, generation: int) -> None:
        self.retire_calls += 1
        raise base.ProtocolError("forced producer stop/drain failure")


class ProducerRollbackBarrierTests(unittest.TestCase):
    def test_failed_producer_rollback_retains_suppression_and_prepared_state(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            bridge_sock, fake_vnc = socket.socketpair()
            bridge = H1Cp2pRfbPiBridge(
                bridge_sock,
                queue_capacity=1024 * 1024,
                max_payload=8192,
                send_data=lambda payload: None,
                desktop_width=640,
                desktop_height=448,
            )
            session = SimpleNamespace(
                profile={
                    "session_id": 0x12345678,
                    "desktop_width": 640,
                    "desktop_height": 448,
                },
                evidence=Path(temp_dir),
                display=":0",
            )
            receiver = H1Cp2pSuppressionStartReceiver(session, bridge)
            frame = make_start_frame(
                session_id=session.profile["session_id"],
                generation=17,
            )

            try:
                receiver._handle_start_frame(frame)
                self.assertEqual(receiver.peek_prepared().generation, 17)
                self.assertEqual(receiver.capture_plan.generation, 17)
                self.assertEqual(bridge.suppression_generation, 17)

                failing = FailingRollbackProducer(17)
                receiver.producer = failing
                with self.assertRaisesRegex(
                    base.ProtocolError,
                    "suppression retained fail-closed",
                ):
                    receiver._rollback_compound_preparation(
                        17,
                        suppression_installed=True,
                    )

                self.assertEqual(failing.retire_calls, 1)
                self.assertEqual(receiver.peek_prepared().generation, 17)
                self.assertEqual(receiver.capture_plan.generation, 17)
                self.assertEqual(bridge.suppression_generation, 17)
                pending, active, rect = bridge._suppression_state()
                self.assertTrue(pending)
                self.assertFalse(active)
                self.assertIsNotNone(rect)
                self.assertTrue((Path(temp_dir) / "mpeg_start_prepared.json").exists())
                self.assertTrue((Path(temp_dir) / "rfb_suppression_prepared.json").exists())
                self.assertTrue((Path(temp_dir) / "mpeg_capture_prepared.json").exists())
            finally:
                bridge.stop()
                fake_vnc.close()
'''

if "class ProducerRollbackBarrierTests" not in test:
    if marker not in test:
        raise SystemExit("compound rollback test footer missing")
    test_path.write_text(test.replace(marker, new_test + marker, 1))

print("ITEM8_SAFETY_PATCH=PASS")

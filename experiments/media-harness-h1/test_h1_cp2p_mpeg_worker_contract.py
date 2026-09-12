#!/usr/bin/env python3
from pathlib import Path
import tempfile
import subprocess

root = Path(__file__).resolve().parents[2]
worker = (root / 'experiments/media-harness-h1/h1_cp2p_mpeg_worker.c').read_text()
coord = (root / 'experiments/media-harness-h1/h1_cp2p_session_coordinator.c').read_text()
transport = (root / 'experiments/media-harness-h1/h1_transport_runtime.c').read_text()
main = (root / 'experiments/media-harness-h1/h1_main_cp2p_visible_interaction_pcm.c').read_text()
mk = (root / 'mk/media-harness-h1-cp2p-application-link.mk').read_text()

assert 'pstvnc_h1_transport_mpeg_read_cancellable' in transport
assert 'return pstvnc_h1_transport_mpeg_read_cancellable(' in transport
assert 'prepare_start' in coord
assert coord.index('coordinator->arm_mpeg(') < coord.index('pstvnc_h1_mpeg_start_transport_send(')
send_fail = coord[coord.index('if (!pstvnc_h1_mpeg_start_transport_send('):]
assert send_fail.index('coordinator->clear_mpeg(') < send_fail.index('pstvnc_h1_mpeg_start_handoff_abort_start(')
assert 'pstvnc_h1_cp2p_mpeg_worker_clear' in worker
assert 'pstvnc_h1_graphics_clear_video()' in worker
assert 'pstvnc_h1_mpeg_start_handoff_abort_start' not in worker
assert 'pstvnc_h1_mpeg_start_handoff_stop' not in worker
assert 'CreateThread' in worker and 'StartThread' in worker and 'DeleteThread' in worker
assert 'config->video_mode == PSTVNC_H1_VIDEO_OFF' in worker
assert 'config->video_mode != PSTVNC_H1_VIDEO_MPEG2_ES' in worker
assert 'PSTVNC_H1_VIDEO_OFF' in main and 'MPEG_WORKER_BIND_FAIL' in main
assert 'filter-out $(BUILD_DIR)/h1_video_runtime.o' in mk
assert 'EE_OBJS += $(BUILD_DIR)/h1_video_runtime_cp2p.o' not in mk
assert 'owner_state != PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY' in coord
assert 'worker->live_decode && pstvnc_h1_graphics_clear_video()' in worker

with tempfile.TemporaryDirectory() as td:
    out = Path(td) / 'generated.c'
    subprocess.run([
        'python3', str(root / 'experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py'),
        '--input', str(root / 'experiments/media-harness-h1/h1_video_runtime.c'),
        '--output', str(out)], check=True)
    generated = out.read_text()
    assert 'pstvnc_h1_transport_mpeg_read_cancellable(' in generated
    assert 'session->cancelled = 1' in generated
    assert 'const volatile int *stop_requested' in generated

print('H1_CP2P_MPEG_WORKER_HOST_TEST=PASS')
print('H1_CP2P_ARM_BEFORE_START_HOST_TEST=PASS')
print('H1_CP2P_WORKER_CLEAR_BEFORE_ABORT_HOST_TEST=PASS')
print('H1_CP2P_PUBLIC_MPEG_GATE=DORMANT')

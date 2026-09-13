#!/usr/bin/env python3
from pathlib import Path
import tempfile
import subprocess

root = Path(__file__).resolve().parents[2]

worker = (
    root / "experiments/media-harness-h1/h1_cp2p_mpeg_worker.c"
).read_text()
coord = (
    root / "experiments/media-harness-h1/h1_cp2p_session_coordinator.c"
).read_text()
transport = (
    root / "experiments/media-harness-h1/h1_transport_runtime.c"
).read_text()
main = (
    root / "experiments/media-harness-h1/h1_main_cp2p_visible_interaction_pcm.c"
).read_text()
mk = (
    root / "mk/media-harness-h1-cp2p-application-link.mk"
).read_text()

# The cancellable transport primitive remains available for diagnostics, but the
# production MPEG data callback must not use it.
assert "pstvnc_h1_transport_mpeg_read_cancellable" in transport
assert "return pstvnc_h1_transport_mpeg_read_cancellable(" in transport

assert "pstvnc_h1_cp2p_mpeg_worker_request_stop" in worker
assert "pstvnc_h1_cp2p_mpeg_worker_stop_poll" in worker
assert "worker->stop_requested = 1;" in worker

assert "coordinator->request_mpeg_stop(" in coord
assert "coordinator->poll_mpeg_stop(" in coord
assert "pstvnc_h1_transport_mpeg_retire_begin(" in coord
assert "mpeg_stop_pending" in coord

assert "pstvnc_h1_cp2p_mpeg_worker_request_stop" in main
assert "pstvnc_h1_cp2p_mpeg_worker_stop_poll" in main

assert (
    "CP2P_VIDEO_GENERATOR ?= $(CP2P_VIDEO_BASE_GENERATOR)"
    in mk
)
assert "H1_CP2P_MPEG_ASYNC_CALLBACK_STOP=ABSENT" in mk
assert "H1_CP2P_MPEG_STOP_BOUNDARY=COMPLETED_PICTURE" in mk

assert "pstvnc_h1_graphics_clear_video()" in worker
assert "pstvnc_h1_mpeg_start_handoff_abort_start" not in worker
assert "pstvnc_h1_mpeg_start_handoff_stop" not in worker
assert "CreateThread" in worker
assert "StartThread" in worker
assert "DeleteThread" in worker
assert "thread.initial_priority = 67;" in worker

with tempfile.TemporaryDirectory() as td:
    out = Path(td) / "generated.c"
    subprocess.run(
        [
            "python3",
            str(
                root
                / "experiments/media-harness-h1/"
                  "generate_h1_video_runtime_cp2p.py"
            ),
            "--input",
            str(root / "experiments/media-harness-h1/h1_video_runtime.c"),
            "--output",
            str(out),
        ],
        check=True,
    )

    generated = out.read_text()

    feed_start = generated.index(
        "static int h1_video_feed_ipu(void *user_data)"
    )
    feed_end = generated.index(
        "static void *h1_video_sequence_init(",
        feed_start,
    )
    feed = generated[feed_start:feed_end]

    release_start = generated.index(
        "static void h1_video_release_session("
    )
    release_end = generated.index(
        "int pstvnc_h1_video_chassis_init",
        release_start,
    )
    release = generated[release_start:release_end]

    assert "pstvnc_h1_transport_mpeg_read(" in feed
    assert "pstvnc_h1_transport_mpeg_read_cancellable(" not in feed
    assert "stop_requested" not in feed

    # One check after first MPEG_Picture and one in the normal decode loop.
    assert (
        generated.count(
            "session.stop_requested != NULL && *session.stop_requested"
        )
        == 2
    )

    assert "dma_channel_wait(DMA_CHANNEL_toIPU, 0);" not in release
    destroy = release.index("MPEG_Destroy();")
    reset = release.index("h1_video_reference_ipu_reset();")
    assert destroy < reset

print("H1_CP2P_MPEG_WORKER_HOST_TEST=PASS")
print("H1_CP2P_MPEG_ASYNC_CALLBACK_STOP=ABSENT")
print("H1_CP2P_MPEG_STOP_POINT=BETWEEN_COMPLETED_MPEG_PICTURE_CALLS")
print("H1_CP2P_MPEG_RELEASE_ORDER=DESTROY_THEN_LOCAL_IPU_RESET")

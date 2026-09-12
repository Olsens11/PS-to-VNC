#!/usr/bin/env python3
"""One-shot, fail-closed hardening patch for CP2P item #10 live MPEG runtime."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def patch(path: Path, transform) -> None:
    old = path.read_text()
    new = transform(old)
    if new == old:
        raise SystemExit(f"{path}: patch made no change")
    path.write_text(new)


def patch_generator(text: str) -> str:
    marker = '    draw_call = "        h1_video_draw(&session);"\n'
    insertion = r'''    source = replace_once(
        source,
        "    if (!h1_video_wait_prefill(&session)) {\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_PREFILL);\n"
        "        goto done;\n"
        "    }",
        "    if (!h1_video_wait_prefill(&session)) {\n"
        "        if (session.cancelled) {\n"
        "            success = 1;\n"
        "            goto done;\n"
        "        }\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_PREFILL);\n"
        "        goto done;\n"
        "    }",
        "CP2P clean cancellation during prefill",
    )

    source = replace_once(
        source,
        "    if (picture_result == 0 ||\n"
        "        session.transfer_packet == NULL ||\n"
        "        session.draw_packet == NULL) {\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);\n"
        "        goto done;\n"
        "    }",
        "    if (picture_result == 0) {\n"
        "        if (session.cancelled) {\n"
        "            success = 1;\n"
        "            goto done;\n"
        "        }\n"
        "        h1_video_record_error(&session, PSTVNC_H1_VIDEO_ERROR_DECODE);\n"
        "        goto done;\n"
        "    }",
        "CP2P shared-compositor first-picture guard",
    )

    source = replace_once(
        source,
        "        if (picture_result == 0)\n"
        "            break;",
        "        if (picture_result == 0) {\n"
        "            if (session.cancelled) {\n"
        "                success = 1;\n"
        "                goto done;\n"
        "            }\n"
        "            break;\n"
        "        }",
        "CP2P clean cancellation during decode loop",
    )

'''
    text = replace_once(text, marker, insertion + marker, "generator live-runtime insertion")
    text = replace_once(
        text,
        '        "DMA_CHANNEL_GIF",\n',
        '        "DMA_CHANNEL_GIF",\n'
        '        "session.transfer_packet == NULL",\n'
        '        "session.draw_packet == NULL",\n',
        "generated standalone-packet forbidden markers",
    )
    return text


def patch_worker_header(text: str) -> str:
    return replace_once(
        text,
        "    volatile int run_result;\n",
        "    volatile int run_result;\n"
        "    volatile int failure_latched;\n",
        "worker sticky failure field",
    )


def patch_worker_c(text: str) -> str:
    old = '''    worker->run_result = pstvnc_h1_video_run_cp2p_session(
        worker->transport,
        worker->clock,
        &worker->result,
        worker->handoff,
        &worker->contract,
        &worker->stop_requested);
    worker->finished = 1;
'''
    new = '''    worker->run_result = pstvnc_h1_video_run_cp2p_session(
        worker->transport,
        worker->clock,
        &worker->result,
        worker->handoff,
        &worker->contract,
        &worker->stop_requested);
    if (worker->run_result < 0)
        worker->failure_latched = 1;
    worker->finished = 1;
'''
    return replace_once(text, old, new, "worker unexpected-exit latch")


def patch_main(text: str) -> str:
    text = replace_once(
        text,
        '"H1_BOOT=PASS mode=CP2P_RFB_VISIBLE_INTERACTION_PCM_CAPABLE "\n',
        '"H1_BOOT=PASS mode=CP2P_ALL_GUNS "\n',
        "CP2P boot identity",
    )
    old = '''                if (mpeg_worker.result.pictures_displayed == 0u) {
                    printf("H1_CP2P=MPEG_PRESENTATION_UNPROVEN\\n");
                    session_ok = 0;
                }
'''
    new = '''                if (mpeg_worker.failure_latched ||
                    mpeg_worker.run_result != 0 ||
                    mpeg_worker.result.error != PSTVNC_H1_VIDEO_ERROR_NONE ||
                    mpeg_worker.result.pictures_displayed == 0u) {
                    printf(
                        "H1_CP2P=MPEG_RUNTIME_UNPROVEN failure_latched=%d "
                        "run_result=%d error=%u displayed=%u\\n",
                        (int)mpeg_worker.failure_latched,
                        (int)mpeg_worker.run_result,
                        (unsigned int)mpeg_worker.result.error,
                        (unsigned int)mpeg_worker.result.pictures_displayed);
                    session_ok = 0;
                }
'''
    return replace_once(text, old, new, "CP2P live MPEG runtime result gate")


def patch_activation_test(text: str) -> str:
    text = replace_once(
        text,
        "import h1_mux_server_cp2p_start_receiver as cp2p\n",
        "import h1_mux_server_cp2p_start_receiver as cp2p\n"
        "import generate_h1_video_runtime_cp2p as video_generator\n",
        "activation test generator import",
    )
    marker = '''    def test_wrong_active_generation_never_sends(self) -> None:
'''
    method = '''    def test_generated_live_runtime_has_no_standalone_packet_guard_and_clean_cancel(self) -> None:
        source = Path("h1_video_runtime.c").read_text()
        generated = video_generator.generate(source)
        self.assertNotIn("session.transfer_packet == NULL", generated)
        self.assertNotIn("session.draw_packet == NULL", generated)
        self.assertIn("CP2P", Path("CP2P_ITEM10_ALL_GUNS_ACTIVATION.md").read_text())
        self.assertGreaterEqual(generated.count("if (session.cancelled)"), 3)
        self.assertIn("success = 1;", generated)

        worker_header = Path("h1_cp2p_mpeg_worker.h").read_text()
        worker_source = Path("h1_cp2p_mpeg_worker.c").read_text()
        main_source = Path("h1_main_cp2p_visible_interaction_pcm.c").read_text()
        self.assertIn("failure_latched", worker_header)
        self.assertIn("worker->failure_latched = 1", worker_source)
        self.assertIn("mpeg_worker.failure_latched", main_source)

'''
    return replace_once(text, marker, method + marker, "activation live-runtime test")


def patch_doc(text: str) -> str:
    anchor = "Live PS2/Pi all-guns qualification remains item #12.\n"
    addition = '''Live PS2/Pi all-guns qualification remains item #12.

The generated CP2P decoder also removes the obsolete standalone-GS first-picture
`transfer_packet`/`draw_packet` prerequisite. Explicit exact-generation retirement
is a clean decoder cancellation, while any decoder worker that exits without an
owner stop request latches a session failure. This prevents a previously displayed
frame from masking a later live decoder/runtime failure.
'''
    return replace_once(text, anchor, addition, "item10 live-runtime documentation")


patch(ROOT / "generate_h1_video_runtime_cp2p.py", patch_generator)
patch(ROOT / "h1_cp2p_mpeg_worker.h", patch_worker_header)
patch(ROOT / "h1_cp2p_mpeg_worker.c", patch_worker_c)
patch(ROOT / "h1_main_cp2p_visible_interaction_pcm.c", patch_main)
patch(ROOT / "h1_cp2p_item10_activation_test.py", patch_activation_test)
patch(ROOT / "CP2P_ITEM10_ALL_GUNS_ACTIVATION.md", patch_doc)
print("H1_CP2P_ITEM10_LIVE_RUNTIME_HARDENING=APPLIED")

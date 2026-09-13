#!/usr/bin/env python3
"""Generate a CP2P MPEG runtime using a mature PS2 decoder lifecycle pattern.

This diagnostic is a clean-room behavioral adaptation, not a source-code port.
It keeps the project's existing CP2P transport, cancellation, diagnostics,
shared compositor, worker, and queue ownership. The only lifecycle change is
what happens after libmpeg has been allowed to report end-of-input normally:

    data callback returns 0 -> MPEG_Picture() unwinds/returns -> MPEG_Destroy()
    -> local IPU/DMAC reset -> ordinary CP2P worker/session cleanup.

That ordering is intentionally based on behavior proven by mature PS2 MPEG
players and the PS2SDK sample, but this file contains no copied player
implementation. In particular, it does not reproduce SMS_MPEG12.c control flow
or Sony sceMpeg code. The existing project-owned IPU reset helper is reused only
after decoder destruction so this resident application does not import a broad
EE reset intended for a standalone movie player.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import generate_h1_video_runtime_cp2p as basegen
import generate_h1_video_runtime_cp2p_persistent_diag as parent


def generate(source: str) -> str:
    out = parent.generate(source)

    out = basegen.replace_function(
        out,
        "static void h1_video_release_session(",
        """static void h1_video_release_session(h1_video_session_t *session)
{
    pstvnc_h1_mpeg_presentation_owner_state_t state;
    int had_mpeg;

    if (session == NULL)
        return;

    had_mpeg = session->mpeg_initialized;

    /*
     * Mature-lifecycle A/B:
     * let libmpeg consume its ordinary callback EOF and return from
     * MPEG_Picture(), then destroy decoder ownership before touching IPU/DMAC
     * hardware. There is deliberately no pre-destroy TO_IPU wait and no
     * pre-destroy IPU reset on this path.
     */
    if (had_mpeg) {
        h1_diag_stage(session, 0xD8200001u); /* before MPEG_Destroy */
        MPEG_Destroy();
        h1_diag_stage(session, 0xD8200002u); /* MPEG_Destroy returned */
        session->mpeg_initialized = 0;

        /*
         * Reclaim the local IPU/DMAC state only after decoder destruction.
         * This helper is project-owned and already establishes the clean
         * hardware state used before a new CP2P decoder session.
         */
        h1_diag_stage(session, 0xD8200003u); /* before post-destroy IPU reset */
        h1_video_reference_ipu_reset(session);
        h1_diag_stage(session, 0xD8200004u); /* post-destroy IPU reset returned */
    }

    free(session->picture_allocation);
    free(session->feed_allocation);
    session->picture_allocation = NULL;
    session->feed_allocation = NULL;
    session->picture_buffer = NULL;
    session->feed_buffer = NULL;

    if (session->handoff == NULL) {
        h1_diag_stage(session, 0xD8200005u); /* release return */
        return;
    }

    state = pstvnc_h1_mpeg_presentation_owner_state(
        &session->handoff->owner);

    /*
     * Preserve the existing CP2P ownership rule for a start that never reached
     * first-frame presentation. Successful MPEG_OWNED retirement remains the
     * coordinator's responsibility after the worker has returned.
     */
    if (state == PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
        !session->cancelled) {
        (void)pstvnc_h1_graphics_clear_video();
        (void)pstvnc_h1_mpeg_start_handoff_abort_start(
            session->handoff,
            session->start_contract.generation);
    }

    h1_diag_stage(session, 0xD8200005u); /* release return */
}""",
    )

    required = (
        "0xD8200001u",
        "MPEG_Destroy();",
        "0xD8200002u",
        "0xD8200003u",
        "h1_video_reference_ipu_reset(session);",
        "0xD8200004u",
        "0xD8200005u",
    )
    for marker in required:
        if marker not in out:
            raise RuntimeError(f"mature lifecycle generation missing marker: {marker}")

    start = out.find("static void h1_video_release_session(")
    end = out.find("int pstvnc_h1_video_chassis_init", start)
    if start < 0 or end < 0:
        raise RuntimeError("could not isolate generated release_session")

    release = out[start:end]
    if "dma_channel_wait(DMA_CHANNEL_toIPU, 0);" in release:
        raise RuntimeError("mature lifecycle retained pre-destroy TO_IPU wait")

    destroy_pos = release.find("MPEG_Destroy();")
    reset_pos = release.find("h1_video_reference_ipu_reset(session);")
    if destroy_pos < 0 or reset_pos < 0 or destroy_pos >= reset_pos:
        raise RuntimeError("mature lifecycle destroy/reset ordering mismatch")

    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    data = input_path.read_bytes()
    actual_sha = basegen.git_blob_sha1(data)
    if actual_sha != basegen.EXPECTED_INPUT_BLOB_SHA1:
        raise SystemExit(
            "H1_CP2P_VIDEO_SOURCE_MISMATCH="
            f"expected:{basegen.EXPECTED_INPUT_BLOB_SHA1}:actual:{actual_sha}"
        )

    generated = generate(data.decode("utf-8"))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(generated, encoding="utf-8")

    print(f"H1_CP2P_VIDEO_SOURCE_BLOB={actual_sha}")
    print("H1_CP2P_MPEG_STALL_DIAGNOSTICS=PASS")
    print("H1_CP2P_MPEG_PERSISTENT_WITNESS=PASS")
    print("H1_CP2P_MATURE_LIFECYCLE=PASS")
    print("H1_CP2P_MATURE_PRE_DESTROY_DMA_WAIT=ABSENT")
    print("H1_CP2P_MATURE_ORDER=MPEG_DESTROY_THEN_LOCAL_IPU_RESET")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

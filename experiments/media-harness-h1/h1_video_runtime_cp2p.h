/*
 * File synopsis:
 * Declares the CP2P MPEG runtime variant generated mechanically from the
 * authoritative H1 MPEG decoder/runtime source.
 *
 * The decoder, IPU feed, timing and scheduler remain H1-owned. Presentation is
 * delegated to the single RFB/MPEG compositor and is bound to one already-armed
 * calibrated MPEG start generation.
 */
#ifndef PSTVNC_H1_VIDEO_RUNTIME_CP2P_H
#define PSTVNC_H1_VIDEO_RUNTIME_CP2P_H

#include "h1_video_runtime.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

int pstvnc_h1_video_run_cp2p_session(
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock,
    pstvnc_h1_video_result_t *result,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_h1_mpeg_start_contract_t *start_contract,
    const volatile int *stop_requested);

/*
 * Retire an active compositor presentation after the concurrent RFB worker is
 * proven dormant. This ordering prevents a direct-RFB/compositor ownership race
 * while the final MPEG frame is removed.
 */
int pstvnc_h1_video_cp2p_retire_presentation(
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    uint32_t generation);

#endif /* PSTVNC_H1_VIDEO_RUNTIME_CP2P_H */

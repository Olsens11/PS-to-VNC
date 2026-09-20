/*
 * File synopsis:
 * Defines the Display-owned A004 MPEG compositor coordinator that binds one
 * exact P3 run snapshot to the sole PS2 graphics owner and to the session-scoped
 * common media clock only at a synchronized physical presentation boundary.
 *
 * This module does not decode MPEG, own GS/dmaKit, edit calibration, manage RFB
 * refresh debt, send Transport START/RETIRE, schedule/drop frames, prove
 * producer/decoder/RFB retirement prerequisites, or orchestrate the Application
 * transaction. It does coordinate the final exact-run synchronized visible
 * reveal after the caller has explicitly sealed retirement.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-SHARED-COMPOSITOR-FIRST-SYNC-R4;
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md, Q6/Q7.
 */

#ifndef PSTVNC_MPEG_COMPOSITOR_H
#define PSTVNC_MPEG_COMPOSITOR_H

#include <stdint.h>

#include "media/clock.h"
#include "mpeg_frame.h"
#include "mpeg_presentation.h"

typedef struct pstvnc_mpeg_compositor_frame {
    pstvnc_mpeg_rgb16_macroblock_surface_t surface;
    uint32_t run_generation;
} pstvnc_mpeg_compositor_frame_t;

typedef enum pstvnc_mpeg_compositor_result {
    PSTVNC_MPEG_COMPOSITOR_OK = 0,
    PSTVNC_MPEG_COMPOSITOR_INVALID = -1,
    PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED = -2,
    PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID = -3,
    PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED = -4,
    PSTVNC_MPEG_COMPOSITOR_PROMOTION_FAILED = -5,
    PSTVNC_MPEG_COMPOSITOR_RETIREMENT_COMMIT_FAILED = -6
} pstvnc_mpeg_compositor_result_t;

typedef struct pstvnc_mpeg_compositor_effects {
    unsigned synchronized : 1;
    unsigned clock_armed_now : 1;
    unsigned first_frame_promoted : 1;
    unsigned retirement_revealed : 1;
    uint64_t observed_sync_tick;
} pstvnc_mpeg_compositor_effects_t;

/*
 * Present one exact-run decoded frame through the sole Platform graphics owner.
 *
 * WAIT_FIRST_FRAME:
 *   physical synchronized success -> session clock first-arm if needed ->
 *   exact P3 first-frame promotion.
 *
 * MPEG_OWNED / RETIRING:
 *   physical synchronized success only; the existing session epoch and exact
 *   run ownership remain unchanged. RETIRING therefore permits already-accepted
 *   exact-run frames to drain.
 *
 * REVEAL_PENDING:
 *   frame presentation is rejected; use the exact reveal operation below.
 *
 * The returned effects preserve truthful physical facts even when a later clock
 * or promotion step fails. A non-OK return never means first presentation
 * succeeded as an A004 lifecycle transaction.
 */
int pstvnc_mpeg_compositor_present(
    pstvnc_mpeg_presentation_t *presentation,
    pstvnc_media_clock_t *clock,
    const pstvnc_mpeg_compositor_frame_t *frame,
    pstvnc_mpeg_compositor_effects_t *effects);

/*
 * Perform the exact REVEAL_PENDING run's synchronized final visible handoff.
 *
 * The caller must have independently proven producer/decoder/Transport/RFB
 * prerequisites before sealing Presentation. This function does not inspect or
 * manufacture those facts and does not touch the session media clock.
 *
 * Platform first presents cached desktop + local overlay without retained MPEG.
 * Only after that synchronized physical success may Presentation clear the exact
 * run snapshot and return logically to RFB_ONLY.
 */
int pstvnc_mpeg_compositor_reveal_retired(
    pstvnc_mpeg_presentation_t *presentation,
    uint32_t run_generation,
    pstvnc_mpeg_compositor_effects_t *effects);

#endif /* PSTVNC_MPEG_COMPOSITOR_H */

/*
 * File synopsis:
 * Defines the CP2P-only recalibration-entry lifecycle.
 *
 * START+SELECT never mutates an existing MPEG generation. If a generation is
 * active (or still waiting for its first frame), its immutable calibrated
 * region is preserved as the next calibration seed, its visible MPEG state is
 * withdrawn by the caller-supplied callback, and the generation is retired.
 * Calibration entry is then deferred until a NEW full RFB refresh issued after
 * that retirement has completed. With no MPEG generation, calibration may enter
 * immediately.
 */
#ifndef PSTVNC_H1_MPEG_RECALIBRATION_H
#define PSTVNC_H1_MPEG_RECALIBRATION_H

#include "h1_mpeg_cp2p_rfb_flow.h"
#include "h1_mpeg_start_handoff.h"

typedef enum pstvnc_h1_mpeg_recalibration_begin_result {
    PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW = 0,
    PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB
} pstvnc_h1_mpeg_recalibration_begin_result_t;

typedef int (*pstvnc_h1_mpeg_recalibration_clear_mpeg_fn)(
    void *context,
    uint32_t generation);

typedef struct pstvnc_h1_mpeg_recalibration {
    unsigned awaiting_restored_full_rfb : 1;
} pstvnc_h1_mpeg_recalibration_t;

void pstvnc_h1_mpeg_recalibration_init(
    pstvnc_h1_mpeg_recalibration_t *recalibration);

/*
 * Begin one fresh calibration transaction.
 *
 * RFB_ONLY:
 *   no generation exists, so the caller may enter calibration immediately.
 *
 * WAIT_FIRST_FRAME / MPEG_OWNED:
 *   copy the immutable owner region (including inner/outer matte settings) into
 *   calibration's durable committed seed, withdraw the visible/producer MPEG
 *   state through clear_mpeg(), retire the exact owner generation, and arm the
 *   CP2P RFB flow to recognize only a post-retirement full refresh. The caller
 *   must delay actual calibration entry until _take_entry_ready() succeeds.
 */
int pstvnc_h1_mpeg_recalibration_begin(
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    pstvnc_mpeg_calibration_t *calibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context,
    pstvnc_h1_mpeg_recalibration_begin_result_t *result);

/*
 * One-shot readiness edge for the deferred MPEG->RFB->calibration transition.
 * It cannot be satisfied by a full request that was already in flight when
 * START+SELECT retired the MPEG generation.
 */
int pstvnc_h1_mpeg_recalibration_take_entry_ready(
    pstvnc_h1_mpeg_recalibration_t *recalibration,
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *rfb_flow);

#endif /* PSTVNC_H1_MPEG_RECALIBRATION_H */

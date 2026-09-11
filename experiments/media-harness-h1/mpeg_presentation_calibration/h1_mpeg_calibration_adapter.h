/*
 * File synopsis:
 * Defines the H1 experiment-local adapter between PS-to-VNC's normalized
 * physical-controller facts and the portable MPEG presentation calibration
 * module.
 *
 * This adapter assigns calibration meaning to already-owned controller facts;
 * it never polls libpad, changes qualified controller code, owns RFB framing,
 * or performs permanent MPEG/RFB composition.
 */

#ifndef PSTVNC_H1_MPEG_CALIBRATION_ADAPTER_H
#define PSTVNC_H1_MPEG_CALIBRATION_ADAPTER_H

#include "controller.h"
#include "mpeg_presentation_calibration.h"

#include <stdint.h>

typedef struct pstvnc_h1_mpeg_calibration_adapter {
    pstvnc_mpeg_calibration_t calibration;

    /*
     * The START+SELECT chord is detected from buttons_down, not paired edge
     * events. This preserves the temporary held-chord requirement across a
     * reconnect/epoch sample where no synthetic press edges are manufactured.
     */
    unsigned foreground_claimed : 1;
} pstvnc_h1_mpeg_calibration_adapter_t;

typedef struct pstvnc_h1_mpeg_calibration_adapter_result {
    pstvnc_mpeg_cal_effects_t calibration_effects;

    /*
     * When true, the H1 coordinator must not route this controller observation
     * through ordinary local-controller/OSK/mouse semantics.
     */
    unsigned consume_controller_state : 1;

    /*
     * When true, ordinary framebuffer-update request generation/presentation
     * must remain quiesced at the existing complete-server-message boundary.
     */
    unsigned freeze_rfb_visuals : 1;
} pstvnc_h1_mpeg_calibration_adapter_result_t;

void pstvnc_h1_mpeg_calibration_adapter_init(
    pstvnc_h1_mpeg_calibration_adapter_t *adapter,
    int canvas_width,
    int canvas_height);

int pstvnc_h1_mpeg_calibration_adapter_service_controller(
    pstvnc_h1_mpeg_calibration_adapter_t *adapter,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_h1_mpeg_calibration_adapter_result_t *result);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_ADAPTER_H */

/*
 * File synopsis:
 * Defines the experiment-local runtime boundary that composes MPEG calibration
 * foreground ownership, safe-boundary RFB visual/request gating, and the
 * platform-neutral native render plan without moving any of those concerns
 * into qualified src/ owners or the permanent MPEG compositor.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_RUNTIME_H
#define PSTVNC_H1_MPEG_CALIBRATION_RUNTIME_H

#include "h1_mpeg_calibration_foreground.h"
#include "h1_mpeg_calibration_render.h"
#include "h1_mpeg_calibration_rfb_gate.h"

typedef struct pstvnc_h1_mpeg_calibration_runtime {
    pstvnc_h1_mpeg_calibration_foreground_t foreground;
    pstvnc_h1_mpeg_calibration_rfb_gate_t rfb_gate;
} pstvnc_h1_mpeg_calibration_runtime_t;

typedef struct pstvnc_h1_mpeg_calibration_runtime_result {
    unsigned consume_controller_state : 1;
    unsigned freeze_rfb_visuals : 1;
    unsigned calibration_visible : 1;
} pstvnc_h1_mpeg_calibration_runtime_result_t;

void pstvnc_h1_mpeg_calibration_runtime_init(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    int canvas_width,
    int canvas_height);

/*
 * Service one normalized physical-controller observation.
 *
 * The foreground bridge owns suspend/neutralize/rebase/resume ordering. The
 * runtime immediately mirrors its freeze fact into the experiment-local RFB
 * gate; actual RFB request/presentation scheduling is still performed by the
 * H1 coordinator/worker only at its existing complete-server-message boundary.
 */
int pstvnc_h1_mpeg_calibration_runtime_service_controller(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    const pstvnc_controller_state_t *controller_state,
    const pstvnc_h1_mpeg_calibration_foreground_ops_t *foreground_ops,
    void *foreground_context,
    pstvnc_h1_mpeg_calibration_runtime_result_t *result);

/* Prepare the current native calibration render plan when visible. */
int pstvnc_h1_mpeg_calibration_runtime_prepare_render_plan(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime,
    pstvnc_h1_mpeg_calibration_render_plan_t *plan);

/* Safe-boundary scheduling queries for the existing H1 RFB owner. */
int pstvnc_h1_mpeg_calibration_runtime_allows_rfb_request(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime);

int pstvnc_h1_mpeg_calibration_runtime_allows_remote_present(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime);

int pstvnc_h1_mpeg_calibration_runtime_take_full_refresh(
    pstvnc_h1_mpeg_calibration_runtime_t *runtime);

/* Accepted geometry remains calibration data; a later MPEG presenter consumes it. */
const pstvnc_mpeg_cal_region_t *
pstvnc_h1_mpeg_calibration_runtime_committed_region(
    const pstvnc_h1_mpeg_calibration_runtime_t *runtime);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_RUNTIME_H */

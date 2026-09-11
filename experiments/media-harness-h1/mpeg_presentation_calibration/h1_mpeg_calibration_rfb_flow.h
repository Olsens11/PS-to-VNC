/*
 * File synopsis:
 * Adapts the experiment-local MPEG calibration RFB gate/scheduler to the
 * generic H1 RFB session flow-policy seam without making the RFB session
 * runtime calibration-aware.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_RFB_FLOW_H
#define PSTVNC_H1_MPEG_CALIBRATION_RFB_FLOW_H

#include "h1_mpeg_calibration_rfb_schedule.h"
#include "h1_mpeg_calibration_runtime.h"
#include "../h1_rfb_session_runtime.h"

typedef struct pstvnc_h1_mpeg_calibration_rfb_flow {
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
} pstvnc_h1_mpeg_calibration_rfb_flow_t;

void pstvnc_h1_mpeg_calibration_rfb_flow_init(
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow,
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime);

/*
 * Prepare the generic H1 RFB runtime policy callbacks backed by this flow.
 * The returned policy borrows `flow`; both must remain alive for the RFB run.
 */
int pstvnc_h1_mpeg_calibration_rfb_flow_prepare_policy(
    pstvnc_h1_mpeg_calibration_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_RFB_FLOW_H */

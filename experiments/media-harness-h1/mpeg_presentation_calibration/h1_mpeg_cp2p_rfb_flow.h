/*
 * File synopsis:
 * Defines the CP2P-only RFB flow-policy adapter that composes MPEG calibration
 * freeze/thaw policy with accepted-region/first-frame MPEG presentation
 * ownership without changing the generic H1 RFB runtime or the CP2O path.
 */
#ifndef PSTVNC_H1_MPEG_CP2P_RFB_FLOW_H
#define PSTVNC_H1_MPEG_CP2P_RFB_FLOW_H

#include "h1_mpeg_calibration_rfb_schedule.h"
#include "h1_mpeg_calibration_runtime.h"
#include "h1_mpeg_presentation_owner.h"
#include "../h1_rfb_session_runtime.h"

typedef struct pstvnc_h1_mpeg_cp2p_rfb_flow {
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime;
    pstvnc_h1_mpeg_presentation_owner_t *presentation_owner;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
} pstvnc_h1_mpeg_cp2p_rfb_flow_t;

void pstvnc_h1_mpeg_cp2p_rfb_flow_init(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime,
    pstvnc_h1_mpeg_presentation_owner_t *presentation_owner);

int pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy);

#endif /* PSTVNC_H1_MPEG_CP2P_RFB_FLOW_H */

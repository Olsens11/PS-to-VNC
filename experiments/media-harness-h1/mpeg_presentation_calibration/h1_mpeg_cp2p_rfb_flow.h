/*
 * File synopsis:
 * Defines the CP2P-only RFB flow-policy adapter that composes MPEG calibration
 * freeze/thaw policy with accepted-region/first-frame MPEG presentation
 * ownership without changing the generic H1 RFB runtime or the CP2O path.
 *
 * A tiny restoration watch lets MPEG->calibration re-entry distinguish a NEW
 * post-retirement full refresh from any full request that was already in flight
 * when START+SELECT retired the old MPEG generation.
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

    /* Exact request kind currently owned by the one-outstanding scheduler. */
    pstvnc_h1_mpeg_calibration_rfb_request_t outstanding_request;

    /* One-shot epoch used only by MPEG->full-RFB->calibration re-entry. */
    unsigned restoration_watch_armed : 1;
    unsigned restoration_full_request_sent : 1;
    unsigned restoration_full_refresh_complete : 1;
} pstvnc_h1_mpeg_cp2p_rfb_flow_t;

void pstvnc_h1_mpeg_cp2p_rfb_flow_init(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_mpeg_calibration_runtime_t *calibration_runtime,
    pstvnc_h1_mpeg_presentation_owner_t *presentation_owner);

int pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow,
    pstvnc_h1_rfb_flow_policy_t *policy);

/*
 * Start a fresh restoration watch. A FULL request already outstanding at this
 * instant is intentionally ineligible; only a later FULL request sent while the
 * watch is armed can satisfy it.
 */
void pstvnc_h1_mpeg_cp2p_rfb_flow_arm_restoration_watch(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow);

/* One-shot completion edge consumed by deferred calibration entry. */
int pstvnc_h1_mpeg_cp2p_rfb_flow_take_restoration_complete(
    pstvnc_h1_mpeg_cp2p_rfb_flow_t *flow);

#endif /* PSTVNC_H1_MPEG_CP2P_RFB_FLOW_H */

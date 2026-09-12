/*
 * File synopsis:
 * Defines CP2P's session-level owner for calibration acceptance, MPEG
 * generation start/retirement, the combined RFB flow policy, and the existing
 * PSTV transport/session identity.
 */
#ifndef PSTVNC_H1_CP2P_SESSION_COORDINATOR_H
#define PSTVNC_H1_CP2P_SESSION_COORDINATOR_H

#include "h1_interaction_coordinator.h"
struct pstvnc_h1_transport_runtime;
#include "mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.h"
#include "mpeg_presentation_calibration/h1_mpeg_recalibration.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

typedef int (*pstvnc_h1_cp2p_session_arm_mpeg_fn)(
    void *context,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_h1_mpeg_start_contract_t *contract);

typedef struct pstvnc_h1_cp2p_session_coordinator {
    pstvnc_h1_interaction_coordinator_t interaction;
    pstvnc_h1_mpeg_start_handoff_t mpeg_handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t rfb_flow;
    pstvnc_h1_rfb_flow_policy_t rfb_policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;

    struct pstvnc_h1_transport_runtime *transport;
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg;
    void *clear_mpeg_context;
    pstvnc_h1_cp2p_session_arm_mpeg_fn arm_mpeg;
    void *arm_mpeg_context;
    uint32_t session_id;

    pstvnc_h1_mpeg_start_contract_t current_start_contract;
    uint32_t start_messages_sent;
    uint32_t pi_retire_generation;
    unsigned current_start_contract_valid : 1;
    unsigned pi_retire_pending : 1;
    unsigned initialized : 1;
} pstvnc_h1_cp2p_session_coordinator_t;

int pstvnc_h1_cp2p_session_coordinator_init(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    struct pstvnc_h1_transport_runtime *transport,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context);

int pstvnc_h1_cp2p_session_coordinator_set_mpeg_worker(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    pstvnc_h1_cp2p_session_arm_mpeg_fn arm_mpeg,
    void *arm_mpeg_context);

int pstvnc_h1_cp2p_session_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);

int pstvnc_h1_cp2p_session_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session);

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_cp2p_session_coordinator_rfb_policy(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator);

pstvnc_h1_mpeg_start_handoff_t *
pstvnc_h1_cp2p_session_coordinator_mpeg_handoff(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator);

const pstvnc_h1_mpeg_start_contract_t *
pstvnc_h1_cp2p_session_coordinator_current_start_contract(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator);

int pstvnc_h1_cp2p_session_coordinator_shutdown(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator);

#endif /* PSTVNC_H1_CP2P_SESSION_COORDINATOR_H */

/*
 * File synopsis:
 * Defines the CP2P interaction composition that adds MPEG presentation ownership
 * and clean recalibration entry around the existing H1 interaction coordinator.
 *
 * The ordinary coordinator remains authoritative for controller/mouse/OSK/
 * keyboard/calibration UI behavior. This wrapper owns only CP2P-specific facts:
 * one MPEG start handoff, the combined RFB flow policy, and the recalibration
 * bridge that retires a generation before a fresh calibration transaction.
 */
#ifndef PSTVNC_H1_INTERACTION_CP2P_H
#define PSTVNC_H1_INTERACTION_CP2P_H

#include "h1_interaction_coordinator.h"
#include "mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.h"
#include "mpeg_presentation_calibration/h1_mpeg_recalibration.h"
#include "mpeg_presentation_calibration/h1_mpeg_recalibration_entry_bridge.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

typedef struct pstvnc_h1_interaction_cp2p {
    pstvnc_h1_interaction_coordinator_t interaction;
    pstvnc_h1_mpeg_start_handoff_t mpeg_handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t rfb_flow;
    pstvnc_h1_rfb_flow_policy_t rfb_policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;
    pstvnc_h1_mpeg_recalibration_entry_bridge_t entry_bridge;
    unsigned initialized : 1;
} pstvnc_h1_interaction_cp2p_t;

/*
 * clear_mpeg is invoked only when START+SELECT matures while a generation is
 * WAIT_FIRST_FRAME or MPEG_OWNED. It must stop/withdraw producer-visible MPEG
 * state for that exact generation, but must NOT mutate mpeg_handoff.owner; the
 * recalibration helper performs the one authoritative exact-generation retire.
 */
int pstvnc_h1_interaction_cp2p_init(
    pstvnc_h1_interaction_cp2p_t *cp2p,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context);

/* RFB callbacks for the generic H1 session runtime. */
int pstvnc_h1_interaction_cp2p_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);

int pstvnc_h1_interaction_cp2p_service(
    void *context,
    pstvnc_rfb_session_t *session);

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_interaction_cp2p_rfb_policy(
    const pstvnc_h1_interaction_cp2p_t *cp2p);

pstvnc_h1_mpeg_start_handoff_t *
pstvnc_h1_interaction_cp2p_mpeg_handoff(
    pstvnc_h1_interaction_cp2p_t *cp2p);

int pstvnc_h1_interaction_cp2p_shutdown(
    pstvnc_h1_interaction_cp2p_t *cp2p);

#endif /* PSTVNC_H1_INTERACTION_CP2P_H */

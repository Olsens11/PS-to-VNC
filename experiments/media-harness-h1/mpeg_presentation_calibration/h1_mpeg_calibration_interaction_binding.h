/*
 * File synopsis:
 * Defines the experiment-local binding between H1 interaction-coordinator
 * ownership facts and the MPEG presentation calibration runtime.
 *
 * This binding is intentionally narrow: it borrows the current H1 input runtime,
 * RFB session, published pointer state, and pointer-message counter so the
 * already-proven calibration foreground bridge can execute the same
 * suspend -> neutralize -> rebase -> release-quarantine -> resume lifecycle used
 * by the current source architecture. It does not poll libpad, route ordinary
 * local-controller actions, own the RFB parser, or perform presentation.
 */
#ifndef PSTVNC_H1_MPEG_CALIBRATION_INTERACTION_BINDING_H
#define PSTVNC_H1_MPEG_CALIBRATION_INTERACTION_BINDING_H

#include "h1_mpeg_calibration_rfb_flow.h"

#include "input_runtime.h"
#include "rfb_session.h"

#include <stdint.h>

typedef struct pstvnc_h1_mpeg_calibration_interaction_context {
    pstvnc_input_runtime_t *input_runtime;
    pstvnc_rfb_session_t *rfb_session;

    unsigned int *published_cursor_x;
    unsigned int *published_cursor_y;
    unsigned char *published_click_buttons;
    uint32_t *pointer_messages_sent;
    int *mouse_interpretation_suspended;
} pstvnc_h1_mpeg_calibration_interaction_context_t;

typedef struct pstvnc_h1_mpeg_calibration_interaction_result {
    unsigned consume_controller_state : 1;

    /* Existing one-shot REVIEW acceptance edge, propagated without inference. */
    unsigned accepted : 1;
} pstvnc_h1_mpeg_calibration_interaction_result_t;

typedef struct pstvnc_h1_mpeg_calibration_interaction_binding {
    pstvnc_h1_mpeg_calibration_runtime_t runtime;
    pstvnc_h1_mpeg_calibration_rfb_flow_t rfb_flow;
    pstvnc_h1_rfb_flow_policy_t rfb_policy;
} pstvnc_h1_mpeg_calibration_interaction_binding_t;

void pstvnc_h1_mpeg_calibration_interaction_binding_init(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    int canvas_width,
    int canvas_height);

/*
 * Result-rich service seam for CP2P coordination. It preserves the existing
 * controller-consumption fact and the calibration core's one-shot accepted edge
 * from the same normalized controller observation. No parallel event detector or
 * callback state machine is introduced.
 */
int pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_h1_mpeg_calibration_interaction_result_t *result);

/*
 * Compatibility surface used by the current CP2O interaction coordinator.
 * Behavior is unchanged: service one normalized controller observation and
 * report only whether ordinary routing must consume it.
 */
int pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    const pstvnc_controller_state_t *controller_state,
    int *consume_controller_state);

/*
 * Borrow the immutable committed calibration region after an accepted edge.
 * The returned pointer remains owned by the calibration runtime.
 */
const pstvnc_mpeg_cal_region_t *
pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding);

/*
 * Foreground and suspension facts intentionally exposed to the experiment-local
 * coordinator so ordinary controller routing and the desktop mouse-resume path
 * cannot race calibration ownership.
 */
int pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding);

int pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding);

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_INTERACTION_BINDING_H */

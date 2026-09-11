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
 * Service one already-normalized controller observation before ordinary
 * pstvnc_local_controller_route() dispatch.
 *
 * Returns 1 on success and writes whether the observation belongs exclusively
 * to calibration. A callback/runtime failure returns 0 and preserves the
 * calibration foreground bridge's fail-closed ownership state.
 */
int pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    const pstvnc_controller_state_t *controller_state,
    int *consume_controller_state);

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_INTERACTION_BINDING_H */

/*
 * File synopsis:
 * Owns the CP2L experiment's narrow application-side bridge from the existing
 * through-Issue-39 semantic input runtime to RFB pointer messages.
 *
 * The controller worker remains a semantic producer only. This adapter is
 * serviced exclusively by the H1 RFB coordinator on the main/application
 * thread at complete server-message boundaries. It never reads the physical
 * PSTV socket, creates another socket, touches GS/display state, starts OSK or
 * local UI, or serializes keyboard input.
 *
 * CP2L scope is deliberately mouse-only: D-pad/stick pointer movement, ordinary
 * left/right click state, and the already-earned wheel semantics. Physical
 * controller-state events are consumed but carry no local-UI meaning here.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_INPUT_SERVICE_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_INPUT_SERVICE_H

#include "input_runtime.h"
#include "rfb_session.h"

#include <stdint.h>

typedef struct pstvnc_h1_rfb_input_service {
    pstvnc_input_runtime_t input_runtime;

    unsigned int published_cursor_x;
    unsigned int published_cursor_y;
    unsigned char published_click_buttons;

    int input_initialized;
    int input_started;

    uint32_t controller_state_events_consumed;
    uint32_t mouse_update_events_consumed;
    uint32_t pointer_messages_sent;
    uint32_t wheel_pulses_sent;
} pstvnc_h1_rfb_input_service_t;

void pstvnc_h1_rfb_input_service_init(
    pstvnc_h1_rfb_input_service_t *service);

/*
 * H1 RFB application-service callback.
 *
 * The first successful call initializes libpad/input ownership, publishes the
 * neutral center pointer, and only then starts the controller worker. Later
 * calls drain the semantic FIFO and serialize mouse results through the supplied
 * already-synchronized RFB session.
 */
int pstvnc_h1_rfb_input_service_service(
    void *context,
    pstvnc_rfb_session_t *session);

/*
 * Cooperatively stop the controller worker and release input-family resources.
 * A failure means worker dormancy was not proven; callers must not reuse the
 * service storage for another session in that state.
 */
int pstvnc_h1_rfb_input_service_shutdown(
    pstvnc_h1_rfb_input_service_t *service);

#endif

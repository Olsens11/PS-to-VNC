/*
 * File synopsis:
 * Owns CP2M's narrow application-side bridge from the existing through-Issue-39
 * controller/input runtime to RFB pointer plus transient L1+D-pad arrow-key
 * messages.
 *
 * The controller worker remains a semantic producer only. This adapter is
 * serviced exclusively by the H1 RFB coordinator on the main/application
 * thread at complete server-message boundaries. It never reads the physical
 * PSTV socket, creates another socket, touches GS/display state, starts OSK or
 * local UI, or activates AUDIO/MPEG.
 *
 * CP2M extends the hardware-qualified CP2L mouse path only with the historical
 * Test11F physical chord: while L1 is held, D-pad direction edges produce
 * remote arrow-key taps. L1 is not a mode. Mouse interpretation is synchronously
 * suspended for the chord so D-pad facts cannot become both pointer and keyboard
 * actions or accumulate hidden mouse state.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_KEYBOARD_INPUT_SERVICE_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_KEYBOARD_INPUT_SERVICE_H

#include "input_runtime.h"
#include "rfb_session.h"

#include <stdint.h>

typedef struct pstvnc_h1_rfb_keyboard_input_service {
    pstvnc_input_runtime_t input_runtime;

    unsigned int published_cursor_x;
    unsigned int published_cursor_y;
    unsigned char published_click_buttons;

    int input_initialized;
    int input_started;
    int l1_keyboard_chord_active;

    uint32_t controller_state_events_consumed;
    uint32_t mouse_update_events_consumed;
    uint32_t pointer_messages_sent;
    uint32_t wheel_pulses_sent;

    uint32_t keyboard_taps_sent;
    uint32_t keyboard_messages_sent;
    uint32_t keyboard_chord_entries;
    uint32_t keyboard_chord_exits;
} pstvnc_h1_rfb_keyboard_input_service_t;

void pstvnc_h1_rfb_keyboard_input_service_init(
    pstvnc_h1_rfb_keyboard_input_service_t *service);

/*
 * H1 RFB application-service callback.
 *
 * The first successful call initializes input ownership and establishes a
 * neutral center pointer before starting the controller worker. Later calls
 * drain typed semantic events and serialize only CP2M's pointer and transient
 * arrow-key behavior through the supplied already-synchronized RFB session.
 */
int pstvnc_h1_rfb_keyboard_input_service_service(
    void *context,
    pstvnc_rfb_session_t *session);

/*
 * Cooperatively stop the controller worker and release input-family resources.
 * A failure means worker dormancy was not proven; callers must not reuse the
 * service storage for another session in that state.
 */
int pstvnc_h1_rfb_keyboard_input_service_shutdown(
    pstvnc_h1_rfb_keyboard_input_service_t *service);

#endif

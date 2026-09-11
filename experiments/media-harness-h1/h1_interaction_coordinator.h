/*
 * File synopsis:
 * Defines the H1 experiment-local application-composition scaffold used to
 * exercise the real through-Issue-39 input, local-controller, OSK, keyboard,
 * local-presentation, display, graphics, and RFB modules together over the H1
 * mux-backed RFB session.
 *
 * This coordinator intentionally owns only cross-domain experiment wiring and
 * checkpoint instrumentation. It does not redefine controller, mouse, keyboard,
 * OSK, local-UI, RFB, display, graphics, or transport semantics. Its production
 * analogue is the clean application coordinator; this file remains H1
 * scaffolding until the later architectural-alignment/integration work.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_INTERACTION_COORDINATOR_H
#define PSTVNC_MEDIA_HARNESS_H1_INTERACTION_COORDINATOR_H

#include "display.h"
#include "input_runtime.h"
#include "local_controller.h"
#include "local_ui.h"
#include "mpeg_presentation_calibration/h1_mpeg_calibration_foreground.h"
#include "osk.h"
#include "osk_render.h"
#include "rfb_session.h"

#include <stdint.h>

typedef struct pstvnc_h1_interaction_stats {
    uint32_t controller_state_events_consumed;
    uint32_t mouse_update_events_consumed;
    uint32_t keyboard_taps_published;
    uint32_t pointer_messages_sent;
    uint32_t wheel_pulses_sent;
    uint32_t key_messages_sent;
    uint32_t osk_open_count;
    uint32_t osk_close_count;
    uint32_t local_presentations;
} pstvnc_h1_interaction_stats_t;

typedef struct pstvnc_h1_interaction_coordinator {
    pstvnc_input_runtime_t input_runtime;
    pstvnc_local_controller_t local_controller;
    pstvnc_local_ui_t local_ui;
    pstvnc_osk_t osk;

    /*
     * Experiment-local MPEG calibration foreground ownership. This composes
     * normalized controller facts with the already-qualified input-runtime
     * ownership lifecycle without adding calibration semantics to src/.
     */
    pstvnc_h1_mpeg_calibration_foreground_t mpeg_calibration_foreground;

    const pstvnc_framebuffer_t *current_framebuffer;

    unsigned int published_cursor_x;
    unsigned int published_cursor_y;
    unsigned char published_click_buttons;

    int input_initialized;
    int input_started;
    int mouse_interpretation_suspended;
    int calibration_rfb_visuals_frozen;

    pstvnc_h1_interaction_stats_t stats;

    uint16_t gs_pixels[PSTVNC_DISPLAY_PIXEL_COUNT]
        __attribute__((aligned(128)));

    uint16_t local_overlay_pixels[PSTVNC_OSK_SURFACE_PIXEL_COUNT]
        __attribute__((aligned(128)));
} pstvnc_h1_interaction_coordinator_t;

void pstvnc_h1_interaction_coordinator_init(
    pstvnc_h1_interaction_coordinator_t *coordinator);

/* H1 RFB complete-frame presentation callback. */
int pstvnc_h1_interaction_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);

/* H1 RFB safe-server-message-boundary application-service callback. */
int pstvnc_h1_interaction_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session);

/*
 * Application-level RFB ownership fact. A true result means the RFB worker must
 * keep protocol/session state healthy but issue no new framebuffer requests and
 * present no new remote visuals until this fact clears at a safe message
 * boundary.
 */
int pstvnc_h1_interaction_coordinator_rfb_visuals_frozen(
    const pstvnc_h1_interaction_coordinator_t *coordinator);

/*
 * Cooperatively stop the real through-Issue-39 input runtime. A failure means
 * controller-worker dormancy was not proven and the coordinator storage must not
 * be reused for another H1 session.
 */
int pstvnc_h1_interaction_coordinator_shutdown(
    pstvnc_h1_interaction_coordinator_t *coordinator);

#endif /* PSTVNC_MEDIA_HARNESS_H1_INTERACTION_COORDINATOR_H */

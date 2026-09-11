/*
 * File synopsis:
 * Defines the H1 experiment-local foreground-ownership bridge for MPEG
 * presentation calibration.
 *
 * The bridge composes the normalized-controller calibration adapter with the
 * already-qualified higher-level input-runtime ownership sequence without
 * changing controller, mouse, RFB-parser, local-UI, or permanent compositor
 * implementations.
 */

#ifndef PSTVNC_H1_MPEG_CALIBRATION_FOREGROUND_H
#define PSTVNC_H1_MPEG_CALIBRATION_FOREGROUND_H

#include "h1_mpeg_calibration_adapter.h"

#include <stdint.h>

typedef struct pstvnc_h1_mpeg_calibration_foreground_ops {
    /*
     * Establish the existing mouse-interpretation suspension boundary while
     * physical controller polling continues.
     */
    int (*suspend_mouse_interpretation)(void *context);

    /*
     * Publish any required remote pointer-button release at the last
     * successfully published cursor position.
     */
    int (*neutralize_published_pointer)(void *context);

    /*
     * Rebase the suspended local mouse interpreter to that exact published
     * remote pointer state.
     */
    int (*rebase_suspended_mouse_state)(void *context);

    /*
     * Withdraw the existing input-runtime suspension only after calibration's
     * physical-release quarantine is complete.
     */
    int (*resume_mouse_interpretation)(void *context);
} pstvnc_h1_mpeg_calibration_foreground_ops_t;

typedef struct pstvnc_h1_mpeg_calibration_foreground {
    pstvnc_h1_mpeg_calibration_adapter_t adapter;

    unsigned mouse_interpretation_suspended : 1;
    unsigned rfb_visuals_frozen : 1;
} pstvnc_h1_mpeg_calibration_foreground_t;

typedef struct pstvnc_h1_mpeg_calibration_foreground_result {
    pstvnc_h1_mpeg_calibration_adapter_result_t adapter_result;

    /*
     * True when this controller observation belongs exclusively to calibration
     * rather than ordinary local-controller/OSK/mouse routing.
     */
    unsigned consume_controller_state : 1;

    /*
     * True while the H1 safe-server-message-boundary path must issue no new RFB
     * framebuffer requests and present no new RFB visual updates.
     */
    unsigned freeze_rfb_visuals : 1;
} pstvnc_h1_mpeg_calibration_foreground_result_t;

void pstvnc_h1_mpeg_calibration_foreground_init(
    pstvnc_h1_mpeg_calibration_foreground_t *foreground,
    int canvas_width,
    int canvas_height);

/*
 * Service exactly one normalized physical-controller observation.
 *
 * On entry, the operation callbacks run in the already-qualified order:
 * suspend -> neutralize published pointer -> rebase suspended mouse state.
 *
 * On exit, resume is delayed until the calibration core proves its release
 * quarantine complete. RFB visual freeze follows the same ownership interval.
 *
 * A callback failure returns 0 and deliberately fails closed: suspension/freeze
 * facts already established by the bridge remain asserted rather than silently
 * returning ordinary desktop ownership.
 */
int pstvnc_h1_mpeg_calibration_foreground_service_controller(
    pstvnc_h1_mpeg_calibration_foreground_t *foreground,
    const pstvnc_controller_state_t *controller_state,
    const pstvnc_h1_mpeg_calibration_foreground_ops_t *ops,
    void *ops_context,
    pstvnc_h1_mpeg_calibration_foreground_result_t *result);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_FOREGROUND_H */

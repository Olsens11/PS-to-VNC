/*
 * File synopsis:
 * Defines R34's ordinary Application-owned MPEG product coordinator. It composes
 * accepted P9 calibration, P10 protected activation, R21/R22 live-run service,
 * and R33 local session-abort service without acquiring any lower-owner
 * mechanism or normal R23/R24 retirement/reveal authority.
 *
 * Physical binding recognition remains Input-owned. This coordinator receives
 * only semantic product actions and trustworthy controller facts.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34.
 */

#ifndef PSTVNC_APP_MPEG_PRODUCT_H
#define PSTVNC_APP_MPEG_PRODUCT_H

#include <stddef.h>
#include <stdint.h>

#include "app_mpeg_activation.h"
#include "app_mpeg_calibration.h"
#include "app_mpeg_run.h"
#include "input/input.h"
#include "platform/ps2_media_clock.h"
#include "ui/local_ui.h"

typedef enum pstvnc_app_mpeg_product_result {
    PSTVNC_APP_MPEG_PRODUCT_OK = 0,
    PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE = -1,
    PSTVNC_APP_MPEG_PRODUCT_INVALID = -2
} pstvnc_app_mpeg_product_result_t;

typedef struct pstvnc_app_mpeg_product {
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_app_mpeg_calibration_t calibration;
    pstvnc_app_mpeg_run_t run;
    pstvnc_transport_access_t transport_access;
    pstvnc_media_clock_t *media_clock;
    int initialized;
} pstvnc_app_mpeg_product_t;

int pstvnc_app_mpeg_product_init(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels);

int pstvnc_app_mpeg_product_desktop_action_eligible(
    const pstvnc_app_mpeg_product_t *product,
    const pstvnc_local_ui_t *local_ui);

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_route_action(
    pstvnc_app_mpeg_product_t *product,
    pstvnc_product_action_t action,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *rfb_session,
    const pstvnc_local_ui_t *local_ui,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char *published_click_buttons,
    const uint16_t *last_presented_desktop,
    size_t last_presented_pixel_count);

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_controller(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_controller_state_t *controller_state,
    int *consumed);

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_live(
    pstvnc_app_mpeg_product_t *product,
    uint64_t current_tick);

/*
 * Report only a genuinely started R21 owner eligible for current-tick R22
 * service. Pre-START teardown debt must never satisfy this predicate.
 */
int pstvnc_app_mpeg_product_has_started_run(
    const pstvnc_app_mpeg_product_t *product);

/*
 * Report whether the current attempt owns MPEG lifecycle state that requires
 * retained-session abnormal dormancy before Transport release. This includes a
 * healthy started run plus accepted R33/R34P teardown-required generations, and
 * excludes clean IDLE/current-generation-zero rollback.
 */
int pstvnc_app_mpeg_product_requires_session_abort(
    const pstvnc_app_mpeg_product_t *product);

pstvnc_app_mpeg_product_result_t
pstvnc_app_mpeg_product_service_session_abort(
    pstvnc_app_mpeg_product_t *product,
    int *abort_ready);

#endif /* PSTVNC_APP_MPEG_PRODUCT_H */

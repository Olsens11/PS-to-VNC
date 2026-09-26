/*
 * File synopsis:
 * Proves R34's narrow Application MPEG product coordinator with deterministic
 * accepted-owner stubs. The fixture exercises semantic admission, P9 controller
 * first refusal, one-shot P10 handoff, live R22 service, overlap rejection and
 * R33 local-abort readiness without duplicating lower mechanisms.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A006-ORDINARY-MPEG-ACTION-ACTIVATION-R34.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_mpeg_product.h"

static int failures;

#define CHECK(expr) do {     if (!(expr)) {         fprintf(stderr, "CHECK failed line %d: %s\n", __LINE__, #expr);         failures++;     } } while (0)

static unsigned int calibration_init_calls;
static unsigned int calibration_begin_calls;
static unsigned int calibration_service_calls;
static unsigned int activation_calls;
static unsigned int run_service_calls;
static unsigned int run_abort_calls;
static unsigned int run_status_calls;

static pstvnc_app_mpeg_calibration_result_t calibration_begin_result;
static pstvnc_app_mpeg_calibration_result_t calibration_service_result;
static pstvnc_app_mpeg_calibration_service_result_t calibration_service_detail;
static pstvnc_app_mpeg_activation_result_t activation_result;
static pstvnc_app_mpeg_run_result_t run_service_result;
static pstvnc_app_mpeg_run_result_t run_abort_result;
static pstvnc_app_mpeg_run_state_t abort_state_after_service;
static uint64_t observed_live_tick;

int pstvnc_product_action_is_valid(pstvnc_product_action_t action)
{
    return action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
}

int pstvnc_local_ui_input_is_quarantined(const pstvnc_local_ui_t *ui)
{
    return ui != NULL && ui->input_quarantined;
}

void pstvnc_mpeg_presentation_init(pstvnc_mpeg_presentation_t *presentation)
{
    memset(presentation, 0, sizeof(*presentation));
    presentation->state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
}

pstvnc_mpeg_presentation_state_t
pstvnc_mpeg_presentation_state(const pstvnc_mpeg_presentation_t *presentation)
{
    return presentation != NULL
        ? presentation->state
        : PSTVNC_MPEG_PRESENTATION_STATE_INVALID;
}

void pstvnc_app_mpeg_run_init(pstvnc_app_mpeg_run_t *run)
{
    memset(run, 0, sizeof(*run));
    run->state = PSTVNC_APP_MPEG_RUN_IDLE;
}

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_status(
    const pstvnc_app_mpeg_run_t *run,
    pstvnc_app_mpeg_run_status_t *status)
{
    run_status_calls++;

    if (run == NULL || status == NULL)
        return PSTVNC_APP_MPEG_RUN_INVALID;

    memset(status, 0, sizeof(*status));
    status->state = run->state;
    status->last_result = run->last_result;
    status->last_allocated_generation = run->last_allocated_generation;
    status->current_generation = run->current_generation;
    status->session_teardown_required = run->session_teardown_required;
    status->retire_invoked = run->retire_invoked;
    status->retire_completion_taken = run->retire_completion_taken;
    status->producer_done_published = run->producer_done_published;
    status->worker_joined = run->worker_joined;
    status->rfb_restoration_presented = run->rfb_restoration_presented;
    status->session_abort_stop_requested =
        run->session_abort_stop_requested;
    status->session_abort_outcome_recorded =
        run->session_abort_outcome_recorded;
    status->session_abort_worker_outcome =
        run->session_abort_worker_outcome;
    return PSTVNC_APP_MPEG_RUN_OK;
}

int pstvnc_app_mpeg_calibration_init(
    pstvnc_app_mpeg_calibration_t *calibration,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels)
{
    calibration_init_calls++;

    if (calibration == NULL ||
        canvas_width <= 0 ||
        canvas_height <= 0 ||
        frozen_desktop == NULL ||
        work_surface == NULL ||
        frozen_desktop == work_surface ||
        surface_capacity_pixels == 0u)
        return 0;

    memset(calibration, 0, sizeof(*calibration));
    calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    return 1;
}

pstvnc_app_mpeg_calibration_state_t
pstvnc_app_mpeg_calibration_state(
    const pstvnc_app_mpeg_calibration_t *calibration)
{
    return calibration != NULL
        ? calibration->state
        : PSTVNC_APP_MPEG_CALIBRATION_FAULTED;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_begin(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *rfb_session,
    const pstvnc_local_ui_t *local_ui,
    pstvnc_mpeg_presentation_t *presentation,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char *published_click_buttons,
    const uint16_t *last_presented_desktop,
    size_t last_presented_pixel_count)
{
    (void)rfb_flow_policy;
    (void)input_runtime;
    (void)rfb_session;
    (void)local_ui;
    (void)presentation;
    (void)published_cursor_x;
    (void)published_cursor_y;
    (void)published_click_buttons;
    (void)last_presented_desktop;
    (void)last_presented_pixel_count;

    calibration_begin_calls++;

    if (calibration_begin_result == PSTVNC_APP_MPEG_CALIBRATION_OK)
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_ACTIVE;

    return calibration_begin_result;
}

pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_service_controller(
    pstvnc_app_mpeg_calibration_t *calibration,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_app_mpeg_calibration_service_result_t *result)
{
    (void)controller_state;
    calibration_service_calls++;

    if (result == NULL)
        return PSTVNC_APP_MPEG_CALIBRATION_INVALID;

    *result = calibration_service_detail;

    if (calibration_service_result == PSTVNC_APP_MPEG_CALIBRATION_OK) {
        if (result->accepted_protected)
            calibration->state =
                PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED;
        else if (result->cancelled_completed)
            calibration->state =
                PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    }

    return calibration_service_result;
}

pstvnc_app_mpeg_activation_result_t
pstvnc_app_mpeg_activation_start_protected(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock)
{
    (void)transport_access;
    (void)media_clock;
    activation_calls++;

    if (activation_result == PSTVNC_APP_MPEG_ACTIVATION_OK) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
        run->state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
        run->current_generation = 1u;
        run->transport_run_open = 1;
        run->worker_runtime_owned = 1;
        run->worker_started = 1;
        run->frame_consumer_initialized = 1;
        run->presentation_armed = 1;
        run->start_invoked = 1;
    } else if (
        activation_result ==
            PSTVNC_APP_MPEG_ACTIVATION_START_ROLLED_BACK) {
        calibration->state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
        run->state = PSTVNC_APP_MPEG_RUN_IDLE;
    }

    return activation_result;
}

pstvnc_app_mpeg_run_result_t pstvnc_app_mpeg_run_service(
    pstvnc_app_mpeg_run_t *run,
    uint64_t current_tick,
    pstvnc_app_mpeg_frame_service_result_t *service_result)
{
    run_service_calls++;
    observed_live_tick = current_tick;
    memset(service_result, 0, sizeof(*service_result));

    if (run_service_result == PSTVNC_APP_MPEG_RUN_OK &&
        run->state == PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME)
        run->state = PSTVNC_APP_MPEG_RUN_MPEG_OWNED;

    return run_service_result;
}

pstvnc_app_mpeg_run_result_t
pstvnc_app_mpeg_run_session_abort_service(pstvnc_app_mpeg_run_t *run)
{
    run_abort_calls++;

    if (run_abort_result == PSTVNC_APP_MPEG_RUN_OK)
        run->state = abort_state_after_service;

    return run_abort_result;
}

static void reset_fixture(void)
{
    calibration_init_calls = 0u;
    calibration_begin_calls = 0u;
    calibration_service_calls = 0u;
    activation_calls = 0u;
    run_service_calls = 0u;
    run_abort_calls = 0u;
    run_status_calls = 0u;

    calibration_begin_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    calibration_service_result = PSTVNC_APP_MPEG_CALIBRATION_OK;
    memset(&calibration_service_detail, 0, sizeof(calibration_service_detail));
    activation_result = PSTVNC_APP_MPEG_ACTIVATION_OK;
    run_service_result = PSTVNC_APP_MPEG_RUN_OK;
    run_abort_result = PSTVNC_APP_MPEG_RUN_OK;
    abort_state_after_service = PSTVNC_APP_MPEG_RUN_SESSION_ABORTING;
    observed_live_tick = 0u;
}

static int init_product(
    pstvnc_app_mpeg_product_t *product,
    pstvnc_media_clock_t *clock,
    uint16_t *frozen,
    uint16_t *work)
{
    pstvnc_transport_access_t access;

    memset(&access, 0, sizeof(access));
    access.opaque_ticket = 77u;
    memset(clock, 0, sizeof(*clock));

    return pstvnc_app_mpeg_product_init(
        product,
        &access,
        clock,
        64,
        64,
        frozen,
        work,
        4096u);
}

static void test_desktop_eligibility_tracks_real_owners(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    pstvnc_local_ui_t ui;
    uint16_t frozen[4096];
    uint16_t work[4096];

    reset_fixture();
    memset(&ui, 0, sizeof(ui));
    ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    CHECK(init_product(&product, &clock, frozen, work));
    CHECK(calibration_init_calls == 1u);
    CHECK(pstvnc_app_mpeg_product_desktop_action_eligible(&product, &ui));

    ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_OSK;
    CHECK(!pstvnc_app_mpeg_product_desktop_action_eligible(&product, &ui));

    ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;
    ui.input_quarantined = 1;
    CHECK(!pstvnc_app_mpeg_product_desktop_action_eligible(&product, &ui));

    ui.input_quarantined = 0;
    product.calibration.state = PSTVNC_APP_MPEG_CALIBRATION_ACTIVE;
    CHECK(!pstvnc_app_mpeg_product_desktop_action_eligible(&product, &ui));

    product.calibration.state = PSTVNC_APP_MPEG_CALIBRATION_IDLE;
    product.run.state = PSTVNC_APP_MPEG_RUN_MPEG_OWNED;
    CHECK(!pstvnc_app_mpeg_product_desktop_action_eligible(&product, &ui));
}

static void test_semantic_action_only_begins_idle_calibration(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    pstvnc_local_ui_t ui;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_input_runtime_t input;
    pstvnc_rfb_session_t rfb;
    uint16_t frozen[4096], work[4096], desktop[4096];
    unsigned char clicks = 0u;

    reset_fixture();
    memset(&ui, 0, sizeof(ui));
    memset(&flow, 0, sizeof(flow));
    memset(&input, 0, sizeof(input));
    memset(&rfb, 0, sizeof(rfb));
    ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    CHECK(init_product(&product, &clock, frozen, work));
    CHECK(pstvnc_app_mpeg_product_route_action(
        &product,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        &flow,
        &input,
        &rfb,
        &ui,
        100u,
        200u,
        &clicks,
        desktop,
        4096u) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(calibration_begin_calls == 1u);
    CHECK(product.calibration.state == PSTVNC_APP_MPEG_CALIBRATION_ACTIVE);

    /* An overlapping semantic request is consumed but starts nothing new. */
    CHECK(pstvnc_app_mpeg_product_route_action(
        &product,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        &flow,
        &input,
        &rfb,
        &ui,
        100u,
        200u,
        &clicks,
        desktop,
        4096u) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(calibration_begin_calls == 1u);
}

static void test_calibration_first_refusal_accepts_into_p10_once(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    pstvnc_controller_state_t controller;
    uint16_t frozen[4096], work[4096];
    int consumed = 0;

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));
    product.calibration.state = PSTVNC_APP_MPEG_CALIBRATION_ACTIVE;

    memset(&controller, 0, sizeof(controller));
    calibration_service_detail.consume_controller_state = 1u;
    calibration_service_detail.accepted_protected = 1u;

    CHECK(pstvnc_app_mpeg_product_service_controller(
        &product,
        &controller,
        &consumed) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(consumed == 1);
    CHECK(calibration_service_calls == 1u);
    CHECK(activation_calls == 1u);
    CHECK(product.run.state ==
        PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME);

    /* Once P9 handed off, later controller facts cannot invoke P10 again. */
    consumed = -1;
    CHECK(pstvnc_app_mpeg_product_service_controller(
        &product,
        &controller,
        &consumed) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(consumed == 0);
    CHECK(activation_calls == 1u);
}

static void test_calibration_cancel_never_invokes_activation(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    pstvnc_controller_state_t controller;
    uint16_t frozen[4096], work[4096];
    int consumed = 0;

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));
    product.calibration.state = PSTVNC_APP_MPEG_CALIBRATION_ACTIVE;

    memset(&controller, 0, sizeof(controller));
    calibration_service_detail.consume_controller_state = 1u;
    calibration_service_detail.cancelled_completed = 1u;

    CHECK(pstvnc_app_mpeg_product_service_controller(
        &product,
        &controller,
        &consumed) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(consumed == 1);
    CHECK(activation_calls == 0u);
    CHECK(product.calibration.state == PSTVNC_APP_MPEG_CALIBRATION_IDLE);
    CHECK(product.run.state == PSTVNC_APP_MPEG_RUN_IDLE);
}

static void test_live_service_uses_exact_tick_and_no_overlap_action(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    pstvnc_local_ui_t ui;
    pstvnc_rfb_flow_policy_t flow;
    pstvnc_input_runtime_t input;
    pstvnc_rfb_session_t rfb;
    uint16_t frozen[4096], work[4096], desktop[4096];
    unsigned char clicks = 0u;

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));
    product.run.state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    product.run.current_generation = 9u;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 1;
    product.run.worker_started = 1;
    product.run.frame_consumer_initialized = 1;
    product.run.presentation_armed = 1;
    product.run.start_invoked = 1;

    CHECK(pstvnc_app_mpeg_product_service_live(
        &product,
        UINT64_C(0x1122334455667788)) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(run_service_calls == 1u);
    CHECK(observed_live_tick == UINT64_C(0x1122334455667788));
    CHECK(product.run.state == PSTVNC_APP_MPEG_RUN_MPEG_OWNED);

    memset(&ui, 0, sizeof(ui));
    memset(&flow, 0, sizeof(flow));
    memset(&input, 0, sizeof(input));
    memset(&rfb, 0, sizeof(rfb));
    ui.foreground = PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    CHECK(pstvnc_app_mpeg_product_route_action(
        &product,
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION,
        &flow,
        &input,
        &rfb,
        &ui,
        1u,
        2u,
        &clicks,
        desktop,
        4096u) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(calibration_begin_calls == 0u);
}

static void test_session_abort_reports_only_r33_ready(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    uint16_t frozen[4096], work[4096];
    int ready = -1;

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));

    product.run.state = PSTVNC_APP_MPEG_RUN_MPEG_OWNED;
    product.run.current_generation = 4u;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 1;
    product.run.worker_started = 1;
    product.run.frame_consumer_initialized = 1;
    product.run.presentation_armed = 1;
    product.run.start_invoked = 1;

    abort_state_after_service = PSTVNC_APP_MPEG_RUN_SESSION_ABORTING;
    CHECK(pstvnc_app_mpeg_product_service_session_abort(
        &product,
        &ready) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(ready == 0);

    abort_state_after_service = PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY;
    CHECK(pstvnc_app_mpeg_product_service_session_abort(
        &product,
        &ready) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(ready == 1);
    CHECK(run_abort_calls == 2u);
}

static void test_abort_owner_is_distinct_from_live_service_owner(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    uint16_t frozen[4096], work[4096];

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));

    /* Fresh/clean R21 state has neither live-service nor abort ownership. */
    CHECK(!pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(!pstvnc_app_mpeg_product_requires_session_abort(&product));

    /* Healthy post-START owner is both live-serviceable and abort-owned. */
    product.run.state = PSTVNC_APP_MPEG_RUN_STARTED_WAIT_FIRST_FRAME;
    product.run.current_generation = 4u;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 1;
    product.run.worker_started = 1;
    product.run.frame_consumer_initialized = 1;
    product.run.presentation_armed = 1;
    product.run.start_invoked = 1;

    CHECK(pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(pstvnc_app_mpeg_product_requires_session_abort(&product));

    /*
     * Accepted R34P pre-START teardown debt must not become R22-live merely
     * because it owns a nonzero generation. It still requires two-phase abort.
     */
    product.run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    product.run.current_generation = 5u;
    product.run.session_teardown_required = 1;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 1;
    product.run.worker_started = 0;
    product.run.frame_consumer_initialized = 0;
    product.run.presentation_armed = 0;
    product.run.start_invoked = 0;

    CHECK(!pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(pstvnc_app_mpeg_product_requires_session_abort(&product));

    /*
     * The same public teardown fact persists after partial local reclaim, so a
     * retry cannot accidentally fall back to one-shot R16B abort.
     */
    product.run.state = PSTVNC_APP_MPEG_RUN_SESSION_ABORTING;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 0;
    product.run.worker_started = 0;
    CHECK(!pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(pstvnc_app_mpeg_product_requires_session_abort(&product));

    /* Clean rollback clears the current generation and therefore the debt. */
    product.run.state = PSTVNC_APP_MPEG_RUN_IDLE;
    product.run.current_generation = 0u;
    product.run.session_teardown_required = 0;
    product.run.transport_run_open = 0;
    CHECK(!pstvnc_app_mpeg_product_requires_session_abort(&product));
    CHECK(run_status_calls >= 4u);
}

static void test_prestart_abort_service_uses_same_product_seam(void)
{
    pstvnc_app_mpeg_product_t product;
    pstvnc_media_clock_t clock;
    uint16_t frozen[4096], work[4096];
    int ready = -1;

    reset_fixture();
    CHECK(init_product(&product, &clock, frozen, work));

    product.run.state = PSTVNC_APP_MPEG_RUN_FAULTED;
    product.run.current_generation = 7u;
    product.run.session_teardown_required = 1;
    product.run.transport_run_open = 1;
    product.run.worker_runtime_owned = 1;
    product.run.start_invoked = 0;

    CHECK(!pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(pstvnc_app_mpeg_product_requires_session_abort(&product));

    abort_state_after_service = PSTVNC_APP_MPEG_RUN_SESSION_ABORTING;
    CHECK(pstvnc_app_mpeg_product_service_session_abort(
        &product,
        &ready) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(ready == 0);
    CHECK(run_abort_calls == 1u);

    /*
     * Lower-owner R34P may reclaim worker/runtime state while the product still
     * owes final retained-session close. Teardown admission must remain true.
     */
    product.run.worker_runtime_owned = 0;
    abort_state_after_service = PSTVNC_APP_MPEG_RUN_SESSION_ABORT_READY;
    CHECK(pstvnc_app_mpeg_product_service_session_abort(
        &product,
        &ready) == PSTVNC_APP_MPEG_PRODUCT_OK);
    CHECK(ready == 1);
    CHECK(run_abort_calls == 2u);
    CHECK(!pstvnc_app_mpeg_product_has_started_run(&product));
    CHECK(pstvnc_app_mpeg_product_requires_session_abort(&product));
}

int main(void)
{
    test_desktop_eligibility_tracks_real_owners();
    test_semantic_action_only_begins_idle_calibration();
    test_calibration_first_refusal_accepts_into_p10_once();
    test_calibration_cancel_never_invokes_activation();
    test_live_service_uses_exact_tick_and_no_overlap_action();
    test_session_abort_reports_only_r33_ready();
    test_abort_owner_is_distinct_from_live_service_owner();
    test_prestart_abort_service_uses_same_product_seam();

    if (failures != 0) {
        fprintf(stderr, "APP_MPEG_PRODUCT_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("APP_MPEG_PRODUCT_TEST=PASS");
    return 0;
}

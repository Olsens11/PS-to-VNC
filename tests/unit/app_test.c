/*
 * File synopsis:
 * Runs the established Application host fixture plus R15 default-entry, R16B
 * provider-failure recovery, and R19 P2-flow composition assertions. The frozen
 * legacy body remains the broad lifecycle/input/cleanup regression fixture;
 * this wrapper adds bounded scripting for fresh-attempt recovery and for the
 * public RFB-flow policy seam without changing lower-layer mechanisms.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-RFB-ORDINARY-APPLICATION-ACTIVATION-R15.
 */

#define main legacy_app_test_main
#include "app_test_legacy.inc"
#undef main

#include "config/media_clock_profile.h"
#include "config/mpeg_runtime_profile.h"
#include "app_product_bindings.h"
#include "app_mpeg_product.h"

static int selected_projection_available;
static size_t selected_projection_calls;
static size_t selected_projection_event_count;
static pstvnc_transport_session_config_t selected_projection;
static int selected_mpeg_profile_available;
static size_t selected_mpeg_profile_calls;
static size_t selected_mpeg_profile_event_count;
static pstvnc_config_mpeg_runtime_profile_t selected_mpeg_profile;
static size_t selected_media_clock_profile_calls;
static size_t selected_media_clock_profile_event_count;
static pstvnc_config_media_clock_profile_t selected_media_clock_profile;

static size_t product_bindings_acquire_calls;
static size_t product_bindings_acquire_event_count;
static pstvnc_app_product_bindings_snapshot_t product_bindings_stub_snapshot;
static int product_bindings_acquire_result;

static size_t r34_binding_install_calls;
static size_t r34_desktop_eligibility_calls;
static size_t r34_product_init_calls;
static size_t r34_route_action_calls;
static size_t r34_controller_service_calls;
static size_t r34_live_service_calls;
static size_t r34_abort_service_calls;
static size_t r34_transport_access_calls;
static size_t r34_begin_abort_calls;
static size_t r34_transport_close_calls;
static size_t r34_current_tick_calls;
static pstvnc_product_action_binding_t r34_installed_binding;
static size_t r34_installed_binding_count;
static int r34_last_desktop_eligible;
static pstvnc_product_action_t r34_last_routed_action;
static int r34_desktop_eligible_at_route;
static int r34_has_started_run;
static int r34_requires_session_abort;
static pstvnc_app_mpeg_product_result_t r34_route_result;
static pstvnc_app_mpeg_product_result_t r34_controller_result;
static pstvnc_app_mpeg_product_result_t r34_live_result;
static pstvnc_app_mpeg_product_result_t r34_abort_result;
static int r34_abort_ready;
static pstvnc_transport_result_t r34_begin_abort_result;
static pstvnc_transport_result_t r34_close_result;
static uint64_t r34_current_tick_value;
static size_t r34_begin_abort_legacy_event_count;
static size_t r34_abort_service_legacy_event_count;
static size_t r34_close_legacy_event_count;

int pstvnc_config_rfb_runtime_profile_selected(
    pstvnc_transport_session_config_t *transport_config)
{
    selected_projection_calls++;
    selected_projection_event_count = event_count;

    if (!selected_projection_available || transport_config == NULL)
        return 0;

    *transport_config = selected_projection;
    return 1;
}

const pstvnc_config_mpeg_runtime_profile_t *
pstvnc_config_mpeg_runtime_profile_selected(void)
{
    selected_mpeg_profile_calls++;
    selected_mpeg_profile_event_count = event_count;
    return selected_mpeg_profile_available ? &selected_mpeg_profile : NULL;
}

pstvnc_config_media_clock_profile_t
pstvnc_config_media_clock_profile_selected(void)
{
    selected_media_clock_profile_calls++;
    selected_media_clock_profile_event_count = event_count;
    return selected_media_clock_profile;
}

int pstvnc_app_product_bindings_acquire(
    pstvnc_app_product_bindings_snapshot_t *snapshot)
{
    product_bindings_acquire_calls++;
    product_bindings_acquire_event_count = event_count;

    if (!product_bindings_acquire_result || snapshot == NULL)
        return 0;

    *snapshot = product_bindings_stub_snapshot;
    return 1;
}


pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *access)
{
    r34_transport_access_calls++;
    if (access == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    access->opaque_ticket = 0x3400u + (uint32_t)r34_transport_access_calls;
    return PSTVNC_TRANSPORT_OK;
}

int pstvnc_input_runtime_set_product_action_bindings(
    pstvnc_input_runtime_t *runtime,
    const pstvnc_product_action_binding_t *bindings,
    size_t binding_count)
{
    (void)runtime;
    r34_binding_install_calls++;
    r34_installed_binding_count = binding_count;
    memset(&r34_installed_binding, 0, sizeof(r34_installed_binding));
    if (binding_count > 0u && bindings != NULL)
        r34_installed_binding = bindings[0];
    return 0;
}

int pstvnc_input_runtime_set_product_action_desktop_eligible(
    pstvnc_input_runtime_t *runtime,
    int desktop_eligible)
{
    (void)runtime;
    r34_desktop_eligibility_calls++;
    r34_last_desktop_eligible = desktop_eligible;
    return (desktop_eligible == 0 || desktop_eligible == 1) ? 0 : -1;
}

int pstvnc_app_mpeg_product_init(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels)
{
    r34_product_init_calls++;
    if (product == NULL || transport_access == NULL ||
        transport_access->opaque_ticket == 0u || media_clock == NULL ||
        canvas_width != PSTVNC_DISPLAY_WIDTH ||
        canvas_height != PSTVNC_DISPLAY_HEIGHT ||
        frozen_desktop == NULL || work_surface == NULL ||
        frozen_desktop == work_surface ||
        surface_capacity_pixels != PSTVNC_DISPLAY_PIXEL_COUNT)
        return 0;

    memset(product, 0, sizeof(*product));
    product->initialized = 1;
    product->transport_access = *transport_access;
    product->media_clock = media_clock;
    product->run.state = PSTVNC_APP_MPEG_RUN_IDLE;
    product->presentation.state = PSTVNC_MPEG_PRESENTATION_RFB_ONLY;
    return 1;
}

int pstvnc_app_mpeg_product_desktop_action_eligible(
    const pstvnc_app_mpeg_product_t *product,
    const pstvnc_local_ui_t *local_ui)
{
    return product != NULL && product->initialized &&
        local_ui != NULL &&
        local_ui->foreground == PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP &&
        !local_ui->input_quarantined &&
        !r34_has_started_run;
}

pstvnc_app_mpeg_product_result_t pstvnc_app_mpeg_product_route_action(
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
    size_t last_presented_pixel_count)
{
    (void)product; (void)rfb_flow_policy; (void)input_runtime;
    (void)rfb_session; (void)local_ui; (void)published_cursor_x;
    (void)published_cursor_y; (void)published_click_buttons;
    (void)last_presented_desktop; (void)last_presented_pixel_count;
    r34_route_action_calls++;
    r34_last_routed_action = action;
    r34_desktop_eligible_at_route = r34_last_desktop_eligible;
    return r34_route_result;
}

pstvnc_app_mpeg_product_result_t pstvnc_app_mpeg_product_service_controller(
    pstvnc_app_mpeg_product_t *product,
    const pstvnc_controller_state_t *controller_state,
    int *consumed)
{
    (void)product; (void)controller_state;
    r34_controller_service_calls++;
    if (consumed != NULL)
        *consumed = 0;
    return r34_controller_result;
}

pstvnc_app_mpeg_product_result_t pstvnc_app_mpeg_product_service_live(
    pstvnc_app_mpeg_product_t *product,
    uint64_t current_tick)
{
    (void)product;
    r34_live_service_calls++;
    CHECK(current_tick == r34_current_tick_value);
    return r34_live_result;
}

int pstvnc_app_mpeg_product_has_started_run(
    const pstvnc_app_mpeg_product_t *product)
{
    return product != NULL && product->initialized && r34_has_started_run;
}

int pstvnc_app_mpeg_product_requires_session_abort(
    const pstvnc_app_mpeg_product_t *product)
{
    return product != NULL &&
        product->initialized &&
        r34_requires_session_abort;
}

pstvnc_app_mpeg_product_result_t pstvnc_app_mpeg_product_service_session_abort(
    pstvnc_app_mpeg_product_t *product,
    int *abort_ready)
{
    (void)product;
    r34_abort_service_calls++;
    r34_abort_service_legacy_event_count = event_count;
    if (abort_ready != NULL)
        *abort_ready = r34_abort_ready;
    return r34_abort_result;
}

pstvnc_transport_result_t pstvnc_transport_session_begin_abort(void)
{
    r34_begin_abort_calls++;
    r34_begin_abort_legacy_event_count = event_count;
    return r34_begin_abort_result;
}

pstvnc_transport_result_t pstvnc_transport_session_close(void)
{
    r34_transport_close_calls++;
    r34_close_legacy_event_count = event_count;
    return r34_close_result;
}

int pstvnc_ps2_media_clock_binding_current_tick(
    const pstvnc_ps2_media_clock_binding_t *binding,
    uint64_t *tick)
{
    r34_current_tick_calls++;
    if (binding == NULL || tick == NULL)
        return -1;
    *tick = r34_current_tick_value;
    return 0;
}

static void reset_selected_projection(void)
{
    selected_projection_available = 0;
    selected_projection_calls = 0u;
    selected_projection_event_count = 0u;
    memset(&selected_projection, 0, sizeof(selected_projection));

    selected_mpeg_profile_available = 0;
    selected_mpeg_profile_calls = 0u;
    selected_mpeg_profile_event_count = 0u;
    memset(&selected_mpeg_profile, 0, sizeof(selected_mpeg_profile));

    selected_media_clock_profile_calls = 0u;
    selected_media_clock_profile_event_count = 0u;
    memset(&selected_media_clock_profile, 0, sizeof(selected_media_clock_profile));

    product_bindings_acquire_calls = 0u;
    product_bindings_acquire_event_count = 0u;
    memset(&product_bindings_stub_snapshot, 0, sizeof(product_bindings_stub_snapshot));
    product_bindings_stub_snapshot.status =
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID;
    product_bindings_acquire_result = 1;

    r34_binding_install_calls = 0u;
    r34_desktop_eligibility_calls = 0u;
    r34_product_init_calls = 0u;
    r34_route_action_calls = 0u;
    r34_controller_service_calls = 0u;
    r34_live_service_calls = 0u;
    r34_abort_service_calls = 0u;
    r34_transport_access_calls = 0u;
    r34_begin_abort_calls = 0u;
    r34_transport_close_calls = 0u;
    r34_current_tick_calls = 0u;
    memset(&r34_installed_binding, 0, sizeof(r34_installed_binding));
    r34_installed_binding_count = 0u;
    r34_last_desktop_eligible = -1;
    r34_last_routed_action = PSTVNC_PRODUCT_ACTION_NONE;
    r34_desktop_eligible_at_route = -1;
    r34_has_started_run = 0;
    r34_requires_session_abort = 0;
    r34_route_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_controller_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_live_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_abort_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_abort_ready = 1;
    r34_begin_abort_result = PSTVNC_TRANSPORT_OK;
    r34_close_result = PSTVNC_TRANSPORT_OK;
    r34_current_tick_value = UINT64_C(0x123456789abcdef0);
    r34_begin_abort_legacy_event_count = 0u;
    r34_abort_service_legacy_event_count = 0u;
    r34_close_legacy_event_count = 0u;
}

static int nth_event_index(event_id_t event, size_t occurrence)
{
    size_t i;
    size_t seen = 0u;

    for (i = 0; i < event_count; i++) {
        if (events[i] != event)
            continue;
        if (seen == occurrence)
            return (int)i;
        seen++;
    }

    return -1;
}

static int event_occurs_between(
    event_id_t event,
    int after_index,
    int before_index)
{
    int i;

    if (after_index < 0 || before_index < 0 || after_index >= before_index)
        return 0;

    for (i = after_index + 1; i < before_index; i++) {
        if (events[i] == event)
            return 1;
    }

    return 0;
}

static size_t diagnostic_occurrences_of(const char *text)
{
    size_t i;
    size_t count = 0u;

    for (i = 0; i < diagnostic_count; i++) {
        if (strcmp(diagnostics[i], text) == 0)
            count++;
    }

    return count;
}

static void script_fresh_connections(size_t count)
{
    size_t i;

    CHECK(count <= sizeof(connect_results) / sizeof(connect_results[0]));
    connect_result_count = count;

    for (i = 0; i < count; i++)
        connect_results[i] = 7 + (int)i;
}

static void test_r16b_provider_connect_failure_restarts_fresh(void)
{
    reset_script();
    script_fresh_connections(2u);

    session_start_results[0] = 0;
    session_start_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_CONNECT;
    session_start_results[1] = 1;
    session_start_result_count = 2u;

    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 2u);
    CHECK(transport_open_calls == 2u);
    CHECK(transport_open_socket_fds[0] == 7);
    CHECK(transport_open_socket_fds[1] == 8);
    CHECK(event_occurrences(EV_SESSION_START) == 2u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 2u);
    CHECK(event_occurrences(EV_INPUT_INIT) == 1u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE INPUT_READY") == 1u);
    CHECK(nth_event_index(EV_TRANSPORT_ABORT, 0u) <
          nth_event_index(EV_CONNECT_PSTV, 1u));
}

static void test_r16b_provider_read_closes_admission_before_restart(void)
{
    int failed_receive;
    int first_shutdown;
    int first_abort;
    int second_connect;

    reset_script();
    script_fresh_connections(2u);

    request_results[0] = 1;
    request_results[1] = 1;
    request_result_count = 2u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);

    failed_receive = nth_event_index(EV_TRY_RECEIVE, 0u);
    first_shutdown = nth_event_index(EV_INPUT_SHUTDOWN, 0u);
    first_abort = nth_event_index(EV_TRANSPORT_ABORT, 0u);
    second_connect = nth_event_index(EV_CONNECT_PSTV, 1u);

    CHECK(failed_receive >= 0);
    CHECK(failed_receive < first_shutdown);
    CHECK(first_shutdown < first_abort);
    CHECK(first_abort < second_connect);
    CHECK(!event_occurs_between(EV_INPUT_POP, failed_receive, first_shutdown));
    CHECK(!event_occurs_between(EV_KEY_SEND, failed_receive, first_shutdown));
    CHECK(!event_occurs_between(EV_POINTER_SEND, failed_receive, first_shutdown));
    CHECK(!event_occurs_between(EV_REQUEST_UPDATE, failed_receive, first_shutdown));
    CHECK(connect_calls == 2u);
    CHECK(event_occurrences(EV_INPUT_INIT) == 2u);
    CHECK(event_occurrences(EV_INPUT_START) == 2u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 2u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 2u);
    CHECK(event_occurrences(EV_FB_INIT) == 2u);
    CHECK(event_occurrences(EV_INITIAL_FRAME) == 2u);
    CHECK(event_occurrences(EV_GRAPHICS_INIT) == 1u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE INPUT_READY") == 2u);
}

static void test_r16b_provider_write_failure_restarts_fresh(void)
{
    int failed_request;
    int first_shutdown;
    int first_abort;
    int second_connect;

    reset_script();
    script_fresh_connections(2u);

    request_results[0] = 0;
    request_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_WRITE;
    request_results[1] = 1;
    request_result_count = 2u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);

    failed_request = nth_event_index(EV_REQUEST_UPDATE, 0u);
    first_shutdown = nth_event_index(EV_INPUT_SHUTDOWN, 0u);
    first_abort = nth_event_index(EV_TRANSPORT_ABORT, 0u);
    second_connect = nth_event_index(EV_CONNECT_PSTV, 1u);

    CHECK(failed_request >= 0);
    CHECK(failed_request < first_shutdown);
    CHECK(first_shutdown < first_abort);
    CHECK(first_abort < second_connect);
    CHECK(!event_occurs_between(EV_INPUT_POP, failed_request, first_shutdown));
    CHECK(!event_occurs_between(EV_TRY_RECEIVE, failed_request, first_shutdown));
    CHECK(connect_calls == 2u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 2u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 2u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE INPUT_READY") == 2u);
}

static void test_r16b_repeated_provider_cycles_rebuild_attempt_state(void)
{
    size_t i;

    reset_script();
    script_fresh_connections(3u);

    request_results[0] = 1;
    request_results[1] = 0;
    request_errors[1] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_WRITE;
    request_results[2] = 1;
    request_result_count = 3u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 3u);
    CHECK(transport_open_calls == 3u);
    CHECK(transport_open_socket_fds[0] == 7);
    CHECK(transport_open_socket_fds[1] == 8);
    CHECK(transport_open_socket_fds[2] == 9);
    CHECK(event_occurrences(EV_SESSION_INIT) == 3u);
    CHECK(event_occurrences(EV_SESSION_START) == 3u);
    CHECK(event_occurrences(EV_FB_INIT) == 3u);
    CHECK(event_occurrences(EV_INITIAL_FRAME) == 3u);
    CHECK(event_occurrences(EV_INPUT_INIT) == 3u);
    CHECK(event_occurrences(EV_INPUT_START) == 3u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 3u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 3u);
    CHECK(event_occurrences(EV_GRAPHICS_INIT) == 1u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE DESKTOP_READY") == 3u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE INPUT_READY") == 3u);

    CHECK(pointer_call_count == 3u);
    for (i = 0; i < pointer_call_count; i++) {
        CHECK(pointer_calls[i].button_mask == 0);
        CHECK(pointer_calls[i].x == PSTVNC_DISPLAY_WIDTH / 2u);
        CHECK(pointer_calls[i].y == PSTVNC_DISPLAY_HEIGHT / 2u);
    }

    CHECK(nth_event_index(EV_TRANSPORT_ABORT, 0u) <
          nth_event_index(EV_CONNECT_PSTV, 1u));
    CHECK(nth_event_index(EV_TRANSPORT_ABORT, 1u) <
          nth_event_index(EV_CONNECT_PSTV, 2u));
}

static void test_r16b_generic_rfb_io_remains_fatal_not_provider_recovery(void)
{
    reset_script();
    script_fresh_connections(2u);

    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 1u);
    CHECK(transport_open_calls == 1u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE FATAL") == 1u);
}

static void test_r16b_unproven_input_stop_blocks_replacement(void)
{
    reset_script();
    script_fresh_connections(2u);

    input_shutdown_result = -1;
    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 1u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 2u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE FATAL") == 1u);
}

static void test_r16b_unproven_transport_stop_blocks_replacement(void)
{
    reset_script();
    script_fresh_connections(2u);

    transport_abort_result = PSTVNC_TRANSPORT_FAILED;
    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 1u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 2u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE FATAL") == 1u);
}

static void test_r15_projection_failure_precedes_all_startup(void)
{
    reset_script();
    reset_selected_projection();

    CHECK(pstvnc_app_run() == -1);
    CHECK(selected_projection_calls == 1u);
    CHECK(selected_projection_event_count == 0u);
    CHECK(event_count == 0u);
    CHECK(transport_open_calls == 0u);
}

static void test_r27_mpeg_profile_failure_precedes_all_startup(void)
{
    reset_script();
    reset_selected_projection();

    selected_projection_available = 1;
    selected_projection = test_transport_config;

    CHECK(pstvnc_app_run() == -1);
    CHECK(selected_projection_calls == 1u);
    CHECK(selected_mpeg_profile_calls == 1u);
    CHECK(selected_media_clock_profile_calls == 0u);
    CHECK(selected_projection_event_count == 0u);
    CHECK(selected_mpeg_profile_event_count == 0u);
    CHECK(event_count == 0u);
    CHECK(connect_calls == 0u);
    CHECK(transport_open_calls == 0u);
    CHECK(media_binding_init_calls == 0u);
}

static void test_r15_selected_profile_reaches_existing_lifecycle_exactly(void)
{
    reset_script();
    reset_selected_projection();

    selected_projection_available = 1;
    selected_projection.rfb_queue_capacity = 32768u;
    selected_projection.rfb_initial_credit_bytes = 32768u;
    selected_projection.rfb_credit_batch_bytes = 8192u;
    selected_projection.rfb_credit_flush_on_empty = 1;
    selected_projection.rfb_credit_return_enabled = 1;
    selected_projection.receiver_thread_stack_size = 16384u;
    selected_projection.receiver_thread_priority = 63;
    selected_projection.max_data_payload = 8192u;

    selected_mpeg_profile_available = 1;
    selected_mpeg_profile.transport.queue_capacity = 65536u;
    selected_mpeg_profile.transport.initial_credit_bytes = 49152u;
    selected_mpeg_profile.transport.credit_batch_bytes = 16384u;
    selected_mpeg_profile.transport.credit_flush_on_empty = 1;
    selected_mpeg_profile.transport.credit_return_enabled = 1;

    selected_media_clock_profile.epoch_lead_us = 0u;
    selected_media_clock_profile.audio_presentation_offset_us = 0;
    selected_media_clock_profile.video_presentation_offset_us = 0;

    /* Stop at Transport admission after proving every selected value was
     * acquired before the first platform startup event and forwarded exactly. */
    transport_open_result = PSTVNC_TRANSPORT_FAILED;
    transport_open_adopts = 0;

    CHECK(pstvnc_app_run() == -1);
    CHECK(selected_projection_calls == 1u);
    CHECK(selected_mpeg_profile_calls == 1u);
    CHECK(selected_media_clock_profile_calls == 1u);
    CHECK(selected_projection_event_count == 0u);
    CHECK(selected_mpeg_profile_event_count == 0u);
    CHECK(selected_media_clock_profile_event_count == 0u);
    CHECK(event_count > 0u);
    CHECK(events[0] == EV_PREPARE_IOP);
    CHECK(transport_open_calls == 1u);
    CHECK(plain_transport_open_calls == 0u);
    CHECK(observed_transport_config.rfb_queue_capacity == 32768u);
    CHECK(observed_transport_config.rfb_initial_credit_bytes == 32768u);
    CHECK(observed_transport_config.rfb_credit_batch_bytes == 8192u);
    CHECK(observed_transport_config.rfb_credit_flush_on_empty == 1);
    CHECK(observed_transport_config.rfb_credit_return_enabled == 1);
    CHECK(observed_transport_config.receiver_thread_stack_size == 16384u);
    CHECK(observed_transport_config.receiver_thread_priority == 63);
    CHECK(observed_transport_config.max_data_payload == 8192u);
    CHECK(observed_mpeg_transport_config.queue_capacity == 65536u);
    CHECK(observed_mpeg_transport_config.initial_credit_bytes == 49152u);
    CHECK(observed_mpeg_transport_config.credit_batch_bytes == 16384u);
    CHECK(observed_mpeg_transport_config.credit_flush_on_empty == 1);
    CHECK(observed_mpeg_transport_config.credit_return_enabled == 1);
    CHECK(media_binding_init_calls == 0u);
}

static void test_r32_acquisition_occurs_once_after_link_before_first_pstv(void)
{
    int wait_link;
    int first_connect;

    reset_script();
    reset_selected_projection();
    transport_open_result = PSTVNC_TRANSPORT_FAILED;
    transport_open_adopts = 0;

    CHECK(run_configured_app() == -1);

    wait_link = nth_event_index(EV_WAIT_LINK, 0u);
    first_connect = nth_event_index(EV_CONNECT_PSTV, 0u);

    CHECK(product_bindings_acquire_calls == 1u);
    CHECK(wait_link >= 0);
    CHECK(first_connect >= 0);
    CHECK(product_bindings_acquire_event_count == (size_t)(wait_link + 1));
    CHECK((int)product_bindings_acquire_event_count <= first_connect);
}

static void test_r32_provider_replacement_reuses_resident_snapshot(void)
{
    pstvnc_app_product_bindings_snapshot_t original_snapshot;

    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);

    product_bindings_stub_snapshot.status =
        PSTVNC_APP_PRODUCT_BINDINGS_STATUS_VALID;
    product_bindings_stub_snapshot.desired.binding_count = 1u;
    product_bindings_stub_snapshot.desired.bindings[0].button_mask =
        PSTVNC_CONTROLLER_BUTTON_L2 | PSTVNC_CONTROLLER_BUTTON_TRIANGLE;
    product_bindings_stub_snapshot.desired.bindings[0].action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    product_bindings_stub_snapshot.desired.bindings[0].trigger =
        PSTVNC_PRODUCT_ACTION_TRIGGER_RELEASE;
    product_bindings_stub_snapshot.desired.bindings[0].context =
        PSTVNC_PRODUCT_ACTION_CONTEXT_GLOBAL;
    original_snapshot = product_bindings_stub_snapshot;

    request_results[0] = 1;
    request_results[1] = 1;
    request_result_count = 2u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);
    CHECK(connect_calls == 2u);
    CHECK(product_bindings_acquire_calls == 1u);
    CHECK(memcmp(
        &product_bindings_stub_snapshot,
        &original_snapshot,
        sizeof(original_snapshot)) == 0);
}

static void test_r32_acquisition_internal_failure_is_resident_fatal(void)
{
    reset_script();
    reset_selected_projection();
    product_bindings_acquire_result = 0;

    CHECK(run_configured_app() == -1);
    CHECK(product_bindings_acquire_calls == 1u);
    CHECK(connect_calls == 0u);
    CHECK(transport_open_calls == 0u);

    /* Keep this one injected invariant failure isolated from later regressions. */
    product_bindings_acquire_result = 1;
}

static void test_r19_thawed_dirty_update_completes_presents_and_reschedules(void)
{
    int first_next;
    int first_request;
    int first_record;
    int receive;
    int completion;
    int publication;
    int second_next;
    int second_request;

    reset_script();

    request_results[0] = 1;
    request_results[1] = 0;
    request_result_count = 2u;

    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_UPDATE;
    try_receive_valid[0] = 1;
    try_receive_dirty[0] = 1;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_init_calls == 1u);
    CHECK(flow_next_calls == 2u);
    CHECK(flow_record_sent_calls == 1u);
    CHECK(flow_update_complete_calls == 1u);
    CHECK(flow_publication_query_calls == 1u);
    CHECK(request_calls == 2u);
    CHECK(request_incremental[0] == 1);
    CHECK(request_incremental[1] == 1);
    CHECK(flow_recorded_requests[0] ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
    CHECK(present_call_count == 2u);

    first_next = nth_event_index(EV_FLOW_NEXT_REQUEST, 0u);
    first_request = nth_event_index(EV_REQUEST_UPDATE, 0u);
    first_record = nth_event_index(EV_FLOW_RECORD_SENT, 0u);
    receive = nth_event_index(EV_TRY_RECEIVE, 0u);
    completion = nth_event_index(EV_FLOW_UPDATE_COMPLETE, 0u);
    publication = nth_event_index(EV_FLOW_PUBLICATION_QUERY, 0u);
    second_next = nth_event_index(EV_FLOW_NEXT_REQUEST, 1u);
    second_request = nth_event_index(EV_REQUEST_UPDATE, 1u);

    CHECK(first_next < first_request);
    CHECK(first_request < first_record);
    CHECK(first_record < receive);
    CHECK(receive < completion);
    CHECK(completion < publication);
    CHECK(publication < second_next);
    CHECK(second_next < second_request);
}

static void test_r19_failed_send_never_records_policy_send(void)
{
    reset_script();

    request_results[0] = 0;
    request_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_init_calls == 1u);
    CHECK(flow_next_calls == 1u);
    CHECK(request_calls == 1u);
    CHECK(flow_record_sent_calls == 0u);
    CHECK(flow_update_complete_calls == 0u);
    CHECK(try_receive_calls == 0u);
}

static void test_r19_idle_preserves_outstanding_without_reschedule(void)
{
    reset_script();

    request_results[0] = 1;
    request_result_count = 1u;

    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_IDLE;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_init_calls == 1u);
    CHECK(flow_next_calls == 1u);
    CHECK(flow_record_sent_calls == 1u);
    CHECK(flow_update_complete_calls == 0u);
    CHECK(request_calls == 1u);
    CHECK(idle_delay_calls == 1u);
}

static void test_r19_hold_is_no_send_no_accounting(void)
{
    reset_script();

    flow_next_script[0] = PSTVNC_RFB_FLOW_REQUEST_HOLD;
    flow_next_script_count = 1u;

    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_IDLE;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_init_calls == 1u);
    CHECK(flow_next_calls == 1u);
    CHECK(request_calls == 0u);
    CHECK(flow_record_sent_calls == 0u);
    CHECK(flow_update_complete_calls == 0u);
    CHECK(idle_delay_calls == 1u);
}

static void test_r19_full_policy_decision_maps_to_nonincremental_request(void)
{
    reset_script();

    flow_next_script[0] = PSTVNC_RFB_FLOW_REQUEST_FULL;
    flow_next_script_count = 1u;
    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_next_calls == 1u);
    CHECK(request_calls == 1u);
    CHECK(request_incremental[0] == 0);
    CHECK(flow_record_sent_calls == 1u);
    CHECK(flow_recorded_requests[0] == PSTVNC_RFB_FLOW_REQUEST_FULL);
}

static void test_r19_publication_gate_suppresses_dirty_remote_present(void)
{
    reset_script();

    flow_publication_allowed = 0;
    request_results[0] = 1;
    request_results[1] = 0;
    request_result_count = 2u;

    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_UPDATE;
    try_receive_valid[0] = 1;
    try_receive_dirty[0] = 1;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_update_complete_calls == 1u);
    CHECK(flow_publication_query_calls == 1u);
    CHECK(request_calls == 2u);
    CHECK(present_call_count == 1u);
    CHECK(event_occurrences(EV_DISPLAY_PREPARE) == 1u);
}

static void test_r19_provider_replacement_reinitializes_flow_policy(void)
{
    size_t i;

    reset_script();
    script_fresh_connections(2u);

    request_results[0] = 1;
    request_results[1] = 1;
    request_result_count = 2u;

    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);

    CHECK(flow_init_calls == 2u);
    CHECK(flow_next_calls == 2u);
    CHECK(flow_record_sent_calls == 2u);
    CHECK(request_calls == 2u);

    for (i = 0u; i < 2u; i++) {
        CHECK(flow_init_snapshot_frozen[i] == 0u);
        CHECK(flow_init_snapshot_outstanding[i] == 0u);
        CHECK(flow_init_snapshot_full_pending[i] == 0u);
        CHECK(flow_recorded_requests[i] ==
            PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
        CHECK(request_incremental[i] == 1);
    }
}

static void test_r19_policy_accounting_rejection_fails_before_receive(void)
{
    reset_script();

    flow_record_sent_allowed = 0;
    request_results[0] = 1;
    request_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(request_calls == 1u);
    CHECK(flow_record_sent_calls == 1u);
    CHECK(try_receive_calls == 0u);
    CHECK(diagnostic_occurrences_of("PSTVNC_STAGE FATAL") == 1u);
}


static void check_media_profile_observation(size_t index)
{
    CHECK(index < media_clock_init_calls);
    if (index >= media_clock_init_calls)
        return;

    CHECK(media_clock_observed_profiles[index].epoch_lead_us == 0u);
    CHECK(
        media_clock_observed_profiles[index].audio_presentation_offset_us == 0);
    CHECK(
        media_clock_observed_profiles[index].video_presentation_offset_us == 0);
    CHECK(media_clock_observed_tick_rates[index] == 147456000u);
    CHECK(media_clock_sync_contexts[index] != NULL);
}

static void test_r27_session_foundation_is_fresh_unarmed_and_mpeg_capable(void)
{
    reset_script();
    session_start_result = 0;

    CHECK(run_configured_app() == -1);
    CHECK(transport_open_calls == 1u);
    CHECK(plain_transport_open_calls == 0u);
    CHECK(media_binding_init_calls == 1u);
    CHECK(media_clock_init_calls == 1u);
    CHECK(media_clock_is_armed_calls == 1u);
    CHECK(media_binding_release_calls == 1u);
    check_config_was_forwarded();
    check_media_profile_observation(0u);

    CHECK(
        nth_event_index(EV_TRANSPORT_OPEN, 0u) <
        nth_event_index(EV_MEDIA_BINDING_INIT, 0u));
    CHECK(
        nth_event_index(EV_MEDIA_BINDING_INIT, 0u) <
        nth_event_index(EV_MEDIA_CLOCK_INIT, 0u));
    CHECK(
        nth_event_index(EV_MEDIA_CLOCK_INIT, 0u) <
        nth_event_index(EV_MEDIA_CLOCK_IS_ARMED, 0u));
    CHECK(
        nth_event_index(EV_MEDIA_CLOCK_IS_ARMED, 0u) <
        nth_event_index(EV_SESSION_START, 0u));
    CHECK(
        nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u) <
        nth_event_index(EV_TRANSPORT_ABORT, 0u));
}

static void test_r27_provider_replacement_retires_clock_before_transport(void)
{
    int first_shutdown;
    int first_release;
    int first_abort;
    int second_connect;

    reset_script();
    script_fresh_connections(2u);

    request_results[0] = 1;
    request_results[1] = 1;
    request_result_count = 2u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);

    CHECK(connect_calls == 2u);
    CHECK(transport_open_calls == 2u);
    CHECK(plain_transport_open_calls == 0u);
    CHECK(media_binding_init_calls == 2u);
    CHECK(media_clock_init_calls == 2u);
    CHECK(media_clock_is_armed_calls == 2u);
    CHECK(media_binding_release_calls == 2u);
    CHECK(media_binding_assigned_ids[0] != 0);
    CHECK(media_binding_assigned_ids[1] != 0);
    CHECK(media_binding_assigned_ids[0] != media_binding_assigned_ids[1]);
    check_media_profile_observation(0u);
    check_media_profile_observation(1u);

    first_shutdown = nth_event_index(EV_INPUT_SHUTDOWN, 0u);
    first_release = nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u);
    first_abort = nth_event_index(EV_TRANSPORT_ABORT, 0u);
    second_connect = nth_event_index(EV_CONNECT_PSTV, 1u);

    CHECK(first_shutdown >= 0);
    CHECK(first_shutdown < first_release);
    CHECK(first_release < first_abort);
    CHECK(first_abort < second_connect);
}

static void test_r34_exact_binding_installs_on_each_fresh_input_runtime(void)
{
    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);

    product_bindings_stub_snapshot.desired.binding_count = 1u;
    product_bindings_stub_snapshot.desired.bindings[0].button_mask =
        PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_CIRCLE;
    product_bindings_stub_snapshot.desired.bindings[0].action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    product_bindings_stub_snapshot.desired.bindings[0].trigger =
        PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD;
    product_bindings_stub_snapshot.desired.bindings[0].context =
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP;

    request_results[0] = 1;
    request_results[1] = 1;
    request_result_count = 2u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_results[1] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[1] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 2u;

    CHECK(run_configured_app() == -1);
    CHECK(r34_binding_install_calls == 2u);
    CHECK(r34_installed_binding_count == 1u);
    CHECK(r34_installed_binding.button_mask ==
        (PSTVNC_CONTROLLER_BUTTON_L1 | PSTVNC_CONTROLLER_BUTTON_CIRCLE));
    CHECK(r34_installed_binding.action ==
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(r34_installed_binding.trigger == PSTVNC_PRODUCT_ACTION_TRIGGER_HOLD);
    CHECK(r34_installed_binding.context ==
        PSTVNC_PRODUCT_ACTION_CONTEXT_DESKTOP);
    CHECK(r34_product_init_calls == 2u);
    CHECK(r34_transport_access_calls == 2u);
    CHECK(r34_desktop_eligibility_calls >= 2u);
}

static void test_r34_semantic_product_action_routes_without_physical_logic(void)
{
    reset_script();
    reset_selected_projection();

    input_events[0].type = PSTVNC_INPUT_EVENT_PRODUCT_ACTION;
    input_events[0].payload.product_action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    input_event_count = 1u;

    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_IO;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);
    CHECK(r34_route_action_calls == 1u);
    CHECK(r34_last_routed_action == PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION);
    CHECK(r34_desktop_eligible_at_route == 0);
}

static void test_r34_live_failure_uses_r33_before_release_and_close(void)
{
    int shutdown_index;
    int release_index;

    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);
    connect_results[1] = -1;

    r34_has_started_run = 1;
    r34_live_result = PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;
    r34_abort_ready = 1;

    request_results[0] = 1;
    request_result_count = 1u;

    CHECK(run_configured_app() == -1);

    shutdown_index = nth_event_index(EV_INPUT_SHUTDOWN, 0u);
    release_index = nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u);

    CHECK(r34_current_tick_calls == 1u);
    CHECK(r34_live_service_calls == 1u);
    CHECK(r34_begin_abort_calls == 1u);
    CHECK(r34_abort_service_calls == 1u);
    CHECK(r34_transport_close_calls == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 0u);

    CHECK(shutdown_index >= 0);
    CHECK((size_t)(shutdown_index + 1) <= r34_begin_abort_legacy_event_count);
    CHECK(r34_begin_abort_legacy_event_count <=
        r34_abort_service_legacy_event_count);
    CHECK(release_index >= 0);
    CHECK(r34_abort_service_legacy_event_count <= (size_t)release_index);
    CHECK((size_t)(release_index + 1) <= r34_close_legacy_event_count);

    /* Do not leak this R34-only scripted live owner into legacy scenarios. */
    r34_has_started_run = 0;
    r34_live_result = PSTVNC_APP_MPEG_PRODUCT_OK;
}

static void test_r34c_prestart_teardown_uses_r34p_before_release_and_close(void)
{
    int shutdown_index;
    int release_index;

    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);
    connect_results[1] = -1;

    /*
     * Route one semantic action into a deterministic P10 teardown-required
     * result. It owns R34P abnormal teardown but is deliberately not an R22
     * live-serviceable started run.
     */
    input_events[0].type = PSTVNC_INPUT_EVENT_PRODUCT_ACTION;
    input_events[0].payload.product_action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    input_event_count = 1u;
    r34_route_result = PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;
    r34_requires_session_abort = 1;
    r34_has_started_run = 0;
    r34_abort_ready = 1;

    request_results[0] = 1;
    request_result_count = 1u;

    CHECK(run_configured_app() == -1);

    shutdown_index = nth_event_index(EV_INPUT_SHUTDOWN, 0u);
    release_index = nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u);

    CHECK(r34_route_action_calls == 1u);
    CHECK(r34_current_tick_calls == 0u);
    CHECK(r34_live_service_calls == 0u);
    CHECK(r34_begin_abort_calls == 1u);
    CHECK(r34_abort_service_calls == 1u);
    CHECK(r34_transport_close_calls == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 0u);

    CHECK(shutdown_index >= 0);
    CHECK((size_t)(shutdown_index + 1) <=
        r34_begin_abort_legacy_event_count);
    CHECK(r34_begin_abort_legacy_event_count <=
        r34_abort_service_legacy_event_count);
    CHECK(release_index >= 0);
    CHECK(r34_abort_service_legacy_event_count <= (size_t)release_index);
    CHECK((size_t)(release_index + 1) <= r34_close_legacy_event_count);

    r34_route_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_requires_session_abort = 0;
}

static void test_r34c_prestart_input_shutdown_failure_blocks_begin_abort(void)
{
    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);

    input_events[0].type = PSTVNC_INPUT_EVENT_PRODUCT_ACTION;
    input_events[0].payload.product_action =
        PSTVNC_PRODUCT_ACTION_MPEG_CALIBRATION;
    input_event_count = 1u;
    r34_route_result = PSTVNC_APP_MPEG_PRODUCT_SESSION_FAILURE;
    r34_requires_session_abort = 1;
    r34_has_started_run = 0;
    input_shutdown_result = -1;

    request_results[0] = 1;
    request_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(connect_calls == 1u);
    CHECK(event_occurrences(EV_INPUT_SHUTDOWN) == 2u);
    CHECK(r34_begin_abort_calls == 0u);
    CHECK(r34_abort_service_calls == 0u);
    CHECK(r34_transport_close_calls == 0u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 0u);
    CHECK(r34_current_tick_calls == 0u);
    CHECK(r34_live_service_calls == 0u);

    input_shutdown_result = 0;
    r34_route_result = PSTVNC_APP_MPEG_PRODUCT_OK;
    r34_requires_session_abort = 0;
}

static void test_r34c_clean_no_mpeg_failure_keeps_one_shot_abort(void)
{
    reset_script();
    reset_selected_projection();
    script_fresh_connections(2u);
    connect_results[1] = -1;

    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_result_count = 1u;

    r34_has_started_run = 0;
    r34_requires_session_abort = 0;

    CHECK(run_configured_app() == -1);
    CHECK(r34_begin_abort_calls == 0u);
    CHECK(r34_abort_service_calls == 0u);
    CHECK(r34_transport_close_calls == 0u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
}

static void test_r27_binding_release_failure_blocks_replacement(void)
{
    reset_script();
    script_fresh_connections(2u);

    media_binding_release_result = -1;
    request_results[0] = 1;
    request_result_count = 1u;
    try_receive_results[0] = PSTVNC_RFB_SESSION_RECEIVE_FAILED;
    try_receive_errors[0] = PSTVNC_RFB_SESSION_ERROR_PROVIDER_READ;
    try_receive_result_count = 1u;

    CHECK(run_configured_app() == -1);

    CHECK(connect_calls == 1u);
    CHECK(transport_open_calls == 1u);
    CHECK(media_binding_init_calls == 1u);
    CHECK(media_binding_release_calls == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
    CHECK(
        nth_event_index(EV_INPUT_SHUTDOWN, 0u) <
        nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u));
    CHECK(
        nth_event_index(EV_MEDIA_BINDING_RELEASE, 0u) <
        nth_event_index(EV_TRANSPORT_ABORT, 0u));
}

static void test_r27_prebinding_failure_never_releases_clock(void)
{
    reset_script();
    transport_open_result = PSTVNC_TRANSPORT_FAILED;
    transport_open_adopts = 0;

    CHECK(run_configured_app() == -1);
    CHECK(transport_open_calls == 1u);
    CHECK(media_binding_init_calls == 0u);
    CHECK(media_clock_init_calls == 0u);
    CHECK(media_binding_release_calls == 0u);
}

static void test_r27_binding_init_failure_aborts_transport_without_release(void)
{
    reset_script();
    media_binding_init_result = -1;

    CHECK(run_configured_app() == -1);
    CHECK(transport_open_calls == 1u);
    CHECK(media_binding_init_calls == 1u);
    CHECK(media_clock_init_calls == 0u);
    CHECK(media_binding_release_calls == 0u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
}

static void test_r27_clock_must_begin_unarmed(void)
{
    reset_script();
    media_clock_initial_armed = 1;

    CHECK(run_configured_app() == -1);
    CHECK(media_binding_init_calls == 1u);
    CHECK(media_clock_init_calls == 1u);
    CHECK(media_clock_is_armed_calls == 1u);
    CHECK(event_occurrences(EV_SESSION_START) == 0u);
    CHECK(media_binding_release_calls == 1u);
    CHECK(event_occurrences(EV_TRANSPORT_ABORT) == 1u);
}

int main(void)
{
    int legacy_result;

    reset_selected_projection();
    legacy_result = legacy_app_test_main();
    if (legacy_result != 0)
        return legacy_result;

    test_r15_projection_failure_precedes_all_startup();
    test_r27_mpeg_profile_failure_precedes_all_startup();
    test_r15_selected_profile_reaches_existing_lifecycle_exactly();

    test_r32_acquisition_occurs_once_after_link_before_first_pstv();
    test_r32_provider_replacement_reuses_resident_snapshot();
    test_r32_acquisition_internal_failure_is_resident_fatal();

    test_r27_session_foundation_is_fresh_unarmed_and_mpeg_capable();
    test_r27_provider_replacement_retires_clock_before_transport();
    test_r27_binding_release_failure_blocks_replacement();
    test_r27_prebinding_failure_never_releases_clock();
    test_r27_binding_init_failure_aborts_transport_without_release();
    test_r27_clock_must_begin_unarmed();

    test_r34_exact_binding_installs_on_each_fresh_input_runtime();
    test_r34_semantic_product_action_routes_without_physical_logic();
    test_r34_live_failure_uses_r33_before_release_and_close();
    test_r34c_prestart_teardown_uses_r34p_before_release_and_close();
    test_r34c_prestart_input_shutdown_failure_blocks_begin_abort();
    test_r34c_clean_no_mpeg_failure_keeps_one_shot_abort();

    test_r16b_provider_connect_failure_restarts_fresh();
    test_r16b_provider_read_closes_admission_before_restart();
    test_r16b_provider_write_failure_restarts_fresh();
    test_r16b_repeated_provider_cycles_rebuild_attempt_state();
    test_r16b_generic_rfb_io_remains_fatal_not_provider_recovery();
    test_r16b_unproven_input_stop_blocks_replacement();
    test_r16b_unproven_transport_stop_blocks_replacement();

    test_r19_thawed_dirty_update_completes_presents_and_reschedules();
    test_r19_failed_send_never_records_policy_send();
    test_r19_idle_preserves_outstanding_without_reschedule();
    test_r19_hold_is_no_send_no_accounting();
    test_r19_full_policy_decision_maps_to_nonincremental_request();
    test_r19_publication_gate_suppresses_dirty_remote_present();
    test_r19_provider_replacement_reinitializes_flow_policy();
    test_r19_policy_accounting_rejection_fails_before_receive();

    if (failures != 0) {
        fprintf(stderr, "app R15/R16B/R19/R27/R32/R34 tests: %d failure(s)\n", failures);
        return 1;
    }

    printf("app R15/R16B/R19/R27/R32/R34 tests: PASS\n");
    return 0;
}

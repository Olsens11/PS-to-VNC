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

static int selected_projection_available;
static size_t selected_projection_calls;
static size_t selected_projection_event_count;
static pstvnc_transport_session_config_t selected_projection;

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

static void reset_selected_projection(void)
{
    selected_projection_available = 0;
    selected_projection_calls = 0u;
    selected_projection_event_count = 0u;
    memset(&selected_projection, 0, sizeof(selected_projection));
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

    /* Stop immediately after the existing configured lifecycle observes the
     * selected value; this keeps the assertion about composition, not UI/RFB
     * session behavior already covered by the legacy fixture. */
    transport_open_result = PSTVNC_TRANSPORT_FAILED;
    transport_open_adopts = 0;

    CHECK(pstvnc_app_run() == -1);
    CHECK(selected_projection_calls == 1u);
    CHECK(selected_projection_event_count == 0u);
    CHECK(event_count > 0u);
    CHECK(events[0] == EV_PREPARE_IOP);
    CHECK(transport_open_calls == 1u);
    CHECK(observed_transport_config.rfb_queue_capacity == 32768u);
    CHECK(observed_transport_config.rfb_initial_credit_bytes == 32768u);
    CHECK(observed_transport_config.rfb_credit_batch_bytes == 8192u);
    CHECK(observed_transport_config.rfb_credit_flush_on_empty == 1);
    CHECK(observed_transport_config.rfb_credit_return_enabled == 1);
    CHECK(observed_transport_config.receiver_thread_stack_size == 16384u);
    CHECK(observed_transport_config.receiver_thread_priority == 63);
    CHECK(observed_transport_config.max_data_payload == 8192u);
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

int main(void)
{
    int legacy_result;

    reset_selected_projection();
    legacy_result = legacy_app_test_main();
    if (legacy_result != 0)
        return legacy_result;

    test_r15_projection_failure_precedes_all_startup();
    test_r15_selected_profile_reaches_existing_lifecycle_exactly();

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
        fprintf(stderr, "app R15/R16B/R19 tests: %d failure(s)\n", failures);
        return 1;
    }

    printf("app R15/R16B/R19 tests: PASS\n");
    return 0;
}

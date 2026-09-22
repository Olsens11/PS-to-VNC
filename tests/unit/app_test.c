/*
 * File synopsis:
 * Runs the established Application host fixture plus R15 default-entry
 * composition assertions. The frozen legacy body remains the broad lifecycle,
 * input and cleanup regression fixture; this wrapper supplies only the new
 * selected-profile seam and R15-specific observations.
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

int main(void)
{
    int legacy_result;

    reset_selected_projection();
    legacy_result = legacy_app_test_main();
    if (legacy_result != 0)
        return legacy_result;

    test_r15_projection_failure_precedes_all_startup();
    test_r15_selected_profile_reaches_existing_lifecycle_exactly();

    if (failures != 0) {
        fprintf(stderr, "app R15 activation tests: %d failure(s)\n", failures);
        return 1;
    }

    printf("app R15 activation tests: PASS\n");
    return 0;
}

/*
 * File synopsis:
 * R16A overlay on the established deterministic Transport runtime host harness.
 * It proves a reader genuinely blocked waiting for provider-supplied RFB bytes
 * and a writer genuinely blocked waiting for provider capacity are both woken
 * by the typed channel-1 provider-terminal fact without converting that fact
 * into physical Wire/Transport failure.
 *
 * The existing runtime fixture remains the owner of fake kernel/thread/physical
 * stream behavior. This overlay only renames its WaitSema stub so the test can
 * observe the exact moment production runtime enters an RFB activity wait.
 *
 * Context: docs/ledge/LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT.md.
 */

#define WaitSema pstvnc_test_legacy_wait_sema
#define main pstvnc_test_legacy_runtime_main
#include "transport_runtime_test.c"
#undef main
#undef WaitSema

static pthread_mutex_t provider_wait_probe_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t provider_wait_probe_condition = PTHREAD_COND_INITIALIZER;
static int provider_wait_probe_semaphore_id = -1;
static int provider_wait_probe_entered;

int WaitSema(int semaphore_id)
{
    pthread_mutex_lock(&provider_wait_probe_mutex);
    if (semaphore_id == provider_wait_probe_semaphore_id) {
        provider_wait_probe_entered = 1;
        pthread_cond_broadcast(&provider_wait_probe_condition);
    }
    pthread_mutex_unlock(&provider_wait_probe_mutex);

    return pstvnc_test_legacy_wait_sema(semaphore_id);
}

static void arm_provider_wait_probe(int semaphore_id)
{
    pthread_mutex_lock(&provider_wait_probe_mutex);
    provider_wait_probe_semaphore_id = semaphore_id;
    provider_wait_probe_entered = 0;
    pthread_mutex_unlock(&provider_wait_probe_mutex);
}

static void wait_for_provider_wait_probe(void)
{
    pthread_mutex_lock(&provider_wait_probe_mutex);
    while (!provider_wait_probe_entered)
        pthread_cond_wait(
            &provider_wait_probe_condition,
            &provider_wait_probe_mutex);
    pthread_mutex_unlock(&provider_wait_probe_mutex);
}

static void disarm_provider_wait_probe(void)
{
    pthread_mutex_lock(&provider_wait_probe_mutex);
    provider_wait_probe_semaphore_id = -1;
    provider_wait_probe_entered = 0;
    pthread_mutex_unlock(&provider_wait_probe_mutex);
}

typedef struct rfb_read_probe {
    pstvnc_transport_runtime_t *runtime;
    int result;
    uint8_t byte;
} rfb_read_probe_t;

static void *run_blocked_rfb_read(void *opaque)
{
    rfb_read_probe_t *probe = (rfb_read_probe_t *)opaque;

    probe->result = pstvnc_transport_runtime_rfb_read_exact(
        probe->runtime,
        &probe->byte,
        1u);
    return NULL;
}

typedef struct rfb_write_probe {
    pstvnc_transport_runtime_t *runtime;
    int result;
    uint8_t byte;
} rfb_write_probe_t;

static void *run_blocked_rfb_write(void *opaque)
{
    rfb_write_probe_t *probe = (rfb_write_probe_t *)opaque;

    probe->result = pstvnc_transport_runtime_rfb_write_exact(
        probe->runtime,
        &probe->byte,
        1u);
    return NULL;
}

static void push_provider_failure(
    pstvnc_rfb_provider_failure_reason_t reason)
{
    uint8_t payload[PSTVNC_RFB_PROVIDER_FAILURE_PAYLOAD_SIZE];
    pstvnc_rfb_provider_failure_payload_t failure;

    failure.reason = (uint32_t)reason;
    CHECK(pstvnc_rfb_provider_failure_payload_encode(payload, &failure));
    push_rx_frame(
        PSTVNC_TRANSPORT_FRAME_ERROR,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload));
}

static void test_blocked_reader_wakes_on_typed_provider_failure(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_rfb_provider_failure_reason_t reason =
        PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    rfb_read_probe_t probe;
    pthread_t reader;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    memset(&probe, 0, sizeof(probe));
    probe.runtime = &runtime;
    probe.result = 99;

    arm_provider_wait_probe(runtime.rfb_activity_semaphore_id);
    CHECK(pthread_create(&reader, NULL, run_blocked_rfb_read, &probe) == 0);
    wait_for_provider_wait_probe();

    /* The reader is now genuinely asleep waiting for provider-side RFB data. */
    push_provider_failure(PSTVNC_RFB_PROVIDER_FAILURE_READ);
    CHECK(pthread_join(reader, NULL) == 0);
    disarm_provider_wait_probe();

    CHECK(probe.result == 0);
    CHECK(pstvnc_transport_runtime_rfb_provider_failure(
        &runtime, &reason) == PSTVNC_TRANSPORT_OK);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_READ);
    CHECK(pstvnc_transport_runtime_rfb_poll_receive(&runtime) == -1);

    /* Correct provider terminality is channel-local: Wire owner remains live. */
    CHECK(runtime.failed == 0);
    CHECK(runtime.receiver_done == 0);
    CHECK(runtime.stop_requested == 0);
    CHECK(runtime.receiver_thread_started == 1);
    CHECK(physical_shutdown_calls == 0);

    stop_and_release_runtime(&runtime);
}

static void test_blocked_writer_wakes_without_reusing_provider_credit(void)
{
    pstvnc_transport_runtime_t runtime;
    pstvnc_transport_session_config_t config = make_config();
    pstvnc_rfb_provider_failure_reason_t reason =
        PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    rfb_write_probe_t probe;
    pthread_t writer;

    reset_fixture();
    initialize_runtime(&runtime, &config);
    start_runtime(&runtime);
    clear_send_records();

    memset(&probe, 0, sizeof(probe));
    probe.runtime = &runtime;
    probe.byte = 0x5au;
    probe.result = 99;

    CHECK(pthread_create(&writer, NULL, run_blocked_rfb_write, &probe) == 0);
    wait_for_rfb_credit_wait_calls(1);

    push_provider_failure(PSTVNC_RFB_PROVIDER_FAILURE_WRITE);
    CHECK(pthread_join(writer, NULL) == 0);

    CHECK(probe.result == 0);
    CHECK(runtime.rfb_outbound_credit_bytes == 0u);
    CHECK(pstvnc_transport_runtime_rfb_provider_failure(
        &runtime, &reason) == PSTVNC_TRANSPORT_OK);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_WRITE);
    CHECK(runtime.failed == 0);
    CHECK(runtime.receiver_done == 0);
    CHECK(runtime.stop_requested == 0);
    CHECK(runtime.receiver_thread_started == 1);
    CHECK(send_record_count == 0u);
    CHECK(physical_shutdown_calls == 0);

    stop_and_release_runtime(&runtime);
}

int main(void)
{
    test_blocked_reader_wakes_on_typed_provider_failure();
    test_blocked_writer_wakes_without_reusing_provider_credit();

    reset_fixture();
    pthread_mutex_destroy(&provider_wait_probe_mutex);
    pthread_cond_destroy(&provider_wait_probe_condition);

    if (failures != 0) {
        fprintf(stderr,
            "transport_rfb_provider_failure_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("transport_rfb_provider_failure_test: PASS");
    return 0;
}

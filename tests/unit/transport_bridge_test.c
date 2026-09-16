/*
 * File synopsis:
 * Host-tests Transport's cross-component bridge ownership and result mapping by
 * replacing the private runtime operations with deterministic stubs.
 *
 * The test proves socket ownership transfer is unambiguous, failed receiver
 * start retires an already-adopted descriptor through Transport, live receiver
 * state prevents resource reclamation, and logical RFB/quiesce operations expose
 * only bridge results rather than a physical socket.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include <stdio.h>
#include <string.h>

#include "transport/bridge.h"
#include "transport/runtime.h"

static int failures;
static int initialize_result = 1;
static int start_result = 1;
static int wait_done_result = 1;
static int release_result = 1;
static int read_result = 1;
static int poll_result = 1;
static int write_result = 1;
static int quiesce_requested_result = 1;
static int quiesce_boundary_result = 1;
static int quiesce_commit_result = 1;
static int residual_snapshot_result = 1;
static int residual_discard_result = 1;
static int quiesce_complete_result = 1;
static int initialize_calls;
static int start_calls;
static int release_calls;
static int adopted_socket_fd;
static pstvnc_transport_runtime_t *observed_runtime;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void reset_fixture(void)
{
    initialize_result = 1;
    start_result = 1;
    wait_done_result = 1;
    release_result = 1;
    read_result = 1;
    poll_result = 1;
    write_result = 1;
    quiesce_requested_result = 1;
    quiesce_boundary_result = 1;
    quiesce_commit_result = 1;
    residual_snapshot_result = 1;
    residual_discard_result = 1;
    quiesce_complete_result = 1;
    initialize_calls = 0;
    start_calls = 0;
    release_calls = 0;
    adopted_socket_fd = -1;
    observed_runtime = NULL;
}

static pstvnc_transport_session_config_t make_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 4096u;
    config.rfb_initial_credit_bytes = 4096u;
    config.rfb_credit_batch_bytes = 1024u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 4096u;
    config.receiver_thread_priority = 64;
    config.max_data_payload = 1024u;
    return config;
}

int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config)
{
    initialize_calls++;
    adopted_socket_fd = socket_fd;

    if (!initialize_result || runtime == NULL || config == NULL)
        return 0;

    memset(runtime, 0, sizeof(*runtime));
    runtime->initialized = 1;
    runtime->physical_stream.socket_fd = socket_fd;
    runtime->physical_stream.send_semaphore_id = 1;
    runtime->receiver_thread_id = -1;
    observed_runtime = runtime;
    return 1;
}

int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime)
{
    start_calls++;

    if (!start_result)
        return 0;

    runtime->receiver_thread_started = 1;
    observed_runtime = runtime;
    return 1;
}

int pstvnc_transport_runtime_wait_receiver_done(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return wait_done_result;
}

int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime)
{
    release_calls++;
    memset(runtime, 0, sizeof(*runtime));
    runtime->physical_stream.socket_fd = -1;
    runtime->physical_stream.send_semaphore_id = -1;
    runtime->receiver_thread_id = -1;
    return release_result;
}

int pstvnc_transport_runtime_rfb_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    (void)runtime;
    (void)buffer;
    (void)count;
    return read_result;
}

int pstvnc_transport_runtime_rfb_poll_receive(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return poll_result;
}

int pstvnc_transport_runtime_rfb_write_exact(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    (void)runtime;
    (void)buffer;
    (void)count;
    return write_result;
}

int pstvnc_transport_runtime_rfb_quiesce_requested(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_requested_result;
}

int pstvnc_transport_runtime_rfb_send_quiesce_boundary(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_boundary_result;
}

int pstvnc_transport_runtime_rfb_wait_quiesce_commit(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_commit_result;
}

int pstvnc_transport_runtime_rfb_snapshot_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t *residual_count)
{
    (void)runtime;
    if (residual_count != NULL)
        *residual_count = 7u;
    return residual_snapshot_result;
}

int pstvnc_transport_runtime_rfb_discard_quiesce_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t expected_count,
    size_t *discarded_count)
{
    (void)runtime;
    if (discarded_count != NULL)
        *discarded_count = expected_count;
    return residual_discard_result;
}

int pstvnc_transport_runtime_rfb_send_quiesce_complete(
    pstvnc_transport_runtime_t *runtime)
{
    (void)runtime;
    return quiesce_complete_result;
}

static void test_socket_ownership_and_close_guard(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 42;

    reset_fixture();

    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(socket_fd == -1);
    CHECK(adopted_socket_fd == 42);
    CHECK(initialize_calls == 1);
    CHECK(start_calls == 1);
    CHECK(release_calls == 0);
    CHECK(observed_runtime != NULL);

    CHECK(pstvnc_transport_session_close() ==
        PSTVNC_TRANSPORT_WOULD_BLOCK);
    CHECK(release_calls == 0);

    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_session_wait_receiver_done() ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
    CHECK(release_calls == 1);
}

static void test_failed_initialize_keeps_caller_socket(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 17;

    reset_fixture();
    initialize_result = 0;

    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_FAILED);
    CHECK(socket_fd == 17);
    CHECK(initialize_calls == 1);
    CHECK(start_calls == 0);
    CHECK(release_calls == 0);
}

static void test_failed_start_consumes_adopted_socket(void)
{
    pstvnc_transport_session_config_t config = make_config();
    int socket_fd = 23;

    reset_fixture();
    start_result = 0;

    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_FAILED);
    CHECK(socket_fd == -1);
    CHECK(adopted_socket_fd == 23);
    CHECK(initialize_calls == 1);
    CHECK(start_calls == 1);
    CHECK(release_calls == 1);
}

static void test_logical_rfb_and_quiesce_result_mapping(void)
{
    pstvnc_transport_session_config_t config = make_config();
    unsigned char byte = 0;
    size_t residual = 0u;
    size_t discarded = 0u;
    int socket_fd = 31;

    reset_fixture();

    CHECK(pstvnc_transport_session_open(&socket_fd, &config) ==
        PSTVNC_TRANSPORT_OK);

    CHECK(pstvnc_transport_rfb_read_exact(&byte, 1u) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_write_exact(&byte, 1u) ==
        PSTVNC_TRANSPORT_OK);

    poll_result = 0;
    CHECK(pstvnc_transport_rfb_poll_receive() ==
        PSTVNC_TRANSPORT_WOULD_BLOCK);
    poll_result = 1;
    CHECK(pstvnc_transport_rfb_poll_receive() == PSTVNC_TRANSPORT_OK);

    quiesce_requested_result = 0;
    CHECK(pstvnc_transport_rfb_quiesce_requested() ==
        PSTVNC_TRANSPORT_WOULD_BLOCK);
    quiesce_requested_result = 1;
    CHECK(pstvnc_transport_rfb_quiesce_requested() == PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_send_quiesce_boundary() ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_wait_quiesce_commit() ==
        PSTVNC_TRANSPORT_OK);
    CHECK(pstvnc_transport_rfb_snapshot_quiesce_residual(&residual) ==
        PSTVNC_TRANSPORT_OK);
    CHECK(residual == 7u);
    CHECK(pstvnc_transport_rfb_discard_quiesce_residual(
        residual, &discarded) == PSTVNC_TRANSPORT_OK);
    CHECK(discarded == residual);
    CHECK(pstvnc_transport_rfb_send_quiesce_complete() ==
        PSTVNC_TRANSPORT_OK);

    read_result = 0;
    observed_runtime->failed = 1;
    CHECK(pstvnc_transport_rfb_read_exact(&byte, 1u) ==
        PSTVNC_TRANSPORT_FAILED);

    observed_runtime->failed = 0;
    observed_runtime->receiver_done = 1;
    CHECK(pstvnc_transport_rfb_read_exact(&byte, 1u) ==
        PSTVNC_TRANSPORT_CLOSED);

    CHECK(pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK);
}

int main(void)
{
    test_socket_ownership_and_close_guard();
    test_failed_initialize_keeps_caller_socket();
    test_failed_start_consumes_adopted_socket();
    test_logical_rfb_and_quiesce_result_mapping();

    if (failures != 0) {
        fprintf(stderr, "%d transport bridge test(s) failed\n", failures);
        return 1;
    }

    puts("transport bridge tests passed");
    return 0;
}

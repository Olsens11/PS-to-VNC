/*
 * File synopsis:
 * Host-tests RFB's cross-component byte-stream bridge by replacing Transport's
 * public bridge operations with deterministic stubs. The test verifies exact
 * I/O result mapping, typed RFB-provider failure mapping, and the finite-session
 * quiesce ordering owned by this seam.
 *
 * This fixture does not exercise physical sockets, Transport lifecycle, RFB
 * parsing, or choose the complete-message safe boundary.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include <stdio.h>
#include <string.h>

#include "rfb/bridge.h"
#include "transport/bridge.h"

static int failures;
static pstvnc_transport_access_t test_access;
static pstvnc_transport_result_t read_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t poll_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t write_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t provider_failure_result =
    PSTVNC_TRANSPORT_WOULD_BLOCK;
static pstvnc_rfb_provider_failure_reason_t provider_failure_reason =
    PSTVNC_RFB_PROVIDER_FAILURE_NONE;
static pstvnc_transport_result_t request_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
static pstvnc_transport_result_t boundary_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t commit_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t snapshot_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t discard_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t complete_result = PSTVNC_TRANSPORT_OK;
static void *observed_read_buffer;
static const void *observed_write_buffer;
static size_t observed_read_count;
static size_t observed_write_count;
static size_t residual_count = 7u;
static size_t observed_discard_expected;
static unsigned int quiesce_step;
static unsigned int boundary_step;
static unsigned int commit_step;
static unsigned int snapshot_step;
static unsigned int discard_step;
static unsigned int complete_step;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    transport_access->opaque_ticket = 1u;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    CHECK(transport_access == &test_access);
    observed_read_buffer = buffer;
    observed_read_count = count;
    return read_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    CHECK(transport_access == &test_access);
    return poll_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    CHECK(transport_access == &test_access);
    observed_write_buffer = buffer;
    observed_write_count = count;
    return write_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_provider_failure(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_rfb_provider_failure_reason_t *reason)
{
    CHECK(transport_access == &test_access);
    if (reason == NULL)
        return PSTVNC_TRANSPORT_INVALID;
    *reason = provider_failure_reason;
    return provider_failure_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    CHECK(transport_access == &test_access);
    return request_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    CHECK(transport_access == &test_access);
    boundary_step = ++quiesce_step;
    return boundary_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(
    const pstvnc_transport_access_t *transport_access)
{
    CHECK(transport_access == &test_access);
    commit_step = ++quiesce_step;
    return commit_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t *count)
{
    CHECK(transport_access == &test_access);
    snapshot_step = ++quiesce_step;
    if (snapshot_result == PSTVNC_TRANSPORT_OK)
        *count = residual_count;
    return snapshot_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t expected_count,
    size_t *discarded_count)
{
    CHECK(transport_access == &test_access);
    discard_step = ++quiesce_step;
    observed_discard_expected = expected_count;
    if (discard_result == PSTVNC_TRANSPORT_OK)
        *discarded_count = expected_count;
    return discard_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(
    const pstvnc_transport_access_t *transport_access)
{
    CHECK(transport_access == &test_access);
    complete_step = ++quiesce_step;
    return complete_result;
}

static void test_exact_io_mapping(void)
{
    unsigned char bytes[3] = {0u, 1u, 2u};

    memset(&test_access, 0, sizeof(test_access));
    CHECK(pstvnc_rfb_bridge_acquire(&test_access) == 0);

    read_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_read_exact(&test_access, bytes, sizeof(bytes)) == 0);
    CHECK(observed_read_buffer == bytes);
    CHECK(observed_read_count == sizeof(bytes));
    read_result = PSTVNC_TRANSPORT_CLOSED;
    CHECK(pstvnc_rfb_bridge_read_exact(&test_access, bytes, sizeof(bytes)) == -1);

    write_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_write_exact(&test_access, bytes, sizeof(bytes)) == 0);
    CHECK(observed_write_buffer == bytes);
    CHECK(observed_write_count == sizeof(bytes));
    write_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_write_exact(&test_access, bytes, sizeof(bytes)) == -1);
}

static void test_provider_failure_mapping(void)
{
    pstvnc_rfb_provider_failure_reason_t reason =
        PSTVNC_RFB_PROVIDER_FAILURE_NONE;

    provider_failure_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    CHECK(pstvnc_rfb_bridge_provider_failure(&test_access, &reason) == 0);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_NONE);

    provider_failure_result = PSTVNC_TRANSPORT_OK;
    provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_CONNECT;
    CHECK(pstvnc_rfb_bridge_provider_failure(&test_access, &reason) == 1);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_CONNECT);

    provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_READ;
    CHECK(pstvnc_rfb_bridge_provider_failure(&test_access, &reason) == 1);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_READ);

    provider_failure_reason = PSTVNC_RFB_PROVIDER_FAILURE_WRITE;
    CHECK(pstvnc_rfb_bridge_provider_failure(&test_access, &reason) == 1);
    CHECK(reason == PSTVNC_RFB_PROVIDER_FAILURE_WRITE);

    provider_failure_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_provider_failure(&test_access, &reason) == -1);
}

static void test_readiness_mapping(void)
{
    poll_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_poll_receive(&test_access) == 1);
    poll_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    CHECK(pstvnc_rfb_bridge_poll_receive(&test_access) == 0);
    poll_result = PSTVNC_TRANSPORT_CLOSED;
    CHECK(pstvnc_rfb_bridge_poll_receive(&test_access) == -1);
}

static void reset_quiesce_results(void)
{
    request_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    boundary_result = PSTVNC_TRANSPORT_OK;
    commit_result = PSTVNC_TRANSPORT_OK;
    snapshot_result = PSTVNC_TRANSPORT_OK;
    discard_result = PSTVNC_TRANSPORT_OK;
    complete_result = PSTVNC_TRANSPORT_OK;
    quiesce_step = 0;
    boundary_step = 0;
    commit_step = 0;
    snapshot_step = 0;
    discard_step = 0;
    complete_step = 0;
    observed_discard_expected = 0;
}

static void test_quiesce_mapping_and_order(void)
{
    reset_quiesce_results();
    CHECK(pstvnc_rfb_bridge_quiesce_requested(&test_access) == 0);
    request_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_quiesce_requested(&test_access) == 1);
    request_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_quiesce_requested(&test_access) == -1);

    reset_quiesce_results();
    CHECK(pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(&test_access) == 0);
    CHECK(boundary_step == 1u);
    CHECK(commit_step == 2u);
    CHECK(snapshot_step == 3u);
    CHECK(discard_step == 4u);
    CHECK(complete_step == 5u);
    CHECK(observed_discard_expected == residual_count);

    reset_quiesce_results();
    commit_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(&test_access) == -1);
    CHECK(boundary_step == 1u);
    CHECK(commit_step == 2u);
    CHECK(snapshot_step == 0u);
    CHECK(discard_step == 0u);
    CHECK(complete_step == 0u);
}

int main(void)
{
    test_exact_io_mapping();
    test_provider_failure_mapping();
    test_readiness_mapping();
    test_quiesce_mapping_and_order();

    if (failures != 0) {
        fprintf(stderr, "%d RFB bridge test(s) failed\n", failures);
        return 1;
    }

    puts("RFB bridge tests passed");
    return 0;
}
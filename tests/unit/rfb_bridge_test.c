/*
 * File synopsis:
 * Host-tests RFB's cross-component byte-stream bridge by replacing Transport's
 * public bridge operations with deterministic stubs. The test verifies exact
 * I/O result mapping and the finite-session quiesce ordering owned by this seam.
 *
 * This fixture does not exercise physical sockets, Transport lifecycle, RFB
 * parsing, or choose the complete-message safe boundary.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include <stdio.h>

#include "rfb/bridge.h"
#include "transport/bridge.h"

static int failures;
static pstvnc_transport_result_t read_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t poll_result = PSTVNC_TRANSPORT_OK;
static pstvnc_transport_result_t write_result = PSTVNC_TRANSPORT_OK;
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

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(void *buffer, size_t count)
{
    observed_read_buffer = buffer;
    observed_read_count = count;
    return read_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    return poll_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(const void *buffer, size_t count)
{
    observed_write_buffer = buffer;
    observed_write_count = count;
    return write_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void)
{
    return request_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void)
{
    boundary_step = ++quiesce_step;
    return boundary_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void)
{
    commit_step = ++quiesce_step;
    return commit_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(size_t *count)
{
    snapshot_step = ++quiesce_step;
    if (snapshot_result == PSTVNC_TRANSPORT_OK)
        *count = residual_count;
    return snapshot_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count)
{
    discard_step = ++quiesce_step;
    observed_discard_expected = expected_count;
    if (discard_result == PSTVNC_TRANSPORT_OK)
        *discarded_count = expected_count;
    return discard_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void)
{
    complete_step = ++quiesce_step;
    return complete_result;
}

static void test_exact_io_mapping(void)
{
    unsigned char bytes[3] = {0u, 1u, 2u};

    read_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_read_exact(bytes, sizeof(bytes)) == 0);
    CHECK(observed_read_buffer == bytes);
    CHECK(observed_read_count == sizeof(bytes));
    read_result = PSTVNC_TRANSPORT_CLOSED;
    CHECK(pstvnc_rfb_bridge_read_exact(bytes, sizeof(bytes)) == -1);

    write_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_write_exact(bytes, sizeof(bytes)) == 0);
    CHECK(observed_write_buffer == bytes);
    CHECK(observed_write_count == sizeof(bytes));
    write_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_write_exact(bytes, sizeof(bytes)) == -1);
}

static void test_readiness_mapping(void)
{
    poll_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_poll_receive() == 1);
    poll_result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    CHECK(pstvnc_rfb_bridge_poll_receive() == 0);
    poll_result = PSTVNC_TRANSPORT_CLOSED;
    CHECK(pstvnc_rfb_bridge_poll_receive() == -1);
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
    CHECK(pstvnc_rfb_bridge_quiesce_requested() == 0);
    request_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_quiesce_requested() == 1);
    request_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_quiesce_requested() == -1);

    reset_quiesce_results();
    CHECK(pstvnc_rfb_bridge_complete_quiesce_at_message_boundary() == 0);
    CHECK(boundary_step == 1u);
    CHECK(commit_step == 2u);
    CHECK(snapshot_step == 3u);
    CHECK(discard_step == 4u);
    CHECK(complete_step == 5u);
    CHECK(observed_discard_expected == residual_count);

    reset_quiesce_results();
    commit_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_complete_quiesce_at_message_boundary() == -1);
    CHECK(boundary_step == 1u);
    CHECK(commit_step == 2u);
    CHECK(snapshot_step == 0u);
    CHECK(discard_step == 0u);
    CHECK(complete_step == 0u);
}

int main(void)
{
    test_exact_io_mapping();
    test_readiness_mapping();
    test_quiesce_mapping_and_order();

    if (failures != 0) {
        fprintf(stderr, "%d RFB bridge test(s) failed\n", failures);
        return 1;
    }

    puts("RFB bridge tests passed");
    return 0;
}

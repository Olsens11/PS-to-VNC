/*
 * File synopsis:
 * Host-tests RFB's cross-component byte-stream bridge by replacing Transport's
 * public bridge operations with deterministic stubs. The test verifies that RFB
 * exposes only its exact-read/readiness/exact-write vocabulary while preserving
 * Transport success, idle, and terminal-failure distinctions at the seam.
 *
 * This fixture does not exercise physical sockets, Transport lifecycle, RFB
 * parsing, or complete-message safe-boundary policy.
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
static void *observed_read_buffer;
static const void *observed_write_buffer;
static size_t observed_read_count;
static size_t observed_write_count;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count)
{
    observed_read_buffer = buffer;
    observed_read_count = count;
    return read_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    return poll_result;
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count)
{
    observed_write_buffer = buffer;
    observed_write_count = count;
    return write_result;
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
    read_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_read_exact(bytes, sizeof(bytes)) == -1);

    write_result = PSTVNC_TRANSPORT_OK;
    CHECK(pstvnc_rfb_bridge_write_exact(bytes, sizeof(bytes)) == 0);
    CHECK(observed_write_buffer == bytes);
    CHECK(observed_write_count == sizeof(bytes));

    write_result = PSTVNC_TRANSPORT_CLOSED;
    CHECK(pstvnc_rfb_bridge_write_exact(bytes, sizeof(bytes)) == -1);
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

    poll_result = PSTVNC_TRANSPORT_FAILED;
    CHECK(pstvnc_rfb_bridge_poll_receive() == -1);
}

int main(void)
{
    test_exact_io_mapping();
    test_readiness_mapping();

    if (failures != 0) {
        fprintf(stderr, "%d RFB bridge test(s) failed\n", failures);
        return 1;
    }

    puts("RFB bridge tests passed");
    return 0;
}

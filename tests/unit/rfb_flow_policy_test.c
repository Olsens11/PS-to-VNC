/*
 * File synopsis:
 * Host contract tests for the clean generic RFB freeze/request/full-refresh
 * policy, including integration with the real current RFB session parser at its
 * complete-server-message boundary.
 */

#include "flow_policy.h"
#include "rfb_session.h"
#include "transport/protocol.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned char input_bytes[4096];
static size_t input_size;
static size_t input_pos;

static unsigned char output_bytes[4096];
static size_t output_size;

static void script_reset(void)
{
    memset(input_bytes, 0, sizeof(input_bytes));
    memset(output_bytes, 0, sizeof(output_bytes));
    input_size = 0u;
    input_pos = 0u;
    output_size = 0u;
}

static void append_input(
    const void *bytes,
    size_t count)
{
    assert(input_size + count <= sizeof(input_bytes));
    memcpy(&input_bytes[input_size], bytes, count);
    input_size += count;
}

static void append_update_header(uint16_t rectangle_count)
{
    unsigned char update[4] = {
        0u,
        0u,
        (unsigned char)(rectangle_count >> 8),
        (unsigned char)rectangle_count
    };

    append_input(update, sizeof(update));
}

static void append_raw_rectangle(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    const uint16_t *pixels)
{
    unsigned char header[12];
    uint16_t row;
    uint16_t column;

    memset(header, 0, sizeof(header));
    header[0] = (unsigned char)(x >> 8);
    header[1] = (unsigned char)x;
    header[2] = (unsigned char)(y >> 8);
    header[3] = (unsigned char)y;
    header[4] = (unsigned char)(width >> 8);
    header[5] = (unsigned char)width;
    header[6] = (unsigned char)(height >> 8);
    header[7] = (unsigned char)height;

    append_input(header, sizeof(header));

    for (row = 0u; row < height; ++row) {
        for (column = 0u; column < width; ++column) {
            uint16_t value =
                pixels[(size_t)row * width + column];
            unsigned char wire[2] = {
                (unsigned char)value,
                (unsigned char)(value >> 8)
            };

            append_input(wire, sizeof(wire));
        }
    }
}

int pstvnc_rfb_bridge_acquire(
    pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL)
        return -1;

    transport_access->opaque_ticket = 1u;
    return 0;
}

int pstvnc_rfb_bridge_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    (void)transport_access;

    if (buffer == NULL || input_pos + count > input_size)
        return -1;

    memcpy(buffer, &input_bytes[input_pos], count);
    input_pos += count;
    return 0;
}

int pstvnc_rfb_bridge_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    return input_pos < input_size ? 1 : 0;
}

int pstvnc_rfb_bridge_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    (void)transport_access;

    if (buffer == NULL ||
        output_size + count > sizeof(output_bytes))
        return -1;

    memcpy(&output_bytes[output_size], buffer, count);
    output_size += count;
    return 0;
}

int pstvnc_rfb_bridge_provider_failure(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_rfb_provider_failure_reason_t *reason)
{
    (void)transport_access;
    if (reason == NULL)
        return -1;
    *reason = PSTVNC_RFB_PROVIDER_FAILURE_NONE;
    return 0;
}

int pstvnc_rfb_bridge_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    return 0;
}

int pstvnc_rfb_bridge_complete_quiesce_at_message_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    (void)transport_access;
    return 0;
}

static void prepare_live(
    pstvnc_rfb_session_t *session,
    pstvnc_framebuffer_t *framebuffer,
    uint16_t *pixels)
{
    pstvnc_rfb_session_init(session);
    session->transport_access.opaque_ticket = 1u;
    session->state = PSTVNC_RFB_SESSION_READY;
    session->server_init.width = 4u;
    session->server_init.height = 3u;

    assert(pstvnc_framebuffer_init(framebuffer, pixels, 12u));
    assert(pstvnc_framebuffer_set_geometry(framebuffer, 4u, 3u));
    assert(pstvnc_framebuffer_mark_valid(framebuffer));
}

static void test_ordinary_one_outstanding_request(void)
{
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_rfb_flow_request_t request;

    pstvnc_rfb_flow_policy_init(&policy);

    assert(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    assert(!pstvnc_rfb_flow_policy_has_outstanding_request(&policy));

    request = pstvnc_rfb_flow_policy_next_request(&policy);
    assert(request == PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
    assert(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        request));

    assert(pstvnc_rfb_flow_policy_has_outstanding_request(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(!pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL));

    assert(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    assert(!pstvnc_rfb_flow_policy_record_update_complete(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
}

static void test_full_obligation_is_nonmutating_until_send(void)
{
    pstvnc_rfb_flow_policy_t policy;

    pstvnc_rfb_flow_policy_init(&policy);

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(!pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    assert(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    assert(!pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    assert(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);
}

static void test_coalescing_and_distinct_later_interval(void)
{
    pstvnc_rfb_flow_policy_t policy;

    pstvnc_rfb_flow_policy_init(&policy);

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    assert(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);
}

static void test_invalid_inputs_fail_closed(void)
{
    pstvnc_rfb_flow_policy_t policy;

    pstvnc_rfb_flow_policy_init(&policy);

    assert(!pstvnc_rfb_flow_policy_set_frozen(&policy, -1));
    assert(!pstvnc_rfb_flow_policy_set_frozen(&policy, 2));
    assert(pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(NULL) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
    assert(!pstvnc_rfb_flow_policy_allows_remote_publication(NULL));
    assert(!pstvnc_rfb_flow_policy_record_request_sent(
        NULL,
        PSTVNC_RFB_FLOW_REQUEST_FULL));
    assert(!pstvnc_rfb_flow_policy_record_update_complete(NULL));
}

static void test_inflight_freeze_completes_real_rfb_message(void)
{
    static const uint16_t changed[2] = {
        0x1234u,
        0x5678u
    };
    uint16_t pixels[12] = { 0u };
    pstvnc_rfb_session_t session;
    pstvnc_framebuffer_t framebuffer;
    pstvnc_rfb_flow_policy_t policy;
    pstvnc_rfb_flow_request_t request;
    size_t first_request_offset;

    script_reset();
    prepare_live(&session, &framebuffer, pixels);
    pstvnc_rfb_flow_policy_init(&policy);

    request = pstvnc_rfb_flow_policy_next_request(&policy);
    assert(request == PSTVNC_RFB_FLOW_REQUEST_INCREMENTAL);

    first_request_offset = output_size;
    assert(pstvnc_rfb_session_request_update(&session, 1));
    assert(output_size - first_request_offset ==
        PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE);
    assert(output_bytes[first_request_offset] == 3u);
    assert(output_bytes[first_request_offset + 1u] == 1u);

    assert(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        request));

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 1));
    assert(!pstvnc_rfb_flow_policy_allows_remote_publication(&policy));

    append_update_header(1u);
    append_raw_rectangle(1u, 1u, 2u, 1u, changed);

    assert(pstvnc_rfb_session_try_receive_update(
        &session,
        &framebuffer) ==
        PSTVNC_RFB_SESSION_RECEIVE_UPDATE);

    assert(input_pos == input_size);
    assert(pixels[5] == changed[0]);
    assert(pixels[6] == changed[1]);
    assert(framebuffer.dirty);
    assert(pstvnc_rfb_flow_policy_record_update_complete(&policy));
    assert(!pstvnc_rfb_flow_policy_has_outstanding_request(&policy));
    assert(!pstvnc_rfb_flow_policy_allows_remote_publication(&policy));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);

    assert(pstvnc_rfb_flow_policy_set_frozen(&policy, 0));
    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_FULL);

    first_request_offset = output_size;
    assert(pstvnc_rfb_session_request_update(&session, 0));
    assert(output_size - first_request_offset ==
        PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE);
    assert(output_bytes[first_request_offset] == 3u);
    assert(output_bytes[first_request_offset + 1u] == 0u);

    assert(pstvnc_rfb_flow_policy_record_request_sent(
        &policy,
        PSTVNC_RFB_FLOW_REQUEST_FULL));

    assert(pstvnc_rfb_flow_policy_next_request(&policy) ==
        PSTVNC_RFB_FLOW_REQUEST_HOLD);
}

int main(void)
{
    test_ordinary_one_outstanding_request();
    test_full_obligation_is_nonmutating_until_send();
    test_coalescing_and_distinct_later_interval();
    test_invalid_inputs_fail_closed();
    test_inflight_freeze_completes_real_rfb_message();

    puts("RFB_FLOW_POLICY_TEST=PASS");
    return 0;
}

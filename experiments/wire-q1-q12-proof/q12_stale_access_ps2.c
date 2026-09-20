/*
 * File synopsis:
 * Hardware proof for Q12 Transport-owned stale-session fencing.
 *
 * The same PS2 process establishes Wire Session A, opens the real product
 * Transport bridge, acquires opaque access A, and proves A can carry ordinary
 * RFB work.  After the Pi kills A, the product bridge is fully retired and the
 * same process establishes Session B and acquires opaque access B.
 *
 * While B is active, the proof deliberately reuses stale access A on both the
 * outbound and inbound RFB paths.  Both calls must terminate as CLOSED before
 * they can touch B.  Fresh access B must then observe the still-present B
 * payload and send the only accepted B acknowledgement.
 *
 * Wire establishment remains proof-local because Q4 establishment is not yet
 * productized.  Session identity is proof apparatus only; rider-facing Q12
 * behavior uses opaque Transport access exclusively.
 */

#include <debug.h>
#include <kernel.h>

#include <sys/socket.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "transport/bridge.h"
#include "transport/protocol.h"
#include "transport/transport.h"

#include "wire_establish_protocol.h"

#define Q12_MAX_POLL_ATTEMPTS 5000
#define Q12_POLL_DELAY_US     1000u

static const uint8_t q12_a_marker[4] = { 'A', 'O', 'K', '!' };
static const uint8_t q12_stale_marker[4] = { 'O', 'L', 'D', '!' };
static const uint8_t q12_b_marker[4] = { 'B', 'O', 'K', '!' };
static const uint8_t q12_b_payload[4] = { 0xb1u, 0xb2u, 0xb3u, 0xb4u };

static void q12_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int q12_send_exact(int socket_fd, const void *buffer, size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t sent = 0u;

    while (sent < count) {
        int result = send(socket_fd, bytes + sent, count - sent, 0);

        if (result <= 0)
            return 0;

        sent += (size_t)result;
    }

    return 1;
}

static int q12_receive_exact(int socket_fd, void *buffer, size_t count)
{
    uint8_t *bytes = (uint8_t *)buffer;
    size_t received = 0u;

    while (received < count) {
        int result = recv(socket_fd, bytes + received, count - received, 0);

        if (result <= 0)
            return 0;

        received += (size_t)result;
    }

    return 1;
}

static int q12_establish_session(
    int *socket_fd,
    uint32_t *session_id)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    uint8_t hello_payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t result_payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_transport_header_t header;
    pstvnc_wire_proof_hello_t hello;
    pstvnc_wire_proof_accept_t acceptance;

    if (socket_fd == NULL || session_id == NULL)
        return 0;

    *session_id = 0u;
    *socket_fd = pstvnc_ps2_network_connect_pstv();
    if (*socket_fd < 0)
        return 0;

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(hello_payload, &hello))
        goto fail;

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_HELLO;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    header.sequence = 1u;
    header.payload_length = sizeof(hello_payload);

    if (!pstvnc_transport_header_encode(wire_header, &header) ||
        !q12_send_exact(*socket_fd, wire_header, sizeof(wire_header)) ||
        !q12_send_exact(*socket_fd, hello_payload, sizeof(hello_payload)))
        goto fail;

    if (!q12_receive_exact(*socket_fd, wire_header, sizeof(wire_header)) ||
        !pstvnc_transport_header_decode(&header, wire_header) ||
        header.kind != PSTVNC_WIRE_PROOF_FRAME_ACCEPT ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.sequence != 1u ||
        header.payload_length != sizeof(result_payload) ||
        !q12_receive_exact(*socket_fd, result_payload, sizeof(result_payload)) ||
        !pstvnc_wire_proof_accept_decode(
            &acceptance,
            result_payload,
            sizeof(result_payload)) ||
        acceptance.session_id == 0u)
        goto fail;

    *session_id = acceptance.session_id;
    return 1;

fail:
    pstvnc_ps2_network_close(*socket_fd);
    *socket_fd = -1;
    return 0;
}

static pstvnc_transport_session_config_t q12_make_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 16u;
    config.rfb_initial_credit_bytes = 16u;
    config.rfb_credit_batch_bytes = 4u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 16384u;
    config.receiver_thread_priority = 63;
    config.max_data_payload = 16u;
    return config;
}

static int q12_wait_for_rfb(
    const pstvnc_transport_access_t *transport_access)
{
    int attempt;

    for (attempt = 0; attempt < Q12_MAX_POLL_ATTEMPTS; attempt++) {
        pstvnc_transport_result_t result =
            pstvnc_transport_rfb_poll_receive(transport_access);

        if (result == PSTVNC_TRANSPORT_OK)
            return 1;

        if (result != PSTVNC_TRANSPORT_WOULD_BLOCK)
            return 0;

        if (pstvnc_ps2_system_delay_us(Q12_POLL_DELAY_US) < 0)
            return 0;
    }

    return 0;
}

static int q12_retire_closed_session(void)
{
    if (pstvnc_transport_session_wait_receiver_done() != PSTVNC_TRANSPORT_OK)
        return 0;

    return pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK;
}

int main(int argc, char **argv)
{
    pstvnc_transport_session_config_t config = q12_make_config();
    pstvnc_transport_access_t access_a;
    pstvnc_transport_access_t access_b;
    pstvnc_transport_result_t result;
    uint8_t received_b[sizeof(q12_b_payload)];
    uint32_t session_a = 0u;
    uint32_t session_b = 0u;
    int socket_fd = -1;
    char line[128];

    (void)argc;
    (void)argv;

    memset(&access_a, 0, sizeof(access_a));
    memset(&access_b, 0, sizeof(access_b));
    memset(received_b, 0, sizeof(received_b));

    init_scr();

    q12_print("WIRE Q12 STALE-ACCESS HARDWARE PROOF");
    q12_print("state=BOOT");
    q12_print("Q4_ESTABLISHMENT=PROOF_LOCAL");
    q12_print("Q12_TRANSPORT_BRIDGE=PRODUCT_SOURCE");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        q12_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        q12_print("FAIL network");
        SleepThread();
        return 11;
    }

    q12_print("state=SESSION_A_ESTABLISH");

    if (!q12_establish_session(&socket_fd, &session_a)) {
        q12_print("FAIL session_a_establish");
        SleepThread();
        return 12;
    }

    if (pstvnc_transport_session_open(&socket_fd, &config) !=
        PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL session_a_transport_open");
        SleepThread();
        return 13;
    }

    if (pstvnc_transport_access_acquire(&access_a) != PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL access_a_acquire");
        SleepThread();
        return 14;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_A ACTIVE proof_id=%u",
        (unsigned int)session_a);
    q12_print(line);

    if (pstvnc_transport_rfb_write_exact(
            &access_a,
            q12_a_marker,
            sizeof(q12_a_marker)) != PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL access_a_valid_write");
        SleepThread();
        return 15;
    }

    q12_print("ACCESS_A_VALID_WRITE=PASS");
    q12_print("state=WAIT_SESSION_A_FORCED_LOSS");

    if (!q12_retire_closed_session()) {
        q12_print("FAIL session_a_retire");
        SleepThread();
        return 16;
    }

    q12_print("SESSION_A_RETIRED=YES");
    q12_print("state=SESSION_B_ESTABLISH");

    if (!q12_establish_session(&socket_fd, &session_b)) {
        q12_print("FAIL session_b_establish");
        SleepThread();
        return 17;
    }

    if (session_b == session_a) {
        q12_print("FAIL proof_session_id_reused");
        pstvnc_ps2_network_close(socket_fd);
        SleepThread();
        return 18;
    }

    if (pstvnc_transport_session_open(&socket_fd, &config) !=
        PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL session_b_transport_open");
        SleepThread();
        return 19;
    }

    if (pstvnc_transport_access_acquire(&access_b) != PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL access_b_acquire");
        SleepThread();
        return 20;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_B ACTIVE proof_id=%u",
        (unsigned int)session_b);
    q12_print(line);

    /*
     * Outbound discriminator: stale A must die at Transport.  If this call
     * reaches B, the Pi sees OLD! before the valid B acknowledgement and fails.
     */
    result = pstvnc_transport_rfb_write_exact(
        &access_a,
        q12_stale_marker,
        sizeof(q12_stale_marker));

    if (result != PSTVNC_TRANSPORT_CLOSED) {
        q12_print("FAIL stale_a_write_not_closed");
        SleepThread();
        return 21;
    }

    q12_print("STALE_A_OUTBOUND_FENCED=YES");

    if (!q12_wait_for_rfb(&access_b)) {
        q12_print("FAIL b_payload_not_ready");
        SleepThread();
        return 22;
    }

    /*
     * Return-path discriminator: B data is known present before stale A reads.
     * Stale A must return CLOSED without consuming any byte from B.
     */
    result = pstvnc_transport_rfb_read_exact(
        &access_a,
        received_b,
        sizeof(received_b));

    if (result != PSTVNC_TRANSPORT_CLOSED) {
        q12_print("FAIL stale_a_read_not_closed");
        SleepThread();
        return 23;
    }

    if (!q12_wait_for_rfb(&access_b)) {
        q12_print("FAIL stale_a_consumed_b_payload");
        SleepThread();
        return 24;
    }

    q12_print("STALE_A_RETURN_PATH_FENCED=YES");

    if (pstvnc_transport_rfb_read_exact(
            &access_b,
            received_b,
            sizeof(received_b)) != PSTVNC_TRANSPORT_OK ||
        memcmp(received_b, q12_b_payload, sizeof(received_b)) != 0) {
        q12_print("FAIL fresh_b_read");
        SleepThread();
        return 25;
    }

    q12_print("FRESH_B_READ=PASS");

    if (pstvnc_transport_rfb_write_exact(
            &access_b,
            q12_b_marker,
            sizeof(q12_b_marker)) != PSTVNC_TRANSPORT_OK) {
        q12_print("FAIL fresh_b_write");
        SleepThread();
        return 26;
    }

    q12_print("FRESH_B_WRITE=PASS");
    q12_print("state=WAIT_SESSION_B_CLOSE");

    if (!q12_retire_closed_session()) {
        q12_print("FAIL session_b_retire");
        SleepThread();
        return 27;
    }

    q12_print("SESSION_B_RETIRED=YES");
    q12_print("Q12_STALE_A_CANNOT_TOUCH_B=PASS");
    q12_print("Q12 HARDWARE PROOF COMPLETE");

    SleepThread();
    return 0;
}

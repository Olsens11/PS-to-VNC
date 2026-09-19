/*
 * File synopsis:
 * Implements the PS2 half of Q1-Q12 Proof 1A.
 *
 * The PS2 establishes the qualified private Ethernet link, initiates TCP to the
 * dedicated Wire port, and sends the first application bytes as a Wire-framed
 * minimal HELLO containing only Wire protocol version and product version.
 *
 * On ACCEPT it records the Pi-assigned non-zero session ID, sends no rider or
 * configuration traffic, leaves the Wire Session idle for ten seconds, closes
 * it, and parks with the physical result visible.
 *
 * This proof deliberately does not start RFB, audio, MPEG, configuration,
 * desktop, or the current rider-oriented clean Transport runtime.
 */

#include <debug.h>
#include <kernel.h>

#include <sys/socket.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "transport/protocol.h"

#include "wire_establish_protocol.h"

#define WIRE_PROOF_IDLE_US 10000000u

static void proof_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int send_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t sent = 0u;

    while (sent < count) {
        int result = send(
            socket_fd,
            bytes + sent,
            count - sent,
            0);

        if (result <= 0)
            return 0;

        sent += (size_t)result;
    }

    return 1;
}

static int receive_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    uint8_t *bytes = (uint8_t *)buffer;
    size_t received = 0u;

    while (received < count) {
        int result = recv(
            socket_fd,
            bytes + received,
            count - received,
            0);

        if (result <= 0)
            return 0;

        received += (size_t)result;
    }

    return 1;
}

static int send_establishment_hello(int socket_fd)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    uint8_t payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    pstvnc_transport_header_t header;
    pstvnc_wire_proof_hello_t hello;

    memset(&header, 0, sizeof(header));

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(payload, &hello))
        return 0;

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_HELLO;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    header.flags = 0u;
    header.sequence = 1u;
    header.payload_length = sizeof(payload);

    if (!pstvnc_transport_header_encode(wire_header, &header))
        return 0;

    return
        send_exact(socket_fd, wire_header, sizeof(wire_header)) &&
        send_exact(socket_fd, payload, sizeof(payload));
}

static int receive_establishment_result(
    int socket_fd,
    uint32_t *session_id,
    uint32_t *rejection_reason)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    uint8_t payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_transport_header_t header;

    if (session_id == NULL || rejection_reason == NULL)
        return 0;

    *session_id = 0u;
    *rejection_reason = 0u;

    if (!receive_exact(socket_fd, wire_header, sizeof(wire_header)))
        return 0;

    if (!pstvnc_transport_header_decode(&header, wire_header))
        return 0;

    if (header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.sequence != 1u ||
        header.payload_length != sizeof(payload))
        return 0;

    if (!receive_exact(socket_fd, payload, sizeof(payload)))
        return 0;

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_ACCEPT) {
        pstvnc_wire_proof_accept_t acceptance;

        if (!pstvnc_wire_proof_accept_decode(
                &acceptance,
                payload,
                sizeof(payload)))
            return 0;

        *session_id = acceptance.session_id;
        return 1;
    }

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_NOT_ACCEPTED) {
        pstvnc_wire_proof_not_accepted_t rejection;

        if (!pstvnc_wire_proof_not_accepted_decode(
                &rejection,
                payload,
                sizeof(payload)))
            return 0;

        *rejection_reason = rejection.reason;
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int socket_fd = -1;
    int result;
    uint32_t session_id = 0u;
    uint32_t rejection_reason = 0u;
    char line[96];

    (void)argc;
    (void)argv;

    init_scr();

    proof_print("WIRE Q1-Q12 PROOF 1A");
    proof_print("state=BOOT");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        proof_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    proof_print("state=NETWORK_INIT");

    if (pstvnc_ps2_network_init() < 0) {
        proof_print("FAIL network_init");
        SleepThread();
        return 11;
    }

    if (pstvnc_ps2_network_wait_link() < 0) {
        proof_print("FAIL link_wait");
        SleepThread();
        return 12;
    }

    proof_print("state=TCP_CONNECT");

    socket_fd = pstvnc_ps2_network_connect_pstv();
    if (socket_fd < 0) {
        proof_print("FAIL tcp_connect");
        SleepThread();
        return 13;
    }

    proof_print("state=PROVISIONAL");

    if (!send_establishment_hello(socket_fd)) {
        proof_print("FAIL hello_send");
        pstvnc_ps2_network_close(socket_fd);
        SleepThread();
        return 14;
    }

    result = receive_establishment_result(
        socket_fd,
        &session_id,
        &rejection_reason);

    if (result < 0) {
        snprintf(
            line,
            sizeof(line),
            "NOT_ACCEPTED reason=%u",
            (unsigned int)rejection_reason);
        proof_print(line);
        pstvnc_ps2_network_close(socket_fd);
        SleepThread();
        return 15;
    }

    if (result == 0 || session_id == 0u) {
        proof_print("FAIL establishment_result");
        pstvnc_ps2_network_close(socket_fd);
        SleepThread();
        return 16;
    }

    snprintf(
        line,
        sizeof(line),
        "WIRE ACTIVE session_id=%u",
        (unsigned int)session_id);
    proof_print(line);

    proof_print("riders=NONE");
    proof_print("idle_window=10_seconds");

    if (pstvnc_ps2_system_delay_us(WIRE_PROOF_IDLE_US) < 0) {
        proof_print("FAIL idle_delay");
        pstvnc_ps2_network_close(socket_fd);
        SleepThread();
        return 17;
    }

    shutdown(socket_fd, SHUT_RDWR);
    pstvnc_ps2_network_close(socket_fd);

    proof_print("WIRE INACTIVE");
    proof_print("PROOF1A CLIENT COMPLETE");

    SleepThread();
    return 0;
}

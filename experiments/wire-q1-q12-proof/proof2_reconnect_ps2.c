/*
 * File synopsis:
 * Implements the PS2 half of Q9/Q12 Wire reconnect Proof 2.
 *
 * The PS2 initializes the private Ethernet stack once, establishes Wire
 * Session A, then blocks waiting for the Pi to deliberately close that
 * physical connection. The same still-running process retains session A's ID,
 * creates a new TCP connection, performs a fresh provisional handshake, and
 * requires the Pi-assigned Session B ID to be non-zero and different from A.
 *
 * No rider is started. This experiment isolates Wire Session lifetime,
 * local-process survival, reconnect establishment, and session-identity
 * replacement from RFB, audio, MPEG, desktop, and configuration behavior.
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

#define PROOF2_RECONNECT_DELAY_US 2000000u
#define PROOF2_SESSION_B_IDLE_US  3000000u

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

/*
 * Wait for the Pi to terminate Session A.
 *
 * No ordinary Wire data is expected in this proof. Zero means an orderly FIN;
 * a negative result also establishes that this descriptor can no longer make
 * progress. Any positive byte is a proof failure because the server promised
 * no post-establishment rider/control traffic before the forced loss.
 */
static int wait_for_wire_loss(int socket_fd)
{
    uint8_t unexpected_byte = 0u;
    int result = recv(socket_fd, &unexpected_byte, 1u, 0);

    if (result > 0)
        return 0;

    return 1;
}

static int establish_session(
    const char *provisional_state,
    int *socket_fd,
    uint32_t *session_id,
    uint32_t *rejection_reason)
{
    int result;

    if (provisional_state == NULL ||
        socket_fd == NULL ||
        session_id == NULL ||
        rejection_reason == NULL)
        return 0;

    *socket_fd = pstvnc_ps2_network_connect_pstv();
    if (*socket_fd < 0)
        return 0;

    proof_print(provisional_state);

    if (!send_establishment_hello(*socket_fd)) {
        pstvnc_ps2_network_close(*socket_fd);
        *socket_fd = -1;
        return 0;
    }

    result = receive_establishment_result(
        *socket_fd,
        session_id,
        rejection_reason);

    if (result != 1 || *session_id == 0u) {
        pstvnc_ps2_network_close(*socket_fd);
        *socket_fd = -1;
        return result;
    }

    return 1;
}

int main(int argc, char **argv)
{
    int socket_a = -1;
    int socket_b = -1;
    int result;
    uint32_t session_a = 0u;
    uint32_t session_b = 0u;
    uint32_t rejection_reason = 0u;
    char line[112];

    (void)argc;
    (void)argv;

    init_scr();

    proof_print("WIRE Q9/Q12 PROOF 2");
    proof_print("state=BOOT");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        proof_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    proof_print("state=NETWORK_INIT_ONCE");

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

    proof_print("state=SESSION_A_TCP_CONNECT");

    result = establish_session(
        "state=SESSION_A_PROVISIONAL",
        &socket_a,
        &session_a,
        &rejection_reason);

    if (result < 0) {
        snprintf(
            line,
            sizeof(line),
            "FAIL session_a_rejected reason=%u",
            (unsigned int)rejection_reason);
        proof_print(line);
        SleepThread();
        return 13;
    }

    if (result != 1) {
        proof_print("FAIL session_a_establishment");
        SleepThread();
        return 14;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_A ACTIVE id=%u",
        (unsigned int)session_a);
    proof_print(line);

    proof_print("riders=NONE");
    proof_print("state=WAIT_FOR_PI_FORCED_LOSS");

    if (!wait_for_wire_loss(socket_a)) {
        proof_print("FAIL unexpected_session_a_data");
        pstvnc_ps2_network_close(socket_a);
        SleepThread();
        return 15;
    }

    pstvnc_ps2_network_close(socket_a);
    socket_a = -1;

    snprintf(
        line,
        sizeof(line),
        "SESSION_A LOST retained_id=%u",
        (unsigned int)session_a);
    proof_print(line);

    proof_print("PROCESS_SURVIVED_WIRE_LOSS=YES");

    if (pstvnc_ps2_system_delay_us(PROOF2_RECONNECT_DELAY_US) < 0) {
        proof_print("FAIL reconnect_delay");
        SleepThread();
        return 16;
    }

    proof_print("state=SESSION_B_TCP_CONNECT");

    rejection_reason = 0u;

    result = establish_session(
        "state=SESSION_B_PROVISIONAL",
        &socket_b,
        &session_b,
        &rejection_reason);

    if (result < 0) {
        snprintf(
            line,
            sizeof(line),
            "FAIL session_b_rejected reason=%u",
            (unsigned int)rejection_reason);
        proof_print(line);
        SleepThread();
        return 17;
    }

    if (result != 1) {
        proof_print("FAIL session_b_establishment");
        SleepThread();
        return 18;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_B ACTIVE id=%u",
        (unsigned int)session_b);
    proof_print(line);

    if (session_b == session_a) {
        proof_print("FAIL session_id_reused");
        shutdown(socket_b, SHUT_RDWR);
        pstvnc_ps2_network_close(socket_b);
        SleepThread();
        return 19;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_A_RETAINED id=%u",
        (unsigned int)session_a);
    proof_print(line);

    proof_print("SESSION_IDS_DISTINCT=YES");
    proof_print("riders=NONE");
    proof_print("session_b_idle=3_seconds");

    if (pstvnc_ps2_system_delay_us(PROOF2_SESSION_B_IDLE_US) < 0) {
        proof_print("FAIL session_b_idle_delay");
        shutdown(socket_b, SHUT_RDWR);
        pstvnc_ps2_network_close(socket_b);
        SleepThread();
        return 20;
    }

    shutdown(socket_b, SHUT_RDWR);
    pstvnc_ps2_network_close(socket_b);
    socket_b = -1;

    proof_print("SESSION_B INACTIVE");
    proof_print("PROOF2 RECONNECT COMPLETE");

    SleepThread();
    return 0;
}

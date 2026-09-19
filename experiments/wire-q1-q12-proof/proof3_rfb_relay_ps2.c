/*
 * File synopsis:
 * Implements the PS2 half of Q3 Wire Channel Relay Proof 3.
 *
 * One physical PSTV stream owns the socket and sequence space from HELLO byte
 * one through establishment and all later relay traffic. This avoids a false
 * sequence reset between Q4 establishment and Q3 logical-channel service.
 *
 * The proof reuses:
 *   - Transport's real physical_stream framing/sequence owner;
 *   - Transport's real bounded rfb_channel byte storage;
 *   - RFB's real public bridge exact-read/exact-write vocabulary.
 *
 * Only the glue that connects those mechanisms for this experiment is local.
 * It deliberately does not run the RFB parser, framebuffer, provider, audio,
 * MPEG, application coordinator, or product recovery policy.
 */

#include <debug.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb/bridge.h"
#include "transport/bridge.h"
#include "transport/physical_stream.h"
#include "transport/protocol.h"
#include "transport/rfb_channel.h"

#include "wire_establish_protocol.h"

#define PROOF3_QUEUE_CAPACITY 32u
#define PROOF3_MAX_DATA_PAYLOAD 16u
#define PROOF3_STALL_DELAY_US 3000000u

static pstvnc_transport_physical_stream_t proof_stream;
static pstvnc_transport_rfb_channel_t proof_rfb_channel;
static uint8_t proof_rfb_storage[PROOF3_QUEUE_CAPACITY];
static int proof_relay_active;

static const uint8_t expected_inbound[PROOF3_QUEUE_CAPACITY] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

static const uint8_t expected_outbound[37] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4
};

static void proof_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int proof_send_credit(uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    pstvnc_transport_write_be32(payload, amount);

    return pstvnc_transport_physical_stream_send_frame(
        &proof_stream,
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload));
}

/*
 * These functions are the proof-local Transport side of RFB's real public
 * bridge. The RFB component sees only exact logical byte I/O; it never sees the
 * physical socket, PSTV framing, sequence numbers, queue storage, or credit.
 */

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count)
{
    if (!proof_relay_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (count > UINT32_MAX)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_rfb_channel_read_exact(
            &proof_rfb_channel,
            buffer,
            count) != 0)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    /*
     * Credit is earned by domain acceptance across the public boundary.
     * Merely receiving/queuing the bytes does not return credit.
     */
    if (count != 0u && !proof_send_credit((uint32_t)count))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    if (!proof_relay_active)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_rfb_channel_available(&proof_rfb_channel) != 0u
        ? PSTVNC_TRANSPORT_OK
        : PSTVNC_TRANSPORT_WOULD_BLOCK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t offset = 0u;

    if (!proof_relay_active || (buffer == NULL && count != 0u))
        return PSTVNC_TRANSPORT_INVALID;

    while (offset < count) {
        size_t fragment = count - offset;

        if (fragment > PROOF3_MAX_DATA_PAYLOAD)
            fragment = PROOF3_MAX_DATA_PAYLOAD;

        if (!pstvnc_transport_physical_stream_send_frame(
                &proof_stream,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0u,
                bytes + offset,
                fragment))
            return PSTVNC_TRANSPORT_FAILED;

        offset += fragment;
    }

    return PSTVNC_TRANSPORT_OK;
}

/* Quiesce is outside Proof 3's bounded Relay discriminator. */
pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void)
{
    return proof_relay_active
        ? PSTVNC_TRANSPORT_WOULD_BLOCK
        : PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    size_t *residual_count)
{
    if (residual_count != NULL)
        *residual_count = 0u;

    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count)
{
    (void)expected_count;

    if (discarded_count != NULL)
        *discarded_count = 0u;

    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

static int establish_wire(uint32_t *session_id)
{
    uint8_t hello_payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t result_payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_wire_proof_hello_t hello;
    pstvnc_wire_proof_accept_t acceptance;
    pstvnc_wire_proof_not_accepted_t rejection;
    pstvnc_transport_header_t header;

    if (session_id == NULL)
        return 0;

    *session_id = 0u;

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(hello_payload, &hello))
        return 0;

    /*
     * physical_stream owns the connection sequence from the first Wire frame.
     * HELLO is therefore PS2->Pi sequence 1.
     */
    if (!pstvnc_transport_physical_stream_send_frame(
            &proof_stream,
            PSTVNC_TRANSPORT_FRAME_HELLO,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            hello_payload,
            sizeof(hello_payload)))
        return 0;

    /*
     * ACCEPT/NOT_ACCEPTED is Pi->PS2 sequence 1. After this receive succeeds,
     * the next inbound sequence is 2 without any handoff/reset.
     */
    if (!pstvnc_transport_physical_stream_receive_frame(
            &proof_stream,
            &header,
            result_payload,
            sizeof(result_payload)))
        return 0;

    if (header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.payload_length != sizeof(result_payload))
        return 0;

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_ACCEPT) {
        if (!pstvnc_wire_proof_accept_decode(
                &acceptance,
                result_payload,
                sizeof(result_payload)) ||
            acceptance.session_id == 0u)
            return 0;

        *session_id = acceptance.session_id;
        return 1;
    }

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_NOT_ACCEPTED) {
        if (!pstvnc_wire_proof_not_accepted_decode(
                &rejection,
                result_payload,
                sizeof(result_payload)))
            return 0;

        return -(int)rejection.reason;
    }

    return 0;
}

static int receive_rfb_fragment(size_t expected_length)
{
    uint8_t payload[PROOF3_MAX_DATA_PAYLOAD];
    pstvnc_transport_header_t header;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &proof_stream,
            &header,
            payload,
            sizeof(payload)))
        return 0;

    if (header.kind != PSTVNC_TRANSPORT_FRAME_DATA ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_RFB ||
        header.flags != 0u ||
        header.payload_length != expected_length)
        return 0;

    return pstvnc_transport_rfb_channel_commit(
        &proof_rfb_channel,
        payload,
        header.payload_length) == 0;
}

static int receive_heartbeat(void)
{
    uint8_t payload[1];
    pstvnc_transport_header_t header;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &proof_stream,
            &header,
            payload,
            sizeof(payload)))
        return 0;

    return header.kind == PSTVNC_TRANSPORT_FRAME_HEARTBEAT &&
        header.channel == PSTVNC_TRANSPORT_CHANNEL_CONTROL &&
        header.flags == 0u &&
        header.payload_length == 0u;
}

static int send_heartbeat(void)
{
    return pstvnc_transport_physical_stream_send_frame(
        &proof_stream,
        PSTVNC_TRANSPORT_FRAME_HEARTBEAT,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0u,
        NULL,
        0u);
}

int main(int argc, char **argv)
{
    int socket_fd = -1;
    int establish_result;
    uint32_t session_id = 0u;
    uint8_t domain_bytes[PROOF3_QUEUE_CAPACITY];
    char line[128];

    (void)argc;
    (void)argv;

    memset(&proof_stream, 0, sizeof(proof_stream));
    proof_stream.socket_fd = -1;
    proof_stream.send_semaphore_id = -1;

    init_scr();

    proof_print("WIRE Q3 PROOF 3 RFB RELAY");
    proof_print("state=BOOT");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        proof_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    proof_print("state=NETWORK_INIT");

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        proof_print("FAIL network");
        SleepThread();
        return 11;
    }

    proof_print("state=TCP_CONNECT");

    socket_fd = pstvnc_ps2_network_connect_pstv();
    if (socket_fd < 0) {
        proof_print("FAIL tcp_connect");
        SleepThread();
        return 12;
    }

    /*
     * Socket ownership enters Transport before Wire byte one, so establishment
     * and rider traffic share one unbroken sequence space.
     */
    if (!pstvnc_transport_physical_stream_adopt(
            &proof_stream,
            socket_fd)) {
        pstvnc_ps2_network_close(socket_fd);
        proof_print("FAIL stream_adopt");
        SleepThread();
        return 13;
    }

    socket_fd = -1;

    proof_print("state=PROVISIONAL");

    establish_result = establish_wire(&session_id);
    if (establish_result <= 0) {
        snprintf(
            line,
            sizeof(line),
            "FAIL establishment result=%d",
            establish_result);
        proof_print(line);
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 14;
    }

    snprintf(
        line,
        sizeof(line),
        "WIRE ACTIVE session_id=%u",
        (unsigned int)session_id);
    proof_print(line);

    if (pstvnc_transport_rfb_channel_initialize(
            &proof_rfb_channel,
            proof_rfb_storage,
            sizeof(proof_rfb_storage)) != 0) {
        proof_print("FAIL relay_init");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 15;
    }

    proof_relay_active = 1;

    proof_print("RFB_RELAY capacity=32 max_payload=16");

    /*
     * This is PS2->Pi sequence 2 because HELLO already consumed sequence 1.
     */
    if (!proof_send_credit(PROOF3_QUEUE_CAPACITY)) {
        proof_print("FAIL initial_credit");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 16;
    }

    proof_print("RFB_INITIAL_CREDIT=32");

    /*
     * Pi->PS2 sequences 2 and 3 fill the bounded Relay exactly.
     * No domain read has occurred, therefore no return credit is generated.
     */
    if (!receive_rfb_fragment(PROOF3_MAX_DATA_PAYLOAD) ||
        !receive_rfb_fragment(PROOF3_MAX_DATA_PAYLOAD)) {
        proof_print("FAIL inbound_fragments");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 17;
    }

    if (pstvnc_transport_rfb_channel_available(
            &proof_rfb_channel) != PROOF3_QUEUE_CAPACITY) {
        proof_print("FAIL relay_not_full");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 18;
    }

    proof_print("WIRE_RECEIVED_RFB_BYTES=32");
    proof_print("RFB_RELAY_AVAILABLE=32");
    proof_print("RFB_DOMAIN_ACCEPTED=0");
    proof_print("state=RFB_DOMAIN_STALLED");

    /*
     * Pi->PS2 sequence 4 is CONTROL traffic. Receiving it while the RFB Relay
     * is full proves that one stalled rider does not inherently stop Wire.
     */
    if (!receive_heartbeat()) {
        proof_print("FAIL stalled_wire_progress");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 19;
    }

    /*
     * PS2->Pi sequence 3 is also CONTROL traffic; initial CREDIT was sequence 2.
     */
    if (!send_heartbeat()) {
        proof_print("FAIL stalled_wire_reply");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 20;
    }

    proof_print("WIRE_PROGRESS_WHILE_RFB_STALLED=YES");

    if (pstvnc_ps2_system_delay_us(PROOF3_STALL_DELAY_US) < 0) {
        proof_print("FAIL stall_delay");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 21;
    }

    /*
     * This is the actual RFB component's public exact-read boundary.
     * The proof-local Transport backing sends CREDIT only after this succeeds.
     */
    if (pstvnc_rfb_bridge_read_exact(
            domain_bytes,
            sizeof(domain_bytes)) != 0) {
        proof_print("FAIL domain_accept");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 22;
    }

    proof_print("RFB_DOMAIN_ACCEPTED=32");

    if (memcmp(
            domain_bytes,
            expected_inbound,
            sizeof(domain_bytes)) != 0) {
        proof_print("FAIL domain_bytes");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 23;
    }

    proof_print("RFB_DOMAIN_PROCESSED=32");
    proof_print("RFB_RETURN_CREDIT=32");

    if (pstvnc_transport_rfb_channel_available(
            &proof_rfb_channel) != 0u) {
        proof_print("FAIL relay_not_empty");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 24;
    }

    /*
     * RFB creates opaque domain bytes before handoff. The real RFB bridge
     * presents them to proof-local Transport, which fragments 37 bytes into
     * 16 + 16 + 5 channel-1 DATA frames without interpreting them.
     */
    if (pstvnc_rfb_bridge_write_exact(
            expected_outbound,
            sizeof(expected_outbound)) != 0) {
        proof_print("FAIL outbound_handoff");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 25;
    }

    proof_print("RFB_OUTBOUND_BYTES=37");
    proof_print("RFB_OUTBOUND_FRAGMENTS=16+16+5");

    /*
     * Final Pi heartbeat is inbound sequence 5, proving the Wire remains usable
     * after inbound stall, domain acceptance, credit return, and outbound relay.
     */
    if (!receive_heartbeat()) {
        proof_print("FAIL final_wire_health");
        pstvnc_transport_physical_stream_release(&proof_stream);
        SleepThread();
        return 26;
    }

    proof_print("WIRE_HEALTHY_AFTER_RELAY=YES");

    proof_relay_active = 0;
    pstvnc_transport_physical_stream_release(&proof_stream);

    proof_print("WIRE INACTIVE");
    proof_print("PROOF3 RFB RELAY COMPLETE");

    SleepThread();
    return 0;
}

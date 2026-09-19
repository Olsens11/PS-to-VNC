/*
 * File synopsis:
 * Hardware discriminator for the Proof 4 PS2IP full-duplex stall.
 *
 * Proof 4 used the clean Transport runtime's receive thread while the caller
 * thread emitted CREDIT/DATA on the same TCP socket. Hardware packet capture
 * showed a bidirectional stall: a PS2 CREDIT header reached the peer without
 * its payload while the PS2 simultaneously stopped acknowledging the next
 * inbound RFB frame.
 *
 * This experiment preserves the same PSTV transaction shape but deliberately
 * issues EVERY physical send and receive from this one EE/main thread.
 *
 * It is NOT a replacement Transport implementation and it does NOT prove
 * Q8/Q11 domain failure containment. It answers one lower-level hardware
 * question only:
 *
 *   Does the problematic transaction remain live when one PS2 execution
 *   context owns all socket I/O?
 */

#include <debug.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "transport/physical_stream.h"
#include "transport/protocol.h"

#include "wire_establish_protocol.h"

#define PROOF4B_RFB_BYTES 32u
#define PROOF4B_AUDIO_TRIGGER_BYTES 4u
#define PROOF4B_AUDIO_FULL_BYTES 16u
#define PROOF4B_OUTBOUND_RFB_BYTES 37u
#define PROOF4B_MAX_PAYLOAD 16u

static const uint8_t expected_rfb_first[PROOF4B_RFB_BYTES] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

static const uint8_t expected_rfb_second[PROOF4B_RFB_BYTES] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
};

static const uint8_t expected_audio_trigger[PROOF4B_AUDIO_TRIGGER_BYTES] = {
    0xa1, 0xa2, 0xa3, 0xa4
};

static const uint8_t expected_audio_full[PROOF4B_AUDIO_FULL_BYTES] = {
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
};

static const uint8_t outbound_rfb[PROOF4B_OUTBOUND_RFB_BYTES] = {
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

static void write_be32(uint8_t destination[4], uint32_t value)
{
    destination[0] = (uint8_t)((value >> 24) & 0xffu);
    destination[1] = (uint8_t)((value >> 16) & 0xffu);
    destination[2] = (uint8_t)((value >> 8) & 0xffu);
    destination[3] = (uint8_t)(value & 0xffu);
}

static int send_credit(
    pstvnc_transport_physical_stream_t *stream,
    uint8_t channel,
    uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    write_be32(payload, amount);

    return pstvnc_transport_physical_stream_send_frame(
        stream,
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        channel,
        0u,
        payload,
        sizeof(payload));
}

static int send_heartbeat(
    pstvnc_transport_physical_stream_t *stream)
{
    return pstvnc_transport_physical_stream_send_frame(
        stream,
        PSTVNC_TRANSPORT_FRAME_HEARTBEAT,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0u,
        NULL,
        0u);
}

static int receive_data(
    pstvnc_transport_physical_stream_t *stream,
    uint8_t channel,
    uint8_t *destination,
    size_t expected_count)
{
    pstvnc_transport_header_t header;

    if (!pstvnc_transport_physical_stream_receive_frame(
            stream,
            &header,
            destination,
            expected_count))
        return 0;

    if (header.kind != PSTVNC_TRANSPORT_FRAME_DATA ||
        header.channel != channel ||
        header.flags != 0u ||
        header.payload_length != expected_count)
        return 0;

    return 1;
}

static int establish_wire(
    pstvnc_transport_physical_stream_t *stream,
    uint32_t *session_id)
{
    uint8_t hello_payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t result_payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_wire_proof_hello_t hello;
    pstvnc_wire_proof_accept_t acceptance;
    pstvnc_wire_proof_not_accepted_t rejection;
    pstvnc_transport_header_t header;

    if (stream == NULL || session_id == NULL)
        return 0;

    *session_id = 0u;

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(
            hello_payload,
            &hello))
        return 0;

    /*
     * PS2 -> Pi sequence 1.
     * The same physical stream remains owned by this same EE thread for every
     * subsequent send and receive in the diagnostic transaction.
     */
    if (!pstvnc_transport_physical_stream_send_frame(
            stream,
            PSTVNC_TRANSPORT_FRAME_HELLO,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            hello_payload,
            sizeof(hello_payload)))
        return 0;

    if (!pstvnc_transport_physical_stream_receive_frame(
            stream,
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

static int send_outbound_rfb(
    pstvnc_transport_physical_stream_t *stream)
{
    size_t offset = 0u;

    while (offset < sizeof(outbound_rfb)) {
        size_t remaining = sizeof(outbound_rfb) - offset;
        size_t count = remaining;

        if (count > PROOF4B_MAX_PAYLOAD)
            count = PROOF4B_MAX_PAYLOAD;

        if (!pstvnc_transport_physical_stream_send_frame(
                stream,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0u,
                outbound_rfb + offset,
                count))
            return 0;

        offset += count;
    }

    return 1;
}

int main(int argc, char **argv)
{
    int socket_fd = -1;
    int establish_result;
    uint32_t session_id = 0u;
    pstvnc_transport_physical_stream_t stream;
    uint8_t first_rfb[PROOF4B_RFB_BYTES];
    uint8_t second_rfb[PROOF4B_RFB_BYTES];
    uint8_t audio_trigger[PROOF4B_AUDIO_TRIGGER_BYTES];
    uint8_t audio_full[PROOF4B_AUDIO_FULL_BYTES];
    char line[128];

    (void)argc;
    (void)argv;

    memset(&stream, 0, sizeof(stream));

    init_scr();

    proof_print("WIRE IO-OWNER PROOF 4B");
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
        proof_print("FAIL connect");
        SleepThread();
        return 12;
    }

    if (!pstvnc_transport_physical_stream_adopt(
            &stream,
            socket_fd)) {
        pstvnc_ps2_network_close(socket_fd);
        proof_print("FAIL stream_adopt");
        SleepThread();
        return 13;
    }

    socket_fd = -1;

    proof_print("PHYSICAL_IO_OWNER=ONE_EE_THREAD");
    proof_print("state=PROVISIONAL");

    establish_result = establish_wire(
        &stream,
        &session_id);

    if (establish_result <= 0) {
        snprintf(
            line,
            sizeof(line),
            "FAIL establish=%d",
            establish_result);
        proof_print(line);
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 14;
    }

    snprintf(
        line,
        sizeof(line),
        "WIRE ACTIVE id=%u",
        (unsigned int)session_id);
    proof_print(line);

    /*
     * PS2 sequences 2 and 3.
     */
    if (!send_credit(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            32u) ||
        !send_credit(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            16u)) {
        proof_print("FAIL initial_credit");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 15;
    }

    proof_print("INITIAL_CREDITS=32+16");

    /*
     * Pi sequences 2 and 3: first RFB window.
     */
    if (!receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            first_rfb,
            16u) ||
        !receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            first_rfb + 16u,
            16u) ||
        memcmp(
            first_rfb,
            expected_rfb_first,
            sizeof(first_rfb)) != 0) {
        proof_print("FAIL first_rfb");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 16;
    }

    proof_print("FIRST_RFB_WINDOW=32");

    /*
     * Pi sequence 4: four AUDIO bytes.
     * PS2 sequence 4: return exactly four AUDIO credits.
     */
    if (!receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            audio_trigger,
            sizeof(audio_trigger)) ||
        memcmp(
            audio_trigger,
            expected_audio_trigger,
            sizeof(audio_trigger)) != 0 ||
        !send_credit(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            4u)) {
        proof_print("FAIL audio_trigger");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 17;
    }

    proof_print("AUDIO_TRIGGER_ROUNDTRIP=4");

    /*
     * Pi sequence 5: 16 more AUDIO bytes.
     * They are intentionally not credited again.
     */
    if (!receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            audio_full,
            sizeof(audio_full)) ||
        memcmp(
            audio_full,
            expected_audio_full,
            sizeof(audio_full)) != 0) {
        proof_print("FAIL audio_full");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 18;
    }

    proof_print("AUDIO_SECOND_PAYLOAD=16");

    /*
     * PS2 sequence 5: neutral machine-visible ordering marker.
     */
    if (!send_heartbeat(&stream)) {
        proof_print("FAIL marker");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 19;
    }

    proof_print("ORDER_MARKER_SENT=YES");

    /*
     * PS2 sequence 6: return first RFB window credit.
     */
    if (!send_credit(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            32u)) {
        proof_print("FAIL first_rfb_credit");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 20;
    }

    proof_print("FIRST_RFB_CREDIT=32");

    /*
     * Pi sequences 6 and 7: this is the exact transaction boundary where the
     * split-thread Proof 4 runtime stalled.
     */
    if (!receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            second_rfb,
            16u) ||
        !receive_data(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            second_rfb + 16u,
            16u) ||
        memcmp(
            second_rfb,
            expected_rfb_second,
            sizeof(second_rfb)) != 0) {
        proof_print("FAIL second_rfb");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 21;
    }

    proof_print("SECOND_RFB_WINDOW=32");

    /*
     * PS2 sequence 7.
     *
     * In failing Proof 4 hardware evidence only this CREDIT frame's 16-byte
     * PSTV header reached the Pi. Its four-byte payload never appeared.
     */
    if (!send_credit(
            &stream,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            32u)) {
        proof_print("FAIL second_rfb_credit");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 22;
    }

    proof_print("SEQ7_CREDIT_COMPLETE=YES");

    /*
     * PS2 sequences 8, 9 and 10.
     */
    if (!send_outbound_rfb(&stream)) {
        proof_print("FAIL outbound_rfb");
        (void)pstvnc_transport_physical_stream_release(&stream);
        SleepThread();
        return 23;
    }

    proof_print("OUTBOUND_RFB=16+16+5");
    proof_print("SINGLE_IO_TRANSACTION=PASS");

    (void)pstvnc_transport_physical_stream_release(&stream);

    proof_print("WIRE INACTIVE");
    proof_print("PROOF4B IO OWNER COMPLETE");

    SleepThread();
    return 0;
}

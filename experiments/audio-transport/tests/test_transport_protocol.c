#include "transport_protocol.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_header_round_trip(void)
{
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    pstvnc_transport_header_t input;
    pstvnc_transport_header_t output;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.version = PSTVNC_TRANSPORT_VERSION;
    input.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    input.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;
    input.flags = 0x5au;
    input.sequence = 0x12345678u;
    input.payload_length = 4096u;

    assert(
        pstvnc_transport_header_encode(
            wire,
            &input));

    assert(wire[0] == 'P');
    assert(wire[1] == 'S');
    assert(wire[2] == 'T');
    assert(wire[3] == 'V');

    assert(wire[8] == 0x12u);
    assert(wire[9] == 0x34u);
    assert(wire[10] == 0x56u);
    assert(wire[11] == 0x78u);

    assert(
        pstvnc_transport_header_decode(
            &output,
            wire));

    assert(output.version == input.version);
    assert(output.kind == input.kind);
    assert(output.channel == input.channel);
    assert(output.flags == input.flags);
    assert(output.sequence == input.sequence);
    assert(output.payload_length == input.payload_length);
}

static void test_invalid_magic_rejected(void)
{
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    pstvnc_transport_header_t header;

    memset(&header, 0, sizeof(header));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_AUDIO;
    header.sequence = 7u;
    header.payload_length = 128u;

    assert(
        pstvnc_transport_header_encode(
            wire,
            &header));

    wire[0] = 'X';

    assert(
        !pstvnc_transport_header_decode(
            &header,
            wire));
}

static void test_invalid_version_rejected(void)
{
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    pstvnc_transport_header_t header;

    memset(&header, 0, sizeof(header));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_HEARTBEAT;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    header.sequence = 8u;
    header.payload_length = 0;

    assert(
        pstvnc_transport_header_encode(
            wire,
            &header));

    wire[4] =
        (uint8_t)(
            PSTVNC_TRANSPORT_VERSION + 1u);

    assert(
        !pstvnc_transport_header_decode(
            &header,
            wire));
}

static void test_oversized_payload_rejected(void)
{
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    pstvnc_transport_header_t header;

    memset(&header, 0, sizeof(header));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;
    header.sequence = 9u;

    header.payload_length =
        PSTVNC_TRANSPORT_MAX_PAYLOAD + 1u;

    assert(
        !pstvnc_transport_header_encode(
            wire,
            &header));

    header.payload_length =
        PSTVNC_TRANSPORT_MAX_PAYLOAD;

    assert(
        pstvnc_transport_header_encode(
            wire,
            &header));

    pstvnc_transport_write_be32(
        &wire[12],
        PSTVNC_TRANSPORT_MAX_PAYLOAD + 1u);

    assert(
        !pstvnc_transport_header_decode(
            &header,
            wire));
}

int main(void)
{
    test_header_round_trip();
    test_invalid_magic_rejected();
    test_invalid_version_rejected();
    test_oversized_payload_rejected();

    puts("TRANSPORT_PROTOCOL_TEST=PASS");
    return 0;
}

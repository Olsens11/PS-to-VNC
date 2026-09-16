#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "transport/protocol.h"

static int failures;

#define CHECK(expression)                                                \
    do {                                                                 \
        if (!(expression)) {                                             \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expression);                    \
            failures++;                                                  \
        }                                                                \
    } while (0)

static void test_big_endian_helpers(void)
{
    uint8_t bytes[4];

    pstvnc_transport_write_be32(bytes, 0x12345678u);
    CHECK(bytes[0] == 0x12u);
    CHECK(bytes[1] == 0x34u);
    CHECK(bytes[2] == 0x56u);
    CHECK(bytes[3] == 0x78u);
    CHECK(pstvnc_transport_read_be32(bytes) == 0x12345678u);
}

static void test_header_round_trip_and_exact_wire_bytes(void)
{
    pstvnc_transport_header_t input;
    pstvnc_transport_header_t output;
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];
    static const uint8_t expected[PSTVNC_TRANSPORT_HEADER_SIZE] = {
        0x50u, 0x53u, 0x54u, 0x56u,
        0x01u, 0x03u, 0x01u, 0xa5u,
        0x01u, 0x02u, 0x03u, 0x04u,
        0x00u, 0x00u, 0x10u, 0x00u
    };

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));
    memset(wire, 0, sizeof(wire));

    input.version = PSTVNC_TRANSPORT_VERSION;
    input.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    input.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;
    input.flags = 0xa5u;
    input.sequence = 0x01020304u;
    input.payload_length = 4096u;

    CHECK(pstvnc_transport_header_encode(wire, &input));
    CHECK(memcmp(wire, expected, sizeof(wire)) == 0);
    CHECK(pstvnc_transport_header_decode(&output, wire));
    CHECK(output.version == input.version);
    CHECK(output.kind == input.kind);
    CHECK(output.channel == input.channel);
    CHECK(output.flags == input.flags);
    CHECK(output.sequence == input.sequence);
    CHECK(output.payload_length == input.payload_length);
}

static void test_encode_rejects_invalid_contract(void)
{
    pstvnc_transport_header_t header;
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    memset(&header, 0, sizeof(header));
    memset(wire, 0, sizeof(wire));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;

    CHECK(!pstvnc_transport_header_encode(NULL, &header));
    CHECK(!pstvnc_transport_header_encode(wire, NULL));

    header.version = (uint8_t)(PSTVNC_TRANSPORT_VERSION + 1u);
    CHECK(!pstvnc_transport_header_encode(wire, &header));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.payload_length = PSTVNC_TRANSPORT_MAX_PAYLOAD + 1u;
    CHECK(!pstvnc_transport_header_encode(wire, &header));
}

static void test_decode_rejects_invalid_wire_contract(void)
{
    pstvnc_transport_header_t header;
    uint8_t wire[PSTVNC_TRANSPORT_HEADER_SIZE];

    memset(&header, 0, sizeof(header));
    memset(wire, 0, sizeof(wire));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;
    header.sequence = 7u;
    header.payload_length = 1u;
    CHECK(pstvnc_transport_header_encode(wire, &header));

    CHECK(!pstvnc_transport_header_decode(NULL, wire));
    CHECK(!pstvnc_transport_header_decode(&header, NULL));

    wire[0] ^= 0xffu;
    CHECK(!pstvnc_transport_header_decode(&header, wire));
    wire[0] ^= 0xffu;

    wire[4] = (uint8_t)(PSTVNC_TRANSPORT_VERSION + 1u);
    CHECK(!pstvnc_transport_header_decode(&header, wire));
    wire[4] = PSTVNC_TRANSPORT_VERSION;

    pstvnc_transport_write_be32(
        &wire[12],
        PSTVNC_TRANSPORT_MAX_PAYLOAD + 1u);
    CHECK(!pstvnc_transport_header_decode(&header, wire));
}

int main(void)
{
    test_big_endian_helpers();
    test_header_round_trip_and_exact_wire_bytes();
    test_encode_rejects_invalid_contract();
    test_decode_rejects_invalid_wire_contract();

    if (failures != 0) {
        fprintf(stderr, "transport protocol tests failed: %d\n", failures);
        return 1;
    }

    puts("transport protocol tests passed");
    return 0;
}

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

static void test_mpeg_retire_exact_codec(void)
{
    pstvnc_mpeg_retire_payload_t input;
    pstvnc_mpeg_retire_payload_t output;
    uint8_t wire[PSTVNC_MPEG_RETIRE_PAYLOAD_SIZE];
    static const uint8_t expected[PSTVNC_MPEG_RETIRE_PAYLOAD_SIZE] = {
        0x00u, 0x00u, 0x00u, 0x01u,
        0x12u, 0x34u, 0x56u, 0x78u,
        0x9au, 0xbcu, 0xdeu, 0xf0u
    };

    input.version = PSTVNC_MPEG_GENERATION_CONTROL_VERSION;
    input.session_id = 0x12345678u;
    input.generation = 0x9abcdef0u;
    memset(&output, 0, sizeof(output));

    CHECK(pstvnc_mpeg_retire_payload_encode(wire, &input));
    CHECK(memcmp(wire, expected, sizeof(wire)) == 0);
    CHECK(pstvnc_mpeg_retire_payload_decode(&output, wire, sizeof(wire)));
    CHECK(output.version == input.version);
    CHECK(output.session_id == input.session_id);
    CHECK(output.generation == input.generation);
    CHECK(!pstvnc_mpeg_retire_payload_decode(&output, wire, sizeof(wire) - 1u));

    wire[3] = 2u;
    CHECK(!pstvnc_mpeg_retire_payload_decode(&output, wire, sizeof(wire)));
}

static void fill_start(pstvnc_mpeg_start_payload_t *start)
{
    memset(start, 0, sizeof(*start));
    start->version = PSTVNC_MPEG_GENERATION_CONTROL_VERSION;
    start->session_id = 0x10203040u;
    start->generation = 7u;
    start->base_x = 0u;
    start->base_y = 0u;
    start->base_width = 640u;
    start->base_height = 448u;
    start->suppression_x = 16u;
    start->suppression_y = 24u;
    start->suppression_width = 320u;
    start->suppression_height = 180u;
}

static void test_mpeg_start_exact_codec(void)
{
    pstvnc_mpeg_start_payload_t input;
    pstvnc_mpeg_start_payload_t output;
    uint8_t wire[PSTVNC_MPEG_START_PAYLOAD_SIZE];
    uint8_t rewritten[PSTVNC_MPEG_START_PAYLOAD_SIZE];

    fill_start(&input);
    memset(&output, 0, sizeof(output));

    CHECK(pstvnc_mpeg_start_payload_encode(wire, &input));
    CHECK(pstvnc_mpeg_start_payload_decode(&output, wire, sizeof(wire)));
    CHECK(memcmp(&output, &input, sizeof(input)) == 0);
    CHECK(pstvnc_mpeg_start_payload_encode(rewritten, &output));
    CHECK(memcmp(rewritten, wire, sizeof(wire)) == 0);
    CHECK(!pstvnc_mpeg_start_payload_decode(&output, wire, sizeof(wire) - 1u));

    wire[3] = 2u;
    CHECK(!pstvnc_mpeg_start_payload_decode(&output, wire, sizeof(wire)));
}

static void test_explicit_mpeg_frame_identity_ignores_payload_shape(void)
{
    pstvnc_transport_header_t header;
    pstvnc_mpeg_start_payload_t start;
    uint8_t collision_payload[PSTVNC_MPEG_START_PAYLOAD_SIZE];

    fill_start(&start);
    CHECK(pstvnc_mpeg_start_payload_encode(collision_payload, &start));

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_DATA;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_MPEG2;
    header.flags = 0u;
    header.payload_length = sizeof(collision_payload);

    /* Even START-shaped 44-byte bytes remain ordinary MPEG by frame identity. */
    CHECK(pstvnc_transport_header_is_mpeg_data(&header));
    CHECK(!pstvnc_transport_header_is_mpeg_start(&header));
    CHECK(!pstvnc_transport_header_is_mpeg_retire(&header));

    header.kind = PSTVNC_TRANSPORT_FRAME_MPEG_START;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    CHECK(!pstvnc_transport_header_is_mpeg_data(&header));
    CHECK(pstvnc_transport_header_is_mpeg_start(&header));

    header.kind = PSTVNC_TRANSPORT_FRAME_MPEG_RETIRE;
    header.payload_length = PSTVNC_MPEG_RETIRE_PAYLOAD_SIZE;
    CHECK(!pstvnc_transport_header_is_mpeg_data(&header));
    CHECK(!pstvnc_transport_header_is_mpeg_start(&header));
    CHECK(pstvnc_transport_header_is_mpeg_retire(&header));
}

int main(void)
{
    test_big_endian_helpers();
    test_header_round_trip_and_exact_wire_bytes();
    test_encode_rejects_invalid_contract();
    test_decode_rejects_invalid_wire_contract();
    test_mpeg_retire_exact_codec();
    test_mpeg_start_exact_codec();
    test_explicit_mpeg_frame_identity_ignores_payload_shape();

    if (failures != 0) {
        fprintf(stderr, "transport protocol tests failed: %d\n", failures);
        return 1;
    }

    puts("transport protocol tests passed");
    return 0;
}

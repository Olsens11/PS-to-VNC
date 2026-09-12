/*
 * File synopsis:
 * Strict host contract tests for the fixed CP2P MPEG-start payload codec.
 */
#include "h1_mpeg_start_wire.h"

#include "transport_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pstvnc_h1_mpeg_start_contract_t valid_contract(void)
{
    pstvnc_h1_mpeg_start_contract_t contract;

    memset(&contract, 0, sizeof(contract));
    contract.draw_x = 176;
    contract.draw_y = 119;
    contract.draw_width = 352;
    contract.draw_height = 224;
    contract.inner_matte_x = 8;
    contract.inner_matte_y = 4;
    contract.suppression_rect.x = 164;
    contract.suppression_rect.y = 113;
    contract.suppression_rect.width = 376;
    contract.suppression_rect.height = 236;
    contract.generation = 7u;
    return contract;
}

static void test_encode_is_exact_44_byte_big_endian_schema(void)
{
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();
    uint8_t payload[PSTVNC_H1_MPEG_START_WIRE_BYTES];

    memset(payload, 0xa5, sizeof(payload));
    assert(sizeof(payload) == 44u);
    assert(pstvnc_h1_mpeg_start_wire_encode(payload, 0x10203040u, &contract));

    assert(pstvnc_transport_read_be32(&payload[0]) ==
        PSTVNC_H1_MPEG_START_WIRE_VERSION);
    assert(pstvnc_transport_read_be32(&payload[4]) == 0x10203040u);
    assert(pstvnc_transport_read_be32(&payload[8]) == 7u);
    assert(pstvnc_transport_read_be32(&payload[12]) == 176u);
    assert(pstvnc_transport_read_be32(&payload[16]) == 119u);
    assert(pstvnc_transport_read_be32(&payload[20]) == 352u);
    assert(pstvnc_transport_read_be32(&payload[24]) == 224u);
    assert(pstvnc_transport_read_be32(&payload[28]) == 164u);
    assert(pstvnc_transport_read_be32(&payload[32]) == 113u);
    assert(pstvnc_transport_read_be32(&payload[36]) == 376u);
    assert(pstvnc_transport_read_be32(&payload[40]) == 236u);
}

static void test_round_trip_preserves_pi_owned_start_facts(void)
{
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();
    pstvnc_h1_mpeg_start_wire_message_t decoded;
    uint8_t payload[PSTVNC_H1_MPEG_START_WIRE_BYTES];

    assert(pstvnc_h1_mpeg_start_wire_encode(payload, 91u, &contract));
    assert(pstvnc_h1_mpeg_start_wire_decode(&decoded, payload));

    assert(decoded.session_id == 91u);
    assert(decoded.generation == contract.generation);
    assert(decoded.draw_x == contract.draw_x);
    assert(decoded.draw_y == contract.draw_y);
    assert(decoded.draw_width == contract.draw_width);
    assert(decoded.draw_height == contract.draw_height);
    assert(memcmp(
        &decoded.suppression_rect,
        &contract.suppression_rect,
        sizeof(decoded.suppression_rect)) == 0);
}

static void test_invalid_or_stale_shape_is_rejected(void)
{
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();
    pstvnc_h1_mpeg_start_wire_message_t decoded;
    uint8_t payload[PSTVNC_H1_MPEG_START_WIRE_BYTES];

    contract.generation = 0u;
    assert(!pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));

    contract = valid_contract();
    contract.draw_width = 350;
    assert(!pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));

    contract = valid_contract();
    contract.suppression_rect.x = 200;
    assert(!pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));

    contract = valid_contract();
    assert(pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));

    pstvnc_transport_write_be32(&payload[0],
        PSTVNC_H1_MPEG_START_WIRE_VERSION + 1u);
    assert(!pstvnc_h1_mpeg_start_wire_decode(&decoded, payload));

    assert(pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));
    pstvnc_transport_write_be32(&payload[8], 0u);
    assert(!pstvnc_h1_mpeg_start_wire_decode(&decoded, payload));

    assert(pstvnc_h1_mpeg_start_wire_encode(payload, 1u, &contract));
    pstvnc_transport_write_be32(&payload[28], 200u);
    assert(!pstvnc_h1_mpeg_start_wire_decode(&decoded, payload));
}

int main(void)
{
    test_encode_is_exact_44_byte_big_endian_schema();
    test_round_trip_preserves_pi_owned_start_facts();
    test_invalid_or_stale_shape_is_rejected();

    puts("H1_MPEG_START_WIRE_HOST_TEST=PASS");
    return 0;
}

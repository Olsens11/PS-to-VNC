/*
 * File synopsis:
 * Strict host contract tests for the CP2P MPEG-start transport seam.
 *
 * The test replaces H1's internal framed-send bridge with a spy so it can prove
 * the new seam selects DATA/MPEG2, emits exactly one 44-byte payload, and never
 * bypasses the existing transport writer.
 */
#include "h1_mpeg_start_transport.h"

#include "transport_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned int send_calls;
static int send_result;
static struct pstvnc_h1_transport_runtime *last_runtime;
static uint8_t last_kind;
static uint8_t last_channel;
static size_t last_payload_length;
static uint8_t last_payload[PSTVNC_H1_MPEG_START_WIRE_BYTES];

int pstvnc_h1_transport_send_frame_internal(
    struct pstvnc_h1_transport_runtime *runtime,
    uint8_t kind,
    uint8_t channel,
    const void *payload,
    size_t payload_length)
{
    send_calls++;
    last_runtime = runtime;
    last_kind = kind;
    last_channel = channel;
    last_payload_length = payload_length;

    assert(payload != NULL);
    assert(payload_length == sizeof(last_payload));
    memcpy(last_payload, payload, payload_length);
    return send_result;
}

static pstvnc_h1_mpeg_start_contract_t valid_contract(void)
{
    pstvnc_h1_mpeg_start_contract_t contract;

    memset(&contract, 0, sizeof(contract));
    contract.draw_x = 176;
    contract.draw_y = 119;
    contract.draw_width = 352;
    contract.draw_height = 224;
    contract.suppression_rect.x = 164;
    contract.suppression_rect.y = 113;
    contract.suppression_rect.width = 376;
    contract.suppression_rect.height = 236;
    contract.generation = 9u;
    return contract;
}

static void reset_spy(void)
{
    send_calls = 0u;
    send_result = 1;
    last_runtime = NULL;
    last_kind = 0u;
    last_channel = 0u;
    last_payload_length = 0u;
    memset(last_payload, 0, sizeof(last_payload));
}

static void test_start_uses_existing_data_mpeg2_frame_path(void)
{
    unsigned char runtime_token = 0u;
    struct pstvnc_h1_transport_runtime *runtime =
        (struct pstvnc_h1_transport_runtime *)&runtime_token;
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();
    pstvnc_h1_mpeg_start_wire_message_t decoded;

    reset_spy();

    assert(pstvnc_h1_mpeg_start_transport_send(runtime, 77u, &contract));
    assert(send_calls == 1u);
    assert(last_runtime == runtime);
    assert(last_kind == PSTVNC_TRANSPORT_FRAME_DATA);
    assert(last_channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2);
    assert(last_payload_length == PSTVNC_H1_MPEG_START_WIRE_BYTES);
    assert(pstvnc_h1_mpeg_start_wire_decode(&decoded, last_payload));
    assert(decoded.session_id == 77u);
    assert(decoded.generation == 9u);
    assert(decoded.draw_x == 176);
    assert(decoded.draw_y == 119);
    assert(decoded.draw_width == 352);
    assert(decoded.draw_height == 224);
}

static void test_invalid_contract_never_reaches_transport_writer(void)
{
    unsigned char runtime_token = 0u;
    struct pstvnc_h1_transport_runtime *runtime =
        (struct pstvnc_h1_transport_runtime *)&runtime_token;
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();

    reset_spy();
    contract.generation = 0u;

    assert(!pstvnc_h1_mpeg_start_transport_send(runtime, 77u, &contract));
    assert(send_calls == 0u);
}

static void test_transport_failure_is_propagated(void)
{
    unsigned char runtime_token = 0u;
    struct pstvnc_h1_transport_runtime *runtime =
        (struct pstvnc_h1_transport_runtime *)&runtime_token;
    pstvnc_h1_mpeg_start_contract_t contract = valid_contract();

    reset_spy();
    send_result = 0;

    assert(!pstvnc_h1_mpeg_start_transport_send(runtime, 77u, &contract));
    assert(send_calls == 1u);
}

int main(void)
{
    test_start_uses_existing_data_mpeg2_frame_path();
    test_invalid_contract_never_reaches_transport_writer();
    test_transport_failure_is_propagated();

    puts("H1_MPEG_START_TRANSPORT_HOST_TEST=PASS");
    return 0;
}

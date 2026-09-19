/*
 * File synopsis:
 * Implements the proof-local fixed-width Wire establishment payload codecs.
 *
 * These helpers contain representation only. They do not own TCP, session
 * lifecycle, reconnect policy, rider readiness, or Application policy.
 */

#include "wire_establish_protocol.h"

#include "transport/protocol.h"

int pstvnc_wire_proof_hello_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_HELLO_BYTES],
    const pstvnc_wire_proof_hello_t *hello)
{
    if (output == NULL || hello == NULL)
        return 0;

    pstvnc_transport_write_be32(&output[0], hello->wire_version);
    pstvnc_transport_write_be32(&output[4], hello->product_version);
    return 1;
}

int pstvnc_wire_proof_hello_decode(
    pstvnc_wire_proof_hello_t *hello,
    const uint8_t *input,
    size_t input_size)
{
    if (hello == NULL || input == NULL ||
        input_size != PSTVNC_WIRE_PROOF_HELLO_BYTES)
        return 0;

    hello->wire_version = pstvnc_transport_read_be32(&input[0]);
    hello->product_version = pstvnc_transport_read_be32(&input[4]);
    return 1;
}

int pstvnc_wire_proof_accept_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_ACCEPT_BYTES],
    const pstvnc_wire_proof_accept_t *acceptance)
{
    if (output == NULL || acceptance == NULL || acceptance->session_id == 0u)
        return 0;

    pstvnc_transport_write_be32(&output[0], acceptance->session_id);
    return 1;
}

int pstvnc_wire_proof_accept_decode(
    pstvnc_wire_proof_accept_t *acceptance,
    const uint8_t *input,
    size_t input_size)
{
    if (acceptance == NULL || input == NULL ||
        input_size != PSTVNC_WIRE_PROOF_ACCEPT_BYTES)
        return 0;

    acceptance->session_id = pstvnc_transport_read_be32(&input[0]);
    return acceptance->session_id != 0u;
}

int pstvnc_wire_proof_not_accepted_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_NOT_ACCEPTED_BYTES],
    const pstvnc_wire_proof_not_accepted_t *rejection)
{
    if (output == NULL || rejection == NULL || rejection->reason == 0u)
        return 0;

    pstvnc_transport_write_be32(&output[0], rejection->reason);
    return 1;
}

int pstvnc_wire_proof_not_accepted_decode(
    pstvnc_wire_proof_not_accepted_t *rejection,
    const uint8_t *input,
    size_t input_size)
{
    if (rejection == NULL || input == NULL ||
        input_size != PSTVNC_WIRE_PROOF_NOT_ACCEPTED_BYTES)
        return 0;

    rejection->reason = pstvnc_transport_read_be32(&input[0]);
    return rejection->reason != 0u;
}

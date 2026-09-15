/*
 * File synopsis:
 * Implements the backend-independent PSTV fixed-header encoder/decoder used by
 * the shared transport owner. This file deliberately contains no socket,
 * queue, scheduler, RFB, media, or PS2-specific policy.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include "protocol.h"

uint32_t pstvnc_transport_read_be32(const uint8_t input[4])
{
    return ((uint32_t)input[0] << 24) |
           ((uint32_t)input[1] << 16) |
           ((uint32_t)input[2] << 8) |
           (uint32_t)input[3];
}

void pstvnc_transport_write_be32(uint8_t output[4], uint32_t value)
{
    output[0] = (uint8_t)(value >> 24);
    output[1] = (uint8_t)(value >> 16);
    output[2] = (uint8_t)(value >> 8);
    output[3] = (uint8_t)value;
}

int pstvnc_transport_header_encode(
    uint8_t output[PSTVNC_TRANSPORT_HEADER_SIZE],
    const pstvnc_transport_header_t *header)
{
    if (output == NULL || header == NULL ||
        header->version != PSTVNC_TRANSPORT_VERSION ||
        header->payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD)
        return 0;

    pstvnc_transport_write_be32(&output[0], PSTVNC_TRANSPORT_MAGIC);
    output[4] = header->version;
    output[5] = header->kind;
    output[6] = header->channel;
    output[7] = header->flags;
    pstvnc_transport_write_be32(&output[8], header->sequence);
    pstvnc_transport_write_be32(&output[12], header->payload_length);
    return 1;
}

int pstvnc_transport_header_decode(
    pstvnc_transport_header_t *header,
    const uint8_t input[PSTVNC_TRANSPORT_HEADER_SIZE])
{
    if (header == NULL || input == NULL)
        return 0;
    if (pstvnc_transport_read_be32(&input[0]) != PSTVNC_TRANSPORT_MAGIC)
        return 0;
    if (input[4] != PSTVNC_TRANSPORT_VERSION)
        return 0;

    header->version = input[4];
    header->kind = input[5];
    header->channel = input[6];
    header->flags = input[7];
    header->sequence = pstvnc_transport_read_be32(&input[8]);
    header->payload_length = pstvnc_transport_read_be32(&input[12]);

    return header->payload_length <= PSTVNC_TRANSPORT_MAX_PAYLOAD;
}

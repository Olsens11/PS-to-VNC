/*
 * File synopsis:
 * Defines the shared PSTV transport wire-header representation and stable
 * logical-channel identities. This file owns framing vocabulary only; it does
 * not own sockets, dispatch, queues, RFB parsing, media policy, or threading.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#ifndef PSTVNC_TRANSPORT_PROTOCOL_H
#define PSTVNC_TRANSPORT_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_TRANSPORT_MAGIC 0x50535456u
#define PSTVNC_TRANSPORT_VERSION 1u
#define PSTVNC_TRANSPORT_HEADER_SIZE 16u
#define PSTVNC_TRANSPORT_MAX_PAYLOAD 8192u
#define PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE 4u

typedef enum pstvnc_transport_frame_kind {
    PSTVNC_TRANSPORT_FRAME_HELLO = 1,
    PSTVNC_TRANSPORT_FRAME_CONFIG = 2,
    PSTVNC_TRANSPORT_FRAME_DATA = 3,
    PSTVNC_TRANSPORT_FRAME_CREDIT = 4,
    PSTVNC_TRANSPORT_FRAME_TELEMETRY = 5,
    PSTVNC_TRANSPORT_FRAME_HEARTBEAT = 6,
    PSTVNC_TRANSPORT_FRAME_ERROR = 7
} pstvnc_transport_frame_kind_t;

typedef enum pstvnc_transport_channel {
    PSTVNC_TRANSPORT_CHANNEL_CONTROL = 0,
    PSTVNC_TRANSPORT_CHANNEL_RFB = 1,
    PSTVNC_TRANSPORT_CHANNEL_AUDIO = 2,
    PSTVNC_TRANSPORT_CHANNEL_TELEMETRY = 3,
    PSTVNC_TRANSPORT_CHANNEL_MPEG2 = 4
} pstvnc_transport_channel_t;

typedef struct pstvnc_transport_header {
    uint8_t version;
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    uint32_t sequence;
    uint32_t payload_length;
} pstvnc_transport_header_t;

int pstvnc_transport_header_encode(
    uint8_t output[PSTVNC_TRANSPORT_HEADER_SIZE],
    const pstvnc_transport_header_t *header);
int pstvnc_transport_header_decode(
    pstvnc_transport_header_t *header,
    const uint8_t input[PSTVNC_TRANSPORT_HEADER_SIZE]);
uint32_t pstvnc_transport_read_be32(const uint8_t input[4]);
void pstvnc_transport_write_be32(uint8_t output[4], uint32_t value);

#endif

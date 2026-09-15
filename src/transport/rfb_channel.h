/*
 * File synopsis:
 * Defines Transport's logical RFB byte-stream storage independent of physical
 * PSTV framing and RFB protocol parsing. The channel owns committed inbound
 * bytes and producer-activity generation; synchronization and session runtime
 * policy are supplied by the higher transport runtime.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#ifndef PSTVNC_TRANSPORT_RFB_CHANNEL_H
#define PSTVNC_TRANSPORT_RFB_CHANNEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_rfb_channel {
    uint8_t *storage;
    size_t capacity;
    size_t read_offset;
    size_t byte_count;
    uint32_t activity_generation;
} pstvnc_transport_rfb_channel_t;

/* Binds caller-owned session storage and starts an empty logical stream. */
int pstvnc_transport_rfb_channel_initialize(
    pstvnc_transport_rfb_channel_t *channel,
    void *storage,
    size_t capacity);

/* Commits one complete received channel-1 DATA payload atomically. */
int pstvnc_transport_rfb_channel_commit(
    pstvnc_transport_rfb_channel_t *channel,
    const void *payload,
    size_t payload_length);

/* Consumes exactly count committed bytes, or leaves the stream unchanged. */
int pstvnc_transport_rfb_channel_read_exact(
    pstvnc_transport_rfb_channel_t *channel,
    void *buffer,
    size_t count);

size_t pstvnc_transport_rfb_channel_available(
    const pstvnc_transport_rfb_channel_t *channel);

uint32_t pstvnc_transport_rfb_channel_activity_generation(
    const pstvnc_transport_rfb_channel_t *channel);

#endif

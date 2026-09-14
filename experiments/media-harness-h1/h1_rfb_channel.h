/*
 * File synopsis:
 * Defines the experiment-owned logical RFB channel mechanics that sit between
 * H1's sole PSTV receiver/sender and the unchanged clean RFB session.
 *
 * This module owns byte-queue accounting, immediate consumed-byte credit
 * accounting, and fragmentation of outbound logical RFB bytes. It deliberately
 * owns no socket, thread, semaphore, CONFIG activation, or RFB protocol parsing.
 * Queue storage/capacity are caller-owned so H1 CONFIG can tune them per session.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_RFB_CHANNEL_H
#define PSTVNC_MEDIA_HARNESS_H1_RFB_CHANNEL_H

#include "transport_queue.h"

#include <stddef.h>
#include <stdint.h>

/* Evidence-based default only; CONFIG v4 may request another capacity. */
#define PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES 32768u

typedef int (*pstvnc_h1_rfb_fragment_sender_t)(
    void *context,
    const void *payload,
    size_t payload_length);

typedef struct pstvnc_h1_rfb_channel_stats {
    uint32_t data_frames_received;
    uint32_t bytes_enqueued;
    uint32_t bytes_consumed;

    /*
     * Bytes abandoned only after the ordered finite-session COMMIT fence.
     * These bytes were never consumed by the RFB parser and therefore must
     * remain distinct from bytes_consumed.
     */
    uint32_t bytes_discarded_quiesce;

    uint32_t credit_bytes_pending;
    uint32_t credit_frames_sent;
    uint32_t credit_bytes_sent;
    uint32_t logical_write_calls;
    uint32_t logical_bytes_written;
    uint32_t data_frames_sent;
} pstvnc_h1_rfb_channel_stats_t;

typedef struct pstvnc_h1_rfb_channel {
    pstvnc_transport_queue_t queue;
    pstvnc_h1_rfb_channel_stats_t stats;
    int initialized;
} pstvnc_h1_rfb_channel_t;

int pstvnc_h1_rfb_channel_init(
    pstvnc_h1_rfb_channel_t *channel,
    uint8_t *storage,
    size_t storage_bytes);

int pstvnc_h1_rfb_channel_accept_data(
    pstvnc_h1_rfb_channel_t *channel,
    const void *payload,
    size_t payload_length);

size_t pstvnc_h1_rfb_channel_queue_size(
    const pstvnc_h1_rfb_channel_t *channel);

size_t pstvnc_h1_rfb_channel_queue_high_water(
    const pstvnc_h1_rfb_channel_t *channel);

int pstvnc_h1_rfb_channel_poll(
    const pstvnc_h1_rfb_channel_t *channel);

size_t pstvnc_h1_rfb_channel_read_available(
    pstvnc_h1_rfb_channel_t *channel,
    void *buffer,
    size_t maximum_count);

/*
 * Discard exactly the residual queue proven by an ordered terminal quiesce
 * fence. This operation does not count bytes as parser-consumed and does not
 * create credit pending for a producer that has already committed shutdown.
 */
int pstvnc_h1_rfb_channel_discard_quiesce_residual(
    pstvnc_h1_rfb_channel_t *channel,
    size_t expected_queue_bytes,
    uint32_t *bytes_discarded);

uint32_t pstvnc_h1_rfb_channel_take_credit(
    pstvnc_h1_rfb_channel_t *channel);

int pstvnc_h1_rfb_channel_write_logical(
    pstvnc_h1_rfb_channel_t *channel,
    const void *buffer,
    size_t count,
    size_t maximum_payload,
    pstvnc_h1_rfb_fragment_sender_t sender,
    void *sender_context);

#endif

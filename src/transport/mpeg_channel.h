/*
 * File synopsis:
 * Defines Transport's session-local logical MPEG2 byte queue. Complete non-empty
 * channel-4 DATA is buffered here; finite-producer completion is a separate
 * Transport-owned fact. Exact-run finalization may atomically discard residual
 * bytes and reset producer/offset state only through the runtime-owned
 * synchronization boundary.
 *
 * This storage owner does not invent a wire EOF marker, observe local decoder
 * stop, own physical receive, or perform decoder/DMA policy. Runtime
 * synchronization, flow control, activity wakeups, and terminal state remain
 * outside this ring.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_TRANSPORT_MPEG_CHANNEL_H
#define PSTVNC_TRANSPORT_MPEG_CHANNEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_mpeg_channel {
    uint8_t *storage;
    size_t capacity;
    size_t read_offset;
    size_t byte_count;
    int producer_done;
} pstvnc_transport_mpeg_channel_t;

int pstvnc_transport_mpeg_channel_initialize(
    pstvnc_transport_mpeg_channel_t *channel,
    uint8_t *storage,
    size_t capacity);
int pstvnc_transport_mpeg_channel_commit_data(
    pstvnc_transport_mpeg_channel_t *channel,
    const uint8_t *payload,
    size_t payload_length);
int pstvnc_transport_mpeg_channel_mark_producer_done(
    pstvnc_transport_mpeg_channel_t *channel);
size_t pstvnc_transport_mpeg_channel_read_available(
    pstvnc_transport_mpeg_channel_t *channel,
    uint8_t *destination,
    size_t maximum_count);
size_t pstvnc_transport_mpeg_channel_available(
    const pstvnc_transport_mpeg_channel_t *channel);
int pstvnc_transport_mpeg_channel_producer_done(
    const pstvnc_transport_mpeg_channel_t *channel);

/*
 * Run-finalization helpers. discard_all() returns the exact residual byte count
 * and resets ring offsets; it does not claim decoder consumption or touch the
 * producer fact. reset_run_state() is legal only after the queue is empty.
 */
size_t pstvnc_transport_mpeg_channel_discard_all(
    pstvnc_transport_mpeg_channel_t *channel);
int pstvnc_transport_mpeg_channel_reset_run_state(
    pstvnc_transport_mpeg_channel_t *channel);

#endif

/*
 * File synopsis:
 * Defines Transport's session-local logical AUDIO byte queue. Complete non-empty
 * channel-2 DATA is buffered here; one zero-length DATA marker retires the
 * finite producer while already queued bytes remain drainable.
 *
 * Flow-control policy, physical receive ownership, consumer waiting, and PCM
 * playback remain outside this byte-storage owner.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_TRANSPORT_AUDIO_CHANNEL_H
#define PSTVNC_TRANSPORT_AUDIO_CHANNEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_audio_channel {
    uint8_t *storage;
    size_t capacity;
    size_t read_offset;
    size_t byte_count;
    int producer_done;
} pstvnc_transport_audio_channel_t;

int pstvnc_transport_audio_channel_initialize(
    pstvnc_transport_audio_channel_t *channel,
    uint8_t *storage,
    size_t capacity);
int pstvnc_transport_audio_channel_commit_data(
    pstvnc_transport_audio_channel_t *channel,
    const uint8_t *payload,
    size_t payload_length);
int pstvnc_transport_audio_channel_mark_producer_done(
    pstvnc_transport_audio_channel_t *channel);
size_t pstvnc_transport_audio_channel_read_available(
    pstvnc_transport_audio_channel_t *channel,
    uint8_t *destination,
    size_t maximum_count);
size_t pstvnc_transport_audio_channel_available(
    const pstvnc_transport_audio_channel_t *channel);
int pstvnc_transport_audio_channel_producer_done(
    const pstvnc_transport_audio_channel_t *channel);

#endif

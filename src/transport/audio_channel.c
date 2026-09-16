/*
 * File synopsis:
 * Implements Transport's bounded logical AUDIO ring and one-shot finite-producer
 * marker. Storage mutation is deliberately synchronization-agnostic: the
 * Transport runtime serializes producer/consumer access with its AUDIO queue
 * semaphore and owns credit/wakeup semantics.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "audio_channel.h"

#include <string.h>

int pstvnc_transport_audio_channel_initialize(
    pstvnc_transport_audio_channel_t *channel,
    uint8_t *storage,
    size_t capacity)
{
    if (channel == NULL || storage == NULL || capacity == 0u)
        return -1;

    memset(channel, 0, sizeof(*channel));
    channel->storage = storage;
    channel->capacity = capacity;
    return 0;
}

int pstvnc_transport_audio_channel_commit_data(
    pstvnc_transport_audio_channel_t *channel,
    const uint8_t *payload,
    size_t payload_length)
{
    size_t write_offset;
    size_t first_count;

    if (channel == NULL || payload == NULL || payload_length == 0u ||
        channel->producer_done || payload_length > channel->capacity ||
        payload_length > channel->capacity - channel->byte_count)
        return -1;

    write_offset = (channel->read_offset + channel->byte_count) % channel->capacity;
    first_count = channel->capacity - write_offset;
    if (first_count > payload_length)
        first_count = payload_length;

    memcpy(channel->storage + write_offset, payload, first_count);
    if (first_count < payload_length) {
        memcpy(
            channel->storage,
            payload + first_count,
            payload_length - first_count);
    }

    channel->byte_count += payload_length;
    return 0;
}

int pstvnc_transport_audio_channel_mark_producer_done(
    pstvnc_transport_audio_channel_t *channel)
{
    if (channel == NULL || channel->producer_done)
        return -1;

    channel->producer_done = 1;
    return 0;
}

size_t pstvnc_transport_audio_channel_read_available(
    pstvnc_transport_audio_channel_t *channel,
    uint8_t *destination,
    size_t maximum_count)
{
    size_t count;
    size_t first_count;

    if (channel == NULL || (destination == NULL && maximum_count != 0u))
        return 0u;

    count = channel->byte_count;
    if (count > maximum_count)
        count = maximum_count;
    if (count == 0u)
        return 0u;

    first_count = channel->capacity - channel->read_offset;
    if (first_count > count)
        first_count = count;

    memcpy(destination, channel->storage + channel->read_offset, first_count);
    if (first_count < count) {
        memcpy(
            destination + first_count,
            channel->storage,
            count - first_count);
    }

    channel->read_offset = (channel->read_offset + count) % channel->capacity;
    channel->byte_count -= count;
    return count;
}

size_t pstvnc_transport_audio_channel_available(
    const pstvnc_transport_audio_channel_t *channel)
{
    if (channel == NULL)
        return 0u;

    return channel->byte_count;
}

int pstvnc_transport_audio_channel_producer_done(
    const pstvnc_transport_audio_channel_t *channel)
{
    return channel != NULL && channel->producer_done != 0;
}

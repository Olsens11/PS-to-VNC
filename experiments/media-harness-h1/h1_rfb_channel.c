/*
 * File synopsis:
 * Implements queue, credit, and outbound-fragment mechanics for H1 logical RFB.
 *
 * No physical socket I/O occurs here. The future H1 runtime integration will
 * hold the appropriate queue/send semaphore around these operations and will
 * translate each outbound fragment into one channel-1 PSTV DATA frame.
 *
 * Context: RFB_MUX_INTEGRATION_PREP.md, P2.
 */

#include "h1_rfb_channel.h"

#include <string.h>

static int add_u32(uint32_t *value, size_t amount)
{
    if (amount > 0xffffffffu || *value > 0xffffffffu - (uint32_t)amount)
        return 0;
    *value += (uint32_t)amount;
    return 1;
}

int pstvnc_h1_rfb_channel_init(
    pstvnc_h1_rfb_channel_t *channel,
    uint8_t *storage,
    size_t storage_bytes)
{
    if (channel == NULL || storage == NULL ||
        storage_bytes != PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES)
        return 0;

    memset(channel, 0, sizeof(*channel));
    if (!pstvnc_transport_queue_init(&channel->queue, storage, storage_bytes))
        return 0;

    channel->initialized = 1;
    return 1;
}

int pstvnc_h1_rfb_channel_accept_data(
    pstvnc_h1_rfb_channel_t *channel,
    const void *payload,
    size_t payload_length)
{
    if (channel == NULL || !channel->initialized || payload == NULL ||
        payload_length == 0u || payload_length > 0xffffffffu)
        return 0;

    if (channel->stats.bytes_enqueued >
        0xffffffffu - (uint32_t)payload_length)
        return 0;

    if (!pstvnc_transport_queue_write(
            &channel->queue, payload, payload_length))
        return 0;

    channel->stats.data_frames_received++;
    channel->stats.bytes_enqueued += (uint32_t)payload_length;
    return 1;
}

size_t pstvnc_h1_rfb_channel_queue_size(
    const pstvnc_h1_rfb_channel_t *channel)
{
    if (channel == NULL || !channel->initialized)
        return 0u;
    return pstvnc_transport_queue_size(&channel->queue);
}

size_t pstvnc_h1_rfb_channel_queue_high_water(
    const pstvnc_h1_rfb_channel_t *channel)
{
    if (channel == NULL || !channel->initialized)
        return 0u;
    return pstvnc_transport_queue_high_water(&channel->queue);
}

int pstvnc_h1_rfb_channel_poll(
    const pstvnc_h1_rfb_channel_t *channel)
{
    if (channel == NULL || !channel->initialized)
        return -1;
    return pstvnc_transport_queue_size(&channel->queue) != 0u ? 1 : 0;
}

size_t pstvnc_h1_rfb_channel_read_available(
    pstvnc_h1_rfb_channel_t *channel,
    void *buffer,
    size_t maximum_count)
{
    size_t available;
    size_t take;

    if (channel == NULL || !channel->initialized ||
        (buffer == NULL && maximum_count != 0u))
        return 0u;

    available = pstvnc_transport_queue_size(&channel->queue);
    take = available < maximum_count ? available : maximum_count;
    if (take == 0u)
        return 0u;

    if (!pstvnc_transport_queue_read(&channel->queue, buffer, take))
        return 0u;

    if (!add_u32(&channel->stats.bytes_consumed, take) ||
        !add_u32(&channel->stats.credit_bytes_pending, take))
        return 0u;

    return take;
}

uint32_t pstvnc_h1_rfb_channel_take_credit(
    pstvnc_h1_rfb_channel_t *channel)
{
    uint32_t amount;

    if (channel == NULL || !channel->initialized)
        return 0u;

    amount = channel->stats.credit_bytes_pending;
    channel->stats.credit_bytes_pending = 0u;
    return amount;
}

int pstvnc_h1_rfb_channel_write_logical(
    pstvnc_h1_rfb_channel_t *channel,
    const void *buffer,
    size_t count,
    size_t maximum_payload,
    pstvnc_h1_rfb_fragment_sender_t sender,
    void *sender_context)
{
    const uint8_t *source = (const uint8_t *)buffer;
    size_t done = 0u;

    if (channel == NULL || !channel->initialized ||
        (buffer == NULL && count != 0u) || maximum_payload == 0u ||
        sender == NULL || count > 0xffffffffu)
        return 0;

    while (done < count) {
        size_t remaining = count - done;
        size_t fragment = remaining < maximum_payload ? remaining : maximum_payload;

        if (!sender(sender_context, source + done, fragment))
            return 0;

        if (!add_u32(&channel->stats.logical_bytes_written, fragment))
            return 0;
        channel->stats.data_frames_sent++;
        done += fragment;
    }

    channel->stats.logical_write_calls++;
    return 1;
}

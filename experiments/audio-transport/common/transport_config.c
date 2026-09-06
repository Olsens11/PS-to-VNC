/*
 * File synopsis:
 * Decodes and validates configurable Audio Transport EXP2's complete scalar
 * operating profile.
 *
 * Queue sizes are not capped here. Allocation success on the PS2 is the
 * experimental authority for whether a requested capacity actually fits.
 */

#include "transport_config.h"
#include "transport_protocol.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define CONFIG_HEADER_BYTES 8u
#define CONFIG_ENTRY_BYTES  8u

#define CONFIG_REQUIRED_MASK \
    ((1u << PSTVNC_TRANSPORT_CONFIG_FIELD_COUNT) - 1u)

#define CONFIG_SIGNED_INT_MAX 0x7fffffffu

static int config_set_field(
    pstvnc_transport_config_t *config,
    uint32_t field_id,
    uint32_t value)
{
    switch (field_id) {
        case PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY:
            config->rfb_queue_capacity = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_QUEUE_CAPACITY:
            config->audio_queue_capacity = value;
            break;

        case PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES:
            config->rfb_credit_batch_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_CREDIT_BATCH_BYTES:
            config->audio_credit_batch_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY:
            config->rfb_credit_flush_on_empty = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_CREDIT_FLUSH_ON_EMPTY:
            config->audio_credit_flush_on_empty = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_START_MODE:
            config->audio_start_mode = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_START_TARGET_BYTES:
            config->audio_start_target_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_START_DELAY_US:
            config->audio_start_delay_us = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_CHUNK_BYTES:
            config->audio_chunk_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_IDLE_DELAY_US:
            config->audio_idle_delay_us = value;
            break;

        case PSTVNC_CONFIG_FIELD_RFB_EMPTY_DELAY_US:
            config->rfb_empty_delay_us = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_THREAD_PRIORITY:
            config->audio_thread_priority = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_THREAD_STACK_SIZE:
            config->audio_thread_stack_size = value;
            break;

        case PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY:
            config->receiver_thread_priority = value;
            break;

        case PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE:
            config->receiver_thread_stack_size = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_RATE:
            config->audio_rate = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS:
            config->audio_channels = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_BITS:
            config->audio_bits = value;
            break;

        case PSTVNC_CONFIG_FIELD_AUDIO_VOLUME:
            config->audio_volume = value;
            break;

        case PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD:
            config->max_data_payload = value;
            break;

        case PSTVNC_CONFIG_FIELD_SOCKET_RECEIVE_BUFFER_BYTES:
            config->socket_receive_buffer_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_SOCKET_SEND_BUFFER_BYTES:
            config->socket_send_buffer_bytes = value;
            break;

        case PSTVNC_CONFIG_FIELD_QUEUE_ALLOCATION_ORDER:
            config->queue_allocation_order = value;
            break;

        default:
            return 0;
    }

    return 1;
}

uint32_t pstvnc_transport_config_audio_frame_bytes(
    const pstvnc_transport_config_t *config)
{
    uint32_t sample_bytes;

    if (config == NULL)
        return 0;

    if (config->audio_bits != 8u &&
        config->audio_bits != 16u)
        return 0;

    if (config->audio_channels != 1u &&
        config->audio_channels != 2u)
        return 0;

    sample_bytes = config->audio_bits / 8u;

    return sample_bytes * config->audio_channels;
}

int pstvnc_transport_config_decode(
    pstvnc_transport_config_t *config,
    const uint8_t *payload,
    size_t payload_length)
{
    uint32_t seen = 0;
    size_t offset;

    if (config == NULL ||
        payload == NULL ||
        payload_length < CONFIG_HEADER_BYTES ||
        ((payload_length - CONFIG_HEADER_BYTES) %
         CONFIG_ENTRY_BYTES) != 0)
        return 0;

    memset(config, 0, sizeof(*config));

    config->version =
        pstvnc_transport_read_be32(&payload[0]);

    config->profile_id =
        pstvnc_transport_read_be32(&payload[4]);

    if (config->version !=
        PSTVNC_TRANSPORT_CONFIG_VERSION)
        return 0;

    for (offset = CONFIG_HEADER_BYTES;
         offset < payload_length;
         offset += CONFIG_ENTRY_BYTES) {

        uint32_t field_id;
        uint32_t value;
        uint32_t bit;

        field_id =
            pstvnc_transport_read_be32(
                &payload[offset]);

        value =
            pstvnc_transport_read_be32(
                &payload[offset + 4u]);

        if (field_id == 0u ||
            field_id >
                PSTVNC_TRANSPORT_CONFIG_FIELD_COUNT)
            return 0;

        bit = 1u << (field_id - 1u);

        if ((seen & bit) != 0u)
            return 0;

        seen |= bit;

        if (!config_set_field(
                config,
                field_id,
                value))
            return 0;
    }

    return seen == CONFIG_REQUIRED_MASK;
}

int pstvnc_transport_config_validate(
    const pstvnc_transport_config_t *config)
{
    uint32_t frame_bytes;

    if (config == NULL ||
        config->version !=
            PSTVNC_TRANSPORT_CONFIG_VERSION)
        return 0;

    frame_bytes =
        pstvnc_transport_config_audio_frame_bytes(
            config);

    if (frame_bytes == 0u)
        return 0;

    /*
     * No guessed queue-size maxima.
     *
     * The only queue-size constraints here are internal consistency. The PS2
     * allocator decides whether the requested capacities actually fit.
     */
    if (config->rfb_queue_capacity == 0u ||
        config->audio_queue_capacity == 0u)
        return 0;

    if (config->max_data_payload == 0u ||
        config->max_data_payload >
            PSTVNC_TRANSPORT_MAX_PAYLOAD)
        return 0;

    /*
     * A complete legal DATA frame must fit in an empty destination queue.
     */
    if (config->rfb_queue_capacity <
            config->max_data_payload ||
        config->audio_queue_capacity <
            config->max_data_payload)
        return 0;

    if ((config->audio_queue_capacity %
         frame_bytes) != 0u)
        return 0;

    if (config->rfb_credit_batch_bytes == 0u ||
        config->rfb_credit_batch_bytes >
            config->rfb_queue_capacity)
        return 0;

    if (config->audio_credit_batch_bytes == 0u ||
        config->audio_credit_batch_bytes >
            config->audio_queue_capacity ||
        (config->audio_credit_batch_bytes %
         frame_bytes) != 0u)
        return 0;

    if (config->rfb_credit_flush_on_empty > 1u ||
        config->audio_credit_flush_on_empty > 1u)
        return 0;

    if (config->audio_start_mode >
        PSTVNC_TRANSPORT_AUDIO_START_DELAY)
        return 0;

    if (config->audio_start_target_bytes >
            config->audio_queue_capacity ||
        (config->audio_start_target_bytes %
         frame_bytes) != 0u)
        return 0;

    if (config->audio_start_mode ==
            PSTVNC_TRANSPORT_AUDIO_START_TARGET &&
        config->audio_start_target_bytes == 0u)
        return 0;

    if (config->audio_start_mode ==
            PSTVNC_TRANSPORT_AUDIO_START_DELAY &&
        config->audio_start_delay_us == 0u)
        return 0;

    if (config->audio_chunk_bytes == 0u ||
        config->audio_chunk_bytes >
            config->audio_queue_capacity ||
        config->audio_chunk_bytes >
            CONFIG_SIGNED_INT_MAX ||
        (config->audio_chunk_bytes %
         frame_bytes) != 0u)
        return 0;

    /*
     * EE thread priorities are represented in the kernel's normal 1..127
     * priority vocabulary. This is an API domain, not an experimental buffer
     * ceiling.
     */
    if (config->audio_thread_priority == 0u ||
        config->audio_thread_priority > 127u ||
        config->receiver_thread_priority == 0u ||
        config->receiver_thread_priority > 127u)
        return 0;

    /*
     * Stack sizes are runtime allocated. Require only a minimally coherent,
     * 16-byte-sized EE stack and an int-representable CreateThread size.
     */
    if (config->audio_thread_stack_size < 256u ||
        config->audio_thread_stack_size >
            CONFIG_SIGNED_INT_MAX ||
        (config->audio_thread_stack_size % 16u) != 0u)
        return 0;

    if (config->receiver_thread_stack_size < 256u ||
        config->receiver_thread_stack_size >
            CONFIG_SIGNED_INT_MAX ||
        (config->receiver_thread_stack_size % 16u) != 0u)
        return 0;

    if (config->audio_rate == 0u ||
        config->audio_rate >
            CONFIG_SIGNED_INT_MAX)
        return 0;

    if (config->audio_volume > 100u)
        return 0;

    /*
     * SO_RCVBUF/SO_SNDBUF consume int values in the PS2IP socket API.
     * Zero means leave the stack default unchanged.
     */
    if (config->socket_receive_buffer_bytes >
            CONFIG_SIGNED_INT_MAX ||
        config->socket_send_buffer_bytes >
            CONFIG_SIGNED_INT_MAX)
        return 0;

    if (config->queue_allocation_order >
        PSTVNC_TRANSPORT_ALLOCATE_AUDIO_FIRST)
        return 0;

    return 1;
}

uint32_t pstvnc_transport_config_digest(
    const uint8_t *payload,
    size_t payload_length)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (payload == NULL)
        return 0u;

    for (index = 0;
         index < payload_length;
         index++) {

        hash ^= (uint32_t)payload[index];
        hash *= 16777619u;
    }

    return hash;
}

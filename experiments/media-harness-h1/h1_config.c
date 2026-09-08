/*
 * File synopsis:
 * Decodes and structurally validates H1's complete muxed audio/video profile.
 *
 * Experimental capacities and latency values are not clamped to known-good
 * ranges. Allocation success and hardware behavior are the experimental
 * authority. Validation is limited to wire completeness, API representation,
 * queue/credit consistency, format alignment, and mechanisms the current H1
 * implementation can actually represent.
 */

#include "h1_config.h"
#include "transport_protocol.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define H1_CONFIG_HEADER_BYTES 8u
#define H1_CONFIG_ENTRY_BYTES 8u
#define H1_CONFIG_SIGNED_INT_MAX 0x7fffffffu
#define H1_REQUIRED_MASK \
    ((UINT64_C(1) << PSTVNC_H1_CONFIG_FIELD_COUNT) - UINT64_C(1))

static int h1_set_field(
    pstvnc_h1_config_t *config,
    uint32_t field_id,
    uint32_t value)
{
    switch (field_id) {
        case PSTVNC_H1_FIELD_SESSION_ID:
            config->session_id = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_MODE:
            config->audio_mode = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_MODE:
            config->video_mode = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_QUEUE_CAPACITY:
            config->audio_queue_capacity = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_QUEUE_CAPACITY:
            config->mpeg_queue_capacity = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_CREDIT_BATCH_BYTES:
            config->audio_credit_batch_bytes = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_CREDIT_BATCH_BYTES:
            config->mpeg_credit_batch_bytes = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_CREDIT_FLUSH_ON_EMPTY:
            config->audio_credit_flush_on_empty = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_CREDIT_FLUSH_ON_EMPTY:
            config->mpeg_credit_flush_on_empty = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_CREDIT_RETURN_ENABLED:
            config->audio_credit_return_enabled = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_CREDIT_RETURN_ENABLED:
            config->mpeg_credit_return_enabled = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_INITIAL_CREDIT_BYTES:
            config->audio_initial_credit_bytes = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_INITIAL_CREDIT_BYTES:
            config->mpeg_initial_credit_bytes = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_START_MODE:
            config->audio_start_mode = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_START_TARGET_BYTES:
            config->audio_start_target_bytes = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_START_DELAY_US:
            config->audio_start_delay_us = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_CHUNK_BYTES:
            config->audio_chunk_bytes = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_IDLE_DELAY_US:
            config->audio_idle_delay_us = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_THREAD_PRIORITY:
            config->audio_thread_priority = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_THREAD_STACK_SIZE:
            config->audio_thread_stack_size = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_RATE:
            config->audio_rate = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_CHANNELS:
            config->audio_channels = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_BITS:
            config->audio_bits = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_VOLUME:
            config->audio_volume = value;
            break;
        case PSTVNC_H1_FIELD_AUDIO_PRESENTATION_OFFSET_US:
            config->audio_presentation_offset_us = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_START_TARGET_BYTES:
            config->mpeg_start_target_bytes = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_PREFILL_WAIT_US:
            config->mpeg_prefill_wait_us = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_PREFILL_MAX_LOOPS:
            config->mpeg_prefill_max_loops = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_EMPTY_DELAY_US:
            config->mpeg_empty_delay_us = value;
            break;
        case PSTVNC_H1_FIELD_MPEG_FEED_BYTES:
            config->mpeg_feed_bytes = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_FPS_NUM:
            config->video_fps_num = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_FPS_DEN:
            config->video_fps_den = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_SCHEDULER_MODE:
            config->video_scheduler_mode = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_PRESENTATION_OFFSET_US:
            config->video_presentation_offset_us = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_PIXEL_MODE:
            config->video_pixel_mode = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_MAX_WIDTH:
            config->video_max_width = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_MAX_HEIGHT:
            config->video_max_height = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DRAW_WIDTH:
            config->video_draw_width = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DRAW_HEIGHT:
            config->video_draw_height = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DRAW_X:
            config->video_draw_x = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DRAW_Y:
            config->video_draw_y = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_STAGE_MARKERS:
            config->video_stage_markers = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_STAGE_HOLD_VSYNCS:
            config->video_stage_hold_vsyncs = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_IPU_RESET_EACH_SESSION:
            config->video_ipu_reset_each_session = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DROP_ENABLED:
            config->video_drop_enabled = value;
            break;
        case PSTVNC_H1_FIELD_VIDEO_DROP_THRESHOLD_MILLIFRAMES:
            config->video_drop_threshold_milliframes = value;
            break;
        case PSTVNC_H1_FIELD_RECEIVER_THREAD_PRIORITY:
            config->receiver_thread_priority = value;
            break;
        case PSTVNC_H1_FIELD_RECEIVER_THREAD_STACK_SIZE:
            config->receiver_thread_stack_size = value;
            break;
        case PSTVNC_H1_FIELD_MAX_DATA_PAYLOAD:
            config->max_data_payload = value;
            break;
        case PSTVNC_H1_FIELD_SOCKET_RECEIVE_BUFFER_BYTES:
            config->socket_receive_buffer_bytes = value;
            break;
        case PSTVNC_H1_FIELD_SOCKET_SEND_BUFFER_BYTES:
            config->socket_send_buffer_bytes = value;
            break;
        case PSTVNC_H1_FIELD_QUEUE_ALLOCATION_ORDER:
            config->queue_allocation_order = value;
            break;
        case PSTVNC_H1_FIELD_MEDIA_EPOCH_LEAD_US:
            config->media_epoch_lead_us = value;
            break;
        default:
            return 0;
    }

    return 1;
}

uint32_t pstvnc_h1_config_audio_frame_bytes(
    const pstvnc_h1_config_t *config)
{
    uint32_t sample_bytes;

    if (config == NULL)
        return 0u;

    if (config->audio_bits != 8u &&
        config->audio_bits != 16u)
        return 0u;

    if (config->audio_channels != 1u &&
        config->audio_channels != 2u)
        return 0u;

    sample_bytes = config->audio_bits / 8u;
    return sample_bytes * config->audio_channels;
}

static int32_t h1_signed_bits(uint32_t raw)
{
    int32_t value;

    memcpy(&value, &raw, sizeof(value));
    return value;
}

int32_t pstvnc_h1_config_audio_offset_us(
    const pstvnc_h1_config_t *config)
{
    if (config == NULL)
        return 0;

    return h1_signed_bits(
        config->audio_presentation_offset_us);
}

int32_t pstvnc_h1_config_video_offset_us(
    const pstvnc_h1_config_t *config)
{
    if (config == NULL)
        return 0;

    return h1_signed_bits(
        config->video_presentation_offset_us);
}

int pstvnc_h1_config_decode(
    pstvnc_h1_config_t *config,
    const uint8_t *payload,
    size_t payload_length)
{
    uint64_t seen = UINT64_C(0);
    size_t offset;

    if (config == NULL ||
        payload == NULL ||
        payload_length < H1_CONFIG_HEADER_BYTES ||
        ((payload_length - H1_CONFIG_HEADER_BYTES) %
         H1_CONFIG_ENTRY_BYTES) != 0u)
        return 0;

    memset(config, 0, sizeof(*config));

    config->version =
        pstvnc_transport_read_be32(&payload[0]);
    config->profile_id =
        pstvnc_transport_read_be32(&payload[4]);

    if (config->version != PSTVNC_H1_CONFIG_VERSION)
        return 0;

    for (offset = H1_CONFIG_HEADER_BYTES;
         offset < payload_length;
         offset += H1_CONFIG_ENTRY_BYTES) {

        uint32_t field_id =
            pstvnc_transport_read_be32(&payload[offset]);
        uint32_t value =
            pstvnc_transport_read_be32(&payload[offset + 4u]);
        uint64_t bit;

        if (field_id == 0u ||
            field_id > PSTVNC_H1_CONFIG_FIELD_COUNT)
            return 0;

        bit = UINT64_C(1) << (field_id - 1u);

        if ((seen & bit) != 0u)
            return 0;

        seen |= bit;

        if (!h1_set_field(config, field_id, value))
            return 0;
    }

    return seen == H1_REQUIRED_MASK;
}

static int h1_validate_thread(
    uint32_t priority,
    uint32_t stack_size)
{
    if (priority == 0u || priority > 127u)
        return 0;

    if (stack_size < 256u ||
        stack_size > H1_CONFIG_SIGNED_INT_MAX ||
        (stack_size % 16u) != 0u)
        return 0;

    return 1;
}

int pstvnc_h1_config_validate(
    const pstvnc_h1_config_t *config)
{
    uint32_t frame_bytes = 0u;

    if (config == NULL ||
        config->version != PSTVNC_H1_CONFIG_VERSION)
        return 0;

    /* This H1 implementation currently owns PCM and MPEG-2 ES only. */
    if (config->audio_mode > PSTVNC_H1_AUDIO_PCM ||
        config->video_mode > PSTVNC_H1_VIDEO_MPEG2_ES)
        return 0;

    if (config->max_data_payload == 0u ||
        config->max_data_payload > PSTVNC_TRANSPORT_MAX_PAYLOAD)
        return 0;

    if (config->audio_credit_flush_on_empty > 1u ||
        config->mpeg_credit_flush_on_empty > 1u ||
        config->audio_credit_return_enabled > 1u ||
        config->mpeg_credit_return_enabled > 1u)
        return 0;

    if (config->audio_mode == PSTVNC_H1_AUDIO_PCM) {
        frame_bytes = pstvnc_h1_config_audio_frame_bytes(config);

        if (frame_bytes == 0u)
            return 0;

        if (config->audio_queue_capacity <
            config->max_data_payload)
            return 0;

        if ((config->audio_queue_capacity % frame_bytes) != 0u)
            return 0;

        if (config->audio_initial_credit_bytes >
            config->audio_queue_capacity ||
            (config->audio_initial_credit_bytes % frame_bytes) != 0u)
            return 0;

        if (config->audio_credit_return_enabled != 0u &&
            (config->audio_credit_batch_bytes == 0u ||
             config->audio_credit_batch_bytes >
                config->audio_queue_capacity ||
             (config->audio_credit_batch_bytes % frame_bytes) != 0u))
            return 0;

        if (config->audio_start_mode >
            PSTVNC_H1_AUDIO_START_DELAY)
            return 0;

        if (config->audio_start_target_bytes >
            config->audio_queue_capacity ||
            (config->audio_start_target_bytes % frame_bytes) != 0u)
            return 0;

        if (config->audio_start_mode ==
                PSTVNC_H1_AUDIO_START_TARGET &&
            config->audio_start_target_bytes == 0u)
            return 0;

        if (config->audio_chunk_bytes == 0u ||
            config->audio_chunk_bytes >
                config->audio_queue_capacity ||
            config->audio_chunk_bytes >
                H1_CONFIG_SIGNED_INT_MAX ||
            (config->audio_chunk_bytes % frame_bytes) != 0u)
            return 0;

        if (!h1_validate_thread(
                config->audio_thread_priority,
                config->audio_thread_stack_size))
            return 0;

        if (config->audio_rate == 0u ||
            config->audio_rate > H1_CONFIG_SIGNED_INT_MAX)
            return 0;

        if (config->audio_volume > 100u)
            return 0;
    } else {
        if (config->audio_queue_capacity != 0u ||
            config->audio_initial_credit_bytes != 0u)
            return 0;
    }

    if (config->video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        if (config->mpeg_queue_capacity <
            config->max_data_payload)
            return 0;

        if (config->mpeg_initial_credit_bytes >
            config->mpeg_queue_capacity)
            return 0;

        if (config->mpeg_credit_return_enabled != 0u &&
            (config->mpeg_credit_batch_bytes == 0u ||
             config->mpeg_credit_batch_bytes >
                config->mpeg_queue_capacity))
            return 0;

        if (config->mpeg_start_target_bytes >
            config->mpeg_queue_capacity)
            return 0;

        if (config->mpeg_feed_bytes == 0u ||
            config->mpeg_feed_bytes > H1_CONFIG_SIGNED_INT_MAX)
            return 0;

        if (config->video_fps_num == 0u ||
            config->video_fps_den == 0u)
            return 0;

        if (config->video_scheduler_mode >
            PSTVNC_H1_VIDEO_SCHED_TWO_VSYNC)
            return 0;

        if (config->video_pixel_mode >
            PSTVNC_H1_VIDEO_RGB32)
            return 0;

        /*
         * The qualified GS uploader is macroblock-oriented. Multiples of 16
         * are therefore an implementation shape requirement, not a guessed
         * performance limit.
         */
        if (config->video_max_width == 0u ||
            config->video_max_height == 0u ||
            (config->video_max_width % 16u) != 0u ||
            (config->video_max_height % 16u) != 0u ||
            config->video_max_width > H1_CONFIG_SIGNED_INT_MAX ||
            config->video_max_height > H1_CONFIG_SIGNED_INT_MAX)
            return 0;

        if (config->video_draw_width == 0u ||
            config->video_draw_height == 0u ||
            config->video_draw_width > H1_CONFIG_SIGNED_INT_MAX ||
            config->video_draw_height > H1_CONFIG_SIGNED_INT_MAX ||
            config->video_draw_x > H1_CONFIG_SIGNED_INT_MAX ||
            config->video_draw_y > H1_CONFIG_SIGNED_INT_MAX)
            return 0;

        if (config->video_stage_markers > 1u ||
            config->video_ipu_reset_each_session > 1u ||
            config->video_drop_enabled > 1u)
            return 0;
    } else {
        if (config->mpeg_queue_capacity != 0u ||
            config->mpeg_initial_credit_bytes != 0u)
            return 0;
    }

    if (!h1_validate_thread(
            config->receiver_thread_priority,
            config->receiver_thread_stack_size))
        return 0;

    if (config->socket_receive_buffer_bytes >
            H1_CONFIG_SIGNED_INT_MAX ||
        config->socket_send_buffer_bytes >
            H1_CONFIG_SIGNED_INT_MAX)
        return 0;

    if (config->queue_allocation_order >
        PSTVNC_H1_ALLOCATE_MPEG_FIRST)
        return 0;

    return 1;
}

uint32_t pstvnc_h1_config_digest(
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

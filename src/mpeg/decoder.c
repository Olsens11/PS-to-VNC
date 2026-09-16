/*
 * File synopsis:
 * Implements A003's synchronous MPEG decoder ownership and safe-stop core.
 * Decoder-visible feed/picture resources are explicitly bounded and owned from
 * known-state preparation through decoder destruction. MPEG bytes arrive only
 * through Transport's public logical channel and temporary starvation waits on
 * Transport activity rather than timer polling.
 *
 * The critical safe-stop rule is structural here: the libmpeg-facing feed
 * callback never reads local owner-stop state. It reports only real Transport
 * data, real finite exhaustion, or real Transport/platform failure. Local stop
 * is observed only before entering or after returning from the injected picture
 * call, so an active decoder call can never receive synthetic lifecycle EOF.
 *
 * This file performs no exact-generation/Pi orchestration, GS presentation,
 * media-clock arm, scheduler/drop/calibration policy, application orchestration,
 * or hardware qualification.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#include "decoder.h"

#include "transport/bridge.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

static int pstvnc_mpeg_decoder_alignment_valid(size_t alignment)
{
    return alignment != 0u && (alignment & (alignment - 1u)) == 0u;
}

static int pstvnc_mpeg_decoder_config_valid(
    const pstvnc_mpeg_decoder_config_t *config)
{
    return config != NULL &&
        config->max_width != 0u &&
        config->max_height != 0u &&
        config->bytes_per_pixel != 0u &&
        config->feed_payload_capacity != 0u &&
        pstvnc_mpeg_decoder_alignment_valid(config->transfer_alignment) &&
        pstvnc_mpeg_decoder_alignment_valid(config->buffer_alignment) &&
        config->buffer_alignment >= config->transfer_alignment &&
        (config->buffer_alignment % config->transfer_alignment) == 0u;
}

static int pstvnc_mpeg_decoder_memory_ops_valid(
    const pstvnc_mpeg_decoder_memory_ops_t *memory_ops)
{
    return memory_ops != NULL &&
        memory_ops->allocate != NULL &&
        memory_ops->release != NULL;
}

static int pstvnc_mpeg_decoder_sync_ops_valid(
    const pstvnc_mpeg_decoder_sync_ops_t *sync_ops)
{
    return sync_ops != NULL &&
        sync_ops->lock != NULL &&
        sync_ops->unlock != NULL;
}

static int pstvnc_mpeg_decoder_platform_ops_valid(
    const pstvnc_mpeg_decoder_platform_ops_t *platform_ops)
{
    return platform_ops != NULL &&
        platform_ops->prepare_known_state != NULL &&
        platform_ops->initialize != NULL &&
        platform_ops->picture != NULL &&
        platform_ops->submit_feed != NULL &&
        platform_ops->destroy != NULL &&
        platform_ops->release_known_state != NULL;
}

static int pstvnc_mpeg_decoder_align_up(
    size_t value,
    size_t alignment,
    size_t *aligned_value)
{
    size_t mask;

    if (aligned_value == NULL ||
        !pstvnc_mpeg_decoder_alignment_valid(alignment))
        return 0;

    mask = alignment - 1u;
    if (value > SIZE_MAX - mask)
        return 0;

    *aligned_value = (value + mask) & ~mask;
    return *aligned_value != 0u;
}

static int pstvnc_mpeg_decoder_picture_capacity(
    const pstvnc_mpeg_decoder_config_t *config,
    size_t *picture_capacity)
{
    uint64_t macroblock_width;
    uint64_t macroblock_height;
    uint64_t macroblocks;
    uint64_t bytes;

    if (config == NULL || picture_capacity == NULL)
        return 0;

    macroblock_width = ((uint64_t)config->max_width + 15u) / 16u;
    macroblock_height = ((uint64_t)config->max_height + 15u) / 16u;

    if (macroblock_width == 0u || macroblock_height == 0u ||
        macroblock_width > UINT64_MAX / macroblock_height)
        return 0;

    macroblocks = macroblock_width * macroblock_height;
    if (macroblocks > UINT64_MAX / 256u)
        return 0;

    bytes = macroblocks * 256u;
    if (bytes > UINT64_MAX / (uint64_t)config->bytes_per_pixel)
        return 0;

    bytes *= (uint64_t)config->bytes_per_pixel;
    if (bytes == 0u || bytes > (uint64_t)SIZE_MAX)
        return 0;

    *picture_capacity = (size_t)bytes;
    return 1;
}

static void pstvnc_mpeg_decoder_free_buffers(
    pstvnc_mpeg_decoder_t *decoder)
{
    if (decoder->picture_buffer != NULL) {
        decoder->memory_ops.release(
            decoder->memory_ops.context,
            decoder->picture_buffer);
        decoder->picture_buffer = NULL;
    }

    if (decoder->feed_buffer != NULL) {
        decoder->memory_ops.release(
            decoder->memory_ops.context,
            decoder->feed_buffer);
        decoder->feed_buffer = NULL;
    }
}

static int pstvnc_mpeg_decoder_observe_stop(
    pstvnc_mpeg_decoder_t *decoder,
    int *stop_requested)
{
    int observed;

    if (decoder == NULL || stop_requested == NULL)
        return 0;

    if (decoder->sync_ops.lock(decoder->sync_ops.context) != 0)
        return 0;

    observed = decoder->stop_requested;

    if (decoder->sync_ops.unlock(decoder->sync_ops.context) != 0)
        return 0;

    *stop_requested = observed != 0;
    return 1;
}

static int pstvnc_mpeg_decoder_set_call_active(
    pstvnc_mpeg_decoder_t *decoder,
    int active)
{
    if (decoder->sync_ops.lock(decoder->sync_ops.context) != 0)
        return 0;

    decoder->decoder_call_active = active != 0;

    if (decoder->sync_ops.unlock(decoder->sync_ops.context) != 0)
        return 0;

    return 1;
}

static int pstvnc_mpeg_decoder_call_is_active(
    pstvnc_mpeg_decoder_t *decoder,
    int *active)
{
    int observed;

    if (active == NULL)
        return 0;

    if (decoder->sync_ops.lock(decoder->sync_ops.context) != 0)
        return 0;

    observed = decoder->decoder_call_active;

    if (decoder->sync_ops.unlock(decoder->sync_ops.context) != 0)
        return 0;

    *active = observed != 0;
    return 1;
}

static void pstvnc_mpeg_decoder_record_transport_failure(
    pstvnc_mpeg_decoder_t *decoder,
    pstvnc_transport_result_t result)
{
    decoder->feed_failed = 1;
    decoder->feed_transport_result = result;
    decoder->report.transport_result = result;
}

/*
 * Decoder data callback. Deliberately do not inspect decoder->stop_requested.
 * Local cancellation is not a stream fact and therefore cannot become EOF here.
 */
static int pstvnc_mpeg_decoder_feed(void *context)
{
    pstvnc_mpeg_decoder_t *decoder = (pstvnc_mpeg_decoder_t *)context;
    uint32_t activity_sequence;
    pstvnc_transport_result_t transport_result;

    if (decoder == NULL || !decoder->initialized ||
        decoder->feed_buffer == NULL)
        return -1;

    transport_result = pstvnc_transport_mpeg_activity_snapshot(
        &activity_sequence);
    if (transport_result != PSTVNC_TRANSPORT_OK) {
        pstvnc_mpeg_decoder_record_transport_failure(decoder, transport_result);
        return -1;
    }

    for (;;) {
        size_t read_count = 0u;

        transport_result = pstvnc_transport_mpeg_read_available(
            decoder->feed_buffer,
            decoder->config.feed_payload_capacity,
            &read_count);

        if (transport_result == PSTVNC_TRANSPORT_OK) {
            size_t transfer_bytes;

            if (read_count == 0u ||
                !pstvnc_mpeg_decoder_align_up(
                    read_count,
                    decoder->config.transfer_alignment,
                    &transfer_bytes) ||
                transfer_bytes > decoder->feed_transfer_capacity) {
                decoder->feed_failed = 1;
                decoder->feed_transport_result = PSTVNC_TRANSPORT_OK;
                return -1;
            }

            if (transfer_bytes > read_count) {
                memset(
                    decoder->feed_buffer + read_count,
                    0,
                    transfer_bytes - read_count);
            }

            if (decoder->platform_ops.submit_feed(
                    decoder->platform_ops.context,
                    decoder->feed_buffer,
                    read_count,
                    transfer_bytes) != 0) {
                decoder->feed_failed = 1;
                decoder->feed_transport_result = PSTVNC_TRANSPORT_OK;
                return -1;
            }

            if (decoder->report.payload_bytes_consumed >
                    UINT64_MAX - (uint64_t)read_count ||
                decoder->report.transfer_bytes_submitted >
                    UINT64_MAX - (uint64_t)transfer_bytes ||
                decoder->report.feed_callbacks == UINT32_MAX) {
                decoder->feed_failed = 1;
                decoder->feed_transport_result = PSTVNC_TRANSPORT_OK;
                return -1;
            }

            decoder->report.payload_bytes_consumed += (uint64_t)read_count;
            decoder->report.transfer_bytes_submitted +=
                (uint64_t)transfer_bytes;
            decoder->report.feed_callbacks += 1u;
            return 1;
        }

        if (transport_result == PSTVNC_TRANSPORT_WOULD_BLOCK) {
            transport_result = pstvnc_transport_mpeg_wait_activity(
                &activity_sequence);
            if (transport_result != PSTVNC_TRANSPORT_OK) {
                pstvnc_mpeg_decoder_record_transport_failure(
                    decoder, transport_result);
                return -1;
            }
            continue;
        }

        if (transport_result == PSTVNC_TRANSPORT_EXHAUSTED) {
            decoder->feed_exhausted = 1;
            decoder->report.transport_result = PSTVNC_TRANSPORT_EXHAUSTED;
            return 0;
        }

        pstvnc_mpeg_decoder_record_transport_failure(decoder, transport_result);
        return -1;
    }
}

static void *pstvnc_mpeg_decoder_sequence(
    void *context,
    uint32_t width,
    uint32_t height,
    size_t *picture_capacity)
{
    pstvnc_mpeg_decoder_t *decoder = (pstvnc_mpeg_decoder_t *)context;
    uint64_t macroblock_width;
    uint64_t macroblock_height;
    uint64_t required;

    if (decoder == NULL || picture_capacity == NULL ||
        width == 0u || height == 0u ||
        width > decoder->config.max_width ||
        height > decoder->config.max_height) {
        if (decoder != NULL)
            decoder->sequence_invalid = 1;
        return NULL;
    }

    macroblock_width = ((uint64_t)width + 15u) / 16u;
    macroblock_height = ((uint64_t)height + 15u) / 16u;

    if (macroblock_width > UINT64_MAX / macroblock_height) {
        decoder->sequence_invalid = 1;
        return NULL;
    }

    required = macroblock_width * macroblock_height;
    if (required > UINT64_MAX / 256u) {
        decoder->sequence_invalid = 1;
        return NULL;
    }
    required *= 256u;

    if (required > UINT64_MAX / (uint64_t)decoder->config.bytes_per_pixel) {
        decoder->sequence_invalid = 1;
        return NULL;
    }
    required *= (uint64_t)decoder->config.bytes_per_pixel;

    if (required == 0u || required > (uint64_t)decoder->picture_capacity) {
        decoder->sequence_invalid = 1;
        return NULL;
    }

    decoder->sequence_seen = 1;
    decoder->report.sequence_width = width;
    decoder->report.sequence_height = height;
    *picture_capacity = decoder->picture_capacity;
    return decoder->picture_buffer;
}

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_initialize(
    pstvnc_mpeg_decoder_t *decoder,
    const pstvnc_mpeg_decoder_config_t *config,
    const pstvnc_mpeg_decoder_memory_ops_t *memory_ops,
    const pstvnc_mpeg_decoder_sync_ops_t *sync_ops,
    const pstvnc_mpeg_decoder_platform_ops_t *platform_ops)
{
    size_t picture_capacity;
    size_t feed_transfer_capacity;

    if (decoder == NULL ||
        !pstvnc_mpeg_decoder_config_valid(config) ||
        !pstvnc_mpeg_decoder_memory_ops_valid(memory_ops) ||
        !pstvnc_mpeg_decoder_sync_ops_valid(sync_ops) ||
        !pstvnc_mpeg_decoder_platform_ops_valid(platform_ops) ||
        !pstvnc_mpeg_decoder_picture_capacity(config, &picture_capacity) ||
        !pstvnc_mpeg_decoder_align_up(
            config->feed_payload_capacity,
            config->transfer_alignment,
            &feed_transfer_capacity))
        return PSTVNC_MPEG_DECODER_INVALID;

    memset(decoder, 0, sizeof(*decoder));
    decoder->config = *config;
    decoder->memory_ops = *memory_ops;
    decoder->sync_ops = *sync_ops;
    decoder->platform_ops = *platform_ops;
    decoder->picture_capacity = picture_capacity;
    decoder->feed_transfer_capacity = feed_transfer_capacity;
    decoder->report.transport_result = PSTVNC_TRANSPORT_WOULD_BLOCK;

    decoder->feed_buffer = (uint8_t *)decoder->memory_ops.allocate(
        decoder->memory_ops.context,
        decoder->feed_transfer_capacity,
        decoder->config.buffer_alignment);
    if (decoder->feed_buffer == NULL)
        return PSTVNC_MPEG_DECODER_ALLOCATION_FAILED;

    decoder->picture_buffer = decoder->memory_ops.allocate(
        decoder->memory_ops.context,
        decoder->picture_capacity,
        decoder->config.buffer_alignment);
    if (decoder->picture_buffer == NULL) {
        pstvnc_mpeg_decoder_free_buffers(decoder);
        return PSTVNC_MPEG_DECODER_ALLOCATION_FAILED;
    }

    if (decoder->platform_ops.prepare_known_state(
            decoder->platform_ops.context) != 0) {
        pstvnc_mpeg_decoder_free_buffers(decoder);
        return PSTVNC_MPEG_DECODER_PREPARE_FAILED;
    }
    decoder->known_state_prepared = 1;

    /*
     * Mark initialized before registering callbacks because a conforming
     * decoder may request sequence/data synchronously during initialize().
     */
    decoder->initialized = 1;

    if (decoder->platform_ops.initialize(
            decoder->platform_ops.context,
            pstvnc_mpeg_decoder_feed,
            decoder,
            pstvnc_mpeg_decoder_sequence,
            decoder) != 0) {
        decoder->initialized = 0;
        if (decoder->platform_ops.release_known_state(
                decoder->platform_ops.context) != 0)
            return PSTVNC_MPEG_DECODER_STATE_RELEASE_FAILED;
        decoder->known_state_prepared = 0;
        pstvnc_mpeg_decoder_free_buffers(decoder);
        return PSTVNC_MPEG_DECODER_INITIALIZE_FAILED;
    }

    decoder->decoder_initialized = 1;
    return PSTVNC_MPEG_DECODER_COMPLETE;
}

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_request_stop(
    pstvnc_mpeg_decoder_t *decoder)
{
    if (decoder == NULL || !decoder->initialized)
        return PSTVNC_MPEG_DECODER_INVALID;

    if (decoder->sync_ops.lock(decoder->sync_ops.context) != 0)
        return PSTVNC_MPEG_DECODER_SYNC_FAILED;

    decoder->stop_requested = 1;

    if (decoder->sync_ops.unlock(decoder->sync_ops.context) != 0)
        return PSTVNC_MPEG_DECODER_SYNC_FAILED;

    return PSTVNC_MPEG_DECODER_COMPLETE;
}

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_run(
    pstvnc_mpeg_decoder_t *decoder,
    pstvnc_mpeg_decoder_report_t *report)
{
    if (decoder == NULL || report == NULL || !decoder->initialized ||
        !decoder->decoder_initialized)
        return PSTVNC_MPEG_DECODER_INVALID;

    for (;;) {
        int stop_requested = 0;
        int picture_result;

        if (!pstvnc_mpeg_decoder_observe_stop(decoder, &stop_requested))
            return PSTVNC_MPEG_DECODER_SYNC_FAILED;
        if (stop_requested) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_STOPPED;
        }

        if (!pstvnc_mpeg_decoder_set_call_active(decoder, 1))
            return PSTVNC_MPEG_DECODER_SYNC_FAILED;

        picture_result = decoder->platform_ops.picture(
            decoder->platform_ops.context);

        if (!pstvnc_mpeg_decoder_set_call_active(decoder, 0))
            return PSTVNC_MPEG_DECODER_SYNC_FAILED;

        if (decoder->sequence_invalid) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_SEQUENCE_INVALID;
        }

        if (decoder->feed_failed) {
            *report = decoder->report;
            return decoder->feed_transport_result == PSTVNC_TRANSPORT_OK
                ? PSTVNC_MPEG_DECODER_TRANSFER_FAILED
                : PSTVNC_MPEG_DECODER_TRANSPORT_FAILED;
        }

        if (picture_result < 0) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_PICTURE_FAILED;
        }

        if (picture_result == 0) {
            if (decoder->feed_exhausted) {
                *report = decoder->report;
                return PSTVNC_MPEG_DECODER_COMPLETE;
            }

            if (!pstvnc_mpeg_decoder_observe_stop(
                    decoder, &stop_requested))
                return PSTVNC_MPEG_DECODER_SYNC_FAILED;

            *report = decoder->report;
            return stop_requested
                ? PSTVNC_MPEG_DECODER_STOPPED
                : PSTVNC_MPEG_DECODER_UNEXPECTED_END;
        }

        if (!decoder->sequence_seen) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_SEQUENCE_INVALID;
        }

        if (decoder->report.pictures_decoded == UINT32_MAX) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_ACCOUNTING_FAILED;
        }
        decoder->report.pictures_decoded += 1u;

        /* Safe-stop authority point: only after picture/decode ownership returns. */
        if (!pstvnc_mpeg_decoder_observe_stop(decoder, &stop_requested))
            return PSTVNC_MPEG_DECODER_SYNC_FAILED;
        if (stop_requested) {
            *report = decoder->report;
            return PSTVNC_MPEG_DECODER_STOPPED;
        }
    }
}

pstvnc_mpeg_decoder_result_t pstvnc_mpeg_decoder_release(
    pstvnc_mpeg_decoder_t *decoder)
{
    int call_active;

    if (decoder == NULL)
        return PSTVNC_MPEG_DECODER_INVALID;

    if (!decoder->initialized && !decoder->known_state_prepared &&
        !decoder->decoder_initialized && decoder->feed_buffer == NULL &&
        decoder->picture_buffer == NULL)
        return PSTVNC_MPEG_DECODER_COMPLETE;

    if (!pstvnc_mpeg_decoder_call_is_active(decoder, &call_active))
        return PSTVNC_MPEG_DECODER_SYNC_FAILED;
    if (call_active)
        return PSTVNC_MPEG_DECODER_CALL_ACTIVE;

    if (decoder->decoder_initialized) {
        if (decoder->platform_ops.destroy(
                decoder->platform_ops.context) != 0)
            return PSTVNC_MPEG_DECODER_DESTROY_FAILED;
        decoder->decoder_initialized = 0;
    }

    if (decoder->known_state_prepared) {
        if (decoder->platform_ops.release_known_state(
                decoder->platform_ops.context) != 0)
            return PSTVNC_MPEG_DECODER_STATE_RELEASE_FAILED;
        decoder->known_state_prepared = 0;
    }

    pstvnc_mpeg_decoder_free_buffers(decoder);
    decoder->initialized = 0;
    return PSTVNC_MPEG_DECODER_COMPLETE;
}

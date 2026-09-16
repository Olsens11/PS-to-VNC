/*
 * File synopsis:
 * Decodes and validates the clean production session CONFIG payload into narrow
 * immutable subprofiles. It preserves explicit provenance for every A001
 * Transport value and the audited A002 PCM/common-clock facts while rejecting
 * malformed, incomplete, duplicate, unknown, or structurally impossible input
 * before publishing any decoded state.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "profile.h"

#include "transport/protocol.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define PSTVNC_CONFIG_RECEIVER_PRIORITY_MAX 127u
#define PSTVNC_CONFIG_RECEIVER_STACK_MIN 256u
#define PSTVNC_CONFIG_RECEIVER_STACK_ALIGNMENT 16u
#define PSTVNC_CONFIG_PCM_VOLUME_MAX 100u
#define PSTVNC_CONFIG_REQUIRED_MASK \
    ((UINT32_C(1) << PSTVNC_CONFIG_PROFILE_FIELD_COUNT) - UINT32_C(1))

static uint32_t pstvnc_config_read_be32(const uint8_t input[4])
{
    return ((uint32_t)input[0] << 24) |
           ((uint32_t)input[1] << 16) |
           ((uint32_t)input[2] << 8) |
           (uint32_t)input[3];
}

static int32_t pstvnc_config_signed_bits(uint32_t raw)
{
    int32_t value;

    memcpy(&value, &raw, sizeof(value));
    return value;
}

static int pstvnc_config_set_field(
    pstvnc_config_session_profile_t *profile,
    uint32_t field_id,
    uint32_t raw_value)
{
    switch (field_id) {
        case PSTVNC_CONFIG_FIELD_SESSION_ID:
            profile->composition.session_id = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_MODE:
            if (raw_value > (uint32_t)PSTVNC_CONFIG_RFB_ON)
                return 0;
            profile->composition.rfb_mode =
                (pstvnc_config_rfb_mode_t)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_MODE:
            if (raw_value > (uint32_t)PSTVNC_CONFIG_AUDIO_PCM)
                return 0;
            profile->composition.audio_mode =
                (pstvnc_config_audio_mode_t)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_VIDEO_MODE:
            if (raw_value > (uint32_t)PSTVNC_CONFIG_VIDEO_ACTIVE)
                return 0;
            profile->composition.video_mode =
                (pstvnc_config_video_mode_t)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_RATE:
            profile->pcm.rate_hz = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS:
            profile->pcm.channels = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_BITS:
            profile->pcm.bits_per_sample = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_VOLUME:
            profile->pcm.volume_percent = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US:
            profile->media_clock.audio_presentation_offset_us =
                pstvnc_config_signed_bits(raw_value);
            return 1;
        case PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US:
            profile->media_clock.video_presentation_offset_us =
                pstvnc_config_signed_bits(raw_value);
            return 1;
        case PSTVNC_CONFIG_FIELD_MEDIA_EPOCH_LEAD_US:
            profile->media_clock.epoch_lead_us = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY:
            profile->transport.rfb_queue_capacity = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES:
            profile->transport.rfb_initial_credit_bytes = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES:
            profile->transport.rfb_credit_batch_bytes = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY:
            if (raw_value > 1u)
                return 0;
            profile->transport.rfb_credit_flush_on_empty = (int)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RFB_CREDIT_RETURN_ENABLED:
            if (raw_value > 1u)
                return 0;
            profile->transport.rfb_credit_return_enabled = (int)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE:
            profile->transport.receiver_thread_stack_size = raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY:
            if (raw_value > (uint32_t)INT_MAX)
                return 0;
            profile->transport.receiver_thread_priority = (int)raw_value;
            return 1;
        case PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD:
            profile->transport.max_data_payload = raw_value;
            return 1;
        default:
            return 0;
    }
}

static int pstvnc_config_transport_valid(
    const pstvnc_config_session_profile_t *profile)
{
    const pstvnc_transport_session_config_t *transport = &profile->transport;

    if (transport->receiver_thread_priority <= 0 ||
        transport->receiver_thread_priority >
            (int)PSTVNC_CONFIG_RECEIVER_PRIORITY_MAX ||
        transport->receiver_thread_stack_size <
            PSTVNC_CONFIG_RECEIVER_STACK_MIN ||
        transport->receiver_thread_stack_size > (uint32_t)INT_MAX ||
        (transport->receiver_thread_stack_size %
            PSTVNC_CONFIG_RECEIVER_STACK_ALIGNMENT) != 0u ||
        transport->max_data_payload == 0u ||
        transport->max_data_payload > PSTVNC_TRANSPORT_MAX_PAYLOAD)
        return 0;

    if (profile->composition.rfb_mode == PSTVNC_CONFIG_RFB_OFF) {
        return transport->rfb_queue_capacity == 0u &&
               transport->rfb_initial_credit_bytes == 0u &&
               transport->rfb_credit_batch_bytes == 0u &&
               transport->rfb_credit_flush_on_empty == 0 &&
               transport->rfb_credit_return_enabled == 0;
    }

    if (transport->rfb_queue_capacity == 0u ||
        transport->max_data_payload > transport->rfb_queue_capacity ||
        transport->rfb_initial_credit_bytes > transport->rfb_queue_capacity ||
        (transport->rfb_credit_flush_on_empty != 0 &&
         transport->rfb_credit_flush_on_empty != 1) ||
        (transport->rfb_credit_return_enabled != 0 &&
         transport->rfb_credit_return_enabled != 1))
        return 0;

    if (transport->rfb_credit_return_enabled) {
        if (transport->rfb_credit_batch_bytes == 0u ||
            transport->rfb_credit_batch_bytes > transport->rfb_queue_capacity)
            return 0;
    } else if (transport->rfb_credit_batch_bytes != 0u) {
        return 0;
    }

    return 1;
}

static int pstvnc_config_pcm_valid(
    const pstvnc_config_session_profile_t *profile)
{
    const pstvnc_config_pcm_profile_t *pcm = &profile->pcm;

    if (profile->composition.audio_mode == PSTVNC_CONFIG_AUDIO_OFF) {
        return pcm->rate_hz == 0u &&
               pcm->channels == 0u &&
               pcm->bits_per_sample == 0u &&
               pcm->volume_percent == 0u &&
               profile->media_clock.audio_presentation_offset_us == 0;
    }

    if (pcm->rate_hz == 0u || pcm->rate_hz > (uint32_t)INT_MAX ||
        (pcm->channels != 1u && pcm->channels != 2u) ||
        (pcm->bits_per_sample != 8u && pcm->bits_per_sample != 16u) ||
        pcm->volume_percent > PSTVNC_CONFIG_PCM_VOLUME_MAX)
        return 0;

    return 1;
}

static int pstvnc_config_profile_valid(
    const pstvnc_config_session_profile_t *profile)
{
    if (profile->composition.rfb_mode != PSTVNC_CONFIG_RFB_OFF &&
        profile->composition.rfb_mode != PSTVNC_CONFIG_RFB_ON)
        return 0;

    if (profile->composition.audio_mode != PSTVNC_CONFIG_AUDIO_OFF &&
        profile->composition.audio_mode != PSTVNC_CONFIG_AUDIO_PCM)
        return 0;

    if (profile->composition.video_mode != PSTVNC_CONFIG_VIDEO_OFF &&
        profile->composition.video_mode != PSTVNC_CONFIG_VIDEO_ACTIVE)
        return 0;

    if (profile->composition.video_mode == PSTVNC_CONFIG_VIDEO_OFF &&
        profile->media_clock.video_presentation_offset_us != 0)
        return 0;

    return pstvnc_config_transport_valid(profile) &&
           pstvnc_config_pcm_valid(profile);
}

int pstvnc_config_profile_decode(
    pstvnc_config_session_profile_t *profile,
    const uint8_t *payload,
    size_t payload_length)
{
    pstvnc_config_session_profile_t candidate;
    uint32_t seen = 0u;
    uint32_t entry_count;
    size_t expected_length;
    size_t offset;

    if (profile == NULL || payload == NULL ||
        payload_length < PSTVNC_CONFIG_PROFILE_HEADER_BYTES)
        return 0;

    if (pstvnc_config_read_be32(&payload[0]) !=
        PSTVNC_CONFIG_PROFILE_VERSION)
        return 0;

    entry_count = pstvnc_config_read_be32(&payload[4]);
    if (entry_count > PSTVNC_CONFIG_PROFILE_FIELD_COUNT)
        return 0;

    expected_length = PSTVNC_CONFIG_PROFILE_HEADER_BYTES +
        (size_t)entry_count * PSTVNC_CONFIG_PROFILE_ENTRY_BYTES;
    if (payload_length != expected_length)
        return 0;

    memset(&candidate, 0, sizeof(candidate));

    for (offset = PSTVNC_CONFIG_PROFILE_HEADER_BYTES;
         offset < payload_length;
         offset += PSTVNC_CONFIG_PROFILE_ENTRY_BYTES) {
        uint32_t field_id = pstvnc_config_read_be32(&payload[offset]);
        uint32_t raw_value = pstvnc_config_read_be32(&payload[offset + 4u]);
        uint32_t bit;

        if (field_id == 0u || field_id > PSTVNC_CONFIG_PROFILE_FIELD_COUNT)
            return 0;

        bit = UINT32_C(1) << (field_id - 1u);
        if ((seen & bit) != 0u)
            return 0;

        if (!pstvnc_config_set_field(&candidate, field_id, raw_value))
            return 0;

        seen |= bit;
    }

    if (seen != PSTVNC_CONFIG_REQUIRED_MASK ||
        !pstvnc_config_profile_valid(&candidate))
        return 0;

    *profile = candidate;
    return 1;
}

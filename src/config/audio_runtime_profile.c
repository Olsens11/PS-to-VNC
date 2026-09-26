/*
 * File synopsis:
 * Publishes Configuration's one evidence-selected AUDIO runtime profile using
 * existing narrow owner types. Values are the clean projection of qualified H1
 * P11_COMPAT_PLUS_PCM evidence and are not a user/network tuning surface.
 *
 * This file performs no allocation, Transport open/access, AUDIO worker start,
 * AUDSRV call, media-clock arm/wait, Application routing, or Pi mutation.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36.
 */

#include "audio_runtime_profile.h"
#include "audio_runtime_profile_generated.h"

#include <limits.h>

static const pstvnc_config_audio_runtime_profile_t
    pstvnc_config_selected_audio_runtime_profile = {
        .transport = {
            .queue_capacity = PSTVNC_CONFIG_AUDIO_QUEUE_CAPACITY,
            .initial_credit_bytes = PSTVNC_CONFIG_AUDIO_INITIAL_CREDIT_BYTES,
            .credit_batch_bytes = PSTVNC_CONFIG_AUDIO_CREDIT_BATCH_BYTES,
            .credit_flush_on_empty =
                PSTVNC_CONFIG_AUDIO_CREDIT_FLUSH_ON_EMPTY,
            .credit_return_enabled =
                PSTVNC_CONFIG_AUDIO_CREDIT_RETURN_ENABLED
        },
        .pcm = {
            .rate_hz = PSTVNC_CONFIG_AUDIO_PCM_RATE_HZ,
            .channels = PSTVNC_CONFIG_AUDIO_PCM_CHANNELS,
            .bits_per_sample = PSTVNC_CONFIG_AUDIO_PCM_BITS_PER_SAMPLE,
            .volume_percent = PSTVNC_CONFIG_AUDIO_PCM_VOLUME_PERCENT
        },
        .session = {
            .worker_stack_bytes = PSTVNC_CONFIG_AUDIO_WORKER_STACK_BYTES,
            .worker_priority = PSTVNC_CONFIG_AUDIO_WORKER_PRIORITY,
            .playback_buffer_capacity =
                PSTVNC_CONFIG_AUDIO_PLAYBACK_BUFFER_CAPACITY,
            .startup_reservoir_bytes =
                PSTVNC_CONFIG_AUDIO_STARTUP_RESERVOIR_BYTES,
            .reservoir_poll_us = PSTVNC_CONFIG_AUDIO_RESERVOIR_POLL_US,
            .clock_poll_us = PSTVNC_CONFIG_AUDIO_CLOCK_POLL_US
        }
    };

int pstvnc_config_audio_runtime_profile_valid(
    const pstvnc_config_audio_runtime_profile_t *profile)
{
    const pstvnc_transport_audio_channel_config_t *transport;
    const pstvnc_config_pcm_profile_t *pcm;
    const pstvnc_audio_session_values_t *session;

    if (profile == NULL)
        return 0;

    transport = &profile->transport;
    pcm = &profile->pcm;
    session = &profile->session;

    if (transport->queue_capacity == 0u ||
        transport->initial_credit_bytes > transport->queue_capacity ||
        transport->credit_batch_bytes == 0u ||
        transport->credit_batch_bytes > transport->queue_capacity ||
        (transport->credit_flush_on_empty != 0 &&
         transport->credit_flush_on_empty != 1) ||
        (transport->credit_return_enabled != 0 &&
         transport->credit_return_enabled != 1) ||
        !transport->credit_return_enabled)
        return 0;

    if (pcm->rate_hz == 0u || pcm->rate_hz > (uint32_t)INT_MAX ||
        (pcm->channels != 1u && pcm->channels != 2u) ||
        (pcm->bits_per_sample != 8u && pcm->bits_per_sample != 16u) ||
        pcm->volume_percent > 100u)
        return 0;

    if (session->worker_stack_bytes == 0u ||
        session->worker_stack_bytes > (size_t)INT_MAX ||
        session->worker_priority <= 0 ||
        session->worker_priority > 127 ||
        session->playback_buffer_capacity == 0u ||
        session->playback_buffer_capacity > (size_t)INT_MAX ||
        session->playback_buffer_capacity > transport->queue_capacity ||
        session->startup_reservoir_bytes == 0u ||
        session->startup_reservoir_bytes > transport->queue_capacity ||
        session->reservoir_poll_us == 0u ||
        session->clock_poll_us == 0u)
        return 0;

    return 1;
}

int pstvnc_config_audio_runtime_profile_selected(
    pstvnc_config_audio_runtime_profile_t *profile)
{
    if (profile == NULL ||
        !pstvnc_config_audio_runtime_profile_valid(
            &pstvnc_config_selected_audio_runtime_profile))
        return 0;

    *profile = pstvnc_config_selected_audio_runtime_profile;
    return 1;
}

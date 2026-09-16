/*
 * File synopsis:
 * Implements the synchronous A002 PCM playback core over Transport's logical
 * AUDIO consumer seam. It applies the immutable PCM profile exactly, waits for
 * AUDSRV capacity before every nonzero submission, and records bytes only after
 * successful playback submission.
 *
 * AUDSRV/LIBSD service lifetime is resident-ELF scoped: this component may
 * initialize/use the resident service and stops the current session stream on
 * retirement, but it has no quit operation. Worker lifecycle, startup reservoir,
 * media-clock timing, and application policy remain outside this file.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "playback.h"

#include "transport/bridge.h"

#include <limits.h>
#include <string.h>

static int pstvnc_audio_playback_profile_valid(
    const pstvnc_config_pcm_profile_t *profile)
{
    return profile != NULL &&
           profile->rate_hz != 0u &&
           profile->rate_hz <= (uint32_t)INT_MAX &&
           (profile->channels == 1u || profile->channels == 2u) &&
           (profile->bits_per_sample == 8u || profile->bits_per_sample == 16u) &&
           profile->volume_percent <= 100u;
}

static int pstvnc_audio_playback_service_valid(
    const pstvnc_audio_service_ops_t *service)
{
    return service != NULL &&
           service->initialize != NULL &&
           service->set_format != NULL &&
           service->set_volume != NULL &&
           service->wait_audio != NULL &&
           service->play_audio != NULL &&
           service->stop_audio != NULL;
}

static pstvnc_audio_playback_result_t pstvnc_audio_playback_transport_result(
    pstvnc_transport_result_t result)
{
    switch (result) {
        case PSTVNC_TRANSPORT_STOPPED:
            return PSTVNC_AUDIO_PLAYBACK_STOPPED;
        case PSTVNC_TRANSPORT_CLOSED:
            return PSTVNC_AUDIO_PLAYBACK_CLOSED;
        case PSTVNC_TRANSPORT_INVALID:
            return PSTVNC_AUDIO_PLAYBACK_TRANSPORT_INVALID;
        case PSTVNC_TRANSPORT_FAILED:
            return PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED;
        default:
            return PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED;
    }
}

static pstvnc_audio_playback_result_t pstvnc_audio_playback_finish(
    const pstvnc_audio_service_ops_t *service,
    pstvnc_audio_playback_result_t result,
    pstvnc_audio_playback_report_t *report)
{
    report->cleanup_attempted = 1;
    if (service->stop_audio(service->context) != 0) {
        report->cleanup_failed = 1;
        if (result == PSTVNC_AUDIO_PLAYBACK_COMPLETE)
            return PSTVNC_AUDIO_PLAYBACK_SERVICE_STOP_FAILED;
    }

    return result;
}

pstvnc_audio_playback_result_t pstvnc_audio_playback_run(
    const pstvnc_config_pcm_profile_t *profile,
    uint8_t *buffer,
    size_t buffer_capacity,
    const pstvnc_audio_service_ops_t *service,
    pstvnc_audio_playback_report_t *report)
{
    pstvnc_audio_playback_result_t result = PSTVNC_AUDIO_PLAYBACK_COMPLETE;

    if (report != NULL)
        memset(report, 0, sizeof(*report));

    if (!pstvnc_audio_playback_profile_valid(profile) ||
        buffer == NULL || buffer_capacity == 0u ||
        buffer_capacity > (size_t)INT_MAX ||
        !pstvnc_audio_playback_service_valid(service) || report == NULL)
        return PSTVNC_AUDIO_PLAYBACK_INVALID;

    if (service->initialize(service->context) != 0)
        return PSTVNC_AUDIO_PLAYBACK_SERVICE_INIT_FAILED;

    if (service->set_format(
            service->context,
            profile->rate_hz,
            profile->channels,
            profile->bits_per_sample) != 0) {
        result = PSTVNC_AUDIO_PLAYBACK_SERVICE_FORMAT_FAILED;
        return pstvnc_audio_playback_finish(service, result, report);
    }

    if (service->set_volume(
            service->context, profile->volume_percent) != 0) {
        result = PSTVNC_AUDIO_PLAYBACK_SERVICE_VOLUME_FAILED;
        return pstvnc_audio_playback_finish(service, result, report);
    }

    for (;;) {
        uint32_t activity_sequence = 0u;
        size_t read_count = 0u;
        pstvnc_transport_result_t transport_result;

        transport_result = pstvnc_transport_audio_activity_snapshot(
            &activity_sequence);
        if (transport_result != PSTVNC_TRANSPORT_OK) {
            result = pstvnc_audio_playback_transport_result(transport_result);
            break;
        }

        transport_result = pstvnc_transport_audio_read_available(
            buffer, buffer_capacity, &read_count);

        if (transport_result == PSTVNC_TRANSPORT_EXHAUSTED) {
            result = PSTVNC_AUDIO_PLAYBACK_COMPLETE;
            break;
        }

        if (transport_result == PSTVNC_TRANSPORT_WOULD_BLOCK) {
            transport_result = pstvnc_transport_audio_wait_activity(
                &activity_sequence);
            if (transport_result != PSTVNC_TRANSPORT_OK) {
                result = pstvnc_audio_playback_transport_result(transport_result);
                break;
            }
            continue;
        }

        if (transport_result != PSTVNC_TRANSPORT_OK) {
            result = pstvnc_audio_playback_transport_result(transport_result);
            break;
        }

        if (read_count == 0u || read_count > buffer_capacity) {
            result = PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED;
            break;
        }

        if (report->submitted_bytes > UINT64_MAX - (uint64_t)read_count ||
            report->submitted_chunks == UINT64_MAX) {
            result = PSTVNC_AUDIO_PLAYBACK_ACCOUNTING_FAILED;
            break;
        }

        if (service->wait_audio(service->context, read_count) != 0) {
            result = PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED;
            break;
        }

        if (service->play_audio(
                service->context, buffer, read_count) != 0) {
            result = PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED;
            break;
        }

        report->submitted_bytes += (uint64_t)read_count;
        report->submitted_chunks += 1u;
    }

    return pstvnc_audio_playback_finish(service, result, report);
}

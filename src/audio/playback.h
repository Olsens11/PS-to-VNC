/*
 * File synopsis:
 * Defines the synchronous A002 PCM playback owner. It consumes the immutable
 * PCM session profile and the public Transport AUDIO seam, while all AUDSRV
 * effects are supplied through a narrow injectable resident-service boundary.
 *
 * This interface does not own a playback thread, chunk-size policy, startup
 * reservoir, common-clock timing, application orchestration, or service quit.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_AUDIO_PLAYBACK_H
#define PSTVNC_AUDIO_PLAYBACK_H

#include "config/profile.h"
#include "transport/transport.h"

#include <stddef.h>
#include <stdint.h>

typedef enum pstvnc_audio_playback_result {
    PSTVNC_AUDIO_PLAYBACK_COMPLETE = 0,
    PSTVNC_AUDIO_PLAYBACK_STOPPED = 1,
    PSTVNC_AUDIO_PLAYBACK_CLOSED = 2,
    PSTVNC_AUDIO_PLAYBACK_INVALID = -1,
    PSTVNC_AUDIO_PLAYBACK_TRANSPORT_INVALID = -2,
    PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED = -3,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_INIT_FAILED = -4,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_FORMAT_FAILED = -5,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_VOLUME_FAILED = -6,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED = -7,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED = -8,
    PSTVNC_AUDIO_PLAYBACK_SERVICE_STOP_FAILED = -9,
    PSTVNC_AUDIO_PLAYBACK_ACCOUNTING_FAILED = -10
} pstvnc_audio_playback_result_t;

typedef struct pstvnc_audio_service_ops {
    int (*initialize)(void *context);
    int (*set_format)(
        void *context,
        uint32_t rate_hz,
        uint32_t channels,
        uint32_t bits_per_sample);
    int (*set_volume)(void *context, uint32_t volume_percent);
    int (*wait_audio)(void *context, size_t byte_count);
    int (*play_audio)(
        void *context,
        const uint8_t *bytes,
        size_t byte_count);
    int (*stop_audio)(void *context);
    void *context;
} pstvnc_audio_service_ops_t;

typedef struct pstvnc_audio_playback_report {
    uint64_t submitted_bytes;
    uint64_t submitted_chunks;
    int cleanup_attempted;
    int cleanup_failed;
} pstvnc_audio_playback_report_t;

pstvnc_audio_playback_result_t pstvnc_audio_playback_run(
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_config_pcm_profile_t *profile,
    uint8_t *buffer,
    size_t buffer_capacity,
    const pstvnc_audio_service_ops_t *service,
    pstvnc_audio_playback_report_t *report);

#endif

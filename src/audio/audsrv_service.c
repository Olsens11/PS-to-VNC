/*
 * File synopsis:
 * Binds the clean synchronous audio service boundary to PS2SDK AUDSRV. Exact
 * PCM format/volume values are forwarded without coercion, capacity is awaited
 * before playback, and a chunk is successful only when AUDSRV reports every
 * requested byte submitted.
 *
 * AUDSRV/LIBSD service lifetime is resident-ELF scoped. audsrv_init() may attach
 * to the already-resident service, and per-session retirement stops only the
 * current stream. This file deliberately never calls or exposes audsrv_quit().
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "audsrv_service.h"

#include <audsrv.h>

#include <limits.h>

static int pstvnc_audio_audsrv_initialize(void *context)
{
    (void)context;
    return audsrv_init() == 0 ? 0 : -1;
}

static int pstvnc_audio_audsrv_set_format(
    void *context,
    uint32_t rate_hz,
    uint32_t channels,
    uint32_t bits_per_sample)
{
    audsrv_fmt_t format;

    (void)context;
    if (rate_hz == 0u || rate_hz > (uint32_t)INT_MAX ||
        (channels != 1u && channels != 2u) ||
        (bits_per_sample != 8u && bits_per_sample != 16u))
        return -1;

    format.freq = (int)rate_hz;
    format.bits = (int)bits_per_sample;
    format.channels = (int)channels;
    return audsrv_set_format(&format) == 0 ? 0 : -1;
}

static int pstvnc_audio_audsrv_set_volume(
    void *context,
    uint32_t volume_percent)
{
    (void)context;
    if (volume_percent > 100u)
        return -1;

    return audsrv_set_volume((int)volume_percent) == 0 ? 0 : -1;
}

static int pstvnc_audio_audsrv_wait_audio(
    void *context,
    size_t byte_count)
{
    (void)context;
    if (byte_count == 0u || byte_count > (size_t)INT_MAX)
        return -1;

    return audsrv_wait_audio((int)byte_count) == 0 ? 0 : -1;
}

static int pstvnc_audio_audsrv_play_audio(
    void *context,
    const uint8_t *bytes,
    size_t byte_count)
{
    int submitted;

    (void)context;
    if (bytes == NULL || byte_count == 0u || byte_count > (size_t)INT_MAX)
        return -1;

    submitted = audsrv_play_audio((const char *)bytes, (int)byte_count);
    return submitted == (int)byte_count ? 0 : -1;
}

static int pstvnc_audio_audsrv_stop_audio(void *context)
{
    (void)context;
    return audsrv_stop_audio() == 0 ? 0 : -1;
}

pstvnc_audio_service_ops_t pstvnc_audio_audsrv_service_ops(void)
{
    pstvnc_audio_service_ops_t service;

    service.initialize = pstvnc_audio_audsrv_initialize;
    service.set_format = pstvnc_audio_audsrv_set_format;
    service.set_volume = pstvnc_audio_audsrv_set_volume;
    service.wait_audio = pstvnc_audio_audsrv_wait_audio;
    service.play_audio = pstvnc_audio_audsrv_play_audio;
    service.stop_audio = pstvnc_audio_audsrv_stop_audio;
    service.context = NULL;
    return service;
}

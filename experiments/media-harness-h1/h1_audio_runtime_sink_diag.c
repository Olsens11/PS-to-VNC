/*
 * File synopsis:
 * H1 diagnostic PCM consumer that preserves AUDIO queue consumption, credit
 * return, startup policy, shared-clock gating, thread priority, chunk size,
 * and approximately real-time PCM pacing while deliberately removing AUDSRV
 * and SPU2 playback from the combined AUDIO + MPEG workload.
 *
 * Purpose:
 *   Audio-only H1 is clean and fully qualified, while combined AUDIO + MPEG
 *   freezes the PS2. This diagnostic keeps the mux/network/queue/audio-thread
 *   load but replaces audsrv_wait_audio()/audsrv_play_audio() with a time delay
 *   derived from the configured PCM byte rate. A stable combined run therefore
 *   implicates the AUDSRV/SPU2 interaction; a freeze still present implicates
 *   the shared combined workload below actual audio playback.
 */

#include "h1_audio_runtime.h"

#include <delaythread.h>
#include <kernel.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define H1_AUDIO_STOP_WAIT_STEP_US 1000u
#define H1_AUDIO_STOP_WAIT_STEPS 3000u

static void h1_audio_record_error(
    pstvnc_h1_audio_runtime_t *runtime,
    pstvnc_h1_audio_error_t error)
{
    if (runtime != NULL && runtime->error == PSTVNC_H1_AUDIO_ERROR_NONE)
        runtime->error = error;
}

static void *h1_audio_allocate_aligned16(
    size_t byte_count,
    void **allocation)
{
    uintptr_t address;
    void *raw;

    if (allocation == NULL ||
        byte_count == 0u ||
        byte_count > ((size_t)-1 - 15u))
        return NULL;

    raw = malloc(byte_count + 15u);
    if (raw == NULL)
        return NULL;

    address = ((uintptr_t)raw + 15u) & ~(uintptr_t)15u;
    *allocation = raw;
    return (void *)address;
}

/* This diagnostic intentionally does not load LIBSD or AUDSRV. */
int pstvnc_h1_audio_load_modules_once(void)
{
    return 0;
}

static int h1_audio_wait_start_policy(
    pstvnc_h1_audio_runtime_t *runtime,
    const pstvnc_h1_config_t *config)
{
    if (config->audio_start_mode == PSTVNC_H1_AUDIO_START_IMMEDIATE)
        return 0;

    if (config->audio_start_mode == PSTVNC_H1_AUDIO_START_DELAY) {
        if (config->audio_start_delay_us == 0u)
            return 0;

        return DelayThread(config->audio_start_delay_us) < 0 ? -1 : 0;
    }

    while (!runtime->stop_requested) {
        if (pstvnc_h1_transport_last_error(runtime->transport) !=
            PSTVNC_H1_ERROR_NONE)
            return -1;

        if (pstvnc_h1_transport_audio_queue_size(runtime->transport) >=
            config->audio_start_target_bytes)
            return 0;

        if (runtime->transport->end_received)
            return -1;

        if (DelayThread(config->audio_idle_delay_us) < 0)
            return -1;
    }

    return -1;
}

static int h1_audio_sink_delay(
    const pstvnc_h1_config_t *config,
    size_t byte_count)
{
    uint64_t bits_per_second;
    uint64_t microseconds;

    bits_per_second =
        (uint64_t)config->audio_rate *
        (uint64_t)config->audio_channels *
        (uint64_t)config->audio_bits;

    if (bits_per_second == 0u)
        return -1;

    microseconds =
        ((uint64_t)byte_count * UINT64_C(8000000)) /
        bits_per_second;

    if (microseconds == 0u)
        microseconds = 1u;

    if (microseconds > UINT32_MAX)
        return -1;

    return DelayThread((unsigned int)microseconds) < 0 ? -1 : 0;
}

static void h1_audio_thread(void *argument)
{
    pstvnc_h1_audio_runtime_t *runtime =
        (pstvnc_h1_audio_runtime_t *)argument;
    const pstvnc_h1_config_t *config;
    int clock_result;

    config = pstvnc_h1_transport_config(runtime->transport);

    if (config == NULL || runtime->audio_buffer == NULL) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_TRANSPORT);
        goto done;
    }

    if (h1_audio_wait_start_policy(runtime, config) < 0) {
        if (!runtime->stop_requested)
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_DELAY);
        goto done;
    }

    clock_result = pstvnc_h1_media_clock_wait_offset(
        runtime->clock,
        pstvnc_h1_config_audio_offset_us(config),
        config->audio_idle_delay_us,
        &runtime->stop_requested);

    if (clock_result < 0) {
        if (!runtime->stop_requested)
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_CLOCK);
        goto done;
    }

    if (clock_result == 0)
        goto done;

    for (;;) {
        size_t bytes_read = 0u;
        int read_result;

        if (runtime->stop_requested)
            break;

        read_result = pstvnc_h1_transport_audio_read(
            runtime->transport,
            runtime->audio_buffer,
            config->audio_chunk_bytes,
            &bytes_read);

        if (read_result < 0) {
            if (!runtime->stop_requested)
                h1_audio_record_error(
                    runtime,
                    PSTVNC_H1_AUDIO_ERROR_TRANSPORT);
            break;
        }

        if (read_result == 0) {
            if (pstvnc_h1_transport_audio_exhausted(runtime->transport))
                break;

            if (DelayThread(config->audio_idle_delay_us) < 0) {
                h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_DELAY);
                break;
            }

            continue;
        }

        if (h1_audio_sink_delay(config, bytes_read) < 0) {
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_DELAY);
            break;
        }

        /* "played" means paced/retired by the sink in this diagnostic ELF. */
        pstvnc_h1_transport_record_audio_played(
            runtime->transport,
            bytes_read);
    }

done:
    runtime->finished = 1;
    ExitThread();
}

int pstvnc_h1_audio_start(
    pstvnc_h1_audio_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock)
{
    const pstvnc_h1_config_t *config;
    ee_thread_t thread;
    size_t chunk_bytes;
    size_t stack_bytes;

    if (runtime == NULL || transport == NULL || clock == NULL)
        return -1;

    config = pstvnc_h1_transport_config(transport);
    if (config == NULL || config->audio_mode != PSTVNC_H1_AUDIO_PCM)
        return -1;

    memset(runtime, 0, sizeof(*runtime));
    runtime->transport = transport;
    runtime->clock = clock;
    runtime->thread_id = -1;

    chunk_bytes = (size_t)config->audio_chunk_bytes;
    stack_bytes = (size_t)config->audio_thread_stack_size;

    if ((uint32_t)chunk_bytes != config->audio_chunk_bytes ||
        (uint32_t)stack_bytes != config->audio_thread_stack_size) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_ALLOCATION);
        goto fail;
    }

    runtime->audio_buffer = (unsigned char *)malloc(chunk_bytes);
    if (runtime->audio_buffer == NULL) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_ALLOCATION);
        goto fail;
    }

    runtime->thread_stack =
        (unsigned char *)h1_audio_allocate_aligned16(
            stack_bytes,
            &runtime->thread_stack_allocation);

    if (runtime->thread_stack == NULL) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_ALLOCATION);
        goto fail;
    }

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)h1_audio_thread;
    thread.stack = runtime->thread_stack;
    thread.stack_size = (int)config->audio_thread_stack_size;
    thread.gp_reg = &_gp;
    thread.initial_priority = (int)config->audio_thread_priority;
    thread.attr = 0;
    thread.option = 0;

    runtime->thread_id = CreateThread(&thread);
    if (runtime->thread_id < 0) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_THREAD);
        goto fail;
    }

    if (StartThread(runtime->thread_id, runtime) < 0) {
        (void)DeleteThread(runtime->thread_id);
        runtime->thread_id = -1;
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_THREAD);
        goto fail;
    }

    runtime->thread_started = 1;
    runtime->initialized = 1;
    return 0;

fail:
    free(runtime->thread_stack_allocation);
    free(runtime->audio_buffer);
    runtime->thread_stack_allocation = NULL;
    runtime->thread_stack = NULL;
    runtime->audio_buffer = NULL;
    runtime->transport = NULL;
    runtime->clock = NULL;
    return -1;
}

int pstvnc_h1_audio_shutdown(
    pstvnc_h1_audio_runtime_t *runtime)
{
    unsigned int wait_step;

    if (runtime == NULL)
        return -1;

    if (!runtime->initialized)
        return 0;

    runtime->stop_requested = 1;

    for (wait_step = 0u;
         runtime->thread_started && wait_step < H1_AUDIO_STOP_WAIT_STEPS;
         ++wait_step) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));

        if (ReferThreadStatus(runtime->thread_id, &status) < 0)
            return -1;

        if (status.status == THS_DORMANT) {
            if (DeleteThread(runtime->thread_id) < 0)
                return -1;

            runtime->thread_id = -1;
            runtime->thread_started = 0;
            break;
        }

        if (DelayThread(H1_AUDIO_STOP_WAIT_STEP_US) < 0)
            return -1;
    }

    if (runtime->thread_started)
        return -1;

    free(runtime->thread_stack_allocation);
    free(runtime->audio_buffer);
    runtime->thread_stack_allocation = NULL;
    runtime->thread_stack = NULL;
    runtime->audio_buffer = NULL;
    runtime->initialized = 0;
    runtime->transport = NULL;
    runtime->clock = NULL;
    return 0;
}

int pstvnc_h1_audio_finished(
    const pstvnc_h1_audio_runtime_t *runtime)
{
    return runtime != NULL && runtime->finished != 0;
}

pstvnc_h1_audio_error_t pstvnc_h1_audio_last_error(
    const pstvnc_h1_audio_runtime_t *runtime)
{
    return runtime == NULL ?
        PSTVNC_H1_AUDIO_ERROR_TRANSPORT : runtime->error;
}

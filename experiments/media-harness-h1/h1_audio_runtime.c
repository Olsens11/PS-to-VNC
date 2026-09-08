/*
 * File synopsis:
 * Implements H1's configurable PCM audio session consumer.
 *
 * Playback preserves Audio Transport EXP2's proven audsrv ordering:
 *   audsrv_wait_audio(bytes) -> audsrv_play_audio(bytes)
 *
 * H1 adds three session mechanisms only:
 *   - bytes come from the H1 mux AUDIO queue;
 *   - startup reservoir policy completes before waiting for the shared epoch;
 *   - MEDIA_END + empty AUDIO queue naturally retires the worker.
 */

#include "h1_audio_runtime.h"

#include <audsrv.h>
#include <delaythread.h>
#include <kernel.h>
#include <loadfile.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char AUDSRV_irx[];
extern unsigned int size_AUDSRV_irx;

#define H1_AUDIO_STOP_WAIT_STEP_US 1000u
#define H1_AUDIO_STOP_WAIT_STEPS 3000u

static int s_h1_audio_modules_loaded;

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

int pstvnc_h1_audio_load_modules_once(void)
{
    int result;

    if (s_h1_audio_modules_loaded)
        return 0;

    result = SifLoadModule("rom0:LIBSD", 0, NULL);
    if (result < 0)
        return -1;

    result = SifExecModuleBuffer(
        AUDSRV_irx,
        size_AUDSRV_irx,
        0,
        NULL,
        NULL);

    if (result < 0)
        return -1;

    s_h1_audio_modules_loaded = 1;
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

static void h1_audio_thread(void *argument)
{
    pstvnc_h1_audio_runtime_t *runtime =
        (pstvnc_h1_audio_runtime_t *)argument;
    const pstvnc_h1_config_t *config;
    struct audsrv_fmt_t format;
    int clock_result;

    config = pstvnc_h1_transport_config(runtime->transport);

    if (config == NULL || runtime->audio_buffer == NULL) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_TRANSPORT);
        goto done;
    }

    if (audsrv_init() != 0) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_AUDSRV_INIT);
        goto done;
    }

    format.freq = (int)config->audio_rate;
    format.bits = (int)config->audio_bits;
    format.channels = (int)config->audio_channels;

    if (audsrv_set_format(&format) != 0) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_AUDSRV_FORMAT);
        goto quit_audio;
    }

    if (audsrv_set_volume((int)config->audio_volume) != 0) {
        h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_AUDSRV_VOLUME);
        goto quit_audio;
    }

    if (h1_audio_wait_start_policy(runtime, config) < 0) {
        if (!runtime->stop_requested)
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_DELAY);
        goto quit_audio;
    }

    clock_result = pstvnc_h1_media_clock_wait_offset(
        runtime->clock,
        pstvnc_h1_config_audio_offset_us(config),
        config->audio_idle_delay_us,
        &runtime->stop_requested);

    if (clock_result < 0) {
        if (!runtime->stop_requested)
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_CLOCK);
        goto quit_audio;
    }

    if (clock_result == 0)
        goto quit_audio;

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

        if (audsrv_wait_audio((int)bytes_read) != 0) {
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_WAIT);
            break;
        }

        if (audsrv_play_audio(
                (const char *)runtime->audio_buffer,
                (int)bytes_read) < 0) {
            h1_audio_record_error(runtime, PSTVNC_H1_AUDIO_ERROR_PLAY);
            break;
        }

        pstvnc_h1_transport_record_audio_played(
            runtime->transport,
            bytes_read);
    }

quit_audio:
    (void)audsrv_stop_audio();
    (void)audsrv_quit();

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

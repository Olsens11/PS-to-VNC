/*
 * File synopsis:
 * Implements the experiment-only PS2 PCM consumer.
 *
 * Preserved hardware-tested playback shape:
 * 48000 Hz / 16-bit / stereo, 4096-byte consumer buffer,
 * audsrv_wait_audio() before audsrv_play_audio(), priority 65.
 */

#include "audio_runtime.h"

#include <audsrv.h>
#include <delaythread.h>
#include <kernel.h>
#include <loadfile.h>

#include <stddef.h>
#include <string.h>

extern unsigned char AUDSRV_irx[];
extern unsigned int size_AUDSRV_irx;

#define AUDIO_EXP_CHUNK_BYTES         4096u
#define AUDIO_EXP_THREAD_PRIORITY     65
#define AUDIO_EXP_IDLE_DELAY_US       1000u
#define AUDIO_EXP_STOP_WAIT_STEP_US   1000u
#define AUDIO_EXP_STOP_WAIT_STEPS     3000u

static void audio_exp_record_error(
    pstvnc_audio_exp_runtime_t *runtime,
    pstvnc_audio_exp_error_t error)
{
    if (runtime != NULL &&
        runtime->error == PSTVNC_AUDIO_EXP_ERROR_NONE)
        runtime->error = error;
}

static void audio_exp_thread(void *argument)
{
    pstvnc_audio_exp_runtime_t *runtime =
        (pstvnc_audio_exp_runtime_t *)argument;

    struct audsrv_fmt_t format;

    unsigned char audio_bytes[
        AUDIO_EXP_CHUNK_BYTES
    ];

    if (audsrv_init() != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_INIT);
        goto done;
    }

    format.freq =
        PSTVNC_TRANSPORT_PS2_AUDIO_RATE;

    format.bits = 16;
    format.channels = 2;

    if (audsrv_set_format(&format) != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_FORMAT);
        goto quit_audio;
    }

    if (audsrv_set_volume(100) != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_VOLUME);
        goto quit_audio;
    }

    while (!runtime->stop_requested) {
        size_t bytes_read = 0;

        int read_result =
            pstvnc_transport_runtime_audio_read(
                runtime->transport,
                audio_bytes,
                sizeof(audio_bytes),
                &bytes_read);

        if (read_result < 0) {
            if (!runtime->stop_requested) {
                audio_exp_record_error(
                    runtime,
                    PSTVNC_AUDIO_EXP_ERROR_TRANSPORT);
            }

            break;
        }

        if (read_result == 0) {
            if (DelayThread(
                    AUDIO_EXP_IDLE_DELAY_US) < 0) {

                audio_exp_record_error(
                    runtime,
                    PSTVNC_AUDIO_EXP_ERROR_DELAY);
                break;
            }

            continue;
        }

        if (audsrv_wait_audio(
                (int)bytes_read) != 0) {

            audio_exp_record_error(
                runtime,
                PSTVNC_AUDIO_EXP_ERROR_WAIT);
            break;
        }

        if (audsrv_play_audio(
                (const char *)audio_bytes,
                (int)bytes_read) < 0) {

            audio_exp_record_error(
                runtime,
                PSTVNC_AUDIO_EXP_ERROR_PLAY);
            break;
        }

        pstvnc_transport_runtime_record_audio_played(
            runtime->transport,
            bytes_read);
    }

quit_audio:
    (void)audsrv_stop_audio();
    (void)audsrv_quit();

done:
    ExitThread();
}

int pstvnc_audio_exp_runtime_start(
    pstvnc_audio_exp_runtime_t *runtime,
    pstvnc_transport_runtime_t *transport)
{
    ee_thread_t thread;

    if (runtime == NULL ||
        transport == NULL ||
        pstvnc_transport_runtime_socket_fd(
            transport) < 0)
        return -1;

    memset(runtime, 0, sizeof(*runtime));

    runtime->transport = transport;
    runtime->thread_id = -1;

    if (SifLoadModule(
            "rom0:LIBSD",
            0,
            NULL) < 0)
        return -1;

    if (SifExecModuleBuffer(
            AUDSRV_irx,
            size_AUDSRV_irx,
            0,
            NULL,
            NULL) < 0)
        return -1;

    memset(&thread, 0, sizeof(thread));

    thread.func = (void *)audio_exp_thread;
    thread.stack = (void *)runtime->thread_stack;
    thread.stack_size =
        (int)sizeof(runtime->thread_stack);
    thread.gp_reg = &_gp;
    thread.initial_priority =
        AUDIO_EXP_THREAD_PRIORITY;

    runtime->thread_id =
        CreateThread(&thread);

    if (runtime->thread_id < 0)
        return -1;

    if (StartThread(
            runtime->thread_id,
            runtime) < 0) {

        (void)DeleteThread(runtime->thread_id);
        runtime->thread_id = -1;
        return -1;
    }

    runtime->initialized = 1;
    runtime->thread_started = 1;

    return 0;
}

int pstvnc_audio_exp_runtime_shutdown(
    pstvnc_audio_exp_runtime_t *runtime)
{
    unsigned int wait_step;

    if (runtime == NULL)
        return -1;

    if (!runtime->initialized)
        return 0;

    runtime->stop_requested = 1;

    for (wait_step = 0;
         runtime->thread_started &&
         wait_step < AUDIO_EXP_STOP_WAIT_STEPS;
         wait_step++) {

        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));

        if (ReferThreadStatus(
                runtime->thread_id,
                &status) < 0)
            return -1;

        if (status.status == THS_DORMANT) {
            if (DeleteThread(runtime->thread_id) < 0)
                return -1;

            runtime->thread_id = -1;
            runtime->thread_started = 0;
            break;
        }

        if (DelayThread(
                AUDIO_EXP_STOP_WAIT_STEP_US) < 0)
            return -1;
    }

    if (runtime->thread_started)
        return -1;

    runtime->initialized = 0;
    runtime->transport = NULL;

    return 0;
}

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
#include <stdint.h>
#include <string.h>

extern unsigned char AUDSRV_irx[];
extern unsigned int size_AUDSRV_irx;

#define AUDIO_EXP_CHUNK_BYTES         4096u
#define AUDIO_EXP_THREAD_PRIORITY     65
#define AUDIO_EXP_IDLE_DELAY_US       1000u

/*
 * Test-only startup reservoir fill.
 *
 * 512 KiB at 192000 PCM bytes/second represents about 2.73 seconds.
 * Hold the consumer for 3 seconds after audsrv setup so the transport
 * receiver can actually populate the enlarged queue before playback starts.
 *
 * This is experimental setup, not proposed product playback policy.
 */
#define AUDIO_EXP_STARTUP_PREFILL_US  3000000u

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

/*
 * Experiment-only audio-worker breadcrumb.
 *
 * Wire format in the transport telemetry reserved word:
 *
 *   31..24  0xA1 diagnostic identity
 *   23..16  audio worker stage
 *   15..8   pstvnc_audio_exp_error_t
 *   7..0    rolling stage-generation counter
 *
 * The generation is intentionally only eight bits. It exists to distinguish
 * an actively revisiting stage from a worker frozen at one stage; wraparound
 * is harmless.
 */
typedef enum audio_exp_diag_stage {
    AUDIO_EXP_DIAG_THREAD_ENTER       = 0x01,
    AUDIO_EXP_DIAG_AUDSRV_READY       = 0x02,
    AUDIO_EXP_DIAG_FORMAT_READY       = 0x03,
    AUDIO_EXP_DIAG_VOLUME_READY       = 0x04,
    AUDIO_EXP_DIAG_PREFILL_ENTER      = 0x05,
    AUDIO_EXP_DIAG_PREFILL_RETURN     = 0x06,

    AUDIO_EXP_DIAG_LOOP_TOP           = 0x10,
    AUDIO_EXP_DIAG_STOP_REQUESTED     = 0x11,
    AUDIO_EXP_DIAG_READ_ENTER         = 0x12,
    AUDIO_EXP_DIAG_READ_IDLE          = 0x13,
    AUDIO_EXP_DIAG_DELAY_ENTER        = 0x14,
    AUDIO_EXP_DIAG_DELAY_RETURN       = 0x15,
    AUDIO_EXP_DIAG_READ_DATA          = 0x16,

    AUDIO_EXP_DIAG_WAIT_ENTER         = 0x20,
    AUDIO_EXP_DIAG_WAIT_RETURN        = 0x21,
    AUDIO_EXP_DIAG_PLAY_ENTER         = 0x22,
    AUDIO_EXP_DIAG_PLAY_RETURN        = 0x23,
    AUDIO_EXP_DIAG_RECORDED           = 0x24,

    AUDIO_EXP_DIAG_TRANSPORT_ERROR    = 0x30,
    AUDIO_EXP_DIAG_WAIT_ERROR         = 0x31,
    AUDIO_EXP_DIAG_PLAY_ERROR         = 0x32,
    AUDIO_EXP_DIAG_DELAY_ERROR        = 0x33,

    AUDIO_EXP_DIAG_AUDSRV_INIT_ERROR  = 0x40,
    AUDIO_EXP_DIAG_FORMAT_ERROR       = 0x41,
    AUDIO_EXP_DIAG_VOLUME_ERROR       = 0x42,

    AUDIO_EXP_DIAG_STOP_AUDIO_ENTER   = 0x50,
    AUDIO_EXP_DIAG_STOP_AUDIO_RETURN  = 0x51,
    AUDIO_EXP_DIAG_QUIT_ENTER         = 0x52,
    AUDIO_EXP_DIAG_QUIT_RETURN        = 0x53,
    AUDIO_EXP_DIAG_EXIT_THREAD        = 0x54
} audio_exp_diag_stage_t;

static void audio_exp_publish_diag(
    pstvnc_audio_exp_runtime_t *runtime,
    uint8_t *generation,
    audio_exp_diag_stage_t stage)
{
    uint32_t diagnostic_word;

    if (runtime == NULL ||
        runtime->transport == NULL ||
        generation == NULL)
        return;

    *generation =
        (uint8_t)(*generation + 1u);

    diagnostic_word =
        0xA1000000u |
        (((uint32_t)stage & 0xffu) << 16) |
        (((uint32_t)runtime->error & 0xffu) << 8) |
        (uint32_t)(*generation);

    pstvnc_transport_runtime_set_diagnostic_word(
        runtime->transport,
        diagnostic_word);
}

static void audio_exp_thread(void *argument)
{
    pstvnc_audio_exp_runtime_t *runtime =
        (pstvnc_audio_exp_runtime_t *)argument;

    struct audsrv_fmt_t format;

    unsigned char audio_bytes[
        AUDIO_EXP_CHUNK_BYTES
    ];

    uint8_t diagnostic_generation = 0;

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_THREAD_ENTER);

    if (audsrv_init() != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_INIT);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_AUDSRV_INIT_ERROR);

        goto done;
    }

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_AUDSRV_READY);

    format.freq =
        PSTVNC_TRANSPORT_PS2_AUDIO_RATE;

    format.bits = 16;
    format.channels = 2;

    if (audsrv_set_format(&format) != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_FORMAT);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_FORMAT_ERROR);

        goto quit_audio;
    }

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_FORMAT_READY);

    if (audsrv_set_volume(100) != 0) {
        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_AUDSRV_VOLUME);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_VOLUME_ERROR);

        goto quit_audio;
    }

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_VOLUME_READY);

    /*
     * Deliberately prefill the enlarged experimental audio reservoir before
     * the consumer begins draining it. Without this pause, merely increasing
     * capacity would not prove anything: a real-time producer and real-time
     * consumer would begin together and the extra capacity could remain empty.
     */
    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_PREFILL_ENTER);

    if (DelayThread(
            AUDIO_EXP_STARTUP_PREFILL_US) < 0) {

        audio_exp_record_error(
            runtime,
            PSTVNC_AUDIO_EXP_ERROR_DELAY);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_DELAY_ERROR);

        goto quit_audio;
    }

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_PREFILL_RETURN);

    for (;;) {
        size_t bytes_read = 0;
        int read_result;

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_LOOP_TOP);

        if (runtime->stop_requested) {
            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_STOP_REQUESTED);
            break;
        }

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_READ_ENTER);

        read_result =
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

            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_TRANSPORT_ERROR);

            break;
        }

        if (read_result == 0) {
            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_READ_IDLE);

            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_DELAY_ENTER);

            if (DelayThread(
                    AUDIO_EXP_IDLE_DELAY_US) < 0) {

                audio_exp_record_error(
                    runtime,
                    PSTVNC_AUDIO_EXP_ERROR_DELAY);

                audio_exp_publish_diag(
                    runtime,
                    &diagnostic_generation,
                    AUDIO_EXP_DIAG_DELAY_ERROR);

                break;
            }

            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_DELAY_RETURN);

            continue;
        }

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_READ_DATA);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_WAIT_ENTER);

        if (audsrv_wait_audio(
                (int)bytes_read) != 0) {

            audio_exp_record_error(
                runtime,
                PSTVNC_AUDIO_EXP_ERROR_WAIT);

            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_WAIT_ERROR);

            break;
        }

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_WAIT_RETURN);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_PLAY_ENTER);

        if (audsrv_play_audio(
                (const char *)audio_bytes,
                (int)bytes_read) < 0) {

            audio_exp_record_error(
                runtime,
                PSTVNC_AUDIO_EXP_ERROR_PLAY);

            audio_exp_publish_diag(
                runtime,
                &diagnostic_generation,
                AUDIO_EXP_DIAG_PLAY_ERROR);

            break;
        }

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_PLAY_RETURN);

        pstvnc_transport_runtime_record_audio_played(
            runtime->transport,
            bytes_read);

        audio_exp_publish_diag(
            runtime,
            &diagnostic_generation,
            AUDIO_EXP_DIAG_RECORDED);
    }

quit_audio:
    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_STOP_AUDIO_ENTER);

    (void)audsrv_stop_audio();

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_STOP_AUDIO_RETURN);

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_QUIT_ENTER);

    (void)audsrv_quit();

    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_QUIT_RETURN);

done:
    audio_exp_publish_diag(
        runtime,
        &diagnostic_generation,
        AUDIO_EXP_DIAG_EXIT_THREAD);

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

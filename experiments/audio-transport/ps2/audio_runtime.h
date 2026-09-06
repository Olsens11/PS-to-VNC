/*
 * File synopsis:
 * Defines the experiment-only PS2 audio consumer that preserves the proven
 * audsrv/SPU2 playback mechanism while sourcing PCM from the common AUDIO
 * transport channel instead of a dedicated TCP socket.
 */

#ifndef PSTVNC_EXPERIMENT_PS2_AUDIO_RUNTIME_H
#define PSTVNC_EXPERIMENT_PS2_AUDIO_RUNTIME_H

#include "transport_runtime.h"

#define PSTVNC_AUDIO_EXP_THREAD_STACK_SIZE 16384u

typedef enum pstvnc_audio_exp_error {
    PSTVNC_AUDIO_EXP_ERROR_NONE = 0,
    PSTVNC_AUDIO_EXP_ERROR_MODULE,
    PSTVNC_AUDIO_EXP_ERROR_THREAD,
    PSTVNC_AUDIO_EXP_ERROR_AUDSRV_INIT,
    PSTVNC_AUDIO_EXP_ERROR_AUDSRV_FORMAT,
    PSTVNC_AUDIO_EXP_ERROR_AUDSRV_VOLUME,
    PSTVNC_AUDIO_EXP_ERROR_TRANSPORT,
    PSTVNC_AUDIO_EXP_ERROR_WAIT,
    PSTVNC_AUDIO_EXP_ERROR_PLAY,
    PSTVNC_AUDIO_EXP_ERROR_DELAY
} pstvnc_audio_exp_error_t;

typedef struct pstvnc_audio_exp_runtime {
    pstvnc_transport_runtime_t *transport;

    int thread_id;
    int thread_started;
    int initialized;

    volatile int stop_requested;
    volatile pstvnc_audio_exp_error_t error;

    unsigned char thread_stack[
        PSTVNC_AUDIO_EXP_THREAD_STACK_SIZE
    ] __attribute__((aligned(16)));
} pstvnc_audio_exp_runtime_t;

int pstvnc_audio_exp_runtime_start(
    pstvnc_audio_exp_runtime_t *runtime,
    pstvnc_transport_runtime_t *transport);

int pstvnc_audio_exp_runtime_shutdown(
    pstvnc_audio_exp_runtime_t *runtime);

#endif

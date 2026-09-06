/*
 * File synopsis:
 * Defines configurable Audio Transport EXP2's audsrv PCM consumer.
 *
 * The consumer buffer and EE thread stack are runtime allocations sized by the
 * accepted Pi profile rather than fixed backing arrays.
 */

#ifndef PSTVNC_EXPERIMENT_PS2_AUDIO_RUNTIME_EXP2_H
#define PSTVNC_EXPERIMENT_PS2_AUDIO_RUNTIME_EXP2_H

#include "transport_runtime_exp2.h"

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
    PSTVNC_AUDIO_EXP_ERROR_DELAY,
    PSTVNC_AUDIO_EXP_ERROR_ALLOCATION
} pstvnc_audio_exp_error_t;

typedef struct pstvnc_audio_exp_runtime {
    pstvnc_transport_runtime_t *transport;

    int thread_id;
    int thread_started;
    int initialized;

    volatile int stop_requested;
    volatile pstvnc_audio_exp_error_t error;

    unsigned char *audio_buffer;

    void *thread_stack_allocation;
    unsigned char *thread_stack;
} pstvnc_audio_exp_runtime_t;

int pstvnc_audio_exp_runtime_start(
    pstvnc_audio_exp_runtime_t *runtime,
    pstvnc_transport_runtime_t *transport);

int pstvnc_audio_exp_runtime_shutdown(
    pstvnc_audio_exp_runtime_t *runtime);

#endif

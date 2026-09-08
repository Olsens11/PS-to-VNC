/*
 * File synopsis:
 * Defines H1's configurable PCM audio session consumer.
 *
 * The playback mechanism descends from Audio Transport EXP2. H1 changes the
 * byte source to the H1 mux AUDIO queue and gates first presentation on the
 * shared media epoch so audio/video offsets can be measured independently of
 * reservoir depth.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_AUDIO_RUNTIME_H
#define PSTVNC_MEDIA_HARNESS_H1_AUDIO_RUNTIME_H

#include "h1_media_clock.h"
#include "h1_transport_runtime.h"

typedef enum pstvnc_h1_audio_error {
    PSTVNC_H1_AUDIO_ERROR_NONE = 0,
    PSTVNC_H1_AUDIO_ERROR_MODULE = 1,
    PSTVNC_H1_AUDIO_ERROR_THREAD = 2,
    PSTVNC_H1_AUDIO_ERROR_AUDSRV_INIT = 3,
    PSTVNC_H1_AUDIO_ERROR_AUDSRV_FORMAT = 4,
    PSTVNC_H1_AUDIO_ERROR_AUDSRV_VOLUME = 5,
    PSTVNC_H1_AUDIO_ERROR_TRANSPORT = 6,
    PSTVNC_H1_AUDIO_ERROR_WAIT = 7,
    PSTVNC_H1_AUDIO_ERROR_PLAY = 8,
    PSTVNC_H1_AUDIO_ERROR_DELAY = 9,
    PSTVNC_H1_AUDIO_ERROR_ALLOCATION = 10,
    PSTVNC_H1_AUDIO_ERROR_CLOCK = 11
} pstvnc_h1_audio_error_t;

typedef struct pstvnc_h1_audio_runtime {
    pstvnc_h1_transport_runtime_t *transport;
    pstvnc_h1_media_clock_t *clock;

    int thread_id;
    int thread_started;
    int initialized;

    volatile int stop_requested;
    volatile int finished;
    volatile pstvnc_h1_audio_error_t error;

    unsigned char *audio_buffer;
    void *thread_stack_allocation;
    unsigned char *thread_stack;
} pstvnc_h1_audio_runtime_t;

int pstvnc_h1_audio_load_modules_once(void);

int pstvnc_h1_audio_start(
    pstvnc_h1_audio_runtime_t *runtime,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock);

int pstvnc_h1_audio_shutdown(
    pstvnc_h1_audio_runtime_t *runtime);

int pstvnc_h1_audio_finished(
    const pstvnc_h1_audio_runtime_t *runtime);

pstvnc_h1_audio_error_t pstvnc_h1_audio_last_error(
    const pstvnc_h1_audio_runtime_t *runtime);

#endif

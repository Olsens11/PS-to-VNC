/*
 * File synopsis:
 * Defines CP2P's exact-generation MPEG decode worker. The worker owns
 * only thread/decoder/pixel lifetime; session coordinator owns MPEG
 * presentation-generation state.
 */
#ifndef PSTVNC_H1_CP2P_MPEG_WORKER_H
#define PSTVNC_H1_CP2P_MPEG_WORKER_H

#include "h1_media_clock.h"
#include "h1_transport_runtime.h"
#include "h1_video_runtime.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

typedef struct pstvnc_h1_cp2p_mpeg_worker {
    pstvnc_h1_transport_runtime_t *transport;
    pstvnc_h1_media_clock_t *clock;
    pstvnc_h1_mpeg_start_handoff_t *handoff;
    pstvnc_h1_mpeg_start_contract_t contract;
    pstvnc_h1_video_result_t result;
    int thread_id;
    int thread_started;
    int initialized;
    int live_decode;
    volatile int stop_requested;
    volatile int finished;
    volatile int run_result;
    volatile int failure_latched;
    void *thread_stack_allocation;
    unsigned char *thread_stack;
    uint32_t generation;
    unsigned armed : 1;
} pstvnc_h1_cp2p_mpeg_worker_t;

int pstvnc_h1_cp2p_mpeg_worker_init(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock);

int pstvnc_h1_cp2p_mpeg_worker_arm(
    void *context,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_h1_mpeg_start_contract_t *contract);

int pstvnc_h1_cp2p_mpeg_worker_clear(
    void *context,
    uint32_t generation);

int pstvnc_h1_cp2p_mpeg_worker_shutdown(
    pstvnc_h1_cp2p_mpeg_worker_t *worker);

#endif

/*
 * File synopsis:
 * Implements the CP2P generation-bound MPEG decode worker. One optional EE
 * decoder thread consumes the existing transport MPEG queue; no socket,
 * recv owner, queue, sequence owner, or presentation owner is added.
 *
 * The disposable stall-diagnostic derivative writes observation-only stage
 * values to the existing transport diagnostic_word around worker lifecycle
 * boundaries. Thread priority and all runtime policy remain unchanged.
 */
#include "h1_cp2p_mpeg_worker.h"

#include "h1_cumulative39_graphics.h"
#include "h1_video_runtime_cp2p.h"

#include <delaythread.h>
#include <kernel.h>
#include <malloc.h>
#include <string.h>

#define H1_CP2P_MPEG_WORKER_STACK_BYTES (64u * 1024u)
#define H1_CP2P_MPEG_WORKER_STOP_POLL_US 1000u
#define H1_CP2P_MPEG_WORKER_STOP_MAX_LOOPS 3000u

static void h1_cp2p_mpeg_diag(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    uint32_t stage)
{
    if (worker != NULL && worker->transport != NULL)
        pstvnc_h1_transport_set_diagnostic_word(worker->transport, stage);
}

static unsigned char *h1_worker_align64(void *allocation)
{
    uintptr_t value = (uintptr_t)allocation;
    value = (value + 63u) & ~(uintptr_t)63u;
    return (unsigned char *)value;
}

static void h1_cp2p_mpeg_worker_thread(void *argument)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)argument;

    h1_cp2p_mpeg_diag(worker, 0xD1000001u); /* worker entered */
    worker->run_result = pstvnc_h1_video_run_cp2p_session(
        worker->transport,
        worker->clock,
        &worker->result,
        worker->handoff,
        &worker->contract,
        &worker->stop_requested);
    h1_cp2p_mpeg_diag(worker, 0xD1000002u); /* video runtime returned */

    if (worker->run_result < 0)
        worker->failure_latched = 1;
    worker->finished = 1;
    h1_cp2p_mpeg_diag(worker, 0xD1000003u); /* worker finished */
    ExitThread();
}

static int h1_cp2p_mpeg_worker_join(
    pstvnc_h1_cp2p_mpeg_worker_t *worker)
{
    unsigned int loops = 0u;

    if (!worker->thread_started)
        return 1;

    worker->stop_requested = 1;
    while (!worker->finished && loops < H1_CP2P_MPEG_WORKER_STOP_MAX_LOOPS) {
        ee_thread_status_t status;
        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(worker->thread_id, &status) < 0)
            return 0;
        if (status.status == THS_DORMANT) {
            worker->finished = 1;
            break;
        }
        if (DelayThread(H1_CP2P_MPEG_WORKER_STOP_POLL_US) < 0)
            return 0;
        loops++;
    }

    if (!worker->finished)
        return 0;
    if (DeleteThread(worker->thread_id) < 0)
        return 0;

    worker->thread_started = 0;
    worker->thread_id = -1;
    free(worker->thread_stack_allocation);
    worker->thread_stack_allocation = NULL;
    worker->thread_stack = NULL;
    return 1;
}

int pstvnc_h1_cp2p_mpeg_worker_init(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock)
{
    if (worker == NULL || transport == NULL || clock == NULL)
        return 0;
    memset(worker, 0, sizeof(*worker));
    worker->transport = transport;
    worker->clock = clock;
    worker->thread_id = -1;
    worker->initialized = 1;
    h1_cp2p_mpeg_diag(worker, 0xD1000010u); /* worker initialized */
    return 1;
}

int pstvnc_h1_cp2p_mpeg_worker_arm(
    void *context,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)context;
    const pstvnc_h1_config_t *config;
    ee_thread_t thread;

    if (worker == NULL || !worker->initialized || handoff == NULL ||
        contract == NULL || contract->generation == 0u || worker->armed)
        return 0;
    if (pstvnc_h1_mpeg_presentation_owner_state(&handoff->owner) !=
            PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME ||
        handoff->owner.generation != contract->generation)
        return 0;

    config = pstvnc_h1_transport_config(worker->transport);
    if (config == NULL)
        return 0;

    worker->handoff = handoff;
    worker->contract = *contract;
    worker->generation = contract->generation;
    worker->stop_requested = 0;
    worker->finished = 0;
    worker->run_result = 0;
    worker->live_decode = 0;
    worker->armed = 1;
    h1_cp2p_mpeg_diag(worker, 0xD1000011u); /* arm accepted */

    /* Item #10 owns public MPEG activation. OFF is an armed dormant worker. */
    if (config->video_mode == PSTVNC_H1_VIDEO_OFF)
        return 1;
    if (config->video_mode != PSTVNC_H1_VIDEO_MPEG2_ES)
        goto fail;

    h1_cp2p_mpeg_diag(worker, 0xD1000012u); /* before chassis init */
    if (pstvnc_h1_video_chassis_init() < 0)
        goto fail;
    h1_cp2p_mpeg_diag(worker, 0xD1000013u); /* chassis init returned */

    worker->thread_stack_allocation =
        malloc(H1_CP2P_MPEG_WORKER_STACK_BYTES + 63u);
    if (worker->thread_stack_allocation == NULL)
        goto fail;
    worker->thread_stack = h1_worker_align64(worker->thread_stack_allocation);

    memset(&thread, 0, sizeof(thread));
    thread.func = h1_cp2p_mpeg_worker_thread;
    thread.stack = worker->thread_stack;
    thread.stack_size = H1_CP2P_MPEG_WORKER_STACK_BYTES;
    thread.gp_reg = &_gp;
    thread.initial_priority = 42;
    worker->thread_id = CreateThread(&thread);
    if (worker->thread_id < 0)
        goto fail;

    h1_cp2p_mpeg_diag(worker, 0xD1000014u); /* before StartThread */
    if (StartThread(worker->thread_id, worker) < 0) {
        (void)DeleteThread(worker->thread_id);
        worker->thread_id = -1;
        goto fail;
    }
    worker->thread_started = 1;
    worker->live_decode = 1;
    return 1;

fail:
    h1_cp2p_mpeg_diag(worker, 0xD1FF0001u);
    free(worker->thread_stack_allocation);
    worker->thread_stack_allocation = NULL;
    worker->thread_stack = NULL;
    worker->handoff = NULL;
    worker->generation = 0u;
    worker->armed = 0;
    return 0;
}

int pstvnc_h1_cp2p_mpeg_worker_clear(void *context, uint32_t generation)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)context;

    if (worker == NULL || !worker->initialized || !worker->armed ||
        generation == 0u || worker->generation != generation)
        return 0;
    if (!h1_cp2p_mpeg_worker_join(worker))
        return 0;

    if (worker->live_decode && pstvnc_h1_graphics_clear_video() < 0)
        return 0;

    memset(&worker->contract, 0, sizeof(worker->contract));
    worker->handoff = NULL;
    worker->generation = 0u;
    worker->live_decode = 0;
    worker->armed = 0;
    return 1;
}

int pstvnc_h1_cp2p_mpeg_worker_shutdown(
    pstvnc_h1_cp2p_mpeg_worker_t *worker)
{
    if (worker == NULL || !worker->initialized)
        return -1;
    if (worker->armed &&
        !pstvnc_h1_cp2p_mpeg_worker_clear(worker, worker->generation))
        return -1;
    worker->transport = NULL;
    worker->clock = NULL;
    worker->initialized = 0;
    return 0;
}

/*
 * File synopsis:
 * Implements the CP2P generation-bound MPEG decode worker. One optional EE
 * decoder thread consumes the existing transport MPEG queue; no socket,
 * recv owner, queue, sequence owner, or presentation owner is added.
 *
 * The disposable stall-diagnostic derivative writes observation-only stage
 * values to the existing transport diagnostic_word around worker lifecycle
 * boundaries. This scheduling diagnostic changes only the MPEG worker's
 * initial priority from 42 to 67; all other runtime policy is unchanged.
 */
#include "h1_cp2p_mpeg_worker.h"
#include "h1_transport_runtime.h"

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


#define H1_CP2P_MPEG_JOIN_DIAG_JOIN_ENTER       0xE2030001u
#define H1_CP2P_MPEG_JOIN_DIAG_STOP_REQUESTED   0xE2030002u
#define H1_CP2P_MPEG_JOIN_DIAG_REFER_ENTER      0xE2030003u
#define H1_CP2P_MPEG_JOIN_DIAG_REFER_RETURN     0xE2030004u
#define H1_CP2P_MPEG_JOIN_DIAG_DORMANT          0xE2030005u
#define H1_CP2P_MPEG_JOIN_DIAG_DELAY_ENTER      0xE2030006u
#define H1_CP2P_MPEG_JOIN_DIAG_DELAY_RETURN     0xE2030007u
#define H1_CP2P_MPEG_JOIN_DIAG_LOOP_EXHAUSTED   0xE2030008u
#define H1_CP2P_MPEG_JOIN_DIAG_FINISHED         0xE2030009u
#define H1_CP2P_MPEG_JOIN_DIAG_DELETE_ENTER     0xE203000Au
#define H1_CP2P_MPEG_JOIN_DIAG_DELETE_RETURN    0xE203000Bu
#define H1_CP2P_MPEG_JOIN_DIAG_JOIN_RETURN      0xE203000Cu
#define H1_CP2P_MPEG_JOIN_DIAG_NO_THREAD        0xE203000Du

#define H1_CP2P_MPEG_JOIN_DIAG_REFER_FAIL       0xE20300F1u
#define H1_CP2P_MPEG_JOIN_DIAG_DELAY_FAIL       0xE20300F2u
#define H1_CP2P_MPEG_JOIN_DIAG_TIMEOUT_FAIL     0xE20300F3u
#define H1_CP2P_MPEG_JOIN_DIAG_DELETE_FAIL      0xE20300F4u

/*
 * E204 explicit join-call witness.
 *
 * E203 stored detailed stages in mpeg_diag_stage, but a blocked join can
 * prevent any later ordinary telemetry snapshot from exporting that value.
 * E204 therefore emits an explicit telemetry snapshot immediately before
 * each potentially blocking EE kernel call and at terminal/error boundaries.
 *
 * This is diagnostic-only instrumentation.  It deliberately perturbs the
 * 1-ms polling loop more than E203 in exchange for hardware-visible
 * classification of the blocking call.
 */
#define H1_CP2P_MPEG_JOIN_WITNESS_JOIN_ENTER      0xE2040001u
#define H1_CP2P_MPEG_JOIN_WITNESS_STOP_REQUESTED  0xE2040002u
#define H1_CP2P_MPEG_JOIN_WITNESS_REFER_ENTER     0xE2040003u
#define H1_CP2P_MPEG_JOIN_WITNESS_DORMANT         0xE2040004u
#define H1_CP2P_MPEG_JOIN_WITNESS_DELAY_ENTER     0xE2040005u
#define H1_CP2P_MPEG_JOIN_WITNESS_FINISHED        0xE2040006u
#define H1_CP2P_MPEG_JOIN_WITNESS_DELETE_ENTER    0xE2040007u
#define H1_CP2P_MPEG_JOIN_WITNESS_JOIN_RETURN     0xE2040008u
#define H1_CP2P_MPEG_JOIN_WITNESS_NO_THREAD       0xE204000Du

#define H1_CP2P_MPEG_JOIN_WITNESS_REFER_FAIL      0xE20400F1u
#define H1_CP2P_MPEG_JOIN_WITNESS_DELAY_FAIL      0xE20400F2u
#define H1_CP2P_MPEG_JOIN_WITNESS_TIMEOUT_FAIL    0xE20400F3u
#define H1_CP2P_MPEG_JOIN_WITNESS_DELETE_FAIL     0xE20400F4u

/*
 * Observation-only MPEG join witness.
 *
 * Do not send a telemetry snapshot here: this function executes inside the
 * 1-ms join polling loop.  Repeated synchronous telemetry sends would alter
 * the scheduling behavior being diagnosed.  The existing persistent/heartbeat
 * telemetry paths observe mpeg_diag_stage asynchronously.
 */
static void h1_cp2p_mpeg_join_diag(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    uint32_t stage)
{
    if (worker != NULL && worker->transport != NULL)
        worker->transport->mpeg_diag_stage = stage;
}

/*
 * E204 hardware-visible blocking-call witness.
 *
 * The snapshot return value is intentionally observation-only.  Diagnostic
 * transport failure must not rewrite the MPEG worker join's functional
 * success/failure policy.
 */
static void h1_cp2p_mpeg_join_witness(
    pstvnc_h1_cp2p_mpeg_worker_t *worker,
    uint32_t stage)
{
    h1_cp2p_mpeg_join_diag(worker, stage);

    if (worker != NULL && worker->transport != NULL)
        (void)pstvnc_h1_transport_send_telemetry_snapshot(worker->transport);
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

    h1_cp2p_mpeg_join_witness(
        worker,
        H1_CP2P_MPEG_JOIN_WITNESS_JOIN_ENTER);

    if (!worker->thread_started) {
        h1_cp2p_mpeg_join_witness(
            worker,
            H1_CP2P_MPEG_JOIN_WITNESS_NO_THREAD);
        return 1;
    }

    worker->stop_requested = 1;

    h1_cp2p_mpeg_join_witness(
        worker,
        H1_CP2P_MPEG_JOIN_WITNESS_STOP_REQUESTED);

    while (!worker->finished &&
           loops < H1_CP2P_MPEG_WORKER_STOP_MAX_LOOPS) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));

        /*
         * If this snapshot reaches the Pi and no DELAY_ENTER follows,
         * ReferThreadStatus() is the blocking boundary.
         */
        h1_cp2p_mpeg_join_witness(
            worker,
            H1_CP2P_MPEG_JOIN_WITNESS_REFER_ENTER);

        if (ReferThreadStatus(
                worker->thread_id,
                &status) < 0) {
            h1_cp2p_mpeg_join_witness(
                worker,
                H1_CP2P_MPEG_JOIN_WITNESS_REFER_FAIL);
            return 0;
        }

        if (status.status == THS_DORMANT) {
            worker->finished = 1;

            h1_cp2p_mpeg_join_witness(
                worker,
                H1_CP2P_MPEG_JOIN_WITNESS_DORMANT);

            break;
        }

        /*
         * If this snapshot reaches the Pi and no later REFER_ENTER
         * follows, DelayThread() is the blocking boundary.
         */
        h1_cp2p_mpeg_join_witness(
            worker,
            H1_CP2P_MPEG_JOIN_WITNESS_DELAY_ENTER);

        if (DelayThread(
                H1_CP2P_MPEG_WORKER_STOP_POLL_US) < 0) {
            h1_cp2p_mpeg_join_witness(
                worker,
                H1_CP2P_MPEG_JOIN_WITNESS_DELAY_FAIL);
            return 0;
        }

        loops++;
    }

    if (!worker->finished) {
        h1_cp2p_mpeg_join_witness(
            worker,
            H1_CP2P_MPEG_JOIN_WITNESS_TIMEOUT_FAIL);
        return 0;
    }

    h1_cp2p_mpeg_join_witness(
        worker,
        H1_CP2P_MPEG_JOIN_WITNESS_FINISHED);

    /*
     * DeleteThread() is the only remaining EE kernel call in the join.
     */
    h1_cp2p_mpeg_join_witness(
        worker,
        H1_CP2P_MPEG_JOIN_WITNESS_DELETE_ENTER);

    if (DeleteThread(worker->thread_id) < 0) {
        h1_cp2p_mpeg_join_witness(
            worker,
            H1_CP2P_MPEG_JOIN_WITNESS_DELETE_FAIL);
        return 0;
    }

    worker->thread_started = 0;
    worker->thread_id = -1;
    free(worker->thread_stack_allocation);
    worker->thread_stack_allocation = NULL;
    worker->thread_stack = NULL;

    h1_cp2p_mpeg_join_witness(
        worker,
        H1_CP2P_MPEG_JOIN_WITNESS_JOIN_RETURN);

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
    thread.initial_priority = 67;
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


int pstvnc_h1_cp2p_mpeg_worker_request_stop(
    void *context,
    uint32_t generation)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)context;

    if (worker == NULL || !worker->initialized || !worker->armed ||
        generation == 0u || worker->generation != generation)
        return 0;

    /*
     * This is only a lifecycle request. The CP2P video runtime deliberately
     * does not expose this flag to libmpeg's data callback.
     */
    worker->stop_requested = 1;
    return 1;
}

int pstvnc_h1_cp2p_mpeg_worker_stop_poll(
    void *context,
    uint32_t generation)
{
    pstvnc_h1_cp2p_mpeg_worker_t *worker =
        (pstvnc_h1_cp2p_mpeg_worker_t *)context;

    if (worker == NULL || !worker->initialized || !worker->armed ||
        generation == 0u || worker->generation != generation)
        return -1;

    if (!worker->thread_started)
        return 1;

    if (worker->failure_latched)
        return -1;

    if (!worker->finished)
        return 0;

    return worker->run_result < 0 ? -1 : 1;
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

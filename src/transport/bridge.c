/*
 * File synopsis:
 * Implements Transport's one cross-component bridge body. It coordinates the
 * application-owned session lifecycle and adapts logical RFB plus optional
 * AUDIO/MPEG2 delivery to the private Transport runtime without exposing the
 * physical PSTV descriptor or moving protocol/media policy into Transport.
 *
 * One active bridge still means one physical connection and one sole receiver.
 * Opaque rider access is admitted under a process-lifetime bridge gate and
 * counted until return, so a dead session's singleton runtime cannot be
 * released/reinitialized while previously admitted work still owns it.
 * RFB safe-boundary choice, PCM playback, MPEG decoding, media-clock use,
 * exact-generation orchestration, and presentation remain outside this bridge.
 *
 * Context: docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md; docs/ledge/
 * LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#include "bridge.h"
#include "runtime.h"

#include <string.h>

#if defined(_EE)
#include <kernel.h>
#endif

static pstvnc_transport_runtime_t pstvnc_transport_bridge_runtime;
static int pstvnc_transport_bridge_session_active;
static uint32_t pstvnc_transport_bridge_last_ticket;
static uint32_t pstvnc_transport_bridge_active_ticket;
static uint32_t pstvnc_transport_bridge_admitted_calls;

static pstvnc_transport_result_t pstvnc_transport_bridge_access_result(
    const pstvnc_transport_access_t *transport_access);

#if defined(_EE)
static int pstvnc_transport_bridge_gate_semaphore_id = -1;
#endif

static int pstvnc_transport_bridge_gate_initialize(void)
{
#if defined(_EE)
    ee_sema_t semaphore;

    if (pstvnc_transport_bridge_gate_semaphore_id >= 0)
        return 1;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    pstvnc_transport_bridge_gate_semaphore_id = CreateSema(&semaphore);
    return pstvnc_transport_bridge_gate_semaphore_id >= 0;
#else
    return 1;
#endif
}

static int pstvnc_transport_bridge_gate_lock(void)
{
#if defined(_EE)
    if (pstvnc_transport_bridge_gate_semaphore_id < 0)
        return 0;

    return WaitSema(pstvnc_transport_bridge_gate_semaphore_id) >= 0;
#else
    return 1;
#endif
}

static int pstvnc_transport_bridge_gate_unlock(void)
{
#if defined(_EE)
    if (pstvnc_transport_bridge_gate_semaphore_id < 0)
        return 0;

    return SignalSema(pstvnc_transport_bridge_gate_semaphore_id) >= 0;
#else
    return 1;
#endif
}


static pstvnc_transport_result_t pstvnc_transport_bridge_access_begin(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t result;

    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    result = pstvnc_transport_bridge_access_result(transport_access);

    if (result == PSTVNC_TRANSPORT_OK) {
        if (pstvnc_transport_bridge_admitted_calls == UINT32_MAX) {
            result = PSTVNC_TRANSPORT_FAILED;
        } else {
            pstvnc_transport_bridge_admitted_calls++;
        }
    }

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    return result;
}

static pstvnc_transport_result_t pstvnc_transport_bridge_access_finish(
    pstvnc_transport_result_t result)
{
    if (!pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (pstvnc_transport_bridge_admitted_calls == 0u) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_FAILED;
    }

    pstvnc_transport_bridge_admitted_calls--;

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    return result;
}

static pstvnc_transport_result_t pstvnc_transport_bridge_terminal_result(void)
{
    if (pstvnc_transport_bridge_runtime.failed)
        return PSTVNC_TRANSPORT_FAILED;

    if (pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;

    if (pstvnc_transport_bridge_runtime.stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;

    return PSTVNC_TRANSPORT_FAILED;
}

static pstvnc_transport_result_t pstvnc_transport_bridge_access_result(
    const pstvnc_transport_access_t *transport_access)
{
    if (transport_access == NULL || transport_access->opaque_ticket == 0u)
        return PSTVNC_TRANSPORT_INVALID;

    if (!pstvnc_transport_bridge_session_active ||
        transport_access->opaque_ticket !=
            pstvnc_transport_bridge_active_ticket)
        return PSTVNC_TRANSPORT_CLOSED;

    if (pstvnc_transport_bridge_runtime.failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;
    if (pstvnc_transport_bridge_runtime.stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_access_acquire(
    pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t result = PSTVNC_TRANSPORT_OK;

    if (transport_access == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    transport_access->opaque_ticket = 0u;

    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_session_active ||
        pstvnc_transport_bridge_active_ticket == 0u) {
        result = PSTVNC_TRANSPORT_CLOSED;
    } else if (pstvnc_transport_bridge_runtime.failed) {
        result = PSTVNC_TRANSPORT_FAILED;
    } else if (pstvnc_transport_bridge_runtime.receiver_done) {
        result = PSTVNC_TRANSPORT_CLOSED;
    } else if (pstvnc_transport_bridge_runtime.stop_requested) {
        result = PSTVNC_TRANSPORT_STOPPED;
    } else {
        transport_access->opaque_ticket =
            pstvnc_transport_bridge_active_ticket;
    }

    if (!pstvnc_transport_bridge_gate_unlock()) {
        transport_access->opaque_ticket = 0u;
        return PSTVNC_TRANSPORT_FAILED;
    }

    return result;
}

static pstvnc_transport_result_t pstvnc_transport_bridge_finish_release(void)
{
    int released;

    if (!pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_session_active) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_INVALID;
    }

    pstvnc_transport_bridge_active_ticket = 0u;

    if (pstvnc_transport_bridge_admitted_calls != 0u) {
        if (!pstvnc_transport_bridge_gate_unlock())
            return PSTVNC_TRANSPORT_FAILED;
        return PSTVNC_TRANSPORT_WOULD_BLOCK;
    }

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    released = pstvnc_transport_runtime_release(
        &pstvnc_transport_bridge_runtime);

    if (!released && pstvnc_transport_bridge_runtime.initialized)
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    pstvnc_transport_bridge_active_ticket = 0u;
    pstvnc_transport_bridge_session_active = 0;

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    return released ? PSTVNC_TRANSPORT_OK : PSTVNC_TRANSPORT_FAILED;
}

static pstvnc_transport_result_t pstvnc_transport_session_open_internal(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    uint32_t candidate_ticket;
    int initialized;

    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (socket_fd == NULL || *socket_fd < 0 || config == NULL ||
        pstvnc_transport_bridge_session_active ||
        pstvnc_transport_bridge_admitted_calls != 0u) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_INVALID;
    }

    if (pstvnc_transport_bridge_last_ticket == UINT32_MAX) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_FAILED;
    }

    candidate_ticket = pstvnc_transport_bridge_last_ticket + 1u;

    if (audio_config != NULL && mpeg_config != NULL) {
        initialized = pstvnc_transport_runtime_initialize_with_audio_mpeg(
            &pstvnc_transport_bridge_runtime,
            *socket_fd,
            config,
            audio_config,
            mpeg_config);
    } else if (audio_config != NULL) {
        initialized = pstvnc_transport_runtime_initialize_with_audio(
            &pstvnc_transport_bridge_runtime,
            *socket_fd,
            config,
            audio_config);
    } else if (mpeg_config != NULL) {
        initialized = pstvnc_transport_runtime_initialize_with_mpeg(
            &pstvnc_transport_bridge_runtime,
            *socket_fd,
            config,
            mpeg_config);
    } else {
        initialized = pstvnc_transport_runtime_initialize(
            &pstvnc_transport_bridge_runtime,
            *socket_fd,
            config);
    }

    if (!initialized) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_FAILED;
    }

    *socket_fd = -1;

    if (!pstvnc_transport_runtime_start_receiver(
            &pstvnc_transport_bridge_runtime)) {
        (void)pstvnc_transport_runtime_release(
            &pstvnc_transport_bridge_runtime);
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_FAILED;
    }

    pstvnc_transport_bridge_last_ticket = candidate_ticket;
    pstvnc_transport_bridge_active_ticket = candidate_ticket;
    pstvnc_transport_bridge_session_active = 1;

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_session_open(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config)
{
    return pstvnc_transport_session_open_internal(
        socket_fd, config, NULL, NULL);
}

pstvnc_transport_result_t pstvnc_transport_session_open_with_audio(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    if (audio_config == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_session_open_internal(
        socket_fd, config, audio_config, NULL);
}

pstvnc_transport_result_t pstvnc_transport_session_open_with_mpeg(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (mpeg_config == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_session_open_internal(
        socket_fd, config, NULL, mpeg_config);
}

pstvnc_transport_result_t pstvnc_transport_session_open_with_audio_mpeg(
    int *socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (audio_config == NULL || mpeg_config == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_session_open_internal(
        socket_fd, config, audio_config, mpeg_config);
}

pstvnc_transport_result_t pstvnc_transport_session_abort(void)
{
    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_session_active) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_INVALID;
    }

    pstvnc_transport_bridge_active_ticket = 0u;

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_runtime_request_stop(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_runtime_wait_receiver_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    return pstvnc_transport_bridge_finish_release();
}

pstvnc_transport_result_t pstvnc_transport_session_wait_receiver_done(void)
{
    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_session_active) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_INVALID;
    }

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_runtime_wait_receiver_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_session_close(void)
{
    if (!pstvnc_transport_bridge_gate_initialize() ||
        !pstvnc_transport_bridge_gate_lock())
        return PSTVNC_TRANSPORT_FAILED;

    if (!pstvnc_transport_bridge_session_active) {
        (void)pstvnc_transport_bridge_gate_unlock();
        return PSTVNC_TRANSPORT_INVALID;
    }

    /*
     * Closing a session permanently closes its admission gate immediately.
     * Existing calls retain the old runtime until they return; new calls using
     * the old ticket become terminal even if close must be retried.
     */
    pstvnc_transport_bridge_active_ticket = 0u;

    if (pstvnc_transport_bridge_runtime.receiver_thread_started &&
        !pstvnc_transport_bridge_runtime.receiver_done) {
        if (!pstvnc_transport_bridge_gate_unlock())
            return PSTVNC_TRANSPORT_FAILED;
        return PSTVNC_TRANSPORT_WOULD_BLOCK;
    }

    if (!pstvnc_transport_bridge_gate_unlock())
        return PSTVNC_TRANSPORT_FAILED;

    return pstvnc_transport_bridge_finish_release();
}

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_read_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    int result;

    result = pstvnc_transport_runtime_rfb_poll_receive(
        &pstvnc_transport_bridge_runtime);

    if (result > 0)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);
    if (result == 0)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_WOULD_BLOCK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_write_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_runtime_audio_read_available(
        &pstvnc_transport_bridge_runtime,
        buffer,
        maximum_count,
        read_count));
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_runtime_audio_status(
        &pstvnc_transport_bridge_runtime,
        available_count,
        producer_done));
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_INVALID);

    if (pstvnc_transport_runtime_audio_activity_snapshot(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_INVALID);

    if (pstvnc_transport_runtime_audio_wait_activity(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_mpeg_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_runtime_mpeg_read_available(
        &pstvnc_transport_bridge_runtime,
        buffer,
        maximum_count,
        read_count));
}

pstvnc_transport_result_t pstvnc_transport_mpeg_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_runtime_mpeg_status(
        &pstvnc_transport_bridge_runtime,
        available_count,
        producer_done));
}

pstvnc_transport_result_t pstvnc_transport_mpeg_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_INVALID);

    if (pstvnc_transport_runtime_mpeg_activity_snapshot(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_mpeg_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_INVALID);

    if (pstvnc_transport_runtime_mpeg_wait_activity(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_mpeg_mark_producer_done(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_mpeg_mark_producer_done(
            &pstvnc_transport_bridge_runtime))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    int requested;

    requested = pstvnc_transport_runtime_rfb_quiesce_requested(
        &pstvnc_transport_bridge_runtime);

    if (requested > 0)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);
    if (requested == 0)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_WOULD_BLOCK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_send_quiesce_boundary(
            &pstvnc_transport_bridge_runtime))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_wait_quiesce_commit(
            &pstvnc_transport_bridge_runtime))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t *residual_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (residual_count == NULL)
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_INVALID);

    if (pstvnc_transport_runtime_rfb_snapshot_residual(
            &pstvnc_transport_bridge_runtime,
            residual_count))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t expected_count,
    size_t *discarded_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_discard_quiesce_residual(
            &pstvnc_transport_bridge_runtime,
            expected_count,
            discarded_count))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_begin(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_send_quiesce_complete(
            &pstvnc_transport_bridge_runtime))
        return pstvnc_transport_bridge_access_finish(PSTVNC_TRANSPORT_OK);

    return pstvnc_transport_bridge_access_finish(pstvnc_transport_bridge_terminal_result());
}

/*
 * File synopsis:
 * Implements Transport's one cross-component bridge body. It coordinates the
 * application-owned session lifecycle and adapts logical RFB plus optional
 * AUDIO/MPEG2 delivery and exact MPEG generation-control relay operations to the
 * private Transport runtime without exposing the physical PSTV descriptor or
 * moving active-generation/media policy into Transport.
 *
 * One active bridge still means one physical connection and one sole receiver.
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

static pstvnc_transport_runtime_t pstvnc_transport_bridge_runtime;
static int pstvnc_transport_bridge_session_active;
static uint32_t pstvnc_transport_bridge_last_ticket;
static uint32_t pstvnc_transport_bridge_active_ticket;

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
    if (transport_access == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    transport_access->opaque_ticket = 0u;

    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_CLOSED;
    if (pstvnc_transport_bridge_runtime.failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;
    if (pstvnc_transport_bridge_runtime.stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;
    if (pstvnc_transport_bridge_active_ticket == 0u)
        return PSTVNC_TRANSPORT_FAILED;

    transport_access->opaque_ticket =
        pstvnc_transport_bridge_active_ticket;
    return PSTVNC_TRANSPORT_OK;
}

static pstvnc_transport_result_t pstvnc_transport_bridge_finish_release(void)
{
    int released = pstvnc_transport_runtime_release(
        &pstvnc_transport_bridge_runtime);

    if (!released && pstvnc_transport_bridge_runtime.initialized)
        return PSTVNC_TRANSPORT_FAILED;

    pstvnc_transport_bridge_active_ticket = 0u;
    pstvnc_transport_bridge_session_active = 0;
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

    if (socket_fd == NULL || *socket_fd < 0 || config == NULL ||
        pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_bridge_last_ticket == UINT32_MAX)
        return PSTVNC_TRANSPORT_FAILED;

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

    if (!initialized)
        return PSTVNC_TRANSPORT_FAILED;

    *socket_fd = -1;

    if (!pstvnc_transport_runtime_start_receiver(
            &pstvnc_transport_bridge_runtime)) {
        (void)pstvnc_transport_runtime_release(
            &pstvnc_transport_bridge_runtime);
        return PSTVNC_TRANSPORT_FAILED;
    }

    pstvnc_transport_bridge_last_ticket = candidate_ticket;
    pstvnc_transport_bridge_active_ticket = candidate_ticket;
    pstvnc_transport_bridge_session_active = 1;
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
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

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
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (!pstvnc_transport_runtime_wait_receiver_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_session_close(void)
{
    if (!pstvnc_transport_bridge_session_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_bridge_runtime.receiver_thread_started &&
        !pstvnc_transport_bridge_runtime.receiver_done)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return pstvnc_transport_bridge_finish_release();
}

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_read_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    int result;

    result = pstvnc_transport_runtime_rfb_poll_receive(
        &pstvnc_transport_bridge_runtime);

    if (result > 0)
        return PSTVNC_TRANSPORT_OK;
    if (result == 0)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const pstvnc_transport_access_t *transport_access,
    const void *buffer,
    size_t count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_write_exact(
            &pstvnc_transport_bridge_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_audio_read_available(
        &pstvnc_transport_bridge_runtime,
        buffer,
        maximum_count,
        read_count);
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_audio_status(
        &pstvnc_transport_bridge_runtime,
        available_count,
        producer_done);
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_audio_activity_snapshot(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_audio_wait_activity(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_mpeg_read_available(
    const pstvnc_transport_access_t *transport_access,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_mpeg_read_available(
        &pstvnc_transport_bridge_runtime,
        buffer,
        maximum_count,
        read_count);
}

pstvnc_transport_result_t pstvnc_transport_mpeg_status(
    const pstvnc_transport_access_t *transport_access,
    size_t *available_count,
    int *producer_done)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_mpeg_status(
        &pstvnc_transport_bridge_runtime,
        available_count,
        producer_done);
}

pstvnc_transport_result_t pstvnc_transport_mpeg_activity_snapshot(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_mpeg_activity_snapshot(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_mpeg_wait_activity(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *activity_sequence)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_mpeg_wait_activity(
            &pstvnc_transport_bridge_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_mpeg_mark_producer_done(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_mpeg_mark_producer_done(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_mpeg_send_start(
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_mpeg_start_payload_t *start)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_mpeg_send_start(
        &pstvnc_transport_bridge_runtime,
        start);
}

pstvnc_transport_result_t pstvnc_transport_mpeg_send_retire(
    const pstvnc_transport_access_t *transport_access,
    const pstvnc_mpeg_retire_payload_t *retire)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_mpeg_send_retire(
        &pstvnc_transport_bridge_runtime,
        retire);
}

pstvnc_transport_result_t pstvnc_transport_mpeg_take_retire_completion(
    const pstvnc_transport_access_t *transport_access,
    pstvnc_mpeg_retire_payload_t *completion)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    return pstvnc_transport_runtime_mpeg_take_retire_completion(
        &pstvnc_transport_bridge_runtime,
        completion);
}

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    int requested;

    requested = pstvnc_transport_runtime_rfb_quiesce_requested(
        &pstvnc_transport_bridge_runtime);

    if (requested > 0)
        return PSTVNC_TRANSPORT_OK;
    if (requested == 0)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_send_quiesce_boundary(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_wait_quiesce_commit(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t *residual_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (residual_count == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_snapshot_residual(
            &pstvnc_transport_bridge_runtime,
            residual_count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    const pstvnc_transport_access_t *transport_access,
    size_t expected_count,
    size_t *discarded_count)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_discard_quiesce_residual(
            &pstvnc_transport_bridge_runtime,
            expected_count,
            discarded_count))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(
    const pstvnc_transport_access_t *transport_access)
{
    pstvnc_transport_result_t access_result =
        pstvnc_transport_bridge_access_result(transport_access);

    if (access_result != PSTVNC_TRANSPORT_OK)
        return access_result;

    if (pstvnc_transport_runtime_rfb_send_quiesce_complete(
            &pstvnc_transport_bridge_runtime))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_bridge_terminal_result();
}

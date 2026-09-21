/*
 * File synopsis:
 * Implements Transport's session-local single physical-I/O owner and synchronized logical
 * RFB/AUDIO/MPEG2 runtime. One EE Transport I/O thread is the only caller of
 * physical framed send and receive primitives; complete DATA frames are
 * dispatched into three
 * independent bounded logical channels with independent credit/activity state.
 * On PS2, an idle physical-socket readiness timeout cooperatively yields the EE
 * so queued network-stack work can progress without delaying active I/O paths.
 *
 * RFB parser consumption/quiesce, AUDIO's audited finite marker, MPEG's explicit
 * producer-completion fact, exact MPEG generation-control envelope relay,
 * outbound logical RFB fragmentation, and receiver completion remain
 * Transport-owned. RFB parsing, PCM playback, MPEG decoding,
 * common-clock/presentation policy, active-generation business meaning, and
 * product recovery remain outside this file.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#include "runtime.h"

#if defined(_EE)
#include "platform/ps2_system.h"
#endif

#include <kernel.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PSTVNC_TRANSPORT_IO_SELECT_TIMEOUT_US 1000u

#if defined(_EE)
#define PSTVNC_TRANSPORT_IO_IDLE_YIELD_US 1000u
#endif

static int pstvnc_transport_runtime_create_semaphore(
    int initial_count,
    int maximum_count)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = initial_count;
    semaphore.max_count = maximum_count;
    semaphore.option = 0;
    return CreateSema(&semaphore);
}

static void *pstvnc_transport_runtime_allocate_aligned16(
    size_t byte_count,
    void **allocation)
{
    uintptr_t aligned_address;
    void *raw;

    if (allocation == NULL || byte_count == 0u ||
        byte_count > (size_t)-1 - 15u)
        return NULL;

    raw = malloc(byte_count + 15u);
    if (raw == NULL)
        return NULL;

    aligned_address = ((uintptr_t)raw + 15u) & ~(uintptr_t)15u;
    *allocation = raw;
    return (void *)aligned_address;
}

static int pstvnc_transport_runtime_config_valid(
    const pstvnc_transport_runtime_config_t *config)
{
    if (config == NULL ||
        config->rfb_queue_capacity == 0u ||
        config->receiver_thread_stack_size == 0u ||
        config->receiver_thread_stack_size > (uint32_t)INT_MAX ||
        config->max_data_payload == 0u ||
        config->max_data_payload > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        config->max_data_payload > config->rfb_queue_capacity ||
        config->rfb_initial_credit_bytes > config->rfb_queue_capacity ||
        (config->rfb_credit_flush_on_empty != 0 &&
         config->rfb_credit_flush_on_empty != 1) ||
        (config->rfb_credit_return_enabled != 0 &&
         config->rfb_credit_return_enabled != 1))
        return 0;

    if (config->rfb_credit_return_enabled &&
        (config->rfb_credit_batch_bytes == 0u ||
         config->rfb_credit_batch_bytes > config->rfb_queue_capacity))
        return 0;

    if (!config->rfb_credit_return_enabled &&
        config->rfb_credit_batch_bytes != 0u)
        return 0;

    return 1;
}

static int pstvnc_transport_runtime_channel_config_valid(
    const pstvnc_transport_runtime_config_t *config,
    uint32_t queue_capacity,
    uint32_t initial_credit_bytes,
    uint32_t credit_batch_bytes,
    int credit_flush_on_empty,
    int credit_return_enabled)
{
    if (config == NULL || queue_capacity == 0u ||
        config->max_data_payload > queue_capacity ||
        initial_credit_bytes > queue_capacity ||
        (credit_flush_on_empty != 0 && credit_flush_on_empty != 1) ||
        (credit_return_enabled != 0 && credit_return_enabled != 1))
        return 0;

    if (credit_return_enabled &&
        (credit_batch_bytes == 0u || credit_batch_bytes > queue_capacity))
        return 0;

    if (!credit_return_enabled && credit_batch_bytes != 0u)
        return 0;

    return 1;
}

static int pstvnc_transport_runtime_audio_config_valid(
    const pstvnc_transport_runtime_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    return audio_config != NULL &&
        pstvnc_transport_runtime_channel_config_valid(
            config,
            audio_config->queue_capacity,
            audio_config->initial_credit_bytes,
            audio_config->credit_batch_bytes,
            audio_config->credit_flush_on_empty,
            audio_config->credit_return_enabled);
}

static int pstvnc_transport_runtime_mpeg_config_valid(
    const pstvnc_transport_runtime_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    return mpeg_config != NULL &&
        pstvnc_transport_runtime_channel_config_valid(
            config,
            mpeg_config->queue_capacity,
            mpeg_config->initial_credit_bytes,
            mpeg_config->credit_batch_bytes,
            mpeg_config->credit_flush_on_empty,
            mpeg_config->credit_return_enabled);
}

static void pstvnc_transport_runtime_reset_identifiers(
    pstvnc_transport_runtime_t *runtime)
{
    runtime->physical_stream.socket_fd = -1;
    runtime->physical_stream.send_semaphore_id = -1;
    runtime->rfb_queue_semaphore_id = -1;
    runtime->rfb_activity_semaphore_id = -1;
    runtime->rfb_outbound_credit_semaphore_id = -1;
    runtime->audio_queue_semaphore_id = -1;
    runtime->audio_activity_semaphore_id = -1;
    runtime->mpeg_queue_semaphore_id = -1;
    runtime->mpeg_activity_semaphore_id = -1;
    runtime->mpeg_control_semaphore_id = -1;
    runtime->receiver_done_semaphore_id = -1;
    runtime->outbound_slot_semaphore_id = -1;
    runtime->outbound_ready_semaphore_id = -1;
    runtime->outbound_done_semaphore_id = -1;
    runtime->receiver_thread_id = -1;
}

int pstvnc_transport_runtime_submit_frame(
    pstvnc_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    int result;

    if (runtime == NULL ||
        (payload_length != 0u && payload == NULL) ||
        payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        !runtime->initialized ||
        !runtime->receiver_thread_started ||
        runtime->receiver_done ||
        runtime->failed ||
        runtime->stop_requested)
        return 0;

    if (WaitSema(runtime->outbound_slot_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (runtime->receiver_done ||
        runtime->failed ||
        runtime->stop_requested) {
        (void)SignalSema(runtime->outbound_slot_semaphore_id);
        return 0;
    }

    memset(&runtime->outbound_work, 0, sizeof(runtime->outbound_work));
    runtime->outbound_work.kind = kind;
    runtime->outbound_work.channel = channel;
    runtime->outbound_work.flags = flags;
    runtime->outbound_work.payload_length = payload_length;

    if (payload_length != 0u)
        memcpy(runtime->outbound_work.payload, payload, payload_length);

    runtime->outbound_work.result = 0;
    runtime->outbound_pending = 1;

    /*
     * receiver_done is published before terminal I/O-owner cleanup. Recheck
     * after publishing the pending item so a submission racing thread exit
     * cannot sleep forever waiting for a dead owner.
     */
    if (runtime->receiver_done ||
        runtime->failed ||
        runtime->stop_requested) {
        runtime->outbound_pending = 0;
        (void)SignalSema(runtime->outbound_slot_semaphore_id);
        return 0;
    }

    if (SignalSema(runtime->outbound_ready_semaphore_id) < 0) {
        runtime->outbound_pending = 0;
        runtime->failed = 1;
        (void)SignalSema(runtime->outbound_slot_semaphore_id);
        return 0;
    }

    if (WaitSema(runtime->outbound_done_semaphore_id) < 0) {
        runtime->failed = 1;
        (void)SignalSema(runtime->outbound_slot_semaphore_id);
        return 0;
    }

    result = runtime->outbound_work.result;

    if (SignalSema(runtime->outbound_slot_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return result;
}

static int pstvnc_transport_runtime_send_credit(
    pstvnc_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    if (amount == 0u)
        return 1;

    pstvnc_transport_write_be32(payload, amount);
    return pstvnc_transport_runtime_submit_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        channel,
        0u,
        payload,
        sizeof(payload));
}


/* Caller holds rfb_queue_semaphore_id. */
static int pstvnc_transport_runtime_publish_rfb_activity_locked(
    pstvnc_transport_runtime_t *runtime)
{
    int signal_waiter = 0;

    runtime->activity_sequence++;
    if (runtime->activity_wait_armed) {
        runtime->activity_wait_armed = 0;
        signal_waiter = 1;
    }

    return signal_waiter;
}

static int pstvnc_transport_runtime_signal_rfb_activity(
    pstvnc_transport_runtime_t *runtime,
    int signal_waiter)
{
    if (!signal_waiter)
        return 1;

    if (SignalSema(runtime->rfb_activity_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

/* Caller holds the applicable media queue semaphore. */
static int pstvnc_transport_runtime_publish_media_activity_locked(
    uint32_t *activity_sequence,
    int *activity_wait_armed)
{
    int signal_waiter = 0;

    (*activity_sequence)++;
    if (*activity_wait_armed == 1) {
        *activity_wait_armed = 2;
        signal_waiter = 1;
    }

    return signal_waiter;
}

static int pstvnc_transport_runtime_signal_media_activity(
    pstvnc_transport_runtime_t *runtime,
    int activity_semaphore_id,
    int signal_waiter)
{
    if (!signal_waiter)
        return 1;

    if (SignalSema(activity_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_publish_media_terminal(
    pstvnc_transport_runtime_t *runtime,
    int enabled,
    int queue_semaphore_id,
    int activity_semaphore_id,
    uint32_t *activity_sequence,
    int *activity_wait_armed)
{
    int signal_waiter;

    if (!enabled)
        return 1;

    if (WaitSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    signal_waiter = pstvnc_transport_runtime_publish_media_activity_locked(
        activity_sequence,
        activity_wait_armed);

    if (SignalSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return pstvnc_transport_runtime_signal_media_activity(
        runtime,
        activity_semaphore_id,
        signal_waiter);
}

static int pstvnc_transport_runtime_publish_audio_terminal(
    pstvnc_transport_runtime_t *runtime)
{
    return pstvnc_transport_runtime_publish_media_terminal(
        runtime,
        runtime->audio_enabled,
        runtime->audio_queue_semaphore_id,
        runtime->audio_activity_semaphore_id,
        &runtime->audio_activity_sequence,
        &runtime->audio_activity_wait_armed);
}

static int pstvnc_transport_runtime_publish_mpeg_terminal(
    pstvnc_transport_runtime_t *runtime)
{
    return pstvnc_transport_runtime_publish_media_terminal(
        runtime,
        runtime->mpeg_enabled,
        runtime->mpeg_queue_semaphore_id,
        runtime->mpeg_activity_semaphore_id,
        &runtime->mpeg_activity_sequence,
        &runtime->mpeg_activity_wait_armed);
}

static int pstvnc_transport_runtime_accept_quiesce_marker_locked(
    pstvnc_transport_runtime_t *runtime)
{
    if (runtime->rfb_quiesce_request_received == 0u) {
        if (runtime->rfb_quiesce_boundary_sent != 0u ||
            runtime->rfb_quiesce_commit_received != 0u ||
            runtime->rfb_quiesce_complete_sent != 0u)
            return 0;

        runtime->rfb_quiesce_request_received = 1u;
        return 1;
    }

    if (runtime->rfb_quiesce_boundary_sent != 0u &&
        runtime->rfb_quiesce_commit_received == 0u &&
        runtime->rfb_quiesce_complete_sent == 0u) {
        runtime->rfb_quiesce_commit_received = 1u;
        return 1;
    }

    return 0;
}

/*
 * Pi CREDIT for channel 1 is the only authority that may release outbound RFB
 * DATA toward the provider relay. The counter and three-state waiter fence are
 * protected by the existing RFB queue semaphore so no separate generic rider
 * scheduler is introduced.
 */
static int pstvnc_transport_runtime_accept_rfb_credit(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    uint32_t amount;
    int signal_waiter = 0;

    if (header == NULL ||
        header->channel != PSTVNC_TRANSPORT_CHANNEL_RFB ||
        header->flags != 0u ||
        header->payload_length != PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE)
        return 0;

    amount = pstvnc_transport_read_be32(runtime->receiver_payload);
    if (amount == 0u)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_outbound_credit_bytes > UINT32_MAX - amount) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        return 0;
    }

    runtime->rfb_outbound_credit_bytes += amount;

    if (runtime->rfb_outbound_credit_wait_state == 1) {
        /*
         * State 2 means the wake token has been published but the writer still
         * owns the rendezvous until it returns through the protected state.
         */
        runtime->rfb_outbound_credit_wait_state = 2;
        signal_waiter = 1;
    }

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (signal_waiter &&
        SignalSema(runtime->rfb_outbound_credit_semaphore_id) < 0)
        return 0;

    return 1;
}

static int pstvnc_transport_runtime_wake_rfb_outbound_credit_waiter(
    pstvnc_transport_runtime_t *runtime)
{
    int signal_waiter = 0;

    if (runtime->rfb_queue_semaphore_id < 0 ||
        WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->rfb_outbound_credit_wait_state == 1) {
        runtime->rfb_outbound_credit_wait_state = 2;
        signal_waiter = 1;
    }

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (signal_waiter &&
        SignalSema(runtime->rfb_outbound_credit_semaphore_id) < 0)
        return 0;

    return 1;
}

static int pstvnc_transport_runtime_reserve_rfb_outbound_credit(
    pstvnc_transport_runtime_t *runtime,
    size_t requested,
    size_t *granted)
{
    if (runtime == NULL || requested == 0u || granted == NULL)
        return 0;

    *granted = 0u;

    for (;;) {
        size_t available;
        size_t amount;

        if (WaitSema(runtime->rfb_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }

        if (runtime->rfb_outbound_credit_wait_state == 2)
            runtime->rfb_outbound_credit_wait_state = 0;

        if (runtime->failed ||
            runtime->receiver_done ||
            runtime->stop_requested) {
            if (SignalSema(runtime->rfb_queue_semaphore_id) < 0)
                runtime->failed = 1;
            return 0;
        }

        available = (size_t)runtime->rfb_outbound_credit_bytes;
        if (available != 0u) {
            amount = requested;
            if (amount > available)
                amount = available;
            if (amount > (size_t)runtime->max_data_payload)
                amount = (size_t)runtime->max_data_payload;

            runtime->rfb_outbound_credit_bytes -= (uint32_t)amount;

            if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
                runtime->failed = 1;
                return 0;
            }

            *granted = amount;
            return 1;
        }

        if (runtime->rfb_outbound_credit_wait_state != 0) {
            (void)SignalSema(runtime->rfb_queue_semaphore_id);
            runtime->failed = 1;
            return 0;
        }

        runtime->rfb_outbound_credit_wait_state = 1;

        if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }

        if (WaitSema(runtime->rfb_outbound_credit_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }
    }
}

static int pstvnc_transport_runtime_accept_rfb_frame(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    int accepted;
    int signal_waiter;

    if (header->flags != 0u ||
        header->payload_length > runtime->max_data_payload)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (header->payload_length == 0u) {
        accepted = pstvnc_transport_runtime_accept_quiesce_marker_locked(runtime);
    } else {
        accepted = pstvnc_transport_rfb_channel_commit(
            &runtime->rfb_channel,
            runtime->receiver_payload,
            header->payload_length) == 0;
    }

    signal_waiter = accepted
        ? pstvnc_transport_runtime_publish_rfb_activity_locked(runtime)
        : 0;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (!accepted)
        return 0;

    return pstvnc_transport_runtime_signal_rfb_activity(runtime, signal_waiter);
}

static int pstvnc_transport_runtime_accept_audio_frame(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    int accepted;
    int signal_waiter;

    if (!runtime->audio_enabled || header->flags != 0u ||
        header->payload_length > runtime->max_data_payload)
        return 0;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0)
        return 0;

    if (header->payload_length == 0u) {
        accepted = pstvnc_transport_audio_channel_mark_producer_done(
            &runtime->audio_channel) == 0;
    } else {
        accepted = pstvnc_transport_audio_channel_commit_data(
            &runtime->audio_channel,
            runtime->receiver_payload,
            header->payload_length) == 0;
    }

    signal_waiter = accepted
        ? pstvnc_transport_runtime_publish_media_activity_locked(
            &runtime->audio_activity_sequence,
            &runtime->audio_activity_wait_armed)
        : 0;

    if (SignalSema(runtime->audio_queue_semaphore_id) < 0)
        return 0;

    if (!accepted)
        return 0;

    return pstvnc_transport_runtime_signal_media_activity(
        runtime,
        runtime->audio_activity_semaphore_id,
        signal_waiter);
}

static int pstvnc_transport_runtime_accept_mpeg_frame(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    int accepted;
    int signal_waiter;

    /* Current clean framing has no channel-4 zero-length EOF representation. */
    if (!runtime->mpeg_enabled || header->flags != 0u ||
        header->payload_length == 0u ||
        header->payload_length > runtime->max_data_payload)
        return 0;

    if (WaitSema(runtime->mpeg_queue_semaphore_id) < 0)
        return 0;

    accepted = pstvnc_transport_mpeg_channel_commit_data(
        &runtime->mpeg_channel,
        runtime->receiver_payload,
        header->payload_length) == 0;

    signal_waiter = accepted
        ? pstvnc_transport_runtime_publish_media_activity_locked(
            &runtime->mpeg_activity_sequence,
            &runtime->mpeg_activity_wait_armed)
        : 0;

    if (SignalSema(runtime->mpeg_queue_semaphore_id) < 0)
        return 0;

    if (!accepted)
        return 0;

    return pstvnc_transport_runtime_signal_media_activity(
        runtime,
        runtime->mpeg_activity_semaphore_id,
        signal_waiter);
}

static int pstvnc_transport_runtime_accept_mpeg_retire_completion(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    pstvnc_mpeg_retire_payload_t completion;
    int accepted = 0;

    if (!runtime->mpeg_enabled ||
        !pstvnc_transport_header_is_mpeg_retire(header) ||
        !pstvnc_mpeg_retire_payload_decode(
            &completion,
            runtime->receiver_payload,
            header->payload_length))
        return 0;

    if (WaitSema(runtime->mpeg_control_semaphore_id) < 0)
        return 0;

    if (!runtime->mpeg_retire_completion_pending) {
        runtime->mpeg_retire_completion = completion;
        runtime->mpeg_retire_completion_pending = 1;
        accepted = 1;
    }

    if (SignalSema(runtime->mpeg_control_semaphore_id) < 0)
        return 0;

    return accepted;
}

static int pstvnc_transport_runtime_take_outbound_ready(
    pstvnc_transport_runtime_t *runtime)
{
#if defined(_EE)
    return PollSema(runtime->outbound_ready_semaphore_id) >= 0;
#else
    /*
     * Host fixtures do not emulate PollSema. outbound_pending provides the
     * nonblocking observation; WaitSema then consumes the already-present token.
     */
    if (!runtime->outbound_pending)
        return 0;

    return WaitSema(runtime->outbound_ready_semaphore_id) >= 0;
#endif
}

static int pstvnc_transport_runtime_process_outbound(
    pstvnc_transport_runtime_t *runtime)
{
    int result;

    if (!runtime->outbound_pending) {
        runtime->failed = 1;
        return 0;
    }

    result = pstvnc_transport_physical_stream_send_frame(
        &runtime->physical_stream,
        runtime->outbound_work.kind,
        runtime->outbound_work.channel,
        runtime->outbound_work.flags,
        runtime->outbound_work.payload_length != 0u
            ? runtime->outbound_work.payload
            : NULL,
        runtime->outbound_work.payload_length);

    runtime->outbound_work.result = result ? 1 : 0;
    runtime->outbound_pending = 0;

    if (!result)
        runtime->failed = 1;

    if (SignalSema(runtime->outbound_done_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return result;
}

static void pstvnc_transport_runtime_fail_pending_outbound(
    pstvnc_transport_runtime_t *runtime)
{
    if (!runtime->outbound_pending)
        return;

    runtime->outbound_work.result = 0;
    runtime->outbound_pending = 0;

    if (SignalSema(runtime->outbound_done_semaphore_id) < 0)
        runtime->failed = 1;
}

static void pstvnc_transport_runtime_receiver_thread(void *argument)
{
    pstvnc_transport_runtime_t *runtime =
        (pstvnc_transport_runtime_t *)argument;

    while (!runtime->failed && !runtime->stop_requested) {
        pstvnc_transport_header_t header;
        int accepted = 0;
        int readable;

        /*
         * Outbound work is always serviced before entering the bounded socket
         * readiness wait. The one-item synchronous queue provides explicit
         * backpressure to domain owners.
         */
        if (pstvnc_transport_runtime_take_outbound_ready(runtime)) {
            if (!pstvnc_transport_runtime_process_outbound(runtime))
                break;
            continue;
        }

        readable = pstvnc_transport_physical_stream_wait_readable(
            &runtime->physical_stream,
            PSTVNC_TRANSPORT_IO_SELECT_TIMEOUT_US);

        if (readable < 0) {
            if (!runtime->stop_requested)
                runtime->failed = 1;
            break;
        }

        if (readable == 0) {
#if defined(_EE)
            /*
             * Proof 4I hardware established that this sole physical-I/O owner
             * must relinquish EE execution during an otherwise idle polling
             * cycle so queued PS2IP/network-stack work can make progress.
             *
             * Keep the scheduling opportunity strictly on the idle path:
             * outbound work was already checked before the readiness wait,
             * and immediately readable inbound Wire data bypasses this delay.
             *
             * 1000 us is the hardware-tested implementation baseline. It is
             * not part of the Wire ABI and is not asserted to be the final or
             * globally optimal scheduling value.
             */
            if (pstvnc_ps2_system_delay_us(
                    PSTVNC_TRANSPORT_IO_IDLE_YIELD_US) < 0) {
                runtime->failed = 1;
                break;
            }
#endif
            continue;
        }

        if (!pstvnc_transport_physical_stream_receive_frame(
                &runtime->physical_stream,
                &header,
                runtime->receiver_payload,
                sizeof(runtime->receiver_payload))) {
            if (!runtime->stop_requested)
                runtime->failed = 1;
            break;
        }

        if (runtime->stop_requested)
            break;

        if (header.kind == PSTVNC_TRANSPORT_FRAME_DATA) {
            if (header.channel == PSTVNC_TRANSPORT_CHANNEL_RFB)
                accepted = pstvnc_transport_runtime_accept_rfb_frame(
                    runtime, &header);
            else if (header.channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO)
                accepted = pstvnc_transport_runtime_accept_audio_frame(
                    runtime, &header);
            else if (header.channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2)
                accepted = pstvnc_transport_runtime_accept_mpeg_frame(
                    runtime, &header);
        } else if (header.kind == PSTVNC_TRANSPORT_FRAME_CREDIT &&
                   header.channel == PSTVNC_TRANSPORT_CHANNEL_RFB) {
            accepted = pstvnc_transport_runtime_accept_rfb_credit(
                runtime, &header);
        } else if (header.kind == PSTVNC_TRANSPORT_FRAME_MPEG_RETIRE) {
            accepted = pstvnc_transport_runtime_accept_mpeg_retire_completion(
                runtime, &header);
        }

        /*
         * Inbound START and every malformed/unrecognized control envelope are
         * invalid PS2-side traffic and terminate this Wire Session.
         */
        if (!accepted) {
            runtime->failed = 1;
            break;
        }
    }

    /*
     * Publish terminality before resolving a possibly racing outbound submitter.
     * submit_frame() rechecks this field after publishing outbound_pending.
     */
    runtime->receiver_done = 1;
    pstvnc_transport_runtime_fail_pending_outbound(runtime);

    /*
     * A domain writer may be blocked waiting for Pi-granted RFB credit. Wake
     * that one RFB-specific waiter so terminal Wire state is observable without
     * deleting its semaphore underneath a sleeping thread.
     */
    (void)pstvnc_transport_runtime_wake_rfb_outbound_credit_waiter(runtime);

    /* Wake each enabled logical owner so terminal state is event-visible. */
    if (runtime->rfb_queue_semaphore_id >= 0 &&
        WaitSema(runtime->rfb_queue_semaphore_id) >= 0) {
        int signal_waiter =
            pstvnc_transport_runtime_publish_rfb_activity_locked(runtime);

        if (SignalSema(runtime->rfb_queue_semaphore_id) >= 0)
            (void)pstvnc_transport_runtime_signal_rfb_activity(
                runtime, signal_waiter);
        else
            runtime->failed = 1;
    }

    (void)pstvnc_transport_runtime_publish_audio_terminal(runtime);
    (void)pstvnc_transport_runtime_publish_mpeg_terminal(runtime);

    if (runtime->receiver_done_semaphore_id >= 0 &&
        SignalSema(runtime->receiver_done_semaphore_id) < 0)
        runtime->failed = 1;

    ExitThread();
}


static int pstvnc_transport_runtime_initialize_internal(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    pstvnc_transport_physical_stream_t *established_stream,
    const pstvnc_transport_runtime_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (runtime == NULL ||
        ((socket_fd < 0) == (established_stream == NULL)) ||
        !pstvnc_transport_runtime_config_valid(config) ||
        (audio_config != NULL &&
         !pstvnc_transport_runtime_audio_config_valid(config, audio_config)) ||
        (mpeg_config != NULL &&
         !pstvnc_transport_runtime_mpeg_config_valid(config, mpeg_config)))
        return 0;

    memset(runtime, 0, sizeof(*runtime));
    pstvnc_transport_runtime_reset_identifiers(runtime);

    runtime->rfb_queue_storage =
        (uint8_t *)malloc((size_t)config->rfb_queue_capacity);
    if (runtime->rfb_queue_storage == NULL)
        return 0;

    if (pstvnc_transport_rfb_channel_initialize(
            &runtime->rfb_channel,
            runtime->rfb_queue_storage,
            (size_t)config->rfb_queue_capacity) != 0)
        goto fail;

    runtime->rfb_queue_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(1, 1);
    if (runtime->rfb_queue_semaphore_id < 0)
        goto fail;

    runtime->rfb_activity_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->rfb_activity_semaphore_id < 0)
        goto fail;

    runtime->rfb_outbound_credit_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->rfb_outbound_credit_semaphore_id < 0)
        goto fail;

    if (audio_config != NULL) {
        runtime->audio_queue_storage =
            (uint8_t *)malloc((size_t)audio_config->queue_capacity);
        if (runtime->audio_queue_storage == NULL)
            goto fail;

        if (pstvnc_transport_audio_channel_initialize(
                &runtime->audio_channel,
                runtime->audio_queue_storage,
                (size_t)audio_config->queue_capacity) != 0)
            goto fail;

        runtime->audio_queue_semaphore_id =
            pstvnc_transport_runtime_create_semaphore(1, 1);
        if (runtime->audio_queue_semaphore_id < 0)
            goto fail;

        runtime->audio_activity_semaphore_id =
            pstvnc_transport_runtime_create_semaphore(0, 1);
        if (runtime->audio_activity_semaphore_id < 0)
            goto fail;
    }

    if (mpeg_config != NULL) {
        runtime->mpeg_queue_storage =
            (uint8_t *)malloc((size_t)mpeg_config->queue_capacity);
        if (runtime->mpeg_queue_storage == NULL)
            goto fail;

        if (pstvnc_transport_mpeg_channel_initialize(
                &runtime->mpeg_channel,
                runtime->mpeg_queue_storage,
                (size_t)mpeg_config->queue_capacity) != 0)
            goto fail;

        runtime->mpeg_queue_semaphore_id =
            pstvnc_transport_runtime_create_semaphore(1, 1);
        if (runtime->mpeg_queue_semaphore_id < 0)
            goto fail;

        runtime->mpeg_activity_semaphore_id =
            pstvnc_transport_runtime_create_semaphore(0, 1);
        if (runtime->mpeg_activity_semaphore_id < 0)
            goto fail;

        runtime->mpeg_control_semaphore_id =
            pstvnc_transport_runtime_create_semaphore(1, 1);
        if (runtime->mpeg_control_semaphore_id < 0)
            goto fail;
    }

    runtime->receiver_done_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->receiver_done_semaphore_id < 0)
        goto fail;

    runtime->outbound_slot_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(1, 1);
    if (runtime->outbound_slot_semaphore_id < 0)
        goto fail;

    runtime->outbound_ready_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->outbound_ready_semaphore_id < 0)
        goto fail;

    runtime->outbound_done_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->outbound_done_semaphore_id < 0)
        goto fail;

    runtime->receiver_stack =
        (unsigned char *)pstvnc_transport_runtime_allocate_aligned16(
            (size_t)config->receiver_thread_stack_size,
            &runtime->receiver_stack_allocation);
    if (runtime->receiver_stack == NULL)
        goto fail;

    if (established_stream != NULL) {
        /*
         * Q4 already consumed sequence 1 in each direction. Move the physical
         * owner intact so runtime I/O continues at 2/2 without any public
         * sequence seed or second descriptor adoption.
         */
        if (!pstvnc_transport_physical_stream_transfer_established(
                &runtime->physical_stream,
                established_stream))
            goto fail;
    } else if (!pstvnc_transport_physical_stream_adopt(
                   &runtime->physical_stream,
                   socket_fd)) {
        goto fail;
    }

    runtime->rfb_initial_credit_bytes = config->rfb_initial_credit_bytes;
    runtime->rfb_credit_batch_bytes = config->rfb_credit_batch_bytes;
    runtime->rfb_credit_flush_on_empty = config->rfb_credit_flush_on_empty;
    runtime->rfb_credit_return_enabled = config->rfb_credit_return_enabled;

    if (audio_config != NULL) {
        runtime->audio_enabled = 1;
        runtime->audio_initial_credit_bytes = audio_config->initial_credit_bytes;
        runtime->audio_credit_batch_bytes = audio_config->credit_batch_bytes;
        runtime->audio_credit_flush_on_empty = audio_config->credit_flush_on_empty;
        runtime->audio_credit_return_enabled = audio_config->credit_return_enabled;
    }

    if (mpeg_config != NULL) {
        runtime->mpeg_enabled = 1;
        runtime->mpeg_initial_credit_bytes = mpeg_config->initial_credit_bytes;
        runtime->mpeg_credit_batch_bytes = mpeg_config->credit_batch_bytes;
        runtime->mpeg_credit_flush_on_empty = mpeg_config->credit_flush_on_empty;
        runtime->mpeg_credit_return_enabled = mpeg_config->credit_return_enabled;
    }

    runtime->receiver_thread_stack_size = config->receiver_thread_stack_size;
    runtime->receiver_thread_priority = config->receiver_thread_priority;
    runtime->max_data_payload = config->max_data_payload;
    runtime->initialized = 1;
    return 1;

fail:
    if (runtime->outbound_done_semaphore_id >= 0)
        (void)DeleteSema(runtime->outbound_done_semaphore_id);
    if (runtime->outbound_ready_semaphore_id >= 0)
        (void)DeleteSema(runtime->outbound_ready_semaphore_id);
    if (runtime->outbound_slot_semaphore_id >= 0)
        (void)DeleteSema(runtime->outbound_slot_semaphore_id);
    if (runtime->receiver_done_semaphore_id >= 0)
        (void)DeleteSema(runtime->receiver_done_semaphore_id);
    if (runtime->mpeg_control_semaphore_id >= 0)
        (void)DeleteSema(runtime->mpeg_control_semaphore_id);
    if (runtime->mpeg_activity_semaphore_id >= 0)
        (void)DeleteSema(runtime->mpeg_activity_semaphore_id);
    if (runtime->mpeg_queue_semaphore_id >= 0)
        (void)DeleteSema(runtime->mpeg_queue_semaphore_id);
    if (runtime->audio_activity_semaphore_id >= 0)
        (void)DeleteSema(runtime->audio_activity_semaphore_id);
    if (runtime->audio_queue_semaphore_id >= 0)
        (void)DeleteSema(runtime->audio_queue_semaphore_id);
    if (runtime->rfb_outbound_credit_semaphore_id >= 0)
        (void)DeleteSema(runtime->rfb_outbound_credit_semaphore_id);
    if (runtime->rfb_activity_semaphore_id >= 0)
        (void)DeleteSema(runtime->rfb_activity_semaphore_id);
    if (runtime->rfb_queue_semaphore_id >= 0)
        (void)DeleteSema(runtime->rfb_queue_semaphore_id);
    free(runtime->receiver_stack_allocation);
    free(runtime->mpeg_queue_storage);
    free(runtime->audio_queue_storage);
    free(runtime->rfb_queue_storage);
    memset(runtime, 0, sizeof(*runtime));
    pstvnc_transport_runtime_reset_identifiers(runtime);
    return 0;
}

int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config)
{
    return pstvnc_transport_runtime_initialize_internal(
        runtime, socket_fd, NULL, config, NULL, NULL);
}

int pstvnc_transport_runtime_initialize_with_audio(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    if (audio_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, socket_fd, NULL, config, audio_config, NULL);
}

int pstvnc_transport_runtime_initialize_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (mpeg_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, socket_fd, NULL, config, NULL, mpeg_config);
}

int pstvnc_transport_runtime_initialize_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (audio_config == NULL || mpeg_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, socket_fd, NULL, config, audio_config, mpeg_config);
}

int pstvnc_transport_runtime_initialize_established(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config)
{
    return pstvnc_transport_runtime_initialize_internal(
        runtime, -1, physical_stream, config, NULL, NULL);
}

int pstvnc_transport_runtime_initialize_established_with_audio(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    if (audio_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, -1, physical_stream, config, audio_config, NULL);
}

int pstvnc_transport_runtime_initialize_established_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (mpeg_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, -1, physical_stream, config, NULL, mpeg_config);
}

int pstvnc_transport_runtime_initialize_established_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config)
{
    if (audio_config == NULL || mpeg_config == NULL)
        return 0;

    return pstvnc_transport_runtime_initialize_internal(
        runtime, -1, physical_stream, config, audio_config, mpeg_config);
}


int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime)
{
    ee_thread_t thread;

    if (runtime == NULL || !runtime->initialized ||
        runtime->receiver_thread_started || runtime->failed ||
        runtime->stop_requested)
        return 0;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)pstvnc_transport_runtime_receiver_thread;
    thread.stack = runtime->receiver_stack;
    thread.stack_size = (int)runtime->receiver_thread_stack_size;
    thread.gp_reg = &_gp;
    thread.initial_priority = runtime->receiver_thread_priority;
    thread.attr = 0;
    thread.option = 0;

    runtime->receiver_thread_id = CreateThread(&thread);
    if (runtime->receiver_thread_id < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (StartThread(runtime->receiver_thread_id, runtime) < 0) {
        (void)DeleteThread(runtime->receiver_thread_id);
        runtime->receiver_thread_id = -1;
        runtime->failed = 1;
        return 0;
    }

    /*
     * The compatibility name remains "receiver", but this thread is now the
     * sole physical-I/O owner. Publish liveness before submitting startup
     * credits so those frames are executed by that owner.
     */
    runtime->receiver_thread_started = 1;

    if (!pstvnc_transport_runtime_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            runtime->rfb_initial_credit_bytes) ||
        (runtime->audio_enabled &&
         !pstvnc_transport_runtime_send_credit(
             runtime,
             PSTVNC_TRANSPORT_CHANNEL_AUDIO,
             runtime->audio_initial_credit_bytes)) ||
        (runtime->mpeg_enabled &&
         !pstvnc_transport_runtime_send_credit(
             runtime,
             PSTVNC_TRANSPORT_CHANNEL_MPEG2,
             runtime->mpeg_initial_credit_bytes))) {
        runtime->failed = 1;

        /*
         * A startup send failure is terminal. If the owner has not yet
         * published completion, interrupt any readiness/receive wait and wait
         * for that owner before returning failure.
         */
        if (!runtime->receiver_done) {
            runtime->stop_requested = 1;
            (void)pstvnc_transport_physical_stream_shutdown_io(
                &runtime->physical_stream);
            (void)pstvnc_transport_runtime_wait_receiver_done(runtime);
        }

        return 0;
    }

    return 1;
}


int pstvnc_transport_runtime_request_stop(
    pstvnc_transport_runtime_t *runtime)
{
    int audio_wake_ok;
    int mpeg_wake_ok;
    int shutdown_ok;

    if (runtime == NULL || !runtime->initialized ||
        !runtime->receiver_thread_started)
        return 0;

    if (runtime->receiver_done)
        return 1;

    runtime->stop_requested = 1;
    audio_wake_ok = pstvnc_transport_runtime_publish_audio_terminal(runtime);
    mpeg_wake_ok = pstvnc_transport_runtime_publish_mpeg_terminal(runtime);
    shutdown_ok = pstvnc_transport_physical_stream_shutdown_io(
        &runtime->physical_stream);
    return audio_wake_ok && mpeg_wake_ok && shutdown_ok;
}

int pstvnc_transport_runtime_rfb_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    if (runtime == NULL || activity_sequence == NULL ||
        !runtime->initialized)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    *activity_sequence = runtime->activity_sequence;

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_rfb_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    if (runtime == NULL || activity_sequence == NULL ||
        !runtime->initialized)
        return 0;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;

    if (runtime->activity_sequence != *activity_sequence ||
        runtime->receiver_done || runtime->failed || runtime->stop_requested) {
        *activity_sequence = runtime->activity_sequence;
        if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }
        return 1;
    }

    if (runtime->activity_wait_armed) {
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    runtime->activity_wait_armed = 1;
    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (WaitSema(runtime->rfb_activity_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (runtime->activity_wait_armed) {
        runtime->activity_wait_armed = 0;
        (void)SignalSema(runtime->rfb_queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    *activity_sequence = runtime->activity_sequence;
    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_return_credit(
    pstvnc_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t *pending,
    uint32_t batch_bytes,
    int flush_on_empty,
    int return_enabled,
    uint32_t consumed,
    int queue_empty)
{
    uint32_t amount = 0u;

    if (!return_enabled)
        return 1;

    if (*pending > UINT32_MAX - consumed) {
        runtime->failed = 1;
        return 0;
    }

    *pending += consumed;
    if (*pending >= batch_bytes ||
        (flush_on_empty && queue_empty && *pending != 0u)) {
        amount = *pending;
        *pending = 0u;
    }

    if (amount != 0u &&
        !pstvnc_transport_runtime_send_credit(runtime, channel, amount)) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_return_rfb_credit(
    pstvnc_transport_runtime_t *runtime,
    uint32_t consumed,
    int queue_empty)
{
    return pstvnc_transport_runtime_return_credit(
        runtime,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        &runtime->rfb_credit_pending,
        runtime->rfb_credit_batch_bytes,
        runtime->rfb_credit_flush_on_empty,
        runtime->rfb_credit_return_enabled,
        consumed,
        queue_empty);
}

static int pstvnc_transport_runtime_return_audio_credit(
    pstvnc_transport_runtime_t *runtime,
    uint32_t consumed,
    int queue_empty)
{
    if (!pstvnc_transport_runtime_return_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            &runtime->audio_credit_pending,
            runtime->audio_credit_batch_bytes,
            runtime->audio_credit_flush_on_empty,
            runtime->audio_credit_return_enabled,
            consumed,
            queue_empty)) {
        (void)pstvnc_transport_runtime_publish_audio_terminal(runtime);
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_return_mpeg_credit(
    pstvnc_transport_runtime_t *runtime,
    uint32_t consumed,
    int queue_empty)
{
    if (!pstvnc_transport_runtime_return_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_MPEG2,
            &runtime->mpeg_credit_pending,
            runtime->mpeg_credit_batch_bytes,
            runtime->mpeg_credit_flush_on_empty,
            runtime->mpeg_credit_return_enabled,
            consumed,
            queue_empty)) {
        (void)pstvnc_transport_runtime_publish_mpeg_terminal(runtime);
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_rfb_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    uint32_t activity_sequence;
    size_t done = 0u;

    if (runtime == NULL || (buffer == NULL && count != 0u) ||
        count > UINT32_MAX || !runtime->initialized)
        return 0;

    if (count == 0u)
        return 1;

    if (!pstvnc_transport_runtime_rfb_activity_snapshot(
            runtime, &activity_sequence))
        return 0;

    while (done < count) {
        size_t taken;
        int queue_empty;

        if (runtime->failed || runtime->stop_requested)
            return 0;

        if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
            return 0;

        taken = pstvnc_transport_rfb_channel_read_available(
            &runtime->rfb_channel,
            destination + done,
            count - done);
        queue_empty =
            pstvnc_transport_rfb_channel_available(&runtime->rfb_channel) == 0u;

        if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }

        if (taken != 0u) {
            if (taken > UINT32_MAX ||
                !pstvnc_transport_runtime_return_rfb_credit(
                    runtime, (uint32_t)taken, queue_empty))
                return 0;

            done += taken;
            continue;
        }

        if (runtime->receiver_done)
            return 0;

        if (!pstvnc_transport_runtime_rfb_wait_activity(
                runtime, &activity_sequence))
            return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_rfb_poll_receive(
    pstvnc_transport_runtime_t *runtime)
{
    size_t available;

    if (runtime == NULL || !runtime->initialized)
        return -1;

    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return -1;

    available = pstvnc_transport_rfb_channel_available(&runtime->rfb_channel);

    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return -1;
    }

    if (available != 0u)
        return 1;
    if (runtime->failed || runtime->receiver_done || runtime->stop_requested)
        return -1;
    return 0;
}


int pstvnc_transport_runtime_rfb_write_exact(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t offset = 0u;

    if (runtime == NULL || (buffer == NULL && count != 0u) ||
        !runtime->initialized || runtime->failed || runtime->stop_requested)
        return 0;

    while (offset < count) {
        size_t fragment;

        /*
         * Outbound RFB bytes may not enter Wire merely because the socket and
         * rider runtime are live. Each fragment reserves exact Pi-granted free
         * provider capacity first. Partial credit therefore releases only the
         * matching prefix and the same writer waits for later replenishment.
         */
        if (!pstvnc_transport_runtime_reserve_rfb_outbound_credit(
                runtime,
                count - offset,
                &fragment))
            return 0;

        if (!pstvnc_transport_runtime_submit_frame(
                runtime,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0u,
                bytes + offset,
                fragment)) {
            /*
             * Do not restore reserved credit after a send failure. The physical
             * send may have exposed a frame prefix and the Wire Session is now
             * terminal, so reusing the same capacity would be ambiguous.
             */
            runtime->failed = 1;
            return 0;
        }

        offset += fragment;
    }

    return 1;
}


pstvnc_transport_result_t pstvnc_transport_runtime_audio_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    size_t taken;
    int queue_empty;
    int producer_done;

    if (runtime == NULL || buffer == NULL || maximum_count == 0u ||
        read_count == NULL || maximum_count > UINT32_MAX ||
        !runtime->initialized || !runtime->audio_enabled)
        return PSTVNC_TRANSPORT_INVALID;

    *read_count = 0u;

    if (runtime->stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    taken = pstvnc_transport_audio_channel_read_available(
        &runtime->audio_channel,
        (uint8_t *)buffer,
        maximum_count);
    queue_empty =
        pstvnc_transport_audio_channel_available(&runtime->audio_channel) == 0u;
    producer_done =
        pstvnc_transport_audio_channel_producer_done(&runtime->audio_channel);

    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    if (taken != 0u) {
        *read_count = taken;
        if (!runtime->failed &&
            !pstvnc_transport_runtime_return_audio_credit(
                runtime, (uint32_t)taken, queue_empty))
            return PSTVNC_TRANSPORT_FAILED;
        return PSTVNC_TRANSPORT_OK;
    }

    if (runtime->failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (runtime->stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;
    if (producer_done)
        return PSTVNC_TRANSPORT_EXHAUSTED;
    if (runtime->receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;
    return PSTVNC_TRANSPORT_WOULD_BLOCK;
}

pstvnc_transport_result_t pstvnc_transport_runtime_audio_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done)
{
    if (runtime == NULL || available_count == NULL || producer_done == NULL ||
        !runtime->initialized || !runtime->audio_enabled)
        return PSTVNC_TRANSPORT_INVALID;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    *available_count =
        pstvnc_transport_audio_channel_available(&runtime->audio_channel);
    *producer_done =
        pstvnc_transport_audio_channel_producer_done(&runtime->audio_channel);

    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    return PSTVNC_TRANSPORT_OK;
}

static int pstvnc_transport_runtime_media_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    int enabled,
    int queue_semaphore_id,
    uint32_t *current_sequence,
    uint32_t *snapshot)
{
    if (runtime == NULL || snapshot == NULL || current_sequence == NULL ||
        !runtime->initialized || !enabled)
        return 0;

    if (WaitSema(queue_semaphore_id) < 0)
        return 0;

    *snapshot = *current_sequence;

    if (SignalSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_media_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    int enabled,
    int queue_semaphore_id,
    int activity_semaphore_id,
    uint32_t *current_sequence,
    int *activity_wait_armed,
    uint32_t *observed_sequence)
{
    if (runtime == NULL || observed_sequence == NULL || !runtime->initialized ||
        !enabled)
        return 0;

    if (WaitSema(queue_semaphore_id) < 0)
        return 0;

    if (*current_sequence != *observed_sequence ||
        runtime->receiver_done || runtime->failed || runtime->stop_requested) {
        *observed_sequence = *current_sequence;
        if (SignalSema(queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }
        return 1;
    }

    if (*activity_wait_armed != 0) {
        (void)SignalSema(queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    *activity_wait_armed = 1;
    if (SignalSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (WaitSema(activity_semaphore_id) < 0) {
        runtime->failed = 1;
        if (WaitSema(queue_semaphore_id) >= 0) {
            *activity_wait_armed = 0;
            (void)SignalSema(queue_semaphore_id);
        }
        return 0;
    }

    if (WaitSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (*activity_wait_armed != 2) {
        *activity_wait_armed = 0;
        (void)SignalSema(queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    *activity_wait_armed = 0;
    *observed_sequence = *current_sequence;
    if (SignalSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_audio_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    return pstvnc_transport_runtime_media_activity_snapshot(
        runtime,
        runtime != NULL ? runtime->audio_enabled : 0,
        runtime != NULL ? runtime->audio_queue_semaphore_id : -1,
        runtime != NULL ? &runtime->audio_activity_sequence : NULL,
        activity_sequence);
}

int pstvnc_transport_runtime_audio_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    return pstvnc_transport_runtime_media_wait_activity(
        runtime,
        runtime != NULL ? runtime->audio_enabled : 0,
        runtime != NULL ? runtime->audio_queue_semaphore_id : -1,
        runtime != NULL ? runtime->audio_activity_semaphore_id : -1,
        runtime != NULL ? &runtime->audio_activity_sequence : NULL,
        runtime != NULL ? &runtime->audio_activity_wait_armed : NULL,
        activity_sequence);
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    size_t taken;
    int queue_empty;
    int producer_done;

    if (runtime == NULL || buffer == NULL || maximum_count == 0u ||
        read_count == NULL || maximum_count > UINT32_MAX ||
        !runtime->initialized || !runtime->mpeg_enabled)
        return PSTVNC_TRANSPORT_INVALID;

    *read_count = 0u;

    if (runtime->stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;

    if (WaitSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    taken = pstvnc_transport_mpeg_channel_read_available(
        &runtime->mpeg_channel,
        (uint8_t *)buffer,
        maximum_count);
    queue_empty =
        pstvnc_transport_mpeg_channel_available(&runtime->mpeg_channel) == 0u;
    producer_done =
        pstvnc_transport_mpeg_channel_producer_done(&runtime->mpeg_channel);

    if (SignalSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    if (taken != 0u) {
        *read_count = taken;
        if (!runtime->failed &&
            !pstvnc_transport_runtime_return_mpeg_credit(
                runtime, (uint32_t)taken, queue_empty))
            return PSTVNC_TRANSPORT_FAILED;
        return PSTVNC_TRANSPORT_OK;
    }

    if (runtime->failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (runtime->stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;
    if (producer_done)
        return PSTVNC_TRANSPORT_EXHAUSTED;
    if (runtime->receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;
    return PSTVNC_TRANSPORT_WOULD_BLOCK;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done)
{
    if (runtime == NULL || available_count == NULL || producer_done == NULL ||
        !runtime->initialized || !runtime->mpeg_enabled)
        return PSTVNC_TRANSPORT_INVALID;

    if (WaitSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    *available_count =
        pstvnc_transport_mpeg_channel_available(&runtime->mpeg_channel);
    *producer_done =
        pstvnc_transport_mpeg_channel_producer_done(&runtime->mpeg_channel);

    if (SignalSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    return PSTVNC_TRANSPORT_OK;
}

int pstvnc_transport_runtime_mpeg_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    return pstvnc_transport_runtime_media_activity_snapshot(
        runtime,
        runtime != NULL ? runtime->mpeg_enabled : 0,
        runtime != NULL ? runtime->mpeg_queue_semaphore_id : -1,
        runtime != NULL ? &runtime->mpeg_activity_sequence : NULL,
        activity_sequence);
}

int pstvnc_transport_runtime_mpeg_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    return pstvnc_transport_runtime_media_wait_activity(
        runtime,
        runtime != NULL ? runtime->mpeg_enabled : 0,
        runtime != NULL ? runtime->mpeg_queue_semaphore_id : -1,
        runtime != NULL ? runtime->mpeg_activity_semaphore_id : -1,
        runtime != NULL ? &runtime->mpeg_activity_sequence : NULL,
        runtime != NULL ? &runtime->mpeg_activity_wait_armed : NULL,
        activity_sequence);
}

int pstvnc_transport_runtime_mpeg_mark_producer_done(
    pstvnc_transport_runtime_t *runtime)
{
    int accepted;
    int signal_waiter;

    if (runtime == NULL || !runtime->initialized || !runtime->mpeg_enabled ||
        runtime->failed || runtime->stop_requested)
        return 0;

    if (WaitSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    accepted = pstvnc_transport_mpeg_channel_mark_producer_done(
        &runtime->mpeg_channel) == 0;
    signal_waiter = accepted
        ? pstvnc_transport_runtime_publish_media_activity_locked(
            &runtime->mpeg_activity_sequence,
            &runtime->mpeg_activity_wait_armed)
        : 0;

    if (SignalSema(runtime->mpeg_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (!accepted)
        return 0;

    return pstvnc_transport_runtime_signal_media_activity(
        runtime,
        runtime->mpeg_activity_semaphore_id,
        signal_waiter);
}

static pstvnc_transport_result_t pstvnc_transport_runtime_control_failure(
    const pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->initialized || !runtime->mpeg_enabled)
        return PSTVNC_TRANSPORT_INVALID;
    if (runtime->failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (runtime->receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;
    if (runtime->stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;
    return PSTVNC_TRANSPORT_FAILED;
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_start(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_start_payload_t *start)
{
    uint8_t payload[PSTVNC_MPEG_START_PAYLOAD_SIZE];

    if (runtime == NULL || start == NULL ||
        !runtime->initialized || !runtime->mpeg_enabled ||
        !pstvnc_mpeg_start_payload_encode(payload, start))
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_submit_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_MPEG_START,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            payload,
            sizeof(payload)))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_runtime_control_failure(runtime);
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_retire(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_retire_payload_t *retire)
{
    uint8_t payload[PSTVNC_MPEG_RETIRE_PAYLOAD_SIZE];

    if (runtime == NULL || retire == NULL ||
        !runtime->initialized || !runtime->mpeg_enabled ||
        !pstvnc_mpeg_retire_payload_encode(payload, retire))
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_submit_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_MPEG_RETIRE,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            payload,
            sizeof(payload)))
        return PSTVNC_TRANSPORT_OK;

    return pstvnc_transport_runtime_control_failure(runtime);
}

pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_take_retire_completion(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_mpeg_retire_payload_t *completion)
{
    pstvnc_transport_result_t result;

    if (runtime == NULL || completion == NULL ||
        !runtime->initialized || !runtime->mpeg_enabled)
        return PSTVNC_TRANSPORT_INVALID;

    if (runtime->failed || runtime->receiver_done || runtime->stop_requested)
        return pstvnc_transport_runtime_control_failure(runtime);

    if (WaitSema(runtime->mpeg_control_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    if (!runtime->mpeg_retire_completion_pending) {
        result = PSTVNC_TRANSPORT_WOULD_BLOCK;
    } else {
        *completion = runtime->mpeg_retire_completion;
        memset(&runtime->mpeg_retire_completion, 0,
            sizeof(runtime->mpeg_retire_completion));
        runtime->mpeg_retire_completion_pending = 0;
        result = PSTVNC_TRANSPORT_OK;
    }

    if (SignalSema(runtime->mpeg_control_semaphore_id) < 0) {
        runtime->failed = 1;
        return PSTVNC_TRANSPORT_FAILED;
    }

    return result;
}

int pstvnc_transport_runtime_wait_receiver_done(
    pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->initialized ||
        !runtime->receiver_thread_started)
        return 0;

    if (runtime->receiver_done)
        return 1;

    if (WaitSema(runtime->receiver_done_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return runtime->receiver_done != 0;
}

static int pstvnc_transport_runtime_media_waiter_live(
    pstvnc_transport_runtime_t *runtime,
    int enabled,
    int queue_semaphore_id,
    int *activity_wait_armed,
    int *waiter_live)
{
    if (waiter_live == NULL || activity_wait_armed == NULL)
        return 0;

    if (!enabled) {
        *waiter_live = 0;
        return 1;
    }

    if (WaitSema(queue_semaphore_id) < 0)
        return 0;

    *waiter_live = *activity_wait_armed != 0;

    if (SignalSema(queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime)
{
    int result = 1;
    int waiter_live;

    if (runtime == NULL || !runtime->initialized)
        return 0;

    if (runtime->receiver_thread_started && !runtime->receiver_done)
        return 0;

    /*
     * Once receiver_done is visible, new media waiters return terminally before
     * arming. A nonzero protected wait state can therefore only belong to an
     * already-existing waiter that still owns its rendezvous. Preserve all
     * waiter-visible resources and let the caller retry after it returns.
     */
    if (WaitSema(runtime->rfb_queue_semaphore_id) < 0)
        return 0;
    waiter_live = runtime->rfb_outbound_credit_wait_state != 0;
    if (SignalSema(runtime->rfb_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }
    if (waiter_live)
        return 0;

    if (!pstvnc_transport_runtime_media_waiter_live(
            runtime,
            runtime->audio_enabled,
            runtime->audio_queue_semaphore_id,
            &runtime->audio_activity_wait_armed,
            &waiter_live) || waiter_live)
        return 0;

    if (!pstvnc_transport_runtime_media_waiter_live(
            runtime,
            runtime->mpeg_enabled,
            runtime->mpeg_queue_semaphore_id,
            &runtime->mpeg_activity_wait_armed,
            &waiter_live) || waiter_live)
        return 0;

    if (runtime->receiver_thread_started) {
        ee_thread_status_t status;

        if (runtime->receiver_thread_id < 0)
            return 0;

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(runtime->receiver_thread_id, &status) < 0)
            return 0;

        if (status.status != THS_DORMANT) {
            if (TerminateThread(runtime->receiver_thread_id) < 0)
                return 0;

            memset(&status, 0, sizeof(status));
            if (ReferThreadStatus(runtime->receiver_thread_id, &status) < 0 ||
                status.status != THS_DORMANT)
                return 0;
        }

        if (DeleteThread(runtime->receiver_thread_id) < 0)
            return 0;

        runtime->receiver_thread_id = -1;
        runtime->receiver_thread_started = 0;
    }

    if (runtime->outbound_done_semaphore_id >= 0 &&
        DeleteSema(runtime->outbound_done_semaphore_id) < 0)
        result = 0;
    if (runtime->outbound_ready_semaphore_id >= 0 &&
        DeleteSema(runtime->outbound_ready_semaphore_id) < 0)
        result = 0;
    if (runtime->outbound_slot_semaphore_id >= 0 &&
        DeleteSema(runtime->outbound_slot_semaphore_id) < 0)
        result = 0;
    if (runtime->receiver_done_semaphore_id >= 0 &&
        DeleteSema(runtime->receiver_done_semaphore_id) < 0)
        result = 0;
    if (runtime->mpeg_control_semaphore_id >= 0 &&
        DeleteSema(runtime->mpeg_control_semaphore_id) < 0)
        result = 0;
    if (runtime->mpeg_activity_semaphore_id >= 0 &&
        DeleteSema(runtime->mpeg_activity_semaphore_id) < 0)
        result = 0;
    if (runtime->mpeg_queue_semaphore_id >= 0 &&
        DeleteSema(runtime->mpeg_queue_semaphore_id) < 0)
        result = 0;
    if (runtime->audio_activity_semaphore_id >= 0 &&
        DeleteSema(runtime->audio_activity_semaphore_id) < 0)
        result = 0;
    if (runtime->audio_queue_semaphore_id >= 0 &&
        DeleteSema(runtime->audio_queue_semaphore_id) < 0)
        result = 0;
    if (runtime->rfb_outbound_credit_semaphore_id >= 0 &&
        DeleteSema(runtime->rfb_outbound_credit_semaphore_id) < 0)
        result = 0;
    if (runtime->rfb_activity_semaphore_id >= 0 &&
        DeleteSema(runtime->rfb_activity_semaphore_id) < 0)
        result = 0;
    if (runtime->rfb_queue_semaphore_id >= 0 &&
        DeleteSema(runtime->rfb_queue_semaphore_id) < 0)
        result = 0;

    free(runtime->receiver_stack_allocation);
    free(runtime->mpeg_queue_storage);
    free(runtime->audio_queue_storage);
    free(runtime->rfb_queue_storage);
    pstvnc_transport_physical_stream_release(&runtime->physical_stream);

    memset(runtime, 0, sizeof(*runtime));
    pstvnc_transport_runtime_reset_identifiers(runtime);
    return result;
}

/*
 * File synopsis:
 * Implements live logical RFB channel-1 mechanics for the cumulative H1 mux.
 *
 * This module does not open a socket and never calls recv(). H1's existing sole
 * physical receiver dispatches inbound channel-1 DATA here; the unchanged RFB
 * parser consumes those bytes through the mux adapter. Parser-consumed bytes
 * drive the configurable receiver-credit policy, and outbound RFB client bytes
 * are fragmented into H1 DATA frames through the existing serialized send path.
 *
 * Clean finite-session shutdown uses zero-length channel-1 DATA frames as
 * lifecycle markers. They contain no RFB bytes and are consumed here instead of
 * entering the parser queue. Direction plus strict state ordering makes their
 * meaning unambiguous:
 *   Pi request -> PS2 boundary -> Pi commit -> PS2 complete.
 *
 * The authoritative CONFIG validator still controls whether RFB ON may enter
 * this path. Keeping mechanics complete behind that gate lets source/build tests
 * prove ownership before the first RFB hardware activation.
 */

#include "h1_rfb_transport_live.h"

#include "h1_rfb_credit_policy.h"
#include "h1_rfb_mux_io.h"
#include "h1_transport_runtime.h"
#include "transport_protocol.h"

#include <delaythread.h>
#include <kernel.h>

#include <stdint.h>

#define H1_RFB_EXACT_READ_WAIT_US 1000u

/*
 * Observation-only exact-read inner-loop witnesses.
 *
 * The existing outer RFB witness proves that the parser entered an exact read.
 * These markers determine whether the thread then blocks on the queue semaphore,
 * consumes bytes, sends receiver credit, sleeps waiting for more bytes, or
 * continues looping normally.
 */
#define H1_RFB_DIAG_READ_LOOP_ENTER          0xE1060001u
#define H1_RFB_DIAG_QUEUE_WAIT_ENTER         0xE1060002u
#define H1_RFB_DIAG_QUEUE_WAIT_RETURN        0xE1060003u
#define H1_RFB_DIAG_QUEUE_READ_RETURN        0xE1060004u
#define H1_RFB_DIAG_QUEUE_SIGNAL_ENTER       0xE1060005u
#define H1_RFB_DIAG_QUEUE_SIGNAL_RETURN      0xE1060006u
#define H1_RFB_DIAG_CREDIT_SEND_ENTER        0xE1060007u
#define H1_RFB_DIAG_CREDIT_SEND_RETURN       0xE1060008u
#define H1_RFB_DIAG_READ_PROGRESS            0xE1060009u
#define H1_RFB_DIAG_READ_DELAY_ENTER         0xE106000Au
#define H1_RFB_DIAG_READ_DELAY_RETURN        0xE106000Bu

/* Implemented by h1_transport_runtime.c around its existing send semaphore. */
int pstvnc_h1_transport_send_frame_internal(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    const void *payload,
    size_t payload_length);

static void h1_rfb_record_error(
    pstvnc_h1_transport_runtime_t *runtime,
    pstvnc_h1_transport_error_t error)
{
    if (runtime != NULL && runtime->error == PSTVNC_H1_ERROR_NONE)
        runtime->error = error;
}

static int h1_rfb_policy_valid(
    const pstvnc_h1_transport_runtime_t *runtime)
{
    const pstvnc_h1_config_t *config;

    if (runtime == NULL)
        return 0;

    config = &runtime->config;

    if (config->rfb_mode == PSTVNC_H1_RFB_OFF)
        return config->rfb_queue_capacity == 0u &&
            config->rfb_credit_batch_bytes == 0u &&
            config->rfb_credit_flush_on_empty == 0u &&
            config->rfb_credit_return_enabled == 0u &&
            config->rfb_initial_credit_bytes == 0u;

    if (config->rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        config->rfb_queue_capacity < config->max_data_payload ||
        config->rfb_initial_credit_bytes > config->rfb_queue_capacity ||
        config->rfb_credit_flush_on_empty > 1u ||
        config->rfb_credit_return_enabled > 1u)
        return 0;

    if (config->rfb_credit_return_enabled != 0u &&
        (config->rfb_credit_batch_bytes == 0u ||
         config->rfb_credit_batch_bytes > config->rfb_queue_capacity))
        return 0;

    return 1;
}

int pstvnc_h1_rfb_transport_prepare(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int enabled;
    uint32_t capacity;

    if (runtime == NULL)
        return 0;

    pstvnc_h1_rfb_runtime_resources_init(&runtime->rfb_resources);

    if (!h1_rfb_policy_valid(runtime)) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CONFIG);
        return 0;
    }

    enabled = runtime->config.rfb_mode == PSTVNC_H1_RFB_ON_RESERVED;
    capacity = enabled ? runtime->config.rfb_queue_capacity : 0u;

    if (!pstvnc_h1_rfb_runtime_resources_activate(
            &runtime->rfb_resources,
            enabled,
            capacity)) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_ALLOCATION);
        return 0;
    }

    if (enabled && !pstvnc_h1_rfb_mux_io_bind(runtime)) {
        (void)pstvnc_h1_rfb_runtime_resources_release(
            &runtime->rfb_resources);
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return 0;
    }

    return 1;
}

int pstvnc_h1_rfb_transport_release(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0;

    pstvnc_h1_rfb_mux_io_unbind(runtime);
    return pstvnc_h1_rfb_runtime_resources_release(
        &runtime->rfb_resources);
}

static int h1_rfb_accept_quiesce_marker(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime->rfb_quiesce_request_received == 0u) {
        if (runtime->rfb_quiesce_boundary_sent != 0u ||
            runtime->rfb_quiesce_commit_received != 0u ||
            runtime->rfb_quiesce_complete_sent != 0u) {
            h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
            return 0;
        }

        runtime->rfb_quiesce_request_received = 1u;
        return 1;
    }

    if (runtime->rfb_quiesce_boundary_sent != 0u &&
        runtime->rfb_quiesce_commit_received == 0u &&
        runtime->rfb_quiesce_complete_sent == 0u) {
        runtime->rfb_quiesce_commit_received = 1u;
        return 1;
    }

    h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
    return 0;
}

int pstvnc_h1_rfb_transport_activity_snapshot(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    if (runtime == NULL ||
        activity_sequence == NULL ||
        !runtime->rfb_resources.active ||
        runtime->rfb_resources.queue_sema_id < 0)
        return 0;

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    *activity_sequence =
        runtime->rfb_resources.activity_sequence;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    return 1;
}

int pstvnc_h1_rfb_transport_notify_activity(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int waiter_thread_id = -1;
    int witness_lifecycle = 0;

    if (runtime == NULL ||
        !runtime->rfb_resources.active ||
        runtime->rfb_resources.queue_sema_id < 0)
        return 0;

    witness_lifecycle =
        runtime->receiver_diag_stage ==
            PSTVNC_H1_RX_DIAG_RFB_NOTIFY_ENTER;

    if (witness_lifecycle)
        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_WAIT_ENTER;

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (witness_lifecycle)
        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_WAIT_RETURN;

    runtime->rfb_resources.activity_sequence++;

    if (runtime->rfb_resources.wait_thread_id >= 0) {
        waiter_thread_id =
            runtime->rfb_resources.wait_thread_id;
        runtime->rfb_resources.wait_thread_id = -1;
    }

    if (witness_lifecycle)
        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_SIGNAL_ENTER;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (witness_lifecycle)
        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_SIGNAL_RETURN;

    /*
     * Clearing the waiter before WakeupThread coalesces concurrent producers.
     * Wake-before-SleepThread is safe because EE wakeup count is pending state,
     * the same ordering already hardware-qualified by the MPEG event wake.
     */
    if (waiter_thread_id >= 0) {
        if (witness_lifecycle)
            runtime->receiver_diag_stage =
                PSTVNC_H1_RX_DIAG_RFB_WAKE_ENTER;

        if (WakeupThread(waiter_thread_id) < 0) {
            h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_THREAD_DELAY);
            return 0;
        }

        if (witness_lifecycle)
            runtime->receiver_diag_stage =
                PSTVNC_H1_RX_DIAG_RFB_WAKE_RETURN;
    }

    return 1;
}

int pstvnc_h1_rfb_transport_wait_for_activity(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    int current_thread_id;

    if (runtime == NULL ||
        activity_sequence == NULL ||
        !runtime->rfb_resources.active ||
        runtime->rfb_resources.queue_sema_id < 0)
        return 0;

    current_thread_id = GetThreadId();
    if (current_thread_id < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_THREAD_DELAY);
        return 0;
    }

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    /*
     * This comparison closes the producer-before-registration race. A producer
     * that ran since the caller's snapshot already changed the ticket, so the
     * owner returns immediately instead of sleeping through that work.
     */
    if (runtime->rfb_resources.activity_sequence !=
            *activity_sequence ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->receiver_done ||
        runtime->stop_requested) {

        *activity_sequence =
            runtime->rfb_resources.activity_sequence;

        if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
            h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        return 1;
    }

    if (runtime->rfb_resources.wait_thread_id >= 0) {
        (void)SignalSema(runtime->rfb_resources.queue_sema_id);
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    runtime->rfb_resources.wait_thread_id =
        current_thread_id;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (SleepThread() < 0) {
        if (WaitSema(runtime->rfb_resources.queue_sema_id) >= 0) {
            if (runtime->rfb_resources.wait_thread_id ==
                    current_thread_id)
                runtime->rfb_resources.wait_thread_id = -1;

            (void)SignalSema(
                runtime->rfb_resources.queue_sema_id);
        }

        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_THREAD_DELAY);
        return 0;
    }

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    /*
     * A normal producer clears this before waking us. Clear it here as well so
     * a spurious/pending wake can never leave a stale waiter registration.
     */
    if (runtime->rfb_resources.wait_thread_id ==
            current_thread_id)
        runtime->rfb_resources.wait_thread_id = -1;

    *activity_sequence =
        runtime->rfb_resources.activity_sequence;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    return 1;
}

int pstvnc_h1_rfb_transport_accept_data(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *payload,
    size_t payload_length)
{
    int accepted;

    if (runtime == NULL ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active ||
        payload_length > runtime->config.max_data_payload) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    /*
     * Zero bytes are lifecycle control and never enter the raw RFB queue, but
     * REQUEST/COMMIT are still activity for the sole RFB owner.
     */
    if (payload_length == 0u) {
        if (!h1_rfb_accept_quiesce_marker(runtime))
            return 0;

        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_MARKER_ACCEPTED;
        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_NOTIFY_ENTER;

        accepted = pstvnc_h1_rfb_transport_notify_activity(runtime);

        runtime->receiver_diag_stage =
            PSTVNC_H1_RX_DIAG_RFB_NOTIFY_RETURN;
        return accepted;
    }

    if (payload == NULL) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    accepted = pstvnc_h1_rfb_channel_accept_data(
        &runtime->rfb_resources.channel,
        payload,
        payload_length);

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (!accepted) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
        return 0;
    }

    return pstvnc_h1_rfb_transport_notify_activity(runtime);
}

static int h1_rfb_record_credit_sent(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t amount)
{
    pstvnc_h1_rfb_channel_stats_t *stats;

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    stats = &runtime->rfb_resources.channel.stats;
    if (stats->credit_frames_sent == 0xffffffffu ||
        stats->credit_bytes_sent > 0xffffffffu - amount) {
        (void)SignalSema(runtime->rfb_resources.queue_sema_id);
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
        return 0;
    }

    stats->credit_frames_sent++;
    stats->credit_bytes_sent += amount;

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    return 1;
}

static int h1_rfb_send_credit(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    if (amount == 0u)
        return 1;

    pstvnc_transport_write_be32(payload, amount);
    if (!pstvnc_h1_transport_send_frame_internal(
            runtime,
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            payload,
            sizeof(payload)))
        return 0;

    return h1_rfb_record_credit_sent(runtime, amount);
}

int pstvnc_h1_rfb_transport_send_initial_credit(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0;

    if (runtime->config.rfb_mode == PSTVNC_H1_RFB_OFF)
        return 1;

    if (!runtime->rfb_resources.active) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    return h1_rfb_send_credit(
        runtime,
        runtime->config.rfb_initial_credit_bytes);
}

static size_t h1_rfb_read_available_and_credit(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count)
{
    pstvnc_h1_rfb_channel_t *channel;
    size_t taken;
    int queue_empty;
    uint32_t credit = 0u;

    pstvnc_h1_rfb_mux_io_diag_stage(
        runtime->socket_fd,
        H1_RFB_DIAG_QUEUE_WAIT_ENTER,
        maximum_count);

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0u;
    }

    pstvnc_h1_rfb_mux_io_diag_stage(
        runtime->socket_fd,
        H1_RFB_DIAG_QUEUE_WAIT_RETURN,
        pstvnc_h1_rfb_channel_queue_size(
            &runtime->rfb_resources.channel));

    channel = &runtime->rfb_resources.channel;
    taken = pstvnc_h1_rfb_channel_read_available(
        channel,
        buffer,
        maximum_count);
    queue_empty = pstvnc_h1_rfb_channel_queue_size(channel) == 0u;

    pstvnc_h1_rfb_mux_io_diag_stage(
        runtime->socket_fd,
        H1_RFB_DIAG_QUEUE_READ_RETURN,
        taken);

    if (pstvnc_h1_rfb_credit_should_return(
            channel->stats.credit_bytes_pending,
            runtime->config.rfb_credit_batch_bytes,
            runtime->config.rfb_credit_flush_on_empty != 0u,
            runtime->config.rfb_credit_return_enabled != 0u,
            queue_empty))
        credit = pstvnc_h1_rfb_channel_take_credit(channel);

    pstvnc_h1_rfb_mux_io_diag_stage(
        runtime->socket_fd,
        H1_RFB_DIAG_QUEUE_SIGNAL_ENTER,
        channel->stats.credit_bytes_pending);

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0u;
    }

    pstvnc_h1_rfb_mux_io_diag_stage(
        runtime->socket_fd,
        H1_RFB_DIAG_QUEUE_SIGNAL_RETURN,
        credit);

    if (credit != 0u) {
        pstvnc_h1_rfb_mux_io_diag_stage(
            runtime->socket_fd,
            H1_RFB_DIAG_CREDIT_SEND_ENTER,
            credit);

        if (!h1_rfb_send_credit(runtime, credit))
            return 0u;

        pstvnc_h1_rfb_mux_io_diag_stage(
            runtime->socket_fd,
            H1_RFB_DIAG_CREDIT_SEND_RETURN,
            credit);
    }

    return taken;
}

int pstvnc_h1_rfb_transport_read_exact(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    uint32_t activity_sequence;
    size_t done = 0u;

    if (runtime == NULL ||
        (buffer == NULL && count != 0u) ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active)
        return -1;

    if (!pstvnc_h1_rfb_transport_activity_snapshot(
            runtime,
            &activity_sequence))
        return -1;

    while (done < count) {
        size_t taken;
        size_t remaining = count - done;

        pstvnc_h1_rfb_mux_io_diag_stage(
            runtime->socket_fd,
            H1_RFB_DIAG_READ_LOOP_ENTER,
            remaining);

        if (runtime->error != PSTVNC_H1_ERROR_NONE ||
            runtime->stop_requested)
            return -1;

        taken = h1_rfb_read_available_and_credit(
            runtime,
            destination + done,
            remaining);

        if (taken != 0u) {
            done += taken;

            pstvnc_h1_rfb_mux_io_diag_stage(
                runtime->socket_fd,
                H1_RFB_DIAG_READ_PROGRESS,
                count - done);

            continue;
        }

        if (runtime->error != PSTVNC_H1_ERROR_NONE ||
            runtime->receiver_done)
            return -1;

        /*
         * Preserve the existing diagnostic marker identity so old witness
         * tooling remains useful. These markers now bracket an event wait,
         * not DelayThread().
         */
        pstvnc_h1_rfb_mux_io_diag_stage(
            runtime->socket_fd,
            H1_RFB_DIAG_READ_DELAY_ENTER,
            count - done);

        if (!pstvnc_h1_rfb_transport_wait_for_activity(
                runtime,
                &activity_sequence))
            return -1;

        pstvnc_h1_rfb_mux_io_diag_stage(
            runtime->socket_fd,
            H1_RFB_DIAG_READ_DELAY_RETURN,
            count - done);
    }

    return 0;
}

int pstvnc_h1_rfb_transport_poll_receive(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int available;

    if (runtime == NULL ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active)
        return -1;

    if (WaitSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    available = pstvnc_h1_rfb_channel_poll(
        &runtime->rfb_resources.channel);

    if (SignalSema(runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    if (available > 0)
        return 1;

    if (runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->receiver_done ||
        runtime->stop_requested)
        return -1;

    return 0;
}

static int h1_rfb_send_fragment(
    void *context,
    const void *payload,
    size_t payload_length)
{
    return pstvnc_h1_transport_send_frame_internal(
        (pstvnc_h1_transport_runtime_t *)context,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        payload,
        payload_length);
}

int pstvnc_h1_rfb_transport_write_exact(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    if (runtime == NULL ||
        (buffer == NULL && count != 0u) ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active ||
        runtime->error != PSTVNC_H1_ERROR_NONE)
        return -1;

    if (!pstvnc_h1_rfb_channel_write_logical(
            &runtime->rfb_resources.channel,
            buffer,
            count,
            runtime->config.max_data_payload,
            h1_rfb_send_fragment,
            runtime)) {
        if (runtime->error == PSTVNC_H1_ERROR_NONE)
            h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
        return -1;
    }

    return 0;
}

static int h1_rfb_send_quiesce_marker(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return pstvnc_h1_transport_send_frame_internal(
        runtime,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        NULL,
        0u);
}


int pstvnc_h1_rfb_transport_discard_quiesce_residual(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t expected_queue_bytes,
    uint32_t *bytes_discarded)
{
    int ok;

    if (bytes_discarded != NULL)
        *bytes_discarded = 0u;

    if (runtime == NULL ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active ||
        runtime->rfb_quiesce_commit_received == 0u ||
        runtime->error != PSTVNC_H1_ERROR_NONE)
        return 0;

    if (WaitSema(
            runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(
            runtime,
            PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    ok = pstvnc_h1_rfb_channel_discard_quiesce_residual(
        &runtime->rfb_resources.channel,
        (size_t)expected_queue_bytes,
        bytes_discarded);

    if (SignalSema(
            runtime->rfb_resources.queue_sema_id) < 0) {
        h1_rfb_record_error(
            runtime,
            PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (!ok) {
        h1_rfb_record_error(
            runtime,
            PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    return 1;
}

int pstvnc_h1_rfb_transport_send_quiesce_boundary(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active ||
        runtime->rfb_quiesce_request_received == 0u ||
        runtime->rfb_quiesce_boundary_sent != 0u ||
        runtime->rfb_quiesce_commit_received != 0u ||
        runtime->rfb_quiesce_complete_sent != 0u) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    if (!h1_rfb_send_quiesce_marker(runtime))
        return 0;

    runtime->rfb_quiesce_boundary_sent = 1u;
    return 1;
}

int pstvnc_h1_rfb_transport_send_quiesce_complete(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        runtime->config.rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        !runtime->rfb_resources.active ||
        runtime->rfb_quiesce_request_received == 0u ||
        runtime->rfb_quiesce_boundary_sent == 0u ||
        runtime->rfb_quiesce_commit_received == 0u ||
        runtime->rfb_quiesce_complete_sent != 0u) {
        h1_rfb_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    if (!h1_rfb_send_quiesce_marker(runtime))
        return 0;

    runtime->rfb_quiesce_complete_sent = 1u;
    return 1;
}

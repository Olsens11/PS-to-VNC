/*
 * File synopsis:
 * Implements Transport's session-local sole receiver and synchronized logical
 * RFB/AUDIO runtime. One EE receiver thread is the only caller of the physical
 * receive primitive; complete channel-1 DATA feeds RFB and optional channel-2
 * DATA feeds an independent bounded AUDIO queue with its own credit/wakeup state.
 *
 * Parser-consumed RFB bytes, finite AUDIO producer completion, outbound logical
 * RFB fragmentation, and receiver completion all remain Transport-owned. RFB
 * parsing, PCM/AUDSRV playback, common-clock policy, and application recovery
 * remain outside this file.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#include "runtime.h"

#include <kernel.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

static int pstvnc_transport_runtime_audio_config_valid(
    const pstvnc_transport_runtime_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    if (config == NULL || audio_config == NULL ||
        audio_config->queue_capacity == 0u ||
        config->max_data_payload > audio_config->queue_capacity ||
        audio_config->initial_credit_bytes > audio_config->queue_capacity ||
        (audio_config->credit_flush_on_empty != 0 &&
         audio_config->credit_flush_on_empty != 1) ||
        (audio_config->credit_return_enabled != 0 &&
         audio_config->credit_return_enabled != 1))
        return 0;

    if (audio_config->credit_return_enabled &&
        (audio_config->credit_batch_bytes == 0u ||
         audio_config->credit_batch_bytes > audio_config->queue_capacity))
        return 0;

    if (!audio_config->credit_return_enabled &&
        audio_config->credit_batch_bytes != 0u)
        return 0;

    return 1;
}

static void pstvnc_transport_runtime_reset_identifiers(
    pstvnc_transport_runtime_t *runtime)
{
    runtime->physical_stream.socket_fd = -1;
    runtime->physical_stream.send_semaphore_id = -1;
    runtime->rfb_queue_semaphore_id = -1;
    runtime->rfb_activity_semaphore_id = -1;
    runtime->audio_queue_semaphore_id = -1;
    runtime->audio_activity_semaphore_id = -1;
    runtime->receiver_done_semaphore_id = -1;
    runtime->receiver_thread_id = -1;
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
    return pstvnc_transport_physical_stream_send_frame(
        &runtime->physical_stream,
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

/* Caller holds audio_queue_semaphore_id. */
static int pstvnc_transport_runtime_publish_audio_activity_locked(
    pstvnc_transport_runtime_t *runtime)
{
    int signal_waiter = 0;

    runtime->audio_activity_sequence++;
    if (runtime->audio_activity_wait_armed) {
        runtime->audio_activity_wait_armed = 0;
        signal_waiter = 1;
    }

    return signal_waiter;
}

static int pstvnc_transport_runtime_signal_audio_activity(
    pstvnc_transport_runtime_t *runtime,
    int signal_waiter)
{
    if (!signal_waiter)
        return 1;

    if (SignalSema(runtime->audio_activity_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_publish_audio_terminal(
    pstvnc_transport_runtime_t *runtime)
{
    int signal_waiter;

    if (!runtime->audio_enabled)
        return 1;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    signal_waiter = pstvnc_transport_runtime_publish_audio_activity_locked(runtime);
    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return pstvnc_transport_runtime_signal_audio_activity(runtime, signal_waiter);
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
        ? pstvnc_transport_runtime_publish_audio_activity_locked(runtime)
        : 0;

    if (SignalSema(runtime->audio_queue_semaphore_id) < 0)
        return 0;

    if (!accepted)
        return 0;

    return pstvnc_transport_runtime_signal_audio_activity(runtime, signal_waiter);
}

static void pstvnc_transport_runtime_receiver_thread(void *argument)
{
    pstvnc_transport_runtime_t *runtime =
        (pstvnc_transport_runtime_t *)argument;

    while (!runtime->failed && !runtime->stop_requested) {
        pstvnc_transport_header_t header;
        int accepted = 0;

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
                accepted = pstvnc_transport_runtime_accept_rfb_frame(runtime, &header);
            else if (header.channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO)
                accepted = pstvnc_transport_runtime_accept_audio_frame(runtime, &header);
        }

        if (!accepted) {
            runtime->failed = 1;
            break;
        }
    }

    runtime->receiver_done = 1;

    /* Wake the RFB owner so terminal receiver state is observed without polling. */
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

    /* The same terminal point wakes the independent logical AUDIO waiter. */
    (void)pstvnc_transport_runtime_publish_audio_terminal(runtime);

    if (runtime->receiver_done_semaphore_id >= 0 &&
        SignalSema(runtime->receiver_done_semaphore_id) < 0)
        runtime->failed = 1;

    ExitThread();
}

static int pstvnc_transport_runtime_initialize_internal(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_runtime_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config)
{
    if (runtime == NULL || socket_fd < 0 ||
        !pstvnc_transport_runtime_config_valid(config) ||
        (audio_config != NULL &&
         !pstvnc_transport_runtime_audio_config_valid(config, audio_config)))
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

    runtime->receiver_done_semaphore_id =
        pstvnc_transport_runtime_create_semaphore(0, 1);
    if (runtime->receiver_done_semaphore_id < 0)
        goto fail;

    runtime->receiver_stack =
        (unsigned char *)pstvnc_transport_runtime_allocate_aligned16(
            (size_t)config->receiver_thread_stack_size,
            &runtime->receiver_stack_allocation);
    if (runtime->receiver_stack == NULL)
        goto fail;

    if (!pstvnc_transport_physical_stream_adopt(
            &runtime->physical_stream, socket_fd))
        goto fail;

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

    runtime->receiver_thread_stack_size = config->receiver_thread_stack_size;
    runtime->receiver_thread_priority = config->receiver_thread_priority;
    runtime->max_data_payload = config->max_data_payload;
    runtime->initialized = 1;
    return 1;

fail:
    if (runtime->receiver_done_semaphore_id >= 0)
        (void)DeleteSema(runtime->receiver_done_semaphore_id);
    if (runtime->audio_activity_semaphore_id >= 0)
        (void)DeleteSema(runtime->audio_activity_semaphore_id);
    if (runtime->audio_queue_semaphore_id >= 0)
        (void)DeleteSema(runtime->audio_queue_semaphore_id);
    if (runtime->rfb_activity_semaphore_id >= 0)
        (void)DeleteSema(runtime->rfb_activity_semaphore_id);
    if (runtime->rfb_queue_semaphore_id >= 0)
        (void)DeleteSema(runtime->rfb_queue_semaphore_id);
    free(runtime->receiver_stack_allocation);
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
        runtime, socket_fd, config, NULL);
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
        runtime, socket_fd, config, audio_config);
}

int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime)
{
    ee_thread_t thread;

    if (runtime == NULL || !runtime->initialized ||
        runtime->receiver_thread_started || runtime->failed ||
        runtime->stop_requested)
        return 0;

    if (!pstvnc_transport_runtime_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            runtime->rfb_initial_credit_bytes)) {
        runtime->failed = 1;
        return 0;
    }

    if (runtime->audio_enabled &&
        !pstvnc_transport_runtime_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            runtime->audio_initial_credit_bytes)) {
        runtime->failed = 1;
        return 0;
    }

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

    runtime->receiver_thread_started = 1;
    return 1;
}

int pstvnc_transport_runtime_request_stop(
    pstvnc_transport_runtime_t *runtime)
{
    int wake_ok;
    int shutdown_ok;

    if (runtime == NULL || !runtime->initialized ||
        !runtime->receiver_thread_started)
        return 0;

    if (runtime->receiver_done)
        return 1;

    runtime->stop_requested = 1;
    wake_ok = pstvnc_transport_runtime_publish_audio_terminal(runtime);
    shutdown_ok = pstvnc_transport_physical_stream_shutdown_io(
        &runtime->physical_stream);
    return wake_ok && shutdown_ok;
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

static int pstvnc_transport_runtime_return_rfb_credit(
    pstvnc_transport_runtime_t *runtime,
    uint32_t consumed,
    int queue_empty)
{
    uint32_t amount = 0u;

    if (!runtime->rfb_credit_return_enabled)
        return 1;

    if (runtime->rfb_credit_pending > UINT32_MAX - consumed) {
        runtime->failed = 1;
        return 0;
    }

    runtime->rfb_credit_pending += consumed;
    if (runtime->rfb_credit_pending >= runtime->rfb_credit_batch_bytes ||
        (runtime->rfb_credit_flush_on_empty && queue_empty &&
         runtime->rfb_credit_pending != 0u)) {
        amount = runtime->rfb_credit_pending;
        runtime->rfb_credit_pending = 0u;
    }

    if (amount != 0u &&
        !pstvnc_transport_runtime_send_credit(
            runtime, PSTVNC_TRANSPORT_CHANNEL_RFB, amount)) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

static int pstvnc_transport_runtime_return_audio_credit(
    pstvnc_transport_runtime_t *runtime,
    uint32_t consumed,
    int queue_empty)
{
    uint32_t amount = 0u;

    if (!runtime->audio_credit_return_enabled)
        return 1;

    if (runtime->audio_credit_pending > UINT32_MAX - consumed) {
        runtime->failed = 1;
        return 0;
    }

    runtime->audio_credit_pending += consumed;
    if (runtime->audio_credit_pending >= runtime->audio_credit_batch_bytes ||
        (runtime->audio_credit_flush_on_empty && queue_empty &&
         runtime->audio_credit_pending != 0u)) {
        amount = runtime->audio_credit_pending;
        runtime->audio_credit_pending = 0u;
    }

    if (amount != 0u &&
        !pstvnc_transport_runtime_send_credit(
            runtime, PSTVNC_TRANSPORT_CHANNEL_AUDIO, amount)) {
        runtime->failed = 1;
        (void)pstvnc_transport_runtime_publish_audio_terminal(runtime);
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
        size_t remaining = count - offset;
        size_t fragment = remaining;

        if (fragment > (size_t)runtime->max_data_payload)
            fragment = (size_t)runtime->max_data_payload;

        if (!pstvnc_transport_physical_stream_send_frame(
                &runtime->physical_stream,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0u,
                bytes + offset,
                fragment)) {
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

int pstvnc_transport_runtime_audio_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    if (runtime == NULL || activity_sequence == NULL ||
        !runtime->initialized || !runtime->audio_enabled)
        return 0;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0)
        return 0;

    *activity_sequence = runtime->audio_activity_sequence;

    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
}

int pstvnc_transport_runtime_audio_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence)
{
    if (runtime == NULL || activity_sequence == NULL ||
        !runtime->initialized || !runtime->audio_enabled)
        return 0;

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0)
        return 0;

    if (runtime->audio_activity_sequence != *activity_sequence ||
        runtime->receiver_done || runtime->failed || runtime->stop_requested) {
        *activity_sequence = runtime->audio_activity_sequence;
        if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
            runtime->failed = 1;
            return 0;
        }
        return 1;
    }

    if (runtime->audio_activity_wait_armed) {
        (void)SignalSema(runtime->audio_queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    runtime->audio_activity_wait_armed = 1;
    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (WaitSema(runtime->audio_activity_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (WaitSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    if (runtime->audio_activity_wait_armed) {
        runtime->audio_activity_wait_armed = 0;
        (void)SignalSema(runtime->audio_queue_semaphore_id);
        runtime->failed = 1;
        return 0;
    }

    *activity_sequence = runtime->audio_activity_sequence;
    if (SignalSema(runtime->audio_queue_semaphore_id) < 0) {
        runtime->failed = 1;
        return 0;
    }

    return 1;
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

int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime)
{
    int result = 1;

    if (runtime == NULL || !runtime->initialized)
        return 0;

    if (runtime->receiver_thread_started && !runtime->receiver_done)
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

    if (runtime->receiver_done_semaphore_id >= 0 &&
        DeleteSema(runtime->receiver_done_semaphore_id) < 0)
        result = 0;
    if (runtime->audio_activity_semaphore_id >= 0 &&
        DeleteSema(runtime->audio_activity_semaphore_id) < 0)
        result = 0;
    if (runtime->audio_queue_semaphore_id >= 0 &&
        DeleteSema(runtime->audio_queue_semaphore_id) < 0)
        result = 0;
    if (runtime->rfb_activity_semaphore_id >= 0 &&
        DeleteSema(runtime->rfb_activity_semaphore_id) < 0)
        result = 0;
    if (runtime->rfb_queue_semaphore_id >= 0 &&
        DeleteSema(runtime->rfb_queue_semaphore_id) < 0)
        result = 0;

    free(runtime->receiver_stack_allocation);
    free(runtime->audio_queue_storage);
    free(runtime->rfb_queue_storage);
    pstvnc_transport_physical_stream_release(&runtime->physical_stream);

    memset(runtime, 0, sizeof(*runtime));
    pstvnc_transport_runtime_reset_identifiers(runtime);
    return result;
}

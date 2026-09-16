/*
 * File synopsis:
 * Defines Transport's session-local runtime above the physical PSTV stream.
 * The runtime owns the sole receiver thread, synchronized logical RFB storage,
 * optional synchronized logical AUDIO storage, independent per-channel flow
 * control/activity rendezvous, and the receiver completion event required before
 * receiver-touched resources can be reclaimed.
 *
 * This is an internal Transport boundary. It does not parse RFB, play PCM,
 * decide product recovery policy, expose the physical socket, or use diagnostic
 * counters as synchronization authority. Cross-component session values arrive
 * through stable caller-supplied types in transport.h.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_TRANSPORT_RUNTIME_H
#define PSTVNC_TRANSPORT_RUNTIME_H

#include "audio_channel.h"
#include "physical_stream.h"
#include "rfb_channel.h"
#include "transport.h"

#include <stddef.h>
#include <stdint.h>

typedef pstvnc_transport_session_config_t pstvnc_transport_runtime_config_t;

typedef struct pstvnc_transport_runtime {
    pstvnc_transport_physical_stream_t physical_stream;
    pstvnc_transport_rfb_channel_t rfb_channel;
    pstvnc_transport_audio_channel_t audio_channel;

    uint8_t *rfb_queue_storage;
    uint8_t *audio_queue_storage;
    void *receiver_stack_allocation;
    unsigned char *receiver_stack;

    int rfb_queue_semaphore_id;
    int rfb_activity_semaphore_id;
    int audio_queue_semaphore_id;
    int audio_activity_semaphore_id;
    int receiver_done_semaphore_id;
    int receiver_thread_id;

    int initialized;
    int audio_enabled;
    int receiver_thread_started;
    volatile int receiver_done;
    volatile int stop_requested;
    volatile int failed;

    /* RFB producer activity is protected by rfb_queue_semaphore_id. */
    uint32_t activity_sequence;
    int activity_wait_armed;

    /* AUDIO producer/terminal activity is protected by audio_queue_semaphore_id. */
    uint32_t audio_activity_sequence;
    int audio_activity_wait_armed;
    int audio_activity_waiter_active;

    uint32_t rfb_credit_pending;
    uint32_t rfb_initial_credit_bytes;
    uint32_t rfb_credit_batch_bytes;
    int rfb_credit_flush_on_empty;
    int rfb_credit_return_enabled;

    uint32_t audio_credit_pending;
    uint32_t audio_initial_credit_bytes;
    uint32_t audio_credit_batch_bytes;
    int audio_credit_flush_on_empty;
    int audio_credit_return_enabled;

    uint32_t max_data_payload;
    uint32_t receiver_thread_stack_size;
    int receiver_thread_priority;

    /*
     * Zero-length channel-1 DATA carries only the audited finite-session RFB
     * request/commit markers. These flags are product synchronization state,
     * not diagnostics. AUDIO finite completion is owned by audio_channel.
     */
    volatile uint32_t rfb_quiesce_request_received;
    volatile uint32_t rfb_quiesce_boundary_sent;
    volatile uint32_t rfb_quiesce_commit_received;
    volatile uint32_t rfb_quiesce_complete_sent;

    uint8_t receiver_payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} pstvnc_transport_runtime_t;

/* Preserve the existing RFB-only initialization behavior. */
int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config);

/* Opt in to one logical AUDIO channel using explicit caller/profile values. */
int pstvnc_transport_runtime_initialize_with_audio(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config);

int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_request_stop(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_rfb_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);
int pstvnc_transport_runtime_rfb_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);

int pstvnc_transport_runtime_rfb_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count);
int pstvnc_transport_runtime_rfb_poll_receive(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_rfb_write_exact(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count);

/*
 * Logical AUDIO consumer seam. read_available() is nonblocking and bounded:
 * OK returns bytes, WOULD_BLOCK means live producer/no bytes, EXHAUSTED means
 * the one-shot producer marker is visible and the queue is empty, STOPPED means
 * session stop, and FAILED/CLOSED are terminal Transport outcomes.
 */
pstvnc_transport_result_t pstvnc_transport_runtime_audio_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_runtime_audio_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done);
int pstvnc_transport_runtime_audio_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);
int pstvnc_transport_runtime_audio_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);

int pstvnc_transport_runtime_rfb_quiesce_requested(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_rfb_send_quiesce_boundary(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_rfb_wait_quiesce_commit(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_rfb_snapshot_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t *residual_count);
int pstvnc_transport_runtime_rfb_discard_quiesce_residual(
    pstvnc_transport_runtime_t *runtime,
    size_t expected_count,
    size_t *discarded_count);
int pstvnc_transport_runtime_rfb_send_quiesce_complete(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_wait_receiver_done(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime);

#endif

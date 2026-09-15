/*
 * File synopsis:
 * Defines Transport's session-local runtime above the physical PSTV stream.
 * The runtime owns the sole receiver thread, synchronized logical RFB storage,
 * producer-driven activity rendezvous, RFB flow-control state, and the receiver
 * completion event needed before receiver-touched resources can be reclaimed.
 *
 * This is an internal Transport boundary. It does not parse RFB, decide product
 * recovery policy, expose the physical socket, or use diagnostic counters as
 * synchronization authority.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#ifndef PSTVNC_TRANSPORT_RUNTIME_H
#define PSTVNC_TRANSPORT_RUNTIME_H

#include "physical_stream.h"
#include "rfb_channel.h"

#include <stddef.h>
#include <stdint.h>

typedef struct pstvnc_transport_runtime_config {
    uint32_t rfb_queue_capacity;
    uint32_t rfb_initial_credit_bytes;
    uint32_t rfb_credit_batch_bytes;
    int rfb_credit_flush_on_empty;
    int rfb_credit_return_enabled;
    uint32_t receiver_thread_stack_size;
    int receiver_thread_priority;
    uint32_t max_data_payload;
} pstvnc_transport_runtime_config_t;

typedef struct pstvnc_transport_runtime {
    pstvnc_transport_physical_stream_t physical_stream;
    pstvnc_transport_rfb_channel_t rfb_channel;

    uint8_t *rfb_queue_storage;
    void *receiver_stack_allocation;
    unsigned char *receiver_stack;

    int rfb_queue_semaphore_id;
    int rfb_activity_semaphore_id;
    int receiver_done_semaphore_id;
    int receiver_thread_id;

    int initialized;
    int receiver_thread_started;
    volatile int receiver_done;
    volatile int failed;

    /*
     * activity_sequence and activity_wait_armed are protected by the RFB queue
     * semaphore. Producers advance the sequence after committed work becomes
     * visible and signal the event only when one consumer wait is armed.
     */
    uint32_t activity_sequence;
    int activity_wait_armed;

    uint32_t rfb_credit_pending;
    uint32_t rfb_initial_credit_bytes;
    uint32_t rfb_credit_batch_bytes;
    int rfb_credit_flush_on_empty;
    int rfb_credit_return_enabled;
    uint32_t max_data_payload;

    uint32_t receiver_thread_stack_size;
    int receiver_thread_priority;

    /*
     * Zero-length channel-1 DATA carries only the audited finite-session RFB
     * request/commit markers. These flags are product synchronization state,
     * not diagnostics.
     */
    volatile uint32_t rfb_quiesce_request_received;
    volatile uint32_t rfb_quiesce_boundary_sent;
    volatile uint32_t rfb_quiesce_commit_received;
    volatile uint32_t rfb_quiesce_complete_sent;

    uint8_t receiver_payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} pstvnc_transport_runtime_t;

/*
 * Adopt one physical socket and allocate the enabled logical-RFB/session
 * resources. The caller supplies already validated session configuration.
 */
int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_runtime_config_t *config);

/* Start the sole physical receiver after all receiver-visible resources exist. */
int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime);

/* Snapshot/wait pair for timer-free producer activity rendezvous. */
int pstvnc_transport_runtime_rfb_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);
int pstvnc_transport_runtime_rfb_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);

/* Logical RFB operations; no caller receives physical socket authority. */
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
 * Ordered finite-RFB shutdown: Pi REQUEST -> PS2 BOUNDARY -> Pi COMMIT -> PS2
 * COMPLETE. Residual bytes after COMMIT are snapshotted/discarded explicitly and
 * never earn parser-consumption credit.
 */
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

/*
 * Receiver completion is an explicit event. Resource release is legal only
 * after this proves the receiver can no longer touch channel/session state.
 */
int pstvnc_transport_runtime_wait_receiver_done(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_release(
    pstvnc_transport_runtime_t *runtime);

#endif

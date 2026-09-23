/*
 * File synopsis:
 * Defines Transport's session-local runtime above the physical PSTV stream.
 * The runtime owns the sole physical-I/O thread plus synchronized logical RFB
 * and optional AUDIO/MPEG2 storage, independent per-channel flow-control/activity
 * rendezvous, one typed RFB provider-terminal fact, one bounded MPEG RETIRE-
 * completion control slot, and the receiver completion event required before
 * receiver-touched resources can be reclaimed.
 *
 * This is an internal Transport boundary. It does not parse RFB, play PCM,
 * decode MPEG, decide product recovery/presentation policy, expose the physical
 * socket, or use diagnostic counters as synchronization authority. Cross-
 * component session values arrive through stable caller-supplied types in
 * transport.h.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md; docs/ledge/
 * LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md; docs/ledge/
 * LEDGE_AUDIT_A003_MPEG_GENERATION.md.
 */

#ifndef PSTVNC_TRANSPORT_RUNTIME_H
#define PSTVNC_TRANSPORT_RUNTIME_H

#include "audio_channel.h"
#include "mpeg_channel.h"
#include "physical_stream.h"
#include "rfb_channel.h"
#include "transport.h"

#include <stddef.h>
#include <stdint.h>

typedef pstvnc_transport_session_config_t pstvnc_transport_runtime_config_t;

typedef struct pstvnc_transport_outbound_work {
    uint8_t kind;
    uint8_t channel;
    uint8_t flags;
    size_t payload_length;
    uint8_t payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
    int result;
} pstvnc_transport_outbound_work_t;

typedef struct pstvnc_transport_runtime {
    pstvnc_transport_physical_stream_t physical_stream;
    pstvnc_transport_rfb_channel_t rfb_channel;
    pstvnc_transport_audio_channel_t audio_channel;
    pstvnc_transport_mpeg_channel_t mpeg_channel;

    uint8_t *rfb_queue_storage;
    uint8_t *audio_queue_storage;
    uint8_t *mpeg_queue_storage;
    void *receiver_stack_allocation;
    unsigned char *receiver_stack;

    int rfb_queue_semaphore_id;
    int rfb_activity_semaphore_id;
    /*
     * RFB-specific outbound-credit wake. The RFB queue semaphore protects both
     * the credit counter and this wait state; 0=no waiter, 1=armed, 2=signaled
     * but not yet returned through the protected state.
     */
    int rfb_outbound_credit_semaphore_id;
    int audio_queue_semaphore_id;
    int audio_activity_semaphore_id;
    int mpeg_queue_semaphore_id;
    int mpeg_activity_semaphore_id;
    int mpeg_control_semaphore_id;
    int receiver_done_semaphore_id;

    /*
     * One bounded synchronous outbound slot. Domain owners submit complete
     * logical frames here; only the Transport I/O thread calls the physical
     * framed-send primitive.
     */
    int outbound_slot_semaphore_id;
    int outbound_ready_semaphore_id;
    int outbound_done_semaphore_id;

    int receiver_thread_id;

    int initialized;
    int audio_enabled;
    int mpeg_enabled;
    int receiver_thread_started;
    volatile int receiver_done;
    volatile int stop_requested;
    volatile int failed;
    volatile int outbound_pending;

    /* RFB producer activity is protected by rfb_queue_semaphore_id. */
    uint32_t activity_sequence;
    int activity_wait_armed;

    /*
     * Pi-granted credit gates PS2->provider RFB DATA. This state is deliberately
     * RFB-specific: it does not establish a generic all-rider scheduler or
     * reusable module-generation mechanism.
     */
    uint32_t rfb_outbound_credit_bytes;
    int rfb_outbound_credit_wait_state;

    /*
     * First decoded channel-1 provider-terminal cause. NONE means the provider
     * is not known terminal. The same RFB queue semaphore protects this fact so
     * DATA-before-ERROR ordering, blocked-reader wake, and blocked-writer wake
     * share one session-local authority. It dies when this runtime is released.
     */
    pstvnc_rfb_provider_failure_reason_t rfb_provider_failure_reason;

    /*
     * AUDIO and MPEG producer/terminal activity use identical three-state
     * rendezvous lifetime fences under their own queue semaphores: 0 means no
     * waiter, 1 armed/not signaled, 2 signaled/not yet returned through the
     * queue lock. Resource release is legal only when the applicable state is 0.
     */
    uint32_t audio_activity_sequence;
    int audio_activity_wait_armed;
    uint32_t mpeg_activity_sequence;
    int mpeg_activity_wait_armed;

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

    uint32_t mpeg_credit_pending;
    uint32_t mpeg_initial_credit_bytes;
    uint32_t mpeg_credit_batch_bytes;
    int mpeg_credit_flush_on_empty;
    int mpeg_credit_return_enabled;

    uint32_t max_data_payload;
    uint32_t receiver_thread_stack_size;
    int receiver_thread_priority;

    /*
     * Zero-length channel-1 DATA carries only the audited finite-session RFB
     * request/commit markers. AUDIO's audited zero-length channel-2 DATA owns
     * its finite producer marker. MPEG finite completion is deliberately not
     * inferred from zero-length channel-4 DATA; it is an explicit Transport
     * fact published only after a later owner proves the real ordered producer
     * fence.
     */
    volatile uint32_t rfb_quiesce_request_received;
    volatile uint32_t rfb_quiesce_boundary_sent;
    volatile uint32_t rfb_quiesce_commit_received;
    volatile uint32_t rfb_quiesce_complete_sent;

    /*
     * MPEG run-boundary facts are protected by mpeg_queue_semaphore_id. They
     * do not allocate product generation identity: they only fence whether this
     * session-scoped channel may admit DATA, whether START/RETIRE were submitted
     * for the current run, and whether finalization is still in progress.
     *
     * RETIRE completion itself remains in the dedicated control slot below. A
     * completion closes DATA admission under the queue lock before becoming
     * observable through that slot. Taking the slot never reopens admission.
     */
    int mpeg_run_open;
    int mpeg_start_submitted;
    int mpeg_retire_submitted;
    int mpeg_retirement_latched;
    int mpeg_finalization_in_progress;
    int mpeg_consumer_active;

    /*
     * Transaction-correlation evidence only. Transport does not allocate or
     * interpret generation identity; it preserves the exact submitted RETIRE
     * payload so only that transaction's echoed completion can close DATA
     * admission for this run.
     */
    pstvnc_mpeg_retire_payload_t mpeg_expected_retire_completion;
    int mpeg_expected_retire_completion_valid;

    pstvnc_mpeg_retire_payload_t mpeg_retire_completion;
    int mpeg_retire_completion_pending;

    pstvnc_transport_outbound_work_t outbound_work;
    uint8_t receiver_payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];
} pstvnc_transport_runtime_t;

/*
 * Raw-descriptor constructors remain Transport-internal regression seams.
 * Cross-component bridge code must use the established-lineage constructors
 * below so ordinary rider activation cannot bypass product Q4.
 */
int pstvnc_transport_runtime_initialize(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config);

/* Opt in to logical media channels using explicit caller/profile authority. */
int pstvnc_transport_runtime_initialize_with_audio(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config);
int pstvnc_transport_runtime_initialize_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);
int pstvnc_transport_runtime_initialize_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    int socket_fd,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);

/*
 * Rider-runtime constructors that consume one already-established Q4 physical
 * lineage. Ownership moves only on success and preserves sequence 2/2.
 */
int pstvnc_transport_runtime_initialize_established(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config);
int pstvnc_transport_runtime_initialize_established_with_audio(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config);
int pstvnc_transport_runtime_initialize_established_with_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);
int pstvnc_transport_runtime_initialize_established_with_audio_mpeg(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_physical_stream_t *physical_stream,
    const pstvnc_transport_session_config_t *config,
    const pstvnc_transport_audio_channel_config_t *audio_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_config);

int pstvnc_transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime);
int pstvnc_transport_runtime_request_stop(
    pstvnc_transport_runtime_t *runtime);

/*
 * Internal Transport outbound rendezvous. Logical/domain owners may request a
 * framed send, but the physical send itself is executed only by the Transport
 * I/O thread.
 */
int pstvnc_transport_runtime_submit_frame(
    pstvnc_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length);

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
 * Inspect the first typed RFB-provider terminal cause without changing Wire
 * health. OK returns a specific reason, WOULD_BLOCK means no provider terminal
 * fact is latched, and ordinary Transport terminal results remain distinct.
 */
pstvnc_transport_result_t pstvnc_transport_runtime_rfb_provider_failure(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_rfb_provider_failure_reason_t *reason);

/*
 * Logical AUDIO consumer seam. read_available() is nonblocking and bounded:
 * OK returns bytes, WOULD_BLOCK means live producer/no bytes, EXHAUSTED means
 * the audited one-shot producer marker is visible and the queue is empty,
 * STOPPED means session stop, and FAILED/CLOSED are terminal Transport outcomes.
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

/*
 * Logical MPEG consumer seam. Finite exhaustion is a real producer fact and is
 * separate from Transport stop/cancellation. The future exact producer/control
 * owner may publish producer completion only after proving its ordered wire
 * fence; ordinary MPEG DATA reception never manufactures that fact.
 */
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_read_available(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *read_count);
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_status(
    pstvnc_transport_runtime_t *runtime,
    size_t *available_count,
    int *producer_done);
int pstvnc_transport_runtime_mpeg_activity_snapshot(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);
int pstvnc_transport_runtime_mpeg_wait_activity(
    pstvnc_transport_runtime_t *runtime,
    uint32_t *activity_sequence);
int pstvnc_transport_runtime_mpeg_mark_producer_done(
    pstvnc_transport_runtime_t *runtime);

/*
 * One session-local MPEG run boundary. open() is the only DATA-admission edge.
 * abort_pre_start() is valid only before START submission and only from proven
 * clean state. finalize() is valid only after exact RETIRE completion has been
 * taken and no Transport-visible MPEG activity waiter remains; it discards old
 * residual bytes, returns exact owed credit once, resets run-local state, and
 * leaves the session-scoped queue allocation ready for a later open().
 */
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_open(
    pstvnc_transport_runtime_t *runtime);
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_abort_pre_start(
    pstvnc_transport_runtime_t *runtime);
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_run_finalize(
    pstvnc_transport_runtime_t *runtime);

/*
 * Exact generation-control relay mechanics. These operations move accepted
 * protocol payloads through Transport but do not decide active-generation or
 * producer lifecycle meaning.
 */
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_start(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_start_payload_t *start);
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_send_retire(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_mpeg_retire_payload_t *retire);
pstvnc_transport_result_t pstvnc_transport_runtime_mpeg_take_retire_completion(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_mpeg_retire_payload_t *completion);

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

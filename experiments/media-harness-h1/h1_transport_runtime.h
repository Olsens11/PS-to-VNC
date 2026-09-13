/*
 * File synopsis:
 * Defines H1's one-socket mux runtime for logical AUDIO and MPEG2 channels,
 * plus the experiment-owned logical RFB channel-1 resources and quiesce state.
 *
 * One EE receiver thread is the sole recv() owner. Audio and MPEG bytes land in
 * separate runtime-allocated queues and receive independent credits. RFB uses
 * its own queue/credit resources only in the cumulative preparation build.
 * CONFIG still remains the public activation authority.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_TRANSPORT_RUNTIME_H
#define PSTVNC_MEDIA_HARNESS_H1_TRANSPORT_RUNTIME_H

#include "h1_config.h"
#include "h1_rfb_runtime_resources.h"
#include "transport_protocol.h"
#include "transport_queue.h"

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_H1_SERVER_IP "192.168.50.1"
#define PSTVNC_H1_SERVER_PORT 5902

/* H1 extensions to the v1 PSTV framing vocabulary. */
#define PSTVNC_H1_FRAME_MEDIA_END 8u
#define PSTVNC_H1_FRAME_SESSION_RESULT 9u
#define PSTVNC_H1_FRAME_MPEG_RETIRE 10u

#define PSTVNC_H1_MPEG_RETIRE_VERSION 1u
#define PSTVNC_H1_MPEG_RETIRE_WORDS 3u
#define PSTVNC_H1_MPEG_RETIRE_BYTES (PSTVNC_H1_MPEG_RETIRE_WORDS * 4u)

#define PSTVNC_H1_MEDIA_END_VERSION 1u
#define PSTVNC_H1_MEDIA_END_WORDS 16u
#define PSTVNC_H1_MEDIA_END_BYTES \
    (PSTVNC_H1_MEDIA_END_WORDS * 4u)

#define PSTVNC_H1_TELEMETRY_VERSION 2u
#define PSTVNC_H1_TELEMETRY_WORDS 40u
#define PSTVNC_H1_TELEMETRY_BYTES \
    (PSTVNC_H1_TELEMETRY_WORDS * 4u)

#define PSTVNC_H1_RESULT_VERSION 1u
#define PSTVNC_H1_RESULT_WORDS 32u
#define PSTVNC_H1_RESULT_BYTES \
    (PSTVNC_H1_RESULT_WORDS * 4u)

typedef enum pstvnc_h1_transport_error {
    PSTVNC_H1_ERROR_NONE = 0,
    PSTVNC_H1_ERROR_ARGUMENT = 1,
    PSTVNC_H1_ERROR_SEMAPHORE = 2,
    PSTVNC_H1_ERROR_SOCKET = 3,
    PSTVNC_H1_ERROR_CONNECT = 4,
    PSTVNC_H1_ERROR_SEND = 5,
    PSTVNC_H1_ERROR_RECEIVE = 6,
    PSTVNC_H1_ERROR_HEADER = 7,
    PSTVNC_H1_ERROR_SEQUENCE = 8,
    PSTVNC_H1_ERROR_FRAME_KIND = 9,
    PSTVNC_H1_ERROR_CHANNEL = 10,
    PSTVNC_H1_ERROR_QUEUE_FULL = 11,
    PSTVNC_H1_ERROR_AUDIO_ALIGNMENT = 12,
    PSTVNC_H1_ERROR_THREAD = 13,
    PSTVNC_H1_ERROR_THREAD_DELAY = 14,
    PSTVNC_H1_ERROR_CONFIG = 15,
    PSTVNC_H1_ERROR_ALLOCATION = 16,
    PSTVNC_H1_ERROR_END_METADATA = 17,
    PSTVNC_H1_ERROR_OVERFLOW = 18,
    PSTVNC_H1_ERROR_MPEG_RETIRE = 19
} pstvnc_h1_transport_error_t;

typedef struct pstvnc_h1_transport_stats {
    volatile uint32_t frames_received;
    volatile uint32_t payload_bytes_received;
    volatile uint32_t receiver_loop_count;
    volatile uint32_t last_received_sequence;
    volatile uint32_t last_sent_sequence;

    volatile uint32_t audio_frames_received;
    volatile uint32_t audio_bytes_enqueued;
    volatile uint32_t audio_bytes_consumed;
    volatile uint32_t audio_credit_frames_sent;
    volatile uint32_t audio_credit_bytes_sent;
    volatile uint32_t audio_read_calls;
    volatile uint32_t audio_chunks_played;
    volatile uint32_t audio_bytes_played;

    volatile uint32_t mpeg_frames_received;
    volatile uint32_t mpeg_bytes_enqueued;
    volatile uint32_t mpeg_bytes_consumed;
    volatile uint32_t mpeg_bytes_discarded_generation_boundary;
    volatile uint32_t mpeg_credit_frames_sent;
    volatile uint32_t mpeg_credit_bytes_sent;
    volatile uint32_t mpeg_read_calls;
    volatile uint32_t mpeg_wait_events;
    volatile uint32_t mpeg_wait_loops;
    volatile uint32_t mpeg_wait_max_loops;
} pstvnc_h1_transport_stats_t;

typedef struct pstvnc_h1_transport_runtime {
    int socket_fd;
    int audio_queue_sema_id;
    int mpeg_queue_sema_id;
    int send_sema_id;
    int receiver_thread_id;

    int initialized;
    int config_accepted;
    int receiver_thread_started;

    volatile int stop_requested;
    volatile int receiver_done;
    volatile int end_received;
    volatile pstvnc_h1_transport_error_t error;
    volatile uint32_t diagnostic_word;

    /*
     * Disposable MPEG inner-stall diagnostic witness.
     *
     * This field is independent of diagnostic_word and the producer_* fields
     * already reused by the concurrent RFB diagnostics.
     */
    volatile uint32_t mpeg_diag_stage;

    /*
     * MPEG consumer wait ownership.
     *
     * -1 means no sleeping MPEG consumer. When the MPEG queue becomes empty,
     * the sole MPEG consumer publishes its EE thread id while still holding
     * the MPEG queue semaphore. The sole receiver clears this field and
     * WakeupThread()s that consumer after MPEG DATA is committed.
     */
    volatile int mpeg_wait_thread_id;

    /*
     * Clean RFB shutdown uses four zero-length PSTV DATA markers on logical
     * channel 1: request(Pi)->boundary(PS2)->commit(Pi)->complete(PS2).
     * Non-empty channel-1 DATA remains raw RFB bytes and is never interpreted
     * here as lifecycle control.
     */
    volatile uint32_t rfb_quiesce_request_received;
    volatile uint32_t rfb_quiesce_boundary_sent;
    volatile uint32_t rfb_quiesce_commit_received;
    volatile uint32_t rfb_quiesce_complete_sent;

    /*
     * CP2P MPEG epoch state. DATA belongs to the one active generation until
     * the sole receiver observes that generation's ordered Pi RETIRE ACK.
     */
    volatile uint32_t mpeg_data_generation;
    volatile uint32_t mpeg_retire_pending_generation;
    volatile uint32_t mpeg_retire_ack_generation;

    pstvnc_h1_config_t config;
    uint32_t config_digest;
    uint32_t config_payload_length;

    uint32_t next_send_sequence;
    uint32_t expected_receive_sequence;

    uint32_t audio_credit_pending;
    uint32_t mpeg_credit_pending;

    pstvnc_transport_queue_t audio_queue;
    pstvnc_transport_queue_t mpeg_queue;

    uint8_t *audio_queue_storage;
    uint8_t *mpeg_queue_storage;

    /*
     * Experiment-owned RFB channel-1 resources live with the one-socket H1
     * transport owner. They remain inactive while CONFIG accepts only RFB OFF.
     */
    pstvnc_h1_rfb_runtime_resources_t rfb_resources;

    void *receiver_thread_stack_allocation;
    unsigned char *receiver_thread_stack;

    uint8_t receiver_payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];

    uint32_t audio_crc_state;
    uint32_t mpeg_crc_state;
    uint32_t audio_crc32;
    uint32_t mpeg_crc32;
    uint32_t audio_last_data_sequence;
    uint32_t mpeg_last_data_sequence;

    uint32_t producer_session_id;
    uint32_t producer_audio_bytes;
    uint32_t producer_audio_frames;
    uint32_t producer_audio_last_sequence;
    uint32_t producer_audio_crc32;
    uint32_t producer_mpeg_bytes;
    uint32_t producer_mpeg_frames;
    uint32_t producer_mpeg_last_sequence;
    uint32_t producer_mpeg_crc32;
    uint32_t producer_picture_starts;
    uint32_t producer_sequence_headers;
    uint32_t producer_sequence_ends;
    uint32_t producer_stop_reason;

    pstvnc_h1_transport_stats_t stats;
} pstvnc_h1_transport_runtime_t;

int pstvnc_h1_transport_start(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_shutdown(
    pstvnc_h1_transport_runtime_t *runtime);

const pstvnc_h1_config_t *pstvnc_h1_transport_config(
    const pstvnc_h1_transport_runtime_t *runtime);

size_t pstvnc_h1_transport_audio_queue_size(
    pstvnc_h1_transport_runtime_t *runtime);

size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_audio_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read);

int pstvnc_h1_transport_mpeg_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read);

/* CP2P consumer-local cancellation; does not alter queue/session ownership. */
int pstvnc_h1_transport_mpeg_read_cancellable(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read,
    const volatile int *cancel_requested);

/* Bind channel-4 DATA acceptance to one immutable CP2P generation. */
int pstvnc_h1_transport_mpeg_generation_open(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation);

/* Abort a just-opened generation only while no MPEG bytes are queued/pending. */
int pstvnc_h1_transport_mpeg_generation_abort(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation);

int pstvnc_h1_transport_mpeg_retire_begin(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation);

/* 1=ordered Pi completion observed, 0=still pending, -1=invalid/failed. */
int pstvnc_h1_transport_mpeg_retire_poll(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation);

/*
 * Called only after the generation's decoder worker is stopped. Discards the
 * residual old-generation queue, returns all withheld credit, and closes the
 * exact retirement transaction so a fresh generation may open.
 */
int pstvnc_h1_transport_mpeg_retire_finalize(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation,
    uint32_t *bytes_discarded);

int pstvnc_h1_transport_audio_exhausted(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_mpeg_exhausted(
    pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_integrity_pass(
    pstvnc_h1_transport_runtime_t *runtime);

void pstvnc_h1_transport_record_audio_played(
    pstvnc_h1_transport_runtime_t *runtime,
    size_t byte_count);

void pstvnc_h1_transport_set_diagnostic_word(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t diagnostic_word);

pstvnc_h1_transport_error_t pstvnc_h1_transport_last_error(
    const pstvnc_h1_transport_runtime_t *runtime);

int pstvnc_h1_transport_send_result(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t pictures_decoded,
    uint32_t pictures_displayed,
    uint32_t feed_calls,
    uint32_t payload_bytes_submitted,
    uint32_t dma_bytes_submitted,
    uint32_t deadline_misses,
    uint32_t max_deadline_late_ticks_lo);

#endif

/*
 * File synopsis:
 * Defines configurable Audio Transport EXP2's PS2 dedicated-link runtime.
 *
 * Unlike EXP1, logical RFB/audio queue storage and the receiver-thread stack
 * are allocated only after the Pi's complete CONFIG profile is accepted.
 *
 * There is no compile-time RFB/audio queue backing ceiling. Requested queue
 * capacity is limited by actual runtime allocation success.
 */

#ifndef PSTVNC_EXPERIMENT_PS2_TRANSPORT_RUNTIME_EXP2_H
#define PSTVNC_EXPERIMENT_PS2_TRANSPORT_RUNTIME_EXP2_H

#include <stddef.h>
#include <stdint.h>

#include "transport_config.h"
#include "transport_protocol.h"
#include "transport_queue.h"

#define PSTVNC_TRANSPORT_PS2_SERVER_IP "192.168.50.1"
#define PSTVNC_TRANSPORT_PS2_SERVER_PORT 5902

typedef enum pstvnc_transport_runtime_error {
    PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE = 0,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_SOCKET,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_CONNECT,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_SEND,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_RECEIVE,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_SEQUENCE,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_FRAME_KIND,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_CHANNEL,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_QUEUE_FULL,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_AUDIO_ALIGNMENT,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD_DELAY,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_CONFIG,
    PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION
} pstvnc_transport_runtime_error_t;

typedef struct pstvnc_transport_runtime_stats {
    volatile uint32_t frames_received;
    volatile uint32_t payload_bytes_received;
    volatile uint32_t receiver_loop_count;

    volatile uint32_t rfb_bytes_enqueued;
    volatile uint32_t rfb_bytes_consumed;
    volatile uint32_t rfb_bytes_sent;
    volatile uint32_t rfb_credit_bytes_sent;
    volatile uint32_t rfb_poll_calls;
    volatile uint32_t rfb_read_calls;

    volatile uint32_t audio_bytes_enqueued;
    volatile uint32_t audio_bytes_consumed;
    volatile uint32_t audio_credit_bytes_sent;
    volatile uint32_t audio_read_calls;
    volatile uint32_t audio_chunks_played;
    volatile uint32_t audio_bytes_played;

    volatile uint32_t last_received_sequence;
    volatile uint32_t last_sent_sequence;
} pstvnc_transport_runtime_stats_t;

typedef struct pstvnc_transport_runtime {
    int socket_fd;

    int rfb_queue_sema_id;
    int audio_queue_sema_id;
    int send_sema_id;

    int receiver_thread_id;

    int initialized;
    int config_accepted;
    int receiver_thread_started;

    volatile int stop_requested;
    volatile pstvnc_transport_runtime_error_t error;

    volatile uint32_t diagnostic_word;

    pstvnc_transport_config_t config;
    uint32_t config_digest;
    uint32_t config_payload_length;

    uint32_t next_send_sequence;
    uint32_t expected_receive_sequence;

    uint32_t rfb_credit_pending;
    uint32_t audio_credit_pending;

    pstvnc_transport_queue_t rfb_queue;
    pstvnc_transport_queue_t audio_queue;

    pstvnc_transport_runtime_stats_t stats;

    /*
     * Exact successful malloc() results for this connection.
     */
    uint8_t *rfb_queue_storage;
    uint8_t *audio_queue_storage;

    /*
     * Receiver stack allocation keeps both the original malloc pointer and the
     * aligned stack address passed to CreateThread().
     */
    void *receiver_thread_stack_allocation;
    unsigned char *receiver_thread_stack;

    /*
     * One full protocol payload scratch area remains fixed because 8192 bytes
     * is an actual v1 wire-protocol maximum rather than experimental policy.
     */
    uint8_t receiver_payload[
        PSTVNC_TRANSPORT_MAX_PAYLOAD
    ];
} pstvnc_transport_runtime_t;

int pstvnc_transport_runtime_start(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_shutdown(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_socket_fd(
    const pstvnc_transport_runtime_t *runtime);

const pstvnc_transport_config_t *
pstvnc_transport_runtime_config(
    const pstvnc_transport_runtime_t *runtime);

size_t pstvnc_transport_runtime_audio_queue_size(
    pstvnc_transport_runtime_t *runtime);

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

int pstvnc_transport_runtime_audio_read(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read);

void pstvnc_transport_runtime_set_diagnostic_word(
    pstvnc_transport_runtime_t *runtime,
    uint32_t diagnostic_word);

void pstvnc_transport_runtime_record_audio_played(
    pstvnc_transport_runtime_t *runtime,
    size_t byte_count);

pstvnc_transport_runtime_error_t
pstvnc_transport_runtime_last_error(
    const pstvnc_transport_runtime_t *runtime);

#endif

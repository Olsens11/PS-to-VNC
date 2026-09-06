/*
 * File synopsis:
 * Defines the experiment-only PS2 owner for one dedicated-link transport
 * connection, one receive/demultiplex worker, bounded RFB/audio queues,
 * serialized outbound framing, receiver credits, and transport telemetry.
 *
 * The runtime supplies mechanisms only. It does not parse RFB, play audio,
 * choose Pi scheduling policy, or implement product recovery.
 *
 * Context:
 *   experiments/audio-transport/README.md
 */

#ifndef PSTVNC_EXPERIMENT_PS2_TRANSPORT_RUNTIME_H
#define PSTVNC_EXPERIMENT_PS2_TRANSPORT_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#include "transport_protocol.h"
#include "transport_queue.h"

#define PSTVNC_TRANSPORT_PS2_SERVER_IP "192.168.50.1"
#define PSTVNC_TRANSPORT_PS2_SERVER_PORT 5902

#define PSTVNC_TRANSPORT_PS2_RFB_QUEUE_CAPACITY   32768u
#define PSTVNC_TRANSPORT_PS2_AUDIO_QUEUE_CAPACITY 32768u

#define PSTVNC_TRANSPORT_PS2_AUDIO_RATE        48000u
#define PSTVNC_TRANSPORT_PS2_AUDIO_FRAME_BYTES 4u

#define PSTVNC_TRANSPORT_PS2_RECEIVER_STACK_SIZE 16384u
#define PSTVNC_TRANSPORT_PS2_RECEIVER_PRIORITY   63

#define PSTVNC_TRANSPORT_PS2_CREDIT_BATCH_BYTES 4096u

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
    PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD_DELAY
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
    int receiver_thread_started;

    volatile int stop_requested;
    volatile pstvnc_transport_runtime_error_t error;

    uint32_t next_send_sequence;
    uint32_t expected_receive_sequence;

    uint32_t rfb_credit_pending;
    uint32_t audio_credit_pending;

    pstvnc_transport_queue_t rfb_queue;
    pstvnc_transport_queue_t audio_queue;

    pstvnc_transport_runtime_stats_t stats;

    uint8_t rfb_queue_storage[
        PSTVNC_TRANSPORT_PS2_RFB_QUEUE_CAPACITY
    ];

    uint8_t audio_queue_storage[
        PSTVNC_TRANSPORT_PS2_AUDIO_QUEUE_CAPACITY
    ];

    uint8_t receiver_payload[
        PSTVNC_TRANSPORT_MAX_PAYLOAD
    ];

    unsigned char receiver_thread_stack[
        PSTVNC_TRANSPORT_PS2_RECEIVER_STACK_SIZE
    ] __attribute__((aligned(16)));
} pstvnc_transport_runtime_t;

int pstvnc_transport_runtime_start(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_shutdown(
    pstvnc_transport_runtime_t *runtime);

int pstvnc_transport_runtime_socket_fd(
    const pstvnc_transport_runtime_t *runtime);

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

void pstvnc_transport_runtime_record_audio_played(
    pstvnc_transport_runtime_t *runtime,
    size_t byte_count);

pstvnc_transport_runtime_error_t
pstvnc_transport_runtime_last_error(
    const pstvnc_transport_runtime_t *runtime);

#endif

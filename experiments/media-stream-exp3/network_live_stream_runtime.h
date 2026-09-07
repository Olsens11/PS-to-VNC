/*
 * File synopsis:
 * Defines EXP3 O3's finite live MPEG network-stream mechanism.
 *
 * O3 keeps O2's qualified concurrent byte-ring/credit architecture but no
 * longer knows the MPEG byte count in advance. The Pi sends DATA/channel 4 as
 * ffmpeg produces it, followed by one terminal TELEMETRY frame describing the
 * exact bytes, DATA-frame count, CRC32, and MPEG start-code counts actually
 * produced. Temporary queue starvation is therefore distinct from true EOF.
 */

#ifndef EXP3_NETWORK_LIVE_STREAM_RUNTIME_H
#define EXP3_NETWORK_LIVE_STREAM_RUNTIME_H

#include "transport_protocol.h"
#include "transport_queue.h"

#include <stddef.h>
#include <stdint.h>

#define EXP3_O3_CONFIG_VERSION 1u
#define EXP3_O3_CONFIG_PAYLOAD_BYTES 16u

#define EXP3_O3_END_VERSION 1u
#define EXP3_O3_END_WORDS 8u
#define EXP3_O3_END_PAYLOAD_BYTES (EXP3_O3_END_WORDS * 4u)

#define EXP3_O3_RESULT_VERSION 1u
#define EXP3_O3_RESULT_WORDS 30u
#define EXP3_O3_RESULT_PAYLOAD_BYTES (EXP3_O3_RESULT_WORDS * 4u)

typedef enum Exp3LiveMpegStreamError
{
    EXP3_LIVE_STREAM_ERROR_NONE = 0,
    EXP3_LIVE_STREAM_ERROR_ARGUMENT = 1,
    EXP3_LIVE_STREAM_ERROR_SOCKET = 2,
    EXP3_LIVE_STREAM_ERROR_CONNECT = 3,
    EXP3_LIVE_STREAM_ERROR_CONFIG = 4,
    EXP3_LIVE_STREAM_ERROR_ALLOCATION = 5,
    EXP3_LIVE_STREAM_ERROR_SEMAPHORE = 6,
    EXP3_LIVE_STREAM_ERROR_THREAD = 7,
    EXP3_LIVE_STREAM_ERROR_RECEIVE = 8,
    EXP3_LIVE_STREAM_ERROR_HEADER = 9,
    EXP3_LIVE_STREAM_ERROR_SEQUENCE = 10,
    EXP3_LIVE_STREAM_ERROR_QUEUE_FULL = 11,
    EXP3_LIVE_STREAM_ERROR_CRC = 12,
    EXP3_LIVE_STREAM_ERROR_TRUNCATED = 13,
    EXP3_LIVE_STREAM_ERROR_SEND = 14,
    EXP3_LIVE_STREAM_ERROR_DELAY = 15,
    EXP3_LIVE_STREAM_ERROR_END_METADATA = 16,
    EXP3_LIVE_STREAM_ERROR_OVERFLOW = 17
} Exp3LiveMpegStreamError;

typedef struct Exp3LiveMpegStreamRuntime
{
    int socket_fd;
    int queue_sema_id;
    int receiver_thread_id;
    int receiver_thread_started;

    uint8_t *queue_storage;
    void *receiver_thread_stack_allocation;
    uint8_t *receiver_thread_stack;

    pstvnc_transport_queue_t queue;

    uint8_t receiver_payload[PSTVNC_TRANSPORT_MAX_PAYLOAD];

    uint32_t queue_capacity;
    uint32_t receiver_thread_stack_size;
    uint32_t receiver_thread_priority;

    uint32_t expected_receive_sequence;
    uint32_t next_send_sequence;

    uint32_t frames_received;
    uint32_t bytes_received;
    uint32_t last_data_sequence;

    uint32_t crc_state;
    uint32_t crc32;

    uint32_t producer_bytes;
    uint32_t producer_frames;
    uint32_t producer_last_data_sequence;
    uint32_t producer_crc32;
    uint32_t producer_picture_starts;
    uint32_t producer_sequence_headers;
    uint32_t producer_sequence_ends;

    uint32_t bytes_consumed;

    uint32_t feed_wait_events;
    uint32_t feed_wait_loops;
    uint32_t feed_wait_max_loops;

    uint32_t credit_pending;
    uint32_t credit_frames_sent;
    uint32_t credit_bytes_sent;

    volatile int end_received;
    volatile int receiver_done;
    volatile int error;
} Exp3LiveMpegStreamRuntime;

int exp3_live_mpeg_stream_start(
    Exp3LiveMpegStreamRuntime *runtime);

int exp3_live_mpeg_stream_read(
    Exp3LiveMpegStreamRuntime *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *actual_count);

int exp3_live_mpeg_stream_is_exhausted(
    Exp3LiveMpegStreamRuntime *runtime);

int exp3_live_mpeg_stream_integrity_pass(
    Exp3LiveMpegStreamRuntime *runtime);

uint32_t exp3_live_mpeg_stream_remaining_bytes(
    Exp3LiveMpegStreamRuntime *runtime);

int exp3_live_mpeg_stream_send_result(
    Exp3LiveMpegStreamRuntime *runtime,
    uint32_t pictures_decoded,
    uint32_t pictures_displayed,
    uint32_t feed_calls,
    uint32_t payload_bytes_submitted,
    uint32_t dma_bytes_submitted);

#endif

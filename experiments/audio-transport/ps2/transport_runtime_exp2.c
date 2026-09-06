/*
 * File synopsis:
 * Implements the experiment-only PS2 dedicated-link transport runtime.
 *
 * One EE worker is the sole recv() owner for one TCP connection. It validates
 * frames and demultiplexes Pi->PS2 RFB and PCM payloads into bounded queues.
 *
 * Outbound frames share one serialized send path. The Pi owns scheduling,
 * pacing, batching, weighting, and policy ceilings.
 *
 * Context:
 *   experiments/audio-transport/README.md
 */

#include "transport_runtime_exp2.h"

#include <delaythread.h>
#include <kernel.h>
#include <ps2ip.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TRANSPORT_RECEIVER_STOP_WAIT_STEP_US 1000u
#define TRANSPORT_RECEIVER_STOP_WAIT_STEPS   3000u

static void transport_runtime_record_error(
    pstvnc_transport_runtime_t *runtime,
    pstvnc_transport_runtime_error_t error)
{
    if (runtime != NULL &&
        runtime->error == PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE)
        runtime->error = error;
}

static int transport_runtime_create_mutex(void)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    return CreateSema(&semaphore);
}

static int transport_socket_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    size_t done = 0;

    while (done < count) {
        int received;

        if (runtime->stop_requested)
            return 0;

        received = recv(
            runtime->socket_fd,
            destination + done,
            count - done,
            0);

        if (received <= 0)
            return 0;

        done += (size_t)received;
    }

    return 1;
}

static int transport_socket_send_exact_locked(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    const uint8_t *source = (const uint8_t *)buffer;
    size_t done = 0;

    while (done < count) {
        int sent = send(
            runtime->socket_fd,
            source + done,
            count - done,
            0);

        if (sent <= 0)
            return 0;

        done += (size_t)sent;
    }

    return 1;
}

static int transport_runtime_send_frame(
    pstvnc_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    uint8_t wire_header[
        PSTVNC_TRANSPORT_HEADER_SIZE
    ];

    pstvnc_transport_header_t header;
    int ok = 0;

    if (runtime == NULL ||
        runtime->socket_fd < 0 ||
        runtime->send_sema_id < 0 ||
        payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        (payload == NULL && payload_length != 0)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT);

        return 0;
    }

    if (WaitSema(runtime->send_sema_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    memset(&header, 0, sizeof(header));

    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = kind;
    header.channel = channel;
    header.flags = flags;
    header.sequence = runtime->next_send_sequence;
    header.payload_length = (uint32_t)payload_length;

    if (!pstvnc_transport_header_encode(
            wire_header,
            &header)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
        goto done;
    }

    if (!transport_socket_send_exact_locked(
            runtime,
            wire_header,
            sizeof(wire_header))) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEND);
        goto done;
    }

    if (payload_length != 0 &&
        !transport_socket_send_exact_locked(
            runtime,
            payload,
            payload_length)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEND);
        goto done;
    }

    runtime->stats.last_sent_sequence =
        runtime->next_send_sequence;

    runtime->next_send_sequence++;
    ok = 1;

done:
    if (SignalSema(runtime->send_sema_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    return ok;
}


static void *transport_runtime_allocate_aligned16(
    size_t byte_count,
    void **allocation)
{
    uintptr_t address;
    void *raw;

    if (allocation == NULL ||
        byte_count == 0 ||
        byte_count > ((size_t)-1 - 15u))
        return NULL;

    raw = malloc(byte_count + 15u);

    if (raw == NULL)
        return NULL;

    address =
        ((uintptr_t)raw + 15u) &
        ~(uintptr_t)15u;

    *allocation = raw;

    return (void *)address;
}

static uint32_t transport_runtime_audio_frame_bytes(
    const pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL)
        return 0u;

    return pstvnc_transport_config_audio_frame_bytes(
        &runtime->config);
}

static int transport_runtime_send_error_code(
    pstvnc_transport_runtime_t *runtime)
{
    uint8_t payload[4];

    if (runtime == NULL)
        return 0;

    pstvnc_transport_write_be32(
        payload,
        (uint32_t)runtime->error);

    return transport_runtime_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_ERROR,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0,
        payload,
        sizeof(payload));
}

static int transport_runtime_receive_config(
    pstvnc_transport_runtime_t *runtime)
{
    uint8_t wire_header[
        PSTVNC_TRANSPORT_HEADER_SIZE
    ];

    pstvnc_transport_header_t header;

    if (!transport_socket_read_exact(
            runtime,
            wire_header,
            sizeof(wire_header))) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_RECEIVE);
        return 0;
    }

    if (!pstvnc_transport_header_decode(
            &header,
            wire_header)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
        return 0;
    }

    if (header.sequence !=
        runtime->expected_receive_sequence) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEQUENCE);
        return 0;
    }

    if (header.kind !=
            PSTVNC_TRANSPORT_FRAME_CONFIG ||
        header.channel !=
            PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.payload_length == 0u ||
        header.payload_length >
            PSTVNC_TRANSPORT_MAX_PAYLOAD) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_CONFIG);
        return 0;
    }

    if (!transport_socket_read_exact(
            runtime,
            runtime->receiver_payload,
            header.payload_length)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_RECEIVE);
        return 0;
    }

    runtime->stats.frames_received++;
    runtime->stats.payload_bytes_received +=
        header.payload_length;

    runtime->stats.last_received_sequence =
        header.sequence;

    runtime->expected_receive_sequence++;

    if (!pstvnc_transport_config_decode(
            &runtime->config,
            runtime->receiver_payload,
            header.payload_length) ||
        !pstvnc_transport_config_validate(
            &runtime->config)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_CONFIG);
        return 0;
    }

    runtime->config_payload_length =
        header.payload_length;

    runtime->config_digest =
        pstvnc_transport_config_digest(
            runtime->receiver_payload,
            header.payload_length);

    runtime->config_accepted = 1;

    return 1;
}

static int transport_runtime_allocate_queue_storage(
    pstvnc_transport_runtime_t *runtime)
{
    size_t rfb_bytes;
    size_t audio_bytes;

    if (runtime == NULL ||
        !runtime->config_accepted)
        return 0;

    rfb_bytes =
        (size_t)runtime->config.rfb_queue_capacity;

    audio_bytes =
        (size_t)runtime->config.audio_queue_capacity;

    /*
     * Detect any platform size_t narrowing instead of silently allocating a
     * different queue from the one the Pi requested.
     */
    if ((uint32_t)rfb_bytes !=
            runtime->config.rfb_queue_capacity ||
        (uint32_t)audio_bytes !=
            runtime->config.audio_queue_capacity) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION);
        return 0;
    }

    if (runtime->config.queue_allocation_order ==
        PSTVNC_TRANSPORT_ALLOCATE_AUDIO_FIRST) {

        runtime->audio_queue_storage =
            (uint8_t *)malloc(audio_bytes);

        if (runtime->audio_queue_storage == NULL)
            goto fail;

        runtime->rfb_queue_storage =
            (uint8_t *)malloc(rfb_bytes);

        if (runtime->rfb_queue_storage == NULL)
            goto fail;

    } else {
        runtime->rfb_queue_storage =
            (uint8_t *)malloc(rfb_bytes);

        if (runtime->rfb_queue_storage == NULL)
            goto fail;

        runtime->audio_queue_storage =
            (uint8_t *)malloc(audio_bytes);

        if (runtime->audio_queue_storage == NULL)
            goto fail;
    }

    if (!pstvnc_transport_queue_init(
            &runtime->rfb_queue,
            runtime->rfb_queue_storage,
            rfb_bytes) ||
        !pstvnc_transport_queue_init(
            &runtime->audio_queue,
            runtime->audio_queue_storage,
            audio_bytes)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION);
        goto fail;
    }

    return 1;

fail:
    transport_runtime_record_error(
        runtime,
        PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION);

    free(runtime->rfb_queue_storage);
    free(runtime->audio_queue_storage);

    runtime->rfb_queue_storage = NULL;
    runtime->audio_queue_storage = NULL;

    return 0;
}

static int transport_runtime_allocate_receiver_stack(
    pstvnc_transport_runtime_t *runtime)
{
    size_t stack_bytes;

    if (runtime == NULL ||
        !runtime->config_accepted)
        return 0;

    stack_bytes =
        (size_t)runtime->config.receiver_thread_stack_size;

    if ((uint32_t)stack_bytes !=
        runtime->config.receiver_thread_stack_size) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION);
        return 0;
    }

    runtime->receiver_thread_stack =
        (unsigned char *)
        transport_runtime_allocate_aligned16(
            stack_bytes,
            &runtime->receiver_thread_stack_allocation);

    if (runtime->receiver_thread_stack == NULL) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ALLOCATION);
        return 0;
    }

    return 1;
}

static int transport_runtime_apply_socket_options(
    pstvnc_transport_runtime_t *runtime)
{
    int value;

    if (runtime == NULL ||
        runtime->socket_fd < 0)
        return 0;

    if (runtime->config.socket_receive_buffer_bytes != 0u) {
        value =
            (int)runtime->config.socket_receive_buffer_bytes;

        if (setsockopt(
                runtime->socket_fd,
                SOL_SOCKET,
                SO_RCVBUF,
                &value,
                sizeof(value)) < 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_SOCKET);
            return 0;
        }
    }

    if (runtime->config.socket_send_buffer_bytes != 0u) {
        value =
            (int)runtime->config.socket_send_buffer_bytes;

        if (setsockopt(
                runtime->socket_fd,
                SOL_SOCKET,
                SO_SNDBUF,
                &value,
                sizeof(value)) < 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_SOCKET);
            return 0;
        }
    }

    return 1;
}

static int transport_runtime_send_config_ack(
    pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->config_accepted ||
        runtime->config_payload_length == 0u)
        return 0;

    /*
     * Echoing the exact original wire payload proves that PS2 and Pi agree on
     * every field, not merely on a profile number.
     */
    return transport_runtime_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_CONFIG,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        PSTVNC_TRANSPORT_CONFIG_ACK_FLAG,
        runtime->receiver_payload,
        runtime->config_payload_length);
}

static int transport_runtime_send_credit(
    pstvnc_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t credit_bytes)
{
    uint8_t payload[
        PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE
    ];

    if (credit_bytes == 0)
        return 1;

    pstvnc_transport_write_be32(
        payload,
        credit_bytes);

    if (!transport_runtime_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            channel,
            0,
            payload,
            sizeof(payload)))
        return 0;

    if (channel == PSTVNC_TRANSPORT_CHANNEL_RFB) {
        runtime->stats.rfb_credit_bytes_sent +=
            credit_bytes;

    } else if (
        channel ==
        PSTVNC_TRANSPORT_CHANNEL_AUDIO) {

        runtime->stats.audio_credit_bytes_sent +=
            credit_bytes;
    }

    return 1;
}

static int transport_runtime_send_hello(
    pstvnc_transport_runtime_t *runtime)
{
    uint8_t payload[
        PSTVNC_TRANSPORT_HELLO_PAYLOAD_SIZE
    ];

    uint32_t capabilities =
        PSTVNC_TRANSPORT_CAP_RFB |
        PSTVNC_TRANSPORT_CAP_AUDIO_PCM_S16 |
        PSTVNC_TRANSPORT_CAP_RECEIVER_CREDIT |
        PSTVNC_TRANSPORT_CAP_TELEMETRY_SNAPSHOT |
        PSTVNC_TRANSPORT_CAP_CONFIG_DYNAMIC;

    memset(payload, 0, sizeof(payload));

    pstvnc_transport_write_be32(
        &payload[0],
        capabilities);

    pstvnc_transport_write_be32(
        &payload[4],
        PSTVNC_TRANSPORT_MAX_PAYLOAD);

    /*
     * Zero capacity/format words deliberately mean "selected by CONFIG" when
     * PSTVNC_TRANSPORT_CAP_CONFIG_DYNAMIC is present.
     */
    pstvnc_transport_write_be32(
        &payload[8],
        0u);

    pstvnc_transport_write_be32(
        &payload[12],
        0u);

    pstvnc_transport_write_be32(
        &payload[16],
        0u);

    pstvnc_transport_write_be32(
        &payload[20],
        0u);

    return transport_runtime_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_HELLO,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0,
        payload,
        sizeof(payload));
}

static int transport_runtime_snapshot_queue(
    pstvnc_transport_runtime_t *runtime,
    int semaphore_id,
    const pstvnc_transport_queue_t *queue,
    uint32_t *current,
    uint32_t *high_water)
{
    if (WaitSema(semaphore_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    *current =
        (uint32_t)pstvnc_transport_queue_size(
            queue);

    *high_water =
        (uint32_t)pstvnc_transport_queue_high_water(
            queue);

    if (SignalSema(semaphore_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    return 1;
}

static int transport_runtime_send_telemetry(
    pstvnc_transport_runtime_t *runtime)
{
    uint8_t payload[
        PSTVNC_TRANSPORT_TELEMETRY_PAYLOAD_SIZE
    ];

    uint32_t rfb_current;
    uint32_t rfb_high_water;
    uint32_t audio_current;
    uint32_t audio_high_water;

    if (!transport_runtime_snapshot_queue(
            runtime,
            runtime->rfb_queue_sema_id,
            &runtime->rfb_queue,
            &rfb_current,
            &rfb_high_water))
        return 0;

    if (!transport_runtime_snapshot_queue(
            runtime,
            runtime->audio_queue_sema_id,
            &runtime->audio_queue,
            &audio_current,
            &audio_high_water))
        return 0;

    memset(payload, 0, sizeof(payload));

#define PUT32(offset, value) \
    pstvnc_transport_write_be32( \
        &payload[(offset)], \
        (uint32_t)(value))

    PUT32(0, PSTVNC_TRANSPORT_TELEMETRY_VERSION);
    PUT32(4, runtime->error);

    PUT32(8, runtime->stats.frames_received);
    PUT32(12, runtime->stats.payload_bytes_received);
    PUT32(16, runtime->stats.receiver_loop_count);
    PUT32(20, runtime->stats.last_received_sequence);
    PUT32(24, runtime->stats.last_sent_sequence);

    PUT32(28, runtime->stats.rfb_bytes_enqueued);
    PUT32(32, runtime->stats.rfb_bytes_consumed);
    PUT32(36, runtime->stats.rfb_bytes_sent);
    PUT32(40, rfb_current);
    PUT32(44, rfb_high_water);
    PUT32(48, runtime->stats.rfb_credit_bytes_sent);
    PUT32(52, runtime->stats.rfb_poll_calls);
    PUT32(56, runtime->stats.rfb_read_calls);

    PUT32(60, runtime->stats.audio_bytes_enqueued);
    PUT32(64, runtime->stats.audio_bytes_consumed);
    PUT32(68, audio_current);
    PUT32(72, audio_high_water);
    PUT32(76, runtime->stats.audio_credit_bytes_sent);
    PUT32(80, runtime->stats.audio_read_calls);
    PUT32(84, runtime->stats.audio_chunks_played);
    PUT32(88, runtime->stats.audio_bytes_played);

    PUT32(92, runtime->diagnostic_word);

#undef PUT32

    return transport_runtime_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_TELEMETRY,
        PSTVNC_TRANSPORT_CHANNEL_TELEMETRY,
        0,
        payload,
        sizeof(payload));
}

static int transport_runtime_enqueue_data(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header,
    const uint8_t *payload)
{
    pstvnc_transport_queue_t *queue;
    int semaphore_id;
    int written;

    if (header->payload_length == 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
        return 0;
    }

    if (header->payload_length >
        runtime->config.max_data_payload) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
        return 0;
    }

    if (header->channel ==
        PSTVNC_TRANSPORT_CHANNEL_RFB) {

        queue = &runtime->rfb_queue;
        semaphore_id =
            runtime->rfb_queue_sema_id;

    } else if (
        header->channel ==
        PSTVNC_TRANSPORT_CHANNEL_AUDIO) {

        if ((header->payload_length %
             transport_runtime_audio_frame_bytes(
                 runtime)) != 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_AUDIO_ALIGNMENT);
            return 0;
        }

        queue = &runtime->audio_queue;
        semaphore_id =
            runtime->audio_queue_sema_id;

    } else {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_CHANNEL);
        return 0;
    }

    if (WaitSema(semaphore_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    written =
        pstvnc_transport_queue_write(
            queue,
            payload,
            header->payload_length);

    if (SignalSema(semaphore_id) < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    if (!written) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_QUEUE_FULL);
        return 0;
    }

    if (header->channel ==
        PSTVNC_TRANSPORT_CHANNEL_RFB) {

        runtime->stats.rfb_bytes_enqueued +=
            header->payload_length;

    } else {
        runtime->stats.audio_bytes_enqueued +=
            header->payload_length;
    }

    return 1;
}

static int transport_runtime_accept_received_frame(
    pstvnc_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    switch (header->kind) {
        case PSTVNC_TRANSPORT_FRAME_DATA:
            return transport_runtime_enqueue_data(
                runtime,
                header,
                runtime->receiver_payload);

        case PSTVNC_TRANSPORT_FRAME_HELLO:
            if (header->channel !=
                    PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
                header->payload_length != 0) {

                transport_runtime_record_error(
                    runtime,
                    PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
                return 0;
            }

            return 1;

        case PSTVNC_TRANSPORT_FRAME_HEARTBEAT:
            if (header->channel !=
                    PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
                header->payload_length != 0) {

                transport_runtime_record_error(
                    runtime,
                    PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
                return 0;
            }

            return transport_runtime_send_telemetry(
                runtime);

        default:
            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_FRAME_KIND);
            return 0;
    }
}

static void transport_receiver_thread(
    void *argument)
{
    pstvnc_transport_runtime_t *runtime =
        (pstvnc_transport_runtime_t *)argument;

    for (;;) {
        uint8_t wire_header[
            PSTVNC_TRANSPORT_HEADER_SIZE
        ];

        pstvnc_transport_header_t header;

        if (runtime->stop_requested)
            break;

        runtime->stats.receiver_loop_count++;

        if (!transport_socket_read_exact(
                runtime,
                wire_header,
                sizeof(wire_header))) {

            if (!runtime->stop_requested) {
                transport_runtime_record_error(
                    runtime,
                    PSTVNC_TRANSPORT_RUNTIME_ERROR_RECEIVE);
            }

            break;
        }

        if (!pstvnc_transport_header_decode(
                &header,
                wire_header)) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_HEADER);
            break;
        }

        if (header.sequence !=
            runtime->expected_receive_sequence) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_SEQUENCE);
            break;
        }

        if (header.payload_length != 0) {
            if (!transport_socket_read_exact(
                    runtime,
                    runtime->receiver_payload,
                    header.payload_length)) {

                if (!runtime->stop_requested) {
                    transport_runtime_record_error(
                        runtime,
                        PSTVNC_TRANSPORT_RUNTIME_ERROR_RECEIVE);
                }

                break;
            }
        }

        runtime->stats.frames_received++;
        runtime->stats.payload_bytes_received +=
            header.payload_length;

        runtime->stats.last_received_sequence =
            header.sequence;

        runtime->expected_receive_sequence++;

        if (!transport_runtime_accept_received_frame(
                runtime,
                &header))
            break;
    }

    ExitThread();
}

static int transport_runtime_start_receiver(
    pstvnc_transport_runtime_t *runtime)
{
    ee_thread_t thread;

    memset(&thread, 0, sizeof(thread));

    thread.func =
        (void *)transport_receiver_thread;

    thread.stack =
        (void *)runtime->receiver_thread_stack;

    thread.stack_size =
        (int)runtime->config.receiver_thread_stack_size;

    thread.gp_reg = &_gp;
    thread.initial_priority =
        (int)runtime->config.receiver_thread_priority;
    thread.attr = 0;
    thread.option = 0;

    runtime->receiver_thread_id =
        CreateThread(&thread);

    if (runtime->receiver_thread_id < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD);
        return 0;
    }

    if (StartThread(
            runtime->receiver_thread_id,
            runtime) < 0) {

        (void)DeleteThread(
            runtime->receiver_thread_id);

        runtime->receiver_thread_id = -1;

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD);
        return 0;
    }

    runtime->receiver_thread_started = 1;
    return 1;
}

static int transport_runtime_return_consumed_credit(
    pstvnc_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t consumed_bytes,
    int queue_now_empty)
{
    uint32_t *pending;
    uint32_t batch;
    uint32_t flush_on_empty;
    uint32_t credit_to_send = 0;

    if (channel ==
        PSTVNC_TRANSPORT_CHANNEL_RFB) {

        pending =
            &runtime->rfb_credit_pending;

        batch =
            runtime->config.rfb_credit_batch_bytes;

        flush_on_empty =
            runtime->config.rfb_credit_flush_on_empty;

    } else {
        pending =
            &runtime->audio_credit_pending;

        batch =
            runtime->config.audio_credit_batch_bytes;

        flush_on_empty =
            runtime->config.audio_credit_flush_on_empty;
    }

    *pending += consumed_bytes;

    if (*pending >= batch ||
        (flush_on_empty != 0u &&
         queue_now_empty &&
         *pending != 0u)) {

        credit_to_send = *pending;
        *pending = 0;
    }

    if (credit_to_send == 0)
        return 1;

    return transport_runtime_send_credit(
        runtime,
        channel,
        credit_to_send);
}

int pstvnc_transport_runtime_start(
    pstvnc_transport_runtime_t *runtime)
{
    struct sockaddr_in server;

    if (runtime == NULL)
        return -1;

    memset(runtime, 0, sizeof(*runtime));

    runtime->socket_fd = -1;
    runtime->rfb_queue_sema_id = -1;
    runtime->audio_queue_sema_id = -1;
    runtime->send_sema_id = -1;
    runtime->receiver_thread_id = -1;

    runtime->next_send_sequence = 1u;
    runtime->expected_receive_sequence = 1u;

    runtime->error =
        PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE;

    runtime->rfb_queue_sema_id =
        transport_runtime_create_mutex();

    runtime->audio_queue_sema_id =
        transport_runtime_create_mutex();

    runtime->send_sema_id =
        transport_runtime_create_mutex();

    if (runtime->rfb_queue_sema_id < 0 ||
        runtime->audio_queue_sema_id < 0 ||
        runtime->send_sema_id < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        goto fail;
    }

    runtime->socket_fd =
        socket(AF_INET, SOCK_STREAM, 0);

    if (runtime->socket_fd < 0) {
        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SOCKET);
        goto fail;
    }

    memset(&server, 0, sizeof(server));

    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;

    server.sin_port =
        htons(PSTVNC_TRANSPORT_PS2_SERVER_PORT);

    server.sin_addr.s_addr =
        inet_addr(
            PSTVNC_TRANSPORT_PS2_SERVER_IP);

    if (connect(
            runtime->socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_CONNECT);
        goto fail;
    }

    runtime->initialized = 1;

    if (!transport_runtime_send_hello(runtime))
        goto fail;

    /*
     * No DATA credits exist yet. Receive the complete Pi profile synchronously
     * before starting the normal demultiplex receiver.
     */
    if (!transport_runtime_receive_config(runtime)) {
        (void)transport_runtime_send_error_code(runtime);
        goto fail;
    }

    /*
     * Queue allocation itself is the capacity authority. There is intentionally
     * no smaller compile-time experiment ceiling.
     */
    if (!transport_runtime_allocate_queue_storage(runtime)) {
        (void)transport_runtime_send_error_code(runtime);
        goto fail;
    }

    if (!transport_runtime_allocate_receiver_stack(runtime)) {
        (void)transport_runtime_send_error_code(runtime);
        goto fail;
    }

    if (!transport_runtime_apply_socket_options(runtime)) {
        (void)transport_runtime_send_error_code(runtime);
        goto fail;
    }

    if (!transport_runtime_send_config_ack(runtime))
        goto fail;

    if (!transport_runtime_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            runtime->config.rfb_queue_capacity))
        goto fail;

    if (!transport_runtime_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            runtime->config.audio_queue_capacity))
        goto fail;

    if (!transport_runtime_start_receiver(runtime))
        goto fail;

    return 0;

fail:
    (void)pstvnc_transport_runtime_shutdown(
        runtime);

    return -1;
}

int pstvnc_transport_runtime_shutdown(
    pstvnc_transport_runtime_t *runtime)
{
    unsigned int wait_step;
    int result = 0;

    if (runtime == NULL)
        return -1;

    runtime->stop_requested = 1;

    if (runtime->socket_fd >= 0) {
        (void)shutdown(
            runtime->socket_fd,
            SHUT_RDWR);
    }

    if (runtime->receiver_thread_started) {
        for (wait_step = 0;
             wait_step <
                TRANSPORT_RECEIVER_STOP_WAIT_STEPS;
             wait_step++) {

            ee_thread_status_t status;

            memset(&status, 0, sizeof(status));

            if (ReferThreadStatus(
                    runtime->receiver_thread_id,
                    &status) < 0) {
                result = -1;
                break;
            }

            if (status.status == THS_DORMANT) {
                if (DeleteThread(
                        runtime->receiver_thread_id) < 0) {
                    result = -1;
                    break;
                }

                runtime->receiver_thread_id = -1;
                runtime->receiver_thread_started = 0;
                break;
            }

            if (DelayThread(
                    TRANSPORT_RECEIVER_STOP_WAIT_STEP_US) < 0) {
                result = -1;
                break;
            }
        }

        if (runtime->receiver_thread_started)
            return -1;
    }

    if (runtime->socket_fd >= 0) {
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
    }

    if (runtime->rfb_queue_sema_id >= 0) {
        if (DeleteSema(
                runtime->rfb_queue_sema_id) < 0)
            result = -1;

        runtime->rfb_queue_sema_id = -1;
    }

    if (runtime->audio_queue_sema_id >= 0) {
        if (DeleteSema(
                runtime->audio_queue_sema_id) < 0)
            result = -1;

        runtime->audio_queue_sema_id = -1;
    }

    if (runtime->send_sema_id >= 0) {
        if (DeleteSema(
                runtime->send_sema_id) < 0)
            result = -1;

        runtime->send_sema_id = -1;
    }

    free(runtime->receiver_thread_stack_allocation);
    runtime->receiver_thread_stack_allocation = NULL;
    runtime->receiver_thread_stack = NULL;

    free(runtime->rfb_queue_storage);
    runtime->rfb_queue_storage = NULL;

    free(runtime->audio_queue_storage);
    runtime->audio_queue_storage = NULL;

    runtime->config_accepted = 0;
    runtime->initialized = 0;

    return result;
}

int pstvnc_transport_runtime_socket_fd(
    const pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized)
        return -1;

    return runtime->socket_fd;
}

int pstvnc_transport_runtime_rfb_read_exact(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    uint8_t *destination =
        (uint8_t *)buffer;

    size_t done = 0;

    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->rfb_queue_sema_id < 0 ||
        (buffer == NULL && count != 0)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT);
        return -1;
    }

    runtime->stats.rfb_read_calls++;

    while (done < count) {
        size_t available;
        size_t take;
        int queue_empty;

        if (runtime->error !=
            PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE)
            return -1;

        if (WaitSema(
                runtime->rfb_queue_sema_id) < 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
            return -1;
        }

        available =
            pstvnc_transport_queue_size(
                &runtime->rfb_queue);

        take = count - done;

        if (take > available)
            take = available;

        if (take != 0 &&
            !pstvnc_transport_queue_read(
                &runtime->rfb_queue,
                destination + done,
                take)) {

            (void)SignalSema(
                runtime->rfb_queue_sema_id);

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_QUEUE_FULL);
            return -1;
        }

        queue_empty =
            pstvnc_transport_queue_size(
                &runtime->rfb_queue) == 0;

        if (SignalSema(
                runtime->rfb_queue_sema_id) < 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
            return -1;
        }

        if (take != 0) {
            runtime->stats.rfb_bytes_consumed +=
                (uint32_t)take;

            if (!transport_runtime_return_consumed_credit(
                    runtime,
                    PSTVNC_TRANSPORT_CHANNEL_RFB,
                    (uint32_t)take,
                    queue_empty))
                return -1;

            done += take;
            continue;
        }

        if (DelayThread(
                runtime->config.rfb_empty_delay_us) < 0) {

            transport_runtime_record_error(
                runtime,
                PSTVNC_TRANSPORT_RUNTIME_ERROR_THREAD_DELAY);
            return -1;
        }
    }

    return 0;
}

int pstvnc_transport_runtime_rfb_poll_receive(
    pstvnc_transport_runtime_t *runtime)
{
    size_t available;

    if (runtime == NULL ||
        !runtime->initialized ||
        runtime->rfb_queue_sema_id < 0)
        return -1;

    runtime->stats.rfb_poll_calls++;

    if (runtime->error !=
        PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE)
        return -1;

    if (WaitSema(
            runtime->rfb_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return -1;
    }

    available =
        pstvnc_transport_queue_size(
            &runtime->rfb_queue);

    if (SignalSema(
            runtime->rfb_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return -1;
    }

    return available != 0 ? 1 : 0;
}

int pstvnc_transport_runtime_rfb_write_exact(
    pstvnc_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        count > runtime->config.max_data_payload ||
        (buffer == NULL && count != 0)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT);
        return -1;
    }

    if (runtime->error !=
        PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE)
        return -1;

    if (count == 0)
        return 0;

    if (!transport_runtime_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_DATA,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            0,
            buffer,
            count))
        return -1;

    runtime->stats.rfb_bytes_sent +=
        (uint32_t)count;

    return 0;
}

int pstvnc_transport_runtime_audio_read(
    pstvnc_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read)
{
    size_t available;
    size_t take;
    int queue_empty;

    if (runtime == NULL ||
        buffer == NULL ||
        bytes_read == NULL ||
        !runtime->initialized ||
        runtime->audio_queue_sema_id < 0 ||
        buffer_capacity <
            transport_runtime_audio_frame_bytes(
                runtime)) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT);
        return -1;
    }

    *bytes_read = 0;
    runtime->stats.audio_read_calls++;

    if (runtime->error !=
        PSTVNC_TRANSPORT_RUNTIME_ERROR_NONE)
        return -1;

    if (WaitSema(
            runtime->audio_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return -1;
    }

    available =
        pstvnc_transport_queue_size(
            &runtime->audio_queue);

    take = available;

    if (take > buffer_capacity)
        take = buffer_capacity;

    take -=
        take %
        transport_runtime_audio_frame_bytes(
            runtime);

    if (take != 0 &&
        !pstvnc_transport_queue_read(
            &runtime->audio_queue,
            buffer,
            take)) {

        (void)SignalSema(
            runtime->audio_queue_sema_id);

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_QUEUE_FULL);
        return -1;
    }

    queue_empty =
        pstvnc_transport_queue_size(
            &runtime->audio_queue) == 0;

    if (SignalSema(
            runtime->audio_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return -1;
    }

    if (take == 0)
        return 0;

    runtime->stats.audio_bytes_consumed +=
        (uint32_t)take;

    if (!transport_runtime_return_consumed_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            (uint32_t)take,
            queue_empty))
        return -1;

    *bytes_read = take;
    return 1;
}

void pstvnc_transport_runtime_set_diagnostic_word(
    pstvnc_transport_runtime_t *runtime,
    uint32_t diagnostic_word)
{
    if (runtime == NULL)
        return;

    runtime->diagnostic_word = diagnostic_word;
}

void pstvnc_transport_runtime_record_audio_played(
    pstvnc_transport_runtime_t *runtime,
    size_t byte_count)
{
    if (runtime == NULL)
        return;

    runtime->stats.audio_chunks_played++;
    runtime->stats.audio_bytes_played +=
        (uint32_t)byte_count;
}


const pstvnc_transport_config_t *
pstvnc_transport_runtime_config(
    const pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->config_accepted)
        return NULL;

    return &runtime->config;
}

size_t pstvnc_transport_runtime_audio_queue_size(
    pstvnc_transport_runtime_t *runtime)
{
    size_t current;

    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->config_accepted ||
        runtime->audio_queue_sema_id < 0)
        return 0;

    if (WaitSema(
            runtime->audio_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    current =
        pstvnc_transport_queue_size(
            &runtime->audio_queue);

    if (SignalSema(
            runtime->audio_queue_sema_id) < 0) {

        transport_runtime_record_error(
            runtime,
            PSTVNC_TRANSPORT_RUNTIME_ERROR_SEMAPHORE);
        return 0;
    }

    return current;
}

pstvnc_transport_runtime_error_t
pstvnc_transport_runtime_last_error(
    const pstvnc_transport_runtime_t *runtime)
{
    if (runtime == NULL)
        return PSTVNC_TRANSPORT_RUNTIME_ERROR_ARGUMENT;

    return runtime->error;
}

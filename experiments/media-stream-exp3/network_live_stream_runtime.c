/*
 * File synopsis:
 * Implements EXP3 O3's finite live MPEG network byte stream.
 *
 * This is intentionally descended from hardware-qualified O2. One EE thread
 * remains the sole recv() owner; the existing EXP2 byte ring remains the
 * producer/consumer boundary; and the decoder thread returns CREDIT only as
 * bytes leave real queue storage.
 *
 * The only protocol change is EOF ownership. O2 knew an exact fixture size in
 * advance. O3 accepts live DATA/channel 4 until the Pi sends one terminal
 * TELEMETRY/channel 3 frame containing the exact byte count, DATA-frame count,
 * CRC32, and MPEG start-code counts that ffmpeg actually produced. An empty
 * queue before that terminal frame is starvation, not EOF.
 */

#include "network_live_stream_runtime.h"

#include <delaythread.h>
#include <kernel.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXP3_O3_SERVER_IP "192.168.50.1"
#define EXP3_O3_SERVER_PORT 5904

#define EXP3_O3_CONFIG_ACK_FLAG 0x01u
#define EXP3_O3_CREDIT_BATCH_BYTES PSTVNC_TRANSPORT_MAX_PAYLOAD
#define EXP3_O3_EMPTY_WAIT_US 1000u

static int exp3_live_create_mutex(void)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    return CreateSema(&semaphore);
}

static void *exp3_live_allocate_aligned16(
    size_t byte_count,
    void **allocation)
{
    uintptr_t address;
    void *raw;

    if (allocation == NULL || byte_count == 0 || byte_count > ((size_t)-1 - 15u))
        return NULL;

    raw = malloc(byte_count + 15u);
    if (raw == NULL)
        return NULL;

    address = ((uintptr_t)raw + 15u) & ~(uintptr_t)15u;
    *allocation = raw;

    return (void *)address;
}

static int exp3_live_read_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    size_t done = 0;

    while (done < count) {
        int received = recv(socket_fd, destination + done, count - done, 0);

        if (received <= 0)
            return 0;

        done += (size_t)received;
    }

    return 1;
}

static int exp3_live_send_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    const uint8_t *source = (const uint8_t *)buffer;
    size_t done = 0;

    while (done < count) {
        int sent = send(socket_fd, source + done, count - done, 0);

        if (sent <= 0)
            return 0;

        done += (size_t)sent;
    }

    return 1;
}

static uint32_t exp3_live_crc32_update(
    uint32_t crc,
    const uint8_t *data,
    size_t count)
{
    size_t index;

    for (index = 0; index < count; ++index) {
        int bit;

        crc ^= (uint32_t)data[index];

        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return crc;
}

static int exp3_live_send_frame(
    Exp3LiveMpegStreamRuntime *runtime,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    pstvnc_transport_header_t header;

    if (
        runtime == NULL ||
        runtime->socket_fd < 0 ||
        payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        (payload == NULL && payload_length != 0)
    ) {
        return 0;
    }

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = kind;
    header.channel = channel;
    header.flags = flags;
    header.sequence = runtime->next_send_sequence;
    header.payload_length = (uint32_t)payload_length;

    if (!pstvnc_transport_header_encode(wire_header, &header))
        return 0;

    if (!exp3_live_send_exact(runtime->socket_fd, wire_header, sizeof(wire_header)))
        return 0;

    if (
        payload_length != 0 &&
        !exp3_live_send_exact(runtime->socket_fd, payload, payload_length)
    ) {
        return 0;
    }

    runtime->next_send_sequence += 1;
    return 1;
}

static int exp3_live_send_credit(
    Exp3LiveMpegStreamRuntime *runtime,
    uint32_t credit_bytes)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    if (credit_bytes == 0)
        return 1;

    pstvnc_transport_write_be32(payload, credit_bytes);

    if (
        !exp3_live_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_MPEG2,
            0,
            payload,
            sizeof(payload))
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEND;
        return 0;
    }

    runtime->credit_frames_sent += 1;
    runtime->credit_bytes_sent += credit_bytes;

    return 1;
}

static int exp3_live_receive_config(
    Exp3LiveMpegStreamRuntime *runtime,
    uint8_t config_payload[EXP3_O3_CONFIG_PAYLOAD_BYTES])
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    pstvnc_transport_header_t header;

    uint32_t version;
    uint32_t queue_capacity;
    uint32_t stack_size;
    uint32_t priority;

    size_t queue_bytes;
    size_t stack_bytes;

    if (!exp3_live_read_exact(runtime->socket_fd, wire_header, sizeof(wire_header))) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_RECEIVE;
        return 0;
    }

    if (!pstvnc_transport_header_decode(&header, wire_header)) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_HEADER;
        return 0;
    }

    if (
        header.sequence != 0u ||
        header.kind != PSTVNC_TRANSPORT_FRAME_CONFIG ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.payload_length != EXP3_O3_CONFIG_PAYLOAD_BYTES
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_CONFIG;
        return 0;
    }

    if (
        !exp3_live_read_exact(
            runtime->socket_fd,
            config_payload,
            EXP3_O3_CONFIG_PAYLOAD_BYTES)
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_RECEIVE;
        return 0;
    }

    version = pstvnc_transport_read_be32(&config_payload[0]);
    queue_capacity = pstvnc_transport_read_be32(&config_payload[4]);
    stack_size = pstvnc_transport_read_be32(&config_payload[8]);
    priority = pstvnc_transport_read_be32(&config_payload[12]);

    queue_bytes = (size_t)queue_capacity;
    stack_bytes = (size_t)stack_size;

    if (
        version != EXP3_O3_CONFIG_VERSION ||
        queue_capacity < PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        (queue_capacity % PSTVNC_TRANSPORT_MAX_PAYLOAD) != 0u ||
        (uint32_t)queue_bytes != queue_capacity ||
        stack_size < 256u ||
        (stack_size % 16u) != 0u ||
        stack_size > 0x7FFFFFFFu ||
        (uint32_t)stack_bytes != stack_size ||
        priority == 0u ||
        priority > 127u
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_CONFIG;
        return 0;
    }

    runtime->queue_capacity = queue_capacity;
    runtime->receiver_thread_stack_size = stack_size;
    runtime->receiver_thread_priority = priority;

    printf(
        "EXP3_O3_CONFIG queue_capacity=%u receiver_stack=%u receiver_priority=%u\n",
        (unsigned int)runtime->queue_capacity,
        (unsigned int)runtime->receiver_thread_stack_size,
        (unsigned int)runtime->receiver_thread_priority);

    return 1;
}

static void exp3_live_receiver_fail(
    Exp3LiveMpegStreamRuntime *runtime,
    int error)
{
    if (runtime == NULL || runtime->queue_sema_id < 0)
        return;

    if (WaitSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        runtime->receiver_done = 1;
        return;
    }

    if (runtime->error == EXP3_LIVE_STREAM_ERROR_NONE)
        runtime->error = error;

    runtime->receiver_done = 1;

    if (SignalSema(runtime->queue_sema_id) < 0)
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
}

static int exp3_live_accept_data(
    Exp3LiveMpegStreamRuntime *runtime,
    const pstvnc_transport_header_t *header)
{
    if (header->payload_length == 0u || header->payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_HEADER);
        return 0;
    }

    if (
        !exp3_live_read_exact(
            runtime->socket_fd,
            runtime->receiver_payload,
            header->payload_length)
    ) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_RECEIVE);
        return 0;
    }

    if (runtime->bytes_received > 0xFFFFFFFFu - header->payload_length) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_OVERFLOW);
        return 0;
    }

    if (WaitSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        runtime->receiver_done = 1;
        return 0;
    }

    if (
        header->payload_length > pstvnc_transport_queue_free(&runtime->queue) ||
        !pstvnc_transport_queue_write(
            &runtime->queue,
            runtime->receiver_payload,
            header->payload_length)
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_QUEUE_FULL;
        runtime->receiver_done = 1;
        (void)SignalSema(runtime->queue_sema_id);
        return 0;
    }

    runtime->crc_state = exp3_live_crc32_update(
        runtime->crc_state,
        runtime->receiver_payload,
        header->payload_length);

    runtime->frames_received += 1;
    runtime->bytes_received += header->payload_length;
    runtime->last_data_sequence = header->sequence;

    if (SignalSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        runtime->receiver_done = 1;
        return 0;
    }

    return 1;
}

static int exp3_live_accept_end(
    Exp3LiveMpegStreamRuntime *runtime,
    const pstvnc_transport_header_t *header)
{
    uint8_t payload[EXP3_O3_END_PAYLOAD_BYTES];
    uint32_t version;
    uint32_t final_crc32;
    int metadata_ok;

    if (header->payload_length != EXP3_O3_END_PAYLOAD_BYTES) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_END_METADATA);
        return 0;
    }

    if (!exp3_live_read_exact(runtime->socket_fd, payload, sizeof(payload))) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_RECEIVE);
        return 0;
    }

    version = pstvnc_transport_read_be32(&payload[0]);

    if (version != EXP3_O3_END_VERSION) {
        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_END_METADATA);
        return 0;
    }

    if (WaitSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        runtime->receiver_done = 1;
        return 0;
    }

    runtime->producer_bytes = pstvnc_transport_read_be32(&payload[4]);
    runtime->producer_frames = pstvnc_transport_read_be32(&payload[8]);
    runtime->producer_last_data_sequence = pstvnc_transport_read_be32(&payload[12]);
    runtime->producer_crc32 = pstvnc_transport_read_be32(&payload[16]);
    runtime->producer_picture_starts = pstvnc_transport_read_be32(&payload[20]);
    runtime->producer_sequence_headers = pstvnc_transport_read_be32(&payload[24]);
    runtime->producer_sequence_ends = pstvnc_transport_read_be32(&payload[28]);

    final_crc32 = runtime->crc_state ^ 0xFFFFFFFFu;
    runtime->crc32 = final_crc32;

    metadata_ok =
        runtime->producer_bytes == runtime->bytes_received &&
        runtime->producer_frames == runtime->frames_received &&
        runtime->producer_last_data_sequence == runtime->last_data_sequence &&
        runtime->producer_crc32 == final_crc32 &&
        runtime->producer_frames != 0u &&
        runtime->producer_bytes != 0u &&
        runtime->producer_picture_starts != 0u;

    runtime->end_received = 1;
    runtime->receiver_done = 1;

    if (!metadata_ok && runtime->error == EXP3_LIVE_STREAM_ERROR_NONE)
        runtime->error = EXP3_LIVE_STREAM_ERROR_END_METADATA;

    if (SignalSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        return 0;
    }

    return metadata_ok;
}

static void exp3_live_receiver_thread(void *argument)
{
    Exp3LiveMpegStreamRuntime *runtime = (Exp3LiveMpegStreamRuntime *)argument;

    for (;;) {
        uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
        pstvnc_transport_header_t header;

        if (!exp3_live_read_exact(runtime->socket_fd, wire_header, sizeof(wire_header))) {
            exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_RECEIVE);
            break;
        }

        if (!pstvnc_transport_header_decode(&header, wire_header)) {
            exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_HEADER);
            break;
        }

        if (header.sequence != runtime->expected_receive_sequence) {
            exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_SEQUENCE);
            break;
        }

        if (
            header.kind == PSTVNC_TRANSPORT_FRAME_DATA &&
            header.channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&
            header.flags == 0u
        ) {
            if (!exp3_live_accept_data(runtime, &header))
                break;

            runtime->expected_receive_sequence += 1;
            continue;
        }

        if (
            header.kind == PSTVNC_TRANSPORT_FRAME_TELEMETRY &&
            header.channel == PSTVNC_TRANSPORT_CHANNEL_TELEMETRY &&
            header.flags == 0u
        ) {
            (void)exp3_live_accept_end(runtime, &header);
            runtime->expected_receive_sequence += 1;
            break;
        }

        exp3_live_receiver_fail(runtime, EXP3_LIVE_STREAM_ERROR_HEADER);
        break;
    }

    ExitThread();
}

static int exp3_live_start_receiver(Exp3LiveMpegStreamRuntime *runtime)
{
    ee_thread_t thread;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)exp3_live_receiver_thread;
    thread.stack = runtime->receiver_thread_stack;
    thread.stack_size = (int)runtime->receiver_thread_stack_size;
    thread.gp_reg = &_gp;
    thread.initial_priority = (int)runtime->receiver_thread_priority;
    thread.attr = 0;
    thread.option = 0;

    runtime->receiver_thread_id = CreateThread(&thread);

    if (runtime->receiver_thread_id < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_THREAD;
        return 0;
    }

    if (StartThread(runtime->receiver_thread_id, runtime) < 0) {
        (void)DeleteThread(runtime->receiver_thread_id);
        runtime->receiver_thread_id = -1;
        runtime->error = EXP3_LIVE_STREAM_ERROR_THREAD;
        return 0;
    }

    runtime->receiver_thread_started = 1;
    return 1;
}

int exp3_live_mpeg_stream_start(Exp3LiveMpegStreamRuntime *runtime)
{
    struct sockaddr_in server;
    uint8_t config_payload[EXP3_O3_CONFIG_PAYLOAD_BYTES];
    size_t queue_bytes;
    size_t stack_bytes;

    if (runtime == NULL)
        return 0;

    memset(runtime, 0, sizeof(*runtime));
    runtime->socket_fd = -1;
    runtime->queue_sema_id = -1;
    runtime->receiver_thread_id = -1;
    runtime->next_send_sequence = 0u;
    runtime->expected_receive_sequence = 1u;
    runtime->crc_state = 0xFFFFFFFFu;
    runtime->error = EXP3_LIVE_STREAM_ERROR_NONE;

    runtime->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (runtime->socket_fd < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SOCKET;
        return 0;
    }

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(EXP3_O3_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(EXP3_O3_SERVER_IP);

    if (
        connect(
            runtime->socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_CONNECT;
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    if (!exp3_live_receive_config(runtime, config_payload)) {
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    queue_bytes = (size_t)runtime->queue_capacity;
    stack_bytes = (size_t)runtime->receiver_thread_stack_size;

    runtime->queue_storage = (uint8_t *)malloc(queue_bytes);

    if (runtime->queue_storage == NULL) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_ALLOCATION;
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    if (!pstvnc_transport_queue_init(&runtime->queue, runtime->queue_storage, queue_bytes)) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_ALLOCATION;
        free(runtime->queue_storage);
        runtime->queue_storage = NULL;
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    runtime->receiver_thread_stack = (uint8_t *)exp3_live_allocate_aligned16(
        stack_bytes,
        &runtime->receiver_thread_stack_allocation);

    if (runtime->receiver_thread_stack == NULL) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_ALLOCATION;
        free(runtime->queue_storage);
        runtime->queue_storage = NULL;
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    runtime->queue_sema_id = exp3_live_create_mutex();

    if (runtime->queue_sema_id < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        free(runtime->receiver_thread_stack_allocation);
        runtime->receiver_thread_stack_allocation = NULL;
        runtime->receiver_thread_stack = NULL;
        free(runtime->queue_storage);
        runtime->queue_storage = NULL;
        close(runtime->socket_fd);
        runtime->socket_fd = -1;
        return 0;
    }

    if (
        !exp3_live_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_CONFIG,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            EXP3_O3_CONFIG_ACK_FLAG,
            config_payload,
            sizeof(config_payload))
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEND;
        return 0;
    }

    if (!exp3_live_send_credit(runtime, runtime->queue_capacity))
        return 0;

    if (!exp3_live_start_receiver(runtime))
        return 0;

    printf(
        "EXP3_O3_STREAM_START=PASS queue_capacity=%u\n",
        (unsigned int)runtime->queue_capacity);

    return 1;
}

int exp3_live_mpeg_stream_read(
    Exp3LiveMpegStreamRuntime *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *actual_count)
{
    uint8_t *destination = (uint8_t *)buffer;
    uint32_t wait_loops = 0u;
    int waiting = 0;

    if (
        runtime == NULL ||
        buffer == NULL ||
        maximum_count == 0 ||
        actual_count == NULL ||
        runtime->queue_sema_id < 0
    ) {
        return 0;
    }

    *actual_count = 0;

    for (;;) {
        size_t available;
        size_t take;
        int queue_empty;
        int receiver_done;

        if (WaitSema(runtime->queue_sema_id) < 0) {
            runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
            return 0;
        }

        available = pstvnc_transport_queue_size(&runtime->queue);
        take = maximum_count;

        if (take > available)
            take = available;

        if (
            take != 0 &&
            !pstvnc_transport_queue_read(&runtime->queue, destination, take)
        ) {
            runtime->error = EXP3_LIVE_STREAM_ERROR_QUEUE_FULL;
            (void)SignalSema(runtime->queue_sema_id);
            return 0;
        }

        if (take != 0)
            runtime->bytes_consumed += (uint32_t)take;

        queue_empty = pstvnc_transport_queue_size(&runtime->queue) == 0;
        receiver_done = runtime->receiver_done;

        if (SignalSema(runtime->queue_sema_id) < 0) {
            runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
            return 0;
        }

        if (take != 0) {
            uint32_t credit_to_send = 0u;

            runtime->credit_pending += (uint32_t)take;

            if (
                runtime->credit_pending >= EXP3_O3_CREDIT_BATCH_BYTES ||
                (queue_empty && runtime->credit_pending != 0u)
            ) {
                credit_to_send = runtime->credit_pending;
                runtime->credit_pending = 0u;
            }

            if (credit_to_send != 0u && !exp3_live_send_credit(runtime, credit_to_send))
                return 0;

            if (waiting && wait_loops > runtime->feed_wait_max_loops)
                runtime->feed_wait_max_loops = wait_loops;

            *actual_count = take;
            return 1;
        }

        if (receiver_done) {
            if (waiting && wait_loops > runtime->feed_wait_max_loops)
                runtime->feed_wait_max_loops = wait_loops;

            return 1;
        }

        if (!waiting) {
            runtime->feed_wait_events += 1;
            waiting = 1;
        }

        wait_loops += 1;
        runtime->feed_wait_loops += 1;

        if (DelayThread(EXP3_O3_EMPTY_WAIT_US) < 0) {
            runtime->error = EXP3_LIVE_STREAM_ERROR_DELAY;
            return 0;
        }
    }
}

int exp3_live_mpeg_stream_is_exhausted(Exp3LiveMpegStreamRuntime *runtime)
{
    size_t current;
    int result;

    if (runtime == NULL || runtime->queue_sema_id < 0)
        return 0;

    if (WaitSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        return 0;
    }

    current = pstvnc_transport_queue_size(&runtime->queue);

    result =
        runtime->receiver_done &&
        runtime->end_received &&
        current == 0u &&
        runtime->bytes_consumed == runtime->producer_bytes;

    if (SignalSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        return 0;
    }

    return result;
}

int exp3_live_mpeg_stream_integrity_pass(Exp3LiveMpegStreamRuntime *runtime)
{
    if (runtime == NULL)
        return 0;

    return
        exp3_live_mpeg_stream_is_exhausted(runtime) &&
        runtime->error == EXP3_LIVE_STREAM_ERROR_NONE &&
        runtime->end_received &&
        runtime->frames_received == runtime->producer_frames &&
        runtime->bytes_received == runtime->producer_bytes &&
        runtime->last_data_sequence == runtime->producer_last_data_sequence &&
        runtime->crc32 == runtime->producer_crc32;
}

uint32_t exp3_live_mpeg_stream_remaining_bytes(Exp3LiveMpegStreamRuntime *runtime)
{
    if (
        runtime == NULL ||
        !runtime->end_received ||
        runtime->bytes_consumed >= runtime->producer_bytes
    ) {
        return 0u;
    }

    return runtime->producer_bytes - runtime->bytes_consumed;
}

int exp3_live_mpeg_stream_send_result(
    Exp3LiveMpegStreamRuntime *runtime,
    uint32_t pictures_decoded,
    uint32_t pictures_displayed,
    uint32_t feed_calls,
    uint32_t payload_bytes_submitted,
    uint32_t dma_bytes_submitted)
{
    uint8_t payload[EXP3_O3_RESULT_PAYLOAD_BYTES];
    uint32_t queue_current;
    uint32_t queue_high_water;
    uint32_t integrity_pass;

    if (runtime == NULL || runtime->queue_sema_id < 0)
        return 0;

    if (WaitSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        return 0;
    }

    queue_current = (uint32_t)pstvnc_transport_queue_size(&runtime->queue);
    queue_high_water = (uint32_t)pstvnc_transport_queue_high_water(&runtime->queue);

    if (SignalSema(runtime->queue_sema_id) < 0) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEMAPHORE;
        return 0;
    }

    integrity_pass = (uint32_t)exp3_live_mpeg_stream_integrity_pass(runtime);

#define PUT32(word, value) \
    pstvnc_transport_write_be32(&payload[(word) * 4u], (uint32_t)(value))

    PUT32(0, EXP3_O3_RESULT_VERSION);
    PUT32(1, runtime->error);
    PUT32(2, runtime->queue_capacity);
    PUT32(3, queue_current);
    PUT32(4, queue_high_water);
    PUT32(5, runtime->frames_received);
    PUT32(6, runtime->bytes_received);
    PUT32(7, runtime->last_data_sequence);
    PUT32(8, runtime->crc32);
    PUT32(9, runtime->bytes_consumed);
    PUT32(10, runtime->feed_wait_events);
    PUT32(11, runtime->feed_wait_loops);
    PUT32(12, runtime->feed_wait_max_loops);
    PUT32(13, runtime->credit_frames_sent);
    PUT32(14, runtime->credit_bytes_sent);
    PUT32(15, pictures_decoded);
    PUT32(16, pictures_displayed);
    PUT32(17, feed_calls);
    PUT32(18, payload_bytes_submitted);
    PUT32(19, dma_bytes_submitted);
    PUT32(20, runtime->receiver_done);
    PUT32(21, integrity_pass);
    PUT32(22, runtime->end_received);
    PUT32(23, runtime->producer_bytes);
    PUT32(24, runtime->producer_frames);
    PUT32(25, runtime->producer_last_data_sequence);
    PUT32(26, runtime->producer_crc32);
    PUT32(27, runtime->producer_picture_starts);
    PUT32(28, runtime->producer_sequence_headers);
    PUT32(29, runtime->producer_sequence_ends);

#undef PUT32

    if (
        !exp3_live_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_TELEMETRY,
            PSTVNC_TRANSPORT_CHANNEL_TELEMETRY,
            0,
            payload,
            sizeof(payload))
    ) {
        runtime->error = EXP3_LIVE_STREAM_ERROR_SEND;
        return 0;
    }

    return 1;
}

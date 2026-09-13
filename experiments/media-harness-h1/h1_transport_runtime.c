/*
 * File synopsis:
 * Implements H1's one-socket AUDIO + MPEG2 PSTV receiver, with cumulative-build
 * hooks for the prepared logical RFB channel 1.
 *
 * The Pi schedules logical channels. One EE receiver thread owns recv(), each
 * enabled media channel has its own runtime-allocated ring and credit stream,
 * and temporary MPEG queue starvation remains distinct from terminal EOF.
 *
 * In the cumulative RFB-prep target only, channel-1 resources are prepared
 * before the sole receiver thread starts, inbound RFB DATA is dispatched to its
 * independent queue, and initial RFB credit is emitted through the same
 * serialized send path. The authoritative CONFIG validator remains the public
 * activation gate.
 */

#include "h1_transport_runtime.h"

#ifdef PSTVNC_H1_RFB_MUX_PREP
#include "h1_rfb_transport_live.h"
#endif

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

#define H1_RECEIVER_STOP_WAIT_STEP_US 1000u
#define H1_RECEIVER_STOP_WAIT_STEPS 3000u

static void h1_record_error(
    pstvnc_h1_transport_runtime_t *runtime,
    pstvnc_h1_transport_error_t error)
{
    if (runtime != NULL && runtime->error == PSTVNC_H1_ERROR_NONE)
        runtime->error = error;
}

static int h1_create_mutex(void)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;
    return CreateSema(&semaphore);
}

static void *h1_allocate_aligned16(
    size_t byte_count,
    void **allocation)
{
    uintptr_t address;
    void *raw;

    if (allocation == NULL ||
        byte_count == 0u ||
        byte_count > ((size_t)-1 - 15u))
        return NULL;

    raw = malloc(byte_count + 15u);
    if (raw == NULL)
        return NULL;

    address = ((uintptr_t)raw + 15u) & ~(uintptr_t)15u;
    *allocation = raw;
    return (void *)address;
}

static int h1_socket_read_exact(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t count)
{
    uint8_t *destination = (uint8_t *)buffer;
    size_t done = 0u;

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

static int h1_socket_send_exact_locked(
    pstvnc_h1_transport_runtime_t *runtime,
    const void *buffer,
    size_t count)
{
    const uint8_t *source = (const uint8_t *)buffer;
    size_t done = 0u;

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

static uint32_t h1_crc32_update(
    uint32_t crc,
    const uint8_t *data,
    size_t count)
{
    size_t index;

    for (index = 0u; index < count; ++index) {
        int bit;

        crc ^= (uint32_t)data[index];

        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return crc;
}

static int h1_send_frame(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    const void *payload,
    size_t payload_length)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    pstvnc_transport_header_t header;
    int ok = 0;

    if (runtime == NULL ||
        runtime->socket_fd < 0 ||
        runtime->send_sema_id < 0 ||
        payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD ||
        (payload == NULL && payload_length != 0u)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return 0;
    }

    if (WaitSema(runtime->send_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = kind;
    header.channel = channel;
    header.flags = flags;
    header.sequence = runtime->next_send_sequence;
    header.payload_length = (uint32_t)payload_length;

    if (!pstvnc_transport_header_encode(wire_header, &header)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_HEADER);
        goto done;
    }

    if (!h1_socket_send_exact_locked(
            runtime,
            wire_header,
            sizeof(wire_header))) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEND);
        goto done;
    }

    if (payload_length != 0u &&
        !h1_socket_send_exact_locked(runtime, payload, payload_length)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEND);
        goto done;
    }

    runtime->stats.last_sent_sequence = runtime->next_send_sequence;
    runtime->next_send_sequence++;
    ok = 1;

done:
    if (SignalSema(runtime->send_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    return ok;
}

/*
 * Experiment-internal bridge used by logical RFB channel 1. It intentionally
 * exposes no raw socket access: every frame still passes through H1's existing
 * send semaphore, sequence counter, framing, and error accounting.
 */
int pstvnc_h1_transport_send_frame_internal(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t kind,
    uint8_t channel,
    const void *payload,
    size_t payload_length)
{
    return h1_send_frame(
        runtime,
        kind,
        channel,
        0,
        payload,
        payload_length);
}

static int h1_send_error_code(
    pstvnc_h1_transport_runtime_t *runtime)
{
    uint8_t payload[4];

    pstvnc_transport_write_be32(payload, (uint32_t)runtime->error);
    return h1_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_ERROR,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0,
        payload,
        sizeof(payload));
}

static int h1_send_hello(
    pstvnc_h1_transport_runtime_t *runtime)
{
    uint8_t payload[PSTVNC_TRANSPORT_HELLO_PAYLOAD_SIZE];
    uint32_t capabilities =
        PSTVNC_TRANSPORT_CAP_AUDIO_PCM_S16 |
        PSTVNC_TRANSPORT_CAP_RECEIVER_CREDIT |
        PSTVNC_TRANSPORT_CAP_TELEMETRY_SNAPSHOT |
        PSTVNC_TRANSPORT_CAP_CONFIG_DYNAMIC |
        PSTVNC_H1_CAP_MEDIA_MUX_DYNAMIC |
        PSTVNC_H1_CAP_MPEG2_ES |
        PSTVNC_H1_CAP_REPEAT_SESSIONS;

    /* CAP_RFB deliberately remains absent until the activation gate is opened. */
    memset(payload, 0, sizeof(payload));
    pstvnc_transport_write_be32(&payload[0], capabilities);
    pstvnc_transport_write_be32(
        &payload[4], PSTVNC_TRANSPORT_MAX_PAYLOAD);

    return h1_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_HELLO,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0,
        payload,
        sizeof(payload));
}

static int h1_receive_config(
    pstvnc_h1_transport_runtime_t *runtime)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    pstvnc_transport_header_t header;

    if (!h1_socket_read_exact(runtime, wire_header, sizeof(wire_header))) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_RECEIVE);
        return 0;
    }

    if (!pstvnc_transport_header_decode(&header, wire_header)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_HEADER);
        return 0;
    }

    if (header.sequence != runtime->expected_receive_sequence ||
        header.kind != PSTVNC_TRANSPORT_FRAME_CONFIG ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.payload_length == 0u ||
        header.payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CONFIG);
        return 0;
    }

    if (!h1_socket_read_exact(
            runtime,
            runtime->receiver_payload,
            header.payload_length)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_RECEIVE);
        return 0;
    }

    runtime->stats.frames_received++;
    runtime->stats.payload_bytes_received += header.payload_length;
    runtime->stats.last_received_sequence = header.sequence;
    runtime->expected_receive_sequence++;

    if (!pstvnc_h1_config_decode(
            &runtime->config,
            runtime->receiver_payload,
            header.payload_length) ||
        !pstvnc_h1_config_validate(&runtime->config)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CONFIG);
        return 0;
    }

    runtime->config_payload_length = header.payload_length;
    runtime->config_digest = pstvnc_h1_config_digest(
        runtime->receiver_payload,
        header.payload_length);
    runtime->config_accepted = 1;
    return 1;
}

static int h1_allocate_one_queue(
    pstvnc_transport_queue_t *queue,
    uint8_t **storage,
    uint32_t requested)
{
    size_t bytes = (size_t)requested;

    if (requested == 0u)
        return 1;

    if ((uint32_t)bytes != requested)
        return 0;

    *storage = (uint8_t *)malloc(bytes);
    if (*storage == NULL)
        return 0;

    if (!pstvnc_transport_queue_init(queue, *storage, bytes)) {
        free(*storage);
        *storage = NULL;
        return 0;
    }

    return 1;
}

static int h1_allocate_queues(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int first_ok;
    int second_ok;

    if (runtime->config.queue_allocation_order ==
        PSTVNC_H1_ALLOCATE_MPEG_FIRST) {
        first_ok = h1_allocate_one_queue(
            &runtime->mpeg_queue,
            &runtime->mpeg_queue_storage,
            runtime->config.mpeg_queue_capacity);
        second_ok = first_ok && h1_allocate_one_queue(
            &runtime->audio_queue,
            &runtime->audio_queue_storage,
            runtime->config.audio_queue_capacity);
    } else {
        first_ok = h1_allocate_one_queue(
            &runtime->audio_queue,
            &runtime->audio_queue_storage,
            runtime->config.audio_queue_capacity);
        second_ok = first_ok && h1_allocate_one_queue(
            &runtime->mpeg_queue,
            &runtime->mpeg_queue_storage,
            runtime->config.mpeg_queue_capacity);
    }

    if (!first_ok || !second_ok) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ALLOCATION);
        free(runtime->audio_queue_storage);
        free(runtime->mpeg_queue_storage);
        runtime->audio_queue_storage = NULL;
        runtime->mpeg_queue_storage = NULL;
        return 0;
    }

    return 1;
}

static int h1_allocate_receiver_stack(
    pstvnc_h1_transport_runtime_t *runtime)
{
    size_t bytes = (size_t)runtime->config.receiver_thread_stack_size;

    if ((uint32_t)bytes != runtime->config.receiver_thread_stack_size)
        return 0;

    runtime->receiver_thread_stack =
        (unsigned char *)h1_allocate_aligned16(
            bytes,
            &runtime->receiver_thread_stack_allocation);

    if (runtime->receiver_thread_stack == NULL) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ALLOCATION);
        return 0;
    }

    return 1;
}

static int h1_apply_socket_options(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int value;

    if (runtime->config.socket_receive_buffer_bytes != 0u) {
        value = (int)runtime->config.socket_receive_buffer_bytes;
        if (setsockopt(
                runtime->socket_fd,
                SOL_SOCKET,
                SO_RCVBUF,
                &value,
                sizeof(value)) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SOCKET);
            return 0;
        }
    }

    if (runtime->config.socket_send_buffer_bytes != 0u) {
        value = (int)runtime->config.socket_send_buffer_bytes;
        if (setsockopt(
                runtime->socket_fd,
                SOL_SOCKET,
                SO_SNDBUF,
                &value,
                sizeof(value)) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SOCKET);
            return 0;
        }
    }

    return 1;
}

static int h1_send_config_ack(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_CONFIG,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        PSTVNC_H1_CONFIG_ACK_FLAG,
        runtime->receiver_payload,
        runtime->config_payload_length);
}

static int h1_send_credit(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    if (amount == 0u)
        return 1;

    pstvnc_transport_write_be32(payload, amount);

    if (!h1_send_frame(
            runtime,
            PSTVNC_TRANSPORT_FRAME_CREDIT,
            channel,
            0,
            payload,
            sizeof(payload)))
        return 0;

    if (channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        runtime->stats.audio_credit_frames_sent++;
        runtime->stats.audio_credit_bytes_sent += amount;
    } else if (channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2) {
        runtime->stats.mpeg_credit_frames_sent++;
        runtime->stats.mpeg_credit_bytes_sent += amount;
    }

    return 1;
}

static int h1_snapshot_queue(
    pstvnc_h1_transport_runtime_t *runtime,
    int sema,
    const pstvnc_transport_queue_t *queue,
    int enabled,
    uint32_t *current,
    uint32_t *high_water)
{
    if (!enabled) {
        *current = 0u;
        *high_water = 0u;
        return 1;
    }

    if (WaitSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    *current = (uint32_t)pstvnc_transport_queue_size(queue);
    *high_water = (uint32_t)pstvnc_transport_queue_high_water(queue);

    if (SignalSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    return 1;
}

static int h1_send_telemetry(
    pstvnc_h1_transport_runtime_t *runtime)
{
    uint8_t payload[PSTVNC_H1_TELEMETRY_BYTES];
    uint32_t audio_current;
    uint32_t audio_high_water;
    uint32_t mpeg_current;
    uint32_t mpeg_high_water;

    if (!h1_snapshot_queue(
            runtime,
            runtime->audio_queue_sema_id,
            &runtime->audio_queue,
            runtime->config.audio_mode != PSTVNC_H1_AUDIO_OFF,
            &audio_current,
            &audio_high_water) ||
        !h1_snapshot_queue(
            runtime,
            runtime->mpeg_queue_sema_id,
            &runtime->mpeg_queue,
            runtime->config.video_mode != PSTVNC_H1_VIDEO_OFF,
            &mpeg_current,
            &mpeg_high_water))
        return 0;

    memset(payload, 0, sizeof(payload));

#define PUT32(offset, value) \
    pstvnc_transport_write_be32(&payload[(offset)], (uint32_t)(value))

    PUT32(0, PSTVNC_H1_TELEMETRY_VERSION);
    PUT32(4, runtime->error);
    PUT32(8, runtime->config.session_id);
    PUT32(12, runtime->diagnostic_word);
    PUT32(16, runtime->stats.frames_received);
    PUT32(20, runtime->stats.payload_bytes_received);
    PUT32(24, runtime->stats.last_received_sequence);
    PUT32(28, runtime->stats.last_sent_sequence);
    PUT32(32, audio_current);
    PUT32(36, audio_high_water);
    PUT32(40, runtime->stats.audio_bytes_enqueued);
    PUT32(44, runtime->stats.audio_bytes_consumed);
    PUT32(48, runtime->stats.audio_credit_bytes_sent);
    PUT32(52, runtime->stats.audio_chunks_played);
    PUT32(56, runtime->stats.audio_bytes_played);
    PUT32(60, runtime->stats.audio_read_calls);
    PUT32(64, mpeg_current);
    PUT32(68, mpeg_high_water);
    PUT32(72, runtime->stats.mpeg_bytes_enqueued);
    PUT32(76, runtime->stats.mpeg_bytes_consumed);
    PUT32(80, runtime->stats.mpeg_credit_bytes_sent);
    PUT32(84, runtime->stats.mpeg_read_calls);
    PUT32(88, runtime->stats.mpeg_wait_events);
    PUT32(92, runtime->stats.mpeg_wait_loops);
    PUT32(96, runtime->stats.mpeg_wait_max_loops);
    PUT32(100, runtime->end_received);
    PUT32(104, runtime->receiver_done);
    PUT32(108, runtime->producer_audio_bytes);
    PUT32(112, runtime->producer_mpeg_bytes);
    PUT32(116, runtime->producer_picture_starts);
    PUT32(120, runtime->config_digest);
    PUT32(124, runtime->config.profile_id);
    PUT32(128, runtime->config.audio_initial_credit_bytes);
    PUT32(132, runtime->config.mpeg_initial_credit_bytes);
    PUT32(136, runtime->config.audio_queue_capacity);
    PUT32(140, runtime->config.mpeg_queue_capacity);
    PUT32(144, runtime->config.audio_mode);
    PUT32(148, runtime->config.video_mode);
    PUT32(152, runtime->stats.receiver_loop_count);
    PUT32(156, runtime->mpeg_diag_stage);

#undef PUT32

    return h1_send_frame(
        runtime,
        PSTVNC_TRANSPORT_FRAME_TELEMETRY,
        PSTVNC_TRANSPORT_CHANNEL_TELEMETRY,
        0,
        payload,
        sizeof(payload));
}

static int h1_queue_for_channel(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t channel,
    pstvnc_transport_queue_t **queue,
    int *sema,
    uint32_t *frame_bytes)
{
    if (channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO &&
        runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM) {
        *queue = &runtime->audio_queue;
        *sema = runtime->audio_queue_sema_id;
        *frame_bytes = pstvnc_h1_config_audio_frame_bytes(&runtime->config);
        return 1;
    }

    if (channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        *queue = &runtime->mpeg_queue;
        *sema = runtime->mpeg_queue_sema_id;
        *frame_bytes = 1u;
        return 1;
    }

    return 0;
}

static int h1_accept_data(
    pstvnc_h1_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    pstvnc_transport_queue_t *queue;
    int sema;
    uint32_t alignment;
    int written;

#ifdef PSTVNC_H1_RFB_MUX_PREP
    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_RFB)
        return pstvnc_h1_rfb_transport_accept_data(
            runtime,
            runtime->receiver_payload,
            header->payload_length);
#endif

    /*
     * Channel-4 DATA is session-framed but generation-owned by CP2P. The ACK
     * receiver closes mpeg_data_generation at the ordered wire fence, so any
     * later stale DATA is a protocol error rather than input to N+1.
     */
    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_MPEG2 &&
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&
        runtime->mpeg_data_generation == 0u) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
        return 0;
    }

    if (header->payload_length == 0u ||
        header->payload_length > runtime->config.max_data_payload ||
        !h1_queue_for_channel(
            runtime,
            header->channel,
            &queue,
            &sema,
            &alignment)) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CHANNEL);
        return 0;
    }

    if ((header->payload_length % alignment) != 0u) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_AUDIO_ALIGNMENT);
        return 0;
    }

    if (WaitSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    written = pstvnc_transport_queue_write(
        queue,
        runtime->receiver_payload,
        header->payload_length);

    if (SignalSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }

    if (!written) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
        return 0;
    }

    if (header->channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        if (runtime->stats.audio_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->audio_crc_state = h1_crc32_update(
            runtime->audio_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.audio_frames_received++;
        runtime->stats.audio_bytes_enqueued += header->payload_length;
        runtime->audio_last_data_sequence = header->sequence;
    } else {
        if (runtime->stats.mpeg_bytes_enqueued >
            0xffffffffu - header->payload_length) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        runtime->mpeg_crc_state = h1_crc32_update(
            runtime->mpeg_crc_state,
            runtime->receiver_payload,
            header->payload_length);
        runtime->stats.mpeg_frames_received++;
        runtime->stats.mpeg_bytes_enqueued += header->payload_length;
        runtime->mpeg_last_data_sequence = header->sequence;
    }

    return 1;
}

static int h1_accept_end(
    pstvnc_h1_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    int ok = 1;

    if (header->channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header->payload_length != PSTVNC_H1_MEDIA_END_BYTES) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_END_METADATA);
        return 0;
    }

    if (pstvnc_transport_read_be32(&runtime->receiver_payload[0]) !=
        PSTVNC_H1_MEDIA_END_VERSION) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_END_METADATA);
        return 0;
    }

    runtime->producer_session_id =
        pstvnc_transport_read_be32(&runtime->receiver_payload[4]);
    runtime->producer_audio_bytes =
        pstvnc_transport_read_be32(&runtime->receiver_payload[8]);
    runtime->producer_audio_frames =
        pstvnc_transport_read_be32(&runtime->receiver_payload[12]);
    runtime->producer_audio_last_sequence =
        pstvnc_transport_read_be32(&runtime->receiver_payload[16]);
    runtime->producer_audio_crc32 =
        pstvnc_transport_read_be32(&runtime->receiver_payload[20]);
    runtime->producer_mpeg_bytes =
        pstvnc_transport_read_be32(&runtime->receiver_payload[24]);
    runtime->producer_mpeg_frames =
        pstvnc_transport_read_be32(&runtime->receiver_payload[28]);
    runtime->producer_mpeg_last_sequence =
        pstvnc_transport_read_be32(&runtime->receiver_payload[32]);
    runtime->producer_mpeg_crc32 =
        pstvnc_transport_read_be32(&runtime->receiver_payload[36]);
    runtime->producer_picture_starts =
        pstvnc_transport_read_be32(&runtime->receiver_payload[40]);
    runtime->producer_sequence_headers =
        pstvnc_transport_read_be32(&runtime->receiver_payload[44]);
    runtime->producer_sequence_ends =
        pstvnc_transport_read_be32(&runtime->receiver_payload[48]);
    runtime->producer_stop_reason =
        pstvnc_transport_read_be32(&runtime->receiver_payload[52]);

    runtime->audio_crc32 = runtime->audio_crc_state ^ 0xffffffffu;
    runtime->mpeg_crc32 = runtime->mpeg_crc_state ^ 0xffffffffu;

    if (runtime->producer_session_id != runtime->config.session_id)
        ok = 0;

    if (runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM) {
        if (runtime->producer_audio_bytes !=
                runtime->stats.audio_bytes_enqueued ||
            runtime->producer_audio_frames !=
                runtime->stats.audio_frames_received ||
            runtime->producer_audio_last_sequence !=
                runtime->audio_last_data_sequence ||
            runtime->producer_audio_crc32 != runtime->audio_crc32)
            ok = 0;
    } else if (runtime->producer_audio_bytes != 0u ||
               runtime->producer_audio_frames != 0u) {
        ok = 0;
    }

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        if (runtime->producer_mpeg_bytes !=
                runtime->stats.mpeg_bytes_enqueued ||
            runtime->producer_mpeg_frames !=
                runtime->stats.mpeg_frames_received ||
            runtime->producer_mpeg_last_sequence !=
                runtime->mpeg_last_data_sequence ||
            runtime->producer_mpeg_crc32 != runtime->mpeg_crc32 ||
            runtime->producer_picture_starts == 0u)
            ok = 0;
    } else if (runtime->producer_mpeg_bytes != 0u ||
               runtime->producer_mpeg_frames != 0u) {
        ok = 0;
    }

    runtime->end_received = 1;
    runtime->receiver_done = 1;

    if (!ok)
        h1_record_error(runtime, PSTVNC_H1_ERROR_END_METADATA);

    return ok;
}

static int h1_accept_mpeg_retire(
    pstvnc_h1_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    uint32_t version;
    uint32_t session_id;
    uint32_t generation;

    if (runtime == NULL || header == NULL ||
        header->channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header->flags != 0u ||
        header->payload_length != PSTVNC_H1_MPEG_RETIRE_BYTES) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
        return 0;
    }

    version = pstvnc_transport_read_be32(&runtime->receiver_payload[0]);
    session_id = pstvnc_transport_read_be32(&runtime->receiver_payload[4]);
    generation = pstvnc_transport_read_be32(&runtime->receiver_payload[8]);

    if (version != PSTVNC_H1_MPEG_RETIRE_VERSION ||
        session_id != runtime->config.session_id ||
        generation == 0u ||
        runtime->mpeg_data_generation != generation ||
        runtime->mpeg_retire_pending_generation != generation ||
        runtime->mpeg_retire_ack_generation != 0u) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
        return 0;
    }

    /*
     * TCP/PSTV order makes this ACK the wire fence: all earlier generation-N
     * DATA has already been accepted into the queue by this sole receiver.
     * Close DATA acceptance immediately; local worker/queue cleanup follows.
     */
    runtime->mpeg_data_generation = 0u;
    runtime->mpeg_retire_ack_generation = generation;
    return 1;
}

static int h1_accept_frame(
    pstvnc_h1_transport_runtime_t *runtime,
    const pstvnc_transport_header_t *header)
{
    switch (header->kind) {
        case PSTVNC_TRANSPORT_FRAME_DATA:
            return h1_accept_data(runtime, header);

        case PSTVNC_TRANSPORT_FRAME_HELLO:
            if (header->channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
                header->payload_length != 0u) {
                h1_record_error(runtime, PSTVNC_H1_ERROR_HEADER);
                return 0;
            }
            return 1;

        case PSTVNC_TRANSPORT_FRAME_HEARTBEAT:
            if (header->channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
                header->payload_length != 0u) {
                h1_record_error(runtime, PSTVNC_H1_ERROR_HEADER);
                return 0;
            }
            return h1_send_telemetry(runtime);

        case PSTVNC_H1_FRAME_MEDIA_END:
            return h1_accept_end(runtime, header);

        case PSTVNC_H1_FRAME_MPEG_RETIRE:
            return h1_accept_mpeg_retire(runtime, header);

        default:
            h1_record_error(runtime, PSTVNC_H1_ERROR_FRAME_KIND);
            return 0;
    }
}

static void h1_receiver_thread(void *argument)
{
    pstvnc_h1_transport_runtime_t *runtime =
        (pstvnc_h1_transport_runtime_t *)argument;

    while (!runtime->stop_requested) {
        uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
        pstvnc_transport_header_t header;

        runtime->stats.receiver_loop_count++;

        if (!h1_socket_read_exact(runtime, wire_header, sizeof(wire_header))) {
            if (!runtime->stop_requested)
                h1_record_error(runtime, PSTVNC_H1_ERROR_RECEIVE);
            break;
        }

        if (!pstvnc_transport_header_decode(&header, wire_header)) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_HEADER);
            break;
        }

        if (header.sequence != runtime->expected_receive_sequence ||
            header.payload_length > PSTVNC_TRANSPORT_MAX_PAYLOAD) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEQUENCE);
            break;
        }

        if (header.payload_length != 0u &&
            !h1_socket_read_exact(
                runtime,
                runtime->receiver_payload,
                header.payload_length)) {
            if (!runtime->stop_requested)
                h1_record_error(runtime, PSTVNC_H1_ERROR_RECEIVE);
            break;
        }

        runtime->stats.frames_received++;
        runtime->stats.payload_bytes_received += header.payload_length;
        runtime->stats.last_received_sequence = header.sequence;
        runtime->expected_receive_sequence++;

        if (!h1_accept_frame(runtime, &header))
            break;

        if (runtime->end_received)
            break;
    }

    runtime->receiver_done = 1;
    ExitThread();
}

static int h1_start_receiver(
    pstvnc_h1_transport_runtime_t *runtime)
{
    ee_thread_t thread;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)h1_receiver_thread;
    thread.stack = runtime->receiver_thread_stack;
    thread.stack_size = (int)runtime->config.receiver_thread_stack_size;
    thread.gp_reg = &_gp;
    thread.initial_priority = (int)runtime->config.receiver_thread_priority;
    thread.attr = 0;
    thread.option = 0;

    runtime->receiver_thread_id = CreateThread(&thread);
    if (runtime->receiver_thread_id < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_THREAD);
        return 0;
    }

    if (StartThread(runtime->receiver_thread_id, runtime) < 0) {
        (void)DeleteThread(runtime->receiver_thread_id);
        runtime->receiver_thread_id = -1;
        h1_record_error(runtime, PSTVNC_H1_ERROR_THREAD);
        return 0;
    }

    runtime->receiver_thread_started = 1;
    return 1;
}

static int h1_return_credit(
    pstvnc_h1_transport_runtime_t *runtime,
    uint8_t channel,
    uint32_t consumed,
    int queue_empty)
{
    uint32_t *pending;
    uint32_t batch;
    uint32_t flush;
    uint32_t enabled;
    uint32_t amount = 0u;

    if (channel == PSTVNC_TRANSPORT_CHANNEL_AUDIO) {
        pending = &runtime->audio_credit_pending;
        batch = runtime->config.audio_credit_batch_bytes;
        flush = runtime->config.audio_credit_flush_on_empty;
        enabled = runtime->config.audio_credit_return_enabled;
    } else {
        pending = &runtime->mpeg_credit_pending;
        batch = runtime->config.mpeg_credit_batch_bytes;
        flush = runtime->config.mpeg_credit_flush_on_empty;
        enabled = runtime->config.mpeg_credit_return_enabled;
    }

    if (!enabled)
        return 1;

    *pending += consumed;

    if (*pending >= batch ||
        (flush != 0u && queue_empty && *pending != 0u)) {
        amount = *pending;
        *pending = 0u;
    }

    return amount == 0u || h1_send_credit(runtime, channel, amount);
}

static int h1_mpeg_queue_empty_for_generation_boundary(
    pstvnc_h1_transport_runtime_t *runtime)
{
    int empty;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_OFF)
        return 1;
    if (runtime->config.video_mode != PSTVNC_H1_VIDEO_MPEG2_ES)
        return 0;

    if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }
    empty = pstvnc_transport_queue_size(&runtime->mpeg_queue) == 0u;
    if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0;
    }
    return empty;
}

int pstvnc_h1_transport_mpeg_generation_open(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->mpeg_data_generation != 0u ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u ||
        runtime->mpeg_credit_pending != 0u ||
        !h1_mpeg_queue_empty_for_generation_boundary(runtime))
        return 0;

    runtime->mpeg_data_generation = generation;
    return 1;
}

int pstvnc_h1_transport_mpeg_generation_abort(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->mpeg_data_generation != generation ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u ||
        runtime->mpeg_credit_pending != 0u ||
        !h1_mpeg_queue_empty_for_generation_boundary(runtime))
        return 0;

    runtime->mpeg_data_generation = 0u;
    return 1;
}

int pstvnc_h1_transport_mpeg_retire_begin(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    uint8_t payload[PSTVNC_H1_MPEG_RETIRE_BYTES];

    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->stop_requested || runtime->receiver_done ||
        runtime->mpeg_data_generation != generation ||
        runtime->mpeg_retire_pending_generation != 0u ||
        runtime->mpeg_retire_ack_generation != 0u)
        return 0;

    pstvnc_transport_write_be32(
        &payload[0], PSTVNC_H1_MPEG_RETIRE_VERSION);
    pstvnc_transport_write_be32(
        &payload[4], runtime->config.session_id);
    pstvnc_transport_write_be32(&payload[8], generation);

    /* Arm exact completion matching before the request can reach the Pi. */
    runtime->mpeg_retire_pending_generation = generation;
    runtime->mpeg_retire_ack_generation = 0u;

    if (!h1_send_frame(
            runtime,
            PSTVNC_H1_FRAME_MPEG_RETIRE,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            payload,
            sizeof(payload))) {
        runtime->mpeg_retire_pending_generation = 0u;
        runtime->mpeg_retire_ack_generation = 0u;
        return 0;
    }

    return 1;
}

int pstvnc_h1_transport_mpeg_retire_poll(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation)
{
    if (runtime == NULL || generation == 0u ||
        runtime->mpeg_retire_pending_generation != generation)
        return -1;

    if (runtime->mpeg_retire_ack_generation == generation)
        return 1;

    if (runtime->mpeg_retire_ack_generation != 0u ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->stop_requested || runtime->receiver_done)
        return -1;

    return 0;
}

int pstvnc_h1_transport_mpeg_retire_finalize(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t generation,
    uint32_t *bytes_discarded)
{
    size_t queued = 0u;
    uint32_t credit_amount = 0u;

    if (bytes_discarded != NULL)
        *bytes_discarded = 0u;

    if (runtime == NULL || generation == 0u ||
        !runtime->initialized || !runtime->config_accepted ||
        runtime->error != PSTVNC_H1_ERROR_NONE ||
        runtime->mpeg_data_generation != 0u ||
        runtime->mpeg_retire_pending_generation != generation ||
        runtime->mpeg_retire_ack_generation != generation)
        return 0;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES) {
        if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        queued = pstvnc_transport_queue_size(&runtime->mpeg_queue);
        if (queued > 0xffffffffu ||
            runtime->mpeg_credit_pending > 0xffffffffu - (uint32_t)queued ||
            runtime->stats.mpeg_bytes_discarded_generation_boundary >
                0xffffffffu - (uint32_t)queued) {
            (void)SignalSema(runtime->mpeg_queue_sema_id);
            h1_record_error(runtime, PSTVNC_H1_ERROR_OVERFLOW);
            return 0;
        }

        if (pstvnc_transport_queue_discard_all(&runtime->mpeg_queue) != queued) {
            (void)SignalSema(runtime->mpeg_queue_sema_id);
            h1_record_error(runtime, PSTVNC_H1_ERROR_MPEG_RETIRE);
            return 0;
        }

        if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        credit_amount = runtime->mpeg_credit_pending + (uint32_t)queued;
        if (credit_amount != 0u &&
            !h1_send_credit(
                runtime,
                PSTVNC_TRANSPORT_CHANNEL_MPEG2,
                credit_amount))
            return 0;

        runtime->mpeg_credit_pending = 0u;
        runtime->stats.mpeg_bytes_discarded_generation_boundary +=
            (uint32_t)queued;
    } else if (runtime->config.video_mode == PSTVNC_H1_VIDEO_OFF) {
        if (runtime->mpeg_credit_pending != 0u)
            return 0;
    } else {
        return 0;
    }

    runtime->mpeg_retire_ack_generation = 0u;
    runtime->mpeg_retire_pending_generation = 0u;
    if (bytes_discarded != NULL)
        *bytes_discarded = (uint32_t)queued;
    return 1;
}

int pstvnc_h1_transport_start(
    pstvnc_h1_transport_runtime_t *runtime)
{
    struct sockaddr_in server;

    if (runtime == NULL)
        return -1;

    memset(runtime, 0, sizeof(*runtime));
    runtime->socket_fd = -1;
    runtime->audio_queue_sema_id = -1;
    runtime->mpeg_queue_sema_id = -1;
    runtime->send_sema_id = -1;
    runtime->receiver_thread_id = -1;
    runtime->next_send_sequence = 1u;
    runtime->expected_receive_sequence = 1u;
    runtime->audio_crc_state = 0xffffffffu;
    runtime->mpeg_crc_state = 0xffffffffu;

    runtime->audio_queue_sema_id = h1_create_mutex();
    runtime->mpeg_queue_sema_id = h1_create_mutex();
    runtime->send_sema_id = h1_create_mutex();

    if (runtime->audio_queue_sema_id < 0 ||
        runtime->mpeg_queue_sema_id < 0 ||
        runtime->send_sema_id < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        goto fail;
    }

    runtime->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (runtime->socket_fd < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SOCKET);
        goto fail;
    }

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(PSTVNC_H1_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(PSTVNC_H1_SERVER_IP);

    if (connect(
            runtime->socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_CONNECT);
        goto fail;
    }

    runtime->initialized = 1;

    if (!h1_send_hello(runtime))
        goto fail;

    if (!h1_receive_config(runtime)) {
        (void)h1_send_error_code(runtime);
        goto fail;
    }

    if (!h1_allocate_queues(runtime) ||
        !h1_allocate_receiver_stack(runtime) ||
        !h1_apply_socket_options(runtime)) {
        (void)h1_send_error_code(runtime);
        goto fail;
    }

#ifdef PSTVNC_H1_RFB_MUX_PREP
    /*
     * Critical ordering invariant: if channel 1 is ever accepted by CONFIG, its
     * queue/semaphore/binding must exist before CONFIG ACK, initial credit, and
     * especially before the sole physical receiver thread can accept DATA.
     */
    if (!pstvnc_h1_rfb_transport_prepare(runtime)) {
        (void)h1_send_error_code(runtime);
        goto fail;
    }
#endif

    if (!h1_send_config_ack(runtime))
        goto fail;

    if (runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM &&
        !h1_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            runtime->config.audio_initial_credit_bytes))
        goto fail;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&
        !h1_send_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_MPEG2,
            runtime->config.mpeg_initial_credit_bytes))
        goto fail;

#ifdef PSTVNC_H1_RFB_MUX_PREP
    if (!pstvnc_h1_rfb_transport_send_initial_credit(runtime))
        goto fail;
#endif

    if (!h1_start_receiver(runtime))
        goto fail;

    return 0;

fail:
    (void)pstvnc_h1_transport_shutdown(runtime);
    return -1;
}

int pstvnc_h1_transport_shutdown(
    pstvnc_h1_transport_runtime_t *runtime)
{
    unsigned int wait_step;
    int result = 0;

    if (runtime == NULL)
        return -1;

    runtime->stop_requested = 1;

    if (runtime->socket_fd >= 0)
        (void)shutdown(runtime->socket_fd, SHUT_RDWR);

    if (runtime->receiver_thread_started) {
        for (wait_step = 0u;
             wait_step < H1_RECEIVER_STOP_WAIT_STEPS;
             ++wait_step) {
            ee_thread_status_t status;

            memset(&status, 0, sizeof(status));
            if (ReferThreadStatus(runtime->receiver_thread_id, &status) < 0) {
                result = -1;
                break;
            }

            if (status.status == THS_DORMANT) {
                if (DeleteThread(runtime->receiver_thread_id) < 0)
                    result = -1;
                else {
                    runtime->receiver_thread_id = -1;
                    runtime->receiver_thread_started = 0;
                }
                break;
            }

            if (DelayThread(H1_RECEIVER_STOP_WAIT_STEP_US) < 0) {
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

    if (runtime->audio_queue_sema_id >= 0) {
        if (DeleteSema(runtime->audio_queue_sema_id) < 0)
            result = -1;
        runtime->audio_queue_sema_id = -1;
    }

    if (runtime->mpeg_queue_sema_id >= 0) {
        if (DeleteSema(runtime->mpeg_queue_sema_id) < 0)
            result = -1;
        runtime->mpeg_queue_sema_id = -1;
    }

    if (runtime->send_sema_id >= 0) {
        if (DeleteSema(runtime->send_sema_id) < 0)
            result = -1;
        runtime->send_sema_id = -1;
    }

    free(runtime->receiver_thread_stack_allocation);
    free(runtime->audio_queue_storage);
    free(runtime->mpeg_queue_storage);

    runtime->receiver_thread_stack_allocation = NULL;
    runtime->receiver_thread_stack = NULL;
    runtime->audio_queue_storage = NULL;
    runtime->mpeg_queue_storage = NULL;
    runtime->config_accepted = 0;
    runtime->initialized = 0;
    return result;
}

const pstvnc_h1_config_t *pstvnc_h1_transport_config(
    const pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL ||
        !runtime->initialized ||
        !runtime->config_accepted)
        return NULL;

    return &runtime->config;
}

static size_t h1_queue_size(
    pstvnc_h1_transport_runtime_t *runtime,
    int sema,
    const pstvnc_transport_queue_t *queue,
    int enabled)
{
    size_t current;

    if (!enabled)
        return 0u;

    if (WaitSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0u;
    }

    current = pstvnc_transport_queue_size(queue);

    if (SignalSema(sema) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return 0u;
    }

    return current;
}

size_t pstvnc_h1_transport_audio_queue_size(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_queue_size(
        runtime,
        runtime->audio_queue_sema_id,
        &runtime->audio_queue,
        runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM);
}

size_t pstvnc_h1_transport_mpeg_queue_size(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return h1_queue_size(
        runtime,
        runtime->mpeg_queue_sema_id,
        &runtime->mpeg_queue,
        runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES);
}

int pstvnc_h1_transport_audio_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t buffer_capacity,
    size_t *bytes_read)
{
    size_t available;
    size_t take;
    int queue_empty;
    uint32_t frame_bytes;

    if (runtime == NULL || buffer == NULL || bytes_read == NULL ||
        runtime->config.audio_mode != PSTVNC_H1_AUDIO_PCM) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return -1;
    }

    frame_bytes = pstvnc_h1_config_audio_frame_bytes(&runtime->config);
    if (frame_bytes == 0u || buffer_capacity < frame_bytes) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return -1;
    }

    *bytes_read = 0u;
    runtime->stats.audio_read_calls++;

    if (runtime->error != PSTVNC_H1_ERROR_NONE)
        return -1;

    if (WaitSema(runtime->audio_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    available = pstvnc_transport_queue_size(&runtime->audio_queue);
    take = available < buffer_capacity ? available : buffer_capacity;
    take -= take % frame_bytes;

    if (take != 0u &&
        !pstvnc_transport_queue_read(&runtime->audio_queue, buffer, take)) {
        (void)SignalSema(runtime->audio_queue_sema_id);
        h1_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
        return -1;
    }

    queue_empty = pstvnc_transport_queue_size(&runtime->audio_queue) == 0u;

    if (SignalSema(runtime->audio_queue_sema_id) < 0) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
        return -1;
    }

    if (take == 0u)
        return 0;

    runtime->stats.audio_bytes_consumed += (uint32_t)take;

    if (!h1_return_credit(
            runtime,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            (uint32_t)take,
            queue_empty))
        return -1;

    *bytes_read = take;
    return 1;
}

/*
 * Disposable MPEG consumer-stage witness.
 *
 * Observation only: one volatile 32-bit store. No waits, sends, queue
 * operations, priority changes, or scheduling changes are introduced.
 */
static void h1_mpeg_diag_stage(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t stage)
{
    if (runtime != NULL)
        runtime->mpeg_diag_stage = stage;
}

int pstvnc_h1_transport_mpeg_read_cancellable(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read,
    const volatile int *cancel_requested)
{
    uint8_t *destination = (uint8_t *)buffer;
    unsigned int wait_loops = 0u;

    if (runtime == NULL || buffer == NULL || bytes_read == NULL ||
        maximum_count == 0u ||
        runtime->config.video_mode != PSTVNC_H1_VIDEO_MPEG2_ES) {
        h1_record_error(runtime, PSTVNC_H1_ERROR_ARGUMENT);
        return 0;
    }

    *bytes_read = 0u;
    runtime->stats.mpeg_read_calls++;
    h1_mpeg_diag_stage(
        runtime, 0xD5100001u); /* MPEG transport read enter */

    for (;;) {
        size_t available;

        if (cancel_requested != NULL && *cancel_requested)
            return 0;
        size_t take;
        int queue_empty;
        int end_received;

        if (runtime->error != PSTVNC_H1_ERROR_NONE)
            return 0;

        h1_mpeg_diag_stage(
            runtime, 0xD5100002u); /* queue WaitSema enter */

        if (WaitSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        h1_mpeg_diag_stage(
            runtime, 0xD5100003u); /* queue WaitSema return */

        available = pstvnc_transport_queue_size(&runtime->mpeg_queue);
        take = available < maximum_count ? available : maximum_count;
        end_received = runtime->end_received;

        if (take != 0u)
            h1_mpeg_diag_stage(
                runtime, 0xD5100004u); /* queue read enter */

        if (take != 0u &&
            !pstvnc_transport_queue_read(&runtime->mpeg_queue, destination, take)) {
            (void)SignalSema(runtime->mpeg_queue_sema_id);
            h1_record_error(runtime, PSTVNC_H1_ERROR_QUEUE_FULL);
            return 0;
        }

        if (take != 0u)
            h1_mpeg_diag_stage(
                runtime, 0xD5100005u); /* queue read return */

        queue_empty = pstvnc_transport_queue_size(&runtime->mpeg_queue) == 0u;

        h1_mpeg_diag_stage(
            runtime, 0xD5100006u); /* queue SignalSema enter */

        if (SignalSema(runtime->mpeg_queue_sema_id) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_SEMAPHORE);
            return 0;
        }

        h1_mpeg_diag_stage(
            runtime, 0xD5100007u); /* queue SignalSema return */

        if (take != 0u) {
            runtime->stats.mpeg_bytes_consumed += (uint32_t)take;

            h1_mpeg_diag_stage(
                runtime, 0xD5100008u); /* credit return enter */

            if (!h1_return_credit(
                    runtime,
                    PSTVNC_TRANSPORT_CHANNEL_MPEG2,
                    (uint32_t)take,
                    queue_empty))
                return 0;

            h1_mpeg_diag_stage(
                runtime, 0xD5100009u); /* credit return complete */

            if (wait_loops != 0u) {
                runtime->stats.mpeg_wait_events++;
                runtime->stats.mpeg_wait_loops += wait_loops;
                if (wait_loops > runtime->stats.mpeg_wait_max_loops)
                    runtime->stats.mpeg_wait_max_loops = wait_loops;
            }

            *bytes_read = take;
            h1_mpeg_diag_stage(
                runtime, 0xD510000Au); /* successful MPEG read return */
            return 1;
        }

        if (end_received)
            return 0;

        wait_loops++;

        h1_mpeg_diag_stage(
            runtime, 0xD510000Bu); /* empty DelayThread enter */

        if (DelayThread(runtime->config.mpeg_empty_delay_us) < 0) {
            h1_record_error(runtime, PSTVNC_H1_ERROR_THREAD_DELAY);
            return 0;
        }

        h1_mpeg_diag_stage(
            runtime, 0xD510000Cu); /* empty DelayThread return */
    }
}

int pstvnc_h1_transport_mpeg_read(
    pstvnc_h1_transport_runtime_t *runtime,
    void *buffer,
    size_t maximum_count,
    size_t *bytes_read)
{
    return pstvnc_h1_transport_mpeg_read_cancellable(
        runtime, buffer, maximum_count, bytes_read, NULL);
}

int pstvnc_h1_transport_audio_exhausted(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return runtime != NULL &&
        runtime->end_received &&
        pstvnc_h1_transport_audio_queue_size(runtime) == 0u;
}

int pstvnc_h1_transport_mpeg_exhausted(
    pstvnc_h1_transport_runtime_t *runtime)
{
    return runtime != NULL &&
        runtime->end_received &&
        pstvnc_h1_transport_mpeg_queue_size(runtime) == 0u;
}

int pstvnc_h1_transport_integrity_pass(
    pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->end_received ||
        runtime->error != PSTVNC_H1_ERROR_NONE)
        return 0;

    if (runtime->config.audio_mode == PSTVNC_H1_AUDIO_PCM &&
        (runtime->producer_audio_bytes != runtime->stats.audio_bytes_enqueued ||
         runtime->producer_audio_crc32 != runtime->audio_crc32))
        return 0;

    if (runtime->config.video_mode == PSTVNC_H1_VIDEO_MPEG2_ES &&
        (runtime->producer_mpeg_bytes != runtime->stats.mpeg_bytes_enqueued ||
         runtime->producer_mpeg_crc32 != runtime->mpeg_crc32))
        return 0;

    return 1;
}

void pstvnc_h1_transport_record_audio_played(
    pstvnc_h1_transport_runtime_t *runtime,
    size_t byte_count)
{
    if (runtime == NULL)
        return;

    runtime->stats.audio_chunks_played++;
    runtime->stats.audio_bytes_played += (uint32_t)byte_count;
}

void pstvnc_h1_transport_set_diagnostic_word(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t diagnostic_word)
{
    if (runtime != NULL)
        runtime->diagnostic_word = diagnostic_word;
}

pstvnc_h1_transport_error_t pstvnc_h1_transport_last_error(
    const pstvnc_h1_transport_runtime_t *runtime)
{
    return runtime == NULL ? PSTVNC_H1_ERROR_ARGUMENT : runtime->error;
}

int pstvnc_h1_transport_send_result(
    pstvnc_h1_transport_runtime_t *runtime,
    uint32_t pictures_decoded,
    uint32_t pictures_displayed,
    uint32_t feed_calls,
    uint32_t payload_bytes_submitted,
    uint32_t dma_bytes_submitted,
    uint32_t deadline_misses,
    uint32_t max_deadline_late_ticks_lo)
{
    uint8_t payload[PSTVNC_H1_RESULT_BYTES];

    if (runtime == NULL)
        return 0;

    memset(payload, 0, sizeof(payload));

#define PUT32(offset, value) \
    pstvnc_transport_write_be32(&payload[(offset)], (uint32_t)(value))

    PUT32(0, PSTVNC_H1_RESULT_VERSION);
    PUT32(4, runtime->config.session_id);
    PUT32(8, runtime->error);
    PUT32(12, pstvnc_h1_transport_integrity_pass(runtime));
    PUT32(16, pictures_decoded);
    PUT32(20, pictures_displayed);
    PUT32(24, feed_calls);
    PUT32(28, payload_bytes_submitted);
    PUT32(32, dma_bytes_submitted);
    PUT32(36, deadline_misses);
    PUT32(40, max_deadline_late_ticks_lo);
    PUT32(44, runtime->producer_picture_starts);
    PUT32(48, runtime->stats.audio_bytes_enqueued);
    PUT32(52, runtime->stats.audio_bytes_consumed);
    PUT32(56, runtime->stats.audio_bytes_played);
    PUT32(60, runtime->stats.mpeg_bytes_enqueued);
    PUT32(64, runtime->stats.mpeg_bytes_consumed);
    PUT32(68, runtime->stats.mpeg_wait_events);
    PUT32(72, runtime->stats.mpeg_wait_loops);
    PUT32(76, runtime->stats.mpeg_wait_max_loops);
    PUT32(80, runtime->stats.audio_credit_bytes_sent);
    PUT32(84, runtime->stats.mpeg_credit_bytes_sent);
    PUT32(88, runtime->producer_audio_crc32);
    PUT32(92, runtime->audio_crc32);
    PUT32(96, runtime->producer_mpeg_crc32);
    PUT32(100, runtime->mpeg_crc32);
    PUT32(104, runtime->config_digest);
    PUT32(108, runtime->config.profile_id);
    PUT32(112, runtime->config.audio_mode);
    PUT32(116, runtime->config.video_mode);
    PUT32(120, runtime->producer_stop_reason);
    PUT32(124, runtime->diagnostic_word);

#undef PUT32

    return h1_send_frame(
        runtime,
        PSTVNC_H1_FRAME_SESSION_RESULT,
        PSTVNC_TRANSPORT_CHANNEL_TELEMETRY,
        0,
        payload,
        sizeof(payload));
}
